/*
 * TeamChannel.cpp
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#include "TeamChannel.h"

TeamChannel::TeamChannel()
{
	// TODO Auto-generated constructor stub

}

TeamChannel::~TeamChannel()
{
	// TODO Auto-generated destructor stub
}

void TeamChannel::reset(void)
{
	BaseChannel::reset();
}

int TeamChannel::init(ChannelAgency *agency,int64_t channel_id)
{
	return BaseChannel::init(agency,channel_id);
}

int TeamChannel::channel_type(void)
{
	return CHANNEL_TEAM;
}

int TeamChannel::notify(void)
{
	return BaseChannel::notify();
}
