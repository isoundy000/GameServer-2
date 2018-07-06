/*
 * AIStruct.cpp
 *
 * Created on: 2013-04-02 14:49
 *     Author: lyz
 */

#include "AIStruct.h"
#include "FightField.h"

#include "AIManager.h"
#include "MapMonitor.h"
#include "ScriptAI.h"

double cross(MoverCoord &p1, MoverCoord &p2, MoverCoord &p3)
{
    return (p2.pixel_x() - p1.pixel_x()) * (p3.pixel_y() - p1.pixel_y()) - (p2.pixel_y() - p1.pixel_y()) * (p3.pixel_x() - p1.pixel_x());
}

bool is_in_polygon(std::vector<MoverCoord> &coord_list, MoverCoord &point)
{
    int counter = 0;
    for (size_t i = 0; i < coord_list.size(); ++i)
    {
        MoverCoord &p1 = coord_list[i];
        MoverCoord &p2 = coord_list[(i + 1) % coord_list.size()];
        if (p1.pixel_y() == p2.pixel_y())
            continue;
        if (point.pixel_y() < std::min(p1.pixel_y(), p2.pixel_y()))
            continue;
        if (point.pixel_y() > std::max(p1.pixel_y(), p2.pixel_y()))
            continue;
        double x =  (double)(point.pixel_y() - p1.pixel_y()) * (double)(p2.pixel_x() - p1.pixel_x()) / (double)(p2.pixel_y() - p1.pixel_y()) + p1.pixel_x();
        if (x > point.pixel_x())
            ++counter;
    }
    return (counter & 1) == 1;
}

void AIDetail::reset(void)
{
    this->__sort = 0;
    this->__name.clear();
    this->left_blood_skill_.clear();
    this->__left_blood_skill_use.clear();

    this->__view_sort = 0;
    this->__monster_type = 0;

    this->__birth_coord.reset();
    this->__condition_flag.reset();

    this->__born_tick = Time_Value::zero;
    this->__recycle_tick = Time_Value::zero;
    this->__auto_attack_tick.clear();

    this->around_blood_dis_ = 0;
    this->around_blood_per_ = 0;
    this->around_blood_span_ = 1;
    this->around_use_tick_ = 0;

    this->__pause_flag = 0;
    this->__attack_interval = 0;
    this->modify_blood_mode_ = 0;
    this->hurt_count_ = 0;
    this->reward_exp_mode_ = 0;

    this->__caller = 0;
    this->__leader = 0;
    this->__killed_id = 0;
    this->__config_type = 0;

    this->__combo_tag = 0;
    this->__fight_stage = 0;
    while(!this->__random_combo_skill.empty())
    {
    	this->__random_combo_skill.pop();
    }
    this->__next_interval = Time_Value::zero;
    this->__interval = Time_Value(10);

    this->__is_back = false;
    this->__is_remote = false;
    this->__gather_count = 0;
    this->__touch_count = 0;
    this->__volume = 0;
    this->__full_screen_chase = 0;

    this->__extend_back_distance = 0;
    this->__idle_back_tick = Time_Value::zero;

    this->__chase_index = -1;
    this->__chase_distance = 0;
    this->__chase_x_start = 0;
    this->__chase_y_start = 0;
    this->__layout_index = -1;
    this->__patrol_index = 0;
    this->__born_span = 0;
    this->__force_hurt = 0;

	this->league_index_ = 0;
	this->league_name_.clear();

    this->__born_type.clear();
    this->__safe_points.clear();
    this->__safe_point_ids.clear();
    this->__safe_radius = 0;
    this->__safe_radius_pixel = 0;

    this->__hurt_map.clear();
    this->__max_hurt_id = 0;
    this->__toucher_map.clear();

    this->__avoid_buff_set.clear();
    this->__moveto_action_coord.reset();
    this->__drop_owner_set.clear();

    this->__hit_stiff_last = 0.0;
    this->__last_is_move = false;
    this->__is_interval_attack = false;
}

void AIDropDetail::reset()
{
	this->ai_id_  = 0;
	this->ai_sort_ = 0;

	this->drop_type_ = 0;
	this->team_share_ = 0;
	this->pick_up_prep_ = false;

	this->item_obj_.reset();
	this->money_map_.clear();
	this->player_map_.clear();
	this->no_view_.clear();

	this->no_auto_pickup_ = 0;
    this->drop_tick_ = Time_Value::zero;
    this->recycle_tick_ = Time_Value::zero;
}

int AIDropDetail::goods_type()
{
	if (this->money_map_.empty() == false)
	{
		return AIDropDetail::TYPE_MONEY;
	}
	else
	{
		return AIDropDetail::TYPE_GOODS;
	}
}

SceneAIRecord::BirthRecord::BirthRecord(void) : 
    __level_index(0), __chase_index(0)
{ /*NULL*/ }

void SceneAIRecord::BirthRecord::reset(void)
{
    this->__birth_coord.reset();
    this->__level_index = 0;
    this->__chase_index = -1;
}

void SceneAIRecord::reset(void)
{
    this->__config_index = -1;
    this->__record_index = -1;

    this->__left_fresh_sec = 0.0;
    this->__fresh_tick = Time_Value::zero;
    this->__wave_id = 0;
    this->__script_ai_map.unbind_all();
    this->__birth_record_list.clear();
}

bool AIRecordCmp::operator() (SceneAIRecord *&left, SceneAIRecord *&right)
{
    return left->__fresh_tick < right->__fresh_tick;
}

void ScriptAIDetail::reset(void)
{
    this->__scene_config_index = -1;
    this->__record_index = -1;
    this->__level_index = 0;
    this->__wave_id = 0;

    this->__script_sort = 0;
}

AIHurtSort::AIHurtSort(void) :
    __role_id(0), __value(0)
{ /*NULL*/ }

bool AIHurtSortCmp::operator() (const AIHurtSort &left, const AIHurtSort &right)
{
    return left.__value >= right.__value;
}

