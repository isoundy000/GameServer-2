/*
 * GameNoticeSys.cpp
 *
 *  Created on: Aug 12, 2014
 *      Author: peizhibi
 */

#include "LogicMonitor.h"
#include "GameNoticeSys.h"
#include "LogicPlayer.h"

#include "BackField.h"
#include "DBCommon.h"
#include <mongo/client/dbclient.h>

int GameNoticeSys::RewardTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int GameNoticeSys::RewardTimer::handle_timeout(const Time_Value &nowtime)
{
	return GAME_NOTICE_SYS->handle_timeout();
}

GameNoticeSys::GameNoticeSys()
{
	// TODO Auto-generated constructor stub
}

GameNoticeSys::~GameNoticeSys()
{
	// TODO Auto-generated destructor stub
}

void GameNoticeSys::init()
{
	this->request_load_notice();
}

void GameNoticeSys::fina()
{

}

int GameNoticeSys::request_load_notice()
{
	return LOGIC_MONITOR->db_load_mode_begin(TRANS_LOAD_GAME_NOTICE);
}

int GameNoticeSys::after_load_notice(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode != NULL, -1);

	BSONObj& res = *shop_mode->output_argv_.bson_obj_;
	JUDGE_RETURN(res.isEmpty() == false, -1);
	JUDGE_RETURN(this->notice_detail_.tick_ < res[DBBackNotice::TICK].numberLong(), -1);

	this->notice_detail_.content_.clear();
	this->notice_detail_.item_set_.clear();
	this->reward_timer_.cancel_timer();

	this->notice_detail_.title_ = res[DBBackNotice::TITLE].str();
	this->notice_detail_.tick_ = res[DBBackNotice::TICK].numberLong();
	this->notice_detail_.notify_ = res[DBBackNotice::NOTIFY].numberInt();
	this->notice_detail_.start_tick_ = res[DBBackNotice::START_TICK].numberLong();

	{
		BSONObjIterator iter(res.getObjectField(DBBackNotice::CONTENT.c_str()));
		while (iter.more())
		{
			this->notice_detail_.content_.push_back(iter.next().str());
		}
	}

	{
		BSONObjIterator iter(res.getObjectField(DBBackNotice::ITEM_SET.c_str()));
		DBCommon::bson_to_item_vec(this->notice_detail_.item_set_, iter);
	}

	Int64 left_time = GameCommon::left_time(this->notice_detail_.start_tick_);
	if (left_time > 0 && this->notice_detail_.item_set_.empty() == false)
	{
		this->handle_timeout();
		this->reward_timer_.schedule_timer(Time_Value(left_time));
	}

	return 0;
}

int GameNoticeSys::is_need_view(Int64 last_view)
{
	JUDGE_RETURN(this->notice_detail_.notify_ == true, false);
	return this->notice_detail_.tick_ > last_view;
}

GameNoticeDetial& GameNoticeSys::notice_detail()
{
	return this->notice_detail_;
}

int GameNoticeSys::handle_timeout()
{
	this->reward_timer_.cancel_timer();

	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		player->check_and_game_notice_tips();
	}

	return 0;
}

