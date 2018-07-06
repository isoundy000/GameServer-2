/*
 * SystemChannel.h
 *
 *  Created on: Oct 15, 2014
 *      Author: jinxing
 */

#ifndef SYSTEMCHANNEL_H_
#define SYSTEMCHANNEL_H_

#include "BaseChannel.h"

class SystemChannel: public BaseChannel
{
public:
	SystemChannel();
	virtual ~SystemChannel();

	virtual int init(ChannelAgency *agency);
	virtual int channel_type(void);
};

#endif /* SYSTEMCHANNEL_H_ */
