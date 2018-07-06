/*
 * LogicOfflineHookPlayer.h
 *
 *  Created on: Mar 21, 2017
 *      Author: root
 */

#ifndef LOGICOFFLINEHOOKPLAYER_H_
#define LOGICOFFLINEHOOKPLAYER_H_

#include "MLPacker.h"

struct Offline_Hookdetial
{
	int soul_limit;   //每天仙魂掉落
	Int64 offhook_playerid;				//离线玩家id
	Int64 last_offhook_exp;				//可获得离线经验
	Time_Value offline_surplustime;		//挂机剩余时间
	Time_Value last_offhook_starttime;
	Time_Value last_offhook_endtime;
	Time_Value last_offhook_time;		//上次挂机时间
	IntMap offhook_costProp;
	IntMap offline_reward;				//离线挂机获得奖励列表
	IntMap today_offline_reward;		//记录今天领取过的奖励，判断奖励是否达到最大领取数

	Time_Value vip_plus_time;				//vip加成时间
	Time_Value one_point_five_plue_time;	//1.5倍道具加成时间
	Time_Value two_plus_time;				//2倍道具加成时间

	Offline_Hookdetial();
	void reset();

	static int max_surplus_time();
};

class LogicOfflineHookPlayer: virtual public MLPacker
{
public:
	enum
	{
		ONE_OFFLINE_ITEM_ID = 212200003,
		TWO_OFFLINE_ITEM_ID = 212200004,
		VIP_OFFLINE_ITEM_ID = 212200005
	};

public:
	LogicOfflineHookPlayer();

	int reset_every_day_offline();

	Offline_Hookdetial& offline_hook_detail();
	//玩家挂机申请
	int player_request_applyofflineHook(Message *msg);
	//把玩家使用离线挂机的消息通知给其余进程
	int player_request_offlineHook(Message *msg);
	int player_replay_offlinereward(Message *msg);

	void player_sigin();
	void player_sigout();
	void hook_reset();

	int fetch_offline_plus_info();
	int use_offline_plus_item(Message *msg);
	int use_offline_plus_item(int item_id, int item_amount = 1);
	int handler_offhookgood_weight(const Json::Value& lvl_json,const int size);

	int hook_sync_transfer_scene(int sceneid);
	int read_hook_sync_transfer_scen(Message *msg);

	int notify_godsoul_limit_info();
	int cmd_hooktime(int hooktimes);

private:
	Offline_Hookdetial m_offdetial;
	bool m_isApplyHook;
};

#endif /* LOGICOFFLINEHOOKPLAYER_H_ */
