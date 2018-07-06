/*
 * NearbyChannel.h
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#ifndef NEARBYCHANNEL_H_
#define NEARBYCHANNEL_H_

#include "BaseChannel.h"

class NearbyChannel: public BaseChannel
{
public:
	NearbyChannel();
	virtual ~NearbyChannel();

	virtual int init(ChannelAgency *agency,int64_t channel_id);
	virtual int channel_type(void);
	virtual int notify(void);
	virtual void reset(void);
};

#endif /* NEARBYCHANNEL_H_ */
