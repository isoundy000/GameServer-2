/*
 * MapLogicLabel.h
 *
 *  Created on: 2013-12-1
 *      Author: louis
 */

#ifndef MAPLOGICLABEL_H_
#define MAPLOGICLABEL_H_

#include "MLPacker.h"

struct LabelDetail
{
	int cur_label_id_;
	int pre_label_id_;//进入某些场景需要保存之前称号(篝火)
	IntSet permant_label_list_;
	IntSet expire_unshown_list_;
	IntSet new_list_;
	std::map<int, Int64> limit_time_label_list_;
	LabelDetail() { this->reset(); };
	void reset();
};
class MapLogicLabel : virtual public MLPacker
{
public:
	MapLogicLabel();
	virtual ~MapLogicLabel();
	void reset(void);

	virtual int fetch_label_panel_info(Message* msg);
	virtual int select_label(Message* msg);
	virtual int cancel_label(Message* msg);
	virtual int fetch_single_label_info(Message* msg);

	int check_label_pa_event(void);
	int notify_new_label();
	int check_new_label(Message* msg);
	int sync_add_label(Message* msg);
	int label_time_up(const Time_Value& now_time);
	int notify_label_list_update(int opera, int label_id, int left_time = 0);
	int modify_expire_unshown_list(const int opera, const int label_id);

	int cur_label_id(void);
	int set_cur_label_id(int label_id);
	int pre_label_id(void);
	int set_pre_label_id(int label_id);
	IntSet& new_list(void);
	IntSet& permant_label_list(void);
	IntSet& expire_unshown_list(void);
	std::map<int, Int64>& limit_time_label_list(void);

	bool is_has_label(const int label_id);
	int insert_label(int label_id);
	int insert_label_i(int label_id, Int64 start_tick = 0);
	int calc_label_total_prop(IntMap& prop_map);
	int delete_label(int label_id);
	int calc_label_total_prop_ex(const int label_id, IntMap& prop_map);

	int refresh_cur_label_shape(const int label_id = 0);

	int sync_transfer_label_info(int scene_id); // 角色切换场景时, 同步信息到其他地图进程
	int read_transfer_label_info(Message *msg);	// 角色切换场景时,读取来自其他进程的同步信息

	int insert_label_by_item(const Json::Value &effect, PackageItem *pack_item, int& use_num);

private:
	LabelDetail label_detail_;
};

#endif /* MAPLOGICLABEL_H_ */
