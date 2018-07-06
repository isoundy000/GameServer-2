/*
 * LogicGameSwitcherSys.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: jinxing
 */

#include "LogicGameSwitcherSys.h"
#include "PoolMonitor.h"
#include "TransactionMonitor.h"
#include "BackGameSwitcher.h"
#include "Transaction.h"
#include "MongoDataMap.h"
#include "BackField.h"
#include "TQueryCursor.h"
#include "MongoData.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include <mongo/client/dbclient.h>
#include "BaseLogicPlayer.h"

LogicGameSwitcherSys::LogicGameSwitcherSys()
{
	// TODO Auto-generated constructor stub

}

LogicGameSwitcherSys::~LogicGameSwitcherSys()
{
	// TODO Auto-generated destructor stub
}

GameSwitcherDetail& LogicGameSwitcherSys::detail(void)
{
	return this->detail_;
}

int LogicGameSwitcherSys::start(void)
{
	this->detail_.reset();

	GameSwitcherDetail prev_detail = this->detail_;
	this->direct_load_date_from_db();
	this->process_detail_has_update(prev_detail);

	MSG_USER("LogicGameSwitcherSys start...");
	return 0;
}

int LogicGameSwitcherSys::stop(void)
{
	return 0;
}

int LogicGameSwitcherSys::interval_run(void)
{
	return this->request_load_date_from_db();
}

int LogicGameSwitcherSys::direct_load_date_from_db(void)
{
	return BackGameSwitcher::direct_load_data(this->detail_);
}

int LogicGameSwitcherSys::request_load_date_from_db(void)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, 0);

	BackGameSwitcher::load_data_request(data_map);

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_GAME_SWITCHER_INFO,
			data_map, LOGIC_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int LogicGameSwitcherSys::after_load_data(Transaction* trans)
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
	GameSwitcherDetail prev_detail = this->detail_;

	if (data_map->find_data(DBBackSwitcher::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
		while(cursor->more())
		{
			BSONObj res = cursor->next();
			BackGameSwitcher::set_detail_value_by_bson(res, this->detail_);
		}
	}
	trans->summit();

	this->push_detail_to_ml();
	this->process_detail_has_update(prev_detail);

	return 0;
}

int LogicGameSwitcherSys::process_detail_has_update(GameSwitcherDetail& old)
{
	JUDGE_RETURN(this->detail_.has_update(old) == true, -1)
	return this->notify_client();
}

int LogicGameSwitcherSys::push_detail_to_ml(void)
{
	Proto30101401 msg;
	this->detail_.serialize(&msg);
	return LOGIC_MONITOR->dispatch_to_all_map(&msg);
}

int LogicGameSwitcherSys::notify_client(BaseLogicPlayer *player)
{
	BStrIntMap& switcher = this->detail_.switcher_map_;

	Proto81401901 msg;
	msg.set_equip_red(switcher[GameSwitcherName::equip_red]);
	msg.set_treasures(switcher[GameSwitcherName::treasures]);
	msg.set_gift(switcher[GameSwitcherName::gift]);
	msg.set_market(switcher[GameSwitcherName::market]);
	msg.set_shop(switcher[GameSwitcherName::shop]);
	msg.set_transfer(switcher[GameSwitcherName::transfer]);
	msg.set_molding_spirit(switcher[GameSwitcherName::molding_spirit]);
	msg.set_jewel_sublime(switcher[GameSwitcherName::jewel_sublime]);
	msg.set_special_box(switcher[GameSwitcherName::special_box]);

	if (player != NULL)
	{
		return player->respond_to_client(ACTIVE_NOTIFY_GAME_SWITCH_INFO, &msg);
	}
	else
	{
		return LOGIC_MONITOR->notify_all_player(ACTIVE_NOTIFY_GAME_SWITCH_INFO, &msg);
	}
}

bool LogicGameSwitcherSys::logic_check_switcher(const std::string& name)
{
	return this->detail_.switcher_map_[name];
}
