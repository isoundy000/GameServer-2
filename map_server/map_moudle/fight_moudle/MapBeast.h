/*
 * MapBeast.h
 *
 *  Created on: Nov 15, 2013
 *      Author: peizhibi
 */

#ifndef MAPBEAST_H_
#define MAPBEAST_H_

#include "AutoMapFighter.h"

class Proto80400103;

class MapBeast : public AutoMapFighter
{
public:
	MapBeast(void);
	virtual ~MapBeast(void);

    virtual int team_id(void);
    virtual int client_sid(void);
	virtual int64_t entity_id(void);
	virtual Int64 league_id(void);

	virtual int auto_action_timeout(const Time_Value& now_time);
	virtual int make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate);

    virtual const char* name();
    virtual GameFighter* fetch_hurt_figher();
    virtual double fetch_beast_hurt_rate();
    virtual double fetch_beast_hurt_value();

    virtual BlockIndexType cur_block_index(void);
    virtual void push_attack_interval();
    virtual void set_cur_location(const MoverCoord& coord);

public:
	int enter_scene();
	int exit_scene();

	int notify_enter_info();
	int notify_exit_info();

	int refresh_fight_property(Message* msg);
	int refresh_fight_property_item(int offset, const ProtoPairObj& pair_obj);

	int is_a_beast();
	int is_attack_beast();
	int set_beast_offline(MapPlayerEx* player);

	void reset();
	void recycle_all_skill_map();

	int update_beast_speed();
	int update_beast_info(int type, int value = 0);

	Int64 beast_id();
	Int64 master_id();
	int beast_sort();

	MapPlayerEx* fetch_master();
	BeastDetail* fetch_beast_detail();

	void fetch_prop_map(IntMap& prop_map);
	void fetch_prop_map(IDMap& prop_map, int offset);
	void fetch_skill_map(IntVec& skill_set);
	void fetch_other_shape_info(Proto80400103* respond);

	virtual int validate_movable(const MoverCoord &step);
	virtual int insert_master_status(const int status,
	            const double interval, const double last, const int accumulate_times = 0,
	    		const double val1 = 0.0, const double val2 = 0.0, const double val3 = 0.0,
	            const double val4 = 0.0, const double val5 = 0.0);

private:
	MapPlayerEx* master_;
	BeastDetail beast_detial_;
};

#endif /* MAPBEAST_H_ */
