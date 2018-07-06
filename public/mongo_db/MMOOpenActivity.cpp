/*
 * MMOOpenActivity.cpp
 *
 *  Created on: Sep 4, 2014
 *      Author: jinxing
 */

#include "MMOOpenActivity.h"
#include "GameField.h"
#include "BackField.h"
#include "MongoDataMap.h"
#include "MongoConnector.h"
#include "LogicPlayer.h"
#include "DBCommon.h"
#include "OpenActivitySys.h"
#include "FestActivitySys.h"
#include "MayActivitySys.h"

#include <mongo/client/dbclient.h>

MMOOpenActivity::MMOOpenActivity()
{
	// TODO Auto-generated constructor stub

}

MMOOpenActivity::~MMOOpenActivity()
{
	// TODO Auto-generated destructor stub
}

int MMOOpenActivity::load_open_activity(LogicPlayer *player)
{
	BSONObj res = this->conection().findOne(DBOpenActivity::COLLECTION,
			QUERY(DBOpenActivity::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	MLActivityerDetial& draw_detail = player->fetch_act_detail();
	BSONObjIterator iter(res.getObjectField(DBOpenActivity::OPEN_ACT.c_str()));

	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();

		Int64 start = obj[DBOpenActivity::Act::START_TICK].numberLong();
		Int64 stop = obj[DBOpenActivity::Act::STOP_TICK].numberLong();

		int act_id = obj[DBOpenActivity::Act::ID].numberInt();
		MLActivityerDetial::ActTypeItem& act_t_item = draw_detail.act_type_item_set_[act_id];
		act_t_item.star_tick_ = start;
		act_t_item.stop_tick_ = stop;
		act_t_item.sub_value_ = obj[DBOpenActivity::Act::SUB_VALUE].numberLong();
		act_t_item.update_tick_ = obj[DBOpenActivity::Act::UPDATE_TICK].numberLong();
		act_t_item.second_sub_ = obj[DBOpenActivity::Act::SECOND_SUB].numberInt();

		BSONObjIterator sec_iter(obj.getObjectField(DBOpenActivity::Act::DRAWED_SET.c_str()));
		while (sec_iter.more())
		{
			BSONObj sec_obj = sec_iter.next().embeddedObject();

			int aid = sec_obj[DBOpenActivity::Act::ID].numberInt();
			MLActivityerDetial::ActItem& act_item = act_t_item.act_item_map_[aid];

			act_item.arrive_ = sec_obj[DBOpenActivity::Act::ARRIVE].numberInt();
			act_item.drawed_ = sec_obj[DBOpenActivity::Act::DRAWED].numberInt();

			GameCommon::bson_to_map(act_item.arrive_map_,
					sec_obj.getObjectField(DBOpenActivity::Act::ARRIVE_MAP.c_str()));
		}

		player->role_detail().cur_red_packet_group_ = res[DBOpenActivity::RED_PACKET_GROUP].numberInt();
		player->role_detail().act_data_reset_flag_ = res[DBOpenActivity::ACT_DATA_RESET_FLAG].numberInt();

		IntMap& stage_map = player->role_detail().reward_stage_map_;
		BSONObjIterator stage_iter = res.getObjectField(DBOpenActivity::CORNUCOPIA_STAGE.c_str());
		for(int i = 0; stage_iter.more(); ++i)
		{
			BSONObj obj = stage_iter.next().embeddedObject();
			int stage_id = obj[DBOpenActivity::CornucopiaStage::STAGEID].numberInt();
			stage_map[stage_id] = obj[DBOpenActivity::CornucopiaStage::FLAG].numberInt();
			MSG_USER("mmo open load stage_id:%d, value:%d", stage_id, stage_map[stage_id]);
		}

		CornucopiaTaskMap& task_map = player->role_detail().cornucopia_task_map_;
		BSONObjIterator task_iter = res.getObjectField(DBOpenActivity::CORNUCOPIA_TASK.c_str());
		for(int i = 0; task_iter.more(); ++i)
		{
			BSONObj obj = task_iter.next().embeddedObject();
			int task_id = obj[DBOpenActivity::CornucopiaTask::TASKID].numberInt();
			task_map[task_id].task_id_ = task_id;
			task_map[task_id].completion_times_ = obj[DBOpenActivity::CornucopiaTask::COMPLETION_TIMES].numberInt();
			task_map[task_id].total_times = obj[DBOpenActivity::CornucopiaTask::TOTAL_TIMES].numberInt();
		}
	}

	CumulativeLoginDetail& cur_day_detail = player->role_detail().cur_day_detail_;
	cur_day_detail.__single = res[DBOpenActivity::ReLogin::SINGLE].numberInt();
	cur_day_detail.__ten = res[DBOpenActivity::ReLogin::TEN].numberInt();
	cur_day_detail.__hundred = res[DBOpenActivity::ReLogin::HUNDRED].numberInt();
	cur_day_detail.__multiple = res[DBOpenActivity::ReLogin::MULTIPLE].numberInt();
	cur_day_detail.__single_state = res[DBOpenActivity::ReLogin::SINGLE_STATE].numberInt();
	cur_day_detail.__ten_state = res[DBOpenActivity::ReLogin::TEN_STATE].numberInt();
	cur_day_detail.__hundred_state = res[DBOpenActivity::ReLogin::HUNDRED_STATE].numberInt();
	cur_day_detail.__multiple_state = res[DBOpenActivity::ReLogin::MULTIPLE_STATE].numberInt();
	cur_day_detail.__cumulative_day = res[DBOpenActivity::ReLogin::CUMULATIVE_DAY].numberInt();

	draw_detail.combine_tick_ = res[DBOpenActivity::COMBINE_TICK].numberLong();
	GameCommon::bson_to_map(draw_detail.accu_value_map_, res.getObjectField(DBOpenActivity::ACCU_MAP.c_str()));

	return 0;
}

int MMOOpenActivity::update_data(LogicPlayer *player, MongoDataMap *mongo_data)
{
	MLActivityerDetial& draw_detail = player->fetch_act_detail();

	BSONVec draw_bson_vec;
	for (MLActivityerDetial::ActTypeItemMap::iterator
			iter = draw_detail.act_type_item_set_.begin();
			iter != draw_detail.act_type_item_set_.end(); ++iter)
	{
		BSONVec bson_vec;
		for (MLActivityerDetial::ActItemMap::iterator sec_iter = iter->second.act_item_map_.begin();
				sec_iter != iter->second.act_item_map_.end(); ++sec_iter)
		{
			BSONVec arrive_bson_vec;
			GameCommon::map_to_bson(arrive_bson_vec, sec_iter->second.arrive_map_);

			bson_vec.push_back(BSON(DBOpenActivity::Act::ID << sec_iter->first
					<< DBOpenActivity::Act::ARRIVE << sec_iter->second.arrive_
					<< DBOpenActivity::Act::DRAWED << sec_iter->second.drawed_
					<< DBOpenActivity::Act::ARRIVE_MAP << arrive_bson_vec));
		}

		draw_bson_vec.push_back(BSON(DBOpenActivity::Act::ID << iter->first
				<< DBOpenActivity::Act::SUB_VALUE << iter->second.sub_value_
				<< DBOpenActivity::Act::UPDATE_TICK << iter->second.update_tick_
				<< DBOpenActivity::Act::SECOND_SUB << iter->second.second_sub_
				<< DBOpenActivity::Act::START_TICK << iter->second.star_tick_
				<< DBOpenActivity::Act::STOP_TICK << iter->second.stop_tick_
				<< DBOpenActivity::Act::DRAWED_SET << bson_vec));
	}
	BSONVec task_info_vec;
	CornucopiaTaskMap& task_map = player->role_detail().cornucopia_task_map_;
	CornucopiaTaskMap::iterator task_iter = task_map.begin();
	for( ; task_iter != task_map.end(); ++task_iter)
	{
		BSONObjBuilder cornucopia_builder;
		CornucopiaTask &task_info = (*task_iter).second;

		task_info_vec.push_back(
				BSON(DBOpenActivity::CornucopiaTask::TASKID << task_info.task_id_
				<< DBOpenActivity::CornucopiaTask::COMPLETION_TIMES << task_info.completion_times_
				<< DBOpenActivity::CornucopiaTask::TOTAL_TIMES << task_info.total_times));
	}

	BSONVec stage_info_vec;
	IntMap& stage_map = player->role_detail().reward_stage_map_;
	IntMap::iterator stage_iter = stage_map.begin();
	for (; stage_iter != stage_map.end(); ++stage_iter)
	{
		stage_info_vec.push_back(
				BSON(DBOpenActivity::CornucopiaStage::STAGEID << stage_iter->first
						<< DBOpenActivity::CornucopiaStage::FLAG << stage_iter->second));
		MSG_USER("mmo open update stage_id:%d, value:%d", stage_iter->first, stage_iter->second);
	}

	BSONVec accu_bson_vec;
	GameCommon::map_to_bson(accu_bson_vec, draw_detail.accu_value_map_);

	CumulativeLoginDetail cur_day_detail = player->role_detail().cur_day_detail_;
    	BSONObjBuilder builder;
    	builder << DBOpenActivity::OPEN_ACT << draw_bson_vec
    		<< DBOpenActivity::ACCU_MAP << accu_bson_vec
    		<< DBOpenActivity::COMBINE_TICK << draw_detail.combine_tick_
    		<< DBOpenActivity::ReLogin::SINGLE << cur_day_detail.__single
    		<< DBOpenActivity::ReLogin::TEN << cur_day_detail.__ten
    		<< DBOpenActivity::ReLogin::HUNDRED << cur_day_detail.__hundred
    		<< DBOpenActivity::ReLogin::MULTIPLE << cur_day_detail.__multiple
    		<< DBOpenActivity::ReLogin::SINGLE_STATE << cur_day_detail.__single_state
    		<< DBOpenActivity::ReLogin::TEN_STATE << cur_day_detail.__ten_state
    		<< DBOpenActivity::ReLogin::HUNDRED_STATE << cur_day_detail.__hundred_state
    		<< DBOpenActivity::ReLogin::MULTIPLE_STATE << cur_day_detail.__multiple_state
    		<< DBOpenActivity::ReLogin::CUMULATIVE_DAY << cur_day_detail.__cumulative_day
    		<< DBOpenActivity::CORNUCOPIA_TASK << task_info_vec
    		<< DBOpenActivity::CORNUCOPIA_STAGE << stage_info_vec
    		<< DBOpenActivity::RED_PACKET_GROUP << player->role_detail().cur_red_packet_group_
    		<< DBOpenActivity::ACT_DATA_RESET_FLAG << player->role_detail().act_data_reset_flag_
    		;

    	mongo_data->push_update(DBOpenActivity::COLLECTION,
    		BSON(DBOpenActivity::ID << player->role_id()), builder.obj());


    	return 0;
}

void MMOOpenActivity::load_open_activity_sys(OpenActivitySys* sys)
{
	std::auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBBackActivity::COLLECTION);

	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		int act_index = res[DBBackActivity::ACT_INDEX].numberInt();
		JUDGE_CONTINUE(act_index > 0);

		BackSetActDetail::ActTypeItem* s_act_t_item = sys->find_item(act_index);
		JUDGE_CONTINUE(s_act_t_item != NULL && s_act_t_item->is_need_save() == true);

		s_act_t_item->special_notify_ = res[DBBackActivity::SPECIAL_NOTIFY].numberInt();

		DBCommon::bson_to_threeobj_map(s_act_t_item->t_sub_map_,
				res.getObjectField(DBBackActivity::T_SUB_MAP.c_str()));

		DBCommon::bson_to_base_member(s_act_t_item->first_info_,
				res.getObjectField(DBBackActivity::F_RANK_INFO.c_str()));


		ServerRecordMap& record_map = s_act_t_item->server_record_map_;
		BSONObjIterator record_iter = res.getObjectField(DBBackActivity::CORNUCOPIA_RECHARGE.c_str());
		for(int i = 0; record_iter.more(); ++i)
		{
			BSONObj obj = record_iter.next().embeddedObject();

			int gold = obj[DBBackActivity::CornucopiaRecharge::CORNUCOPIA_GOLD].numberInt();
			JUDGE_CONTINUE(gold > 0);

			Int64 role_id = obj[DBBackActivity::CornucopiaRecharge::ROLE_ID].numberLong();
			ServerRecord& record = record_map[role_id];
			record.player_id_ = role_id;
			record.cornucopia_gold_ = gold;
			record.player_name_ = obj[DBBackActivity::CornucopiaRecharge::PLAYER_NAME].str();
			record.get_time_ = obj[DBBackActivity::CornucopiaRecharge::GET_TIME].numberInt();
			record.reward_mult_ = obj[DBBackActivity::CornucopiaRecharge::REWARD_MULT].numberInt();
		}

		uint index = 0;
		BSONObjIterator iter = res.getObjectField(DBBackActivity::DRAWED.c_str());
		while (iter.more())
		{
			BSONObj obj = iter.next().embeddedObject();
			JUDGE_CONTINUE(index < s_act_t_item->act_item_set_.size());

			BackSetActDetail::ActItem& act_item = s_act_t_item->act_item_set_[index];
			DBCommon::bson_to_long_map(act_item.sub_map_,
					obj.getObjectField(DBBackActivity::Reward::SUB_MAP.c_str()));
			DBCommon::bson_to_long_map(act_item.drawed_map_,
					obj.getObjectField(DBBackActivity::Reward::DRAWED_MAP.c_str()));

			++index;
		}
	}
}

void MMOOpenActivity::save_open_activity_sys(OpenActivitySys* sys)
{
	IntMap index_map;
	sys->fetch_all_index(index_map);

	for (IntMap::iterator i_iter = index_map.begin(); i_iter != index_map.end(); ++i_iter)
	{
		BackSetActDetail::ActTypeItem* s_act_t_item = sys->find_item(i_iter->first);
		JUDGE_CONTINUE(s_act_t_item != NULL && s_act_t_item->is_need_save() == true);

		BSONVec r_sub_vec;
		for (BackSetActDetail::ActItemSet::iterator s_iter = s_act_t_item->act_item_set_.begin();
				s_iter != s_act_t_item->act_item_set_.end(); ++s_iter)
		{
			BSONVec sub_vec;
			BSONVec drawed_vec;

			DBCommon::long_map_to_bson(sub_vec, s_iter->sub_map_);
			DBCommon::long_map_to_bson(drawed_vec, s_iter->drawed_map_);

			r_sub_vec.push_back(BSON(DBBackActivity::Reward::SUB_MAP << sub_vec
					<< DBBackActivity::Reward::DRAWED_MAP << drawed_vec));
		}

		BSONVec t_sub_vec;
		DBCommon::threeobj_map_to_bson(t_sub_vec, s_act_t_item->t_sub_map_);

		BSONObj first_info = DBCommon::base_member_to_bson(s_act_t_item->first_info_);

		BSONVec record_info_vec;
		ServerRecordMap& server_record_map = s_act_t_item->server_record_map_;
		ServerRecordMap::iterator record_iter = server_record_map.begin();
		for( ; record_iter != server_record_map.end(); ++record_iter)
		{
			ServerRecord& record = record_iter->second;
			JUDGE_CONTINUE(record.cornucopia_gold_ > 0);
			record_info_vec.push_back(
					BSON(DBBackActivity::CornucopiaRecharge::ROLE_ID << record.player_id_
							<<DBBackActivity::CornucopiaRecharge::PLAYER_NAME << record.player_name_
							<<DBBackActivity::CornucopiaRecharge::GET_TIME << record.get_time_
							<<DBBackActivity::CornucopiaRecharge::CORNUCOPIA_GOLD << record.cornucopia_gold_
							<<DBBackActivity::CornucopiaRecharge::REWARD_MULT << record.reward_mult_));
		}

		BSONObjBuilder builder;
		builder << DBBackActivity::T_SUB_MAP << t_sub_vec
				<< DBBackActivity::DRAWED << r_sub_vec
				<< DBBackActivity::F_RANK_INFO << first_info
				<< DBBackActivity::CORNUCOPIA_RECHARGE << record_info_vec
				<< DBBackActivity::SPECIAL_NOTIFY << s_act_t_item->special_notify_;


	    GameCommon::request_save_mmo_begin(DBBackActivity::COLLECTION,
	    		BSON(DBBackActivity::ACT_INDEX << s_act_t_item->act_index_),
	    		BSON("$set" << builder.obj()));
	}
}

void MMOOpenActivity::load_fetst_act_time()
{
	DBShopMode shop_mode;
	CACHED_INSTANCE->mmo_open_activity_->load_fest_act_time(&shop_mode);
	FEST_ACTIVITY_SYS->request_festival_time_done(&shop_mode);
}

void MMOOpenActivity::load_fest_act_time(DBShopMode* shop_mode)
{
	BSONObj res = this->conection().findOne(DBFestActivity::COLLECTION,
			QUERY(DBFestActivity::ID << 0));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	Int64 update_tick = res[DBFestActivity::UPDATE_TICK].numberLong();
	JUDGE_RETURN(shop_mode->input_argv_.type_int64_ != update_tick, ;);

	shop_mode->sub_value_ = 1;
	shop_mode->output_argv_.type_int_ = res[DBFestActivity::ICON_TYPE].numberLong();
	shop_mode->output_argv_.long_vec_.push_back(res[DBFestActivity::STATCK_TICK].numberLong());
	shop_mode->output_argv_.long_vec_.push_back(res[DBFestActivity::END_TICK].numberLong());
	shop_mode->output_argv_.long_vec_.push_back(update_tick);
}

void MMOOpenActivity::save_may_activity_sys(MayActivitySys* sys)
{
	int has_act = false;

	IntMap &act_map = sys->fetch_act_list();
	for (IntMap::iterator act_iter = act_map.begin(); act_iter != act_map.end(); ++act_iter)
	{
		MayActDetail::ActInfo* act_info = sys->find_act_info(act_iter->first);
		JUDGE_CONTINUE(act_info != NULL);

		BSONVec reward_vec;
		for (MayActDetail::ActRewardSet::iterator reward_iter = act_info->reward_set_.begin();
				reward_iter != act_info->reward_set_.end(); ++reward_iter)
		{
			BSONVec sub_vec;
			BSONVec drawed_vec;
			BSONVec act_draw_vec;

			DBCommon::long_map_to_bson(sub_vec, reward_iter->sub_map_);
			DBCommon::long_map_to_bson(drawed_vec, reward_iter->drawed_map_);
			DBCommon::long_map_to_bson(act_draw_vec, reward_iter->act_drawed_map_);

			reward_vec.push_back(BSON(DBMayActivity::RewardSet::SUB_MAP << sub_vec
					<< DBMayActivity::RewardSet::DRAW_MAP << drawed_vec
					<< DBMayActivity::RewardSet::ACT_DRAW_MAP << act_draw_vec
					));
		}

		BSONVec couple_vec;
		for (MayActDetail::CoupleInfoMap::iterator couple_iter = act_info->couple_info_map_.begin();
				couple_iter != act_info->couple_info_map_.end(); ++couple_iter)
		{
			MayActDetail::CoupleInfo &couple_info = couple_iter->second;

			couple_vec.push_back(BSON(DBMayActivity::CoupleMap::WEDDING_ID << couple_info.wedding_id_
					<< DBMayActivity::CoupleMap::ONLINE_TICK << couple_info.online_tick_
					<< DBMayActivity::CoupleMap::BUY_TICK << couple_info.buy_tick_));
		}

		BSONVec run_vec;
		for (MayActDetail::RunRoleMap::iterator run_iter = act_info->run_role_map_.begin();
				run_iter != act_info->run_role_map_.end(); ++run_iter)
		{
			MayActDetail::RunInfo &player_run = run_iter->second;

			run_vec.push_back(BSON(DBMayActivity::RunInfo::ROLE_ID << player_run.role_id_
					<< DBMayActivity::RunInfo::NAME << player_run.name_
					<< DBMayActivity::RunInfo::SEX << player_run.sex_
					<< DBMayActivity::RunInfo::VALUE << player_run.location_
					<< DBMayActivity::RunInfo::TICK << player_run.tick_));
		}

//		BSONVec player_red_packet_info;
//		MayActDetail::GroupMap& reward_info = act_info->player_reward_info_;
//		for(MayActDetail::GroupMap::iterator group_iter = reward_info.begin();
//				group_iter != reward_info.end(); ++group_iter)
//		{
//			MayActDetail::LUMap record_map = group_iter->second;
//			BSONVec record_vec;
//			for(MayActDetail::LUMap::iterator record_iter = record_map.begin();
//					record_iter != record_map.end(); ++record_iter)
//			{
//				record_vec.push_back(BSON(
//						record_map->first, record->second));
//			}
//
//		}

		BSONObjBuilder builder;
		builder << DBMayActivity::ACT_TYPE << act_info->act_type_
				<< DBMayActivity::REWARD_SET << reward_vec
				<< DBMayActivity::COUPLE_MAP << couple_vec
				<< DBMayActivity::RUN_MAP << run_vec;

		GameCommon::request_save_mmo_begin(DBMayActivity::COLLECTION,
			    BSON(DBMayActivity::ID << act_info->act_index_),
			    BSON("$set" << builder.obj()));

		has_act = true;
	}
	JUDGE_RETURN(has_act == false, ;);

	BSONObj empty_obj;
	GameCommon::request_remove_mmo_begin(DBMayActivity::COLLECTION, empty_obj, false);
}

void MMOOpenActivity::load_may_activity_sys(MayActivitySys* sys)
{
	std::auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBMayActivity::COLLECTION);

	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		int act_index = res[DBMayActivity::ID].numberInt();
		JUDGE_CONTINUE(act_index > 0);

		MayActDetail::ActInfo* act_info = sys->find_act_info(act_index);
		JUDGE_CONTINUE(act_info != NULL);

		act_info->act_type_ = res[DBMayActivity::ACT_TYPE].numberInt();

		uint index = 0;
		BSONObjIterator reward_iter = res.getObjectField(DBMayActivity::REWARD_SET.c_str());
		while (reward_iter.more())
		{
			BSONObj obj = reward_iter.next().embeddedObject();
			JUDGE_CONTINUE(index < act_info->reward_set_.size());

			MayActDetail::ActReward& act_reward = act_info->reward_set_[index];
			DBCommon::bson_to_long_map(act_reward.sub_map_,
					obj.getObjectField(DBMayActivity::RewardSet::SUB_MAP.c_str()));
			DBCommon::bson_to_long_map(act_reward.drawed_map_,
					obj.getObjectField(DBMayActivity::RewardSet::DRAW_MAP.c_str()));
			DBCommon::bson_to_long_map(act_reward.act_drawed_map_,
					obj.getObjectField(DBMayActivity::RewardSet::ACT_DRAW_MAP.c_str()));
			++index;
		}

		BSONObjIterator couple_iter = res.getObjectField(DBMayActivity::COUPLE_MAP.c_str());
		while (couple_iter.more())
		{
			BSONObj obj = couple_iter.next().embeddedObject();

			Int64 wedding_id = obj[DBMayActivity::CoupleMap::WEDDING_ID].numberLong();
			MayActDetail::CoupleInfo &couple_info = act_info->couple_info_map_[wedding_id];
			couple_info.wedding_id_ = wedding_id;
			couple_info.online_tick_ = obj[DBMayActivity::CoupleMap::ONLINE_TICK].numberInt();
			couple_info.buy_tick_ = obj[DBMayActivity::CoupleMap::BUY_TICK].numberInt();
		}

		BSONObjIterator run_iter = res.getObjectField(DBMayActivity::RUN_MAP.c_str());
		while (run_iter.more())
		{
			BSONObj obj = run_iter.next().embeddedObject();

			Int64 role_id = obj[DBMayActivity::RunInfo::ROLE_ID].numberLong();
			MayActDetail::RunInfo &player_run = act_info->run_role_map_[role_id];
			player_run.role_id_ = role_id;
			player_run.name_ 	= obj[DBMayActivity::RunInfo::NAME].str();
			player_run.sex_ 	= obj[DBMayActivity::RunInfo::SEX].numberInt();
			player_run.location_= obj[DBMayActivity::RunInfo::VALUE].numberInt();
			player_run.tick_ 	= obj[DBMayActivity::RunInfo::TICK].numberLong();
		}
	}
}

int MMOOpenActivity::load_may_activityer(LogicPlayer *player)
{
	BSONObj res = this->conection().findOne(DBMayActivityer::COLLECTION,
			QUERY(DBMayActivityer::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	MayActivityer& may_detial = player->may_detail();
	may_detial.act_type_ = res[DBMayActivityer::ACT_TYPE].numberInt();
	player->role_detail().cumulative_times_ = res[DBMayActivityer::Act::CUMULATIVE_TIMES].numberInt();

	BSONObjIterator iter(res.getObjectField(DBMayActivityer::MAY_ACT.c_str()));
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();

		int act_id = obj[DBMayActivityer::Act::ID].numberInt();
		MayActivityer::ActTypeItem& act_t_item = may_detial.act_type_item_set_[act_id];

		act_t_item.sub_value_ = obj[DBMayActivityer::Act::SUB_VALUE].numberLong();
		act_t_item.second_sub_ = obj[DBMayActivityer::Act::SECOND_SUB].numberInt();

		DBCommon::bson_to_long_map(act_t_item.send_map_,
				obj.getObjectField(DBMayActivityer::Act::SEND_MAP.c_str()));

		DBCommon::bson_to_long_map(act_t_item.role_map_,
				obj.getObjectField(DBMayActivityer::Act::ROLE_MAP.c_str()));

		BSONObjIterator sec_iter(obj.getObjectField(DBMayActivityer::Act::DRAWED_SET.c_str()));
		while (sec_iter.more())
		{
			BSONObj sec_obj = sec_iter.next().embeddedObject();

			int aid = sec_obj[DBMayActivityer::Act::ID].numberInt();
			MayActivityer::ActItem& act_item = act_t_item.act_item_map_[aid];

			act_item.arrive_ = sec_obj[DBMayActivityer::Act::ARRIVE].numberInt();
			act_item.drawed_ = sec_obj[DBMayActivityer::Act::DRAWED].numberInt();


			GameCommon::bson_to_map(act_item.arrive_map_,
					sec_obj.getObjectField(DBMayActivityer::Act::ARRIVE_MAP.c_str()));
		}

		//刷新次数数据
		act_t_item.refresh_tick_ = obj[DBMayActivityer::Act::REFRESH_TICK].numberLong();
		IntVec refresh_times_vec;
		DBCommon::bson_to_int_vec(refresh_times_vec,
				obj.getObjectField(DBMayActivityer::Act::REFRESH_TIMES_MAP.c_str()));
		for (int i = 0; i < (int)refresh_times_vec.size(); ++i)
		{
			act_t_item.group_refresh_times_map_[i] = refresh_times_vec[i] - 100;
		}


		int len = 0;
		BSONObjIterator it_shop_slot(obj.getObjectField(DBMayActivityer::SHOP_SLOT_MAP.c_str()));
		while (it_shop_slot.more())
		{
			BSONObj shop_slot_obj = it_shop_slot.next().embeddedObject();
			JUDGE_CONTINUE(shop_slot_obj.isEmpty() == false);

			MayActivityer::ShopSlot &shop_slot = act_t_item.shop_slot_map_[len];
			MayActDetail::SlotInfo &slot_info = shop_slot.slot_info_;
			shop_slot.is_buy_ = shop_slot_obj[DBMayActivityer::ShopSlot::IS_BUY].numberInt();
			slot_info.slot_id_ = shop_slot_obj[DBMayActivityer::ShopSlot::SLOT_ID].numberInt();
			slot_info.group_id_ = shop_slot_obj[DBMayActivityer::ShopSlot::GROUP_ID].numberInt();

			int item_id = shop_slot_obj[DBMayActivityer::ShopSlot::ITEM_ID].numberInt();
			int item_amount = shop_slot_obj[DBMayActivityer::ShopSlot::ITEM_AMOUNT].numberInt();
			int item_bind = shop_slot_obj[DBMayActivityer::ShopSlot::ITEM_BIND].numberInt();

			slot_info.item_obj_ = ItemObj(item_id, item_amount, item_bind);
			len ++;
		}

		BSONObjIterator fashion_builder(obj.getObjectField(DBMayActivityer::FASHION_ACT_INFO.c_str()));
		while(fashion_builder.more())
		{
			BSONObj fashion_act_info = fashion_builder.next().embeddedObject();
			JUDGE_CONTINUE(fashion_act_info.isEmpty() == false);

			act_t_item.liveness_ = fashion_act_info[DBMayActivityer::FashionActInfo::LIVENESS].numberInt();
			act_t_item.cur_fashion_times_ = fashion_act_info[DBMayActivityer::FashionActInfo::CUR_FASHION_TIMES].numberInt();
			BSONObjIterator liveness_iter(fashion_act_info.getObjectField(DBMayActivityer::FashionActInfo::FASHION_LIVENESS_MAP.c_str()));
			while(liveness_iter.more())
			{
				BSONObj liveness_vec = liveness_iter.next().embeddedObject();
				JUDGE_CONTINUE(liveness_vec.isEmpty() == false);
				std::map<int, IntMap> &liveness_map = act_t_item.liveness_map_;
				int times = liveness_vec[DBMayActivityer::FashionLivenessMap::TIMES].numberInt();
				IntMap &reward_map = liveness_map[times];

				BSONObjIterator reward_iter(liveness_vec.getObjectField(DBMayActivityer::FashionLivenessMap::LIVENESS_REWARD_MAP.c_str()));
				while(reward_iter.more())
				{
					BSONObj reward_vec = reward_iter.next().embeddedObject();
					JUDGE_CONTINUE(reward_vec.isEmpty() == false);
					int slot_id = reward_vec[DBMayActivityer::LivenessRewardMap::SLOT_ID].number();
					reward_map[slot_id] = reward_vec[DBMayActivityer::LivenessRewardMap::STATE].number();
				}
			}
		}

		CornucopiaTaskMap& task_map = act_t_item.labour_task_map_;
		BSONObjIterator task_iter = obj.getObjectField(DBMayActivityer::CORNUCOPIA_TASK.c_str());
		for(int i = 0; task_iter.more(); ++i)
		{
			BSONObj obj = task_iter.next().embeddedObject();
			int task_id = obj[DBMayActivityer::CornucopiaTask::TASKID].numberInt();
			task_map[task_id].task_id_ = task_id;
			task_map[task_id].completion_times_ = obj[DBMayActivityer::CornucopiaTask::COMPLETION_TIMES].numberInt();
			task_map[task_id].total_times = obj[DBMayActivityer::CornucopiaTask::TOTAL_TIMES].numberInt();
			//task_map[task_id].task_name_ = obj[DBMayActivityer::CornucopiaTask::TASK_NAME].str();
		}
	}

	return 0;
}

int MMOOpenActivity::save_may_activity(LogicPlayer *player, MongoDataMap *mongo_data)
{
	MayActivityer& may_detial = player->may_detail();

	BSONVec draw_bson_vec;
	for (MayActivityer::ActTypeItemMap::iterator iter = may_detial.act_type_item_set_.begin();
			iter != may_detial.act_type_item_set_.end(); ++iter)
	{
		BSONVec bson_vec, send_vec, role_vec;
		for (MayActivityer::ActItemMap::iterator sec_iter = iter->second.act_item_map_.begin();
				sec_iter != iter->second.act_item_map_.end(); ++sec_iter)
		{
			BSONVec arrive_bson_vec;
			GameCommon::map_to_bson(arrive_bson_vec, sec_iter->second.arrive_map_);

			bson_vec.push_back(BSON(DBMayActivityer::Act::ID << sec_iter->first
					<< DBMayActivityer::Act::ARRIVE << sec_iter->second.arrive_
					<< DBMayActivityer::Act::DRAWED << sec_iter->second.drawed_
					<< DBMayActivityer::Act::ARRIVE_MAP << arrive_bson_vec));
		}

		DBCommon::long_map_to_bson(send_vec, iter->second.send_map_);
		DBCommon::long_map_to_bson(role_vec, iter->second.role_map_);

		BSONVec shop_vec;
		for (MayActivityer::ShopSlotMap::iterator shop_iter = iter->second.shop_slot_map_.begin();
				shop_iter != iter->second.shop_slot_map_.end(); ++shop_iter)
		{
			MayActivityer::ShopSlot &shop_slot = shop_iter->second;
			MayActDetail::SlotInfo &slot_info = shop_slot.slot_info_;
			shop_vec.push_back(BSON(DBMayActivityer::ShopSlot::IS_BUY << shop_slot.is_buy_
					<< DBMayActivityer::ShopSlot::SLOT_ID << slot_info.slot_id_
					<< DBMayActivityer::ShopSlot::GROUP_ID << slot_info.group_id_
					<< DBMayActivityer::ShopSlot::ITEM_ID << slot_info.item_obj_.id_
					<< DBMayActivityer::ShopSlot::ITEM_AMOUNT << slot_info.item_obj_.amount_
					<< DBMayActivityer::ShopSlot::ITEM_BIND << slot_info.item_obj_.bind_));
		}

		BSONVec Liveness_vec;
		std::map<int, IntMap> &liveness_map = iter->second.liveness_map_;
		for(std::map<int, IntMap>::iterator liveness_iter = liveness_map.begin(); liveness_iter != liveness_map.end(); ++liveness_iter)
		{
			BSONVec reward_vec;
			IntMap &reward_info = liveness_iter->second;
			for(IntMap::iterator reward_iter = reward_info.begin(); reward_iter != reward_info.end(); ++reward_iter)
			{
				reward_vec.push_back(BSON(DBMayActivityer::LivenessRewardMap::SLOT_ID << reward_iter->first
						<< DBMayActivityer::LivenessRewardMap::STATE <<reward_iter->second));
			}
			Liveness_vec.push_back(BSON(DBMayActivityer::FashionLivenessMap::TIMES << liveness_iter->first
					<< DBMayActivityer::FashionLivenessMap::LIVENESS_REWARD_MAP << reward_vec));
		}

		BSONVec fashion_vec;
		fashion_vec.push_back(BSON(DBMayActivityer::FashionActInfo::LIVENESS << iter->second.liveness_
				<< DBMayActivityer::FashionActInfo::CUR_FASHION_TIMES << iter->second.cur_fashion_times_
				<< DBMayActivityer::FashionActInfo::FASHION_LIVENESS_MAP << Liveness_vec));

		BSONVec task_info_vec;
		CornucopiaTaskMap& task_map = iter->second.labour_task_map_;
		CornucopiaTaskMap::iterator task_iter = task_map.begin();
		for( ; task_iter != task_map.end(); ++task_iter)
		{
			BSONObjBuilder cornucopia_builder;
			CornucopiaTask &task_info = (*task_iter).second;

			task_info_vec.push_back(
					BSON(DBMayActivityer::CornucopiaTask::TASKID << task_info.task_id_
					<< DBMayActivityer::CornucopiaTask::COMPLETION_TIMES << task_info.completion_times_
					<< DBMayActivityer::CornucopiaTask::TOTAL_TIMES << task_info.total_times
					//<< DBMayActivityer::CornucopiaTask::TASK_NAME << task_info.task_name_
					));
		}

		IntVec temp_1;
		for (int i = 0; i < (int)iter->second.group_refresh_times_map_.size(); ++i)
		{
			temp_1.push_back(iter->second.group_refresh_times_map_[i] + 100);
		}

		draw_bson_vec.push_back(BSON(DBMayActivityer::Act::ID << iter->first
				<< DBMayActivityer::Act::SUB_VALUE << iter->second.sub_value_
				<< DBMayActivityer::Act::SECOND_SUB << iter->second.second_sub_
				<< DBMayActivityer::Act::DRAWED_SET << bson_vec
				<< DBMayActivityer::Act::SEND_MAP << send_vec
				<< DBMayActivityer::Act::ROLE_MAP << role_vec
				<< DBMayActivityer::SHOP_SLOT_MAP << shop_vec
				<< DBMayActivityer::FASHION_ACT_INFO << fashion_vec
				<< DBMayActivityer::CORNUCOPIA_TASK << task_info_vec
				<< DBMayActivityer::Act::REFRESH_TIMES_MAP << temp_1
				));
	}

	BSONObjBuilder builder;
	builder << DBMayActivityer::MAY_ACT << draw_bson_vec
			<< DBMayActivityer::ACT_TYPE << may_detial.act_type_
			<< DBMayActivityer::Act::CUMULATIVE_TIMES << player->role_detail().cumulative_times_;

	mongo_data->push_update(DBMayActivityer::COLLECTION,
	    	BSON(DBMayActivityer::ID << player->role_id()), builder.obj());

	return 0;
}

int MMOOpenActivity::load_back_may_activity(DBShopMode* shop_mode)
{
	BSONObj res = this->conection().findOne(DBBackMayActivity::COLLECTION,
			QUERY(DBBackMayActivity::ID << 0));
	JUDGE_RETURN(res.isEmpty() == false, 0);

	Int64 refresh_tick = res[DBBackMayActivity::REFRESH_TICK].numberLong();
	JUDGE_RETURN(refresh_tick != 0, 0);
	JUDGE_RETURN(shop_mode->input_argv_.type_int64_ != refresh_tick, 0);

	*shop_mode->output_argv_.bson_obj_ = res.copy();
	return 0;
}

void MMOOpenActivity::ensure_all_index(void)
{
	this->conection().ensureIndex(DBOpenActivity::COLLECTION,
			BSON(DBOpenActivity::ID << 1), true);
	this->conection().ensureIndex(DBFestActivity::COLLECTION,
			BSON(DBFestActivity::ID << 1), true);

	this->conection().ensureIndex(DBBackActivity::COLLECTION,
			BSON(DBBackActivity::ACT_INDEX << 1), true);
	this->conection().ensureIndex(DBBackActivity::COLLECTION,
			BSON(DBBackActivity::OPEN_FLAG << 1), false);

	this->conection().ensureIndex(DBMayActivity::COLLECTION,
			BSON(DBMayActivity::ID << 1), true);
	this->conection().ensureIndex(DBMayActivityer::COLLECTION,
			BSON(DBMayActivityer::ID << 1), true);
	this->conection().ensureIndex(DBBackMayActivity::COLLECTION,
			BSON(DBBackMayActivity::ID << 1), true);

}

