/*
 * BaseScript.h
 *
 * Created on: 2013-12-13 19:23
 *     Author: lyz
 */

#ifndef _BASESCRIPT_H_
#define _BASESCRIPT_H_

#include "ScriptStruct.h"

class Proto50400903;
class Proto80400906;
class Proto80400929;

class ScriptAI;
class ScriptScene;
class MapPlayerScript;

class BaseScript {
public:
	enum {
		SCRIPT_MONSTER_MAX_FAIL_TICK = 5,
		SCRIPT_MONSTER_MAX_NOTIFY_INTERVAL = 5,

		SKILL_SET = 1,
		SKILL_DEL = 2,

		WIN = 1,
		LOSE = 2,
		END
	};

	typedef HashMap<int, ScriptScene *, NULL_MUTEX> SceneMap;

public:
	static int check_enter_script(const int gate_sid, const int64_t role_id,
			Message *msg);
	static ScriptPlayerRel *init_script_player_rel(Message *msg);
	static int load_data_for_check_script_progress(ScriptPlayerRel *player_rel);
	static int after_load_script_progress(Transaction *transaction);
	static int validate_script_config(ScriptPlayerRel *player_rel);
	static int fetch_single_script(ScriptPlayerRel *player_rel,
			BaseScript *&script);
	static int fetch_team_script(ScriptPlayerRel *player_rel,
			BaseScript *&script);
	static int request_load_player_copy(BaseScript* script,
			ScriptPlayerRel* player_rel);
	static int start_copy_player(DBShopMode* shop_mode);


	class ScriptTimer: public GameTimer {
	public:
		ScriptTimer(void);
		virtual ~ScriptTimer(void);

		virtual int type(void);
		virtual int handle_timeout(const Time_Value &nowtime);
		BaseScript *script_;
	};

public:
	BaseScript(void);
	virtual ~BaseScript(void);
	virtual void reset(void);

	MapMonitor *monitor(void);
	MapPlayerEx* find_player(Int64 role_id);

	const Json::Value& scene_conf();
	const Json::Value& script_conf();

	int fetch_pass_total_reward(ThreeObjVec& reward_vec, int flag);
	int fetch_drop_reward(ThreeObjVec& reward_vec);

	virtual int fetch_reward_index();
	virtual int fetch_first_reward(ThreeObjVec& reward_vec);
	virtual int fetch_normal_reward();
	virtual int fetch_wave_reward(ThreeObjVec& reward_vec);
	virtual int script_pass_chapter();
	virtual int sync_restore_pass(MapPlayerScript* player); //更新资源找回

	virtual ScriptScene *fetch_scene(const int scene_id);
	virtual ScriptScene *fetch_current_scene(void);

	int script_type(void);
	int set_script_type(const int type);
	bool is_exp_script(void);
	bool is_rama_script(void);
	bool is_league_fb_script(void);
	bool is_climb_tower_script(void);
	bool is_top_script(void);
	bool is_upclass_script(void);
	bool is_tower_defense_script(void);
	bool is_monster_tower_script(void);
	bool is_couble_script(void);

	ScriptDetail &script_detail(void);
	int script_id(void);
	int script_sort(void);
	Int64 progress_id(void);
	int monster_level_index(void);
	Int64 owner_id(void);
	void set_owner_id(const Int64 id);

	// 是否单人副本
	void set_single_script(const bool flag);
	bool is_single_script(void);
	int team_id(void);

	void reset_all_scene_flag(void);
	void set_scene_flag(const string& flag_str);
	void set_scene_flag(const int flag);
	void reset_scene_flag(const int flag);
	bool test_scene_flag(const int flag);

	BLongSet &player_set(void);
	int current_script_scene(void);
	int current_script_floor(void);
	int last_script_scene(void);
	int last_script_floor(void);
	int next_script_scene(void);
	int next_script_floor(void);
	int passed_script_scene(void);
	int passed_script_floor(void);
	int piece(void);
	int chapter(void);
	int chapter_key(void);
	int floor_id(void);
	int finish_tick(void);
	const Time_Value &monster_fail_tick(void);
	const Time_Value &monster_amount_notify_tick(void);

	bool is_holdup(void);
	bool is_finish_script_scene(void);
	bool is_failure_script_scene(void);
	bool is_finish_script(void);
	bool is_failure_script(void);
	bool is_self_script(MapPlayerScript* player);

	Time_Value &recycle_tick(void);
	int left_scene_ready_tick(void);
	int total_scene_tick(void);
	int left_scene_tick(void);
	int used_scene_tick(void);
	int total_used_script_tick(void);
	int reset_left_tick(void);

	int current_wave(void);
	int total_wave(void);
	int finish_wave(void);
	int pass_wave(void);
	int begin_wave(void);
	int script_status(void);

	virtual int script_finish_flag();
	virtual int fetch_task_wave();
	virtual int left_relive_amount(const Int64 role_id);
	virtual int total_relive_amount(const Int64 role_id);
	virtual int used_relive_amount(const Int64 role_id);
	virtual int used_relive_amout_by_item(const Int64 role_id);
	virtual int reduce_relive_times(const Int64 role_id, const int relive_mode);

	int left_monster_amount(void);
	int total_monster_amount(void);
	int kill_monster_amount(void);
	// 当前最高连斩数
	int top_evencut(void);
	// 获取要保护的NPC
	virtual ScriptAI *fetch_protect_npc(void);

	virtual int init_script_base(ScriptPlayerRel *player_rel);
	// bind script to monitor;
	virtual int init_for_single(ScriptPlayerRel *player_rel);
	// bind script to monitor;
	virtual int init_for_team(ScriptPlayerRel *player_rel);

	virtual int insert_history_role(const int64_t role_id,
			const Time_Value &used_times_tick);
	virtual int fetch_enter_scene_coord(const int64_t role_id, int &scene_id,
			MoverCoord &enter_coord);
	virtual int relive_coord_from_config_point(const int64_t role_id,
			MoverCoord &relive_coord);

	virtual int player_enter_script_scene(MapPlayerScript *player,
			const int scene_id);
	virtual int start_script_timer(Message *msg = 0);
	virtual int keepon_special_script_tick(void);
	virtual int stop_script_timer(void);
	virtual int holdup_special_script_tick(void);
	virtual int player_transfer_script_scene(MapPlayerScript *player,
			const int scene_from, const int scene_to);
	virtual int player_exit_script(MapPlayerScript *player,
			bool transfer = true);
	int process_team_player_exit(MapPlayerScript *player);

	virtual int sync_increase_monster(ScriptAI *script_ai);
	virtual int sync_increase_monster(ScriptAI *script_ai,
			const Json::Value &json);
	virtual int sync_kill_monster(ScriptAI *script_ai, const Int64 fighter_id);
	virtual int sync_kill_npc(ScriptAI *script_ai);

	virtual int check_add_player_buff();	//增加玩家buff

	bool check_couple_fb_finish();
	bool check_couple_fb_failure(const Time_Value &nowtime);
	bool check_and_handle_kill_script_npc(ScriptAI *script_ai);
	virtual int process_script_timeout(const Time_Value &nowtime);
	virtual int other_script_timeout(const Time_Value &nowtime);

	bool check_and_process_no_enter_timeout(const Time_Value &nowtime);
	bool check_and_process_recycle_timeout(const Time_Value &nowtime);
	bool check_and_process_pre_timeout(const Time_Value &nowtime);	//副本开始前准备
	bool check_and_process_use_timeout(const Time_Value &nowtime);	//副本使用时间超时

	int notify_all_player(Message *msg);
	int notify_all_player(int recogn, Message* msg = NULL);

	virtual int run_current_scene(void);
	virtual int generate_new_wave(const int scene_id, const int wave_id);

	virtual int make_up_script_progress_detail(Proto50400903* respond);
	virtual int make_up_special_script_progress_detail(Message *msg);
	virtual int make_up_finish_scene_detail(Message *msg);

	virtual int make_up_script_finish_detail(MapPlayerScript *player,
			Proto80400906* respond1, Proto80400929* respond2);
	virtual int make_up_special_script_finish_detail(MapPlayerScript *player,
			Message *msg);
	virtual int sync_script_inc_exp(const Int64 role_id, const int inc_exp);
	virtual int sync_script_inc_money(const Int64 role_id, const Money &money);
	virtual int sync_script_inc_item(const Int64 role_id, const ItemObj &item);
	virtual int process_additional_exp(MapPlayerScript *player);
	virtual int process_scene_tick_failure(void);

	virtual int notify_fight_hurt_detail();
	virtual int notify_npc_blood(const Int64 npc_id, const int inc_blood);
	virtual int make_up_spirit_info(Message *msg);

	void descrease_monster_amount(ScriptAI *script_ai);
	void check_and_handle_next_wave(ScriptAI *script_ai);
	void update_player_exp(MapPlayerEx *player, int inc_exp);
	int notify_script_progress_detail(void);
	int first_start_script(void);
	int check_script_scene_failure(void);

	int record_player_hurt(Int64 role_id, int real_hurt);
	int update_boss_hurt(const int sort, const double real_hurt);
	int update_player_hurt(const Int64 role_id, const double real_hurt);

	virtual bool is_top_star_level(void);
	virtual int script_star_level();

	virtual double recalc_ai_modify_blood(const double inc_val);
	int fetch_enter_teamers();
	virtual int set_player_die_coords(MoverCoord die_coords);
	virtual int reset_player_die_coords();

	void check_recycle_team_info();
	void set_script_recycle_time(int delay = 15);

	virtual int fetch_monster_coord(MoverCoord &coord);

	bool is_wave_script();

protected:
	virtual void recycle_self(void);
	virtual void recycle_self_to_pool(void) = 0;

	virtual int init_script_scene(void);
	int init_script_all_scenes();
	virtual int init_script_first_scene(void);
	virtual int init_scene_exec_condition(const Json::Value &scene_json);

	virtual int validate_exit_script(MapPlayerScript *player);
	virtual int validate_legal_scene(MapPlayerScript *player,
			const int scene_from, const int scene_to);
	virtual int validate_exit_scene(MapPlayerScript *player,
			const int scene_to);

	virtual int process_change_scene(MapPlayerScript *player,
			const int scene_from, const int scene_to);
	virtual int process_ready_tick_timeout(void);
	virtual int process_generate_monster(void);
	virtual int process_script_failure(void);
	virtual int process_finish_script(void);
	virtual int process_script_player_failure(void);
	int process_script_player_finish(void);
	virtual int process_script_player_finish(MapPlayerEx* player);
	virtual int process_kill_wave(const int wave_id);
	virtual int broad_script_pass(void);

	virtual bool check_script_scene_finish(void);
	virtual int process_script_stage_finish();	//阶段完成处理
	int script_scene_finish_operate(void);
	virtual bool check_script_finish(void);
	virtual int check_area_matrix(void);

	virtual int notify_failure_script(void);
	virtual int notify_update_evencut_rec(void);
	virtual int notify_update_monster_rec(void);
	virtual int notify_update_relive_rec(void);
	virtual int notify_kill_wave(void);
	virtual int notify_update_spirit(void);
	int notify_monster_amount_ready_to_max(void);

	virtual int refresh_script_evencut(void);
	virtual int update_script_evencut(void);
	int request_save_script_progress(void);
	int update_script_tick_info();
	int force_set_script_failure(void);
	int update_chapter_history(void);
	int sync_logic_team_script_end(void);

	int recycle_copy_player(void);
	int copy_player_change_scene(void);
	int exp_restore_sync_finish_scene(int stage);
	int restore_script_state(MapPlayerScript *player);
	int kickout_all_player(void);
	int force_kill_all_monster(void);

	bool is_top_star_level(const int value);

	int request_update_intimacy_by_finish(void);
	int calc_appear_text(ScriptAI *script_ai);
	int poem_text_size(void);
	virtual int summon_ai_inherit_player_attr(ScriptAI *script_ai,
			const Json::Value &json);

private:
	static int global_script_id_;

protected:
	MapMonitor *monitor_;
	int script_type_;
	int script_id_;
	SceneMap scene_map_;

	ScriptDetail script_detail_;
	ScriptTimer script_timer_;
	BLongSet history_role_;
	BLongSet first_role_;

	Time_Value failure_check_tick_;	//如果玩家1分钟，没在回收
	Time_Value couple_check_tick_;	//夫妻副本检测时间
	int couple_flag_;
	int save_enter_teamers_;
	int timer_stage_;
};

#endif //_BASESCRIPT_H_
