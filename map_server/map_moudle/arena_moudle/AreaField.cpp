/*
 * AreaField.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: peizhibi
 */

#include "AreaField.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"

#include "AreaMonitor.h"
#include "ProtoDefine.h"

int AreaField::StartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AreaField::StartTimer::handle_timeout(const Time_Value &tv)
{
	return this->area_field_->arena_start_timeout();
}

int AreaField::FieldTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AreaField::FieldTimer::handle_timeout(const Time_Value &tv)
{
	return this->area_field_->area_field_timeout();
}

int AreaField::FinishTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AreaField::FinishTimer::handle_timeout(const Time_Value &tv)
{
	this->cancel_timer();
	this->area_field_->notify_all_player_exit(GameEnum::EXIT_TYPE_SAVE);

	AREA_MONITOR->recycle_area_field(this->area_field_);
	return 0;
}

AreaField::AreaField()
{
	// TODO Auto-generated constructor stub
	this->start_timer_.area_field_ = this;
	this->field_timer_.area_field_ = this;
	this->finish_timer_.area_field_ = this;

	this->first_role_ = new ProtoRoleInfo;
	this->second_role_ = new ProtoRoleInfo;
}

AreaField::~AreaField()
{
	// TODO Auto-generated destructor stub
}

void AreaField::reset()
{
	this->area_index_ = 0;
	this->area_state_ = ARENA_STATE_NONE;

	this->enter_flag_ = false;
	this->start_timer_.tick_count_ = 0;

	this->first_id_ = 0;
	this->second_id_ = 0;

	this->start_tick_ = 0;
	this->end_tick_ = 0;

	this->src_first_id_ = 0;
	this->src_second_id_ = 0;

	this->start_timer_.cancel_timer();
	this->field_timer_.cancel_timer();
	this->finish_timer_.cancel_timer();
}

void AreaField::start_area_field(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400438*, request, ;);

	this->area_state_ = ARENA_STATE_CREATE;
	this->area_index_ = request->area_index();
	this->is_first_ = request->is_first();

	this->src_first_id_ = request->first_id();
	this->src_second_id_ = request->second_id();

	this->first_role_->CopyFrom(request->first_role());
	this->second_role_->CopyFrom(request->second_role());

	this->first_id_ = this->src_first_id_;
	this->second_id_ = this->src_second_id_;

	this->init_scene(this->area_index_, GameEnum::ARENA_SCENE_ID);
	MAP_MONITOR->bind_scene(this->area_index_, GameEnum::ARENA_SCENE_ID, this);

	this->fight_time_ = 60;
	this->max_start_time_ = 2;

	this->start_scene();
	this->start_timer_.schedule_timer(Time_Value(1));

	this->buff_flag_ = 1;
}

int AreaField::area_index()
{
	return this->area_index_;
}

int AreaField::arena_start_timeout()
{
	this->start_timer_.tick_count_ += 1;

	switch(this->area_state_)
	{
	default:
	case ARENA_STATE_NONE:
	{
		this->start_timer_.cancel_timer();
		break;
	}

	case ARENA_STATE_CREATE:
	{
		JUDGE_BREAK(this->enter_flag_ == true
				|| this->start_timer_.tick_count_ >= this->max_start_time_);

		this->area_state_ = ARENA_STATE_START;
		this->start_timer_.tick_count_ = 0;

		break;
	}

	case ARENA_STATE_START:
	{
		JUDGE_BREAK(this->start_timer_.tick_count_ > 1);

		this->area_state_ = ARENA_STATE_FIND;
		this->start_timer_.cancel_timer();

		this->field_timer_.schedule_timer(Time_Value(this->fight_time_));
		this->notiy_player_msg(this->first_id_, ACTIVE_ARENA_START_FIGHT);

		this->start_tick_ = ::time(NULL);
		this->end_tick_ = this->start_tick_ + this->fight_time_;

		break;
	}
	}

	return 0;
}

int AreaField::area_field_timeout()
{
	JUDGE_RETURN(this->area_state_ == ARENA_STATE_FIND, -1);

	MapPlayerEx* first_player = this->find_player(this->first_id_);
	MapPlayerEx* second_player = this->find_player(this->second_id_);

	if (first_player != NULL && second_player != NULL)
	{
		double left_blood[2]={0,0};
		left_blood[0] = first_player->cur_blood_percent();
		left_blood[1] = second_player->cur_blood_percent();
		{
			if( left_blood[0] < 0.01 &&  left_blood[0] > 0 )
				left_blood[0] = 0.01;
			left_blood[0] = floor( left_blood[0] * 100 )/100.00f;
			if( left_blood[1] < 0.01 && left_blood[1] > 0 )
				left_blood[1] = 0.01;
			left_blood[1] = floor( left_blood[1] * 100 )/100.00f;
		}
		if ( left_blood[0] > left_blood[1] )
		{
			this->area_field_finish(this->src_second_id_);
		}
		else
		{
			this->area_field_finish(this->src_first_id_);
		}
	}
	else if (first_player != NULL)
	{
		this->area_field_finish(this->src_second_id_);
	}
	else
	{
		this->area_field_finish(this->src_first_id_);
	}

	return 0;
}

int AreaField::area_field_finish(Int64 lose_id)
{
	JUDGE_RETURN(this->area_state_ == ARENA_STATE_FIND, -1);

	this->field_timer_.cancel_timer();
	this->area_state_ = ARENA_STATE_FINISH;

//	int finish_time = CONFIG_INSTANCE->arena("finish_time").asInt();
	int finish_time = 12;
	this->finish_timer_.schedule_timer(Time_Value(finish_time));

	Proto30100233 finish_info;
	finish_info.set_lose_id(lose_id);
	finish_info.set_area_index(this->area_index_);
	return MAP_MONITOR->dispatch_to_logic(&finish_info);
}

int AreaField::notify_all_fight_info()
{
	Proto80400350 fight_info;

	MapPlayerEx* first_player = this->find_player(this->first_id_);
	MapPlayerEx* second_player = this->find_player(this->second_id_);

	if (first_player != NULL) first_player->shrink_cur_blood();
	if (second_player != NULL) second_player->shrink_cur_blood();

	JUDGE_RETURN(first_player != NULL && second_player != NULL, -1);

	if (this->buff_flag_)
	{
		first_player->blood_max_multi_add(double(2.0), BasicElement::STATUS);
		second_player->blood_max_multi_add(double(2.0), BasicElement::STATUS);
		this->buff_flag_ = 0;
	}

	JUDGE_RETURN(first_player != NULL && second_player != NULL, -1);

	{
		FightDetail& fight_detail = first_player->fight_detail();

		fight_info.set_first_id(first_player->role_id());
		fight_info.set_first_cur_blood(fight_detail.cur_blood());
		fight_info.set_first_total_blood(fight_detail.__blood_total_i(first_player));

		fight_info.set_first_cur_magic(fight_detail.__magic);
		fight_info.set_first_total_magic(fight_detail.__magic_total_i(first_player));
	}

	{
		FightDetail& fight_detail = second_player->fight_detail();

		fight_info.set_second_id(second_player->role_id());
		fight_info.set_second_cur_blood(fight_detail.cur_blood());
		fight_info.set_second_total_blood(fight_detail.__blood_total_i(second_player));

		fight_info.set_second_cur_magic(fight_detail.__magic);
		fight_info.set_second_total_magic(fight_detail.__magic_total_i(second_player));
	}

	first_player->respond_to_client(ACTIVE_ARENA_FIGHT_INFO, &fight_info);
	second_player->respond_to_client(ACTIVE_ARENA_FIGHT_INFO, &fight_info);

	return 0;
}

int AreaField::fetch_arena_enter_info(Proto80400351* enter_info)
{
	enter_info->set_left_time(this->fight_time_);

	ProtoRoleInfo* first_role = enter_info->mutable_first_role();
	first_role->CopyFrom(*this->first_role_);

	ProtoRoleInfo* second_role = enter_info->mutable_second_role();
	second_role->CopyFrom(*this->second_role_);

	enter_info->set_is_first(this->is_first_);

	return 0;
}

int AreaField::update_arena_fighter(Int64 new_id, Int64 src_id)
{
	if (src_id == this->src_first_id_)
	{
		this->first_id_ = new_id;
		this->first_role_->set_role_id(new_id);
	}
	else
	{
		this->second_id_ = new_id;
		this->second_role_->set_role_id(new_id);
	}

	return 0;
}

int AreaField::validate_enter(Int64 role_id)
{
	JUDGE_RETURN(this->first_id_ == role_id, false);
	JUDGE_RETURN(this->enter_flag_ == false, false);

	this->enter_flag_ = true;
	return true;
}

MapPlayerEx* AreaField::find_rivial(Int64 self_id)
{
	JUDGE_RETURN(this->area_state_ == ARENA_STATE_FIND, NULL);

	if (this->first_id_ == self_id)
	{
		return this->find_player(this->second_id_);
	}
	else
	{
		return this->find_player(this->first_id_);
	}
}

