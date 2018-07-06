/*
 * SendActReward.cpp
 *
 *  Created on: 2016年11月21日
 *      Author: lzy
 */

#include "SendActReward.h"
#include "ProtoDefine.h"

SendActReward::SendActReward()
{
	this->refresh_timer_.cancel_timer();
}
SendActReward::~SendActReward()
{

}

int SendActReward::RefreshTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int SendActReward::RefreshTimer::handle_timeout(const Time_Value &tv)
{
	return SEND_ACTREWARD->reset();
}

int SendActReward::reset()
{
	this->reward_map_.clear();
	this->refresh_timer_.cancel_timer();
	this->refresh_timer_.schedule_timer((int)(::next_day(0,0).sec() - Time_Value::gettimeofday().sec()));
	return 0;
}

void SendActReward::init()
{
	MSG_USER("SEND_REWARD %d %d %d", this->reward_map_.size());
	this->reward_map_.clear();
	this->refresh_timer_.cancel_timer();
	this->refresh_timer_.schedule_timer((int)(::next_day(0,0).sec() - Time_Value::gettimeofday().sec()));

	MSG_USER("SEND_REWARD %d %d %d", this->refresh_timer_.left_second());
}

int SendActReward::send_player_act_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30103001*, request, -1);
	Int64 role_id = request->role_id();
	int mail_id = request->mail_id();
	int type = request->act_type();
	IntMap &role_map = this->reward_map_[role_id];
	JUDGE_RETURN(role_map[type] == 0 && request->reward_list_size() > 0, -1);

	role_map[type] = 1;

	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str());

	for (int i = 0; i < request->reward_list_size(); ++i)
	{
		mail_info->add_goods(request->reward_list(i));
	}

	GameCommon::request_save_mail_content(role_id, mail_info);
	return 0;
}

