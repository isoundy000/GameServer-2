/*
 * GameFighter.cpp
 *
 * Created on: 2013-03-26 15:42
 *     Author: lyz
 */

#include "GameFighter.h"
#include "MapPlayerEx.h"
#include "MapBeast.h"
#include "MapMonitor.h"
#include "FightField.h"
#include "ProtoDefine.h"
#include "AIManager.h"
#include "WorldBossScene.h"

int GameFighter::FighterTimer::type()
{
	return GTT_MAP_PLAYER;
}

int GameFighter::FighterTimer::handle_timeout(const Time_Value &tv)
{
	return this->fighter_->time_up(tv);
}

GameFighter::GameFighter(void)
{
    this->magic_recovert_tick_ = 0;
    this->blood_recovert_tick_ = 0;

    this->loop_index_ = 0;
    this->fight_times_ = 0;
    this->fighter_timer_.fighter_ = this;
}

GameFighter::~GameFighter(void)
{ /*NULL*/ }

int GameFighter::time_up(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->is_active() == true, -1);
	JUDGE_RETURN((this->is_death() == false) || (this->is_monster() == true), -1);

    this->process_launch_skill(nowtime);
    this->process_launch_loop_skill(nowtime);
	this->process_launch_mutual_skill(nowtime);
    JUDGE_RETURN(this->is_death() == false, -1);

    this->refresh_status(nowtime);
    this->refresh_fight_state(nowtime);
    this->refresh_blood_recover(nowtime);
    this->auto_modify_around_blood(nowtime);

    return 0;
}

void GameFighter::reset_fighter(void)
{
    this->reset_mover();
    this->reset_status();

    this->recycle_all_skill_map();
    this->fight_detail_.reset();
    this->fight_skill_.reset();
    this->prepare_tick_map_.unbind_all();

    this->fight_frequency_tick_ = Time_Value::zero;
    this->magic_recovert_tick_ = 0;
    this->blood_recovert_tick_ = 0;

    while (this->skill_delay_launch_queue_.size() > 0)
    {
        DelaySkillInfo *info = this->skill_delay_launch_queue_.pop();
        JUDGE_CONTINUE(info != NULL);
        this->monitor()->delay_skill_pool()->push(info);
    }

    this->loop_index_ = 0;
    for (int i = 0; i < TOTAL_LOOP; ++i)
    {
        this->loop_skill_[i].reset();
    }

    this->mutual_skill_.reset();
    this->frozen_skills_.clear();
    this->skill_freeze_map_.clear();

    this->fight_times_ = 0;
	this->fighter_timer_.cancel_timer();
}

int GameFighter::exit_scene(const int type)
{
    return GameMover::exit_scene(type);
}

int GameFighter::fetch_attack_distance()
{
	static int default_skill_distance = CONFIG_INSTANCE->const_set("near_attack_dis");
	return default_skill_distance;
}

void GameFighter::set_cur_toward(const MoverCoord& coord)
{
	GameMover::set_cur_toward(coord);
}

bool GameFighter::is_blood_full()
{
	return this->fight_detail_.__blood >= this->fight_detail_.__blood_total_i(this);
}

bool GameFighter::is_magic_full()
{
	return this->fight_detail_.__magic >= this->fight_detail_.__magic_total_i(this);
}

int GameFighter::cur_blood()
{
	return this->fight_detail_.cur_blood();
}

int GameFighter::total_recover_blood()
{
	return this->fight_detail_.__recover_blood + this->fetch_max_blood(
			this->fight_detail_.__recover_blood_per);
}

int GameFighter::fetch_check_distance()
{
	JUDGE_RETURN(this->fight_skill_.need_check_distance() == true, -1);
	return this->fight_skill_.__skill->__distance;
}

int GameFighter::fetch_blood_differ()
{
	return this->fight_detail_.__blood_total_i(this) - this->fight_detail_.__blood;
}

double GameFighter::cur_blood_percent(int percent)
{
	int total_blood = this->fight_detail_.__blood_total_i(this);
	JUDGE_RETURN(total_blood  > 0, 0);
	return this->fight_detail_.__blood * 1.0 / total_blood * percent;
}

double GameFighter::fetch_max_blood(double percent)
{
	return GameCommon::div_percent(this->fight_detail_.__blood_total(this) * percent);
}

double GameFighter::fetch_cur_blood(double percent)
{
	return GameCommon::div_percent(this->fight_detail_.__blood * percent);
}

void GameFighter::fighter_restore_all(int fight_tips, int restore_type, double percent)
{
	if (restore_type == 0)
	{
		this->modify_blood_by_levelup(this->fight_detail_.__blood_total_i(this) * percent, fight_tips);
    	this->modify_magic_by_notify(-this->fight_detail_.__magic_total_i(this) * percent, fight_tips);
	}
	else if (restore_type == FIGHT_UPDATE_BLOOD)
	{
		this->modify_blood_by_levelup(this->fight_detail_.__blood_total_i(this) * percent, fight_tips);
	}
	else
	{
		this->modify_magic_by_notify(-this->fight_detail_.__magic_total_i(this) * percent, fight_tips);
	}
}

void GameFighter::fighter_check_and_restore_all()
{
	JUDGE_RETURN(this->is_blood_full() == false, ;);

	if (this->is_death() == true)
	{
		this->fighter_restore_all(FIGHT_TIPS_RELIVE);
	}
	else
	{
		this->fighter_restore_all(FIGHT_TIPS_SYSTEM_AUTO);
	}
}

void GameFighter::shrink_cur_blood()
{
	int total_blood = this->fight_detail_.__blood_total_i(this);
	JUDGE_RETURN(this->fight_detail_.__blood > total_blood, ;);

	this->fighter_restore_all(FIGHT_TIPS_NOTHING, FIGHT_UPDATE_BLOOD);
}

void GameFighter::init_klv_property()
{
    const Json::Value &klv_json = CONFIG_INSTANCE->role_level(0, this->level());
    JUDGE_RETURN(klv_json.empty() == false, ;);

	FightDetail &fight_detail = this->fight_detail();
	fight_detail.hit_klv_ = klv_json[GameName::HIT_KLV].asInt();
	fight_detail.avoid_klv_ = klv_json[GameName::AVOID_KLV].asInt();
	fight_detail.crit_klv_ = klv_json[GameName::CRIT_KLV].asInt();
	fight_detail.toughness_klv_ = klv_json[GameName::TOUGHNESS_KLV].asInt();
}

FightDetail &GameFighter::fight_detail(void)
{
    return this->fight_detail_;
}

CurrentSkill &GameFighter::fight_cur_skill(void)
{
    return this->fight_skill_;
}

int64_t GameFighter::fighter_id(void)
{
    return this->mover_id();
}

int GameFighter::fighter_id_low(void)
{
    return this->mover_id_low();
}

int GameFighter::fighter_id_high(void)
{
    return this->mover_id_high();
}

int GameFighter::fight_career(void)
{
	return 0;
}

int GameFighter::fight_sex(void)
{
	return 0;
}

int GameFighter::fetch_name_color()
{
	return GameEnum::NAME_WHITE;
}

bool GameFighter::is_attack_by_id(Int64 role_id)
{
	return false;
}

int GameFighter::fighter_sort()
{
	return 0;
}

int GameFighter::is_lrf_change_mode()
{
	return false;
}

void GameFighter::set_camp_id(const int camp)
{
    this->fight_detail_.__camp_id = camp;
}

int GameFighter::camp_id(void)
{
    return this->fight_detail_.__camp_id;
}

int GameFighter::level(void)
{
    return this->fight_detail_.__level;
}

Int64 GameFighter::league_id(void)
{
	return 0;
}

int GameFighter::skill_id_for_step(void)
{
	return this->fight_detail_.__skill_id_for_step;
}

int GameFighter::skill_step(void)
{
	return this->fight_detail_.__skill_step;
}

Time_Value &GameFighter::skill_step_tick(void)
{
	return this->fight_detail_.__skill_step_tick;
}

void GameFighter::refresh_skill_step(int skill_step)
{
	FighterSkill* skill = this->fight_skill_.__skill;
	int cur_skill = this->fight_skill_.__skill_id;

	//随机步骤
	if (skill->__rand_step > 0)
	{
		this->fight_detail_.__skill_id_for_step = cur_skill;
		this->fight_detail_.__skill_step = skill_step;
		return;
	}

	//顺序步骤
	if (this->skill_id_for_step() != cur_skill)
	{
		JUDGE_RETURN(skill->__max_step > 0, ;);

		this->fight_detail_.__skill_id_for_step = cur_skill;
		this->fight_detail_.__skill_step = 0;
	}

//	if ((skill_step <= this->skill_step() + 1) && skill_step > 0)
	if (skill_step  > 0)
	{
		this->fight_skill_.__skill_step = skill_step;
	}
	else
	{
		this->fight_skill_.__skill_step = this->skill_step() + 1;
	}
}

bool GameFighter::is_vip()
{
	return false;
}

int GameFighter::vip_type()
{
	return VIP_NOT_VIP;
}

int GameFighter::send_to_logic_thread(int recogn)
{
	return 0;
}

int GameFighter::send_to_logic_thread(Message& msg)
{
	return 0;
}

int GameFighter::send_to_other_scene(const int scene_id, Message& msg)
{
	return 0;
}

int GameFighter::find_fighter(Int64 fighter_id, GameFighter *&fighter)
{
	Scene *scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	return scene->find_fighter(fighter_id, fighter);
}

MapPlayerEx* GameFighter::find_player(Int64 fighter_id)
{
	GameFighter* fighter = NULL;
	this->find_fighter(fighter_id, fighter);
	JUDGE_RETURN(fighter != NULL, NULL);

	if (fighter->validate_online_player() == true)
	{
		return dynamic_cast<MapPlayerEx*>(fighter);
	}
	else if (fighter->is_beast() == true)
	{
		return dynamic_cast<MapBeast*>(fighter)->fetch_master();
	}

	return NULL;
}

MapPlayerEx* GameFighter::find_player_with_offline(Int64 fighter_id)
{
	GameFighter* fighter = NULL;
	JUDGE_RETURN(this->find_fighter(fighter_id, fighter) == 0, NULL);

	if (fighter->is_player() == true)
	{
		return fighter->self_player();
	}
	else if (fighter->is_beast() == true)
	{
		return dynamic_cast<MapBeast*>(fighter)->fetch_master();
	}

	return NULL;
}

MapPlayerEx* GameFighter::fetch_benefited_player(Int64 attackor_id)
{
	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, NULL);
	return scene->fetch_benefited_player(attackor_id);
}

GameAI* GameFighter::self_ai()
{
	return dynamic_cast<GameAI*>(this);
}

MapPlayerEx* GameFighter::self_player()
{
	JUDGE_RETURN(this->is_player() == true, NULL);
	return dynamic_cast<MapPlayerEx*>(this);
}

MapPlayerEx* GameFighter::dynamic_player()
{
	return dynamic_cast<MapPlayerEx*>(this);
}

void GameFighter::update_pk_tick(void)
{
    double pk_tick = CONFIG_INSTANCE->pk_tick();
    this->fight_detail_.__pk_tick = GameCommon::fetch_add_time_value(pk_tick);
}

int GameFighter::set_pk_state(const int state)
{
	JUDGE_RETURN(this->check_pk_state(state) == true, -1);
    this->fight_detail_.__pk_state = state;
    return 0;
}

bool GameFighter::check_pk_state(const int state)
{
	return (PK_STATE_NONE < state && state < PK_STATE_END);
}

bool GameFighter::is_pk_state(void)
{
	return this->is_same_pk_state(PK_PEACE) == false;
}

bool GameFighter::is_plenary_pk(void)
{
	return this->is_same_pk_state(PK_PLENARY);
}

bool GameFighter::is_team_pk(void)
{
	return this->is_same_pk_state(PK_TEAM);
}

bool GameFighter::is_league_pk(void)
{
	return this->is_same_pk_state(PK_LEAGUE);
}

bool GameFighter::is_same_pk_state(int state)
{
	return this->fight_detail_.__pk_state == state;
}

bool GameFighter::is_fight_state(void)
{
    return (this->fight_detail_.__fight_tick > Time_Value::gettimeofday());
}

bool GameFighter::is_fight_active(void)
{
	return (this->is_fight_state() && this->fight_detail_.__fight_state == GameEnum::FIGHT_STATE_ACTIVE);
}

bool GameFighter::is_fight_passive(void)
{
	return (this->is_fight_state() && this->fight_detail_.__fight_state == GameEnum::FIGHT_STATE_PASSIVE);
}

bool GameFighter::is_gather_state(void)
{
	return (this->fight_detail_.gather_state_ == 1);
}

int GameFighter::gather_state_end()
{
	return 0;
}

void GameFighter::update_fight_state(const Int64 fighter_id, const int state, const Time_Value &last_tick)
{
	bool is_notify = false;
	Time_Value nowtime = Time_Value::gettimeofday();

	if (this->fight_detail_.__fight_tick < nowtime
			|| (state == GameEnum::FIGHT_STATE_ACTIVE && this->is_fight_passive() == true))
	{
		is_notify = true;
	}

	// 主动可以覆盖被动，被动不能覆盖主动
	if (this->fight_detail_.__fight_state != GameEnum::FIGHT_STATE_ACTIVE)
	{
		this->fight_detail_.__fight_state = state;
	}

    this->fight_detail_.__fight_tick = nowtime + last_tick;
    if (last_tick > Time_Value::zero && is_notify == true)
    {
    	this->notify_fight_state(fighter_id);
    }

    if (state != GameEnum::FIGHT_STATE_NO && this->is_gather_state())
    {
        this->gather_state_end();
    }
}

void GameFighter::update_active_fight_state(const Int64 target_id)
{
}

bool GameFighter::is_death(void)
{
    return this->fight_detail_.__blood <= 0;
}

bool GameFighter::is_jumping(void)
{
	return this->is_have_status(BasicStatus::JUMPING);
}

bool GameFighter::is_skill_frozen(int skill_id)
{
	if (this->frozen_skills_.find(skill_id) != this->frozen_skills_.end())
	{
		MSG_DEBUG("skill %d in frozen_skills", skill_id);
		return true;
	}

	return false;
}

int GameFighter::fetch_fight_state()
{
	if (this->is_fight_state() == true)
	{
		return this->fight_detail_.__fight_state;
	}

	return GameEnum::FIGHT_STATE_NO;
}

int GameFighter::modify_blood_by_fight(const double inc_org_val, const int org_fight_tips, const int64_t attackor, const int skill_id)
{
    JUDGE_RETURN(this->is_death() == false, ERROR_PLAYER_DEATH);

    int fight_tips = org_fight_tips;
    int inc_val = inc_org_val;
    switch (fight_tips)
    {
    case FIGHT_TIPS_NOTHING:
    case FIGHT_TIPS_RELIVE:
    case FIGHT_TIPS_SYSTEM_AUTO:
    case FIGHT_TIPS_USE_PROPS:
    {
    	break;
    }

    default:
    {
    	this->update_fight_state(attackor, GameEnum::FIGHT_STATE_PASSIVE);
    	break;
    }
    }

    BasicStatus *status = 0;
    if (inc_val > 0 && this->find_first_status(BasicStatus::SHIELD, status) == 0)
    {
		int reduce_value = this->calc_shield_reduce_value(inc_val, status);
		return this->notify_fight_update(FIGHT_UPDATE_BLOOD, reduce_value, attackor, skill_id,
					this->fight_detail_.__blood, FIGHT_TIPS_ABSORB, skill_id);
    }

    if (inc_val > 0 && this->find_first_status(BasicStatus::ROLE_SHIELD, status) == 0)
    {
		int reduce_value = this->calc_shield_reduce_value_b(inc_val, status);
		return this->notify_fight_update(FIGHT_UPDATE_BLOOD, reduce_value, attackor, skill_id,
					this->fight_detail_.__blood, FIGHT_TIPS_ABSORB, skill_id);
    }

    if (inc_val > 0 && this->find_first_status(BasicStatus::FIX_SHIELD, status) == 0)
    {
		int reduce_value = this->calc_shield_reduce_value_c(inc_val, status, attackor);
		return this->notify_fight_update(FIGHT_UPDATE_BLOOD, reduce_value, attackor, skill_id,
					this->fight_detail_.__blood, FIGHT_TIPS_ABSORB, skill_id);
    }

    int real_value = this->modify_blood(inc_val, attackor);

    if (inc_val != 0)
    {
    	this->notify_fight_update(FIGHT_UPDATE_BLOOD, inc_val, attackor,
    			0, this->fight_detail_.__blood, fight_tips, skill_id);
    }

    return real_value;
}

int GameFighter::modify_blood_by_relive(const double inc_val)
{
    JUDGE_RETURN(this->is_death() == true, -1);

    int real_value = this->modify_blood(inc_val, this->fighter_id());
    return this->notify_fight_update(FIGHT_UPDATE_BLOOD, real_value,
    		this->fighter_id(), 0, this->fight_detail_.__blood, FIGHT_TIPS_RELIVE);
}

int GameFighter::modify_blood_by_levelup(const double target_val, const int fight_tips)
{
	int inc_val = this->fight_detail_.__blood - target_val;

    int real_value = this->modify_blood(inc_val, 0);
    JUDGE_RETURN(real_value != 0, 0);

    return this->notify_fight_update(FIGHT_UPDATE_BLOOD, real_value, 0,
    		0, this->fight_detail_.__blood, fight_tips);
}

int GameFighter::modify_magic_by_notify(const double value, const int fight_tips)
{
	JUDGE_RETURN(this->is_player() == true, 0);

    int real_magic = this->modify_magic(value);
    if (std::abs(real_magic) > 0)
    {
        this->notify_fight_update(FIGHT_UPDATE_MAGIC, real_magic, 0, 0,
        		this->fight_detail_.__magic, fight_tips);
    }

    return real_magic;
}

void GameFighter::trigger_left_blood_skill(Int64 attackor)
{
}

int GameFighter::modify_magic(const int inc_val)
{
    int prev_magic = this->fight_detail_.__magic;
    this->fight_detail_.__magic -= inc_val;
    if (this->fight_detail_.__magic < 0)
        this->fight_detail_.__magic = 0;
    if (this->fight_detail_.__magic > this->fight_detail_.__magic_total_i(this))
        this->fight_detail_.__magic = this->fight_detail_.__magic_total_i(this);
    return (prev_magic - this->fight_detail_.__magic);
}

int GameFighter::modify_blood(const int org_inc_val, const int64_t attackor)
{
	// 宠物不可攻击，不会死亡
	JUDGE_RETURN(this->is_beast() == false, 0);

	int inc_val = org_inc_val;
    int prev_blood = this->fight_detail_.__blood;

    if (inc_val < 0)
    {
    	if ((INT_MAX - this->fight_detail_.__blood) < std::abs(inc_val))
    		inc_val = -(INT_MAX - this->fight_detail_.__blood);
    }

    this->fight_detail_.__blood -= inc_val;
    if (this->fight_detail_.__blood < 0)
    {
        this->fight_detail_.__blood = 0;
    }

    int total_blood = this->fight_detail_.__blood_total_i(this);
    if (this->fight_detail_.__blood > total_blood)
    {
        this->fight_detail_.__blood = total_blood;
    }

    int inc_blood = (prev_blood - this->fight_detail_.__blood);
    if (this->is_death() == true && prev_blood > 0)
    {
        this->fight_detail_.__death_tick = ::time(NULL);
        this->die_process(attackor);
    }
    else
    {
    	this->trigger_left_blood_skill(attackor);
    }

    return inc_blood;
}

int GameFighter::insert_passive_skill_queue(FighterSkill *skill)
{
    const Json::Value &skill_json = skill->conf();

    if (skill_json.isMember("when_hurt") == true)
    {
        PassiveSkillInfo *pass_info = MAP_MONITOR->passive_skill_qn_pool()->pop();
        pass_info->__skill = skill;
        this->fight_detail_.__hurt_passive_skill_queue.push(pass_info);
    }
    else
    {
        PassiveSkillInfo *pass_info = MAP_MONITOR->passive_skill_qn_pool()->pop();
        pass_info->__skill = skill;
        pass_info->__launch_tick.sec(this->fight_detail_.__passive_skill_use_time[skill->__skill_id]);
        this->fight_detail_.__fight_passive_skill_queue.push(pass_info);
    }

    return 0;
}

int GameFighter::insert_skill(int skill_id, int level, int notify)
{
	FighterSkill* skill = NULL;
	if (this->find_skill(skill_id, skill) == 0)
	{
		return this->update_skill_level(skill, level);
	}
	else
	{
		return this->insert_skill_i(skill_id, level);
	}
}

int GameFighter::insert_skill_i(int skill_id, int level)
{
	const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
	if (skill_json.empty() == true)
	{
		MSG_USER("Error Skill ID %d", skill_id);
		return -1;
	}

	FighterSkill* skill = this->monitor()->pop_skill(skill_id, std::max<int>(level, 1));
	this->fight_detail_.__skill_map[skill->__skill_id] = skill;

    if (skill->__passive_trigger == 1)
    {
        this->insert_passive_skill_queue(skill);
    }

    Time_Value use_tick = this->monitor()->fetch_skill_cool(this->fighter_id(), skill_id);
    if (use_tick != Time_Value::zero)
    {
    	skill->__use_tick = use_tick;
    }

//    if (skill_json["trigger_when_insert"].asInt() == 1)
//    {
//    	this->process_launch_rate_use_skill(skill);
//    }

	return 0;
}

int GameFighter::remove_skill(int skill_id, int record)
{
    FighterSkill *skill = NULL;
    JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, -1);

    if (record == true)
    {
    	this->monitor()->add_skill_cool(this, skill);
    }

    this->remove_passive_skill(skill);
    this->remove_skill_buff(skill);
    this->fight_detail_.__skill_map.erase(skill_id);
    return this->monitor()->skill_pool()->push(skill);
}

int GameFighter::remove_passive_skill(FighterSkill* skill)
{
    JUDGE_RETURN(skill->__passive_trigger == 1, -1);

    int total = this->fight_detail_.__fight_passive_skill_queue.size();
	for (int i = 0; i < total; ++i)
	{
		PassiveSkillInfo *info = this->fight_detail_.__fight_passive_skill_queue.node(i);
		JUDGE_CONTINUE(info != NULL && info->__skill == skill);

		this->fight_detail_.__passive_skill_use_time[skill->__skill_id] = skill->__use_tick.sec();
		this->fight_detail_.__fight_passive_skill_queue.remove(info);
		break;
	}

	total = this->fight_detail_.__hurt_passive_skill_queue.size();
	for (int i = 0; i < total; ++i)
	{
		PassiveSkillInfo *info = this->fight_detail_.__hurt_passive_skill_queue.node(i);
		JUDGE_CONTINUE(info != NULL && info->__skill == skill);

		this->fight_detail_.__hurt_passive_skill_queue.remove(info);
		break;
	}

	return 0;
}

int GameFighter::remove_skill_buff(FighterSkill* skill)
{
	JUDGE_RETURN(skill->__del_buff == 1, -1);

	const Json::Value& detial_json = skill->detail();
	JUDGE_RETURN(detial_json.empty() == false, -1);

	if (detial_json.isMember("effect") == true)
	{
		for (uint i = 0; i < detial_json["effect"].size(); ++i)
		{
			const Json::Value &effect = detial_json["effect"][i];
			const std::string effect_name = effect["name"].asString();
			if (effect_name == Effect::BUFF)
			{
				this->remove_status(effect["id"].asInt());
			}
		}
	}

	return 0;
}

int GameFighter::is_have_skill(int skill_id)
{
	return this->fight_detail_.__skill_map.count(skill_id) > 0;
}

int GameFighter::find_skill(int skill_id, FighterSkill *&skill)
{
	if (this->fight_detail_.__skill_map.count(skill_id) > 0)
	{
		skill = this->fight_detail_.__skill_map[skill_id];
		return 0;
	}
	else
	{
		skill = NULL;
		return -1;
	}
}

int GameFighter::prepare_fight_skill(Message *msg)
{
	Time_Value nowtime = Time_Value::gettimeofday();
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400201*, request, ERROR_CLIENT_OPERATE);

    int owner_skill_id = request->skill_id();
	JUDGE_RETURN(this->is_skill_frozen(owner_skill_id) == false, 0);

	Scene* scene = this->fetch_scene();
	CONDITION_NOTIFY_RETURN(scene != NULL && scene->is_fighting_time() == true,
			RETURN_PREPARE_FIGHT, ERROR_ACTIVITY_TIME_INVALID);

    const Json::Value &skill_json = CONFIG_INSTANCE->skill(owner_skill_id);
    CONDITION_NOTIFY_RETURN(skill_json.empty() == false, RETURN_PREPARE_FIGHT,
    		ERROR_CONFIG_NOT_EXIST);

    FighterSkill* skill = 0;
	CONDITION_NOTIFY_RETURN(this->find_skill(owner_skill_id, skill) == 0,
			RETURN_PREPARE_FIGHT, ERROR_SKILL_NOT_EXISTS);

	//判断变身
	int ret = this->validate_transfer_skill(skill);
	JUDGE_RETURN(ret != 1, 0);

	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PREPARE_FIGHT, ret);

    this->fight_skill_.reset();
    this->fight_detail_.__history_defender_set.clear();

    this->fight_skill_.set_skill_id(this, request->skill_id(), skill->__level);
    this->fight_skill_.__client_target = request->target_id();

    int pixel_x = request->skill_pixel_x();
    int pixel_y = request->skill_pixel_y();
    this->fight_skill_.__skill_coord.set_pixel(pixel_x, pixel_y);
    this->fight_skill_.__play_coord.set_pixel(pixel_x, pixel_y);

	FighterSkill* real_skill = this->fight_skill_.__skill;
	CONDITION_NOTIFY_RETURN(real_skill != NULL,	RETURN_PREPARE_FIGHT,
			ERROR_SKILL_NOT_EXISTS);

	//更新多步骤技能
    this->refresh_skill_step(request->skill_step());

    //设置客户端播放技能
    this->set_current_skill_display(skill_json);
    this->fight_skill_.__is_full_screen = real_skill->__full_screen;

#ifdef TEST_COMMAND
    if (this->is_player())
    {
		MSG_USER("prepare fight client %ld(%d,%d) %d angle(%lf) skill_coord(%d,%d) %d (%d) step(%d, %d)", this->fighter_id(),
				this->location().pixel_x(), this->location().pixel_y(),
				request->skill_id(), this->fight_skill_.__angle, request->skill_pixel_x(), request->skill_pixel_y(),
				request->target_list_size(), coord_offset_grid(this->location(), this->fight_skill_.__skill_coord),
				request->skill_step(), this->fight_skill_.__skill_step);
    }
#endif

    //校正服务端和客户端细微位置不一致
    this->adjust_player_coord(request);
    // 修正客户端发过来的技能点坐标
    this->adjust_current_skill_coord(real_skill->__aoe_type, request);
    // 设置技能弧度
    this->set_current_skill_radian(request->angle());

    // 不发攻击目标时自己搜索可攻击目标, 可能比较耗时, 尽量少用
    this->make_up_skill_target(real_skill, this->fight_skill_.__skill_coord, request);

#ifdef TEST_COMMAND
    if (this->is_player())
    {
    	MSG_USER("prepare fight %ld(%d,%d) %d (%d,%d) %d %d (%d)", this->fighter_id(),
				this->location().pixel_x(), this->location().pixel_y(),
				request->skill_id(), request->skill_pixel_x(), request->skill_pixel_y(),
				request->target_list_size(), request->skill_step(),
				coord_offset_grid(this->location(), this->fight_skill_.__skill_coord));
    }
#endif

    // 验证技能限制
    ret = this->validate_skill_usage();
    if (ret != 0)
    {
		MSG_USER("error prepare fight not validate %s %ld %d (%d,%d)->(%d,%d)",
				this->name(), this->mover_id(), ret,
				this->location().pixel_x(), this->location().pixel_y(),
				this->fight_skill_.__skill_coord.pixel_x(),
				this->fight_skill_.__skill_coord.pixel_y());

        CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PREPARE_FIGHT, ret);
    }

    std::auto_ptr<Proto10400202> launch_req(new Proto10400202);
    launch_req->set_skill_id(this->fight_skill_.__skill_id);
    launch_req->set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
    launch_req->set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());
    launch_req->set_target_id(request->target_id());
    launch_req->set_skill_step(this->fight_skill_.__skill_step);
    launch_req->set_angle(this->fight_skill_.__radian);
    this->fight_skill_.__play_coord.serialize(launch_req->mutable_play_coord());

    if (request->target_id() <= 0 && request->target_list_size() > 0)
    {
        request->set_target_id(request->target_list(0).target_id());
        launch_req->set_target_id(request->target_id());
    }

    int target_size = request->target_list_size();
    CONDITION_NOTIFY_RETURN(target_size <= 64, RETURN_PREPARE_FIGHT, ERROR_TARGET_TOO_MORE);

    // 查找并验证新的攻击对象
    bool is_all_death = true;
    bool is_all_target_monster = true;
	for (int i = 0; i < target_size; ++i)
	{
		const ProtoSkillTarget &target = request->target_list(i);

		Int64 target_id = target.target_id();
		if (GameCommon::fetch_mover_type(target_id) != MOVER_TYPE_MONSTER)
		{
			is_all_target_monster = false;
		}

		JUDGE_CONTINUE(this->fight_skill_.__client_target_set.count(target_id) == 0);

		this->fight_skill_.__client_target_set.insert(target_id);
        this->fight_skill_.__defender_id = target_id;
        GameFighter *defender = this->fetch_defender();
        JUDGE_CONTINUE(GameCommon::validate_fighter(defender) == true);

		is_all_death = false;
        ret = this->validate_prepare_attack_target();
        if (ret != 0)
        {
        	MoverCoord defender_coord = defender->location();

        	if (this->is_player())
        	{
        		MSG_USER("not validate target %ld %ld %d %d (%d,%d)->(%d,%d) step(%d, %d) angle(%f) %s",
        				this->fighter_id(), target_id, ret, this->fight_skill_.__skill_id, defender_coord.pixel_x(),
        				defender_coord.pixel_y(), this->location().pixel_x(), this->location().pixel_y(),
        				request->skill_step(), this->fight_skill_.__skill_step, this->fight_skill_.__angle, defender->name());
        	}
            continue;
        }

        launch_req->add_target_list(target_id);
        defender->check_enter_stiff(this->fight_skill_.__skill_id, this->fight_skill_.__skill_level);

        this->fight_skill_.__defender_set.insert(target_id);
        JUDGE_CONTINUE(target_id != this->fighter_id());

		if (this->fight_detail_.__last_defender_id <= 0)
		{
			this->fight_detail_.__last_defender_id = target_id;
		}

		if (real_skill->is_active_skill() == true)
		{
			defender->update_fight_state(this->fighter_id(), GameEnum::FIGHT_STATE_PASSIVE);
		}

		this->add_history_target_id(target_id);
    }

    // 更新消耗的蓝和技能冷却时间,战斗状态
    this->refresh_skill_info();
    // 进入主动战斗状态
    this->update_fight_state(this->fighter_id(), GameEnum::FIGHT_STATE_ACTIVE);

	//防止空打
    if (this->fight_skill_.__defender_set.size() <= 0 && real_skill->__no_object_limit != 1)
    {
    	return 0;
    }

    if (real_skill->__effect_ai_skill == 1)
    {
    	this->notify_launch_effect_ai_skill(skill_json);
    }
    else
    {
    	this->notify_launch_skill();
    }

    // 去除复活保护
    this->remove_die_protect();
    // 最近使用的技能
    this->fight_detail_.__last_skill = this->fight_skill_.__skill_id;

    this->process_monster_talk(this->fight_skill_.__skill_id, this->mover_id());
    this->process_skill_freeze(skill_json);

    const Json::Value &detail_json = this->cur_skill_detail_conf();
    this->process_skill_note(this->fight_skill_.__skill_id, detail_json);

    // 技能触发伤害效果
    this->process_skill_launch_way(skill, launch_req);

    this->monitor()->fight_total_use_ += (Time_Value::gettimeofday() - nowtime);
    return this->respond_to_client(RETURN_PREPARE_FIGHT);
}

int GameFighter::launch_fight_skill(Message *msg, const bool is_check_prepare)
{
	Time_Value nowtime = Time_Value::gettimeofday();
    MSG_DYNAMIC_CAST_RETURN(Proto10400202 *, request, ERROR_CLIENT_OPERATE);

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL && scene->is_fighting_time() == true, ERROR_ACTIVITY_TIME_INVALID);

    int skill_id = request->skill_id();
    this->fight_skill_.reset();
    this->fight_skill_.set_skill_id(this, skill_id);

    FighterSkill* real_skill = this->fight_skill_.__skill;
    JUDGE_RETURN(real_skill != NULL, ERROR_CONFIG_NOT_EXIST);

    this->fight_skill_.__skill_level = real_skill->__level;
    this->fight_skill_.__skill_step = request->skill_step();
    this->fight_skill_.__client_target = request->target_id();
    this->fight_skill_.__skill_coord.set_pixel(request->skill_pixel_x(), request->skill_pixel_y());
    this->fight_skill_.__play_coord.unserialize(request->mutable_play_coord());
    this->set_current_skill_radian(request->angle());

//    switch (real_skill->__aoe_type)
//    {
//    case GameEnum::SKILL_AOE_CUR_AIM_TARGET:
//    case GameEnum::SKILL_AOE_TARGET_CIRCLE:
//    {
//		GameFighter *target_fighter = 0;
//		JUDGE_BREAK(this->find_fighter(request->target_id(), target_fighter) == 0);
//
//		this->fight_skill_.__skill_coord.set_pixel(target_fighter->location().pixel_x(),
//				target_fighter->location().pixel_y());
//		request->set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
//		request->set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());
//    	break;
//    }
//    }

    if (this->is_player())
    {
    	MSG_DEBUG("launch fight %ld %d (%d,%d) angle(%f) %d %d %x (%d.%06d)", this->fighter_id(),
    		request->skill_id(), request->skill_pixel_x(), request->skill_pixel_y(), this->fight_skill_.__angle,
    		request->target_list_size(), request->skill_step(), msg, nowtime.sec(), nowtime.usec());
    }

	// check skill has prepared;
	if (is_check_prepare == true)
	{
		Time_Value prepare_tick;
		JUDGE_RETURN(this->prepare_tick_map_.find(skill_id,
				prepare_tick) == 0, ERROR_CLIENT_OPERATE);

		JUDGE_RETURN(prepare_tick <= nowtime, ERROR_CLIENT_OPERATE);
		this->prepare_tick_map_.unbind(skill_id);
	}

//    if (GameCommon::is_passive_skill(request->skill_id()) == false)
//    {
//        this->update_fight_state(this->fighter_id(), GameEnum::FIGHT_STATE_ACTIVE);
//    }

    int target_size = request->target_list_size();
    JUDGE_RETURN(target_size <= 64, ERROR_TARGET_TOO_MORE);

    // 先处理给自己给属性的效果
    uint max_target_size = std::max<uint>(real_skill->__object, 1);
    this->process_skill_effect_no_defender();

    // 查找新的攻击对象
    for (int i = 0; i < target_size; ++i)
    {
        Int64 target_id = request->target_list(i);
        JUDGE_CONTINUE(this->fight_skill_.__defender_set.count(target_id) == 0);
        
        this->fight_skill_.__defender_id = target_id;
        GameFighter *defender = this->fetch_defender();
        JUDGE_CONTINUE(defender != NULL);

        this->fight_skill_.__has_defender_buff = 0;
        this->fight_skill_.effect_flag_reset();
        int ret = this->validate_launch_attack_target();
        if (ret != 0)
        {
        	MoverCoord target_coord = defender->location();
            MSG_USER("target no attack %ld(%d,%d) %d %ld(%d,%d) %d", this->fighter_id(),
            		this->location().pixel_x(), this->location().pixel_y(), skill_id, target_id,
                    target_coord.pixel_x(), target_coord.pixel_y(), ret);
            continue;
        }

        // 触发战斗效果
        this->process_skill_effect_to_defender();
        defender->process_skill_post_effect(skill_id);

        this->add_history_target_id(target_id);
        this->fight_skill_.__defender_set.insert(target_id);

        if (real_skill->is_active_skill() == true)
        {
        	defender->update_fight_state(this->fighter_id(), GameEnum::FIGHT_STATE_PASSIVE);
        	defender->insert_attack_me(this->fetch_benefited_attackor_id(this->fighter_id()));
        }

        if (this->fight_skill_.__defender_set.size() >= max_target_size)
        {
        	break;
        }
    }

    if (this->fight_skill_.__skill_self_blood != 0)
    {
        this->modify_blood_by_fight(this->fight_skill_.__skill_self_blood,
        		FIGHT_TIPS_NORMAL, this->fighter_id(), skill_id);
    }

    this->set_client_target_id(request->target_id());
    this->monitor()->fight_total_use_ += (Time_Value::gettimeofday() - nowtime);
    return 0;
}

int GameFighter::direct_luanch_skill_effect(FighterSkill* skill, GameFighter* defender)
{
	JUDGE_RETURN(skill != NULL, -1);

	if (defender == NULL)
	{
		defender = this;
	}

	this->fight_skill_.__skill_id = skill->__skill_id;
	this->fight_skill_.__defender_id = defender->fighter_id();
	this->notify_launch_skill(skill, defender);

	const Json::Value& detail_conf = skill->detail();
	if (detail_conf.isMember("effect") == true)
	{
		for (uint i = 0; i < detail_conf["effect"].size(); ++i)
		{
			this->trigger_skill_effect_to_defender(detail_conf["effect"][i]);
		}
	}
	else
	{
		this->trigger_skill_effect_to_defender(detail_conf);
	}

	return 0;
}

void GameFighter::set_client_target_id(Int64 target_id)
{
	JUDGE_RETURN(target_id > 0, ;);

    this->fight_skill_.__defender_id = target_id;
    this->fight_skill_.effect_flag_reset();

    int ret = this->validate_launch_attack_target();
    JUDGE_RETURN(ret == 0, ;);

	this->update_active_fight_state(target_id);
}

void GameFighter::add_history_target_id(Int64 target_id)
{
	JUDGE_RETURN(this->is_player() == true, ;);
	JUDGE_RETURN(this->fighter_id() != target_id, ;);

	this->fight_detail_.__history_defender_set.insert(target_id);
}

void GameFighter::set_current_skill_radian(double radian)
{
    if (GameCommon::is_zero(radian) == false)
    {
        this->fight_skill_.set_radian(radian);
    }
    else if (this->fight_skill_.__skill_coord == this->location())
    {
        this->fight_skill_.set_angle(this->mover_detial_.__toward);
    }
    else
    {
        this->fight_skill_.set_radian(vector_to_radian(this->location(),
        		this->fight_skill_.__skill_coord));
    }
}

void GameFighter::use_skill_magic(const Json::Value &skill_json)
{
    int magic = GameCommon::json_by_level(skill_json["magic"],
    		this->fight_skill_.__skill_level).asInt();

    BasicStatus *status = NULL;
    if (this->find_status(BasicStatus::INFMAGIC, status) == 0)
    {
        double inc_magic = -magic * status->__value2 / 100.0 - status->__value1;
        magic += inc_magic;
    }

    JUDGE_RETURN(magic > 0, ;);
    this->modify_magic_by_notify(magic);
}

void GameFighter::set_start_jump(const Json::Value &effect)
{
	//插入跳跃BUFF
	const Json::Value& skill_json = CONFIG_INSTANCE->skill(this->fight_skill_.__skill_id);
	double jump_last = GameCommon::json_by_level(skill_json["last_time"],
			this->fight_skill_.__skill_level).asDouble();
	this->insert_defender_status(this, BasicStatus::JUMPING, 0, jump_last, 0,
			this->fight_skill_.__play_coord.pixel_x(),
			this->fight_skill_.__play_coord.pixel_y());
}

void GameFighter::set_finish_jump(BasicStatus* status, int is_timeout)
{
	JUDGE_RETURN(is_timeout == true, ;);
	JUDGE_RETURN(GameCommon::is_zero(status->__value3) == true, ;);

	this->fight_detail_.__jump_times = 0;

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, ;);

	MoverCoord mover_coord;
	mover_coord.set_pixel(status->__value1, status->__value2);
	scene->refresh_mover_location(this, mover_coord, false);
}

void GameFighter::set_current_skill_display(const Json::Value &skill_json)
{
    JUDGE_RETURN(skill_json.isMember("play") == true, ;);
	this->fight_skill_.__display_skill = skill_json["play"][0u].asInt();
}

void GameFighter::adjust_player_coord(Proto10400201* request)	//校正服务端和客户端位置不一致
{
	JUDGE_RETURN(this->is_player() == true, ;);
	JUDGE_RETURN(request->has_cur_coord() == true, ;);

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, ;);

	MoverCoord cur_coord;
	cur_coord.unserialize(request->mutable_cur_coord());

	MoverCoord& self_coord = this->location();
	JUDGE_RETURN(self_coord != cur_coord, ;);
	JUDGE_RETURN(::check_coord_distance(self_coord, cur_coord, 2) == true, ;);

	scene->refresh_mover_location(this, cur_coord, false);
}

void GameFighter::adjust_current_skill_coord(int aoeType, Proto10400201* request)
{
    JUDGE_RETURN(this->is_player() == true, ;);

    switch (aoeType)
    {
    case GameEnum::SKILL_AOE_CUR_AIM_TARGET:
    case GameEnum::SKILL_AOE_TARGET_CIRCLE:
    case GameEnum::SKILL_AOE_TARGET_RECT:
    {
    	Int64 target_id = request->target_id();
    	JUDGE_RETURN(target_id > 0, ;);

    	GameFighter *target_fighter = 0;
    	JUDGE_RETURN(this->find_fighter(target_id, target_fighter) == 0, ;);

    	this->fight_skill_.__skill_coord = target_fighter->location();
		request->set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
		request->set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());

    	break;
    }

    case GameEnum::SKILL_AOE_SELF_CIRCLE:
    case GameEnum::SKILL_AOE_SELF_TARGET:
    case GameEnum::SKILL_AOE_SELF_RING:
    case GameEnum::SKILL_AOE_SELF_RECT:
    {
		this->fight_skill_.__skill_coord = this->location();
		request->set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
		request->set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());
		break;
    }
    }
}

GameFighter* GameFighter::fetch_defender(void)
{
	GameFighter* defender = this->fight_skill_.__defender;
    if (defender != 0 && defender->is_active() == true
    		&& defender->fighter_id() == this->fight_skill_.__defender_id)
    {
        return this->fight_skill_.__defender;
    }

    Scene* scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, 0);

    if (scene->find_fighter(this->fight_skill_.__defender_id, defender) == 0)
    {
        this->fight_skill_.__defender = defender;
        return defender;
    }

    return 0;
}

GameFighter* GameFighter::fetch_self_owner(void)
{
	return NULL;
}

CurrentSkill &GameFighter::current_skill(void)
{
	return this->fight_skill_;
}

int GameFighter::validate_prepare_attack_target(void)
{
    int ret = this->validate_attack_target();
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

int GameFighter::validate_launch_attack_target(void)
{
    int ret = this->validate_attack_target();
    JUDGE_RETURN(ret == 0, ret);

    GameFighter *target = this->fetch_defender();
    JUDGE_RETURN(target != 0, ERROR_NO_ATTACK_TARGET);

    JUDGE_RETURN(target->is_have_status_type(BasicStatus::SUPPERMAN) == false,
    		ERROR_TARGET_SUPPERMAN);
    JUDGE_RETURN(target->is_have_status_type(BasicStatus::RELIVE_PROTECT) == false,
    		ERROR_TARGET_SUPPERMAN);

    return 0;
}

int GameFighter::validate_attack_target(void)
{
	JUDGE_RETURN(this->is_active() == true, ERROR_PLAYER_OFFLINE);	//宠物没有enter_scene，所以采用active

    GameFighter* target = this->fetch_defender();
    JUDGE_RETURN(GameCommon::validate_fighter(target) == true, ERROR_TARGET_NOT_FOUND);

    if (target->fighter_id() != this->fighter_id())
    {
        JUDGE_RETURN(target->is_have_status(BasicStatus::RELIVE_PROTECT) == false, ERROR_RELIVE_PROTECT);

        if (target->is_player() == true && (this->is_player() == true || this->is_beast() == true))
        {
        	JUDGE_RETURN(target->is_in_safe_area() == false && this->is_in_safe_area() == false, ERROR_IN_SAFE_AREA);
        }
    }

    int skill_id = this->fight_skill_.__skill_id;
    int skill_level = this->fight_skill_.__skill_level;

	FighterSkill* skill = 0;
	JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, ERROR_SKILL_NOT_EXISTS);

    const Json::Value &skill_json = skill->conf();
    JUDGE_RETURN(skill_json.empty() == false, ERROR_CONFIG_NOT_EXIST);

    switch (skill->__aoe_type)
    {
    case GameEnum::SKILL_AOE_SELF_CIRCLE:
    case GameEnum::SKILL_AOE_SELF_SECTOR:
    {
    	JUDGE_RETURN(Scene::is_validate_around_mover(target, this,
    	        skill->__radius) == true, ERROR_TARGET_EXTEND_RANGE);
    	break;
    }
    case GameEnum::SKILL_AOE_SELF_RECT:
    case GameEnum::SKILL_AOE_TARGET_RECT:
    {
    	MSG_DEBUG("validate rect calc %d angle(%f %f) skill_coord(%d,%d)", this->fight_skill_.__skill_id,
    			this->fight_skill_.__radian, this->fight_skill_.__angle,
    			this->fight_skill_.__skill_coord.pixel_x(), this->fight_skill_.__skill_coord.pixel_y());

        MoverCoord pointA, pointB, pointC, pointD;
        center_to_rect(pointA, pointB, pointC, pointD, this->fight_skill_.__skill_coord,
                this->fight_skill_.__radian, GameCommon::json_by_level(skill_json["width"], skill_level).asInt() * 30,
                GameCommon::json_by_level(skill_json["height"], skill_level).asInt() * 30);

        JUDGE_RETURN(Scene::is_validate_rect_mover(target, pointA, pointB, pointC, pointD) == true, ERROR_TARGET_EXTEND_RANGE);
        break;
    }
    case GameEnum::SKILL_AOE_SELF_TARGET:
    {
    	JUDGE_RETURN(target->fighter_id() == this->fighter_id(), ERROR_TARGET_NO_ATTACK);
    	return 0;
    }
    case GameEnum::SKILL_AOE_SELF_RING:
    {
		int outer_radius = std::max(GameCommon::json_by_level(skill_json["outer_radius"], skill_level).asInt(), 4);
		int inner_radius = std::max(GameCommon::json_by_level(skill_json["inner_radius"], skill_level).asInt(), 4);
		if (outer_radius <= inner_radius)
			outer_radius = inner_radius + 4;

    	JUDGE_RETURN(Scene::is_validate_around_mover(target, this->location(),
    			outer_radius, GameEnum::CLIENT_MAPPING_FACTOR) == true,
    			ERROR_TARGET_EXTEND_RANGE);

    	JUDGE_RETURN(check_coord_distance(target->location(), this->location(),
    			std::max(inner_radius - 2, 0),GameEnum::CLIENT_MAPPING_FACTOR)== false,
    			ERROR_TARGET_EXTEND_RANGE);
    	break;
    }
    case GameEnum::SKILL_AOE_CUR_AIM_TARGET:
    {
    	int distance = std::max<int>(this->fetch_attack_distance(), skill->__distance);
    	JUDGE_RETURN(Scene::is_validate_around_mover(target, this, distance) == true,
    			ERROR_TARGET_EXTEND_RANGE);
    	break;
    }
    default:
    {
    	JUDGE_RETURN(Scene::is_validate_around_mover(target, this,
    			skill->__distance) == true, ERROR_TARGET_EXTEND_RANGE);
    }
    }

    return GameCommon::validate_skill_target(skill, this, target);
}

int GameFighter::validate_attack_to_defender(GameFighter *defender, int skill_id)
{
	FighterSkill* skill = 0;
	JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, ERROR_SKILL_NOT_EXISTS);
	return GameCommon::validate_skill_target(skill, this, defender);
}

int GameFighter::validate_transfer_skill(FighterSkill* skill)
{
	MapPlayerEx* player = this->dynamic_player();
	JUDGE_RETURN(skill != NULL && player != NULL, 0);

	if (skill->__transfer_no_release == 1)
	{
		JUDGE_RETURN(player->check_is_in_transfer_time() == false, ERROR_IN_TRANSFER_TIME);
	}
	else if (skill->__transfer_no_release == 2)
	{
		JUDGE_RETURN(player->check_is_in_cool_time() == false, ERROR_TRANSFER_IN_COOL);
		JUDGE_RETURN(player->is_lrf_change_mode() == false, ERROR_CLIENT_OPERATE);

		player->player_use_transfer();
		return 1;
	}

	return 0;
}

int GameFighter::is_in_safe_area()
{
	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, false);
	return scene->is_in_safe_area(this);
}

int GameFighter::validate_safe_area(GameFighter *defender)
{
    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, 0);

    if (scene->validate_in_safe_area(defender) == 0)
    {
        return ERROR_IN_SAFE_AREA;
    }

    return 0;
}

int GameFighter::validate_skill_attack_type(int skill_id, GameEnum::SkillTargetFlagSet &target_flag_set)
{
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
    JUDGE_RETURN(skill_json.isMember("type") == true, ERROR_CONFIG_NOT_EXIST);

    std::string type_str = skill_json["type"].asString();
    for (int i = 0; i < GameEnum::SKILL_TARGET_END; ++i)
    {
    	JUDGE_CONTINUE(target_flag_set.test(i) == true);
    	JUDGE_CONTINUE(GameCommon::check_config_value(type_str, i) == true);
    	return 0;
    }

    return ERROR_TARGET_NO_ATTACK;
}

int GameFighter::validate_skill_usage(void)
{
	JUDGE_RETURN(this->is_player() == true, 0);
	JUDGE_RETURN(this->mutual_skill() <= 0, ERROR_MUTUAL_SKILL);

    FighterSkill* skill = this->fight_skill_.__skill;
    JUDGE_RETURN(skill->is_active_skill() == true, ERROR_SKILL_NOT_EXISTS);
    JUDGE_RETURN(skill->is_cool_finish() == true, ERROR_SKILL_COOL_LIMIT);
    JUDGE_RETURN(skill->arrive_fight_max_times() == false, ERROR_SKILL_FIGHT_USE_TIMES);

    if (GameCommon::is_jump_skill(skill) == true)
    {
    	JUDGE_RETURN(this->fight_detail_.__jump_times < 2, ERROR_OPERATE_TOO_FAST);
    	this->fight_detail_.__jump_times += 1;
    }
    else
    {
    	JUDGE_RETURN(this->is_jumping() == false, ERROR_USE_SKILL_JUMPING);
    }

    if (GameCommon::is_angry_skill(skill) == true)
    {
    	JUDGE_RETURN(this->fight_detail_.__angry >= GameEnum::MAX_ANGRY, ERROR_ANGRY_FULL);
    }

    JUDGE_RETURN(this->is_have_status_type(BasicStatus::DIZZY) == false, ERROR_PLAYER_DIZZY);

//    if (GameCommon::is_base_skill(skill) == true)
//    {
//    	JUDGE_RETURN(this->find_status(BasicStatus::SILENCE, status) != 0, ERROR_PLAYER_SILENCE);
//    }

//    if (this->find_status(BasicStatus::PEASANT, status) == 0)
//    {
//        JUDGE_RETURN(GameCommon::is_base_skill(skill) == true, ERROR_PLAYER_PEASANT);
//    }

	JUDGE_RETURN(this->level() >= skill->__use_level, ERROR_PLAYER_LEVEL_LIMIT);

	int limit_distance = this->fetch_check_distance();
	if (limit_distance > 0)
	{
		int cur_distance = ::coord_offset_grid(this->location(), this->fight_skill_.__skill_coord);
		JUDGE_RETURN(cur_distance <= limit_distance, ERROR_SKILL_DISTANCE_LIMIT);
	}

    return 0;
}

int GameFighter::refresh_skill_info(int skill_id)
{
    if (skill_id == 0)
	{
    	skill_id = this->fight_skill_.__skill_id;
	}
//    int skill_level = this->fight_skill_.__skill_level;

    FighterSkill *skill = 0;
    JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, ERROR_SKILL_NOT_EXISTS);

    skill->add_use_tick();
    skill->__fight_use_times += 1;

    const Json::Value &skill_json = skill->conf();
//    this->use_skill_magic(skill_json);

    if (skill_json.isMember("prepare") == true)
    {
        double prepare_time = skill_json["prepare"].asDouble();
        this->prepare_tick_map_[skill_id] = Time_Value::gettimeofday()
        	+ GameCommon::fetch_time_value(prepare_time);
    }
    else
    {
    	this->prepare_tick_map_[skill_id] = Time_Value::gettimeofday();
    }

    if (skill->__max_step > 0 && this->skill_id_for_step() == skill_id)
    {
    	this->fight_detail_.__skill_step = this->fight_skill_.__skill_step % skill->__max_step;
    }

    if (GameCommon::is_angry_skill(skill) == true)
    {
    	this->add_and_notify_angry_value(-1 * this->fight_detail_.__angry);
    }

    return 0;
}

int GameFighter::notify_launch_skill(int flag)
{
//	JUDGE_RETURN(CONFIG_INSTANCE->is_in_skill_noplay(this->fight_skill_.__display_skill) == false, -1);

    Proto80400201 respond;
    respond.set_fighter_id(this->fighter_id());
    respond.set_skill_id(this->fight_skill_.__display_skill);
    respond.set_skill_level(this->fight_skill_.__skill_level);

    respond.set_skill_step(this->fight_skill_.__skill_step);
    respond.set_add_tick(this->fight_skill_.__add_effect_tick);
    respond.set_target_id(this->fight_skill_.__client_target);

    respond.set_skill_pixel_x(this->fight_skill_.__play_coord.pixel_x());
    respond.set_skill_pixel_y(this->fight_skill_.__play_coord.pixel_y());

    LongSet* target_set = NULL;
    if (flag == false)
    {
    	target_set = &this->fight_skill_.__defender_set;
    }
    else
    {
    	target_set = &this->fight_skill_.__client_target_set;
    }

    for (LongSet::iterator iter = target_set->begin(); iter != target_set->end(); ++iter)
    {
        ProtoSkillTarget *target = respond.add_target_list();
        target->set_target_id(*iter);
        target->set_is_death(flag);
    }

    return this->respond_to_broad_area(&respond, this->fight_skill_.__is_full_screen);
}

void GameFighter::notify_launch_skill(FighterSkill* fighter_skill, const SkillExtInfo& ext_info)
{
	GameFighter* defender = ext_info.fighter_;
	if (defender == NULL)
	{
		defender = this->fetch_defender();
	}

	JUDGE_RETURN(defender != NULL, ;);

	Proto80400201 respond;
	respond.set_skill_step(0);
	respond.set_fighter_id(this->fighter_id());
	respond.set_skill_id(fighter_skill->__skill_id);
	respond.set_skill_level(fighter_skill->__level);

	if (ext_info.have_coord_ == true)
	{
		respond.set_skill_pixel_x(ext_info.coord_.pixel_x());
		respond.set_skill_pixel_y(ext_info.coord_.pixel_y());
	}
	else
	{
		respond.set_skill_pixel_x(defender->location().pixel_x());
		respond.set_skill_pixel_y(defender->location().pixel_y());
	}

	ProtoSkillTarget *target = respond.add_target_list();
	target->set_target_id(defender->fighter_id());
	this->respond_to_broad_area(&respond);
}

int GameFighter::notify_launch_effect_ai_skill(const Json::Value &skill_json)
{
//	uint step = this->fight_skill_.__skill_step;

	Proto80400114 respond;
    respond.set_effect_id(MAP_MONITOR->generate_effect_id());
    respond.set_effect_sort(skill_json["effect_id"][0u].asInt());

    respond.set_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
    respond.set_pixel_y(this->fight_skill_.__skill_coord.pixel_y());
    return this->respond_to_broad_area(&respond, this->fight_skill_.__is_full_screen);
}

int GameFighter::process_skill_effect_to_defender(void)
{
    // 1. 临时存储伤害前的相关伤害值
    DefenderHurt defender_hurt = this->fight_skill_.__defender_map[this->fight_skill_.__defender_id];
    defender_hurt.__defender_id = this->fight_skill_.__defender_id;

    GameFighter *defender = this->fetch_defender();
    JUDGE_RETURN(defender != NULL, 0);

    this->update_active_fight_state(defender_hurt.__defender_id);

    DefenderHurt attackor_hurt = defender->fight_detail().__attackor_map[this->fighter_id()];
    attackor_hurt.__defender_id = this->fighter_id();

    int prev_defender_blood = defender->fight_detail().__blood;
    int prev_defender_magic = defender->fight_detail().__magic;

    // 2. 计算并产生技能伤害效果
    int skill_id = this->fight_skill_.__skill_id;
    int skill_level = this->fight_skill_.__skill_level;

//    defender->stop_guide_skill_by_skill(skill_id);

    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
    const Json::Value &detail_json = this->detail_conf(skill_id, skill_level);
    JUDGE_RETURN(detail_json.empty() == false, ERROR_CONFIG_NOT_EXIST);

    if (skill_json["safe_area"].asInt() == 1
    		&& this->validate_safe_area(defender) == ERROR_IN_SAFE_AREA)
    {
        this->fight_skill_.__in_safe_area = true;
    }

    const Json::Value &effect_json = detail_json["effect"];
    for (uint i = 0; i < effect_json.size(); ++i)
    {
        JUDGE_CONTINUE(this->validate_trig_skill_effect(effect_json[i]) == 0);
        this->trigger_skill_effect_to_defender(effect_json[i]);
    }

    if (this->fight_skill_.__skill_blood != 0)
    {
        defender->modify_blood_by_fight(this->fight_skill_.__skill_blood, FIGHT_TIPS_NORMAL,
                this->fighter_id(), this->fight_skill_.__skill_id);
    }

    if (this->fight_skill_.__hurt_flag == true)
    {
        defender->process_rate_use_skill_by_hurt(this);

    	BasicStatus *status = 0;
		if (defender->find_status(BasicStatus::REPEATHURT, status) == 0)
		{
			status->__value1 = this->fight_skill_.__skill_hurt;
			status->__value2 = this->fight_skill_.__skill_hurt_percent;
		}

//		if (defender->find_status(BasicStatus::MREDUCE_HURT, status) == 0)
//		{
//			defender->process_magic_reduce_hurt(this, status->__value1, status->__value2);
//		}
    
		this->hurt(this->fight_skill_.__skill_hurt, this->fight_skill_.__skill_hurt_percent);
		this->fight_skill_.__hurt_flag = false;
		this->fight_skill_.__skill_hurt = 0;
		this->fight_skill_.__skill_hurt_percent = 0;
    }
    
    // 清除单次对被攻击者的附加值
    this->fight_skill_.__skill_hurt_deep = 0;
    this->fight_skill_.__skill_hurt_deep_percent = 0;
    this->fight_skill_.__skill_blood = 0;
    this->fight_skill_.__avoid_hurt = 0;

    // 3. 统计实际伤害
    int inc_hurt = prev_defender_blood - defender->fight_detail().__blood;
    int inc_magic = prev_defender_magic - defender->fight_detail().__magic;
    if (inc_hurt > 0)   // 统计实际伤害
    {
        defender_hurt.__hurt_blood += inc_hurt;
        attackor_hurt.__hurt_blood += inc_hurt;

        // 处理反伤效果
        this->process_defender_rebound(inc_hurt, defender_hurt.__hurt_blood);
    }

    if (inc_magic > 0)
    {
        defender_hurt.__hurt_magic += inc_magic;
        attackor_hurt.__hurt_magic += inc_magic;
    }

    this->fight_skill_.__defender_map[this->fight_skill_.__defender_id] = defender_hurt;
    defender->fight_detail().__attackor_map[this->fighter_id()] = attackor_hurt;

    return 0;
}

int GameFighter::process_skill_effect_no_defender(void)
{
    int skill_id = this->fight_skill_.__skill_id;
    int skill_level = this->fight_skill_.__skill_level;
    JUDGE_RETURN(skill_id > 0, 0);
    
    const Json::Value &detail_json = this->detail_conf(skill_id, skill_level);
    const Json::Value &effect_json = detail_json["effect"];

	const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
	int aoe_type = skill_json["aoeType"].asInt();
	for (uint i = 0; i < effect_json.size(); ++i)
	{
		JUDGE_CONTINUE(this->validate_trig_skill_effect(effect_json[i]) == 0);
		this->trigger_skill_effect_no_defender(effect_json[i], aoe_type);
	}

	return 0;
}

int GameFighter::validate_trig_skill_effect(const Json::Value &effect)
{
	JUDGE_RETURN(effect.isMember("condition") == true, 0);

    const Json::Value &condition_json = effect["condition"];
    if (condition_json.isMember("rate"))
    {
    	JUDGE_RETURN(GameCommon::validate_cur_rand(condition_json["rate"].asInt()) == true, -1);
    }
    if (condition_json.isMember("blood_percent"))
    {
        JUDGE_RETURN(this->fight_detail_.blood_percent(this) <= condition_json["blood_percent"].asInt(), -1);
    }
    if (condition_json.isMember("defender_blood_percent"))
    {
        GameFighter *defender = this->fetch_defender();
        JUDGE_RETURN(defender != 0, ERROR_NO_ATTACK_TARGET);

        JUDGE_RETURN(defender->fight_detail().blood_percent(defender) <= condition_json["defender_blood_percent"].asInt(), -1);
    }
    if (condition_json.isMember("defender_status"))
    {
        GameFighter *defender = this->fetch_defender();
        JUDGE_RETURN(defender != 0, ERROR_NO_ATTACK_TARGET);

        const Json::Value &defender_json = condition_json["defender_status"];
        for (uint i = 0; i < defender_json.size(); ++i)
        {
            BasicStatus *status = 0;
            JUDGE_RETURN(defender->find_status(defender_json[i].asInt(), status) == 0, -1);
        }
    }

    if (condition_json.isMember("attackor_status"))
    {
        const Json::Value &attackor_json = condition_json["attackor_status"];
        for (uint i = 0; i < attackor_json.size(); ++i)
        {
            BasicStatus *status = 0;
            JUDGE_RETURN(this->find_status(attackor_json[i].asInt(), status) == 0, -1);
        }
    }

    if (condition_json.isMember("skill_id"))
    {
    	int skill_id = condition_json["skill_id"].asInt();
        JUDGE_RETURN(this->fight_detail_.__passive_skill_set.count(skill_id) > 0, -1);
    }

    if (condition_json.isMember("skill_step"))
    {
        int step = condition_json["skill_step"].asInt();
        JUDGE_RETURN(this->fight_skill_.__skill_step == step, -1);
    }

    if (this->fight_skill_.__in_safe_area == true)
    {
    	JUDGE_RETURN(effect.isMember("safe_area_percent") == true
    			&& std::abs(effect["safe_area_percent"].asDouble()) > 0.00001, -1);
    }

    return 0;
}

int GameFighter::validate_movable(const MoverCoord &step)
{
    int ret = this->validate_fighter_movable();
    JUDGE_RETURN(ret == 0, ret);

    return GameMover::validate_movable(step);
}

bool GameFighter::validate_online_player()
{
	JUDGE_RETURN(this->is_player() == true, false);

	MapPlayerEx* player = dynamic_cast<MapPlayerEx*>(this);
	JUDGE_RETURN(player != NULL && player->is_need_send_message() == true, false);

	return true;
}

bool GameFighter::validate_left_blood(int need_left_blood)
{
	return this->fight_detail_.blood_percent(this) <= need_left_blood;
}

bool GameFighter::validate_enough_magic(int need_magic)
{
	return this->fight_detail_.__magic >= need_magic;
}

int GameFighter::validate_fighter_movable(void)
{
    JUDGE_RETURN(this->is_jumping() == false, -1);
    JUDGE_RETURN(this->is_death() == false, ERROR_PLAYER_DEATH);

    int ret = this->validate_no_forbit_move_status();
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

// 对defender产生效果的effect
// effect::name的处理只能在trigger_skill_effect_to_defender或trigger_skill_effect_no_defender中其中一个存在
int GameFighter::trigger_skill_effect_to_defender(const Json::Value &effect)
{
    GameFighter *effect_defender = this->fetch_defender();
    JUDGE_RETURN(effect_defender != NULL, -1);

    const std::string effect_name = effect["name"].asString();
    if (effect_name == Effect::HURT)
    {
        int value = 0;
        int percent = 0;

        //固定值
       	value += effect["value"].asInt();
        percent += effect["percent"].asInt();

        if (effect_defender->is_player() == true)
        {
        	//玩家
           	value += effect["player_value"].asInt();
            percent += effect["player_percent"].asInt();
        }
        else if (effect_defender->is_monster() == true)
        {
        	//怪物
           	value += effect["ai_value"].asInt();
            percent += effect["ai_percent"].asInt();
        }

        if (effect_defender->is_player() == true && effect.isMember("weaken_per") == true
        		&& this->fight_skill_.__client_target != effect_defender->mover_id())
        {
        	//玩家减弱
        	percent += effect["weaken_per"].asInt();
        }

        if (effect.isMember("percent_step") == true)
        {
        	//分步万分比
        	int skill_step = GameCommon::adjust_positive_integer(
        			this->fight_skill_.__skill_step, effect["percent_step"].size());
        	percent += effect["percent_step"][skill_step - 1].asInt();
        }

        if (effect.isMember("value_step") == true)
        {
        	//分步值
        	int skill_step = GameCommon::adjust_positive_integer(
        			this->fight_skill_.__skill_step, effect["value_step"].size());
        	value += effect["value_step"][skill_step - 1].asInt();
        }

		this->fight_skill_.__hurt_flag = true;
		this->fight_skill_.__skill_hurt += value;
		this->fight_skill_.__skill_hurt_percent += percent;

        return 0;
    }
    else if (effect_name == Effect::BUFF)
    {
        return this->insert_defender_status(effect_defender, effect["id"].asInt(),
        		effect["interval"].asDouble(), effect["last"].asDouble(),
                effect["accumulate_times"].asInt(), effect["value"].asInt(),
                effect["percent"].asDouble());
    }
    else if (effect_name == Effect::DIRECTHURT)
    {
    	//根据攻击力扣血
    	BasicStatus status(effect["value"].asInt(), effect["percent"].asInt(), this->fighter_id());
    	return effect_defender->direct_hurt(&status);
    }
    else if (effect_name == Effect::DIRECTMAXBLOOD)
    {
    	//根据受击者最大血量扣血
    	return this->direct_hurt_by_defender_max_blood(effect_defender, effect["percent"].asInt());
    }
    else if (effect_name == Effect::PULL)
    {
        return this->process_pull_near();
    }
    else if (effect_name == Effect::PUSH)
    {
        return this->process_push_away(effect["range"].asInt());
    }
    else if (effect_name == Effect::SHIELD)
    {
        return this->process_insert_shield_effect(effect_defender, effect);
    }
    else if (effect_name == Effect::LIFE_SHIELD)
    {
        return this->process_insert_life_shield_effect(effect_defender, effect);
    }
    else if (effect_name == Effect::PERCY)
    {
        return this->process_insert_percy_effect(effect_defender, effect);
    }
    else if (effect_name == Effect::FLASH_AVOID)
    {
        return this->process_insert_flash_avoid_effect(effect_defender, effect);
    }
    else if (effect_name == Effect::STONE_PLAYER)
    {
        return this->process_generate_stone_player(effect_defender, effect);
    }
    else if (effect_name == Effect::CRAZY_ANGRY)
    {
        return this->insert_defender_status(effect_defender,
        		BasicStatus::CRAZY_ANGRY, 0, effect["last"].asDouble());
    }
    else if (effect_name == Effect::ICE_INSIDE)
    {
    	GameAI* game_ai = this->self_ai();
    	JUDGE_RETURN(game_ai != NULL, 0);
    	game_ai->trigger_ice_inside_skill(this->fight_skill_.__skill);
    }
    else if (effect_name == Effect::JIAN_DROP)
    {
    	GameAI* game_ai = this->self_ai();
    	JUDGE_RETURN(game_ai != NULL, 0);
    	game_ai->trigger_jian_drop_skill(this->fight_skill_.__skill, effect_defender->fighter_id());
    }
    else if (effect_name == Effect::QUIT_REGION_TRANSFER)
    {
    	MapPlayerEx* player = this->self_player();
    	JUDGE_RETURN(player != NULL, 0);
    	player->quit_lrf_change_mode();
    }

    return 0;
}

// 对自身触发或在空地生成一只怪
// effect::name的处理只能在trigger_skill_effect_to_defender或trigger_skill_effect_no_defender中其中一个存在
int GameFighter::trigger_skill_effect_no_defender(const Json::Value &effect, int aoe_type)
{
	const std::string effect_name = effect["name"].asString();
	if (effect_name == Effect::SELFMAGIC)
	{
		int inc_magic = effect["value"].asInt() * -1;
		return this->modify_magic_by_notify(inc_magic, FIGHT_TIPS_NORMAL);
	}
    if (effect_name == Effect::AREAHURT)
    {
    	return this->process_area_hurt(effect, aoe_type);
    }
    if (effect_name == Effect::SUMMON)
    {
        return this->summon_monster(effect);
    }
    if (effect_name == Effect::FORWARD)
    {
        return this->process_self_forward(effect["range"].asInt());
    }
    if (effect_name == Effect::ASSAULT)
    {
        return this->process_assault(effect["range"].asInt(), effect);
    }
    if (effect_name == Effect::BACKWARD)
    {
        return this->process_self_backward(effect["range"].asInt());
    }
    if (effect_name == Effect::JUMP)
    {
    	this->set_start_jump(effect);
    	return 0;
    }
    if (effect_name == Effect::SELFBLOOD)
    {
    	double value = effect["value"].asDouble() * -1, percent = effect["percent"].asDouble() * -1;
    	if (std::abs(percent) > 0.000001)
    		value += (this->fight_detail_.__blood_total(this) * percent / 100.0);

        this->fight_skill_.__skill_self_blood += value;
    	//this->modify_blood_by_fight(value, FIGHT_TIPS_NORMAL, this->fighter_id(), this->fight_skill_.__skill_id);
    	return 0;
    }
    if (effect_name == Effect::SELF_HURTDEEP)
    {
        if (effect["last"].asDouble() > 0.000001)
        {
            return this->insert_defender_status(this, BasicStatus::HURTDEEP,
                    effect["interval"].asDouble(), effect["last"].asDouble(), effect["accumulate_times"].asInt(),
                    effect["value"].asInt(), effect["percent"].asDouble());
        }
    }
//    if (effect_name == Effect::SELF_DIZZY)
//    {
//        return this->insert_defender_status(this, BasicStatus::DIZZY,
//                effect["interval"].asDouble(), effect["last"].asDouble());
//    }
    if (effect_name == Effect::SELF_STAY)
    {
        return this->insert_defender_status(this, BasicStatus::STAY,
                effect["interval"].asDouble(), effect["last"].asDouble());
    }
    if (effect_name == Effect::SELF_ATTACK)
    {
        return this->insert_defender_status(this, BasicStatus::ATTACK,
                effect["interval"].asDouble(), effect["last"].asDouble(), effect["accumulate_times"].asInt(),
                effect["value"].asDouble(), effect["percent"].asDouble());
    }
    if (effect_name == Effect::SELF_DEFENCE)
    {
        return this->insert_defender_status(this, BasicStatus::DEFENCE,
                effect["interval"].asDouble(), effect["last"].asDouble(), effect["accumulate_times"].asInt(),
                effect["value"].asDouble(), effect["percent"].asDouble());
    }
    if (effect_name == Effect::SELF_INFMAGIC)
    {
        return this->insert_defender_status(this, BasicStatus::INFMAGIC,
                effect["interval"].asDouble(), effect["last"].asDouble(), effect["accumulate_times"].asInt(),
                effect["value"].asDouble(), effect["percent"].asDouble());
    }

    return 0;
}

int GameFighter::make_up_fight_update_info(Message *msg, const int type, const int value, const int64_t attackor, const int skill_id,
		const int tips1, const int tips2, const int tips3, const int tips4, const int64_t experience)
{
	MSG_DYNAMIC_CAST_RETURN(Proto80400202 *, res, -1);

    Proto80400202 &respond = *res;
    respond.set_fighter_id(this->fighter_id());

    ProtoFightInfo *proto_fight_info = respond.mutable_fight_info();
    proto_fight_info->set_type(type);
    proto_fight_info->set_value(value);

    if (attackor > 0)
    	proto_fight_info->set_attackor_id(attackor);
    if (skill_id > 0)
    	proto_fight_info->set_skill_id(skill_id);
    if (tips1 != 0)
    	proto_fight_info->set_tips1(tips1);
    if (tips2 != 0)
    	proto_fight_info->set_tips2(tips2);
    if (tips3 != 0)
    	proto_fight_info->set_tips3(tips3);
    if (tips4 != 0)
    	proto_fight_info->set_tips4(tips4);
    if (experience != 0)
    	proto_fight_info->set_experience(experience);
    return 0;
}

int GameFighter::notify_fight_update(const int type, int value, const int64_t attackor, const int skill_id,
		const int tips1, const int tips2, const int tips3, const int tips4, const int64_t experience)
{
	JUDGE_RETURN(this->is_enter_scene() == true, 0);

    Proto80400202 respond;
    this->make_up_fight_update_info(&respond, type, value, attackor, skill_id,
    		tips1, tips2, tips3, tips4, experience);

    if (FIGHT_UPDATE_BLOOD == type && this->is_player() == true)
    {
    	this->team_notify_teamer_blood();
    }

    switch(type)
    {
    case FIGHT_UPDATE_MAGIC:
    case FIGHT_UPDATE_PK:
	case FIGHT_UPDATE_EXP:
	case FIGHT_UPDATE_DISTANCE:
	case FIGHT_UPDATE_ANGRY:
	case FIGHT_UPDATE_SKILL:
	case FIGHT_UPDATE_JUMP:
	{
		this->respond_from_broad_client(ACTIVE_UPDATE_FIGHT, 0, &respond);
		break;
	}
	default:
	{
		this->respond_to_broad_area(&respond);
		break;
	}
    }
    return 0;
}

int GameFighter::update_fighter_jump_value(int value, int skill_id)
{
	this->fight_detail_.__jump += value;
	return this->notify_fight_update(FIGHT_UPDATE_JUMP, this->fight_detail_.__jump, 0, skill_id);
}

int GameFighter::update_fighter_speed(int type, double value, int offset)
{
	switch (type)
	{
	case GameEnum::SPEED:
	{
		this->mover_detial_.__speed.set_single(value, offset);
		break;
	}
	case GameEnum::SPEED_MULTI:
	{
		value = GameCommon::div_percent(value);
		this->mover_detial_.__speed_multi.set_single(value, offset);
		break;
	}
	default:
	{
		return -1;
	}
	}

	return this->notify_update_speed();
}

int GameFighter::notify_update_speed(void)
{
	JUDGE_RETURN(this->is_enter_scene() == true, -1);
    return this->notify_fight_update(FIGHT_UPDATE_SPEED, this->speed_total_i());
}

int GameFighter::notify_fighter_stay(void)
{
//    int hit_type = 0;
//    if (this->validate_hit_forbit_move_status() != 0)
//        hit_type = 1;   // 没有受击动作的不可移动状态
//
//    Proto80400211 respond;
//    respond.set_hit_type(hit_type);
//    return this->respond_to_client(ACTIVE_PLAYER_CANT_MOVE, &respond);
	return 0;
}

int GameFighter::notify_fighter_exit_stay(void)
{
//    Proto80400212 respond;
//    respond.set_fighter_id(this->fighter_id());
    return this->respond_to_client(ACTIVE_PLAYER_CAN_MOVE);
}

int GameFighter::update_fight_property(int type)
{
	return 0;
}

int GameFighter::notify_update_player_info(int update_type, Int64 value)
{
	return 0;
}

int GameFighter::notify_fight_state(const Int64 fighter_id)
{
//	JUDGE_RETURN(this->is_player() == true, 0);
	return this->notify_fight_update(FIGHT_UPDATE_FIGHT_STATE,
			this->fight_detail_.__fight_state, fighter_id);
}

int GameFighter::find_avoid_buff(const int status)
{
	return -1;
}

GameFighter* GameFighter::fetch_hurt_figher()
{
	return this;
}

double GameFighter::avoid_rate_in_hurt(GameFighter *defender)
{
	double avoid_rate = ::sqrt(defender->fight_detail().__avoid_total(defender))
			- ::sqrt(this->fight_detail().__hit_total(this));
	return std::max<double>(0, avoid_rate);
}

double GameFighter::hit_rate_in_hurt(GameFighter *defender)		// 计算命中率
{
	double defender_avoid = defender->fight_detail().__avoid_total(defender);
	double attacker_hit = this->fight_detail().__hit_total(this);

	double avoid_rate = defender_avoid * defender_avoid
			/ (defender->fight_detail().avoid_klv_ + defender_avoid)
			/ (0.01 + attacker_hit + this->fight_detail().hit_klv_);
	return std::max<double>(0.6, 1 - avoid_rate);
}

bool GameFighter::is_jump_avoid_hurt()
{
	JUDGE_RETURN(this->is_jumping() == true, false);

	static int JUMP_VALUE = 1;
	JUDGE_RETURN(this->fight_detail_.__jump >= JUMP_VALUE, false);

	this->update_fighter_jump_value(-1 * JUMP_VALUE);
	return true;
}

bool GameFighter::is_hit_in_hurt(GameFighter *defender)
{
	int hit_rate = this->hit_rate_in_hurt(defender) * GameEnum::DAMAGE_ATTR_PERCENT;
	return GameCommon::validate_cur_rand(hit_rate);
}

double GameFighter::attack_in_hurt(void)
{
	return this->fight_detail_.__attack_total(this);
}

double GameFighter::defence_in_hurt(void)
{
	return this->fight_detail_.__defence_total(this);
}

double GameFighter::crit_power_in_hurt(GameFighter *defender)
{
	double attack_crit = this->fight_detail_.__crit_total(this);
	double defend_toughness = defender->fight_detail().__toughness_total(defender);

    double real_crit_rate = attack_crit * attack_crit
    		/ (this->fight_detail_.crit_klv_ + attack_crit)
    		/ (0.01 + defend_toughness + defender->fight_detail().toughness_klv_);

	return std::min<double>(0.6, real_crit_rate);
}

bool GameFighter::is_crit_in_hurt(GameFighter *defender)
{
    if (this->is_have_status_type(BasicStatus::CRAZY_ANGRY) == true)
    {
    	return true;
    }

    if (this->is_beast() == true)
    {
    	return false;
    }

    {
    	//原始概率
    	double src_rate = this->crit_power_in_hurt(defender) * GameEnum::DAMAGE_ATTR_PERCENT;
    	//状态加成
    	DoublePair buff_add = this->find_status_pair_value(BasicStatus::CRIT_RATE);

        //总概率
        int total_rate = (src_rate + buff_add.first) * (1 + GameCommon::div_percent(buff_add.second));
    	return GameCommon::validate_cur_rand(total_rate);
    }
}

int GameFighter::hurt(double value, double percent)
{
    GameFighter *defender = this->fetch_defender();
    JUDGE_RETURN(defender != 0, ERROR_TARGET_NOT_FOUND);

    int skill_id = this->fight_skill_.__skill_id;
//    int skill_level = this->fight_skill_.__skill_level;

	GameFighter* hurt_fighter = this->fetch_hurt_figher();
	JUDGE_RETURN(hurt_fighter != NULL, 0);

	int avoid_tips_type = 0;
	if (defender->is_jump_avoid_hurt() == true)
	{
		avoid_tips_type = FIGHT_TIPS_JUMP_MISS;
	}
	else if (hurt_fighter->is_hit_in_hurt(defender) == false)
	{
		avoid_tips_type = FIGHT_TIPS_MISS;
	}

#ifdef LOCAL_DEBUG
	avoid_tips_type = 0;
#endif

	if (avoid_tips_type > 0)
	{
//		MSG_USER("avoid hurt %d %d %ld <- %ld", avoid_tips_type, skill_id,
//				defender->fighter_id(), this->fighter_id());

		this->fight_skill_.__avoid_hurt = 1;
		defender->notify_fight_update(FIGHT_UPDATE_BLOOD, 0, this->fighter_id(),
				skill_id, defender->fight_detail().__blood, avoid_tips_type, skill_id);
		defender->calc_rebound_hurt_when_avoid(this);

		//avoid
		return 0;
	}

	// 攻击力
	double attack =  hurt_fighter->attack_in_hurt();
	// 被攻击者防御力
	double defence = defender->defence_in_hurt();
	// 伤害加成率
	double increase_rate = hurt_fighter->fetch_increase_hurt_rate(defender);
	// 伤害减免率
	double reduce_rate = defender->fetch_reduce_hurt_rate();
	// 额外伤害
	double increase_hurt_value = hurt_fighter->fetch_increase_hurt_value()
			+ this->fetch_beast_hurt_value();
	// 免伤值
	double reduce_hurt_value = defender->fetch_reduce_hurt_value();
	// 暴击
	bool crit_flag = this->is_crit_in_hurt(defender);

	double real_hurt = std::max<double>(0, (attack - defence) * percent / GameEnum::DAMAGE_ATTR_PERCENT + value)
			* std::max<double>(0.0001, (1 + increase_rate - reduce_rate)) + increase_hurt_value - reduce_hurt_value;
	real_hurt = real_hurt * this->fetch_beast_hurt_rate();
	real_hurt = std::max<double>(attack / 100, real_hurt);

	// 暴击
	int fight_tips = FIGHT_TIPS_NORMAL;
	if (crit_flag == true)
	{
		fight_tips = FIGHT_TIPS_CRIT;

		//状态加成倍数
		DoublePair buff_add = this->find_status_pair_value(BasicStatus::CRIT_HURT);
		//防御者减少
		DoublePair def_reduce = defender->fetch_reduce_crit_hurt_pair();

		//原始倍数
		double src_times = hurt_fighter->fight_detail_.__crit_hurt_multi_total(hurt_fighter)
				+ hurt_fighter->fetch_increase_crit_hurt_rate(defender) - def_reduce.second;
		//总倍数
		double total_times = (src_times + GameCommon::div_percent(buff_add.first))
				* (1 + GameCommon::div_percent(buff_add.second));
		real_hurt *= total_times;
		real_hurt -= def_reduce.first;
	}

	return defender->modify_blood_by_fight(std::max<double>(real_hurt, 1.00),
			fight_tips, this->fighter_id(), skill_id);
}

int GameFighter::cure(double value, double percent)
{
	 JUDGE_RETURN(this->is_blood_full() == false, 0);
	 JUDGE_RETURN(value >= 0 && percent >= 0,-1);

	 int total_blood = this->fight_detail_.__blood_total_i(this);
	 double percentage = percent / 100;
	 if(int(this->fight_detail_.__blood + value + total_blood * percentage) >= total_blood)
	     this->modify_blood_by_fight(-(total_blood - this->fight_detail_.__blood), FIGHT_TIPS_SYSTEM_AUTO);
	 else
		 this->modify_blood_by_fight(-(value + total_blood * percentage),FIGHT_TIPS_SYSTEM_AUTO);

	return 0;
}

int GameFighter::modify_element_experience(const int value, const SerialObj &serial_obj)
{
    return -1;
}

int GameFighter::level_upgrade(void)
{
	this->fight_detail_.set_level(this->level() + 1);
    return 0;
}

int GameFighter::force_total_i(void)
{
	return 0;
}

int GameFighter::die_process(const int64_t fighter_id)
{
    if (this->fight_detail().__fight_state != GameEnum::FIGHT_STATE_NO)
    {
    	this->fight_detail().__fight_state = GameEnum::FIGHT_STATE_NO;
        this->fight_detail_.__fight_tick = Time_Value::zero;
    }

	if(this->is_beast())
	{
		MSG_USER("beast not die");
		this->fight_detail_.__blood = this->fight_detail_.__blood_total_i(this);
		return 0;
	}

	MSG_DEBUG("fighter die ==> %ld attack %ld", fighter_id, this->fighter_id());

    this->clear_status(StatusQueueNode::DIE_REMOVE);

    // 清除互斥技能
    if (this->mutual_skill() > 0)
    {
        this->notify_cancel_loop_skill(this->mutual_skill());
        this->mutual_skill_.reset();
    }

    for (int i = 0; i < TOTAL_LOOP; ++i)
    {
    	LoopSkillDetail& loop_skill = this->loop_skill_[i];
    	this->notify_cancel_loop_skill(loop_skill.__skill);
    	loop_skill.reset();
    }

    return 0;
}

int GameFighter::refresh_fight_state(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->fight_detail_.__fight_tick != Time_Value::zero, 0);
	JUDGE_RETURN(this->fight_detail_.__fight_tick <= nowtime, 0);

	this->fight_detail_.__fight_tick = Time_Value::zero;
	this->fight_detail_.__fight_state = GameEnum::FIGHT_STATE_NO;
	this->fight_detail_.__attackor_map.unbind_all();

	this->notify_fight_state(0);
	this->update_active_fight_state(0);
//	this->blood_recovert_tick_ = nowtime + Time_Value(RECOVERT_BLOOD_INTERVAL);
	return 0;
}

int GameFighter::refresh_blood_recover(const Time_Value &now_time)
{
	JUDGE_RETURN(this->fight_detail_.__auto_recover_blood > 0, 0);
	JUDGE_RETURN(this->fight_detail_.__recover_blood_per > 0, 0);

	JUDGE_RETURN(this->blood_recovert_tick_ <= now_time.sec(), 0);
	JUDGE_RETURN(this->is_blood_full() == false, 0);

	int total_blood = this->total_recover_blood();
	this->modify_blood_by_fight(-1 * total_blood, FIGHT_TIPS_SYSTEM_AUTO);
	this->blood_recovert_tick_ = now_time.sec() + this->fight_detail_.__recover_blood_span;

	return 0;
}

// 自动扣除周围敌人的血
int GameFighter::auto_modify_around_blood(const Time_Value &now_time)
{
	return 0;
}

int GameFighter::process_launch_skill(const Time_Value &nowtime)
{
    while (this->skill_delay_launch_queue_.size() > 0)
    {
        if (this->skill_delay_launch_queue_.top()->__launch_tick > nowtime)
        {
            break;
        }

        DelaySkillInfo *skill_ptr = this->skill_delay_launch_queue_.top();
        std::auto_ptr<Proto10400202> request = skill_ptr->__request;
        this->launch_fight_skill(request.get(), false);
        this->process_skill_post_launch(request->skill_id(), 0);

        this->skill_delay_launch_queue_.pop();
        this->monitor()->delay_skill_pool()->push(skill_ptr);
        break;
    }

    return 0;
}

double GameFighter::fetch_beast_hurt_rate()
{
	return 1;
}

double GameFighter::fetch_beast_hurt_value()
{
	return 0;
}

double GameFighter::fetch_reduce_hurt_rate(void)
{
    double buff_exempt = this->find_status_value_by_type(
    		BasicStatus::EXEMPT, BasicStatus::VALUE2);
    double buff_increase = this->find_status_value_by_type(
    		BasicStatus::HURTDEEP, BasicStatus::VALUE2);

    double src_rate = this->fight_detail_.__reduction_rate_total(this);
    double buff_rate = GameCommon::div_percent(buff_exempt - buff_increase);

    return src_rate + buff_rate;
}

double GameFighter::fetch_reduce_hurt_value()
{
    double buff_exempt = this->find_status_value_by_type(
    		BasicStatus::EXEMPT, BasicStatus::VALUE1);
    double buff_increase = this->find_status_value_by_type(
    		BasicStatus::HURTDEEP, BasicStatus::VALUE1);

    double src_value = this->fight_detail_.__reduction_total(this);
    double buff_value = buff_exempt - buff_increase;

    return src_value + buff_value;
}

DoublePair GameFighter::fetch_reduce_crit_hurt_pair()
{
	DoublePair pair;
	return pair;
}

double GameFighter::fetch_increase_hurt_rate(GameFighter* fighter)
{
	return this->fight_detail_.__damage_rate_total(this);
}

double GameFighter::fetch_increase_hurt_value(void)
{
	return this->fight_detail_.__damage_total(this);
}

double GameFighter::fetch_increase_crit_hurt_rate(GameFighter* fighter)
{
	return 0;
}

double GameFighter::fetch_increase_crit_hurt_value(void)
{
	return 0;
}

double GameFighter::fetch_passive_skill_use_rate(FighterSkill* skill)	//被动技能使用概率
{
	GameFighter* target = this->fetch_defender();
	JUDGE_RETURN(target != NULL, skill->__use_rate);

    int sub_rate = target->fetch_sub_skill_use_rate(skill);
    JUDGE_RETURN(sub_rate > 0, skill->__use_rate);

   return skill->__use_rate * (GameEnum::DAMAGE_ATTR_PERCENT
			- sub_rate) * 1.0 / GameEnum::DAMAGE_ATTR_PERCENT;
}

double GameFighter::fetch_sub_skill_use_rate(FighterSkill* skill)
{
	return 0;
}

// 处理被攻击者的反弹效果
int GameFighter::process_defender_rebound(const double cur_hurt, const double total_hurt)
{
//	JUDGE_RETURN(this->is_beast() == false, -1);
//	JUDGE_RETURN(cur_hurt > 0, -1);
//
//    GameFighter *defender = this->fetch_defender();
//    JUDGE_RETURN(defender != 0, -1);
//
//    BasicStatus *status = 0;
//    if (defender->find_status(BasicStatus::REBOUNDHURT, status) == 0)
//    {
//        double reduce_blood = cur_hurt * (status->__value2 / 100.0);
//        this->modify_blood_by_fight((reduce_blood > 1.0 ? reduce_blood : 1.0),
//        		FIGHT_TIPS_STATUS, defender->fighter_id(), status->__skill_id);
//    }
//
//    if (defender->find_status(BasicStatus::REBOUNDDEFENCE, status) == 0)
//    {
//        double reduce_blood = defender->fight_detail().__defence_total(defender) * (status->__value2 / 100.0);
//        this->modify_blood_by_fight((reduce_blood > 1.0 ? reduce_blood : 1.0),
//        		FIGHT_TIPS_STATUS, defender->fighter_id(), status->__skill_id);
//    }
//
//    if (defender->find_status(BasicStatus::REBOUNDMAXBLOOD, status) == 0)
//    {
//        double reduce_blood = this->fight_detail().__blood_total(this) * (status->__value2 / 100.0);
//        this->modify_blood_by_fight((reduce_blood > 1.0 ? reduce_blood : 1.0),
//        		FIGHT_TIPS_STATUS, defender->fighter_id(), status->__skill_id);
//    }
    return 0;
}

int GameFighter::recovert_magic(const Time_Value &nowtime)
{
	return 0;
}

int GameFighter::recovert_blood(const Time_Value &nowtime)
{
    return 0;
}

const Json::Value &GameFighter::cur_skill_detail_conf()
{
	return GameCommon::skill_detail(this->fight_skill_.__skill_id,
			this->fight_skill_.__skill_level);
}

const Json::Value &GameFighter::detail_conf(const int skill_id, const int skill_level)
{
    return GameCommon::skill_detail(skill_id, skill_level);
}

int GameFighter::make_up_skill_target(MoverMap &fighter_map, const int skill_id, const MoverCoord &org_skill_coord, const double org_angle/* = 90.0*/)
{
    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    FighterSkill *skill = 0;
    JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, -1);
    
    MoverCoord skill_coord = org_skill_coord;
    const Json::Value &skill_json = skill->conf();

    int skill_level = skill->__level;
    int distance = skill->__distance;
    distance = (distance <= 0 ? 4 : distance);
    int radius = skill->__radius;
    radius = (radius <= 1 ? distance : radius);

    double angle = org_angle;
    if (skill_json.isMember("angle"))
    {
        angle = skill_json["angle"].asInt();
    }

    switch (skill->__aoe_type)
    {
	case GameEnum::SKILL_AOE_SELF_TARGET:
	{
		fighter_map[this->fighter_id()] = this;
		break;
	}
	case GameEnum::SKILL_AOE_CUR_AIM_TARGET:
	{
		Int64 last_defender = this->fight_detail_.__last_defender_id;
		GameFighter *defender = 0;
		if (scene->find_fighter(last_defender, defender) != 0
				|| defender->is_death() == true
				|| defender->is_enter_scene() == false)
		{
			this->fight_detail_.__last_defender_id = 0;
			scene->fetch_all_around_fighter(this, fighter_map, this->location(), radius, 20);
		}
		else
		{
			fighter_map[defender->fighter_id()] = defender;
		}
		break;
	}
	case GameEnum::SKILL_AOE_TARGET_CIRCLE:
	case GameEnum::SKILL_AOE_TARGET_POINT_CIRCLE:
	{
		scene->fetch_all_around_fighter(this, fighter_map, skill_coord, radius, 20);
		break;
	}
	case GameEnum::SKILL_AOE_SELF_CIRCLE:
	{
		scene->fetch_all_around_fighter(this, fighter_map, this->location(), radius, 20);
		break;
	}
	case GameEnum::SKILL_AOE_SELF_SECTOR:
	{
		this->fight_skill_.set_angle(angle);
		if (this->location() == skill_coord)
		{
			int pixel_x = this->location().pixel_x() + ::cos(this->fight_skill_.__radian) * distance;
			int pixel_y = this->location().pixel_y() + ::sin(this->fight_skill_.__radian) * distance;
			skill_coord.set_pixel(pixel_x, pixel_y);
		}

		scene->fetch_all_sector_fighter(this, fighter_map, this->location(), skill_coord, radius, angle, 20);
		break;
	}
	case GameEnum::SKILL_AOE_SELF_RECT:
	{
		MoverCoord& self = this->location();

		int width = GameCommon::json_by_level(skill_json["width"], skill_level).asInt() * 30;
		int height = GameCommon::json_by_level(skill_json["height"], skill_level).asInt() * 30;

		//this->fight_skill_.set_angle(angle);
		skill_coord.set_pixel(self.pixel_x() + height / 2.0 * ::cos(this->fight_skill_.__radian),
				self.pixel_y() + height / 2.0 * ::sin(this->fight_skill_.__radian));

		this->fight_skill_.__skill_coord = skill_coord;
		scene->fetch_all_rect_fighter(this, fighter_map, skill_coord, angle, width, height, 20);
		break;
	}
	case GameEnum::SKILL_AOE_SELF_RING:
	{
		int outer_radius = std::max(GameCommon::json_by_level(skill_json["outer_radius"], skill_level).asInt(), 4);
		 int inner_radius = std::max(GameCommon::json_by_level(skill_json["inner_radius"], skill_level).asInt(), 4);
		 if (outer_radius <= inner_radius)
			outer_radius = inner_radius + 4;

		scene->fetch_all_around_player_in_ring(this, fighter_map, this->location(), inner_radius, outer_radius);
		break;
	}
	case GameEnum::SKILL_AOE_TARGET_RECT:
	{
		int width = GameCommon::json_by_level(skill_json["width"], skill_level).asInt() * 30,
				height = GameCommon::json_by_level(skill_json["height"], skill_level).asInt() * 30;

		this->fight_skill_.__skill_coord = skill_coord;
		//this->fight_skill_.__radian = angle;
		scene->fetch_all_rect_fighter(this, fighter_map, skill_coord, angle, width, height, 20);
		break;
	}
	case GameEnum::SKILL_AOE_BOSS:
	{
		scene->fetch_all_around_boss(this, fighter_map, this->location(), distance);
		break;
	}
	default:
	{
		GameFighter *defender = this->fetch_defender();
		if (defender != 0)
		{
			fighter_map[defender->fighter_id()] = defender;
		}
		break;
	}
    }
    return 0;
}

int GameFighter::make_up_skill_target(FighterSkill* skill, const MoverCoord &org_skill_coord, Proto10400201* request)
{
	JUDGE_RETURN(skill->__object_from_server == 1, -1);

    MoverCoord skill_coord = org_skill_coord;
    if (request->target_list_size() > 0 && check_coord_distance(skill_coord, this->location(), skill->__distance) == false)
    {
		for (int i = 0; i < request->target_list_size(); ++i)
		{
			GameFighter *fighter = 0;
			if (this->find_fighter(request->target_list(i).target_id(), fighter) != 0)
				continue;
			if (check_coord_distance(fighter->location(), this->location(), skill->__distance) == false)
				continue;

			skill_coord = fighter->location();
			this->fight_skill_.__skill_coord = fighter->location();
			request->set_skill_pixel_x(skill_coord.pixel_x());
			request->set_skill_pixel_y(skill_coord.pixel_y());
			break;
		}
    }

    MoverMap fighter_map;
    int ret = this->make_up_skill_target(fighter_map, skill->skill_id(), skill_coord, this->fight_skill_.__angle);
    JUDGE_RETURN(ret == 0, ret);

    for (MoverMap::iterator iter = fighter_map.begin(); iter != fighter_map.end(); ++iter)
    {
        request->add_target_list()->set_target_id(iter->first);
    }

    return 0;
}

//percent: 万分比
int GameFighter::direct_hurt_by_defender_max_blood(GameFighter* defender, double percent)
{
	return this->direct_hurt_by_defender_max_blood(defender, DoublePair(0, percent));
}

int GameFighter::direct_hurt_by_defender_max_blood(GameFighter* defender, const DoublePair& pair)
{
	JUDGE_RETURN(defender != 0, -1);

	BasicStatus status;
	status.__buff_type = BasicStatus::DIRECTMAXBLOOD;
	status.__attacker = this->mover_id();

	status.set_normal_value(pair.first, pair.second);
	return defender->update_blood_status(&status, FIGHT_TIPS_NORMAL);
}

int GameFighter::generate_area_hurt_monster(const int sort, MoverCoord &location, const double angle)
{
    Int64 ai_id = AIMANAGER->generate_ai_effect(sort, location, this);
    GameAI *game_ai = AIMANAGER->effect_ai_package()->find_object(ai_id);
    JUDGE_RETURN(game_ai != NULL, -1);

    game_ai->set_caller(this->fighter_id());
    game_ai->set_camp_id(this->camp_id());
    game_ai->mover_detail().__toward = angle;
	game_ai->enter_scene();
    return 0;
}

int GameFighter::summon_monster(const Json::Value &effect)
{
    Scene *scene = this->fetch_scene();
    if (scene == 0)
        return -1; 

    int sum_ret = scene->summon_monster(this, effect);
    if (sum_ret == 0 && effect["kill_caller"].asInt() == 1)
    {
    	this->modify_blood_by_fight(this->fight_detail().__blood_total(this));
    }

    return 0;
}

int GameFighter::clean_status(int status_id)
{
	int loop_max = 0;
    BasicStatus *clean_status = 0;
    while (this->find_status(status_id, clean_status) == 0 && (++loop_max) <= 20)
    {
        this->remove_status(clean_status);
    }

    return this->notify_status_update_property();
}

int GameFighter::blood_max_set(const double value, const int offset, const int enter_type)
{
	double left_percent = this->fight_detail_.blood_percent(this, 1);
	this->fight_detail_.__blood_max.set_single(value, offset);
	JUDGE_RETURN(enter_type != ENTER_SCENE_TRANSFER, 0);

	this->fight_detail_.set_cur_blood(left_percent);
	return 1;
}

int GameFighter::blood_max_add(const double value, const int offset, const int enter_type)
{
	double left_percent = this->fight_detail_.blood_percent(this, 1);
	this->fight_detail_.__blood_max.add_single(value, offset);
	JUDGE_RETURN(enter_type != ENTER_SCENE_TRANSFER, 0);

	this->fight_detail_.set_cur_blood(left_percent);
	return 1;
}

int GameFighter::blood_max_reduce(const double value, const int offset)
{
	double left_percent = this->fight_detail_.blood_percent(this, 1);
	this->fight_detail_.__blood_max.reduce_single(value, offset);
	this->fight_detail_.set_cur_blood(left_percent);
	return 1;
}

int GameFighter::blood_max_multi_set(const double value, const int offset, const int enter_type)
{
	double left_percent = this->fight_detail_.blood_percent(this, 1);
	this->fight_detail_.__blood_multi.set_single(value, offset);
	JUDGE_RETURN(enter_type != ENTER_SCENE_TRANSFER, 0);

	this->fight_detail_.set_cur_blood(left_percent);
	return 1;
}

int GameFighter::blood_max_multi_add(const double value, const int offset, const int enter_type)
{
	double left_percent = this->fight_detail_.blood_percent(this, 1);
	this->fight_detail_.__blood_multi.add_single(value, offset);
	JUDGE_RETURN(enter_type != ENTER_SCENE_TRANSFER, 0);

	this->fight_detail_.set_cur_blood(left_percent);
	return 1;
}

int GameFighter::blood_max_multi_reduce(const double value, const int offset)
{
	double left_percent = this->fight_detail_.blood_percent(this, 1);
	this->fight_detail_.__blood_multi.reduce_single(value, offset);
	this->fight_detail_.set_cur_blood(left_percent);
	return 1;
}

int GameFighter::magic_max_set(const double value, const int offset, const int enter_type)
{
	double left_percent = this->fight_detail_.magic_percent(this, 1);
	this->fight_detail_.__magic_max.set_single(value, offset);
	JUDGE_RETURN(enter_type != ENTER_SCENE_TRANSFER, 0);

	this->fight_detail_.set_cur_magic(left_percent);
	return 1;
}

int GameFighter::magic_max_add(const double value, const int offset)
{
	double left_percent = this->fight_detail_.magic_percent(this, 1);
	this->fight_detail_.__magic_max.add_single(value, offset);
	this->fight_detail_.set_cur_magic(left_percent);
	return 1;
}

int GameFighter::magic_max_reduce(const double value, const int offset)
{
	double left_percent = this->fight_detail_.magic_percent(this, 1);
	this->fight_detail_.__magic_max.reduce_single(value, offset);
	this->fight_detail_.set_cur_magic(left_percent);
	return 1;
}

int GameFighter::magic_max_multi_set(const double value, const int offset, const int enter_type)
{
	double left_percent = this->fight_detail_.magic_percent(this, 1);
	this->fight_detail_.__magic_multi.set_single(value, offset);
	JUDGE_RETURN(enter_type != ENTER_SCENE_TRANSFER, 0);

	this->fight_detail_.set_cur_magic(left_percent);
	return 1;
}

int GameFighter::magic_max_multi_add(const double value, const int offset)
{
	double left_percent = this->fight_detail_.magic_percent(this, 1);
	this->fight_detail_.__magic_multi.add_single(value, offset);
	this->fight_detail_.set_cur_magic(left_percent);
	return 1;
}

int GameFighter::magic_max_multi_reduce(const double value, const int offset)
{
	double left_percent = this->fight_detail_.magic_percent(this, 1);
	this->fight_detail_.__magic_multi.reduce_single(value, offset);
	this->fight_detail_.set_cur_magic(left_percent);
	return 1;
}

int GameFighter::base_skill_id()
{
	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
			iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
		JUDGE_CONTINUE(GameCommon::is_base_skill(iter->second) == true);
		return iter->first;
	}
	return 0;
}

int GameFighter::first_skill_id()
{
	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
			iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
		return iter->first;
	}
	return 0;
}

int GameFighter::process_pull_near(void)
{
    GameFighter *defender = this->fetch_defender();
    JUDGE_RETURN(defender != 0, 0);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, 0);

    GameAI *game_ai = dynamic_cast<GameAI *>(defender);
    if (game_ai != 0)
    {
        if (game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) ||
                game_ai->test_ai_flag(GameEnum::AI_CF_NO_PUSH))
            return 0;
    }

    if (coord_offset_grid(this->location(), defender->location()) <= 3)
        return 0;

    MoverCoord target_coord = this->fetch_direct_point(this->location(), defender->location(), MoverCoord::pos_to_pixel(3));

    defender->notify_fighter_teleport(this->fighter_id(), this->fight_skill_.__skill_id, target_coord);

    scene->refresh_mover_location(defender, target_coord, false);
    return 0;
}

MoverCoord GameFighter::fetch_direct_point(const MoverCoord &src_coord,
		const MoverCoord &reference_point, const int range,
		const bool is_ignore_middle)
{
    int dx = reference_point.pos_x() - src_coord.pos_x(),
        dy = reference_point.pos_y() - src_coord.pos_y();
    double k = 0, k_dlta = 0.0;
    int dlta = 0, is_use_dlta_x = 1, inc_unit = 1;
    if (std::abs(dy) > std::abs(dx))
    {
        is_use_dlta_x = 0;
        dlta = (dy < 0 ? -inc_unit : inc_unit);
        if (dy != 0)
        {
            k = double(dx) / dy * inc_unit;
            k = (dlta < 0 ? -k : k);
        }
    }
    else
    {
        is_use_dlta_x = 1;
        dlta = (dx < 0 ? -inc_unit : inc_unit);
        if (dx != 0)
        {
            k = double(dy) / dx * inc_unit;
            k = (dlta < 0 ? -k : k);
        }
    }

    int offset_ref = coord_offset_grid(reference_point, src_coord),
    		offset_target_coord = 0;
    int range_pos = MoverCoord::pixel_to_pos(range) +  (range % 30 == 0 ? 0 : 1), i = 0;
    MoverCoord target_coord = src_coord, prev_coord = src_coord;
    for (i = 0; i < range_pos; ++i)
    {
        k_dlta += k;
        if (is_use_dlta_x == 1)
        {
            target_coord.set_pos(prev_coord.pos_x() + dlta, src_coord.pos_y() + int(k_dlta));
        }
        else
        {
            target_coord.set_pos(src_coord.pos_x() + int(k_dlta), prev_coord.pos_y() + dlta);
        }
		if (this->is_movable_coord(target_coord) == false
				|| is_movable_coord_skill_push(target_coord) == false)
        {
        	offset_target_coord = coord_offset_grid(target_coord, src_coord);
        	if (is_ignore_middle == false || offset_target_coord > offset_ref)
        		break;
        }
        prev_coord = target_coord;
    }
    return prev_coord;
}

int GameFighter::process_push_away(const int range)
{
    GameFighter *defender = this->fetch_defender();
    JUDGE_RETURN(defender != 0, 0);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, 0);

    GameAI *game_ai = dynamic_cast<GameAI *>(defender);
    if (game_ai != 0)
    {
        if (game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) ||
                game_ai->test_ai_flag(GameEnum::AI_CF_NO_PUSH))
            return 0;
    }

    MoverCoord target_coord = defender->fetch_direct_point(this->location(), defender->location(), range, true);
    int offset_defender = coord_offset_grid(defender->location(), this->location()),
    		offset_target_coord = coord_offset_grid(target_coord, this->location());
    if (target_coord != defender->location() && offset_target_coord > offset_defender)
    {
    	defender->notify_fighter_teleport(this->fighter_id(), this->fight_skill_.__skill_id, target_coord);
    	scene->refresh_mover_location(defender, target_coord, false);
    }
    return 0;
}

int GameFighter::process_self_forward(const int range)
{
    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, 0);

    MoverCoord target_coord = this->fetch_direct_point(this->location(), this->fight_skill_.__skill_coord, range);

    this->notify_fighter_teleport(this->fighter_id(), this->fight_skill_.__skill_id, target_coord);

    scene->refresh_mover_location(this, target_coord, false);

    return 0;
}

int GameFighter::process_assault(const int org_range, const Json::Value &effect, GameFighter *calc_fighter)
{
    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, 0);

    if(calc_fighter == 0)
    	calc_fighter = this;

    int range = org_range;
    MoverCoord target_coord = calc_fighter->fetch_direct_point(this->location(), this->fight_skill_.__skill_coord, range), skill_coord = this->fight_skill_.__skill_coord;
    int offset_x = ::abs(this->location().pixel_x() - target_coord.pixel_x()), offset_y = ::abs(this->location().pixel_y() - target_coord.pixel_y());
    range = offset_x > offset_y ? offset_x : offset_y;

    double angle = vector_to_radian(this->location(), target_coord);
    MoverCoord center_coord;
    center_coord.set_pixel(this->location().pixel_x() + ::cos(angle) * range / 2, this->location().pixel_y() + ::sin(angle) * range / 2);
//    target_coord.set_pixel(this->location().pixel_x() + ::cos(angle) * range, this->location().pixel_y() + ::sin(angle) * range);

    this->notify_fighter_teleport(this->fighter_id(), this->fight_skill_.__skill_id, target_coord);

    const Json::Value &skill_json = CONFIG_INSTANCE->skill(this->fight_skill_.__skill_id);

    Scene::MoverMap mover_map;
    if (scene->fetch_all_rect_fighter(this, mover_map, center_coord, angle, 4 * 30, range, GameCommon::json_by_level(skill_json["object"], this->fight_skill_.__skill_level).asInt()) == 0)
    {
    	for (Scene::MoverMap::iterator iter = mover_map.begin(); iter != mover_map.end(); ++iter)
    	{
    		GameFighter *fighter = dynamic_cast<GameFighter *>(iter->second);
    		if (fighter == 0 || fighter->fighter_id() == this->fighter_id())
    			continue;

            this->fight_skill_.__defender_id = fighter->fighter_id();
            this->fight_skill_.effect_flag_reset();
            this->fight_skill_.__skill_coord = fighter->location();
    		int ret = this->validate_attack_target();
    		if (ret != 0)
    			continue;

    		const Json::Value &addtion_effect = effect["addition_effect"];
    		for (uint i = 0; i < addtion_effect.size(); ++i)
    		{
    			this->trigger_skill_effect_to_defender(addtion_effect[i]);
    		}
    		MSG_DEBUG("process assault %ld(%d,%d) -> %ld from(%d,%d) to(%d,%d)",
    				this->fighter_id(), this->location().pixel_x(), this->location().pixel_y(),
    				fighter->fighter_id(), fighter->location().pixel_x(), fighter->location().pixel_y(),
    				target_coord.pixel_x(), target_coord.pixel_y());

    		fighter->notify_fighter_teleport(this->fighter_id(), this->fight_skill_.__skill_id, target_coord);
    		scene->refresh_mover_location(fighter, target_coord, false);
    	}
    }
    this->fight_skill_.__skill_coord = skill_coord;

    scene->refresh_mover_location(this, target_coord, false);

    return 0;
}

int GameFighter::process_self_backward(const int range)
{
    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, 0);

    MoverCoord refrence_point;
    int dx = this->location().pixel_x() - this->fight_skill_.__skill_coord.pixel_x(),
        dy = this->location().pixel_y() - this->fight_skill_.__skill_coord.pixel_y();
    double k = 0.0;
    int dlta = 0, inc_unit = 30;
    if (std::abs(dy) > std::abs(dx))
    {
        dlta = (dy < 0 ? -inc_unit : inc_unit);
        if (dy != 0)
            k = double(dx) / dy * inc_unit;

        refrence_point.set_pixel(this->location().pixel_x() + k, this->location().pixel_y() + dlta);
    }
    else
    {
        dlta = (dx < 0 ? -inc_unit : inc_unit);
        if (dx != 0)
            k = double(dy) / dx * inc_unit;

        refrence_point.set_pixel(this->location().pixel_x() + dlta, this->location().pixel_y() + k);
    }

    MoverCoord target_coord = this->fetch_direct_point(this->location(), refrence_point, range);

    this->notify_fighter_teleport(this->fighter_id(), this->fight_skill_.__skill_id, target_coord);
    scene->refresh_mover_location(this, target_coord, false);
    return 0;
}

int GameFighter::notify_fighter_teleport(const Int64 attackor_id, const int skill_id, const MoverCoord &target_coord)
{
	Proto80400225 respond;
	respond.set_mover_id(this->fighter_id());
	respond.set_pixel_x(target_coord.pixel_x());
	respond.set_pixel_y(target_coord.pixel_y());
	respond.set_attack_id(0);		// 置为0, 不能是攻击者ＩＤ，不然客户端不播放
	return this->respond_to_broad_area(&respond);
}

int GameFighter::mutual_skill(void)
{
    return this->mutual_skill_.__skill;
}

void GameFighter::stop_guide_skill(void)
{ /*NULL; for AutoMapFighter*/ }

void GameFighter::stop_guide_skill_by_skill(const int skill_id)
{ /*NULL; for AutoMapFighter*/ }

void GameFighter::stop_last_skill(const int status)
{
}

int GameFighter::process_launch_mutual_skill(const Time_Value &nowtime)
{
    return 0;
}


int GameFighter::notify_cancel_loop_skill(const int skill_id, const int stop_by_buff)
{
	JUDGE_RETURN(skill_id > 0, -1);

    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
    JUDGE_RETURN(skill_json["no_notify_mutual_exit"].asInt() == 0, -1);

    Proto80400113 respond;
    respond.set_mover_id(this->fighter_id());
    respond.set_skill_id(skill_id);
    respond.set_stop_by_buff(stop_by_buff);
    return this->respond_to_broad_area(&respond);
}

int GameFighter::process_launch_loop_skill(const Time_Value &nowtime)
{
	for (int i = 0; i < TOTAL_LOOP; ++i)
	{
		this->process_launch_loop_skill(this->loop_skill_[i], nowtime);
	}
    return 0;
}

int GameFighter::process_launch_loop_skill(LoopSkillDetail& loop_skill, const Time_Value &nowtime)
{
	JUDGE_RETURN(loop_skill.__skill > 0, -1);
	JUDGE_RETURN(loop_skill.__timeout != Time_Value::zero, -1);

    int loop = 0;
    while (loop_skill.__next <= nowtime && loop_skill.__next <= loop_skill.__timeout && (++loop) < 50)
    {
    	MSG_DEBUG("loop skill %d %d next(%ld.%06d) interval(%ld.%06d) timeout(%ld.%06d)",
    			loop_skill.__skill, loop_skill.__cnt,
    			loop_skill.__next.sec(), loop_skill.__next.usec(),
    			loop_skill.__interval.sec(), loop_skill.__interval.usec(),
    			loop_skill.__timeout.sec(), loop_skill.__timeout.usec());

    	++loop_skill.__cnt;

		FighterSkill *fight_skill = 0;
		JUDGE_CONTINUE(this->find_skill(loop_skill.__skill, fight_skill) == 0);

    	const Json::Value &skill_json = fight_skill->conf();
    	double prev_time = GameCommon::json_by_level(skill_json["last_time"], loop_skill.__cnt).asDouble();
    	double cur_time = GameCommon::json_by_level(skill_json["last_time"], loop_skill.__cnt + 1).asDouble();

    	double span_time = std::max<double>(cur_time - prev_time, 0.1);
    	loop_skill.__interval = Time_Value::gettime(span_time);
    	loop_skill.__next += loop_skill.__interval;

        if (loop_skill.__cnt > 50)
        {
        	break;
        }

        MoverCoord skill_coord;
        if (loop_skill.__skill_coord_type == GameEnum::SKILL_COORD_T_FIXED)
        {
            skill_coord = loop_skill.__fixed_skill_coord;
        }
        else
        {
            skill_coord = this->location();
        }

        Proto10400202 launch_req;
        launch_req.set_skill_id(loop_skill.__skill);
        launch_req.set_skill_step(0);

        double radian = loop_skill.__radian;
        this->fight_skill_.set_skill_id(this, loop_skill.__skill);
        this->fight_skill_.set_radian(radian);
        this->fight_skill_.__skill_coord = skill_coord;
        this->fight_skill_.__client_target = loop_skill.__client_target;

        Scene::MoverMap fighter_map;
        this->make_up_skill_target(fighter_map, loop_skill.__skill, skill_coord, this->fight_skill_.__angle);

        launch_req.set_angle(this->fight_skill_.__radian);
        launch_req.set_target_id(loop_skill.__client_target);
        launch_req.set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
        launch_req.set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());

        MSG_DEBUG("loop skill angle %ld %d %d angle(%f %f)", this->fighter_id(), loop_skill.__skill,
        		fighter_map.size(), radian * 57.3, this->fight_skill_.__angle);

        {
//			int skill_level = fight_skill->__level;
//			int max_object = fight_skill->__object;

			Proto80400220 respond;
			respond.set_fighter_id(this->fighter_id());
			respond.set_skill_id(loop_skill.__skill);
			respond.set_skill_level(fight_skill->__level);
			respond.set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
			respond.set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());

			for (Scene::MoverMap::iterator iter = fighter_map.begin();
					iter != fighter_map.end(); ++iter)
			{
				GameFighter* fighter = dynamic_cast<GameFighter *>(iter->second);
				JUDGE_CONTINUE(GameCommon::validate_fighter(fighter) == true);

				this->fight_skill_.__defender_id = fighter->fighter_id();
				JUDGE_CONTINUE(this->validate_launch_attack_target() == 0);

				launch_req.add_target_list(iter->first);
				respond.add_target_list()->set_target_id(iter->first);
			}

			this->respond_to_broad_area(&respond);
        }

		if (launch_req.target_list_size() > 0)
		{
			this->launch_fight_skill(&launch_req, false);
		}
    }

    if (loop_skill.__timeout < nowtime)
    {
    	this->process_skill_post_launch(loop_skill.__skill);
    	this->notify_cancel_loop_skill(loop_skill.__skill);
    	loop_skill.reset();
    }

    return 0;
}

int GameFighter::alive_summon_ai_size(void)
{
	return 0;
}

int GameFighter::insert_summon_ai(const Int64 ai_id)
{
    return 0;
}

int GameFighter::remove_summon_ai(const Int64 ai_id)
{
    return 0;
}

void GameFighter::recycle_all_skill_map(void)
{
	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
	        iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
	    this->monitor()->skill_pool()->push(iter->second);
	}

	this->fight_detail_.__skill_map.clear();
}

int GameFighter::process_area_hurt(const Json::Value& effect, int aoe_type)
{
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(this->fight_skill_.__skill_id);

	if (effect.isMember("player_point") && effect["player_point"].asInt() == 1)
	{
		aoe_type = GameEnum::SKILL_AOE_ALL_PLAYER;
	}

	IntVec monster_sort_vec;
	if(effect["sort"].isArray())
	{
		for (uint i = 0; i < effect["sort"].size(); ++i)
			monster_sort_vec.push_back(effect["sort"][i].asInt());
	}
	else
		monster_sort_vec.push_back( effect["sort"].asInt());

	switch(aoe_type)
	{
	case GameEnum::SKILL_AOE_SELF_CIRCLE:
	{
		Scene* scene = this->fetch_scene();
		JUDGE_RETURN(scene != 0, -1);

		int radius = GameCommon::json_by_level(skill_json["radius"], this->fight_skill_.__skill_level).asInt();

		MoverCoord gen_coord;
		int sort = 0;
		for (std::vector<int>::iterator iter = monster_sort_vec.begin();
				iter != monster_sort_vec.end(); iter++)
	    {
			if(effect.isMember("max_exists"))
			{
				int max_exist = effect["max_exists"].asInt();

				int monster_sort = *iter;
				Scene* scene = this->fetch_scene();
				JUDGE_CONTINUE(scene != 0);

				int total_monster_n = 0;
				Scene::MoverMap &mover_map = scene->mover_map();
				for(Scene::MoverMap::iterator iter = mover_map.begin(); iter !=mover_map.end(); ++iter)
				{
					GameMover* game_mover = iter->second;
					JUDGE_CONTINUE(game_mover != 0);

					GameAI* other_ai = dynamic_cast<GameAI*>(game_mover);
					JUDGE_CONTINUE(other_ai != 0);
					JUDGE_CONTINUE(other_ai->ai_detail().__sort == monster_sort);
					total_monster_n ++;
				}

				JUDGE_CONTINUE(total_monster_n < max_exist);
			}

			sort = *iter;
			gen_coord = this->location();
			if(radius > 0)
				gen_coord = scene->rand_coord(this->location(), radius, this);
			MSG_DEBUG("center: %d,%d gen_coord: %d,%d", this->location().pixel_x(), this->location().pixel_y()
					,gen_coord.pixel_x(), gen_coord.pixel_y());
			int ret = this->generate_area_hurt_monster(sort, gen_coord, 0);
			JUDGE_RETURN(ret == 0, ret);
		}
		return 0;
	}
	case GameEnum::SKILL_AOE_SELF_RECT:
	{
		double radian = this->fight_skill_.__radian;
		double height = GameCommon::json_by_level(skill_json["height"], this->fight_skill_.__skill_level).asDouble() * 30;

		MoverCoord center;
		center.set_pixel(this->location().pixel_x() + height / 2.0 * ::cos(radian),
				this->location().pixel_y() + height / 2.0 * ::sin(radian));

		for (IntVec::iterator iter = monster_sort_vec.begin();
				iter != monster_sort_vec.end(); iter++)
	    {
		    this->generate_area_hurt_monster(*iter, center, this->fight_skill_.__angle);
        }
        return 0;
	}
	case GameEnum::SKILL_AOE_SELF_TARGET:
	{
		Scene* scene = this->fetch_scene();
		JUDGE_RETURN(scene != 0, -1);

		MoverCoord gen_coord;
		int sort = 0;
		for (std::vector<int>::iterator iter = monster_sort_vec.begin();
				iter != monster_sort_vec.end(); iter++)
	    {
			sort = *iter;
			gen_coord = this->location();
			MSG_DEBUG("center: %d,%d gen_coord: %d,%d", this->location().pixel_x(), this->location().pixel_y()
					,gen_coord.pixel_x(), gen_coord.pixel_y());
			int ret = this->generate_area_hurt_monster(sort, gen_coord, 0);
			JUDGE_RETURN(ret == 0, ret);
		}
		return 0;
	}
    case GameEnum::SKILL_AOE_ALL_PLAYER:
    {
        Scene *scene = this->fetch_scene();
        JUDGE_RETURN(scene != NULL, 0);

        int sort = 0;
        Scene::MoverMap &player_map = scene->player_map();
        for (Scene::MoverMap::iterator iter = player_map.begin(); iter != player_map.end(); ++iter)
        {
            GameMover *mover = iter->second;
            GameFighter *fighter = dynamic_cast<GameFighter *>(mover);
            if (fighter == NULL || fighter->is_enter_scene() == false || fighter->is_death())
                continue;

		    for (std::vector<int>::iterator iter = monster_sort_vec.begin();
                    iter != monster_sort_vec.end(); iter++)
	        {
			    sort = *iter;
                this->generate_area_hurt_monster(sort, fighter->location(), 0);
            }
        }
        return 0;
    }
	default:
		MSG_USER("!!!!!! error area_hurt AOE type, %d", aoe_type);
		return -1;
	}
	return 0;
}

int GameFighter::process_skill_freeze(const Json::Value& skill_json)
{
	if(!skill_json.isMember("freeze_last"))
	{
		if (skill_json.isMember("freeze_skills")
				|| skill_json.isMember("freeze_skills_exclude"))
		{
			MSG_DEBUG("Error!!!!! 'freeze_last' is not set, skill_json:%s",
					skill_json.toStyledString().c_str());
			return ERROR_CONFIG_ERROR;
		}
		return 0;
	}

	SkillFreezeDetail detail;
	detail.__trigger_skill = this->fight_skill_.__skill_id;
    detail.__timeout = GameCommon::fetch_add_time_value(skill_json["freeze_last"].asDouble());

	if (skill_json.isMember("freeze_skills"))
	{
		for(uint i=0; i < skill_json["freeze_skills"].size(); i++)
		{
			int skill_id = skill_json["freeze_skills"][i].asInt();
			detail.__skills.insert(skill_id);
		}
	}

	if (skill_json.isMember("freeze_skills_exclude"))
	{
		IntSet exclude_skills;
		for (uint i=0; i<skill_json["freeze_skills_exclude"].size(); i++)
		{
			int skill_id = skill_json["freeze_skills_exclude"][i].asInt();
			exclude_skills.insert(skill_id);

			JUDGE_CONTINUE(detail.__skills.find(skill_id) != detail.__skills.end());
			detail.__skills.erase(skill_id);
		}

		for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
				iter != this->fight_detail_.__skill_map.end(); ++iter)
		{
			JUDGE_CONTINUE(exclude_skills.find(iter->first) == exclude_skills.end());
			detail.__skills.insert(iter->first);
		}
	}

	if (detail.__skills.size() > 0)
	{
		this->frozen_skills_.insert(detail.__skills.begin(), detail.__skills.end());
		this->skill_freeze_map_.insert(
				SkillFreezeMap::value_type(detail.__trigger_skill, detail));
	}
	return 0;
}

int GameFighter::check_skill_freeze_timeout(const Time_Value &nowtime)
{
    this->frozen_skills_.clear();
	for (SkillFreezeMap::iterator iter = this->skill_freeze_map_.begin();
			iter != this->skill_freeze_map_.end(); )
	{
		SkillFreezeMap::iterator curr_iter = iter++;
		SkillFreezeDetail &detail =  curr_iter->second;
		if (nowtime >= detail.__timeout)
			this->skill_freeze_map_.erase(curr_iter);
		else
			this->frozen_skills_.insert(detail.__skills.begin(), detail.__skills.end());
	}
	return 0;
}

int GameFighter::process_skill_post_effect(int skill_id)
{
	return 0;
}

int GameFighter::process_skill_post_launch(int skill_id, int source)
{
	return 0;
}

int GameFighter::process_skill_note(int skill_id, const Json::Value& detail_json)
{
	// AutoMapFighter::process_skill_note()
	return 0;
}

int GameFighter::process_player_force_up(int force)
{
	return 0;
}

void GameFighter::insert_attack_me(Int64 role_id)
{
	return ;
}

int GameFighter::correct_add_blood_by_status(const int add_value)
{
	int reduce_value = add_value;
	BasicStatus *status = 0;
	if (this->find_status(BasicStatus::BLOOD_BACK_REDUCE, status) == 0)
	{
		if (std::abs(status->__value1) > 0.0000001)
			reduce_value -= status->__value1;
		if (std::abs(status->__value2) > 0.0000001)
			reduce_value -= (add_value * (status->__value2 / 100.0));
		if (reduce_value <= 0)
			reduce_value = 1;
	}
	return reduce_value;
}

int GameFighter::team_notify_teamer_blood(int type)
{
	return 0;
}

Int64 GameFighter::fetch_benefited_attackor_id(Int64 attackor_id)
{
	Int64 benefit_attackor_id = attackor_id;

	GameFighter *fighter = NULL;
	JUDGE_RETURN(this->find_fighter(attackor_id, fighter) == 0, benefit_attackor_id);

	if (fighter->is_beast())
	{
		MapBeast *beast = dynamic_cast<MapBeast *>(fighter);
		if (beast != 0 && beast->master_id() > 0)
		{
			benefit_attackor_id = beast->master_id();
		}
	}
	else if (fighter->is_monster())
	{
		GameAI *fighter_ai = dynamic_cast<GameAI *>(fighter);
		if (fighter_ai != 0 && fighter_ai->caller() > 0)
		{
			benefit_attackor_id = fighter_ai->caller();
		}
	}

	return benefit_attackor_id;
}

int GameFighter::notify_exit_scene_cancel_info(int type, int scene_id)
{
	for (int i = 0; i < TOTAL_LOOP; ++i)
	{
		LoopSkillDetail& loop_skill = this->loop_skill_[i];
		this->notify_cancel_loop_skill(loop_skill.__skill);
		loop_skill.reset();
	}

    return this->clear_status(StatusQueueNode::EXIT_SCENE_REMOVE);
}

int GameFighter::process_rate_use_auto_skill_in_fight(FighterSkill* src_skill)
{
	JUDGE_RETURN(src_skill->is_active_skill() == true, -1);
	JUDGE_RETURN(GameCommon::is_jump_skill(src_skill) == false, -1);

	PassiveSkillInfoVec skill_vc;
	Time_Value nowtime = Time_Value::gettimeofday();

    PassiveSkillInfo *pass_info = 0;
    while ((pass_info = this->fight_detail_.__fight_passive_skill_queue.top()) != 0)
    {
        if (pass_info->__launch_tick > nowtime)
        {
            break;
        }

        this->fight_detail_.__fight_passive_skill_queue.pop();

        skill_vc.push_back(pass_info);
        JUDGE_CONTINUE(pass_info->__skill != NULL);

        this->process_launch_rate_use_skill(pass_info->__skill);
        pass_info->__launch_tick = pass_info->__skill->__use_tick;
    }

    for (PassiveSkillInfoVec::iterator iter = skill_vc.begin();
            iter != skill_vc.end(); ++iter)
    {
        this->fight_detail_.__fight_passive_skill_queue.push(*iter);
    }
	return 0;
}

int GameFighter::process_launch_rate_use_skill(FighterSkill *skill)
{
    JUDGE_RETURN(this->level() >= skill->__use_level, -1);

    this->fight_skill_.reset();
    this->fight_skill_.set_skill_id(this, skill->__skill_id);

    switch (skill->__aoe_type)
    {
    case GameEnum::SKILL_AOE_CUR_AIM_TARGET:
    {
    	this->fight_skill_.__defender_id = this->fight_skill_.__client_target;
    	break;
    }
    case GameEnum::SKILL_AOE_SELF_TARGET:
    {
    	this->fight_skill_.__defender_id = this->mover_id();
    	break;
    }
    default:
    {
    	return -1;
    }
    }

    Int64 target_id = this->fight_skill_.__defender_id;
    GameFighter* target = this->fetch_defender();
    if (target != NULL)
    {
    	this->fight_skill_.__skill_coord = target->location();
    }

    int use_rate = this->fetch_passive_skill_use_rate(skill);
    JUDGE_RETURN(GameCommon::validate_cur_rand(use_rate) == true, -1);
    JUDGE_RETURN(this->validate_launch_attack_target() == 0, -1);

    std::auto_ptr<Proto10400202> launch_req(new Proto10400202());
    launch_req->set_skill_id(skill->__skill_id);
    launch_req->set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
    launch_req->set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());
    launch_req->set_target_id(target_id);
	launch_req->add_target_list(target_id);

    this->refresh_skill_info();
    this->notify_launch_skill(skill);
    this->process_skill_launch_way(skill, launch_req);
    return 0;
}

int GameFighter::notify_fight_passive_skill_talisman()
{
	return 0;
}

int GameFighter::notify_fight_passive_skill(void)
{
	return 0;
}

int GameFighter::process_rate_use_skill_by_hurt(GameFighter *attacker)
{
    JUDGE_RETURN(this != attacker, 0);

    Time_Value nowtime = Time_Value::gettimeofday();
    std::vector<PassiveSkillInfo *> skill_vc;
    PassiveSkillInfo *pass_info = 0;

    while ((pass_info = this->fight_detail_.__hurt_passive_skill_queue.top()) != 0)
    {
        if (pass_info->__launch_tick > nowtime)
            break;

        this->fight_detail_.__hurt_passive_skill_queue.pop();

        skill_vc.push_back(pass_info);
        JUDGE_CONTINUE(pass_info->__skill != NULL);

        this->process_launch_use_skill_by_hurt(attacker, pass_info->__skill);
        pass_info->__launch_tick = pass_info->__skill->__use_tick;
    }

    for (std::vector<PassiveSkillInfo *>::iterator iter = skill_vc.begin();
            iter != skill_vc.end(); ++iter)
    {
        this->fight_detail_.__hurt_passive_skill_queue.push(*iter);
    }

    return 0;
}

int GameFighter::process_launch_use_skill_by_hurt(GameFighter *attacker, FighterSkill *skill)
{
    const Json::Value &skill_json = skill->conf();
    int skill_level = skill->__level;
    if (this->level() < GameCommon::json_by_level(skill_json["useLvl"], skill_level).asInt())
    	return 0;

    if (skill_json.isMember("use_rate"))
    {
        int rand_val = rand() % 10000;
        if (rand_val >= GameCommon::json_by_level(skill_json["use_rate"], skill_level).asDouble() * 100)
            return 0;
    }

    Int64 sel_defender_id = attacker->fighter_id();
    this->fight_skill_.reset();
    this->fight_skill_.__sel_defender = attacker->fighter_id();
    this->fight_skill_.__skill_id = skill->__skill_id;
    this->fight_skill_.__display_skill = skill->__skill_id;
    this->fight_skill_.__skill_level = skill->__level;
    this->fight_skill_.__skill_step = 0;
    this->fight_skill_.__skill_coord.set_pixel(attacker->location().pixel_x(), attacker->location().pixel_y());
    this->refresh_skill_info();

    std::auto_ptr<Proto10400202> launch_req(new Proto10400202());
    launch_req->set_skill_id(skill->__skill_id);
    launch_req->set_skill_pixel_x(this->fight_skill_.__skill_coord.pixel_x());
    launch_req->set_skill_pixel_y(this->fight_skill_.__skill_coord.pixel_y());
    launch_req->set_target_id(sel_defender_id);
    launch_req->set_skill_step(0);
    launch_req->add_target_list(sel_defender_id);

    const std::string &type_str = skill_json["type"].asString();
    if (type_str.length() > GameEnum::SKILL_TARGET_SELF && type_str[GameEnum::SKILL_TARGET_SELF] == '1')
    {
    	this->fight_skill_.__defender_set.insert(this->mover_id());
    	launch_req->add_target_list(this->mover_id());
    }

    if (GameCommon::json_by_level(skill_json["object"], skill_level).asInt() > 1)
    {
        MoverMap fighter_map;
        this->fight_skill_.__defender_set.clear();
        this->make_up_skill_target(fighter_map, skill->__skill_id, this->fight_skill_.__skill_coord);
        for (MoverMap::iterator iter = fighter_map.begin(); iter != fighter_map.end(); ++iter)
        {
            JUDGE_CONTINUE(iter->first != this->fighter_id());

            this->fight_skill_.__defender_set.insert(iter->first);
            launch_req->add_target_list(iter->first);
        }
    }

    this->process_skill_launch_way(skill, launch_req);
    return 0;
}

int GameFighter::process_skill_launch_way(FighterSkill *skill, std::auto_ptr<Proto10400202> launch_req)
{
    Time_Value nowtime = Time_Value::gettimeofday();
	const Json::Value &skill_json = skill->conf();

    int skill_level = skill->__level;
    if (skill->__is_mutual == 1)
    {
    	//互斥触发技能
    	int total_size = skill_json["last_time"].size();
    	JUDGE_RETURN(total_size > 1, -1);

    	this->mutual_skill_.reset();

        this->mutual_skill_.__cnt = 0;
        this->mutual_skill_.__skill = skill->__skill_id;
        this->mutual_skill_.__radian = this->fight_skill_.__radian;

        double wait_tick = skill_json["last_time"][0u].asDouble();
        this->mutual_skill_.__interval = GameCommon::fetch_time_value(wait_tick);
        this->mutual_skill_.__next = nowtime + this->mutual_skill_.__interval;

        double mutual_last = skill_json["last_time"][total_size - 1].asDouble();
        this->mutual_skill_.__timeout = nowtime + GameCommon::fetch_time_value(mutual_last);

        if (skill_json["mutual_fixed_point"].asInt() == 1)
        {
            this->mutual_skill_.__skill_coord_type = GameEnum::SKILL_COORD_T_FIXED;
            this->mutual_skill_.__fixed_skill_coord = this->fight_skill_.__skill_coord;
        }
    }
    else if (skill->__is_loop == true)
    {
    	// 循环触发技能
    	int total_size = skill_json["last_time"].size();
    	JUDGE_RETURN(total_size >= 1, -1);

    	int cur_index = this->loop_index_ % TOTAL_LOOP;
    	this->loop_index_ += 1;

    	LoopSkillDetail& cur_loop = this->loop_skill_[cur_index];
    	cur_loop.reset();

    	cur_loop.__cnt = 0;
    	cur_loop.__skill = skill->__skill_id;
    	cur_loop.__radian = this->fight_skill_.__radian;
    	cur_loop.__client_target = this->fight_skill_.__client_target;

        double wait_tick = skill_json["last_time"][0u].asDouble();
        cur_loop.__interval = GameCommon::fetch_time_value(wait_tick);
        cur_loop.__next = nowtime + cur_loop.__interval;

        double mutual_last = skill_json["last_time"][total_size - 1].asDouble();
        cur_loop.__timeout = nowtime + GameCommon::fetch_time_value(mutual_last);

        if (skill_json["loop_fixed_point"].asInt() == 1)
        {
        	cur_loop.__skill_coord_type = GameEnum::SKILL_COORD_T_FIXED;
        	cur_loop.__fixed_skill_coord = this->fight_skill_.__skill_coord;
        }
    }
    else
    {
        double launch_wait_tick = 0;
        const Json::Value &launch_wait_json = GameCommon::json_by_level(skill_json["launch_wait"], skill_level);

        if (launch_wait_json.empty() == false && launch_wait_json.isArray())
        {
            int index = (this->fight_sex() - 1) % launch_wait_json.size();
            launch_wait_tick = launch_wait_json[index].asDouble();
        }
        else
        {
            launch_wait_tick = launch_wait_json.asDouble();
        }

        if (GameCommon::is_zero(launch_wait_tick) == false)
        {
        	// 等待播放完特效才扣血
            Time_Value delay_tick = nowtime;
            delay_tick.usec(delay_tick.usec() / 100000 * 100000);
            delay_tick += GameCommon::fetch_time_value(launch_wait_tick);

            DelaySkillInfo *skill_ptr = this->monitor()->delay_skill_pool()->pop();
            skill_ptr->__launch_tick = delay_tick;
            skill_ptr->__request = launch_req;
            this->skill_delay_launch_queue_.push(skill_ptr);
        }
        else
        {
        	// 立即触发扣血
            this->launch_fight_skill(launch_req.get());
            this->process_skill_post_launch(skill->__skill_id, 0);
        }
    }

    return 0;
}

void GameFighter::remove_die_protect(void)
{
	JUDGE_RETURN(this->is_player() == true, ;);

	BasicStatus *status = NULL;
	JUDGE_RETURN(this->find_status(BasicStatus::RELIVE_PROTECT, status) == 0, ;);

    this->clean_status(BasicStatus::RELIVE_PROTECT);
    this->notify_update_player_info(GameEnum::PLAYER_INFO_PROTECT);
}

int GameFighter::monster_hurt_reduce(GameFighter *defender, double &real_hurt)
{
    GameAI *defender_ai = dynamic_cast<GameAI *>(defender);
    JUDGE_RETURN(defender_ai != NULL, 0);

    const Json::Value &monster_json = CONFIG_INSTANCE->monster(defender_ai->ai_sort());
    JUDGE_RETURN(monster_json.isMember("hurt_tipping") && monster_json.isMember("hurt_ratio"), 0);

    double hurt_tipping = 0.0, hurt_ratio = 0.0;
    if (monster_json.isMember("hurt_tipping") == true)
        hurt_tipping = monster_json["hurt_tipping"].asDouble();
    if (monster_json.isMember("hurt_ratio") == true)
        hurt_ratio = monster_json["hurt_ratio"].asDouble();
    if (real_hurt <= hurt_tipping)
        return 0;
   
    real_hurt = hurt_tipping + (real_hurt - hurt_tipping) * hurt_ratio;
    return 0;
}

int GameFighter::check_and_die_away_safe_range(GameFighter *defender)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    if (scene_json["exec"].isMember("safe_range"))
    {
        JUDGE_RETURN(defender != NULL && defender->is_player() && defender->is_death() == false, 0);

        const Json::Value &safe_range_json = scene_json["exec"]["safe_range"];
        MoverCoord center;
        center.set_pixel(safe_range_json["point"][0u].asInt(), safe_range_json["point"][1u].asInt());
        int range = safe_range_json["range"].asInt();

        int offset_x = defender->location().pixel_x() - center.pixel_x();
        int offset_y = GameEnum::CLIENT_MAPPING_FACTOR * (defender->location().pixel_y() - center.pixel_y());
        if ((offset_x * offset_x + offset_y * offset_y) > ((range + 1) * (range + 1)))
        {
        	// 进入非安全区域，直接死亡
            defender->modify_blood_by_fight(defender->fight_detail().__blood_total_i(defender));
        }
    }
    return 0;
}

int GameFighter::check_enter_stiff(const int skill_id, const int skill_level)
{
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);

    double stiff_sec = GameCommon::json_by_level(skill_json["defender_stiff"], skill_level).asDouble();
    JUDGE_RETURN(stiff_sec > 0.000001, 0);

    return this->insert_defender_status(this, BasicStatus::STIFF, 0.0, stiff_sec);
}

int GameFighter::process_magic_reduce_hurt(GameFighter *attackor, const double value, const double percent)
{
    int reduce_hurt = int(this->fight_detail().__magic * (percent / 100.0));

    if (reduce_hurt > 0)
    	this->modify_magic_by_notify(reduce_hurt, FIGHT_TIPS_NORMAL);

    reduce_hurt += value;
    attackor->current_skill().__skill_hurt -= reduce_hurt;
    return 0;
}

int GameFighter::update_skill_level(FighterSkill *skill, const int target_level)
{
    this->monitor()->update_skill_level(skill, target_level);
    return 0;
}

int GameFighter::process_insert_shield_effect(GameFighter *defender, const Json::Value &effect)
{
    double percent = effect["percent"].asDouble();
    double value = defender->fight_detail().__blood_total(defender) * GameCommon::div_percent(percent);

    int break_hurt_range = effect["break_hurt_range"].asInt();
    return this->insert_defender_status(defender, BasicStatus::SHIELD,
            0, effect["last"].asDouble(), effect["accumulate_times"].asInt(),
            value, percent, break_hurt_range, 0);
}

int GameFighter::process_insert_life_shield_effect(GameFighter *defender, const Json::Value &effect)
{
    return this->insert_defender_status(defender, effect["id"].asInt(), 0, effect["last"].asDouble(),
    		0, effect["value"].asInt(), effect["per_value"].asInt());
}

double GameFighter::calc_shield_reduce_value(double inc_value, BasicStatus *status)
{
	double reduce_value = std::min(status->__value1, status->__value2);
    status->__value5 += reduce_value;
    status->__value1 -= reduce_value;

    {
    	Scene *scene = this->fetch_scene();
    	if (scene != NULL)
    	{
    		scene->notify_shield_info(status);
    	}
    }

    if (GameCommon::is_zero(status->__value1) == true)
    {
    	Scene *scene = this->fetch_scene();
    	if (scene != NULL)
    	{
    		WorldBossScene *wboss = dynamic_cast<WorldBossScene*>(scene);
    		if (wboss != NULL)
    		{
    			wboss->notify_dice_announce();
    			wboss->notify_play_dice();
    		}
    	}
        this->remove_status(status);
    }
    return reduce_value;
}

double GameFighter::calc_shield_reduce_value_b(double inc_value, BasicStatus *status)
{
	if (status->__flag == false)
	{
		status->__flag = true;
		status->__value4 = status->__value1;
	}

	status->__value1 -= inc_value;
	status->__value1 = std::max<double>(0, status->__value1);

	if (GameCommon::is_zero(status->__value1) == true)
	{
		this->remove_status(status);
	}

	return inc_value;
}

double GameFighter::calc_shield_reduce_value_c(double inc_value, BasicStatus* status, Int64 attackor)
{
	GameFighter* fighter = NULL;
	BasicStatus* through_status = NULL;

	double adjust_value = status->__value2;
	if (this->find_fighter(attackor, fighter) == 0
			&& fighter->find_first_status(BasicStatus::FIGHT_THROUGH, through_status) == 0)
	{
		adjust_value *= GameCommon::div_percent(through_status->__value2);
	}

	status->__value1 -= adjust_value;
	status->__value1 = std::max<int>(0, status->__value1);

	if (GameCommon::is_zero(status->__value1) == true)
	{
		this->remove_status(status);
	}

	return adjust_value;
}

int GameFighter::launch_shield_hurt(BasicStatus *status)
{
	double break_hurt_range = status->__value3;
	double hurt = status->__value5;
    JUDGE_RETURN(this->is_death() == false && this->is_enter_scene() == true
    		&& this->is_in_safe_area() == false && this->is_jumping() == false, -1);
    
    MapPlayerEx *attack_player = dynamic_cast<MapPlayerEx *>(this);
    JUDGE_RETURN(attack_player != NULL, -1);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    MoverMap fighter_map;
    scene->fetch_all_around_fighter(this, fighter_map, this->location(), break_hurt_range, 30);

    Proto80400220 respond;
    respond.set_fighter_id(this->fighter_id());
    respond.set_skill_id(status->__skill_id);
    respond.set_skill_level(status->__level);
    respond.set_skill_pixel_x(this->location().pixel_x());
    respond.set_skill_pixel_y(this->location().pixel_y());

    std::vector<GameFighter *> hurt_fighter_vc;
    for (MoverMap::iterator iter = fighter_map.begin(); iter != fighter_map.end(); ++iter)
    {
        JUDGE_CONTINUE(iter->first != this->fighter_id());

        GameFighter *fighter = dynamic_cast<GameFighter *>(iter->second);
        JUDGE_CONTINUE(GameCommon::validate_fighter(fighter) == true);
        JUDGE_CONTINUE(this->validate_attack_to_defender(fighter, this->base_skill_id()) == 0);

        hurt_fighter_vc.push_back(fighter);
        respond.add_target_list()->set_target_id(fighter->fighter_id());
    }
    this->respond_to_broad_area(&respond);

    for (std::vector<GameFighter *>::iterator iter = hurt_fighter_vc.begin();
    		iter != hurt_fighter_vc.end(); ++iter)
    {
    	this->hurt_no_defence(*iter, hurt);
    }
    return 0;
}

int GameFighter::add_and_notify_angry_value(int value)
{
	JUDGE_RETURN(this->is_player() == true, -1);

	if (value > 0)
	{
		JUDGE_RETURN(this->fight_detail_.__angry < GameEnum::MAX_ANGRY, -1);
	}

	this->fight_detail_.__angry += value;
	this->fight_detail_.__angry = std::min<int>(GameEnum::MAX_ANGRY,
			this->fight_detail_.__angry);

	return this->notify_fight_update(FIGHT_UPDATE_ANGRY, this->fight_detail_.__angry);
}

int GameFighter::hurt_no_defence(GameFighter *defender, const double hurt)
{
    if (this->is_hit_in_hurt(defender) == false)
    {
        defender->notify_fight_update(FIGHT_UPDATE_BLOOD, 0, this->fighter_id(), 0,
        		defender->fight_detail().__blood, FIGHT_TIPS_MISS, 0);
    }
    else
    {
        int fight_tips = FIGHT_TIPS_NORMAL;

        double crit_times = this->crit_power_in_hurt(defender);
        if (crit_times > 1)
        {
            fight_tips = FIGHT_TIPS_CRIT;
        }

        double real_hurt = std::max<double>(hurt * crit_times, 1);
        defender->modify_blood_by_fight(real_hurt, fight_tips, this->fighter_id(), 0);
    }
    return 0;
}

int GameFighter::process_insert_percy_effect(GameFighter *defender, const Json::Value &effect)
{
	return 0;
}

int GameFighter::process_insert_flash_avoid_effect(GameFighter *defender, const Json::Value &effect)
{
    double avoid_percent = effect["percent"].asDouble(),
           hurt_rate = effect["hurt_rate"].asDouble(),
           last = effect["last"].asDouble();
    return this->insert_defender_status(defender, BasicStatus::FLASH_AVOID,
            0.0, last, 0, 0, avoid_percent, hurt_rate);
}

int GameFighter::calc_rebound_hurt_when_avoid(GameFighter *attacker)
{
    BasicStatus *status = NULL;
    if (this->find_status(BasicStatus::FLASH_AVOID, status) == 0)
    {
    	Proto80400220 respond;
    	respond.set_fighter_id(this->fighter_id());
    	respond.set_skill_id(status->__skill_id);
    	respond.set_skill_level(status->__level);
    	respond.set_skill_pixel_x(attacker->location().pixel_x());
    	respond.set_skill_pixel_y(attacker->location().pixel_y());
    	respond.add_target_list()->set_target_id(attacker->fighter_id());
    	this->respond_to_broad_area(&respond);

        double hurt = this->fight_detail().__avoid_total(this) * status->__value3;
        attacker->modify_blood_by_fight(hurt, FIGHT_TIPS_NORMAL, this->fighter_id(), status->__skill_id);
    }

    return 0;
}

int GameFighter::insert_master_status(const int status,
            const double interval, const double last, const int accumulate_times,
    		const double val1, const double val2, const double val3,
            const double val4, const double val5)
{
	return 0;
}

int GameFighter::process_monster_talk(int skill_id, int mover_id)
{
	return 0;
}

int GameFighter::process_generate_stone_player(GameFighter *effect_defender, const Json::Value &effect)
{
//    JUDGE_RETURN(effect_defender != NULL, 0);
//
//    int tips_id = effect["tips_id"].asInt();
//    double last_tick = effect["last"].asDouble();
//    double interval_tick = effect["interval"].asDouble();
//
//    this->insert_defender_status(effect_defender, BasicStatus::STONE_PLAYER, interval_tick, last_tick, 0, tips_id);
//    this->insert_defender_status(effect_defender, BasicStatus::DIZZY, 0.0, last_tick + 1.0);
//
//    return effect_defender->notify_enter_stone_state();
	return 0;
}

void GameFighter::call_protected_process_push_away(int range)
{
	this->process_push_away(range);
}

