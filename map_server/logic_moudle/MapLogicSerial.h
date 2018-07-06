/*
 * MapLogicSerial.h
 *
 * Created on: 2013-06-09 10:42
 *     Author: lyz
 */

#ifndef _MAPLOGICSERIAL_H_
#define _MAPLOGICSERIAL_H_

#include "MapStruct.h"
#include "SerialOperate.h"

class MapLogicSerial : public SerialOperate
{
public:
    typedef boost::unordered_map<int, SerialInfo> SerialMap;
    typedef boost::unordered_map<int, SerialMap> ItemSerialMap;

public:
    void reset(void);
    virtual SerialInfo &item_serial(const int serial_type, const int item_id, const int item_amount);
    virtual bool is_validate_item_serial(const int serial_type, const int item_id, const int item_amout);

    virtual SerialInfo &money_serial(const int serial_type);
    virtual bool is_validate_money_serial(const int serial_type, const Money &money);

    virtual ItemSerialMap &item_serial_map(void);
    virtual SerialMap &money_serial_map(void);

protected:
    ItemSerialMap item_serial_map_;
    SerialMap money_serial_map_;
};

#endif //_MAPLOGICSERIAL_H_
