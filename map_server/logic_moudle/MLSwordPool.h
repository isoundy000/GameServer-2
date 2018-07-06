/*
 * MLSwordPool.h
 *
 *  Created on: 2016年10月12日
 *      Author: lyw
 */

#ifndef MLSWORDPOOL_H_
#define MLSWORDPOOL_H_

#include "MLPacker.h"

class MLSwordPool : virtual public MLPacker
{
public:
	MLSwordPool();
	virtual ~MLSwordPool();
	void reset(void);

	int get_spool_info();
	int uplevel_sword_pool();
	int find_back_task_for_one(Message *msg);
	int find_back_all_task();
	int change_spool_style_lv(Message *msg);
	int fetch_spool_style_lvl();

	// transfer
	int sync_transfer_spool(int scene_id);
	int read_transfer_spool(Message* msg);

	int update_task_info(Message *msg); //其他模块调用该函数更新任务信息
	void update_sword_pool(int task_id, int num);
	void add_sword_pool_exp(int add_exp, int task_id = 0);

	SwordPoolDetail &sword_pool_detail();

	void check_spool_day_reset();
	void test_spool_day_reset();
	void test_find_task();

	void set_spool_level(int level);
	void spool_handle_player_levelup();
	void spool_handle_player_task(int task_id);
	void spool_update_mount_info();

protected:
	void check_level_open_spool();
	void check_task_open_spool(int task_id);
	void uplevel_change_task(); //升级后检测是否开启了新任务
	void task_reset(int task_id);
	int cal_task_total_num(int script_type);
	int refresh_spool_attr_add(const int enter_type = 0);
	bool is_sword_pool_task(int task_id);

	void notify_spool_style_lvl(int notify = true);//0:不通知，1:通知外形

private:
	SwordPoolDetail spool_detail_;
};

#endif /* MLSWORDPOOL_H_ */
