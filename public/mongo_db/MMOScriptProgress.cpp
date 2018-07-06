/*
 * MMOScriptProgress.cpp
 *
 * Created on: 2014-04-22 21:17
 *     Author: lyz
 */

#include "MMOScriptProgress.h"
#include "GameField.h"
#include "MongoConnector.h" 
#include "MongoDataMap.h"
#include "BaseScript.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOScriptProgress::~MMOScriptProgress(void)
{ /*NULL*/ }

int MMOScriptProgress::load_script_progress(ScriptPlayerRel *player_rel)
{
BEGIN_CATCH

    this->conection().remove(DBScriptProgress::COLLECTION, QUERY(DBScriptProgress::SAVE_TICK << BSON("$lt" << int(::time(0)))));

    *(player_rel->__progress_obj) = this->conection().findOne(DBScriptProgress::COLLECTION, QUERY(DBScriptProgress::ID << player_rel->__progress_id));
    return 0;
END_CATCH
    return -1;
}

int MMOScriptProgress::update_data(BaseScript *script, MongoDataMap *data_map)
{
    Time_Value next_day_tick = next_day(24, 0);
	{
		int is_finish = ((script->is_finish_script() || script->is_failure_script()) ? 1 : 0);
		BSONObjBuilder builder;
		builder << DBScriptProgress::ID << script->progress_id()
			<< DBScriptProgress::SCRIPT_SORT << script->script_sort()
			<< DBScriptProgress::IS_FINISH << is_finish
			<< DBScriptProgress::OWNER << script->owner_id()
            << DBScriptProgress::SAVE_TICK << Int64(next_day_tick.sec());

		data_map->push_update(DBScriptProgress::COLLECTION,
				BSON(DBScriptProgress::ID << script->progress_id()),
				builder.obj(), true);
	}
	{
		char sz_script_sort[32];
		::sprintf(sz_script_sort, "%d", script->script_sort());

		data_map->push_update(Global::COLLECTION,
				BSON(Global::KEY << Global::SCRIPT_PROGRESS),
				BSON(sz_script_sort << script->progress_id()), false);
	}

    return 0;
}

int MMOScriptProgress::fetch_and_check_script_progress(ScriptPlayerRel *player_rel)
{
    BSONObj obj = *(player_rel->__progress_obj);

    if (player_rel->__progress_id == 0)
    	return -1;

    long long int save_tick = obj[DBScriptProgress::SAVE_TICK].numberLong();
    Time_Value midnight_tick = current_day(0, 0);
    if (save_tick <= midnight_tick.sec())
    {
        player_rel->__progress_id = 0;
        return -1;
    }

    if (player_rel->__progress_id != obj[DBScriptProgress::ID].numberLong() ||
            player_rel->__script_sort != obj[DBScriptProgress::SCRIPT_SORT].numberInt() ||
            obj[DBScriptProgress::IS_FINISH].numberInt() == 0)
    {
        --player_rel->__used_times;
        player_rel->__progress_id = 0;
        if (player_rel->__used_times < 0)
            player_rel->__used_times = 0;
    }

    return 0;
}

void MMOScriptProgress::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBScriptProgress::COLLECTION, BSON(DBScriptProgress::ID << 1), true);
	this->conection().ensureIndex(DBScriptProgress::COLLECTION, BSON(DBScriptProgress::SAVE_TICK << 1), false);
END_CATCH
}


