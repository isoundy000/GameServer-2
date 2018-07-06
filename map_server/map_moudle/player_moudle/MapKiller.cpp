/*
 * MapKiller.cpp
 *
 * Created on: 2014-05-04 10:42
 *     Author: lyz
 */

#include "MapKiller.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "TipsEnum.h"

void KilledInfo::reset()
{
	this->is_yellow_ 	= 0;
	this->kill_num_ 	= 0;
	this->is_brocast_ 	= 0;
	this->online_ticks_	= 0;
	this->killing_value_= 0;
    this->attackor_map_.clear();
}

int KilledInfo::pk_value()
{
	return 0;
}

MapKiller::~MapKiller(void)
{ /*NULL*/ }


void MapKiller::reset_map_killer(void)
{
	this->killed_info_.reset();
    this->monster_map_.clear();
    this->task_monster_map_.clear();
}

void MapKiller::reset_killer_everyday()
{
    this->killed_info_.kill_num_ = 0;
    this->killed_info_.is_yellow_ = 0;
}

int MapKiller::inc_kill_monster(const int sort, const int amount)
{
    this->monster_map_[sort] += amount;
    return 0;
}

int MapKiller::inc_task_monster(const int sort, const int amount)
{
    this->task_monster_map_[sort] += amount;

    return 0;
}

int MapKiller::killer_time_up(const Time_Value &nowtime)
{
    this->refresh_kill_monster_info();
    this->refresh_task_monster_info();

    return 0;
}

int MapKiller::refresh_kill_monster_info(void)
{
	JUDGE_RETURN(this->monster_map_.size() > 0, 0);

    Proto31400014 request;
    for (BIntMap::iterator iter = this->monster_map_.begin();
            iter != this->monster_map_.end(); ++iter)
    {
        ProtoMonster *proto_monster = request.add_monster_set();
        proto_monster->set_sort(iter->first);
        proto_monster->set_total_amount(iter->second);
    }

    this->monster_map_.clear();
    return this->send_to_logic_thread(request);
}

int MapKiller::refresh_task_monster_info(void)
{
	JUDGE_RETURN(this->task_monster_map_.size() > 0, 0);

	Proto31400022 request;
	for (BIntMap::iterator iter = this->task_monster_map_.begin();
			iter != this->task_monster_map_.end(); ++iter)
	{
		ProtoMonster *proto_monster = request.add_monster_set();
		proto_monster->set_sort(iter->first);
		proto_monster->set_total_amount(iter->second);
	}

    this->task_monster_map_.clear();
    return this->send_to_logic_thread(request);
}

int MapKiller::die_not_notify()
{
	return false;
}

int MapKiller::notify_kill_fighter(Int64 attacker_id)
{
//	JUDGE_RETURN(this->die_not_notify() == false, -1);
//
//    Int64 benefited_id = this->fetch_benefited_attackor_id(attacker_id);
//    JUDGE_RETURN(benefited_id != this->fighter_id(), -1);
//
//    MapPlayerEx *attack_player = this->find_player_with_offline(benefited_id);
//    JUDGE_RETURN(attack_player != NULL, -1);
//
//	Proto80401102 killer_info;
//	killer_info.set_killer_id(attack_player->role_id());
//	killer_info.set_killer_name(attack_player->role_name());
//	this->respond_to_client(ACTIVE_SHUSAN_KILLER, &killer_info);
//
//	Proto80401103 killed_info;
//	killed_info.set_bekilled_id(this->fighter_id());
//	killed_info.set_bekilled_name(this->name());
//	attack_player->respond_to_client(ACTIVE_KILL_PLAYER, &killed_info);
//
//    this->handle_kill_ditheism(attack_player);
//    attack_player->check_by_light_kill();
    return 0;
}

int MapKiller::record_hurt_attackor(Int64 attackor, int fight_tips)
{
	JUDGE_RETURN(this->is_death() == false, -1);
	JUDGE_RETURN(this->validate_kill_value_scene() == true, -1);

	MapPlayerEx* player = this->find_player(attackor);
	JUDGE_RETURN(player != NULL && player->role_id() != this->fighter_id(), -1);

	// 攻击者不是反击
	JUDGE_RETURN(player->is_attack_by_id(this->fighter_id()) == false, -1);
	// 红名玩家没有反击权利
	JUDGE_RETURN(this->fetch_name_color() >= GameEnum::NAME_WHITE, -1);


	if (this->is_attack_by_id(player->role_id()) == false)	// 第一次被攻击
	{
		// 不是玩家主动攻击的不能反击
		JUDGE_RETURN(player->role_id() == attackor, -1);

		Proto80400375 notify_info;
		notify_info.set_attackor(player->role_name());
		notify_info.set_attackor_id(player->role_id());
		this->respond_to_client(ACTIVE_BE_ATTACK_NOW, &notify_info);
	}

	//记录主动攻击方
	Int64 t_nowtime = ::time(NULL);
	this->killed_info_.attackor_map_[player->role_id()] = t_nowtime;
	return 0;
}

int MapKiller::handle_kill_ditheism(MapKiller* attacker, int add_test)
{
	JUDGE_RETURN(this->validate_kill_value_scene() == true, -1);

	int name_color = this->fetch_name_color();
	int attack_color = attacker->fetch_name_color();

    {
        // 排行统计计算击杀的数量
        Proto30100105 inner_req;
        if (name_color < GameEnum::NAME_WHITE)
        {
            inner_req.set_kill_evil(1);
        }
        else
        {
            inner_req.set_kill_normal(1);
        }
        this->monitor()->dispatch_to_logic(attacker, &inner_req);
    }

	if (add_test > 0 || name_color < GameEnum::NAME_WHITE)
	{
		// 红名杀白名以上必加杀戳值
		if (add_test > 0 || attack_color >= GameEnum::NAME_WHITE)
		{
			attacker->killed_info_.kill_num_ += 1;
			attacker->killed_info_.kill_num_ += add_test;
		}
	}
	else
	{
		// 不是反击者则加杀戳值, 和平或善恶模式不加杀戳和不减幸运
		JUDGE_RETURN(attacker->is_attack_by_id(this->fighter_id()) == false, -1);
		JUDGE_RETURN(attacker->is_same_pk_state(PK_DITHESISM) == false && attacker->is_same_pk_state(PK_PEACE) == false, -1);

		int kill_value = GameCommon::fetch_kill_value(name_color);
		JUDGE_RETURN(kill_value > 0, -1);

		attacker->killed_info_.kill_num_ = 0;
		attacker->inc_kill_value(this, kill_value);
	}

	if (attack_color != attacker->fetch_name_color())
	{
		attacker->remove_last_color_info(attack_color);
		attacker->nofity_name_color_change();
	}

	MapPlayer* player = dynamic_cast<MapPlayer*>(this);
	player->cache_tick().update_cache(MapPlayerEx::CACHE_SYNC_LOGIC_FORCE, true);

	return 0;
}

int MapKiller::inc_kill_value(MapKiller *die_player, const int value)
{
	this->notify_tips_info(GameEnum::FORMAT_MSG_ADD_KILLING_VALUE, die_player->name(), value);

	this->killed_info_.kill_num_ = 0;
	this->killed_info_.killing_value_ += value;

	if (value > 0 && (this->fight_detail().__pk_state == PK_PEACE || this->fight_detail().__pk_state == PK_DITHESISM))
	{
		MapPlayer* player = dynamic_cast<MapPlayer*>(this);
		player->record_other_serial(KILLING_SERIAL, SUB_KILL_ADD, this->killed_info_.killing_value_,
				this->fight_detail().__pk_state, this->scene_id());
	}

	this->killed_info_.attackor_map_.erase(die_player->fighter_id());
	die_player->killed_info().attackor_map_.erase(this->fighter_id());

	this->killed_info_.online_ticks_ = Time_Value::gettimeofday().sec() + //
			CONFIG_INSTANCE->tiny("reduce_time").asInt() * 60;

	this->check_and_brocast_ditheism();
	return 0;
}

int MapKiller::handle_reduce_kill_value()
{
//	JUDGE_RETURN(this->killed_info_.killing_value_ > 0, 0);
//	JUDGE_RETURN(this->is_in_normal_mode() == true, 0);
//
//	Time_Value nowtime = Time_Value::gettimeofday();
//	JUDGE_RETURN(this->killed_info_.online_ticks_ <= nowtime.sec(), -1);
//
//	int reduce_kill = CONFIG_INSTANCE->tiny("reduce_kill").asInt();
//	int reduce_time = CONFIG_INSTANCE->tiny("reduce_time").asInt() * 60;
//
//	this->modify_kill_value(reduce_kill);
//	this->killed_info_.online_ticks_ = nowtime.sec() + reduce_time;

	return 0;
}

int MapKiller::validate_kill_value_scene()
{
	return true;
}

int MapKiller::modify_kill_value(int value)
{
	int last_color = this->fetch_name_color();

	this->killed_info_.killing_value_ += value;
	this->killed_info_.killing_value_ = std::max<int>(0,
			this->killed_info_.killing_value_);

	int cur_color = this->fetch_name_color();
	JUDGE_RETURN(last_color != cur_color, -1);

	this->remove_last_color_info(last_color);
	this->nofity_name_color_change();

	this->update_fight_property(GameEnum::UT_PROP_KILL_VALUE);
	return 0;
}

int MapKiller::remove_last_color_info(int last_color)
{
	const Json::Value &color_conf = CONFIG_INSTANCE->killing(last_color);

	if (color_conf.isMember("label_id") == true)
	{
		MapPlayer* player = dynamic_cast<MapPlayer*>(this);
		player->request_map_logic_add_label(color_conf["label_id"].asInt(), true);
	}

	return 0;
}

int MapKiller::nofity_name_color_change(int flag)
{
//	JUDGE_RETURN(flag == true, -1);
//
//	int cur_color = this->fetch_name_color();
//	const Json::Value &color_conf = CONFIG_INSTANCE->killing(cur_color);
//
//	this->notify_update_player_info(GameEnum::PLAYER_INFO_COLOR, cur_color);
//
//	if (color_conf.isMember("brocast_id") == true)
//	{
//		BrocastParaVec para_vec;
//		GameCommon::push_brocast_para_string(para_vec, this->name());
//		MAP_MONITOR->announce_world(SHOUT_ALL_KILL_HERO, para_vec);
//	}
//
//	if (cur_color != GameEnum::NAME_RED)
//	{
//		this->killed_info_.is_brocast_ = false;
//	}
//
//	if (color_conf.isMember("label_id") == true)
//	{
//		MapPlayer* player = dynamic_cast<MapPlayer*>(this);
//		player->request_map_logic_add_label(color_conf["label_id"].asInt());
//	}

	return 0;
}

int MapKiller::check_and_brocast_ditheism()
{
//	static int kill_brocast = CONFIG_INSTANCE->tiny("kill_brocast").asInt();
//	JUDGE_RETURN(this->killed_info_.killing_value_ > kill_brocast, -1);
//	JUDGE_RETURN(this->killed_info_.is_brocast_ == false, -1);
//
//	BrocastParaVec para_vec;
//	GameCommon::push_brocast_para_string(para_vec, this->name());
//	MAP_MONITOR->announce_world(SHOUT_ALL_KILL_CRAZY, para_vec);
//
//	this->killed_info_.is_brocast_ = true;
	return 0;
}

int MapKiller::fetch_name_color()
{
	int will_color = GameCommon::fetch_name_color(
			this->killed_info_.kill_num_,
			this->killed_info_.killing_value_);
	return will_color;
}

bool MapKiller::is_attack_by_id(const Int64 role_id)
{
	LongMap::iterator iter = this->killed_info_.attackor_map_.find(role_id);
	if (iter == this->killed_info_.attackor_map_.end())
	{
		return false;
	}

	if (iter->second + Time_Value::MINUTE <= ::time(NULL))
	{
		return false;
	}

	return true;
}

KilledInfo& MapKiller::killed_info()
{
	return this->killed_info_;
}

int MapKiller::sync_transfer_killer(void)
{
    Proto30400110 request;
    request.set_is_yellow(this->killed_info_.is_yellow_);
    request.set_kill_num(this->killed_info_.kill_num_);
    request.set_is_brocast(this->killed_info_.is_brocast_);
    request.set_online_ticks(this->killed_info_.online_ticks_);
    request.set_killing_value(this->killed_info_.killing_value_);

    for (BIntMap::iterator iter = this->monster_map_.begin();
            iter != this->monster_map_.end(); ++iter)
    {
        ProtoMonster *proto_monster = request.add_monster_set();
        proto_monster->set_sort(iter->first);
        proto_monster->set_total_amount(iter->second);
    }

    return this->send_to_other_scene(this->scene_id(), request);
}

int MapKiller::read_transfer_killer(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400110 *, request, -1);

    KilledInfo& kill_info = this->killed_info_;
    kill_info.is_yellow_	= request->is_yellow();
    kill_info.kill_num_  	= request->kill_num();
    kill_info.is_brocast_	= request->is_brocast();
    kill_info.online_ticks_	= request->online_ticks();
    kill_info.killing_value_= request->killing_value();

    for (int i = 0; i < request->monster_set_size(); ++i)
    {
        const ProtoMonster &proto_monster = request->monster_set(i);
        this->monster_map_[proto_monster.sort()] = proto_monster.total_amount();
    }

    return 0;
}

