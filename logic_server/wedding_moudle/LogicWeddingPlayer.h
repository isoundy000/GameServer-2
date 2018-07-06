/*
 * LogicWeddingPlayer.h
 *
 * Created on: 2015-06-03 18:29
 *     Author: lyz
 */

#ifndef _LOGICWEDDINGPLAYER_H_
#define _LOGICWEDDINGPLAYER_H_

#include "BaseLogicPlayer.h"
#include "ProtoDefine.h"

class LogicWeddingPlayer : virtual public BaseLogicPlayer
{
public:
	enum
	{
		WED_NONE	= 1,	//未拥有
		WED_HAVE	= 2,	//可拥有
		WED_GONE	= 3,	//已拥有
		WED_END
	};

	class RefreshTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_father(LogicWeddingPlayer* player);
		LogicWeddingPlayer* player_;
	};

public:
    LogicWeddingPlayer(void);
    virtual ~LogicWeddingPlayer(void);

    int refresh_player_wedding_info(void);
    int wedding_login_in(void);
    int wedding_login_out(void);
    int wedding_online_work(void);
    void reset_wedding(void);
    bool is_has_wedding(void);
    bool is_wedding_prepare(void);      // 是否准备举办婚礼

    Int64 wedding_id(void);
    int day_wedding_times(void);
    WeddingDetail *wedding_detail(void);
    PlayerWeddingDetail &self_wedding_info(void);

    int get_vip_sweet_num();
    int get_ring_level(void);
    int get_sys_level(void);
    int get_tree_level(void);
    Int64 fetch_wedding_partner_id(void);
    LogicPlayer *fetch_wedding_partner(void);

    int intimacy(const Int64 partner_id);
    int total_recv_flower(void);
    int total_send_flower(void);
    int act_total_recv_flower(void);
    int act_total_send_flower(void);

    int set_intimacy(const Int64 partner_id, int value);
    int request_wedding_before(Message *msg);
    int request_reply_wedding(Message *msg);

    int request_update_ring_begin(Message *msg);
    int request_update_sys(Message *msg);
    int request_update_tree_begin(Message *msg);
    int request_update_ring_or_tree_end(Message *msg);

    int request_buy_wedding_treasures_begin(Message *msg);
    int request_buy_wedding_treasures_end(Message *msg);
    int request_fetch_wedding_treasures(Message *msg);

    int request_wedding_label_info();
    int request_wedding_get_label(Message *msg);
    int refresh_label_info();

    int sync_wedding_remove_item(int type, IntMap &item_id, IntMap &item_num);
    int update_wedding_property(int type, int exp);
    int update_couple_fb_times();

    int is_has_ring();
    int refresh_ring_info();
    int request_wedding_invest(Message *msg);
    int request_wedding_reply(Message *msg);

    int request_wedding(Message *msg);
    int request_wedding_make_up(Message *msg);
    int process_wedding_after_pack_check(Message *msg);

    int request_divorce(Message *msg);
    int process_divorce_after_pack_check(Message *msg);

    int request_keepsake_upgrade(Message *msg);
    int process_keepsake_upgrade_after_pack_check(Message *msg);

    int request_present_flower(Message *msg);
    int process_present_flower_after_pack_check(Message *msg);
    int update_intimacy(const int inc_intimacy, const Int64 receiver_id);
    int update_flower_amount(const Int64 receiver_id, const int item_id, const int item_num);

    int request_wedding_pannel(void);
    int make_pannel_info(ProtoWeddingDetail* detail, int type);

    int notify_is_has_wedding(void);
    int update_wedding_reply(const Int64 reply_id, const int reply);

    int sync_wedding_property(const int enter_type = ENTER_SCENE_TRANSFER);
    int divorce_sync_property(const int enter_type = ENTER_SCENE_TRANSFER);

    void finish_wedding_branch_task();
    void check_award_label(WeddingDetail *wedding_info);
    void check_adjust_wedding_id();

    int notify_cruise_icon(void);
    int notify_wedding_cartoon_play(void);
    int sync_wedding_info_to_map(int wedding_type, string partner_name);

protected:
    int validate_can_wedding(const Int64 partner_id);
    int notify_all_wedding_reply(const int reply);
    int send_wedding_info_to_travel(int wedding_type, LogicPlayer* partner);
    void refresh_day_wedding_times(void);

protected:
    PlayerWeddingDetail player_wedding_detail_;

    RefreshTimer refresh_timer_;
    Time_Value wedding_req_tick_;
    bool is_login_;
};

#endif //_LOGICWEDDINGPLAYER_H_
