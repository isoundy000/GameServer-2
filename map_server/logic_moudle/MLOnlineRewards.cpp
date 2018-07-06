/*
 * MLOnlineRewards.cpp
 *
 *  Created on: 2014-1-16
 *      Author: root
 */

#include "MLOnlineRewards.h"
#include "ProtoDefine.h"
#include "GameHeader.h"
#include "MapLogicPlayer.h"

int MLOnlineRewards::OnlineRewardsTimer::type(void)
{
	return GTT_ML_ONE_SECOND;
}

int MLOnlineRewards::OnlineRewardsTimer::handle_timeout(const Time_Value &tv)
{
	return online_rewards_->check_rewards_timeout();
}

int MLOnlineRewards::GetAwardTimer::type(void)
{
	return GTT_ML_ONE_SECOND;
}

int MLOnlineRewards::GetAwardTimer::handle_timeout(const Time_Value &tv)
{
	return online_rewards_->get_award_timeout();
}

void MLOnlineRewards::GetAwardTimer::set_parent(MLOnlineRewards* online_rewards)
{
	this->online_rewards_ = online_rewards;
}

void MLOnlineRewards::OnlineRewardsTimer::set_parent(MLOnlineRewards* online_rewards)
{
	this->online_rewards_ = online_rewards;
}

MLOnlineRewards::MLOnlineRewards()
{
	this->online_rewards_timer_.set_parent(this);
	this->get_award_timer_.set_parent(this);
}

MLOnlineRewards::~MLOnlineRewards()
{
	// TODO Auto-generated destructor stub
}

void MLOnlineRewards::reset()
{
	this->get_award_timer_.cancel_timer();
	this->online_rewards_detail_.reset();
}

int MLOnlineRewards::login_online_rewards()
{
	JUDGE_RETURN(true == validate_online_rewards_level_limit(), 0);

	int mark = this->online_rewards_detail().__received_mark;
	Int64 tick = this->online_rewards_detail().__login_time;

	if(GameCommon::time_to_date(tick) != GameCommon::time_to_date(Time_Value::gettimeofday().get_tv().tv_sec))
	{
		// (判断昨日剩余时间是否足够领取，如果不够相当于第一次登录)
	//	if((0 == mark) || (0 != this->next_stage_times()))
		//{
			// 今天第一次登录
//			this->update_config();
			this->reset_stage();
	//	}
	}
	else
		this->online_rewards_detail().__login_time = Time_Value::gettimeofday().get_tv().tv_sec;
	JUDGE_RETURN(mark != 0, -1);
	this->online_rewards_timer_start(this->next_stage_times());
	return notify_online_rewards();
}

int MLOnlineRewards::test_online()
{
	this->fetch_online_rewards();
	return 0;

}
int MLOnlineRewards::logout_online_rewards()
{
	int cur_online = ::time(NULL) - this->online_rewards_detail().__login_time;
	this->online_rewards_detail().__online_sum += cur_online;

	this->online_rewards_timer_.cancel_timer();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_ONLINE_REWARDS);

	MSG_USER("%ld online_sum %d pre_stage %d stage %d  !!!!",
			this->role_id(), this->online_rewards_detail_.__online_sum,
			this->online_rewards_detail_.__pre_stage, this->online_rewards_detail_.__stage);

	return 0;
}

int MLOnlineRewards::fetch_online_rewards()
{
	// 是否可以领取
	int mark = this->online_rewards_detail().__received_mark;
	Int64 tick = this->online_rewards_detail().__login_time;

	CONDITION_NOTIFY_RETURN(0 == this->next_stage_times(),RETURN_ONLINE_REWARDS_FETCH_AWARDS, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(0 != mark, RETURN_ONLINE_REWARDS_FETCH_AWARDS, ERROR_CLIENT_OPERATE);
	Proto51401301 respond;

	this->online_rewards_detail_.__reward.reset();

	// 领取礼物
	for (int i = this->online_rewards_detail_.__pre_stage; i < this->online_rewards_detail_.__stage; ++i)
	{

		const Json::Value &onlinecheck_json = CONFIG_INSTANCE->onlinecheck_json(i + 1);
		const Json::Value &awardcheck_json = onlinecheck_json["award_id"];
		int award_id = awardcheck_json.asInt();

//		SerialObj obj(ADD_FROM_ONLINE_REWARDS , 0, 0);

		const Json::Value& reward_json = CONFIG_INSTANCE->reward(award_id);
		JUDGE_RETURN(reward_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

		int index = GameCommon::make_up_rand_reward_items(this->online_rewards_detail_.__reward, 1, reward_json);

		this->online_rewards_detail_.award_list.push_back(index);
		respond.add_award_list(index);

	}
	this->get_award_timer_.cancel_timer();
	this->get_award_timer_.schedule_timer(3);


	respond.set_pre_stage(this->online_rewards_detail_.__pre_stage);
	respond.set_stage(this->online_rewards_detail_.__stage);
	respond.set_login_time(this->online_rewards_detail_.__online_sum +
			Time_Value::gettimeofday().get_tv().tv_sec - this->online_rewards_detail_.__login_time);

//	this->online_rewards_detail().__received_mark = 0;
	MSG_USER("成功领取到 %ld, %d 阶段的礼物", this->role_id(), this->now_rewards_stage());

	// 处理状态


	if(GameCommon::time_to_date(tick) != GameCommon::time_to_date(time(0)))
	{
//			this->update_config();
			this->reset_stage();
			this->online_rewards_timer_start(this->next_stage_times());
			//MSG_USER("前一天未领取");
	}
	else
	{
		this->online_rewards_detail_.__pre_stage = this->online_rewards_detail_.__stage;
		if (this->online_rewards_detail_.__pre_stage == CONFIG_INSTANCE->checkmodel_json(1)["online_sum"].asInt())
		{
			this->online_rewards_detail_.__received_mark = 0;
		}
		else
		{
			this->online_rewards_timer_start(this->next_stage_times());
		}
	}


	this->notify_online_rewards();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_ONLINE_REWARDS);

	FINER_PROCESS_RETURN(RETURN_ONLINE_REWARDS_FETCH_AWARDS, &respond);
}

int MLOnlineRewards::get_online_rewards_info()
{
	Proto51401302 respond;
	const Json::Value online_check = CONFIG_INSTANCE->onlinecheck_json();
	Json::Value::iterator it = online_check.begin();
	for (; it != online_check.end(); ++it )
	{
		respond.add_config_item_id((*it)["award_id"].asInt());
	}

	IntVec::iterator i = this->online_rewards_detail_.award_list.begin();
	for (; i != this->online_rewards_detail_.award_list.end(); ++ i)
	{
		respond.add_award_list(*i);
	}

	respond.set_left_time(MAX(0, this->next_stage_times()));
	respond.set_stage(this->now_rewards_stage());
	respond.set_pre_stage(this->online_rewards_detail().__pre_stage);
	respond.set_received_mark(this->online_rewards_detail().__received_mark);
	respond.set_login_time(this->online_rewards_detail_.__online_sum +
			Time_Value::gettimeofday().get_tv().tv_sec - this->online_rewards_detail().__login_time);

	this->notify_online_rewards();
	//MSG_USER(\n %s, respond.Utf8DebugString().c_str());
	FINER_PROCESS_RETURN(RETURN_ONLINE_REWARDS_GET_INFO, &respond);
}

int MLOnlineRewards::notify_online_rewards()
{
	JUDGE_RETURN(true == validate_online_rewards_level_limit(), 0);

	Proto81401301 respond;

	int left_time = this->next_stage_times();

//	MSG_USER("online test1 领奖剩余时间%d", left_time);

	int item_count = this->online_rewards_detail_.__stage - this->online_rewards_detail_.__pre_stage;

	MapLogicPlayer * player = dynamic_cast<MapLogicPlayer *>(this);
	if (item_count == 0)
		player->update_player_assist_single_event(GameEnum::PA_EVENT_ONLINE_AWRAD, item_count);
	else
		player->update_player_assist_single_event(GameEnum::PA_EVENT_ONLINE_AWRAD,
				this->online_rewards_detail_.__pre_stage + item_count);

	JUDGE_RETURN(item_count != 0, -1);

//	MSG_USER("online test1 %d", left_time);

	respond.set_item_count(item_count);
	respond.set_left_time(MAX(0, left_time));
	respond.set_stage(this->online_rewards_detail_.__stage);
	respond.set_pre_stage(this->online_rewards_detail_.__pre_stage);
	if (this->online_rewards_detail_.__pre_stage == CONFIG_INSTANCE->checkmodel_json(1)["online_sum"].asInt())
		this->online_rewards_detail_.__received_mark = 0;

	if(0 != left_time)
	{
		respond.set_item_count(0);
	}

	//MSG_USER(\n %s, respond.Utf8DebugString().c_str());
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_ONLINE_REWARDS_INFO, &respond);
}

IntVec& MLOnlineRewards::get_award_list()
{
	return this->online_rewards_detail_.award_list;
}

int MLOnlineRewards::get_award_timeout()
{
	this->insert_package(ADD_FROM_ONLINE_REWARDS, this->online_rewards_detail_.__reward);
	this->get_award_timer_.cancel_timer();
	return 0;
}

int MLOnlineRewards::check_rewards_timeout()
{
	//MSG_USER("TIME OUT");
	JUDGE_RETURN(this->online_rewards_detail_.__received_mark != 0, -1);

	this->notify_online_rewards();
	this->online_rewards_timer_.cancel_timer();
	if (this->online_rewards_detail_.__pre_stage == CONFIG_INSTANCE->checkmodel_json(1)["online_sum"].asInt())
		this->online_rewards_timer_.cancel_timer();
	else
		this->online_rewards_timer_start(this->next_stage_times());
	return 0;
}

int MLOnlineRewards::online_rewards_timer_start(int interval)
{
	JUDGE_RETURN(interval >= 0, -1);

	if (interval == 0)
	{
		return this->notify_online_rewards();
	}
	else
	{
		return this->online_rewards_timer_.schedule_timer(interval);
	}
}

int MLOnlineRewards::online_rewards_timer_stop()
{
	this->online_rewards_timer_.cancel_timer();
	return 0;
}

OnlineRewardsDetail& MLOnlineRewards::online_rewards_detail()
{
	return this->online_rewards_detail_;
}

void MLOnlineRewards::reset_today_stage()
{
	this->reset_stage();
}

int MLOnlineRewards::next_stage_times()//计算领奖倒计时
{
	Int64 mark = this->online_rewards_detail().__received_mark;
	Int64 tick = this->online_rewards_detail().__login_time;
	Int64 online_sum = this->online_rewards_detail_.__online_sum;
	JUDGE_RETURN(0 != mark, -1);

//	const Json::Value &config = online_rewards_config();
//	JUDGE_RETURN(0 != stage_time, -1);

	int left_times = 0;

	int cur_date_stamp = GameCommon::time_to_date(::time(0));
	int last_date_stamp = GameCommon::time_to_date(tick);

	int pre_stage = this->pre_rewards_stage();

	const Json::Value &onlinecheck_json = CONFIG_INSTANCE->onlinecheck_json(pre_stage + 1);
	const Json::Value &awardcheck_json = onlinecheck_json["online_time"];
	int stage_time = awardcheck_json.asInt();

	const Json::Value &model_json = CONFIG_INSTANCE->checkmodel_json(1);
	const Json::Value &sum_json = model_json["online_sum"];
	int online_size = sum_json.asInt();

	if(cur_date_stamp == last_date_stamp)
	{
		int pass_time = ::time(0) - tick + online_sum;
		left_times = MAX(0, stage_time - pass_time);

		for (int i = pre_stage; i < online_size; ++i)
		{
			const Json::Value &onlinecheck_json = CONFIG_INSTANCE->onlinecheck_json(i + 1);
			const Json::Value &awardcheck_json = onlinecheck_json["online_time"];

			stage_time = awardcheck_json.asInt();
			if (stage_time - pass_time <= 0) this->online_rewards_detail_.__stage = i + 1;
			else break;
		}

	    return left_times;
	}
/*
	else if(cur_date_stamp == (last_date_stamp + 1))
	{// 上一天的未领取
		tm tm_now = GameCommon::fetch_cur_tm();
		left_times = Time_Value::DAY - ( tm_now.tm_hour * 3600 + tm_now.tm_min * 60 + tm_now.tm_sec );

		return (left_times >= stage_time ? 0 : -1);
	}
*/
	return -1;
}

int MLOnlineRewards::now_rewards_stage()
{
	//MSG_USER("STAGE %d", this->online_rewards_detail().__stage);
	return this->online_rewards_detail().__stage;
}

int MLOnlineRewards::pre_rewards_stage()
{
	//MSG_USER("STAGE %d", this->online_rewards_detail().__stage);
	return this->online_rewards_detail().__pre_stage;
}

int MLOnlineRewards::reset_stage()
{
	int mark = this->online_rewards_detail().__received_mark;
	Int64 tick = this->online_rewards_detail().__login_time;

	if((GameCommon::time_to_date(tick) + 1) == GameCommon::time_to_date(time(0)) &&
			(0 != mark) && (this->next_stage_times() == 0))
	{// 如果昨天有可领取奖励
		MSG_USER("yesterday online awards is still available");
	}

	this->online_rewards_timer_.cancel_timer();
	this->online_rewards_detail().__stage = 0;
	this->online_rewards_detail().__pre_stage = 0;
	this->online_rewards_detail().__received_mark = 1;
	this->online_rewards_detail().__login_time = Time_Value::gettimeofday().get_tv().tv_sec;
	this->online_rewards_detail().__online_sum = 0;
	this->online_rewards_detail().award_list.clear();
	return 0;
}


int MLOnlineRewards::sync_transfer_online_rewards(int scene_id)
{
	Proto31400118 online_rewards_info;
	online_rewards_info.set_stage(this->online_rewards_detail().__stage);
	online_rewards_info.set_pre_stage(this->online_rewards_detail().__pre_stage);
	online_rewards_info.set_received_mark(this->online_rewards_detail().__received_mark);
	online_rewards_info.set_login_time(this->online_rewards_detail().__login_time);
	online_rewards_info.set_online_sum(this->online_rewards_detail().__online_sum);
	for (IntVec::iterator it = this->online_rewards_detail_.award_list.begin();
			it != this->online_rewards_detail_.award_list.end(); ++it)
	{
		online_rewards_info.add_award_list(*it);
	}
	return this->send_to_other_logic_thread(scene_id, online_rewards_info);
}

int MLOnlineRewards::read_transfer_online_rewards(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400118*, request, -1);
	this->online_rewards_detail_.reset();

	this->online_rewards_detail().__stage = request->stage();
	this->online_rewards_detail().__received_mark = request->received_mark();
	this->online_rewards_detail().__pre_stage = request->pre_stage();
	this->online_rewards_detail().__login_time = request->login_time();
	this->online_rewards_detail().__online_sum = request->online_sum();

	for (int i = 0;i < request->award_list_size(); ++i)
	{
		this->online_rewards_detail_.award_list.push_back(request->award_list(i));
	}
	//this->online_rewards_timer_start(this->next_stage_times());
	return 0;
}

bool MLOnlineRewards::validate_online_rewards_level_limit(void)
{

//	if(this->role_detail().__scene_id ==  GameEnum::MINI_SCRIPT_SCENE_1)
//		return false;

	int use_online_rewards_level = 1;


	return this->role_level() >= use_online_rewards_level;
}
