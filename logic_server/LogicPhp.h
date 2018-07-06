/*
 * LogicPhp.h
 *
 * Created on: 2013-06-09 17:22
 *     Author: lyz
 */

#ifndef _LOGICPHP_H_
#define _LOGICPHP_H_

#include "GameHeader.h"
#include "GatePlayer.h"
#include "ObjectPoolEx.h"

class Block_Buffer;

class LogicPhp
{
public:
	typedef ObjectPoolEx<GatePlayer> GatePlayerPool;

public:
    int request_test_command(const int sid, const std::vector<int64_t> &role_list, Block_Buffer *msg);
    int request_insert_item(const int sid, const std::vector<int64_t> &role_list, Block_Buffer *msg);
    int request_remove_item(const int sid, const std::vector<int64_t> &role_list, Block_Buffer *msg);
    int request_role_info(const int sid, Message *proto_msg);

    int load_role_info_by_account(const std::string &account, const int sid=0);
    int after_load_role_info(Transaction *transaction);

	int make_up_center_post_msg(LogicPlayer* player, Message *msg);
	int count_online_users(int interval);

    GatePlayerPool* gate_player_pool(void);

private:
    int count_online_users_by_agent();
    int count_online_users_by_platform(); //
    int count_online_users_by_market();
    int count_online_users_all();

private:
    GatePlayerPool gate_player_pool_; //通过 account 加载玩家信息时借用 GatePlayer
    Time_Value last_count_time_;	// 上一次统计在线玩家的时间
};

typedef Singleton<LogicPhp> LogicPhpSingle;
#define LOGIC_PHP   (LogicPhpSingle::instance())

#endif //_LOGICPHP_H_
