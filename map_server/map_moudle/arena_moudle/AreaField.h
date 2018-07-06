/*
 * AreaField.h
 *
 *  Created on: Aug 19, 2014
 *      Author: peizhibi
 */

#ifndef AREAFIELD_H_
#define AREAFIELD_H_

#include "Scene.h"

class AreaField : public Scene
{
public:
	enum
	{
		ARENA_STATE_NONE 	= 0,
		ARENA_STATE_CREATE 	= 1,
		ARENA_STATE_START	= 2,
		ARENA_STATE_FIND	= 3,
		ARENA_STATE_FINISH	= 4,

		END
	};

public:
	class StartTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);

	    int tick_count_;
		AreaField* area_field_;
	};

	class FieldTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);

		AreaField* area_field_;
	};

	class FinishTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);

		AreaField* area_field_;
	};

public:
	AreaField();
	virtual ~AreaField();

	void reset();
	void start_area_field(Message* msg);

	int area_index();
	int arena_start_timeout();

	int area_field_timeout();
	int area_field_finish(Int64 lose_id);

	int notify_all_fight_info();
	int fetch_arena_enter_info(Proto80400351* enter_info);
	int update_arena_fighter(Int64 new_id, Int64 src_id);

	int validate_enter(Int64 role_id);
	MapPlayerEx* find_rivial(Int64 self_id);

private:
	int area_index_;
	int buff_flag_;
	int area_state_;
	int enter_flag_;

	int is_first_;

	int fight_time_;
	int max_start_time_;

	Int64 start_tick_;
	Int64 end_tick_;

	Int64 first_id_;
	Int64 second_id_;

	Int64 src_first_id_;
	Int64 src_second_id_;

	ProtoRoleInfo* first_role_;
	ProtoRoleInfo* second_role_;

	StartTimer start_timer_;
	FieldTimer field_timer_;
	FinishTimer finish_timer_;
};

#endif /* AREAFIELD_H_ */

