/*
 * PlayerAssist.h
 *
 *  Created on: Apr 26, 2014
 *      Author: louis
 */

#ifndef PLAYERASSIST_H_
#define PLAYERASSIST_H_

#include "MLPacker.h"

class PlayerAssist : virtual public MLPacker
{
public:
	typedef std::map<int, PlayerAssistTip> PATipPannel; //key : event_id

public:
	PlayerAssist(void);
	virtual ~PlayerAssist(void);

	void reset_everyday();
    void reset_player_assist(void);

    PATipPannel &assist_tip_pannel(void);
    bool is_ingore_show_pannel(void);
    void set_ingore_show_pannel(const bool flag);

    int cancel_player_assist(Message* msg);
	int fetch_player_assist_pannel(Message* msg);
	int fetch_player_assist_tips(Message* msg);
	int request_ingore_show_pannel(Message* msg);

	int process_inner_player_assist_tips_event(Message *msg);
	int update_player_assist_single_event(int event_id, int event_value);	//event_value表示删除红点
    int update_new_mail_amount_event(void);

	//sync info when tranfer scene
	int sync_transfer_tip_pannel(int scene_id);
	int read_transfer_tip_pannel(Message *msg);

	void check_pa_event_game_resource(int id, int value, const SerialObj& serial_obj);
    void check_pa_event_when_savvy_update(void);
    void check_pa_event_role_pass_skill_a(int skill_id, int skill_level);
    void check_pa_event_role_pass_skill(int item_id, int tips_id = 0);
    void check_pa_event_when_item_update(int item_id, int item_amount);
    void check_pa_event_when_level_up(void);
    void check_pa_event_when_transfer(void);
    void check_pa_event_daily_run_item(int tips_id, int value);

private:
    int notify_player_assist_event_appear(int event_id);
    int notify_player_assist_event_disappear(int event_id);

private:
	bool ingore_show_pannel_;
	PATipPannel pas_tip_pannel_;
};

#endif /* PLAYERASSIST_H_ */
