/*
 * LogicPlayer.h
 *
 * Created on: 2013-02-06 14:05
 *     Author: glendy
 */

#ifndef _LOGICPLAYER_H_
#define _LOGICPLAYER_H_

#include "ChatLogic.h"
#include "LogicOnline.h"
#include "LogicShoper.h"
#include "LogicTeamer.h"

#include "LogicLeaguer.h"
#include "LogicSocialer.h"
#include "LogicMarketer.h"
#include "TinyLogicPlayer.h"
#include "LogicActivityer.h"
#include "LogicWheelPlayer.h"
#include "DailyActPlayer.h"
#include "LogicRankPlayer.h"
#include "LogicActivityReminder.h"

#include "ArenaLPlayer.h"
#include "LogicCustomer.h"
#include "LogicWeddingPlayer.h"
#include "LogicLuckyTable.h"
#include "LogicBackActivityer.h"
#include "LogicMayActivityer.h"
#include "TrvlTeamPlayer.h"
#include "SpecialBox.h"

class LogicPlayer : public LogicTeamer,
					public LogicLeaguer,
					public LogicSocialer,
					public LogicMarketer,
					public LogicActivityer,
					public ChatLogic,
					public ArenaLPlayer,
					public LogicShoper,
					public LogicRankPlayer,
					public TinyLogicPlayer,
					public LogicActivityReminder,
					public LogicCustomer,
                    public LogicWeddingPlayer,
                    public LogicLuckyTable,
                    public LogicWheelPlayer,
                    public DailyActPlayer,
                    public LogicBackActivityer,
                    public LogicMayActivityer,
                    public TrvlTeamPlayer,
                    public SpecialBox
{
	friend class LogicBackActivityer;
public:
    enum {
        CACHE_BASE				   = 1,
        CACHE_ACTIVITY_TIPS		   = 2,
        CACHE_PANIC_SHOP           = 4,
        CACHE_WEDDING              = 5,
        CACHE_STRENGTH             = 6,
        CACHE_BACK_ACTIVITY        = 7,

        CACHE_END,

        TIMEUP_SAVE				   = 1,
        TIMEUP_CUSTOMER_SVC_REPLAY = 2,
        TIMEUP_END
    };

    struct CachedTimer : public GameTimer
    {
        CachedTimer(void);
        virtual ~CachedTimer(void);

    	virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);

        LogicPlayer *player_;
    };

public:
    LogicPlayer(void);
    virtual ~LogicPlayer(void);

    LogicMonitor *monitor(void);
    virtual int respond_to_client_error(const int recogn, const int error,
    		const Message *msg_proto = 0);

    void set_client_ip(const string &client_ip);
    std::string &client_ip(void);
    void set_session(const string& session);
    std::string &session(void);
    void set_role_id_for_load(const int64_t role_id);
    void set_uc_sid(const std::string &uc_sid);
    std::string &uc_sid(void);
    int kill_num(void);
    int killing_value(void);

    void set_new_role_flag(const bool flag);
    bool is_new_role(void);

    int sign_in(const int gate_sid);
    int resign(const int gate_sid, Proto30100101 *request);
    int sign_out(void);

    virtual int gate_sid(void);
    virtual int64_t entity_id(void);

    virtual Int64 role_id(void);
    virtual int role_id_low(void);
    virtual int role_id_high(void);

    virtual int scene_id(void);
    virtual int role_level(void);

    virtual LogicRoleDetail &role_detail(void);

    virtual int time_up(const Time_Value &nowtime);
    virtual GameCache &cache_tick(void);
    
    virtual int sync_role_info(Message *msg_proto);
    virtual int sync_vip_info(Message* msg);

    void update_player_force(int new_force);
    void update_leaguer_vip(int vip_level);

	virtual bool is_vip(void);
	virtual bool notify_msg_flag();

	virtual int vip_type(void);
	virtual BaseVipDetail& vip_detail(void);

    virtual const char *name(void);
    virtual const char *account(void);

	void reset(void);
	void reset_every_day(int test = false);
	void reset_every_week(void);
    LogicOnline &online(void);

    void send_script_reset();

    int map_enter_scene(Message* msg);
    int map_obtain_area(Message* msg);
    int map_fisrt_obtain_area();
	int map_add_goods_result(Message* msg);
	int map_consume_gold(Message* msg);

    int test_command(Message *msg);
    int logic_test_command(Message* msg);

    int fetch_other_player_info(Message* msg, int recogn);
    int respond_other_player_info(Message* msg);

    int respond_single_player_all_info(Message* msg);
    int respond_ranker_detail(Message* msg);

    int request_query_center_acti_code(Message* msg);
    int return_query_center_acti_code(Message* msg);

    int sync_update_player_name(Message *msg);
    int sync_update_player_sex();
    int sync_update_fight_detail(Message *msg);
    int sync_update_kill_player(Message *msg);
    int sync_update_mount_info(Message* msg);
    int sync_update_player_total_recharge_gold(Message* msg);
    int sync_update_act_serial_info(Message* msg);
    int sync_update_escort_info(Message* msg);
    int sync_update_quintuple_exp_info(Message* msg);
    int sync_update_buff_info(int status, int buff_id, int buff_time, int serial_from = 0);
    int sync_uddate_fashion_info(Message* msg);
    int send_chat_speak_state(void);

    void new_role_send_to_php_center();
    void set_speak_state(int type, int64_t expired_time);

    int notify_fight_prop_info(Message *msg);

    LogicHookDetail* hook_detail(void);
    int sync_hook_info(Message *msg);
    int notify_client_popup_info(Message* msg);
    int process_uc_extend_info(void);

    int request_sever_flag(void);
    int request_add_new_role_reward();
    //trade
    int trade_fetch_on_line_state(Message *msg);

    //escort
    int refresh_escort_info(Message *msg);

    //璇玑宝匣
    int fetch_player_dice_mult(Message *msg);

    int refresh_permission_info(Message *msg);

    void check_and_notify_qq();

    //resex
    int handle_resex(Message *msg);

    void set_client_mac(const string& mac);
    std::string& client_mac();

    int process_travel_reward_by_mail(Message *msg);
    static int process_tbattle_practice_mail(Message *msg);

protected:
    virtual int request_save_player(const int trans_recogn = TRANS_SAVE_LOGIC_PLAYER);

protected:
    LogicRoleDetail role_detail_;
    BaseVipDetail vip_detail_;

private:
    int gate_sid_;
    int64_t role_id_;

    bool is_new_role_;
    bool is_notify_msg_;
    bool is_first_area_;
    LongVec is_sgin_in;

    string client_ip_;
    string client_mac_;
    int ban_speak_;
    int64_t ban_speak_expired_;

    std::string session_;
    std::string uc_sid_;
    LogicMonitor *monitor_;

    GameCache cache_tick_;
    CachedTimer cached_timer_;

    LogicOnline online_;
    LogicHookDetail hook_detail_;
};

#endif //_LOGICPLAYER_H_
