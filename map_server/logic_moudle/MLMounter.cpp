/*
 * MLMounter.cpp
 *
 *  Created on: Nov 26, 2013
 *      Author: peizhibi
 */

#include "MLMounter.h"
#include "MapMonitor.h"
#include "MapLogicPlayer.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"

MLMounter::MLMounter()
{
	// TODO Auto-generated constructor stub
}

MLMounter::~MLMounter()
{
	// TODO Auto-generated destructor stub
}

void MLMounter::reset()
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		mount_detail.reset(i, 1);

		const Json::Value &cur_conf = mount_detail.conf();
		mount_detail.add_new_skill(cur_conf["add_skill"].asInt());
	}
}

int MLMounter::index_to_mount(int index_type)
{
	return index_type / 10;
}

int MLMounter::mount_to_index(int mount_type)
{
	return mount_type * 10;
}

MountDetail& MLMounter::mount_detail(int type)
{
	int adjust_type = GameCommon::adjust_positive_integer(type,
			GameEnum::FUN_TOTAL_MOUNT_TYPE);
	return this->mount_detail_[adjust_type - 1];
}

MountDetail& MLMounter::mount_detail_by_skill(int fun_id)
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		JUDGE_CONTINUE(mount_detail.skill_fun_ == fun_id);
		return mount_detail;
	}
	return this->mount_detail(1);
}

int MLMounter::fetch_mounter_info(Proto50100031* mounter_info)
{
	JUDGE_RETURN(mounter_info != NULL, -1);

	MountDetail& mount_detail = this->mount_detail();
	mounter_info->set_mount_grade(mount_detail.mount_grade_);
	mounter_info->set_on_mount(mount_detail.on_mount_);
	mounter_info->set_mount_shape(mount_detail.mount_shape_);

	return 0;
}

int MLMounter::fetch_mount_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400401 *, request, -1);

	int adjust_type = GameCommon::adjust_positive_integer(request->type(),
			GameEnum::FUN_TOTAL_MOUNT_TYPE);
	return this->fetch_mount_info(adjust_type);
}

int MLMounter::fetch_mount_info(int mount_type)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);

	Proto51400401 mount_info;
	mount_info.set_type(mount_type);
	mount_info.set_mount_grade(mount_detail.mount_grade_);
	mount_info.set_hidden_flag(mount_detail.hidden_flag());
	mount_info.set_mount_shape(mount_detail.mount_shape_);
	mount_info.set_act_shape(mount_detail.act_shape_);

	mount_info.set_bless(mount_detail.bless_);
	mount_info.set_left_time(mount_detail.left_time());
	mount_info.set_ability(mount_detail.ability_amount_);
	mount_info.set_growth(mount_detail.growth_amount_);
	mount_info.set_force(mount_detail.total_prop_.force_);

	mount_detail.fight_prop_.serialize(mount_info.mutable_prop());
	mount_detail.temp_prop_.serialize(mount_info.mutable_temp());
	for (SkillMap::iterator iter = mount_detail.skill_map_.begin();
			iter != mount_detail.skill_map_.end(); ++iter)
	{
		ProtoSkill* proto_skill = mount_info.add_skill();
		iter->second->serialize(proto_skill);
	}

	Proto31403200 task_info;
	int task_id = 0;
	switch(mount_type)
	{
		case GameEnum::FUN_MOUNT:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_FIRE_RIDING_PROGRESS;
			break;
		}
		case GameEnum::FUN_GOD_SOLIDER:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_TROOPS_PROGRESS;
			break;
		}
		case GameEnum::FUN_MAGIC_EQUIP:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_MAGIC_PROGRESS;
			break;
		}
		case GameEnum::FUN_XIAN_WING:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_WING_PROGRESS;
			break;
		}
		case GameEnum::FUN_LING_BEAST:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_PET_PROGRESS;
			break;
		}
		case GameEnum::FUN_BEAST_EQUIP:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_WEAPON_PROGRESS;
			break;
		}
		case GameEnum::FUN_BEAST_MOUNT:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_NIMBUS_RIDING_PROGRESS;
			break;
		}
		case GameEnum::FUN_BEAST_WING:
		{
			break;
		}
		case GameEnum::FUN_BEAST_MAO:
		{
			break;
		}
		case GameEnum::FUN_TIAN_GANG:
		{
			break;
		}
	}

	if (task_id != 0)
	{
		task_info.set_task_id(task_id);
		const Json::Value& value = CONFIG_INSTANCE->cornucopia_info(task_id);
		int conditon = value["condition"].asInt();
		if(mount_detail.mount_grade_ > conditon)
			task_info.set_task_finish_count(conditon);
		else
			task_info.set_task_finish_count(mount_detail.mount_grade_);
		task_info.set_type(1);
		MAP_MONITOR->dispatch_to_logic(this, &task_info);
	}

	this->respond_to_client(RETURN_FETCH_MOUNT_INFO, &mount_info);
	this->notify_pack_info(mount_detail.equip_index_);

	return 0;
}

//进阶
int MLMounter::mount_evoluate(Message *msg)
{
	JUDGE_RETURN(this->validate_operate_tick() == true, 0);
	MSG_DYNAMIC_CAST_RETURN(Proto11400402 *, request, -1);

	int adjust_type = GameCommon::adjust_positive_integer(
			request->type(), GameEnum::FUN_TOTAL_MOUNT_TYPE);

	MountDetail& mount_detail = this->mount_detail(adjust_type);
	const Json::Value &evolution_conf = mount_detail.conf();

	UpgradeAmountInfo amount_info(this, evolution_conf);
	CONDITION_NOTIFY_RETURN(amount_info.init_flag_ == true, RETURN_MOUNT_EVALUATE,
			ERROR_EQUIP_UPRISE_LEVEL_HIGH);

	if (request->auto_buy() == false)
	{
		CONDITION_NOTIFY_RETURN(amount_info.buy_amount_ == 0, RETURN_MOUNT_EVALUATE,
				ERROR_PACKAGE_GOODS_AMOUNT);
	}

    // buy item
	if (amount_info.buy_amount_ > 0)
	{
		Money need_money;

		int ret = amount_info.total_money(need_money, this->own_money());
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_MOUNT_EVALUATE,
				ERROR_PACKAGE_GOLD_AMOUNT);

		SerialObj money_serial(SUB_MONEY_MOUNT_EVA, amount_info.buy_item_,
				amount_info.buy_amount_);

		ret = this->pack_money_sub(need_money, money_serial);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_MOUNT_EVALUATE, ret);
	}

	// sub item
	int total_size = amount_info.use_amount_.size();
	for (int i = 0; i < total_size; ++i)
    {
		JUDGE_CONTINUE(amount_info.use_amount_[i] > 0);
		SerialObj item_serail(ITEM_MOUNT_EVOLUATE, mount_detail.mount_grade_);
		this->pack_remove(item_serail, amount_info.item_id_[i], amount_info.use_amount_[i]);
    }

	int add_bless = GameCommon::rand_value(evolution_conf["bless"]);
	mount_detail.bless_ += add_bless;

	int eva_flag = false;
	if (GameCommon::validate_range_rand(mount_detail.bless_, evolution_conf["success"]) == true)
	{
		//进阶成功
		eva_flag = true;
		this->add_validate_operate_tick();
		this->mount_upgrade_success(adjust_type, evolution_conf["reward"].asInt());
	}
	else
	{
		//进阶失败
		eva_flag = false;
		mount_detail.fail_times_ += 1;
		mount_detail.set_bless_time();
		this->refresh_notify_mount_info(adjust_type);
		this->record_mount(MOUNT_SER_UPGRADE, adjust_type);
	}

	//红点
	MapLogicPlayer* player = this->map_logic_player();
	player->check_pa_event_mount_evoluate(adjust_type);
	player->cache_tick().update_cache(MapLogicPlayer::CACHE_MOUNT);

	Proto51400402 respond;
	respond.set_eva_flag(eva_flag);
	respond.set_bless(add_bless);
	respond.set_type(adjust_type);
	respond.set_force(mount_detail.total_prop_.force_);
	respond.set_mount_grade(mount_detail.mount_grade_);
	FINER_PROCESS_RETURN(RETURN_MOUNT_EVALUATE, &respond);
}

int MLMounter::takeon_mount(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400403*, request, -1);

	int type = request->type();

	MountDetail& mount_detail = this->mount_detail(type);
	mount_detail.on_mount_ = true;

	this->fetch_mount_info(mount_detail.type_);
	this->notify_mount_shape(true, mount_detail.type_);

	Proto51400403 respond;
	respond.set_type(type);
	FINER_PROCESS_RETURN(RETURN_TAKEON_MOUNT, &respond);
}

int MLMounter::takeoff_mount(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400404*, request, -1);

	int type = request->type();

	MountDetail& mount_detail = this->mount_detail(type);
	mount_detail.on_mount_ = false;

	this->fetch_mount_info(mount_detail.type_);
	this->notify_mount_shape(true, mount_detail.type_);

	Proto51400404 respond;
	respond.set_type(type);
	FINER_PROCESS_RETURN(RETURN_TAKEOFF_MOUNT, &respond);
}

int MLMounter::select_mount_shape(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400405*, request, RETURN_SELECT_MOUNT_SHAPE);
	this->select_mount_shape(request->type(), request->mount_shape());
	this->fetch_mount_info(request->type());

	Proto51400405 respond;
	respond.set_type(request->type());
	FINER_PROCESS_RETURN(RETURN_SELECT_MOUNT_SHAPE, &respond);
}

int MLMounter::select_mount_shape(int type, int mount_shape, int flag)
{
	MountDetail& mount_detail = this->mount_detail(type);
	JUDGE_RETURN(mount_shape != mount_detail.mount_shape_, -1);
	JUDGE_RETURN(mount_shape > 0 && mount_shape <= mount_detail.mount_grade_, -1);

	mount_detail.on_mount_ = true;
	mount_detail.mount_shape_ = mount_shape;

	this->notify_mount_shape(true, type);
	this->send_mount_logic_server(mount_detail, flag);	//逻辑服
	return 0;
}

int MLMounter::use_mount_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400415*, request, RETURN_MOUNT_USE_GOODS);

	int mount_type = GameCommon::adjust_positive_integer(
			request->mount_type(), GameEnum::FUN_TOTAL_MOUNT_TYPE);
	MountDetail& mount_detail = this->mount_detail(mount_type);

	int item_id = 0;
	int max_amount = 0;
	int have_amount = 0;

	const Json::Value& mount_conf = mount_detail.conf();
	switch (request->type())
	{
	default:
	case 0:
	{
		item_id = mount_conf["ability_item"].asInt();
		max_amount = mount_conf["ability"].asInt();
		have_amount = mount_detail.ability_amount_;
		break;
	}
	case 1:
	{
		item_id = mount_conf["growth_item"].asInt();
		max_amount = mount_conf["growth"].asInt();
		have_amount = mount_detail.growth_amount_;
		break;
	}
	}

	CONDITION_NOTIFY_RETURN(have_amount < max_amount, RETURN_MOUNT_USE_GOODS,
			ERROR_MOUNT_GOODS_FULL);

	int add_amount = this->pack_count(item_id);
	CONDITION_NOTIFY_RETURN(add_amount > 0, RETURN_MOUNT_USE_GOODS, ERROR_NO_THIS_ITEM);

	int max_need = max_amount - have_amount;
	int use_amount = std::min<int>(max_need, add_amount);

	SerialObj serial(ITEM_MOUNT_ABILITY_GROWTH, request->type());
	this->pack_remove(serial, item_id, use_amount);

	switch (request->type())
	{
	default:
	case 0:
	{
		mount_detail.ability_amount_ += use_amount;
		this->check_pa_event_mount_ability(mount_type, item_id);
		break;
	}
	case 1:
	{
		mount_detail.growth_amount_ += use_amount;
		this->check_pa_event_mount_growth(mount_type, item_id);
		break;
	}
	}

	this->refresh_notify_mount_info(mount_type);
	FINER_PROCESS_NOTIFY(RETURN_MOUNT_USE_GOODS);
}

int MLMounter::use_beast_wing_act_goods(int item_id, int mount_type)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	JUDGE_RETURN(mount_detail.act_shape_ == 0, -1);

	mount_detail.act_shape_ = item_id;
	this->refresh_notify_mount_info(mount_type);
	this->notify_mount_shape(true, mount_type);
	return 0;
}

int MLMounter::sync_transfer_mounter(int scene_id)
{
	Proto31400109 mounter_info;
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		ProtoMount* proto_mount = mounter_info.add_mount_set();

		proto_mount->set_open(mount_detail.open_);
		proto_mount->set_mount_grade(mount_detail.mount_grade_);
		proto_mount->set_mount_shape(mount_detail.mount_shape_);
		proto_mount->set_on_mount(mount_detail.on_mount_);
		proto_mount->set_bless(mount_detail.bless_);
		proto_mount->set_fail_times(mount_detail.fail_times_);
		proto_mount->set_finish_bless(mount_detail.finish_bless_);
		proto_mount->set_ability(mount_detail.ability_amount_);
		proto_mount->set_growth(mount_detail.growth_amount_);
		proto_mount->set_act_shape(mount_detail.act_shape_);

		for (SkillMap::iterator iter = mount_detail.skill_map_.begin();
				iter != mount_detail.skill_map_.end(); ++iter)
		{
			ProtoPairObj* pair = proto_mount->add_skill();
			iter->second->serialize(pair);
		}
	}

	return this->send_to_other_logic_thread(scene_id, mounter_info);
}

int MLMounter::read_transfer_mounter(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400109*, request, -1);
	for (int i = 0; i < request->mount_set_size(); ++i)
	{
		int type = i + 1;
		const ProtoMount& proto_mount = request->mount_set(i);

		MountDetail& mount_detail = this->mount_detail(type);
		mount_detail.set_grade(proto_mount.mount_grade());
		mount_detail.open_ 			= proto_mount.open();
		mount_detail.on_mount_ 		= proto_mount.on_mount();
		mount_detail.mount_shape_ 	= proto_mount.mount_shape();
		mount_detail.bless_ 		= proto_mount.bless();
		mount_detail.fail_times_ 	= proto_mount.fail_times();
		mount_detail.finish_bless_ 	= proto_mount.finish_bless();
		mount_detail.ability_amount_= proto_mount.ability();
		mount_detail.growth_amount_ = proto_mount.growth();
		mount_detail.act_shape_ 	= proto_mount.act_shape();

		for (int j = 0; j < proto_mount.skill_size(); ++j)
		{
			const ProtoPairObj obj = proto_mount.skill(j);
			mount_detail.add_new_skill(obj.obj_id(), obj.obj_value());
		}
		this->caculate_mount_prop(type);
	}

	return 0;
}

void MLMounter::mount_upgrade_success(int mount_type, int reward_id)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);

	mount_detail.bless_ = 0;
	mount_detail.fail_times_ = 0;
	mount_detail.finish_bless_ = 0;
	mount_detail.set_grade(mount_detail.mount_grade_, 1);

	//更新阶级成就
	MapLogicPlayer* player = this->map_logic_player();
	player->update_achieve_info(mount_detail.ach_grade_, mount_detail.mount_grade_);

	//增加技能
	const Json::Value &cur_conf = mount_detail.conf();
	mount_detail.add_new_skill(cur_conf["add_skill"].asInt());

	//进阶奖励物品
	SerialObj obj(ADD_FROM_MOUNT_GRADE, mount_detail.mount_grade_, mount_type);
	this->add_reward(reward_id, obj);

	this->record_mount(MOUNT_SER_UPGRADE, mount_type);
	this->select_mount_shape(mount_type, mount_detail.mount_grade_, true);
	this->refresh_notify_mount_info(mount_type);
	JUDGE_RETURN(mount_detail.mount_grade_ >= mount_detail.shout_start_, ;);

	//传闻
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, this->role_name());
	GameCommon::push_brocast_para_int(para_vec, mount_detail.mount_grade_);
	this->announce(mount_detail.shout_id_, para_vec);
}

void MLMounter::caculate_mount_open_prop(MountDetail& mount_detail)
{
	JUDGE_RETURN(mount_detail.open_ == true, ;);

	FightProperty double_prop;
	FightProperty& fight_prop = mount_detail.fight_prop_;
	FightProperty& temp_prop = mount_detail.temp_prop_;

	//基础属性
	const Json::Value& mount_prop = mount_detail.conf();
	fight_prop.make_up_name_prop(mount_prop);

	//临时属性
	int fail_times = mount_detail.fail_times_;
	if (fail_times > 0)
	{
		temp_prop.make_up_name_prop(mount_prop["fail_prop"], fail_times);
	}

	//等级属性
	const Json::Value& level_prop = CONFIG_INSTANCE->mount_level(
			mount_detail.type_, this->role_level());
	fight_prop.make_up_name_prop(level_prop);

	//资质丹
	if (mount_detail.ability_amount_ > 0)
	{
		const Json::Value& ability_prop = CONFIG_INSTANCE->prop_unit(
				mount_prop["ability_item"].asInt());
		fight_prop.make_up_name_prop(ability_prop, mount_detail.ability_amount_);
	}

	//成长丹
	if (mount_detail.growth_amount_ > 0)
	{
		int growth = CONFIG_INSTANCE->prop_unit(
				mount_prop["growth_item"].asInt())["growth"].asInt();
		fight_prop.add_multi_times(mount_detail.growth_amount_ * growth);
	}

	//技能属性
	for (SkillMap::iterator iter = mount_detail.skill_map_.begin();
			iter != mount_detail.skill_map_.end(); ++iter)
	{
		FighterSkill* skill = iter->second;
		JUDGE_CONTINUE(skill->is_passive_prop_skill() == true);

		const Json::Value& detail_json = skill->detail();
		fight_prop.make_up_name_prop(detail_json);
		JUDGE_CONTINUE(mount_detail.type_ == GameEnum::FUN_BEAST_MOUNT);

		double_prop.make_up_name_prop(detail_json);
	}

	//装备属性
	GamePackage* package = this->find_package(mount_detail.equip_index_);
	for (ItemListMap::iterator iter = package->item_map().begin();
			iter != package->item_map().end(); ++iter)
	{
		PackageItem* item = iter->second;
		JUDGE_CONTINUE(item != NULL);
		fight_prop.add_fight_property(item->__prop_info);
	}

	mount_detail.total_prop_.add_fight_property(temp_prop);
	mount_detail.total_prop_.add_fight_property(double_prop);
}

void MLMounter::caculate_mount_act_prop(MountDetail& mount_detail)
{
	//活动翅榜
	JUDGE_RETURN(mount_detail.act_shape_ > 0, ;);

	const Json::Value& act_prop = CONFIG_INSTANCE->prop_unit(mount_detail.act_shape_);
	mount_detail.fight_prop_.make_up_name_prop(act_prop);
}


void MLMounter::mount_handle_player_levelup()
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		this->check_level_open_mount(i);
	}
}

void MLMounter::check_level_open_mount(int mount_type)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	if (mount_detail.open_ == false)
	{
		int cur_level = this->role_level();
		JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_level(
				mount_detail.fun_str_, cur_level) == true, ;);

		mount_detail.open_ = true;
		this->select_mount_shape(mount_type, 1);
		this->refresh_notify_mount_info(mount_type, true);
		this->check_pa_event_mount_evoluate(mount_type);
	}
	else
	{
		this->refresh_notify_mount_info(mount_type, true);
	}
}

void MLMounter::check_task_open_mount(int mount_type, int task_id)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	JUDGE_RETURN(mount_detail.open_ == false, ;);

	mount_detail.open_ = true;
	this->select_mount_shape(mount_type, 1);
	this->refresh_notify_mount_info(mount_type);
	this->check_pa_event_mount_evoluate(mount_type);
}

void MLMounter::send_mount_logic_server(MountDetail& mount_detail, int flag)
{
	Proto30100239 inner;
	inner.set_flag(flag);

	ProtoThreeObj* proto = inner.add_mount_info();
	proto->set_id(mount_detail.type_);
	proto->set_value(mount_detail.mount_grade_);
	proto->set_tick(mount_detail.mount_shape_);

	MAP_MONITOR->dispatch_to_logic(this, &inner);
}

void MLMounter::notify_mount_open_activity(int type)
{
	MountDetail& mount_detail = this->mount_detail(type);
	JUDGE_RETURN(mount_detail.open_ == true, ;);

	int first_type = 2; //BackSetActDetail::F_ACT_GROWTH_MOUNT;
	this->sync_open_activity_info(first_type, SubObj(mount_detail.open_activity_,
			mount_detail.mount_grade_, mount_detail.total_prop_.force_));
}

void MLMounter::notify_mount_shape(int notify, int type)
{
	Proto30400406 inner;
	inner.set_type(type);
	inner.set_notify_flag(notify);

	MountDetail& mount_detail = this->mount_detail(type);
	inner.set_open(mount_detail.open_);
	inner.set_on_mount(mount_detail.on_mount_);
	inner.set_act_shape(mount_detail.act_shape_);
	inner.set_mount_grade(mount_detail.mount_grade_);
	inner.set_mount_shape(mount_detail.mount_shape_);

	for (SkillMap::iterator iter = mount_detail.skill_map_.begin();
			iter != mount_detail.skill_map_.end(); ++iter)
	{
		ProtoPairObj* pair = inner.add_skill_set();
		iter->second->serialize(pair);
	}

	this->send_to_map_thread(inner);
}

void MLMounter::refresh_mount_property(int mount_type, const SubObj& obj)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	JUDGE_RETURN(mount_detail.prop_open() == true, ;);

	IntMap mount_map;
	mount_detail.total_prop_.serialize(mount_map);

	int offset = mount_detail.prop_index_;
	this->refresh_fight_property(offset, mount_map, obj);
}

int MLMounter::adjust_mount_open()
{
	int level = this->role_level();
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		mount_detail.check_adjust_open(level);
	}

	return 0;
}

int MLMounter::caculate_all_mount_prop()
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		this->caculate_mount_prop(i);
	}

	return 0;
}

int MLMounter::caculate_mount_prop(int type)
{
	MountDetail& mount_detail = this->mount_detail(type);
	JUDGE_RETURN(mount_detail.prop_open() == true, -1);

	FightProperty& fight_prop = mount_detail.fight_prop_;
	FightProperty& temp_prop = mount_detail.temp_prop_;

	fight_prop.reset();
	temp_prop.reset();

	//属性计算
	mount_detail.total_prop_.reset();
	this->caculate_mount_open_prop(mount_detail);
	this->caculate_mount_act_prop(mount_detail);

	//剑池等级加成
	int total_multi = mount_detail.sword_pool_multi();
	if (total_multi > 0)
	{
		fight_prop.add_multi_times(total_multi);
		mount_detail.total_prop_.add_fight_property(fight_prop);
	}
	else
	{
		mount_detail.total_prop_.add_fight_property(fight_prop);
	}

	//战力
	return mount_detail.total_prop_.caculate_force(mount_detail.skill_force());
}

int MLMounter::check_and_upgrade_mount(int type, const Json::Value& effect)
{
	MountDetail& mount_detail = this->mount_detail(type);
	JUDGE_RETURN(mount_detail.mount_grade_ <= effect["value"].asInt(),
			ERROR_MOUNT_MAX_UPGRADE_CONF);

	const Json::Value &evolution_conf = mount_detail.conf();
	JUDGE_RETURN(evolution_conf.isMember("item") == true,
			ERROR_MOUNT_MAX_UPGRADE_CONF);

	this->mount_upgrade_success(type, evolution_conf["reward"].asInt());
	return 0;
}

int MLMounter::check_mount_skill_achieve(int mount_type)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);

	int skill_flag = mount_detail.is_all_skill_open();
	if (skill_flag == false)
	{
		return this->map_logic_player()->update_achieve_info(
				mount_detail.ach_skill_, 0);
	}

	int min_level = INT_MAX;
	for (SkillMap::iterator iter = mount_detail.skill_map_.begin();
			iter != mount_detail.skill_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(mount_detail.is_one_level_skill(iter->second->__skill_id) == false);
		JUDGE_CONTINUE(iter->second->__level < min_level);
		min_level = iter->second->__level;
	}

	return this->map_logic_player()->update_achieve_info(
			mount_detail.ach_skill_, min_level);
}

int MLMounter::check_pa_event_mount_evoluate(int mount_type)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	JUDGE_RETURN(mount_detail.open_ == true, -1);

	MapLogicPlayer* player = this->map_logic_player();
	JUDGE_RETURN(player->check_finish_task(mount_detail.tips_task_) == true, -1);

	int state = this->validate_tips_mount_evoluate(mount_type);
	return player->update_player_assist_single_event(mount_detail.evoluate_red_, state);
}

int MLMounter::check_pa_event_mount_ability(int mount_type, int item_id)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	const Json::Value& mount_conf = mount_detail.conf();

	int state = 0;
	if (this->pack_count(item_id) > 0)
	{
		state = mount_detail.ability_amount_ < mount_conf["ability"].asInt() ? true : false;
	}

	return this->map_logic_player()->update_player_assist_single_event(
			mount_detail.ability_red_, state);
}

int MLMounter::check_pa_event_mount_growth(int mount_type, int item_id)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	const Json::Value& mount_conf = mount_detail.conf();

	int state = 0;
	if (this->pack_count(item_id) > 0)
	{
		state = mount_detail.growth_amount_ < mount_conf["growth"].asInt() ? true : false;
	}

	return this->map_logic_player()->update_player_assist_single_event(
			mount_detail.growth_red_, state);
}

int MLMounter::check_pa_event_mount_skill(int mount_type)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);

	int state = 0;
	for (SkillMap::iterator iter = mount_detail.skill_map_.begin();
			iter != mount_detail.skill_map_.end(); ++iter)
	{
		FighterSkill* skill = iter->second;
		JUDGE_CONTINUE(skill != NULL);

		int ret = this->skill_upgrade_use_goods(skill->__skill_id, skill->__level + 1, true);
		JUDGE_CONTINUE(ret == 0);

		state = 1;
		break;
	}

	return this->map_logic_player()->update_player_assist_single_event(
			mount_detail.skill_red_, state);
}

int MLMounter::check_pa_event_mount_equip(int mount_type)
{
	JUDGE_RETURN(mount_type > 0, -1);

	GamePackage* pack = this->find_package();
	JUDGE_RETURN(pack != NULL, -1);

	int state = 0;
	for (ItemListMap::iterator iter = pack->item_map().begin();
			iter != pack->item_map().end(); ++iter)
	{
		PackageItem* item = iter->second;
		JUDGE_CONTINUE(item->is_equipment() == true);

		IntPair part = item->equip_part();
		JUDGE_CONTINUE(MLMounter::index_to_mount(part.first) == mount_type);

		state = this->validate_tips_mount_equip(mount_type, item->__id);
		JUDGE_CONTINUE(state == true);

		break;
	}

	MountDetail& mount_detail = this->mount_detail(mount_type);
	return this->map_logic_player()->update_player_assist_single_event(
			mount_detail.equip_red_, state);
}

int MLMounter::check_pa_event_mount_equip(int mount_type, int item_id)
{
	int state = this->validate_tips_mount_equip(mount_type, item_id);

	MountDetail& mount_detail = this->mount_detail(mount_type);
	return this->map_logic_player()->update_player_assist_single_event(
			mount_detail.equip_red_, state);
}

int MLMounter::validate_tips_mount_evoluate(int mount_type)
{
	MountDetail& mount_detail = this->mount_detail(mount_type);
	JUDGE_RETURN(mount_detail.mount_grade_ < mount_detail.no_tips_, false);

	UpgradeAmountInfo amount_info(this, mount_detail.conf());
	JUDGE_RETURN(amount_info.init_flag_ == true, false);
	JUDGE_RETURN(amount_info.buy_amount_ == 0, false);

	return true;
}

int MLMounter::validate_tips_mount_equip(int mount_type, int item_id)
{
	const Json::Value &item_json = CONFIG_INSTANCE->item(item_id);
	JUDGE_RETURN(item_json.empty() == false, false);

	MountDetail& mount_detail = this->mount_detail(mount_type);
	JUDGE_RETURN(mount_detail.mount_grade_ >= item_json["use_lvl"].asInt(), false);

	int aim_pack_type = MLMounter::mount_to_index(mount_type);
	GamePackage* aim_package = this->find_package(aim_pack_type);
	JUDGE_RETURN(aim_package != NULL, false);

	int part_index = item_json["part"].asInt() % 100 - 1;
	JUDGE_RETURN(aim_package->validate_pack_index(part_index) == true, false);

	PackageItem* aim_item = aim_package->find_by_index(part_index);
	if (aim_item == NULL)
	{
		//没佩戴
		return true;
	}
	else
	{
		//有更好的
		return item_id > aim_item->__id;
	}
}

void MLMounter::record_mount(int serial, int mount_type)
{
	SERIAL_RECORD->record_mount(this->map_logic_player(),
			this->agent_code(), serial, mount_type);
}

void MLMounter::mount_try_task(int task_id)
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);

		if (mount_detail.try_task_id_ == task_id)
		{
//			this->select_mount_shape(i, 1);
			this->check_task_open_mount(i, task_id);
		}

//		if (CONFIG_INSTANCE->arrive_fun_open_task(
//				mount_detail.fun_str_, task_id) == true)
//		{
//			this->check_task_open_mount(i, task_id);
//		}
	}
}

void MLMounter::update_mount_open_recharge_act(int total_recharge)
{
//	JUDGE_RETURN(CONFIG_INSTANCE->left_open_activity_time() > 0, );

//	static int RECHARGE_MOUNT[] = {GameEnum::FUN_BEAST_MOUNT, GameEnum::FUN_BEAST_MAO};
//	static int TOTAL_SIZE = 2;
//
//	for (int i = 0; i < TOTAL_SIZE; ++i)
//	{
//		MountDetail& mount_detail = this->mount_detail(RECHARGE_MOUNT[i]);
//		JUDGE_CONTINUE(mount_detail.act_shape_ == 0);
//
//		const Json::Value& set_conf = mount_detail.set_conf();
//		JUDGE_CONTINUE(set_conf.isMember("recharge_open") == true);
//
//		int need_gold = set_conf["recharge_open"][0u].asInt();
//		JUDGE_CONTINUE(need_gold > 0 && total_recharge > need_gold);
//
//		int item_id = set_conf["recharge_open"][1u].asInt();
//		this->use_beast_wing_act_goods(item_id, mount_detail.type_);
//	}

}

void MLMounter::check_handle_mount_bless_timeout()
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		JUDGE_CONTINUE(mount_detail.limit_flag_ == true && mount_detail.bless_ > 0);

		int left_time = mount_detail.left_time();
		JUDGE_CONTINUE(left_time <= 0);

		mount_detail.bless_ 		= 0;
		mount_detail.fail_times_	= 0;
		mount_detail.finish_bless_ 	= 0;

		int force = mount_detail.temp_prop_.caculate_force();
		int mail_id = CONFIG_INSTANCE->const_set("mount_mail_id");
		MailInformation* mail_info = GameCommon::create_sys_mail(mail_id);

		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), mount_detail.conf()["name"].asCString(), force);
		GameCommon::request_save_mail_content(this->role_id(), mail_info);

		this->refresh_notify_mount_info(i);
	}
}

void MLMounter::refresh_notify_mount_info(int mount_type, int unnotify)
{
	this->caculate_mount_prop(mount_type);
	this->notify_mount_open_activity(mount_type);
	this->fetch_mount_info(mount_type);
	this->refresh_mount_property(mount_type, SubObj(0, unnotify));
}

int MLMounter::make_up_mount_list(Proto50100156* &respond)
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		JUDGE_CONTINUE(mount_detail.open_ == true);

		ProtoMountInfo *mount_info = respond->add_mount_info();
		mount_info->set_type(i);
		mount_info->set_mount_grade(mount_detail.mount_grade_);
		mount_info->set_mount_shape(mount_detail.mount_shape_);
		mount_info->set_act_shape(mount_detail.act_shape_);
		mount_info->set_force(mount_detail.total_prop_.force_);

		mount_detail.fight_prop_.serialize(mount_info->mutable_prop());
		mount_detail.temp_prop_.serialize(mount_info->mutable_temp());

		for (SkillMap::iterator iter = mount_detail.skill_map_.begin();
				iter != mount_detail.skill_map_.end(); ++iter)
		{
			ProtoSkill* proto_skill = mount_info->add_skill();
			iter->second->serialize(proto_skill);
		}
	}

	return 0;
}

