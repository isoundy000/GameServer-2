/*
 * MMOPlayerTip.cpp
 *
 *  Created on: May 7, 2014
 *      Author: louis
 */

#include "MongoConnector.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

#include "MMOPlayerTip.h"
#include "PlayerAssist.h"
#include "MapMonitor.h"
#include "GameField.h"
#include "MongoDataMap.h"
#include "MapLogicPlayer.h"
#include "DBCommon.h"

MMOPlayerTip::MMOPlayerTip() {
	// TODO Auto-generated constructor stub

}

MMOPlayerTip::~MMOPlayerTip() {
	// TODO Auto-generated destructor stub
}


int MMOPlayerTip::load_player_tip_pannel(MapLogicPlayer *player)
{
	BSONObj res = this->conection().findOne(DBPlayerTip::COLLECTION,
			QUERY(DBPlayerTip::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);


	PlayerAssist::PATipPannel &pannel = player->assist_tip_pannel();
	BSONObjIterator it(res.getObjectField(DBPlayerTip::TIP_DETAIL.c_str()));
	while(it.more())
	{
		BSONObj obj = it.next().embeddedObject();

		int event_id = obj[DBPlayerTip::TipDetail::EVENT_ID].numberInt();
		JUDGE_CONTINUE(CONFIG_INSTANCE->validate_red_tips(event_id) == true);

		PlayerAssistTip *tip = &(pannel[event_id]);
		DBCommon::bson_to_player_tip(tip, obj);
	}

	return 0;
}

int MMOPlayerTip::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	PlayerAssist::PATipPannel &pannel = player->assist_tip_pannel();

	BSONVec tips_set;
	for(PlayerAssist::PATipPannel::iterator iter = pannel.begin();
			iter != pannel.end(); ++iter)
	{
		PlayerAssistTip *tip = &(iter->second);
		JUDGE_CONTINUE(tip->__tips_flag > 0);
		tips_set.push_back(DBCommon::player_tip_to_bson(tip));
	}

	BSONObjBuilder build;
	build << DBPlayerTip::TIP_DETAIL << tips_set;
	data_map->push_update(DBPlayerTip::COLLECTION, BSON(
			DBPlayerTip::ID << player->role_id()), build.obj(), true);

	return 0;
}

void MMOPlayerTip::ensure_all_index(void)
{
	this->conection().ensureIndex(DBPlayerTip::COLLECTION, BSON(DBPlayerTip::ID << 1), true);
}
