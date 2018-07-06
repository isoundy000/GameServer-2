/*
 * MapLogicIllustration.cpp
 *
 *  Created on: 2016年7月20日
 *      Author: lzy0927
 */

#include "MapLogicIllustration.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "PubStruct.h"

void MapLogicIllustration::reset()
{
	this->cur_illus_class_id_ = 0;
	this->cur_illus_group_id_ = 0;
	this->cur_illus_id_ = 0;

	this->get_group_list_.clear();
	this->get_illus_list_.clear();
	this->illus_group_list_.clear();
	this->illus_list_.clear();
}

MapLogicIllustration::MapLogicIllustration()
{
	// TODO Auto-generated constructor stub
	reset();
}

MapLogicIllustration::~MapLogicIllustration()
{
	// TODO Auto-generated constructor stub
//	test();
}

IntSet &MapLogicIllustration::get_group_list(void)
{
	return this->get_group_list_;
}
IntMap &MapLogicIllustration::get_illus_list(void)
{
	return this->get_illus_list_;
}

int &MapLogicIllustration::get_open(void)
{
	return this->open_;
}

int MapLogicIllustration::received_award_illus_set(IntSet &res_set)
{
	IntSet tmp_set(this->get_group_list_.begin(),
			this->get_group_list_.end());
	res_set.swap(tmp_set);
	return 0;
}

int MapLogicIllustration::received_award_illus_map(IntMap &res_map)
{
	IntMap tmp_map(this->get_illus_list_.begin(),
			this->get_illus_list_.end());
	res_map.swap(tmp_map);
	return 0;
}

int MapLogicIllustration::attr_type_int(string str)
{
	if (str == GameName::ATTACK) return GameEnum::ATTACK;
	if (str == GameName::DEFENSE) return GameEnum::DEFENSE;
	if (str == GameName::BLOOD_MAX) return GameEnum::BLOOD_MAX;
	if (str == GameName::MAGIC_MAX) return GameEnum::MAGIC_MAX;
	if (str == GameName::CRIT) return GameEnum::CRIT;
	if (str == GameName::TOUGHNESS) return GameEnum::TOUGHNESS;
	if (str == GameName::HIT) return GameEnum::HIT;
	if (str == GameName::AVOID) return GameEnum::AVOID;
	if (str == GameName::LUCK) return GameEnum::LUCKY;

	return -1;
}

void MapLogicIllustration::refresh_illustration_prop(int enter_type)
{
	int ret = this->check_open_illustration();
	if (ret < 0)
	{
		return ;
	}

	IntMap prop_map;
	this->calc_illus_total_prop(prop_map);
	this->refresh_fight_property(BasicElement::ILLUSTRATION, prop_map, enter_type);
}

int MapLogicIllustration::check_open_illustration(int task_id)
{
	if (this->open_) return 1;
	if (task_id != 0)
	{
		if (CONFIG_INSTANCE->arrive_fun_open_task("fun_illiusion",
				task_id) == false)
		{
			return ERROR_BACK_GAME_SWITCH_CLOSE;
		}
		if (this->open_ == 0)
		{
			MapLogicPlayer * player = dynamic_cast<MapLogicPlayer *>(this);
			player->update_player_assist_single_event(GameEnum::PA_EVENT_ILLUS_OPEN, 1);
		}
		this->open_ = 1;
		return this->open_;
	}

	if (CONFIG_INSTANCE->arrive_fun_open_level("fun_illiusion",
			this->role_detail().__level) == false)
	{
		return ERROR_BACK_GAME_SWITCH_CLOSE;
	}
	if (this->open_ == 0)
	{
		MapLogicPlayer * player = dynamic_cast<MapLogicPlayer *>(this);
		player->update_player_assist_single_event(GameEnum::PA_EVENT_ILLUS_OPEN, 1);
	}
	this->open_ = 1;
	return 0;
}

int MapLogicIllustration::fetch_illus_panel_info(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto11404001*, request, RETURN_REQUEST_ILLUS_PANEL_INFO);
    Proto51404001 respond;

	int ret = this->check_open_illustration();
	if (ret < 0)
	{
		return this->respond_to_client_error(RETURN_REQUEST_ILLUS_PANEL_INFO, ret);
	}

    IntSet::iterator it = this->get_group_list_.begin();
    for (; it != this->get_group_list_.end(); ++it)
    {
    	respond.add_illus_group_list(*it);
    }

    IntMap::iterator it1 = this->get_illus_list_.begin();
    for (; it1 != this->get_illus_list_.end(); ++it1)
    {
    	Illustrations *illus_temp = respond.add_illus_list();
    	int illus_id = it1->first;

        const Json::Value &illus_json = CONFIG_INSTANCE->illus(illus_id);

    	int cur_level = it1->second;

    	JUDGE_RETURN(illus_json != Json::Value::null, -1);
    	illus_temp->set_illus_id(illus_id);
    	illus_temp->set_illus_group_id(illus_json["illus_group_id"].asInt());
    	illus_temp->set_upgrade_goods_id(illus_json["upgrade_goods_id"].asInt());
    	illus_temp->set_attr_type(attr_type_int(illus_json["attr_type"].asString()));
    	illus_temp->set_open_level(illus_json["open_level"].asInt());
    	illus_temp->set_illus_class_id(illus_json["illus_class_id"].asInt());
    	illus_temp->set_cur_value(illus_json["value_list"][cur_level].asInt());
    	illus_temp->set_upgrade_goods_amount(illus_json["goods_amount_list"][cur_level].asInt());
    	illus_temp->set_illus_level(cur_level);
    }

	IntMap prop_map;
	this->calc_illus_total_prop(prop_map);
//	this->refresh_fight_property(BasicElement::ILLUSTRATION, prop_map);

	FightProperty temp;
	temp.unserialize(prop_map);
	temp.serialize(respond.mutable_prop_list());
// 	MSG_USER("图鉴测试 %d", temp.force_);
//	MSG_DEBUG(%s,respond.Utf8DebugString().c_str());
	FINER_PROCESS_RETURN(RETURN_REQUEST_ILLUS_PANEL_INFO, &respond);
}


int MapLogicIllustration::calc_illus_total_prop(IntMap& prop_map)
{
	IntMap::iterator it = this->get_illus_list_.begin();
	int i = 0;
	for(; it != this->get_illus_list().end(); ++it, ++i)
	{
//    	MSG_USER("图鉴测试 图鉴集合 %d %d", it->first, it->second);
		if (it->second == 0) continue;
		this->calc_illus_total_prop_ex(it->first, it->second - 1,prop_map);
	}
	return 0;
}


int MapLogicIllustration::calc_illus_total_prop_ex(const int illus_id, const int illus_level, IntMap& prop_map)
{
    const Json::Value &illus_json = CONFIG_INSTANCE->illus(illus_id);
    JUDGE_RETURN(illus_json != Json::Value::null, -1);

    int attr_type = attr_type_int(illus_json["attr_type"].asString());

//    MSG_USER("图鉴测试 图鉴集合2 %d %d", attr_type, illus_json["value_list"][illus_level].asInt());
    prop_map[attr_type] += illus_json["value_list"][illus_level].asInt();

	return 0;
}

int MapLogicIllustration::select_illus_class(Message* msg)
{
	Proto51404002 respond;

	MSG_DYNAMIC_CAST_RETURN(Proto11404002*, request, RETURN_SELECT_ILLUS_CLASS);

	int class_id = request->illus_class_id();

	const Json::Value &illus_class_json = CONFIG_INSTANCE->illus_class(class_id);
	int g_size = illus_class_json["illus_group_list"].size();

	for (int i = 0; i < g_size; ++i)
	{
		//test
		int group_id = illus_class_json["illus_group_list"][i].asInt();
        this->sync_add_illus(group_id);


		Illus_group *group_temp = respond.add_illus_group_list();
		group_temp->set_group_id(illus_class_json["illus_group_list"][i].asInt());
	}

	//更新当前组信息

	this->refresh_group_shape(class_id);

	respond.set_illus_class_id(class_id);
	FINER_PROCESS_RETURN(RETURN_SELECT_ILLUS_CLASS, &respond);
}


int MapLogicIllustration::select_illus_group(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11404003*, request, RETURN_SELECT_ILLUS_GROUP);

	int group_id = request->illus_group_id();

	Proto51404003 respond;
	respond.set_illus_group_id(group_id);

	const Json::Value &illus_group_json = CONFIG_INSTANCE->illus_group(group_id);
	int i_size = illus_group_json["illus_list"].size();

	for (int i = 0; i < i_size; ++i)
	{
		Illustrations *illus_temp = respond.add_illus_list();
        int illus_id = illus_group_json["illus_list"][i].asInt();

        const Json::Value &illus_json = CONFIG_INSTANCE->illus(illus_id);

    	int cur_level = 0;
    	if (this->get_illus_list_.count(illus_id) > 0)
    		cur_level = this->get_illus_list_.find(illus_id)->second;

    	JUDGE_RETURN(illus_json != Json::Value::null, -1);
    	illus_temp->set_illus_id(illus_id);
    	illus_temp->set_illus_group_id(illus_json["illus_group_id"].asInt());
    	illus_temp->set_upgrade_goods_id(illus_json["upgrade_goods_id"].asInt());
    	illus_temp->set_attr_type(attr_type_int(illus_json["attr_type"].asString()));
    	illus_temp->set_open_level(illus_json["open_level"].asInt());
    	illus_temp->set_illus_class_id(illus_json["illus_class_id"].asInt());
    	illus_temp->set_cur_value(illus_json["value_list"][cur_level].asInt());
    	illus_temp->set_upgrade_goods_amount(illus_json["goods_amount_list"][cur_level].asInt());
    	illus_temp->set_illus_level(cur_level);
	}

	this->refresh_illus_shape(this->cur_illus_class_id(),group_id);


	FINER_PROCESS_RETURN(RETURN_SELECT_ILLUS_GROUP, &respond);
}

int MapLogicIllustration::refresh_group_shape(const int class_id)
{

	if (class_id < 1 && class_id > 3) return -1;

	this->set_cur_illus_class_id(class_id);
	this->illus_group_list_.clear();
	const Json::Value &illus_class_json = CONFIG_INSTANCE->illus_class(class_id);

	int group_size = illus_class_json["group_list"].size();

	for (int i = 0; i < group_size; ++i)
	{
		int group_id = illus_class_json["group_list"][i].asInt();
		this->illus_group_list_.push_back(group_id);
	}
	int group_id = this->illus_group_list_[0];
    this->set_cur_illus_group_id(group_id);

    this->refresh_illus_shape(class_id, group_id);

    return 0;
}


int MapLogicIllustration::refresh_illus_shape(const int class_id, const int group_id)
{

	if (group_id < 0 && group_id > 100) return -1;

	this->illus_list_.clear();
	//this->illus_level_list_.clear();

	const Json::Value &illus_group_json = CONFIG_INSTANCE->illus_group(group_id);

	int illus_size = illus_group_json["illus_list"].size();

	for (int i = 0; i < illus_size; ++i)
	{
		int illus_id = illus_group_json["illus_list"][i].asInt();
		this->illus_list_.push_back(illus_id);
	//	this->illus_level_list_[i] = 0;
	}
	Proto30400603 respond;
	respond.set_cur_illus_class_id(this->cur_illus_class_id_);
	IntSet::iterator it = this->get_group_list_.begin();
	int i = 0;
	for (; it != this->get_group_list_.end(); ++i, ++it)
	{
		respond.set_illus_group_list(i, *it);
	}

	IntMap::iterator it1 = this->get_illus_list_.begin();
	i = 0;
	for (; it1 != this->get_illus_list_.end(); ++i, ++it1)
	{
		respond.set_illus_list(i, it1->first);
		respond.set_illus_level_list(i, it1->second);
	}


	MAP_MONITOR->process_inner_map_request(this->role_id(), respond);
	return 0;
}

int MapLogicIllustration::select_single_illus(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11404006*, request, RETURN_SELECT_ILLUS);

	int illus_id = request->illus_id();



	this->refresh_illus_shape(this->cur_illus_class_id(),this->cur_illus_group_id());

	Proto51404006 respond;

	respond.set_illus_id(illus_id);


	FINER_PROCESS_RETURN(RETURN_SELECT_ILLUS, &respond);
}

int MapLogicIllustration::upgrade_illus_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11404005*, request, RETURN_UPGRADE_ILLUS);

	int illus_id = request->illus_id();
    const Json::Value &illus_json = CONFIG_INSTANCE->illus(illus_id);
//	const Json::Value& cfg = CONFIG_INSTANCE->label(illus_id);//need to change

//
	Proto51404005 respond;
	respond.set_illus_id(illus_id);
	int cur_level = 0;
	if (this->get_illus_list_.count(illus_id) > 0)
		cur_level = this->get_illus_list_.find(illus_id)->second;

	if (this->get_illus_list_.count(illus_id) == 0) return -1;

	this->upgrade_illus(illus_id, illus_json["upgrade_goods_id"].asInt());//升级图鉴需消耗物品
	Illustrations *illus_temp = respond.mutable_illus_info();
	illus_temp->set_illus_id(illus_id);




	JUDGE_RETURN(illus_json != Json::Value::null, -1);

	illus_temp->set_illus_group_id(illus_json["illus_group_id"].asInt());
	illus_temp->set_upgrade_goods_id(illus_json["upgrade_goods_id"].asInt());
	illus_temp->set_attr_type(attr_type_int(illus_json["attr_type"].asString()));
	illus_temp->set_open_level(illus_json["open_level"].asInt());
	illus_temp->set_illus_class_id(illus_json["illus_class_id"].asInt());
	illus_temp->set_cur_value(illus_json["value_list"][cur_level].asInt());
	illus_temp->set_upgrade_goods_amount(illus_json["goods_amount_list"][cur_level].asInt());
	illus_temp->set_illus_level(this->get_illus_list_.find(illus_id)->second);

	this->notify_illus_group_list_update(1,this->cur_illus_group_id_);

	MSG_DEBUG(%s,respond.Utf8DebugString().c_str());
	FINER_PROCESS_RETURN(RETURN_UPGRADE_ILLUS, &respond);

}

int MapLogicIllustration::test_max_illus(int level)
{
	IntMap::iterator it = this->get_illus_list_.begin();
	for (; it != this->get_illus_list_.end(); ++it)
	{
		it->second = level;
	}

	return 0;
}

int MapLogicIllustration::upgrade_illus(int illus_id, int item_id)
{
    const Json::Value &illus_json = CONFIG_INSTANCE->illus(illus_id);
	int cur_level = 0;
		if (this->get_illus_list_.count(illus_id) > 0)
			cur_level = this->get_illus_list_.find(illus_id)->second;

	CONDITION_NOTIFY_RETURN(this->pack_remove(ITEM_UPGRADE_ILLUS, item_id,
		illus_json["goods_amount_list"][cur_level].asInt()) == 0,
		RETURN_UPGRADE_ILLUS, ERROR_ILLUS_ITEM_COUNT_ERROR);

	IntMap::iterator it;
	if (this->get_illus_list_.count(illus_id) > 0)
		it = this->get_illus_list_.find(illus_id);
	else
	{
		MSG_USER("this illus_id not get %d", illus_id);
		return -1;
	}
    it->second = it->second + 1;

    //计算提升属性
	IntMap prop_map;
	GameCommon::init_property_map(prop_map);
	this->calc_illus_total_prop(prop_map);
	this->refresh_fight_property(BasicElement::ILLUSTRATION, prop_map);

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_ILLUSTRATION);

	this->record_other_serial(ILLUS_UPGRADE_SERIAL, illus_id, it->second, illus_json["goods_amount_list"][cur_level].asInt());
	return 0;
}

int MapLogicIllustration::sync_add_illus(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401702*, request, -1);

    int illus_group_id = request->illus_group_id();

    const Json::Value &illus_group_json = CONFIG_INSTANCE->illus_group(illus_group_id);
    int group_size = illus_group_json["illus_list"].size();
    for (int i = 0; i < group_size; ++i)
    {
    	int illus_id = illus_group_json["illus_list"][i].asInt();
    	this->get_illus_list_.insert(IntMap::value_type (illus_id, 0));
    }

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_ILLUSTRATION);
	return 0;
}

void MapLogicIllustration::test_illus(void)
{
	Message* temp = NULL;
	this->fetch_illus_panel_info(temp);
	this->sync_add_illus(101);
	this->sync_add_illus(102);
	this->sync_add_illus(202);

}
int MapLogicIllustration::new_sync_add_illus(int level)
{
	const Json::Value &illus_json = CONFIG_INSTANCE->illus_class_json();

	Json::Value::iterator iter = illus_json.begin();
	for (; iter != illus_json.end(); ++iter)
	{

		Json::Value &illus_class = *iter;
		int j = 0;
		int group_size = (int)illus_class["illus_group_list"].size();
		for (; j < group_size; ++j)
		{
			int group_id = illus_class["illus_group_list"][j].asInt();
			if (this->get_group_list_.count(group_id) != 0) continue;

			int open_level = CONFIG_INSTANCE->illus_group(group_id)["open_lv"].asInt();
			if (level >= open_level) this->sync_add_illus(group_id);
		}
	}

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_ILLUSTRATION);
	return 0;
}

int MapLogicIllustration::sync_add_illus(int group_id)
{

    this->get_group_list_.insert(group_id);

    const Json::Value &illus_group_json = CONFIG_INSTANCE->illus_group(group_id);
    int group_size = illus_group_json["illus_list"].size();
    for (int i = 0; i < group_size; ++i)
    {
    	int illus_id = illus_group_json["illus_list"][i].asInt();
    	if (this->get_illus_list_.find(illus_id) != this->get_illus_list_.end()) continue;

    	this->get_illus_list_.insert(IntMap::value_type (illus_id, 0));

    }

	return 0;
}

int MapLogicIllustration::notify_illus_group_list_update(int opera, int group_id)
{
	Proto81404007 respond;

	IntVec::iterator it1 = this->illus_list_.begin();
	    int i = 0;
	    for (;it1 != this->illus_list_.end(); ++it1, ++i)
	    {
	    	Illustrations* illus_temp = respond.add_illus_list();
	    	int cur_level = 0;
	    	if (this->get_illus_list_.count(*it1) > 0)
	    		cur_level = this->get_illus_list_.find(*it1)->second;

	    	const Json::Value &illus_json = CONFIG_INSTANCE->illus(*it1);
	    	JUDGE_RETURN(illus_json != Json::Value::null, -1);

	    	illus_temp->set_illus_group_id(illus_json["illus_group_id"].asInt());
	    	illus_temp->set_upgrade_goods_id(illus_json["upgrade_goods_id"].asInt());
	    	illus_temp->set_attr_type(attr_type_int(illus_json["attr_type"].asString()));
	    	illus_temp->set_open_level(illus_json["open_level"].asInt());
	    	illus_temp->set_illus_class_id(illus_json["illus_class_id"].asInt());
	    	illus_temp->set_cur_value(illus_json["value_list"][cur_level].asInt());
	    	illus_temp->set_upgrade_goods_amount(illus_json["goods_amount_list"][cur_level].asInt());
	    	illus_temp->set_illus_level(cur_level);
	    }


	    IntVec::iterator it2 = this->illus_group_list_.begin();
	    for (;it2 != this->illus_group_list_.end(); ++it2)
	    {
	    	Illus_group* illus_group_temp = respond.add_illus_group_list();

	    	const Json::Value &illus_group_json = CONFIG_INSTANCE->illus_group(*it2);
	    	JUDGE_RETURN(illus_group_json != Json::Value::null, -1);

	    	illus_group_temp->set_group_id(*it2);
	    	illus_group_temp->set_group_type(illus_group_json["illus_group_type"].asInt());
	    }


	 //   Illus_group *group_temp1 = respond.add_illus_group_list();

	 //   Illustrations *illus_temp2 = group_temp1->add_illus_list();

		IntMap prop_map;
		this->calc_illus_total_prop(prop_map);
		FightProperty temp;
		temp.unserialize(prop_map);
		temp.serialize(respond.mutable_prop_list());

	//	MapLogicPlayer* player = dynamic_cast<MapLogicPlayer*>(this);
		MSG_USER("updata");
		FINER_PROCESS_RETURN(ACTIVE_UPDATE_ILLUS_LIST, &respond);
}



int MapLogicIllustration::cur_illus_id()
{
	return this->cur_illus_id_;
}

int MapLogicIllustration::cur_illus_class_id(void)
{
	return this->cur_illus_class_id_;
}

int MapLogicIllustration::cur_illus_group_id(void)
{
	return this->cur_illus_group_id_;
}

int MapLogicIllustration::set_cur_illus_id(int illus_id)
{
	this->cur_illus_id_ = illus_id;
	return 0;
}

int MapLogicIllustration::set_cur_illus_class_id(int illus_class_id)
{
	this->cur_illus_id_ = illus_class_id;
	return 0;
}
int MapLogicIllustration::set_cur_illus_group_id(int illus_group_id)
{
	this->cur_illus_id_ = illus_group_id;
	return 0;
}

int MapLogicIllustration::pre_illus_id(void)
{
	return this->pre_illus_id_;
}

int MapLogicIllustration::pre_illus_class_id(void)
{
	return this->pre_illus_class_id_;
}

int MapLogicIllustration::pre_illus_group_id(void)
{
	return this->pre_illus_group_id_;
}

int MapLogicIllustration::set_pre_illus_id(int illus_id)
{
	this->pre_illus_id_ = illus_id;
	return 0;
}
int MapLogicIllustration::set_pre_illus_class_id(int illus_class_id)
{
	this->pre_illus_class_id_ = illus_class_id;
	return 0;
}
int MapLogicIllustration::set_pre_illus_group_id(int illus_group_id)
{
	this->pre_illus_group_id_ = illus_group_id;
	return 0;
}


int MapLogicIllustration::fetch_single_illus_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11404004*, request, RETURN_REQUEST_SINGLE_ILLUS_INFO);
	int illus_id = request->illus_id();
	//work
//	const Json::Value& cfg = CONFIG_INSTANCE->label(illus_id);//need to change

	Proto51404004 respond;

	respond.set_illus_id(illus_id);


	const Json::Value &illus_json = CONFIG_INSTANCE->illus(illus_id);
	int cur_level = 0;
	if (this->get_illus_list_.count(illus_id) > 0)
		cur_level = this->get_illus_list_.find(illus_id)->second;
	JUDGE_RETURN(illus_json != Json::Value::null, -1);

	Illustrations *illus_temp = respond.mutable_illus_info();

	illus_temp->set_illus_id(illus_id);
	illus_temp->set_illus_group_id(illus_json["illus_group_id"].asInt());
	illus_temp->set_upgrade_goods_id(illus_json["upgrade_goods_id"].asInt());
	illus_temp->set_attr_type(attr_type_int(illus_json["attr_type"].asString()));
	illus_temp->set_open_level(illus_json["open_level"].asInt());
	illus_temp->set_illus_class_id(illus_json["illus_class_id"].asInt());
	illus_temp->set_cur_value(illus_json["value_list"][cur_level].asInt());
	illus_temp->set_upgrade_goods_amount(illus_json["goods_amount_list"][cur_level].asInt());
	illus_temp->set_illus_level(cur_level);


	FINER_PROCESS_RETURN(RETURN_REQUEST_SINGLE_ILLUS_INFO, &respond);

}


int MapLogicIllustration::sync_transfer_illus_info(int scene_id)
{
	Proto31400160 respond;

	respond.set_cur_class_id(this->open_); //改为开启状态
	IntSet::iterator it = this->get_group_list_.begin();
	int i = 0;
	for (; it != this->get_group_list_.end(); ++i, ++it)
	{
		respond.add_illus_group_list(*it);
	}

	i = 0;
	IntMap::iterator it1 = this->get_illus_list_.begin();
	for (; it1 != this->get_illus_list_.end();++i, ++it1)
	{
		respond.add_illus_list(it1->first);
		respond.add_illus_level_list(it1->second);
	}


//	MSG_DEBUG(SYNC_ILLUS:%s, respond.Utf8DebugString().c_str());
	return this->send_to_other_logic_thread(scene_id, respond);
}

int MapLogicIllustration::read_transfer_illus_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400160*, respond, -1);

	this->open_ = respond->cur_class_id();//改为开启状态

	/*
	//更新当前图鉴组集合 默认为第一个组
	const Json::Value &illus_class_json = CONFIG_INSTANCE->illus_class(this->cur_illus_class_id_);
	JUDGE_RETURN(illus_class_json != Json::Value::null, -1);

    this->illus_group_list_.clear();

    int group_size = illus_class_json["illus_group_list"].size();

    for (int i = 0; i < group_size; ++i)
    	this->illus_group_list_[i] = illus_class_json["illus_group_list"][i].asInt();

    //更新当前图鉴集合 默认为第一个集合
    int group_id = this->illus_group_list_[0];
    this->cur_illus_group_id_ = group_id;
	const Json::Value &illus_group_json = CONFIG_INSTANCE->illus_group(group_id);
	JUDGE_RETURN(illus_group_json != Json::Value::null, -1);

	this->illus_list_.clear();
	int illus_size = illus_group_json["illus_list"].size();
	for (int i = 0; i < illus_size; ++i)
		this->illus_list_[i] = illus_group_json["illus_list"][i].asInt();

	*/
    //更新已拥有组列表
	this->get_group_list_.clear();
	for (int i = 0; i < respond->illus_group_list_size(); ++i)
	{
		int group_id = respond->illus_group_list(i);

		this->get_group_list_.insert(group_id);
	}

	this->get_illus_list_.clear();
	for (int i = 0; i < respond->illus_list_size(); ++i)
	{
		int illus_id = respond->illus_list(i);
		int illus_level = respond->illus_level_list(i);
		this->get_illus_list_.insert(IntMap::value_type (illus_id, illus_level));
	}

	return 0;
}



