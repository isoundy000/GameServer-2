/*
 * ActivityStruct.cpp
 *
 *  Created on: Dec 16, 2016
 *      Author: peizhibi
 */

#include "ActivityStruct.h"
#include "BackField.h"
#include "ProtoPublic.pb.h"
#include "ProtoInnerPublic.pb.h"
#include "LogicActivityer.h"
#include "ProtoDefine.h"

#include <mongo/client/dbclient.h>


BackSetActDetail::ActRewardRate::ActRewardRate()
{
	ActRewardRate::reset();
}
void BackSetActDetail::ActRewardRate::reset()
{
	this->contribute = 0;
	this->salary = 0;
	this->join_in = 0;
	this->city_salary = 0;
	this->rank = 0;
	this->wanner = 0;
	this->exploit = 0;
	this->rate = 1;
}
void BackSetActDetail::ActRewardRate::set(int type)
{
	switch(type)
	{
	case REWARD_TYPE_CONTRIBUTE://贡献
	{
		this->contribute = 1;
		break;
	}
	case REWARD_TYPE_SALARY://俸禄
	{
		this->salary = 1;
		break;
	}
	case REWARD_TYPE_JOIN_IN://参与奖
	{
		this->join_in = 1;
		break;
	}
	case REWARD_TYPE_CITY_SALARY://城主方俸禄
	{
		this->city_salary = 1;
		break;
	}
	case REWARD_TYPE_RANK://排名奖励
	{
		this->rank = 1;
		break;
	}
	case REWARD_TYPE_WINNER://胜利方奖励
	{
		this->wanner = 1;
		break;
	}
	case REWARD_TYPE_EXPLOIT://功勋奖励
	{
		this->exploit = 1;
		break;
	}
	}
}

BackSetActDetail::ActItem::ActItem()
{
	this->cur_index_ 	= -1;
	this->brocast_ 		= 0;
	this->times_ 		= 0;
	this->must_reset_ 	= 0;
	this->sub_value_    = 0;
	this->cash_coupon_	= 0;
	this->reward_id_ 	= 0;
	this->reward_type_ 	= 0;
	this->reward_start_cond_ = 0;
	this->exchange_type_ = 0;
	this->handle_type_  = 0;
}

void BackSetActDetail::ActItem::set_cond(const string& cond_str)
{
	StringVec cond_set;
	GameCommon::stl_split(cond_set, cond_str);

	for (StringVec::iterator cond_iter = cond_set.begin();
			cond_iter != cond_set.end(); ++cond_iter)
	{
		string cond_value = *cond_iter;
		JUDGE_CONTINUE(cond_value.empty() == false);
		this->cond_value_.push_back(::atoi(cond_value.c_str()));
	}
}

void BackSetActDetail::ActItem::set_reward(const string& reward_str)
{
	StringVec reward_set;
	GameCommon::stl_split(reward_set, reward_str, ";");

	for (StringVec::iterator reward_iter = reward_set.begin();
			reward_iter != reward_set.end(); ++reward_iter)
	{
		JUDGE_CONTINUE(reward_iter->empty() == false);
		ItemObj obj = BackSetActDetail::str_to_itemobj(*reward_iter);
		this->reward_.push_back(obj);
	}
}

bool BackSetActDetail::ActItem::arrive()
{
	return int(this->sub_map_.size()) >= this->cond_value_[1];
}

bool BackSetActDetail::ActItem::drawed(Int64 role)
{
	return this->sub_map_.count(role) > 0;
}

BackSetActDetail::BackSetActDetail(void)
{
	BackSetActDetail::reset();
}

BackSetActDetail::~BackSetActDetail(void)
{

}

void BackSetActDetail::reset()
{
	this->act_type_item_set_.clear();
	this->act_type_item_set_.reserve(128);
}

void BackSetActDetail::fetch_all_index(IntMap& index_map)
{
	BackSetActDetail::ActTypeItemSet::iterator iter = this->act_type_item_set_.begin();
	for (; iter != this->act_type_item_set_.end(); ++iter)
	{
		index_map[iter->act_index_] = true;
	}
}

int BackSetActDetail::add_new_item(const BSONObj& res)
{
	ActTypeItem act_item;
	this->update_item(&act_item, res);
	this->act_type_item_set_.push_back(act_item);
	return act_item.act_index_;
}

int BackSetActDetail::add_new_item(int index, const Json::Value& conf)
{
	ActTypeItem act_item;
	act_item.act_index_ = index;

	this->update_item(&act_item, conf);
	this->act_type_item_set_.push_back(act_item);

	return act_item.act_index_;
}

int BackSetActDetail::update_item(BackSetActDetail::ActTypeItem* act_item, const BSONObj& res)
{
    BSONObjIterator agent(res.getObjectField(DBBackActivity::AGENT.c_str()));
    act_item->agent_.clear();

    while (agent.more())
    {
    	int id = agent.next().numberInt();
    	act_item->agent_[id] = true;
    }

    act_item->priority_ = res[DBBackActivity::PRIORITY].numberInt();
    act_item->icon_type_ = res[DBBackActivity::ICON_TYPE].numberInt();
    act_item->act_index_ = res[DBBackActivity::ACT_INDEX].numberInt();
    act_item->sort_ = res[DBBackActivity::SORT].numberInt();
    act_item->first_type_ = res[DBBackActivity::FIRST_TYPE].numberInt();
    act_item->second_type_ = res[DBBackActivity::SECOND_TYPE].numberInt();

    act_item->start_tick_ = res[DBBackActivity::START_TICK].numberLong();
	act_item->stop_tick_ = res[DBBackActivity::STOP_TICK].numberLong();
	act_item->update_tick_ = res[DBBackActivity::UPDATE_TICK].numberLong();

	act_item->act_title_ = res[DBBackActivity::ACT_TITLE].str();
	act_item->act_content_ = res[DBBackActivity::ACT_CONTENT].str();

	act_item->mail_title_ = res[DBBackActivity::MAIL_TITLE].str();
	act_item->mail_content_ = res[DBBackActivity::MAIL_CONTENT].str();

	StringVec time_set;
	GameCommon::stl_split(time_set, res[DBBackActivity::OPEN_TIME].str());
	for (StringVec::iterator cond_iter = time_set.begin();
			cond_iter != time_set.end(); ++cond_iter)
	{
		string time_value = *cond_iter;
		JUDGE_CONTINUE(time_value.empty() == false);

		int day = ::atoi(time_value.c_str());
		act_item->open_time_[day] = true;
	}

//	switch(act_item->first_type_)
//	{
//	case BackSetActDetail::ACT_RANK:
//	{
//		act_item->reward_start_ = res[DBBackActivity::REWARD_START].numberLong();
//		act_item->reward_end_ = res[DBBackActivity::REWARD_END].numberLong();
//		break;
//	}
//	}

    BSONObjIterator iter(res.getObjectField(DBBackActivity::REWARD.c_str()));
    while (iter.more())
    {
    	BSONObj obj = iter.next().embeddedObject();

    	ActItem reward_item;
    	reward_item.cur_index_ 	= act_item->act_item_set_.size();
    	reward_item.content_ 	= obj[DBBackActivity::Reward::CONTENT].str();
    	reward_item.brocast_ 	= obj[DBBackActivity::Reward::BROCAST].numberInt();
    	reward_item.times_ 		= obj[DBBackActivity::Reward::TIMES].numberInt();
    	reward_item.set_cond(obj[DBBackActivity::Reward::COND].str());
    	reward_item.set_reward(obj[DBBackActivity::Reward::ITEM].str());
    	act_item->act_item_set_.push_back(reward_item);
    }

    return 0;
}

int BackSetActDetail::update_item(ActTypeItem* act_item, const Json::Value& conf)
{
    act_item->first_type_ 	= conf[DBBackActivity::FIRST_TYPE].asInt();
    act_item->second_type_ 	= conf[DBBackActivity::SECOND_TYPE].asInt();
    act_item->cond_type_ 	= conf[DBBackActivity::COND_TYPE].asInt();
    act_item->start_cond_ 	= conf[DBBackActivity::START_COND].asInt();
    act_item->mail_id_ 		= conf[DBBackActivity::MAIL_ID].asInt();
    act_item->red_event_ 	= conf[DBBackActivity::RED_EVENT].asInt();
    act_item->act_title_ 	= conf[DBBackActivity::ACT_TITLE].asString();
    act_item->limit_		= conf[DBBackActivity::LIMIT].asInt();
    act_item->redraw_ 		= conf[DBBackActivity::REDRAW].asInt();
    act_item->day_clear_ 	= conf[DBBackActivity::DAY_CLEAR].asInt();
    act_item->record_value_ = conf[DBBackActivity::RECORD_VALUE].asInt();

    //设置循环
    int cycle_times = conf[DBBackActivity::CYCLE_TIMES].asInt();
    if (cycle_times > 1)
    {
    	IntVec open_times;
    	GameCommon::json_to_t_vec(open_times, conf[DBBackActivity::OPEN_TIME]);

    	for (int i = 0; i < cycle_times; ++i)
    	{
    		//开启时间 + 循环时间
    		int open_day = open_times[0] + open_times[1] * i;
    		act_item->open_time_[open_day] = true;
    	}
    }
    else
    {
        GameCommon::json_to_t_map(act_item->open_time_, conf[DBBackActivity::OPEN_TIME]);
    }

    //具体每个奖励
    for (uint i = 0; i < conf[DBBackActivity::REWARD].size(); ++i)
    {
    	const Json::Value& reward_conf = conf[DBBackActivity::REWARD][i];

    	ActItem reward_item;
    	reward_item.cur_index_ = i;
    	reward_item.reward_type_ = reward_conf[DBBackActivity::Reward::REWARD_TYPE].asInt();
    	reward_item.reward_start_cond_ = reward_conf[DBBackActivity::Reward::REWARD_START_COND].asInt();
    	reward_item.reward_id_ = reward_conf[DBBackActivity::Reward::REWARD_ID].asInt();
    	reward_item.content_  = reward_conf[DBBackActivity::Reward::CONTENT].asString();
    	reward_item.times_ = reward_conf[DBBackActivity::Reward::TIMES].asInt();
    	reward_item.must_reset_ = reward_conf[DBBackActivity::Reward::MUST_RESET].asInt();
    	reward_item.handle_type_ = reward_conf[DBBackActivity::Reward::HANDLE_TYPE].asInt();
    	reward_item.cash_coupon_ = reward_conf[DBBackActivity::Reward::CASH_COUPON].asInt();
    	reward_item.exchange_type_ = reward_conf[DBBackActivity::Reward::EXCHANGE_TYPE].asInt();
    	reward_item.exchange_item_name_ = reward_conf[DBBackActivity::Reward::EXCHANGE_ITEM_NAME].asString();

    	GameCommon::json_to_t_vec(reward_item.cond_value_,
    			reward_conf[DBBackActivity::Reward::COND]);

    	const Json::Value& cost_item_json = reward_conf[DBBackActivity::Reward::COST_ITEM];
    	if (cost_item_json != Json::Value::null)
    	{
			const Json::Value& single_item = cost_item_json[0u];
			if (single_item.isArray() == true)
			{
				GameCommon::make_up_conf_items(reward_item.cost_item_, cost_item_json);
			}
			else if (single_item.isInt() == true)
			{
				ItemObj obj = GameCommon::make_up_itemobj(cost_item_json);
				JUDGE_CONTINUE(obj.validate() == true);
				reward_item.cost_item_.push_back(obj);
			}
    	}

    	const Json::Value& pre_cost_json = reward_conf[DBBackActivity::Reward::PRE_COST];
    	if (pre_cost_json != Json::Value::null)
    	{
    		const Json::Value& single_item = pre_cost_json[0u];
    		if (single_item.isArray() == true)
    		{
    			GameCommon::make_up_conf_items(reward_item.pre_cost_, pre_cost_json);
    		}
    		else if (single_item.isInt() == true)
    		{
    			ItemObj obj = GameCommon::make_up_itemobj(pre_cost_json);
    			JUDGE_CONTINUE(obj.validate() == true);
    			reward_item.pre_cost_.push_back(obj);
    		}
    	}

    	act_item->act_item_set_.push_back(reward_item);
    }

	return 0;
}

ItemObj BackSetActDetail::str_to_itemobj(const string& reward_value)
{
	StringVec str_set;
	GameCommon::stl_split(str_set, reward_value);

	ItemObj obj;
	obj.id_ 	= ::atoi(str_set[0].c_str());
	obj.amount_	= ::atoi(str_set[1].c_str());
	obj.bind_ 	= ::atoi(str_set[2].c_str());

	return obj;
}

BackSetActDetail::ActTypeItem* BackSetActDetail::find_item(int act_index, int agent_code, bool validate_time)
{
	BackSetActDetail::ActTypeItemSet::iterator iter = this->act_type_item_set_.begin();
	for (; iter != this->act_type_item_set_.end(); ++iter)
	{
		if (validate_time == true && iter->validate_time() == false)
		{
			continue;
		}

		JUDGE_CONTINUE(iter->act_index_ == act_index);

		if(agent_code == -1 || (int)iter->agent_.size() == 0)	//不分渠道或全渠道
		{
			return &(*iter);
		}
		else
		{
			JUDGE_RETURN(iter->agent_.count(agent_code) > 0, NULL);
			return &(*iter);
		}
	}

	return NULL;
}

BackSetActDetail::ActTypeItem* BackSetActDetail::find_item_by_day(const PairObj& type, int day, int agent_code)
{
	for (BackSetActDetail::ActTypeItemSet::iterator iter = this->act_type_item_set_.begin();
			iter != this->act_type_item_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first_type_ == type.id_);
		JUDGE_CONTINUE(iter->open_time_.count(day) > 0);

		//检测第二类型
		if (type.value_ > 0)
		{
			JUDGE_CONTINUE(iter->second_type_ == type.value_);
		}

		if (agent_code == -1 || iter->agent_.empty() == true)	//不分渠道或全渠道
		{
			return &(*iter);
		}
		else
		{
			JUDGE_CONTINUE(iter->agent_.count(agent_code) > 0);
			return &(*iter);
		}
	}

	return NULL;
}

BackSetActDetail::ActTypeItem* BackSetActDetail::find_end_item(int act_index)
{
	for (BackSetActDetail::ActTypeItemSet::iterator
			iter = this->end_act_set_.begin();
			iter != this->end_act_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->act_index_ == act_index);
		return &(*iter);
	}

	return NULL;
}

BackSetActDetail::ActTypeItem* BackSetActDetail::find_end_item_by_type(int act_type, int second_type)
{
	for (BackSetActDetail::ActTypeItemSet::iterator
			iter = this->end_act_set_.begin();
			iter != this->end_act_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first_type_ == act_type
				&& iter->second_type_ == second_type);

		return &(*iter);
	}
	return NULL;
}
void BackSetActDetail::remove_long_end_act()
{
	Int64 now = (Int64)::time(NULL);
	for (BackSetActDetail::ActTypeItemSet::iterator iter = this->end_act_set_.begin(); iter != this->end_act_set_.end(); )
	{
		if(now - iter->update_tick_ >= 300 && now >= iter->stop_tick_)	//删除已经结束5分钟的活动
		{
			iter = this->end_act_set_.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}
bool BackSetActDetail::compare_act_type_item(const BackSetActDetail::ActTypeItem& first,
			const BackSetActDetail::ActTypeItem& second)
{
	if (first.sort_ > second.sort_)
	{
		return true;
	}

	if (first.sort_ < second.sort_)
	{
		return false;
	}

	if (first.act_index_ > second.act_index_)
	{
		return true;
	}

	return false;
}

BackSetActDetail& BackSetActDetail::act_detail()
{
	return (*this);
}

bool BackSetActDetail::ActTypeItem::is_need_save() const
{
	if (CONFIG_INSTANCE->open_activity_map().find(this->act_index_))
	{
		return CONFIG_INSTANCE->left_open_activity_time() > 0;
	}

	return true;
}

void BackSetActDetail::ActTypeItem::serialize(PActTypeItem* act_item) const
{
	act_item->set_content(this->act_content_);	//描述
	act_item->set_act_index(this->act_index_);	//活动id

	for(BackSetActDetail::ActItemSet::const_iterator it = this->act_item_set_.begin();
			it != this->act_item_set_.end(); ++it)	//活动奖励（同个活动有多个奖励，如累计充值10，20。。。）
	{
		PRewardItem * reward_item = act_item->add_reward_info();
		reward_item->set_brocast(it->brocast_);	//广播标志
		reward_item->set_times(it->times_);		//奖励领取次数限制
		reward_item->set_must_reset(it->must_reset_); //图谱兑换重置
		reward_item->set_cash_coupon(it->cash_coupon_);

		for(LongVec::const_iterator it_cond = it->cond_value_.begin();
				it_cond != it->cond_value_.end(); ++it_cond)	//每个奖励的条件
		{
			reward_item->add_cond(*it_cond);
		}

		for (ItemObjVec::const_iterator it_item = it->cost_item_.begin();
				it_item != it->cost_item_.end(); ++it_item)
		{
			ProtoItem *cost_item = reward_item->add_cost_item();
			const ItemObj item = *it_item;
			item.serialize(cost_item);
		}

		for (ItemObjVec::const_iterator it_pre = it->pre_cost_.begin();
				it_pre != it->pre_cost_.end(); ++it_pre)
		{
			ProtoItem *pre_item = reward_item->add_pre_item();
			const ItemObj item = *it_pre;
			item.serialize(pre_item);
		}

		for(LongMap::const_iterator it_sub_map = it->sub_map_.begin();
				it_sub_map != it->sub_map_.end(); ++it_sub_map)	//排行榜信息
		{
			ProtoPairLong * pair = reward_item->add_sub_map();
			pair->set_obj_id(it_sub_map->first);
			pair->set_obj_value(it_sub_map->second);

			ProtoPairLongString* three = reward_item->add_role_id_name();
			std::map<Int64, FontPair>::const_iterator it_id_name = this->id_name.find(it_sub_map->second);
			if (it_id_name != this->id_name.end())
			{
				three->set_data_int64(it_id_name->first);
				three->set_data_string(it_id_name->second.first);
				three->set_league(it_id_name->second.second);
			}
		}

//		for (LongMap::const_iterator it_recharge = it->recharge_map_.begin();
//				it_recharge != it->recharge_map_.end(); ++it_recharge)
//		{
//			ProtoPairLong * pair = reward_item->add_recharge_map();
//			pair->set_obj_id(it_recharge->first);
//			pair->set_obj_value(it_recharge->second);
//		}

		reward_item->set_content(it->content_);	//每个奖励的描述
		reward_item->set_index(it->cur_index_);	//每个奖励的下标
		reward_item->set_exchange_type(it->exchange_type_);	//兑换物品类型
		reward_item->set_exchange_item_name(it->exchange_item_name_); //兑换物品名称

		for(ItemObjVec::const_iterator it_obj = it->reward_.begin();
				it_obj != it->reward_.end(); ++it_obj)	//每个奖励的道具
		{
			ProtoItem* obj = reward_item->add_reward_set();
			it_obj->serialize(obj);
		}
	}

	act_item->set_title(this->act_title_);	//活动的标题
	for(IntMap::const_iterator it_agent = this->agent_.begin();
			it_agent != this->agent_.end(); ++it_agent)	//活动开通的渠道（空表示所有渠道）
	{
		act_item->add_agent(it_agent->first);
	}

	act_item->set_first_type(this->first_type_);	//活动第一类型
	act_item->set_second_type(this->second_type_);	//活动第二类型
	act_item->set_start_tick(this->start_tick_);	//活动开始时间
	act_item->set_stop_tick(this->stop_tick_);		//活动结束时间
	act_item->set_update_tick(this->update_tick_);
	act_item->set_priority(this->priority_);		//活动优先级（或类型）
	act_item->set_icon_type(this->icon_type_);
	act_item->set_limit(this->limit_);

	for (IntMap::const_iterator iter = this->open_time_.begin();
			iter != this->open_time_.end(); ++iter)
	{
		act_item->add_open_time(iter->first);
	}
}

void BackSetActDetail::ActTypeItem::unserialize(const PActTypeItem* act_item)
{
	BackSetActDetail::ActTypeItem act_sys;
	this->act_content_ = act_item->content();		//描述
	this->act_index_ = act_item->act_index();		//活动id

	for(int i = 0; i < (int)act_item->reward_info_size(); ++i)	//活动奖励（同个活动有多个奖励，如累计充值10，20。。。）
	{
		ActItem reward_item;
		reward_item.brocast_ = act_item->reward_info(i).brocast();	//广播标志
		reward_item.times_ = act_item->reward_info(i).times();		//奖励领取次数限制
		reward_item.must_reset_ = act_item->reward_info(i).must_reset(); //图谱兑换重置
		reward_item.cash_coupon_ = act_item->reward_info(i).cash_coupon();

		for(int j = 0; j < (int)act_item->reward_info(i).cond_size(); ++j)	//每个奖励的条件
		{
			reward_item.cond_value_.push_back(act_item->reward_info(i).cond(j));
		}

		for(int j = 0; j < (int)act_item->reward_info(i).cost_item_size(); ++j)
		{
			ItemObj item;
			item.unserialize(act_item->reward_info(i).cost_item(j));
			reward_item.cost_item_.push_back(item);
		}

		for(int j = 0; j < (int)act_item->reward_info(i).pre_item_size(); ++j)
		{
			ItemObj item;
			item.unserialize(act_item->reward_info(i).pre_item(j));
			reward_item.pre_cost_.push_back(item);
		}

		for(int j = 0; j < (int)act_item->reward_info(i).sub_map_size(); ++j)	//排行榜信息
		{
			const ProtoPairLong& pair = act_item->reward_info(i).sub_map(j);
			reward_item.sub_map_[pair.obj_id()] = pair.obj_value();
		}

//		for (int j = 0; j < (int)act_item->reward_info(i).recharge_map_size(); ++j)
//		{
//			const ProtoPairLong& pair = act_item->reward_info(i).recharge_map(j);
//			reward_item.recharge_map_[pair.obj_id()] = pair.obj_value();
//		}

		for(int j = 0; j < (int)act_item->reward_info(i).role_id_name_size(); ++j)
		{
			const ProtoPairLongString& tmp = act_item->reward_info(i).role_id_name(j);
			FontPair pair;
			pair.first = tmp.data_string();
			pair.second = tmp.league();
			this->id_name[tmp.data_int64()] = pair;
		}

		reward_item.content_ = act_item->reward_info(i).content();	//每个奖励的描述
		reward_item.cur_index_ = act_item->reward_info(i).index();	//每个奖励的下标
		reward_item.exchange_type_ = act_item->reward_info(i).exchange_type(); //兑换物品类型
		reward_item.exchange_item_name_ = act_item->reward_info(i).exchange_item_name(); //兑换物品名称
		for(int j = 0; j < act_item->reward_info(i).reward_set_size(); ++j)	//每个奖励的道具
		{
			ItemObj item_obj(act_item->reward_info(i).reward_set(j));
			reward_item.reward_.push_back(item_obj);
		}
		this->act_item_set_.push_back(reward_item);
	}

	this->act_title_ = act_item->title();	//活动的标题
	for(int i = 0; i < (int)act_item->agent_size(); ++i)	//活动开通的渠道（空表示所有渠道）
	{
		this->agent_[act_item->agent(i)] = true;
	}

	this->first_type_ = act_item->first_type();	//活动第一类型
	this->second_type_ = act_item->second_type();	//活动第二类型
	this->start_tick_ = act_item->start_tick();	//活动开始时间
	this->stop_tick_ = act_item->stop_tick();		//活动结束时间
	this->update_tick_ = act_item->update_tick();
	this->priority_ = act_item->priority();		//活动优先级（或类型）
	this->icon_type_ = act_item->icon_type();
	this->limit_ = act_item->limit();

	for (int i = 0; i < act_item->open_time_size(); ++i)
	{
		int open_time = act_item->open_time(i);
		this->open_time_[open_time] = true;
	}
}

void BackSetActDetail::ActTypeItem::sort_t_sub_map()	//冲榜排序
{
	ThreeObjVec& rank_vec = this->t_sub_rank_;
	rank_vec.reserve(this->t_sub_map_.size());

	rank_vec.clear();
	for (ThreeObjMap::iterator iter = this->t_sub_map_.begin();
			iter != this->t_sub_map_.end(); ++iter)
	{
		rank_vec.push_back(iter->second);
	}

	int rank = 1;
	std::stable_sort(rank_vec.begin(), rank_vec.end(), GameCommon::three_comp_by_desc_b);

	for (ThreeObjVec::iterator iter = rank_vec.begin(); iter != rank_vec.end(); ++iter)
	{
		iter->sub_ = rank;
		this->t_sub_map_[iter->id_].sub_ = rank;	//排名
		rank += 1;
	}
}

void BackSetActDetail::ActTypeItem::sort_t_sub_map_b()
{
	JUDGE_RETURN(this->t_sub_map_.empty() == false, ;);

	ThreeObjVec rank_vec;
	rank_vec.reserve(this->t_sub_map_.size());

	for (ThreeObjMap::iterator iter = this->t_sub_map_.begin();
			iter != this->t_sub_map_.end(); ++iter)
	{
		rank_vec.push_back(iter->second);
	}

	int set_index = 0;
	int total_size = rank_vec.size();
	std::stable_sort(rank_vec.begin(), rank_vec.end(), GameCommon::three_comp_by_desc);

	ThreeObjVec& sub_rank_vec = this->t_sub_rank_;
	sub_rank_vec.clear();

	for (ActItemSet::iterator iter = this->act_item_set_.begin();
			iter != this->act_item_set_.end(); ++iter)
	{
		if (set_index >= total_size)
		{
			break;
		}

		int start_rank = iter->cond_value_[0];
		int end_rank = iter->cond_value_[1];

		int reward_start_cond = iter->reward_start_cond_;
		for (int i = start_rank; i <= end_rank; ++i)
		{
			if (rank_vec[set_index].value_ >= reward_start_cond)
			{
				ThreeObj& obj = rank_vec[set_index];
				obj.sub_ = i;

				this->t_sub_map_[obj.id_].sub_ = i;	//排名
				sub_rank_vec.push_back(obj);

				++set_index;
			}
			else
			{
				ThreeObj obj;
				obj.sub_ = i;
				sub_rank_vec.push_back(obj);
			}

			if (set_index >= total_size)
			{
				break;
			}
		}
	}

	int rank = sub_rank_vec.size();
	for (int i = set_index; i < total_size; ++i)
	{
		ThreeObj& obj = rank_vec[i];
		obj.sub_ = rank;

		this->t_sub_map_[obj.id_].sub_ = rank;	//排名
		sub_rank_vec.push_back(obj);

		++rank;
	}
}

void BackSetActDetail::ActTypeItem::reset_everyday()
{
	this->t_sub_map_.clear();
	this->t_sub_rank_.clear();
	int total = this->act_item_set_.size();
	for (int i = 0; i < total; ++i)
	{
		BackSetActDetail::ActItem& act_item = this->act_item_set_[i];
		act_item.drawed_map_.clear();
		act_item.sub_value_ = false;
	}
}

int BackSetActDetail::get_multile_rate_act(int agent_code, int type, BackSetActDetail::ActRewardRate& rate_info)
{
//	IntVec own_agent_set;
//	Int64 now = ::time(NULL);
//
//	//下面的循环先筛选出优先级
//	for(int i = 0; i < (int)this->act_type_item_set_.size(); ++i)
//	{
//		const BackSetActDetail::ActTypeItem& back_conf =  this->act_type_item_set_[i];
//		JUDGE_CONTINUE(back_conf.first_type_ == BackSetActDetail::ACT_MULTILE_RATE);
//		BackSetActDetail::ActTypeItem* act_info = this->find_item(back_conf.act_index_, agent_code, true);
//		JUDGE_CONTINUE(act_info != NULL);	//只要本渠道的
//		int j = 0;
//		bool has_same_type = false;
//		for(; j < (int)own_agent_set.size(); ++j)
//		{
//			int act_index = own_agent_set[j];
//			BackSetActDetail::ActTypeItem* own_info = this->find_item(act_index);
//			if(own_info != NULL)
//			{
//				if(back_conf.first_type_ == own_info->first_type_ && back_conf.second_type_ == own_info->second_type_ &&
//						back_conf.stop_tick_ >= now && own_info->stop_tick_ >= now && back_conf.start_tick_ < now && own_info->start_tick_ < now)
//				{
//					has_same_type = true;
//					if(back_conf.priority_ < own_info->priority_)	//数值越小优先级越高
//					{
//						own_agent_set[j] = back_conf.act_index_;
//						break;
//					}
//					if(own_info->priority_ == back_conf.priority_ && back_conf.first_type_ == BackSetActDetail::ACT_MULTILE_RATE)		//多倍奖励允许多个同优先级的并存
//					{
//						own_agent_set.push_back(back_conf.act_index_);
//						break;
//					}
//				}
//			}
//		}
//		if(j == (int)own_agent_set.size() && has_same_type == false)	//当前列表中不存在该类型活动则添加
//		{
//			own_agent_set.push_back(back_conf.act_index_);
//		}
//	}
//
//	rate_info.reset();
//	for(IntVec::const_iterator it = own_agent_set.begin(); it != own_agent_set.end(); ++it)
//	{
//		BackSetActDetail::ActTypeItem* s_act_t_item = this->find_item(*it);
//		JUDGE_CONTINUE(s_act_t_item->validate_time() == true);
//		for(BackSetActDetail::ActItemSet::iterator it = s_act_t_item->act_item_set_.begin();
//				it != s_act_t_item->act_item_set_.end(); ++it)
//		{
//			if(it->cond_value_.size() >= 2 && type == it->cond_value_[0])
//			{
//				rate_info.rate = it->cond_value_[1];
//				for(ItemObjVec::const_iterator it_rate_type = it->reward_.begin(); it_rate_type != it->reward_.end(); ++it_rate_type)
//				{
//					rate_info.set(it_rate_type->item_id_);
//				}
//				break;
//			}
//		}
//	}
//#ifdef TEST_COMMAND
//	MSG_USER("type: %d, rate:%d, contribute:%d, salary:%d, join_in:%d, city_salary:%d, rank:%d, wanner:%d, exploit:%d",
//			type, rate_info.rate, rate_info.contribute, rate_info.salary, rate_info.join_in, rate_info.city_salary, rate_info.rank, rate_info.wanner, rate_info.exploit);
//#endif
	return 0;
}

void JYBackActivityItem::Reward::reset(void)
{
    this->__cond_type = 0;
    this->__cond_list.clear();
//    this->__reward_name.clear();
    this->__reward_type = 0;
    this->__reward_item.clear();
    this->__restore_gold_rate = 0;
}

void JYBackActivityItem::reset(void)
{
    this->__act_id = 0;
    this->__first_type = 0;
    this->__second_type = 0;
    this->__act_title.clear();
    this->__act_content.clear();
    this->__act_start = Time_Value::zero;
    this->__act_end = Time_Value::zero;
    this->__show_end = Time_Value::zero;
    this->__is_open = 0;
    this->__order = 0;
    this->__reward_mail_title.clear();
    this->__reward_mail_content.clear();
    this->__need_gold = 0;
    this->__update_tick = Time_Value::zero;
    this->__reward_list.clear();
}

void JYBackActivityItem::serialize(Message *msg)
{
    DYNAMIC_CAST_RETURN(ProtoBackActInfo *, request, msg, ;);
    
    request->set_act_id(this->__act_id);
    request->set_second_type(this->__second_type);
    request->set_act_start(this->__act_start.sec());
    request->set_act_end(this->__act_end.sec());
    request->set_act_content(this->__act_content);
    request->set_need_gold(this->__need_gold);
    int reward_id = 0;
    for (RewardList::iterator iter = this->__reward_list.begin(); iter != this->__reward_list.end(); ++iter)
    {
        Reward &reward_info = *iter;
        ProtoBackActReward *proto_reward = request->add_reward_list();
        proto_reward->set_reward_id(reward_id++);
        proto_reward->set_restore_gold_rate(reward_info.__restore_gold_rate);
//        proto_reward->set_reward_content(reward_info.__reward_name);
        
        for (IntVec::iterator cond_iter = reward_info.__cond_list.begin(); cond_iter != reward_info.__cond_list.end(); ++cond_iter)
        {
        	proto_reward->add_cond_value_list(*cond_iter);
        }

        for (ItemList::iterator item_iter = reward_info.__reward_item.begin(); item_iter != reward_info.__reward_item.end(); ++item_iter)
        {
            ItemObj &item_obj = *item_iter;
            item_obj.serialize(proto_reward->add_reward_item_list());
        }
    }
}

bool JYBackActivityItem::is_recharge_activity(void)
{
	if (this->__second_type == JYBackActivityItem::STYPE_ACCU_RECHARGE ||
			this->__second_type == JYBackActivityItem::STYPE_REPEAT_RECHARGE)
	{
		return true;
	}
	return false;
}

bool JYBackActivityItem::is_consum_activity(void)
{
	if (this->__second_type == JYBackActivityItem::STYPE_ACCU_CONSUM)
		return true;
	return false;
}

bool JYBackActivityItemCmp(JYBackActivityItem * const &left, JYBackActivityItem * const &right)
{
	if (left->__order != right->__order)
		return left->__order < right->__order;
	if (left->__act_id != right->__act_id)
		return left->__act_id < right->__act_id;
	return left < right;
}

void JYBackActivityPlayerRecord::reset(void)
{
    this->__act_id = 0;
    this->__act_start = 0;
    this->__act_end = 0;
    this->__mail_title.clear();
    this->__mail_content.clear();
    this->__daily_refresh_tick = 0;
    this->__daily_value = 0;
    this->__single_cond_value = 0;
    this->__reward_value_map.clear();
    this->__update_tick = Time_Value::zero;
    this->__index_reward_item_map.clear();
}

bool JYBackActivityPlayerRecord::is_has_reward(void)
{
	for (IndexRewardMap::iterator index_iter = this->__reward_value_map.begin(); index_iter != this->__reward_value_map.end(); ++index_iter)
	{
		RewardMap &reward_map = index_iter->second;
		for (RewardMap::iterator reward_iter = reward_map.begin(); reward_iter != reward_map.end(); ++reward_iter)
		{
			if (reward_iter->second == JYBackActivityItem::UNDRAW)
				return true;
		}
	}
	return false;
}

MayActDetail::ActReward::ActReward()
{
	ActReward::reset();
}

void MayActDetail::ActReward::reset()
{
	this->cur_id_ = 0;
	this->name_.clear();
	this->content_.clear();
	this->cond_.clear();

	this->pre_cost_ = 0;
	this->now_cost_ = 0;
	this->times_ 	= 0;
	this->show_id_ 	= 0;

	this->reward_id_= 0;

	this->sub_map_.clear();
	this->drawed_map_.clear();

	this->open_day_ = 0;
	this->group_weight_ = 0;

	this->packet_count_ = 0;
	this->packet_money_ = 0;
	this->get_times_ = 0;
	this->min_money_ = 0;
	this->money_limit_ = 0;
	this->red_act_interval_ = 0;
	this->change_item_.clear();
	this->every_change_times_ = 0;
	this->act_change_times_ = 0;
	this->act_drawed_map_.clear();
}

void MayActDetail::ActReward::request_reset()
{
	this->sub_map_.clear();
	this->drawed_map_.clear();
	this->act_drawed_map_.clear();
}

bool MayActDetail::ActReward::arrive()
{
	return int(this->sub_map_.size()) >= this->cond_[1];
}

MayActDetail::ActInfo::ActInfo()
{
	ActInfo::reset();
}

void MayActDetail::ActInfo::reset()
{
	this->act_index_ = 0;
	this->act_name_.clear();
	this->act_title_.clear();

	this->first_type_ 	= 0;
	this->second_type_ 	= 0;
	this->start_cond_ 	= 0;
	this->mail_id_ 		= 0;
	this->red_point_ 	= 0;
	this->send_reward_ 	= 0;
	this->day_reset_ 	= 0;

	this->open_time_.clear();
	this->limit_cond_.clear();
	this->rand_vec.clear();

	this->reward_set_.clear();

	this->act_type_ = 0;
	this->couple_info_map_.clear();
	this->run_role_map_.clear();

	this->limit_time_vec_.clear();	//活动开启时间段
	this->player_reward_info_.clear();	//玩家红包信息
	this->all_red_packet_map_.clear();
	this->cur_packet_act_times_ = 0;
	this->cur_time_act_open_state_ = 0;
	this->red_act_info_vec_.clear();
}

void MayActDetail::ActInfo::request_reset()
{
	this->couple_info_map_.clear();
	this->run_role_map_.clear();

	this->player_reward_info_.clear();
	this->all_red_packet_map_.clear();
	this->cur_packet_act_times_ = 0;
	this->cur_time_act_open_state_ = 0;
	this->red_act_info_vec_.clear();

	for (ActRewardSet::iterator iter = this->reward_set_.begin();
			iter != this->reward_set_.end(); ++iter)
	{
		iter->request_reset();
	}
}

void MayActDetail::ActInfo::serialize_limit_cond(ProtoLimitValue *proto)
{
	JUDGE_RETURN(this->limit_cond_.size() > 0, ;);

	if (this->limit_cond_.size() >= 1)
		proto->set_value1(this->limit_cond_[0]);

	if (this->limit_cond_.size() >= 2)
		proto->set_value2(this->limit_cond_[1]);

	if (this->limit_cond_.size() >= 3)
		proto->set_value3(this->limit_cond_[2]);

	if (this->limit_cond_.size() >= 4)
		proto->set_value4(this->limit_cond_[3]);

	if (this->limit_cond_.size() >= 5)
		proto->set_value5(this->limit_cond_[4]);
}

MayActDetail::MayActDetail()
{
	MayActDetail::reset();
}

void MayActDetail::reset()
{
	this->open_flag_ = 0;
	this->refresh_tick_ = 0;
	this->begin_date_ = 0;
	this->end_date_ = 0;
	this->act_type_ = 0;

	this->back_act_map_.clear();
	this->agent_.clear();
	this->act_info_set_.clear();
}

void MayActDetail::add_new_item(int index, const Json::Value& conf)
{
	ActInfo act_Info;
	act_Info.act_index_ = index;

	this->update_item(&act_Info, conf);
	this->act_info_set_.push_back(act_Info);
}

void MayActDetail::update_item(ActInfo* act_info, const Json::Value& conf)
{
	act_info->act_name_ 	= conf["name"].asString();
	act_info->act_title_ 	= conf["act_title"].asString();
	act_info->first_type_ 	= conf["first_type"].asInt();
	act_info->second_type_ 	= conf["second_type"].asInt();
	act_info->start_cond_ 	= conf["start_cond"].asInt();
	act_info->mail_id_ 		= conf["mail"].asInt();
	act_info->red_point_ 	= conf["red_point"].asInt();
	act_info->send_reward_ 	= conf["send_reward"].asInt();
	act_info->day_reset_ 	= conf["reset"].asInt();
	act_info->reward_show_id_  = conf["reward_show_id"].asInt();

	GameCommon::json_to_t_map(act_info->open_time_, conf["open_time"]);

	const Json::Value &limit_cond = conf["limit_cond"];
	act_info->limit_cond_.reserve(limit_cond.size());
	for (uint i = 0; i < limit_cond.size(); ++i)
	{
		act_info->limit_cond_.push_back(limit_cond[i].asInt());
	}

	const Json::Value &rand_value = conf["rand_value"];
	act_info->rand_vec.reserve(rand_value.size());
	for (uint i = 0; i < rand_value.size(); ++i)
	{
		ThreeObj obj;
		obj.id_ 	= rand_value[i][0u].asInt();
		obj.tick_ 	= rand_value[i][1u].asInt();
		obj.value_ 	= rand_value[i][2u].asInt();
		obj.sub_ 	= rand_value[i][3u].asInt();
		act_info->rand_vec.push_back(obj);
	}

	const Json::Value &limit_time = conf["limit_time"];
	int group = 1;
	for(uint i = 0 ; i < limit_time.size(); ++i, ++group)
	{
		LimitTime limit;
		limit.group_ = group;
		limit.down_time_ = limit_time[i][0u].asInt();
		limit.up_time_ = limit_time[i][1u].asInt();
		act_info->limit_time_vec_.push_back(limit);
	}

	int pro_size = (int) conf["group_pro_list"].size();
	int group_size = (int)conf["show_times"].size();
	int group_may_be_list_size = (int)conf["group_may_be_list"].size();

	for(int i = 0; i < group_may_be_list_size; ++i)
	{
		act_info->group_may_be_list_[i] = conf["group_may_be_list"][i].asInt();
	}

	for (int i = 0; i < pro_size; ++i)
	{
		act_info->group_pro_list_[i] = conf["group_pro_list"][i].asInt();
	}

	for (int i = 0; i < group_size; ++i)
	{
		act_info->group_show_list_[i] = conf["show_times"][i].asInt();
		act_info->group_no_show_list_[i] = conf["no_show_times"][i].asInt();
		act_info->group_limit_list_[i] = conf["show_limit"][i].asInt();
	}


	for (uint i = 0; i < conf["reward"].size(); ++i)
	{
	    const Json::Value& reward_conf = conf["reward"][i];

	    ActReward act_reward;
	    act_reward.cur_id_ 	 = i;
	    act_reward.name_ 	 = reward_conf["name_str"].asString();
	    act_reward.content_  = reward_conf["content"].asString();
	    act_reward.pre_cost_ = reward_conf["pre_cost"].asInt();
	    act_reward.now_cost_ = reward_conf["now_cost"].asInt();
	    act_reward.times_ 	 = reward_conf["times"].asInt();
	    act_reward.show_id_  = reward_conf["show_id"].asInt();
	    act_reward.reward_id_= reward_conf["reward_id"].asInt();
	    act_reward.task_id_ = reward_conf["task_id"].asInt();
	    ItemObj obj;
	    int id_index = 0, amount_index = 1, bind_index = 2;
	    for(uint j = 0; j < reward_conf["change_item"].size(); ++j)
	    {
	    	const Json::Value &arr_item = reward_conf["change_item"][j];
	    	obj.id_ = arr_item[id_index].asInt();
	    	obj.amount_ = arr_item[amount_index].asInt();
	    	obj.bind_ = arr_item[bind_index].asInt();
	    	act_reward.change_item_.push_back(obj);
	    	obj.reset();
	    }

	    act_reward.every_change_times_ = reward_conf["every_change"].asInt();
	    act_reward.act_change_times_ = reward_conf["act_change"].asInt();

	    act_reward.open_day_ = reward_conf["day"].asInt();
	    act_reward.group_weight_ = reward_conf["group_weight"].asInt();

	    act_reward.packet_count_ = reward_conf["packet_count"].asInt();
	    act_reward.packet_money_ = reward_conf["total_money"].asInt();
	    act_reward.get_times_ = reward_conf["get_times"].asInt();
	    act_reward.min_money_ = reward_conf["min_money"].asInt();
	    act_reward.money_limit_ = reward_conf["limit"].asInt();
	    act_reward.red_act_interval_ = reward_conf["sec"].asInt();


		SlotInfo slot_info;
		slot_info.slot_id_ 		 = i + 1;
		slot_info.the_weight_ 	 = reward_conf["the_weight"].asInt();
		slot_info.pool_percent_  = reward_conf["pool_precent"].asInt();
		slot_info.person_record_ = reward_conf["person_record"].asInt();
		slot_info.server_record_ = reward_conf["server_record"].asInt();
		slot_info.is_shout_		 = reward_conf["is_shout"].asInt();
		slot_info.is_precious_ 	 = reward_conf["is_precious"].asInt();
		slot_info.pre_cost_ 	 = reward_conf["pre_cost"].asInt();
		slot_info.now_cost_		 = reward_conf["now_cost"].asInt();
		slot_info.group_id_ 	 = reward_conf["group_id"].asInt();
		slot_info.item_obj_.id_  = reward_conf["slot_item"][0u].asInt();
		slot_info.item_obj_.amount_  = reward_conf["slot_item"][1u].asInt();
		slot_info.item_obj_.bind_  = reward_conf["slot_item"][2u].asInt();

	    GameCommon::json_to_t_vec(act_reward.cond_, reward_conf["cond"]);

	    act_info->reward_set_.push_back(act_reward);

		SlotInfoVec &slot_map_by_group = act_info->group_slot_map_[reward_conf["group_id"].asInt()];
		slot_map_by_group.push_back(slot_info);
		act_info->group_weight_map_[reward_conf["group_id"].asInt()] += reward_conf["the_weight"].asInt();
	}
}

MayActDetail::ActInfo* MayActDetail::find_act_info(int act_index, int agent_code)
{
	for (ActInfoSet::iterator iter = this->act_info_set_.begin();
			iter != this->act_info_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->act_index_ == act_index);

		if(agent_code == -1 || this->agent_.size() == 0)	//不分渠道或全渠道
		{
			return &(*iter);
		}
		else
		{
			JUDGE_RETURN(this->agent_.count(agent_code) > 0, NULL);
			return &(*iter);
		}
	}

	return NULL;
}


MayActDetail::ActInfo* MayActDetail::find_item_by_day(const PairObj& type, int day,
		int agent_code)
{
	for (ActInfoSet::iterator iter = this->act_info_set_.begin();
			iter != this->act_info_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first_type_ == type.id_);
		JUDGE_CONTINUE(iter->open_time_.count(day) > 0);

		//检测第二类型
		if (type.value_ > 0)
		{
			JUDGE_CONTINUE(iter->second_type_ == type.value_);
		}

		if (agent_code == -1 || this->agent_.empty() == true)	//不分渠道或全渠道
		{
			return &(*iter);
		}
		else
		{
			JUDGE_CONTINUE(this->agent_.count(agent_code) > 0);
			return &(*iter);
		}
	}

	return NULL;
}

void MayActDetail::refresh_act_open_time(int act_index)
{
	ActInfo* act_info = find_act_info(act_index);
	JUDGE_RETURN(act_info != NULL, ;);
	Int64 now_tick = ::time(NULL);
	int today_zero = GameCommon::today_zero();
	LimitTimeVec& time_vec = act_info->limit_time_vec_;
	LimitTimeVec::iterator time_iter = time_vec.begin();
	for( ; time_iter != time_vec.end(); ++time_iter)
	{
		Int64 end_time = today_zero + GameCommon::cal_day_time(time_iter->up_time_);
		Int64 start_time = today_zero + GameCommon::cal_day_time(time_iter->down_time_);
		if(now_tick >= start_time && now_tick <= end_time)
			time_iter->state_ = 0;
		if(now_tick > end_time)
			time_iter->state_ = 1;
	}
}

int MayActDetail::act_left_tick()
{
	int cur_tick = ::time(NULL);
	JUDGE_RETURN(cur_tick >= this->begin_date_ && cur_tick <= this->end_date_, 0);

	return this->end_date_ - cur_tick;
}

int MayActDetail::act_total_day()
{
	int total_tick = this->end_date_ - this->begin_date_;
	return total_tick / Time_Value::DAY;
}

int MayActDetail::act_cur_day()
{
	int cur_tick = ::time(NULL);
	JUDGE_RETURN(cur_tick >= this->begin_date_ && cur_tick < this->end_date_, 0);

	int pass_tick = cur_tick - this->begin_date_;
	return pass_tick / Time_Value::DAY + 1;
}

bool MayActDetail::is_act_open_time(int act_type, int day)
{
	ActInfo *act_info = find_item_by_day(act_type, day);
	JUDGE_RETURN(act_info != NULL, -1);

	this->refresh_act_open_time(act_info->act_index_);
	LimitTimeVec::iterator iter = act_info->limit_time_vec_.begin();
	for( ; iter != act_info->limit_time_vec_.end(); ++iter)
	{
		LimitTime &limit_time = *iter;
		if(limit_time.state_ == 0)
			return true;
	}
	return false;
}

bool MayActDetail::is_act_open_time_by_id(int act_index)
{
	ActInfo *act_info = find_act_info(act_index);
	JUDGE_RETURN(act_info != NULL, -1);

	this->refresh_act_open_time(act_index);
	LimitTimeVec::iterator iter = act_info->limit_time_vec_.begin();
	for( ; iter != act_info->limit_time_vec_.end(); ++iter)
	{
		LimitTime &limit_time = *iter;
		if(limit_time.state_ == 0)
			return true;
	}
	return false;
}

MayActDetail::SlotInfoVec* const MayActDetail::fetch_slot_map_by_group(ActInfo* act_detail, int group_id)
{
	JUDGE_RETURN(act_detail != NULL && act_detail->group_slot_map_.count(group_id) >= 0, NULL);
	return &act_detail->group_slot_map_[group_id];
}

int MayActDetail::fetch_slot_map_weight_by_group(ActInfo* act_detail, int group_id)
{
	JUDGE_RETURN(act_detail != NULL && act_detail->group_weight_map_.count(group_id) >= 0, -1);
	return act_detail->group_weight_map_[group_id];
}

MayActDetail::SlotInfo* MayActDetail::fetch_slot_info(ActInfo* act_detail, int slot_id, int group_id)
{

	SlotInfoVec *slot_vec;
	MayActDetail::GroupSlotInfoMap group_slot_map = act_detail->group_slot_map_;
	if(group_id == 0)
	{
		MayActDetail::GroupSlotInfoMap::iterator g_slot_iter = group_slot_map.begin();
		for( ; g_slot_iter != group_slot_map.end(); ++g_slot_iter)
		{
			for(SlotInfoVec::iterator iter = g_slot_iter->second.begin(); iter != g_slot_iter->second.end(); ++iter)
			{
				if(iter->slot_id_ == slot_id)
				{
					return &(*iter);
				}
			}
		}
	}
	else
	{
		slot_vec = &(group_slot_map[group_id]);
		JUDGE_RETURN(slot_vec != NULL, NULL);
		for(SlotInfoVec::iterator iter = slot_vec->begin(); iter != slot_vec->end(); ++iter)
		{
			if(iter->slot_id_ == slot_id)
			{
				return &(*iter);
			}
		}
	}
	return NULL;
}

int MayActDetail::act_type()
{
	return this->act_type_;
}

MayActDetail::LimitTime::LimitTime()
{
	LimitTime::reset();
}

void MayActDetail::LimitTime::reset()
{
	this->down_time_ = 0;
	this->up_time_ = 0;
	this->state_ = -1;
	this->group_ = 0;
}

MayActDetail::CoupleInfo::CoupleInfo()
{
	CoupleInfo::reset();
}

void MayActDetail::CoupleInfo::reset()
{
	this->wedding_id_ 	= 0;
	this->online_tick_ 	= 0;
	this->buy_tick_ 	= 0;
}

MayActDetail::RunInfo::RunInfo()
{
	RunInfo::reset();
}

void MayActDetail::RunInfo::reset()
{
	this->role_id_ = 0;
	this->name_.clear();
	this->sex_ = 0;
	this->tick_ = 0;
	this->location_ = 0;
}

MayActDetail::RedActInfo::RedActInfo()
{
	RedActInfo::reset();
}
MayActivityer::ActTypeItem::ActTypeItem()
{
	ActTypeItem::reset();
}

void MayActivityer::ActTypeItem::reset()
{
	this->cur_index_ = -1;
	this->sub_value_ = 0;
	this->second_sub_ = 0;
	this->act_item_map_.clear();
	this->send_map_.clear();
	this->role_map_.clear();
	this->group_refresh_times_map_.clear();
	this->cur_fashion_times_ = 1;
	this->liveness_map_.clear();
	this->labour_task_map_.clear();
	this->liveness_ = 0;
	this->max_fashion_times_ = 0;
	this->max_cond_count_ = 0;
	this->cur_fashion_times_ = 0;
	this->shop_slot_vec_temp_.clear();
}

void MayActivityer::ActTypeItem::reset_every()
{
	this->sub_value_ = 0;
	this->second_sub_ = 0;
	this->send_map_.clear();
	this->role_map_.clear();
	this->labour_task_map_.clear();
}

int MayActivityer::ActTypeItem::fetch_role_size(int type)
{
	int num = 0;
	for (LongMap::iterator iter = role_map_.begin(); iter != role_map_.end();++iter)
	{
		JUDGE_CONTINUE(type == iter->second);
		++num;
	}
	return num;
}



void MayActDetail::RedActInfo::reset()
{
	this->state_ = CLOSE;
	this->tick_ = 0;
	this->money_ = 0;
}

MayActDetail::PlayerInfo::PlayerInfo()
{
	PlayerInfo::reset();
}

void MayActDetail::PlayerInfo::reset()
{
	this->money_ = 0;
	this->name_.clear();
	this->role_id_ = 0;
}
