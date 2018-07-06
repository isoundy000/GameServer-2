/*
 * MLScriptCompact.cpp
 *
 * Created on: 2015-02-10 11:23
 *     Author: lyz
 */

#include "MLScriptCompact.h"
#include "MapLogicPlayer.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
//#include "ShoutNumber.h"

MLScriptCompact::MLScriptCompact(void)
{ /*NULL*/ }

MLScriptCompact::~MLScriptCompact(void)
{ /*NULL*/ }

MapLogicPlayer *MLScriptCompact::script_compact_player(void)
{
    return dynamic_cast<MapLogicPlayer *>(this);
}

void MLScriptCompact::reset_script_compact(void)
{
    this->script_compact_detail_.reset();
}

ScriptCompactDetail &MLScriptCompact::script_compact_detail(void)
{
    return this->script_compact_detail_;
}

int MLScriptCompact::use_script_compact(const int last_day)
{
//    MapLogicPlayer *player = this->script_compact_player();
    if (this->is_script_compact_status()) 
    {
        this->script_compact_detail_.__compact_type = last_day;
        this->script_compact_detail_.__start_tick = Time_Value::gettimeofday();
        this->script_compact_detail_.__expired_tick += Time_Value(last_day * 24 * 3600);
        this->script_compact_detail_.__sys_notify = 0;
    }
    else
    {
        this->script_compact_detail_.__compact_type = last_day;
        this->script_compact_detail_.__start_tick = Time_Value::gettimeofday();
        Date_Time now_date(this->script_compact_detail_.__start_tick);
        now_date.second(0);
        now_date.minute(0);
        now_date.hour(0);
        this->script_compact_detail_.__expired_tick = Time_Value(now_date.time_sec() + last_day * 24 * 3600);
        this->script_compact_detail_.__sys_notify = 0;
    }

    this->script_compact_player()->update_player_assist_single_event(GameEnum::PA_EVENT_SCRIPT_COMPACT, 0);
    this->sync_script_compact_info();
    this->notify_script_compact_info();

//    {
//        BrocastParaVec para_vec;
//        GameCommon::push_brocast_para_role_detail(para_vec, player->role_id(), player->role_name(), player->teamer_state());
//        GameCommon::push_brocast_para_int(para_vec, last_day);
//        player->monitor()->announce_world(SHOUT_ALL_SCRIPT_COMPACT, para_vec);
//    }

    return 0;
}

int MLScriptCompact::sync_script_compact_info(void)
{
    Proto31400907 req;
    req.set_type(this->script_compact_detail_.__compact_type);
    req.set_expired_tick(this->script_compact_detail_.__expired_tick.sec());
    this->script_compact_player()->send_to_map_thread(req);

    return 0;
}

int MLScriptCompact::notify_script_compact_info(void)
{
    Proto80400916 res;
    res.set_compact_type(this->script_compact_detail_.__compact_type);

    int left_tick = this->left_script_compact_tick();
    res.set_left_tick(left_tick);

    if (left_tick > 0)
    	res.set_date_tick(this->script_compact_detail_.__expired_tick.sec());

    this->script_compact_player()->respond_to_client(ACTIVE_SCRIPT_COMPACT_INFO, &res);
    return 0;
}

bool MLScriptCompact::is_script_compact_status(void)
{
    return (this->script_compact_detail_.__expired_tick >= Time_Value::gettimeofday());
}

int MLScriptCompact::script_compact_time_up(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->script_compact_detail_.__compact_type > 0, 0);

    if (this->is_script_compact_status())
    {
    	if (this->script_compact_detail_.__sys_notify == 0)
    	{
			int left_tick = this->left_script_compact_tick();
			if (left_tick <= 3600 * 24)
			{
				this->script_compact_player()->update_player_assist_single_event(GameEnum::PA_EVENT_SCRIPT_COMPACT, 1);
				this->script_compact_detail_.__sys_notify = 1;
			}
    	}
    }
    else
    {
        this->script_compact_detail_.reset();
        this->sync_script_compact_info();
        this->notify_script_compact_info();
        this->script_compact_player()->update_player_assist_single_event(GameEnum::PA_EVENT_SCRIPT_COMPACT, 0);
    }
    return 0;
}

int MLScriptCompact::left_script_compact_tick(void)
{
    int left_tick = this->script_compact_detail_.__expired_tick.sec() - Time_Value::gettimeofday().sec();
    if (left_tick < 0)
        left_tick = 0;
    return left_tick;
}

int MLScriptCompact::script_compact_inc_exp(const int exp)
{
	if (this->is_script_compact_status())
		return exp * 0.2;
	return 0;
}

