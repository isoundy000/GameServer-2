/*
 * MLBackRebateRechargeSys.cpp
 *
 *  Created on: 2016年11月9日
 *      Author: lzy
 */
#include "MLBackRebateRechargeSys.h"
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

MLBackRebateRechargeSys::MLBackRebateRechargeSys():
start_time_(0),end_time_(0),
activity_opening_(false)
{
	// TODO Auto-generated constructor stub

}

MLBackRebateRechargeSys::~MLBackRebateRechargeSys()
{
	// TODO Auto-generated destructor stub
}

void MLBackRebateRechargeSys::init(void)
{
	start_time_ = 0;
	end_time_ = 0;
	request_load_DR_open_time();
}

time_t MLBackRebateRechargeSys::start_tick()
{
	return this->start_time_;
}

time_t MLBackRebateRechargeSys::end_tick()
{
	return this->end_time_;
}

int MLBackRebateRechargeSys::check_and_notify_state_to_all(void)
{
	if(activity_opening_ == daily_recharge_opening())	// 状态没有改变
		return 0;

	activity_opening_ = daily_recharge_opening();

	PlayerManager::LogicPlayerSet player_set;
	MAP_MONITOR->player_manager()->get_logic_player_set(player_set);
	for(PlayerManager::LogicPlayerSet::iterator iter = player_set.begin();
			iter != player_set.end(); iter++)
	{
		MapLogicPlayer *player = *iter;
		player->notify_daily_recharge_info(activity_opening_);
	}

	return 0;
}


bool MLBackRebateRechargeSys::daily_recharge_opening(time_t now)
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

int MLBackRebateRechargeSys::request_load_DR_open_time()
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

int MLBackRebateRechargeSys::after_load_DR_open_time(Transaction* trans)
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

int MLBackRebateRechargeSys::update_daily_recharge_open_time(time_t s, time_t e)
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








