/*
 * MLTreasures.cpp
 *
 *  Created on: 2016年11月30日
 *      Author: lzy
 */

#include "MLTreasures.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include <google/protobuf/message.h>

MLTreasures::MLTreasures() {
	// TODO Auto-generated constructor stub
}

MLTreasures::~MLTreasures() {
	// TODO Auto-generated destructor stub
}

TreasuresDetail& MLTreasures::treasures_detail() {
	return this->treasures_detail_;
}

int MLTreasures::rand_dice() {
	int dice_num = 0;
	const Json::Value &base_json = CONFIG_INSTANCE->treasures_base_json(1);
	int max_length = base_json["length"].asInt();
	this->treasures_detail_.game_length_ = max_length;
	int size = (int) base_json["dice_prob"][0u].size();

	int prob[size + 1], sum = 0;

	int index_size = (int) base_json["dice_prob"].size() - 1;

	if (this->treasures_detail_.reset_times_ < index_size)
		index_size = this->treasures_detail_.reset_times_;

	index_size = index_size >= 0 ? index_size : 0;

	for (int i = 0; i < size; ++i) {
		prob[i] = base_json["dice_prob"][index_size][i].asInt();
		sum += prob[i];
		JUDGE_CONTINUE(prob[i] > 0);
		prob[i] = sum;
	}

	int rand_num = ::std::rand() % sum;
	for (int i = 0; i < size; ++i) {
		JUDGE_CONTINUE(prob[i] > 0);
		if (rand_num <= prob[i]) {
			dice_num = i + 1;
			break;
		}
	}

	if (max_length - this->treasures_detail_.game_index_ < dice_num)
		dice_num = max_length - this->treasures_detail_.game_index_;
	return dice_num;
}

int MLTreasures::player_dice_begin(Message* msg) {
	MSG_DYNAMIC_CAST_RETURN(Proto11405012*, request, -1);

	Proto31400052 inner;
	inner.set_type(request->type());
	inner.set_is_reset(false);
	return MAP_MONITOR->dispatch_to_logic(this, &inner);
}

int MLTreasures::player_dice_end(Message* msg) {
	MSG_DYNAMIC_CAST_RETURN(Proto31400052*, request, -1);
	Time_Value nowtime = Time_Value::gettimeofday();
	CONDITION_NOTIFY_RETURN(this->treasures_detail_.req_tick_ <= nowtime,
			RETURN_FETCH_TREASURES_DICE, ERROR_OPERATE_TOO_FAST);
	this->treasures_detail_.req_tick_ = nowtime
			+ GameCommon::fetch_time_value(1);

	int type = request->type();
	int mult = request->mult();
	const Json::Value &base_json = CONFIG_INSTANCE->treasures_base_json(1);
	int max_length = base_json["length"].asInt();
	this->treasures_detail_.game_length_ = max_length;
	Money money = GameCommon::make_up_money(base_json["play_gold"].asInt(),
			GameEnum::MONEY_UNBIND_GOLD);
	int left_length = max_length - this->treasures_detail_.game_index_;

	CONDITION_NOTIFY_RETURN(left_length > 0, RETURN_FETCH_TREASURES_DICE,
			ERROR_CLIENT_OPERATE);

	//剩余免费次数
	int free_times = base_json["free_times"].asInt()
			- this->treasures_detail_.free_times_;

	if (type == 1 && free_times == 0) {
		int item_id = base_json["special_good"].asInt();
		int item_num = 1;
		//物品足够
		GamePackage *package = this->find_package(GameEnum::INDEX_PACKAGE);
		JUDGE_RETURN(NULL != package, -1);

		while (package->count_by_id(item_id) >= item_num && money.__gold > 0) {
			Money discount = GameCommon::make_up_money(
					base_json["good_gold"].asInt(),
					GameEnum::MONEY_UNBIND_GOLD);
			money -= discount;
			this->pack_remove(package, SerialObj(ITEM_TREASURES_DISCOUNT),
					item_id, item_num);
		}
	}

	CONDITION_NOTIFY_RETURN(
			true == GameCommon::validate_money(money) || free_times > 0
					|| money.__gold == 0, RETURN_FETCH_TREASURES_DICE,
			ERROR_PACKAGE_GOLD_AMOUNT);

	if (free_times > 0) {
		this->treasures_detail_.free_times_++;
	} else {
		if (money.__gold > 0) {
			GameCommon::adjust_money(money, this->own_money());
			int ret = this->pack_money_sub(money, SUB_MONEY_TREASURES);
			CONDITION_NOTIFY_RETURN(0 == ret, RETURN_FETCH_TREASURES_DICE,
					ERROR_PACKAGE_MONEY_AMOUNT);
		}
	}

	int dice_num = this->rand_dice();

	Proto51405012 respond;
	respond.set_dice_num(dice_num);

	this->respond_to_client(RETURN_FETCH_TREASURES_DICE, &respond);

	for (int i = 1; i <= dice_num; ++i) {
		this->treasures_detail_.game_index_++;
		this->check_treasures_reward(this->treasures_detail_.game_index_, mult);
	}

	if (this->treasures_detail_.game_index_
			== this->treasures_detail_.game_length_)
		this->notify_treasures_all_reward();

	return this->notify_treasures_info();
}

int MLTreasures::treasures_info_reset() {
	this->treasures_detail_.reset();
	this->notify_treasures_info();
	return 0;
}

int MLTreasures::fetch_act_mult(Message* msg) {
	MSG_DYNAMIC_CAST_RETURN(Proto31400052*, request, -1);

	if (request->is_reset() == false)
		return this->player_dice_end(msg);
	else
		return this->player_reset_treasures_game_end(msg);
}

int MLTreasures::player_reset_treasures_game_begin() {
	Proto31400052 inner;
	inner.set_is_reset(true);
	return MAP_MONITOR->dispatch_to_logic(this, &inner);
}

int MLTreasures::player_reset_treasures_game_end(Message* msg) {
	MSG_DYNAMIC_CAST_RETURN(Proto31400052*, request, -1);

	const Json::Value &base_json = CONFIG_INSTANCE->treasures_base_json(1);
	Money money = GameCommon::make_up_money(base_json["reset_gold"].asInt(),
			GameEnum::MONEY_UNBIND_GOLD);
	int reward_id = base_json["reset_reward"].asInt();
	int reset_times =
			base_json["reset_times"][this->vip_detail().__vip_level].asInt()
					- this->treasures_detail_.reset_times_;

	CONDITION_NOTIFY_RETURN(
			true == GameCommon::validate_money(money) && reset_times > 0,
			RETURN_FETCH_TREASURES_RESET, ERROR_SERVER_INNER);

	GameCommon::adjust_money(money, this->own_money());
	int ret = this->pack_money_sub(money, SUB_MONEY_TREASURES);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_FETCH_TREASURES_RESET,
			ERROR_PACKAGE_MONEY_AMOUNT);

	const Json::Value& reward_json = CONFIG_INSTANCE->reward(reward_id);
	CONDITION_NOTIFY_RETURN(reward_json.empty() == false,
			RETURN_FETCH_TREASURES_RESET, ERROR_CONFIG_NOT_EXIST);

	RewardInfo reward_info;
	GameCommon::make_up_reward_items(reward_info, reward_json);

	if (request->mult() > 0) {
		for (ItemObjVec::iterator it = reward_info.item_vec_.begin();
				it != reward_info.item_vec_.end(); ++it) {
			it->amount_ *= 2;
		}
	}
	MapLogicPlayer* player = dynamic_cast<MapLogicPlayer *>(this);
	player->insert_package(ITEM_TREASURES_DISCOUNT, reward_info,
			this->role_id());

//	this->add_reward(reward_id, SerialObj(ITEM_TREASURES_DISCOUNT));
	this->treasures_detail_.game_index_ = 1;
	this->treasures_detail_.reset_times_++;
	this->treasures_detail_.reset_tick_ = Time_Value::gettimeofday().sec();
	this->treasures_detail_.item_list_.clear();

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_TREASURES_INFO);
	this->notify_treasures_info();
	FINER_PROCESS_NOTIFY(RETURN_FETCH_TREASURES_RESET);
}

int MLTreasures::fetch_treasures_info() {
//	MSG_DYNAMIC_CAST_RETURN(Proto11405011*, request, -1);
	return this->notify_treasures_info();
}

int MLTreasures::notify_treasures_all_reward() {
//	const Json::Value &base_json = CONFIG_INSTANCE->treasures_base_json(1);
//	int reward_id = base_json["reset_reward"].asInt();

//	this->add_reward(reward_id, SerialObj(ITEM_TREASURES_DISCOUNT));

	Proto81405013 respond;
	respond.set_reward_id(0);

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_TREASURES_INFO);
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_TREASURES_ALL_REWARD, &respond);
}

int MLTreasures::notify_treasures_info() {
	const Json::Value &base_json = CONFIG_INSTANCE->treasures_base_json(1);
	this->treasures_detail_.game_length_ = base_json["length"].asInt();
	//剩余免费次数
	int free_times = base_json["free_times"].asInt() - this->treasures_detail_.free_times_;
//	int reset_times = base_json["reset_times"][this->vip_detail().__vip_level].asInt()
//			- this->treasures_detail_.reset_times_;

	Proto81405011 respond;
	respond.set_reset_tick(this->treasures_detail_.reset_tick_);
	respond.set_reset_times(this->treasures_detail_.reset_times_);
	respond.set_free_times(free_times);
	respond.set_game_index(this->treasures_detail_.game_index_);
	respond.set_game_length(this->treasures_detail_.game_length_);

	for (ItemObjVec::iterator it = this->treasures_detail_.item_list_.begin();
			it != this->treasures_detail_.item_list_.end(); ++it)
	{
		ProtoItem* item_obj = respond.add_item_list();
		it->serialize(item_obj);
	}

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_TREASURES_INFO);
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_TREASURES_INFO, &respond);
}

int MLTreasures::check_treasures_reward(int index, int mult) {
	Proto81405012 respond;
	const Json::Value &grid_json = CONFIG_INSTANCE->treasures_grid_json(index);
	int rand_num = ::std::rand() % RAND_BASE_NUM;

	int prob_index = this->treasures_detail_.reset_times_;
	if ((int) grid_json["get_prob"].size() <= prob_index)
		prob_index = (int) grid_json["get_prob"].size() - 1;

	int get_prob = grid_json["get_prob"][prob_index].asInt();
	int double_prob = grid_json["double_prob"][prob_index].asInt();
	int reward_id = grid_json["reward_id"][prob_index].asInt();
	if (rand_num <= get_prob && get_prob != 0) {
		respond.set_is_get(1);
		int double_num = ::std::rand() % RAND_BASE_NUM;
		if (double_num <= double_prob && double_prob != 0) {
			respond.set_is_double(1);
		} else {
			respond.set_is_double(0);
		}
	} else {
		respond.set_is_get(0);
		respond.set_is_double(0);
	}

	if (respond.is_get()) {
		const Json::Value& reward_json = CONFIG_INSTANCE->reward(reward_id);
		if (reward_json.empty() == false)
		{

			RewardInfo reward_info;
			GameCommon::make_up_reward_items(reward_info, reward_json);

			if (mult > 0)
			{
				for (ItemObjVec::iterator it = reward_info.item_vec_.begin();
						it != reward_info.item_vec_.end(); ++it)
				{
					it->amount_ *= 2;
				}
			}

			MapLogicPlayer* player = dynamic_cast<MapLogicPlayer *>(this);
			player->insert_package(ADD_FROM_TREASURES_GAME, reward_info, this->role_id());
			if (respond.is_double())
			player->insert_package(ADD_FROM_TREASURES_GAME, reward_info, this->role_id());

			//物品
			for (ItemObjVec::iterator it = reward_info.item_vec_.begin(); it != reward_info.item_vec_.end(); ++it)
			{
				ItemObj& obj = (*it);
				if (respond.is_double()) obj.amount_ *= 2;
				ProtoItem* item_obj = respond.add_item_list();
				obj.serialize(item_obj);

				this->treasures_detail_.item_list_.push_back(obj);
			}
		}
	}

	respond.set_gird_index(index);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_TREASURES_INFO);
	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_TREASURES_REWARD, &respond);
}

int MLTreasures::sync_transfer_treasures_info(int scene_id) {
	Proto31400164 temp_info;

	temp_info.set_free_times(this->treasures_detail_.free_times_);
	temp_info.set_game_index(this->treasures_detail_.game_index_);
	temp_info.set_game_length(this->treasures_detail_.game_length_);
	temp_info.set_reset_tick(this->treasures_detail_.reset_tick_);
	temp_info.set_reset_times(this->treasures_detail_.reset_times_);

	for (ItemObjVec::iterator it = this->treasures_detail_.item_list_.begin();
			it != this->treasures_detail_.item_list_.end(); ++it) {
		ItemObj& obj = (*it);
		ProtoItem* item_obj = temp_info.add_item_list();
		obj.serialize(item_obj);
	}

	return this->send_to_other_logic_thread(scene_id, temp_info);
}

int MLTreasures::read_transfer_treasures_info(Message* msg) {
	MSG_DYNAMIC_CAST_RETURN(Proto31400164*, request, -1);

	this->treasures_detail_.free_times_ = request->free_times();
	this->treasures_detail_.game_index_ = request->game_index();
	this->treasures_detail_.game_length_ = request->game_length();
	this->treasures_detail_.reset_tick_ = request->reset_tick();
	this->treasures_detail_.reset_times_ = request->reset_times();

	for (int i = 0; i < request->item_list_size(); ++i) {
		ProtoItem temp = request->item_list(i);
		ItemObj item;
		item.unserialize(temp);
		this->treasures_detail_.item_list_.push_back(item);
	}

	return 0;
}

