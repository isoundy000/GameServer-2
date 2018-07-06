/*
 * TrvlWeddingMonitor.cpp
 *
 *  Created on: 2017年2月10日
 *      Author: lyw
 */

#include "TrvlWeddingMonitor.h"
#include "GameCommon.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "MMOTravel.h"

TrvlWeddingRank::TrvlWeddingRank()
{
	TrvlWeddingRank::reset();
}

void TrvlWeddingRank::reset()
{
	BaseServerInfo::reset();

	this->rank_ = 0;
	this->tick_ = 0;
	this->player1_.reset();
	this->player2_.reset();
}

void TrvlWeddingRank::serialize(ProtoWeddingRank *rank_info)
{
	rank_info->set_rank(this->rank_);
	rank_info->set_tick(this->tick_);
	rank_info->set_server_id(this->__server_id);
	rank_info->set_server_flag(this->__server_flag);
	rank_info->set_server_prev(this->__server_prev);
	rank_info->set_server_name(this->__server_name);

	ProtoWeddingRole *player1 = rank_info->mutable_player1();
	ProtoWeddingRole *player2 = rank_info->mutable_player2();
	player1->set_role_id(this->player1_.id_);
	player1->set_role_name(this->player1_.name_);
	player1->set_sex(this->player1_.sex_);
	player2->set_role_id(this->player2_.id_);
	player2->set_role_name(this->player2_.name_);
	player2->set_sex(this->player2_.sex_);
}

void TrvlWeddingRank::unserialize(ProtoWeddingRank *rank_info)
{
	this->__server_id   = rank_info->server_id();
	this->__server_flag = rank_info->server_flag();
	this->__server_prev = rank_info->server_prev();
	this->__server_name = rank_info->server_name();

	ProtoWeddingRole player1 = rank_info->player1();
	ProtoWeddingRole player2 = rank_info->player2();
	this->player1_.id_ 	 = player1.role_id();
	this->player1_.name_ = player1.role_name();
	this->player1_.sex_  = player1.sex();
	this->player2_.id_ 	 = player2.role_id();
	this->player2_.name_ = player2.role_name();
	this->player2_.sex_  = player2.sex();
}

int TrvlWeddingMonitor::ResetTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlWeddingMonitor::ResetTimer::handle_timeout(const Time_Value &tv)
{
	TRVL_WEDDING_MONITOR->handle_del_rank_info();
	return 0;
}

TrvlWeddingMonitor::TrvlWeddingMonitor() {
	// TODO Auto-generated constructor stub
	this->rank_package_ = new PoolPackage<TrvlWeddingRank, int>;
}

TrvlWeddingMonitor::~TrvlWeddingMonitor() {
	// TODO Auto-generated destructor stub
}

void TrvlWeddingMonitor::start()
{
	this->load_rank_info();
	this->reset_timer_.schedule_timer(GameCommon::next_day());

	MSG_USER("TrvlWeddingMonitor start!");
}

void TrvlWeddingMonitor::stop()
{
	this->save_rank_info();
	this->reset_timer_.cancel_timer();

	MSG_USER("TrvlWeddingMonitor stop!");
}

int TrvlWeddingMonitor::wedding_rank_info(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400507*, request, -1);
	MSG_USER("wedding_rank_info,Proto30400507: %s", request->Utf8DebugString().c_str());

	int rank = this->rank_package_->size() + 1;
	TrvlWeddingRank *rank_info = this->pop_rank_info(rank);
	rank_info->tick_ = ::time(NULL);

	ProtoWeddingRank proto_rank = request->rank_info();
	rank_info->unserialize(&proto_rank);

	MSG_USER("rank_size: %d", this->rank_package_->size());

	ProtoWeddingRole player1 = proto_rank.player1();
	ProtoWeddingRole player2 = proto_rank.player2();

	Proto30400509 inner;
	inner.set_rank(rank);
	inner.set_player1(player1.role_id());
	inner.set_player2(player2.role_id());

//	return MAP_MONITOR->dispatch_to_logic_in_all_server(request);
	return MAP_MONITOR->dispatch_to_logic(sid, &inner);
}

int TrvlWeddingMonitor::fetch_wedding_rank_info(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400508*, request, -1);

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page(), this->rank_package_->size(), 10);

	Proto50102009 respond;
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);
	respond.set_couples(this->rank_package_->size());
	respond.set_label_reward(request->label_reward());
	respond.set_rank_reward(request->rank_reward());
	respond.set_left_tick(request->left_tick());

	for (int i = page_info.start_index_ + 1; i <= ::std::min(page_info.total_count_,
			page_info.start_index_ + 10); ++i)
	{
		TrvlWeddingRank *rank_info = this->rank_package_->find_object(i);
		ProtoWeddingRank *proto_rank = respond.add_rank_info();
		rank_info->serialize(proto_rank);
	}

	//我的排名
	TrvlWeddingRank *rank_info = this->fetch_rank_info(request->role_id());
	if (rank_info != NULL)
	{
		ProtoWeddingRank *my_rank = respond.mutable_my_rank();
		rank_info->serialize(my_rank);

		respond.set_label_get(request->label_get());
		respond.set_reward_get(request->reward_get());
	}
	else
	{
		respond.set_label_get(-1);
		respond.set_reward_get(-1);
	}

	MSG_USER("fetch_wedding_rank_info,Proto50102009: %s", respond.Utf8DebugString().c_str());
	return MAP_MONITOR->dispatch_to_client_from_gate(sid, request->role_id(), &respond);
}

int TrvlWeddingMonitor::fetch_wedding_rank_reward(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400510*, request, -1);

	TrvlWeddingRank *rank_info = this->fetch_rank_info(role);
	if (rank_info == NULL)
	{
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, RETURN_FETCH_COUPLE_ACT_REWARD,
				ERROR_COUPLE_ACT_NO_RANK);
	}

	request->set_rank(rank_info->rank_);
	return MAP_MONITOR->dispatch_to_logic(sid, request);
}

TrvlWeddingRank* TrvlWeddingMonitor::fetch_rank_info(Int64 role_id)
{
	JUDGE_RETURN(this->rank_package_->size() > 0, NULL);

	int rank = INT_MAX;
	BIntMap index_map = this->rank_package_->fetch_index_map();
	for (BIntMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TrvlWeddingRank *rank_info = this->rank_package_->find_object(iter->first);
		JUDGE_CONTINUE(rank > rank_info->rank_ && (role_id == rank_info->player1_.id_
				||role_id == rank_info->player2_.id_));

		rank = rank_info->rank_;
	}

	return this->rank_package_->find_object(rank);
}

TrvlWeddingRank* TrvlWeddingMonitor::pop_rank_info(int rank)
{
	TrvlWeddingRank *rank_info = this->rank_package_->pop_object();
	rank_info->rank_ = rank;
	this->rank_package_->bind_object(rank, rank_info);

	return rank_info;
}

void TrvlWeddingMonitor::handle_del_rank_info()
{
	BIntMap index_map = this->rank_package_->fetch_index_map();
	for (BIntMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TrvlWeddingRank *rank_info = this->rank_package_->find_object(iter->first);
		this->rank_package_->unbind_and_push(iter->first, rank_info);
	}

	MSG_USER("rank size: %d", this->rank_package_->size());

	this->reset_timer_.cancel_timer();
	this->reset_timer_.schedule_timer(GameCommon::next_day());
}

void TrvlWeddingMonitor::save_rank_info()
{
	BIntMap index_map = this->rank_package_->fetch_index_map();
	for (BIntMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TrvlWeddingRank *rank_info = this->rank_package_->find_object(iter->first);
		JUDGE_CONTINUE(rank_info != NULL);

		MMOTravel::save_wedding_rank(rank_info);
	}
}

void TrvlWeddingMonitor::load_rank_info()
{
	MMOTravel::load_wedding_rank(this);
}


