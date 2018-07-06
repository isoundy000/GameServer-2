/*
 * LogicMayActivityer.cpp
 *
 *  Created on: 2017年4月13日
 *      Author: lyw
 */

#include "LogicMayActivityer.h"
#include "MayActivitySys.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"

LogicMayActivityer::CachedTimer::CachedTimer(void):player_(0)
{
}

LogicMayActivityer::CachedTimer::~CachedTimer(void)
{
}

int LogicMayActivityer::CachedTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int LogicMayActivityer::CachedTimer::handle_timeout(const Time_Value& nowtime)
{
	JUDGE_RETURN(this->player_ != NULL, -1);
	return this->player_->notify_is_open_red_packet();
}

LogicMayActivityer::LogicMayActivityer() {
	// TODO Auto-generated constructor stub

}

LogicMayActivityer::~LogicMayActivityer() {
	// TODO Auto-generated destructor stub
}

void LogicMayActivityer::reset_may_activityer()
{
	this->may_detial_.reset();
}

void LogicMayActivityer::refresh_may_act()
{
	JUDGE_RETURN(this->may_detial_.act_type_ != MAY_ACTIVITY_SYS->act_type()
			&& MAY_ACTIVITY_SYS->act_type() > 0, ;);

	LogicRoleDetail &role_detail = this->logic_player()->role_detail();
//
//	this->reset_every_day_by_change(1);
	role_detail.act_data_reset_flag_ = false;

	this->may_detial_.act_type_ = MAY_ACTIVITY_SYS->act_type();
	this->may_detial_.act_type_item_set_.clear();

	role_detail.cur_red_packet_group_ = 0;
	role_detail.cumulative_times_ = 0;
	role_detail.continuity_login_day_ = 1;
	role_detail.continuity_login_flag_ = false;

	this->daily_login_refresh_day();
}

void LogicMayActivityer::refresh_act_type_item(int index)
{
	JUDGE_RETURN(this->may_detial_.act_type_item_set_.count(index) == 0, ;);

	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(
			index, this->role_detail().__agent_code);
	JUDGE_RETURN(act_info != NULL, ;);

	MayActivityer::ActTypeItem& act_item = this->may_detial_.act_type_item_set_[index];
	act_item.cur_index_ = index;
}

MayActivityer::ActTypeItem& LogicMayActivityer::find_may_act_item(int index)
{
	this->refresh_act_type_item(index);
	return this->may_detial_.act_type_item_set_[index];
}

int LogicMayActivityer::has_may_activity_reward(MayActDetail::ActInfo& act_info)
{
	for (uint i = 0; i < act_info.reward_set_.size(); ++i)
	{
		JUDGE_CONTINUE(this->has_may_activity_reward(act_info, i) == BackSetActDetail::UNDRAW);
		return true;
	}

	return false;
}

void LogicMayActivityer::may_act_start_tick(LogicPlayer *player)
{
	JUDGE_RETURN(player != NULL, );
	this->timer_.player_ = player;
	this->timer_.schedule_timer(1);
}

void LogicMayActivityer::may_act_stop_tick()
{
	this->timer_.cancel_timer();
}

int LogicMayActivityer::notify_is_open_red_packet()
{
    int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
    MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(act_info != NULL, -1);

	return this->fetch_cur_time_open_state(act_info->act_index_);
}

int LogicMayActivityer::notify_cur_group_reward_info(MayActDetail::ActInfo *act_info)
{
	JUDGE_RETURN(act_info != NULL, -1);
	LogicRoleDetail &role_detail = this->role_detail();
	uint cur_red_group = role_detail.cur_red_packet_group_;
	MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
	JUDGE_RETURN(cur_red_group <= reward_set.size(), -1);
    MayActDetail::ActReward& reward = reward_set[cur_red_group - 1];
    Proto50100256 respond;
    respond.set_red_packet_max(reward.get_times_);
    respond.set_bing_money(reward.packet_money_);
    MayActDetail::IndexMap player_reward_info = act_info->player_reward_info_[cur_red_group];
    MayActDetail::IndexMap::iterator iter = player_reward_info.begin();
    for( ; iter != player_reward_info.end(); ++iter)
    {
    	MayActDetail::LUMap &single_red_player_info = iter->second;
    	MayActDetail::LUMap::iterator single_iter = single_red_player_info.begin();
    	for( ; single_iter != single_red_player_info.end(); ++single_iter)
    	{
    		MayActDetail::PlayerInfo &player_info = single_iter->second;
			if(this->role_id() == single_iter->first)
				respond.set_self_money(player_info.money_);
			ProtoRedPacketInfo *info = respond.add_red_packet_info();
			info->set_money(player_info.money_);
			info->set_player_name(player_info.name_);
			//MSG_USER("cur group:%d, player_name:%s, money:%d", cur_red_group, player_info.name_.c_str(), player_info.money_);
    	}
    }

    return this->respond_to_client(RETURN_FETCH_RED_PACKET_REWARD, &respond);
}

void LogicMayActivityer::update_red_act_info_list()
{
    int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
    MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(act_info != NULL, );
    MayActDetail::RedActInfoVec &red_info_vec = act_info->red_act_info_vec_;
    MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
    if(MAY_ACTIVITY_SYS->is_act_open_time(MayActDetail::SEND_POCKET, cur_passed_day))
    {
    	int cur_time = ::time(0);
		int interval = cur_time - act_info->cur_open_red_act_tick_;
		for(uint i = 0; i < red_info_vec.size(); ++i)
		{
			MayActDetail::RedActInfo &info = red_info_vec[i];
			if(info.state_ == MayActDetail::RedActInfo::BEGIN)
			{
				//|| i != red_info_vec.size() - 1
				if(!this->fetch_single_red_packet_info(i + 1) )
					info.state_ = MayActDetail::RedActInfo::FINISH;
				info.money_ = reward_set[i].packet_money_;
			}
			else if(info.state_ == MayActDetail::RedActInfo::READY)
			{
				if(interval >= reward_set[i].red_act_interval_ * (int)i)
				{
					info.state_ = MayActDetail::RedActInfo::BEGIN;
					info.money_ = reward_set[i].packet_money_;
					if(i + 1 < red_info_vec.size())
					{
						red_info_vec[i + 1].state_ = MayActDetail::RedActInfo::READY;
						red_info_vec[i + 1].tick_ = reward_set[i].red_act_interval_ - (i * reward_set[i].red_act_interval_ - cur_time);
					}
					return this->update_red_act_info_list();
				}
				else
				{
					info.tick_ = reward_set[i].red_act_interval_ - (interval - (i - 1) * reward_set[i].red_act_interval_);
				}
			}
		}
    }
    else
    {
    	for(uint i = 0; i < red_info_vec.size(); ++i)
		{
    		red_info_vec[i].reset();
		}
    }
}

int LogicMayActivityer::has_may_activity_reward(MayActDetail::ActInfo& act_info, int index)
{
	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info.act_index_);
	MayActivityer::ActItem& s_act_item = m_act_item.act_item_map_[index];

	if (s_act_item.arrive_ == 0)
	{
		//条件未达到
		return MayActDetail::CNDRAW;
	}

	if (s_act_item.drawed_ < s_act_item.arrive_)
	{
		//未领取
		return MayActDetail::UNDRAW;
	}

	//已领取
	return MayActDetail::DRAWED;
}

void LogicMayActivityer::serialize_sub_value(Proto50100252 *respond, int index)
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(
			index, this->role_detail().__agent_code);
	JUDGE_RETURN(act_info != NULL, ;);

	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info->act_index_);

	switch (act_info->first_type_)
	{
	case MayActDetail::DAILY_RUN:
	{
		respond->set_sub_value(m_act_item.sub_value_);
		respond->set_second_sub(m_act_item.second_sub_);

		if (act_info->run_role_map_.count(this->role_id()) <= 0)
		{
			MayActDetail::RunInfo &info = act_info->run_role_map_[this->role_id()];
			info.role_id_ = this->role_id();
			info.name_ = this->name();
			info.sex_ = this->role_detail().__sex;
		}

		MayActDetail::RunInfo &player_run = act_info->run_role_map_[this->role_id()];
		respond->set_third_sub(player_run.location_);
		break;
	}
	case MayActDetail::COUPLE_HEART:
	{
		LogicPlayer *player = this->logic_player();
		Int64 wedding_id = player->wedding_id();
		JUDGE_RETURN(act_info->couple_info_map_.count(wedding_id) > 0,;);

		MayActDetail::CoupleInfo &couple_info = act_info->couple_info_map_[wedding_id];
		respond->set_sub_value(couple_info.online_tick_ / 60);
		respond->set_second_sub(couple_info.buy_tick_ / 60);

		LogicPlayer *partner = player->fetch_wedding_partner();
		if (partner != NULL)
		{
			respond->set_third_sub(true);
		}

		break;
	}
	default:
		break;
	}
}

void LogicMayActivityer::check_rand_role_view(MayActDetail::ActInfo& act_info)
{
	JUDGE_RETURN(act_info.rand_vec.size() > 0, ;);

	MayActDetail::RunInfo &my_run = act_info.run_role_map_[this->role_id()];
	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info.act_index_);

	LongVec del_vec;
	for (LongMap::iterator iter = m_act_item.role_map_.begin();
			iter != m_act_item.role_map_.end(); ++iter)
	{
		MayActDetail::RunInfo &player_run = act_info.run_role_map_[iter->first];
		if (iter->second == true)
		{
			ThreeObj &obj = act_info.rand_vec[1];
			JUDGE_CONTINUE(player_run.location_ - obj.sub_ <= my_run.location_);
		}
		else
		{
			ThreeObj &obj = act_info.rand_vec[0];
			JUDGE_CONTINUE(player_run.location_ - obj.sub_ >= my_run.location_);
		}

		del_vec.push_back(iter->first);
	}

	for (LongVec::iterator iter = del_vec.begin(); iter != del_vec.end(); ++iter)
	{
		m_act_item.role_map_.erase(*iter);
	}

	this->create_rand_role_view(act_info);
}

int LogicMayActivityer::fetch_fashion_info(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto50100252*, request, -1);
	int act_id = request->index();
	MayActDetail::ActInfo *act_detail = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::NICE_FASHION, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_detail != NULL && act_id == act_detail->act_index_, 0);

	MayActivityer::ActTypeItem& player_detail = this->find_may_act_item(act_id);
	if(player_detail.cur_fashion_times_ <= 0)
		player_detail.cur_fashion_times_ = 1;
	int times = player_detail.cur_fashion_times_;

	request->set_reward_show_id(act_detail->reward_show_id_);

	ProtoMayFashionInfo *fashion_info = request->mutable_act_fashion_info();
	fashion_info->set_liveness(player_detail.liveness_);

	if(player_detail.liveness_map_.empty())
	{
		GameConfig::ConfigMap &fashion_info_config = CONFIG_INSTANCE->fashion_act_info();
		for(uint i = 1; i <= fashion_info_config.size(); ++i)
		{
			Json::Value info = *fashion_info_config[i];
			int config_times = info["times"].asInt();
			int index = info["index"].asInt();
			player_detail.liveness_map_[config_times][index] = false;
			if(config_times == 1)
				player_detail.max_cond_count_ = index;
		}
	}

	GameConfig::ConfigMap &fashion_info_config = CONFIG_INSTANCE->fashion_act_info();
	for(uint i = 1; i <= fashion_info_config.size(); ++i)
	{
		Json::Value info = *fashion_info_config[i];
		int config_time = info["times"].asInt();
		int config_index = info["index"].asInt();
		if(config_time == times)
		{
			ProtoItemId *item = fashion_info->add_item();
			int index = info["index"].asInt();
			if(player_detail.second_sub_ != 0 && index == player_detail.max_cond_count_)
				item->set_id(player_detail.second_sub_);
			else
				item->set_id(info["reward_id"].asInt());

			int cond = info["move"].asInt();
			item->set_cond(cond);
			int state = -1;
			if(player_detail.liveness_ >= cond)
			{
				state = player_detail.liveness_map_[times][config_index];
			}
			item->set_state(state);
		}
		player_detail.max_fashion_times_ = config_time;
		//MSG_USER("time:%d, index:%d, state:%d", config_time, config_index, player_detail.liveness_map_[config_time][config_index]);
	}

	fashion_info->set_one_times_money(act_detail->limit_cond_[1]);
	fashion_info->set_ten_times_money(act_detail->limit_cond_[3]);

	return 0;
}

int LogicMayActivityer::fetch_fashion_buy_begin(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100265*, request, -1);
	int buy_times = request->times();
	JUDGE_RETURN(buy_times > 0, -1);

	MayActDetail::ActInfo *act_detail = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::NICE_FASHION, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_detail != NULL, -1);

	int money = 0;
	if(buy_times == act_detail->limit_cond_[0])
		money = act_detail->limit_cond_[1];
	else if(buy_times == act_detail->limit_cond_[2])
		money = act_detail->limit_cond_[3];
	else
	{
		MSG_USER("config error,limit_cond buy_time != limit_cond!!");
		return 0;
	}

	MayActivityer::ActTypeItem& player_detail = this->find_may_act_item(act_detail->act_index_);

	player_detail.shop_slot_vec_temp_.clear();

	for(int i = 0; i < buy_times; ++i)
	{
		player_detail.shop_slot_map_.clear();
		this->may_rand_slot_item(act_detail->act_index_, 1);
		for(MayActivityer::ShopSlotMap::iterator iter = player_detail.shop_slot_map_.begin(); iter != player_detail.shop_slot_map_.end(); ++iter)
		{
			player_detail.shop_slot_vec_temp_.push_back(iter->second);
		}
	}

	int rand_item_size = player_detail.shop_slot_vec_temp_.size();
	MSG_USER("rand item count:%d， buy_time:%d", rand_item_size, buy_times);
	JUDGE_RETURN(rand_item_size > 0, -1);
	JUDGE_RETURN(rand_item_size == buy_times, -1);

	int times = player_detail.cur_fashion_times_;
	int cur_times_max_liveness = 0;
	GameConfig::ConfigMap &fashion_info_config = CONFIG_INSTANCE->fashion_act_info();
	for(uint i = 1; i <= fashion_info_config.size(); ++i)
	{
		Json::Value info = *fashion_info_config[i];
		int config_times = info["times"].asInt();
		if(config_times == times)
			cur_times_max_liveness = info["move"].asInt();

	}

	if(player_detail.liveness_ >= cur_times_max_liveness)
	{
		IntMap &reward_state = player_detail.liveness_map_[times];
		for(IntMap::iterator iter = reward_state.begin(); iter != reward_state.end(); ++iter)
		{
			//有奖励没有领取，不让继续抽
			if(iter->second == false)
				return this->respond_to_client_error(RETURN_FASHION_ACT_TAKE_REWARD, ERROR_TASK_UNSUBMIT_EXP);
		}
	}

	Proto31400055 inner;
	inner.set_type(MayActDetail::NICE_FASHION);
	inner.set_first_index(act_detail->act_index_);
	inner.set_money(money);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicMayActivityer::fetch_fashion_buy_End(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);
	MayActDetail::ActInfo *act_detail = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::NICE_FASHION, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_detail != NULL, -1);

	CONDITION_NOTIFY_RETURN(request->ret() == 0, RETURN_FASHION_ACT_TAKE_REWARD, request->ret());
	int act_id = request->first_index();
	MayActivityer::ActTypeItem& player_detail = this->find_may_act_item(act_id);
	int item_count = player_detail.shop_slot_vec_temp_.size();
	JUDGE_RETURN(item_count > 0, -1);

	Proto50100265 respond;
	MayActivityer::ShopSlotVec &slot_vec = player_detail.shop_slot_vec_temp_;
	int &times = player_detail.cur_fashion_times_;
	int cur_times_max_liveness = 0;
	int max_times = player_detail.max_fashion_times_;
	int cond_reward = 0;
	int change_reward = 0;
	GameConfig::ConfigMap &fashion_info_config = CONFIG_INSTANCE->fashion_act_info();
	for(uint i = 1; i <= fashion_info_config.size(); ++i)
	{
		Json::Value info = *fashion_info_config[i];
		int config_times = info["times"].asInt();
		if(config_times == times)
			cur_times_max_liveness = info["move"].asInt();

		int cond = info["cond"].asInt();
		if(cond)
		{
			cond_reward = info["cond"].asInt();
			change_reward = info["change_reward"].asInt();
		}
	}

	//增加活跃度
	player_detail.liveness_ += slot_vec.size() * act_detail->limit_cond_[4];

	MSG_USER("player liveness:%d", player_detail.liveness_);

	//活跃度超过后，设置为最后一轮, 并且清空当前获得记录
	if(times > max_times)
	{
		times = max_times;
		player_detail.liveness_map_[times].clear();
	}

	for(MayActivityer::ShopSlotVec::iterator iter = slot_vec.begin(); iter != slot_vec.end(); ++iter)
	{
		MayActivityer::ShopSlot &shop_info = *iter;
		ProtoItemId *item = respond.add_item();
		ItemObj item_obj = shop_info.slot_info_.item_obj_;
		item->set_id(item_obj.id_);
		item->set_amount(item_obj.amount_);
		item->set_bind(item_obj.bind_);
		SerialObj obj(ADD_FROM_FASHION_TAKE_REWARD, act_id);
		this->request_add_item(obj, item_obj.id_, item_obj.amount_, item_obj.bind_);
		if(item_obj.id_ == cond_reward)
		{
			player_detail.second_sub_ = change_reward;
			this->may_activity_announce(act_id, shop_info.slot_info_.index_);
		}
	}

	this->fetch_may_activity_info(act_id);
	return this->respond_to_client(RETURN_FASHION_ACT_TAKE_REWARD, &respond);
}

int LogicMayActivityer::fetch_fashion_reward(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100266*, request, -1);
	int index = request->index() + 1;

	int act_id = request->activity_id();
	MayActivityer::ActTypeItem& player_detail = this->find_may_act_item(act_id);
	//当前玩家的活跃度
	int liveness = player_detail.liveness_;

	//记住某特殊奖励
	int fashion_id = player_detail.second_sub_;

	//当前轮数
	int &times = player_detail.cur_fashion_times_;

	GameConfig::ConfigMap &fashion_info_config = CONFIG_INSTANCE->fashion_act_info();

	int cur_times_max_liveness = 0;
	for(uint i = 1; i <= fashion_info_config.size(); ++i)
	{
		Json::Value info = *fashion_info_config[i];
		int config_times = info["times"].asInt();
		if(config_times == times)
			cur_times_max_liveness = info["move"].asInt();
	}

	for(uint i = 1; i <= fashion_info_config.size(); ++i)
	{
		Json::Value fashion_info = *fashion_info_config[i];
		int config_times = fashion_info["times"].asInt();
		if(config_times != times)
			continue;
		int cond = fashion_info["move"].asInt();
		if(liveness >= cond)
		{
			int info_index = fashion_info["index"].asInt();
			//判断当前活跃度是否由领取

			if(player_detail.liveness_map_[times][index] == false && index == info_index)
			{
				int reward_id = fashion_info["reward_id"].asInt();
				//若第一轮抽到特殊时装，就更换id
				int cond = fashion_info["cond"].asInt();
				if(fashion_id && times == 1 && cond)
				{
					reward_id = fashion_info["change_reward"].asInt();
					MSG_USER("is change reward:%d", reward_id);
				}
				SerialObj obj(ADD_FROM_FASHION_LIVENESS_REWARD, act_id);
				this->request_add_reward(reward_id, obj);
				player_detail.liveness_map_[times][info_index] = true;
				MSG_USER("get reward: times:%d, index:%d, state:%d", times, info_index, player_detail.liveness_map_[times][info_index]);
			}
		}
	}

	//增加轮数，并补充玩家超过的活跃读值
	if(player_detail.liveness_ >= cur_times_max_liveness)
	{
		int flag = true;
		//没有领取，false
		IntMap::iterator iter = player_detail.liveness_map_[times].begin();
		for( ; iter != player_detail.liveness_map_[times].end(); ++iter)
		{
			if(!iter->second)
			{
				flag = iter->second;
				break;
			}
		}
		if(flag)
		{
			player_detail.liveness_ -= cur_times_max_liveness;
			++times;
			MSG_USER("times:%d", times);
			if(times > player_detail.max_fashion_times_)
			{
				times = player_detail.max_fashion_times_;
				player_detail.liveness_map_[times].clear();
			}
		}
	}

	this->fetch_may_activity_info(act_id);
	return this->respond_to_client(RETURN_FASHION_ACT_LIVENESS_REWARD);
}

int LogicMayActivityer::may_rand_slot_item(int activity_id, int slot_num)
{
	//活动数据
	MayActDetail::ActInfo *act_detail = MAY_ACTIVITY_SYS->find_act_info(activity_id);
	JUDGE_RETURN(act_detail != NULL, -1);

	//玩家数据
	MayActivityer::ActTypeItem& player_detail = this->find_may_act_item(activity_id);
	//获取组个数，最后一组为补漏
	int group_count = act_detail->group_no_show_list_.size();
	CONDITION_NOTIFY_RETURN(group_count > 0, RETURN_REQUEST_TIME_LIMIT, ERROR_CONFIG_ERROR);

	//必出组集合
	IntSet must_show_set;
	IntSet may_be_show_set;
	//组id从1开始 插入符合次数条件的组
	for (int i = 0; i < group_count; ++i)
	{
		//这里group_no_show_list 和 group_show_list 索引从0开始的
		if (player_detail.group_refresh_times_map_[i] >= act_detail->group_no_show_list_[i])
		{
			if (player_detail.group_refresh_times_map_[i] >= act_detail->group_show_list_[i])
			{
				must_show_set.insert(i + 1);
				MSG_USER("must_show group:%d, refresh_times:%d", i + 1, player_detail.group_refresh_times_map_[i]);
			}
			else
			{
				may_be_show_set.insert(i + 1);
				MSG_USER("may_be_show group:%d, refresh_times:%d", i + 1, player_detail.group_refresh_times_map_[i]);
			}
		}
	}

	//必出个数概率分布
	IntMap show_limit_pro_map;
	int pro_sum = 0;
	for (size_t i = 0; i < act_detail->group_pro_list_.size(); ++i)
	{
		pro_sum += act_detail->group_pro_list_[i];
		show_limit_pro_map[i] = pro_sum;
	}

	//对应组的个数
	IntMap group_item_count_map;
	int sum_group_item_count = 0;
	//必出组插入
	for (IntSet::const_iterator iter = must_show_set.begin(); iter != must_show_set.end(); ++iter)
	{
		//若是补漏组，跳出
		if (*iter == group_count)
			break;
		int rand_num = std::rand() % pro_sum;
		//MSG_USER("no group_count, cur rand_num:%d", rand_num);
		//show_limit_pro_map索引为个数，0代表1个
		for (IntMap::const_iterator limit_iter = show_limit_pro_map.begin(); limit_iter != show_limit_pro_map.end(); ++limit_iter)
		{
			MSG_USER("cur limit:%d, rand_num:%d", limit_iter->second, rand_num);
			if (limit_iter->second >= rand_num)
			{
				group_item_count_map[*iter] = limit_iter->first + 1;
				break;
			}
		}

		int max_rand_times = act_detail->group_limit_list_[*iter - 1];
		if (group_item_count_map[*iter] > max_rand_times)
			group_item_count_map[*iter] = max_rand_times;
		sum_group_item_count += group_item_count_map[*iter];
	}

	//可能出组插入
	for (IntSet::const_iterator iter = may_be_show_set.begin(); iter != may_be_show_set.end(); ++iter)
	{
		//必出组有该组的时候,可出组不出
		if (must_show_set.count(*iter) > 0)
			break;
		CONDITION_NOTIFY_RETURN(act_detail->group_may_be_list_.count(*iter - 1) > 0, RETURN_REQUEST_TIME_LIMIT, ERROR_CONFIG_ERROR);
		int cur_may_be_weight = act_detail->group_may_be_list_[*iter - 1];
		int rand_num = std::rand() % 10000;
		if(rand_num < cur_may_be_weight)
		{
			must_show_set.insert(*iter);
			rand_num = std::rand() % pro_sum;
			MSG_USER("may be no group_count, cur rand_num:%d", rand_num);
			//show_limit_pro_map索引为个数，0代表1个
			for (IntMap::const_iterator limit_iter = show_limit_pro_map.begin(); limit_iter != show_limit_pro_map.end(); ++limit_iter)
			{
				MSG_USER("may be cur limit:%d, rand_num:%d", limit_iter->second, rand_num);
				if (limit_iter->second >= rand_num)
				{
					group_item_count_map[*iter] = limit_iter->first + 1;
					break;
				}
			}
		}

		int max_rand_times = act_detail->group_limit_list_[*iter - 1];
		if (group_item_count_map[*iter] > max_rand_times)
			group_item_count_map[*iter] = max_rand_times;
		sum_group_item_count += group_item_count_map[*iter];
	}


	//如果总个数小于要求的个数，由补漏组提供
	if (sum_group_item_count < slot_num)
	{
		//这里修改了，原本是 int sub = sum_group_item_count < slot_num;
		int sub = slot_num - sum_group_item_count;
		//add
		sum_group_item_count = slot_num;
		group_item_count_map[group_count] += sub;
	}

	//最终插入的格子id
	IntSet last_slot_map;
	if (must_show_set.size() > 0)
	{
		//符合条件的组的总权重
		int total_weight_by_group = 0;
		int base_weight = 0;
		//对应物品的权重
		IntMap slot_weight;
		IntMap slot_base_weight;
		for (IntSet::const_iterator iter = must_show_set.begin(); iter != must_show_set.end(); ++iter)
		{
			//如果是补漏组，跳出
			JUDGE_BREAK(*iter != group_count);
			//获取该组的所有物品
			MayActDetail::SlotInfoVec* slot_info_vec = MAY_ACTIVITY_SYS ->fetch_slot_map_by_group(act_detail, *iter);
			JUDGE_RETURN(slot_info_vec != NULL, NULL);

			//获取该组的总权重
			total_weight_by_group += MAY_ACTIVITY_SYS->fetch_slot_map_weight_by_group(act_detail, *iter);
			JUDGE_RETURN(total_weight_by_group >= 0, NULL);

			for (size_t i = 0; i < slot_info_vec->size(); ++i)
			{
				MayActDetail::SlotInfo *slot_info = &(*slot_info_vec)[i];
				base_weight += slot_info->the_weight_;
				slot_weight[slot_info->slot_id_] = base_weight;
				slot_base_weight[slot_info->slot_id_] = slot_info->the_weight_;
			}

			int cur_group_count = group_item_count_map[*iter];
			MSG_USER("group_id:%d, cur_group_count:%d", *iter, cur_group_count);

			IntMap::iterator weight_iter;
			for (int i = 0; i < cur_group_count; ++i)
			{
				if(total_weight_by_group <= 0)
					break;
				int rand_num = std::rand() % total_weight_by_group;
				weight_iter = slot_weight.begin();
				//这里每删除一个slot_weight，重新计算新的权重比例
				for (; weight_iter != slot_weight.end(); )
				{
					//MSG_USER("weight_iter:%d, rand_num:%d ", weight_iter->second, rand_num);
					if (weight_iter->second >= rand_num)
					{
						last_slot_map.insert(weight_iter->first);
						//MSG_USER("erase slot id:%d, weight:%d\n", weight_iter->first, weight_iter->second);
						int temp_weight = weight_iter->second;
						slot_weight.erase(weight_iter++);
						IntMap::iterator next_iter = weight_iter;
						//IntMap::iterator next_iter = slot_weight.erase(weight_iter);

						total_weight_by_group -= slot_base_weight[weight_iter->first];
						if (next_iter == slot_weight.end())
							break;
						IntMap::iterator pre_iter = next_iter;
						if(pre_iter != slot_weight.begin())
						{
							pre_iter--;
						}

						if (pre_iter == slot_weight.end())
							break;
						for (; next_iter != slot_weight.end(); ++next_iter)
						{
							int next_temp_weight = next_iter->second;
							if(next_iter == slot_weight.begin())
							{
								next_iter->second = next_iter->second - temp_weight;
								//MSG_USER("move value:: next_iter:%d, pre_iter:%d\n", next_iter->second, pre_iter->second);
							}
							else
							{
								next_iter->second = next_iter->second - temp_weight + pre_iter->second;
								//MSG_USER("move value:: next_iter:%d, pre_iter:%d\n", next_iter->second, pre_iter->second);
								++pre_iter;
							}
							temp_weight = next_temp_weight;

						}
						break;
					}
					else
						++weight_iter;
				}
			}
		}
	}

	//这里当随机出来的slotid不足需要随机的物品个数，就在最后一组添加个数
	if ((int)last_slot_map.size() < slot_num)
	{
		//剩余需要插入个数
		int rest = slot_num - last_slot_map.size();

		//获取该组的所有物品
		MayActDetail::SlotInfoVec* slot_info_vec = MAY_ACTIVITY_SYS->fetch_slot_map_by_group(act_detail, group_count);
		JUDGE_RETURN(slot_info_vec != NULL, NULL);

		//获取该组的总权重
		int total_weight_by_group = MAY_ACTIVITY_SYS->fetch_slot_map_weight_by_group(act_detail, group_count);
		JUDGE_RETURN(total_weight_by_group > 0, NULL);

		int base_weight = 0;
		IntMap slot_weight;
		IntMap slot_base_weight;
		for (size_t i = 0; i < slot_info_vec->size(); ++i)
		{
			MayActDetail::SlotInfo *slot_info = &(*slot_info_vec)[i];
			if(slot_info == NULL)
				continue;
			base_weight += slot_info->the_weight_;
			slot_weight[slot_info->slot_id_] = base_weight;
			slot_base_weight[slot_info->slot_id_] = slot_info->the_weight_;
			//MSG_USER("cur insert slot_id:%d, slot_weight:%d", slot_info->slot_id_, base_weight);
		}

		MayActDetail::SlotInfoVec temp(*slot_info_vec);
		IntMap::iterator weight_iter;
		for (int i = 0; i < rest; ++i)
		{
			//MSG_USER("group_count,total_weight_by_group:%d, after", total_weight_by_group);
			if(total_weight_by_group <= 0)
				break;
			int rand_num = std::rand() % total_weight_by_group;
			//MSG_USER("last insert rand_num:%d", rand_num);
			weight_iter = slot_weight.begin();

			for (; weight_iter != slot_weight.end(); )
			{
				if (weight_iter->second >= rand_num)
				{
					last_slot_map.insert(weight_iter->first);
					//MSG_USER("erase slot id:%d, weight:%d\n", weight_iter->first, weight_iter->second);
					int temp_weight = weight_iter->second;
					slot_weight.erase(weight_iter++);
					IntMap::iterator next_iter = weight_iter;
					total_weight_by_group -= slot_base_weight[weight_iter->first];
					if (next_iter == slot_weight.end())
						break;
					IntMap::iterator pre_iter = next_iter;
					if(pre_iter != slot_weight.begin())
						pre_iter--;

					if (pre_iter == slot_weight.end())
						break;
					for (; next_iter != slot_weight.end(); ++next_iter)
					{
						int next_temp_weight = next_iter->second;
						if(next_iter == slot_weight.begin())
						{
							next_iter->second = next_iter->second - temp_weight;
						}
						else
						{
							next_iter->second = next_iter->second - temp_weight + pre_iter->second;
							++pre_iter;
						}
						temp_weight = next_temp_weight;

					}
					break;
				}
				else
					++weight_iter;
			}
		}

		//补充随机后，还是不足够的问题，已询问策划
		int rand_count = 50;
		while (rand_count)
		{
			if((int)last_slot_map.size() == slot_num)
				break;
			int index = std::rand() % temp.size();
			if (last_slot_map.count(temp[index].slot_id_)>0)
				continue;
			last_slot_map.insert(temp[index].slot_id_);
			MSG_USER("while rand_count--- slot_id:%d", temp[index].slot_id_);
			rand_count--;
		}
	}

	//刷新
	int i = 0;
	for (IntSet::iterator it = last_slot_map.begin(); it != last_slot_map.end(); ++it, ++i)
	{
		int slot_id = *it;
		MayActDetail::SlotInfo* slot_info = MAY_ACTIVITY_SYS->fetch_slot_info(act_detail, slot_id);
		if(slot_info == NULL)
			continue;
		MayActDetail::SlotInfo &shop_slot_info = player_detail.shop_slot_map_[i].slot_info_;
		shop_slot_info.slot_id_ = slot_info->slot_id_;
		shop_slot_info.group_id_ = slot_info->group_id_;
		shop_slot_info.is_precious_ = slot_info->is_precious_;
		shop_slot_info.now_cost_ =  slot_info->now_cost_;
		shop_slot_info.item_obj_.id_ = slot_info->item_obj_.id_;
		shop_slot_info.item_obj_.bind_ = slot_info->item_obj_.bind_;
		shop_slot_info.item_obj_.amount_ = slot_info->item_obj_.amount_;
		player_detail.shop_slot_map_[i].is_buy_ = 0;
		MSG_USER("item:%d, group_id:%d", shop_slot_info.slot_id_, shop_slot_info.group_id_);
	}

	for (int i = 0; i < group_count; ++i)
	{
		if (group_item_count_map[i + 1] > 0)
		{
			player_detail.group_refresh_times_map_[i] = 0;
		}
		else
		{
			player_detail.group_refresh_times_map_[i] ++;
		}
	}
	return 0;
}

void LogicMayActivityer::create_rand_role_view(MayActDetail::ActInfo& act_info)
{
	MayActDetail::RunInfo &my_run = act_info.run_role_map_[this->role_id()];
	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info.act_index_);

	int type = 0;
	for (ThreeObjVec::iterator iter = act_info.rand_vec.begin();
			iter != act_info.rand_vec.end(); ++iter)
	{
		int amount = m_act_item.fetch_role_size(type);
		JUDGE_CONTINUE(amount < iter->id_);

		int left_amount = iter->id_ - amount;
		for (MayActDetail::RunRoleMap::iterator it = act_info.run_role_map_.begin();
				it != act_info.run_role_map_.end(); ++it)
		{
			MayActDetail::RunInfo &player_run = it->second;
			JUDGE_CONTINUE(player_run.location_ > 0);
			JUDGE_CONTINUE(player_run.location_ >= my_run.location_ + iter->tick_
					&& player_run.location_ <= my_run.location_ + iter->value_);
			JUDGE_CONTINUE(m_act_item.role_map_.count(it->first) == 0);

			m_act_item.role_map_[it->first] = type;
			JUDGE_BREAK((--left_amount) > 0);
		}
		++type;
	}
}

int LogicMayActivityer::fetch_labour_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto50100252*, request, -1);
	int act_id = request->index();
	MayActDetail::ActInfo *act_detail = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::LABOR_HONOUR, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_detail != NULL && act_id == act_detail->act_index_, 0);

	MayActivityer::ActTypeItem& player_detail = this->find_may_act_item(act_id);
	request->set_sub_value(player_detail.sub_value_);

	//获取记录的任务列表
	for(int i = GameEnum::LABOUR_TASK_BEGIN; i < GameEnum::LABOUR_TASK_END; ++i)
	{
		CornucopiaTask *task = this->fetch_labour_task(i);
		if(task == NULL)
		{
			MSG_USER("task_id:%d is NULL", i);
			continue;
		}

		if(task->total_times <= 0)
		{
			MSG_USER("fetch_labour_info: task is NULL, total_times:%d", task->total_times);
			continue;
		}

		int is_open = this->is_labour_task_open_time(i);
//		MSG_USER("task_id:%d, is_open:%d, completion_times:%d, total_times:%d",
//				task->task_id_, is_open, task->completion_times_,task->total_times);
		if(!is_open)
			continue;
		PActTastList *task_list = request->add_task_info();
		task_list->set_task_id(task->task_id_);
		task_list->set_total_num(task->total_times);
		task_list->set_left_num(task->completion_times_);
//		if(task->task_name_.c_str() == NULL)
//			task->task_name_ = ;
//		string task_name = CONFIG_INSTANCE->labour_act_info(i)["name"].asString();
//		task_list->set_task_name(task_name);
	}

	return 0;
}

CornucopiaTask* LogicMayActivityer::fetch_labour_task(int task_id)
{
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::LABOR_HONOUR, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_info != NULL, NULL);

	//任务id必须在配置的范围内
	JUDGE_RETURN(task_id < GameEnum::LABOUR_TASK_END && task_id >= GameEnum::LABOUR_TASK_BEGIN, NULL);

	MayActivityer::ActTypeItem& player_detail = this->find_may_act_item(act_info->act_index_);
	CornucopiaTaskMap &task_map = player_detail.labour_task_map_;
	int reward_size = act_info->reward_set_.size();
	for(int i = 0; i < reward_size; ++i)
	{
		int reward_task_id = act_info->reward_set_[i].task_id_;
		//初始化任务列表
		if(task_map.count(reward_task_id) == 0)
		{
			CornucopiaTask task;
			task.task_id_ = reward_task_id;
			task.completion_times_ = 0;
			const Json::Value &cond_value = CONFIG_INSTANCE->labour_act_info(task.task_id_)["condition"];
			if(cond_value == Json::Value::null)
			{
				MSG_USER("No find task:%d", task.task_id_);
				continue;
			}

			if(cond_value.isArray())
			{
				int index = 0;
				task.total_times = cond_value[index].asInt();
			}
			else
				task.total_times = cond_value.asInt();
			//task.task_name_ = CONFIG_INSTANCE->labour_act_info(i)["name"].asString();
			MSG_USER("task_name:%s", CONFIG_INSTANCE->labour_act_info(task.task_id_)["name"].asCString());
			task_map[task.task_id_] = task;
		}
	}
	return &task_map[task_id];
}

int LogicMayActivityer::update_labour_activity_value(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403201*, request, RETURN_MAY_ACTIVITY_INFO);
	int task_id = request->task_id();
	int count = request->task_finish_count();
	int type = request->type();
	update_labour_activity_value(task_id, count, type);
	return 0;
}

void LogicMayActivityer::update_labour_activity_value(int task_id, int value, int type)
{
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::LABOR_HONOUR, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_info != NULL, );

	//任务id必须在配置的范围内
	JUDGE_RETURN(task_id < GameEnum::LABOUR_TASK_END && task_id >= GameEnum::LABOUR_TASK_BEGIN, );
	CornucopiaTask& task = *fetch_labour_task(task_id);
	if(task.total_times <= 0)
	{
		MSG_USER("update_labour_activity_value: task is NULL, total_times:%d", task.total_times);
		return ;
	}

	if(task.completion_times_ < task.total_times)
	{
		if(value > task.total_times)
			value = task.total_times;
		if(type)
			task.completion_times_ = value;
		else
			task.completion_times_ += value;
		if(task.completion_times_ > task.total_times)
			task.completion_times_ = task.total_times;
	}

	if(task.completion_times_ == task.total_times)
	{
		for(uint i = 0; i < act_info->reward_set_.size(); ++i)
		{
			if(act_info->reward_set_[i].task_id_ == task.task_id_)
			{
				this->update_may_act_one_value_by_index(act_info->first_type_, act_info->second_type_, i, task.completion_times_);
				break;
			}
		}
	}

	MayActivityer::ActTypeItem& act_type_item = this->find_may_act_item(act_info->act_index_);
	//检测是否完成全部任务，累计n次邮件发放称号，限制一天只累计一次
	if(is_all_labour_task_finish() && !act_type_item.sub_value_)
	{
		LogicRoleDetail &player_detail = this->logic_player()->role_detail();
		player_detail.cumulative_times_++;
		act_type_item.sub_value_ = true;
		MSG_USER("cumulative finish all task times:%d", player_detail.cumulative_times_);
		if(player_detail.cumulative_times_ == act_info->start_cond_)
		{
			MailInformation* mail_info = GameCommon::create_sys_mail(act_info->mail_id_);
			if(mail_info == NULL)
			{
				MSG_USER("LogicMayActivityer fetch_labour_info:Create mail obj error");
			}
			else
			{
				::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
						mail_info->mail_content_.c_str());
				mail_info->add_goods(act_info->send_reward_, 1, true);
				GameCommon::request_save_mail_content(this->role_id(), mail_info);
			}
		}
	}

	this->fetch_may_activity_info(act_info->act_index_);
}

int LogicMayActivityer::is_labour_task_open_time(int task_id)
{
	int parsed_days = MAY_ACTIVITY_SYS->cur_may_day();
	const Json::Value& value = CONFIG_INSTANCE->labour_act_info(task_id);
	if(value == Json::Value::null)
	{
		MSG_USER("No find task:%d", task_id);
		return 0;
	}
	const Json::Value& day_array = value["open_day"];
	bool is_array = day_array.isArray();
	bool is_open_day = false;
	if(is_array)
	{
		Json::Value::const_iterator iter = day_array.begin();
		for( ; iter != day_array.end(); ++iter)
		{
			if((*iter).asInt() == parsed_days)
				is_open_day = true;
		}
	}
	else
	{
		if(day_array.asInt() == parsed_days)
			is_open_day = true;
	}

	return is_open_day;
}

int LogicMayActivityer::is_all_labour_task_finish()
{
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::LABOR_HONOUR, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_info != NULL, NULL);
	for(int task_id = GameEnum::LABOUR_TASK_BEGIN; task_id < GameEnum::LABOUR_TASK_END; ++task_id)
	{
		CornucopiaTask *p_task = this->fetch_labour_task(task_id);
		JUDGE_CONTINUE(p_task != NULL);
		if(p_task->total_times <= 0)
		{
			MSG_USER("is_all_labour_task_finish: task is NULL, total_times:%d", p_task->total_times);
			continue;
		}
		if(p_task->completion_times_ < p_task->total_times)
		{
			return false;
		}
	}
	return true;
}

void LogicMayActivityer::update_may_act_one_value_by_index(int first_type, int day, int index, int value)
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_item_by_day(first_type, day);
	JUDGE_RETURN(act_info != NULL, ;);

	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info->act_index_);

	int flag = false;

	int reward_set_size = act_info->reward_set_.size();
	JUDGE_RETURN(index < reward_set_size, ;);

	MayActDetail::ActReward &act_reward = act_info->reward_set_[index];
	JUDGE_RETURN(value == act_reward.cond_[0], ;);

	m_act_item.act_item_map_[index].arrive_ = true;
	if(act_reward.drawed_map_.count(this->role_id()) == 0)
	{
		act_reward.drawed_map_[this->role_id()] = 0;
		flag = true;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_may_activity_update(act_info);
}

int LogicMayActivityer::fetch_change_buy_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100271 *, request, -1);
	int act_id = request->act_id();
	int index = request->index();

	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(act_id);
	JUDGE_RETURN(act_info != NULL, -1);
	MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
	JUDGE_RETURN((int)reward_set.size() > index, -2);
	MayActDetail::ActReward &act_reward = reward_set[index];
	int ret = -1;
	//判断活动期间兑换次数
	if(act_reward.act_change_times_ != 0)
	{
		if(act_reward.act_drawed_map_[this->role_id()] < act_reward.act_change_times_)
		{
			ret = 0;
		}
		else
			ret = ERROR_ACT_CHANGE_MAX;
	}
	//每天兑换
	else if(act_reward.every_change_times_ != 0)
	{
		if(act_reward.every_change_times_ > act_reward.drawed_map_[this->role_id()])
		{
			ret = 0;
		}
		else
			ret = ERROR_EVERY_DAY_CHANGE_MAX;
	}
	else
		ret = 0;

	if(ret == 0)
	{
		Proto31403202 inner;
		inner.set_act_id(act_id);
		inner.set_index(index);
		for(uint i = 0; i < act_reward.change_item_.size(); ++i)
		{
			ItemObj &obj = act_reward.change_item_[i];
			inner.add_item_amount(obj.amount_);
			inner.add_item_id(obj.id_);
			inner.add_item_bind(obj.bind_);
		}
		return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
	}
	else
	{
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ADD_COUPLE_TICK, ret);
	}
	return 0;
}

int LogicMayActivityer::fetch_change_buy_end(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31403202 *, request, -1);
	int act_id = request->act_id();
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(act_id);

	//活动结束，清除道具这里会返回非0
	if(request->type() == 1 || act_info == NULL)
	{
		LogicRoleDetail &role_detail = this->logic_player()->role_detail();
		act_info = MAY_ACTIVITY_SYS->find_act_info(70901);
		role_detail.last_act_type_ = act_info->act_type_;
		role_detail.last_act_end_time_ = MAY_ACTIVITY_SYS->end_date_;
		return 0;
	}

	JUDGE_RETURN(act_info != NULL, -1);

	int ret = request->ret();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_CHANGE_REWARD, ERROR_TALISMAN_NOT_FIND_ACTIVE);

	int index = request->index();
	MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
	JUDGE_RETURN((int)reward_set.size() > index, -2);
	MayActDetail::ActReward &act_reward = reward_set[index];

	SerialObj obj(ADD_FROM_CHANGE, act_id);
	this->request_add_reward(act_reward.reward_id_, obj);
	act_reward.act_drawed_map_[this->role_id()]++;
	act_reward.drawed_map_[this->role_id()]++;

	MSG_DEBUG("change_count:%d, every_day_count:%d", act_reward.act_drawed_map_[this->role_id()], act_reward.drawed_map_[this->role_id()]);
	Proto50100271 respond;
	respond.set_act_id(act_id);
	respond.set_index(index);
	respond.set_ret(ret);
	return this->respond_to_client(RETURN_CHANGE_REWARD, &respond);
}

void LogicMayActivityer::reset_every_day_by_change(int type)
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::CHANGE_ITEM, MAY_ACTIVITY_SYS->cur_may_day());

	LogicRoleDetail &role_detail = this->logic_player()->role_detail();
	if(act_info == NULL)
	{
		act_info = MAY_ACTIVITY_SYS->find_act_info(70901);
		role_detail.act_data_reset_flag_ = false;
	}
	JUDGE_RETURN(act_info != NULL, );
	MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
	Int64 cur_time = time(0);
//	Int64 end_time = MAY_ACTIVITY_SYS->end_date_;

	//当前时间大于活动结束时间，并且下个活动还没开启和刷新时间还没有到达的时候
	if(act_info->act_type_ != role_detail.last_act_type_ || cur_time > role_detail.last_act_end_time_)
	{
		//最后一天，清空当前已有的道具
		Proto31403202 inner;
		inner.set_type(1);
		inner.set_act_id(70901);
		for(uint i = 0; i < reward_set.size(); ++i)
		{
			MayActDetail::ActReward &reward = reward_set[i];
			ItemObjVec &item_vec = reward.change_item_;
			for(uint j = 0; j < item_vec.size(); ++j)
			{
				inner.add_item_id(item_vec[i].id_);
			}
		}
		if(reward_set.size() > 0)
		{
			LOGIC_MONITOR->dispatch_to_scene(this, &inner);
		}
	}

	for(uint i = 0; i < reward_set.size(); ++i)
	{
		MayActDetail::ActReward &reward = reward_set[i];
		if(reward.every_change_times_ != 0)
		{
			reward.drawed_map_.clear();
		}
	}
}

void LogicMayActivityer::test_clear_data_by_change()
{
	this->reset_every_day_by_change();
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::CHANGE_ITEM, MAY_ACTIVITY_SYS->cur_may_day());
	MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
	for(uint i = 0; i < reward_set.size(); ++i)
	{
		reward_set[i].act_drawed_map_.clear();
	}
}

void LogicMayActivityer::refresh_daily_login_data()
{
	int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::DAILY_LOGIN, cur_passed_day);
	JUDGE_RETURN(act_info != NULL, );

	LogicPlayer *player = this->logic_player();
    int offline_day = player->online().offline_day_from_logout();
    LogicRoleDetail& role_detail = player->role_detail();
    if (offline_day > 1)
    {//非连续登陆
    	role_detail.continuity_login_day_ = 1;
    }
    else if (offline_day == 1)
    {//连续登陆
    	if(cur_passed_day == 1)
    	{
    		role_detail.continuity_login_day_ = 1;
    	}
    	else
    		role_detail.continuity_login_day_ += 1;
    }
    else
    {
    	if (role_detail.continuity_login_day_ < 1)
    	{
    		role_detail.continuity_login_day_ = 1;
    	}
    	else
    	{
        	if(cur_passed_day == 1)
        	{
        		role_detail.continuity_login_day_ = 1;
        	}
        	else
        		role_detail.continuity_login_day_ += 1;
    	}
    }
}

void LogicMayActivityer::send_run_direct_award(int before, int after, int base)
{
	JUDGE_RETURN(after > before && base > 0, ;);

	for (int i = before + 1; i <= after; i = i + base)
	{
		const Json::Value &extra = CONFIG_INSTANCE->daily_run_extra(i);
		JUDGE_CONTINUE(extra.empty() == false);

		int direct_reward = extra["direct_reward"].asInt();
		JUDGE_CONTINUE(direct_reward > 0);

		SerialObj obj(ADD_FROM_DAILY_RUN_DIRECT, i);
		this->request_add_reward(direct_reward, obj);
	}
}

void LogicMayActivityer::reset_everyday()
{
	IntMap& act_list = MAY_ACTIVITY_SYS->fetch_act_list();
	for (IntMap::iterator iter = act_list.begin(); iter != act_list.end(); ++iter)
	{
		MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(
				iter->first, this->role_detail().__agent_code);
		JUDGE_CONTINUE(act_info != NULL);

		//取消昨日红点
		if (act_info->first_type_ != MayActDetail::DAILY_RUN && act_info->red_point_ > 0)
		{
			this->inner_notify_assist_event(act_info->red_point_, 0);
		}

		JUDGE_CONTINUE(act_info->day_reset_ == true);

		MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info->act_index_);
		m_act_item.reset_every();
	}

	this->refresh_daily_login_data();
	this->daily_login_refresh_day();

	MSG_USER("role_detail.last_act_type: %d", this->role_detail().last_act_type_);
	this->reset_every_day_by_change();
}

void LogicMayActivityer::daily_login_refresh_day()
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::DAILY_LOGIN, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_info != NULL, ;);

	MayActivityer::ActTypeItem& m_act_t_item = this->find_may_act_item(act_info->act_index_);

	int cur_index = 0;
	MayActivityer::ActItem& m_act_item_0 = m_act_t_item.act_item_map_[cur_index];
	if (m_act_item_0.arrive_ == 0)
		m_act_item_0.arrive_ += 1;
	if (m_act_item_0.arrive_ > m_act_item_0.drawed_)
		this->notify_may_activity_update(act_info);
	MSG_USER("m_act_item.act_item_map_0.arrive_:%d",m_act_item_0.arrive_);

	cur_index += 1;
	MayActivityer::ActItem& m_act_item_1 = m_act_t_item.act_item_map_[cur_index];
	LongVec &cond_vec = act_info->reward_set_[cur_index].cond_;
	int cond_size = cond_vec.size();
	LogicRoleDetail& role_detail = this->role_detail();
	for (int i = 0; i < cond_size; ++i)
	{
		int cond_day = cond_vec[i];
		JUDGE_CONTINUE(role_detail.continuity_login_day_ >= cond_day);
		JUDGE_CONTINUE(m_act_item_1.arrive_map_[i] != true);
		m_act_item_1.arrive_ += 1;
		m_act_item_1.arrive_map_[i] = true;
	}

	if (m_act_item_1.arrive_ > m_act_item_1.drawed_)
		this->notify_may_activity_update(act_info);
	MSG_USER("m_act_item.act_item_map_1.arrive_:%d", m_act_item_1.arrive_);
}

int LogicMayActivityer::fetch_cur_time_open_state(int activity_id)
{
	int open_state = MAY_ACTIVITY_SYS->is_act_open_time_by_id(activity_id);
	MayActDetail::ActInfo* info = MAY_ACTIVITY_SYS->find_act_info(activity_id);
	if(info->cur_time_act_open_state_ != open_state)
	{
		info->cur_time_act_open_state_ = open_state;
		if(open_state)
		{
			//活动开启准备
			MAY_ACTIVITY_SYS->rand_all_red_packet();
			this->init_red_act_info_list();
			GameCommon::announce(info->reward_set_[0].show_id_);
		}
		else
		{
			//活动关闭后
			this->clear_red_act_info();
		}
	}

    this->inner_notify_assist_event(info->red_point_, open_state);
	return open_state;
}

int LogicMayActivityer::fetch_may_activity_list()
{
	Proto50100251 respond;

	this->refresh_may_act();

	IntMap& act_list = MAY_ACTIVITY_SYS->fetch_act_list();
	for (IntMap::iterator iter = act_list.begin(); iter != act_list.end(); ++iter)
	{
		respond.add_act_list(iter->first);
	}

	respond.set_type(this->may_detial_.act_type_);
	FINER_PROCESS_RETURN(RETURN_MAY_ACTIVITY_INFO, &respond);
}

int LogicMayActivityer::fetch_may_activity(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100252*, request, RETURN_ONE_MAY_ACTIVITY);
	return this->fetch_may_activity_info(request->index());
}

int LogicMayActivityer::fetch_may_activity_info(int index)
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(
			index, this->role_detail().__agent_code);
	JUDGE_RETURN(act_info != NULL, 0);

	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info->act_index_);

	Proto50100252 respond;
	respond.set_index(act_info->act_index_);
	respond.set_left_tick(MAY_ACTIVITY_SYS->act_left_tick());
	respond.set_night_tick(GameCommon::next_day());
	respond.set_name(act_info->act_name_);
	respond.set_title(act_info->act_title_);
	respond.set_first_type(act_info->first_type_);
	respond.set_second_type(act_info->second_type_);
	respond.set_start_cond(act_info->start_cond_);

	this->serialize_sub_value(&respond, index);
	this->check_rand_role_view(*act_info);

	for (LongMap::iterator iter = m_act_item.role_map_.begin();
			iter != m_act_item.role_map_.end(); ++iter)
	{
		MayActDetail::RunInfo &player_run = act_info->run_role_map_[iter->first];

		ProtoRunRole *run_role = respond.add_role_info();
		run_role->set_role_id(player_run.role_id_);
		run_role->set_role_name(player_run.name_);
		run_role->set_role_sex(player_run.sex_);
		run_role->set_value(player_run.location_);
	}

	//必须刷新当前时间来判断活动开启
	MAY_ACTIVITY_SYS->refresh_act_open_time(act_info->act_index_);
	for(MayActDetail::LimitTimeVec::iterator iter = act_info->limit_time_vec_.begin();
			iter != act_info->limit_time_vec_.end(); ++iter)
	{
		ActOpenLimitTime *limit_time = respond.add_open_time();
		limit_time->set_open_time(iter->down_time_);
		limit_time->set_open_state(iter->state_);
	}

	this->fetch_grab_red_packet_info(&respond);
	this->fetch_fashion_info(&respond);
	this->fetch_labour_info(&respond);

	ProtoLimitValue *limit_value = respond.mutable_limit_value();
	act_info->serialize_limit_cond(limit_value);

	int cur_index = 0;
	for (MayActDetail::ActRewardSet::iterator iter = act_info->reward_set_.begin();
			iter != act_info->reward_set_.end(); ++iter)
	{
		MayRewardInfo *reward_info = respond.add_reward_info();
		reward_info->set_index(cur_index);
		reward_info->set_name(iter->name_);
		reward_info->set_content(iter->content_);
		reward_info->set_pre_cost(iter->pre_cost_);
		reward_info->set_now_cost(iter->now_cost_);
		reward_info->set_times(iter->times_);
		reward_info->set_reward_id(iter->reward_id_);

		if(iter->act_change_times_ != 0)
		{
			if(iter->act_drawed_map_[this->role_id()] < iter->act_change_times_)
			{
				//可兑换
				reward_info->set_change_state(iter->act_change_times_ - iter->act_drawed_map_[this->role_id()]);
			}
			else
			{
				reward_info->set_change_state(-1);
			}
		}
		else if(iter->every_change_times_ != 0)
		{
			if(iter->drawed_map_[this->role_id()] < iter->every_change_times_)
			{
				reward_info->set_change_state(iter->every_change_times_ - iter->drawed_map_[this->role_id()]);
			}
			else
			{
				reward_info->set_change_state(-1);
			}
		}
		//每天限制和活动期间限制没有配置，无限兑换
		else
		{
			reward_info->set_change_state(-2);
		}

		ItemObjVec &item_vec = iter->change_item_;
		for(uint i = 0; i < item_vec.size(); ++i)
		{
			ItemObj &item_obj= item_vec[i];
			ProtoItemId *item_id = reward_info->add_change_item();
			item_id->set_id(item_obj.id_);
			item_id->set_amount(item_obj.amount_);
			item_id->set_bind(item_obj.bind_);
		}

		// 是否领取
		int draw_flag = this->has_may_activity_reward(*act_info, cur_index);
		//MSG_USER("index:%d, reward_cond:%d",cur_index, draw_flag);
		reward_info->set_draw_flag(draw_flag);

		//奖励领取条件
		for(uint i = 0; i < iter->cond_.size(); ++i)
		{
			reward_info->add_cond(iter->cond_[i]);
		}

		reward_info->set_arrive(m_act_item.act_item_map_[cur_index].arrive_);
		reward_info->set_drawed(m_act_item.act_item_map_[cur_index].drawed_);

		++cur_index;
	}

	LogicRoleDetail& role_detail = this->role_detail();
	respond.set_continuity_login_day(role_detail.continuity_login_day_);
	respond.set_daily_cumulative_recharge(role_detail.today_recharge_gold_);

	//MSG_USER("Proto50100252: %s", respond.Utf8DebugString().c_str());

	FINER_PROCESS_RETURN(RETURN_ONE_MAY_ACTIVITY, &respond);
}

int LogicMayActivityer::draw_may_activity_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100253*, request, RETURN_FETCH_MAY_ACT_REWARD);

	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(
			request->index(), this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_FETCH_MAY_ACT_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	uint reward_index = request->reward_index();
	CONDITION_NOTIFY_RETURN(reward_index < act_info->reward_set_.size(),
			RETURN_FETCH_MAY_ACT_REWARD, ERROR_CLIENT_OPERATE);

	//判断是否有奖励可以领取
	int reward_flag = this->has_may_activity_reward(*act_info, reward_index);
	CONDITION_NOTIFY_RETURN(reward_flag == BackSetActDetail::UNDRAW,
			RETURN_FETCH_MAY_ACT_REWARD, ERROR_ACT_NO_REWARD);

	int ret = this->draw_may_activity_reward(*act_info, reward_index, 0);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FETCH_MAY_ACT_REWARD, ret);

	//取消红点
	if (this->has_may_activity_reward(*act_info) == false)
	{
		this->inner_notify_assist_event(act_info->red_point_, 0);
	}

	this->fetch_may_activity_info(request->index());

	Proto50100253 respond;
	respond.set_type(act_info->first_type_);
	FINER_PROCESS_RETURN(RETURN_FETCH_MAY_ACT_REWARD, &respond);
}

int LogicMayActivityer::draw_may_activity_reward(MayActDetail::ActInfo& act_info,
		uint index, int num)
{
	JUDGE_RETURN(act_info.reward_set_.size() > index, ERROR_CLIENT_OPERATE);
	MayActDetail::ActReward &act_reward = act_info.reward_set_[index];

	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info.act_index_);
	JUDGE_RETURN(m_act_item.act_item_map_[index].have_reward() == true, ERROR_ACT_REWARD_ERROR_COUNT);

	act_reward.drawed_map_[this->role_id()] = true;
	m_act_item.act_item_map_[index].drawed_ += 1;

	this->may_activity_announce(act_info.act_index_, index);

	SerialObj obj(ADD_FROM_MAY_ACTIVITY, act_info.first_type_, act_info.second_type_);
	return this->request_add_reward(act_reward.reward_id_, obj);
}

int LogicMayActivityer::inner_act_buy_operate(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);

	int type = request->type();
	switch (type)
	{
		case MayActDetail::DAILY_BUY:
		{
			this->request_may_activity_buy_done(msg);
			break;
		}
		case MayActDetail::COUPLE_HEART:
		{
			this->request_add_couple_tick_end(msg);
			break;
		}
		case MayActDetail::DAILY_RUN:
		{
			if (request->money() > 0)
			{
				this->request_daily_run_buy_end(msg);
			}
			else if (request->amount() > 0)
			{
				this->request_start_run_end(msg);
			}
			else
			{
				this->request_daily_run_jump_end(msg);
			}
			break;
		}
		case MayActDetail::NICE_FASHION:
		{
			this->fetch_fashion_buy_End(msg);
			break;
		}
		default:
			break;
	}

	return 0;
}

int LogicMayActivityer::request_add_couple_tick_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100260*, request, RETURN_ADD_COUPLE_TICK);

	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(
			request->index(), this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_ADD_COUPLE_TICK, ERROR_NO_FUN_ACTIVITY);

	LogicPlayer *player = this->logic_player();
	Int64 wedding_id = player->wedding_id();
	CONDITION_NOTIFY_RETURN(wedding_id > 0, RETURN_ADD_COUPLE_TICK, ERROR_NO_WEDDING);
	CONDITION_NOTIFY_RETURN(act_info->limit_cond_.size() == 2, RETURN_ADD_COUPLE_TICK,
			ERROR_CONFIG_ERROR);

	int min = request->minute();
	MayActDetail::CoupleInfo &couple_info = act_info->couple_info_map_[wedding_id];
	couple_info.wedding_id_ = wedding_id;
	CONDITION_NOTIFY_RETURN(couple_info.buy_tick_ / 60 + min <= act_info->limit_cond_[1],
			RETURN_ADD_COUPLE_TICK, ERROR_CHANGE_TICK_UPPER_LIMIT);

	int need_gold = min * act_info->limit_cond_[0];
	CONDITION_NOTIFY_RETURN(need_gold > 0, RETURN_ADD_COUPLE_TICK, ERROR_SERVER_INNER);

	Proto31400055 inner;
	inner.set_type(act_info->first_type_);
	inner.set_first_index(request->index());
	inner.set_money(need_gold);
	inner.set_amount(min);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicMayActivityer::request_add_couple_tick_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);

	CONDITION_NOTIFY_RETURN(request->ret() == 0, RETURN_ADD_COUPLE_TICK, request->ret());

	int first_index = request->first_index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			first_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL,RETURN_ADD_COUPLE_TICK, ERROR_NO_FUN_ACTIVITY);

	LogicPlayer *player = this->logic_player();
	Int64 wedding_id = player->wedding_id();
	CONDITION_NOTIFY_RETURN(wedding_id > 0, RETURN_ADD_COUPLE_TICK, ERROR_NO_WEDDING);

	MayActDetail::CoupleInfo &couple_info = act_info->couple_info_map_[wedding_id];
	couple_info.buy_tick_ += request->amount() * 60;

	int total_min = (couple_info.online_tick_ + couple_info.buy_tick_) / 60;
	this->update_may_act_one_value(act_info->first_type_, act_info->second_type_, total_min);

	LogicPlayer *partner = player->fetch_wedding_partner();
	if (partner != NULL)
	{
		partner->update_may_act_one_value(act_info->first_type_, act_info->second_type_, total_min);
		partner->fetch_may_activity_info(first_index);
	}

	this->fetch_may_activity_info(first_index);

	Proto50100260 respond;
	respond.set_index(first_index);
	FINER_PROCESS_RETURN(RETURN_ADD_COUPLE_TICK, &respond);
}

int LogicMayActivityer::request_may_activity_buy_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100254*, request, RETURN_MAY_ACT_BUY);

	int act_index = request->index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			act_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_MAY_ACT_BUY, ERROR_NO_FUN_ACTIVITY);

	uint buy_index = request->buy_index();
	int amount = request->amount();
	CONDITION_NOTIFY_RETURN(buy_index < act_info->reward_set_.size() && amount > 0,
			RETURN_MAY_ACT_BUY, ERROR_CLIENT_OPERATE);

	MayActivityer::ActTypeItem& m_act_t_item = this->find_may_act_item(act_index);
	MayActDetail::ActReward& act_reward = act_info->reward_set_[buy_index];
	CONDITION_NOTIFY_RETURN(act_reward.now_cost_ > 0, RETURN_MAY_ACT_BUY, ERROR_CONFIG_ERROR);

	if (act_reward.times_ > 0)
	{
		MayActivityer::ActItem& m_act_item = m_act_t_item.act_item_map_[buy_index];
		CONDITION_NOTIFY_RETURN(m_act_item.drawed_ + amount <= act_reward.times_,
				RETURN_MAY_ACT_BUY, ERROR_ARENA_BUY_LIMIT);
	}

	Proto31400055 inner;
	inner.set_type(act_info->first_type_);
	inner.set_first_index(act_index);
	inner.set_second_index(buy_index);
	inner.set_money(act_reward.now_cost_);
	inner.set_amount(amount);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicMayActivityer::request_may_activity_buy_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);

	CONDITION_NOTIFY_RETURN(request->ret() == 0, RETURN_MAY_ACT_BUY, request->ret());

	int first_index = request->first_index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			first_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL,RETURN_MAY_ACT_BUY, ERROR_NO_FUN_ACTIVITY);

	int second_index = request->second_index();
	MayActDetail::ActReward& act_reward = act_info->reward_set_[second_index];
	MayActivityer::ActTypeItem& m_act_t_item = this->find_may_act_item(first_index);
	MayActivityer::ActItem& m_act_item = m_act_t_item.act_item_map_[second_index];

	int amount = request->amount();
	m_act_item.drawed_ += amount;

	const Json::Value& reward_json = CONFIG_INSTANCE->reward(act_reward.reward_id_);
	JUDGE_RETURN(reward_json.empty() == false, 0);

	SerialObj obj(ADD_FROM_MAY_ACT_BUY, act_info->first_type_, act_info->second_type_);
	RewardInfo reward_info;
	GameCommon::make_up_reward_items(reward_info, reward_json);
	for (ItemObjVec::iterator iter = reward_info.item_vec_.begin();
			iter != reward_info.item_vec_.end(); ++iter)
	{
		this->request_add_item(obj, iter->id_, iter->amount_ * amount, iter->bind_);
	}

	this->fetch_may_activity_info(first_index);

	Proto50100254 respond;
	respond.set_index(first_index);
	FINER_PROCESS_RETURN(RETURN_MAY_ACT_BUY, &respond);
}

int LogicMayActivityer::request_daily_run_friend_list(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100264*, request, RETURN_DAILY_RUN_FRIEND);

	return this->daily_run_friend_list(request->index(), request->page());
}

int LogicMayActivityer::daily_run_friend_list(int index, int page)
{
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_FRIEND, ERROR_NO_FUN_ACTIVITY);

	MayActivityer::ActTypeItem& m_act_t_item = this->find_may_act_item(index);
	if (m_act_t_item.sub_value_ >= act_info->limit_cond_[1])
	{
		FINER_PROCESS_NOTIFY(RETURN_DAILY_RUN_FRIEND);
	}

	LogicPlayer *friend_player = NULL;
	LogicPlayer *player = this->logic_player();
	ThreeObjVec friend_vec;

	LogicSocialerDetail &socialer_detail = player->socialer_detail();
	for(LongMap::iterator iter = socialer_detail.__friend_list.begin();
			iter != socialer_detail.__friend_list.end(); ++iter)
	{
		JUDGE_CONTINUE(m_act_t_item.send_map_.count(iter->first) <= 0);
		JUDGE_CONTINUE(LOGIC_MONITOR->find_player(iter->first, friend_player) == 0);

		MayActivityer::ActTypeItem& friend_act_item = friend_player->find_may_act_item(index);
		JUDGE_CONTINUE(friend_act_item.second_sub_ < act_info->limit_cond_[0]);

		ThreeObj obj;
		obj.id_ = iter->first;
		obj.value_ = friend_player->role_detail().__level;
		obj.tick_ = friend_player->vip_detail().__vip_level;
		obj.sub_ = friend_player->role_detail().__sex;
		obj.name_ = friend_player->name();
		friend_vec.push_back(obj);
	}

	std::sort(friend_vec.begin(), friend_vec.end(), GameCommon::three_comp_by_desc);

	PageInfo page_info;
	GameCommon::game_page_info(page_info, page, friend_vec.size(), MayActDetail::FRIEND_PAGE);

	Proto50100264 respond;
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);

	for (int i = page_info.start_index_; i < ::std::min(page_info.total_count_,
			page_info.start_index_ + MayActDetail::FRIEND_PAGE); ++i)
	{
		ThreeObj &obj = friend_vec[i];
		ProtoRoleInfo *role_info = respond.add_role_info();
		role_info->set_role_id(obj.id_);
		role_info->set_role_sex(obj.sub_);
		role_info->set_role_name(obj.name_);
	}

	FINER_PROCESS_RETURN(RETURN_DAILY_RUN_FRIEND, &respond);
}

int LogicMayActivityer::request_start_run_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100261*, request, RETURN_DAILY_RUN_RUN);

	int act_index = request->index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			act_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_RUN, ERROR_NO_FUN_ACTIVITY);

	int item_type = request->type();
	const Json::Value &daily_run_item = CONFIG_INSTANCE->daily_run_item(item_type);
	CONDITION_NOTIFY_RETURN(daily_run_item.empty() == false,
			RETURN_DAILY_RUN_RUN, ERROR_CLIENT_OPERATE);

	Proto31400055 inner;
	inner.set_type(act_info->first_type_);
	inner.set_first_index(act_index);
	inner.set_item_id(daily_run_item["item_id"].asInt());
	inner.set_item_type(item_type);
	inner.set_amount(request->amount());
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicMayActivityer::request_start_run_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);

	CONDITION_NOTIFY_RETURN(request->ret() == 0, RETURN_DAILY_RUN_RUN, request->ret());

	int first_index = request->first_index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			first_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_RUN, ERROR_NO_FUN_ACTIVITY);

	int item_type = request->item_type();
	const Json::Value &daily_run_item = CONFIG_INSTANCE->daily_run_item(item_type);
	CONDITION_NOTIFY_RETURN(daily_run_item.empty() == false,
			RETURN_DAILY_RUN_RUN, ERROR_CLIENT_OPERATE);

	int reward_id = daily_run_item["reward_id"].asInt();
	const Json::Value& reward_json = CONFIG_INSTANCE->reward(reward_id);
	JUDGE_RETURN(reward_json.empty() == false, 0);

	SerialObj obj(ADD_FROM_DAILY_RUN_RUN, act_info->first_type_, act_info->second_type_);
	RewardInfo reward_info;
	GameCommon::make_up_reward_items(reward_info, reward_json);
	for (ItemObjVec::iterator iter = reward_info.item_vec_.begin();
			iter != reward_info.item_vec_.end(); ++iter)
	{
		this->request_add_item(obj, iter->id_, iter->amount_ * request->amount(), iter->bind_);
	}

	Proto50100261 respond;
	respond.set_index(first_index);

	MayActDetail::RunInfo &player_run = act_info->run_role_map_[this->role_id()];
	if (player_run.location_ >= act_info->limit_cond_[3])
	{
		player_run.location_ = act_info->limit_cond_[3];

		int exp = daily_run_item["exp"].asInt() * request->amount();
		if (exp > 0)
		{
			this->request_add_exp(exp,EXP_FROM_DAILY_RUN);
		}
	}
	else
	{
		int move = daily_run_item["move"].asInt();
		int before_value = player_run.location_;
		player_run.tick_ = ::time(NULL);
		player_run.location_ += move * request->amount() * act_info->limit_cond_[4];

		if (player_run.location_ > act_info->limit_cond_[3])
			player_run.location_ = act_info->limit_cond_[3];

		int after_value = player_run.location_;
//		this->send_run_direct_award(before_value, after_value, act_info->limit_cond_[4]);  //暂时屏蔽

		respond.set_move(after_value - before_value);
		respond.set_position(after_value);

		this->update_may_act_one_value(act_info->first_type_, act_info->second_type_, player_run.location_);
	}

	FINER_PROCESS_RETURN(RETURN_DAILY_RUN_RUN, &respond);
}

int LogicMayActivityer::request_daily_run_buy_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100262*, request, RETURN_DAILY_RUN_BUY);

	int act_index = request->index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			act_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_BUY, ERROR_NO_FUN_ACTIVITY);

	int item_type = request->type();
	const Json::Value &daily_run_item = CONFIG_INSTANCE->daily_run_item(item_type);
	CONDITION_NOTIFY_RETURN(daily_run_item.empty() == false,
			RETURN_DAILY_RUN_BUY, ERROR_CLIENT_OPERATE);

	int amount = request->amount();
	int cost = daily_run_item["cost"].asInt();
	CONDITION_NOTIFY_RETURN(cost > 0 && amount > 0, RETURN_DAILY_RUN_BUY,
			ERROR_CLIENT_OPERATE);

	Proto31400055 inner;
	inner.set_type(act_info->first_type_);
	inner.set_first_index(act_index);
	inner.set_item_type(item_type);
	inner.set_money(cost * amount);
	inner.set_amount(amount);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicMayActivityer::request_daily_run_buy_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);

	CONDITION_NOTIFY_RETURN(request->ret() == 0, RETURN_DAILY_RUN_BUY, request->ret());

	int first_index = request->first_index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			first_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_BUY, ERROR_NO_FUN_ACTIVITY);

	int item_type = request->type();
	const Json::Value &daily_run_item = CONFIG_INSTANCE->daily_run_item(item_type);
	CONDITION_NOTIFY_RETURN(daily_run_item.empty() == false,
			RETURN_DAILY_RUN_BUY, ERROR_CLIENT_OPERATE);

	int item_id = daily_run_item["item_id"].asInt();
	this->request_add_item(ADD_FROM_DAILY_RUN_BUY, item_id, request->amount());

	Proto50100262 respond;
	respond.set_index(first_index);
	FINER_PROCESS_RETURN(RETURN_DAILY_RUN_BUY, &respond);
}

int LogicMayActivityer::requrst_daily_run_send(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100263*, request, RETURN_DAILY_RUN_SEND);

	int act_index = request->index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			act_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_SEND, ERROR_NO_FUN_ACTIVITY);

	Int64 player_id = request->player_id();
	LogicPlayer *player = NULL;
	if (LOGIC_MONITOR->find_player(player_id, player) != 0)
	{
		this->daily_run_friend_list(act_index);
		return this->respond_to_client_error(RETURN_DAILY_RUN_SEND, ERROR_PLAYER_OFFLINE);
	}

	MayActivityer::ActTypeItem& friend_act_item = player->find_may_act_item(act_index);
	CONDITION_NOTIFY_RETURN(friend_act_item.second_sub_ < act_info->limit_cond_[0],
			RETURN_DAILY_RUN_SEND, ERROR_FRIEND_HELP_RUN_LIMIT);

	MayActivityer::ActTypeItem& m_act_t_item = this->find_may_act_item(act_index);
	CONDITION_NOTIFY_RETURN(m_act_t_item.sub_value_ < act_info->limit_cond_[1],
			RETURN_DAILY_RUN_SEND, ERROR_HELP_RUN_LIMIT);
	CONDITION_NOTIFY_RETURN(m_act_t_item.send_map_.count(player_id) <= 0,
			RETURN_DAILY_RUN_SEND, ERROR_HAS_SEND_RUN);

	m_act_t_item.send_map_[player_id] = true;
	++m_act_t_item.sub_value_;
	++friend_act_item.second_sub_;

	this->request_add_item(ADD_FROM_DAILY_RUN_SEND, act_info->send_reward_, 1);
	player->request_add_item(ADD_FROM_DAILY_RUN_SEND, act_info->send_reward_, 1);

	this->fetch_may_activity_info(act_index);
	this->daily_run_friend_list(act_index);

	Proto50100263 respond;
	respond.set_index(act_index);
	FINER_PROCESS_RETURN(RETURN_DAILY_RUN_SEND, &respond);
}

int LogicMayActivityer::request_daily_run_jump_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100267*, request, RETURN_DAILY_RUN_JUMP);

	int act_index = request->index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			act_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_JUMP, ERROR_NO_FUN_ACTIVITY);

	MayActDetail::RunInfo &player_run = act_info->run_role_map_[this->role_id()];
	int move = player_run.location_;
	const Json::Value &run_extra = CONFIG_INSTANCE->daily_run_extra(move);
	CONDITION_NOTIFY_RETURN(run_extra.empty() == false, RETURN_DAILY_RUN_JUMP,
			ERROR_CLIENT_OPERATE);

	int cost_item = run_extra["cost_item"].asInt();
	CONDITION_NOTIFY_RETURN(cost_item > 0, RETURN_DAILY_RUN_JUMP, ERROR_CLIENT_OPERATE);

	Proto31400055 inner;
	inner.set_type(act_info->first_type_);
	inner.set_first_index(act_index);
	inner.set_item_id(cost_item);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicMayActivityer::request_daily_run_jump_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400055*, request, -1);

	CONDITION_NOTIFY_RETURN(request->ret() == 0, RETURN_DAILY_RUN_JUMP, request->ret());

	int first_index = request->first_index();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_act_info(
			first_index, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_info != NULL, RETURN_DAILY_RUN_JUMP, ERROR_NO_FUN_ACTIVITY);

	MayActDetail::RunInfo &player_run = act_info->run_role_map_[this->role_id()];
	int move = player_run.location_;
	const Json::Value &run_extra = CONFIG_INSTANCE->daily_run_extra(move);
	CONDITION_NOTIFY_RETURN(run_extra.empty() == false, RETURN_DAILY_RUN_JUMP,
			ERROR_CLIENT_OPERATE);

	int jump_reward = run_extra["jump_reward"].asInt();
	SerialObj obj(ADD_FROM_DAILY_RUN_JUMP, move);
	this->request_add_reward(jump_reward, obj);

	Proto50100267 respond;
	respond.set_index(first_index);
	FINER_PROCESS_RETURN(RETURN_DAILY_RUN_JUMP, &respond);
}

MayActivityer &LogicMayActivityer::may_detail()
{
	return this->may_detial_;
}

int LogicMayActivityer::fetch_grab_red_packet_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto50100252*, request, -1);
	int activity_id = request->index();
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(activity_id);
	JUDGE_RETURN(act_info != NULL && act_info->first_type_ == MayActDetail::SEND_POCKET, -1);

	//更新各红包活动状态信息
	this->update_red_act_info_list();

	if(act_info->red_act_info_vec_.empty())
	{
		MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
		for(uint i = 0; i < reward_set.size(); ++i)
		{
			ProtoAllRedActInfo *info = request->add_all_red_info();
			MayActDetail::RedActInfo red_act_info;
			info->set_money(0);
			info->set_state(MayActDetail::RedActInfo::CLOSE);
			info->set_tick(0);
			act_info->red_act_info_vec_.push_back(red_act_info);
			//MSG_DEBUG("empty index:%d, money:%d, state:%d, tick:%d", i, red_act_info.money_, red_act_info.state_, red_act_info.tick_);
		}
	}
	else
	{
		for(uint i = 0; i < act_info->red_act_info_vec_.size(); ++i)
		{
			ProtoAllRedActInfo *info = request->add_all_red_info();
			MayActDetail::RedActInfo &red_act_info = act_info->red_act_info_vec_[i];
			info->set_money(red_act_info.money_);
			info->set_state(red_act_info.state_);
			info->set_tick(red_act_info.tick_);
			//MSG_DEBUG("index:%d, money:%d, state:%d, tick:%d", i, red_act_info.money_, red_act_info.state_, red_act_info.tick_);
		}
	}


	return 0;
}

int LogicMayActivityer::fetch_red_packet_reward(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100256*, request, -1);
	int get_reward_index = request->index();
	int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
	JUDGE_RETURN(act_info != NULL, -1);
	//MayActDetail::ActRewardSet& reward_set = act_info->reward_set_;
	act_info->cur_packet_act_times_ = this->fetch_act_open_time_index(act_info);
	//int &cur_red_group = role_detail.cur_red_packet_group_;
	int &cur_red_group = get_reward_index;
	LogicRoleDetail &role_detail = this->role_detail();
	role_detail.cur_red_packet_group_ = get_reward_index;
	//先判断该玩家是否有获取奖励
	//MayActDetail::GroupMap& group = act_info->player_reward_info_[act_info->cur_packet_act_times_];
	MayActDetail::IndexMap &index_info = act_info->player_reward_info_[get_reward_index];

	int no_get_index = -1;  //没有记录领取红包的索引
	for(MayActDetail::IndexMap::iterator iter = index_info.begin(); iter != index_info.end(); ++iter)
	{
		MayActDetail::LUMap &lu_map = iter->second;
		if(lu_map.end() == lu_map.find(this->role_id()))
		{
			no_get_index = iter->first;
			break;
		}
	}

	if(index_info.size() == 1 && no_get_index == -1)
	{
		return this->notify_cur_group_reward_info(act_info);
	}
	//当前红包组没有记录
	if(no_get_index == -1)
		no_get_index = 1;


	if(index_info.count(this->role_id()) > 0)
	{
		return this->notify_cur_group_reward_info(act_info);
	}

	MayActDetail::GroupPacketMap& group_map = act_info->all_red_packet_map_[act_info->cur_packet_act_times_];
	int money = 0;

	JUDGE_RETURN(group_map.count(cur_red_group) > 0, -1);
	MayActDetail::RedPacketVec& red_packet_vec = group_map[cur_red_group];

	for(uint i = 0; i < red_packet_vec.size(); ++i)
	{
		IntVec& money_vec = red_packet_vec[i];
		if(money_vec.empty() && i == (uint)no_get_index)
			continue;
		IntVec::iterator iter = money_vec.begin();
		money = *iter;
		money_vec.erase(iter);
		if(money_vec.empty())
		{
			red_packet_vec.erase(red_packet_vec.begin() + i);
		}

		MSG_USER("get money:%d", money);
		SerialObj obj(ADD_FROM_RED_PACKET_MONEY, act_info->act_index_);
		int reward_size = act_info->reward_set_.size();
		JUDGE_RETURN(reward_size >= no_get_index && no_get_index > 0, 0);
		int reward_id = act_info->reward_set_[no_get_index - 1].reward_id_;
		this->request_add_item(obj, reward_id, money, true);
		MayActDetail::IndexMap& player_reward_info = act_info->player_reward_info_[cur_red_group];
		MayActDetail::LUMap& single_red_reward_info = player_reward_info[no_get_index];
		MayActDetail::PlayerInfo player_info;
		player_info.money_ = money;
		player_info.role_id_ = this->role_id();
		player_info.name_ = this->name();
		single_red_reward_info[this->role_id()] = player_info;
		break;
	}

	if(money == 0)
	{
//			this->fetch_red_packet_reward_info(NULL);
//			return this->fetch_red_packet_reward(NULL);

		role_detail.cur_red_packet_group_ = get_reward_index;
	}

	return this->notify_cur_group_reward_info(act_info);
}



int LogicMayActivityer::fetch_act_open_time_index(MayActDetail::ActInfo *act_info)
{
	JUDGE_RETURN(act_info != NULL, -1);
	MayActDetail::LimitTimeVec &limit_time_vec = act_info->limit_time_vec_;
	MayActDetail::LimitTimeVec::iterator iter = limit_time_vec.begin();
	for( ; iter != limit_time_vec.end(); ++iter)
	{
		if((*iter).state_ == 0)
			return (*iter).group_;
	}
	return -1;
}

int LogicMayActivityer::fetch_red_packet_reward_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100255*, request, RETURN_RED_PACKET_INFO);
	int index = request->index();
    int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
    MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(act_info != NULL, -1);
    MAY_ACTIVITY_SYS->refresh_act_open_time(act_info->act_index_);

    //判断当前活动是否开启
    bool is_open_time = MAY_ACTIVITY_SYS->is_act_open_time(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(is_open_time, -1);

    act_info->cur_packet_act_times_ = this->fetch_act_open_time_index(act_info);
    MayActDetail::GroupPacketMap& group_map = act_info->all_red_packet_map_[act_info->cur_packet_act_times_];
    int group_size = group_map.size();
    JUDGE_RETURN(group_size > 0, -1);

    MayActDetail::ActRewardSet& reward_set = act_info->reward_set_;
    JUDGE_RETURN((uint)index <= reward_set.size(), -1);

    LogicRoleDetail &role_detail = this->role_detail();
    Proto50100255 respond;
	MayActDetail::RedPacketVec& red_packet_vec = group_map[index];
	MSG_USER("group_map group:%d", index);
	int red_packet_size = red_packet_vec.size();
	JUDGE_RETURN(red_packet_size > 0, -1);

	IntVec red_vec;
	for(uint i = 0; i < red_packet_vec.size(); ++i)
	{
		red_vec = red_packet_vec[i];
		if(!red_vec.empty())
			break;
	}

	if(red_vec.empty())
		return -1;

	for(uint i = 0; i < red_vec.size(); ++i)
	{
		MSG_USER("index:%d, rand_red_packet:%d", i, red_vec[i]);
	}
	role_detail.cur_red_packet_group_ = index;

	//index是从1开始
	MayActDetail::ActReward& act_reward = reward_set[index - 1];
	respond.set_person_count(red_vec.size());
	respond.set_red_packet_money(act_reward.packet_money_);
	FINER_PROCESS_RETURN(RETURN_RED_PACKET_INFO, &respond);
}

int LogicMayActivityer::fetch_single_red_packet_info(int group)
{
	JUDGE_RETURN(group > 0, -1);

    int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
    MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(act_info != NULL, -1);
    MAY_ACTIVITY_SYS->refresh_act_open_time(act_info->act_index_);

    //判断当前活动是否开启
    bool is_open_time = MAY_ACTIVITY_SYS->is_act_open_time(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(is_open_time, -1);

    act_info->cur_packet_act_times_ = this->fetch_act_open_time_index(act_info);
    MayActDetail::GroupPacketMap &group_map = act_info->all_red_packet_map_[act_info->cur_packet_act_times_];
    MayActDetail::RedPacketVec &red_packet_vec = group_map[group];
    if(red_packet_vec.size() > 0)
    {
    	bool flag = false;
    	for(uint i = 0; i < red_packet_vec.size(); ++i)
    	{
    		IntVec &vec = red_packet_vec[i];
    		if(vec.size() != 0)
    		{
    			flag = true;
    			break;
    		}
    	}

    	if(flag)
    		return red_packet_vec.size();
    	else
    		return 0;
    }
    else
    	return 0;
}

void LogicMayActivityer::init_red_act_info_list()
{
    int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
    MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(act_info != NULL, );
    MayActDetail::RedActInfoVec &red_info_vec = act_info->red_act_info_vec_;
    red_info_vec.clear();
    MayActDetail::ActRewardSet &reward_set = act_info->reward_set_;
    MayActDetail::RedActInfo red_info;
    act_info->cur_open_red_act_tick_ = ::time(0);
    for(uint i = 0; i < reward_set.size(); ++i)
    {
    	red_info.reset();
    	if(i == 0)
    	{
    		red_info.state_ = MayActDetail::RedActInfo::BEGIN;
    		red_info.money_ = reward_set[i].packet_money_;
    	}
    	else if(i == 1)
    	{
    		red_info.state_ = MayActDetail::RedActInfo::READY;
    		red_info.tick_ = reward_set[i].red_act_interval_;
    	}
    	else
    		red_info.state_ = MayActDetail::RedActInfo::CLOSE;
    	red_info_vec.push_back(red_info);
    }
    this->inner_notify_assist_event(act_info->red_point_, 1);
}

void LogicMayActivityer::clear_red_act_info()
{
    int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
    MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
    JUDGE_RETURN(act_info != NULL, );
    act_info->all_red_packet_map_.clear();
    for(uint i = 0; i < act_info->red_act_info_vec_.size(); ++i)
    {
    	MayActDetail::RedActInfo &red_act_info = act_info->red_act_info_vec_[i];
    	red_act_info.money_ = 0;
    	red_act_info.state_ = MayActDetail::RedActInfo::END;
    	red_act_info.tick_ = 0;
    }
    this->inner_notify_assist_event(act_info->red_point_, 0);
}

int LogicMayActivityer::fetch_red_packet_state_by_group(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100259*, request, RETURN_ONE_MAY_ACTIVITY);
	int index = request->index();
	int cur_passed_day = MAY_ACTIVITY_SYS->cur_may_day();
	MayActDetail::ActInfo* act_info = MAY_ACTIVITY_SYS->find_item_by_day(MayActDetail::SEND_POCKET, cur_passed_day);
	JUDGE_RETURN(act_info != NULL, -1);

	MayActDetail::IndexMap &index_info = act_info->player_reward_info_[index];

	Proto50100259 respond;
	int flag = false;

	if(this->fetch_single_red_packet_info(index))
	{
		for(MayActDetail::IndexMap::iterator iter = index_info.begin(); iter != index_info.end(); ++iter)
		{
			if(iter->second.find(this->role_id()) != iter->second.end())
			{
				flag = true;
				break;
			}
		}
	}
	else
		flag = true;
	respond.set_index(index);
	respond.set_activity_id(act_info->act_index_);
	respond.set_open_state(flag);
	return this->respond_to_client(RETURN_RED_PACKET_ALL_INFO, &respond);
}


void LogicMayActivityer::update_daily_login_recharge_sign_handle()
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::DAILY_LOGIN, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_info != NULL, ;);

	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info->act_index_);
	MayActivityer::ActItem& s_act_item = m_act_item.act_item_map_[0];
	//计算登陆奖励的可领次数
	LogicRoleDetail& role_detail = this->role_detail();
	if (m_act_item.sub_value_ == 0 && role_detail.today_recharge_gold_ > 0)
	{
		MSG_USER("add reward frequency of daily login");
		m_act_item.sub_value_ = 1;
		s_act_item.arrive_ += 1;
	}

	if (s_act_item.arrive_ > s_act_item.drawed_)
	{
		this->notify_may_activity_update(act_info);
	}
}

void LogicMayActivityer::update_daily_recharge_recharge_sign_handle()
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_item_by_day(
			MayActDetail::DAILY_RECHARGE, MAY_ACTIVITY_SYS->cur_may_day());
	JUDGE_RETURN(act_info != NULL, ;);

	LogicRoleDetail& role_detail = this->role_detail();
	JUDGE_RETURN(role_detail.today_recharge_gold_ > 0, ;);

	this->update_may_act_one_value(act_info->first_type_, act_info->second_type_, role_detail.today_recharge_gold_);
}

void LogicMayActivityer::update_may_act_one_value(int first_type, int day, int value)
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_item_by_day(first_type, day);
	JUDGE_RETURN(act_info != NULL, ;);

	MayActivityer::ActTypeItem& m_act_item = this->find_may_act_item(act_info->act_index_);

	int flag = false;
	int total = act_info->reward_set_.size();
	for (int i = 0; i < total; ++i)
	{
		MayActDetail::ActReward &act_reward = act_info->reward_set_[i];
		JUDGE_CONTINUE(value >= act_reward.cond_[0]);

		m_act_item.act_item_map_[i].arrive_ = true;
		JUDGE_CONTINUE(act_reward.drawed_map_.count(this->role_id()) == 0);

		flag = true;
		act_reward.drawed_map_[this->role_id()] = 0;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_may_activity_update(act_info);
}

void LogicMayActivityer::notify_may_activity_update(MayActDetail::ActInfo* act_info)
{
	if (this->has_may_activity_reward(*act_info) == true)
	{
		this->inner_notify_assist_event(act_info->red_point_, 1);
	}
}

void LogicMayActivityer::may_activity_announce(int index, uint reward_index)
{
	MayActDetail::ActInfo *act_info = MAY_ACTIVITY_SYS->find_act_info(index);
	JUDGE_RETURN(act_info != NULL, ;);

	MayActDetail::ActReward &act_reward = act_info->reward_set_[reward_index];
	JUDGE_RETURN(act_reward.show_id_ > 0, ;);

	BrocastParaVec para_vec;

	switch (act_info->first_type_)
	{
	case MayActDetail::DAILY_RUN:
	{
		GameCommon::push_brocast_para_string(para_vec, this->name());
		if (act_reward.cond_[0] < act_info->limit_cond_[3])
		{
			GameCommon::push_brocast_para_int(para_vec, act_reward.cond_[0]);
		}
		break;
	}
	case MayActDetail::COUPLE_HEART:
	{
		LogicPlayer *player = this->logic_player();
		WeddingDetail *wedding_detail = player->wedding_detail();
		JUDGE_RETURN(wedding_detail != NULL, ;);

		WeddingDetail::WeddingRole *partner_info = NULL;
		if (this->role_id() == wedding_detail->__partner_1.__role_id)
		{
		    partner_info = &wedding_detail->__partner_2;
		}
		else
		{
			partner_info = &wedding_detail->__partner_1;
		}
		JUDGE_RETURN(partner_info != NULL, ;);

		GameCommon::push_brocast_para_string(para_vec, this->name());
		GameCommon::push_brocast_para_string(para_vec, partner_info->__role_name);

		break;
	}
	case MayActDetail::NICE_FASHION:
	{
		GameCommon::push_brocast_para_string(para_vec, this->name());
		GameCommon::push_brocast_para_string(para_vec, act_reward.name_);
		break;
	}
	default:
		break;
	}

	GameCommon::announce(act_reward.show_id_, &para_vec);
}

int LogicMayActivityer::may_act_test_reset(int type)
{
	for (MayActDetail::ActInfoSet::iterator iter = MAY_ACTIVITY_SYS->act_info_set_.begin();
			iter != MAY_ACTIVITY_SYS->act_info_set_.end(); ++iter)
	{
		MayActDetail::ActInfo &act_info = *iter;
		JUDGE_CONTINUE(act_info.first_type_ == type);

		MayActivityer::ActTypeItem& m_act_t_item = this->find_may_act_item(act_info.act_index_);

		switch (act_info.first_type_)
		{
		case MayActDetail::DAILY_BUY:
		{
			m_act_t_item.reset_every();
			m_act_t_item.act_item_map_.clear();
			break;
		}
		case MayActDetail::DAILY_RUN:
		{
			m_act_t_item.sub_value_ = 0;
			m_act_t_item.second_sub_ = 0;
			m_act_t_item.send_map_.clear();
			m_act_t_item.role_map_.clear();
			m_act_t_item.act_item_map_.clear();

			MayActDetail::RunInfo &player_run = act_info.run_role_map_[this->role_id()];
			player_run.location_ = 0;
			player_run.tick_ = 0;
			break;
		}
		case MayActDetail::COUPLE_HEART:
		{
			LogicPlayer *player = this->logic_player();
			Int64 wedding_id = player->wedding_id();
			JUDGE_RETURN(wedding_id > 0, 0);

			MayActDetail::CoupleInfo &couple_info = act_info.couple_info_map_[wedding_id];
			couple_info.buy_tick_ = 0;
			couple_info.online_tick_ = 0;

			m_act_t_item.reset_every();
			m_act_t_item.act_item_map_.clear();

			break;
		}
		default:
			break;
		}
	}

	return 0;
}

void LogicMayActivityer::test_set_login_day(int day)
{
	this->role_detail().continuity_login_day_ = day;
	this->daily_login_refresh_day();
}



