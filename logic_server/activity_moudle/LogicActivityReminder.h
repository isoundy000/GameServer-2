/*
 * LogicActivityReminder.h
 *
 *  Created on: Apr 11, 2014
 *      Author: root
 */

#ifndef LOGICACTIVITYREMINDER_H_
#define LOGICACTIVITYREMINDER_H_

#include "BaseLogicPlayer.h"

class LogicActivityReminder :virtual public BaseLogicPlayer
{
public:
	LogicActivityReminder();
	virtual ~LogicActivityReminder();
	void reset();

    void refresh_activity_record(ActivityTipsInfo *act_tips_info);
    // 查出个人相关的活动图标记录
    ActivityRec *find_activity_record(const int activity_id);

    int validate_activity_condition(const Json::Value& activity_json);
    int need_notify_activity_tips(ActivityTipsInfo *act_tips);
    int check_and_notify_activity_tips(ActivityTipsInfo *act_tips);

    int fetch_activity_finish_count(ActivityTipsInfo *act_tips, const Json::Value &activity_json);

	int fetch_all_tips_info(void);
	int touch_tips_icon(Message* msg);

	int check_activity_reminder();//登录和升级时需要调用
	int fetch_acti_state(ActivityTipsInfo *tips_info);

	int on_join_activity(int activity_id);
	ActivityInfoMap& activity_info();

private:
	ActivityInfoMap activity_info_;
};

#endif /* LOGICACTIVITYREMINDER_H_ */
