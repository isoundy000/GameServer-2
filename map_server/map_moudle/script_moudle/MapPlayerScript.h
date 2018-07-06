/*
 * MapPlayerScript.h
 *
 * Created on: 2013-12-24 17:50
 *     Author: lyz
 */

#ifndef _MAPPLAYERSCRIPT_H_
#define _MAPPLAYERSCRIPT_H_

#include "MapPlayer.h"
#include "ScriptStruct.h"
//#include "MLMagicWeapon.h"

class ProtoLegendTop;

class MapPlayerScript : virtual public MapPlayer
{
public:
    MapPlayerScript(void);
    virtual ~MapPlayerScript(void);

    void reset_script(void);
    void script_reset_everyday();
    int logic_reset_script();

    int request_enter_script(Message *msg);
    int sync_enter_script(Message *msg);
    int request_exit_script(bool force = false);
    int request_exit_special(void);
    BaseScript *fetch_script(void);

    // 副本星级
    bool is_script_top_star_level(const int script_sort, const int value);

    int process_exit_script();
    int process_script_failure(BaseScript *script);
    int process_script_finish(BaseScript *script);

    virtual int enter_scene(const int type);
    virtual int exit_scene(const int type);
    virtual int sign_out(const bool is_save_player);

    virtual int transfer_to_other_scene(Message *msg);
    virtual int prepare_fight_skill(Message *msg);
    virtual int request_relive(Message *msg);
    virtual int obtain_area_info(int request_flag = false);

    int request_script_detail_progress(void);

    ScriptPlayerDetail &script_detail(void);
    ScriptPlayerDetail::ScriptRecord *script_record(int script_sort);
    ScriptPlayerDetail::TypeRecord *type_record(int script_type);
    ScriptPlayerDetail::ScriptWaveRecord *special_record(const int key);
    ScriptPlayerDetail::LegendTopInfo &top_info(int script_type);

    int sync_transfer_script(void);
    int serialize_script(Message *msg);
    int unserialize_script(Message *msg);

    void serialize_top_info(ProtoLegendTop *top, ScriptPlayerDetail::LegendTopInfo &top_info);
    void unserialize_top_info(const ProtoLegendTop &top, ScriptPlayerDetail::LegendTopInfo &top_info);

    int script_sort(void);
    int script_id(void);
    int script_chapter_key(void);
    int convert_chapter_key(const int piece, const int chapter);
    int convert_script_wave_key(const int wave, const int chapter);

    ScriptPlayerDetail::ScriptRecord *refresh_script_record(int script_sort);
    int update_script_used_times_in_enter(BaseScript *script, const Time_Value &tick);

    int refresh_script_record_in_finish(BaseScript *script, const Time_Value &tick);
    int update_pass_chapter(BaseScript *script);
    int update_pass_exp(BaseScript *script);
    int update_pass_lfb(BaseScript *script);
    int update_pass_rama(BaseScript *script);
    int update_pass_floor(BaseScript *script);
    int update_finish_script_rec(BaseScript *script, bool finish = true);
    int update_open_activity(BaseScript *script);
    int update_sword_pool_info(int script_type, int num);
    int update_cornucopia_task_info(int num, int type);
    int update_labour_task_info(int num, int type);

    int request_first_start_script(Message *msg);
    int request_stop_script(Message *msg);
    int request_run_script(Message *msg);
    virtual int process_relive_after_used_item(void);

    virtual int schedule_move(const MoverCoord &step, const int toward = -1, const Time_Value &arrive_tick = Time_Value::zero);
    virtual int modify_blood_by_fight(const double value, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);

    int request_update_script_matrix(Message *msg);
    int inner_fetch_script_clean_times(Message *msg);
    int fetch_script_clean_top_script(const int script_sort);
    int make_up_script_clean_times(const int script_sort, Message *msg);
    int make_up_normal_script_clean_times(const int script_sort, Message *msg);
    int make_up_chapter_script_clean_times(const int script_sort, Message *msg);
    int rollback_script_times(Message *msg);

    int request_chapter_script_detail(Message *msg);
    int request_extract_script_card(Message *msg);
    int request_script_piece_detail(void);

    int fetch_script_clean_tick(const int script_sort, int &left_times, int &use_tick, IntMap &script_times_map);
    int request_single_clean_script_tick(Message *msg);
    int request_clean_all_script_tick(Message *msg);

    int sync_team_script_use_times(Message *msg);
    int process_increase_script_times(const int script_sort, const int chapter_key = 0,
    		const int finish_times = 1, const bool is_script_clean = false);

    int fetch_relive_data(const int relive_mode, int &blood_percent, int &item_id, int &item_amount);
    int set_script_wave_task(Message *msg);
    int check_script_wave_task(BaseScript *script);

    virtual bool is_movable_coord(const MoverCoord &coord);
    virtual double fetch_addition_exp_percent(void);

    int request_script_list_info(Message *msg);
    int process_script_list_info(Message *msg);

    int login_add_couple_fb_times(Message *msg); //登录时检测夫妻副本赠送次数

    // 一键重置所有副本
    int command_reset_script(void);

    int request_script_player_info(Message *msg);
    int request_script_type_info(const int script_sort);
    int request_fetch_special_award(Message *msg);

    int request_script_add_times_gold(Message *msg);
    int request_script_add_times(Message *msg);
    int process_add_script_after_use_gold(Message *msg);

    int process_after_reset_script(Message *msg);

    int request_script_type_reset_begin(Message *msg);
    int request_script_type_reset_end(Message *msg);

    int sync_share_script_used_times(const int script_sort, const int vary_times);
    int sync_share_script_buy_left_times(const int script_sort, const int vary_times);

    int call_puppet(const int puppet_sort);

    int request_piece_total_star(Message *msg);
    int request_piece_total_star_award(Message *msg);
    int process_piece_total_star_after_award(Message *msg);

    int process_fetch_climb_tower_info(Message *msg);

    int read_script_compact_info(Message *msg);
    bool is_script_compact_status(void);

    int fetch_script_left_times(const int script_sort);
    int fetch_script_left_times(ScriptPlayerDetail::ScriptRecord *record, const Json::Value &script_json);
    int fetch_script_left_buy_times(ScriptPlayerDetail::ScriptRecord *record, const Json::Value &script_json, const int cur_gold = INT_MAX);

    int script_relive_left_times(int &relive_total, int &relive_left);

    virtual int process_pick_up_suceess(AIDropPack *drop_pack);

    int process_check_pa_event_script_times(void);
    int check_pa_event_climb_tower_script(void);
    int check_pa_event_all_script(IntMap &event_map);
    int check_pa_event_script_type_finish(int script_type);
    int check_pa_event_script_level();

    void set_protect_beast_index(const int script_sort, const int index);

    int process_fetch_couple_script_times(Message *msg); //无用
    //夫妻副本
    int couple_fb_select_key(Message *msg);

    //武林论剑设置技能
    int sword_top_select_skill(Message *msg);
    int sword_script_set_skill(int type);

    int request_remove_stone_state(Message *msg);
    int get_monster_tower_pass_time_by_floor(int sort, const ScriptPlayerDetail &record);
    int update_monster_tower_pass_time(int sort, ScriptPlayerDetail &record);

    //drop dragon
    virtual int gather_goods_begin(Message* msg);
    virtual int gather_goods_done(Message* msg);

    int fetch_legend_top_rank(Message *msg);
    int fetch_sword_top_rank(Message *msg);
    int script_red_point_check_uplvl();

    int process_script_player_die();
    int process_script_pass_award(BaseScript *script);
    int process_script_pass_award(BaseScript *script, bool first);
    int process_script_travel(BaseScript *script);

    int script_sync_restore_info(Message *msg);
    int script_clean_sync_restore(int script_sort);
    int check_script_restore_info(const Json::Value &script_json, int &event_id, int &value);
    int update_branch_task_info(Message* msg);

    int test_reset_lfb_script();

protected:
    virtual int validate_movable(const MoverCoord &step);
    virtual int validate_relive_point(int check_type);
    virtual int validate_relive_locate(const int item_id);
    virtual int process_relive(const int relive_mode, MoverCoord &relive_coord);
    virtual int die_process(const int64_t fighter_id = 0);
    virtual int validate_prepare_attack_target(void);

    //副本奖励
    int process_pass_floor_award(BaseScript *script);	//通过奖励
    int process_first_pass_floor(BaseScript *script);	//首通奖励

    int sync_script_wave(BaseScript *script);

    virtual int recovert_magic(const Time_Value &nowtime);
    virtual int recovert_blood(const Time_Value &nowtime);

    int piece_total_star(const int piece);
    int update_drawed_star(const int piece, const int update_star, const int value);
    int fetch_gold_for_buy_script_times(const int script_sort, const int inc_times = 1);
    int fetch_script_id_use_type(IntVec &sort_vec, int script_type);

    bool is_wave_script(int script_type);

    int check_enter_condition(int script_sort, const Json::Value &cond);

protected:
    ScriptPlayerDetail script_detail_;

    int compact_type_;
    Time_Value expired_tick_;
    Time_Value drop_dragon_clean_tick_; //防止坠龙窟副本扫荡过快导致体力没及时同步
};

#endif //_MAPPLAYERSCRIPT_H_
