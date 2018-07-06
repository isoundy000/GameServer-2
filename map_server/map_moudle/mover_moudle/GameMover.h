/*
 * GameMover.h
 *
 * Created on: 2013-01-22 21:21
 *     Author: glendy
 */

#ifndef _GAMEMOVER_H_
#define _GAMEMOVER_H_

#include "MapMapStruct.h"
#include "EntityCommunicate.h"

class GameMover : virtual public EntityCommunicate
{
public:
    GameMover(void);
    virtual ~GameMover(void);

    void reset_mover(void);
    virtual void set_cur_location(const MoverCoord& coord);
    virtual void set_cur_toward(const MoverCoord& coord);

    MapMonitor *monitor(void);
    const Json::Value& scene_set_conf();
    const Json::Value& scene_conf();

    virtual int respond_from_broad_client(Block_Buffer *buff);
    virtual int respond_from_broad_client(const int recogn, const int error, Block_Buffer *msg_body);
    virtual int respond_from_broad_client(const int recogn, const int error, const Message *msg_proto = 0);

    virtual int respond_to_broad_area(const Message *msg, const int is_full_screen = 0);
    int respond_to_broad_area(const int recogn, const int is_full_screen = 0);

    virtual int client_sid(void) = 0;
    virtual int64_t mover_id(void);
    virtual int mover_id_low(void);
    virtual int mover_id_high(void);
    virtual int fetch_mover_volume();
    virtual void set_active(const bool flag);

    bool is_active(void);
    bool is_enter_scene(void);

    bool is_player(void);
    bool is_beast(void);
    bool is_monster(void);

    void set_scene_mode(const int scene_mode);
    virtual int scene_id(void);
    MoverCoord &location(void);
    virtual int scene_mode(void);
    virtual int space_id(void);

    virtual int prev_scene_id(void);
    MoverCoord &prev_location(void);
    virtual int prev_scene_mode(void);
    virtual int prev_space_id(void);

    virtual void set_room_scene_index(const int index);
    virtual int room_scene_index(void);

    bool is_in_normal_mode(void);
    bool is_in_script_mode(void);
    bool is_in_league_mode(void);
    bool is_in_world_boss_mode(void);
    bool is_npc_nearby(int npc_id);
    bool is_in_league_region();

    int init_mover_scene(Scene* scene);
    MoverDetail &mover_detail(void);

    int notify_mover_cur_location();
    int check_cur_distance(const MoverCoord& aim_coord, int max_distance);

    int make_up_appear_info(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_appear_other_info(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_buff_info(Block_Buffer *buff, const bool send_by_gate = false);
    int make_up_disappear_info(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_disappear_info_base(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_disappear_other_info(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_move_info(Block_Buffer *buff, Int64 mover_id = 0);

    virtual int sign_in(void);
    virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
    virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
    virtual int sign_out(void);
    virtual int schedule_move(const MoverCoord &step, const int toward = -1, const Time_Value &arrive_tick = Time_Value::zero);

    virtual bool is_movable_coord(const MoverCoord &coord);
    virtual bool is_movable_coord_skill_push(const MoverCoord &coord);

    Scene *fetch_scene(void);

    virtual void set_cur_block_index(const BlockIndexType index);
    virtual BlockIndexType cur_block_index(void);

    virtual bool is_movable_path_coord(const DynamicMoverCoord &coord);
    virtual int generate_move_path(const MoverCoord &target, const int max_step = 1000);
    virtual int correct_scene(const int scene_id);
    int generate_move_path_no_dynamic_mpt(const MoverCoord &target, const int max_step = 1000);

    virtual double speed_total(const int buff = BasicElement::ELEM_ALL) const;
    virtual int speed_total_i(const int buff = BasicElement::ELEM_ALL) const;

    virtual double speed_base_total(const int buff = BasicElement::ELEM_ALL) const;
    virtual int speed_base_total_i(const int buff = BasicElement::ELEM_ALL) const;

    int fetch_mover_type();
    int check_mover_type(int mover_type);

    int adjust_mover_coord();
    int set_mover_cur_block();
    int check_and_set_mover_cur_block();

    MoverCoord fetch_config_relive_coord(int camp = 1);

protected:
    virtual int generate_move_step_list(void);
    int generate_move_step_list_no_dynamic_mpt(void);

    virtual int generate_path_by_direct(const MoverCoord &target, const int max_step = 1000);
    virtual int generate_path_by_astar(const MoverCoord &target);
    virtual int calculate_target_priority(const DynamicMoverCoord &source, const DynamicMoverCoord &target);

    int generate_path_by_astar_no_dynamic_mpt(const MoverCoord &target);
    int calculate_target_priority_by_mpt(const MoverCoord &source, const MoverCoord &target);

    virtual int validate_movable(const MoverCoord &step);

    virtual int calculate_toward(const MoverCoord &step);
    virtual Time_Value calculate_arrive_tick(const MoverCoord &pos, const MoverCoord &step);

    virtual Time_Value calculate_move_tick(void);

protected:
    bool is_active_;
    bool is_enter_scene_;
    bool is_send_disappear_;

    Scene *scene_;
    int mover_type_;

    MapMonitor *monitor_;
    MoverDetail mover_detial_;

    int room_scene_index_;
};

#endif //_GAMEMOVER_H_
