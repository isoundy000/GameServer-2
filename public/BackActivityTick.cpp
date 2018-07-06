/*
 * BackActivityTick.cpp
 *
 * Created on: 2015-05-08 11:49
 *     Author: lyz
 */

#include "BackActivityTick.h"
#include "BaseUnit.h"
#include "Transaction.h"
#include "TransactionMonitor.h"
#include "BackField.h"
#include "BackBrocast.h"
#include "PoolMonitor.h"
#include "LogicMonitor.h"
#include "TQueryCursor.h"
#include "ProtoDefine.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include <mongo/client/dbclient.h>

void BackActivityTick::ActivityTickInfo::reset(void)
{
    this->__begin_tick = Time_Value::zero;
    this->__end_tick = Time_Value::zero;
}

BackActivityTick::BackActivityTick(void) :
    version_cnt_(0)
{ /*NULL*/ }

int BackActivityTick::init(void)
{
    BackDraw::load_activity_tick_at_init(this);
    MSG_USER("BackActivityTick %d...", this->cur_activity_tick_map().size());
    return 0;
}

int BackActivityTick::check_update_back_activity_tick(BaseUnit *unit)
{
    MongoDataMap *mongo_data = POOL_MONITOR->mongo_data_map_pool()->pop();
    mongo_data->push_multithread_query(DBBackDraw::COLLECTION);
    TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_BACK_ACTIVITY_TICK, mongo_data, unit);
    return 0;
}

int BackActivityTick::update_activity_tick_in_logic_server(Transaction *trans)
{
    JUDGE_RETURN(trans != NULL, 0);
    MongoDataMap *data_map = trans->fetch_mongo_data_map();
    if (data_map == NULL)
    {
        trans->rollback();
        return 0;
    }

    MongoData *mongo_data = NULL;
    if (data_map->find_data(DBBackDraw::COLLECTION, mongo_data) != 0)
    {
        trans->rollback();
        return 0;
    }

    ActivityTickMap &tick_map = this->next_activity_tick_map();
    tick_map.clear();
    tick_map = this->cur_activity_tick_map();

    ActivityTickInfo info;
    bool is_has_data = false;

    auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
    while (cursor->more())
    {
        BSONObj obj = cursor->next();
        info.reset();
        info.__begin_tick.sec(obj[DBBackDraw::S_TICK].numberInt());
        info.__end_tick.sec(obj[DBBackDraw::E_TICK].numberInt());
        int activity_id = obj[DBBackDraw::ACTIVITY_ID].numberInt();
        tick_map[activity_id] = info;
        is_has_data = true;

        MSG_USER("update activity tick %d begin[%d] end[%d]", activity_id, info.__begin_tick.sec(), info.__end_tick.sec());
    }
    if (is_has_data)
    {
        this->update_version();

        Proto31101601 req;
        for (ActivityTickMap::iterator iter = tick_map.begin(); iter != tick_map.end(); ++iter)
        {
            ActivityTickInfo &tick_info = iter->second;
            ProtoActivityTick *proto_tick = req.add_activity_list();
            proto_tick->set_activity_id(iter->first);
            proto_tick->set_begin_tick(tick_info.__begin_tick.sec());
            proto_tick->set_end_tick(tick_info.__end_tick.sec());
        }
        const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
        int i = 0;
        for (GameConfig::ServerList::const_iterator iter = server_list.begin();
                iter != server_list.end(); ++iter)
        {
            const ServerDetail &detail = *iter;
            if (detail.__server_type != SERVER_MAP)
                continue;
            if (detail.__scene_list.size() <= 0)
                continue;
            int scene_id = *(detail.__scene_list.begin());
            LOGIC_MONITOR->dispatch_to_scene(scene_id, &req);

            MSG_USER("sync activity tick %d %d", i, scene_id);
            ++i;
        }
    }

    return 0;
}

bool BackActivityTick::is_has_activity(const int activity_id)
{
    ActivityTickMap &tick_map = this->cur_activity_tick_map();
    return (tick_map.find(activity_id) != tick_map.end());
}

Time_Value BackActivityTick::activity_begin_tick(const int activity_id)
{
    ActivityTickMap &tick_map = this->cur_activity_tick_map();
    ActivityTickMap::iterator iter = tick_map.find(activity_id);
    if (iter == tick_map.end())
        return Time_Value::zero;

    return iter->second.__begin_tick;
}

Time_Value BackActivityTick::activity_end_tick(const int activity_id)
{
    ActivityTickMap &tick_map = this->cur_activity_tick_map();
    ActivityTickMap::iterator iter = tick_map.find(activity_id);
    if (iter == tick_map.end())
        return Time_Value::zero;

    return iter->second.__end_tick;
}

BackActivityTick::ActivityTickMap &BackActivityTick::next_activity_tick_map(void)
{
    return this->activity_tick_map_[(this->version_cnt_ + 1) % 2];
}

BackActivityTick::ActivityTickMap &BackActivityTick::cur_activity_tick_map(void)
{
    return this->activity_tick_map_[this->version_cnt_ % 2];
}

void BackActivityTick::update_version(void)
{
    this->version_cnt_ = (this->version_cnt_ + 1) % 2;
}

int BackActivityTick::sync_activity_tick(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31101601 *, request, msg, -1);

    ActivityTickMap &tick_map = this->next_activity_tick_map();
    ActivityTickInfo info;

    for (int i = 0; i < request->activity_list_size(); ++i)
    {
        info.reset();
        const ProtoActivityTick &proto_tick = request->activity_list(i);
        info.__begin_tick.sec(proto_tick.begin_tick());
        info.__end_tick.sec(proto_tick.end_tick());
        tick_map[proto_tick.activity_id()] = info;
    }

    this->update_version();

    return 0;
}

