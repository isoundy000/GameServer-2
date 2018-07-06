/*
 * TravelScript.h
 *
 *  Created on: Nov 14, 2016
 *      Author: peizhibi
 */

#ifndef TRAVELSCRIPT_H_
#define TRAVELSCRIPT_H_

#include "BaseScript.h"

class TravelScript : public BaseScript
{
public:
	TravelScript();
	virtual ~TravelScript();

	virtual void reset(void);
	virtual int notify_fight_hurt_detail();

protected:
    virtual void recycle_self_to_pool(void);
};

#endif /* TRAVELSCRIPT_H_ */
