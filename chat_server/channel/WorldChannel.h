/*
 * WorldChannel.h
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#ifndef WORLDCHANNEL_H_
#define WORLDCHANNEL_H_

#include "BaseChannel.h"

class WorldChannel: public BaseChannel
{
public:
	WorldChannel(int word_type);
	virtual ~WorldChannel();

	virtual int init(ChannelAgency *agency,int64_t channel_id=0);
	virtual int channel_type(void);
	virtual int notify(void);
	virtual void reset(void);

private:
	int world_type_;
};

#endif /* WORLDCHANNEL_H_ */
