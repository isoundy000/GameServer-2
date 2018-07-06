/*
 * MLGameSwither.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: jinxing
 */

#include "MLGameSwither.h"
#include "MapMonitor.h"
#include "PlayerManager.h"
#include "MapLogicPlayer.h"
#include "ProtoDefine.h"

MLGameSwither::MLGameSwither()
{
	// TODO Auto-generated constructor stub

}

MLGameSwither::~MLGameSwither()
{
	// TODO Auto-generated destructor stub
}

void MLGameSwither::init(void)
{
	this->detail_.reset();
	this->request_sync_from_logic();
	MSG_USER("MLGameSwither::init...");
}

GameSwitcherDetail& MLGameSwither::detail(void)
{
//	Proto30101401 msg;
//	detail_.serialize(&msg);
	return this->detail_;
}

int MLGameSwither::request_sync_from_logic()
{
	return MAP_MONITOR->dispatch_to_logic(INNER_MAP_REQ_SYNC_GAME_SWITCHER_INFO);
}

int MLGameSwither::sync_from_logic(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101401*, res, -1);
	GameSwitcherDetail prev_detail = detail_;

	this->detail_.unserialize(res);
	this->process_detail_has_update(prev_detail);

	return 0;
}

int MLGameSwither::process_detail_has_update(GameSwitcherDetail& old)
{
	JUDGE_RETURN(this->detail_.has_update(old) == true, -1);

	PlayerManager::LogicPlayerSet logic_player_set;
	MAP_MONITOR->get_logic_player_set(logic_player_set);

	return 0;
}

bool MLGameSwither::map_check_switcher(const std::string& name)
{
	return false;
}
