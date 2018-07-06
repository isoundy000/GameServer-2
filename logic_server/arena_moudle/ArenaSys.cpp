/*
 * ArenaSys.cpp
 *
 *  Created on: Aug 15, 2014
 *      Author: peizhibi
 */

#include "ArenaSys.h"
#include "MMOLeague.h"

#include "ProtoDefine.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "SerialRecord.h"

int ArenaSys::FightingTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int ArenaSys::FightingTimer::handle_timeout(const Time_Value &tv)
{
	this->arena_sys_->fighting_handle_timeout();
	return 0;
}

ArenaSys::ArenaSys(void) :
		open_flag_(true), cool_time_(0),
		need_level_(0), area_index_(0), max_finish_time_(0)
{
	// TODO Auto-generated constructor stub
	this->fight_node_pool_ = new ObjectPoolEx<AreaFightNode>;
	this->fighting_timer_.arena_sys_ = this;
}

ArenaSys::~ArenaSys()
{
	// TODO Auto-generated destructor stub
	delete this->fight_node_pool_;
}

void ArenaSys::init()
{
	MMOLeague::load_arena(&this->area_detail_);

	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_base();
	this->area_index_ = 1;
	this->cool_time_ = rank_json["left_time"].asInt() * 60;

	this->need_level_ = CONFIG_INSTANCE->athletics_base()["need_level"].asInt() - 1;
	this->max_finish_time_ = CONFIG_INSTANCE->arena("fight_time").asInt()
			+ AreaSysDetail::WAIT_TIME;

	MSG_USER("AREA_SYS %d %d %d %d", this->area_detail_.arena_role_set_.size(), this->area_detail_.rank_robot_map.size(),
			this->area_detail_.robot_id_map.size(), this->area_detail_.role_map_.size());
	LongMap check_map;
	int length = (int)this->area_detail_.role_map_.size();
	for (int i = 1; i <= ::std::max(2000, length); i++)
	{
		Int64 rpm_id = CONFIG_INSTANCE->athletics_rank(i)["rpm_id"].asInt();

		if (check_map.count(rpm_id) == 0)
			check_map[rpm_id] = 1;
		else
			check_map[rpm_id]++;


		check_map[rpm_id] = check_map[rpm_id] % GameEnum::ARENA_BASE_RANK_NUM;
		rpm_id += check_map[rpm_id] * GameEnum::ARENA_BASE_ID_NUM;

		Int64 robot_id = this->area_detail_.robot_id_map[rpm_id];
		this->area_detail_.last_bot_id = robot_id;

		ArenaRole& arena_role = this->area_detail_.arena_role_set_[robot_id];
		if (this->area_detail_.rank_set_[i] == 0)
		{
			this->area_detail_.rank_set_[i] = arena_role.id_;
			arena_role.rank_ = i;
		}

		if (this->area_detail_.role_map_.count(arena_role.id_) == 0)
		{
			this->area_detail_.role_map_[arena_role.id_] = arena_role;
		}
	}

	//bug玩家处理
	for (LongMap::iterator iter = this->area_detail_.same_player_map_.begin();
			iter != this->area_detail_.same_player_map_.end(); ++iter)
	{
		int rank_size = this->area_detail_.rank_set_.size() + 1;
		this->area_detail_.rank_set_[rank_size] = iter->first;
		ArenaRole& arena_role = this->area_detail_.role_map_[iter->first];
		arena_role.rank_ = rank_size;
	}
	MSG_USER("same_player_map: %d", this->area_detail_.same_player_map_.size());

	this->area_detail_.same_player_map_.clear();

	MSG_USER("AREA_SYS %d %d %d %d %d", this->area_detail_.arena_role_set_.size(), this->area_detail_.rank_robot_map.size(),
			this->area_detail_.robot_id_map.size(), this->area_detail_.role_map_.size(), this->area_detail_.rank_set_.size());

	this->reward_and_set_next_timeout(false);

	Time_Value schedule_time(1);
	this->fighting_timer_.schedule_timer(schedule_time);
}

void ArenaSys::fina()
{
	MMOLeague::save_arena(&this->area_detail_, true);
}

Int64 ArenaSys::last_robot_id(int rank)
{
	Int64 rpm_id = CONFIG_INSTANCE->athletics_rank(rank)["rpm_id"].asInt();
	rpm_id += (::std::rand() % GameEnum::ARENA_BASE_RANK_NUM) * GameEnum::ARENA_BASE_ID_NUM;

	Int64 robot_id = this->fetch_detail()->robot_id_map[rpm_id];

	return robot_id;
}

int ArenaSys::reward_and_set_next_timeout(int serial)
{
	Int64 cur_tick = ::time(NULL);
	JUDGE_RETURN(cur_tick >= this->area_detail_.timeout_tick_, -1);

	bool is_new_server = false;
	if (this->area_detail_.timeout_tick_ <= 0)
		is_new_server = true;

	int total_days = CONFIG_INSTANCE->arena("settl_days").asInt();
	int left_cur_day_tick = Time_Value::DAY - GameCommon::fetch_cur_day_sec();

	int left_next_tick = (total_days - 1) * Time_Value::DAY + left_cur_day_tick;
	this->area_detail_.timeout_tick_ = cur_tick + left_next_tick;

	if (is_new_server == false && serial == true)
	{
		for (LongMap::iterator it = this->area_detail_.rank_set_.begin();
				it != this->area_detail_.rank_set_.end(); ++it)
		{
			Int64 role_id = it->second;
			this->set_arena_role_reward(role_id);
		}
	}

	return 0;
}

int ArenaSys::fetch_next_timeout()
{
	return GameCommon::left_time(this->area_detail_.timeout_tick_);
}

int ArenaSys::fetch_buy_money(ArenaRole* area_role)
{
	JUDGE_RETURN(area_role != NULL, 0);

	Int64 buy_time = area_role->buy_times_;
//	JUDGE_RETURN(buy_time > 0, -1);

	int max_money = 1;
	for (int i = GameEnum::ARENA_RANK_END; i >= GameEnum::ARENA_RANK_START; --i)
	{
		const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank_by_id(i);
		if (rank_json.isMember("buy_time") == false) continue;
		max_money = rank_json["buy_money"].asInt();
		if (buy_time >= rank_json["buy_time"].asInt())
			return max_money;
	}
	return max_money;
}


void ArenaSys::save_arena_data()
{
	MMOLeague::save_arena(&this->area_detail_, false);
}

void ArenaSys::fighting_handle_timeout()
{
	Int64 now_tick = ::time(NULL);
	while (this->area_detail_.fight_heap_.size() > 0)
	{
		AreaFightNode* fight_node = this->area_detail_.fight_heap_.top();
		JUDGE_BREAK(fight_node->max_finish_tick_ <= now_tick);

		this->challenge_timeout(fight_node);

		this->area_detail_.fight_heap_.pop();
		this->area_detail_.fight_map_.erase(fight_node->area_index_);
	}
}

void ArenaSys::midnight_handle_timeout()
{
	this->save_arena_data();
	this->reward_and_set_next_timeout(true);
}

int ArenaSys::calc_challenge_result(ArenaRole* rank_role, ArenaRole* player_role)
{
	int force_num = player_role->force_ - rank_role->force_;
	const Json::Value &athletics_base_json = CONFIG_INSTANCE->athletics_base();
	int low_force_num = athletics_base_json["force_win_list"][0u].asInt() - 100;
	int high_force_num = (int)athletics_base_json["force_win_list"][1u].asInt() - 100;
	if (force_num > 0)
	{
		int result_player = player_role->force_ * (100 + (std::rand() % high_force_num)) / 100;
		int result_rank = rank_role->force_ * (100 + (std::rand() % low_force_num)) / 100;
		if (result_player >= result_rank) return 1;
		return 0;
	}
	else
	{
		int result_player = player_role->force_ * (100 + (std::rand() % low_force_num)) / 100;
		int result_rank = rank_role->force_ * (100 + (std::rand() % high_force_num)) / 100;
		if (result_player >= result_rank) return 1;
		return 0;
	}
	return 0;
}

void ArenaSys::rand_fighter(string name, ArenaRole* area_role)
{
	int base_rank = area_role->rank_;
	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank(base_rank);
	JUDGE_RETURN(rank_json != Json::Value::null, ;);

	int	upper = 0, lower = 0, rand_num = 0;

	lower = rank_json[name][0u].asInt() + base_rank;
	upper = rank_json[name][1u].asInt() + base_rank;

	rand_num = (std::rand() % (upper - lower + 1)) + lower;
	area_role->fight_set_.push_back(rand_num);
}

void ArenaSys::set_fighter_set(ArenaRole* area_role)
{
	area_role->fight_set_.clear();

	this->rand_fighter("player_a", area_role);
	this->rand_fighter("player_b", area_role);
	this->rand_fighter("player_c", area_role);
}

void ArenaSys::set_first_fighter_set(ArenaRole* area_role)
{
	area_role->fight_set_.clear();

	int num = 0;
	for (int i = int(this->area_detail_.rank_set_.size()); i > 0; --i)
	{
		Int64 role_id = this->area_detail_.rank_set_[i];
		ArenaRole* rank_role = this->arena_rank_role(role_id);
		JUDGE_CONTINUE(rank_role != NULL && rank_role->career_ == 0);

		area_role->fight_set_.push_back(i);
		JUDGE_BREAK((++num) < 3);
	}
}

void ArenaSys::check_and_set_fighter(ArenaRole* area_role)
{
	JUDGE_RETURN(area_role != NULL, ;);
	this->set_fighter_set(area_role);
}

int ArenaSys::start_challenge(ArenaRole* arena_role, ArenaRole* rank_role, int guide_flag, int first)
{
	JUDGE_RETURN(arena_role->id_ != rank_role->id_, -1);
	JUDGE_RETURN(arena_role->is_fighting_ == false, -1);

	if (guide_flag == false)
	{
		JUDGE_RETURN(rank_role->is_fighting_ == false, -1);
	}
	guide_flag = false;

	AreaFightNode* fight_node = this->fight_node_pool_->pop();
	JUDGE_RETURN(fight_node != NULL, -1);

	fight_node->area_index_ = this->area_index_;
	fight_node->guide_flag_ = guide_flag;
	fight_node->first_id_ = arena_role->id_;
	fight_node->second_id_ = rank_role->id_;

	fight_node->start_tick_ = ::time(NULL);
	fight_node->max_finish_tick_ = ::time(NULL) + this->max_finish_time_;

	arena_role->left_times_ -= 1;
	arena_role->is_fighting_ = true;
	rank_role->is_fighting_ = true;

	arena_role->area_index_ = fight_node->area_index_;
	rank_role->area_index_ = fight_node->area_index_;

	this->area_index_ += 1;
	this->area_detail_.fight_heap_.push(fight_node);
	this->area_detail_.fight_map_[fight_node->area_index_] = fight_node;

	arena_role->heap_index_ = fight_node->__heap_index;
	rank_role->heap_index_ = fight_node->__heap_index;

	LogicPlayer* player = NULL;
	if (LOGIC_MONITOR->find_player(arena_role->id_, player) == 0)
	{
		player->update_sword_pool_info(1, 1);
		player->update_cornucopia_task_info();
	}

	if (arena_role->is_skip_)
	{
		int ret = this->calc_challenge_result(rank_role, arena_role);
		if (ret)
		{
			this->finish_normal_challenge(rank_role->id_, fight_node);
		}
		else
		{
			this->finish_normal_challenge(arena_role->id_, fight_node);
		}

		return 0;
	}

	Proto30400438 create_info;
	create_info.set_first_id(fight_node->first_id_);
	create_info.set_second_id(fight_node->second_id_);
	create_info.set_area_index(fight_node->area_index_);
	create_info.set_is_first(first);

	ProtoRoleInfo* first_role = create_info.mutable_first_role();
	first_role->set_role_id(arena_role->id_);
	first_role->set_role_name(arena_role->name_);
	first_role->set_role_sex(arena_role->sex_);
	first_role->set_role_level(arena_role->level_);
	first_role->set_role_career(arena_role->career_);
	first_role->set_role_force(arena_role->force_);
	first_role->set_role_wing(arena_role->wing_level_);
	first_role->set_role_solider(arena_role->solider_level_);

	ProtoRoleInfo* second_role = create_info.mutable_second_role();
	second_role->set_role_id(rank_role->id_);
	second_role->set_role_name(rank_role->name_);
	second_role->set_role_sex(rank_role->sex_);
	second_role->set_role_level(rank_role->level_);
	second_role->set_role_career(rank_role->career_);
	second_role->set_role_force(rank_role->force_);
	second_role->set_role_wing(rank_role->wing_level_);
	second_role->set_role_solider(rank_role->solider_level_);

	return LOGIC_MONITOR->dispatch_to_scene(GameEnum::ARENA_SCENE_ID, &create_info);
}

int ArenaSys::finish_challenge(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100233*, request, -1);

	AreaFightNode* fight_node = this->area_detail_.fight_map_[request->area_index()];
	JUDGE_RETURN(fight_node != NULL, -1);

//	if (fight_node->guide_flag_ == true)
//	{
//		this->finish_guide_challenge(request->lose_id(), fight_node);
//	}
//	else
//	{
		this->finish_normal_challenge(request->lose_id(), fight_node);
//	}

	this->area_detail_.fight_map_.erase(fight_node->area_index_);
	this->area_detail_.fight_heap_.remove(fight_node->__heap_index);

	return 0;
}

int ArenaSys::finish_guide_challenge(Int64 lose_id, AreaFightNode* fight_node)
{
	Int64 win_id = 0;
	int challenge_win = false;

	if (fight_node->first_id_ == lose_id)
	{
		challenge_win = false;
		win_id = fight_node->second_id_;
	}
	else
	{
		challenge_win = true;
		win_id = fight_node->first_id_;
	}

	ArenaRole* win_role = this->area_role(win_id);
	JUDGE_RETURN(win_role != NULL, -1);

	win_role->heap_index_ = 0;
	win_role->continue_win_ += 1;

	win_role->open_flag_ = true;
	win_role->is_fighting_ = false;

	ArenaRole& lose_role = (*this->area_detail_.arena_role_set_.begin()).second;
	win_role->push_active_record(lose_role.name_, true, true);

	this->notify_challenge_reward(win_role, true, true, 0);
	return 0;
}


int ArenaSys::finish_normal_challenge(Int64 lose_id, AreaFightNode* fight_node)
{
	ArenaRole* lose_role = this->area_role(lose_id);
	JUDGE_RETURN(lose_role != NULL, -1);

	lose_role->heap_index_ = 0;
	lose_role->continue_win_ = 0;
	lose_role->is_fighting_ = false;
	JUDGE_RETURN(fight_node != NULL, -1);

	Int64 win_id = 0;
	int challenge_win = false;
	if (fight_node->first_id_ == lose_id)
	{
		challenge_win = false;
		win_id = fight_node->second_id_;
	}
	else
	{
		challenge_win = true;
		win_id = fight_node->first_id_;
	}

	ArenaRole* win_role = this->area_role(win_id);
	JUDGE_RETURN(win_role != NULL, -1);

	win_role->heap_index_ = 0;
	win_role->continue_win_ += 1;
	win_role->is_fighting_ = false;

	// swap rank and record
	if (challenge_win == true)
	{
		int rank_change = false;
		int rank_differ = win_role->rank_ - lose_role->rank_;
		if (rank_differ > 0)
		{
			std::swap(this->area_detail_.rank_set_[win_role->rank_],
					this->area_detail_.rank_set_[lose_role->rank_]);
			std::swap(win_role->rank_, lose_role->rank_);

			rank_change = true;
			win_role->fight_set_.clear();
		}

		//reward
		this->notify_challenge_reward(win_role, challenge_win, rank_change, rank_differ);

		//record
		win_role->push_active_record(lose_role->name_, true, rank_change);
		lose_role->push_passive_record(win_role->name_, false, rank_change);
	}
	else
	{
		//reward
		this->notify_challenge_reward(lose_role, challenge_win, false, 0);

		//record
		lose_role->push_active_record(win_role->name_, false, false);
		win_role->push_passive_record(lose_role->name_, true, false);
	}

	return 0;
}

int ArenaSys::challenge_timeout(AreaFightNode* fight_node)
{
	ArenaRole* area_role = this->area_role(fight_node->first_id_);
	if (area_role != NULL)
	{
		area_role->is_fighting_ = false;
		area_role->continue_win_ = 0;
		area_role->heap_index_ = 0;
	}

	ArenaRole* rank_role = this->area_role(fight_node->second_id_);
	if (rank_role != NULL)
	{
		rank_role->is_fighting_ = false;
		rank_role->continue_win_ = 0;
		rank_role->heap_index_ = 0;
	}

	return 0;
}

int ArenaSys::validate_register_arena_role(LogicPlayer* player)
{
	JUDGE_RETURN(player->role_level() >= this->need_level_, false);
	return true;
}

int ArenaSys::update_arena_role_shape(Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100228*, request, -1);

	ArenaRole* arena_role = this->area_role(role_id);
	JUDGE_RETURN(arena_role != NULL, -1);

	arena_role->weapon_ = request->weapon();
	arena_role->clothes_ = request->clothes();

	arena_role->fashion_weapon_ = request->fashion_weapon();
	arena_role->fashion_clothes_ = request->fashion_clothes();

	return 0;
}


void ArenaSys::check_and_register_area(LogicPlayer* player)
{
	JUDGE_RETURN(this->validate_register_arena_role(player) == true, ;);

	Int64 role_id = player->role_id();

	LogicRoleDetail& role_detail = player->role_detail();
	int lvl_int = 0;
	{
		int real_level = role_detail.__level;
		const Json::Value& cfg_level_json = CONFIG_INSTANCE->tiny("lvl_exchange");
		const std::string lvl = GameCommon::int2str( real_level );
		int cfg_level_int = 0;
		if( cfg_level_json.isMember(lvl) )
		{
			cfg_level_int = cfg_level_json[lvl].asInt();
		}
		lvl_int = cfg_level_int > 0 ? cfg_level_int : real_level % 1000;
	}

	if (this->area_detail_.role_map_.count(role_id) == 0)
	{
		// new
		ArenaRole& area_role = this->area_detail_.role_map_[role_id];

		area_role.id_ = role_id;
		area_role.name_ = role_detail.name();
		area_role.reward_level_ = lvl_int;
		area_role.level_ = role_detail.__level;
		area_role.sex_  = role_detail.__sex;
		area_role.career_ = role_detail.__career;
		area_role.force_ = role_detail.__fight_force;
		area_role.rank_ = (int)this->area_detail_.rank_set_.size() + 1;

		area_role.fashion_clothes_ = role_detail.fashion_id_;
		area_role.fashion_weapon_ = role_detail.fashion_color_;

		//神兵
		if (role_detail.mount_info_.count(GameEnum::FUN_GOD_SOLIDER) > 0)
		{
			ThreeObj &obj = role_detail.mount_info_[GameEnum::FUN_GOD_SOLIDER];
			area_role.solider_level_ = obj.value_;
		}
		else
		{
			area_role.solider_level_ = 0;
		}

		//仙羽
		if (role_detail.mount_info_.count(GameEnum::FUN_XIAN_WING) > 0)
		{
			ThreeObj &obj = role_detail.mount_info_[GameEnum::FUN_XIAN_WING];
			area_role.wing_level_ = obj.value_;
		}
		else
		{
			area_role.wing_level_ = 0;
		}


		area_role.reset_everyday();
		this->area_detail_.rank_set_[area_role.rank_] = role_id;

		this->set_arena_role_reward(role_id);
		// update shape
		this->check_and_update_shape(player);
		player->check_and_notity_arena_tips();
		player->update_open_activity_area_rank();

	}
	else
	{
		//update
		bool notify = false;
		ArenaRole& area_role = this->area_detail_.role_map_[role_id];
		if(area_role.force_ != role_detail.__fight_force)
			notify = true;

		area_role.sex_ = role_detail.__sex;
		area_role.reward_level_ = lvl_int;
		area_role.level_ = role_detail.__level;
		area_role.force_ = role_detail.__fight_force;
		area_role.name_ = role_detail.__name;
		area_role.career_ = role_detail.__career;

		area_role.fashion_clothes_ = role_detail.fashion_id_;
		area_role.fashion_weapon_ = role_detail.fashion_color_;
		//神兵
		if (role_detail.mount_info_.count(GameEnum::FUN_GOD_SOLIDER) > 0)
		{
			ThreeObj &obj = role_detail.mount_info_[GameEnum::FUN_GOD_SOLIDER];
			area_role.solider_level_ = obj.value_;
		}
		else
		{
			area_role.solider_level_ = 0;
		}

		//仙羽
		if (role_detail.mount_info_.count(GameEnum::FUN_XIAN_WING) > 0)
		{
			ThreeObj &obj = role_detail.mount_info_[GameEnum::FUN_XIAN_WING];
			area_role.wing_level_ = obj.value_;
		}
		else
		{
			area_role.wing_level_ = 0;
		}

	}
}

void ArenaSys::check_and_update_shape(LogicPlayer* player)
{
	JUDGE_RETURN(this->validate_register_arena_role(player) == true, ;);
	LOGIC_MONITOR->dispatch_to_scene(player->gate_sid(), player->role_id(),
			player->scene_id(), INNER_MAP_SYNC_ARENA_SHAPE);
}

void ArenaSys::notify_challenge_reward(ArenaRole* arena_role, int challenge_win,
		int rank_change, int rank_differ)
{

	if (arena_role->next_fight_tick_ > ::time(0))
	{
		arena_role->next_fight_tick_ += this->cool_time_;
	}
	else
	{
		arena_role->is_over_limit_ = 0;
		arena_role->next_fight_tick_ = ::time(0) + this->cool_time_;
	}

	if (arena_role->left_cool_time() >= CONFIG_INSTANCE->athletics_base()["left_time_limit"].asInt() * 60)
		arena_role->is_over_limit_ = 1;

	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank(arena_role->rank_);
	JUDGE_RETURN(rank_json != Json::Value::null, ;);

	int reward_id = 0;
	if (challenge_win != 0 )
	{
		reward_id = rank_json["win_reward_id"].asInt();
	}
	else
	{
		reward_id = rank_json["lose_reward_id"].asInt();
	}


	LogicPlayer* player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(arena_role->id_, player) == 0, ;);
	player->request_add_reward(reward_id, SerialObj(ADD_FROM_ARENA));
	if (arena_role->reward_id_ != 0)
		this->set_arena_role_reward(arena_role->id_);


	player->check_arena_pa_event();
	player->update_open_activity_area_rank();		//江湖榜信息
//	if(player->scene_id() == GameEnum::AREA_SCENE_ID)
//	{
		Proto80400347 reward_info;
		reward_info.set_fight_state(challenge_win);

		reward_info.set_add_exp(arena_role->open_flag_);// 这个字段改为是否第一次；
//		reward_info.set_add_anima(add_anima);

		reward_info.set_add_anima(reward_id);
		reward_info.set_rank(arena_role->rank_);
		reward_info.set_rank_change(rank_change);
		reward_info.set_rank_differ(rank_differ);
		player->respond_to_client(ACTIVE_ARENA_REWARD, &reward_info);

		arena_role->open_flag_ = 1;
//	}
	Proto31401901 request;
	request.set_ach_index(GameEnum::LEGEND_RANK);
	request.set_cur_value(arena_role->rank_);
	LOGIC_MONITOR->dispatch_to_scene(player, &request);
}

int ArenaSys::fetch_clear_money(ArenaRole* area_role)
{
	JUDGE_RETURN(area_role != NULL, 0);

	Int64 left_time = area_role->next_fight_tick_ - ::time(0);
	JUDGE_RETURN(left_time > 0, -1);
	left_time /= 60;

	int max_money = 1;
	for (int i = GameEnum::ARENA_RANK_END; i >= GameEnum::ARENA_RANK_START; --i)
	{
		const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank_by_id(i);
		if (rank_json.isMember("clear_time") == false) continue;
		max_money = rank_json["clear_money"].asInt();
		if (left_time >= rank_json["clear_time"].asInt())
			return max_money;
	}
	return max_money;
}

ArenaRole* ArenaSys::arena_rank_role(int rank)
{
	JUDGE_RETURN(rank < (int)this->area_detail_.rank_set_.size(),
			this->area_role(this->last_robot_id(rank)));

	Int64 role_id = this->area_detail_.rank_set_[rank];
	return this->area_role(role_id);
}

ArenaRole* ArenaSys::area_role(Int64 role_id)
{
	JUDGE_RETURN(this->area_detail_.role_map_.count(role_id) > 0, &this->area_detail_.role_map_[this->area_detail_.last_bot_id]);
	return &this->area_detail_.role_map_[role_id];
}

void ArenaSys::insert_arena_viewer(Int64 role_id)
{
}

void ArenaSys::erase_arena_viewer(Int64 role_id)
{
}

void ArenaSys::set_arena_role_reward(Int64 role_id)
{
	ArenaRole* arena_role = this->area_role(role_id);
	JUDGE_RETURN(arena_role != NULL, ;);

	SERIAL_RECORD->record_rank(role_id, arena_role->name_,
			SERIAL_RANK_TYPE_AREA, arena_role->rank_, ::time(NULL));

	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank(arena_role->rank_);
	JUDGE_RETURN(rank_json != Json::Value::null, ;);

	arena_role->reward_id_ = rank_json["reward_id"].asInt();
}

AreaSysDetail* ArenaSys::fetch_detail()
{
	return &this->area_detail_;
}

void ArenaSys::check_area_need_sort(void)
{
	const Json::Value &athletics_base_json = CONFIG_INSTANCE->athletics_base();

	JUDGE_RETURN(athletics_base_json["area_need_sort"].asInt() == 1, ;);
	//配置有变化

	MSG_USER("AreaSys need sort start   rank_set size : %d  role_map size %d",
			(int)this->area_detail_.rank_set_.size(), (int)this->area_detail_.role_map_.size());
	MMOLeague::sort_arena(&this->area_detail_);

	//重置奖励
	for (LongMap::iterator it = this->area_detail_.rank_set_.begin();
			it != this->area_detail_.rank_set_.end(); ++it)
	{
		Int64 role_id = it->second;
		this->set_arena_role_reward(role_id);
	}
}


