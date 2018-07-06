/*
 * File Name: MMOTrvlBattle.h
 * 
 * Created on: 2017-05-05 20:46:46
 * Author: glendy
 * 
 * Last Modified: 2017-05-06 12:35:01
 * Description: 
 */

#ifndef _MMOTRVLBATTLE_H_
#define _MMOTRVLBATTLE_H_

#include "MongoTable.h"

class TrvlBattleMonitor;
class TrvlBattleRanker;
class TrvlBattleRole;

class MMOTrvlBattle : public MongoTable
{
public:
    virtual ~MMOTrvlBattle(void);

    static int load_trvl_battle_info(TrvlBattleMonitor *monitor);
    static int save_trvl_battle_info(TrvlBattleMonitor *monitor);

    static BSONObj base_info_to_bson(TrvlBattleMonitor *monitor);
    static BSONObj tb_ranker_to_bson(TrvlBattleRanker *tb_ranker);
    static BSONObj tb_role_to_bson(TrvlBattleRole *tb_role);
    static BSONObj cur_rank_to_bson(TrvlBattleMonitor *monitor);
    static BSONObj cur_role_list_to_bson(TrvlBattleMonitor *monitor);
    static BSONObj last_rank_list_to_bson(TrvlBattleMonitor *monitor);
    static BSONObj history_fame_list_to_bson(TrvlBattleMonitor *monitor);

    static void bson_to_base_info(TrvlBattleMonitor *monitor, BSONObj &obj);
    static void bson_to_tb_ranker(TrvlBattleRanker *tb_ranker, BSONObj &obj);
    static void bson_to_tb_role(TrvlBattleRole *tb_role, BSONObj &obj);
    static void bson_to_cur_rank_list(TrvlBattleMonitor *monitor, BSONObj &obj);
    static void bson_to_cur_role_list(TrvlBattleMonitor *monitor, BSONObj &obj);
    static void bson_to_last_rank_list(TrvlBattleMonitor *monitor, BSONObj &obj);
    static void bson_to_history_fame_list(TrvlBattleMonitor *monitor, BSONObj &obj);

protected:
    virtual void ensure_all_index(void);
};

#endif //MMOTRVLBATTLE_H_
