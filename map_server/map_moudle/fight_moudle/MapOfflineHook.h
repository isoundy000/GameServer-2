/*
 * MapOfflineHook.h
 *
 *  Created on: Mar 7, 2014
 *      Author: peizhibi
 */

#ifndef MAPOFFLINEHOOK_H_
#define MAPOFFLINEHOOK_H_

#include "AutoMapFighter.h"

class MapOfflineHook : public AutoMapFighter
{
public:
	class RecoverTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);

		MapOfflineHook* offline_hook_;
	};

public:
	MapOfflineHook();
	virtual ~MapOfflineHook();

    virtual int sign_in(const int type = ENTER_SCENE_TRANSFER);
    virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
    virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
    virtual int sign_out(const bool is_save_player = true);

	virtual void reset();

    virtual int die_process(const int64_t fighter_id = 0);
    virtual int auto_action_timeout(const Time_Value &nowtime);

    virtual Time_Value calculate_move_tick(void);

    virtual int fetch_attack_distance(void);

    virtual int schedule_move_fighter(void);
    virtual int auto_fighter_attack(void);

public:
    int update_blood_in_offline(const double value, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);

    void update_scene_info();
    void copy_fighter_skill();
    void start_recover_timer();

    void fb_releive();
    void update_fb_scene_info();
    void update_arena_scene_info();

    void arena_field_timeout();

    void fb_die_process(const int64_t fighter_id);
    void arena_die_process(const int64_t fighter_id);

    int execute_ai_tree(void);
    int fetch_fb_aim_object(void);

    int recover_timeout(void);
    int update_copy_player_speed(double add_times);


public:
    static bool validate_fb_aim_object(GameFighter* fighter);

protected:
    RecoverTimer recover_timer_;
    OfflineDetail offline_detail_;
};

#endif /* MAPOFFLINEHOOK_H_ */
