/*
 * MMOTravTeam.h
 *
 *  Created on: 2017年5月16日
 *      Author: lyw
 */

#ifndef MMOTRAVTEAM_H_
#define MMOTRAVTEAM_H_

#include "MongoTable.h"
#include "LogicStruct.h"

class TrvlTeamSystem;
class TrvlPeakMonitor;
class TravPeakTeam;

class MMOTravTeam : public MongoTable
{
public:
	MMOTravTeam();
	virtual ~MMOTravTeam();

	static int load_local_travel_team(TrvlTeamSystem *sys);
	static int update_local_travel_team_data(TravelTeamInfo *travel_team, MongoDataMap *data_map);
	static int remove_local_travel_team(const Int64 team_id);

	static void bson_to_trvl_teamer(TravelTeamInfo::TravelTeamer &teamer_info, BSONObj &teamer_obj);
	static void trvl_teamer_to_bson(TravelTeamInfo::TravelTeamer &teamer_info, BSONVec &teamer_vec);

	static int load_remote_travel_team(TrvlPeakMonitor *monitor);
	static int update_remote_travel_team_data(TravPeakTeam *trav_peak_team, MongoDataMap *data_map);
	static int remove_remote_travel_team(const Int64 team_id);

	static int load_travel_peak_quality_info(TrvlPeakMonitor *monitor);
	static int load_travel_peak_knockout_info(TrvlPeakMonitor *monitor);
	static int update_travel_peak_quality_info(TrvlPeakMonitor *monitor);
	static int update_travel_peak_knockout_info(TrvlPeakMonitor *monitor);

protected:
    virtual void ensure_all_index(void);
};

#endif /* MMOTRAVTEAM_H_ */
