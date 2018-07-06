/*
 * GlobalScriptHistory.cpp
 *
 * Created on: 2014-05-09 15:27
 *     Author: lyz
 */

#include "MongoDataMap.h"
#include "GlobalScriptHistory.h"
#include "MapMonitor.h"
#include "GameCommon.h"
#include "MongoConnector.h"
#include "MMOScriptHistory.h"
#include "ProtoDefine.h"

int GlobalScriptHistory::bind_chapter_rec(const int chapter_key, const HistoryChapterRecord &rec)
{
    HistoryChapterRecord &chapter_rec = this->chapter_rec_map_[chapter_key];
    chapter_rec.__first_top_level_player = rec.__first_top_level_player;
    chapter_rec.__first_top_level_role_name = rec.__first_top_level_role_name;
    chapter_rec.__chapter_key = rec.__chapter_key;
    chapter_rec.__best_use_tick = rec.__best_use_tick;

    return 0;
}

HistoryChapterRecord *GlobalScriptHistory::chapter_rec(const int chapter_key)
{
    ChapterRecMap::iterator iter = this->chapter_rec_map_.find(chapter_key);
    if (iter != this->chapter_rec_map_.end())
        return &(iter->second);

    return 0;
}

GlobalScriptHistory::ChapterRecMap &GlobalScriptHistory::chapter_rec_map(void)
{
    return this->chapter_rec_map_;
}

int GlobalScriptHistory::load_script_history_info(void)
{
    this->load_history_chapter_rec();
    MSG_USER("GlobalScriptHistory...");
    return 0;
}

int GlobalScriptHistory::load_history_chapter_rec(void)
{
//    return CACHED_INSTANCE->mmo_script_history()->load_script_chapter_info(this);
	return 0;
}

int GlobalScriptHistory::request_save_history_chapter_rec(void)
{
//    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
//
//    MMOScriptHistory::update_script_chapter_info(this, data_map);
//
//    if (TRANSACTION_MONITOR->request_mongo_transaction(GameEnum::SCRIPT_SORT_CLIMB_TOWER, TRANS_UPDATE_SCRIPT_HISTORY, data_map) != 0)
//    {
//        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
//        return -1;
//    }

    return 0;
}

int GlobalScriptHistory::request_script_history_info(const int gate_sid, const Int64 role_id, Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30400903 *, request, msg, -1);

    int recogn = request->res_recogn();
    switch (recogn)
    {
        case RETURN_CHAPTER_SCRIPT_DETAIL:
            return this->process_chapter_history_info(gate_sid, role_id, msg);
        default:
            break;
    }

    return 0;
}

int GlobalScriptHistory::process_chapter_history_info(const int gate_sid, const Int64 role_id, Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30400903 *, request, msg, -1);

    Proto50400909 respond;
    respond.ParseFromString(request->res_body());

    for (int i = 0; i < respond.chapter_info_size(); ++i)
    {
        ProtoScriptChapter *proto_chapter = respond.mutable_chapter_info(i);
        if (proto_chapter->is_started() == 0)
        	break;
        int chapter_key = respond.piece() * 1000 + proto_chapter->chapter();
        HistoryChapterRecord *chapter_rec = this->chapter_rec(chapter_key);
        if (chapter_rec != 0)
        {
            proto_chapter->set_first_id(chapter_rec->__first_top_level_player);
            proto_chapter->set_first_name(chapter_rec->__first_top_level_role_name);
        }
    }

//    MSG_USER("chapter script history info: %s", respond.Utf8DebugString().c_str());

    return MAP_MONITOR->dispatch_to_client_from_gate(gate_sid, role_id, &respond);
}

