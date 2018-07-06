/*
 * MLAchievement.h
 *
 *  Created on: 2014-1-17
 *      Author: louis
 */

#ifndef MLACHIEVEMENT_H_
#define MLACHIEVEMENT_H_

#include "MLPacker.h"

class MLAchievement : virtual public MLPacker
{
public:
	typedef std::map<int, AchieveDetail*> AchieveMap;
public:
	MLAchievement();
	virtual ~MLAchievement();

	void achieve_player_login();

	virtual int request_fetch_achieve_info();
	virtual int get_achieve_reward(Message* msg);

	virtual int update_achieve_info(const int ach_index, const int value = 0, const int ach_type = 0);	//成就更新
	virtual int finish_map_achievement(Message* msg);

	void reset(void);

	int sync_transfer_achieve(int scene_id);	// 角色切换场景时, 同步信息到其他地图进程
	int read_transfer_achieve(Message *msg);	// 角色切换场景时,读取来自其他进程的同步信息

protected:
	void init_achieve_info();
	void check_achieve_uplevel();
	int refresh_achieve_attr_add(const int enter_type = 0);

private:
	int login_update_achieve_info();	//登录检测成就状态

	int update_achieve_detail(BaseAchieveInfo::ChildAchieve* child_achieve, int value, const int ach_type = 0);	//更新成就
	int login_update_task_achieve_info(BaseAchieveInfo::ChildAchieve* child_achieve);   //登录计算日常，悬赏，帮派任务，问鼎江湖，经验副本问剑江湖
	int fetch_achieve_for_mount_type(int ach_index); //计算战骑类类型

	void calc_achieve_info(AchieveDetail *achieve_detail, int compare, int need_amount);
	void create_and_update_achieve(int achieve_id);

	void serilize_proto_achieve_detail(ProtoAchieveDetail* proto, AchieveDetail* achieve_detail, int ach_index);
public:
	AchieveMap& achieve_map(void);
	BaseAchieveInfo& base_achieve();
private:
	AchieveMap achieve_map_;
	BaseAchieveInfo base_achieve_;
};

#endif /* MLACHIEVEMENT_H_ */
