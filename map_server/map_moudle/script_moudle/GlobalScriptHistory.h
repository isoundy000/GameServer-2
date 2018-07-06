/*
 * GlobalScriptHistory.h
 *
 * Created on: 2014-05-09 15:20
 *     Author: lyz
 */

#ifndef _GLOBALSCRIPTHISTORY_H_
#define _GLOBALSCRIPTHISTORY_H_

#include <boost/unordered_map.hpp>
#include "Singleton.h"
#include "ScriptStruct.h"

class GlobalScriptHistory
{
public:
    typedef boost::unordered_map<int, HistoryChapterRecord> ChapterRecMap;
public:

    int bind_chapter_rec(const int chapter_key, const HistoryChapterRecord &rec);
    HistoryChapterRecord *chapter_rec(const int chapter_key);
    ChapterRecMap &chapter_rec_map(void);

    int load_script_history_info(void);
    int load_history_chapter_rec(void);
    int request_save_history_chapter_rec(void);

    int request_script_history_info(const int gate_sid, const Int64 role_id, Message *msg);
    int process_chapter_history_info(const int gate_sid, const Int64 role_id, Message *msg);

protected:
    ChapterRecMap chapter_rec_map_;
};

typedef Singleton<GlobalScriptHistory> GlobalScriptHistorySingle;

#define GLOBAL_SCRIPT_HISTORY   (GlobalScriptHistorySingle::instance())

#endif //_GLOBALSCRIPTHISTORY_H_
