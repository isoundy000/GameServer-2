/*
 * MMOWorldBoss.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: jinxing
 */

#include "LogicStruct.h"
#include "MMOWorldBoss.h"
#include "MongoDataMap.h"
#include "MongoData.h"
#include "TQueryCursor.h"
#include "MongoException.h"
#include "MongoConnector.h"
#include <mongo/client/dbclient.h>
#include "GameField.h"
#include "GameCommon.h"
#include "MapMapStruct.h"

MMOWorldBoss::MMOWorldBoss()
{
	// xTODO Auto-generated constructor stub

}

MMOWorldBoss::~MMOWorldBoss()
{
	// xTODO Auto-generated destructor stub
}


int MMOWorldBoss::load_wboss_info(WorldBossInfo* wboss_info)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBWorldBossNew::COLLECTION,
					QUERY(DBWorldBossNew::KEY << wboss_info->scene_id_));
	JUDGE_RETURN(res.isEmpty() == false, 0);

	wboss_info->status_ = res[DBWorldBossNew::STATUS].numberInt();
	wboss_info->die_tick_ = res[DBWorldBossNew::DIE_TICK].numberLong();
	return 0;
}

int MMOWorldBoss::update_wboss_info(WorldBossInfo* wboss_info, int direct_save)
{
	BSONObjBuilder builder;
	builder << DBWorldBossNew::SCENE_ID << wboss_info->scene_id_
			<< DBWorldBossNew::STATUS << wboss_info->status_
			<< DBWorldBossNew::DIE_TICK << wboss_info->die_tick_;

	if (direct_save == false)
	{
		GameCommon::request_save_mmo_begin(DBWorldBossNew::COLLECTION,
				BSON(DBWorldBossNew::KEY << wboss_info->scene_id_),
				BSON("$set" << builder.obj()));
	}
	else
	{
		CACHED_CONNECTION.update(DBWorldBossNew::COLLECTION,
				BSON(DBWorldBossNew::KEY << wboss_info->scene_id_),
				BSON("$set" << builder.obj()), true);
	}

	return 0;
}

int MMOWorldBoss::load_mattack_info(MAttackLabelRecord* label_record)
{
BEGIN_CATCH
	BSONObj res = CACHED_CONNECTION.findOne(DBMAttackLabelRecord::COLLECTION,
			QUERY(DBMAttackLabelRecord::KEY << label_record->label_id_));
	if (res.isEmpty())
		return 0;

	label_record->role_id_ = res[DBMAttackLabelRecord::ROLE_ID].numberLong();
	label_record->role_name_ = res[DBMAttackLabelRecord::ROLE_NAME].str();
	label_record->role_sex_ = res[DBMAttackLabelRecord::ROLE_SEX].numberInt();

	return 0;
END_CACHE_CATCH
	return -1;
}

int MMOWorldBoss::update_mattack_info(MAttackLabelRecord* label_record)
{
	BSONObjBuilder builder;
	builder << DBMAttackLabelRecord::ROLE_ID << label_record->role_id_
			<< DBMAttackLabelRecord::ROLE_NAME << label_record->role_name_
			<< DBMAttackLabelRecord::ROLE_SEX << label_record->role_sex_;

	GameCommon::request_save_mmo_begin(DBMAttackLabelRecord::COLLECTION,
			BSON(DBMAttackLabelRecord::KEY << label_record->label_id_), BSON("$set" << builder.obj()));

	return 0;
}

void MMOWorldBoss::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBWorldBossNew::COLLECTION, BSON(DBWorldBossNew::KEY << 1), true);
    this->conection().ensureIndex(DBMAttackLabelRecord::COLLECTION, BSON(DBMAttackLabelRecord::KEY << 1), true);
END_CATCH
}
