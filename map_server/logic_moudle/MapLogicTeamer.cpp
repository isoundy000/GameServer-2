/*
 * MapLogicTeamer.cpp
 *
 *  Created on: Jul 29, 2013
 *      Author: peizhibi
 */

#include "MapLogicTeamer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

MapLogicTeamer::MapLogicTeamer()
{
	// TODO Auto-generated constructor stub

}

MapLogicTeamer::~MapLogicTeamer()
{
	// TODO Auto-generated destructor stub
}

MapTeamInfo& MapLogicTeamer::team_info(int type)
{
	return this->team_info_[type];
}

void MapLogicTeamer::reset()
{
	for (int i = 0; i < GameEnum::TOTAL_TEAM; ++i)
	{
		this->team_info_[i].reset();
	}
}

int MapLogicTeamer::teamer_state(int type)
{
	MapTeamInfo& team_info = this->team_info(type);
	JUDGE_RETURN(team_info.team_index() > 0, GameEnum::TEAMER_STATE_NONE);
	return team_info.leader_id_ == this->role_id() ? GameEnum::TEAMER_STATE_LEADER
			: GameEnum::TEAMER_STATE_TEAMER;
}

int MapLogicTeamer::team_index(int type)
{
	return this->team_info_[type].team_index();
}

int MapLogicTeamer::team_learn_circle(Message* msg)
{
	return 0;
}

int MapLogicTeamer::read_logic_team_info(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto31400211*, request, -1);
//
//	int type = GameEnum::NORMAL_TEAM;
//	if (request->travel_team() == true)
//	{
//		type = GameEnum::TRAVEL_TEAM;
//	}
//
//	MapTeamInfo& team_info = this->team_info(type);
//	team_info.reset();
//	team_info.team_index_ = request->team_index();
//	team_info.leader_id_ = request->leader_id();
//
//	for (int i = 0; i < request->teamer_set_size(); ++i)
//	{
//		int64_t teamer_id = request->teamer_set(i);
//		team_info.teamer_set_[teamer_id] = teamer_id;
//	}
//
//	for(int i = 0; i < request->replacement_set_size(); ++i)
//	{
//		int64_t replacement_id = request->replacement_set(i);
//		team_info.replacement_set_[replacement_id] = replacement_id;
//	}

	return 0;
}

int MapLogicTeamer::sync_transfer_team_info(int scene_id)
{
//	Proto31400117 team_info;
//	team_info.set_team_id(this->team_index());
//	team_info.set_leader_id(this->team_info_.leader_id_);
//
//	for(LongMap::iterator iter = this->team_info_.teamer_set_.begin();
//			iter != this->team_info_.teamer_set_.end(); ++iter)
//	{
//		JUDGE_CONTINUE(this->role_id() != iter->first);
//		team_info.add_teamer_id(iter->first);
//	}
//
//	return this->send_to_other_logic_thread(scene_id, team_info);
	return 0;
}

int MapLogicTeamer::read_transfer_team_info(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto31400117*, request, -1);
//	this->team_info_.team_index_ = request->team_id();
//	this->team_info_.leader_id_ = request->leader_id();
//
//	this->team_info_.teamer_set_.clear();
//	for(int i=0; i < request->teamer_id_size(); ++i)
//	{
//		this->team_info_.teamer_set_[request->teamer_id(i)] = request->teamer_id(i);
//	}

	return 0;
}

//int MapTeamer::nearby_team_info(Message* msg)
//{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto11400207*, request, RETURN_TEAM_NEARBY);
//
//	IntMap team_map;		// nearby team
//	LongMap player_map;		// nearby player
//	PLAYER_MANAGER->fetch_team_and_player_info(this->role_id(),
//			team_map, player_map);
//
//	Proto30100303 nearby_info;
//	for (IntMap::iterator iter = team_map.begin(); iter != team_map.end();
//			++iter)
//	{
//		nearby_info.add_team_set(iter->first);
//	}
//
//	for (LongMap::iterator iter = player_map.begin(); iter != player_map.end();
//			++iter)
//	{
//		nearby_info.add_player_set(iter->first);
//	}
//
//	nearby_info.set_page_index(std::max(request->page_index(), 1));
//	nearby_info.set_sort_type(request->sort_type());
//	MAP_MONITOR->dispatch_to_logic(this, &nearby_info);
//
//	FINER_PROCESS_NOTIFY(RETURN_TEAM_NEARBY);
//}
//
