/*
 * MLTinyer.cpp
 *
 *  Created on: Nov 22, 2013
 *      Author: peizhibi
 */

#include "MLTinyer.h"
#include "MMOTrade.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "GameFont.h"
#include "MLGameSwither.h"

MLTinyer::MLTinyer()
{
	// TODO Auto-generated constructor stub

}

MLTinyer::~MLTinyer()
{
	// TODO Auto-generated destructor stub
}

void MLTinyer::reset_found()
{
	this->tiny_detail_.reset();
	this->daily_total_gold_ = 0;
	this->last_recharge_time_ = 0;
}

void MLTinyer::reset_everyday()
{
	this->last_recharge_time_ = 0;
	this->daily_total_gold_ = 0;
}

void MLTinyer::notify_client_tick()
{
    Proto81401706 server_ticks;
    server_ticks.set_ticks(::time(NULL));
    this->respond_to_client(ACTIVE_SERVER_TICKS_, &server_ticks);
}

TinyDetail* MLTinyer::tiny_detail()
{
	return &this->tiny_detail_;
}

int MLTinyer::check_and_level_up_player(const Json::Value &effect)
{
	JUDGE_RETURN(this->is_arrive_max_level(1) == false, ERROR_ARRIVE_MAX_LEVEL);

	if (this->role_level() < effect["max_level"].asInt())
	{
		this->send_to_map_thread(INNER_MAP_PROP_LEVELUP);
	}
	else
	{
		this->request_add_exp(effect["exp"].asInt(), EXP_FROM_PROP);
	}

	return 0;
}

int MLTinyer::process_item_transfer(const Json::Value &effect)
{
//	Proto30400501 transfer_info;
//	transfer_info.set_oper_result(0);
//	if(effect.isArray() == false)
//	{
//		   transfer_info.set_scene_id(effect["scene_id"].asInt());
//
//			int max_point = effect["pixel"].size() / 2;
//			int rand_index = std::rand() % max_point;
//
//			transfer_info.set_pixel_x(effect["pixel"][rand_index * 2 + 0u].asInt());
//			transfer_info.set_pixel_y(effect["pixel"][rand_index * 2 + 1u].asInt());
//	}
//	else
//	{
//		for(uint i = 0; i<effect.size() ; i++)
//		{
//			if(effect[i].isMember("scene_id")&& effect[i].isMember("pixel"))
//			{
//				    transfer_info.set_scene_id(effect[i]["scene_id"].asInt());
//
//					int max_point = effect[i]["pixel"].size() / 2;
//					int rand_index = std::rand() % max_point;
//
//					transfer_info.set_pixel_x(effect[i]["pixel"][rand_index * 2 + 0u].asInt());
//					transfer_info.set_pixel_y(effect[i]["pixel"][rand_index * 2 + 1u].asInt());
//				    break;
//			}
//		}
//	}
//	return this->send_to_map_thread(transfer_info);
	return 0;
}

int MLTinyer::process_item_transfer_random(const Json::Value &effect)
{
	Proto30400501 transfer_info;
	transfer_info.set_oper_result(0);
//    transfer_info.set_oper_type(1);
    return this->send_to_map_thread(transfer_info);
}

int MLTinyer::check_and_transfer_player(const Json::Value& effect,
		PackageItem* pack_item)
{
//	JUDGE_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
//			ERROR_SPECIAL_SCENE_TRANSFER);
//    JUDGE_RETURN(this->map_logic_player()->is_death() == false, ERROR_PLAYER_DEATH);
//
//	//JUDGE_RETURN(this->map_logic_player()->is_fight_state() == false,
//	//		ERROR_BEING_FIGHT);
//
//    this->process_item_transfer(effect);
//
//	int use_times = effect["use_times"].asInt();
//	JUDGE_RETURN(use_times != -1, GameEnum::USE_GOODS_MUCH_TIMES);
//
//	pack_item->__use_times += 1;
//	if (pack_item->__use_times < use_times)
//	{
//		return GameEnum::USE_GOODS_MUCH_TIMES;
//	}
//	else
//	{
//		pack_item->__use_times = 0;
//		return 0;
//	}
	return 0;
}

int MLTinyer::check_and_transfer_random_player(const Json::Value &effect, PackageItem *pack_item)
{
    JUDGE_RETURN(this->map_logic_player()->is_death() == false, ERROR_PLAYER_DEATH);
    JUDGE_RETURN(GameCommon::is_script_scene(this->scene_id()) == false, ERROR_SCENE_TRANSFER_LIMITED);
	//JUDGE_RETURN(this->map_logic_player()->is_fight_state() == false,
	//		ERROR_BEING_FIGHT);

    this->process_item_transfer_random(effect);

    return 0;
}

int MLTinyer::request_notice_add_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400313*, request, RETURN_MARKET_ONSELL);

	ItemObjVec item_obj_set;
	for (int i = 0; i < request->item_set_size(); ++i)
	{
		ItemObj item_obj(request->item_set(i));
		item_obj_set.push_back(item_obj);
	}

	int ret = this->insert_package(ADD_FROM_GAME_NOTICE, item_obj_set);
	request->set_oper_result(ret);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MLTinyer::request_add_onsell_goods(Message* msg)
{
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400221*, request, RETURN_MARKET_ONSELL);

	PackageItem* pack_item = package->find_by_index(request->index());
	CONDITION_NOTIFY_RETURN(GameCommon::item_can_onsell(pack_item) == true,
			RETURN_MARKET_ONSELL, ERROR_ITEM_ONSELL);

	pack_item->serialize(request->mutable_item_obj(),
			pack_item->__index, request->amount());

	MSG_USER("OnSell before remove %d, %d, %d, %d", pack_item->__index,
			pack_item->__id, pack_item->__amount, request->amount());

	int ret = this->pack_remove(package, ITEM_REMOVE_ONSELL,
			pack_item, request->amount());
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_MARKET_ONSELL, ret);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MLTinyer::market_buy_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400222*, request, RETURN_MARKET_BUY);

	int total_price = request->price() * request->amount();
	Money total_money(total_price);

	if (GameCommon::is_travel_scene(this->scene_id()) == true)
	{
		request->set_operate_result(ERROR_USE_SCENE_LIMIT);
		this->respond_to_client_error(RETURN_MARKET_BUY, ERROR_USE_SCENE_LIMIT);
	}
	else if (this->validate_money(total_money) == false)
	{
		request->set_operate_result(ERROR_PACKAGE_GOLD_AMOUNT);
		this->respond_to_client_error(RETURN_MARKET_BUY, ERROR_PACKAGE_GOLD_AMOUNT);
	}
	else
	{
		PackageItem pack_item;
		pack_item.unserialize(request->item_obj());

		request->set_operate_result(0);
		this->pack_money_sub(total_money, SUB_MONEY_MARKET_BUY);

		int buyer_mail_id = CONFIG_INSTANCE->market_const("buyer_mail").asInt();
		int seller_mail_id = CONFIG_INSTANCE->market_const("seller_mail").asInt();

		MailInformation* buyer_mail = GameCommon::create_sys_mail(buyer_mail_id);
		MailInformation* seller_mail = GameCommon::create_sys_mail(seller_mail_id);

		buyer_mail->sender_id_ = request->role_id();
		buyer_mail->add_goods(pack_item.__id, request->amount());

		seller_mail->sender_id_ = this->role_id();
		seller_mail->add_money(request->money_type(), total_price);

		GameCommon::request_save_mail(this->role_id(), buyer_mail);
		GameCommon::request_save_mail(request->role_id(), seller_mail);
	}

	return MAP_MONITOR->dispatch_to_scene_by_noplayer(this, SCENE_LOGIC, request);
}

int MLTinyer::request_buy_arena_times(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400308*, request, RETURN_ARENA_BUY_TIMES);

	Money money;
	money.__gold = request->need_money();

	int ret = this->pack_money_sub(money, SUB_MONEY_AREA_TIMES);
	request->set_oper_result(ret);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

int MLTinyer::request_buy_clear_cool(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400309*, request, RETURN_ARENA_CLEAR_COOL);

	Money money;
	money.__gold = request->need_money();

	int ret = this->pack_money_sub(money, SUB_MONEY_AREA_COOL);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ARENA_CLEAR_COOL, ret);

	return MAP_MONITOR->dispatch_to_logic(this, request);
}

// transfer
int MLTinyer::sync_transfer_tiny(int scene_id)
{
	Proto31400134 tiny_info;
	tiny_info.set_total_recharge(this->daily_total_gold_);
	tiny_info.set_last_tick(this->last_recharge_time_);

	GameCommon::map_to_proto(tiny_info.mutable_guide(),
			this->tiny_detail_.guide_map_);
	return this->send_to_other_logic_thread(scene_id, tiny_info);
}

int MLTinyer::read_transfer_tiny(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400134*, request, -1);
	this->daily_total_gold_ = request->total_recharge();
	this->last_recharge_time_ = request->last_tick();
	GameCommon::proto_to_map(this->tiny_detail_.guide_map_, request->guide());
	return 0;
}

int MLTinyer::fetch_guide_info()
{
	Proto51400012 respond;
	for (IntMap::iterator iter = this->tiny_detail_.guide_map_.begin();
			iter != this->tiny_detail_.guide_map_.end(); ++iter)
	{
		ProtoPairObj* obj = respond.add_guide_list();
		obj->set_obj_id(iter->first);
		obj->set_obj_value(iter->second);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_GUIDE_INFO, &respond);
}

int MLTinyer::save_guide_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400013*, request, RETURN_SAVE_GUIDE_INFO);
	JUDGE_RETURN(this->tiny_detail_.guide_map_.size() < 100, -1);

	int key = request->id();
	int value = request->times();

	this->tiny_detail_.guide_map_[key] = value;
	FINER_PROCESS_NOTIFY(RETURN_SAVE_GUIDE_INFO);
}

int MLTinyer::get_total_recharge()
{
	return this->daily_total_gold_;
}

Int64 MLTinyer::get_last_recharge_time()
{
	return this->last_recharge_time_;
}

void MLTinyer::set_total_recharge(int gold)
{
	this->daily_total_gold_ = gold;
}

void MLTinyer::set_last_recharge_time(Int64 time)
{
	this->last_recharge_time_ = time;
}

int MLTinyer::lucky_table_cost(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31400039*, request,msg,-1);
	int cost_gold = request->cost();
	if(this->pack_detail().__money.__gold >= cost_gold)
	{
		Money money(cost_gold);
		int ret = this->pack_money_sub(money,SUB_MONEY_LUCKY_TABLE_COST);
		if(ret != 0)
		{
			this->respond_to_client_error(RETURN_OPERATOR_LUCKY_TABLE,ret);
			request->set_result(0);
		}
		else
			request->set_result(1);
	}
	else
	{
		this->set_last_error(ERROR_PACKAGE_MONEY_AMOUNT);
		this->respond_to_client_error(RETURN_OPERATOR_LUCKY_TABLE,ERROR_PACKAGE_MONEY_AMOUNT);
		request->set_result(0);
	}

	return MAP_MONITOR->dispatch_to_logic(this->map_logic_player(),msg);
}

