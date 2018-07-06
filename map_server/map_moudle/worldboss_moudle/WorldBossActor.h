/*
 * WorldBossActor.h
 *
 *  Created on: 2016年9月29日
 *      Author: lyw
 */

#ifndef WORLDBOSSACTOR_H_
#define WORLDBOSSACTOR_H_

#include "MapPlayer.h"

struct SaveInfo
{
	int scene_id_;
	MoverCoord coord_;

	void reset(void);
};

class WorldBossActor : virtual public MapPlayer
{
public:
	WorldBossActor();
	virtual ~WorldBossActor();

	void reset();
	virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
	virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);

	int request_join_wboss(Message* msg);
	int request_join_wboss_done(Message* msg);
	int request_wboss_info();
	int request_wboss_scene_info();
	int request_get_wboss_pocket_award(Message* msg);
	int request_create_dice_num(Message* msg);
	int request_my_rank_info(Message* msg);
	int request_wboss_red_point();

	int handle_exit_wboss_scene(void);
	int on_enter_wboss_scene(const int type);
	int on_exit_wboss_scene(const int type);

	int wboss_enter_scene_type();

	SaveInfo &fetch_save_info();

	int update_sword_pool_info();
	int update_cornucopia_task_info();
	int update_labour_task_info();

	//跨服世界boss
	int request_join_trvl_pre_scene(); 			//进入跨服中转场景
	int request_join_trvl_wboss(Message* msg);	//进入跨服世界boss
	int request_exit_trvl_wboss();				//退出跨服世界boss到中转场景
	int request_trvl_wboss_info();				//获取跨服世界boss信息
	int login_enter_trvl_wboss(int scene_id);	//登录进入跨服世界boss

	int on_enter_trvl_wboss_pre_scene(const int type);
	int on_enter_trvl_wboss_scene(const int type);
	int on_exit_trvl_wboss_scene(const int type);

	int handle_exit_trvl_wboss();

	int trvl_wboss_pre_scene();

private:
	SaveInfo save_info_;

};

#endif /* WORLDBOSSACTOR_H_ */
