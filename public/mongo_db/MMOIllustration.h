/*
 * MMOIllustration.h
 *
 *  Created on: 2016年7月25日
 *      Author: lzy0927
 */

#ifndef MMOILLUSTRATION_H_
#define MMOILLUSTRATION_H_

#include "MongoTable.h"

class MongoDataMap;

class MMOIllustration: public MongoTable
{
public:
	MMOIllustration();
	virtual ~MMOIllustration();

    int load_player_illus(MapLogicPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

public:
    static int mmo_illus_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj);
    static int mmo_illus_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player);

    static int mmo_illus_group_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj);
    static int mmo_illus_group_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player);
protected:
    virtual void ensure_all_index(void);
};

#endif /* MMOILLUSTRATION_H_ */
