/*
 * MapEquiper.h
 *
 *  Created on: 2013-12-13
 *      Author: louis
 */

#ifndef MAPEQUIPER_H_
#define MAPEQUIPER_H_

#include "GameFighter.h"
#include "GameHeader.h"
#include "MapMapStruct.h"

class MapEquiper : virtual public GameFighter
{
public:
	MapEquiper();
	virtual ~MapEquiper();

	int reset_shape(void);

	//shape
	int refresh_player_equip_shape(Message* msg);
	ShapeDetail& shape_detail(void);
	int get_shape_item_id(const int part);

	MapEquipDetail& map_equip_detail();

	//label
	virtual int refresh_player_cur_label(Message* msg);
	std::map<int, Int64>& label_info(void);

	int get_cur_label(void);
	int request_map_logic_add_label(int label_id, int type = 0);
	int sync_arena_shape_info();
	void check_and_fix_label();

	//transfer sync between scenes
	int sync_transfer_shape_and_label();
	int read_transfer_shape_and_label(Message* msg);

	int refresh_player_magic_weapon_skill(Message *msg);
	int refresh_player_magic_weapon_shape(Message *msg);
	int init_magic_weapon_skill(const int magic_skill_id,const int magic_skill_lvl);
    int magic_weapon_id();
    int magic_weapon_lvl();
    void set_magic_weapon_id(int id);
    void set_magic_weapon_lvl(int lvl);
    int magic_weapon_curr_skill_id();

    int refresh_equip_refine_lvl(Message* msg);
    void set_equip_refine_lvl(int lvl);

    int refresh_fairy_act(Message* msg);
    void make_up_fairy_act(ProtoPairObj* pair);

private:
    MapEquipDetail equip_detail_;
	ShapeDetail __shape_map;
	std::map<int, Int64> __label_info;

    int magic_weapon_id_;
    int magic_weapon_rank_lvl_;
    int talisman_pre_skill_id_;
    int talisman_pre_skill_lvl_;
    int equip_refine_lvl_;
    IntPair fairy_act_info_;
};

#endif /* MAPEQUIPER_H_ */
