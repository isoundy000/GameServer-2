/*
 * File Name: JYBackActivitySys.cpp
 * 
 * Created on: 2017-04-06 09:58:54
 * Author: glendy
 * 
 * Last Modified: 2017-04-22 15:55:51
 * Description: 
 */

#include "JYBackActivitySys.h"
#include "BackJYBackActivity.h"
#include "PubStruct.h"
#include "PoolMonitor.h"
#include "Transaction.h"
#include "ActivityStruct.h"
#include "LogicMonitor.h"
#include "MongoDataMap.h"
#include "LogicPlayer.h"
#include "TrvlRechargeMonitor.h"

JYBackActivitySys::JYBackActivitySys(void)
{
    this->first_type_to_act_item_map_ = new TypeActivityItemMap();
    this->second_type_to_act_item_map_ = new TypeActivityItemMap();
    this->id_to_act_item_map_ = new ActivityItemIDMap();
    this->back_update_tick_ = Time_Value::zero;
    this->check_remove_tick_ = Time_Value::zero;

    this->expire_second_type_act_item_map_ = new ActivityItemIDMap();
}

JYBackActivitySys::~JYBackActivitySys(void)
{
    SAFE_DELETE(this->first_type_to_act_item_map_);
    SAFE_DELETE(this->second_type_to_act_item_map_);
    SAFE_DELETE(this->id_to_act_item_map_);
    SAFE_DELETE(this->expire_second_type_act_item_map_);
}

int JYBackActivitySys::start(void)
{
    this->load_back_activity();

    return 0;
}

int JYBackActivitySys::stop(void)
{
    return 0;
}

JYBackActivitySys::ActivityItemList *JYBackActivitySys::find_act_items_by_first_type(const int first_type)
{
    TypeActivityItemMap::iterator iter = this->first_type_to_act_item_map_->find(first_type);
    if (iter != this->first_type_to_act_item_map_->end())
        return &(iter->second);
    return NULL;
}

JYBackActivitySys::ActivityItemList *JYBackActivitySys::find_act_items_by_second_type(const int second_type)
{
    TypeActivityItemMap::iterator iter = this->second_type_to_act_item_map_->find(second_type);
    if (iter != this->second_type_to_act_item_map_->end())
        return &(iter->second);
    return NULL;
}

JYBackActivityItem *JYBackActivitySys::find_act_item_by_id(const int act_id)
{
    ActivityItemIDMap::iterator iter = this->id_to_act_item_map_->find(act_id);
    if (iter != this->id_to_act_item_map_->end())
        return iter->second;
    return NULL;
}

JYBackActivityItem *JYBackActivitySys::find_expire_act_item_by_second_type(const int second_type)
{
	ActivityItemIDMap::iterator iter = this->expire_second_type_act_item_map_->find(second_type);
	if (iter != this->expire_second_type_act_item_map_->end())
		return iter->second;
	return NULL;
}

int JYBackActivitySys::fetch_back_act_type_list(IntSet &first_type_list)
{
    for (TypeActivityItemMap::iterator iter = this->first_type_to_act_item_map_->begin();
            iter != this->first_type_to_act_item_map_->end(); ++iter)
    {
        first_type_list.insert(iter->first);
    }
    return 0;
}

JYBackActivitySys::TypeActivityItemMap *JYBackActivitySys::first_type_to_act_item_map(void)
{
    return this->first_type_to_act_item_map_;
}

JYBackActivitySys::TypeActivityItemMap *JYBackActivitySys::second_type_to_act_item_map(void)
{
    return this->second_type_to_act_item_map_;
}

JYBackActivitySys::ActivityItemIDMap *JYBackActivitySys::id_to_act_item_map(void)
{
    return this->id_to_act_item_map_;
}

JYBackActivitySys::ActivityItemIDMap *JYBackActivitySys::expire_second_type_act_item_map(void)
{
	return this->expire_second_type_act_item_map_;
}

int JYBackActivitySys::check_back_activity_timeout(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->back_update_tick_ <= nowtime, 0);

    this->back_update_tick_ = Time_Value::gettimeofday() + Time_Value(10);

    this->request_load_back_act_update();

	// 把过期的活动删除
	this->clear_back_act_expire();

    return 0;
}

int JYBackActivitySys::load_back_activity(void)
{
    return BackJYBackActivity::load_back_activity(this);
}

int JYBackActivitySys::request_load_back_act_update(void)
{
    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    BackJYBackActivity::request_load_update_data(this, data_map);
    if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_BACK_ACTIVITY_UPDATE, data_map, LOGIC_MONITOR->logic_unit()) != 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
        return -1;
    }
    return 0;
}

int JYBackActivitySys::process_after_load_back_act_update(Transaction *transaction)
{
    JUDGE_RETURN(transaction != NULL, 0);
    MongoDataMap *data_map = transaction->fetch_mongo_data_map();
    int prev_act_size = this->id_to_act_item_map_->size();
    IntVec update_id_list;
    int ret = BackJYBackActivity::update_from_load(this, data_map, update_id_list);
    if (ret != 0)
    {
    	transaction->summit();
    	return 0;
    }

    if (update_id_list.size() > 0)
    {
		for (TypeActivityItemMap::iterator type_act_iter = this->first_type_to_act_item_map_->begin();
				type_act_iter != this->first_type_to_act_item_map_->end(); ++type_act_iter)
		{
			ActivityItemList &act_item_list = type_act_iter->second;
			// first_type 排序
			std::sort(act_item_list.begin(), act_item_list.end(), JYBackActivityItemCmp);
		}
    }

    std::vector<int> remove_id_list;
    for (IntVec::iterator iter = update_id_list.begin(); iter != update_id_list.end(); ++iter)
    {
        if (this->find_act_item_by_id(*iter) == NULL)
            remove_id_list.push_back(*iter);

    	MongoDataMap *mdata_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    	BackJYBackActivity::update_act_update_flag(*iter, mdata_map);
        if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_BACK_ACTIVITY_UPDATE_FLAG, mdata_map) != 0)
        {
            POOL_MONITOR->mongo_data_map_pool()->push(mdata_map);
        }
    }

    transaction->summit();

    if (update_id_list.size() > 0 || prev_act_size != int(this->id_to_act_item_map_->size()))
    {
    	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
    	for (LogicMonitor::PlayerMap::iterator player_iter = player_map.begin(); player_iter != player_map.end(); ++player_iter)
    	{
    		LogicPlayer *player = player_iter->second;
    		if (player != NULL)
            {
                for (std::vector<int>::iterator remove_iter = remove_id_list.begin(); remove_iter != remove_id_list.end(); ++remove_iter)
                {
                    player->back_act_player_rec_map().erase(*remove_iter);
                }
    			player->notify_activity_icon_info();
            }
    	}
    }


    return 0;
}

void JYBackActivitySys::clear_back_act_expire(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    std::vector<int> remove_id_list;
    JYBackActivityItem *act_item = NULL;
    for (JYBackActivitySys::ActivityItemIDMap::iterator iter = this->id_to_act_item_map_->begin();
            iter != this->id_to_act_item_map_->end(); ++iter)
    {
        act_item = iter->second;
        if (act_item->__act_end <= nowtime)
        {
        	remove_id_list.push_back(act_item->__act_id);
        	if (this->is_expire_send_mail_act(act_item->__second_type))
        	{
        		JYBackActivityItem *prev_act_item = this->find_expire_act_item_by_second_type(act_item->__second_type);
        		if (prev_act_item != NULL)
        			LOGIC_MONITOR->jyback_activity_item_pool()->push(prev_act_item);
        		(*(this->expire_second_type_act_item_map_))[act_item->__second_type] = act_item;
        	}
        	else
        	{
        		LOGIC_MONITOR->jyback_activity_item_pool()->push(act_item);
        	}
        }
    }
    if (remove_id_list.size() > 0)
    {
        this->first_type_to_act_item_map_->clear();
        this->second_type_to_act_item_map_->clear();

        for (std::vector<int>::iterator iter = remove_id_list.begin();
                iter != remove_id_list.end(); ++iter)
        {
            this->id_to_act_item_map_->erase(*iter);
        }
        for (JYBackActivitySys::ActivityItemIDMap::iterator iter = this->id_to_act_item_map_->begin();
                iter != this->id_to_act_item_map_->end(); ++iter)
        {
            act_item = iter->second;
            (*(this->first_type_to_act_item_map_))[act_item->__first_type].push_back(act_item);
            (*(this->second_type_to_act_item_map_))[act_item->__second_type].push_back(act_item);
        }

        for (JYBackActivitySys::TypeActivityItemMap::iterator iter = this->first_type_to_act_item_map_->begin();
                iter != this->first_type_to_act_item_map_->end(); ++iter)
        {
            ActivityItemList &act_list = iter->second;
            std::sort(act_list.begin(), act_list.end(), JYBackActivityItemCmp);
        }

    	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
    	for (LogicMonitor::PlayerMap::iterator player_iter = player_map.begin(); player_iter != player_map.end(); ++player_iter)
    	{
    		LogicPlayer *player = player_iter->second;
    		if (player != NULL)
    			player->notify_activity_icon_info();
    	}
    }
}

void JYBackActivitySys::remove_act_item(JYBackActivityItem *act_item)
{
	if (this->id_to_act_item_map_->find(act_item->__act_id) != this->id_to_act_item_map_->end())
	{
		{
			TypeActivityItemMap::iterator iter = this->first_type_to_act_item_map_->find(act_item->__first_type);
			if (iter != this->first_type_to_act_item_map_->end())
			{
				ActivityItemList &act_item_list = iter->second;
				for (ActivityItemList::iterator act_item_list_iter = act_item_list.begin();
						act_item_list_iter != act_item_list.end(); ++act_item_list_iter)
				{
					if ((*act_item_list_iter)->__act_id == act_item->__act_id)
					{
						act_item_list.erase(act_item_list_iter);
						break;
					}
				}
			}
		}
		{
			TypeActivityItemMap::iterator iter = this->second_type_to_act_item_map_->find(act_item->__second_type);
			if (iter != this->second_type_to_act_item_map_->end())
			{
				ActivityItemList &act_item_list = iter->second;
				for (ActivityItemList::iterator act_item_list_iter = act_item_list.begin();
						act_item_list_iter != act_item_list.end(); ++act_item_list_iter)
				{
					if ((*act_item_list_iter)->__act_id == act_item->__act_id)
					{
						act_item_list.erase(act_item_list_iter);
						break;
					}
				}
			}
		}

		this->id_to_act_item_map_->erase(act_item->__act_id);

		MSG_USER("remove act_item %d %d %d %d %d", act_item->__act_id, act_item->__first_type, act_item->__second_type,
				act_item->__act_start.sec(), act_item->__act_end.sec());

		LOGIC_MONITOR->jyback_activity_item_pool()->push(act_item);
	}
}

int JYBackActivitySys::send_travel_recharge_rank_reward_mail(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400513*, request, -1);
	JYBackActivitySys::ActivityItemList *act_item_list = NULL;
	JYBackActivityItem *act_item = NULL;
	bool is_expire_act = false;
	if (request->type() == TrvlRechargeMonitor::TYPE_BACK_RECHAGE)
	{
		act_item = this->find_expire_act_item_by_second_type(JYBackActivityItem::STYPE_TRAVEL_RECHARGE_RANK);
		if (act_item == NULL)
		{
			act_item_list = this->find_act_items_by_second_type(JYBackActivityItem::STYPE_TRAVEL_RECHARGE_RANK);
			JUDGE_RETURN(act_item_list != NULL && act_item_list->size() > 0, 0);
			act_item = (*act_item_list)[0];
		}
		else
		{
			is_expire_act = true;
			this->expire_second_type_act_item_map_->erase(JYBackActivityItem::STYPE_TRAVEL_RECHARGE_RANK);
		}
	}
	JUDGE_RETURN(act_item != NULL, 0);
	JUDGE_RETURN(act_item->__is_open == 1, 0);

	for (int i = 0; i < request->obj_size(); ++i)
	{
		const ProtoThreeObj &obj = request->obj(i);
		Int64 role_id = obj.id();
		int rank = obj.value();

		MSG_USER("travel back rank reward %ld %d", role_id, rank);

		JYBackActivityItem::RewardList &reward_list = act_item->__reward_list;
		for (JYBackActivityItem::RewardList::iterator reward_iter = reward_list.begin(); reward_iter != reward_list.end(); ++reward_iter)
		{
			JYBackActivityItem::Reward &reward_info = *reward_iter;
			int min_rank = 0, max_rank = 99999999;
			if (reward_info.__cond_list.size() > 0)
				min_rank = reward_info.__cond_list[0];
			if (reward_info.__cond_list.size() > 1)
				max_rank = reward_info.__cond_list[1];
			JUDGE_CONTINUE(min_rank <= rank && rank <= max_rank);

			MSG_USER("travel back rank has reward %ld %d range(%d %d) %d", role_id, rank, min_rank, max_rank, obj.tick());

			MailInformation *mail_info = GameCommon::create_sys_mail(act_item->__reward_mail_title, act_item->__reward_mail_content, ADD_FROM_BACK_ACTIVITY);
			JYBackActivityItem::ItemList &item_list = reward_info.__reward_item;
			for (JYBackActivityItem::ItemList::iterator item_iter = item_list.begin(); item_iter != item_list.end(); ++item_iter)
			{
				ItemObj &item_obj = *item_iter;
				mail_info->add_goods(item_obj.id_, item_obj.amount_, item_obj.amount_);
			}
			if (reward_info.__restore_gold_rate > 0)
			{
				int bind_gold_amount = obj.tick() * (reward_info.__restore_gold_rate / 100.0);
				if (bind_gold_amount > 0)
					mail_info->add_money(GameEnum::ITEM_MONEY_BIND_GOLD, bind_gold_amount);
			}
			GameCommon::request_save_mail(role_id, mail_info);
		}
	}

	if (is_expire_act == true)
		LOGIC_MONITOR->jyback_activity_item_pool()->push(act_item);
	return 0;
}

bool JYBackActivitySys::is_expire_send_mail_act(const int second_type)
{
	if (this->is_rank_act(second_type))
		return true;
	return false;
}

bool JYBackActivitySys::is_rank_act(const int second_type)
{
	if (second_type == JYBackActivityItem::STYPE_SINGLE_RECHARGE_RANK ||
			second_type == JYBackActivityItem::STYPE_TRAVEL_RECHARGE_RANK)
	{
		return true;
	}
	return false;
}
