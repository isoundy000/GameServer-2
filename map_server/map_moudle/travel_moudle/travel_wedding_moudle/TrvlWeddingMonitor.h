/*
 * TrvlWeddingMonitor.h
 *
 *  Created on: 2017年2月10日
 *      Author: lyw
 */

#ifndef TRVLWEDDINGMONITOR_H_
#define TRVLWEDDINGMONITOR_H_

#include "PubStruct.h"

class ProtoWeddingRank;

struct TrvlWeddingRank : public BaseServerInfo
{
	int rank_;
	Int64 tick_;
	BaseMember player1_;
	BaseMember player2_;

	TrvlWeddingRank();
	void reset();
	void serialize(ProtoWeddingRank *rank_info);
	void unserialize(ProtoWeddingRank *rank_info);
};

class TrvlWeddingMonitor
{
public:
	class ResetTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	TrvlWeddingMonitor();
	virtual ~TrvlWeddingMonitor();

	void start();
	void stop();

	int wedding_rank_info(int sid, Int64 role, Message* msg);
	int fetch_wedding_rank_info(int sid, Int64 role, Message* msg);
	int fetch_wedding_rank_reward(int sid, Int64 role, Message* msg);

	TrvlWeddingRank* fetch_rank_info(Int64 role_id);
	TrvlWeddingRank* pop_rank_info(int rank);

	void handle_del_rank_info();

protected:
	void save_rank_info();
	void load_rank_info();

private:
	PoolPackage<TrvlWeddingRank, int>* rank_package_;
	ResetTimer reset_timer_;
};

typedef Singleton<TrvlWeddingMonitor> TrvlWeddingMonitorSingle;
#define TRVL_WEDDING_MONITOR   (TrvlWeddingMonitorSingle::instance())

#endif /* TRVLWEDDINGMONITOR_H_ */
