/*
 * LogicTeamer.h
 *
 *  Created on: Jul 22, 2013
 *      Author: peizhibi
 */

#ifndef LOGICTEAMER_H_
#define LOGICTEAMER_H_

#include "TeamPanel.h"
#include "BaseLogicPlayer.h"

class LogicTeamer :  virtual public BaseLogicPlayer
{
public:
	LogicTeamer();
	virtual ~LogicTeamer();

	int teamer_state();
	int teamer_sign_in();
	int teamer_sign_out();
	int teamer_delay_sign_in();

	int logic_fetch_team_info();
	int logic_close_team_info();
	int logic_create_team();
	int check_and_quit_team();
	int logic_quit_team();
	int logic_appoint_leader(Message* msg);
	int logic_kick_teamer(Message* msg);
	int logic_request_org_team(Message* msg);
	int accept_invite_into_team(Message* msg);
	int accept_apply_into_team(Message* msg);
	int validate_map_team_info(Message* msg);

	int logic_fast_join_team(Message* msg);
	int logic_near_team_begin(Message* msg);
	int logic_near_team_done(Message* msg);
	int logic_auto_invite(Message* msg);
	int logic_auto_accept(Message* msg);
	int logic_near_player(Message* msg);

	int handle_request_time_out();
	void map_enter_scene();
	int replace_leader();

	int notify_all_teamer_team_info();
	int sync_all_teamer_team_info();
	int notify_all_teamer_couple_fb_info();

	TeamSetInfo& team_set_info();
	int team_index(int team_index = -1);
	TeamPanel* team_panel(int team_index = -1);
	int team_info_output_test(const char *str, Message *msg=0);
	int notify_self_quit_team(int quit_type);

	int team_monster_info_update(Message* msg);

private:
	int create_team_i();
	int dismiss_team_i();
	int quit_team_i();
	int sign_out_with_team();
	int accept_invite_team_i(int64_t role_id);
	int accept_apply_team_i(int64_t role_id);

	int check_and_leave_fb();
	int check_and_dismiss_team();
	TeamPanel* check_and_create_team();

	int notify_team_oper_info(LogicPlayer* player, int oper_type);
	int notify_team_oper_result(int oper_type, int result, const char* role_name);

	int appoint_leader_i(int64_t role_id, int push_self = true);

	int handle_enter_team(int64_t role_id, TeamPanel* team_panel, const int oper_type);
	int handle_quit_team(int quit_type);

	int logic_invite_into_team(LogicPlayer* player);
	int logic_apply_into_team(LogicPlayer* player);
	int logic_request_switch_fb_team(LogicPlayer* player);

	// 响应请求
	int reply_invite_into_team(int64_t role_id, bool respone);
	int reply_apply_into_team(int64_t role_id, bool respone);
	int reply_confirm_switch_fb_team(bool respone);

	int notify_other_quit_team(int64_t role_id, int quit_type);
	// 通知客户端和同步线程信息
	int sync_team_info_to_scene();

	int fetch_team_info(Proto80400306* team_info);
	int make_up_teamer_info(ProtoTeamer* teamer_info);

	int fetch_states_of_all_teamers();
	int notify_self_states_to_teamer();
	int notify_offline_state_to_teamer();
	int notify_team_info();

	int notify_all_teamer(int recogn, Message* msg = NULL);

	int save_teamer_info_with_team(TeamPanel *team_panel);
	int save_teamer_info_quit_team();

	bool validate_team_level_limit();
public:
	int team_fb_respond_team_switch(Message* msg);
	int team_fb_recruit_replacement(Message* msg);
	int team_fb_invite_into_team(Message* msg);
	int team_fb_request_enter_couple_fb();				//请求进入夫妻副本
	int team_fb_respond_enter_couple_fb(Message* msg);	//回应进入夫妻副本请求
	int handle_respond_enter_couple_fb(int respond_state);
	int request_couple_fb_send_times();					//夫妻副本请求赠送
	int team_fb_apply_into_team(Message* msg);
	int team_fb_respond_invite(Message* msg);
	int team_fb_respond_apply(Message* msg);
	int team_fb_change_fb(Message* msg);
	int team_fb_get_ready(Message* msg);
	int team_fb_get_ready_from_scene(Message* msg);
	int team_fb_organize(Message* msg);
	int team_fb_enter_fb(Message* msg);
	int team_fb_enter_fb_from_scene(Message* msg);
	int team_fb_leave_fb(Message* msg);
	int team_fb_broadcast_recruit();
	int team_fb_organize_cancel();
	int team_fb_friend_list();
	int team_fb_leaguer_list();
	int team_fb_dismiss_team();
	int team_fb_deduct_strength(int script_id); //team_fb_valiate_or_cost_vit
   bool team_fb_valiate_strength_enougth(int script_id);
	int finish_sync_fb_use_times(Message* msg);

	int team_fb_clean_replacement();
	int team_fb_remove_replacement(int64_t role_id);

	int begin_get_rpm_recomand_info(Message* msg);
	int after_get_rpm_recomand_info(Transaction* trans);
	int sync_player_enter_result(Message* msg);

private:
	int team_fb_notify_invite_info(LogicPlayer* player);
	int team_fb_notify_apply_info(LogicPlayer* player);
	int team_fb_notify_request(LogicPlayer* player, int oper_type);

	int erase_team_fb_info();
	int set_ready_state(bool is_ready);
	int cancel_all_teamer_ready_state();
	int notify_all_teamer_ready_state(bool is_ready);
	int notify_all_teamer_fb_info();
	int notify_all_teamer_use_times();
	int fetch_teamers_ready_state();
	int fetch_team_use_times();
	int fetch_team_fb_info();
	int notify_fb_team_info();

	int validate_enter_couple_fb();
	int validate_use_times(int script_sort, int use_times);
	int validate_fb(int script_sort, int extra_condition=GameEnum::EXTRA_CONDITION_NONE);
	bool is_get_ready(int64_t role_id);
	bool is_runing_script();

	int sync_fb_use_times(const int oper_type);
	int sync_all_teamer_fb_use_times();

protected:
	void reset();

private:
	TeamSetInfo team_set_info_;
};

#endif /* LOGICTEAMER_H_ */
