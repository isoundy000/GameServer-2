/*
 * MMOStatus.cpp
 *
 * Created on: 2013-06-18 10:54
 *     Author: lyz
 */

#include "GameField.h"
#include "MMOStatus.h"
#include "MongoConnector.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "MongoDataMap.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOStatus::~MMOStatus(void)
{ /*NULL*/ }

int MMOStatus::load_player_status(MapPlayerEx *player)
{
    GameStatus::StatusMap &status_map = player->status_map();
    GameStatus::StatusQueue &status_queue = player->status_queue();

    BSONObj res = this->conection().findOne(Status::COLLECTION,
            QUERY(Status::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, 0);

    StatusQueueNode *node = 0;
    BSONObjIterator iter(res.getObjectField(Status::STATUS.c_str()));
    while (iter.more())
    {
        BSONObj status_obj = iter.next().embeddedObject();
        BasicStatus *status = player->monitor()->status_pool()->pop();
        status->set_status(status_obj[Status::SStatus::STATUS_TYPE].numberInt());
        status->__value1 = status_obj[Status::SStatus::VALUE1].numberDouble();
        status->__value2 = status_obj[Status::SStatus::VALUE2].numberDouble();
        status->__value3 = status_obj[Status::SStatus::VALUE3].numberDouble();
        status->__value4 = status_obj[Status::SStatus::VALUE4].numberDouble();
        status->__value5 = status_obj[Status::SStatus::VALUE5].numberDouble();

        status->__skill_id = status_obj[Status::SStatus::SKILL_ID].numberInt();
        status->__level = status_obj[Status::SStatus::SKILL_LEVEL].numberInt();
        status->__attacker = status_obj[Status::SStatus::ATTACK_ID].numberLong();
        status->__accumulate_tims = status_obj[Status::SStatus::ACCUMULATE].numberInt();

        BSONObj check_obj = status_obj.getObjectField(Status::SStatus::CHECKTICK.c_str());
        status->__check_tick.sec(check_obj[Status::SStatus::Tick::SEC].numberLong());
        status->__check_tick.usec(check_obj[Status::SStatus::Tick::USEC].numberLong());

        BSONObj interval_obj = status_obj.getObjectField(Status::SStatus::INTERVAL.c_str());
        status->__interval.sec(interval_obj[Status::SStatus::Tick::SEC].numberLong());
        status->__interval.usec(interval_obj[Status::SStatus::Tick::USEC].numberLong());

        BSONObj last_obj = status_obj.getObjectField(Status::SStatus::LASTTICK.c_str());
        status->__last_tick.sec(last_obj[Status::SStatus::Tick::SEC].numberLong());
        status->__last_tick.usec(last_obj[Status::SStatus::Tick::USEC].numberLong());

        if (status_map.find(status->__status, node) != 0)
        {
            node = player->monitor()->status_queue_node_pool()->pop();
            node->set_status(*status);
            node->__check_tick = status->__check_tick;
            if (node->__remove_type[StatusQueueNode::EXIT_GAME_REMOVE] == 1
            		|| status_map.bind(status->__status, node) != 0)
            {
                player->monitor()->status_queue_node_pool()->push(node);
                player->monitor()->status_pool()->push(status);
                continue;
            }
            status_queue.push(node);
//            player->increase_status_effect(status, ENTER_SCENE_TRANSFER);
        }
        node->__status_list.push(status);
    }

    return 0;
}

int MMOStatus::update_data(MapPlayerEx *player, MongoDataMap *mongo_data)
{
    BSONVec status_vc;
    for (GameStatus::StatusMap::iterator iter = player->status_map().begin();
            iter != player->status_map().end(); ++iter)
    {
        StatusQueueNode *node = iter->second;
        for (size_t i = 0; i < node->__status_list.size(); ++i)
        {
            BasicStatus *status = node->__status_list.node(i);

            status_vc.push_back(BSON(Status::SStatus::STATUS_TYPE << status->__status
                        << Status::SStatus::VALUE1 << status->__value1
                        << Status::SStatus::VALUE2 << status->__value2
                        << Status::SStatus::VALUE3 << status->__value3
                        << Status::SStatus::VALUE4 << status->__value4
                        << Status::SStatus::VALUE5 << status->__value5
                        << Status::SStatus::SKILL_ID << status->__skill_id
                        << Status::SStatus::ACCUMULATE << status->__accumulate_tims
                        << Status::SStatus::SKILL_LEVEL << status->__level
                        << Status::SStatus::ATTACK_ID << status->__attacker
                        << Status::SStatus::CHECKTICK << BSON(
                            Status::SStatus::Tick::SEC << (long long int)status->__check_tick.sec()
                            << Status::SStatus::Tick::USEC << (long long int)status->__check_tick.usec())
                        << Status::SStatus::INTERVAL << BSON(
                            Status::SStatus::Tick::SEC << (long long int)status->__interval.sec()
                            << Status::SStatus::Tick::USEC << (long long int)status->__interval.usec())
                        << Status::SStatus::LASTTICK << BSON(
                            Status::SStatus::Tick::SEC << (long long int)status->__last_tick.sec()
                            << Status::SStatus::Tick::USEC << (long long int)status->__last_tick.usec())));
        }
    }

    BSONObjBuilder builder;
    builder << Status::STATUS << status_vc;

    mongo_data->push_update(Status::COLLECTION, BSON(Status::ID << player->role_id()),
            builder.obj(), true);
    return 0;
}

void MMOStatus::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(Status::COLLECTION, BSON(Status::ID << 1), true);
END_CATCH
}

