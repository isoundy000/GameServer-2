/*
 * MLTransfer.cpp
 *
 *  Created on: 2017年3月29日
 *      Author: lyw
 */

#include "MLTransfer.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "MapLogicPlayer.h"

MLTransfer::MLTransfer() {
	// TODO Auto-generated constructor stub

}

MLTransfer::~MLTransfer() {
	// TODO Auto-generated destructor stub
}

void MLTransfer::reset(void)
{
	this->transfer_detail_.reset();
}

void MLTransfer::check_level_open_transfer()
{
	static std::string fun_str = "fun_transfer";

	TransferDetail& transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.open_ != true, ;);

	int cur_level = this->role_level();
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_level(
			fun_str, cur_level) == true, ;);

	this->init_transfer();
}

void MLTransfer::check_task_open_transfer(int task_id)
{
	static std::string fun_str = "fun_transfer";

	TransferDetail& transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.open_ != true, ;);
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_task(fun_str, task_id) == true, ;);

	this->init_transfer();
}

void MLTransfer::init_transfer()
{
	TransferDetail& transfer_detail = this->transfer_detail();
	transfer_detail.open_  = true;
	transfer_detail.level_ = 1;
	transfer_detail.stage_ = 1;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);

	this->refresh_transfer_attr_add();
	this->request_spirit_info();
	this->request_transfer_info();
	this->send_transfer_to_map(TransferDetail::SYNC_TRANSFER, false);
}

void MLTransfer::active_up_spirit_level()
{
	TransferDetail& transfer_detail = this->transfer_detail();
	const Json::Value& spirit_level = transfer_detail.spirit_level_json();
	const Json::Value& next_level = CONFIG_INSTANCE->spirit_level(transfer_detail.level_+1);
	JUDGE_RETURN(spirit_level.empty() == false && next_level.empty() == false, ;);

	int need_exp = spirit_level["upgrade"].asInt();
	JUDGE_RETURN(need_exp > 0, ;);
	JUDGE_RETURN(transfer_detail.exp_ >= need_exp, ;);

	transfer_detail.level_ += 1;
	transfer_detail.exp_ -= need_exp;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);

	//奖励
	SerialObj obj(ADD_FROM_SPIRIT_UP_LVL, transfer_detail.level_, transfer_detail.exp_);
	int reward_id = spirit_level["reward_id"].asInt();
	this->add_reward(reward_id, obj);

	this->active_up_spirit_level();
}

void MLTransfer::create_transfer_in_time(int notify, int send_skill)
{
	TransferDetail &transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.is_in_transfer() == true, ;);

	TransferDetail::TransferInfo *transfer_info = transfer_detail.fetch_transfer_info(
			transfer_detail.active_id_);
	JUDGE_RETURN(transfer_info != NULL, ;);

	transfer_info->create_skill();
	this->send_transfer_to_map(TransferDetail::USE_TRANSFER, notify);

	JUDGE_RETURN(send_skill == true, ;);

	MapLogicPlayer *player = dynamic_cast<MapLogicPlayer *>(this);
	for (IntMap::iterator iter = transfer_info->skill_map_.begin();
			iter != transfer_info->skill_map_.end(); ++iter)
	{
		player->request_sync_map_skill(iter->first, iter->second);
	}
}

int MLTransfer::cal_spirit_advence()
{
	TransferDetail& transfer_detail = this->transfer_detail();

	const Json::Value& spirit_stage = transfer_detail.spirit_stage_json();
	JUDGE_RETURN(spirit_stage.empty() == false, 0);

	transfer_detail.reduce_cool_ = spirit_stage["reduce_cool"].asInt();
	transfer_detail.add_time_ 	 = spirit_stage["add_time"].asInt();

	return 0;
}

int MLTransfer::cal_one_transfer_attr(TransferDetail::TransferInfo &transfer_info)
{
	const Json::Value& detail_conf = transfer_info.detail_conf();
	JUDGE_RETURN(detail_conf.empty() == false, 0);

	FightProperty& one_prop = transfer_info.one_prop_;
	one_prop.reset();
	one_prop.make_up_name_prop(detail_conf);
	one_prop.caculate_force();

	return 0;
}

int MLTransfer::cal_total_transfer_attr()
{
	TransferDetail& transfer_detail = this->transfer_detail();
	FightProperty& total_fight = transfer_detail.total_prop_;
	total_fight.reset();

	//等级属性
	const Json::Value& spirit_level = transfer_detail.spirit_level_json();
	total_fight.make_up_name_prop(spirit_level);

	//等阶属性
	const Json::Value& spirit_stage = transfer_detail.spirit_stage_json();
	if (spirit_stage.empty() == false)
	{
		const Json::Value &attr_json = spirit_stage["attr"];
		for (uint i = 0; i < attr_json.size(); ++i)
		{
			total_fight.make_up_one_prop(attr_json[i][0u].asString(), attr_json[i][1u].asInt());
		}
	}

	//变身属性
	for (TransferDetail::TransferInfoMap::iterator iter = transfer_detail.transfer_map_.begin();
			iter != transfer_detail.transfer_map_.end(); ++iter)
	{
		TransferDetail::TransferInfo &transfer_info = iter->second;
		JUDGE_CONTINUE(transfer_info.is_active_time() == true);

		this->cal_one_transfer_attr(transfer_info);
		total_fight.add_fight_property(transfer_info.one_prop_);
	}

	return 0;
}

int MLTransfer::refresh_transfer_attr_add(const int enter_type)
{
	TransferDetail& transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.open_ == true, 0);

	this->cal_total_transfer_attr();

	IntMap prop_map;
	transfer_detail.total_prop_.serialize(prop_map);
	this->refresh_fight_property(BasicElement::TRANSFER, prop_map, enter_type);

	return 0;
}

int MLTransfer::fetch_make_spirit_mult(const Json::Value& rate_json)
{
	JUDGE_RETURN(rate_json.empty() == false, 1);

	int total_rate = 0;
	for (uint i = 0; i < rate_json.size(); ++i)
	{
		int rate = rate_json[i][1u].asInt();
		total_rate += rate;
	}
	JUDGE_RETURN(total_rate > 0, 1);

	int rand_rate = ::rand() % total_rate;
	int now_rate = 0;
	for (uint i = 0; i < rate_json.size(); ++i)
	{
		int mult = rate_json[i][0u].asInt();
		int rate = rate_json[i][1u].asInt();
		now_rate += rate;
		if (rand_rate <= now_rate)
			return mult;
	}

	return 1;
}

void MLTransfer::handle_transfer_time_out()
{
	IntVec transfer_set;
	TransferDetail &transfer_detail = this->transfer_detail();
	for (TransferDetail::TransferInfoMap::iterator iter = transfer_detail.transfer_map_.begin();
			iter != transfer_detail.transfer_map_.end(); ++iter)
	{
		TransferDetail::TransferInfo &transfer_info = iter->second;
		JUDGE_CONTINUE(transfer_info.is_active_ == true);
		JUDGE_CONTINUE(transfer_info.is_active_time() == false);

		transfer_info.is_active_ = false;
		transfer_set.push_back(iter->first);
	}
	JUDGE_RETURN(transfer_set.size() > 0, ;);

	for (IntVec::iterator iter = transfer_set.begin(); iter != transfer_set.end(); ++iter)
	{
		Proto81400801 active;
		active.set_transfer_id(*iter);
		this->respond_to_client(ACTIVE_TRANSFER_TIME_OUT, &active);
	}

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);
	this->refresh_transfer_attr_add();
	this->request_transfer_info();
}

void MLTransfer::handle_transfer_cd_time_out()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.is_in_transfer() == false, ;);
	JUDGE_RETURN(transfer_detail.active_id_ > 0, ;);

	int send_flag = false;

	TransferDetail::TransferInfo *transfer_info = transfer_detail.fetch_transfer_info(
			transfer_detail.active_id_);
	if (transfer_info == NULL || transfer_info->is_active_time() == false)
	{
		transfer_detail.active_id_ = 0;
		transfer_detail.last_ = 0;

		if (transfer_info != NULL)
			transfer_info->delete_skill();

		send_flag = true;
	}
	else if (transfer_info->skill_info_.size() > 0)
	{
		transfer_info->delete_skill();
		send_flag = true;
	}

	JUDGE_RETURN(send_flag == true, ;);

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);
	this->refresh_transfer_attr_add();
	this->request_transfer_info();
	this->send_transfer_to_map(TransferDetail::CD_TRANSFER, true);
}

void MLTransfer::send_transfer_to_map(int type, int notify)
{
	TransferDetail &transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.open_ == true, ;);

	Proto30400411 inner;
	inner.set_transfer_tick(transfer_detail.transfer_tick_);
	inner.set_last(transfer_detail.last_);
	inner.set_active_id(transfer_detail.active_id_);
	inner.set_reduce_cool(transfer_detail.reduce_cool_);
	inner.set_add_time(transfer_detail.add_time_);
	inner.set_type(type);
	inner.set_notify(notify);

	TransferDetail::TransferInfo *transfer_info = transfer_detail.fetch_transfer_info(
			transfer_detail.active_id_);
	if (transfer_detail.active_id_ > 0 && transfer_info != NULL)
	{
		ProtoTransferInfo *transfer_proto = inner.mutable_transfer_info();
		transfer_info->serialize(transfer_proto);
	}

	this->send_to_map_thread(inner);
}

int MLTransfer::request_spirit_info()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.open_ == true, 0);

	transfer_detail.level_ = transfer_detail.level_ > 0 ? transfer_detail.level_ : 1;
	transfer_detail.stage_ = transfer_detail.stage_ > 0 ? transfer_detail.stage_ : 1;

	Proto51403001 respond;
	respond.set_level(transfer_detail.level_);
	respond.set_exp(transfer_detail.exp_);
	respond.set_stage(transfer_detail.stage_);
	respond.set_gold_times(transfer_detail.gold_times_);
	FINER_PROCESS_RETURN(RETURN_FETCH_SPIRIT_INFO, &respond);
}

int MLTransfer::request_make_spirit(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto11403002 *, request, msg, -1);

	TransferDetail &transfer_detail = this->transfer_detail();
	const Json::Value& spirit_level = transfer_detail.spirit_level_json();
	const Json::Value& transfer_base = transfer_detail.transfer_base_json();
	const Json::Value& spirit_stage = transfer_detail.spirit_stage_json();
	CONDITION_NOTIFY_RETURN(spirit_level.empty() == false && transfer_base.empty() == false
			&& spirit_stage.empty() == false, RETURN_MAKE_SPIRIT, ERROR_CONFIG_NOT_EXIST);

	int max_level = spirit_stage["max_level"].asInt();
	CONDITION_NOTIFY_RETURN(transfer_detail.level_ < max_level,
			RETURN_MAKE_SPIRIT, ERROR_CLIENT_OPERATE);

	int mult = 0;
	int type = request->type();
	SerialObj obj(SUB_MONEY_SPIRIT_UP_LVL, transfer_detail.level_, transfer_detail.exp_);

	switch (type)
	{
		case TransferDetail::TYPE_NORMAL:
		{
			int item_cost = spirit_level["item_cost"].asInt();
			int ret = this->sub_game_resource(GameEnum::ITEM_ID_SPIRIT, item_cost, obj);
			CONDITION_NOTIFY_RETURN(ret == 0, RETURN_MAKE_SPIRIT, ret);

			const Json::Value& normal_rate = transfer_base["normal_rate"];
			mult = this->fetch_make_spirit_mult(normal_rate);
			break;
		}
		case TransferDetail::TYPE_GOLD:
		{
			int max_gold_times = transfer_base["gold_times"].asInt();
			CONDITION_NOTIFY_RETURN(transfer_detail.gold_times_ < max_gold_times,
					RETURN_MAKE_SPIRIT, ERROR_CLIENT_OPERATE);

			int gold_cost = spirit_level["gold_cost"].asInt();
			Money money(gold_cost);
			int ret = this->pack_money_sub(money, obj);
			CONDITION_NOTIFY_RETURN(ret == 0, RETURN_MAKE_SPIRIT, ret);

			const Json::Value& gold_rate = transfer_base["gold_rate"];
			mult = this->fetch_make_spirit_mult(gold_rate);
			transfer_detail.gold_times_ += 1;
			break;
		}
		default:
		{
			return this->respond_to_client_error(RETURN_MAKE_SPIRIT, ERROR_CLIENT_OPERATE);
		}
	}

	int add_exp = GameCommon::rand_value(spirit_level["spirit"]);
	transfer_detail.exp_ += add_exp * mult;
	this->active_up_spirit_level();

	this->refresh_transfer_attr_add();
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);

	Proto51403002 respond;
	respond.set_level(transfer_detail.level_);
	respond.set_exp(transfer_detail.exp_);
	respond.set_type(type);
	respond.set_gold_times(transfer_detail.gold_times_);
	respond.set_mult(mult);
	FINER_PROCESS_RETURN(RETURN_MAKE_SPIRIT, &respond);
}

int MLTransfer::request_up_stage()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	const Json::Value& spirit_stage = transfer_detail.spirit_stage_json();
	const Json::Value& next_stage = CONFIG_INSTANCE->spirit_stage(transfer_detail.stage_+1);
	CONDITION_NOTIFY_RETURN(spirit_stage.empty() == false && next_stage.empty() == false,
			RETURN_UP_STAGE, ERROR_CLIENT_OPERATE);

	int max_level = spirit_stage["max_level"].asInt();
	CONDITION_NOTIFY_RETURN(transfer_detail.level_ >= max_level,
			RETURN_UP_STAGE, ERROR_CLIENT_OPERATE);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_UP_STAGE, ERROR_SERVER_INNER);

	const Json::Value& cost = spirit_stage["cost"];
	if (cost.empty() == false)
	{
		for (uint i = 0; i < cost.size(); ++i)
		{
			int item_id = cost[i][0u].asInt();
			int amount = cost[i][1u].asInt();
			PackageItem *pack_item = this->pack_find_by_id(item_id);
			CONDITION_NOTIFY_RETURN(pack_item != NULL && pack_item->__amount >= amount,
					RETURN_UP_STAGE, ERROR_PACKAGE_ITEM_AMOUNT);
		}

		for (uint i = 0; i < cost.size(); ++i)
		{
			int item_id = cost[i][0u].asInt();
			int amount = cost[i][1u].asInt();
			this->pack_remove(ITEM_TRANSFER_UP_STAGE, item_id, amount);
		}
	}

	transfer_detail.stage_ += 1;

	SerialObj obj(ADD_FROM_SPIRIT_UP_STAGE, transfer_detail.stage_);
	int reward_id = spirit_stage["reward_id"].asInt();
	this->add_reward(reward_id, obj);

	this->active_up_spirit_level();

	transfer_detail.reduce_cool_ = next_stage["reduce_cool"].asInt();
	transfer_detail.add_time_ 	 = next_stage["add_time"].asInt();

	this->refresh_transfer_attr_add();
	this->request_transfer_info();
	this->send_transfer_to_map(TransferDetail::SYNC_TRANSFER, false);
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);

	Proto51403003 respond;
	respond.set_level(transfer_detail.level_);
	respond.set_exp(transfer_detail.exp_);
	respond.set_stage(transfer_detail.stage_);
	FINER_PROCESS_RETURN(RETURN_UP_STAGE, &respond);
}

int MLTransfer::request_transfer_info()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.open_ == true, 0);

	Proto51403004 respond;
	respond.set_select_id(transfer_detail.active_id_);
	respond.set_transfer_tick(transfer_detail.transfer_tick_);
	respond.set_last(transfer_detail.last_);
	respond.set_is_in_cool(transfer_detail.is_in_cool());
	respond.set_is_in_transfer(transfer_detail.is_in_transfer());
	respond.set_cool(transfer_detail.transfer_cool());
	respond.set_open_reward(transfer_detail.open_reward_);

	for (TransferDetail::TransferInfoMap::iterator iter = transfer_detail.transfer_map_.begin();
			iter != transfer_detail.transfer_map_.end(); ++iter)
	{
		TransferDetail::TransferInfo &info = iter->second;

		ProtoTransferClient* transfer_proto = respond.add_transfer();
		transfer_proto->set_transfer_id(info.transfer_id_);
		transfer_proto->set_transfer_lv(info.transfer_lv_);
		transfer_proto->set_is_active(info.is_active_);
		transfer_proto->set_is_permanent(info.is_permanent_);
		transfer_proto->set_active_tick(info.active_tick_);
		transfer_proto->set_end_tick(info.end_tick_);
		transfer_proto->set_transfer_skill(info.transfer_skill_);

		const Json::Value& conf = info.conf();
		transfer_proto->set_cool(conf["cool"].asInt() * (1 - double(transfer_detail.reduce_cool_) / 100));
		transfer_proto->set_last(conf["last"].asInt() * (1 + double(transfer_detail.add_time_) / 100));

		info.one_prop_.serialize(transfer_proto->mutable_prop());

		if (info.transfer_id_ == transfer_detail.active_id_ && info.skill_info_.size() > 0)
		{
			for (SkillMap::iterator it = info.skill_info_.begin();
					it != info.skill_info_.end(); ++it)
			{
				ProtoSkill* proto_skill = transfer_proto->add_skill();
				it->second->serialize(proto_skill);
			}
		}
		else
		{
			for (IntMap::iterator it = info.skill_map_.begin();
					it != info.skill_map_.end(); ++it)
			{
				ProtoSkill* proto_skill = transfer_proto->add_skill();
				proto_skill->set_skill_id(it->first);
				proto_skill->set_skill_level(it->second);
			}
		}
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_TRANSFER_INFO, &respond);
}

int MLTransfer::request_change_transfer_id(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto11403005 *, request, msg, -1);

	return this->change_transfer_id(request->transfer_id());
}

int MLTransfer::change_transfer_id(int transfer_id)
{
	JUDGE_RETURN(transfer_id > 0, 0);

	TransferDetail &transfer_detail = this->transfer_detail();
	CONDITION_NOTIFY_RETURN(transfer_detail.open_ == true, RETURN_CHANGE_TRANSFER_ID,
			ERROR_TRANSFER_HAS_NOT_OPEN);
	CONDITION_NOTIFY_RETURN(transfer_detail.is_in_transfer() == false,
			RETURN_CHANGE_TRANSFER_ID, ERROR_TRANSFER_IS_IN_USE);

	TransferDetail::TransferInfo *transfer_info = transfer_detail.fetch_transfer_info(transfer_id);
	CONDITION_NOTIFY_RETURN(transfer_info != NULL, RETURN_CHANGE_TRANSFER_ID, ERROR_HAS_NOT_TRANSFER);
	CONDITION_NOTIFY_RETURN(transfer_info->is_active_time() == true,
			RETURN_CHANGE_TRANSFER_ID, ERROR_HAS_NOT_TRANSFER);
	CONDITION_NOTIFY_RETURN(transfer_detail.active_id_ != transfer_id,
			RETURN_CHANGE_TRANSFER_ID, ERROR_USE_SAME_TRANSFER);

	transfer_detail.active_id_ = transfer_id;

	this->request_transfer_info();

	this->send_transfer_to_map(TransferDetail::CHANGE_TRANSFER, true);
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);

	Proto51403005 respond;
	respond.set_tansfer_id(transfer_id);
	FINER_PROCESS_RETURN(RETURN_CHANGE_TRANSFER_ID, &respond);
}

int MLTransfer::request_use_transfer()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	CONDITION_NOTIFY_RETURN(transfer_detail.open_ == true, RETURN_USER_TRANSFER,
			ERROR_TRANSFER_HAS_NOT_OPEN);
	CONDITION_NOTIFY_RETURN(transfer_detail.active_id_ > 0, RETURN_USER_TRANSFER,
			ERROR_HAS_NOT_USE_TRANSFER);
	CONDITION_NOTIFY_RETURN(transfer_detail.is_in_cool() == false
			&& transfer_detail.is_in_transfer() == false,
			RETURN_USER_TRANSFER,ERROR_TRANSFER_IN_COOL);

	TransferDetail::TransferInfo *transfer_info = transfer_detail.fetch_transfer_info(
			transfer_detail.active_id_);
	CONDITION_NOTIFY_RETURN(transfer_info != NULL, RETURN_USER_TRANSFER, ERROR_HAS_NOT_TRANSFER);
	CONDITION_NOTIFY_RETURN(transfer_info->is_active_time() == true,
			RETURN_USER_TRANSFER, ERROR_HAS_NOT_TRANSFER);

	const Json::Value &conf = transfer_info->conf();
	CONDITION_NOTIFY_RETURN(conf.empty() == false, RETURN_USER_TRANSFER, ERROR_CONFIG_NOT_EXIST);

	transfer_detail.transfer_tick_ = ::time(NULL);
	transfer_detail.last_ = conf["last"].asInt() * (1 + double(transfer_detail.add_time_) / 100);

	this->create_transfer_in_time(true, true);

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);

	Proto51403006 respond;
	respond.set_transfer_id(transfer_detail.active_id_);
	respond.set_transfer_tick(transfer_detail.transfer_tick_);
	respond.set_last(transfer_detail.last_);
	FINER_PROCESS_RETURN(RETURN_USER_TRANSFER, &respond);
}

int MLTransfer::fetch_open_reward()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	CONDITION_NOTIFY_RETURN(transfer_detail.open_ == true, RETURN_GET_OPEN_REWARD,
			ERROR_TRANSFER_HAS_NOT_OPEN);
	CONDITION_NOTIFY_RETURN(transfer_detail.open_reward_ == false,
			RETURN_GET_OPEN_REWARD, ERROR_OPEN_REWARD_IS_GET);

	const Json::Value& transfer_base = transfer_detail.transfer_base_json();
	CONDITION_NOTIFY_RETURN(transfer_base.empty() == false,
			RETURN_GET_OPEN_REWARD, ERROR_CONFIG_NOT_EXIST);

	transfer_detail.open_reward_ = true;

	int reward_id = transfer_base["open_reward"].asInt();
	this->add_reward(reward_id, ADD_FROM_TRANSFER_OPEN);

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);

	Proto51403007 respond;
	respond.set_open_reward(transfer_detail.open_reward_);
	FINER_PROCESS_RETURN(RETURN_GET_OPEN_REWARD, &respond);
}

int MLTransfer::request_buy_transfer(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto11403008 *, request, msg, -1);

	TransferDetail &transfer_detail = this->transfer_detail();
	CONDITION_NOTIFY_RETURN(transfer_detail.open_ == true,
			RETURN_GET_OPEN_REWARD, RETURN_BUY_TRANSFER);

	int transfer_id = request->transfer_id();
	const Json::Value& conf = CONFIG_INSTANCE->transfer_total(transfer_id);
	CONDITION_NOTIFY_RETURN(conf.empty() == false, RETURN_GET_OPEN_REWARD,
			ERROR_CONFIG_NOT_EXIST);

	int cost = conf["cost"].asInt();
	CONDITION_NOTIFY_RETURN(cost > 0, RETURN_GET_OPEN_REWARD, ERROR_CAN_NOT_BUY_TRANSFER);

	Money money(cost);
	SerialObj obj(SUB_MONEY_BUY_TRANSFER_ID, transfer_id);
	int ret = this->pack_money_sub(money, obj);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_GET_OPEN_REWARD, ret);

	this->add_transfer(transfer_id, -1);
	this->request_transfer_info();

	Proto51403008 respond;
	respond.set_transfer_id(transfer_id);
	FINER_PROCESS_RETURN(RETURN_GET_OPEN_REWARD, &respond);
}

int MLTransfer::add_transfer(int id, int dur_time)
{
	const Json::Value &conf = CONFIG_INSTANCE->transfer_total(id);
	JUDGE_RETURN(conf.empty() == false, 0);

	TransferDetail &transfer_detail = this->transfer_detail();

	int use_flag = false;
	if (transfer_detail.transfer_map_.size() <= 0)
	{
		use_flag = true;
	}

	TransferDetail::TransferInfo *transfer_info = transfer_detail.fetch_transfer_info(id);
	if (transfer_info != NULL)
	{
		if (transfer_info->is_permanent_ == true)
		{
			const Json::Value& transfer_base = transfer_detail.transfer_base_json();
			int mail_id   = transfer_base["same_mail"].asInt();
			int reward_id = conf["reward_id"].asInt();
			JUDGE_RETURN(mail_id > 0 && reward_id > 0, 0);

			MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str(), conf["name"].asCString());

			mail_info->add_goods(reward_id);
			GameCommon::request_save_mail_content(this->role_id(), mail_info);
		}
		else
		{
			if (dur_time <= 0)
			{
				transfer_info->is_permanent_ = true;
				transfer_info->active_tick_  = -1;
				transfer_info->end_tick_ 	 = -1;

				//传闻
				this->show_broad(id, conf["broad"].asInt());
			}
			else if (transfer_info->is_active_time() == true)
			{
				transfer_info->end_tick_ += dur_time;
			}
			else
			{
				transfer_info->active_tick_ = ::time(NULL);
				transfer_info->end_tick_ = ::time(NULL) + dur_time;
			}
		}
		transfer_info->is_active_ = true;
	}
	else
	{
		TransferDetail::TransferInfo &new_info = transfer_detail.transfer_map_[id];
		new_info.transfer_id_ = id;
		new_info.transfer_lv_ = 1;
		new_info.is_active_   = true;
		if (dur_time <= 0)
		{
			new_info.is_permanent_ = true;
			new_info.active_tick_  = -1;
			new_info.end_tick_ 	   = -1;

			//传闻
			this->show_broad(id, conf["broad"].asInt());
		}
		else
		{
			new_info.active_tick_ = ::time(NULL);
			new_info.end_tick_ 	= ::time(NULL) + dur_time;
		}
		new_info.add_skill();
	}

	if (use_flag == true)
	{
		this->change_transfer_id(id);
		this->request_use_transfer();
	}

	//刷新属性
	this->refresh_transfer_attr_add();

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);
	this->request_transfer_info();

	return 0;
}

void MLTransfer::show_broad(int transfer_id, int show_id)
{
	JUDGE_RETURN(transfer_id > 0, ;);

	//获得新变身通知
	Proto81400802 active_res;
	active_res.set_transfer_id(transfer_id);
	this->respond_to_client(ACTIVE_ADD_NEW_TRANSFER, &active_res);

	JUDGE_RETURN(show_id > 0, ;);

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, this->name());
	GameCommon::announce(show_id, &para_vec);
}

void MLTransfer::check_handle_transfer_timeout()
{
	this->handle_transfer_time_out();
	this->handle_transfer_cd_time_out();
}

int MLTransfer::sync_transfer_trans(int scene_id)
{
	TransferDetail &transfer_detail = this->transfer_detail();

	Proto31400147 transfer_inner;
	transfer_inner.set_level(transfer_detail.level_);
	transfer_inner.set_exp(transfer_detail.exp_);
	transfer_inner.set_open(transfer_detail.open_);
	transfer_inner.set_stage(transfer_detail.stage_);
	transfer_inner.set_transfer_tick(transfer_detail.transfer_tick_);
	transfer_inner.set_last(transfer_detail.last_);
	transfer_inner.set_active_id(transfer_detail.active_id_);
	transfer_inner.set_open_reward(transfer_detail.open_reward_);
	transfer_inner.set_gold_times(transfer_detail.gold_times_);

	Int64 refresh_tick = transfer_detail.refresh_tick_.sec();
	transfer_inner.set_refresh_tick(refresh_tick);
	transfer_inner.set_reduce_cool(transfer_detail.reduce_cool_);
	transfer_inner.set_add_time(transfer_detail.add_time_);

	for (TransferDetail::TransferInfoMap::iterator iter = transfer_detail.transfer_map_.begin();
			iter != transfer_detail.transfer_map_.end(); ++iter)
	{
		TransferDetail::TransferInfo &transfer_info = iter->second;
		ProtoTransferInfo *proto_transfer = transfer_inner.add_transfer_info();
		transfer_info.serialize(proto_transfer);
	}

	return this->send_to_other_logic_thread(scene_id, transfer_inner);
}

int MLTransfer::read_transfer_trans(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto31400147 *, request, msg, -1);

	TransferDetail &transfer_detail = this->transfer_detail();
	transfer_detail.level_ 	= request->level();
	transfer_detail.exp_ 	= request->exp();
	transfer_detail.open_ 	= request->open();
	transfer_detail.stage_ 	= request->stage();
	transfer_detail.transfer_tick_ 	= request->transfer_tick();
	transfer_detail.last_ 	= request->last();
	transfer_detail.active_id_ 	= request->active_id();
	transfer_detail.open_reward_= request->open_reward();
	transfer_detail.gold_times_ = request->gold_times();

	Int64 refresh_tick = request->refresh_tick();
	Time_Value tick = Time_Value(refresh_tick);
	transfer_detail.refresh_tick_ = tick;

	transfer_detail.reduce_cool_ = request->reduce_cool();
	transfer_detail.add_time_ 	 = request->add_time();

	for (int i = 0; i < request->transfer_info_size(); ++i)
	{
		const ProtoTransferInfo &proto_transfer = request->transfer_info(i);

		int id = proto_transfer.transfer_id();
		TransferDetail::TransferInfo &transfer_info = transfer_detail.transfer_map_[id];
		transfer_info.unserialize(proto_transfer);
	}
	this->cal_total_transfer_attr();
	this->create_transfer_in_time(false, false);

	return 0;
}

void MLTransfer::transfer_login_operate()
{
	this->cal_spirit_advence();
	this->send_transfer_to_map(TransferDetail::SYNC_TRANSFER, false);
	this->create_transfer_in_time(true, true);
}

void MLTransfer::transfer_login_obtain()
{
	this->transfer_handle_player_levelup();
}

void MLTransfer::transfer_handle_player_levelup()
{
	this->check_level_open_transfer();
}

void MLTransfer::transfer_handle_player_task(int task_id)
{
	this->check_task_open_transfer(task_id);
}

void MLTransfer::check_spirit_day_reset()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	Time_Value nowtime = Time_Value::gettimeofday();
	if (transfer_detail.refresh_tick_ <= nowtime)
	{
		transfer_detail.reset_every_day();
		this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_TRANSFER_INFO, true);
	}

	this->request_spirit_info();
}

TransferDetail &MLTransfer::transfer_detail()
{
	return this->transfer_detail_;
}

void MLTransfer::test_reset_transfer_cd()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	transfer_detail.transfer_tick_ = 0;
	transfer_detail.last_ = 0;

	this->request_transfer_info();
}

