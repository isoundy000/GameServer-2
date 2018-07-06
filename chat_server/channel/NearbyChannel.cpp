/*
 * NearbyChannel.cpp
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#include "NearbyChannel.h"

NearbyChannel::NearbyChannel()
{
	// TODO Auto-generated constructor stub

}

NearbyChannel::~NearbyChannel()
{
	// TODO Auto-generated destructor stub
}

void NearbyChannel::reset(void)
{
	BaseChannel::reset();
}

int NearbyChannel::init(ChannelAgency *agency,int64_t channel_id)
{
	return BaseChannel::init(agency,channel_id);
}

int NearbyChannel::channel_type(void)
{
	return CHANNEL_SCENE;
}

int NearbyChannel::notify(void)
{
	return BaseChannel::notify();
}
