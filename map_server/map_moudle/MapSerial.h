/*
 * MapSerial.h
 *
 * Created on: 2013-06-08 17:10
 *     Author: lyz
 */

#ifndef _MAPSERIAL_H_
#define _MAPSERIAL_H_

#include "MapStruct.h"
#include "SerialOperate.h"

class MapSerial : public SerialOperate
{
public:
    typedef boost::unordered_map<int, SerialInfo> SerialMap;

public:
    void reset(void);
    SerialInfo &exp_serial(const int serial_type);
    virtual bool is_validate_exp_serial(const int serial_type, const int exp);

    SerialMap &exp_serial_map(void);

protected:
    SerialMap exp_serial_map_;
};

#endif //_MAPSERIAL_H_
