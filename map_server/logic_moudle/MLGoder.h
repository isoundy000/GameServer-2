/*
 * MLGoder.h
 *
 *  Created on: May 23, 2017
 *      Author: peizhibi
 */

#ifndef MLGODER_H_
#define MLGODER_H_

#include "MLPacker.h"

struct GoderDetail
{
	int open_;
	int grade_;

	FightProperty fight_prop_;

	void reset();
	const Json::Value& conf();
};

class MLGoder : virtual public MLPacker
{
public:
	MLGoder();
	virtual ~MLGoder();

	void reset_goder();
	void add_goder_grade();
	void calculate_goder_prop();
	void refresh_goder_property(int type = 0);

	int fetch_goder_info(int equip = true);
	int goder_operate();

	int open_goder();
	int goder_upgrade();

protected:
	GoderDetail goder_detail_;
};

#endif /* MLGODER_H_ */
