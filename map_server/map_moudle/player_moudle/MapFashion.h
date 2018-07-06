/*
 * MapFashion.h
 *
 *  Created on: 2017年1月23日
 *      Author: lyw
 */

#ifndef MAPFASHION_H_
#define MAPFASHION_H_

#include "GameFighter.h"

class MapFashion : virtual public GameFighter
{
public:
	MapFashion();
	virtual ~MapFashion();
	void reset_fashion(void);

	int update_fashion_info(Message* msg);
	int refresh_fashion_shape();
	int fetch_fashion_id();
	int fetch_fashion_color();

	RoleFashion &fashion_detail();

private:
	RoleFashion fashion_detail_;
};

#endif /* MAPFASHION_H_ */
