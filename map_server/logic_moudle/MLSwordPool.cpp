/*
 * MLSwordPool.cpp
 *
 *  Created on: 2016年10月12日
 *      Author: lyw
 */

#include "MLSwordPool.h"
#include "MMOSwordPool.h"
#include "MapLogicPlayer.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "ScriptStruct.h"

MLSwordPool::MLSwordPool() {
	// TODO Auto-generated constructor stub

}

MLSwordPool::~MLSwordPool() {
	// TODO Auto-generated destructor stub
}

void MLSwordPool::reset(void)
{
	SwordPoolDetail& spool = this->spool_detail_;
	spool.reset();
}

int MLSwordPool::get_spool_info()
{
	SwordPoolDetail& spool = this->spool_detail_;
	JUDGE_RETURN(spool.open_ == true, 0);

	Proto51406001 respond;
	respond.set_level(spool.level_);
	respond.set_exp(spool.exp_);
	respond.set_style_lv(spool.stype_lv_);

	for (SwordPoolDetail::TodayTaskInfoMap::iterator iter = spool.today_task_map_.begin();
			iter != spool.today_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &pool_task = iter->second;
		JUDGE_CONTINUE(this->is_sword_pool_task(pool_task.task_id_) == true);

		const Json::Value &task = CONFIG_INSTANCE->sword_pool_task(pool_task.task_id_);
		ProtoSwordPoolTask *task_info = respond.add_task_info();
		task_info->set_task_id(pool_task.task_id_);
		task_info->set_total_num(pool_task.total_num_);
		task_info->set_left_num(pool_task.left_num_);
		task_info->set_once_exp(task["once_exp"].asInt());
	}

	for (SwordPoolDetail::LastTaskInfoMap::iterator iter = spool.last_task_map_.begin();
			iter != spool.last_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &pool_task = iter->second;
		JUDGE_CONTINUE(this->is_sword_pool_task(pool_task.task_id_) == true);

		const Json::Value &task = CONFIG_INSTANCE->sword_pool_task(pool_task.task_id_);
		ProtoSwordPoolTask *find_task = respond.add_find_task();

		find_task->set_task_id(pool_task.task_id_);
		find_task->set_total_num(pool_task.total_num_);
		find_task->set_left_num(pool_task.left_num_);
		find_task->set_once_exp(task["once_exp"].asInt());
	}
//	MSG_USER("Proto51406001: %s", respond.Utf8DebugString().c_str());

	FINER_PROCESS_RETURN(RETURN_FETCH_SWORD_POOL_INFO, &respond);
}

int MLSwordPool::uplevel_sword_pool()
{
	SwordPoolDetail& spool = this->spool_detail_;
	CONDITION_NOTIFY_RETURN(spool.open_ == true, RETURN_UPLEVEL_SWORD_POOL, ERROR_SWORD_POOL_IS_NOT_OPEN);

	int level = spool.level_;
	int cur_exp = spool.exp_;

	const Json::Value &next_level_info = CONFIG_INSTANCE->sword_pool_set_up(level+1);
	CONDITION_NOTIFY_RETURN(next_level_info != Json::Value::null, RETURN_UPLEVEL_SWORD_POOL, ERROR_IS_MAX_LEVEL);

	const Json::Value &set_up_info = CONFIG_INSTANCE->sword_pool_set_up(level);
	int upgrade_exp = set_up_info["upgrade_exp"].asInt();
	CONDITION_NOTIFY_RETURN(cur_exp >= upgrade_exp, RETURN_UPLEVEL_SWORD_POOL, ERROR_EXP_NO_ENOUGH);

	this->set_spool_level(spool.level_ + 1);
	int style_lvl = next_level_info["style_level"].asInt();
	if (style_lvl != spool.stype_lv_)
	{
		spool.stype_lv_ = style_lvl;
		// 改变外观
		this->notify_spool_style_lvl(true);
	}

	int add_exp = -upgrade_exp;
	this->add_sword_pool_exp(add_exp);

	// 改变人物属性
	this->spool_update_mount_info();
	this->refresh_spool_attr_add();

	// 发奖励
	int reward_id = next_level_info["reward_id"].asInt();
	SerialObj obj(ADD_FROM_SWORD_POOL_UP, spool.level_);
	this->add_reward(reward_id, obj);

	//剑池灵气流水
	this->record_other_serial(SWORD_POOL_EXP_DEL_SERIAL, spool.exp_, add_exp, spool.level_);
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

	Proto51406002 respond;
	respond.set_level(spool.level_);
	respond.set_exp(spool.exp_);
	respond.set_style_lv(spool.stype_lv_);
	FINER_PROCESS_RETURN(RETURN_UPLEVEL_SWORD_POOL, &respond);
}

int MLSwordPool::find_back_task_for_one(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto11406003 *, request, msg, -1);

	int find_vip = CONFIG_INSTANCE->sword_pool("find_vip").asInt();
	CONDITION_NOTIFY_RETURN(this->vip_detail().__vip_level >= find_vip, RETURN_FIND_BACK_ONE_TASK,
			ERROR_VIP_LEVEL);

	SwordPoolDetail& spool = this->spool_detail_;
	CONDITION_NOTIFY_RETURN(spool.open_ == true, RETURN_FIND_BACK_ONE_TASK,
			ERROR_SWORD_POOL_IS_NOT_OPEN);

	int task_id = request->task_id();
	CONDITION_NOTIFY_RETURN(this->is_sword_pool_task(task_id) == true,
			RETURN_FIND_BACK_ONE_TASK, ERROR_TASK_ID);

	SwordPoolDetail::PoolTaskInfo &last_task = spool.last_task_map_[task_id];
	CONDITION_NOTIFY_RETURN(last_task.left_num_ > 0,
			RETURN_FIND_BACK_ONE_TASK, ERROR_TASK_HAS_FINISH);

	const Json::Value &task = CONFIG_INSTANCE->sword_pool_task(task_id);
	int once_exp = task["once_exp"].asInt();
	int find_cost = task["find_cost"].asInt();

	int add_exp = once_exp * last_task.left_num_;
	int total_cost = find_cost * last_task.left_num_;
	Money need_money(total_cost);
	int ret = this->pack_money_sub(need_money, SUB_MONEY_SPOOL_FIND_BACK_COST);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FIND_BACK_ONE_TASK, ret);

	this->add_sword_pool_exp(add_exp);
	last_task.left_num_ = 0;

	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

	//向客户端发送最新的剑池数据
//	this->get_spool_info();

	Proto51406003 respond;
	respond.set_add_exp(add_exp);
	respond.set_task_id(task_id);
	FINER_PROCESS_RETURN(RETURN_FIND_BACK_ONE_TASK, &respond);
}

int MLSwordPool::find_back_all_task()
{

	int find_vip = CONFIG_INSTANCE->sword_pool("find_vip").asInt();
	CONDITION_NOTIFY_RETURN(this->vip_detail().__vip_level >= find_vip, RETURN_FIND_BACK_ALL_TASK,
			ERROR_VIP_LEVEL);

	SwordPoolDetail& spool = this->spool_detail_;
	CONDITION_NOTIFY_RETURN(spool.open_ == true, RETURN_FIND_BACK_ALL_TASK,
			ERROR_SWORD_POOL_IS_NOT_OPEN);

	int total_cost = 0;
	int add_exp = 0;
	for (SwordPoolDetail::LastTaskInfoMap::iterator iter = spool.last_task_map_.begin();
			iter != spool.last_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &last_task = iter->second;
		JUDGE_CONTINUE(last_task.left_num_ > 0);
		JUDGE_CONTINUE(this->is_sword_pool_task(last_task.task_id_) == true);

		const Json::Value &task = CONFIG_INSTANCE->sword_pool_task(last_task.task_id_);
		int once_exp = task["once_exp"].asInt();
		int find_cost = task["find_cost"].asInt();

		total_cost += find_cost * last_task.left_num_;
		add_exp += once_exp * last_task.left_num_;
	}
	CONDITION_NOTIFY_RETURN(total_cost > 0, RETURN_FIND_BACK_ALL_TASK, ERROR_LAST_TASK_IS_FINISH);

	Money need_money(total_cost);
	int ret = this->pack_money_sub(need_money, SUB_MONEY_SPOOL_FIND_BACK_COST);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FIND_BACK_ALL_TASK, ret);

	this->add_sword_pool_exp(add_exp);
	for (SwordPoolDetail::LastTaskInfoMap::iterator iter = spool.last_task_map_.begin();
			iter != spool.last_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &last_task = iter->second;
		last_task.left_num_ = 0;
	}
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

	//向客户端发送最新的剑池数据
	this->get_spool_info();

	Proto51406004 respond;
	respond.set_add_exp(add_exp);
	FINER_PROCESS_RETURN(RETURN_FIND_BACK_ALL_TASK, &respond);
}

int MLSwordPool::change_spool_style_lv(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto11406005 *, request, msg, -1);

	SwordPoolDetail& spool = this->spool_detail_;
	CONDITION_NOTIFY_RETURN(spool.open_ == true, RETURN_CHANGE_STYLE_LV, ERROR_SWORD_POOL_IS_NOT_OPEN);

	int change_lv = request->target_lv();
	const Json::Value &set_up_info = CONFIG_INSTANCE->sword_pool_set_up(spool.level_);
	int max_change_lv = set_up_info["style_level"].asInt();
	CONDITION_NOTIFY_RETURN(change_lv <= max_change_lv, RETURN_CHANGE_STYLE_LV, ERROR_CHANGE_STYLE_LV_MAX);
	CONDITION_NOTIFY_RETURN(change_lv != spool.stype_lv_, RETURN_CHANGE_STYLE_LV, ERROR_CHANGE_SAME_STYLE_LV);

	spool.stype_lv_ = change_lv;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

	this->notify_spool_style_lvl(true);

	Proto51406005 respond;
	respond.set_style_lv(spool.stype_lv_);
	FINER_PROCESS_RETURN(RETURN_CHANGE_STYLE_LV, &respond);
}

int MLSwordPool::fetch_spool_style_lvl()
{
	SwordPoolDetail& spool = this->spool_detail_;
	JUDGE_RETURN(spool.open_ == true, 0);

	return spool.stype_lv_;
}

int MLSwordPool::sync_transfer_spool(int scene_id)
{
	Proto31400144 spool_info;

	SwordPoolDetail& spool = this->spool_detail_;
	spool_info.set_level(spool.level_);
	spool_info.set_exp(spool.exp_);
	spool_info.set_open(spool.open_);
	spool_info.set_style_lvl(spool.stype_lv_);
	spool_info.set_refresh_tick(spool.refresh_tick_.sec());

	for (SwordPoolDetail::TodayTaskInfoMap::iterator iter = spool.today_task_map_.begin();
			iter != spool.today_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &pool_task = iter->second;
		ProtoSwordPoolTask *task_info = spool_info.add_task_info();
		task_info->set_task_id(pool_task.task_id_);
		task_info->set_total_num(pool_task.total_num_);
		task_info->set_left_num(pool_task.left_num_);
	}

	for (SwordPoolDetail::LastTaskInfoMap::iterator iter = spool.last_task_map_.begin();
			iter != spool.last_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &pool_task = iter->second;
		ProtoSwordPoolTask *find_task = spool_info.add_find_task();
		find_task->set_task_id(pool_task.task_id_);
		find_task->set_total_num(pool_task.total_num_);
		find_task->set_left_num(pool_task.left_num_);
	}

	return this->send_to_other_logic_thread(scene_id, spool_info);
}

int MLSwordPool::read_transfer_spool(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400144 *, request, -1);

	SwordPoolDetail& spool = this->spool_detail_;
	this->set_spool_level(request->level());
	spool.exp_ = request->exp();
	spool.open_ = request->open();
	spool.stype_lv_ = request->style_lvl();
	spool.refresh_tick_ = Time_Value(request->refresh_tick());

	for (int i = 0; i < request->task_info_size(); ++i)
	{
		const ProtoSwordPoolTask &task_info = request->task_info(i);
		SwordPoolDetail::PoolTaskInfo &pool_task = this->spool_detail_.today_task_map_[task_info.task_id()];
		pool_task.task_id_ = task_info.task_id();
		pool_task.total_num_ = task_info.total_num();
		pool_task.left_num_ = task_info.left_num();
	}

	for (int i = 0; i < request->find_task_size(); ++i)
	{
		const ProtoSwordPoolTask &find_task = request->find_task(i);
		SwordPoolDetail::PoolTaskInfo &pool_task = this->spool_detail_.last_task_map_[find_task.task_id()];
		pool_task.task_id_ = find_task.task_id();
		pool_task.total_num_ = find_task.total_num();
		pool_task.left_num_ = find_task.left_num();
	}

	return 0;
}

int MLSwordPool::update_task_info(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31402901 *, request, msg, -1);

	int task_id = request->task_id();
	JUDGE_RETURN(this->is_sword_pool_task(task_id) == true, 0);

	this->check_spool_day_reset();

	SwordPoolDetail& spool = this->spool_detail_;
	SwordPoolDetail::PoolTaskInfo &pool_task = spool.today_task_map_[task_id];
	pool_task.task_id_ = task_id;

	int left_add_flag = request->left_add_flag();
	if (left_add_flag == 1)
	{
		int add_exp_num = 0;
		if (pool_task.left_num_ >= request->left_add_num())
		{
			pool_task.left_num_ -= request->left_add_num();
			add_exp_num = request->left_add_num();
		}
		else
		{
			add_exp_num = pool_task.left_num_;
			pool_task.left_num_ = 0;
		}

//		if (spool.open_ == true)
		this->update_sword_pool(task_id, add_exp_num);
	}
	else if (left_add_flag == 2)
	{
		pool_task.left_num_ += request->left_add_num();
		pool_task.total_num_ += request->left_add_num();
	}
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

	// 向客户端发送最新的剑池数据
	this->get_spool_info();

	return 0;
}

void MLSwordPool::update_sword_pool(int task_id, int num)
{
	const Json::Value &task = CONFIG_INSTANCE->sword_pool_task(task_id);
	int once_exp = task["once_exp"].asInt();
	int add_exp = once_exp * num;

	this->add_sword_pool_exp(add_exp, task_id);
}

void MLSwordPool::add_sword_pool_exp(int add_exp, int task_id)
{
	SwordPoolDetail& spool = this->spool_detail_;
//	JUDGE_RETURN(spool.open_ == true, ;);

	spool.exp_ += add_exp;

	//剑池灵气流水
	this->record_other_serial(SWORD_POOL_EXP_ADD_SERIAL, spool.exp_, add_exp, task_id);

	MapLogicPlayer* player = this->map_logic_player();
	const Json::Value &next_level_info = CONFIG_INSTANCE->sword_pool_set_up(spool.level_+1);
	if (next_level_info == Json::Value::null)
	{
		//取消红点
		player->update_player_assist_single_event(GameEnum::PA_EVENT_SWORD_POOL_LEVEL_UP, 0);
	}
	else
	{
		const Json::Value &set_up_info = CONFIG_INSTANCE->sword_pool_set_up(spool.level_);
		int upgrade_exp = set_up_info["upgrade_exp"].asInt();
		if (spool.exp_ >= upgrade_exp)
		{
			//发送红点
			player->update_player_assist_single_event(GameEnum::PA_EVENT_SWORD_POOL_LEVEL_UP, 1);
		}
		else
		{
			player->update_player_assist_single_event(GameEnum::PA_EVENT_SWORD_POOL_LEVEL_UP, 0);
		}
	}

	JUDGE_RETURN(add_exp > 0 && spool.open_ == true, ;);

	Proto81400651 active_res;
	active_res.set_add_exp(add_exp);
	this->respond_to_client(ACTIVE_ADD_SWORD_POOL_EXP, &active_res);
}

SwordPoolDetail& MLSwordPool::sword_pool_detail()
{
	return this->spool_detail_;
}

void MLSwordPool::check_spool_day_reset()
{
	SwordPoolDetail& spool = this->spool_detail_;
	Time_Value nowtime = Time_Value::gettimeofday();
	if (spool.refresh_tick_ <= nowtime)
	{
		const Json::Value &task_info = CONFIG_INSTANCE->sword_pool_total_task();
		for (uint i = 1; i <= task_info.size(); ++i)
		{
			this->task_reset((int)i);
		}
		spool.refresh_tick_ = next_day(0, 0, nowtime);
		this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

		this->get_spool_info();
	}
}

void MLSwordPool::test_spool_day_reset()
{
	const Json::Value &task_info = CONFIG_INSTANCE->sword_pool_total_task();
	for (uint i = 1; i <= task_info.size(); ++i)
	{
		this->task_reset((int)i);
	}
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);
}

void MLSwordPool::test_find_task()
{
	SwordPoolDetail& spool = this->spool_detail_;
	const Json::Value &task_info = CONFIG_INSTANCE->sword_pool_total_task();
	for (uint i = 1; i <= task_info.size(); ++i)
	{
		int task_id = (int)i;
		SwordPoolDetail::PoolTaskInfo &last_pool_task = spool.last_task_map_[task_id];
		last_pool_task.task_id_ = task_id;

		const Json::Value &task = CONFIG_INSTANCE->sword_pool_task(task_id);
		int limit = task["limit"].asInt();
		if (limit > 0)
		{
			last_pool_task.total_num_ = limit;
			last_pool_task.left_num_ = limit;
		}
		else
		{
			int total_num = 0;
			switch (task_id)
			{
			case GameEnum::SPOOL_TASK_LEGEND:
			{
				total_num = CONFIG_INSTANCE->arena("challenge_times").asInt();
				break;
			}
			case GameEnum::SPOOL_TASK_ESCORT:
			{
				total_num = CONFIG_INSTANCE->convoy_json()["escort_times"].asInt();
				break;
			}
			case GameEnum::SPOOL_TASK_ADVANCE_SCRIPT:
			{
				total_num = this->cal_task_total_num(GameEnum::SCRIPT_T_ADVANCE);
				break;
			}
			case GameEnum::SPOOL_TASK_STORY_SCRIPT:
			{
				total_num = this->cal_task_total_num(GameEnum::SCRIPT_T_STORY);
				break;
			}
			default:
				break;
			}
			last_pool_task.total_num_ = total_num;
			last_pool_task.left_num_ = total_num;
		}
	}
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

}

void MLSwordPool::set_spool_level(int level)
{
	this->spool_detail_.level_ = level;

	MapLogicPlayer* player = this->map_logic_player();
	MountDetail &detail_mount = player->mount_detail(GameEnum::FUN_MOUNT);
	detail_mount.sword_pool_level_ = this->spool_detail_.level_;

	MountDetail &detail_wing = player->mount_detail(GameEnum::FUN_XIAN_WING);
	detail_wing.sword_pool_level_ = this->spool_detail_.level_;
}

void MLSwordPool::spool_handle_player_levelup()
{
	this->check_level_open_spool();
}

void MLSwordPool::spool_handle_player_task(int task_id)
{
	this->check_task_open_spool(task_id);
}

void MLSwordPool::spool_update_mount_info()
{
	//战骑，仙羽加成
	MapLogicPlayer* player = this->map_logic_player();
	player->refresh_notify_mount_info(GameEnum::FUN_MOUNT);
	player->refresh_notify_mount_info(GameEnum::FUN_XIAN_WING);
}

void MLSwordPool::check_level_open_spool()
{
	static std::string fun_str = "fun_jianchi";

	SwordPoolDetail& spool = this->spool_detail_;
	JUDGE_RETURN(spool.open_ == false, ;);

	int cur_level = this->role_level();
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_level(
			fun_str, cur_level) == true, ;);

	spool.open_ = true;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

	// 改变人物属性
	this->refresh_spool_attr_add();
	this->get_spool_info();
}

void MLSwordPool::check_task_open_spool(int task_id)
{
	static std::string fun_str = "fun_jianchi";

	SwordPoolDetail& spool = this->spool_detail_;
	JUDGE_RETURN(spool.open_ == false, ;);
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_task(fun_str, task_id) == true, ;);

	spool.open_ = true;
	this->map_logic_player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SWORD_POOL, true);

	// 改变人物属性
	this->refresh_spool_attr_add();
	this->get_spool_info();
}

void MLSwordPool::uplevel_change_task()
{
	SwordPoolDetail& spool = this->spool_detail_;
	for (SwordPoolDetail::TodayTaskInfoMap::iterator iter = spool.today_task_map_.begin();
			iter != spool.today_task_map_.end(); ++iter)
	{
		SwordPoolDetail::PoolTaskInfo &pool_task = iter->second;
		JUDGE_CONTINUE(this->is_sword_pool_task(pool_task.task_id_) == true);

		int total_num = 0;
		switch(pool_task.task_id_)
		{
		case GameEnum::SPOOL_TASK_ADVANCE_SCRIPT:
		{
			total_num = this->cal_task_total_num(GameEnum::SCRIPT_T_ADVANCE);
			break;
		}
		case GameEnum::SPOOL_TASK_STORY_SCRIPT:
		{
			total_num = this->cal_task_total_num(GameEnum::SCRIPT_T_STORY);
			break;
		}
		default:
			break;
		}
		int add_num = total_num - pool_task.total_num_;
		JUDGE_CONTINUE(add_num > 0);

		pool_task.total_num_ = total_num;
		pool_task.left_num_ += add_num;
	}
}

void MLSwordPool::task_reset(int task_id)
{
	JUDGE_RETURN(this->is_sword_pool_task(task_id) == true, ;);

	SwordPoolDetail& spool = this->spool_detail_;
	SwordPoolDetail::PoolTaskInfo &today_pool_task = spool.today_task_map_[task_id];
	today_pool_task.task_id_ = task_id;

	if (spool.open_ == true)
	{
		SwordPoolDetail::PoolTaskInfo &last_pool_task = spool.last_task_map_[task_id];
		last_pool_task.task_id_ = task_id;
		last_pool_task.total_num_ = today_pool_task.total_num_;
		last_pool_task.left_num_ = today_pool_task.left_num_;
	}

	const Json::Value &task = CONFIG_INSTANCE->sword_pool_task(task_id);
	int limit = task["limit"].asInt();
	if (limit > 0)
	{
		today_pool_task.total_num_ = limit;
		today_pool_task.left_num_ = limit;
	}
	else
	{
		int total_num = 0;
		switch(task_id)
		{
		case GameEnum::SPOOL_TASK_LEGEND:
		{
			total_num = CONFIG_INSTANCE->arena("challenge_times").asInt();
			break;
		}
		case GameEnum::SPOOL_TASK_ESCORT:
		{
			total_num = CONFIG_INSTANCE->convoy_json()["escort_times"].asInt();
			break;
		}
		case GameEnum::SPOOL_TASK_ADVANCE_SCRIPT:
		{
			total_num = this->cal_task_total_num(GameEnum::SCRIPT_T_ADVANCE);
			break;
		}
		case GameEnum::SPOOL_TASK_STORY_SCRIPT:
		{
			total_num = this->cal_task_total_num(GameEnum::SCRIPT_T_STORY);
			break;
		}
		default:
			break;
		}
		today_pool_task.total_num_ = total_num;
		today_pool_task.left_num_ = total_num;
	}
}

int MLSwordPool::cal_task_total_num(int script_type)
{
	int total_num = 0;
	const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
	for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
			iter != script_map.end(); ++iter)
	{
		const Json::Value &script_json = *(iter->second);
		int type = script_json["type"].asInt();
		JUDGE_CONTINUE(type == script_type);

		int open_level = script_json["prev_condition"]["level"].asInt();
		if (this->role_level() >= open_level)
			total_num += 1;
	}
	return total_num;
}

int MLSwordPool::refresh_spool_attr_add(const int enter_type)
{
	SwordPoolDetail& spool = this->spool_detail_;
	JUDGE_RETURN(spool.open_ == true, -1);

	int level = spool.level_;
	const Json::Value &set_up_info = CONFIG_INSTANCE->sword_pool_set_up(level);
	JUDGE_RETURN(set_up_info != Json::Value::null, 0);

	//人物属性
	IntMap prop_map;
	prop_map[GameEnum::ATTACK] = set_up_info["attack"].asInt();
	prop_map[GameEnum::DEFENSE] = set_up_info["defence"].asInt();
	prop_map[GameEnum::BLOOD_MAX] = set_up_info["health"].asInt();

	if (prop_map.size() > 0)
	{
		this->refresh_fight_property(BasicElement::SWORD_POOL, prop_map, enter_type);
	}

	return 0;
}

bool MLSwordPool::is_sword_pool_task(int task_id)
{
	if (CONFIG_INSTANCE->sword_pool_task(task_id) == Json::Value::null)
		return false;
	return true;
}

void MLSwordPool::notify_spool_style_lvl(int notify)
{
	SwordPoolDetail& spool = this->spool_detail_;

	Proto30400408 inner;
	inner.set_level(spool.level_);
	inner.set_exp(spool.exp_);
	inner.set_open(spool.open_);
	inner.set_style_lvl(spool.stype_lv_);
	inner.set_notify_flag(notify);

	this->send_to_map_thread(inner);
}


