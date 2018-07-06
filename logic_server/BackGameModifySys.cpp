/*
 * BackGameModifySys.cpp
 *
 *  Created on: 2015年12月30日
 *      Author: sky
 */

#include <BackGameModifySys.h>
#include "PoolMonitor.h"
#include "TransactionMonitor.h"
#include "Transaction.h"
#include "MongoDataMap.h"
#include "BackField.h"
#include "TQueryCursor.h"
#include "MongoData.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "BackGameSwitcher.h"
#include <mongo/client/dbclient.h>

#include "LeagueStruct.h"
#include "LeagueSystem.h"
#include "MMOLeague.h"
#include "LogicPlayer.h"
#include "LucktTableMonitor.h"
#include "GameField.h"

BackGameModifySys::BackGameModifySys()
{}
BackGameModifySys::~BackGameModifySys()
{}

int BackGameModifySys::interval_run(void)
{
	this->request_load_date_from_db();
	return 0;
}

int BackGameModifySys::request_load_date_from_db(void)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, 0);

	data_map->push_multithread_query(DBBackModify::COLLECTION);

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_GAME_MODIFY_INFO,
			data_map, LOGIC_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int BackGameModifySys::after_load_data(Transaction* trans)
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
	int ret = data_map->find_data(DBBackModify::COLLECTION, mongo_data);
	if( ret == 0)
	{
		this->update_vec.clear();
		auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
		while(cursor->more())
		{
			BSONObj res = cursor->next();
			int is_update = res[DBBackModify::IS_UPDATE].numberInt();
			JUDGE_CONTINUE(is_update == 1);
			BackGameModify modify;
			modify.is_update = 1;
			modify.name = res[DBBackModify::NAME].str();
			modify.role_id = res[DBBackModify::ROLE_ID].numberLong();
			modify.league_id = res[DBBackModify::LEAGUE_ID].numberLong();

			BSONObj value_obj = res.getObjectField(DBBackModify::VALUE.c_str());
			std::set<std::string> agent_code_set;
			value_obj.getFieldNames(agent_code_set);
			for (std::set<std::string>::iterator iter = agent_code_set.begin();
					iter != agent_code_set.end(); ++ iter)
			{
				int agent_code = ::atoi((*iter).c_str());
				JUDGE_CONTINUE(agent_code != 0);

				BSONObj info_obj = value_obj.getObjectField((*iter).c_str());

				BackGameModify::SuperVipInfo &super_vip_info = modify.value_map[agent_code];
				super_vip_info.qq_num = info_obj[DBBackModify::ValueEle::QQ_NUM].str();
				super_vip_info.des_content = info_obj[DBBackModify::ValueEle::DES_CONTENT].str();
				super_vip_info.des_mail = info_obj[DBBackModify::ValueEle::DES_MAIL].str();
				super_vip_info.vip_level_limit = info_obj[DBBackModify::ValueEle::VIP_LEVEL_LIMIT].numberInt();
				super_vip_info.need_recharge = info_obj[DBBackModify::ValueEle::RECHARGE].numberInt();
			}

			if(modify.name == GameModifyName::lucky_table)
			{
				modify.ltable.activity_id = res[DBBackModify::ACTIVITY_ID].numberLong();
				modify.ltable.open_state = res[DBBackModify::OPEN_STATE].numberInt();
				BSONObjIterator iter(res.getObjectField(DBBackModify::START_TICK.c_str()));
				int i = 0;
				while (iter.more())
				{
					modify.ltable.start_tick[i++] = iter.next().numberLong();
				}
				iter = res.getObjectField(DBBackModify::END_TICK.c_str());
				i = 0;
				while (iter.more())
				{
					modify.ltable.end_tick[i++] = iter.next().numberLong();
				}
			}
			this->update_vec.push_back(modify);
		}
	}
	trans->summit();

//	if(!this->update_vec.empty())
//	{
//		for(BackGameModifyVec::iterator iter = this->update_vec.begin(); iter != this->update_vec.end(); ++iter)
//		{
//			this->handle_game_modify(*iter);
//		}
//
//		BackGameSwitcher::save_game_modify(this->update_vec);
//		this->update_vec.clear();
//	}

	return ret;
}

int BackGameModifySys::fetch_super_vip_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400059*, request, -1);

	int64_t role_id = request->role_id();
	int agent_code = request->agent_code();
	JUDGE_RETURN(!this->update_vec.empty(), -1);

	for(BackGameModifyVec::iterator iter = this->update_vec.begin();
			iter != this->update_vec.end(); ++iter)
	{
		if(iter->name == GameModifyName::you49_qq)
		{
			BackGameModify::SuperVipInfoMap::iterator super_vip_info_pos = iter->value_map.find(agent_code);
			JUDGE_RETURN(super_vip_info_pos != iter->value_map.end(), -1);

			BackGameModify::SuperVipInfo &super_vip_info = iter->value_map[agent_code];
			request->set_qq_num(super_vip_info.qq_num);
			request->set_des_content(super_vip_info.des_content);
			request->set_des_mail(super_vip_info.des_mail);
			request->set_limit(super_vip_info.vip_level_limit);

			LogicPlayer* player = NULL;
			JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, -1);
			return LOGIC_MONITOR->dispatch_to_scene(player, request);
		}
	}

	return 0;
}

void BackGameModifySys::handle_game_modify(BackGameModify& game_modify)
{
	int ret = 0;
	if(game_modify.name == GameModifyName::you49_qq)
		ret = this->handle_49youqq(game_modify);
	else if(game_modify.name == GameModifyName::fashion_box)
		ret = this->handle_fashion_box(game_modify);
	else if(game_modify.name == GameModifyName::lucky_table)
		ret = this->handle_lucky_table(game_modify);

	if(ret != 0)
		MSG_USER("UPDATE GAME MODIFY ERROR:error_code:%d, name:%s,role_id:%ld,league_id:%ld,value:%ld,value_str:%s",
				ret,game_modify.name.c_str(),game_modify.role_id,game_modify.league_id,game_modify.value,game_modify.value_str.c_str());
}

int BackGameModifySys::handle_49youqq(BackGameModify& game_modify)
{
//	int ret = GameCommon::update_49_qq(LOGIC_MONITOR->fetch_qq_49(), game_modify.value_str);
//	this->notify_49_qq(LOGIC_MONITOR->fetch_qq_49());
//	if(ret != 0)
//		return ret;
	return 0;
}

void BackGameModifySys::notify_49_qq(LongMap& qq_set)
{
	std::string field;
	Proto81401202 notify;
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	LogicMonitor::PlayerMap::iterator iter = player_map.begin();
	for(;iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		int vip = GameCommon::qq_fetch_agent_vip(player->role_detail().__agent_code);
		if(vip > 0 && player->vip_type() >= vip)
		{
			notify.set_qq(qq_set[player->role_detail().__agent_code]);
			player->respond_to_client(ACTIVE_NOTIFY_49YOU_QQ,&notify);
		}
	}
}

int BackGameModifySys::handle_fashion_box(BackGameModify& game_modify)
{
	Proto30400203 req;
	LOGIC_MONITOR->dispatch_to_all_map(&req);
	return 0;
}

int BackGameModifySys::handle_lucky_table(BackGameModify& game_modify)
{
	//更新当前活动信息
	if(game_modify.ltable.activity_id == LACKY_TABLE->get_activity_key())
		return LACKY_TABLE->update_cfg(&game_modify);
	return 0;
}

