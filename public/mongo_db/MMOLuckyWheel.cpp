/*
 * MMOLuckyWheel.cpp
 *
 *  Created on: 2016年12月14日
 *      Author: lyw
 */

#include "MMOLuckyWheel.h"
#include "GameField.h"
#include "BackField.h"
#include "MongoDataMap.h"
#include "MongoConnector.h"
#include "LogicPlayer.h"
#include "DBCommon.h"
#include "LuckyWheelSys.h"
#include "DailyActSys.h"

#include <mongo/client/dbclient.h>

MMOLuckyWheel::MMOLuckyWheel() {
	// TODO Auto-generated constructor stub

}

MMOLuckyWheel::~MMOLuckyWheel() {
	// TODO Auto-generated destructor stub
}

void MMOLuckyWheel::ensure_all_index(void)
{
	this->conection().ensureIndex(DBLuckyWheel::COLLECTION,
			BSON(DBLuckyWheel::ID << 1), true);
	this->conection().ensureIndex(DBLuckyWheelSys::COLLECTION,
			BSON(DBLuckyWheelSys::ID << 1));
	this->conection().ensureIndex(DBLuckyWheelSys::COLLECTION,
			BSON(DBLuckyWheelSys::ACT_TYPE << 1), true);
	this->conection().ensureIndex(DBBackWonderfulActivity::COLLECTION,
			BSON(DBBackWonderfulActivity::ACTIVITY_ID << 1));
	this->conection().ensureIndex(DBBackWonderfulActivity::COLLECTION,
			BSON(DBBackWonderfulActivity::ACTIVITY_TYPE << 1), true);
	this->conection().ensureIndex(DBLuckyWheelSys::COLLECTION,
			BSON(DBDailyActSys::ACT_TYPE << 1), true);
}

int MMOLuckyWheel::load_lucky_wheel_activity(LogicPlayer *player)
{
	BSONObj res = this->conection().findOne(DBLuckyWheel::COLLECTION,
			QUERY(DBLuckyWheel::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	WheelPlayerInfo &wheel_info = player->wheel_player_info();
	BSONObjIterator iter(res.getObjectField(DBLuckyWheel::ACTIVITY_DETAIL.c_str()));
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();
		int activity_id = obj[DBLuckyWheel::ActivityDetail::ACTIVITY_ID].numberInt();
		WheelPlayerInfo::PlayerDetail &player_detail = wheel_info.player_detail_map_[activity_id];
		player_detail.activity_id_ = activity_id;
		player_detail.act_score_   = obj[DBLuckyWheel::ActivityDetail::ACT_SCORE].numberInt();
		player_detail.wheel_times_ = obj[DBLuckyWheel::ActivityDetail::WHEEL_TIMES].numberInt();
		player_detail.reset_times_ = obj[DBLuckyWheel::ActivityDetail::RESET_TIMES].numberInt();
		player_detail.login_tick_  = obj[DBLuckyWheel::ActivityDetail::LOGIN_TICK].numberInt();
		player_detail.use_free_    = obj[DBLuckyWheel::ActivityDetail::USE_FREE].numberInt();
		player_detail.label_get_   = obj[DBLuckyWheel::ActivityDetail::LABEL_GET].numberInt();
		player_detail.rank_get_    = obj[DBLuckyWheel::ActivityDetail::RANK_GET].numberInt();
		player_detail.reward_get_  = obj[DBLuckyWheel::ActivityDetail::REWARD_GET].numberInt();
		player_detail.nine_word_reward_ = obj[DBLuckyWheel::ActivityDetail::NINE_WORD_REWARD].numberInt();
		player_detail.is_first_    = obj[DBLuckyWheel::ActivityDetail::IS_FIRST].numberInt();
		player_detail.open_times_  = obj[DBLuckyWheel::ActivityDetail::OPEN_TIMES].numberInt();
		player_detail.reset_tick_  = obj[DBLuckyWheel::ActivityDetail::RESET_TICK].numberLong();
		player_detail.combine_reset_ = obj[DBLuckyWheel::ActivityDetail::COMBINE_RESET].numberInt();

		GameCommon::bson_to_map(player_detail.rebate_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::REBATE_MAP.c_str()));

		BSONObjIterator it_item(obj.getObjectField(DBLuckyWheel::ActivityDetail::ITEM_RECORD.c_str()));
		while (it_item.more())
		{
			BSONObj item_obj = it_item.next().embeddedObject();
			JUDGE_CONTINUE(item_obj.isEmpty() == false);

			WheelPlayerInfo::ItemRecord record;
			record.item_bind_ 		= item_obj[DBLuckyWheel::ActivityDetail::ItemRecord::ITEM_BIND].numberInt();
			record.amount_	  		= item_obj[DBLuckyWheel::ActivityDetail::ItemRecord::AMOUNT].numberInt();
			record.get_time_  		= item_obj[DBLuckyWheel::ActivityDetail::ItemRecord::GET_TIME].numberLong();
			record.item_id_   		= item_obj[DBLuckyWheel::ActivityDetail::ItemRecord::ITEM_ID].numberInt();
			record.reward_mult_		= item_obj[DBLuckyWheel::ActivityDetail::ItemRecord::REWARD_MULT].numberInt();
			record.sub_value_ 	    = item_obj[DBLuckyWheel::ActivityDetail::ItemRecord::SUB_VALUE].numberInt();
			player_detail.item_record_.push_back(record);
		}

		BSONObjIterator it_slot(obj.getObjectField(DBLuckyWheel::ActivityDetail::PERSON_SLOT_SET.c_str()));
		while (it_slot.more())
		{
			BSONObj slot_obj = it_slot.next().embeddedObject();
			JUDGE_CONTINUE(slot_obj.isEmpty() == false);

			WheelPlayerInfo::PersonSlot person_slot;
			person_slot.time_point_ = slot_obj[DBLuckyWheel::ActivityDetail::PersonSlotSet::TIME_POINT].numberInt();
			person_slot.slot_id_ = slot_obj[DBLuckyWheel::ActivityDetail::PersonSlotSet::SLOT_ID].numberInt();
			person_slot.buy_times_ = slot_obj[DBLuckyWheel::ActivityDetail::PersonSlotSet::BUY_TIMES].numberInt();
			person_slot.is_color_ = slot_obj[DBLuckyWheel::ActivityDetail::PersonSlotSet::IS_COLOR].numberInt();

			ItemObj item_obj;
			item_obj.id_ 	= slot_obj[DBItemObj::ID].numberInt();
			item_obj.amount_= slot_obj[DBItemObj::AMOUNT].numberInt();
			item_obj.bind_ 	= slot_obj[DBItemObj::BIND].numberInt();
			person_slot.item_ 	= item_obj;

			DBCommon::bson_to_int_map(person_slot.nine_slot_,
					slot_obj.getObjectField(DBLuckyWheel::ActivityDetail::PersonSlotSet::NINE_SLOT.c_str()));

			player_detail.person_slot_set_.push_back(person_slot);
		}

		DBCommon::bson_to_int_vec(player_detail.reward_location_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::REWARD_LOCATION.c_str()));

		//藏珍阁数据
		player_detail.refresh_tick_ = obj[DBLuckyWheel::ActivityDetail::REFRESH_TICK].numberLong();
		IntVec refresh_times_vec;
		DBCommon::bson_to_int_vec(refresh_times_vec,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::REFRESH_TIMES_MAP.c_str()));
		for (int i = 0; i < (int)refresh_times_vec.size(); ++i)
		{
			player_detail.group_refresh_times_map_[i] = refresh_times_vec[i] - 100;
		}

		IntVec refresh_reward_vec;
		DBCommon::bson_to_int_vec(refresh_reward_vec,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::REFRESH_REWARD_MAP.c_str()));
		for (int i = 0; i < (int)refresh_reward_vec.size(); ++i)
		{
			player_detail.refresh_reward_map_[i] = refresh_reward_vec[i] - 100;
		}

		BSONObjIterator fish_vec(obj.getObjectField(DBLuckyWheel::ActivityDetail::FISH_INFO_VEC.c_str()));
		player_detail.fish_info_vec_.clear();
		while(fish_vec.more())
		{
			BSONObj fish_obj = fish_vec.next().embeddedObject();
			JUDGE_CONTINUE(fish_obj.isEmpty() == false);
			FishInfo info;
			info.type_ = fish_obj[DBLuckyWheel::ActivityDetail::FISH_INFO::TYPE].numberInt();
			info.layer_ = fish_obj[DBLuckyWheel::ActivityDetail::FISH_INFO::LAYER].numberInt();
			info.flag_ = fish_obj[DBLuckyWheel::ActivityDetail::FISH_INFO::FLAG].numberInt();
			info.coord_.set_pos(fish_obj[DBLuckyWheel::ActivityDetail::FISH_INFO::POS_X].numberInt(),
					fish_obj[DBLuckyWheel::ActivityDetail::FISH_INFO::POS_Y].numberInt()
					);
			player_detail.fish_info_vec_.push_back(info);
		}

		GameCommon::bson_to_map(player_detail.fish_reward_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::FISH_REWARD.c_str()));

		int len = 0;
		BSONObjIterator it_shop_slot(obj.getObjectField(DBLuckyWheel::ActivityDetail::SHOP_SLOT_MAP.c_str()));
		while (it_shop_slot.more())
		{
			BSONObj shop_slot_obj = it_shop_slot.next().embeddedObject();
			JUDGE_CONTINUE(shop_slot_obj.isEmpty() == false);

			WheelPlayerInfo::ShopSlot &shop_slot = player_detail.shop_slot_map_[len];
			shop_slot.is_buy_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::IS_BUY].numberInt();
			shop_slot.is_cast_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::IS_CAST].numberInt();
			shop_slot.slot_id_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::SLOT_ID].numberInt();
			shop_slot.is_rarity_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::IS_RARITY].numberInt();
			shop_slot.item_price_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_PRICE].numberInt();
			shop_slot.item_price_pre_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_PRICE_PRE].numberInt();
			shop_slot.group_id_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::GROUP_ID].numberInt();
			shop_slot.day_ = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::DAY].numberInt();


			int item_id = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_ID].numberInt();
			int item_amount = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_AMOUNT].numberInt();
			int item_bind = shop_slot_obj[DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_BIND].numberInt();

			shop_slot.item_ = ItemObj(item_id, item_amount, item_bind);
			len ++;
		}

		//迷宫
		GameCommon::bson_to_map(player_detail.group_show_times_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::SHOW_TIMES_MAP.c_str()));

		GameCommon::bson_to_map(player_detail.group_show_times_map_fina_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::SHOW_TIMES_FINA_MAP.c_str()));

		GameCommon::bson_to_map(player_detail.free_times_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::FREE_TIMES_MAP.c_str()));

		GameCommon::bson_to_map(player_detail.slot_item_id_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::SLOT_ITEM_ID.c_str()));
		GameCommon::bson_to_map(player_detail.slot_item_num_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::SLOT_ITEM_NUM.c_str()));

		//鉴宝
		GameCommon::bson_to_map(player_detail.two_same_show_times_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::TWO_SHOW_MAP.c_str()));
		GameCommon::bson_to_map(player_detail.three_same_show_times_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::THREE_SHOW_MAP.c_str()));
		GameCommon::bson_to_map(player_detail.now_slot_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::NOW_SLOT_MAP.c_str()));
		GameCommon::bson_to_map(player_detail.fina_slot_map_,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::FINA_SLOT_MAP.c_str()));

		player_detail.slot_index_ = obj[DBLuckyWheel::ActivityDetail::SLOT_INDEX].numberInt();
		player_detail.slot_scale_ = obj[DBLuckyWheel::ActivityDetail::SLOT_SCALE].numberInt();
		player_detail.reward_scale_ = obj[DBLuckyWheel::ActivityDetail::REWARD_SCALE].numberInt();
		player_detail.maze_free_ = obj[DBLuckyWheel::ActivityDetail::MAZE_FREE].numberInt();
		player_detail.bless_ 	 = obj[DBLuckyWheel::ActivityDetail::BLESS].numberInt();

		player_detail.bless_value = obj[DBLuckyWheel::ActivityDetail::BLESS_VALUE].numberInt();
		GameCommon::bson_to_map(player_detail.reward_record_map,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::REWARD_RECORD_MAP.c_str()));
		GameCommon::bson_to_map(player_detail.exchange_item_frequency,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::EXCHANGE_ITEM_FREQUENCY.c_str()));
		GameCommon::bson_to_map(player_detail.bless_reward_frequency,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::BLESS_REWARD_FREQUENCY.c_str()));
		GameCommon::bson_to_map(player_detail.bless_reward_possess,
				obj.getObjectField(DBLuckyWheel::ActivityDetail::BLESS_REWARD_POSSESS.c_str()));
	}

	return 0;
}

int MMOLuckyWheel::update_data(LogicPlayer *player, MongoDataMap *mongo_data)
{
	WheelPlayerInfo &wheel_info = player->wheel_player_info();

	BSONVec wheel_vec;
	for (WheelPlayerInfo::PlayerDetailMap::iterator iter = wheel_info.player_detail_map_.begin();
			iter != wheel_info.player_detail_map_.end(); ++iter)
	{
		WheelPlayerInfo::PlayerDetail &player_detail = iter->second;

		BSONVec item_vec, slot_vec;
		for (WheelPlayerInfo::ItemRecordSet::iterator item_iter = player_detail.item_record_.begin();
				item_iter != player_detail.item_record_.end(); ++item_iter)
		{
			item_vec.push_back(BSON(DBLuckyWheel::ActivityDetail::ItemRecord::ITEM_BIND << item_iter->item_bind_
					<< DBLuckyWheel::ActivityDetail::ItemRecord::AMOUNT << item_iter->amount_
					<< DBLuckyWheel::ActivityDetail::ItemRecord::GET_TIME << item_iter->get_time_
					<< DBLuckyWheel::ActivityDetail::ItemRecord::ITEM_ID << item_iter->item_id_
					<< DBLuckyWheel::ActivityDetail::ItemRecord::REWARD_MULT << item_iter->reward_mult_
					<< DBLuckyWheel::ActivityDetail::ItemRecord::SUB_VALUE << item_iter->sub_value_
					));
		}

		for (WheelPlayerInfo::PersonSlotSet::iterator slot_iter = player_detail.person_slot_set_.begin();
				slot_iter != player_detail.person_slot_set_.end(); ++slot_iter)
		{
			BSONVec nine_slot_vec;
			DBCommon::int_map_to_bson(nine_slot_vec, slot_iter->nine_slot_);

			slot_vec.push_back(BSON(DBLuckyWheel::ActivityDetail::PersonSlotSet::TIME_POINT << slot_iter->time_point_
					<< DBLuckyWheel::ActivityDetail::PersonSlotSet::SLOT_ID << slot_iter->slot_id_
					<< DBLuckyWheel::ActivityDetail::PersonSlotSet::BUY_TIMES << slot_iter->buy_times_
					<< DBLuckyWheel::ActivityDetail::PersonSlotSet::IS_COLOR << slot_iter->is_color_
					<< DBItemObj::ID << slot_iter->item_.id_
					<< DBItemObj::AMOUNT << slot_iter->item_.amount_
					<< DBItemObj::BIND << slot_iter->item_.bind_
					<< DBLuckyWheel::ActivityDetail::PersonSlotSet::NINE_SLOT << nine_slot_vec));
		}

		BSONVec shop_vec;
		for (WheelPlayerInfo::ShopSlotMap::iterator shop_iter = player_detail.shop_slot_map_.begin();
				shop_iter != player_detail.shop_slot_map_.end(); ++shop_iter)
		{
			WheelPlayerInfo::ShopSlot &shop_slot = shop_iter->second;
			shop_vec.push_back(BSON(DBLuckyWheel::ActivityDetail::ShopSlot::IS_RARITY <<  shop_slot.is_rarity_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::IS_BUY << shop_slot.is_buy_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::IS_CAST << shop_slot.is_cast_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::SLOT_ID << shop_slot.slot_id_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_PRICE << shop_slot.item_price_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_PRICE_PRE << shop_slot.item_price_pre_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::GROUP_ID << shop_slot.group_id_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::DAY << shop_slot.day_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_ID << shop_slot.item_.id_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_AMOUNT << shop_slot.item_.amount_
					<< DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_BIND << shop_slot.item_.bind_));
		}

		BSONVec fish_vec;
		for(uint i = 0; i < player_detail.fish_info_vec_.size(); ++i)
		{
			FishInfo &info = player_detail.fish_info_vec_[i];
			fish_vec.push_back(BSON(DBLuckyWheel::ActivityDetail::FISH_INFO::TYPE << info.type_
					<< DBLuckyWheel::ActivityDetail::FISH_INFO::LAYER << info.layer_
					<< DBLuckyWheel::ActivityDetail::FISH_INFO::FLAG << info.flag_
					<< DBLuckyWheel::ActivityDetail::FISH_INFO::POS_X << info.coord_.pos_x()
					<< DBLuckyWheel::ActivityDetail::FISH_INFO::POS_Y << info.coord_.pos_y()));
		}

		IntVec temp_1, temp_2;
		for (int i = 0; i < (int)player_detail.group_refresh_times_map_.size(); ++i)
		{
			temp_1.push_back(player_detail.group_refresh_times_map_[i] + 100);
		}
		for (int i = 0; i < (int) player_detail.refresh_reward_map_.size(); ++i)
		{
			temp_2.push_back(player_detail.refresh_reward_map_[i] + 100);
		}

		BSONVec fish_reward;
		GameCommon::map_to_bson(fish_reward, player_detail.fish_reward_map_);

		//迷宫
		BSONVec maze_set_1;
		GameCommon::map_to_bson(maze_set_1, player_detail.group_show_times_map_);
		BSONVec maze_set_2;
		GameCommon::map_to_bson(maze_set_2, player_detail.group_show_times_map_fina_);
		BSONVec maze_set_3;
		GameCommon::map_to_bson(maze_set_3, player_detail.free_times_map_);

		BSONVec maze_item_id;
		BSONVec maze_item_num;
		GameCommon::map_to_bson(maze_item_id, player_detail.slot_item_id_);
		GameCommon::map_to_bson(maze_item_num, player_detail.slot_item_num_);

		//鉴宝
		BSONVec two_show_map, three_show_map, now_slot_map, fina_slot_map;
		GameCommon::map_to_bson(two_show_map, player_detail.two_same_show_times_map_);
		GameCommon::map_to_bson(three_show_map, player_detail.three_same_show_times_map_);
		GameCommon::map_to_bson(now_slot_map, player_detail.now_slot_map_, true);
		GameCommon::map_to_bson(fina_slot_map, player_detail.fina_slot_map_);

		BSONVec rebate_vec;
		GameCommon::map_to_bson(rebate_vec, player_detail.rebate_map_);

		BSONVec reward_record_map, exchange_item_fre_map, bless_reward_fre_map, bless_reward_pos_map;
		GameCommon::map_to_bson(reward_record_map, player_detail.reward_record_map);
		GameCommon::map_to_bson(exchange_item_fre_map, player_detail.exchange_item_frequency);
		GameCommon::map_to_bson(bless_reward_fre_map, player_detail.bless_reward_frequency);
		GameCommon::map_to_bson(bless_reward_pos_map, player_detail.bless_reward_possess);

		wheel_vec.push_back(BSON(DBLuckyWheel::ActivityDetail::ACTIVITY_ID << iter->first
				<< DBLuckyWheel::ActivityDetail::ACT_SCORE << player_detail.act_score_
				<< DBLuckyWheel::ActivityDetail::WHEEL_TIMES << player_detail.wheel_times_
				<< DBLuckyWheel::ActivityDetail::RESET_TIMES << player_detail.reset_times_
				<< DBLuckyWheel::ActivityDetail::LOGIN_TICK << player_detail.login_tick_
				<< DBLuckyWheel::ActivityDetail::USE_FREE << player_detail.use_free_
				<< DBLuckyWheel::ActivityDetail::LABEL_GET << player_detail.label_get_
				<< DBLuckyWheel::ActivityDetail::RANK_GET << player_detail.rank_get_
				<< DBLuckyWheel::ActivityDetail::REWARD_GET << player_detail.reward_get_
				<< DBLuckyWheel::ActivityDetail::NINE_WORD_REWARD << player_detail.nine_word_reward_
				<< DBLuckyWheel::ActivityDetail::IS_FIRST << player_detail.is_first_
				<< DBLuckyWheel::ActivityDetail::OPEN_TIMES << player_detail.open_times_
				<< DBLuckyWheel::ActivityDetail::RESET_TICK << player_detail.reset_tick_
				<< DBLuckyWheel::ActivityDetail::COMBINE_RESET << player_detail.combine_reset_
				<< DBLuckyWheel::ActivityDetail::REBATE_MAP << rebate_vec
				<< DBLuckyWheel::ActivityDetail::ITEM_RECORD << item_vec
				<< DBLuckyWheel::ActivityDetail::PERSON_SLOT_SET << slot_vec
				<< DBLuckyWheel::ActivityDetail::REWARD_LOCATION << player_detail.reward_location_
				<< DBLuckyWheel::ActivityDetail::REFRESH_TIMES_MAP << temp_1
				<< DBLuckyWheel::ActivityDetail::REFRESH_REWARD_MAP << temp_2
				<< DBLuckyWheel::ActivityDetail::SHOP_SLOT_MAP << shop_vec
				<< DBLuckyWheel::ActivityDetail::REFRESH_TICK << player_detail.refresh_tick_
				<< DBLuckyWheel::ActivityDetail::SLOT_INDEX << player_detail.slot_index_
				<< DBLuckyWheel::ActivityDetail::SLOT_SCALE << player_detail.slot_scale_
				<< DBLuckyWheel::ActivityDetail::REWARD_SCALE << player_detail.reward_scale_
				<< DBLuckyWheel::ActivityDetail::MAZE_FREE << player_detail.maze_free_
				<< DBLuckyWheel::ActivityDetail::BLESS << player_detail.bless_
				<< DBLuckyWheel::ActivityDetail::SHOW_TIMES_MAP << maze_set_1
				<< DBLuckyWheel::ActivityDetail::SHOW_TIMES_FINA_MAP << maze_set_2
				<< DBLuckyWheel::ActivityDetail::FREE_TIMES_MAP << maze_set_3
				<< DBLuckyWheel::ActivityDetail::SLOT_ITEM_ID << maze_item_id
				<< DBLuckyWheel::ActivityDetail::SLOT_ITEM_NUM << maze_item_num
				<< DBLuckyWheel::ActivityDetail::TWO_SHOW_MAP << two_show_map
				<< DBLuckyWheel::ActivityDetail::THREE_SHOW_MAP << three_show_map
				<< DBLuckyWheel::ActivityDetail::NOW_SLOT_MAP << now_slot_map
				<< DBLuckyWheel::ActivityDetail::FINA_SLOT_MAP << fina_slot_map
				<< DBLuckyWheel::ActivityDetail::FISH_INFO_VEC << fish_vec
				<< DBLuckyWheel::ActivityDetail::FISH_REWARD << fish_reward
				<< DBLuckyWheel::ActivityDetail::BLESS_VALUE << player_detail.bless_value
				<< DBLuckyWheel::ActivityDetail::REWARD_RECORD_MAP << reward_record_map
				<< DBLuckyWheel::ActivityDetail::EXCHANGE_ITEM_FREQUENCY << exchange_item_fre_map
				<< DBLuckyWheel::ActivityDetail::BLESS_REWARD_FREQUENCY << bless_reward_fre_map
				<< DBLuckyWheel::ActivityDetail::BLESS_REWARD_POSSESS << bless_reward_pos_map
		));
	}

	BSONObjBuilder builder;
	builder << DBLuckyWheel::ACTIVITY_DETAIL << wheel_vec;
	mongo_data->push_update(DBLuckyWheel::COLLECTION,
	    	BSON(DBLuckyWheel::ID << player->role_id()), builder.obj());

	return 0;
}

void MMOLuckyWheel::load_lucky_wheel_system(LuckyWheelSys* sys)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBLuckyWheelSys::COLLECTION);
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		LuckyWheelActivity::ActivityDetail act_detail;
		act_detail.activity_id_ = res[DBLuckyWheelSys::ID].numberInt();
		int find_flag = sys->fetch_act_base_conf(act_detail);
		JUDGE_CONTINUE(find_flag == true);

		act_detail.time_slot_map_.clear();
		act_detail.act_type_ 	  	= res[DBLuckyWheelSys::ACT_TYPE].numberInt();
		act_detail.save_gold_ 	  	= res[DBLuckyWheelSys::SAVE_GOLD].numberInt();
		act_detail.save_date_type_  = res[DBLuckyWheelSys::DATE_TYPE].numberInt();
		act_detail.save_first_date_ = res[DBLuckyWheelSys::FIRST_DATE].numberLong();
		act_detail.save_last_date_  = res[DBLuckyWheelSys::LAST_DATE].numberLong();
		act_detail.is_combine_reset_= res[DBLuckyWheelSys::IS_COMBINE_RESET].numberInt();
		act_detail.reset_tick_ 	  	= res[DBLuckyWheelSys::RESET_TICK].numberLong();
		act_detail.refresh_times_	= res[DBLuckyWheelSys::REFRESH_TIMES].numberInt();
		act_detail.finish_all_times_ = res[DBLuckyWheelSys::ALL_FINISH_TIMES].numberLong();

		int save_day = res[DBLuckyWheelSys::SAVE_DAY].numberInt();

		BSONObjIterator iter_item = res.getObjectField(DBLuckyWheelSys::ITEM_SET.c_str());
		while (iter_item.more())
		{
			BSONObj iter_res = iter_item.next().embeddedObject();
			JUDGE_CONTINUE(iter_res.isEmpty() == false);

			LuckyWheelActivity::ServerItemInfo item_info;
			item_info.player_id_ = iter_res[DBLuckyWheelSys::ServerItemSet::PLAYER_ID].numberLong();
			item_info.player_name_ = iter_res[DBLuckyWheelSys::ServerItemSet::PLAYER_NAME].str();
			item_info.get_time_ = iter_res[DBLuckyWheelSys::ServerItemSet::GET_TIME].numberLong();
			item_info.item_bind_ = iter_res[DBLuckyWheelSys::ServerItemSet::ITEM_BIND].numberInt();
			item_info.amount_ = iter_res[DBLuckyWheelSys::ServerItemSet::AMOUNT].numberInt();
			item_info.item_id_ = iter_res[DBLuckyWheelSys::ServerItemSet::ITEM_ID].numberInt();
			item_info.reward_mult_ = iter_res[DBLuckyWheelSys::ServerItemSet::REWARD_MULT].numberInt();
			item_info.sub_value_ = iter_res[DBLuckyWheelSys::ServerItemSet::SUB_VALUE].numberInt();
			act_detail.item_set_.push_back(item_info);
		}

		BSONObjIterator iter_rank_num = res.getObjectField(DBLuckyWheelSys::RANK_NUM_SET.c_str());
		while (iter_rank_num.more())
		{
			BSONObj iter_res = iter_rank_num.next().embeddedObject();
			JUDGE_CONTINUE(iter_res.isEmpty() == false);

			Int64 role_id = iter_res[DBLuckyWheelSys::OneRankSet::ROLE_ID].numberLong();
			LuckyWheelActivity::OneRankInfo &rank_info = act_detail.rank_num_map_[role_id];
			rank_info.add_rank_item(iter_res);
		}
		act_detail.sort_player_rank(LuckyWheelActivity::RANK_TYPE_NUM);

		BSONObjIterator iter_rank_lucky = res.getObjectField(DBLuckyWheelSys::RANK_LUCKY_SET.c_str());
		while (iter_rank_lucky.more())
		{
			BSONObj iter_res = iter_rank_lucky.next().embeddedObject();
			JUDGE_CONTINUE(iter_res.isEmpty() == false);

			Int64 role_id = iter_res[DBLuckyWheelSys::OneRankSet::ROLE_ID].numberLong();
			LuckyWheelActivity::OneRankInfo &rank_info = act_detail.rank_lucky_map_[role_id];
			rank_info.add_rank_item(iter_res);
		}
		act_detail.sort_player_rank(LuckyWheelActivity::RANK_TYPE_LUCKY);

		BSONObjIterator iter_mail = res.getObjectField(DBLuckyWheelSys::ROLE_MAIL_SET.c_str());
		while(iter_mail.more())
		{
			BSONObj iter_res = iter_mail.next().embeddedObject();
			JUDGE_CONTINUE(iter_res.isEmpty() == false);

			Int64 role_id = iter_res[DBLuckyWheelSys::RoleMailSet::ROLE_ID].numberLong();
			LuckyWheelActivity::RoleMailInfo &role_mail_info = act_detail.role_mail_map_[role_id];
			role_mail_info.role_id_ = role_id;

			DBCommon::bson_to_int_map(role_mail_info.reward_map_,
					iter_res.getObjectField(DBLuckyWheelSys::RoleMailSet::REWARD_SET.c_str()));
		}

		BSONObjIterator iter_gem_mail = res.getObjectField(DBLuckyWheelSys::GEM_ROLE_MAIL_SET.c_str());
		while(iter_gem_mail.more())
		{
			BSONObj iter_res = iter_gem_mail.next().embeddedObject();
			JUDGE_CONTINUE(iter_res.isEmpty() == false);

			Int64 role_id = iter_res[DBLuckyWheelSys::GemRoleMailSet::ROLE_ID].numberLong();
			LuckyWheelActivity::RoleMailInfo &role_mail_info = act_detail.gem_role_mail_map_[role_id];

			DBCommon::bson_to_int_map(role_mail_info.reward_map_,
					iter_res.getObjectField(DBLuckyWheelSys::GemRoleMailSet::REWARD_SET.c_str()));
		}

		BSONObjIterator iter_bless_reward = res.getObjectField(DBLuckyWheelSys::BLESS_REWARD_SET.c_str());
		while (iter_bless_reward.more())
		{
			BSONObj iter_res = iter_bless_reward.next().embeddedObject();
			JUDGE_CONTINUE(iter_res.isEmpty() == false);

			LuckyWheelActivity::ServerItemInfo item_info;
			item_info.player_id_ = iter_res[DBLuckyWheelSys::ServerItemSet::PLAYER_ID].numberLong();
			item_info.player_name_ = iter_res[DBLuckyWheelSys::ServerItemSet::PLAYER_NAME].str();
			item_info.get_time_ = iter_res[DBLuckyWheelSys::ServerItemSet::GET_TIME].numberLong();
			item_info.item_bind_ = iter_res[DBLuckyWheelSys::ServerItemSet::ITEM_BIND].numberInt();
			item_info.amount_ = iter_res[DBLuckyWheelSys::ServerItemSet::AMOUNT].numberInt();
			item_info.item_id_ = iter_res[DBLuckyWheelSys::ServerItemSet::ITEM_ID].numberInt();
			item_info.reward_mult_ = iter_res[DBLuckyWheelSys::ServerItemSet::REWARD_MULT].numberInt();
			item_info.sub_value_ = iter_res[DBLuckyWheelSys::ServerItemSet::SUB_VALUE].numberInt();
			act_detail.bless_reward_set_.push_back(item_info);
		}

		LuckyWheelActivity::SlotInfoMap *slot_info_map = sys->fetch_slot_map(&act_detail, save_day);
		if (slot_info_map != NULL)
		{
			BSONObjIterator iter_slot = res.getObjectField(DBLuckyWheelSys::SLOT_SET.c_str());
			while (iter_slot.more())
			{
				BSONObj iter_res = iter_slot.next().embeddedObject();
				JUDGE_CONTINUE(iter_res.isEmpty() == false);

				int time_point = iter_res[DBLuckyWheelSys::SlotSet::TIME_POINT].numberInt();
				int slot_id = iter_res[DBLuckyWheelSys::SlotSet::SLOT_ID].numberInt();
				int server_buy = iter_res[DBLuckyWheelSys::SlotSet::SERVER_BUY].numberInt();

				LuckyWheelActivity::SlotInfoMap::iterator it = slot_info_map->find(slot_id);
				JUDGE_CONTINUE(it != slot_info_map->end());

				LuckyWheelActivity::SlotInfo slot_info = it->second;
				slot_info.server_buy_ = server_buy;

				LuckyWheelActivity::SlotInfoMap &slot_map = act_detail.time_slot_map_[time_point];
				slot_map.insert(LuckyWheelActivity::SlotInfoMap::value_type(slot_id, slot_info));
			}
		}
		JUDGE_CONTINUE(sys->is_activity_time(&act_detail) == true);

		sys->act_detail_map_.insert(LuckyWheelActivity::ActivityDetailMap::value_type(
				act_detail.act_type_, act_detail));
	}
}

void MMOLuckyWheel::save_lucky_wheel_system(LuckyWheelSys* sys)
{
	for (LuckyWheelActivity::ActivityDetailMap::iterator iter = sys->act_detail_map_.begin();
			iter != sys->act_detail_map_.end(); ++iter)
	{
		LuckyWheelActivity::ActivityDetail &act_detail = iter->second;

		BSONVec item_set, rank_num_set, rank_lucky_set, mail_set, gem_mail_set, bless_reward_set, slot_set;
		for (LuckyWheelActivity::ServerItemSet::iterator item_iter = act_detail.item_set_.begin();
				item_iter != act_detail.item_set_.end(); ++item_iter)
		{
			LuckyWheelActivity::ServerItemInfo &item_info = *item_iter;
			item_set.push_back(BSON(DBLuckyWheelSys::ServerItemSet::PLAYER_ID << item_info.player_id_
					<< DBLuckyWheelSys::ServerItemSet::PLAYER_NAME << item_info.player_name_
					<< DBLuckyWheelSys::ServerItemSet::GET_TIME << item_info.get_time_
					<< DBLuckyWheelSys::ServerItemSet::ITEM_BIND << item_info.item_bind_
					<< DBLuckyWheelSys::ServerItemSet::AMOUNT << item_info.amount_
					<< DBLuckyWheelSys::ServerItemSet::ITEM_ID << item_info.item_id_
					<< DBLuckyWheelSys::ServerItemSet::REWARD_MULT << item_info.reward_mult_
					<< DBLuckyWheelSys::ServerItemSet::SUB_VALUE << item_info.sub_value_
					));
		}

		for (LuckyWheelActivity::RankNumMap::iterator rank_num_iter = act_detail.rank_num_map_.begin();
				rank_num_iter != act_detail.rank_num_map_.end(); ++rank_num_iter)
		{
			LuckyWheelActivity::OneRankInfo &rank_info = rank_num_iter->second;
			rank_num_set.push_back(BSON(DBLuckyWheelSys::OneRankSet::RANK << rank_info.rank_
					<< DBLuckyWheelSys::OneRankSet::TICK << rank_info.tick_
					<< DBLuckyWheelSys::OneRankSet::NUM << rank_info.num_
					<< DBLuckyWheelSys::OneRankSet::ROLE_ID << rank_info.role_id_
					<< DBLuckyWheelSys::OneRankSet::NAME << rank_info.name_));
		}

		for (LuckyWheelActivity::RankNumMap::iterator rank_lucky_iter = act_detail.rank_lucky_map_.begin();
				rank_lucky_iter != act_detail.rank_lucky_map_.end(); ++rank_lucky_iter)
		{
			LuckyWheelActivity::OneRankInfo &rank_info = rank_lucky_iter->second;
			rank_lucky_set.push_back(BSON(DBLuckyWheelSys::OneRankSet::RANK << rank_info.rank_
					<< DBLuckyWheelSys::OneRankSet::TICK << rank_info.tick_
					<< DBLuckyWheelSys::OneRankSet::NUM << rank_info.num_
					<< DBLuckyWheelSys::OneRankSet::ROLE_ID << rank_info.role_id_
					<< DBLuckyWheelSys::OneRankSet::NAME << rank_info.name_));
		}

		for (LuckyWheelActivity::RoleMailMap::iterator mail_iter = act_detail.role_mail_map_.begin();
				mail_iter != act_detail.role_mail_map_.end(); ++mail_iter)
		{
			LuckyWheelActivity::RoleMailInfo &role_mail_info = mail_iter->second;
			BSONVec reward_set;
			DBCommon::int_map_to_bson(reward_set, role_mail_info.reward_map_);

			mail_set.push_back(BSON(DBLuckyWheelSys::RoleMailSet::ROLE_ID << role_mail_info.role_id_
					<< DBLuckyWheelSys::RoleMailSet::REWARD_SET << reward_set));
		}

		for (LuckyWheelActivity::RoleMailMap::iterator mail_iter = act_detail.gem_role_mail_map_.begin();
				mail_iter != act_detail.gem_role_mail_map_.end(); ++mail_iter)
		{
			LuckyWheelActivity::RoleMailInfo &role_mail_info = mail_iter->second;
			BSONVec reward_set;
			DBCommon::int_map_to_bson(reward_set, role_mail_info.reward_map_);

			gem_mail_set.push_back(BSON(DBLuckyWheelSys::GemRoleMailSet::ROLE_ID << mail_iter->first
					<< DBLuckyWheelSys::GemRoleMailSet::REWARD_SET << reward_set));
		}

		for (LuckyWheelActivity::ServerItemSet::iterator item_iter = act_detail.bless_reward_set_.begin();
				item_iter != act_detail.bless_reward_set_.end(); ++item_iter)
		{
			LuckyWheelActivity::ServerItemInfo &item_info = *item_iter;
			bless_reward_set.push_back(BSON(DBLuckyWheelSys::ServerItemSet::PLAYER_ID << item_info.player_id_
					<< DBLuckyWheelSys::ServerItemSet::PLAYER_NAME << item_info.player_name_
					<< DBLuckyWheelSys::ServerItemSet::GET_TIME << item_info.get_time_
					<< DBLuckyWheelSys::ServerItemSet::ITEM_BIND << item_info.item_bind_
					<< DBLuckyWheelSys::ServerItemSet::AMOUNT << item_info.amount_
					<< DBLuckyWheelSys::ServerItemSet::ITEM_ID << item_info.item_id_
					<< DBLuckyWheelSys::ServerItemSet::REWARD_MULT << item_info.reward_mult_
					<< DBLuckyWheelSys::ServerItemSet::SUB_VALUE << item_info.sub_value_
					));
		}

		for (LuckyWheelActivity::ActivityDetail::TimeSlotInfoMap::iterator slot_iter = act_detail.time_slot_map_.begin();
				slot_iter != act_detail.time_slot_map_.end(); ++slot_iter)
		{
			LuckyWheelActivity::SlotInfoMap &slot_info_map = slot_iter->second;
			for (LuckyWheelActivity::SlotInfoMap::iterator it = slot_info_map.begin();
					it != slot_info_map.end(); ++it)
			{
				LuckyWheelActivity::SlotInfo &slot_info = it->second;
				slot_set.push_back(BSON(DBLuckyWheelSys::SlotSet::TIME_POINT << slot_iter->first
						<< DBLuckyWheelSys::SlotSet::SLOT_ID << it->first
						<< DBLuckyWheelSys::SlotSet::SERVER_BUY << slot_info.server_buy_));
			}
		}

		BSONObjBuilder builder;
		builder << DBLuckyWheelSys::ID << act_detail.activity_id_
				<< DBLuckyWheelSys::SAVE_GOLD << act_detail.save_gold_
				<< DBLuckyWheelSys::DATE_TYPE << act_detail.save_date_type_
				<< DBLuckyWheelSys::FIRST_DATE << act_detail.save_first_date_
				<< DBLuckyWheelSys::LAST_DATE << act_detail.save_last_date_
				<< DBLuckyWheelSys::IS_COMBINE_RESET << act_detail.is_combine_reset_
				<< DBLuckyWheelSys::RESET_TICK << act_detail.reset_tick_
				<< DBLuckyWheelSys::REFRESH_TIMES << act_detail.refresh_times_
				<< DBLuckyWheelSys::ALL_FINISH_TIMES << act_detail.finish_all_times_
				<< DBLuckyWheelSys::SAVE_DAY << sys->fetch_cur_day(act_detail.activity_id_)
				<< DBLuckyWheelSys::ITEM_SET << item_set
				<< DBLuckyWheelSys::RANK_NUM_SET << rank_num_set
				<< DBLuckyWheelSys::RANK_LUCKY_SET << rank_lucky_set
				<< DBLuckyWheelSys::ROLE_MAIL_SET << mail_set
				<< DBLuckyWheelSys::GEM_ROLE_MAIL_SET << gem_mail_set
				<< DBLuckyWheelSys::BLESS_REWARD_SET << bless_reward_set
				<< DBLuckyWheelSys::SLOT_SET << slot_set;

		GameCommon::request_save_mmo_begin(DBLuckyWheelSys::COLLECTION,
			    BSON(DBLuckyWheelSys::ID << act_detail.activity_id_
			    << DBLuckyWheelSys::ACT_TYPE << act_detail.act_type_),
			    BSON("$set" << builder.obj()));
	}
}

void MMOLuckyWheel::load_daily_act_system(DailyActSys* sys)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBDailyActSys::COLLECTION);
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		DailyActivity::DailyDetail daily_detail;
		daily_detail.activity_id_ = res[DBDailyActSys::ACTIVITY_ID].numberInt();
		int find_flag = sys->fetch_act_base_conf(daily_detail);
		JUDGE_CONTINUE(find_flag == true);

		daily_detail.act_type_ = res[DBDailyActSys::ACT_TYPE].numberInt();
		daily_detail.save_refresh_tick_ = res[DBDailyActSys::REFRESH_TICK].numberLong();
		daily_detail.reset_tick_ = res[DBDailyActSys::RESET_TICK].numberLong();
		daily_detail.save_first_date_ = res[DBDailyActSys::FIRST_DATE].numberLong();
		daily_detail.save_last_date_ = res[DBDailyActSys::LAST_DATE].numberLong();

		JUDGE_CONTINUE(daily_detail.is_activity_time(true) == true);

		sys->daily_detail_map_.insert(DailyActivity::DailyDetailMap::value_type(
				daily_detail.act_type_, daily_detail));
	}
}

void MMOLuckyWheel::save_daily_act_system(DailyActSys* sys)
{
	for (DailyActivity::DailyDetailMap::iterator iter = sys->daily_detail_map_.begin();
			iter != sys->daily_detail_map_.end(); ++iter)
	{
		DailyActivity::DailyDetail &daily_detail = iter->second;

		BSONObjBuilder builder;
		builder << DBDailyActSys::ACTIVITY_ID << daily_detail.activity_id_
				<< DBDailyActSys::FIRST_DATE << daily_detail.save_first_date_
				<< DBDailyActSys::LAST_DATE << daily_detail.save_last_date_
				<< DBDailyActSys::RESET_TICK << daily_detail.reset_tick_
				<< DBDailyActSys::REFRESH_TICK << daily_detail.save_refresh_tick_;

		GameCommon::request_save_mmo_begin(DBDailyActSys::COLLECTION,
				BSON(DBDailyActSys::ACT_TYPE << daily_detail.act_type_),
				BSON("$set" << builder.obj()));
	}
}

int MMOLuckyWheel::load_back_activity_info(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode->sub_value_ == 1 || shop_mode->sub_value_ == 2, 0);

	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBBackWonderfulActivity::COLLECTION, mongo::Query());
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		int activity_id = res[DBBackWonderfulActivity::ACTIVITY_ID].numberInt();
		if (shop_mode->sub_value_ == 1)
		{
			JUDGE_CONTINUE(GameCommon::is_operate_activity(activity_id));
		}
		else
		{
			JUDGE_CONTINUE(GameCommon::is_daily_activity(activity_id));
		}

		shop_mode->output_argv_.bson_vec_->push_back(res.copy());
	}

	return 0;
}
