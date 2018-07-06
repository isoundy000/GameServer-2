/*
 * ChatPlayer.h
 *
 * Created on: 2013-03-05 10:31
 *     Author: glendy
 */

#ifndef _CHATPLAYER_H_
#define _CHATPLAYER_H_

#include "EntityCommunicate.h"
#include "ChatStruct.h"

class ChatMonitor;
class Proto30200101;
class Proto10200002;

class ChatPlayer : virtual public EntityCommunicate
{
public:
    ChatPlayer(void);
    virtual ~ChatPlayer(void);

    ChatMonitor *monitor(void);

//    virtual int respond_to_client_error(const int recogn, const int error, Block_Buffer *buff);
    virtual int respond_to_client_error(const int recogn, const int error, const Message *msg_proto = 0);
    virtual int respond_to_client(Block_Buffer *buff);

    virtual int64_t entity_id(void);
    virtual int64_t role_id(void);
    virtual int role_id_low(void);
    virtual int role_id_high(void);
    virtual void set_client_sid(const int client_sid);
    virtual int client_sid(void);
    const char* account(void);
    virtual const char* name();
    virtual bool is_active(void);

    void reset(void);
    virtual int resign(const int64_t role_id, Message *msg_proto);
    virtual int sign_in(const int64_t role_id, Message *msg_proto);
    virtual int sign_out(void);
    int client_close(void);

    int init_role_detail(Proto30200101 *msg_proto);
    ChatRoleDetail &role_detail(void);

    //////////////////////////////////////////////////////
    const Int64 league_id(void);
    const int team_id(void);
    const int scene_id(void);
    const int label_id(void);
    const int travel_area_id(void);
    bool is_vip(void);

    int sync_role_info(Message *msg_proto);

    int establish_team(Message *msg_proto);//创建队伍
    int dismiss_team(Message *msg_proto);//解散队伍
    int join_team(Message *msg_proto);//加入队伍
    int leave_team(Message *msg_proto);//离开队伍

    int establish_league(Message *msg_proto);//建立宗派
    int join_league(Message *msg_proto);//加入宗派
    int dismiss_league(Message *msg_proto);//解散宗派
    int leave_league(Message *msg_proto);//退出宗派

	int add_black_list(Message *msg_proto);//加黑名单
	int remove_black_list(Message *msg_proto);//删除黑名单

	int add_friend_list(Message *msg_proto);//加名单
	int remove_friend_list(Message *msg_proto);//删除名单

	int refrsh_vip_status(Message *msg);

	int check_chat_channel_level(int channel_type);
	int check_chat_vip_times(int channel_type);
	string check_workcheck_to_client(const string& word);
    int check_chat_conditon(void);
    int check_league_channel();
    int check_nearby_channel();
    int check_black_list(Int64 tar_player_id);
    int check_travel_channel();
//    int check_is_add_to_stranger_friend(Int64 tar_player_id);

    int chat_normal(Message *msg);
    int chat_private(Message *msg);
    int chat_loudspeaker(Message *msg);

    int announce_with_player(Message* msg);
    int flaunt_dispatch(Message* msg);

    int chat_get_league_history(Message *msg);//获取宗派历史记录
    int chat_get_private_history(Message *msg);//获取私聊记录
    int chat_get_voice(Message *msg);//获取语音
    int chat_get_flaunt_record(Message *msg);

    int chat_private_display_offline_message(int64_t peer_id, int time_offset = Time_Value::gettimeofday().sec());

    int sync_update_player_name(Message *msg);
    int sync_update_player_sex();
    int sync_role_level(Message *msg);

    int sync_role_speak_state(Message *msg);
    int speak_state();

protected:
    bool is_active_;
    int64_t role_id_;
    int client_sid_;
    ChatMonitor *monitor_;
    ChatRoleDetail role_detail_;

private:
    int chat_world(Proto10200002 *proto, bool trick=false);
    int chat_nearby(Proto10200002 *proto, bool trick=false);
    int chat_league(Proto10200002 *proto, bool trick=false);
    int chat_team(Proto10200002 *proto, bool trick=false);
    int chat_travel(Proto10200002 *proto, bool trick=false);
    int chat_system(Proto10200002 *proto, bool trick=false);

    Time_Value last_world_chat_;
    Time_Value last_nearby_chat_;
    Time_Value last_league_chat_;
    Time_Value last_team_chat_;
    Time_Value last_private_chat_;

    Time_Value last_flaunt_tick_;
    Time_Value last_flaunt_position_tick_;

};

#endif //_CHATPLAYER_H_
