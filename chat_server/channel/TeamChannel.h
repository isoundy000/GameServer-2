/*
 * TeamChannel.h
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#ifndef TEAMCHANNEL_H_
#define TEAMCHANNEL_H_

#include "BaseChannel.h"

class TeamChannel: public BaseChannel
{
public:
	TeamChannel();
	virtual ~TeamChannel();

	virtual int init(ChannelAgency *agency,int64_t channel_id);
	virtual int channel_type(void);
	virtual int notify(void);
	virtual void reset(void);
};

#endif /* TEAMCHANNEL_H_ */
