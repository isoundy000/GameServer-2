/*
 * MapSwordPool.h
 *
 *  Created on: 2016年11月3日
 *      Author: lyw
 */

#ifndef MAPSWORDPOOL_H_
#define MAPSWORDPOOL_H_

#include "GameFighter.h"

class MapSwordPool : virtual public GameFighter
{
public:
	MapSwordPool();
	virtual ~MapSwordPool();
	void reset_spool(void);

	int update_spool_info(Message* msg);
	int refresh_spool_shape(SwordPoolDetail& spool_detail);
	int fetch_spool_style_lvl();

private:
	SwordPoolDetail spool_detail_;
};

#endif /* MAPSWORDPOOL_H_ */
