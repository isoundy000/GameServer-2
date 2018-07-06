/*
 * File Name: MMOTrvlBattle.cpp
 * 
 * Created on: 2017-05-05 22:52:55
 * Author: glendy
 * 
 * Last Modified: 2017-05-15 16:05:34
 * Description: 
 */

#include "MMOTrvlBattle.h"
#include "TrvlBattleMonitor.h"
#include "GameField.h"
#include "DBCommon.h"
#include "GameCommon.h"
#include "MongoConnector.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOTrvlBattle::~MMOTrvlBattle(void)
{ /*NULL*/ }

int MMOTrvlBattle::load_trvl_battle_info(TrvlBattleMonitor *monitor)
{
BEGIN_CATCH
    BSONObj base_obj = CACHED_CONNECTION.findOne(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 0));
    MMOTrvlBattle::bson_to_base_info(monitor, base_obj);

    BSONObj cur_rank_obj = CACHED_CONNECTION.findOne(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 1));
    MMOTrvlBattle::bson_to_cur_rank_list(monitor, cur_rank_obj);

    BSONObj cur_role_obj = CACHED_CONNECTION.findOne(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 2));
    MMOTrvlBattle::bson_to_cur_role_list(monitor, cur_role_obj);

    BSONObj last_rank_obj = CACHED_CONNECTION.findOne(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 3));
    MMOTrvlBattle::bson_to_last_rank_list(monitor, last_rank_obj);

    BSONObj fame_obj = CACHED_CONNECTION.findOne(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 4));
    MMOTrvlBattle::bson_to_history_fame_list(monitor, fame_obj);

    return 0;
END_CACHE_CATCH
    return -1;
}

int MMOTrvlBattle::save_trvl_battle_info(TrvlBattleMonitor *monitor)
{
BEGIN_CATCH
    CACHED_CONNECTION.update(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 0),
            BSON("$set" << MMOTrvlBattle::base_info_to_bson(monitor)), true);

    CACHED_CONNECTION.update(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 1),
            BSON("$set" << MMOTrvlBattle::cur_rank_to_bson(monitor)), true);

    CACHED_CONNECTION.update(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 2),
            BSON("$set" << MMOTrvlBattle::cur_role_list_to_bson(monitor)), true);

    CACHED_CONNECTION.update(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 3),
            BSON("$set" << MMOTrvlBattle::last_rank_list_to_bson(monitor)), true);

    CACHED_CONNECTION.update(DBTrvlBattle::COLLECTION, QUERY(DBTrvlBattle::ID << 4),
            BSON("$set" << MMOTrvlBattle::history_fame_list_to_bson(monitor)), true);

    return 0;
END_CACHE_CATCH
    return -1;

}

BSONObj MMOTrvlBattle::base_info_to_bson(TrvlBattleMonitor *monitor)
{
    BSONObjBuilder builder;
    builder << DBTrvlBattle::FIRST_TOP_ID << monitor->first_top_id()
        << DBTrvlBattle::FIRST_TOP_NAME << monitor->first_top_name();

    return builder.obj();
}

BSONObj MMOTrvlBattle::tb_ranker_to_bson(TrvlBattleRanker *tb_ranker)
{
    BSONObjBuilder builder;
    builder << DBTrvlBattle::BattleRank::RANK << tb_ranker->__rank
        << DBTrvlBattle::BattleRank::SCORE << tb_ranker->__score
        << DBTrvlBattle::BattleRank::TOTAL_KILL_AMOUNT << tb_ranker->__total_kill_amount
        << DBTrvlBattle::BattleRank::TICK << tb_ranker->__tick
        << DBTrvlBattle::BattleRank::ROLE_ID << tb_ranker->__role_id
        << DBTrvlBattle::BattleRank::ROLE_NAME << tb_ranker->__role_name
        << DBTrvlBattle::BattleRank::PREV << tb_ranker->__prev
        << DBTrvlBattle::BattleRank::SERVER_FLAG << tb_ranker->__server_flag
        << DBTrvlBattle::BattleRank::SEX << tb_ranker->__sex
        << DBTrvlBattle::BattleRank::CAREER << tb_ranker->__career
        << DBTrvlBattle::BattleRank::LEVEL << tb_ranker->__level
        << DBTrvlBattle::BattleRank::FORCE << tb_ranker->__force
        << DBTrvlBattle::BattleRank::WEAPON << tb_ranker->__weapon
        << DBTrvlBattle::BattleRank::CLOTHES << tb_ranker->__clothes
        << DBTrvlBattle::BattleRank::WING_LEVEL << tb_ranker->__wing_level
        << DBTrvlBattle::BattleRank::SOLIDER_LEVEL << tb_ranker->__solider_level
        << DBTrvlBattle::BattleRank::VIP_TYPE << tb_ranker->__vip_type
        << DBTrvlBattle::BattleRank::MOUNT_SORT << tb_ranker->__mount_sort
        << DBTrvlBattle::BattleRank::SWORD_POOL << tb_ranker->__sword_pool
        << DBTrvlBattle::BattleRank::TIAN_GANG << tb_ranker->__tian_gang
        << DBTrvlBattle::BattleRank::FASHION_ID << tb_ranker->__fashion_id
        << DBTrvlBattle::BattleRank::FASHION_COLOR << tb_ranker->__fashion_color;

    return builder.obj();
}

BSONObj MMOTrvlBattle::tb_role_to_bson(TrvlBattleRole *tb_role)
{
    BSONObjBuilder builder;
    builder << DBTrvlBattle::BattleRole::ROLE_ID << tb_role->__role_id
        << DBTrvlBattle::BattleRole::LEAGUE_ID << tb_role->__league_id
        << DBTrvlBattle::BattleRole::ROLE_NAME << tb_role->__role_name
        << DBTrvlBattle::BattleRole::MAX_FLOOR << tb_role->__max_floor
        << DBTrvlBattle::BattleRole::LAST_REWARD_SCORE << tb_role->__last_reward_score
        << DBTrvlBattle::BattleRole::NEXT_REWARD_SCORE << tb_role->__next_reward_score
        << DBTrvlBattle::BattleRole::NEXT_SCORE_REWARD_ID << tb_role->__next_score_reward_id
        << DBTrvlBattle::BattleRole::SERVER << DBCommon::base_server_to_bson(*tb_role);
    return builder.obj();
}

BSONObj MMOTrvlBattle::cur_rank_to_bson(TrvlBattleMonitor *monitor)
{
    std::vector<BSONObj> cur_rank_vc;
    for (TrvlBattleMonitor::TrvlBattleRankList::iterator iter = monitor->cur_ranker_list_->begin();
            iter != monitor->cur_ranker_list_->end(); ++iter)
    {
        TrvlBattleRanker *tb_ranker = *iter;
        cur_rank_vc.push_back(MMOTrvlBattle::tb_ranker_to_bson(tb_ranker));
    }
    BSONObjBuilder builder;
    builder << DBTrvlBattle::CUR_RANK_LIST << cur_rank_vc;
    return builder.obj();
}

BSONObj MMOTrvlBattle::cur_role_list_to_bson(TrvlBattleMonitor *monitor)
{
    std::vector<BSONObj> cur_role_vc;
    for (TrvlBattleMonitor::TrvlBattleRoleMap::iterator iter = monitor->battle_role_map_->begin();
            iter != monitor->battle_role_map_->end(); ++iter)
    {
        TrvlBattleRole *tb_role = iter->second;
        cur_role_vc.push_back(MMOTrvlBattle::tb_role_to_bson(tb_role));
    }
    BSONObjBuilder builder;
    builder << DBTrvlBattle::CUR_ROLE_LIST << cur_role_vc;
    return builder.obj();
}

BSONObj MMOTrvlBattle::last_rank_list_to_bson(TrvlBattleMonitor *monitor)
{
    std::vector<BSONObj> last_rank_vc;
    for (TrvlBattleMonitor::TrvlBattleRankList::iterator iter = monitor->last_ranker_list_->begin();
            iter != monitor->last_ranker_list_->end(); ++iter)
    {
        TrvlBattleRanker *tb_ranker = *iter;
        last_rank_vc.push_back(MMOTrvlBattle::tb_ranker_to_bson(tb_ranker));
    }
    BSONObjBuilder builder;
    builder << DBTrvlBattle::LAST_RANK_LIST << last_rank_vc;
    return builder.obj();
}

BSONObj MMOTrvlBattle::history_fame_list_to_bson(TrvlBattleMonitor *monitor)
{
    std::vector<BSONObj> history_vc;
    for (TrvlBattleMonitor::TrvlBattleRankList::iterator iter = monitor->history_hall_of_fame_list_->begin();
            iter != monitor->history_hall_of_fame_list_->end(); ++iter)
    {
        TrvlBattleRanker *tb_ranker = *iter;
        history_vc.push_back(MMOTrvlBattle::tb_ranker_to_bson(tb_ranker));
    }
    BSONObjBuilder builder;
    builder << DBTrvlBattle::HISTORY_FAME_LIST << history_vc;
    return builder.obj();
}

void MMOTrvlBattle::bson_to_base_info(TrvlBattleMonitor *monitor, BSONObj &obj)
{
    JUDGE_RETURN(obj.isEmpty() == false, ;);

    monitor->first_top_id_ = obj[DBTrvlBattle::FIRST_TOP_ID].numberLong();
    monitor->first_top_name_ = obj[DBTrvlBattle::FIRST_TOP_NAME].str();
}

void MMOTrvlBattle::bson_to_tb_ranker(TrvlBattleRanker *tb_ranker, BSONObj &obj)
{
    JUDGE_RETURN(obj.isEmpty() == false, ;);

    tb_ranker->__rank = obj[DBTrvlBattle::BattleRank::RANK].numberInt();
    tb_ranker->__score = obj[DBTrvlBattle::BattleRank::SCORE].numberInt();
    tb_ranker->__total_kill_amount = obj[DBTrvlBattle::BattleRank::TOTAL_KILL_AMOUNT].numberInt();
    tb_ranker->__tick = obj[DBTrvlBattle::BattleRank::TICK].numberLong();
    tb_ranker->__role_id = obj[DBTrvlBattle::BattleRank::ROLE_ID].numberLong();
    tb_ranker->__role_name = obj[DBTrvlBattle::BattleRank::ROLE_NAME].str();
    tb_ranker->__prev = obj[DBTrvlBattle::BattleRank::PREV].str();
    tb_ranker->__server_flag = obj[DBTrvlBattle::BattleRank::SERVER_FLAG].str();
    tb_ranker->__sex = obj[DBTrvlBattle::BattleRank::SEX].numberInt();
    tb_ranker->__career = obj[DBTrvlBattle::BattleRank::CAREER].numberInt();
    tb_ranker->__level = obj[DBTrvlBattle::BattleRank::LEVEL].numberInt();
    tb_ranker->__force = obj[DBTrvlBattle::BattleRank::FORCE].numberInt();
    tb_ranker->__weapon = obj[DBTrvlBattle::BattleRank::WEAPON].numberInt();
    tb_ranker->__clothes = obj[DBTrvlBattle::BattleRank::CLOTHES].numberInt();
    tb_ranker->__wing_level = obj[DBTrvlBattle::BattleRank::WING_LEVEL].numberInt();
    tb_ranker->__solider_level = obj[DBTrvlBattle::BattleRank::SOLIDER_LEVEL].numberInt();
    tb_ranker->__vip_type = obj[DBTrvlBattle::BattleRank::VIP_TYPE].numberInt();
    tb_ranker->__mount_sort = obj[DBTrvlBattle::BattleRank::MOUNT_SORT].numberInt();
    tb_ranker->__sword_pool = obj[DBTrvlBattle::BattleRank::SWORD_POOL].numberInt();
    tb_ranker->__tian_gang = obj[DBTrvlBattle::BattleRank::TIAN_GANG].numberInt();
    tb_ranker->__fashion_id = obj[DBTrvlBattle::BattleRank::FASHION_ID].numberInt();
    tb_ranker->__fashion_color = obj[DBTrvlBattle::BattleRank::FASHION_COLOR].numberInt();
}

void MMOTrvlBattle::bson_to_tb_role(TrvlBattleRole *tb_role, BSONObj &obj)
{
    JUDGE_RETURN(obj.isEmpty() == false, ;);

    tb_role->__role_id = obj[DBTrvlBattle::BattleRole::ROLE_ID].numberLong();
    tb_role->__league_id = obj[DBTrvlBattle::BattleRole::LEAGUE_ID].numberLong();
    tb_role->__role_name = obj[DBTrvlBattle::BattleRole::ROLE_NAME].str();
    tb_role->__max_floor = obj[DBTrvlBattle::BattleRole::MAX_FLOOR].numberInt();
    tb_role->__last_reward_score = obj[DBTrvlBattle::BattleRole::LAST_REWARD_SCORE].numberInt();
    tb_role->__next_reward_score = obj[DBTrvlBattle::BattleRole::NEXT_REWARD_SCORE].numberInt();
    tb_role->__next_score_reward_id = obj[DBTrvlBattle::BattleRole::NEXT_SCORE_REWARD_ID].numberInt();
    DBCommon::bson_to_base_server(*tb_role, obj.getObjectField(DBTrvlBattle::BattleRole::SERVER.c_str()));
}

void MMOTrvlBattle::bson_to_cur_rank_list(TrvlBattleMonitor *monitor, BSONObj &obj)
{
    JUDGE_RETURN(obj.isEmpty() == false, ;);

    BSONObjIterator cur_rank_iter(obj.getObjectField(DBTrvlBattle::CUR_RANK_LIST.c_str()));
    while (cur_rank_iter.more())
    {
        BSONObj rank_obj = cur_rank_iter.next().embeddedObject();
        TrvlBattleRanker *tb_ranker = monitor->trvl_battle_ranker_pool_->pop();
        MMOTrvlBattle::bson_to_tb_ranker(tb_ranker, rank_obj);
        tb_ranker->__rank_score = tb_ranker->__score;

        monitor->cur_ranker_map_->rebind(tb_ranker->__role_id, tb_ranker);
        monitor->cur_ranker_list_->push_back(tb_ranker);
    }
}

void MMOTrvlBattle::bson_to_cur_role_list(TrvlBattleMonitor *monitor, BSONObj &obj)
{
    JUDGE_RETURN(obj.isEmpty() == false, ;);

    BSONObjIterator cur_role_iter(obj.getObjectField(DBTrvlBattle::CUR_ROLE_LIST.c_str()));
    while (cur_role_iter.more())
    {
        BSONObj role_obj = cur_role_iter.next().embeddedObject();
        TrvlBattleRole *tb_role = monitor->trvl_battle_role_pool_->pop();
        MMOTrvlBattle::bson_to_tb_role(tb_role, role_obj);

        monitor->battle_role_map_->rebind(tb_role->__role_id, tb_role);
    }
}

void MMOTrvlBattle::bson_to_last_rank_list(TrvlBattleMonitor *monitor, BSONObj &obj)
{
    JUDGE_RETURN(obj.isEmpty() == false, ;);

    BSONObjIterator last_rank_iter(obj.getObjectField(DBTrvlBattle::LAST_RANK_LIST.c_str()));
    while (last_rank_iter.more())
    {
        BSONObj rank_obj = last_rank_iter.next().embeddedObject();
        TrvlBattleRanker *tb_ranker = monitor->trvl_battle_ranker_pool_->pop();
        MMOTrvlBattle::bson_to_tb_ranker(tb_ranker, rank_obj);
        tb_ranker->__rank_score = tb_ranker->__score;

        monitor->last_ranker_map_->rebind(tb_ranker->__role_id, tb_ranker);
        monitor->last_ranker_list_->push_back(tb_ranker);
    }
}

void MMOTrvlBattle::bson_to_history_fame_list(TrvlBattleMonitor *monitor, BSONObj &obj)
{
    JUDGE_RETURN(obj.isEmpty() == false, ;);

    BSONObjIterator fame_iter(obj.getObjectField(DBTrvlBattle::HISTORY_FAME_LIST.c_str()));
    while (fame_iter.more())
    {
        BSONObj rank_obj = fame_iter.next().embeddedObject();
        TrvlBattleRanker *tb_ranker = monitor->trvl_battle_ranker_pool_->pop();
        MMOTrvlBattle::bson_to_tb_ranker(tb_ranker, rank_obj);
        tb_ranker->__rank_score = tb_ranker->__score;

        monitor->history_hall_of_fame_list_->push_back(tb_ranker);
    }
}

void MMOTrvlBattle::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBTrvlBattle::COLLECTION, BSON(DBTrvlBattle::ID << 1), true);
END_CATCH
}

