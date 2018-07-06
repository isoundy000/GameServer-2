/*
 * MLMounter.h
 *
 *  Created on: Nov 26, 2013
 *      Author: peizhibi
 */

#ifndef MLMOUNTER_H_
#define MLMOUNTER_H_

#include "MLPacker.h"

class Proto50100031;
class Proto50100156;

class MLMounter : virtual public MLPacker
{
public:
	MLMounter();
	virtual ~MLMounter();
	void reset();

	static int index_to_mount(int index_type);
	static int mount_to_index(int mount_type);

    MountDetail& mount_detail(int type = 1);
    MountDetail& mount_detail_by_skill(int skill_id);

	int fetch_mounter_info(Proto50100031* mounter_info);
    int make_up_mount_list(Proto50100156* &respond);

	int fetch_mount_info(Message* msg);
	int fetch_mount_info(int type = 1);
	int mount_evoluate(Message *msg);
	int takeon_mount(Message* msg);
	int takeoff_mount(Message* msg);
	int select_mount_shape(Message* msg);
	int select_mount_shape(int mount_type, int mount_shape, int flag = false);
	int use_mount_goods(Message* msg);
	int use_beast_wing_act_goods(int item_id, int mount_type = GameEnum::FUN_BEAST_WING);

	// transfer
	int sync_transfer_mounter(int scene_id);
	int read_transfer_mounter(Message* msg);

	int adjust_mount_open();
	int caculate_all_mount_prop();
	int caculate_mount_prop(int type = 1);
    int check_and_upgrade_mount(int type, const Json::Value& effect);
    int check_mount_skill_achieve(int mount_type);
    int check_pa_event_mount_evoluate(int mount_type);
    int check_pa_event_mount_ability(int mount_type, int item_id);
    int check_pa_event_mount_growth(int mount_type, int item_id);
    int check_pa_event_mount_skill(int mount_type);
    int check_pa_event_mount_equip(int mount_type);
    int check_pa_event_mount_equip(int mount_type, int item_id);

    int validate_tips_mount_evoluate(int mount_type);
    int validate_tips_mount_equip(int mount_type, int item_id);

    void record_mount(int serial, int mount_type);
    void mount_try_task(int task_id);
    void update_mount_open_recharge_act(int total_recharge);
    void check_handle_mount_bless_timeout();
    void refresh_notify_mount_info(int mount_type, int unnotify = false);

protected:
	void caculate_mount_open_prop(MountDetail& mount_detail);
	void caculate_mount_act_prop(MountDetail& mount_detail);

    void mount_upgrade_success(int mount_type, int reward_id);
    void mount_handle_player_levelup();
    void check_level_open_mount(int mount_type);
    void check_task_open_mount(int mount_type, int task_id);
    void send_mount_logic_server(MountDetail& mount_detail, int flag);
    void notify_mount_open_activity(int type);

	void notify_mount_shape(int notify = 3, int type = 1);//0:不通知，1:通知外形，2:通知等级，3:通知全部
	void refresh_mount_property(int mount_type, const SubObj& obj);

private:
	MountDetail mount_detail_[GameEnum::FUN_TOTAL_MOUNT_TYPE];
};

#endif /* MLMOUNTER_H_ */
