/*
 * MMORankPannel.h
 *
 *  Created on: Mar 5, 2014
 *      Author: louis
 */

#ifndef MMORANKPANNEL_H_
#define MMORANKPANNEL_H_

#include "MongoTable.h"
#include "RankStruct.h"


class MMORankPannel : public MongoTable
{
public:
	MMORankPannel();
	virtual ~MMORankPannel();

	static void load_rank_pannel_data();
	static int check_error_role(Int64 role);

//	static int update_data(RankDetail& rank_detail, MongoDataMap *mongo_data_map, const int rank_type);
	static int update_data(RankPannel& rank_pannel, MongoDataMap *mongo_data_map, const int rank_type);
	static int update_data(RankShowPannel& rank_pannel, MongoDataMap *mongo_data_map, const int rank_type);
	//for save player source rank data
	static int update_date(LogicPlayer* player, MongoDataMap *mongo_data_map);
	static int update_date(MapLogicPlayer* player, MongoDataMap *mongo_data_map);
	static int update_date(MapPlayerEx* player, MongoDataMap *mongo_data_map);

	static int push_equip(BSONVec &equip_vec, GamePackage* package);

	static int update_date_player_script_zyfm(const RankRecord* rank_record, MongoDataMap *mongo_data_map);

	int load_rank_data(MongoDataMap* data_map, const int rank_type);

	static void save_offline_data(PlayerOfflineData* offline_data, int save_type, int direct_save = false);

	int save_beast_detail_on_pet_rank(DBShopMode* shop_mode);

	static int fix_rank_manager_next_refresh(RankRefreshManager &rank_manager);
	static int save_rank_manager_last_refresh(const RankRefreshManager &rank_manager);
	static int update_rank_last_refresh(MongoDataMap *data_map, const int rank_type, const int refresh_tick);

protected:
	virtual void ensure_all_index();
};

#endif /* MMORANKPANNEL_H_ */
