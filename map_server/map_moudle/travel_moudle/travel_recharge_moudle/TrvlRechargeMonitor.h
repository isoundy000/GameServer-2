/*
 * TrvlRechargeMonitor.h
 *
 *  Created on: 2017年3月7日
 *      Author: lyw
 */

#ifndef TRVLRECHARGEMONITOR_H_
#define TRVLRECHARGEMONITOR_H_

#include "PubStruct.h"

class ProtoRechargeRank;
class Transaction;

struct TrvlRechargeRank : public BaseServerInfo, public BaseMember
{
	int rank_;
	int amount_;
	int sid_;
	Int64 tick_;
	int activity_id_;	// 后台跨服活动ID

	TrvlRechargeRank();
	void reset();
	void serialize(ProtoRechargeRank *rank_info);
};

class TrvlRechargeMonitor
{
public:
	enum
	{
		TYPE_GET 	= 1,	//充值榜
		TYPE_COST 	= 2,	//消费榜
		TYPE_BACK_RECHAGE = 3,	// 后台配置的跨服充值榜

		RANK_PAGE_AMOUNT = 10,	//排行榜每页10条

		END
	};

	class ResetTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	// 每小时保存一次
	class SaveTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};
    
    class TenSecTimer : public GameTimer
    {
    public:
        virtual int type(void);
        virtual int handle_timeout(const Time_Value &tv);
    };

	typedef std::map<int, LongMap> SidPlayerMap;	//发邮件的玩家列表

public:
	TrvlRechargeMonitor();
	virtual ~TrvlRechargeMonitor();

	void start();
	void stop();

	int add_recharge_rank_info(int sid, Int64 role, Message* msg);
    int add_back_recharge_rank_info(int sid, Int64 role_id, Message* msg);
	int fetch_recharge_rank_info(int sid, Int64 role, Message* msg);
    int fetch_back_trvl_recharge_rank_info(int sid, Int64 role_id, Message *msg);

	void rank_reset_everyday();
    void recharge_back_rank_reset();

    void recharge_back_rank_check_end();

	void sort_rank(int type);
	void sort_rank_by_act_info(int type, Message *msg);
	void handle_send_mail();
    void handle_send_recharge_back_mail();

	TrvlRechargeRank* find_and_pop(Int64 role, int type);
	TrvlRechargeRank* find_role(Int64 role, int type);
	BLongMap fetch_long_map(int type);
	const Json::Value &conf(int type);

	int test_send_recharge_mail(int sid, Int64 role, Message* msg);

	ThreeObjVec &recharge_back_vec(void);

    void request_correct_rank_info(void);
    int correct_trvl_rank(Transaction *transaction);

protected:
	void save_rank_info();
	void load_rank_info();

	void set_mail(BLongMap &index_map, int type);
	void send_mail(int type);
	void push_in_vec(BLongMap &index_map, int type, int need);
	void set_rank(Int64 role, int type, int& rank);
	void set_rank_by_act_info(Int64 role, int type, int &rank, Message *msg);

private:
	PoolPackage<TrvlRechargeRank, Int64>* recharge_get_package_;
	PoolPackage<TrvlRechargeRank, Int64>* recharge_cost_package_;
	PoolPackage<TrvlRechargeRank, Int64>* recharge_back_package_;
	ResetTimer reset_timer_;
	SaveTimer save_timer_;
    TenSecTimer tensec_timer_;
	ThreeObjVec recharge_get_vec_;
	ThreeObjVec recharge_cost_vec_;
	ThreeObjVec recharge_back_vec_;
	SidPlayerMap sid_player_map_;

	Time_Value recharge_back_start_tick_;	// 后台控制的跨服充值活动开始时间
	Time_Value recharge_back_end_tick_;		// 后台控制的跨服充值活动结束时间
};

typedef Singleton<TrvlRechargeMonitor> TrvlRechargeMonitorSingle;
#define TRVL_RECHARGE_MONITOR   (TrvlRechargeMonitorSingle::instance())

#endif /* TRVLRECHARGEMONITOR_H_ */
