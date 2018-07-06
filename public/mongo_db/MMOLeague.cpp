/*
 * MMOLeague.cpp
 *
 *  Created on: Aug 14, 2013
 *      Author: peizhibi
 */

#include "MMORole.h"
#include "MMOSwordPool.h"
#include "MMOLeague.h"
#include "GameField.h"

#include "MapPlayerEx.h"
#include "LogicPlayer.h"
#include "LeagueSystem.h"
#include "MongoDataMap.h"
#include "LeagueReginFightSystem.h"

#include "DBCommon.h"
#include "MongoConnector.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOLeague::MMOLeague()
{
	// TODO Auto-generated constructor stub

}

MMOLeague::~MMOLeague()
{
	// TODO Auto-generated destructor stub
}

void MMOLeague::load_leaguer_info(LogicPlayer* player)
{
	BSONObj res = this->conection().findOne(DBLeaguer::COLLECTION,
			QUERY(DBLeaguer::ID << player->role_id()));

	JUDGE_RETURN(res.isEmpty() == false, ;);

	LeaguerInfo& leaguer_info = player->leaguer_info();
	GameCommon::bson_to_map(leaguer_info.buy_map_,
			res.getObjectField(DBLeaguer::SHOP_BUY.c_str()));

	GameCommon::bson_to_map(leaguer_info.skill_map_,
			res.getObjectField(DBLeaguer::LEAGUE_SKILL.c_str()));

	DBCommon::bson_to_long_map(leaguer_info.apply_list_,
			res.getObjectField(DBLeaguer::APPLY_LIST.c_str()));

	GameCommon::bson_to_map(leaguer_info.region_draw_,
			res.getObjectField(DBLeaguer::REGION_DRAW.c_str()));

	BSONObjIterator wave_iter = res.getObjectField(DBLeaguer::WAVE_REWARD_MAP.c_str());
	while (wave_iter.more())
	{
		BSONObj obj = wave_iter.next().embeddedObject();
		int obj_key = obj[DBPairObj::KEY].numberInt();
		BSONObjIterator iter2 = obj.getObjectField(DBPairObj::VALUE.c_str());
		IntMap reward_map;
		while (iter2.more())
		{
			BSONObj obj2 = iter2.next().embeddedObject();
			int key = obj2[DBPairObj::KEY].numberInt();
			int value = obj2[DBPairObj::VALUE].numberInt();
			reward_map[key] = value;
		}
		leaguer_info.wave_reward_map_[obj_key] = reward_map;
	}


	leaguer_info.day_admire_times_ = res[DBLeaguer::DAY_ADMIRE_TIMES].numberInt();
	leaguer_info.day_siege_refresh_tick_ = Time_Value(res[DBLeaguer::SIEGE_SHOP_REFRESH].numberLong());

	leaguer_info.open_ =  res[DBLeaguer::OPEN].numberInt();
	leaguer_info.gold_donate_ = res[DBLeaguer::GOLD_DONATE].numberInt();
	leaguer_info.draw_welfare_ = res[DBLeaguer::DRAW_WELFARE].numberInt();
	leaguer_info.wand_donate_ = res[DBLeaguer::WAND_DONATE].numberInt();

	leaguer_info.send_flag_ = res[DBLeaguer::SEND_FLAG].numberInt();
	leaguer_info.cur_contri_ = res[DBLeaguer::CUR_CONTRI].numberInt();
	leaguer_info.salary_flag_ = res[DBLeaguer::SALARY_FLAG].numberInt();
	leaguer_info.store_times_ =  res[DBLeaguer::STORE_TIMES].numberInt();
	leaguer_info.leave_type_ = res[DBLeaguer::LEAVE_TYPE].numberInt();
	leaguer_info.leave_tick_ = res[DBLeaguer::LEAVE_TICK].numberLong();
}

void MMOLeague::load_leaguer_fb_flag(BLongSet& lfb_flag)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBLeaguer::COLLECTION);
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		if(res[DBLeaguer::FB_FLAG].numberInt() > 0)
			lfb_flag.insert(res[DBLeaguer::ID].numberLong());
	}
}

void MMOLeague::save_leaguer_fb_flag(BLongSet& lfb_flag,int flag)
{
	for(BLongSet::const_iterator iter = lfb_flag.begin();
			iter != lfb_flag.end(); ++iter)
	{
		CACHED_CONNECTION.update(DBLeaguer::COLLECTION, BSON(DBLeaguer::ID
					<< *iter), BSON("$set" << BSON(DBLeaguer::FB_FLAG << flag)));
	}
}

void MMOLeague::load_leaguer_skill_prop(MapPlayerEx* player)
{
	BSONObj res = this->conection().findOne(DBLeaguer::COLLECTION,
			QUERY(DBLeaguer::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	MapLeaguerInfo &league_info = player->leaguer_info();
	GameCommon::bson_to_map(league_info.skill_map_,
			res.getObjectField(DBLeaguer::LEAGUE_SKILL.c_str()));
}

void MMOLeague::load_league_flag_prop(MapPlayerEx* player)
{
	BSONObj res = this->conection().findOne(DBLeague::COLLECTION, QUERY(DBLeague::ID << player->league_id()));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	MapLeaguerInfo &league_info = player->leaguer_info();
	int flag_lvl = res[DBLeague::FLAG_LVL].numberInt();
	league_info.flag_lvl_ = flag_lvl;

	Int64 leader_index = res[DBLeague::LEADER_INDEX].numberLong();
	league_info.leader_id_ = leader_index;
}

int MMOLeague::load_lwar_info(DBShopMode* shop_mode)
{
	BSONObj res = this->conection().findOne(DBLeagueWarInfo::COLLECTION,
			BSON(DBLeagueWarInfo::ID << int(0)));
	*shop_mode->output_argv_.bson_obj_ = res.copy();
	return 0;
}

int MMOLeague::save_league_index(int league_index)
{
    BSONObjBuilder builder;
    builder << Global::GID << league_index;

    GameCommon::request_save_mmo_begin(Global::COLLECTION,
    		BSON(Global::KEY << Global::LEAGUE), BSON("$set" << builder.obj()));
    return 0;
}

void MMOLeague::load_all_league()
{
	LongMap league_map;
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBLeague::COLLECTION);

	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		Int64 league_index = res[DBLeague::ID].numberLong();
		league_map[league_index] = true;
	}

	for (LongMap::iterator iter = league_map.begin(); iter != league_map.end(); ++iter)
	{
		BSONObj res = CACHED_CONNECTION.findOne(DBLeague::COLLECTION, BSON(DBLeague::ID << iter->first));
		JUDGE_CONTINUE(res.isEmpty() == false);

        League* league = LEAGUE_PACKAGE->pop_object();
        JUDGE_CONTINUE(league != NULL);

        league->league_index_ = res[DBLeague::ID].numberLong();
        league->league_name_ = res[DBLeague::LEAGUE_NAME].str();
        league->league_intro_ = res[DBLeague::LEAGUE_INTRO].str();
        league->league_lvl_ = res[DBLeague::LEAGUE_LVL].numberInt();
        league->league_resource_ = res[DBLeague::LEAGUE_RESOURCE].numberInt();
        league->region_rank_ = res[DBLeague::REGION_RANK].numberInt();
        league->region_tick_ = res[DBLeague::REGION_TICK].numberLong();
        league->region_leader_reward_ = res[DBLeague::REGION_LEADER_REWARD].numberInt();

        league->create_tick_ = res[DBLeague::CREATE_TICK].numberLong();
        league->leader_index_ = res[DBLeague::LEADER_INDEX].numberLong();
        league->last_login_tick_ = res[DBLeague::LAST_LOGIN].numberLong();

        league->auto_accept_ = res[DBLeague::AUTO_ACCEPT].numberInt();
        league->accept_force_ = res[DBLeague::ACCEPT_FORCE].numberInt();

        league->flag_lvl_ = res[DBLeague::FLAG_LVL].numberInt();
        league->flag_exp_ = res[DBLeague::FLAG_EXP].numberInt();

        // member
        BSONObjIterator iter(res.getObjectField(DBLeague::LEAGUE_MEMBER.c_str()));
        while (iter.more())
        {
        	BSONObj obj = iter.next().embeddedObject();

        	Int64 member_index = obj[DBLeague::Member::ROLE_INDEX].numberLong();
        	JUDGE_CONTINUE(MMORole::validate_league_member(member_index,
        			league->league_index_) == true);

        	LeagueMember& league_member = league->member_map_[member_index];

        	league_member.role_index_ = member_index;
        	MMORole::load_league_member(league_member);
        	MMOFashion::fetch_player_fashion(league_member);

        	league_member.join_tick_ = obj[DBLeague::Member::JOIN_TICK].numberLong();
        	league_member.league_pos_ = obj[DBLeague::Member::LEAGUE_POS].numberInt();
        	league_member.cur_contribute_ = obj[DBLeague::Member::CUR_CONTRI].numberInt();
        	league_member.today_contribute_= obj[DBLeague::Member::TODAY_CONTRI].numberInt();
        	league_member.total_contribute_= obj[DBLeague::Member::TOTAL_CONTRI].numberInt();
        	league_member.offline_contri_ = obj[DBLeague::Member::OFFLINE_CONTRI].numberInt();
        	league_member.today_rank_ = obj[DBLeague::Member::TODAY_RANK].numberInt();
        	league_member.total_rank_ = obj[DBLeague::Member::TOTAL_RANK].numberInt();
        	league_member.lrf_bet_league_id = obj[DBLeague::Member::LRF_BET_LEAGUE_ID].numberInt();
        }

        // applier
        iter = res.getObjectField(DBLeague::LEAGUE_APPLIER.c_str());
        while (iter.more())
        {
        	BSONObj obj = iter.next().embeddedObject();

        	long applier_index = obj[DBBaseRole::INDEX].numberLong();
        	JUDGE_CONTINUE(MMORole::validate_player(applier_index) == true);

        	LeagueApplier& league_applier = league->applier_map_[applier_index];

        	league_applier.role_index_ = applier_index;
        	league_applier.role_name_ = obj[DBBaseRole::NAME].str();
        	league_applier.vip_type_ = obj[DBBaseRole::VIP].numberInt();
        	league_applier.role_sex_ = obj[DBBaseRole::SEX].numberInt();
        	league_applier.role_lvl_ = obj[DBBaseRole::LVL].numberInt();
        	league_applier.role_force_ = obj[DBBaseRole::FORCE].numberInt();
        	league_applier.role_career_ = obj[DBBaseRole::CAREER].numberInt();
        }

        //league boss
        if (res.hasElement(DBLeague::LEAGUE_BOSS.c_str()) == true)
        {
        	BSONObj obj = res.getObjectField(DBLeague::LEAGUE_BOSS.c_str());
        	league->league_boss_.reset_tick_ = obj[DBLeague::LeagueBoss::RESET_TICK].numberLong();
        	league->league_boss_.boss_index_ = obj[DBLeague::LeagueBoss::BOSS_INDEX].numberInt();
        	league->league_boss_.boss_exp_ 	 = obj[DBLeague::LeagueBoss::BOSS_EXP].numberInt();
        	league->league_boss_.super_summon_role_ = obj[DBLeague::LeagueBoss::SUPER_SUMMON_ROLE].str();
        	league->league_boss_.normal_summon_tick_ = obj[DBLeague::LeagueBoss::NORMAL_SUMMON_TICK].numberLong();
        	league->league_boss_.super_summon_tick_ = obj[DBLeague::LeagueBoss::SUPER_SUMMON_TICK].numberLong();
        	league->league_boss_.normal_summon_type_ = obj[DBLeague::LeagueBoss::NORMAL_SUMMON_TYPE].numberInt();
        	league->league_boss_.super_summon_type_ = obj[DBLeague::LeagueBoss::SUPER_SUMMON_TYPE].numberInt();
        	league->league_boss_.normal_die_tick_ = obj[DBLeague::LeagueBoss::NORMAL_DIE_TICK].numberInt();
        	league->league_boss_.super_die_tick_ = obj[DBLeague::LeagueBoss::SUPER_DIE_TICK].numberInt();
        }

        // league_impeach
        if (res.hasElement(DBLeague::LEAGUE_IMPEACH.c_str()) == true)
        {
        	BSONObj obj = res.getObjectField(DBLeague::LEAGUE_IMPEACH.c_str());
        	league->league_impeach_.impeach_role_ = obj[DBLeague::LeagueImpeach::IMPEACH_ROLE].numberLong();
        	league->league_impeach_.impeach_tick_ = obj[DBLeague::LeagueImpeach::IMPEACH_TICK].numberLong();
        }
        DBCommon::bson_to_long_map(league->league_impeach_.voter_map_,
                res.getObjectField(DBLeague::VOTE_MAP.c_str()));

        // league_fb
        BSONObjIterator fb_iter(res.getObjectField(DBLeague::LFB_PLAYER_SET.c_str()));
        while (fb_iter.more())
        {
        	BSONObj obj = fb_iter.next().embeddedObject();
        	Int64 player_id = obj[DBLeague::LFbPlayerSet::PLAYER_ID].numberLong();
        	LFbPlayer &lfb_player = league->lfb_player_map_[player_id];
        	lfb_player.role_id_ = player_id;
        	lfb_player.tick_ = obj[DBLeague::LFbPlayerSet::TICK].numberLong();
        	lfb_player.name_ = obj[DBLeague::LFbPlayerSet::NAME].str();
        	lfb_player.sex_  = obj[DBLeague::LFbPlayerSet::SEX].numberInt();
        	lfb_player.wave_ = obj[DBLeague::LFbPlayerSet::WAVE].numberInt();
        	lfb_player.last_wave_ = obj[DBLeague::LFbPlayerSet::LAST_WAVE].numberInt();
        	lfb_player.cheer_ = obj[DBLeague::LFbPlayerSet::CHEER].numberInt();
        	lfb_player.encourage_ = obj[DBLeague::LFbPlayerSet::ENCOURAGE].numberInt();
        	lfb_player.be_cheer_ = obj[DBLeague::LFbPlayerSet::BE_CHEER].numberInt();
        	lfb_player.be_encourage_ = obj[DBLeague::LFbPlayerSet::BE_ENCOURAGE].numberInt();

        	BSONObjIterator record_iter(obj.getObjectField(DBLeague::LFbPlayerSet::RECORD_VEC.c_str()));
        	while (record_iter.more())
        	{
        		BSONObj record_obj = record_iter.next().embeddedObject();
        		CheerRecord record;
        		record.role_id_ = record_obj[DBLeague::LFbPlayerSet::RecordSet::ROLE_ID].numberLong();
        		record.type_ = record_obj[DBLeague::LFbPlayerSet::RecordSet::TYPE].numberInt();
        		record.is_active_ = record_obj[DBLeague::LFbPlayerSet::RecordSet::IS_ACTIVE].numberInt();
        		record.time_ = record_obj[DBLeague::LFbPlayerSet::RecordSet::TIME].numberInt();
        		record.name_ = record_obj[DBLeague::LFbPlayerSet::RecordSet::NAME].str();
        		lfb_player.record_vec_.push_back(record);
        	}
        }
        league->sort_lfb_player();
        league->set_max_last_wave_player();

        // league log
        iter = res.getObjectField(DBLeague::LEAGUE_LOG.c_str());
        while (iter.more())
        {
        	BSONObj obj = iter.next().embeddedObject();

        	LeagueLogItem log_item;
        	MMOLeague::unserial_league_log(log_item, obj);

        	JUDGE_CONTINUE(log_item.log_conent_.empty() == false);
        	league->league_log_set_.push_back(log_item);
        }

        LEAGUE_PACKAGE->bind_object(league->league_index_, league);
        league->caculate_league_force();
        league->handle_start_impeach();
	}
}

void MMOLeague::save_league(League* league, int direct_save)
{
	JUDGE_RETURN(league != NULL, ;);

	// member
	BSONVec member_set;
	member_set.reserve(league->member_map_.size());
	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		member_set.push_back(BSON(DBLeague::Member::ROLE_INDEX << iter->second.role_index_
				<< DBLeague::Member::JOIN_TICK << iter->second.join_tick_
				<< DBLeague::Member::LEAGUE_POS << iter->second.league_pos_
				<< DBLeague::Member::CUR_CONTRI << iter->second.cur_contribute_
				<< DBLeague::Member::TODAY_CONTRI << iter->second.today_contribute_
				<< DBLeague::Member::TOTAL_CONTRI << iter->second.total_contribute_
				<< DBLeague::Member::OFFLINE_CONTRI << iter->second.offline_contri_
				<< DBLeague::Member::TODAY_RANK << iter->second.today_rank_
				<< DBLeague::Member::TOTAL_RANK << iter->second.total_rank_
				<< DBLeague::Member::LRF_BET_LEAGUE_ID << iter->second.lrf_bet_league_id));
	}

	// applier
	BSONVec applier_set;
	applier_set.reserve(league->applier_map_.size());
	for (League::ApplierMap::iterator iter = league->applier_map_.begin();
			iter != league->applier_map_.end(); ++iter)
	{
		applier_set.push_back(BSON(DBBaseRole::INDEX << iter->second.role_index_
				<< DBBaseRole::VIP << iter->second.vip_type_
				<< DBBaseRole::SEX << iter->second.role_sex_
				<< DBBaseRole::NAME << iter->second.role_name_
				<< DBBaseRole::LVL << iter->second.role_lvl_
				<< DBBaseRole::FORCE << iter->second.role_force_
				<< DBBaseRole::CAREER << iter->second.role_career_));
	}

	// league log
	BSONVec league_log_set;
	MMOLeague::serial_league_log(league_log_set, league->league_log_set_);

	// league boss
	BSONObj league_boss_bson = BSON(
			DBLeague::LeagueBoss::RESET_TICK << league->league_boss_.reset_tick_
			<< DBLeague::LeagueBoss::BOSS_INDEX << league->league_boss_.boss_index_
			<< DBLeague::LeagueBoss::BOSS_EXP << league->league_boss_.boss_exp_
			<< DBLeague::LeagueBoss::SUPER_SUMMON_ROLE << league->league_boss_.super_summon_role_
			<< DBLeague::LeagueBoss::NORMAL_SUMMON_TICK << league->league_boss_.normal_summon_tick_
			<< DBLeague::LeagueBoss::SUPER_SUMMON_TICK << league->league_boss_.super_summon_tick_
			<< DBLeague::LeagueBoss::NORMAL_SUMMON_TYPE << league->league_boss_.normal_summon_type_
			<< DBLeague::LeagueBoss::SUPER_SUMMON_TYPE << league->league_boss_.super_summon_type_
			<< DBLeague::LeagueBoss::NORMAL_DIE_TICK << league->league_boss_.normal_die_tick_
			<< DBLeague::LeagueBoss::SUPER_DIE_TICK << league->league_boss_.super_die_tick_);

	// league_impeach
	BSONVec voter_map;
	DBCommon::long_map_to_bson(voter_map, league->league_impeach_.voter_map_);
	BSONObj league_impeach_bson = BSON(
			DBLeague::LeagueImpeach::IMPEACH_ROLE << league->league_impeach_.impeach_role_
			<< DBLeague::LeagueImpeach::IMPEACH_TICK << league->league_impeach_.impeach_tick_);

	// league fb
	BSONVec lfb_set;
	for (League::LFbPlayerMap::iterator iter = league->lfb_player_map_.begin();
			iter != league->lfb_player_map_.end(); ++iter)
	{
		BSONVec record_vec;
		LFbPlayer &lfb_player = iter->second;
		for (LFbPlayer::CheerRecordVec::iterator it = lfb_player.record_vec_.begin();
				it != lfb_player.record_vec_.end(); ++it)
		{
			record_vec.push_back(BSON(DBLeague::LFbPlayerSet::RecordSet::ROLE_ID << it->role_id_
					<< DBLeague::LFbPlayerSet::RecordSet::TYPE << it->type_
					<< DBLeague::LFbPlayerSet::RecordSet::IS_ACTIVE << it->is_active_
					<< DBLeague::LFbPlayerSet::RecordSet::TIME << it->time_
					<< DBLeague::LFbPlayerSet::RecordSet::NAME << it->name_));
		}

		lfb_set.push_back(BSON(DBLeague::LFbPlayerSet::PLAYER_ID << lfb_player.role_id_
				<< DBLeague::LFbPlayerSet::TICK << lfb_player.tick_
				<< DBLeague::LFbPlayerSet::NAME << lfb_player.name_
				<< DBLeague::LFbPlayerSet::SEX << lfb_player.sex_
				<< DBLeague::LFbPlayerSet::WAVE << lfb_player.wave_
				<< DBLeague::LFbPlayerSet::LAST_WAVE << lfb_player.last_wave_
				<< DBLeague::LFbPlayerSet::CHEER << lfb_player.cheer_
				<< DBLeague::LFbPlayerSet::ENCOURAGE << lfb_player.encourage_
				<< DBLeague::LFbPlayerSet::BE_CHEER << lfb_player.be_cheer_
				<< DBLeague::LFbPlayerSet::BE_ENCOURAGE << lfb_player.be_encourage_
				<< DBLeague::LFbPlayerSet::RECORD_VEC << record_vec));
	}

	// league
    BSONObjBuilder builder;
    builder << DBLeague::LEAGUE_NAME << league->league_name_
    		<< DBLeague::LEAGUE_INTRO << league->league_intro_
    		<< DBLeague::LEAGUE_LVL << league->league_lvl_
    		<< DBLeague::LEAGUE_RESOURCE << league->league_resource_
    		<< DBLeague::CREATE_TICK << league->create_tick_
    		<< DBLeague::LAST_LOGIN << league->last_login_tick_
    		<< DBLeague::LEADER_INDEX << league->leader_index_
    		<< DBLeague::AUTO_ACCEPT << league->auto_accept_
    		<< DBLeague::ACCEPT_FORCE << league->accept_force_
    		<< DBLeague::FLAG_LVL << league->flag_lvl_
    		<< DBLeague::FLAG_EXP << league->flag_exp_
    		<< DBLeague::REGION_RANK << league->region_rank_
    		<< DBLeague::REGION_TICK << league->region_tick_
    		<< DBLeague::REGION_LEADER_REWARD << league->region_leader_reward_
    		<< DBLeague::LEAGUE_MEMBER << member_set
    		<< DBLeague::LEAGUE_APPLIER << applier_set
    		<< DBLeague::LEAGUE_LOG << league_log_set
    		<< DBLeague::LEAGUE_BOSS << league_boss_bson
    		<< DBLeague::LEAGUE_IMPEACH << league_impeach_bson
    		<< DBLeague::VOTE_MAP << voter_map
    		<< DBLeague::LFB_PLAYER_SET << lfb_set;

    if (direct_save == false)
    {
		GameCommon::request_save_mmo_begin(DBLeague::COLLECTION,
				BSON(DBLeague::ID << league->league_index_),
				BSON("$set" << builder.obj()));
    }
    else
    {
    	CACHED_CONNECTION.update(DBLeague::COLLECTION,
				BSON(DBLeague::ID << league->league_index_),
				BSON("$set" << builder.obj()), true);
    }
}

void MMOLeague::remove_league(Int64 league_index)
{
	GameCommon::request_remove_mmo_begin(DBLeague::COLLECTION,
			BSON(DBLeague::ID << league_index));
}

void MMOLeague::save_quit_info(Int64 role_index, int leave_type, Int64 leave_tick)
{
    BSONObjBuilder builder;
    builder	<< DBLeaguer::LEAVE_TYPE << leave_type
    		<< DBLeaguer::LEAVE_TICK << leave_tick;

    GameCommon::request_save_mmo_begin(DBLeaguer::COLLECTION,
    		BSON(DBLeaguer::ID << role_index), BSON("$set" << builder.obj()));
}

std::string MMOLeague::fetch_league_name(const Int64 league_id)
{
	JUDGE_RETURN(league_id > 0, GameCommon::NullString);

	BSONObj ret_fields = BSON(DBLeague::LEAGUE_NAME << 1 << DBLeague::ID << 1);
	BSONObj res = CACHED_CONNECTION.findOne(DBLeague::COLLECTION,
			QUERY(DBLeague::ID << league_id), &ret_fields);
	JUDGE_RETURN(res.isEmpty() == false, GameCommon::NullString);

	return res[DBLeague::LEAGUE_NAME].str();
}

int MMOLeague::fetch_league_pos(const Int64 league_id, Int64 self_id)
{
	BSONObj ret_fields = BSON(DBLeague::LEAGUE_MEMBER << 1 << DBLeague::ID << 1);
	BSONObj res = CACHED_CONNECTION.findOne(DBLeague::COLLECTION, QUERY(DBLeague::ID << league_id), &ret_fields);
	if (res.isEmpty() == true)
		return 0;
	BSONObjIterator mem_iter = res.getObjectField(DBLeague::LEAGUE_MEMBER.c_str());
	while(mem_iter.more())
	{
		BSONObj mem_obj = mem_iter.next().embeddedObject();
		Int64 role_id = mem_obj[DBLeague::Member::ROLE_INDEX.c_str()].numberLong();
		JUDGE_CONTINUE(role_id == self_id);
		return mem_obj[DBLeague::Member::LEAGUE_POS.c_str()].numberInt();
	}
	return 0;
}

Int64 MMOLeague::fetch_leader_id(const Int64 league_id)
{
	Int64 leader_id = 0;
	BSONObj res = CACHED_CONNECTION.findOne(DBLeague::COLLECTION, QUERY(DBLeague::ID << league_id));
	if (res.isEmpty() == false)
	{
		return res[DBLeague::LEADER_INDEX].numberLong();
	}
	return leader_id;
}

void MMOLeague::save_leaguer_info(LogicPlayer* player, MongoDataMap *mongo_data)
{
	LeaguerInfo& leaguer_info = player->leaguer_info();

	BSONVec buy_vec;
	GameCommon::map_to_bson(buy_vec, leaguer_info.buy_map_);

	BSONVec skill_vec;
	GameCommon::map_to_bson(skill_vec, leaguer_info.skill_map_);

	BSONVec apply_vec;
	DBCommon::long_map_to_bson(apply_vec, leaguer_info.apply_list_);

	BSONVec region_draw_vec;
	GameCommon::map_to_bson(region_draw_vec, leaguer_info.region_draw_);

	BSONVec wave_reward_set;
	for (std::map<int, IntMap>::const_iterator iter = leaguer_info.wave_reward_map_.begin();
			iter != leaguer_info.wave_reward_map_.end(); ++iter)
	{
		BSONVec bson_set;
		bson_set.reserve(iter->second.size());
		for (IntMap::const_iterator it = iter->second.begin();
				it != iter->second.end(); ++it)
		{
			JUDGE_CONTINUE(it->first != 0);
			bson_set.push_back(BSON(DBPairObj::KEY << it->first
					<< DBPairObj::VALUE << it->second));
		}
		wave_reward_set.push_back(BSON(DBPairObj::KEY << iter->first
				<< DBPairObj::VALUE << bson_set));
	}

    BSONObjBuilder builder;
    builder << DBLeaguer::SHOP_BUY << buy_vec
    		<< DBLeaguer::WAVE_REWARD_MAP << wave_reward_set
    		<< DBLeaguer::SIEGE_SHOP_REFRESH << Int64(leaguer_info.day_siege_refresh_tick_.sec())
    		<< DBLeaguer::DAY_ADMIRE_TIMES << leaguer_info.day_admire_times_
    		<< DBLeaguer::LEAGUE_SKILL << skill_vec
    		<< DBLeaguer::APPLY_LIST << apply_vec
    		<< DBLeaguer::REGION_DRAW << region_draw_vec
    		<< DBLeaguer::OPEN << leaguer_info.open_
    		<< DBLeaguer::DRAW_WELFARE << leaguer_info.draw_welfare_
    		<< DBLeaguer::GOLD_DONATE << leaguer_info.gold_donate_
    		<< DBLeaguer::WAND_DONATE << leaguer_info.wand_donate_
    		<< DBLeaguer::SEND_FLAG << leaguer_info.send_flag_
    		<< DBLeaguer::CUR_CONTRI << leaguer_info.cur_contri_
    		<< DBLeaguer::SALARY_FLAG << leaguer_info.salary_flag_
    		<< DBLeaguer::STORE_TIMES << leaguer_info.store_times_
    		<< DBLeaguer::LEAVE_TYPE << leaguer_info.leave_type_
    		<< DBLeaguer::LEAVE_TICK << leaguer_info.leave_tick_;

    mongo_data->push_update(DBLeaguer::COLLECTION, BSON(DBLeaguer::ID
    		<< player->role_id()), builder.obj());
}

void MMOLeague::save_leaguer_info(MapPlayerEx* player, MongoDataMap *mongo_data)
{
}

void MMOLeague::save_league_war(LeagueWarInfo& war_info)
{
	BSONVec rank_set;
	rank_set.reserve(war_info.lwar_rank_map_.size());

	for (LeagueWarInfo::LWarRankMap::iterator iter = war_info.lwar_rank_map_.begin();
			iter != war_info.lwar_rank_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.league_index_ > 0);

		rank_set.push_back(BSON(DBLeagueWarInfo::LeagueWarRank::ID << iter->second.rank_
				<< DBLeagueWarInfo::LeagueWarRank::LEAGUE_INDEX << iter->second.league_index_
				<< DBLeagueWarInfo::LeagueWarRank::LEAGUE_NAME << iter->second.league_name_
				<< DBLeagueWarInfo::LeagueWarRank::LEADER << iter->second.leader_
				<< DBLeagueWarInfo::LeagueWarRank::FORCE << iter->second.force_
				<< DBLeagueWarInfo::LeagueWarRank::SCORE << iter->second.score_
				<< DBLeagueWarInfo::LeagueWarRank::FLAG_LVL << iter->second.flag_lvl_));
	}

	BSONObjBuilder builder;
	builder << DBLeagueWarInfo::TOTAL_NUM << war_info.total_num_
			<< DBLeagueWarInfo::LAST_TICK << war_info.tick_
			<< DBLeagueWarInfo::RANK_SET << rank_set;

	GameCommon::request_save_mmo_begin(DBLeagueWarInfo::COLLECTION,
	    	BSON(DBLeagueWarInfo::ID << int(0)), BSON("$set" << builder.obj()));
}

void MMOLeague::load_league_war(LeagueWarInfo& war_info)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBLeagueWarInfo::COLLECTION,
			BSON(DBLeagueWarInfo::ID << int(0)));
	MMOLeague::load_league_war(war_info, &res);
}

void MMOLeague::load_league_war(LeagueWarInfo& war_info, BSONObj* p_res)
{
	const BSONObj& res = *p_res;
	JUDGE_RETURN(res.isEmpty() == false, ;);

	war_info.total_num_ = res[DBLeagueWarInfo::TOTAL_NUM].numberInt();
	war_info.tick_ = res[DBLeagueWarInfo::LAST_TICK].numberLong();

	int rank = 1;
	BSONObjIterator iter(res.getObjectField(DBLeagueWarInfo::RANK_SET.c_str()));
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();
		LeagueWarInfo::LeagueWarRank& rank_info = war_info.lwar_rank_map_[rank];

		rank_info.rank_ = rank;
		rank_info.league_name_ = obj[DBLeagueWarInfo::LeagueWarRank::LEAGUE_NAME].str();
		rank_info.league_index_ = obj[DBLeagueWarInfo::LeagueWarRank::LEAGUE_INDEX].numberLong();
		rank_info.force_ = obj[DBLeagueWarInfo::LeagueWarRank::FORCE].numberInt();
		rank_info.leader_ = obj[DBLeagueWarInfo::LeagueWarRank::LEADER].str();
		rank_info.score_ = obj[DBLeagueWarInfo::LeagueWarRank::SCORE].numberInt();
		rank_info.flag_lvl_ = obj[DBLeagueWarInfo::LeagueWarRank::FLAG_LVL].numberInt();

		++rank;
	}
}

void MMOLeague::save_arena(AreaSysDetail* arena_detail, int direct_save)
{
	//save arena role
	for (LongMap::iterator iter = arena_detail->rank_set_.begin();
			iter != arena_detail->rank_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(arena_detail->role_map_.count(iter->second) > 0);

		ArenaRole& arena_role = arena_detail->role_map_[iter->second];
		arena_role.reset_everyday();

		MMOLeague::save_arena_role(arena_role, direct_save);
	}

	//save arena system
	{
		arena_detail->re_rank_ = 1;

		BSONObjBuilder builder;
		builder << DBLWTicker::Arena::TIMEOUT_TICK << arena_detail->timeout_tick_
				<< DBLWTicker::Arena::RE_RANK << arena_detail->re_rank_;

		if (direct_save == true)
		{
			CACHED_CONNECTION.update(DBLWTicker::COLLECTION, BSON(DBLWTicker::ID << int(1)),
					BSON("$set" << builder.obj()), true);
		}
		else
		{
			GameCommon::request_save_mmo_begin(DBLWTicker::COLLECTION,
					BSON(DBLWTicker::ID << int(1)),	BSON("$set" << builder.obj()), true);
		}
	}
}

void MMOLeague::sort_arena(AreaSysDetail* arena_detail)
{

	do
	{
		BSONObj res = CACHED_CONNECTION.findOne(DBLWTicker::COLLECTION,
				BSON(DBLWTicker::ID << int(1)));
		JUDGE_BREAK(res.isEmpty() == false);

		arena_detail->timeout_tick_ = res[DBLWTicker::Arena::TIMEOUT_TICK].numberLong();
//		arena_detail->re_rank_ = res[DBLWTicker::Arena::RE_RANK].numberInt();
	} while (false);

	MSG_USER("AreaSys need sort work   rank_set size : %d  role_map size %d",
			arena_detail->rank_set_.size(), arena_detail->role_map_.size());

    arena_detail->rank_set_.clear();
	{
		PairObjVec pair_set;
		pair_set.reserve(GameEnum::DEFAULT_VECTOR_SIZE * 10);

	    auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBArenaRole::COLLECTION);
	    while (cursor->more())
	    {
	        BSONObj res = cursor->next();

	        Int64 role_id = res[DBArenaRole::ID].numberLong();
	        JUDGE_CONTINUE(MMORole::validate_player(role_id) == true);

	        ArenaRole& arena_role = arena_detail->role_map_[role_id];
	        arena_role.id_ = role_id;

	        arena_role.name_ = res[DBArenaRole::NAME].str();
	        arena_role.sex_ = res[DBArenaRole::SEX].numberInt();
	        arena_role.career_ = res[DBArenaRole::CAREER].numberInt();
	        arena_role.force_ = res[DBArenaRole::FORCE].numberInt();
	        arena_role.level_ = res[DBArenaRole::LEVEL].numberInt();
	        arena_role.wing_level_ = res[DBArenaRole::WING_LEVEL].numberInt();
	        arena_role.solider_level_ = res[DBArenaRole::SOLIDER_LEVEL].numberInt();
	        arena_role.reward_level_ = res[DBArenaRole::REWARD_LEVEL].numberInt();
	        arena_role.refresh_tick_.sec(res[DBArenaRole::REFRESH_TICK].numberInt());
	        arena_role.left_times_ = res[DBArenaRole::LEFT_TIMES].numberInt();
	        arena_role.buy_times_ = res[DBArenaRole::BUY_TIMES].numberInt();
	        arena_role.rank_ = res[DBArenaRole::RANK].numberInt();
	        arena_role.is_skip_ = res[DBArenaRole::IS_SKIP].numberInt();
	        arena_role.is_over_limit_ = res[DBArenaRole::IS_OVER_LIMIT].numberInt();
	        arena_role.open_flag_ = res[DBArenaRole::OPEN_FLAG].numberInt();

	        arena_detail->rank_set_[arena_role.rank_] = arena_role.id_;

	        DBCommon::bson_to_int_vec(arena_role.fight_set_,
	        		res.getObjectField(DBArenaRole::FIGHT_SET.c_str()));
	        arena_role.last_rank_ = res[DBArenaRole::LAST_RANK].numberInt();
	        arena_role.reward_id_ = res[DBArenaRole::ADD_ANIMA].numberInt();
	        arena_role.continue_win_ = res[DBArenaRole::CONTINUE_WIN].numberInt();

	    	BSONObjIterator record_iter(res.getObjectField(DBArenaRole::HIS_RECORD.c_str()));
	    	while (record_iter.more())
	    	{
	    		BSONObj record_obj = record_iter.next().embeddedObject();

	    		ArenaRole::Record record;
	    		record.tick_ = record_obj[DBArenaRole::FIGHT_TICK].numberInt();
	    		record.fight_type_ = record_obj[DBArenaRole::FIGHT_TYPE].numberInt();
	    		record.fight_state_ = record_obj[DBArenaRole::FIGHT_STATE].numberInt();
	    		record.name_ = record_obj[DBArenaRole::FIGHT_NAME].str();
	    		record.rank_ = record_obj[DBArenaRole::FIGHT_RANK].numberInt();
	    		record.rank_change_ = record_obj[DBArenaRole::RANK_CHANGE].numberInt();

	    		arena_role.push_record(record);
	    	}

	    	PairObj obj;
	    	obj.id_ = role_id;

//	    	if (arena_detail->re_rank_ == 2)
//	    	{
	    		obj.value_ = arena_role.force_;
//	    	}
//	    	else
//	    	{
//	    		obj.value_ = arena_role.rank_;
//	    	}

	    	pair_set.push_back(obj);
	    }

//	    //和服才重新排名
//	    if (arena_detail->re_rank_ == 2)
//	    {
	    	std::sort(pair_set.begin(), pair_set.end(), GameCommon::pair_comp_by_desc);

		    int re_rank = 0;
		    arena_detail->rank_set_.clear();

		    for (PairObjVec::iterator iter = pair_set.begin(); iter != pair_set.end(); ++iter)
		    {
		    	re_rank += 1;

		    	ArenaRole& arena_role = arena_detail->role_map_[iter->id_];
		    	arena_role.rank_ = re_rank;

		        arena_detail->rank_set_[arena_role.rank_] = arena_role.id_;
		    }
//	    }
//	    else
//	    {
//	    	std::sort(pair_set.begin(), pair_set.end(), GameCommon::pair_comp_by_asc);
//	    }

		MSG_USER("AreaSys need sort work   rank_set size : %d  role_map size %d",
				(int)arena_detail->rank_set_.size(), (int)arena_detail->role_map_.size());
//	    arena_detail->re_rank_ = 1;

	    LongMap check_map;
		int length = (int)arena_detail->role_map_.size();
		for (int i = 1; i <= ::std::max(2000, length); i++)
	    {
	    	Int64 rpm_id = CONFIG_INSTANCE->athletics_rank(i)["rpm_id"].asInt();

			if (check_map.count(rpm_id) == 0)
				check_map[rpm_id] = 1;
			else
				check_map[rpm_id]++;

	//		if (check_map[rpm_id] == GameEnum::AREA_BASE_RANK_NUM)
	//			check_map[rpm_id] -= GameEnum::AREA_BASE_RANK_NUM;

			check_map[rpm_id] = check_map[rpm_id] % GameEnum::ARENA_BASE_RANK_NUM;
			rpm_id += check_map[rpm_id] * GameEnum::ARENA_BASE_ID_NUM;

	    	Int64 robot_id = arena_detail->robot_id_map[rpm_id];

	    	ArenaRole& arena_role = arena_detail->arena_role_set_[robot_id];
	    	if (arena_detail->rank_set_[i] == 0)
	    	{
	    		arena_detail->rank_set_[i] = arena_role.id_;
	    		arena_role.rank_ = i;
	    	}

	    	if (arena_detail->role_map_.count(arena_role.id_) == 0)
	    		arena_detail->role_map_[arena_role.id_] = arena_role;
	    }

		MSG_USER("AreaSys need sort done  rank_set size : %d  role_map size %d",
				(int)arena_detail->rank_set_.size(), (int)arena_detail->role_map_.size());
	}
}

void MMOLeague::load_arena(AreaSysDetail* arena_detail)
{
	do
	{
		BSONObj res = CACHED_CONNECTION.findOne(DBLWTicker::COLLECTION,
				BSON(DBLWTicker::ID << int(1)));
		JUDGE_BREAK(res.isEmpty() == false);

		arena_detail->timeout_tick_ = res[DBLWTicker::Arena::TIMEOUT_TICK].numberLong();
		arena_detail->re_rank_ = res[DBLWTicker::Arena::RE_RANK].numberInt();
	} while (false);

	MSG_USER("AreaSys refresh work   rank_set size : %d  role_map size %d",
			(int)arena_detail->rank_set_.size(), (int)arena_detail->role_map_.size());

    arena_detail->rank_set_.clear();
	{
		PairObjVec pair_set;
		pair_set.reserve(GameEnum::DEFAULT_VECTOR_SIZE * 10);

	    auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBArenaRole::COLLECTION);
	    while (cursor->more())
	    {
	        BSONObj res = cursor->next();

	        Int64 role_id = res[DBArenaRole::ID].numberLong();
	        JUDGE_CONTINUE(MMORole::validate_player(role_id) == true);

	        ArenaRole& arena_role = arena_detail->role_map_[role_id];
	        arena_role.id_ = role_id;

	        arena_role.name_ = res[DBArenaRole::NAME].str();
	        arena_role.sex_ = res[DBArenaRole::SEX].numberInt();
	        arena_role.career_ = res[DBArenaRole::CAREER].numberInt();
	        arena_role.force_ = res[DBArenaRole::FORCE].numberInt();
	        arena_role.level_ = res[DBArenaRole::LEVEL].numberInt();
	        arena_role.wing_level_ = res[DBArenaRole::WING_LEVEL].numberInt();
	        arena_role.solider_level_ = res[DBArenaRole::SOLIDER_LEVEL].numberInt();
	        arena_role.reward_level_ = res[DBArenaRole::REWARD_LEVEL].numberInt();
	        arena_role.refresh_tick_.sec(res[DBArenaRole::REFRESH_TICK].numberInt());
	        arena_role.left_times_ = res[DBArenaRole::LEFT_TIMES].numberInt();
	        arena_role.buy_times_ = res[DBArenaRole::BUY_TIMES].numberInt();
	        arena_role.rank_ = res[DBArenaRole::RANK].numberInt();
	        arena_role.is_skip_ = res[DBArenaRole::IS_SKIP].numberInt();
	        arena_role.is_over_limit_ = res[DBArenaRole::IS_OVER_LIMIT].numberInt();
	        arena_role.open_flag_ = res[DBArenaRole::OPEN_FLAG].numberInt();

//	        arena_detail->rank_set_[arena_role.rank_] = arena_role.id_;

	        DBCommon::bson_to_int_vec(arena_role.fight_set_,
	        		res.getObjectField(DBArenaRole::FIGHT_SET.c_str()));
	        arena_role.last_rank_ = res[DBArenaRole::LAST_RANK].numberInt();
	        arena_role.reward_id_ = res[DBArenaRole::ADD_ANIMA].numberInt();
	        arena_role.continue_win_ = res[DBArenaRole::CONTINUE_WIN].numberInt();

	    	BSONObjIterator record_iter(res.getObjectField(DBArenaRole::HIS_RECORD.c_str()));
	    	while (record_iter.more())
	    	{
	    		BSONObj record_obj = record_iter.next().embeddedObject();

	    		ArenaRole::Record record;
	    		record.tick_ = record_obj[DBArenaRole::FIGHT_TICK].numberInt();
	    		record.fight_type_ = record_obj[DBArenaRole::FIGHT_TYPE].numberInt();
	    		record.fight_state_ = record_obj[DBArenaRole::FIGHT_STATE].numberInt();
	    		record.name_ = record_obj[DBArenaRole::FIGHT_NAME].str();
	    		record.rank_ = record_obj[DBArenaRole::FIGHT_RANK].numberInt();
	    		record.rank_change_ = record_obj[DBArenaRole::RANK_CHANGE].numberInt();

	    		arena_role.push_record(record);
	    	}

	    	//处理排名重复bug
			bool is_same = arena_detail->is_same_player(arena_role.rank_);
			if (is_same == true && arena_detail->re_rank_ == 1)
			{
				Int64 before__id = arena_detail->rank_set_[arena_role.rank_];
				ArenaRole& before_role = arena_detail->role_map_[before__id];
				if (before_role.force_ < arena_role.force_)
				{
					arena_detail->rank_set_[arena_role.rank_] = arena_role.id_;
					arena_detail->same_player_map_[before__id] = true;
				}
				else
				{
					arena_detail->same_player_map_[arena_role.id_] = true;
				}
			}
			else
			{
				arena_detail->rank_set_[arena_role.rank_] = arena_role.id_;
			}

	    	PairObj obj;
	    	obj.id_ = role_id;

	    	if (arena_detail->re_rank_ == 2)
	    	{
	    		obj.value_ = arena_role.force_;
	    	}
	    	else
	    	{
	    		obj.value_ = arena_role.rank_;
	    	}

	    	pair_set.push_back(obj);
	    }

	    //和服才重新排名
	    if (arena_detail->re_rank_ == 2)
	    {
	    	std::sort(pair_set.begin(), pair_set.end(), GameCommon::pair_comp_by_desc);

		    int re_rank = 0;
		    arena_detail->rank_set_.clear();

		    for (PairObjVec::iterator iter = pair_set.begin(); iter != pair_set.end(); ++iter)
		    {
		    	re_rank += 1;

		    	ArenaRole& arena_role = arena_detail->role_map_[iter->id_];
		    	arena_role.rank_ = re_rank;

		        arena_detail->rank_set_[arena_role.rank_] = arena_role.id_;
		    }
	    }
	    else
	    {
	    	std::sort(pair_set.begin(), pair_set.end(), GameCommon::pair_comp_by_asc);
	    }

		MSG_USER("AreaSys refresh work   rank_set size : %d  role_map size %d",
				(int)arena_detail->rank_set_.size(), (int)arena_detail->role_map_.size());

	    arena_detail->re_rank_ = 1;

	    LongMap check_map;
		int length = (int)arena_detail->role_map_.size();
		for (int i = 1; i <= ::std::max(2000, length); i++)
	    {
	    	Int64 rpm_id = CONFIG_INSTANCE->athletics_rank(i)["rpm_id"].asInt();

			if (check_map.count(rpm_id) == 0)
				check_map[rpm_id] = 1;
			else
				check_map[rpm_id]++;

	//		if (check_map[rpm_id] == GameEnum::AREA_BASE_RANK_NUM)
	//			check_map[rpm_id] -= GameEnum::AREA_BASE_RANK_NUM;

			check_map[rpm_id] = check_map[rpm_id] % GameEnum::ARENA_BASE_RANK_NUM;
			rpm_id += check_map[rpm_id] * GameEnum::ARENA_BASE_ID_NUM;

	    	Int64 robot_id = arena_detail->robot_id_map[rpm_id];

	    	ArenaRole& arena_role = arena_detail->arena_role_set_[robot_id];
	    	if (arena_detail->rank_set_[i] == 0)
	    	{
	    		arena_detail->rank_set_[i] = arena_role.id_;
	    		arena_role.rank_ = i;
	    	}

	    	if (arena_detail->role_map_.count(arena_role.id_) == 0)
	    		arena_detail->role_map_[arena_role.id_] = arena_role;
	    }

	}
}

void MMOLeague::load_arena_guide(AreaSysDetail* arena_detail)
{
	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank();
	Json::Value::iterator it = rank_json.begin();
	for (int i = GameEnum::ARENA_RANK_END; i >= GameEnum::ARENA_RANK_START; --i)
	{
		const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank_by_id(i);

		for (int j = 0; j < GameEnum::ARENA_BASE_RANK_NUM; j++)
		{
			int guide_id = rank_json["rpm_id"].asInt() + j * GameEnum::ARENA_BASE_ID_NUM;

			BSONObj res = CACHED_CONNECTION.findOne(DBRpmFakeInfo::COLLECTION,
					QUERY(DBRpmFakeInfo::CONFIG_ID << guide_id));
			JUDGE_RETURN(res.isEmpty() == false, ;);

			ArenaRole arena_role;
			arena_role.id_ = res[DBRpmFakeInfo::ID].numberLong();
			arena_role.name_ = res[DBRpmFakeInfo::NAME].str();
			arena_role.force_ = res[DBRpmFakeInfo::FORCE].numberInt();
			arena_role.level_ = res[DBRpmFakeInfo::LEVEL].numberInt();
			arena_role.sex_ = res[DBRpmFakeInfo::SEX].numberInt();
			arena_role.career_ = res[DBRpmFakeInfo::CAREER].numberInt();
			arena_role.wing_level_ = res[DBRpmFakeInfo::WING_LEVEL].numberInt();
			arena_role.solider_level_ = res[DBRpmFakeInfo::SOLIDER_LEVEL].numberInt();

			arena_detail->robot_id_map.insert(LongMap::value_type(guide_id, arena_role.id_));
			arena_detail->arena_role_set_.insert(std::map<Int64, ArenaRole>::value_type(arena_role.id_, arena_role));
		}
	}
}

void MMOLeague::save_arena_role(const ArenaRole& arena_role, int direct_save)
{
	BSONVec his_record_set;
	for (std::list<ArenaRole::Record>::const_iterator iter = arena_role.his_record_.begin();
			iter != arena_role.his_record_.end(); ++iter)
	{
		his_record_set.push_back(BSON(DBArenaRole::FIGHT_TICK << iter->tick_
				<< DBArenaRole::FIGHT_TYPE << iter->fight_type_
				<< DBArenaRole::FIGHT_STATE << iter->fight_state_
				<< DBArenaRole::FIGHT_NAME << iter->name_
				<< DBArenaRole::FIGHT_RANK << iter->rank_
				<< DBArenaRole::RANK_CHANGE << iter->rank_change_));
	}

	BSONObjBuilder builder;
	builder << DBArenaRole::NAME << arena_role.name_
			<< DBArenaRole::SEX << arena_role.sex_
			<< DBArenaRole::CAREER << arena_role.career_
			<< DBArenaRole::FORCE << arena_role.force_
			<< DBArenaRole::LEVEL << arena_role.level_
			<< DBArenaRole::WING_LEVEL << arena_role.wing_level_
			<< DBArenaRole::SOLIDER_LEVEL << arena_role.solider_level_
			<< DBArenaRole::REWARD_LEVEL << arena_role.reward_level_
			<< DBArenaRole::REFRESH_TICK << int(arena_role.refresh_tick_.sec())
			<< DBArenaRole::LEFT_TIMES << arena_role.left_times_
			<< DBArenaRole::BUY_TIMES << arena_role.buy_times_
			<< DBArenaRole::RANK << arena_role.rank_
			<< DBArenaRole::IS_SKIP << arena_role.is_skip_
			<< DBArenaRole::IS_OVER_LIMIT << arena_role.is_over_limit_
			<< DBArenaRole::OPEN_FLAG << arena_role.open_flag_
			<< DBArenaRole::FIGHT_SET << arena_role.fight_set_
			<< DBArenaRole::LAST_RANK << arena_role.last_rank_
			<< DBArenaRole::ADD_ANIMA << arena_role.reward_id_
			<< DBArenaRole::CONTINUE_WIN << arena_role.continue_win_
			<< DBArenaRole::HIS_RECORD << his_record_set;

	if (direct_save == true)
	{
		CACHED_CONNECTION.update(DBArenaRole::COLLECTION,
				BSON(DBArenaRole::ID << arena_role.id_),
				BSON("$set" << builder.obj()), true);
	}
	else
	{
		GameCommon::request_save_mmo_begin(DBArenaRole::COLLECTION,
				BSON(DBArenaRole::ID << arena_role.id_),
				BSON("$set" << builder.obj()), true);
	}
}

void MMOLeague::ensure_all_index()
{
	this->conection().ensureIndex(DBLeague::COLLECTION, BSON(DBLeague::ID << 1), true);
	this->conection().ensureIndex(DBLeaguer::COLLECTION, BSON(DBLeaguer::ID << 1), true);
	this->conection().ensureIndex(DBLWTicker::COLLECTION, BSON(DBLWTicker::ID << 1), true);
	this->conection().ensureIndex(DBArenaRole::COLLECTION, BSON(DBArenaRole::ID << 1), true);
	this->conection().ensureIndex(DBLeagueWarInfo::COLLECTION, BSON(DBLeagueWarInfo::ID << 1), true);
	BEGIN_CATCH
		this->conection().ensureIndex(DBLeagueRegionFight::COLLECTION, BSON(DBLeagueRegionFight::ID << 1), true);
	END_CATCH
}

void MMOLeague::unserial_league_log(LeagueLogItem& log_item, BSONObj& bson)
{
	log_item.log_tick_ = bson[DBLeague::Log::LOG_TICK].numberLong();
	log_item.log_conent_ = bson[DBLeague::Log::LOG_CONTENT].str();
}

void MMOLeague::serial_league_log(BSONVec& bson_set, League::LeagueLogSet& league_log)
{
	bson_set.reserve(league_log.size());

	for (League::LeagueLogSet::iterator iter = league_log.begin();
			iter != league_log.end(); ++iter)
	{
		bson_set.push_back(BSON(DBLeague::Log::LOG_TICK << iter->log_tick_
				<< DBLeague::Log::LOG_CONTENT << iter->log_conent_));
	}
}

int MMOLeague::loadLeagueRegionFightInfo(LeagueRegionResult& lrf_info)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBLeagueRegionFight::COLLECTION,
				BSON(DBLeagueRegionFight::ID << int(0)));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	lrf_info.finish_ = res[DBLeagueRegionFight::FINISH].numberInt();
	lrf_info.tick_ = res[DBLeagueRegionFight::LAST_TICK].numberLong();
	BSONObjIterator f_iter(res.getObjectField(DBLeagueRegionFight::LRF_RESULT.c_str()));

	int rank = 1;
	while (f_iter.more())
	{
		BSONObj obj = f_iter.next().embeddedObject();

		LRFLeagueInfo& item_info = lrf_info.history_league_[rank];
		item_info.rank_ = rank;
		item_info.id_ = obj[DBLeagueRegionFight::LRF_RES::LEAGUE_ID].numberLong();
		item_info.name_ = obj[DBLeagueRegionFight::LRF_RES::LEAGUE_NAME].str();
		item_info.leader_ = obj[DBLeagueRegionFight::LRF_RES::LEADER].str();
		item_info.force_ = obj[DBLeagueRegionFight::LRF_RES::FORCE].numberInt();

		lrf_info.history_result_[item_info.id_] = (rank + 1) % 2 + 1;
		++rank;
	}

	BSONObjIterator s_iter(res.getObjectField(DBLeagueRegionFight::SUPP_RESULT.c_str()));
	while (s_iter.more())
	{
		BSONObj obj = s_iter.next().embeddedObject();

		Int64 role = obj[DBLeagueRegionFight::SUPP_RES::ROLE_ID].numberLong();
		LRFBetSupport& support = lrf_info.bet_support_[role];

		support.role_id = role;
		support.support_league_id = obj[DBLeagueRegionFight::SUPP_RES::SUPPORT_LEAGUE].numberLong();
		support.result_ = obj[DBLeagueRegionFight::SUPP_RES::RESULT].numberInt();
	}

	return 0;
}

void MMOLeague::updateLeagueRegionFightInfo(LeagueRegionResult& lrf_info)
{
	BSONVec region_vec;
	BSONVec support_vec;

	region_vec.reserve(lrf_info.history_league_.size());
	support_vec.reserve(lrf_info.bet_support_.size());

	for (LeagueRegionResult::HistoryMap::iterator iter = lrf_info.history_league_.begin();
			iter != lrf_info.history_league_.end(); ++iter)
	{
		LRFLeagueInfo& item_info = iter->second;
		region_vec.push_back(BSON(DBLeagueRegionFight::LRF_RES::LEAGUE_RANK << item_info.rank_
				<< DBLeagueRegionFight::LRF_RES::LEAGUE_ID << item_info.id_
				<< DBLeagueRegionFight::LRF_RES::LEAGUE_NAME << item_info.name_
				<< DBLeagueRegionFight::LRF_RES::LEADER << item_info.leader_
				<< DBLeagueRegionFight::LRF_RES::FORCE << item_info.force_));
	}

	for (LeagueRegionResult::SupportMap::iterator iter = lrf_info.bet_support_.begin();
			iter != lrf_info.bet_support_.end(); ++iter)
	{
		LRFBetSupport& item_info = iter->second;
		support_vec.push_back(BSON(DBLeagueRegionFight::SUPP_RES::ROLE_ID << item_info.role_id
				<< DBLeagueRegionFight::SUPP_RES::SUPPORT_LEAGUE << item_info.support_league_id
				<< DBLeagueRegionFight::SUPP_RES::RESULT << item_info.result_));
	}

	BSONObjBuilder builder;
	builder << DBLeagueRegionFight::FINISH << lrf_info.finish_
			<< DBLeagueRegionFight::LAST_TICK << lrf_info.tick_
			<< DBLeagueRegionFight::LRF_RESULT << region_vec
			<< DBLeagueRegionFight::SUPP_RESULT << support_vec;

	GameCommon::request_save_mmo_begin(DBLeagueRegionFight::COLLECTION,
			BSON(DBLeagueRegionFight::ID << int(0)), BSON("$set" << builder.obj()));
}
