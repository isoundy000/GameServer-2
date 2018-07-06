/*
 * MayActivitySys.cpp
 *
 *  Created on: 2017年4月12日
 *      Author: lyw
 */

#include "MayActivitySys.h"
#include "MMOOpenActivity.h"
#include "LogicMonitor.h"
#include "BackField.h"
#include "ProtoDefine.h"
#include "LogicPlayer.h"
#include "WeddingMonitor.h"

#include <mongo/client/dbclient.h>

int MayActivitySys::BackActTimer::type()
{
	return GTT_LOGIC_ONE_MINUTE;
}

int MayActivitySys::BackActTimer::handle_timeout(const Time_Value &tv)
{
	MAY_ACTIVITY_SYS->request_load_activity();
	MAY_ACTIVITY_SYS->handle_sync_map_act();
	return 0;
}

MayActivitySys::MayActivitySys() {
	// TODO Auto-generated constructor stub
	MayActivitySys::reset();
}

MayActivitySys::~MayActivitySys() {
	// TODO Auto-generated destructor stub
}

void MayActivitySys::reset()
{
	this->cur_may_day_ = 0;

	this->act_list_.clear();
	this->act_timer_.cancel_timer();
}

void MayActivitySys::start()
{
	this->init_may_activity();

	this->load_activity();
	this->request_load_activity();

	this->act_timer_.schedule_timer(1);

	MSG_USER("MayActivitySys start");
}

void MayActivitySys::stop()
{
	this->save_activity();

	this->act_timer_.cancel_timer();
}

int MayActivitySys::request_load_activity()
{
	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, 0);

	shop_mode->recogn_ = TRANS_LOAD_MAY_ACTIVITY;
	shop_mode->input_argv_.type_int64_ = this->refresh_tick_;
	return LOGIC_MONITOR->db_load_mode_begin(shop_mode);
}

int MayActivitySys::after_load_activity_done(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode != NULL, -1);

	BSONObj& res = *shop_mode->output_argv_.bson_obj_;
	JUDGE_RETURN(res.isEmpty() == false, -1);

	Int64 refresh_tick = res[DBBackMayActivity::REFRESH_TICK].numberLong();
	int act_type_ = res[DBBackMayActivity::ACT_TYPE].numberInt();
	JUDGE_RETURN(refresh_tick != this->refresh_tick_ && act_type_ > 0, -1);

	this->refresh_tick_ = refresh_tick;
	this->act_type_ 	= act_type_;
	this->open_flag_ 	= res[DBBackMayActivity::OPEN_FLAG].numberInt();
	Int64 begin_date 	= res[DBBackMayActivity::BEGIN_DATE].numberLong();
	Int64 end_date 		= res[DBBackMayActivity::END_DATE].numberLong();
	this->begin_date_ 	= GameCommon::day_zero(begin_date);
	this->end_date_ 	= GameCommon::day_zero(end_date);

	this->back_act_map_.clear();
	BSONObjIterator act_set(res.getObjectField(DBBackMayActivity::ACT_SET.c_str()));
	while (act_set.more())
	{
		int act = act_set.next().numberInt();
		this->back_act_map_[act] = true;
	}

	this->agent_.clear();
	BSONObjIterator agent(res.getObjectField(DBBackMayActivity::AGENT.c_str()));
	while (agent.more())
	{
		int id = agent.next().numberInt();
		this->agent_[id] = true;
	}

	for (ActInfoSet::iterator iter = this->act_info_set_.begin();
			iter != this->act_info_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->act_type_ != this->act_type_);
		iter->request_reset();
		iter->act_type_ = this->act_type_;
	}

	this->update_activity_sys();

	return 0;
}

void MayActivitySys::handle_sync_map_act()
{
	Proto30400042 inner;
	inner.set_type(this->act_type_);
	inner.set_start_tick(this->begin_date_);
	inner.set_end_tick(this->end_date_);
	LOGIC_MONITOR->dispatch_to_all_map(&inner);
}

int MayActivitySys::midnight_handle_timeout(int test_day)
{
	this->update_activity_sys(test_day);
	this->rand_all_red_packet();
	return 0;
}

void MayActivitySys::init_may_activity()
{
	GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->may_activity_map();
	for (GameConfig::ConfigMap::iterator iter = act_map.begin();
			iter != act_map.end(); ++iter)
	{
		this->add_new_item(iter->first, *(iter->second));
	}
}

void MayActivitySys::init_cur_day_act()
{
	this->act_list_.clear();

	Int64 cur_tick = ::time(NULL);
	for (ActInfoSet::iterator iter = this->act_info_set_.begin();
			iter != this->act_info_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->open_flag_ == true);
		JUDGE_CONTINUE(cur_tick >= this->begin_date_ && cur_tick < this->end_date_);
		JUDGE_CONTINUE(iter->open_time_.count(this->cur_may_day_) > 0);
		JUDGE_CONTINUE(this->back_act_map_.count(iter->first_type_) > 0);

		this->act_list_[iter->act_index_] = true;
	}

	MSG_USER("act_list size: %d", this->act_list_.size());

	//进行全服玩家更新
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::const_iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		player->fetch_may_activity_list();
	}
}

void MayActivitySys::update_activity_sys(int test_day)
{
	this->cur_may_day_ = 0;

	if (test_day == -1)
	{
		this->cur_may_day_ = this->act_cur_day();
	}
	else if (test_day <= ACT_LAST_DAY)
	{
		this->cur_may_day_ = test_day;
	}

	this->init_cur_day_act();
}

int MayActivitySys::update_couple_tick(int sec)
{
	JUDGE_RETURN(this->cur_may_day_ > 0, 0);

	ActInfo *act_info = this->find_item_by_day(COUPLE_HEART, this->cur_may_day_);
	JUDGE_RETURN(act_info != NULL, 0);

	LogicPlayer* player1 = NULL;
	LogicPlayer* player2 = NULL;

	WeddingMonitor::WeddingMap &wedding_map = WEDDING_MONITOR->wedding_by_id_map();
	for (WeddingMonitor::WeddingMap::iterator iter = wedding_map.begin();
			iter != wedding_map.end(); ++iter)
	{
		WeddingDetail *detail = iter->second;
		JUDGE_CONTINUE(detail != NULL);

		Int64 role_1 = detail->__partner_1.__role_id;
		Int64 role_2 = detail->__partner_2.__role_id;
		JUDGE_CONTINUE(LOGIC_MONITOR->find_player(role_1, player1) == 0
				&& LOGIC_MONITOR->find_player(role_2, player2) == 0);

		CoupleInfo &couple_info = act_info->couple_info_map_[iter->first];
		couple_info.wedding_id_ = iter->first;
		couple_info.online_tick_ += sec;

		int total_min = (couple_info.online_tick_ + couple_info.buy_tick_) / 60;
		player1->update_may_act_one_value(COUPLE_HEART, this->cur_may_day_, total_min);
		player2->update_may_act_one_value(COUPLE_HEART, this->cur_may_day_, total_min);
	}

	return 0;
}

IntMap& MayActivitySys::fetch_act_list()
{
	return this->act_list_;
}

void MayActivitySys::save_activity()
{
	MMOOpenActivity::save_may_activity_sys(this);
}

void MayActivitySys::load_activity()
{
	MMOOpenActivity::load_may_activity_sys(this);
}

int MayActivitySys::cur_may_day() const
{
	return cur_may_day_;
}

void MayActivitySys::init_today_red_packet_list()
{
	int cur_may_day = this->cur_may_day();
	ActInfo *act_info = this->find_item_by_day(SEND_POCKET, cur_may_day);
	JUDGE_RETURN(act_info != NULL, ;);

}

int MayActivitySys::rand_red_packet(int index)
{
	int cur_may_day = this->cur_may_day();
	ActInfo *act_info = this->find_item_by_day(SEND_POCKET, cur_may_day);
	JUDGE_RETURN(act_info != NULL, -1;);
	ActRewardSet& reward_set = act_info->reward_set_;
	int reward_count = reward_set.size();
	JUDGE_RETURN(reward_count > 0, -1);

	GroupPacketMap group_packet_map;

	//最后一个是补漏
	for(int i = 1; i <= reward_count; ++i)
	{
		ActReward &reward = reward_set[i - 1];
		if(reward.packet_count_ <= 0)
			return -1;
		RedPacketVec red_packet_vec;
		for(int j = 0; j < reward.packet_count_; ++j)
		{
			int money = reward.packet_money_;              //配置金额
			float min_money = reward.min_money_;        //最小可获得金额数

			int config_limit_num = reward.money_limit_;

			int min_num = 0, max_num = 0;
			IntVec packet_vec = GameCommon::rand_red_packet(money, reward.get_times_, min_money);
			//限制随机10次以内
			int rand_time = 10;
			while(rand_time--)
			{
				for(IntVec::iterator iter = packet_vec.begin(); iter != packet_vec.end(); ++iter)
				{
					if(*iter > max_num)
						max_num = *iter;
					if(*iter < min_num)
						min_num = *iter;
				}
				if(max_num - min_num > config_limit_num)
				{
					packet_vec = GameCommon::rand_red_packet(money, reward.get_times_, min_money);
					continue;
				}
				break;
			}
			red_packet_vec.push_back(packet_vec);
		}
		group_packet_map[i] = red_packet_vec;
	}

	act_info->all_red_packet_map_[index] = group_packet_map;
    return 0;
}

int MayActivitySys::rand_all_red_packet()
{
	int cur_may_day = this->cur_may_day();
//	cur_may_day = 1;
	ActInfo *act_info = this->find_item_by_day(SEND_POCKET, cur_may_day);
	JUDGE_RETURN(act_info != NULL, -1;);

    int group_count = act_info->limit_time_vec_.size();//配置
    for(int i = 1; i <= group_count; ++i)
    {
        this->rand_red_packet(i);
    }
    return 0;
}

bool MayActivitySys::is_open_activity() const
{
	return this->open_flag_;
}
