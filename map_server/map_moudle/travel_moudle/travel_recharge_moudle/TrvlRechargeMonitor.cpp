/*
 * TrvlRechargeMonitor.cpp
 *
 *  Created on: 2017年3月7日
 *      Author: lyw
 */

#include "TrvlRechargeMonitor.h"
#include "GameCommon.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "MMOTravel.h"
#include "ActivityStruct.h"
#include "Transaction.h"

TrvlRechargeRank::TrvlRechargeRank()
{
	TrvlRechargeRank::reset();
}

void TrvlRechargeRank::reset()
{
	BaseServerInfo::reset();
	BaseMember::reset();

	this->rank_   = 0;
	this->amount_ = 0;
	this->sid_	  = 0;
	this->tick_   = 0;
	this->activity_id_ = 0;
}

void TrvlRechargeRank::serialize(ProtoRechargeRank *rank_info)
{
	rank_info->set_role_id(this->id_);
	rank_info->set_name(this->name_);
	rank_info->set_rank(this->rank_);
	rank_info->set_amount(this->amount_);
}

int TrvlRechargeMonitor::ResetTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlRechargeMonitor::ResetTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_RECHARGE_MONITOR->handle_send_mail();
	TRVL_RECHARGE_MONITOR->rank_reset_everyday();
    TRVL_RECHARGE_MONITOR->recharge_back_rank_check_end();
	return 0;
}

int TrvlRechargeMonitor::SaveTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlRechargeMonitor::SaveTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_RECHARGE_MONITOR->save_rank_info();
	return 0;
}

int TrvlRechargeMonitor::TenSecTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlRechargeMonitor::TenSecTimer::handle_timeout(const Time_Value &tv)
{
    TRVL_RECHARGE_MONITOR->recharge_back_rank_check_end();
   TRVL_RECHARGE_MONITOR->request_correct_rank_info();
	return 0;
}

TrvlRechargeMonitor::TrvlRechargeMonitor() {
	// TODO Auto-generated constructor stub
	this->recharge_get_package_ = new PoolPackage<TrvlRechargeRank, Int64>;
	this->recharge_cost_package_ = new PoolPackage<TrvlRechargeRank, Int64>;
	this->recharge_back_package_ = new PoolPackage<TrvlRechargeRank, Int64>;
	this->recharge_back_start_tick_ = Time_Value::zero;
	this->recharge_back_end_tick_ = Time_Value::zero;
}

TrvlRechargeMonitor::~TrvlRechargeMonitor() {
	// TODO Auto-generated destructor stub
}

void TrvlRechargeMonitor::start()
{
	this->load_rank_info();
	this->reset_timer_.schedule_timer(GameCommon::next_day());
	this->save_timer_.schedule_timer(Time_Value::HOUR);
    this->tensec_timer_.schedule_timer(10);

	MSG_USER("TrvlRechargeMonitor start!");
}

void TrvlRechargeMonitor::stop()
{
	this->save_rank_info();
	this->reset_timer_.cancel_timer();
	this->save_timer_.cancel_timer();
    this->tensec_timer_.cancel_timer();
}

int TrvlRechargeMonitor::add_recharge_rank_info(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400511*, request, -1);

	MSG_USER("sid: %d, Proto30400511: %s", sid, request->Utf8DebugString().c_str());

	int type = request->type();
	TrvlRechargeRank *rank_info = this->find_and_pop(role, type);
	JUDGE_RETURN(rank_info != NULL, 0);

	rank_info->BaseMember::unserialize(request->self_info());
	rank_info->BaseServerInfo::unserialize(request->server_info());
	rank_info->sid_  	= sid;
	rank_info->tick_ 	= ::time(NULL);
	rank_info->amount_ += request->amount();
	rank_info->activity_id_ = request->activity_id();

	const Json::Value &conf = this->conf(type);
	JUDGE_RETURN(conf != Json::Value::null && rank_info->amount_ >= conf["need"].asInt(), 0);

	this->sort_rank(type);

#ifdef LOCAL_DEBUG
	this->save_rank_info();
#endif

	return 0;
}

int TrvlRechargeMonitor::add_back_recharge_rank_info(int sid, Int64 role_id, Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400516*, request, -1);

	MSG_USER("sid: %d, Proto30400516: %s", sid, request->Utf8DebugString().c_str());

	int type = request->type();
	TrvlRechargeRank *rank_info = this->find_and_pop(role_id, type);
	JUDGE_RETURN(rank_info != NULL, 0);

	this->recharge_back_start_tick_.sec(request->act_info().act_start());
	this->recharge_back_end_tick_.sec(request->act_info().act_end());

	rank_info->BaseMember::unserialize(request->self_info());
	rank_info->BaseServerInfo::unserialize(request->server_info());
	rank_info->sid_  	= sid;
	rank_info->tick_ 	= ::time(NULL);
	rank_info->amount_  += request->amount();
	rank_info->activity_id_ = request->activity_id();

	JUDGE_RETURN(rank_info->amount_ >= request->act_info().need_gold(), 0);

	this->sort_rank_by_act_info(type, request->mutable_act_info());

#ifdef LOCAL_DEBUG
	this->save_rank_info();
#endif

	return 0;
}

int TrvlRechargeMonitor::fetch_recharge_rank_info(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400512*, request, -1);

	PageInfo page_info;
	Int64 max_role = 0;
	int type = request->type();
	if (type == TYPE_GET && this->recharge_get_vec_.size() > 0)
	{
		int size = this->recharge_get_vec_.size() - 1;
		max_role = this->recharge_get_vec_[size].id_;
	}
	else if (type == TYPE_BACK_RECHAGE && this->recharge_back_vec_.size() > 0)
	{
		int size = this->recharge_back_vec_.size() - 1;
		max_role = this->recharge_back_vec_[size].id_;
	}
	else if (type == TYPE_COST && this->recharge_cost_vec_.size() > 0)
	{
		int size = this->recharge_cost_vec_.size() - 1;
		max_role = this->recharge_cost_vec_[size].id_;
	}

	if (max_role > 0)
	{
		TrvlRechargeRank *max_rank = this->find_role(max_role, type);
		JUDGE_RETURN(max_rank != NULL, 0);

		GameCommon::game_page_info(page_info, request->page(),
				max_rank->rank_, RANK_PAGE_AMOUNT);
	}
	else
	{
		GameCommon::game_page_info(page_info, request->page(),
				0, RANK_PAGE_AMOUNT);
	}

	Proto50102051 respond;
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);

	int left_tick = GameCommon::next_day();
	respond.set_left_tick(left_tick);

	TrvlRechargeRank *my_rank = this->find_role(role, type);
	if (my_rank != NULL)
	{
		ProtoRechargeRank *proto_rank = respond.mutable_my_rank();
		my_rank->serialize(proto_rank);
	}

	if (type == TYPE_GET)
	{
		for (ThreeObjVec::iterator iter = this->recharge_get_vec_.begin();
				iter != this->recharge_get_vec_.end(); ++iter)
		{
			TrvlRechargeRank *rank_info = this->find_role(iter->id_, type);
			JUDGE_CONTINUE(rank_info != NULL);
			JUDGE_CONTINUE((rank_info->rank_-1) >= page_info.start_index_
					&& (rank_info->rank_-1) < (page_info.start_index_ + RANK_PAGE_AMOUNT));

			ProtoRechargeRank *proto_rank = respond.add_rank_info();
			rank_info->serialize(proto_rank);
		}
	}
	else if (type == TYPE_BACK_RECHAGE)
	{
		for (ThreeObjVec::iterator iter = this->recharge_back_vec_.begin();
				iter != this->recharge_back_vec_.end(); ++iter)
		{
			TrvlRechargeRank *rank_info = this->find_role(iter->id_, type);
			JUDGE_CONTINUE(rank_info != NULL);
			JUDGE_CONTINUE((rank_info->rank_-1) >= page_info.start_index_
					&& (rank_info->rank_-1) < (page_info.start_index_ + RANK_PAGE_AMOUNT));

			ProtoRechargeRank *proto_rank = respond.add_rank_info();
			rank_info->serialize(proto_rank);
		}
	}
	else
	{
		for (ThreeObjVec::iterator iter = this->recharge_cost_vec_.begin();
				iter != this->recharge_cost_vec_.end(); ++iter)
		{
			TrvlRechargeRank *rank_info = this->find_role(iter->id_, type);
			JUDGE_CONTINUE(rank_info != NULL);
			JUDGE_CONTINUE((rank_info->rank_-1) >= page_info.start_index_
					&& (rank_info->rank_-1) < (page_info.start_index_ + RANK_PAGE_AMOUNT));

			ProtoRechargeRank *proto_rank = respond.add_rank_info();
			rank_info->serialize(proto_rank);
		}
	}

	MSG_USER("sid: %d, Proto50102051: %s", sid, respond.Utf8DebugString().c_str());

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

int TrvlRechargeMonitor::fetch_back_trvl_recharge_rank_info(int sid, Int64 role_id, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400515*, request, -1);

	PageInfo page_info;
	Int64 max_role = 0;
	int second_type = request->second_type(), type = 0;
    if (second_type == JYBackActivityItem::STYPE_TRAVEL_RECHARGE_RANK)
	{
        type = TYPE_BACK_RECHAGE;
        if (this->recharge_back_vec_.size() > 0)
        {
		    int size = this->recharge_back_vec_.size() - 1;
		    max_role = this->recharge_back_vec_[size].id_;
        }
	}

	if (max_role > 0)
	{
		TrvlRechargeRank *max_rank = this->find_role(max_role, type);
		JUDGE_RETURN(max_rank != NULL, 0);

		GameCommon::game_page_info(page_info, request->page(),
				max_rank->rank_, RANK_PAGE_AMOUNT);
	}
	else
	{
		GameCommon::game_page_info(page_info, request->page(),
				0, RANK_PAGE_AMOUNT);
	}

	Proto50100222 respond;
    respond.set_first_type(request->first_type());
    respond.set_system_tick(::time(NULL));
    respond.mutable_act_info()->CopyFrom(request->act_info());
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);

	TrvlRechargeRank *my_rank = this->find_role(role_id, type);
	if (my_rank != NULL)
	{
		ProtoRechargeRank *proto_rank = respond.mutable_my_rank();
		my_rank->serialize(proto_rank);
	}

	if (type == TYPE_BACK_RECHAGE)
	{
		for (ThreeObjVec::iterator iter = this->recharge_back_vec_.begin();
				iter != this->recharge_back_vec_.end(); ++iter)
		{
			TrvlRechargeRank *rank_info = this->find_role(iter->id_, type);
			JUDGE_CONTINUE(rank_info != NULL);
			JUDGE_CONTINUE((rank_info->rank_-1) >= page_info.start_index_
					&& (rank_info->rank_-1) < (page_info.start_index_ + RANK_PAGE_AMOUNT));

			ProtoRechargeRank *proto_rank = respond.add_rank_info();
			rank_info->serialize(proto_rank);
		}
	}

    MSG_USER("back travel recharge rank sid: %d, role:%ld %s %d", sid, role_id, respond.my_rank().name().c_str(), respond.my_rank().rank());
	//MSG_USER("sid: %d, Proto50102051: %s", sid, respond.Utf8DebugString().c_str());

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
}

void TrvlRechargeMonitor::rank_reset_everyday()
{
	BLongMap get_map = this->recharge_get_package_->fetch_index_map();
	for (BLongMap::iterator iter = get_map.begin(); iter != get_map.end(); ++iter)
	{
		TrvlRechargeRank *rank_info = this->recharge_get_package_->find_object(iter->first);
		JUDGE_CONTINUE(rank_info != NULL);

		this->recharge_get_package_->unbind_and_push(iter->first, rank_info);
	}

	BLongMap cost_map = this->recharge_cost_package_->fetch_index_map();
	for (BLongMap::iterator iter = cost_map.begin(); iter != cost_map.end(); ++iter)
	{
		TrvlRechargeRank *rank_info = this->recharge_cost_package_->find_object(iter->first);
		JUDGE_CONTINUE(rank_info != NULL);

		this->recharge_cost_package_->unbind_and_push(iter->first, rank_info);
	}

	MMOTravel::remove_recharge_rank();

	this->recharge_get_vec_.clear();
	this->recharge_cost_vec_.clear();

    this->save_rank_info();

	this->reset_timer_.cancel_timer();
	this->reset_timer_.schedule_timer(GameCommon::next_day());
}

void TrvlRechargeMonitor::recharge_back_rank_reset()
{
    BLongMap recharge_back_map = this->recharge_back_package_->fetch_index_map();
	for (BLongMap::iterator iter = recharge_back_map.begin(); iter != recharge_back_map.end(); ++iter)
	{
		TrvlRechargeRank *rank_info = this->recharge_back_package_->find_object(iter->first);
		JUDGE_CONTINUE(rank_info != NULL);

		this->recharge_back_package_->unbind_and_push(iter->first, rank_info);
	}

	this->recharge_back_vec_.clear();

    this->save_rank_info();
}

void TrvlRechargeMonitor::recharge_back_rank_check_end()
{
    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->recharge_back_end_tick_ <= nowtime && this->recharge_back_end_tick_ != Time_Value::zero, ;);

    this->handle_send_recharge_back_mail();
    this->recharge_back_rank_reset();
    this->recharge_back_end_tick_ = Time_Value::zero;
}

void TrvlRechargeMonitor::set_mail(BLongMap &index_map, int type)
{
	for (BLongMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TrvlRechargeRank *rank_info = this->find_role(iter->first, type);
		JUDGE_CONTINUE(rank_info != NULL && rank_info->rank_ > 0);

		int sid = MAP_MONITOR->fetch_gate_sid(rank_info->fetch_flag_by_trvl());
		JUDGE_CONTINUE(sid >= 0);

		LongMap &role_rank = this->sid_player_map_[sid];
		role_rank[rank_info->id_] = rank_info->rank_;
	}
}

void TrvlRechargeMonitor::send_mail(int type)
{
	int i = 0;
	for (SidPlayerMap::iterator iter = this->sid_player_map_.begin();
			iter != this->sid_player_map_.end(); ++iter)
	{
		int sid = iter->first;
		LongMap &role_rank = iter->second;

		Proto30400513 inner;
		inner.set_type(type);
		for (LongMap::iterator role_it = role_rank.begin(); role_it != role_rank.end(); ++role_it)
		{
			ProtoThreeObj *obj = inner.add_obj();
			obj->set_id(role_it->first);
			obj->set_value(role_it->second);
			if (type == TYPE_BACK_RECHAGE)
			{
				TrvlRechargeRank *rank_info = this->find_role(role_it->first, type);
				if (rank_info != NULL)
					obj->set_tick(rank_info->amount_);

				if (obj->value() <= 100 && (i++) == 0)
				{
					MSG_USER("recharge bak rank %d %ld %d %d", sid, obj->id(), obj->value(), obj->tick());
				}
			}
		}
		MAP_MONITOR->dispatch_to_logic(sid, &inner);
	}
}

void TrvlRechargeMonitor::push_in_vec(BLongMap &index_map, int type, int need)
{
	for (BLongMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TrvlRechargeRank *rank_info = this->find_role(iter->first, type);
		JUDGE_CONTINUE(rank_info != NULL && rank_info->amount_ >= need);

		ThreeObj obj;
		obj.id_ 	= rank_info->id_;
		obj.value_  = rank_info->amount_;
		obj.tick_ 	= rank_info->tick_;

		if (type == TYPE_GET)
			this->recharge_get_vec_.push_back(obj);
		else if (type == TYPE_BACK_RECHAGE)
			this->recharge_back_vec_.push_back(obj);
		else
			this->recharge_cost_vec_.push_back(obj);
	}
}

void TrvlRechargeMonitor::set_rank(Int64 role, int type, int& rank)
{
	const Json::Value &conf = this->conf(type);
	JUDGE_RETURN(conf != Json::Value::null, ;);

	const Json::Value &rank_limit = conf["rank_limit"];

	TrvlRechargeRank* rank_info = this->find_role(role, type);
	JUDGE_RETURN(rank_info != NULL, ;);

	uint size = rank_limit.size() - 1;
	int max_rank = rank_limit[size][1u].asInt();

	if (rank_limit != Json::Value::null && rank <= max_rank)
	{
		int has_rank = false;
		for (uint i = 0; i < rank_limit.size(); ++i)
		{
			int rank1 = rank_limit[i][0u].asInt();
			int rank2 = rank_limit[i][1u].asInt();
			int need_score = rank_limit[i][2u].asInt();
			if (rank >= rank1 && rank <= rank2 && rank_info->amount_ >= need_score)
			{
				rank_info->rank_ = rank;
				++rank;
				has_rank = true;
				break;
			}
			if (rank < rank2 + 1)
				rank = rank2 + 1;
		}
		if (has_rank == false)
		{
			rank_info->rank_ = max_rank + 1;
			rank = rank_info->rank_ + 1;
		}
	}
	else
	{
		rank_info->rank_ = rank;
		++rank;
	}
}

void TrvlRechargeMonitor::sort_rank(int type)
{
	const Json::Value &conf = this->conf(type);
	JUDGE_RETURN(conf != Json::Value::null, ;);

	int need = conf["need"].asInt();
	int rank = 1;
	if (type == TYPE_GET)
	{
		this->recharge_get_vec_.clear();
		BLongMap index_map = this->recharge_get_package_->fetch_index_map();
		this->push_in_vec(index_map, type, need);

		std::sort(this->recharge_get_vec_.begin(), this->recharge_get_vec_.end(),
				GameCommon::three_comp_by_desc);

		for (ThreeObjVec::iterator iter = this->recharge_get_vec_.begin();
				iter != this->recharge_get_vec_.end(); ++iter)
		{
			this->set_rank(iter->id_, type, rank);
		}
	}
	else if (type == TYPE_BACK_RECHAGE)
	{
		this->recharge_back_vec_.clear();
		BLongMap index_map = this->recharge_back_package_->fetch_index_map();
		this->push_in_vec(index_map, type, need);

		std::sort(this->recharge_back_vec_.begin(), this->recharge_back_vec_.end(),
				GameCommon::three_comp_by_desc);

		for (ThreeObjVec::iterator iter = this->recharge_back_vec_.begin();
				iter != this->recharge_back_vec_.end(); ++iter)
		{
			this->set_rank(iter->id_, type, rank);
		}
	}
	else
	{
		this->recharge_cost_vec_.clear();
		BLongMap index_map = this->recharge_cost_package_->fetch_index_map();
		this->push_in_vec(index_map, type, need);

		std::sort(this->recharge_cost_vec_.begin(), this->recharge_cost_vec_.end(),
				GameCommon::three_comp_by_desc);

		for (ThreeObjVec::iterator iter = this->recharge_cost_vec_.begin();
				iter != this->recharge_cost_vec_.end(); ++iter)
		{
			this->set_rank(iter->id_, type, rank);
		}
	}
}

void TrvlRechargeMonitor::set_rank_by_act_info(Int64 role, int type, int &rank, Message *msg)
{
	DYNAMIC_CAST_RETURN(ProtoBackActInfo *, proto_back_act, msg, ;);

	TrvlRechargeRank* rank_info = this->find_role(role, type);
	JUDGE_RETURN(rank_info != NULL, ;);

	int size = proto_back_act->reward_list_size() - 1, max_rank = 50;
	if (0 <= size && proto_back_act->reward_list(size).cond_value_list_size() > 1)
		max_rank = proto_back_act->reward_list(size).cond_value_list(1);

	if (max_rank > 0 && rank <= max_rank)
	{
		int has_rank = false;
		for (int i = 0; i < proto_back_act->reward_list_size(); ++i)
		{
			const ProtoBackActReward &proto_reward = proto_back_act->reward_list(i);
			int rank1 = 0, rank2 = 999999999, need_gold = 0;
			if (proto_reward.cond_value_list_size() > 0)
				rank1 = proto_reward.cond_value_list(0);
			if (proto_reward.cond_value_list_size() > 1)
				rank2 = proto_reward.cond_value_list(1);
			if (proto_reward.cond_value_list_size() > 2)
				need_gold = proto_reward.cond_value_list(2);
			if (rank1 <= rank && rank <= rank2 && rank_info->amount_ >= need_gold)
			{
				rank_info->rank_ = rank;
				++rank;
				has_rank = true;
				break;
			}
			if (rank < rank2 + 1)
				rank = rank2 + 1;
		}
		if (has_rank == false)
		{
			rank_info->rank_ = max_rank + 1;
			rank = rank_info->rank_ + 1;
		}
	}
	else
	{
		rank_info->rank_ = rank;
		++rank;
	}
}

void TrvlRechargeMonitor::sort_rank_by_act_info(int type, Message *msg)
{
	DYNAMIC_CAST_RETURN(ProtoBackActInfo *, proto_back_act, msg, ;);

	this->recharge_back_vec_.clear();
	BLongMap index_map = this->recharge_back_package_->fetch_index_map();
	this->push_in_vec(index_map, type, proto_back_act->need_gold());

	std::sort(this->recharge_back_vec_.begin(), this->recharge_back_vec_.end(),
			GameCommon::three_comp_by_desc);

	int rank = 1;
	for (ThreeObjVec::iterator iter = this->recharge_back_vec_.begin();
			iter != this->recharge_back_vec_.end(); ++iter)
	{
		this->set_rank_by_act_info(iter->id_, type, rank, proto_back_act);
	}
}

void TrvlRechargeMonitor::handle_send_mail()
{
	this->sid_player_map_.clear();
	BLongMap get_map = this->recharge_get_package_->fetch_index_map();
	this->set_mail(get_map, TYPE_GET);
	this->send_mail(TYPE_GET);

	this->sid_player_map_.clear();
	BLongMap cost_map = this->recharge_cost_package_->fetch_index_map();
	this->set_mail(cost_map, TYPE_COST);
	this->send_mail(TYPE_COST);
}

void TrvlRechargeMonitor::handle_send_recharge_back_mail()
{
    this->sid_player_map_.clear();
	BLongMap back_recharge_map = this->recharge_back_package_->fetch_index_map();
	this->set_mail(back_recharge_map, TYPE_BACK_RECHAGE);
	this->send_mail(TYPE_BACK_RECHAGE);
}

TrvlRechargeRank* TrvlRechargeMonitor::find_and_pop(Int64 role, int type)
{
	JUDGE_RETURN(role > 0, NULL);

	if (type == TYPE_GET)
	{
		TrvlRechargeRank *get_rank = this->recharge_get_package_->find_object(role);
		JUDGE_RETURN(get_rank == NULL, get_rank);

		get_rank = this->recharge_get_package_->pop_object();
		get_rank->reset();
		get_rank->id_ = role;

		this->recharge_get_package_->bind_object(role, get_rank);
		return get_rank;
	}
	else if (type == TYPE_BACK_RECHAGE)
	{
		TrvlRechargeRank *get_rank = this->recharge_back_package_->find_object(role);
		JUDGE_RETURN(get_rank == NULL, get_rank);

		get_rank = this->recharge_back_package_->pop_object();
		get_rank->reset();
		get_rank->id_ = role;

		this->recharge_back_package_->bind_object(role, get_rank);
		return get_rank;
	}
	else
	{
		TrvlRechargeRank *cost_rank = this->recharge_cost_package_->find_object(role);
		JUDGE_RETURN(cost_rank == NULL, cost_rank);

		cost_rank = this->recharge_cost_package_->pop_object();
		cost_rank->reset();
		cost_rank->id_ = role;

		this->recharge_cost_package_->bind_object(role, cost_rank);
		return cost_rank;
	}

	return NULL;
}

TrvlRechargeRank* TrvlRechargeMonitor::find_role(Int64 role, int type)
{
	if (type == TYPE_GET)
		return this->recharge_get_package_->find_object(role);
	else if (type == TYPE_BACK_RECHAGE)
		return this->recharge_back_package_->find_object(role);
	else
		return this->recharge_cost_package_->find_object(role);

	return NULL;
}

BLongMap TrvlRechargeMonitor::fetch_long_map(int type)
{
	if (type == TYPE_GET)
		return this->recharge_get_package_->fetch_index_map();
	else if (type == TYPE_BACK_RECHAGE)
		return this->recharge_back_package_->fetch_index_map();
	else
		return this->recharge_cost_package_->fetch_index_map();
}

const Json::Value &TrvlRechargeMonitor::conf(int type)
{
	return CONFIG_INSTANCE->daily_recharge_rank(type);
}

void TrvlRechargeMonitor::save_rank_info()
{
	MSG_USER("recharge_get_package size: %d, recharge_cost_package size: %d",
			this->recharge_get_package_->size(), this->recharge_cost_package_->size());

	MMOTravel::save_recharge_rank(this);
}

void TrvlRechargeMonitor::load_rank_info()
{
	MMOTravel::load_recharge_rank(this);

	this->sort_rank(TYPE_GET);
	this->sort_rank(TYPE_COST);
}

int TrvlRechargeMonitor::test_send_recharge_mail(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400514*, request, -1);

	this->handle_send_mail();
	this->rank_reset_everyday();
    this->recharge_back_rank_check_end();

#ifdef LOCAL_DEBUG
	this->save_rank_info();
#endif

	return 0;
}

ThreeObjVec &TrvlRechargeMonitor::recharge_back_vec(void)
{
	return this->recharge_back_vec_;
}

void TrvlRechargeMonitor::request_correct_rank_info(void)
{
    MMOTravel::request_correct_rank_info();
}

int TrvlRechargeMonitor::correct_trvl_rank(Transaction *transaction)
{
    JUDGE_RETURN(transaction != NULL, -1);
    if (transaction->detail().__error != 0)
    {
        transaction->rollback();
        return -1;
    }

    MongoDataMap *data_map = transaction->fetch_mongo_data_map();
    if (data_map == NULL)
    {
        transaction->summit();
        return -1;
    }
    MMOTravel::correct_rank_info_from_db(this, data_map);

    return 0;
}

