/*
 * LogicPhp.cpp
 *
 * Created on: 2013-06-09 17:26
 *     Author: lyz
 */

#include "LogicPhp.h"
#include "BaseUnit.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"
#include "Transaction.h"

int LogicPhp::request_test_command(const int sid, const std::vector<int64_t> &role_list, Block_Buffer *msg)
{
    int req_recogn = 0;
    msg->read_int32(req_recogn);
    return 0;
}

int LogicPhp::request_insert_item(const int sid, const std::vector<int64_t> &role_list, Block_Buffer *msg)
{
    int32_t item_index = -1, item_id = 0, item_num = 0, item_bind = 0;
    (*msg) >> item_index >> item_id >> item_num >> item_bind;

    //    Proto31800101 request;
    //    request.set_item_index(item_index);
    //    request.set_item_id(item_id);
    //    request.set_item_num(item_num);
    //    request.set_item_bind(item_bind);
    //
    //    LogicPlayerEx *player = 0;
    //    for (std::vector<int64_t>::const_iterator iter = role_list.begin(); iter != role_list.end(); ++iter)
    //    {
    //        if (LOGIC_MONITOR->find_player(*iter, player) != 0)
    //            continue;
    //
    //        LOGIC_MONITOR->dispatch_to_scene(player->gate_sid(), *iter, player->scene_id(), &request);
    //    }
    return 0;
}

int LogicPhp::request_remove_item(const int sid, const std::vector<int64_t> &role_list, Block_Buffer *msg)
{
    int32_t item_index = -1, item_id = 0, item_num = 0, item_bind = 0;
    (*msg) >> item_index >> item_id >> item_num >> item_bind;

    //    Proto31800102 request;
    //    request.set_item_index(item_index);
    //    request.set_item_id(item_id);
    //    request.set_item_num(item_num);
    //    request.set_item_bind(item_bind);
    //
    //    LogicPlayerEx *player = 0;
    //    for (std::vector<int64_t>::const_iterator iter = role_list.begin(); iter != role_list.end(); ++iter)
    //    {
    //        if (LOGIC_MONITOR->find_player(*iter, player) != 0)
    //            continue;
    //
    //        LOGIC_MONITOR->dispatch_to_scene(player->gate_sid(), *iter, player->scene_id(), &request);
    //    }
    return 0;
}

int LogicPhp::request_role_info(const int sid, Message *proto_msg)
{
	JUDGE_RETURN(proto_msg != NULL, -1);
	DYNAMIC_CAST(Proto30810101*, msg, proto_msg);
	if(msg == NULL)
	{
		MSG_USER("Message dynamic_cast fail, %s", proto_msg->ShortDebugString().c_str());
		return -1;
	}
//	MSG_DEBUG(%s, msg->ShortDebugString().c_str());
	std::string account = msg->account();
	if(account.size()==0)
	{
		MSG_USER("empty account, %s", msg->ShortDebugString().c_str());
		return -1;
	}
//	LOGIC_MONITOR->dispatch_to_scene();
	return load_role_info_by_account(account, sid);
}

int LogicPhp::load_role_info_by_account(const std::string &account, const int sid)
{
    JUDGE_RETURN(account.size() > 0, -1);

    GatePlayer *player = this->gate_player_pool()->pop();
    player->set_account(account.c_str());
    player->set_client_sid(sid);
    if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_GATE_PLAYER_PHP,
    		DB_GATE_PLAYER, player, this->gate_player_pool(),
                LOGIC_MONITOR->logic_unit()) != 0)
    {
    	this->gate_player_pool()->push(player);
    	MSG_USER("request_mongo_transaction() error");
        return -1;
    }
    return 0;
}

int LogicPhp::after_load_role_info(Transaction *transaction)
{
    JUDGE_RETURN(transaction != NULL, -1);

    if (transaction->detail().__error != 0 &&
    		transaction->detail().__error != ERROR_ROLE_NOT_EXISTS)
    {
        transaction->rollback();
        return -1;
    }

    TransactionData *trans_data = transaction->fetch_data(DB_GATE_PLAYER);
    JUDGE_RETURN(trans_data != NULL, -1);
    GatePlayer *player = trans_data->__data.__gate_player;
    trans_data->reset();
    transaction->summit();

	JUDGE_RETURN(player != NULL, -1);
    Proto30860101 respond;
    respond.set_role_name(player->detail().__name);
    respond.set_level(player->detail().__level);
    respond.set_sex(player->detail().__sex);
    respond.set_career(player->detail().__career);
    return LOGIC_MONITOR->dispatch_to_php(player->client_sid(), &respond);
}

LogicPhp::GatePlayerPool* LogicPhp::gate_player_pool(void)
{
	return &(this->gate_player_pool_);
}

int LogicPhp::make_up_center_post_msg(LogicPlayer* player, Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto32101101*, req, -1);

	LogicRoleDetail& detail = player->role_detail();
	req->set_account(detail.__account);
	req->set_role_name(detail.name());
	req->set_agent(detail.__agent);
	req->set_sex(detail.__sex);
	req->set_role_id(player->role_id());
	req->set_level(detail.__level);
    req->set_server_flag(detail.__server_flag);
	return 0;
}
/*
 * 统计在线玩家
 * */
int LogicPhp::count_online_users(int interval)
{
	Time_Value now = Time_Value::gettimeofday();
	// 第一次执行或者错过了一个1.5个周期
	if(this->last_count_time_.sec() == 0 ||
			(now-last_count_time_).sec() > (1.5)*interval)
	{
		// 取离现在最近的整分钟
		this->last_count_time_ = Time_Value(now.sec() - now.sec()%interval, 0);
	}

	if ((now - this->last_count_time_).sec() < interval)
	{
		return 0;
	}

	this->last_count_time_ = Time_Value(now.sec() - now.sec()%interval, 0);
	this->count_online_users_by_agent();
//	count_online_users_by_platform(); // 不用统计不同平台的在线
	this->count_online_users_by_market();
	this->count_online_users_all();
	return 0;
}

int LogicPhp::count_online_users_by_market(void)
{
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();

	IntMap users_map;
	IntMap hooking_map;
	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin(); iter != player_map.end(); iter++)
	{
		LogicPlayer *player = iter->second;
		int m_code = player->role_detail().__market_code;
		users_map[m_code]++;
		if(player->hook_detail()->__is_hooking)
			hooking_map[m_code]++;
	}

	for(IntMap::iterator iter = users_map.begin(); iter != users_map.end(); iter++)
	{
		int market = iter->first;
		int users = iter->second;
		int hooking_users = 0;
		if(hooking_map.find(iter->first) != hooking_map.end())
			hooking_users = hooking_map[market];
		SERIAL_RECORD->record_online_users(0, market, users, hooking_users, last_count_time_.sec());
	}
	return 0;
}

int LogicPhp::count_online_users_by_agent(void)
{
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();

	IntMap users_map;
	IntMap hooking_map;
	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin(); iter != player_map.end(); iter++)
	{
		LogicPlayer *player = iter->second;
		int agent_code = player->role_detail().__agent_code;
		users_map[agent_code]++;
		if(player->hook_detail()->__is_hooking)
			hooking_map[agent_code]++;
	}

	for(IntMap::iterator iter = users_map.begin(); iter != users_map.end(); iter++)
	{
		int agent_code = iter->first;
		int users = iter->second;
		int hooking_users = 0;
		if(hooking_map.find(iter->first) != hooking_map.end())
			hooking_users = hooking_map[agent_code];
		SERIAL_RECORD->record_online_users(agent_code, 0, users, hooking_users, last_count_time_.sec());
	}
	return 0;
}

int LogicPhp::count_online_users_by_platform(void)
{
	return 0;
}

int LogicPhp::count_online_users_all(void)
{
	int user_cnt = 0;
	int hooking_cnt = 0;

	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); iter++)
	{
		LogicPlayer *player = iter->second;
		user_cnt++;

		JUDGE_CONTINUE(player->hook_detail()->__is_hooking);
		hooking_cnt++;
	}

	return SERIAL_RECORD->record_online_users(0, 0, user_cnt, hooking_cnt, last_count_time_.sec());
}
