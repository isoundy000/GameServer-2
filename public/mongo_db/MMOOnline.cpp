/*
 * MMOOnline.cpp
 *
 * Created on: 2013-07-08 10:38
 *     Author: lyz
 */

#include "GameField.h"
#include "LogicOnline.h"
#include "MMOOnline.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOOnline::~MMOOnline(void)
{ /*NULL*/ }

int MMOOnline::load_player_online(int64_t role_id, LogicOnline *online)
{
	JUDGE_RETURN(role_id > 0, 0);

    BSONObj res = this->conection().findOne(DBOnline::COLLECTION,
            QUERY(DBOnline::ID << (long long int)role_id));
    JUDGE_RETURN(res.isEmpty() == false, 0);

    LogicOnline::OnlineDetail &detail = online->online_detail();
    detail.__sign_in_tick = res[DBOnline::SIGN_IN_TICK].numberInt();
    detail.__sign_out_tick = res[DBOnline::SIGN_OUT_TICK].numberInt();

    detail.__total_online_tick = res[DBOnline::TOTAL_ONLINE].numberLong();
    detail.__day_online_tick = res[DBOnline::DAY_ONLINE].numberLong();
    detail.__week_online_tick = res[DBOnline::WEEK_ONLINE].numberLong();
    detail.__month_online_tick = res[DBOnline::MONTH_ONLINE].numberLong();
    detail.__year_online_tick = res[DBOnline::YEAR_ONLINE].numberLong();

    detail.__day_refresh_tick = Time_Value(res[DBOnline::DAY_REFRESH].numberInt());
    detail.__week_refresh_tick = Time_Value(res[DBOnline::WEEK_REFRESH].numberInt());
    detail.__month_refresh_tick = Time_Value(res[DBOnline::MONTH_REFRESH].numberInt());
    detail.__year_refresh_tick = Time_Value(res[DBOnline::YEAR_REFRESH].numberInt());
    
    return 0;
}

int MMOOnline::update_player_login_tick(int64_t role_id)
{
BEGIN_CATCH
	BSONObjBuilder builder;
	builder << DBOnline::IS_ONLINE << 1
			<< DBOnline::SIGN_IN_TICK << int(Time_Value::gettimeofday().sec());

	this->conection().update(DBOnline::COLLECTION,
            QUERY(DBOnline::ID << (long long int)role_id),
            BSON("$set" << builder.obj()), true);
END_CATCH
    MSG_USER("Update player online flag to online %ld", role_id);
    return -1;
}

int MMOOnline::update_all_player_offline(void)
{
BEGIN_CATCH

	BSONObjBuilder builder;
	builder << DBOnline::IS_ONLINE << 0
			<< DBOnline::SIGN_OUT_TICK << int(Time_Value::gettimeofday().sec());

	this->conection().update(DBOnline::COLLECTION,
				QUERY(DBOnline::IS_ONLINE << 1),
				BSON("$set" << builder.obj()));

	return 0;

END_CATCH
	MSG_USER("ERROR update player online flag to offline");
	return -1;
}

int MMOOnline::update_data(const int64_t role_id, LogicOnline *online, MongoDataMap *mongo_data)
{
    LogicOnline::OnlineDetail &detail = online->online_detail();

    mongo_data->push_update(DBOnline::COLLECTION,
            BSON(DBOnline::ID << (long long int)role_id),
            BSON(DBOnline::SIGN_IN_TICK << detail.__sign_in_tick
            	<< DBOnline::IS_ONLINE << detail.__is_online
                << DBOnline::SIGN_OUT_TICK << detail.__sign_out_tick
                << DBOnline::TOTAL_ONLINE << (long long int)detail.__total_online_tick
                << DBOnline::DAY_ONLINE << (long long int)detail.__day_online_tick
                << DBOnline::WEEK_ONLINE << (long long int)detail.__week_online_tick
                << DBOnline::MONTH_ONLINE << (long long int)detail.__month_online_tick
                << DBOnline::YEAR_ONLINE << (long long int)detail.__year_online_tick
                << DBOnline::DAY_REFRESH << int(detail.__day_refresh_tick.sec())
                << DBOnline::WEEK_REFRESH << int(detail.__week_refresh_tick.sec())
                << DBOnline::MONTH_REFRESH << int(detail.__month_refresh_tick.sec())
                << DBOnline::YEAR_REFRESH << int(detail.__year_refresh_tick.sec())));
    return 0;
}

void MMOOnline::ensure_all_index(void)
{
    BEGIN_CATCH
        this->conection().ensureIndex(DBOnline::COLLECTION, BSON(DBOnline::ID << 1), true);
    END_CATCH
}


