/*
 * MMOScript.h
 *
 * Created on: 2014-01-13 22:19
 *     Author: lyz
 */

#ifndef _MMOSCRIPT_H_
#define _MMOSCRIPT_H_

#include "MongoTable.h"

class MongoDataMap;

class MMOScript : public MongoTable
{
public:
    virtual ~MMOScript(void);

    int load_player_script(MapPlayerEx *player);
    static int update_data(MapPlayerEx *player, MongoDataMap *data_map);

    void load_top_info(MapPlayerEx *player, int script_type, BSONObj &obj);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOSCRIPT_H_
