/*
 * LogicRankPlayer.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: louis
 */

#include "LogicRankPlayer.h"
#include "RankStruct.h"
#include "RankSystem.h"
#include "ProtoDefine.h"
#include "GameField.h"
#include "LeagueSystem.h"
#include "Transaction.h"
#include "MapStruct.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "TransactionMonitor.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "PoolMonitor.h"
#include <mongo/client/dbclient.h>

#include "GameFont.h"
#include "LogicGameSwitcherSys.h"

LogicRankPlayer::LogicRankPlayer() 
{
    this->last_water_flag_ = 0;
}

LogicRankPlayer::~LogicRankPlayer() 
{ /*NULL*/ }

int LogicRankPlayer::fetch_rank_data(Message* msg)
{
	// 后台开关检查
//	CONDITION_NOTIFY_RETURN(
//			LOGIC_SWITCHER_SYS->logic_check_switcher(GameSwitcherName::rank),
//			RETURN_REQUEST_FETCH_RANK_DATA, ERROR_MODEL_CLOSED);
	MSG_DYNAMIC_CAST_RETURN(Proto10100701*, request, -1);
	int rank_type = request->rank_type();
	int data_type = request->data_type();

	if(rank_type == RANK_LEAGUE)
	{
		return this->fetch_league_rank_data(data_type);
	}
	else if (rank_type == RANK_COMBINE_CONSUM_RANK)
	{
		return this->logic_player()->fetch_combine_activity_consum_rank(data_type);
	}

    Proto31402302 inner_res;
    inner_res.set_rank_type(rank_type);
    inner_res.set_data_type(data_type);

    if (rank_type == RANK_FIGHT_FORCE)
    {
        inner_res.set_rank_value(this->role_detail().__fight_force);
    }
    else if (rank_type == RANK_KILL_VALUE)
    {
    	inner_res.set_rank_value(this->role_detail().kill_normal_);
    }
    else if (rank_type == RANK_HERO)
    {
    	inner_res.set_rank_value(this->role_detail().kill_evil_);
    }
    else if (rank_type == RANK_FIGHT_LEVEL)
    {
        inner_res.set_rank_value(this->role_detail().__level);
    }
    else if (rank_type == RANK_SEND_FLOWER)
    {
    	inner_res.set_rank_value(this->logic_player()->total_send_flower());
    }
    else if (rank_type == RANK_RECV_FLOWER)
    {
    	inner_res.set_rank_value(this->logic_player()->total_recv_flower());
    }
    else if (rank_type == RANK_PET || rank_type == RANK_MOUNT  ||
    		 rank_type == RANK_FUN_MOUNT || rank_type == RANK_FUN_GOD_SOLIDER ||
    		 rank_type == RANK_FUN_MAGIC_EQUIP || rank_type == RANK_FUN_XIAN_WING ||
    		 rank_type == RANK_FUN_LING_BEAST || rank_type == RANK_FUN_BEAST_EQUIP ||
    		 rank_type == RANK_FUN_BEAST_MOUNT || rank_type == RANK_FUN_BEAST_WING ||
    		 rank_type == RANK_FUN_BEAST_MAO || rank_type == RANK_FUN_TIAN_GANG)
    {
        return this->monitor()->dispatch_to_scene(this, &inner_res);
    }
    else if (rank_type == RANK_SINGLE_SCRIPT_ZYFM)
    {
        Proto30401601 inner_req;
        inner_req.set_rank_type(rank_type);
        inner_req.set_data_type(data_type);
        return this->monitor()->dispatch_to_scene(this, &inner_req);
    }

    return this->process_fetch_rank_data_after_fetch_self_data(&inner_res);
}

int LogicRankPlayer::process_fetch_rank_data_after_fetch_self_data(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31402302 *, request, -1);
	int rank_type = request->rank_type();
	int data_type = request->data_type();

    Proto50100701 respond;
	respond.set_data_type(data_type);
	respond.set_rank_type(rank_type);

	RankShowPannel& rank_pannel = RANK_SYS->rank_show_pannel();

	if(rank_pannel.count(rank_type) <= 0)
	{
		FINER_PROCESS_RETURN(RETURN_REQUEST_FETCH_RANK_DATA, &respond);
	}

	RankRecordVec& rank_vec = rank_pannel[rank_type];

	const Json::Value& rank_json = CONFIG_INSTANCE->rank_json();
	JUDGE_RETURN(rank_json != Json::Value::null, -1);

	int msg_num = 0;

	if(data_type == 0)//refresh cache rank data
	{
		const Json::Value &pannel_json = CONFIG_INSTANCE->rank_pannel(rank_type);
		if (pannel_json != Json::Value::null && pannel_json.isMember("send_ranker_data_amount"))
		{
			msg_num = pannel_json["send_ranker_data_amount"][0u].asInt();
		}
		else
		{
			msg_num = rank_json["send_ranker_data_amount"][0u].asInt();
		}
		if(msg_num == 0)
			msg_num = GameEnum::RANK_DATA_SEND_FIRST;
		msg_num = msg_num > (int)rank_vec.size() ? (int)rank_vec.size() : msg_num;

		this->last_water_flag_ = 0;
	}
	else if(data_type == 1)//send data by water flag
	{
		const Json::Value &pannel_json = CONFIG_INSTANCE->rank_pannel(rank_type);
		if (pannel_json != Json::Value::null && pannel_json.isMember("send_ranker_data_amount"))
		{
			msg_num = pannel_json["send_ranker_data_amount"][0u].asInt();
		}
		else
		{
			msg_num = rank_json["send_ranker_data_amount"][0u].asInt();
		}
		if(msg_num == 0)
			msg_num = GameEnum::RANK_DATA_SEND_SECOND;;
		int left_msg_num = MAX(rank_vec.size() - this->last_water_flag_, 0);
		msg_num = MIN(msg_num, left_msg_num);
	}

	for(int i = 0; i < msg_num; ++i)
	{
		if (this->last_water_flag_ + i >= int(rank_vec.size()))
			continue;

		RankRecord* &cur_record = rank_vec[this->last_water_flag_ + i];
	    if(cur_record->__single_player_info.__rank_value == 0)
	    	break;
		ProtoRankRecord* proto_record = respond.add_rank_record_list();
		cur_record->serialize(proto_record);
	}

	if(msg_num == 0)
		FINER_PROCESS_RETURN(RETURN_REQUEST_FETCH_RANK_DATA, &respond);

	this->last_water_flag_ += msg_num;

	if(RANK_SYS->is_player_on_rank(rank_type, this->role_id()))
	{
		RankRecord* my_record = RANK_SYS->fetch_rank_record(rank_type, this->role_id());
		if(NULL != my_record && my_record->__single_player_info.__rank_value != 0)
		{
			ProtoRankRecord* proto_record = respond.mutable_my_rank_record();
			my_record->serialize(proto_record);
		}
	}
    else
    {
        ProtoRankRecord* proto_record = respond.mutable_my_rank_record();
        proto_record->set_role_id(this->role_id());
        proto_record->set_rank_value(request->rank_value());
        proto_record->set_cur_rank(RANK_SYS->fetch_player_rank(rank_type, this->role_id()));
        proto_record->set_display_content(this->role_detail().__name);
    }

	FINER_PROCESS_RETURN(RETURN_REQUEST_FETCH_RANK_DATA, &respond);
}

int LogicRankPlayer::fetch_league_rank_data(int data_type)
{
	Proto50100701 respond;
	respond.set_data_type(data_type);
	respond.set_rank_type(RANK_LEAGUE);

	const Json::Value& rank_json = CONFIG_INSTANCE->rank_json();
	JUDGE_RETURN(rank_json != Json::Value::null, -1);

	int data_num = 0;

	if(data_type == 0)//refresh cache rank data
	{
		this->last_water_flag_ = 0;
		data_num = rank_json["send_ranker_data_amount"][0u].asInt();
		if(data_num == 0)
			data_num = GameEnum::RANK_DATA_SEND_FIRST;
	}
	else if(data_type == 1)//send data by water flag
	{
		data_num = rank_json["send_ranker_data_amount"][1u].asInt();
		if(data_num == 0)
			data_num = GameEnum::RANK_DATA_SEND_SECOND;
	}

	LongVec league_set;
	LEAGUE_SYSTEM->fetch_league_set(false, league_set);

	int rank_index = this->last_water_flag_ + 1;
	int add_count = 0;
	for (LongVec::iterator iter = league_set.begin() + this->last_water_flag_;
			iter != league_set.end(); ++iter)
	{
		League* league = this->logic_player()->find_league(*iter);
		JUDGE_CONTINUE(league != NULL);

		ProtoRankRecord* proto_record = respond.add_rank_record_list();
		proto_record->set_cur_rank(rank_index);
		proto_record->set_last_rank(rank_index);
		proto_record->set_role_id(league->leader_index_);
		proto_record->set_display_content(league->league_name_);
		proto_record->set_rank_value(league->league_lvl_);
		proto_record->set_additional_id(league->league_force_);

		rank_index += 1;
		add_count += 1;

		JUDGE_BREAK(add_count < data_num);
	}

	if(respond.rank_record_list_size() != 0)
		this->last_water_flag_ += respond.rank_record_list_size();
	else
		FINER_PROCESS_RETURN(RETURN_REQUEST_FETCH_RANK_DATA, &respond);

	rank_index = 1;
	for(LongVec::iterator iter = league_set.begin(); iter != league_set.end(); ++iter, ++rank_index)
	{
		League* league = this->logic_player()->find_league(*iter);
		League* self_league = this->logic_player()->league();

		JUDGE_BREAK(0 != self_league && 0 != league);
		JUDGE_BREAK(rank_index <= GameEnum::RANK_RECORD_DEFAULT_LIMIT_NUM);
		JUDGE_CONTINUE(league->league_index_ == self_league->league_index_);
		ProtoRankRecord* proto_self_record = respond.mutable_my_rank_record();

		proto_self_record->set_cur_rank(rank_index);
		proto_self_record->set_last_rank(rank_index);
		proto_self_record->set_role_id(league->leader_index_);
		proto_self_record->set_display_content(league->league_name_);
		proto_self_record->set_rank_value(league->league_lvl_);
		proto_self_record->set_additional_id(league->league_force_);

		break;
	}
    if (rank_index > int(league_set.size()))
    {
        League *self_league = this->logic_player()->league();
        if (NULL != self_league)
        {
            ProtoRankRecord* proto_self_record = respond.mutable_my_rank_record(); 
            proto_self_record->set_role_id(self_league->leader_index_);
            proto_self_record->set_display_content(self_league->league_name_);
            proto_self_record->set_rank_value(self_league->league_lvl_);
            proto_self_record->set_additional_id(self_league->league_force_);
        }
    }

	FINER_PROCESS_RETURN(RETURN_REQUEST_FETCH_RANK_DATA, &respond);
}

int LogicRankPlayer::fetch_rank_data_ex(Message* msg)
{
	return 0;
}

int LogicRankPlayer::fetch_ranker_detail_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100702*, request, -1);
	Int64 player_id = request->role_id();
	int rank_type = request->rank_type();

//	LogicPlayer *player = this->find_player(request->role_id());
//	if (player != 0)
//	{
//		Proto50100702 respond;
//		respond.set_req_role_id(this->role_id());
//		respond.set_role_id(request->role_id());
//		respond.set_rank_type(request->rank_type());
//        respond.set_career(player->role_detail().__career);
//        respond.set_sex(player->role_detail().__sex);

//        Int64 result = 0;
//		  this->decode_worship_num(result, rank_type, player_id);
//        respond.set_is_worship(this->role_detail().is_worship_.count(result));

//		return player->monitor()->dispatch_to_scene(player, &respond);
//	}

//	RankRecord* my_record = RANK_SYS->fetch_rank_record(rank_type, player_id);
//	CONDITION_NOTIFY_RETURN(my_record != NULL, RETURN_REQUEST_FETCH_RANKER_DETAIL, ERROR_SERVER_INNER);

	int recogn = 0;
	switch(rank_type)
	{
	case RANK_PET:
		recogn = TRANS_LOAD_RANK_PET_INFO;
		break;
	case RANK_MOUNT:
		recogn = TRANS_LOAD_RANK_MOUNT_INFO;
		break;

		/*
	case RANK_FUN_MOUNT:
		recogn = TRANS_LOAD_PLAYER_FUN_MOUNT_INFO;
		break;
	case RANK_FUN_GOD_SOLIDER:
		recogn = TRANS_LOAD_PLAYER_FUN_GOD_SOLIDER_INFO;
		break;
	case RANK_FUN_MAGIC_EQUIP:
		recogn = TRANS_LOAD_PLAYER_FUN_MAGIC_EQUIP_INFO;
		break;
	case RANK_FUN_XIAN_WING:
		recogn = TRANS_LOAD_PLAYER_FUN_XIAN_WING_INFO;
		break;
	case RANK_FUN_LING_BEAST:
		recogn = TRANS_LOAD_PLAYER_FUN_LING_BEAST_INFO;
		break;
	case RANK_FUN_BEAST_EQUIP:
		recogn = TRANS_LOAD_PLAYER_FUN_BEAST_EQUIP_INFO;
		break;
	case RANK_FUN_BEAST_MOUNT:
		recogn = TRANS_LOAD_PLAYER_FUN_BEAST_MOUNT_INFO;
		break;
		*/
	default:
		recogn = TRANS_LOAD_RANKER_INFO;
		break;
	}

	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(data_map != NULL, -1);

	data_map->other_value_ = rank_type;
	data_map->push_find(DBRanker::COLLECTION, BSON(DBRanker::ID << player_id));
	TRANSACTION_MONITOR->request_mongo_transaction(this->logic_player()->role_id(),
			recogn, data_map, LOGIC_MONITOR->logic_unit());
	return 0;
}

int LogicRankPlayer::after_fetch_ranker_detail(Transaction* transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);
	if (transaction->detail().__error != 0)
	{
		transaction->rollback();
		return transaction->detail().__error;
	}

	MongoDataMap* data_map = transaction->fetch_mongo_data_map();
	if (data_map == 0)
	{
		transaction->rollback();
		return -1;
	}

	int rank_type = 0;
	int recogn = transaction->res_recogn();
	switch(recogn)
	{
	case TRANS_LOAD_RANK_PET_INFO:
		rank_type = RANK_PET;
		break;

	case TRANS_LOAD_RANK_MOUNT_INFO:
		rank_type = RANK_MOUNT;
		break;
	case TRANS_LOAD_KILL_VALUE_RANK_DATA:
		rank_type = RANK_KILL_VALUE;
		break;
	case TRANS_LOAD_HERO_RANK_DATA:
		rank_type = RANK_HERO;
		break;
/*
	case TRANS_LOAD_PLAYER_FUN_MOUNT_INFO:
		rank_type = RANK_FUN_MOUNT;
		break;
	case TRANS_LOAD_PLAYER_FUN_GOD_SOLIDER_INFO:
		rank_type = RANK_FUN_GOD_SOLIDER;
		break;
	case TRANS_LOAD_PLAYER_FUN_MAGIC_EQUIP_INFO:
		rank_type = RANK_FUN_MAGIC_EQUIP;
		break;
	case TRANS_LOAD_PLAYER_FUN_XIAN_WING_INFO:
		rank_type = RANK_FUN_XIAN_WING;
		break;
	case TRANS_LOAD_PLAYER_FUN_LING_BEAST_INFO:
		rank_type = RANK_FUN_LING_BEAST;
		break;
	case TRANS_LOAD_PLAYER_FUN_BEAST_EQUIP_INFO:
		rank_type = RANK_FUN_BEAST_EQUIP;
		break;
	case TRANS_LOAD_PLAYER_FUN_BEAST_MOUNT_INFO:
		rank_type = RANK_FUN_BEAST_MOUNT;
		break;
*/
	default:
		rank_type = data_map->other_value_;
		break;
	}
	MongoData* mongo_data = NULL;
	if(data_map->find_data(DBRanker::COLLECTION, mongo_data) == 0)
	{
		BSONObj res = mongo_data->data_bson();

		Proto50100702 respond;

		for (int i = GameEnum::FUN_MOUNT; i < GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
		{
			ProtoPairObj* temp = respond.add_mount_info();
			temp->set_obj_id(i);
			temp->set_obj_value(0);
			switch(i)
			{
			case GameEnum::FUN_MOUNT :
				temp->set_obj_value(res[DBRanker::FUN_MOUNT_GRADE].numberInt());
				break;
			case GameEnum::FUN_GOD_SOLIDER :
				temp->set_obj_value(res[DBRanker::FUN_GOD_SOLIDER_GRADE].numberInt());
				break;
			case GameEnum::FUN_MAGIC_EQUIP :
				temp->set_obj_value(res[DBRanker::FUN_MAGIC_EQUIP_GRADE].numberInt());
				break;
			case GameEnum::FUN_XIAN_WING :
				temp->set_obj_value(res[DBRanker::FUN_XIAN_WING_GRADE].numberInt());
				break;
			case GameEnum::FUN_LING_BEAST :
				temp->set_obj_value(res[DBRanker::FUN_LING_BEAST_GRADE].numberInt());
				break;
			case GameEnum::FUN_BEAST_EQUIP :
				temp->set_obj_value(res[DBRanker::FUN_BEAST_EQUIP_GRADE].numberInt());
				break;
			case GameEnum::FUN_BEAST_MOUNT :
				temp->set_obj_value(res[DBRanker::FUN_BEAST_MOUNT_GRADE].numberInt());
				break;
			case GameEnum::FUN_BEAST_WING :
				temp->set_obj_value(res[DBRanker::FUN_BEAST_WING_GRADE].numberInt());
				break;
			case GameEnum::FUN_BEAST_MAO :
				temp->set_obj_value(res[DBRanker::FUN_BEAST_MAO_GRADE].numberInt());
				break;
			case GameEnum::FUN_TIAN_GANG :
				temp->set_obj_value(res[DBRanker::FUN_TIAN_GANG_GRADE].numberInt());
				break;
			}

		}
		respond.set_rank_type(rank_type);
		respond.set_pet_id(res[DBRanker::Pet::PET_INDEX].numberLong());
		respond.set_role_id(res[DBRanker::ID].numberLong());
		respond.set_pet_sort(res[DBRanker::Pet::PET_SORT].numberInt());
		respond.set_pet_cur_sort(res[DBRanker::Pet::PET_CUR_SORT].numberInt());
		respond.set_mounter(res[DBRanker::MOUNT_SHAPE].numberInt());
		respond.set_label(res[DBRanker::LABEL].numberInt());
		respond.set_clothes(res[DBRanker::CLOTHES].numberInt());
		respond.set_fashion_clothes(res[DBRanker::FASHION_CLOTHES].numberInt());
		respond.set_weapon(res[DBRanker::WEAPON].numberInt());
		respond.set_fashion_weapon(res[DBRanker::FASHION_WEAPON].numberInt());
		respond.set_career(res[DBRanker::CAREER].numberInt());
		respond.set_sex(res[DBRanker::SEX].numberInt());

        Int64 result = 0;
        Int64 player_id = res[DBRanker::ID].numberLong();
        this->decode_worship_num(result, rank_type, player_id);
        respond.set_is_worship(this->role_detail().is_worship_.count(result));

	    switch (rank_type)
	    {
	    	case RANK_FIGHT_FORCE:
	    	case RANK_FIGHT_LEVEL:
	    		respond.set_force(res[DBRanker::FIGHT_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_MOUNT:
	    		respond.set_force(res[DBRanker::FUN_MOUNT_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_GOD_SOLIDER:
	    		respond.set_force(res[DBRanker::FUN_GOD_SOLIDER_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_MAGIC_EQUIP:
	    		respond.set_force(res[DBRanker::FUN_MAGIC_EQUIP_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_XIAN_WING:
	    		respond.set_force(res[DBRanker::FUN_XIAN_WING_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_LING_BEAST:
	    		respond.set_force(res[DBRanker::FUN_LING_BEAST_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_BEAST_EQUIP:
	    		respond.set_force(res[DBRanker::FUN_BEAST_EQUIP_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_BEAST_MOUNT:
	    		respond.set_force(res[DBRanker::FUN_BEAST_MOUNT_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_BEAST_WING:
	    		respond.set_force(res[DBRanker::FUN_BEAST_WING_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_BEAST_MAO:
	    		respond.set_force(res[DBRanker::FUN_BEAST_MAO_FORCE].numberLong());
	    		break;
	    	case RANK_FUN_TIAN_GANG:
	    		respond.set_force(res[DBRanker::FUN_TIAN_GANG_FORCE].numberLong());
	    		break;

	    }

	    //战力不要用最新的
	    if (rank_type == RANK_FIGHT_FORCE || rank_type == RANK_FIGHT_LEVEL)
	    {
	    	Int64 rank_player_id = respond.role_id();
			RankRecord* rank_player = RANK_SYS->fetch_rank_record(RANK_FIGHT_FORCE, rank_player_id);
			if (rank_player != NULL)
			{
				respond.set_force(rank_player->__single_player_info.__rank_value);
			}
	    }

		int wing_level = res[DBRanker::WING_LEVEL].numberInt();
		respond.set_top_wing_id(wing_level / 1000);
		respond.set_top_wing_level(wing_level % 1000);

		if(res.hasField(DBRanker::EQUIP_LIST.c_str()))
		{
			BSONObj obj = res.getObjectField(DBRanker::EQUIP_LIST.c_str());
			BSONObjIterator it(obj);
			IntVec refine_lvl;
			int weapon_refine_lvl = 0;
			while(it.more())
			{
				BSONObj obj_item = it.next().embeddedObject();
				int lvl = obj_item[Package::PackEquip::REFINE_LEVEL].numberInt();
				if(obj_item[Package::PackItem::INDEX].numberInt() == GameEnum::EQUIP_WEAPON)
				{
					weapon_refine_lvl = lvl;
				}
				refine_lvl.push_back(lvl);
			}
			respond.set_weapon_refine_lvl(weapon_refine_lvl);
			respond.set_equip_refine_view_index(0);
		}
		if(res.hasField(DBRanker::FAIRY_ACT.c_str()))
		{
			BSONObj obj = res.getObjectField(DBRanker::FAIRY_ACT.c_str());
			ProtoPairObj *pair = respond.mutable_fairy_act();
			pair->set_obj_id(obj[DBRanker::Fairy_act::ID].numberInt());
			pair->set_obj_value(obj[DBRanker::Fairy_act::SELECT_ICON].numberInt());
		}
	    this->respond_to_client(RETURN_REQUEST_FETCH_RANKER_DETAIL, &respond);
//	    MSG_USER(-@@@ -%s- @@-, respond.Utf8DebugString().c_str());
	}

	transaction->summit();
	return 0;
}

int LogicRankPlayer::reset()
{
	this->last_water_flag_ = 0;
	return 0;
}

/*
 * 此接口仅在玩家在宠物排行榜查询上榜宠物信息时调用;
 *
 * mmo.player_rank_detail: 这张表专门存储上榜宠物信息；起缓存数据的作用；
 * 防止出现以下情况：玩家丢弃上榜宠物但是排行榜还没更新，导致玩家不能查询该宠物信息
 * */
int LogicRankPlayer::request_fetch_rank_beast_info(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto10100031*, request, -1);
//	Int64 player_id = request->role_id();
//	Int64 pet_id = request->beast_id();
//	RankPannel& rank_pannel = RANK_SYS->rank_pannel();
//
//	int rank_type = RANK_PET;
//
//	JUDGE_RETURN(rank_pannel.count(rank_type) > 0, -1);
//	RankRecordMap& rank_map = rank_pannel[rank_type];
//
//	RankRecordMap::iterator it = rank_map.find(player_id);
//	if(it != rank_map.end()) // found
//	{
//		RankRecord* &record = it->second;
//		JUDGE_RETURN(record != NULL, -1);
//
//		MSG_DEBUG(pet_id:(online && on_rank)%ld-%ld,
//				record->__single_player_info.__additional_id, pet_id);
//		MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
//		JUDGE_RETURN(data_map != NULL, -1);
//
//		data_map->push_find(DBPlayerRankDetail::COLLECTION, BSON(DBPlayerRankDetail::ID << player_id));
//		if(TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), TRANS_LOAD_RANKER_PET_MOUNT_INFO,
//				data_map, LOGIC_MONITOR->logic_unit()) != 0)
//		{
//			POOL_MONITOR->mongo_data_map_pool()->push(data_map);
//			return -1;
//		}
//	}
//	else
//	{
//		MSG_USER(pet_id:(online && not_on_rank)%ld, pet_id);
//	}

	return 0;
}

int LogicRankPlayer::request_worship_rank(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100705*, request, -1);

	LogicPlayer* player = this->find_player(request->player_id());
	JUDGE_RETURN(player != NULL, 0);

	Int64 result = 0;
	Int64 role_id = request->role_id();

	int type = request->rank_type();
	this->decode_worship_num(result, type, role_id);

	int worship_num = this->role_detail().is_worship_.count(result);
	CONDITION_NOTIFY_RETURN(worship_num == 0, RETURN_REQUEST_WORSHIP_RANK, ERROR_CLIENT_OPERATE);

	int rank_num = request->rank_num();
	int rank_type = request->rank_type();

	RankRecord* rank_player = RANK_SYS->fetch_rank_record(rank_type, rank_num - 1);
	JUDGE_RETURN(rank_player != NULL, 0);

	rank_player->__single_player_info.__worship_num ++;
	player->role_detail().is_worship_.insert(result);
	RANK_SYS->request_save_rank_data(rank_type);

	Proto50100705 respond;
	respond.set_status(1);
	respond.set_worship_num(rank_player->__single_player_info.__worship_num);
	FINER_PROCESS_RETURN(RETURN_REQUEST_WORSHIP_RANK, &respond);
}

int LogicRankPlayer::respond_fetch_rank_beast_info(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if (trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	MongoDataMap* data_map = trans->fetch_mongo_data_map();
	if (data_map == 0)
	{
		trans->rollback();
		return -1;
	}

	trans->summit();
	return 0;
}

void LogicRankPlayer::decode_worship_num(Int64 &result, int &type, Int64 &role_id)
{
	Int64 temp = result;

	if (temp == 0) //加密
	{
		Int64 temp_id = role_id;
		Int64 len = 1;
		while (temp_id > 0)
		{
			temp_id /= 10;
			len *= 10;
		}
		result = type * len * 100 + role_id;
	}
	else //解密
	{
		Int64 temp_id = 0;
		Int64 len = 1;
		while (temp != type && temp > 0)
		{
			temp_id += len * (temp % 10);
			len *= 10;
			temp /= 10;
		}
		role_id = temp_id;
	}
}
