/*
 * File Name: LogicBackActivityer.cpp
 * 
 * Created on: 2017-04-06 15:13:41
 * Author: glendy
 * 
 * Last Modified: 2017-04-13 17:50:06
 * Description: 
 */

#include "LogicBackActivityer.h"
#include "JYBackActivitySys.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "TrvlRechargeMonitor.h"
#include "LogicPlayer.h"

LogicBackActivityer::~LogicBackActivityer(void)
{ /*NULL*/ }

void LogicBackActivityer::reset_back_activity(void)
{
    this->player_act_record_map_.clear();
    this->reward_first_type_set_.clear();
    this->reward_act_id_set_.clear();
    this->back_act_draw_op_tick_ = Time_Value::zero;
}

void LogicBackActivityer::check_finish_back_act_every_day(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    std::vector<int> remove_id_list;
    for (BackActivityRecordMap::iterator record_iter = this->player_act_record_map_.begin();
            record_iter != this->player_act_record_map_.end(); ++record_iter)
    {
        int act_id = record_iter->first;
        JYBackActivityPlayerRecord *player_act_record = &(record_iter->second);

        if (player_act_record->__act_end > nowtime.sec())
        {
            JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(act_id);
            if (act_item != NULL && act_item->__update_tick != player_act_record->__update_tick)
            {
                this->init_player_act_record_by_act_item(act_item, player_act_record);
            }
            if (player_act_record->is_has_reward())
            {
            	this->reward_act_id_set_.insert(player_act_record->__act_id);
            	if (act_item != NULL)
            		this->reward_first_type_set_.insert(act_item->__first_type);
            }
            continue;
        }

        if (nowtime.sec() <= player_act_record->__act_end + (3600 * 24 * 7))
        {
            // 七天内登录可以通过邮件发放奖励
            this->send_back_act_reward_mail(player_act_record);
        }

        player_act_record->__reward_value_map.clear();
        remove_id_list.push_back(act_id);

        this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_BACK_ACTIVITY);
    }

    for (std::vector<int>::iterator iter = remove_id_list.begin();
            iter != remove_id_list.end(); ++iter)
    {
        this->player_act_record_map_.erase(*iter);
    }

    if (remove_id_list.size() > 0)
    	this->logic_player()->request_save_player();
}

int LogicBackActivityer::fetch_back_activity_list(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto10100217 *, request, msg, -1);

    int first_type = request->first_type();
    
    JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_first_type(first_type);
    CONDITION_NOTIFY_RETURN(act_list != NULL, RETURN_BACK_ACT_LIST, ERROR_ACTIVITY_NO_OPEN);
 
    Proto50100217 respond;
    respond.set_first_type(first_type);

    Time_Value nowtime = Time_Value::gettimeofday();
    
    for (JYBackActivitySys::ActivityItemList::iterator iter = act_list->begin();
            iter != act_list->end(); ++iter)
    {
        JYBackActivityItem *act_item = *iter;
        JUDGE_CONTINUE(act_item != NULL);
        JUDGE_CONTINUE(act_item->__act_start <= nowtime && nowtime <= act_item->__act_end);

        ProtoBackActName *proto_act_name = respond.add_act_name_list();
        proto_act_name->set_act_id(act_item->__act_id);
        proto_act_name->set_act_title(act_item->__act_title);
        if (this->reward_act_id_set_.find(act_item->__act_id) != this->reward_act_id_set_.end())
        	proto_act_name->set_has_reward(1);
    }

    return this->respond_to_client(RETURN_BACK_ACT_LIST, &respond);
}

int LogicBackActivityer::fetch_single_back_activity_info(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto10100218 *, request, msg, -1);

    int act_id = request->act_id();
    JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(act_id);
    CONDITION_NOTIFY_RETURN(act_item != NULL, RETURN_SINGLE_BACK_ACT_INFO, ERROR_ACTIVITY_NO_OPEN);

    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(act_item->__act_start <= nowtime && nowtime <= act_item->__act_end, RETURN_SINGLE_BACK_ACT_INFO, ERROR_ACTIVITY_NO_OPEN);

    Proto50100218 respond;
    respond.set_act_id(act_item->__act_id);
    respond.set_system_tick(::time(NULL));
    act_item->serialize(respond.mutable_act_info());
    ProtoBackActInfo *proto_act_info = respond.mutable_act_info();

    JYBackActivityPlayerRecord *player_act_record = this->find_player_act_record(act_id);
    if (player_act_record != NULL)
    {
        for (int i = 0; i < proto_act_info->reward_list_size(); ++i)
        {
            JYBackActivityPlayerRecord::IndexRewardMap::iterator idx_reward_iter = player_act_record->__reward_value_map.find(i);
            if (idx_reward_iter != player_act_record->__reward_value_map.end())
            {
            	int reward_amount = 0, drawed_amount = 0;
                JYBackActivityPlayerRecord::RewardMap &reward_map = idx_reward_iter->second;
                for (JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.begin();
                        reward_iter != reward_map.end(); ++reward_iter)
                {
                    if (reward_iter->second == JYBackActivityItem::UNDRAW)
                    {
                        ++reward_amount;
                    }
                    else if (reward_iter->second == JYBackActivityItem::DRAWED)
                    {
                    	++drawed_amount;
                    }
                }
                proto_act_info->mutable_reward_list(i)->set_reward_amount(this->fetch_reward_amount_state(act_item, reward_amount, drawed_amount));
            }
        }
    }

    if (act_item->is_recharge_activity())
    	respond.set_accu_recharge(this->fetch_accum_recharge_gold(act_item));
    if (act_item->is_consum_activity())
    	respond.set_accu_consum(this->fetch_accum_consum(act_item));

    return this->respond_to_client(RETURN_SINGLE_BACK_ACT_INFO, &respond);
}

int LogicBackActivityer::find_reward_item_list(const int act_id, const int reward_id, JYBackActivityItem::ItemList &reward_item_list, IntSet &reward_value)
{
    JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(act_id);
    JUDGE_RETURN(act_item != NULL, ERROR_ACTIVITY_NO_OPEN);
 
    JYBackActivityPlayerRecord *player_act_record = this->find_player_act_record(act_id);
    JUDGE_RETURN(player_act_record != NULL, ERROR_ACT_NO_REWARD);


    JYBackActivityPlayerRecord::IndexRewardMap::iterator reward_iter = player_act_record->__reward_value_map.find(reward_id);
    JUDGE_RETURN(reward_iter != player_act_record->__reward_value_map.end(), ERROR_ACT_NO_REWARD);

    JYBackActivityPlayerRecord::RewardMap &reward_map = reward_iter->second;
    for (JYBackActivityPlayerRecord::RewardMap::iterator iter = reward_map.begin();
            iter != reward_map.end(); ++iter)
    {
        if (iter->second == JYBackActivityItem::UNDRAW)
        {
            reward_value.insert(iter->first);
            if (0 <= reward_id && reward_id < int(act_item->__reward_list.size()))
            {
            	JYBackActivityItem::ItemList &reward_items = act_item->__reward_list[reward_id].__reward_item;
            	for (JYBackActivityItem::ItemList::iterator reward_iter = reward_items.begin(); reward_iter != reward_items.end(); ++reward_iter)
            		reward_item_list.push_back(*reward_iter);
            }
        }
    }
    return 0;
}


int LogicBackActivityer::fetch_reward_item_list(const int act_id, const int reward_id, IntMap &bind_item_map, IntMap &unbind_item_map, IntSet &reward_value)
{
    JYBackActivityItem::ItemList reward_item_list;
    int ret = this->find_reward_item_list(act_id, reward_id, reward_item_list, reward_value);
    JUDGE_RETURN(ret == 0, ret);
    JUDGE_RETURN(reward_item_list.size() > 0, ERROR_ACT_NO_REWARD);

    for (JYBackActivityItem::ItemList::iterator iter = reward_item_list.begin();
            iter != reward_item_list.end(); ++iter)
    {
        ItemObj &item_obj = *iter;
        if (item_obj.bind_ == GameEnum::ITEM_BIND)
            bind_item_map[item_obj.id_] += item_obj.amount_;
        else
            unbind_item_map[item_obj.id_] += item_obj.amount_;
    }
    return 0;
}

int LogicBackActivityer::draw_single_back_activity_reward(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto10100219 *, request, msg, -1);
    int act_id = request->act_id(), reward_id = request->reward_id();

    JYBackActivityPlayerRecord *player_act_record = this->find_player_act_record(act_id);
    CONDITION_NOTIFY_RETURN(player_act_record != NULL, RETURN_DRAW_SINGLE_BACK_ACT, ERROR_ACT_NO_REWARD);

    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(player_act_record->__act_start <= nowtime.sec() && nowtime.sec() < (player_act_record->__act_end - 1), RETURN_DRAW_SINGLE_BACK_ACT, ERROR_ACT_NO_REWARD);
    CONDITION_NOTIFY_RETURN(this->back_act_draw_op_tick_ < nowtime, RETURN_DRAW_SINGLE_BACK_ACT, ERROR_OPERATE_TOO_FAST);

    Proto31400054 inner_req;
    inner_req.set_act_id(act_id);

    IntMap unbind_item_map, bind_item_map;
    if (reward_id >= 0)
    {
        // 领取单个条件项奖励
        IntSet reward_value;
        int ret = this->fetch_reward_item_list(act_id, reward_id, bind_item_map, unbind_item_map, reward_value);
        CONDITION_NOTIFY_RETURN(ret == 0, RETURN_DRAW_SINGLE_BACK_ACT, ret);

        ProtoBackActRewardIndex *proto_reward_index = inner_req.add_reward_index_list();
        proto_reward_index->set_reward_id(reward_id);
        for (IntSet::iterator set_iter = reward_value.begin(); set_iter != reward_value.end(); ++set_iter)
        	proto_reward_index->add_reward_value(*set_iter);
    }
    else
    {
        // 领取所有条件项奖励
        for (JYBackActivityPlayerRecord::IndexRewardMap::iterator reward_iter = player_act_record->__reward_value_map.begin();
                reward_iter != player_act_record->__reward_value_map.end(); ++reward_iter)
        {
        	IntSet reward_value;
            reward_id = reward_iter->first;
            int ret = this->fetch_reward_item_list(act_id, reward_id, bind_item_map, unbind_item_map, reward_value);
            JUDGE_CONTINUE(ret == 0);

            ProtoBackActRewardIndex *proto_reward_index = inner_req.add_reward_index_list();
            proto_reward_index->set_reward_id(reward_id);
            for (IntSet::iterator set_iter = reward_value.begin(); set_iter != reward_value.end(); ++set_iter)
            	proto_reward_index->add_reward_value(*set_iter);
        }
    }

    CONDITION_NOTIFY_RETURN(bind_item_map.size() > 0 || unbind_item_map.size() > 0, RETURN_DRAW_SINGLE_BACK_ACT, ERROR_ACT_NO_REWARD);
    
    this->back_act_draw_op_tick_ = nowtime + Time_Value(1);

    for (IntMap::iterator iter = bind_item_map.begin(); iter != bind_item_map.end(); ++iter)
    {
        ProtoItem *proto_item = inner_req.add_bind_item_list();
        proto_item->set_id(iter->first);
        proto_item->set_amount(iter->second);
        proto_item->set_bind(GameEnum::ITEM_BIND);
    }
    for (IntMap::iterator iter = unbind_item_map.begin(); iter != unbind_item_map.end(); ++iter)
    {
        ProtoItem *proto_item = inner_req.add_unbind_item_list();
        proto_item->set_id(iter->first);
        proto_item->set_amount(iter->second);
        proto_item->set_bind(GameEnum::ITEM_NO_BIND);
    }

    return this->dispatch_to_map_server(&inner_req);
}

int LogicBackActivityer::notify_activity_icon_info(void)
{
    Proto80100201 respond;

    IntSet first_type_set;
    BACK_ACTIVITY_SYS->fetch_back_act_type_list(first_type_set);
    for (IntSet::iterator iter = first_type_set.begin(); iter != first_type_set.end(); ++iter)
    {
    	JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_first_type(*iter);
    	JUDGE_CONTINUE(act_list != NULL && act_list->size() > 0);
        respond.add_show_type(*iter);
    }
    for (IntSet::iterator iter = this->reward_first_type_set_.begin(); iter != this->reward_first_type_set_.end(); ++iter)
    {
    	JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_first_type(*iter);
    	JUDGE_CONTINUE(act_list != NULL && act_list->size() > 0);
    	respond.add_reward_show_type(*iter);
    }
    return this->respond_to_client(ACTIVE_BACK_ACT_ICON, &respond);
}

JYBackActivityPlayerRecord *LogicBackActivityer::find_player_act_record(const int act_id)
{
    BackActivityRecordMap::iterator iter = this->player_act_record_map_.find(act_id);
    if (iter != this->player_act_record_map_.end())
        return &(iter->second);
    return NULL;
}

int LogicBackActivityer::process_back_act_reward_state_after_insert(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400054 *, request, msg, -1);

    int act_id = request->act_id();
    JYBackActivityPlayerRecord *player_act_record = this->find_player_act_record(act_id);
    JUDGE_RETURN(player_act_record != NULL, act_id);

    for (int i = 0; i < request->reward_index_list_size(); ++i)
    {
        const ProtoBackActRewardIndex &proto_reward_index = request->reward_index_list(i);
        int reward_id = proto_reward_index.reward_id();

        JYBackActivityPlayerRecord::IndexRewardMap::iterator reward_iter = player_act_record->__reward_value_map.find(reward_id);
        JUDGE_CONTINUE(reward_iter != player_act_record->__reward_value_map.end());

        JYBackActivityPlayerRecord::RewardMap &reward_map = reward_iter->second;

        for (int j = 0; j < proto_reward_index.reward_value_size(); ++j)
        {
            int reward_value = proto_reward_index.reward_value(j);
            JYBackActivityPlayerRecord::RewardMap::iterator value_iter = reward_map.find(reward_value);
            JUDGE_CONTINUE(value_iter != reward_map.end());
            value_iter->second = JYBackActivityItem::DRAWED;
        }
    }

    this->update_single_cond_value_after_draw(player_act_record);

    this->respond_to_client(RETURN_DRAW_SINGLE_BACK_ACT);

    if (request->act_id() > 0)
    {
//    	JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(request->act_id());
//    	if (act_item != NULL)
//    	{
//    		Proto10100217 req;
//    		req.set_first_type(act_item->__first_type);
//    		this->fetch_back_activity_list(&req);
//    	}
    	{
    		Proto10100218 req;
    		req.set_act_id(request->act_id());
    		this->fetch_single_back_activity_info(&req);
    	}
    }
    return 0;
}

void LogicBackActivityer::update_back_act_accu_recharge(const int daily_gold)
{
    JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_second_type(JYBackActivityItem::STYPE_ACCU_RECHARGE);

    this->update_back_act_record_by_daily_value(act_list, daily_gold);
}

void LogicBackActivityer::update_back_act_repeat_recharge(const int daily_gold)
{
    JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_second_type(JYBackActivityItem::STYPE_REPEAT_RECHARGE);

    this->update_back_act_record_by_daily_value(act_list, daily_gold);
}

void LogicBackActivityer::update_back_act_accu_consum(const int daily_consum)
{
    JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_second_type(JYBackActivityItem::STYPE_ACCU_CONSUM);

    this->update_back_act_record_by_inc_value(act_list, daily_consum);
}

void LogicBackActivityer::update_back_act_travel_recharge_rank(const int cur_gold)
{
    JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_second_type(JYBackActivityItem::STYPE_TRAVEL_RECHARGE_RANK);
    JUDGE_RETURN(act_list != NULL, ;);

    Time_Value nowtime = Time_Value::gettimeofday();
    int activity_id = 0;
    for (JYBackActivitySys::ActivityItemList::iterator act_list_iter = act_list->begin(); act_list_iter != act_list->end(); ++act_list_iter)
    {
    	JYBackActivityItem *act_item = *act_list_iter;
    	JUDGE_CONTINUE(act_item->__act_start <= nowtime && nowtime < act_item->__act_end);
//    	if (act_item->__need_gold > 0 && cur_gold < act_item->__need_gold)
//    		return ;

    	activity_id = act_item->__act_id;
//        JYBackActivityPlayerRecord *player_act_record = NULL;
//        BackActivityRecordMap::iterator player_rec_iter = this->player_act_record_map_.find(act_item->__act_id);
//        if (player_rec_iter == this->player_act_record_map_.end())
//        {
//            player_act_record = &(this->player_act_record_map_[act_item->__act_id]);
//            player_act_record->reset();
//            this->init_player_act_record_by_act_item(act_item, player_act_record);
//        }
//        else if (player_rec_iter->second.__update_tick != act_item->__update_tick)
//        {
//        	player_act_record->reset();
//        	this->init_player_act_record_by_act_item(act_item, player_act_record);
//        }
        break;
    }
    JUDGE_RETURN(activity_id > 0, ;);

	Proto30400516 inner;
	inner.set_type(TrvlRechargeMonitor::TYPE_BACK_RECHAGE);
	inner.set_amount(cur_gold);
	inner.set_activity_id(activity_id);

	JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(activity_id);
	if (act_item != NULL)
		act_item->serialize(inner.mutable_act_info());

	ProtoServer* server = inner.mutable_server_info();
	ProtoTeamer* teamer = inner.mutable_self_info();

	this->role_detail().serialize(server, true);
	this->role_detail().make_up_teamer_info(this->role_id(), teamer);

	LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_RECHARGE_SCENE_ID, &inner);
}

void LogicBackActivityer::refresh_back_act_record_daily_value(JYBackActivityPlayerRecord *player_act_record)
{
    JUDGE_RETURN(player_act_record != NULL, ;);

    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(player_act_record->__daily_refresh_tick <= nowtime.sec(), ;);

    player_act_record->__daily_refresh_tick = next_day(0, 0, nowtime).sec();
    player_act_record->__daily_value = 0;
}

void LogicBackActivityer::check_back_act_reward(JYBackActivityItem *act_item, JYBackActivityPlayerRecord *player_act_record)
{
    int reward_id = 0;
    for (JYBackActivityItem::RewardList::iterator reward_iter = act_item->__reward_list.begin();
            reward_iter != act_item->__reward_list.end(); ++reward_iter)
    {
        JYBackActivityItem::Reward &reward_info = *reward_iter;
        this->check_and_update_reward_map(reward_id++, reward_info, player_act_record);
    }

    if (player_act_record->is_has_reward())
    {
    	this->reward_act_id_set_.insert(player_act_record->__act_id);
    	if (act_item != NULL)
    		this->reward_first_type_set_.insert(act_item->__first_type);
    	this->notify_activity_icon_info();
    }
}

void LogicBackActivityer::check_and_update_reward_map(const int reward_id, JYBackActivityItem::Reward &reward_info, JYBackActivityPlayerRecord *player_act_record)
{
    if (reward_info.__cond_type == JYBackActivityItem::CONDTYPE_SINGLE_EQUAL)
    {
        // 等值比较
        int check_value = 999999999;
        if (reward_info.__cond_list.size() > 0)
            check_value = reward_info.__cond_list[0];

        JYBackActivityPlayerRecord::RewardMap &reward_map = player_act_record->__reward_value_map[reward_id];
        JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.find(check_value);
        if (reward_iter != reward_map.end() && reward_iter->second != JYBackActivityItem::CNDRAW)
            return ;

        JUDGE_RETURN(player_act_record->__single_cond_value == check_value, ;);
        reward_map[check_value] = JYBackActivityItem::UNDRAW;
    }
    else if (reward_info.__cond_type == JYBackActivityItem::CONDTYPE_SINGLE_GTE)
    {
        // 大于等于比较
        int check_value = 999999999;
        if (reward_info.__cond_list.size() > 0)
            check_value = reward_info.__cond_list[0];

        JYBackActivityPlayerRecord::RewardMap &reward_map = player_act_record->__reward_value_map[reward_id];
        JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.find(check_value);
        if (reward_iter != reward_map.end() && reward_iter->second != JYBackActivityItem::CNDRAW)
            return ;

        JUDGE_RETURN(player_act_record->__single_cond_value >= check_value, ;);
        reward_map[check_value] = JYBackActivityItem::UNDRAW;
    }
    else if (reward_info.__cond_type == JYBackActivityItem::CONDTYPE_SINGLE_LTE)
    {
        // 小于等于比较
        int check_value = 999999999;
        if (reward_info.__cond_list.size() > 0)
            check_value = reward_info.__cond_list[0];

        JYBackActivityPlayerRecord::RewardMap &reward_map = player_act_record->__reward_value_map[reward_id];
        JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.find(check_value);
        if (reward_iter != reward_map.end() && reward_iter->second != JYBackActivityItem::CNDRAW)
            return ;

        JUDGE_RETURN(player_act_record->__single_cond_value <= check_value, ;);
        reward_map[check_value] = JYBackActivityItem::UNDRAW;
    }
    else if (reward_info.__cond_type == JYBackActivityItem::CONDTYPE_RANGE)
    {
        // 范围比较
        int min_check_value = 999999999, max_check_value = 999999999;
        if (reward_info.__cond_list.size() > 0)
            min_check_value = reward_info.__cond_list[0];
        if (reward_info.__cond_list.size() > 1)
            max_check_value = reward_info.__cond_list[1];

        JYBackActivityPlayerRecord::RewardMap &reward_map = player_act_record->__reward_value_map[reward_id];
        JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.find(min_check_value);
        if (reward_iter != reward_map.end() && reward_iter->second != JYBackActivityItem::CNDRAW)
            return ;

        JUDGE_RETURN(min_check_value <= player_act_record->__single_cond_value && player_act_record->__single_cond_value <= max_check_value, ;);
        reward_map[min_check_value] = JYBackActivityItem::UNDRAW;
    }
    else if (reward_info.__cond_type == JYBackActivityItem::CONDTYPE_SINGLE_LIST)
    {
        // 等值列表比较
        for (IntVec::iterator cond_iter = reward_info.__cond_list.begin(); cond_iter != reward_info.__cond_list.end(); ++cond_iter)
        {
            int check_value = *cond_iter;

            JYBackActivityPlayerRecord::RewardMap &reward_map = player_act_record->__reward_value_map[reward_id];
            JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.find(check_value);
            if (reward_iter != reward_map.end() && reward_iter->second != JYBackActivityItem::CNDRAW)
                continue;

            if (player_act_record->__single_cond_value == check_value)
                reward_map[check_value] = JYBackActivityItem::UNDRAW;
        }
    }
    else if (reward_info.__cond_type == JYBackActivityItem::CONDTYPE_SINGLE_DIVID)
    {
        // 每次整除都可获得
        int check_value = 999999999;
        if (reward_info.__cond_list.size() > 0)
            check_value = reward_info.__cond_list[0];

        if (player_act_record->__single_cond_value > 0)
        {
            int range_size = player_act_record->__single_cond_value / check_value;
            for (int i = 1; i <= range_size; ++i)
            {
                int check_tmp_value = check_value * i;
                JYBackActivityPlayerRecord::RewardMap &reward_map = player_act_record->__reward_value_map[reward_id];
                JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.find(check_tmp_value);
                if (reward_iter != reward_map.end() && reward_iter->second != JYBackActivityItem::CNDRAW)
                    continue;
            
                reward_map[check_tmp_value] = JYBackActivityItem::UNDRAW;
            }
        }
    }
}

void LogicBackActivityer::update_back_act_record_by_daily_value(ActivityItemList *act_list, const int daily_value)
{
    JUDGE_RETURN(act_list != NULL, ;);

    Time_Value nowtime = Time_Value::gettimeofday();
    for (ActivityItemList::iterator iter = act_list->begin(); iter != act_list->end(); ++iter)
    {
        JYBackActivityItem *act_item = *iter;
        JUDGE_CONTINUE(act_item != NULL);
        JUDGE_CONTINUE(act_item->__act_start <= nowtime && nowtime <= act_item->__act_end);

        JYBackActivityPlayerRecord *player_act_record = NULL;
        BackActivityRecordMap::iterator player_rec_iter = this->player_act_record_map_.find(act_item->__act_id);
        if (player_rec_iter == this->player_act_record_map_.end())
        {
            player_act_record = &(this->player_act_record_map_[act_item->__act_id]);
            player_act_record->reset();
            this->init_player_act_record_by_act_item(act_item, player_act_record);
        }
        else
        {
            player_act_record = &(player_rec_iter->second);
        }

        // 处理活动结束后新开活动ID重复时，先发放当前未发放的奖励，并重新初始化player_act_record
        this->check_and_finish_last_activity_record(player_act_record);

        // 重新更新奖励的道具列表
        if (player_act_record->__update_tick != act_item->__update_tick)
            this->init_player_act_record_by_act_item(act_item, player_act_record);

        this->refresh_back_act_record_daily_value(player_act_record);

        if (player_act_record->__daily_value < daily_value)
        {
            player_act_record->__single_cond_value += (daily_value - player_act_record->__daily_value);
            player_act_record->__daily_value = daily_value;
        }

        this->check_back_act_reward(act_item, player_act_record);

        this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_BACK_ACTIVITY);
    }
}

void LogicBackActivityer::update_back_act_record_by_inc_value(ActivityItemList *act_list, const int inc_value)
{
    JUDGE_RETURN(act_list != NULL, ;);

    Time_Value nowtime = Time_Value::gettimeofday();
    for (ActivityItemList::iterator iter = act_list->begin(); iter != act_list->end(); ++iter)
    {
        JYBackActivityItem *act_item = *iter;
        JUDGE_CONTINUE(act_item != NULL);
        JUDGE_CONTINUE(act_item->__act_start <= nowtime && nowtime <= act_item->__act_end);

        JYBackActivityPlayerRecord *player_act_record = NULL;
        BackActivityRecordMap::iterator player_rec_iter = this->player_act_record_map_.find(act_item->__act_id);
        if (player_rec_iter == this->player_act_record_map_.end())
        {
            player_act_record = &(this->player_act_record_map_[act_item->__act_id]);
            player_act_record->reset();
            this->init_player_act_record_by_act_item(act_item, player_act_record);
        }
        else
        {
            player_act_record = &(player_rec_iter->second);
        }

        // 处理活动结束后新开活动ID重复时，先发放当前未发放的奖励，并重新初始化player_act_record
        this->check_and_finish_last_activity_record(player_act_record);

        // 重新更新奖励的道具列表
        if (player_act_record->__update_tick != act_item->__update_tick)
            this->init_player_act_record_by_act_item(act_item, player_act_record);

        this->refresh_back_act_record_daily_value(player_act_record);

        player_act_record->__single_cond_value += inc_value;
        player_act_record->__daily_value += inc_value;

        this->check_back_act_reward(act_item, player_act_record);

        this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_BACK_ACTIVITY);
    }
}

void LogicBackActivityer::update_single_cond_value_after_draw(JYBackActivityPlayerRecord *player_act_record)
{
	JUDGE_RETURN(player_act_record != NULL, ;);
	JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(player_act_record->__act_id);

	// 处理红点
	if (player_act_record->is_has_reward() == false)
	{
		this->reward_act_id_set_.erase(player_act_record->__act_id);
		if (act_item != NULL)
		{
			this->reward_first_type_set_.erase(act_item->__first_type);

			JYBackActivitySys::ActivityItemList *act_list = BACK_ACTIVITY_SYS->find_act_items_by_first_type(act_item->__first_type);
			if (act_list != NULL)
			{
				for (JYBackActivitySys::ActivityItemList::iterator act_list_iter = act_list->begin(); act_list_iter != act_list->end(); ++act_list_iter)
				{
					JYBackActivityItem *tmp_act_item = *act_list_iter;
					if (this->reward_act_id_set_.find(tmp_act_item->__act_id) != this->reward_act_id_set_.end())
					{
						this->reward_first_type_set_.insert(tmp_act_item->__first_type);
						break;
					}
				}
			}
		}
		this->notify_activity_icon_info();
	}

	JUDGE_RETURN(act_item != NULL, ; );

	if (act_item->__second_type == JYBackActivityItem::STYPE_REPEAT_RECHARGE)
	{
		int cond_value = 1;
		if (act_item->__reward_list.size() > 0 && act_item->__reward_list[0].__cond_list.size() > 0)
			cond_value = act_item->__reward_list[0].__cond_list[0];
		if (cond_value <= 0)
			cond_value = player_act_record->__single_cond_value + 1;
		player_act_record->__single_cond_value = player_act_record->__single_cond_value % cond_value;
		player_act_record->__reward_value_map.clear();
	}
}

void LogicBackActivityer::init_player_act_record_by_act_item(JYBackActivityItem *act_item, JYBackActivityPlayerRecord *player_act_record)
{
    player_act_record->__act_id = act_item->__act_id;
    player_act_record->__act_start = act_item->__act_start.sec();
    player_act_record->__act_end = act_item->__act_end.sec();
    player_act_record->__mail_title = act_item->__reward_mail_title;
    player_act_record->__mail_content = act_item->__reward_mail_content;
    player_act_record->__update_tick = act_item->__update_tick;

    player_act_record->__index_reward_item_map.clear();
    int reward_id = 0;
    for (JYBackActivityItem::RewardList::iterator reward_list_iter = act_item->__reward_list.begin(); reward_list_iter != act_item->__reward_list.end(); ++reward_list_iter)
    {
        JYBackActivityItem::Reward &reward_info = *reward_list_iter;
        player_act_record->__index_reward_item_map[reward_id++] = reward_info.__reward_item;
    }
}

void LogicBackActivityer::check_and_finish_last_activity_record(JYBackActivityPlayerRecord *player_act_record)
{
    JUDGE_RETURN(player_act_record != NULL, ;);
    Time_Value nowtime = Time_Value::gettimeofday();
    if (player_act_record->__act_end <= nowtime.sec() && nowtime.sec() <= player_act_record->__act_end + (3600 * 24 * 7))
    {
        // 邮件发放上一次奖励
        this->send_back_act_reward_mail(player_act_record);

        player_act_record->__reward_value_map.clear();
    }

    JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(player_act_record->__act_id);
    if (act_item != NULL && (act_item->__act_start.sec() != player_act_record->__act_start ||
                act_item->__act_end.sec() != player_act_record->__act_end))
    {
        if (player_act_record->__act_end <= nowtime.sec())
        {
            // 上一次已结束，把所有记录清空重新初始化
            player_act_record->reset();
        }
        this->init_player_act_record_by_act_item(act_item, player_act_record);
    }
}

void LogicBackActivityer::send_back_act_reward_mail(JYBackActivityPlayerRecord *player_act_record)
{
    IntMap item_map;
    for (JYBackActivityPlayerRecord::IndexRewardMap::iterator index_reward_iter = player_act_record->__reward_value_map.begin();
            index_reward_iter != player_act_record->__reward_value_map.end(); ++index_reward_iter)
    {
        int reward_id = index_reward_iter->first;
        JYBackActivityPlayerRecord::IndexRewardItemMap::iterator index_reward_item_iter = player_act_record->__index_reward_item_map.find(reward_id);
        JUDGE_CONTINUE(index_reward_item_iter != player_act_record->__index_reward_item_map.end());
        JYBackActivityPlayerRecord::ItemVec &item_list = index_reward_item_iter->second;

        JYBackActivityPlayerRecord::RewardMap &reward_map = index_reward_iter->second;
        for (JYBackActivityPlayerRecord::RewardMap::iterator reward_iter = reward_map.begin();
                reward_iter != reward_map.end(); ++reward_iter)
        {
            if (reward_iter->second == JYBackActivityItem::UNDRAW)
            {
                for (JYBackActivityPlayerRecord::ItemVec::iterator item_iter = item_list.begin();
                        item_iter != item_list.end(); ++item_iter)
                {
                    ItemObj &item_obj = *item_iter;
                    item_map[item_obj.id_] += item_obj.amount_;
                }
            }
        }
    }
    if (item_map.size() > 0)
    {
        MailInformation *mail_info = GameCommon::create_sys_mail(player_act_record->__mail_title, player_act_record->__mail_content, ADD_FROM_BACK_ACTIVITY);

        for (IntMap::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
        {
            mail_info->add_goods(iter->first, iter->second, GameEnum::ITEM_BIND);
        }
        GameCommon::request_save_mail(this->role_id(), mail_info);
    }
}

int LogicBackActivityer::fetch_accum_recharge_gold(JYBackActivityItem *act_item)
{
	JUDGE_RETURN(act_item != NULL, 0);
    JYBackActivityPlayerRecord *player_act_record = this->find_player_act_record(act_item->__act_id);
    JUDGE_RETURN(player_act_record != NULL, 0);

    this->refresh_back_act_record_daily_value(player_act_record);

    return player_act_record->__single_cond_value;
}

int LogicBackActivityer::fetch_accum_consum(JYBackActivityItem *act_item)
{
	JUDGE_RETURN(act_item != NULL, 0);
    JYBackActivityPlayerRecord *player_act_record = this->find_player_act_record(act_item->__act_id);
    JUDGE_RETURN(player_act_record != NULL, 0);

    this->refresh_back_act_record_daily_value(player_act_record);

    return player_act_record->__single_cond_value;
}

int LogicBackActivityer::fetch_reward_amount_state(JYBackActivityItem *act_item, const int reward_amount, const int drawed_amount)
{
	JUDGE_RETURN(act_item != NULL, reward_amount);
	if (act_item->__second_type == JYBackActivityItem::STYPE_ACCU_RECHARGE ||
			act_item->__second_type == JYBackActivityItem::STYPE_ACCU_CONSUM)
	{
		if (drawed_amount > 0)
			return -1;
		return reward_amount;
	}

	return reward_amount;
}

LogicBackActivityer::BackActivityRecordMap &LogicBackActivityer::back_act_player_rec_map(void)
{
    return this->player_act_record_map_;
}

int LogicBackActivityer::fetch_back_travel_rank_info(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10100222 *, request, msg, -1);

	int act_id = request->act_id();
    JYBackActivityItem *act_item = BACK_ACTIVITY_SYS->find_act_item_by_id(act_id);
    JUDGE_RETURN(act_item != NULL, -1);

	Proto30400515 inner_req;
    inner_req.set_first_type(act_item->__first_type);
    inner_req.set_second_type(act_item->__second_type);
    inner_req.set_page(request->page());
    ProtoBackActInfo *proto_act_info = inner_req.mutable_act_info();
    act_item->serialize(proto_act_info);
    
    return this->monitor()->dispatch_to_scene(this, GameEnum::TRVL_RECHARGE_SCENE_ID, &inner_req);
}
