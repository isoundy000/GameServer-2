/*
 * MapKiller.h
 *
 * Created on: 2014-04-30 16:44
 *     Author: lyz
 */

#ifndef _MAPKILLER_H_
#define _MAPKILLER_H_

#include "GameFighter.h"

struct KilledInfo
{
    LongMap attackor_map_;

    int is_yellow_;
    int kill_num_;
    int is_brocast_;
    int online_ticks_;
    int killing_value_;		// PK值

    void reset();
    int pk_value();
};

class MapKiller : virtual public GameFighter
{
public:
    virtual ~MapKiller(void);
    virtual int fetch_name_color();

    void reset_map_killer();
    void reset_killer_everyday();

    // monster
    int inc_kill_monster(const int sort, const int amount = 1);
    int inc_task_monster(const int sort, const int amount = 1);

    int killer_time_up(const Time_Value &nowtime);
    int refresh_kill_monster_info(void);
    int refresh_task_monster_info(void);

    // kill value
    int die_not_notify();
    int notify_kill_fighter(Int64 attacker_id);
    int record_hurt_attackor(Int64 attackor, int fight_tips);

    int handle_kill_ditheism(MapKiller* attacker, int add_test = false);
    int inc_kill_value(MapKiller *attacker, const int value);
    int handle_reduce_kill_value();
    int validate_kill_value_scene();
    int modify_kill_value(int value);
    int remove_last_color_info(int last_color);
    int nofity_name_color_change(int flag = true);
    int check_and_brocast_ditheism();

	bool is_attack_by_id(Int64 role_id);
    KilledInfo& killed_info();

    int sync_transfer_killer(void);
    int read_transfer_killer(Message *msg);

protected:
    BIntMap monster_map_;
    BIntMap task_monster_map_;

    KilledInfo killed_info_;
};

#endif //_MAPKILLER_H_
