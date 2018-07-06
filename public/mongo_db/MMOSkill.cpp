/*
 * MMOSkill.cpp
 *
 * Created on: 2013-06-18 10:01
 *     Author: lyz
 */

#include "GameField.h"
#include "MMOSkill.h"
#include "MongoConnector.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"

#include "DBCommon.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOSkill::~MMOSkill(void)
{ /*NULL*/ }

int MMOSkill::load_player_skill(MapPlayerEx *player)
{
    BSONObj res = this->conection().findOne(Skill::COLLECTION,
    		QUERY(Skill::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, -1);

    SkillerDetail& skiller_detail = player->skiller_detail();
    GameCommon::bson_to_map(skiller_detail.rama_skill_list_,
    		res.getObjectField(Skill::RAMA_LIST.c_str()));

    skiller_detail.current_scheme_ = res[Skill::CUR_SCHEME].numberInt();
    skiller_detail.rama_skill_ = res[Skill::CUR_RAMA].numberInt();

	if (res.hasField(Skill::SCHEME_LIST.c_str()))
	{
		SkillerDetail::SchemeList &scheme_list = player->scheme_list();
		BSONObjIterator scheme_iter(res.getObjectField(Skill::SCHEME_LIST.c_str()));

		int i = 0, j = 0;
		while (scheme_iter.more())
		{
			SkillerDetail::SkillIdList &id_list = scheme_list[i++];

			BSONObj scheme_obj = scheme_iter.next().embeddedObject();
			BSONObjIterator id_iter(scheme_obj);

			j = 0;
			while (id_iter.more())
			{
				id_list[j++] = id_iter.next().numberInt();
			}
		}
	}

	BSONObjIterator iter(res.getObjectField(Skill::SKILL.c_str()));
	while (iter.more())
	{
		BSONObj skill_obj = iter.next().embeddedObject();

		int skill_id = skill_obj[Skill::SSkill::SKILL_ID].numberInt();
		JUDGE_CONTINUE(player->validate_init_insert_skill(skill_id) == true);

		int skill_level = skill_obj[Skill::SSkill::LEVEL].numberInt();
		player->insert_skill(skill_id, skill_level);

		FighterSkill *skill = 0;
		JUDGE_CONTINUE(player->find_skill(skill_id, skill) == 0);

		skill->__used_times = skill_obj[Skill::SSkill::USED_TIMES].numberInt();
	}

	if (skiller_detail.rama_skill_ > 0)
	{
		player->insert_skill(skiller_detail.rama_skill_, 1);
	}

    return 0;
}

int MMOSkill::update_data(MapPlayerEx *player, MongoDataMap *mongo_data)
{
    FightDetail& fight_detail = player->fight_detail();
    SkillerDetail& skiller_detail = player->skiller_detail();

    BSONVec skill_vc;
    DBCommon::skill_map_to_bson(skill_vc, fight_detail.__skill_map);

    BSONVec rama_vc;
    GameCommon::map_to_bson(rama_vc, skiller_detail.rama_skill_list_);

    BSONObjBuilder builder;
    builder << Skill::SKILL << skill_vc
        << Skill::CUR_SCHEME << skiller_detail.current_scheme_
        << Skill::SCHEME_LIST << skiller_detail.scheme_list_
        << Skill::CUR_RAMA << skiller_detail.rama_skill_
        << Skill::RAMA_LIST << rama_vc;

    mongo_data->push_update(Skill::COLLECTION, BSON(Skill::ID << player->role_id()), builder.obj(), true);
    return 0;
}

void MMOSkill::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(Skill::COLLECTION, BSON(Skill::ID << 1), true);
END_CATCH
}

