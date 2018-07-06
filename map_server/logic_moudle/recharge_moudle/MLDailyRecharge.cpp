/*
 * MLDailyRecharge.cpp
 *
 *  Created on: Nov 3, 2014
 *      Author: jinxing
 */

#include "MapLogicPlayer.h"
#include "MLDailyRecharge.h"
#include "ProtoDefine.h"
#include "MLBackDailyRechargeSys.h"
#include "MapMonitor.h"
#include "SerialRecord.h"

MLDailyRecharge::MLDailyRecharge()
{
	// xTODO Auto-generated constructor stub
}

MLDailyRecharge::~MLDailyRecharge()
{
	// xTODO Auto-generated destructor stub
}

void MLDailyRecharge::daily_recharge_clean()
{
//	BackDR_SYS->send_mail_to_player(this->role_id());

	int prev_recharge = this->daily_recharge_dtail_.__today_recharge_gold;
	int prev_first_gold = this->daily_recharge_dtail_.__first_recharge_gold;
	int prev_act_num = this->daily_recharge_dtail_.__act_recharge_times;
	int prev_act_mail = this->daily_recharge_dtail_.__act_has_mail;

	IntMap prev_last_recharge_map;
	for (int i = DAILY_FIRST; i < TYPE_NUM; ++i)
	{
		if (this->daily_recharge_dtail_.__daily_recharge_rewards[i] == REWARD_HAVE)
			this->daily_recharge_dtail_.__daily_recharge_rewards[i] = REWARD_GONE;

		prev_last_recharge_map[i] = this->daily_recharge_dtail_.__daily_recharge_rewards[i];
	}

	MLDailyRecharge::reset();
	this->daily_recharge_dtail_.__first_recharge_gold = prev_first_gold;
	this->daily_recharge_dtail_.__act_recharge_times = prev_act_num;
	this->daily_recharge_dtail_.__act_has_mail = prev_act_mail;

	if (this->daily_recharge_dtail_.__first_recharge_gold == 2)
	{
		this->notify_daily_recharge_info();
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS, true);
		return ;
	}

	if(prev_recharge >= ACT_TOTAL_LIMIT)
	{
		this->daily_recharge_dtail_.__first_recharge_gold = 2;
	}
	else
	{
		for (int i = DAILY_FIRST; i < TYPE_NUM; ++i)
		{
			this->daily_recharge_dtail_.__daily_recharge_rewards[i] = prev_last_recharge_map[i];
		}
	}

	this->notify_daily_recharge_info();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS, true);
}

void MLDailyRecharge::reset()
{
	this->daily_recharge_dtail_.reset();
//	init_daily_recharge_rewards();
}

void MLDailyRecharge::init_daily_recharge_rewards()
{
//	const Json::Value &steps_json = CONFIG_INSTANCE->daily_recharge_json()["steps"];
//	JUDGE_RETURN(!steps_json.empty(),);

//	MSG_DEBUG("steps_json: %s", steps_json.toStyledString().c_str());
	IntMap &rewards_state = this->daily_recharge_dtail_.__daily_recharge_rewards;
//	rewards_state.reserve(steps_json.size());
//	for(size_t idx=0; idx<steps_json.size(); idx++)
//	{
//		if(rewards_state.size() <= idx)
//			rewards_state.push_back(REWARD_NONE);
//	}
	for (int i = 0; i < TYPE_NUM; ++i)
	{
		if (rewards_state[i] != 1)
			rewards_state[i] = 0;
	}
}

int MLDailyRecharge::check_get_label_event()
{
	if (this->daily_recharge_dtail_.__act_recharge_times == ACT_REACHARGE_TIMES + 1)
	{
		this->daily_recharge_dtail_.__act_has_mail = 1;
	}

	//新的规则
	if (this->daily_recharge_dtail_.__act_has_mail == 0)
	{
		if (this->map_logic_player()->recharge_detail().__recharge_money >= ACT_NEW_RECHARGE_NUM)
		{
			int mail_id = ACT_MAIL_ID;
			MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
						mail_info->mail_content_.c_str());
			int reward_id = CONFIG_INSTANCE->const_set("act_recharge_label");
			mail_info->add_goods(reward_id);
			GameCommon::request_save_mail_content(this->role_id(), mail_info);
			this->daily_recharge_dtail_.__act_has_mail = 1;
		}
	}

	return 0;
}

int MLDailyRecharge::handle_recharge_event(size_t gold)
{
//#ifndef DAILY_RECHARGE
//	return 0;
//#endif

	JUDGE_RETURN(gold > 0, -1);
	JUDGE_RETURN(is_daily_recharge_during(), ERROR_ACTIVITY_ENDED);

	this->daily_recharge_dtail_.__today_recharge_gold += gold;

	this->act_serial_info_update(SERIAL_ACT_DAILY_RECHARGE, this->map_logic_player()->role_id(),
			this->daily_recharge_dtail_.__today_recharge_gold);

	//开服活动额外称号
	if (this->daily_recharge_dtail_.__today_recharge_gold >= ACT_REACHARGE_NUM
			&& CONFIG_INSTANCE->open_server_days() <= ACT_OPEN_DAY
			&& this->daily_recharge_dtail_.__act_recharge_times < ACT_REACHARGE_TIMES
			&& !is_same_day(Time_Value(this->daily_recharge_dtail_.__last_recharge_time),
					Time_Value(Time_Value::gettimeofday().sec())))
		this->daily_recharge_dtail_.__act_recharge_times++;

	if (this->daily_recharge_dtail_.__act_recharge_times == ACT_REACHARGE_TIMES
			&& this->daily_recharge_dtail_.__act_has_mail == 0)
	{
		int mail_id = ACT_MAIL_ID;
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str());
		int reward_id = CONFIG_INSTANCE->const_set("act_recharge_label");
		mail_info->add_goods(reward_id);
		GameCommon::request_save_mail_content(this->role_id(), mail_info);
		this->daily_recharge_dtail_.__act_recharge_times++;
		this->daily_recharge_dtail_.__act_has_mail = 1;
	}

	this->check_get_label_event();

	this->daily_recharge_dtail_.__last_recharge_time = Time_Value::gettimeofday().sec();

//	const Json::Value &steps_json = CONFIG_INSTANCE->daily_recharge_json()["steps"];
//	JUDGE_RETURN(!steps_json.empty(), ERROR_CONFIG_ERROR);

	bool new_reward = false;
	IntMap &rewards_state = this->daily_recharge_dtail_.__daily_recharge_rewards;

	int first_gold = this->get_gold_num(0);
	int total_gold = this->get_gold_num(1);

	for (int i = 0; i < TYPE_NUM; ++i)
	{
		if (rewards_state[i] == REWARD_GONE) continue;
		if (i == 0 && this->daily_recharge_dtail_.__today_recharge_gold >= first_gold)
		{
			rewards_state[i] = REWARD_HAVE;
			new_reward = true;
		}
		if (i > 0 && this->daily_recharge_dtail_.__today_recharge_gold >= total_gold)
		{
			rewards_state[i] = REWARD_HAVE;
			new_reward = true;
		}

		if (i == DAILY_TOTAL && rewards_state[i] == REWARD_HAVE)
		{
			MapLogicPlayer* player = this->map_logic_player();
			JUDGE_CONTINUE(player != NULL);

			player->active_hi_treasure();
		}
	}
	if(new_reward)
	{
		notify_daily_recharge_info();
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS, true);
	}
	return 0;
}

int MLDailyRecharge::fetch_daily_recharge_info()
{
	return this->notify_daily_recharge_info();
}

int MLDailyRecharge::notify_daily_recharge_info(int show_icon)
{
	DailyRechargeDetail &detail = this->daily_recharge_dtail();

	Proto81401803 msg;
	for (int i = 0; i < TYPE_NUM; ++i)
	{
		int reward_id = this->get_reward_id(i);
		if (reward_id == 0) continue;
		msg.add_reward_list(reward_id);
		msg.add_rewards_state(detail.__daily_recharge_rewards[i]);
	}

	show_icon = 1;
	if (detail.__daily_recharge_rewards[DAILY_FIRST] == REWARD_GONE &&
			detail.__daily_recharge_rewards[DAILY_TOTAL] == REWARD_GONE)
		show_icon = 0;

	msg.set_first_gold(this->get_gold_num(0));
	msg.set_total_gold(this->get_gold_num(1));
	msg.set_today_recharge(detail.__today_recharge_gold);
	msg.set_first_recharge(detail.__first_recharge_gold);
	msg.set_vip_level(this->vip_detail().__vip_level);
	msg.set_index(this->get_index_num());
	msg.set_show_icon(show_icon);

	this->update_daily_recharge_to_sys();

	return this->respond_to_client(ACTIVE_NOTIFY_DAILY_RECHARGE_INFO, &msg);
}

int MLDailyRecharge::update_daily_recharge_to_sys()
{
	DailyRechargeDetail &detail = this->daily_recharge_dtail();

	Proto31400060 inner;
	inner.set_role_id(this->role_id());
	inner.set_today_recharge_gold(detail.__today_recharge_gold);
	inner.set_first_recharge_gold(detail.__first_recharge_gold);
	inner.set_last_recharge_time(detail.__last_recharge_time);
	inner.set_act_recharge_times(detail.__act_recharge_times);
	inner.set_act_has_mail(detail.__act_has_mail);
	for(int i = 0; i < TYPE_NUM; ++i)
	{
		inner.add_reward(detail.__daily_recharge_rewards[i]);
	}

	return this->send_to_other_logic_thread(GameEnum::RECHARGE_BASE_SCENE_ID, inner);
}

int MLDailyRecharge::fetch_daily_recharge_rewards(Message *msg)
{
	DYNAMIC_CAST_NOTIFY(Proto11401804*, request, msg, ERROR_CLIENT_OPERATE);

	IntMap &rewards_state = this->daily_recharge_dtail_.__daily_recharge_rewards;
	int reward_idx = request->reward_index();

	CONDITION_NOTIFY_RETURN(reward_idx >= 0 && reward_idx < TYPE_NUM,
			RETURN_FETCH_DAILY_RECHARGE_REWARDS, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(rewards_state[reward_idx] != REWARD_NONE,
			RETURN_FETCH_DAILY_RECHARGE_REWARDS, ERROR_NO_REWARD);
	CONDITION_NOTIFY_RETURN(rewards_state[reward_idx] != REWARD_GONE,
			RETURN_FETCH_DAILY_RECHARGE_REWARDS, ERROR_REWARD_DRAWED);

	if(rewards_state[reward_idx] == REWARD_HAVE)
	{
		int reward_id = this->get_reward_id(reward_idx);

		this->add_reward(reward_id,SerialObj(ADD_FROM_ITEM_DAILY_RECHARGE));
		if (reward_idx)
		{
			rewards_state[TOTAL_EXT] = REWARD_GONE;
			for (int i = 0; i < request->item_list_size(); ++i)
			{
				int id = request->item_list(i).id();
				int num = request->item_list(i).amount();
				int bind = request->item_list(i).bind();
				this->insert_package(SerialObj(ADD_FROM_ITEM_DAILY_RECHARGE), id, num, bind);
			}
		}
		rewards_state[reward_idx] = REWARD_GONE;

		//首充传闻
		int shout_id = ACT_CAST_ID;
		BrocastParaVec para_vec;
		const Json::Value& reward_json = CONFIG_INSTANCE->reward(reward_id)["fix_goods_list"];
		int item_id_1 = reward_json[0u][0u].asInt();
		int item_id_2 = reward_json[1u][0u].asInt();
		string item_name_1 = CONFIG_INSTANCE->item(item_id_1)["name"].asString();
		string item_name_2 = CONFIG_INSTANCE->item(item_id_2)["name"].asString();

		int gold = 0;
		if (reward_idx)
		{
			gold = ACT_TOTAL_LIMIT;
		}
		else
		{
			gold = ACT_FIRST_LIMIT;
		}

		GameCommon::push_brocast_para_string(para_vec, this->role_name());
		GameCommon::push_brocast_para_int(para_vec, gold);
		GameCommon::push_brocast_para_string(para_vec, item_name_1);
		GameCommon::push_brocast_para_string(para_vec, item_name_2);

		GameCommon::announce(shout_id, &para_vec);

		this->cache_tick().update_cache(MapLogicPlayer::CACHE_RECHARGE_REWARDS, true);
	}
	else
	{
		return this->respond_to_client_error(RETURN_FETCH_DAILY_RECHARGE_REWARDS, ERROR_SERVER_INNER);
	}



	return notify_daily_recharge_info();
}

bool MLDailyRecharge::is_daily_recharge_during()
{
	return true; //test
//	return BackDR_SYS->daily_recharge_opening();
}

DailyRechargeDetail& MLDailyRecharge::daily_recharge_dtail()
{
	return this->daily_recharge_dtail_;
}

int MLDailyRecharge::sync_transfer_daily_recharge(int scene_id)
{
	Proto31400131 info;

	DailyRechargeDetail &detail = this->daily_recharge_dtail();
	info.set_today_gold(detail.__today_recharge_gold);
	info.set_first_recharge(detail.__first_recharge_gold);
	info.set_last_recharge(detail.__last_recharge_time);
	info.set_act_recharge_times(detail.__act_recharge_times);
	info.set_act_has_mail(detail.__act_has_mail);
	for(int i = 0; i < TYPE_NUM; ++i)
	{
		info.add_rewards(detail.__daily_recharge_rewards[i]);
	}

	return this->send_to_other_logic_thread(scene_id, info);
}

int MLDailyRecharge::read_transfer_daily_recharge(Message *msg)
{
//#ifndef DAILY_RECHARGE
//	return 0;
//#endif

	MSG_DYNAMIC_CAST_RETURN(Proto31400131*, request, -1);

	DailyRechargeDetail &detail = this->daily_recharge_dtail();
	detail.reset();
	detail.__today_recharge_gold = request->today_gold();
	detail.__last_recharge_time = request->last_recharge();
	detail.__first_recharge_gold = request->first_recharge();
	detail.__act_recharge_times = request->act_recharge_times();
	detail.__act_has_mail = request->act_has_mail();
	for(int i = 0; i< request->rewards_size(); ++i)
		detail.__daily_recharge_rewards[i] = request->rewards(i);

	return 0;
}

int MLDailyRecharge::get_gold_num(int type)
{
	int id = this->get_json_id();
	int open_day = CONFIG_INSTANCE->open_server_days();
	const Json::Value &dr_json = CONFIG_INSTANCE->daily_recharge_json(id);
	int week_num = open_day % dr_json["first_recharge"].size();
	if (id == 1) week_num = 0;

	return dr_json["recharge_list"][week_num][type].asInt();
}

int MLDailyRecharge::get_index_num()
{
	int id = this->get_json_id();
	int open_day = CONFIG_INSTANCE->open_server_days();
	const Json::Value &dr_json = CONFIG_INSTANCE->daily_recharge_json(id);
	int week_num = open_day % dr_json["first_recharge"].size();
	if (id == 1) week_num = 0;
	return dr_json["index_list"][week_num].asInt();
}

int MLDailyRecharge::get_json_id()
{
	int id = 0;
	int open_day = CONFIG_INSTANCE->open_server_days();
	if (this->daily_recharge_dtail_.__first_recharge_gold <
			CONFIG_INSTANCE->daily_recharge_json(1)["recharge_times"].asInt())
	{
		id = 1;
	}
	else
	{
		int size = CONFIG_INSTANCE->daily_recharge_json().size();
		for (int i = size; i > 1; --i)
		{
			const Json::Value &dr_json = CONFIG_INSTANCE->daily_recharge_json(i);
			if (dr_json["open_day"].asInt() <= open_day)
			{
				id = i;
				break;
			}
		}
	}
	return id;
}

int MLDailyRecharge::get_reward_id(int type)
{
	int reward_id = 0;
	int id = this->get_json_id();
	int open_day = CONFIG_INSTANCE->open_server_days();
	const Json::Value &dr_json = CONFIG_INSTANCE->daily_recharge_json(id);
	int week_num = open_day % dr_json["first_recharge"].size();
	if (id == 1) week_num = 0;

	switch(type)
	{
	case 0:
		reward_id = dr_json["first_recharge"][week_num].asInt();
		break;
	case 1:
		reward_id = dr_json["total_recharge"][week_num].asInt();
		break;
	case 2:
		reward_id = dr_json["ext_award"][week_num].asInt();
		break;
	}
	return reward_id;
}
