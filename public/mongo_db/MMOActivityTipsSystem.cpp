/*
 * MMOActivityTipsSystem.cpp
 *
 *  Created on: Apr 18, 2014
 *      Author: lijin
 */

#include "MMOActivityTipsSystem.h"
#include "ActivityTipsSystem.h"
#include "GameField.h"
#include "LogicPlayer.h"
#include "MongoConnector.h"
#include "MongoException.h"
#include "MongoDataMap.h"
#include <mongo/client/dbclient.h>

MMOActivityTips::MMOActivityTips()
{
    // TODO Auto-generated constructor stub

}

MMOActivityTips::~MMOActivityTips()
{
    // TODO Auto-generated destructor stub
}

int MMOActivityTips::load_player_activity_tips(LogicPlayer *player)
{
    BSONObj res = this->conection().findOne(DBActivityTipsInfo::COLLECTION,
            QUERY(DBActivityTipsInfo::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, 0);

    ActivityInfoMap& activity_info = player->activity_info();
    BSONObjIterator iter = res.getObjectField(DBActivityTipsInfo::NOTIFY_REC.c_str());
    while (iter.more())
    {
        BSONObj obj = iter.next().embeddedObject();
        int activity_id = obj[DBActivityTipsInfo::NotifyTipsRec::ACTIVITY_ID].numberInt();

        ActivityRec &rec = activity_info[activity_id];
        rec.__activity_id = activity_id;
        rec.__start_tick = obj[DBActivityTipsInfo::NotifyTipsRec::START_TICK].numberLong();
        rec.__end_tick = obj[DBActivityTipsInfo::NotifyTipsRec::END_TICK].numberLong();
        rec.__refresh_tick = obj[DBActivityTipsInfo::NotifyTipsRec::REFRESH_TICK].numberLong();
        rec.__finish_count = obj[DBActivityTipsInfo::NotifyTipsRec::FINISH_COUNT].numberInt();
    }

    return 0;
}
int MMOActivityTips::update_data(LogicPlayer *player, MongoDataMap *mongo_data)
{
    ActivityInfoMap& activity_info = player->activity_info();

    BSONVec bson_set;
    for (ActivityInfoMap::iterator iter = activity_info.begin();
            iter != activity_info.end(); ++iter)
    {
        ActivityRec &activity_rec = iter->second;
        bson_set.push_back(BSON(DBActivityTipsInfo::NotifyTipsRec::ACTIVITY_ID << iter->first
                << DBActivityTipsInfo::NotifyTipsRec::START_TICK << activity_rec.__start_tick
                << DBActivityTipsInfo::NotifyTipsRec::END_TICK << activity_rec.__end_tick
                << DBActivityTipsInfo::NotifyTipsRec::REFRESH_TICK << activity_rec.__refresh_tick
                << DBActivityTipsInfo::NotifyTipsRec::FINISH_COUNT << activity_rec.__finish_count
                << DBActivityTipsInfo::NotifyTipsRec::IS_TOUCH << activity_rec.__is_touch));
    }

    BSONObjBuilder builder;
    builder << DBActivityTipsInfo::NOTIFY_REC << bson_set;

    mongo_data->push_update(DBActivityTipsInfo::COLLECTION,
            BSON(DBActivityTipsInfo::ID << player->role_id()), builder.obj(), true);

    return 0;
}

void MMOActivityTips::ensure_all_index(void)
{
    this->conection().ensureIndex(DBActivityTipsInfo::COLLECTION, BSON(DBActivityTipsInfo::ID << 1), true);
}


