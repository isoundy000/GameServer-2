/*
 * MMOScriptClean.cpp
 *
 * Created on: 2014-04-25 12:34
 *     Author: lyz
 */

#include "MMOScriptClean.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MapLogicPlayer.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOScriptClean::~MMOScriptClean(void)
{ /*NULL*/ }

int MMOScriptClean::load_player_script_clean(MapLogicPlayer *player)
{
BEGIN_CATCH
    BSONObj res = this->conection().findOne(DBScriptClean::COLLECTION, QUERY(DBScriptClean::ID << player->role_id()));
    if (res.isEmpty())
        return 0;

    ScriptCleanDetail &detail = player->script_clean_detail();
    ScriptCleanDetail::ScriptInfo script_info;
    {
        BSONObjIterator curr_iter(res.getObjectField(DBScriptClean::CURRENT_SCRIPT.c_str()));
        while (curr_iter.more())
        {
            script_info.reset();
            BSONObj obj = curr_iter.next().embeddedObject();
            script_info.__script_sort = obj[DBScriptClean::ScriptInfo::SCRIPT_SORT].numberInt();
            script_info.__chapter_key = obj[DBScriptClean::ScriptInfo::CHAPTER_KEY].numberInt();
            script_info.__use_times = obj[DBScriptClean::ScriptInfo::USE_TIMES].numberInt();
            script_info.__use_tick = obj[DBScriptClean::ScriptInfo::USE_TICK].numberInt();
            detail.__current_script_vc.push_back(script_info);
        }
        BSONObjIterator finish_iter(res.getObjectField(DBScriptClean::FINISH_SCRIPT.c_str()));
        while (finish_iter.more())
        {
            script_info.reset();
            BSONObj obj = finish_iter.next().embeddedObject();
            script_info.__script_sort = obj[DBScriptClean::ScriptInfo::SCRIPT_SORT].numberInt();
            script_info.__chapter_key = obj[DBScriptClean::ScriptInfo::CHAPTER_KEY].numberInt();
            script_info.__use_times = obj[DBScriptClean::ScriptInfo::USE_TIMES].numberInt();
            detail.__finish_script_vc.push_back(script_info);
        }
    }

    detail.__state = res[DBScriptClean::STATE].numberInt();
    detail.__exp = res[DBScriptClean::EXP].numberInt();
    detail.__savvy = res[DBScriptClean::SAVVY].numberInt();
    detail.__money.__copper = res[DBScriptClean::MONEY_COPPER].numberInt();
    detail.__money.__bind_copper = res[DBScriptClean::MONEY_BIND_COPPER].numberInt();
    detail.__money.__gold = res[DBScriptClean::MONEY_GOLD].numberInt();
    detail.__money.__bind_gold = res[DBScriptClean::MONEY_BIND_GOLD].numberInt();
    detail.__begin_tick.sec(res[DBScriptClean::BEGIN_TICK].numberInt());
    detail.__end_tick.sec(res[DBScriptClean::END_TICK].numberInt());
    detail.__terminate_tick.sec(res[DBScriptClean::TERMINATE_TICK].numberInt());
    detail.__next_check_tick.sec(res[DBScriptClean::NEXT_CHECK_TICK].numberInt());

    {
        BSONObj compact_obj = res.getObjectField(DBScriptClean::SCRIPT_COMPACT.c_str());
        ScriptCompactDetail &compact_detail = player->script_compact_detail();

        compact_detail.reset();
        compact_detail.__compact_type = compact_obj[DBScriptClean::ScriptCompact::COMPACT_TYPE].numberInt();
        compact_detail.__start_tick.sec(compact_obj[DBScriptClean::ScriptCompact::START_TICK].numberLong());
        compact_detail.__expired_tick.sec(compact_obj[DBScriptClean::ScriptCompact::EXPIRED_TICK].numberLong());
    }

    return 0;
END_CATCH
    return -1;
}

int MMOScriptClean::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
    ScriptCleanDetail &detail = player->script_clean_detail();
    ScriptCompactDetail &compact_detail = player->script_compact_detail();

    std::vector<BSONObj> curr_script_vc, finish_script_vc;
    {
        for (ScriptCleanDetail::ScriptInfoVec::iterator iter = detail.__current_script_vc.begin();
                iter != detail.__current_script_vc.end(); ++iter)
        {
            ScriptCleanDetail::ScriptInfo &info = *iter;
            curr_script_vc.push_back(BSON(DBScriptClean::ScriptInfo::SCRIPT_SORT << info.__script_sort
                        << DBScriptClean::ScriptInfo::CHAPTER_KEY << info.__chapter_key
                        << DBScriptClean::ScriptInfo::USE_TIMES << info.__use_times
                        << DBScriptClean::ScriptInfo::USE_TICK << info.__use_tick));
        }
        for (ScriptCleanDetail::ScriptInfoVec::iterator iter = detail.__finish_script_vc.begin();
                iter != detail.__finish_script_vc.end(); ++iter)
        {
            ScriptCleanDetail::ScriptInfo &info = *iter;
            finish_script_vc.push_back(BSON(DBScriptClean::ScriptInfo::SCRIPT_SORT << info.__script_sort
                        << DBScriptClean::ScriptInfo::CHAPTER_KEY << info.__chapter_key
                        << DBScriptClean::ScriptInfo::USE_TIMES << info.__use_times));
        }
    }

    BSONObjBuilder builder;
    builder << DBScriptClean::CURRENT_SCRIPT << curr_script_vc
        << DBScriptClean::FINISH_SCRIPT << finish_script_vc
        << DBScriptClean::EXP << detail.__exp
        << DBScriptClean::SAVVY << detail.__savvy
        << DBScriptClean::MONEY_COPPER << detail.__money.__copper
        << DBScriptClean::MONEY_BIND_COPPER << detail.__money.__bind_copper
        << DBScriptClean::MONEY_GOLD << detail.__money.__gold
        << DBScriptClean::MONEY_BIND_GOLD << detail.__money.__bind_gold
        << DBScriptClean::STATE << detail.__state
        << DBScriptClean::BEGIN_TICK << int(detail.__begin_tick.sec())
        << DBScriptClean::END_TICK << int(detail.__end_tick.sec())
        << DBScriptClean::TERMINATE_TICK << int(detail.__terminate_tick.sec())
        << DBScriptClean::NEXT_CHECK_TICK << int(detail.__next_check_tick.sec())
        << DBScriptClean::SCRIPT_COMPACT << BSON(DBScriptClean::ScriptCompact::COMPACT_TYPE << compact_detail.__compact_type
                << DBScriptClean::ScriptCompact::START_TICK << (long long int)(compact_detail.__start_tick.sec())
                << DBScriptClean::ScriptCompact::EXPIRED_TICK << (long long int)(compact_detail.__expired_tick.sec()));

    data_map->push_update(DBScriptClean::COLLECTION,
            BSON(DBScriptClean::ID << player->role_id()),
            builder.obj(), true);

    return 0;
}

void MMOScriptClean::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBScriptClean::COLLECTION, BSON(DBScriptClean::ID << 1), true);
END_CATCH
}

