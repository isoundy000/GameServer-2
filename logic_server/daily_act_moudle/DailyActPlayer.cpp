/*
 * DailyActPlayer.cpp
 *
 *  Created on: 2017年3月21日
 *      Author: lyw
 */

#include "DailyActPlayer.h"
#include "DailyActSys.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"

DailyActPlayer::DailyActPlayer() {
	// TODO Auto-generated constructor stub

}

DailyActPlayer::~DailyActPlayer() {
	// TODO Auto-generated destructor stub
}

void DailyActPlayer::reset()
{

}

int DailyActPlayer::fetch_daily_act_list()
{
	Proto50102201 respond;
	IntMap &act_map = DAILY_ACT_SYSTEM->fetch_all_activity();
	for (IntMap::iterator iter = act_map.begin(); iter != act_map.end(); ++iter)
	{
		DailyActivity::DailyDetail *daily_detail = DAILY_ACT_SYSTEM->fetch_daily_detail(
				iter->first, this->role_detail().__agent_code);
		JUDGE_CONTINUE(daily_detail != NULL);

		respond.add_activity_set(iter->first);
	}
	MSG_USER("Proto50102201: %s", respond.Utf8DebugString().c_str());

	FINER_PROCESS_RETURN(RETURN_DAILY_ACT_LIST, &respond);
}

int DailyActPlayer::request_total_double_info()
{
	int activity_id = DailyActivity::ACT_TOTAL_DOUBLE;
	DailyActivity::DailyDetail *daily_detail = DAILY_ACT_SYSTEM->fetch_daily_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(daily_detail != NULL, 0);

	Proto50102202 respond;
	respond.set_value1(daily_detail->value1_);
	respond.set_value2(daily_detail->value2_);
	respond.set_left_tick(daily_detail->fetch_left_tick());
	FINER_PROCESS_RETURN(RETURN_TOTAL_DOUBLE_ACT, &respond);
}

int DailyActPlayer::is_xuanji_double()
{
	int activity_id = DailyActivity::ACT_TOTAL_DOUBLE;
	DailyActivity::DailyDetail *daily_detail = DAILY_ACT_SYSTEM->fetch_daily_detail(
			activity_id, this->role_detail().__agent_code);

	if (daily_detail != NULL)
	{
		JUDGE_RETURN(daily_detail->value1_ == DailyActivity::DOUBLE_XUANJI
				|| daily_detail->value2_ == DailyActivity::DOUBLE_XUANJI, false);

		return true;
	}
	else
	{
		return false;
	}
}
