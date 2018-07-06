/*
 * LeagueStruct.h
 *
 *  Created on: Aug 8, 2013
 *      Author: peizhibi
 */

#ifndef LEAGUESTRUCT_H_
#define LEAGUESTRUCT_H_

#include "LogicStruct.h"
#include "GamePackage.h"

class League;
class ProtoLeagueItem;

class Proto31400301;
class Proto50100401;
class Proto50100644;
class ProtoCheerRecord;

//仙盟个人信息
struct LeaguerInfo
{
	IntVec skill_prop_set_;

	IntMap buy_map_;
	IntMap skill_map_;
	IntMap region_draw_;

	int open_;	//模块是否开启

    int draw_welfare_;
    int wand_donate_; //帮会令牌捐献
    int gold_donate_; //元宝捐献

    int send_flag_;
    //是否已领取
    int salary_flag_;

    int cur_contri_;
    int leave_type_;
    int store_times_;//物资申请次数

    Int64 leave_tick_;
    LongMap apply_list_;

    //帮派副本
    typedef std::map<int, IntMap> WaveRewardMap;
    WaveRewardMap wave_reward_map_;		//帮派副本宝箱领取情况

    // 攻城相关
    int day_admire_times_;          // 当日敬仰次数

    typedef std::map<int, IntMap> TypeItemNumberMap;
    TypeItemNumberMap type_item_num_map_;       // 每日各商店道具购买数量
    Time_Value day_siege_refresh_tick_;   // 每日刷新时间


    LeaguerInfo(void);
	void reset(void);

	IntMap *reward_map(int wave);
};

//仙盟成员
struct LeagueMember
{
	Int64 role_index_;
	Int64 join_tick_;
	Int64 offline_tick_;

	int role_sex_;
	int vip_type_;
	int role_lvl_;
	int role_force_;
	int new_role_force_;
	int role_career_;

	std::string role_name_;

	int league_pos_;
	int cur_contribute_;
	int today_contribute_;
	int total_contribute_;
	int today_rank_;	//今日贡献排名
	int total_rank_;	//历史贡献排名
	int offline_contri_;			//离线累积的

	int fashion_id_;
	int fashion_color_;

	Int64 lrf_bet_league_id;

	LeagueMember(void);
	void reset(void);

	int left_time(void);
};

//仙盟申请者
struct LeagueApplier
{
	Int64 role_index_;
	Int64 apply_tick_;

	std::string role_name_;

	int vip_type_;
	int role_lvl_;
	int role_career_;

	int role_force_;
	int role_sex_;

	LeagueApplier(void);
};

//帮派boss
struct LeagueBossInfo
{
	int boss_index_; 	//boss索引
	int boss_exp_;		//当前经验

	string super_summon_role_;	//超级召唤玩家
	Int64 reset_tick_;			//上一次更新时间
	Int64 normal_summon_tick_;	//普通召唤时间
	Int64 super_summon_tick_;	//超级召唤时间

	Int64 start_tick_;	//召唤开始时间
	Int64 end_tick_;	//召唤结束时间

	Int64 normal_die_tick_;
	Int64 super_die_tick_;

	int normal_summon_type_; 	//是否普通召唤中
	int super_summon_type_;		//是否超级召唤中

	LeagueBossInfo(void);

	void reset();
	void reset_everyday(); 	//每天重置
	void reset_activity_tick();
};

//仙盟日志
struct LeagueLogItem
{
	Int64 log_tick_;
	std::string log_conent_;

	LeagueLogItem(void);
};

struct ApplyInfo
{
	int item_id;
	int item_num;
	Int64 item_unique_id;
	Int64 role_id;
	string role_name;

	ApplyInfo(void);
    void reset(void);
};

struct ApplyHistory
{
	Int64 tick;
	int item_id;
	int item_num;
	int opt;
	string role_name;
	string checker_name;

	ApplyHistory(void);
    void reset(void);
};

struct LeagueStore
{
	GamePackage package;
	BIntLongMap lock_map;//value:-1 有物品，-2 有物品但被锁定 ==0 闲置， >0 有时间限制
	std::map<Int64,ApplyInfo> apply_map;
	std::vector<Int64> apply_vec;
	std::list<ApplyHistory> apply_history;
	LongSet view_list;//打开仓库的玩家
	std::map< Int64,LongSet > item_role_map;//申请该物品的玩家列表
	Int64 next_refresh_tick;

	LeagueStore(void);
    void reset(void);
    int search_empty_index_i(void);
    void apply_his_timeout(void);
    void luck_block(int index);
};

struct LeagueImpeach
{
	struct ImpeachTimer : public GameTimer
	{
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		League* league_;
	};

	Int64 impeach_role_;
	Int64 impeach_tick_;
	LongMap voter_map_;
	ImpeachTimer impeach_timer_;

	LeagueImpeach();
	void reset(void);
};

//助威鼓舞记录
struct CheerRecord
{
	Int64 role_id_;		//对方id
	int type_;			//1：助威 2：鼓舞
	int is_active_;		//是否主动发起 1是 0否
	int time_;			//今日0点开始的时间戳
	string name_;		//对方姓名

	CheerRecord();
	void reset();
	void serialize(ProtoCheerRecord *record);
};

//帮派副本玩家信息
struct LFbPlayer
{
	Int64 role_id_;		//角色id
	Int64 tick_;		//时间
	string name_;		//姓名
	int sex_;			//性别
	int wave_;			//通过波数
	int last_wave_;		//昨日通过波数
	int cheer_;			//助威次数
	int encourage_;		//鼓舞次数
	int be_cheer_;		//受助威次数
	int be_encourage_;	//受鼓舞次数

	typedef std::vector<CheerRecord> CheerRecordVec;
	CheerRecordVec record_vec_;

	LFbPlayer();
	void reset();
	void reset_every_day();
	int fetch_wave();
	int fetch_buff_num();
	int fetch_in_record(Int64 role_id, int type, int is_active);
	CheerRecord create_record(Int64 role_id, int type, int is_active, string name);
};

struct League
{
	enum
	{
		MIN_FORCE 		= 1000,
		MAX_FORCE 		= 999999,

		RECRUIT_TICK	=	300,
		END
	};
	typedef std::map<long, LeagueMember> MemberMap;
	typedef std::map<long, LeagueApplier> ApplierMap;
	typedef std::map<long, LFbPlayer> LFbPlayerMap;
	typedef std::list<LeagueLogItem> LeagueLogSet;

	//仙盟ID
	Int64 league_index_;

	//等级
	int league_lvl_;
	//贡献
	int league_resource_;
	//贡献来源细分
	IntMap resource_map_;

	//每个玩家战力总和
	int league_force_;
	//排名
	int league_rank_;

	//自动接受成员设置
	int auto_accept_;
	int accept_force_;

	//帮派旗帜
	int flag_lvl_;
	int flag_exp_;
	IntVec flag_set_;

	//盟主ID
	Int64 leader_index_;
	Int64 create_tick_;
	Int64 last_login_tick_;

	//每日帮贡总数
	Int64 league_daily_total_contribution;
	//昨日波数最高玩家id
	Int64 last_wave_player_;

	//仙盟名称及简介
	std::string league_name_;
	std::string league_intro_;

	//成员
	MemberMap member_map_;
	ThreeObjVec today_contri_rank_; //帮派今日贡献排名
	ThreeObjVec total_contri_rank_; //帮派总贡献排名
	//申请列表
	ApplierMap applier_map_;
	//仙盟操作日子
	LeagueLogSet league_log_set_;

	LeagueStore __lstore;
	//boss
	LeagueBossInfo league_boss_;

	//弹劾帮主
	LeagueImpeach league_impeach_;

	//帮派副本
	LFbPlayerMap lfb_player_map_;
	ThreeObjVec lfb_rank_vec_;

	//领地
	int region_rank_;
	Int64 region_tick_;
	int region_leader_reward_;

	League(void);
	void reset(void);
	void reset_everyday(bool force_reset = false);

	int leader_vip();
	int leader_career();
	int leader_sex();
	std::string leader_name();

	long fetch_next_leader();
	const Json::Value& set_up();

	bool league_full();

	bool is_can_dismiss();
	bool is_arrive_max_level();
	bool is_leader(long role_index);
	bool is_arrive_max_contri(int value, int source);

	bool can_view_apply(long role_index);
	bool can_operate_league(long role_index);
	bool online_flag(Int64 role_index);
	bool have_region_reward();

	//自动接收
	bool auto_accept();
	bool arrive_auto_accept(int level);
	bool arrive_auto_accept(Int64 role_id);

	void dismiss_league();
	void caculate_league_force();

	//处理加入/退出仙盟
	void handle_join_league(long role_index, int league_pos);
	void handle_quit_league(long role_index, int leave_type);

	int validate_join(long role_index);
	int validate_member(long role_index);

	int member_pos(long role_index);
	int transfer_leader(long role_index);
	int league_pos_count(int league_pos);
	int fetch_max_contri(int source);

	LeagueMember* league_member(long role_index);

	//申请
	void push_applier(long role_index);
	void erase_applier(long role_index);
	bool validate_applier(long role_index);

	void sync_map_league_info();
	void add_league_resource(long role_index, int contribute, int source = 0);

	void fetch_sort_member(PairObjVec& role_set);
	void fetch_sort_applier(ThreeObjVec& applier_set);
	void make_up_league_list_info(long role_index, ProtoLeagueItem* league_item);
	void make_up_league_list_info(ProtoLeagueItem* league_item);

	//帮贡排名
	void sort_rank();

	//日志
	void add_league_log(const LeagueLogItem& log_item);
	void add_league_member_log(int log_type, const string& role_name);
	void add_league_member_log(int log_type, const string& first_name,
			const string& second_name);
	void add_league_donate_log(int log_type, int donate_value, const string& role_name);

	void change_name_log(int log_type, const string& role_name);

	void check_leader_offline();
	void check_leader_validate();
	void check_and_upgrade_league();

	//弹劾帮主
	void impeach_leader(const Int64 role_id);
	void impeach_voter(const Int64 role_id, int vote_type);
	void handle_start_impeach();
	int handle_timeout_impeach();
	bool check_impeach_role_leave();
	bool cal_impeach_enough();
	bool is_in_impeach();
	bool is_vote(const Int64 role_id);
	int impeach_need_num();
	int impeach_now_num();

	//帮派副本
	int fetch_wave_player(int wave);
	LFbPlayer *lfb_player(Int64 role_id);
	void sort_lfb_player();
	void set_max_last_wave_player();

	void notify_store();

	//领地战
};

#endif /* LEAGUESTRUCT_H_ */
