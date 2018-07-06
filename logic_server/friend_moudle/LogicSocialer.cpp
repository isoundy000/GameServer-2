/*
 * LogicSocialer.cpp
 *
 *  Created on: 2013-10-31
 *      Author: louis
 */

#include "LogicSocialer.h"
#include "RankSystem.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "LogicFriendSystem.h"
#include "Transaction.h"
#include "TransactionMonitor.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "ProtoDefine.h"
#include "LogicLeaguer.h"
#include "GameField.h"
#include "DBCommon.h"
#include "LeagueSystem.h"
#include <mongo/client/dbclient.h>

LogicSocialer::LogicSocialer() {
	// TODO Auto-generated constructor stub
	this->socialer_detail_.reset();
}

LogicSocialer::~LogicSocialer() {
	// TODO Auto-generated destructor stub
}

void LogicSocialer::socialer_handle_player_levelup()
{

}

int LogicSocialer::fetch_friend_list(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100150*, msg_proto, -1);
	int friend_type = msg_proto->friend_type();
	CONDITION_NOTIFY_RETURN(friend_type > FRIEND_TYPE_NULL && friend_type <= FRINED_TYPE_NEARBY,
			RETURN_REQUEST_FRIEND_LIST, ERROR_CLIENT_OPERATE);
//	CONDITION_NOTIFY_RETURN(friend_type != FRIEND_TYPE_STRANGER,RETURN_REQUEST_FRIEND_LIST, ERROR_CLIENT_OPERATE);

	int64_t role_id = 0;
	LogicPlayer *player = NULL;
	FriendInfo info;
	info.__friend_type = friend_type;

	DBFriendInfo* friend_datail = POOL_MONITOR->friend_detail_pool()->pop();
	friend_datail->__friend_type = friend_type;

	switch(friend_type)
	{
		case FRIEND_TYPE_CLOSE:
		{
			LongMap::iterator it = this->socialer_detail_.__friend_list.begin();
			for(; it != this->socialer_detail_.__friend_list.end(); ++it)
			{
				role_id = it->first;
				if(LOGIC_MONITOR->find_player(role_id, player) != 0)
					friend_datail->__offine_set.insert(role_id);
				else
				{
					this->fetch_friend_info(player, info);
					friend_datail->__friend_info_vec.push_back(info);
				}
			}
			std::sort(friend_datail->__friend_info_vec.begin(),
					friend_datail->__friend_info_vec.end(),
					LogicSocialer::cmp_close_friend);
			break;
		}
		case FRIEND_TYPE_STRANGER:
		{
			LongMap::iterator it = this->socialer_detail_.__stranger_list.begin();
			for(; it != this->socialer_detail_.__stranger_list.end(); ++it)
			{
				role_id = it->first;
				if(LOGIC_MONITOR->find_player(role_id, player) != 0)
					friend_datail->__offine_set.insert(role_id);
				else
				{
					info.__stranger_tick = this->socialer_detail_.__stranger_list[role_id];
					this->fetch_friend_info(player, info);
					friend_datail->__friend_info_vec.push_back(info);
				}
			}
			std::sort(friend_datail->__friend_info_vec.begin(),
					friend_datail->__friend_info_vec.end(),
					LogicSocialer::cmp_stranger_friend);
			break;
		}
		case FRIEND_TYPE_BLACK:
		{
			LongMap::iterator it = this->socialer_detail_.__black_list.begin();
			for(; it != this->socialer_detail_.__black_list.end(); ++it)
			{
				role_id = it->first;
				if(LOGIC_MONITOR->find_player(role_id, player) != 0)
					friend_datail->__offine_set.insert(role_id);
				else
				{
					info.__black_tick = this->socialer_detail_.__black_list[role_id];
					this->fetch_friend_info(player, info);
					friend_datail->__friend_info_vec.push_back(info);
				}
			}
			std::sort(friend_datail->__friend_info_vec.begin(),
					friend_datail->__friend_info_vec.end(),
					LogicSocialer::cmp_black_friend);
			break;
		}
		case FRINED_TYPE_ENEMY:
		{
			LongMap::iterator it = this->socialer_detail_.__enemy_list.begin();
			for(; it != this->socialer_detail_.__enemy_list.end(); ++it)
			{
				role_id = it->first;
				if(LOGIC_MONITOR->find_player(role_id, player) != 0)
					friend_datail->__offine_set.insert(role_id);
				else
				{
					info.__enemy_tick = this->socialer_detail_.__enemy_list[role_id];
					this->fetch_friend_info(player, info);
					friend_datail->__friend_info_vec.push_back(info);
					}
				}
				std::sort(friend_datail->__friend_info_vec.begin(),
					friend_datail->__friend_info_vec.end(),
					LogicSocialer::cmp_enemy);
				break;
		}
		case FRINED_TYPE_NEARBY:
		{
			POOL_MONITOR->friend_detail_pool()->push(friend_datail);

             Proto30101603 req;
             req.set_role_id(this->role_id());
             return this->dispatch_to_map_server(&req);

		}
		default:
			break;
	}

	//append offline player -> load from db
	TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			TRANS_LOAD_LOGIC_SOCIALER_INFO,
			DB_FRIEND_INFO_DETAIL, friend_datail,
			POOL_MONITOR->friend_detail_pool(), LOGIC_MONITOR->logic_unit());
	return 0;
}

int LogicSocialer::fetch_nearby_player(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101603*, request, -1);
	LogicPlayer *player = NULL;

	Proto50100150 respond;
	respond.set_friend_type(FRINED_TYPE_NEARBY);

	for (int i = 0 ; i < request->nearby_list().size(); i++)
	{
		ProtoFriendInfo* proto_info = respond.add_friend_list();
		Int64 role_id = request->nearby_list(i).role_id();
		LOGIC_MONITOR->find_player(role_id, player);
		if(player != NULL)
		{
			proto_info->set_scene_id(request->nearby_list(i).scene_id());
			proto_info->set_pixel_x(request->nearby_list(i).pixel_x());
			proto_info->set_pixel_y(request->nearby_list(i).pixel_y());
			proto_info->set_role_id(player->role_id());
			proto_info->set_icon_id(0);
			proto_info->set_league_id(player->role_detail().__league_id);
			proto_info->set_vip_status(player->vip_type());
			proto_info->set_force(player->role_detail().__fight_force);
			proto_info->set_is_online(1);
			proto_info->set_sex(player->role_detail().__sex);
			proto_info->set_career(player->role_detail().__career);
			proto_info->set_level(player->role_detail().__level);
			proto_info->set_team_status( player->teamer_state());
			proto_info->set_name(player->role_detail().__name);
			proto_info->set_friend_type(FRINED_TYPE_NEARBY);
			proto_info->set_name_color(request->nearby_list(i).name_color());
		}
	player = NULL;
	}

	return this->respond_to_client(RETURN_REQUEST_FRIEND_LIST, &respond);
}

int LogicSocialer::append_to_friend_list(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100151*, msg_proto, -1);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false, RETURN_APPEND_TO_FRIEND_LIST, ERROR_IN_TRAVEL_ACTIVITY);

	//check self level
	int des_type = msg_proto->friend_type();

//	if(des_type != FRINED_TYPE_ENEMY)
//	{
//		int social_level = LogicSocialer::socialer_system_open_level();
//		CONDITION_NOTIFY_RETURN(this->role_level() >= social_level ,
//				RETURN_APPEND_TO_FRIEND_LIST, ERROR_ADD_FRIEND_LEVEL_LIMIT);
//	}

	CONDITION_NOTIFY_RETURN(des_type > FRIEND_TYPE_NULL && des_type <= FRINED_TYPE_ENEMY,
			RETURN_APPEND_TO_FRIEND_LIST, ERROR_CLIENT_OPERATE);

	int src_type = 0;
	int64_t role_id = 0;
	LongVec succeed_set;
	for(int i = 0; i < msg_proto->role_id_size(); ++i)
	{
		role_id = msg_proto->role_id(i);
		JUDGE_CONTINUE(role_id != this->role_id());

		src_type = this->modify_friend_type(role_id);
		JUDGE_CONTINUE(src_type != des_type);

		//insert to des_type
		int ret = this->insert_to_list(role_id, des_type);
		if(ret != 0)//if insert fail
		{
			//notify
			this->respond_to_client_error(RETURN_APPEND_TO_FRIEND_LIST, ret);
			break;
		}

		if (src_type == FRIEND_TYPE_BLACK)
		{
			LongVec black_set;
			black_set.push_back(role_id);
			this->logic_player()->chat_remove_black_list(black_set);
		}

		if (des_type == FRIEND_TYPE_CLOSE)
		{
			this->handle_be_added_as_friend(role_id);
		}

//		if (des_type == FRIEND_TYPE_STRANGER)
//		{
//			LogicPlayer* target_player = NULL;
//			if(LOGIC_MONITOR->find_player(role_id, target_player) == 0)
//			{
//				this->record_be_added_as_stranger_friend(target_player, this->role_id());
//			}
//		}

		if (src_type != FRIEND_TYPE_NULL)
		{
			//delete from src_list
			if(src_type == FRINED_TYPE_ENEMY && des_type == FRIEND_TYPE_CLOSE)
			{//策划要求添加好友时不删除仇人
			}
			else
			{
				this->delete_from_list(role_id, src_type);
			}
		}

		//record
		succeed_set.push_back(role_id);
	}

	if (des_type == FRIEND_TYPE_BLACK)
	{
		this->logic_player()->chat_add_black_list(succeed_set);
	}

//	if(des_type == FRIEND_TYPE_CLOSE)
//		this->logic_player()->chat_add_friend_list(succeed_set);

	//return to client
	Proto50100151 answer;
	for(int i = 0; i < (int)succeed_set.size(); ++i)
	{
		answer.add_role_id(succeed_set[i]);
	}
	answer.set_src_friend_type(src_type);
	answer.set_des_friend_type(des_type);

	MSG_DEBUG("append to friend list: des_type[%d]-succeed_size[%d]\n", des_type, succeed_set.size());
	this->socialer_detail_info();

	return this->respond_to_client(RETURN_APPEND_TO_FRIEND_LIST, &answer);
}

int LogicSocialer::send_friend_apply(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100164*, request, -1);

	Int64 friend_id = request->friend_id();
	CONDITION_NOTIFY_RETURN(friend_id != this->role_id(), RETURN_REQUEST_SEND_FRIEND_APPLY, ERROR_CAN_ADD_SELF);

	int friend_type = this->modify_friend_type(friend_id);
	CONDITION_NOTIFY_RETURN(friend_type == FRIEND_TYPE_NULL, RETURN_REQUEST_SEND_FRIEND_APPLY, ERROR_FRIEND_ON_LIST);

	LogicPlayer *player = NULL;
	CONDITION_NOTIFY_RETURN(LOGIC_MONITOR->find_player(friend_id, player) == 0,
			RETURN_REQUEST_SEND_FRIEND_APPLY, ERROR_PLAYER_OFFLINE);
	CONDITION_NOTIFY_RETURN(player != NULL, RETURN_REQUEST_SEND_FRIEND_APPLY, ERROR_PLAYER_OFFLINE);

	int ret = player->add_friend_to_apply_list(this->role_id());
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REQUEST_SEND_FRIEND_APPLY, ret);

	Proto80100109 active_req;
	ProtoApplyInfo *friend_info = active_req.mutable_friend_info();
	friend_info->set_friend_id(this->role_id());
	friend_info->set_friend_name(this->name());
	friend_info->set_level(this->role_level());
	friend_info->set_sex(this->role_detail().__sex);
	friend_info->set_tick(::time(NULL));
	friend_info->set_league_id(this->role_detail().__league_id);

	LogicPlayer *my_player = NULL;
	LOGIC_MONITOR->find_player(this->role_id(), my_player);
	if (LOGIC_MONITOR->find_player(this->role_id(), my_player) == 0)
	{
		League *league = player->league();
		if (league != NULL)
			friend_info->set_league_name(league->league_name_);
	}

	player->respond_to_client(NOTIFY_REMIND_ADD_FRIEND, &active_req);
	player->inner_notify_assist_event(GameEnum::PA_EVENT_FRIEND_ACCEPT, 1);

	return this->respond_to_client(RETURN_REQUEST_SEND_FRIEND_APPLY);
}

int LogicSocialer::accept_friend_apply(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100165*, request, -1);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
			RETURN_REQUEST_ACCEPT_FRIEND_APPLY, ERROR_IN_TRAVEL_ACTIVITY);

	int add_size = request->friend_id_set_size();
	CONDITION_NOTIFY_RETURN(this->socialer_detail_.__friend_list.size() + add_size <= MAX_CLOSE_FRIEND_NUM,
			RETURN_REQUEST_ACCEPT_FRIEND_APPLY, ERROR_CLOSE_FRIEND_NUM_LIMIT);

	if (add_size >= (int)this->socialer_detail_.__apply_map.size())
	{
		this->inner_notify_assist_event(GameEnum::PA_EVENT_FRIEND_ACCEPT, 0);
	}

	FriendInfo info;
	info.__friend_type = FRIEND_TYPE_CLOSE;
	DBFriendInfo* friend_datail = POOL_MONITOR->friend_detail_pool()->pop();
	friend_datail->__friend_type = FRIEND_TYPE_CLOSE;

	LogicPlayer *player = NULL;
	int64_t role_id = 0;
	for(int i = 0; i < request->friend_id_set_size(); ++i)
	{
		role_id = request->friend_id_set(i);
		JUDGE_CONTINUE(role_id != this->role_id());
		JUDGE_CONTINUE(this->socialer_detail_.__apply_map.count(role_id) > 0);
//		JUDGE_CONTINUE(this->socialer_detail_.__friend_list.count(role_id) == 0);

		this->insert_to_list(role_id, FRIEND_TYPE_CLOSE);
		this->handle_be_added_as_friend(role_id);
		this->remove_apply_player(role_id);

		if(LOGIC_MONITOR->find_player(role_id, player) != 0)
			friend_datail->__offine_set.insert(role_id);
		else
		{
			this->fetch_friend_info(player, info);
			friend_datail->__friend_info_vec.push_back(info);

			std::sort(friend_datail->__friend_info_vec.begin(),
					friend_datail->__friend_info_vec.end(),
					LogicSocialer::cmp_close_friend);
		}
	}
	TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			TRANS_LOAD_LOGIC_APPLY_INFO,
			DB_FRIEND_INFO_DETAIL, friend_datail,
			POOL_MONITOR->friend_detail_pool(), LOGIC_MONITOR->logic_unit());

	return 0;
}

int LogicSocialer::remove_apply_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100166*, request, -1);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
			RETURN_REQUEST_DELETE_FRIEND_APPLY, ERROR_IN_TRAVEL_ACTIVITY);

	int remove_size = request->friend_id_set_size();
	if (remove_size >= (int)this->socialer_detail_.__apply_map.size())
	{
		this->inner_notify_assist_event(GameEnum::PA_EVENT_FRIEND_ACCEPT, 0);
	}

	Proto50100166 respond;
	int64_t role_id = 0;
	for(int i = 0; i < request->friend_id_set_size(); ++i)
	{
		role_id = request->friend_id_set(i);
		this->remove_apply_player(role_id);
		respond.add_role_set(role_id);
	}
//	this->request_fetch_apply_list();


	return this->respond_to_client(RETURN_REQUEST_DELETE_FRIEND_APPLY, &respond);
}

int LogicSocialer::request_fetch_apply_list()
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
			RETURN_REQUEST_FETCH_APPLY_LIST, ERROR_IN_TRAVEL_ACTIVITY);

	Proto50100167 respond;
	for (LogicSocialerDetail::ApplyInfoMap::iterator iter = this->socialer_detail_.__apply_map.begin();
			iter != this->socialer_detail_.__apply_map.end(); ++iter)
	{
		LogicSocialerDetail::ApplyInfo &apply_info = iter->second;
		ProtoApplyInfo *apply_list = respond.add_apply_list();
		apply_list->set_friend_id(apply_info.friend_id_);
		apply_list->set_friend_name(apply_info.friend_name_);
		apply_list->set_league_id(apply_info.league_id_);
		apply_list->set_league_name(apply_info.league_name_);
		apply_list->set_level(apply_info.level_);
		apply_list->set_sex(apply_info.sex_);
		apply_list->set_tick(apply_info.tick_);
	}

	if (this->socialer_detail_.__apply_map.size() > 0)
	{
		this->inner_notify_assist_event(GameEnum::PA_EVENT_FRIEND_ACCEPT, 1);
	}

	return this->respond_to_client(RETURN_REQUEST_FETCH_APPLY_LIST, &respond);
}

int LogicSocialer::send_friend_to_black(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100158*, msg_proto, -1);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
			RETURN_SEND_FRIEND_TO_BLACK, ERROR_IN_TRAVEL_ACTIVITY);

	Int64 role_id = msg_proto->role_id();
	int src_type = this->modify_friend_type(role_id);
	CONDITION_NOTIFY_RETURN(src_type != FRIEND_TYPE_BLACK,
			RETURN_SEND_FRIEND_TO_BLACK, ERROR_IN_BLACK);

	int ret = this->insert_to_list(role_id, FRIEND_TYPE_BLACK);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SEND_FRIEND_TO_BLACK, ret);

	if (src_type != FRIEND_TYPE_NULL)
	{
		this->delete_from_list(role_id, src_type);
	}
	LongVec succeed_set;
	succeed_set.push_back(role_id);
	this->logic_player()->chat_add_black_list(succeed_set);

	Proto50100158 answer;
	answer.set_role_id(role_id);
	answer.set_src_friend_type(src_type);
	answer.set_des_friend_type(FRIEND_TYPE_BLACK);

	this->socialer_detail_info();

	return this->respond_to_client(RETURN_SEND_FRIEND_TO_BLACK, &answer);
}

int LogicSocialer::remove_black_friend(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100159*, msg_proto, -1);

	Int64 role_id = msg_proto->role_id();
	int src_type = this->modify_friend_type(role_id);
	CONDITION_NOTIFY_RETURN(src_type == FRIEND_TYPE_BLACK,
			RETURN_REMOVE_BLACK_FRIEND, ERROR_CLIENT_OPERATE);

	int remove_type = msg_proto->remove_type();
	int des_type = 0;
	if (remove_type == 0)
	{
		des_type = FRIEND_TYPE_CLOSE;
		int ret = this->insert_to_list(role_id, FRIEND_TYPE_CLOSE);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REMOVE_BLACK_FRIEND, ret);
	}
	else
	{
		des_type = FRIEND_TYPE_NULL;
		this->remove_player_friend_list(role_id, FRIEND_TYPE_CLOSE);
	}
	this->delete_from_list(role_id, src_type);

	LongVec black_set;
	black_set.push_back(role_id);
	this->logic_player()->chat_remove_black_list(black_set);

	Proto50100159 answer;
	answer.set_role_id(role_id);
	answer.set_src_friend_type(src_type);
	answer.set_des_friend_type(FRIEND_TYPE_CLOSE);

	this->socialer_detail_info();
	return this->respond_to_client(RETURN_REMOVE_BLACK_FRIEND, &answer);
}

int LogicSocialer::remove_from_friend_list(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100152*, msg_proto, -1);
	int src_type = msg_proto->src_friend_type();
	CONDITION_NOTIFY_RETURN(src_type > FRIEND_TYPE_NULL &&
			src_type <= FRIEND_TYPE_BLACK,
			RETURN_REMOVE_FROM_FRIEND_LIST, ERROR_CLIENT_OPERATE);

	Proto50100152 respond;
	LongVec remove_set;
	for(int i = 0; i < msg_proto->role_id_size(); ++i)
	{
		int64_t role_id = msg_proto->role_id(i);

		Int64 partner_id = this->logic_player()->fetch_wedding_partner_id();
		JUDGE_CONTINUE(partner_id != role_id);

		//delete from src_list
		this->delete_from_list(role_id, src_type);
		remove_set.push_back(role_id);
		respond.add_role_set(role_id);

		if (src_type == FRIEND_TYPE_CLOSE)
		{
			this->remove_player_friend_list(role_id, FRIEND_TYPE_CLOSE);
			this->logic_player()->set_intimacy(role_id, 0);
		}
	}
	if(src_type == FRIEND_TYPE_BLACK && remove_set.size() > 0)
		this->logic_player()->chat_remove_black_list(remove_set);

	this->socialer_detail_info();
	return this->respond_to_client(RETURN_REMOVE_FROM_FRIEND_LIST, &respond);
}

int LogicSocialer::remove_player_friend_list(Int64 role_id, int src_type)
{
	LogicPlayer *player = NULL;
	if (LOGIC_MONITOR->find_player(role_id, player) == 0)
	{
		player->delete_from_list(this->role_id(), src_type);

		Proto80100110 active_res;
		active_res.set_role_id(this->role_id());
		active_res.set_role_name(this->name());
		active_res.set_level(this->role_detail().__level);
		player->respond_to_client(NOTIFY_DELETE_FRIEND, &active_res);

		player->set_intimacy(this->role_id(), 0);
	}
	else
	{
		LOGIC_FRIEND_SYSTEM->insert_offline_friend_pair(role_id, this->role_id(), FriendPairInfo::FRIEND_DELETE);
	}

	return 0;
}

int LogicSocialer::search_friend_by_name(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100153*, msg_proto, -1);
	//find online player
//	LogicPlayer *player = NULL;
	std::string name = msg_proto->name();

	StringVec player_set;
	LOGIC_MONITOR->find_player_name(name, player_set);

	CONDITION_NOTIFY_RETURN(player_set.size() > 0,
			RETURN_SEARCH_FRIEND_BY_NAME, ERROR_PLAYER_OFFLINE);

	Proto50100153 answer;
	for (StringVec::iterator iter = player_set.begin(); iter != player_set.end(); ++iter)
	{
		LogicPlayer *player = NULL;
		JUDGE_CONTINUE(LOGIC_MONITOR->find_player(*iter, player) == 0);
		JUDGE_CONTINUE(player != NULL);
		JUDGE_CONTINUE(this->role_id() != player->role_id());

		FriendInfo info;
		info.__friend_type = this->modify_friend_type(player->role_id());
		this->fetch_friend_info(player, info);

		ProtoFriendInfo* proto_info = answer.add_friend_info();
		this->serialize(proto_info, info);
	}
//	CONDITION_NOTIFY_RETURN(LOGIC_MONITOR->find_player(name, player) == 0,
//			RETURN_SEARCH_FRIEND_BY_NAME, ERROR_PLAYER_OFFLINE);

//	JUDGE_RETURN(player != NULL, -1);

//	CONDITION_NOTIFY_RETURN(player->role_level() > 1,
//			RETURN_SEARCH_FRIEND_BY_NAME, ERROR_ADDED_FRIEND_LOW_LEVEL);

//	FriendInfo info;
//	info.__friend_type = this->modify_friend_type(player->role_id());
//	this->fetch_friend_info(player, info);

//	Proto50100153 answer;
//	ProtoFriendInfo* proto_info = answer.mutable_friend_info();
//	this->serialize(proto_info, info);

	return this->respond_to_client(RETURN_SEARCH_FRIEND_BY_NAME, &answer);
}


int LogicSocialer::recommend_friend(Message *msg)
{
//	CONDITION_NOTIFY_RETURN(this->socialer_detail_.__friend_list.size() <
//			MAX_CLOSE_FRIEND_NUM,
//			RETURN_RECOMMEND_FRIEND,
//			ERROR_RECOMMEND_FRIEND_HITH_WATER_FLOW);

	if (this->socialer_detail_.__friend_list.size() >= MAX_CLOSE_FRIEND_NUM)
	{
		Proto50100154 respond;
		return this->respond_to_client(RETURN_RECOMMEND_FRIEND, &respond);
	}

	/*
	LongSet target_set;
	int calc_cnt = 0, cur_need_cnt = 0, dir = 0;


	int calc_limit = this->recommend_friend_calc_limit();
	CONDITION_NOTIFY_RETURN(calc_limit != -1,
			RETURN_RECOMMEND_FRIEND, ERROR_CLIENT_OPERATE);
	//calc target player set : target_set
	while(calc_cnt <= calc_limit)
	{
		dir = (calc_cnt % 2 == 0) ?
				FRIEND_RECOMMEND_LEVEL_UP : FRIEND_RECOMMEND_LEVEL_DOWN;
		cur_need_cnt = FRIEND_RECOMMEND_PER_NUM - target_set.size();
		this->recommend_friend_ex(dir, calc_cnt, cur_need_cnt, target_set);
		if(target_set.size() < FRIEND_RECOMMEND_PER_NUM)
		{
			++calc_cnt;
		}
		else
			break;
	}
	 */
	//fetch friend info : friend_vec
	FriendInfo info;
	FriendInfoSet friend_vec;

	//首先推荐等级高的玩家
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;

		JUDGE_CONTINUE(player->role_level() >= this->role_level() && player->role_id() != this->role_id()
				&& !player->is_friend(this->role_id()) && !player->socialer_detail_.__apply_map.count(this->role_id()));

		info.reset();
		this->fetch_friend_info(player, info);
		friend_vec.push_back(info);

		if (friend_vec.size() >= MAX_RECOMMEND_NUM) break;
	}

	//推荐等级低的
	if (friend_vec.size() < MAX_RECOMMEND_NUM)
	{
		for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
				iter != player_map.end(); ++iter)
		{
			LogicPlayer* player = iter->second;
			JUDGE_CONTINUE(player->role_level() < this->role_level() && player->role_id() != this->role_id()
					&& !player->is_friend(this->role_id()) && !player->socialer_detail_.__apply_map.count(this->role_id()));

			info.reset();
			this->fetch_friend_info(player, info);
			friend_vec.push_back(info);

			if (friend_vec.size() >= MAX_RECOMMEND_NUM) break;
		}
	}

	//return to client
	Proto50100154 answer;
	FriendInfoSet::iterator it = friend_vec.begin();
	for(; it != friend_vec.end(); ++it)
	{
		ProtoFriendInfo *proto_info = answer.add_friend_list();
		this->serialize(proto_info, *it);
	}

	return this->respond_to_client(RETURN_RECOMMEND_FRIEND, &answer);
}

int LogicSocialer::fetch_friend_info_by_role_id(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100155*, msg_proto, -1);
	int64_t role_id = msg_proto->role_id();
	LogicPlayer* player = NULL;
	FriendInfo info;
	if(LOGIC_MONITOR->find_player(role_id, player) == 0)
	{
		this->fetch_friend_info(player, info);
		//return to client
		Proto50100155 answer;
		ProtoFriendInfo *proto_info = answer.mutable_friend_info();
		this->serialize(proto_info, info);
		FINER_PROCESS_RETURN(RETURN_FETCH_SINGLE_PLAYER_INFO, &answer);
	}
	else
	{
		int friend_type = this->modify_friend_type(role_id);
		this->load_friend_info_offine(TRANS_LOAD_SINGLE_LOGIC_SOCIALER_INFO,
				friend_type);
	}
	return 0;
}

int LogicSocialer::fetch_single_player_all_by_role_id(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100156*, request, -1);
	Int64 player_id = request->role_id();

	LogicPlayer* player = this->find_player(player_id);
//	int op_type = request->op_type();
	if(NULL == player) // offline
	{
		this->request_load_player_detail(player_id);
	}
	else // on line
	{
		Proto50100156 respond;
//		respond.set_op_type(op_type);
		respond.set_role_id(player_id);
		respond.set_role_name(player->role_detail().__name);
		respond.set_league_id(player->role_detail().__league_id);

		League* league = this->logic_player()->find_league(
				player->role_detail().__league_id);
		if(NULL != league)
			respond.set_league_name(league->league_name_);

		respond.set_level(player->role_detail().__level);
		respond.set_career(player->role_detail().__career);
		respond.set_sex(player->role_detail().__sex);
		respond.set_vip_status(player->vip_type());
		respond.set_checker_id(this->role_id());

//		MSG_USER(logic_server-my_name:%s-my_id:%ld-my_scene:%d-query_name:%s-query_id:%ld-query_scene:%d-respond:%s,
//				this->role_detail().__name, this->role_id(),this->scene_id(),
//				player->role_detail().__name, player_id,player->scene_id(),
//				respond.Utf8DebugString().c_str());

		LOGIC_MONITOR->dispatch_to_scene(player, &respond);
	}
	return 0;
}

LogicSocialerDetail &LogicSocialer::socialer_detail(void)
{
	return this->socialer_detail_;
}


int LogicSocialer::after_fetch_friend_list(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	TransactionData* trans_data = trans->fetch_data(DB_FRIEND_INFO_DETAIL);
	if(trans_data == 0)
	{
		trans->rollback();
		return -1;
	}

	DBFriendInfo *friend_datail = trans_data->__data.__friend_info_detail;
	trans_data->reset();

	Proto50100150 send_proto;
	send_proto.set_friend_type(friend_datail->__friend_type);
	FriendInfoSet::iterator it = friend_datail->__friend_info_vec.begin();
	for (; it != friend_datail->__friend_info_vec.end(); ++it)
	{
		ProtoFriendInfo *info_proto = send_proto.add_friend_list();
		this->serialize(info_proto, *it);
	}

	POOL_MONITOR->friend_detail_pool()->push(friend_datail);
	trans->summit();

	return this->respond_to_client(RETURN_REQUEST_FRIEND_LIST, &send_proto);
}

int LogicSocialer::after_accept_friend_apply(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	TransactionData* trans_data = trans->fetch_data(DB_FRIEND_INFO_DETAIL);
	if(trans_data == 0)
	{
		trans->rollback();
		return -1;
	}

	DBFriendInfo *friend_datail = trans_data->__data.__friend_info_detail;
	trans_data->reset();

	Proto50100165 send_proto;
	FriendInfoSet::iterator it = friend_datail->__friend_info_vec.begin();
	for (; it != friend_datail->__friend_info_vec.end(); ++it)
	{
		ProtoFriendInfo *info_proto = send_proto.add_friend_list();
		this->serialize(info_proto, *it);
	}
	POOL_MONITOR->friend_detail_pool()->push(friend_datail);
	trans->summit();

	return this->respond_to_client(RETURN_REQUEST_ACCEPT_FRIEND_APPLY, &send_proto);
}


int LogicSocialer::record_be_added_as_stranger_friend(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200119*, request, -1);
	this->record_be_added_as_stranger_friend(this->logic_player(),
			request->role_id());
	return 0;
}

int LogicSocialer::after_fetch_friend_info_by_role_id(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	TransactionData* trans_data = trans->fetch_data(DB_FRIEND_INFO_DETAIL);
	if(trans_data == 0)
		return -1;

	DBFriendInfo *friend_datail = trans_data->__data.__friend_info_detail;
	trans_data->reset();

	Proto50100155 send_proto;
	FriendInfoSet::iterator it = friend_datail->__friend_info_vec.begin();
	for (; it != friend_datail->__friend_info_vec.end(); ++it)
	{
		ProtoFriendInfo *info_proto = send_proto.mutable_friend_info();
		this->serialize(info_proto, *it);
	}
	trans->summit();
	POOL_MONITOR->friend_detail_pool()->push(friend_datail);

	return this->respond_to_client(RETURN_FETCH_SINGLE_PLAYER_INFO, &send_proto);
}

int LogicSocialer::after_fetch_single_player_all_info(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	MongoDataMap* data_map = trans->fetch_mongo_data_map();
	if(data_map == 0)
	{
		trans->rollback();
		return -1;
	}

	MongoData* mongo_data = NULL;
	if(data_map->find_data(DBRanker::COLLECTION, mongo_data) == 0)
	{
		BSONObj obj = mongo_data->data_bson();
		Proto50100156 respond;
//		respond.set_op_type(data_map->other_value_);
		int ret = this->bson2player_info(obj, respond);
		if (ret != 0)
		{
			this->respond_to_client_error(RETURN_FETCH_SINGLE_PLAYER_DETAIL, ret);
//			LOGIC_MONITOR->dispatch_to_client(this->gate_sid(), this->role_id(), ret);
		}
		else
		{
			if (respond.role_id() <= 0)
			{
				this->respond_to_client_error(RETURN_FETCH_SINGLE_PLAYER_DETAIL, ERROR_ROLE_NOT_EXISTS);
//				LOGIC_MONITOR->dispatch_to_client(this->gate_sid(), this->role_id(), ERROR_ROLE_NOT_EXISTS);
			}
			else
			{
				this->respond_to_client(RETURN_FETCH_SINGLE_PLAYER_DETAIL, &respond);
//				LOGIC_MONITOR->dispatch_to_client(this->gate_sid(), this->role_id(), 0, &respond);
			}
		}
	}
	trans->summit();
	return 0;
}

int LogicSocialer::modify_friend_type(const int64_t role_id)
{
	JUDGE_RETURN(this->socialer_detail_.__friend_list.count(role_id) == 0, FRIEND_TYPE_CLOSE);
	JUDGE_RETURN(this->socialer_detail_.__stranger_list.count(role_id) == 0, FRIEND_TYPE_STRANGER);
	JUDGE_RETURN(this->socialer_detail_.__black_list.count(role_id) == 0, FRIEND_TYPE_BLACK);
	JUDGE_RETURN(this->socialer_detail_.__enemy_list.count(role_id) == 0, FRINED_TYPE_ENEMY);
	JUDGE_RETURN(this->socialer_detail_.__nearby_list.count(role_id) == 0, FRINED_TYPE_NEARBY);
	return FRIEND_TYPE_NULL;
}

int LogicSocialer::fetch_friend_info(LogicPlayer *player, FriendInfo& info)
{
	info.__role_id = player->role_id();
	info.__icon_id = 0;//
	info.__league_id = player->role_detail().__league_id;
	info.__scene_id = player->scene_id();
	info.__vip_status = player->vip_type();
	info.__force = player->role_detail().__fight_force;
	info.__is_online = 1;
	info.__sex = player->role_detail().__sex;
	info.__career = player->role_detail().__career;
	info.__level = player->role_detail().__level;
	info.__team_status = player->teamer_state();

	int killing_value = player->killing_value();
	int kill_num = player->kill_num();
	info.__name_color = GameCommon::fetch_name_color(kill_num,killing_value);
	info.__name = player->role_detail().__name;
	return 0;
}

int LogicSocialer::load_friend_info_offine(const int recogn, const int friend_type)
{
	DBFriendInfo* friend_datail = POOL_MONITOR->friend_detail_pool()->pop();
	friend_datail->__friend_type = friend_type;
	TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), recogn,
			DB_FRIEND_INFO_DETAIL, friend_datail,
			POOL_MONITOR->friend_detail_pool(), LOGIC_MONITOR->logic_unit());
	return 0;
}

int LogicSocialer::insert_to_list(const int64_t role_id, const int des_type)
{
//	LogicPlayer* target_player = NULL;
//	if(this->monitor()->find_player(role_id, target_player) == 0)
//	{
//		if(NULL != target_player && target_player->role_level() < 2)
//			return ERROR_ADDED_FRIEND_LOW_LEVEL;
//	}

	Int64 now = ::time(NULL);
	switch(des_type)
	{
		case FRIEND_TYPE_CLOSE:
			JUDGE_RETURN(this->socialer_detail_.__friend_list.size() <
					MAX_CLOSE_FRIEND_NUM, ERROR_CLOSE_FRIEND_NUM_LIMIT);
			this->socialer_detail_.__friend_list[role_id] = now;
			break;
		case FRIEND_TYPE_STRANGER:
			this->socialer_detail_.__stranger_list[role_id] = now;
			if(this->socialer_detail_.__stranger_list.size() >
			MAX_STRANGER_FRIEND_NUM)
				this->stranger_list_limit_size();
			break;
		case FRIEND_TYPE_BLACK:
//			JUDGE_RETURN(this->socialer_detail_.__black_list.size() <
//					MAX_BLACK_LIST_SIZE, ERROR_BLACK_LIST_SIZE_LIMIT);
			this->socialer_detail_.__black_list.insert(
					std::pair<int64_t,int>(role_id, now));
			break;
		case FRINED_TYPE_ENEMY:
			this->socialer_detail_.__enemy_list[role_id] = now;
			if(this->socialer_detail_.__enemy_list.size() > MAX_ENEMY_NUM)
				this->enemy_list_limit_size();
			break;
		case FRINED_TYPE_NEARBY:
			JUDGE_RETURN(this->socialer_detail_.__nearby_list.size() <
					MAX_NEARBY_NUM, ERROR_MAX_NEARBY_LIMIT);
			this->socialer_detail_.__nearby_list.insert(
					std::pair<int64_t,int>(role_id, now));
			break;
		default:
			break;
	}
	return 0;
}

int LogicSocialer::delete_from_list(const int64_t role_id, const int des_type)
{
	switch(des_type)
	{
		case FRIEND_TYPE_CLOSE:
			this->socialer_detail_.__friend_list.erase(role_id);
			break;
		case FRIEND_TYPE_STRANGER:
			this->socialer_detail_.__stranger_list.erase(role_id);
			break;
		case FRIEND_TYPE_BLACK:
			this->socialer_detail_.__black_list.erase(role_id);
			break;
		case FRINED_TYPE_ENEMY:
			this->socialer_detail_.__enemy_list.erase(role_id);
			break;
		case FRINED_TYPE_NEARBY:
			this->socialer_detail_.__nearby_list.erase(role_id);
		    break;
		default:
			break;
	}
	return 0;
}

int LogicSocialer::handle_be_added_as_friend(const int64_t target_id)
{
	LogicPlayer* target_player = NULL;
	if(LOGIC_MONITOR->find_player(target_id, target_player) == 0)
	{
		this->active_notify_be_added_to_friend(target_player);
		target_player->insert_to_list(this->role_id(), FRIEND_TYPE_CLOSE);
//		this->record_be_added_as_stranger_friend(target_player, this->role_id());
	}
	else
	{
		LOGIC_FRIEND_SYSTEM->insert_offline_friend_pair(target_id, this->role_id(), FriendPairInfo::FRIEND_ADD);
	}
	return 0;
}

int LogicSocialer::active_notify_be_added_to_friend(LogicPlayer* player)
{
	Proto80100101 answer;
	LogicPlayer *my_player = NULL;
	if (LOGIC_MONITOR->find_player(this->role_id(), my_player) == 0)
	{
		FriendInfo info;
		info.__friend_type = FRIEND_TYPE_CLOSE;
		this->fetch_friend_info(my_player, info);

		ProtoFriendInfo* proto_info = answer.mutable_friend_info();
		this->serialize(proto_info, info);
	}
	player->respond_to_client(ACTIVE_BE_ADDED_FRIEND, &answer);
	return 0;
}

int LogicSocialer::record_be_added_as_stranger_friend(LogicPlayer* player, Int64 maker_id)
{
	int friend_type = player->modify_friend_type(maker_id);
	JUDGE_RETURN(friend_type == FRIEND_TYPE_NULL ||
			friend_type == FRIEND_TYPE_STRANGER, 0);

	player->insert_to_list(maker_id, FRIEND_TYPE_STRANGER);
	return 0;
}

int LogicSocialer::stranger_list_limit_size()
{
	IntSet tick_set;
	LongMap::iterator it = this->socialer_detail_.__stranger_list.begin();
	for(; it != this->socialer_detail_.__stranger_list.end(); ++it)
	{
		tick_set.insert(it->second);
	}
	int old_tick = *(tick_set.begin());

	LongMap::iterator iter = this->socialer_detail_.__stranger_list.begin();
	for(; iter != this->socialer_detail_.__stranger_list.end(); ++iter)
	{
		if(iter->second == old_tick)
			break;
	}

	JUDGE_RETURN(iter != this->socialer_detail_.__stranger_list.end(), -1);
	this->socialer_detail_.__stranger_list.erase(iter);
	return 0;
}

int LogicSocialer::enemy_list_limit_size()
{
	LongVec tick_vec;
	LongMap::iterator it = this->socialer_detail_.__enemy_list.begin();
	for(; it != this->socialer_detail_.__enemy_list.end(); ++it)
	{
		tick_vec.push_back(it->second);
	}
	std::sort(tick_vec.begin(),tick_vec.end());

	LongMap::iterator iter = this->socialer_detail_.__enemy_list.begin();
	for(; iter != this->socialer_detail_.__enemy_list.end(); ++iter)
	{
		if(iter->second == tick_vec.front())
		{
			this->socialer_detail_.__enemy_list.erase(iter);
		}
			break;
	}
	return 0;
}

int LogicSocialer::translate_to_enemy_position(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100157*, request, -1);
	int64_t role_id = request->role_id();
	LogicPlayer *player = NULL;
	LOGIC_MONITOR->find_player(role_id, player);
	if(player != NULL)
	{
		int scene_id = player->scene_id();

		Proto30101602 req;
		req.set_scene_id(scene_id);
		req.set_enemy_id(role_id);
		req.set_role_id(this->role_id());
		return  LOGIC_MONITOR->dispatch_to_scene(player, scene_id, &req);
	}

	return 0;
}

int LogicSocialer::after_fetch_to_enemy_position(Message* msg)
{

	MSG_DYNAMIC_CAST_RETURN(Proto30101602 *, request, -1);
	Int64 role_id = request->role_id();
	Int64 enemy_id = request->enemy_id();

	LogicPlayer* player = this->find_player(role_id);
	if(player != NULL)
	{
		 Proto50100157 respond;
		 respond.set_scene_id(request->scene_id());
		 respond.set_role_id(enemy_id);
		 respond.set_pixel_x(request->pixel_x());
		 respond.set_pixel_y(request->pixel_y());
		 return player->respond_to_client(RETURN_TRANSALTE_TO_ENEMY_POSTION,&respond);
	}
    return 0;
}


int LogicSocialer::recommend_friend_ex(int dir, int cur_cnt, int need_num, LongSet& result)
{
	int my_level = logic_player()->role_detail().__level;
	int max_level = 0, min_level = 0;
	int top_level = CONFIG_INSTANCE->top_level();
	switch(dir)
	{
		case FRIEND_RECOMMEND_LEVEL_UP:
		{
			max_level = my_level + (cur_cnt/2 + 1) * FRIEND_RECOMMEND_LEVEL_GAP;
			min_level = my_level + (cur_cnt/2) * FRIEND_RECOMMEND_LEVEL_GAP;
			break;
		}
		case FRIEND_RECOMMEND_LEVEL_DOWN:
		{
			max_level = my_level - (cur_cnt/2) * FRIEND_RECOMMEND_LEVEL_GAP - 1;
			min_level = my_level - (cur_cnt/2 + 1) * FRIEND_RECOMMEND_LEVEL_GAP;
			break;
		}
		default:
			break;
	}

//	max_level = max_level > 0 ? max_level : 0;
//	min_level = min_level > 0 ? min_level : 0;

	if((max_level <= 1 && min_level <= 1) || (max_level >= top_level && min_level >= top_level))
		return 0;

	if(min_level <= 1)
		min_level = 2;

	MSG_DEBUG("** recommend friends **: -[id:%ld][lvl:%d][name:%s]-%d-%d\n",
			this->role_id(), my_level, this->name(), max_level, min_level);

	int64_t target_id = 0;
	LogicPlayer *target_player = NULL;
	std::vector<int64_t> cur_set;
	int target_lvl = 0;
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	LogicMonitor::PlayerMap::iterator it = player_map.begin();
	for(; it != player_map.end(); ++it)
	{
		target_id = it->first;
		if(LOGIC_MONITOR->find_player(target_id, target_player) == 0)
		{
			JUDGE_CONTINUE(target_player != NULL);
			JUDGE_CONTINUE(target_id != logic_player()->role_id());
			if(this->modify_friend_type(target_id) == FRIEND_TYPE_CLOSE ||
					this->modify_friend_type(target_id) == FRIEND_TYPE_BLACK)
				continue;

			target_lvl = target_player->role_detail().__level;
			if(target_lvl >= min_level && target_lvl <= max_level )
			{
				cur_set.push_back(target_id);
				MSG_DEBUG("recommend friends : -[%d][%d][%d][dir:%d][cur_cnt:%d][need_num:%d][cur_set:%d][target_id:%ld][name:%s]-\n",
						min_level, my_level, max_level, dir, cur_cnt, need_num, cur_set.size(), target_id, target_player->name());
			}
		}
	}

	JUDGE_RETURN(cur_set.size() != 0, -1);
	std::random_shuffle(cur_set.begin(), cur_set.end());

	int cnt = std::min(need_num, (int)cur_set.size());
	result.insert(cur_set.begin(), cur_set.begin() + cnt);
	return 0;
}

int LogicSocialer::recommend_friend_calc_limit(void)
{
	int calc_limit = -1;
	int my_level = logic_player()->role_detail().__level;
//#ifdef LOCAL_DEBUG
//	int high_limit = (20 - my_level) % FRIEND_RECOMMEND_LEVEL_GAP == 0 ?
//			(20 - my_level) / FRIEND_RECOMMEND_LEVEL_GAP :
//	        (20 - my_level) / FRIEND_RECOMMEND_LEVEL_GAP  + 1;
//#else
	int high_limit = (MAX_PLAYER_LEVEL - my_level) % FRIEND_RECOMMEND_LEVEL_GAP == 0 ?
			(MAX_PLAYER_LEVEL - my_level) / FRIEND_RECOMMEND_LEVEL_GAP :
	        (MAX_PLAYER_LEVEL - my_level) / FRIEND_RECOMMEND_LEVEL_GAP + 1;
//#endif
	int low_limit = my_level % FRIEND_RECOMMEND_LEVEL_GAP == 0 ?
			my_level / FRIEND_RECOMMEND_LEVEL_GAP : my_level / FRIEND_RECOMMEND_LEVEL_GAP + 1;
	calc_limit = std::max(high_limit, low_limit) * 2;
	return calc_limit;
}

int LogicSocialer::socialer_system_open_level(void)
{
	int social_level = 21;
	const Json::Value& cfg = CONFIG_INSTANCE->tiny("level_limit");
	if(cfg != Json::Value::null && cfg["add_friend"].asInt() != 0)
		social_level = cfg["add_friend"].asInt();

	return social_level;
}

bool LogicSocialer::cmp_close_friend(const FriendInfo& lhs, const FriendInfo& rhs)
{
	return lhs.__vip_status < rhs.__vip_status;
}

bool LogicSocialer::cmp_stranger_friend(const FriendInfo& lhs, const FriendInfo& rhs)
{
	return lhs.__stranger_tick > rhs.__stranger_tick;
}

bool LogicSocialer::cmp_black_friend(const FriendInfo& lhs, const FriendInfo& rhs)
{
	return lhs.__black_tick > rhs.__black_tick;
}

bool LogicSocialer::cmp_enemy(const FriendInfo& lhs, const FriendInfo& rhs)
{
	return lhs.__enemy_tick > rhs.__enemy_tick;
}

int LogicSocialer::serialize(ProtoFriendInfo *msg_proto, FriendInfo& info)
{
	msg_proto->set_role_id(info.__role_id);
	msg_proto->set_icon_id(info.__icon_id);
	msg_proto->set_league_id(info.__league_id);
	msg_proto->set_scene_id(info.__scene_id);
	msg_proto->set_friend_type(info.__friend_type);
	msg_proto->set_vip_status(info.__vip_status);
	msg_proto->set_is_online(info.__is_online);
	msg_proto->set_sex(info.__sex);
	msg_proto->set_career(info.__career);
	msg_proto->set_level(info.__level);
	msg_proto->set_team_status(info.__team_status);
	msg_proto->set_name(info.__name);
	msg_proto->set_force(info.__force);
	msg_proto->set_name_color(info.__name_color);
	msg_proto->set_intimacy(this->logic_player()->intimacy(info.__role_id));

	League *league = LEAGUE_SYSTEM->find_league(info.__league_id);
	if (league != NULL)
	{
		msg_proto->set_league_name(league->league_name_);
	}

	return 0;
}

int LogicSocialer::unserialize(ProtoFriendInfo *msg_proto, FriendInfo& info)
{
	info.__role_id = msg_proto->role_id();
	info.__icon_id = msg_proto->icon_id();
	info.__league_id = msg_proto->league_id();
	info.__friend_type = msg_proto->friend_type();
	info.__vip_status = msg_proto->vip_status();
	info.__is_online = msg_proto->is_online();
	info.__sex = msg_proto->sex();
	info.__force = msg_proto->force();
	info.__career = msg_proto->career();
	info.__level = msg_proto->level();
	info.__team_status = msg_proto->team_status();
	info.__name = msg_proto->name();
	return 0;
}

int LogicSocialer::request_load_player_detail(Int64 role_id)
{
	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	data_map->push_find(DBRanker::COLLECTION, BSON(DBRanker::ID << role_id));
	TRANSACTION_MONITOR->request_mongo_transaction(this->logic_player()->role_id(),
			TRANS_LOAD_SINGLE_PLAYER_ALL_INFO, data_map, LOGIC_MONITOR->logic_unit());
	return 0;
}

int LogicSocialer::bson2player_info(BSONObj& res, Proto50100156& respond)
{
	JUDGE_RETURN(res.isEmpty() == false, ERROR_ROLE_NOT_EXISTS);
	respond.set_role_id(res[DBRanker::ID].numberLong());
	respond.set_league_id(res[DBRanker::LEAGUE_ID].numberLong());
	respond.set_role_name(res[DBRanker::NAME].str());
	respond.set_league_name(res[DBRanker::LEAGUE_NAME].str());
	respond.set_level(res[DBRanker::FIGHT_LEVEL].numberInt());
	respond.set_career(res[DBRanker::CAREER].numberInt());
	respond.set_sex(res[Role::SEX].numberInt());
	respond.set_vip_status(res[DBRanker::VIP_STATUS].numberInt());
	respond.set_fight_force(res[DBRanker::FIGHT_FORCE].numberInt());

	respond.set_attack(res[DBRanker::ATTACK_UPPER].numberInt());
	respond.set_defence(res[DBRanker::DEFENCE_UPPER].numberInt());
	respond.set_max_blood(res[DBRanker::MAX_BLOOD].numberInt());
	respond.set_crit(res[DBRanker::CRIT].numberInt());
	respond.set_toughness(res[DBRanker::TOUGHNESS].numberInt());
	respond.set_hit(res[DBRanker::HIT].numberInt());
	respond.set_avoid(res[DBRanker::DODGE].numberInt());
	respond.set_crit_hurt(res[DBRanker::CRIT_HIT].numberInt());
	respond.set_damage(res[DBRanker::DAMAGE].numberInt());
	respond.set_reduction(res[DBRanker::REDUCTION].numberInt());
	respond.set_pk(res[DBRanker::KILL_VALUE].numberInt());
	respond.set_glamour(res[DBRanker::GLAMOUR].numberInt());

	respond.set_fashion_color(res[DBRanker::FASHION_WEAPON].numberInt());
	respond.set_fashion_id((res[DBRanker::FASHION_CLOTHES].numberInt()));

	//角色装备
	if(res.hasField(DBRanker::EQUIP_LIST.c_str()))
	{
		ProtoEquipList *equip_list = respond.add_equip_list();
		equip_list->set_pack_type(GameEnum::INDEX_EQUIP);

		BSONObj obj = res.getObjectField(DBRanker::EQUIP_LIST.c_str());
		BSONObjIterator it(obj);
		while(it.more())
		{
			BSONObj obj_item = it.next().embeddedObject();

			PackageItem* item = POOL_MONITOR->pack_item_pool()->pop();
			JUDGE_CONTINUE(item != NULL);

			GameCommon::bson_to_item(item, obj_item);
			ProtoItem* proto_item = equip_list->add_pack_item_list();
			item->serialize(proto_item);

			POOL_MONITOR->pack_item_pool()->push(item);
		}

	}

	//战骑装备
	if(res.hasField(DBRanker::EQUIP_SET.c_str()))
	{
		BSONObjIterator iter_equip(res.getObjectField(DBRanker::EQUIP_SET.c_str()));
		while(iter_equip.more())
		{
			BSONObj obj_equip = iter_equip.next().embeddedObject();

			ProtoEquipList *equip_list = respond.add_equip_list();
			equip_list->set_pack_type(obj_equip[Package::Pack::PACK_TYPE].numberInt());

			BSONObjIterator item_iter(obj_equip.getObjectField(Package::Pack::PACK_ITEM.c_str()));
			while (item_iter.more())
			{
				BSONObj item_obj = item_iter.next().embeddedObject();

				PackageItem* item = POOL_MONITOR->pack_item_pool()->pop();
				JUDGE_CONTINUE(item != NULL);

				GameCommon::bson_to_item(item, item_obj);
				ProtoItem* proto_item = equip_list->add_pack_item_list();
				item->serialize(proto_item);

				POOL_MONITOR->pack_item_pool()->push(item);
			}
		}
	}

	if(res.hasField(DBRanker::MOUNT_SET.c_str()))
	{
		BSONObj obj = res.getObjectField(DBRanker::MOUNT_SET.c_str());
		BSONObjIterator it(obj);
		while(it.more())
		{
			BSONObj obj_mount = it.next().embeddedObject();

			int open = obj_mount[DBRanker::OFF_MOUNT_OPEN].numberInt();
			JUDGE_CONTINUE(open == true);

			ProtoMountInfo *mount_info = respond.add_mount_info();
			mount_info->set_type(obj_mount[DBRanker::OFF_MOUNT_TYPE].numberInt());
			mount_info->set_mount_grade(obj_mount[DBRanker::OFF_MOUNT_GRADE].numberInt());
			mount_info->set_mount_shape(obj_mount[DBRanker::OFF_MOUNT_SHAPE].numberInt());
			mount_info->set_act_shape(obj_mount[DBRanker::OFF_ACT_SHAPE].numberInt());

			mount_info->set_force(obj_mount[DBRanker::OFF_MOUNT_FORCE].numberInt());

			FightProperty fight_prop;
			FightProperty temp_prop;
			DBCommon::bson_to_fight_property(fight_prop, obj_mount.getObjectField(DBRanker::OFF_MOUNT_PROP.c_str()));
			DBCommon::bson_to_fight_property(temp_prop, obj_mount.getObjectField(DBRanker::OFF_MOUNT_TEMP.c_str()));
			fight_prop.serialize(mount_info->mutable_prop());
			fight_prop.serialize(mount_info->mutable_temp());

			BSONObjIterator skill_it(obj_mount.getObjectField(DBRanker::OFF_MOUNT_SKILL.c_str()));
			while (skill_it.more())
			{
				BSONObj skill_obj = skill_it.next().embeddedObject();
				ProtoSkill* proto_skill = mount_info->add_skill();

				int id = skill_obj[Skill::SSkill::SKILL_ID].numberInt();
				int lvl = skill_obj[Skill::SSkill::LEVEL].numberInt();

				JUDGE_CONTINUE(id > 0);

				proto_skill->set_skill_id(id);
				proto_skill->set_skill_level(lvl);
			}
		}
	}
	else
	{
		return ERROR_ROLE_NOT_EXISTS;
	}

//	MSG_DEBUG(%s, respond.Utf8DebugString().c_str());
	return 0;
}

int LogicSocialer::socialer_detail_info(void)
{
#ifdef LOCAL_DEBUG
	std::stringstream ss;
	ss << "\n******* - close friend list -******\n";
	LongMap::iterator it = this->socialer_detail_.__friend_list.begin();
	ss << "\n close friend size: " << "\t" << this->socialer_detail_.__friend_list.size() << std::endl;
	for(; it != this->socialer_detail_.__friend_list.end(); ++it)
	{
		ss << it->first << "\t";
	}

	ss << "\n******* - strang friend list -******\n";
	ss << "\n stranger size: " << "\t" << this->socialer_detail_.__stranger_list.size() << std::endl;
	LongMap::iterator iter = this->socialer_detail_.__stranger_list.begin();
	for(; iter != this->socialer_detail_.__stranger_list.end(); ++iter)
	{
		ss << iter->first << "\t";
	}

	ss << "\n******* - black list -******\n";
	ss << "\n black size: " << "\t" << this->socialer_detail_.__black_list.size() << std::endl;
	LongMap::iterator iter2 = this->socialer_detail_.__black_list.begin();
	for(; iter2 != this->socialer_detail_.__black_list.end(); ++iter2)
	{
		ss << iter2->first;
	}

	ss << "\n******* - enemy list -******\n";
	ss << "\n enemy size: " << "\t" << this->socialer_detail_.__enemy_list.size() << std::endl;
	LongMap::iterator iter3 = this->socialer_detail_.__enemy_list.begin();
	for(; iter3 != this->socialer_detail_.__enemy_list.end(); ++iter3)
	{
		ss << iter3->first;
	}

	ss << "\n******* - nearby list -******\n";
	ss << "\n nearby size: " << "\t" << this->socialer_detail_.__nearby_list.size() << std::endl;
	LongMap::iterator iter4 = this->socialer_detail_.__nearby_list.begin();
	for(; iter4 != this->socialer_detail_.__nearby_list.end(); ++iter4)
	{
		ss << iter4->first;
	}
	ss << std::endl;
	MSG_DEBUG("%s\n", ss.str().c_str());
#endif
	return 0;
}

int LogicSocialer::request_load_other_master(Int64 role_id, const int query_type)
{
	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	data_map->other_value_ = query_type;

	data_map->push_find(DBOfflineData::COLLECTION, BSON(DBOfflineData::ID << role_id));
	TRANSACTION_MONITOR->request_mongo_transaction(this->logic_player()->role_id(),
			TRANS_LOAD_MASTER_OFFLINE, data_map, LOGIC_MONITOR->logic_unit());
	return 0;
}

int LogicSocialer::after_fetch_other_master(Transaction* trans)
{
//	JUDGE_RETURN(trans != NULL, -1);
//	if(trans->detail().__error != 0)
//	{
//		trans->rollback();
//		return this->respond_to_client_error(RETURN_OTHER_MASTER_INFO, trans->detail().__error);
//	}
//
//	MongoDataMap* data_map = trans->fetch_mongo_data_map();
//	if(data_map == 0)
//	{
//		trans->rollback();
//		return 0;
//	}
//
//	MongoData* mongo_data = NULL;
//	if(data_map->find_data(DBOfflineData::COLLECTION, mongo_data) == 0)
//	{
//		BSONObj obj = mongo_data->data_bson();
//		Int64 other_role_id = obj[DBOfflineData::ID].numberLong();
//		PlayerOfflineData* offline_data = RANK_SYS->check_and_pop_player_offline(other_role_id);
//		if (offline_data != NULL)
//		{
//			offline_data->mount_beast_info_ = obj[DBOfflineData::BEAST_MOUNT_INFO].str();
//			offline_data->mount_beast_tick_ = ::time(NULL);
//
//			Proto50100031 master_info;
//			master_info.ParseFromString(offline_data->mount_beast_info_);
//			master_info.set_query_type(data_map->other_value_);
//			this->respond_to_client(RETURN_OTHER_MASTER_INFO, &master_info);
//		}
//	}
//	trans->summit();
	return 0;
}

int LogicSocialer::friend_sign_in()
{
	LongVec apply_set = LOGIC_FRIEND_SYSTEM->get_friend_pair_set(this->role_id(), FriendPairInfo::FRIEND_ADD);
	for (LongVec::iterator iter = apply_set.begin(); iter != apply_set.end(); ++iter)
	{
		this->insert_to_list(*iter, FRIEND_TYPE_CLOSE);
	}

	LongVec delete_set = LOGIC_FRIEND_SYSTEM->get_friend_pair_set(this->role_id(), FriendPairInfo::FRIEND_DELETE);
	for (LongVec::iterator iter = delete_set.begin(); iter != delete_set.end(); ++iter)
	{
		this->delete_from_list(*iter, FRIEND_TYPE_CLOSE);

		this->logic_player()->set_intimacy(*iter, 0);
	}

	return 0;
}

int LogicSocialer::add_friend_to_apply_list(Int64 role_id)
{
	LogicPlayer *player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, ERROR_PLAYER_OFFLINE);
	JUDGE_RETURN(this->socialer_detail_.__friend_list.size() < MAX_CLOSE_FRIEND_NUM,
			ERROR_CLOSE_FRIEND_NUM_LIMIT);

	LogicSocialerDetail::ApplyInfo &apply_info = this->socialer_detail_.__apply_map[role_id];
	apply_info.friend_id_ = role_id;
	apply_info.friend_name_ = player->name();
	apply_info.league_id_ = player->league_index();
	apply_info.level_ = player->role_detail().__level;
	apply_info.sex_ = player->role_detail().__sex;
	apply_info.tick_ = ::time(NULL);

	League *league = player->league();
	if (league != NULL)
		apply_info.league_name_ = league->league_name_;

	return 0;
}

int LogicSocialer::remove_apply_player(Int64 role_id)
{
	JUDGE_RETURN(this->socialer_detail_.__apply_map.count(role_id) > 0, -1);

	this->socialer_detail_.__apply_map.erase(role_id);
	return 0;
}

int LogicSocialer::is_friend(Int64 role_id)
{
	JUDGE_RETURN(this->socialer_detail_.__friend_list.count(role_id) > 0, false);

	return true;
}


