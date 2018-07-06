/*
 * PlayerManager.cpp
 *
 * Created on: 2013-03-23 11:32
 *     Author: lyz
 */

#include "PlayerManager.h"
#include "MapPlayerEx.h"
#include "MapLogicPlayer.h"
#include "Scene.h"
#include "MapMonitor.h"
#include "TransactionMonitor.h"
#include "Transaction.h"
#include "ProtoDefine.h"

PlayerManager::PlayerManager(void) :
    monitor_(0),
    prev_player_map_(get_hash_table_size(MapMonitor::MAP_OBJECT_BUCKET)),
    player_map_(get_hash_table_size(MapMonitor::MAP_OBJECT_BUCKET)),
    sid_player_map_(get_hash_table_size(MapMonitor::MAP_OBJECT_BUCKET)),
    prev_logic_player_map_(get_hash_table_size(MapMonitor::MAP_OBJECT_BUCKET)),
    logic_player_map_(get_hash_table_size(MapMonitor::MAP_OBJECT_BUCKET)),
    role_clientsid_map_(get_hash_table_size(MapMonitor::MAP_OBJECT_BUCKET))
{
    this->monitor_ = MAP_MONITOR;
}

PlayerManager::~PlayerManager(void)
{
    this->prev_player_map_.unbind_all();
    this->player_map_.unbind_all();
    this->sid_player_map_.unbind_all();

    this->prev_logic_player_map_.unbind_all();
    this->logic_player_map_.unbind_all();

    this->role_clientsid_map_.unbind_all();
}

MapMonitor *PlayerManager::monitor(void)
{
	return this->monitor_;
}

int PlayerManager::request_map_player_login(const int gate_sid, const Int64 role_id, Message *msg)
{
	//每个玩家每秒最多只处理一次
    int64_t last_tick = 0;
    if (this->map_login_tick_map_.find(role_id, last_tick) == 0
    		&& last_tick == Time_Value::gettimeofday().sec())
    {
    	return -1;
    }

    //有上次玩家在线
    MapPlayerEx *player = NULL;
    if (this->find_player(role_id, player) == 0)
    {
        MSG_USER("ERROR map player unbind before load %ld %s %s %d",
        		player->role_id(),player->role_detail().__account.c_str(),
                player->name(), player->is_enter_scene());

        if (player->is_enter_scene())
        {
            player->exit_scene(EXIT_SCENE_ERROR);
        }

        player->sign_out(true);
        this->unbind_player(role_id);
    }

    player = this->monitor()->player_pool()->pop();
    JUDGE_RETURN(player != NULL, -1);

    Time_Value nowtime = Time_Value::gettimeofday();
    this->map_login_tick_map_.rebind(role_id, nowtime.sec());
    this->map_logout_tick_map_.rebind(role_id, 0);

    player->set_role_id(role_id);
    player->set_gate_sid(gate_sid);
    player->set_is_loading_mongo(true);
    player->set_load_mongo_tick(nowtime);

    if (TRANSACTION_MONITOR->request_mongo_transaction(role_id, TRANS_LOAD_MAP_PLAYER,
                DB_MAP_PLAYER, player, this->monitor()->player_pool(),
                this->monitor()->map_unit(), gate_sid) != 0)
    {
        this->monitor()->player_pool()->push(player);
        return -1;
    }

    return 0;
}

int PlayerManager::after_load_player(Transaction *transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);

    if (transaction->detail().__error != 0)
    {
        transaction->rollback();
        return -1;
    }

    TransactionData *trans_data = transaction->fetch_data(DB_MAP_PLAYER);
    JUDGE_RETURN(trans_data != NULL, -1);

    MapPlayerEx *player = trans_data->__data.__map_player;
    JUDGE_RETURN(player != NULL, -1);

    trans_data->reset();
    transaction->summit();

    // 处理短时间内重复登录导致的多个请求异常
    int64_t logout_tick = 0, login_tick = 0, player_login_tick = 0;
    if (this->map_login_tick_map_.find(player->role_id(), login_tick) != 0
    		|| login_tick > player->load_mongo_tick().sec())
    {
        if (player != NULL)
        {
            player_login_tick = player->load_mongo_tick().sec();
        }

        MSG_USER("ERROR map player has relogin %ld %s %s load_tick(%ld %ld)", player->role_id(),
                player->role_detail().__account.c_str(), player->name(),
                player_login_tick, login_tick);

        this->monitor()->player_pool()->push(player);
        return -1;
    }

    // 处理加载过程中的离线
    if (this->map_logout_tick_map_.find(player->role_id(), logout_tick) == 0
    		&& logout_tick >= player->load_mongo_tick().sec())
    {
        MSG_USER("ERROR map player has signout %ld %s %s load_tick(%ld %ld)",
        		player->role_id(),  player->role_detail().__account.c_str(),
        		player->name(), player->load_mongo_tick().sec(), logout_tick);

        this->monitor()->player_pool()->push(player);
        return -1;
    }

    MapPlayerEx *error_player = NULL;
    if (this->find_player(player->role_id(), error_player) == 0)
    {
    	//error_player->role_id()和player->role_id()可能不一样
        MSG_USER("ERROR map player unbind %ld %ld %s %s %d",
        		player->role_id(),	error_player->role_id(),
                error_player->role_detail().__account.c_str(),
                error_player->name(), error_player->is_enter_scene());

        if (error_player->is_enter_scene())
        {
            error_player->exit_scene();
        }

        error_player->sign_out(false);
        this->unbind_player(player->role_id());
    }

    //以前遗留玩家对象
    if (this->prev_player_map_.find(player->role_id(), error_player) == 0)
    {
        MSG_USER("ERROR prev map player unbind %ld %s %s", error_player->role_id(),
                error_player->role_detail().__account.c_str(), error_player->name());

        this->prev_player_map_.unbind(player->role_id());
        error_player->sign_out(false);
    }

    player->set_is_loading_mongo(false);
    player->set_is_login(true);

    int ret = player->sign_in(ENTER_SCENE_LOGIN);
    MSG_USER("PlayerManager sign in %d, %ld, %s", ret, player->role_id(), player->name());

    if (ret == 0)
    {
        player->start_login_map_logic(login_tick);
    }
    else
    {
    	player->sign_out(false);
    }
    return ret;
}

int PlayerManager::process_map_player_logout(const int gate_sid, const Int64 role_id)
{
	this->map_logout_tick_map_.rebind(role_id, Time_Value::gettimeofday().sec());

    MapPlayerEx *player = NULL;
    if (this->find_player(role_id, player) != 0)
    {
        MSG_USER("ERROR map player no found %ld %d", role_id, gate_sid);
        return 0;
    }

//    // 可能离线请求来自不同的网关，忽略不是登录网关的请求
//    if (player->gate_sid() != gate_sid)
//    {
//    	MSG_USER("ERROR map player gate_sid diff %ld %s %d %d", role_id,
//    			player->name(), player->gate_sid(), gate_sid);
//    	return 0;
//    }

    return player->request_logout_game();
}

int PlayerManager::request_load_logic_player(const int gate_sid, const int64_t role_id)
{
    MapLogicPlayer *player = NULL;
    if (this->find_logic_player(role_id, player) == 0)
    {
        MSG_USER("ERROR ml player unbind before load %ld %s %s %d", player->role_id(),
                player->role_detail().__account.c_str(),
                player->name(), player->is_active());

        player->sign_out(EXIT_SCENE_ERROR, true);
        this->unbind_logic_player(role_id);
    }

    Time_Value nowtime = Time_Value::gettimeofday();
    this->ml_login_tick_map_.rebind(role_id, nowtime.sec());
    this->ml_logout_tick_map_.rebind(role_id, 0);
    MSG_USER("ml player load mongo %ld %ld", role_id, nowtime.sec());

    player = this->monitor()->logic_player_pool()->pop();
    player->set_is_loading_mongo(true);
    player->set_load_mongo_tick(nowtime);

    if (player->request_load_data(gate_sid, role_id) != 0)
    {
        this->monitor()->logic_player_pool()->push(player);
        return -1;
    }
    return 0;
}

int PlayerManager::after_load_logic_player(Transaction *transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);

    if (transaction->detail().__error != 0)
    {
        transaction->rollback();
        return -1;
    }

    TransactionData *trans_data = transaction->fetch_data(DB_MAP_LOGIC_PLAYER);
    JUDGE_RETURN(trans_data != NULL, -1);

    MapLogicPlayer *player = trans_data->__data.__map_logic_player;
    trans_data->reset();
    transaction->summit();

    //短时间多个请求
    int64_t logout_tick = 0, login_tick = 0, player_login_tick = 0;
    if (this->ml_login_tick_map_.find(player->role_id(), login_tick) != 0
    		|| login_tick > player->load_mongo_tick().sec())
    {
        if (player != NULL)
        {
            player_login_tick = player->load_mongo_tick().sec();
        }

        MSG_USER("ERROR ml player has relogin %ld %s %s load_tick(%ld %ld)",
        		player->role_id(), player->role_detail().__account.c_str(),
                player->name(), player_login_tick, login_tick);

        this->monitor()->logic_player_pool()->push(player);
        return -1;
    }

    // 加载过程中离线
    if (this->ml_logout_tick_map_.find(player->role_id(), logout_tick) == 0
    		&& logout_tick >= player->load_mongo_tick().sec())
    {
        MSG_USER("ERROR ml player has signout %ld %s %s load_tick(%ld %ld)",
        		player->role_id(), player->role_detail().__account.c_str(),
        		player->name(), player->load_mongo_tick().sec(), logout_tick);

        this->monitor()->logic_player_pool()->push(player);
        return -1;
    }

    MapLogicPlayer *error_player = NULL;
    if (this->prev_logic_player_map_.find(player->role_id(), error_player) == 0)
    {
		MSG_USER("ERROR prev ml player unbind %ld %s %s", error_player->role_id(),
				error_player->role_detail().__account.c_str(), error_player->name());

		this->prev_logic_player_map_.unbind(player->role_id());
		error_player->sign_out(EXIT_SCENE_ERROR, false);
    }

    if (this->find_logic_player(player->role_id(), error_player) == 0)
    {
        MSG_USER("ERROR ml player unbind %ld %s %s %d", error_player->role_id(),
                error_player->role_detail().__account.c_str(),
                error_player->name(), error_player->is_active());

        this->unbind_logic_player(player->role_id());
        error_player->sign_out(EXIT_SCENE_ERROR, false);
    }

    player->set_is_loading_mongo(false);
    if (player->sign_in(ENTER_SCENE_LOGIN) != 0)
    {
    	MSG_USER("ERROR ml bind error %ld %s", player->role_id(), player->name());
        this->monitor()->logic_player_pool()->push(player);
    }
    else
    {
    	player->sync_info_before_enter_scene(ENTER_SCENE_LOGIN);
    }
    return 0;
}

int PlayerManager::process_ml_player_logout(int gate_sid, Int64 role_id, int type)
{
	this->ml_logout_tick_map_.rebind(role_id, Time_Value::gettimeofday().sec());

    MapLogicPlayer *player = NULL;
    if (this->prev_logic_player_map_.find(role_id, player) == 0
    		&& player->is_loading_mongo() == false)
    {
    	this->prev_logic_player_map_.unbind(role_id);
    	this->monitor()->logic_player_pool()->push(player);
    	MSG_USER("ERROR prev ml player logout %ld %d", role_id, gate_sid);
    }

    if (this->find_logic_player(role_id, player) != 0)
    {
        MSG_USER("ERROR ml player no found %ld %d", role_id, gate_sid);
        return 0;
    }

    // 可能离线请求来自不同的网关，忽略不是登录网关的请求
    if (player->gate_sid() != gate_sid)
    {
    	MSG_USER("ERROR ml player gate_sid diff %ld %s %d %d", player->role_id(),
    			player->name(), player->gate_sid(), gate_sid);
//    	return 0;
    }

    return player->sign_out(type);
}

int PlayerManager::update_transfer_base(const int gate_sid, Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400101*, request, -1);

    MapPlayerEx* prev_player = NULL;
    if (this->prev_player_map_.find(role_id, prev_player) == 0)
    {
        this->prev_player_map_.unbind(role_id);
        this->monitor()->player_pool()->push(prev_player);
    }

    Int64 transfer_tick = request->transfer_tick();
    Time_Value nowtime = Time_Value::gettimeofday();
    if ((transfer_tick > 0) && (transfer_tick + Time_Value::MINUTE < nowtime.sec()))
    {
    	//传送消息超时
    	MSG_USER("ERROR map transfer start timeout %ld %s %d %ld", request->role_id(),
    			request->name().c_str(), request->scene_id(), transfer_tick);
    	return -1;
    }

	MapPlayerEx* player = this->monitor()->player_pool()->pop();
    player->read_transfer_base(msg);
    player->set_is_loading_mongo(true);
    player->set_load_mongo_tick(nowtime);

    this->map_login_tick_map_.rebind(player->role_id(), nowtime.sec());
    this->prev_player_map_.rebind(player->role_id(), player);
    return 0;
}

int PlayerManager::finish_sync_transfer(const int gate_sid, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400102*, request, -1);
	MSG_USER("finish_sync_transfer map player %ld", request->role_id());

    MapPlayerEx *player = 0;
    JUDGE_RETURN(this->prev_player_map_.find(request->role_id(), player) == 0, -1);
    this->prev_player_map_.unbind(request->role_id());

    Int64 transfer_tick = request->transfer_tick();
    Time_Value nowtime = Time_Value::gettimeofday();

    if ((transfer_tick > 0) && (transfer_tick + Time_Value::MINUTE < nowtime.sec()))
    {
    	//传送消息超时
    	MSG_USER("ERROR map transfer finish timeout %ld %ld", request->role_id(), transfer_tick);
    	this->monitor()->player_pool()->push(player);
    	return -1;
    }

    // 处理加载过程中的离线
//    int64_t logout_tick = 0;
//    if (this->map_logout_tick_map_.find(player->role_id(), logout_tick) == 0
//    		&& logout_tick > player->load_mongo_tick().sec())
//    {
//        MSG_USER("map player has signout %ld %s %s load_tick(%ld %ld)", player->role_id(),
//                player->role_detail().__account.c_str(), player->name(),
//                player->load_mongo_tick().sec(), logout_tick);
//
//        this->monitor()->player_pool()->push(player);
//        return -1;
//    }

    MapPlayerEx *error_player = NULL;
    if (this->find_player(player->role_id(), error_player) == 0)
    {
        MSG_USER("ERROR map player unbind %ld %s %s %d", error_player->role_id(),
                error_player->role_detail().__account.c_str(),
                error_player->name(), error_player->is_enter_scene());

        if (error_player->is_enter_scene())
        {
            error_player->exit_scene();
        }

        error_player->sign_out(false);
        this->unbind_player(player->role_id());
    }

    player->set_is_loading_mongo(false);
    player->set_gate_sid(gate_sid);
    if (player->sign_in() != 0)
    {
        MSG_USER("ERROR map player signin %ld %s %s %d", player->role_id(), 
                player->role_detail().__account.c_str(),
                player->name(), player->is_enter_scene());

        this->monitor()->player_pool()->push(player);
        return -1;
    }

    MSG_USER("transfer map player sign in %ld", request->role_id());
    this->monitor()->process_inner_logic_request(request->role_id(), INNER_TRANSFER_SCENE_END);

    return 0;
}

// finish sync logic thread info
int PlayerManager::finish_sync_transfer(const int64_t role_id)
{
    return 0;
}

int PlayerManager::sync_transfer_map(const int64_t role_id, const int recogn, Message *msg)
{
    MapPlayerEx *player = 0;
    JUDGE_RETURN(this->prev_player_map_.find(role_id, player) == 0, -1);

    switch (recogn)
    {
    case INNER_SYNC_PLAYER_MOVE:
        return player->unserialize_move(msg);
    case INNER_SYNC_PLAYER_FIGHT:
        return player->unserialize_fight(msg);
    case INNER_SYNC_PLAYER_SCRIPT:
        return player->unserialize_script(msg);
    case INNER_SYNC_PLAYER_VIP:
    	return player->unserialize_vip(msg);
    case INNER_SYNC_PLAYER_MAPTEAM:
    	return player->read_transfer_team(msg);
    case INNER_SYNC_PLAYER_KILL:
    	return player->read_transfer_killer(msg);
    case INNER_SYNC_PLAYER_TINY:
    	return player->read_transfer_tiny(msg);
    case INNER_SYNC_PLAYER_ESCORT:
    	return player->read_transfer_escort(msg);
    case INNER_SYNC_PLAYER_SHAPE_AND_LABEL:
    	return player->read_transfer_shape_and_label(msg);
    case INNER_SYNC_PLAYER_BATTLEGRUOND:
    	return player->read_transfer_sm_battler(msg);
    case INNER_SYNC_LEAGUER_INFO:
    	return player->read_transfer_leaguer(msg);
    }
    return 0;
}

int PlayerManager::sync_transfer_online(const int gate_sid, Proto30400105 *request)
{
    JUDGE_RETURN(request != NULL, -1);

    MapPlayerEx *player = 0;
    JUDGE_RETURN(this->prev_player_map_.find(request->role_id(), player) == 0, -1);

    return player->unserialize_online(request);
}

int PlayerManager::bind_player(const int64_t role_id, MapPlayerEx *player)
{
    return this->player_map_.bind(role_id, player);
}

int PlayerManager::unbind_player(const int64_t role_id)
{
    return this->player_map_.unbind(role_id);
}

int PlayerManager::find_player(const int64_t role_id, MapPlayerEx *&player)
{
    return this->player_map_.find(role_id, player);
}

int PlayerManager::process_player_size(void)
{
    return this->player_map_.size();
}

int PlayerManager::get_map_player_set(MapPlayerExVec& map_player_set)
{
	map_player_set.clear();
	map_player_set.reserve(this->player_map_.size());

	for(PlayerMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++ iter)
	{
		map_player_set.push_back(iter->second);
	}

	return 0;
}

void PlayerManager::reset_map_player_everyday()
{
	for(PlayerMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++ iter)
	{
		MapPlayerEx* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		player->reset_everyday();
	}
}

int PlayerManager::bind_sid_player(const int client_sid, MapPlayerEx *player)
{
//    if (player->is_same_thread() == false)
//    {
//        MSG_USER("ERROR no same thread");
//        return -1;
//    }

#ifdef LOCAL_DEBUG
    this->role_clientsid_map_.rebind(player->role_id(), client_sid);
#endif
    return this->sid_player_map_.bind(client_sid, player);
}

int PlayerManager::unbind_sid_player(const int client_sid, MapPlayerEx *&player)
{
    int ret = this->sid_player_map_.unbind(client_sid, player);
#ifdef LOCAL_DEBUG
    if (ret == 0)
    	this->role_clientsid_map_.unbind(player->role_id());
#endif
    return ret;
}

int PlayerManager::find_sid_player(const int client_sid, MapPlayerEx *&player)
{
    int ret = this->sid_player_map_.find(client_sid, player);
//    if (ret == 0)
//    {
//        if (player->is_same_thread() == true)
//            return 0;
//        else
//            MSG_USER("ERROR no same thread");
//    }
    return ret;
}

int PlayerManager::find_client_sid_by_role(const int64_t role_id, int &client_sid)
{
    return this->role_clientsid_map_.find(role_id, client_sid);
}

int PlayerManager::bind_logic_player(const int64_t role_id, MapLogicPlayer *player)
{
    return this->logic_player_map_.bind(role_id, player);
}

int PlayerManager::unbind_logic_player(const int64_t role_id)
{
    return this->logic_player_map_.unbind(role_id);
}

int PlayerManager::find_logic_player(const int64_t role_id, MapLogicPlayer *&player)
{
    int ret = this->logic_player_map_.find(role_id, player);
//    if (ret == 0)
//    {
//        if (player->is_same_thread() == true)
//            return 0;
//        else
//            MSG_USER("ERROR no same thread");
//    }
    return ret;
}

int PlayerManager::get_logic_player_set(LogicPlayerSet& logic_player_set)
{
	logic_player_set.clear();
	logic_player_set.reserve(this->logic_player_map_.size());

	for(LogicPlayerMap::iterator iter = logic_player_map_.begin();
			iter != logic_player_map_.end(); ++ iter)
	{
		JUDGE_CONTINUE(iter->second->is_active() == true);
		logic_player_set.push_back(iter->second);
	}

	return 0;
}

int PlayerManager::update_transfer_logic(Int64 role_id, int recogn, Message* msg)
{
    MapLogicPlayer *player = 0;
    JUDGE_RETURN(this->prev_logic_player_map_.find(role_id, player) == 0, -1);

    switch (recogn)
    {
    case INNER_ML_SYNC_PACKAGE:
    	return player->unserialize_package(msg);
    case INNER_ML_SYNC_MOUNT:
    	return player->read_transfer_mounter(msg);
    case INNER_MAP_SYNC_CHECK_IN_INFO:
    	return player->read_transfer_check_in(msg);
    case INNER_ML_SYNC_VIP:
    	return player->read_transfer_vip(msg);
    case INNER_ML_SYNC_EXP_RESOTRE:
    	return player->read_transfer_exp_restore(msg);
    case INNER_ML_SYNC_ONLINE_AWARD:
    	return player->read_transfer_online_rewards(msg);
    case INNER_ML_SYNC_OFFLINE_AWARD:
    	return player->read_transfer_offline_rewards(msg);
    case INNER_ML_SYNC_TREASURES_INFO:
    	return player->read_transfer_treasures_info(msg);
    case INNER_ML_SYNC_ILLUS:
    	return player->read_transfer_illus_info(msg);
    case INNER_ML_SYNC_COLLECT_CHESTS:
    	return player->read_transfer_collect_chests(msg);
    case INNER_ML_SYNC_MAGICAL_POLISH:
    	return player->read_transfer_equipment_info(msg);
    case INNER_ML_SYNC_LABEL_INFO:
    	return player->read_transfer_label_info(msg);
    case INNER_ML_SYNC_ACHIEVEMENT:
    	return player->read_transfer_achieve(msg);
    case INNER_ML_SYNC_MAIL_BOX_INFO:
    	return player->read_transfer_mail_box_info(msg);
    case INNER_ML_SYNC_PLAYER_TIP_PANNEL:
    	return player->read_transfer_tip_pannel(msg);
    case INNER_ML_SYNC_MEDIA_GIFT:
    	return player->read_transfer_media_gift(msg);
    case INNER_ML_SYNC_RECHARGE_REWARDS:
    	return player->read_transfer_recharge_rewards(msg);
    case INNER_ML_SYNC_SCRIPT_CLEAN:
    	return player->unserialize_script_clean(msg);
    case INNER_ML_SYNC_DAILY_RECHARGE:
    	return player->read_transfer_daily_recharge(msg);
    case INNER_ML_SYNC_REBATE_RECHARGE:
        return player->read_transfer_rebate_recharge(msg);
    case INNER_ML_SYNC_INVEST_RECHARGE:
        return player->read_transfer_invest_recharge(msg);
    case INNER_ML_SYNC_TINY:
    	return player->read_transfer_tiny(msg);
    case INNER_ML_SYNC_TOTAL_REWARDS:
    	return player->read_transfer_total_recharge(msg);
    case INNER_ML_SYNC_MAGICWEAPON:
        return player->read_transfer_magic_weapon_info(msg);
    case INNER_ML_SYNC_SWORD_POOL_INFO:
    	return player->read_transfer_spool(msg);
    case INNER_ML_SYNC_FASHION_INFO:
    	return player->read_transfer_fashion(msg);
    case INNER_ML_SYNC_TRANSFER_INFO:
    	return player->read_transfer_trans(msg);
    case INNER_ML_SYNC_HIDDEN_TREASURE_INFO:
    	return player->read_transfer_hi_treasure(msg);
    case INNER_MAP_SYNC_LOGIC_TASK:
        return player->unserialize_task(msg);
    case SYNC_OFFLINE_HOOK_INFO:
    	return player->read_hook_sync_transfer_scen(msg);
    }

    return 0;
}

int PlayerManager::update_transfer_logic_base(const int gate_sid, Proto31400103 *request)
{
	JUDGE_RETURN(request != NULL, -1);

    MapLogicPlayer *player = NULL;
    if (this->prev_logic_player_map_.find(request->role_id(), player) == 0)
    {
    	MSG_USER("ERROR exist ml player %ld", request->role_id());
        this->prev_logic_player_map_.unbind(request->role_id());
        this->monitor()->logic_player_pool()->push(player);
    }

    Int64 transfer_tick = request->transfer_tick();
    Time_Value nowtime = Time_Value::gettimeofday();
    if (transfer_tick > 0 && (transfer_tick + Time_Value::MINUTE < nowtime.sec()))
    {
    	//传送消息超时
    	MSG_USER("ERROR logic transfer start timeout %ld %s %d %ld", request->role_id(),
    			request->role_name().c_str(), request->scene_id(), transfer_tick);
    	return -1;
    }

    player = this->monitor()->logic_player_pool()->pop();
    player->set_is_loading_mongo(true);
    player->set_load_mongo_tick(nowtime);

    player->unserialize_base(request);
    MSG_USER("ml player base %ld %s", player->role_id(), player->name());

    this->ml_login_tick_map_.rebind(request->role_id(), nowtime.sec());
    this->prev_logic_player_map_.rebind(player->role_id(), player);
    return 0;
}

int PlayerManager::update_transfer_logic_package(const int gate_id, Proto31400104 *request)
{
	return 0;
}

int PlayerManager::update_transfer_logic_online(const int gate_sid, Proto31400105 *request)
{
    MapLogicPlayer *player = 0;
    JUDGE_RETURN(this->prev_logic_player_map_.find(request->role_id(), player) == 0, -1);

    return player->unserialize_online(request);
}

int PlayerManager::finish_sync_transfer_logic_info(int gate_sid, Proto31400102 *request)
{
	MSG_USER("logic info %d %ld", gate_sid, request->role_id());

    MapLogicPlayer *player = 0;
    JUDGE_RETURN(this->prev_logic_player_map_.find(request->role_id(), player) == 0, -1);

    Int64 transfer_tick = request->transfer_tick();
    Time_Value nowtime = Time_Value::gettimeofday();

    if (transfer_tick > 0 && (transfer_tick + Time_Value::MINUTE < nowtime.sec()))
    {
    	//传送消息超时
    	MSG_USER("ERROR logic transfer finish timeout %ld %ld", request->role_id(), transfer_tick);
    	this->prev_logic_player_map_.unbind(request->role_id());
    	this->monitor()->logic_player_pool()->push(player);
    	return -1;
    }
    else
    {
		player->set_gate_sid(gate_sid);
		player->set_is_loading_mongo(false);
		return 0;
    }
}

// finish sync map thread info
int PlayerManager::finish_sync_transfer_start_logic(int64_t role_id)
{
	MapLogicPlayer *player = NULL;
    JUDGE_RETURN(this->prev_logic_player_map_.find(role_id, player) == 0, -1);

    this->prev_logic_player_map_.unbind(role_id);

#ifndef LOCAL_DEBUG
    // 加载过程中离线
    int64_t logout_tick = 0;
    if (this->ml_logout_tick_map_.find(player->role_id(), logout_tick) == 0
    		&& logout_tick >= player->load_mongo_tick().sec())
    {
        MSG_USER("ERROR ml player has signout %ld %s %s load_tick(%ld %ld)", player->role_id(),
                player->role_detail().__account.c_str(), player->name(),
                player->load_mongo_tick().sec(), logout_tick);

        this->monitor()->logic_player_pool()->push(player);
        return -1;
    }
#endif

    MapLogicPlayer *error_player = NULL;
    if (this->find_logic_player(role_id, error_player) == 0)
    {
        MSG_USER("ERROR ml player unbind %ld %s %s %d", error_player->role_id(),
                error_player->role_detail().__account.c_str(),
                error_player->name(), error_player->is_active());

        error_player->sign_out(EXIT_SCENE_ERROR, false);
        this->unbind_logic_player(role_id);
    }

    if (player->sign_in() != 0)
    {
        MSG_USER("ERROR ml player signin %ld %s %s %d", error_player->role_id(), 
                error_player->role_detail().__account.c_str(),
                error_player->name(), error_player->is_active());
        this->monitor()->logic_player_pool()->push(player);
        return -1;
    }
    else
    {
    	player->sync_info_before_enter_scene(ENTER_SCENE_TRANSFER);
    	return 0;
    }
}

int PlayerManager::logout_map_all_player(void)
{
	MapPlayerExVec player_list;
	this->get_map_player_set(player_list);

    for (MapPlayerExVec::iterator iter = player_list.begin();
    		iter != player_list.end(); ++iter)
    {
    	MapPlayerEx* player = *iter;
        JUDGE_CONTINUE(player->is_online_player() == true);

        MSG_USER("stop map player logout %s %ld", player->name(), player->role_id());
        player->sign_out();
    }

    return 0;
}

int PlayerManager::logout_logic_all_player(void)
{
//    std::vector<MapLogicPlayer *> player_list;
//    MapLogicPlayer *player = 0;
//    for (LogicPlayerMap::iterator iter = this->logic_player_map_.begin();
//    		iter != this->logic_player_map_.end(); ++iter)
//    {
//        player_list.push_back(iter->second);
//    }

	LogicPlayerSet player_list;
	this->get_logic_player_set(player_list);

    for (std::vector<MapLogicPlayer *>::iterator iter = player_list.begin();
    		iter != player_list.end(); ++iter)
    {
    	MapLogicPlayer* player = *iter;
        MSG_USER("stop map_logic player logout %s %ld", player->name(), player->role_id());

        player->sign_out(EXIT_SCENE_LOGOUT);
    }
    return 0;
}

int PlayerManager::notify_all_player_offline()
{
	MapPlayerExVec player_list;
	this->get_map_player_set(player_list);

    for (MapPlayerExVec::iterator iter = player_list.begin();
    		iter != player_list.end(); ++iter)
    {
    	MapPlayerEx* player = *iter;
        JUDGE_CONTINUE(player->is_online_player() == true);

        player->notify_client_map_offline();
    }

    return 0;
}

int PlayerManager::update_transfer_logic_by_bson(Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401404*, request, -1);

	MapLogicPlayer *ml_player = 0;
	JUDGE_RETURN(this->prev_logic_player_map_.find(role_id, ml_player) == 0, -1);

	switch(request->obj_type())
	{
	case GameEnum::MAP_LOGIC_ONCE_REWARDS:
		return ml_player->read_transfer_once_rewards(msg);
	default:
		MSG_USER("update_transfer_logic_by_bson(), unknow object type %ld", request->obj_type());
		return -1;
	}
}

int PlayerManager::sync_transfer_map_by_bson(Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400114*, request, ERROR_SERVER_INNER);

    MapPlayerEx *player = 0;
    JUDGE_RETURN(this->prev_player_map_.find(role_id, player) == 0, -1);

    switch (request->obj_type())
    {
//    case GameEnum::MAP_OBJ_HOME_CITY_BLESS:
//    	return player->read_transfer_bless(msg);
	default:
		MSG_USER("sync_transfer_map_by_bson(), unknow object type %ld", request->obj_type());
		return -1;
    }
    return 0;
}

Time_Value PlayerManager::map_player_login_tick(const Int64 role_id)
{
	int64_t login_tick = 0;
	if (this->map_login_tick_map_.find(role_id, login_tick) == 0)
		return Time_Value(login_tick);
	return Time_Value::zero;
}

Time_Value PlayerManager::map_player_logout_tick(const Int64 role_id)
{
	int64_t logout_tick = 0;
	if (this->map_logout_tick_map_.find(role_id, logout_tick) == 0)
		return Time_Value(logout_tick);
	return Time_Value::zero;
}

Time_Value PlayerManager::ml_player_login_tick(const Int64 role_id)
{
	int64_t login_tick = 0;
	if (this->ml_login_tick_map_.find(role_id, login_tick) == 0)
		return Time_Value(login_tick);
	return Time_Value::zero;
}

Time_Value PlayerManager::ml_player_logout_tick(const Int64 role_id)
{
	int64_t logout_tick = 0;
	if (this->ml_logout_tick_map_.find(role_id, logout_tick) == 0)
		return Time_Value(logout_tick);
	return Time_Value::zero;
}

