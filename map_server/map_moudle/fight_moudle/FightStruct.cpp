/*
 * FightStruct.cpp
 *
 * Created on: 2013-12-06 11:03
 *     Author: lyz
 */

#include "FightStruct.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "GameFighter.h"

BasicElement::BasicElement()
{
	BasicElement::reset();
}

double BasicElement::__total(int buff, bool multi/* = false*/) const
{
    switch (buff)
    {
	case BasicElement::ELEM_MULTI_BASE:
	{
		if (multi == true)  // 防止百分比加成重复计算
		{
			return 1;
		}

		double total = 0;
		for (int i = 0; i < OFFSET_END; ++i)
		{
			total += this->__elem[i];
		}

		total -= this->__elem[STATUS];
		total -= this->__elem[PROP_ITEM];
		return total;
	}
	case BasicElement::ELEM_OTHER_BASE:
	{
		if (multi == true)  // 防止百分比加成重复计算
		{
			return 1.00;
		}

		return (this->__elem[STATUS] + this->__elem[PROP_ITEM]);
	}
	case BasicElement::ELEM_FORCE:
	{
		if (multi == true)
		{
			return 1;
		}

		double total = 0;
		for (int i = 0; i < OFFSET_END; ++i)
		{
			total += this->__elem[i];
		}

		total -= this->__elem[STATUS];
		total -= this->__elem[PROP_ITEM];
		return total;
	}
	case BasicElement::ELEM_NO_BASE:
	{
		double total = 0;
		for (int i = 1; i < OFFSET_END; ++i)
		{
			total += this->__elem[i];
		}
		return total;
	}
	case BasicElement::ELEM_ALL:
	default:
	{
		double total = 0;
		for (int i = 0; i < OFFSET_END; ++i)
		{
			total += this->__elem[i];
		}
		return total;
	}
    }
}

int BasicElement::__total_i(int buff, bool multi/* = false*/) const
{
    return this->__total(buff, multi);
}

void BasicElement::reset(void)
{
    ::memset(this, 0, sizeof(BasicElement));
}

bool BasicElement::validate_offset(int offset)
{
	JUDGE_RETURN(offset >= 0 && offset < OFFSET_END, false);
	return true;
}

void BasicElement::reset_single(int offset)
{
	JUDGE_RETURN(this->validate_offset(offset) == true, ;);
    this->__elem[offset] = 0;
}

void BasicElement::set_single(double value, int offset)
{
    JUDGE_RETURN(this->validate_offset(offset) == true, ;);
    this->__elem[offset] = value;
}

void BasicElement::add_single(double value, int offset)
{
    JUDGE_RETURN(this->validate_offset(offset) == true, ;);
    this->__elem[offset] += value;
}

void BasicElement::reduce_single(double value, int offset)
{
    JUDGE_RETURN(this->validate_offset(offset) == true, ;);
    this->__elem[offset] -= value;
}

double BasicElement::basic() const
{
	return this->__elem[BASIC];
}

double BasicElement::single(int offset)
{
    JUDGE_RETURN(this->validate_offset(offset) == true, 0);
    return this->__elem[offset];
}

void HistoryStatus::reset(void)
{
    ::memset(this, 0, sizeof(HistoryStatus));
}

int BasicStatus::status_type() const
{
	return this->__buff_type;
}

int BasicStatus::client_status()
{
	if (this->__client_status > 0)
	{
		return this->__client_status;
	}
	else
	{
		return this->__status;
	}
}

int BasicStatus::left_time(int type)
{
	JUDGE_RETURN(this->__check_tick != Time_Value::zero, 0);
	Time_Value diff = this->__check_tick - Time_Value::gettimeofday();

	switch (type)
	{
	case 0:	//秒
	{
		return std::max<Int64>(diff.sec(), 0);
	}
	case 1:	//毫秒
	{
		return (diff.sec() * 1000 + diff.usec() / 1000);
	}
	}

	return 0;
}

double BasicStatus::fetch_value(int type)
{
	switch (type)
	{
	case BasicStatus::VALUE1:
	{
		return this->__value1;
	}
	case BasicStatus::VALUE2:
	{
		return this->__value2;
	}
	case BasicStatus::VALUE3:
	{
		return this->__value3;
	}
	case BasicStatus::VALUE4:
	{
		return this->__value4;
	}
	case BasicStatus::VALUE5:
	{
		return this->__value5;
	}
	}
	return 0;
}

const Json::Value& BasicStatus::conf() const
{
	return CONFIG_INSTANCE->buff(this->__status);
}

BasicStatus::BasicStatus()
{
	BasicStatus::reset();
}

BasicStatus::BasicStatus(int status)
{
	BasicStatus::reset();
	this->set_status(status);
}

BasicStatus::BasicStatus(double value, double percent, Int64 attackor)
{
	BasicStatus::reset();
	this->set_normal_value(value, percent);
	this->__attacker = attackor;
}

void BasicStatus::reset()
{
    this->__status = 0;
    this->__buff_type = 0;
    this->__show_type = 0;
    this->__client_msg = 0;
    this->__client_status = 0;

    this->__flag = 0;
    this->__value1 = 0;
    this->__value2 = 0;
    this->__value3 = 0;
    this->__value4 = 0;
    this->__value5 = 0;
    this->__check_tick = Time_Value::zero;
    this->__interval = Time_Value::zero;
    this->__last_tick = Time_Value::zero;

    this->__skill_id = 0;
    this->__level = 0;
    this->__attacker = 0;
    this->__accumulate_tims = 0;

    this->__value6.clear();
    this->__avoid_buff.clear();
    this->__inoperative_scene.clear();
    this->recyle_after_status();
}

void BasicStatus::recyle_after_status()
{
	for (std::vector<BasicStatus*>::iterator iter = this->__after_status.begin();
			iter != this->__after_status.end(); ++iter)
	{
		MAP_MONITOR->status_pool()->push(*iter);
	}

	this->__after_status.clear();
}

void BasicStatus::set_value(int type, double val)
{
	switch (type)
	{
	case BasicStatus::VALUE1:
	{
		this->__value1 = val;
		break;
	}
	case BasicStatus::VALUE2:
	{
		this->__value2 = val;
		break;
	}
	case BasicStatus::VALUE3:
	{
		this->__value3 = val;
		break;
	}
	case BasicStatus::VALUE4:
	{
		this->__value4 = val;
		break;
	}
	case BasicStatus::VALUE5:
	{
		this->__value5 = val;
		break;
	}
	}
}

void BasicStatus::set_normal_value(double value, double percent)
{
	this->set_value(BasicStatus::VALUE1, value);
	this->set_value(BasicStatus::VALUE2, percent);
}

void BasicStatus::set_status(int status)
{
	this->__status = status;

	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf.empty() == false, ;);

	this->set_all(Time_Value::gettime(conf["last"].asDouble()),
			Time_Value::gettime(conf["interval"].asDouble()),
			0, 0, 0, conf["value"].asInt(), conf["percent"].asInt());
}

void BasicStatus::set_client_status(int client_status)
{
	this->__client_status = client_status;
}

void BasicStatus::set_normal(int status, double last, double interval, double val1, double val2)
{
	this->__status = status;
	this->set_all(Time_Value::gettime(last), Time_Value::gettime(interval), 0, 0, 0, val1, val2);
}

void BasicStatus::set_all(const Time_Value &last, const Time_Value &interval,
        Int64 attackor, int skill_id, int level,
        double val1, double val2, double val3,
        double val4, double val5)
{
	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf.empty() == false, ;);

	this->__buff_type 	= conf["buff_type"].asInt();
	this->__show_type 	= conf["show_type"].asInt();
	this->__client_msg 	= conf["client_msg"].asInt();
    this->__check_tick 	= Time_Value::zero;
    this->__last_tick 	= last;
    this->__interval 	= interval;

    this->__attacker = attackor;
    this->__skill_id = skill_id;
    this->__level = level;

    this->__value1 = val1;
    this->__value2 = val2;
    this->__value3 = val3;
    this->__value4 = val4;
    this->__value5 = val5;

    GameCommon::json_to_t_map(this->__avoid_buff, conf["avoid_buff"]);
    GameCommon::json_to_t_map(this->__inoperative_scene, conf["inoperative_scene"]);
}

void BasicStatus::serialize(ProtoStatus *proto)
{
	proto->set_status(this->client_status());
    proto->set_value1(this->__value1);
    proto->set_value2(this->__value2);
    proto->set_value3(this->__value3);
    proto->set_last_tick(this->__last_tick.sec());
    proto->set_attackor(this->__attacker);

    int left_tick = this->left_time();
    proto->set_cool_tick(left_tick);
}

void BasicStatus::serialize_b(int i, ProtoStatus *proto)
{
	proto->set_value1(this->__value6[i * 2]);
	proto->set_value2(this->__value6[i * 2 + 1]);

	proto->set_status(this->client_status());
    proto->set_value3(this->__value3);
    proto->set_last_tick(this->__last_tick.sec());
    proto->set_attackor(this->__attacker);

    int left_tick = this->left_time();
    proto->set_cool_tick(left_tick);
}

bool StatusValueCmp::operator() (BasicStatus *&left, BasicStatus *&right)
{
    double value_l_1 = std::abs(left->__value1),
           value_r_1 = std::abs(right->__value1),
           value_l_2 = std::abs(left->__value2),
           value_r_2 = std::abs(right->__value2);
    if (value_l_2 > 0.000001 || value_r_2 > 0.000001)
        return value_l_2 > value_r_2;
    else if (value_l_1 > 0.000001 || value_r_1 > 0.000001)
        return value_l_1 > value_r_1;
    else if (left->__check_tick < right->__check_tick)
        return true;
    return false;
}

StatusQueueNode::StatusQueueNode()
{
	StatusQueueNode::reset();
}

void StatusQueueNode::reset()
{
    this->__status = 0;
    this->__check_tick = Time_Value::zero;

    for (int i = 0; i < TOTAL_REMOVE_TYPE; ++i)
    {
    	this->__remove_type[i] = 0;
    }

    for (size_t i = 0; i < this->__status_list.size(); ++i)
    {
        MAP_MONITOR->status_pool()->push(this->__status_list.node(i));
    }
    this->__status_list.clear();
}

void StatusQueueNode::set_status(const BasicStatus &status)
{
	this->__status = status.__status;

	const Json::Value& conf = status.conf();
	this->__remove_type[DIE_REMOVE] = conf["die_remove"].asInt();
	this->__remove_type[EXIT_SCENE_REMOVE] = conf["exit_scene_remove"].asInt();
	this->__remove_type[EXIT_GAME_REMOVE] = conf["exit_game_remove"].asInt();
}

bool StatusQueueNodeCmp::operator() (StatusQueueNode *&left, StatusQueueNode *&right)
{
    return left->__check_tick < right->__check_tick;
}

bool StatusTickCmp::operator() (BasicStatus *&left, BasicStatus *&right)
{
    return left->__check_tick < right->__check_tick;
}


TraceForceInfo::TraceForceInfo()
{
	this->prev_force_ = 0;
	this->fighter_ = NULL;
}

SubObj& TraceForceInfo::sub_info()
{
	return this->sub_;
}

void TraceForceInfo::set_trace_info(GameFighter* fighter, const SubObj& sub)
{
	this->fighter_ = fighter;
	JUDGE_RETURN(this->fighter_ != NULL, ;);

	this->sub_ = sub;
	this->prev_force_ = this->fighter_->force_total_i();
}

bool TraceForceInfo::trace_dump_a()
{
	JUDGE_RETURN(this->fighter_ != NULL, false);
	return this->fighter_->force_total_i() != this->prev_force_;
}

bool TraceForceInfo::trace_dump_b()
{
	return false;
}

TraceForceInfo::~TraceForceInfo()
{
}

bool FighterSkillCmp::operator() (FighterSkill *&left, FighterSkill *&right)
{
	return left->__use_tick < right->__use_tick;
}

void DelaySkillInfo::reset(void)
{
    this->__launch_tick = Time_Value::zero;
    this->__request.reset();
}
bool DelaySkillCmp::operator() (DelaySkillInfo *&left, DelaySkillInfo *&right)
{
    return left->__launch_tick < right->__launch_tick;
}

void PassiveSkillInfo::reset(void)
{
	this->__launch_tick = Time_Value::zero;
	this->__skill = 0;
}
bool PassiveSkillCmp::operator()(PassiveSkillInfo *&left, PassiveSkillInfo *&right)
{
	if (left->__launch_tick < right->__launch_tick)
		return true;
	return false;
}

double FightDetail::blood_percent(GameFighter *fighter, int coefficient)
{
	int total_blood = this->__blood_total_i(fighter);
	JUDGE_RETURN(total_blood > 0, 0);

	return double(this->cur_blood()) / total_blood * coefficient;
}

double FightDetail::magic_percent(GameFighter *fighter, int coefficient)
{
	int total_magic = this->__magic_total_i(fighter);
	JUDGE_RETURN(total_magic > 0, 0);

	return double(this->__magic) / total_magic * coefficient;
}

int FightDetail::cur_blood()
{
	return this->__blood;
}

int FightDetail::cur_magic()
{
	return this->__magic;
}

void FightDetail::set_cur_blood(double percent)
{
	JUDGE_RETURN(this->__blood > 0, ;);

	int max_blood = this->__blood_total_i(NULL);
	this->__blood = max_blood * std::min<double>(1, percent);
}

void FightDetail::set_cur_magic(double percent)
{
	int max_magic = this->__magic_total_i(NULL);
	this->__magic = max_magic * std::min<double>(1, percent);
}

int FightDetail::enough_exp_upgrade()
{
	return this->__experience >= this->__next_exp;
}

int FightDetail::left_relive_time(int delay)
{
	Int64 left_wait = this->__death_tick + delay - ::time(NULL);
	return left_wait > 0 ? left_wait : 0;
}

bool FightDetail::validate_offset(int offset)
{
	return BasicElement::validate_offset(offset);
}

void FightDetail::reset(void)
{
    this->__camp_id = 0;
    this->__glamour = 0;
    this->__pk_state = 0;
    this->__pk_value = 0;
    this->__last_skill = 0;
    this->__fight_state = 0;

    this->__fight_tick = Time_Value::zero;
    this->__relive_request_two = 0;
    this->__death_tick = 0;
    this->__relive_tick = Time_Value::zero;
    this->__pk_tick = Time_Value::zero;
    this->__gather_tick= Time_Value::zero;

    this->gather_state_ = 0;
	this->hit_klv_ = 0;
	this->avoid_klv_ = 0;
	this->crit_klv_	= 0;
	this->toughness_klv_ = 0;

    this->__level = 1;
    this->__experience = 0;
    this->__jump = 0;
    this->__jump_times = 0;
    this->__angry = 0;
    this->__next_exp = 0;
    this->__recover_blood = 0;
    this->__recover_blood_per = 0;
	this->__auto_recover_blood = 0;
	this->__recover_blood_span = 1;

    this->__attack_lower.reset();
    this->__attack_upper.reset();
    this->__defence_lower.reset();
    this->__defence_upper.reset();
    this->__crit.reset();
    this->__toughness.reset();
    this->__hit.reset();
    this->__avoid.reset();
    this->__lucky.reset();
    this->__damage.reset();
    this->__reduction.reset();

    this->__attack_lower_multi.reset();
    this->__attack_lower_multi.set_single(1.00);
    this->__attack_upper_multi.reset();
    this->__attack_upper_multi.set_single(1.00);
    this->__defence_lower_multi.reset();
    this->__defence_lower_multi.set_single(1.00);
    this->__defence_upper_multi.reset();
    this->__defence_upper_multi.set_single(1.00);
    this->__crit_value_multi.reset();
    this->__crit_value_multi.set_single(1.00);
    this->__crit_hurt_multi.reset();
    this->__crit_hurt_multi.set_single(1.00);
    this->__toughness_multi.reset();
    this->__toughness_multi.set_single(1.00);
    this->__hit_multi.reset();
    this->__hit_multi.set_single(1.00);
    this->__avoid_multi.reset();
    this->__avoid_multi.set_single(1.00);
    this->__lucky_multi.reset();
    this->__lucky_multi.set_single(1.00);
    this->__damage_multi.reset();
//    this->__damage_multi.set_single(1.00);
    this->__reduction_multi.reset();
//    this->__reduction_multi.set_single(1.00);

    this->__blood = 0;
    this->__blood_max.reset();
    this->__magic = 0;
    this->__magic_max.reset();

    this->__blood_multi.reset();
    this->__blood_multi.set_single(1.00);
    this->__magic_multi.reset();
    this->__magic_multi.set_single(1.00);

    this->__skill_map.clear();
    this->__passive_skill_use_time.clear();
    this->__attackor_map.unbind_all();
    this->__last_defender_id = 0;
    this->__history_defender_set.clear();
    this->__passive_skill_set.clear();

    this->__color_all_per = 0;
    this->__skill_id_for_step = 0;
    this->__skill_step = 0;
    this->__skill_step_tick = Time_Value::zero;

	PassiveSkillInfo *info = 0;
	while ((info = this->__fight_passive_skill_queue.pop()) != 0)
	{
		MAP_MONITOR->passive_skill_qn_pool()->push(info);
	}

	info = 0;
	while ((info = this->__hurt_passive_skill_queue.pop()) != 0)
	{
		MAP_MONITOR->passive_skill_qn_pool()->push(info);
	}
}

void FightDetail::set_level(int level)
{
	JUDGE_RETURN(level > 0, ;);

	this->__level = level;

	const Json::Value& level_json = CONFIG_INSTANCE->role_level(0, level);
	this->__recover_blood = level_json["recover_blood"].asInt();
	this->__recover_blood_per = level_json["recover_blood_per"].asInt();

	JUDGE_RETURN(GameCommon::is_max_level(level) == false, ;);
	this->__next_exp = std::max<Int64>(level_json["exp"].asDouble(), 1);
}

void FightDetail::clear_all_fight_property(int offset)
{
	this->__blood_max.set_single(0, offset);
	this->__blood_multi.set_single(0, offset);
	this->__magic_max.set_single(0, offset);
	this->__magic_multi.set_single(0, offset);
	this->__attack_lower.set_single(0, offset);
	this->__attack_lower_multi.set_single(0, offset);
	this->__attack_upper.set_single(0, offset);
	this->__attack_upper_multi.set_single(0, offset);
	this->__defence_lower.set_single(0, offset);
	this->__defence_lower_multi.set_single(0, offset);
	this->__defence_upper.set_single(0, offset);
	this->__defence_upper_multi.set_single(0, offset);
	this->__hit.set_single(0, offset);
	this->__hit_multi.set_single(0, offset);
	this->__avoid.set_single(0, offset);
	this->__avoid_multi.set_single(0, offset);
	this->__crit.set_single(0, offset);
	this->__crit_value_multi.set_single(0, offset);
	this->__crit_hurt_multi.set_single(0, offset);
	this->__toughness.set_single(0, offset);
	this->__toughness_multi.set_single(0, offset);
	this->__lucky.set_single(0, offset);
	this->__lucky_multi.set_single(0, offset);
	this->__damage.set_single(0, offset);
	this->__damage_multi.set_single(0, offset);
	this->__reduction.set_single(0, offset);
	this->__reduction_multi.set_single(0, offset);
}

void FightDetail::add_fighter_property(int offset, const FightProperty& prop)
{
    this->__attack_lower.add_single(prop.attack_, offset);
    this->__attack_upper.add_single(prop.attack_, offset);
    this->__defence_lower.add_single(prop.defence_, offset);
    this->__defence_upper.add_single(prop.defence_, offset);
    this->__blood_max.add_single(prop.blood_max_, offset);
    this->__hit.add_single(prop.hit_, offset);
    this->__avoid.add_single(prop.avoid_, offset);
    this->__crit.add_single(prop.crit_, offset);
    this->__toughness.add_single(prop.toughness_, offset);
}

double FightDetail::__blood_total(GameFighter *fighter, const int buff) const
{
    double blood = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        blood = this->__blood_max.__total(buff);
    }
    else
    {
        blood = this->__blood_max.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__blood_multi.__total(buff, true) +
            this->__blood_max.__total(BasicElement::ELEM_OTHER_BASE);
        blood *= ( 1 + this->__color_all_per / 100.0);
    }

    if (blood > INT_MAX)
    {
        blood = INT_MAX;
    }

    return blood > 1.00 ? blood : 1.00;
}

int FightDetail::__blood_total_i(GameFighter *fighter, const int buff) const
{
    return this->__blood_total(fighter, buff);
}

double FightDetail::__magic_total(GameFighter *fighter, const int buff) const
{
    double magic = this->__magic_max.__total(BasicElement::ELEM_MULTI_BASE) * //
        this->__magic_multi.__total(buff, true) +
        this->__magic_max.__total(BasicElement::ELEM_OTHER_BASE);

    magic *= ( 1 + this->__color_all_per / 100.0);
    return magic > 1.00 ? magic : 1.00;
}

int FightDetail::__magic_total_i(GameFighter *fighter, const int buff) const
{
    return this->__magic_total(fighter, buff);
}

double FightDetail::__attack_total(GameFighter *fighter, const int buff) const
{
	return this->__attack_upper_total(fighter, buff);
}

int FightDetail::__attack_total_i(GameFighter *fighter, const int buff) const
{
    return this->__attack_total(fighter, buff);
}

// 攻击下限
double FightDetail::__attack_lower_total(GameFighter *fighter, const int buff) const
{
    double attack = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        attack = this->__attack_lower.__total(buff);
    }
    else
    {
        attack = this->__attack_lower.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__attack_lower_multi.__total(buff, true) + //
    		this->__attack_lower.__total(BasicElement::ELEM_OTHER_BASE);

        attack *= ( 1 + this->__color_all_per / 100.0);
    }
    return attack > 1.00 ? attack : 1.00;
}

// 攻击上限
double FightDetail::__attack_upper_total(GameFighter *fighter, const int buff) const
{
    double attack = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        attack = this->__attack_upper.__total(buff);
    }
    else
    {
        attack = this->__attack_upper.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__attack_upper_multi.__total(buff, true) + //
    		this->__attack_upper.__total(BasicElement::ELEM_OTHER_BASE);

        attack *= ( 1 + this->__color_all_per / 100.0);
    }
    return attack > 1.00 ? attack : 1.00;
}

double FightDetail::__defence_total(GameFighter *fighter, const int buff) const
{
	return this->__defence_lower_total(fighter, buff);
}

int FightDetail::__defence_total_i(GameFighter *fighter, const int buff) const
{
    return this->__defence_upper_total(fighter, buff);
}

// 防御下限
double FightDetail::__defence_lower_total(GameFighter *fighter, const int buff) const
{
    double defence = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        defence = this->__defence_lower.__total(buff);
    }
    else
    {
        defence = this->__defence_lower.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__defence_lower_multi.__total(buff, true) + //
            this->__defence_lower.__total(BasicElement::ELEM_OTHER_BASE);

        defence *= ( 1 + this->__color_all_per / 100.0);
    }

    return defence > 1.00 ? defence : 1.00;
}

// 防御上限
double FightDetail::__defence_upper_total(GameFighter *fighter, const int buff) const
{
    double defence = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        defence = this->__defence_upper.__total(buff);
    }
    else
    {
        defence = this->__defence_upper.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__defence_upper_multi.__total(buff, true) + //
            this->__defence_upper.__total(BasicElement::ELEM_OTHER_BASE);

        defence *= ( 1 + this->__color_all_per / 100.0);
    }

    return defence > 1.00 ? defence : 1.00;
}

double FightDetail::__crit_total(GameFighter *fighter, const int buff) const
{
	double crit = 0.0;
	if (buff == BasicElement::ELEM_FORCE)
	{
		crit = this->__crit.__total(buff);
	}
	else
	{
		crit = this->__crit.__total(BasicElement::ELEM_MULTI_BASE)
				+ this->__crit_value_multi.__total(buff, true)
				+ this->__crit.__total(BasicElement::ELEM_OTHER_BASE);

		crit *= ( 1 + this->__color_all_per / 100.0);
	}
	return crit > 0.000001 ? crit : 1;
}

int FightDetail::__crit_total_i(GameFighter *fighter, const int buff) const
{
	return this->__crit_total(fighter, buff);
}

double FightDetail::__crit_hurt_multi_total(GameFighter *fighter, const int buff) const
{
	return std::max<double>(this->__crit_hurt_multi.__total(buff), 1.00);
}

double FightDetail::__toughness_total(GameFighter *fighter, const int buff) const
{
    double toughness = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        toughness = this->__toughness.__total(buff);
    }
    else
    {
        toughness = this->__toughness.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__toughness_multi.__total(buff, true) + //
            this->__toughness.__total(BasicElement::ELEM_OTHER_BASE);

        toughness *= ( 1 + this->__color_all_per / 100.0);
    }
    return toughness > 0.000001 ? toughness : 1;
}

int FightDetail::__toughness_total_i(GameFighter *fighter, const int buff) const
{
    return this->__toughness_total(fighter, buff);
}

double FightDetail::__hit_total(GameFighter *fighter, const int buff) const
{
    double hit = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        hit = this->__hit.__total(buff);
    }
    else
    {
        hit = this->__hit.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__hit_multi.__total(buff, true) + //
    	    this->__hit.__total(BasicElement::ELEM_OTHER_BASE);

        hit *= ( 1 + this->__color_all_per / 100.0);
    }
    return hit > 0.0100 ? hit : 0.0100;
}

int FightDetail::__hit_total_i(GameFighter *fighter, const int buff) const
{
    return this->__hit_total(fighter, buff);
}

double FightDetail::__avoid_total(GameFighter *fighter, const int buff) const
{
    double avoid = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
        avoid = this->__avoid.__total(buff);
    }
    else
    {
        avoid = this->__avoid.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__avoid_multi.__total(buff, true) + //
    	    this->__avoid.__total(BasicElement::ELEM_OTHER_BASE);

        avoid *= ( 1 + this->__color_all_per / 100.0);
    }

    return avoid > 0.0100 ? avoid : 0.0100;
}

int FightDetail::__avoid_total_i(GameFighter *fighter, const int buff) const
{
    return this->__avoid_total(fighter, buff);
}

double FightDetail::__lucky_total(GameFighter *fighter, const int buff) const
{
    double lucky = 0.0;
    if (buff == BasicElement::ELEM_FORCE)
    {
    	lucky = this->__lucky.__total(buff);
    }
    else
    {
    	lucky = this->__lucky.__total(BasicElement::ELEM_MULTI_BASE) * //
            this->__lucky_multi.__total(buff, true) + //
    	    this->__lucky.__total(BasicElement::ELEM_OTHER_BASE);

    	lucky *= ( 1 + this->__color_all_per / 100.0);
    }
    return lucky > 0.000001 ? lucky : 0.0;
}

int FightDetail::__lucky_total_i(GameFighter *fighter, const int buff) const
{
	return this->__lucky_total(fighter, buff);
}

double FightDetail::__damage_total(GameFighter *fighter, const int buff) const
{
    return this->__damage.__total(buff);
}

double FightDetail::__damage_rate_total(GameFighter *fighter, const int buff) const
{
    return this->__damage_multi.__total(buff, true);
}

double FightDetail::__reduction_total(GameFighter *fighter, const int buff) const
{
    return this->__reduction.__total(buff);
}

double FightDetail::__reduction_rate_total(GameFighter *fighter, const int buff) const
{
    return this->__reduction_multi.__total(buff);
}

double FightDetail::__force_total(GameFighter *fighter)
{
	double 	attack_upper = this->__attack_upper_total(fighter, BasicElement::ELEM_FORCE),
			defence_upper = this->__defence_upper_total(fighter, BasicElement::ELEM_FORCE),
			blood = this->__blood_total(fighter, BasicElement::ELEM_FORCE),
			hit = this->__hit_total(fighter, BasicElement::ELEM_FORCE),
			avoid = this->__avoid_total(fighter, BasicElement::ELEM_FORCE),
			crit = this->__crit_total(fighter, BasicElement::ELEM_FORCE),
			toughness = this->__toughness_total(fighter, BasicElement::ELEM_FORCE);

    double property_force = attack_upper * 5 + defence_upper * 5
    		+ blood * 0.5 + hit * 15 + avoid * 15 + crit * 15 + toughness * 15;
    return property_force;
}

int FightDetail::__force_total_i(GameFighter *fighter)
{
    return this->__force_total(fighter);
}

double FightDetail::__element_force(int offset)
{
    double force = this->__attack_upper.__elem[offset] * 5 + this->__defence_upper.__elem[offset] * 5
    		+ this->__blood_max.__elem[offset] * 0.5 + this->__hit.__elem[offset] * 15
    		+ this->__avoid.__elem[offset] * 15 + this->__crit.__elem[offset] * 15
    		+ this->__toughness.__elem[offset] * 15;
    return force;
}

DefenderHurt::DefenderHurt(void) :
    __defender_id(0), __hurt_blood(0), __hurt_magic(0)
{ /*NULL*/ }

void DefenderHurt::reset(void)
{
    ::memset(this, 0, sizeof(DefenderHurt));
}

CurrentSkill::CurrentSkill()
{
	this->__skill = NULL;
	this->__from_pool = false;
    this->__client_target = 0;	// no reset

	CurrentSkill::reset();
}

void CurrentSkill::reset(void)
{
    this->__effect_flag.reset();
    this->__client_target_set.clear();

    this->__skill_id = 0;
    this->__display_skill = 0;
    this->__skill_level = 1;
    this->__skill_coord.reset();
    this->__play_coord.reset();

    this->__angle = 0;
    this->__radian = 0;
    this->__skill_step = 0;
    this->__prev_skill_launch_tick = Time_Value::zero;
    this->__is_full_screen = 0;
    this->__add_effect_tick = 0;

    this->__hurt_flag = false;
    this->__defender_set.clear();
    this->__defender_id = 0;
    this->__has_defender_buff = 0;
    this->__skill_hurt = 0;
    this->__skill_hurt_percent = 0;
    this->__skill_blood = 0;
    this->__skill_self_blood = 0;
    this->__skill_hurt_deep = 0;
    this->__skill_hurt_deep_percent = 0;
    this->__defender = NULL;
    this->__avoid_hurt = 0;
    this->__in_safe_area = false;
    this->__sel_defender = 0;

    if (this->__from_pool == true)
    {
    	MAP_MONITOR->skill_pool()->push(this->__skill);
    }
    this->__skill = NULL;
    this->__from_pool = false;

    this->__defender_map.unbind_all();
    this->__passive_effect_set.clear();

    this->save_skill_ = 0;
    this->save_defender_ = 0;
}

void CurrentSkill::set_angle(double angle)
{
	this->__angle = angle;
	this->__radian = angle * PI / 180.0;
}

void CurrentSkill::set_radian(double radian)
{
	this->__radian = radian;
	this->__angle = radian * 57.3;
}

void CurrentSkill::set_skill_id(GameFighter* fighter, int skill_id, int skill_level)
{
	this->__skill_id = skill_id;
	this->__display_skill = skill_id;
	JUDGE_RETURN(fighter->find_skill(skill_id, this->__skill) != 0, ;);

	this->__from_pool = true;
	this->__skill = MAP_MONITOR->pop_skill(skill_id, skill_level);
}

void CurrentSkill::save_info()
{
	this->save_skill_ = this->__skill_id;
	this->save_defender_ = this->__defender_id;
}

void CurrentSkill::restore_info()
{
	this->__skill_id = this->save_skill_;
	this->__defender_id = this->save_defender_;
}

int CurrentSkill::aoe_type()
{
	return this->__skill->__aoe_type;
}

int CurrentSkill::is_base_skill()
{
	return GameCommon::is_base_skill(this->__skill);
}

int CurrentSkill::is_jump_skill()
{
	return GameCommon::is_jump_skill(this->__skill);
}

int CurrentSkill::need_check_distance()
{
	switch (this->__skill->__aoe_type)
	{
	case GameEnum::SKILL_AOE_SELF_CIRCLE:
	case GameEnum::SKILL_AOE_SELF_SECTOR:
	case GameEnum::SKILL_AOE_SELF_RECT:
	case GameEnum::SKILL_AOE_SCENE_LEFT_SIDE:
	case GameEnum::SKILL_AOE_SCENE_RIGHT_SIDE:
	case GameEnum::SKILL_AOE_SELF_RING:
	case GameEnum::SKILL_AOE_CUR_AIM_TARGET:
	{
		return false;
	}
	}

	return true;
}

void CurrentSkill::effect_flag_reset(void)
{
    this->__effect_flag.reset();
}

void CurrentSkill::effect_flag_reset(const int flag)
{
    if (flag < 0 || SKILL_EFFECT_END <= flag)
        return;
    this->__effect_flag.reset(flag);
}

void CurrentSkill::effect_flag_set(const int flag)
{
    if (flag < 0 || SKILL_EFFECT_END <= flag)
        return;
    this->__effect_flag.set(flag);
}

bool CurrentSkill::effect_flag_test(const int flag)
{
    if (flag < 0 || SKILL_EFFECT_END <= flag)
    {
        return false;
    }
    return this->__effect_flag.test(flag);
}

void LoopSkillDetail::reset(void)
{
	this->__skill 	= 0;
	this->__cnt 	= 0;
    this->__radian 	= 0;

    this->__client_target 	= 0;
    this->__skill_coord_type= 0;

	this->__next 	= Time_Value::zero;
	this->__timeout = Time_Value::zero;
	this->__interval= Time_Value::zero;

	this->__fixed_skill_coord.reset();
}

