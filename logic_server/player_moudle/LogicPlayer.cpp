/*
 * LogicPlayer.cpp
 *
 * Created on: 2013-02-23 10:53
 *     Author: glendy
 */

#include "LogicPlayer.h"
#include "BaseUnit.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"

#include "MMORole.h"
#include "MMOOnline.h"
#include "MMOOpenActivity.h"
#include "MMOLuckyWheel.h"
#include "RankSystem.h"
#include "MMOSocialer.h"
#include "LeagueSystem.h"
#include "MarketSystem.h"
#include "FestActivitySys.h"

#include "MMOLeague.h"
#include "MMORankPannel.h"
#include "MMOActivityTipsSystem.h"

#include "DoubleEscort.h"
#include "QuintupleOnline.h"
#include "SendActReward.h"
#include "InvestRechargeSys.h"
#include "ArenaSys.h"
#include "LogicPhp.h"
#include "GameNoticeSys.h"
#include "CenterPostCache.h"
#include "OpenActivitySys.h"
#include "MongoDataMap.h"
#include "LogicGameSwitcherSys.h"
#include "MMOWedding.h"
#include "ActivityTipsSystem.h"
#include "BackGameSwitcher.h"
#include "BackBrocast.h"
#include "LuckyWheelSys.h"
#include "SerialRecord.h"
#include "BackJYBackActivity.h"
#include "MayActivitySys.h"
#include "GameFont.h"

LogicPlayer::CachedTimer::CachedTimer(void) : player_(0)
{ /*NULL*/ }

LogicPlayer::CachedTimer::~CachedTimer(void)
{ /*NULL*/ }

int LogicPlayer::CachedTimer::type(void)
{
    return GTT_LOGIC_ONE_SEC;
}

int LogicPlayer::CachedTimer::handle_timeout(const Time_Value &nowtime)
{
    return this->player_->time_up(nowtime);
}

LogicPlayer::LogicPlayer(void)
{
    this->gate_sid_ = 0;
    this->role_id_ = 0;
    this->is_new_role_ = false;
    this->ban_speak_ = 0;
    this->ban_speak_expired_ = 0;
    this->is_first_area_ = true;
    this->is_notify_msg_ = false;
    this->monitor_ = LOGIC_MONITOR;
    this->cached_timer_.player_ = this;
    this->is_sgin_in.clear();
    this->client_mac_.clear();

    static double timeup_inter[] = {0, SAVE_TIMEOUT, 10};
    static int timeup_inter_size = ARRAY_LENGTH(timeup_inter);
    this->cache_tick_.init(LogicPlayer::CACHE_END, LogicPlayer::TIMEUP_END, timeup_inter, timeup_inter_size);
}

LogicPlayer::~LogicPlayer(void)
{ /*NULL*/ }

void LogicPlayer::reset(void)
{
    this->role_id_ = 0;
    this->gate_sid_ = 0;
    this->is_sgin_in.clear();

    this->is_new_role_ = false;
    this->is_notify_msg_ = false;
    this->is_first_area_ = true;

    this->ban_speak_ = 0;
    this->ban_speak_expired_ = 0;

    this->client_ip_.clear();
    this->session_.clear();
    this->uc_sid_.clear();
    this->client_mac_.clear();
    this->role_detail_.reset();
    this->hook_detail_.reset();
    this->socialer_detail_.reset();
    this->online_.reset();
    this->vip_detail_.reset();

    EntityCommunicate::reset_entity();
    LogicTeamer::reset();
    LogicLeaguer::reset();
    LogicCustomer::reset();
    LogicRankPlayer::reset();
    LogicActivityReminder::reset();
    LogicWeddingPlayer::reset_wedding();
    LogicLuckyTable::reset();
    LogicActivityer::reset_activityer();
    LogicMayActivityer::reset_may_activityer();
    LogicWheelPlayer::reset();
    DailyActPlayer::reset();
    LogicBackActivityer::reset_back_activity();
    TrvlTeamPlayer::reset_travel_teamer();
}

LogicMonitor *LogicPlayer::monitor(void)
{
    return this->monitor_;
}

int LogicPlayer::respond_to_client_error(const int recogn, const int error, const Message *msg_proto)
{
    if (msg_proto == 0)
    {
        return this->monitor()->dispatch_to_client(this->gate_sid(),
        		this->role_id(), recogn,error);
    }
    else
    {
    	return this->monitor()->dispatch_to_client(this->gate_sid(),
    			this->role_id(), error, msg_proto);
    }
}

void LogicPlayer::set_client_ip(const string &client_ip)
{
	this->client_ip_ = client_ip;
}

std::string &LogicPlayer::client_ip(void)
{
    return this->client_ip_;
}

void LogicPlayer::set_session(const string& session)
{
    this->session_ = session;
}

std::string &LogicPlayer::session(void)
{
    return this->session_;
}

void LogicPlayer::set_role_id_for_load(const int64_t role_id)
{
    this->role_id_ = role_id;
}

void LogicPlayer::set_uc_sid(const std::string &uc_sid)
{
	this->uc_sid_ = uc_sid;
}

std::string &LogicPlayer::uc_sid(void)
{
	return this->uc_sid_;
}

int LogicPlayer::kill_num(void)
{
	return this->role_detail_.kill_num_;
}

int LogicPlayer::killing_value(void)
{
	return this->role_detail_.killing_value_;
}

void LogicPlayer::set_new_role_flag(const bool flag)
{
	this->is_new_role_ = flag;
}

bool LogicPlayer::is_new_role(void)
{
	return this->is_new_role_;
}

// 不要在这个接口做客户端消息推送，客户端地图模型没有加载完会直接把消息丢弃；
int LogicPlayer::sign_in(const int gate_sid)
{
	JUDGE_RETURN(this->monitor()->bind_player(this->role_id(), this) == 0, -1);

	this->monitor()->bind_player(this->role_detail_.__name, this);
    this->gate_sid_ = gate_sid;

    this->online_.sign_in();
    this->teamer_sign_in();
    this->leaguer_sign_in();
    this->leaguer_check_open();
    this->friend_sign_in();
    this->reset_every_day();
    this->reset_every_week();
    this->is_sgin_in.push_back(this->role_id());
    this->update_open_activity_area_rank();
    this->cabinet_sign_in(LuckyWheelActivity::ACT_CABINET);
    this->cabinet_sign_in(LuckyWheelActivity::ACT_CABINET_DISCOUNT);
    this->check_finish_back_act_every_day();
//    this->wedding_login_in();

    ACTIVITY_TIPS_SYSTEM->update_activity_buff(this->role_id());
    ARENA_SYS->check_and_register_area(this);
    this->update_apply_league_info();

    Time_Value nowtime = Time_Value::gettimeofday(), interval(1);
    for (int i = 0; i < LogicPlayer::TIMEUP_END; ++i)
    {
        this->cache_tick_.update_timeup_tick(i, nowtime);
    }
    this->cached_timer_.schedule_timer(interval);

    this->may_act_start_tick(this);
    this->new_role_send_to_php_center();

    return 0;
}

int LogicPlayer::resign(const int gate_sid, Proto30100101 *request)
{
	MSG_USER("WARNING logic relogin %d %s %ld", gate_sid, this->name(), this->role_id());

    this->gate_sid_ = gate_sid;
    this->session_ = request->session_info().session();

    this->online_.sign_in();
    this->set_client_ip(request->client_ip());
    this->set_uc_sid(request->uc_sid());
    this->set_client_mac(request->client_mac());

    Time_Value nowtime = Time_Value::gettimeofday(), interval(1);
    for (int i = TIMEUP_SAVE; i < TIMEUP_END; ++i)
    {
        this->cache_tick_.update_timeup_tick(i, nowtime);
    }

    this->cached_timer_.schedule_timer(interval);
    this->process_uc_extend_info();
    return 0;
}

int LogicPlayer::sign_out(void)
{
	MSG_USER("sign out logic player %ld %s", this->role_id(), this->name());

	this->reset_arena();
    this->online_.sign_out();

    this->teamer_sign_out();
    this->leaguer_sign_out();
    this->trvl_teamer_sign_out();
    this->wheel_player_sign_out();
    this->wedding_login_out();

    // unregister client_sid to role_id;
    this->monitor()->unbind_player(this->role_id());
    this->monitor()->unbind_player(this->role_detail_.__name);

    //this->notify_all_teamer_team_info();
    this->may_act_stop_tick();
    this->cached_timer_.cancel_timer();
    this->request_save_player(TRANS_LOGOUT_LOGIC_PLAYER);
    this->monitor()->player_pool()->push(this);
    return 0;
}

int LogicPlayer::gate_sid(void)
{
    return this->gate_sid_;
}

int64_t LogicPlayer::entity_id(void)
{
    return this->role_id_;
}

Int64 LogicPlayer::role_id(void)
{
    return this->role_id_;
}

int LogicPlayer::role_id_low(void)
{
    return (int32_t)this->role_id_;
}

int LogicPlayer::role_id_high(void)
{
    return (int32_t)(this->role_id_ >> 32);
}

int LogicPlayer::scene_id(void)
{
    return this->role_detail_.__scene_id;
}

int LogicPlayer::role_level(void)
{
	return this->role_detail_.__level;
}

const char *LogicPlayer::name(void)
{
    return this->role_detail_.name();
}

const char *LogicPlayer::account(void)
{
    return this->role_detail_.__account.c_str();
}

LogicRoleDetail &LogicPlayer::role_detail(void)
{
    return this->role_detail_;
}

int LogicPlayer::time_up(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->is_first_area_ == false, -1);

	//10秒一次
	if (this->cache_tick_.check_timeout(LogicPlayer::TIMEUP_CUSTOMER_SVC_REPLAY, nowtime) == true)
	{
		this->update_fest_activity_login();
	    this->check_arena_pa_event();
		this->customer_svc_time_up();
		this->cache_tick_.update_timeup_tick(LogicPlayer::TIMEUP_CUSTOMER_SVC_REPLAY, nowtime);
	}

    if (this->cache_tick_.check_timeout(LogicPlayer::TIMEUP_SAVE, nowtime) == true)
    {
        if (this->request_save_player() == 0)
        {
        	this->cache_tick().reset_cache_flag();
        }
        this->cache_tick_.update_timeup_tick(LogicPlayer::TIMEUP_SAVE, nowtime);
    }

    return 0;
}

GameCache &LogicPlayer::cache_tick(void)
{
    return this->cache_tick_;
}

int LogicPlayer::sync_role_info(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400201*, request, -1);
    MSG_USER("sync_role_info %ld %s %d->%d", this->role_id(), this->name(),
    		this->scene_id(), request->scene_id());

    this->role_detail_.__career = request->career();
    this->role_detail_.__scene_id = request->scene_id();
//    this->role_detail_.__camp_id = request->camp_id();
//    this->role_detail_.__permission = request->permission();

    int pre_level = this->role_detail_.__level;
    this->role_detail_.__level = request->level();

	this->update_player_force(request->force());
	this->update_open_activity_force(request->force());

    // level up
    JUDGE_RETURN(pre_level != request->level(), 0);

    //开启帮派模块
    this->league_handle_player_levelup();
    this->socialer_handle_player_levelup();

	this->update_apply_league_info();
	this->check_activity_reminder();
	this->update_open_activity_level(request->level());

	ARENA_SYS->check_and_register_area(this);

	Proto32101101 req;
	LOGIC_PHP->make_up_center_post_msg(this, &req);
	return LOGIC_MONITOR->process_inner_center_thread(req);
}

int LogicPlayer::sync_vip_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30100801*, request, 0);

	this->vip_detail_.set_vip_type(request->vip_type());
	this->vip_detail_.__start_time = request->vip_start_time();
	this->vip_detail_.__expired_time = request->period_time();
	this->role_detail().__vip_type = request->vip_type();
    this->check_and_notify_qq();
    this->update_leaguer_vip(this->vip_detail().__vip_level);

	return 0;
}

void LogicPlayer::update_player_force(int new_force)
{
    this->role_detail_.__fight_force = new_force;

    LeagueMember* league_member = this->league_member();
    if (league_member != NULL)
    {
    	league_member->new_role_force_ = new_force;
    	league_member->role_lvl_ = this->role_level();
    }

    this->update_teamer_force(new_force);
}

void LogicPlayer::update_leaguer_vip(int vip_level)
{
	LeagueMember* league_member = this->league_member();
	JUDGE_RETURN(league_member != NULL, ;);
	league_member->vip_type_ = vip_level;
}

bool LogicPlayer::is_vip()
{
	return this->vip_detail_.is_vip();
}

bool LogicPlayer::notify_msg_flag()
{
	return this->is_notify_msg_;
}

int LogicPlayer::vip_type()
{
	return this->vip_detail_.__vip_type;
}

BaseVipDetail& LogicPlayer::vip_detail(void)
{
	return this->vip_detail_;
}

/*
 * refresh when player sign in and in midnight
 * */
void LogicPlayer::reset_every_day(int test)
{
	if (test == true)
	{
		this->role_detail_.day_reset_tick_ = 0;
	}

	JUDGE_RETURN(LOGIC_MONITOR->is_need_day_reset(
			this->role_detail_.day_reset_tick_) == true, ;);

	this->role_detail_.inc_adjust_login_days();

	LogicLeaguer::reset_every_day();
	LogicActivityer::reset_everyday();
	ArenaLPlayer::reset_arena_everyday();
	LogicWheelPlayer::reset_every_day();
	LogicMayActivityer::reset_everyday();
	SpecialBox::reset_every_day();

	this->role_detail_.buy_map_.clear();
	this->role_detail_.panic_buy_notify_ = 0;
	this->role_detail_.brother_reward_index.clear();
	this->role_detail_.is_worship_.clear();
	this->role_detail_.today_recharge_gold_ = 0;
	this->role_detail_.today_consume_gold_ = 0;
	this->role_detail_.today_market_buy_times_ = 0;
	this->role_detail_.today_can_buy_times_ = 0;
	this->role_detail_.today_total_buy_times_ = 0;

	for (ThreeObjMap::iterator iter = this->role_detail_.mount_info_.begin();
			iter != this->role_detail_.mount_info_.end(); ++iter)
	{
		this->update_return_activity_mount(iter->second.id_, iter->second.value_);
		this->update_combine_activity_mount(iter->second.id_, iter->second.value_);
		this->update_combine_return_activity_mount(iter->second.id_, iter->second.value_);
	}

	this->send_script_reset();
	this->update_combine_activity_login();
	this->reset_cumulative_login();
	this->update_open_activity_cumulative_login();
    this->check_finish_back_act_every_day();

	this->role_detail_.day_reset_tick_ = ::next_day(0,0).sec();
}

void LogicPlayer::reset_every_week(void)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	JUDGE_RETURN(this->role_detail_.week_reset_tick_ < nowtime, ;);

	this->role_detail().kill_evil_ = 0;
	this->role_detail().kill_normal_ = 0;
	//RANK_SYS->clear_realtime_rank(RANK_KILL_VALUE);
	//RANK_SYS->clear_realtime_rank(RANK_HERO);
	this->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
	this->role_detail_.week_reset_tick_ = next_week(7, 0, 0, nowtime);
}

void LogicPlayer::send_script_reset()
{
	LOGIC_MONITOR->dispatch_to_scene(this->gate_sid(), this->role_id(),
			this->scene_id(), INNER_SEND_SCRIPT_RESET);
}

int LogicPlayer::map_enter_scene(Message* msg)
{
//	this->send_map_skill_info();
	return 0;
}

int LogicPlayer::map_obtain_area(Message* msg)	//每次执行
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100302*, request, -1);

//	int cur_scene = request->cur_scene();
	ARENA_SYS->check_and_update_shape(this);

	LogicTeamer::map_enter_scene();
	this->teamer_delay_sign_in();
	this->check_and_quit_team();

//	this->send_map_skill_info(ENTER_SCENE_TRANSFER);
//	this->send_map_flag_info(ENTER_SCENE_TRANSFER);

	this->is_notify_msg_ = true;
    this->map_fisrt_obtain_area();

    this->wedding_login_in();
//	this->check_and_notity_arena_tips();
//    this->notify_wedding_cartoon_play();
//    this->notify_cruise_icon();
//    this->sync_wedding_info_to_map();
//    this->sync_wedding_property(ENTER_SCENE_TRANSFER);
//    this->refresh_brother_prop(ENTER_SCENE_TRANSFER);
//    this->notify_is_has_wedding();
//    this->sync_brother_task_info();

	return 0;
}

int LogicPlayer::map_fisrt_obtain_area()	//第一次执行
{
	JUDGE_RETURN(this->is_first_area_ == true, -1);
	this->is_first_area_ = false;

	this->check_activity_reminder();
	this->check_and_game_notice_tips();

	this->notify_ltable_state();
    this->check_and_notify_qq();
	this->send_chat_speak_state();
	this->act_login_check_red_point();

	LOGIC_SWITCHER_SYS->notify_client(this);
	FEST_ACTIVITY_SYS->notify_activity_state(this);
    LEAGUE_SYSTEM->sync_fb_flag(this->role_id());
    this->notify_activity_icon_info();

    this->update_fest_activity_login();
	this->combine_activity_first_times();
    this->update_combine_activity_login();
    this->update_open_activity_cumulative_login();
    this->update_arena_exp_restore();

	return 0;
}

int LogicPlayer::map_add_goods_result(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400012*, request, -1);

	switch(request->source_type())
	{
	default:
	{
		break;
	}
	}

	return 0;
}

int LogicPlayer::map_consume_gold(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100606*, request, -1);
	JUDGE_RETURN(request->consum_gold() > 0, -1);

	this->role_detail_.today_consume_gold_ = request->today_gold();
	this->update_fest_activity_consume_money(request->today_gold());
	this->update_combine_activity_consum(request->consum_gold());
	this->update_recharge_rank(2, request->consum_gold());	//每日冲榜
	this->update_back_act_accu_consum(request->consum_gold());

	return 0;
}

int LogicPlayer::request_save_player(const int trans_recogn)
{
	if (trans_recogn == TRANS_LOGOUT_LOGIC_PLAYER)
	{
		this->role_detail_.__last_logout_tick = Time_Value::gettimeofday().sec();
	}

    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    if (trans_recogn == TRANS_LOGOUT_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(LogicPlayer::CACHE_BASE) == true)
    {
        MMORole::update_data(this, data_map, trans_recogn);
    }

    if (trans_recogn == TRANS_LOGOUT_LOGIC_PLAYER
    		|| this->cache_tick().check_cache(LogicPlayer::CACHE_ACTIVITY_TIPS) == true)
	{
    	MMOActivityTips::update_data(this, data_map);
	}

//    if (trans_recogn == TRANS_LOGOUT_LOGIC_PLAYER || this->cache_tick().check_cache(LogicPlayer::CACHE_PANIC_SHOP) == true)
//    {
//        MMOPanic::update_data(this, data_map);
//    }
//    if (this->cache_tick().check_cache(LogicPlayer::CACHE_STRENGTH) == true)
//    {
//    	MMOSocialer::update_strength_data(this, data_map);
//    }
    if (trans_recogn == TRANS_LOGOUT_LOGIC_PLAYER ||
            this->cache_tick().check_cache(LogicPlayer::CACHE_WEDDING) == true)
    {
        MMOWedding::update_data(this, data_map);
        MMORankPannel::update_date(this, data_map);
    }
    if (trans_recogn == TRANS_LOGOUT_LOGIC_PLAYER ||
            this->cache_tick().check_cache(LogicPlayer::CACHE_BACK_ACTIVITY) == true)
    {
        BackJYBackActivity::update_data(this, data_map);
    }

    switch(trans_recogn)
    {
    case TRANS_LOGOUT_LOGIC_PLAYER:
    {
    	MMOOnline::update_data(this->role_id(), &(this->online()), data_map);
    	MMOSocialer::update_data(this, data_map);
    	MMOOpenActivity::update_data(this, data_map);
    	MMOOpenActivity::save_may_activity(this, data_map);
    	MMOLuckyWheel::update_data(this, data_map);
    	MMOLeague::save_leaguer_info(this, data_map);
    	MMORankPannel::update_date(this, data_map);
    	MMOActivityTips::update_data(this, data_map);
    	BackCustomerSVC::update_player_customer_service_detail(this, data_map);
    	MMORoleEx::update_data(this, data_map);
    	break;
    }
    }

    if (TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), trans_recogn, 
                DB_MONGO_DATA_MAP, data_map, POOL_MONITOR->mongo_data_map_pool()) != 0)
    {
        return -1;
    }

    return 0;
}

LogicOnline &LogicPlayer::online(void)
{
    return this->online_;
}

int LogicPlayer::test_command(Message *msg)
{
//    Proto10100001 *request = dynamic_cast<Proto10100001 *>(msg);
//    JUDGE_RETURN(request != 0, -1);
//
//    char type_name[64];
//    ::sprintf(type_name, "Proto%d", request->recogn());
//
//    Message *respond = create_message(std::string(type_name));
//    if (respond != 0 && respond->ParseFromString(request->body()) == false)
//    {
//    	delete respond;
//    	return -1;
//    }
//
//    if (request->route() == "chat")
//    {
//        LOGIC_MONITOR->dispatch_to_chat(this, respond);
//    }
//    else if (request->route() == "map")
//    {
//        LOGIC_MONITOR->dispatch_to_scene(this->gate_sid(), this->role_id(), this->scene_id(), respond);
//    }
//    else
//    {
//        UnitMessage unit_msg;
//        unit_msg.reset();
//        unit_msg.__sid = this->gate_sid();
//        unit_msg.__msg_head.__recogn = request->recogn();
//        unit_msg.__msg_head.__role_id = this->role_id();
//        unit_msg.__type = UnitMessage::TYPE_PROTO_MSG;
//        unit_msg.__data.__proto_msg = respond;
//        if (LOGIC_MONITOR->logic_unit()->push_request(unit_msg) == 0)
//            return 0;
//    }
//
//    if (respond != 0)
//        delete respond;
    return 0;
}

int LogicPlayer::logic_test_command(Message* msg)
{
#ifdef TEST_COMMAND
	MSG_DYNAMIC_CAST_NOTIFY(Proto11499999*, request, RETURN_ML_TEST_COMMAND);

	if (request->cmd_name() == "wed_treasures")
	{
	    WeddingDetail *wedding_info = this->wedding_detail();
	    if (wedding_info == NULL)
	    {
	    	return 0;
	    }
	    WeddingDetail::WeddingRole *self_info = NULL;
	    WeddingDetail::WeddingRole *side_info = NULL;
	    if (this->role_id() == wedding_info->__partner_1.__role_id)
	    {
	    	self_info = &wedding_info->__partner_1;
	    	side_info = &wedding_info->__partner_2;
	    }
	    else
	    {
	    	self_info = &wedding_info->__partner_2;
	    	side_info = &wedding_info->__partner_1;
	    }
	    self_info->__fetch_tick -= Time_Value::DAY;
	    side_info->__fetch_tick -= Time_Value::DAY;

	    LogicPlayer* partner = this->fetch_wedding_partner();
	    if (partner != NULL) partner->request_wedding_pannel();
	    return this->request_wedding_pannel();
	}
	if (request->cmd_name() == "wed_update")
	{
		return this->update_wedding_property(request->param1(), request->param2());
	}
	if (request->cmd_name() == "area_night_refresh")
	{
		ARENA_SYS->midnight_handle_timeout();
	}
	if (request->cmd_name() == "serial_escort")
	{
		return DOUBLE_ESCORT->every_day_serial_work();
	}
	else if (request->cmd_name() == "serial_invest")
	{
		return BackIR_SYS->every_day_serial_work();
	}
	else if (request->cmd_name() == "recommend_friend")
	{
		return this->recommend_friend(NULL);
	}
	else if (request->cmd_name() == "team_creat")
	{
		return this->logic_create_team();
	}
	else if (request->cmd_name() == "team_quit")
	{
		return this->logic_quit_team();
	}
	else if (request->cmd_name() == "team_fast_join")
	{
		this->logic_fast_join_team(NULL);
		return 0;
	}
	else if (request->cmd_name() == "team_near")
	{
		this->logic_near_team_begin(NULL);
		return 0;
	}
	else if (request->cmd_name() == "team_near_player")
	{
		this->logic_near_player(NULL);
		return 0;
	}
	else if (request->cmd_name() == "team_invite")
	{
		Proto10100322 test;
		test.set_status(request->param1());
		this->logic_auto_invite(&test);
		return 0;
	}
	else if (request->cmd_name() == "team_accept")
	{
		Proto10100323 test;
		test.set_status(request->param1());
		this->logic_auto_accept(&test);
		return 0;
	}
	else if (request->cmd_name() == "invest_test")
	{
		BackIR_SYS->send_mail_to_player(this->role_id());
		return 0;
	}
	else if (request->cmd_name() == "mail_reward_clear")
	{
		SEND_ACTREWARD->reset();
		return 0;
	}
	else if (request->cmd_name() == "logic_reset_script")
	{
		this->send_script_reset();
		return 0;
	}
	else if (request->cmd_name() == "fest_start_boss")
	{
		FEST_ACTIVITY_SYS->start_boss();
		FEST_ACTIVITY_SYS->shout_boss();
		return 0;
	}
	else if (request->cmd_name() == "quintuple")
	{
		QUINTUPLE_SYS->test_quintuple(request->param1(), request->param2());
		return 0;
	}
	else if (request->cmd_name() == "fetch_player_info")
	{
		Proto10100156 respond;
		respond.set_role_id(85903640902799);
		return this->fetch_single_player_all_by_role_id(&respond);
	}
    else if (request->cmd_name() == "system_info")
    {
    	int type= request->has_param1() ? request->param1() : 0;
    	std::string content = "顶部滚动广播测试 ●▽●";
    	LOGIC_MONITOR->back_stage_push_system_announce(content, type);
    }
	else if (request->cmd_name() == "double_escort")
	{
		DOUBLE_ESCORT->test_escort(request->param1(), request->param2());
		return 0;
	}
	else if (request->cmd_name() == "test_new_rank")
	{
		Proto10100701 temp;
		temp.set_data_type(0);
		temp.set_rank_type(14);
		this->fetch_rank_data(&temp);
		temp.set_data_type(0);
		temp.set_rank_type(17);
		return this->fetch_rank_data(&temp);
	}
	else if(request->cmd_name() == "labour_task")
	{
		int task_id = request->param1();
		int value = request->param2();
		int type = request->param3();
		this->update_labour_activity_value(task_id, value, type);
		return 0;
	}
	else if (request->cmd_name() == "may_act_day")
	{
		return MAY_ACTIVITY_SYS->midnight_handle_timeout(request->param1());
	}
	else if (request->cmd_name() == "reset_may_act")
	{
		return this->may_act_test_reset(request->param1());
	}
	else if (request->cmd_name() == "login_day")
	{
		this->test_set_login_day(request->param1());
		return 0;
	}
	else if (request->cmd_name() == "save_may")
	{
		MAY_ACTIVITY_SYS->save_activity();
	}
	else if (request->cmd_name() == "fetch_lucky_wheel")
	{
		int activity_id = request->param1();
		Proto10102002 respond;
		respond.set_activity_id(activity_id);

		return this->fetch_one_wheel_activity(&respond);
	}
	else if (request->cmd_name() == "open_egg")
	{
		Proto10102041 respond;
		respond.set_slot_id(request->param1());

		return this->request_open_lucky_egg_begin(&respond);
	}
	else if (request->cmd_name() == "recharge_rank")
	{
		Proto10102051 respond;
		respond.set_activity_id(request->param1());
		respond.set_page(1);
		return this->fetch_recharge_rank_info(&respond);
	}
	else if (request->cmd_name() == "fetch_cabinet")
	{
		int activity_id = (request->param1() == 0) ? LuckyWheelActivity::ACT_CABINET : request->param1();
		return this->fetch_cabinet_info(activity_id);
	}
	else if (request->cmd_name() == "cabinet_buy")
	{
		Proto10102013 info;
		info.set_is_all(request->param1());
		info.set_slot_id(request->param2());

		return this->cabinet_buy_begin(&info);
	}
	else if (request->cmd_name() == "cabinet_refresh")
	{
		Proto10102012 info;
		info.set_times(request->param1());

		return this->cabinet_refresh_begin(&info);
	}
	else if (request->cmd_name() == "cabinet_reward")
	{
		Proto10102014 info;
		info.set_type(request->param1());

		return this->cabinet_refresh_reward(&info);
	}
	else if (request->cmd_name() == "immortal_info")
	{
		return this->fetch_immortal_treasures();
	}
	else if (request->cmd_name() == "immortal_draw")
	{
		return this->draw_immortal_treasures_begin();
	}
	else if (request->cmd_name() == "immortal_rand")
	{
		Proto10102033 info;
		info.set_id(request->param1());

		return this->rand_immortal_treasures_begin(&info);
	}
	else if (request->cmd_name() == "immortal_reward")
	{
		return this->fetch_immortal_treasures_reward();
	}
	else if (request->cmd_name() == "maze_info")
	{
		Proto10102021 info;
		info.set_activity_id(request->param1());

		this->fetch_maze_treasures(&info);
	}
	else if (request->cmd_name() == "maze_draw")
	{
		Proto10102022 info;
		info.set_activity_id(request->param1());

		this->draw_maze_begin(&info);
	}
	else if (request->cmd_name() == "time_limit")
	{
		return this->fetch_time_limit_info();
	}
	else if (request->cmd_name() == "time_limit_buy")
	{
		int slot_id = request->param1();
		Proto10102007 respond;
		respond.set_activity_id(50401);
		respond.set_slot_id(slot_id);
		return this->time_limit_item_buy_begin(&respond);
	}
	else if (request->cmd_name() == "lucky_wheel")
	{
		int activity_id = request->param1();
		int date_type 	= request->param2();
		int first_date	= request->param3();
		int last_date 	= request->param4();

		LUCKY_WHEEL_SYSTEM->test_set_activity(activity_id, date_type, first_date, last_date);
		if (activity_id == LuckyWheelActivity::ACT_CABINET || activity_id == LuckyWheelActivity::ACT_CABINET_DISCOUNT)
			LUCKY_WHEEL_SYSTEM->refresh_cabinet_info(activity_id);
		return 0;
	}
	else if (request->cmd_name() == "reset_lucky_wheel")
	{
		int activity_id = request->param1();
		this->test_reset_activity(activity_id);
		return 0;
	}
	else if (request->cmd_name() == "send_recharge_mail")
	{
		return this->test_send_recharge_mail();
	}
	else if (request->cmd_name() == "save_wheel")
	{
		LUCKY_WHEEL_SYSTEM->save_activity();
	}
	else if (request->cmd_name() == "opinion_test")
	{
		return this->request_open_customer_service_pannel(NULL);
	}
	else if (request->cmd_name() == "need_sort_area")
	{
		ARENA_SYS->check_area_need_sort();
		return 0;
	}
	else if (request->cmd_name() == "test_area")
	{
		Proto10101003 temp;
		temp.set_rank(request->param1());
		return this->start_area_challenge(&temp);
	}
	else if (request->cmd_name() == "save_area")
	{
		ARENA_SYS->save_arena_data();
		return 0;
	}
	else if (request->cmd_name() == "league_wealfare")
	{
		return this->fetch_league_welfare();
	}
	else if (request->cmd_name() == "other_league")
	{
		Proto10100653 req;
		req.set_league_index(request->param1());
		return this->fetch_other_league_info(&req);
	}
	else if (request->cmd_name() == "league_system_reset")
	{
		LEAGUE_SYSTEM->league_check_everyday(true);
		return 0;
	}
	else if (request->cmd_name() == "reset_boss")
	{
		return this->test_reset_boss(request->param1());
	}
	else if (request->cmd_name() == "flag_exp")
	{
		League* league = this->league();
		if (league != NULL)
		{
			league->flag_exp_ += request->param1();
		}
		return 0;
	}
	else if (request->cmd_name() == "flag_lvl")
	{
		return this->upgrade_league_flag();
	}
	else if (request->cmd_name() == "lfb_info")
	{
		return this->request_lfb_wave_reward();
	}
	else if (request->cmd_name() == "reset_all_lfb")
	{
		return this->test_reset_lfb();
	}
	else if (request->cmd_name() == "lfb_cheer_info")
	{
		return this->request_lfb_cheer_info();
	}
	else if (request->cmd_name() == "save_invest_recharge")
	{
		return BackIR_SYS->update_system();
	}
	else if (request->cmd_name() == "open_act")
	{
		CONFIG_INSTANCE->test_client_set_open_day(request->param1());
		LOGIC_OPEN_ACT_SYS->midnight_handle_timeout(request->param1());
		LOGIC_MONITOR->reset_all_player_everyday(true);
		return 0;
	}
	else if(request->cmd_name() == "reset_day")
	{
		this->test_reset_act_update_tick(request->param1());
		return 0;
	}
	else if(request->cmd_name() == "reset_cum_login")
	{
		this->test_reset_act_update_tick(10801);
		this->update_open_activity_cumulative_login();
		return 0;
	}
	else if(request->cmd_name() == "fetch_gashapon")
	{
		Proto10102060 msg;
		msg.set_activity_id(51601);
		this->fetch_gashapon_info(&msg);
		return 0;
	}
	else if(request->cmd_name() == "gashapon_recharge")
	{
		this->update_gashapon_recharge(request->param1());
		return 0;
	}
	else if(request->cmd_name() == "gashapon_begin")
	{
		Proto10102061 msg;
		msg.set_activity_id(51601);
		msg.set_count(request->param1());
		this->fetch_gashapon_reward(&msg);
		return 0;
	}
	else if(request->cmd_name() == "rand_slot")
	{
		int activity_id = request->param1();
		int slot_num = request->param2();
		this->rand_slot_item(activity_id, slot_num);
		return 0;
	}
	else if(request->cmd_name() == "rand_slot_count")
	{
		int activity_id = request->param1();
		int slot_num = request->param2();
		int refresh_count = request->param3();
		do
		{
			if(this->rand_slot_item(activity_id, slot_num) != 0)
			{
				MSG_USER("error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				break;
			}
		}while(refresh_count--);
	}
	else if(request->cmd_name() == "cabinet_work")
	{
		this->cabinet_refresh_work(request->param1(),0, request->param2(), request->param3());
		return 0;
	}
	else if(request->cmd_name() == "loop_cabinet")
	{
		for(int i = 0;i < request->param1(); ++i)
		{
			MSG_USER("cur out loop:%d", i);
			this->cabinet_refresh_work(51501,1);
		}
	}
	else if (request->cmd_name() == "festival_reset")
	{
		this->role_detail_.day_reset_tick_ = 0;
		this->reset_every_day();
		FEST_ACTIVITY_SYS->midnight_handle_timeout();
		return 0;
	}
	else if (request->cmd_name() == "reset_lwar_act")
	{
		LOGIC_OPEN_ACT_SYS->test_reset_lwar_activity();
		return 0;
	}
	else if (request->cmd_name() == "reset_open_server_act")
	{
		int index = request->param1();
		this->test_reset_acrivity(index);
		return 0;
	}
	else if (request->cmd_name() == "open_act_save")
	{
		LOGIC_OPEN_ACT_SYS->stop(1);
		return 0;
	}
	else if (request->cmd_name() == "boss_info")
	{
		League* league = this->league();
		LeagueBossInfo boss_info = league->league_boss_;
		MSG_USER("boss_info! index: %d, exp: %d", boss_info.boss_index_, boss_info.boss_exp_);
		return 0;
	}
	else if (request->cmd_name() == "donate")
	{
		if (this->league() == NULL)
		{
			MSG_USER("NOT HAS LEAGUE!");
			return 0;
		}
		MSG_USER("HAS LEAGUE!");

		Proto10100613 donate_info;
		donate_info.set_donate_type(request->param1());
		donate_info.set_donate_number(request->param2());
		donate_info.set_send_flag(0);
		this->league_donate(&donate_info);
		return 0;
	}
	else if (request->cmd_name() == "friend")
	{
		Proto10100153 respond;
		this->search_friend_by_name(&respond);
	}
	else if (request->cmd_name() == "logic_reset_day")
	{
		this->role_detail_.day_reset_tick_ = 0;
		this->reset_every_day();
		return 0;
	}
	else if (request->cmd_name() == "logic_reset_week")
	{
		this->role_detail_.week_reset_tick_.sec(0);
		this->reset_every_week();
		return 0;
	}
	else if (request->cmd_name() == "stop_market")
	{
		return MARKET_SYS->stop();
	}
	else if (request->cmd_name() == "market")
	{
		Proto10100501 req;
		req.set_main_type(request->param1());
		req.set_sub_type(request->param2());
		req.set_sort(2);
		return this->fetch_market_info(&req);
	}
	else if (request->cmd_name() == "arena_reward")
	{
		ARENA_SYS->set_arena_role_reward(this->role_id());
		return 0;
	}
	else if (request->cmd_name() == "draw_area")
	{
		ARENA_SYS->set_arena_role_reward(this->role_id());
		this->draw_area_reward();
		return 0;
	}
    else if (request->cmd_name() == "evaluate")
    {
    	Proto10100906 req;
    	req.set_evaluate_level(request->param1());

    	const Json::Value& cfg = CONFIG_INSTANCE->tiny("tmp_id");
    	std::string str = cfg[0u].asString();
    	Int64 record_id = ::atoll(str.c_str());
    	req.set_record_id(record_id);

    	return this->request_evaluate_customer_service_replay(&req);
    }
    else if (request->cmd_name() == "escort_car")
    {
    	Proto10100645 msg;
    	msg.set_type(0);
    	return this->select_escort_car_type(&msg);
    }
    else if (request->cmd_name() == "chests_open")
    {
        Proto81401201 respond;
        ProtoActivityInfo *info = respond.add_activity_info();
        info->set_activity_id(GameEnum::FIRST_COLLECT_CHESTS_ID);
        info->set_activity_state(GameEnum::ACTIVITY_STATE_AHEAD);
        info->set_left_time(10);
        info->set_icon_bling(1);

    	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
    	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
    			iter != player_map.end(); ++iter)
    	{
    		LogicPlayer* player = iter->second;
    		player->respond_to_client(ACTIVE_NOTIFY_ACTIVITY_INFO, &respond);
    	}
        return 0;
    }
    else if (request->cmd_name() == "clear_area")
    {
    	ArenaRole* area_role = ARENA_SYS->area_role(this->role_id());
    	JUDGE_RETURN(area_role != NULL, 0);
    	area_role->left_times_ = 1;
    	return 0;
    }
    else if (request->cmd_name() == "wedinfo")
    {
        return this->request_wedding_pannel();
    }
    else if (request->cmd_name() == "wed")
    {
        Proto10101402 req;
        req.set_type(request->param1());

        TeamPanel *team_info = this->team_panel();
        if (team_info != NULL)
        {
            Int64 partner_id = 0;
            for (LongList::iterator iter = team_info->teamer_set_.begin();
                    iter != team_info->teamer_set_.end(); ++iter)
            {
                if (*iter == this->role_id())
                    continue;

                partner_id = *iter;
                break;
            }

            if (partner_id != 0)
            {
            	this->update_intimacy(100, partner_id);
            }
        }
        return this->request_wedding(&req);
    }
    else if (request->cmd_name() == "wedmake")
    {
        Proto10101403 req;
        req.set_type(request->param1());
        return this->request_wedding_make_up(&req);
    }
    else if (request->cmd_name() == "divorce")
    {
        return this->request_divorce(0);
    }
    else if (request->cmd_name() == "keepsakeup")
    {
        return this->request_keepsake_upgrade(0);
    }
    else if (request->cmd_name() == "flower")
    {
        Proto10101406 req;
        req.set_item_id(request->param1());
        req.set_item_num(request->param2());
        Int64 reveiver_id = merge_int_to_long(request->param3(), this->role_id_high());
        req.set_receiver_id(reveiver_id);
        return this->request_present_flower(&req);
    }
    else if (request->cmd_name() == "refresh_rank")
    {
    	return RANK_SYS->request_refresh_rank_data(request->param1());
    }
    else if(request->cmd_name() == "monster_tower")
    {
    	int script_sort = request->param1();
    	Proto10100401 req;
    	req.set_fb_id(script_sort);
    	this->team_fb_organize(&req);

    	Proto10100414 enter;
    	enter.set_script_sort(script_sort);
    	this->team_fb_enter_fb(&enter);
    }
    else if(request->cmd_name() == "ddh_team")
    {
    	int script_sort = request->param1();
		Proto10100401 req;
		req.set_fb_id(script_sort);
		this->team_fb_organize(&req);
    }
    else if(request->cmd_name() == "ddh_in")
    {
    	int script_sort = request->param1();
    	Proto10100414 enter;
		enter.set_script_sort(script_sort);
		this->team_fb_enter_fb(&enter);
    }
	else if(request->cmd_name() == "up_cfg")
    {
    	CONFIG_INSTANCE->update_config("all");
    }
    else if (request->cmd_name() == "sfb")
	{
		MMOLeague::save_leaguer_fb_flag(LEAGUE_SYSTEM->league_fb_flag());
		return 0;
	}
    else if(request->cmd_name() == "fetch_cornucopia")
    {
    	Proto10100220 msg;
    	msg.set_activity_id(10901);
    	this->fetch_cornucopia_msg(&msg);
    	return 0;
    }
    else if(request->cmd_name() == "fetch_cornucopia_reward")
    {
    	Proto10100221 msg;
    	msg.set_activity_id(10901);
    	msg.set_index(request->param1());
    	return this->fetch_cornucopia_reward(&msg);
    }
    else if(request->cmd_name() == "fetch_cornucopia_task")
    {
    	Proto31403200 msg;
    	msg.set_task_id(request->param1());
    	msg.set_task_finish_count(request->param2());
    	return this->update_cornucopia_activity_value(&msg);
    }
    else if(request->cmd_name() == "fetch_cornucopia_gold_reward")
    {
    	return this->fetch_cornucopia_mail_reward();
    }
    else if(request->cmd_name() == "reset_cornucopia_info")
    {
    	this->reset_cornucopia_info();
    	LOGIC_OPEN_ACT_SYS->init_cornucopia_act();
		return 0;
    }
    else if(request->cmd_name() == "rand_all_red")
    {
    	MAY_ACTIVITY_SYS->rand_all_red_packet();
    	return 0;
    }
    else if(request->cmd_name() == "notify_act")
    {
    	Proto81401201 respond;
    	ProtoActivityInfo *act_info = respond.add_activity_info();
    	act_info->set_activity_id(request->param1());
    	act_info->set_activity_state(request->param2());
    	act_info->set_icon_bling(1);
    	act_info->set_left_time(request->param3());

    	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
    	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
    			iter != player_map.end(); ++iter)
    	{
    		LogicPlayer* player = iter->second;
    		player->respond_to_client(ACTIVE_NOTIFY_ACTIVITY_INFO, &respond);
    	}
    	return 0;
    }
    else if(request->cmd_name() == "fetch_red_reward")
    {
    	Proto10100256 msg;
    	msg.set_index(request->param1());
    	this->fetch_red_packet_reward(&msg);
    	return 0;
    }
    else if(request->cmd_name() == "fetch_red_reward_info")
    {
    	Proto10100255 msg;
    	msg.set_index(request->param1());
    	this->fetch_red_packet_reward_info(&msg);
    	return 0;
    }
    else if(request->cmd_name() == "clear_red_reward")
    {
    	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, 1);
    	JUDGE_RETURN(act_info != NULL, -1);
    	act_info->player_reward_info_.clear();
    	act_info->all_red_packet_map_.clear();
    }
    else if(request->cmd_name() == "may_rand_item")
    {
    	this->may_rand_slot_item(request->param1(), request->param2());
    	return 0;
    }
    else if(request->cmd_name() == "fashion_take_times")
    {
    	Proto10100265 msg;
    	msg.set_times(request->param1());
    	return this->fetch_fashion_buy_begin(&msg);
    }
    else if(request->cmd_name() == "fashion_get_reward")
    {
    	Proto10100266 msg;
    	msg.set_activity_id(request->param1());
    	msg.set_index(request->param2());
    	return this->fetch_fashion_reward(&msg);
    }
    else if(request->cmd_name() == "draw_may_reward")
    {
    	Proto10100253 msg;
    	msg.set_index(request->param1());
    	msg.set_reward_index(request->param2());
    	return this->draw_may_activity_reward(&msg);
    }
    else if (request->cmd_name() == "backactinfo")
    {
    	if (request->param2() == 0)
    	{
    		Proto10100217 req;
    		req.set_first_type(request->param1());
    		return this->fetch_back_activity_list(&req);
    	}
    	else
    	{
    		Proto10100218 req;
    		req.set_act_id(request->param2());
    		return this->fetch_single_back_activity_info(&req);
    	}
    }
    else if (request->cmd_name() == "draw_backact")
    {
    	Proto10100219 req;
    	req.set_act_id(request->param1());
    	req.set_reward_id(request->param2());
    	return this->draw_single_back_activity_reward(&req);
    }
    else if(request->cmd_name() == "change_item")
    {
    	Proto10100271 req;
    	req.set_act_id(70901);
    	req.set_index(request->param1());
    	return this->fetch_change_buy_begin(&req);
    }
    else if(request->cmd_name() == "clean_change_data")
    {
    	this->test_clear_data_by_change();
    	return 0;
    }
    else if(request->cmd_name() == "clean_fish_data")
    {
    	this->clean_fish_info();
    	return 0;
    }
    else if(request->cmd_name() == "fish_score_reward")
    {
    	Proto10102065 req;
    	req.set_index(request->param1());
    	return this->fetch_fish_score_reward(&req);
    }
    else if(request->cmd_name() == "get_fish")
    {
    	Proto10102062 req;
    	req.set_type(request->param1());
    	req.add_index(request->param2());
    	return this->fetch_fish_begin(&req);
    }
    else if(request->cmd_name() == "fetch_fish_score")
    {
    	return this->fetch_fish_score_info(NULL);
    }
    else if(request->cmd_name() == "fetch_fish_tips")
    {
    	return this->fetch_fish_tips_info(NULL);
    }
    else if(request->cmd_name() == "get_wheel_act")
    {
    	Proto10102003 req;
    	req.set_activity_id(request->param1());
    	req.set_type(request->param2());
    	return this->draw_award_begin(&req);
    }
    else if(request->cmd_name() == "buy_key")
    {
    	Proto10102070 req;
    	req.set_count(request->param1());
    	return this->fetch_special_box_buy_key_begin(&req);
    }
    else if(request->cmd_name() == "clean_special_box_info")
    {
    	this->clean_special_box_info();
    	return 0;
    }
    else if(request->cmd_name() == "check_money")
    {
    	Proto10102072 req;
    	req.set_type(request->param1());
    	return this->fetch_special_box_reward_check_money(&req);
    }
    else if(request->cmd_name() == "check_item")
    {
    	Proto10102071 req;
    	req.set_type(request->param1());
    	return this->fetch_special_box_reward_check_item(&req);
    }
    else if(request->cmd_name() == "fetch_change_info")
    {
    	return this->fetch_special_box_change_info(NULL);
    }
    else if(request->cmd_name() == "sp_change_item")
    {
    	Proto10102075 req;
    	req.set_index(request->param1());
    	req.set_times(request->param2());
    	return this->fetch_special_box_change_reward_begin(&req);
    }
#endif
	return 0;
}

int LogicPlayer::fetch_other_player_info(Message* msg, int recogn)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100031*, request, recogn);

	Int64 role_id = 0;
	int query_type = request->query_type();
	if(query_type == GameEnum::RANK_QUERY_PET_DETAIL)
		return this->request_fetch_rank_beast_info(msg);

	Proto31400261 lookup_info;
	lookup_info.set_recogn(recogn);
	lookup_info.set_self_id(this->role_id());
	lookup_info.set_query_type(query_type);

	switch (recogn)
	{
	case RETURN_OTHER_MASTER_INFO:
	{
		role_id = request->role_id();

		lookup_info.set_other_id(request->role_id());
		lookup_info.set_beast_id(request->beast_id());
		break;
	}

	default:
	{
		return -1;
	}
	}

	LogicPlayer* player = this->find_player(role_id);
	if (player == NULL)
	{
		if (query_type == GameEnum::RANK_QUERY_TRAVEL_PET && GameCommon::is_travel_scene(this->scene_id()))
		{
			return this->dispatch_to_scene_server(this->scene_id(), &lookup_info);
		}
		return RANK_SYS->fetch_player_offline(this, &lookup_info);
	}
	else
	{
		return LOGIC_MONITOR->dispatch_to_scene(player, &lookup_info);
	}
}

int LogicPlayer::respond_other_player_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400261*, request, -1);

	LogicPlayer* player = this->find_player(request->self_id());
	JUDGE_RETURN(player != NULL, -1);

	return LOGIC_MONITOR->dispatch_to_client(player, request->recogn(),
			request->msg_body());
}

int LogicPlayer::respond_single_player_all_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto50100156*, respond, -1);

	LogicPlayer* player = this->find_player(respond->checker_id());
	JUDGE_RETURN(player != NULL, -1);

	return LOGIC_MONITOR->dispatch_to_client(player->gate_sid(), player->role_id(),
			0, respond);
}

int LogicPlayer::respond_ranker_detail(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto50100702 *, respond, -1);

    LogicPlayer *req_player = this->find_player(respond->req_role_id());
    JUDGE_RETURN(req_player != NULL, -1);

    return req_player->respond_to_client(RETURN_REQUEST_FETCH_RANKER_DETAIL, respond);
}

int LogicPlayer::request_query_center_acti_code(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101101*, request, -1);

	Proto32101201 req;
	req.set_acti_code(request->acti_code());
	req.set_code_id(request->code_id());

	return this->monitor()->process_inner_center_thread(req, this->role_id());
}

int LogicPlayer::return_query_center_acti_code(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101102*, request, -1);

	MSG_USER("request center acti code %ld %d",
			request->code_id(), request->query_ret());

	Proto31402002 req;
	req.set_code_id(request->code_id());
	req.set_query_ret(request->query_ret());
	return this->monitor()->dispatch_to_scene(this, &req);
}

int LogicPlayer::sync_update_player_name(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400020 *, request, -1);

    this->monitor()->unbind_player(this->name());
    this->role_detail_.set_name(request->role_name());;
    this->monitor()->bind_player(request->role_name(), this);
    RANK_SYS->update_player_name(this->role_id(), this->name());

    this->handle_rename_leaguer();
    this->handle_rename_teamer();

    {
    	Proto32101101 req;
    	LOGIC_PHP->make_up_center_post_msg(this, &req);
		LOGIC_MONITOR->process_inner_center_thread(req);
    }

    //婚姻信息更新
    this->wedding_login_in();

    return 0;
}

int LogicPlayer::sync_update_player_sex()
{
	this->role_detail_.set_sex();
	this->handle_resex_leaguer();
	this->handle_resex_teamer();
	return 0;
}

int LogicPlayer::sync_update_fight_detail(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101001 *, request, -1);

	int info_size = request->info_type_size();
	for(int i = 0; i < info_size; ++i)
	{
		int info_type = request->info_type(i);
		switch(info_type)
		{
		case GameEnum::SYNC_LOGIC_FORCE:
		{
			this->update_player_force(request->force());
			this->update_open_activity_force(request->force());
			this->role_detail().kill_num_ = request->kill_num();
			this->role_detail().killing_value_ = request->killing_value();
			ARENA_SYS->check_and_register_area(this);
			break;
		}

		default:
			break;

		}
	}

	return 0;
}

int LogicPlayer::sync_update_kill_player(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100105 *, request, -1);

	if (request->kill_evil() == 1)
	{
		this->role_detail().kill_evil_ ++;
	}

	if (request->kill_normal() == 1)
	{
		this->role_detail().kill_normal_ ++;
	}

	this->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
	return 0;
}

int LogicPlayer::sync_update_mount_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100239*, request, -1);

	LogicRoleDetail& detail = this->role_detail();
	for (int i = 0; i < request->mount_info_size(); ++i)
	{
		const ProtoThreeObj& proto = request->mount_info(i);

		ThreeObj& obj = detail.mount_info_[proto.id()];
		obj.id_ 	= proto.id();
		obj.value_ 	= proto.value();
		obj.tick_ 	= proto.tick();
		JUDGE_CONTINUE(request->flag() == true);

		this->update_open_activity_mount(proto.id(), proto.value());
		this->update_return_activity_mount(proto.id(), proto.value());
		this->update_combine_activity_mount(proto.id(), proto.value());
		this->update_combine_return_activity_mount(proto.id(), proto.value());
	}

	return 0;
}

int LogicPlayer::sync_update_player_total_recharge_gold(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101110*, request, -1);

	int cur_gold = request->cur_gold();
	JUDGE_RETURN(cur_gold > 0, 0);

	LogicRoleDetail& role_detail = this->role_detail();
	role_detail.cur_gold_ = cur_gold;
	role_detail.today_recharge_gold_ += cur_gold;
	role_detail.__recharge_total_gold = request->total_gold();

	int today_gold = role_detail.today_recharge_gold_;
	this->update_open_activity_recharge(today_gold);
	this->update_return_activity_recharge(today_gold);
	this->update_open_activity_total_recharge(today_gold);
	this->update_combine_activity_recharge(today_gold, cur_gold);
	this->update_combine_return_activity_recharge(today_gold);
	this->update_recharge_rank(1, cur_gold);
	this->update_gashapon_recharge(today_gold);
	this->update_combine_activity_login(true);
	this->update_cornucopia_activity_recharge(cur_gold);
	this->update_back_act_accu_recharge(today_gold);
	this->update_back_act_repeat_recharge(today_gold);
	this->update_back_act_travel_recharge_rank(cur_gold);
	this->update_daily_login_recharge_sign_handle();
	this->update_daily_recharge_recharge_sign_handle();

	return 0;
}

int LogicPlayer::notify_fight_prop_info(Message *msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30102102*, request, -1);
//	int type = request->type();
//
//	switch (type)
//	{
//	case GameEnum::WEDDING_TYPE:
//		this->sync_wedding_property();
//		break;
//
//	default:
//		break;
//	}

	return 0;
}

int LogicPlayer::sync_update_quintuple_exp_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100410*, request, -1);

	return QUINTUPLE_SYS->update_monster_exp_info(this->role_id(), request->exp());
}

int LogicPlayer::sync_update_buff_info(int status, int buff_id, int buff_time, int serial_from)
{
	Proto30102101 buff_info;
	buff_info.set_status(status);
	buff_info.set_buff_id(buff_id);
	buff_info.set_buff_time(buff_time);
	return LOGIC_MONITOR->dispatch_to_scene(this, &buff_info);
}

int LogicPlayer::sync_uddate_fashion_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400410*, request, -1);

	this->role_detail().fashion_id_ = request->select_id();
	this->role_detail().fashion_color_ = request->sel_color_id();

	this->update_leaguer_fashion();

	return 0;
}

int LogicPlayer::sync_update_escort_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100409*, request, -1);

	return DOUBLE_ESCORT->update_daily_gold(this->role_id(), request->gold(),
			request->other_info().id(), request->other_info().value());

}

int LogicPlayer::sync_update_act_serial_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30103102*, request, -1);
	int type = request->type();

	switch (type)
	{
	case SERIAL_ACT_DAILY_RECHARGE:
	case SERIAL_ACT_REBATE_RECHARGE:
	case SERIAL_ACT_INVEST_RECHARGE:
		return BackIR_SYS->update_serial_info(msg);

	case SERIAL_ACT_ESCORT:
		return DOUBLE_ESCORT->update_serial_info(msg);

	default:
		break;
	}
	return 0;
}
LogicHookDetail* LogicPlayer::hook_detail(void)
{
	return &this->hook_detail_;
}

int LogicPlayer::sync_hook_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100901 *, request, -1);
	this->hook_detail()->__is_hooking = request->is_hooking();
	return 0;
}

int LogicPlayer::notify_client_popup_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto81401405*, request, -1);

	if (this->is_new_role() == false && GameCommon::is_normal_scene(
			this->scene_id()) == true)
	{
		int need_view = GAME_NOTICE_SYS->is_need_view(this->role_detail().view_tick_);
		request->set_notice_popup(need_view);
	}
	else
	{
		request->set_notice_popup(false);
	}

	FINER_PROCESS_RETURN(ACTIVE_POPUP_INFO, request);
}

int LogicPlayer::process_uc_extend_info(void)
{
	JUDGE_RETURN(this->role_detail_.__agent == "ucand",-1);
	Proto32101102 request;
	request.set_channel(this->role_detail_.__agent);
	request.set_uc_sid(this->uc_sid());
	request.set_role_id(this->role_id());
	request.set_role_name(this->role_detail_.__name);
	request.set_role_level(this->role_detail_.__level);

	return this->monitor()->process_inner_center_thread(request);
}

int LogicPlayer::send_chat_speak_state(void)
{
	Proto30200127 sync_info;
	sync_info.set_speak_state(this->ban_speak_);
	sync_info.set_expired_time(this->ban_speak_expired_);

	return this->monitor()->dispatch_to_chat(this, &sync_info);
}

void LogicPlayer::new_role_send_to_php_center()
{
	JUDGE_RETURN(this->is_new_role() == true, ;) //新创建的角色推送一次信息到后台中央服

	Proto32101101 req;
	LOGIC_PHP->make_up_center_post_msg(this, &req);
	LOGIC_MONITOR->process_inner_center_thread(req);

	if (CONFIG_INSTANCE->global()["recharge_return"].asInt() == 1
			|| LOGIC_SWITCHER_SYS->logic_check_switcher(GameSwitcherName::first_double_return) == true)
	{
		//不删档正式服 请求删档充值返利
		Proto32101105 request;
		request.set_role_id(this->role_id());
		request.set_account(this->role_detail().__account);
		LOGIC_MONITOR->process_inner_center_thread(request);
	}
}

void LogicPlayer::set_speak_state(int type, int64_t expired_time)
{
	JUDGE_RETURN((type == GameEnum::OPER_BAN_SPEAK_TRICK) ||
			(type == GameEnum::OPER_BAN_SPEAK) ||
			(type == GameEnum::OPER_BAN_NONE), ;);

	if(expired_time < ::time(0))
	{
		type = GameEnum::OPER_BAN_NONE;
	}

	this->ban_speak_ = type;
	this->ban_speak_expired_ = expired_time;
}

int LogicPlayer::request_sever_flag(void)
{
	Proto50100101 respond;
	respond.set_server_flag(this->role_detail().__server_flag);

	MSG_USER("client fetch flag %ld %s %s", this->role_id(), this->name(),
			this->role_detail().__server_flag.c_str());

	FINER_PROCESS_RETURN(RETURN_SERVER_FLAG, &respond);
}

int LogicPlayer::request_add_new_role_reward()
{
	//新玩家首次登录时mmo.dragon数据库表中并没有玩家的id和活动奖励信息，而后续的操作需要用到id和奖励，所以需要先插入一条空白的活动奖励数
	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, -1);

	shop_mode->recogn_ = TRANS_ADD_NEW_ROLE_REWARD;
	shop_mode->input_argv_.type_int_ = this->role_detail().__career;
	shop_mode->input_argv_.type_int64_ = this->role_id();
	return LOGIC_MONITOR->db_load_mode_begin(shop_mode);
}

int LogicPlayer::refresh_permission_info(Message *msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30100120*, request, -1);

//	this->role_detail().__permission = request->permission();

	return 0;
}

int LogicPlayer::refresh_escort_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400453*, request, -1);

	request->set_cycle_id(DOUBLE_ESCORT->get_cycle_id());
	request->set_type(DOUBLE_ESCORT->get_open());

	if (request->from() == 3)
	{
		DOUBLE_ESCORT->escort_serial_work(this->role_id());
	}

	return LOGIC_MONITOR->dispatch_to_scene(this, request);
}

int LogicPlayer::fetch_player_dice_mult(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400052*, request, -1);

	int is_double = this->is_xuanji_double();
	request->set_mult(is_double);
	return LOGIC_MONITOR->dispatch_to_scene(this, request);
}

int LogicPlayer::trade_fetch_on_line_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101607 *, request, -1);
	int64_t role_id = request->role_id();
	bool on_line = false;
	LogicPlayer *player = NULL;
	LOGIC_MONITOR->find_player(role_id, player);
	if(player != NULL)
	{
		on_line = true;
		player->respond_to_client_error(INNER_TRADE_FETCH_ON_LINE_STATE,ERROR_TRADE_DES_OVER_DISTANCE);
	}
	else
	{
		on_line = false;
	}
	Proto30101607 req;
	req.set_on_line(on_line);
	return  LOGIC_MONITOR->dispatch_to_scene(this,&req);

}

void LogicPlayer::check_and_notify_qq()
{
	int vip = GameCommon::qq_fetch_agent_vip(this->role_detail().__agent_code);
	if(vip > 0 && this->vip_type() >= vip)
	{
		Proto81401202 notify;
		notify.set_qq(LOGIC_MONITOR->fetch_qq_49()[this->role_detail().__agent_code]);
		this->respond_to_client(ACTIVE_NOTIFY_49YOU_QQ,&notify);
	}
}

int LogicPlayer::handle_resex(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400050*, request, RETURN_RESEX_ROLE);

	if (this->is_has_wedding() == true)
	{
		request->set_operate(false);
	}
	else
	{
		request->set_operate(true);
	}

	return LOGIC_MONITOR->dispatch_to_scene(this, request);
}

void LogicPlayer::set_client_mac(const string& mac)
{
	this->client_mac_ = mac;
}

std::string& LogicPlayer::client_mac()
{
	return this->client_mac_;
}

int LogicPlayer::process_travel_reward_by_mail(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30402505 *, request, msg, -1);

    int reward_id = request->reward_id();
    SerialObj serial_obj(request->serial());
    int mail_id = request->mail_id();

    MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
    mail_info->add_goods(reward_id);
    return GameCommon::request_save_mail(this->role_id(), mail_info);
}

int LogicPlayer::process_tbattle_practice_mail(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30402506 *, request, msg, -1);

    Int64 role_id = request->role_id();
    int practice = request->practice();
    MailInformation *mail_info = GameCommon::create_sys_mail(FONT_TBATTLE_PRACTICE_MAIL);
    char str_content[GameEnum::MAX_MAIL_CONTENT_LENGTH + 1];
    ::snprintf(str_content, GameEnum::MAX_MAIL_CONTENT_LENGTH, mail_info->mail_content_.c_str(), practice);
    mail_info->mail_content_ = str_content;
    mail_info->add_goods(GameEnum::ITEM_ID_PRACTICE, practice, GameEnum::ITEM_BIND);
    return GameCommon::request_save_mail(role_id, mail_info);
}

