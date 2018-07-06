/*
 * MLRecharger.cpp
 *
 *  Created on: Aug 25, 2014
 *      Author: louis
 */

#include "MLRecharger.h"
#include "BackField.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "PoolMonitor.h"
#include "ProtoDefine.h"
#include "Transaction.h"
#include "BackBrocast.h"
#include <memory>
#include "TQueryCursor.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include <mongo/client/dbclient.h>
#include "MLGameSwither.h"

MLRecharger::MLRecharger() {
	// TODO Auto-generated constructor stub

}

MLRecharger::~MLRecharger() {
	// TODO Auto-generated destructor stub
}

int MLRecharger::total_recharge_gold()
{
	return this->recharge_detail_.__recharge_money;
}

int MLRecharger::reset()
{
	this->recharge_detail_.reset();
	return 0;
}

int MLRecharger::player_recharge_timeup(void)
{
	this->request_load_recharge_order();
	this->refresh_order_record_id();
	return 0;
}

int MLRecharger::after_load_recharge_order(Transaction* trans)
{
	JUDGE_RETURN(this->map_logic_player()->is_active() == true, -1);

	JUDGE_RETURN(NULL != trans, -1);
	if (trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	MongoDataMap* data_map = trans->fetch_mongo_data_map();
	if (NULL == data_map)
	{
		trans->rollback();
		return 0;
	}

	MongoData* mongo_data = NULL;
	if (data_map->find_data(DBBackRecharge::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
		while (cursor->more())
		{
			BSONObj res = cursor->next();
			JUDGE_CONTINUE(res.isEmpty() == false);

			int order_id = res[DBBackRecharge::ID].numberInt();
			int ret = this->request_handle_recharge_order(res);

			if (0 == ret)
			{
				this->request_update_recharge_order_flag(order_id);
			}
			else
			{
				MSG_USER("recharge order %d, %d, %lld, %s", ret, order_id,
						this->role_id(), this->role_name().c_str());
			}
		}
	}

	trans->summit();
	return 0;
}

int MLRecharger::request_load_recharge_order(void)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, 0);

	data_map->push_multithread_query(DBBackRecharge::COLLECTION,
			BSON(DBBackRecharge::ROLE_ID << this->role_id() << DBBackRecharge::FLAG << 0));

	if(TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			TRANS_LOAD_BACK_RECHARGE_ORDER, data_map, MAP_MONITOR->logic_unit()) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

	return 0;
}

int MLRecharger::request_handle_recharge_order(mongo::BSONObj& res,
		bool is_use_item /*=false*/)
{
	int order_id = res[DBBackRecharge::ID].numberInt();
	int gold = res[DBBackRecharge::RECHANGE_GOLD].numberInt();
	std::string order_num = res[DBBackRecharge::ORDER_NUM].str();
	std::string account = res[DBBackRecharge::ACCOUNT].str();

	MSG_USER("handle recharge order prev %d, %d, %lld, %s, %d", order_id, gold,
			this->role_id(), this->name(), is_use_item);

	JUDGE_RETURN(gold > 0, -1);
	JUDGE_RETURN(this->role_detail().__account == account, -1);

	if (is_use_item == false)
	{
		JUDGE_RETURN(this->recharge_detail_.__latest_order_set.count(order_id) == 0, -1);
		JUDGE_RETURN(this->recharge_detail_.__prev_order_set.count(order_id) == 0, -1);
		this->recharge_detail_.__latest_order_set.insert(order_id);
	}

	MSG_USER("handle recharge order after %d, %d, %lld, %s, %d", order_id, gold,
			this->role_id(), this->name(), is_use_item);

	if (is_use_item == true)
	{
		this->recharge(gold, ADD_FROM_RECHARGE_ITEM);
	}
	else
	{
		this->recharge(gold);
	    this->play800_income_log(res, 0);
	}

	return 0;
}

void MLRecharger::play800_income_log(mongo::BSONObj& res,int recharge_ret)
{
	JUDGE_RETURN(this->role_detail().__agent == "play800",;);

	Proto32101107 request;
	int account_len = this->role_detail().__account.size();
	for(int i=0; i < account_len; i++)
	{
		JUDGE_CONTINUE(this->role_detail().__account[i] == '_');
		request.set_uid(&this->role_detail().__account[i+1]);
		break;
	}

	char str_money[128];
	snprintf(str_money, sizeof(str_money),"%d",res[DBBackRecharge::RECHARGE_MONEY].numberInt());
	request.set_sid(this->role_detail().__server_id);
	request.set_username(this->role_detail().__account);
	request.set_role_id(this->role_id());
	request.set_role_name(this->role_name());
	request.set_level(this->role_level());
	request.set_order_id(res[DBBackRecharge::ORDER_NUM].str());
	request.set_income_channel(res[DBBackRecharge::RECHARGE_CHANNEL].str());
	request.set_income_money(str_money);
	request.set_income_gold(res[DBBackRecharge::RECHANGE_GOLD].numberInt());
	request.set_own_gold(this->total_recharge_gold());
	request.set_income_status(recharge_ret);
	request.set_time(res[DBBackRecharge::RECHARGE_TICK].numberLong());
	this->monitor()->dispatch_to_logic(&request, -1, this->role_id());
}

int MLRecharger::request_sync_recharge_info_to_other_role(const std::string& account,
		const int gold, const Int64 exclusive_id)
{
	RechargeOrder* recharge_order = POOL_MONITOR->back_recharge_order_pool()->pop();
	JUDGE_RETURN(NULL != recharge_order, 0);

	recharge_order->__account = account;
	recharge_order->__gold = gold;
	recharge_order->__role_id = exclusive_id;
	recharge_order->__tick = this->recharge_detail_.__first_recharge_time;
	recharge_order->__money = this->total_recharge_gold();

	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SYNC_OTHER_ROLE_RECHARGE_INFO,
			DB_BACK_RECHARGE_ORDER,
			recharge_order, POOL_MONITOR->back_recharge_order_pool()) != 0)
	{
		POOL_MONITOR->back_recharge_order_pool()->push(recharge_order);
		return -1;
	}

	return 0;
}

int MLRecharger::request_update_recharge_order_flag(const int order_id)
{
	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, 0);

	BackRecharge::update_data(data_map, order_id);
	if(TRANSACTION_MONITOR->request_mongo_transaction(0,
			TRANS_UPDATE_BACK_RECHARGE_ORDER_FLAG,data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}

void MLRecharger::first_recharge_annouce_world(ItemObjVec &shout_item_set)
{
//	BrocastParaVec para_vec;
//	GameCommon::push_brocast_para_role_detail(para_vec,
//			this->role_id(), std::string(this->role_detail().__name), this->map_logic_player()->teamer_state());
//
//    const Json::Value &recharge_json = CONFIG_INSTANCE->recharge_rewards_json();
//    for (uint i = 0; i < recharge_json["shout_item"].size(); ++i)
//    {
//        const Json::Value &shout_json = recharge_json["shout_item"][i];
//        int item_id = GameCommon::json_by_level(shout_json, this->role_career()).asInt();
//
//        for (ItemObjVec::iterator iter = shout_item_set.begin(); iter != shout_item_set.end(); ++iter)
//        {
//            ItemObj &item_obj = *iter;
//            JUDGE_CONTINUE(item_obj.id_ == item_id);
//
//            GameCommon::push_brocast_para_item(para_vec, item_id, item_obj.bind_);
//            break;
//        }
//    }
//
//	this->monitor()->announce_world(SHOUT_ALL_FIRST_RECHARGE, para_vec);
}

int MLRecharger::recharger_check_notify(void)
{
	JUDGE_RETURN(true == this->is_on_recharge_activity(), -1);
	this->notify_recharge_activity();// 如果活动不开启，客户端需要通知关闭 T_T

	JUDGE_RETURN(this->recharge_detail_.__recharge_awards.empty() == false, -1);
	this->notify_recharge_awards();

	return 0;
}

int MLRecharger::notify_recharge_awards(void)
{
	FINER_PROCESS_NOTIFY(ACTIVE_GAIN_RECHARGE_AWARD);
}

int MLRecharger::gain_multiple_money(int gold, const Json::Value &awards_json)
{
	JUDGE_RETURN(Json::Value::null != awards_json, -1);
	JUDGE_RETURN(0 != gold, -1);

	Money gain_money( awards_json["multiple_awards"][0u].asDouble() * gold,
			awards_json["multiple_awards"][1u].asDouble() * gold,
			awards_json["multiple_awards"][2u].asDouble() * gold,
			awards_json["multiple_awards"][3u].asDouble() * gold);

	JUDGE_RETURN(GameCommon::validate_money(gain_money), -1);
	this->pack_money_add(gain_money, ADD_MONEY_OPEN_ACT_RECHARGE);

	return 0;
}

int MLRecharger::record_recharge_awards(int gold, int serial)
{
	this->recharge_detail_.record_recharge(gold);
//	JUDGE_RETURN(serial == ADD_FROM_BACK_RECHARGE, -1);

	int id = CONFIG_INSTANCE->fetch_recharge_id(gold, this->recharge_detail_.__recharge_type);
	JUDGE_RETURN(id > 0, -1);


	const Json::Value& conf = CONFIG_INSTANCE->recharge_json(id);
	int ext_gold = conf["ext_gold"].asInt();
	if (ext_gold > 0)
	{
		this->pack_money_add(Money(ext_gold), ADD_FROM_BACK_RECHARGE_EXT);
		Proto30103200 msg;
		msg.set_add_gold(ext_gold);
		MSG_USER("30103200 recharge:%d", ext_gold);
		MAP_MONITOR->dispatch_to_logic(this, &msg);
	}
	JUDGE_RETURN(this->recharge_detail_.__recharge_map.count(id) == 0, -1);

	//开服活动,可以有两分钟的延迟
	int left_time = CONFIG_INSTANCE->left_open_activity_time(2 * Time_Value::MINUTE);
	JUDGE_RETURN(left_time > 0, -1);

	this->recharge_detail_.__recharge_map[id] = true;
	this->pack_money_add(Money(0, conf["bind_gold"].asInt()), ADD_FROM_BACK_RECHARGE_ACT);
//	this->map_logic_player()->update_mount_open_recharge_act(this->total_recharge_gold());

	return 0;
}

int MLRecharger::update_recharge_rebate(int gold)
{
	Proto30101111 request;
	request.set_cur_gold(gold);
	request.set_recharge_type(this->recharge_detail_.__recharge_type);
	return MAP_MONITOR->dispatch_to_logic(this, &request);
}

int MLRecharger::fetch_recharge_awards_info(void)
{
//	//JUDGE_RETURN(true == this->is_on_recharge_activity(), -1);
//	const Json::Value &recharge_json = CONFIG_INSTANCE->recharge_rewards_json();
//	JUDGE_RETURN(Json::Value::null != recharge_json, -1);
//
//	int rewards_stage = 0, has_awards = false;
//	IntVec::reverse_iterator iter = this->recharge_detail_.__recharge_awards.rbegin();
//	if(this->recharge_detail_.__recharge_awards.empty() == false)
//	{
//		rewards_stage = *iter;
//		has_awards = true;
//	}
//
//	Proto51401801 rewards_info;
//	rewards_info.set_has_awards(has_awards);
//
//	ProtoItem *display_item = rewards_info.mutable_display_item();
//	display_item->set_id(GameCommon::json_by_level(recharge_json["display_item"][0u], this->role_career()).asInt());
//	display_item->set_amount(recharge_json["display_item"][1u].asInt());
//	display_item->set_bind(recharge_json["display_item"][2u].asInt());
//	display_item->set_index(recharge_json["display_item"][3u].asInt());
//
//	ProtoItem *equit_item = rewards_info.add_awards_item();
//	*equit_item = *display_item;
//
//	for(uint i = 0; i < recharge_json["first_awards"].size(); ++i)
//	{
//		const Json::Value &awards_json = recharge_json["first_awards"][i];
//		JUDGE_CONTINUE(awards_json["recharge_times"].asInt() == 1);
//
//		for(uint n = 0; n < awards_json["awards_items"].size(); ++n)
//		{
//			ProtoItem *proto_item = rewards_info.add_awards_item();
//			proto_item->set_id(awards_json["awards_items"][n][0u].asInt());
//			proto_item->set_amount(awards_json["awards_items"][n][1u].asInt());
//			proto_item->set_bind(awards_json["awards_items"][n][2u].asInt());
//		}
//
//		break;
//	}
//
//	//MSG_USER(%s, rewards_info.Utf8DebugString().c_str());
//	FINER_PROCESS_RETURN(RETURN_FETCH_RECHARGE_ACTI_INFO, &rewards_info);
	return 0;
}

int MLRecharger::notify_recharge_activity(void)
{
	Proto81401801 recharge_acti_info;

	bool hide_recharge = ((this->recharge_detail_.__recharge_times > 0 &&
			this->recharge_detail_.__recharge_awards.empty()) ||
			false == this->is_on_recharge_activity());

	recharge_acti_info.set_is_start(!hide_recharge);
	FINER_PROCESS_RETURN(ACITIVE_RECHARGE_ACTI_INFO, &recharge_acti_info);
}

int MLRecharger::fetch_recharge_awards(void)
{
//	//JUDGE_RETURN(true == this->is_on_recharge_activity(), -1);
//	const Json::Value &recharge_json = CONFIG_INSTANCE->recharge_rewards_json();
//
//	CONDITION_NOTIFY_RETURN(this->recharge_detail_.__recharge_awards.empty() == false,
//			RETURN_FETCH_RECHARGE_AWARDS, ERROR_NO_REWARD);
//
//	{
//		if(this->recharge_detail_.__love_gift_index < 1)
//			this->recharge_detail_.__love_gift_index = 1;
//		MSG_USER("id:%ld,love_gift_index:%d",this->role_id(),this->recharge_detail_.__love_gift_index);
//		this->check_and_notify_love_icon();
//	}
//
//	ItemObjVec item_obj_set, shout_item_set;
//	ItemVector package_item_set;
//
//	for(IntVec::iterator iter = this->recharge_detail_.__recharge_awards.begin();
//			iter != this->recharge_detail_.__recharge_awards.end(); ++iter)
//	{
//		JUDGE_CONTINUE(0 <= *iter && *iter < int(recharge_json["first_awards"].size()));
//
//		const Json::Value &recharge_step_json = recharge_json["first_awards"][*iter];
//		JUDGE_CONTINUE(Json::Value::null != recharge_step_json);
//		if(recharge_step_json["recharge_times"].asInt() == 1)
//		{
//			// 职业相关装备,只有首充时奖励
//			ItemObj equip_obj;
//			equip_obj.item_id_ = GameCommon::json_by_level(recharge_json["display_item"][0u], this->role_career()).asInt();
//			equip_obj.item_amount_ = recharge_json["display_item"][1u].asInt();
//			equip_obj.item_bind_ = recharge_json["display_item"][2u].asInt();
//			equip_obj.refine_level_ = recharge_json["display_item"][3u].asInt();
//
//
//			PackageItem* equip_item = GamePackage::pop_item(equip_obj.item_id_, equip_obj.item_amount_);
//			CONDITION_NOTIFY_RETURN(0 != equip_item, RETURN_FETCH_RECHARGE_AWARDS, ERROR_SERVER_INNER);
//			equip_item->__equipment.__refine_level = equip_obj.refine_level_;
//			equip_item->__bind = equip_obj.item_bind_;
//
//			package_item_set.push_back(equip_item);
//			shout_item_set.push_back(equip_obj);
//		}
//
//		const Json::Value &cur_awards_json = recharge_step_json["awards_items"];
//		JUDGE_CONTINUE(Json::Value::null != cur_awards_json);
//
//		for(uint i = 0; i < cur_awards_json.size(); ++i)
//		{
//			const Json::Value &item_json = cur_awards_json[i];
//
//			ItemObj item_obj;
//			item_obj.item_id_ = item_json[0u].asInt();
//			item_obj.item_amount_ = item_json[1u].asInt();
//			item_obj.item_bind_ = item_json[2u].asInt();
//
//			JUDGE_CONTINUE(GameCommon::validate_item_id(item_obj.item_id_) == true);
//			item_obj_set.push_back(item_obj);
//			shout_item_set.push_back(item_obj);
//		}
//	}
//
//	if(item_obj_set.empty() == false || package_item_set.empty() == false)
//	{
////		int ret = this->insert_package(ITEM_RECHARGE_REWARD, item_obj_set, package_item_set);
////		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FETCH_RECHARGE_AWARDS, ret);
//	}
//
//	if(shout_item_set.size() > 0)
//	{
//		this->first_recharge_annouce_world(shout_item_set);
//	}
//
//	this->recharge_detail_.__recharge_awards.clear();
//	this->cache_tick().update_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS, true);
//	this->recharger_check_notify();

	FINER_PROCESS_NOTIFY(RETURN_FETCH_RECHARGE_AWARDS);
}

int MLRecharger::fetch_recharge_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11401812*, request, ERROR_CLIENT_OPERATE);

	int left_time = CONFIG_INSTANCE->left_open_activity_time();

	IntMap recharge_map;
	CONFIG_INSTANCE->recharge_id_map(recharge_map, request->type());

	Proto51401812 respond;
	respond.set_left_time(left_time);

	for (IntMap::iterator iter = recharge_map.begin();
			iter != recharge_map.end(); ++iter)
	{
		int flag = this->recharge_detail_.has_recharge(iter->first);
		respond.add_fist_recharge(flag);
	}

	this->recharge_detail_.__recharge_type = request->type();
	FINER_PROCESS_RETURN(RETURN_FETCH_RECHARGE_INFO, &respond);
}

int MLRecharger::sync_transfer_recharge_rewards(int scene_id)
{
	RechargeDetail &detail = this->recharge_detail();

	Proto31400129 sync_info;
	sync_info.set_recharge_money(detail.__recharge_money);
	sync_info.set_recharge_times(detail.__recharge_times);
	sync_info.set_feedback_awards(detail.__feedback_awards);
	sync_info.set_recharge_type(detail.__recharge_type);
	sync_info.set_last_recharge_time(detail.__last_recharge_time);
	sync_info.set_first_recharge_time(detail.__first_recharge_time);
	sync_info.set_order_fresh_tick(detail.__order_fresh_tick.sec());

	GameCommon::map_to_proto(sync_info.mutable_recharge_map(), detail.__recharge_map);

	for (IntVec::iterator iter = detail.__recharge_awards.begin();
			iter != detail.__recharge_awards.end(); ++iter)
	{
		sync_info.add_recharge_awards(*iter);
	}

	for (BIntSet::iterator iter = detail.__latest_order_set.begin();
			iter != detail.__latest_order_set.end(); ++iter)
	{
		sync_info.add_latest_order_list(*iter);
	}

	for (BIntSet::iterator iter = detail.__prev_order_set.begin();
			iter != detail.__prev_order_set.end(); ++iter)
	{
		sync_info.add_prev_order_list(*iter);
	}

	return this->send_to_other_logic_thread(scene_id, sync_info);
}

int MLRecharger::read_transfer_recharge_rewards(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400129*, request, -1);

	RechargeDetail &detail = this->recharge_detail();
	detail.__recharge_money = request->recharge_money();
	detail.__recharge_times = request->recharge_times();
	detail.__feedback_awards = request->feedback_awards();
	detail.__recharge_type = request->recharge_type();
	detail.__last_recharge_time = request->last_recharge_time();
	detail.__first_recharge_time = request->first_recharge_time();
	detail.__order_fresh_tick.sec(request->order_fresh_tick());

	GameCommon::proto_to_map(detail.__recharge_map, request->recharge_map());

	for(int i = 0; i < request->recharge_awards_size(); ++i)
	{
		int recharge_awards_index = request->recharge_awards(i);
		detail.__recharge_awards.push_back(recharge_awards_index);
	}

	for (int i = 0; i < request->latest_order_list_size(); ++i)
	{
		detail.__latest_order_set.insert(request->latest_order_list(i));
	}

	for (int i = 0; i < request->prev_order_list_size(); ++i)
	{
		detail.__prev_order_set.insert(request->prev_order_list(i));
	}

	return 0;
}

/* 判断[首充]奖励开放 */
bool MLRecharger::is_on_recharge_activity(void)
{
//	return ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::first_recharge);
	return false;
}

RechargeDetail &MLRecharger::recharge_detail(void)
{
	return this->recharge_detail_;
}

int MLRecharger::use_recharge_item(const Json::Value& effect, int num)
{
	int gold = effect["gold"].asInt() * num;
	JUDGE_RETURN(gold > 0, ERROR_CONFIG_ERROR);

	mongo::BSONObjBuilder builder;
	builder << DBBackRecharge::RECHANGE_GOLD << gold
			<< DBBackRecharge::ORDER_NUM << "use_recharge_item"
			<< DBBackRecharge::ACCOUNT << this->role_detail().__account;

	mongo::BSONObj obj = builder.obj();
	return this->request_handle_recharge_order(obj, true);
}

int MLRecharger::refresh_order_record_id(void)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	JUDGE_RETURN(this->recharge_detail_.__order_fresh_tick <= nowtime, 0);

	RechargeDetail& detail = this->recharge_detail_;
	detail.__order_fresh_tick = GameCommon::fetch_add_time_value(Time_Value::HOUR);
	detail.__prev_order_set.clear();

	detail.__prev_order_set = detail.__latest_order_set;
	detail.__latest_order_set.clear();

	return 0;
}


