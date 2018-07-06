/*
 * MapEquipment.cpp
 *
 *  Created on: 2013-11-15
 *      Author: louis
 */

#include "MapEquipment.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "SerialRecord.h"
#include "GameCommon.h"
#include "GameFont.h"
#include "SerialDefine.h"
//#include "LogicGameSwitcherSys.h"

MapEquipment::MapEquipment():
		__last_request_tick(0)
{
	// TODO Auto-generated constructor stub
}

MapEquipment::~MapEquipment() {
	// TODO Auto-generated destructor stub
}

int MapEquipment::equip_red_item_pair()
{
	return 2;
}

string MapEquipment::equip_red_item_name(int index)
{
	static string CONF_NAME[] = {"item", "amount", "item2", "amount2"};
	return CONF_NAME[index];
}

int MapEquipment::equip_refine_item_pair()
{
	return 3;
}

string MapEquipment::equip_refine_item_name(int index)
{
	static string CONF_NAME[] = {"item_attack", "max_attack",
			"item_defence", "max_defence",
			"item_health", "max_health"};
	return CONF_NAME[index];
}

SmeltInfo& MapEquipment::smelt_info(void)
{
	return this->__smelt_info;
}

void MapEquipment::check_smelt_pa_event()
{
	IntVec smelt_temp;
	this->get_smelt_item(smelt_temp);

	MapLogicPlayer* player = this->map_logic_player();
	if (this->__smelt_info.__smelt_level == SmeltInfo::MAX_LEVEL)
		player->update_player_assist_single_event(GameEnum::PA_EVENT_SMELT, 0);
	else
		player->update_player_assist_single_event(GameEnum::PA_EVENT_SMELT, smelt_temp.size());
}

void MapEquipment::open_smelt(int task_id)
{
 	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_task(
 			"fun_equip",task_id) == true, ;);

	MapLogicPlayer * player = this->map_logic_player();
	player->update_player_assist_single_event(GameEnum::PA_EVENT_SMELT_OPEN, 1);

 	this->__smelt_info.open_ = true;
 	this->refresh_smelt_prop();
}

void MapEquipment::refresh_smelt_prop(int enter_type)
{
	JUDGE_RETURN(this->__smelt_info.open_ == true, ;);
	const Json::Value smelt_json = CONFIG_INSTANCE->equip_smelt_level(this->smelt_info().__smelt_level);
	IntMap prop_map;
    for(int prop_id = GameEnum::ATTR_BEGIN; prop_id <= GameEnum::ATTR_END; ++prop_id){
    	string prop_name = GameCommon::fight_prop_name(prop_id);
    	if(smelt_json.isMember(prop_name))
    	{
    		prop_map[prop_id] += smelt_json[prop_name].asInt();
    	}
    }

	this->refresh_fight_property(BasicElement::EQUIP_SMELT, prop_map, enter_type);
}

int MapEquipment::fetch_equip_smelt_panel_info()
{
	JUDGE_RETURN(this->__smelt_info.open_ == true, 0);
	Proto51400661 respond;
	respond.set_recommend(this->smelt_info().__recommend);
	respond.set_smelt_level(this->smelt_info().__smelt_level);
	respond.set_smelt_exp(this->smelt_info().__smelt_exp);
	const Json::Value smelt_json = CONFIG_INSTANCE->equip_smelt_level(this->smelt_info().__smelt_level);
	int exp_limit = smelt_json["exp_num"].asInt();
	respond.set_exp_limit(exp_limit);

	IntVec smelt_temp;
	this->get_smelt_item(smelt_temp);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);

	IntVec::iterator it = smelt_temp.begin();

	for (; it != smelt_temp.end(); ++it)
	{
		PackageItem* pack_item = package->find_by_index(*it);
		JUDGE_CONTINUE(pack_item != NULL);

		ProtoItem* temp = respond.add_item_list();
		pack_item->serialize(temp);
	}

	IntMap prop_map;
    for(int prop_id = GameEnum::ATTR_BEGIN; prop_id <= GameEnum::ATTR_END; ++prop_id){
    	string prop_name = GameCommon::fight_prop_name(prop_id);
    	if(smelt_json.isMember(prop_name))
    	{
    		prop_map[prop_id] += smelt_json[prop_name].asInt();
    	}
    }

	FightProperty temp;
	temp.unserialize(prop_map);
	temp.serialize(respond.mutable_prop_list());

	FINER_PROCESS_RETURN(RETURN_FETCH_EQUIP_SMELT_INFO, &respond);
}

int MapEquipment::calc_item_prop(int equip_id)
{
	const Json::Value &item_json = CONFIG_INSTANCE->item(equip_id);

	IntMap prop_map;

    for(int prop_id = GameEnum::ATTR_BEGIN; prop_id <= GameEnum::ATTR_END; ++prop_id){
    	string prop_name = GameCommon::fight_prop_name(prop_id);
    	if(item_json.isMember(prop_name))
    	{
    		prop_map[prop_id] += item_json[prop_name].asInt();
    	}
    }

	FightProperty temp;
	temp.unserialize(prop_map);

	return temp.force_;
}
int MapEquipment::calc_prop_map(int equip_id)
{
	const Json::Value &item_json = CONFIG_INSTANCE->item(equip_id);
	int type = item_json["part"].asInt();
	int main_part = (type / 100) * 10;
	if (main_part == 0) main_part = GameEnum::INDEX_EQUIP;
//	int sub_part = type % 100;
	GamePackage* package = this->find_package(main_part);
	JUDGE_RETURN(package != NULL, 1);

	for (ItemListMap::iterator iter = package->item_list_map_.begin();
								iter != package->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item != NULL);
		int item_id = pack_item->__id;
		int item_type = CONFIG_INSTANCE->item(item_id)["part"].asInt();

		if (type == item_type)
		{
			return calc_item_prop(equip_id) - calc_item_prop(item_id);
		}
	}
	//没有穿戴 不可熔炼
	return 1;
}

void MapEquipment::get_smelt_item(IntVec &smelt)
{

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
//	JUDGE_RETURN(package != NULL, -1);
	for (ItemListMap::iterator iter = package->item_list_map_.begin();
				iter != package->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item != NULL);
		int equip_id = pack_item->__id;
		if (GameCommon::item_is_equipment(equip_id))
		{
			//判断是否超过当前装备战力
			if (this->__smelt_info.__recommend == 1 && this->calc_prop_map(equip_id) > 0) continue;
			smelt.push_back(pack_item->__index);
		}
		else
		{
			const Json::Value &item_json = CONFIG_INSTANCE->item(equip_id);
			if (item_json["smelt_exp"].asInt() > 0 && item_json["smelt_vip_limit"].asInt() <= this->vip_detail().__vip_type)
			{
				smelt.push_back(pack_item->__index);
			}
		}
	}

}

int MapEquipment::change_smelt_recommend(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400662*, request, RETURN_CHANGE_SMELT_RECOMMEND);
	int recommend = request->status();
	this->__smelt_info.__recommend = (recommend + 1) % 2;
	Proto51400662 respond;
	IntVec smelt_temp;
	this->get_smelt_item(smelt_temp);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);

	IntVec::iterator it = smelt_temp.begin();

	for (; it != smelt_temp.end(); ++it)
	{
		PackageItem* pack_item = package->find_by_index(*it);
		JUDGE_CONTINUE(pack_item != NULL);

		ProtoItem* temp = respond.add_item_list();
		pack_item->serialize(temp);
	}

	respond.set_status(this->__smelt_info.__recommend);

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_EQUIP_SMELT);
	FINER_PROCESS_RETURN(RETURN_CHANGE_SMELT_RECOMMEND, &respond);
}

int MapEquipment::equip_smelt(int sum)
{
	const Json::Value &smelt_json = CONFIG_INSTANCE->equip_smelt_level(this->smelt_info().__smelt_level + 1);
	CONDITION_NOTIFY_RETURN(smelt_json.empty() == false, RETURN_EQUIP_SMELT, ERROR_SMELT_MOST_LEVEL);

	int upgrade_exp = CONFIG_INSTANCE->equip_smelt_level(this->smelt_info().__smelt_level)["exp_num"].asInt();

	this->__smelt_info.__smelt_exp += sum;

	int is_most = 0;
	while (this->smelt_info().__smelt_exp >= upgrade_exp)
	{
		this->smelt_info().__smelt_exp -= upgrade_exp;

		const Json::Value &smelt_js = CONFIG_INSTANCE->equip_smelt_level(this->smelt_info().__smelt_level + 1);
		if (smelt_js.empty())
		{
			is_most = 1;
			break;
		}
		this->smelt_info().__smelt_level ++;
		upgrade_exp = smelt_js["exp_num"].asInt();
	}

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_EQUIP_SMELT);
	Proto51400660 respond;
	respond.set_exp_num(this->smelt_info().__smelt_exp);
	respond.set_level_num(this->smelt_info().__smelt_level);
	const Json::Value &smelt_js = CONFIG_INSTANCE->equip_smelt_level(this->smelt_info().__smelt_level);
	int exp_limit = smelt_js["exp_num"].asInt();

	respond.set_exp_limit(exp_limit);
	respond.set_status(1);
	respond.set_exp_sum(sum);
	if (is_most)
	{
		this->smelt_info().__smelt_exp = exp_limit;
		respond.set_exp_num(exp_limit);
	}

	this->refresh_smelt_prop();
	this->check_smelt_pa_event();
	FINER_PROCESS_RETURN(RETURN_EQUIP_SMELT, &respond);
}

int MapEquipment::equip_smelt(Message* msg)
{
	JUDGE_RETURN(this->__smelt_info.open_ == true, 0);
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400660*, request, RETURN_EQUIP_SMELT);

	const Json::Value &smelt_json = CONFIG_INSTANCE->equip_smelt_level(this->smelt_info().__smelt_level + 1);
	CONDITION_NOTIFY_RETURN(smelt_json.empty() == false, RETURN_EQUIP_SMELT, ERROR_SMELT_MOST_LEVEL);

	int len = request->item_list_size();
	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);

	int sum = 0;
	IntMap equip_map;
	for (int i = 0; i < len ; ++i)
	{
		int item_index = request->item_list(i);

		PackageItem* pack_item = package->find_by_index(item_index);
		JUDGE_CONTINUE(pack_item != NULL);

		int item_id = pack_item->__id;
		int amount = pack_item->__amount;

		int exp = CONFIG_INSTANCE->item(item_id)["smelt_exp"].asInt();
		JUDGE_CONTINUE(exp > 0);

		if (GameCommon::item_is_equipment(item_id))
		{
			IntPair pair = pack_item->equip_part();
			equip_map[item_id] = pair.first;
		}

		this->pack_remove(package, ITEM_EQUIP_SMELT, pack_item);
		sum += exp * amount;
	}

	MapLogicPlayer* player = this->map_logic_player();
	for (IntMap::iterator iter = equip_map.begin(); iter != equip_map.end(); ++iter)
	{
		int mount_type = MLMounter::index_to_mount(iter->second);
		JUDGE_CONTINUE(mount_type > 0);

		player->check_pa_event_mount_equip(mount_type);
	}

	return this->equip_smelt(sum);
}

int MapEquipment::fetch_equip_refine_panel_info(Message* msg)
{
	return 0;
}

/*强化*/
int MapEquipment::equip_strengthen(Message* msg)
{
	static int shout_id = CONFIG_INSTANCE->const_set_conf(
			"shout_equip_strengthen")["arr"][0u].asInt();
	static int start_level = CONFIG_INSTANCE->const_set_conf(
			"shout_equip_strengthen")["arr"][1u].asInt();
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400601*, request, RETURN_EQUIPMENT_REFINE);

	uint pkg_index = request->pkg_index();
	uint auto_buy = request->is_auto_buy();

	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_EQUIPMENT_REFINE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(package->validate_pack_index(pkg_index) == true,
			RETURN_EQUIPMENT_REFINE, ERROR_CLIENT_OPERATE);

	PackageItem* item = package->find_by_index(pkg_index);
	CONDITION_NOTIFY_RETURN(item != NULL, RETURN_EQUIPMENT_REFINE, ERROR_CLIENT_OPERATE);

	PackGridInfo& pack_grid = package->grid_vec()[pkg_index];
	const Json::Value& strengthen_conf = CONFIG_INSTANCE->equip_strengthen(
			pkg_index, pack_grid.strengthen_lvl_);
	CONDITION_NOTIFY_RETURN(strengthen_conf.empty() == false, RETURN_EQUIPMENT_REFINE,
			ERROR_CONFIG_NOT_EXIST);

	int max_strength = 0;
	int color_index = item->red_grade();
	if (color_index <= 0)
	{
		max_strength = CONFIG_INSTANCE->const_set("new_max_strengthen");
	}
	else
	{
		const Json::Value& uprising_conf = CONFIG_INSTANCE->red_uprising(item->__index, color_index-1);
		CONDITION_NOTIFY_RETURN(uprising_conf.empty() == false, RETURN_EQUIPMENT_REFINE,
				ERROR_CONFIG_NOT_EXIST);

		max_strength = uprising_conf["max_strength"].asInt();
	}

	CONDITION_NOTIFY_RETURN(pack_grid.strengthen_lvl_ < max_strength,
			RETURN_EQUIPMENT_REFINE, ERROR_MAX_REFINE_LEVEL);

	UpgradeAmountInfo amount_info(this, strengthen_conf);
	CONDITION_NOTIFY_RETURN(amount_info.init_flag_ == true, RETURN_EQUIPMENT_REFINE,
			ERROR_MAX_REFINE_LEVEL);

	if (auto_buy == false)
	{
		CONDITION_NOTIFY_RETURN(amount_info.buy_amount_ == 0, RETURN_EQUIPMENT_REFINE,
				ERROR_PACKAGE_GOODS_AMOUNT);
	}

	if (amount_info.buy_amount_ > 0)
	{
		Money need_money;

		int ret = amount_info.total_money(need_money, this->own_money());
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_EQUIPMENT_REFINE,
				ERROR_PACKAGE_GOLD_AMOUNT);

		SerialObj money_serial(SUB_MONEY_FAST_BUY_REFINE, amount_info.buy_item_,
				amount_info.buy_amount_);

		ret = this->pack_money_sub(need_money, money_serial);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_EQUIPMENT_REFINE, ret);
	}

	if (amount_info.total_use_amount_ > 0)
	{
		this->pack_remove(SerialObj(ITEM_EQUIP_REFINE_USE, pkg_index),
				amount_info.item_id_[0], amount_info.total_use_amount_);
	}

	bool is_upgrade = false;
	if (GameCommon::validate_cur_rand(strengthen_conf["success"].asInt()) == true
			|| pack_grid.strengthen_bless_ >= strengthen_conf["max_fail"].asInt())
	{
		is_upgrade = true;
		pack_grid.strengthen_lvl_ += 1;
		pack_grid.strengthen_bless_ = 0;
		package->caculate_equip_force(pkg_index);
		package->notify_item_info(pkg_index);

		item->__equipment.strengthen_level_ = pack_grid.strengthen_lvl_;
		this->record_equip_item(item, EQUIP_STRENGTHEN);
		this->check_pa_event_jewel_all(0x01);
	}
	else
	{
		is_upgrade = false;
		pack_grid.strengthen_bless_ += strengthen_conf["fail"].asInt();
	}

	//传闻
	if (is_upgrade == true && pack_grid.strengthen_lvl_ >= start_level)
	{
		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, this->name());
		GameCommon::push_brocast_para_item_name(para_vec, item->__id);
		GameCommon::push_brocast_para_int(para_vec, pack_grid.strengthen_lvl_);
		this->announce(shout_id, para_vec);
	}

	this->sync_equip_prop_to_map();
	this->check_pa_event_equip_strengthen();
	this->check_pa_event_equip_red_uprising();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

	Proto51400601 respond;
	respond.set_pkg_index(pkg_index);
	respond.set_is_grade_up(is_upgrade);
	respond.set_strengthen_lvl(pack_grid.strengthen_lvl_);

	Proto31403200 task_info;
	task_info.set_task_id(GameEnum::CORNUCOPIA_TASK_EQUIP_REFINE);
	task_info.set_task_finish_count(1);
	MAP_MONITOR->dispatch_to_logic(this, &task_info);

	if (pkg_index == 0)
	{
		this->map_logic_player()->task_listen_branch(GameEnum::BRANCH_EQUIP,
				pack_grid.strengthen_lvl_);
	}

	FINER_PROCESS_RETURN(RETURN_EQUIPMENT_REFINE, &respond);
}

int MapEquipment::equip_red_uprising(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400602*, request, RETURN_EQUIPMENT_RED_UPRISING);

	GamePackage* equip_pkg = this->find_package(GameEnum::INDEX_EQUIP);
	CONDITION_NOTIFY_RETURN(equip_pkg != NULL, RETURN_EQUIPMENT_RED_UPRISING,
			ERROR_CLIENT_OPERATE);

	PackageItem* item = equip_pkg->find_by_index(request->pkg_index());
	CONDITION_NOTIFY_RETURN(item != NULL, RETURN_EQUIPMENT_RED_UPRISING,
			ERROR_CLIENT_OPERATE);

	int item_id = item->__id;
	int item_index = item->__index;

	int color_index = item->red_grade();
	CONDITION_NOTIFY_RETURN(color_index >= 0, RETURN_EQUIPMENT_RED_UPRISING,
			ERROR_CLIENT_OPERATE);

	const Json::Value& uprising_conf = CONFIG_INSTANCE->red_uprising(item->__index, color_index);
	CONDITION_NOTIFY_RETURN(uprising_conf.empty() == false, RETURN_EQUIPMENT_RED_UPRISING,
			ERROR_CONFIG_NOT_EXIST);

	IntMap need_map;
	for (int i = 0; i < MapEquipment::equip_red_item_pair(); i++)
	{
		string first_name = MapEquipment::equip_red_item_name(i * 2);
		string second_name = MapEquipment::equip_red_item_name(i * 2 + 1);

		int id = uprising_conf[first_name].asInt();
		int amount = uprising_conf[second_name].asInt();

		int ret = this->validate_pack_amount(id, amount);
		CONDITION_NOTIFY_RETURN(ret == true, RETURN_EQUIPMENT_RED_UPRISING,
				ERROR_CLIENT_OPERATE);

		need_map[id] = amount;
	}

	SerialObj serial(ITEM_RED_UPRISING, item_id, item_index);
	for (IntMap::iterator iter = need_map.begin(); iter != need_map.end(); ++iter)
	{
		this->pack_remove(serial, iter->first, iter->second);
	}

	int aim_id = item->conf()["equip_upgrade"].asInt();
	this->uprising_equip_item(item_id, EQUIP_RED_UPRADE);
	this->check_pa_event_equip_strengthen();
	this->check_pa_event_equip_red_uprising();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

	Proto51400602 respond;
	respond.set_aim_item_id(aim_id);
	respond.set_item_id(item_id);
	respond.set_pkg_index(item_index);
	FINER_PROCESS_RETURN(RETURN_EQUIPMENT_RED_UPRISING, &respond);
}

int MapEquipment::equip_orange_uprising(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto11400622*, request, RETURN_EQUIP_ORANGE_UPRISING);

	Proto51400622 respond;
	FINER_PROCESS_RETURN(RETURN_EQUIP_ORANGE_UPRISING, &respond);
}

int MapEquipment::equip_brighten(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400603*, request, RETURN_EQUIPMENT_BRIGHTEN);

	Proto51400603 respond;
	FINER_PROCESS_RETURN(RETURN_EQUIPMENT_BRIGHTEN, &respond);
}

int MapEquipment::equip_god_refine(Message* msg) //神炼
{
	return 0;
}

int MapEquipment::equip_god_refine_diff(Message* msg)
{
	return 0;
}

int MapEquipment::put_on_equip(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400604*, request, RETURN_PUT_ON_EQUIPMENT);

	this->put_on_equip(request->pkg_index());
	FINER_PROCESS_NOTIFY(RETURN_PUT_ON_EQUIPMENT);
}

int MapEquipment::put_on_equip(int pkg_index, int pkg_type)
{
	GamePackage* src_package = this->find_package(pkg_type);
	CONDITION_NOTIFY_RETURN(src_package != NULL, RETURN_PUT_ON_EQUIPMENT, ERROR_CLIENT_OPERATE);

	PackageItem* pack_item = src_package->find_by_index(pkg_index);
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_PUT_ON_EQUIPMENT, ERROR_EQUIP_NOT_EXIST);
	CONDITION_NOTIFY_RETURN(GameCommon::item_is_equipment(pack_item->__id) == true,
			RETURN_PUT_ON_EQUIPMENT, ERROR_CLIENT_OPERATE);

	MapLogicPlayer* player = this->map_logic_player();
	const Json::Value &item_json = pack_item->conf();

	int sex_limit = item_json["sex"].asInt();
	CONDITION_NOTIFY_RETURN(sex_limit == 0 || sex_limit == player->role_detail().__sex,
			RETURN_PUT_ON_EQUIPMENT, ERROR_EQUIP_SEX_LIMIT);

	int have_level 	= 0;
	int switch_type = 0;
	int aim_pack_type = 0;

	IntPair part = pack_item->equip_part();
	switch (part.first)
	{
	case 0:
	{
		//身上装备
		switch_type = 0;
		have_level = this->role_level();
		aim_pack_type = GameEnum::INDEX_EQUIP;
		break;
	}

	case GameEnum::INDEX_MOUNT:
	case GameEnum::INDEX_GOD_SOLIDER:
	case GameEnum::INDEX_MAGIC_WEAPON:
	case GameEnum::INDEX_XIANYU:
	case GameEnum::INDEX_BEAST:
	case GameEnum::INDEX_BEAST_WU:
	case GameEnum::INDEX_BEAST_WEAPON:
	case GameEnum::INDEX_BEAST_WING:
	case GameEnum::INDEX_BEAST_MAO:
	case GameEnum::INDEX_TIAN_GANG:
	{
		//战骑类
		switch_type = 1;
		aim_pack_type = part.first;

		int mount_type = MLMounter::index_to_mount(part.first);
		have_level = player->mount_detail(mount_type).mount_grade_;
		break;
	}

	default:
	{
		return this->respond_to_client_error(RETURN_PUT_ON_EQUIPMENT, ERROR_CLIENT_OPERATE);
	}
	}

	CONDITION_NOTIFY_RETURN(have_level >= item_json["use_lvl"].asInt(),RETURN_PUT_ON_EQUIPMENT,
			ERROR_EQUIP_LEVEL_LIMIT);

	int career_limit = item_json["career"].asInt();
	CONDITION_NOTIFY_RETURN(career_limit == 0 || career_limit == player->role_detail().__career,
			RETURN_PUT_ON_EQUIPMENT, ERROR_EQUIP_CAREER_LIMIT);

	GamePackage* aim_package = this->find_package(aim_pack_type);
	CONDITION_NOTIFY_RETURN(aim_package != NULL, RETURN_PUT_ON_EQUIPMENT, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(aim_package->validate_pack_index(part.second - 1) == true,
			RETURN_PUT_ON_EQUIPMENT, ERROR_CONFIG_ERROR);

	int ret = this->pack_move_between_package(pkg_index, pkg_type, part.second - 1,
			aim_pack_type, ITEM_PUT_ON_EQUIP, EQUIP_PUT_ON);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PUT_ON_EQUIPMENT, ret);

	switch (switch_type)
	{
	case 0:
	{
		player->refresh_equip_prop_to_map();
		player->refresh_player_equip_shape(part.second, pack_item->__id);
		break;
	}
	case 1:
	{
		int mount_type = MLMounter::index_to_mount(part.first);
		player->refresh_notify_mount_info(mount_type);
		player->check_pa_event_mount_equip(mount_type);
		player->record_mount(MOUNT_SER_EQUIP, mount_type);
		player->cache_tick().update_cache(MapLogicPlayer::CACHE_MOUNT);
		break;
	}
	}

	return 0;
}

int MapEquipment::take_off_equip(Message* msg)
{
	return 0;
}

int MapEquipment::fetch_magical_panel_info(void)
{
	return 0;
}

int MapEquipment::activate_magical_item(Message* msg)
{
	return 0;
}

int MapEquipment::fetch_magical_detail_info(Message* msg)
{
	return 0;
}

int MapEquipment::magical_polish(Message* msg)
{
	return 0;
}

int MapEquipment::magical_polish_clear_single_record(Message* msg)
{
	return 0;
}

int MapEquipment::magical_polish_select_result(Message* msg)
{
	return 0;
}

int MapEquipment::sync_transfer_equipment_info(int scene_id)
{
	Proto31400120 request;

	//同步熔炼信息
	request.set_smelt_level(this->__smelt_info.__smelt_level);
	request.set_smelt_exp(this->__smelt_info.__smelt_exp);
	request.set_recommend(this->__smelt_info.__recommend);
	request.set_open(this->__smelt_info.open_);
	return this->send_to_other_logic_thread(scene_id, request);
}

int MapEquipment::read_transfer_equipment_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400120*, respond, -1);
	this->__smelt_info.__recommend = respond->recommend();
	this->__smelt_info.__smelt_exp = respond->smelt_exp();
	this->__smelt_info.__smelt_level = respond->smelt_level();
	this->__smelt_info.open_ = respond->open();
	return 0;
}

void MapEquipment::map_logic_equipment_reset(void)
{
	this->__smelt_info.reset();
    this->fashion_id_set.clear();
}

int MapEquipment::make_up_equipment_detail(Proto50100156* &respond, GamePackage* package)
{
	JUDGE_RETURN(package != NULL, 0);

	ProtoEquipList *equip_list = respond->add_equip_list();
	equip_list->set_pack_type(package->type());

	ItemListMap::iterator it = package->item_list_map_.begin();
	for (; it != package->item_list_map_.end(); ++it)
	{
		JUDGE_CONTINUE(it->second != NULL);

		PackageItem* pack_item = it->second;
		ProtoItem* proto_item = equip_list->add_pack_item_list();
		pack_item->serialize(proto_item);
	}

	return 0;
}

int MapEquipment::make_up_equipment_list(Proto50100156* &respond)
{
	GamePackage* equip_package = this->find_package(GameEnum::INDEX_EQUIP);
	this->make_up_equipment_detail(respond, equip_package);

	GamePackage* mount_package = this->find_package(GameEnum::INDEX_MOUNT);
	this->make_up_equipment_detail(respond, mount_package);

	GamePackage* god_package = this->find_package(GameEnum::INDEX_GOD_SOLIDER);
	this->make_up_equipment_detail(respond, god_package);

	GamePackage* magic_package = this->find_package(GameEnum::INDEX_MAGIC_WEAPON);
	this->make_up_equipment_detail(respond, magic_package);

	GamePackage* wing_package = this->find_package(GameEnum::INDEX_XIANYU);
	this->make_up_equipment_detail(respond, wing_package);

	GamePackage* beast_package = this->find_package(GameEnum::INDEX_BEAST);
	this->make_up_equipment_detail(respond, beast_package);

	GamePackage* beast_wu_package = this->find_package(GameEnum::INDEX_BEAST_WU);
	this->make_up_equipment_detail(respond, beast_wu_package);

	GamePackage* beast_weapon_package = this->find_package(GameEnum::INDEX_BEAST_WEAPON);
	this->make_up_equipment_detail(respond, beast_weapon_package);

	GamePackage* beast_wing_package = this->find_package(GameEnum::INDEX_BEAST_WING);
	this->make_up_equipment_detail(respond, beast_wing_package);

	GamePackage* beast_mao_package = this->find_package(GameEnum::INDEX_BEAST_MAO);
	this->make_up_equipment_detail(respond, beast_mao_package);

	GamePackage* tian_gang_package = this->find_package(GameEnum::INDEX_TIAN_GANG);
	this->make_up_equipment_detail(respond, tian_gang_package);

	return 0;
}

void MapEquipment::calc_equip_property()
{
	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(package != NULL,);

	ItemListMap& item_map = package->item_map();
	for (ItemListMap::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
	{
		package->caculate_equip_force(iter->first);
	}
}

void MapEquipment::calc_equip_property(IntMap &prop_map)
{
	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(package != NULL,);

	int min_strengthen = INT_MAX;
	int min_red_grade = INT_MAX;
	int pack_size = package->size();

	IntMap gem_info;	// key: level, value: count
	FightProperty prop_info;
	PackGridVec& grid_vec = package->grid_vec();

	//装备属性
	for (int i = 0; i < pack_size; ++i)
	{
		min_strengthen = std::min(grid_vec[i].strengthen_lvl_, min_strengthen);

		PackageItem* item = package->find_by_index(i);
		JUDGE_CONTINUE(item != NULL);

		prop_info.add_fight_property(item->__prop_info);
		for (IntMap::iterator iter = item->__equipment.__jewel_detail.begin();
				iter != item->__equipment.__jewel_detail.end(); ++iter)
		{
			int level = iter->second % 100;
			JUDGE_CONTINUE(level > 1);

			for (int j = 2; j <= level; ++j)
			{
				gem_info[j] += 1;
			}
		}

		int red_grade = item->red_grade();
		if (red_grade < min_red_grade)
		{
			min_red_grade = red_grade;
		}
	}

	//强化套装属性
	if (min_strengthen > 0)
	{
		int suit_id = 10000 + 800 + min_strengthen;

		const Json::Value &suit_conf = CONFIG_INSTANCE->prop_suit(suit_id);
		prop_info.make_up_name_prop(suit_conf);
	}

	//宝石套装属性
	{
		int max_suit_id = 0;
		for (IntMap::iterator iter = gem_info.begin(); iter != gem_info.end(); ++iter)
		{
			for (int i = 1; i <= iter->second; ++i)
			{
				int suit_id = 20000 + i * 100 + iter->first;

				const Json::Value &suit_conf = CONFIG_INSTANCE->prop_suit(suit_id);
				JUDGE_CONTINUE(suit_conf.empty() == false);
				JUDGE_CONTINUE(suit_id > max_suit_id);

				max_suit_id = suit_id;
			}
		}

		if (max_suit_id > 0)
		{
			const Json::Value &suit_conf = CONFIG_INSTANCE->prop_suit(max_suit_id);
			prop_info.make_up_name_prop(suit_conf);
		}
	}

	//红装套装属性
	if (min_red_grade > 0)
	{
		int suit_id = 30000 + 800 + min_red_grade;

		const Json::Value &suit_conf = CONFIG_INSTANCE->prop_suit(suit_id);
		prop_info.make_up_name_prop(suit_conf);
	}


	prop_info.serialize(prop_map);
}

void MapEquipment::refresh_player_equip_shape(int part, int item_id, bool no_notify)
{
	switch (part)
	{
	case GameEnum::EQUIP_YIFU:
	case GameEnum::EQUIP_WEAPON:
	{
		break;
	}
	default:
	{
		return;
	}
	}

	if (item_id == -1)
	{
		item_id = this->pack_fetch_item_id(part - 1, GameEnum::INDEX_EQUIP);
	}

	Proto30400601 request;
	request.set_no_notify(no_notify);

	ProtoPairObj* proto_pair = request.add_shape_list();
	proto_pair->set_obj_id(part);
	proto_pair->set_obj_value(item_id);

	this->send_to_map_thread(request);
}

int MapEquipment::fetch_jewel_prop_type(int item_id)
{
	return (item_id / 10000) % 100;
}

string MapEquipment::fetch_jewel_prop_name(int prop_type)
{
	static string gem_name[] = {"gem_health", "gem_attack", "gem_defence"};
	return gem_name[prop_type - 1];
}

int MapEquipment::equip_inherit(Message* msg)	//装备继承
{
	return 0;
}
int MapEquipment::equip_tempered(Message* msg)	//装备淬练
{
	return 0;
}

int MapEquipment::equip_insert_jewel(Message* msg)	//装备镶嵌宝石
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400617*, request, RETURN_EQUIP_INSERT_JEWEL);

	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, -1);

	GamePackage* bg_pack = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(bg_pack != NULL, -1);

	int equip_index = request->equip_index();
	int jewal_index = request->jewal_index();

	int str_level = equip_pack->strengthen_level(equip_index);
	CONDITION_NOTIFY_RETURN(str_level > 0, RETURN_EQUIP_INSERT_JEWEL, ERROR_CLIENT_OPERATE);

	const Json::Value& str_conf = CONFIG_INSTANCE->equip_strengthen(equip_index, str_level);
	CONDITION_NOTIFY_RETURN(str_conf.empty() == false, RETURN_EQUIP_INSERT_JEWEL, ERROR_CLIENT_OPERATE);

	PackageItem* equip = equip_pack->find_by_index(equip_index);
	CONDITION_NOTIFY_RETURN(equip != NULL, RETURN_EQUIP_INSERT_JEWEL, ERROR_CLIENT_OPERATE);

	PackageItem* jewal = bg_pack->find_by_index(jewal_index);
	CONDITION_NOTIFY_RETURN(jewal != NULL, RETURN_EQUIP_INSERT_JEWEL, ERROR_CLIENT_OPERATE);

	const Json::Value& jewal_conf = jewal->conf();
	CONDITION_NOTIFY_RETURN(jewal_conf["sub_type"].asInt() == GameEnum::ITEM_SUB_TYPE_JEWAL,
			RETURN_EQUIP_INSERT_JEWEL, ERROR_CLIENT_OPERATE);

	int prop_type = MapEquipment::fetch_jewel_prop_type(jewal->__id);
	CONDITION_NOTIFY_RETURN(prop_type > 0 && prop_type <= 3, RETURN_EQUIP_INSERT_JEWEL,
			ERROR_CLIENT_OPERATE);

	string prop_name = MapEquipment::fetch_jewel_prop_name(prop_type);
	CONDITION_NOTIFY_RETURN(str_conf[prop_name].asInt() == 1,
			RETURN_EQUIP_INSERT_JEWEL, ERROR_CLIENT_OPERATE);

	int src_id = 0;
	Equipment& equipment = equip->__equipment;

	if (equipment.__jewel_detail.count(prop_type) > 0)
	{
		src_id = equipment.__jewel_detail[prop_type];
	}

	//新宝石
	equipment.__jewel_detail[prop_type] = jewal->__id;
	this->pack_remove(bg_pack, ITEM_INSERT_REMOVE_JEWEL, jewal, 1);

	//旧宝石
	if (src_id > 0)
	{
		this->pack_insert(bg_pack, ADD_FROM_TAKE_OFF_JEWEL, ItemObj(src_id, 1, true));
	}
	else
	{
		this->check_pa_event_jewel_all();
	}

	this->refresh_equip_prop_to_map();
	this->notify_pack_info(GameEnum::INDEX_EQUIP);
	this->record_equip_item(equip, EQUIP_JEWEL_INSERT);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

	//宝石升华相关
	this->validate_open_sublime_jewel();

	Proto51400617 respond;
	respond.set_equip_id(equip->__id);
	FINER_PROCESS_RETURN(RETURN_EQUIP_INSERT_JEWEL,&respond);
}

int MapEquipment::equip_good_refine(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400624*, request, RETURN_EQUIP_GOOD_REFINE);

	int pkg_index = request->pkg_index();
	int total_pair = MapEquipment::equip_refine_item_pair();

	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_EQUIP_GOOD_REFINE, ERROR_CLIENT_OPERATE);

	PackageItem* dst_item = package->find_by_index(pkg_index);
	CONDITION_NOTIFY_RETURN(dst_item != NULL, RETURN_EQUIP_GOOD_REFINE, ERROR_CLIENT_OPERATE);

	const Json::Value& dst_conf = dst_item->conf();
	CONDITION_NOTIFY_RETURN(dst_conf.empty() == false, RETURN_EQUIP_GOOD_REFINE, ERROR_CLIENT_OPERATE);

	int add_flag = 0;
	int red_flag = 0;
	int error_code = 0;

	IntMap item_map;
	const Json::Value& color_conf = CONFIG_INSTANCE->equip_refine(dst_conf["color"].asInt());

	for (int i = 0; i < total_pair; ++i)
	{
		string first_name = MapEquipment::equip_refine_item_name(i * 2);
		string second_name = MapEquipment::equip_refine_item_name(i * 2 + 1);

		int item_id = color_conf[first_name].asInt();
		int max_amount = color_conf[second_name].asInt();

		ItemObj& item_obj = dst_item->__equipment.__good_refine[item_id];
		item_obj.id_ = item_id;

		int have_amount = this->pack_count(item_id);
		if (have_amount == 0)
		{
			// no amount
			error_code = ERROR_PACKAGE_GOODS_AMOUNT;
			continue;
		}

		int differ_amount = max_amount - item_obj.amount_;
		if (differ_amount <= 0)
		{
			// full
			error_code = ERROR_MOUNT_GOODS_FULL;
			continue;
		}

		int use_amount = std::min<int>(1, differ_amount);
		this->pack_remove(SerialObj(ITEM_GOOD_REFINE_USE, pkg_index), item_id, use_amount);

		item_obj.amount_ += use_amount;

		add_flag = true;
		item_map[item_id] = true;
	}

	for (IntMap::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
	{
		red_flag = this->equip_good_refine_is_operate(iter->first % 10 - 1);
		JUDGE_CONTINUE(red_flag == true);
		break;
	}

	if (add_flag == true)
	{
		error_code = 0;
		this->refresh_equip_prop_to_map();
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

		this->notify_pack_info(GameEnum::INDEX_EQUIP);	//通知更新装备信息
		this->record_equip_item(dst_item, EQUIP_GOOD_REFINE);

		this->map_logic_player()->update_player_assist_single_event(
		    		GameEnum::PA_EVENT_EQUIP_GOOD_REFINE, red_flag);
	}

	Proto51400624 respond;
	respond.set_pkg_index(pkg_index);
	return this->respond_to_client_error(RETURN_EQUIP_GOOD_REFINE, error_code, &respond);
}

int MapEquipment::equip_polish(Message* msg)			//装备洗练
{
	return 0;
}

int MapEquipment::equip_polish_fetch_attr_list(Message* msg)	//装备洗练时获取需要进行洗练的属性列表
{
	return 0;
}

int MapEquipment::equip_polish_replace_attr(Message* msg)	//装备洗练：属性替换
{
	return 0;
}

int MapEquipment::equip_compose(Message* msg)	//装备合成
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400625*, request, RETURN_EQUIP_COMPSE);

	const Json::Value &compose_conf = CONFIG_INSTANCE->equip_compse(request->id());
	CONDITION_NOTIFY_RETURN(compose_conf.empty() == false, RETURN_EQUIP_COMPSE,
			ERROR_CONFIG_NOT_EXIST);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, -1);

	ItemObjVec need_vec;
	GameCommon::make_up_conf_items(need_vec, compose_conf["materials"]);
	CONDITION_NOTIFY_RETURN(need_vec.empty() == false, RETURN_EQUIP_COMPSE,
			ERROR_CONFIG_NOT_EXIST);

	int item_id = need_vec[0].id_;
	int item_amount = need_vec[0].amount_ * 4;

	need_vec.clear();
	need_vec.push_back(ItemObj(item_id, item_amount));

	int ret = this->pack_remove(package, ITEM_EQUIP_COMPOSE_USE, need_vec);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_EQUIP_COMPSE, ret);

	int output = compose_conf["output"].asInt();
	this->pack_insert(package, ADD_FROM_ITEM_COMPOSE, ItemObj(output, 1, true));
	this->check_pa_event_jewel_all();

	Proto31400058 request_reward;
	request_reward.set_synthesis_item_id(output);
	MAP_MONITOR->dispatch_to_logic(this, &request_reward);

	FINER_PROCESS_NOTIFY(RETURN_EQUIP_COMPSE);
}

int MapEquipment::equip_decompose(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto11400623 *, request, RETURN_EQUIP_DECOMPOSE);
    return 0;
}

int MapEquipment::equip_remove_jewel(Message* msg)	//镶嵌的宝石卸下
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400626*, request, RETURN_EQUIP_REMOVE_JEWEL);

	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, -1);

	GamePackage* bg_pack = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(bg_pack != NULL, -1);

	int equip_index = request->equip_index();
	int jewal_id = request->jewal_id();
	CONDITION_NOTIFY_RETURN(jewal_id > 0, RETURN_EQUIP_REMOVE_JEWEL, ERROR_CLIENT_OPERATE);

	int str_level = equip_pack->strengthen_level(equip_index);
	CONDITION_NOTIFY_RETURN(str_level > 0, RETURN_EQUIP_REMOVE_JEWEL, ERROR_CLIENT_OPERATE);

	const Json::Value& str_conf = CONFIG_INSTANCE->equip_strengthen(equip_index, str_level);
	CONDITION_NOTIFY_RETURN(str_conf.empty() == false, RETURN_EQUIP_REMOVE_JEWEL, ERROR_CLIENT_OPERATE);

	PackageItem* equip = equip_pack->find_by_index(equip_index);
	CONDITION_NOTIFY_RETURN(equip != NULL, RETURN_EQUIP_REMOVE_JEWEL, ERROR_CLIENT_OPERATE);

	int prop_type = MapEquipment::fetch_jewel_prop_type(jewal_id);
	CONDITION_NOTIFY_RETURN(prop_type > 0 && prop_type <= 3, RETURN_EQUIP_REMOVE_JEWEL,
			ERROR_CLIENT_OPERATE);

	string prop_name = MapEquipment::fetch_jewel_prop_name(prop_type);
	CONDITION_NOTIFY_RETURN(str_conf[prop_name].asInt() == 1,
			RETURN_EQUIP_REMOVE_JEWEL, ERROR_CLIENT_OPERATE);

	Equipment& equipment = equip->__equipment;
	IntMap::iterator pos = equipment.__jewel_detail.find(prop_type);
	CONDITION_NOTIFY_RETURN(pos != equipment.__jewel_detail.end(),
			RETURN_EQUIP_REMOVE_JEWEL, ERROR_CLIENT_OPERATE);

	equipment.__jewel_detail.erase(pos);
	this->pack_insert(bg_pack, ADD_FROM_EQUIP_JEWEL_REMOVE, ItemObj(jewal_id, 1, true));

	this->refresh_equip_prop_to_map();
	this->notify_pack_info(GameEnum::INDEX_EQUIP);
	this->record_equip_item(equip, EQUIP_JEWEL_REMOVE);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

	Proto51400626 respond;
	respond.set_equip_id(equip->__id);
	FINER_PROCESS_RETURN(RETURN_EQUIP_REMOVE_JEWEL,&respond);
}

int MapEquipment::equip_upgrade_jewel(Message* msg)		//镶嵌的宝石升级
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400628*, request, RETURN_EQUIP_UPGRADE_JEWEL);

	int equip_index = request->equip_index();

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, -1);

	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, -1);

	const Json::Value &jewel_upgrade_conf = CONFIG_INSTANCE->jewel_upgrade(request->id());
	CONDITION_NOTIFY_RETURN(jewel_upgrade_conf.empty() == false, RETURN_EQUIP_UPGRADE_JEWEL,
			ERROR_CONFIG_NOT_EXIST);

	PackageItem* equip = equip_pack->find_by_index(equip_index);
	CONDITION_NOTIFY_RETURN(equip != NULL, RETURN_EQUIP_UPGRADE_JEWEL, ERROR_CLIENT_OPERATE);

	int str_level = equip_pack->strengthen_level(equip_index);
	CONDITION_NOTIFY_RETURN(str_level > 0, RETURN_EQUIP_UPGRADE_JEWEL, ERROR_CLIENT_OPERATE);

	const Json::Value& str_conf = CONFIG_INSTANCE->equip_strengthen(equip_index, str_level);
	CONDITION_NOTIFY_RETURN(str_conf.empty() == false, RETURN_EQUIP_UPGRADE_JEWEL,
			ERROR_CLIENT_OPERATE);

	int output_item_id = jewel_upgrade_conf["output"].asInt();
	int prop_type = MapEquipment::fetch_jewel_prop_type(output_item_id);
	CONDITION_NOTIFY_RETURN(prop_type > 0 && prop_type <= 3, RETURN_EQUIP_UPGRADE_JEWEL,
			ERROR_CLIENT_OPERATE);

	string prop_name = MapEquipment::fetch_jewel_prop_name(prop_type);
	CONDITION_NOTIFY_RETURN(str_conf[prop_name].asInt() == 1, RETURN_EQUIP_UPGRADE_JEWEL,
			ERROR_CLIENT_OPERATE);

	ItemObjVec need_vec;
	GameCommon::make_up_conf_items(need_vec, jewel_upgrade_conf["materials"]);
	CONDITION_NOTIFY_RETURN(need_vec.empty() == false, RETURN_EQUIP_UPGRADE_JEWEL,
			ERROR_CONFIG_NOT_EXIST);

	int item_id = need_vec[0].id_;
	int item_amount = 0;
	for (int i = 0; i < (int)need_vec.size(); ++i)
		item_amount += need_vec[0].amount_;
	CONDITION_NOTIFY_RETURN(item_amount > 0, RETURN_EQUIP_UPGRADE_JEWEL, ERROR_CONFIG_NOT_EXIST);

	int fixed_amount = item_amount;
	item_amount -= 1;	//计算数量时将已经镶嵌上去的旧宝石计算进去

	int cond = jewel_upgrade_conf["cond"].asInt();
	int jewel_level = item_id % 100;
	CONDITION_NOTIFY_RETURN(jewel_level >= cond, RETURN_EQUIP_UPGRADE_JEWEL, ERROR_CLIENT_OPERATE);

	int flag = false;
	int need_amount = 0;
	need_vec.clear();

	/**********************************************************/
	IntMap trans_map;
	for (int i = 1; i < jewel_level; ++i)
		trans_map[i] = (int)pow(fixed_amount, jewel_level - i) * item_amount;
	trans_map[jewel_level] = item_amount;

	IntMap pack_map;
	for (int i = 1; i <= jewel_level; ++i)
	{
		int tar_jewel_id = item_id - jewel_level + i;
		ItemVector item_set;
		JUDGE_CONTINUE(package->find_all(item_set, tar_jewel_id) == true);
		for (ItemVector::iterator iter = item_set.begin(); iter != item_set.end(); ++iter)
			pack_map[i] += (*iter)->__amount;
	}

	for (int i = 1; i < jewel_level; ++i)
	{
		need_amount = 0;
		need_vec.clear();
		int base_num = 0;

		for (int j = 1; j <= i; ++j)
		{
			int tar_jewel_id = item_id - jewel_level + j;
			int base_num_buf = base_num;
			base_num = base_num + pack_map[j];
			int need_pack_amount = base_num - base_num % fixed_amount;
			need_pack_amount -= base_num_buf;
			if (need_pack_amount > 0)
				need_vec.push_back(ItemObj(tar_jewel_id, need_pack_amount));

			base_num = base_num / fixed_amount;
		}

		need_amount = base_num / (int)pow(fixed_amount, jewel_level - i - 1);
		JUDGE_CONTINUE(need_amount >= item_amount);

		flag = true;
		break;
	}

	if (flag == false)
	{
		int defference = item_amount - need_amount;
		CONDITION_NOTIFY_RETURN(pack_map[jewel_level] >= defference, RETURN_EQUIP_UPGRADE_JEWEL,
					ERROR_CLIENT_OPERATE);
		need_vec.push_back(ItemObj(item_id, defference));
	}

	/**********************************************************/

	CONDITION_NOTIFY_RETURN(need_vec.empty() == false, RETURN_EQUIP_UPGRADE_JEWEL,
			ERROR_CLIENT_OPERATE);

	int ret = this->pack_remove(package, ITEM_EQUIP_JEWEL_UPGRADE, need_vec);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_EQUIP_UPGRADE_JEWEL, ret);

	Equipment& equipment = equip->__equipment;
	equipment.__jewel_detail[prop_type] = output_item_id;

	this->check_pa_event_jewel_all();
	this->refresh_equip_prop_to_map();
	this->notify_pack_info(GameEnum::INDEX_EQUIP);
	this->record_equip_item(equip, EQUIP_JEWEL_UPGRADE);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

	Proto31400058 request_reward;
	request_reward.set_synthesis_item_id(output_item_id);
	MAP_MONITOR->dispatch_to_logic(this, &request_reward);

	Proto51400628 respond;
	for (ItemObjVec::iterator iter = need_vec.begin(); iter != need_vec.end(); ++iter)
	{
		ProtoGemInfo *gem_info_ptr = respond.add_consume_gem_info();
		JUDGE_CONTINUE(0 != gem_info_ptr);
		gem_info_ptr->set_gem_id(iter->id_);
		gem_info_ptr->set_gem_amount(iter->amount_);
	}
	respond.set_equip_id(equip->__id);
	FINER_PROCESS_RETURN(RETURN_EQUIP_UPGRADE_JEWEL,&respond);
}


int MapEquipment::validate_open_sublime_jewel()		//效验宝石升华功能是否可以开启
{
	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, -1);
	JUDGE_RETURN(equip_pack->is_open_sublime() == 0, 0);

	const Json::Value &jewel_sublime_conf = CONFIG_INSTANCE->jewel_sublime(1);
	JUDGE_RETURN(jewel_sublime_conf.empty() == false, -1);

	int jewel_level_of_cond = jewel_sublime_conf["cond"].asInt();

	ItemListMap& item_list_map = equip_pack->item_list_map_;
	JUDGE_RETURN(GameEnum::EQUIP_MAX_INDEX == item_list_map.size(), -1);

	for (ItemListMap::iterator iter = item_list_map.begin(); iter != item_list_map.end(); ++iter)
	{
		PackageItem *pack_item = iter->second;
		JUDGE_RETURN(pack_item != NULL, -1);

		IntMap &jewel_map = pack_item->__equipment.__jewel_detail;
		JUDGE_RETURN(GameEnum::EQUIP_JEWEL_MAX_INDEX == jewel_map.size(), -1);

		for (IntMap::iterator jew_it = jewel_map.begin(); jew_it != jewel_map.end(); ++jew_it)
		{
			int jew_level = jew_it->second % 100;
			JUDGE_RETURN(jew_level >= jewel_level_of_cond, -1);
		}
	}

	equip_pack->is_open_sublime_ = 1;

	Proto51400627 respond;
	respond.set_is_open_sublime(equip_pack->is_open_sublime());
	FINER_PROCESS_RETURN(RETURN_EQUIP_SUBLIME_JEWEL_INFO,&respond);
}

int MapEquipment::fetch_equip_sublime_jewel_info()		//获取宝石升华信息
{
	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, -1);

	int level = equip_pack->sublime_level();
	if (level == 0)
	{
		Proto51400627 respond;
		respond.set_is_open_sublime(equip_pack->is_open_sublime());
		FINER_PROCESS_RETURN(RETURN_EQUIP_SUBLIME_JEWEL_INFO,&respond);
	}

	//计算属性
	Int64 blood_max = 0, attack = 0, defense = 0;
	CONDITION_NOTIFY_RETURN(this->calc_equip_attached_property(blood_max, attack, defense) == true,
			RETURN_EQUIP_SUBLIME_JEWEL_INFO, ERROR_CONFIG_NOT_EXIST);

	//计算概率
	int add_attr_rate = 0;
	for (int i = 1; i <= level; ++i)
	{
		const Json::Value &jewel_sublime_conf = CONFIG_INSTANCE->jewel_sublime(i);
		add_attr_rate += jewel_sublime_conf["add_attr_rate"].asInt();
	}

	//计算战斗力
	FightProperty prop_info;
	prop_info.make_up_one_prop(GameName::ATTACK, attack);
	prop_info.make_up_one_prop(GameName::BLOOD_MAX, blood_max);
	prop_info.make_up_one_prop(GameName::DEFENSE, defense);
	int fight_force = prop_info.caculate_force();

	this->refresh_jewel_sublime_prop();

	Proto51400627 respond;
	respond.set_level(level);
	respond.set_health(blood_max);
	respond.set_attack(attack);
	respond.set_defense(defense);
	respond.set_fight_force(fight_force);
	respond.set_attr_add_rate(add_attr_rate);
	respond.set_is_open_sublime(equip_pack->is_open_sublime());
	FINER_PROCESS_RETURN(RETURN_EQUIP_SUBLIME_JEWEL_INFO,&respond);
}

//计算身上的装备镶嵌后的附加属性
bool MapEquipment::calc_equip_attached_property(Int64 &blood_max, Int64 &attack, Int64 &defense)
{
	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, false);

	int level = equip_pack->sublime_level();
	int add_attr_rate = 0;
	for (int i = 1; i <= level; ++i)
	{
		const Json::Value &jewel_sublime_conf = CONFIG_INSTANCE->jewel_sublime(i);
		add_attr_rate += jewel_sublime_conf["add_attr_rate"].asInt();
	}

	Int64 blood_max_ = 0, attack_ = 0, defense_ = 0;

	if (add_attr_rate == 0)
	{
		blood_max = blood_max_;
		attack = attack_;
		defense = defense_;
		MSG_USER("because of add_attr_rate == 0, so not calculated");
		return true;
	}

	ItemListMap & item_list_map = equip_pack->item_list_map_;
	for (ItemListMap::iterator iter = item_list_map.begin(); iter != item_list_map.end(); ++iter)
	{
		PackageItem *pack_item = iter->second;
		IntMap &jewel_map = pack_item->__equipment.__jewel_detail;
		for (IntMap::iterator jew_it = jewel_map.begin(); jew_it != jewel_map.end(); ++jew_it)
		{
			int jewel_id = jew_it->second;

			const Json::Value& jewel_prop_conf = CONFIG_INSTANCE->prop_unit(jewel_id);
			FightProperty fight_prop;

			fight_prop.make_up_name_prop(jewel_prop_conf);
			blood_max_ += fight_prop.blood_max_;
			attack_ +=  fight_prop.attack_;
			defense_ += fight_prop.defence_;
		}
	}

	blood_max = blood_max_ * add_attr_rate / 100;
	attack = attack_ * add_attr_rate / 100;
	defense = defense_ * add_attr_rate / 100;
	MSG_USER("calculate equip attached property:[blood_max:%d,attack:%d,defense:%d]",blood_max, attack, defense);
	return true;
}

int MapEquipment::refresh_jewel_sublime_prop(int enter_type)
{
	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, -1);
	JUDGE_RETURN(equip_pack->is_open_sublime() == 1, 0);

	Int64 blood_max = 0, attack = 0, defense = 0;
	JUDGE_RETURN(this->calc_equip_attached_property(blood_max, attack, defense) == true, 0);

	FightProperty prop_info;
	prop_info.make_up_one_prop(GameName::ATTACK, attack);
	prop_info.make_up_one_prop(GameName::BLOOD_MAX, blood_max);
	prop_info.make_up_one_prop(GameName::DEFENSE, defense);

	IntMap prop_map;
	prop_info.serialize(prop_map);

	this->refresh_fight_property(BasicElement::JEWEL_SUBLIME, prop_map, enter_type);
	return 0;
}

int MapEquipment::equip_sublime_jewel()	//镶嵌的宝石升华操作
{
	CONDITION_NOTIFY_RETURN(GameCommon::validate_time_span(
			this->__last_request_tick, Time_Value::SECOND),
			RETURN_EQUIP_SUBLIME_JEWEL, ERROR_OPERATE_TOO_FAST);
	this->__last_request_tick = ::time(0);

	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, -1);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package != NULL, -1);

	const GameConfig::ConfigMap &jewel_sublime_map = CONFIG_INSTANCE->jewel_sublime_map();
	int level = equip_pack->sublime_level() + 1;
	JUDGE_RETURN(jewel_sublime_map.size() >= (std::size_t)level, -1);

	const Json::Value &jewel_sublime_conf = CONFIG_INSTANCE->jewel_sublime(level);
	CONDITION_NOTIFY_RETURN(jewel_sublime_conf.empty() == false, RETURN_EQUIP_SUBLIME_JEWEL,
			ERROR_CONFIG_NOT_EXIST);

	//消耗
	ItemObjVec need_vec;
	GameCommon::make_up_conf_items(need_vec, jewel_sublime_conf["materials"]);
	int item_id = need_vec[0].id_;
	int item_amount = need_vec[0].amount_;

	need_vec.clear();
	need_vec.push_back(ItemObj(item_id, item_amount));

	int ret = this->pack_remove(package, ITEM_EQUIP_JEWEL_SUBLIME, need_vec);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_EQUIP_SUBLIME_JEWEL, ret);

	//奖励
	int reward_id = jewel_sublime_conf["reward_id"].asInt();
	this->add_reward(reward_id, SerialObj(ADD_FROM_EQUIP_JEWEL_SUBLIME));

	equip_pack->sublime_level_ = level;

	this->fetch_equip_sublime_jewel_info();

	FINER_PROCESS_NOTIFY(RETURN_EQUIP_SUBLIME_JEWEL);
}

//身上是否有可强化的装备
bool MapEquipment::equip_strengthen_is_operate()
{
	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(package != NULL, false);

	static int STRENGTH_LEVEL = CONFIG_INSTANCE->red_tips(
			GameEnum::PA_EVENT_EQUIP_STRENGTHEN)["sub1"].asInt();

	for (int i = 0; i < package->size(); ++i)
	{
		PackGridInfo* pack_grid = package->grid_info(i);
		JUDGE_CONTINUE(pack_grid->strengthen_lvl_ < STRENGTH_LEVEL);

		const Json::Value& conf = CONFIG_INSTANCE->equip_strengthen(
				i, pack_grid->strengthen_lvl_);
		JUDGE_CONTINUE(conf.empty() == false);

		UpgradeAmountInfo amount_info(this, conf);
		JUDGE_CONTINUE(amount_info.init_flag_ == true);
		JUDGE_CONTINUE(amount_info.buy_amount_ == 0);

		return true;
	}

	return false;
}

//身上或背包是否有可升阶的装备
bool MapEquipment::equip_red_uprising_is_operate()
{
	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	for (int i = 0; i < package->size(); ++i)
	{
		PackageItem* item = package->find_by_index(i);
		JUDGE_CONTINUE(item != NULL);

		int color_index = item->red_grade();
		JUDGE_CONTINUE(color_index >= 0);

		const Json::Value& uprising_conf = CONFIG_INSTANCE->red_uprising(
				item->__index, color_index);
		JUDGE_CONTINUE(uprising_conf.empty() == false);

		int flag = true;
		for (int j = 0; j < MapEquipment::equip_red_item_pair(); j++)
		{
			string first_name = MapEquipment::equip_red_item_name(j * 2);
			string second_name = MapEquipment::equip_red_item_name(j * 2 + 1);

			int id = uprising_conf[first_name].asInt();
			int amount = uprising_conf[second_name].asInt();
			JUDGE_CONTINUE(this->validate_pack_amount(id, amount) == false);

			flag = false;
			break;
		}

		if (flag == true)
		{
			return true;
		}
	}

	return false;
}

bool MapEquipment::equip_good_refine_is_operate(int type)
{
	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	for (int i = 0; i < package->size(); ++i)
	{
		PackageItem* item = package->find_by_index(i);
		JUDGE_CONTINUE(item != NULL);

		const Json::Value& color_conf = CONFIG_INSTANCE->equip_refine(
				item->conf()["color"].asInt());

		string first_name = MapEquipment::equip_refine_item_name(type * 2);
		string second_name = MapEquipment::equip_refine_item_name(type * 2 + 1);

		int item_id = color_conf[first_name].asInt();
		int max_amount = color_conf[second_name].asInt();

		int have_amount = this->pack_count(item_id);
		JUDGE_CONTINUE(have_amount > 0);

		ItemObj& item_obj = item->__equipment.__good_refine[item_id];
		JUDGE_CONTINUE(item_obj.amount_ < max_amount);

		return true;
	}

	return false;
}

// 身上或背包是否有可橙炼的装备
bool MapEquipment::equip_orange_uprising_is_operate(int pack_type)
{
	return false;
}

//身上或背包是否有可淬练的装备
bool MapEquipment::equip_tempered_is_operate()
{
	return false;
}
//身上是否有可洗练的装备
bool MapEquipment::equip_polish_is_operate()
{
	return false;
}

//身上是否有可镶嵌宝石的装备
bool MapEquipment::equip_insert_jewel_is_operate(int item_id)
{
	GamePackage* equip_pack = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(equip_pack != NULL, false);

	const Json::Value& jewal_conf = CONFIG_INSTANCE->item(item_id);
	JUDGE_RETURN(jewal_conf["sub_type"].asInt() == GameEnum::ITEM_SUB_TYPE_JEWAL, false);

	int prop_type = MapEquipment::fetch_jewel_prop_type(item_id);
	JUDGE_RETURN(prop_type > 0 && prop_type <= 3, false);

	string prop_name = MapEquipment::fetch_jewel_prop_name(prop_type);
	int total_size = equip_pack->size();

	for (int i = 0; i < total_size; ++i)
	{
		PackageItem* equip = equip_pack->find_by_index(i);
		JUDGE_CONTINUE(equip != NULL);

		int str_level = equip_pack->strengthen_level(i);
		JUDGE_CONTINUE(str_level > 0);

		const Json::Value& str_conf = CONFIG_INSTANCE->equip_strengthen(i, str_level);
		JUDGE_CONTINUE(str_conf[prop_name].asInt() == 1);

		Equipment& equipment = equip->__equipment;
		if (equipment.__jewel_detail.count(prop_type) == 0)
		{
			return true;
		}

		if (equipment.__jewel_detail[prop_type] < item_id)
		{
			return true;
		}
	}

	return false;
}

bool MapEquipment::equip_jewel_combine_is_operate(int item_id)	//身上是否有可合成宝石
{
	const Json::Value& jewal_conf = CONFIG_INSTANCE->item(item_id);
	JUDGE_RETURN(jewal_conf["sub_type"].asInt() == GameEnum::ITEM_SUB_TYPE_JEWAL, false);

	int prop_type = MapEquipment::fetch_jewel_prop_type(item_id) - 1;
	int index = 10000 + prop_type * 9 + item_id % 10 - 1;

	ItemObjVec need_vec;
	const Json::Value& conf = CONFIG_INSTANCE->equip_compse(index);

	GameCommon::make_up_conf_items(need_vec, conf["materials"]);
	JUDGE_RETURN(need_vec.empty() == false, false);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	return this->pack_check(package, need_vec) == 0;
}

void MapEquipment::check_pa_event_equip_strengthen()
{
	int flag = this->equip_strengthen_is_operate();
	this->map_logic_player()->update_player_assist_single_event(
			GameEnum::PA_EVENT_EQUIP_STRENGTHEN, flag);
}

void MapEquipment::check_pa_event_equip_red_uprising()
{
    int flag = this->equip_red_uprising_is_operate();
    this->map_logic_player()->update_player_assist_single_event(
    		GameEnum::PA_EVENT_EQUIP_RED_UPRISING, flag);
}

void MapEquipment::check_pa_event_equip_good_refine(int item_id)
{
	int flag = this->equip_good_refine_is_operate(item_id % 10 - 1);
    this->map_logic_player()->update_player_assist_single_event(
    		GameEnum::PA_EVENT_EQUIP_GOOD_REFINE, flag);
}

void MapEquipment::check_pa_event_jewel_all(int check)
{
	GamePackage* pack = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(pack != NULL, ;);

	bool check_first = check & 0x01;
	bool check_second = check & 0x02;

	int first_flag = check_first == true ? false : true;
	int second_flag = check_second == true ? false : true;

	ItemListMap& item_map = pack->item_map();
	for (ItemListMap::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
	{
		PackageItem* item = iter->second;
		JUDGE_CONTINUE(item->conf()["sub_type"].asInt() == GameEnum::ITEM_SUB_TYPE_JEWAL);

		if (check_first == true && first_flag == false)
		{
			first_flag = this->equip_insert_jewel_is_operate(item->__id);
		}

		if (check_second == true && second_flag == false)
		{
			second_flag = this->equip_jewel_combine_is_operate(item->__id);
		}

		if (first_flag == true && second_flag == true)
		{
			break;
		}
	}

	if (check_first == true)
	{
		this->map_logic_player()->update_player_assist_single_event(
				GameEnum::PA_EVENT_EQUIP_INSERT_JEWEL, first_flag);
	}

	if (check_second == true)
	{
		this->map_logic_player()->update_player_assist_single_event(
				GameEnum::PA_EVENT_ITEM_MERGE, second_flag);
	}
}

void MapEquipment::check_pa_event_equip_insert_jewel(int item_id)
{
	int flag = this->equip_insert_jewel_is_operate(item_id);
	this->map_logic_player()->update_player_assist_single_event(
			GameEnum::PA_EVENT_EQUIP_INSERT_JEWEL, flag);
}

void MapEquipment::check_pa_event_jewel_combine(int item_id)
{
	int flag = this->equip_jewel_combine_is_operate(item_id);
	this->map_logic_player()->update_player_assist_single_event(
			GameEnum::PA_EVENT_ITEM_MERGE, flag);
}

void MapEquipment::refresh_equip_prop_to_map()
{
	this->calc_equip_property();
	this->sync_equip_prop_to_map();
}

void MapEquipment::sync_equip_prop_to_map(int enter_type)
{
	IntMap prop_map;
	this->calc_equip_property(prop_map);
	this->refresh_fight_property(BasicElement::EQUIP, prop_map, enter_type);
	this->refresh_smelt_prop(enter_type);
	this->refresh_jewel_sublime_prop(enter_type);
}

int MapEquipment::equip_molding_spirit(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400663*, request, RETURN_MOLDING_SPIRIT);

	//等级不足
	MapLogicPlayer *player = this->map_logic_player();
	JUDGE_RETURN(player != NULL && player->role_level() >= 70, -1);

	//执行次数
	int times = request->times();
	//装备索引
	int index = request->index();

	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(package != NULL, false);

	//查找装备
	PackageItem* item = package->find_by_index(index);
	JUDGE_RETURN(item != NULL, ERROR_CLIENT_OPERATE);

	//判断是否红装
	int color_index = item->red_grade();
	CONDITION_NOTIFY_RETURN(color_index >= 0, RETURN_MOLDING_SPIRIT,
			ERROR_MOLDING_SPIRIT_UP_RED_EQUIP);

	//判断是否达到条件，升级铸魂总属性
	MoldingSpiritDetail &molding_detail = item->__equipment.__molding_detail;
	molding_detail.__red_grade = color_index + GameEnum::ORANGE;

	Proto51400663 respond;
	respond.set_index(index);

	for(int i = 0; i < times; ++i)
	{
		//随机属性索引
		int rand_index = this->molding_spirit_rand_nature();
		JUDGE_RETURN(rand_index >= 0, ERROR_CONFIG_ERROR);

		const Json::Value &item_config = CONFIG_INSTANCE->const_set_conf("molding_spirit_item")["arr"];

		//获取常量表中固定的物品
		int config_item_id = 0, config_item_amount = 1, config_schedule = 2;
		int item_id = item_config[config_item_id].asInt();
		int need_amount = item_config[config_item_amount].asInt();
		int add_schedule = item_config[config_schedule].asInt();
		int have_amount = this->pack_count(item_id);
		CONDITION_NOTIFY_RETURN(have_amount >= need_amount, RETURN_MOLDING_SPIRIT, ERROR_PACKAGE_GOODS_AMOUNT);
		//增加进度
		int ret = molding_detail.update_nature_schedule(rand_index, add_schedule);
		if(ret == 0)
			this->record_equip_item(item, EQUIP_MOLDING_SPIRIT);
		//铸魂失败也消耗物品
		this->pack_remove(SerialObj(ITEM_GOOD_MOLDING_SPIRIT, index), item_id, need_amount);

		respond.set_nature(rand_index);
		respond.set_state(!ret);

		int all_nature_value = molding_detail.fetch_nature_level(GameEnum::MOLDING_SPIRIT_ALL_NATURE);
		if(molding_detail.check_up_all_nature() == 0)
		{
			const Json::Value &up_level_cond = CONFIG_INSTANCE->molding_spirit_all_nature(
					all_nature_value, color_index + GameEnum::ORANGE);
			CONDITION_NOTIFY_RETURN(up_level_cond != Json::Value::null, RETURN_MOLDING_SPIRIT, ERROR_MOLDING_SPIRIT_MAX_LEVEL);
			int level_cond = up_level_cond["red_grade"].asInt();
			CONDITION_NOTIFY_RETURN(color_index + GameEnum::ORANGE >= level_cond && level_cond > 0, RETURN_MOLDING_SPIRIT,
					ERROR_MOLDING_SPIRIT_UP_RED_EQUIP);
			for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_END; ++i)
			{
				molding_detail.update_nature_level(i, 1);
				this->record_equip_item(item, EQUIP_MOLDING_SPIRIT);
			}
			respond.set_nature(GameEnum::MOLDING_SPIRIT_ALL_NATURE);
			respond.set_state(true);
		}
	}

	package->notify_item_info(index);
	this->refresh_equip_prop_to_map();
	this->notify_pack_info(GameEnum::INDEX_EQUIP);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);

	//重新刷新面板
	this->fetch_molding_spirit_info(index);
	return this->respond_to_client(RETURN_MOLDING_SPIRIT, &respond);
}


int MapEquipment::molding_spirit_rand_nature()
{
	//获取常量表的权重
	const Json::Value &weight_value = CONFIG_INSTANCE->const_set_conf("molding_spirit_weight")["arr"];
	if(weight_value.isArray())
	{
		IntVec total_weight;
		int base_weight = 0;
		for(uint i = 0; i < weight_value.size(); ++i)
		{
			base_weight += weight_value[i].asInt();
			total_weight.push_back(base_weight);
		}

		int rand_num = rand() % base_weight;
		for(uint i = 0; i < total_weight.size(); ++i)
		{
			if(rand_num < total_weight[i])
				return i;
		}
	}
	return -1;
}


int MapEquipment::fetch_molding_spirit_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400664*, request, RETURN_MOLDING_SPIRIT_INFO);
	int index = request->index();
	return this->fetch_molding_spirit_info(index);
}

int MapEquipment::fetch_molding_spirit_info(int index)
{
	//等级不足
	MapLogicPlayer *player = this->map_logic_player();
	JUDGE_RETURN(player != NULL && player->role_level() >= 70, 0);

	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(package != NULL, false);

	//查找装备
	PackageItem* item = package->find_by_index(index);
	CONDITION_NOTIFY_RETURN(item != NULL, RETURN_MOLDING_SPIRIT,
			ERROR_CLIENT_OPERATE);

	//判断是否红装
	int color_index = item->red_grade();
	CONDITION_NOTIFY_RETURN(color_index > 0, RETURN_MOLDING_SPIRIT,
			ERROR_MOLDING_SPIRIT_UP_RED_EQUIP);

	Proto51400664 respond;
	respond.set_index(index);

	MoldingSpiritDetail &molding_detail = item->__equipment.__molding_detail;
	molding_detail.__red_grade = color_index + GameEnum::ORANGE;

	string str_nature = "";
	const string str_cond = "cond";

	for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_END; ++i)
	{
		MoldingNature *nature = respond.add_nature_set();
		int cur_nature_level = molding_detail.fetch_nature_level(i);
		int cur_nature_schedule = molding_detail.fetch_nature_schedule(i);
		//这里红装阶数只有铸魂总属性才有用
		const Json::Value &value = CONFIG_INSTANCE->molding_spirit_info(i, cur_nature_level, color_index + GameEnum::ORANGE);
		nature->set_level(cur_nature_level);

		if(i == GameEnum::MOLDING_SPIRIT_ALL_NATURE)
		{
			int nature_index = CONFIG_INSTANCE->molding_spirit_equip_nature(item->__index + 1)["nature"].asInt();
			str_nature = CONFIG_INSTANCE->molding_spirit_return_sys_nature_name(nature_index);
			nature->set_value(value[str_nature].asInt());
		}
		else
		{
			str_nature = "nature";
			//当进度满的话，就读取full_nature属性
			if(cur_nature_schedule == value[str_cond].asInt())
				nature->set_value(value["full_nature"].asInt());
			else
				nature->set_value(value[str_nature].asInt());
		}

		nature->set_max_schedule(value[str_cond].asInt());
		nature->set_cur_schedule(cur_nature_schedule);
		int nature_id = CONFIG_INSTANCE->molding_spirit_return_sys_nature_id(i);
		JUDGE_CONTINUE(nature_id > 0);
		nature->set_nature_id(nature_id);
		MSG_DEBUG("index:%d, nature_id:%d level:%d, value:%d, cur_sch:%d, max_sch:%d",
				i, nature_id, cur_nature_level, value[str_nature].asInt(),
				cur_nature_schedule, value[str_cond].asInt());
	}

	FINER_PROCESS_RETURN(RETURN_MOLDING_SPIRIT_INFO, &respond);
}

int MapEquipment::test_command_clean_molding_nature()
{
	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(package != NULL, 0);

	for (int i = 0; i < package->size(); ++i)
	{
		//查找装备
		PackageItem* item = package->find_by_index(i);
		JUDGE_CONTINUE(item != NULL);

		MoldingSpiritDetail &molding_detail = item->__equipment.__molding_detail;
		molding_detail.reset();
		for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_END; ++i)
		{
			molding_detail.fetch_nature_level(i);
		}
	}

	this->refresh_equip_prop_to_map();
	this->notify_pack_info(GameEnum::INDEX_EQUIP);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_PACKAGE);
	return 0;
}

int MapEquipment::fetch_molding_spirit_all_equip_info()
{
	GamePackage* package = this->find_package(GameEnum::INDEX_EQUIP);
	JUDGE_RETURN(package != NULL, 0);
	Proto51400665 respond;

	for (int i = 0; i < package->size(); ++i)
	{
		//查找装备
		PackageItem* item = package->find_by_index(i);
		JUDGE_CONTINUE(item != NULL);
		ProtoItem *proto_item = respond.add_equip_info();
		proto_item->set_index(i);
		proto_item->set_id(item->__id);
		ProtoEquip *proto_equip = proto_item->mutable_equipment();

		MoldingSpiritDetail &molding_detail = item->__equipment.__molding_detail;
		for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_END; ++i)
		{
			MoldingNature *proto_molding = proto_equip->add_molding_nature();
			int cur_level = molding_detail.fetch_nature_level(i);
			const Json::Value &config_molding = CONFIG_INSTANCE->molding_spirit_info(i, cur_level);
			proto_molding->set_level(cur_level);
			proto_molding->set_cur_schedule(molding_detail.fetch_nature_schedule(i));
			if(Json::Value::null != config_molding)
				proto_molding->set_max_schedule(config_molding["cond"].asInt());
			else
				proto_molding->set_max_schedule(0);
		}
	}

	FINER_PROCESS_RETURN(RETURN_MOLDING_SPIRIT_ALL_EQUIP_INFO, &respond);
}
