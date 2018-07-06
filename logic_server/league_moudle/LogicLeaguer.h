/*
 * LogicLeaguer.h
 *
 *  Created on: Aug 8, 2013
 *      Author: peizhibi
 */

#ifndef LOGICLEAGUER_H_
#define LOGICLEAGUER_H_

#include "LeagueStruct.h"
#include "BaseLogicPlayer.h"
class Proto30400011;
class ProtoWaveReward;

class LogicLeaguer : public virtual BaseLogicPlayer
{
public:
	LogicLeaguer();
	virtual ~LogicLeaguer();

	Int64 league_index();
	void leaguer_sign_in();
	void leaguer_sign_out();

	int open_league();
	int fetch_league_list_info(Message* msg);

	int create_league(Message* msg);//创建仙盟
	int map_create_league(Message* msg);
	int request_rename_league(Message* msg);//仙盟重命名
	int handle_resex_leaguer(); //变性
	int handle_rename_leaguer(); //改名
	int handle_leader_sign();	//帮主登录处理

	int request_check_same_league(Message* msg);//检查玩家是否同一仙盟
	int apply_join_league(Message* msg);//申请加入
	int cancel_join_league(Message* msg);//取消申请
	int handle_league_apply(League* league);//有成员申请时操作
	int send_apply_red_point(League* league, Int64 role_id);//申请列表红点

	int fetch_leader_inpeach(); //获取弹劾帮主信息
	int fetch_league_info();//获取自己仙盟信息
	int fetch_other_league_info(Message* msg);//获取其他宗派信息
	int quit_league();//退出仙盟
	int map_quit_league(Message* msg);

	int fetch_league_shop_info(Message* msg);//仙盟商店
	int league_shop_buy(Message* msg);//仙盟商店购买
	int map_league_shop_buy(Message* msg);
	int modify_league_intro(Message* msg);//修改仙盟简介

	int fetch_league_welfare();//仙盟福利
	int open_league_donate();//仙盟贡献
	int league_donate(Message* msg);
	int map_league_donate(Message* msg);

	int fetch_league_member_list(Message* msg);//仙盟成员
	int league_appoint(Message* msg);//任命
	int league_kick(Message* msg);//踢出仙盟

	int league_leader_transfer(Message* msg);//盟主转换
	int impeach_league_leader();//弹劾盟主
	int impeach_vote(Message* msg);//投票弹劾
	int impeach_timeout();
	int check_impeach_time();//是否弹劾期间

	int fetch_league_apply_list(Message* msg);//申请列表
	int accept_league_apply(Message* msg);//接受申请
	int reject_league_apply(Message* msg);//拒绝申请

	int set_league_auto_accept(Message* msg);//设置自动接受
	int fetch_league_log(Message* msg);//仙盟日志

	int add_league_member_contri(int add_contri);//增加仙盟贡献
	int sync_league_member_contri(Message* msg);

	int fetch_league_skill_info();//仙盟技能
	int upgrade_league_skill(Message* msg);
	int check_skill_red_point();
	int send_skill_red_point(int skill_id, int value);	//技能升级红点

	// 帮派旗帜
	int fetch_league_flag_info();
	int upgrade_league_flag();
	int sync_flag_lvl_to_map();

	//league boss
	int fetch_league_boss_info();
	int feed_league_boss(Message* msg);
	int map_feed_league_boss(Message* msg);
	int summon_boss(Message* msg);
	int map_summon_boss(Message* msg);
	int request_enter_league_boss();
	int test_reset_boss(int boss_lvl);

	// league escort
	int open_escort_info();
	int select_escort_car_type(Message* msg);
    int refresh_escort_quality(Message* msg);

    //league_store
	int fetch_store();
	int fetch_apply_list(Message* msg);
	int sort_store();
	int insert_store(Message* msg);
	int apply_item(Message* msg);
	int check_apply(Message* msg);
	int fetch_apply_history(Message* msg);
	int open_store();
	int close_store();

	int insert_store_after(Message* msg);
	int lstore_insert_failed(Message* msg);
	int remove_apply(Int64 item_unique_id,int left_amount);
	int record_apply(int item_id,int item_num, Int64 item_unique_id);
	int send_lsrote_mail(PackageItem* item,Int64 role_id,int item_num);
	int record_apply_his(int item_id,int item_num,Int64 role_id,int opt,Int64 apply_id = 0);
	int remove_apply_by_id(Int64 apply_id);
	int apply_item_result(Message *msg);
    int sync_league_info();

    void update_leaguer_fashion();

    //帮派副本
    int request_lfb_wave_reward();
    int fetch_lfb_wave_reward(Message *msg);
    int request_lfb_cheer_info();
    int lfb_cheer_leaguer(Message *msg);
    int update_lfb_wave(Message *msg);
    int request_enter_lfb(Message *msg);
    int test_reset_lfb(); //命令重置帮派副本所有在线成员数据

    //领地战
    int fetch_league_region_info();
    int leader_draw_region_reward();
    int draw_league_region_reward(Message* msg);

public:
	League* league();
	League* find_league(Int64 league_index);

	LeaguerInfo& leaguer_info();
	LeagueMember* league_member();

	bool is_league_leader();

	bool can_apply_join_league();
	bool validate_league_join(Int64 role_index, bool single_flag);

	void handle_join_league(Int64 league_index);
	void handle_quit_league(int leave_type);
	void notify_join_league(Int64 role_index);
	void notify_quit_league(Int64 league_index, string role_name);
	void notify_change_leader(Int64 role_index);

	int fetch_league_shop_info(int shop_type);

	int send_map_league_info();
	int send_map_skill_info(const int enter_type = 0);
	int fetch_league_skill_add_attr(int prop_index, int skill_lvl);
	int send_map_flag_info(const int enter_type = 0);

	int update_apply_league_info();
	int notify_league_state(Int64 role_id, int league_state);

	int league_handle_player_task();
	void league_handle_player_levelup();
	void leaguer_check_open();

protected:
	void reset();
	void reset_every_day();
	void reset_lfb_wave_reward();
	void update_contri_to_client();

private:
	void upgrade_cur_contri(LeagueMember* member, int contri_num, int add_num = 0);
	void add_league_donate(int donate_type, int donate_number);

	void add_league_donate(Proto31400301* request);
	void check_and_brocast_league_donate(Proto31400301* request);
	void add_flag_attr(Proto30400011* request, int add_index, int attr, int is_add);
	void quit_league_miss_red_point();

	int fetch_wave_reward(const Json::Value& conf, int amount);
	int add_wave_reward(League* league, int wave, IntMap* reward_map, IntMap& get_reward, ProtoWaveReward *wave_record);
	void check_create_lfb_player(League* league);

private:
	LeaguerInfo leaguer_info_;
};

#endif /* LOGICLEAGUER_H_ */
