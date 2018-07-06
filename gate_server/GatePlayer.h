/*
 * GatePlayer.h
 *
 * Created on: 2013-04-16 10:27
 *     Author: lyz
 */

#ifndef _GATEPLAYER_H_
#define _GATEPLAYER_H_

#include "EntityCommunicate.h"
#include "GateStruct.h"
#include "LogicOnline.h"

class GateMonitor;

class GatePlayer : public EntityCommunicate
{
public:
    struct CachedTimer : public GameTimer
    {
        CachedTimer(void);
        virtual ~CachedTimer(void);

        virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);

        GatePlayer *player_;
    };

public:
    GatePlayer(void);
    virtual ~GatePlayer(void);

    void reset(void);
    GateMonitor *monitor(void);

    void set_agent(const char *agent);
    const char *agent(void);
    int agent_code(void);

    void set_market_code(int code);
    int market_code(void);

    void set_account(const char *account);
    const char* account(void);
    virtual const char* name();

    void set_session(const char *session);
    std::string &session(void);

    void set_active(const bool flag);
    bool is_active(void);

    void set_role_id(const int64_t id);
    Int64 role_id(void);
    int64_t entity_id(void);

    void set_line_id(const int line_id);
    int line_id(void);

    void set_client_sid(const int sid);
    int client_sid(void);

    void set_client_ip(const std::string &ip);
    void set_client_port(const int port);
    const std::string &client_ip(void);
    int client_port(void);

    void set_alive_amount(const int amount);
    int alive_amount(void);

    void set_ban_state(int ban_type, const int64_t expired_time);
    int ban_type(void);
    int64_t ban_expried_time(void);
    void set_ban_ip(bool is_ban);
    bool is_ban_ip(void);
    bool is_ban_login(void);

    GateRoleDetail &detail(void);

    int start_game(void);
    int stop_game(int reason, const string& desc = string());
    int update_map_sid();

    int sync_role_info(Message *msg_proto);
    int reqeuest_login_logic(void);
    int sync_update_player_name(Message *msg_proto);

    LogicOnline &online(void);

    virtual int respond_to_client_error(const int recogn, const int error, const Message *msg_proto = 0);

    int check_accelerate(const Time_Value &nowtime);

    int level_upgrade(int prev_level);
    const std::string &client_mac(void);
    bool is_ban_mac(void);
    void set_ban_mac(bool is_ban);

    void set_is_loading_mongo(const bool is_loading);
    bool is_loading_mongo(void);
    void set_agent_str(string agent);

protected:
    int64_t role_id_;
    int client_sid_;
    int line_id_;

    bool is_active_;
    std::string session_;

    GateMonitor *monitor_;

    GateRoleDetail role_detail_;
    LogicOnline online_;
    string client_ip_;
    int client_port_;

    Time_Value prev_alive_tick_;
    int alive_amount_;
    int ban_ip_;
    int ban_type_;
    int64_t ban_expire_time_;
    int ban_mac_;
    CachedTimer cached_timer_;

    bool is_loading_mongo_;
    string agent_;
};

#endif //_GATEPLAYER_H_
