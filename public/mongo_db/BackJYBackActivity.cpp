/*
 * File Name: BackJYBackActivity.cpp
 * 
 * Created on: 2017-04-07 16:55:49
 * Author: glendy
 * 
 * Last Modified: 2017-04-22 15:47:19
 * Description: 
 */

#include "BackJYBackActivity.h"
#include "ActivityStruct.h"
#include "BackField.h"
#include "GameCommon.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MongoData.h"
#include "LogicMonitor.h"
#include "JYBackActivitySys.h"
#include "TQueryCursor.h"
#include "LogicPlayer.h"
#include "GameField.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

BackJYBackActivity::~BackJYBackActivity(void)
{ /*NULL*/ }

int BackJYBackActivity::cond_typ_from_second_type(const int second_type)
{
    switch (second_type)
    {
    case JYBackActivityItem::STYPE_ACCU_RECHARGE:
    case JYBackActivityItem::STYPE_ACCU_CONSUM:
        return JYBackActivityItem::CONDTYPE_SINGLE_GTE;
    case JYBackActivityItem::STYPE_REPEAT_RECHARGE:
        return JYBackActivityItem::CONDTYPE_SINGLE_DIVID;
    case JYBackActivityItem::STYPE_SINGLE_RECHARGE_RANK:
    case JYBackActivityItem::STYPE_TRAVEL_RECHARGE_RANK:
        return JYBackActivityItem::CONDTYPE_RANGE;
    default:
        return JYBackActivityItem::CONDTYPE_SINGLE_EQUAL;
    }
}

int BackJYBackActivity::reward_type_from_second_type(const int second_type)
{
    return JYBackActivityItem::REWARDTYPE_ACT_END;
}

int BackJYBackActivity::bson_to_act_item(BSONObj &act_obj, JYBackActivityItem *act_item)
{
    act_item->__act_id = act_obj[DBJYBackActivity::ACT_ID].numberInt();
    act_item->__first_type = act_obj[DBJYBackActivity::FIRST_TYPE].numberInt();
    act_item->__second_type = act_obj[DBJYBackActivity::SECOND_TYPE].numberInt();
    act_item->__act_title = act_obj[DBJYBackActivity::ACT_TITLE].str();
    act_item->__act_content = act_obj[DBJYBackActivity::ACT_CONTENT].str();
    act_item->__act_start.sec(act_obj[DBJYBackActivity::ACT_START].numberLong());
    act_item->__act_end.sec(act_obj[DBJYBackActivity::ACT_END].numberLong());
    act_item->__show_end = act_item->__act_end + Time_Value(1800);
    act_item->__is_open = act_obj[DBJYBackActivity::IS_OPEN].numberInt();
    act_item->__order = act_obj[DBJYBackActivity::ORDER].numberInt();
    act_item->__reward_mail_title = act_obj[DBJYBackActivity::REWARD_MAIL_TITLE].str();
    act_item->__reward_mail_content = act_obj[DBJYBackActivity::REWARD_MAIL_CONTENT].str();
    act_item->__need_gold = act_obj[DBJYBackActivity::NEED_GOLD].numberInt();
    act_item->__update_tick.sec(act_obj[DBJYBackActivity::UPDATE_TICK].numberLong());
    if (act_item->__update_tick.sec() == 0)
        act_item->__update_tick.sec(::time(NULL));

    if (act_obj.hasField(DBJYBackActivity::REWARD.c_str()))
    {
		ItemObj item_info;
		BSONObjIterator reward_iter(act_obj.getObjectField(DBJYBackActivity::REWARD.c_str()));
		while (reward_iter.more())
		{
			BSONObj reward_obj = reward_iter.next().embeddedObject();
			act_item->__reward_list.push_back(JYBackActivityItem::Reward());
			JYBackActivityItem::Reward &reward_info = act_item->__reward_list[int(act_item->__reward_list.size()) - 1];
			reward_info.reset();
			reward_info.__cond_type = BackJYBackActivity::cond_typ_from_second_type(act_item->__second_type);
			reward_info.__restore_gold_rate = reward_obj[DBJYBackActivity::Reward::RETURN_GOLD_RATE].numberInt();

			{
				BSONObjIterator cond_iter(reward_obj.getObjectField(DBJYBackActivity::Reward::COND.c_str()));
				while (cond_iter.more())
				{
					reward_info.__cond_list.push_back(cond_iter.next().numberInt());
				}
			}

	//        reward_info.__reward_name = reward_obj[DBJYBackActivity::Reward::REWARD_NAME].str();
			reward_info.__reward_type = BackJYBackActivity::reward_type_from_second_type(act_item->__second_type);

			{
				BSONObjIterator item_iter(reward_obj.getObjectField(DBJYBackActivity::Reward::REWARD_ITEM.c_str()));
				while (item_iter.more())
				{
					BSONObjIterator value_iter(item_iter.next().embeddedObject());
					while (value_iter.more())
					{
						item_info.reset();
						item_info.bind_ = GameEnum::ITEM_BIND;
						if (value_iter.more())
							item_info.id_ = value_iter.next().numberInt();
						if (value_iter.more())
							item_info.amount_ = value_iter.next().numberInt();
						if (value_iter.more())
							item_info.bind_ = value_iter.next().numberInt();
						if (item_info.bind_ != GameEnum::ITEM_NO_BIND)
							item_info.bind_ = GameEnum::ITEM_BIND;
						reward_info.__reward_item.push_back(item_info);
					}
				}
			}
		}
    }

    MSG_USER("load back act info %d %d %d %d %d", act_item->__act_id,
    		act_item->__first_type, act_item->__second_type,
    		act_item->__act_start.sec(), act_item->__act_end.sec());
    return 0;
}

int BackJYBackActivity::load_back_activity(JYBackActivitySys *activity_sys)
{
BEGIN_CATCH
    Time_Value nowtime = Time_Value::gettimeofday();
	{
		auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBJYBackActivity::COLLECTION, QUERY(DBJYBackActivity::ACT_START << BSON("$lte" << int(nowtime.sec() - 7200)) << DBJYBackActivity::ACT_END << BSON("$gte" << int(nowtime.sec())) << DBJYBackActivity::IS_OPEN << 1).sort(DBJYBackActivity::ORDER));
		while (cursor->more())
		{
			BSONObj act_obj = cursor->next();
			JYBackActivityItem *act_item = LOGIC_MONITOR->jyback_activity_item_pool()->pop();
			BackJYBackActivity::bson_to_act_item(act_obj, act_item);
			if (act_item->__first_type == 0 || act_item->__second_type <= 0)
			{
				MSG_USER("back activity error %d %d %d", act_item->__act_id, act_item->__first_type, act_item->__second_type);
				LOGIC_MONITOR->jyback_activity_item_pool()->push(act_item);
				continue;
			}
			if (act_obj[DBJYBackActivity::UPDATE_FLAG].numberInt() == 1)
				act_item->__update_tick = Time_Value::gettimeofday();

			JYBackActivitySys::ActivityItemIDMap::iterator act_iter = activity_sys->id_to_act_item_map()->find(act_item->__act_id);
			if (act_iter != activity_sys->id_to_act_item_map()->end())
			{
				MSG_USER("back activity repeat error %d %d %d", act_item->__act_id, act_item->__first_type, act_item->__second_type);
				LOGIC_MONITOR->jyback_activity_item_pool()->push(act_item);
				continue;
			}
			(*(activity_sys->id_to_act_item_map()))[act_item->__act_id] = act_item;
			(*(activity_sys->first_type_to_act_item_map()))[act_item->__first_type].push_back(act_item);
			(*(activity_sys->second_type_to_act_item_map()))[act_item->__second_type].push_back(act_item);
		}
	}

	{
		JYBackActivitySys::ActivityItemIDMap *expire_act_map = activity_sys->expire_second_type_act_item_map();
		auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBJYBackActivity::COLLECTION, QUERY(DBJYBackActivity::ACT_END << BSON("$lt" << int(nowtime.sec())) << DBJYBackActivity::IS_OPEN << 1).sort(DBJYBackActivity::ACT_END, -1));
		while (cursor->more())
		{
			BSONObj act_obj = cursor->next();
			int second_type = act_obj[DBJYBackActivity::SECOND_TYPE].numberInt();
			JUDGE_CONTINUE(expire_act_map->find(second_type) == expire_act_map->end());
			JUDGE_CONTINUE(activity_sys->is_expire_send_mail_act(second_type));

			JYBackActivityItem *act_item = LOGIC_MONITOR->jyback_activity_item_pool()->pop();
			BackJYBackActivity::bson_to_act_item(act_obj, act_item);
			(*(expire_act_map))[second_type] = act_item;
		}
	}
    return 0;
END_CACHE_CATCH
	return -1;
}

int BackJYBackActivity::request_load_update_data(JYBackActivitySys *activity_sys, MongoDataMap *data_map)
{
	Time_Value nowtime = Time_Value::gettimeofday();
    data_map->push_multithread_query(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::ACT_START << BSON("$lte" << int(nowtime.sec())) << DBJYBackActivity::ACT_END << BSON("$gte" << int(nowtime.sec())) << DBJYBackActivity::UPDATE_FLAG << 1));
    return 0;
}

void BackJYBackActivity::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::ACT_ID << 1), true);
    this->conection().ensureIndex(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::UPDATE_FLAG << 1), false);
    this->conection().ensureIndex(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::IS_OPEN << 1), false);
    this->conection().ensureIndex(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::ACT_END << -1), false);
    this->conection().ensureIndex(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::ACT_START << 1 << DBJYBackActivity::ACT_END << -1), false);
    this->conection().ensureIndex(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::ACT_START << 1 << DBJYBackActivity::ACT_END << -1 << DBJYBackActivity::IS_OPEN << 1), false);

    this->conection().ensureIndex(DBJYBackActPlayerRec::COLLECTION, BSON(DBJYBackActPlayerRec::ID << 1), true);
END_CATCH
}

int BackJYBackActivity::update_from_load(JYBackActivitySys *activity_sys, MongoDataMap *data_map, std::vector<int> &update_id_list)
{
    JUDGE_RETURN(data_map != NULL, 0);

    MongoData *mongo_data = NULL;
    if (data_map->find_data(DBJYBackActivity::COLLECTION, mongo_data) != 0 || mongo_data == NULL)
        return 0;

    bool has_data = false;
    auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
    while (cursor->more())
    {
    	has_data = true;
        BSONObj act_obj = cursor->next();
        int act_id = act_obj[DBJYBackActivity::ACT_ID].numberInt();
        JYBackActivityItem *act_item = activity_sys->find_act_item_by_id(act_id);
        if (act_item == NULL)
        {
            act_item = LOGIC_MONITOR->jyback_activity_item_pool()->pop();
            BackJYBackActivity::bson_to_act_item(act_obj, act_item);
            act_item->__update_tick = Time_Value::gettimeofday();
            update_id_list.push_back(act_item->__act_id);
            if (act_item->__first_type == 0 || act_item->__second_type <= 0)
            {
                MSG_USER("back activity error %d %d %d", act_item->__act_id, act_item->__first_type, act_item->__second_type);
                LOGIC_MONITOR->jyback_activity_item_pool()->push(act_item);
                continue;
            }

            if (act_item->__is_open == 0 || act_item->__act_end <= act_item->__update_tick)
            	continue;

            (*(activity_sys->id_to_act_item_map()))[act_item->__act_id] = act_item;
            (*(activity_sys->first_type_to_act_item_map()))[act_item->__first_type].push_back(act_item);
            (*(activity_sys->second_type_to_act_item_map()))[act_item->__second_type].push_back(act_item);
        }
        else
        {
        	act_item->reset();
            BackJYBackActivity::bson_to_act_item(act_obj, act_item);
            act_item->__update_tick = Time_Value::gettimeofday();
            update_id_list.push_back(act_item->__act_id);
            if (act_item->__is_open == 0 || act_item->__act_end <= act_item->__update_tick)
            {
            	activity_sys->remove_act_item(act_item);
            }
        }
    }
    JUDGE_RETURN(has_data == true, -1);
    return 0;
}

int BackJYBackActivity::update_act_update_flag(const int act_id, MongoDataMap *data_map)
{
	data_map->push_update(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::ACT_ID << act_id << DBJYBackActivity::UPDATE_FLAG << 1), BSON(DBJYBackActivity::UPDATE_FLAG << 0 << DBJYBackActivity::UPDATE_TICK << int(time(NULL))));
	return 0;
}

int BackJYBackActivity::load_player_back_act_rec(LogicPlayer *player)
{
BEGIN_CATCH
    BSONObj res = this->conection().findOne(DBJYBackActPlayerRec::COLLECTION, QUERY(DBJYBackActPlayerRec::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, -1);

    BSONObjIterator act_record_iter(res.getObjectField(DBJYBackActPlayerRec::ACT_RECORD.c_str()));
    while (act_record_iter.more())
    {
        BSONObj record_obj = act_record_iter.next().embeddedObject();
        int act_id = record_obj[DBJYBackActPlayerRec::ActRecord::ACT_ID].numberInt();
        JUDGE_CONTINUE(act_id > 0);
        JYBackActivityPlayerRecord &player_record_info = player->back_act_player_rec_map()[act_id];
        player_record_info.reset();
        player_record_info.__act_id = act_id;
        player_record_info.__act_start = record_obj[DBJYBackActPlayerRec::ActRecord::ACT_START].numberLong();
        player_record_info.__act_end = record_obj[DBJYBackActPlayerRec::ActRecord::ACT_END].numberLong();
        player_record_info.__mail_title = record_obj[DBJYBackActPlayerRec::ActRecord::MAIL_TITLE].str();
        player_record_info.__mail_content = record_obj[DBJYBackActPlayerRec::ActRecord::MAIL_CONTENT].str();
        player_record_info.__daily_refresh_tick = record_obj[DBJYBackActPlayerRec::ActRecord::DAILY_REFRESH_TICK].numberLong();
        player_record_info.__daily_value = record_obj[DBJYBackActPlayerRec::ActRecord::DAILY_VALUE].numberInt();
        player_record_info.__single_cond_value = record_obj[DBJYBackActPlayerRec::ActRecord::SINGLE_COND_VALUE].numberInt();

        {
            BSONObjIterator reward_value_iter(record_obj.getObjectField(DBJYBackActPlayerRec::ActRecord::REWARD_VALUE.c_str()));
            while (reward_value_iter.more())
            {
                BSONObj reward_value_obj = reward_value_iter.next().embeddedObject();
                int reward_id = reward_value_obj[DBJYBackActPlayerRec::ActRecord::RewardValue::REWARD_ID].numberInt();
                JUDGE_CONTINUE(reward_id >= 0);
                JYBackActivityPlayerRecord::RewardMap &reward_map = player_record_info.__reward_value_map[reward_id];
                BSONObjIterator reward_iter(reward_value_obj.getObjectField(DBJYBackActPlayerRec::ActRecord::RewardValue::REWARD_FLAG.c_str()));
                while (reward_iter.more())
                {
                    int cond_value = reward_iter.next().numberInt(), flag = 0;
                    if (reward_iter.more())
                    {
                        flag = reward_iter.next().numberInt();
                        reward_map[cond_value] = flag;
                    }
                }
            }
        }

        player_record_info.__update_tick.sec(record_obj[DBJYBackActPlayerRec::ActRecord::UPDATE_TICK].numberInt());

        {
            ItemObj item_obj;
            BSONObjIterator reward_items_iter(record_obj.getObjectField(DBJYBackActPlayerRec::ActRecord::REWARD_ITEM.c_str()));
            while (reward_items_iter.more())
            {
                BSONObj reward_items_obj = reward_items_iter.next().embeddedObject();
                int reward_id = reward_items_obj[DBJYBackActPlayerRec::ActRecord::RewardItem::REWARD_ID].numberInt();
                JUDGE_CONTINUE(reward_id >= 0);

                JYBackActivityPlayerRecord::ItemVec &item_list = player_record_info.__index_reward_item_map[reward_id];
                BSONObjIterator item_iter(reward_items_obj.getObjectField(DBJYBackActPlayerRec::ActRecord::RewardItem::ITEM_LIST.c_str()));
                while (item_iter.more())
                {
                    item_obj.reset();
                    item_obj.bind_ = GameEnum::ITEM_BIND;
                    item_obj.id_ = item_iter.next().numberInt();
                    if (item_iter.more() == false)
                        continue;
                    item_obj.amount_ = item_iter.next().numberInt();
                    if (item_iter.more())
                        item_obj.bind_ = item_iter.next().numberInt();
                    item_list.push_back(item_obj);
                }
            }
        }
    }

    std::vector<int> remove_id_vc;
    LogicBackActivityer::BackActivityRecordMap &record_map = player->back_act_player_rec_map();
    for (LogicBackActivityer::BackActivityRecordMap::iterator iter = record_map.begin(); iter != record_map.end(); ++iter)
    {
        int act_id = iter->first;
        BSONObj res = this->conection().findOne(DBJYBackActivity::COLLECTION, BSON(DBJYBackActivity::ACT_ID << act_id << DBJYBackActivity::IS_OPEN << 1));
        if (res.isEmpty() == true)
            remove_id_vc.push_back(act_id);
    }
    for (std::vector<int>::iterator iter = remove_id_vc.begin(); iter != remove_id_vc.end(); ++iter)
    {
        record_map.erase(*iter);
    }
    
    return 0;
END_CATCH
    return -1;
}

int BackJYBackActivity::update_data(LogicPlayer *player, MongoDataMap *data_map)
{
    LogicBackActivityer::BackActivityRecordMap &player_record_map = player->back_act_player_rec_map();

    std::vector<BSONObj> record_vc, reward_value_vc, reward_items_vc;
    std::vector<int> reward_vc, items_vc;

    for (LogicBackActivityer::BackActivityRecordMap::iterator record_iter = player_record_map.begin();
            record_iter != player_record_map.end(); ++record_iter)
    {
        JYBackActivityPlayerRecord &player_record_info = record_iter->second;
        reward_value_vc.clear();
        for (JYBackActivityPlayerRecord::IndexRewardMap::iterator index_reward_iter = player_record_info.__reward_value_map.begin();
                index_reward_iter != player_record_info.__reward_value_map.end(); ++index_reward_iter)
        {
            int reward_id = index_reward_iter->first;

            JYBackActivityPlayerRecord::RewardMap &reward_map = index_reward_iter->second;
            reward_vc.clear();
            for (JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.begin();
                    reward_iter != reward_map.end(); ++reward_iter)
            {
                reward_vc.push_back(reward_iter->first);
                reward_vc.push_back(reward_iter->second);
            }
            reward_value_vc.push_back(BSON(DBJYBackActPlayerRec::ActRecord::RewardValue::REWARD_ID << reward_id
                        << DBJYBackActPlayerRec::ActRecord::RewardValue::REWARD_FLAG << reward_vc));
        }

        reward_items_vc.clear();
        for (JYBackActivityPlayerRecord::IndexRewardItemMap::iterator index_items_iter = player_record_info.__index_reward_item_map.begin();
                index_items_iter != player_record_info.__index_reward_item_map.end(); ++index_items_iter)
        {
            int reward_id = index_items_iter->first;

            JYBackActivityPlayerRecord::ItemVec &item_list = index_items_iter->second;
            items_vc.clear();
            for (JYBackActivityPlayerRecord::ItemVec::iterator items_iter = item_list.begin();
                    items_iter != item_list.end(); ++items_iter)
            {
                ItemObj &item_obj = *items_iter;
                items_vc.push_back(item_obj.id_);
                items_vc.push_back(item_obj.amount_);
                items_vc.push_back(item_obj.bind_);
            }
            reward_items_vc.push_back(BSON(DBJYBackActPlayerRec::ActRecord::RewardItem::REWARD_ID << reward_id
            		<< DBJYBackActPlayerRec::ActRecord::RewardItem::ITEM_LIST << items_vc));
        }

        record_vc.push_back(BSON(DBJYBackActPlayerRec::ActRecord::ACT_ID << player_record_info.__act_id
                    << DBJYBackActPlayerRec::ActRecord::ACT_START << player_record_info.__act_start
                    << DBJYBackActPlayerRec::ActRecord::ACT_END << player_record_info.__act_end
                    << DBJYBackActPlayerRec::ActRecord::MAIL_TITLE << player_record_info.__mail_title
                    << DBJYBackActPlayerRec::ActRecord::MAIL_CONTENT << player_record_info.__mail_content
                    << DBJYBackActPlayerRec::ActRecord::DAILY_REFRESH_TICK << player_record_info.__daily_refresh_tick
                    << DBJYBackActPlayerRec::ActRecord::DAILY_VALUE << player_record_info.__daily_value
                    << DBJYBackActPlayerRec::ActRecord::SINGLE_COND_VALUE << player_record_info.__single_cond_value
                    << DBJYBackActPlayerRec::ActRecord::REWARD_VALUE << reward_value_vc
                    << DBJYBackActPlayerRec::ActRecord::UPDATE_TICK << int(player_record_info.__update_tick.sec())
                    << DBJYBackActPlayerRec::ActRecord::REWARD_ITEM << reward_items_vc));
    }

    BSONObjBuilder builder;
    builder << DBJYBackActPlayerRec::ID << player->role_id()
        << DBJYBackActPlayerRec::ACT_RECORD << record_vc;

    data_map->push_update(DBJYBackActPlayerRec::COLLECTION, BSON(DBJYBackActPlayerRec::ID << player->role_id()), builder.obj(), true);

    return 0;
}

