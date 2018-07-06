/*
 * MMOWedding.cpp
 *
 * Created on: 2015-06-08 16:59
 *     Author: lyz
 */

#include "GameField.h"
#include "MMOWedding.h"
#include "WeddingMonitor.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "MMORole.h"

#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

MMOWedding::~MMOWedding(void)
{ /*NULL*/ }

int MMOWedding::load_all_wedding_detail(WeddingMonitor *monitor)
{
    WeddingMonitor::WeddingMap &wedding_role_map = monitor->wedding_by_role_map();
    WeddingMonitor::WeddingMap &wedding_id_map = monitor->wedding_by_id_map();

    auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBGWedding::COLLECTION);
    while (cursor->more())
    {
        BSONObj res = cursor->next();
        
        WeddingDetail *wedding_info = monitor->monitor()->wedding_detail_pool()->pop();
        wedding_info->__wedding_id = res[DBGWedding::ID].numberLong();
        wedding_info->__wedding_tick.sec(res[DBGWedding::WEDDING_TICK].numberInt());
        wedding_info->__partner_1.__role_id = res[DBGWedding::PARTNER_ONE].numberLong();
        wedding_info->__partner_2.__role_id = res[DBGWedding::PARTNER_TWO].numberLong();
        wedding_info->__day_wedding_times = res[DBGWedding::DAY_WEDDING_TIMES].numberInt();
        wedding_info->__day_refresh_tick.sec(res[DBGWedding::DAY_REFRESH_TICK].numberInt());
        wedding_info->__intimacy = res[DBGWedding::INTIMACY].numberInt();
        wedding_info->__history_intimacy = res[DBGWedding::HISTORY_INTIMACY].numberInt();
        wedding_info->__wedding_type = res[DBGWedding::WEDDING_TYPE].numberInt();
        wedding_info->__keepsake_id = res[DBGWedding::KEEPSAKE_ID].numberInt();
        wedding_info->__keepsake_level = res[DBGWedding::KEEPSAKE_LEVEL].numberInt();
        wedding_info->__keepsake_sublevel = res[DBGWedding::KEEPSAKE_SUBLEVEL].numberInt();
        wedding_info->__keepsake_progress = res[DBGWedding::KEEPSAKE_PROGRESS].numberDouble();

        MMORole::fetch_role_info_for_wedding( //
                wedding_info->__partner_1.__role_id, //
                wedding_info->__partner_1.__role_name, //
                wedding_info->__partner_1.__sex,
                wedding_info->__partner_1.__career);
        MMORole::fetch_role_info_for_wedding( //
                wedding_info->__partner_2.__role_id, //
                wedding_info->__partner_2.__role_name, //
                wedding_info->__partner_2.__sex,
                wedding_info->__partner_2.__career);

        wedding_info->__partner_1.__sweet_degree = res[DBGWedding::SWEET_DEGREE_1].numberInt();
        wedding_info->__partner_1.__ring_level = res[DBGWedding::RING_LEVEL_1].numberInt();
        wedding_info->__partner_1.__sys_level = res[DBGWedding::SYS_LEVEL_1].numberInt();
        wedding_info->__partner_1.__tree_level = res[DBGWedding::TREE_LEVEL_1].numberInt();
        wedding_info->__partner_1.__tick = res[DBGWedding::TICK_1].numberLong();
        wedding_info->__partner_1.__fetch_tick = res[DBGWedding::FETCH_TICK_1].numberLong();
        wedding_info->__partner_1.__once_reward = res[DBGWedding::ONCE_REWARD_1].numberInt();
        wedding_info->__partner_1.__left_times = res[DBGWedding::LEFT_TIMES_1].numberInt();

        wedding_info->__partner_2.__sweet_degree = res[DBGWedding::SWEET_DEGREE_2].numberInt();
        wedding_info->__partner_2.__ring_level = res[DBGWedding::RING_LEVEL_2].numberInt();
        wedding_info->__partner_2.__sys_level = res[DBGWedding::SYS_LEVEL_2].numberInt();
        wedding_info->__partner_2.__tree_level = res[DBGWedding::TREE_LEVEL_2].numberInt();
        wedding_info->__partner_2.__tick = res[DBGWedding::TICK_2].numberLong();
        wedding_info->__partner_2.__fetch_tick = res[DBGWedding::FETCH_TICK_2].numberLong();
        wedding_info->__partner_2.__once_reward = res[DBGWedding::ONCE_REWARD_2].numberInt();
        wedding_info->__partner_2.__left_times = res[DBGWedding::LEFT_TIMES_2].numberInt();


        wedding_id_map[wedding_info->__wedding_id] = wedding_info;
        wedding_role_map[wedding_info->__partner_1.__role_id] = wedding_info;
        wedding_role_map[wedding_info->__partner_2.__role_id] = wedding_info;
    }

    return 0;
}

int MMOWedding::update_data(WeddingDetail *wedding_info, MongoDataMap *mongo_data)
{
    BSONObjBuilder builder;
    builder << DBGWedding::WEDDING_TICK << int(wedding_info->__wedding_tick.sec())
        << DBGWedding::PARTNER_ONE << wedding_info->__partner_1.__role_id
        << DBGWedding::PARTNER_TWO << wedding_info->__partner_2.__role_id
        << DBGWedding::DAY_WEDDING_TIMES << wedding_info->__day_wedding_times
        << DBGWedding::DAY_REFRESH_TICK << int(wedding_info->__day_refresh_tick.sec())
        << DBGWedding::INTIMACY << wedding_info->__intimacy
        << DBGWedding::HISTORY_INTIMACY << wedding_info->__history_intimacy
        << DBGWedding::WEDDING_TYPE << wedding_info->__wedding_type
        << DBGWedding::KEEPSAKE_ID << wedding_info->__keepsake_id
        << DBGWedding::KEEPSAKE_LEVEL << wedding_info->__keepsake_level
        << DBGWedding::KEEPSAKE_SUBLEVEL << wedding_info->__keepsake_sublevel
        << DBGWedding::KEEPSAKE_PROGRESS << wedding_info->__keepsake_progress
        << DBGWedding::SWEET_DEGREE_1 << wedding_info->__partner_1.__sweet_degree
        << DBGWedding::RING_LEVEL_1 << wedding_info->__partner_1.__ring_level
        << DBGWedding::SYS_LEVEL_1 << wedding_info->__partner_1.__sys_level
        << DBGWedding::TREE_LEVEL_1 << wedding_info->__partner_1.__tree_level
        << DBGWedding::TICK_1 << wedding_info->__partner_1.__tick
        << DBGWedding::FETCH_TICK_1 << wedding_info->__partner_1.__fetch_tick
        << DBGWedding::ONCE_REWARD_1 << wedding_info->__partner_1.__once_reward
        << DBGWedding::LEFT_TIMES_1 << wedding_info->__partner_1.__left_times

        << DBGWedding::SWEET_DEGREE_2 << wedding_info->__partner_2.__sweet_degree
        << DBGWedding::RING_LEVEL_2 << wedding_info->__partner_2.__ring_level
        << DBGWedding::SYS_LEVEL_2 << wedding_info->__partner_2.__sys_level
        << DBGWedding::TREE_LEVEL_2 << wedding_info->__partner_2.__tree_level
        << DBGWedding::TICK_2 << wedding_info->__partner_2.__tick
        << DBGWedding::FETCH_TICK_2 << wedding_info->__partner_2.__fetch_tick
        << DBGWedding::ONCE_REWARD_2 << wedding_info->__partner_2.__once_reward
        << DBGWedding::LEFT_TIMES_2 << wedding_info->__partner_2.__left_times;

    mongo_data->push_update(DBGWedding::COLLECTION,
            BSON(DBGWedding::ID << wedding_info->__wedding_id),
            builder.obj(), true);
    return 0;
}

int MMOWedding::remove_data(const long long int wedding_id, MongoDataMap *mongo_data)
{
    mongo_data->push_remove(DBGWedding::COLLECTION,
            BSON(DBGWedding::ID << wedding_id));
    return 0;
}

int MMOWedding::load_player_wedding(LogicPlayer *player)
{
    BSONObj res = this->conection().findOne(DBPlayerWedding::COLLECTION,
    		QUERY(DBPlayerWedding::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, 0);

    PlayerWeddingDetail &self_wedding_info = player->self_wedding_info();
    self_wedding_info.__wedding_id = res[DBPlayerWedding::WEDDING_ID].numberLong();

//    //出错的id
//	if (0 > self_wedding_info.__wedding_id)
//	{
//		WeddingDetail* weddetial = player->wedding_detail();
//		if (NULL != weddetial)
//		{
//			int wedding_id = weddetial->__wedding_id;
//
//			PlayerWeddingDetail &self_wedding_info = player->self_wedding_info();
//			self_wedding_info.__wedding_id = wedding_id;
//
//			LogicRoleDetail &role_detail = player->role_detail();
//			role_detail.__wedding_id = weddetial->__wedding_id;
//			role_detail.__wedding_type = weddetial->__wedding_type;
//			role_detail.__partner_id = weddetial->__partner_2.__role_id;
//			role_detail.__partner_name = weddetial->__partner_2.__role_name;
//		}
//	}

    self_wedding_info.__total_recv_flower = res[DBPlayerWedding::RECV_FLOWER].numberInt();
    self_wedding_info.__total_send_flower = res[DBPlayerWedding::SEND_FLOWER].numberInt();
    self_wedding_info.__is_has_ring = res[DBPlayerWedding::IS_HAS_RING].numberInt();
    self_wedding_info.__side_fashion_id = res[DBPlayerWedding::SIDE_FASHION_ID].numberInt();
    self_wedding_info.__side_fashion_color = res[DBPlayerWedding::SIDE_FASHION_COLOR].numberInt();

    GameCommon::bson_to_map(self_wedding_info.__wedding_label_map,
    		res.getObjectField(DBPlayerWedding::WEDDING_LABEL.c_str()));

    BSONObjIterator iter(res.getObjectField(DBPlayerWedding::WEDDING_PROPERTY.c_str()));
    int type =	WED_RING;
    while (iter.more())
    {
    	BSONObj obj = iter.next().embeddedObject();
    	PlayerWeddingDetail::wedding_property &info = self_wedding_info.__wedding_pro_map[type];
    	info.__exp = obj[DBPlayerWedding::Wedding_detail::EXP].numberInt();
    	info.__level = obj[DBPlayerWedding::Wedding_detail::LEVEL].numberInt();
    	info.__side_level = obj[DBPlayerWedding::Wedding_detail::SIDE_LEVEL].numberInt();
    	info.__side_order = obj[DBPlayerWedding::Wedding_detail::SIDE_ORDER].numberInt();
    	type++;
    }

    {
        BSONObjIterator iter(res.getObjectField(DBPlayerWedding::INTIMACY.c_str()));
        while (iter.more())
        {
            Int64 role_id = iter.next().numberLong();
            if (iter.more() == false)
                break;
            self_wedding_info.__intimacy_map[role_id] = int(iter.next().numberLong());
            if (self_wedding_info.__intimacy_map[role_id] < 0)
            {
            	self_wedding_info.__intimacy_map[role_id] = 0;
            }
        }
    }
    return 0;
}

int MMOWedding::update_data(LogicPlayer *player, MongoDataMap *mongo_data)
{
    PlayerWeddingDetail &self_wedding_info = player->self_wedding_info();
    std::vector<Int64> intimacy_vc;
    {
        for (PlayerWeddingDetail::RoleValueMap::iterator iter = self_wedding_info.__intimacy_map.begin();
                iter != self_wedding_info.__intimacy_map.end(); ++iter)
        {
            intimacy_vc.push_back(iter->first);
            intimacy_vc.push_back(iter->second);
        }
    }
	std::vector<BSONObj> bson_vec;
	for(int type = WED_RING; type < WED_TYPE_END; type++)
	{
    	PlayerWeddingDetail::wedding_property &info = self_wedding_info.__wedding_pro_map[type];
		bson_vec.push_back(BSON(DBPlayerWedding::Wedding_detail::LEVEL << info.__level
						<< DBPlayerWedding::Wedding_detail::EXP << info.__exp
						<< DBPlayerWedding::Wedding_detail::SIDE_LEVEL << info.__side_level
						<< DBPlayerWedding::Wedding_detail::SIDE_ORDER << info.__side_order));
	}

	BSONVec label_map_bson;
	GameCommon::map_to_bson(label_map_bson, self_wedding_info.__wedding_label_map);

    BSONObjBuilder builder;
    builder << DBPlayerWedding::WEDDING_ID << self_wedding_info.__wedding_id
        << DBPlayerWedding::INTIMACY << intimacy_vc
        << DBPlayerWedding::RECV_FLOWER << player->total_recv_flower()
        << DBPlayerWedding::SEND_FLOWER << player->total_send_flower()
        << DBPlayerWedding::IS_HAS_RING << self_wedding_info.__is_has_ring
        << DBPlayerWedding::SIDE_FASHION_ID << self_wedding_info.__side_fashion_id
        << DBPlayerWedding::SIDE_FASHION_COLOR << self_wedding_info.__side_fashion_color
        << DBPlayerWedding::WEDDING_PROPERTY << bson_vec
        << DBPlayerWedding::WEDDING_LABEL << label_map_bson;

    mongo_data->push_update(DBPlayerWedding::COLLECTION, 
            BSON(DBPlayerWedding::ID << player->role_id()),
            builder.obj(), true);

    return 0;
}

void MMOWedding::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBGWedding::COLLECTION, BSON(DBGWedding::ID << 1), true);
    this->conection().ensureIndex(DBGWedding::COLLECTION, BSON(DBGWedding::PARTNER_ONE << 1), false);
    this->conection().ensureIndex(DBGWedding::COLLECTION, BSON(DBGWedding::PARTNER_TWO << 1), false);

    this->conection().ensureIndex(DBPlayerWedding::COLLECTION, BSON(DBPlayerWedding::ID << 1), true);
END_CATCH
}

