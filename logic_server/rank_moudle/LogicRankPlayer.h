/*
 * LogicRankPlayer.h
 *
 *  Created on: Mar 4, 2014
 *      Author: louis
 */

#ifndef LOGICRANKPLAYER_H_
#define LOGICRANKPLAYER_H_

#include "BaseLogicPlayer.h"
#include "RankStruct.h"


class LogicRankPlayer: virtual public BaseLogicPlayer
{
public:
	LogicRankPlayer();
	virtual ~LogicRankPlayer();

	virtual int fetch_rank_data(Message* msg);
	virtual int fetch_rank_data_ex(Message* msg);
	virtual int fetch_ranker_detail_info(Message* msg);

	int after_fetch_ranker_detail(Transaction* transaction);
	int reset();

	int request_worship_rank(Message* msg);
    int request_fetch_rank_beast_info(Message* msg);
    int respond_fetch_rank_beast_info(Transaction* trans);
    int process_fetch_rank_data_after_fetch_self_data(Message *msg);

    void decode_worship_num(Int64 &result, int &type, Int64 &role_id);
    int get_is_worship();

    int refresh_open_activity_rank_info();
private:
    int fetch_league_rank_data(int data_type);

private:
	int last_water_flag_; // index
//	RankRecordVec cache_record_set_;
};

#endif /* LOGICRANKPLAYER_H_ */
