/*
 * LogicStruct.cpp
 *
 * Created on: 2013-01-09 11:25
 *     Author: glendy
 */

#include <BackGameModifySys.h>
#include "LogicStruct.h"
#include "TeamPlatform.h"
#include "LeagueSystem.h"
#include "MarketSystem.h"
#include "ShopMonitor.h"
#include "RankSystem.h"
#include "ActivityTipsSystem.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "FlowControl.h"
#include "LogicPhp.h"
#include "GameNoticeSys.h"
#include "BackstageBrocastControl.h"

#include "ArenaSys.h"
#include "BackstageMailSystem.h"
#include "RestrictionSystem.h"
#include "LogicGameSwitcherSys.h"
#include "OpenActivitySys.h"
#include "LuckyWheelSys.h"
#include "DailyActSys.h"
#include "BackActivityTick.h"
#include "ProtoDefine.h"
#include "WeddingMonitor.h"
#include "MMOSocialer.h"
#include "LucktTableMonitor.h"
#include "BackField.h"
#include "GameField.h"
#include "FestActivitySys.h"
#include "InvestRechargeSys.h"
#include "JYBackActivitySys.h"
#include "MayActivitySys.h"
#include "TrvlTeamSystem.h"

#include <mongo/client/dbclient.h>

void AccountDetail::RoleInfo::reset(void)
{
    ::memset(this, 0, sizeof(AccountDetail::RoleInfo));
}

void AccountDetail::reset(void)
{
    ::memset(this, 0, sizeof(AccountDetail));
    this->__role_index = -1;
}

LogicRoleDetail::LogicRoleDetail(void)
{
	LogicRoleDetail::reset();
}

int LogicRoleDetail::draw_flag(Int64 start_tick)
{
	return start_tick > this->draw_tick_;
}

void LogicRoleDetail::reset(void)
{
	BaseRoleInfo::reset();
    this->__camp_id = 0;
    this->prev_draw_ = 0;

    this->fashion_id_ = 0;
    this->fashion_color_ = 0;

    this->view_tick_ = 0;
    this->draw_tick_ = 0;
    this->day_reset_tick_ = 0;
    this->week_reset_tick_.sec(0);

    this->buy_map_.clear();
    this->buy_total_map_.clear();
    this->mount_info_.clear();
    this->panic_buy_notify_ = 0;

    this->kill_num_ = 0;
    this->killing_value_ = 0;
    this->kill_normal_ = 0;
    this->kill_evil_ = 0;
    this->is_worship_.clear();

    this->today_recharge_gold_ = 0;
    this->today_consume_gold_ = 0;
    this->today_market_buy_times_ = 0;
    this->brother_reward_index.clear();
    this->cur_day_detail_.reset();
    this->cornucopia_task_map_.clear();
    this->reward_stage_map_.clear();
    this->today_can_buy_times_ = 0;
    this->today_total_buy_times_ = 0;
    this->cur_red_packet_group_ = 0;

    this->continuity_login_day_ = 1;
    this->cumulative_times_ = 0;
    this->act_data_reset_flag_ = false;
    this->last_act_type_ = 0;
    this->last_act_end_time_ = 0;
    this->continuity_login_flag_ = false;
}


void LogicHookDetail::reset(void)
{
    ::memset(this, 0, sizeof(LogicHookDetail));
}

BackstageBrocastRecord::BackstageBrocastRecord()
{
	BackstageBrocastRecord::reset();
}

void BackstageBrocastRecord::reset()
{
	this->__index = 0;
	this->__brocast_type = 0;
	this->__brocast_tick = 0;
	this->__brocast_times = 0;
	this->__max_repeat_times = 0;
	this->__interval_sec = 0;
	this->__content.clear();
}

CustomerServiceRecord::CustomerServiceRecord()
{
	CustomerServiceRecord::reset();
}

void CustomerServiceRecord::reset()
{
	this->__record_type = 0;
	this->__has_replay = 0;
	this->__has_read = 0;
	this->__record_id = 0;
	this->__sender_id = 0;
	this->__send_tick = 0;
	this->__evaluate_tick = 0;
	this->__evaluate_level = 0;

	this->__sender_name.clear();
	this->__content.clear();
	this->__title.clear();
	this->__replay_content.clear();

	this->__sender_level = 0;
	this->__server_code = 0;
	this->__platform = 0;
	this->__agent = 0;
	this->__recharge_gold = 0;

	this->__opinion_index = 0;
	this->__evaluate_star = 0;
}

int CustomerServiceRecord::serialize(ProtoCustomerSVCRecord& proto_record)
{
	return this->serialize(proto_record);
}

int CustomerServiceRecord::serialize(ProtoCustomerSVCRecord* proto_record)
{
	proto_record->set_record_id(this->__record_id);
	proto_record->set_send_tick(this->__send_tick);
	proto_record->set_record_status(this->__has_replay);
	proto_record->set_record_type(this->__record_type);
	proto_record->set_title(this->__title);
	proto_record->set_content(this->__content);
	proto_record->set_replay_content(this->__replay_content);
	proto_record->set_last_evaluate(this->__evaluate_level);
	proto_record->set_opinion_index(this->__opinion_index);
	proto_record->set_evaluate_star(this->__evaluate_star);
	return 0;
}

int CustomerServiceRecord::unserialize(const ProtoCustomerSVCRecord& proto_record)
{
	this->__record_id = proto_record.record_id();
	this->__send_tick = proto_record.send_tick();
	this->__has_replay = proto_record.record_status();
	this->__record_type = proto_record.record_type();
	this->__title = proto_record.title();
	this->__content = proto_record.content();
	this->__replay_content = proto_record.replay_content();
	this->__evaluate_level = proto_record.last_evaluate();
	this->__opinion_index = proto_record.opinion_index();
	this->__evaluate_star = proto_record.evaluate_star();
	return 0;
}

CustomerServiceDetail::CustomerServiceDetail() :
		__last_summit_type(0), __unread_amount(0)
{/**/}

void CustomerServiceDetail::reset()
{
	this->__last_summit_type = 0;
	this->__unread_amount = 0;

	this->__content.clear();
	this->__title.clear();

	CustomerRecordVec::iterator it = this->__customer_record_vec.begin();
	for(; it != this->__customer_record_vec.end(); ++it)
	{
		POOL_MONITOR->customer_service_record_pool()->push(*it);
	}
	this->__customer_record_vec.clear();
	this->__customer_record_map.clear();

	this->__opinion_reward.clear();
}

void CustomerServiceDetail::fetch_record_map(LongMap& index_map)
{
	CustomerRecordMap::iterator it = this->__customer_record_map.begin();
	for(; it != this->__customer_record_map.end(); ++it)
	{
		index_map[it->first] = true;
	}
}

LogicSocialerDetail::LogicSocialerDetail(void)
{
	this->open_ = 0;
	this->reset();
}

void LogicSocialerDetail::reset()
{
	this->open_ = 0;
	this->__friend_list.clear();
	this->__stranger_list.clear();
	this->__black_list.clear();
	this->__enemy_list.clear();
	this->__nearby_list.clear();
	this->__apply_map.clear();
}

void LogicSocialerDetail::ApplyInfo::reset(void)
{
	this->friend_id_ = 0;
	this->league_id_ = 0;
	this->level_ = 0;
	this->sex_ = 0;
	this->tick_ = 0;
}

FriendPairInfo::FriendPairInfo()
{
	FriendPairInfo::reset();
}

void FriendPairInfo::reset()
{
	this->friend_map_.clear();
	this->delete_map_.clear();
}

LogicVipDetail::LogicVipDetail(void) :
    __type(VIP_NOT_VIP), __period_time(0)
{ /*NULL*/ }

void LogicVipDetail::reset(void)
{
	this->__type = VIP_NOT_VIP;
	this->__period_time = 0;
}

BoxRecord::BoxRecord(void)
{
	BoxRecord::reset();
}

void BoxRecord::reset(void)
{
    this->__role_id = 0;
    this->__name.clear();

    this->__time = 0;
    this->__item_id = 0;
    this->__item_amount = 0;
}

ActivityTipsInfo::ActivityTipsInfo()
{
	ActivityTipsInfo::reset();
}

const Json::Value& ActivityTipsInfo::conf()
{
	return CONFIG_INSTANCE->common_activity(this->__activity_id);
}

void ActivityTipsInfo::reset()
{
	this->__activity_id = 0;
	this->__acti_state = 0;
	this->__start_time = 0;
	this->__end_time = 0;
	this->__time_dynamic = false;
	this->__limit_level = 0;
    this->__sub_value = 0;
    this->__reset_time = 0;
    this->__update_tick = 0;
    this->__open_day_type = 0;
    this->__open_day = 0;
    this->__day_check = 0;
    this->__notify_cfg = 0;
    this->time_info_.reset();
}

void ActivityTipsInfo::serialize(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(ProtoActivityInfo *, request, ;);

    request->set_activity_id(this->__activity_id);
    request->set_sub_value(this->__sub_value);

    int left_time = 0;
    switch (this->__acti_state)
    {
    case GameEnum::ACTIVITY_STATE_AHEAD:
    {
    	left_time = GameCommon::left_time(this->__start_time);
    	break;
    }

    case GameEnum::ACTIVITY_STATE_START:
    {
    	left_time = GameCommon::left_time(this->__end_time);
    	break;
    }
    }

    request->set_left_time(left_time);
}

ActivityRec::ActivityRec(void)
{
	ActivityRec::reset();
}

void ActivityRec::reset(void)
{
    ::memset(this, 0, sizeof(ActivityRec));
}

LogicOneSecTimer::LogicOneSecTimer()
	: FixedTimer(GTT_LOGIC_ONE_SEC, Time_Value::SECOND)
{
}

int LogicOneSecTimer::handle_timeout(const Time_Value &tv)
{
	Int64 now_tick = tv.sec();

	TEAM_PLANTFORM->offline_handle_time_out();
	TEAM_PLANTFORM->team_request_time_out();
	TEAM_PLANTFORM->sign_view_time_out();
	MARKET_SYS->market_handle_time_out(now_tick);
	BBC_INSTANCE->backstage_brocast_handle_timeout(now_tick);
//    PANIC_SHOP_MONITOR->panic_shop_timeout(tv);
//	LOTTERY_MONITOR->check_activity_finish(tv);
//    LOGIC_MONITOR->check_update_boss_info(tv);
//    WEDDING_MONITOR->process_handle_timeout(tv);
//    LEAGUE_SYSTEM->check_league_siege(tv);
//    BROTHER_MONITOR->update_cache(tv);

	return 0;
}

LogicTenSecTimer::LogicTenSecTimer()
	: FixedTimer(GTT_LOGIC_TRANS, LOGIC_TRANSACTION_INTERVAL)
{

}

int LogicTenSecTimer::handle_timeout(const Time_Value &tv)
{
	RESTRI_SYSTEM->request_update_restriction_info();
    BACK_ACTIVITY_TICK->check_update_back_activity_tick(LOGIC_MONITOR->logic_unit());
    BACK_ACTIVITY_SYS->check_back_activity_timeout(tv);
    MAY_ACTIVITY_SYS->update_couple_tick(10);
	return 0;
}

LogicOneMinTimer::LogicOneMinTimer()
	: FixedTimer(GTT_LOGIC_ONE_MINUTE, Time_Value::MINUTE)
{
}

int LogicOneMinTimer::handle_timeout(const Time_Value &tv)
{
	SHOP_MONITOR->reqeust_load_game_shop();
	SHOP_MONITOR->request_load_mall_activity();
	SHOP_MONITOR->request_save_mall_activity(); //save db interval: 10min

	LOGIC_SWITCHER_SYS->interval_run();
	GAME_NOTICE_SYS->request_load_notice();
	FEST_ACTIVITY_SYS->request_festival_time_begin();

	this->check_online_player(tv);

//	LACKY_TABLE->check_and_reset(tv.sec());
	BACK_GAME_MODIFY_SYS->interval_run();
	return 0;
}

void LogicOneMinTimer::check_online_player(const Time_Value &tv)
{
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		player->check_and_notity_arena_tips();
//		player->time_up_recovery_strength(tv);
	}
}

LogicOneHourTimer::LogicOneHourTimer()
	: FixedTimer(GTT_LOGIC_ONE_HOUR, Time_Value::HOUR)
{
}

int LogicOneHourTimer::handle_timeout(const Time_Value &tv)
{
	//减少CPU瞬间压力
	MSG_USER("LogicOneHourTimer");

	if (std::rand() % 2 == 0)
	{
		MARKET_SYS->save_market(false);
		LOGIC_OPEN_ACT_SYS->save_open_activity_sys();
	}
	else
	{
		ARENA_SYS->save_arena_data();
		LEAGUE_SYSTEM->save_all_league(false);
	}

	if (std::rand() % 2 == 0)
	{
		WEDDING_MONITOR->request_save_all_wedding_info();
		BackIR_SYS->update_system();
		TRAVEL_TEAM_SYS->save_update_travel_team();
	}
	else
	{
		LUCKY_WHEEL_SYSTEM->save_activity();
		DAILY_ACT_SYSTEM->save_activity();
		MAY_ACTIVITY_SYS->save_activity();
	}

	return 0;
}

int LogicIntMinTimer::schedule_timer()
{
	int next_minute = GameCommon::next_minute();
	MSG_USER("LogicIntMinTimer %d", next_minute);

	return GameTimer::schedule_timer(Time_Value(next_minute));
}

int LogicIntMinTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int LogicIntMinTimer::handle_timeout(const Time_Value &tv)
{
	RANK_SYS->rank_manager_maintenance();
	RANK_SYS->req_get_hide_player();

	FLOW_INSTANCE->request_load_flow_detail();
	FLOW_INSTANCE->	check_force_refresh_rank_pannel();

	// PHP 要求统计时间点必须是整分钟(10:00,10:05,10:10)
	LOGIC_PHP->count_online_users(300);	// 5分钟统计一次在线
	BBC_INSTANCE->request_load_data_from_db();
	BACK_MAIL_SYS->request_load_back_mail_request();

	int next_minute = GameCommon::next_minute();
	MSG_USER("LogicIntMinTimer %d", next_minute);

	this->cancel_timer();
	this->GameTimer::schedule_timer(next_minute);

	return 0;
}

LogicMidNightTimer::LogicMidNightTimer()
{
	this->nextday_zero_ = 0;
}

int LogicMidNightTimer::schedule_timer()
{
	int next_day = GameCommon::next_day();
	MSG_USER("LogicMidNightTimer %d", next_day);

	this->nextday_zero_ = ::next_day(0,0).sec();
	return GameTimer::schedule_timer(Time_Value(next_day));
}

int LogicMidNightTimer::type(void)
{
	return GTT_LOGIC_ONE_MINUTE;
}

int LogicMidNightTimer::handle_timeout(const Time_Value &tv)
{
	this->nextday_zero_ = ::next_day(0,0).sec();

	ARENA_SYS->midnight_handle_timeout();
	LOGIC_OPEN_ACT_SYS->midnight_handle_timeout();
	FEST_ACTIVITY_SYS->midnight_handle_timeout();
	MAY_ACTIVITY_SYS->midnight_handle_timeout();
	LUCKY_WHEEL_SYSTEM->midnight_handle_timeout();
	DAILY_ACT_SYSTEM->midnight_handle_timeout();
	ACTIVITY_TIPS_SYSTEM->handle_tips_midnight_timeout();
	LEAGUE_SYSTEM->league_check_everyday();
	BackIR_SYS->every_day_serial_work();

	SHOP_MONITOR->reset_everyday();
	SHOP_MONITOR->mall_activity().mall_activity_refresh();

	RANK_SYS->refresh_rank_manager_at_midnight();
	LOGIC_MONITOR->reset_all_player_everyday();

	int next_day = GameCommon::next_day();
	MSG_USER("LogicMidNightTimer %d", next_day);

	this->cancel_timer();
	this->GameTimer::schedule_timer(Time_Value(next_day));

	return 0;
}

GameNoticeDetial::GameNoticeDetial()
{
	this->tick_ = 0;
	this->notify_ = 0;
	this->start_tick_ = 0;

	this->title_.clear();
	this->content_.clear();
}

ArenaRole::ArenaRole()
{
	ArenaRole::reset();
}

void ArenaRole::push_active_record(const string& name, int state, int rank_change)
{
	Record record;
	record.fight_type_ = false;
	record.fight_state_ = state;
	record.tick_ = ::time(NULL);

	record.name_ = name;
	record.rank_ = this->rank_;
	record.rank_change_ = rank_change;

	this->push_record(record);
}

void ArenaRole::push_passive_record(const string& name, int state, int rank_change)
{
	Record record;
	record.fight_type_ = true;
	record.fight_state_ = state;
	record.tick_ = ::time(NULL);

	record.name_ = name;
	record.rank_ = this->rank_;
	record.rank_change_ = rank_change;

	this->push_record(record);
}

void ArenaRole::push_record(const Record& record)
{
	if (this->his_record_.size() >= MAX_COUNT)
	{
		this->his_record_.pop_front();
	}

	this->his_record_.push_back(record);
}

void ArenaRole::reset()
{
	this->refresh_tick_ = Time_Value::zero;
	this->left_times_ = 0;
	this->buy_times_ = 0;

	this->next_fight_tick_ = 0;
	this->is_over_limit_ = 0;
	this->last_view_tick_ = 0;

	this->is_fighting_ = 0;
	this->area_index_ = 0;
	this->heap_index_ = 0;
	this->continue_win_ = 0;

	this->weapon_ = 0;
	this->clothes_ = 0;
	this->fashion_weapon_ = 0;
	this->fashion_clothes_ = 0;

	this->id_ = 0;
	this->rank_ = 0;
	this->is_skip_ = 0;
	this->last_rank_ = 0;
	this->open_flag_ = false;
	this->notify_flag_ = false;

	this->sex_ = 0;
	this->force_ = 0;
	this->level_ = 0;
	this->career_ = 0;
	this->reward_level_=0;
	this->wing_level_ = 0;
	this->solider_level_ = 0;
	this->reward_id_ = 0;

	this->name_.clear();
	this->his_record_.clear();

	this->fight_set_.clear();
	this->fight_set_.reserve(AreaSysDetail::TOTAL_FIGHTER);
}

void ArenaRole::reset_everyday()
{
	Time_Value nowtime = Time_Value::gettimeofday();
	if (this->refresh_tick_ < nowtime)
	{
		this->buy_times_ = 0;
		this->left_times_ = CONFIG_INSTANCE->athletics_base()["left_time"].asInt();
		this->refresh_tick_ = next_day(0, 0, nowtime);
	}
}

int ArenaRole::have_reward()
{
	return this->reward_id_ > 0;
}

int ArenaRole::left_cool_time()
{
	return GameCommon::left_time(this->next_fight_tick_);
}

void AreaFightNode::reset()
{
	this->area_index_ = 0;
	this->guide_flag_ = false;

	this->first_id_ = 0;
	this->second_id_ = 0;

	this->start_tick_ = 0;
	this->max_finish_tick_ = 0;
}

AreaSysDetail::AreaSysDetail()
{
	AreaSysDetail::reset();
}

void AreaSysDetail::reset()
{
	this->re_rank_ = 1;
	this->timeout_tick_ = 0;
}

bool AreaSysDetail::is_same_player(int rank)
{
	JUDGE_RETURN(this->rank_set_.count(rank) > 0, false);
	return true;
}

RechargeTimeRankDetail::RechargeTimeRankDetail(void):
	__activity_ongoing(0), __activity_op_code(0), __activity_version(0),
	__update_award_config(0), __update_extra_awarder(0)

{ /*null*/ }

void RechargeTimeRankDetail::reset_rank_player_map(void)
{
	for(RechargeRankItemMap::iterator iter = this->__recharge_rank_player_increment_map.begin();
			iter != this->__recharge_rank_player_increment_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second != 0);
		LOGIC_MONITOR->recharge_rank_item_pool()->push(iter->second);
	}

	for(RechargeRankItemMap::iterator iter = this->__recharge_rank_player_map.begin();
			iter != this->__recharge_rank_player_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second != 0);
		LOGIC_MONITOR->recharge_rank_item_pool()->push(iter->second);
	}

	this->__recharge_rank_player_increment_map.clear();
	this->__recharge_rank_player_map.clear();
	this->__recharge_rank_awarder_map.clear();
}

void RechargeTimeRankDetail::reset_extra_awarder(void)
{
	this->__extra_awarder.reset();
	this->__extra_awarder_bk.reset();
}
void RechargeTimeRankDetail::reset(void)
{
	this->__activity_ongoing = 0;
	this->__activity_op_code = 0;
	this->__activity_version = 0;
	this->__update_award_config = 0;
	this->__update_extra_awarder = 0;

	this->__update_time = Time_Value::zero;
	this->__start_time = Time_Value::zero;
	this->__end_time = Time_Value::zero;

	this->__award_mail_des.clear();
	this->__award_announce.clear();
	this->__award_mail_des_extra.clear();
	this->__award_announce_extra.clear();
	this->__extra_awarder_title.clear();

	this->__recharge_award_item_map.clear();

	this->reset_rank_player_map();
}

RechargeAwardItemDetail::RechargeAwardItemDetail(void)
	: __rank_pos(0), __award_sort(0), __award_value(0)
{ /*null*/}

void RechargeAwardItemDetail::reset(void)
{
	this->__rank_pos = 0;
	this->__award_sort = 0;
	this->__award_value = 0;
	this->__serial_code.clear();
}

RechargeRankItem::RechargeRankItem(void)
	:__role_id(0), __rank_pos(0), __award_sort(0), __award_value(0),
	 __award_valid(0)
{

}

RechargeRankExtraAwarder::RechargeRankExtraAwarder(void)
	: __role_id(0), __sex(0), __carrer(0), __weapon(0), __clothes(0),
	  __fashion_weapon(0), __fashion_clothes(0), __label(0), __timestamp(0)
{

}

void RechargeRankExtraAwarder::reset(void)
{
	this->__role_id = 0;

	this->__sex = 0;
	this->__carrer = 0;
	this->__weapon = 0;
	this->__clothes = 0;
	this->__fashion_weapon = 0;
	this->__fashion_clothes = 0;
	this->__label = 0;
	this->__timestamp = 0;

	this->__role_name.clear();
	this->__client_title.clear();
	this->__announce_des.clear();
}


void RechargeRankItem::reset(void)
{
	this->__role_id = 0;

	this->__rank_pos = 0;
	this->__award_sort = 0;
	this->__award_value = 0;
	this->__award_valid = 0;

	this->__role_name.clear();
	this->__serial_code.clear();
	this->__announce_des.clear();
}

BossDetail::BossDetail(void)
{
	BossDetail::reset();
}

void BossDetail::reset(void)
{
    this->__boss_id = 0;
    this->__boss_sort = 0;
    this->__boss_level = 0;
    this->__born_tick = Time_Value::zero;

    this->__last_award_role_id = 0;
    this->__last_award_role_name.clear();

    this->__floor = 0;
    this->__scene_id = 0;
    this->__born_point.reset();
    this->__space_id = 0;
}

void BossDetail::serialize(ProtoShusanBoss *proto_boss)
{
    proto_boss->set_boss_id(this->__boss_id);
    proto_boss->set_boss_sort(this->__boss_sort);

    int left_tick = this->__born_tick.sec() - Time_Value::gettimeofday().sec();
    if (left_tick < 0)
        left_tick = 0;
    proto_boss->set_born_time(left_tick);
    
    proto_boss->set_last_role_id(this->__last_award_role_id);
    proto_boss->set_last_role_name(this->__last_award_role_name);
    proto_boss->set_floor(this->__floor);
    proto_boss->set_scene_id(this->__scene_id);
    this->__born_point.serialize(proto_boss->mutable_born_point());
    proto_boss->set_space_id(this->__space_id);
}

bool boss_cmp(const BossDetail *left, const BossDetail *right)
{
    if (left->__boss_level == right->__boss_level)
    {
        return left->__boss_sort < right->__boss_sort;
    }
    return left->__boss_level < right->__boss_level;
}

void WeddingDetail::WeddingRole::reset(void)
{
    this->__role_id = 0;
    this->__role_name.clear();
    this->__sex = 0;
    this->__career = 0;

	this->__fetch_tick = 0;
	this->__left_times = 0;
	this->__once_reward = 0;
	this->__tick = 0;

    this->__ring_level = 0;
    this->__sys_level = 0;
    this->__tree_level = 0;
    this->__sweet_degree = 0;
}


void WeddingDetail::reset(void)
{
    this->__wedding_id = 0;
    this->__wedding_tick = Time_Value::zero;

    this->__partner_1.reset();
    this->__partner_2.reset();

    this->__day_wedding_times = 0;
    this->__day_refresh_tick = Time_Value::zero;

    this->__intimacy = 0;
    this->__history_intimacy = 0;
    this->__wedding_type = 0;
    this->__keepsake_id = 0;
    this->__keepsake_level = 0;
    this->__keepsake_sublevel = 0;
    this->__keepsake_progress = 0;

    this->__wedding_cartoon_tick = Time_Value::zero;
    this->__cruise_tick = Time_Value::zero;
}

void PlayerWeddingDetail::wedding_property::reset()
{
	this->__exp = 0;
	this->__level = 0;
	this->__side_level = 0;
	this->__side_order = 0;
}

void PlayerWeddingDetail::reset(void)
{
    this->__intimacy_map.clear();
    this->__wedding_reply.clear();
    this->__wedding_id = 0;
    this->__total_recv_flower = 0;
    this->__total_send_flower = 0;
    this->__act_total_recv_flower = 0;
    this->__act_total_send_flower = 0;

    this->__is_has_ring = 0;
    this->__side_fashion_id = 0;
    this->__side_fashion_color = 0;
    this->__wedding_pro_map.clear();
    this->__wedding_label_map.clear();
}


LuckyWheelActivity::ChangeReward::ChangeReward()
{
	ChangeReward::reset();
}

void LuckyWheelActivity::ChangeReward::reset()
{
	this->first_ 	= 0;
	this->second_ 	= 0;
	this->item_id_ 	= 0;
	this->amount_ 	= 0;
	this->bind_ 	= 0;
	this->weight_ 	= 0;
}

LuckyWheelActivity::SlotInfo::SlotInfo(void)
{
	SlotInfo::reset();
}

void LuckyWheelActivity::SlotInfo::reset(void)
{
	this->slot_id_ 		 = 0;
	this->index_ 		 = 0;
	this->the_weight_ 	 = 0;
	this->pool_percent_  = 0;
	this->person_record_ = 0;
	this->server_record_ = 0;
	this->is_shout_ 	 = 0;
	this->is_precious_ 	 = 0;
	this->appear_time_ 	 = 0;
	this->reward_mult_	 = 0;
	this->person_limit_  = 0;
	this->server_limit_  = 0;
	this->pre_cost_ 	 = 0;
	this->now_cost_ 	 = 0;
	this->must_appear_.clear();
	this->day_ 			 = 1;

	this->server_buy_ 	 = 0;

	this->rand_list_.clear();
	this->item_obj_.reset();
	this->change_reward_.clear();
	this->rand_amount_.clear();
	this->normal_item_vec_.clear();
	this->special_item_vec_.clear();

	this->item_name_.clear();
	this->item_price_ = 0;
	this->is_cast_ = 0;
	this->is_rarity_ = 0;
	this->group_id_ = 0;
	this->group_weight_ = 0;

	this->slot_type_ = 0;
	this->slot_num_ = 0;
	this->slot_clear_ = 0;
	this->item_min_times_ = 0;
	this->item_max_times_ = 0;
	this->item_show_weight_ = 0;
	this->clean_bless_ = 0;
	this->refresh_weight_.clear();
	this->other_.clear();
	this->max_count_.clear();			//同一界面出现最大个数
	this->score_ = 0;				//捕鱼积分
	this->layer_.clear();			//层数
	this->fish_type_ = 0;			//鱼类型

	this->need_item_obj_.reset();
	this->frequency_limit_ = 0;
	this->is_rare_ = 0;
}

LuckyWheelActivity::ServerItemInfo::ServerItemInfo()
{
	ServerItemInfo::reset();
}

void LuckyWheelActivity::ServerItemInfo::reset()
{
	this->player_id_   = 0;
	this->get_time_    = 0;
	this->item_id_ 	   = 0;
	this->amount_ 	   = 0;
	this->item_bind_   = 0;
	this->reward_mult_ = 0;
	this->sub_value_ = 0;

	this->player_name_.clear();
}

LuckyWheelActivity::RankPlayer::RankPlayer()
{
	RankPlayer::reset();
}

void LuckyWheelActivity::RankPlayer::reset()
{
	this->role_id_ = 0;
	this->sex_ = 0;
	this->name_.clear();
}

LuckyWheelActivity::OneRankInfo::OneRankInfo()
{
	OneRankInfo::reset();
}

void LuckyWheelActivity::OneRankInfo::reset()
{
	this->rank_ = 0;
	this->num_ = 0;
	this->role_id_ = 0;
	this->tick_ = 0;
	this->name_.clear();
}

void LuckyWheelActivity::OneRankInfo::serialize(ProtoActRankInfo *rank_info)
{
	rank_info->set_rank(this->rank_);
	rank_info->set_role_id(this->role_id_);
	rank_info->set_rank_value(this->num_);
	rank_info->set_name(this->name_);
	rank_info->set_tick(this->tick_);
}

void LuckyWheelActivity::OneRankInfo::add_rank_item(const BSONObj& res)
{
	this->role_id_ = res[DBLuckyWheelSys::OneRankSet::ROLE_ID].numberLong();
	this->tick_ = res[DBLuckyWheelSys::OneRankSet::TICK].numberLong();
	this->rank_ = res[DBLuckyWheelSys::OneRankSet::RANK].numberInt();
	this->num_  = res[DBLuckyWheelSys::OneRankSet::NUM].numberInt();
	this->name_ = res[DBLuckyWheelSys::OneRankSet::NAME].str();
}

LuckyWheelActivity::RoleMailInfo::RoleMailInfo()
{
	RoleMailInfo::reset();
}

void LuckyWheelActivity::RoleMailInfo::reset()
{
	this->role_id_ = 0;
	this->reward_map_.clear();
}

LuckyWheelActivity::ActivityDetail::ActivityDetail(void)
{
	ActivityDetail::reset();
}

void LuckyWheelActivity::ActivityDetail::reset(void)
{
	this->activity_id_ 	= 0;
	this->act_type_ 	= 0;
	this->draw_cost_	= 0;
	this->ten_cost_ 	= 0;
	this->add_gold_		= 0;
	this->add_score_	= 0;
	this->base_gold_	= 0;
	this->low_gold_ 	= 0;
	this->reset_flag_ 	= 0;
	this->continue_ 	= 0;
	this->slot_num_		= 0;
	this->shout_id_ 	= 0;
	this->red_point_ 	= 0;
	this->brocast_time_ = 0;
	this->flicker_time_ = 0;
	this->label_reward_ = 0;
	this->rank_reward_ 	= 0;
	this->rank_limit_ 	= 0;
	this->six_reward_ 	= 0;
	this->lighten_reward_ = 0;
	this->draw_limit_ 	= 0;
	this->get_bless_ 	= 0;
	this->show_bless_ 	= 0;
	this->mult_rate_ 	= 0;
	this->bless_slot_id_= 0;
	this->combine_reset_act_  = 0;
	this->combine_reset_self_ = 0;
	this->free_time_.clear();
	this->reset_cost_.clear();
	this->draw_type_list_.clear();
	this->limit_time_set_.clear();
	this->lighten_rate_.clear();
	this->bless_interval_.clear();
	this->bless_total_.clear();
	this->total_cost_.clear();

	this->save_gold_		= 0;
	this->save_date_type_ 	= 0;
	this->save_first_date_	= 0;
	this->save_last_date_	= 0;
	this->is_combine_reset_ = 0;

	this->back_date_type_	= 0;
	this->back_first_date_ 	= 0;
	this->back_last_date_	= 0;
	this->open_flag_ 		= 0;
	this->refresh_tick_ 	= 0;
	this->sort_ 			= 0;
	this->agent_.clear();
	this->act_content_.clear();

	this->shop_time_ = 0;
	this->reset_tick_ = 0;

	this->day_slot_map_.clear();
	this->time_slot_map_.clear();
	this->item_set_.clear();
	this->role_mail_map_.clear();
	this->gem_role_mail_map_.clear();
	this->bless_reward_set_.clear();
	this->rank_num_map_.clear();
	this->rank_lucky_map_.clear();
	this->player_rank_vec_.clear();
	this->lucky_rank_vec_.clear();
	this->group_slot_map_.clear();

	this->shop_time_ = 0;
	this->refresh_times_ = 0;
	this->refresh_cost_ = 0;
	this->refresh_reward_ = 0;
	this->last_refresh_tick_ = 0;
	this->mail_id_ = 0;
	this->group_limit_list_.clear();
	this->group_no_show_list_.clear();
	this->group_show_list_.clear();
	this->group_pro_list_.clear();
	this->group_may_be_list_.clear();
	this->refresh_reward_list_.clear();
	this->day_gem_synthesis_info_map_.clear();
	this->finish_free_time_.clear();
	this->finish_free_shout_index_ = 0;
	this->finish_free_shout_id_ = 0;
	this->treasure_shout_id_ = 0;
	this->finish_all_times_ = 0;
	this->slot_fina_id_ = 0;
	this->draw_return_ = 0;
	this->draw_same_scale_.clear();
}

void LuckyWheelActivity::ActivityDetail::new_act_reset()
{
	this->item_set_.clear();
	this->rank_num_map_.clear();
	this->rank_lucky_map_.clear();
	this->player_rank_vec_.clear();
	this->lucky_rank_vec_.clear();
	this->save_date_type_  = this->back_date_type_;
	this->save_first_date_ = this->back_first_date_;
	this->save_last_date_  = this->back_last_date_;
	this->reset_tick_ 	   = GameCommon::day_zero(this->back_first_date_);
	this->save_gold_ 	   = this->base_gold_;
}

void LuckyWheelActivity::ActivityDetail::combine_reset(void)
{
	this->save_gold_ 		= this->base_gold_;
	this->save_date_type_ 	= 0;
	this->save_first_date_	= 0;
	this->save_last_date_	= 0;
	this->refresh_times_ 	= 0;
	this->finish_all_times_ = 0;
	this->reset_tick_ 	   	= GameCommon::day_zero(this->back_first_date_);
	this->last_refresh_tick_= Time_Value::gettimeofday().sec();
	this->is_combine_reset_ = true;

	this->item_set_.clear();
	this->time_slot_map_.clear();
	this->rank_num_map_.clear();
	this->rank_lucky_map_.clear();
	this->player_rank_vec_.clear();
	this->lucky_rank_vec_.clear();
	this->role_mail_map_.clear();
	this->gem_role_mail_map_.clear();
	this->bless_reward_set_.clear();
}

void LuckyWheelActivity::ActivityDetail::test_reset(void)
{
	this->save_gold_ = this->base_gold_;
	this->refresh_times_ = 0;
	this->finish_all_times_ = 0;
	this->last_refresh_tick_= Time_Value::gettimeofday().sec();

	this->item_set_.clear();
	this->time_slot_map_.clear();
	this->rank_num_map_.clear();
	this->rank_lucky_map_.clear();
	this->player_rank_vec_.clear();
	this->lucky_rank_vec_.clear();
	this->role_mail_map_.clear();
	this->gem_role_mail_map_.clear();
	this->bless_reward_set_.clear();
}

void LuckyWheelActivity::ActivityDetail::check_in_role_mail(Int64 role_id, int rank)
{
	JUDGE_RETURN(this->role_mail_map_.count(role_id) <= 0, ;);

	RoleMailInfo &role_mail_info = this->role_mail_map_[role_id];
	role_mail_info.role_id_ = role_id;
	role_mail_info.reward_map_[this->label_reward_] = true;

	JUDGE_RETURN(rank <= this->rank_limit_, ;);
	role_mail_info.reward_map_[this->rank_reward_] = true;
}

void LuckyWheelActivity::ActivityDetail::erase_role_mail_reward(Int64 role_id, int reward_id)
{
	JUDGE_RETURN(this->role_mail_map_.count(role_id) > 0, ;);

	RoleMailInfo &role_mail = this->role_mail_map_[role_id];
	role_mail.reward_map_.erase(reward_id);
}

int LuckyWheelActivity::ActivityDetail::check_is_open_word(int num)
{
	JUDGE_RETURN(num < int(this->lighten_rate_.size()), false);

	int rate = this->lighten_rate_[num];
	int rand_rate = ::rand() % 100;
	return rand_rate <= rate ? true : false;
}

void LuckyWheelActivity::ActivityDetail::update_rank_info(Int64 role_id, string name, int num, int rank_type)
{
	OneRankInfo *rank_info = this->fetch_rank_info(role_id, rank_type);
	if (rank_info != NULL)
	{
		if (rank_type == RANK_TYPE_NUM)
		{
			rank_info->num_ += num;
		}
		else
		{
			JUDGE_RETURN(rank_info->num_ != 0 && rank_info->num_ > num, ;);
			rank_info->num_ = num;
		}
		rank_info->tick_ = ::time(NULL);
	}
	else
	{
		if (rank_type == RANK_TYPE_NUM)
		{
			OneRankInfo& rank_info = this->rank_num_map_[role_id];
			this->add_rank(rank_info, role_id, name, num);
		}
		else
		{
			OneRankInfo& rank_info = this->rank_lucky_map_[role_id];
			this->add_rank(rank_info, role_id, name, num);
		}
	}

	this->sort_player_rank(rank_type);
}

void LuckyWheelActivity::ActivityDetail::sort_player_rank(int rank_type)
{
	if (rank_type == RANK_TYPE_NUM)
	{
		JUDGE_RETURN(this->rank_num_map_.size() > 0, ;);

		this->player_rank_vec_.clear();
		for (RankNumMap::iterator iter = this->rank_num_map_.begin();
				iter != this->rank_num_map_.end(); ++iter)
		{
			this->push_rank_vec(iter->second, rank_type);
		}
		std::sort(this->player_rank_vec_.begin(), this->player_rank_vec_.end(),
				GameCommon::three_comp_by_desc);
	}
	else
	{
		JUDGE_RETURN(this->rank_lucky_map_.size() > 0, ;);

		this->lucky_rank_vec_.clear();
		for (RankLuckyMap::iterator iter = this->rank_lucky_map_.begin();
				iter != this->rank_lucky_map_.end(); ++iter)
		{
			this->push_rank_vec(iter->second, rank_type);
		}
		std::sort(this->lucky_rank_vec_.begin(), this->lucky_rank_vec_.end(),
				GameCommon::three_comp_by_asc);
	}

	int rank = 1;
	if (rank_type == RANK_TYPE_NUM)
	{
		for (ThreeObjVec::iterator iter = this->player_rank_vec_.begin();
				iter != this->player_rank_vec_.end(); ++iter)
		{
			OneRankInfo& rank_info = this->rank_num_map_[iter->id_];
			rank_info.rank_ = rank;
			++rank;
		}
	}
	else
	{
		for (ThreeObjVec::iterator iter = this->lucky_rank_vec_.begin();
				iter != this->lucky_rank_vec_.end(); ++iter)
		{
			OneRankInfo& rank_info = this->rank_lucky_map_[iter->id_];
			rank_info.rank_ = rank;
			++rank;
		}
	}
}

void LuckyWheelActivity::ActivityDetail::push_rank_vec(OneRankInfo &rank_info, int rank_type)
{
	ThreeObj obj;
	obj.id_ = rank_info.role_id_;
	obj.tick_ = rank_info.tick_;
	obj.value_ = rank_info.num_;

	if (rank_type == RANK_TYPE_NUM)
		this->player_rank_vec_.push_back(obj);
	else
		this->lucky_rank_vec_.push_back(obj);
}

void LuckyWheelActivity::ActivityDetail::add_rank(OneRankInfo &rank_info, Int64 role_id, string name, int num)
{
	rank_info.role_id_ = role_id;
	rank_info.name_ = name;
	rank_info.num_ = num;
	rank_info.tick_ = ::time(NULL);
}

void LuckyWheelActivity::ActivityDetail::add_weight(ChangeReward &obj, int &weight, int type)
{
	if (type == false)
		weight += obj.first_;
	else
		weight += obj.second_;
}

LuckyWheelActivity::OneRankInfo *LuckyWheelActivity::ActivityDetail::fetch_rank_info(Int64 role_id, int rank_type)
{
	if (rank_type == RANK_TYPE_NUM)
	{
		JUDGE_RETURN(this->rank_num_map_.count(role_id) > 0, NULL);
		return &(this->rank_num_map_[role_id]);
	}
	else
	{
		JUDGE_RETURN(this->rank_lucky_map_.count(role_id) > 0, NULL);
		return &(this->rank_lucky_map_[role_id]);
	}
}

ItemObj LuckyWheelActivity::ActivityDetail::fetch_lucky_egg_slot_reward(SlotInfo* slot_info, int is_color, int type)
{
	int total_weight = 0;
	if (is_color == false)
	{
		for (ChangeVec::iterator iter = slot_info->normal_item_vec_.begin();
				iter != slot_info->normal_item_vec_.end(); ++iter)
		{
			this->add_weight(*iter, total_weight, type);
		}
	}
	else
	{
		for (ChangeVec::iterator iter = slot_info->special_item_vec_.begin();
				iter != slot_info->special_item_vec_.end(); ++iter)
		{
			this->add_weight(*iter, total_weight, type);
		}
	}

	ItemObj return_item;
	JUDGE_RETURN(total_weight > 0, return_item);

	int rand_weight = ::rand() % total_weight;
	int now_weight = 0;

	if (is_color == false)
	{
		for (ChangeVec::iterator iter = slot_info->normal_item_vec_.begin();
				iter != slot_info->normal_item_vec_.end(); ++iter)
		{
			this->add_weight(*iter, now_weight, type);

			ChangeReward &obj = *iter;
			if (rand_weight <= now_weight)
			{
				ItemObj item(obj.item_id_, obj.amount_, obj.bind_);
				return item;
			}
		}
	}
	else
	{
		for (ChangeVec::iterator iter = slot_info->special_item_vec_.begin();
				iter != slot_info->special_item_vec_.end(); ++iter)
		{
			this->add_weight(*iter, now_weight, type);

			ChangeReward &obj = *iter;
			if (rand_weight <= now_weight)
			{
				ItemObj item(obj.item_id_, obj.amount_, obj.bind_);
				return item;
			}
		}
	}

	return return_item;
}

LuckyWheelActivity::LuckyWheelActivity(void)
{
	LuckyWheelActivity::reset();
}

int LuckyWheelActivity::fetch_slot_map_weight_by_group(
		ActivityDetail* act_detail, int group_id)
{
	JUDGE_RETURN(act_detail->group_weight_map_.count(group_id) >= 0, -1);
	return act_detail->group_weight_map_[group_id];
}

void LuckyWheelActivity::reset(void)
{
	this->act_detail_map_.clear();
}

void LuckyWheelActivity::add_new_item(ActivityDetail& act_detail, const BSONObj& res)
{
	act_detail.activity_id_ = res[DBBackWonderfulActivity::ACTIVITY_ID].numberInt();
	int find_flag = this->fetch_act_base_conf(act_detail);
	JUDGE_RETURN(find_flag == true, ;);

	this->update_item(&act_detail, res);
}

void LuckyWheelActivity::update_item(ActivityDetail* act_detail, const Json::Value& conf)
{
	act_detail->draw_cost_	 = conf["draw_cost"].asInt();
	act_detail->ten_cost_	 = conf["ten_cost"].asInt();
	act_detail->add_gold_	 = conf["add_gold"].asInt();
	act_detail->add_score_	 = conf["add_score"].asInt();
	act_detail->base_gold_	 = conf["base_gold"].asInt();
	act_detail->low_gold_ 	 = conf["low_gold"].asInt();
	act_detail->reset_flag_  = conf["reset_flag"].asInt();
	act_detail->shout_id_ 	 = conf["shout_id"].asInt();
	act_detail->red_point_ 	 = conf["red_point"].asInt();
	act_detail->act_shout_ 	 = conf["act_shout"].asInt();
	act_detail->draw_limit_  = conf["draw_limit"].asInt();
	act_detail->continue_	 = conf["continuous"].asInt();
	act_detail->slot_num_ 	 = conf["slot_num"].asInt();
	act_detail->brocast_time_= conf["brocast_time"].asInt();
	act_detail->flicker_time_= conf["flicker_time"].asInt();
	act_detail->label_reward_= conf["label_reward"].asInt();
	act_detail->rank_reward_ = conf["rank_reward"].asInt();
	act_detail->rank_limit_  = conf["rank_limit"].asInt();
	act_detail->six_reward_  = conf["six_reward"].asInt();
	act_detail->lighten_reward_ = conf["lighten_reward"].asInt();
	act_detail->get_bless_ 	 = conf["get_bless"].asInt();
	act_detail->show_bless_  = conf["show_bless"].asInt();
	act_detail->mult_rate_   = conf["mult_rate"].asInt();
	act_detail->combine_reset_act_ 	 = conf["combine_reset_act"].asInt();
	act_detail->combine_reset_self_  = conf["combine_reset_self"].asInt();
	act_detail->server_record_count_ = conf["server_record_count"].asInt();
	act_detail->person_record_count_ = conf["person_record_count"].asInt();

	for (uint i = 0; i < conf["draw_type_list"].size(); ++i)
	{
		act_detail->draw_type_list_[i] = conf["draw_type_list"][i].asInt();
	}

	for (uint i = 0; i < conf["free_time"].size(); ++ i)
	{
		int mins = conf["free_time"][i].asInt();
		act_detail->free_time_.push_back(mins);
	}

	for (uint i = 0; i < conf["reset_cost"].size(); ++i)
	{
		int times = conf["reset_cost"][i][0u].asInt();
		int cost  = conf["reset_cost"][i][1u].asInt();
		act_detail->reset_cost_[times] = cost;
	}

	for (uint i = 0; i < conf["limit_time"].size(); ++i)
	{
		int time = conf["limit_time"][i].asInt();
		act_detail->limit_time_set_.push_back(time);
	}

	for (uint i = 0; i < conf["lighten_rate"].size(); ++i)
	{
		int rate = conf["lighten_rate"][i].asInt();
		act_detail->lighten_rate_.push_back(rate);
	}

	//通关获得免费次数
	for (uint i = 0; i < conf["finish_free_time"].size(); ++i)
	{
		act_detail->finish_free_time_[i] = conf["finish_free_time"][i].asInt();
	}
	act_detail->finish_free_shout_index_ = 0;
	act_detail->finish_free_shout_id_ = conf["finish_free_shout_id"].asInt();
	act_detail->treasure_shout_id_ = conf["treasure_shout_id"].asInt();

	for (uint i = 0; i < conf["bless_interval"].size(); ++i)
	{
		int bless = conf["bless_interval"][i].asInt();
		act_detail->bless_interval_.push_back(bless);
	}

	for (uint i = 0; i < conf["bless_total"].size(); ++i)
	{
		ThreeObj obj;
		obj.id_    = conf["bless_total"][i][0u].asInt();
		obj.tick_  = conf["bless_total"][i][1u].asInt();
		obj.value_ = conf["bless_total"][i][2u].asInt();
		act_detail->bless_total_.push_back(obj);
	}

	for (uint i = 0; i < conf["total_cost"].size(); ++i)
	{
		ThreeObj obj;
		obj.id_    = conf["total_cost"][i][0u].asInt();
		obj.tick_  = conf["total_cost"][i][1u].asInt();
		obj.value_ = conf["total_cost"][i][2u].asInt();
		act_detail->total_cost_.push_back(obj);
	}

	//神仙鉴宝
	act_detail->draw_return_ = conf["draw_return"].asInt();
	for (uint i = 0; i < conf["draw_same_scale"].size(); ++i)
	{
		act_detail->draw_same_scale_[i] = conf["draw_same_scale"][i].asInt();
	}

	//商店信息
	act_detail->shop_time_ = conf["shop_time"].asInt();
	act_detail->refresh_cost_ = conf["refresh_cost"].asInt();
	act_detail->refresh_reward_ = conf["refresh_reward"].asInt();
	act_detail->mail_id_ = conf["mail_id"].asInt();
	act_detail->last_refresh_tick_ = Time_Value::gettimeofday().sec();

	int pro_size = (int) conf["group_pro_list"].size();
	int group_size = (int)conf["show_times"].size();
	int reward_size = (int)conf["shop_times_reward"].size();

	for(int i = 0; i < (int)conf["group_may_be_list"].size(); ++i)
	{
		act_detail->group_may_be_list_[i] = conf["group_may_be_list"][i].asInt();
	}
	for (int i = 0; i < pro_size; ++i)
	{
		act_detail->group_pro_list_[i] = conf["group_pro_list"][i].asInt();
	}

	for (int i = 0; i < group_size; ++i)
	{
		act_detail->group_show_list_[i] = conf["show_times"][i].asInt();
		act_detail->group_no_show_list_[i] = conf["no_show_times"][i].asInt();
		act_detail->group_limit_list_[i] = conf["show_limit"][i].asInt();
	}

	for (int i = 0; i < reward_size; ++i)
	{
		act_detail->refresh_reward_list_[i].refresh_reward_ = conf["shop_times_reward"][i][2u].asInt();
		act_detail->refresh_reward_list_[i].vip_level_ = conf["shop_times_reward"][i][1u].asInt();
		act_detail->refresh_reward_list_[i].refresh_times_ = conf["shop_times_reward"][i][0u].asInt();
	}

	int cur_open_day = CONFIG_INSTANCE->open_server_days();
	int max_day = 0;
	for (uint i = 0; i < conf["reward"].size(); ++i)
	{
		const Json::Value& detail_conf = conf["reward"][i];

		int day		= detail_conf["day"].asInt();
		int slot_id = detail_conf["slot"].asInt();
		SlotInfoMap &slot_map = act_detail->day_slot_map_[day];
		SlotInfo &slot_info = slot_map[slot_id];

		slot_info.slot_id_ 		 = slot_id;
		slot_info.index_ 		 = i+1;
		slot_info.the_weight_ 	 = detail_conf["the_weight"].asInt();
		slot_info.pool_percent_  = detail_conf["pool_precent"].asInt();
		slot_info.person_record_ = detail_conf["person_record"].asInt();
		slot_info.server_record_ = detail_conf["server_record"].asInt();
		slot_info.is_shout_		 = detail_conf["is_shout"].asInt();
		slot_info.is_precious_ 	 = detail_conf["is_precious"].asInt();
		slot_info.appear_time_ 	 = detail_conf["appear_times"].asInt();
		slot_info.reward_mult_   = detail_conf["reward_mult"].asInt();
		slot_info.person_limit_  = detail_conf["person_limit"].asInt();
		slot_info.server_limit_  = detail_conf["server_limit"].asInt();
		slot_info.pre_cost_ 	 = detail_conf["pre_cost"].asInt();
		slot_info.now_cost_		 = detail_conf["now_cost"].asInt();
		slot_info.day_			 = day;

		const Json::Value& must_appear = detail_conf["must_apper"];
		for (uint j = 0; j < must_appear.size(); ++j)
		{
			slot_info.must_appear_.push_back(must_appear[j].asInt());
		}

		//商店信息
		slot_info.item_name_ = detail_conf["item_name"].asString();
		slot_info.item_price_ = detail_conf["item_price"].asInt();
		slot_info.group_id_ = detail_conf["group_id"].asInt();
		slot_info.group_weight_ = detail_conf["group_weight"].asInt();
		slot_info.is_cast_ = detail_conf["is_cast"].asInt();
		slot_info.is_rarity_ = detail_conf["is_rarity"].asInt();

//		MSG_USER("ACT_TEST day %d slot %d group_id %d", day, slot_info.slot_id_, slot_info.group_id_);

		const Json::Value& item_json = detail_conf["item"];
		slot_info.item_obj_.id_ 	 = item_json[0u].asInt();
		slot_info.item_obj_.amount_  = item_json[1u].asInt();
		slot_info.item_obj_.bind_ 	 = item_json[2u].asInt();

		//神仙鉴宝
		slot_info.two_same_times_ 	= detail_conf["two_same_times"].asInt();
		slot_info.three_same_times_	= detail_conf["three_same_times"].asInt();

		//迷宫寻宝
		slot_info.slot_type_	 	= detail_conf["slot_type"].asInt();
		slot_info.slot_clear_    	= detail_conf["slot_clear"].asInt();
		slot_info.item_min_times_	= detail_conf["item_min_times"].asInt();
		slot_info.item_max_times_	= detail_conf["item_max_times"].asInt();
		slot_info.item_show_weight_ = detail_conf["item_show_weight"].asInt();
		slot_info.clean_bless_ 		= detail_conf["clean_bless"].asInt();

		if (slot_info.clean_bless_ == true)
			act_detail->bless_slot_id_ = slot_id;

		if (slot_info.item_obj_.id_ == 0 && slot_info.item_obj_.amount_ == 0 &&
				slot_info.item_obj_.bind_ == 0 && slot_info.slot_type_ == 3)
			act_detail->slot_fina_id_ 	= slot_id;

		const Json::Value& change_reward = detail_conf["change_reward"];
		for (uint j = 0; j < change_reward.size(); ++j)
		{
			ChangeReward obj;
			obj.first_ 	 = change_reward[j][0u].asInt();
			obj.second_  = change_reward[j][1u].asInt();
			obj.item_id_ = change_reward[j][2u].asInt();
			obj.amount_  = change_reward[j][3u].asInt();
			obj.bind_ 	 = change_reward[j][4u].asInt();
			obj.weight_  = change_reward[j][5u].asInt();
			slot_info.change_reward_.push_back(obj);
		}

		const Json::Value& item_normal = detail_conf["item_normal"];
		for (uint j = 0; j < item_normal.size(); ++j)
		{
			ChangeReward obj;
			obj.item_id_ = item_normal[j][0u].asInt();
			obj.amount_  = item_normal[j][1u].asInt();
			obj.bind_ 	 = item_normal[j][2u].asInt();
			obj.first_ 	 = item_normal[j][3u].asInt();
			obj.second_  = item_normal[j][4u].asInt();
			slot_info.normal_item_vec_.push_back(obj);
		}

		const Json::Value& item_special = detail_conf["item_special"];
		for (uint j = 0; j < item_special.size(); ++j)
		{
			ChangeReward obj;
			obj.item_id_ = item_special[j][0u].asInt();
			obj.amount_  = item_special[j][1u].asInt();
			obj.bind_ 	 = item_special[j][2u].asInt();
			obj.first_ 	 = item_special[j][3u].asInt();
			obj.second_  = item_special[j][4u].asInt();
			slot_info.special_item_vec_.push_back(obj);
		}

		const Json::Value& rand_amount = detail_conf["rand_amount"];
		for (uint j = 0; j < rand_amount.size(); ++j)
		{
			PairObj obj;
			obj.id_ 	= rand_amount[j][0u].asInt();
			obj.value_	= rand_amount[j][1u].asInt();
			slot_info.rand_amount_.push_back(obj);
		}

		const Json::Value& rand_list = detail_conf["rand_list"];
		for (uint j = 0; j < rand_list.size(); ++j)
		{
			slot_info.rand_list_[j] = rand_list[j].asInt();
		}

		const Json::Value& refresh_weight = detail_conf["refresh_weight"];
		if(refresh_weight.size() > 0)
		{
			for(uint j = 0; j < refresh_weight.size(); ++j)
			{
				slot_info.refresh_weight_.push_back(refresh_weight[j].asInt());
			}
		}
		else
			slot_info.refresh_weight_.push_back(0);

		const Json::Value& item_other = detail_conf["other"];
		for(uint j = 0; j < item_other.size(); ++j)
		{
			slot_info.other_.push_back(item_other[j].asInt());
		}

		const Json::Value& max_count = detail_conf["max_count"];
		for(uint j = 0; j < max_count.size(); ++j)
		{
			slot_info.max_count_.push_back(max_count[j].asInt());
		}

		slot_info.score_ = detail_conf["score"].asInt();

		const Json::Value& layer = detail_conf["layer"];
		for(uint j = 0; j < layer.size(); ++j)
		{
			slot_info.layer_.push_back(layer[j].asInt());
		}

		slot_info.fish_type_ = detail_conf["fish_type"].asInt();

		GemSynthesisInfoVec &gem_synthesis_info_vec = act_detail->day_gem_synthesis_info_map_[day];
		GemSynthesisInfo gem_syn_info_obj;
		gem_syn_info_obj.type_ = detail_conf["type"].asInt();
		const Json::Value& need_gem = detail_conf["need_gem"];
		for (uint j = 0; j < need_gem.size(); ++j)
		{
			gem_syn_info_obj.need_gem_.push_back(need_gem[j].asInt());
		}
		const Json::Value& synthesis_gem = detail_conf["synthesis_gem"];
		for (uint j = 0; j < synthesis_gem.size(); ++j)
		{
			gem_syn_info_obj.synthesis_gem_.push_back(synthesis_gem[j].asInt());
		}
		const Json::Value& reward_gem = detail_conf["reward_gem"];
		for (uint j = 0; j < reward_gem.size(); ++j)
		{
			gem_syn_info_obj.reward_gem_.push_back(reward_gem[j].asInt());
		}
		gem_synthesis_info_vec.push_back(gem_syn_info_obj);

		JUDGE_CONTINUE(cur_open_day >= day);
		if(day != max_day)
		{
			act_detail->group_slot_map_.clear();
			act_detail->group_weight_map_.clear();
			max_day = day;
		}
		SlotInfoVec &slot_map_by_group = act_detail->group_slot_map_[detail_conf["group_id"].asInt()];
		slot_map_by_group.push_back(slot_info);
		act_detail->group_weight_map_[detail_conf["group_id"].asInt()] += detail_conf["group_weight"].asInt();

		const Json::Value& need_item_json = detail_conf["need_item"];
		slot_info.need_item_obj_.id_ 	 = need_item_json[0u].asInt();
		slot_info.need_item_obj_.amount_  = need_item_json[1u].asInt();
		slot_info.need_item_obj_.bind_ 	 = need_item_json[2u].asInt();
		slot_info.frequency_limit_ = detail_conf["frequency_limit"].asInt();
		slot_info.is_rare_ = detail_conf["is_rare"].asInt();
	}
}

void LuckyWheelActivity::update_item(ActivityDetail* act_detail, const BSONObj& res)
{
	act_detail->activity_id_ 	= res[DBBackWonderfulActivity::ACTIVITY_ID].numberInt();
	act_detail->back_date_type_ = res[DBBackWonderfulActivity::DATE_TYPE].numberInt();
	act_detail->open_flag_ = res[DBBackWonderfulActivity::OPEN_FLAG].numberInt();
	act_detail->refresh_tick_ = res[DBBackWonderfulActivity::REFRESH_RICK].numberLong();
	act_detail->sort_ = res[DBBackWonderfulActivity::SORT].numberInt();
	act_detail->act_type_ = res[DBBackWonderfulActivity::ACTIVITY_TYPE].numberInt();
	act_detail->act_content_ = res[DBBackWonderfulActivity::ACT_CONTENT].str();
	Int64 back_first_date = res[DBBackWonderfulActivity::FIRST_DATE].numberLong();
	Int64 back_last_date = res[DBBackWonderfulActivity::LAST_DATE].numberLong();
	act_detail->back_first_date_ = GameCommon::day_zero(back_first_date);
	act_detail->back_last_date_ = GameCommon::day_zero(back_last_date);

	BSONObjIterator agent(res.getObjectField(DBBackWonderfulActivity::AGENT.c_str()));
	act_detail->agent_.clear();

	while (agent.more())
	{
		int id = agent.next().numberInt();
		act_detail->agent_[id] = true;
	}
}

int LuckyWheelActivity::fetch_act_base_conf(ActivityDetail& act_detail)
{
	const Json::Value& conf = CONFIG_INSTANCE->operate_activity(act_detail.activity_id_);
	JUDGE_RETURN(conf.empty() == false, false);

	this->update_item(&act_detail, conf);
	return true;
}

LuckyWheelActivity::ActivityDetail* LuckyWheelActivity::fetch_activity_detail(int activity_id, int agent_code)
{
	int update_time = 0;
	ActivityDetail* act = NULL;
	for (ActivityDetailMap::iterator iter = this->act_detail_map_.begin();
			iter != this->act_detail_map_.end(); ++iter)
	{
		ActivityDetail &act_detail = iter->second;
		JUDGE_CONTINUE(activity_id == act_detail.activity_id_);
		JUDGE_CONTINUE(act_detail.open_flag_ == true);
		JUDGE_CONTINUE(this->is_activity_time(&act_detail));
		JUDGE_CONTINUE((agent_code == -1 || act_detail.agent_.size() <= 0) ||	//不分渠道或全渠道
				act_detail.agent_.count(agent_code) > 0);
		JUDGE_CONTINUE(update_time < act_detail.refresh_tick_);

		update_time = act_detail.refresh_tick_;
		act = &act_detail;
	}

	return act;
}

LuckyWheelActivity::ActivityDetail* LuckyWheelActivity::fetch_act_detail_by_type(int act_type)
{
	JUDGE_RETURN(this->act_detail_map_.count(act_type) > 0, NULL);
	return &this->act_detail_map_[act_type];
}

LuckyWheelActivity::SlotInfoMap *LuckyWheelActivity::fetch_slot_map(ActivityDetail* act_detail, int day)
{
	JUDGE_RETURN(act_detail != NULL, NULL);

	if (act_detail->day_slot_map_.count(day) <= 0)
	{
		int max_day = 0;
		for (ActivityDetail::DaySlotInfoMap::iterator iter = act_detail->day_slot_map_.begin();
				iter != act_detail->day_slot_map_.end(); ++iter)
		{
			JUDGE_CONTINUE(day >= iter->first);
			JUDGE_CONTINUE(max_day < iter->first);
			max_day = iter->first;
		}
		return &(act_detail->day_slot_map_[max_day]);
	}
	else
	{
		return &(act_detail->day_slot_map_[day]);
	}
}

LuckyWheelActivity::SlotInfoVec*const LuckyWheelActivity::fetch_slot_map_by_group(ActivityDetail* act_detail, int group_id, int day)
{
	JUDGE_RETURN(act_detail->group_slot_map_.count(group_id) >= 0, NULL);
	SlotInfoVec &slot_vec = act_detail->group_slot_map_[group_id];
	JUDGE_RETURN(slot_vec.size() > 0, NULL);
	if(slot_vec[0].day_ != day)
	{
		slot_vec.clear();
		act_detail->group_weight_map_.clear();
		SlotInfoMap &slot_map = *this->fetch_slot_map(act_detail, day);
		for(SlotInfoMap::iterator iter = slot_map.begin(); iter != slot_map.end(); ++iter)
		{
			SlotInfo &slot_info = iter->second;
			if(slot_info.group_id_ == group_id)
			{
				slot_info.day_ = day;
				slot_vec.push_back(slot_info);
			}
			act_detail->group_weight_map_[slot_info.group_id_] += slot_info.group_weight_;
		}
	}
	return &slot_vec;
}

LuckyWheelActivity::SlotInfo* LuckyWheelActivity::gold_box_rand_get_slot(int activity_id, int type)
{
	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	JUDGE_RETURN(act_detail != NULL, NULL);

	SlotInfoMap *slot_map = this->fetch_slot_map(act_detail, 1);
	JUDGE_RETURN(slot_map != NULL, NULL);

	IntVec slot_id_set;
	int total_weight = 0;
	for (SlotInfoMap::iterator iter = slot_map->begin();
			iter != slot_map->end(); ++iter)
	{
		int slot_weight = iter->second.rand_list_[type -1];
		if (slot_weight > 0)
		{
			total_weight += slot_weight;
			slot_id_set.push_back(iter->first);
		}
	}

	JUDGE_RETURN(total_weight > 0, NULL);

	int rand_weight = ::rand() % total_weight;
	int now_weight = 0;
	for (IntVec::iterator iter = slot_id_set.begin(); iter != slot_id_set.end(); ++iter)
	{
		SlotInfoMap::iterator it = slot_map->find(*iter);
		JUDGE_CONTINUE(it != slot_map->end());

		SlotInfo &slot_info = it->second;
		int slot_weight = it->second.rand_list_[type -1];
		now_weight += slot_weight;

		if (rand_weight < now_weight) return &slot_info;
	}
	return NULL;
}

LuckyWheelActivity::SlotInfo* LuckyWheelActivity::rand_get_slot(int activity_id, Int64 wheel_times)
{
	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	JUDGE_RETURN(act_detail != NULL, NULL);

	int cur_day = this->fetch_cur_day(activity_id);
	JUDGE_RETURN(cur_day > 0, 0);

	SlotInfoMap *slot_map = this->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_map != NULL, NULL);

	IntVec slot_id_set;
	int total_weight = 0;
	for (SlotInfoMap::iterator iter = slot_map->begin();
			iter != slot_map->end(); ++iter)
	{
		int slot_weight = this->fetch_slot_wehght(&(iter->second), wheel_times);
		if (slot_weight > 0)
		{
			total_weight += slot_weight;
			slot_id_set.push_back(iter->first);
		}

		JUDGE_RETURN(this->is_must_appear_slot(&iter->second, wheel_times) == false, &iter->second);
	}
	JUDGE_RETURN(total_weight > 0, NULL);

	int rand_weight = ::rand() % total_weight;
	int now_weight = 0;
	for (IntVec::iterator iter = slot_id_set.begin(); iter != slot_id_set.end(); ++iter)
	{
		SlotInfoMap::iterator it = slot_map->find(*iter);
		JUDGE_CONTINUE(it != slot_map->end());

		SlotInfo &slot_info = it->second;
		int slot_weight = this->fetch_slot_wehght(&slot_info, wheel_times);
		now_weight += slot_weight;

		if (rand_weight < now_weight) return &slot_info;
	}

	return NULL;
}

LuckyWheelActivity::SlotInfo* LuckyWheelActivity::fetch_slot_info(
		ActivityDetail* act_detail, int day, int slot_id)
{
	SlotInfoMap *slot_map = this->fetch_slot_map(act_detail, day);
	SlotInfoMap::iterator iter = slot_map->find(slot_id);
	JUDGE_RETURN(iter != slot_map->end(), NULL);

	return &(iter->second);
}

int LuckyWheelActivity::is_activity_time(ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL, false);
	JUDGE_RETURN(act_detail->save_date_type_ == SERVER_DATE || act_detail->save_date_type_ == REAL_DATE, false);

	if (act_detail->save_date_type_ == SERVER_DATE)
	{
		Int64 cur_open_day = CONFIG_INSTANCE->open_server_days();
		JUDGE_RETURN(cur_open_day >= act_detail->save_first_date_ && cur_open_day <= act_detail->save_last_date_, false);
	}
	else
	{
		Int64 cur_tick = ::time(NULL);
		JUDGE_RETURN(cur_tick >= act_detail->save_first_date_ && cur_tick <= act_detail->save_last_date_, false);
	}
	return true;
}

int LuckyWheelActivity::is_activity_time(int activity_id)
{
	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	return this->is_activity_time(act_detail);
}

int LuckyWheelActivity::is_activity_time(int activity_id, int date_type, Int64 first_date, Int64 last_date)
{
	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	JUDGE_RETURN(act_detail != NULL, false);
	JUDGE_RETURN(date_type == SERVER_DATE || date_type == REAL_DATE, false);

	if (date_type == SERVER_DATE)
	{
		Int64 cur_open_day = CONFIG_INSTANCE->open_server_days();
		JUDGE_RETURN(cur_open_day >= first_date && cur_open_day <= last_date, false);
	}
	else
	{
		Int64 cur_tick = ::time(NULL);
		JUDGE_RETURN(cur_tick >= first_date && cur_tick <= last_date, false);
	}
	return true;
}

int LuckyWheelActivity::fetch_cur_day(int activity_id)
{
	JUDGE_RETURN(this->is_activity_time(activity_id) == true, 0);

	int now_day = CONFIG_INSTANCE->open_server_days();
//	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
//	if (act_detail->save_date_type_ == SERVER_DATE)
//	{
//		Int64 cur_open_day = CONFIG_INSTANCE->open_server_days();
//		now_day = cur_open_day - act_detail->save_first_date_ + 1;
//	}
//	else
//	{
//		Int64 cur_tick = ::time(NULL);
//		now_day = GameCommon::day_interval(cur_tick, act_detail->save_first_date_) + 1;
//	}
	return now_day;
}

int LuckyWheelActivity::fetch_left_day(int activity_id)
{
	JUDGE_RETURN(this->is_activity_time(activity_id) == true, 0);

	int left_day = 0;
	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	if (act_detail->save_date_type_ == SERVER_DATE)
	{
		Int64 cur_open_day = CONFIG_INSTANCE->open_server_days();
		left_day = act_detail->save_last_date_ - cur_open_day;
	}
	else
	{
		Int64 cur_tick = ::time(NULL);
		left_day = GameCommon::day_interval(cur_tick, act_detail->save_last_date_) - 1;
	}
	return left_day;
}

int LuckyWheelActivity::fetch_left_tick(int activity_id)
{
	int left_day = this->fetch_left_day(activity_id);
	JUDGE_RETURN(left_day >= 0, 0);

	int left_tick = GameCommon::next_day() + left_day * Time_Value::DAY;
	return left_tick;
}

int LuckyWheelActivity::cal_day_time(int time_point)
{
	int hour = time_point / 100;
	int min = time_point % 100;
	int secs = hour * 3600 + min * 60;
	return secs;
}

int LuckyWheelActivity::fecth_now_time_point(ActivityDetail* act_detail)
{
	int cur_tick = ::time(NULL) - GameCommon::today_zero();
	int time_point = 0;
	for (IntVec::iterator iter = act_detail->limit_time_set_.begin();
			iter != act_detail->limit_time_set_.end(); ++iter)
	{
		int time_point_time = this->cal_day_time(*iter);
		if (time_point_time <= cur_tick && (time_point_time + act_detail->continue_) >= cur_tick)
		{
			return *iter;
		}

		if (time_point_time > cur_tick)
			return time_point;
		else
			time_point = *iter;
	}
	return time_point;
}

int LuckyWheelActivity::fetch_next_time_point(ActivityDetail* act_detail)
{
	int cur_tick = ::time(NULL) - GameCommon::today_zero();
	for (IntVec::iterator iter = act_detail->limit_time_set_.begin();
			iter != act_detail->limit_time_set_.end(); ++iter)
	{
		int time_point_time = this->cal_day_time(*iter);
		JUDGE_CONTINUE(time_point_time > cur_tick);

		return *iter;
	}
	return 0;
}

int LuckyWheelActivity::fetch_limit_end_tick(ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL, 0);

	int cur_tick = ::time(NULL) - GameCommon::today_zero();
	int limit_end_tick = 0;
	for (IntVec::iterator iter = act_detail->limit_time_set_.begin();
			iter != act_detail->limit_time_set_.end(); ++iter)
	{
		int time_point_time = this->cal_day_time(*iter);
		JUDGE_CONTINUE(time_point_time <= cur_tick && (time_point_time + act_detail->continue_) >= cur_tick);

		limit_end_tick = time_point_time + act_detail->continue_ - cur_tick;
		break;
	}
	return limit_end_tick;
}

int LuckyWheelActivity::fetch_start_tick(ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL, 0);

	int cur_tick = ::time(NULL) - GameCommon::today_zero();
	int start_tick = 0;
	for (IntVec::iterator iter = act_detail->limit_time_set_.begin();
			iter != act_detail->limit_time_set_.end(); ++iter)
	{
		int time_point_time = this->cal_day_time(*iter);
		JUDGE_CONTINUE(time_point_time > cur_tick);

		start_tick = time_point_time - cur_tick;
		break;
	}
	return start_tick;
}

LuckyWheelActivity::SlotInfo* LuckyWheelActivity::fetch_limit_time_slot(
		ActivityDetail* act_detail, int time_point, int slot_id)
{
	JUDGE_RETURN(act_detail != NULL, NULL);
	JUDGE_RETURN(act_detail->time_slot_map_.count(time_point) > 0, NULL);

	SlotInfoMap &slot_map_info = act_detail->time_slot_map_[time_point];
	JUDGE_RETURN(slot_map_info.count(slot_id) > 0, NULL);

	return &slot_map_info[slot_id];
}

int LuckyWheelActivity::is_must_appear_slot(SlotInfo* slot_info, Int64 wheel_time)
{
	JUDGE_RETURN(slot_info != NULL, false);
	JUDGE_RETURN(slot_info->must_appear_.size() > 0, false);

	for (IntVec::iterator iter = slot_info->must_appear_.begin();
			iter != slot_info->must_appear_.end(); ++iter)
	{
		JUDGE_RETURN(wheel_time != *iter, true);
	}

	return false;
}

int LuckyWheelActivity::fetch_slot_wehght(SlotInfo* slot_info, Int64 wheel_time)
{
	JUDGE_RETURN(slot_info != NULL, 0);
	JUDGE_RETURN(wheel_time >= slot_info->appear_time_, 0);

	for (ChangeVec::iterator iter = slot_info->change_reward_.begin();
			iter != slot_info->change_reward_.end(); ++iter)
	{
		ChangeReward &reward = *iter;
		JUDGE_CONTINUE(wheel_time >= reward.first_ && wheel_time <= reward.second_);
		return reward.weight_;
	}

	return slot_info->the_weight_;
}

ItemObj LuckyWheelActivity::fetch_slot_reward(SlotInfo* slot_info, Int64 wheel_time)
{
	JUDGE_RETURN(slot_info != NULL, 0);
	JUDGE_RETURN(wheel_time >= slot_info->appear_time_, slot_info->item_obj_);
	for (ChangeVec::iterator iter = slot_info->change_reward_.begin();
			iter != slot_info->change_reward_.end(); ++iter)
	{
		ChangeReward &reward = *iter;
		JUDGE_CONTINUE(wheel_time >= reward.first_ && wheel_time <= reward.second_);

		ItemObj obj(reward.item_id_, reward.amount_, reward.bind_);
		return obj;
	}

	return slot_info->item_obj_;
}

int LuckyWheelActivity::fetch_rand_amount(SlotInfo* slot_info)
{
	JUDGE_RETURN(slot_info != NULL, 0);

	int total_weight = 0;
	for (PairObjVec::iterator iter = slot_info->rand_amount_.begin();
			iter != slot_info->rand_amount_.end(); ++iter)
	{
		total_weight += iter->value_;
	}

	JUDGE_RETURN(total_weight > 0, 0);

	int rand_weight = ::rand() % total_weight;
	int now_weight = 0;
	for (PairObjVec::iterator iter = slot_info->rand_amount_.begin();
			iter != slot_info->rand_amount_.end(); ++iter)
	{
		now_weight += iter->value_;

		if (rand_weight <= now_weight)
		{
			if (slot_info->is_precious_ == true)
				return int(iter->id_) * 2;
			else
				return int(iter->id_);
		}
	}
	return 0;
}

void LuckyWheelActivity::rand_get_slot_map(ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL, ;);
	JUDGE_RETURN(act_detail->limit_time_set_.size() > 0, ;);
	JUDGE_RETURN(act_detail->slot_num_ > 0, ;);

	act_detail->time_slot_map_.clear();

	int cur_day = this->fetch_cur_day(act_detail->activity_id_);
	SlotInfoMap *slot_info_map = this->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, ;);

	int total_weight = 0;
	for (SlotInfoMap::iterator iter = slot_info_map->begin();
				iter != slot_info_map->end(); ++iter)
	{
		SlotInfo &slot_info = iter->second;
		total_weight += slot_info.the_weight_;
	}
	JUDGE_RETURN(total_weight > 0, ;);

	for (IntVec::iterator iter = act_detail->limit_time_set_.begin();
			iter != act_detail->limit_time_set_.end(); ++iter)
	{
		int time = *iter;
		SlotInfoMap &slot_map = act_detail->time_slot_map_[time];

		if (act_detail->slot_num_ >= int(slot_info_map->size()))
		{
			for (SlotInfoMap::iterator it = slot_info_map->begin();
					it != slot_info_map->end(); ++it)
			{
				slot_map.insert(SlotInfoMap::value_type(it->first, it->second));
			}
		}
		else
		{
			IntMap slot_id_map;
			while (true)
			{
				int rand_weight = ::rand() % total_weight;
				int now_weight = 0;
				for (SlotInfoMap::iterator it = slot_info_map->begin();
						it != slot_info_map->end(); ++it)
				{
					SlotInfo &slot_info = it->second;
					now_weight += slot_info.the_weight_;
					if (rand_weight < now_weight && slot_id_map.count(slot_info.slot_id_) <= 0)
					{
						slot_map.insert(SlotInfoMap::value_type(it->first, it->second));
						slot_id_map[slot_info.slot_id_] = true;
						break;
					}
					else if (rand_weight < now_weight && slot_id_map.count(slot_info.slot_id_) > 0)
					{
						break;
					}
				}
				JUDGE_BREAK(int(slot_id_map.size()) < act_detail->slot_num_);
			}
		}
	}
}

bool LuckyWheelActivity::comp_by_time_desc(const ServerItemInfo &first, const ServerItemInfo &second)
{
	return first.get_time_ > second.get_time_;
}

void LuckyWheelActivity::record_serialize(ProtoServerRecord* server_record, ServerItemInfo& record)
{
	server_record->set_player_id(record.player_id_);
	server_record->set_player_name(record.player_name_);
	server_record->set_get_tme(record.get_time_);
	server_record->set_item_id(record.item_id_);
	server_record->set_amount(record.amount_);
	server_record->set_item_bind(record.item_bind_);
	server_record->set_reward_mult(record.reward_mult_);
	server_record->set_sub_value(record.sub_value_);
}

void LuckyWheelActivity::act_reset_every_day(ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL, ;);
	JUDGE_RETURN(act_detail->reset_flag_ == true, ;);

	act_detail->finish_all_times_ = 0;
	act_detail->finish_free_shout_index_ = 0;
}

void LuckyWheelActivity::act_end_send_mail(ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL && act_detail->mail_id_ > 0, ;);

	for (RoleMailMap::iterator iter = act_detail->role_mail_map_.begin();
			iter != act_detail->role_mail_map_.end(); ++iter)
	{
		RoleMailInfo &role_mail = iter->second;
		JUDGE_CONTINUE(role_mail.reward_map_.size() > 0);

		MailInformation *mail_info = GameCommon::create_sys_mail(act_detail->mail_id_);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str());

		for (IntMap::iterator reward_iter = role_mail.reward_map_.begin();
				reward_iter != role_mail.reward_map_.end(); ++reward_iter)
		{
			mail_info->add_goods(reward_iter->first);
		}
		GameCommon::request_save_mail_content(iter->first, mail_info);
	}
	act_detail->role_mail_map_.clear();

	int is_act_end = false;
	if (act_detail->save_date_type_ == SERVER_DATE)
	{
		Int64 cur_open_day = CONFIG_INSTANCE->open_server_days();
		if (cur_open_day > act_detail->save_last_date_)
			is_act_end = true;
	}
	else
	{
		Int64 cur_tick = ::time(NULL);
		if (cur_tick >= act_detail->save_last_date_)
			is_act_end = true;
	}

	JUDGE_RETURN(is_act_end == true, ;);

	for (RankLuckyMap::iterator iter = act_detail->rank_lucky_map_.begin();
			iter != act_detail->rank_lucky_map_.end(); ++iter)
	{
		OneRankInfo &rank_info = iter->second;
		JUDGE_CONTINUE(rank_info.rank_ <= act_detail->rank_limit_);

		MailInformation *mail_info = GameCommon::create_sys_mail(act_detail->mail_id_);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str());
		mail_info->add_goods(act_detail->rank_reward_);
		GameCommon::request_save_mail_content(iter->first, mail_info);
	}
	act_detail->rank_lucky_map_.clear();

	this->gem_act_end_send_mail(act_detail);
}

void LuckyWheelActivity::gem_act_end_send_mail(ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL, ;);
	int role_map_length = act_detail->gem_role_mail_map_.size();

	JUDGE_RETURN(role_map_length > 0, ;);

	for (RoleMailMap::iterator iter = act_detail->gem_role_mail_map_.begin();
				iter != act_detail->gem_role_mail_map_.end(); ++iter)
	{
		Int64 role_id = iter->first;
		RoleMailInfo &role_mail_info =  iter->second;
		IntMap& reward_map =  role_mail_info.reward_map_;
		JUDGE_CONTINUE(reward_map.size() > 0);

		int mail_id = act_detail->mail_id_;
		MailInformation* mail_info = GameCommon::create_sys_mail(mail_id);
		if(mail_info == NULL)
		{
			MSG_USER("LogicWheelPlayer gem_act_end_send_mail:Create mail obj error");
		}
		else
		{
			for (IntMap::iterator iter = reward_map.begin();
					iter != reward_map.end(); ++iter)
			{
				int item_id = iter->first;
				int item_amount = iter->second;
				::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
						mail_info->mail_content_.c_str());
				mail_info->add_goods(ItemObj(item_id, item_amount));
			}
			GameCommon::request_save_mail_content(role_id, mail_info);

			reward_map.clear();
		}
	}

	if (act_detail->gem_role_mail_map_.size() > 0)
	{
		MSG_USER("3ERROR gem_synthesis_detail.gem_role_mail_map_.size():%d",
				act_detail->gem_role_mail_map_.size());
	}
}

WheelPlayerInfo::ItemRecord::ItemRecord()
{
	ItemRecord::reset();
}

void WheelPlayerInfo::ItemRecord::reset()
{
	this->item_id_	 = 0;
	this->amount_	 = 0;
	this->get_time_	 = 0;
	this->item_bind_ = 0;
	this->reward_mult_ = 0;
	this->sub_value_ = 0;
}

WheelPlayerInfo::PersonSlot::PersonSlot()
{
	PersonSlot::reset();
}

void WheelPlayerInfo::PersonSlot::reset()
{
	this->time_point_ = 0;
	this->slot_id_ = 0;
	this->buy_times_ = 0;
	this->is_color_ = 0;

	this->nine_slot_.clear();
	this->item_.reset();
}

int WheelPlayerInfo::PersonSlot::is_find_word()
{
	for (IntMap::iterator iter = this->nine_slot_.begin();
			iter != this->nine_slot_.end(); ++iter)
	{
		int word = iter->second;
		JUDGE_CONTINUE(word == LuckyWheelActivity::SLOT_IS_WORD);

		return true;
	}

	return false;
}

int WheelPlayerInfo::PersonSlot::open_num()
{
	int num = 0;
	for (IntMap::iterator iter = this->nine_slot_.begin();
			iter != this->nine_slot_.end(); ++iter)
	{
		int word = iter->second;
		JUDGE_CONTINUE(word != LuckyWheelActivity::SLOT_NOT_OPEN);

		++num;
	}
	return num;
}

WheelPlayerInfo::ShopSlot::ShopSlot()
{
	ShopSlot::reset();
}

void WheelPlayerInfo::ShopSlot::reset()
{
	this->is_buy_ = 0;
	this->is_cast_ = 0;
	this->slot_id_ = 0;
	this->is_rarity_ = 0;
	this->item_price_ = 0;
	this->item_price_pre_ = 0;
	this->item_.reset();
}

bool WheelPlayerInfo::comp_by_time_desc(const ItemRecord &first, const ItemRecord &second)
{
	return first.get_time_ > second.get_time_;
}

void WheelPlayerInfo::record_serialize(ProtoPersonRecord* person_record, ItemRecord& record)
{
	person_record->set_item_id(record.item_id_);
	person_record->set_amount(record.amount_);
	person_record->set_item_bind(record.item_bind_);
	person_record->set_get_time(record.get_time_);
	person_record->set_reward_mult(record.reward_mult_);
	person_record->set_sub_value(record.sub_value_);
}

WheelPlayerInfo::PlayerDetail::PlayerDetail()
{
	PlayerDetail::reset();
}

void WheelPlayerInfo::PlayerDetail::reset()
{
	this->activity_id_ 	= 0;
	this->act_score_	= 0;
	this->wheel_times_	= 0;
	this->reset_times_ 	= 0;
	this->login_tick_ 	= 0;
	this->use_free_ 	= 0;
	this->reset_tick_ 	= 0;
	this->label_get_ 	= 0;
	this->rank_get_ 	= 0;
	this->reward_get_ 	= 0;
	this->nine_word_reward_ = 0;
	this->is_first_ 	= 0;
	this->open_times_ 	= 0;
	this->combine_reset_ = 0;

	this->rebate_map_.clear();

	this->item_record_.clear();
	this->person_slot_set_.clear();
	this->reward_location_.clear();

	this->group_refresh_times_map_.clear();
	this->shop_slot_map_.clear();
	this->refresh_tick_ = 0;
	this->refresh_reward_map_.clear();

	this->slot_index_ 	= 0;
	this->slot_scale_ 	= 0;
	this->reward_scale_ = 0;
	this->maze_free_ 	= 0;
	this->bless_ 		= 0;
	this->group_show_times_map_.clear();
	this->group_show_times_map_fina_.clear();
	this->free_times_map_.clear();

	this->slot_item_id_.clear();
	this->slot_item_num_.clear();

	this->two_same_show_times_map_.clear();
	this->three_same_show_times_map_.clear();
	this->now_slot_map_.clear();
	this->fina_slot_map_.clear();
	this->fish_info_vec_.clear();
	this->refresh_fish_flag_ = false;
	this->fish_reward_map_.clear();

	this->bless_value = 0;
	this->reward_record_map.clear();
	this->exchange_item_frequency.clear();
}

void WheelPlayerInfo::PlayerDetail::reset_every_day()
{
	this->wheel_times_ 	= 0;
	this->reset_times_ 	= 0;
	this->login_tick_  	= 0;
	this->use_free_    	= 0;
	this->label_get_   	= 0;
	this->rank_get_    	= 0;
	this->reward_get_  	= 0;
	this->open_times_ 	= 0;
	this->nine_word_reward_ = 0;

	this->person_slot_set_.clear();
	this->reward_location_.clear();

	this->slot_index_ = 0;
	this->slot_item_id_.clear();
	this->slot_item_num_.clear();

}

void WheelPlayerInfo::PlayerDetail::restart_reset()
{
	this->act_score_	= 0;
	this->wheel_times_ 	= 0;
	this->reset_times_ 	= 0;
	this->login_tick_ 	= 0;
	this->use_free_ 	= 0;
	this->label_get_ 	= 0;
	this->rank_get_ 	= 0;
	this->reward_get_   = 0;
	this->nine_word_reward_ = 0;
	this->is_first_ 	= 0;
	this->open_times_ 	= 0;

	this->rebate_map_.clear();

	this->item_record_.clear();
	this->person_slot_set_.clear();
	this->reward_location_.clear();

	this->group_refresh_times_map_.clear();
	this->shop_slot_map_.clear();
	this->refresh_tick_ = 0;
	this->refresh_reward_map_.clear();

	this->slot_index_ 	= 0;
	this->slot_scale_ 	= 0;
	this->reward_scale_ = 0;
	this->maze_free_ 	= 0;
	this->bless_ 		= 0;
	this->group_show_times_map_.clear();
	this->group_show_times_map_fina_.clear();
	this->free_times_map_.clear();

	this->slot_item_id_.clear();
	this->slot_item_num_.clear();

	this->two_same_show_times_map_.clear();
	this->three_same_show_times_map_.clear();
	this->now_slot_map_.clear();
	this->fina_slot_map_.clear();
	this->fish_info_vec_.clear();
	this->refresh_fish_flag_ = false;
	this->fish_reward_map_.clear();

	this->bless_value = 0;
	this->reward_record_map.clear();
	this->exchange_item_frequency.clear();
}

void WheelPlayerInfo::PlayerDetail::request_reset()
{
	this->reset_times_ += 1;
	this->wheel_times_ = 0;
	this->person_slot_set_.clear();
}

int WheelPlayerInfo::PlayerDetail::check_in_reward_location(int location)
{
	JUDGE_RETURN(this->reward_location_.size() > 0, false);

	for (IntVec::iterator iter = this->reward_location_.begin();
			iter != this->reward_location_.end(); ++iter)
	{
		JUDGE_CONTINUE((*iter) == location);
		return true;
	}
	return false;
}

WheelPlayerInfo::PersonSlot* WheelPlayerInfo::PlayerDetail::fetch_person_slot(int time_point, int slot_id)
{
	for (PersonSlotSet::iterator iter = this->person_slot_set_.begin();
			iter != this->person_slot_set_.end(); ++iter)
	{
		PersonSlot &person_slot = *iter;
		JUDGE_CONTINUE(person_slot.time_point_ == time_point
				&& person_slot.slot_id_ == slot_id);

		return &person_slot;
	}

	return NULL;
}

int WheelPlayerInfo::PlayerDetail::fetch_buy_times()
{
	int buy_times = 0;
	for (PersonSlotSet::iterator iter = this->person_slot_set_.begin();
			iter != this->person_slot_set_.end(); ++iter)
	{
		buy_times += iter->buy_times_;
	}
	return buy_times;
}

int WheelPlayerInfo::PlayerDetail::fetch_nine_word_num()
{
	int num = 0;
	for (PersonSlotSet::iterator iter = this->person_slot_set_.begin();
			iter != this->person_slot_set_.end(); ++iter)
	{
		PersonSlot &person_slot = *iter;
		for (IntMap::iterator it = person_slot.nine_slot_.begin();
				it != person_slot.nine_slot_.end(); ++it)
		{
			JUDGE_CONTINUE(it->second == LuckyWheelActivity::SLOT_IS_WORD);
			++num;
			break;
		}
	}
	return num;
}

int WheelPlayerInfo::PlayerDetail::fetch_total_open_num()
{
	int num = 0;
	for (PersonSlotSet::iterator iter = this->person_slot_set_.begin();
			iter != this->person_slot_set_.end(); ++iter)
	{
		PersonSlot &person_slot = *iter;
		for (IntMap::iterator it = person_slot.nine_slot_.begin();
				it != person_slot.nine_slot_.end(); ++it)
		{
			JUDGE_CONTINUE(it->second > LuckyWheelActivity::SLOT_NOT_OPEN);
			++num;
		}
	}
	return num;
}

int WheelPlayerInfo::PlayerDetail::fetch_lucky_egg_open(int type)
{
	int num = 0;
	for (PersonSlotSet::iterator iter = this->person_slot_set_.begin();
			iter != this->person_slot_set_.end(); ++iter)
	{
		PersonSlot &person_slot = *iter;

		if (type == 1)
		{
			JUDGE_CONTINUE(person_slot.item_.id_ > 0);
			++num;
		}
		else
		{
			JUDGE_CONTINUE(person_slot.is_color_ > 0);
			++num;
		}
	}
	return num;
}

WheelPlayerInfo::WheelPlayerInfo()
{
	WheelPlayerInfo::reset();
}

void WheelPlayerInfo::reset()
{
	this->player_detail_map_.clear();
}


DailyActivity::DailyDetail::DailyDetail()
{
	DailyDetail::reset();
}

void DailyActivity::DailyDetail::reset()
{
	this->activity_id_ = 0;
	this->act_type_ = 0;

	this->back_date_type_ = 0;
	this->back_first_date_ = 0;
	this->back_last_date_ = 0;
	this->refresh_tick_ = 0;
	this->value1_ = 0;
	this->value2_ = 0;
	this->open_flag_ = 0;
	this->sort_ = 0;
	this->agent_.clear();
	this->act_content_.clear();

	this->save_refresh_tick_ = 0;
	this->reset_tick_ = 0;
	this->save_first_date_ = 0;
	this->save_last_date_ = 0;
}

void DailyActivity::DailyDetail::new_act_reset()
{
	this->save_refresh_tick_ = this->refresh_tick_;
	this->save_first_date_ = this->back_first_date_;
	this->save_last_date_ = this->back_last_date_;
	this->reset_tick_ = GameCommon::day_zero(this->back_first_date_);
}

int DailyActivity::DailyDetail::is_activity_time(int use_save)
{
	Int64 cur_tick = ::time(NULL);
	if (use_save == false)
	{
		JUDGE_RETURN(cur_tick >= this->back_first_date_ && cur_tick <= this->back_last_date_, false);
	}
	else
	{
		JUDGE_RETURN(cur_tick >= this->save_first_date_ && cur_tick <= this->save_last_date_, false);
	}

	return true;
}

int DailyActivity::DailyDetail::fetch_left_day()
{
	JUDGE_RETURN(this->is_activity_time() == true, 0);

	Int64 cur_tick = ::time(NULL);
	return GameCommon::day_interval(cur_tick, this->back_last_date_) - 1;
}

int DailyActivity::DailyDetail::fetch_left_tick()
{
	int left_day = this->fetch_left_day();
	JUDGE_RETURN(left_day >= 0, 0);

	int left_tick = GameCommon::next_day() + left_day * Time_Value::DAY;
	return left_tick;
}

DailyActivity::DailyActivity()
{
	DailyActivity::reset();
}

void DailyActivity::reset()
{
	this->daily_detail_map_.clear();
}

void DailyActivity::add_new_item(DailyDetail& daily_detail, const BSONObj& res)
{
	daily_detail.activity_id_ = res[DBBackWonderfulActivity::ACTIVITY_ID].numberInt();
	int find_flag = this->fetch_act_base_conf(daily_detail);
	JUDGE_RETURN(find_flag == true, ;);

	this->update_item(&daily_detail, res);
}

void DailyActivity::update_item(DailyDetail* daily_detail, const Json::Value& conf)
{}

void DailyActivity::update_item(DailyDetail* daily_detail, const BSONObj& res)
{
	daily_detail->activity_id_ 	= res[DBBackWonderfulActivity::ACTIVITY_ID].numberInt();
	daily_detail->back_date_type_ = res[DBBackWonderfulActivity::DATE_TYPE].numberInt();
	daily_detail->open_flag_ = res[DBBackWonderfulActivity::OPEN_FLAG].numberInt();
	daily_detail->refresh_tick_ = res[DBBackWonderfulActivity::REFRESH_RICK].numberLong();
	daily_detail->sort_ = res[DBBackWonderfulActivity::SORT].numberInt();
	daily_detail->act_type_ = res[DBBackWonderfulActivity::ACTIVITY_TYPE].numberInt();
	daily_detail->act_content_ = res[DBBackWonderfulActivity::ACT_CONTENT].str();
	Int64 back_first_date = res[DBBackWonderfulActivity::FIRST_DATE].numberLong();
	Int64 back_last_date = res[DBBackWonderfulActivity::LAST_DATE].numberLong();
	daily_detail->back_first_date_ = GameCommon::day_zero(back_first_date);
	daily_detail->back_last_date_ = GameCommon::day_zero(back_last_date);
	daily_detail->value1_ = res[DBBackWonderfulActivity::VALUE1].numberInt();
	daily_detail->value2_ = res[DBBackWonderfulActivity::VALUE2].numberInt();

	BSONObjIterator agent(res.getObjectField(DBBackWonderfulActivity::AGENT.c_str()));
	daily_detail->agent_.clear();

	while (agent.more())
	{
		int id = agent.next().numberInt();
		daily_detail->agent_[id] = true;
	}
}

int DailyActivity::fetch_act_base_conf(DailyDetail& daily_detail)
{
	GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->daily_activity_map();
	GameConfig::ConfigMap::iterator iter = act_map.begin();
	for (; iter != act_map.end(); ++iter)
	{
		JUDGE_CONTINUE(daily_detail.activity_id_ == iter->first);

		this->update_item(&daily_detail, *(iter->second));
		return true;
	}

	return false;
}

DailyActivity::DailyDetail* DailyActivity::fetch_daily_detail_by_type(int act_type)
{
	JUDGE_RETURN(this->daily_detail_map_.count(act_type) > 0, NULL);
	return &this->daily_detail_map_[act_type];
}

DailyActivity::DailyDetail* DailyActivity::fetch_daily_detail(int activity_id, int agent_code)
{
	int update_time = 0;
	DailyDetail* dialy = NULL;
	for (DailyDetailMap::iterator iter = this->daily_detail_map_.begin();
			iter != this->daily_detail_map_.end(); ++iter)
	{
		DailyDetail &daily_detail = iter->second;
		JUDGE_CONTINUE(activity_id == daily_detail.activity_id_);
		JUDGE_CONTINUE(daily_detail.open_flag_ == true);
		JUDGE_CONTINUE(daily_detail.is_activity_time());
		JUDGE_CONTINUE((agent_code == -1 || daily_detail.agent_.size() <= 0) ||	//不分渠道或全渠道
				daily_detail.agent_.count(agent_code) > 0);
		JUDGE_CONTINUE(update_time < daily_detail.refresh_tick_);

		update_time = daily_detail.refresh_tick_;
		dialy = &daily_detail;
	}

	return dialy;
}


CumulativeLoginDetail::CumulativeLoginDetail()
{
	CumulativeLoginDetail::reset();
}

void CumulativeLoginDetail::reset()
{
	__single = -1;
	__ten = -1;
	__hundred = -1;
	__multiple = 0;
	__single_state = 0;
	__ten_state = 0;
	__hundred_state = 0;
	__multiple_state = 0;
	__cumulative_day = 0;
}

int CumulativeLoginDetail::get_reward()
{
	int ten = __ten * 10;
	int hundred = __hundred * 100;
	if(ten < 0)
		ten = 0;
	if(hundred < 0)
		hundred = 0;
	return __single + hundred + ten;
}

int CumulativeLoginDetail::get_multiple_reward()
{
	return get_reward() * __multiple;
}

int CumulativeLoginDetail::check_detail_by_day(int day)
{
	if(day < 0)
	{
		MSG_USER("day < 0");
		return -1;
	}
	if(__multiple > day)
	{
		MSG_USER("multiple > day");
		return -2;
	}
	switch(day)
	{
		case 1:
		{
			if(this->__hundred >= 0 ||this->__ten >= 0)
			{
				MSG_USER("hundred:%d, ten:%d", this->__hundred, this->__ten);
				return -3;
			}
			break;
		}
		case 2:
		{
			if(this->__hundred >= 0)
			{
				MSG_USER("hunred:%d", this->__hundred);
				return -4;
			}
			if(this->__ten < 0)
				this->__ten = 0;
			break;
		}
		default:
		{
			if(this->__ten < 0)
				this->__ten = 0;
			if(this->__hundred < 0)
				this->__hundred = 0;
		}
	}
	return 0;
}

void CumulativeLoginDetail::set_all_state(bool state)
{
	__multiple_state = __hundred_state = __ten_state = __single_state = state;
}


ServerRecord::ServerRecord()
{
	this->reset();
}

void ServerRecord::reset()
{
	player_id_ = 0;
	player_name_ = "";
	get_time_ = 0;
	cornucopia_gold_ = 0;
	reward_mult_ = 0;
}

CornucopiaTask::CornucopiaTask()
{
	this->reset();
}

void CornucopiaTask::reset()
{
	this->completion_times_ = 0;
	this->total_times = 0;
	this->task_id_ = 0;
	this->task_name_.clear();
}

FishInfo::FishInfo()
{
	this->reset();
}

void FishInfo::reset()
{
	this->flag_ = 0;
	this->layer_ = 0;
	this->type_ = 0;
	this->coord_.reset();
}

TravelTeamInfo::TravelTeamer::TravelTeamer()
{
	TravelTeamer::reset();
}



void TravelTeamInfo::TravelTeamer::reset()
{
	this->__teamer_id = 0;
	this->__teamer_name.clear();
	this->__teamer_sex = 0;
	this->__teamer_career = 0;
	this->__teamer_level = 0;
	this->__teamer_force = 0;
	this->__logout_tick = Time_Value::zero;
	this->__join_tick = Time_Value::zero;
}

void TravelTeamInfo::TravelTeamer::serialize(ProtoRoleInfo *msg)
{
    msg->set_role_id(this->__teamer_id);
    msg->set_role_name(this->__teamer_name);
    msg->set_role_sex(this->__teamer_sex);
    msg->set_role_career(this->__teamer_career);
    msg->set_role_level(this->__teamer_level);
    msg->set_role_force(this->__teamer_force);
}

void TravelTeamInfo::TravelTeamer::serialize(ProtoTeamer *msg)
{
    msg->set_role_id(this->__teamer_id);
    msg->set_role_name(this->__teamer_name);
    msg->set_role_sex(this->__teamer_sex);
    msg->set_role_career(this->__teamer_career);
    msg->set_role_level(this->__teamer_level);
    msg->set_role_force(this->__teamer_force);
}

void TravelTeamInfo::reset()
{
	this->__team_id = 0;
	this->__team_name.clear();
	this->__leader_id = 0;
	this->__auto_signup = 0;
	this->__auto_accept = 0;
	this->__need_force 	= 0;

	this->__is_signup = 0;
	this->__refresh_signup_tick = Time_Value::zero;
	this->__create_tick = Time_Value::zero;
	this->__last_logout_tick = Time_Value::zero;

	this->__teamer_map.clear();
	this->__apply_map.clear();

	this->__invite_role_set.clear();
}

TravelTeamInfo::TravelTeamer *TravelTeamInfo::trvl_teamer(Int64 role_id)
{
	TeamerMap::iterator iter = this->__teamer_map.find(role_id);

	if (iter != this->__teamer_map.end())
	{
		return &iter->second;
	}
	else
	{
		return NULL;
	}
}

bool TravelTeamInfo::team_is_pass_miss_day()
{
	JUDGE_RETURN(this->__last_logout_tick != Time_Value::zero, false);

	LogicPlayer *player = NULL;
	for (TeamerMap::iterator iter = this->__teamer_map.begin();
			iter != this->__teamer_map.end(); ++iter)
	{
		JUDGE_RETURN(LOGIC_MONITOR->find_player(iter->first, player) != 0, false);
	}

	Int64 cur_tick = ::time(NULL);
	int leave_tick = cur_tick - this->__last_logout_tick.sec();

	const Json::Value &peak_base = CONFIG_INSTANCE->travel_peak_base();
	int dimiss_day = peak_base["dimiss_day"].asInt();
	JUDGE_RETURN(leave_tick / Time_Value::DAY >= dimiss_day, false);

	return true;
}

bool TravelTeamInfo::member_is_pass_miss_day(Int64 role_id)
{
	TravelTeamer *teamer = this->trvl_teamer(role_id);
	JUDGE_RETURN(teamer != NULL, true);
	JUDGE_RETURN(teamer->__logout_tick != Time_Value::zero, false);

	LogicPlayer *player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) != 0, false);

	Int64 cur_tick = ::time(NULL);
	int leave_tick = cur_tick - teamer->__logout_tick.sec();

	const Json::Value &peak_base = CONFIG_INSTANCE->travel_peak_base();
	int change_leader_day = peak_base["change_leader_day"].asInt();
	JUDGE_RETURN(leave_tick / Time_Value::DAY >= change_leader_day, false);

	return true;
}

ServerItemRecord::ServerItemRecord()
{
	this->reset();
}

void ServerItemRecord::reset()
{
	player_id_ = 0;
	get_time_ = 0;
	item_id_ = 0;
	amount_ = 0;
	item_bind_ = 0;
	sub_value_ = 0;
}

SpecialBoxItem::SpecialBoxItem()
{
	this->init();
}

void SpecialBoxItem::init()
{
	slot_id_ = 0;
	name_ = "";
	min_times_ = 0;
	max_times_ = 0;
	weight_ = 0;
	is_shout_ = 0;
	server_record_ = 0;
}

WeightedRandomInfo::IdWeightDATA::IdWeightDATA(void)
{
	this->reset();
}

void WeightedRandomInfo::IdWeightDATA::reset(void)
{
	this->__id = 0;
	this->__weight = 0;
}

WeightedRandomInfo::WeightedRandomInfo(void)
{
	this->reset();
}

void WeightedRandomInfo::reset(void)
{
	this->__id_weight_set.clear();
}

int WeightedRandomInfo::reward_item_to_weighted_set(IntMap &reward_item,
		GameConfig::ConfigMap& reward_item_conf)
{
	JUDGE_RETURN(reward_item.empty() == false, -1);
	JUDGE_RETURN(reward_item_conf.empty() == false, -1);

	int flag = false;
	for (IntMap::iterator iter = reward_item.begin(); iter != reward_item.end(); ++iter)
	{
		const Json::Value &reward_item_info = reward_item_conf[iter->first];
		int rate = reward_item_info["rate"].asInt();
		int item_id = reward_item_info["item_id"].asInt();

		if (rate > 0)
		{
			IdWeightDATA id_weight_obj;
			id_weight_obj.__id = item_id;
			id_weight_obj.__weight = rate;
			this->__id_weight_set.insert(id_weight_obj);
			flag = true;
		}
	}
	JUDGE_RETURN(flag == false, -1);

	return 0;
}

int WeightedRandomInfo::weighted_random_operator(void)
{

	IdWeightSET &id_weight_set = this->__id_weight_set;
	int weight_sum = 0; //总权值
	for (WeightedRandomInfo::IdWeightSET::iterator iter = id_weight_set.begin();
			iter != id_weight_set.end(); ++iter)
	{
		weight_sum += iter->__weight;
	}

	srand(time(NULL));

	for (WeightedRandomInfo::IdWeightSET::iterator iter = id_weight_set.begin();
			iter != id_weight_set.end(); ++iter)
    {
		int rand_num = rand() % weight_sum;
		for (WeightedRandomInfo::IdWeightSET::iterator it = id_weight_set.begin();
				it != this->__id_weight_set.end(); ++it)
		{
			rand_num -= it->__weight;
	        if (rand_num < 0) {
	            return it->__id;
	        }
		}

    }

	return -1;
}

ChangeItem::ChangeItem()
{
	init();
}

void ChangeItem::init()
{
	this->group_ = 0;
	this->name_ = "";
	this->page_ = 0;
	this->score_ = 0;
	this->slot_id_ = 0;
}
