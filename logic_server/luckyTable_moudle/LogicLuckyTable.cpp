/*
 * LogicLuckyTable.cpp
 *
 *  Created on: Mar 23, 2016
 *      Author: zhangshaopeng
 */

#include "LucktTableMonitor.h"
#include "LogicLuckyTable.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "GameConfig.h"

void LuckyTableDetial::reset()
{
	this->left_times[0] = 0;
	this->left_times[1] = 0;
	this->exec_times[0] = 0;
	this->exec_times[1] = 0;
	this->gold[0] = 0;
	this->gold[1] = 0;
	this->activity_key = 0;
}


LogicLuckyTable::LogicLuckyTable()
{
	this->reset();
}

LogicLuckyTable::~LogicLuckyTable()
{
}

void LogicLuckyTable::reset()
{
	this->detail_.reset();
}

LuckyTableDetial& LogicLuckyTable::fetch_ltable_detail()
{
	return this->detail_;
}

int LogicLuckyTable::open_ltable(Message *msg)
{
	CONDITION_NOTIFY_RETURN(LACKY_TABLE->open_state() > 0,RETURN_OPERATOR_LUCKY_TABLE,ERROR_CLIENT_OPERATE);
	DYNAMIC_CAST_RETURN(Proto10101801 *, request, msg, -1);
	int type = request->type();
	CONDITION_NOTIFY_RETURN(type >= 0 && type < 2,RETURN_OPERATOR_LUCKY_TABLE,ERROR_CLIENT_OPERATE);
	this->check_and_reset_ltable();

	Int64 left_tick = LACKY_TABLE->fetch_left_time(type);
	if(left_tick < 0)
	{
		this->respond_to_client_error(RETURN_OPEN_LUCKY_TABLE,ERROR_ACTIVITY_TIME_INVALID);
		return this->notify_ltable_state();
	}

	const LuckyTableConfig& cfg = LACKY_TABLE->get_ltable_cfg();
	Proto50101801 respond;
	respond.set_left_time(this->detail_.left_times[type]);
	respond.set_start_tick(cfg.start_tick[type]);
	respond.set_end_tick(cfg.end_tick[type]);
	respond.set_left_tick(left_tick);
	ItemObj obj;
	for(uint i = 0; i < cfg.rewrad_table[type][0].size(); ++i)
	{
		obj.amount_ = cfg.rewrad_table[type][0][i].item_num;
		obj.bind_ = cfg.rewrad_table[type][0][i].item_bind;
		obj.id_ = cfg.rewrad_table[type][0][i].item_id;
		ProtoItem* item = respond.add_item_list();
		obj.serialize(item);
	}
	respond.set_type(type);
	respond.add_cost(cfg.cost[type][0]);
	respond.add_cost(cfg.cost[type][1]);
	respond.add_cost(cfg.cost[type][2]);
	respond.set_baseline(cfg.base[type]);

	LACKY_TABLE->register_role(this->role_id(),type);
	return this->respond_to_client(RETURN_OPEN_LUCKY_TABLE, &respond);
}

int LogicLuckyTable::close_ltable()
{
	LACKY_TABLE->register_role(this->role_id(),-1);
	return 0;
}

int LogicLuckyTable::exec_ltable(Message *msg)
{
	CONDITION_NOTIFY_RETURN(LACKY_TABLE->open_state() > 0,RETURN_OPERATOR_LUCKY_TABLE,ERROR_CLIENT_OPERATE);
	DYNAMIC_CAST_RETURN(Proto10101802 *, request, msg, -1);
	int type1 = request->type1();
	int type2 = request->type2();
	CONDITION_NOTIFY_RETURN(type1 >= 0 && type1 < 2,RETURN_OPERATOR_LUCKY_TABLE,ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(type2 >= 0 && type2 < 3,RETURN_OPERATOR_LUCKY_TABLE,ERROR_CLIENT_OPERATE);
	this->check_and_reset_ltable();
	if(LACKY_TABLE->fetch_left_time(type1) < 0)
	{
		this->respond_to_client_error(RETURN_OPERATOR_LUCKY_TABLE,ERROR_ACTIVITY_TIME_INVALID);
		return this->notify_ltable_state();
	}

	int times[] = {1,10,50};
	CONDITION_NOTIFY_RETURN(this->detail_.left_times[type1] >= times[type2],RETURN_OPERATOR_LUCKY_TABLE,ERROR_LIMIT_TIMES);

	std::vector<int> reward_index_list;
	std::vector<ItemObj> reward_item_list;
	RateSet rate_set;
	int ret = 0;
	for(int i = 0; i < times[type2]; ++i)
	{
		if(this->detail_.left_times[type1] <= 0)
		{
			ret = ERROR_LIMIT_TIMES;
			break;
		}
		this->calc_rate_set(type1,rate_set);
		ret = this->gen_reward_item(rate_set);
		JUDGE_BREAK(ret >= 0);

		int item_id = rate_set[ret].item_id,
			item_num = rate_set[ret].item_num,
			item_bind = rate_set[ret].item_bind;
		reward_item_list.push_back(ItemObj(item_id,item_num,item_bind));
		--this->detail_.left_times[type1];
		++this->detail_.exec_times[type1];
		MSG_USER("id:%ld,left_times:%d,exec_times:%d,item_id:%d",this->role_id(),this->detail_.left_times[type1],this->detail_.exec_times[type1],item_id);
		reward_index_list.push_back(LACKY_TABLE->fetch_item_index(type1,item_id));
	}

	if(ret < 0)
		this->respond_to_client_error(RETURN_OPERATOR_LUCKY_TABLE, ret);
	if(!reward_item_list.empty())
	{
		Proto31400039 req;
		for(uint i = 0; i < reward_item_list.size(); ++i)
		{
			ProtoItem *item = req.add_item();
			reward_item_list[i].serialize(item);
			req.add_index(reward_index_list[i]);
		}
		req.set_type1(type1);
		req.set_type2(type2);
		int cost = LACKY_TABLE->get_ltable_cfg().cost[type1][type2];
		if(cost > 0)
		{
			req.set_cost(cost);
			return LOGIC_MONITOR->dispatch_to_scene(this,&req);
		}
		else
		{
			req.set_result(1);
			return this->exec_ltable_after(&req);
		}
	}

	return 0;
}

int LogicLuckyTable::exec_ltable_after(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31400039 *, request, msg, -1);
	if(request->result() == 0)
	{
		this->detail_.left_times[request->type1()] += request->item_size();
		this->detail_.exec_times[request->type1()] -= request->item_size();
		MSG_USER("lucky_table failed:%ld",this->role_id());
		return 0;
	}

	this->record_and_send_reward(request);
	Proto50101802 respond;
	respond.set_type1(request->type1());
	respond.set_type2(request->type2());
	respond.set_left_time(this->detail_.left_times[request->type1()]);
	for(int i = 0; i < request->index_size(); ++i)
	{
		respond.add_item_index_list(request->index(i));
	}
	return this->respond_to_client(RETURN_OPERATOR_LUCKY_TABLE, &respond);
}

int LogicLuckyTable::calc_rate_set(int type,RateSet& rate_set)
{
	rate_set.clear();
	const LuckyTableConfig& cfg = LACKY_TABLE->get_ltable_cfg();
	uint index = 0;
	if(cfg.play_time[type] > 0 && cfg.play_table[type].size() != 0)
	{
		if(this->detail_.exec_times[type] > 0 && this->detail_.exec_times[type] % cfg.play_time[type] == 0)
		{
			rate_set = cfg.play_table[type];
			return 0;
		}
	}
	if(!cfg.dis_play_time[type].empty())
	{
		while(index < cfg.dis_play_time[type].size() && this->detail_.exec_times[type] >= cfg.dis_play_time[type][index])
			++index;
	}
	if(index < cfg.rewrad_table[type].size())
		rate_set = cfg.rewrad_table[type][index];
	else
		rate_set = cfg.rewrad_table[type].back();

	return 0;
}

int LogicLuckyTable::gen_reward_item(const RateSet& rate_set)
{
	IntMap map_rate;
	int rate_sum = 0;
	for(uint i = 0; i < rate_set.size(); ++i)
	{
		rate_sum += rate_set[i].item_weight;
	}

	JUDGE_RETURN(rate_sum > 0,ERROR_CONFIG_ERROR);
	int rand_num = std::rand() % rate_sum, tmp = 0;
	for(uint i = 0; i < rate_set.size(); ++i)
	{
		tmp += rate_set[i].item_weight;
		if(rand_num < tmp)
		{
			return i;
		}
	}
	return ERROR_CONFIG_ERROR;
}

void LogicLuckyTable::record_and_send_reward(Message *msg)
{
//	DYNAMIC_CAST_RETURN(Proto31400039 *, request, msg,);
//
//	Proto31400029 req;
//	SerialObj serial(ITEM_LUCKY_TABLE_ADD);
//	serial.serialize(req.mutable_serial());
//
//	std::vector<BoxRecord> record_list;
//	BoxRecord record;
//	record.__role_id = this->role_id();
//	record.__name = this->role_detail().__name;
//
//	const LuckyTableConfig& cfg = LACKY_TABLE->get_ltable_cfg();
//	string table_name = CONFIG_INSTANCE->tiny("lucky_table")[uint(request->type1())].asString();
//
//	for(int i = 0; i < request->item_size(); ++i)
//	{
//		ProtoItem* item = req.add_item_list();
//		*item = request->item(i);
//
//		if(cfg.best_item.count(request->item(i).id()))
//		{
//			BrocastParaVec para_vec;
//			GameCommon::push_brocast_para_role_detail(para_vec,this->role_id(), std::string(this->name()), false);
//			GameCommon::push_brocast_para_string(para_vec,table_name);
//			GameCommon::push_brocast_para_item(para_vec, request->item(i).id(),request->item(i).bind());
//			LOGIC_MONITOR->announce_world(SHOUT_ALL_LUCKY_TABLE, para_vec);
//
//			record.__item_amount = request->item(i).amount();
//			record.__item_id = request->item(i).id();
//			record_list.push_back(record);
//		}
//	}
//
//	LACKY_TABLE->insert_ltable_record(record_list,request->type1());
//	LOGIC_MONITOR->dispatch_to_scene(this,&req);
}

int LogicLuckyTable::add_ltable_times(Message *msg)
{
	JUDGE_RETURN(LACKY_TABLE->open_state() > 0,0);
	DYNAMIC_CAST_RETURN(Proto30100109 *, request, msg, -1);
	this->check_and_reset_ltable();
	int type = request->type();
	Int64 left_tick = LACKY_TABLE->fetch_left_time(type);
	JUDGE_RETURN(left_tick > 0,0);

	int value = request->value();
	this->detail_.gold[type] += value;

	const LuckyTableConfig& cfg = LACKY_TABLE->get_ltable_cfg();
	while(this->detail_.gold[type] >= cfg.base[type])
	{
		++this->detail_.left_times[type];
		this->detail_.gold[type] -= cfg.base[type];
		MSG_USER("id:%ld,left_times:%d,gold[%d]:%d",this->role_id(),this->detail_.left_times[type],type,value);
	}
	return 0;
}

int LogicLuckyTable::notify_ltable_state()
{
	JUDGE_RETURN(LACKY_TABLE->open_state() != 0,0);
	Proto81400912 notify;
	if(LACKY_TABLE->fetch_left_time(0) > 0)
		notify.add_state(1);
	else
		notify.add_state(0);

	if(LACKY_TABLE->fetch_left_time(1) > 0)
		notify.add_state(1);
	else
		notify.add_state(0);

	return this->respond_to_client(ACTIVE_LUCKY_TABLE_STATE, &notify);
}

void LogicLuckyTable::check_and_reset_ltable()
{
	for(int i = 0; i < 2; ++i)
	{
		if(LACKY_TABLE->fetch_left_time(i) <= 0)
		{
			this->detail_.exec_times[i] = 0;
			this->detail_.left_times[i] = 0;
			this->detail_.gold[i] = 0;
		}
	}
	int activity_id = LACKY_TABLE->get_activity_key();
	if(this->detail_.activity_key != activity_id && activity_id != 0)
	{
		for(int i = 0; i < 2; ++i)
		{
			this->detail_.exec_times[i] = 0;
			this->detail_.left_times[i] = 0;
			this->detail_.gold[i] = 0;
		}
		this->detail_.activity_key = activity_id;
	}
}



