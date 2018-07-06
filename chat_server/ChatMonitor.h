/*
 * ChatMonitor.h
 *
 * Created on: 2013-01-18 14:21
 *     Author: glendy
 */

#ifndef _CHATMONITOR_H_
#define _CHATMONITOR_H_

#include <map>
#include <set>
#include "ChatStruct.h"
#include "Singleton.h"
#include "ObjectPoolEx.h"
#include "ChatCommunicate.h"
#include "Svc_Static_List.h"
#include "HashMap.h"
#include "ChatUnit.h"
#include "ChatTimerHandler.h"
#include "ChatPlayer.h"
#include "ServerDefine.h"

class SessionManager;
class ChannelAgency;

class Proto30200117;

class ChatMonitor : public ServerMonitor<ChatClientService, ChatInnerService, ChatConnectService>
{
public:
    typedef ServerMonitor<ChatClientService, ChatInnerService, ChatConnectService> SUPPER;
    typedef ObjectPoolEx<UnitMessage> UnitMessagePool;
    typedef ObjectPoolEx<ChatPlayer> ChatPlayerPool;
    typedef ObjectPoolEx<FlauntRecord> FlauntRecordPool;
    typedef ObjectPoolEx<ChatTimes> ChatTimesPool;

    typedef std::map<int, ChatLimit>ChatLimitMap;
    typedef std::vector<ChatTimerHandler> TimerHandlerList;
    typedef HashMap<int64_t, ChatPlayer *, NULL_MUTEX> PlayerMap;

    typedef HashMap<int64_t, FlauntRecord *, NULL_MUTEX> FlauntMap;

    typedef std::map<int64_t, ChatTimes *> ChatTimesMap;

    class ChatTenSecTimer : public FixedTimer
    {
    public:
    	ChatTenSecTimer(void);
        virtual int handle_timeout(const Time_Value &tv);
    };

    class ChatTimer : public FixedTimer
    {
    public:
    	ChatTimer(void);
        virtual int handle_timeout(const Time_Value &tv);
    };

    class VipTimesTimer : public GameTimer
    {
    public:
    	virtual int type();
        virtual int handle_timeout(const Time_Value &tv);
    };

public:
    ChatMonitor(void);

    virtual int init(void);
    virtual int start(void);
    virtual int start_game(void);
    virtual void fina(void);
    int logout_all_player(void);

    virtual BaseUnit *logic_unit(void);

    Block_Buffer *pop_block(int cid = 0);
    int push_block(Block_Buffer *buff, int cid = 0);

    UnitMessagePool *unit_msg_pool(void);

    int fetch_receive_timeout();
    int find_client_service(const int sid, ChatClientService *&svc);
    int find_inner_service(const int sid, ChatInnerService *&svc);
    int find_connect_service(const int sid, ChatConnectService *&svc);

    int dispatch_to_client(const int sid, Block_Buffer *buff);
    int dispatch_to_client(const int sid, const int recogn, const int error = 0, const Message *msg_proto = 0);
    int dispatch_to_client(ChatPlayer *player, Block_Buffer *buff);
    int dispatch_to_client(ChatPlayer *player, const int recogn, const int error = 0);
    int dispatch_to_client(ChatPlayer *player, const Message *msg_proto, const int error = 0);
//    int dispatch_to_scene(ProtoHead *head, const Message *msg_proto);

    SessionManager *session_manager(void);
    ChatPlayerPool *player_pool(void);
    int bind_player(const int64_t role_id, ChatPlayer *player);
    int unbind_player(const int64_t role_id);
    int find_player(const int64_t role_id, ChatPlayer *&player);

    int client_login_chat(const int client_sid, Message *msg_proto);
    int bind_sid_player(const int client_sid, ChatPlayer *player);
    int unbind_sid_player(const int client_sid);
    int find_sid_player(const int client_sid, ChatPlayer *&player);

    FlauntRecordPool *flaunt_pool(void);
    Int64 generate_flaunt_record(Proto30200117* proto);
    int bind_flaunt_record(const int64_t flaunt_id, FlauntRecord *flaunt);
    int unbind_flaunt_record(const int64_t flaunt_id);
    int find_flaunt_record(const int64_t flaunt_id, FlauntRecord *&flaunt);

    ChatTimesPool *chat_times_pool(void);
    int add_player_chat_times(Int64 role_id, int channel_id);
    ChatTimes *fetch_player_chat_times(Int64 role_id);
    int reset_player_chat_times();

    int announce_world(Message* msg);
    int back_stage_push_system_announce(Message* msg);

    void report_pool_info(void);

    ChannelAgency* channel_agency(void);
    int after_db_opera_reset_base_channel(int channel_type, Int64 channel_id);

    int request_load_chat_limit();
    int after_load_chat_limit(DBShopMode* shop_mode);

    int request_load_word_check();
    int after_load_word_check(DBShopMode* shop_mode);

    int request_load_vip_limit();
    int after_load_vip_check(DBShopMode* shop_mode);

    ChatLimit* fetch_chat_limit(int channel_type);
    VipLimit& fetch_vip_limit();
    WordCheck &word_check();

    /*
     * chat thread
     * */
    int db_load_mode_begin(int trans_recogn, Int64 role_id = 0);
    int db_load_mode_begin(DBShopMode* shop_mode, Int64 role_id = 0);
    int db_load_mode_done(Transaction* trans);

protected:
    virtual int process_init_scene(const int scene_id, const int config_index, const int space_id = 0);

    virtual int init_game_timer_handler(void);
    virtual int start_game_timer_handler(void);

protected:
    ChatClientPacker client_packer_;
    ChatInnerPacker inner_packer_;

    ChatUnit logic_unit_;

    TimerHandlerList timer_handler_list_;

    SessionManager *session_manager_;

    ChatPlayerPool player_pool_;
    PlayerMap player_map_;
    PlayerMap sid_player_map_;

    FlauntRecordPool flaunt_record_pool_;
    FlauntMap flaunt_map_;

    ChatTimesPool chat_times_pool_;
    ChatTimesMap chat_times_map_;

    ChannelAgency *channel_agency_;

    ChatTenSecTimer ten_sec_timer_;
    ChatTimer chat_timer_;
    VipTimesTimer vip_times_timer_;

    ChatLimitMap limit_map_;
    VipLimit vip_limit_;
    WordCheck workcheck_;

};

typedef Singleton<ChatMonitor> ChatMonitorSingle;
#define CHAT_MONITOR    	(ChatMonitorSingle::instance())
#define CHANNEL_AGENCY		CHAT_MONITOR->channel_agency()

#endif //_CHATMONITOR_H_
