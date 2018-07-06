/*
 * MLVipPlayer.cpp
 *
 *  Created on: 2013-12-3
 *      Author: root
 */

#include "MLVipPlayer.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

MLVipPlayer::MLVipPlayer()
{
}

MLVipPlayer::~MLVipPlayer()
{
}

void MLVipPlayer::reset()
{
    this->vip_detail_.reset();
}

//是否是VIP玩家
bool MLVipPlayer::is_vip()
{
	return this->vip_detail_.is_vip();
}

bool MLVipPlayer::is_max_vip()
{
	return this->vip_detail_.is_max_vip();
}

int MLVipPlayer::vip_type(void)
{
    return this->vip_detail_.__vip_type;
}

int MLVipPlayer::gain_vip_gift(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto11400803 *, request, -1);

    int vip_type = request->vip_type();
    CONDITION_NOTIFY_RETURN(vip_type > 0 && this->vip_type() >= vip_type,
    		RETURN_GAIN_VIP_GIFT, ERROR_VIP_LEVEL);

    const Json::Value& gift_config = CONFIG_INSTANCE->vip(vip_type);
    CONDITION_NOTIFY_RETURN(gift_config != Json::Value::null,
    		RETURN_GAIN_VIP_GIFT, ERROR_CONFIG_NOT_EXIST);

    if (request->reward_type() == 0)
    {
        CONDITION_NOTIFY_RETURN(this->vip_detail_.__is_given[vip_type] == 0,
        		RETURN_GAIN_VIP_GIFT, ERROR_REWARD_DRAWED);

        this->add_reward(gift_config["up_reward"].asInt(), ADD_FROM_VIP_GIFT);
    	this->vip_detail_.__is_given[vip_type] = 1;
    }
    else
    {
		RewardInfo reward_info_total;
    	for (int i = VIP_1; i <= this->vip_type(); ++i)
    	{
    		JUDGE_CONTINUE(this->vip_detail_.__weekly_given[i] == 0);
    		this->vip_detail_.__weekly_given[i] = 1;

    	    const Json::Value& week_gift = CONFIG_INSTANCE->vip(i);
    		int award_id = week_gift["weekly_reward"].asInt();
			const Json::Value& reward_json = CONFIG_INSTANCE->reward(award_id);
			JUDGE_CONTINUE(reward_json.empty() == false);

			RewardInfo reward_info;
			GameCommon::make_up_reward_items(reward_info, reward_json);
			reward_info_total.add_rewards(reward_info.item_vec_);
    	}

    	if (reward_info_total.item_vec_.empty() == false)
    	{
			this->insert_package(ADD_FROM_VIP_GIFT, reward_info_total);
    	}

        this->vip_detail_.__weekly_tick = Time_Value::gettimeofday();
    }

    this->fetch_vip_info();
    this->cache_tick().update_cache(MapLogicPlayer::CACHE_VIP);

    return 0;
}

int MLVipPlayer::justify_recharge_gold()
{
	JUDGE_RETURN(this->is_max_vip() == false, 0);

	int next_vip = this->vip_type();
    int total_recharge_gold = this->total_recharge_gold();

    for(int i = this->vip_type() + 1; i <= VIP_MAX; ++i)
    {
        int need_gold = CONFIG_INSTANCE->vip(i)["exp"].asInt();
        JUDGE_BREAK(total_recharge_gold >= need_gold);

        next_vip = i;
    }

    if (next_vip != this->vip_type())
    {
    	//VIP升级
    	this->on_vip_status_upgrade(next_vip);
    }

    return this->fetch_vip_info();
}

int MLVipPlayer::fetch_vip_info(void)
{
    Proto51400804 vip_info;
    vip_info.set_vip_type(this->vip_type());

    //VIP升级信息
	int vip_type = std::max<int>(VIP_1, this->vip_type());
	int need_gold = CONFIG_INSTANCE->vip(vip_type)["exp"].asInt();
	if (this->is_max_vip())
	{
		vip_info.set_need_gold(0);
		vip_info.set_gold(need_gold);
	}
	else
	{
		vip_info.set_need_gold(need_gold);
		vip_info.set_gold(this->total_recharge_gold());
	}

    //VIP奖励领取情况
    if (!(GameCommon::is_same_week(this->vip_detail_.__weekly_tick)))
    {
    	this->vip_detail_.__weekly_given.clear();
        this->vip_detail_.__weekly_tick = Time_Value::gettimeofday();
    }

    for(int i = VIP_1; i <= VIP_MAX; ++i)
    {
    	//升级奖励
    	ProtoVipGift* gift_info = vip_info.add_vip_gift();
        int reward_id = CONFIG_INSTANCE->vip(i)["up_reward"].asInt();
        gift_info->set_vip_type(i);
        gift_info->set_reward_id(reward_id);
        gift_info->set_ishasgift(this->vip_detail_.is_has_reward(i, 0));

        //周礼包
        ProtoVipGift* w_gift_info = vip_info.add_vip_weekly_gift();
        reward_id = CONFIG_INSTANCE->vip(i)["weekly_reward"].asInt();
        w_gift_info->set_vip_type(i);
        w_gift_info->set_reward_id(reward_id);
        w_gift_info->set_ishasgift(this->vip_detail_.is_has_reward(i, 1));
    }

    FINER_PROCESS_RETURN(RETURN_VIP_INFO, &vip_info);
}

int MLVipPlayer::notify_sync_vip_info(bool is_notify)
{
    int vip_type = this->vip_detail().__vip_type;
    int start_time = this->vip_detail().__start_time;
    int expired_time = this->vip_detail().__expired_time;

    //战斗线程
    Proto30400801 sync_map_info;
    sync_map_info.set_vip_type(vip_type);
    sync_map_info.set_vip_start_time(start_time);
    sync_map_info.set_expired_time(expired_time);
    sync_map_info.set_is_notify(is_notify);
    this->send_to_map_thread(sync_map_info);

    Proto30100801 sync_logic_info;
    sync_logic_info.set_vip_type(vip_type);
    sync_logic_info.set_vip_start_time(start_time);
    sync_logic_info.set_period_time(expired_time);
    MAP_MONITOR->dispatch_to_logic(this, &sync_logic_info);

    Proto30200122 sync_chat_info;
    sync_chat_info.set_vip_type(this->vip_detail().__vip_level); //amend
    return MAP_MONITOR->dispatch_to_chat(this->map_logic_player(), &sync_chat_info);
}

//VIP玩家免费传送的次数
int MLVipPlayer::vip_free_transfer(void)
{
    JUDGE_RETURN(true == is_vip(), 0);
    return CONFIG_INSTANCE->vip(this->vip_type())["free_transfer"].asInt();
}

int MLVipPlayer::sync_transfer_vip(int scene_id)
{
    Proto31400113 vip_info;
    vip_info.set_vip_type(this->vip_detail_.__vip_type);
    vip_info.set_vip_start_time(this->vip_detail_.__start_time);
    vip_info.set_check_flag(this->vip_detail_.__check_flag);
    vip_info.set_period_time(this->vip_detail_.__expired_time);
    vip_info.set_weekly_tick(this->vip_detail_.__weekly_tick.sec());
    vip_info.set_super_vip_type(this->vip_detail_.__super_get_type);
    vip_info.set_des_mail(this->vip_detail_.__des_mail);

    for( uint i = VIP_1; i <= VIP_MAX; ++i )
    {
        ProtoPairObj* p = vip_info.add_is_gven();
        p->set_obj_id(i);
        p->set_obj_value(this->vip_detail_.__is_given[i]);

        ProtoPairObj* w = vip_info.add_weekly();
        w->set_obj_id(i);
        w->set_obj_value(this->vip_detail_.__weekly_given[i]);
    }

    return this->send_to_other_logic_thread(scene_id, vip_info);
}

int MLVipPlayer::read_transfer_vip(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400113*, request, -1);

    this->vip_detail_.set_vip_type(request->vip_type());
    this->vip_detail_.__start_time = request->vip_start_time();
    this->vip_detail_.__expired_time = request->period_time();
    this->vip_detail_.__check_flag = request->check_flag();
    this->vip_detail_.__weekly_tick.sec(request->weekly_tick());
    this->vip_detail_.__super_get_type = request->super_vip_type();
    this->vip_detail_.__des_mail = request->des_mail();

    for( int i = 0; i < request->is_gven_size(); ++i )
    {
        const ProtoPairObj p = request->is_gven(i);
        this->vip_detail_.__is_given[p.obj_id()]= p.obj_value();

        const ProtoPairObj w = request->weekly(i);
        this->vip_detail_.__weekly_given[w.obj_id()] = w.obj_value();
    }

    return 0;
}

MLVipDetail& MLVipPlayer::vip_detail()
{
    return this->vip_detail_;
}

void MLVipPlayer::check_and_adjust_vip()
{
	JUDGE_RETURN(this->vip_detail_.__check_flag == false, ;);
	JUDGE_RETURN(this->total_recharge_gold() > 0, ;);

	this->vip_detail_.__check_flag = true;
	this->justify_recharge_gold();
}

void MLVipPlayer::on_vip_status_upgrade(int vip_type)
{
    this->vip_detail_.set_vip_type(vip_type);

    //同步到其他线程
    this->notify_sync_vip_info(true);

    //投资计划VIP奖励
    MapLogicPlayer* player = this->map_logic_player();
    player->check_invest_vip_reward();
    player->sync_fight_property_to_map();
	player->change_magic_weapon_status(GameEnum::RAMA_GET_FROM_VIP, vip_type);
	player->update_player_assist_single_event(GameEnum::PLAYER_ASSIST_EVENT_VIP, 1);

    Proto80400803 respond;
    respond.set_vip_type(vip_type);
    this->respond_to_client(ACTIVE_VIP_UPGRADE, &respond);
    this->cache_tick().update_cache(MapLogicPlayer::CACHE_VIP);
}

int MLVipPlayer::vip_addtion_exp(const int inc_value)
{
    JUDGE_RETURN(this->is_vip() == true, inc_value);

	const Json::Value vip_json = CONFIG_INSTANCE->vip(this->vip_type());
	JUDGE_RETURN(Json::Value::null != vip_json, inc_value);

	double vip_exp_percent = vip_json["extra_exp"].asDouble() / 100.0;
	return (inc_value * vip_exp_percent);
}

int MLVipPlayer::calc_vip_prop(IntMap& prop_map)
{
    JUDGE_RETURN(this->is_vip() == true, 0);

    const Json::Value &vip_json = CONFIG_INSTANCE->vip(this->vip_type());
	if (vip_json.isMember("hurt_reduce"))
	{
		GameCommon::add_prop_attr(prop_map, GameEnum::REDUCTION_MULTI,
				(vip_json["hurt_reduce"].asDouble() * GameEnum::DAMAGE_ATTR_PERCENT));
	}
	return 0;
}

int MLVipPlayer::fetch_super_vip_info_begin()
{
	JUDGE_RETURN(this->is_vip() == true, 0);

	Proto31400059 request_info;
	request_info.set_role_id(this->role_id());
	request_info.set_agent_code(this->role_detail().__agent_code);
	return MAP_MONITOR->dispatch_to_logic(this, &request_info);
}

int MLVipPlayer::fetch_super_vip_info_end(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400059*, request, -1);
    JUDGE_RETURN(this->vip_detail().__vip_level >= request->limit(), 0);
    //JUDGE_RETURN(this->check_open_super_vip(request->type(), request->limit()), 0);

    this->vip_detail_.__des_mail = request->des_mail();

    Proto51400805 respond;
    respond.set_qq_num(request->qq_num());
    respond.set_des_content(request->des_content());

    if (this->vip_detail_.__super_get_type == false)
    {
    	respond.set_is_open(1);
    }
    else
    {
    	respond.set_is_open(0);
    }

    this->respond_to_client(RETURN_SUPER_VIP_INFO, &respond);
    return 0;
}

bool MLVipPlayer::check_open_super_vip(int type, int limit)
{
	MapLogicPlayer* player = this->map_logic_player();
	if (type == 1)
	{
		int total_recharge = player->total_recharge_gold();
		JUDGE_RETURN(limit >= total_recharge, false);
	}
	else
	{
		int vip_level = this->vip_detail().__vip_level;
		JUDGE_RETURN(limit >= vip_level, false);
	}

	return true;
}
