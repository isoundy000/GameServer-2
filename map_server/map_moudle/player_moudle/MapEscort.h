/*
 * MapEscort.h
 *
 *  Created on: 2016年10月24日
 *      Author: lzy
 */

#ifndef MAPESCORT_H_
#define MAPESCORT_H_

#include "GameFighter.h"

struct Escort_detail
{
	Int64 car_index_;
	LongSet protect_map;

	Int64 protect_id;
	int escort_type_;
	int escort_times_;
	int total_exp_;
	Int64 start_tick_;

	int till;
	int target_level;

	Escort_detail();
	int escort_type();

	void reset();
	void set_escort_type(int type);
};

class MapEscort : virtual public GameFighter
{
public:
	MapEscort();
	virtual ~MapEscort();
	Escort_detail &get_escort_detail();

	void reset();
	int generate_escort_info();
	int start_protect_player(Int64 player_id);
	int protect_someone(Int64 protect_id);
	int stop_protect_player();

	int get_activity_status(const int from = 1);
	int request_get_activity_status(Message* msg);
	int sync_transfer_escort();
	int read_transfer_escort(Message* msg);

protected:
	Escort_detail escort_detail_;

};


#endif /* MAPESCORT_H_ */
