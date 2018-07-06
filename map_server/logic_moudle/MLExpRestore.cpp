/*
 * MLExpRestore.cpp
 *
 *  Created on: 2014-2-17
 *      Author: lijin
 */

#include "MLExpRestore.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include <google/protobuf/message.h>

MLExpRestore::MLExpRestore()
{
	// TODO Auto-generated constructor stub
}

MLExpRestore::~MLExpRestore()
{
	// TODO Auto-generated destructor stub
}

void MLExpRestore::reset()
{
	exp_restore_detail_.reset();
}

int MLExpRestore::fetch_diff_day(const Time_Value &check_day_tick, const Time_Value &nowtime)
{
	Date_Time check_date(check_day_tick), now_date(nowtime);
	check_date.hour(0);
	check_date.minute(0);
	check_date.second(0);

	now_date.hour(0);
	now_date.minute(0);
	now_date.second(0);

	return std::abs(check_date.time_sec() - now_date.time_sec()) / Time_Value::DAY;
}

void MLExpRestore::reinit_restore_exp_map(void)
{
	//刷新昨日活动数据
	this->exp_restore_detail_.__pre_act_map.clear();

	for (int i = GameEnum::ES_ACT_TYPE_BEGIN; i <= GameEnum::ES_ACT_TYPE_END; ++i)
	{
		JUDGE_CONTINUE(this->exp_restore_detail_.__now_act_map[i].__times
				&& this->exp_restore_detail_.__now_act_map[i].__ext_type > 0);

		RestoreActInfo &temp = this->exp_restore_detail_.__pre_act_map[i];
		temp.__ext_type = this->exp_restore_detail_.__now_act_map[i].__ext_type;
		temp.__times = this->exp_restore_detail_.__now_act_map[i].__times;
	}
}

int MLExpRestore::check_and_update_exp_restore_info()
{
	Time_Value cur_time = Time_Value::gettimeofday();
	JUDGE_RETURN(this->exp_restore_detail().__check_timestamp < cur_time, -1);

//	this->cal_stamp_level_map(cur_time);
//	this->cal_stamp_vip_map(cur_time);
//	this->cal_restore_record(cur_time);

	int diff_day = this->fetch_diff_day(this->exp_restore_detail().__check_timestamp, cur_time);
	if (diff_day < 1)
	{
		this->reinit_restore_exp_map();
	}

	//重置今日活动数据
	for (int i = GameEnum::ES_ACT_TYPE_BEGIN; i <= GameEnum::ES_ACT_TYPE_END; ++i)
	{
		this->exp_restore_detail_.__now_act_map[i].__times = 1;
		switch(i)
		{
		case GameEnum::ES_ACT_DAILY_ROUTINE:
		case GameEnum::ES_ACT_LEAGUE_ROUTINE:
		case GameEnum::ES_ACT_OFFER_ROUTINE:
			this->exp_restore_detail_.__now_act_map[i].__times = 20;
			break;
		}
	}

	if (diff_day >= 1)
	{
		this->reinit_restore_exp_map();
	}

	this->exp_restore_detail().__check_timestamp = next_day(0,0, cur_time);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_EXP_RESTORE);
	return 0;
}

int MLExpRestore::sync_transfer_exp_restore(int scene_id)
{
	Proto31400115 restore_info;
	restore_info.set_update_tick(this->exp_restore_detail().__check_timestamp.sec());
	restore_info.set_vip_type_rec(this->exp_restore_detail().__vip_type_record);
	restore_info.set_vip_start_time(this->exp_restore_detail().__vip_start_time);
	restore_info.set_vip_expried_time(this->exp_restore_detail().__vip_expried_time);

	for (RestoreMap::iterator it = this->exp_restore_detail_.__pre_act_map.begin();
			it != this->exp_restore_detail_.__pre_act_map.end(); ++it)
	{
		ProtoThreeObj* temp = restore_info.add_pre_act_map();
		temp->set_id(it->first);
		temp->set_value(it->second.__ext_type);
		temp->set_total_times(it->second.__times);
	}

	for (RestoreMap::iterator it = this->exp_restore_detail_.__now_act_map.begin();
			it != this->exp_restore_detail_.__now_act_map.end(); ++it)
	{
		ProtoThreeObj* temp = restore_info.add_now_act_map();
		temp->set_id(it->first);
		temp->set_value(it->second.__ext_type);
		temp->set_total_times(it->second.__times);
	}

	for (LongMap::iterator iter = this->stamp_level_map().begin();
			iter != this->stamp_level_map().end(); ++iter)
	{
		ProtoPairObj *obj = restore_info.add_level_record();
		obj->set_obj_id(iter->first);
		obj->set_obj_value(iter->second);
	}

	for(LongMap::iterator iter = this->stamp_vip_map().begin();
			iter != this->stamp_vip_map().end(); ++iter)
	{
		ProtoPairObj *obj = restore_info.add_vip_record();
		obj->set_obj_id(iter->first);
		obj->set_obj_value(iter->second);
	}

	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		ProtoExpRestoreRecord *record = restore_info.add_exp_restore_record();

		record->set_id(iter->__storage_id);
		record->set_count(iter->__finish_count);
		record->set_valid(iter->__storage_valid);
		record->set_date(iter->__record_timestamp.sec());
	}

	StorageStageInfo &storage_stage_info = this->exp_restore_detail().__storage_stage_info;
	for(StorageStageInfo::iterator iter_i = storage_stage_info.begin();
			iter_i != storage_stage_info.end(); ++iter_i)
	{
		ProtoERScriptStageInfo* script_stage_info = restore_info.add_script_stage_info();
		script_stage_info->set_script_sort(iter_i->first);

		TimestampStageMap &timestamp_stage_map = iter_i->second;
		for(TimestampStageMap::iterator iter_n = timestamp_stage_map.begin();
				iter_n != timestamp_stage_map.end(); ++iter_n)
		{
			ProtoERTimeStage* script_time_stage = script_stage_info->add_timestageset();
			script_time_stage->set_time_sec(iter_n->first);
			script_time_stage->set_stage(iter_n->second);
		}
	}

	//MSG_USER(%s, restore_info.Utf8DebugString().c_str());
	return this->send_to_other_logic_thread(scene_id, restore_info);
}

int MLExpRestore::read_transfer_exp_restore(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400115*, request, -1);

	MLExpRestore::reset();
	this->exp_restore_detail().__check_timestamp.set(request->update_tick(), 0);
	this->exp_restore_detail().__vip_type_record = request->vip_type_rec();
	this->exp_restore_detail().__vip_start_time = request->vip_start_time();
	this->exp_restore_detail().__vip_expried_time = request->vip_expried_time();

	for (int i = 0; i < request->pre_act_map_size(); ++i)
	{
		const ProtoThreeObj &obj = request->pre_act_map(i);
		this->exp_restore_detail_.__pre_act_map[obj.id()].__ext_type = obj.value();
		this->exp_restore_detail_.__pre_act_map[obj.id()].__times = obj.total_times();
	}

	for (int i = 0; i < request->now_act_map_size(); ++i)
	{
		const ProtoThreeObj &obj = request->now_act_map(i);
		this->exp_restore_detail_.__now_act_map[obj.id()].__ext_type = obj.value();
		this->exp_restore_detail_.__now_act_map[obj.id()].__times = obj.total_times();
	}

	for(int i = 0; i < request->level_record_size(); ++i)
	{
		const ProtoPairObj &obj = request->level_record(i);
		this->stamp_level_map()[obj.obj_id()] = obj.obj_value();
	}

	for(int i = 0; i< request->vip_record_size(); ++i)
	{
		const ProtoPairObj &obj = request->vip_record(i);
		this->stamp_vip_map()[obj.obj_id()] = obj.obj_value();
	}

	StorageRecord record;
	for(int i = 0; i < request->exp_restore_record_size(); ++i)
	{
		const ProtoExpRestoreRecord &proto_record = request->exp_restore_record(i);
		record.__storage_id = proto_record.id();
		record.__finish_count = proto_record.count();
		record.__storage_valid = proto_record.valid();
		record.__record_timestamp.set(proto_record.date(), 0);
		this->storage_record_set().push_back(record);
	}

	StorageStageInfo &storage_stage_info = this->exp_restore_detail().__storage_stage_info;
	storage_stage_info.clear();

	for(int i=0; i< request->script_stage_info_size(); ++i)
	{
		const ProtoERScriptStageInfo& script_stage_info = request->script_stage_info(i);
		TimestampStageMap &time_stage_map = storage_stage_info[script_stage_info.script_sort()];

		for(int n=0; n < script_stage_info.timestageset_size(); ++n)
		{
			const ProtoERTimeStage &time_stage = script_stage_info.timestageset(n);
			time_stage_map[time_stage.time_sec()] = time_stage.stage();
		}
	}

	return 0;
}

int MLExpRestore::insert_restore_good(int& exp, int& reputation, int& honour, int& exploit,
			int& league_contri, int& reiki, int& gold, int& bind_gold, RewardInfo& reward_info_total)
{
	MapLogicPlayer* player = dynamic_cast<MapLogicPlayer *>(this);
	int bind_staus = GameEnum::ITEM_NO_BIND;

	if (gold > 0)
	{
		ItemObj item(GameEnum::ITEM_MONEY_UNBIND_GOLD, gold, bind_staus);
		reward_info_total.add_rewards(item);
	}
	if (bind_gold > 0)
	{
		ItemObj item(GameEnum::ITEM_MONEY_BIND_GOLD, bind_gold, bind_staus);
		reward_info_total.add_rewards(item);
	}
	if (exp > 0)
	{
		ItemObj item(GameEnum::ITEM_ID_PLAYER_EXP, exp, bind_staus);
		reward_info_total.add_rewards(item);
	}
	if (honour > 0)
	{
		ItemObj item(GameEnum::ITEM_ID_HONOUR, honour, bind_staus);
		reward_info_total.add_rewards(item);
	}
	if (exploit > 0)
	{
		ItemObj item(GameEnum::ITEM_ID_EXPLOIT, exploit, bind_staus);
		reward_info_total.add_rewards(item);
	}
	if (league_contri > 0)
	{
		ItemObj item(GameEnum::ITEM_ID_LEAGUE_CONTRI, league_contri, bind_staus);
		reward_info_total.add_rewards(item);
	}
	if (reiki > 0)
	{
		ItemObj item(GameEnum::ITEM_ID_REIKI, reiki, bind_staus);
		reward_info_total.add_rewards(item);
	}

	player->insert_package(ADD_FROM_RESTORE, reward_info_total);
	return 0;
}

int MLExpRestore::check_restore_info(RewardInfo& reward_info_total, int act_type, int& exp, int& reputation, int& honour,
		int& exploit, int& league_contri, int& reiki, int& gold, int& bind_gold, IntVec& free_list,
		IntVec& money_list, int check_type, int get_type, int times)
{
	int act_id = act_type * GameEnum::ES_ACT_BASE_NUM + this->exp_restore_detail_.__pre_act_map[act_type].__ext_type;
	const Json::Value &restore_json = CONFIG_INSTANCE->restore_json_by_act_id(act_id);
	JUDGE_RETURN(restore_json != Json::Value::null, 1);
//	int total = times;
	if (restore_json["free_reward_id"].asInt() > 0)
	{
//		total = times;
//		while (total > 0)
//		{
			free_list.push_back(restore_json["free_reward_id"].asInt());
//			total--;
//		}
	}
	if (restore_json["money_reward_id"].asInt() > 0)
	{
//		total = times;
///		while (total > 0)
//		{
			money_list.push_back(restore_json["money_reward_id"].asInt());
//			total--;
//		}
	}

	exp += restore_json["exp"].asInt();
	reputation += restore_json["reputation"].asInt();
	honour += restore_json["honour"].asInt();
	exploit += restore_json["exploit"].asInt();
	league_contri += restore_json["league_contri"].asInt();
	reiki += restore_json["reiki"].asInt();
	gold += restore_json["gold"].asInt();
	bind_gold += restore_json["bind_gold"].asInt();

	if (restore_json["add_up"] == 1)
	{
		int size = (int)CONFIG_INSTANCE->restore_json().size();
		for (int i = 1; i <= size; ++i)
		{
			const Json::Value &res_json = CONFIG_INSTANCE->restore_json(i);
			JUDGE_CONTINUE(res_json["act_type"].asInt() ==  act_type &&
				res_json["act_ext_type"].asInt() <
				this->exp_restore_detail_.__pre_act_map[act_type].__ext_type);

			exp += res_json["exp"].asInt();
			reputation += res_json["reputation"].asInt();
			honour += res_json["honour"].asInt();
			exploit += res_json["exploit"].asInt();
			league_contri += res_json["league_contri"].asInt();
			reiki += res_json["reiki"].asInt();
			gold += res_json["gold"].asInt();
			bind_gold += res_json["bind_gold"].asInt();

			if (res_json["free_reward_id"].asInt() > 0)
			{
//				total = times;
//				while (total > 0)
//				{
					free_list.push_back(res_json["free_reward_id"].asInt());
//					total--;
//				}
			}

			if (res_json["money_reward_id"].asInt() > 0)
			{
//				total = times;
//				while (total > 0)
//				{
					money_list.push_back(res_json["money_reward_id"].asInt());
//					total--;
//				}
			}
		}
	}

	exp *= times; reputation *= times; honour *= times; exploit *= times;
	reiki *= times; gold *= times; bind_gold *= times;

	//环式任务额外奖励
    int ext_award = 0;
    int ext_exp = 0;

    switch(act_type)
    {
    	case GameEnum::ES_ACT_DAILY_ROUTINE:
    	{
    		ext_award = CONFIG_INSTANCE->role_level(0, this->role_level())["routine_ext_award"].asInt();
    		ext_exp = CONFIG_INSTANCE->role_level(0, this->role_level())["routine_ext_exp"].asInt();
    		break;
    	}
    	case GameEnum::ES_ACT_OFFER_ROUTINE:
    	{
    		ext_award = CONFIG_INSTANCE->role_level(0, this->role_level())["offer_routine_ext_award"].asInt();
    		ext_exp = CONFIG_INSTANCE->role_level(0, this->role_level())["offer_routine_ext_exp"].asInt();
    		break;
    	}
    	case GameEnum::ES_ACT_LEAGUE_ROUTINE:
    	{
    		ext_award = CONFIG_INSTANCE->role_level(0, this->role_level())["league_routine_ext_award"].asInt();
    		ext_exp = CONFIG_INSTANCE->role_level(0, this->role_level())["league_routine_ext_exp"].asInt();
    		break;
    	}
    }
    exp += ext_exp;
    if (ext_award > 0) money_list.push_back(ext_award);


	if (check_type)
	{
		if (get_type == 2)
		{
			Money money = GameCommon::make_up_money(restore_json["price"].asInt(), GameEnum::MONEY_UNBIND_GOLD);
			CONDITION_NOTIFY_RETURN(true == GameCommon::validate_money(money),
					RETURN_RESTORE_EXP_SINGLE, ERROR_SERVER_INNER);

			GameCommon::adjust_money(money, this->own_money());
			int ret = this->pack_money_sub(money, SUB_MONEY_RESTORE);
			CONDITION_NOTIFY_RETURN(0 == ret, RETURN_RESTORE_EXP_SINGLE, ERROR_PACKAGE_MONEY_AMOUNT);
		}

//		MapLogicPlayer* player = dynamic_cast<MapLogicPlayer *>(this);
//		int bind_staus = GameEnum::ITEM_NO_BIND;
		Int64 rate = 10000;
//		RewardInfo reward_info_total;

		/*
		//环式任务额外奖励元宝找回才可领取
		if (ext_award && get_type == 2)
		{
			player->add_reward(ext_award, ADD_FROM_RESTORE);
			IntVec::iterator it = money_list.begin();
			for (; it != money_list.end(); ++it)
			{
				if (*it == ext_award) it = money_list.erase(it);
				if (it == money_list.end()) break;
			}
		}
		*/

		if (get_type == 1)
		{
			rate = restore_json["free_rate"].asInt();
			for (uint i = 0; i < free_list.size(); ++i)
			{
				int id = free_list[i];
				const Json::Value& reward_json = CONFIG_INSTANCE->reward(id);
				JUDGE_CONTINUE(reward_json.empty() == false);

				RewardInfo reward_info;
				GameCommon::make_up_reward_items(reward_info, reward_json);
				reward_info_total.add_rewards(reward_info.item_vec_);
			}
		}
		else
		{
			for (uint i = 0; i < money_list.size(); ++i)
			{
				int id = money_list[i];
				const Json::Value& reward_json = CONFIG_INSTANCE->reward(id);
				JUDGE_CONTINUE(reward_json.empty() == false);

				RewardInfo reward_info;
				GameCommon::make_up_reward_items(reward_info, reward_json);
				reward_info_total.add_rewards(reward_info.item_vec_);
			}
		}
//		player->insert_package(ADD_FROM_RESTORE, reward_info_total);

		exp = int((double) exp * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		gold = int((double) gold * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		bind_gold = int((double) bind_gold * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		honour = int((double) honour * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		exploit = int((double) exploit * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		league_contri = int((double) league_contri * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		reiki = int((double) reiki * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);

		/*
		if (gold > 0) player->insert_package(ADD_FROM_RESTORE, GameEnum::ITEM_MONEY_UNBIND_GOLD, gold, bind_staus);
		if (bind_gold > 0) player->insert_package(ADD_FROM_RESTORE, GameEnum::ITEM_MONEY_BIND_GOLD, bind_gold, bind_staus);
		if (exp > 0) player->insert_package(ADD_FROM_RESTORE, GameEnum::ITEM_ID_PLAYER_EXP, exp, bind_staus);
		if (honour > 0) player->insert_package(ADD_FROM_RESTORE, GameEnum::ITEM_ID_HONOUR, honour, bind_staus);
		if (exploit > 0) player->insert_package(ADD_FROM_RESTORE, GameEnum::ITEM_ID_EXPLOIT, exploit, bind_staus);
		if (league_contri > 0) player->insert_package(ADD_FROM_RESTORE, GameEnum::ITEM_ID_LEAGUE_CONTRI, league_contri, bind_staus);
		if (reiki > 0) player->insert_package(ADD_FROM_RESTORE, GameEnum::ITEM_ID_REIKI, reiki, bind_staus);
		 */

	}
	return 0;
}

int MLExpRestore::fetch_exp_restore_info()
{
	IntMap storage_map;
	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		JUDGE_CONTINUE(0 >= storage_map.count(iter->__storage_id))
		storage_map[iter->__storage_id] = iter->__storage_id;
	}

	Proto51401411 restore_info;

	for (int i = GameEnum::ES_ACT_TYPE_BEGIN; i <= GameEnum::ES_ACT_TYPE_END; ++i)
	{
		JUDGE_CONTINUE(this->exp_restore_detail_.__pre_act_map[i].__ext_type > 0);
		int act_id = i * GameEnum::ES_ACT_BASE_NUM + this->exp_restore_detail_.__pre_act_map[i].__ext_type;
		const Json::Value &restore_json = CONFIG_INSTANCE->restore_json_by_act_id(act_id);
		JUDGE_CONTINUE(restore_json != Json::Value::null);

		//等级 和 vip 限制
		JUDGE_CONTINUE(this->role_level() >= restore_json["open_level"].asInt() &&
				this->vip_detail().__vip_level >= restore_json["vip_limit"].asInt());

		ProtoExpRestore* info = restore_info.add_restore_info();

		int exp = 0, reputation = 0, honour = 0, exploit = 0,league_contri = 0,
			reiki = 0, gold = 0, bind_gold = 0;

		IntVec free_list, money_list;
		RewardInfo reward_info_total;
		this->check_restore_info(reward_info_total, i, exp, reputation, honour, exploit, league_contri,
				reiki, gold, bind_gold, free_list, money_list, 0, 1,
				this->exp_restore_detail_.__pre_act_map[i].__times);

		if (this->exp_restore_detail_.__pre_act_map[-i].__ext_type > 0)
			info->set_show_index(1);

		info->set_activity_id(i * GameEnum::ES_ACT_BASE_NUM + 1);
		info->set_exp(exp);
		info->set_reputation(reputation);
		info->set_honour(honour);
		info->set_exploit(exploit);
		info->set_league_contri(league_contri);
		info->set_reiki(reiki);
		ProtoMoney* money = info->mutable_money();
		money->set_gold(gold);
		money->set_bind_gold(bind_gold);
		for (int i = 0; i < (int)free_list.size(); ++i)
			info->add_free_reward_list(free_list[i]);
		for (int i = 0; i < (int)money_list.size(); ++i)
			info->add_money_reward_list(money_list[i]);


		info->set_free_rate(restore_json["free_rate"].asInt());
		info->set_need_money(restore_json["price"].asInt());
		info->set_open_level(restore_json["open_level"].asInt());
		info->set_vip_limit(restore_json["vip_limit"].asInt());
	}

//	Time_Value timev = Time_Value::gettimeofday();
//	for(IntMap::iterator iter = storage_map.begin(); iter != storage_map.end(); ++iter)
//	{
//		int total_exp = 0, restore_count = 0, need_money = 0;
//		JUDGE_CONTINUE(0 == this->cal_exp_storage(iter->first, timev, restore_count, total_exp, need_money));
//		JUDGE_CONTINUE(restore_count > 0);
//		//JUDGE_CONTINUE(need_money > 0);
//		JUDGE_CONTINUE(total_exp > 0);

//		const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json(iter->first);
//		int free_restore_rate = exp_restore_json["free_restore_rate"].asInt();
//		int free_exp = ::rint(total_exp * free_restore_rate / 100.0);

//		ProtoExpRestore* proto_exp_restore = restore_info.add_restore_info();
//		proto_exp_restore->set_activity_id(iter->first);
//		proto_exp_restore->set_restore_count(restore_count);
//		proto_exp_restore->set_total_exp(total_exp);
//		proto_exp_restore->set_free_exp(free_exp);
//		proto_exp_restore->set_need_money(need_money);
//	}

	//this->test_output_storage_info();
	//MSG_USER("\r\n %s", restore_info.Utf8DebugString().c_str());
	FINER_PROCESS_RETURN(RETURN_FETCH_EXP_RESTORE_INFO, &restore_info);
}

int MLExpRestore::fetch_restore_goods_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11401417*, request, -1);
	int act_id = request->activity_id() / GameEnum::ES_ACT_BASE_NUM;
	int get_type = request->restore_type();
	int is_all = request->is_all();

	int exp = 0, reputation = 0, honour = 0, exploit = 0,league_contri = 0,
		reiki = 0, gold = 0, bind_gold = 0;
	RewardInfo reward_info_total;
	IntVec free_list, money_list;

	if (is_all)
	{
		for (RestoreMap::iterator it = this->exp_restore_detail_.__pre_act_map.begin();
				it != this->exp_restore_detail_.__pre_act_map.end(); ++it)
		{
			int id = it->first;
			//今日是否领取
			JUDGE_CONTINUE(this->exp_restore_detail_.__pre_act_map[-id].__ext_type == 0
					&& this->exp_restore_detail_.__pre_act_map[id].__ext_type > 0);

			int t_exp = 0, t_reputation = 0, t_honour = 0, t_exploit = 0,t_league_contri = 0,
					t_reiki = 0, t_gold = 0, t_bind_gold = 0;
			this->check_restore_info(reward_info_total, id, t_exp, t_reputation, t_honour, t_exploit, t_league_contri,
					t_reiki, t_gold, t_bind_gold, free_list, money_list, 0, get_type,
					this->exp_restore_detail_.__pre_act_map[id].__times);

			exp += t_exp; reputation += t_reputation; honour += t_honour; exploit += t_exploit;
			league_contri += t_league_contri; reiki += t_reiki; gold += t_gold; bind_gold += t_bind_gold;
		}
	}
	else
	{
		this->check_restore_info(reward_info_total, act_id, exp, reputation, honour, exploit, league_contri,
				reiki, gold, bind_gold, free_list, money_list, 0, get_type,
				this->exp_restore_detail_.__pre_act_map[act_id].__times);
	}

	if (get_type == 1)
	{
		int rate = 5000;
		exp = int((double) exp * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		gold = int((double) gold * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		bind_gold = int((double) bind_gold * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		honour = int((double) honour * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		exploit = int((double) exploit * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		league_contri = int((double) league_contri * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
		reiki = int((double) reiki * (double) rate / (double) GameEnum::ES_ACT_BASE_NUM);
	}

	Proto51401417 respond;
	ProtoExpRestore* info = respond.mutable_restore_info();
	info->set_activity_id(request->activity_id());
	info->set_exp(exp);
	info->set_reputation(reputation);
	info->set_honour(honour);
	info->set_exploit(exploit);
	info->set_league_contri(league_contri);
	info->set_reiki(reiki);
	ProtoMoney* money = info->mutable_money();
	money->set_gold(gold);
	money->set_bind_gold(bind_gold);
	for (int i = 0; i < (int)free_list.size(); ++i)
		info->add_free_reward_list(free_list[i]);
	for (int i = 0; i < (int)money_list.size(); ++i)
		info->add_money_reward_list(money_list[i]);
	FINER_PROCESS_RETURN(RETURN_RESTORE_GOODS_INFO, &respond);
}

int MLExpRestore::exp_restore_single(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11401412*, request, -1);

	int id = request->activity_id() / GameEnum::ES_ACT_BASE_NUM;
	int get_type = request->restore_type();

	//今日是否领取
	CONDITION_NOTIFY_RETURN(this->exp_restore_detail_.__pre_act_map[-id].__ext_type == 0
			&& this->exp_restore_detail_.__pre_act_map[id].__ext_type > 0,
			RETURN_RESTORE_EXP_SINGLE, ERROR_CLIENT_OPERATE);

	int exp = 0, reputation = 0, honour = 0, exploit = 0,league_contri = 0,
		reiki = 0, gold = 0, bind_gold = 0;

	IntVec free_list, money_list;
	RewardInfo reward_info_total;
	this->check_restore_info(reward_info_total, id, exp, reputation, honour, exploit, league_contri,
			reiki, gold, bind_gold, free_list, money_list, 1, get_type,
			this->exp_restore_detail_.__pre_act_map[id].__times);

	this->insert_restore_good(exp, reputation, honour, exploit, league_contri, reiki, gold, bind_gold, reward_info_total);

	RestoreActInfo &temp = this->exp_restore_detail_.__pre_act_map[-id];
	temp.__ext_type = this->exp_restore_detail_.__pre_act_map[id].__ext_type;
	temp.__times = this->exp_restore_detail_.__pre_act_map[id].__times;

	Proto51401412 respond;
	respond.set_activity(request->activity_id());
	FINER_PROCESS_RETURN(RETURN_RESTORE_EXP_SINGLE, &respond);
	/*
	int storage_id = request->activity_id();
	int restore_type = request->restore_type();
	Time_Value timev = Time_Value::gettimeofday();


	const Json::Value &restore_json = CONFIG_INSTANCE->exp_restore_json(storage_id);
	const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json(storage_id);
	CONDITION_NOTIFY_RETURN(exp_restore_json != Json::Value::null,RETURN_RESTORE_EXP_SINGLE, ERROR_CLIENT_OPERATE);

	int total_exp = 0, restore_count = 0, need_money = 0;

	CONDITION_NOTIFY_RETURN(0 == this->cal_exp_storage(storage_id, timev, restore_count, total_exp, need_money),
			RETURN_RESTORE_EXP_SINGLE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(0 != total_exp, RETURN_RESTORE_EXP_SINGLE, ERROR_CLIENT_OPERATE);

	if(GameEnum::ER_TYPE_FREE == restore_type)
	{
		int free_restore_rate = exp_restore_json["free_restore_rate"].asInt();
		int free_exp = ::rint(total_exp * free_restore_rate / 100.0) ;
		// 回复经验
		this->map_logic_player()->request_add_exp(free_exp, EXP_RESTORE);
	}
	else
	{
		if(0 != need_money)
		{
			Money money = GameCommon::make_up_money(need_money, GameEnum::MONEY_UNBIND_GOLD);
			CONDITION_NOTIFY_RETURN(true == GameCommon::validate_money(money),
					RETURN_RESTORE_EXP_SINGLE, ERROR_SERVER_INNER);

			GameCommon::adjust_money(money, this->own_money());
			int ret = this->pack_money_sub(money, SUB_MONEY_EXP_RESTORE);
			CONDITION_NOTIFY_RETURN(0 == ret, RETURN_RESTORE_EXP_SINGLE, ERROR_PACKAGE_MONEY_AMOUNT);
		}

		this->map_logic_player()->request_add_exp(total_exp, EXP_RESTORE);
	}

	// 清除记录
	this->mark_storage_record_invalid(storage_id, timev);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_EXP_RESTORE);
	this->map_logic_player()->notify_player_welfare_status(this);
*/

}

int MLExpRestore::exp_restore_all(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11401413*, request, -1);
	int get_type = request->restore_type();

	int exp = 0, reputation = 0, honour = 0, exploit = 0,league_contri = 0,
		reiki = 0, gold = 0, bind_gold = 0;

	RewardInfo reward_info_total;

	for (RestoreMap::iterator it = this->exp_restore_detail_.__pre_act_map.begin();
			it != this->exp_restore_detail_.__pre_act_map.end(); ++it)
	{
		int id = it->first;
		//今日是否领取
		JUDGE_CONTINUE(this->exp_restore_detail_.__pre_act_map[-id].__ext_type == 0
				&& this->exp_restore_detail_.__pre_act_map[id].__ext_type > 0);

		int t_exp = 0, t_reputation = 0, t_honour = 0, t_exploit = 0,t_league_contri = 0,
				t_reiki = 0, t_gold = 0, t_bind_gold = 0;
		IntVec free_list, money_list;
		this->check_restore_info(reward_info_total, id, t_exp, t_reputation, t_honour, t_exploit, t_league_contri,
				t_reiki, t_gold, t_bind_gold, free_list, money_list, 1, get_type,
				this->exp_restore_detail_.__pre_act_map[id].__times);

		RestoreActInfo &temp = this->exp_restore_detail_.__pre_act_map[-id];
		temp.__ext_type = this->exp_restore_detail_.__pre_act_map[id].__ext_type;
		temp.__times = this->exp_restore_detail_.__pre_act_map[id].__times;

		exp += t_exp; reputation += t_reputation; honour += t_honour; exploit += t_exploit;
		league_contri += t_league_contri; reiki += t_reiki; gold += t_gold; bind_gold += t_bind_gold;
	}

	this->insert_restore_good(exp, reputation, honour, exploit, league_contri, reiki, gold, bind_gold, reward_info_total);
	/*
	int restore_type = request->restore_type();
	Time_Value timev = Time_Value::gettimeofday();

	IntMap storage_map;
	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		JUDGE_CONTINUE(0 >= storage_map.count(iter->__storage_id))
		storage_map[iter->__storage_id] = iter->__storage_id;
	}

	int all_exp = 0, all_free_exp = 0, all_money = 0;

	for(IntMap::iterator iter = storage_map.begin(); iter != storage_map.end(); ++iter)
	{
		int total_exp = 0, restore_count = 0, need_money = 0;

		JUDGE_CONTINUE(0 == this->cal_exp_storage(iter->first, timev, restore_count, total_exp, need_money));
		JUDGE_CONTINUE(restore_count != 0);

		const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json(iter->first);
		JUDGE_CONTINUE(exp_restore_json != Json::Value::null);

		int free_restore_rate = exp_restore_json["free_restore_rate"].asInt();
		int free_exp = ::rint(total_exp * free_restore_rate / 100.0);

		all_exp += total_exp;
		all_money += need_money;
		all_free_exp += free_exp;
	}

	if(GameEnum::ER_TYPE_FREE == restore_type)
	{
		this->map_logic_player()->request_add_exp(all_free_exp, EXP_RESTORE);
	}
	else
	{
		if(0 != all_money)
		{
			Money money = GameCommon::make_up_money(all_money, GameEnum::MONEY_UNBIND_GOLD);
			CONDITION_NOTIFY_RETURN(true == GameCommon::validate_money(money),
					RETURN_RESTORE_EXP_SINGLE, ERROR_SERVER_INNER);

			GameCommon::adjust_money(money, this->own_money());
			int ret = this->pack_money_sub(money, SUB_MONEY_EXP_RESTORE);
			CONDITION_NOTIFY_RETURN(0 == ret, RETURN_RESTORE_EXP_SINGLE, ERROR_PACKAGE_MONEY_AMOUNT);
		}

		this->map_logic_player()->request_add_exp(all_exp, EXP_RESTORE);
	}

	// 清除记录
	for(IntMap::iterator iter = storage_map.begin(); iter != storage_map.end(); ++iter)
	{
		this->mark_storage_record_invalid(iter->first, timev);
	}

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_EXP_RESTORE);
	this->map_logic_player()->notify_player_welfare_status(this);
*/
	FINER_PROCESS_NOTIFY(RETURN_RESTORE_EXP_ALL);
}

int MLExpRestore::refresh_restore_info(int event_id, int value, int times)
{
	this->exp_restore_detail_.__now_act_map[event_id].__ext_type = value;
	this->exp_restore_detail_.__now_act_map[event_id].__times -= times;

	return 0;
}

int MLExpRestore::exp_restore_event_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400401*, request, -1);

	int act_id = request->event_id(); //活动ID
	int value = request->value(); //关键字
	int times = request->times(); //次数

	if (act_id == GameEnum::ES_ACT_ARENA)
	{
		const Json::Value &json = CONFIG_INSTANCE->restore_json();
		for (int i = (int)json.size(); i > 0; --i)
		{
			const Json::Value &temp = CONFIG_INSTANCE->restore_json(i);
			if (temp["act_type"].asInt() == act_id && value >= temp["act_ext_type"].asInt())
			{
				value = temp["act_ext_type"].asInt();
				break;
			}
		}
	}

	if (value > 0) this->exp_restore_detail_.__now_act_map[act_id].__ext_type = value;

	if (this->exp_restore_detail_.__now_act_map[act_id].__times > 0)
	this->exp_restore_detail_.__now_act_map[act_id].__times -= times;

	return 0;
	/*
	const Json::Value &request_event_json = CONFIG_INSTANCE->exp_restore_json(request->event_id());
	JUDGE_RETURN(Json::Value::null != request_event_json, -1);
	int storage_id = request_event_json["storage_id"].asInt();

	Time_Value timev(request->times());
	if(0 == request->times())
	{
		timev = Time_Value::gettimeofday();
	}

	return this->storage_finish_times_reduce(storage_id, timev);
	*/
}

int MLExpRestore::sync_storage_stage_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400402*, request, -1);

	int act_id = request->event_id(); //活动ID
	int value = request->value(); //关键字
	int times = request->times(); //次数

	if (act_id == GameEnum::ES_ACT_ARENA)
	{
		const Json::Value &json = CONFIG_INSTANCE->restore_json();
		for (int i = (int)json.size(); i > 0; --i)
		{
			const Json::Value &temp = CONFIG_INSTANCE->restore_json(i);
			if (temp["act_type"].asInt() == act_id && value >= temp["act_ext_type"].asInt())
			{
				value = temp["act_ext_type"].asInt();
				break;
			}
		}
	}

	if (value > 0)
	{
		this->exp_restore_detail_.__now_act_map[act_id].__ext_type = value;
	}

	if (this->exp_restore_detail_.__now_act_map[act_id].__times > 0)
	{
		this->exp_restore_detail_.__now_act_map[act_id].__times -= times;
	}

//	Time_Value timev(request->time_sec());
//	this->upsert_storage_stage_info(timev, request->event_id(), request->stage());
	return 0;
}

ExpRestoreDetail &MLExpRestore::exp_restore_detail()
{
	return this->exp_restore_detail_;
}

StorageRecordSet &MLExpRestore::storage_record_set()
{
	return this->exp_restore_detail().__storage_record_set;
}

LongMap &MLExpRestore::stamp_level_map()
{
	return this->exp_restore_detail_.__timestamp_level_map;
}

LongMap &MLExpRestore::stamp_vip_map()
{
	return this->exp_restore_detail_.__timestamp_vip_map;
}

int MLExpRestore::set_notify_msg_exp_restore_info(Message *msg)
{
	if (GameCommon::check_welfare_open_condition("exp_restore", this->role_level(), this->role_create_days()) != 0)
		return 0;

	DYNAMIC_CAST_RETURN(Proto81401401*, respond, msg, -1);
	respond->set_exp_restore(this->count_exp_storage_num(Time_Value::gettimeofday()));
	return 0;
}

/*只计算可以找回经验的活动个数*/
int MLExpRestore::count_exp_storage_num(const Time_Value &timev)
{
	Time_Value time_stamp = next_day(0, 0, timev);
	int max_day = this->exp_restore_max_day();

	IntMap storage_map;
	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		JUDGE_CONTINUE(storage_map.count(iter->__storage_id) <= 0);

		int time_stamp_sec = time_stamp.sec() - Time_Value::DAY * max_day;
		JUDGE_CONTINUE(iter->__record_timestamp.sec() >= time_stamp_sec);
		JUDGE_CONTINUE(iter->__record_timestamp < time_stamp);
		JUDGE_CONTINUE(iter->__storage_valid == true);

		int vip_type = this->vip_type_with_timestamp(iter->__record_timestamp.sec());
		int max_num = this->storage_max_finish_times(iter->__storage_id, vip_type, iter->__record_timestamp.sec());
		JUDGE_CONTINUE(iter->__finish_count < max_num);

		int per_exp_value = storage_exp_value(iter->__storage_id, iter->__record_timestamp.sec());
		JUDGE_CONTINUE(per_exp_value > 0);

		storage_map[iter->__storage_id] = iter->__storage_id;
	}

	return storage_map.size();
}

int MLExpRestore::storage_finish_times_reduce(int storage_id, const Time_Value &timev)
{
	const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json(storage_id);
	JUDGE_RETURN(Json::Value::null != exp_restore_json, -1);

	Time_Value timestamp = next_day(0, 0, timev);
	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		JUDGE_CONTINUE(iter->__storage_valid == true);
		JUDGE_CONTINUE(iter->__storage_id == storage_id);
		JUDGE_CONTINUE(iter->__record_timestamp == timestamp);
		iter->__finish_count +=1;

		return 0;
	}

	return -1;
}

void MLExpRestore::cal_restore_record(const Time_Value &timev)
{
	JUDGE_RETURN(this->exp_restore_detail().__check_timestamp < timev, ;);
	Time_Value timestamp = next_day(0,0, timev);

	IntVec activity_set;
	this->get_storage_id_set(activity_set);
	JUDGE_RETURN(activity_set.size() > 0, ;);

	int max_restore_day = this->exp_restore_max_day();

	for(IntVec::iterator iter = activity_set.begin(); iter != activity_set.end(); ++iter)
	{
		for(int i = 0; i <= max_restore_day; i++)
		{
			int64_t timestamp_sec = timestamp.sec() - Time_Value::DAY * i;
			JUDGE_CONTINUE(this->exp_restore_detail().__check_timestamp.sec() < timestamp_sec)

			StorageRecord record;
			record.__storage_id = *iter;
			record.__finish_count = 0;
			record.__storage_valid = true;
			record.__record_timestamp.set(timestamp_sec, 0);

			this->storage_record_set().push_back(record);
		}
	}
}

int MLExpRestore::exp_restore_max_day()
{
	const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json();
	JUDGE_RETURN(exp_restore_json != Json::Value::null, 0);

	return exp_restore_json["restore_days"].asInt();
}

int MLExpRestore::level_with_timestamp(int64_t timestamp)
{
	LongMap::iterator iter = this->stamp_level_map().find(timestamp);
	JUDGE_RETURN(this->stamp_level_map().end() != iter, 1);

	return iter->second;
}

int MLExpRestore::vip_type_with_timestamp(int64_t timestamp)
{
	LongMap::iterator iter = this->stamp_vip_map().find(timestamp);
	JUDGE_RETURN(this->stamp_vip_map().end() != iter, VIP_NOT_VIP);

	return iter->second;
}

// 单个次数可找回的经验
int MLExpRestore::storage_exp_value(int storage_id, int64_t time_stamp)
{
	const Json::Value &storage_json = CONFIG_INSTANCE->exp_restore_json(storage_id);
	JUDGE_RETURN(storage_json != Json::Value::null, 0);

	if(storage_json["stage_type"].asString() == GameName::ER_STAGE_PASS_CHAPTER)
	{
		int stage_exp = 0;
		if(storage_json["stage_exp"].isArray())
			stage_exp = storage_json["stage_exp"][0u].asInt();
		else
			stage_exp = storage_json["stage_exp"].asInt();

		return stage_exp;
	}
	else
	{
		JUDGE_RETURN(0 != time_stamp, 0);
		int stage_exp = 0;
		int64_t check_stamp = 0;

		StorageStageInfo::iterator stage_info_iter =
				this->exp_restore_detail().__storage_stage_info.find(storage_id);
		JUDGE_RETURN(stage_info_iter != this->exp_restore_detail().__storage_stage_info.end(), 0);

		TimestampStageMap &timestamp_stage_map = stage_info_iter->second;
		for(TimestampStageMap::iterator iter = timestamp_stage_map.begin();
				iter != timestamp_stage_map.end(); ++iter)
		{
			JUDGE_CONTINUE(iter->first <= time_stamp); // 跳过记录产生在timev之后
			JUDGE_CONTINUE(iter->first >= check_stamp); // 取最近的记录

			check_stamp = iter->first;
			stage_exp = iter->second;
		}

		return stage_exp;
	}
}

// 该活动可参与的最大次数
int MLExpRestore::storage_max_finish_times(int storage_id, int vip_type, int64_t time_stamp)
{
	const Json::Value &storage_json = CONFIG_INSTANCE->exp_restore_json(storage_id);
	JUDGE_RETURN(storage_json != Json::Value::null, 0);

	if(storage_json["stage_type"].asString() == GameName::ER_STAGE_PASS_CHAPTER)
	{
		JUDGE_RETURN(0 != time_stamp, 0);
		int finish_times = 0;
		int64_t check_stamp = 0;

		StorageStageInfo::iterator stage_info_iter =
				this->exp_restore_detail().__storage_stage_info.find(storage_id);
		JUDGE_RETURN(stage_info_iter != this->exp_restore_detail().__storage_stage_info.end(), 0);

		TimestampStageMap &timestamp_stage_map = stage_info_iter->second;
		for(TimestampStageMap::iterator iter = timestamp_stage_map.begin();
				iter != timestamp_stage_map.end(); ++iter)
		{
			JUDGE_CONTINUE(iter->first <= time_stamp); // 跳过记录产生在timev之后
			JUDGE_CONTINUE(iter->first >= check_stamp); // 取最近的记录

			check_stamp = iter->first;
			finish_times = iter->second;
		}

		return finish_times;
	}
	else // 固定次数
	{
		JUDGE_RETURN(true == storage_json.isMember("finish_times"), 0);

		int finish_times = storage_json["finish_times"].asInt();
		if(true == storage_json.isMember("vip_finish_times") && VIP_NOT_VIP != vip_type)
		{
		//	(VIP_SLIVER  == vip_type) && (finish_times += storage_json["vip_finish_times"][0u].asInt());
	//		(VIP_GOLD    == vip_type) && (finish_times += storage_json["vip_finish_times"][1u].asInt());
		//	(VIP_DIAMOND == vip_type) && (finish_times += storage_json["vip_finish_times"][2u].asInt());
		//	(VIP_EXTREME == vip_type) && (finish_times += storage_json["vip_finish_times"][3u].asInt());
		}

		return finish_times;
	}
}

int MLExpRestore::vip_extra_finish_times(int storage_id, int vip_type)
{
	const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json(storage_id);
	JUDGE_RETURN(Json::Value::null != exp_restore_json, 0);
	JUDGE_RETURN(exp_restore_json.isMember("vip_extra_count"), 0);
	JUDGE_RETURN(VIP_NOT_VIP != vip_type, 0);

	int finish_times = 0;
/*	(VIP_SLIVER	 == vip_type) && (finish_times = exp_restore_json["vip_extra_count"][0u].asInt());
	(VIP_GOLD	 == vip_type) && (finish_times = exp_restore_json["vip_extra_count"][1u].asInt());
	(VIP_DIAMOND == vip_type) && (finish_times = exp_restore_json["vip_extra_count"][2u].asInt());
	(VIP_EXTREME == vip_type) && (finish_times = exp_restore_json["vip_extra_count"][3u].asInt());*/
	return finish_times;
}

int MLExpRestore::exp_restore_cost_money(int exp_value)
{
	JUDGE_RETURN(0 != exp_value, 0);
	const Json::Value exp_restore_json = CONFIG_INSTANCE->exp_restore_json();
	JUDGE_RETURN(Json::Value::null != exp_restore_json, 100 * exp_value);
	JUDGE_RETURN(exp_restore_json.isMember("exp_price")== true, 100 * exp_value);

	int exp = exp_restore_json["exp_price"][0u].asInt();
	int money = exp_restore_json["exp_price"][1u].asInt();

	JUDGE_RETURN(0 != exp, 100 * exp_value);
	JUDGE_RETURN(0 != money, 100 * exp_value);

	return ::ceil((exp_value * money) / (double)exp);
}

/*
 * timev 当前时间即可
 * */
int MLExpRestore::cal_exp_storage(int storage_id, const Time_Value &timev, int &count, int &exp, int &money)
{
	const Json::Value exp_storage_json  = CONFIG_INSTANCE->exp_restore_json(storage_id);
	JUDGE_RETURN(Json::Value::null != exp_storage_json, -1);

	Time_Value time_stamp = next_day(0, 0, timev);
	int max_day = this->exp_restore_max_day();

	count = exp = 0;

	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		int64_t rec_time_stamp = iter->__record_timestamp.sec();
		JUDGE_CONTINUE(iter->__storage_id == storage_id);
		JUDGE_CONTINUE(iter->__storage_valid == true);
		JUDGE_CONTINUE(rec_time_stamp >= time_stamp.sec() - max_day * Time_Value::DAY);
		JUDGE_CONTINUE(rec_time_stamp < time_stamp.sec());

		int vip_type = this->vip_type_with_timestamp(rec_time_stamp);

		int per_exp_value = storage_exp_value(storage_id, rec_time_stamp);
		JUDGE_CONTINUE(per_exp_value > 0);

		int finish_times = this->storage_max_finish_times(storage_id, vip_type, rec_time_stamp);
		int unfinished_count = finish_times - iter->__finish_count;
		JUDGE_CONTINUE(unfinished_count > 0);

		count += unfinished_count;
		exp += (per_exp_value * unfinished_count);

//		long time_value1 = rec_time_stamp;
//		long time_value2 = time_stamp.sec();
//		MSG_USER(%ld >> %s, time_value1, ctime(&time_value1));
//		MSG_USER(%ld >> %s, time_value2, ctime(&time_value2));
//		MSG_USER(used %d of %d, iter->__finish_count, finish_times);
//		MSG_USER(第%ld天前, (time_stamp.sec() - rec_time_stamp)/Time_Value::DAY);
//		MSG_USER(副本 %d >>>  %d经验 * %d次数, storage_id, per_exp_value, unfinished_count);
	}

//	MSG_USER(副本%d >>>  %d总次数  总共%d经验, storage_id, count, exp);
	//计算找回需要的元宝
	money = this->exp_restore_cost_money(exp);
	return 0;
}

int MLExpRestore::mark_storage_record_invalid(int storage_id, const Time_Value &timev)
{
	Time_Value timestamp = next_day(0, 0, timev);
	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		JUDGE_CONTINUE(iter->__storage_id == storage_id);
		JUDGE_CONTINUE(iter->__record_timestamp < timestamp);
		iter->__storage_valid = false;
	}

	return 0;
}

int MLExpRestore::exp_restore_lvl_up(int level)
{
	const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json();
	JUDGE_RETURN(Json::Value::null != exp_restore_json, -1);
	JUDGE_RETURN(level >= exp_restore_json["require_lvl"].asInt(), -1);

	Time_Value timestamp = next_day(0, 0, Time_Value::gettimeofday());
	this->stamp_level_map()[timestamp.sec()] = level;

	Json::Value::Members member_names = exp_restore_json.getMemberNames();
	for(Json::Value::Members::iterator iter = member_names.begin(); iter != member_names.end(); iter++)
	{
		const Json::Value &parse_json = exp_restore_json[*iter];
		JUDGE_CONTINUE(true == parse_json.isObject());
		JUDGE_CONTINUE(true == parse_json.isMember("storage_id"));
		JUDGE_CONTINUE(true == parse_json.isMember("free_restore_rate"));
		int storage_id = parse_json["storage_id"].asInt();

		int require_level = parse_json["start_lvl"].asInt();
		JUDGE_CONTINUE(require_level > 0);

		if (level == require_level)
		{
			StorageRecord storeage_rec;
			storeage_rec.__storage_id = storage_id;
			storeage_rec.__finish_count = 0;
			storeage_rec.__storage_valid = true;
			storeage_rec.__record_timestamp = timestamp;

			this->insert_exp_storage_rec(storeage_rec);
		}
	}

	return 0;
}

void MLExpRestore::exp_restore_vip_rec(int vip_type, int expired_time)
{
	Time_Value timev_now = Time_Value::gettimeofday();
	Time_Value timestamp = next_day(0, 0, timev_now);
	this->stamp_vip_map()[timestamp.sec()] = vip_type;

	JUDGE_RETURN(vip_type > VIP_NOT_VIP && expired_time > 0, ;);
	this->exp_restore_detail().__vip_type_record = vip_type;
	this->exp_restore_detail().__vip_start_time = timev_now.sec();
	this->exp_restore_detail().__vip_expried_time = expired_time;
}

int MLExpRestore::exp_restore_notify_reflash()
{
	FINER_PROCESS_NOTIFY(ACTIVE_NOTIFY_EXP_RESTORE_CHANGE);
}

int MLExpRestore::restore_jump_next_day(const int jump_days)
{
	MSG_USER("JUMP NEXT DAY");
	Time_Value &check_timestamp = this->exp_restore_detail().__check_timestamp;

	int jump = std::max(1, jump_days);
	if(Time_Value::zero != check_timestamp)
	{
		check_timestamp = Time_Value(check_timestamp.sec() - Time_Value::DAY * jump);
	}

	/*
	LongMap new_level_map;
	for(LongMap::iterator iter = this->stamp_level_map().begin();
			iter != this->stamp_level_map().end(); ++iter)
	{
		new_level_map[iter->first -Time_Value::DAY] = iter->second;
	}

	new_level_map.erase(new_level_map.begin());
	this->stamp_level_map().swap(new_level_map);

	LongMap new_vip_map;
	for(LongMap::iterator iter = this->stamp_vip_map().begin();
			iter != this->stamp_vip_map().end(); ++iter)
	{
		new_vip_map[iter->first -Time_Value::DAY] = iter->second;
	}

	new_vip_map.erase(new_vip_map.begin());
	this->stamp_vip_map().swap(new_vip_map);

	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		iter->__record_timestamp = Time_Value(iter->__record_timestamp.sec() - Time_Value::DAY);
	}

	StorageStageInfo &storage_stage_info = this->exp_restore_detail().__storage_stage_info;
	for(StorageStageInfo::iterator iter_i = storage_stage_info.begin();
			iter_i != storage_stage_info.end(); ++iter_i)
	{
		TimestampStageMap new_timestamp_stage_map;
		TimestampStageMap &timestamp_stage_map = iter_i->second;
		for(TimestampStageMap::iterator iter_n = timestamp_stage_map.begin();
				iter_n != timestamp_stage_map.end(); ++iter_n)
		{
			new_timestamp_stage_map[iter_n->first - Time_Value::DAY] = iter_n->second;
		}
		timestamp_stage_map.swap(new_timestamp_stage_map);
	}
*/
	this->check_and_update_exp_restore_info();
//	this->exp_restore_notify_reflash();
	this->map_logic_player()->notify_player_welfare_status(this);

	return 0;
}

int MLExpRestore::get_storage_id_set(IntVec &id_set)
{
	const Json::Value &exp_restore_json = CONFIG_INSTANCE->exp_restore_json();
	JUDGE_RETURN(Json::Value::null != exp_restore_json, -1);
	JUDGE_RETURN(this->role_level() >= exp_restore_json["require_lvl"].asInt(), -1);

	id_set.clear();

	Json::Value::Members member_names = exp_restore_json.getMemberNames();
	for(Json::Value::Members::iterator iter = member_names.begin(); iter != member_names.end(); ++iter)
	{
		const Json::Value &parse_json = exp_restore_json[*iter];
		JUDGE_CONTINUE(true == parse_json.isObject());
		JUDGE_CONTINUE(true == parse_json.isMember("storage_id"));
		JUDGE_CONTINUE(true == parse_json.isMember("free_restore_rate"));

		int require_level = parse_json["start_lvl"].asInt();
		if (require_level != 0 && this->role_level() >= require_level)
		{
			int restore_id = parse_json["storage_id"].asInt();
			id_set.push_back(restore_id);
		}
	}

	return 0;
}

void MLExpRestore::cal_stamp_level_map(const Time_Value &timev)
{
	Time_Value timestamp = next_day(0, 0, timev);
	this->stamp_level_map()[timestamp.sec()] = this->role_level();

	int max_day = this->exp_restore_max_day();
	for(int i = max_day; i >= 0; --i)
	{
		int timestamp_sec = timestamp.sec() - Time_Value::DAY * i;
		JUDGE_CONTINUE(0 >= this->stamp_level_map().count(timestamp_sec));
		this->stamp_level_map()[timestamp_sec] = this->role_level();
	}
}
void MLExpRestore::cal_stamp_vip_map(const Time_Value &timev)
{
	Time_Value timestamp = next_day(0, 0, timev);
	this->stamp_vip_map()[timestamp.sec()] = this->vip_type();

	int orig_vip_type = this->exp_restore_detail().__vip_type_record;
	int expried_time = this->exp_restore_detail().__vip_expried_time;
	int start_time = this->exp_restore_detail().__vip_start_time;
	Time_Value expried_stamp = next_day(0, 0, Time_Value(expried_time));
	Time_Value start_stamp = next_day(0, 0, Time_Value(start_time));

	int max_day = this->exp_restore_max_day();
	for(int i = max_day; i > 0; --i)
	{
		int timestamp_sec = timestamp.sec() - Time_Value::DAY * i;
		// VIP开始前的记录不处理
		JUDGE_CONTINUE(timestamp_sec < start_stamp.sec());
		this->stamp_vip_map()[timestamp_sec] = orig_vip_type;
		// VIP过期后记录为 非VIP状态
		JUDGE_CONTINUE(timestamp_sec >= expried_stamp.sec());
		this->stamp_vip_map()[timestamp_sec] = VIP_NOT_VIP;
	}
}
int MLExpRestore::query_storage_stage_info(const Time_Value &timev, int storage_id)
{
	StorageStageInfo &storage_stage_info = this->exp_restore_detail().__storage_stage_info;
	StorageStageInfo::iterator stage_info_iter = storage_stage_info.find(storage_id);
	JUDGE_RETURN(stage_info_iter != storage_stage_info.end(), 0);

	int max_day = this->exp_restore_max_day();
	int stage_value = 0;

	TimestampStageMap &timestamp_stage_map = stage_info_iter->second;
	for(TimestampStageMap::iterator iter = timestamp_stage_map.begin();
			iter != timestamp_stage_map.end(); ++iter)
	{
		JUDGE_CONTINUE((timev.sec() - iter->first) <= Time_Value::DAY * max_day);
		if(iter->second >= stage_value)
			stage_value = iter->second;
	}

	return stage_value;
}

int MLExpRestore::trans_storage_id(int event_id)
{
	const Json::Value &request_event_json = CONFIG_INSTANCE->exp_restore_json(event_id);
	JUDGE_RETURN(Json::Value::null != request_event_json, 0);

	return request_event_json["storage_id"].asInt();
}

int MLExpRestore::cal_storage_stage_value(int event_id, int stage)
{
	const Json::Value &request_event_json = CONFIG_INSTANCE->exp_restore_json(event_id);
	JUDGE_RETURN(Json::Value::null != request_event_json, -1);

	int storage_id = request_event_json["storage_id"].asInt();

	const Json::Value &storage_json = CONFIG_INSTANCE->exp_restore_json(storage_id);
	JUDGE_RETURN(Json::Value::null != storage_json, -1);

	int stage_value = 0;
	if(storage_json["stage_type"].asString() == GameName::ER_STAGE_PASS_CHAPTER)
	{// 计算通关章的总数

		const Json::Value &script_json = CONFIG_INSTANCE->script(storage_id);
		JUDGE_RETURN(Json::Value::null != script_json, stage_value);

		uint pass_piece = stage / 1000;
		uint pass_chapter = stage % 1000;

		const Json::Value &piece_json = script_json["prev_condition"]["piece"];
		JUDGE_RETURN(piece_json.isArray(), stage_value);

		for(uint i = 2; (i <= pass_piece) && (i <= piece_json.size()); ++i)
		{
			stage_value += piece_json[i-2].asInt();
		}

		stage_value += pass_chapter;
		//MSG_USER(stage %d translate to %d, stage, stage_value);
	}
	else
	{// 通关难度对应的经验
		if(request_event_json["stage_exp"].isArray())
		{
			int stage_exp_num = request_event_json["stage_exp"].size();
			for(int i=0; i < stage_exp_num; ++i)
			{
				JUDGE_BREAK(i < stage);
				stage_value += request_event_json["stage_exp"][i].asInt();
			}
		}
		else
		{
			stage_value = stage * request_event_json["stage_exp"].asInt();
		}
	}

	return stage_value;
}

int MLExpRestore::upsert_storage_stage_info(const Time_Value &timev, int event_id, int stage)
{
	int stage_value  = this->cal_storage_stage_value(event_id, stage);
	JUDGE_RETURN(stage_value > 0, -1);

	int storage_id = this->trans_storage_id(event_id);
	StorageStageInfo &storage_stage_info = this->exp_restore_detail().__storage_stage_info;
	TimestampStageMap &timestamp_stage_map = storage_stage_info[storage_id];

	for(TimestampStageMap::iterator iter = timestamp_stage_map.begin();
			iter != timestamp_stage_map.end(); ++iter)
	{
		JUDGE_RETURN(iter->second < stage_value, -1);
	}

	Time_Value timestamp = next_day(0,0, timev);
	timestamp_stage_map[timestamp.sec()] = stage_value;

	return 0;
}

int MLExpRestore::insert_exp_storage_rec(StorageRecord &record)
{
	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		JUDGE_CONTINUE(iter->__storage_id != record.__storage_id);
		JUDGE_CONTINUE(iter->__record_timestamp != iter->__record_timestamp);

		return -1;
	}

	//MSG_USER("INSER RECORD ID:%d", record.__storage_id);
	this->storage_record_set().push_back(record);
	return 0;
}
void MLExpRestore::test_output_storage_info()
{
	std::ostringstream msg_stream;
	msg_stream << std::endl <<"---------------------------------------------------------" << std::endl;
	msg_stream << "VIP_TYPE: "<< this->exp_restore_detail().__vip_type_record << std::endl;
	time_t time_sec = this->exp_restore_detail().__vip_expried_time;
	msg_stream << "\tEXPRIED_TIME: "<< ctime(&time_sec);
	time_sec = this->exp_restore_detail().__check_timestamp.sec();
	msg_stream << "\tCHECK_TIMESTAMP: "<< ctime(&time_sec);
	MSG_USER(%s, msg_stream.str().c_str());
	msg_stream.clear();

	for(LongMap::iterator iter = this->stamp_vip_map().begin();
			iter!= this->stamp_vip_map().end(); ++iter)
	{
		time_sec = iter->first;
		msg_stream << "VIP: " << iter->second << "TS: "<< ctime(&time_sec);
		MSG_USER(%s, msg_stream.str().c_str());
		msg_stream.clear();
	}

	for(StorageRecordSet::iterator iter = this->storage_record_set().begin();
			iter != this->storage_record_set().end(); ++iter)
	{
		JUDGE_CONTINUE(iter->__storage_valid == true);

		time_sec = iter->__record_timestamp.sec();
		msg_stream << "id " << iter->__storage_id << " " <<iter->__finish_count
				<< (iter->__storage_valid == true ? " ":" FALSE") << " TS: "<< ctime(&time_sec);

		MSG_USER(%s, msg_stream.str().c_str());
		msg_stream.clear();
	}

	Time_Value today_timestamp = current_day(0, 0, Time_Value::gettimeofday());
	for(StorageStageInfo::iterator iter = this->exp_restore_detail().__storage_stage_info.begin();
			iter != this->exp_restore_detail().__storage_stage_info.end(); ++iter)
	{
		msg_stream << "script_sort = " << iter->first << std::endl;
		TimestampStageMap &time_stage_map = iter->second;
		for(TimestampStageMap::iterator iter_n = time_stage_map.begin();
				iter_n != time_stage_map.end(); ++iter_n)
		{
			msg_stream << "today-" << ((today_timestamp.sec() - iter_n->first) / Time_Value::DAY);
			msg_stream << " stage " << iter_n->second << std::endl;
			MSG_USER(%s, msg_stream.str().c_str());
			msg_stream.clear();
		}
	}

	msg_stream << "---------------------------------------------------------" << std::endl;
	MSG_USER(%s, msg_stream.str().c_str());
}
