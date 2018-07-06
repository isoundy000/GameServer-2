/*
 * MLBackDailyRechargeSys.cpp
 *
 *  Created on: Nov 5, 2014
 *      Author: jinxing
 */

#include "MLBackDailyRechargeSys.h"
#include "PoolMonitor.h"
#include "GameHeader.h"
#include "BackField.h"
#include <mongo/client/dbclient.h>
#include "TransactionMonitor.h"
#include "MapMonitor.h"
#include "MongoDataMap.h"
#include "Transaction.h"
#include "MongoData.h"
#include "PlayerManager.h"
#include "MapLogicPlayer.h"
#include "TQueryCursor.h"
#include "ProtoDefine.h"
#include "MMORechargeRankConfig.h"

MLBackDailyRechargeSys::MLBackDailyRechargeSys():
start_time_(0),end_time_(0),
activity_opening_(false)
{
	// TODO Auto-generated constructor stub

}

MLBackDailyRechargeSys::~MLBackDailyRechargeSys()
{
	// TODO Auto-generated destructor stub
}

int MLBackDailyRechargeSys::MailTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int MLBackDailyRechargeSys::MailTimer::handle_timeout(const Time_Value &tv)
{
	return BackDR_SYS->send_mail_to_player();
}

int MLBackDailyRechargeSys::get_json_id(Int64 role_id)
{
	int id = 0;
	int open_day = CONFIG_INSTANCE->open_server_days() - 1;
	open_day = open_day > 0 ? open_day : 1;

	if (this->daily_map_.count(role_id) == 0)
	{
		return -1;
	}
	DailyRechargeDetail &detail = this->daily_map_[role_id];
	if (detail.__first_recharge_gold < CONFIG_INSTANCE->daily_recharge_json(1)["recharge_times"].asInt())
	{
		id = 1;
	}
	else
	{
		int size = CONFIG_INSTANCE->daily_recharge_json().size() + 1;
		for (int i = 2; i < size; ++i)
		{
			const Json::Value &dr_json = CONFIG_INSTANCE->daily_recharge_json(i);
			if (dr_json["open_day"].asInt() <= open_day)
			{
				id = i;
			}
		}
	}
	return id;
}

DailyMap &MLBackDailyRechargeSys::daily_map()
{
	return this->daily_map_;
}

int MLBackDailyRechargeSys::get_reward_id(Int64 role_id, int type)
{
	int reward_id = 0;
	int id = this->get_json_id(role_id);
	JUDGE_RETURN(id != -1, 0);

	int open_day = CONFIG_INSTANCE->open_server_days() - 1;
	open_day = open_day > 0 ? open_day : 1;

	const Json::Value &dr_json = CONFIG_INSTANCE->daily_recharge_json(id);
	int week_num = open_day % dr_json["first_recharge"].size();
	if (id == 1) week_num = 0;

	switch(type)
	{
	case 0:
		reward_id = dr_json["first_recharge"][week_num].asInt();
		break;
	case 1:
		reward_id = dr_json["total_recharge"][week_num].asInt();
		break;
	case 2:
		reward_id = dr_json["ext_award"][week_num].asInt();
		break;
	}
	return reward_id;
}

int MLBackDailyRechargeSys::update_player_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400060*, request, -1);

	DailyRechargeDetail &role_detail = this->daily_map_[request->role_id()];
	role_detail.__first_recharge_gold = request->first_recharge_gold();
	role_detail.__last_recharge_time = request->last_recharge_time();
	role_detail.__today_recharge_gold = request->today_recharge_gold();
	role_detail.__act_recharge_times = request->act_recharge_times();
	role_detail.__act_has_mail = request->act_has_mail();
	for (int i = 0; i < request->reward_size(); ++i)
		role_detail.__daily_recharge_rewards[i] = request->reward(i);

	return 0;
}

int MLBackDailyRechargeSys::send_mail_to_player(Int64 player_id)
{
	for (DailyMap::iterator it = this->daily_map_.begin(); it != this->daily_map_.end(); ++it)
	{
		Proto30103001 reward_info;

		Int64 role_id = it->first;
		JUDGE_CONTINUE(it->first == player_id);

		int mail_id = CONFIG_INSTANCE->daily_recharge_json(1)["mail_id"].asInt();

		for (int i = 0; i < MLDailyRecharge::TYPE_NUM; ++i)
		{
			if (it->second.__daily_recharge_rewards[i] == MLDailyRecharge::REWARD_HAVE)
			{
				int reward_id = this->get_reward_id(role_id, i);
				it->second.__daily_recharge_rewards[i] = MLDailyRecharge::REWARD_GONE;
				reward_info.add_reward_list(reward_id);
			}
		}
		reward_info.set_act_type(mail_id);
		reward_info.set_mail_id(mail_id);
		reward_info.set_role_id(role_id);

		MAP_MONITOR->dispatch_to_logic(&reward_info);

		MapLogicPlayer* player = MAP_MONITOR->find_logic_player(role_id);
		if (player != NULL)
		{
			IntMap& temp_map = player->daily_recharge_dtail().__daily_recharge_rewards;
			for (int i = 0; i < MLDailyRecharge::TYPE_NUM; ++i)
			{
				temp_map[i] = it->second.__daily_recharge_rewards[i];
			}
			player->notify_daily_recharge_info();
		}
	}

	return 0;
}

int MLBackDailyRechargeSys::send_mail_to_player(void)
{
	for (DailyMap::iterator it = this->daily_map_.begin(); it != this->daily_map_.end(); ++it)
	{
		Proto30103001 reward_info;

		Int64 role_id = it->first;
		int mail_id = CONFIG_INSTANCE->daily_recharge_json(1)["mail_id"].asInt();

		for (int i = 0; i < MLDailyRecharge::TYPE_NUM; ++i)
		{
			if (it->second.__daily_recharge_rewards[i] == MLDailyRecharge::REWARD_HAVE)
			{
				int reward_id = this->get_reward_id(role_id, i);
				it->second.__daily_recharge_rewards[i] = MLDailyRecharge::REWARD_GONE;
				reward_info.add_reward_list(reward_id);
			}
		}
		reward_info.set_act_type(mail_id);
		reward_info.set_mail_id(mail_id);
		reward_info.set_role_id(role_id);

		MAP_MONITOR->dispatch_to_logic(&reward_info);
	}

	this->daily_map_.clear();
	this->mail_timer_.cancel_timer();
	Int64 left_time = ::next_day(0,5).sec() - Time_Value::gettimeofday().sec();
	this->mail_timer_.schedule_timer((int)left_time);
	return 0;
}

void MLBackDailyRechargeSys::init(void)
{
	this->mail_timer_.cancel_timer();
	Int64 left_time = ::next_day(0,1).sec() - Time_Value::gettimeofday().sec();
	this->mail_timer_.schedule_timer((int)left_time);
	start_time_ = 0;
	end_time_ = 0;
	this->request_load_DR_open_time();

	MMORechargeRewards::load_daily_recharge_mail_role(this);
}

void MLBackDailyRechargeSys::stop(void)
{
	MMORechargeRewards::save_daily_recharge_mail_role(this);
}

time_t MLBackDailyRechargeSys::start_tick()
{
	return this->start_time_;
}

time_t MLBackDailyRechargeSys::end_tick()
{
	return this->end_time_;
}

int MLBackDailyRechargeSys::check_and_notify_state_to_all(void)
{
//	if(activity_opening_ == daily_recharge_opening())	// 状态没有改变
//		return 0;
//
//	activity_opening_ = daily_recharge_opening();
//
//	PlayerManager::LogicPlayerSet player_set;
//	MAP_MONITOR->player_manager()->get_logic_player_set(player_set);
//	for(PlayerManager::LogicPlayerSet::iterator iter = player_set.begin();
//			iter != player_set.end(); iter++)
//	{
//		MapLogicPlayer *player = *iter;
//		player->notify_daily_recharge_info(activity_opening_);
//	}

	return 0;
}


bool MLBackDailyRechargeSys::daily_recharge_opening(time_t now)
{
	if (start_time_ == 0 && end_time_ == 0)
	{
		return false;
	}

	if (now == 0)
	{
		now = ::time(0);
	}

	return (start_time_ <= now && now <= end_time_);
}

int MLBackDailyRechargeSys::request_load_DR_open_time()
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, 0);

	data_map->push_multithread_query(DBBackDailyRecharge::COLLECTION,
			BSON(DBBackDailyRecharge::ID << DBBackDailyRecharge::OPEN_TIME_ID));

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_DAILY_RECHARGE_OPEN_TIME,
			data_map, MAP_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int MLBackDailyRechargeSys::after_load_DR_open_time(Transaction* trans)
{
	JUDGE_RETURN(NULL != trans, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	MongoDataMap* data_map = trans->fetch_mongo_data_map();
	if(NULL == data_map)
	{
		trans->rollback();
		return 0;
	}

	MongoData* mongo_data = NULL;
	int ret = 0;
	if(data_map->find_data(DBBackDailyRecharge::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
		while (cursor->more())
		{
			BSONObj res = cursor->next();
			JUDGE_CONTINUE(res.isEmpty() == false);

			ret = this->update_daily_recharge_open_time(
					res[DBBackDailyRecharge::START_TIME].numberLong(),
					res[DBBackDailyRecharge::END_TIME].numberLong());
		}
	}
	trans->summit();
	return ret;
}

int MLBackDailyRechargeSys::update_daily_recharge_open_time(time_t s, time_t e)
{
	bool time_has_change = false;
	if(s != start_time_ || e != end_time_)
		time_has_change = true;
	start_time_ = s;
	end_time_ = e;

	/*
	 * 时间有变化时需要强制执行一次全服通知
	 * 所以反转设置 activity_opening_ 的状态, 让 check_and_notify_state_to_all() 成功执行
	 * */
	if(time_has_change)
	{
		MSG_USER("daily recharge open time: %ld ~ %ld", start_time_, end_time_);
		activity_opening_ = !daily_recharge_opening();
	}
	return 0;
}
