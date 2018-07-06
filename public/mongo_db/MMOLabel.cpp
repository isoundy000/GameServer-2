/*
 * MMOLabel.cpp
 *
 *  Created on: 2013-12-3
 *      Author: louis
 */

#include "GameField.h"
#include "MMOLabel.h"
#include "PubStruct.h"
#include "PoolMonitor.h"
#include "GameCommon.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include "MapLogicPlayer.h"
#include <mongo/client/dbclient.h>
#include "MapMonitor.h"
#include "DBCommon.h"
#include "MapTaskStruct.h"
#include "MongoConnector.h"

MMOLabel::MMOLabel() {
	// TODO Auto-generated constructor stub

}

MMOLabel::~MMOLabel() {
	// TODO Auto-generated destructor stub
}

int MMOLabel::load_player_label(MapLogicPlayer *player)
{
	BSONObj res = this->conection().findOne(LabelInfo::COLLECTION,
			QUERY(LabelInfo::ID << player->role_id()));
	if (res.isEmpty())
		return 0;

	IntSet& permant_list = player->permant_label_list();
	IntSet& new_list = player->new_list();
	IntSet& expire_list = player->expire_unshown_list();
	std::map<int, Int64>& limit_list = player->limit_time_label_list();

	player->set_cur_label_id(res[LabelInfo::LABEL_ID].numberInt());
	player->set_pre_label_id(res[LabelInfo::PRE_LABEL_ID].numberInt());

	BSONObj list = res.getObjectField(LabelInfo::PERMANT_LIST.c_str());
	BSONObjIterator it(list);

	DBCommon::bson_to_int_set(permant_list, res.getObjectField(LabelInfo::PERMANT_LIST.c_str()), true);
	DBCommon::bson_to_int_set(new_list, res.getObjectField(LabelInfo::NEW_LIST.c_str()), true);
	DBCommon::bson_to_int_set(expire_list, res.getObjectField(LabelInfo::EXPIRE_UNSHOWN_LIST.c_str()), true);

	if(res.hasField(LabelInfo::LIMIT_TIME_LIST.c_str()))
	{
		BSONObj list = res.getObjectField(LabelInfo::LIMIT_TIME_LIST.c_str());
		BSONObjIterator it(list);
		while(it.more())
		{
			BSONObj obj = it.next().embeddedObject();
			int lable_id = obj[LabelInfo::LABEL_ID].numberInt();
			Int64 expired_tick = obj[LabelInfo::EXPIRE_TICK].numberLong();

			if(GameCommon::left_time(expired_tick) > 0)
				limit_list[lable_id] = expired_tick;
			else
				player->modify_expire_unshown_list(GameEnum::LABEL_DELETE, lable_id);
		}
	}

	int martial_label = res[LabelInfo::MATRIAL_LABEL].numberInt();
	if (martial_label > 0)
	{
		player->insert_label_i(martial_label, res[LabelInfo::MARTIAL_LABEL_TICK].numberLong());
		MMOLabel::save_player_label(player->role_id(), 0);
	}

	int war_label = res[LabelInfo::WAR_LABEL].numberInt();
	if (war_label > 0)
	{
		player->insert_label_i(war_label, res[LabelInfo::WAR_LABEL_TICK].numberLong());
		MMOLabel::save_player_label(player->role_id(), 0, true);
	}

	if(res[LabelInfo::LABEL_ID].numberInt() != 0 )
	{
		int limit_type = GameCommon::fetch_label_type(res[LabelInfo::LABEL_ID].numberInt());
		int label_id = res[LabelInfo::LABEL_ID].numberInt();
		if((limit_type == 1 &&limit_list.count(label_id) == 0) || (limit_type == 0 &&permant_list.count(label_id) == 0))
			player->set_cur_label_id(0);
	}
	return 0;
}

int MMOLabel::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	BSONVec limit_set;

	IntSet& permant_list = player->permant_label_list();
	IntSet& new_list = player->new_list();
	IntSet& expire_list = player->expire_unshown_list();
	std::map<int, Int64>& limit_list = player->limit_time_label_list();

	for(std::map<int, Int64>::iterator it = limit_list.begin(); it != limit_list.end(); ++it)
	{
		limit_set.push_back(BSON(LabelInfo::LABEL_ID << it->first
				<< LabelInfo::EXPIRE_TICK << it->second));
	}

	BSONObjBuilder build;
	build << LabelInfo::LABEL_ID << player->cur_label_id()
			<< LabelInfo::PRE_LABEL_ID << player->pre_label_id()
			<< LabelInfo::PERMANT_LIST << permant_list
			<< LabelInfo::NEW_LIST << new_list
			<< LabelInfo::LIMIT_TIME_LIST << limit_set
			<< LabelInfo::EXPIRE_UNSHOWN_LIST << expire_list;
	mongo_data->push_update(LabelInfo::COLLECTION, BSON(LabelInfo::ID << player->role_id()),
			build.obj(), true);
	return 0;
}

void MMOLabel::save_player_label(long role_id, int label_id, int source)
{
	BSONObjBuilder builder;

	switch(source)
	{
	case 0:
	{
		builder << LabelInfo::MATRIAL_LABEL << label_id
				<< LabelInfo::MARTIAL_LABEL_TICK << (Int64)(::time(NULL));
		break;
	}

	case 1:
	{
		builder << LabelInfo::WAR_LABEL << label_id
				<< LabelInfo::WAR_LABEL_TICK << (Int64)(::time(NULL));
		break;
	}

	default:
	{
		return;
	}
	}

	GameCommon::request_save_mmo_begin(LabelInfo::COLLECTION,
			BSON(LabelInfo::ID << (Int64)role_id),
			BSON("$set" << builder.obj()), false);
}

void MMOLabel::ensure_all_index(void)
{
	this->conection().ensureIndex(LabelInfo::COLLECTION, BSON(LabelInfo::ID << 1), true);
}


MMOAchievement::MMOAchievement() {
	// TODO Auto-generated constructor stub

}

MMOAchievement::~MMOAchievement() {
	// TODO Auto-generated destructor stub
}

int MMOAchievement::load_player_achievement(MapLogicPlayer *player)
{
	BSONObj res = this->conection().findOne(AchieveInfo::COLLECTION,
			QUERY(AchieveInfo::ID << player->role_id()));
	if (res.isEmpty())
		return 0;

	MLAchievement::AchieveMap& achieve_map = player->achieve_map();
	if(res.hasField(AchieveInfo::ACHIEVE_LIST.c_str()))
	{
		BSONObjIterator iter(res.getObjectField(AchieveInfo::ACHIEVE_LIST.c_str()));
		while(iter.more())
		{
			AchieveDetail* detail = POOL_MONITOR->achieve_detail_pool()->pop();
			JUDGE_CONTINUE(detail != NULL);

			DBCommon::bson_to_achieve_detail(detail, iter.next().embeddedObject());
			achieve_map.insert(MLAchievement::AchieveMap::value_type(detail->achieve_id_, detail));
		}
	}

	BaseAchieveInfo& base_achieve = player->base_achieve();
	base_achieve.achieve_level_ = res[AchieveInfo::ACHIEVE_LEVEL].numberInt();

	if(res.hasField(AchieveInfo::POINT_MAP.c_str()))
	{
		BSONObjIterator iter(res.getObjectField(AchieveInfo::POINT_MAP.c_str()));
		while(iter.more())
		{
			BSONObj obj = iter.next().embeddedObject();
			int obj_key = obj[DBPairObj::KEY].numberInt();
			int obj_value = obj[DBPairObj::VALUE].numberInt();

			JUDGE_CONTINUE(obj_key != 0);
			base_achieve.achieve_point_map_[obj_key] = obj_value;
		}
	}

//	if(res.hasField(AchieveInfo::DRAWED_LIST.c_str()))
//	{
//		BSONObj drawed = res.getObjectField(AchieveInfo::DRAWED_LIST.c_str());
//		BSONObjIterator it(drawed);
//		while(it.more())
//		{
//			AchieveDetail* detail = POOL_MONITOR->achieve_detail_pool()->pop();
//			JUDGE_CONTINUE(detail != NULL);
//
//			DBCommon::bson_to_achieve_detail(detail, it.next().embeddedObject());
//			drawed_list.insert(MLAchievement::AchieveMap::value_type(detail->__sub_id, detail));
//		}
//	}
//
//	if(res.hasField(AchieveInfo::UNCHECK_LIST.c_str()))
//	{
//		BSONObj uncheck = res.getObjectField(AchieveInfo::UNCHECK_LIST.c_str());
//		BSONObjIterator it(uncheck);
//		while(it.more())
//		{
//			BSONObj obj = it.next().embeddedObject();
//			int sub_id  = obj[AchieveInfo::SUB_TYPE].numberInt();
//
//			JUDGE_CONTINUE(achieve_list.count(sub_id) > 0);
//			uncheck_list.push_back(achieve_list[sub_id]);
//		}
//	}
//
//	if(res.hasField(AchieveInfo::ACHIEVE_PROP.c_str()))
//	{
//		BSONObj prop = res.getObjectField(AchieveInfo::ACHIEVE_PROP.c_str());
//		BSONObjIterator it(prop);
//		while(it.more())
//		{
//			BSONObj obj = it.next().embeddedObject();
//			int prop_id = obj[AchieveInfo::PROP_ID].numberInt();
//			int prop_value = obj[AchieveInfo::PROP_VALUE].numberInt();
//
//			prop_list.insert(IntMap::value_type(prop_id, prop_value));
//		}
//	}
//
//	if(res.hasField(AchieveInfo::OTHER_ACHIEVE_RECORD.c_str()))
//	{
//		BSONObj other = res.getObjectField(AchieveInfo::OTHER_ACHIEVE_RECORD.c_str());
//		BSONObjIterator it(other);
//		while(it.more())
//		{
//			BSONObj obj = it.next().embeddedObject();
//			int id = obj[AchieveInfo::TYPE].numberInt();
//			int value = obj[AchieveInfo::OTHER_CUR_VALUE].numberInt();
//
//			other_record_list.insert(IntMap::value_type(id, value));
//		}
//	}

	return 0;
}

int MMOAchievement::update_data(MapLogicPlayer *player, MongoDataMap *mongo_data)
{
	BSONVec achieve_set, point_set;
	MLAchievement::AchieveMap& achieve_map = player->achieve_map();
	BaseAchieveInfo& base_achieve = player->base_achieve();

	for(MLAchievement::AchieveMap::iterator it = achieve_map.begin(); it != achieve_map.end(); ++it)
	{
		AchieveDetail *achieve = it->second;
		JUDGE_CONTINUE(achieve != NULL);

		achieve_set.push_back(DBCommon::achieve_detail_to_bson(achieve));
	}

	for (IntMap::iterator iter = base_achieve.achieve_point_map_.begin();
			iter != base_achieve.achieve_point_map_.end(); ++iter)
	{
		point_set.push_back(BSON(DBPairObj::KEY << iter->first
				<< DBPairObj::VALUE << iter->second));
	}

	BSONObjBuilder build;
	build << AchieveInfo::ACHIEVE_LEVEL << base_achieve.achieve_level_
		  << AchieveInfo::POINT_MAP << point_set
		  << AchieveInfo::ACHIEVE_LIST << achieve_set;
	mongo_data->push_update(AchieveInfo::COLLECTION, BSON(AchieveInfo::ID << player->role_id()),
			build.obj(), true);

//	BSONVec achieve_set, drawed_set, uncheck_set, prop_set, other_set;
//	MLAchievement::AchieveMap& achieve_list = player->achieve_reward_map();
//	MLAchievement::AchieveMap& drawed_list  = player->achieve_drawed_map();
//	std::list<AchieveDetail*>& uncheck_list = player->uncheck_list();
//	IntMap& prop_list                       = player->achieve_prop_map();
//	IntMap& other_record_list               = player->other_achieve_record_map();
//
//	IntMap::iterator it_vec = other_record_list.begin();
//	for(; it_vec != other_record_list.end(); ++it_vec)
//	{
//		other_set.push_back(BSON(AchieveInfo::TYPE << it_vec->first
//				<< AchieveInfo::OTHER_CUR_VALUE << it_vec->second));
//	}
//
//	std::list<AchieveDetail*>::iterator iter = uncheck_list.begin();
//	for(; iter != uncheck_list.end(); ++iter)
//	{
//		JUDGE_CONTINUE(*iter != NULL);
//		JUDGE_CONTINUE(achieve_list.count((*iter)->__sub_id) > 0);
//
//		uncheck_set.push_back(DBCommon::achieve_detail_to_bson(*iter));
//	}
//
//	MLAchievement::AchieveMap::iterator it = achieve_list.begin();
//	for(; it != achieve_list.end(); ++it)
//		achieve_set.push_back(DBCommon::achieve_detail_to_bson(it->second));
//
//	it = drawed_list.begin();
//	for(; it != drawed_list.end(); ++it)
//		drawed_set.push_back(DBCommon::achieve_detail_to_bson(it->second));
//
//	IntMap::iterator iterator = prop_list.begin();
//	for(; iterator != prop_list.end(); ++iterator)
//		prop_set.push_back(BSON(AchieveInfo::PROP_ID << iterator->first
//				<< AchieveInfo::PROP_VALUE << iterator->second));
//
//	BSONObjBuilder build;
//	build << AchieveInfo::ACHIEVE_LIST << achieve_set
//			<< AchieveInfo::DRAWED_LIST << drawed_set
//			<< AchieveInfo::UNCHECK_LIST << uncheck_set
//			<< AchieveInfo::ACHIEVE_PROP << prop_set
//			<< AchieveInfo::OTHER_ACHIEVE_RECORD << other_set;
//	mongo_data->push_update(AchieveInfo::COLLECTION, BSON(AchieveInfo::ID << player->role_id()),
//			build.obj(), true);
	return 0;
}

int MMOAchievement::request_update_special_FB_achievement(MongoDataMap* data_map, MapLogicPlayer* player)
{
	data_map->push_update(AchieveInfo::COLLECTION,
			BSON(AchieveInfo::ID << player->role_id()), BSONObj(), true);
	return 0;
}

int MMOAchievement::update_special_FB_achievement(MongoDataMap* data_map)
{
//	MongoData *mongo_data = NULL;
//	data_map->find_data(AchieveInfo::COLLECTION, mongo_data);
//	JUDGE_RETURN(NULL != mongo_data, 0);
//
//	Int64 player_id = mongo_data->condition()[AchieveInfo::ID].numberLong();
//	std::string content = AchieveInfo::OTHER_ACHIEVE_RECORD + "." + AchieveInfo::OTHER_CUR_VALUE;
//	this->conection().update(AchieveInfo::COLLECTION, BSON(AchieveInfo::ID << player_id),
//			BSON("$inc" << BSON(content << 1)), true);
//	MSG_DEBUG(ID:%ld, player_id);
	return 0;
}

void MMOAchievement::ensure_all_index(void)
{
	this->conection().ensureIndex(AchieveInfo::COLLECTION, BSON(AchieveInfo::ID << 1), true);
}


MMOTask::MMOTask(void)
{ /*NULL*/ }

MMOTask::~MMOTask(void)
{ /*NULL*/ }

int MMOTask::load_player_task(MapLogicPlayer *player)
{
    BSONObj res = this->conection().findOne(DBTask::COLLECTION, QUERY(DBTask::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, 0);

    player->set_novice_step(res[DBTask::NOVICE_STEP].numberInt());
    player->set_latest_main_task(res[DBTask::LATEST_MAIN_TASK].numberInt());
    player->set_uiopen_step(res[DBTask::UIOPEN_STEP].numberInt());

    player->routine_total_num_[TaskInfo::TASK_GTYPE_ROUTINE] = res[DBTask::ROUTINE_TOTAL_NUM].numberInt();
    player->routine_refresh_tick_.sec(res[DBTask::ROUTINE_REFRESH_TICK].numberInt());
    player->routine_task_index_[TaskInfo::TASK_GTYPE_ROUTINE] = res[DBTask::ROUTINE_TASK_INDEX].numberInt();
    player->is_finish_all_routine_[TaskInfo::TASK_GTYPE_ROUTINE] = res[DBTask::IS_FINISH_ALL_ROUTINE].numberInt();
    player->is_routine_task_[TaskInfo::TASK_GTYPE_ROUTINE] = res[DBTask::IS_ROUTINE_TASK].numberInt();
    player->is_second_routine_[TaskInfo::TASK_GTYPE_ROUTINE] = res[DBTask::IS_SECOND_ROUTINE].numberInt();

    player->routine_total_num_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = res[DBTask::OFFER_ROUTINE_TOTAL_NUM].numberInt();
    player->routine_task_index_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = res[DBTask::OFFER_ROUTINE_TASK_INDEX].numberInt();
    player->is_finish_all_routine_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = res[DBTask::IS_FINISH_ALL_OFFER_ROUTINE].numberInt();
    player->is_routine_task_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = res[DBTask::IS_OFFER_ROUTINE_TASK].numberInt();
    player->is_second_routine_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = res[DBTask::IS_SECOND_OFFER_ROUTINE].numberInt();

    player->routine_total_num_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = res[DBTask::LEAGUE_ROUTINE_TOTAL_NUM].numberInt();
    player->routine_task_index_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = res[DBTask::LEAGUE_ROUTINE_TASK_INDEX].numberInt();
    player->is_finish_all_routine_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = res[DBTask::IS_FINISH_ALL_LEAGUE_ROUTINE].numberInt();
    player->is_routine_task_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = res[DBTask::IS_LEAGUE_ROUTINE_TASK].numberInt();
    player->is_second_routine_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = res[DBTask::IS_SECOND_LEAGUE_ROUTINE].numberInt();

    player->lcontri_day_tick_.sec(res[DBTask::LCONTRI_DAY_TICK].numberLong());
    player->lcontri_day_ = res[DBTask::LCONTRI_DAY].numberInt();
    player->ui_version_ = res[DBTask::UI_VERSION].numberInt();

    DBCommon::bson_to_int_set(player->finish_task_, res.getObjectField(DBTask::FINISH_TASK.c_str()), true);
    {
        int task_id = 0;
        BSONObjIterator iter(res.getObjectField(DBTask::SUBMITED_TASK.c_str()));
        while (iter.more())
        {
            task_id = iter.next().numberInt();
            player->task_submited_once_set().insert(task_id);
        }
    }
//    {
//        BSONObjIterator iter(res.getObjectField(DBTask::OPEN_UI.c_str()));
//        while (iter.more())
//        {
//            player->open_ui_set().insert(iter.next().str());
//        }
//    }
    {
        BSONObjIterator iter(res.getObjectField(DBTask::TASK.c_str()));
        while (iter.more())
        {
            BSONObj task_obj = iter.next().embeddedObject();
            int task_id = task_obj[DBTask::Task::TASK_ID].numberInt();
            if (CONFIG_INSTANCE->find_task(task_id) == Json::Value::null)
                continue;

            TaskInfo *task_info = MAP_MONITOR->task_info_pool()->pop();
            task_info->__task_id = task_id;
            task_info->__game_type = task_obj[DBTask::Task::GAME_TYPE].numberInt();
            task_info->__accept_tick.set(
                    task_obj[DBTask::Task::ACCEPT_TICK_SEC].numberInt(),
                    task_obj[DBTask::Task::ACCEPT_TICK_USEC].numberInt());
            task_info->__refresh_tick.set(
                    task_obj[DBTask::Task::REFRESH_TICK_SEC].numberInt(),
                    task_obj[DBTask::Task::REFRESH_TICK_USEC].numberInt());
            task_info->__prev_task = task_obj[DBTask::Task::PREV_TASK].numberInt();
            task_info->__post_task = task_obj[DBTask::Task::POST_TASK].numberInt();
            task_info->__task_star = task_obj[DBTask::Task::TASK_STAR].numberInt();
            task_info->__fast_finish_rate = task_obj[DBTask::Task::FAST_FINISH_RATE].numberInt();
            task_info->__fresh_star_times = task_obj[DBTask::Task::FRESH_STAR_TIMES].numberInt();

            BSONObjIterator status_iter(task_obj.getObjectField(DBTask::Task::STATUS.c_str()));
            while (status_iter.more())
            {
                task_info->set_task_status(status_iter.next().numberInt());
            }

            int logic_type_size = 0;
            BSONObjIterator logic_type_iter(task_obj.getObjectField(DBTask::Task::LOGIC_TYPE.c_str()));
            while (logic_type_iter.more())
            {
                ++logic_type_size;
                task_info->set_logic_type(logic_type_iter.next().numberInt());
            }
            if (logic_type_size <= 0)
            {
                MAP_MONITOR->task_info_pool()->push(task_info);
                continue;
            }

            if (task_info->is_accepted())
            {
                if (task_info->is_logic_attend())
                    player->task_accepted_attend_set().insert(task_info);
                if (task_info->is_logic_collect_item() ||
                		task_info->is_logic_collect_any())
                    player->task_accepted_collect_set().insert(task_info);
                if (task_info->is_logic_kill_monster() ||
                		task_info->is_logic_lvl_monster() ||
                		task_info->is_logic_kill_boss())
                    player->task_accepted_monster_set().insert(task_info);
                if (task_info->is_logic_scene())
                    player->task_accepted_scene_set().insert(task_info);
                if (task_info->is_logic_level_up())
                    player->task_accepted_lvl_set().insert(task_info);
                if (task_info->is_logic_package())
                {
                	player->init_task_listen_item(task_info);
                    player->task_accepted_package_set().insert(task_info);
                }
                if (task_info->is_branch_task())
                	player->task_accepted_branch_set().insert(task_info);
            }
            task_info->__task_imp = player->pop_task_imp(task_info->__game_type);

            BSONObjIterator cond_iter(task_obj.getObjectField(DBTask::Task::COND_LIST.c_str()));
            while (cond_iter.more())
            {
                BSONObj cond_obj = cond_iter.next().embeddedObject();
                TaskConditon *task_cond = MAP_MONITOR->task_condition_pool()->pop();
                task_cond->__type = cond_obj[DBTask::Task::CondList::TYPE].numberInt();
                task_cond->__current_value = cond_obj[DBTask::Task::CondList::CURRENT_VALUE].numberInt();
                task_cond->__cond_index = cond_obj[DBTask::Task::CondList::COND_INDEX].numberInt();
                task_cond->__id_list_index = cond_obj[DBTask::Task::CondList::ID_LIST_INDEX].numberInt();
                task_cond->__cond_id = cond_obj[DBTask::Task::CondList::COND_ID].numberInt();
                task_cond->__final_value = cond_obj[DBTask::Task::CondList::FINAL_VALUE].numberInt();
                task_cond->__kill_type = cond_obj[DBTask::Task::CondList::KILL_TYPE].numberInt();
                task_cond->__range_level = cond_obj[DBTask::Task::CondList::RANGE_LEVEL].numberInt();
                task_info->__condition_list.push_back(task_cond);
            }

            if (player->bind_task(task_info->__task_id, task_info) != 0)
            {
                MAP_MONITOR->task_info_pool()->push(task_info);
                continue;
            }
        }
    }

    {
        BSONObjIterator iter(res.getObjectField(DBTask::ROUTINE_CONSUME_HISTORY.c_str()));
        while (iter.more())
        {
            player->routine_consume_history_.insert(iter.next().numberInt());
        }
    }
    {
    	int i = 0;
    	BSONObjIterator iter(res.getObjectField(DBTask::UIOPEN_STEP.c_str()));
    	while (iter.more())
    	{
    		if (i > GameEnum::UIOPEN_STEP_SIZE)
    			break;
    		player->uiopen_step_[i++] = iter.next().numberInt();
    	}
    }

    {
    	// 修正进入场景完成任务的ＢＵＧ
    	BSONObj ret_field = BSON(Role::LOCATION << 1);
    	BSONObj res = this->conection().findOne(Role::COLLECTION, QUERY(Role::ID << player->role_id()), &ret_field);
    	BSONObj move_obj = res.getObjectField(Role::LOCATION.c_str());
        BSONObjIterator his_iter(move_obj.getObjectField(Role::Location::SCENE_HISTORY.c_str()));
        while (his_iter.more())
        {
            player->task_listen_enter_special_scene(his_iter.next().numberInt());
        }
    }

    return 0;
}

int MMOTask::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
    MapLogicTasker::TaskMap &task_map = player->task_map();

    BSONVec task_vc, cond_vc;
    IntVec status_vc, logic_type_vc, submited_vc, routine_consume_history_vc, uistep_vc;
    for (MapLogicTasker::TaskMap::iterator iter = task_map.begin();
            iter != task_map.end(); ++iter)
    {
        TaskInfo *task_info = iter->second;
        cond_vc.clear();
        status_vc.clear();
        logic_type_vc.clear();
        for (TaskInfo::TaskConditionList::iterator cond_iter = task_info->__condition_list.begin();
                cond_iter != task_info->__condition_list.end(); ++cond_iter)
        {
            TaskConditon *task_cond = *cond_iter;
            cond_vc.push_back(BSON(DBTask::Task::CondList::TYPE << task_cond->__type
                        << DBTask::Task::CondList::CURRENT_VALUE << task_cond->__current_value
                        << DBTask::Task::CondList::COND_INDEX << task_cond->__cond_index
                        << DBTask::Task::CondList::ID_LIST_INDEX << task_cond->__id_list_index
                        << DBTask::Task::CondList::COND_ID << task_cond->__cond_id
                        << DBTask::Task::CondList::FINAL_VALUE << task_cond->__final_value
                        << DBTask::Task::CondList::KILL_TYPE << task_cond->__kill_type
                        << DBTask::Task::CondList::RANGE_LEVEL << task_cond->__range_level));
        }
        {
            for (int i = TaskInfo::TASK_STATUS_NONE; i < TaskInfo::TASK_STATUS_END; ++i)
            {
                if (task_info->__status.test(i))
                    status_vc.push_back(i);
            }
            for (int i = TaskConditon::TASK_CT_NONE; i < TaskConditon::TASK_CT_END; ++i)
            {
                if (task_info->__logic_type.test(i))
                    logic_type_vc.push_back(i);
            }
        }
        task_vc.push_back(BSON(DBTask::Task::TASK_ID << task_info->__task_id
                    << DBTask::Task::GAME_TYPE << task_info->__game_type
                    << DBTask::Task::ACCEPT_TICK_SEC << int(task_info->__accept_tick.sec())
                    << DBTask::Task::ACCEPT_TICK_USEC << int(task_info->__accept_tick.usec())
                    << DBTask::Task::REFRESH_TICK_SEC << int(task_info->__refresh_tick.sec())
                    << DBTask::Task::REFRESH_TICK_USEC << int(task_info->__refresh_tick.usec())
                    << DBTask::Task::PREV_TASK << task_info->__prev_task
                    << DBTask::Task::POST_TASK << task_info->__post_task
                    << DBTask::Task::TASK_STAR << task_info->__task_star
                    << DBTask::Task::FAST_FINISH_RATE << task_info->__fast_finish_rate
                    << DBTask::Task::FRESH_STAR_TIMES << task_info->__fresh_star_times
                    << DBTask::Task::STATUS << status_vc
                    << DBTask::Task::LOGIC_TYPE << logic_type_vc
                    << DBTask::Task::COND_LIST << cond_vc));
    }
    submited_vc.clear();
    {
        for (MapLogicTasker::TaskIdSet::iterator iter = player->task_submited_once_set().begin();
                iter != player->task_submited_once_set().end(); ++iter)
        {
            submited_vc.push_back(*iter);
        }
    }
    {
    	for (MapLogicTasker::TaskIdSet::iterator iter = player->routine_consume_history_.begin();
    			iter != player->routine_consume_history_.end(); ++iter)
    	{
    		routine_consume_history_vc.push_back(*iter);
    	}
    }
    {
    	for (int i = 0; i <= GameEnum::UIOPEN_STEP_SIZE; ++i)
    	{
    		uistep_vc.push_back(player->uiopen_step_[i]);
    	}
    }
//    std::vector<std::string> open_ui_vc;
//    {
//        for (MapLogicTasker::OpenUiSet::iterator iter = player->open_ui_set().begin(); iter != player->open_ui_set().end(); ++iter)
//            open_ui_vc.push_back(*iter);
//    }

    BSONObjBuilder builder;
    builder << DBTask::NOVICE_STEP << player->novice_step()
    	<< DBTask::LATEST_MAIN_TASK << player->latest_main_task()
        << DBTask::UIOPEN_STEP << uistep_vc
    	<< DBTask::SUBMITED_TASK << submited_vc
        << DBTask::TASK << task_vc
        << DBTask::FINISH_TASK << player->finish_task_

        << DBTask::ROUTINE_TOTAL_NUM << player->routine_total_num_[TaskInfo::TASK_GTYPE_ROUTINE]
        << DBTask::ROUTINE_REFRESH_TICK << int(player->routine_refresh_tick_.sec())
        << DBTask::ROUTINE_TASK_INDEX << player->routine_task_index_[TaskInfo::TASK_GTYPE_ROUTINE]
        << DBTask::ROUTINE_CONSUME_HISTORY << routine_consume_history_vc
        << DBTask::IS_FINISH_ALL_ROUTINE << player->is_finish_all_routine_[TaskInfo::TASK_GTYPE_ROUTINE]
        << DBTask::IS_ROUTINE_TASK << player->is_routine_task_[TaskInfo::TASK_GTYPE_ROUTINE]
        << DBTask::IS_SECOND_ROUTINE << player->is_second_routine_[TaskInfo::TASK_GTYPE_ROUTINE]

        << DBTask::OFFER_ROUTINE_TOTAL_NUM << player->routine_total_num_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE]
        << DBTask::OFFER_ROUTINE_TASK_INDEX << player->routine_task_index_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE]
        << DBTask::IS_FINISH_ALL_OFFER_ROUTINE << player->is_finish_all_routine_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE]
        << DBTask::IS_OFFER_ROUTINE_TASK << player->is_routine_task_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE]
        << DBTask::IS_SECOND_OFFER_ROUTINE << player->is_second_routine_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE]

        << DBTask::LEAGUE_ROUTINE_TOTAL_NUM << player->routine_total_num_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE]
        << DBTask::LEAGUE_ROUTINE_TASK_INDEX << player->routine_task_index_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE]
        << DBTask::IS_FINISH_ALL_LEAGUE_ROUTINE << player->is_finish_all_routine_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE]
        << DBTask::IS_LEAGUE_ROUTINE_TASK << player->is_routine_task_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE]
        << DBTask::IS_SECOND_LEAGUE_ROUTINE << player->is_second_routine_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE]

        << DBTask::LCONTRI_DAY_TICK << Int64(player->lcontri_day_tick_.sec())
        << DBTask::LCONTRI_DAY << player->lcontri_day_
        << DBTask::UI_VERSION << player->ui_version_;

    data_map->push_update(DBTask::COLLECTION,
            BSON(DBTask::ID << player->role_id()),
            builder.obj(), true);

    return 0;
}

void MMOTask::ensure_all_index(void)
{
    this->conection().ensureIndex(DBTask::COLLECTION, BSON(DBTask::ID << 1), true);
}


MMOSysSetting::MMOSysSetting() {
	// TODO Auto-generated constructor stub

}

MMOSysSetting::~MMOSysSetting() {
	// TODO Auto-generated destructor stub
}

int MMOSysSetting::load_player_system_setting(MapLogicPlayer* player)
{
	return 0;
}

int MMOSysSetting::update_data(MapLogicPlayer* player, MongoDataMap* data_map)
{
	return 0;
}

int MMOSysSetting::sync_setting_info_to_other_role(std::string account,
		long long int  role_id, BSONObj& set_detail)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBSysSetting::COLLECTION,
			QUERY(DBSysSetting::ACCOUNT << account));
	while(cursor->more())
	{
		BSONObj res = cursor->next();

		Int64 target_id = res[DBSysSetting::ID].numberLong();
		JUDGE_CONTINUE(target_id != role_id);

		BSONObjBuilder build;
		build << DBSysSetting::ACCOUNT << account
				<< DBSysSetting::SET_DETAIL << set_detail;
		CACHED_CONNECTION.update(DBSysSetting::COLLECTION, QUERY(DBSysSetting::ID << target_id),
				BSON("$set" << build.obj()), true);
	}
	return 0;
}

void MMOSysSetting::ensure_all_index(void)
{
	this->conection().ensureIndex(DBSysSetting::COLLECTION, BSON(DBSysSetting::ACCOUNT << 1), true);
}

MMOWelfare::MMOWelfare() {
	// TODO Auto-generated constructor stub

}

MMOWelfare::~MMOWelfare() {
	// TODO Auto-generated destructor stub
}


int MMOWelfare::load_player_welfare(MapLogicPlayer *player)
{
	CheckInDetail &check_in_detail = player->check_in_detail();
	check_in_detail.__popup = 1;

	BSONObj res = this->conection().findOne(DBWelfare::COLLECTION,
			QUERY(DBWelfare::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, 0);

	BSONObj check_in_obj = res[DBWelfare::CHECK_IN].Obj();
	check_in_detail.__award_index = check_in_obj[DBWelfare::CheckIn::AWARD_INDEX].numberInt();
	check_in_detail.__check_in_point = check_in_obj[DBWelfare::CheckIn::CHECK_IN_POINT].numberLong();
	check_in_detail.__last_time = check_in_obj[DBWelfare::CheckIn::LAST_TIME].numberInt();
	check_in_detail.__show_point = check_in_obj[DBWelfare::CheckIn::SHOW_POINT].numberInt();

	check_in_detail.__charge_money = check_in_obj[DBWelfare::CheckIn::CHARGE_MONEY].numberInt();
	check_in_detail.__check_total_index = check_in_obj[DBWelfare::CheckIn::CHECK_TOTAL_INDEX].numberLong();
	check_in_detail.__total_last_time = check_in_obj[DBWelfare::CheckIn::TOTAL_LAST_TIME].numberInt();
	check_in_detail.__total_popup = check_in_obj[DBWelfare::CheckIn::TOTAL_POPUP].numberInt();

	if(check_in_detail.__check_in_point && !check_in_detail.__show_point)
	{
		check_in_detail.__show_point = 1;
	}

	BSONObj once_reward_bson = res[DBWelfare::ONCE_REWARDS].Obj();
	this->mmo_once_rewards_load_from_bson(once_reward_bson, player);

	return 0;
}

int MMOWelfare::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	CheckInDetail &check_in_detail = player->check_in_detail();

	BSONObj once_reward_bson;
	mmo_once_rewards_dump_to_bson(player, once_reward_bson);

	BSONObjBuilder builder;
	builder << DBWelfare::CHECK_IN << BSON(
			 DBWelfare::CheckIn::AWARD_INDEX << check_in_detail.__award_index
				<< DBWelfare::CheckIn::CHECK_IN_POINT << check_in_detail.__check_in_point
				<< DBWelfare::CheckIn::LAST_TIME << check_in_detail.__last_time
				<< DBWelfare::CheckIn::SHOW_POINT << check_in_detail.__show_point

		        << DBWelfare::CheckIn::CHARGE_MONEY << check_in_detail.__charge_money
				<< DBWelfare::CheckIn::CHECK_TOTAL_INDEX << check_in_detail.__check_total_index
				<< DBWelfare::CheckIn::TOTAL_LAST_TIME << check_in_detail.__total_last_time
				<< DBWelfare::CheckIn::TOTAL_POPUP << check_in_detail.__total_popup)
			<< DBWelfare::ONCE_REWARDS << once_reward_bson;

	data_map->push_update(DBWelfare::COLLECTION, BSON(DBWelfare::ID << player->role_id()),
			builder.obj(), true);
	return 0;
}

void MMOWelfare::ensure_all_index(void)
{
	this->conection().ensureIndex(DBWelfare::COLLECTION, BSON(DBWelfare::ID << 1), true);
}

int MMOWelfare::mmo_once_rewards_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj)
{
	JUDGE_RETURN(player != NULL, -1);

	OnceRewardRecords &records = player->once_reward_records();
	typedef DBWelfare::OnceRewards DBOnceRewards;
	BSONObjBuilder builder;
	builder << DBOnceRewards::UPDATE_RES_FLAG << records.__update_res_flag;

	res_obj = builder.obj();
	return 0;
}

int MMOWelfare::mmo_once_rewards_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player)
{
	JUDGE_RETURN(player != NULL, -1);

	OnceRewardRecords &records = player->once_reward_records();
	records.reset();
	JUDGE_RETURN(!bson_obj.isEmpty(), 0);

	typedef DBWelfare::OnceRewards DBOnceRewards;
	records.__update_res_flag = bson_obj[DBOnceRewards::UPDATE_RES_FLAG].numberInt();
	return 0;
}
