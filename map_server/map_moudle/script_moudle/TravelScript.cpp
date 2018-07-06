/*
 * TravelScript.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: peizhibi
 */

#include "TravelScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "GameCommon.h"
#include "ProtoClient018.pb.h"

TravelScript::TravelScript() {
	// TODO Auto-generated constructor stub

}

TravelScript::~TravelScript() {
	// TODO Auto-generated destructor stub
}


void TravelScript::reset(void)
{
	BaseScript::reset();
}

int TravelScript::notify_fight_hurt_detail()	//DPS排行榜
{
	ThreeObjVec rank_vec;
	ScriptDetail::Team& team = this->script_detail_.__team;
	ScriptDetail::PlayerAwardMap& award_map = this->script_detail_.__player_award_map;

	for (LongStrMap::iterator iter = team.__teamer_map.begin();
			iter != team.__teamer_map.end();  ++iter)
	{
		ThreeObj obj;
		obj.id_ = iter->first;
		obj.value_ = award_map[iter->first].__dps;
		rank_vec.push_back(obj);
	}

	std::stable_sort(rank_vec.begin(), rank_vec.end(), GameCommon::three_comp_by_desc);

	Proto81400112 respond;
	respond.set_script_id(this->script_sort());

	int rank = 1;
	for (ThreeObjVec::iterator iter = rank_vec.begin(); iter != rank_vec.end(); ++iter)
	{
		ProtoLScoreInfo* proto = respond.add_rank_list();
		proto->set_role_rank(rank);
		proto->set_role_name(team.__teamer_map[iter->id_]);
		proto->set_role_hurt(iter->value_);
		++rank;
	}

	return this->notify_all_player(ACTIVE_TRVL_SCRIPT_HURT_INFO, &respond);
}

void TravelScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}
