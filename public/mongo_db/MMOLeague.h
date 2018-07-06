/*
 * MMOLeague.h
 *
 *  Created on: Aug 14, 2013
 *      Author: peizhibi
 */

#ifndef MMOLEAGUE_H_
#define MMOLEAGUE_H_

#include "MongoTable.h"
#include "GameHeader.h"

class League;
class ArenaRole;
class AreaSysDetail;
class LeagueLogItem;
class LeagueWarInfo;
class LeagueRegionResult;

class MMOLeague : public MongoTable
{
public:
	MMOLeague();
	virtual ~MMOLeague();

	void load_leaguer_info(LogicPlayer* player);
	void load_leaguer_skill_prop(MapPlayerEx* player);
	void load_league_flag_prop(MapPlayerEx* player);

	int load_lwar_info(DBShopMode* shop_mode);

public:
	static int save_league_index(int league_index);

	static void load_all_league();

	static void save_league(League* league, int direct_save = false);
	static void remove_league(Int64 league_index);

	static void save_leaguer_info(LogicPlayer* player, MongoDataMap *mongo_data);
	static void save_leaguer_info(MapPlayerEx* player, MongoDataMap *mongo_data);
	static void save_quit_info(Int64 role_index, int leave_type,
			Int64 leave_tick = ::time(NULL));

	static std::string fetch_league_name(const Int64 league_id);
	static int fetch_league_pos(const Int64 league_id, Int64 self_id);
	static Int64 fetch_leader_id(const Int64 league_id);

	static void load_leaguer_fb_flag(BLongSet&);
	static void save_leaguer_fb_flag(BLongSet&, int flag = 1);

public:
	static void save_league_war(LeagueWarInfo& war_info);
	static void load_league_war(LeagueWarInfo& war_info);
	static void load_league_war(LeagueWarInfo& war_info, BSONObj* p_res);

	static void save_arena(AreaSysDetail* arena_detail, int direct_save = true);
	static void load_arena(AreaSysDetail* arena_detail);
	static void load_arena_guide(AreaSysDetail* arena_detail);
	static void sort_arena(AreaSysDetail* arena_detail);
	static void save_arena_role(const ArenaRole& arena_role, int direct);

	static int loadLeagueRegionFightInfo(LeagueRegionResult& lrf_info);
	static void updateLeagueRegionFightInfo(LeagueRegionResult& lrf_info);


protected:
	virtual void ensure_all_index();

private:
	static void unserial_league_log(LeagueLogItem& log_item, BSONObj& bson);
	static void serial_league_log(BSONVec& bson_set, std::list<LeagueLogItem>& league_log);
};

#endif /* MMOLEAGUE_H_ */
