/*
 * MMOOnline.h
 *
 * Created on: 2013-07-08 10:36
 *     Author: lyz
 */

#ifndef _MMOONLINE_H_
#define _MMOONLINE_H_

#include "MongoTable.h"

class MongoDataMap;

class MMOOnline : public MongoTable
{
public:
    virtual ~MMOOnline(void);

    int load_player_online(int64_t role_id, LogicOnline *online);
    int update_player_login_tick(int64_t role_id);
    int update_all_player_offline(void);

    static int update_data(const int64_t role_id, LogicOnline *online, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOONLINE_H_
