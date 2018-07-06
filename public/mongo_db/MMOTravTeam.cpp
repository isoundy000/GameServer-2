/*
 * MMOTravTeam.cpp
 *
 *  Created on: 2017年5月16日
 *      Author: lyw
 */

#include "GameField.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "PoolMonitor.h"
#include "MMOTravTeam.h"
#include "TrvlTeamSystem.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "TrvlPeakMonitor.h"
#include "DBCommon.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOTravTeam::MMOTravTeam() {
	// TODO Auto-generated constructor stub

}

MMOTravTeam::~MMOTravTeam() {
	// TODO Auto-generated destructor stub
}

int MMOTravTeam::load_local_travel_team(TrvlTeamSystem *sys)
{
BEGIN_CATCH
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBLocalTravTeam::COLLECTION);
	while (cursor->more())
	{
	    BSONObj l_team_res = cursor->next();

	    TravelTeamInfo *travel_team = sys->travel_team_info_pool_->pop();

	    travel_team->__team_id = l_team_res[DBLocalTravTeam::ID].numberLong();
	    travel_team->__team_name = l_team_res[DBLocalTravTeam::TEAM_NAME].str();
	    travel_team->__leader_id = l_team_res[DBLocalTravTeam::LEADER_ID].numberLong();
	    travel_team->__auto_signup = l_team_res[DBLocalTravTeam::AUTO_SIGNUP].numberInt();
	    travel_team->__auto_accept = l_team_res[DBLocalTravTeam::AUTO_ACCEPT].numberInt();
	    travel_team->__need_force = l_team_res[DBLocalTravTeam::NEED_FORCE].numberInt();
	    travel_team->__is_signup = l_team_res[DBLocalTravTeam::IS_SIGNUP].numberInt();
	    travel_team->__refresh_signup_tick.sec(l_team_res[DBLocalTravTeam::REFRESH_SIGNUP_TICK].numberInt());
	    travel_team->__create_tick.sec(l_team_res[DBLocalTravTeam::CREATE_TICK].numberInt());
	    travel_team->__last_logout_tick.sec(l_team_res[DBLocalTravTeam::LAST_LOGOUT_TICK].numberInt());

	    if (sys->travel_team_map_->bind(travel_team->__team_id, travel_team) != 0)
	    {
	    	sys->travel_team_info_pool_->push(travel_team);
	        continue;
	    }

	    sys->travel_team_name_set_.insert(travel_team->__team_name);

	    BSONObjIterator teamer_iter(l_team_res.getObjectField(DBLocalTravTeam::TRAV_TEAMER.c_str()));
	    while (teamer_iter.more())
	    {
	        BSONObj teamer_obj = teamer_iter.next().embeddedObject();

	        Int64 teamer_id = teamer_obj[DBLocalTravTeam::TravTeamer::TEAMER_ID].numberLong();
	        TravelTeamInfo::TravelTeamer &teamer_info = travel_team->__teamer_map[teamer_id];

	        MMOTravTeam::bson_to_trvl_teamer(teamer_info, teamer_obj);

	        sys->role_travel_team_map_->rebind(teamer_id, travel_team);
	    }

	    BSONObjIterator apply_iter(l_team_res.getObjectField(DBLocalTravTeam::APPLY_MAP.c_str()));
	    while (apply_iter.more())
	    {
	    	BSONObj apply_obj = apply_iter.next().embeddedObject();

	    	Int64 teamer_id = apply_obj[DBLocalTravTeam::TravTeamer::TEAMER_ID].numberLong();
	    	TravelTeamInfo::TravelTeamer &apply_info = travel_team->__apply_map[teamer_id];

	    	MMOTravTeam::bson_to_trvl_teamer(apply_info, apply_obj);
	    }
	}
	return 0;
END_CACHE_CATCH
	return -1;
}

int MMOTravTeam::update_local_travel_team_data(TravelTeamInfo *travel_team, MongoDataMap *data_map)
{
	BSONVec teamer_vec, apply_vec;
	for (TravelTeamInfo::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.begin();
			teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
	{
		MMOTravTeam::trvl_teamer_to_bson(teamer_iter->second, teamer_vec);
	}

	for (TravelTeamInfo::TeamerMap::iterator apply_iter = travel_team->__apply_map.begin();
			apply_iter != travel_team->__apply_map.end(); ++apply_iter)
	{
		MMOTravTeam::trvl_teamer_to_bson(apply_iter->second, apply_vec);
	}

	BSONObjBuilder builder;
	builder << DBLocalTravTeam::ID << travel_team->__team_id
			<< DBLocalTravTeam::TEAM_NAME << travel_team->__team_name
			<< DBLocalTravTeam::LEADER_ID << travel_team->__leader_id
			<< DBLocalTravTeam::AUTO_SIGNUP << travel_team->__auto_signup
			<< DBLocalTravTeam::AUTO_ACCEPT << travel_team->__auto_accept
			<< DBLocalTravTeam::NEED_FORCE << travel_team->__need_force
			<< DBLocalTravTeam::IS_SIGNUP << travel_team->__is_signup
			<< DBLocalTravTeam::REFRESH_SIGNUP_TICK << int(travel_team->__refresh_signup_tick.sec())
			<< DBLocalTravTeam::CREATE_TICK << int(travel_team->__create_tick.sec())
			<< DBLocalTravTeam::LAST_LOGOUT_TICK << int(travel_team->__last_logout_tick.sec())
			<< DBLocalTravTeam::TRAV_TEAMER << teamer_vec
			<< DBLocalTravTeam::APPLY_MAP << apply_vec;

	data_map->push_update(DBLocalTravTeam::COLLECTION, BSON(DBLocalTravTeam::ID << travel_team->__team_id), builder.obj(), true);

	return 0;
}

int MMOTravTeam::remove_local_travel_team(const Int64 team_id)
{
	 MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	 data_map->push_remove(DBLocalTravTeam::COLLECTION, BSON(DBLocalTravTeam::ID << team_id));

	 TRANSACTION_MONITOR->request_mongo_transaction(team_id, TRANS_DEL_LOCAL_TRAV_TEAM, data_map);

	 return 0;
}

void MMOTravTeam::bson_to_trvl_teamer(TravelTeamInfo::TravelTeamer &teamer_info, BSONObj &teamer_obj)
{
	teamer_info.__teamer_id = teamer_obj[DBLocalTravTeam::TravTeamer::TEAMER_ID].numberLong();
	teamer_info.__teamer_name = teamer_obj[DBLocalTravTeam::TravTeamer::TEAMER_NAME].str();
	teamer_info.__teamer_level = teamer_obj[DBLocalTravTeam::TravTeamer::TEAMER_LEVEl].numberInt();
	teamer_info.__teamer_sex = teamer_obj[DBLocalTravTeam::TravTeamer::TEAMER_SEX].numberInt();
	teamer_info.__teamer_career = teamer_obj[DBLocalTravTeam::TravTeamer::TEAMER_CAREER].numberInt();
	teamer_info.__teamer_force = teamer_obj[DBLocalTravTeam::TravTeamer::TEAMER_FORCE].numberInt();
	teamer_info.__logout_tick.sec(teamer_obj[DBLocalTravTeam::TravTeamer::LOGOUT_TICK].numberInt());
	teamer_info.__join_tick.sec(teamer_obj[DBLocalTravTeam::TravTeamer::JOIN_TICK].numberInt());
}

void MMOTravTeam::trvl_teamer_to_bson(TravelTeamInfo::TravelTeamer &teamer_info, BSONVec &teamer_vec)
{
	teamer_vec.push_back(BSON(DBLocalTravTeam::TravTeamer::TEAMER_ID << teamer_info.__teamer_id
					<< DBLocalTravTeam::TravTeamer::TEAMER_NAME << teamer_info.__teamer_name
					<< DBLocalTravTeam::TravTeamer::TEAMER_LEVEl << teamer_info.__teamer_level
					<< DBLocalTravTeam::TravTeamer::TEAMER_CAREER << teamer_info.__teamer_career
					<< DBLocalTravTeam::TravTeamer::TEAMER_SEX << teamer_info.__teamer_sex
					<< DBLocalTravTeam::TravTeamer::TEAMER_FORCE << teamer_info.__teamer_force
					<< DBLocalTravTeam::TravTeamer::LOGOUT_TICK << int(teamer_info.__logout_tick.sec())
					<< DBLocalTravTeam::TravTeamer::JOIN_TICK << int(teamer_info.__join_tick.sec())));
}

int MMOTravTeam::load_remote_travel_team(TrvlPeakMonitor *monitor)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBRemoteTravTeam::COLLECTION);
    while (cursor->more())
    {
    	BSONObj r_team_res = cursor->next();

    	TravPeakTeam *trav_peak_team = monitor->trav_peak_team_pool_->pop();
    	trav_peak_team->__team_id = r_team_res[DBRemoteTravTeam::ID].numberLong();
    	trav_peak_team->__team_name = r_team_res[DBRemoteTravTeam::TEAM_NAME].str();
    	trav_peak_team->__leader_id = r_team_res[DBRemoteTravTeam::LEADER_ID].numberLong();
    	trav_peak_team->__quality_times = r_team_res[DBRemoteTravTeam::QUALITY_TIMES].numberInt();
    	trav_peak_team->__score = r_team_res[DBRemoteTravTeam::SCORE].numberInt();
		trav_peak_team->__continue_win = r_team_res[DBRemoteTravTeam::CONTINUE_WIN].numberInt();
		trav_peak_team->__update_tick = r_team_res[DBRemoteTravTeam::UPDATE_TICK].numberLong();

    	DBCommon::bson_to_base_server(*trav_peak_team, r_team_res.getObjectField(DBRemoteTravTeam::SERVER.c_str()));

    	if (monitor->trav_peak_team_map_->bind(trav_peak_team->__team_id, trav_peak_team) != 0)
    	{
    	    monitor->trav_peak_team_pool_->push(trav_peak_team);
    	    continue;
    	}

    	BSONObjIterator teamer_iter(r_team_res.getObjectField(DBRemoteTravTeam::TRAV_TEAMER.c_str()));
		while (teamer_iter.more())
		{
			BSONObj teamer_obj = teamer_iter.next().embeddedObject();

			Int64 teamer_id = teamer_obj[DBRemoteTravTeam::TravTeamer::TEAMER_ID].numberLong();

			TravPeakTeam::TravPeakTeamer &teamer_info = trav_peak_team->__teamer_map[teamer_id];
			teamer_info.__teamer_id = teamer_id;
			teamer_info.__teamer_name = teamer_obj[DBRemoteTravTeam::TravTeamer::TEAMER_NAME].str();
			teamer_info.__teamer_sex = teamer_obj[DBRemoteTravTeam::TravTeamer::TEAMER_SEX].numberInt();
			teamer_info.__teamer_career = teamer_obj[DBRemoteTravTeam::TravTeamer::TEAMER_CAREER].numberInt();
			teamer_info.__teamer_level = teamer_obj[DBRemoteTravTeam::TravTeamer::TEAMER_LEVEL].numberInt();
			teamer_info.__teamer_force = teamer_obj[DBRemoteTravTeam::TravTeamer::TEAMER_FORCE].numberInt();
			monitor->trav_role_team_map_->rebind(teamer_id, trav_peak_team->__team_id);
		}
    }

	return 0;
}

int MMOTravTeam::update_remote_travel_team_data(TravPeakTeam *trav_peak_team, MongoDataMap *data_map)
{
	BSONVec teamer_vc;
	for (TravPeakTeam::TeamerMap::iterator teamer_iter = trav_peak_team->__teamer_map.begin();
	        teamer_iter != trav_peak_team->__teamer_map.end(); ++teamer_iter)
	{
		TravPeakTeam::TravPeakTeamer &teamer_info = teamer_iter->second;
		teamer_vc.push_back(BSON(DBRemoteTravTeam::TravTeamer::TEAMER_ID << teamer_info.__teamer_id
					<< DBRemoteTravTeam::TravTeamer::TEAMER_NAME << teamer_info.__teamer_name
					<< DBRemoteTravTeam::TravTeamer::TEAMER_SEX << teamer_info.__teamer_sex
					<< DBRemoteTravTeam::TravTeamer::TEAMER_CAREER << teamer_info.__teamer_career
					<< DBRemoteTravTeam::TravTeamer::TEAMER_LEVEL << teamer_info.__teamer_level
					<< DBRemoteTravTeam::TravTeamer::TEAMER_FORCE << teamer_info.__teamer_force));
	}

	BSONObjBuilder builder;
	builder << DBRemoteTravTeam::ID << trav_peak_team->__team_id
			<< DBRemoteTravTeam::TEAM_NAME << trav_peak_team->__team_name
			<< DBRemoteTravTeam::LEADER_ID << trav_peak_team->__leader_id
			<< DBRemoteTravTeam::QUALITY_TIMES << trav_peak_team->__quality_times
			<< DBRemoteTravTeam::SCORE << trav_peak_team->__score
			<< DBRemoteTravTeam::CONTINUE_WIN << trav_peak_team->__continue_win
			<< DBRemoteTravTeam::UPDATE_TICK << trav_peak_team->__update_tick
			<< DBRemoteTravTeam::SERVER << DBCommon::base_server_to_bson(*trav_peak_team)
			<< DBRemoteTravTeam::TRAV_TEAMER << teamer_vc;

	data_map->push_update(DBRemoteTravTeam::COLLECTION,
	        BSON(DBRemoteTravTeam::ID << trav_peak_team->__team_id),
	        builder.obj(), true);

	return 0;
}

int MMOTravTeam::remove_remote_travel_team(const Int64 team_id)
{
	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	data_map->push_remove(DBRemoteTravTeam::COLLECTION, BSON(DBRemoteTravTeam::ID << team_id));

	TRANSACTION_MONITOR->request_mongo_transaction(team_id, TRANS_DEL_REMOTE_TRAV_TEAM, data_map);

	return 0;
}

int MMOTravTeam::load_travel_peak_quality_info(TrvlPeakMonitor *monitor)
{
	TravPeakQualityInfo &quality_info = monitor->quality_info();

	BSONObj res = CACHED_CONNECTION.findOne(DBQualityInfo::COLLECTION, QUERY(DBQualityInfo::ID << 0));
	JUDGE_RETURN(res.isEmpty() == false, 0);

	BSONObjIterator signup_iter(res.getObjectField(DBQualityInfo::SIGNUP_SET.c_str()));
	while (signup_iter.more())
	{
		Int64 team_id = signup_iter.next().numberLong();
		quality_info.__signup_team_set.insert(team_id);
	}

	return 0;
}

int MMOTravTeam::update_travel_peak_quality_info(TrvlPeakMonitor *monitor)
{
	TravPeakQualityInfo &quality_info = monitor->quality_info();

	LongVec signup_vec;
	for (LongSet::iterator iter = quality_info.__signup_team_set.begin();
			iter != quality_info.__signup_team_set.end(); ++iter)
	{
		signup_vec.push_back(*iter);
	}

	BSONObjBuilder builder;
	builder << DBQualityInfo::SIGNUP_SET << signup_vec;

	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	data_map->push_update(DBQualityInfo::COLLECTION, BSON(DBQualityInfo::ID << 0), builder.obj(), true);
	TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_TRAVEL_PEAK_KNOCK, data_map);

	return 0;
}

void MMOTravTeam::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBLocalTravTeam::COLLECTION, BSON(DBLocalTravTeam::ID << 1), true);
	this->conection().ensureIndex(DBRemoteTravTeam::COLLECTION, BSON(DBRemoteTravTeam::ID << 1), true);
END_CATCH
}


