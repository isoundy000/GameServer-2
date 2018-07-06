/*
 * MapLogicSerial.cpp
 *
 * Created on: 2013-06-09 10:45
 *     Author: lyz
 */

#include "MapLogicSerial.h"

void MapLogicSerial::reset(void)
{
    this->item_serial_map_.clear();
    this->money_serial_map_.clear();
}

SerialInfo &MapLogicSerial::item_serial(const int serial_type, const int item_id, const int item_amount)
{
    SerialInfo &serial_info = this->item_serial_map_[serial_type][item_id];
    serial_info.__value += item_amount;
    
    const Json::Value &limit_json = CONFIG_INSTANCE->item_limit(serial_type);
    Time_Value nowtime = Time_Value::gettimeofday();
    if (limit_json.isMember("day_fresh") == true)
    {
        if (serial_info.__fresh_tick < nowtime.sec())
        {
            serial_info.__fresh_tick = next_day(limit_json["day_fresh"][0u].asInt(), limit_json["day_fresh"][1u].asInt(), nowtime).sec();
            serial_info.__value = 0;
        }
    }
    else
    {
        serial_info.__fresh_tick = nowtime.sec();
    }

    return serial_info;
}

bool MapLogicSerial::is_validate_item_serial(const int serial_type, const int item_id, const int item_amount)
{
    int limit_num = CONFIG_INSTANCE->item_num_limit(serial_type, item_id);
    JUDGE_RETURN(limit_num > 0, true);

    SerialInfo &serial_info = this->item_serial(serial_type, item_id, 0);

    const Json::Value &limit_json = CONFIG_INSTANCE->item_limit(serial_type);
    if (serial_info.__value + item_amount > limit_num)
    {
        // TODO; add notify serial too many;
        
        MSG_USER("WARNING item too more %d %d %d %d", serial_type, serial_info.__value, item_amount, limit_num);
        if (limit_json["force_stop"].asInt() == 1 ||
                (serial_info.__value + item_amount) > (limit_num * 3))
        {
            MSG_USER("WARNING forbit item too more %d %d %d %d", serial_type, serial_info.__value, item_amount, limit_num);
            return false;
        }
    }

    return true;
}

SerialInfo &MapLogicSerial::money_serial(const int serial_type)
{
    SerialInfo &serial_info = this->money_serial_map_[serial_type];
    
    const Json::Value &limit_json = CONFIG_INSTANCE->money_limit(serial_type);
    Time_Value nowtime = Time_Value::gettimeofday();
    if (limit_json.isMember("day_fresh") == true)
    {
        if (serial_info.__fresh_tick < nowtime.sec())
        {
            serial_info.__fresh_tick = next_day(limit_json["day_fresh"][0u].asInt(), limit_json["day_fresh"][1u].asInt(), nowtime).sec();
            serial_info.__copper = 0;
            serial_info.__gold = 0;
            serial_info.__bind_gold = 0;
            serial_info.__bind_copper = 0;
        }
    }
    else
    {
        serial_info.__fresh_tick = nowtime.sec();
    }

    return serial_info;
}

bool MapLogicSerial::is_validate_money_serial(const int serial_type, const Money &money)
{
    return true;
}

MapLogicSerial::ItemSerialMap &MapLogicSerial::item_serial_map(void)
{
	return this->item_serial_map_;
}

MapLogicSerial::SerialMap &MapLogicSerial::money_serial_map(void)
{
	return this->money_serial_map_;
}
