/*
 * GameStatus.h
 *
 * Created on: 2013-08-07 15:20
 *     Author: lyz
 */

#ifndef _GAMESTATUS_H_
#define _GAMESTATUS_H_

#include "FightStruct.h"
#include "EntityCommunicate.h"

class ProtoStatus;
class GameFighter;

class GameStatus : virtual public EntityCommunicate
{
public:
	enum STATUS_REFRESH_TYPE
	{
		STATUS_REFRESH_ONCE = 0,
		STATUS_REFRESH_ALL	= 1,
		STATUS_REFRESH_TYPE_END
	};

	enum STATUS_EFFECT_TYPE
	{
		STATUS_UPDATE_OTHER = 0,	// 不需要统计通知战斗属性等变化的状态
		STATUS_UPDATE_FIGHT_PROP = 1,	// 需要统计所有属性变化后，再通知客户端
		STATUS_UPDATE_SPEED = 2,	// 需要统计所有属性变化后，再通知客户端
		STATUS_EFFECT_TYPE
	};

    typedef HashMap<int, StatusQueueNode *, NULL_MUTEX> StatusMap;
    typedef Heap<StatusQueueNode, StatusQueueNodeCmp> StatusQueue;
    typedef std::bitset<GameEnum::STATUS_U_END> StatusUpdateFlagSet;

public:
    GameStatus(void);
    virtual ~GameStatus(void);
    void reset_status(void);
    virtual GameFighter *fighter(void);

    StatusMap &status_map(void);
    StatusQueue &status_queue(void);

    int insert_status(const BasicStatus &status);
    int remove_status(int status_id);
    int remove_status(BasicStatus *status);
    int find_status(int status_id, BasicStatus *&status);
    int find_status(int buff_type, BasicStatusVec& status_vec);
    int find_first_status(int buff_type, BasicStatus *&status);
    int is_have_status(int status_id);
    int is_have_status_type(int buff_type);
    double find_status_value(int id, int type = 1);
    double find_status_value_by_type(int buff_type, int value_type = 1);
    DoublePair find_status_pair_value(int buff_type);

    int clear_status(int type);

    // 添加状态的加成效果
    int increase_status_effect(BasicStatus *status, int enter_type = 0,
    		int refresh_type = STATUS_REFRESH_ONCE);
    int status_effect_type(int status_type);

    void check_and_dump_trace();
    void check_and_set_status_effect_type(int status_type);

    // 处理状态移除时的相关通知信息
    int process_remove_status(BasicStatus *status, const bool is_timeout = true);
    int refresh_all_status_property(int enter_type = 0);

    StatusUpdateFlagSet &status_u_flags(void);
    int refresh_single_status(BasicStatus *status);

    int insert_defender_status(GameStatus *defender, int status,
            double interval, double last, int accumulate_times = 0,
    		double val1 = 0, double val2 = 0, double val3 = 0,
            double val4 = 0, double val5 = 0);
    int insert_defender_status(GameStatus *defender, const BasicStatus& b_status);
    int insert_defender_stage_status(GameStatus *defender, const Json::Value& effect_json);

    // 判断是否有禁止移动的状态
    int validate_no_forbit_move_status(void);
    bool is_forbit_move_status(int status_type);

    int make_up_status_msg(Block_Buffer* buff, const bool send_by_gate = false);

protected:
    int refresh_status(const Time_Value &nowtime);
    int update_status(StatusQueueNode *node);
    // 处理状态的累加
    int accumulate_status(BasicStatus *n_status, const BasicStatus &status);
    virtual int validate_accumulate_status(BasicStatus *status, const BasicStatus &new_status);
    virtual int validate_cover_status(const BasicStatus *org_status, const BasicStatus *t_status);
    int compare_status_value(BasicStatus *left, BasicStatus *right);

    int notify_status(const bool send_by_gate = false);
    int notify_remove_status(BasicStatus* status);
    int notify_update_status(BasicStatus* status);
    int notify_update_status_info(int type, int recogn, Message* msg);
    int notify_status_update_property(void);
    int notify_status_effect_update(BasicStatus *status);

    void accumulate_blood_status(BasicStatus *n_status, const BasicStatus &status);
    int update_blood_status(BasicStatus *status, int tips = FIGHT_TIPS_STATUS);
    int inc_max_blood(BasicStatus *status, const int enter_type = 0);
    int reduce_max_blood(const int status, HistoryStatus *history);

    int update_magic_status(BasicStatus *status);
    int inc_max_magic(BasicStatus *status);
    int inc_speed(BasicStatus *status);

    int inc_second_fight_prop(BasicStatus *status);
    int inc_all_fight_prop(BasicStatus *status);
    int inc_multi_fight_prop(BasicStatus *status);

    int direct_hurt(BasicStatus *status);
    int repeat_hurt(BasicStatus *status);

    int notify_enter_stone_state(void);
    int notify_exit_stone_state(BasicStatus *status, const bool is_timeout);

    int notify_stone_player_tick(BasicStatus* status);
    int sync_beast_fight_buf();

    int inside_ice_hurt(BasicStatus* status);
    int jian_drop_hurt(BasicStatus* status);
    int set_next_stage_buff(BasicStatus* status);

    virtual int inc_tbattle_treasure_status_effect(BasicStatus *status, const int enter_type, const int refresh_type);
    virtual int process_tbattle_treasure_status_timeout(BasicStatus *status);

    bool is_avoid_dizzy(GameStatus *defender, const BasicStatus& b_status);

protected:
    int saved_id_; //保存的id
    TraceForceInfo trace_force_;

    StatusMap status_map_;
    StatusQueue status_queue_;
    StatusUpdateFlagSet status_u_flags_;
};

#endif //_GAMESTATUS_H_
