/*
 * MapLogicIllustration.h
 *
 *  Created on: 2016年7月19日
 *      Author: lzy0927
 */

#ifndef MAPLOGICILLUSTRATION_H_
#define MAPLOGICILLUSTRATION_H_


#include "MLPacker.h"


class MapLogicIllustration : virtual public MLPacker
{
public:
	MapLogicIllustration();
	virtual ~MapLogicIllustration();
	void reset(void);
	int received_award_illus_set(IntSet &res_set);
	int received_award_illus_map(IntMap &res_map);

	void refresh_illustration_prop(int enter_type = 0);
	int fetch_illus_panel_info(Message* msg);
	int select_illus_class(Message* msg);
	int select_illus_group(Message* msg);
	int fetch_single_illus_info(Message* msg);
	int select_single_illus(Message* msg);
	int upgrade_illus_info(Message* msg);

	int check_open_illustration(int task_id = 0);
	int calc_illus_total_prop(IntMap& prop_map);
	int calc_illus_total_prop_ex(const int illus_id, const int illus_level, IntMap& prop_map);

	int sync_add_illus(Message* msg);
	int sync_add_illus(int group_id);
    void test_illus(void);
    int notify_illus_group_list_update(int opera, int group_id);

	int cur_illus_id(void);
	int cur_illus_class_id(void);
	int cur_illus_group_id(void);

	int set_cur_illus_id(int illus_id);
	int set_cur_illus_class_id(int illus_class_id);
	int set_cur_illus_group_id(int illus_group_id);

	int pre_illus_id(void);
	int pre_illus_class_id(void);
	int pre_illus_group_id(void);

	int set_pre_illus_id(int illus_id);
	int set_pre_illus_class_id(int illus_class_id);
	int set_pre_illus_group_id(int illus_group_id);

	int new_sync_add_illus(int level);

//	bool is_has_illus(const int illus_id, const int illus_class_id, const int illus_group_id);

	int upgrade_illus(int illus_id,const int item_id = 0);
	int test_max_illus(int level);
	int refresh_illus_prop();
    int refresh_group_shape(const int class_id );
    int refresh_illus_shape(const int class_id , const int group_id );

//	int refresh_cur_label_shape(const int label_id = 0);

	int sync_transfer_illus_info(int scene_id); // 角色切换场景时, 同步信息到其他地图进程
	int read_transfer_illus_info(Message *msg);	// 角色切换场景时,读取来自其他进程的同步信息

    int attr_type_int(const string str);
    IntSet &get_group_list(void);
    IntMap &get_illus_list(void);
    int &get_open(void);

private:
    int open_;
	int cur_illus_id_;
	int cur_illus_class_id_;
	int cur_illus_group_id_;

	int pre_illus_id_;
	int pre_illus_class_id_;
	int pre_illus_group_id_;


	IntVec illus_list_;
	IntVec illus_group_list_;

    IntSet get_group_list_;
    IntMap get_illus_list_;
};



#endif /* MAPLOGICILLUSTRATION_H_ */
