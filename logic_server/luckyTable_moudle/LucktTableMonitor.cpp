/*
 * LucktTableMonitor.cpp
 *
 *  Created on: Mar 23, 2016
 *      Author: zhangshaopeng
 */

#include "LucktTableMonitor.h"
#include "LogicMonitor.h"
#include "BackGameSwitcher.h"
#include "ProtoDefine.h"
#include "LogicPlayer.h"
#include "GameField.h"

LuckyTableConfig::LuckyTableConfig()
{
	this->reset();
}

void LuckyTableConfig::reset()
{
	this->activity_key = 0;
	this->open_state = 0;
	this->time_state[0] = 0;
	this->time_state[1] = 0;
	this->start_tick[0] = 0;
	this->start_tick[1] = 0;
	this->end_tick[0] = 0;
	this->end_tick[1] = 0;
	this->base[0] = 0;
	this->base[1] = 0;
	this->best_item.clear();
	this->play_time[0] = 0;
	this->play_time[1] = 0;
	this->dis_play_time[0].clear();
	this->dis_play_time[1].clear();

	this->rewrad_table[0].clear();
	this->rewrad_table[1].clear();
	this->play_table[0].clear();
	this->play_table[1].clear();
	this->cost[0].clear();
	this->cost[1].clear();
	this->reward_item_set[0].clear();
	this->reward_item_set[1].clear();
}

LuckyTableConfig& LuckyTableConfig::operator=(const LuckyTableConfig& rhs)
{
	for(int i = 0; i < 2; ++i)
	{
		this->start_tick[i] = rhs.start_tick[i];
        this->end_tick[i] = rhs.end_tick[i];
        this->base[i] = rhs.base[i];
        this->play_time[i] = rhs.play_time[i];
        this->dis_play_time[i] = rhs.dis_play_time[i];
        this->rewrad_table[i] = rhs.rewrad_table[i];
    	this->play_table[i] = rhs.play_table[i];
    	this->cost[i] = rhs.cost[i];
    	this->reward_item_set[i] = rhs.reward_item_set[i];
	}

	this->activity_key = rhs.activity_key;
	this->open_state = rhs.open_state;
	this->best_item = rhs.best_item;
	return *this;
}

LucktTableMonitor::LucktTableMonitor()
{
	this->reset();
}

void LucktTableMonitor::reset()
{
	this->ltable_cfg_.reset();
	this->view_list_[0].clear();
	this->view_list_[1].clear();
	this->record_list_[0].clear();
	this->record_list_[1].clear();
}

void LucktTableMonitor::refresh_from_db(Int64 tick)
{
	BackGameModify modify;
	modify.is_update = 0;
	BackGameSwitcher::get_lucky_table(modify);
	if(this->ltable_cfg_.activity_key != modify.ltable.activity_id && modify.is_update)
		this->update_cfg(&modify);
}

Int64 LucktTableMonitor::fetch_left_time(int type)
{
	Time_Value now = Time_Value::gettimeofday();
	int start_tick = this->ltable_cfg_.start_tick[type] - now.sec();
	int end_tick = this->ltable_cfg_.end_tick[type] - now.sec();
	if(start_tick <= 0 && end_tick > 0)
		return end_tick;
	else
		return -1;
}

void LucktTableMonitor::register_role(Int64 role_id, int type)
{
	JUDGE_RETURN(this->fetch_left_time(type) > 0 && this->ltable_cfg_.open_state != 0,);
	if(type >= 0)
	{
		this->view_list_[type].insert(role_id);
		this->view_list_[1 - type].erase(role_id);
		this->notify_table_record(role_id,type);
	}
	else
	{
		this->view_list_[0].erase(role_id);
		this->view_list_[1].erase(role_id);
	}
}

void LucktTableMonitor::insert_ltable_record(std::vector<BoxRecord>& new_record, int type)
{
	JUDGE_RETURN(this->fetch_left_time(type) > 0 && this->ltable_cfg_.open_state != 0,);
	uint size = new_record.size();
	for(uint i = 0; i < size; ++i)
	{
		BoxRecord *record = LOGIC_MONITOR->box_record_pool()->pop();
		JUDGE_RETURN(NULL != record,);

		record->__role_id = new_record[i].__role_id;
		record->__item_id = new_record[i].__item_id;
		record->__item_amount = new_record[i].__item_amount;
		record->__time = ::time(NULL);
		record->__name = new_record[i].__name;

		if(this->record_list_[type].size() >= BOX_MAX_RECORD_MAX_COUNT)
		{
			this->delete_record_list(type,BOX_MAX_RECORD_COUNT);
		}
		this->record_list_[type].push_back(record);
	}
	this->notify_all_table_record(type);
}

void LucktTableMonitor::notify_all_table_record(int type)
{
	Proto81400911 respond;
	respond.set_type(type);

	int i = 0;
	for(BoxRecordList::reverse_iterator iter = this->record_list_[type].rbegin();
			iter != this->record_list_[type].rend() && i < BOX_MAX_RECORD_COUNT; ++iter,++i)
	{
		ProtoDivineRecord* proto_record = respond.add_record_list();

		proto_record->set_role_id((*iter)->__role_id);
		proto_record->set_item_id((*iter)->__item_id);
		proto_record->set_item_amount((*iter)->__item_amount);
		proto_record->set_role_name((*iter)->__name);
	}

	LongVec invalid_player_set;
	for(LongSet::iterator iter = this->view_list_[type].begin();
			iter != this->view_list_[type].end(); ++iter)
	{
		LogicPlayer* player = NULL;
		int ret = LOGIC_MONITOR->find_player(*iter, player);

		if(ret != 0 || NULL == player)
		{
			invalid_player_set.push_back(*iter);
		}
		else
		{
			player->respond_to_client(ACTIVE_ALL_LUCKY_TABLE_RECORD, &respond);
		}
	}

	for(LongVec::iterator iter = invalid_player_set.begin();
			iter != invalid_player_set.end(); ++iter)
	{
		this->view_list_[type].erase(*iter);
	}
}

void LucktTableMonitor::notify_table_record(Int64 role_id,int type)
{
	LogicPlayer *player = NULL;
	JUDGE_RETURN(0 == LOGIC_MONITOR->find_player(role_id, player),);
	JUDGE_RETURN(NULL != player,);

	Proto81400911 respond;
	respond.set_type(type);
	int i = 0;
	for(BoxRecordList::reverse_iterator iter = this->record_list_[type].rbegin();
			iter != this->record_list_[type].rend() && i < BOX_MAX_RECORD_COUNT; ++iter,++i)
	{
		ProtoDivineRecord* proto_record = respond.add_record_list();

		proto_record->set_role_id((*iter)->__role_id);
		proto_record->set_item_id((*iter)->__item_id);
		proto_record->set_item_amount((*iter)->__item_amount);
		proto_record->set_role_name((*iter)->__name);
	}
	player->respond_to_client(ACTIVE_ALL_LUCKY_TABLE_RECORD, &respond);
}

void LucktTableMonitor::notify_all_activity_state(int state1,int state2)
{
	LogicMonitor::PlayerMap::iterator iter = LOGIC_MONITOR->player_map().begin();
	Proto81400912 notify;
	notify.add_state(state1);
	notify.add_state(state2);
	for(;iter != LOGIC_MONITOR->player_map().end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		player->respond_to_client(ACTIVE_LUCKY_TABLE_STATE, &notify);
	}
}


const LuckyTableConfig& LucktTableMonitor::get_ltable_cfg()
{
	return this->ltable_cfg_;
}

Int64 LucktTableMonitor::get_activity_key()
{
	return this->ltable_cfg_.activity_key;
}

int LucktTableMonitor::open_state()
{
	return this->ltable_cfg_.open_state;
}


int LucktTableMonitor::update_cfg(BackGameModify* game_modify)
{
	const string& json_str = game_modify->value_str;
	JUDGE_RETURN(!json_str.empty(),0);
	Json::Reader reader;
	Json::Value ltable_js;
	if(reader.parse(json_str,ltable_js) == false)
	{
		return -1;
	}
	Int64 now_tick = Time_Value::gettimeofday().sec();
	LuckyTableConfig cfg_tmp;
	string filed1[] = {"consume_table","recharge_table"};
	string filed2[] = {"consume","recharge"};

	for(uint i = 0; i < 2; i++)
	{
		for(int j = 0; j < 2; ++j)
		{
			if(i == 0)
				cfg_tmp.start_tick[j] = game_modify->ltable.start_tick[j];
			else
				cfg_tmp.end_tick[j] = game_modify->ltable.end_tick[j];
		}
	}

	for(uint i = 0; i < 2; ++i)
	{
		if(cfg_tmp.end_tick[i] > now_tick)
		{
			RateSet rate_set;
			for(uint j = 0; j < ltable_js[filed1[i]].size(); ++j)
			{
				//基础库
				const Json::Value& rate_set_js = ltable_js[filed1[i]][j];
				JUDGE_RETURN(this->construct_item_from_json(rate_set_js,rate_set) == 0,-4);
				cfg_tmp.rewrad_table[i].push_back(rate_set);
				if(cfg_tmp.reward_item_set[i].empty())
				{
					for(uint k = 0; k < rate_set.size(); ++k)
					{
						cfg_tmp.reward_item_set[i].insert(std::make_pair(rate_set[k].item_id,k));
					}
				}
			}

			//多少抽一次
			cfg_tmp.base[i] = ltable_js[filed2[i]].asInt();

			{
				cfg_tmp.play_time[i] = ltable_js["play_time"][i].asInt();
				if(cfg_tmp.play_time[i] > 0)
				{
					//保底库
					const Json::Value& rate_set_js = ltable_js["play_table"][i];
					JUDGE_RETURN(this->construct_item_from_json(rate_set_js,cfg_tmp.play_table[i]) == 0,-5);
				}
			}

			{
				//反保底次数
				for(uint j = 0; j < 3; ++j)
				{
					cfg_tmp.dis_play_time[i].push_back(ltable_js["dis_play_time"][i][j].asInt());
				}
			}
			JUDGE_RETURN(cfg_tmp.rewrad_table[i].size() > 0,-6);
			{
				//保底消费次数
				for(uint j = 0; j < 3; ++j)
				{
					cfg_tmp.cost[i].push_back(ltable_js["cost"][i][j].asInt());
				}
			}
		}
	}

	//广播库
	for(uint i = 0; i < ltable_js["best_item"].size(); ++i)
	{
		cfg_tmp.best_item.insert(ltable_js["best_item"][i].asInt());
	}
	cfg_tmp.activity_key = game_modify->ltable.activity_id;
	cfg_tmp.open_state = game_modify->ltable.open_state;
	if(cfg_tmp.open_state == 0 && this->ltable_cfg_.open_state == 1)
	{
		this->ltable_cfg_.time_state[0] = 0;
		this->ltable_cfg_.time_state[1] = 0;
		this->notify_all_activity_state(0,0);
	}
	this->ltable_cfg_ = cfg_tmp;
	MSG_USER("consume start:%ld,end:%ld",this->ltable_cfg_.start_tick[0],this->ltable_cfg_.end_tick[0]);
	MSG_USER("recharge start:%ld,end:%ld",this->ltable_cfg_.start_tick[1],this->ltable_cfg_.end_tick[1]);
	MSG_USER("openstate:%d",this->ltable_cfg_.open_state);

	return 0;
}

int LucktTableMonitor::construct_item_from_json(const Json::Value& rate_set_js,RateSet& rate_set)
{
	JUDGE_RETURN(rate_set_js[0u].size() == 4,-1);
	rate_set.clear();
	ItemRate item_rate;
	for(uint j = 0; j < rate_set_js.size(); ++j)
	{
		item_rate.item_id = rate_set_js[j][0u].asInt();
		item_rate.item_num = rate_set_js[j][1u].asInt();
		item_rate.item_bind = rate_set_js[j][2u].asInt();
		item_rate.item_weight = rate_set_js[j][3u].asDouble();
		rate_set.push_back(item_rate);
	}
	return 0;
}

void LucktTableMonitor::check_and_reset(Int64 tick)
{
	bool notify = false;
	if(this->ltable_cfg_.open_state == 1)
	{
		for(int i = 0; i < 2; ++i)
		{
			if(this->ltable_cfg_.time_state[i] == 1 && this->fetch_left_time(i) <= 0)
			{
				notify = true;
				this->ltable_cfg_.time_state[i] = 0;
				this->view_list_[i].clear();
				this->delete_record_list(i,0);
			}
			else if(this->ltable_cfg_.time_state[i] == 0 && this->fetch_left_time(i) > 0)
			{
				notify = true;
				this->ltable_cfg_.time_state[i] = 1;
			}
		}
	}
	if(notify)
	{
		this->notify_all_activity_state(this->ltable_cfg_.time_state[0],this->ltable_cfg_.time_state[1]);
	}
	this->refresh_from_db(tick);
}

void LucktTableMonitor::delete_record_list(int type,uint left)
{
	while(this->record_list_[type].size() > left)
	{
		BoxRecord* recycle_record = this->record_list_[type].back();
		this->record_list_[type].pop_front();
		LOGIC_MONITOR->box_record_pool()->push(recycle_record);
	}
}

int LucktTableMonitor::fetch_item_index(int type,int item_id)
{
	if(this->ltable_cfg_.reward_item_set[type].count(item_id))
	{
		return this->ltable_cfg_.reward_item_set[type][item_id];
	}
	return 0;
}
