/*
 * PlayerManager.h
 *
 * Created on: 2013-03-23 10:59
 *     Author: lyz
 */

#ifndef _PLAYERMANAGER_H_
#define _PLAYERMANAGER_H_

#include "PubStruct.h"

class Proto30400101;
class Proto30400102;
class Proto30400103;
class Proto30400104;
class Proto30400105;

class Proto31400102;
class Proto31400103;
class Proto31400104;
class Proto31400105;
class Proto30400350;

class PlayerManager
{
public:
    typedef HashMap<int64_t, MapPlayerEx *, NULL_MUTEX> PlayerMap;
    typedef HashMap<int64_t, MapLogicPlayer *, NULL_MUTEX> LogicPlayerMap;
    typedef HashMap<int64_t, int, NULL_MUTEX> RoleSidMap;
    typedef std::vector<MapLogicPlayer *> LogicPlayerSet;

    typedef HashMap<int64_t, int64_t, NULL_MUTEX> MLPlayerLoginTickMap;
    typedef HashMap<int64_t, int64_t, NULL_MUTEX> MLPlayerLogoutTickMap;

    typedef HashMap<int64_t, int64_t, NULL_MUTEX> MapPlayerLoginTickMap;
    typedef HashMap<int64_t, int64_t, NULL_MUTEX> MapPlayerLogoutTickMap;

public:
    PlayerManager(void);
    ~PlayerManager(void);

    MapMonitor *monitor(void);

    int request_map_player_login(const int sid, const Int64 role_id, Message *msg);
    int after_load_player(Transaction *transaction);
    int process_map_player_logout(const int gate_sid, const Int64 role_id);

    int request_load_logic_player(const int gate_sid, const int64_t role_id);
    int after_load_logic_player(Transaction *transaction);
    int process_ml_player_logout(int gate_sid, Int64 role_id, int type = 1);

    int update_transfer_base(int gate_sid, Int64 role_id, Message* msg);
    int finish_sync_transfer(const int gate_sid, Message *msg);
    int finish_sync_transfer(const int64_t role_id);
    int sync_transfer_map(const int64_t role_id, const int recogn, Message *msg);
    int sync_transfer_online(const int gate_sid, Proto30400105 *request);

    int bind_player(const int64_t role_id, MapPlayerEx *player);
    int unbind_player(const int64_t role_id);
    int find_player(const int64_t role_id, MapPlayerEx *&player);
    int process_player_size(void);
    int get_map_player_set(MapPlayerExVec& map_player_set);
    void reset_map_player_everyday();

    int bind_sid_player(const int client_sid, MapPlayerEx *player);
    int unbind_sid_player(const int client_sid, MapPlayerEx *&player);
    int find_sid_player(const int client_sid, MapPlayerEx *&player);
    int find_client_sid_by_role(const int64_t role_id, int &client_sid);

    int bind_logic_player(const int64_t role_id, MapLogicPlayer *player);
    int unbind_logic_player(const int64_t role_id);
    int find_logic_player(const int64_t role_id, MapLogicPlayer *&player);
    int get_logic_player_set(LogicPlayerSet& logic_player_set);

    int update_transfer_logic(Int64 role_id, int recogn, Message* msg);
    int update_transfer_logic_base(const int gate_sid, Proto31400103 *request);
    int update_transfer_logic_package(const int gate_id, Proto31400104 *request);
    int update_transfer_logic_online(const int gate_sid, Proto31400105 *request);
    int finish_sync_transfer_logic_info(int gate_sid, Proto31400102 *request);
    int finish_sync_transfer_start_logic(int64_t role_id);

    int logout_map_all_player();
    int logout_logic_all_player();
    int notify_all_player_offline();

    int update_transfer_logic_by_bson(Int64 role_id, Message* msg);
    int sync_transfer_map_by_bson(Int64 role_id, Message* msg);

    Time_Value map_player_login_tick(const Int64 role_id);
    Time_Value map_player_logout_tick(const Int64 role_id);
    Time_Value ml_player_login_tick(const Int64 role_id);
    Time_Value ml_player_logout_tick(const Int64 role_id);

protected:
    MapMonitor *monitor_;

    PlayerMap prev_player_map_;
    PlayerMap player_map_;
    PlayerMap sid_player_map_;
    MapPlayerLoginTickMap map_login_tick_map_;  // key: role_id, value: 登录请求时间, 战斗线程登录处理
    MapPlayerLogoutTickMap map_logout_tick_map_;    // key: role_id, value: 离线时间，用于处理加载过程中的断线

    LogicPlayerMap prev_logic_player_map_;
    LogicPlayerMap logic_player_map_;
    MLPlayerLoginTickMap ml_login_tick_map_;    // key: role_id, value: 登录请求时间，背包线程登录处理
    MLPlayerLogoutTickMap ml_logout_tick_map_;  // key: role_id, value: 离线时间，用于处理加载过程中的断线

    RoleSidMap role_clientsid_map_;
};

#endif //_PLAYERMANAGER_H_
