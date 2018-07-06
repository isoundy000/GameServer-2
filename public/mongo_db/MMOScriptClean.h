/*
 * MMOScriptClean.h
 *
 * Created on: 2014-04-25 12:32
 *     Author: lyz
 */

#ifndef _MMOSCRIPTCLEAN_H_
#define _MMOSCRIPTCLEAN_H_

#include "MongoTable.h"

class MapLogicPlayer;
class MongoDataMap;

class MMOScriptClean : public MongoTable
{
public:
    virtual ~MMOScriptClean(void);

    int load_player_script_clean(MapLogicPlayer *player);

    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
    virtual void ensure_all_index(void);

};

#endif //_MMOSCRIPTCLEAN_H_
