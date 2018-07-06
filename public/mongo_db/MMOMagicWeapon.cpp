/*
 * MMOMagicWeapon.cpp
 *
 *  Created on: 2015-12-15
 *      Author: xu
 */

#include "GameField.h"
#include "MMOMagicWeapon.h"
#include "MongoConnector.h"

//#include "MapPlayer.h"
#include "MapLogicPlayer.h"

#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOMagicWeapon::MMOMagicWeapon()
{

}

MMOMagicWeapon::~MMOMagicWeapon()
{

}

int MMOMagicWeapon::load_player_magic_weapon(MapLogicPlayer* player)
{
   BSONObj res = this->conection().findOne(DBMagicWeapon::COLLECTION,
		   QUERY(DBMagicWeapon::ROLEID << player->role_id()));
   JUDGE_RETURN(res.isEmpty() == false, 0);

   MagicWeaponMap &detail = player->magicweapon_list();
   player->set_rama_open(res[DBMagicWeapon::OPEN].numberInt());

   BSONObjIterator iter(res.getObjectField(DBMagicWeapon::MAGICW_LIST.c_str()));
   while (iter.more())
   {
	  BSONObj magicw_obj = iter.next().embeddedObject();
	  JUDGE_CONTINUE(magicw_obj.isEmpty() == false);

	  MagicWeaponDetail temp;
	  temp.mw_id_ = magicw_obj[DBMagicWeapon::MAGICW_ID].numberInt();
	  JUDGE_CONTINUE(temp.validate_id() == true);

	  temp.mw_skill_id_ = magicw_obj[DBMagicWeapon::MAGICW_SKILL_ID].numberInt();
	  temp.mw_skill_lvl_= magicw_obj[DBMagicWeapon::MAGICW_SKILL_LVL].numberInt();
	  temp.mw_is_activate_ =  magicw_obj[DBMagicWeapon::ACTIVED_STATE].numberInt();
	  temp.mw_is_adorn_ = magicw_obj[DBMagicWeapon::IS_ADORN].trueValue();
	  temp.mw_rank_.rank_star_grade_ = magicw_obj[DBMagicWeapon::MAGICW_RANK_GRADE].numberInt();
	  temp.mw_rank_.rank_curr_star_progress_ = magicw_obj[DBMagicWeapon::MAGICW_RANK_PROGRESS].numberInt();
	  temp.mw_quality_.qua_star_grade_ = magicw_obj[DBMagicWeapon::MAGICW_QUALITY_GRADE].numberInt();
	  temp.mw_quality_.qua_curr_star_progress_ = magicw_obj[DBMagicWeapon::MAGICW_QUALITY_PROGRESS].numberInt();
	  detail.insert(MagicWeaponMap::value_type(temp.mw_id_, temp));

	  if (temp.mw_is_adorn_)
	  {
		  player->set_talisman_id(temp.mw_id_);
		  player->set_talisman_rank_lvl(temp.mw_rank_.rank_star_grade_);
	  }

	  if (temp.mw_rank_.rank_star_grade_ >= 10)
	  {
		  player->count_of_zhuling();
	  }
   }

   return 0;
}

int MMOMagicWeapon::load_player_magic_weapon(MapPlayer *player)
{
//	BSONObj res = this->conection().findOne(DBMagicWeapon::COLLECTION,
//			QUERY(DBMagicWeapon::ROLEID << player->role_id()));
//	JUDGE_RETURN(res.isEmpty() == false, 0);
//
//   bool	befind = false;
//   BSONObjIterator iter(res.getObjectField(DBMagicWeapon::MAGICW_LIST.c_str()));
//   while (iter.more())
//   {
//	  BSONObj magicw_obj = iter.next().embeddedObject();
//
//	  MagicWeaponDetail temp;
//	  temp.mw_id_ = magicw_obj[DBMagicWeapon::MAGICW_ID].numberInt();
//	  JUDGE_CONTINUE(temp.validate_id() == true);
//	  JUDGE_CONTINUE(CONFIG_INSTANCE->magic_weapon(temp.mw_id_) != Json::Value::null);
//
//	  temp.mw_skill_id_ = magicw_obj[DBMagicWeapon::MAGICW_SKILL_ID].numberInt();
//	  temp.mw_skill_lvl_= magicw_obj[DBMagicWeapon::MAGICW_SKILL_LVL].numberInt();
//	  temp.mw_is_adorn_ = magicw_obj[DBMagicWeapon::IS_ADORN].trueValue();
//	  temp.mw_is_activate_ =  magicw_obj[DBMagicWeapon::ACTIVED_STATE].numberInt();
//	  temp.mw_rank_.rank_star_grade_ = magicw_obj[DBMagicWeapon::MAGICW_RANK_GRADE].numberInt();
//	  temp.mw_rank_.rank_curr_star_progress_ = magicw_obj[DBMagicWeapon::MAGICW_RANK_PROGRESS].numberInt();
//	  temp.mw_quality_.qua_star_grade_ = magicw_obj[DBMagicWeapon::MAGICW_QUALITY_GRADE].numberInt();
//	  temp.mw_quality_.qua_curr_star_progress_ = magicw_obj[DBMagicWeapon::MAGICW_QUALITY_PROGRESS].numberInt();
//
//	  if (temp.mw_is_adorn_ && !befind )
//	  {
//		  befind = true;
//		  if(temp.mw_skill_id_ > 0)
//		 	player->init_magic_weapon_skill(temp.mw_skill_id_, temp.mw_skill_lvl_);
//
//		  player->set_magic_weapon_id(temp.mw_id_);
//		  player->set_magic_weapon_lvl(temp.mw_rank_.rank_star_grade_);
//
//	  }
//   }

   return 0;
}

int MMOMagicWeapon::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
    BSONVec magicw_vc;
    MagicWeaponMap &detail = player->magicweapon_list();

	for (MagicWeaponMap::iterator iter = detail.begin(); iter != detail.end(); ++iter)
	{
		MagicWeaponDetail& temp = iter->second;
		magicw_vc.push_back(BSON(DBMagicWeapon::MAGICW_ID << temp.mw_id_
					<< DBMagicWeapon::MAGICW_SKILL_ID << temp.mw_skill_id_
					<< DBMagicWeapon::MAGICW_SKILL_LVL << temp.mw_skill_lvl_
					<< DBMagicWeapon::IS_ADORN << temp.mw_is_adorn_
					<< DBMagicWeapon::ACTIVED_STATE << temp.mw_is_activate_
					<< DBMagicWeapon::MAGICW_RANK_GRADE << temp.mw_rank_.rank_star_grade_
					<< DBMagicWeapon::MAGICW_RANK_PROGRESS << temp.mw_rank_.rank_curr_star_progress_
					<< DBMagicWeapon::MAGICW_QUALITY_GRADE << temp.mw_quality_.qua_star_grade_
					<< DBMagicWeapon::MAGICW_QUALITY_PROGRESS << temp.mw_quality_.qua_curr_star_progress_));
	}

	BSONObjBuilder builder;
    builder << DBMagicWeapon::MAGICW_LIST << magicw_vc;
    builder << DBMagicWeapon::OPEN << player->get_rama_open();

	mongo_data->push_update(DBMagicWeapon::COLLECTION,
			BSON(DBMagicWeapon::ROLEID << player->role_id()),builder.obj(), true);
	return 0;
}

void MMOMagicWeapon::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBMagicWeapon::COLLECTION, BSON(DBMagicWeapon::ROLEID << 1), true);
END_CATCH
}
