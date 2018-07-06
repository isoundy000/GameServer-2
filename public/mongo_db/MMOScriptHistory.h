/*
 * MMOScriptHistory.h
 *
 * Created on: 2014-05-09 16:44
 *     Author: lyz
 */

#ifndef _MMOSCRIPTHISTORY_H_
#define _MMOSCRIPTHISTORY_H_

#include "MongoTable.h"
#include "ScriptStruct.h"

class GlobalScriptHistory;

class MMOScriptHistory : public MongoTable
{
public:
    virtual ~MMOScriptHistory(void);

    int load_script_chapter_info(GlobalScriptHistory *history);
    static int update_script_chapter_info(GlobalScriptHistory *history, MongoDataMap *data_map);

    //问鼎江湖排行榜
    static int load_data(LegendTopPlayer &legend_player, int type);
    static int update_data(LegendTopPlayer& legend_player, int type);

    //夫妻副本赠送
    static int load_couples(LongMap &role_map);
    static int update_couples(LongMap &role_map);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOSCRIPTHISTORY_H_
