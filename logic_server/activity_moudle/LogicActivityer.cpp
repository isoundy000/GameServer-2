/*
 * LogicActivityer.cpp
 *
 *  Created on: Oct 13, 2016
 *      Author: peizhibi
 */

#include "LogicActivityer.h"
#include "OpenActivitySys.h"
#include "FestActivitySys.h"
#include "ArenaSys.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"

LogicActivityer::LogicActivityer()
{
	// TODO Auto-generated constructor stub
}

LogicActivityer::~LogicActivityer()
{
	// TODO Auto-generated destructor stub
}

void LogicActivityer::reset_everyday()
{
	IntMap remove_map;
	for (MLActivityerDetial::ActTypeItemMap::iterator
			iter = this->drawed_detial_.act_type_item_set_.begin();
			iter != this->drawed_detial_.act_type_item_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->is_activity_day_clear(iter->first) == true);
		remove_map[iter->first] = true;
	}

	for (IntMap::iterator iter = remove_map.begin();
			iter != remove_map.end(); ++iter)
	{
		this->drawed_detial_.act_type_item_set_.erase(iter->first);
	}

	this->update_combine_activity_recharge(0, 0);
	this->reset_cornucopia_info();
	this->fetch_cornucopia_info(10901);
}

void LogicActivityer::reset_activityer()
{
	this->activity_rank_line_ = 0;
	this->drawed_detial_.reset();
}

void LogicActivityer::refresh_act_type_item(int index)
{
	JUDGE_RETURN(this->drawed_detial_.act_type_item_set_.count(index) == 0, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = this->fetch_activity_by_index(index);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& act_item = this->drawed_detial_.act_type_item_set_[index];
	act_item.cur_index_ = index;
}

void LogicActivityer::combine_activity_first_times()
{
	JUDGE_RETURN(CONFIG_INSTANCE->do_combine_server() == true, ;);

	Int64 combine_tick = CONFIG_INSTANCE->combine_server_date();
	JUDGE_RETURN(combine_tick > this->drawed_detial_.combine_tick_, ;);

	this->drawed_detial_.accu_value_map_.clear();
	this->drawed_detial_.combine_tick_ = combine_tick;

	LogicRoleDetail& role_detail = this->role_detail();
	ThreeObjMap& mount_info = role_detail.mount_info_;

	for (ThreeObjMap::iterator iter = mount_info.begin();
			iter != mount_info.end(); ++iter)
	{
		this->update_combine_activity_mount(iter->second.id_, iter->second.value_);
		this->update_combine_return_activity_mount(iter->second.id_, iter->second.value_);
	}

	if (role_detail.today_recharge_gold_ > 0)
	{
		int today_gold = role_detail.today_recharge_gold_;
		this->update_combine_activity_login(true);
		this->update_combine_activity_recharge(today_gold, today_gold);
		this->update_combine_return_activity_recharge(today_gold);
	}
}

MLActivityerDetial& LogicActivityer::fetch_act_detail()
{
	return this->drawed_detial_;
}

MLActivityerDetial::ActTypeItem& LogicActivityer::find_act_type_item(int index)
{
	this->refresh_act_type_item(index);
	return this->drawed_detial_.act_type_item_set_[index];
}

BackSetActDetail::ActTypeItem* LogicActivityer::fetch_activity_by_index(int index)
{
	if (this->is_festival_activity(index) == true)
	{
		return FEST_ACTIVITY_SYS->find_item(index);
	}
	else
	{
		return LOGIC_OPEN_ACT_SYS->find_item(index);
	}
}

int LogicActivityer::is_activity_day_clear(int index)
{
	if (LOGIC_OPEN_ACT_SYS->is_open_activity_id(index))
	{
		return false;
	}
	else
	{
		return true;
	}
}

int LogicActivityer::is_festival_activity(int index)
{
	return index / 10000 == GameEnum::MAIN_ACT_FEST;
}

int LogicActivityer::has_activity_reward(BackSetActDetail::ActTypeItem& s_act_t_item)
{
	int total = s_act_t_item.act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		JUDGE_CONTINUE(this->has_activity_reward(s_act_t_item, i) == BackSetActDetail::UNDRAW);
		return true;
	}

	return false;
}

int LogicActivityer::has_activity_reward(BackSetActDetail::ActTypeItem& s_act_t_item,
		int second_index)
{
	int reward_type = s_act_t_item.act_item_set_[second_index].reward_type_;
	JUDGE_RETURN(reward_type == BackSetActDetail::D_REWARD_T_0
			|| reward_type == BackSetActDetail::D_REWARD_T_3, BackSetActDetail::CNDRAW);

	//second_index奖励的index（下标索引）
	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(s_act_t_item.act_index_);
	MLActivityerDetial::ActItem& s_act_item = m_act_item.act_item_map_[second_index];

	switch (s_act_t_item.cond_type_)
	{
	case BackSetActDetail::COND_TYPE_1:
	case BackSetActDetail::COND_TYPE_4:
	{
		JUDGE_BREAK(s_act_t_item.t_sub_map_.count(this->role_id()) > 0);
		s_act_item.arrive_ = 1;
		break;
	}
	case BackSetActDetail::COND_TYPE_2:
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item.act_item_set_[second_index];
		JUDGE_BREAK(act_item.arrive() == true);
		s_act_item.arrive_ = 1;
		break;
	}
	case BackSetActDetail::COND_TYPE_3:
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item.act_item_set_[second_index];
		JUDGE_BREAK(act_item.arrive() == true);
		JUDGE_BREAK(this->role_detail().today_recharge_gold_ >= act_item.cond_value_[0]);
		s_act_item.arrive_ = 1;
		break;
	}
	}

	if (s_act_item.arrive_ == 0)
	{
		//条件未达到
		return BackSetActDetail::CNDRAW;
	}

	if (s_act_item.drawed_ < s_act_item.arrive_)
	{
		//未领取
		return BackSetActDetail::UNDRAW;
	}

	//已领取
	return BackSetActDetail::DRAWED;
}

int LogicActivityer::is_no_send_activity(int index)
{
	const Json::Value &conf = CONFIG_INSTANCE->no_send_act();
	JUDGE_RETURN(conf.empty() == false, false);

	const Json::Value &no_send_list = conf["activity_list"];
	for (uint i = 0; i < no_send_list.size(); ++i)
	{
		int id = no_send_list[i].asInt();
		JUDGE_RETURN(index != id, true);
	}

	return false;
}

int LogicActivityer::fetch_left_activity_time(int index)
{
	if (this->is_festival_activity(index) == true)
	{
		return FEST_ACTIVITY_SYS->left_activity_time();
	}
	else if (LOGIC_OPEN_ACT_SYS->is_combine_activity_id(index) == true)
	{
		return CONFIG_INSTANCE->left_combine_activity_time();
	}
	else
	{
		return CONFIG_INSTANCE->left_open_activity_time();
	}
}

int LogicActivityer::fetch_activity_info(int index)
{
	BackSetActDetail::ActTypeItem* act_t_item = this->fetch_activity_by_index(index);
	return this->fetch_open_activity(act_t_item);
}

int LogicActivityer::fetch_activity_draw_serial(int index)
{
	if (this->is_festival_activity(index))
	{
		return ADD_FROM_FESTIVE_ACTIV;
	}
	else if (LOGIC_OPEN_ACT_SYS->is_open_activity_id(index))
	{
		return ADD_FROM_OPEN_ACTIV;
	}
	else if (LOGIC_OPEN_ACT_SYS->is_return_activity_id(index))
	{
		return ADD_FROM_RETURN_ACTIV;
	}
	else if (LOGIC_OPEN_ACT_SYS->is_combine_activity_id(index))
	{
		return ADD_FROM_COBMINE_ACTIV;
	}
	else
	{
		return ADD_FROM_RETURN_COMBINE;
	}
}

int LogicActivityer::fetch_open_activity_list()
{
	int main_type = LOGIC_OPEN_ACT_SYS->main_act_type();
	if (main_type == GameEnum::MAIN_ACT_C_RETURN)
	{
		main_type = GameEnum::MAIN_ACT_RETURN;
	}

	Proto50100213 respond;
	respond.set_main_act(main_type);

	IntMap& act_list = LOGIC_OPEN_ACT_SYS->fetch_act_list();
	for (IntMap::iterator iter = act_list.begin(); iter != act_list.end(); ++iter)
	{
		int ret = this->is_no_send_activity(iter->first);
		JUDGE_CONTINUE(ret == false);

		respond.add_act_list(iter->first);
	}

	FINER_PROCESS_RETURN(RETURN_OPEN_ACT_LIST, &respond);
}

int LogicActivityer::fetch_open_activity(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100211*, request, RETURN_OPEN_ACT_FETCH_INFO);
	return this->fetch_activity_info(request->index());
}

int LogicActivityer::fetch_open_activity(BackSetActDetail::ActTypeItem* act_t_item)
{
	JUDGE_RETURN(act_t_item != NULL, -1);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(act_t_item->act_index_);
	this->drawed_detial_.last_request_index_ = act_t_item->act_index_;

	Proto50100211 respond;
	respond.set_type(act_t_item->first_type_);
	act_t_item->first_info_.serialize(respond.mutable_first_role());

	int left_time = this->fetch_left_activity_time(act_t_item->act_index_);
	respond.set_left_time(left_time);

	PActTypeItem* p_type_item = respond.mutable_act_detail();
	p_type_item->set_act_index(act_t_item->act_index_);
	p_type_item->set_first_type(act_t_item->first_type_);
	p_type_item->set_second_type(act_t_item->second_type_);
	p_type_item->set_limit(act_t_item->limit_);

	//当天倒计时
	int next_day = GameCommon::next_day();
	p_type_item->set_times(next_day);

	int cur_index = 0;
	for (BackSetActDetail::ActItemSet::iterator act_iter = act_t_item->act_item_set_.begin();
			act_iter != act_t_item->act_item_set_.end(); ++act_iter)
	{
		PRewardItem* reward_item = p_type_item->add_reward_info();
		reward_item->set_index(cur_index);
		reward_item->set_times(act_iter->times_);
		reward_item->set_cash_coupon(act_iter->cash_coupon_);
		reward_item->set_content(act_iter->content_);
		reward_item->set_reward_id(act_iter->reward_id_);
		reward_item->set_sub_value(act_iter->sub_value_);
		reward_item->set_exchange_type(act_iter->exchange_type_);
		reward_item->set_exchange_item_name(act_iter->exchange_item_name_);

		switch (act_t_item->cond_type_)
		{
		case BackSetActDetail::COND_TYPE_2:
		case BackSetActDetail::COND_TYPE_3:
		{
			reward_item->set_arrive(act_iter->sub_map_.size());	//达到条件的人数
			reward_item->set_drawed(0);
			break;
		}
		default:
		{
			//该笔奖励的领取情况
			reward_item->set_arrive(m_act_item.act_item_map_[cur_index].arrive_);
			reward_item->set_drawed(m_act_item.act_item_map_[cur_index].drawed_);
			break;
		}
		}

		//图谱兑换重置需求
		if (act_iter->must_reset_ == true)
		{
			m_act_item.act_item_map_[cur_index].drawed_ = 0;
		}

		// 是否领取
		int draw_flag = this->has_activity_reward(*act_t_item, cur_index);
		reward_item->set_draw_flag(draw_flag);

		//各个奖励领取条件
		int total_cond = act_iter->cond_value_.size();
		for(int i = 0; i < total_cond; ++i)
		{
			reward_item->add_cond(act_iter->cond_value_[i]);
		}

		//商店类活动消耗商品
		for (ItemObjVec::const_iterator iter = act_iter->cost_item_.begin();
				iter != act_iter->cost_item_.end(); ++iter)
		{
			ProtoItem *cost_item = reward_item->add_cost_item();
			iter->serialize(cost_item);
		}

		//商店类活动物品原价
		for (ItemObjVec::const_iterator iter = act_iter->pre_cost_.begin();
				iter != act_iter->pre_cost_.end(); ++iter)
		{
			ProtoItem *pre_item = reward_item->add_pre_item();
			iter->serialize(pre_item);
		}

		cur_index += 1;
	}

	switch (act_t_item->first_type_)
	{
	case BackSetActDetail::C_ACT_ACCU_RECHARGE:
	{
		p_type_item->set_sub_value(this->role_detail().today_recharge_gold_);
		p_type_item->set_second_sub(this->drawed_detial_.accu_value_map_[act_t_item->first_type_]);
		break;
	}
	case BackSetActDetail::C_ACT_CONSUM_RANK:
	{
		p_type_item->set_sub_value(this->fetch_self_combine_activity_consum(act_t_item));
		p_type_item->set_second_sub(m_act_item.second_sub_);
		break;
	}
	case BackSetActDetail::F_ACT_CUMULATIVE_LOGIN:
	{
		PActCumulativeLogin* act = p_type_item->mutable_cumulative_login();
		CumulativeLoginDetail act_detail = this->role_detail().cur_day_detail_;
		act->set_single(act_detail.__single);
		act->set_ten(act_detail.__ten);
		act->set_hundred(act_detail.__hundred);
		act->set_multiple(act_detail.__multiple);
		act->set_login_count(act_detail.__cumulative_day);
		act->set_single_state(act_detail.__single_state);
		act->set_ten_state(act_detail.__ten_state);
		act->set_hundred_state(act_detail.__hundred_state);
		act->set_multiple_state(act_detail.__multiple_state);
//		MSG_USER("role id:%lld, hunred:%d, ten:%d, single:%d, multiple:%d, cumulative_day:%d",
//				this->role_id(), act_detail.__hundred, act_detail.__ten, act_detail.__single, act_detail.__multiple,
//				act_detail.__cumulative_day);
//		MSG_USER("hundred_state:%d, ten_state:%d, single_state:%d, multiple_state:%d",
//				act_detail.__hundred_state, act_detail.__ten_state, act_detail.__single_state, act_detail.__multiple_state);
		break;
	}
	default:
	{
		p_type_item->set_sub_value(m_act_item.sub_value_);
		p_type_item->set_second_sub(m_act_item.second_sub_);
		break;
	}
	}

	p_type_item->set_role_recharge(this->role_detail().today_recharge_gold_);
	p_type_item->set_role_consume(this->role_detail().today_consume_gold_);
	FINER_PROCESS_RETURN(RETURN_OPEN_ACT_FETCH_INFO, &respond);
}

int LogicActivityer::draw_open_activity_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100212*, request, RETURN_OPEN_ACT_DRAW);

	BackSetActDetail::ActTypeItem* s_act_t_item = this->fetch_activity_by_index(request->index());
	CONDITION_NOTIFY_RETURN(s_act_t_item != NULL, RETURN_OPEN_ACT_DRAW, ERROR_NO_FUN_ACTIVITY);

	uint reward_index = request->reward_index();
	CONDITION_NOTIFY_RETURN(reward_index < s_act_t_item->act_item_set_.size(),
			RETURN_OPEN_ACT_DRAW, ERROR_CLIENT_OPERATE);

	//判断是否有奖励可以领取
	int reward_flag = this->has_activity_reward(*s_act_t_item, reward_index);
	CONDITION_NOTIFY_RETURN(reward_flag == BackSetActDetail::UNDRAW,
			RETURN_OPEN_ACT_DRAW, ERROR_ACT_NO_REWARD);
	int ret = 0;

	//这里根据策划要求，特殊发奖励
	if(s_act_t_item->first_type_ == BackSetActDetail::F_ACT_CUMULATIVE_LOGIN)
	{
		int num = this->logic_player()->role_detail().cur_day_detail_.get_multiple_reward();
		ret = this->draw_open_activity_item(*s_act_t_item, reward_index, num);
	}
	else
	{
		//领取奖励
		switch(reward_index)
		{
			default:
			{
				ret = this->draw_open_activity_reward(*s_act_t_item, reward_index, 0);
			}
		}
	}
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_OPEN_ACT_DRAW, ret);

	//取消红点
	if (this->has_activity_reward(*s_act_t_item) == false)
	{
		this->inner_notify_assist_event(s_act_t_item->red_event_, 0);
	}

	this->fetch_activity_info(request->index());
	FINER_PROCESS_NOTIFY(RETURN_OPEN_ACT_DRAW);
}

int LogicActivityer::draw_open_activity_item(BackSetActDetail::ActTypeItem& s_act_t_item,
		uint second_index, int num)
{
	//second_index奖励的index（下标索引）, num为领取数量，0表示全部领取
	BackSetActDetail::ActItem& act_item = s_act_t_item.act_item_set_[second_index];
	JUDGE_RETURN(s_act_t_item.act_item_set_.size() > second_index, ERROR_CLIENT_OPERATE);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(s_act_t_item.act_index_);
	JUDGE_RETURN(m_act_item.act_item_map_[second_index].have_reward() == true, ERROR_ACT_REWARD_ERROR_COUNT);

	switch (act_item.handle_type_)
	{
		default:
		{
			act_item.drawed_map_[this->role_id()] = true;
			m_act_item.act_item_map_[second_index].drawed_ += 1;
			break;
		}
	}

	int serial = this->fetch_activity_draw_serial(s_act_t_item.act_index_);
	SerialObj obj(serial, s_act_t_item.first_type_, s_act_t_item.second_type_);
	return this->request_add_item(obj, act_item.reward_id_, num, true);
}

int LogicActivityer::draw_open_activity_reward(BackSetActDetail::ActTypeItem& s_act_t_item,
		uint second_index, int num)
{
	//second_index奖励的index（下标索引）, num为领取数量，0表示全部领取
	BackSetActDetail::ActItem& act_item = s_act_t_item.act_item_set_[second_index];
	JUDGE_RETURN(s_act_t_item.act_item_set_.size() > second_index, ERROR_CLIENT_OPERATE);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(s_act_t_item.act_index_);
	JUDGE_RETURN(m_act_item.act_item_map_[second_index].have_reward() == true, ERROR_ACT_REWARD_ERROR_COUNT);

	switch (act_item.handle_type_)
	{
	default:
	{
		act_item.drawed_map_[this->role_id()] = true;
		m_act_item.act_item_map_[second_index].drawed_ += 1;
		break;
	}
	case 1:
	{
		//累积方式
		m_act_item.act_item_map_[second_index].arrive_ -= 1;
		this->drawed_detial_.accu_value_map_[s_act_t_item.first_type_] -= act_item.cond_value_[0u];
		break;
	}
	}

	int serial = this->fetch_activity_draw_serial(s_act_t_item.act_index_);
	SerialObj obj(serial, s_act_t_item.first_type_, s_act_t_item.second_type_);
	return this->request_add_reward(act_item.reward_id_, obj);
}

int LogicActivityer::draw_once_open_activity(BackSetActDetail::ActTypeItem& s_act_t_item,
		int second_index)
{
	//second_index奖励的index（下标索引）
	MLActivityerDetial::ActTypeItem& m_act_t_item = this->find_act_type_item(s_act_t_item.act_index_);
	JUDGE_RETURN(m_act_t_item.act_item_map_.count(second_index) > 0, -1);

	MLActivityerDetial::ActItem& m_act_item = m_act_t_item.act_item_map_[second_index];
	JUDGE_RETURN(s_act_t_item.act_item_set_[second_index].cond_value_.size() > 0, -1);
	JUDGE_RETURN(m_act_item.drawed_ < m_act_item.arrive_, -1);

	m_act_item.drawed_++;
	return 0;
}

int LogicActivityer::update_open_activity_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100241*, request, -1);

	int first_type = request->first_type();
	switch(first_type)
	{
	case BackSetActDetail::F_ACT_AIM_CHASE:
	{
		this->update_open_activity_value(first_type, request->sub1(), request->sub2());
		break;
	}
	case BackSetActDetail::F_ACT_GROWTH_MOUNT:
	{
		first_type = BackSetActDetail::F_ACT_GROWTH_MOUNT;
		this->update_open_activity_value(first_type, request->sub1(), request->sub2());

		first_type = BackSetActDetail::F_ACT_ALL_PEOPLE_GROWTH;
		this->update_open_activity_total_value(first_type, request->sub1(), request->sub2());

		first_type = BackSetActDetail::F_ACT_ALL_PEOPLE_RANK;
		this->update_open_activity_rank(first_type,	request->sub1(), request->sub2(), request->sub3());
		break;
	}
	default:
	{
		MSG_USER("ERROR %d", first_type);
		break;
	}
	}
	return 0;
}

int LogicActivityer::request_activity_buy_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100214*, request, RETURN_OPEN_ACT_BUY);

	int act_index = request->index();
	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item(act_index);
	CONDITION_NOTIFY_RETURN(s_act_t_item != NULL, RETURN_OPEN_ACT_BUY, ERROR_NO_FUN_ACTIVITY);

	uint buy_index = request->buy_index();
	CONDITION_NOTIFY_RETURN(buy_index < s_act_t_item->act_item_set_.size(),
			RETURN_OPEN_ACT_BUY, ERROR_CLIENT_OPERATE);

	int amount = request->amount() > 0 ? request->amount() : 1;

	MLActivityerDetial::ActTypeItem& m_act_t_item = this->find_act_type_item(act_index);
	BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[buy_index];
	if (s_act_t_item->limit_ > 0)
	{
		CONDITION_NOTIFY_RETURN(int(m_act_t_item.sub_value_) + amount <= s_act_t_item->limit_,
				RETURN_OPEN_ACT_BUY, ERROR_ARENA_BUY_LIMIT);
	}
	else if (act_item.times_ > 0)
	{
		MLActivityerDetial::ActItem& m_act_item = m_act_t_item.act_item_map_[buy_index];
		CONDITION_NOTIFY_RETURN(m_act_item.drawed_ + amount <= act_item.times_,
				RETURN_OPEN_ACT_BUY, ERROR_ARENA_BUY_LIMIT);
	}

	int cash_coupon_use = request->cash_coupon_use();
	if (cash_coupon_use > 0)
	{
		CONDITION_NOTIFY_RETURN(act_item.cash_coupon_ * amount >= cash_coupon_use,
				RETURN_OPEN_ACT_BUY, ERROR_CLIENT_OPERATE);
	}

	Proto31400033 inner_res;
	inner_res.set_first_index(act_index);
	inner_res.set_second_index(buy_index);
	inner_res.set_cash_coupon_use(cash_coupon_use);
	inner_res.set_amount(amount);

	for (ItemObjVec::const_iterator iter = act_item.cost_item_.begin();
			iter != act_item.cost_item_.end(); ++iter)
	{
		ProtoItem *cost_item = inner_res.add_cost_item();
		const ItemObj item = *iter;
		item.serialize(cost_item);
		cost_item->set_amount(item.amount_ * amount);
	}

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner_res);
}

int LogicActivityer::request_activity_buy_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400033*, request, -1);

	int first_index = request->first_index();
	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item(first_index);
	CONDITION_NOTIFY_RETURN(s_act_t_item != NULL,RETURN_OPEN_ACT_BUY, ERROR_NO_FUN_ACTIVITY);

	int second_index = request->second_index();
	BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[second_index];
	MLActivityerDetial::ActTypeItem& m_act_t_item = this->find_act_type_item(first_index);
	MLActivityerDetial::ActItem& m_act_item = m_act_t_item.act_item_map_[second_index];

	int amount = request->amount();
	m_act_item.drawed_ += amount;
	if (s_act_t_item->limit_ > 0)
	{
		m_act_t_item.sub_value_ += amount;
	}

	const Json::Value& reward_json = CONFIG_INSTANCE->reward(act_item.reward_id_);
	JUDGE_RETURN(reward_json.empty() == false, 0);

	SerialObj obj(ADD_FROM_RETURN_ACTIVITY_BUY, s_act_t_item->first_type_, s_act_t_item->second_type_);

	RewardInfo reward_info;
	GameCommon::make_up_reward_items(reward_info, reward_json);
	for (ItemObjVec::iterator iter = reward_info.item_vec_.begin();
			iter != reward_info.item_vec_.end(); ++iter)
	{
		this->request_add_item(obj, iter->id_, iter->amount_ * amount, iter->bind_);
	}

//	for (int i = 0; i < amount; ++i)
//		this->request_add_reward(act_item.reward_id_, obj);

	this->fetch_activity_info(first_index);
	FINER_PROCESS_NOTIFY(RETURN_OPEN_ACT_BUY);
}

void LogicActivityer::update_open_activity_force(int force)
{
	this->update_open_activity_value(BackSetActDetail::F_ACT_AIM_CHASE, 7, force);
}

void LogicActivityer::update_open_activity_level(int level)
{
	this->update_open_activity_value(BackSetActDetail::F_ACT_AIM_CHASE, 1, level);
}

void LogicActivityer::update_open_activity_mount(int type, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, ;);

	const Json::Value& conf = CONFIG_INSTANCE->mount_set(type);
	this->update_open_activity_value(BackSetActDetail::F_ACT_GROWTH_MOUNT,
			conf["open_activity"].asInt(), value);
}

void LogicActivityer::update_open_activity_total_recharge(int gold)
{
	JUDGE_RETURN(gold > 0, ;);

	if (LOGIC_OPEN_ACT_SYS->is_open_activity() == true)
		this->update_open_activity_total_recharge_value(BackSetActDetail::F_ACT_FIRST_REC_ALL,
				CONFIG_INSTANCE->open_server_days(), gold);
	else if (LOGIC_OPEN_ACT_SYS->is_combine_activity() == true)
		this->update_open_activity_total_recharge_value(BackSetActDetail::C_ACT_FIRST_REC_ALL,
				CONFIG_INSTANCE->combine_server_days(), gold);
}

void LogicActivityer::update_open_activity_recharge(int gold)
{
	this->update_open_activity_value(BackSetActDetail::F_ACT_ACCU_RECHARGE,
			CONFIG_INSTANCE->open_server_days(), gold);
}

void LogicActivityer::update_open_activity_area_rank()
{
	ArenaRole* area_role = ARENA_SYS->area_role(this->role_id());
	JUDGE_RETURN(area_role != NULL && area_role->rank_ > 0, ;);

	this->update_open_activity_value(BackSetActDetail::F_ACT_AIM_CHASE, 5, -area_role->rank_);
}

void LogicActivityer::update_open_activity_value(int first_type, int day, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(first_type, day);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(value >= act_item.cond_value_[0]);

		m_act_item.act_item_map_[i].arrive_ = true;
		JUDGE_CONTINUE(act_item.drawed_map_.count(this->role_id()) == 0);

		flag = true;
		act_item.drawed_map_[this->role_id()] = 0;
	}

	JUDGE_RETURN(flag == true, ;);
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->validate_open_day(day) == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_open_activity_total_value(int first_type, int day, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(first_type, day);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	Int64 role_id = this->role_id();
	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(value >= act_item.cond_value_[0]);
		JUDGE_CONTINUE(act_item.sub_map_.count(role_id) == 0);

		flag = true;
		act_item.sub_map_[role_id] = 0;
	}

	JUDGE_RETURN(flag == true, ;);
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->validate_open_day(day) == true, ;);

	this->notify_all_player_red_point(s_act_t_item);
//	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_open_activity_total_recharge_value(int first_type, int day, int value)
{
//	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, ;);
	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(first_type, day);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	Int64 role_id = this->role_id();
	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(value > 0);
		JUDGE_CONTINUE(act_item.sub_map_.count(role_id) == 0);

		flag = true;
		act_item.sub_map_[role_id] = 0;
	}

	JUDGE_RETURN(flag == true, ;);

	if (LOGIC_OPEN_ACT_SYS->is_open_activity() == true)
	{
		JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->validate_open_day(day) == true, ;);
	}
	else if (LOGIC_OPEN_ACT_SYS->is_combine_activity() == true)
	{
		JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->validate_combine_day(day) == true, ;);
	}
	else
	{
		return ;
	}
	this->notify_all_player_red_point(s_act_t_item);
//	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_open_activity_rank(int first_type, int day, int value, int tick)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(first_type, day);
	JUDGE_RETURN(s_act_t_item != NULL, ;);
	JUDGE_RETURN(value >= s_act_t_item->start_cond_, ;);

	ThreeObj& self = s_act_t_item->t_sub_map_[this->role_id()];
	self.id_ = this->role_id();
	self.name_ = this->name();
	self.value_ = value;
	self.tick_ = tick;

	s_act_t_item->sort_t_sub_map();
	if (self.sub_ == 1)
	{
		s_act_t_item->first_info_.unserialize(this->role_detail());
	}

	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->validate_open_day(day) == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::notify_open_activity_update(BackSetActDetail::ActTypeItem* s_act_t_item, int force_value)
{
	if (this->has_activity_reward(*s_act_t_item) == true)
	{
		this->inner_notify_assist_event(s_act_t_item->red_event_, 1);
	}
	else if (force_value != -1)
	{
		this->inner_notify_assist_event(s_act_t_item->red_event_, force_value);
	}
}

void LogicActivityer::notify_all_player_red_point(BackSetActDetail::ActTypeItem* s_act_t_item, int force_value)
{
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
	    JUDGE_CONTINUE(player != NULL);
	    player->notify_open_activity_update(s_act_t_item, force_value);
	}
}

void LogicActivityer::act_login_check_red_point()
{
	IntMap& act_list = LOGIC_OPEN_ACT_SYS->fetch_act_list();
	for (IntMap::iterator iter = act_list.begin(); iter != act_list.end(); ++iter)
	{
		BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item(iter->first);
		this->notify_open_activity_update(act_t_item);
	}
}

void LogicActivityer::update_return_activity_mount(int type, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_return_activity() == true, ;);

	const Json::Value& conf = CONFIG_INSTANCE->mount_set(type);
	this->update_return_activity_value(PairObj(BackSetActDetail::F_ACT_R_GROWTH_MOUNT,
			conf["open_activity"].asInt()), value);
}

void LogicActivityer::update_return_activity_recharge(int gold)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_return_activity() == true, ;);

	int second_type = CONFIG_INSTANCE->open_server_days() % 7;
	second_type = second_type == 0 ? 7 : second_type;

	PairObj pair(BackSetActDetail::F_ACT_R_ACCU_RECHARE, second_type);
	this->update_return_activity_value(pair, gold);
}

void LogicActivityer::update_return_activity_value(const PairObj& pair, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_return_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			pair, LOGIC_OPEN_ACT_SYS->cur_open_day());
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(value >= act_item.cond_value_[0]);

		flag = true;
		act_item.drawed_map_[this->role_id()] = 0;
		m_act_item.act_item_map_[i].arrive_ = true;
		JUDGE_CONTINUE(s_act_t_item->redraw_ == true);

		m_act_item.act_item_map_[i].drawed_ = false;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

int LogicActivityer::fetch_festival_activity_list()
{
	Proto50100215 respond;
	respond.set_icon_type(FEST_ACTIVITY_SYS->icon_type());

	IntMap& act_list = FEST_ACTIVITY_SYS->fetch_act_list();
	for (IntMap::iterator iter = act_list.begin(); iter != act_list.end(); ++iter)
	{
		respond.add_act_list(iter->first);
	}

	FINER_PROCESS_RETURN(RETURN_FEST_ACT_INFO, &respond);
}

void LogicActivityer::update_fest_activity_login()
{
	JUDGE_RETURN(FEST_ACTIVITY_SYS->is_activity_time() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = FEST_ACTIVITY_SYS->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_LOGIN);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int flag = 0;
	int total = s_act_t_item->act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(act_item.drawed_map_.count(this->role_id()) == 0);

		if (act_item.cond_value_.size() == 2)
		{
			int combine_time = GameCommon::fetch_cur_combine_time();
			JUDGE_CONTINUE(combine_time >= act_item.cond_value_[0] &&
					combine_time < act_item.cond_value_[1]);
		}

		flag = true;
		m_act_item.act_item_map_[i].arrive_ = true;
		act_item.drawed_map_[this->role_id()] = 0;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_fest_boss_hurt(int reward_index)
{
	JUDGE_RETURN(FEST_ACTIVITY_SYS->is_activity_time() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = FEST_ACTIVITY_SYS->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_BOSS);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int total = s_act_t_item->act_item_set_.size();
	JUDGE_RETURN(reward_index < total, ;);

	BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[reward_index];
	JUDGE_RETURN(act_item.drawed_map_.count(this->role_id()) == 0, ;);

	act_item.drawed_map_[this->role_id()] = 0;
	m_act_item.act_item_map_[reward_index].arrive_ = true;
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_fest_activity_consume_money(int total_money)
{
	JUDGE_RETURN(FEST_ACTIVITY_SYS->is_activity_time() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = FEST_ACTIVITY_SYS->find_fest_activity(
			BackSetActDetail::F_ACT_FEST_CONSUM);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(total_money >= act_item.cond_value_[0]);

		m_act_item.act_item_map_[i].arrive_ = true;
		JUDGE_CONTINUE(act_item.drawed_map_.count(this->role_id()) == 0);

		flag = true;
		act_item.drawed_map_[this->role_id()] = 0;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_combine_activity_login(int recharge)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, ;);

	int value = 1 + recharge;
	int combine_server_days = CONFIG_INSTANCE->combine_server_days();
	this->update_combine_activity_value(BackSetActDetail::C_ACT_LOGIN,
			combine_server_days, value);
}

void LogicActivityer::update_combine_activity_mount(int type, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, ;);

	const Json::Value& conf = CONFIG_INSTANCE->mount_set(type);
	int combine_server_days = CONFIG_INSTANCE->combine_server_days();

	this->update_combine_activity_value(PairObj(BackSetActDetail::C_ACT_GROWTH,
			conf["open_activity"].asInt()), combine_server_days, value);
}

void LogicActivityer::update_combine_activity_recharge(int today_gold, int inc_gold)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, ;);

	int combine_server_days = CONFIG_INSTANCE->combine_server_days();

	// 累充奖励方式一
	this->update_combine_activity_value(PairObj(BackSetActDetail::C_ACT_ACCU_RECHARGE, combine_server_days),
			combine_server_days, today_gold);
	// 累充奖励方式二
	this->update_combine_activity_value_b(PairObj(BackSetActDetail::C_ACT_ACCU_RECHARGE, combine_server_days),
				combine_server_days, inc_gold);
}

void LogicActivityer::update_combine_activity_consum(int inc_gold)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, ;);
	this->update_combine_activity_rank(BackSetActDetail::C_ACT_CONSUM_RANK,
			CONFIG_INSTANCE->combine_server_days(), inc_gold, ::time(NULL));
}

void LogicActivityer::update_combine_activity_value(const PairObj& pair, int day, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(pair, day);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(act_item.handle_type_ == 0);
		JUDGE_CONTINUE(value >= act_item.cond_value_[0]);

		//记录达到条件值
		if (s_act_t_item->record_value_ == true)
		{
			JUDGE_CONTINUE(m_act_item.act_item_map_[i].arrive_map_.count(value) == 0);
			m_act_item.act_item_map_[i].arrive_ += 1;
			m_act_item.act_item_map_[i].arrive_map_[value] = true;
		}
		else
		{
			m_act_item.act_item_map_[i].arrive_ = true;
		}

		//重复领取
		if (s_act_t_item->redraw_ == false)
		{
			JUDGE_CONTINUE(act_item.drawed_map_.count(this->role_id()) == 0);
		}
		else
		{
			m_act_item.act_item_map_[i].drawed_ = false;
		}

		flag = true;
		act_item.drawed_map_[this->role_id()] = 0;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_combine_activity_value_b(const PairObj& pair, int day, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(pair, day);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	this->drawed_detial_.accu_value_map_[pair.id_] += value;
	int total_value = this->drawed_detial_.accu_value_map_[pair.id_];

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(act_item.handle_type_ == BackSetActDetail::HANDLE_TYPE_1);
		JUDGE_CONTINUE(total_value >= act_item.cond_value_[0]);

		// 条件的整数倍处理
		int check_size = total_value / act_item.cond_value_[0];
		m_act_item.act_item_map_[i].arrive_ = check_size;
		JUDGE_CONTINUE(check_size >= 1);

		flag = true;
		act_item.drawed_map_[this->role_id()] = 0;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::update_combine_activity_rank(const PairObj& pair, int day, int value, Int64 sub)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(pair, day);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	ThreeObj& self = s_act_t_item->t_sub_map_[this->role_id()];
	self.id_ 	= this->role_id();
	self.name_ 	= this->name();
	self.value_ += value;
	self.tick_ 	= sub;
	JUDGE_RETURN(self.value_ >= s_act_t_item->start_cond_, ;);

	s_act_t_item->sort_t_sub_map_b();
	this->notify_open_activity_update(s_act_t_item);
}

int LogicActivityer::fetch_self_combine_activity_consum(BackSetActDetail::ActTypeItem* s_act_t_item)
{
	JUDGE_RETURN(s_act_t_item != NULL, 0);

	ThreeObjMap::iterator self_rank_iter = s_act_t_item->t_sub_map_.find(this->role_id());
	JUDGE_RETURN(self_rank_iter != s_act_t_item->t_sub_map_.end(), 0);

	return self_rank_iter->second.value_;
}

int LogicActivityer::fetch_combine_activity_consum_rank(int data_type)
{
	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			BackSetActDetail::C_ACT_CONSUM_RANK, CONFIG_INSTANCE->combine_server_days());
	CONDITION_NOTIFY_RETURN(s_act_t_item !=  NULL, RETURN_REQUEST_FETCH_RANK_DATA, ERROR_CLIENT_OPERATE);

	Proto50100701 respond;
	respond.set_data_type(data_type);
	respond.set_rank_type(RANK_COMBINE_CONSUM_RANK);

	//自己
	ThreeObjMap::iterator self_rank_iter = s_act_t_item->t_sub_map_.find(this->role_id());
	if (self_rank_iter != s_act_t_item->t_sub_map_.end())
	{
		ProtoRankRecord *self_rank_record = respond.mutable_my_rank_record();
		self_rank_record->set_role_id(this->role_id());
		self_rank_record->set_display_content(self_rank_iter->second.name_);
		self_rank_record->set_cur_rank(self_rank_iter->second.sub_);
		self_rank_record->set_rank_value(self_rank_iter->second.value_);
	}

	if (data_type == 0)
	{
		this->activity_rank_line_ = 0;
	}

	int data_num = 10 + this->activity_rank_line_;
	data_num = std::min<int>(s_act_t_item->t_sub_rank_.size(), data_num);

	for (int i = this->activity_rank_line_; i < data_num; ++i)
	{
		ThreeObj &rank_obj = s_act_t_item->t_sub_rank_[i];
		ProtoRankRecord *rank_record = respond.add_rank_record_list();
		rank_record->set_role_id(rank_obj.id_);
		rank_record->set_display_content(rank_obj.name_);
		rank_record->set_cur_rank(rank_obj.sub_);
		rank_record->set_rank_value(rank_obj.value_);
	}

	this->activity_rank_line_ = data_num;
	FINER_PROCESS_RETURN(RETURN_REQUEST_FETCH_RANK_DATA, &respond);
}

void LogicActivityer::update_combine_return_activity_mount(int type, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_return_activity() == true, ;);

	const Json::Value& conf = CONFIG_INSTANCE->mount_set(type);
	this->update_combine_return_activity_value(PairObj(BackSetActDetail::C_ACT_R_GROWTH_MOUNT,
			conf["open_activity"].asInt()), value);
}

void LogicActivityer::update_combine_return_activity_recharge(int gold)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_return_activity() == true, ;);

	int second_type = CONFIG_INSTANCE->combine_server_days() % 7;
	second_type = second_type == 0 ? 7 : second_type;

	PairObj pair(BackSetActDetail::C_ACT_R_ACCU_RECHARE, second_type);
	this->update_combine_return_activity_value(pair, gold);
}

void LogicActivityer::update_combine_return_activity_value(const PairObj& pair, int value)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_return_activity() == true, ;);

	BackSetActDetail::ActTypeItem* s_act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			pair, LOGIC_OPEN_ACT_SYS->cur_combine_day());
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(
			s_act_t_item->act_index_);

	int flag = false;
	int total = s_act_t_item->act_item_set_.size();

	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[i];
		JUDGE_CONTINUE(value >= act_item.cond_value_[0]);

		flag = true;
		act_item.drawed_map_[this->role_id()] = 0;

		m_act_item.act_item_map_[i].arrive_ = true;
		JUDGE_CONTINUE(s_act_t_item->redraw_ == true);

		m_act_item.act_item_map_[i].drawed_ = false;
	}

	JUDGE_RETURN(flag == true, ;);
	this->notify_open_activity_update(s_act_t_item);
}

void LogicActivityer::test_reset_acrivity(int index)
{
	BackSetActDetail::ActTypeItem* act_t_item = this->fetch_activity_by_index(index);
	JUDGE_RETURN(act_t_item != NULL, ;);

	this->drawed_detial_.act_type_item_set_.erase(act_t_item->act_index_);

	int total = act_t_item->act_item_set_.size();
	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = act_t_item->act_item_set_[i];
		act_item.sub_map_.clear();
		act_item.drawed_map_.clear();
	}
}

void LogicActivityer::update_open_activity_cumulative_login()
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, ;);

	int parsed_days = CONFIG_INSTANCE->open_server_days();
	JUDGE_RETURN(parsed_days > 0, ;);

	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			BackSetActDetail::F_ACT_CUMULATIVE_LOGIN, parsed_days);
	JUDGE_RETURN(act_t_item != NULL && act_t_item->act_index_ >= 0, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(act_t_item->act_index_);
	int open_activity_day = act_t_item->act_item_set_.size();
	JUDGE_RETURN(open_activity_day > 0 , ;);


	JUDGE_RETURN(parsed_days <= open_activity_day, ;);

	CumulativeLoginDetail& login_detail = this->role_detail().cur_day_detail_;
	JUDGE_RETURN(login_detail.__cumulative_day < open_activity_day, ;);

	BackSetActDetail::ActItem& cur_day_act_item = act_t_item->act_item_set_[login_detail.__cumulative_day];

	if(m_act_item.update_tick_ == 0)
	{
		login_detail.__cumulative_day++;

		//随机奖励
		cumulative_login_logic(login_detail.__cumulative_day);
		int error = login_detail.check_detail_by_day(login_detail.__cumulative_day);
		if(error < 0)
		{

			MSG_USER("config is error, errorID:%d!", error);
			return;
		}

		login_detail.set_all_state(false);
		m_act_item.update_tick_ = Time_Value::gettimeofday().sec();

		const Json::Value& v_cumulative_login = CONFIG_INSTANCE->cumulative_login(login_detail.__cumulative_day);

		//倍率后的奖励
		int multiple_reward = login_detail.get_multiple_reward();

		ItemObj reward_obj(v_cumulative_login["reward_id"].asInt(), multiple_reward, true);

		//设置活动各奖励的条件
		for(int i = 0; i < open_activity_day; ++i)
		{
			if(cur_day_act_item.cur_index_ == i)
			{
				m_act_item.act_item_map_[i].arrive_ = true;
				act_t_item->act_item_set_[i].cond_value_[0] = true;
				act_t_item->act_item_set_[i].reward_.push_back(reward_obj);
			}
			else
			{
				m_act_item.act_item_map_[i].arrive_ = false;
				act_t_item->act_item_set_[i].cond_value_[0] = false;
			}
		}

		cur_day_act_item.drawed_map_[this->role_id()] = 0;
		JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->validate_open_day(parsed_days) == true, ;);
		this->notify_open_activity_update(act_t_item);

		this->inner_notify_assist_event(act_t_item->red_event_, 1);

		MSG_USER("role id:%lld, hunred:%d, ten:%d, single:%d, multiple:%d, cumulative_day:%d",
				this->logic_player()->role_id(), login_detail.__hundred, login_detail.__ten,
				login_detail.__single, login_detail.__multiple, login_detail.__cumulative_day);
		MSG_USER("hundred_state:%d, ten_state:%d, single_state:%d, multiple_state:%d",
				login_detail.__hundred_state, login_detail.__ten_state, login_detail.__single_state, login_detail.__multiple_state);
	}
	else
	{
		if(m_act_item.act_item_map_[act_t_item->act_index_].drawed_ == 0 && m_act_item.act_item_map_[act_t_item->act_index_].arrive_ > 0)
		{
			++m_act_item.act_item_map_[act_t_item->act_index_].drawed_;
		}
	}
}

void LogicActivityer::cumulative_login_logic(int cumulative_day)
{
	const Json::Value& v_cumulative_login = CONFIG_INSTANCE->cumulative_login(cumulative_day);
	JUDGE_RETURN(!v_cumulative_login.empty(), ; );

	int mu_limit_size = v_cumulative_login["multiple_limit"].size();
	int mu_chance_size = v_cumulative_login["multiple_chance"].size();
	int re_limit_size = v_cumulative_login["reward_limit"].size();
	int re_chance_size = v_cumulative_login["reward_chance"].size();
	JUDGE_RETURN(mu_limit_size == mu_chance_size && mu_limit_size > 0, ;);
	JUDGE_RETURN(re_limit_size == re_chance_size && re_limit_size > 0, ;);


	std::vector<int> multiple_chance, reward_chance;
	for(int i = 0; i<re_chance_size; ++i)
		reward_chance.push_back(v_cumulative_login["reward_chance"][i].asInt());
	for(int i = 0; i < mu_chance_size; ++i)
		multiple_chance.push_back(v_cumulative_login["multiple_chance"][i].asInt());


	int multiple = 0;
	int reward = 0;
	int ZERO = 0;
	Time_Value nowtime = Time_Value::gettimeofday();
	srand(nowtime.sec() + nowtime.usec());

	if(mu_limit_size > 1)
	{
		int chance_index = GameCommon::rand_by_chance(multiple_chance);
		Json::Value chance = v_cumulative_login["multiple_limit"][chance_index];
		if(chance.size() == 1)
		{
			multiple = chance[ZERO].asInt();
		}
		else
		{
			int down = chance[ZERO].asInt();
			int up = chance[1].asInt();
			multiple = rand()%(up - down + 1) + down;
		}

	}
	else if(mu_limit_size == 1)
	{
		Json::Value limit = v_cumulative_login["multiple_limit"][mu_limit_size - 1];
		int limit_count = limit.size();

		if(limit_count == 1)
		{
			multiple = limit[limit_count - 1].asInt();
		}
		else
		{
			 int down = limit[ZERO].asInt();
			 int up = limit[1].asInt();
			 multiple = rand()%(up - down + 1) + down;
		}
	}
	else
	{
		return;
	}

	if(re_limit_size > 1)
	{
		int chance_index = GameCommon::rand_by_chance(reward_chance);
		Json::Value chance = v_cumulative_login["reward_limit"][chance_index];
		if(chance.size() == 1)
		{
			reward = chance[ZERO].asInt();
		}
		else
		{
			 int down = chance[ZERO].asInt();
			 int up = chance[1].asInt();
			 reward = rand()%(up - down + 1) + down;
		}
	}
	else if(re_limit_size == 1)
	{
		Json::Value limit = v_cumulative_login["reward_limit"][re_limit_size - 1];
		int limit_count = limit.size();
		if(limit_count == 1)
		{
			reward = limit[limit_count - 1].asInt();
		}
		else
		{
			 int down = limit[ZERO].asInt();
			 int up = limit[1].asInt();
			 reward = rand()%(up - down + 1) + down;
		}
	}
	else
	{
		return;
	}

	reward = reward / multiple * multiple;
	int except_multiple_reward = reward / multiple;

	if((except_multiple_reward >= 10 && cumulative_day == 1 )||
			(except_multiple_reward >= 100 && cumulative_day == 2 )||
			(except_multiple_reward >= 1000 && cumulative_day >= 3)||
			(except_multiple_reward <= 0))
	{
		MSG_USER("reRand!!!,except_multiple_reward:%d", except_multiple_reward);
		return cumulative_login_logic(cumulative_day);
	}


	CumulativeLoginDetail& login_detail = this->logic_player()->role_detail().cur_day_detail_;
	if(except_multiple_reward >= 100)
	{
		login_detail.__hundred = except_multiple_reward / 100;
		login_detail.__ten =  except_multiple_reward % 100 / 10;
		login_detail.__single = except_multiple_reward % 10;
	}
	else if(except_multiple_reward >= 10)
	{
		login_detail.__ten =  except_multiple_reward % 100 / 10;
		login_detail.__single = except_multiple_reward % 10;
	}
	else
	{
		login_detail.__single = except_multiple_reward % 10;
	}

	login_detail.__multiple = multiple;
}

void LogicActivityer::reset_cumulative_login()
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	JUDGE_RETURN(parsed_days > 0, ;);

	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			BackSetActDetail::F_ACT_CUMULATIVE_LOGIN, parsed_days);
	JUDGE_RETURN(act_t_item != NULL && act_t_item->act_index_ >= 0, ;);

	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(act_t_item->act_index_);
	m_act_item.update_tick_ = 0;
}

int LogicActivityer::is_cornucopia_task_open_time(int task_id)
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	const Json::Value& value = CONFIG_INSTANCE->cornucopia_info(task_id);
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

void LogicActivityer::test_reset_act_update_tick(int act_index)
{
	MLActivityerDetial::ActTypeItem& m_act_item = this->find_act_type_item(act_index);
	m_act_item.update_tick_ = 0;
}

int LogicActivityer::update_cumulative_logic_info_state(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100216*, request, RETURN_OPEN_ACT_FETCH_INFO);
	CONDITION_NOTIFY_RETURN(set_cumulative_login_info_state(request->act_info()) >= 0, RETURN_OPEN_ACT_FETCH_INFO,ERROR_INDEX_OUT_RANGE);
	return this->fetch_activity_info(request->act_index());
}

int LogicActivityer::set_cumulative_login_info_state(int index)
{
	CumulativeLoginDetail& detail = this->role_detail().cur_day_detail_;
	switch(index)
	{
	case GameEnum::MULTIPLY:
	{
		detail.__multiple_state = true;
		break;
	}
	case GameEnum::SINGLE:
	{
		detail.__single_state = true;
		break;
	}
	case GameEnum::TEN:
	{
		detail.__ten_state = true;
		break;
	}
	case GameEnum::HUNDRED:
	{
		detail.__hundred_state = true;
		break;
	}
	case GameEnum::ALL:
	{
		detail.set_all_state(true);
		break;
	}
	default:
	{
		return -1;
	}
	}
	return 0;
}

int LogicActivityer::fetch_cornucopia_msg(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100220*, request, RETURN_FETCH_CORNUCOPIA_INFO);
	int activity_id = request->activity_id();
	return fetch_cornucopia_info(activity_id);
}

int LogicActivityer::fetch_cornucopia_info(int activity_id)
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	int ret = act_t_item != NULL && act_t_item->act_index_ == activity_id && LOGIC_OPEN_ACT_SYS->is_open_activity_id(activity_id);
	Proto50100220 respond;
	respond.set_is_open(ret);
	if(!ret)
	{
		FINER_PROCESS_RETURN(RETURN_FETCH_CORNUCOPIA_INFO, &respond);
	}


	this->refresh_cornucopia_activity_reward_state();
	this->refresh_cornucopia_activity_liveness();

	ServerRecordMap& server_map = act_t_item->server_record_map_;

	int person_gold_plus = 0;
	if (server_map.count(this->role_id()) > 0)
	{
		person_gold_plus = server_map[this->role_id()].reward_mult_;
	}

	//获取记录的任务列表
	for(int i = GameEnum::CORNUCOPIA_TASK_BEGIN; i < GameEnum::CORNUCOPIA_TASK_END; ++i)
	{
		CornucopiaTask *task = fetch_cornucopia_task(i);
		if(task == NULL)
		{
			MSG_USER("task_id:%d is NULL", i);
			continue;
		}

		int is_open = this->is_cornucopia_task_open_time(i);
		if(!is_open)
			continue;
		PActTastList *task_list = respond.add_task_list();
		task_list->set_task_id(task->task_id_);
		task_list->set_total_num(task->total_times);
		task_list->set_left_num(task->completion_times_);
	}

	LogicRoleDetail& role_detail = this->role_detail();
	IntMap& stage_map = role_detail.reward_stage_map_;
	int red_point_tip = false;
	for(IntMap::iterator iter = stage_map.begin(); iter != stage_map.end(); ++iter)
	{
		respond.add_act_reward_item(iter->second);
		if(iter->second == 0)
			red_point_tip = true;
	}

	this->inner_notify_assist_event(act_t_item->red_event_, red_point_tip);
	int person_gold = fetch_cornucopia_person_gold(this->role_id());
	int act_left_time = this->fetch_left_activity_time(activity_id);;
	int today_left_time = GameCommon::next_day();
	//获取全服的聚宝盆金额
	int server_gold = this->fetch_server_recharge();

	respond.set_person_gold(person_gold);
	respond.set_server_gold(server_gold);
	respond.set_activity_left_time(act_left_time);
	respond.set_today_left_time(today_left_time);
	if(person_gold == 0)
		respond.set_person_gold_plus(0);
	else
		respond.set_person_gold_plus(person_gold_plus);

	FINER_PROCESS_RETURN(RETURN_FETCH_CORNUCOPIA_INFO, &respond);
}

//type 1 设置 0 添加
void LogicActivityer::update_cornucopia_activity_value(int task_id, int value, int type)
{
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, 1);
	JUDGE_RETURN(act_t_item != NULL, ;);

	LogicRoleDetail& role_detail = this->role_detail();
	CornucopiaTaskMap &task_map = role_detail.cornucopia_task_map_;
	JUDGE_RETURN(task_id >= GameEnum::CORNUCOPIA_TASK_BEGIN && task_id < GameEnum::CORNUCOPIA_TASK_END, ;);
	CornucopiaTask& task = task_map[task_id];
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

	//参数发生变化，重新推送活动信息
	fetch_cornucopia_info(act_t_item->act_index_);
}

void LogicActivityer::update_cornucopia_activity_recharge(int cur_gold)
{
	JUDGE_RETURN(cur_gold > 0, ;);
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, ;);

	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	JUDGE_RETURN(act_t_item != NULL, ;);

	Int64 role_id = this->role_id();
	ServerRecordMap& server_map = act_t_item->server_record_map_;

	//当前玩家没有记录
	if(server_map.count(role_id) == 0)
	{
		ServerRecord record;
		record.player_id_ = role_id;
		record.player_name_ = this->role_detail().name();
		//todo 添加邮件倍率
		record.reward_mult_ = 0;
		server_map[role_id] = record;
	}

	const Json::Value& value = CONFIG_INSTANCE->open_activity_json();
	std::stringstream ss;
	ss << act_t_item->act_index_;
	const Json::Value& act_info = value[ss.str()];
	const Json::Value& ratio_value = act_info["ratio"];
	int ratio = 0;
	if(ratio_value.empty())
		ratio = 0;
	else
		ratio = ratio_value.asInt();

	//今天充值总金额*返回比例 = 个人聚宝盆金额
	server_map[role_id].cornucopia_gold_ += cur_gold * ratio / 10000;
	server_map[role_id].get_time_ = time(0);

	int server_gold = 0;
	for(ServerRecordMap::const_iterator iter = server_map.begin(); iter != server_map.end(); ++iter)
	{
		server_gold += iter->second.cornucopia_gold_;
	}

	act_t_item->cornucopia_server_gold_ = server_gold;

	//参数发生变化，重新推送活动信息
	this->fetch_cornucopia_info(act_t_item->act_index_);
}

CornucopiaTask* LogicActivityer::fetch_cornucopia_task(int task_id)
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	JUDGE_RETURN(act_t_item != NULL, NULL);

	//任务id必须在配置的范围内
	JUDGE_RETURN(task_id < GameEnum::CORNUCOPIA_TASK_END && task_id >= GameEnum::CORNUCOPIA_TASK_BEGIN, NULL);

	LogicRoleDetail& role_detail = this->role_detail();
	CornucopiaTaskMap &task_map = role_detail.cornucopia_task_map_;
	for(int i = GameEnum::CORNUCOPIA_TASK_BEGIN; i < GameEnum::CORNUCOPIA_TASK_END; ++i)
	{
		if(task_map.count(i) == 0)
		{
			CornucopiaTask task;
			task.task_id_ = i;
			task.completion_times_ = 0;
			task.total_times = CONFIG_INSTANCE->cornucopia_info(i)["condition"].asInt();
			task_map[i] = task;
		}
	}
	return &task_map[task_id];
}


int LogicActivityer::fetch_cornucopia_reward(Message* msg)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, -1;);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100221*, request, RETURN_FETCH_CORNUCOPIA_REWARD);
	int activity_id = request->activity_id();
	CONDITION_NOTIFY_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity_id(activity_id),
			RETURN_FETCH_CORNUCOPIA_REWARD, ERROR_NO_FUN_ACTIVITY);

	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	CONDITION_NOTIFY_RETURN(act_t_item != NULL && act_t_item->act_index_ == activity_id,
			RETURN_FETCH_CORNUCOPIA_INFO, ERROR_NO_FUN_ACTIVITY);

	this->refresh_cornucopia_activity_reward_state();

	LogicRoleDetail& role_detail = this->role_detail();
	IntMap& stage_map = role_detail.reward_stage_map_;
	int index = request->index();
	CONDITION_NOTIFY_RETURN(stage_map.count(index) > 0, RETURN_FETCH_CORNUCOPIA_REWARD, ERROR_INDEX_INVALID);

	if(stage_map[index] == 1)
		CONDITION_NOTIFY_RETURN(false, RETURN_FETCH_CORNUCOPIA_REWARD, ERROR_AWARD_HAS_GET);
	if(stage_map[index] < 0)
		return -1;

	int serial = this->fetch_activity_draw_serial(act_t_item->act_index_);
	SerialObj obj(serial, act_t_item->first_type_, act_t_item->second_type_);

	const Json::Value& reward = CONFIG_INSTANCE->cornucopia_reward(index);
	if(!reward.empty())
	{
		if(stage_map[index] == 0)
		{
			stage_map[index] = 1;
			MSG_USER("get stage_map reward");
			this->request_add_item(obj, reward["item_id"].asInt(), reward["amount"].asInt(), reward["item_bind"].asInt());
		}
	}

	FINER_PROCESS_NOTIFY(RETURN_FETCH_CORNUCOPIA_REWARD);
}


int LogicActivityer::update_cornucopia_activity_value(Message* msg)
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_open_activity() == true, -1;);
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403200*, request, RETURN_OPEN_ACT_FETCH_INFO);
	int task_id = request->task_id();
	int count = request->task_finish_count();
	int type = request->type();
	update_cornucopia_activity_value(task_id, count, type);
	return 0;
}

int LogicActivityer::fetch_server_recharge()
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	JUDGE_RETURN(act_t_item != NULL, -1;);
	return act_t_item->cornucopia_server_gold_;
}

void LogicActivityer::refresh_cornucopia_activity_reward_state()
{
	int server_gold = this->fetch_server_recharge();
	LogicRoleDetail& role_detail = this->role_detail();
	IntMap& stage_map = role_detail.reward_stage_map_;
	GameConfig::ConfigMap& reward_map = CONFIG_INSTANCE->cornucopia_reward();

	int notify_all_player = false;
	for(GameConfig::ConfigMap::const_iterator iter = reward_map.begin(); iter != reward_map.end(); ++iter)
	{
		Json::Value *value = reward_map[iter->first];
		int stage_gold = (*value)["num"].asInt();
		if(stage_map.count(iter->first) == 0)
		{
			stage_map[iter->first] = -1;
		}

		if(server_gold >= stage_gold && stage_map[iter->first] == -1)
		{
			stage_map[iter->first] = 0;
			notify_all_player = true;
		}
	}

	if(notify_all_player)
	{
		LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
		LogicMonitor::PlayerMap::iterator iter = player_map.begin();
		for( ; iter != player_map.end(); ++iter)
		{
			iter->second->fetch_cornucopia_info(10901);
		}
	}
}

int LogicActivityer::fetch_cornucopia_person_gold(Int64 role_id)
{
//	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item(10901);
	JUDGE_RETURN(act_t_item != NULL, -1);
	LogicRoleDetail& role_detail = this->role_detail();
	ServerRecordMap& server_map = act_t_item->server_record_map_;
	ServerRecord record;
	//当前玩家没有记录
	if(server_map.count(role_id) == 0)
	{
		record.player_id_ = role_id;
		record.player_name_ = role_detail.name();
		record.reward_mult_ = 0;
		server_map[role_id] = record;
	}
	record = server_map[role_id];
	return record.cornucopia_gold_;
}

void LogicActivityer::refresh_cornucopia_activity_liveness()
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	JUDGE_RETURN(act_t_item != NULL, ;);
	ServerRecordMap& server_map = act_t_item->server_record_map_;
	Int64 role_id = this->role_id();
	if(server_map.count(role_id) == 0)
		fetch_cornucopia_person_gold(role_id);
	ServerRecord& record = server_map[role_id];

	int total_liveness = 0;
	for(int i = GameEnum::CORNUCOPIA_TASK_BEGIN; i < GameEnum::CORNUCOPIA_TASK_END; ++i)
	{
		if(this->is_cornucopia_task_open_time(i))
		{
			if(this->check_cornucopia_task_finish_state(i))
			{
				const Json::Value& plus_json = CONFIG_INSTANCE->cornucopia_info(i)["plus"];
				if(!plus_json.empty())
				{
					total_liveness += plus_json.asInt();
				}
			}
		}
	}
	record.reward_mult_ = total_liveness;
}

//return 1:finish -1:error 0:begin
int LogicActivityer::check_cornucopia_task_finish_state(int task_id)
{
	CornucopiaTask *task = fetch_cornucopia_task(task_id);
	if(task == NULL && task->total_times > 0)
		return -1;
	if(task->completion_times_ == task->total_times)
		return 1;
	return 0;
}

int LogicActivityer::fetch_cornucopia_mail_reward()
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	this->refresh_cornucopia_activity_liveness();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	if(act_t_item == NULL)
	{
		act_t_item = LOGIC_OPEN_ACT_SYS->find_item(10901);
		if(act_t_item == NULL)
			return -1;
		if(act_t_item->open_time_.count(parsed_days - 1) == 0)
			return -1;
	}

	BackSetActDetail::ActItemSet& act_item = act_t_item->act_item_set_;
	JUDGE_RETURN(act_item.size() > 0, -1);

	ServerRecordMap& server_map = act_t_item->server_record_map_;
	JUDGE_RETURN(server_map.count(this->role_id()) > 0, 0);

	ServerRecord& record = server_map[this->role_id()];

	int recharge_gold = this->fetch_cornucopia_person_gold(this->role_id());
	int mail_gold = recharge_gold + recharge_gold * record.reward_mult_ / 10000;
	MSG_USER("mail_gold:%d, recharge_gold:%d, record_mult:%d", mail_gold, recharge_gold, record.reward_mult_);
	if(mail_gold > 0)
	{
		MailInformation* mail_info = GameCommon::create_sys_mail(act_t_item->mail_id_);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), recharge_gold, record.reward_mult_ * 100 / 10000, mail_gold);
		mail_info->add_goods(act_item[0].reward_id_, mail_gold, true);
		GameCommon::request_save_mail_content(this->role_id(), mail_info);
	}
	server_map.erase(this->role_id());
	return 0;
}

int LogicActivityer::reset_cornucopia_info()
{
	LogicRoleDetail& role_detail = this->role_detail();
	role_detail.cornucopia_task_map_.clear();
	role_detail.mail_reward_ratio_ = 0;
	role_detail.reward_stage_map_.clear();
	return 0;
}


int LogicActivityer::update_cornucopia_activity_recharge(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30103200*, request, RETURN_FETCH_CORNUCOPIA_INFO);
	int add_gold = request->add_gold();
	this->update_cornucopia_activity_recharge(add_gold);
	return 0;
}
