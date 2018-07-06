/*
 * MapLogicPlayer.cpp
 *
 * Created on: 2013-04-23 11:20
 *     Author: lyz
 */

#include "ProtoDefine.h"
#include "MapLogicPlayer.h"
#include "MapTaskStruct.h"
#include "TaskImplement.h"
#include "MapMonitor.h"
#include "MLMounter.h"
#include "RankSystem.h"
#include "TransactionMonitor.h"
#include "GameField.h"
#include "MLBackDailyRechargeSys.h"
#include "MMORechargeRankConfig.h"

#include "Transaction.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "SerialRecord.h"
#include "MMOPackage.h"
#include "MMOBeast.h"
#include "MMOSkill.h"
#include "MMORole.h"
#include "MMOLabel.h"
#include "BackBrocast.h"
#include "MMOVip.h"
#include "MMOFight.h"
#include "MMOOnlineRewards.h"
#include "MMOOfflineRewards.h"
#include "MMOTreasures.h"
#include "MMOEquipSmelt.h"
#include "MMOInvestSystem.h"

#include "MMOCollectChests.h"
#include "MMOIllustration.h"
#include "MMORankPannel.h"
#include "MMOScriptClean.h"
#include "MMOPlayerTip.h"
#include "MMOMediaGift.h"
#include "MMOMailOffline.h"
#include "PubStruct.h"
#include "GameFont.h"
#include "LeagueMonitor.h"
#include "LogicGameSwitcherSys.h"
#include "MMOMagicWeapon.h"
#include "TipsEnum.h"
#include "MMOSwordPool.h"
#include "MapCommunicate.h"
#include "MMOOfflineHook.h"

MapLogicPlayer::CachedTimer::CachedTimer(void) : player_(0)
{ /*NULL*/ }

MapLogicPlayer::CachedTimer::~CachedTimer(void)
{ /*NULL*/ }

int MapLogicPlayer::CachedTimer::type(void)
{
    return GTT_ML_ONE_SECOND;
}

int MapLogicPlayer::CachedTimer::handle_timeout(const Time_Value &nowtime)
{
    this->player_->time_up(nowtime);
    return 0;
}

MapLogicPlayer::MapLogicPlayer(void)
{
    this->role_id_ = 0;
    this->gate_sid_ = 0;
    this->is_active_ = false;
    this->is_obtain_area_info_ = false;
    this->is_loading_mongo_ = false;
    this->load_mongo_tick_ = Time_Value::zero;
    this->count_of_zhuling_ = 0;
    this->repeat_req_recogn_ = 0;

    this->monitor_ = MAP_MONITOR;
    this->cached_timer_.player_ = this;
//    this->package_.serial_opt(&(this->serial_));
    this->is_synced_logic_info_ = false;
    this->is_synced_map_info_ = false;
    // 设置玩家内部定时功能的间隔时间
    double timeup_inter[] = {0,
        SAVE_TIMEOUT,        // 定时保存间隔
        GameEnum::DEFAULT_TIME_OUT,
        GameEnum::DEFAULT_TIME_OUT,		// TIMEUP_CHECK_FASHION
        GameEnum::DEFAULT_TIME_OUT,		// TIMEUP_CHECK_LABEL
        1,			// TIMEUP_CHECK_ONE_SECOND
        10,         // TIMEUP_CHECK_FRONTER
        10,         // TIMEUP_CHECK_RECHARGE
        10,			// TIMEUP_CHECK_TASK
        10,         // TIMEUP_CHECK_SCRIPT_COMPACT
        5			// TIMEUP_CHECK_TRANSFER
    };
    int timeup_inter_size = sizeof(timeup_inter) / sizeof(double);
    this->cache_tick_.init(MapLogicPlayer::CACHE_END, MapLogicPlayer::TIMEUP_END, timeup_inter, timeup_inter_size);
}

MapLogicPlayer::~MapLogicPlayer(void)
{
	this->pack_detail_.reset();
}

void MapLogicPlayer::reset(void)
{
    this->is_active_ = false;
    this->is_synced_logic_info_ = false;
    this->is_synced_map_info_ = false;
    this->is_obtain_area_info_ = false;

    this->is_loading_mongo_ = false;
    this->load_mongo_tick_ = Time_Value::zero;

    this->role_id_ = 0;
    this->gate_sid_ = 0;
    this->count_of_zhuling_ = 0;

    this->pack_detail_.reset();
    this->role_detail_.reset();
    this->role_ex_detail_.reset();

    this->serial_.reset();
    this->online_.reset();
    this->hook_detail_.reset();

    this->repeat_req_recogn_ = 0;
    this->repeat_req_.reset();

    EntityCommunicate::reset_entity();
    MLTinyer::reset_found();
    MapShoper::reset();
    MapLogicTeamer::reset();
    LogicOfflineHookPlayer::hook_reset();

    MLTreasures::treasures_info_reset();
    MLCollectChests::reset();
    MLMounter::reset();
    MLLeaguer::reset();
    MapMailPlayer::reset();
    MapLogicTasker::task_reset();
    MapEquipment::map_logic_equipment_reset();
    MLVipPlayer::reset();
    MapLogicLabel::reset();
    MapLogicIllustration::reset();
    MLAchievement::reset();
    MLOnlineRewards::reset();
    MLOfflineRewards::reset();
    MLCheckIn::reset();
    MLExpRestore::reset();
    MLScriptClean::reset_script_clean();
    PlayerAssist::reset_player_assist();
    MLMediaGiftPlayer::reset();
    MLOnceRewards::reset();
    MLRecharger::reset();
    MLDailyRecharge::reset();
    MLRebateRecharge::reset();
    MLInvestRecharge::reset();
    MLScriptCompact::reset_script_compact();
    MLTotalRecharge::reset();
    MLTrade::reset_trade();
    MLMagicWeapon::reset_magicweapon();
    MLSwordPool::reset();
    MLHiddenTreasure::reset();
    MLFashion::reset();
    MLTransfer::reset();

    this->add_pack_type(GameEnum::INDEX_EQUIP, GameEnum::EQUIP_MAX_INDEX);
    this->add_pack_type(GameEnum::INDEX_PACKAGE, GameEnum::PACK_MIN_INDEX);
    this->add_pack_type(GameEnum::INDEX_GODER, GameEnum::GODER_STORE_MAX_INDEX);
    this->add_pack_type(GameEnum::INDEX_STORE,  GameEnum::STORE_MIN_INDEX);
    this->add_pack_type(GameEnum::INDEX_BOX_STORE, GameEnum::BOX_STORE_MIN_INDEX);

	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE;  ++i)
	{
		int index = MLMounter::mount_to_index(i);
	    this->add_pack_type(index, GameEnum::MOUNT_STORE_MAX_INDEX);
	}

	this->caculate_all_mount_prop();
}

int MapLogicPlayer::add_pack_type(int pack_type, int pack_size)
{
	JUDGE_RETURN(pack_type > 0, -1);

	GamePackage* package = NULL;
	if (this->pack_detail_.__pack_map.count(pack_type) > 0)
	{
		package = this->pack_detail_.__pack_map[pack_type];
		package->reset();
	}
	else
	{
		package = POOL_MONITOR->game_pack_pool()->pop();
		this->pack_detail_.__pack_map[pack_type] = package;
	}

	package->set_owner(this);
	package->set_type(pack_type, pack_size);
	return 0;
}

GamePackage* MapLogicPlayer::find_package(int pack_type)
{
	PackageMap::iterator iter = this->pack_detail_.__pack_map.find(pack_type);
	JUDGE_RETURN(iter != this->pack_detail_.__pack_map.end(), NULL);
	return iter->second;
}

MapMonitor *MapLogicPlayer::monitor(void)
{
    return this->monitor_;
}

void MapLogicPlayer::set_role_id(const int64_t role_id)
{
    this->role_id_ = role_id;
}

int MapLogicPlayer::scene_id(void)
{
	return this->role_detail_.__scene_id;
}

int MapLogicPlayer::agent_code(void)
{
	return this->role_detail_.__agent_code;
}

int MapLogicPlayer::market_code(void)
{
	return this->role_detail_.__market_code;
}

Int64 MapLogicPlayer::role_id(void)
{
    return this->role_id_;
}

int MapLogicPlayer::role_career()
{
	return this->role_detail_.__career;
}

const char* MapLogicPlayer::name()
{
	return this->role_detail_.name();
}

string &MapLogicPlayer::role_name(void)
{
	return this->role_detail_.__name;
}

int MapLogicPlayer::role_level(void)
{
    return this->role_detail_.__level;
}

int MapLogicPlayer::kill_monster(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400014 *, request, -1);

    for (int i = 0; i < request->monster_set_size(); ++i)
    {
//    	const ProtoMonster &proto_monster = request->monster_set(i);
    	//this->task_listen_kill_monster(proto_monster.sort(), proto_monster.total_amount());

//    	this->daily_liveness_inner_event(proto_monster.sort(),
//    			LivenessFinishType::KILL, proto_monster.total_amount());
    }
    return 0;
}

int MapLogicPlayer::collect_item(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400015 *, request, -1);
	return this->task_listen_collect_item(request->item_id(), request->num());
}

int MapLogicPlayer::team_monster_info_update(Message* msg)
{
	JUDGE_RETURN(GameCommon::is_normal_scene(this->scene_id()), 0);
    MSG_DYNAMIC_CAST_RETURN(Proto31400051 *, request, -1);

    for (int i = 0; i < request->monster_set_size(); ++i)
    {
    	const ProtoMonster& proto = request->monster_set(i);
    	this->task_listen_kill_monster(proto.sort(), proto.total_amount());
    }

	return 0;
}

int MapLogicPlayer::task_collect_monster(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400022 *, request, -1);

    Proto31400051 sync_info;
    for (int i = 0; i < request->monster_set_size(); ++i)
    {
    	const ProtoMonster& proto = request->monster_set(i);
    	this->task_listen_kill_monster(proto.sort(), proto.total_amount());

    	ProtoMonster* monster_info = sync_info.add_monster_set();
    	monster_info->set_sort(proto.sort());
    	monster_info->set_total_amount(proto.total_amount());
    }

    return MAP_MONITOR->dispatch_to_logic(this, &sync_info);
}

int MapLogicPlayer::sync_death_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400016 *, request, -1);

	if (request->death_flag() == true)
	{
		this->role_detail_.__is_death = true;
		this->role_detail_.__death_tick = ::time(NULL);
	}
	else
	{
		this->role_detail_.__is_death = false;
		this->role_detail_.__death_tick = ::time(NULL);
	}

	return 0;
}

int MapLogicPlayer::sync_fight_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400019 *, request, -1);
	this->role_detail_.__fight_state = request->fight_state();
	return 0;
}

int MapLogicPlayer::sync_attend_activity(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400018 *, request, -1);

	int value = request->value();
	if (value <= 0)
		value = 1;
	this->task_listen_attend(request->activity_type(), request->sub_type(), value);

	return 0;
}

int MapLogicPlayer::sync_festival_icon(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400053 *, request, -1);

	this->role_detail_.__drop_act = request->drop_act();
	return 0;
}

bool MapLogicPlayer::logic_validate_festival_activity(int act_index)
{
	return this->role_detail_.__drop_act == act_index;
}

int MapLogicPlayer::sync_big_act_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400056 *, request, -1);

	this->role_detail_.__is_big_act_time = request->is_big_act_time();
	return 0;
}

bool MapLogicPlayer::logic_validate_big_act_time()
{
	return this->role_detail_.__is_big_act_time == true;
}

int64_t MapLogicPlayer::entity_id(void)
{
    return this->role_id_;
}

void MapLogicPlayer::set_gate_sid(const int gate_sid)
{
    this->gate_sid_ = gate_sid;
}

int MapLogicPlayer::gate_sid(void)
{
    return this->gate_sid_;
}

bool MapLogicPlayer::is_active(void)
{
    return this->is_active_;
}

bool MapLogicPlayer::is_enter_scene(void)
{
	return this->is_obtain_area_info_;
}

bool MapLogicPlayer::is_death(void)
{
	return (this->role_detail_.__is_death
			&& (this->role_detail_.__death_tick + 60 > Time_Value::gettimeofday().sec()));
}

bool MapLogicPlayer::is_fight_state(void)
{
	return (this->role_detail_.__fight_state != GameEnum::FIGHT_STATE_NO);
}

bool MapLogicPlayer::is_fight_active(void)
{
	return (this->role_detail_.__fight_state == GameEnum::FIGHT_STATE_ACTIVE);
}

bool MapLogicPlayer::is_fight_passive(void)
{
	return (this->role_detail_.__fight_state == GameEnum::FIGHT_STATE_PASSIVE);
}

bool MapLogicPlayer::is_need_save_mongo(void)
{
    return (this->is_loading_mongo_ == false && this->is_active_ == true);
}

void MapLogicPlayer::set_is_loading_mongo(const bool flag)
{
    this->is_loading_mongo_ = flag;
}

bool MapLogicPlayer::is_loading_mongo(void)
{
    return this->is_loading_mongo_;
}

void MapLogicPlayer::set_load_mongo_tick(const Time_Value &tick)
{
    this->load_mongo_tick_ = tick;
}

Time_Value &MapLogicPlayer::load_mongo_tick(void)
{
    return this->load_mongo_tick_;
}

int MapLogicPlayer::respond_to_client_error(const int recogn, const int error, const Message *msg_proto)
{
    if (msg_proto == 0)
        return this->monitor()->dispatch_to_client_from_gate(this, recogn, error);
    return this->monitor()->dispatch_to_client_from_gate(this, msg_proto, error);
}

int MapLogicPlayer::respond_to_client(Block_Buffer *buff)
{
    return this->monitor()->dispatch_to_client_from_gate(this, buff);
}

int MapLogicPlayer::request_load_data(const int gate_sid, const int64_t role_id)
{
    this->role_id_ = role_id;
    this->set_gate_sid(gate_sid);

    if (TRANSACTION_MONITOR->request_mongo_transaction(role_id, TRANS_LOAD_MAP_LOGIC_PLAYER,
                DB_MAP_LOGIC_PLAYER, this, this->monitor()->logic_player_pool(),
                this->monitor()->logic_unit(), gate_sid) != 0)
    {
        return -1;
    }
    return 0;
}

int MapLogicPlayer::sign_in(const int type)
{
    JUDGE_RETURN(this->monitor()->player_manager()->bind_logic_player(
    		this->role_id(), this) == 0, -1);

    MSG_USER("ml player sign in %ld %s %d %d", this->role_id(), this->name(), this->scene_id(), type);

    this->is_active_ = true;
    this->cached_timer_.schedule_timer(1);

    Time_Value nowtime = Time_Value::gettimeofday();
    for (int i = 0; i < TIMEUP_END; ++i)
    {
        this->cache_tick_.update_timeup_tick(i, nowtime);
    }

    if (type == ENTER_SCENE_LOGIN)
    {
    	this->login_sign_in();
    }
    else
    {
    	this->transfer_sign_in();
    }

    this->task_sign_in(type);
    this->player_sigin();

    return 0;
}

int MapLogicPlayer::sign_out(const int type, const bool is_save)
{
	MSG_USER("ml player signout %ld %s %d %d %d %d", this->role_id(), this->name(),
			this->role_level(), this->scene_id(), type, is_save);
	this->monitor()->player_manager()->unbind_logic_player(this->role_id());

	bool is_active = this->is_active();
    this->cached_timer_.cancel_timer();

    if (is_active == true && EXIT_SCENE_LOGOUT == type)
    {
    	this->online_.sign_out();
    }

    this->logout_online_rewards();
    this->logout_offline_rewards();

    this->task_sign_out();
    this->player_sigout();

    if (this->is_need_save_mongo() == true && is_save == true)
    {
        this->request_save_player(TRANS_LOGOUT_MAP_LOGIC_PLAYER);
    }

    this->is_active_ = false;
    this->is_obtain_area_info_ = false;
    this->monitor()->logic_player_pool()->push(this);

    return 0;
}

void MapLogicPlayer::login_sign_in()
{
	this->reset_every_day();
    this->online_.sign_in();
//    this->notify_system_setting_info();
    this->task_sign_in(0);
    this->login_offline_rewards();
    this->login_online_rewards();
    this->achieve_player_login();
    this->fashion_login_operate();
    this->transfer_login_operate();
    this->check_get_label_event();
    this->fetch_super_vip_info_begin();
    this->validate_open_sublime_jewel();
//    this->fetch_equip_sublime_jewel_info();

    this->refresh_player_equip_shape(GameEnum::EQUIP_WEAPON);
    this->refresh_player_equip_shape(GameEnum::EQUIP_YIFU);
}

void MapLogicPlayer::transfer_sign_in()
{
}

void MapLogicPlayer::reset_every_day(void)
{
	JUDGE_RETURN(this->role_detail_.__ml_day_reset_tick <= ::time(NULL), ;);

	this->role_detail_.inc_adjust_login_days();

    MLTinyer::reset_everyday();
    PlayerAssist::reset_everyday();
    MLInvestRecharge::reset_invest_everyday();

	// 每日充值
	this->daily_recharge_clean();
	// 环式任务
    this->task_sign_in(0);
    // 重置仙魂数据
    this->notify_godsoul_limit_info();

    this->treasures_info_reset();
	this->check_spool_day_reset();
	this->check_spirit_day_reset();
	this->hi_treasure_next_day();

	//离线挂机重置每日获得奖励最大值
	this->reset_every_day_offline();
	this->init_magicweapon_list();
    this->change_magic_weapon_status(GameEnum::RAMA_GET_FROM_ONLINE,
    		this->role_detail_.__login_days);

    this->login_reward_red_point();
    this->check_send_open_gift_mail();

    PackageDetail& pack_detail = this->pack_detail();
    pack_detail.use_resource_map_[GameEnum::ITEM_TODAY_UNBIND_GOLD] = 0;
    pack_detail.use_resource_map_[GameEnum::ITEM_TODAY_BIND_GOLD] = 0;

	MSG_USER("map logic %ld %s next: ld days:%ld", this->role_id(), this->name(),
			this->role_detail_.__ml_day_reset_tick,	this->role_detail_.__login_days);
	this->role_detail_.__ml_day_reset_tick = ::next_day(0,0).sec();
}

int MapLogicPlayer::request_save_player(const int recogn)
{
    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_VIP) == true)
    {
    	MMOVip::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_PACKAGE) == true)
    {
        MMOPackage::update_data(this, data_map);
        MMORole::update_back_role(this, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_MAIL) == true)
    {
        MMOMail::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(CACHE_LABEL) == true)
    {
    	MMOLabel::update_data(this, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(CACHE_WELFARE) == true)
    {
    	MMOWelfare::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(CACHE_ILLUSTRATION) == true)
    {
    	MMOIllustration::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_ONLINE_REWARDS) == true)
	{
    	MMOOnlineRewards::update_data(this, data_map);
	}
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_TREASURES_INFO) == true)
	{
    	MMOTreasures::update_data(this, data_map);
	}
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_OFFLINE_REWARDS) == true)
	{
    	MMOOfflineRewards::update_data(this, data_map);
	}
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_EQUIP_SMELT) == true)
	{
    	MMOEquipSmelt::update_data(this, data_map);
	}
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_COLLECT_CHESTS) == true)
	{
    	MMOCollectChests::update_data(this, data_map);
	}
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_EXP_RESTORE) == true)
    {
    	MMOExpRestore::update_data(this, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_ROLE) == true)
    {
    	MMORole::update_data(this, data_map);
    	MMORoleEx::update_data(this, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_SCRIPT_CLEAN) == true)
    {
        MMOScriptClean::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_MEDIA_GIFT) == true)
    {
        MMOMediaGift::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_INVEST) == true)
    {
    	MMOInvestSystem::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_TASK) == true)
    {
    	MMOTask::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS) == true)
    {
        MMORechargeRewards::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_SWORD_POOL) == true)
    {
        MMOSwordPool::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_FASHION_INFO) == true)
    {
    	MMOFashion::update_data(this, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
            || this->cache_tick().check_cache(MapLogicPlayer::CACHE_TRANSFER_INFO) == true)
    {
        MMOTransfer::update_data(this, data_map);
    }

	if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
			|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON) == true)
	{
		MMOMagicWeapon::update_data(this, data_map);
	}

	if (recogn == TRANS_LOGOUT_MAP_LOGIC_PLAYER
			|| this->cache_tick().check_cache(MapLogicPlayer::CACHE_MOUNT) == true)
	{
		 MMOBeast::save_master(this, data_map);
	}

    switch (recogn)
    {
    case TRANS_LOGOUT_MAP_LOGIC_PLAYER:
    {
        MMOAchievement::update_data(this, data_map);
        MMOHiddenTreasure::update_data(this, data_map);
        MMOBeast::save_master(this, data_map);
    	BackSerial::update_data(this, data_map);
        MMORankPannel::update_date(this, data_map);
    	MMOPlayerTip::update_data(this, data_map);
    	MMOFight::update_tiny_data(this, data_map);
    	MMOOfflineHook::update_data(this, data_map);
    	break;
    }
    }

    if (data_map->data_map().size() <= 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
        this->cache_tick().reset_cache_flag();
        return 0;
    }

    this->cache_tick().reset_cache_flag();

#ifdef LOCAL_DEBUG
    if (GameCommon::is_travel_scene(this->scene_id()))
#else
    if (this->monitor()->is_has_travel_scene() == true)
#endif
    {
        this->process_travel_save(recogn, data_map);
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
    }
    else
    {
        if (TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), recogn, data_map) != 0)
        {
            POOL_MONITOR->mongo_data_map_pool()->push(data_map);
            return -1;
        }
    }

    return 0;
}

int MapLogicPlayer::process_travel_save(const int recogn, MongoDataMap *data_map)
{
    Proto30400054 inner_req;
    inner_req.set_mongo_recogn(recogn);
    inner_req.set_role_id(this->role_id());

    MongoDataMap::DataMap &mongo_data_map = data_map->data_map();
    for (MongoDataMap::DataMap::iterator iter = mongo_data_map.begin();
            iter != mongo_data_map.end(); ++iter)
    {
        MongoData *mongo_data = iter->second;

        ProtoMongoData *proto_data = inner_req.add_mongo_data();
        mongo_data->serialize(proto_data);
    }

    return this->monitor()->dispatch_to_scene(this, SCENE_GATE, &inner_req);
}

int MapLogicPlayer::transfer_scene(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400101*, request, -1);

	if (this->is_active() == true)
	{
		this->request_save_player(TRANS_LOGOUT_MAP_LOGIC_PLAYER);
	}

	int scene_id = request->scene_id();
	int prev_scene = request->prev_scene();

    this->sync_transfer_base(scene_id);
    this->sync_transfer_package(scene_id);
    this->sync_transfer_online(scene_id);
    this->sync_transfer_spool(scene_id);
    this->sync_transfer_mounter(scene_id);
    this->sync_transfer_tiny(scene_id);
    this->sync_transfer_illus_info(scene_id);
    this->sync_transfer_check_in(scene_id);
    this->sync_transfer_once_rewards(scene_id);
    
    this->sync_transfer_vip(scene_id);
    this->sync_transfer_online_rewards(scene_id);
    this->sync_transfer_offline_rewards(scene_id);
    this->sync_transfer_treasures_info(scene_id);
    this->sync_transfer_collect_chests(scene_id);
    this->sync_transfer_exp_restore(scene_id);
    this->sync_transfer_task(scene_id);
    this->sync_transfer_achieve(scene_id);
    this->sync_transfer_label_info(scene_id);
    this->sync_transfer_equipment_info(scene_id);
    this->sync_transfer_mail_box_info(scene_id);
    this->sync_transfer_tip_pannel(scene_id);
    this->sync_transfer_media_gift(scene_id);
    this->sync_transfer_recharge_rewards(scene_id);
    this->sync_transfer_script_clean(scene_id);
    this->sync_transfer_daily_recharge(scene_id);
    this->sync_transfer_rebate_recharge(scene_id);
    this->sync_transfer_invest_recharge(scene_id);
    this->sync_transfer_total_recharge(scene_id);
    this->sync_transfer_magic_weapon_info(scene_id);
    this->sync_transfer_fashion(scene_id);
    this->sync_transfer_trans(scene_id);
    this->sync_transfer_hi_treasure(scene_id);
    this->hook_sync_transfer_scene(scene_id);

    this->finish_sync_transfer_scene(scene_id, prev_scene);
    this->send_to_map_thread(INNER_TRANSFER_LOGIC_INFO_END);

    //设置为false, 不需要再次保存数据库
    this->is_active_ = false;
    PLAYER_MANAGER->process_ml_player_logout(this->gate_sid(),
    		this->role_id(), EXIT_SCENE_TRANSFER);

    return 0;
}

int MapLogicPlayer::serialize_base(Proto31400103 *request)
{
	MapLogicRoleDetail& detail = this->role_detail();
    request->set_role_id(this->role_id());
	request->set_role_name(detail.__name);
	request->set_account(detail.__account);
	request->set_agent(detail.__agent);
    request->set_agent_code(detail.__agent_code);

    request->set_sex(detail.__sex);
    request->set_career(detail.__career);
    request->set_level(detail.__level);
    request->set_force(detail.__fight_force);
    request->set_is_death(detail.__is_death);
    request->set_death_tick(detail.__death_tick);
    request->set_fight_state(detail.__fight_state);
    request->set_create_tick(detail.create_info_.create_tick_);
    request->set_league_id(detail.__league_id);
    request->set_login_day(detail.__login_days);
    request->set_login_tick(detail.__login_tick);
    request->set_open_gift_close(detail.__open_gift_close);
    request->set_drop_act(detail.__drop_act);
    request->set_combine_tick(detail.combine_tick_);
    request->set_is_big_act_time(detail.__is_big_act_time);

    request->set_partner_id(detail.__partner_id);
    request->set_partner_name(detail.__partner_name);
    request->set_wedding_id(detail.__wedding_id);
    request->set_wedding_type(detail.__wedding_type);
    request->set_day_reset_tick(detail.__ml_day_reset_tick);

    for (IntMap::iterator iter = detail.draw_days_.begin();
    		iter != detail.draw_days_.end(); ++iter)
    {
    	request->add_draw_days(iter->first);
    }

    for (IntMap::iterator iter = detail.draw_gift_.begin();
        	iter != detail.draw_gift_.end(); ++iter)
    {
        request->add_draw_gift(iter->first);
    }

    for (IntMap::iterator iter = detail.draw_vips_.begin();
    		iter != detail.draw_vips_.end(); ++iter)
    {
        request->add_draw_vips(iter->first);
    }

    for (int type = WED_RING; type <= WED_TREE; ++type)
    {
    	int level = this->role_detail_.wedding_self_[type];
    	int side_level = this->role_detail_.wedding_side_[type];
    	request->add_wedding_self(level);
    	request->add_wedding_side(side_level);
    }

    GameCommon::map_to_proto(request->mutable_rand_use_times(), detail.rand_use_times_);

    return 0;
}

int MapLogicPlayer::unserialize_base(Proto31400103 *request)
{
	MapLogicRoleDetail& detail = this->role_detail();
	detail.__name = request->role_name();
	detail.__account = request->account();
	detail.__agent = request->agent();

    this->role_id_ = request->role_id();
    detail.__agent_code = request->agent_code();
    detail.__sex = request->sex();
    detail.__career = request->career();
    detail.__level = request->level();
    detail.__fight_force = request->force();
    detail.__scene_id = request->scene_id();
    detail.__is_death = request->is_death();
    detail.__death_tick = request->death_tick();
    detail.__fight_state = request->fight_state();
    detail.set_create_tick(request->create_tick());
    detail.__league_id = request->league_id();
    detail.__login_days = request->login_day();
    detail.__login_tick = request->login_tick();
    detail.__open_gift_close = request->open_gift_close();
    detail.__drop_act = request->drop_act();
    detail.combine_tick_ = request->combine_tick();
    detail.__is_big_act_time = request->is_big_act_time();

    detail.__partner_id = request->partner_id();
    detail.__partner_name = request->partner_name();
    detail.__wedding_id = request->wedding_id();
    detail.__wedding_type = request->wedding_type();
    detail.__ml_day_reset_tick = request->day_reset_tick();

    for (int i = 0; i < request->draw_days_size(); ++i)
    {
    	int draw_days = request->draw_days(i);
    	detail.draw_days_[draw_days] = true;
    }

    for (int i = 0; i < request->draw_gift_size(); ++i)
    {
        int location_id = request->draw_gift(i);
        detail.draw_gift_[location_id] = true;
    }

    for (int i = 0; i < request->draw_vips_size(); ++i)
    {
       	int draw_vips = request->draw_vips(i);
       	detail.draw_vips_[draw_vips] = true;
     }

    for (int i = 0; i < WED_TREE; ++i)
    {
    	int level = 0;
    	int side_level = 0;

    	if (i < request->wedding_self_size())
		{
    		level = request->wedding_self(i);
		}

    	if (i < request->wedding_side_size())
    	{
    		side_level = request->wedding_side(i);
    	}

    	int type = i + WED_RING;
    	detail.wedding_self_[type] = level;
    	detail.wedding_side_[type] = side_level;
    }

    GameCommon::proto_to_map(detail.rand_use_times_, request->rand_use_times());

    return 0;
}

int MapLogicPlayer::serialize_online(Proto31400105 *request)
{
    LogicOnline::OnlineDetail &online_detail = this->online_.online_detail();
    request->set_role_id(this->role_id());
    request->set_sign_in_tick(online_detail.__sign_in_tick);
    request->set_sign_out_tick(online_detail.__sign_out_tick);

    request->set_total_online_tick(online_detail.__total_online_tick);
    request->set_day_online_tick(online_detail.__day_online_tick);
    request->set_week_online_tick(online_detail.__week_online_tick);
    request->set_month_online_tick(online_detail.__month_online_tick);
    request->set_year_online_tick(online_detail.__year_online_tick);

    request->set_day_refresh_tick(online_detail.__day_refresh_tick.sec());
    request->set_week_refresh_tick(online_detail.__week_refresh_tick.sec());
    request->set_month_refresh_tick(online_detail.__month_refresh_tick.sec());
    request->set_year_refresh_tick(online_detail.__year_refresh_tick.sec());
    return 0;
}

int MapLogicPlayer::unserialize_online(Proto31400105 *request)
{
    LogicOnline::OnlineDetail &online_detail = this->online_.online_detail();
    online_detail.__sign_in_tick = request->sign_in_tick();
    online_detail.__sign_out_tick = request->sign_out_tick();

    online_detail.__total_online_tick = request->total_online_tick();
    online_detail.__day_online_tick = request->day_online_tick();
    online_detail.__week_online_tick = request->week_online_tick();
    online_detail.__month_online_tick = request->month_online_tick();
    online_detail.__year_online_tick = request->year_online_tick();

    online_detail.__day_refresh_tick = Time_Value(request->day_refresh_tick());
    online_detail.__week_refresh_tick = Time_Value(request->week_refresh_tick());
    online_detail.__month_refresh_tick = Time_Value(request->month_refresh_tick());
    online_detail.__year_refresh_tick = Time_Value(request->year_refresh_tick());
    return 0;
}

int MapLogicPlayer::sync_info_before_enter_scene(int finish_type)
{
	//this->vip_sign_in();
	this->notify_sync_vip_info(false);
    this->sync_fight_property_to_map(ENTER_SCENE_TRANSFER);

	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		this->notify_mount_shape(false, i);
	    this->refresh_mount_property(i, ENTER_SCENE_TRANSFER);
	}

	this->notify_spool_style_lvl(false);
	this->notify_fashion_style(false);
	this->send_transfer_to_map(TransferDetail::SYNC_TRANSFER, false);
    this->refresh_spool_attr_add(ENTER_SCENE_TRANSFER);
    this->refresh_fashion_attr_add(ENTER_SCENE_TRANSFER);
    this->refresh_transfer_attr_add(ENTER_SCENE_TRANSFER);
    this->refresh_achieve_attr_add(ENTER_SCENE_TRANSFER);
    this->refresh_magic_weapon_property(ENTER_SCENE_TRANSFER);
    this->sync_magic_weapon_rank_lvlup();

	Proto30400006 sync_info;
	sync_info.set_finish_type(finish_type);
	return this->send_to_map_thread(sync_info);
}

int MapLogicPlayer::map_obtain_area_info_done(void)
{
    this->is_obtain_area_info_ = true;

	this->check_in_pa_event();
	this->check_smelt_pa_event();
    this->check_label_pa_event();
//    this->notify_system_setting_info();

    int scene_id = this->scene_id();
    this->task_listen_enter_special_scene(scene_id);
    this->start_script_wave_task_listen(scene_id);
	this->check_and_update_exp_restore_info();

	MapLogicPlayer::notify_player_welfare_status(this);

	this->transfer_login_obtain();
	this->notify_client_popup_info();
	this->notify_script_clean_doing();
//	this->notify_finished_task_set();
    this->notify_daily_recharge_info();

//    if (this->is_script_compact_status())
//    {
//		this->sync_script_compact_info();
//		this->notify_script_compact_info();
//    }

    this->notify_total_recharge_info();
    this->notify_client_tick();

	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		this->fetch_mount_info(i);
	}

	this->check_out_time_item();
	this->check_and_adjust_vip();
	this->adjust_level_by_task();
	return 0;
}

int MapLogicPlayer::check_out_time_item()
{
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, 0);

	ItemVector tips_vec;
	ItemVector remove_vec;
	for (ItemListMap::iterator iter = package->item_list_map_.begin();
				iter != package->item_list_map_.end(); ++iter)
	{
	    PackageItem* pack_item = iter->second;
	    JUDGE_CONTINUE(pack_item != NULL);
	    JUDGE_CONTINUE(pack_item->time_item());

	    if (pack_item->expire_item() == true)
	    {
		    remove_vec.push_back(pack_item);
	    }
	    else
	    {
	    	tips_vec.push_back(pack_item);
	    }
	}

	//过期返还
	for (ItemVector::iterator iter = remove_vec.begin(); iter != remove_vec.end(); ++iter)
	{
		PackageItem* pack_item = *iter;
		JUDGE_CONTINUE(pack_item != NULL);

		const Json::Value& conf = pack_item->conf();
		this->pack_remove(package, ITEM_REMOVE_TIMEOUT, pack_item);
		JUDGE_CONTINUE(conf.isMember("out_time_item") == true);

		int mail_id = CONFIG_INSTANCE->const_set("out_time_item_mail_id");
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str(), conf["name"].asCString());

		mail_info->add_goods(GameCommon::make_up_itemobj(conf["out_time_item"]));
		GameCommon::request_save_mail_content(this->role_id(), mail_info);
	}

	Proto81400113 respond;
	respond.set_type(2);
	//到期提示
	for (ItemVector::iterator iter = tips_vec.begin(); iter != tips_vec.end(); ++iter)
	{
		PackageItem* pack_item = *iter;
		int max_time = 0, min_time = 0;
		for (int i = 0; i < (int)pack_item->__tips_time_map.size(); ++i)
		{
			max_time = pack_item->__tips_time_map[i];
			if (i == (int) pack_item->__tips_time_map.size() - 1)
				min_time = 0;
			else
				min_time = pack_item->__tips_time_map[i + 1];

			int left_time = GameCommon::left_time(pack_item->__timeout);
			if (left_time >= min_time && left_time <= max_time && pack_item->__tips_status_map[i] == 1)
			{
				ProtoItem* temp = respond.add_item();
				pack_item->serialize(temp);
				pack_item->__tips_status_map[i] = 2;
				break;
			}
		}
	}

	JUDGE_RETURN(respond.item_size() > 0, 0);
	FINER_PROCESS_RETURN(ACTIVE_ITEM_TIPS_INFO, &respond);
}

int MapLogicPlayer::sync_exp_restore_event_done(int event_id, Time_Value time_v)
{
	const Json::Value &request_event_json = CONFIG_INSTANCE->exp_restore_json(event_id);
	JUDGE_RETURN(Json::Value::null != request_event_json, -1);
	int storage_id = request_event_json["storage_id"].asInt();

	if(time_v == Time_Value::zero)
		time_v = Time_Value::gettimeofday();

	return this->storage_finish_times_reduce(storage_id, time_v);
}

int MapLogicPlayer::open_package(void)
{
//	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
//	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_PACK_OPEN, ERROR_CLIENT_OPERATE);
//
//	Proto51400112 respond;
//	respond.set_force(this->role_detail_.__fight_force);
//
//	for (ItemListMap::iterator iter = package->item_list_map_.begin();
//			iter != package->item_list_map_.end(); ++iter)
//	{
//		PackageItem* pack_item = iter->second;
//		JUDGE_CONTINUE(pack_item != NULL);
//
//		ProtoItem* proto_item = respond.add_equip_list();
//		pack_item->serialize(proto_item);
//	}
//
//	FINER_PROCESS_RETURN(RETURN_PACK_OPEN, &respond);
	return 0;
}

int MapLogicPlayer::serialize_package(ProtoPackageSet* msg_proto)
{
    ProtoMoney *money = msg_proto->mutable_money();
    this->pack_detail_.__money.serialize(money);

    for (IntMap::iterator iter = this->pack_detail_.resource_map_.begin();
    		iter != this->pack_detail_.resource_map_.end(); ++iter)
    {
    	JUDGE_CONTINUE(iter->first > 0 && iter->second > 0);

    	ProtoPairObj* pair = msg_proto->add_resource_map();
    	pair->set_obj_id(iter->first);
    	pair->set_obj_value(iter->second);
    }

    for (PackageMap::iterator iter = this->pack_detail_.__pack_map.begin();
    		iter != this->pack_detail_.__pack_map.end(); ++iter)
    {
        ProtoPackage* package = msg_proto->add_package_list();
        iter->second->serialize(package);
    }

    GameCommon::map_to_proto(msg_proto->mutable_use_resource_map(),
    		this->pack_detail_.use_resource_map_);
	return 0;
}

int MapLogicPlayer::unserialize_package(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400104*, request, -1);

	ProtoPackageSet* msg_proto = request->mutable_package_set();
	this->pack_detail_.__money.unserialize(msg_proto->money());

	for (int i = 0; i < msg_proto->resource_map_size(); ++i)
	{
		const ProtoPairObj& pair = msg_proto->resource_map(i);
		this->pack_detail_.resource_map_[pair.obj_id()] = pair.obj_value();
	}

	int package_set_size = msg_proto->package_list_size();
	for (int i = 0; i < package_set_size; ++i)
	{
		const ProtoPackage& proto_package = msg_proto->package_list(i);
		this->add_pack_type(proto_package.pack_type(), proto_package.pack_size());

		GamePackage* package = this->find_package(proto_package.pack_type());
		JUDGE_CONTINUE(package != NULL);

		package->unserialize(proto_package);
	}

	GameCommon::proto_to_map(this->pack_detail_.use_resource_map_,
			msg_proto->use_resource_map());

    this->calc_equip_property();

	return 0;
}

int MapLogicPlayer::sync_transfer_base(const int scene_id)
{
    Proto31400103 request;
    this->serialize_base(&request);

    request.set_scene_id(scene_id);
    request.set_transfer_tick(::time(NULL));

    return this->send_to_other_logic_thread(scene_id, request);
}

int MapLogicPlayer::sync_transfer_package(const int scene_id)
{
    Proto31400104 request;
    request.set_role_id(this->role_id());

    ProtoPackageSet* pack_set = request.mutable_package_set();
    this->serialize_package(pack_set);

    return this->send_to_other_logic_thread(scene_id, request);
}

int MapLogicPlayer::sync_transfer_online(const int scene_id)
{
    Proto31400105 request;
    this->serialize_online(&request);
    return this->send_to_other_logic_thread(scene_id, request);
}

int MapLogicPlayer::sync_transfer_task(const int scene_id)
{
    Proto31400108 request;
    this->serialize_task(&request);
    return this->monitor()->dispatch_to_scene(this, scene_id, &request);
}

int MapLogicPlayer::finish_sync_transfer_scene(int scene_id, int prev_scene)
{
    Proto31400102 request;
    request.set_prev_scene(prev_scene);
    request.set_role_id(this->role_id());
    return this->send_to_other_logic_thread(scene_id, request);
}

int MapLogicPlayer::time_up(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->is_active() == true, 0);
	JUDGE_RETURN(this->is_enter_scene() == true, 0);

	if (this->cache_tick_.check_timeout(MapLogicPlayer::TUMEUP_CHECK_TRANSFER, nowtime) == true)
	{
		this->check_handle_transfer_timeout();
		this->cache_tick_.update_timeup_tick(MapLogicPlayer::TUMEUP_CHECK_TRANSFER, nowtime);
	}

#ifndef LOCAL_DEBUG
	if (this->monitor()->is_has_travel_scene() == true)
	{
//		Time_Value login_tick = this->monitor()->player_manager()->ml_player_login_tick(this->role_id());
//		Time_Value logout_tick = this->monitor()->player_manager()->ml_player_logout_tick(this->role_id());
//		if (login_tick <= logout_tick)
		return 0;
	}
#endif

	if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_LOAD_MAIL, nowtime) == true)
	{
		this->check_out_time_item();
		this->mail_time_up(nowtime);
		this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_LOAD_MAIL, nowtime);
	}

	if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_CHECK_FASHION, nowtime) == true)
	{
		this->check_handle_fashion_timeout();
		this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_CHECK_FASHION, nowtime);
	}

	if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_CHECK_RECHARGE, nowtime) == true)
	{
		this->player_recharge_timeup();
		this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_CHECK_RECHARGE, nowtime);
	}

	if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_CHECK_LABEL, nowtime) == true)
	{
		this->label_time_up(nowtime);
		this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_CHECK_LABEL, nowtime);
	}
	if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_CHECK_ONE_SECOND, nowtime) == true)
	{
		this->check_handle_mount_bless_timeout();
		this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_CHECK_ONE_SECOND, nowtime);
	}
    if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_CHECK_TASK, nowtime) == true)
    {
        MapLogicTasker::task_time_up(nowtime);
        this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_CHECK_TASK, nowtime);
    }
    if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_CHECK_SCRIPT_COMPACT, nowtime) == true)
    {
        MLScriptCompact::script_compact_time_up(nowtime);
        this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_CHECK_SCRIPT_COMPACT, nowtime);
    }

	if (this->cache_tick_.check_timeout(MapLogicPlayer::TIMEUP_SAVE, nowtime) == true)
	{
		if (this->request_save_player() == 0)
		{
			this->cache_tick().reset_cache_flag();
		}
		this->cache_tick_.update_timeup_tick(MapLogicPlayer::TIMEUP_SAVE, nowtime);
	}

	return 0;
}

GameCache &MapLogicPlayer::cache_tick(void)
{
    return this->cache_tick_;
}

MapLogicRoleDetail &MapLogicPlayer::role_detail(void)
{
    return this->role_detail_;
}

RoleExDetail &MapLogicPlayer::role_ex_detail(void)
{
    return this->role_ex_detail_;
}

PackageDetail& MapLogicPlayer::pack_detail(void)
{
	return this->pack_detail_;
}

LogicOnline &MapLogicPlayer::online(void)
{
    return this->online_;
}

MapLogicSerial &MapLogicPlayer::serial_record(void)
{
    return this->serial_;
}

HookDetail &MapLogicPlayer::hook_detail(void)
{
	return this->hook_detail_;
}

int MapLogicPlayer::notify_client_popup_info()
{
	Proto81401405 popup_info;
	popup_info.set_check_in_popup(this->check_need_popup() | this->total_check_need_popup());

//	int active_num = this->active_found_num();
//	int draw_num = 0;
//	if(active_num > 0)
//	{
//		draw_num = this->is_found_can_draw();
//	}
//	popup_info.set_found_num(active_num);
//	popup_info.set_found_flag(draw_num);

	return MAP_MONITOR->dispatch_to_logic(this, &popup_info);
}

int MapLogicPlayer::test_midnight_refresh()
{
	PlayerManager::LogicPlayerSet logic_player_set;
	MAP_MONITOR->get_logic_player_set(logic_player_set);

	MapLogicPlayer *player;
	for(PlayerManager::LogicPlayerSet::iterator it = logic_player_set.begin();
			it != logic_player_set.end(); it++)
	{
		player = *it;
		player->check_and_update_exp_restore_info();
		player->exp_restore_notify_reflash();
		MapLogicPlayer::notify_player_welfare_status(player); // 这个调用会自动刷新签到信息

		// 在线奖励
//		player->logout_online_rewards();
		player->reset_today_stage();
		player->login_online_rewards();

		player->reset_every_day();
	}

	return 0;
}

int MapLogicPlayer::logic_test_command(Message* msg)
{
#ifdef TEST_COMMAND
	JUDGE_RETURN(CONFIG_INSTANCE->const_set("test_command") == 1, -1);
	MSG_DYNAMIC_CAST_NOTIFY(Proto11499999*, request, RETURN_ML_TEST_COMMAND);

	if (request->cmd_name() == "get")
	{
		int bind_staus = GameEnum::ITEM_NO_BIND;
		if(request->param3() == GameEnum::ITEM_BIND)
		{
			bind_staus = GameEnum::ITEM_BIND;
		}
		return this->insert_package(ADD_FROM_LOCAL_TEST, request->param1(),
				request->param2(), bind_staus);
	}
	else if (request->cmd_name() == "mount_recharge")
	{
		this->update_mount_open_recharge_act(request->param1());
		return 0;
	}
	else if (request->cmd_name() == "clear_red_point")
	{
		this->assist_tip_pannel().clear();
		return 0;
	}
	else if (request->cmd_name() == "kill_socket")
	{
		Svc* svc = MAP_MONITOR->inner_receiver()->find_svc(this->gate_sid());
		if (svc != NULL)
		{
			MSG_USER("kill socket");
			svc->handle_close();
		}
		else
		{
			MSG_USER("kill socket svc is NULL");
		}
		return 0;
	}
	else if (request->cmd_name() == "all_reward")
	{
		return this->add_reward(request->param1(), ADD_FROM_LOCAL_TEST, true);
	}
	else if (request->cmd_name() == "use")
	{
		PackageItem *pack_item = this->pack_find_by_id(request->param1());
		JUDGE_RETURN(pack_item != NULL, -1);

		Proto11400106 inner_req;
		inner_req.set_index(pack_item->__index);
		inner_req.set_amount(request->param2());
		return this->use_pack_goods(&inner_req);
	}
	else if (request->cmd_name() == "task_open_all")
	{
		return this->test_task_open_ui(request->param1());
	}
	else if (request->cmd_name() == "drop")
	{
		Proto11400107 req;
		req.set_index(request->param1());
		req.set_pack_type(request->param2());
		return this->drop_goods(&req);
	}
	else if (request->cmd_name() == "midnight_refresh")
	{
		return this->test_midnight_refresh();
	}
	else if (request->cmd_name() == "money")
	{
		Money money(request->param1(), request->param1(),
				request->param1(), request->param1());
		this->pack_money_set(money);
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);
		return 0;
	}
	else if (request->cmd_name() == "exploit")
	{
		return this->add_game_resource(GameEnum::ITEM_ID_EXPLOIT,
				request->param1(), ADD_FROM_LOCAL_TEST);
	}
	else if (request->cmd_name() == "invest_time")
	{
		this->set_invest_test_time(request->param1());
		this->reset_invest_everyday();
		return 0;
	}
	else if (request->cmd_name() == "league_contri")
	{
		return this->dispatch_to_logic_league(ADD_FROM_LOCAL_TEST,
				request->param1());
	}
    else if (request->cmd_name() == "reputation")
    {
    	return this->add_game_resource(GameEnum::ITEM_ID_REPUTATION,
    			request->param1(),ADD_FROM_LOCAL_TEST);
    }
    else if (request->cmd_name() == "honor")
    {
    	return this->add_game_resource(GameEnum::ITEM_ID_HONOUR,
    			request->param1(), ADD_FROM_LOCAL_TEST);
    }
    else if (request->cmd_name() == "practice")
    {
    	return this->add_game_resource(GameEnum::ITEM_ID_PRACTICE,
    			request->param1(), ADD_FROM_LOCAL_TEST);
    }
    else if (request->cmd_name() == "mount_open")
    {
    	this->check_task_open_mount(request->param1(), 0);
    	return 0;
    }
    else if (request->cmd_name().compare("clean_script") == 0)
    {
		Proto11401202 req;
		req.set_script_sort(request->param1());
		req.set_script_type(request->param2());
		return this->request_start_clean_single_script(&req);
	}
    else if (request->cmd_name().compare("clean_type") == 0)
    {
		Proto11401202 req;
		req.set_script_type(request->param1());
		return this->request_start_clean_single_script(&req);
	}
	else if (request->cmd_name() == "test_treasures")
	{
		return this->treasures_info_reset();
	}
	else if (request->cmd_name() == "test_mail")
	{
		return this->send_test_mail();
	}
	else if (request->cmd_name() == "test_online")
	{
		return this->test_online();
	}
	else if (request->cmd_name() == "test_label")
	{
		this->insert_label(1000001);
		this->select_label(NULL);
	}
	else if (request->cmd_name() == "test_chests_logic")
	{
		this->get_chests_time_info().cur_state_ = GameEnum::ACTIVITY_STATE_AHEAD;
		this->handle_chests_timeout(this->get_chests_cycle_id());
	}
	else if (request->cmd_name() == "test_rama")
	{
		this->player_get_reiki(NULL);
	}
	else if (request->cmd_name() == "recharge_today_clear")
	{
		this->daily_recharge_dtail().__last_recharge_time = 0;
		this->daily_recharge_dtail().__today_recharge_gold = 0;
	}
	else if (request->cmd_name() == "save_recharge")
	{
		BackDR_SYS->stop();
	}
	else if (request->cmd_name() == "max_illus")
	{
		this->test_max_illus(request->param1());
	}
	else if (request->cmd_name() == "max_rama")
	{
		this->test_max_rama();
	}
	else if (request->cmd_name() == "sword_pool")
	{
		this->get_spool_info();
	}
	else if (request->cmd_name() == "uplevel_sword")
	{
		this->uplevel_sword_pool();
	}
	else if (request->cmd_name() == "find_one")
	{
		Proto11406003 respond;
		respond.set_task_id(request->param1());
		this->find_back_task_for_one(&respond);
	}
	else if (request->cmd_name() == "find_all")
	{
		this->find_back_all_task();
	}
	else if (request->cmd_name() == "test_spool")
	{
		this->test_find_task();
	}
	else if (request->cmd_name() == "reset_spool")
	{
		this->test_spool_day_reset();
	}
	else if (request->cmd_name() == "fashion_info")
	{
		this->request_fetch_fashion_info();
	}
	else if (request->cmd_name() == "spirit_info")
	{
		this->request_spirit_info();
	}
	else if (request->cmd_name() == "make_spirit")
	{
		Proto11403002 respond;
		respond.set_type(request->param1());
		this->request_make_spirit(&respond);
	}
	else if (request->cmd_name() == "up_stage")
	{
		this->request_up_stage();
	}
	else if (request->cmd_name() == "transfer_info")
	{
		this->request_transfer_info();
	}
	else if (request->cmd_name() == "change_transfer")
	{
		Proto11403005 respond;
		respond.set_transfer_id(request->param1());
		this->request_change_transfer_id(&respond);
	}
	else if (request->cmd_name() == "use_transfer")
	{
		this->request_use_transfer();
	}
	else if (request->cmd_name() == "buy_transfer")
	{
		Proto11403008 respond;
		respond.set_transfer_id(request->param1());
		this->request_buy_transfer(&respond);
	}
	else if (request->cmd_name() == "reset_transfer_cd")
	{
		this->test_reset_transfer_cd();
	}
	else if (request->cmd_name() == "seven_day")
	{
		this->fetch_login_reward_info();
	}
	else if (request->cmd_name() == "get_seven")
	{
		Proto11400118 respond;
		respond.set_draw_day(request->param1());
		this->draw_login_reward(&respond);
	}
	else if (request->cmd_name() == "hi_treasure")
	{
		this->fetch_hi_treasure_info();
	}
	else if (request->cmd_name() == "reset_hi_treasure")
	{
		this->test_hi_treasure_reset();
	}
	else if (request->cmd_name() == "hi_treasure_set_day")
	{
		this->test_set_hi_treasure_day(request->param1());
	}
	else if (request->cmd_name() == "test_smelt")
	{
		Message* temp = NULL;
		Proto11400662 text;
		text.set_status(0);
		this->change_smelt_recommend(&text);
		this->fetch_equip_smelt_panel_info();
		return this->equip_smelt(temp);
	}
	else if (request->cmd_name() == "smelt_exp")
	{
		this->equip_smelt(request->param1());
	}
	else if (request->cmd_name() == "test_illus")
	{
		 this->test_illus();
		 this->cache_tick().update_cache(MapLogicPlayer::CACHE_ILLUSTRATION);
		 MSG_USER("test2 work %d /n",this->role_id_);
		 this->request_save_player(TRANS_LOGOUT_MAP_LOGIC_PLAYER);
	}
	else if (request->cmd_name() == "red_exchange")
	{
		Proto11400135 respond;
		respond.set_exchange_id(request->param1());
		this->red_clothes_exchange(&respond);
		return 0;
	}
	else if (request->cmd_name() == "change_name")
	{
		Proto11400120 respond;
		respond.set_name("草草草");
		this->request_rename_role_begin(&respond);
		return 0;
	}
	else if (request->cmd_name() == "set_money")
	{
		Money money(request->param1(), request->param1(),
				request->param2(), request->param2());
		this->pack_money_set(money);
		return 0;
	}
	else if (request->cmd_name() == "add_money")
	{
		Money money(request->param1(), request->param1(),
				request->param1(), request->param1());
		return this->pack_money_add(money, ADD_FROM_LOCAL_TEST);
	}
	else if(request->cmd_name() == "clean_money")
	{
		return this->pack_money_sub(this->pack_detail().__money, SUB_MONEY_LOCAL_TEST);
	}
	else if (request->cmd_name() == "t_accept")
	{
		int task_id = request->param1(), prev_task_id = task_id;
		this->task_submited_once_set().clear();
		do {
			const Json::Value &task_json = CONFIG_INSTANCE->find_task(prev_task_id);
			prev_task_id = task_json["precondition"].asInt();
			if (prev_task_id > 0)
			{
				TaskInfo *prev_task = this->find_task(prev_task_id);
				if (prev_task != NULL)
				{
					prev_task->set_task_status(TaskInfo::TASK_STATUS_FINISH);
					this->process_submit(prev_task_id, false);
				}
				this->task_submited_once_set().insert(prev_task_id);
			}
		} while (prev_task_id > 0);

		if (this->find_task(task_id) == NULL)
		{
			const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
			this->insert_task(task_id, task_json);
		}

		return this->process_accept(task_id);
	}
	else if (request->cmd_name() == "t_abandon")
	{
		Proto11400327 req;
		req.set_task_id(request->param1());
		return this->task_abandon(&req);
	}
	else if (request->cmd_name() == "t_submit")
	{
		Proto11400328 req;
		req.set_task_id(request->param1());
		return this->task_submit(&req, true);
	}
	else if (request->cmd_name() == "t_fast")
	{
		MapLogicTasker::TaskMap &task_map = this->task_map();
		for (MapLogicTasker::TaskMap::iterator iter = task_map.begin(); iter != task_map.end(); ++iter)
		{
			TaskInfo *task_info = iter->second;
			task_info->__task_imp->process_finish(task_info);
		}
		return 0;
	}
	else if (request->cmd_name() == "t_del")
	{
		int task_id = request->param1();
		this->task_submited_once_set().erase(task_id);
		TaskInfo *task_info = this->find_task(task_id);
		this->task_accepted_lvl_.erase(task_info);
		this->task_accepted_monster_.erase(task_info);
		this->task_accepted_collect_.erase(task_info);
	    this->task_accepted_attend_.erase(task_info);
	    this->task_accepted_package_.erase(task_info);
	    this->task_accepted_scene_.erase(task_info);
	    this->task_accepted_branch_.erase(task_info);

	    if (request->param1() == 1)
	    {
	    	const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
	    	this->init_task(task_id, task_json);
	    	this->process_accept(task_id);
	    }
	}
    else if (request->cmd_name() == "t_lvlto")
    {
    	int lvl = request->param1();
    	const Json::Value &task_list = CONFIG_INSTANCE->task_setting()["novice_task"];
    	for (uint i = 0; i < task_list.size(); ++i)
    	{
    		const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_list[i].asInt());
    		if (lvl > task_json["before_accept"]["level"][0u].asInt())
    			this->task_submited_once_set().insert(task_list[i].asInt());
    	}
    	MapLogicTasker::TaskMap &task_map = this->task_map();
    	std::vector<TaskInfo *> task_vc;
		for (MapLogicTasker::TaskMap::iterator iter = task_map.begin(); iter != task_map.end(); ++iter)
		{
			TaskInfo *task_info = iter->second;
			if (this->task_submited_once_set().find(task_info->__task_id) != this->task_submited_once_set().end())
				task_vc.push_back(task_info);
		}
		for (std::vector<TaskInfo *>::iterator iter = task_vc.begin(); iter != task_vc.end(); ++iter)
		{
			TaskInfo *task_info = *iter;
			task_info->__task_imp->process_submit(task_info, 1);
		}

    	this->notify_finished_task_set();
    	request->set_cmd_name("lvlto");
    	return MAP_MONITOR->process_inner_map_request(this->role_id(), *msg);
    }
    else if (request->cmd_name() == "t_task")
    {
    	return this->test_special_task(request->param1());
    }
    else if (request->cmd_name() == "t_fresh_r")
    {
    	this->routine_refresh_tick_ = Time_Value::zero;
    	return 0;
    }
    else if (request->cmd_name() == "t_fstar")
    {
        Proto11400345 req;
        req.set_task_id(request->param1());
        return this->request_refresh_task_star(&req);
    }
    else if (request->cmd_name() == "branch_task_f")
    {
        TaskInfo *task_info = 0;
        for (TaskSet::iterator iter = this->task_accepted_branch_.begin();
                iter != this->task_accepted_branch_.end(); ++iter)
        {
            task_info = *iter;
            if (task_info->is_accepted() == false || task_info->is_logic_branch() == false)
                continue;

            TaskConditon *task_cond = *(task_info->__condition_list.begin());
            task_info->task_imp()->process_listen_branch(task_info,
            		task_cond->__cond_id, request->param1());
        }
    }
    else if (request->cmd_name() == "t_rf")
    {
    	Proto11400346 info;
    	info.set_type(request->param1());
        this->request_routine_finish_all(&info);
        return 0;
    }
	else if (request->cmd_name() == "pa_all")
	{
		return this->fetch_player_assist_pannel(msg);
	}
	else if (request->cmd_name() == "pa_one")
	{
		Proto11401703 info;
		info.set_event_id(request->param1());
		info.set_tip_index(request->param2());
		return this->fetch_player_assist_tips(&info);
	}
	else if (request->cmd_name() == "mount_time")
	{
		MountDetail& mount_detail = this->mount_detail(request->param1());
		mount_detail.finish_bless_ = ::time(NULL) + request->param2();
		return this->fetch_mount_info(mount_detail.type_);
	}
	else if (request->cmd_name() == "t_activity")
	{
		Proto31400018 req;
		req.set_activity_type(request->param1());
		return this->sync_attend_activity(&req);
	}
	else if (request->cmd_name() == "remove")
	{
		ItemIndexObj item_obj(request->param1(), request->param2());
		this->pack_remove_by_index(ITEM_REMOVE_FROM_TEST, item_obj);
	}
	else if (request->cmd_name() == "clean_pkg")
	{
        this->clean_pack(GameEnum::INDEX_PACKAGE);
        return 0;
	}
	else if (request->cmd_name() == "takedown")
	{
		Proto11400605 info;
		info.set_item_id(request->param1());
		info.set_pkg_type(request->param2());
		info.set_pkg_index(request->param3());
		return this->take_off_equip(&info);
	}
	else if (request->cmd_name() == "orguprising")
	{
		Proto11400622 req;
		req.set_item_id(request->param1());
		req.set_pkg_type(request->param2());
		req.set_pkg_index(request->param3());
		return this->equip_orange_uprising(&req);
	}
	else if (request->cmd_name() == "equipdecp")
	{
		Proto11400623 req;
		PackageItem *pack_item = this->pack_find_by_id(request->param1());
		if (pack_item != NULL)
		{
			ProtoItem *proto_item = req.add_pack_item_list();
			proto_item->set_index(pack_item->__index);
			proto_item->set_id(pack_item->__id);
		}
		pack_item = this->pack_find_by_id(request->param2());
		if (pack_item != NULL)
		{
			ProtoItem *proto_item = req.add_pack_item_list();
			proto_item->set_index(pack_item->__index);
			proto_item->set_id(pack_item->__id);
		}
		return this->equip_decompose(&req);
	}
	else if (request->cmd_name() == "active")
	{
		Proto11400609 info;
		info.set_magical_id(request->param1());
		this->activate_magical_item(&info);
	}
	else if (request->cmd_name() == "magical")
	{
		Proto11400611 info;
		info.set_magical_id(request->param1());
		info.set_is_special(0);
		info.set_is_batch(request->param2());
		info.set_is_auto_buy(request->param3());
		this->magical_polish(&info);
	}
	else if (request->cmd_name() == "magicalchoose")
	{
		Proto11400612 info;
		info.set_polish_batch_mode(request->param1());
		info.set_select_index(request->param2());
		info.set_magical_id(request->param3());
		this->magical_polish_select_result(&info);
	}
	else if (request->cmd_name() == "label")
	{
		this->insert_label(request->param1());
	}
	else if (request->cmd_name() == "fetch_label")
	{
		this->fetch_label_panel_info(msg);
	}
	else if (request->cmd_name() == "select_label")
	{
		Proto11400702 info;
		info.set_label_id(request->param1());
		this->select_label(&info);
	}
	else if (request->cmd_name() == "achieve")
	{
		Proto11401102 info;
		info.set_ach_index(request->param1());
		info.set_achieve_id(request->param2());
		return this->get_achieve_reward(&info);
	}
	else if( request->cmd_name() == "check_in_info" )
	{
//		Proto11401401 msg;
		return this->fetch_check_in_info(0);
	}
	else if( request->cmd_name() == "check_in" )
	{
		Proto11401402 msg;
		return this->request_check_in(&msg);
	}
	else if(request->cmd_name() == "check_in_cd")
	{
		// 设置时间为一天以前
		this->check_in_detail().__last_time = ::time(0) - 24*60*60;
		this->check_in_detail().__charge_money = -1;

		this->role_detail().__ml_day_reset_tick = ::time(0) - 24*60*60;
		MapLogicPlayer::notify_player_welfare_status(this);
		MapLogicPlayer::reset_every_day();
	}
	else if(request->cmd_name() == "add_exp")
	{
		this->request_add_exp(request->param1(), SerialObj(ADD_EXP_FROM_TEST));
	}
	else if(request->cmd_name() == "er_event_finish")
	{
		Proto31400401 exp_request;
		exp_request.set_event_id(request->param1());
		this->exp_restore_event_done(&exp_request);
	}
	else if(request->cmd_name() == "er_info")
	{
		this->fetch_exp_restore_info();
	}
	else if(request->cmd_name() == "er_single")
	{
		Proto11401412 exp_request;
		exp_request.set_activity_id(request->param1());
		exp_request.set_restore_type(request->param2());
		this->exp_restore_single(&exp_request);
	}
	else if(request->cmd_name() == "er_all")
	{
		Proto11401413 exp_request;
		exp_request.set_restore_type(request->param1());
		this->exp_restore_all(&exp_request);
	}
	else if (request->cmd_name() == "treasures_info")
	{
		this->fetch_treasures_info();
	}
	else if(request->cmd_name() == "er_next_day")
	{
		this->restore_jump_next_day(request->param1());
	}
    else if(request->cmd_name() == "sm_enter")
    {
    	this->send_to_map_thread(CLIENT_JOIN_SM_BATTLE);
    }
	else if (request->cmd_name() == "rpm_info")
	{
		MAP_MONITOR->dispatch_to_logic(this, CLIENT_TEAM_FB_REPLACEMENT_LIST);
	}
    else if (request->cmd_name() == "rename")
    {
        char sz_name[MAX_NAME_LENGTH];
        ::sprintf(sz_name, "TEST%06d", request->param1());
        Proto11400332 req;
        req.set_role_name(sz_name);
        return this->request_task_rename(&req);
    }
    else if (request->cmd_name() == "rank")
    {
    	Proto31401601 info;
    	info.set_rank_type(RANK_SINGLE_SCRIPT_ZYFM);

    	int random = std::rand() % 10;
    	info.set_achieve_tick(::time(0));
    	info.set_rank_value(random);

    	Int64 role_id = this->role_id() + random;
    	info.set_role_id(role_id);

    	const char arr[10] = {'0','1','2','3','4','5','6','7','8','9'};
    	std::string role_name("ggg");
    	role_name.insert(role_name.end(), arr[random]);
    	info.set_role_name(role_name);
    	return RANK_SYS->update_rank_data(&info);
    }
    else if (request->cmd_name() == "rank_sameone")
    {
    	Proto31401601 info;
    	info.set_rank_type(RANK_SINGLE_SCRIPT_ZYFM);

    	int random = std::rand() % 10;
    	info.set_achieve_tick(::time(0));
    	info.set_rank_value(random);

    	Int64 role_id = this->role_id();
    	info.set_role_id(role_id);

    	const char arr[10] = {'0','1','2','3','4','5','6','7','8','9'};
    	std::string role_name("ggg");
    	role_name.insert(role_name.end(), arr[0]);
    	info.set_role_name(role_name);
    	return RANK_SYS->update_rank_data(&info);
    }
    else if (request->cmd_name() == "flow")
    {
    	int ret = FLOW_INSTANCE->check_flow_detail_type(request->param1());
    	MSG_DEBUG(%d, ret);
    	return ret;
    }
	else if(request->cmd_name() == "open_activity_Login")
	{
		Proto10100211 req;
		req.set_index(request->param1());
		MAP_MONITOR->dispatch_to_logic(this, &req);
	}
	else if(request->cmd_name() == "get_reward")
	{
		Proto10100212 req;
		req.set_index(request->param1());
		req.set_reward_index(request->param2());
		MAP_MONITOR->dispatch_to_logic(this, &req);
	}
    else if (request->cmd_name() == "vip_gift")
	{
    	Proto11400803 req;
    	req.set_vip_type(request->param1());
    	return this->gain_vip_gift(&req);
	}
    else if (request->cmd_name() == "fb_apply")
    {
    	int64_t role_id = request->param1();

    	int mun_tail = request->param2();

    	do
    	{
    		mun_tail /= 10;
    		role_id *=10;
    	}
    	while(mun_tail != 0);

    	role_id += request->param2();

    	Proto10100406 req;
    	req.set_leader_id(role_id);
    	this->monitor()->dispatch_to_logic(this, &req);
    }
    else if (request->cmd_name() == "fb_rpm")
    {
    	Proto10100416 req;
    	req.set_is_introduction(1);
    	this->monitor()->dispatch_to_logic(this, &req);
    }
    else if(request->cmd_name() == "online_rw")
    {
    	this->fetch_online_rewards();
    }
    else if(request->cmd_name() == "enter_script")
    {
    	Proto10400901 req;
    	req.set_script_sort(request->param1());
    	req.set_piece(request->param2());
    	req.set_chapter(request->param3());

    	this->monitor()->dispatch_to_scene(this, this->scene_id(), &req);
    }
    else if(request->cmd_name() == "vip_info")
    {
    	this->fetch_vip_info();
    }
    else if(request->cmd_name() == "super_vip_info")
    {
    	this->fetch_super_vip_info_begin();
    }
    else if(request->cmd_name() == "map_vip_info")
    {
    	MAP_MONITOR->process_inner_map_request(this->role_id(),
    			INNER_MAP_DEBUG_OUTPUT_VIP_INFO);
    }
    else if (request->cmd_name() == "fname")
    {
    	this->fetch_random_rename(0);
    }
    else if (request->cmd_name() == "create_tick")
    {
    	this->role_detail_.set_create_tick(::time(NULL) + request->param1() * Time_Value::DAY);
    }
    else if (request->cmd_name() == "ingore_tip")
	{
		this->request_ingore_show_pannel(0);
	}
    else if (request->cmd_name() == "welfare_keys")
	{
    	Proto51401421 res;
		return this->fetch_display_welfare_elements(0, &res, request->param1());
	}
    else if(request->cmd_name() == "media_gift")
    {
    	int code_num = request->param1();
    	char code_str[32]= {0};
    	sprintf(code_str, "%020x", code_num);

    	Proto11401431 req;
    	req.set_acti_code(code_str);
    	this->use_acti_code_begin(&req);
    }
    else if(request->cmd_name() == "lsy_box")
    {
    	this->fetch_download_box_info();
    }
    else if(request->cmd_name() == "media_def")
    {
    	this->fetch_media_gift_config();
    }
    else if(request->cmd_name() == "recharge")
    {
    	return this->recharge(request->param1());
    }
    else if(request->cmd_name() == "recharge_award")
    {
    	this->fetch_recharge_awards();
    }
    else if(request->cmd_name() == "recharge_info")
    {
    	this->fetch_recharge_awards_info();
    }
    else if(request->cmd_name() == "recharge_")
    {
    	int recharge_money = request->param1();

    	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    	JUDGE_RETURN(NULL != data_map, 0);

    	BackRecharge::recharge_test(data_map, this->role_id(),
    			this->role_detail_.__account, recharge_money);

    	if(TRANSACTION_MONITOR->request_mongo_transaction(0,
    			TRANS_UPDATE_BACK_RECHARGE_ORDER_FLAG,data_map) != 0)
    	{
    		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
    		return -1;
    	}
    }
    else if(request->cmd_name() == "reset_found")
    {
    	this->reset_found();
    }
    else if(request->cmd_name() == "recharge_clean")
    {
    	this->recharge_detail().__recharge_times = 0;
    	this->recharger_check_notify();
    }
    else if(request->cmd_name() == "no_confirm")
    {
    	int recog_id = request->param1();
    	int sys_type = GameCommon::commercial_recogn_type(recog_id);
    	this->pack_detail().__auto_exchange_copper[sys_type] = true;
    }
    else if (request->cmd_name() == "daily_test")
    {
    	BackDR_SYS->send_mail_to_player(this->role_id());
    	this->daily_recharge_clean();
    }
    else if(request->cmd_name() == "dr_reset" || request->cmd_name() == "dr_clean" )
    {
    	this->daily_recharge_clean();
    }
    else if(request->cmd_name() == "dr_info")
    {
    	this->fetch_daily_recharge_info();
    }
    else if(request->cmd_name() == "dr_fetch")
    {
    	Proto11401804 inner_req;
    	inner_req.set_reward_index(request->param1());
    	this->fetch_daily_recharge_rewards(&inner_req);
    }
    else if(request->cmd_name() == "ir_info")
    {
    	this->fetch_invest_recharge_info();
    }
    else if(request->cmd_name() == "ir_fetch")
    {
    	Proto11401811 inner_req;
    	inner_req.set_index(request->param1());
    	this->fetch_invest_recharge_rewards(&inner_req);
    }
    else if(request->cmd_name() == "rr_info")
    {
    	this->fetch_rebate_recharge_info();
    }
    else if(request->cmd_name() == "rr_fetch")
    {
    	this->fetch_rebate_recharge_rewards();
    }
    else if (request->cmd_name() == "town")
    {
    	Proto11400151 req;
    	return this->request_enter_main_town(&req);
    }
    else if(request->cmd_name() == "sr_insert")
    {
    	int param1 = request->param1();
    	int param2 = request->param2();
    	for(int i=param1; i<param2; ++i)
    	{
    		int use_time = ::rand() % 500 + 1;
    		int difficult = :: rand() % 7 + 1;

    		tm now_tm;
    		time_t now_tick = ::time(NULL);
    		::localtime_r(&now_tick, &now_tm);

    		int seven_type = now_tm.tm_wday;
    		int script_sort = 29000 + seven_type*100 + difficult*10;

    		char buff[128]={0};
    		sprintf(buff, "%06d", i);

        	Proto30101501 req;
        	req.set_script_sort(script_sort);
        	req.set_use_time(use_time);
        	req.add_role_name(buff);
        	req.add_role_id(::random());

        	if(difficult > 4)
        		req.set_team_script(true);

        	MSG_DEBUG(%s, req.Utf8DebugString().c_str());

        	this->monitor()->dispatch_to_logic(this, &req);
    	}

    }
    else if (request->cmd_name() == "c_compact")
    {
    	this->script_compact_detail().__expired_tick = Time_Value::gettimeofday() + Time_Value(180, 0);
        this->script_compact_player()->update_player_assist_single_event(GameEnum::PA_EVENT_SCRIPT_COMPACT, 0);
        this->sync_script_compact_info();
        this->notify_script_compact_info();
        return 0;
    }
    else if(request->cmd_name() == "reset_seven_login")
    {
    	this->role_detail().draw_days_.clear();
    	this->role_detail().draw_vips_.clear();
    	return 0;
    }
    else if (request->cmd_name() == "reset_open_gift")
    {
    	return this->test_reset_open_gift();
    }
    else if (request->cmd_name() == "start_tra")
    {
    		return this->trade_start();
    }
    else if (request->cmd_name() == "cancel_tra")
    {
		if (request->param1() == 1 || request->param1() == 0)
		{
			return this->inner_notify_trade_cancel(request->param1());
		}
		else
		{
			return 0;
		}
    }
    else if (request->cmd_name() == "close_tra")
    {
    		return this->trade_close_panel();
    }
    else if (request->cmd_name() == "lock_tra")
    {
    		return this->trade_lock_panel();
    }
    else if (request->cmd_name() == "timeout_tra")
    {
    		this->trade_wait_once_end();
    		return 0;
    }
    else if(request->cmd_name() == "mw_info")
    {
		this->magic_weapon_fetch_info();
		 return 0;
    }
    else if(request->cmd_name() == "mw_act")
   {
	 Proto11402502 msg;
	 msg.set_magicweapon_id(request->param1());
	 this->magic_weapon_active(&msg);
	 return 0;
   }
    else if(request->cmd_name() == "mw_r_up")
    {
    	Proto11402503 msg;
        msg.set_mw_id(request->param1());
        msg.set_auto_buy(request->param2());
        this->magic_weapon_promote_rank(&msg);
        return 0;
     }
    else if(request->cmd_name() == "mw_q_up")
	 {
    	Proto11402504 msg;
	   msg.set_mw_id(request->param1());
	   msg.set_auto_buy(request->param2());
	   this->magic_weapon_promote_quality(&msg);
	   return 0;
	 }
    else if(request->cmd_name() == "mw_on")
	{
		Proto11402505 msg;
		msg.set_mw_id(request->param1());
		msg.set_put_on(request->param2());
		this->magic_weapon_adorn(&msg);
		return 0;
	}
    else if(request->cmd_name() == "output_item")
    {
    	int item_index = request->param1();
    	PackageItem* item = this->pack_find(item_index);
		if(item != NULL)
		{
			GameCommon::output_item_info(item);
		}
    }
    else if(request->cmd_name() == "achieve_info")
    {
    	this->request_fetch_achieve_info();
    	return 0;
    }
    else if(request->cmd_name() == "offhook")
    {
    	return this->cmd_hooktime(request->param1());
    }
    else if(request->cmd_name() == "use_offline_item")
    {
    	return this->use_offline_plus_item(request->param1(), request->param2());
    }
    else if(request->cmd_name() == "molding_nature")
    {
    	Proto11400663 msg;
    	msg.set_index(request->param1());
    	msg.set_times(request->param2());
    	return this->equip_molding_spirit(&msg);
    }
    else if(request->cmd_name() == "fetch_molding")
    {
    	return this->fetch_molding_spirit_info(request->param1());
    }
    else if(request->cmd_name() == "clean_molding")
    {
    	return this->test_command_clean_molding_nature();
    }
    else if(request->cmd_name() == "test_prop")
    {
    	const Json::Value& reward_json = CONFIG_INSTANCE->reward(930001);
    	JUDGE_RETURN(reward_json.empty() == false, -1);

    	RewardInfo reward_info;
    	GameCommon::make_up_rand_reward_items(reward_info, 8, reward_json);
    	SerialObj obj(ADD_FROM_RETURN_ACTIVITY_BUY, 1);
    	for (ItemObjVec::iterator iter = reward_info.item_vec_.begin();
    			iter != reward_info.item_vec_.end(); ++iter)
    	{
    		this->add_reward(iter->id_, obj);
    	}
    }
	else
	{
		//将消息转发到地图线程
		MAP_MONITOR->process_inner_map_request(this->role_id(), *msg);
	}
#endif
	return 0;
}

int MapLogicPlayer::use_pack_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400106*, request, RETURN_USE_GOODS);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_USE_GOODS, ERROR_PACKAGE_TYPE);

	int ret = this->client_auto_use_item(package, request->index());
	JUDGE_RETURN(ret == -1, 0);

	PackageItem* pack_item = package->find_by_index(request->index());
	JUDGE_RETURN(pack_item != NULL, 0);

	int amount = request->amount();
	CONDITION_NOTIFY_RETURN(amount > 0 && amount <= pack_item->__amount, RETURN_USE_GOODS,
			ERROR_CLIENT_OPERATE);

	const Json::Value& item_cfg = pack_item->conf();
	CONDITION_NOTIFY_RETURN(item_cfg.empty() == false, RETURN_USE_GOODS, ERROR_CONFIG_ERROR);

	int limit_lvl = item_cfg["use_lvl"].asInt();
	CONDITION_NOTIFY_RETURN(limit_lvl <= this->role_level(), RETURN_USE_GOODS, ERROR_PLAYER_LEVEL);

	Proto30400444 goods_info;
	goods_info.set_index(request->index());
	goods_info.set_amount(amount);
	goods_info.set_id(pack_item->__id);
	return this->send_to_map_thread(goods_info);
}

int MapLogicPlayer::after_check_use_pack_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30400444*, request, RETURN_USE_GOODS);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_USE_GOODS, ERROR_PACKAGE_TYPE);

	PackageItem* pack_item = package->find_by_index(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_USE_GOODS, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(pack_item->__id == request->id(),
			RETURN_USE_GOODS, ERROR_CLIENT_OPERATE);

	int amount = request->amount();
	CONDITION_NOTIFY_RETURN(amount > 0 && amount <= pack_item->__amount,
			RETURN_USE_GOODS, ERROR_CLIENT_OPERATE);

	const Json::Value& effect_conf = pack_item->conf()["effect_prop"];
	CONDITION_NOTIFY_RETURN(effect_conf.empty() == false, RETURN_USE_GOODS,
			ERROR_GOODS_DIRECT_USE);

	int ret_flag = this->dispatch_prop_status(effect_conf, pack_item, amount);
	CONDITION_NOTIFY_RETURN(ret_flag >= 0, RETURN_USE_GOODS, ret_flag);

	Proto51400106 respond;
	respond.set_item_id(pack_item->__id);

	// none error
	switch(ret_flag)
	{
	case GameEnum::CHECK_AND_USE_IN_MAP:
	{
		return this->pack_remove(package, ITEM_PLAYER_USE, pack_item, amount);
	}

	case GameEnum::USE_GOODS_MUCH_TIMES:
	{
		return 0;
	}

	default:
	case GameEnum::USE_GOODS_WELL:
	{
		this->pack_remove(package, ITEM_PLAYER_USE, pack_item, amount);
		FINER_PROCESS_RETURN(RETURN_USE_GOODS, &respond);
	}
  }
}

int MapLogicPlayer::map_use_pack_goods_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400015*, request, -1);
	return this->pack_insert(ITEM_USE_FAILED_BACK, request->item_info());
}

int MapLogicPlayer::request_add_blood_container(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10400005*, request, -1);

	if (request->use_type() == 3)
	{
		//request use
		return this->request_use_blood_goods(msg);
	}
	else if (request->use_type() == 1)
	{
		//auto use
		return this->auto_use_blood_goods(msg);
	}
	else
	{
		// auto buy
		return this->auto_buy_blood_goods(msg);
	}
}

int MapLogicPlayer::request_use_blood_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10400005*, request, -1);

	PackageItem* pack_item = this->pack_find(request->item_index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_AUTO_USE_BLOOD,
			ERROR_CLIENT_OPERATE);

	const Json::Value& effect_config = CONFIG_INSTANCE->prop(pack_item->__id)["effect"];
	int value = 0;
	if(effect_config.isArray() == true)
	{
		for(uint i = 0; i< effect_config.size(); i++)
		{
			 if(effect_config[i]["name"].asString() == PropName::ADD_BLOOD)
			 {
				 value = effect_config[i]["value"].asInt();
				 break;
			 }

		}

	}
	else
	{
		value = effect_config["value"].asInt();
	}
	CONDITION_NOTIFY_RETURN(request->left_capacity() >= value,RETURN_AUTO_USE_BLOOD, ERROR_BLOOD_CONTAINER_FULL);

	Proto30400444 use_info;
	use_info.set_amount(1);
	use_info.set_index(pack_item->__index);

	return this->after_check_use_pack_goods(&use_info);
}

int MapLogicPlayer::auto_use_blood_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10400005*, request, -1);

	bool have_item = false;
	bool arrive_max = false;

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	const Json::Value &blood_item = CONFIG_INSTANCE->blood_cont("blood_item");

	for (uint i = 0; i < blood_item.size(); ++i)
	{
		PackageItem* pack_item = package->find_by_id(blood_item[i][0u].asInt());
		JUDGE_CONTINUE(pack_item != NULL);

		have_item = true;

		const Json::Value& prop_config = CONFIG_INSTANCE->prop(pack_item->__id);
		int value = 0;
		if(prop_config["effect"].isArray() == true)
			{
			      const Json::Value& effect_json = prop_config["effect"];
				for(uint i = 0; i< prop_config["effect"].size(); i++)
				{
					 if(effect_json.asString() == PropName::ADD_BLOOD)
					 {
						 value = effect_json[i]["value"].asInt();
						 break;
					 }

				}

			}
			else
			{
				value = prop_config["effect"].asInt();
			}

		if (request->left_capacity() < value)
		{
			arrive_max = true;
			continue;
		}

		arrive_max = false;

		Proto30400444 use_info;
		use_info.set_amount(1);
		use_info.set_index(pack_item->__index);

		this->after_check_use_pack_goods(&use_info);
		JUDGE_BREAK(this->get_last_error() == 0);

		this->send_to_map_thread(*request);
		break;
	}

	if (have_item == false && request->use_times() == 1)
	{
		this->respond_to_client_error(RETURN_AUTO_USE_BLOOD,
				ERROR_GOODS_NO_EXIST);
	}

	if (arrive_max == true)
	{
		this->respond_to_client_error(RETURN_AUTO_USE_BLOOD,
				ERROR_BLOOD_CONTAINER_FULL);
	}

	return 0;
}

int MapLogicPlayer::auto_buy_blood_goods(Message* msg)
{
	const Json::Value &blood_item = CONFIG_INSTANCE->blood_cont("blood_item");

	// auto buy
	Money money;
	money.__bind_copper = blood_item[0u][1u].asInt();

	PROCESS_AUTO_BIND_COPPER_EXCHANGE_AUTO_REPEAT(money, RETURN_AUTO_USE_BLOOD, 1, msg);
	GameCommon::adjust_money(money, this->own_money());

	int ret = this->pack_money_sub(money, SUB_MONEY_BUY_BLOOD_PACK);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_AUTO_USE_BLOOD, ret);

	this->send_to_map_thread(INNER_MAP_ADD_CONT_BLOOD);
	//this->send_to_map_thread(*msg);

	return 0;
}

int MapLogicPlayer::validate_prop_usage(const Json::Value& prop_config, PackageItem* pack_item)
{
//	JUDGE_RETURN(prop_config.isMember("limit") == true, 0);
//
//	const Json::Value& limit_config = prop_config["limit"];
//	if (limit_config.isMember("lvl") == true)
//	{
//		JUDGE_RETURN(this->role_level() >= limit_config["lvl"].asInt(), ERROR_PLAYER_LEVEL);
//	}
//
//	if (limit_config.isMember("limit_scene") == true)
//	{
//		JUDGE_RETURN(GameCommon::is_value_in_config(limit_config["limit_scene"],
//				this->scene_id()) == false, ERROR_USE_SCENE_LIMIT);
//	}
//
//	if (limit_config.isMember("cool") == true)
//	{
//		JUDGE_RETURN(GameCommon::validate_time_span(pack_item->__use_tick,
//				limit_config["cool"].asInt()) == true, ERROR_DIVINEGON_TIME);
//	}

	return 0;
}

int MapLogicPlayer::dispatch_prop_status(const Json::Value& effect, PackageItem* pack_item, int& use_num)
{
	string prop_name = effect["name"].asString();

	if (prop_name == PropName::ADD_EXP)
	{
		this->request_add_exp(effect["value"].asInt() * use_num, EXP_FROM_PROP);
		return 0;
	}
	else if (prop_name == PropName::ADD_GOLD)
	{
		Money money;
		money.__gold = effect["value"].asInt() * use_num;
		return this->pack_money_add(money, ADD_FORM_USE_ITEM);
	}
	else if (prop_name == PropName::ADD_BIND_GOLD)
	{
		Money money;
		money.__bind_gold = effect["value"].asInt() * use_num;
		return this->pack_money_add(money, ADD_FORM_USE_ITEM);
	}
	else if (prop_name == PropName::ADD_REPUTATION)
	{
		this->add_game_resource(GameEnum::ITEM_ID_REPUTATION,
				effect["value"].asInt() * use_num, ADD_FORM_USE_ITEM);
		return 0;
	}
	else if (prop_name == PropName::ADD_EXPLOIT)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_EXPLOIT,
				effect["value"].asInt() * use_num, ADD_FORM_USE_ITEM);
	}
	else if (prop_name == PropName::ADD_CONTRI)
	{
		return this->dispatch_to_logic_league(ADD_FORM_USE_ITEM,
				effect["value"].asInt() * use_num);
	}
	else if (prop_name == PropName::ADD_HONOR)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_HONOUR,
				effect["value"].asInt() * use_num, ADD_FORM_USE_ITEM);
	}
	else if (prop_name == PropName::ADD_REIKI)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_REIKI,
				effect["value"].asInt() * use_num, ADD_FORM_USE_ITEM);
	}
	else if (prop_name == PropName::ADD_PRACTICE)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_PRACTICE,
				effect["value"].asInt() * use_num, ADD_FORM_USE_ITEM);
	}
	else if (prop_name == PropName::ADD_SPIRIT)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_SPIRIT,
				effect["value"].asInt() * use_num, ADD_FORM_USE_ITEM);
	}
	else if (prop_name == PropName::HEALTH)
	{
		this->request_map_use_goods(pack_item, use_num);
		return GameEnum::CHECK_AND_USE_IN_MAP;
	}
	else if (prop_name == PropName::EXP_PERCENT)
	{
		use_num = 1;
		return this->request_add_exp_per_buff(effect["buff_id"].asInt(),
				effect["percent"].asInt(), effect["last"].asInt());
	}
	else if (prop_name == PropName::MAGIC)
	{
		return this->request_direct_add_magic(effect["value"].asInt() * use_num);
	}
	else if (prop_name == PropName::ADD_BLOOD)
	{
		return this->request_direct_add_blood(effect["value"].asInt() * use_num);
	}
	else if (prop_name == PropName::GIFT_PACK)
	{
		return this->use_gift_pack(effect, pack_item, use_num);
	}
	else if (prop_name == PropName::FIX_GIF_PACK)
	{
		return this->use_fix_gift_pack(effect, pack_item, use_num);
	}
	else if (prop_name == PropName::RESP_GIF_PACK)
	{
		return this->use_resp_gif_pack(effect, pack_item, use_num);
	}
	else if (prop_name == PropName::TOTAL_GIF_PACK)
	{
		return this->use_total_gift_pack(effect, pack_item, use_num);
	}
    else if (prop_name == PropName::OPEN_BOX_BY_KEY)
    {
        return this->open_box_by_key(effect, pack_item, use_num);
    }
    else if (prop_name == PropName::RAND_GIF_PACK)
    {
    	return this->use_rand_gif_pack(effect, pack_item, use_num);
    }
    else if(prop_name == PropName::MAGIC_ACT_PACK)
    {
    	return this->use_magic_act_pack(effect, pack_item, use_num);
    }
	else if (prop_name == PropName::LEVEL_UP)
	{
		use_num = 1;
		return this->check_and_level_up_player(effect);
	}
	else if (prop_name == PropName::TRANSFER)
	{
		return this->check_and_transfer_player(effect, pack_item);
	}
    else if (prop_name == PropName::TRANSFER_RANDOM)
    {
        return this->check_and_transfer_random_player(effect, pack_item);
    }
    else if (prop_name == PropName::ROUTINE_FINISH)
    {
        return this->routine_fast_finish(effect, pack_item);
    }
    else if(prop_name == PropName::RECHARGE)
    {
    	return this->use_recharge_item(effect , use_num);
    }
    else if (prop_name == PropName::SCRIPT_COMPACT)
    {
        return this->use_script_compact(effect["last_day"].asInt());
    }
    else if (prop_name == PropName::ADD_LUCKY)
    {
    	return this->request_use_lucky_prop(pack_item, use_num);
    }
    else if (prop_name == PropName::REDUCE_KILL)
    {
    	this->request_map_use_goods(pack_item, use_num);
    	return 0;
    }
    else if (prop_name == PropName::ADD_LABEL)
    {
        return this->insert_label_by_item(effect, pack_item, use_num);
    }
    else if (prop_name == PropName::MOUNT_LEVEL)
    {
    	use_num = 1;
    	return this->check_and_upgrade_mount(GameEnum::FUN_MOUNT, effect);
    }
    else if (prop_name == PropName::XIANYU_LEVEL)
	{
    	use_num = 1;
		return this->check_and_upgrade_mount(GameEnum::FUN_XIAN_WING, effect);
	}
    else if (prop_name == PropName::MAGIC_LEVEL)
	{
    	use_num = 1;
		return this->check_and_upgrade_mount(GameEnum::FUN_MAGIC_EQUIP, effect);
	}
    else if (prop_name == PropName::BEAST_LEVEL)
	{
    	use_num = 1;
		return this->check_and_upgrade_mount(GameEnum::FUN_LING_BEAST, effect);
	}
    else if (prop_name == PropName::GODSOLIDER_LEVEL)
	{
    	use_num = 1;
		return this->check_and_upgrade_mount(GameEnum::FUN_GOD_SOLIDER, effect);
	}
    else if (prop_name == PropName::BEAST_EQUIP_LEVEL)
    {
    	use_num = 1;
    	return this->check_and_upgrade_mount(GameEnum::FUN_BEAST_EQUIP, effect);
    }
    else if (prop_name == PropName::BEAST_MOUNT_LEVEL)
	{
    	use_num = 1;
    	return this->check_and_upgrade_mount(GameEnum::FUN_BEAST_MOUNT, effect);
	}
    else if (prop_name == PropName::BEAST_WING_ACT)
    {
    	use_num = 1;
    	return this->use_beast_wing_act_goods(pack_item->__id, effect["value"].asInt());
    }
    else if (prop_name == PropName::SWORD_POOL_EXP)
    {
        this->add_sword_pool_exp(use_num * effect["value"].asInt());
    }
    else if (prop_name == PropName::ADD_FASHION)
    {
    	return this->add_fashion(effect["value"].asInt());
    }
    else if (prop_name == PropName::ADD_TRANSFER)
    {
        return this->add_transfer(effect["value"].asInt(), effect["time"].asInt());
    }
    else if(prop_name == PropName::ADD_FISH_SCORE)
    {
    	return this->add_act_fish_score(use_num * effect["value"].asInt());
    }

	return 0;
}

int MapLogicPlayer::dispatch_beast_prop(const Json::Value& effect, PackageItem* pack_item, int use_num)
{
	return 0;
}

int	MapLogicPlayer::logic_request_add_money(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400011*, request, -1);

	return this->pack_money_add(request->add_money(), request->serial_obj(), request->is_notify());
}

int	MapLogicPlayer::enemy_transalye_sub_money(Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto30101604*, request, -1);
//	int ret = this->pack_money_sub(Money(request->sub_money()),SerialObj(SUB_MONEY_TRANSLATE_TO_ENEMY));
//	if(ret == ERROR_PACK_BIND_GOLD_AMOUNT)
//	{
//		CONDITION_NOTIFY_RETURN(request->sub_money().bind_gold()<= (this->own_money().__bind_gold + (this->own_money().__gold* 10))
//				,RETURN_FETCH_TRANSALTE,ERROR_PACKAGE_GOLD_AMOUNT);
//
//		Money need_money(0,0,0,0);
//		int need_bind_gold = request->sub_money().bind_gold();
//		need_money.__bind_gold = this->own_money().__bind_gold;
//		if((need_bind_gold - this->own_money().__bind_gold) % 10 != 0)
//		{
//			need_money.__bind_gold -= (10-((need_bind_gold - this->own_money().__bind_gold) % 10));
//			need_money.__gold ++;
//		}
//
//		need_money.__gold += (need_bind_gold - this->own_money().__bind_gold) / 10;
//		ret = this->pack_money_sub(need_money,SerialObj(SUB_MONEY_TRANSLATE_TO_ENEMY));
//	}
//	CONDITION_NOTIFY_RETURN(ret == 0 ,RETURN_FETCH_TRANSALTE,ret);
//
//	Proto10401305 req;
//	req.set_scene_id(request->scene_id());
//	req.set_pixel_x(request->pixel_x());
//	req.set_pixel_y(request->pixel_y());
//	req.set_flag(3);
//
//	return this->send_to_map_thread(req);
	return 0;
}

int MapLogicPlayer::logic_request_remove_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400041*, request, -1);

	SerialObj serial_obj;
	serial_obj.unserialize(request->serial());

	PackageItem item;
	item.unserialize(request->item_set());
	this->pack_remove(serial_obj, item.__id, item.__amount);

	return 0;
}

int MapLogicPlayer::logic_request_add_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400012*, request, -1);

	SerialObj serial_obj;
	serial_obj.unserialize(request->serial_obj());

	if (this->pack_insert(serial_obj, request->item_set()) == 0)
	{
		request->set_operate_result(0);
	}
	else
	{
		request->set_operate_result(-1);
	}

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::logic_request_add_batch_goods(Message* msg)
{
	return 0;
}

int MapLogicPlayer::map_request_pickup_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400013*, request, -1);

	int result_flag = 0;
	SerialObj serail(ADD_FROM_MONSTER_DROP, request->sort_id());

	if (request->has_add_money() == true)
	{
		Money add_money(request->add_money());
		result_flag = this->pack_money_add(add_money, serail);
	}
	else
	{
		const ProtoItem &proto_item = request->add_item();
		this->pack_insert(serail, proto_item);
	}

	Proto30400012 add_result;
	add_result.set_result_flag(result_flag);
	add_result.set_drop_id(request->drop_id());
	return this->send_to_map_thread(add_result);
}

int MapLogicPlayer::map_request_gather_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400017*, request, RETURN_GATHER_GOODS);

	const Json::Value& gather_conf = CONFIG_INSTANCE->gather(request->monster_sort());

	int result_flag = 0;
	switch (gather_conf["gather_type"].asInt())
	{
	case GameEnum::GATHER_ITEM:
	{
		ItemObj item_obj = GameCommon::make_up_itemobj(gather_conf["item"]);
		item_obj.serialize(request->mutable_gather_item());
		result_flag = this->pack_insert(ADD_FROM_ITEM_GATHER, item_obj);
		break;
	}

	case GameEnum::GATHER_PROCESS:
	{
		result_flag = 0;
		ItemObj item_obj = GameCommon::make_up_itemobj(gather_conf["item"]);
		item_obj.serialize(request->mutable_gather_item());
		this->task_listen_collect_item(item_obj.id_, std::max(item_obj.amount_, 1));
		break;
	}

	case GameEnum::GATHER_RANDOM_ONE:
	{
		// goods
		int rand_value = std::rand() % 10000, base_value = 0,
				item_id = 0, item_amount = 0, item_bind = 0;
		for (uint i = 0; i < gather_conf["item"].size(); ++i)
		{
			base_value += int(gather_conf["item"][i][3u].asDouble() * 100);
			if (rand_value < base_value)
			{
				item_id = gather_conf["item"][i][0u].asInt();
				item_amount = gather_conf["item"][i][1u].asInt();
				item_bind = gather_conf["item"][i][2u].asInt();
				break;
			}
		}
		ProtoItem *proto_item = request->mutable_gather_item();
		proto_item->set_id(item_id);
		proto_item->set_amount(item_amount);
		proto_item->set_bind(item_bind);
		result_flag = this->insert_package(SerialObj(ADD_FROM_ITEM_GATHER, request->monster_sort()), item_id, item_amount, item_bind);
		break;
	}
	case GameEnum::GATHER_CHESTS:
	{
		Collect_Chests &player_info = this->collect_chests();

		int max_num = CONFIG_INSTANCE->collect_chests_json(player_info.cycle_id)["max_num"].asInt();
		if (player_info.collect_num >= max_num)
		{
			result_flag = -1;
			break;
		}
		else
		{

			int award_id = CONFIG_INSTANCE->monster(request->monster_sort())["drop_reward"].asInt();

			SerialObj obj(ADD_FROM_GATHER_REWARD, 0, 0);
			result_flag = this->add_reward(award_id, obj);
			player_info.collect_num++;
			this->notify_collect_info();

			if (player_info.collect_num == max_num) this->notify_get_extra_award();
			break;
		}
	}
	case GameEnum::GATHER_REWARD:
	{
		int award_id = CONFIG_INSTANCE->monster(request->monster_sort())["drop_reward"].asInt();

		SerialObj obj(ADD_FROM_GATHER_REWARD, 0, 0);
		result_flag = this->add_reward(award_id, obj);

		break;
	}
	}

	request->set_gather_flag(result_flag);
	return this->send_to_map_thread(*request);
}

int MapLogicPlayer::fetch_other_player_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400261*, request, -1);

	MapLogicPlayer *player = MAP_MONITOR->find_logic_player(request->other_id());
	JUDGE_RETURN(player != NULL, -1);

	Proto50100031 master_info;
	switch (request->recogn())
	{
	case RETURN_OTHER_MASTER_INFO:
	{
		player->fetch_mounter_info(&master_info);
		request->set_msg_body(master_info.SerializeAsString());
		break;
	}

	default:
	{
		return -1;
	}
	}

	if (GameCommon::is_travel_scene(this->scene_id()) && request->query_type() == GameEnum::RANK_QUERY_TRAVEL_PET)
	{
		MapLogicPlayer *self_player = MAP_MONITOR->find_logic_player(request->self_id());
		if (self_player != NULL)
			self_player->respond_to_client(RETURN_OTHER_MASTER_INFO, &master_info);
		return 0;
	}

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::fetch_single_player_all_detail(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto50100156*, request, -1);

	this->make_up_equipment_list(request);
	this->make_up_mount_list(request);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::fetch_ranker_detail(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto50100702 *, respond, -1);
    
    respond->set_mounter(this->mount_detail().mount_shape_);

    switch (respond->rank_type())
    {
    	case RANK_FIGHT_FORCE:
    	case RANK_FIGHT_LEVEL:
    		respond->set_force(this->role_detail().__fight_force);
    		break;
    	case RANK_FUN_MOUNT:
    		respond->set_force(this->mount_detail(GameEnum::FUN_MOUNT).total_prop_.force_);
    		break;
    	case RANK_FUN_GOD_SOLIDER:
    		respond->set_force(this->mount_detail(GameEnum::FUN_GOD_SOLIDER).total_prop_.force_);
    		break;
    	case RANK_FUN_MAGIC_EQUIP:
    		respond->set_force(this->mount_detail(GameEnum::FUN_MAGIC_EQUIP).total_prop_.force_);
    		break;
    	case RANK_FUN_XIAN_WING:
    		respond->set_force(this->mount_detail(GameEnum::FUN_XIAN_WING).total_prop_.force_);
    		break;
    	case RANK_FUN_LING_BEAST:
    		respond->set_force(this->mount_detail(GameEnum::FUN_LING_BEAST).total_prop_.force_);
    		break;
    	case RANK_FUN_BEAST_EQUIP:
    		respond->set_force(this->mount_detail(GameEnum::FUN_BEAST_EQUIP).total_prop_.force_);
    		break;
    	case RANK_FUN_BEAST_MOUNT:
    		respond->set_force(this->mount_detail(GameEnum::FUN_BEAST_MOUNT).total_prop_.force_);
    		break;
    	case RANK_FUN_BEAST_WING:
    		respond->set_force(this->mount_detail(GameEnum::FUN_BEAST_WING).total_prop_.force_);
    		break;
    	case RANK_FUN_BEAST_MAO:
    		respond->set_force(this->mount_detail(GameEnum::FUN_BEAST_MAO).total_prop_.force_);
    		break;
    	case RANK_FUN_TIAN_GANG:
    		respond->set_force(this->mount_detail(GameEnum::FUN_TIAN_GANG).total_prop_.force_);
    		break;


    }

    return this->monitor()->dispatch_to_logic(this, respond);
}

int MapLogicPlayer::process_fetch_self_ranker_info(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31402302 *, request, -1);

    int rank_type = request->rank_type();

    if (rank_type == RANK_MOUNT || rank_type == RANK_FUN_MOUNT || rank_type == RANK_FUN_GOD_SOLIDER ||
    		 rank_type == RANK_FUN_MAGIC_EQUIP || rank_type == RANK_FUN_XIAN_WING || rank_type == RANK_FUN_LING_BEAST ||
    		 rank_type == RANK_FUN_BEAST_EQUIP || rank_type == RANK_FUN_BEAST_MOUNT || rank_type == RANK_FUN_BEAST_WING ||
    		 rank_type == RANK_FUN_BEAST_MAO || rank_type == RANK_FUN_TIAN_GANG)
    {
    	int type = rank_type - (RANK_FUN_MOUNT - GameEnum::FUN_MOUNT);
    	if (type < 1) type = 1;
        request->set_rank_value(this->mount_detail(type).mount_grade_);
    }
    else
    {
        MSG_USER("ERROR can't recongnize the rank_type %d", rank_type);
        return -1;
    }

    return this->monitor()->dispatch_to_logic(this, request);
}

int MapLogicPlayer::check_and_set_all_brighten()
{
	return 0;
}

int MapLogicPlayer::transfer_fee_deduct(Message* msg)
{
	static int transfer_item = CONFIG_INSTANCE->const_set("transfer_item");
	MSG_DYNAMIC_CAST_RETURN(Proto31400501*, request, -1);

	int ret = 0;
	if (this->vip_free_transfer() == false)
	{
		ret = this->pack_remove(ITEM_MAP_TRANSFER, transfer_item, 1);
	}

	if (ret != 0)
	{
		Money need_money;
		UpgradeAmountInfo amount_info(transfer_item, 1);

		ret = amount_info.total_money(need_money, this->own_money());
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRANSFER_POINT,
				ERROR_PACKAGE_GOLD_AMOUNT);

		SerialObj money_serial(SUB_MONEY_BUY_TRANSFER, amount_info.buy_item_,
				amount_info.buy_amount_);

		ret = this->pack_money_sub(need_money, money_serial);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRANSFER_POINT, ret);
	}

	Proto30400501 result;
	result.set_oper_result(ret);
	result.set_scene_id(request->scene_id());
	result.set_pixel_x(request->pixel_x());
	result.set_pixel_y(request->pixel_y());
	result.set_client_type(request->client_type());
	return this->send_to_map_thread(result);
}

int MapLogicPlayer::notify_logic_sync_fight_property(int type)
{
	Proto30102102 sync_info;
	sync_info.set_type(type);

	return this->monitor()->dispatch_to_logic(this, &sync_info);
}

int MapLogicPlayer::sync_fight_property_to_map(const int enter_type)
{
	//装备
	this->sync_equip_prop_to_map(enter_type);

	//图鉴
	this->refresh_illustration_prop(enter_type);

	//称号
	{
		IntMap prop_map;
		GameCommon::init_property_map(prop_map);
		this->calc_label_total_prop(prop_map);
		this->refresh_fight_property(BasicElement::LABEL, prop_map, enter_type);
	}

	// VIP
	{
		IntMap prop_map;
		GameCommon::init_property_map(prop_map);
		this->calc_vip_prop(prop_map);
		this->refresh_fight_property(BasicElement::VIP, prop_map, enter_type);
	}

	//婚姻
	this->refresh_wedding_prop(enter_type);
	return 0;
}

int MapLogicPlayer::pack_bill_sell(const int item_id, const int amount, Money &money)
{
	const Json::Value &item_config = CONFIG_INSTANCE->item(item_id);
	JUDGE_RETURN(item_config != Json::nullValue, -1);
	JUDGE_RETURN(item_config.isMember("sell"), -1);
	const Json::Value &sell_json = item_config["sell"];
	JUDGE_RETURN(sell_json.isMember("type") && sell_json.isMember("price"), -1);
	if (sell_json["type"].asInt() == 3)
	{
		money.__copper += amount * sell_json["price"].asInt();
	}
	else if (sell_json["type"].asInt() == 3)
	{
		money.__bind_copper += amount * sell_json["price"].asInt();
	}
	return 0;
}

int MapLogicPlayer::request_pack_item(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400202*, request, -1);

    const int TYPE_USE_ITEM = 1/*, TYPE_LOCK_ITEM = 2, TYPE_UNLOCK_ITEM = 3, TYPE_USE_LOCK_ITEM = 4*/, TYPE_INSERT_ITEM = 5;
    request->set_error(0);
    if (request->type() == TYPE_USE_ITEM)
    {
    	//自动购买物品
    	if (request->auto_buy() == 1)
    	{
    		const ProtoItem &item = request->item(0);
    		GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);

    		Int64 gold = 0;
    		UpgradeAmountInfo use_info(item.id(), item.amount(), package->count_by_id(item.id()));
    		if (use_info.buy_amount_ > 0)
    		{
    			Money need_money;
    			int ret = use_info.total_money(need_money, this->own_money());
    			if (ret != 0)
    			{
    				request->set_error(ERROR_PACKAGE_GOLD_AMOUNT);
    			}
    			else
    			{
    				SerialObj money_serial(SUB_MONEY_PESCORT_CAR, use_info.buy_item_,
    						use_info.buy_amount_);

    				ret = this->pack_money_sub(need_money, money_serial);

    				gold = need_money.__gold;
    				request->set_error(ret);
    			}
    		}
    		this->pack_remove(request->serial(), item.id(), item.amount());

			Proto30100409 serial_info;
			serial_info.set_gold(gold);

			ProtoThreeObj* info = serial_info.mutable_other_info();
			info->set_id(item.id());
			info->set_value(item.amount());
			this->monitor()->dispatch_to_logic(this, &serial_info);
    	}
    	else
    	{
    		for (int i = 0; i < request->item_size(); ++i)
    		{
    			const ProtoItem &item = request->item(i);
    			if (this->pack_count(item.id()) < item.amount())
    			{
    				request->set_error(ERROR_PACKAGE_ITEM_AMOUNT);
    				break;
    			}
    		}

    		if (request->error() == 0)
    		{
    			for (int i = 0; i < request->item_size(); ++i)
    			{
    				const ProtoItem &item = request->item(i);
    				this->pack_remove(request->serial(), item.id(), item.amount());

    				Proto30100409 serial_info;
    				serial_info.set_gold(0);
    				ProtoThreeObj* info = serial_info.mutable_other_info();
    				info->set_id(item.id());
    				info->set_value(item.amount());

    				this->monitor()->dispatch_to_logic(this, &serial_info);
    			}
    		}
    	}
    }
    else if (request->type() == TYPE_INSERT_ITEM)
    {
        if (this->pack_left_capacity() < request->item_size())
        {
//            request->set_error(ERROR_PACKAGE_NO_CAPACITY);

            // send mail
            MailInformation *mail_info = GameCommon::create_sys_mail(FONT_NO_PACKAGE);
            for (int i = 0; i < request->item_size(); ++i)
            {
            	const ProtoItem& proto_item = request->item(i);
                mail_info->add_goods(proto_item.id(), proto_item.amount(), proto_item.bind());
            }

            GameCommon::request_save_mail(this->role_id(), mail_info);
        }
        else
        {
            for (int i = 0; i < request->item_size(); ++i)
            {
                this->pack_insert(request->serial(), request->item(i));
            }
        }
    }
    else
    {
        request->set_error(ERROR_CLIENT_OPERATE);
    }

    int recogn_type = request->recogn() / 100000;
    if (recogn_type == MSG_TYPE_INNER_TO_MAP)
    {
        this->monitor()->process_inner_map_request(this->role_id(), *request);
    }
    else if (recogn_type == MSG_TYPE_INNER_TO_LOGIC)
    {
        this->monitor()->dispatch_to_logic(this, request);
    }

    return 0;
}

int MapLogicPlayer::request_sync_map_skill(int skill_id, int skill_level)
{
    Proto30400017 sync_req;
    ProtoSkill *proto_skill = sync_req.mutable_skill();
    proto_skill->set_skill_id(skill_id);
    proto_skill->set_skill_level(skill_level);
    return this->send_to_map_thread(sync_req);
}


int MapLogicPlayer::sync_info_from_map(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400106*, request, -1);

	int prev_level = this->role_detail_.__level;
	this->role_detail_.__level 		= request->lvl();
	this->role_detail_.__scene_id	= request->scene_id();
	this->role_detail_.__fight_force= request->fight_force();
	JUDGE_RETURN(prev_level < this->role_detail_.__level, 0);

	this->refresh_equip_prop_to_map();
	this->mount_handle_player_levelup();
	this->spool_handle_player_levelup();
	this->fashion_handle_player_levelup();
	this->transfer_handle_player_levelup();
	this->uplevel_change_task();

//	this->fetch_login_reward_flag();
	this->level_upgrade(prev_level, this->role_detail_.__level);

	//level achieve
	this->update_achieve_info(GameEnum::ROLE_LEVEL, this->role_detail_.__level);
	this->exp_restore_lvl_up(this->role_detail_.__level);

	return this->send_to_map_thread(*request);
}

int MapLogicPlayer::level_upgrade(const int prev_level, const int level)
{
	this->new_sync_add_illus(level);
	this->notify_offline_rewards(level);
	this->notify_player_welfare_status(this);
	this->MLTotalRecharge::notify_total_recharge_info();
    this->check_pa_event_when_level_up();
    this->refresh_magic_weapon_property();
	this->task_listen_lvl_up(level);
	return 0;
}

int MapLogicPlayer::process_novice_level_up(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400910 *, request, -1);
    this->role_detail_.__level = request->level();
    return 0;
}

int MapLogicPlayer::notify_player_welfare_status( EntityCommunicate *p)
{
	MapLogicPlayer *player = dynamic_cast<MapLogicPlayer*>(p);
	if(player == NULL)
	{
		MSG_USER("MapLogicPlayer::notify_player_welfare_status(), dynamic_cast error, p:0x%X", p);
		return -1;
	}

	Proto81401401 proto_msg;
	Proto51401421 res;
	player->fetch_display_welfare_elements(0, &res);

	std::set<string> welfare_elements;
	for (int i = 0; i < res.keys_size(); i++)
	{
		welfare_elements.insert(res.keys(i));
	}

	if (welfare_elements.count("sign") > 0)
	{
		player->set_notify_msg_check_in_info(&proto_msg);
	}

	if (welfare_elements.count("exp_restore") > 0)
	{
		player->set_notify_msg_exp_restore_info(&proto_msg);
	}

	return player->respond_to_client(ACTIVE_NOTIFY_WELFAREINFO, &proto_msg);
}

int MapLogicPlayer::request_task_rename(Message *msg)
{
    return 0;
}

int MapLogicPlayer::after_update_player_name(Transaction *transaction)
{
    return 0;
}

int MapLogicPlayer::set_after_role_name(const string& src_name)
{
    this->role_detail_.set_name(src_name);
    this->role_detail_.change_name_tick_ = ::time(NULL);
    return 0;
}

int MapLogicPlayer::handle_after_role_rename()
{
	// notify map thread, logic server, gate server, chat server;
	Proto31400020 req;
	req.set_role_id(this->role_id());
	req.set_role_name(this->role_detail_.__src_name);

	this->send_to_map_thread(req);
	this->monitor()->dispatch_to_logic(this, &req);
	this->monitor()->dispatch_to_chat(this, &req);

    return 0;
}

int MapLogicPlayer::set_after_role_sex()
{
	this->role_detail_.set_sex();
	this->role_detail_.change_sex_tick_ = ::time(NULL);
	return 0;
}

int MapLogicPlayer::handle_after_role_sex()
{
	this->send_to_map_thread(INNER_SYNC_ROLE_SEX);
	this->monitor()->dispatch_to_logic(this, INNER_SYNC_ROLE_SEX);
//	this->monitor()->dispatch_to_chat(this, INNER_SYNC_ROLE_SEX);
	return 0;
}

int MapLogicPlayer::sync_hook_info_to_logic(void)
{
	return 0;
}

int MapLogicPlayer::fetch_random_rename(Message *msg)
{
    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    MMORole::update_sex_condition(data_map, this->role_detail().__sex, this->role_detail().__account);

    if (TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), TRANS_FETCH_RAND_NAME,
    		data_map, this->monitor()->logic_unit()) != 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
        return this->respond_to_client_error(RETURN_FETCH_RAND_NAME, ERROR_SERVER_INNER);
    }
    return 0;
}

int MapLogicPlayer::after_fetch_random_rename(Transaction *transaction)
{
	if (transaction->is_failure())
	{
		this->respond_to_client_error(RETURN_FETCH_RAND_NAME, transaction->detail().__error);
		transaction->rollback();
		return 0;
	}

	MongoDataMap *data_map = transaction->fetch_mongo_data_map();
	MongoData *mongo_data = 0;
	if  (data_map->find_data(Role::COLLECTION, mongo_data) != 0)
	{
		this->respond_to_client_error(RETURN_FETCH_RAND_NAME, ERROR_SERVER_INNER);
		transaction->rollback();
		return -1;
	}

	std::string name = MMORole::fetch_role_name(mongo_data->data_bson());
	Proto51400335 respond;
	respond.set_name(name);
	return this->respond_to_client(RETURN_FETCH_RAND_NAME, &respond);
}

// TODO: remote param `test_lvl`
int MapLogicPlayer::fetch_display_welfare_elements(Message *msg, Message *res, int test_lvl)
{
//	DYNAMIC_CAST_NOTIFY(Proto11401421 *, request, msg, RETURN_DISPLAY_WELFARE_ELEMENTS);
	DYNAMIC_CAST_RETURN(Proto51401421 *, respond, res, -1);
	const Json::Value &elements_json = CONFIG_INSTANCE->welfare_elements_json();
	CONDITION_NOTIFY_RETURN(elements_json != Json::Value::null,
			RETURN_DISPLAY_WELFARE_ELEMENTS, ERROR_CONFIG_ERROR);

	Json::Value::Members keys = elements_json.getMemberNames();
	int create_days = this->role_create_days();
	int role_level = test_lvl > 0 ? test_lvl : this->role_level();
	for(Json::Value::Members::iterator iter = keys.begin(); iter != keys.end(); iter++)
	{
		if (GameCommon::check_welfare_open_condition(*iter, role_level, create_days) != 0)
			continue;
		respond->add_keys(*iter);
	}

	MSG_DEBUG("role:%ld, Proto51401421: %s", this->role_id(),
			respond->Utf8DebugString().c_str());

	return 0;
}

int MapLogicPlayer::request_force_exit_system(int scene_id)
{
	Proto30400007 inner;
	inner.set_scene_id(scene_id);
	return this->send_to_map_thread(inner);
}

int MapLogicPlayer::request_enter_main_town(Message *msg)
{
	return 0;
}

int MapLogicPlayer::callback_after_exchange_copper(const int recogn)
{
	if (recogn != this->repeat_req_recogn_)
		return -1;

    this->repeat_req_recogn_ = 0;
	auto_ptr<Message> req = this->repeat_req_;
	switch (recogn)
	{
	case RETURN_ENTER_MAIN_TOWN:
		this->request_enter_main_town(req.get());
        return 0;
	default:
		break;
	}
	return -1;
}

void MapLogicPlayer::set_repeat_req(const int recogn, const Message *req)
{
	this->repeat_req_ = auto_ptr<Message>();
	this->repeat_req_recogn_ = recogn;

	if (req == 0)
		return;

	this->repeat_req_ = auto_ptr<Message>(create_message(req->GetTypeName()));
	this->repeat_req_->CopyFrom(*req);
}

int MapLogicPlayer::request_function_need_gold(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto11400152 *, request, msg, -1);

    Money cost;
    cost.unserialize(request->mutable_money());
    int item_list_size = request->item_list_size(), item_id = 0, item_amount = 0;
    for (int i = 0; i < item_list_size; ++i)
    {
        const ProtoItem &proto_item = request->item_list(i);
        item_id = proto_item.id();
        item_amount = proto_item.amount();
        this->calc_item_need_money(item_id, item_amount, cost);
    }
    
    int need_gold = 0, exchange_copper = 0;
    GameCommon::check_copper_adjust_exchange_gold(cost, this->own_money(), &need_gold, &exchange_copper);
    cost.__gold += need_gold;

    Proto51400152 respond;
    respond.set_need_gold(cost.__gold);
    return this->respond_to_client(RETURN_FUNCTION_NEED_GOLD, &respond);
}

int MapLogicPlayer::return_activity_cost_item(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400033*, request, -1);

	ItemObjVec cost_item;
	Money money;
	for (int i = 0; i < request->cost_item_size(); ++i)
	{
		ItemObj item;
		item.unserialize(request->cost_item(i));
		Money sub_mon = GameCommon::money_item_to_money(item);
		if (GameCommon::validate_money(sub_mon) == true)
			money += sub_mon;
		else
			cost_item.push_back(item);
	}

	int cash_coupon_id = CONFIG_INSTANCE->const_set("cash_coupon_id");
	int cash_coupon_money = CONFIG_INSTANCE->const_set("cash_coupon_money");
	int cash_coupon_use = request->cash_coupon_use();
	if (cash_coupon_use > 0)
	{
		PackageItem *pack_item = this->pack_find_by_id(cash_coupon_id);
		CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_OPEN_ACT_BUY,
				ERROR_PACKAGE_ITEM_AMOUNT);
		CONDITION_NOTIFY_RETURN(pack_item->__amount >= cash_coupon_use,
				RETURN_OPEN_ACT_BUY, ERROR_PACKAGE_ITEM_AMOUNT);

		money.__gold -= cash_coupon_use * cash_coupon_money;
		money.__gold = money.__gold >= 0 ? money.__gold : 0;
	}

	CONDITION_NOTIFY_RETURN(cost_item.empty() == false || GameCommon::validate_money(money) == true
			|| cash_coupon_use > 0, RETURN_OPEN_ACT_BUY, ERROR_CONFIG_NOT_EXIST);

	if (GameCommon::validate_money(money) == true)
	{
		SerialObj obj(SUB_MONEY_RETURN_ACTIVITY_BUY, request->first_index(), request->second_index());
		int ret = this->pack_money_sub(money, obj);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_OPEN_ACT_BUY, ret);
	}

	if (cash_coupon_use > 0)
	{
		SerialObj obj(SUB_MONEY_RETURN_ACTIVITY_BUY, request->first_index(), request->second_index());
		int ret = this->pack_remove(obj, cash_coupon_id, cash_coupon_use);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_OPEN_ACT_BUY, ret);
	}

	if (cost_item.empty() == false)
	{
		GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
		JUDGE_RETURN(package != NULL, -1);

		SerialObj obj(ITEM_RETURN_ACTIVITY_BUY, request->first_index(), request->second_index());
		int ret = this->pack_remove(package, obj, cost_item);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_OPEN_ACT_BUY, ret);
	}

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::wedding_buy_treasures(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400049*, request, -1);

	int gold = CONFIG_INSTANCE->wedding_treasures(1)["cost_gold"].asInt();
	Money cost(gold);

	if (GameCommon::validate_money(cost) == true)
	{
		SerialObj obj( SUB_MONEY_BUY_WEDDING_TREASURES);
		int ret = this->pack_money_sub(cost, obj);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_BUY_WEDDING_TREASURES, ret);
	}

	return MAP_MONITOR->dispatch_to_logic(this, msg);
}

int MapLogicPlayer::refresh_wedding_prop(int enter_type)
{
    for (int type = WED_RING; type < WED_TYPE_END; ++type)
    {
    	int level = this->role_detail_.wedding_self_[type];
    	JUDGE_CONTINUE(level > 0);

    	int pro_type = type - WED_RING + BasicElement::WED_RING;
    	const Json::Value &pro_json = CONFIG_INSTANCE->wedding_property(type, level);
    	JUDGE_CONTINUE(pro_json != Json::Value::null);

        IntMap prop_map;
    	prop_map[GameEnum::ATTACK] += pro_json["attack"].asInt();
    	prop_map[GameEnum::DEFENSE] += pro_json["defence"].asInt();
    	prop_map[GameEnum::BLOOD_MAX] += pro_json["health"].asInt();
    	prop_map[GameEnum::HIT] += pro_json["hit"].asInt();
    	prop_map[GameEnum::AVOID] += pro_json["dodge"].asInt();
    	prop_map[GameEnum::CRIT] += pro_json["crit"].asInt();
    	prop_map[GameEnum::TOUGHNESS] += pro_json["toughness"].asInt();

    	int side_level = this->role_detail_.wedding_side_[type];
        const Json::Value &min_json = CONFIG_INSTANCE->wedding_property(type, std::min(side_level, level));

        int inc_scale = 0;
    	if (min_json.isMember("order_scale"))
    	{
    		inc_scale += min_json["order_scale"].asInt();
    	}

    	if (min_json.isMember("level_scale"))
    	{
    		inc_scale += min_json["level_scale"].asInt();
    	}

    	if (inc_scale > 0)
    	{
    		double scale = GameCommon::div_percent(inc_scale);
        	prop_map[GameEnum::ATTACK] *= (1 + scale);
        	prop_map[GameEnum::DEFENSE] *= (1 + scale);
        	prop_map[GameEnum::BLOOD_MAX] *= (1 + scale);
        	prop_map[GameEnum::HIT] *= (1 + scale);
        	prop_map[GameEnum::AVOID] *= (1 + scale);
        	prop_map[GameEnum::CRIT] *= (1 + scale);
        	prop_map[GameEnum::TOUGHNESS] *= (1 + scale);
    	}

        this->refresh_fight_property(pro_type, prop_map, enter_type);
    }
    return 0;
}

int MapLogicPlayer::update_player_wedding_info(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403102*, request, -1);

	for (int i = 0; i < request->wed_info_size(); ++i)
	{
		const ProtoWeddingDetail& info = request->wed_info(i);
		int type = WED_RING + i;
		this->role_detail_.wedding_self_[type] = info.level();
		this->role_detail_.wedding_side_[type] = info.order();
	}

	this->role_detail_.__wedding_id = request->wedding_id();
	return this->refresh_wedding_prop();
}

int MapLogicPlayer::wedding_update_work(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400048*, request, -1);

	for (int i = 0; i < request->item_list_size(); ++i)
	{
		int item_id = request->item_list(i).id();
		int item_num = request->item_list(i).amount();

		int recogn = 0;
		int serial = 0;
		if (request->other_info().id() == 1)
		{
			recogn = RETURN_UPDATE_RING;
			serial = ITEM_UPDATE_WEDDING_RING;
		}
		else
		{
			recogn = RETURN_UPDATE_TREE;
			serial = ITEM_UPDATE_WEDDING_TREE;
		}

		int ret = this->pack_remove(this->find_package(), serial, item_id, item_num);
		CONDITION_NOTIFY_RETURN(ret == 0, recogn, ret);
	}

	return MAP_MONITOR->dispatch_to_logic(this, msg);
}

int MapLogicPlayer::lucky_wheel_activity_cost(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	const ProtoMoney &money = request->money();
	Money cost_mon;
	cost_mon.unserialize(money);
	int recogn = request->recogn();

	CONDITION_NOTIFY_RETURN(GameCommon::validate_money(cost_mon) == true,
			recogn, ERROR_PACKAGE_MONEY_AMOUNT);

	SerialObj obj;
	obj.unserialize(request->obj());
	int ret = this->pack_money_sub(cost_mon, obj);
	CONDITION_NOTIFY_RETURN(ret == 0, recogn, ret);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::lucky_wheel_activity_reset(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400045*, request, -1);

	const ProtoMoney &money = request->money();
	Money cost_mon;
	cost_mon.unserialize(money);

	CONDITION_NOTIFY_RETURN(GameCommon::validate_money(cost_mon) == true,
			RETURN_RESET_LUCKY_WHEEL, ERROR_SERVER_INNER);

	SerialObj obj;
	obj.unserialize(request->obj());
	int ret = this->pack_money_sub(cost_mon, obj);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_RESET_LUCKY_WHEEL, ret);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::may_activity_buy(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);

	int ret = 0;
	if (request->money() > 0)
	{
		Money money(request->money());
		SerialObj obj(SUB_MONEY_MAY_ACT_BUY, request->first_index(), request->second_index());
		ret = this->pack_money_sub(money, obj);
	}
	else if (request->item_id() > 0)
	{
		SerialObj obj(ITEM_MAY_ACT_COST, request->first_index(), request->second_index());
		ret = this->pack_remove(obj, request->item_id(), request->amount());

		if (request->amount() > 0 && ret == 0)
		{
			this->check_daily_run_pa_event_cancle();
		}
	}
	else
	{
		ret = ERROR_CONFIG_ERROR;
	}

	request->set_ret(ret);
	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::fetch_item_amount_info(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400061*, request, -1);
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, -1);



	const ProtoSerialObj &obj_buf = request->obj();
	SerialObj obj(obj_buf.serial_type(), obj_buf.sub_type(), obj_buf.serial_value());

	const ProtoItemId &item_id_need_info = request->item_info_list(1);
//	int need_amount = item_id_need_info.amount();
//	int have_amount = this->pack_count(item_id_need_info.id());
//	CONDITION_NOTIFY_RETURN(have_amount >= need_amount, RETURN_GODDESS_BLESS_EXCHANGE, ERROR_PACKAGE_GOODS_AMOUNT);
	int ret = this->pack_remove(obj, item_id_need_info.id(), item_id_need_info.amount());
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_GODDESS_BLESS_EXCHANGE, ret);

	const ProtoItemId &item_id_info = request->item_info_list(0);
	this->insert_package(obj, item_id_info.id(), item_id_info.amount(), GameEnum::ITEM_BIND);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::goddess_bless_reward(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400062*, request, -1);
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, 0);

	for (int i = 0; i < request->item_info_list_size(); ++i)
	{
		const ProtoItemId &item_info_buf = request->item_info_list(i);

		if (item_info_buf.id() > 0)
		{
			const ProtoSerialObj &obj_buf = request->obj();
			this->insert_package(SerialObj(obj_buf.serial_type(), obj_buf.sub_type(), obj_buf.serial_value()),
					item_info_buf.id(), item_info_buf.amount(), GameEnum::ITEM_BIND);
		}
	}

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::check_daily_run_pa_event_cancle()
{
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, 0);

	int red_point = 0;
	bool find_flag = false;

	GameConfig::ConfigMap& conf_map = CONFIG_INSTANCE->daily_run_item_map();
	for (GameConfig::ConfigMap::iterator iter = conf_map.begin();
			iter != conf_map.end(); ++iter)
	{
		const Json::Value &conf = *(iter->second);
		int item_id = conf["item_id"].asInt();
		red_point = conf["red_point"].asInt();
		int count = this->pack_count(package, item_id);
		JUDGE_CONTINUE(count > 0);

		find_flag = true;
	}

	if (find_flag == false)
	{
		this->check_pa_event_daily_run_item(red_point, 0);
	}
	else
	{
		this->check_pa_event_daily_run_item(red_point, 1);
	}

	return 0;
}

int MapLogicPlayer::process_skill_level_up(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31402204*, request, -1);

    uint skill_id = request->skill_id();
    uint skill_level = request->skill_level();

    int fun_id = GameCommon::fetch_skill_fun_bit(skill_id);
    int fun_type = GameCommon::fetch_skill_fun_type(fun_id);

    switch (fun_type)
    {
    case GameEnum::SKILL_FUN_PASSIVE:
    {
    	int ret = this->skill_upgrade_use_goods(skill_id, skill_level);
    	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SKILL_LEVEL_UP, ret);

    	this->request_sync_map_skill(skill_id, skill_level + 1);
    	this->check_pa_event_role_pass_skill_a(skill_id, skill_level + 2);
    	break;
    }
    case GameEnum::SKILL_FUN_MOUNT:
    {
    	MountDetail& mount_detail = this->mount_detail_by_skill(fun_id);
    	CONDITION_NOTIFY_RETURN(mount_detail.skill_map_.count(skill_id) > 0,
    			RETURN_SKILL_LEVEL_UP, ERROR_PLAYER_LEVEL_LIMIT);

    	FighterSkill* skill = mount_detail.skill_map_[skill_id];
    	skill_level = skill->__level;

    	int ret = this->skill_upgrade_use_goods(skill_id, skill_level);
    	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SKILL_LEVEL_UP, ret);

    	mount_detail.add_new_skill(skill_id, skill_level + 1);
    	this->notify_player_skill(skill);
    	this->notify_mount_shape(true, mount_detail.type_);
    	this->refresh_notify_mount_info(mount_detail.type_);
    	this->check_pa_event_mount_skill(mount_detail.type_);
    	this->check_mount_skill_achieve(mount_detail.type_);
    	this->record_mount(MOUNT_SER_SKILL, mount_detail.type_);
    	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_MOUNT);

    	break;
    }
    }

    return 0;
}

int MapLogicPlayer::process_send_flower(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31101605 *, request, msg, -1);
    int item_id = request->item_id(), item_num = request->item_num();

    if (request->auto_buy())
    {
    	UpgradeAmountInfo use_info(item_id, item_num, this->find_package()->count_by_id(item_id));
		if (use_info.buy_amount_ > 0)
		{
			Money need_money;
			int ret = use_info.total_money(need_money, this->own_money());
			CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PRESENT_FLOWER, ret);

			{
				SerialObj money_serial(SUB_MONEY_BUY_FLOWER, use_info.buy_item_,
						use_info.buy_amount_);

				ret = this->pack_money_sub(need_money, money_serial);
				CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PRESENT_FLOWER, ret);
			}
		}
		int result = 0;
		if (use_info.have_amount_ > 0) result = this->pack_remove(ITEM_FLOWER, item_id, use_info.have_amount_);
		CONDITION_NOTIFY_RETURN(result == 0, RETURN_PRESENT_FLOWER, result);
    }
    else
    {
    	int result = this->pack_remove(ITEM_FLOWER, item_id, item_num);
    	CONDITION_NOTIFY_RETURN(result == 0, RETURN_PRESENT_FLOWER, result);
    }

	return MAP_MONITOR->dispatch_to_logic(this, msg);
}

int MapLogicPlayer::process_wedding_check_pack(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31101602 *, request, msg, -1);

    int res_recogn = request->res_recogn(), wedding_type = request->wedding_type();
//    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding_base();

    /*
    //消耗婚戒
    MapLogicPlayer* partner_1 = MAP_MONITOR->find_logic_player(request->role_list(0));
    MapLogicPlayer* partner_2 = MAP_MONITOR->find_logic_player(request->role_list(1));
    int ring_item = wedding_json["wedding_ring"].asInt();
    int res_1 = partner_1->pack_remove(SerialObj(SUB_MONEY_WEDDING, wedding_type), ring_item, 1);
    int res_2 = partner_2->pack_remove(SerialObj(SUB_MONEY_WEDDING, wedding_type), ring_item, 1);

    CONDITION_NOTIFY_RETURN(res_1 == 0 && res_2 == 0, res_recogn, ERROR_TALISMAN_NOT_FIND_ACTIVE);
    */

    CONDITION_NOTIFY_RETURN(0 < wedding_type && wedding_type <= wedding_json["wedding_type_num"].asInt(), res_recogn, ERROR_CONFIG_NOT_EXIST);

//    const Json::Value &wedding_type_json = wedding_json["wedding_type"][wedding_type - 1];
//    const Json::Value &money_json = wedding_type_json["money"];
    Money cost;
//    cost.__bind_copper = money_json[0u].asInt();
//    cost.__copper = money_json[1u].asInt();
    cost.__bind_gold = wedding_json["wedding_cost"][wedding_type - 1][0u].asInt();
    cost.__gold = wedding_json["wedding_cost"][wedding_type - 1][1u].asInt();

//    PROCESS_AUTO_BIND_COPPER_EXCHANGE_AUTO_REPEAT(cost, res_recogn, 0, request);
    GameCommon::adjust_money(cost, this->own_money());
    if (cost.__bind_copper > 0 || cost.__copper > 0 || cost.__bind_gold > 0 || cost.__gold > 0)
    {
        CONDITION_NOTIFY_RETURN(this->validate_money(cost) == true, res_recogn, ERROR_PACKAGE_MONEY_AMOUNT);
        this->pack_money_sub(cost, SerialObj(SUB_MONEY_WEDDING, wedding_type));
    }

    return this->monitor()->dispatch_to_logic(this, request);
}

int MapLogicPlayer::process_divorce_check_pack(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31101603 *, request, -1);
    /*
    const Json::Value &divorce_json = CONFIG_INSTANCE->wedding()["divorce"];
    const Json::Value &money_json = divorce_json["money"];
    Money cost;
    cost.__bind_copper = money_json[0u].asInt();
    cost.__copper = money_json[1u].asInt();
    cost.__bind_gold = money_json[2u].asInt();
    cost.__gold = money_json[3u].asInt();

    PROCESS_AUTO_BIND_COPPER_EXCHANGE_AUTO_REPEAT(cost, RETURN_DIVORCE, 0, request);
    GameCommon::adjust_money(cost, this->own_money());
    if (cost.__bind_copper > 0 || cost.__copper > 0 || cost.__bind_gold > 0 || cost.__gold > 0)
    {
        CONDITION_NOTIFY_RETURN(this->validate_money(cost) == true, RETURN_DIVORCE, ERROR_PACKAGE_MONEY_AMOUNT);
        this->pack_money_sub(cost, SerialObj(SUB_MONEY_DIVORCE));
    }
    */

    this->remove_wedding_label_by_divorce();

    return this->monitor()->dispatch_to_logic(this, request);
}

int MapLogicPlayer::process_keepsake_upgrade_check_pack(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31101604 *, request, msg, -1);

    int keepsake_id = request->keepsake_id(),
        keepsake_level = request->keepsake_level(),
        keepsake_sublevel = request->keepsake_sublevel();

    const Json::Value &wedding_item_json = CONFIG_INSTANCE->wedding_item(keepsake_id);
    const Json::Value &cost_item_json = wedding_item_json["cost_item"];

    const Json::Value *json = NULL;
    for (uint i = 0; i < cost_item_json.size(); ++i)
    {
        if (cost_item_json[i][0u].asInt() == keepsake_level &&
                cost_item_json[i][1u].asInt() == keepsake_sublevel)
        {
            json = &(cost_item_json[i]);
            break;
        }
    }
    JUDGE_RETURN(json != NULL, 0);

    ItemObj item_obj;
    std::vector<ItemObj> need_item_list;

    const Json::Value &item_json = *json;
    for (uint i = 4; i < item_json.size(); ++i)
    {
        item_obj.reset();
        item_obj.id_ = item_json[i][0u].asInt();
        item_obj.amount_ = item_json[i][1u].asInt();

		CONDITION_NOTIFY_RETURN(item_obj.amount_ <= this->pack_count(item_obj.id_), RETURN_KEEPSAKE_UPGRADE, ERROR_PACKAGE_ITEM_AMOUNT);

		if (item_obj.amount_ > 0)
			need_item_list.push_back(item_obj);

    }

    for (std::vector<ItemObj>::iterator iter = need_item_list.begin(); iter != need_item_list.end(); ++iter)
    {
        ItemObj &obj = *iter;
        this->pack_remove(SerialObj(ITEM_KEEPSAKE_UPGRADE, keepsake_level * 1000 + keepsake_sublevel), obj.id_, obj.amount_);
    }

    return this->monitor()->dispatch_to_logic(this, request);
}

int MapLogicPlayer::process_intimacy_award_check(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31101606 *, request, msg, -1);

//    int intimacy = request->intimacy();
    int wedding_type = request->intimacy();
//    const Json::Value &award_label_json = CONFIG_INSTANCE->wedding()["wedding"]["award_label"];

    int label_id = CONFIG_INSTANCE->wedding_base()["wedding_label"][wedding_type].asInt();
    if (this->is_has_label(label_id) == false)
        this->insert_label(label_id);

//    for (uint i = 0; i < award_label_json.size(); ++i)
//    {
//        if (intimacy >= award_label_json[i][0u].asInt())
//        {
//            int label_id = award_label_json[i][1u].asInt();
//            if (this->is_has_label(label_id) == false)
//                this->insert_label(label_id);
//        }
//    }

    return 0;
}

int MapLogicPlayer::process_divorce_clear_label(void)
{
    return this->remove_wedding_label_by_divorce();
}

int MapLogicPlayer::add_act_fish_score(int value)
{
	Proto31403203 request;
	JUDGE_RETURN(value > 0, 0);
	request.set_value(value);
	return this->monitor()->dispatch_to_logic(this, &request);
}

int MapLogicPlayer::remove_wedding_label_by_divorce(void)
{
    // 删除姻缘称号
    const Json::Value &award_label_json = CONFIG_INSTANCE->wedding()["wedding"]["award_label"];
    for (uint i = 0; i < award_label_json.size(); ++i)
    {
        this->delete_label(award_label_json[i][1u].asInt());
    }
    return 0;
}

MongoDataMap *MapLogicPlayer::mongo_datamap()
{
	 return POOL_MONITOR->mongo_data_map_pool()->pop();
}

int MapLogicPlayer::count_of_zhuling()
{
	this->count_of_zhuling_++;
	return 0;
}

int MapLogicPlayer::sync_permission_info(const Json::Value &effect)
{
	/*
	int permission_id = effect["permission_id"].asInt();
	this->role_detail().__permission = permission_id;
	Proto30100120 logic_info;
	Proto30400055 map_info;
	logic_info.set_permission(permission_id);
	map_info.set_permission(permission_id);

	MAP_MONITOR->dispatch_to_logic(this, &logic_info);
	this->send_to_map_thread(map_info);
*/
	return 0;
}

int MapLogicPlayer::process_back_act_reward_item(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400054 *, request, msg, -1);
    int total_use_size = request->bind_item_list_size() + request->unbind_item_list_size();
    CONDITION_NOTIFY_RETURN(total_use_size <= this->pack_left_capacity(), RETURN_DRAW_SINGLE_BACK_ACT, ERROR_PACKAGE_NO_CAPACITY);

    for (int i = 0; i < request->bind_item_list_size(); ++i)
    {
        const ProtoItem &proto_item = request->bind_item_list(i);
        this->insert_package(SerialObj(ADD_FROM_BACK_ACTIVITY, request->act_id()), proto_item.id(), proto_item.amount(), GameEnum::ITEM_BIND);
    }
    for (int i = 0; i < request->unbind_item_list_size(); ++i)
    {
        const ProtoItem &proto_item = request->unbind_item_list(i);
        this->insert_package(SerialObj(ADD_FROM_BACK_ACTIVITY, request->act_id()), proto_item.id(), proto_item.amount(), GameEnum::ITEM_NO_BIND);
    }

    return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::request_tbattle_main_pannel(Message *msg)
{
    Proto30402504 inner_req;
    inner_req.set_tbattle_value(this->fetch_game_resource(GameEnum::ITEM_ID_PRACTICE));

    return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_BATTLE_SCENE_ID, &inner_req);
}

int MapLogicPlayer::process_trvl_reward(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30402505 *, request, msg, -1);


    SerialObj serial_obj(request->serial());
    int ret = this->add_reward(request->reward_id(), serial_obj);
    if (ret != 0)
    {
        MAP_MONITOR->dispatch_to_logic(this, request);
    }
    return 0;
}

int MapLogicPlayer::process_check_money_create_trav_team(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31402702 *, request, msg, -1);

	const Json::Value &peak_base = CONFIG_INSTANCE->travel_peak_base();
	const Json::Value &money_json = peak_base["create_money"];
	Money cost(money_json[0u].asInt(), money_json[1u].asInt());

	CONDITION_NOTIFY_RETURN(GameCommon::validate_money(cost) == true,
			RETURN_CREATE_TRAVEL_TEAM, ERROR_CONFIG_ERROR);

	GameCommon::adjust_money(cost, this->own_money());
	CONDITION_NOTIFY_RETURN(this->validate_money(cost) == true, RETURN_CREATE_TRAVEL_TEAM, ERROR_PACK_BIND_COPPER_AMOUNT);
	this->pack_money_sub(cost, SerialObj(SUB_MONEY_CREATE_TRAVTEAM));

	return this->monitor()->dispatch_to_logic(this, request);
}

int MapLogicPlayer::may_act_buy_many_item(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31403202 *, request, -1);
	int ret = -1;
	//0 正常消耗， 1 正常清除
	int type = request->type();
	if(type == 0)
	{
		if (request->item_id_size() > 0 && request->item_amount_size() > 0)
		{
			for(int i = 0; i < request->item_id_size(); ++i)
			{
				if(this->pack_count(request->item_id(i)) < request->item_amount(i))
				{
					ret = -1;
					break;
				}
				else
					ret = 0;
			}

			if(ret == 0)
			{
				for(int i = 0; i < request->item_id_size(); ++i)
				{
					SerialObj obj(ITEM_MAY_ACT_COST, request->act_id(), request->item_id(i));
					ret = this->pack_remove(obj, request->item_id(i), request->item_amount(i));
				}
			}
		}
	}
	else
	{
		for(int i = 0; i < request->item_id_size(); ++i)
		{
			int cur_count = this->pack_count(request->item_id(i));
			if(cur_count > 0)
			{
				SerialObj obj(ITEM_MAY_ACT_COST, request->act_id(), request->item_id(i));
				ret = this->pack_remove(obj, request->item_id(i), cur_count);
				MSG_DEBUG("pack_remove,item_id:%d, count:%d, ret:%d", request->item_id(i), cur_count, ret);
			}
			else
				ret = -1;
		}
	}
	request->set_ret(ret);
	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MapLogicPlayer::special_box_cost(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403204*, request, -1);

	int ret = ERROR_CONFIG_ERROR;
	if (request->money() > 0)
	{
		Money money(request->money());
		SerialObj obj;
		obj.unserialize(request->serial_obj());
		ret = this->pack_money_sub(money, obj);
	}
	else if(request->item_id() > 0)
	{
		int need_count = request->item_amount();
		SerialObj obj;
		obj.unserialize(request->serial_obj());
		ret = this->pack_remove(obj, request->item_id(), need_count);
	}

	request->set_ret(ret);
	return MAP_MONITOR->dispatch_to_logic(this, request);
}
