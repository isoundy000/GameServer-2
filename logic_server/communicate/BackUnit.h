/*
 * File Name: BackUnit.h
 * 
 * Created on: 2016-08-12 20:26:27
 * Author: glendy
 * 
 * Last Modified: 2016-08-12 20:40:43
 * Description: 
 */

#ifndef _BACKUNIT_H_
#define _BACKUNIT_H_

#include "BaseUnit.h"

class BackUnit : public BaseUnit
{
public:
    virtual int type(void);
    
protected:
    virtual UnitMessage *pop_unit_message(void);
    virtual int push_unit_message(UnitMessage *msg);

    virtual int process_block(UnitMessage *unit_msg);

    int process_back_mail_request(void);

};

#endif //BACKUNIT_H_
