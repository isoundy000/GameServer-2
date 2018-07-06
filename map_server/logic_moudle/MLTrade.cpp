/*
 * MLTrade.cpp
 *
 *  Created on: 2015-11-21
 *      Author: xu
 */

#include "MLTrade.h"
#include "MapLogicPlayer.h"
#include "PubStruct.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MLGameSwither.h"

MLTrade::RespondOneRequestTimer::RespondOneRequestTimer(void):parent_(0)
{

}
MLTrade::RespondOneRequestTimer::~RespondOneRequestTimer(void)
{

}

void MLTrade::RespondOneRequestTimer::set_parent(MLTrade *parent)
{
	this->parent_ = parent;
}

int MLTrade::RespondOneRequestTimer::type(void)
{
	return GTT_ML_ONE_SECOND;
}

int MLTrade::RespondOneRequestTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->parent_ != 0, 0);
	this->parent_->trade_wait_once_end();//1.通知对方移除小图标 2.从对方list中删掉自己的id
	return 0;
}

MLTrade::MLTrade()
{
	this->wait_once_timer_.set_parent(this);
	this->trade_info_.reset();
	this->is_close_panel_ = false;
}

MLTrade::~MLTrade()
{

}

MapLogicPlayer* MLTrade::trade_player(void)
{
	return dynamic_cast<MapLogicPlayer *>(this);
}

void MLTrade::reset_trade()
{
	this->trade_info_.reset();
	this->is_close_panel_ = false;
	if (this->wait_once_timer_.is_registered() == true)
	{
		this->wait_once_timer_.cancel_timer();
	}
}

void MLTrade::trade_wait_once_end()
{
	 if(TRADE_REQUESTING == this->trade_info_.trade_state && this->wait_once_timer_.is_registered())
	 {
		 this->wait_once_timer_.cancel_timer();
		 this->trade_info_.trade_state = TRADE_IDLE;
	 }
	//如果对方小图标对应的id等于当前id，通知remove它
	this->trade_remove_icon(this->trade_info_.requ_role_id);
	//从对方list中删掉自己的id
	this->trade_remove_role_id_from_responser(this->trade_info_.requ_role_id);
}

void MLTrade::trade_state_in_idle(MapLogicPlayer* sponser_player)
{
   if((TRADE_REQUESTING == this->trade_info_.trade_state) && (this->wait_once_timer_.is_registered() == true))
   {
	   this->wait_once_timer_.cancel_timer();
   }
   this->reset_trade();
   if(sponser_player != NULL)
   {
	  sponser_player->reset_trade();
   }
}

void MLTrade::trade_state_in_requesting(Int64 role_id)
{
	this->trade_info_.requ_role_id = role_id;
	this->trade_info_.trade_state = TRADE_REQUESTING;
	this->wait_once_timer_.schedule_timer(Time_Value(30));
}

void MLTrade::trade_state_in_waiting(Int64 role_id)
{
	this->trade_info_.requ_role_id = role_id;
	this->trade_info_.accept_role_id_list.push_back(role_id);
	this->trade_info_.trade_state = TRADE_WAITING;
}

void MLTrade::trade_state_in_progress(Int64 role_id)
{
	if((TRADE_REQUESTING == this->trade_info_.trade_state)&& (this->wait_once_timer_.is_registered() == true) )
	{
		 this->wait_once_timer_.cancel_timer();
	}
	this->trade_info_.curr_role_id = role_id;
	this->trade_info_.trade_state = TRADE_PROCESS;
}

int MLTrade::trade_condition_judgement(const int recogn,MapLogicPlayer *player)
{
	JUDGE_RETURN(player != NULL, 0);
	if(recogn == RETURN_TRADE_SPONSOR)
	{
		//判断自己是否已经在交易中
		CONDITION_NOTIFY_RETURN( TRADE_PROCESS != this->trade_info_.trade_state,recogn,
						ERROR_TRADE_OWN_TRADE_PROCESS);
		//自己是否在战斗状态
		CONDITION_NOTIFY_RETURN(!this->trade_player()->is_fight_state(),recogn,
						ERROR_TRADE_OWN_FIGHTING);
		//对方玩家是否已经在交易
		CONDITION_NOTIFY_RETURN( TRADE_PROCESS != player->trade_info_.trade_state,recogn,
						ERROR_TRADE_OPPONENT_TRADE_PROCESS);
		//对方是否在战斗状态
		CONDITION_NOTIFY_RETURN(!player->is_fight_state() ,recogn,
						ERROR_TRADE_OPPONENT_FIGHTING);
		//交易双方需同屏一定范围（50格）
		this->trade_send_to_map_condition(recogn,player->role_id());
	}
	else
	if(recogn == RETURN_TRADE_INVITE_RESPOND)
	{
		if(this->trade_player()->is_fight_state())
		{
			this->inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
			player->respond_to_client_error(recogn, ERROR_TRADE_REQUEST_IGNORE);
			this->set_last_error(ERROR_TRADE_OWN_FIGHTING);
			return this->respond_to_client_error(recogn, ERROR_TRADE_OWN_FIGHTING);
		}
		else
		if(player->is_fight_state())
		{
			this->inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
			player->respond_to_client_error(recogn, ERROR_TRADE_OWN_FIGHTING);
			this->set_last_error(ERROR_TRADE_OPPONENT_FIGHTING);
			return this->respond_to_client_error(recogn, ERROR_TRADE_OPPONENT_FIGHTING);
		}
		else
		{
			this->trade_send_to_map_condition(recogn,player->role_id());
		}
	}
	return 0;
}

int MLTrade::request_org_trade(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11402401*,request,RETURN_TRADE_SPONSOR);
	//对方玩家是否在线
	MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(request->role_id());
	CONDITION_NOTIFY_RETURN(des_player != NULL, RETURN_TRADE_SPONSOR,
					ERROR_PLAYER_OFFLINE);

	CONDITION_NOTIFY_RETURN(this->trade_check_limit_condition() == true, RETURN_TRADE_SPONSOR,
					ERROR_TRADE_OWN_LEVEL_LIMIT);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false, RETURN_TRADE_SPONSOR, ERROR_OP_IN_ACTIVITY_TICK);

	CONDITION_NOTIFY_RETURN(des_player->trade_check_limit_condition() == true, RETURN_TRADE_SPONSOR,
					ERROR_TRADE_DES_LEVEL_LIMIT);

	CONDITION_NOTIFY_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::trade),
					RETURN_TRADE_SPONSOR, ERROR_TRADE_OPPONENT_NOTOPEN);
	//自己处于等待响应中
	CONDITION_NOTIFY_RETURN( TRADE_WAITING != this->trade_info_.trade_state,RETURN_TRADE_SPONSOR,
					ERROR_TRADE_OWN_STATE_WAITING);
	//自己状态是否为请求中
	CONDITION_NOTIFY_RETURN( TRADE_REQUESTING != this->trade_info_.trade_state,RETURN_TRADE_SPONSOR,
					ERROR_TRADE_OWN_STATE_REQUESTING);
	//对方正在请求中
	CONDITION_NOTIFY_RETURN( TRADE_REQUESTING != des_player->trade_info_.trade_state,RETURN_TRADE_SPONSOR,
							ERROR_TRADE_DES_STATE_REQUESTING);
	this->trade_condition_judgement(RETURN_TRADE_SPONSOR,des_player);
	return 0;
}

int MLTrade::trade_condition_satisfy(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31400242 *, request, msg, -1);
	Int64 role_id = request->role_id();
	int recogn = request->cur_recogn();
	if(recogn == RETURN_TRADE_SPONSOR)  //第一次发起交易条件成功,转给交易响应玩家
	{
		MapLogicPlayer* responser_player = MAP_MONITOR->find_logic_player(role_id);
		CONDITION_NOTIFY_RETURN(responser_player != NULL, recogn,ERROR_PLAYER_OFFLINE);

		this->trade_state_in_requesting(role_id);
		responser_player->trade_state_in_waiting(this->trade_player()->role_id());
		//交易请求已经发送，请耐心等待
		//this->respond_to_client_error(recogn, ERROR_TRADE_REQUEST_SUCESS);
		TipsPlayer tips(this);
		char tips_str[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1];
		const Json::Value &trade_req_json = CONFIG_INSTANCE->tiny("trade_request");
		if(trade_req_json == Json::Value::null)
		{
			MSG_USER("tiny.json-> no trade_request fields");
			return this->respond_to_client_error(recogn, ERROR_CONFIG_NOT_EXIST);
		}
		::strncpy(tips_str,trade_req_json.asCString(), GameEnum::DEFAULT_MAX_CONTENT_LEGNTH);
		tips_str[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';
		tips.push_tips_str(tips_str);
		//通知对方玩家，显示小图标
		//trade_notify_show_icon(responser_player,this->trade_player()->role_id(),this->trade_player()->role_name());
		Proto80400241 respond;
		respond.set_role_id(this->trade_player()->role_id());
		respond.set_role_name(this->trade_player()->role_name());
		responser_player->respond_to_client(ACTIVE_TRADE_TO_RESPONSOR, &respond);
		MSG_USER("trade 1 satisfy ok");
		return this->respond_to_client(recogn);
	}
	else
	if(recogn == RETURN_TRADE_INVITE_RESPOND)  //第二次判断交易条件满足,转给交易发起玩家
	{
		MapLogicPlayer* sponser_player = MAP_MONITOR->find_logic_player(role_id);
		if(sponser_player == NULL)
		{
			this->trade_state_in_idle(NULL);
			return this->respond_to_client_error(RETURN_TRADE_INVITE_RESPOND, ERROR_PLAYER_OFFLINE);
		}
		//设置交易状态
		this->trade_state_in_progress(role_id);
		sponser_player->trade_state_in_progress(this->trade_player()->role_id());
		//同步到MapPlayer
		trade_update_to_map_player(role_id,1);
		//通知发起交易方现在可以开始交易
		Proto80400242 send_to_sponser;
		send_to_sponser.set_respone_type(1);
		send_to_sponser.set_role_id(this->trade_player()->role_id());
		send_to_sponser.set_role_name(this->trade_player()->role_name());
		sponser_player->respond_to_client(ACTIVE_TRADE_TO_SPONSOR, &send_to_sponser);
		//条件满足打开交易UI
		Proto51402402 respon_condition;
		respon_condition.set_condition(1);
		this->respond_to_client(RETURN_TRADE_INVITE_RESPOND,&respon_condition);
		MSG_USER("trade 2 satisfy ok");
	}
	return 0;
}

int MLTrade::trade_update_to_map_player(Int64 role_id,int type)
{
	Proto30400243 update_trade_map;
	update_trade_map.set_role_id(role_id);
	update_trade_map.set_betrading(type);
	MAP_MONITOR->process_inner_map_request(this->trade_player()->role_id(),update_trade_map);
	update_trade_map.set_role_id(this->trade_player()->role_id());
	MAP_MONITOR->process_inner_map_request(role_id,update_trade_map);
	return 0;
}

int MLTrade::trade_send_to_map_condition(int recogn,Int64 role_id)
{
	Proto30400242  send_to_map;
	send_to_map.set_role_id(role_id);
	send_to_map.set_cur_recogn(recogn);
	MAP_MONITOR->process_inner_map_request(this->trade_player()->role_id(),send_to_map);
	return 0;
}

int MLTrade::trade_respond_invite(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11402402*,request,RETURN_TRADE_INVITE_RESPOND);
	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false, RETURN_TRADE_INVITE_RESPOND, ERROR_OP_IN_ACTIVITY_TICK);

	this->trade_notify_others_request_cancel(request->role_id());

	MapLogicPlayer* sponser_player = MAP_MONITOR->find_logic_player(request->role_id());
	if(sponser_player == NULL)
	{
		Proto30101607 inner_req;
		inner_req.set_role_id(request->role_id());
		inner_req.set_recogn(RETURN_TRADE_INVITE_RESPOND);
		inner_req.set_on_line(false);
		return MAP_MONITOR->dispatch_to_logic(this->trade_player(),&inner_req);
	}
	if(request->respone_type() == 1)
	{
		//再次判断交易条件
		this->trade_condition_judgement(RETURN_TRADE_INVITE_RESPOND,sponser_player);
	}
	else
	if(request->respone_type() == 0)
	{   //通知发起方被拒绝
		Proto80400242 send_to_sponser;
		send_to_sponser.set_respone_type(0);
		send_to_sponser.set_role_id(this->trade_player()->role_id());
		send_to_sponser.set_role_name(this->trade_player()->role_name());
		sponser_player->respond_to_client(ACTIVE_TRADE_TO_SPONSOR, &send_to_sponser);
		sponser_player->respond_to_client_error(RETURN_TRADE_INVITE_RESPOND, ERROR_TRADE_DES_REFUSE);
		this->inner_notify_trade_cancel();
	/*	this->trade_notify_others_request_cancel(sponser_player->role_id());
		//此时拒绝方为空闲状态，历史请求被清空
		this->trade_state_in_idle();
		//发起方被拒绝后，如果list为空，更新为空闲状态否则转为waiting状态
		sponser_player->trade_state_in_idle();
		if(!sponser_player->trade_info_.accept_role_id_list.empty())
		{
			//转为wating状态
			sponser_player->trade_info_.trade_state = TRADE_WAITING;
			Int64 new_sponser_role_id = sponser_player->trade_info_.accept_role_id_list.back();
			MapLogicPlayer* new_sponser_player = MAP_MONITOR->find_logic_player(new_sponser_role_id);
			if(new_sponser_player)
			  trade_notify_show_icon(sponser_player,new_sponser_role_id,new_sponser_player->role_name());
		}*/
	}
	return 0;
}

/*int MLTrade::trade_notify_show_icon(MapLogicPlayer *player,Int64 sponser_role_id,const string sponser_role_name)
{
	Proto80400241 respond;
	respond.set_role_id(sponser_role_id);
	respond.set_role_name(sponser_role_name);
	player->respond_to_client(ACTIVE_TRADE_TO_RESPONSOR, &respond);
	return 0;
}*/

int  MLTrade::trade_remove_role_id_from_responser(Int64 responer_role_id)
{
	MapLogicPlayer* responser_player = MAP_MONITOR->find_logic_player(responer_role_id);
	JUDGE_RETURN(responser_player != NULL,0);
	LongList::iterator iter = responser_player->trade_info_.accept_role_id_list.begin();
	for(;iter != responser_player->trade_info_.accept_role_id_list.end();)
	{
		if(*iter == this->trade_player()->role_id() )
		{
			iter = responser_player->trade_info_.accept_role_id_list.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	return 0;
}

int  MLTrade::trade_remove_icon(Int64 responer_role_id)
{
	MapLogicPlayer* responser_player = MAP_MONITOR->find_logic_player(responer_role_id);
	JUDGE_RETURN(responser_player != NULL,0);
	if(responser_player->trade_info_.accept_role_id_list.back() == this->trade_player()->role_id())
	{
		this->inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
		responser_player->trade_info_.trade_state = TRADE_IDLE;
		responser_player->respond_to_client(ACTIVE_TRADE_REMOVE_ICON);
	}
	return 0;
}

void MLTrade::trade_notify_others_request_cancel(Int64 except_role_id)
{
	MapLogicPlayer* sponser_player = NULL;
	LongList::iterator iter = this->trade_info_.accept_role_id_list.begin();
	for(;iter != this->trade_info_.accept_role_id_list.end();iter++)
	{
		JUDGE_CONTINUE(*iter != except_role_id);
		sponser_player = MAP_MONITOR->find_logic_player(*iter);
		if(sponser_player != NULL)
		{
			sponser_player->trade_state_in_idle(NULL);
			Proto80400243 respond;
			respond.set_role_id(this->trade_player()->role_id());
			sponser_player->respond_to_client(ACTIVE_TRADE_IGNOR_REQUEST,&respond);
			MSG_USER("trade rold_id %ld request be canceled",*iter);
		}
	}
	this->trade_info_.accept_role_id_list.clear();
}

////交易中
int MLTrade::trade_push_items(Message *msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto11402403 *, request, RETURN_TRADE_PUSH_ITEMS);
//	CONDITION_NOTIFY_RETURN(this->trade_info_.ItemTempVector.size() < (uint)(CONFIG_INSTANCE->tiny("max_trade_grid").asInt()),
//				RETURN_TRADE_PUSH_ITEMS,ERROR_TRADE_OWN_ITEMS_OVERLIMIT);
//
//	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false, RETURN_TRADE_PUSH_ITEMS, ERROR_OP_IN_ACTIVITY_TICK);
//
//	MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(this->trade_info_.curr_role_id);
//	if(des_player == NULL)
//	{
//		inner_notify_trade_cancel();
//		return 0;
//	}
//	if(this->trade_info_.is_locked == 1)
//	{
//		this->set_last_error(ERROR_TRADE_OWN_GRID_LOCKED);
//		return this->respond_to_client_error(RETURN_TRADE_PUSH_ITEMS, ERROR_TRADE_OWN_GRID_LOCKED);
//	}
//	int item_id = request->item_id();
//	int item_amount = request->item_amount();
//	int item_index = request->pack_index();
//	JUDGE_RETURN(item_amount > 0, 0);
//	JUDGE_RETURN(item_index >= 0, 0);
//	int overlap =  GameCommon::item_overlap_count(item_id);
//	if(item_amount > overlap)
//	{
//		this->set_last_error(ERROR_TRADE_GIRD_AMOUNT_OVER_LIMIT);
//		return this->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_GIRD_AMOUNT_OVER_LIMIT);
//	}
//	bool befind = false;
//	PackageItem* pack_item = pack_find(item_index,GameEnum::INDEX_PACKAGE);
//	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
//	if((pack_item != NULL) && (package != NULL))
//	{
//		CONDITION_NOTIFY_RETURN(pack_item->__id == item_id, RETURN_TRADE_PUSH_ITEMS,ERROR_TRADE_GOODS_NOT_FIND);
//		CONDITION_NOTIFY_RETURN(pack_item->__bind == GameEnum::ITEM_NO_BIND, RETURN_TRADE_PUSH_ITEMS,ERROR_TRADE_BIND_CANNOT_TRADE);
//		CONDITION_NOTIFY_RETURN(item_amount <= pack_item->__amount, RETURN_TRADE_PUSH_ITEMS,ERROR_PACKAGE_GOODS_AMOUNT);
//		//package->pack_lock(pack_item->__index,0); //锁定放入到数组中的物品
//		if(GameCommon::item_is_equipment(item_id))
//		{   //装备中有镶嵌宝石的 无法交易
//			CONDITION_NOTIFY_RETURN(pack_item->__equipment.__jewel_detail.empty() && pack_item->__equipment.__special_jewel.empty() , RETURN_TRADE_PUSH_ITEMS,ERROR_TRADE_EQU_JEWEL_NO_TRADE);
//			this->trade_info_.ItemTempVector.push_back(*pack_item);
//		}
//		else
//		{
//			PackageItem* temp_item = NULL;
//			NPItemVector::iterator iter = this->trade_info_.ItemTempVector.begin();
//			for(;iter != this->trade_info_.ItemTempVector.end();iter++)
//			{
//				JUDGE_CONTINUE(&(*iter) != NULL);
//				temp_item = &(*iter);
//				if((temp_item->__id == pack_item->__id) && (temp_item->__index == pack_item->__index))
//				{
//					CONDITION_NOTIFY_RETURN((temp_item->__amount + item_amount) <= pack_item->__amount, RETURN_TRADE_PUSH_ITEMS,ERROR_PACKAGE_GOODS_AMOUNT);
//					if( (temp_item->__amount + item_amount) < overlap)
//					{
//						temp_item->__amount += item_amount;
//						befind = true;
//					}
//					else
//					{
//						this->set_last_error(ERROR_TRADE_GIRD_AMOUNT_OVER_LIMIT);
//					    this->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_GIRD_AMOUNT_OVER_LIMIT);
//					    break;
//					}
//				}
//			}
//			if(!befind)
//			{
//				int befor_amount = pack_item->__amount;
//				pack_item->__amount = item_amount;
//				this->trade_info_.ItemTempVector.push_back(*pack_item);
//				pack_item->__amount = befor_amount;
//			}
//		}
//	}
//	else
//	{
//		this->set_last_error(ERROR_GOODS_NO_EXIST);
//	    this->respond_to_client_error(RETURN_TRADE_START, ERROR_GOODS_NO_EXIST);
//	}
//	this->trade_make_up_trade_box(des_player);
	return 0;
}

int MLTrade::trade_pop_items(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11402404 *, request, RETURN_TRADE_POP_ITEMS);
	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false, RETURN_TRADE_POP_ITEMS, ERROR_OP_IN_ACTIVITY_TICK);

	MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(this->trade_info_.curr_role_id);
	if(des_player == NULL)
	{
		inner_notify_trade_cancel();
		return 0;
	}
	if(this->trade_info_.is_locked == 1)
	{
		this->set_last_error(ERROR_TRADE_GRID_LOCKED_NOPOP);
		return this->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_GRID_LOCKED_NOPOP);
	}
	int item_id = request->item_id();
	int item_index = request->pack_index();
	int item_amount = request->item_amount();
	PackageItem* temp_item = NULL;
	NPItemVector::iterator iter = this->trade_info_.ItemTempVector.begin();
	for(;iter != this->trade_info_.ItemTempVector.end();)
	{
		JUDGE_CONTINUE(&(*iter) != NULL);
		temp_item = &(*iter);
		if((temp_item->__id == item_id) && (temp_item->__index == item_index))
		{
			if(item_amount <= temp_item->__amount)
			{
				if(item_amount == temp_item->__amount)
				{
					iter = this->trade_info_.ItemTempVector.erase(iter);
				}
				else
				{
					temp_item->__amount -= item_amount;
				}
			}
			else
			{
				this->set_last_error(ERROR_SET_ITEM_AMOUNT);
			    this->respond_to_client_error(RETURN_TRADE_START, ERROR_SET_ITEM_AMOUNT);
			}
			 break;
		}
		else
		{
			iter++;
		}
	}
	this->trade_make_up_trade_box(des_player);
	return 0;
}

int MLTrade::trade_make_up_trade_box(MapLogicPlayer* des_player)
{
	Proto80400247 send_respon;
	Proto51402404 return_respon;
	NPItemVector::iterator iter = this->trade_info_.ItemTempVector.begin();
	PackageItem* temp_item = NULL;
	for(;iter != this->trade_info_.ItemTempVector.end();iter++)
	{
		JUDGE_CONTINUE(&(*iter) != NULL);
		temp_item = &(*iter);
		ProtoItem* proto_item1 = send_respon.add_item_list();
		temp_item->serialize(proto_item1);

		ProtoItem* proto_item2 = return_respon.add_item_list();
		temp_item->serialize(proto_item2);
	}
	des_player->respond_to_client(ACTIVE_TRADE_NOTIFY_POP_ITEMS, &send_respon);
	this->respond_to_client(RETURN_TRADE_POP_ITEMS,&return_respon);
	return 0;
}

int MLTrade::trade_lock_panel()
{
	this->trade_info_.is_locked = 1;
	MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(this->trade_info_.curr_role_id);
	if(des_player == NULL)
	{
		inner_notify_trade_cancel();
		return 0;
	}
	des_player->respond_to_client_error(ACTIVE_TRADE_NOTIFY_TRADE_LOCKED, ERROR_TRADE_DES_LOCKED_TRADE);
	des_player->respond_to_client(ACTIVE_TRADE_NOTIFY_TRADE_LOCKED);
	if(des_player->trade_info_.is_locked == 1)
	{
		this->respond_to_client(ACTIVE_TRADE_NOTIFY_BUTTON_OK);
		des_player->respond_to_client(ACTIVE_TRADE_NOTIFY_BUTTON_OK);
	}
	this->respond_to_client(RETURN_TRADE_LOCK_PANEL);
	return 0;
}

int MLTrade::trade_start()
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false, RETURN_TRADE_START, ERROR_OP_IN_ACTIVITY_TICK);

	//检测对方的背包是否够格子
	MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(this->trade_info_.curr_role_id);
	if(des_player == NULL)
	{
		inner_notify_trade_cancel();
		return 0;
	}
	if(!((this->trade_info_.is_locked == 1)  && (des_player->trade_info_.is_locked == 1)))
	{
		inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
		this->set_last_error(ERROR_NOT_BOTH_IN_LOCKED);
		return this->respond_to_client_error(RETURN_TRADE_START, ERROR_NOT_BOTH_IN_LOCKED);
	}
	GamePackage* des_package = des_player->find_package(GameEnum::INDEX_PACKAGE);
	if(des_package == NULL)
	{
		inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
		this->set_last_error(ERROR_PACKAGE_NOT_EXISTS);
		return this->respond_to_client_error(RETURN_TRADE_START, ERROR_PACKAGE_NOT_EXISTS);
	}
	if(this->trade_info_.ItemTempVector.size() > (uint)CONFIG_INSTANCE->tiny("max_trade_grid").asInt())
	{
		inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
		this->set_last_error(ERROR_NO_ENOUGH_SPACE);
		return this->respond_to_client_error(RETURN_TRADE_START, ERROR_NO_ENOUGH_SPACE);
	}
	if(des_player->trade_judge_package(this->find_package(GameEnum::INDEX_PACKAGE)) == -1)
	{
		inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
		des_player->set_last_error(ERROR_TRADE_DES_BOXS_LACK);
		des_player->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_DES_BOXS_LACK);
		this->set_last_error(ERROR_TRADE_OWN_BOXS_LACK);
		return this->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_OWN_BOXS_LACK);
	}
	if(this->trade_judge_package(des_package) == -1)
	{
		inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
		des_player->set_last_error(ERROR_TRADE_OWN_BOXS_LACK);
		des_player->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_OWN_BOXS_LACK);
		this->set_last_error(ERROR_TRADE_DES_BOXS_LACK);
		return this->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_DES_BOXS_LACK);
	}
	this->respond_to_client(RETURN_TRADE_START);
	this->trade_info_.is_trading = 1;
	if(des_player->trade_info_.is_trading == 1)
	{
		this->trade_info_.is_trading = 0;
		this->trade_insert_des_package(des_package,des_player);
		des_player->trade_insert_des_package(this->find_package(GameEnum::INDEX_PACKAGE),this);
		//交易成功
		this->respond_to_client(ACTIVE_TRADE_NOTIFY_TRADE_SUCCESS);
		des_player->respond_to_client(ACTIVE_TRADE_NOTIFY_TRADE_SUCCESS);
		this->trade_info_.trade_state = TRADE_SUCCESS;
		des_player->trade_info_.trade_state = TRADE_SUCCESS;
		inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);
		//this->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_SUCCESS);
		//des_player->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_SUCCESS);
		TipsPlayer tips1(this),tips2(des_player);
		char tips_str[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1];
		const Json::Value &trade_succ_json = CONFIG_INSTANCE->tiny("trade_success");
		if(trade_succ_json == Json::Value::null)
		{
			MSG_USER("tiny.json-> no trade_success fields");
			return this->respond_to_client_error(RETURN_TRADE_START, ERROR_CONFIG_NOT_EXIST);
		}
		::strncpy(tips_str,trade_succ_json.asCString(), GameEnum::DEFAULT_MAX_CONTENT_LEGNTH);
		tips_str[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';
		tips1.push_tips_str(tips_str);
		tips2.push_tips_str(tips_str);
		MSG_USER("trade success");
	}
	return 0;
}

int MLTrade::trade_judge_package(GamePackage* des_package)
{
	uint min_box = des_package->left_capacity();
	int need_boxs = 0;
	if(min_box >= this->trade_info_.ItemTempVector.size())
	{
		return 0;
	}
	else
	{
		//叠加后还需要的格子数
		bool befind = false;
		PackageItem* temp_item = NULL;
		NPItemVector::iterator iter = this->trade_info_.ItemTempVector.begin();
		for(;iter != this->trade_info_.ItemTempVector.end();iter++)
		{
			JUDGE_CONTINUE(&(*iter) != NULL);
			temp_item = &(*iter);
			if(GameCommon::item_is_equipment(temp_item->__id))
			{
				need_boxs++;
				continue;
			}
			else
			{
				ItemListMap::iterator it = des_package->item_list_map_.begin();
				PackageItem *tmp_item = NULL;
				for(; it != des_package->item_list_map_.end(); ++it)
				{
					JUDGE_CONTINUE(it->second != NULL);
					tmp_item = it->second;
					JUDGE_CONTINUE(tmp_item->__bind == GameEnum::ITEM_NO_BIND);
					JUDGE_CONTINUE(!GameCommon::item_is_equipment(tmp_item->__id));
					if((tmp_item->__id == temp_item->__id) && CONFIG_INSTANCE->item(tmp_item->__id).isMember("overlap"))
					{
						if((tmp_item->__amount + temp_item->__amount) <= GameCommon::item_overlap_count(temp_item->__id))
									befind = true;
					}
				}
				if(!befind)
				{
					need_boxs++;
				}
			}
		}
	}
	if(need_boxs <= des_package->left_capacity())
	{
		return 0;
	}
	else
	{
		return -1;
	}
	return 0;
}

int MLTrade::trade_insert_des_package(GamePackage* des_package,MLPacker* des_mlpacker)
{
//	//插入背包
//	GamePackage* my_package = this->find_package(GameEnum::INDEX_PACKAGE);
//	if(my_package == NULL)
//	{
//		inner_notify_trade_cancel();
//		return 0;
//	}
//	TipsPlayer tips(des_mlpacker);
//	MapLogicPlayer *des_player = dynamic_cast<MapLogicPlayer *>(des_mlpacker);
//	string des_player_name = "",this_player_name = this->trade_player()->role_name();
//	if(des_player)
//	{
//		des_player_name = des_player->role_name();
//	}
//	NPItemVector::iterator iter = this->trade_info_.ItemTempVector.begin();
//	for(;iter != this->trade_info_.ItemTempVector.end();iter++)
//	{
//		JUDGE_CONTINUE(&(*iter) != NULL);
//		PackageItem* temp_item  = &(*iter);
//		PackageItem* pack_item  = pack_find(temp_item->__index,GameEnum::INDEX_PACKAGE);
//	    JUDGE_CONTINUE(pack_item != NULL);
//	    JUDGE_CONTINUE(temp_item->__bind == GameEnum::ITEM_NO_BIND);
//	    if((temp_item->__id == pack_item->__id) && (temp_item->__index == pack_item->__index) && (temp_item->__bind == pack_item->__bind))
//	    {
//	    	if(temp_item->__amount <= pack_item->__amount)
//	    	{
//	    		//先删除
//	    		//my_package->pack_unlock(pack_item->__index,0);
//	    		ItemIndexObj item_obj;
//	    		item_obj.item_index_ = temp_item->__index;
//	    		item_obj.item_amount_ = temp_item->__amount;
//	    		JUDGE_CONTINUE(pack_remove_by_index(SerialObj(ITEM_TRADE_REMOVE),item_obj, GameEnum::INDEX_PACKAGE,this->trade_info_.curr_role_id) == 0);
//	    		MSG_USER("trade %s delete item %d amount %d ok",this_player_name.c_str(),temp_item->__id,temp_item->__amount);
//	    		//后插入
//	    		if(GameCommon::item_is_equipment(temp_item->__id))
//	    		{
//	    			if(!(temp_item->__amount  == 1))
//	    			{
//	    				MSG_USER("trade failure:%d amount is %d",temp_item->__id,temp_item->__amount);
//	    				continue;
//	    			}
//	    			if(!(pack_item->__equipment.__jewel_detail.empty() && pack_item->__equipment.__special_jewel.empty()))
//	    			{
//	    				MSG_USER("trade failure:%d have jewel or special_jewel",temp_item->__id);
//	    				continue;
//	    			}
//					int des_index = des_package->search_empty_index_i();
//					if(des_index < 0)
//					{
//						MSG_USER("trade failure:in equipment des_index < 0");
//						return -1;
//					}
//					temp_item->__index = des_index;
//					JUDGE_CONTINUE(des_package->insert(temp_item) == 0);
//					MSG_USER("trade %s insert equipment item %d amount %d ok",des_player_name.c_str(), temp_item->__id,temp_item->__amount);
//					ItemObj item(temp_item->__id,temp_item->__amount,temp_item->__bind,temp_item->__index);
//					des_mlpacker->pack_insert_serial(item,ITEM_TRADE_GET,this->role_id());
//					tips.push_tips(GameEnum::TIPS_ITEM, temp_item->__id, temp_item->__amount);
//	    		}
//	    		else
//				{
//					bool befind = false;
//					ItemListMap::iterator it = des_package->item_list_map_.begin();
//					for(; it != des_package->item_list_map_.end(); ++it)
//					{
//						JUDGE_CONTINUE(it->second != NULL);
//						PackageItem *tmp_item  = it->second;
//						JUDGE_CONTINUE(tmp_item->__bind == GameEnum::ITEM_NO_BIND);
//						JUDGE_CONTINUE(!GameCommon::item_is_equipment(tmp_item->__id));
//						if((tmp_item->__id == temp_item->__id) && (tmp_item->__bind == temp_item->__bind) && CONFIG_INSTANCE->item(tmp_item->__id).isMember("overlap"))
//						{
//							int overlap =GameCommon::item_overlap_count(temp_item->__id);
//							CONDITION_NOTIFY_RETURN(overlap > 0, RETURN_TRADE_START,ERROR_CONFIG_ERROR);
//							if((tmp_item->__amount + temp_item->__amount) <= overlap)
//							{
//								tmp_item->__amount += temp_item->__amount;
//								MSG_USER("trade %s insert normal item %d amount %d ok",des_player_name.c_str(), temp_item->__id,temp_item->__amount);
//								ItemObj item(tmp_item->__id,temp_item->__amount,tmp_item->__bind,tmp_item->__index);
//								des_mlpacker->pack_insert_serial(item,ITEM_TRADE_GET,this->role_id());
//								tips.push_tips(GameEnum::TIPS_ITEM, temp_item->__id, temp_item->__amount);
//								befind = true;
//								break; //相同道具被重复叠加
//							}
//						}
//					}
//					if(!befind)
//					{
//						int des_index = des_package->search_empty_index_i();
//						if(des_index < 0)
//						{
//							MSG_USER("trade failure:in normal des_index < 0");
//							return -1;
//						}
//						temp_item->__index = des_index;
//						JUDGE_CONTINUE(des_package->insert(temp_item) == 0);
//						MSG_USER("trade %s insert normal item %d amount %d ok",des_player_name.c_str(), temp_item->__id,temp_item->__amount);
//						ItemObj item(temp_item->__id,temp_item->__amount,temp_item->__bind,temp_item->__index);
//						des_mlpacker->pack_insert_serial(item,ITEM_TRADE_GET,this->role_id());
//						tips.push_tips(GameEnum::TIPS_ITEM, temp_item->__id, temp_item->__amount);
//					}
//				}
//	    	}
//	    	else
//	    	{
//	    		char tips_str[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1];
//	    		::strncpy(tips_str,CONFIG_INSTANCE->item(pack_item->__id)["name"].asCString(), GameEnum::DEFAULT_MAX_CONTENT_LEGNTH);
//	    		const Json::Value &trade_fai_json = CONFIG_INSTANCE->tiny("trade_failure");
//				if(trade_fai_json == Json::Value::null)
//				{
//					MSG_USER("tiny.json-> no trade_failure fields");
//					return -1;
//				}
//	    		::strcat(tips_str,trade_fai_json.asCString());
//	    		tips_str[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';
//	    		tips.push_tips_str(tips_str);
//	    	    MSG_USER("trade %s: %s",des_player_name.c_str(),tips_str);
//	    	}
//	    }
//	}
	return 0;
}

int MLTrade::inner_notify_trade_cancel(int type)
{
	if(this->trade_info_.trade_state == TRADE_REQUESTING || this->trade_info_.trade_state == TRADE_WAITING)
	{
		MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(this->trade_info_.requ_role_id);
		if(this->trade_info_.trade_state == TRADE_WAITING)
		{//返回给被邀请者第二次交易条件不满足，不打开自己的UI
			Proto51402402 respon_condition;
			respon_condition.set_condition(0);
			this->respond_to_client(RETURN_TRADE_INVITE_RESPOND,&respon_condition);
		}
		this->trade_state_in_idle(des_player);
	}
	else
	if(this->trade_info_.trade_state == TRADE_PROCESS)
	{
		Int64 des_role_id = this->trade_info_.curr_role_id;
		MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(des_role_id);
		this->trade_update_to_map_player(des_role_id,0); //同步自己和对方取消状态到地图服
		if(!this->is_close_panel_)
		{
		  this->respond_to_client(ACTIVE_TRADE_CLOSE_PANEL);
		}
		if(des_player != NULL)
		{
		   des_player->respond_to_client(ACTIVE_TRADE_CLOSE_PANEL);
		   if(!type)
		   {
			  des_player->respond_to_client_error(RETURN_TRADE_START, ERROR_TRADE_DES_CANCEL_TRADE);
		   }
		}
		this->trade_state_in_idle(des_player);
	}
	else
	if(this->trade_info_.trade_state == TRADE_SUCCESS)
	{
		Int64 des_role_id = this->trade_info_.curr_role_id;
		MapLogicPlayer* des_player = MAP_MONITOR->find_logic_player(des_role_id);
		this->trade_update_to_map_player(des_role_id,0); //同步自己和对发取消状态到地图服
		this->trade_state_in_idle(des_player);
	}
	return 0;
}

int MLTrade::trade_close_panel()
{
	is_close_panel_ = true;
	inner_notify_trade_cancel();
	is_close_panel_ = false;
	this->respond_to_client(RETURN_TRADE_CLOSE_PANEL);
	return 0;
}

bool MLTrade::trade_check_limit_condition()
{
	int trade_level = 0;
	const Json::Value& cfg = CONFIG_INSTANCE->tiny("level_limit");
	if(cfg != Json::Value::null && cfg["trade"].asInt() != 0)
		trade_level = cfg["trade"].asInt();
	if(this->trade_player()->role_level() < trade_level)
	{
	    return false;
	}
	return true;
}

