/*
 * TrvlArenaMonitor.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: peizhibi
 */

#include "TrvlArenaMonitor.h"
#include "GameCommon.h"
#include "TrvlArenaScene.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "MMOTravel.h"
#include "TArenaPrepScene.h"
#include "MapPlayerEx.h"

TrvlArenaRole::TrvlArenaRole()
{
	TrvlArenaRole::reset();
}

const Json::Value& TrvlArenaRole::conf()
{
	return CONFIG_INSTANCE->tarena_stage(this->index());
}

void TrvlArenaRole::reset()
{
	BaseServerInfo::reset();
	BaseMember::reset();

	this->sid_   = -1;
	this->rank_	 = 0;
	this->sync_	 = 0;
	this->init_	 = 0;
	this->score_ = 0;
	this->state_ = 0;
	this->stage_ = 1;

	this->win_times_	= 0;
	this->last_rival_	= 0;
	this->space_id_		= 0;
	this->pos_index_	= 0;
	this->start_tick_	= 0;
	this->update_tick_ 	= 0;

	this->con_lose_times_	= 0;
}

void TrvlArenaRole::reset_everyday()
{
	this->init_ = 0;
	this->sync_ = 0;
	this->state_ = 0;
	this->win_times_ = 0;
	this->last_rival_ = 0;
	this->con_lose_times_ = 0;
}

void TrvlArenaRole::serialize_rank(ProtoLMRole* proto)
{
	proto->set_rank_index(this->rank_);
	proto->set_role_id(this->id_);
	proto->set_role_name(this->name_);
	proto->set_role_force(this->stage_);
	proto->set_fight_score(this->score_);
}

int TrvlArenaRole::index()
{
	return this->stage_;
}

int TrvlArenaRole::validate()
{
	return this->score_ > 0;
}

int TrvlArenaMonitor::MonitorTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlArenaMonitor::MonitorTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_ARENA_MONITOR->handle_monitor_timeout();
	return 0;
}

int TrvlArenaMonitor::ArrangeTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlArenaMonitor::ArrangeTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_ARENA_MONITOR->arrange_rival_timeout();
	return 0;
}

int TrvlArenaMonitor::TransferTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlArenaMonitor::TransferTimer::handle_timeout(const Time_Value &tv)
{
	TARENA_PREP_SCENE->notify_all_player_exit();
	return this->cancel_timer();
}

TrvlArenaMonitor::TrvlArenaMonitor()
{
	// TODO Auto-generated constructor stub
	this->scene_package_ 	= new PoolPackage<TrvlArenaScene>;
	this->role_package_ 	= new PoolPackage<TrvlArenaRole, Int64>;

	this->transfer_time_ 	= 0;
	this->arrange_time_ 	= 0;
	this->left_prep_time_ 	= 0;

	this->monitor_init_ = 0;
	this->activity_id_	= 0;
	this->enter_level_ 	= 0;
	this->arena_id_		= 0;
	this->same_rivial_  = 0;
	this->check_tick_   = 0;

	this->TOTAL_ARRANGE_TIME = 0;
	this->ROBOT_B_LOSE_TIMES = 0;
	this->PREP_WAITING_TIME = 0;
	this->TRANSFER_OUT_TIME = 0;
}

TrvlArenaMonitor::~TrvlArenaMonitor()
{
	// TODO Auto-generated destructor stub
}

const Json::Value& TrvlArenaMonitor::scene_conf()
{
	return CONFIG_INSTANCE->scene(this->scene_id());
}

const Json::Value& TrvlArenaMonitor::scene_set_conf()
{
	return CONFIG_INSTANCE->scene_set(this->scene_id());
}

TrvlArenaRole* TrvlArenaMonitor::find_role(Int64 role)
{
	return this->role_package_->find_object(role);
}

TrvlArenaRole* TrvlArenaMonitor::find_and_pop(Int64 role)
{
	TrvlArenaRole* tarena_role = this->role_package_->find_object(role);
	JUDGE_RETURN(tarena_role == NULL, tarena_role);

	tarena_role = this->role_package_->pop_object();
	tarena_role->id_ = role;

	this->role_package_->bind_object(role, tarena_role);
	return tarena_role;
}

TrvlArenaScene* TrvlArenaMonitor::find_scene(int space_id)
{
	return this->scene_package_->find_object(space_id);
}

int TrvlArenaMonitor::scene_id()
{
	return GameEnum::TRVL_ARENA_SCENE_ID;
}

int TrvlArenaMonitor::prep_scene_id()
{
	return GameEnum::TRVL_ARENA_PREP_SCENE_ID;
}

int TrvlArenaMonitor::left_prep_time()
{
	return this->left_prep_time_;
}

int TrvlArenaMonitor::is_enter_time()
{
	return this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START;
}

int TrvlArenaMonitor::is_fighting_time()
{
	JUDGE_RETURN(this->monitor_init_ == true, false);
	JUDGE_RETURN(this->is_enter_time() == true, false);
	return this->left_prep_time_ <= 0;
}

IntPair TrvlArenaMonitor::fighting_state()
{
	IntPair pair;
	JUDGE_RETURN(this->monitor_init_ == true, pair);

	if (this->is_fighting_time() == true)
	{
		pair.first = 1;
		pair.second = this->monitor_timer_.left_second();
	}
	else
	{
		pair.first = 0;
		pair.second = this->left_prep_time_;
	}

	return pair;
}

void TrvlArenaMonitor::clear_history_db(int force)
{
	JUDGE_RETURN(this->monitor_init_ == true, ;);
	JUDGE_RETURN(this->is_fighting_time() == false, ;);

	if (force == false)
	{
		const Json::Value& scene_conf = this->scene_conf();
		JUDGE_RETURN(scene_conf["noload"].asInt() == 0, );
	}

	MSG_USER("TrvlArenaMonitor clear db %d %d %d...", force,
			this->his_rank_vec_.size(), this->role_package_->size());

	this->his_rank_vec_.clear();
	this->role_package_->clear();

	MMOTravel::clear_all_tarena_role();
}

void TrvlArenaMonitor::recycle_scene(TrvlArenaScene* scene)
{
	this->scene_package_->unbind_and_push(scene->space_id(), scene);
}

void TrvlArenaMonitor::notify_recogn(Int64 role, int recogn)
{
	MapPlayerEx* player = TARENA_PREP_SCENE->find_player(role);
	JUDGE_RETURN(player != NULL, ;);

	player->respond_to_client(recogn);
}

void TrvlArenaMonitor::start()
{
	this->monitor_init_ = true;

	const Json::Value& set_conf = this->scene_set_conf();
	this->enter_level_ = set_conf["enter_level"].asInt();
	this->activity_id_ = set_conf["activity_id"].asInt();

	const Json::Value& scene_conf = this->scene_conf();
	this->ROBOT_B_LOSE_TIMES = scene_conf["max_lose"].asInt();
	this->TOTAL_ARRANGE_TIME = scene_conf["wait_time"].asInt();
	this->PREP_WAITING_TIME	 = scene_conf["prep_waiting_time"].asInt();
	this->TRANSFER_OUT_TIME  = std::max<int>(Time_Value::MINUTE,
			scene_conf["transfer_out_time"].asInt());
	this->same_rivial_ 		 = scene_conf["same_rivial"].asInt();
#ifdef TEST_COMMAND
	this->same_rivial_ = true;
#endif

	for (int i = 0; i < MAX_ROLE; ++i)
	{
		const Json::Value& pos_conf = scene_conf["enter_pos"][i];
		this->enter_pos_[i].set_pixel(pos_conf[0u].asInt(), pos_conf[1u].asInt());
	}

	this->load();
	TARENA_PREP_SCENE->tarena_prep_init();

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->activity_id_);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_, activity_conf);
    this->monitor_timer_.schedule_timer(this->time_info_.refresh_time_);

    MSG_USER("TrvlArenaMonitor %d, %d...", this->his_rank_vec_.size(), this->time_info_.refresh_time_);
    JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

    this->start_event();
}

void TrvlArenaMonitor::stop()
{
	this->save();
}

void TrvlArenaMonitor::test_arena(int id, int last)
{
	this->time_info_.cur_state_ = 2;
	this->time_info_.refresh_time_ = last;
	this->handle_monitor_timeout_i(0);
}

void TrvlArenaMonitor::handle_monitor_timeout()
{
	int state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	this->handle_monitor_timeout_i(state);
}

void TrvlArenaMonitor::handle_monitor_timeout_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
#ifdef TEST_COMMAND
		this->left_prep_time_ = 30;
#else
		this->left_prep_time_ = this->PREP_WAITING_TIME;
#endif
		this->clear_history_db(true);
		this->start_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_event();
		break;
	}
	}

	this->monitor_timer_.cancel_timer();
	this->monitor_timer_.schedule_timer(this->time_info_.refresh_time_);

	MSG_USER("tarena %d, %d, %d", this->time_info_.time_index_,
			this->time_info_.cur_state_, this->time_info_.refresh_time_);
}

void TrvlArenaMonitor::arrange_rival_timeout()
{
	if (this->left_prep_time_ > 0)
	{
		this->left_prep_time_ -= 1;
	}

	this->arrange_rival();
	this->start_transfer();
	this->sort_history_rank();
}

//安排战斗
void TrvlArenaMonitor::arrange_rival()
{
	JUDGE_RETURN(this->monitor_timer_.left_second() > 10, ;);

	this->arrange_time_ += 1;
	int curr_wait = this->fetch_arrange_wait();
	JUDGE_RETURN(this->arrange_time_ >= curr_wait, ;);

	this->fight_list_.clear();
	this->fight_map_.clear();

	this->arrange_vec_.clear();
	this->arrange_vec_.reserve(this->sign_map_.size());

	//报名列表排序
	for (LongMap::iterator iter = this->sign_map_.begin();
			iter != this->sign_map_.end(); ++iter)
	{
		TrvlArenaRole* role = this->find_role(iter->first);
		JUDGE_CONTINUE(role != NULL);

		FourObj obj;
		obj.id_ 			= iter->first;	//玩家ID
		obj.first_value_ 	= role->stage_;
		obj.second_value_	= role->force_;
		this->arrange_vec_.push_back(obj);
	}

	std::sort(this->arrange_vec_.begin(), this->arrange_vec_.end(),
			GameCommon::four_comp_by_desc);

	int total_count = this->arrange_vec_.size();
	for (int i = 0; i < total_count; ++i)
	{
		JUDGE_CONTINUE(this->arrange_vec_[i].tick_ == false);

		TrvlArenaRole* first = this->find_role(this->arrange_vec_[i].id_);
		JUDGE_CONTINUE(first != NULL);

		//0:不合法, 1:找到对手，2：机器人A, 3：机器人B
		IntPair pair = this->find_rival(i);
		JUDGE_CONTINUE(pair.first > 0);

		FourObj obj;
		obj.id_ = pair.first;	//对手类型
		obj.first_value_ = i;
		obj.second_value_= pair.second;

		if (pair.first == TrvlArenaMonitor::RIVAL_ROLE)
		{
			TrvlArenaRole* second = this->find_role(this->arrange_vec_[pair.second].id_);
			obj.first_id_  = first->id_;
			obj.second_id_ = second->id_;

			first->state_ = TrvlArenaRole::STATE_ATTEND;
			second->state_ = TrvlArenaRole::STATE_ATTEND;

			this->arrange_vec_[pair.second].tick_ = true;
			this->arrange_vec_[i].tick_ = true;
		}
		else
		{
			obj.first_id_ = first->id_;
			first->state_ = TrvlArenaRole::STATE_ATTEND;
			this->arrange_vec_[i].tick_ = true;
		}

		this->fight_list_.push_back(obj);
	}

	for (FourObjList::iterator iter = this->fight_list_.begin();
			iter != this->fight_list_.end(); ++iter)
	{
		this->sign_map_.erase(iter->first_id_);
		this->sign_map_.erase(iter->second_id_);
		this->arrange_arena(*iter);
	}

	this->arrange_time_	= 0;
	this->transfer_time_= 5;
}

void TrvlArenaMonitor::start_transfer()
{
	this->transfer_time_ -= 1;
	JUDGE_RETURN(this->transfer_time_ == 0, ;);

	for (LongMap::iterator iter = this->fight_map_.begin();
			iter != this->fight_map_.end(); ++iter)
	{
		TrvlArenaRole* trvl_role = this->find_role(iter->first);
		JUDGE_CONTINUE(trvl_role != NULL);

		MapPlayerEx* player = TARENA_PREP_SCENE->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		MoverCoord& coord = this->enter_pos_[trvl_role->pos_index_];
		player->transfer_dispatcher(this->scene_id(), coord,
				SCENE_MODE_BATTLE_GROUND, trvl_role->space_id_);
	}

	this->fight_map_.clear();
}

//预告
void TrvlArenaMonitor::ahead_event()
{
	GameCommon::map_sync_activity_tips_ahead(PairObj(this->activity_id_, true),
			this->time_info_.refresh_time_);
	this->clear_history_db(true);
	MSG_USER("tarena ahead");
}

//开始
void TrvlArenaMonitor::start_event()
{
	this->arena_id_ 		= 1;
	this->arrange_time_ 	= 0;
	this->transfer_time_ 	= 0;
	this->check_tick_ 		= 0;
	this->transfer_timer_.cancel_timer();
	this->arrange_timer_.schedule_timer(1);

	BLongMap index_map = this->role_package_->fetch_index_map();
	for (BLongMap::iterator iter = index_map.begin();
			iter != index_map.end(); ++iter)
	{
		TrvlArenaRole* role = this->find_role(iter->first);
		JUDGE_CONTINUE(role != NULL);
		role->reset_everyday();
	}

	GameCommon::map_sync_activity_tips_start(PairObj(this->activity_id_, true),
			this->time_info_.refresh_time_);

	MSG_USER("tarena start");
}

//结束
void TrvlArenaMonitor::stop_event()
{
	GameCommon::map_sync_activity_tips_stop(PairObj(this->activity_id_, true));
	TARENA_PREP_SCENE->notify_all_player_msg(ACTIVE_TARENA_ACTIVITY_END);

	this->arrange_timer_.cancel_timer();
	this->transfer_timer_.schedule_timer(this->TRANSFER_OUT_TIME);

	this->sort_history_rank(false);
	this->save();

	MSG_USER("tarena stop %d", this->his_rank_vec_.size());
}

int TrvlArenaMonitor::sign(int sid, Int64 role, Message* msg)
{
	return 0;
}

int TrvlArenaMonitor::unsign(int sid, Int64 role)
{
	this->sign_map_.erase(role);
	return 0;
}

int TrvlArenaMonitor::operate(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400505*, request, -1);

	switch(request->type())
	{
	case 0:
	{
		return this->fetch_info(sid, msg);
	}
	case 1:
	{
		return this->fetch_rank(sid, role, msg);
	}
	case 2:
	{
		return this->unsign(sid, role);
	}
	}

	return 0;
}

int TrvlArenaMonitor::fetch_info(int sid, Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30400505*, request, -1);
	return 0;
}

int TrvlArenaMonitor::fetch_rank(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400505*, request, -1);

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->value(),
			this->his_rank_vec_.size(), TrvlArenaMonitor::PAGE_SIZE);

	Proto50400823 respond;
	respond.set_page(page_info.cur_page_);
	respond.set_total(page_info.total_page_);

	for (int i = page_info.start_index_; i < page_info.total_count_; ++i)
	{
		Int64 role_id = this->his_rank_vec_[i].id_;

		TrvlArenaRole* arena = this->find_role(role_id);
		JUDGE_CONTINUE(arena != NULL);

		ProtoLMRole* proto = respond.add_rank_list();
		arena->serialize_rank(proto);

		++page_info.add_count_;
		JUDGE_BREAK(page_info.add_count_ < TrvlArenaMonitor::PAGE_SIZE);
	}

	TrvlArenaRole* self = this->find_role(role);
	if (self != NULL)
	{
		ProtoLMRole* proto = respond.mutable_self();
		self->serialize_rank(proto);
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

int TrvlArenaMonitor::fetch_arrange_wait()
{
#ifdef LOCAL_DEBUG
	return 10;
#else
	if (this->sign_map_.size() > 10)
	{
		return TrvlArenaMonitor::TOTAL_ARRANGE_TIME - 10;
	}
	else
	{
		return TrvlArenaMonitor::TOTAL_ARRANGE_TIME;
	}
#endif
}

int TrvlArenaMonitor::send_offline_score(TrvlArenaRole* role)
{
	JUDGE_RETURN(role != NULL, -1);

	return 0;
}

int TrvlArenaMonitor::register_tarena_role(BattleGroundActor* player)
{
	TrvlArenaRole* tarena_role = this->find_role(player->role_id());
	JUDGE_RETURN(tarena_role != NULL, -1);

	if (tarena_role->init_ == false)
	{
		MapRoleDetail& role_detail = player->role_detail();
		tarena_role->BaseMember::unserialize(role_detail);
		tarena_role->BaseServerInfo::unserialize(role_detail);

	    tarena_role->init_ = true;
        MSG_USER("TrvlArenaMonitor %s %d %d", tarena_role->name_.c_str(),
        		tarena_role->sex_, tarena_role->force_);
	}

    if (tarena_role->sync_ == false)
    {
    	TArenaDetail& tarena_detail = player->tarena_detail();
    	tarena_role->stage_ = tarena_detail.stage_;
    	tarena_role->score_ = tarena_detail.score_;

      	tarena_role->sync_ = true;
    	MSG_USER("TrvlArenaMonitor %s %d %d", tarena_role->name_.c_str(),
    			tarena_role->stage_, tarena_role->score_);
    }

	this->sign_map_[player->role_id()] = ::time(NULL);
	return 0;
}

int TrvlArenaMonitor::request_enter_tarena(int sid, Proto30400051* request)
{
	string main_version = request->main_version();
	JUDGE_RETURN(CONFIG_INSTANCE->is_same_main_version(main_version) == true,
			ERROR_TRVL_SAME_VERSION);

	JUDGE_RETURN(this->is_enter_time() == true, ERROR_GUARD_TIME);
	JUDGE_RETURN(request->role_level() >= this->enter_level_, ERROR_PLAYER_LEVEL);

	Proto30400052 enter_info;
	enter_info.set_space_id(0);
	enter_info.set_scene_mode(SCENE_MODE_BATTLE_GROUND);

	MoverCoord enter_point = TARENA_PREP_SCENE->fetch_enter_pos();
	enter_info.set_pos_x(enter_point.pixel_x());
	enter_info.set_pos_y(enter_point.pixel_y());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

//查找对手
IntPair TrvlArenaMonitor::find_rival(int start_index)
{
	IntPair pair;

	TrvlArenaRole* first = this->find_role(this->arrange_vec_[start_index].id_);
	JUDGE_RETURN(first != NULL, pair);

//	if (first->con_lose_times_ >= TrvlArenaMonitor::ROBOT_B_LOSE_TIMES)
//	{
//		pair.first = TrvlArenaMonitor::RIVAL_ROBOT_B;
//		return pair;
//	}

	int total_size = this->arrange_vec_.size();
	for (int i = start_index  + 1; i < total_size; ++i)
	{
		TrvlArenaRole* second = this->find_role(this->arrange_vec_[i].id_);
		JUDGE_CONTINUE(second != NULL);
		JUDGE_CONTINUE(this->same_rivial_ == true || first->last_rival_ != second->id_);

		pair.first = TrvlArenaMonitor::RIVAL_ROLE;
		pair.second = i;

		return pair;
	}

//	pair.first = TrvlArenaMonitor::RIVAL_ROBOT_A;
	return pair;
}

//加载
void TrvlArenaMonitor::load()
{
	const Json::Value& scene_conf = this->scene_conf();
	JUDGE_RETURN(scene_conf["noload"].asInt() == 0, );

	MMOTravel::load_tarena_role(this);
	this->sort_history_rank(false);
}

//保存
void TrvlArenaMonitor::save()
{
	BLongMap index_map = this->role_package_->fetch_index_map();
	for (BLongMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TrvlArenaRole* role = this->find_role(iter->first);
		JUDGE_CONTINUE(role != NULL && role->validate() == true);
		MMOTravel::save_tarena_role(role);
	}
}

//安排房间比赛
void TrvlArenaMonitor::arrange_arena(FourObj& obj)
{
	int space_id = this->arena_id_++;

	TrvlArenaScene* scene = this->scene_package_->pop_object();
	scene->init_arena_scene(space_id, obj);
	this->scene_package_->bind_object(space_id, scene);

	FourObj& first = this->arrange_vec_[obj.first_value_];
	this->notify_recogn(first.id_, ACTIVE_TARENA_FIND_RIVAL);

	TrvlArenaRole* first_role = this->find_role(first.id_);
	first_role->space_id_ 	= space_id;
	first_role->start_tick_ = ::time(NULL);
	first_role->pos_index_ 	= 0;
	first_role->last_rival_ = 0;

	this->fight_map_[first.id_] = true;
	JUDGE_RETURN(obj.id_ == TrvlArenaMonitor::RIVAL_ROLE, ;)

	FourObj& second = this->arrange_vec_[obj.second_value_];
	this->notify_recogn(second.id_, ACTIVE_TARENA_FIND_RIVAL);

	TrvlArenaRole* second_role = this->find_role(second.id_);
	second_role->start_tick_= ::time(NULL);
	second_role->last_rival_= first.id_;
	second_role->space_id_	= space_id;
	second_role->pos_index_ = 1;

	first_role->last_rival_ = second.id_;
	this->fight_map_[second.id_] = true;
}

void TrvlArenaMonitor::sort_history_rank(int check)
{
	if (check == true)
	{
		this->check_tick_ += 1;
		JUDGE_RETURN(this->check_tick_ >= 5, ;);

		this->check_tick_ = 0;
	}

	BLongMap index_map = this->role_package_->fetch_index_map();

	this->his_rank_vec_.clear();
	this->his_rank_vec_.reserve(index_map.size());

	for (BLongMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TrvlArenaRole* role = this->find_role(iter->first);
		JUDGE_CONTINUE(role != NULL && role->validate() == true);

		ThreeObj obj;
		obj.id_ = role->id_;
		obj.value_ = role->score_;
		obj.tick_ = role->update_tick_;
		this->his_rank_vec_.push_back(obj);
	}

	int rank = 1;
	std::sort(this->his_rank_vec_.begin(), this->his_rank_vec_.end(),
			GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = this->his_rank_vec_.begin();
			iter != this->his_rank_vec_.end(); ++iter)
	{
		TrvlArenaRole* role = this->find_role(iter->id_);
		JUDGE_CONTINUE(role != NULL);

		role->rank_ = rank;
		++rank;
	}
}
