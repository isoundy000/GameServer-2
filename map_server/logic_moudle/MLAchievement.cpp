/*
 * MLAchievement.cpp
 *
 *  Created on: 2014-1-17
 *      Author: louis
 */

#include "MLAchievement.h"
#include "PoolMonitor.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"


MLAchievement::MLAchievement() {
	// TODO Auto-generated constructor stub

}

MLAchievement::~MLAchievement() {
	// TODO Auto-generated destructor stub
}

void MLAchievement::achieve_player_login()
{
	this->init_achieve_info();
	this->login_update_achieve_info();
}

int MLAchievement::request_fetch_achieve_info()
{
	BaseAchieveInfo &base_achieve = this->base_achieve();
	Proto51401101 respond;
	respond.set_achieve_level(base_achieve.achieve_level_);

	for (BaseAchieveInfo::ChildAchieveMap::iterator iter = base_achieve.child_achieve_map_.begin();
			iter != base_achieve.child_achieve_map_.end(); ++iter)
	{
		BaseAchieveInfo::ChildAchieve &child_achieve = iter->second;
		for (BaseAchieveInfo::AchieveInfoSet::iterator it = child_achieve.ach_info_set_.begin();
				it != child_achieve.ach_info_set_.end(); ++it)
		{
			BaseAchieveInfo::AchieveInfo &achieve_info = *it;
			int achieve_id = achieve_info.achieve_id_;
			if (this->achieve_map_.count(achieve_id) <= 0)
			{
				this->create_and_update_achieve(achieve_id);
			}
			AchieveDetail *achieve_detail = this->achieve_map_[achieve_id];
			JUDGE_CONTINUE(achieve_detail != NULL);

			ProtoAchieveDetail* proto = respond.add_achieve_list();
			this->serilize_proto_achieve_detail(proto, achieve_detail, child_achieve.ach_index_);
		}
	}

	for (IntMap::iterator iter = base_achieve.achieve_point_map_.begin();
			iter != base_achieve.achieve_point_map_.end(); ++iter)
	{
		ProtoPairObj* pair_obj = respond.add_point_set();
		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}

	FINER_PROCESS_RETURN(RETURN_REQUEST_ACHIEVE_PANEL_INFO, &respond);
}

int MLAchievement::get_achieve_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11401102*, request, RETURN_DRAW_ACHIEVE_REWARD);

	int ach_index = request->ach_index();
	int achieve_id = request->achieve_id();
	AchieveDetail *achieve_detail = this->achieve_map_[achieve_id];
	CONDITION_NOTIFY_RETURN(achieve_detail != NULL, RETURN_DRAW_ACHIEVE_REWARD,
			ERROR_CONFIG_NOT_EXIST);
	CONDITION_NOTIFY_RETURN(achieve_detail->get_status_ == GameEnum::GET_STATUS_ABLE,
			RETURN_DRAW_ACHIEVE_REWARD, ERROR_NO_REWARD);

	BaseAchieveInfo::ChildAchieve* child_achieve = this->base_achieve_.find_child_achieve(ach_index);
	CONDITION_NOTIFY_RETURN(child_achieve != NULL, RETURN_DRAW_ACHIEVE_REWARD, ERROR_CONFIG_NOT_EXIST);

	BaseAchieveInfo::AchieveInfo* achieve_info = this->base_achieve_.find_achieve_info(child_achieve, achieve_id);
	CONDITION_NOTIFY_RETURN(achieve_info != NULL, RETURN_DRAW_ACHIEVE_REWARD, ERROR_CONFIG_NOT_EXIST);

	int reward_id = achieve_info->reward_id_;
	SerialObj obj(ADD_FROM_ACHIEVE_REWARD, ach_index, achieve_id);
	this->add_reward(reward_id, obj);

	achieve_detail->get_status_ = GameEnum::GET_STATUS_DONE;

	int achieve_point = achieve_info->ach_amount_;
	int base_type = child_achieve->base_type_;
	this->base_achieve_.achieve_point_map_[base_type] += achieve_point;

	this->check_achieve_uplevel();

	Proto51401102 respond;
	respond.set_achieve_id(achieve_id);
	FINER_PROCESS_RETURN(RETURN_DRAW_ACHIEVE_REWARD, &respond);
}

int MLAchievement::update_achieve_info(const int ach_index, const int value, const int ach_type)
{
	BaseAchieveInfo& base_achieve = this->base_achieve();

	BaseAchieveInfo::ChildAchieve* child_achieve = base_achieve.find_child_achieve(ach_index);
	JUDGE_RETURN(child_achieve != NULL, 0);

	return this->update_achieve_detail(child_achieve, value, ach_type);
}

int MLAchievement::finish_map_achievement(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401901*, request, -1);

	int ach_index = request->ach_index();
	int ach_type  = request->ach_type();
	int cur_value = request->cur_value();
	Int64 special_value = request->special_value();

	int update_flag = true;
	if (special_value != 0)
	{
		BaseAchieveInfo& base_achieve = this->base_achieve();
		BaseAchieveInfo::ChildAchieve* child_achieve = base_achieve.find_child_achieve(ach_index);
		JUDGE_RETURN(child_achieve != NULL, -1);

		BaseAchieveInfo::AchieveInfoSet::iterator iter = child_achieve->ach_info_set_.begin();
		for (; iter != child_achieve->ach_info_set_.end(); ++iter)
		{
			BaseAchieveInfo::AchieveInfo &achieve_info = *iter;
			int achieve_id  = achieve_info.achieve_id_;
			if (this->achieve_map_.count(achieve_id) <= 0)
			{
				this->create_and_update_achieve(achieve_id);
			}
			AchieveDetail *achieve_detail = this->achieve_map_[achieve_id];
			JUDGE_CONTINUE(achieve_detail != NULL);

			if (special_value == achieve_detail->special_value_)
				update_flag = false;
			else
				achieve_detail->special_value_ = special_value;
		}
	}
	if (update_flag == true)
		this->update_achieve_info(ach_index, cur_value, ach_type);

	return 0;
}

int MLAchievement::sync_transfer_achieve(int scene_id)
{
	Proto31400122 request;
	request.set_achieve_level(this->base_achieve_.achieve_level_);

//	for (BaseAchieveInfo::ChildAchieveSet::iterator iter = this->base_achieve_.child_achieve_set_.begin();
//			iter != this->base_achieve_.child_achieve_set_.end(); ++iter)
//	{
//		BaseAchieveInfo::ChildAchieve &child_achieve = *iter;
//		ProtoChildAchieve* child_proto = request.add_child_achieve();
//		child_proto->set_ach_index(child_achieve.ach_index_);
//		child_proto->set_base_type(child_achieve.base_type_);
//		child_proto->set_child_type(child_achieve.child_type_);
////		child_proto->set_act_type(child_achieve.act_type_);
//		child_proto->set_compare(child_achieve.compare_);
//		child_proto->set_sort(child_achieve.sort_);
//		child_proto->set_red_point(child_achieve.red_point_);
//
//		for (BaseAchieveInfo::AchieveInfoSet::iterator it = child_achieve.ach_info_set_.begin();
//				it != child_achieve.ach_info_set_.end(); ++it)
//		{
//			BaseAchieveInfo::AchieveInfo &achieve_info = *it;
//			ProtoAchieveInfo* info_proto = child_proto->add_achieve_info();
//			info_proto->set_achieve_id(achieve_info.achieve_id_);
//			info_proto->set_ach_type(achieve_info.achieve_type_);
//			info_proto->set_number_type(achieve_info.number_type_);
//			info_proto->set_need_amount(achieve_info.need_amount_);
//			info_proto->set_sort(achieve_info.sort_);
//			info_proto->set_reward_id(achieve_info.reward_id_);
//			info_proto->set_ach_amount(achieve_info.ach_amount_);
//		}
//	}

	for (AchieveMap::iterator iter = this->achieve_map_.begin();
			iter != this->achieve_map_.end(); ++iter)
	{
		ProtoAchieveDetail* achieve_detail = request.add_achieve_detail();
		this->serilize_proto_achieve_detail(achieve_detail, iter->second, 0);
	}

	for (IntMap::iterator iter = this->base_achieve_.achieve_point_map_.begin();
			iter != this->base_achieve_.achieve_point_map_.end(); ++iter)
	{
		ProtoPairObj* pair_obj = request.add_achieve_point_map();
		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}

	return this->send_to_other_logic_thread(scene_id, request);
}

int MLAchievement::read_transfer_achieve(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400122*, respond, -1);

	this->init_achieve_info();

	for(int i = 0; i < respond->achieve_detail_size(); ++i)
	{
		ProtoAchieveDetail achieve_detail = respond->achieve_detail(i);
		AchieveDetail* detail = POOL_MONITOR->achieve_detail_pool()->pop();
		JUDGE_CONTINUE(detail != NULL);

		detail->achieve_id_ = achieve_detail.achieve_id();
		detail->finish_num_	= achieve_detail.finish_num();
		detail->get_status_ = achieve_detail.get_status();
		detail->finish_tick_= achieve_detail.finish_tick();
		detail->special_value_ = achieve_detail.special_value();
		this->achieve_map_.insert(AchieveMap::value_type(detail->achieve_id_, detail));
	}

//	for(int i = 0; i < respond->child_achieve_size(); ++i)
//	{
//		ProtoChildAchieve child_proto = respond->child_achieve(i);
//		BaseAchieveInfo::ChildAchieve child_achieve;
//		child_achieve.ach_index_ 	= child_proto.ach_index();
//		child_achieve.base_type_ 	= child_proto.base_type();
//		child_achieve.child_type_ 	= child_proto.child_type();
////		child_achieve.act_type_ 	= child_proto.act_type();
//		child_achieve.compare_ 		= child_proto.compare();
//		child_achieve.sort_ 		= child_proto.sort();
//		child_achieve.red_point_ 	= child_proto.red_point();
//
//		for (int j = 0; j < child_proto.achieve_info_size(); ++j)
//		{
//			ProtoAchieveInfo info_proto = child_proto.achieve_info(j);
//			BaseAchieveInfo::AchieveInfo achieve_info;
//			achieve_info.achieve_id_ 	= info_proto.achieve_id();
//			achieve_info.achieve_type_	= info_proto.ach_type();
//			achieve_info.number_type_	= info_proto.number_type();
//			achieve_info.need_amount_ 	= info_proto.need_amount();
//			achieve_info.sort_ 			= info_proto.sort();
//			achieve_info.reward_id_ 	= info_proto.reward_id();
//			achieve_info.ach_amount_ 	= info_proto.ach_amount();
//			child_achieve.ach_info_set_.push_back(achieve_info);
//		}
//		base_achieve.child_achieve_set_.push_back(child_achieve);
//	}

	BaseAchieveInfo &base_achieve = this->base_achieve();
	base_achieve.achieve_level_ = respond->achieve_level();
	IntMap& point_map = base_achieve.achieve_point_map_;
	for (int i = 0; i < respond->achieve_point_map_size(); ++i)
	{
		ProtoPairObj pair_obj = respond->achieve_point_map(i);
		point_map[pair_obj.obj_id()] += pair_obj.obj_value();
	}

	return 0;
}

void MLAchievement::reset(void)
{
	for (AchieveMap::iterator iter = this->achieve_map_.begin();
			iter != this->achieve_map_.end(); ++iter)
	{
		POOL_MONITOR->achieve_detail_pool()->push(iter->second);
	}
	this->achieve_map_.clear();
	this->base_achieve_.reset();
}

void MLAchievement::init_achieve_info()
{
	GameConfig::ConfigMap& ach_map = CONFIG_INSTANCE->achieve_map();
	GameConfig::ConfigMap::iterator iter = ach_map.begin();
	for (; iter != ach_map.end(); ++iter)
	{
		this->base_achieve_.add_new_achieve(iter->first, *(iter->second));
	}
}

void MLAchievement::check_achieve_uplevel()
{
	BaseAchieveInfo& base_achieve = this->base_achieve();
	int achieve_level = base_achieve.achieve_level_;
	const Json::Value& achieve_level_json = CONFIG_INSTANCE->achieve_level(achieve_level+1);
	const Json::Value& next_level_json = CONFIG_INSTANCE->achieve_level(achieve_level+2);
	JUDGE_RETURN(achieve_level_json != Json::Value::null && next_level_json != Json::Value::null, ;);

	int achieve_exp = 0;
	for (IntMap::iterator iter = base_achieve.achieve_point_map_.begin();
			iter != base_achieve.achieve_point_map_.end(); ++iter)
	{
		achieve_exp += iter->second;
	}
	int need_exp = achieve_level_json["need_exp"].asInt();
	JUDGE_RETURN(achieve_exp >= need_exp, ;);

	++base_achieve.achieve_level_;
	this->refresh_achieve_attr_add();
	this->check_achieve_uplevel();
}

int MLAchievement::refresh_achieve_attr_add(const int enter_type)
{
	int achieve_level = this->base_achieve_.achieve_level_;
	const Json::Value& achieve_level_json = CONFIG_INSTANCE->achieve_level(achieve_level+1);
	JUDGE_RETURN(achieve_level_json != Json::Value::null, 0);

	IntMap prop_map;
	prop_map[GameEnum::ATTACK] = achieve_level_json["attack"].asInt();
	prop_map[GameEnum::DEFENSE] = achieve_level_json["defence"].asInt();
	prop_map[GameEnum::BLOOD_MAX] = achieve_level_json["blood_max"].asInt();

	if (prop_map.size() > 0)
		this->refresh_fight_property(BasicElement::ACHIEVEMENT, prop_map, enter_type);

	return 0;
}

int MLAchievement::login_update_achieve_info()
{
	MapLogicPlayer* player = this->map_logic_player();

	for (BaseAchieveInfo::ChildAchieveMap::iterator iter = this->base_achieve_.child_achieve_map_.begin();
			iter != this->base_achieve_.child_achieve_map_.end(); ++iter)
	{
		BaseAchieveInfo::ChildAchieve &child_achieve = iter->second;
		int ach_index = child_achieve.ach_index_;
		int value = 0;
		switch (ach_index) {
			case GameEnum::ROLE_LEVEL:
			{
				value = this->role_level();
				break;
			}
			case GameEnum::MOUNT_LEVEL:
			case GameEnum::WING_LEVEL:
			case GameEnum::PET_LEVEL:
			case GameEnum::MAGIC_EQUIP_LEVEL:
			case GameEnum::GOD_SOLIDER_LEVEL:
			case GameEnum::BEAST_EQUIP_LEVEL:
			case GameEnum::BEAST_MOUNT_LEVEL:
			case GameEnum::BEAST_WING_LEVEL:
			case GameEnum::BEAST_CAT_LEVEL:
			case GameEnum::PLOUGH_LEVEL:
			{
				int mount_type = this->fetch_achieve_for_mount_type(ach_index);
				JUDGE_CONTINUE(mount_type > 0);

				MountDetail &mount_detail = player->mount_detail(mount_type);
				value = mount_detail.mount_grade_;
				break;
			}
			case GameEnum::MOUNT_SKILL:
			case GameEnum::WING_SKILL:
			case GameEnum::PET_SKILL:
			case GameEnum::MAGIC_EQUIP_SKILL:
			case GameEnum::GOD_SOLIDER_SKILL:
			case GameEnum::BEAST_EQUIP_SKILL:
			case GameEnum::BEAST_MOUNT_SKILL:
			case GameEnum::BEAST_WING_SKILL:
			case GameEnum::BEAST_CAT_SKILL:
			case GameEnum::PLOUGH_SKILL:
			{
				int mount_type = this->fetch_achieve_for_mount_type(ach_index);
				JUDGE_CONTINUE(mount_type > 0);

				player->check_mount_skill_achieve(mount_type);
				continue;
			}
			case GameEnum::DAILY_TASK:
			case GameEnum::REWARD_TASK:
			case GameEnum::LEAGUE_TASK:
			case GameEnum::LEGEND_TOP_SCRIPT:
			case GameEnum::EXP_SCRIPT:
			case GameEnum::SWORD_FIGHT:
			case GameEnum::LEGEND_RANK:
			case GameEnum::SM_BATTLE_KILL:
			case GameEnum::ESCORT_ACH:
			case GameEnum::WORLD_BOSS:
			{
				this->login_update_task_achieve_info(&child_achieve);
				continue;
			}
			default:
				break;
		}
		this->update_achieve_detail(&child_achieve, value);
	}

	return 0;
}

int MLAchievement::update_achieve_detail(BaseAchieveInfo::ChildAchieve* child_achieve, int value, const int ach_type)
{
	Proto81401102 respond;
	BaseAchieveInfo::AchieveInfoSet::iterator iter = child_achieve->ach_info_set_.begin();
	for (; iter != child_achieve->ach_info_set_.end(); ++iter)
	{
		BaseAchieveInfo::AchieveInfo &achieve_info = *iter;
		if (ach_type > 0)
			JUDGE_CONTINUE(achieve_info.achieve_type_ == ach_type);

		int achieve_id  = achieve_info.achieve_id_;
		int need_amount = achieve_info.need_amount_;
		if (this->achieve_map_.count(achieve_id) <= 0)
		{
			this->create_and_update_achieve(achieve_id);
		}
		AchieveDetail *achieve_detail = this->achieve_map_[achieve_id];
		JUDGE_CONTINUE(achieve_detail->get_status_ != GameEnum::GET_STATUS_DONE);

		int compare  = child_achieve->compare_;
//		int act_type = child_achieve->act_type_;
		int number_type = achieve_info.number_type_;
		if (number_type == 1)
			achieve_detail->finish_num_ = value;
		else
			achieve_detail->finish_num_ += value;

		this->calc_achieve_info(achieve_detail, compare, need_amount);

		ProtoAchieveDetail* ach_detail = respond.add_achieve_detail();
		this->serilize_proto_achieve_detail(ach_detail, achieve_detail, child_achieve->ach_index_);
	}
	this->respond_to_client(ACTIVE_ACHIEVEMENT_PROCESS_NOTIFY, &respond);

	return 0;
}

int MLAchievement::login_update_task_achieve_info(BaseAchieveInfo::ChildAchieve* child_achieve)
{
	IntMap finish_map;
	int compare = child_achieve->compare_;
	BaseAchieveInfo::AchieveInfoSet::iterator iter = child_achieve->ach_info_set_.begin();
	for (; iter != child_achieve->ach_info_set_.end(); ++iter)
	{
		BaseAchieveInfo::AchieveInfo &achieve_info = *iter;
		int achieve_type = achieve_info.achieve_type_;
		int achieve_id   = achieve_info.achieve_id_;
		if (this->achieve_map_.count(achieve_id) <= 0)
		{
			this->create_and_update_achieve(achieve_id);
		}
		AchieveDetail *achieve_detail = this->achieve_map_[achieve_id];
		JUDGE_CONTINUE((compare == 1 && (finish_map[achieve_type] <= 0 || finish_map[achieve_type] > achieve_detail->finish_num_))
				|| (compare != 1 && finish_map[achieve_type] < achieve_detail->finish_num_))

		finish_map[achieve_type] = achieve_detail->finish_num_;
	}

	BaseAchieveInfo::AchieveInfoSet::iterator it = child_achieve->ach_info_set_.begin();
	for (; it != child_achieve->ach_info_set_.end(); ++it)
	{
		BaseAchieveInfo::AchieveInfo &achieve_info = *it;
		int achieve_type = achieve_info.achieve_type_;
		int achieve_id   = achieve_info.achieve_id_;
		int need_amount  = achieve_info.need_amount_;
		AchieveDetail *achieve_detail = this->achieve_map_[achieve_id];
		JUDGE_CONTINUE(achieve_detail != NULL);
		JUDGE_CONTINUE(achieve_detail->get_status_ != GameEnum::GET_STATUS_DONE);

		achieve_detail->finish_num_ = finish_map[achieve_type];
		this->calc_achieve_info(achieve_detail, compare, need_amount);
	}

	return 0;
}

int MLAchievement::fetch_achieve_for_mount_type(int ach_index)
{
	int mount_type = 0;
	switch(ach_index)
	{
	case GameEnum::MOUNT_LEVEL:
	case GameEnum::MOUNT_SKILL:
	{
		return GameEnum::FUN_MOUNT;
	}
	case GameEnum::WING_LEVEL:
	case GameEnum::WING_SKILL:
	{
		return GameEnum::FUN_XIAN_WING;
	}
	case GameEnum::PET_LEVEL:
	case GameEnum::PET_SKILL:
	{
		return GameEnum::FUN_LING_BEAST;
	}
	case GameEnum::MAGIC_EQUIP_LEVEL:
	case GameEnum::MAGIC_EQUIP_SKILL:
	{
		return GameEnum::FUN_MAGIC_EQUIP;
	}
	case GameEnum::GOD_SOLIDER_LEVEL:
	case GameEnum::GOD_SOLIDER_SKILL:
	{
		return GameEnum::FUN_GOD_SOLIDER;
	}
	case GameEnum::BEAST_EQUIP_LEVEL:
	case GameEnum::BEAST_EQUIP_SKILL:
	{
		return GameEnum::FUN_BEAST_EQUIP;
	}
	case GameEnum::BEAST_MOUNT_LEVEL:
	case GameEnum::BEAST_MOUNT_SKILL:
	{
		return GameEnum::FUN_BEAST_MOUNT;
	}
	case GameEnum::BEAST_WING_LEVEL:
	case GameEnum::BEAST_WING_SKILL:
	{
		return GameEnum::FUN_BEAST_WING;
	}
	case GameEnum::BEAST_CAT_LEVEL:
	case GameEnum::BEAST_CAT_SKILL:
	{
		return GameEnum::FUN_BEAST_MAO;
	}
	case GameEnum::PLOUGH_LEVEL:
	case GameEnum::PLOUGH_SKILL:
	{
		return GameEnum::FUN_TIAN_GANG;
	}
	default:
		break;
	}

	return mount_type;
}

void MLAchievement::calc_achieve_info(AchieveDetail *achieve_detail, int compare, int need_amount)
{
	int notify_flag = false;
	if (compare == 1 && achieve_detail->finish_num_ > 0)
	{
		if (achieve_detail->finish_num_ <= need_amount)
		{
			achieve_detail->get_status_  = GameEnum::GET_STATUS_ABLE;
			if (achieve_detail->finish_tick_ <= 0)
				notify_flag = true;

			achieve_detail->finish_tick_ = ::time(NULL);
		}
		else
		{
			achieve_detail->get_status_  = GameEnum::GET_STATUS_NULL;
			achieve_detail->finish_tick_ = 0;
		}
	}
	else if (compare != 1)
	{
		if (achieve_detail->finish_num_ >= need_amount)
		{
			achieve_detail->get_status_  = GameEnum::GET_STATUS_ABLE;
			if (achieve_detail->finish_tick_ <= 0)
				notify_flag = true;

			achieve_detail->finish_tick_ = ::time(NULL);
		}
		else
		{
			achieve_detail->get_status_  = GameEnum::GET_STATUS_NULL;
			achieve_detail->finish_tick_ = 0;
		}
	}
	JUDGE_RETURN(notify_flag == true, ;);

	int achieve_notify_level = CONFIG_INSTANCE->const_set("achieve_notify_level");
	JUDGE_RETURN(this->role_level() >= achieve_notify_level, ;);

	Proto81401101 respond;
	respond.set_achieve_id(achieve_detail->achieve_id_);
	this->respond_to_client(ACTIVE_ACHIEVEMENT_UNCHECK_NOTIFY, &respond);
}

void MLAchievement::create_and_update_achieve(int achieve_id)
{
	AchieveDetail *detail = POOL_MONITOR->achieve_detail_pool()->pop();
	JUDGE_RETURN(detail != NULL, ;);

	detail->achieve_id_ = achieve_id;
	this->achieve_map_.insert(MLAchievement::AchieveMap::value_type(achieve_id, detail));
}

void MLAchievement::serilize_proto_achieve_detail(ProtoAchieveDetail* proto, AchieveDetail* achieve_detail, int ach_index)
{
	JUDGE_RETURN(proto != NULL && achieve_detail != NULL, ;);

	proto->set_achieve_id(achieve_detail->achieve_id_);
	proto->set_ach_index(ach_index);
	proto->set_get_status(achieve_detail->get_status_);
	proto->set_finish_tick(achieve_detail->finish_tick_);
	proto->set_finish_num(achieve_detail->finish_num_);
	proto->set_special_value(achieve_detail->special_value_);
//	proto->set_is_done(0);
}

MLAchievement::AchieveMap& MLAchievement::achieve_map(void)
{
	return this->achieve_map_;
}

BaseAchieveInfo& MLAchievement::base_achieve()
{
	return this->base_achieve_;
}

