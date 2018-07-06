/*
 * FlowControl.cpp
 *
 *  Created on: May 21, 2014
 *      Author: louis
 */

#include "FlowControl.h"
#include "BackFlowControl.h"
#include "MongoDataMap.h"
#include "PoolMonitor.h"
#include "RankSystem.h"

FlowControl::FlowControl() : cur_version_(0)
{
	// TODO Auto-generated constructor stub

}

FlowControl::~FlowControl()
{
	// TODO Auto-generated destructor stub
}

FlowControl::FlowControlDetail::FlowControlDetail()
{
	FlowControlDetail::reset();
}

void FlowControl::FlowControlDetail::reset()
{
	__serial_record = 1;
	__money_serial_record = 1;
	__item_serial_record = 1;
	__equip_serial_record = 1;
	__mount_serial_record = 1;
	__pet_serial_record = 1;
	__skill_serial_record = 1;
	__mail_serial_record = 1;
	__market_serial_record = 1;
	__achieve_serial_record = 1;
	__is_forbit_login = 0;
	__player_level_record = 1;
	__other_record = 1;
	__online_user_record = 1;
	__login_record = 1;
	__task_record = 1;
	__rank_record = 1;
	__chat_record = 1;
	this->__force_refresh_rank_type_set.clear();
    this->__forbit_channel_set.clear();
}

int FlowControl::load_flow_detail_when_init()
{
	BackFlowControl::load_flow_control_detail(this);
	MSG_USER("FlowControl...");
	return 0;
}

int FlowControl::request_load_flow_detail()
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, 0);

	if(TRANSACTION_MONITOR->request_mongo_transaction(0,
			TRANS_CHECK_LOAD_BACK_FLOW_CONTROL, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
	}
	return 0;
}

int FlowControl::check_flow_detail_type(const int type)
{
	switch(type)
	{
	case GameEnum::SERIAL_RECORD:
		return this->flow_detail().__serial_record;
	case GameEnum::MONEY_SERIAL_RECORD:
		return this->flow_detail().__money_serial_record;
	case GameEnum::ITEM_SERIAL_RECORD:
		return this->flow_detail().__item_serial_record;
	case GameEnum::EQUIP_SERIAL_RECORD:
		return this->flow_detail().__equip_serial_record;
	case GameEnum::MOUNT_SERIAL_RECORD:
		return this->flow_detail().__mount_serial_record;
	case GameEnum::PET_SERIAL_RECORD:
		return this->flow_detail().__pet_serial_record;
	case GameEnum::SKILL_SERIAL_RECORD:
		return this->flow_detail().__skill_serial_record;
	case GameEnum::MAIL_SERIAL_RECORD:
		return this->flow_detail().__mail_serial_record;
	case GameEnum::MARKET_SERIAL_RECORD:
		return this->flow_detail().__market_serial_record;
	case GameEnum::ACHIEVEMENT_SERIAL_RECORD:
		return this->flow_detail().__achieve_serial_record;
	case GameEnum::IS_FORBIT_LOGIN:
		return this->flow_detail().__is_forbit_login;
	case GameEnum::PLAYER_LEVEL_RECORD:
		return this->flow_detail().__player_level_record;
	case GameEnum::OTHER_SERIAL_RECORD:
		return this->flow_detail().__other_record;
	case GameEnum::ONLINE_USER_RECORD:
		return this->flow_detail().__online_user_record;
	case GameEnum::LOGIN_RECORD:
		return this->flow_detail().__login_record;
	case GameEnum::TASK_RECORD:
		return this->flow_detail().__task_record;
	case GameEnum::RANK_RECORD:
		return this->flow_detail().__rank_record;
	case GameEnum::CHAT_RECORD:
		return this->flow_detail().__chat_record;

	default:
		break;
	}
	this->flow_debug();
	return 0;
}

bool FlowControl::is_forbit_channel(const std::string &channel)
{
	FlowControl::FlowControlDetail &detail = this->flow_detail();

	if (detail.__forbit_channel_set.size() <= 0)
		return true;

	if (detail.__forbit_channel_set.find(channel) == detail.__forbit_channel_set.end())
		return false;
	return true;
}

FlowControl::FlowControlDetail& FlowControl::flow_detail()
{
	return this->flow_detail_[this->cur_version()];
}

FlowControl::FlowControlDetail &FlowControl::revert_flow_detail(void)
{
	int index = (this->cur_version_ + 1) % MAX_VERSION_CNT;
	return this->flow_detail_[index];
}

int FlowControl::cur_version()
{
	return this->cur_version_;
}

int FlowControl::update_version()
{
	this->cur_version_ = (this->cur_version_ + 1) % MAX_VERSION_CNT;
	return 0;
}

int FlowControl::check_force_refresh_rank_pannel(void)
{
	FlowControlDetail& detail = this->flow_detail();
	JUDGE_RETURN(detail.__force_refresh_rank_type_set.empty() == false, 0);

	IntVec& refresh_set = detail.__force_refresh_rank_type_set;
	for (IntVec::iterator it = refresh_set.begin(); it != refresh_set.end(); ++it)
	{
		int rank_type = *it;
		RANK_SYS->request_refresh_rank_data(rank_type);
	}

	refresh_set.clear();
	return 0;
}

bool FlowControl::is_need_serial_record(const int check_type)
{
#ifdef LOCAL_DEBUG
#ifdef TEST_SERIAL
	return true;
#endif
#endif
	JUDGE_RETURN(this->check_flow_detail_type(GameEnum::SERIAL_RECORD), false);
	JUDGE_RETURN(this->check_flow_detail_type(check_type), false);
	return true;
}


void FlowControl::flow_debug()
{
	std::stringstream ss;
	for(int i = 0; i < 2; ++i)
	{
		int version = i;
		ss << "flow_detail: \n" << "\t\t version : " << "\t" << version
				<< "serial_record: " << this->flow_detail_[version].__serial_record << "\n"
				<< "money_serial: " << this->flow_detail_[version].__money_serial_record << "\n"
				<< "equip_serial: " << this->flow_detail_[version].__equip_serial_record << "\n"
				<< "item_serial: " << this->flow_detail_[version].__item_serial_record << "\n"
				<< "\n\n";
	}

	MSG_USER(%s, ss.str().c_str());
}
