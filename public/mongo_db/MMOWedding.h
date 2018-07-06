/*
 * MMOWedding.h
 *
 * Created on: 2015-06-08 16:15
 *     Author: lyz
 */

#ifndef _MMOWEDDING_H_
#define _MMOWEDDING_H_

#include "MongoTable.h"

class WeddingMonitor;
class WeddingDetail;

class MMOWedding : public MongoTable
{
public:
    virtual ~MMOWedding(void);

    static int load_all_wedding_detail(WeddingMonitor *monitor);
    static int update_data(WeddingDetail *wedding_info, MongoDataMap *mongo_data);
    static int remove_data(const long long int wedding_id, MongoDataMap *mongo_data);

    int load_player_wedding(LogicPlayer *player);
    static int update_data(LogicPlayer *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOWEDDING_H_
