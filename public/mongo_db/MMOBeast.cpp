/*
 * MMOBeast.cpp
 *
 *  Created on: Nov 21, 2013
 *      Author: peizhibi
 */

#include "MMOBeast.h"
#include "GameField.h"
#include "PubStruct.h"
#include "MapStruct.h"
#include "MapMonitor.h"
#include "MapBeast.h"
#include "GameConfig.h"

#include "MongoDataMap.h"
#include "MapLogicPlayer.h"
#include "MapPlayerEx.h"

#include "DBCommon.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOBeast::MMOBeast()
{
	// TODO Auto-generated constructor stub
}

MMOBeast::~MMOBeast()
{
	// TODO Auto-generated destructor stub
}

void MMOBeast::load_master(MapLogicPlayer* player)
{
	BSONObj res = this->conection().findOne(DBMaster::COLLECTION,
			QUERY(DBMaster::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	int mount_type = GameEnum::FUN_MOUNT;
	BSONObjIterator mount_iter(res.getObjectField(DBMaster::MOUNT_SET.c_str()));
	while (mount_iter.more())
	{
		BSONObj mount_obj = mount_iter.next().embeddedObject();
		MountDetail& mount_detail = player->mount_detail(mount_type);

		int grade = std::max<int>(mount_obj[DBMaster::MOUNT_GRADE].numberInt(), 1);
		mount_detail.set_grade(grade);
		mount_detail.open_ = mount_obj[DBMaster::OPEN].numberInt();
		mount_detail.bless_ = mount_obj[DBMaster::BLESS].numberInt();
		mount_detail.fail_times_ = mount_obj[DBMaster::FAIL_TIMES].numberInt();
		mount_detail.finish_bless_ = mount_obj[DBMaster::FINISH_BLESS].numberLong();
		mount_detail.on_mount_ = mount_obj[DBMaster::ON_MOUNT].numberInt();
		mount_detail.mount_shape_ = mount_obj[DBMaster::MOUNT_SHAPE].numberInt();
		mount_detail.ability_amount_ = mount_obj[DBMaster::ABILITY].numberInt();
		mount_detail.growth_amount_ = mount_obj[DBMaster::GROWTH].numberInt();
		mount_detail.act_shape_ = mount_obj[DBMaster::ACT_SHAPE].numberInt();
		mount_detail.sword_pool_level_ = mount_obj[DBMaster::SPOOL_LEVEL].numberInt();

		BSONObjIterator skill_iter(mount_obj.getObjectField(DBMaster::SKILL.c_str()));
		while (skill_iter.more())
		{
			BSONObj skill_obj = skill_iter.next().embeddedObject();

			int id = skill_obj[Skill::SSkill::SKILL_ID].numberInt();
			int lvl = skill_obj[Skill::SSkill::LEVEL].numberInt();
			JUDGE_CONTINUE(id > 0);

			mount_detail.add_new_skill(id, lvl);
		}

		++mount_type;
	}
}

void MMOBeast::load_map_master(MapPlayerEx *player)
{
}

void MMOBeast::save_master(MapLogicPlayer* player, MongoDataMap* data_map)
{
	BSONVec mount_vec;
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
	    BSONObjBuilder builder;
		MountDetail& mount_detail = player->mount_detail(i);

		BSONVec skill_vec;
		DBCommon::skill_map_to_bson(skill_vec, mount_detail.skill_map_);

	    builder << DBMaster::MOUNT_GRADE << mount_detail.mount_grade_
	    		<< DBMaster::OPEN << mount_detail.open_
	    		<< DBMaster::BLESS << mount_detail.bless_
	    		<< DBMaster::FAIL_TIMES << mount_detail.fail_times_
	    		<< DBMaster::FINISH_BLESS << mount_detail.finish_bless_
	    		<< DBMaster::ON_MOUNT << mount_detail.on_mount_
	    		<< DBMaster::MOUNT_SHAPE << mount_detail.mount_shape_
	    		<< DBMaster::ABILITY << mount_detail.ability_amount_
	    		<< DBMaster::GROWTH << mount_detail.growth_amount_
	    		<< DBMaster::ACT_SHAPE << mount_detail.act_shape_
	    		<< DBMaster::SPOOL_LEVEL << mount_detail.sword_pool_level_
	    		<< DBMaster::SKILL << skill_vec;
	    mount_vec.push_back(builder.obj());
	}

	BSONObjBuilder builder;
    builder << DBMaster::MOUNT_SET << mount_vec
    		<< DBMaster::MASTER_NAME << player->role_name();

    data_map->push_update(DBMaster::COLLECTION,
    		BSON(DBMaster::ID << player->role_id()), builder.obj());
}


void MMOBeast::ensure_all_index()
{
	this->conection().ensureIndex(DBMaster::COLLECTION, BSON(DBMaster::ID << 1), true);
}
