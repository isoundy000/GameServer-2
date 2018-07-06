/*
 * TinyLogicPlayer.cpp
 *
 *  Created on: Aug 12, 2014
 *      Author: peizhibi
 */

#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "GameNoticeSys.h"
#include "TinyLogicPlayer.h"

TinyLogicPlayer::TinyLogicPlayer()
{
	// TODO Auto-generated constructor stub

}

TinyLogicPlayer::~TinyLogicPlayer()
{
	// TODO Auto-generated destructor stub
}

void TinyLogicPlayer::reset_tiny()
{

}

int TinyLogicPlayer::check_and_game_notice_tips()
{
	GameNoticeDetial& notice_detail = GAME_NOTICE_SYS->notice_detail();
	JUDGE_RETURN(notice_detail.item_set_.empty() == false, -1);
	JUDGE_RETURN(this->role_detail().draw_flag(notice_detail.start_tick_) == true, -1);

	Proto80400367 notice_info;
	if (::time(NULL) >= notice_detail.start_tick_)
	{
		notice_info.set_tips_state(2);
	}
	else
	{
		notice_info.set_tips_state(1);
	}

	FINER_PROCESS_RETURN(ACTIVE_GAME_NOTICE_REWARD, &notice_info);
}

int TinyLogicPlayer::fetch_game_notice_info()
{
	GameNoticeDetial& notice_detail = GAME_NOTICE_SYS->notice_detail();
	this->role_detail().view_tick_ = notice_detail.tick_;

	Proto50100032 notice_info;
	notice_info.set_title(notice_detail.title_);
	notice_info.set_start_tick(notice_detail.start_tick_);

	if (::time(NULL) < notice_detail.start_tick_)
	{
		notice_info.set_draw_flag(2);
	}
	else
	{
		notice_info.set_draw_flag(this->role_detail().draw_flag(
			notice_detail.start_tick_));
	}

	for (StringVec::iterator iter = notice_detail.content_.begin();
			iter != notice_detail.content_.end(); ++iter)
	{
		notice_info.add_content(*iter);
	}

	for (ItemObjVec::iterator iter = notice_detail.item_set_.begin();
			iter != notice_detail.item_set_.end(); ++iter)
	{
		ProtoItem* proto_item = notice_info.add_item_set();
		iter->serialize(proto_item);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_NOTICE_INFO, &notice_info);
}

int TinyLogicPlayer::draw_game_notice_reward_begin()
{
	GameNoticeDetial& notice_detail = GAME_NOTICE_SYS->notice_detail();

	CONDITION_NOTIFY_RETURN(notice_detail.item_set_.empty() == false,
			RETURN_DRAW_NOTICE_REWARD, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(::time(NULL) >= notice_detail.start_tick_,
			RETURN_DRAW_NOTICE_REWARD, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->role_detail().draw_flag(notice_detail.start_tick_) == true,
			RETURN_DRAW_NOTICE_REWARD, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->role_detail().prev_draw_ == false,
			RETURN_DRAW_NOTICE_REWARD, ERROR_CLIENT_OPERATE);

	Proto31400313 reward_info;
	for (ItemObjVec::iterator iter = notice_detail.item_set_.begin();
			iter != notice_detail.item_set_.end(); ++iter)
	{
		ProtoItem* proto_item = reward_info.add_item_set();
		iter->serialize(proto_item);
	}

	this->role_detail().prev_draw_ = true;
	LOGIC_MONITOR->dispatch_to_scene(this, &reward_info);

	return 0;
}

int TinyLogicPlayer::draw_game_notice_reward_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400313*, request, -1);

	this->role_detail().prev_draw_ = false;
	if (request->oper_result() == 0)
	{
		this->role_detail().draw_tick_ = ::time(NULL);
		FINER_PROCESS_NOTIFY(RETURN_DRAW_NOTICE_REWARD);
	}
	else
	{
		return this->respond_to_client_error(RETURN_DRAW_NOTICE_REWARD,
				request->oper_result());
	}
}
