/*
 * MMOScriptProgress.h
 *
 * Created on: 2014-04-22 20:56
 *     Author: lyz
 */

#ifndef _MMOSCRIPTPROGRESS_H_
#define _MMOSCRIPTPROGRESS_H_

#include "MongoTable.h"

class ScriptPlayerRel;
class BaseScript;
class MongoDataMap;

class MMOScriptProgress : public MongoTable
{
public:
    virtual ~MMOScriptProgress(void);

    int load_script_progress(ScriptPlayerRel *player_rel);

    static int update_data(BaseScript *script, MongoDataMap *data_map);
    static int fetch_and_check_script_progress(ScriptPlayerRel *player_rel);
protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOSCRIPTPROGRESS_H_
