/*
 * MLLeaguer.cpp
 *
 *  Created on: Dec 10, 2013
 *      Author: peizhibi
 */

#include "MLLeaguer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapLogicLabel.h"
#include "SerialRecord.h"
#include "MapLogicPlayer.h"
#include "GameFont.h"

MLLeaguer::MLLeaguer()
{
	// TODO Auto-generated constructor stub
}

MLLeaguer::~MLLeaguer()
{
	// TODO Auto-generated destructor stub
}

void MLLeaguer::reset(void)
{
    this->join_tick_ = 0;
}

int MLLeaguer::sync_league_info(Message* msg)
{
    return 0;
}

int MLLeaguer::logic_league_donate(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400301*, request, RETURN_LEAGUE_DONATE);

	int donate_type = request->donate_type();
	const Json::Value& donate_item = CONFIG_INSTANCE->league("donate")[donate_type];
	CONDITION_NOTIFY_RETURN(donate_item != Json::Value::null, RETURN_LEAGUE_DONATE,
			ERROR_SERVER_INNER);

	switch(donate_type)
	{
	case GameEnum::LEAGUE_WAND_DONATE:
	{
		Int64 donate_id   = donate_item["donate_id"].asInt();
		int donate_number = request->donate_number() * GameEnum::LEAGUE_WAND_TIMES;
		CONDITION_NOTIFY_RETURN(this->pack_remove(ITEM_LEAGUE_DONATE, donate_id, donate_number) == 0,
				RETURN_LEAGUE_DONATE, ERROR_DONATE_NOT_ENOUGH);
		break;
	}

	case GameEnum::LEAGUE_GOLD_DONATE:
	{
		Money money;
		money.__gold = request->donate_number();

		GameCommon::adjust_money(money, this->own_money());
		int ret = this->pack_money_sub(money, SUB_MONEY_LEAGUE_DONATE);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_LEAGUE_DONATE, ret);
		break;
	}
	}

//	int add_contri = request->donate_number() * donate_item["contribute"].asDouble();
//	this->change_league_contri(add_contri, SerialObj(LEAGUE_CONTRI_SERIAL,SUB_LEAGUE_CONTRI_ADD));

	return MAP_MONITOR->dispatch_to_logic(this, msg);
}

int MLLeaguer::logic_summon_boss(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400322*, request, RETURN_SUMMON_BOSS);

	int summon_type = request->summon_type();
	if (summon_type == 2)
	{
		int summon_cost = CONFIG_INSTANCE->league_boss("summon_cost").asInt();
		Money need_money(summon_cost);
		CONDITION_NOTIFY_RETURN(this->own_money() >= need_money,
					RETURN_SUMMON_BOSS, ERROR_PACKAGE_GOLD_AMOUNT);
		int ret = this->pack_money_sub(need_money, SUB_MONEY_SUMMON_BOSS);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SUMMON_BOSS, ret);

		//超级召唤奖励，直接发背包
		const Json::Value& boss_info = CONFIG_INSTANCE->boss_info(request->boss_index());
		int super_drop = boss_info["super_drop"].asInt();

		SerialObj obj(ADD_FROM_SUPER_SUMMON , 0, 0);
		this->add_reward(super_drop, obj);
	}
	//跳转到地图服地图线程
	Proto30400451 req;
	req.set_boss_index(request->boss_index());
	req.set_summon_type(summon_type);
	req.set_league_index(request->league_index());
	return this->monitor()->dispatch_to_scene(this, GameEnum::LBOSS_SCENE_ID, &req);
}

int MLLeaguer::logic_feed_league_boss(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400321*, request, RETURN_FEED_LEAGUE_BOSS);

	int item_id  = request->item_id();
	int feed_num = request->feed_num();
	CONDITION_NOTIFY_RETURN(this->pack_remove(ITEM_FEED_LEAGUE_BOSS, item_id, feed_num) == 0, RETURN_FEED_LEAGUE_BOSS, ERROR_FEED_NOT_ENOUGH);

	return MAP_MONITOR->dispatch_to_logic(this, msg);
}

int MLLeaguer::logic_create_league(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400302*, request, RETURN_LEAGUE_CREATE);

	if (request->create_type() == 1)
	{
		int bind_gold_id = CONFIG_INSTANCE->league("bind_gold_id").asInt();
		int create_bind_gold = CONFIG_INSTANCE->league("create_bind_gold").asInt();
		int create_level = CONFIG_INSTANCE->league("create_level").asInt();
		CONDITION_NOTIFY_RETURN(this->role_level() >= create_level,
					RETURN_LEAGUE_CREATE, ERROR_PLAYER_LEVEL);
		CONDITION_NOTIFY_RETURN(bind_gold_id > 0 && create_bind_gold > 0,
					RETURN_LEAGUE_CREATE, ERROR_SERVER_INNER);

		Money need_money(0, create_bind_gold);
		CONDITION_NOTIFY_RETURN(this->own_money() >= need_money,
					RETURN_LEAGUE_CREATE, ERROR_PACKAGE_GOLD_AMOUNT);

		int ret = this->pack_money_sub(need_money, SUB_MONEY_LEAGUE_CREATE);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_LEAGUE_CREATE,	ret);
	}
	else
	{
		int gold_id = CONFIG_INSTANCE->league("gold_id").asInt();
		int create_gold = CONFIG_INSTANCE->league("create_gold").asInt();
		MSG_USER("gold_id: %d, create_gold: %d, create_level: %d", gold_id, create_gold);
		CONDITION_NOTIFY_RETURN(gold_id > 0 && create_gold > 0, RETURN_LEAGUE_CREATE,
					ERROR_SERVER_INNER);

		Money need_money(create_gold);
		CONDITION_NOTIFY_RETURN(this->own_money() >= need_money,
				RETURN_LEAGUE_CREATE, ERROR_PACKAGE_GOLD_AMOUNT);

		int ret = this->pack_money_sub(need_money, SUB_MONEY_LEAGUE_CREATE);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_LEAGUE_CREATE, ret);
	}

	MapLogicPlayer* player = this->map_logic_player();
	player->role_detail().__league_id = 1;
    if (player->role_level() >= CONFIG_INSTANCE->task_setting()["league_routine_start_level"].asInt())
    	player->league_task_construct_routine(true);

    player->task_listen_branch(GameEnum::BRANCH_LEAGUE, 1);

	return MAP_MONITOR->dispatch_to_logic(this, msg);
}

int MLLeaguer::logic_league_shop_buy(Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto31400303*, request, RETURN_LEAGUE_SHOP_BUY);
//
//	int ret = 0;
//	ItemObj add_item(request->buy_item(), request->buy_num(), GameEnum::ITEM_BIND);
//
//	switch (request->need_type())
//	{
//	case GameEnum::MONEY_MATERIAL:
//	{
//		ItemObj remove_item(request->need_item(), request->need_amount());
//		ret = this->pack_transaction(ITEM_SHOP_BUY_REMOVE,
//				remove_item, 0, ITEM_LEAGUE_SHOP_BUY, add_item, 0);
//		break;
//	}
//
//	case GameEnum::MONEY_CONTRIBUTE:
//	{
//		ret = this->pack_insert(ITEM_LEAGUE_SHOP_BUY, add_item);
//		break;
//	}
//
//	default:
//	{
//		Money money = GameCommon::make_up_money(request->need_amount(),
//				request->need_type());
//
//		ret = this->pack_buy(SUB_MONEY_LEAGUE_SHOP_BUY, money,
//				ITEM_LEAGUE_SHOP_BUY, add_item);
//		break;
//	}
//	}
//
//	request->set_oper_result(ret == 0);
//	this->handle_operate_result(RETURN_LEAGUE_SHOP_BUY, ret);
//
//	return MAP_MONITOR->dispatch_to_logic(this, request);
	return 0;
}

int MLLeaguer::logic_sync_league_id(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400423 *, request, -1);

    this->role_detail().__league_id = request->league_index();
    this->role_detail().__league_name = request->league_name();

    return 0;
}

int MLLeaguer::lstore_insert(Message *msg)
{
//	DYNAMIC_CAST(Proto31400316 *, request, msg);
//	int item_id = request->item_id(),item_num = request->item_num(),item_index = request->item_index();
//
//	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
//	JUDGE_RETURN(package != NULL,ERROR_SERVER_INNER);
//
//	PackageItem* item = package->find_by_index(item_index);
//	CONDITION_NOTIFY_RETURN(item != NULL && item->__amount >= item_num && item->__bind < GameEnum::ITEM_BIND,
//			RETURN_LSTORE_INSERT,ERROR_CLIENT_OPERATE);
//
//	Proto30100237 respond;
//	ProtoItem* proto_item = respond.mutable_item();
//	item->serialize(proto_item,-1,item_num);
//
//	respond.set_package_index(request->package_index());
//	int ret = package->remove_item(item_index,item_num);
//	JUDGE_RETURN(ret == 0,ret);
//	this->notify_pack_info(GameEnum::INDEX_PACKAGE);
//
//    ItemObj item_obj(item_id, item_num,GameEnum::ITEM_NO_BIND);
//    SERIAL_RECORD->record_item(this,this->agent_code(), this->market_code(),
//    		ITEM_INSERT_LSTORE, item_obj, this->role_id());
//	return this->monitor()->dispatch_to_logic(this,&respond);
	return 0;
}

int MLLeaguer::lstore_insert_failed(Message *msg)
{
//	DYNAMIC_CAST(Proto31400320 *, request, msg);
//	const ProtoItem& proto_item = request->item();
//	int index_pack = request->pack_index();
//	if(index_pack > 0)
//	{
//		int ret = this->pack_remove_by_index(ITEM_LSTORE_DELETE_FAILED,ItemIndexObj(index_pack,proto_item.amount()));
//		if(ret != 0)
//			MSG_USER("delete failed:%ld,item_id:%d,item_amount:%d,item_bind:%d:ret:%d",this->role_id(),proto_item.id(),proto_item.amount(),proto_item.bind(),ret);
//		return 0;
//	}
//
//	PackageItem* item = GamePackage::pop_item(proto_item.id());
//	item->unserialize(proto_item);
//	GamePackage* pack = this->find_package(GameEnum::INDEX_PACKAGE);
//	int ret = 0;
//	if(pack != 0)
//	{
//		ret = this->pack_insert(pack, ITEM_LSTORE_INSTER_FAILED,item);
//	}
//	if(pack == 0 || ret != 0)
//	{
//	    FontPair mail_font = FONT2(FONT_NO_PACKAGE);
//	    MailInformation *mail_info = GameCommon::create_sys_mail(mail_font,FONT_NO_PACKAGE);
//
//		PackageItem* pack_item = GamePackage::pop_item(proto_item.id());
//		*pack_item = *item;
//		pack_item->__amount = item->__amount;
//		int item_index = mail_info->goods_map_.size();
//		mail_info->goods_map_[item_index] = pack_item;
//	    GameCommon::request_save_mail(this->role_id(), mail_info);
//	    this->respond_to_client_error(ITEM_LSTORE_INSTER_FAILED, ERROR_MEDIA_GIFT_PACKAGE);
//	}
	return 0;
}

int MLLeaguer::lstore_get(Message *msg)
{
//	DYNAMIC_CAST(Proto31400317 *, request, msg);
//	const ProtoItem& proto_item = request->item();
//
//	Proto31400318 respond;
//	ProtoItem* respond_item = respond.mutable_item();
//
//	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
//	CONDITION_NOTIFY_RETURN(package != NULL,RETURN_LSTORE_GET_ITEM,ERROR_SERVER_INNER);
//
//	PackageItem* item = GamePackage::pop_item(proto_item.id());
//	item->unserialize(proto_item);
//	item->serialize(respond_item);
//	item->__index = package->search_empty_index_i();
//
//	if(item->__index < 0)
//	{
//		respond.set_result(1);
//		respond.set_error_code(item->__index);
//		GamePackage::push_item(item);
//		return this->monitor()->dispatch_to_logic(this,&respond);
//	}
//
//	int ret = package->insert_by_index(item);
//	if(ret != 0)
//	{
//		respond.set_result(1);
//		respond.set_error_code(ret);
//		GamePackage::push_item(item);
//		return this->monitor()->dispatch_to_logic(this,&respond);
//	}
//	this->notify_pack_info(GameEnum::INDEX_PACKAGE);
//
//    ItemObj item_obj(item->__id, item->__amount,GameEnum::ITEM_NO_BIND);
//    SERIAL_RECORD->record_item(this, this->agent_code(), this->market_code(),
//    		ITEM_GET_LSTORE, item_obj, this->role_id());
//
//    respond.set_pack_index(item->__index);
//	respond.set_result(0);
//	return this->monitor()->dispatch_to_logic(this,&respond);
	return 0;
}

int MLLeaguer::check_league_open_task(int task_id)
{
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_task("fun_league", task_id) == true, 0);
	return this->monitor()->dispatch_to_logic(this,INNER_CHECK_LEAGUE_OPEN_TASK);
}


