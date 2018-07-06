/*
 * MLFashion.cpp
 *
 *  Created on: 2017年1月20日
 *      Author: lyw
 */

#include "MLFashion.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "MapLogicPlayer.h"

MLFashion::MLFashion() {
	// TODO Auto-generated constructor stub

}

MLFashion::~MLFashion() {
	// TODO Auto-generated destructor stub
}

void MLFashion::reset(void)
{
	this->fashion_detail_.reset();
}

void MLFashion::check_level_open_fashion()
{
	static std::string fun_str = "fun_fashion";

	RoleFashion& fashion_detail = this->fashion_detail();
	JUDGE_RETURN(fashion_detail.open_ != true, ;);

	int cur_level = this->role_level();
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_level(
			fun_str, cur_level) == true, ;);

	fashion_detail.open_ = true;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	this->request_fetch_fashion_info();
}

void MLFashion::check_task_open_fashion(int task_id)
{
	static std::string fun_str = "fun_fashion";

	RoleFashion& fashion_detail = this->fashion_detail();
	JUDGE_RETURN(fashion_detail.open_ == false, ;);
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_task(fun_str, task_id) == true, ;);

	fashion_detail.open_ = true;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	this->request_fetch_fashion_info();
}

void MLFashion::active_up_fashion_level()
{
	RoleFashion &fashion_detail = this->fashion_detail();

	const Json::Value& fashion_level_json = fashion_detail.fashion_level();
	JUDGE_RETURN(fashion_level_json.empty() == false, ;);

	int need_exp = fashion_level_json["exp"].asInt();
	JUDGE_RETURN(need_exp > 0, ;);
	JUDGE_RETURN(fashion_detail.exp_ >= need_exp, ;);

	fashion_detail.level_ += 1;
	fashion_detail.exp_ -= need_exp;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	this->active_up_fashion_level();

	//刷新属性
//	this->refresh_fashion_attr_add();

//	Proto51402603 respond;
//	respond.set_level(fashion_detail.level_);
//	respond.set_exp(fashion_detail.exp_);
//	FINER_PROCESS_RETURN(RETURN_UP_FASHION_LEVEL, &respond);
}

int MLFashion::cal_fashion_fight_attr(int fashion_id)
{
	RoleFashion &fashion_detail = this->fashion_detail();
	RoleFashion::FashionInfo* fashion_info = fashion_detail.fetch_fashion_info(fashion_id);
	JUDGE_RETURN(fashion_info != NULL, 0);
	JUDGE_RETURN(fashion_info->active_type_ == true, 0);

	FightProperty& fight_prop = fashion_info->fight_prop_;
	FightProperty& color_num_prop = fashion_info->color_num_prop_;
	fight_prop.reset();
	color_num_prop.reset();

	//基础属性
	const Json::Value &fashion_json = fashion_info->fashion_json();
	fight_prop.make_up_name_prop(fashion_json);

//	JUDGE_RETURN(fashion_info->active_tick_ == -1, 0);

	//染色属性
	const Json::Value color_attr = fashion_json["color_attr"];
	uint color_num = fashion_info->color_set_.size();
	for (uint i = 0; i < color_num; ++i)
	{
		fight_prop.make_up_one_prop(color_attr[i][0u].asString(), color_attr[i][1u].asInt());
	}

	//时装等级加成（百分比）
	const Json::Value& fashion_level_json = fashion_detail.fashion_level();
	if (fashion_level_json != Json::Value::null)
	{
		fight_prop.add_multi_times(fashion_level_json["attr"].asInt());
	}
	fight_prop.caculate_force();

	//染色数量奖励属性
	const Json::Value& num_attr = fashion_json["num_attr"];
	for (uint i = 0; i < num_attr.size(); ++i)
	{
		JUDGE_CONTINUE(int(color_num) >= num_attr[i][0u].asInt());

		color_num_prop.attack_ 	  = num_attr[i][1u].asInt();
		color_num_prop.defence_   = num_attr[i][2u].asInt();
		color_num_prop.blood_max_ = num_attr[i][3u].asInt();
	}

	return 0;
}

int MLFashion::cal_total_fashion_attr()
{
	RoleFashion &fashion_detail = this->fashion_detail();
	FightProperty& total_fight = fashion_detail.total_prop_;
	total_fight.reset();

	//时装总属性
	for (RoleFashion::FashionInfoMap::iterator iter = fashion_detail.fashion_map_.begin();
			iter != fashion_detail.fashion_map_.end(); ++iter)
	{
		this->cal_fashion_fight_attr(iter->first);

		RoleFashion::FashionInfo& fashion_info = iter->second;
		total_fight.add_fight_property(fashion_info.fight_prop_);
		total_fight.add_fight_property(fashion_info.color_num_prop_);
	}

	//时装数量加成属性
	const Json::Value& fashion_num_json = fashion_detail.fashion_num_conf();
	if (fashion_num_json != Json::Value::null)
	{
		total_fight.make_up_name_prop(fashion_num_json);
	}

	return 0;
}

int MLFashion::refresh_fashion_attr_add(const int enter_type)
{
	RoleFashion &fashion_detail = this->fashion_detail();
	JUDGE_RETURN(fashion_detail.open_ == true, 0);

	this->cal_total_fashion_attr();

	IntMap prop_map;
	fashion_detail.total_prop_.serialize(prop_map);
	this->refresh_fight_property(BasicElement::FASHION, prop_map, enter_type);

	return 0;
}

void MLFashion::serialize_fashion_info(ProtoFashionDetail* info, RoleFashion::FashionInfo *fashion_info)
{
	JUDGE_RETURN(fashion_info != NULL, ;);

	info->set_fashion_id(fashion_info->fashion_id_);
	info->set_color_id(fashion_info->color_id_);
	info->set_is_permanent(fashion_info->is_permanent_);
	info->set_active_type(fashion_info->active_type_);
	info->set_active_tick(fashion_info->active_tick_);
	info->set_end_tick(fashion_info->end_tick_);

	for (IntVec::iterator it = fashion_info->color_set_.begin();
			it != fashion_info->color_set_.end(); ++it)
	{
		info->add_color_set(*it);
	}
}

void MLFashion::notify_fashion_style(int notify)
{
	RoleFashion &fashion_detail = this->fashion_detail();

	Proto30400410 inner;
	inner.set_level(fashion_detail.level_);
	inner.set_exp(fashion_detail.exp_);
	inner.set_open(fashion_detail.open_);
	inner.set_select_id(fashion_detail.select_id_);
	inner.set_sel_color_id(fashion_detail.sel_color_id_);
	inner.set_notify_flag(notify);
	this->send_to_map_thread(inner);

	JUDGE_RETURN(fashion_detail.open_ == true, ;);
	MAP_MONITOR->dispatch_to_logic(this, &inner);
}

int MLFashion::request_fetch_fashion_info()
{
	RoleFashion &fashion_detail = this->fashion_detail();
	JUDGE_RETURN(fashion_detail.open_ == true, 0);

	Proto51402601 respond;
	respond.set_level(fashion_detail.level_);
	respond.set_exp(fashion_detail.exp_);
	respond.set_fashion_id(fashion_detail.select_id_);
	respond.set_color_id(fashion_detail.sel_color_id_);
	for (RoleFashion::FashionInfoMap::iterator iter = fashion_detail.fashion_map_.begin();
			iter != fashion_detail.fashion_map_.end(); ++iter)
	{
		ProtoFashionDetail *info = respond.add_fashion_info();
		RoleFashion::FashionInfo &fashion_info = iter->second;
		this->serialize_fashion_info(info, &fashion_info);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_FASHION_INFO, &respond);
}

int MLFashion::request_fashion_add_color(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto11402602 *, request, msg, -1);

	int fashion_id = request->fashion_id();
	RoleFashion &fashion_detail = this->fashion_detail();
	RoleFashion::FashionInfo* fashion_info = fashion_detail.fetch_fashion_info(fashion_id);
	CONDITION_NOTIFY_RETURN(fashion_info != NULL, RETURN_FASHION_ADD_COLOR, ERROR_FASHION_NOT_EXIT);

	const Json::Value &fashion_json = fashion_info->fashion_json();
	CONDITION_NOTIFY_RETURN(fashion_json != Json::Value::null,
			RETURN_FASHION_ADD_COLOR, ERROR_CONFIG_NOT_EXIST);
	CONDITION_NOTIFY_RETURN(fashion_json["time"] == -1, RETURN_FASHION_ADD_COLOR, ERROR_TIME_FASHION_COLOR);

	int color_id = request->color_id();
	CONDITION_NOTIFY_RETURN(fashion_info->check_color_set_has_color(color_id) == false
			&& fashion_info->check_json_has_color(color_id) == true,
			RETURN_FASHION_ADD_COLOR, ERROR_FASHION_HAS_COLOR);

	int active_buy = request->active_buy();

	int item_id = CONFIG_INSTANCE->fashion_const("item_id");
	int amount = CONFIG_INSTANCE->fashion_const("amount");
	int color_exp = CONFIG_INSTANCE->fashion_const("color_exp");
	CONDITION_NOTIFY_RETURN(item_id > 0 && amount > 0 && color_exp > 0,
			RETURN_FASHION_ADD_COLOR, ERROR_CONFIG_NOT_EXIST);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_FASHION_ADD_COLOR,
			ERROR_CLIENT_OPERATE);

	int cur_amount = package->count_by_id(item_id);

	UpgradeAmountInfo amount_info(item_id, amount, cur_amount);
	CONDITION_NOTIFY_RETURN(amount_info.init_flag_ == true, RETURN_FASHION_ADD_COLOR,
			ERROR_EQUIP_UPRISE_LEVEL_HIGH);

	if (active_buy == false)
	{
		CONDITION_NOTIFY_RETURN(amount_info.buy_amount_ == 0, RETURN_FASHION_ADD_COLOR,
				ERROR_PACKAGE_GOODS_AMOUNT);
	}

	// buy item
	if (amount_info.buy_amount_ > 0)
	{
		Money need_money;

		int ret = amount_info.total_money(need_money, this->own_money());
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FASHION_ADD_COLOR,
				ERROR_PACKAGE_GOLD_AMOUNT);

		SerialObj money_serial(SUB_MONEY_BUY_FASHION_COLOR, amount_info.buy_item_,
				amount_info.buy_amount_);

		ret = this->pack_money_sub(need_money, money_serial);
		CONDITION_NOTIFY_RETURN(ret == 0, SUB_MONEY_BUY_FASHION_COLOR, ret);
	}

	if (amount_info.total_use_amount_ > 0)
	{
		SerialObj item_serail(ITEM_FASHION_ADD_COLOR_COST, fashion_id);
		this->pack_remove(item_serail, item_id, amount_info.total_use_amount_);
	}

	fashion_info->color_set_.push_back(color_id);
	fashion_info->color_id_ = color_id;
	fashion_detail.exp_ += color_exp;
	if (fashion_detail.select_id_ == fashion_id)
		fashion_detail.sel_color_id_ = color_id;

	//自动升级
	this->active_up_fashion_level();
	//刷新属性
	this->refresh_fashion_attr_add();

	this->request_fetch_fashion_info();

	if (fashion_detail.select_id_ == fashion_id)
	{
		fashion_detail.sel_color_id_ = color_id;
		this->notify_fashion_style(true);
	}
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	FINER_PROCESS_NOTIFY(RETURN_FASHION_ADD_COLOR);
}

int MLFashion::request_change_fashion_id(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto11402605 *, request, msg, -1);

	int fashion_id = request->fashion_id();
	RoleFashion &fashion_detail = this->fashion_detail();
	RoleFashion::FashionInfo* fashion_info = fashion_detail.fetch_fashion_info(fashion_id);
	CONDITION_NOTIFY_RETURN(fashion_info != NULL, RETURN_CHANGE_FASHION_ID,
			ERROR_FASHION_NOT_EXIT);
	CONDITION_NOTIFY_RETURN(fashion_id != fashion_detail.select_id_,
			RETURN_CHANGE_FASHION_ID, ERROR_SELECT_SAME_FASHION);

	fashion_detail.select_id_ = fashion_id;
	fashion_detail.sel_color_id_ = fashion_info->color_id_;
	this->notify_fashion_style(true);

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	Proto51402605 respond;
	respond.set_fashion_id(fashion_id);
	FINER_PROCESS_RETURN(RETURN_UP_FASHION_LEVEL, &respond);
}

int MLFashion::request_change_fashion_color(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto11402606 *, request, msg, -1);
	int fashion_id = request->fashion_id();
	int color_id = request->sel_color_id();
	RoleFashion &fashion_detail = this->fashion_detail();
	RoleFashion::FashionInfo* fashion_info = fashion_detail.fetch_fashion_info(fashion_id);
	CONDITION_NOTIFY_RETURN(fashion_info != NULL, RETURN_CHANGE_FASHION_STYLE,
			ERROR_FASHION_NOT_EXIT);
	CONDITION_NOTIFY_RETURN(fashion_info->is_permanent_ == true,
			RETURN_CHANGE_FASHION_STYLE, ERROR_TIME_FASHION_COLOR);
	CONDITION_NOTIFY_RETURN(fashion_info->color_id_ != color_id,
			RETURN_CHANGE_FASHION_STYLE, ERROR_SELECT_SAME_COLOR);

	if (fashion_id == fashion_detail.select_id_)
	{
		fashion_detail.sel_color_id_ = color_id;
		fashion_info->color_id_ = color_id;
		this->notify_fashion_style(true);
	}
	else
	{
		fashion_info->color_id_ = color_id;
	}

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	Proto51402606 respond;
	respond.set_fashion_id(fashion_id);
	respond.set_sel_color_id(color_id);
	FINER_PROCESS_RETURN(RETURN_UP_FASHION_LEVEL, &respond);
}

int MLFashion::request_change_fashion_style(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto11402606 *, request, msg, -1);

	RoleFashion &fashion_detail = this->fashion_detail();
	int fashion_id = request->fashion_id();
	int color_id = request->sel_color_id();
	int send_flag = false;
	if (fashion_id == 0)
	{
		fashion_detail.select_id_ = 0;
		fashion_detail.sel_color_id_ = 0;

		send_flag = true;
	}
	else
	{
		RoleFashion::FashionInfo* fashion_info = fashion_detail.fetch_fashion_info(fashion_id);
		CONDITION_NOTIFY_RETURN(fashion_info != NULL, RETURN_CHANGE_FASHION_STYLE,
					ERROR_FASHION_NOT_EXIT);
		CONDITION_NOTIFY_RETURN(color_id == 0 || (color_id != 0 && fashion_info->check_color_set_has_color(color_id) == true),
				RETURN_CHANGE_FASHION_STYLE, ERROR_FASHION_NOT_HAS_COLOR);

		if (fashion_detail.select_id_ != fashion_id)
			send_flag = true;
		else if (fashion_detail.sel_color_id_ != color_id)
			send_flag = true;

		fashion_detail.select_id_ = fashion_id;
		fashion_detail.sel_color_id_ = color_id;
		fashion_info->color_id_ = color_id;
	}

	if (send_flag)
		this->notify_fashion_style(true);

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	Proto51402606 respond;
	respond.set_fashion_id(fashion_id);
	respond.set_sel_color_id(color_id);
	FINER_PROCESS_RETURN(RETURN_UP_FASHION_LEVEL, &respond);
}

int MLFashion::ckeck_can_add_fashion(const Json::Value &conf)
{
	int ckeck_id = conf["ckeck_id"].asInt();
	JUDGE_RETURN(ckeck_id > 0, true);

	RoleFashion &fashion_detail = this->fashion_detail();
	RoleFashion::FashionInfo* check_info = fashion_detail.fetch_fashion_info(ckeck_id);
	JUDGE_RETURN(check_info != NULL, true);

	int send_reward = conf["send_reward"].asInt();
	SerialObj obj(ADD_FROM_WEDDING_FASHION);
	this->add_reward(send_reward, obj);

	return false;
}

int MLFashion::send_change_mail(const Json::Value &conf)
{
	JUDGE_RETURN(conf != Json::Value::null, 0);

	int active_exp = CONFIG_INSTANCE->fashion_const("active_exp");
	int mail_id = CONFIG_INSTANCE->fashion_const("change_mail");
	JUDGE_RETURN(mail_id > 0, 0);

	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str(), conf["name"].asCString(), active_exp);
	GameCommon::request_save_mail_content(this->role_id(), mail_info);

	return 0;
}

int MLFashion::add_fashion(int fashion_id)
{
	const Json::Value &fashion_json = CONFIG_INSTANCE->fashion(fashion_id);
	JUDGE_RETURN(fashion_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

	int ret = this->ckeck_can_add_fashion(fashion_json);
	JUDGE_RETURN(ret == true, 0);

	int active_time = fashion_json["time"].asInt();
	int active_exp = CONFIG_INSTANCE->fashion_const("active_exp");
	RoleFashion &fashion_detail = this->fashion_detail();
	RoleFashion::FashionInfo* fashion_info = fashion_detail.fetch_fashion_info(fashion_id);
	if (fashion_info != NULL)
	{
		if (fashion_info->is_permanent_ == false)
		{
			JUDGE_RETURN(active_time > 0, ERROR_CONFIG_NOT_EXIST);
			fashion_info->end_tick_ += active_time;
		}
		else
		{
			fashion_detail.exp_ += active_exp;
			this->active_up_fashion_level();
			this->send_change_mail(fashion_json);
		}
	}
	else
	{
		RoleFashion::FashionInfo &fashion_info = fashion_detail.fashion_map_[fashion_id];
		fashion_info.fashion_id_ = fashion_id;
		fashion_info.active_type_ = true;
		if (active_time > 0)
		{
			fashion_info.is_permanent_ = false;
			fashion_info.active_tick_ = ::time(NULL);
			fashion_info.end_tick_ = ::time(NULL) + active_time;
		}
		else
		{
			fashion_info.is_permanent_ = true;
			fashion_info.active_tick_ = -1;
			fashion_detail.exp_ += active_exp;
		}

		fashion_detail.select_id_ = fashion_id;
		fashion_detail.sel_color_id_ = fashion_info.color_id_;

		//自动升级
		this->active_up_fashion_level();
		//刷新属性
		this->refresh_fashion_attr_add();
		//刷新外观
		this->notify_fashion_style(true);

		Proto81402101 respond;
		respond.set_fashion_id(fashion_id);
		this->respond_to_client(NOTIFY_NEW_FASHION_GET, &respond);
	}

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	this->request_fetch_fashion_info();

	return 0;
}

int MLFashion::sync_transfer_fashion(int scene_id)
{
	RoleFashion &fashion_detail = this->fashion_detail();

	Proto31400146 proto_fashion;
	proto_fashion.set_level(fashion_detail.level_);
	proto_fashion.set_exp(fashion_detail.exp_);
	proto_fashion.set_open(fashion_detail.open_);
	proto_fashion.set_select_id(fashion_detail.select_id_);
	proto_fashion.set_select_color_id(fashion_detail.sel_color_id_);

	for (IntVec::iterator iter = fashion_detail.send_set_.begin();
			iter != fashion_detail.send_set_.end(); ++iter)
	{
		proto_fashion.add_send_set(*iter);
	}

	for (RoleFashion::FashionInfoMap::iterator iter = fashion_detail.fashion_map_.begin();
			iter != fashion_detail.fashion_map_.end(); ++iter)
	{
		ProtoFashionDetail *info = proto_fashion.add_fashion_info();
		RoleFashion::FashionInfo &fashion_info = iter->second;
		this->serialize_fashion_info(info, &fashion_info);
	}

	return this->send_to_other_logic_thread(scene_id, proto_fashion);
}

int MLFashion::read_transfer_fashion(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto31400146 *, request, msg, -1);

	RoleFashion &fashion_detail = this->fashion_detail();
	fashion_detail.level_ = request->level();
	fashion_detail.exp_ = request->exp();
	fashion_detail.open_ = request->open();
	fashion_detail.select_id_ = request->select_id();
	fashion_detail.sel_color_id_ = request->select_color_id();

	for (int i = 0; i < request->send_set_size(); ++i)
	{
		int send_id = request->send_set(i);
		fashion_detail.send_set_.push_back(send_id);
	}

	for (int i = 0; i < request->fashion_info_size(); ++i)
	{
		const ProtoFashionDetail &info = request->fashion_info(i);
		RoleFashion::FashionInfo &fashion_info = fashion_detail.fashion_map_[info.fashion_id()];
		fashion_info.fashion_id_ = info.fashion_id();
		fashion_info.color_id_ = info.color_id();
		fashion_info.active_type_ = info.active_type();
		fashion_info.is_permanent_ = info.is_permanent();
		fashion_info.active_tick_ = info.active_tick();
		fashion_info.end_tick_ = info.end_tick();

		for (int j = 0; j < info.color_set_size(); ++j)
		{
			fashion_info.color_set_.push_back(info.color_set(j));
		}
	}
	this->cal_total_fashion_attr();

	return 0;
}

void MLFashion::check_handle_fashion_timeout()
{
	IntVec del_set;
	int mail_id = CONFIG_INSTANCE->fashion_const("lost_fashion_mail");
	RoleFashion &fashion_detail = this->fashion_detail();
	for (RoleFashion::FashionInfoMap::iterator iter = fashion_detail.fashion_map_.begin();
			iter != fashion_detail.fashion_map_.end(); ++iter)
	{
		RoleFashion::FashionInfo &fashion_info = iter->second;
		JUDGE_CONTINUE(fashion_info.is_permanent_ == false);
		JUDGE_CONTINUE(::time(NULL) >= fashion_info.end_tick_);

		del_set.push_back(iter->first);
		if (iter->first == fashion_detail.select_id_)
		{
			fashion_detail.select_id_ = 0;
			fashion_detail.sel_color_id_ = 0;
			this->notify_fashion_style(true);
		}
		JUDGE_CONTINUE(mail_id > 0);

		const Json::Value &fashion_json = fashion_info.fashion_json();
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), fashion_json["name"].asCString(),
				fashion_info.fight_prop_.force_);
		GameCommon::request_save_mail_content(this->role_id(), mail_info);
	}
	JUDGE_RETURN(del_set.size() > 0, ;);

	for (IntVec::iterator iter = del_set.begin(); iter != del_set.end(); ++iter)
	{
		fashion_detail.fashion_map_.erase(*iter);
	}

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);

	this->refresh_fashion_attr_add();
	this->request_fetch_fashion_info();
}

void MLFashion::fashion_login_operate()
{
	this->fashion_handle_player_levelup();
	this->login_check_send_fashion();
}

void MLFashion::fashion_handle_player_levelup()
{
	this->check_level_open_fashion();
}

void MLFashion::fashion_handle_player_task(int task_id)
{
	this->check_task_open_fashion(task_id);
}

void MLFashion::login_check_send_fashion()
{
	RoleFashion &fashion_detail = this->fashion_detail();
	int mail_id = CONFIG_INSTANCE->fashion_const("send_fashion_mail");
	JUDGE_RETURN(mail_id > 0, ;);

	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str());

	int send_flag = false;
	GameConfig::ConfigMap& conf_map = CONFIG_INSTANCE->fashion_num_add();
	for (GameConfig::ConfigMap::iterator iter = conf_map.begin();
			iter != conf_map.end(); ++iter)
	{
		const Json::Value &conf = *(iter->second);
		int fashion_id = conf["fashion_id"].asInt();
		JUDGE_CONTINUE(fashion_detail.check_in_send_set(fashion_id) == false);

		RoleFashion::FashionInfo* fashion_info = fashion_detail.fetch_fashion_info(fashion_id);
		JUDGE_CONTINUE(fashion_info == NULL);

		int ret = this->send_fashion_by_cond(conf);
		JUDGE_CONTINUE(ret == true);

		fashion_detail.send_set_.push_back(fashion_id);
		int item_id = conf["item_id"].asInt();
		int amount = conf["amount"].asInt();
		int item_bind = conf["item_bind"].asInt();
		mail_info->add_goods(item_id, amount, item_bind);

		send_flag = true;
	}
	JUDGE_RETURN(send_flag == true, ;);

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_FASHION_INFO, true);
	GameCommon::request_save_mail_content(this->role_id(), mail_info);
}

int MLFashion::send_fashion_by_cond(const Json::Value& conf)
{
	int cond_type = conf["cond_type"].asInt();
	int cond_value = conf["cond_value"].asInt();

	if (cond_type == RoleFashion::COND_LEVEL && this->role_level() >= cond_value)
	{
		return true;
	}

	if (cond_type == RoleFashion::COND_ONLINE_DAY
			&& this->role_detail().__login_days >= cond_value)
	{
		return true;
	}

	return false;
}

RoleFashion& MLFashion::fashion_detail()
{
	return this->fashion_detail_;
}

