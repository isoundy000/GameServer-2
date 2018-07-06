/*
 * MapPlayerEx.h
 *
 * Created on: 2013-03-04 11:05
 *     Author: glendy
 */

#ifndef _MAPPLAYEREX_H_
#define _MAPPLAYEREX_H_

#include "MapLeaguer.h"
#include "MapOfflineHook.h"
#include "MapPlayerScript.h"
#include "BattleGroundActor.h"
#include "LeagueWarActor.h"
#include "MonsterAttackActor.h"
#include "WorldBossActor.h"
#include "HotspringActor.h"
#include "LeagueRegionFightActor.h"
#include "TrvlPeakActor.h"

class MapPlayerEx : public MapLeaguer,
					public MapOfflineHook,
    				public MapPlayerScript,
    				public BattleGroundActor,
    				public LeagueWarActor,
    				public MonsterAttackActor,
    				public WorldBossActor,
    				public HotspringActor,
    				public LeagueRegionFightActor,
    				public TrvlPeakActor
{
public:
    MapPlayerEx(void);
    virtual ~MapPlayerEx(void);

    virtual void reset(void);
    virtual void reset_everyday();

    int notify_client_map_offline();	//通知客户端掉线
    int request_enter_scene_done(Message* msg);

    int request_exit_cur_system();
    int request_force_exit_system(Message* msg);
    int map_test_command(Message *msg);
    int modify_role_attr_for_test(Message *msg);

    int start_map_sync_transfer();
    int request_logout_game();

    int logout_and_use_offline();
    int logout_and_left_copy();
    int use_copy_when_exit_scene();
    int replace_copy_when_exit_scene();

    int recycle_copy_and_sign_in(int sid, Message* msg);
    int set_offline_player_info(Proto30400432* request, Int64 src_role_id = 0, Int64 machine_role_id = 0);

    int start_offline_copy(Int64 machine_role_id, Proto30400432* request);
    int start_offline_copy(const BSONObj& res);
    int set_offline_copy_by_bson(const BSONObj& res, Int64 role_id=0);

    int start_offline_player(Proto30400432* request);
    int start_offline_beast(Proto30400432* request);

    int online_exit_scene(void);
    int offline_exit_scene(void);

    int online_sign_out();
    int offline_sign_out();

    //trade
    int handle_inner_trade_distance(Message* msg);
    int trade_fetch_player_online_state(Int64 role_id,const int recogn);
    int trade_get_player_online_state(Message* msg);

public:
    virtual int sign_in(const int type = ENTER_SCENE_TRANSFER);
    virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
    virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
    virtual int sign_out(const bool is_save_player = true);

    int exit_scene_i(int type);

    virtual int transfer_to_other_scene(Message *msg);
    virtual int prepare_fight_skill(Message *msg);
    virtual int request_relive(Message *msg);
    virtual int process_relive_after_used_item(void);

    virtual int die_process(int64_t fighter_id);
    virtual int modify_blood_by_fight(const double value, const int fight_tips = FIGHT_TIPS_BLANK,
    		const int64_t attackor = 0, const int skill_id = 0);
    virtual int modify_magic_by_notify(const double value, const int fight_tips = FIGHT_TIPS_BLANK);

    virtual int notify_client_enter_info();

    virtual double fetch_addition_exp_percent(void);
    virtual int schedule_move(const MoverCoord &step, const int toward = -1, const Time_Value &arrive_tick = Time_Value::zero);

    int client_obtain_area_info();
    int loign_enter_scene();
    virtual int obtain_area_info(int request_flag = false);

    virtual bool is_movable_coord(const MoverCoord &coord);
    virtual int fetch_relive_data(const int relive_mode, int &blood_percent, int &item_id, int &item_amount);

    virtual int cached_timeout(const Time_Value &nowtime);
    virtual double fetch_reduce_hurt_rate(void);

    virtual int gather_state_begin(Message* msg);
    virtual int gather_state_end();
    virtual int gather_goods_begin(Message* msg);
    virtual int gather_goods_done(Message* msg);

    virtual int pick_up_drop_goods_begin(Message* msg);
    virtual int process_pick_up_suceess(AIDropPack *drop_pack);
    virtual int set_pk_state(const int state);

    void record_exception_skill_info();
    void debug_output_fight_info(const int type);

protected:
    virtual int validate_movable(const MoverCoord &step);
    virtual int validate_relive_point(int check_type);
    virtual int validate_relive_locate(const int item_id);
    virtual int process_relive(const int relive_mode, MoverCoord &relive_coord);

    virtual int validate_prepare_attack_target(void);
    virtual int validate_launch_attack_target(void);

    virtual int recovert_magic(const Time_Value &nowtime);
    virtual int recovert_blood(const Time_Value &nowtime);

    virtual int check_travel_timeout(void);
};

#endif //_MAPPLAYEREX_H_
