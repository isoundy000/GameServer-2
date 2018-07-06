/*
 * BackBrocast.h
 *
 *  Created on: Jun 7, 2014
 *      Author: louis
 */

#ifndef BACKBROCAST_H_
#define BACKBROCAST_H_

#include "MongoTable.h"

class BackstageBrocastRecord;
class BackstageBrocastControl;
class BackActivityTick;
class CustomerServiceRecord;

class BackRecharge : public MongoTable
{
public:
	BackRecharge();
	virtual ~BackRecharge();

	static int update_data(MongoDataMap* data_map, const int order_id);
	static int recharge_test(MongoDataMap* data_map, int64_t role_id, const std::string& account, int money);

protected:
	void ensure_all_index(void);
};

class BackBrocast : public MongoTable
{
public:
	BackBrocast();
	virtual ~BackBrocast();

	static int updata_data(BackstageBrocastRecord* record, MongoDataMap* data_map);
	int request_load_data(void);

private:
	int update_flag(const int record_id, BSONObj& res);

protected:
	virtual void ensure_all_index(void);
};

class BackDraw : public MongoTable
{
public:
    int load_activity_tick(MongoDataMap *data_map);

    static int load_activity_tick_at_init(BackActivityTick *back_act_tick);

protected:
    virtual void ensure_all_index(void);
};

class BackSerial : public MongoTable
{
public:
    virtual ~BackSerial(void);

    int load_map_serial(MapPlayerEx *player);
    int load_map_logic_serial(MapLogicPlayer *player);

    static int update_data(MapPlayerEx *player, MongoDataMap *data_map);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);
protected:
    virtual void ensure_all_index(void);
};


class BackCustomerSVC : public MongoTable
{
public:
	BackCustomerSVC();
	virtual ~BackCustomerSVC();

	int load_player_customer_service_detail_when_init(LogicPlayer* player);
	int load_customer_way_when_init(LogicPlayer* player);
	static int save_global_customer_service_index(int64_t index);
	static int request_load_customer_service_record(LogicPlayer* player, MongoDataMap* data_map);
	static int request_remove_customer_service_record(const int64_t record_id, MongoDataMap* data_map);
	static int update_player_customer_service_detail(LogicPlayer* player, MongoDataMap* data_map);
	static int update_backstage_customer_service_record(CustomerServiceRecord* record, MongoDataMap* data_map);
protected:
	virtual void ensure_all_index(void);
};


#endif /* BACKBROCAST_H_ */
