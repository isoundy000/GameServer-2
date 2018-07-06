/*
 * MLScriptClean.h
 *
 * Created on: 2014-04-24 14:53
 *     Author: lyz
 */

#ifndef _MLSCRIPTCLEAN_H_
#define _MLSCRIPTCLEAN_H_

#include "MapLogicStruct.h"

class MapLogicPlayer;

class MLScriptClean
{
public:
	typedef std::vector<RewardInfo> RewardInfoSet;
public:
    virtual ~MLScriptClean(void);

    void reset_script_clean(void);
    virtual MapLogicPlayer *player(void);

    int request_script_clean_info(Message *msg);
    int request_start_clean_single_script(Message *msg);
    bool has_clean_script_award(void);
    int request_draw_clean_script_award(void);
    int process_start_script_clean(Message *msg);

    int cal_reset_multiple(const Json::Value reset_multiple, int reset_times);

    ScriptCleanDetail &script_clean_detail(void);
    int script_clean_state(void);

    int notify_script_clean_doing(void);

    int process_script_first_pass_award(Message *msg);

    int process_script_add_times_use_gold(Message *msg);
    int process_script_list_info(Message *msg);

    int serialize_script_clean(Message *msg);
    int unserialize_script_clean(Message *msg);
    int sync_transfer_script_clean(const int scene_id);

    int process_piece_total_star_award(Message *msg);
    int process_script_other_award(Message *msg);
    int process_script_enter_check_pack(Message *msg);

protected:
    int process_rollback_script(void);
    int generate_award(const Time_Value &nowtime);
    int generate_normal_script_award(const int script_sort);

    int script_clean_finish_hook(const int script_sort);
    int process_activity_score_award(const int script_sort, const int chapter_key, const int star_lvl);

private:
    ScriptCleanDetail clean_detail_;
    RewardInfoSet reward_info_set_;
};

#endif //_MLSCRIPTCLEAN_H_
