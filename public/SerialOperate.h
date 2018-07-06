/*
 * SerialOperate.h
 *
 * Created on: 2013-06-09 10:27
 *     Author: lyz
 */

#ifndef _SERIALOPERATE_H_
#define _SERIALOPERATE_H_

#include "PubStruct.h"

class SerialOperate
{
public:
	virtual ~SerialOperate(){}

    SerialInfo &item_serial(const int serial_type, const int item_id);
    virtual bool is_validate_item_serial(const int serial_type, const int item_id, const int item_amount);

    virtual SerialInfo &money_serial(const int serial_type);
    virtual bool is_validate_money_serial(const int serial_type, const int gold = 0, const int bind_gold = 0, const int copper = 0);
protected:
    SerialInfo NULL_SERIAL_INFO;
};

inline SerialInfo &SerialOperate::item_serial(const int serial_type, const int item_id)
{
    return SerialOperate::NULL_SERIAL_INFO;
}

inline bool SerialOperate::is_validate_item_serial(const int serial_type, const int item_id, const int item_amount)
{
    return false;
}

inline SerialInfo &SerialOperate::money_serial(const int serial_type)
{
    return SerialOperate::NULL_SERIAL_INFO;
}

inline bool SerialOperate::is_validate_money_serial(const int serial_type, const int gold, const int bind_gold, const int copper)
{
    return false;
}

#endif //_SERIALOPERATE_H_
