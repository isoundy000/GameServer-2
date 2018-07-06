/*
 * MLTrade.h
 *
 *  Created on: 2015-11-21
 *      Author: xu
 */

#ifndef MLTRADE_H_
#define MLTRADE_H_

#include "MLPacker.h"
#include "PubStruct.h"
#include "MapLogicStruct.h"

class MapLogicPlayer;

//交易状态变量
enum{
	TRADE_IDLE, //未交易
	TRADE_REQUESTING, //交易请求中
	TRADE_WAITING, //交易等待中
	TRADE_PROCESS, //交易进行中
	TRADE_SUCCESS
};

enum{
	CANCEL_DEFAULT,
	CANCEL_NOT_DEFAULT
};

class MLTrade:virtual public MLPacker
{
public:

	class RespondOneRequestTimer: public GameTimer
	{
		public:
			RespondOneRequestTimer(void);
			~RespondOneRequestTimer(void);
			int type(void);
			int handle_timeout(const Time_Value &tv);
			void set_parent(MLTrade *parent);
			MLTrade *parent_;
	};

	MLTrade();
	virtual ~MLTrade();
	virtual MapLogicPlayer *trade_player(void);
	void reset_trade();
	void trade_wait_once_end();
	void trade_state_in_requesting(Int64 role_id);
	void trade_state_in_waiting(Int64 role_id);
	void trade_state_in_progress(Int64 role_id);
	void trade_state_in_idle(MapLogicPlayer* sponser_player);
public:
	int trade_condition_judgement(const int recogn,MapLogicPlayer *player);
	int request_org_trade(Message *msg);
	int trade_condition_satisfy(Message *msg);
	int trade_update_to_map_player(Int64 role_id,int type);
	int trade_respond_invite(Message* msg);
	//int trade_notify_show_icon(MapLogicPlayer *player,Int64 sponser_role_id,const string sponser_role_name);
	int trade_remove_role_id_from_responser(Int64 responer_role_id);
	int trade_remove_icon(Int64 responer_role_id);
   void trade_notify_others_request_cancel(Int64 except_role_id);
	int trade_push_items(Message *msg);
	int trade_pop_items(Message *msg);
	int trade_make_up_trade_box(MapLogicPlayer* des_player);
	int trade_lock_panel();
	int trade_start();
	int trade_judge_package(GamePackage* des_package);
	int trade_insert_des_package(GamePackage* des_package,MLPacker *des_mlpacker);
	int inner_notify_trade_cancel(const int type = CANCEL_DEFAULT);
	int trade_close_panel();
   bool trade_check_limit_condition();
	int trade_send_to_map_condition(int recogn,Int64 role_id);
protected:
	TradeInfo trade_info_;
	bool is_close_panel_;
private:
	RespondOneRequestTimer wait_once_timer_;  //一次等待响应的时间
};

#endif /* MLTRADE_H_ */
