/*
 * SystemChannel.cpp
 *
 *  Created on: Oct 15, 2014
 *      Author: jinxing
 */

#include "SystemChannel.h"

SystemChannel::SystemChannel()
{
	// TODO Auto-generated constructor stub

}

SystemChannel::~SystemChannel()
{
	// TODO Auto-generated destructor stub
}

int SystemChannel::channel_type(void)
{
	return CHANNEL_SYSTEM;
}

int SystemChannel::init(ChannelAgency *agency)
{
	return BaseChannel::init(agency, 0);
}
