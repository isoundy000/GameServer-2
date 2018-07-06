/*
 * MLPacker.cpp
 *
 *  Created on: Jul 5, 2013
 *      Author: peizhibi
 */

#include "MLPacker.h"
#include "BaseUnit.h"
#include "MapMonitor.h"
#include "PoolMonitor.h"
#include "SerialRecord.h"
#include "MapLogicPlayer.h"
#include "GameFont.h"

#include "GameCommon.h"
#include "ProtoDefine.h"

#include <mongo/client/dbclient.h>
#include "MLGameSwither.h"
#include "TipsEnum.h"
#include "MapLogicSerial.h"

MLPacker::MLPacker()
{
	// TODO Auto-generated constructor stub

}

MLPacker::~MLPacker()
{
	// TODO Auto-generated destructor stub
}

MapLogicPlayer* MLPacker::map_logic_player()
{
	return dynamic_cast<MapLogicPlayer*>(this);
}

int MLPacker::request_save_mmo_begin(const string& table_name, const BSONObj& query,
		const BSONObj& content, int upgrade_type, int trans_recogn)
{
	return GameCommon::request_save_mmo_begin(table_name, query, content, upgrade_type,
			trans_recogn, this->role_id());
}

int MLPacker::request_remove_mmo_begin(const string& table_name, const BSONObj& query,
		int just_one, int trans_recogn)
{
	return GameCommon::request_remove_mmo_begin(table_name, query, just_one,
			trans_recogn, this->role_id());
}

int MLPacker::send_to_map_thread(int recogn)
{
	return MAP_MONITOR->process_inner_map_request(this->role_id(), recogn);
}

int MLPacker::send_to_map_thread(Message& msg)
{
	return MAP_MONITOR->process_inner_map_request(this->role_id(), msg);
}

int MLPacker::send_to_map_thread(int cmd, Block_Buffer* buf)
{
	UnitMessage unit_msg;
	unit_msg.__msg_head.__recogn = cmd;
	unit_msg.__msg_head.__role_id = this->role_id();

	Block_Buffer *tmp_buf = this->monitor()->pop_block();
	tmp_buf->copy(buf);
	unit_msg.set_data_buff(tmp_buf);

	return this->monitor()->map_unit()->push_request(unit_msg);
}

//发送场景scene_id的逻辑线程
int MLPacker::send_to_other_logic_thread(int scene_id, Message& msg)
{
	return this->monitor()->dispatch_to_scene(this, scene_id, &msg);
}

// enter_type用于进入场景时指定ENTER_SCENE_TRANSFER, 否则进入场景时会不满血[登录或切换场景时不修改当前血量]
void MLPacker::refresh_fight_property(int offset, const IntMap& prop_map, const SubObj& obj)
{
	Proto30400011 prop_info;
	prop_info.set_offset(offset);
	prop_info.set_enter_type(obj.val1_);
	prop_info.set_unnotify(obj.val2_);

	for (IntMap::const_iterator iter = prop_map.begin(); iter != prop_map.end(); ++iter)
	{
		ProtoPairObj* pair_obj = prop_info.add_prop_set();
		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}

	this->send_to_map_thread(prop_info);
}

void MLPacker::refresh_fight_property(int offset, int prop_type, int prop_value)
{
	IntMap prop_map;
	prop_map[prop_type] = prop_value;
	this->refresh_fight_property(offset, prop_map);
}

void MLPacker::test_add_pack_item()
{
	IntMap goods_map;
	CONFIG_INSTANCE->test_goods(goods_map);

	for (IntMap::iterator iter = goods_map.begin(); iter != goods_map.end(); ++iter)
	{
		this->insert_package(ADD_FROM_LOCAL_TEST, iter->first, iter->second);
	}
}

void MLPacker::sync_today_consume_gold(int consum_gold, int consum_bind_gold)
{
	IntMap& use_map = this->pack_detail().use_resource_map_;

	Proto30100606 inner;
	inner.set_today_gold(use_map[GameEnum::ITEM_TODAY_UNBIND_GOLD]);
	inner.set_today_bind_gold(use_map[GameEnum::ITEM_TODAY_BIND_GOLD]);
	inner.set_consum_gold(consum_gold);
	inner.set_consum_bind_gold(consum_bind_gold);
	this->monitor()->dispatch_to_logic(this, &inner);
}

int MLPacker::handle_operate_result(int recogn, int result)
{
	if (result != 0)
	{
		this->respond_to_client_error(recogn, result);
		return false;
	}
	else
	{
		this->respond_to_client(recogn);
		return true;
	}
}

int MLPacker::request_map_use_goods(PackageItem* pack_item, int &use_num)
{
	Proto30400015 goods_info;
	pack_item->serialize(goods_info.mutable_item_info(), -1, use_num);
    return this->send_to_map_thread(goods_info);
}

int MLPacker::request_add_exp(int add_exp, const SerialObj &serial_obj)
{
	JUDGE_RETURN(add_exp > 0, ERROR_CLIENT_OPERATE);

    Proto30400013 add_info;
    add_info.set_add_exp(add_exp);

    ProtoSerialObj* proto_serial = add_info.mutable_serial_obj();
    serial_obj.serialize(proto_serial);

	int exp_tip = CONFIG_INSTANCE->serial_exp_tips(serial_obj.type_);
	if (exp_tip)
	{
		TipsPlayer tips(this);
		tips.push_goods(GameEnum::ITEM_ID_PLAYER_EXP, add_exp);
	}

    return this->send_to_map_thread(add_info);
}

int MLPacker::request_direct_add_blood(int add_blood)
{
	Proto30400014 add_info;
	add_info.set_add_blood(add_blood);
	return this->send_to_map_thread(add_info);
}

int MLPacker::request_direct_add_magic(int add_magic)
{
	Proto30400009 add_info;
	add_info.set_add_magic(add_magic);
	return this->send_to_map_thread(add_info);
}

int MLPacker::request_use_lucky_prop(PackageItem* pack_item, int& use_num)
{
	return 0;
}

int MLPacker::process_kill_reduce_lucky(Message *msg)
{
    return 0;
}

int MLPacker::request_level_up_player(int type)
{
	Proto30400008 add_info;
	add_info.set_type(type);
	return this->send_to_map_thread(add_info);
}

int MLPacker::request_add_exp_per_buff(int id, int percent, int last)
{
    Proto30400010 buff_info;
    buff_info.set_id(id);
    buff_info.set_percent(percent);
    buff_info.set_last(last);
    return this->send_to_map_thread(buff_info);
}

int MLPacker::notify_gift_pack_show(const ItemObjMap& items_map)
{
	Proto81400110 respond;
	for (ItemObjMap::const_iterator iter = items_map.begin();
			iter != items_map.end(); ++iter)
	{
		iter->second.serialize(respond.add_items());
	}

	FINER_PROCESS_RETURN(ACTIVE_PACK_GIFT_SHOW, &respond);
}

int MLPacker::notify_item_map_tips(const ItemObjMap& items_map)
{
	TipsPlayer tips(this);
	for (ItemObjMap::const_iterator iter = items_map.begin();
			iter != items_map.end(); ++iter)
	{
		const ItemObj& obj = iter->second;
		tips.push_goods(obj.id_, obj.amount_);
	}

	return 0;
}

int MLPacker::notify_pack_info(int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, -1);

	Proto51400101 pack_info;
	pack_info.set_pack_type(package->type());
	pack_info.set_pack_size(package->size());

	ItemListMap& item_map = package->item_map();
	for (ItemListMap::iterator iter = item_map.begin();
			iter != item_map.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item != NULL);

		ProtoItem* proto_item = pack_info.add_item_list();
		package->serialize(proto_item, pack_item);
	}

	this->make_up_other_equip_pack(package, &pack_info);
	FINER_PROCESS_RETURN(RETURN_PACKAGE_INFO, &pack_info);
}

int MLPacker::make_up_other_equip_pack(GamePackage* package, Proto51400101* proto)
{
	JUDGE_RETURN(package->type() == GameEnum::INDEX_EQUIP, -1);

	PackGridVec& pack_grid = package->grid_vec();
	for (PackGridVec::iterator iter = pack_grid.begin(); iter != pack_grid.end(); ++iter)
	{
		proto->add_strengthen(iter->strengthen_lvl_);
	}

	return 0;
}

int MLPacker::fetch_pack_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400101*, request, RETURN_PACKAGE_INFO);

	// enter scene notify
	this->notify_pack_info(request->pack_type());
	JUDGE_RETURN(request->pack_type() == GameEnum::INDEX_PACKAGE, 0);

	this->notif_pack_money();
	return 0;
}

int MLPacker::fetch_rotary_table_result(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400115*, request, RETURN_FETCH_ROTARY_TABLE);

	PackageItem* pack_item = this->pack_find(request->item_index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_FETCH_ROTARY_TABLE,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(GameCommon::is_rotary_table_goods(pack_item->__id) == true,
			RETURN_FETCH_ROTARY_TABLE, ERROR_CLIENT_OPERATE);

	Proto51400115 result_info;
	result_info.set_item_index(pack_item->__index);

	const Json::Value& effect_conf = CONFIG_INSTANCE->prop(pack_item->__id)["effect"];
	for (uint i = 0; i < effect_conf["item_list"].size(); ++i)
	{
		ProtoItem* proto_item = result_info.add_item_set();
		JUDGE_CONTINUE(proto_item != NULL);

		proto_item->set_id(effect_conf["item_list"][i][0u].asInt());
		proto_item->set_amount(effect_conf["item_list"][i][1u].asInt());
		proto_item->set_bind(effect_conf["item_list"][i][2u].asInt());
	}

	GameCommon::set_rotary_table_index(pack_item);
	result_info.set_prop_index(pack_item->__rotary_table.prop_index_);

	FINER_PROCESS_RETURN(RETURN_FETCH_ROTARY_TABLE, &result_info);
}

int MLPacker::use_rotary_table_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400116*, request, RETURN_USE_ROTARY_TABLE);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_USE_ROTARY_TABLE,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(package->left_capacity() > 0, RETURN_USE_ROTARY_TABLE,
			ERROR_PACKAGE_NO_CAPACITY);

	PackageItem* pack_item = package->find_by_index(request->item_index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_USE_ROTARY_TABLE,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(GameCommon::is_rotary_table_goods(pack_item->__id) == true,
			RETURN_USE_ROTARY_TABLE, ERROR_CLIENT_OPERATE);

	uint prop_index = pack_item->__rotary_table.prop_index_;
	const Json::Value& effect_conf = CONFIG_INSTANCE->prop(pack_item->__id)["effect"];
	CONDITION_NOTIFY_RETURN(prop_index > 0 && prop_index <= effect_conf["item_list"].size(),
			RETURN_USE_ROTARY_TABLE, ERROR_CLIENT_OPERATE);

	ItemObj item_obj = GameCommon::make_up_itemobj(effect_conf["item_list"][prop_index - 1]);

	int ret = this->pack_insert(package, ITEM_ROTARY_TABLE_GET, item_obj);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_USE_ROTARY_TABLE, ret);

	Proto51400116 add_info;
	add_info.set_item_id(item_obj.id_);
	add_info.set_item_amount(item_obj.amount_);

	if (pack_item->__amount > 1)
	{
		GameCommon::set_rotary_table_index(pack_item);
		add_info.set_prop_index(pack_item->__rotary_table.prop_index_);
	}
	else
	{
		pack_item->__rotary_table.prop_index_ = 0;
	}

	this->pack_remove(package, ITEM_PLAYER_USE, pack_item, 1);
	FINER_PROCESS_RETURN(RETURN_USE_ROTARY_TABLE, &add_info);
}

int MLPacker::tip_pack_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400153*, request, RETUNR_ITEM_TIP);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETUNR_ITEM_TIP, ERROR_PACKAGE_TYPE);

	if (request->index() >= 0)
	{
		PackageItem* pack_item = package->find_by_index(request->index());
		CONDITION_NOTIFY_RETURN(pack_item != NULL, RETUNR_ITEM_TIP, ERROR_CLIENT_OPERATE);
		pack_item->__new_tag = 1;
	}

	if (request->index() == -1)
	{
		for (ItemListMap::iterator iter = package->item_list_map_.begin();
				iter != package->item_list_map_.end(); ++iter)
		{
			PackageItem* pack_item = iter->second;
			pack_item->__new_tag = 1;
		}
	}

    Proto51400153 respond;
    respond.set_new_tag(1);
    FINER_PROCESS_RETURN(RETUNR_ITEM_TIP, &respond);
}

int MLPacker::role_create_days()
{
	return this->role_detail().create_info_.passed_days();
}

int MLPacker::is_arrive_max_level(int lvl_up_num)
{
	return this->role_level() + lvl_up_num > MAX_PLAYER_LEVEL;
}

int MLPacker::fetch_login_reward_flag()
{
	//是否有七天登录奖励的标志
	Proto51400119 respond;
	int reward_flag = 0;
	if (this->role_detail().draw_days_.size() < 7)
		reward_flag = true;

	respond.set_reward_flag(reward_flag);
	FINER_PROCESS_RETURN(RETURN_LOGIN_REWARD_FLAG, &respond);
}

int MLPacker::fetch_login_reward_info()
{
	//获取七天登录奖励信息
	Proto51400117 reward_info;
//	reward_info.set_login_days(this->role_create_days());
	reward_info.set_login_days(this->role_detail().__login_days);

	for (IntMap::iterator iter = this->role_detail().draw_days_.begin();
			iter != this->role_detail().draw_days_.end(); ++iter)
	{
		reward_info.add_draw_days(iter->first);
	}

	for (int i = 1; i <= 7; ++i)
	{
		const Json::Value& day_info = CONFIG_INSTANCE->seven_day(i);
		JUDGE_CONTINUE(day_info != Json::Value::null);

		ProtoSevenDayItem *login_reward = reward_info.add_login_reward();
		login_reward->set_day(day_info["day"].asInt());
		login_reward->set_item_id(day_info["reward"].asInt());
	}

	FINER_PROCESS_RETURN(RETURN_LOGIN_REWARD_INFO, &reward_info);
}

int MLPacker::draw_login_reward(Message* msg)
{	//领取七天登录奖励
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400118*, request, RETURN_DRAW_LOGIN_REWARD);

	int draw_day = request->draw_day();
	CONDITION_NOTIFY_RETURN(this->role_detail().draw_days_.count(draw_day) == 0,
			RETURN_DRAW_LOGIN_REWARD, ERROR_CLIENT_OPERATE);

	const Json::Value& day_info = CONFIG_INSTANCE->seven_day(draw_day);
	CONDITION_NOTIFY_RETURN(day_info != Json::Value::null,
			RETURN_DRAW_LOGIN_REWARD,ERROR_CLIENT_OPERATE);

	int login_day = this->role_detail().__login_days;
	CONDITION_NOTIFY_RETURN(login_day >= draw_day, RETURN_DRAW_LOGIN_REWARD,
			ERROR_CLIENT_OPERATE);

	this->role_detail().draw_days_[draw_day] = true;

	int reward_id = day_info["reward"].asInt();
	this->add_reward(reward_id, ADD_FROM_SEVEN_DAY_AWARD);

	this->login_reward_red_point();

	Proto51400118 reward_info;
	reward_info.set_draw_day(draw_day);
	reward_info.set_item_id(reward_id);
	FINER_PROCESS_RETURN(RETURN_DRAW_LOGIN_REWARD, &reward_info);
}

int MLPacker::login_reward_red_point()
{
	int login_day =this->role_detail().__login_days;
	int draw_day = this->role_detail().draw_days_.size();

	MapLogicPlayer* player = this->map_logic_player();
	if (login_day > draw_day)
		player->update_player_assist_single_event(GameEnum::PA_EVENT_SEVEN_DAY, 1);
	else
		player->update_player_assist_single_event(GameEnum::PA_EVENT_SEVEN_DAY, 0);

	return 0;
}

int MLPacker::fetch_open_gift_info()
{
	JUDGE_RETURN(this->check_is_in_travel_scene() == false, 0);
	JUDGE_RETURN(this->check_is_open_gift_day() == true, 0);

	MapLogicPlayer* player = this->map_logic_player();
	CONDITION_NOTIFY_RETURN(player != NULL, RETURN_OPEN_GIFT_INFO, ERROR_PLAYER_OFFLINE);

	Proto51400123 respond;
	respond.set_recharge_amount(player->total_recharge_gold());
	respond.set_left_tick(this->fetch_open_gift_left_tick());

	for (IntMap::iterator iter = this->role_detail().draw_gift_.begin();
			iter != this->role_detail().draw_gift_.end(); ++iter)
	{
		respond.add_draw_location(iter->first);
	}
	FINER_PROCESS_RETURN(RETURN_OPEN_GIFT_INFO, &respond);
}

int MLPacker::draw_open_gift_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400124*, request, RETURN_OPEN_GIFT_REWARD);

	CONDITION_NOTIFY_RETURN(this->check_is_in_travel_scene() == false,
			RETURN_OPEN_GIFT_REWARD, ERROR_IN_TRAVEL_SCENE);
	JUDGE_RETURN(this->check_is_open_gift_day() == true, 0);

	int location_id = request->location_id();
	CONDITION_NOTIFY_RETURN(this->role_detail().draw_gift_.count(location_id) <= 0,
			RETURN_OPEN_GIFT_REWARD, ERROR_CLIENT_OPERATE);

	const Json::Value& open_gift_json = CONFIG_INSTANCE->open_gift(location_id);
	CONDITION_NOTIFY_RETURN(open_gift_json != Json::Value::null,
			RETURN_OPEN_GIFT_REWARD, ERROR_CLIENT_OPERATE);

	MapLogicPlayer* player = this->map_logic_player();
	CONDITION_NOTIFY_RETURN(player != NULL, RETURN_OPEN_GIFT_REWARD, ERROR_PLAYER_OFFLINE);

	int need_money = open_gift_json["recharge_money"].asInt();
	int charge_money = player->total_recharge_gold();
	CONDITION_NOTIFY_RETURN(charge_money >= need_money, RETURN_OPEN_GIFT_REWARD, ERROR_CLIENT_OPERATE);

	this->role_detail().draw_gift_[location_id] = true;

	int reward_id = open_gift_json["reward"].asInt();
	this->add_reward(reward_id, ADD_FROM_OPEN_GIFT_REWARD);

	Proto51400124 respond;
	respond.set_location_id(location_id);
	FINER_PROCESS_RETURN(RETURN_OPEN_GIFT_REWARD, &respond);
}

bool MLPacker::check_is_open_gift_day()
{
	int server_day = CONFIG_INSTANCE->open_server_days();
	int open_gift_day = CONFIG_INSTANCE->const_set("open_gift_day");
	return server_day <= open_gift_day;
}

int MLPacker::fetch_open_gift_left_tick()
{
	int server_day = CONFIG_INSTANCE->open_server_days();
	int open_gift_day = CONFIG_INSTANCE->const_set("open_gift_day");
	if (server_day > open_gift_day)
		return 0;

	return (open_gift_day - server_day) * Time_Value::DAY + GameCommon::next_day();
}

int MLPacker::check_send_open_gift_mail()
{
	JUDGE_RETURN(this->check_is_in_travel_scene() == false, 0);
	JUDGE_RETURN(this->check_is_open_gift_day() == false, 0);
	JUDGE_RETURN(this->role_detail().__open_gift_close == false, 0);

	this->role_detail().__open_gift_close = true;
	this->respond_to_client(ACTIVE_CLOSE_OPEN_GIFT);

	MapLogicPlayer* player = this->map_logic_player();
	JUDGE_RETURN(player != NULL, 0);

	int charge_money = player->total_recharge_gold();
	GameConfig::ConfigMap& gift_map = CONFIG_INSTANCE->open_gift_map();
	JUDGE_RETURN(charge_money > 0, 0);
	JUDGE_RETURN(this->role_detail().draw_gift_.size() < gift_map.size(), 0);

	IntMap reward_map;
	for (GameConfig::ConfigMap::iterator iter = gift_map.begin();
			iter != gift_map.end(); ++iter)
	{
		JUDGE_CONTINUE(this->role_detail().draw_gift_.count(iter->first) <= 0);

		const Json::Value& open_gift_json = *(iter->second);
		int need_money = open_gift_json["recharge_money"].asInt();
		JUDGE_CONTINUE(charge_money >= need_money);

		int reward_id = open_gift_json["reward"].asInt();
		reward_map[reward_id] = true;
		this->role_detail().draw_gift_[iter->first] = true;
	}

	JUDGE_RETURN(reward_map.size() > 0, 0);

	int mail_id = CONFIG_INSTANCE->const_set("open_gift_mail_id");
	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str());

	for (IntMap::iterator iter = reward_map.begin();
			iter != reward_map.end(); ++iter)
	{
		mail_info->add_goods(iter->first);
	}
	GameCommon::request_save_mail_content(this->role_id(), mail_info);

	return 0;
}

int MLPacker::check_is_in_travel_scene()
{
#ifdef LOCAL_DEBUG
    if (GameCommon::is_travel_scene(this->scene_id()))
#else
    if (this->monitor()->is_has_travel_scene() == true)
#endif
    {
    	return true;
    }
    else
    {
    	return false;
    }
}

int MLPacker::test_reset_open_gift()
{
	this->role_detail().draw_gift_.clear();
	return 0;
}

int MLPacker::check_del_item(RewardInfo &reward_info)
{
	GamePackage* des_package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(des_package != NULL, 0);

	PackageItem* des_item = des_package->find_by_index(0);	//武器
	JUDGE_RETURN(des_item != NULL, false);

	int amount = 0;	//防止死循环
	for (ItemObjVec::iterator iter = reward_info.item_vec_.begin();
			iter != reward_info.item_vec_.end(); ++iter)
	{
		++amount;
		if (amount > 50)
			break;

		ItemObj &obj = *iter;
		const Json::Value& item_conf = CONFIG_INSTANCE->item(obj.id_);
		JUDGE_CONTINUE(item_conf.empty() == false);

		const Json::Value& effect_prop = item_conf["effect_prop"];
		JUDGE_CONTINUE(effect_prop.empty() == false);
		JUDGE_CONTINUE(effect_prop.isMember("check_equip_id") == true);

		int del_flag = false;
		const Json::Value& check_equip = effect_prop["check_equip_id"];
		for (uint i = 0; i < check_equip.size(); ++i)
		{
			int equip_id = check_equip[i].asInt();
			JUDGE_CONTINUE(des_item->__id == equip_id);

			del_flag = true;
			break;
		}

		if (del_flag == true)
		{
			reward_info.item_vec_.erase(iter);
			--iter;
		}
	}

	return 0;
}

int MLPacker::red_clothes_exchange(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400135*, request, RETURN_REQUEST_RED_CLOTHES_EXCHANGE);

	int exchange_amount = request->amount();
	CONDITION_NOTIFY_RETURN(exchange_amount > 0, RETURN_REQUEST_RED_CLOTHES_EXCHANGE, ERROR_CLIENT_OPERATE);

	int exchange_id = request->exchange_id();
	const Json::Value& exchange_json = CONFIG_INSTANCE->red_clothes_exchange(exchange_id);
	CONDITION_NOTIFY_RETURN(exchange_json != Json::Value::null,
			RETURN_REQUEST_RED_CLOTHES_EXCHANGE, ERROR_CONFIG_NOT_EXIST);

	const Json::Value& cost_item = exchange_json["cost_item"];
	int cost_id = cost_item[0u].asInt();
	int cost_num = cost_item[1u].asInt() * exchange_amount;
	PackageItem *pack_item = this->pack_find_by_id(cost_id);
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_REQUEST_RED_CLOTHES_EXCHANGE,
			ERROR_PACKAGE_ITEM_AMOUNT);
	CONDITION_NOTIFY_RETURN(pack_item->__amount >= cost_num,
			RETURN_REQUEST_RED_CLOTHES_EXCHANGE, ERROR_PACKAGE_ITEM_AMOUNT);

	int ret = this->pack_remove(ITEM_RED_CLOTHES_EXCHANGE, cost_id, cost_num);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REQUEST_RED_CLOTHES_EXCHANGE, ret);

	const Json::Value& get_item = exchange_json["get_item"];
	int item_id = get_item[0u].asInt();
	int amount = get_item[1u].asInt() * exchange_amount;
	int item_bind = get_item[2u].asInt();
	this->insert_package(ADD_FROM_RED_CLOTHES_EXCHANGE, item_id, amount, item_bind);

	Proto51400135 respond;
	respond.set_exchange_id(exchange_id);
	FINER_PROCESS_RETURN(RETURN_REQUEST_RED_CLOTHES_EXCHANGE, &respond);
}

int MLPacker::request_exchange(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400136*, request, RETURN_LEGEND_SECRET_EXCHANGE);

	int type = request->type();
	int exchange_amount = request->amount();
	CONDITION_NOTIFY_RETURN(type == 1 || type == 2, RETURN_LEGEND_SECRET_EXCHANGE, ERROR_CLIENT_OPERATE);

	int exchange_id = request->exchange_id();
	if (type == 1)
	{
		CONDITION_NOTIFY_RETURN(exchange_amount == 1, RETURN_LEGEND_SECRET_EXCHANGE, ERROR_CLIENT_OPERATE);
		const Json::Value& exchange_json = CONFIG_INSTANCE->legend_exchange(exchange_id);
		return this->exchange_detail(request, exchange_json);
	}
	else
	{
		const Json::Value& exchange_json = CONFIG_INSTANCE->secret_exchange(exchange_id);
		return this->exchange_detail(request, exchange_json);
	}
}

int MLPacker::exchange_detail(Proto11400136 *request, const Json::Value& exchange_json)
{
	CONDITION_NOTIFY_RETURN(exchange_json != Json::Value::null,
			RETURN_LEGEND_SECRET_EXCHANGE, ERROR_CONFIG_NOT_EXIST);

	if (request->type() == 1)
	{
		int position = exchange_json["position"].asInt();
		GamePackage* des_package = this->find_package(GameEnum::INDEX_EQUIP);
		CONDITION_NOTIFY_RETURN(des_package != NULL, RETURN_LEGEND_SECRET_EXCHANGE,
				ERROR_SERVER_INNER);

		PackageItem* des_item = des_package->find_by_index(position-1);	//身上装备
		CONDITION_NOTIFY_RETURN(des_item != NULL, RETURN_LEGEND_SECRET_EXCHANGE,
				ERROR_CONFIG_ERROR);

		const Json::Value& item_conf = des_item->conf();
		CONDITION_NOTIFY_RETURN(item_conf.empty() == false, RETURN_LEGEND_SECRET_EXCHANGE,
				ERROR_CONFIG_NOT_EXIST);

		int color = item_conf["color"].asInt();
		int level = exchange_json["level"].asInt();
		CONDITION_NOTIFY_RETURN(color <= level, RETURN_LEGEND_SECRET_EXCHANGE, ERROR_CLIENT_OPERATE);
	}

	int exchange_amount = request->amount();

	const Json::Value& cost_item = exchange_json["cost_item"];
	int cost_id = cost_item[0u].asInt();
	int cost_num = cost_item[1u].asInt() * exchange_amount;
	if (cost_item[1u].asInt() > 0)
	{
		if (INT_MAX / cost_item[1u].asInt() < exchange_amount)
			return this->respond_to_client_error(RETURN_LEGEND_SECRET_EXCHANGE, ERROR_CLIENT_OPERATE);
	}

	PackageItem *pack_item = this->pack_find_by_id(cost_id);
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_LEGEND_SECRET_EXCHANGE,
			ERROR_PACKAGE_ITEM_AMOUNT);
	CONDITION_NOTIFY_RETURN(pack_item->__amount >= cost_num, RETURN_LEGEND_SECRET_EXCHANGE,
			ERROR_PACKAGE_ITEM_AMOUNT);

	int ret = 0;
	if (request->type() == 1)
		ret = this->pack_remove(ITEM_LEGEND_EXCHANGE, cost_id, cost_num);
	else
		ret = this->pack_remove(ITEM_SECRET_EXCHANGE, cost_id, cost_num);

	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_LEGEND_SECRET_EXCHANGE, ret);

	const Json::Value& get_item = exchange_json["get_item"];
	int item_id = get_item[0u].asInt();
	int amount = get_item[1u].asInt() * exchange_amount;
	int item_bind = get_item[2u].asInt();

	if (request->type() == 1)
		this->insert_package(ADD_FROM_LEGEND_EXCHANGE, item_id, amount, item_bind);
	else
		this->insert_package(ADD_FROM_SECRET_EXCHANGE, item_id, amount, item_bind);

	Proto51400136 respond;
	respond.set_type(request->type());
	respond.set_exchange_id(request->exchange_id());
	FINER_PROCESS_RETURN(RETURN_LEGEND_SECRET_EXCHANGE, &respond);
}

int MLPacker::open_pack_grid(Message* msg)
{
	return 0;
}

int MLPacker::check_has_grid_open(void)
{
	return 0;
}

int MLPacker::recharge_mail()
{
	return 0;
}

int MLPacker::recharge_info_update(int gold)
{
	MapLogicPlayer* player = this->map_logic_player();

	//sync to logic server
	Proto30101110 request;
	request.set_cur_gold(gold);
	request.set_total_gold(player->total_recharge_gold());
    request.set_day_total_gold(player->daily_recharge_dtail().__today_recharge_gold);
	return MAP_MONITOR->dispatch_to_logic(this, &request);
}

int MLPacker::act_serial_info_update(int type, Int64 id, Int64 value, int sub_1, int sub_2)
{
	Proto30103102 request;
	request.set_type(type);
	request.set_id(id);
	request.set_value(value);
	request.set_sub_1(sub_1);
	request.set_sub_2(sub_2);

	return MAP_MONITOR->dispatch_to_logic(this, &request);
}

int MLPacker::recharge(int gold, int serial_type /*=ADD_FROM_BACK_RECHARGE*/)
{
	this->pack_money_add(Money(gold), serial_type);

	MapLogicPlayer* player = this->map_logic_player();
	player->record_recharge_awards(gold, serial_type);
	player->update_recharge_rebate(gold);

	player->check_in_pa_event();
	player->task_listen_branch(GameEnum::BRANCH_FIRST_RECHARGE, 1);
    player->handle_recharge_event(gold);
    player->MLTotalRecharge::process_rechare_event();
    player->change_magic_weapon_status(GameEnum::RAMA_GET_FROM_RECHARGE, gold);
	player->justify_recharge_gold(); //MLVipPaler
	player->fetch_super_vip_info_begin();

	this->recharge_info_update(gold);
//	this->handle_open_gift_recharge(gold);

	MSG_USER("%s:recharge gold:%d, total:%d", player->name(), gold, player->total_recharge_gold());
    return 0;
}

int MLPacker::pack_sort_and_merge(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400104*, request, RETURN_SORT_PACKAGE);

	GamePackage* package = this->find_package(request->pack_type());
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_SORT_PACKAGE,
			ERROR_CLIENT_OPERATE);

	package->sort_and_merge(request->remove());
	this->notify_pack_info(request->pack_type());

	FINER_PROCESS_NOTIFY(RETURN_SORT_PACKAGE);
}

int MLPacker::pack_move(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400105*, request, RETURN_MERGE_PACKAGE);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_MERGE_PACKAGE,
			ERROR_CLIENT_OPERATE);

	PackageItem* item_from = package->find_by_index(request->index_from());
	PackageItem* item_to = package->find_by_index(request->index_to());
	CONDITION_NOTIFY_RETURN(item_from != NULL, RETURN_MERGE_PACKAGE,
			ERROR_CLIENT_OPERATE);

	if (item_to != NULL)
	{
		this->pack_insert_with_notify(package, request->index_to(), item_from);
		this->pack_insert_with_notify(package, request->index_from(), item_to);
	}
	else
	{
		this->pack_remove_with_notify(package, item_from);
		this->pack_insert_with_notify(package, request->index_to(), item_from);
	}

	FINER_PROCESS_NOTIFY(RETURN_MERGE_PACKAGE);
}

int MLPacker::fetch_item_detail_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400111*, request, RETURN_PACK_GRID_CLICK);

	GamePackage* package = this->find_package(request->pack_type());
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_PACK_GRID_CLICK,
			ERROR_CLIENT_OPERATE);

	PackageItem* pack_item = package->find_by_index(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_PACK_GRID_CLICK,
			ERROR_CLIENT_OPERATE);

	Proto51400111 respond;
	respond.set_index(request->index());
	respond.set_pack_type(request->pack_type());

	respond.set_item_id(pack_item->__id);
	respond.set_item_bind(pack_item->__bind);
	FINER_PROCESS_RETURN(RETURN_PACK_GRID_CLICK, &respond);
}

int MLPacker::drop_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400107*, request, RETURN_DROP_GOODS);

	int index = request->index();
	int pack_type = request->pack_type();

	PackageItem* pack_item = this->pack_find(index, pack_type);
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_DROP_GOODS, ERROR_PACKAGE_NOT_EXISTS);

	const Json::Value &item_json = CONFIG_INSTANCE->item(pack_item->__id);
	CONDITION_NOTIFY_RETURN(item_json != Json::Value::null, RETURN_DROP_GOODS, ERROR_CONFIG_NOT_EXIST);
	CONDITION_NOTIFY_RETURN(item_json["cannotDestroy"].asInt() == 0, RETURN_DROP_GOODS, ERROR_ITEM_CANT_DROP);

	int ret = this->pack_remove_by_index(ITEM_PLAYER_DROP, index, pack_type);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_DROP_GOODS, ret);

	FINER_PROCESS_NOTIFY(RETURN_DROP_GOODS);
}

int MLPacker::add_gift_pack_money(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	return 0;
}

int MLPacker::use_gift_pack(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	ItemObjMap items_map;
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);

	for (int i = 0; i < amount; ++i)
	{
		int reward_id = effect["reward_id"].asInt();
		this->add_reward(reward_id, SerialObj(ADD_FROM_ITEM_GIFT_PACK, pack_item->__id), true);

		const ItemObjMap& add_result = package->get_insert_result().obj_map_;
		for (ItemObjMap::const_iterator iter = add_result.begin();
				iter != add_result.end(); ++iter)
		{
			const ItemObj& obj = iter->second;
			items_map[obj.id_].id_		= obj.id_;
			items_map[obj.id_].amount_	+= obj.amount_;
			items_map[obj.id_].bind_ 	= obj.bind_;
		}
	}

	this->notify_item_map_tips(items_map);
	this->notify_gift_pack_show(items_map);
	return 0;
}

int MLPacker::use_fix_gift_pack(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	return 0;
}

int MLPacker::use_resp_gif_pack(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	return 0;
}

int MLPacker::use_total_gift_pack(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	return 0;
}

int MLPacker::open_box_by_key(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	return 0;
}

int MLPacker::use_rand_gif_pack(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	return 0;
}

int MLPacker::use_magic_act_pack(const Json::Value& effect, PackageItem* pack_item, const int amount)
{
	return 0;
}

int MLPacker::pack_insert(const SerialObj& serial_obj, const ItemObj& item_obj,
		int pack_type, Int64 src_role_id)
{
	GamePackage* package = this->find_package(pack_type);
	return this->pack_insert(package, serial_obj, item_obj, src_role_id);
}

int MLPacker::pack_insert(const SerialObj& serial_obj, const ProtoItem& proto_item,
		int pack_type, Int64 src_role_id)
{
	GamePackage* package = this->find_package(pack_type);

	RewardInfo reward_info;
	reward_info.add_rewards(proto_item);

	return this->pack_insert(package, serial_obj, reward_info, src_role_id);
}

int MLPacker::pack_insert(const SerialObj& serial_obj, PackageItem* pack_item,
		int pack_type, Int64 src_role_id)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, ERROR_CLIENT_OPERATE);
	return this->pack_insert(package, serial_obj, pack_item, src_role_id);
}

int MLPacker::pack_insert(GamePackage* package, const SerialObj& serial_obj,
		const ItemObj& item_obj, Int64 src_role_id)
{
	RewardInfo reward_info;
	reward_info.add_rewards(item_obj);
	return this->pack_insert(package, serial_obj, reward_info, src_role_id);
}

int MLPacker::pack_insert(GamePackage* package, const SerialObj& serial_obj,
		const ItemObjVec& item_obj_vec, Int64 src_role_id)
{
	RewardInfo reward_info;
	reward_info.add_rewards(item_obj_vec);
	return this->pack_insert(package, serial_obj, reward_info, src_role_id);
}

int MLPacker::pack_insert(GamePackage* package, const SerialObj& serial_obj,
		const RewardInfo& reward_info, Int64 src_role_id)
{
	JUDGE_RETURN(package != NULL, ERROR_SERVER_INNER);

	int ret = package->insert_with_res(reward_info.item_vec_);
	JUDGE_RETURN(ret == 0, ret);

	this->add_game_resource(reward_info.resource_map_, serial_obj);
	this->pack_money_add(reward_info.money_, serial_obj);
	this->request_add_exp(reward_info.exp_, serial_obj);
	this->dispatch_to_logic_league(0, reward_info.contri_);
	this->pack_shout_item(reward_info);

	this->pack_insert_result(package, serial_obj, src_role_id);
	return 0;
}

int MLPacker::insert_package(const SerialObj& serial, int item_id,
		int item_amount, int item_bind, Int64 src_role_id)
{
	ItemObj item_obj(item_id, item_amount, item_bind);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	return this->pack_insert(package, serial, item_obj, src_role_id);
}

int MLPacker::insert_package(const SerialObj& serial, const RewardInfo& reward_info,
		Int64 src_role)
{
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	return this->pack_insert(package, serial, reward_info, src_role);
}

int MLPacker::insert_package(const SerialObj& serial, const ItemObjVec& item_obj_vec, Int64 src_role)
{
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	return this->pack_insert(package, serial, item_obj_vec, src_role);
}

int MLPacker::pack_insert(GamePackage* package, const SerialObj& serial, PackageItem* pack_item, Int64 src_role)
{
	int index = package->search_empty_index_i();
	JUDGE_RETURN(index >= 0, ERROR_NO_ENOUGH_SPACE);

	int ret = package->insert_with_res(index, pack_item);
	JUDGE_RETURN(ret == 0, ret);

	// result: serial, notify and db
	return this->pack_insert_result(package, serial, src_role);
}

int MLPacker::pack_insert_with_notify(GamePackage* package, PackageItem* pack_item)
{
	JUDGE_RETURN(package != NULL && pack_item != NULL, ERROR_CLIENT_OPERATE);
	return this->pack_insert_with_notify(package, pack_item->__index, pack_item);
}

int MLPacker::pack_insert_with_notify(GamePackage* package, int item_index, PackageItem* pack_item)
{
	JUDGE_RETURN(package != NULL && pack_item != NULL, ERROR_CLIENT_OPERATE);

	// insert and notify
	int ret = package->insert_with_res(item_index, pack_item);
	JUDGE_RETURN(ret == 0, ret);

	this->pack_insert_res_notify(package);
	return 0;
}

int MLPacker::pack_insert_result(GamePackage* package, const SerialObj& serial, Int64 src_role)
{
	JUDGE_RETURN(package != NULL, -1);

	const PackInsertResult& insert = package->get_insert_result();
	JUDGE_RETURN(insert.error_no_ == 0, -1);

	// serail
	this->pack_insert_res_serial(package, serial, src_role);

    // notify
    this->pack_insert_res_notify(package);

    // tips
    this->pack_insert_res_tips(package, serial);

    // task listen
    this->pack_insert_res_task_listen(package);

	//auto use
	this->pack_insert_res_auto_use(package);

    // modify operate
    this->pack_insert_res_other(package, serial);

    // update db
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

	return 0;
}

int MLPacker::pack_insert_res_serial(GamePackage* package, const SerialObj& serial, Int64 src_role)
{
	const PackInsertResult& insert = package->get_insert_result();
	for (ItemObjMap::const_iterator iter = insert.obj_map_.begin();
			iter != insert.obj_map_.end(); ++iter)
	{
		const ItemObj& obj = iter->second;
		this->record_item_serial(serial, obj, src_role);
		this->serial_record().item_serial(serial.type_, obj.id_, obj.amount_);
	}

	return 0;
}


int MLPacker::pack_insert_res_notify(GamePackage* package)
{
	Proto81400101 respond;
	respond.set_pack_type(package->type());

	const PackInsertResult& insert = package->get_insert_result();
	for (ItemObjMap::const_iterator iter = insert.obj_map_.begin();
			iter != insert.obj_map_.end(); ++iter)
	{
		PackageItem* pack_item = package->find_by_index(iter->first);
		JUDGE_CONTINUE(pack_item != NULL);

		ProtoItem* proto_item = respond.add_pack_item_list();
		package->serialize(proto_item, pack_item);
	}

	FINER_PROCESS_RETURN(ACTIVE_ADD_GOODS, &respond);
}

int MLPacker::pack_insert_res_task_listen(GamePackage* package)
{
	MapLogicPlayer* player = this->map_logic_player();
	const PackInsertResult& insert = package->get_insert_result();

	for (ItemObjMap::const_iterator iter = insert.obj_map_.begin();
			iter != insert.obj_map_.end(); ++iter)
	{
		const ItemObj& obj = iter->second;
		JUDGE_CONTINUE(player->is_task_listen_item(obj.id_) == true)

	    int unbind_item_count = this->pack_unbind_count(obj.id_);
	    int bind_item_count = this->pack_bind_count(obj.id_);
	    int total_item_couunt = unbind_item_count + bind_item_count;

	    if (unbind_item_count > 0)
	    {
	    	player->task_listen_package_item(obj.id_, unbind_item_count, 0);
	    }

	    if (bind_item_count > 0)
	    {
	    	player->task_listen_package_item(obj.id_, bind_item_count, 1);
	    }

	    if (total_item_couunt > 0)
	    {
	    	player->task_listen_package_item(obj.id_, total_item_couunt, -1);
	    }
	}

	return 0;
}

int MLPacker::pack_insert_res_tips(GamePackage* package, const SerialObj& serial)
{
	TipsPlayer tips(this);
	MapLogicPlayer* player = this->map_logic_player();

	int no_tips = CONFIG_INSTANCE->serial_no_tips(serial.type_);
	const PackInsertResult& insert = package->get_insert_result();

	for (ItemObjMap::const_iterator iter = insert.obj_map_.begin();
			iter != insert.obj_map_.end(); ++iter)
	{
		const ItemObj& obj = iter->second;
		player->check_pa_event_when_item_update(obj.id_, obj.amount_); //red tips

		JUDGE_CONTINUE(no_tips == false);
		tips.push_goods(obj.id_, obj.amount_); //message tips
	}

	return 0;
}

int MLPacker::pack_insert_res_other(GamePackage* package, const SerialObj& serial)
{
	return 0;
}

int MLPacker::pack_insert_res_auto_use(GamePackage* package)
{
	JUDGE_RETURN(package->is_package() == true, -1);

	IntMap index_map;
	const PackInsertResult& insert = package->get_insert_result();
	for (ItemObjMap::const_iterator iter = insert.obj_map_.begin();
			iter != insert.obj_map_.end(); ++iter)
	{
		index_map[iter->first] = true;
	}

	for (IntMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		this->check_auto_use_item(package, iter->first);
	}

	return 0;
}

int MLPacker::pack_remove(const SerialObj& serial, int item_id, int item_amount,
		int pack_type, Int64 des_role)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, ERROR_CLIENT_OPERATE);

	return this->pack_remove(package, serial, item_id, item_amount, des_role);
}

int MLPacker::pack_remove(const SerialObj& serial,
		PackageItem* pack_item, int item_amount, int pack_type, Int64 des_role)
{
	GamePackage* package = this->find_package(pack_type);
	return this->pack_remove(package, serial, pack_item, item_amount, des_role);
}

int MLPacker::pack_remove(GamePackage* package, const SerialObj& serial,
		PackageItem* pack_item,	int item_amount, Int64 des_role)
{
	JUDGE_RETURN(package != NULL && pack_item != NULL, ERROR_CLIENT_OPERATE);

	// remove
	int ret = package->remove_item(pack_item, item_amount);
	JUDGE_RETURN(ret == 0, ret);

	// result: serial, notify and db
	return this->pack_remove_result(package, serial, des_role);
}

int MLPacker::pack_remove(GamePackage* package, const SerialObj& serial,
		int item_id, int item_amount, Int64 des_role)
{
	// remove
	int ret = package->remove_by_id(item_id, item_amount);
	JUDGE_RETURN(ret == 0, ret);

	// result: serial, notify and db
	return this->pack_remove_result(package, serial, des_role);
}

int MLPacker::pack_remove(GamePackage* package, const SerialObj& serial,
		const ItemObjVec& item_vec, Int64 des_role)
{
	int ret = this->pack_check(package, item_vec);
	JUDGE_RETURN(ret == 0, ret);

	for (ItemObjVec::const_iterator iter = item_vec.begin();
			iter != item_vec.end(); ++iter)
	{
		this->pack_remove(package, serial, iter->id_, iter->amount_);
	}

	return 0;
}

int MLPacker::pack_remove(GamePackage* package, const SerialObj& serial,
		PackageItem* pack_item, Int64 des_role)
{
	JUDGE_RETURN(package != NULL && pack_item != NULL, ERROR_CLIENT_OPERATE);
	return this->pack_remove(package, serial, pack_item, pack_item->__amount, des_role);
}

int MLPacker::pack_remove_by_index(const SerialObj& serial,
		const ItemIndexObj& item_obj, int pack_type, Int64 des_role)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, ERROR_CLIENT_OPERATE);

	PackageItem* pack_item = package->find_by_index(item_obj.index_);
	JUDGE_RETURN(pack_item != NULL, ERROR_CLIENT_OPERATE);

	int item_amount = ADJUST_NO_ZERO(item_obj.amount_, pack_item->__amount);
	return this->pack_remove(package, serial, pack_item, item_amount, des_role);
}

int MLPacker::pack_remove_with_notify(GamePackage* package, PackageItem* pack_item)
{
	JUDGE_RETURN(package != NULL && pack_item != NULL, ERROR_CLIENT_OPERATE);

	package->remove_item_index(pack_item);
	this->pack_remove_notify(pack_item);

	return 0;
}

int MLPacker::pack_remove_result(GamePackage* package, const SerialObj& serial, Int64 des_role)
{
	JUDGE_RETURN(package != NULL, ERROR_CLIENT_OPERATE);

	// serial
    this->pack_remove_serial(package, serial, des_role);

    // notify
    this->pack_remove_notify(package);

    // update db
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);
	return 0;
}

int MLPacker::pack_remove_serial(GamePackage* package, const SerialObj& serial, Int64 des_role)
{
	JUDGE_RETURN(package != NULL, -1);

	const PackRemoveResult& remove_result = package->get_remove_result();
	JUDGE_RETURN(remove_result.error_no_ == 0, -1);

	ItemObj item_obj(remove_result.item_id_, remove_result.unbind_count_ + remove_result.bind_count_,
			remove_result.bind_count_ > 0 ? GameEnum::ITEM_BIND : GameEnum::ITEM_NO_BIND);
	return this->record_item_serial(serial, item_obj, des_role);
}

int MLPacker::pack_remove_notify(GamePackage* package)
{
	JUDGE_RETURN(package != NULL, -1);

	const PackRemoveResult& remove_result = package->get_remove_result();
	JUDGE_RETURN(remove_result.error_no_ == 0, -1);

	Proto81400102 respond;
	respond.set_pack_type(package->type());

	for (IntMap::const_iterator iter = remove_result.remove_map_.begin();
			iter != remove_result.remove_map_.end(); ++iter)
	{
		ProtoDelItem* del_item = respond.add_del_item_list();
		JUDGE_CONTINUE(del_item != NULL);

		del_item->set_del_index(iter->first);
		del_item->set_del_amount(iter->second);
	}

	FINER_PROCESS_RETURN(ACTIVE_DEL_GOODS, &respond);
}

int MLPacker::pack_remove_notify(PackageItem* pack_item, int pack_type)
{
	JUDGE_RETURN(pack_item != NULL, -1);
	return this->pack_remove_notify(pack_item->__index, pack_item->__amount, pack_type);
}

int MLPacker::pack_remove_notify(int item_index, int item_amount, int pack_type)
{
	Proto81400102 respond;
	respond.set_pack_type(pack_type);

	ProtoDelItem* del_item = respond.add_del_item_list();
	del_item->set_del_index(item_index);
	del_item->set_del_amount(item_amount);
	FINER_PROCESS_RETURN(ACTIVE_DEL_GOODS, &respond);
}

PackageItem* MLPacker::pack_find(int item_index, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, NULL);
	return package->find_by_index(item_index);
}

PackageItem* MLPacker::pack_find_first(int item_id, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, NULL);
	return package->find_by_id(item_id);
}

PackageItem* MLPacker::pack_find_by_unbind_id(int item_id, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, NULL);
	return package->find_by_unbind_id(item_id);
}

PackageItem* MLPacker::pack_find_by_bind_id(int item_id, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, NULL);
	return package->find_by_bind_id(item_id);
}

PackageItem* MLPacker::pack_find_by_id(int item_id, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, NULL);
	return package->find_by_id(item_id);
}

int MLPacker::pack_move_between_package(int src_index, int src_pkg_type,
		int des_index, int des_pkg_type,
		int item_serial_type, int equip_serial_type)
{
	GamePackage* src_package = this->find_package(src_pkg_type);
	JUDGE_RETURN(src_package != NULL, ERROR_INVALID_PARAM);

	GamePackage* des_package = this->find_package(des_pkg_type);
	JUDGE_RETURN(des_package != NULL, ERROR_INVALID_PARAM);

	PackageItem* src_item = src_package->find_by_index(src_index);
	JUDGE_RETURN(src_item != NULL, ERROR_INVALID_PARAM);

	int src_item_id = src_item->__id, src_item_amount = src_item->__amount;
	int src_item_bind = src_item->__bind, src_item_index = src_item->__index;

	int real_serial_type = equip_serial_type;
	PackageItem* des_item = des_package->find_by_index(des_index);

	if (des_item == NULL)	//身上des_index部位没有装备直接穿上
	{
		des_item = src_item;
		this->pack_remove_with_notify(src_package, src_item);

		des_item->__index = des_index;
		this->pack_insert_with_notify(des_package, des_item);
	}
	else
	{
		this->pack_remove_with_notify(src_package, src_item);
		this->pack_remove_with_notify(des_package, des_item);

		std::swap(src_item, des_item);
		des_item->__index = des_index;
		src_item->__index = src_index;

		this->pack_insert_with_notify(src_package, src_item);
		this->pack_insert_with_notify(des_package, des_item);

		real_serial_type = EQUIP_TAKE_PLACE;
	}

	// 装备流水
	if (real_serial_type != 0)
	{
		this->record_equip_item(des_item, real_serial_type);
	}

	{
		SerialObj serial_obj(item_serial_type);
		ItemObj item(src_item_id, src_item_amount, src_item_bind, src_item_index);
		this->record_item_serial(serial_obj, item);
	}

	// update db
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);
	return 0;
}

int MLPacker::validate_pack_amount(int id, int need_amount, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, false);

	int have_amount = this->pack_count(package, id);
	return have_amount >= need_amount;
}

int MLPacker::pack_money_exchange_with_gold(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400150*, request, RETURN_REQUEST_EXCHANGE_MONEY_WITH_GOLD);
	int gold = request->gold();
	int money_type = request->money_type();

	int never_confirm = request->never_confirm();
	if(never_confirm)
	{
		int recogn = request->recogn();
		int commercial_sys_type = GameCommon::commercial_recogn_type(recogn);
		this->pack_detail().__auto_exchange_copper[commercial_sys_type] = never_confirm;
	}

	Money& pack_money = this->own_money();
	CONDITION_NOTIFY_RETURN((gold > 0) && (pack_money.__gold >= gold),
			RETURN_REQUEST_EXCHANGE_MONEY_WITH_GOLD, ERROR_PACKAGE_GOLD_AMOUNT);

	//process exchange money: between gold and (bind_copper / copper)
	pack_money.__gold -= gold;
	int money_rate = GameCommon::calc_pack_money_exchange_rate(money_type);
	int exchange_amount = gold * money_rate;
	if(money_type == GameEnum::MONEY_BIND_COPPER)
		pack_money.__bind_copper += exchange_amount;
	if(money_type == GameEnum::MONEY_UNBIND_COPPER)
		pack_money.__copper += exchange_amount;

	this->notif_pack_money();

	this->callback_after_exchange_copper(request->recogn());

	FINER_PROCESS_NOTIFY(RETURN_REQUEST_EXCHANGE_MONEY_WITH_GOLD);
}

int MLPacker::notify_lack_gold_suguest(const int gold, const int money_type)
{
	Proto81400109 respond;
	respond.set_need_gold(gold);

	int money_rate = GameCommon::calc_pack_money_exchange_rate(money_type);
	if(money_type == GameEnum::MONEY_BIND_COPPER)
		respond.set_lack_bind_copper(gold * money_rate);
	if(money_type == GameEnum::MONEY_UNBIND_COPPER)
		respond.set_lack_copper(gold * money_rate);

	FINER_PROCESS_RETURN(ACTIVE_LACK_GOLD_SUGUEST, &respond);
}


int MLPacker::notify_exchange_bind_copper(const int need_gold, const int exchange_copper, const int recogn)
{
	this->respond_to_client_error(recogn, ERROR_PACK_BIND_COPPER_AMOUNT);

	Proto81400109 respond;
	respond.set_need_gold(need_gold);
	respond.set_return_recogn(recogn);
	respond.set_lack_bind_copper(exchange_copper);

	FINER_PROCESS_RETURN(ACTIVE_LACK_GOLD_SUGUEST, &respond);
}

/* 1.不需要兑换，或自动兑换所需的铜钱成功 返回0
 * 2.自动兑换钱不足返回相应错误号
 * 3.需通知客户端的话返回 1
 * */

int MLPacker::bind_copper_exchange_or_notify(const Money &need_money, int recogn, const Message *req/*=0*/, const bool force_auto_buy/*=false*/)
{
	this->set_repeat_req(recogn, req);

	Money adjust_money(need_money);
	return this->bind_copper_exchange_or_notify_i(adjust_money, recogn, force_auto_buy);
}

int MLPacker::bind_copper_exchange_or_notify_i(Money &need_money, int recogn, bool force/*=false*/)
{
	Money& pack_money = this->own_money();
    int need_gold = 0, exchange_copper = 0;
    GameCommon::check_copper_adjust_exchange_gold(need_money, pack_money, &need_gold, &exchange_copper);
    JUDGE_RETURN(need_gold > 0 && exchange_copper > 0, 0);

	int commercial_sys_type = GameCommon::commercial_recogn_type(recogn);
	if(false == this->pack_detail().__auto_exchange_copper[commercial_sys_type] && !force)
	{
        this->notify_exchange_bind_copper(need_gold, exchange_copper, recogn);
		return 1;
    }

	Money exchage_money(need_money);
	exchage_money.__gold += need_gold;
	exchage_money.__bind_copper = std::max(int(exchage_money.__bind_copper - exchange_copper), 0);
	GameCommon::adjust_money(exchage_money, pack_money);
	if (exchage_money.__copper > this->own_money().__copper)
		return ERROR_PACKAGE_COPPER_AMOUNT;
	else if (exchage_money.__bind_copper > this->own_money().__bind_copper)
		return ERROR_PACK_BIND_COPPER_AMOUNT;
	else if (exchage_money.__bind_gold > this->own_money().__bind_gold)
		return ERROR_PACK_BIND_GOLD_AMOUNT;
	else if (exchage_money.__gold > this->own_money().__gold)
		return ERROR_PACKAGE_GOLD_AMOUNT;
    
    // 用元宝自动兑换绑铜
    this->exchange_gold_to_bind_copper(need_gold);

    if (this->callback_after_exchange_copper(recogn) == 0)
        return 1;

    return 0;
}

int MLPacker::exchange_gold_to_bind_copper(const int gold)
{
    Money gold_money(gold);
    JUDGE_RETURN(this->validate_money(gold_money), ERROR_PACKAGE_MONEY_AMOUNT);

    int money_rate = GameCommon::calc_pack_money_exchange_rate(GameEnum::MONEY_BIND_COPPER);
    int exchange_copper = money_rate * gold;

    this->own_money().__bind_copper += exchange_copper;
    return this->pack_money_sub(gold_money, SerialObj(SUB_MONEY_BIND_COPPER_ADJ));
}

int MLPacker::request_rename_role_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400120*, request, RETURN_RENAME_ROLE);

//	CONDITION_NOTIFY_RETURN(this->check_is_in_travel_scene() == false,
//			RETURN_RENAME_ROLE, ERROR_IN_TRAVEL_SCENE);

	PackageItem* pack_item = this->pack_find(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL && pack_item->__amount > 0,
			RETURN_RENAME_ROLE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(GameCommon::check_item_effect_value(pack_item,
			GameEnum::ITEM_TYPE_ROLE_RENAME) == true,
			RETURN_RENAME_ROLE, ERROR_CLIENT_OPERATE);

	int change_name_limit = CONFIG_INSTANCE->const_set("change_name_time") * Time_Value::HOUR;
	Int64 cur_tick = ::time(NULL);
	CONDITION_NOTIFY_RETURN(cur_tick >= this->role_detail().change_name_tick_ + change_name_limit,
			RETURN_RENAME_ROLE, ERROR_CHANGE_ROLE_NAME_LIMIT);

    int max_length = request->name().length();
    CONDITION_NOTIFY_RETURN(0 < max_length && max_length <= MAX_NAME_LENGTH,
    		RETURN_RENAME_ROLE, ERROR_NAME_TOO_LONG);

    char buffer[MAX_NAME_LENGTH + 1] = {0,};
    string_remove_black_char(buffer, MAX_NAME_LENGTH, request->name().c_str(), max_length);

    int utf8_length = string_utf8_len(buffer);
    CONDITION_NOTIFY_RETURN(0 < utf8_length && utf8_length <= GameEnum::MAX_PLAYER_NAME_LEN,
    		RETURN_RENAME_ROLE, ERROR_NAME_TOO_LONG);

	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	CONDITION_NOTIFY_RETURN(shop_mode != NULL, RETURN_RENAME_ROLE, ERROR_SERVER_INNER);

	shop_mode->recogn_ = TRANS_PLAYER_ROLE_RENAME;
	shop_mode->input_argv_.type_string_ = buffer;
	shop_mode->input_argv_.type_int64_ = this->role_id();
	shop_mode->input_argv_.type_int_ = this->role_detail().__server_id;
	shop_mode->input_argv_.extra_int_vec_.push_back(pack_item->__index);

	return this->monitor()->db_load_mode_begin(shop_mode, this->role_id());
}

int MLPacker::request_rename_role_done(DBShopMode* shop_mode)
{
	int index = shop_mode->input_argv_.extra_int_vec_[0];

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_RENAME_ROLE,
			ERROR_CLIENT_OPERATE);

	PackageItem* pack_item = package->find_by_index(index);
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_RENAME_ROLE,
			ERROR_CLIENT_OPERATE);

	if (shop_mode->output_argv_.type_int_ == 1)
	{
		this->pack_remove(package, ITEM_PLAYER_USE, pack_item, 1);
		this->map_logic_player()->set_after_role_name(shop_mode->input_argv_.type_string_);
		this->map_logic_player()->handle_after_role_rename();
		this->respond_to_client(RETURN_RENAME_ROLE);
	}
	else
	{
		this->respond_to_client_error(RETURN_RENAME_ROLE,
				shop_mode->output_argv_.int_vec_[0]);
	}

	return 0;
}

int MLPacker::request_rename_league_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400121*, request, RETURN_RENAME_LEAGUE);
	CONDITION_NOTIFY_RETURN(this->role_detail().__league_id > 0,
			RETURN_RENAME_LEAGUE, ERROR_LEAGUE_NO_EXIST);

//	CONDITION_NOTIFY_RETURN(this->check_is_in_travel_scene() == false,
//			RETURN_RENAME_LEAGUE, ERROR_IN_TRAVEL_SCENE);

	PackageItem* pack_item = this->pack_find(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL && pack_item->__amount > 0,
			RETURN_RENAME_LEAGUE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(GameCommon::check_item_effect_value(pack_item,
			GameEnum::ITEM_TYPE_LEAGUE_RENAME) == true,
			RETURN_RENAME_LEAGUE, ERROR_CLIENT_OPERATE);

    char buffer[GameEnum::MAX_LEAGUE_NAME_LENGTH * 6 + 1] = {0};
    string_remove_black_char(buffer, GameEnum::MAX_LEAGUE_NAME_LENGTH * 6,
    		request->name().c_str(), request->name().length());

	string league_name = buffer;
	CONDITION_NOTIFY_RETURN(GameCommon::validate_chinese_name_length(league_name,
			GameEnum::MAX_LEAGUE_NAME_LENGTH) == true,
			RETURN_RENAME_LEAGUE, ERROR_LEAGUE_NAME_LENGTH);

	Proto30400455 league_rename;
	league_rename.set_name(league_name);
	league_rename.set_index(pack_item->__index);
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::LBOSS_SCENE_ID, &league_rename);
}

int MLPacker::request_rename_league_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400024*, request, RETURN_RENAME_LEAGUE);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_RENAME_LEAGUE,
			ERROR_CLIENT_OPERATE);

	PackageItem* pack_item = package->find_by_index(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_RENAME_LEAGUE,
			ERROR_CLIENT_OPERATE);

	if(request->operate() == 1)
	{
		this->pack_remove(package, ITEM_PLAYER_USE, pack_item, 1);

		Proto30100243 inner_res;
		inner_res.set_league_index(this->role_detail().__league_id);
		this->monitor()->dispatch_to_logic(this, &inner_res);
		this->respond_to_client(RETURN_RENAME_ROLE);
	}

	return 0;
}

int MLPacker::request_resex_role_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400122*, request, RETURN_RESEX_ROLE);

//	CONDITION_NOTIFY_RETURN(this->check_is_in_travel_scene() == false,
//			RETURN_RESEX_ROLE, ERROR_IN_TRAVEL_SCENE);

	PackageItem* pack_item = this->pack_find(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL && pack_item->__amount > 0,
			RETURN_RESEX_ROLE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(GameCommon::check_item_effect_value(pack_item,
			GameEnum::ITEM_TYPE_ROLE_SEX) == true,
			RETURN_RESEX_ROLE, ERROR_CLIENT_OPERATE);

	int change_sex_limit = CONFIG_INSTANCE->const_set("change_sex_time") * Time_Value::HOUR;
	Int64 cur_tick = ::time(NULL);
	CONDITION_NOTIFY_RETURN(cur_tick >= this->role_detail().change_sex_tick_ + change_sex_limit,
			RETURN_RESEX_ROLE, ERROR_CHANGE_ROLE_SEX_LIMIT);

	Proto31400050 resex_role;
	resex_role.set_index(pack_item->__index);
	return MAP_MONITOR->dispatch_to_logic(this, &resex_role);
}

int MLPacker::request_resex_role_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400050*, request, RETURN_RESEX_ROLE);

	int operate = request->operate();
	CONDITION_NOTIFY_RETURN(operate == true, RETURN_RENAME_LEAGUE,
			ERROR_WEDDING_CAN_NOT_RESEX);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_RENAME_LEAGUE,
			ERROR_CLIENT_OPERATE);

	PackageItem* pack_item = package->find_by_index(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_RENAME_LEAGUE,
			ERROR_CLIENT_OPERATE);

	this->pack_remove(package, ITEM_PLAYER_USE, pack_item, 1);
	this->map_logic_player()->set_after_role_sex();
	this->map_logic_player()->handle_after_role_sex();
	this->respond_to_client(RETURN_RESEX_ROLE);

	return 0;
}

int MLPacker::uprising_equip_item(int equip_id, int serial)
{
	JUDGE_RETURN(equip_id > 0, ERROR_CONFIG_ERROR);

	GamePackage* equip_pkg = this->find_package(GameEnum::INDEX_EQUIP);
	PackageItem* equip_item = equip_pkg->find_by_id(equip_id);
	JUDGE_RETURN(equip_item != NULL, ERROR_EQUIP_NO_EXIST);

	const Json::Value& equip_conf = equip_item->conf();
	JUDGE_RETURN(equip_conf.isMember("equip_upgrade") == true, ERROR_CONFIG_ERROR);

	int aim_equip = equip_conf["equip_upgrade"].asInt();
	PackageItem* new_equip = GamePackage::pop_item(aim_equip, 1);

	*new_equip = *equip_item;
	new_equip->__id = aim_equip;

	this->pack_remove_with_notify(equip_pkg, equip_item);
	this->pack_insert_with_notify(equip_pkg, new_equip);
	this->record_equip_item(new_equip, serial);

	MapLogicPlayer* player = this->map_logic_player();
	player->refresh_equip_prop_to_map();
	player->refresh_player_equip_shape(new_equip->__index + 1, new_equip->__id);

	GamePackage::push_item(equip_item);

	//检测背包图纸是否可以继续升阶
	this->check_uprising_equip_item(aim_equip);

	return 0;
}

int MLPacker::check_uprising_equip_item(int equip_id)
{
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	ItemListMap& item_map = package->item_map();
	PackageItem *pack_item = NULL;
	for (ItemListMap::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
	{
		PackageItem* item = iter->second;
		const Json::Value& conf = item->conf();
		JUDGE_CONTINUE(conf.empty() == false);
		JUDGE_CONTINUE(conf.isMember("equip_upgrade") == true);
		JUDGE_CONTINUE(conf["equip_upgrade"].asInt() == equip_id);

		pack_item = item;
		break;
	}
	JUDGE_RETURN(pack_item != NULL, -1);

	int ret = this->pack_remove(package, EQUIP_UPGRADE, pack_item, 1);
	JUDGE_RETURN(ret == 0, -1);

	this->uprising_equip_item(equip_id, EQUIP_UPGRADE);
	return 0;
}

int MLPacker::skill_upgrade_use_goods(uint id, uint level, int check)
{
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(id);
    JUDGE_RETURN(level < skill_json["upgrade_goods"].size(), ERROR_CONFIG_ERROR);
    JUDGE_RETURN(this->role_level() >= skill_json["useLvl"].asInt(), ERROR_PLAYER_LEVEL_LIMIT);

    ItemObj obj = GameCommon::make_up_itemobj(skill_json["upgrade_goods"][level - 1]);
    JUDGE_RETURN(obj.validate() == true, ERROR_CONFIG_ERROR);

    int have_amount = this->pack_count(obj.id_);
    JUDGE_RETURN(have_amount > 0, ERROR_PACKAGE_ITEM_AMOUNT);

    if (check == false)
    {
    	return this->pack_remove(SerialObj(ITEM_SKILL_UPGRADE_LEVEL, id),
    	    			obj.id_, obj.amount_);
    }
    else
    {
    	JUDGE_RETURN(have_amount >= obj.amount_, ERROR_PACKAGE_ITEM_AMOUNT);
        return 0;
    }
}

int MLPacker::add_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400323*, request, -1);

	SerialObj obj(request->serial());
	return this->add_reward(request->id(), obj);
}

int MLPacker::add_reward(int id, const SerialObj& obj, int need_player)
{
	const Json::Value& reward_json = CONFIG_INSTANCE->reward(id);
	JUDGE_RETURN(reward_json.empty() == false, ERROR_CONFIG_NOT_EXIST);

	RewardInfo reward_info(true, NULL, this->map_logic_player());
	GameCommon::make_up_reward_items(reward_info, reward_json);

	this->check_del_item(reward_info);
	return this->insert_package(obj, reward_info);
}

int MLPacker::add_game_resource(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400025 *, request, -1);

    SerialObj obj(request->serial_obj());
    return this->add_game_resource(request->item_id(), request->value(), obj);
}

int MLPacker::add_mult_item(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400042 *, request, -1);

	SerialObj obj(request->serial_obj());
	for (int i = 0; i < request->proto_item_size(); ++i)
	{
		const ProtoItem &proto_item = request->proto_item(i);
		this->pack_insert(obj, proto_item);
	}
	return 0;
}

int MLPacker::add_single_item(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400043 *, request, -1);

	return this->insert_package(request->serial_obj(), request->item_id(),
			request->amount(), request->bind());
}

int MLPacker::request_world_boss_enter(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30402203*, reqeust, RETURN_ENTER_WORLD_BOSS);

	int item_fly = CONFIG_INSTANCE->world_boss("item_fly").asInt();

	SerialObj obj1;
	obj1.type_ = ITEM_ENTER_WBOSS_USE_FLY;
	obj1.sub_  = reqeust->boss_scene_id();

	int ret1 = this->pack_remove(obj1, item_fly, 1);
	if (ret1 != 0)
	{
		const Json::Value item_json = CONFIG_INSTANCE->item(item_fly);
		CONDITION_NOTIFY_RETURN(item_json != Json::Value::null,
				RETURN_ENTER_WORLD_BOSS, ERROR_CONFIG_NOT_EXIST);

		int need_bind_gold = item_json["gold_price"][0u].asInt();
		Money bind_money(0, need_bind_gold);

		SerialObj obj2;
		obj2.type_ = SUB_MONEY_ENTER_WBOSS_USE_FLY;
		obj2.sub_  = reqeust->boss_scene_id();

		int ret2 = this->pack_money_sub(bind_money, obj2);
		if (ret2 != 0)
		{
			int need_gold = item_json["gold_price"][1u].asInt();
			Money money(need_gold);
			int ret3 = this->pack_money_sub(money, obj2);
			CONDITION_NOTIFY_RETURN(ret3 == 0, RETURN_ENTER_WORLD_BOSS, ret3);
		}
	}

	return this->send_to_map_thread(*msg);
}


int MLPacker::calc_item_need_money(const int item_id, int &item_amount, Money &cost)
{
    int pack_amount = this->pack_count(item_id);
    int need_amount = item_amount - pack_amount;
    JUDGE_RETURN(need_amount > 0, 0);

    item_amount = pack_amount;

    const Json::Value &item_json = CONFIG_INSTANCE->item(item_id);
    int single_gold = item_json["goldPrice"].asInt();
    cost.__gold += (single_gold * need_amount);
    return 0;
}

int MLPacker::pack_check(GamePackage* package, const ItemObjVec& item_vec)
{
	IntMap item_map;
	for (ItemObjVec::const_iterator iter = item_vec.begin();
			iter != item_vec.end(); ++iter)
	{
		item_map[iter->id_] += iter->amount_;
	}

	for (IntMap::iterator iter = item_map.begin();
			iter != item_map.end(); ++iter)
	{
		int have_amount = package->count_by_id(iter->first);
		JUDGE_CONTINUE(have_amount < iter->second);

		return ERROR_PACKAGE_GOODS_AMOUNT;
	}

	return 0;
}

int MLPacker::pack_count(GamePackage* package, int item_id)
{
	JUDGE_RETURN(package != NULL, 0);
	return package->count_by_id(item_id);
}

int MLPacker::pack_count(int item_id, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	return this->pack_count(package, item_id);
}

int MLPacker::pack_bind_count(int item_id, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, 0);
	return package->count_by_bind_id(item_id);
}

int MLPacker::pack_unbind_count(int item_id, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, 0);
	return package->count_by_unbind_id(item_id);
}

int MLPacker::pack_left_capacity(int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, 0);
	return package->left_capacity();
}

int MLPacker::pack_fetch_item_id(int index, int pack_type)
{
	GamePackage* package = this->find_package(pack_type);
	JUDGE_RETURN(package != NULL, 0);

	PackageItem* item = package->find_by_index(index);
	JUDGE_RETURN(item != NULL, 0);

	return item->__id;
}

int MLPacker::pack_transaction(const SerialObj& remove_serial, const ItemObj& remove_item, Int64 des_role_id,
		const SerialObj& add_serial, const ItemObj& add_item, Int64 src_role_id)
{
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, ERROR_CLIENT_OPERATE);

	// remove and add
	int ret = package->pack_transaction(remove_item, add_item);
	JUDGE_RETURN(ret == 0, ret);

	// result: serial, notify and db
	this->pack_remove_result(package, remove_serial, des_role_id);
	this->pack_insert_result(package, add_serial, src_role_id);

	return 0;
}

int MLPacker::pack_buy(const SerialObj& money_serial, Money& money,
		const SerialObj& add_serial, const ItemObj& add_item)
{
	JUDGE_RETURN(this->validate_money(money) == true, ERROR_PACKAGE_MONEY_AMOUNT);

	int ret = this->pack_insert(add_serial, add_item);
	JUDGE_RETURN(ret == 0, ret);

	this->pack_money_sub(money, money_serial);
	return 0;
}

int MLPacker::pack_money_add(const Money& money, const SerialObj& serial, int is_notify)
{
	JUDGE_RETURN(GameCommon::validate_money(money) == true, ERROR_CLIENT_OPERATE);
    JUDGE_RETURN(this->serial_record().is_validate_money_serial(
    		serial.type_, money) == true, ERROR_SERIAL_TOO_MORE);

    this->pack_money_set(this->pack_detail().__money + money);
	this->pack_money_serail(money, serial);

    int no_tips = CONFIG_INSTANCE->serial_no_tips(serial.type_);
	if (is_notify && !no_tips)
	{
		TipsPlayer tips(this);
		tips.push_money(money);
	}

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);
	return 0;
}

int MLPacker::pack_money_add(const ProtoMoney& proto_money, const SerialObj& serial)
{
	Money add_money;
	add_money.unserialize(proto_money);

	JUDGE_RETURN(GameCommon::validate_money(add_money) == true, -1);
	return this->pack_money_add(add_money, serial);
}

int MLPacker::pack_money_add(const ProtoMoney& proto_money, const ProtoSerialObj& proto_serial, int is_notify)
{
	Money add_money;
	add_money.unserialize(proto_money);

	SerialObj serial_obj;
	serial_obj.unserialize(proto_serial);

	JUDGE_RETURN(GameCommon::validate_money(add_money) == true, -1);
	return this->pack_money_add(add_money, serial_obj, is_notify);
}

int MLPacker::pack_money_item_add(const ItemObjVec& item_obj_set, const SerialObj& serial)
{
	Money total_money;
	for (ItemObjVec::const_iterator iter = item_obj_set.begin();
			iter != item_obj_set.end(); iter++)
	{
		const ItemObj &item = *iter;
		JUDGE_CONTINUE(item.amount_ > 0);

		Money money = GameCommon::money_item_to_money(item);
		JUDGE_CONTINUE(GameCommon::validate_money(money));

		total_money += money;
	}

	return pack_money_add(total_money, serial);
}

int MLPacker::pack_money_sub(const Money& money, const SerialObj& serial)
{
	JUDGE_RETURN(GameCommon::validate_money(money) == true, ERROR_CLIENT_OPERATE);

	PackageDetail& pack_detail = this->pack_detail();
    JUDGE_RETURN(pack_detail.__money.__gold >= money.__gold, ERROR_PACKAGE_GOLD_AMOUNT);
    JUDGE_RETURN(pack_detail.__money.__bind_gold >= money.__bind_gold, ERROR_PACK_BIND_GOLD_AMOUNT);

//    JUDGE_RETURN(pack_money.__copper >= money.__copper, ERROR_PACKAGE_COPPER_AMOUNT);
//    JUDGE_RETURN(pack_money.__bind_copper >= money.__bind_copper, ERROR_PACK_BIND_COPPER_AMOUNT);

    this->pack_money_set(pack_detail.__money - money);
    this->pack_money_serail(money, serial);
    this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);
    JUDGE_RETURN(serial.type_ != SUB_MONEY_MARKET_BUY, 0);

    update_labour_task_info(GameEnum::LABOUR_TASK_EXPEND_BIND_MONEY, money.__bind_gold);
    update_labour_task_info(GameEnum::LABOUR_TASK_EXPEND_MONEY, money.__gold);

	IntMap& use_map = pack_detail.use_resource_map_;
	use_map[GameEnum::ITEM_MONEY_UNBIND_GOLD] += money.__gold;
	use_map[GameEnum::ITEM_MONEY_BIND_GOLD] += money.__bind_gold;
	use_map[GameEnum::ITEM_TODAY_UNBIND_GOLD] += money.__gold;
	use_map[GameEnum::ITEM_TODAY_BIND_GOLD] += money.__bind_gold;
	this->sync_today_consume_gold(money.__gold, money.__bind_gold);
	return 0;
}

int MLPacker::pack_money_serail(const Money& money, const SerialObj& serial)
{
//	SerialInfo& serial_info = this->serial_record().money_serial(serial.type_);
//	serial_info.__copper 		+= money.__copper;
//	serial_info.__gold 			+= money.__gold;
//	serial_info.__bind_gold 	+= money.__bind_gold;
//	serial_info.__bind_copper 	+= money.__bind_copper;

	Money& pack_money = this->pack_detail().__money;
	SERIAL_RECORD->record_money(this, this->agent_code(),
			this->market_code(), serial, money, pack_money);

	return 0;
}

int MLPacker::notif_pack_money(int type)
{
	PackageDetail& pack_detail = this->pack_detail();

	Proto81400103 respond;
	respond.set_gold(pack_detail.__money.__gold);
	respond.set_bind_gold(pack_detail.__money.__bind_gold);

	respond.set_reiki(this->reiki());
	respond.set_exploit(this->exploit());
	respond.set_reputation(this->reputation());
	respond.set_honour(this->honour());
	respond.set_spirit(this->spirit());
	respond.set_practice(this->practice());
	FINER_PROCESS_RETURN(ACTIVE_PACK_MONEY, &respond);
}

int MLPacker::pack_money_set(const Money& money)
{
    this->pack_detail().__money = money;
	return this->notif_pack_money(0);
}

Money& MLPacker::own_money()
{
	return this->pack_detail().__money;
}

bool MLPacker::validate_money(const Money& money)
{
	JUDGE_RETURN(GameCommon::validate_money(money) == true, false);
	return this->pack_detail().__money >= money;
}

bool MLPacker::pack_money_reset(const Money& set_money, const Money& cur_money,
		const SerialObj& serial_obj)
{
	Money left_money = this->pack_detail().__money + cur_money - set_money;
	JUDGE_RETURN(left_money >= Money(0), false);

	this->pack_money_set(left_money);
	return true;
}

int MLPacker::record_item_serial(const SerialObj& serial, const ItemObj& item, Int64 src_role)
{
	return SERIAL_RECORD->record_item(this,	this->agent_code(), this->market_code(),
			serial, item, src_role);
}

int MLPacker::record_other_serial(int main_serial, int sub_serial, Int64 value,
		Int64 ext1, Int64 ext2)
{
	return SERIAL_RECORD->record_other_serial(SerialPlayer(this->role_id(), this),
			this->agent_code(), this->market_code(),
			main_serial, sub_serial, value, ext1, ext2);
}

int MLPacker::client_auto_use_item(GamePackage* package, int pkg_index)
{
	int handle = this->check_auto_use_item(package, pkg_index);
	if (handle == -1)
	{
		//-1表示未处理
		return -1;
	}

	if (handle < 0)
	{
		//自动使用物品失败
		this->respond_to_client_error(RETURN_USE_GOODS, handle);
	}

	return 0;
}

int MLPacker::check_auto_use_item(GamePackage* package, int pkg_index)
{
	PackageItem* item = package->find_by_index(pkg_index);
	JUDGE_RETURN(item != NULL, ERROR_NO_THIS_ITEM);

	if (GameCommon::item_is_equipment(item->__id) == false)
	{
		//自动使用物品
		int ret = this->check_auto_goods_item(item);
		JUDGE_RETURN(ret == 0, ret);

		this->pack_remove(package, ITEM_SYSTEM_AUTO_USE, item);
		return 0;
	}
	else
	{
		//玩家装备
		this->check_auto_equip_item(item);
		return 0;
	}
}

int MLPacker::check_auto_goods_item(PackageItem* item)
{
	const Json::Value& conf = item->conf();
	if (conf.isMember("auto_use_item") == true)
	{
		Proto30400444 goods_info;
		goods_info.set_index(item->__index);
		goods_info.set_amount(item->__amount);
		goods_info.set_id(item->__id);
		return this->map_logic_player()->after_check_use_pack_goods(&goods_info);
	}
	else if (conf.isMember("equip_upgrade") == true)
	{
		return this->uprising_equip_item(conf["equip_upgrade"].asInt(), EQUIP_UPGRADE);
	}

	return -1;
}

int MLPacker::check_auto_equip_item(PackageItem* item)
{
	const Json::Value& conf = item->conf();
	int part_index = conf["part"].asInt() - 1;

	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack->validate_pack_index(part_index) == true, -1);

	return this->map_logic_player()->put_on_equip(item->__index);
}

int MLPacker::validate_game_resource(int item_id, int value)
{
	JUDGE_RETURN(value > 0, ERROR_INVALID_PARAM);

	IntMap need_resource;
	need_resource[item_id] = value;
	return this->validate_game_resource(need_resource);
}

int MLPacker::validate_game_resource(const IntMap& need_resource)
{
	JUDGE_RETURN(need_resource.empty() == false, ERROR_INVALID_PARAM);

	PackageDetail& pack_detail = this->pack_detail();
	for (IntMap::const_iterator iter = need_resource.begin();
			iter != need_resource.end(); ++iter)
	{
		if (iter->second < 0)
		{
			return ERROR_INVALID_PARAM;
		}

		IntMap::iterator have_iter = pack_detail.resource_map_.find(iter->first);
		if (have_iter == pack_detail.resource_map_.end())
		{
			return ERROR_PACKAGE_GOODS_AMOUNT;
		}

		if (have_iter->second < iter->second)
		{
			return ERROR_CLIENT_OPERATE;
		}
	}

	return 0;
}

int MLPacker::add_game_resource(int item_id, int value, const SerialObj& obj)
{
	JUDGE_RETURN(value > 0, ERROR_INVALID_PARAM);

	IntMap add_resource;
	add_resource[item_id] = value;
	return this->add_game_resource(add_resource, obj);
}

int MLPacker::add_game_resource(const IntMap& add_resource, const SerialObj& obj)
{
	JUDGE_RETURN(add_resource.empty() == false, ERROR_CLIENT_OPERATE);

	TipsPlayer tips(this);
	IntMap& resource_map = this->pack_detail().resource_map_;

	for (IntMap::const_iterator iter = add_resource.begin(); iter != add_resource.end(); ++iter)
	{
		resource_map[iter->first] += iter->second;
		tips.push_goods(iter->first, iter->second);

		this->map_logic_player()->check_pa_event_game_resource(iter->first,
				resource_map[iter->first], obj);
		this->record_other_serial(obj.type_, obj.sub_, iter->second,
				iter->first, resource_map[iter->first]);
	}

	return this->notif_pack_money(1);
}

int MLPacker::sub_game_resource(int item_id, int value, const SerialObj& obj)
{
	JUDGE_RETURN(value > 0, ERROR_INVALID_PARAM);

	IntMap sub_resource;
	sub_resource[item_id] = value;
	return this->sub_game_resource(sub_resource, obj);
}

int MLPacker::sub_game_resource(const IntMap& sub_resource, const SerialObj& obj)
{
	int ret = this->validate_game_resource(sub_resource);
	JUDGE_RETURN(ret == 0, ret);

	IntMap& resource_map = this->pack_detail().resource_map_;
	for (IntMap::const_iterator iter = sub_resource.begin();
			iter != sub_resource.end(); ++iter)
	{
		resource_map[iter->first] -= iter->second;

		this->map_logic_player()->check_pa_event_game_resource(iter->first,
				resource_map[iter->first], obj);
		this->record_other_serial(obj.type_, obj.sub_,	iter->second,
				iter->first, resource_map[iter->first]);
	}

	return this->notif_pack_money(1);
}

int MLPacker::fetch_game_resource(int item_id)
{
	PackageDetail& pack_detail = this->pack_detail();
	JUDGE_RETURN(pack_detail.resource_map_.count(item_id) > 0, 0);

	return pack_detail.resource_map_[item_id];
}

int MLPacker::exploit(void)
{
	return this->fetch_game_resource(GameEnum::ITEM_ID_EXPLOIT);
}

int MLPacker::exploit_sub(const int exploit, const SerialObj &serial_obj)
{
	JUDGE_RETURN(exploit > 0, ERROR_INVALID_PARAM);
    return this->sub_game_resource(GameEnum::ITEM_ID_EXPLOIT, exploit, serial_obj);
}

int MLPacker::dispatch_to_logic_league(const int item_id, const int add_num)
{
	JUDGE_RETURN(add_num > 0, -1);

	Proto30101802 respond;
	respond.set_add_num(add_num);
	return MAP_MONITOR->dispatch_to_logic(this, &respond);
}

int MLPacker::honour(void)
{
	return this->fetch_game_resource(GameEnum::ITEM_ID_HONOUR);
}

int MLPacker::change_honour(const int honour, const SerialObj &serial_obj)
{
	if (honour > 0)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_HONOUR, honour, serial_obj);
	}
	else
	{
		return this->sub_game_resource(GameEnum::ITEM_ID_HONOUR, -1 * honour, serial_obj);
	}
}

int MLPacker::reputation(void)
{
	return this->fetch_game_resource(GameEnum::ITEM_ID_REPUTATION);
}

int MLPacker::change_reputation(const int reputation, const SerialObj &serial_obj)
{
	if (reputation > 0)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_REPUTATION, reputation, serial_obj);
	}
	else
	{
		return this->sub_game_resource(GameEnum::ITEM_ID_REPUTATION, -1 * reputation, serial_obj);
	}
}

int MLPacker::reiki(void)
{
	return this->fetch_game_resource(GameEnum::ITEM_ID_REIKI);
}

int MLPacker::change_reiki(const int reiki_num, const SerialObj &serial_obj)
{
	if (reiki_num > 0)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_REIKI, reiki_num, serial_obj);
	}
	else
	{
		return this->sub_game_resource(GameEnum::ITEM_ID_REIKI, -1 * reiki_num, serial_obj);
	}
}

int MLPacker::spirit(void)
{
	return this->fetch_game_resource(GameEnum::ITEM_ID_SPIRIT);
}

int MLPacker::change_spirit(const int spirit_num, const SerialObj &serial_obj)
{
	if (spirit_num > 0)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_SPIRIT, spirit_num, serial_obj);
	}
	else
	{
		return this->sub_game_resource(GameEnum::ITEM_ID_SPIRIT, -1 * spirit_num, serial_obj);
	}
}

int MLPacker::practice(void)
{
	return this->fetch_game_resource(GameEnum::ITEM_ID_PRACTICE);
}

int MLPacker::change_practice(const int practice, const SerialObj &serial_obj)
{
	if (practice > 0)
	{
		return this->add_game_resource(GameEnum::ITEM_ID_PRACTICE, practice, serial_obj);
	}
	else
	{
		return this->sub_game_resource(GameEnum::ITEM_ID_PRACTICE, -1 * practice, serial_obj);
	}
}

void MLPacker::clean_all_pack()
{
    this->clean_pack(GameEnum::INDEX_EQUIP);
    this->clean_pack(GameEnum::INDEX_PACKAGE);
    this->clean_pack(GameEnum::INDEX_STORE);
    this->clean_pack(GameEnum::INDEX_BOX_STORE);
    this->clean_pack(GameEnum::INDEX_MOUNT);
}

void MLPacker::clean_pack(const int pack_type/*= GameEnum::INDEX_PACKAGE*/)
{
    GamePackage* package = this->find_package(pack_type);
    JUDGE_RETURN( package != NULL, ;);

    ItemVector item_vec;
    for (ItemListMap::iterator iter = package->item_list_map_.begin();
            iter != package->item_list_map_.end(); ++iter)
    {
        item_vec.push_back(iter->second);
    }

    for(ItemVector::iterator it = item_vec.begin(); it != item_vec.end(); it++)
    {
        PackageItem* pack_item = *it;
        JUDGE_CONTINUE(pack_item != NULL);
        this->pack_remove(package, ITEM_REMOVE_FROM_TEST, pack_item, pack_item->__amount);
    }
}

// 记录装备流水
void MLPacker::record_equip_item(PackageItem* pack_item, int serial)
{
	JUDGE_RETURN(pack_item != NULL, ;);

	RecordEquipObj obj(GameEnum::INDEX_EQUIP, pack_item);
	SERIAL_RECORD->record_equipment(this, this->agent_code(), this->market_code(), serial, serial, obj);
}

// 道具传闻
void MLPacker::pack_shout_item(const RewardInfo& reward_info)
{
	JUDGE_RETURN(reward_info.id_ > 0, ;);

	const Json::Value& conf = reward_info.conf();
	JUDGE_RETURN(conf.isMember("shout") == true, ;);

	int shout_id = conf["shout"][0u].asInt();
	int item_id = conf["shout"][1u].asInt();

	for (ItemObjVec::const_iterator iter = reward_info.item_vec_.begin();
			iter != reward_info.item_vec_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->id_ == item_id);

		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, this->name());
		GameCommon::push_brocast_para_item_name(para_vec, item_id);
		GameCommon::announce(shout_id, &para_vec);
		break;
	}
}

int MLPacker::update_labour_task_info(int type, int value)
{
	JUDGE_RETURN(MAP_MONITOR->is_in_big_act_time() && value > 0, 0);

	Proto31403201 msg;
	msg.set_task_finish_count(value);
	msg.set_task_id(type);

	return MAP_MONITOR->dispatch_to_logic(this, &msg);
}
