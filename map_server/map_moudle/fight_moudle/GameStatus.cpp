/*
 * GameStatus.cpp
 *
 * Created on: 2013-08-07 17:12
 *     Author: lyz
 */

#include "GameStatus.h"
#include "Scene.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayer.h"
#include "MapBeast.h"

GameStatus::GameStatus(void)
{ /*NULL*/
	  this->saved_id_ = 0; //保存的id
}

GameStatus::~GameStatus(void)
{ /*NULL*/ }

void GameStatus::reset_status(void)
{
	this->saved_id_ = 0; //保存的id
    this->status_queue().clear();

    MapMonitor *monitor = this->fighter()->monitor();
    for (GameStatus::StatusMap::iterator iter = this->status_map().begin();
            iter != this->status_map().end(); ++iter)
    {
        monitor->status_queue_node_pool()->push(iter->second);
    }

    this->status_map().unbind_all();
}

GameFighter *GameStatus::fighter(void)
{
    return dynamic_cast<GameFighter *>(this);
}

GameStatus::StatusMap &GameStatus::status_map(void)
{
    return this->status_map_;
}

GameStatus::StatusQueue &GameStatus::status_queue(void)
{
    return this->status_queue_;
}

int GameStatus::insert_status(const BasicStatus &status)
{
    StatusQueueNode *status_q_node = 0;
    BasicStatus *n_status = 0;
    if (this->find_status(status.__status, n_status) == 0)
    {
        // 判断状态是否可以累加在一起
        int ret = this->validate_accumulate_status(n_status, status);
        if (ret == 0)
        {
            return this->accumulate_status(n_status, status);
        }

        // 超过叠加次数不处理
        if (status.__accumulate_tims > 0 && ret != -2)
        {
        	return -1;
        }

        // 判断状态是否需要覆盖，默认是不需要覆盖，但队列最长是5个；
        if (this->validate_cover_status(n_status, &status) == 0)
        {
            this->remove_status(n_status);
        }
    }

    // fetch status_list;
    bool is_new_status_list = false;
    if (this->status_map().find(status.__status, status_q_node) != 0)
    {
        status_q_node = MAP_MONITOR->status_queue_node_pool()->pop();
        status_q_node->set_status(status);
        if (this->status_map().bind(status.__status, status_q_node) != 0)
        {
            MAP_MONITOR->status_queue_node_pool()->push(status_q_node);
            return -1;
        }
        is_new_status_list = true;
    }
    StatusValueQueue *status_list = &(status_q_node->__status_list);

    // init_status;
    n_status = MAP_MONITOR->status_pool()->pop();
    if (n_status == 0)
    {
		if (is_new_status_list == true)
		{
			MAP_MONITOR->status_queue_node_pool()->push(status_q_node);
		}
		return -1;
    }

    *n_status = status;
    n_status->__accumulate_tims = 1;

    Time_Value nowtime = Time_Value::gettimeofday();
    if (n_status->__last_tick > Time_Value::zero)
    {
        if (n_status->__interval == Time_Value::zero
        		|| n_status->__last_tick < n_status->__interval)
        {
            n_status->__check_tick = nowtime + n_status->__last_tick;
            if (n_status->__interval == Time_Value::zero)
                n_status->__last_tick = Time_Value::zero;
        }
        else
        {
            n_status->__check_tick = nowtime + n_status->__interval;
        }
    }
    else
    {
        MAP_MONITOR->status_pool()->push(n_status);
        if (is_new_status_list == true)
        {
            MAP_MONITOR->status_queue_node_pool()->push(status_q_node);
        }
        return -1;
    }

    // find new status insert location;
    BasicStatus *first_status = status_list->top();
    status_list->push(n_status);
    BasicStatus *new_top_status = status_list->top();

    //　更新新的状态的属性加成
    Time_Value prev_interval = new_top_status->__interval;
    if (first_status != new_top_status)
    {
        if (first_status != 0)
        {
            this->status_queue().remove(status_q_node);
        }

        status_q_node->__check_tick = new_top_status->__check_tick;
        this->status_queue().push(status_q_node);

        if (prev_interval == Time_Value::zero)
        {
        	this->refresh_all_status_property();
        }
    }

    return 0;
}

int GameStatus::remove_status(int status_id)
{
	BasicStatus* status = NULL;
	JUDGE_RETURN(this->find_status(status_id, status) == 0, -1);
	return this->remove_status(status);
}

int GameStatus::remove_status(BasicStatus *status)
{
    StatusQueueNode *queue_node = 0;
    JUDGE_RETURN(this->status_map().find(status->__status, queue_node) == 0, -1);

    bool is_remove_fist_status = false;
    if (queue_node->__status_list.top() == status)
    {
        this->check_and_set_status_effect_type(status->status_type());
        is_remove_fist_status = true;
    }

    queue_node->__status_list.remove(status);
    if (queue_node->__status_list.top() == 0)
    {
        this->process_remove_status(status, false);
        this->status_map().unbind(status->__status);
        this->status_queue().remove(queue_node);
        MAP_MONITOR->status_queue_node_pool()->push(queue_node);
    }
    else
    {
        if (is_remove_fist_status == true)
        {
            BasicStatus *top_status = queue_node->__status_list.top();
            this->status_queue().remove(queue_node);
            queue_node->__check_tick = top_status->__check_tick;
            this->status_queue().push(queue_node);
        }
    }

    MAP_MONITOR->status_pool()->push(status);
    if (this->status_u_flags_.test(GameEnum::STATUS_U_PROP)
    		|| this->status_u_flags_.test(GameEnum::STATUS_U_SPEED))
    {
    	this->refresh_all_status_property();
    }

    return 0;
}

int GameStatus::find_status(int status_id, BasicStatus *&status)
{
    StatusQueueNode *queue_node = 0;
    if (this->status_map().find(status_id, queue_node) != 0)
    {
        return -1;
    }

    status = queue_node->__status_list.top();
    if (status == 0)
    {
        return -1;
    }

    if (status->__status != status_id)
    {
    	status = 0;
    	return -1;
    }

    return 0;
}

int GameStatus::find_status(int buff_type, BasicStatusVec& status_vec)
{
	status_vec.reserve(GameEnum::DEFAULT_VECTOR_SIZE);

    for (GameStatus::StatusMap::iterator iter = this->status_map_.begin();
            iter != this->status_map_.end(); ++iter)
    {
    	BasicStatus* cur_status = iter->second->__status_list.top();
    	JUDGE_CONTINUE(cur_status != NULL && cur_status->__buff_type == buff_type);

    	status_vec.push_back(cur_status);
    }

    return 0;
}

int GameStatus::find_first_status(int buff_type, BasicStatus *&status)
{
    for (GameStatus::StatusMap::iterator iter = this->status_map_.begin();
            iter != this->status_map_.end(); ++iter)
    {
    	BasicStatus* cur_status = iter->second->__status_list.top();
    	JUDGE_CONTINUE(cur_status != NULL && cur_status->__buff_type == buff_type);

    	status = cur_status;
    	return 0;
    }

    return -1;
}

int GameStatus::is_have_status(int status_id)
{
    BasicStatus *status = 0;
    return this->find_status(status_id, status) == 0;
}

int GameStatus::is_have_status_type(int buff_type)
{
	for (GameStatus::StatusMap::iterator iter = this->status_map_.begin();
			iter != this->status_map_.end(); ++iter)
	{
		BasicStatus* status = iter->second->__status_list.top();
		JUDGE_CONTINUE(status != NULL && status->__buff_type == buff_type);
		return true;
	}

	return false;
}

double GameStatus::find_status_value(int id, int type)
{
	BasicStatus* status = NULL;
	JUDGE_RETURN(this->find_status(id, status) == 0, 0);
	return status->fetch_value(type);
}

double GameStatus::find_status_value_by_type(int buff_type, int value_type)
{
    BasicStatusVec status_vec;
    this->find_status(buff_type, status_vec);

    double total_value = 0;
    for (BasicStatusVec::iterator iter = status_vec.begin();
    		iter != status_vec.end(); ++iter)
    {
    	BasicStatus *status = *iter;
    	total_value += status->fetch_value(value_type);
    }

    return total_value;
}

DoublePair GameStatus::find_status_pair_value(int buff_type)
{
    BasicStatusVec status_vec;
    this->find_status(buff_type, status_vec);

    DoublePair total;
    for (BasicStatusVec::iterator iter = status_vec.begin();
    		iter != status_vec.end(); ++iter)
    {
    	BasicStatus *status = *iter;
    	total.first += status->fetch_value(1);
    	total.second += status->fetch_value(2);
    }

    return total;
}

// type: 0 == die, 1 == exit scene, 2 == exit game
int GameStatus::clear_status(int type)
{
	GameFighter* fighter = this->fighter();
	fighter->remove_die_protect();

	// 清除有效果的状态BUFF
	std::vector<StatusQueueNode *> del_list;
	for (GameStatus::StatusMap::iterator iter = this->status_map().begin();
	        iter != this->status_map().end(); ++iter)
	{
		StatusQueueNode* node = iter->second;
		JUDGE_CONTINUE(node->__remove_type[type] == 1);

	    for (size_t i = 0; i < node->__status_list.size(); ++i)
	    {
	    	BasicStatus* status = node->__status_list.node(i);
	        this->check_and_set_status_effect_type(status->status_type());
	    }

	    del_list.push_back(node);
	}

	if (this->status_u_flags_.test(GameEnum::STATUS_U_PROP) && del_list.empty() == false)
	{
		StatusQueueNode* node = del_list[0u];
		this->trace_force_.set_trace_info(fighter, SubObj(node->__status, type, del_list.size()));
	}

	for (std::vector<StatusQueueNode *>::iterator iter = del_list.begin();
	        iter != del_list.end(); ++iter)
	{
		StatusQueueNode* node = *iter;
	    this->status_map().unbind(node->__status);
	    this->status_queue().remove(node);

	    if (node->__status_list.top() != 0)
	    {
	    	this->process_remove_status(node->__status_list.top(), false);
	    }

	    MAP_MONITOR->status_queue_node_pool()->push(node);
	}

	if (this->status_u_flags_.test(GameEnum::STATUS_U_PROP))
	{
		this->refresh_all_status_property();
		this->check_and_dump_trace();
	}
	else if (this->status_u_flags_.test(GameEnum::STATUS_U_SPEED))
    {
    	this->refresh_all_status_property();
    }

	return 0;
}

// 处理状态添加时的属性增加或加血效果
int GameStatus::increase_status_effect(BasicStatus *status, int org_enter_type, int refresh_type)
{
	int enter_type = org_enter_type;
    switch (status->status_type())
    {
	case BasicStatus::BLOOD:
	case BasicStatus::DIRECTBLOOD:
	case BasicStatus::DIRECTMAXBLOOD:
		if (enter_type == ENTER_SCENE_TRANSFER || refresh_type == STATUS_REFRESH_ALL)
		{
			break;
		}
		return this->update_blood_status(status);
	case BasicStatus::MAXBLOOD:
		this->status_u_flags_.set(GameEnum::STATUS_U_PROP);
		if (refresh_type == STATUS_REFRESH_ALL)
		{
			enter_type = ENTER_SCENE_TRANSFER;
		}
		return this->inc_max_blood(status, enter_type);
	case BasicStatus::MAGIC:
	case BasicStatus::DIRECTMAGIC:
	case BasicStatus::DIRECTMAXMAGIC:
		if (enter_type == ENTER_SCENE_TRANSFER || refresh_type == STATUS_REFRESH_ALL)
		{
			break;
		}
		return this->update_magic_status(status);
	case BasicStatus::MAXMAGIC:
	{
		this->status_u_flags_.set(GameEnum::STATUS_U_PROP);
		if (refresh_type == STATUS_REFRESH_ALL)
		{
			enter_type = ENTER_SCENE_TRANSFER;
		}
		return this->inc_max_magic(status);
	}
	case BasicStatus::SPEED:
	{
		this->status_u_flags_.set(GameEnum::STATUS_U_SPEED);
		return this->inc_speed(status);
	}
	case BasicStatus::ATTACK:
	case BasicStatus::DEFENCE:
	case BasicStatus::CRIT:
	case BasicStatus::TOUGHNESS:
	case BasicStatus::HIT:
	case BasicStatus::AVOID:
	case BasicStatus::DAMAGE:
	case BasicStatus::FLASH_AVOID:
	case BasicStatus::EXEMPT:
	{
		this->status_u_flags_.set(GameEnum::STATUS_U_PROP);
		return this->inc_second_fight_prop(status);
	}
	case BasicStatus::DIRECTHURT:
	{
		return this->direct_hurt(status);
	}
	case BasicStatus::REPEATHURT:
	{
		return this->repeat_hurt(status);
	}
	case BasicStatus::STONE_PLAYER:
	{
		return this->notify_stone_player_tick(status);
	}
	case BasicStatus::INSIDE_ICE:
	{
		JUDGE_RETURN(refresh_type == STATUS_REFRESH_ONCE, 0);
		return this->inside_ice_hurt(status);
	}
	case BasicStatus::JIAN_DROP:
	{
		JUDGE_RETURN(refresh_type == STATUS_REFRESH_ONCE, 0);
		return this->jian_drop_hurt(status);
	}
	case BasicStatus::MULTI_PROP:
	{
		this->status_u_flags_.set(GameEnum::STATUS_U_PROP);
		return this->inc_multi_fight_prop(status);
	}
    case BasicStatus::TBATTLE_TREASURE:
    {
    	if (refresh_type == STATUS_REFRESH_ONCE)
    	{
    		return this->inc_tbattle_treasure_status_effect(status, enter_type, refresh_type);
    	}
    	break;
    }
	default:
		break;
    }

    return 0;
}

int GameStatus::status_effect_type(int status_type)
{
    switch (status_type)
    {
	case BasicStatus::MAXBLOOD:
	case BasicStatus::MAXMAGIC:
	case BasicStatus::ATTACK:
	case BasicStatus::DEFENCE:
	case BasicStatus::CRIT:
	case BasicStatus::TOUGHNESS:
	case BasicStatus::HIT:
	case BasicStatus::AVOID:
	case BasicStatus::DAMAGE:
	case BasicStatus::EXEMPT:
		return STATUS_UPDATE_FIGHT_PROP;
	case BasicStatus::SPEED:
		return STATUS_UPDATE_SPEED;
	default:
		break;
    }
    return STATUS_UPDATE_OTHER;
}

void GameStatus::check_and_dump_trace()
{
	JUDGE_RETURN(this->trace_force_.trace_dump_a() == true, ;);

	MapPlayer* player = dynamic_cast<MapPlayer*>(this->fighter());
	JUDGE_RETURN(player != NULL, ;);

	SubObj& sub = this->trace_force_.sub_info();
	player->record_other_serial(MAIN_EXECEPTION_SERIAL, 1, sub.val1_, sub.val2_, sub.val3_);
}

void GameStatus::check_and_set_status_effect_type(int status_type)
{
	int status_effect_type = this->status_effect_type(status_type);
	if (status_effect_type == STATUS_UPDATE_FIGHT_PROP)
	{
		this->status_u_flags_.set(GameEnum::STATUS_U_PROP);
	}
	else if (status_effect_type == STATUS_UPDATE_SPEED)
	{
		this->status_u_flags_.set(GameEnum::STATUS_U_SPEED);
	}
}

int GameStatus::refresh_status(const Time_Value &nowtime)
{
    int i = 0;
    StatusQueueNode *queue_node = 0;

    while ((queue_node = this->status_queue().top()) != 0)
    {
        if (queue_node->__check_tick > nowtime && 
                queue_node->__status_list.size() > 0)
        {
            break;
        }

        this->status_queue().pop();
        if (queue_node->__status_list.size() <= 0)
        {
            this->status_map().unbind(queue_node->__status);
            MAP_MONITOR->status_queue_node_pool()->push(queue_node);
            continue;
        }

        this->update_status(queue_node);

        if (++i == 10)
        {
            break;
        }
    }

    if (this->status_u_flags_.test(GameEnum::STATUS_U_PROP) ||
    		this->status_u_flags_.test(GameEnum::STATUS_U_SPEED))
    {
    	this->refresh_all_status_property();
    }

    return 0;
}

// 状态时间到或强制删除时调用此接口;
int GameStatus::process_remove_status(BasicStatus *status, const bool is_timeout)
{
	GameFighter* fighter = this->fighter();
    if (this->is_forbit_move_status(status->status_type()) == true
    		&& this->validate_no_forbit_move_status() == 0)
    {
    	fighter->notify_fighter_exit_stay();
    }

    // 处理当前所移除的状态相关通知接口
	switch(status->status_type())
	{
	case BasicStatus::RELIVE_PROTECT:
	{
		fighter->notify_update_player_info(GameEnum::PLAYER_INFO_PROTECT);
		break;
	}
    case BasicStatus::SHIELD:
    {
    	fighter->launch_shield_hurt(status);
        break;
    }
    case BasicStatus::ROLE_SHIELD:
    {
    	MapPlayer* player = dynamic_cast<MapPlayer*>(fighter);
    	JUDGE_BREAK(player != NULL);
    	player->trigger_role_shield_disappear(std::max<double>(
    			status->__value1, status->__value4));
    	break;
    }
    case BasicStatus::STONE_PLAYER:
    {
        this->notify_exit_stone_state(status, is_timeout);
        if (is_timeout == true && fighter->is_death() == false)
        {
        	fighter->fighter_restore_all(FIGHT_TIPS_SYSTEM_AUTO, FIGHT_UPDATE_BLOOD);
        }
        break;
    }
    case BasicStatus::JUMPING:
    {
    	fighter->set_finish_jump(status, is_timeout);
    	break;
    }
    case BasicStatus::TBATTLE_TREASURE:
    {
        if (is_timeout == true)
        {
            // 增加积分
            this->process_tbattle_treasure_status_timeout(status);
        }
        break;
    }
	default:
		break;
	}

	return this->notify_remove_status(status);
}

int GameStatus::update_status(StatusQueueNode *node)
{
    BasicStatus *status = node->__status_list.top();
    Time_Value nowtime = Time_Value::gettimeofday();
    if (status->__last_tick > Time_Value::zero)
    {
        if (status->__interval == Time_Value::zero
        		|| status->__last_tick < status->__interval)
        {
            status->__check_tick = nowtime + status->__last_tick;
            status->__last_tick = Time_Value::zero;
            node->__check_tick = status->__check_tick;
        }
        else
        {
            status->__check_tick = nowtime + status->__interval;
            status->__last_tick -= status->__interval;
            node->__check_tick = status->__check_tick;
        }

        this->refresh_single_status(status);
        this->notify_status_effect_update(status);

        node->__status_list.remove(status);
        node->__status_list.push(status);
        this->status_queue().push(node);
    }
    else
    {
    	this->check_and_set_status_effect_type(status->status_type());
        node->__status_list.pop();
        {
            BasicStatus *top_status = 0;
            while ((top_status = node->__status_list.top()) != 0)
            {
                if (top_status->__check_tick > nowtime
                		|| top_status->__last_tick > Time_Value::zero)
                {
                    break;
                }

                node->__status_list.pop();
                MAP_MONITOR->status_pool()->push(top_status);
            }
        }

        if (node->__status_list.top() == 0)
        {
            this->status_map().unbind(node->__status);
            MAP_MONITOR->status_queue_node_pool()->push(node);

            this->process_remove_status(status);
            if (this->status_u_flags_.test(GameEnum::STATUS_U_PROP)
            		|| this->status_u_flags_.test(GameEnum::STATUS_U_SPEED))
            {
            	this->refresh_all_status_property();
            }
        }
        else
        {
            BasicStatus* top_status = node->__status_list.top();
            node->__check_tick = top_status->__check_tick;
            this->refresh_single_status(top_status);
            this->status_queue().push(node);
        }

        MAP_MONITOR->status_pool()->push(status);
    }
    return 0;
}

int GameStatus::accumulate_status(BasicStatus *n_status, const BasicStatus &status)
{
    // 处理状态累加;
    switch (status.__status)
    {
        case BasicStatus::BLOOD:
        {
            this->accumulate_blood_status(n_status, status);
            break;
        }
        case BasicStatus::ATTACK:
        case BasicStatus::DEFENCE:
        case BasicStatus::CRIT:
        case BasicStatus::TOUGHNESS:
        case BasicStatus::HIT:
        case BasicStatus::AVOID:
        case BasicStatus::FLASH_AVOID:
        case BasicStatus::SPEED:
        {
        	// 不叠加数值，时间直接重置；
            StatusQueueNode *queue_node = 0;
            if (this->status_map().find(n_status->__status, queue_node) == 0)
            {
                this->status_queue().remove(queue_node);
                queue_node->__status_list.remove(n_status);

        	    n_status->__value1 = status.__value1;
        	    n_status->__value2 = status.__value2;
        	    n_status->__value3 = status.__value3;
        	    n_status->__value4 = status.__value4;
        	    n_status->__value5 = status.__value5;
        	    n_status->__accumulate_tims += 1;
                n_status->__attacker = status.__attacker;
                n_status->__level = status.__level;
                n_status->__skill_id = status.__skill_id;

                Time_Value nowtime = Time_Value::gettimeofday();
                if (status.__last_tick > Time_Value::zero)
                {
                    if (status.__interval == Time_Value::zero ||
                            status.__last_tick < status.__interval)
                    {
                        n_status->__check_tick = nowtime + status.__last_tick;
                        n_status->__last_tick = Time_Value::zero;
                    }
                    else
                    {
                        n_status->__last_tick = status.__last_tick;
                        n_status->__interval = status.__interval;
                    }
                }

                queue_node->__status_list.push(n_status);

                BasicStatus *top_status = queue_node->__status_list.top();
                queue_node->__check_tick = top_status->__check_tick;
                this->status_queue().push(queue_node);
            }
            break;
        }

        default:
        {
        	// 叠加数值，时间直接重置；
            StatusQueueNode *queue_node = 0;
            if (this->status_map().find(n_status->__status, queue_node) == 0)
            {
                this->status_queue().remove(queue_node);
                queue_node->__status_list.remove(n_status);

        	    n_status->__value1 += status.__value1;
        	    n_status->__value2 += status.__value2;
        	    n_status->__value3 += status.__value3;
        	    n_status->__value4 += status.__value4;
        	    n_status->__value5 += status.__value5;
        	    n_status->__accumulate_tims += 1;
                n_status->__attacker = status.__attacker;
                n_status->__level = status.__level;
                n_status->__skill_id = status.__skill_id;

                Time_Value nowtime = Time_Value::gettimeofday();
                if (status.__last_tick > Time_Value::zero)
                {
                    if (status.__interval == Time_Value::zero ||
                            status.__last_tick < status.__interval)
                    {
                        n_status->__check_tick = nowtime + status.__last_tick;
                        n_status->__last_tick = Time_Value::zero;
                    }
                    else
                    {
                        n_status->__last_tick = status.__last_tick;
                        n_status->__interval = status.__interval;
                    }
                }
                
                queue_node->__status_list.push(n_status);

                BasicStatus *top_status = queue_node->__status_list.top();
                queue_node->__check_tick = top_status->__check_tick;
                this->status_queue().push(queue_node);
            }
            break;
        }
    }
    
    if (n_status->__interval == Time_Value::zero)
    {
    	this->refresh_all_status_property();
    }

    return 0;
}

int GameStatus::validate_accumulate_status(BasicStatus *status, const BasicStatus &new_status)
{
    switch (status->status_type())
    {
    case BasicStatus::ATTACK:
    case BasicStatus::DEFENCE:
    case BasicStatus::CRIT:
    case BasicStatus::TOUGHNESS:
    case BasicStatus::HIT:
    case BasicStatus::AVOID:
    case BasicStatus::FLASH_AVOID:
    case BasicStatus::SPEED:
    case BasicStatus::MAXBLOOD:
    case BasicStatus::MAXMAGIC:
    	// 加属性的BUFF不可叠加
    	return -1;
    	break;
    case BasicStatus::BLOOD:
    default:
    {
        // 判断是否超过叠加次数
    	if (new_status.__accumulate_tims > 0 && status->__accumulate_tims < new_status.__accumulate_tims)
    		return 0;
        break;
    }
    }

    return -1;
}

int GameStatus::validate_cover_status(const BasicStatus *org_status, const BasicStatus *t_status)
{
//    switch (org_status->status_type())
//    { // 不需要覆盖的状态
//    case BasicStatus::SPEED:
//    case BasicStatus::ATTACK:
//    case BasicStatus::DEFENCE:
//    case BasicStatus::CRIT:
//    case BasicStatus::TOUGHNESS:
//    case BasicStatus::HIT:
//    case BasicStatus::AVOID:
//    case BasicStatus::DATTACK:
//    case BasicStatus::DDEFENCE:
//    case BasicStatus::DTOUGHNESS:
//    case BasicStatus::DHIT:
//    case BasicStatus::DAVOID:
//    {
//        // 以防状态队列过长，当队列超过5个时清除首状态再插入
//        StatusQueueNode *queue_node = 0;
//        if (this->status_map().find(org_status->__status, queue_node) == 0 &&
//                queue_node->__status_list.size() > 20)
//            return 0;
//        return -1;
//        break;
//    }
//    case BasicStatus::DIZZY:
//    case BasicStatus::SILENCE:
//    case BasicStatus::STAY:
//    case BasicStatus::SUPPERMAN:
//    case BasicStatus::EXEMPT:
//    {
//        Time_Value nowtime = Time_Value::gettimeofday();
//        Time_Value last_tick = org_status->__check_tick - nowtime;
//        if (last_tick < t_status->__last_tick)
//            return 0;
//        return -1;
//        break;
//    }
//    default:
//        break;
//    }
    return 0;
}

int GameStatus::compare_status_value(BasicStatus *left, BasicStatus *right)
{
    double *value_left = &(left->__value1),
           *value_right = &(right->__value1);
    for (int i = 0; i < 5; ++i)
    {
        if (*(value_left + i) < *(value_right + i))
            return -1;
        else if (*(value_left + i) > *(value_right + i))
            return 1;
    }

    return 0;
}

int GameStatus::make_up_status_msg(Block_Buffer* buff, const bool send_by_gate)
{
	GameFighter* fighter = this->fighter();

	Proto80400203 respond;
	respond.set_fighter_id(fighter->fighter_id());

    for (GameStatus::StatusMap::iterator iter = this->status_map_.begin();
            iter != this->status_map_.end(); ++iter)
    {
    	BasicStatus* status = iter->second->__status_list.top();
    	JUDGE_CONTINUE(status != NULL && status->__show_type > 0);

        ProtoStatus *proto_status = respond.add_status_list();
        status->serialize(proto_status);

        respond.add_status_id_list(status->__status);
    }

	MapPlayer* player = dynamic_cast<MapPlayer*>(this);
    if (player != NULL && player->blood_container().cur_blood_ > 0)
    {
    	//血包
    	BloodContainer& container = player->blood_container();
        ProtoStatus* proto_status = respond.add_status_list();

        container.copy_to_status();
        container.status_.serialize(proto_status);

    	respond.add_status_id_list(proto_status->status());
    }

    if (send_by_gate == false)
    {
        ProtoClientHead head;
        head.__recogn = ACTIVE_FIGHT_STATUS;
        return this->make_up_client_block(buff, &head, &respond);
    }
    else
    {
		ProtoHead head;
		head.__recogn = ACTIVE_FIGHT_STATUS;
		head.__role_id = fighter->fighter_id();
		head.__scene_id = fighter->scene_id();
		return this->make_up_gate_block(buff, &head, &respond);
    }
}

int GameStatus::notify_status(const bool send_by_gate)
{
    Block_Buffer buff;
    this->make_up_status_msg(&buff, send_by_gate);
    return this->respond_to_client(&buff);
}

int GameStatus::notify_remove_status(BasicStatus* status)
{
	GameFighter* fighter = this->fighter();
	JUDGE_RETURN(status->__show_type > 0, -1);

    Scene *scene = fighter->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    Proto80400205 respond;
    respond.set_fighter_id(fighter->fighter_id());
    respond.set_status(status->__status);

	switch(status->__show_type)
	{
	case 1:
	{
		fighter->respond_to_broad_area(&respond);
		break;
	}
	case 2:
	{
		fighter->respond_to_client(ACTIVE_REMOVE_FIGHT_STATUS, &respond);
		break;
	}
	}

	return 0;
}

int GameStatus::notify_update_status(BasicStatus *status)
{
	GameFighter* fighter = this->fighter();
	JUDGE_RETURN(status->__show_type > 0, 0);

    if (status->__client_msg == 1)
    {
        Proto80400203 respond;
        respond.set_fighter_id(fighter->fighter_id());

    	int total_size = status->__value6.size() / 2;
    	for (int i = 0; i < total_size; ++i)
    	{
    		respond.add_status_id_list(status->client_status());
            status->serialize_b(i, respond.add_status_list());
    	}

        return this->notify_update_status_info(status->__show_type,
        		ACTIVE_FIGHT_STATUS, &respond);
    }
    else
    {
        Proto80400207 respond;
        respond.set_status(status->client_status());
        respond.set_fighter_id(fighter->fighter_id());
        status->serialize(respond.mutable_status_detial());
        return this->notify_update_status_info(status->__show_type,
        		ACTIVE_FIGHT_STATUS_UPDATE, &respond);
    }
}

int GameStatus::notify_update_status_info(int type, int recogn, Message* msg)
{
	switch(type)
	{
	case 1:
	{
		return this->fighter()->respond_to_broad_area(msg);
	}
	case 2:
	{
		return this->fighter()->respond_to_client(recogn, msg);
	}
	default:
	{
		return -1;
	}
	}

	return 0;
}

int GameStatus::notify_status_update_property(void)
{
    if (this->status_u_flags_.test(GameEnum::STATUS_U_PROP))
    {
        this->fighter()->update_fight_property(BasicElement::STATUS);
    }

    if (this->status_u_flags_.test(GameEnum::STATUS_U_SPEED))
    {
        this->fighter()->notify_update_speed();
    }

    this->status_u_flags_.reset();
    return 0;
}

int GameStatus::refresh_all_status_property(int enter_type)
{
	GameFighter *fighter = this->fighter();
	FightDetail& fight_detail = fighter->fight_detail();

	double prev_blood = fight_detail.__blood;
	double prev_max_blood = fight_detail.__blood_total(fighter);
	double left_blood_percent = prev_blood / prev_max_blood;

	double prev_magic = fight_detail.__magic;
	double prev_max_magic = fight_detail.__magic_total(fighter);
	double left_magic_percent = prev_magic / prev_max_magic;

	fight_detail.clear_all_fight_property(BasicElement::STATUS);
	fighter->mover_detail().__speed.set_single(0, BasicElement::STATUS);
	fighter->mover_detail().__speed_multi.set_single(0, BasicElement::STATUS);

	uint total_size = this->status_queue_.size();
	for (uint i = 0; i < total_size; ++i)
	{
		JUDGE_BREAK(i < this->status_queue_.size());

		StatusQueueNode *node = this->status_queue_.node(i);
		JUDGE_CONTINUE(node != NULL && node->__status_list.top() != NULL);

		BasicStatus *status = node->__status_list.top();
		JUDGE_CONTINUE(status != NULL);
		JUDGE_CONTINUE(status->__inoperative_scene.count(fighter->scene_id()) <= 0);

		this->increase_status_effect(status, enter_type, STATUS_REFRESH_ALL);
	}

	if (enter_type != ENTER_SCENE_TRANSFER && fighter->is_enter_scene() == true)
	{
		fight_detail.set_cur_blood(left_blood_percent);
	    fight_detail.set_cur_magic(left_magic_percent);

	    if (prev_blood != fight_detail.__blood)
	    {
	    	int real_value = prev_blood - fight_detail.__blood;
	    	fighter->notify_fight_update(FIGHT_UPDATE_BLOOD, real_value, 0,
	        		0, fight_detail.__blood, FIGHT_TIPS_SYSTEM_AUTO);
	    }

	    if (prev_max_magic != fight_detail.__magic)
	    {
	    	int real_value = prev_magic - fight_detail.__magic;
	    	fighter->notify_fight_update(FIGHT_UPDATE_MAGIC, real_value, 0,
	        		0, fight_detail.__magic, FIGHT_TIPS_SYSTEM_AUTO);
	    }

	    this->notify_status_update_property();
	}

	return 0;
}

GameStatus::StatusUpdateFlagSet &GameStatus::status_u_flags(void)
{
	return this->status_u_flags_;
}

int GameStatus::refresh_single_status(BasicStatus *status)
{
	return this->increase_status_effect(status);
}

int GameStatus::notify_status_effect_update(BasicStatus *status)
{
    return 0;
}

int GameStatus::insert_defender_status(GameStatus *defender, int status,
		double interval, double last, int accumulate_times,
		double val1, double val2, double val3, double val4, double val5)
{
	JUDGE_RETURN(CONFIG_INSTANCE->validate_buff(status) == true, -1);

	GameFighter* attackor = this->fighter();
    Int64 attack_id = attackor->fighter_id();
    int skill_id = attackor->current_skill().__skill_id;
    int skill_level = attackor->current_skill().__skill_level;

    Time_Value interval_tick = Time_Value::gettime(interval);
    Time_Value last_tick = Time_Value::gettime(last);

	BasicStatus b_status(status);
    b_status.__accumulate_tims = accumulate_times;
    b_status.set_all(last_tick, interval_tick,
    		attack_id, skill_id, skill_level,
            val1, val2, val3, val4, val5);

    return this->insert_defender_status(defender, b_status);
}

int GameStatus::insert_defender_status(GameStatus *defender, const BasicStatus& b_status)
{
	GameFighter* fighter = defender->fighter();
	JUDGE_RETURN(fighter->find_avoid_buff(b_status.status_type()) != 0, -1);

	for (IntMap::const_iterator iter = b_status.__avoid_buff.begin();
			iter != b_status.__avoid_buff.end(); ++iter)
	{
		JUDGE_CONTINUE(fighter->is_have_status_type(iter->first) == true);
		return -1;
	}

	bool is_avoid = this->is_avoid_dizzy(defender, b_status);
	JUDGE_RETURN(is_avoid == false, -1);

    int ret = defender->insert_status(b_status);
    JUDGE_RETURN(ret == 0, ret);

	BasicStatus* c_status = NULL;
	JUDGE_RETURN(defender->find_status(b_status.__status, c_status) == 0, ret);

	return defender->notify_update_status(c_status);
}

int GameStatus::insert_defender_stage_status(GameStatus *defender, const Json::Value& effect_json)
{
//	const Json::Value& stage_conf = effect_json["stage_info"];
//	JUDGE_RETURN(stage_conf != Json::Value::null, -1);
//
//	BasicStatus head_status;
//	head_status.__client_status = effect_json["client_buff"].asInt();
//
//	head_status.set_normal(stage_conf[0u]["id"].asInt(),
//			stage_conf[0u]["last"].asDouble(),stage_conf[0u]["interval"].asDouble(),
//			stage_conf[0u]["value"].asDouble(), stage_conf[0u]["percent"].asDouble());
//
//	for (uint i = 1; i < stage_conf.size(); ++i)
//	{
//		BasicStatus* next_status = MAP_MONITOR->status_pool()->pop();
//		next_status->set_normal(stage_conf[i]["id"].asInt(),
//				stage_conf[i]["last"].asDouble(),stage_conf[i]["interval"].asDouble(),
//				stage_conf[i]["value"].asDouble(), stage_conf[i]["percent"].asDouble());
//		head_status.__after_status.push_back(next_status);
//	}
//
//	return this->insert_defender_status(defender, head_status);
	return 0;
}

void GameStatus::accumulate_blood_status(BasicStatus *n_status, const BasicStatus &status)
{
    n_status->__last_tick += status.__last_tick;
    n_status->__value1 += status.__value1;
//    n_status->__view1 = n_status->__value1;
}

int GameStatus::update_blood_status(BasicStatus *status, int tips)
{
	GameFighter* fighter = this->fighter();
	switch (status->status_type())
	{
	case BasicStatus::BLOOD:
	{
        int inc_blood = fighter->fetch_blood_differ();
        JUDGE_RETURN(inc_blood > 0, 0);

        inc_blood = std::min<int>(inc_blood, status->__value1);
        JUDGE_RETURN(inc_blood > 0, 0);

        int prev_blood = fighter->fight_detail().__blood;
        fighter->modify_blood_by_fight(-inc_blood, FIGHT_TIPS_STATUS,
        		status->__attacker, status->__skill_id);

        inc_blood = fighter->fight_detail().__blood - prev_blood;
        status->__value1 -= inc_blood;
		break;
	}
	case BasicStatus::DIRECTBLOOD:
	{
        double inc_blood = status->__value1 * -1;
        inc_blood += fighter->fetch_cur_blood(status->__value2 * -1);
        fighter->modify_blood_by_fight(inc_blood, FIGHT_TIPS_STATUS,
        		status->__attacker, status->__skill_id);
		break;
	}
	case BasicStatus::DIRECTMAXBLOOD:
	{
        double inc_blood = -1 * status->__value1;
        inc_blood += fighter->fetch_max_blood(-1 * status->__value2);
        fighter->modify_blood_by_fight(inc_blood, tips, status->__attacker, status->__skill_id);
		break;
	}
	}

    return 0;
}

int GameStatus::inc_max_blood(BasicStatus *status, const int enter_type)
{
    GameFighter *fighter = this->fighter();

    int prev_blood = fighter->fight_detail().__blood;
    if (GameCommon::is_zero(status->__value1) == false)
    {
    	fighter->blood_max_add(status->__value1, BasicElement::STATUS, enter_type);
    }

    if (GameCommon::is_zero(status->__value2) == false)
    {
    	double value = GameCommon::div_percent(status->__value2);
    	fighter->blood_max_multi_add(value, BasicElement::STATUS, enter_type);
    }

    int cur_blood = fighter->fight_detail().__blood;
    if (fighter->is_enter_scene() == true && prev_blood != cur_blood)
    {
    	int real_value = prev_blood - cur_blood;
    	fighter->notify_fight_update(FIGHT_UPDATE_BLOOD, real_value, 0,
        		0, fighter->fight_detail().__blood, FIGHT_TIPS_SYSTEM_AUTO);
    }

    return 0;
}

int GameStatus::reduce_max_blood(const int status, HistoryStatus *history)
{
    GameFighter *fighter = this->fighter();

    int prev_blood = fighter->fight_detail().__blood, is_modify = 0;

    if (std::abs(history->__value1) > 0.000001)
    	is_modify += fighter->blood_max_reduce(history->__value1, BasicElement::STATUS);
    if (std::abs(history->__value2) > 0.000001)
    	is_modify += fighter->blood_max_multi_reduce(history->__value2 / 100, BasicElement::STATUS);

    if (is_modify != 0)
    {
    	int real_value = prev_blood - fighter->fight_detail().__blood;
    	fighter->notify_fight_update(FIGHT_UPDATE_BLOOD, real_value, 0,
        		0, fighter->fight_detail().__blood, FIGHT_TIPS_SYSTEM_AUTO);
    }
    return 0;
}

int GameStatus::update_magic_status(BasicStatus *status)
{
//    if (status->__status == BasicStatus::MAGIC)
//    {
//        int prev_magic = this->fighter()->fight_detail().__magic;
//        int inc_magic = this->fighter()->fight_detail().__magic_total_i(this->fighter()) - prev_magic;
//        if (inc_magic <= 0)
//            return 0;
//
//        if (inc_magic > status->__value1)
//            inc_magic = status->__value1;
//
//        this->fighter()->modify_magic_by_notify(-inc_magic, FIGHT_TIPS_STATUS);
//        inc_magic = this->fighter()->fight_detail().__magic - prev_magic;
//        status->__value1 -= inc_magic;
//        status->__view1 = status->__value1;
//    }
//    else if (status->__status == BasicStatus::DIRECTMAGIC)
//    {
//        double inc_magic = status->__value1 * -1;
//        inc_magic += (status->__value2 * this->fighter()->fight_detail().__magic / 100 * -1);
//
//        this->fighter()->modify_magic_by_notify(inc_magic, FIGHT_TIPS_STATUS);
//    }
//    else if (status->__status == BasicStatus::DIRECTMAXMAGIC)
//    {
//        double inc_magic = status->__value1 * -1;
//        inc_magic += (status->__value2 * this->fighter()->fight_detail().__magic_total(this->fighter()) / 100 * -1);
//
//        this->fighter()->modify_magic_by_notify(inc_magic, FIGHT_TIPS_STATUS);
//    }

    return 0;
}

int GameStatus::inc_max_magic(BasicStatus *status)
{
    GameFighter *fighter = this->fighter();

    int prev_magic = fighter->fight_detail().__magic, is_modify = 0;

    if (GameCommon::is_zero(status->__value1) == false)
    	is_modify += fighter->magic_max_add(status->__value1, BasicElement::STATUS);
    if (GameCommon::is_zero(status->__value2) == false)
    	is_modify += fighter->magic_max_multi_add(status->__value2 / 100, BasicElement::STATUS);

    if (is_modify != 0)
    {
    	int real_value = prev_magic - fighter->fight_detail().__magic;
    	fighter->notify_fight_update(FIGHT_UPDATE_MAGIC, real_value, 0,
        		0, fighter->fight_detail().__magic, FIGHT_TIPS_SYSTEM_AUTO);
    }

    return 0;
}

int GameStatus::inc_speed(BasicStatus *status)
{
	int offset = BasicElement::STATUS;
    GameFighter *fighter = this->fighter();

    if (GameCommon::is_zero(status->__value1) == false)
    {
    	fighter->mover_detail().__speed.add_single(status->__value1, offset);
    }

    if (GameCommon::is_zero(status->__value2) == false)
    {
    	double value2 = GameCommon::div_percent(status->__value2);
    	fighter->mover_detail().__speed_multi.add_single(value2, offset);
    }

    return 0;
}

int GameStatus::inc_second_fight_prop(BasicStatus *status)
{
	GameFighter* fighter = this->fighter();
    FightDetail& fight_detail = fighter->fight_detail();
    int prev_attack = fight_detail.__attack_upper_total(fighter);

    //增加值
    if (GameCommon::is_zero(status->__value1) == false)
    {
        switch(status->status_type())
        {
        case BasicStatus::ATTACK:
        	fight_detail.__attack_lower.add_single(status->__value1, BasicElement::STATUS);
        	fight_detail.__attack_upper.add_single(status->__value1, BasicElement::STATUS);
            break;
        case BasicStatus::DEFENCE:
        	fight_detail.__defence_lower.add_single(status->__value1, BasicElement::STATUS);
        	fight_detail.__defence_upper.add_single(status->__value1, BasicElement::STATUS);
            break;
        case BasicStatus::CRIT:
        	fight_detail.__crit.add_single(status->__value1, BasicElement::STATUS);
            break;
        case BasicStatus::TOUGHNESS:
        	fight_detail.__toughness.add_single(status->__value1, BasicElement::STATUS);
            break;
        case BasicStatus::HIT:
        	fight_detail.__hit.add_single(status->__value1, BasicElement::STATUS);
            break;
        case BasicStatus::AVOID:
        	fight_detail.__avoid.add_single(status->__value1, BasicElement::STATUS);
            break;
        case BasicStatus::DAMAGE:
        	fight_detail.__damage.add_single(status->__value1, BasicElement::STATUS);
            break;
        default:
            break;
        }
    }

    //增加万分比
    if (GameCommon::is_zero(status->__value2) == false)
    {
        double value2 = GameCommon::div_percent(status->__value2);;
        switch(status->status_type())
        {
        case BasicStatus::ATTACK:
        	fight_detail.__attack_lower_multi.add_single(value2, BasicElement::STATUS);
        	fight_detail.__attack_upper_multi.add_single(value2, BasicElement::STATUS);
            break;
        case BasicStatus::DEFENCE:
        	fight_detail.__defence_lower_multi.add_single(value2, BasicElement::STATUS);
        	fight_detail.__defence_upper_multi.add_single(value2, BasicElement::STATUS);
            break;
        case BasicStatus::CRIT:
        	fight_detail.__crit_value_multi.add_single(value2, BasicElement::STATUS);
            break;
        case BasicStatus::TOUGHNESS:
        	fight_detail.__toughness_multi.add_single(value2, BasicElement::STATUS);
            break;
        case BasicStatus::HIT:
        	fight_detail.__hit_multi.add_single(value2, BasicElement::STATUS);
            break;
        case BasicStatus::AVOID:
        	fight_detail.__avoid_multi.add_single(value2, BasicElement::STATUS);
            break;
        case BasicStatus::DAMAGE:
        	fight_detail.__damage_multi.add_single(value2, BasicElement::STATUS);
        	break;
        default:
            break;
        }
    }

    int after_attack = fight_detail.__attack_upper_total(fighter);
    JUDGE_RETURN((after_attack - prev_attack) > 10000, 0);

	MapPlayer *player = dynamic_cast<MapPlayer *>(fighter);
	JUDGE_RETURN(player != NULL, 0);

	MSG_USER("attack bigger %ld %s status(%d %d %d) %d %d", player->role_id(), player->name(),
			status->__status, status->__skill_id, status->__accumulate_tims, prev_attack, after_attack);
    return 0;

}

int GameStatus::inc_all_fight_prop(BasicStatus *status)
{
	FightDetail& fight_detail = this->fighter()->fight_detail();

	if (GameCommon::is_zero(status->__value1) == false)
	{
	    double inc_value = status->__value1;
	    fight_detail.__attack_lower.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__attack_upper.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__defence_lower.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__defence_upper.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__blood_max.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__hit.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__damage.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__avoid.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__crit.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__toughness.add_single(inc_value, BasicElement::STATUS);
	}

	if (GameCommon::is_zero(status->__value2) == false)
	{
	    double inc_value = GameCommon::div_percent(status->__value2);
	    fight_detail.__attack_lower_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__attack_upper_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__defence_lower_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__defence_upper_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__blood_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__hit_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__damage_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__avoid_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__crit_value_multi.add_single(inc_value, BasicElement::STATUS);
	    fight_detail.__toughness_multi.add_single(inc_value, BasicElement::STATUS);
	}

    return 0;
}

int GameStatus::inc_multi_fight_prop(BasicStatus *status)
{
	int prop_index = status->conf()["prop"].asInt();
	JUDGE_RETURN(prop_index > 0, -1);

	const Json::Value& conf = CONFIG_INSTANCE->prop_unit(prop_index);
	JUDGE_RETURN(conf.empty() == false, -1);

	FightProperty conf_prop;
	conf_prop.make_up_name_prop(conf);

	FightDetail& fight_detail = this->fighter()->fight_detail();
	fight_detail.add_fighter_property(BasicElement::STATUS, conf_prop);

	return 0;
}

int GameStatus::direct_hurt(BasicStatus *status)
{
	GameFighter* fighter = this->fighter();
	JUDGE_RETURN(fighter != NULL, 0);

	double hurt_value = fighter->attack_in_hurt() * GameCommon::div_percent(
			status->__value2) + status->__value1;
	return fighter->modify_blood_by_fight(std::max<int>(1, hurt_value),
			FIGHT_TIPS_STATUS, status->__attacker);
}

int GameStatus::repeat_hurt(BasicStatus *status)
{
	status->__value3 += 1;
	JUDGE_RETURN(status->__value3 > 1, 0);	// 防止第一次调用时清除当前的上下文；

    GameFighter *attacker = 0;
    JUDGE_RETURN(this->fighter()->find_fighter(status->__attacker, attacker) == 0, 0);

	FighterSkill *skill = 0;
	JUDGE_RETURN(attacker->find_skill(status->__skill_id, skill) == 0, 0);

	CurrentSkill &fight_skill = attacker->fight_cur_skill();
	fight_skill.__skill_id = status->__skill_id;
	fight_skill.__skill_level = skill->__level;
	fight_skill.__defender_id = this->fighter()->fighter_id();
	fight_skill.effect_flag_reset();
	attacker->hurt(status->__value1, status->__value2);

    return 0;
}

int GameStatus::validate_no_forbit_move_status(void)
{
    JUDGE_RETURN(this->is_have_status_type(BasicStatus::STAY) == false, ERROR_MOVE_STAY);
    JUDGE_RETURN(this->is_have_status_type(BasicStatus::DIZZY) == false, ERROR_PLAYER_DIZZY);
    JUDGE_RETURN(this->is_have_status_type(BasicStatus::FORCE_DIZZY) == false, ERROR_PLAYER_DIZZY);

    return 0;
}

bool GameStatus::is_forbit_move_status(int status_type)
{
    switch (status_type)
    {
    case BasicStatus::STAY:
    case BasicStatus::DIZZY:
    case BasicStatus::STIFF:
    case BasicStatus::FORCE_DIZZY:
        return true;
    default:
        break;
    }
    return false;
}

int GameStatus::notify_enter_stone_state(void)
{
	GameFighter* fighter = this->fighter();

    BasicStatus *status = NULL;
    JUDGE_RETURN(this->find_status(BasicStatus::STONE_PLAYER, status) == 0, 0);

    Proto80400924 respond;
    respond.set_role_id(fighter->fighter_id());
    respond.set_state(1);
    return fighter->respond_to_broad_area(&respond, 1);
}

int GameStatus::notify_exit_stone_state(BasicStatus *status, const bool is_timeout)
{
	GameFighter* fighter = this->fighter();

    Proto80400924 respond;
    respond.set_role_id(fighter->fighter_id());
    respond.set_state(0);
    return fighter->respond_to_broad_area(&respond, 1);
}

int GameStatus::notify_stone_player_tick(BasicStatus *status)
{
	int left_tick = status->__last_tick.sec();
	if (left_tick < 0)
		left_tick = 0;

	Proto80400925 respond;
	respond.set_role_id(this->fighter()->fighter_id());
	respond.set_last_sec(left_tick);
	return this->fighter()->respond_to_broad_area(&respond, 1);
}

int GameStatus::sync_beast_fight_buf()
{
//	Int64 master_id = 0;
//	GameFighter *fighter = this->fighter();
//	if (fighter->is_beast())
//	{
//		MapBeast *beast = dynamic_cast<MapBeast *>(this->fighter());
//		if (beast != 0 && beast->master_id() > 0)
//		{
//			master_id = beast->master_id();
//		}
//		if(master_id > 0)
//		{
//			IDMap prop_map;
//			beast->fetch_prop_map(prop_map,BasicElement::STATUS);
//
//			Proto31402601 req;
//			for(IDMap::iterator iter = prop_map.begin(); iter != prop_map.end(); ++iter)
//			{
//				req.add_prop_id(iter->first);
//				req.add_prop_value(iter->second);
//			}
//			req.set_beast_id(beast->beast_id());
//			MAP_MONITOR->process_inner_logic_request(master_id,req);
//		}
//	}
	return 0;
}

int GameStatus::inside_ice_hurt(BasicStatus* status)
{
	status->__value3 += 1;
	JUDGE_RETURN(GameCommon::double_compare(status->__value3, status->__value4) >= 0, -1);

    GameFighter* attacker = this->fighter();
	JUDGE_RETURN(attacker->is_death() == false, -1);

	Scene* scene = attacker->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	CurrentSkill &fight_skill = attacker->fight_cur_skill();
	fight_skill.__skill_id = status->__skill_id;
	fight_skill.__skill_level = status->__level;

	MoverCoord center_coord;
	center_coord.set_pixel(status->__value1, status->__value2);

	const Json::Value& trigger_conf = CONFIG_INSTANCE->skill_detail(
			status->__skill_id, status->__level)["effect"][0u];

	Scene::MoverMap fighter_map;
	scene->fetch_all_rect_fighter(attacker, fighter_map, center_coord, 0,
			trigger_conf["rect"][0u].asInt(), trigger_conf["rect"][1u].asInt(), SubObj(20, 1));

	for (Scene::MoverMap::iterator iter = fighter_map.begin();
			iter != fighter_map.end(); ++iter)
	{
		GameFighter* defender = dynamic_cast<GameFighter*>(iter->second);
		attacker->direct_hurt_by_defender_max_blood(defender, trigger_conf["blood_percent"].asInt());
	}

	return 0;
}

int GameStatus::jian_drop_hurt(BasicStatus* status)
{
	status->__value3 += 1;
	JUDGE_RETURN(GameCommon::double_compare(status->__value3, status->__value4) >= 0, -1);

    GameFighter* attacker = this->fighter();
	JUDGE_RETURN(attacker->is_death() == false, -1);

	Scene* scene = attacker->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	CurrentSkill &fight_skill = attacker->fight_cur_skill();
	fight_skill.__skill_id = status->__skill_id;
	fight_skill.__skill_level = status->__level;

	const Json::Value& trigger_conf = CONFIG_INSTANCE->skill_detail(
			status->__skill_id, status->__level)["effect"][0u];
	int blood_percent = trigger_conf["blood_percent"].asInt();

	LongMap history_map;
	int total_size = status->__value6.size();

	for (int i = 0; i < total_size; i += 2)
	{
		MoverCoord center_coord;
		center_coord.set_pixel(status->__value6[i], status->__value6[i + 1]);

		Scene::MoverMap fighter_map;
		scene->fetch_all_around_player(attacker, fighter_map, center_coord, status->__value1);

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			JUDGE_CONTINUE(history_map.count(iter->first) == 0);

			GameFighter* defender = dynamic_cast<GameFighter*>(iter->second);
			attacker->direct_hurt_by_defender_max_blood(defender, blood_percent);

			history_map[iter->first] = true;
		}
	}

	return 0;
}

int GameStatus::set_next_stage_buff(BasicStatus* status)
{
//	GameFighter* fighter = NULL;
//	JUDGE_RETURN(this->fighter()->find_fighter(status->__attacker, fighter) == 0, -1);
//	JUDGE_RETURN(GameCommon::validate_fighter(fighter) == true, -1);
//
//	for (std::vector<BasicStatus*>::iterator iter = status->__after_status.begin();
//			iter != status->__after_status.end(); ++iter)
//	{
//		BasicStatus* next_status = *iter;
//		this->insert_defender_status(fighter, *next_status);
//	}
//
//	status->recyle_after_status();
	return 0;
}

int GameStatus::inc_tbattle_treasure_status_effect(BasicStatus *status, const int enter_type, const int refresh_type)
{
    return 0;
    // 属性增加
    
}

int GameStatus::process_tbattle_treasure_status_timeout(BasicStatus *status)
{
    return 0;
}

bool GameStatus::is_avoid_dizzy(GameStatus *defender, const BasicStatus& b_status)
{
	JUDGE_RETURN(defender != NULL && b_status.__buff_type == BasicStatus::DIZZY, false);

	int total_rate = 0;
	for (GameStatus::StatusMap::iterator iter = defender->status_map_.begin();
			iter != defender->status_map_.end(); ++iter)
	{
		BasicStatus* status = iter->second->__status_list.top();
		JUDGE_CONTINUE(status != NULL && status->__buff_type == BasicStatus::IMMUNE_DIZZY);

		total_rate += status->__value2;
	}
	JUDGE_RETURN(total_rate > 0, false);

	int rand_value = ::rand() % GameEnum::DAMAGE_ATTR_PERCENT;
	JUDGE_RETURN(rand_value <= total_rate, false);

	return true;
}

