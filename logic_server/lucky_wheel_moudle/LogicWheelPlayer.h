/*
 * LogicWheelPlayer.h
 *
 *  Created on: 2016年12月15日
 *      Author: lyw
 */

#ifndef LOGICWHEELPLAYER_H_
#define LOGICWHEELPLAYER_H_

#include "BaseLogicPlayer.h"
#include "ProtoDefine.h"

class LogicWheelPlayer : virtual public BaseLogicPlayer
{
	enum
	{
		NONE_REWARD = 1,
		HAVE_REWARD = 2,
		GONE_REWARD = 3,
	};

	enum
	{
		REFRESH_FISH,
		GET_FISH,
		SUPER_GET_FISH,
		GET_ALL_FISH,
	};
public:
	LogicWheelPlayer();
	virtual ~LogicWheelPlayer();

	void reset();
	void reset_every_day();
	void wheel_player_sign_out();

	int fetch_wheel_act_list();
	int fetch_one_wheel_activity(Message* msg);
	int fetch_activity_info(LuckyWheelActivity::ActivityDetail* act_detail); //轮盘类活动

	int fetch_recharge_rebate_info();
	int recharge_rebate(Message* msg);	//充值返元宝

	int fetch_time_limit_info(); //限时秒杀
	int time_limit_item_buy_begin(Message* msg);
	int time_limit_item_buy_done(Message* msg);

	int fetch_activity_reward(Message* msg);	//限时秒杀/幸运砸蛋领取次数奖励

	int fetch_wedding_rank_info(Message* msg); //我们结婚吧活动
	int fetch_wedding_act_reward_begin(Message* msg);
	int fetch_wedding_act_reward_end(int type, int rank);

	int fetch_recharge_rank_info(Message* msg); //每日冲榜
	int update_recharge_rank(int type, int money);
	int test_send_recharge_mail();

	int request_open_nine_word_begin(Message* msg); //开启密宗九字格子
	int request_open_nine_word_end(Message* msg);
	int fetch_nine_word_reward(Message* msg);

	int fetch_act_rank_info(Message* msg);		//活动排行榜

	int request_open_lucky_egg_begin(Message* msg);	//开启幸运砸蛋格子
	int request_open_lucky_egg_end(Message* msg);

	//神仙鉴宝
	int fetch_immortal_treasures(int type = 0);
	int fetch_immortal_treasures_reward();
	int draw_immortal_treasures_begin();
	int draw_immortal_treasures_end(Message* msg);
	int rand_immortal_treasures_begin(Message* msg);
	int rand_immortal_treasures_end(Message* msg);
	int refresh_immortal_treasures_info();
	int refresh_immortal_treasures_info_by_id(int id = 0);
	int check_fina_slot_map();
	//迷宫寻宝
	int fetch_maze_treasures(Message* msg);
	int draw_maze_begin(Message* msg);
	int draw_maze_end(Message* msg);
	int draw_maze_times_work(int slot_id, int slot_type);	//同类型其他格子刷新次数
	int draw_maze_type_work(int slot_id, int type);
	int draw_maze_msg_info(Proto50102022 &respond, int slot_id, int pre_slot_id);

	//珍宝阁和折扣商店
	int cabinet_sign_in(int activity_id);
	int fetch_cabinet_info(Message* msg);
	int fetch_cabinet_info(int activity_id);
	int cabinet_buy_begin(Message* msg);
	int cabinet_buy_end(Message* msg);
	int cabinet_refresh_begin(Message* msg);
	int cabinet_refresh_end(Message* msg);
	int cabinet_refresh_reward(Message* msg);
	int cabinet_refresh_work(int activity_id, int type = 0,
			int shop_slot_num = LuckyWheelActivity::SHOP_SLOT_NUM, int shop_last_group = LuckyWheelActivity::SHOP_LAST_GROUP);

	//七彩扭蛋
	int fetch_gashapon_info(Message* msg);
	int update_gashapon_recharge(int gold);
	int fetch_gashapon_reward(Message* msg);

	//宝石合成
	int fetch_gem_activity_info(void);
	int gem_synthesis(Message* msg);
	int draw_gem_synthesis_reward(void);
	int inner_gem_synthesis_pack_operate(Message* msg);

	//女神赐福
	int fetch_bless_activity_info(void);
	int goddess_bless_operator_begin(Message* msg);
	int goddess_bless_operator(Message* msg);
	int goddess_bless_operator_end(Message* msg);
	int fetch_random_reward(ItemRate &item_obj);
	int goddess_bless_exchange_begin(Message* msg);
	int goddess_bless_exchange_end(Message *msg);

	int draw_award_begin(Message* msg);
	int draw_award_done(Message* msg);
	int draw_lucky_wheel_award_done(Message* msg);
	int draw_advance_box_award_done(Message* msg);
	int draw_gold_box_award_done(Message* msg);

	int lucky_wheel_exchange(Message* msg);
	int request_activity_reset_begin(Message* msg);
	int request_activity_reset_done(Message* msg);

	//深海捕鱼
	int fetch_fish_info(Message *msg);
	int init_all_fish();
	int fetch_fish_begin(Message *msg);
	int fetch_fish_end(Message *msg);
	int fetch_fish_score_info(Message *msg);
	int fetch_fish_tips_info(Message *msg);
	int fetch_fish_score_reward(Message *msg);
	int use_prop_add_score(Message *msg);
	int syna_fish_info(Message *msg);
	void serialize_fish_info(const FishInfo *read_info, FishDetail* write_detail);
	void unserialize_fish_info(FishInfo *write_info, const FishDetail* read_detail);

	void test_show_fish_info();
	void clean_fish_info();

	int rand_slot_item(int activity_id, int slot_num = LuckyWheelActivity::SHOP_SLOT_NUM);

	WheelPlayerInfo &wheel_player_info();
	WheelPlayerInfo::PlayerDetail* fetch_player_detail(int activity_id);

	void test_reset_activity(int activity_id);

protected:
	int login_interval_tick();
	void serial_draw_serialize(ProtoSerialObj* obj, int activity_id, int type = 0);	//抽奖流水
	void serial_reset_serialize(ProtoSerialObj* obj, int activity_id);	//重置流水

	void player_detail_change(WheelPlayerInfo::PlayerDetail* player_detail,
			LuckyWheelActivity::ActivityDetail* act_detail);
	void add_player_slot_info(WheelPlayerInfo::PlayerDetail* player_detail,
			LuckyWheelActivity::ActivityDetail* act_detail);
	WheelPlayerInfo::ItemRecord create_record(ItemObj& obj, int reward_mult = 0, int sub_value = 0);
	void record_person(WheelPlayerInfo::PlayerDetail* player_detail,
			int is_record, WheelPlayerInfo::ItemRecord& record);
	int cal_free_times(WheelPlayerInfo::PlayerDetail* player_detail,
			LuckyWheelActivity::ActivityDetail* act_detail);
	int cal_left_free_time(WheelPlayerInfo::PlayerDetail* player_detail,
			LuckyWheelActivity::ActivityDetail* act_detail);
	int check_red_point(WheelPlayerInfo::PlayerDetail* player_detail,
			LuckyWheelActivity::ActivityDetail* act_detail);
	void add_lucky_rank_num(WheelPlayerInfo::PlayerDetail* player_detail,
			LuckyWheelActivity::ActivityDetail* act_detail);
	void add_item(ItemObj &item_obj, SerialObj &obj);
	int fetch_reset_cost(int activity_id, int reset_times, IntMap &reset_cost);

private:
	WheelPlayerInfo wheel_info_;
};

#endif /* LOGICWHEELPLAYER_H_ */
