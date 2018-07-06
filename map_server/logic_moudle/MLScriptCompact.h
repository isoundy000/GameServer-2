/*
 * MLScriptCompact.h
 *
 * Created on: 2015-02-09 17:35
 *     Author: lyz
 */

#ifndef _MLSCRIPTCOMPACT_H_
#define _MLSCRIPTCOMPACT_H_

#include "MapLogicStruct.h"

class MapLogicPlayer;

class MLScriptCompact
{
public:
    MLScriptCompact(void);
    virtual ~MLScriptCompact(void);

    virtual MapLogicPlayer *script_compact_player(void);

    void reset_script_compact(void);
    ScriptCompactDetail &script_compact_detail(void);

    int use_script_compact(const int last_day);

    int sync_script_compact_info(void);
    int notify_script_compact_info(void);

    bool is_script_compact_status(void);

    int script_compact_time_up(const Time_Value &nowtime);
    int left_script_compact_tick(void);

    int script_compact_inc_exp(const int exp);

protected:
    ScriptCompactDetail script_compact_detail_;
};

#endif //_MLSCRIPTCOMPACT_H_
