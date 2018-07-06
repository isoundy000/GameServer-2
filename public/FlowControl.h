/*
 * FlowControl.h
 *
 *  Created on: May 21, 2014
 *      Author: louis
 */

#ifndef FLOWCONTROL_H_
#define FLOWCONTROL_H_

#include "GameHeader.h"


class FlowControl
{
public:

	typedef std::set<std::string> ChannelSet;

	enum
	{
		MAX_VERSION_CNT = 2
	};

	struct FlowControlDetail
	{
		int __serial_record;
		int __money_serial_record;
		int __item_serial_record;
		int __equip_serial_record;
		int __mount_serial_record;
		int __pet_serial_record;
		int __skill_serial_record;
		int __mail_serial_record;
		int __market_serial_record;
		int __achieve_serial_record;
		int __is_forbit_login;
		int __player_level_record;
		int __other_record;
		int __online_user_record;
		int __login_record;
		int __task_record;
		int __rank_record;
		int __chat_record;

		IntVec __force_refresh_rank_type_set;
		ChannelSet __forbit_channel_set;

		FlowControlDetail();
		void reset();
	};
public:
	FlowControl();
	virtual ~FlowControl();

	int load_flow_detail_when_init();
	int check_flow_detail_type(const int type);
	bool is_forbit_channel(const std::string &channel);

	int request_load_flow_detail();

	int cur_version();
	int update_version();
	FlowControl::FlowControlDetail &flow_detail(void);
	FlowControl::FlowControlDetail &revert_flow_detail(void);
	void flow_debug();

	int check_force_refresh_rank_pannel(void);

	bool is_need_serial_record(const int check_type);
private:
	FlowControlDetail flow_detail_[MAX_VERSION_CNT];
	int cur_version_;
};

#define FLOW_INSTANCE Singleton<FlowControl>::instance()

#endif /* FLOWCONTROL_H_ */
