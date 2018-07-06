/*
 * WorldChannel.cpp
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#include "WorldChannel.h"

WorldChannel::WorldChannel(int world_type)
{
	this->world_type_ = world_type;
}

WorldChannel::~WorldChannel()
{
	// TODO Auto-generated destructor stub
}

void WorldChannel::reset(void)
{
	BaseChannel::reset();
}

int WorldChannel::init(ChannelAgency *agency,int64_t channel_id)
{
	return BaseChannel::init(agency,channel_id);
}

int WorldChannel::channel_type(void)
{
	return this->world_type_;
}

int WorldChannel::notify(void)
{
	return BaseChannel::notify();
}
