/*
 * MapSerial.cpp
 *
 * Created on: 2013-06-08 17:33
 *     Author: lyz
 */

#include "MapSerial.h"

void MapSerial::reset(void)
{
    this->exp_serial_map_.clear();
}

SerialInfo &MapSerial::exp_serial(const int serial_type)
{
    SerialInfo &serial_info = this->exp_serial_map_[serial_type];

    const Json::Value &limit_json = CONFIG_INSTANCE->exp_limit(serial_type);
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

bool MapSerial::is_validate_exp_serial(const int serial_type, const int exp)
{
    const Json::Value &limit_json = CONFIG_INSTANCE->exp_limit(serial_type);
    if (limit_json == Json::Value::null)
        return true;

    int64_t limit_value = limit_json["limit"].asDouble();
    SerialInfo &serial_info = this->exp_serial(serial_type);
    if (serial_info.__value + exp > limit_value)
    {
        MSG_USER("WARNING experience too more %d (%d %d %d)",
                serial_type, serial_info.__value, exp, limit_value);
        if (limit_json["force_stop"].asInt() == 1 ||
                (serial_info.__value + exp > limit_value * 3))
        {
            MSG_USER("WARNING forbit experience too more %d (%d %d %d)",
                    serial_type, serial_info.__value, exp, limit_value);
            return false;
        }
    }
    return true;
}

MapSerial::SerialMap &MapSerial::exp_serial_map(void)
{
    return this->exp_serial_map_;
}

