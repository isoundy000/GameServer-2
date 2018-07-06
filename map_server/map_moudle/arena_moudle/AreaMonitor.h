/*
 * AreaMonitor.h
 *
 *  Created on: Aug 19, 2014
 *      Author: peizhibi
 */

#ifndef AREAMONITOR_H_
#define AREAMONITOR_H_

#include "MapMapStruct.h"

class AreaMonitor
{
public:
	AreaMonitor();
	~AreaMonitor();

	int create_area_field(Message* msg);
	int recycle_area_field(AreaField* area_field);

	AreaField* find_area_field(int area_index);

private:
	PoolPackage<AreaField>* area_field_package_;
};

typedef Singleton<AreaMonitor> 		AreaMonitorSingle;
#define AREA_MONITOR           		AreaMonitorSingle::instance()

#endif /* AREAMONITOR_H_ */

