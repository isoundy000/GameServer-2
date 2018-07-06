/*
 * MapPlayer.cpp
 *
 * Created on: 2013-01-23 10:35
 *     Author: glendy
 */

#include "MongoDataMap.h"
#include "MongoData.h"
#include "MapPlayerEx.h"
#include "MapPlayer.h"
#include "MapMonitor.h"
#include "TransactionMonitor.h"
#include "SessionManager.h"
#include "ProtoDefine.h"
#include "MMORole.h"
#include "MMOFight.h"
#include "MMOSkill.h"
#include "MMOStatus.h"
#include "MMOScript.h"
#include "MMOEscort.h"
#include "MMOTravel.h"
#include "SerialRecord.h"
#include "MapPlayerScript.h"
#include "AIManager.h"
#include "MMORankPannel.h"
#include "CollectChestsScene.h"
#include "MapMaster.h"
#include "SceneLineManager.h"
#include "MapBeast.h"
//#include "MLLiveness.h"
#include "MapLogicStruct.h"
#include "MapCommunicate.h"
#include "MMOLeague.h"
#include "LeagueMonitor.h"
#include "TipsEnum.h"
#include "BackBrocast.h"


MapPlayer::CachedTimer::CachedTimer(void) : player_(0)
{ /*NULL*/ }

MapPlayer::CachedTimer::~CachedTimer(void)
{ /*NULL*/ }

int MapPlayer::CachedTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int MapPlayer::CachedTimer::handle_timeout(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->player_->is_active() == true, -1);
	JUDGE_RETURN(this->player_->is_enter_scene() == true, -1);
	JUDGE_RETURN(this->player_->is_need_send_message() == true, -1);
    JUDGE_RETURN(this->player_->transfer_flag() == false, -1);

	if (MAP_MONITOR->is_has_travel_scene() == true)
	{
		Time_Value login_tick = PLAYER_MANAGER->map_player_login_tick(this->player_->role_id());
		Time_Value logout_tick = PLAYER_MANAGER->map_player_logout_tick(this->player_->role_id());
		if (login_tick <= logout_tick)
		{
			return -1;
		}
	}

	this->player_->cached_timeout(nowtime);
    return 0;
}

MapPlayer::MapPlayer(void) : 
    gate_sid_(0), client_sid_(0), transfer_flag_(0), copy_offline_(0),
    start_offline_(0), enter_error_(0), offline_enter_(0),
    is_new_role_(false), role_id_(0), src_role_id_(0)
{
	this->is_trade_ = false;
	this->is_login_ = false;
	this->load_first_notify_ = false;
	this->responser_id = 0;
	this->league_escort_car_ = 0;
    this->cached_timer_.player_ = this;
    // 设置玩家内部定时功能的间隔时间
    double timeup_inter[] = {0,
        SAVE_TIMEOUT,   // 定时保存间隔 TIMEUP_SAVE
        FIGHT_TIMEOUT,  // 战斗定时间隔 TIMEUP_FIGHT
        SYNC_KILL_INFO_TIMEOUT,      // 同步击杀信息 TIMEUP_SYNC_KILL
        10,							// TIMEUP_USE_BLOOD
        SYNC_LOGIC_TIMEOUT
    };
    this->cache_tick_.init(MapPlayer::CACHE_END, MapPlayer::TIMEUP_END,
    		timeup_inter, ARRAY_LENGTH(timeup_inter));
}

MapPlayer::~MapPlayer(void)
{
}

void MapPlayer::reset(void)
{
    this->cached_timer_.cancel_timer();
    this->role_id_ = 0;
    this->src_role_id_ = 0;

    this->gate_sid_ = 0;
    this->client_sid_ = 0;

    this->is_login_ = false;
    this->is_new_role_ = false;
    this->transfer_flag_ = false;
    this->transfer_timeout_tick_ = Time_Value::zero;
    this->is_loading_mongo_ = false;
    this->load_mongo_tick_ = Time_Value::zero;

    this->copy_offline_ = false;
    this->start_offline_ = false;

    this->enter_error_ = false;
    this->offline_enter_ = false;
    this->is_trade_ = false;
    this->load_first_notify_ = false;
    this->responser_id = 0;

    this->serial_.reset();
    this->online_.reset();
    this->role_detail_.reset();
    this->hook_detail_.reset();
    this->role_escort_car_ = 0;
    this->league_escort_car_ = 0;

    MapTeamer::reset();
    MapVipPlayer::reset();
    MapSkiller::reset_map_skiller();
    MapMaster::reset_master();
    MapMounter::reset_mount();
    MapSwordPool::reset_spool();
    MapFashion::reset_fashion();
    MapEquiper::reset_shape();
    MapTinyPlayer::reset_tiny();
    MapKiller::reset_map_killer();
    MapWedding::reset_wedding();
    MapEscort::reset();
    MapTransfer::reset_transfer();
}

int MapPlayer::respond_to_client(Block_Buffer *buff)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);
    return this->monitor()->dispatch_to_client_from_gate(this, buff);
}

int MapPlayer::respond_to_client(const int recogn, const Message *msg_proto)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);
	return EntityCommunicate::respond_to_client(recogn, msg_proto);
}

int MapPlayer::respond_to_client_error(const int recogn, const int error, const Message *msg_proto)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);

	if (msg_proto == 0)
	{
		return this->monitor()->dispatch_to_client_from_gate(this, recogn, error);
	}
	else
	{
		return this->monitor()->dispatch_to_client_from_gate(this, msg_proto, error);
	}
}

int MapPlayer::respond_from_broad_client(Block_Buffer *buff)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);
    return this->monitor()->dispatch_to_client_direct(this, buff);
}

int MapPlayer::respond_from_broad_client(const int recogn, const int error, const Message *msg_proto)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);

	if (msg_proto == 0)
	{
		return this->monitor()->dispatch_to_client_direct(this, recogn, error);
	}
	else
	{
		return this->monitor()->dispatch_to_client_direct(this, msg_proto, error);
	}
}

int64_t MapPlayer::entity_id(void)
{
    return this->role_id_;
}

void MapPlayer::set_cur_location(const MoverCoord& coord)
{
	GameFighter::set_cur_location(coord);
	this->update_cur_beast_location();
}

Int64 MapPlayer::role_id(void)
{
    return this->role_id_;
}

Int64 MapPlayer::src_role_id(void)
{
	return this->src_role_id_;
}

Int64 MapPlayer::real_role_id(void)
{
	return is_online_player() ? role_id_ : src_role_id_;
}

int MapPlayer::role_id_low(void)
{
    return (int32_t)this->role_id_;
}

int MapPlayer::role_id_high(void)
{
    return (int32_t)(this->role_id_ >> 32);
}

int MapPlayer::copy_offline(void)
{
	return this->copy_offline_;
}

int MapPlayer::start_offline(void)
{
	return this->start_offline_;
}

int MapPlayer::is_online_player(void)
{
	return (!copy_offline_ && !start_offline_);
}

int MapPlayer::is_lrf_change_mode()
{
	return this->role_detail().hickty_id_ > 0;
}

int MapPlayer::transfer_flag(void)
{
	return this->transfer_flag_;
}

Time_Value &MapPlayer::transfer_timeout_tick(void)
{
	return this->transfer_timeout_tick_;
}

void MapPlayer::set_is_loading_mongo(const bool flag)
{
    this->is_loading_mongo_ = flag;
}

bool MapPlayer::is_loading_mongo(void)
{
    return this->is_loading_mongo_;
}

void MapPlayer::set_load_mongo_tick(const Time_Value &nowtime)
{
    this->load_mongo_tick_ = nowtime;
}

Time_Value &MapPlayer::load_mongo_tick(void)
{
    return this->load_mongo_tick_;
}

bool MapPlayer::is_need_save_mongo(void)
{
    return (this->is_online_player() && this->is_loading_mongo_ == false && this->transfer_flag_ == false);
}

const string &MapPlayer::role_name(void)
{
	return this->role_detail_.__name;
}

const char* MapPlayer::name()
{
	return this->role_detail_.name();
}

int MapPlayer::gate_sid(void)
{
    return this->gate_sid_;
//    return this->monitor()->role_to_logic_sid(this->role_id());
}

int MapPlayer::client_sid(void)
{
    return this->client_sid_;
}

int MapPlayer::team_id(void)
{
	if (GameCommon::is_travel_scene(this->scene_id()) == true
			&& GameCommon::is_script_scene(this->scene_id()) == true)
	{
		return this->team_index(GameEnum::TRAVEL_TEAM);
	}
	else
	{
		return this->team_index(GameEnum::NORMAL_TEAM);
	}
}

Int64 MapPlayer::league_id(void)
{
	return this->role_detail_.__league_id;
}

int MapPlayer::fight_career(void)
{
	return this->role_detail_.__career;
}

int MapPlayer::fight_sex(void)
{
	return this->role_detail_.__sex;
}

int MapPlayer::agent_code()
{
    return this->role_detail_.__agent_code;
}

int MapPlayer::market_code()
{
	return this->role_detail_.__market_code;
}

int MapPlayer::client_fetch_max_blood()
{
	if (this->is_lrf_change_mode() == true)
	{
		return this->role_detail_.health_;
	}
	else
	{
		return this->fight_detail_.__blood_total_i(this);
	}
}

void MapPlayer::set_role_id(const int64_t role_id)
{
    this->role_id_ = role_id;
}

void MapPlayer::set_client_sid(const int client_sid)
{
	this->client_sid_ = client_sid;
}

void MapPlayer::set_gate_sid(const int gate_sid)
{
	this->gate_sid_ = gate_sid;
}

void MapPlayer::set_is_login(const bool flag)
{
	this->is_login_ = flag;
}

bool MapPlayer::is_login(void)
{
	return this->is_login_;
}

void MapPlayer::base_sign_in()
{
    Time_Value nowtime = Time_Value::gettimeofday();
    for (int i = 0; i < TIMEUP_END; ++i)
    {
        this->cache_tick_.update_timeup_tick(i, nowtime);
    }

    this->reset_everyday();
    this->request_chat_login();
    this->notify_get_couple_buy();
    this->start_jump_value_time();
    this->cached_timer_.schedule_timer(1);
}

void MapPlayer::reset_everyday()
{
    MapTinyPlayer::reset_tiny_everyday();
    MapKiller::reset_killer_everyday();

    this->send_festival_icon_to_logic();
    this->send_big_act_state_to_logic();

    this->role_detail_.escort_times = 0;
    this->role_detail_.protect_times = 0;
    this->role_detail_.rob_times = 0;
}

int MapPlayer::refresh_player_info(Block_Buffer *buff)
{
    // TODO;
    return -1;
}

int MapPlayer::resign(const int gate_sid, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400004*, request, -1);
    SESSION_MANAGER->update_session(this->gate_sid(), request->session_info().account(),
    		request->session_info().session(), this->role_id());

    this->set_gate_sid(gate_sid);
    this->base_sign_in();

    this->notify_player_login();
    return 0;
}

int MapPlayer::sign_in(const int type)
{
    JUDGE_RETURN(GameFighter::sign_in() == 0, -1);

    if (MAP_MONITOR->bind_player(this->role_id(), dynamic_cast<MapPlayerEx *>(this)) != 0)
    {
        MSG_USER("ERROR bind player %ld %d", this->role_id(), this->gate_sid());
        return -1;
    }

    this->base_sign_in();
    MSG_USER("map player sign in %ld %s %d %d %d", this->role_id(), this->name(),
    		this->level(), this->scene_id(), type);
    return 0;
}

int MapPlayer::enter_scene(const int type)
{
//	if (this->get_escort_detail().car_index_ != 0)
//	{
//		MapLeaguer* player = dynamic_cast<MapLeaguer* >(this);
//		player->insert_escort_speed_buff();
//	}

    if (GameCommon::is_normal_scene(this->scene_id()) == true)
    {
    	this->set_scene_mode(SCENE_MODE_NORMAL);
    	// 玩家移动坐标修正
    	this->adjust_mover_coord();

    	//自动参加寻宝
    	if (this->scene_id() == GameEnum::COLLECT_CHESTS_SCENE_ID
    			&& COLLECTCHESTS_INSTANCE->get_time_info().cur_state_ == GameEnum::ACTIVITY_STATE_START
    			&& this->level() >= CONFIG_INSTANCE->collect_chests_json(
    			COLLECTCHESTS_INSTANCE->get_cycle_id())["open_level"].asInt())
    	{
    		Proto11405001 info;
    		info.set_enter_type(1);
    		if (COLLECTCHESTS_INSTANCE->is_player_have_join(this->role_id()))
    		{
    			info.set_is_reset(0);
    		}
    		else
    		{
    			info.set_is_reset(1);
    		}
    		this->send_to_logic_thread(info);
    	}
    }

    if (type == ENTER_SCENE_LOGIN || this->enter_error_ == true)
    {
    	this->enter_error_ = false;
    	this->load_first_notify_ = true;
    	this->notify_player_login();
    }

    this->role_detail_.__scene_history.insert(this->scene_id());

    if (type != ENTER_SCENE_LOGIN)
    {
    	GameFighter::enter_scene(type);
    }

    Time_Value nowtime = Time_Value::gettimeofday();
    MSG_USER("enter scene %d %ld %s %d, %d %d[%d,%d] now(%d.%06d) %x",
            type, this->role_id(), this->name(), this->level(), this->space_id(),
            this->scene_id(), this->location().pixel_x(), this->location().pixel_y(),
            nowtime.sec(), nowtime.usec(), this->fetch_scene());

    return 0;
}

int MapPlayer::notify_player_login(void)
{
    this->online_.sign_in();
    this->caculate_total_force();
    this->send_festival_icon_to_logic();
    this->send_big_act_state_to_logic();

    //记录战力异常
    for (int i = 0; i < BasicElement::OFFSET_END; ++i)
    {
    	int cur_elem_force = this->fight_detail_.__element_force(i);
    	JUDGE_CONTINUE(this->role_detail_.prev_force_map_.count(i) > 0);

    	int prev_elem_force = this->role_detail_.prev_force_map_[i];
    	JUDGE_CONTINUE(cur_elem_force != prev_elem_force);

		this->record_other_serial(MAIN_EXECEPTION_SERIAL, 3,
				i, cur_elem_force, prev_elem_force);
    }

    Block_Buffer buff;
    this->make_up_login_info(&buff, true);
    this->respond_to_client(&buff);

    MSG_USER("map player %ld %s %d %d[%d,%d,%d]", this->role_id(), this->name(),
    		this->scene_id(), this->force_total_i(), this->fight_detail_.__force_total_i(this),
    		this->fetch_skill_special_force(), this->fetch_mount_special_force());
   return this->respond_to_client(RETURN_START_GAME);
}

int MapPlayer::insert_skill(int skill_id, int skill_level, int notify)
{
	GameFighter::insert_skill(skill_id, skill_level, notify);
	JUDGE_RETURN(notify == true && this->is_enter_scene() == true, -1);

	FighterSkill* skill = NULL;
	JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, -1);

	return this->notify_player_skill(skill);
}

int MapPlayer::die_process(const int64_t fighter_id)
{
    MapPlayerEx* player_ex = this->self_player();
    player_ex->rob_escort(fighter_id);

	this->master_exit_scene();
	this->sync_update_intimacy_by_kill(fighter_id);
	this->sync_fighter_status_to_map_logic(fighter_id);

    Int64 redefine_fighter_id = this->fetch_benefited_attackor_id(fighter_id);
    this->notify_killed_info(redefine_fighter_id);
    this->check_and_add_event_cut(redefine_fighter_id);

	GameFighter::die_process(fighter_id);
	return 0;
}

int MapPlayer::update_fight_property(int type)
{
	this->caculate_total_force();
	this->notify_fight_property(type);

	int diff = this->role_detail_.__fight_force - this->role_detail_.prev_force_;
	JUDGE_RETURN(diff != 0, 0);

	switch (type)
	{
	case BasicElement::STATUS:
	{
		this->record_other_serial(MAIN_EXECEPTION_SERIAL, 2,
				this->fight_detail_.__force_total_i(this),
				this->fetch_skill_special_force(),
				this->fetch_mount_special_force());
		break;
	}
	default:
	{
		JUDGE_BREAK(this->fight_detail_.validate_offset(type) == true);
		this->role_detail_.prev_force_map_[type] = this->fight_detail_.__element_force(type);
	}
	}

	this->cache_tick_.update_cache(MapPlayerEx::CACHE_SYNC_LOGIC_FORCE, true);
	this->record_other_serial(MAIN_FIGHT_FORCE, type, diff,
			this->role_detail_.__fight_force, this->role_detail_.prev_force_);

	return 0;
}

int MapPlayer::update_glamour(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30402401 *, request, msg, -1);
    int item_id = request->item_id(), item_num = request->item_num();

    int inc_glamour = 0;
    const Json::Value &wedding_item_json = CONFIG_INSTANCE->wedding_flower(item_id);
    inc_glamour = wedding_item_json["glamour"].asInt() * item_num;

    this->fight_detail_.__glamour += inc_glamour;
    this->update_fight_property(GameEnum::UT_PROP_GLAMOUR);

    return 0;
//    return MAP_MONITOR->dispatch_to_logic(this, msg);
}

int MapPlayer::notify_fight_property(int type)
{
	JUDGE_RETURN(this->is_enter_scene() == true, 0);

    Proto80400206 respond;
    respond.set_force(this->force_total_i());
    respond.set_level(this->level());

    respond.set_attack(this->fight_detail_.__attack_total_i(this));
    respond.set_defence(this->fight_detail_.__defence_total_i(this));
    respond.set_max_blood(this->client_fetch_max_blood());

    respond.set_hit(this->fight_detail_.__hit_total_i(this)); // 命中
    respond.set_avoid(this->fight_detail_.__avoid_total_i(this)); // 闪避
    respond.set_crit(this->fight_detail_.__crit_total_i(this)); // 暴击
    respond.set_toughness(this->fight_detail_.__toughness_total_i(this));

    int cirt_hurt_rate 	= this->fight_detail_.__crit_hurt_multi_total(this) * 100;
    int damage_rate		= this->fight_detail_.__damage_rate_total(this) * 100;
//    int reduction_rate 	= this->fight_detail_.__reduction_rate_total(this) * 100;
    int reduction_rate 	= this->fetch_reduce_hurt_rate() * 100;

    respond.set_crit_hurt(cirt_hurt_rate);	//暴击伤害率
    respond.set_damage(damage_rate);	//伤害加成率
    respond.set_reduction(reduction_rate);	//伤害减免率
    respond.set_pk(this->killed_info_.pk_value());	//PK值
    respond.set_glamour(this->fight_detail_.__glamour);	//魅力值

    FINER_PROCESS_RETURN(ACTIVE_UPDATE_PROPERTY, &respond);
}

int MapPlayer::handle_player_scene_skill(int type)
{
	const Json::Value& set_conf = this->scene_set_conf();
	JUDGE_RETURN(set_conf.isMember("skill_list") == true, -1);

	IntVec skill_vec;
	GameCommon::json_to_t_vec(skill_vec, set_conf["skill_list"]);

	for (IntVec::iterator iter = skill_vec.begin(); iter != skill_vec.end(); ++iter)
	{
		if (type == 1)
		{
			this->insert_skill(*iter, 1);
		}
		else
		{
			this->remove_skill(*iter, true);
		}
	}

	return 0;
}

int MapPlayer::handle_player_scene_buff(int type)
{
	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	IntVec buff_vec;
	scene->fetch_enter_buff(buff_vec, this->role_id());

	for (IntVec::iterator iter = buff_vec.begin(); iter != buff_vec.end(); ++iter)
	{
		BasicStatus status(*iter);
		JUDGE_CONTINUE(status.__buff_type > 0);
		this->insert_defender_status(this, status);
	}

	return 0;
}

int MapPlayer::exit_scene(const int type)
{
    MSG_USER("player exit scene %ld %s %d %d[%d,%d] prev:%d[%d,%d] %d",
            this->role_id(), this->name(), this->space_id(), this->scene_id(),
            this->location().pixel_x(), this->location().pixel_y(),
            this->prev_scene_id(), this->prev_location().pixel_x(),
            this->prev_location().pixel_y(), type);

    this->handle_special_scene_when_exit();
    // 恢复场景信息
    this->enter_recover_scene_info();

    this->master_exit_scene(type);
    this->killed_info_.attackor_map_.clear();

    switch (type)
    {
    case EXIT_SCENE_LOGOUT:
//    case EXIT_SCENE_TRANSFER:
    {
    	this->master_sign_out(false);
    	break;
    }
    }

//	if (GameCommon::is_protect_exit_remove(this->scene_id()) == true)
//	{
//		this->clean_status(BasicStatus::RELIVE_PROTECT);
//	}

    if (type == EXIT_SCENE_LOGOUT)
    {
        // 退出游戏的逻辑
        this->online_.sign_out();
    }

    return 0;
}


int MapPlayer::handle_special_scene_when_exit()
{
	this->handle_player_scene_skill(0);

//	Escort_detail &item = this->get_escort_detail();
//	if( item.car_index_ > 0 )
//	{
//		this->record_other_serial(ESCORT_SERIAL, SUB_ESCORT_OFFLINE,
//				item.escort_type_, this->role_detail().escort_times);
//	}

	return 0;
}

int MapPlayer::handle_special_scene_when_enter()
{
	this->handle_player_scene_skill(1);
	this->handle_player_scene_buff(1);
	return 0;
}

int MapPlayer::sign_out(const bool is_save_player)
{
#ifndef LOCAL_DEBUG
	MapClientService *svc = 0;
	if (this->monitor()->find_client_service(this->client_sid(), svc) == 0)
	{
		svc->handle_close();
	}
    this->set_client_sid(0);
#endif

    this->cached_timer_.cancel_timer();

    this->master_sign_out(false);

    if(this->is_need_send_message() == false)
    {
    	return -1;
    }

	if (this->is_need_save_mongo() == true && is_save_player == true)
	{
		this->request_save_player(TRANS_LOGOUT_MAP_PLAYER);
	}

	MSG_USER("Map Player sign out save %ld %s %d %d %d %d[%d,%d] %d transfer(%d) online(%d) loading(%d)",
			this->role_id(), this->name(), this->level(), this->space_id(), this->scene_id(),
			this->location().pixel_x(),	this->location().pixel_y(), is_save_player,
			this->transfer_flag(), this->is_online_player(), this->is_loading_mongo());

    return 0;
}

int MapPlayer::request_sync_to_logic(const int cache_type)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);

	Proto30101001 sync_info;
	if(cache_type == MapPlayerEx::CACHE_SYNC_LOGIC_FORCE
			|| this->cache_tick().check_cache(MapPlayerEx::CACHE_SYNC_LOGIC_FORCE) == true)
	{
		sync_info.add_info_type(GameEnum::SYNC_LOGIC_FORCE);
		sync_info.set_force(this->force_total_i());
		sync_info.set_kill_num(this->killed_info_.kill_num_);
		sync_info.set_killing_value(this->killed_info_.killing_value_);

		//同步战力
		this->sync_info_to_map_logic();
		this->cache_tick_.update_cache(MapPlayer::CACHE_SYNC_LOGIC_FORCE, false);
	}

	JUDGE_RETURN(sync_info.info_type_size() > 0, 0);
    return this->monitor()->dispatch_to_logic(this, &sync_info);
}

int MapPlayer::request_save_player(const int recogn)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);

    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    MapPlayerEx* player = dynamic_cast<MapPlayerEx *>(this);

    if (recogn == TRANS_LOGOUT_MAP_PLAYER || this->cache_tick().check_cache(MapPlayerEx::CACHE_BASE) == true)
    {
        MMORole::update_data(player, data_map);
    	MMORankPannel::update_date(player, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_PLAYER || this->cache_tick().check_cache(MapPlayerEx::CACHE_FIGHT) == true)
    {
        MMORole::update_back_role(player, data_map);
        MMOFight::update_data(player, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_PLAYER || this->cache_tick().check_cache(MapPlayerEx::CACHE_SKILL) == true)
    {
        MMOSkill::update_data(player, data_map);
        MMORole::update_copy_player(player, data_map);
        player->record_exception_skill_info();
    }
    if (recogn == TRANS_LOGOUT_MAP_PLAYER || this->cache_tick().check_cache(MapPlayerEx::CACHE_STATUS) == true)
    {
        MMOStatus::update_data(player, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_PLAYER || this->cache_tick().check_cache(MapPlayerEx::CACHE_SCRIPT) == true)
    {
        MMOScript::update_data(player, data_map);
    }
    if (recogn == TRANS_LOGOUT_MAP_PLAYER || this->cache_tick().check_cache(MapPlayerEx::CACHE_ESCORT) == true)
    {
        MMOEscort::update_data(player, data_map);
    }

    if (recogn == TRANS_LOGOUT_MAP_PLAYER)
    {
        BackSerial::update_data(player, data_map);
        MMOTravel::update_data(player, data_map);

        MMOFight::update_tiny_data(player,data_map);
        MMOLeague::save_leaguer_info(player, data_map);
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

int MapPlayer::process_travel_save(const int recogn, MongoDataMap *data_map)
{
    Proto30400054 inner_req;
    inner_req.set_mongo_recogn(recogn);
    inner_req.set_role_id(this->role_id());
    inner_req.set_tick(::time(NULL));

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

MapRoleDetail &MapPlayer::role_detail(void)
{
    return this->role_detail_;
}

LogicOnline &MapPlayer::online(void)
{
    return this->online_;
}

MapSerial &MapPlayer::serial_record(void)
{
    return this->serial_;
}

GameCache &MapPlayer::cache_tick(void)
{
    return this->cache_tick_;
}

int MapPlayer::cached_timeout(const Time_Value &nowtime)
{
#ifndef LOCAL_DEBUG
	if (this->monitor()->is_has_travel_scene() == false)
#endif
	{
		if (this->cache_tick_.check_timeout(MapPlayer::TIMEUP_SYNC_LOGIC, nowtime) == true)
		{
			this->request_sync_to_logic();
			this->cache_tick_.update_timeup_tick(MapPlayer::TIMEUP_SYNC_LOGIC, nowtime);
		}
	}

    if (this->cache_tick_.check_timeout(MapPlayer::TIMEUP_SAVE, nowtime) == true)
    {
        if (this->request_save_player() == 0)
        {
        	this->cache_tick().reset_cache_flag();
        }

        this->cache_tick_.update_timeup_tick(MapPlayer::TIMEUP_SAVE, nowtime);
    }

#ifndef LOCAL_DEBUG
	if (this->monitor()->is_has_travel_scene() == false)
#endif
	{
		if (this->cache_tick_.check_timeout(MapPlayer::TIMEUP_SYNC_KILL, nowtime) == true)
		{
			this->handle_reduce_kill_value();
			this->check_attack_me();

			MapKiller::killer_time_up(nowtime);
			this->cache_tick_.update_timeup_tick(MapPlayer::TIMEUP_SYNC_KILL, nowtime);
		}
	}

	if (this->is_lrf_change_mode() == false)
	{
		this->check_timeout_cont_blood();
		this->check_jump_value_timeout();
		this->check_beast_skill_timeout();
	}

    this->check_travel_timeout();
	return 0;
}

int MapPlayer::time_up(const Time_Value &nowtime)
{
	GameFighter::time_up(nowtime);
	return 0;
}

int MapPlayer::serialize_move(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400103*, msg_proto, -1);

    MoverDetail& mover_detail = this->mover_detail();
    MapRoleDetail& role_detail = this->role_detail();

    Proto30400103 &request = *msg_proto;
    request.set_role_id(this->role_id());
    request.set_scene_id(this->scene_id());
    request.set_toward(mover_detail.__toward);
    request.set_scene_mode(this->scene_mode());
    request.set_space_id(this->space_id());

    request.set_prev_scene_id(this->prev_scene_id());
    request.set_prev_scene_mode(this->prev_scene_mode());
    request.set_prev_space_id(this->prev_space_id());
    request.set_speed_basic(mover_detail.__speed.basic());
    request.set_server_tick(role_detail.server_tick_);
    request.set_combine_tick(role_detail.combine_tick_);

    SaveSceneInfo& save_info = role_detail.__save_scene;
    request.set_s_scene_id(save_info.scene_id_);
    request.set_s_blood(save_info.blood_);
    request.set_s_pk(save_info.pk_state_);

    save_info.coord_.serialize(request.mutable_s_coord());
    mover_detail.__location.serialize(request.mutable_coord());
    mover_detail.__prev_location.serialize(request.mutable_prev_scene_coord());

    for (IntSet::iterator iter = role_detail.__scene_history.begin();
    		iter != role_detail.__scene_history.end(); ++iter)
    {
    	request.add_scene_history(*iter);
    }

    if (MAP_MONITOR->is_has_travel_scene() == false)
    {
    	request.set_drop_act(MAP_MONITOR->festival_icon_type());
    	request.set_is_big_act_time(MAP_MONITOR->is_in_big_act_time());
    }
    else
    {
        request.set_drop_act(role_detail.drop_act_);
        request.set_is_big_act_time(role_detail.is_big_act_time_);
    }

    GameCommon::map_to_proto(request.mutable_prev_force_map(),
    		role_detail.prev_force_map_);
    return 0;
}

int MapPlayer::unserialize_move(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400103*, msg_proto, -1);

    Proto30400103 &request = *msg_proto;
    MoverDetail& mover_detail = this->mover_detail();
    MapRoleDetail& role_detail = this->role_detail();

    mover_detail.__scene_id = request.scene_id();
    mover_detail.__location.unserialize(request.mutable_coord());
    mover_detail.__toward = request.toward();
    mover_detail.__scene_mode = request.scene_mode();
    mover_detail.__space_id = request.space_id();
    mover_detail.__prev_scene_id = request.prev_scene_id();
    mover_detail.__prev_location.unserialize(request.mutable_prev_scene_coord());
    mover_detail.__prev_scene_mode = request.prev_scene_mode();
    mover_detail.__prev_space_id = request.prev_space_id();
    mover_detail.__speed.set_single(request.speed_basic(), BasicElement::BASIC);

    SaveSceneInfo& save_info = role_detail.__save_scene;
    save_info.scene_id_ = request.s_scene_id();
    save_info.coord_.unserialize(request.mutable_s_coord());
    save_info.blood_ = request.s_blood();
    save_info.pk_state_ = request.s_pk();

    role_detail.server_tick_ = request.server_tick();
    role_detail.combine_tick_ = request.combine_tick();
    role_detail.drop_act_ = request.drop_act();
    role_detail.is_big_act_time_ = request.is_big_act_time();

    for (int i = 0; i < request.scene_history_size(); ++i)
    {
    	role_detail.__scene_history.insert(request.scene_history(i));
    }

    GameCommon::proto_to_map(role_detail.prev_force_map_,
    		request.prev_force_map());
    return 0;
}

int MapPlayer::serialize_fight(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400104*, msg_proto, -1);

    Proto30400104 &request = *msg_proto;
    FightDetail &fight_info = this->fight_detail();
    SkillerDetail &skiller_detail = this->skiller_detail();

    request.set_role_id(this->role_id());
    request.set_pk_state(fight_info.__pk_state);
    request.set_camp_id(fight_info.__camp_id);
    request.set_fight_state(fight_info.__fight_state);
    request.set_fight_tick_sec(fight_info.__fight_tick.sec());
    request.set_fight_tick_usec(fight_info.__fight_tick.usec());

    request.set_level(fight_info.__level);
    request.set_experience(fight_info.__experience);
    request.set_blood(fight_info.__blood);
    request.set_magic(fight_info.__magic);
    request.set_angry(fight_info.__angry);
    request.set_jump(fight_info.__jump);
    request.set_glamour(fight_info.__glamour);

    for (SkillMap::iterator iter = fight_info.__skill_map.begin();
    		iter != fight_info.__skill_map.end(); ++iter)
    {
        ProtoSkill *proto_skill = request.add_skill_list();
        iter->second->serialize(proto_skill);
    }

    request.set_cur_rama(skiller_detail.rama_skill_);
    for (IntMap::iterator iter = skiller_detail.rama_skill_list_.begin();
    		iter != skiller_detail.rama_skill_list_.end(); ++iter)
    {
    	ProtoPairObj* obj = request.add_rama_list();
    	obj->set_obj_id(iter->first);
    	obj->set_obj_value(iter->second);
    }

    StatusQueueNode *node = 0;
    BasicStatus *status = 0;
    for (GameStatus::StatusMap::iterator iter = this->status_map().begin();
            iter != this->status_map().end(); ++iter)
    {
        node = iter->second;

        for (size_t i = 0; i < node->__status_list.size(); ++i)
        {
            status = node->__status_list.node(i);

            ProtoSyncStatus *proto_status = request.add_status_list();
            proto_status->set_status(status->__status);
            proto_status->set_value1(status->__value1);
            proto_status->set_value2(status->__value2);
            proto_status->set_value3(status->__value3);
            proto_status->set_value4(status->__value4);
            proto_status->set_value5(status->__value5);
            proto_status->set_check_sec(status->__check_tick.sec());
            proto_status->set_check_usec(status->__check_tick.usec());
            proto_status->set_interval_sec(status->__interval.sec());
            proto_status->set_interval_usec(status->__interval.usec());
            proto_status->set_last_sec(status->__last_tick.sec());
            proto_status->set_last_usec(status->__last_tick.usec());
            proto_status->set_skill_id(status->__skill_id);
            proto_status->set_skill_level(status->__level);
            proto_status->set_attacker(status->__attacker);
            proto_status->set_accumulate(status->__accumulate_tims);
        }
    }

    return 0;
}

int MapPlayer::unserialize_fight(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400104*, msg_proto, -1);

    Proto30400104 &request = *msg_proto;
    FightDetail &fight_info = this->fight_detail();
    SkillerDetail &skiller_detail = this->skiller_detail();

    fight_info.__pk_state = request.pk_state();
    fight_info.__camp_id = request.camp_id();
    fight_info.__fight_state = request.fight_state();
    fight_info.__fight_tick = Time_Value(request.fight_tick_sec(), request.fight_tick_usec());

    fight_info.set_level(request.level());
    fight_info.__experience = request.experience();

    fight_info.__blood = request.blood();
    fight_info.__magic = request.magic();
    fight_info.__angry = request.angry();
    fight_info.__jump  = request.jump();
    fight_info.__glamour = request.glamour();

    for (int i = 0; i < request.skill_list_size(); ++i)
    {
        const ProtoSkill &proto_skill = request.skill_list(i);
        this->insert_skill(proto_skill.skill_id(), proto_skill.skill_level());

    	FighterSkill* skill = NULL;
    	JUDGE_CONTINUE(this->find_skill(proto_skill.skill_id(), skill) == 0);

        skill->unserialize(proto_skill);
    }

    skiller_detail.rama_skill_ = request.cur_rama();
    for (int i = 0; i < request.rama_list_size(); ++i)
    {
    	const ProtoPairObj& obj = request.rama_list(i);
    	skiller_detail.rama_skill_list_[obj.obj_id()] = obj.obj_value();
    }

    StatusQueueNode *node = 0;
    BasicStatus *status = 0;
    for (int i = 0; i < request.status_list_size(); ++i)
    {
        const ProtoSyncStatus &proto_status = request.status_list(i);

        status = this->monitor()->status_pool()->pop();
        JUDGE_CONTINUE(status != NULL);

        status->set_status(proto_status.status());
        status->__value1 = proto_status.value1();
        status->__value2 = proto_status.value2();
        status->__value3 = proto_status.value3();
        status->__value4 = proto_status.value4();
        status->__value5 = proto_status.value5();
        status->__check_tick = Time_Value(proto_status.check_sec(), proto_status.check_usec());
        status->__interval = Time_Value(proto_status.interval_sec(), proto_status.interval_usec());
        status->__last_tick = Time_Value(proto_status.last_sec(), proto_status.last_usec());
        status->__skill_id = proto_status.skill_id();
        status->__level = proto_status.skill_level();
        status->__attacker = proto_status.attacker();
        status->__accumulate_tims = proto_status.accumulate();

        if (this->status_map().find(status->__status, node) != 0)
        {
            node = this->monitor()->status_queue_node_pool()->pop();
            node->set_status(*status);
            node->__check_tick = status->__check_tick;
            if (this->status_map().bind(status->__status, node) != 0)
            {
                this->monitor()->status_pool()->push(status);
                this->monitor()->status_queue_node_pool()->push(node);
                continue;
            }
            this->status_queue().push(node);
        }
        node->__status_list.push(status);
    }

    this->init_level_property();
    this->init_skill_property();
    this->init_skill_buff();
    this->refresh_all_status_property(ENTER_SCENE_TRANSFER);
    return 0;
}

int MapPlayer::serialize_online(Proto30400105 *request)
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

int MapPlayer::unserialize_online(Proto30400105 *request)
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

int MapPlayer::prop_item_level_up()
{
	return this->level_upgrade();
}

int MapPlayer::logic_refresh_fight_property(Message* msg)
{
	static double GAME_PERCENT = GameEnum::DAMAGE_ATTR_PERCENT;
	MSG_DYNAMIC_CAST_RETURN(Proto30400011*, request, -1);

	FightDetail& fight_detail = this->fight_detail_;

	int prev_blood = fight_detail.__blood;
	int prev_magic = fight_detail.__magic;

	int blood_magic_flag = 0;
	int offset = request->offset();
	for (int i = 0; i < request->prop_set_size(); ++i)
	{
		const ProtoPairObj& pair_obj = request->prop_set(i);

		switch(pair_obj.obj_id())
		{
		case GameEnum::SPEED:
		case GameEnum::SPEED_MULTI:
		{
			this->update_fighter_speed(pair_obj.obj_id(), pair_obj.obj_value(), offset);
			break;
		}
		case GameEnum::BLOOD_MAX:
		{
			blood_magic_flag += this->blood_max_set(pair_obj.obj_value(), offset, request->enter_type());
			break;
		}
		case GameEnum::MAGIC_MAX:
		{
			blood_magic_flag += this->magic_max_set(pair_obj.obj_value(), offset, request->enter_type());
			break;
		}
		case GameEnum::ATTACK:
		{
			fight_detail.__attack_lower.set_single(pair_obj.obj_value(), offset);
			fight_detail.__attack_upper.set_single(pair_obj.obj_value(), offset);
			break;
		}
		case GameEnum::DEFENSE:
		{
			fight_detail.__defence_lower.set_single(pair_obj.obj_value(), offset);
			fight_detail.__defence_upper.set_single(pair_obj.obj_value(), offset);
			break;
		}
		case GameEnum::CRIT:
		{
			fight_detail.__crit.set_single(pair_obj.obj_value(), offset);
			break;
		}
		case GameEnum::TOUGHNESS:
		{
			fight_detail.__toughness.set_single(pair_obj.obj_value(), offset);
			break;
		}
		case GameEnum::HIT:
		{
			fight_detail.__hit.set_single(pair_obj.obj_value(), offset);
			break;
		}
		case GameEnum::AVOID:
		{
			fight_detail.__avoid.set_single(pair_obj.obj_value(), offset);
			break;
		}
        case GameEnum::LUCKY:
        {
            fight_detail.__lucky.set_single(pair_obj.obj_value(), offset);
            break;
        }
        case GameEnum::DAMAGE:
        {
            fight_detail.__damage.set_single(pair_obj.obj_value(), offset);
            break;
        }
        case GameEnum::REDUCTION:
        {
            fight_detail.__reduction.set_single(pair_obj.obj_value(), offset);
            break;
        }
		case GameEnum::ATTACK_MULTI:
		{
			fight_detail.__attack_lower_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			fight_detail.__attack_upper_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			break;
		}
		case GameEnum::DEFENCE_MULTI:
		{
			fight_detail.__defence_lower_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			fight_detail.__defence_upper_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			break;
		}
		case GameEnum::CRIT_HURT_MULTI:
		{
			fight_detail.__crit_hurt_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			break;
		}
		case GameEnum::CRIT_VALUE_MULTI:
		{
			fight_detail.__crit_value_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			break;
		}
		case GameEnum::TOUGHNESS_MULTI:
		{
			fight_detail.__toughness_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			break;
		}
		case GameEnum::HIT_MULTI:
		{
			fight_detail.__hit_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			break;
		}
		case GameEnum::AVOID_MULTI:
		{
			fight_detail.__avoid_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
			break;
		}
		case GameEnum::BLOOD_MULTI:
		{
			blood_magic_flag += this->blood_max_multi_set(pair_obj.obj_value() / GAME_PERCENT,
					offset, request->enter_type());
			break;
		}
		case GameEnum::MAGIC_MULTI:
		{
			blood_magic_flag += this->magic_max_multi_set(pair_obj.obj_value() / GAME_PERCENT,
					offset, request->enter_type());
			break;
		}
        case GameEnum::LUCKY_MULTI:
        {
            fight_detail.__lucky_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
            break;
        }
        case GameEnum::DAMAGE_MULTI:
        {
            fight_detail.__damage_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
            break;
        }
        case GameEnum::REDUCTION_MULTI:
        {
            fight_detail.__reduction_multi.set_single(pair_obj.obj_value() / GAME_PERCENT, offset);
            break;
        }
		}
	}

	JUDGE_RETURN(this->is_enter_scene() == true, 0);

	if (blood_magic_flag > 0)
	{
		int inc_blood = prev_blood - fight_detail.__blood;
		int inc_magic = prev_magic - fight_detail.__magic;

		if (inc_blood != 0)
		{
			this->notify_fight_update(FIGHT_UPDATE_BLOOD, inc_blood, 0, 0, fight_detail.__blood);
		}

		if (inc_magic != 0)
		{
			this->notify_fight_update(FIGHT_UPDATE_MAGIC, inc_magic, 0, 0, fight_detail.__magic);
		}
	}

	JUDGE_RETURN(request->unnotify() == false, 0);
	return this->update_fight_property(offset);
}

int MapPlayer::start_login_map_logic(Int64 login_tick)
{
	Proto31400001 gate_info;
	gate_info.set_gate_sid(this->gate_sid());
	gate_info.set_login_tick(login_tick);
    return this->send_to_logic_thread(gate_info);
}

// 切场景同步map logic完成
int MapPlayer::finish_login_map_logic(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400006*, request, -1);

	if (request->finish_type() == ENTER_SCENE_LOGIN)
	{
		this->finish_map_logic_by_login(request);
	}
	else
	{
		this->finish_map_logic_by_transfer(request);
	}

	if (this->is_enter_scene() == true)
	{
		MAP_MONITOR->dispatch_to_logic(this, INNER_LOGIC_MAP_ENTER_SCENE);
	}

	return 0;
}

int MapPlayer::finish_map_logic_by_login(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400006 *, request, -1);

	int ret = this->enter_scene(request->finish_type());
	JUDGE_RETURN(ret != 0, ret);

	MSG_USER("MapPlayer login error %d %ld", ret, this->role_id());
	this->enter_error_ = true;
	this->handle_enter_scene_error(ret);

	return 0;
}

int MapPlayer::finish_map_logic_by_transfer(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400006 *, request, -1);

    MSG_USER("log transfer 3 %ld %d %d %d", this->role_id(), this->enter_error_,
    		request->finish_type(), this->scene_id());
	if (this->enter_error_ == true)
	{
		int ret = this->enter_scene(request->finish_type());
		JUDGE_RETURN(ret != 0, ret);

		this->enter_error_ = true;
		this->handle_enter_scene_error(ret);
	}
	else
	{
		SubObj sub(0, 0, this->role_detail_.client_type_);
		this->notify_transfer_scene_info(sub);
		this->role_detail_.client_type_ = 0;
	}

	return 0;
}

int MapPlayer::finish_ml_level_upgrade()
{
	this->update_fight_property(BasicElement::BASIC);
	this->notify_update_player_info(GameEnum::PLAYER_INFO_LEVEL);
	return 0;
}

int MapPlayer::send_request_enter_info(int request_scene, Proto30400051& enter_info)
{
	int ret = this->validate_player_transfer();
	JUDGE_RETURN(ret == 0, ret);

	enter_info.set_role_id(this->role_id());
	enter_info.set_scene_id(this->scene_id());
	enter_info.set_role_level(this->level());

	enter_info.set_request_scene(request_scene);
	return MAP_MONITOR->dispatch_to_scene(this, request_scene, &enter_info);
}

int MapPlayer::request_enter_scene_done(Proto30400052* request, int recogn)
{
	this->respond_to_client(recogn);

	MoverCoord coord;
	coord.set_pixel(request->pos_x(), request->pos_y());
	return this->transfer_dispatcher(request->scene_id(), coord,
			request->scene_mode(), request->space_id());
}

int MapPlayer::send_to_logic_thread(int recogn)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);
	return this->monitor()->process_inner_logic_request(this->role_id(), recogn);
}

int MapPlayer::send_to_logic_thread(Message& msg)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);
	return this->monitor()->process_inner_logic_request(this->role_id(), msg);
}

int MapPlayer::send_to_other_scene(int scene_id, Message& msg)
{
	JUDGE_RETURN(this->is_need_send_message() == true, -1);
	return this->monitor()->dispatch_to_scene(this, scene_id, &msg);
}

int MapPlayer::validate_player_transfer(int jump_flag)
{
	if (this->is_death() == true)
	{
		return ERROR_PLAYER_DEATH;
	}

	if (jump_flag == true && this->is_jumping() == true)
	{
		return ERROR_USE_SKILL_JUMPING;
	}

    if (GameCommon::is_normal_scene(this->scene_id()) == false)
    {
        return ERROR_NORMAL_SCENE;
    }

    if (this->is_float_cruise() == true)
    {
        return ERROR_PLAYER_CRUISE;
    }

	return 0;
}



int MapPlayer::validate_scene_level(int scene_id)
{
	const Json::Value& set_conf = CONFIG_INSTANCE->scene_set(scene_id);
	JUDGE_RETURN(set_conf.empty() == false, false);
	JUDGE_RETURN(this->level() >= set_conf["enter_level"].asInt(), false);
	return true;
}

int MapPlayer::start_ml_sync_transfer(int cur_scene, int target_scene)
{
	Proto31400101 request;
	request.set_scene_id(target_scene);
	request.set_prev_scene(cur_scene);
	return this->send_to_logic_thread(request);
}

int MapPlayer::login_notify_enter_info()
{
	{
		this->notify_status(true);
		this->nofity_name_color_change(false);
	}

	{
		this->notify_server_activity_info();
	}

	return 0;
}

int MapPlayer::transfer_notify_enter_info()
{
	return 0;
}

int MapPlayer::notify_client_enter_info()
{
	if (this->load_first_notify_ == true)
	{
		this->login_notify_enter_info();
	}
	else
	{
		this->transfer_notify_enter_info();
	}

	this->load_first_notify_ = false;
	this->notify_fight_property(0);
	this->notify_connect_travel_chat();

	return 0;
}

int MapPlayer::obtain_area_info(int request_flag)
{
	//客户端获取周围场景信息
    MSG_USER("%ld %s %d %d[%d,%d]", this->role_id(), this->name(), this->force_total_i(),
    		this->scene_id(), this->location().pixel_x(), this->location().pixel_y());

    this->notify_update_player_space_id();
    this->all_beast_enter_scene();

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

#ifdef NO_BROAD_PORT
    Block_Buffer buff;
    scene->fetch_around_appear_info(this, buff);
    this->respond_from_broad_client(&buff);
#else
    Block_Buffer *pbuff = this->monitor_->pop_block(this->client_sid());
    pbuff->write_int32(this->client_sid());
    scene->fetch_around_appear_info(this, *pbuff);
    this->monitor_->client_sender(this->client_sid())->push_pool_block_with_len(pbuff);
#endif

    JUDGE_RETURN(request_flag == true, 0);

    //处理进入场景
    this->handle_special_scene_when_enter();
    // 请求Logic判断队伍信息
    this->notify_validate_team_info();
    //通知客户端信息
    this->notify_client_enter_info();
    //同步到逻辑服
    this->notify_logic_enter_scene(ENTER_SCENE_LOGIN);
    //同步到逻辑线程
    this->send_to_logic_thread(INNER_ML_MAP_OBTAIN_AREA_DONE);
    //变身外观同步
    this->trans_refresh_transfer_shape();

    // 切换场景时更改 pk 状态可能没同步到客户端
//    this->notify_fight_update(FIGHT_UPDATE_PK, this->fight_detail().__pk_state);
    this->enter_set_scene_info(this->scene_id());
    this->notify_fight_update(FIGHT_UPDATE_CAMP, this->camp_id());

//    // 广播石化状态
//    this->notify_enter_stone_state();

//    if (this->is_login() == false)
//    {
//    	// 跨服场景与普通场景切换时通知切换聊天
//		if ((GameCommon::is_travel_scene(this->scene_id()) && GameCommon::is_normal_scene(this->prev_scene_id())) ||
//				(GameCommon::is_normal_scene(this->scene_id()) == true && GameCommon::is_travel_scene(this->prev_scene_id()) == true))
//		{
//			this->notify_connect_travel_chat();
//		}
//    }

    this->set_is_login(false);
    this->clear_attack_me();

    this->login_transfer_to_trvl();
    FINER_PROCESS_NOTIFY(RETURN_OBTAIN_AREA);
}

int MapPlayer::transfer_dispatcher(int target_scene, const MoverCoord &coord,
        int scene_mode, int space_id)
{
	SceneInfo scene(target_scene, scene_mode, space_id);
	return this->transfer_dispatcher_b(scene, coord, SubObj());
}

int MapPlayer::transfer_dispatcher_b(const SceneInfo& scene, const MoverCoord &coord, const SubObj& sub)
{
    if (this->scene_id() == scene.id_ && this->scene_mode() == scene.mode_ && this->space_id() == scene.space_
    		&& this->location().pos_x() == coord.pos_x() && this->location().pos_y() == coord.pos_y())
    {
//    	if (this->is_in_script_mode() == false)
    	return -1;
    }

    int scene_id = scene.id_;
    int prev_scene = this->scene_id();
    // 退出场景前把需要发的消息先发了
    this->notify_exit_scene_cancel_info(EXIT_SCENE_TRANSFER, scene_id);

    if (this->is_gather_state())
    {
        this->fight_detail_.gather_state_ = 0;
    }

    this->exit_scene(EXIT_SCENE_TRANSFER);

#ifndef LOCAL_DEBUG
    if (this->monitor()->is_current_server_scene(scene_id) == false)
#else
	if (this->mover_detail().__prev_scene_mode != scene.mode_)
#endif
    {
        // 切换场景前保存，防止背包没保存时在新的进程读取加载
        if (this->transfer_flag() == false)
        {
        	this->request_save_player(TRANS_LOGOUT_MAP_PLAYER);
        }
    }

    {
        MoverDetail& detail = this->mover_detail();
		detail.__prev_scene_id 	= detail.__scene_id;
		detail.__prev_location 	= detail.__location;
		detail.__prev_space_id 	= detail.__space_id;
		detail.__prev_scene_mode= detail.__scene_mode;

		detail.__scene_id 	= scene_id;
		detail.__location 	= coord;
		detail.__scene_mode = scene.mode_;
		detail.__space_id 	= scene.space_;
    }

    this->sync_role_info_to_logic(prev_scene);
    this->sync_role_info_to_chat();

#ifndef LOCAL_DEBUG
    if (this->monitor()->is_current_server_scene(scene_id) == true)
#else
	if (this->mover_detail().__prev_scene_mode == scene.mode_)
#endif
    {
    	MSG_USER("Transfer %ld %d %d %d", this->role_id(), this->scene_id(),
    			this->location().pixel_x(), this->location().pixel_y());

    	this->notify_transfer_scene_info(sub);
    	if (scene_id == prev_scene)
    	{
    		this->enter_scene(ENTER_SCENE_TRANSFER);
    		this->obtain_area_info(false);
    	}
    	else
    	{
    		this->sync_info_to_map_logic();
    	}
    }
    else
    {
//        int prev_chat_scene = this->monitor()->chat_scene(prev_scene);
//        int next_chat_scene = this->monitor()->chat_scene(scene_id);
//
//        if (prev_chat_scene != next_chat_scene)
//        {
//            this->request_chat_logout();
//        }

        //start map logic sync
        this->transfer_flag_ = true;
        this->role_detail_.client_type_ = sub.val3_;
        this->transfer_timeout_tick_ = Time_Value::gettimeofday() + Time_Value(30);

        MSG_USER("transfer dispatch timeout tick %ld %s %d %ld.%06ld", this->role_id(), this->name(),
        		sub.val3_, this->transfer_timeout_tick_.sec(), this->transfer_timeout_tick_.usec());
        this->start_ml_sync_transfer(prev_scene, scene_id);
    }

	return 0;
}

int MapPlayer::make_up_login_info(Block_Buffer *buff, const bool send_by_gate)
{
	Proto80400101 respond;
	respond.set_chat_scene(MAP_MONITOR->chat_scene());
	respond.set_main_act(CONFIG_INSTANCE->main_activity_type());
	respond.set_open_days(CONFIG_INSTANCE->client_open_days());
	respond.set_role_id(this->role_id());
	respond.set_name(this->role_name());
	respond.set_full_name(this->role_name());
	respond.set_level(this->level());
	respond.set_sex(this->fight_sex());
	respond.set_server_flag(this->role_detail_.__server_flag);

	respond.set_scene_id(this->scene_id());
	respond.set_pixel_x(this->location().pixel_x());
	respond.set_pixel_y(this->location().pixel_y());
	respond.set_space_id(this->space_id());
	respond.set_toward(this->mover_detial_.__toward);
	respond.set_pk_state(this->fight_detail_.__pk_state);
	respond.set_experience(this->fight_detail_.__experience);

	respond.set_angry(this->fight_detail_.__angry);
	respond.set_jump_value(this->fight_detail_.__jump);
	respond.set_blood(this->fight_detail_.cur_blood());
	respond.set_max_blood(this->fight_detail_.__blood_total_i(this));

	respond.set_vip_type(this->vip_type());
	respond.set_force(this->force_total_i());
    respond.set_speed(this->speed_total_i());
    respond.set_league_id(this->role_detail_.__league_id);
    respond.set_league_name(this->role_detail_.__league_name);
    respond.set_league_pos(this->role_detail_.__league_pos);

    respond.set_partner_id(this->role_detail_.__partner_id);
    respond.set_partner_name(this->role_detail_.__partner_name);
    respond.set_wedding_id(this->role_detail_.__wedding_id);
    respond.set_wedding_type(this->role_detail_.__wedding_type);

//    respond.set_team_id(this->team_id());
    respond.set_camp_id(this->camp_id());
//    respond.set_permission(this->role_detail_.__permission);
    respond.set_mw_id(this->magic_weapon_id());
    respond.set_mw_rank_level(this->magic_weapon_lvl());
    this->makeu_up_shape_info(respond.mutable_shape_info());

    if (send_by_gate == false)
	{
    	ProtoClientHead head;
    	head.__recogn = ACTIVE_ROLE_LOGIN;
    	return this->make_up_client_block(buff, &head, &respond);
	}
	else
	{
        ProtoHead head;
        head.__recogn = ACTIVE_ROLE_LOGIN;
        head.__role_id = this->mover_id();
        head.__scene_id = this->scene_id();
        return this->make_up_gate_block(buff, &head, &respond);
	}
}

int MapPlayer::make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate)
{
	this->make_up_role_appear_info(buff, send_by_gate);
	this->make_up_beast_appear_info(buff, send_by_gate);

    return 0;
}

int MapPlayer::make_up_move_info(Block_Buffer *buff, Int64 mover_id/* = 0*/)
{
    if (this->is_float_cruise() == false)
        return GameFighter::make_up_move_info(buff, mover_id);

    Proto80400227 respond;
    if(mover_id == 0)
    	mover_id = this->mover_id();
    respond.set_mover_id(mover_id);
    for (MoverDetail::MoverStepList::iterator iter = this->mover_detial_.__step_list.begin();
            iter != this->mover_detial_.__step_list.end(); ++iter)
    {
        ProtoCoord *coord = respond.add_step_list();
        coord->set_pixel_x(iter->pixel_x());
        coord->set_pixel_y(iter->pixel_y());
    }

    ProtoClientHead head;
    head.__recogn = ACTIVE_FOLLOW_FLOAT_MOVE;
    return this->make_up_client_block(buff, &head, &respond);
}

int MapPlayer::make_up_role_appear_info(Block_Buffer *buff, const bool send_by_gate)
{
    Proto80400102 respond;
    respond.set_role_id(this->role_id());
    respond.set_name(this->role_name());
    respond.set_full_name(this->role_name());
    respond.set_level(this->level());
    respond.set_sex(this->role_detail_.__sex);
    respond.set_scene_id(this->mover_detial_.__scene_id);
    respond.set_pixel_x(this->mover_detial_.__location.pixel_x());
    respond.set_pixel_y(this->mover_detial_.__location.pixel_y());
    respond.set_space_id(this->space_id());
    respond.set_toward(this->mover_detial_.__toward);
    respond.set_blood(this->fight_detail_.__blood);
    respond.set_max_blood(this->fight_detail_.__blood_total_i(this));
    respond.set_pk_state(this->fight_detail_.__pk_state);
    respond.set_camp_id(this->camp_id());
    respond.set_speed(this->speed_total_i());
    respond.set_fight_state(this->fetch_fight_state());

    respond.set_vip_type(this->vip_type());
    respond.set_force(this->force_total_i());
	respond.set_league_id(this->league_id());
    respond.set_league_name(this->role_detail_.__league_name);
    respond.set_league_pos(this->role_detail_.__league_pos);
    respond.set_partner_id(this->role_detail_.__partner_id);
    respond.set_partner_name(this->role_detail_.__partner_name);
    respond.set_wedding_id(this->role_detail_.__wedding_id);
    respond.set_wedding_type(this->role_detail_.__wedding_type);
	respond.set_is_copy(this->copy_offline_);
    respond.set_name_color(this->fetch_name_color());
    respond.set_gather_state(this->fight_detail_.gather_state_);
    respond.set_mw_id(this->magic_weapon_id());
    respond.set_mw_rank_level(this->magic_weapon_lvl());
    this->makeu_up_shape_info(respond.mutable_shape_info());

	Scene* scene = this->fetch_scene();
    if (scene != 0)
    {
    	scene->makeup_role_appear_info(this, &respond);
    }

    ProtoClientHead head;
    head.__recogn = ACTIVE_ROLE_APPEAR;
    return this->make_up_client_block(buff, &head, &respond);
}

int MapPlayer::make_up_beast_appear_info(Block_Buffer *buff, const bool send_by_gate)
{
	for (int i = MapMaster::TYPE_BEAST; i < MapMaster::BEAST_TOTAL; ++i)
	{
	    MapBeast* beast = this->fetch_cur_beast(i);
	    JUDGE_CONTINUE(beast != NULL);
	    beast->make_up_appear_info_base(buff, send_by_gate);
	}

    return 0;
}

int MapPlayer::makeu_up_shape_info(ProtoRoleShape* shape_info, int type)
{
	JUDGE_RETURN(shape_info != NULL, -1);

	shape_info->set_sex(this->fight_sex());
	shape_info->set_career(this->fight_career());
	shape_info->set_label(this->get_cur_label());

	shape_info->set_weapon(this->get_shape_item_id(GameEnum::EQUIP_WEAPON));
	shape_info->set_clothes(this->get_shape_item_id(GameEnum::EQUIP_YIFU));

	shape_info->set_mount_sort(this->fetch_mount_id(GameEnum::FUN_MOUNT));
	shape_info->set_wing(this->fetch_mount_id(GameEnum::FUN_XIAN_WING));
	shape_info->set_god_weapon(this->fetch_mount_id(GameEnum::FUN_GOD_SOLIDER));
	shape_info->set_tian_gang(this->fetch_mount_id(GameEnum::FUN_TIAN_GANG));

	shape_info->set_sword_pool(this->fetch_spool_style_lvl());
	shape_info->set_fashion_id(this->fetch_fashion_id());
	shape_info->set_fashion_color(this->fetch_fashion_color());
	shape_info->set_transfer_id(this->fetch_show_mode_id());

	return 0;
}

int MapPlayer::fetch_show_mode_id()
{
	if (this->is_lrf_change_mode() == true && this->is_in_league_region() == true)
	{
		return this->role_detail_.hickty_id_;
	}
	else
	{
		return this->fetch_transfer_id();
	}
}

int MapPlayer::schedule_move_action(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400109*, request, RETURN_SCHEDULE_MOVE);

//    static int move_max_step = CONFIG_INSTANCE->max_move_step();
//    if (request->step_list_size() <= 0 || request->step_list_size() > move_max_step)
//    {
//    	return this->adjust_to_cur_coord(ERROR_COORD_OFFSET);
//    }

    if (request->scene_id() != this->scene_id())
    {
//    	return this->adjust_to_cur_coord(ERROR_COORD_OFFSET);
    	return 0;
    }

    MoverCoord step = this->location();
    MoverCoord prev_step = this->location();
    this->mover_detial_.__step_list.clear();

    for (int i = 0; i < request->step_list_size(); ++i)
    {
        const ProtoCoord &coord = request->step_list(i);
        step.set_pixel(coord.pixel_x(), coord.pixel_y());

        int offset = coord_offset_grid(prev_step, step);
        if ((i == 0 && offset > 6) || (i > 0 && offset > 3))
        {
            // 客户端有时发第一步时相隔当前位置很远
        	return this->adjust_to_cur_coord(ERROR_COORD_OFFSET);
        }

        prev_step = step;
        this->mover_detial_.__step_list.push_back(step);
    }

    Time_Value arrive_tick = Time_Value::gettime(request->arrive_tick());
    int ret = this->schedule_move(step, request->toward(), arrive_tick);
    if (ret != 0)
    {
    	if (ret == -1)
    	{
    		return this->respond_to_client(RETURN_SCHEDULE_MOVE);
    	}
    	else
    	{
    		return this->adjust_to_cur_coord(ret);
    	}
    }

    // 通知打开小地图界面的队友有移动
    return this->notify_teamer_move();
}

int MapPlayer::transfer_to_other_scene(Message *msg)
{
//    MSG_DYNAMIC_CAST_NOTIFY(Proto10400110 *, request, ERROR_CLIENT_OPERATE);
//
//    int scene_from = CONFIG_INSTANCE->convert_scene_to(this->scene_id());
//    int scene_to = request->scene_id();
//    MoverCoord coord_from = this->location(), target;
//
//    MSG_DEBUG("transfer to %ld %s %d(%d,%d) -> %d", this->role_id(), this->name(),
//    		this->scene_id(), this->location().pixel_x(), this->location().pixel_y(), scene_to);
//
//    const Json::Value &scene_from_json = CONFIG_INSTANCE->scene(scene_from);
//    CONDITION_NOTIFY_RETURN(scene_from_json != Json::nullValue, RETURN_TRANSFER_SCENE, ERROR_CONFIG_NOT_EXIST);
//
//    {
//        const Json::Value &exits_json = scene_from_json["exits"];
//        bool is_exists_exits = false;
//        for (uint i = 0; i < exits_json.size(); ++i)
//        {
//        	int scene_id = ::atoi(exits_json[i]["id"].asCString());
//            if (scene_id == scene_to)
//            {
//                MoverCoord exits_coord;
//                exits_coord.set_pixel(exits_json[i]["posX"].asInt(), exits_json[i]["posY"].asInt());
//                if (coord_offset_grid(coord_from, exits_coord) > 4)
//                    continue;
//                is_exists_exits = true;
//                break;
//            }
//        }
//        CONDITION_NOTIFY_RETURN(is_exists_exits == true, RETURN_TRANSFER_SCENE, ERROR_COORD_OFFSET);
//    }
//
//    const Json::Value &scene_to_json = CONFIG_INSTANCE->scene(scene_to);
//    CONDITION_NOTIFY_RETURN(scene_to_json != Json::nullValue, RETURN_TRANSFER_SCENE, ERROR_CONFIG_NOT_EXIST);
//    MoverCoord coord_to;
//    {
//        const Json::Value &exits_json = scene_to_json["exits"];
//        bool is_exists_exits = false;
//        for (uint i = 0; i < exits_json.size(); ++i)
//        {
//        	int scene_id = ::atoi(exits_json[i]["id"].asCString());
//            if (scene_id == scene_from)
//            {
//                coord_to.set_pixel(exits_json[i]["posX"].asInt(), exits_json[i]["posY"].asInt());
//                is_exists_exits = true;
//                break;
//            }
//        }
//        if (is_exists_exits == false)
//        {
//        	if (GameCommon::is_script_scene(scene_to) && scene_to_json.isMember("relive"))
//        	{
//        		coord_to.set_pixel(scene_to_json["relive"]["posX"].asInt(), scene_to_json["relive"]["posY"].asInt());
//        	}
//        	else
//        	{
//        		return this->respond_to_client_error(RETURN_TRANSFER_SCENE, ERROR_SCENE_NO_ADJACENT);
//        	}
//        }
//    }
//    if (GameCommon::is_boss_fusion_scene(scene_to))
//    {
//    	CONDITION_NOTIFY_RETURN(GameCommon::is_arr_scene_level_limit(scene_to, this->level()) == true,
//    			RETURN_TRANSFER_SCENE, ERROR_PLAYER_LEVEL_LIMIT);
//    	return this->transfer_dispatcher(scene_to, coord_to, SCENE_MODE_SHUSAN, 1);
//    }
//
//    return this->transfer_dispatcher(scene_to, coord_to);
	return 0;
}

int MapPlayer::adjust_to_cur_coord(int error)
{
    Proto50400109 respond;
    respond.set_scene_id(this->scene_id());
    respond.set_pixel_x(this->location().pixel_x());
    respond.set_pixel_y(this->location().pixel_y());
	return this->respond_to_client_error(RETURN_SCHEDULE_MOVE, error, &respond);
}

int MapPlayer::sync_transfer_base(void)
{
	Proto30400101 request;
	request.set_transfer_tick(::time(NULL));

    SessionDetail *session = 0;
    if (SESSION_MANAGER->find_account_session(this->role_detail_.__account, session) != 0)
    {
        MSG_USER("ERROR no session %d %s", this->role_id(), this->role_detail_.__account.c_str());
    }
    else
    {
        request.set_session(session->__session);
    }

    request.set_role_id(this->role_id_);
    request.set_enter_error(this->enter_error_);
    request.set_team_id(this->role_detail_.__team_id);
    request.set_label_id(this->role_detail_.__label_id);
    request.set_permission(this->role_detail().__permission);

//    request.set_vip_type(this->role_detail_.__vip_type);
    request.set_league_id(this->role_detail_.__league_id);
    request.set_league_name(this->role_detail_.__league_name);
    request.set_league_pos(this->role_detail_.__league_pos);
    request.set_fight_force(this->role_detail_.__fight_force);
    request.set_partner_id(this->role_detail_.__partner_id);
    request.set_partner_name(this->role_detail_.__partner_name);
    request.set_wedding_id(this->role_detail_.__wedding_id);
    request.set_wedding_type(this->role_detail_.__wedding_type);
    request.set_login_day(this->role_detail_.__login_days);
    request.set_login_tick(this->role_detail_.__login_tick);

    request.set_agent(this->role_detail_.__agent);
    request.set_agent_code(this->role_detail_.__agent_code);
    request.set_market_code(this->role_detail_.__market_code);
    request.set_sex(this->role_detail_.__sex);
    request.set_level(this->role_detail_.__level);
    request.set_escort_times(this->role_detail_.escort_times);
    request.set_protect_times(this->role_detail_.protect_times);
    request.set_rob_times(this->role_detail_.rob_times);

    request.set_name(this->role_detail_.name());
    request.set_src_name(this->role_detail_.__src_name);
    request.set_career(this->role_detail_.__career);
    request.set_account(this->role_detail_.__account);
    request.set_scene_id(this->role_detail_.__scene_id);
    request.set_fb_flag(this->role_detail_.fb_flag_);
    request.set_client_type(this->role_detail_.client_type_);
    request.set_sign_trvl_team(this->role_detail_.sign_trvl_team_);

    request.set_wedding_giftbox_tick(this->role_detail_.__wedding_giftbox_tick.sec());
    request.set_wedding_giftbox_times(this->role_detail_.__wedding_giftbox_times);
    request.set_fresh_free_relive_tick(this->role_detail_.__refresh_free_relive_tick.sec());
    request.set_used_free_relive(this->role_detail_.__used_free_relive);
    request.set_collect_chest_amount(this->role_detail_.__collect_chest_amount);
    request.set_server_flag(this->role_detail_.__server_flag);
    if (GameCommon::is_travel_scene(this->scene_id()) && this->role_detail().__trvl_server_flag.empty() == true)
    {
    	request.set_cur_server_flag(CONFIG_INSTANCE->server_flag());
    }
    else
    {
    	request.set_cur_server_flag(this->role_detail().__trvl_server_flag);
    }

    return this->send_to_other_scene(this->scene_id(), request);
}

int MapPlayer::read_transfer_base(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400101*, request, -1);

    this->role_id_ = request->role_id();
    this->enter_error_ = request->enter_error();

    this->role_detail_.set_server_flag(request->server_flag());
    this->role_detail().__cur_server_flag = request->cur_server_flag();
    this->role_detail().__trvl_server_flag = request->cur_server_flag();
    this->role_detail_.__id = request->role_id();
    this->role_detail_.__name = request->name();
    this->role_detail_.__src_name = request->src_name();
    this->role_detail_.__team_id = request->team_id();
    this->role_detail_.__label_id = request->label_id();
//    this->role_detail_.__vip_type = request->vip_type();
    this->role_detail_.__league_id = request->league_id();
    this->role_detail_.__league_name = request->league_name();
    this->role_detail_.__league_pos = request->league_pos();
    this->role_detail_.__fight_force = request->fight_force();

    this->role_detail_.__partner_id = request->partner_id();
    this->role_detail_.__partner_name = request->partner_name();
    this->role_detail_.__wedding_id = request->wedding_id();
    this->role_detail_.__wedding_type = request->wedding_type();

    this->role_detail_.__agent = request->agent();
    this->role_detail_.__account = request->account();
    this->role_detail_.__sex = request->sex();
    this->role_detail_.__level = request->level();
    this->role_detail_.__career = request->career();
    this->role_detail_.__permission = request->permission();

    this->role_detail_.__scene_id = request->scene_id();
    this->role_detail_.__agent_code = request->agent_code();
    this->role_detail_.__market_code = request->market_code();
    this->role_detail_.fb_flag_ = request->fb_flag();
    this->role_detail_.sign_trvl_team_ = request->sign_trvl_team();
    this->role_detail_.client_type_ = request->client_type();

    this->role_detail_.__login_days = request->login_day();
    this->role_detail_.__login_tick = request->login_tick();

    SessionManager *session_manager = this->monitor()->session_manager();
    session_manager->update_session(0, request->account(), request->session(), request->role_id());

    this->role_detail_.escort_times = request->escort_times();
    this->role_detail_.protect_times = request->protect_times();
    this->role_detail_.rob_times = request->rob_times();

    this->role_detail_.__wedding_giftbox_tick.sec(request->wedding_giftbox_tick());
    this->role_detail_.__wedding_giftbox_times = request->wedding_giftbox_times();
    this->role_detail_.__refresh_free_relive_tick.sec(request->fresh_free_relive_tick());
    this->role_detail_.__used_free_relive = request->used_free_relive();
    this->role_detail_.__collect_chest_amount = request->collect_chest_amount();

    return 0;
}

int MapPlayer::sync_role_info_to_chat(void)
{
	Proto30200126 role_info;
	role_info.set_role_id(this->role_id());
	role_info.set_role_level(this->level());
	role_info.set_scene_id(this->scene_id());
	role_info.set_space_id(this->space_id());
	return this->monitor()->dispatch_to_chat(this, &role_info);
}

int MapPlayer::sync_role_info_to_logic(const int prev_scene /*=0*/)
{
    Proto30400201 request;
    request.set_role_id(this->role_id());
    request.set_career(this->role_detail_.__career);
    request.set_scene_id(this->scene_id());
    request.set_camp_id(this->fight_detail_.__camp_id);
    request.set_level(this->level());
    request.set_force(this->role_detail_.__fight_force);
    request.set_scene_mode(this->scene_mode());
    request.set_space_id(this->space_id());
    request.set_prev_scene(prev_scene);
//    request.set_permission(this->role_detail_.__permission);
    return this->monitor()->dispatch_to_logic(this, &request);
}

int MapPlayer::finish_sync_transfer_scene(void)
{
    Proto30400102 request;
    request.set_role_id(this->role_id());
    request.set_transfer_tick(::time(NULL));
    return this->monitor()->dispatch_to_scene(this, this->scene_id(), &request);
}

int MapPlayer::sync_transfer_move(void)
{
    Proto30400103 request;
    this->serialize_move(&request);
    return this->send_to_other_scene(this->scene_id(), request);
}

int MapPlayer::sync_transfer_fight(void)
{
    Proto30400104 request_fight;
    this->serialize_fight(&request_fight);
    return this->send_to_other_scene(this->scene_id(), request_fight);
}

int MapPlayer::sync_transfer_online(void)
{
    Proto30400105 request;
    this->serialize_online(&request);
    return this->send_to_other_scene(this->scene_id(), request);
}

int MapPlayer::handle_player_relive(const MoverCoord& relive_coord, double percent)
{
	JUDGE_RETURN(this->is_death() == true, -1);
	JUDGE_RETURN(this->is_movable_coord(relive_coord) == true, -1);

	this->fight_detail_.__death_tick = 0;
	this->fight_detail_.__relive_tick = Time_Value::gettimeofday();

    this->fighter_restore_all(FIGHT_TIPS_RELIVE, 0, percent);
    this->team_notify_teamer_blood();

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, -1);

	scene->refresh_mover_location_for_relive(this, relive_coord);

	int protected_last = scene->fetch_relive_protected_time(this->role_id());
	this->insert_protect_buff(protected_last);
	this->notify_mover_cur_location();

	this->all_beast_enter_scene();
    this->sync_fighter_status_to_map_logic();

    return 0;
}

void MapPlayer::set_new_role_flag(const bool flag)
{
	this->is_new_role_ = flag;
}

bool MapPlayer::is_new_role(void)
{
	return this->is_new_role_;
}

bool MapPlayer::is_need_send_message(void)
{
	if (this->copy_offline_ == true)
	{
		return false;
	}

	if (this->start_offline_ == true)
	{
		return false;
	}

	return true;
}


bool MapPlayer::validate_festival_activity(int act_index)
{
	return this->role_detail_.drop_act_ == act_index;
}

bool MapPlayer::validate_big_act_time()
{
	return this->role_detail_.is_big_act_time_ == true;
}

void MapPlayer::init_level_property(int cur_level)
{
    const Json::Value &level_json = CONFIG_INSTANCE->role_level(
    		this->role_detail().__career, this->level());
    JUDGE_RETURN(level_json.empty() == false, ;);

    FightProperty fight_prop(BasicElement::BASIC);
    fight_prop.make_up_name_prop(level_json);

    this->set_fight_property(fight_prop);
    this->init_klv_property();
}

void MapPlayer::init_skill_property()
{
    FightProperty fight_prop(BasicElement::PLAYER_SKILL);

	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
			iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
		FighterSkill* skill = iter->second;
		JUDGE_CONTINUE(skill->is_passive_prop_skill() == true);

		const Json::Value& detial_json = skill->detail();
		fight_prop.make_up_name_prop(detial_json);
	}

	this->set_fight_property(fight_prop);
}

void MapPlayer::init_skill_buff()
{
	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
			iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
		FighterSkill* skill = iter->second;
		JUDGE_CONTINUE(skill->is_passive_buff_skill() == true);

		const Json::Value& detial_json = skill->detail();
		if (detial_json.isMember("effect") == true)
		{
			for (uint i = 0; i < detial_json["effect"].size(); ++i)
			{
				const Json::Value &effect = detial_json["effect"][i];
				JUDGE_CONTINUE(effect["percent"].asDouble() > 0 || effect["value"].asInt() > 0);

				this->insert_defender_status(this, effect["id"].asInt(),
						effect["interval"].asDouble(), effect["last"].asDouble(),
				        effect["accumulate_times"].asInt(), effect["value"].asInt(),
				        effect["percent"].asDouble());
			}
		}
	}
}

void MapPlayer::set_fight_property(const FightProperty& fight_prop)
{
    FightDetail &fight_detail = this->fight_detail();
    fight_detail.__blood_max.set_single(fight_prop.blood_max_, fight_prop.type_);
    fight_detail.__attack_upper.set_single(fight_prop.attack_, fight_prop.type_);
    fight_detail.__attack_lower.set_single(fight_prop.attack_, fight_prop.type_);
    fight_detail.__defence_upper.set_single(fight_prop.defence_, fight_prop.type_);
    fight_detail.__defence_lower.set_single(fight_prop.defence_, fight_prop.type_);
    fight_detail.__hit.set_single(fight_prop.hit_, fight_prop.type_);
    fight_detail.__avoid.set_single(fight_prop.avoid_, fight_prop.type_);
    fight_detail.__crit.set_single(fight_prop.crit_, fight_prop.type_);
    fight_detail.__toughness.set_single(fight_prop.toughness_, fight_prop.type_);
    fight_detail.__damage.set_single(fight_prop.damage_, fight_prop.type_);
    fight_detail.__reduction.set_single(fight_prop.reduction_, fight_prop.type_);

    double tmp = fight_prop.crit_hurt_multi_ * 1.0 / GameEnum::DAMAGE_ATTR_PERCENT;
    fight_detail.__crit_hurt_multi.set_single(tmp, fight_prop.type_);

    tmp = fight_prop.damage_multi_ * 1.0 / GameEnum::DAMAGE_ATTR_PERCENT;
    fight_detail.__damage_multi.set_single(tmp,fight_prop.type_);

    tmp = fight_prop.reduction_multi_ * 1.0 / GameEnum::DAMAGE_ATTR_PERCENT;
    fight_detail.__reduction_multi.set_single(tmp,fight_prop.type_);
}

void MapPlayer::set_base_member_info(BaseMember& base)
{
	MapRoleDetail& role_detail = this->role_detail();
	base.unserialize(role_detail);

	base.fashion_ 	= this->fetch_fashion_id();
	base.wing_ 		= this->fetch_mount_id(GameEnum::FUN_XIAN_WING);
	base.solider_ 	= this->fetch_mount_id(GameEnum::FUN_GOD_SOLIDER);
}

int MapPlayer::level_upgrade(void)
{
    int prev_level = this->level();
    JUDGE_RETURN(prev_level < MAX_PLAYER_LEVEL, -1);

    GameFighter::level_upgrade();
    this->role_detail_.__level = this->level();
//    this->check_update_achievement(GameEnum::SKILL, this->count_player_normal_skill_num());

    this->init_level_property();
//    this->init_all_player_skill();
//    this->init_skill_property();

    this->modify_blood_by_levelup(this->fight_detail_.__blood_total_i(this), FIGHT_TIPS_NOTHING);
    this->modify_magic_by_notify(-this->fight_detail_.__magic_total_i(this), FIGHT_TIPS_NOTHING);

    this->sync_info_to_map_logic();
    this->sync_role_info_to_logic();
    this->sync_role_info_to_chat();

    MapPlayerEx* self = this->self_player();
    self->check_tarena_pa_event(0);
    self->update_tarena_open_activity();

    this->cache_tick().update_cache(MapPlayerEx::CACHE_FIGHT, true);
    return SERIAL_RECORD->record_player_level(this, this->agent_code(),
    		this->market_code(), 1, this->level());
}

int MapPlayer::request_chat_login(void)
{
//#ifndef LOCAL_DEBUG
//	JUDGE_RETURN(this->monitor()->is_has_travel_scene() == false, -1);
//#endif

    Proto30200101 chat_request;
    chat_request.set_role_id(this->role_id());
    chat_request.set_name(this->role_name());
    chat_request.set_level(this->level());
    chat_request.set_sex(this->role_detail_.__sex);
    chat_request.set_career(this->role_detail_.__career);
    chat_request.set_scene_id(this->scene_id());
    chat_request.set_vip(this->vip_detail().__vip_level);
    chat_request.set_league_id(this->role_detail_.__league_id);
    chat_request.set_league_name(this->role_detail_.__league_name);
    chat_request.set_label_id(this->role_detail_.__label_id);
    chat_request.set_team_id(this->team_id());
    chat_request.set_agent_code(this->agent_code());
    chat_request.set_server_flag(this->role_detail_.__server_flag);
    chat_request.set_market_code(this->role_detail().__market_code);
    chat_request.set_last_sign_out(this->online().online_detail().__sign_out_tick);
    chat_request.set_permission(this->role_detail().__permission);

	// 同步服务器前缀
	chat_request.set_server_prev(this->role_detail_.__server_prev);
	chat_request.set_server_name(this->role_detail_.__server_name);

//    int area_id = this->fetch_travel_area_id();
//    chat_request.set_area_id(area_id);

    GameCommon::make_up_session(chat_request.mutable_session_info(),
     		this->role_detail_.__account);

    MSG_USER("MAP CHAT REQUEST ==> %ld %s last_sign_out:%d, sub:%d %s %s %s",
    		this->role_id(), this->name(), this->online().online_detail().__sign_out_tick,
    		Time_Value::gettimeofday().sec() - this->online().online_detail().__sign_out_tick,
    		this->role_detail_.__server_flag.c_str(),
    		chat_request.server_prev().c_str(), chat_request.server_name().c_str());

    return this->monitor()->dispatch_to_chat(this, &chat_request);
}

int MapPlayer::request_chat_logout(void)
{
//    return this->monitor()->dispatch_to_chat(this, INNER_CHAT_LOGOUT);
	return 0;
}

int MapPlayer::caculate_total_force()
{
	int player_force = this->fight_detail_.__force_total_i(this);
	int skill_force = this->fetch_skill_special_force();
	int mount_force = this->fetch_mount_special_force();

//	int prev_force = this->role_detail_.__fight_force;
	this->role_detail_.prev_force_ = this->role_detail_.__fight_force;
	this->role_detail_.__fight_force = player_force + skill_force + mount_force;
	return this->role_detail_.__fight_force;
}

int MapPlayer::force_total_i(void)
{
	return this->role_detail_.__fight_force;
}

double MapPlayer::fetch_addition_exp_percent()
{
	double inc = 0;
	inc += this->fetch_level_exp_percent();
	inc += this->fetch_vip_exp_percent();
	inc += this->fetch_team_exp_percent();

	inc += this->find_status_value_by_type(BasicStatus::PROP_ADD_EXP, 2)
			/ GameEnum::DAMAGE_ATTR_PERCENT;

	inc += this->find_status_value_by_type(BasicStatus::TRANSFER_ADD_EXP, 2)
			/ GameEnum::DAMAGE_ATTR_PERCENT;

	BasicStatus *status = 0;
    if (this->find_status(BasicStatus::QUINTUPLE_ADD_EXP, status) == 0)
    {
        inc += status->__value2 / GameEnum::DAMAGE_ATTR_PERCENT;
    }

    return inc;
}

double MapPlayer::fetch_team_exp_percent()
{
	JUDGE_RETURN(this->team_info().team_index_ != 0, 0);

	int team_num = this->teamate_count() - 1;
	int add_scale = CONFIG_INSTANCE->normal_team(1)["add_exp"][team_num].asInt();

	return (double)add_scale / (double)GameEnum::DAMAGE_ATTR_PERCENT;
}

double MapPlayer::fetch_vip_exp_percent()
{
	int add_scale = CONFIG_INSTANCE->vip(this->vip_type())["monster_extra_exp"].asInt();
	JUDGE_RETURN(add_scale > 0, 0);

	//vip 经验加成
	return (double)add_scale / (double)GameEnum::DAMAGE_ATTR_PERCENT;
}

double MapPlayer::fetch_level_exp_percent()
{
	static int WORLD_LV_MIN = CONFIG_INSTANCE->const_set("world_lv_min");
	JUDGE_RETURN(MAP_MONITOR->average_level() >= WORLD_LV_MIN, 0);

	static int WORLD_PLAYER_LV_MIN = CONFIG_INSTANCE->const_set("world_playerlv_min");
	JUDGE_RETURN(this->level() >= WORLD_PLAYER_LV_MIN, 0);

	int level_diff = MAP_MONITOR->average_level() - this->level();
	return CONFIG_INSTANCE->add_level_exp(level_diff);
}

int MapPlayer::notify_experience_update(int value)
{
    this->notify_fight_update(FIGHT_UPDATE_EXP, value, 0, 0,
    		0, 0, 0, 0, this->fight_detail_.__experience);
//    this->notify_one_tips_info(GameEnum::TIPS_EXP, 0, value);

    return 0;
}

int MapPlayer::save_cur_scene_info(int type)
{
	JUDGE_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true, -1);

	SaveSceneInfo& scene_info = this->role_detail_.__save_scene;
	scene_info.scene_id_ = this->scene_id();
	scene_info.coord_ = this->location();

	scene_info.blood_ = this->fight_detail_.__blood;
	scene_info.magic_ = this->fight_detail_.__magic;
	scene_info.pk_state_ = this->fight_detail_.__pk_state;
	return 0;
}

int MapPlayer::save_spacial_scene_info()
{
	JUDGE_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true, -1);

	WorldBossActor *wboss = this->self_player();
	SaveInfo &info = wboss->fetch_save_info();

	SaveSceneInfo& scene_info = this->role_detail_.__save_scene;
	scene_info.scene_id_ = info.scene_id_;
	scene_info.coord_ = info.coord_;

	scene_info.blood_ = this->fight_detail_.__blood;
	scene_info.magic_ = this->fight_detail_.__magic;
	scene_info.pk_state_ = this->fight_detail_.__pk_state;
	return 0;
}

int MapPlayer::enter_recover_scene_info(int check)
{
	bool save_flag = false;
	bool blood_flag = false;

	if (check == false)
	{
		save_flag = true;	//退出恢复上个场景信息
	}
	else
	{
		const Json::Value& set_conf = this->scene_set_conf();
		if (set_conf["save_recover_scene"].asInt() == 1)
		{
			save_flag = true;	//退出恢复上个场景信息
		}
		else if (set_conf["enter_blood_full"].asInt() == 1)
		{
			blood_flag = true;	//退出恢复满血
		}
	}

	JUDGE_RETURN(save_flag == true || blood_flag == true, -1);

	bool is_death = this->is_death();
	if (save_flag == true)
	{
		SaveSceneInfo& scene_info = this->role_detail_.__save_scene;
		this->set_pk_state(scene_info.pk_state_);

		this->modify_blood_by_levelup(scene_info.blood_);
		this->modify_magic_by_notify(this->fight_detail().__magic - scene_info.magic_);
	}
	else if (blood_flag == true)
	{
		this->fighter_restore_all();
	}

	if (is_death == true)
	{
		this->notify_mover_cur_location();
	}

	return 0;
}

int MapPlayer::enter_set_scene_info(int enter_scene)
{
	const Json::Value& set_conf = this->scene_set_conf();
	if (set_conf["save_recover_scene"].asInt() == 1
			|| set_conf["enter_blood_full"].asInt() == 1)
	{
		this->fighter_restore_all();
	}

	if (set_conf.isMember("default_pk_state") == true)
	{
		this->set_pk_state(set_conf["default_pk_state"].asInt());
	}

	return 0;
}

int MapPlayer::adjust_load_db_info()
{
	JUDGE_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true, -1);

	if (this->mover_detial_.__scene_mode != 0)
	{
		this->mover_detial_.__scene_mode = 0;
	}

	if (this->mover_detial_.__space_id != 0)
	{
		this->mover_detial_.__space_id = 0;
	}

	return 0;
}

int MapPlayer::login_transfer_to_trvl()
{
	JUDGE_RETURN(this->role_detail_.save_trvl_scene_ > 0, 0);

	if (GameCommon::is_trvl_wboss_scene(this->role_detail_.save_trvl_scene_))
	{
		MapPlayerEx* player = dynamic_cast<MapPlayerEx *>(this);
		player->login_enter_trvl_wboss(this->role_detail_.save_trvl_scene_);
	}

	this->role_detail_.save_trvl_scene_ = 0;
	return 0;
}

int MapPlayer::modify_element_experience(const int value, const SerialObj &serial_obj)
{
	int adjust_value = value;
    switch (serial_obj.type_)
    {
    case EXP_FROM_MONSTER:
    {
    	this->notify_monster_exp(value);
    	break;
    }
    case EXP_FROM_SCRIPT_MONSTER:
    {
    	adjust_value *= (1 + this->fetch_level_exp_percent());
    	break;
    }
    default:
    {
    	adjust_value *= (1 + this->fetch_level_exp_percent());

    	//经验流水
    	this->record_other_serial(MAIN_EXP_SERIAL, serial_obj.type_,
    			adjust_value, this->fight_detail().__experience);

    	break;
    }
    }

    JUDGE_RETURN(adjust_value > 0, 0);

    FightDetail& fight_detail = this->fight_detail();
    if (fight_detail.__experience >= LONG_MAX - adjust_value)
    {
    	fight_detail.__experience = LONG_MAX;
    }
    else
    {
    	fight_detail.__experience += adjust_value;
    }

    bool upgrade = false;
    this->notify_experience_update(adjust_value);
    while (fight_detail.enough_exp_upgrade() == true
    		&& GameCommon::is_max_level(this->level()) == false)
    {
    	upgrade = true;
    	fight_detail.__experience -= fight_detail.__next_exp;
    	this->record_other_serial(MAIN_EXP_SERIAL, serial_obj.type_,
    			adjust_value, fight_detail.__experience, true);
        this->level_upgrade();
    }

    if (upgrade == true)
    {
    	this->notify_experience_update(0);
    }

    return adjust_value;
}

int MapPlayer::modify_blood_by_fight(const double value, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
	int real_value = GameFighter::modify_blood_by_fight(value, fight_tips, attackor, skill_id);
	JUDGE_RETURN(real_value > 0 && attackor > 0, real_value);

    Scene* scene = this->fetch_scene();
    if (scene != NULL)
    {
    	Int64 benefited_attackor = this->fetch_benefited_attackor_id(attackor);
    	scene->handle_fighter_hurt(this, benefited_attackor, real_value);
    	scene->handle_player_hurt(this, benefited_attackor, real_value);
    }

	return real_value;
}

int MapPlayer::modify_blood_by_levelup(const double value, const int fight_tips)
{
	return GameFighter::modify_blood_by_levelup(value, fight_tips);
}

int MapPlayer::modify_magic_by_notify(const double value, const int fight_tips)
{
    int real_magic = GameFighter::modify_magic_by_notify(value, fight_tips);
    return real_magic;
}

int MapPlayer::validate_movable(const MoverCoord &step)
{
    int ret = this->validate_fighter_movable();
    if(ret != 0)
    {
    	MSG_DEBUG("ret %d", ret);
    	return ret;
    }

    if (this->is_movable_coord(step) == false &&
    		CONFIG_INSTANCE->is_border_coord(this->scene_id(), step.pos_x(), step.pos_y()) == false)
    {
        return ERROR_COORD_ILLEGAL;
    }
    return 0;
}

int MapPlayer::request_relive(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400203*, request, RETURN_RELIVE);

    int relive_type = request->relive();
    switch(relive_type)
    {
    case GameEnum::RELIVE_CONFIG_POINT:
    {
        int ret = this->validate_relive_point(relive_type);
        CONDITION_NOTIFY_RETURN(ret == 0, RETURN_RELIVE, ret);
        break;
    }
    case GameEnum::RELIVE_LOCATE:
    {
        int ret = this->validate_relive_locate(request->item_id());
        if (ret == -1)
        {
            return 0;
        }

        CONDITION_NOTIFY_RETURN(ret == 0, RETURN_RELIVE, ret);
    	break;
    }
    default:
    {
    	return this->respond_to_client_error(RETURN_RELIVE, ERROR_CLIENT_OPERATE);
    }
    }

    MoverCoord relive_coord;
    return this->process_relive(relive_type, relive_coord);
}

int MapPlayer::validate_relive_point(int check_type)
{
    const Json::Value &scene_json = this->scene_conf();
    JUDGE_RETURN(scene_json.isMember("relive") == true, ERROR_NO_RELIVE_POINT);

    const Json::Value &relive_json = scene_json["relive"];
    JUDGE_RETURN(relive_json.isMember("type") == true, ERROR_CONFIG_NOT_EXIST);

    JUDGE_RETURN(GameCommon::check_config_value(relive_json["type"].asString(),
    		check_type - 1) == true, ERROR_CONFIG_ERROR);

    int need_wait = relive_json["wait"].asInt();
    int left_wait = this->fight_detail_.left_relive_time(need_wait);
    JUDGE_RETURN(left_wait > 0, 0);

	this->notify_tips_info(GameEnum::FORMAT_MSG_NEED_LIEVE_TIME, left_wait);
	return ERROR_RELIVE_TIME;
}

int MapPlayer::validate_relive_locate(const int item_id)
{
    const Json::Value &scene_json = this->scene_conf();
    JUDGE_RETURN(scene_json.isMember("relive") == true, ERROR_NO_RELIVE_POINT);

    std::string relive_type_str = scene_json["relive"]["type"].asString();
    JUDGE_RETURN(GameCommon::check_config_value(relive_type_str,
    		GameEnum::RELIVE_LOCATE - 1) == true, ERROR_CONFIG_NOT_EXIST);

    // validate item use;
    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->fight_detail_.__relive_tick < nowtime, -1);

    this->fight_detail_.__relive_tick = nowtime + Time_Value(3);

    if (this->left_free_relive_times() > 0)
    {
        ++this->role_detail_.__used_free_relive;
        return 0;
    }
    else
    {
		static int relive_item = CONFIG_INSTANCE->const_set("relive_item");
		this->request_use_item(INNER_RELIVE_USED_ITEM, ITEM_RELIVE_USE, relive_item, 1, 1);
		return -1;
    }
}

int MapPlayer::process_relive(const int relive_mode, MoverCoord &relive_coord)
{
    const Json::Value &relive_json = this->scene_conf()["relive"];

    double percent = 0.1;
    switch (relive_mode)
    {
    case GameEnum::RELIVE_LOCATE:
    {
    	percent = 1.0;
    	relive_coord = this->location();
    	break;
    }
    case GameEnum::RELIVE_CONFIG_POINT:
    default:
    {
        percent = relive_json["state"][0u].asInt() / 100.0;
        relive_coord = this->fetch_config_relive_coord(this->camp_id());
    	break;
    }
    }

    return this->handle_player_relive(relive_coord, percent);
}

int MapPlayer::request_pk_state(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400204 *, request, RETURN_PK_STATE);

    const Json::Value& set_conf = this->scene_set_conf();
	CONDITION_NOTIFY_RETURN(set_conf.isMember("pk_state_locked") == false,
			RETURN_PK_STATE, ERROR_PK_STATE_LOCKED);

    int pk_state = request->pk_state();
    CONDITION_NOTIFY_RETURN(PK_PEACE <= pk_state && pk_state < PK_STATE_END,
    		RETURN_PK_STATE, ERROR_CLIENT_OPERATE);

    this->set_pk_state(pk_state);

    Proto50400204 respond;
    respond.set_pk_state(pk_state);
    FINER_PROCESS_RETURN(RETURN_PK_STATE, &respond);
}

int MapPlayer::request_use_item(int recogn, int serial, int item_id, int amount, int auto_buy, int from)
{
    Proto30400202 request;
    request.set_auto_buy(auto_buy);
    request.set_from(from);

    ProtoItem *item = request.add_item();
    item->set_id(item_id);
    item->set_amount(amount);

    return this->request_use_item(recogn, serial, &request);
}

int MapPlayer::request_use_item(const int recogn, const int serial, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400202 *, request, -1);

    request->set_recogn(recogn);
    request->set_serial(serial);
    request->set_type(1);
    request->set_relive_mode(GameEnum::RELIVE_LOCATE);
    return this->send_to_logic_thread(*request);
}

int MapPlayer::request_insert_item(const int recogn, const int serial, const ItemObjVec& item_set)
{
	JUDGE_RETURN(item_set.empty() == false, -1);
    Proto30400202 request;

    for (ItemObjVec::const_iterator iter = item_set.begin(); iter != item_set.end(); ++iter)
    {
        ProtoItem *item = request.add_item();
        item->set_id(iter->id_);
        item->set_amount(iter->amount_);
        item->set_bind(iter->bind_);
    }

    return this->request_insert_item(recogn, serial, &request);
}

int MapPlayer::request_insert_item(const int recogn, const int serial, const int item_id, const int amount)
{
    Proto30400202 request;

    ProtoItem *item = request.add_item();
    item->set_id(item_id);
    item->set_amount(amount);

    return this->request_insert_item(recogn, serial, &request);
}

int MapPlayer::request_insert_item(const int recogn, const int serial, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400202 *, request, ERROR_CLIENT_OPERATE);

    request->set_recogn(recogn);
    request->set_serial(serial);
    request->set_type(5);

    return this->monitor()->process_inner_logic_request(this->role_id(), *request);
}

int MapPlayer::process_relive_after_used_item(void)
{
	MoverCoord cur_location = this->location();
	return this->process_relive(GameEnum::RELIVE_LOCATE, cur_location);
}

int MapPlayer::map_use_pack_goods(Message* msg)
{
	int ret = this->handle_use_pack_goods(msg);
	JUDGE_RETURN(ret != 0, 0);

	this->send_to_logic_thread(*msg);
	return this->respond_to_client_error(RETURN_USE_GOODS, ret);
}

int MapPlayer::handle_use_pack_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400015*, request, ERROR_SERVER_INNER);

	ProtoItem* proto_item = request->mutable_item_info();
	int item_amount = proto_item->amount(), use_amount = 0;

	const Json::Value& effect = CONFIG_INSTANCE->item(proto_item->id())["effect_prop"];
	JUDGE_RETURN(effect.empty() == false, ERROR_CONFIG_ERROR);

	int ret = 0;
	for (int amount_i = 0; amount_i < item_amount; ++amount_i)
	{
		string prop_name = effect["name"].asString();
		if (prop_name == PropName::HEALTH)
		{
			ret = this->check_and_use_health(proto_item, effect["value"].asInt());
		}
		else if (prop_name == PropName::REDUCE_KILL)
		{
			if (this->killed_info_.killing_value_ <= 0)
			{
				ret = ERROR_KILLING_VALUE_ZERO;
				break;
			}
			this->modify_kill_value(effect["value"].asInt() * -1);
		}

		if (ret != 0)
		{
			break;
		}
		++use_amount;
	}
	proto_item->set_amount(item_amount - use_amount);
	return ret;
}

int MapPlayer::handle_enter_scene_error(int ret)
{
	JUDGE_RETURN(ret != 0, 0);

	if (this->role_detail_.__save_scene.scene_id_ > 0)
	{
		//传送到以前保存的场景
		this->transfer_to_save_scene();
	}
	else
	{
		//传送回出生点
		this->transfer_to_born(true);
	}

	return 0;
}

int MapPlayer::notify_monster_exp(int value)
{
	Proto30100410 request;
	request.set_exp(value);
	return MAP_MONITOR->dispatch_to_logic(this, &request);
}

int MapPlayer::notify_validate_team_info()
{
	JUDGE_RETURN(this->team_info().teamer_set_.empty() == false, 0);
	Proto30100703 request;
	do
	{
		JUDGE_BREAK(GameCommon::is_script_scene(this->scene_id()) == true);

		MapPlayerEx *player = this->self_player();
		JUDGE_BREAK(NULL != player);

		int script_sort = player->script_sort();
		const Json::Value script_json = CONFIG_INSTANCE->script(script_sort);
		JUDGE_BREAK(Json::Value::null != script_json);
		JUDGE_BREAK(script_json["prev_conditon"]["is_single"].asInt() == 0);

		request.set_in_team_script(true);

	}while(0);

	return MAP_MONITOR->dispatch_to_logic(this, &request);
}

int MapPlayer::notify_transfer_scene_info(const SubObj& type)
{
	int scene_id = this->scene_id();
	int flag = this->role_detail_.__scene_history.count(scene_id) == 0;

    Proto50400110 respond;
  	respond.set_type(type.val1_);
  	respond.set_client_type(type.val3_);
	respond.set_is_first_enter(flag);

    respond.set_scene_id(scene_id);
    respond.set_space_id(this->space_id());
    respond.set_pixel_x(this->location().pixel_x());
    respond.set_pixel_y(this->location().pixel_y());
    FINER_PROCESS_RETURN(RETURN_TRANSFER_SCENE, &respond);
}


int MapPlayer::respond_pack_item(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400202*, request, -1);

    if (request->from())
    {
    	MapPlayerEx *player = dynamic_cast<MapPlayerEx *>(this);
    	player->upgrade_escort_level_done(msg);
    }

    int res_recogn = request->recogn(), error = request->error();
    switch (request->recogn())
    {
        case INNER_RELIVE_USED_ITEM:
		{
			res_recogn = RETURN_RELIVE;
			if (error == 0)
			{
				return this->process_relive_after_used_item();
			}
			break;
		}
        default:
            return -1;
    }

    CONDITION_NOTIFY_RETURN(error == 0, res_recogn, error);

    return 0;
}

int MapPlayer::notify_logic_enter_scene(int type)
{
	JUDGE_RETURN(type == ENTER_SCENE_LOGIN, -1);

	Proto30100302 enter_info;
	enter_info.set_last_scene(this->mover_detial_.__prev_scene_id);
	enter_info.set_cur_scene(this->mover_detial_.__scene_id);
	return MAP_MONITOR->dispatch_to_logic(this, &enter_info);
}

int MapPlayer::notify_update_self_id()
{
	Proto80100102 id_info;
	id_info.set_role_id(this->role_id());
	id_info.set_src_role_id(this->src_role_id());
	FINER_PROCESS_RETURN(ACTIVE_UPDATE_PLAYER_ID, &id_info);
}

int MapPlayer::notify_update_fashion_add_prop(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto81400604*, request, -1);
	request->set_fight_force(this->force_total_i());
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_FASHION_ADD_PROP, request);
}

int MapPlayer::notify_update_player_space_id(void)
{
	Proto80400115 respond;
	respond.set_space_id(this->space_id());
	return this->respond_from_broad_client(ACTIVE_UPDATE_SPACE_ID, 0, &respond);
}

int MapPlayer::notify_update_player_info(int update_type, Int64 value)
{
	Proto80400208 respond;
	respond.set_role_id(this->role_id());
	respond.set_update_type(update_type);
	respond.set_value(value);

	switch (update_type)
	{
	case GameEnum::PLAYER_INFO_VIP:
	{
		respond.set_vip_type(this->vip_type());
		break;
	}
	case GameEnum::PLAYER_INFO_EQUIP_SHAPE:
	{
	    respond.set_weapon_id(this->get_shape_item_id(GameEnum::EQUIP_WEAPON));
//	    respond.set_clothes_id(this->get_shape_item_id(GameEnum::EQUIP_YIFU));
		break;
	}
	case GameEnum::PLAYER_INFO_LABEL:
	{
	    respond.set_cur_label(this->get_cur_label());
		break;
	}
	case GameEnum::PLAYER_INFO_LEAGUE:
	{
		respond.set_league_id(this->role_detail_.__league_id);
		respond.set_league_name(this->role_detail_.__league_name);
		break;
	}
	case GameEnum::PLAYER_INFO_ID:
	{
		respond.set_src_role_id(this->src_role_id());
		break;
	}
	case GameEnum::PLAYER_INFO_LEVEL:
	{
		respond.set_role_level(this->level());
		break;
	}
	case GameEnum::PLAYER_INFO_PROTECT:
	{
		BasicStatus *status = 0;
		if (this->find_status(BasicStatus::RELIVE_PROTECT, status) == 0)
		{
			respond.set_is_protect(1);
		}
		else
		{
			respond.set_is_protect(0);
		}
		break;
	}
	case GameEnum::PLAYER_INFO_MAGICWEAPON:
	{
		respond.set_mw_id(this->magic_weapon_id());
		respond.set_mw_rank_lvl(this->magic_weapon_lvl());
		break;
	}
	case GameEnum::PLAYER_INFO_HOTSRING:
	{
		if (value > 0)
		{
			respond.set_hotspring_status(1);
			respond.set_wedding_id(value);	//结婚ID作为双修玩家ID
		}
		else
		{
			respond.set_hotspring_status(0);
			respond.set_wedding_id(0);
		}
		break;
	}
	case GameEnum::PLAYER_INFO_SWORD_POOL:
	{
		respond.set_sword_pool(this->fetch_spool_style_lvl());
		break;
	}
	case GameEnum::PLAYER_INFO_FASHION:
	{
		respond.set_fashion_id(value);
		respond.set_fashion_color(this->fetch_fashion_color());
		break;
	}
	case GameEnum::PLAYER_INFO_TRANSFER:
	{
		respond.set_transfer_id(value);
		break;
	}
	case GameEnum::PLAYER_INFO_WEDDING:
	{
		respond.set_partner_id(this->role_detail_.__partner_id);
		respond.set_partner_name(this->role_detail_.__partner_name);
		respond.set_wedding_id(this->role_detail_.__wedding_id);
		respond.set_wedding_type(this->role_detail_.__wedding_type);
		break;
	}
	}

	return this->respond_to_broad_area(&respond);
}


int MapPlayer::check_is_near_npc(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400231 *, request, -1);

	if (this->is_npc_nearby(request->npc_id()) == false)
	{
		return this->respond_to_client_error(request->recogn() + 40000000, ERROR_TASK_NPC_DISTANCE);
	}

	return this->monitor()->process_inner_logic_request(this->role_id(), *request);
}

int MapPlayer::sync_update_player_name(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400020 *, request, -1);
    this->role_detail_.set_name(request->role_name());

    Proto81400105 respond;
    respond.set_role_id(this->role_id());
    respond.set_role_name(this->role_name());
    return this->respond_to_broad_area(&respond);
}

int MapPlayer::sync_update_player_sex()
{
	this->role_detail_.set_sex();
	return 0;
}

int MapPlayer::send_festival_icon_to_logic()
{
	int drop_act = MAP_MONITOR->festival_icon_type();

	Proto31400053 inner;
	inner.set_drop_act(drop_act);
	return this->send_to_logic_thread(inner);
}

int MapPlayer::send_big_act_state_to_logic()
{
	int is_big_act_time = MAP_MONITOR->is_in_big_act_time();

	Proto31400056 inner;
	inner.set_is_big_act_time(is_big_act_time);
	return this->send_to_logic_thread(inner);
}

int MapPlayer::check_is_near_finish_npc(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400502 *, request, -1);

    int task_id = request->task_id();
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
    JUDGE_RETURN(task_json.empty() == false, -1);

    int finish_npc = task_json["finish"]["npc"].asInt();
    if (this->is_npc_nearby(finish_npc) == false)
    {
        Proto81400201 respond;
        respond.set_task_id(task_id);
        this->respond_to_client(ACTIVE_TASK_TRANSFER, &respond);
    }
    return 0;
}

int MapPlayer::fetch_role_detail()
{
	Proto50400007 role_info;
	role_info.set_color(this->fetch_name_color());
	role_info.set_kill_value(this->killed_info_.killing_value_);
	FINER_PROCESS_RETURN(RETURN_FETCH_ROLE_DETAIL, &role_info);
}

int MapPlayer::sync_add_exp(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400013 *, request, -1);

	SerialObj serial_obj(request->serial_obj());
    this->modify_element_experience(request->add_exp(), serial_obj);

    return 0;
}

int MapPlayer::sync_add_exp_percent(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400010 *, request, -1);

	int id = request->id();
	int total_last = request->last();

	BasicStatus* status = NULL;
	if (this->find_status(id, status) == 0)
	{
		total_last += status->left_time();
	}

    return this->insert_defender_status(this, id,
    		0, total_last, 0, 0, request->percent());
}

int MapPlayer::refresh_buff_status(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30102101*, request, -1);

	int buff_id = request->buff_id();
	int total_last = request->buff_time();

	BasicStatus* status = NULL;
	if (this->find_status(buff_id, status) == 0)
	{
		total_last = status->left_time();
	}

	if (request->status())
	{
	    this->insert_defender_status(this, buff_id, 0, total_last, 0, 0,  GameEnum::DAMAGE_ATTR_PERCENT);
	}
	else
	{
		this->remove_status(buff_id);
	}

	return 0;
}

int MapPlayer::add_quintuple_exp_percent(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30102002*, request, -1);

	int id = BasicStatus::QUINTUPLE_ADD_EXP;
	int total_last = request->buff_time();

	BasicStatus* status = NULL;
	if (this->find_status(BasicStatus::QUINTUPLE_ADD_EXP, status) == 0)
	{
		total_last = status->left_time();
	}

    return this->insert_defender_status(this, id,
    		0, total_last, 0, 0,  GameEnum::QUINTUPLE_BUFF_PERCENT);
}

int MapPlayer::remove_quintuple_exp_percent(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30102004*, request, -1);

	if (request->status() == true)
		this->remove_status(BasicStatus::QUINTUPLE_ADD_EXP);

	return 0;
}

int MapPlayer::sync_direct_add_blood(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400014 *, request, -1);
	this->modify_blood_by_fight(-1 * request->add_blood(), FIGHT_TIPS_USE_PROPS);
	return 0;
}

int MapPlayer::sync_direct_add_magic(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400009 *, request, -1);
	this->modify_magic_by_notify(-1 * request->add_magic(), FIGHT_TIPS_SYSTEM_AUTO);
	return 0;
}

int MapPlayer::fetch_enemy_position(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101602 *, request, -1);
	Int64 role_id = request->role_id();
	Int64 enemy_id = request->enemy_id();

	MapPlayer* player = this->find_player(enemy_id);
	JUDGE_RETURN(player != NULL, -1);

	MoverCoord& location = player->location();
	Proto30101602 inner_req;
	inner_req.set_scene_id(player->scene_id());
	inner_req.set_role_id(role_id);
	inner_req.set_enemy_id(enemy_id);
	inner_req.set_pixel_x(location.pixel_x());
	inner_req.set_pixel_y(location.pixel_y());

	return MAP_MONITOR->dispatch_to_logic(this,&inner_req);
}

int MapPlayer::fetch_enemy_translate(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10401305 *, request, -1);
	return 0;
}

int MapPlayer::fetch_nearby_player(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30101603 *, request, -1);
//    Int64 role_id = request->role_id();
//    MapPlayer* player = this->find_player(role_id);
//    if(player != NULL)
//    {
//    	Scene::MoverMap player_map;
//    	Scene *scene = player->fetch_scene();
//    	int radius = 114; //1.5屏半径
//    	if(scene->fetch_all_around_player(this, player_map, player->location(), radius, 100) == 0)
//    	{
//    		Proto30101603 inner_req;
//    		for (MoverMap::iterator player_iter = player_map.begin();
//    				player_iter != player_map.end(); ++player_iter)
//    		{
//    			 Int64 other_id = player_iter->first;
//    			 if(other_id == role_id)
//    			 {
//    				 continue;
//    			 }
//    			 MapPlayer* other_player = this->find_player(other_id);
//
//    			 if(other_player != NULL)
//    			 {
//    				 ProtoFriendInfo *info_proto = inner_req.add_nearby_list();
//    				 info_proto->set_role_id(other_id);
//    				 info_proto->set_pixel_x(other_player->location().pixel_x());
//    				 info_proto->set_pixel_y(other_player->location().pixel_y());
//    				 info_proto->set_scene_id(other_player->scene_id());
//
//    				 int name_color = other_player->fetch_name_color();
//    				 info_proto->set_name_color(name_color);
//    				// info_proto->set_distance(3);
//    			 }
//
//    		}
//        return this->monitor()->dispatch_to_logic(this,&inner_req);
//    	}
//
//    }
	return 0;
}

int MapPlayer::sync_collect_item(const int item_id, const int num)
{
    Proto31400015 request;
    request.set_item_id(item_id);
    request.set_num(num);
    return this->send_to_logic_thread(request);
}

int MapPlayer::sync_ext_restore_info(int event_id, int value, int times)
{
	Proto31400401 sync_info;
	sync_info.set_event_id(event_id);
	sync_info.set_value(value);
	sync_info.set_times(times);

	return this->monitor()->process_inner_logic_request(this->role_id(), sync_info);
}

int MapPlayer::update_wedding_info(Int64 partner_id, const string& partner_name,
		Int64 wedding_id, int wedding_type)
{
	this->role_detail_.__partner_id = partner_id;
	this->role_detail_.__partner_name = partner_name;
	this->role_detail_.__wedding_id = wedding_id;
	this->role_detail_.__wedding_type = wedding_type;

	if (this->role_detail_.__wedding_id <= 0)
	{
		this->role_detail_.__partner_id = 0;
		this->role_detail_.__partner_name.clear();
		this->role_detail_.__wedding_type = 0;
	}

    return this->notify_update_player_info(GameEnum::PLAYER_INFO_WEDDING);
}

int MapPlayer::sync_restore_info(int event_id, int value, int times)
{
	Proto31400402 sync_info;
	sync_info.set_event_id(event_id);
	sync_info.set_value(value);
	sync_info.set_times(times);
	return this->send_to_logic_thread(sync_info);
}

int MapPlayer::sync_branch_task_info(int event_id, int value)
{
	Proto31400023 sync_info;
	sync_info.set_id(event_id);
	sync_info.set_num(value);
	return this->send_to_logic_thread(sync_info);

}

int MapPlayer::act_serial_info_update(int type, Int64 id, Int64 value, int sub_1, int sub_2)
{
	Proto30103102 request;
	request.set_type(type);
	request.set_id(id);
	request.set_value(value);
	request.set_sub_1(sub_1);
	request.set_sub_2(sub_2);

	return MAP_MONITOR->dispatch_to_logic(this, &request);
}

int MapPlayer::sync_to_ml_activity_finish(const int activity_type, const int sub_type, const int value)
{
    Proto31400018 request;
    request.set_activity_type(activity_type);
    request.set_sub_type(sub_type);
    request.set_value(value);

    return this->send_to_logic_thread(request);
}

int MapPlayer::sync_to_logic_single_script_rank(const int rank_type, const int rank_value)
{
	Proto31401601 request;
	request.set_rank_type(rank_type);
	request.set_rank_value(rank_value);
	request.set_role_id(this->role_id());
	request.set_achieve_tick(::time(0));
	request.set_role_name(this->role_name());
	request.set_rank_force(this->force_total_i());

	return this->monitor()->dispatch_to_logic(&request);
}

int MapPlayer::sync_info_to_map_logic(void)
{
	Proto31400106 sync_info;
	sync_info.set_lvl(this->fight_detail().__level);
	sync_info.set_fight_force(this->force_total_i());
	sync_info.set_scene_id(this->scene_id());
	sync_info.set_scene_mode(this->scene_mode());
	sync_info.set_space_id(this->space_id());
	return this->send_to_logic_thread(sync_info);
}

int MapPlayer::sync_fighter_status_to_map_logic(Int64 attacker)
{
	Proto31400016 request;
	request.set_attackor(attacker);
	request.set_death_flag(this->is_death());
	request.set_mount_flag(this->is_on_mount());
	return this->send_to_logic_thread(request);
}

int MapPlayer::pick_up_drop_goods_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400002*, request, RETURN_PICKUP_GOODS);
	CONDITION_NOTIFY_RETURN(this->is_enter_scene() == true, RETURN_PICKUP_GOODS,
			ERROR_CLIENT_OPERATE);
//	CONDITION_NOTIFY_RETURN(this->is_death() == false, RETURN_PICKUP_GOODS, ERROR_PLAYER_DEATH);

	AIDropPack* drop_pack = AIDROP_PACKAGE->find_object(request->drop_id());
	CONDITION_NOTIFY_RETURN(drop_pack != NULL, RETURN_PICKUP_GOODS,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(drop_pack->scene_id() == this->scene_id()
			&& drop_pack->space_id() == this->space_id(),
			RETURN_PICKUP_GOODS, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(check_coord_distance(this->location(), drop_pack->location(), 20) == true,
			RETURN_PICKUP_GOODS, ERROR_CLIENT_OPERATE);

    int ret = drop_pack->validate_pick_up(this->role_id());
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PICKUP_GOODS, ret);

	Proto31400013 pickup_info;
	pickup_info.set_drop_id(request->drop_id());
	pickup_info.set_src_id(drop_pack->drop_detail().ai_id_);

	drop_pack->make_up_item_info(&pickup_info);
	drop_pack->make_up_money_info(&pickup_info);

	drop_pack->set_pick_up_prep(this->role_id());
	pickup_info.set_sort_id(drop_pack->drop_detail().ai_sort_);
	return this->send_to_logic_thread(pickup_info);
}

int MapPlayer::pick_up_drop_goods_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400012*, request, -1);

	AIDropPack* drop_pack = AIDROP_PACKAGE->find_object(request->drop_id());
	CONDITION_NOTIFY_RETURN(drop_pack != NULL, RETURN_PICKUP_GOODS,
			ERROR_CLIENT_OPERATE);

	Proto50400002 pick_info;
	pick_info.set_drop_id(request->drop_id());

	int result_flag = request->result_flag();
	if (result_flag != 0)
	{
		drop_pack->pick_up_failure(this->role_id());
		this->respond_to_client_error(RETURN_PICKUP_GOODS, result_flag, &pick_info);
	}
	else
	{
        this->process_pick_up_suceess(drop_pack);
		drop_pack->pick_up_shout_drop_goods(this);

		this->respond_from_broad_client(RETURN_PICKUP_GOODS,  0, &pick_info);

		drop_pack->pick_up_suceess();
	}

	return 0;
}

int MapPlayer::process_pick_up_suceess(AIDropPack *drop_pack)
{
    return 0;
}

int MapPlayer::gather_state_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400010*, request, RETURN_GATHER_GOODS);

	Int64 AI_ID = request->gather_good_id();
	this->fight_detail_.gather_state_ = 1;
	this->fight_to_takeoff_mount_tick();

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, ERROR_COLLECTION_NOT_EXIT);

	GameAI* monster = scene->find_ai(AI_ID);
	JUDGE_RETURN(monster != NULL, ERROR_COLLECTION_NOT_EXIT);

	const Json::Value& monster_conf = CONFIG_INSTANCE->monster(monster->ai_sort());
	CONDITION_NOTIFY_RETURN(monster_conf != Json::Value::null, RETURN_GATHER_GOODS, ERROR_SERVER_INNER);
//	CONDITION_NOTIFY_RETURN(monster_conf["monster_type"].asInt() != 1, RETURN_GATHER_GOODS, ERROR_SERVER_INNER);

	Proto50400010 return_info;
	return_info.set_gather_good_id(AI_ID);
	ProtoThreeObj* temp = return_info.mutable_other_info();
	if (monster->ai_detail().__toucher_map.empty())
	{
		temp->set_value(1);
	}
	else
	{
		temp->set_value(0);
	}

	if (monster_conf["monster_type"].asInt() == 1) temp->set_value(1);

	this->respond_to_client(RETURN_GATHER_STATE_B, &return_info);

	if (temp->value())
	{
		Proto80400320 respond;
		respond.set_role_id(this->role_id());
		respond.set_state(1);
		respond.set_gather_good_id(request->gather_good_id());
		monster->gather_limit_collect_begin(this->role_id());
		this->respond_to_broad_area(&respond);
	}

	return 0;
}

int MapPlayer::gather_state_end()
{
	this->fight_detail_.gather_state_ = 0;
    this->fight_detail_.__gather_tick = Time_Value::gettimeofday() + Time_Value(3);

	Proto80400320 respond;
	respond.set_role_id(this->role_id());
	respond.set_state(0);
	this->respond_to_broad_area(&respond);
	FINER_PROCESS_NOTIFY(RETURN_GATHER_STATE_E);
}

int MapPlayer::gather_goods_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400003*, request, RETURN_GATHER_GOODS);

	this->gather_state_end();
	CONDITION_NOTIFY_RETURN(this->is_death() == false, RETURN_GATHER_GOODS, ERROR_PLAYER_DEATH);

	Scene* scene = this->fetch_scene();
	CONDITION_NOTIFY_RETURN(scene != NULL, RETURN_GATHER_GOODS, ERROR_SCENE_NOT_OPEN);

	GameAI* game_ai = scene->find_ai(request->goods_id());
	CONDITION_NOTIFY_RETURN(game_ai != NULL, RETURN_GATHER_GOODS, ERROR_COLLECTION_NOT_EXIT);

	CONDITION_NOTIFY_RETURN(game_ai->is_gather_goods() == true,
			RETURN_GATHER_GOODS, ERROR_CLIENT_OPERATE);

	int ret = scene->validate_ai_pickup(game_ai, this);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_GATHER_GOODS, ret);

	ret = game_ai->gather_limit_collect_begin(this->role_id());
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_GATHER_GOODS, ret);

	const Json::Value& gather_conf = CONFIG_INSTANCE->gather(game_ai->ai_sort());
	int gather_type = gather_conf["gather_type"].asInt();
	if (scene->scene_id() == GameEnum::COLLECT_CHESTS_SCENE_ID && gather_type == GameEnum::GATHER_CHESTS)
	{
		COLLECTCHESTS_INSTANCE->handle_activity_ai_die(this->role_id());
	}

	Proto31400017 gather_info;
	gather_info.set_goods_id(request->goods_id());
	gather_info.set_monster_sort(game_ai->ai_sort());
	return this->send_to_logic_thread(gather_info);
}

int MapPlayer::gather_goods_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400017*, request, RETURN_GATHER_GOODS);
	CONDITION_NOTIFY_RETURN(this->is_death() == false, RETURN_GATHER_GOODS, ERROR_PLAYER_DEATH);

	Scene* scene = this->fetch_scene();
	CONDITION_NOTIFY_RETURN(scene != NULL, RETURN_GATHER_GOODS, ERROR_SCENE_NOT_OPEN);

	GameAI* game_ai = scene->find_ai(request->goods_id());
	CONDITION_NOTIFY_RETURN(game_ai != NULL, RETURN_GATHER_GOODS, ERROR_COLLECTION_NOT_EXIT);

	ItemObj gather_item;
	gather_item.unserialize(request->gather_item());
	game_ai->gather_limit_collect_done(this->role_id(), request->gather_flag(), gather_item);

	Proto50400003 gather_result;
	gather_result.set_gather_flag(request->gather_flag() == 0);
	FINER_PROCESS_RETURN(RETURN_GATHER_GOODS, &gather_result);
}

int MapPlayer::fight_to_takeoff_mount_tick(void)
{
	return 0;
}

int MapPlayer::timeout_takeon_mount(void)
{
  	return 0;
}

void MapPlayer::update_fight_state(const Int64 fighter_id, const int state, const Time_Value &last_tick)
{
    bool prev_state = this->is_fight_state();
    int prev_state_val = this->fight_detail_.__fight_state;

    GameFighter::update_fight_state(fighter_id, state, last_tick);

    bool after_state = this->is_fight_state();
    int after_state_val = this->fight_detail_.__fight_state;

    if (prev_state != after_state || prev_state_val != after_state_val)
    {
    	Proto31400019 req;
    	req.set_fight_state(after_state_val);
    	req.set_fight_tick_sec(this->fight_detail_.__fight_tick.sec());
    	req.set_fight_tick_usec(this->fight_detail_.__fight_tick.usec());
    	this->send_to_logic_thread(req);
    }
}

void MapPlayer::update_active_fight_state(const Int64 target_id)
{
	JUDGE_RETURN(this->role_id() != target_id, ;);
	this->set_cur_beast_aim(target_id);
}

int MapPlayer::process_skill_post_launch(int skill_id, int source)
{
	FighterSkill* skill = 0;
	JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, -1);

	this->add_used_times_and_level_up(skill);
	this->process_rate_use_auto_skill_in_fight(skill);
	return 0;
}

int MapPlayer::refresh_fight_state(const Time_Value &nowtime)
{
    bool prev_state = (this->fight_detail_.__fight_tick > Time_Value::zero);
    int ret = GameFighter::refresh_fight_state(nowtime);
    bool after_state = (this->fight_detail_.__fight_tick > Time_Value::zero), gather_out_state = false;

    if (prev_state == true && after_state == false)
    {
		// 同步战斗状态
		Proto31400019 req;
		req.set_fight_state(this->fight_detail_.__fight_state);
		req.set_fight_tick_sec(this->fight_detail_.__fight_tick.sec());
		req.set_fight_tick_usec(this->fight_detail_.__fight_tick.usec());
		this->send_to_logic_thread(req);
    }
    else if (prev_state == false && after_state == false
    		&& this->fight_detail_.__gather_tick != Time_Value::zero && this->fight_detail_.__gather_tick <= nowtime
    		&& this->is_gather_state() == false)
    {
    	this->notify_fight_state(0);
    	gather_out_state = true;
    }

    return ret;
}

int MapPlayer::transfer_to_born(int command_flag)
{
    int scene_id = 10600;
	const Json::Value &relive_conf = CONFIG_INSTANCE->scene(scene_id)["relive"];

    MoverCoord transfer_coord;
    transfer_coord.set_pixel(relive_conf["fixed_pos"][0u].asInt(),
			relive_conf["fixed_pos"][1u].asInt());

	return this->transfer_dispatcher(scene_id, transfer_coord);
}

int MapPlayer::transfer_to_main_town()
{
	JUDGE_RETURN(this->add_validate_operate_tick(3, GameEnum::MAIN_TOWN_OPERATE) == true, -1);

	const Json::Value& main_town_pos = CONFIG_INSTANCE->const_set_conf(
			"main_town_position")["arr"];
	JUDGE_RETURN(main_town_pos != Json::Value::null, -1);

    MoverCoord transfer_coord;
    transfer_coord.set_pixel(main_town_pos[1u].asInt(),
    		main_town_pos[2u].asInt());

	return this->transfer_dispatcher(main_town_pos[0u].asInt(), transfer_coord);
}

int MapPlayer::transfer_to_prev_scene()
{
	int prev_scene = this->mover_detial_.__prev_scene_id;
	JUDGE_RETURN(prev_scene > 0, -1);

	MoverCoord prev_coord = this->prev_location();
	return this->transfer_dispatcher(prev_scene, prev_coord,
			this->prev_scene_mode(), this->prev_space_id());
}

int MapPlayer::transfer_to_save_scene()
{
	SaveSceneInfo& scene_info = this->role_detail_.__save_scene;;
	JUDGE_RETURN(scene_info.scene_id_ > 0, -1);

	MoverCoord save_location = scene_info.coord_;
	return this->transfer_dispatcher(scene_info.scene_id_, save_location);
}

int MapPlayer::transfer_to_point(Message *msg, bool free_transefer/*=false*/)
{
	JUDGE_RETURN(this->is_enter_scene() == true, 0);
	JUDGE_RETURN(this->add_validate_operate_tick() == true, 0);

	MSG_DYNAMIC_CAST_NOTIFY(Proto10400111*, request, RETURN_TRANSFER_POINT);
	MSG_USER("log transfer 0 %d %d %d", request->scene_id(), request->pixel_x(),
			request->pixel_y());

	JUDGE_RETURN(GameCommon::is_trvl_wboss_scene(request->scene_id()) == false, 0);

	int ret = this->validate_player_transfer(false);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRANSFER_POINT, ret);

	MoverCoord transfer_coord;
	transfer_coord.set_pixel(request->pixel_x(), request->pixel_y());
	CONDITION_NOTIFY_RETURN(GameCommon::is_movable_coord_no_border(request->scene_id(),
			transfer_coord) == true, RETURN_TRANSFER_POINT, ERROR_COORD_ILLEGAL);

	MSG_USER("log transfer 1 %d %d %d", request->scene_id(), transfer_coord.pixel_x(),
			transfer_coord.pixel_y());

	switch(request->type())
	{
	case 1:
	{
		Proto31400501 inner;
		inner.set_scene_id(request->scene_id());
		inner.set_pixel_x(request->pixel_x());
		inner.set_pixel_y(request->pixel_y());
		inner.set_client_type(request->client_type());
		return this->send_to_logic_thread(inner);
	}
	default:
	{
		SubObj sub(0, 0, request->client_type());
		SceneInfo scene(request->scene_id(), 0, 0);

		this->remove_status(BasicStatus::JUMPING);
		this->transfer_dispatcher_b(scene, transfer_coord, sub);

		return 0;
	}
	}
}

int MapPlayer::transfer_fee_deduct_finish(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400501 *, request, -1);

	int ret = request->oper_result();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRANSFER_POINT, ret);

	//去除跳跃
	this->remove_status(BasicStatus::JUMPING);

	MoverCoord point;
	point.set_pixel(request->pixel_x(), request->pixel_y());

	SceneInfo scene(request->scene_id(), this->scene_mode());
	SubObj sub(1, 0, request->client_type());
	return this->transfer_dispatcher_b(scene, point, sub);
}

int MapPlayer::refresh_random_location(void)
{
    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    // fetch random coord;
    MoverCoord random_coord = scene->fetch_random_coord();
    JUDGE_RETURN(this->is_movable_coord(random_coord) == true, -1);

    return this->refresh_aim_location(random_coord);
}

int MapPlayer::refresh_aim_location(const MoverCoord& aim_location)
{
    JUDGE_RETURN(this->is_movable_coord(aim_location) == true, -1);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    scene->refresh_mover_location(this, aim_location, false);
    this->notify_transfer_scene_info();
    return 0;
}

int MapPlayer::fetch_role_panel_info(Message* msg)
{
	return 0;
}

int MapPlayer::fetch_single_player_all_detail(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto50100156*, respond, -1);
	Int64 player_id = respond->role_id();

	MapPlayerEx *player = NULL;
	if(MAP_MONITOR->find_player(player_id, player) != 0)
	{
		MSG_USER(%ld, player_id);
		return MAP_MONITOR->process_inner_logic_request(player_id, *respond);
	}

//	MapPlayerEx *req_player = NULL;
//	if (this->monitor()->find_player(respond->checker_id(), req_player) == 0)
//		respond->set_gate_sid(req_player->gate_sid());
//	else
//		respond->set_gate_sid(this->gate_sid());
	this->make_up_single_player_detail(respond);

	return MAP_MONITOR->process_inner_logic_request(player_id, *respond);
}

int MapPlayer::make_up_single_player_detail(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto50100156*, respond, -1);

	respond->set_role_name(this->role_name());
	respond->set_sex(this->role_detail_.__sex);
	respond->set_career(this->role_detail_.__career);
	respond->set_level(this->level());
	respond->set_league_id(this->role_detail_.__league_id);
	respond->set_league_name(this->role_detail_.__league_name);
	respond->set_vip_status(this->vip_detail().__vip_level);

	respond->set_fight_force(this->force_total_i());
	respond->set_attack(this->fight_detail_.__attack_total_i(this));
	respond->set_defence(this->fight_detail_.__defence_total_i(this));
	respond->set_max_blood(this->fight_detail_.__blood_total_i(this));

	respond->set_hit(this->fight_detail_.__hit_total_i(this)); // 命中
	respond->set_avoid(this->fight_detail_.__avoid_total_i(this)); //闪避
	respond->set_crit(this->fight_detail_.__crit_total_i(this)); // 暴击
	respond->set_toughness(this->fight_detail_.__toughness_total_i(this));

	int cirt_hurt_rate 	= this->fight_detail_.__crit_hurt_multi_total(this) * 100;
	int damage_rate		= this->fight_detail_.__damage_rate_total(this) * 100;
	int reduction_rate 	= this->fight_detail_.__reduction_rate_total(this) * 100;

	respond->set_crit_hurt(cirt_hurt_rate);	//暴击伤害率
	respond->set_damage(damage_rate);	//伤害加成率
	respond->set_reduction(reduction_rate);	//伤害减免率
	respond->set_pk(this->killed_info_.pk_value());	//PK值
	respond->set_glamour(this->fight_detail_.__glamour);	//魅力值

	respond->set_fashion_id(this->fetch_fashion_id());
	respond->set_fashion_color(this->fetch_fashion_color());

	return 0;
}

int MapPlayer::fetch_ranker_detail(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto50100702 *, respond, -1);
    JUDGE_RETURN(this->role_id() == respond->role_id(), -1);

    respond->set_label(this->get_cur_label());
    respond->set_weapon(this->get_shape_item_id(GameEnum::EQUIP_WEAPON));
    respond->set_clothes(this->get_shape_item_id(GameEnum::EQUIP_YIFU));
    respond->set_fashion_clothes(this->fetch_fashion_id());
    respond->set_fashion_weapon(this->fetch_fashion_color());
//    respond->set_fashion_weapon(this->get_shape_item_id(GameEnum::FASHION_WEAPON));
//    respond->set_fashion_clothes(this->get_shape_item_id(GameEnum::FASHION_YIFU));
//    respond->set_weapon_refine_lvl(this->map_equip_detail().weapon_lvl_);

    return this->monitor()->process_inner_logic_request(respond->role_id(), *respond);
}

int MapPlayer::process_fetch_self_ranker_info(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30401601 *, request, -1);

    int rank_type = request->rank_type();

    Proto31402302 inner_req;
    inner_req.set_rank_type(rank_type);
    inner_req.set_data_type(request->data_type());
    if (rank_type == RANK_SINGLE_SCRIPT_ZYFM)
    {
        MapPlayerEx *player = dynamic_cast<MapPlayerEx *>(this);
        int pass_piece = player->script_detail().__piece_record.__pass_piece,
            pass_chapter = player->script_detail().__piece_record.__pass_chapter;
        inner_req.set_rank_value(pass_piece * 1000 + pass_chapter);
    }
    else
    {
        MSG_USER("ERROR can't read rank_type %d", rank_type);
        return -1;
    }
    return this->monitor()->dispatch_to_logic(this, &inner_req);
}

int MapPlayer::update_copy_role_id()
{
	Int64 copy_id = MAP_MONITOR->generate_role_copy_id();
    this->notify_exit_scene_cancel_info(EXIT_SCENE_JUMP);
	GameFighter::exit_scene(EXIT_SCENE_JUMP);

	this->src_role_id_ = this->role_id_;
	this->role_id_ = copy_id;

	GameFighter::enter_scene(ENTER_SCENE_JUMP);
	this->notify_update_player_info(GameEnum::PLAYER_INFO_ID);

	return 0;
}

int MapPlayer::request_glide(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400113 *, request, RETURN_PLAYER_GLIDE);

    MoverCoord target;
    target.set_pixel(request->pixel_x(), request->pixel_y());
    CONDITION_NOTIFY_RETURN(this->is_movable_coord(target) == true,
    		RETURN_PLAYER_GLIDE, ERROR_COORD_ILLEGAL);

    int action_type = request->action_type();
    switch (action_type)
    {
    case 1:
    {
    	//jump
        const Json::Value& jump_conf = this->scene_conf()["jump"];
        CONDITION_NOTIFY_RETURN(jump_conf.empty() == false,
        		RETURN_PLAYER_GLIDE, ERROR_CONFIG_ERROR);

    	break;
    }
    default:
    {
    	return this->respond_to_client_error(RETURN_PLAYER_GLIDE, ERROR_CLIENT_OPERATE);
    }
    }

    Scene *scene = this->fetch_scene();
    CONDITION_NOTIFY_RETURN(scene != 0, RETURN_PLAYER_GLIDE, ERROR_SERVER_INNER);

	Proto80400214 respond;
	respond.set_mover_id(this->mover_id());
	respond.set_action_type(action_type);
	respond.set_pixel_x(target.pixel_x());
	respond.set_pixel_y(target.pixel_y());

	this->all_beast_exit_scene(1);
	this->respond_to_broad_area(&respond);

    scene->refresh_mover_location(this, target, false);
    this->all_beast_enter_scene(1);

    switch (action_type)
    {
    case 1:
    {
    	this->insert_defender_status(this, BasicStatus::JUMPING, 0, 0.4, 0, 0, 0, 1);
    	break;
    }
    }

    FINER_PROCESS_NOTIFY(RETURN_PLAYER_GLIDE);
}

int MapPlayer::fetch_relive_data_common(const Json::Value &relive_json,
		const int relive_mode, int &blood_percent, int &item_id, int &item_amount)
{
	JUDGE_RETURN(relive_json != Json::Value::null, ERROR_CONFIG_ERROR);
	JUDGE_RETURN( GameEnum::RELIVE_CONFIG_POINT <= relive_mode
					&& relive_mode <= GameEnum::RELIVE_LOCATE, ERROR_CLIENT_OPERATE);

    blood_percent = 10;
    item_id = 0;
    item_amount = 0;
    if (relive_mode <= int(relive_json["state"].size()))
        blood_percent = relive_json["state"][relive_mode - 1].asInt();

    if (relive_json.isMember("relive_item") == true)
    {
        if(relive_json["relive_item"].isArray())
        {
    		if (relive_mode <= int(relive_json["relive_item"].size()))
    		{
    			item_id = relive_json["relive_item"][relive_mode - 1].asInt();
    			if (item_id == 0 && relive_json["relive_item"].size() >= 3 && relive_mode == GameEnum::RELIVE_LOCATE)
    				item_id = relive_json["relive_item"][2u].asInt();
    			item_amount = 1;
    		}
        }
        else
        {
        	item_id = relive_json["relive_item"].asInt();
        	item_amount = 1;
        }
    }
    else
    {
    	item_id = 200000008;
    	item_amount = 1;
    }

    return 0;
}

int MapPlayer::fetch_relive_data(const int relive_mode, int &blood_percent,
		int &item_id, int &item_amount)
{
	const Json::Value &relive_json = CONFIG_INSTANCE->scene(this->scene_id())["relive"];
	return fetch_relive_data_common(relive_json, relive_mode, blood_percent, item_id, item_amount);
}

int MapPlayer::process_accelerate_forbit(Message *msg)
{
	//加速定身15s
    MSG_DYNAMIC_CAST_RETURN(Proto30400053 *, request, -1);
    return this->insert_defender_status(this, BasicStatus::DIZZY, 0, 15);
}

void MapPlayer::record_other_serial(int main_serial, int sub_serial, Int64 value,
    		Int64 ext1, Int64 ext2)
{
	SERIAL_RECORD->record_other_serial(SerialPlayer(this->role_id(), this),
			this->agent_code(), this->market_code(), main_serial, sub_serial, value, ext1, ext2);
}

int MapPlayer::init_all_player_skill(void)
{
	int role_level = this->level();
    const GameConfig::ConfigMap &skill_map = CONFIG_INSTANCE->role_skill_map();

    for (GameConfig::ConfigMap::const_iterator iter = skill_map.begin();
    		iter != skill_map.end(); ++iter)
    {
    	int skill_id = iter->first;
    	JUDGE_CONTINUE(this->validate_init_insert_skill(skill_id) == true);
    	JUDGE_CONTINUE(this->is_have_skill(skill_id) == false);

    	this->insert_skill(skill_id);
        JUDGE_CONTINUE(role_level >= 65);

		this->record_other_serial(MAIN_EXECEPTION_SERIAL, 4, skill_id, role_level, 0);
    }

    return 0;
}

int MapPlayer::check_and_use_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30400444*, request, RETURN_USE_GOODS);

	const Json::Value& item_conf = CONFIG_INSTANCE->item(request->id());
	CONDITION_NOTIFY_RETURN(item_conf.empty() == false, RETURN_USE_GOODS,
			ERROR_GOODS_DIRECT_USE);

//	if(prop_config["effect"].isArray() == true)
//	{
//		const Json::Value& effect_config  = prop_config["effect"];
//		 for(uint i = 0 ; i< effect_config.size() ; i++)
//		 {
//			 string prop_name = effect_config[i]["name"].asString();
//			 if (prop_name == PropName::REDUCE_KILL)
//			 {
//			 		CONDITION_NOTIFY_RETURN(this->role_detail_.killing_value_ > 0,
//			 				RETURN_USE_GOODS, ERROR_NO_NEED_USE);
//			 		break;
//			 }
//		 }
//	}
//	else
//	{
//		    string prop_name = prop_config["effect"]["name"].asString();
//			if (prop_name == PropName::REDUCE_KILL)
//			{
//				CONDITION_NOTIFY_RETURN(this->role_detail_.killing_value_ > 0,
//						RETURN_USE_GOODS, ERROR_NO_NEED_USE);
//			}
//	}

	return this->send_to_logic_thread(*msg);
}

int MapPlayer::request_enter_answer_activity()
{
	CONDITION_NOTIFY_RETURN(this->scene_id() != GameEnum::ANSWER_ACTIVITY_SCENE_ID,
			RETURN_ANSWER_ACTIVITY_ENTER, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_ANSWER_ACTIVITY_ENTER, ERROR_NORMAL_SCENE);

	CONDITION_NOTIFY_RETURN(this->get_escort_detail().car_index_ == 0,
			RETURN_ANSWER_ACTIVITY_ENTER, ERROR_ESCORT_START);

	static int need_lvl = CONFIG_INSTANCE->answer_activity_json(GameEnum::ANSWER_ACTIVITY_ID)["open_level"].asInt();
	CONDITION_NOTIFY_RETURN(this->level() >= need_lvl, RETURN_ANSWER_ACTIVITY_ENTER,
			ERROR_NEED_LEVLE_39);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_ANSWER_ACTIVITY);

	int ret = this->send_request_enter_info(GameEnum::ANSWER_ACTIVITY_SCENE_ID, enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ANSWER_ACTIVITY_ENTER, ret);

	return 0;
}

int MapPlayer::request_enter_collect_chests()
{
//	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
//			RETURN_COLLECT_CHESTS_ENTER, ERROR_NORMAL_SCENE);
//
//	static int need_lvl = CONFIG_INSTANCE->collect_chests_json(1)["open_level"].asInt();
//	CONDITION_NOTIFY_RETURN(this->level() >= need_lvl, RETURN_COLLECT_CHESTS_ENTER,
//			ERROR_NEED_LEVLE_39);
//
//	Proto30400051 enter_info;
//	enter_info.set_enter_type(GameEnum::ET_COLLECT_CHESTS);
//
//	int ret = this->send_request_enter_info(GameEnum::COLLECT_CHESTS_SCENE_ID, enter_info);
//	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_COLLECT_CHESTS_ENTER, ret);

	return 0;
}

int MapPlayer::request_add_reward(int id, const SerialObj& obj)
{
	JUDGE_RETURN(id > 0, -1);

	Proto31400323 inner;
	inner.set_id(id);
	obj.serialize(inner.mutable_serial());
	return this->send_to_logic_thread(inner);
}

int MapPlayer::request_add_game_resource(int item_id, int value, const SerialObj& obj)
{
	Proto31400025 inner;
	inner.set_item_id(item_id);
	inner.set_value(value);
	obj.serialize(inner.mutable_serial_obj());
	return this->send_to_logic_thread(inner);
}

int MapPlayer::request_add_mult_item(RewardInfo &reward_info, const SerialObj& obj, int mult)
{
	Proto31400042 inner;
	obj.serialize(inner.mutable_serial_obj());

	for (ItemObjVec::iterator item_iter = reward_info.item_vec_.begin();
	    	item_iter != reward_info.item_vec_.end(); ++item_iter)
	{
		ItemObj &item_obj = *item_iter;
		item_obj.amount_ *= mult;

		ProtoItem *item = inner.add_proto_item();
		item_obj.serialize(item);
	}

	for (IntMap::iterator iter = reward_info.resource_map_.begin();
			iter != reward_info.resource_map_.end(); ++iter)
	{
		iter->second *= mult;
	}

	if (reward_info.exp_ > 0)
	{
		reward_info.exp_ *= mult;
	}

	return this->send_to_logic_thread(inner);
}

int MapPlayer::request_add_single_item(int item_id, int amount, int item_bind, const SerialObj& obj)
{
	Proto31400043 inner;
	inner.set_item_id(item_id);
	inner.set_amount(amount);
	inner.set_bind(item_bind);
	obj.serialize(inner.mutable_serial_obj());
	return this->send_to_logic_thread(inner);
}

//int MapPlayer::stop_watching(int type /*=EXIT_SCENE_WATCH*/, int flag) //
//{
//	return 0;
//}
//
//bool MapPlayer::is_watching(bool strict /*=false*/)
//{
//	return false;
//}

int MapPlayer::request_dart_forward(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400117 *, request, RETURN_DART_FORWARD);
    MSG_USER("player req dart %ld %s -> %ld (%d,%d)", this->role_id(), this->name(),
    		request->target_id(), request->pixel_x(), request->pixel_y());

    //特殊AI不受突进
    Int64 target_id = request->target_id();
    CONDITION_NOTIFY_RETURN(this->special_ai_not_dart_forward(target_id) == false,
    		RETURN_DART_FORWARD, ERROR_TARGET_CANT_DART);

    const Json::Value &skill_json = CONFIG_INSTANCE->skill(request->skill_id());
    CONDITION_NOTIFY_RETURN(skill_json != Json::Value::null, RETURN_DART_FORWARD,
    		ERROR_CONFIG_NOT_EXIST);

	Scene *scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, 0);

	MoverCoord coord;
    coord.set_pixel(request->pixel_x(), request->pixel_y());

    // 修正突进坐标
    bool has_movable_coord = false;
    int org_pos_x = coord.pos_x(), org_pos_y = coord.pos_y();
   	if (GameCommon::is_movable_coord_no_border(this->scene_id(), coord) == false)
    {
   		MoverCoord org_coord = coord;
    	for (int x = org_pos_x - 1 ; x <= org_pos_x + 1; ++x)
    	{
    		for (int y = org_pos_y - 1; y <= org_pos_y + 1; ++y)
    		{
    			if (x == org_pos_x && y == org_pos_y)
    				continue;
    			coord.set_pos(x, y);
    			if (GameCommon::is_movable_coord_no_border(this->scene_id(), coord) == true)
    			{
    				has_movable_coord = true;
    				break;
    			}
    		}
    		if (has_movable_coord == true)
    			break;
    	}

    	if (has_movable_coord == false)
    	{
    	   coord = scene->rand_coord_by_pixel_radius(org_coord, 2 * 30);
    	}
    }
    CONDITION_NOTIFY_RETURN(GameCommon::is_movable_coord_no_border(this->scene_id(), coord) == true,
    		RETURN_DART_FORWARD, ERROR_COORD_ILLEGAL);
    
    if (target_id == this->role_id())
    {
    	int rushRange = skill_json["onrushRange"][1u].asInt();
    	CONDITION_NOTIFY_RETURN(check_coord_pixel_distance(coord, this->location(),
    			MoverCoord::pos_to_pixel(rushRange + 1)) == true, RETURN_DART_FORWARD, ERROR_COORD_OFFSET);

		scene->refresh_mover_location(this, coord, false);

		Proto80400225 respond;
		respond.set_mover_id(this->role_id());
		respond.set_pixel_x(coord.pixel_x());
		respond.set_pixel_y(coord.pixel_y());
		respond.set_attack_id(request->attack_id());
		respond.set_dart_type(request->dart_type());
		this->respond_to_broad_area(&respond);

		if (coord.pixel_x() != request->pixel_x() || coord.pixel_y() != request->pixel_y())
		{
			Proto50400117 ret_msg;
			ret_msg.set_need_correct(1);
			ret_msg.set_pixel_x(coord.pixel_x());
			ret_msg.set_pixel_y(coord.pixel_y());
			this->respond_to_client(RETURN_DART_FORWARD, &ret_msg);
		}
    }
    else
    {
//    	if (GameCommon::fetch_mover_type(target_id) == MOVER_TYPE_MONSTER)
//    	{
//    		GameFighter *fighter = 0;
//    		JUDGE_RETURN(this->find_fighter(target_id, fighter) == 0, -1);
//
//    		fighter->insert_defender_status(fighter, BasicStatus::STAY, 0.0, 1.0);
//
//    		int farthest_distance = skill_json["backrange"][1u].asInt();
//    		CONDITION_NOTIFY_RETURN(check_coord_pixel_distance(coord, fighter->location(),
//    				MoverCoord::pos_to_pixel(farthest_distance)) == true, RETURN_DART_FORWARD, ERROR_COORD_OFFSET);
//
//    		CONDITION_NOTIFY_RETURN(this->is_movable_coord(coord), RETURN_DART_FORWARD, ERROR_TARGET_CANT_DART);
//
//			scene->refresh_mover_location(fighter, coord, false);
//
//			Proto80400225 respond;
//			respond.set_mover_id(target_id);
//			respond.set_pixel_x(coord.pixel_x());
//			respond.set_pixel_y(coord.pixel_y());
//			respond.set_attack_id(request->attack_id());
//			respond.set_dart_type(request->dart_type());
//			fighter->respond_to_broad_area(&respond);
//    	}
//        else
//        {
//            return this->respond_to_client_error(RETURN_DART_FORWARD, ERROR_TARGET_CANT_DART);
//        }
    }
    return 0;
}

int MapPlayer::special_ai_not_dart_forward( Int64 ai_id )
{
	return 0;
}

double MapPlayer::fetch_reduce_hurt_rate()
{
    return GameFighter::fetch_reduce_hurt_rate() + MapMaster::fetch_reduce_hurt_rate();
}

double MapPlayer::fetch_increase_hurt_value()
{
	return GameFighter::fetch_increase_hurt_value();
}

double MapPlayer::fetch_increase_hurt_rate(GameFighter* fighter)
{
	return GameFighter::fetch_increase_hurt_rate(fighter)
		+ MapMaster::fetch_increase_hurt_rate(fighter);
}

double MapPlayer::fetch_sub_skill_use_rate(FighterSkill* skill)
{
	return MapTransfer::fetch_sub_skill_use_rate(skill)
		+ MapMaster::fetch_sub_skill_use_rate(skill);
}

void MapPlayer::refresh_free_relive_times(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->role_detail_.__refresh_free_relive_tick <= nowtime, ;);

    this->role_detail_.__refresh_free_relive_tick = next_day(0, 0, nowtime);
    this->role_detail_.__used_free_relive = 0;
}

int MapPlayer::left_free_relive_times(void)
{
    JUDGE_RETURN(this->vip_type() > 0, 0);

    this->refresh_free_relive_times();

    const Json::Value &vip_json = CONFIG_INSTANCE->vip(this->vip_type());
    int left_times = vip_json["free_relive"].asInt() - this->role_detail_.__used_free_relive;
    if (left_times < 0)
        left_times = 0;
    return left_times;
}

int MapPlayer::process_read_hook_info(Message *msg)
{
    return 0;
}

void MapPlayer::set_role_escort_id(const Int64 id)
{
    this->role_escort_car_ = id;
}

Int64 MapPlayer::role_escort_id(void)
{
    return this->role_escort_car_;
}

GameAI *MapPlayer::role_escort_car(void)
{
    JUDGE_RETURN(this->role_escort_id() > 0, NULL);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, NULL);

    return scene->find_ai(this->role_escort_id());
}

void MapPlayer::set_league_escort_id(const Int64 id)
{
    this->league_escort_car_ = id;
}

Int64 MapPlayer::league_escort_id(void)
{
    return this->league_escort_car_;
}

GameAI *MapPlayer::league_escort_car(void)
{
    JUDGE_RETURN(this->league_escort_id() > 0, NULL);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, NULL);

    return scene->find_ai(this->league_escort_id());
}

int MapPlayer::set_pk_state(const int state)
{
    GameFighter::set_pk_state(state);

    GameAI *game_ai = this->role_escort_car();
    if (game_ai != NULL)
    {
        game_ai->set_pk_state(state);
    }

    game_ai = this->league_escort_car();
    if (game_ai != NULL)
    {
        game_ai->set_pk_state(state);
    }

    this->update_cur_beast_pk_state();
    this->notify_fight_update(FIGHT_UPDATE_PK, this->fight_detail_.__pk_state);
    return 0;
}

int MapPlayer::notify_exit_scene_cancel_info(int type, int scene_id)
{
	JUDGE_RETURN(type == EXIT_SCENE_TRANSFER && scene_id != this->scene_id(), -1);
	return GameFighter::notify_exit_scene_cancel_info(type, scene_id);
}

int MapPlayer::notify_trade_to_mapplayer(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30400243 *, request, msg, -1);

	if(request->betrading())
	{
		this->is_trade_ = true;
		this->responser_id = request->role_id();
	}
	else
	{
		this->is_trade_ = false;
		this->responser_id = 0;
	}
	return 0;
}

bool MapPlayer::trade_state()
{
	return this->is_trade_;
}

Int64 MapPlayer::trade_des_player_id()
{
	return this->responser_id;
}

int MapPlayer::process_flaunt_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200117*, request, -1);

//	switch (request->flaunt_type())
//	{
//		case GameEnum::FLAUNT_POSITION:
//		{
//			if (GameCommon::is_normal_scene(this->scene_id()) == false &&
//					GameCommon::is_value_in_config(CONFIG_INSTANCE->tiny("flaunt_position_scene"), this->scene_id()) == false)
//			{
//				return this->respond_to_client_error(RETURN_CHAT_FLAUNT, ERROR_SCENE_LIMIT_FLAUNT_POSITION);
//			}
//			this->make_up_flaunt_position(msg);
//			break;
//		}
//	}
	return this->monitor()->dispatch_to_chat(this, request);
}

int MapPlayer::make_up_flaunt_position(Message *msg)
{
//	DYNAMIC_CAST_RETURN(Proto30200117 *, request, msg, -1);
//
//	char content_str[65];
//	::snprintf(content_str, 64, "%d,%d,%d", this->scene_id(), this->location().pixel_x(), this->location().pixel_y());
//	content_str[64] = '\0';
//	request->set_show_content(content_str);
//
//	Proto10006 position_info;
//	position_info.set_scene_id(this->scene_id());
//	position_info.set_pixel_x(this->location().pixel_x());
//	position_info.set_pixel_y(this->location().pixel_y());
//
//	request->set_msg(position_info.SerializeAsString());
	return 0;
}

int MapPlayer::enter_rand_scene(Message *msg)
{
	return 0;
}

int MapPlayer::notify_quit_trvl_team(int force)
{
	if (force == false)
	{
		JUDGE_RETURN(this->role_detail_.sign_trvl_team_ == true, -1);
	}

	Proto30400503 inner;
	inner.set_type(1);
	return MAP_MONITOR->dispatch_to_scene_by_noplayer(this, GameEnum::TRVL_SCRIPT_SCENE_ID, &inner);
}

int MapPlayer::notify_quit_normal_team()
{
	JUDGE_RETURN(this->team_empty() == false, 0);
	return MAP_MONITOR->dispatch_to_logic(this, CLIENT_TEAM_QUIT_TEAM);
}

int MapPlayer::notify_killed_info(Int64 benefited)
{
	if (this->is_in_normal_mode() == false)
	{
		const Json::Value& conf = this->scene_conf();
		JUDGE_RETURN(conf.isMember("die_tips") == true, -1);
	}

	GameFighter* fighter = NULL;
	JUDGE_RETURN(this->find_fighter(benefited, fighter) == 0, -1);

	Proto80100112 respond;
	respond.set_fighter_id(benefited);
	respond.set_fighter_name(fighter->name());
	respond.set_force(fighter->force_total_i());
	respond.set_mover_type(fighter->fetch_mover_type());
	FINER_PROCESS_RETURN(ACTIVE_PLAYER_KILLED, &respond);
}

int MapPlayer::notify_get_couple_buy()
{
	Proto30400915 inner;
	inner.set_prev_scene_id(this->scene_id());
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::LEGEND_TOP_SCENE, &inner);
}

int MapPlayer::notify_connect_travel_chat(void)
{
	JUDGE_RETURN(GameCommon::is_travel_scene(this->prev_scene_id()) == true
			|| GameCommon::is_travel_scene(this->scene_id()) == true, -1);
	JUDGE_RETURN(this->role_detail_.notify_trvl_chat_ == false, -1);

	Proto80400116 respond;
	if (GameCommon::is_travel_scene(this->scene_id()) == true)
	{
		respond.set_chat_ip(MAP_MONITOR->chat_ip());
		respond.set_chat_port(MAP_MONITOR->chat_port());

		respond.set_state(1);
		GameCommon::make_up_session(respond.mutable_session_info(),
				this->role_detail_.__account);
	}
	else if (GameCommon::is_normal_scene(this->scene_id()) == false)
	{
		respond.set_state(0);
	}

	this->role_detail_.notify_trvl_chat_ = true;
	FINER_PROCESS_RETURN(ACTIVE_CONNECT_TRAVEL_CHAT, &respond);
}

int MapPlayer::sync_permission_info(Message* msg)
{
	/*
	MSG_DYNAMIC_CAST_RETURN(Proto30400055*, request, -1);

	this->role_detail().__permission = request->permission();
    Block_Buffer buff;
    this->make_up_login_info(&buff, true);
    this->respond_to_client(&buff);

    return this->sync_role_info_to_logic();
    */
	return 0;
}

int MapPlayer::insert_protect_buff(int last/*=0*/, int relive_mode/*=0*/)
{
	JUDGE_RETURN(last > 0, -1);
    this->insert_defender_status(this, BasicStatus::RELIVE_PROTECT, 0, last, 0, 0);
    this->notify_update_player_info(GameEnum::PLAYER_INFO_PROTECT);
    return 0;
}

int MapPlayer::check_travel_timeout(void)
{
    return 0;
}

