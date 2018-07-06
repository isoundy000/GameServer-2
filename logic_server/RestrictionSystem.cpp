/*
 * RestrictionSystem.cpp
 *
 *  Created on: Nov 17, 2014
 *      Author: LiJin
 */

#include "GameHeader.h"
#include "RestrictionSystem.h"
#include "ProtoDefine.h"
#include "PoolMonitor.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"
#include "Transaction.h"
#include "TQueryCursor.h"
#include "MongoData.h"
#include "MongoDataMap.h"

#include "GameField.h"
#include "BackField.h"
#include <mongo/client/dbclient.h>

RestrictionSystem::RestrictionSystem() {
	// TODO Auto-generated constructor stub

}

RestrictionSystem::~RestrictionSystem() {
	// TODO Auto-generated destructor stub
}

int RestrictionSystem::request_update_restriction_info(void)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	data_map->push_multithread_query(DBBackRestriction::COLLECTION,
			BSON(DBBackRestriction::FLAG << 0));

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_RESTRICTION_INFO,
			data_map, LOGIC_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}

int RestrictionSystem::respond_update_restirction_info(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	MongoDataMap* data_map = trans->fetch_mongo_data_map();
	if(data_map == 0)
	{
		trans->rollback();
		return -1;
	}

	IntMap result_map;
	MongoData* mongo_data = NULL;
	if(data_map->find_data(DBBackRestriction::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();

		while(cursor->more())
		{
			BSONObj obj = cursor->next();
			MSG_USER([#%s#], obj.toString().c_str());

			int flag = obj[DBBackRestriction::FLAG].numberInt();
			int id = obj[DBBackRestriction::ID].numberInt();

			JUDGE_RETURN(flag == 0, -1);

			int ret = (!process_restrion_operation(&obj)) ? 1 : 2;
			result_map[id] = ret;
		}

		for(IntMap::iterator iter = result_map.begin(); iter != result_map.end(); ++iter)
		{
			MSG_USER(MARK[%d]=[%d], iter->first, iter->second);
			this->request_mark_restriction_info_fin(iter->first, iter->second);
		}
	}

	trans->summit();
	return 0;
}

int RestrictionSystem::request_mark_restriction_info_fin(int id, int retsult)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	data_map->push_update(DBBackRestriction::COLLECTION,
			BSON(DBBackRestriction::ID << id), BSON(DBBackRestriction::FLAG << retsult), false);

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_MARK_RESTRICTION_INFO,
			data_map, 0) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}

int RestrictionSystem::sync_player_speak_state(int64_t role_id, int state, int expired_time)
{
	LogicPlayer* player = 0;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, -1);

	Proto30200127 sync_info;
	sync_info.set_speak_state(state);
	sync_info.set_expired_time(expired_time);

	return LOGIC_MONITOR->dispatch_to_chat(player, &sync_info);
}

int RestrictionSystem::ip_ban_state_change(int type, const string& ip_addr, int64_t expired)
{
	JUDGE_RETURN(ip_addr.empty() == false, -1);
	JUDGE_RETURN((type == GameEnum::BAN_OPT_RESTRIC) == (expired > ::time(0)), -1);

	uint ip_value = GameCommon::format_ip_string_to_int(ip_addr);
	JUDGE_RETURN(ip_value != 0u, -1);

	if (type == GameEnum::BAN_OPT_RESTRIC)
	{
		JUDGE_RETURN(expired > ::time(0), -1);
		this->force_logout_online_player(GameEnum::OPER_BAN_IP, ip_addr);
		this->save_ban_ip_to_db(ip_value, ip_addr, expired);
	}
	else if(type == GameEnum::BAN_OPT_DERESTRIC)
	{
		this->drop_ban_ip_from_db(ip_value);
	}

	return 0;
}

int RestrictionSystem::account_ban_state_change(int type, const string& account, int64_t expired)
{
	JUDGE_RETURN(account.empty() == false, -1);
	JUDGE_RETURN((type == GameEnum::BAN_OPT_RESTRIC) == (expired > ::time(0)), -1);

	if (type == GameEnum::BAN_OPT_RESTRIC)
	{
		this->force_logout_online_player(GameEnum::OPER_BAN_ACCOUNT, account);
	}

	BSONObj query_obj;
	this->make_query_bson_account(&query_obj, account);
	return this->save_restrion_to_db(&query_obj, GameEnum::OPER_BAN_ACCOUNT, expired);
}

int RestrictionSystem::role_id_ban_state_change(int type, int64_t role_id, int64_t expired)
{
	JUDGE_RETURN(role_id != 0, -1);
	JUDGE_RETURN((type == GameEnum::BAN_OPT_RESTRIC) == (expired > ::time(0)), -1);

	if(type == GameEnum::BAN_OPT_RESTRIC)
	{
		this->force_logout(role_id, type);
	}

	BSONObj query_obj(BSON(Role::ID << (long long int)role_id));
	return this->save_restrion_to_db(&query_obj, GameEnum::OPER_BAN_ROLE, expired);
}

int RestrictionSystem::role_name_ban_state_change(int type, const string& role_name, int64_t expired)
{
	JUDGE_RETURN(role_name.empty() == false, -1);
	JUDGE_RETURN((type == GameEnum::BAN_OPT_RESTRIC) == (expired > ::time(0)), -1);

	if(type == GameEnum::BAN_OPT_RESTRIC)
		this->force_logout_online_player(GameEnum::OPER_BAN_ROLE, role_name);

	BSONObj query_obj;
	this->make_query_bson_role_name(&query_obj, role_name);
	return this->save_restrion_to_db(&query_obj, GameEnum::OPER_BAN_ACCOUNT, expired);
}

int RestrictionSystem::role_id_speak_state_change(int type, int64_t role_id, int64_t expired)
{
	JUDGE_RETURN(role_id != 0, -1);

	LogicPlayer* player = 0;
	if(this->monitor()->find_player(role_id, player) == 0)
	{
		player->set_speak_state(type, expired);
		player->send_chat_speak_state();
	}

	BSONObj query_obj(BSON(Role::ID << (long long int)role_id));
	return this->save_restrion_to_db(&query_obj, type, expired);
}

int RestrictionSystem::role_name_speak_state_change(int type, const string& role_name, int64_t expired)
{
	JUDGE_RETURN(role_name.empty() == false, -1);

	LogicPlayerSet player_set;
	if(this->query_online_player(GameEnum::OPER_BAN_ROLE, role_name, player_set) > 0)
	{
		LogicPlayerSet::iterator iter = player_set.begin();
		for( ; iter != player_set.end(); ++iter )
		{
			LogicPlayer* player = *iter;
			player->set_speak_state(type, expired);
			player->send_chat_speak_state();
		}
	}

	BSONObj query_obj;
	this->make_query_bson_role_name(&query_obj, role_name);
	return this->save_restrion_to_db(&query_obj, type, expired);
}

int RestrictionSystem::role_kick_offline(int64_t role_id, const string& role_name, const string& account)
{
	int ret = -1;
	if(role_id != 0)
	{
		ret = this->force_logout(role_id);
	}

	if((ret != 0) && (role_name.empty() == false))
	{
		ret = this->force_logout_online_player(GameEnum::OPER_BAN_ROLE, role_name);
	}

	if((ret != 0) && (account.empty() == false))
	{
		ret = this->force_logout_online_player(GameEnum::OPER_BAN_ACCOUNT, account);
	}

	return ret;
}

int RestrictionSystem::process_restrion_operation(const BSONObj* bson_obj)
{
	JUDGE_RETURN(bson_obj != 0, -1);
	JUDGE_RETURN(bson_obj->isEmpty() == false , -1);

	const BSONObj &obj = *bson_obj;
	int operation = obj[DBBackRestriction::OPERATION].numberInt();
	int oper_type = obj[DBBackRestriction::OPER_TYPE].numberInt();

	Int64 role_id = obj[DBBackRestriction::ROLE_ID].numberLong();
	Int64 expired_time = obj[DBBackRestriction::EXPIRED_TIME].numberLong();

	string role_name = obj[DBBackRestriction::ROLE_NAME].str();
	string account = obj[DBBackRestriction::ACCOUNT].str();

	string ip_addr = obj[DBBackRestriction::IP_ADDR].str();
	string mac = obj[DBBackRestriction::MAC].str();

	if ((oper_type == GameEnum::BAN_OPT_RESTRIC) && (expired_time < ::time(0)))
	{
		MSG_USER(ERROR_EXPIRED_TIME[[ %ld ]], expired_time);
	}

	if (oper_type == GameEnum::BAN_OPT_DERESTRIC)
	{
		expired_time = 0;
	}

	switch(operation)
	{
	case GameEnum::OPER_BAN_IP:
		return this->ip_ban_state_change(oper_type, ip_addr, expired_time);

	case GameEnum::OPER_BAN_ACCOUNT:
		return this->account_ban_state_change(oper_type, account, expired_time);

	case GameEnum::OPER_BAN_ROLE:
	{
		if(0 != this->role_id_ban_state_change(oper_type, role_id, expired_time))
			return this->role_name_ban_state_change(oper_type, role_name, expired_time);

		return 0;
	}

	case GameEnum::OPER_BAN_SPEAK:
	case GameEnum::OPER_BAN_SPEAK_TRICK:
	{
		if(0 != this->role_id_speak_state_change(operation, role_id, expired_time))
			return this->role_name_speak_state_change(operation, role_name, expired_time);

		return 0;
	}

	case GameEnum::OPER_BNA_KICK:
		return this->role_kick_offline(role_id, role_name, account);
	case GameEnum::OPER_BAN_MAC:
		return this->mac_ban_state_change(oper_type, mac, expired_time);

	default:
		break;
	}

	return 0;
}

int RestrictionSystem::force_logout_online_player(int type, const string& match)
{
	LogicPlayerSet player_set;
	JUDGE_RETURN(this->query_online_player(type, match, player_set) > 0, -1);

	for(LogicPlayerSet::iterator iter = player_set.begin();
			iter != player_set.end(); ++iter)
	{
		this->force_logout(*iter, IntStrPair(type, match));
	}

	return 0;
}

int RestrictionSystem::query_online_player(int type, const string& match, LogicPlayerSet &player_set)
{
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		switch(type)
		{
		case GameEnum::OPER_BAN_ACCOUNT:
		{
			JUDGE_CONTINUE(match == player->account());
			player_set.push_back(player);
			break;
		}
		case GameEnum::OPER_BAN_ROLE:
		{
			JUDGE_CONTINUE(match == player->name());
			player_set.push_back(player);
			break;
		}
		case GameEnum::OPER_BAN_IP:
		{
			JUDGE_CONTINUE(match == player->client_ip());
			player_set.push_back(player);
			break;
		}
		case GameEnum::OPER_BAN_MAC:
		{
			JUDGE_CONTINUE(match == player->client_mac());
			player_set.push_back(player);
			break;
		}

		default:
			break;
		}
	}

	return player_set.size();
}

int RestrictionSystem::save_restrion_to_db(const BSONObj* querry, int ban_type, int64_t ban_expired)
{
	JUDGE_RETURN(querry != 0, -1);
	const BSONObj &obj = *querry;

	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	int ban_state = ban_expired > ::time(0) ? ban_type : GameEnum::OPER_BAN_NONE;

	if(ban_type == GameEnum::OPER_BAN_SPEAK_TRICK ||
			ban_type == GameEnum::OPER_BAN_SPEAK ||
			ban_type == GameEnum::OPER_BAN_ACCOUNT ||
			ban_type == GameEnum::OPER_BAN_ROLE ||
			ban_type == GameEnum::OPER_BAN_NONE )
	{
		data_map->push_update(Role::COLLECTION, obj, BSON(Role::BAN_TYPE << ban_state
				<< Role::BAN_EXPIRED << (long long int) ban_expired), false);
	}

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_MARK_RESTRICTION_INFO,
			data_map, 0) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}

int RestrictionSystem::save_ban_ip_to_db(uint ip_value, const string& ip_str, int64_t expired)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	data_map->push_update(DBBanIpInfo::COLLECTION,BSON(DBBanIpInfo::IP_UINT << ip_value),
			BSON(DBBanIpInfo::IP_STRING << ip_str << DBBanIpInfo::EXPIRED_TIME << (long long int)expired));

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_BAN_IP_INFO,
			data_map, 0) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}

int RestrictionSystem::drop_ban_ip_from_db(uint ip_value)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	data_map->push_remove(DBBanIpInfo::COLLECTION, BSON(DBBanIpInfo::IP_UINT << ip_value));

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_REMOVE_BAN_IP_INFO,
			data_map, 0) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}

void RestrictionSystem::make_query_bson_role_id(BSONObj* bson_obj, int64_t role_id)
{
	JUDGE_RETURN(bson_obj != 0, ;);
	BSONObj &obj = *bson_obj;
	obj = BSON(Role::ID << (long long int)role_id);
}

void RestrictionSystem::make_query_bson_role_name(BSONObj* bson_obj, const string& role_name)
{
	JUDGE_RETURN(bson_obj != 0, ;);
	BSONObj &obj = *bson_obj;
	obj = BSON(Role::NAME << role_name);
}

void RestrictionSystem::make_query_bson_account(BSONObj* bson_obj, const string& account)
{
	JUDGE_RETURN(bson_obj != 0, ;);
	BSONObj &obj = *bson_obj;
	obj = BSON(Role::ACCOUNT << account);
}

int RestrictionSystem::force_logout(int64_t role_id, int type)
{
	JUDGE_RETURN(role_id != 0, -1);

	LogicPlayer* player = 0;
	JUDGE_RETURN(this->monitor()->find_player(role_id, player) == 0, -1);

	return this->force_logout(player, IntStrPair(type, player->name()));
}

int RestrictionSystem::force_logout(LogicPlayer* player, const IntStrPair& pair)
{
	JUDGE_RETURN(player != NULL, -1);
	MSG_USER("try to force logout accout[%s]", player->account());

	Block_Buffer buff;
	buff << player->account() << pair.first << pair.second.c_str();

    ProtoHead proto_head;
    proto_head.__recogn = INNER_FORCE_GATE_LOGOUT;

    InnerRouteHead route_head;
    return this->monitor()->dispatch_to_scene(player->gate_sid(), route_head, proto_head, &buff);
}

LogicMonitor *RestrictionSystem::monitor(void)
{
    return LOGIC_MONITOR;
}

int RestrictionSystem::mac_ban_state_change(int type,const string& mac,int64_t expired)
{
	JUDGE_RETURN(mac.empty() == false, -1);

	MSG_USER("in function mac_ban_state_change");
	if(type == GameEnum::BAN_OPT_RESTRIC)
	{
		JUDGE_RETURN(expired > ::time(0), -1);
		this->force_logout_online_player(GameEnum::OPER_BAN_MAC, mac);
		this->save_ban_mac_to_db(mac, expired);
	}
	else if(type == GameEnum::BAN_OPT_DERESTRIC)
	{
		this->drop_ban_mac_from_db(mac);
	}
	return 0;
}

int RestrictionSystem::save_ban_mac_to_db(const string& mac_str, int64_t expired)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	BSONObjBuilder builder;
	builder<< DBBanMacInfo::EXPIRED_TIME << (long long int)expired;
	data_map->push_update(DBBanMacInfo::COLLECTION,BSON(DBBanMacInfo::MAC_STRING << mac_str),
			builder.obj(),true);
	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_BAN_MAC_INFO,
			data_map, 0) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

int RestrictionSystem::drop_ban_mac_from_db(const string& mac_str)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	data_map->push_remove(DBBanMacInfo::COLLECTION, BSON(DBBanMacInfo::MAC_STRING << mac_str));

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_REMOVE_BAN_MAC_INFO,
			data_map, 0) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}
