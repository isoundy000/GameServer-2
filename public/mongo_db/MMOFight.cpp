/*
 * MMOFight.cpp
 *
 * Created on: 2013-04-28 15:19
 *     Author: lyz
 */

#include "GameField.h"
#include "MMOFight.h"
#include "MongoConnector.h"
#include "MapPlayerEx.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include "MapLogicPlayer.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOFight::~MMOFight(void)
{ /*NULL*/ }

int MMOFight::load_player_fight(MapPlayerEx *player)
{
    FightDetail &fight_detail = player->fight_detail();

    BSONObj res = this->conection().findOne(Fight::COLLECTION, QUERY(Fight::ID << player->role_id()));
    if (res.isEmpty() == true)
    {
        fight_detail.set_level(1);
        fight_detail.__experience = 0;
        fight_detail.__pk_state = PK_PEACE;
        player->set_new_role_flag(true);
    }
    else
    {
		fight_detail.__pk_state = res[Fight::PK].numberInt();
		fight_detail.__pk_tick = Time_Value::gettime(res[Fight::PK_TICK].numberDouble());
		fight_detail.__camp_id = res[Fight::CAMP_ID].numberInt();
		fight_detail.__experience = res[Fight::EXPERIENCE].numberLong();
		fight_detail.__blood = res[Fight::BLOOD].numberInt();
		fight_detail.__magic = res[Fight::MAGIC].numberInt();
		fight_detail.__angry = res[Fight::ANGRY].numberInt();
		fight_detail.__glamour = res[Fight::GLAMOUR].numberInt();
		fight_detail.__jump = res[Fight::JUMP].numberInt();

		fight_detail.set_level(res[Fight::LEVEL].numberInt());
		player->role_detail().__level = fight_detail.__level;
    }


    return 0;
}

int MMOFight::update_data(MapPlayerEx *player, MongoDataMap *mongo_data)
{
    FightDetail &fight_detail = player->fight_detail();

//    Time_Value pk_tick = Time_Value::gettimeofday();
//    double d_pk_tick = 0.0;
//    if (fight_detail.__pk_tick > pk_tick)
//    {
//        pk_tick = fight_detail.__pk_tick - pk_tick;
//        d_pk_tick = pk_tick.sec() + pk_tick.usec() / 1000000.0;
//    }

    BSONObjBuilder builder;
    builder << Fight::PK << fight_detail.__pk_state
        << Fight::PK_TICK << 0
        << Fight::CAMP_ID << fight_detail.__camp_id
        << Fight::LEVEL << fight_detail.__level
        << Fight::ANGRY << fight_detail.__angry
        << Fight::GLAMOUR << fight_detail.__glamour
        << Fight::JUMP << fight_detail.__jump
        << Fight::EXPERIENCE << fight_detail.__experience
        << Fight::BLOOD_BASIC << fight_detail.__blood_max.basic()
        << Fight::ATTACK_LOWER_BASIC << fight_detail.__attack_lower.basic()
        << Fight::ATTACK_UPPER_BASIC << fight_detail.__attack_upper.basic()
        << Fight::DEFENCE_LOWER_BASIC << fight_detail.__defence_lower.basic()
        << Fight::DEFENCE_UPPER_BASIC << fight_detail.__defence_upper.basic()
        << Fight::HIT_BASIC << fight_detail.__hit.basic()
        << Fight::AVOID_BASIC << fight_detail.__avoid.basic()
        << Fight::CRIT_BASIC << fight_detail.__crit.basic()
        << Fight::TOUGHNESS_BASIC << fight_detail.__toughness.basic()
        << Fight::LUCKY_BASIC << fight_detail.__lucky.basic()
        << Fight::MAGIC_BASIC << fight_detail.__magic_max.basic()
        << Fight::BLOOD << fight_detail.__blood
        << Fight::MAGIC << fight_detail.__magic;

    mongo_data->push_update(Fight::COLLECTION,
            BSON(Fight::ID << player->role_id()), builder.obj(), true);

    return 0;
}

int MMOFight::load_tiny_player(MapPlayerEx *player)
{
	BSONObj res = this->conection().findOne(DBMapTiny::COLLECTION,
			QUERY(DBMapTiny::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	BloodContainer& blood_container = player->blood_container();
	blood_container.cur_blood_ = res[DBMapTiny::CUR_BLOOD].numberInt();

	blood_container.non_tips_ = res[DBMapTiny::NON_TIPS].numberInt();
	blood_container.everyday_tick_ = res[DBMapTiny::EVERYDAY_TICK].numberLong();

	return 0;
}

int MMOFight::update_tiny_data(MapPlayerEx *player, MongoDataMap *mongo_data)
{
	BloodContainer& blood_container = player->blood_container();

    BSONObjBuilder builder;
    builder << DBMapTiny::CUR_BLOOD << blood_container.cur_blood_
    		<< DBMapTiny::NON_TIPS << blood_container.non_tips_
    		<< DBMapTiny::EVERYDAY_TICK << blood_container.everyday_tick_;

    mongo_data->push_update(DBMapTiny::COLLECTION, BSON(DBMapTiny::ID << player->role_id()),
    		builder.obj(), true);
    return 0;
}

int MMOFight::load_tiny_player(MapLogicPlayer *player)
{
	BSONObj res = this->conection().findOne(DBMapTiny::COLLECTION,
			QUERY(DBMapTiny::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	TinyDetail* tiny_detail = player->tiny_detail();
	GameCommon::bson_to_map(tiny_detail->guide_map_,
			res.getObjectField(DBMapTiny::CLIENT_GUIDE.c_str()));

	player->set_total_recharge(res[DBMapTiny::DAILY_TOTAL_RECHARGE].numberInt());
	player->set_last_recharge_time(res[DBMapTiny::LAST_RECHARGE_TICK].numberLong());


//    BSONObjIterator iter(res.getObjectField(DBMapTiny::FUND_SET.c_str()));
//    while (iter.more())
//    {
//    	BSONObj obj = iter.next().embeddedObject();
//    	TinyDetail::FundItem& fund_item = tiny_detail->fund_set_[index];
//
//    	fund_item.buy_flag_ = obj[DBMapTiny::FundItem::BUY_FLAG].numberInt();
//    	fund_item.draw_flag_ = obj[DBMapTiny::FundItem::DRAW_FLAG].numberInt();
//    	fund_item.draw_times_ = obj[DBMapTiny::FundItem::DRAW_TIMES].numberInt();
//    	fund_item.get_tick_ = obj[DBMapTiny::FundItem::GET_TICK].numberLong();
//    	fund_item.total_times_ = obj[DBMapTiny::FundItem::TOTAL_TIMES].numberInt();
//    	++index;
//    }

	return 0;
}

int MMOFight::update_tiny_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	TinyDetail* tiny_detail = player->tiny_detail();

	BSONVec guide_bson_vec;
	GameCommon::map_to_bson(guide_bson_vec, tiny_detail->guide_map_);

//	BSONVec fund_set;
//	fund_set.reserve(TinyDetail::TOTAL_FUND);
//
//	for (int i = 0; i < TinyDetail::TOTAL_FUND; ++i)
//	{
//		TinyDetail::FundItem& fund_item = tiny_detail->fund_set_[i];
//		fund_set.push_back(BSON(DBMapTiny::FundItem::BUY_FLAG << fund_item.buy_flag_
//				<< DBMapTiny::FundItem::DRAW_FLAG << fund_item.draw_flag_
//				<< DBMapTiny::FundItem::DRAW_TIMES << fund_item.draw_times_
//				<< DBMapTiny::FundItem::GET_TICK << fund_item.get_tick_
//				<< DBMapTiny::FundItem::TOTAL_TIMES << fund_item.total_times_));
//	}

    BSONObjBuilder builder;
    builder << DBMapTiny::CLIENT_GUIDE << guide_bson_vec
    		<< DBMapTiny::LAST_RECHARGE_TICK << player->get_last_recharge_time()
    		<< DBMapTiny::DAILY_TOTAL_RECHARGE << player->get_total_recharge();

    mongo_data->push_update(DBMapTiny::COLLECTION, BSON(DBMapTiny::ID << player->role_id()),
    		builder.obj(), true);
	return 0;
}

void MMOFight::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(Fight::COLLECTION, BSON(Fight::ID << 1), true);
	this->conection().ensureIndex(DBMapTiny::COLLECTION, BSON(DBMapTiny::ID << 1), true);
END_CATCH
}

