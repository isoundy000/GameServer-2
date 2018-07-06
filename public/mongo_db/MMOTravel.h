/*
 * MMOTravel.h
 *
 *  Created on: Nov 17, 2016
 *      Author: peizhibi
 */

#ifndef MMOTRAVEL_H_
#define MMOTRAVEL_H_

#include "MongoTable.h"

class TrvlArenaRole;
class TrvlArenaMonitor;
class TrvlWeddingMonitor;
class TrvlWeddingRank;
class TrvlRechargeMonitor;
class TrvlRechargeRank;
class WorldBossInfo;

class MMOTravel : public MongoTable
{
public:
	MMOTravel();
	virtual ~MMOTravel();

	//travel arena
    int load_player_tarena(MapPlayerEx *player);
    static int update_data(MapPlayerEx *player, MongoDataMap *data_map);

    static void clear_all_tarena_role();
	static void load_tarena_role(TrvlArenaMonitor* monitor);
	static void save_tarena_role(TrvlArenaRole* arena);

	static void load_wedding_rank(TrvlWeddingMonitor* monitor);
	static void save_wedding_rank(TrvlWeddingRank* rank_info);

	static void remove_recharge_rank();

	static void save_recharge_rank(TrvlRechargeMonitor* monitor);
	static void load_recharge_rank(TrvlRechargeMonitor* monitor);
	static void add_recharge_rank_info(TrvlRechargeMonitor* monitor, BSONObj &obj, int type);

    static void request_correct_rank_info(void);
    static void correct_rank_info_from_db(TrvlRechargeMonitor *monitor, MongoDataMap *data_map);

    static int load_wboss_info(WorldBossInfo* wboss_info);
    static int update_wboss_info(WorldBossInfo* wboss_info, int direct_save);

protected:
    virtual void ensure_all_index(void);
};

#endif /* MMOTRAVEL_H_ */
