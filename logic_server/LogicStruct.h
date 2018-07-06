/*
 * LogicStruct.h
 *
 * Created on: 2013-01-09 11:25
 *     Author: glendy
 */

#ifndef _LOGICSTRUCT_H_
#define _LOGICSTRUCT_H_

#include "Heap.h"
#include "PubStruct.h"
#include "GameCommon.h"

class Proto30100101;
class Proto80400348;
class Proto80400349;
class ProtoShusanBoss;
class ProtoRoleInfo;
class ProtoServerRecord;
class ProtoPersonRecord;
class ProtoActRankInfo;

namespace GameEnum
{
	enum TEAM_ENUM
	{
		TEAM_SET_AUTO_INVITE		= 1,		// 自动接收组队邀请
		TEAM_SET_AUTO_APPLY			= 2, 		// 自动接收入队申请
		TEAM_SET_ALLOW_INVITE		= 3, 		// 允许队员邀请

		DISMISS_TEAM_COUNT			= 1,		// 解散队伍
		TEAM_OFFLINE_TIME			= 60,		// 队伍最长离线时间

		TEAMER_PUSH_FRONT			= 0,		// 队前
		TEAMER_PUSH_BACK			= 1,		// 队尾

		TEAM_OPER_INVITE			= 0,		// 组队邀请
		TEAM_OPER_APPLY				= 1,		// 申请入队
		TEAM_OPER_FB_APPLY			= 2,		// 申请进入对方组队队伍
		TEAM_OPER_FB_INVITE			= 3,		// 副本邀请入队
		TEAM_OPER_TEAMER_INFO		= 4,		// 获取队伍信息
		TEAM_OPER_SYNC_FB_TIMES		= 5,		// 同步所有队友副本次数
		TEAM_OPER_DELAY_SIGN_IN		= 6,		// 延迟登录后的次数同步

		TEAM_OPER_REJECT			= 0,
		TEAM_OPER_AGREE				= 1,
		TEAM_OPER_FULL				= 2,

		MAX_TEAM_DISTANCE			= 100,		// 附近队伍最长距离

		TEAM_TEAM_SIGN				= 0,
		TEAM_PLAYER_SIGN			= 1,

		TEAM_SORT_TEAM_FIRST		= 1,
		TEAM_SORT_PLAYER_FIRST		= 2,

		TEAM_SIGN_PAGE_COUNT		= 10,
		TEAM_NEAR_PAGE_COUNT 		= 10,

		TEAMER_ONLINE				= 1,		// 在线队员
		TEAMER_OFFLINE				= 2,		// 离线队员
		TEAMER_REPLACEMENT			= 3,		// 化身

		TEAM_STATE_NORMAL			= 1,		// 普通队伍
		TEAM_STATE_FB_ORG			= 2,		// 进行副本组队
		TEAM_STATE_FB_INSIDE		= 3,		// 队伍在副本中

		TRVL_TEAM_PAGE_COUNT		= 5,		// 跨服队伍列表每页数量

		TEAM_COUPLE_FB				= 20701,	// 夫妻副本id

		TEAM_ENUM_END
	};

	enum LEAGUE_ENUM
	{
		LEAGUE_LIST_PAGE_COUNT		= 6,		//宗派每页多少条
		LEAGUE_MEMBER_PAGE_COUNT 	= 10,		//成员每页多少条
		LEAGUE_LOG_PAGE_COUNT 		= 10,		//日志每页多少条
		LSTORE_PAGE_APPLY_COUNT 	= 10,		//仓库申请列表每页多少条
		LSTORE_PAGE_APPLY_HIS_COUNT = 10,		//仓库申请历史每页多少条
		LSTORE_PAGE_CHECK_HIS_COUNT = 30,		//仓库申请历史每页多少条

		LEAGUE_DEGRADE_DAYS			= 3,		//维护费用3天不足，降级
		LEAGUE_MAX_APPLY_LIST		= 6,		//申请列表

		DEFAULT_LEAGUE_LVL			= 1,		//铜钱创建1级宗派
		DEFAULT_HGIH_LEAGUE_LVL		= 2,		//道具或元宝创建2级宗派

		LEAGUE_SKILL_TIMES			= 2,

		LEAGUE_ACTIVITY_BONFIRE		= 1,		//宗派篝火活动
		LEAGUE_ACTIVITY_GUARD		= 2,		//仙盟守卫

		LEAGUE_ACTIVTIY_START		= 1,		//开始
		LEAGUE_ACTIVITY_STOP		= 2,		//结束

		LEAGUE_STATE_JOIN			= 1,
		LEAGUE_STATE_KICK			= 2,

		LEAGUE_MAX_RANK				= 1000,


		LEAGUE_APPLY_NO				= 0,		//没有申请
		LEAGUE_APPLY_YES			= 1,		//已申请
		LEAGUE_FULL					= 2,		//宗派已滿

		LEAGUE_SHOP_ITEM			= 0,		//宗派首具
		LEAGUE_SHOP_EQUIP			= 1,		//宗派装备

		MAX_LEAGUE_LEADER_OFFLINE	= 7,		//宗住最多7天离线
		MAX_LEAGUE_INTRO_LENGTH		= 88,		//宗派简介

		LEAGUE_LEADER_COUNT			= 1,		//宗主个数
		LEAGUE_LOG_MAX_COUNT		= 100,		//日志最多数目

		//宗派职位
		LEAGUE_POS_NONE				= 0,		//普通成员
		LEAGUE_POS_EXCELLENT 		= 10,		//精英
		LEAGUE_POS_ELDER 			= 20,		//长老
		LEAGUE_POS_DEPUTY			= 30,		//副宗主
		LEAGUE_POS_LEADER			= 40,		//宗主

		//宗派日志
		LEAGUE_LOG_MEMBER			= 1,		//成员变更
		LEAGUE_LOG_DONATE			= 2,		//捐献

		//成员变更
		LEAGUE_LOG_MEMBER_JOIN		= 101,		//加入宗派
		LEAGUE_LOG_MEMBER_QUIT		= 102,		//离开宗派
		LEAGUE_LOG_MEMBER_QUITED	= 104,		//被踢出宗派
		LEAGEU_LOG_LEADER_TRANSFER	= 105,		//转让宗主
		LEAGUE_LOG_APPOINT_DEPUTY	= 106,		//提升副宗主


		//捐献
		LEAGUE_LOG_DONATE_WAND		= 201,		//铜钱
		LEAGUE_LOG_DONATE_GOLD		= 202,		//元宝

		//捐献
		LEAGUE_LOG_CHANGENAME		= 301,		//改名

		LEAGUE_END
	};
}

struct AccountDetail
{
    int __role_index;
    int __role_amount;
    int __client_sid;

    struct RoleInfo
    {
        bool __active;
       
        int64_t __id;
        char __name[MAX_NAME_LENGTH + 1];
        int __level;
        int __sex;
        int __career;
        int __scene_id;
        int __pos_x;
        int __pos_y;

        void reset(void);
    };
    RoleInfo __role_list[MAX_ROLE_AMOUNT];

    void reset(void);
};

struct CumulativeLoginDetail
{
	CumulativeLoginDetail();
	void 	reset();
	int 	__single;
	int 	__ten;
	int 	__hundred;
	int 	__multiple;
	int		__cumulative_day;
	//是否开启状态
	int 	__single_state;
	int 	__ten_state;
	int 	__hundred_state;
	int 	__multiple_state;
	int		get_reward();
	int		get_multiple_reward();
	int 	check_detail_by_day(int day);
	void  	set_all_state(bool state);
};

//全服记录
struct ServerRecord
{
	Int64 player_id_;		//玩家id
	string player_name_;	//玩家名字
	Int64 get_time_;		//获取时间
	int cornucopia_gold_;	//聚宝盘金额
	int reward_mult_;		//奖励倍数

	ServerRecord();
	void reset();
};
typedef std::map<Int64, ServerRecord> ServerRecordMap;

//聚宝盆任务
struct CornucopiaTask
{
	int task_id_;				//任务id
	string task_name_;			//任务名字
	int completion_times_;		//完成次数
	int total_times;			//总次数
	CornucopiaTask();
	void reset();
};
typedef std::map<int, CornucopiaTask>  CornucopiaTaskMap;

struct ServerItemRecord
{
	Int64 player_id_;		//玩家id
	string player_name_;	//玩家名字
	Int64 get_time_;		//获取时间
	int item_id_;			//物品id标识
	int amount_;			//数量
	int item_bind_;			//绑定状态

	int sub_value_;
	ServerItemRecord();
	void reset();
};
typedef std::vector<ServerItemRecord> ServerItemSet;

struct SpecialBoxItem
{
	SpecialBoxItem();
	void init();

	int slot_id_;
	string name_;
	ItemObj item_obj_;
	int min_times_;
	int max_times_;
	int weight_;
	int is_shout_;
	int server_record_;
};
typedef std::map<int, SpecialBoxItem> SpecialBoxItemMap;
typedef std::map<int, SpecialBoxItemMap> DayBoxItemMap;

struct ChangeItem
{
	ChangeItem();
	void init();
	int slot_id_;
	int group_;
	int page_;
	string name_;
	ItemObj change_item_;
	ItemObj need_item_;
	int score_;
};
typedef std::map<int, ChangeItem> ChangeItemMap;
typedef std::map<int, ChangeItemMap> DayChangeItemMap;

struct LogicRoleDetail : public BaseRoleInfo
{
	LogicRoleDetail(void);

    int __camp_id;
    int prev_draw_;

    int fashion_id_;
    int fashion_color_;

    Int64 view_tick_;
    Int64 draw_tick_;
    Int64 day_reset_tick_;
    Time_Value week_reset_tick_;

    IntMap buy_map_; //每天刷新
    IntMap buy_total_map_; //每天不刷新
    ThreeObjMap mount_info_;	//key:战骑类型, id:战骑类型, value:阶数, tick:外形

    int panic_buy_notify_;	// 开服抢购的系统提示
    int cur_gold_; //当前充值金额

    int kill_num_;	// 杀人数
    int killing_value_;	// 杀戳值
    int kill_normal_;//击杀普通玩家（每周刷新）
    int kill_evil_;//击杀恶人数（每周刷新）
    LongSet is_worship_; //是否膜拜 每日更新
    IntSet brother_reward_index;

    int today_recharge_gold_; //今日充值金额
    int today_consume_gold_;
    int today_market_buy_times_;
    int today_total_buy_times_;	//扭蛋总次数
    int today_can_buy_times_;	//扭蛋今日可抽的次数
    int draw_flag(Int64 start_tick);

    CumulativeLoginDetail cur_day_detail_;	//累计登录

	CornucopiaTaskMap cornucopia_task_map_;		//聚宝盆任务记录
	int mail_reward_ratio_;					//邮件奖励比例
	IntMap reward_stage_map_;					//聚宝盆各阶段奖励领取状况

	int cur_red_packet_group_;				//当前红包组
	int continuity_login_day_;				//累计登录天数
	int continuity_login_flag_;
	int cumulative_times_;					//活动期间累计次数

	int act_data_reset_flag_;				//用于五一类型活动结束时，清除活动时道具使用 0为未清除 其他为已清除
	int last_act_type_;						//这里用于记录上一次活动的主类型
	int last_act_end_time_;						//用于记录上一次活动的时间

    void reset(void);

};

struct LogicHookDetail
{
	int __is_hooking;
	void reset(void);
};

struct BackstageBrocastRecord
{
	int __index;
	int __brocast_type;
	Int64 __brocast_tick;
	int __brocast_times;
	int __max_repeat_times;
	int __interval_sec;

	std::string __content;

	BackstageBrocastRecord(void);
	void reset(void);
};

struct CustomerServiceRecord
{
	enum CustomerServiceRecordType
	{
		RECORD_TYPE_BUG            =      1,
		RECORD_TYPE_COMPLAIN       =      2,
		RECORD_TYPE_SUGGEST        =      3,
		RECORD_TYPE_OTHER          =      4,

		RECORD_TYPE_END
	};

	enum
	{
		CUSTOMER_SERVICE_RECORD_TITLE_LENGTH             =      20,
		CUSTOMER_SERVICE_RECORD_CONTENT_LENGTH           =      200,
	};

	int __record_type;
	int __has_replay;
	int __has_read;
	Int64 __record_id;
	Int64 __sender_id;
	Int64 __send_tick;
	Int64 __evaluate_tick;
	int __evaluate_level;
	std::string __sender_name;
	std::string __content;
	std::string __title;
	std::string __replay_content;
	int __sender_level;
	int __server_code;
	int __platform;
	int __agent;
	int __recharge_gold;

	int __evaluate_star;
	int __opinion_index; //官方反馈序号

	int serialize(ProtoCustomerSVCRecord& proto_record);
	int serialize(ProtoCustomerSVCRecord* proto_record);
	int unserialize(const ProtoCustomerSVCRecord& proto_record);

	CustomerServiceRecord();
	void reset();
};

typedef std::map<Int64, CustomerServiceRecord*> CustomerRecordMap;
typedef std::vector<CustomerServiceRecord*> CustomerRecordVec;

struct CustomerServiceDetail
{
	int __last_summit_type;
	int __unread_amount;

	std::string __content;
	std::string __title;

	CustomerRecordVec __customer_record_vec;
	CustomerRecordMap __customer_record_map;

	IntMap __opinion_reward; 	//官方反馈状态

	CustomerServiceDetail();
	void reset();
	void fetch_record_map(LongMap& index_map);
};

enum
{
	FRIEND_TYPE_NULL = 0,
	FRIEND_TYPE_CLOSE = 1,
	FRIEND_TYPE_STRANGER = 2,
	FRIEND_TYPE_BLACK = 3,
	FRINED_TYPE_ENEMY = 5,
	FRINED_TYPE_NEARBY = 6,


	FRIEND_RECOMMEND_LEVEL_UP = 1,
	FRIEND_RECOMMEND_LEVEL_DOWN = 2,
	FRIEND_RECOMMEND_PER_NUM = 5,
	FRIEND_RECOMMEND_LEVEL_GAP = 5,
	FRIEND_RECOMMEND_WATER_FLOW = 70,

	MAX_RECOMMEND_NUM = 20,
	MAX_STRANGER_FRIEND_NUM = 30,
	MAX_BLACK_LIST_SIZE = 50,
	MAX_CLOSE_FRIEND_NUM = 200,
	MAX_ENEMY_NUM = 100,
	MAX_NEARBY_NUM = 100,

	MAX_SERVER_RECORD_NUM = 100,
	MAX_PERSON_RECORD_NUM = 200,

	FRIEND_TYPE_END
};

struct LogicSocialerDetail
{
	struct ApplyInfo
	{
		Int64 friend_id_;
		Int64 league_id_;
		std::string friend_name_;
		std::string league_name_;
		int level_;
		int sex_;
		Int64 tick_;

		void reset(void);
	};
	typedef std::map<Int64, ApplyInfo> ApplyInfoMap;
	ApplyInfoMap __apply_map;

	LongMap __friend_list;
	LongMap __stranger_list;
	LongMap __black_list;
	LongMap __enemy_list;
	LongMap __nearby_list;

	int open_;

	LogicSocialerDetail();
	void reset(void);
};

typedef std::map<int, LongPair> FriendPair;

struct FriendPairInfo
{
	enum
	{
		FRIEND_ADD 	  = 1,
		FRIEND_DELETE = 2,

		END
	};

	FriendPair friend_map_;
	FriendPair delete_map_;

	FriendPairInfo();
	void reset(void);
};

struct LogicVipDetail
{
	int	__type;
	int __period_time;
	LogicVipDetail(void);
	void reset(void);
};

struct BoxRecord   //藏宝库系统记录
{
    int64_t __role_id;
    string __name;
    int __time;
    int __item_id;
    int __item_amount;

    void reset(void);
    BoxRecord(void);
};
typedef std::list<BoxRecord*> BoxRecordList;

// 全服统一的活动记录
struct ActivityTipsInfo
{
	int __activity_id;
	int __acti_state;

	Int64 __start_time;   // 当前的开始时间
	Int64 __end_time;     // 当前的结束时间
	Int64 __reset_time;		// 将结束状态重置为未开始的时间
	Int64 __update_tick;	//更新时间
	int __open_day_type;
    int __open_day;   // 开放时间
    int __day_check;	// 几天后每天检查
    int __notify_cfg;   // 更新配置后通知客户端活动状态

    int __limit_level;	//等级限制
    int __time_dynamic;	//时间不通过活动配置控制
    int __active_time;	//活动时长
    int __sub_value;	//附加值

    ActivityTimeInfo time_info_;

	ActivityTipsInfo();
	const Json::Value& conf();

	void reset();
    void serialize(Message *msg);
};

// 个人玩家的活动图标记录
struct ActivityRec
{
    int __activity_id;
    Int64 __start_tick;
    Int64 __end_tick;
    Int64 __refresh_tick;
    Int64 __update_tick;
	int __finish_count;     // 活动时间段内完成次数
    int __is_touch;         // 是否已点击活动图标

    ActivityRec(void);
	void reset(void);
};
typedef std::map<int, ActivityRec> ActivityInfoMap;

/*
 * 每一秒定时器
 * */
class LogicOneSecTimer : public FixedTimer
{
public:
	LogicOneSecTimer();
    virtual int handle_timeout(const Time_Value &tv);
};

/*
 * 每十秒定时器
 * */
class LogicTenSecTimer : public FixedTimer
{
public:
	LogicTenSecTimer();
    virtual int handle_timeout(const Time_Value &tv);
};

/*
 * 每一分钟定时器
 * */
class LogicOneMinTimer : public FixedTimer
{
public:
	LogicOneMinTimer();
    virtual int handle_timeout(const Time_Value &tv);

    void check_online_player(const Time_Value &tv);
};

/*每一小时定时器*/
class LogicOneHourTimer : public FixedTimer
{
public:
	LogicOneHourTimer();
	virtual int handle_timeout(const Time_Value &tv);
};

/*
 * 整分钟定时器，由于时间间隔，可能会出现1秒相差
 * */
class LogicIntMinTimer : public GameTimer
{
public:
	int schedule_timer();

	virtual int type(void);
	virtual int handle_timeout(const Time_Value &tv);
};


/*
 * 每天晚上12点定时器，由于时间间隔，可能会出现最长不超过1分钟的相差
 * */
class LogicMidNightTimer : public GameTimer
{
public:
	LogicMidNightTimer();
	int schedule_timer();

	virtual int type(void);
	virtual int handle_timeout(const Time_Value &tv);

	Int64 nextday_zero_;
};

struct GameNoticeDetial
{
	GameNoticeDetial();

	int notify_;
	Int64 tick_;

	string title_;
	StringVec content_;

	Int64 start_tick_;
	ItemObjVec item_set_;
};

struct ArenaRole
{
	enum
	{
		MAX_COUNT = 3,
		END
	};

	int rank_;
	int is_skip_; //是否跳过动画
	int open_flag_;
	int notify_flag_;
	IntVec fight_set_;

	Int64 id_;
	string name_;

	Time_Value refresh_tick_; //每日刷新
	int left_times_; //剩余次数
	int buy_times_; //购买次数

	Int64 next_fight_tick_; //下次挑战时间
	int is_over_limit_;
	Int64 last_view_tick_;

	int sex_;
	int force_;
	int level_;
	int reward_level_;
	int career_;

	int wing_level_;
	int solider_level_;

	int last_rank_;
	int reward_id_;

	int is_fighting_;
	int area_index_;
	int heap_index_;
	int continue_win_;

	int weapon_;
	int clothes_;
	int fashion_weapon_;
	int fashion_clothes_;

	struct Record
	{
		Int64 tick_;
		int fight_type_;// 0: active, 1: passive
		int fight_state_;//0: lose, 1: win

		string name_;
		int rank_;
		int rank_change_;// 0: false, 1: true
	};

	std::list<Record> his_record_;

	ArenaRole();

	void push_active_record(const string& name, int state, int rank_change);
	void push_passive_record(const string& name, int state, int rank_change);
	void push_record(const Record& record);

	void reset();
	void reset_everyday();

	int have_reward();
	int left_cool_time();
};

struct AreaFightNode : public HeapNode
{
	int area_index_;
	int guide_flag_;

	Int64 first_id_;
	Int64 second_id_;

	Int64 start_tick_;
	Int64 max_finish_tick_;

	void reset();
};

class AreaFightNodeCmp
{
public:
	bool operator()(AreaFightNode *left, AreaFightNode *right)
	{
		return left->max_finish_tick_ < right->max_finish_tick_;
	}
};

typedef Heap<AreaFightNode, AreaFightNodeCmp> AreaFightHeap;

struct AreaSysDetail
{
	enum
	{
		MIN_RANK			= 5,
		TOTAL_FIGHTER		= 4,
		MAX_UP				= 600,
		WAIT_TIME			= 10,
		BROCAST_TICK		= 8,
		END
	};

	typedef std::map<Int64, ArenaRole> AreaRoleMap;
	typedef std::map<int, AreaFightNode*> ArenaFightMap;

	int re_rank_;
	Int64 last_bot_id; //最弱机器人ID
	Int64 timeout_tick_;
	AreaRoleMap arena_role_set_;

	LongMap robot_id_map;
	LongMap rank_robot_map;
	LongMap rank_set_;      	//排名，玩家
	AreaRoleMap role_map_;		//玩家ID，玩家

	ArenaFightMap fight_map_;
	AreaFightHeap fight_heap_;

	LongMap same_player_map_;	//出bug的玩家集合，只修复一次

	AreaSysDetail();
	void reset();
	bool is_same_player(int rank);
};

////////////////////////////////////
struct RechargeAwardItemDetail
{
	RechargeAwardItemDetail(void);
	void reset(void);

	int __rank_pos;
	int __award_sort;
	int __award_value;

	std::string __serial_code;
};
typedef std::map<int , RechargeAwardItemDetail> RechargeAwardItemMap;

struct RechargeRankItem
{
	RechargeRankItem(void);
	void reset(void);
	Int64 __role_id;

	std::string __role_name;
	std::string __serial_code;
	std::string __announce_des;

	int __rank_pos;
	int __award_sort;
	int __award_value;
	int __award_valid;
};

struct RechargeRankExtraAwarder
{
	RechargeRankExtraAwarder(void);
	void reset(void);

	Int64 __role_id;

	int __sex;
	int __carrer;
	int __weapon;
	int __clothes;
	int __fashion_weapon;
	int __fashion_clothes;
	int __label;
	int __timestamp;

	std::string __role_name;
	std::string __client_title;
	std::string __announce_des;
};

typedef std::map<int, RechargeRankItem* > RechargeRankItemMap;

struct RechargeTimeRankDetail
{
	RechargeTimeRankDetail(void);
	void reset_rank_player_map(void);
	void reset_extra_awarder(void);

	void reset(void);

	int __activity_ongoing;
	int __activity_op_code;
	int __activity_version;

	int __update_award_config;
	int __update_extra_awarder;

	Time_Value __update_time;
	Time_Value __start_time;
	Time_Value __end_time;

	std::string __award_mail_des;
	std::string __award_mail_des_extra;
	std::string __award_announce;
	std::string __award_announce_extra;
	std::string __extra_awarder_title;

	RechargeAwardItemMap __recharge_award_item_map;
	RechargeRankItemMap __recharge_rank_player_increment_map;
	RechargeRankItemMap __recharge_rank_player_map;
	RechargeRankItemMap __recharge_rank_awarder_map;

	RechargeRankExtraAwarder __extra_awarder;
	RechargeRankExtraAwarder __extra_awarder_bk;
};

struct ShopMonitorDetail
{
	IntMap limited_map_;
	IntMap limited_total_map_;
};

// 用于收集BOSS系统的怪物信息，方便活动面板信息的获取
struct BossDetail
{
    Int64 __boss_id;
    int __boss_sort;
    int __boss_level;
    Time_Value __born_tick;

    Int64 __last_award_role_id;
    std::string __last_award_role_name;

    int __floor;    // boss_type
    int __scene_id;
    MoverCoord __born_point;
    int __space_id;

    BossDetail(void);
    void reset(void);

    void serialize(ProtoShusanBoss *proto_boss);
};

extern bool boss_cmp(const BossDetail *left, const BossDetail *right);

struct WeddingDetail    // 结缘信物信息
{
    Int64 __wedding_id;
    Time_Value __wedding_tick;  // 结婚日期

    struct WeddingRole
    {
        Int64 __role_id;
        std::string __role_name;
        int __sex;
        int __career;
        int __sweet_degree;			// 伴侣甜蜜度 （结婚才有）
        int __ring_level;			// 伴侣戒指等级
        int __sys_level;			// 伴侣姻缘等级
        int __tree_level;			// 伴侣爱情树等级

    	Int64 __tick;		//购买时间
    	Int64 __fetch_tick;	//每日奖励领取时间
    	int __once_reward;	//一次性奖励 是否领取
    	int __left_times;		//剩余次数
        void reset(void);
    };
    WeddingRole __partner_1;    // 伴侣1
    WeddingRole __partner_2;    // 伴侣2

    int __day_wedding_times;        // 当天已举办次数
    Time_Value __day_refresh_tick;  // 每举办婚礼次数刷新时间

    int __intimacy;             // 未结婚前亲密度保存在玩家各自关系中，结婚后保存在结婚信物信息表
    int __history_intimacy;     // 历史最高亲密值
    int __wedding_type;         // 信物类型
    int __keepsake_id;          // 信物ID
    int __keepsake_level;       // 信物等级
    int __keepsake_sublevel;    // 信物子等级（多少次满进度）
    double __keepsake_progress; // 信物进度

    Time_Value __wedding_cartoon_tick;  // 结婚动画播放时间
    Time_Value __cruise_tick;   // 巡游超时时间（防止CORE掉进状态不改变）

    void reset(void);
};

struct PlayerWeddingDetail
{
    typedef std::map<Int64, int> RoleValueMap;
    RoleValueMap __intimacy_map;    // key: role_id, value: intimacy

    RoleValueMap __wedding_reply;   // key: role_id, value: reply

    Int64 __wedding_id;                 // 结婚后的ID
    int __total_recv_flower;        // 收到的花数量
    int __total_send_flower;        // 发出的花数量
    int __act_total_recv_flower;	// 活动期间收到的花数量
    int __act_total_send_flower;    // 活动期间发出的花数量

    int __side_fashion_id;			//对象时装id
    int __side_fashion_color;		//对象时装color
    int __is_has_ring;				//是否拥有婚戒
    //结婚开启
    struct wedding_property
    {
    	int __level;	//等级
    	int __exp;		//经验

    	int __side_order;	//双方等阶
    	int __side_level;	//对方等级
    	void reset(void);
    };

    typedef std::map<int, wedding_property> WeddingProMap;
    WeddingProMap __wedding_pro_map;
    IntMap __wedding_label_map;
    void reset(void);
};

struct ItemRate
{
	int item_id;
	int item_num;
	int item_bind;
	double item_weight;
};

struct FishInfo
{
	int type_;
	int layer_;
	int flag_;
	MoverCoord coord_;

	FishInfo();
	void reset();
};
typedef std::vector<FishInfo> FishInfoVec;

//轮盘类活动
struct LuckyWheelActivity
{
	//活动索引枚举
	enum
	{
		ACT_LUCKY_WHEEL		= 50101,	//幸运大转盘
		ACT_GOLD_BOX		= 50201,	//元宝宝匣
		ACT_ADVANCE_BOX		= 50301,	//进阶宝匣
		ACT_LIMIT_BUY  		= 50401,	//限时秒杀
		ACT_CABINET			= 50501,	//珍藏阁
		ACT_COUPLE			= 50601,	//我们结婚吧
		ACT_MAZE			= 50701,	//迷宫寻宝
		ACT_ANTIQUES		= 50801,	//神仙鉴宝
		ACT_NINE_WORD		= 50901,	//密宗九字
		ACT_LUCKY_EGG		= 51001,	//幸运砸蛋
		ACT_DOUBLE_REBATE	= 51101,	//两倍返利
		ACT_DAILY_GET		= 51201,	//每日充值榜
		ACT_DAILY_COST		= 51301,	//每日消费榜
		ACT_RECHARGE_REBATE = 51401,	//充值返元宝
		ACT_CABINET_DISCOUNT= 51501,	//珍藏阁折扣店
		ACT_GASHAPON		= 51601,	//七彩扭蛋
		ACT_GEM_SYNTHESIS	= 51701,	//宝石合成
		ACT_FISH			= 51801,	//深海捕鱼
		ACT_GODDESS_BLESS	= 51901,	//女神赐福

		SERVER_DATE			= 1,	//按开服时间
		REAL_DATE			= 2,	//按真实时间

		SHOP_SLOT_NUM		= 9,
		SHOP_LAST_GROUP		= 4,

		WEDDING_RANK_PAGE 	= 10,
		RANK_PAGE 			= 15,

		SIX_WORD_REWARD_NUM = 6,	//最少点亮字数奖励
		NINE_WORD_NUM 		= 9,	//密宗九字个数

		SLOT_NOT_OPEN 		= 0,	//九字格子未开
		SLOT_NOT_WORD 		= 1,	//九字格子非文字
		SLOT_IS_WORD		= 2,	//九字格子是文字
		SLOT_WORD_END,

		RANK_TYPE_NUM 		= 1,	//密宗榜(也包括其他活动的排行榜)
		RANK_TYPE_LUCKY 	= 2,	//密宗幸运榜

		END
	};

	//次数转换奖励 或者 幸运砸蛋物品表
	struct ChangeReward
	{
		int first_;		//比较左值 或者 幸运砸蛋概率1
		int second_;	//比较右值 或者 幸运砸蛋概率2
		int item_id_;
		int amount_;
		int bind_;
		int weight_;	//权重

		ChangeReward();
		void reset();
	};
	typedef std::vector<ChangeReward> ChangeVec;

	//格子信息
	struct SlotInfo
	{
		int slot_id_;		//格子id
		int index_;			//配置表索引id
		int the_weight_;	//权重
		int pool_percent_;	//奖池百分比
		int person_record_;	//是否记录个人
		int server_record_;	//是否记录全服
		int is_shout_;		//是否播报
		int is_precious_;	//是否贵重物品（双倍奖励）
		int appear_time_;	//奖励出现次数（少于该次数权重为0）
		int person_limit_; 	//个人限购次数
		int server_limit_;	//全服限购次数
		int pre_cost_;		//原价
		int now_cost_;		//现价
		IntVec must_appear_;//奖励必现次数（等于该次数必定出现该奖励）
		int day_;			//天数

		int server_buy_; 	//全服购买次数

		IntMap rand_list_; 	//类型抽奖权重
		int reward_mult_;	//奖励倍数

		ItemObj item_obj_;	//奖励
		ChangeVec change_reward_;	//转换奖励表id
		PairObjVec rand_amount_;	//奖励数量概率
		ChangeVec normal_item_vec_;	//普通蛋物品表
		ChangeVec special_item_vec_;//彩蛋物品表

		//商店信息
		string item_name_;
		int item_price_;
		int group_weight_;
		int group_id_;
		int is_rarity_;
		int is_cast_;

		//迷宫寻宝
		int slot_type_;			//格子类型
		int slot_num_;			//格子边界
		int slot_clear_;		//获得是否清空
		int item_min_times_;	//物品最小出现次数
		int item_max_times_;	//物品最大出现次数
		int item_show_weight_;	//同时出现权重
		int clean_bless_;		//是否清空祝福值

		//神仙鉴宝
		int two_same_times_;	//2个必出次数
		int three_same_times_;	//3个必出次数

		IntVec refresh_weight_;	//刷新出现概率
		IntVec other_;			//补漏
		IntVec max_count_;			//同一界面出现最大个数
		int score_;				//捕鱼积分
		IntVec layer_;			//层数
		int fish_type_;			//鱼类型

		ItemObj need_item_obj_;
		int frequency_limit_;
		int is_rare_;

		SlotInfo(void);
		void reset(void);
	};
	typedef std::map<int, SlotInfo> SlotInfoMap;
	typedef std::vector<SlotInfo> SlotInfoVec;
	struct ServerItemInfo
	{
		Int64 player_id_;		//玩家id
		string player_name_;	//玩家名字
		Int64 get_time_;		//获取时间
		int item_id_;			//物品id标识
		int amount_;			//数量
		int item_bind_;			//绑定状态

		int reward_mult_;		//奖励倍数	//元宝宝匣倍数
		int sub_value_;
		ServerItemInfo();
		void reset();
	};
	typedef std::vector<ServerItemInfo> ServerItemSet;

	struct RankPlayer
	{
		Int64 role_id_;
		string name_;
		int sex_;

		RankPlayer();
		void reset();
	};

	struct OneRankInfo
	{
		int rank_;
		int num_;
		Int64 tick_;
		Int64 role_id_;
		string name_;

		OneRankInfo();
		void reset();
		void serialize(ProtoActRankInfo *rank_info);
		void add_rank_item(const BSONObj& res);
	};
	typedef std::map<Int64, OneRankInfo> RankNumMap;
	typedef std::map<Int64, OneRankInfo> RankLuckyMap;

	struct RoleMailInfo
	{
		Int64 role_id_;
		IntMap reward_map_;

		RoleMailInfo();
		void reset();
	};
	typedef std::map<Int64, RoleMailInfo> RoleMailMap;

	struct RefreshReward
	{
		int vip_level_;
		int refresh_times_;
		int refresh_reward_;
	};
	typedef std::map<int, RefreshReward> RefreshRewardMap;

	struct GemSynthesisInfo
	{
		int type_;
		//第一个元素是ID,第二个元素是数量
		IntVec need_gem_;
		IntVec synthesis_gem_;
		IntVec reward_gem_;
	};
	typedef std::vector<GemSynthesisInfo> GemSynthesisInfoVec;

	struct FishConfigDetail
	{
		int type_;
		IntVec layer_;
		int max_count_;
		IntVec weight_;
		IntVec other_;
		int score_;
		int person_record_;
		int server_record_;
	};

	//活动详情
	struct ActivityDetail
	{
		int activity_id_;			//活动id
		int act_type_;				//活动编号(唯一索引)
		int draw_cost_;				//抽奖价格
		int ten_cost_;				//十次抽奖价格
		int add_gold_;				//奖池增加元宝
		int add_score_;				//玩家增加积分
		int base_gold_;				//奖池基础资金
		int low_gold_;				//奖池保底资金
		int reset_flag_;			//是否每天重置
		int continue_;				//持续时间
		int slot_num_;				//随机抽取格子的数量
		int shout_id_;				//播报id
		int red_point_;				//红点事件
		int act_shout_;				//活动开始播报
		int brocast_time_;			//播报时间
		int flicker_time_;			//闪烁时间
		int label_reward_;			//特殊称号奖励
		int rank_reward_;			//排名奖励
		int rank_limit_;			//排名限制
		int six_reward_;			//点亮6字奖励
		int lighten_reward_;		//全部点亮奖励
		int draw_limit_;			//每日抽奖次数 （0为无限制）
		int get_bless_;				//必定获得祝福值
		int show_bless_;			//客户端展示祝福值
		int mult_rate_;				//祝福值暴击概率(%)
		int bless_slot_id_;			//通过祝福值获得的格子id
		int combine_reset_act_;		//合服后是否重置活动
		int combine_reset_self_;	//合服后是否重置个人
		IntVec free_time_;			//免费次数时间
		IntMap reset_cost_;			//重置次数价格
		IntMap draw_type_list_;		//类型抽奖价格
		IntVec limit_time_set_;		//限时时间点
		IntVec lighten_rate_;		//点亮概率
		IntVec bless_interval_;		//单次祝福值区间
		ThreeObjVec bless_total_;	//一键到底祝福值次数
		ThreeObjVec total_cost_;	//一键到底消耗金额

		int save_gold_;				//保存奖池资金
		int save_date_type_;		//保存的日期类型
		Int64 save_first_date_;		//保存第一天日期
		Int64 save_last_date_;		//保存最后日期
		int is_combine_reset_;		//是否已合服重置标识

		int back_date_type_;		//后台日期类型（1为根据开服天数，2为根据实际时间）
		Int64 back_first_date_;		//后台第一天日期
		Int64 back_last_date_;		//后台最后日期
		Int64 refresh_tick_;		//配置写入mongo时间
		int open_flag_;				//活动状态标识： 1开启 0：关闭
		int sort_;					//后台活动排行
		IntMap agent_;				//渠道列表，指定哪些渠道可以看到这些活动信息，如果为空：表示所有渠道都可以看到
		string act_content_;		//活动内容

		//神仙鉴宝
		int draw_return_;			//抽奖返元
		IntMap draw_same_scale_;	//抽奖相同倍数

		//商店信息
		int shop_time_;				//刷新间隔
		int refresh_cost_;			//刷新花费
		int refresh_reward_;		//刷新奖励
		int refresh_times_;			//全服刷新次数
		int last_refresh_tick_;		//上次刷新时间
		int mail_id_;				//邮件ID
		IntMap group_pro_list_;		//分组权重
		IntMap group_may_be_list_;	//可出组出现的概率，个数为0，1，索引引用来原来的从0开始为第一组
		IntMap group_show_list_;	//必出
		IntMap group_no_show_list_;	//必不出
		IntMap group_limit_list_;	//出现个数上限
		RefreshRewardMap refresh_reward_list_;	//刷新奖励

		//宝石合成
		typedef std::map<int, GemSynthesisInfoVec> DayGemSynInfoMap;
		DayGemSynInfoMap day_gem_synthesis_info_map_;

		int slot_fina_id_; 			//终点格子id
		Int64 reset_tick_;	//记录活动重置时间（与玩家自身记录时间对比来重置玩家数据）

		ServerItemSet item_set_; //全服记录(玩家id 玩家姓名 获得时间 物品id 物品数量)

		typedef std::map<int, SlotInfoMap> DaySlotInfoMap;
		DaySlotInfoMap day_slot_map_;

		typedef std::map<int, SlotInfoVec> GroupSlotInfoMap;
		GroupSlotInfoMap group_slot_map_;

		typedef std::map<int, int> GroupWeightMap;
		GroupWeightMap group_weight_map_;

		typedef std::map<int, SlotInfoMap> TimeSlotInfoMap;
		TimeSlotInfoMap time_slot_map_;

		RankNumMap rank_num_map_;	//密宗榜(也包括其他活动的排行榜)
		RankLuckyMap rank_lucky_map_; //密宗幸运榜
		ThreeObjVec player_rank_vec_; //玩家排行榜
		ThreeObjVec lucky_rank_vec_;  //幸运排行榜
		RoleMailMap role_mail_map_;	//要发送邮件的玩家id
		RoleMailMap gem_role_mail_map_;	//宝石活动要发送邮件的玩家id以及奖励
		ServerItemSet bless_reward_set_;	//女神赐福活动珍稀奖励记录
		IntMap finish_free_time_;	//通关获得免费次数
		int finish_free_shout_index_;	//免费次数播报序号
		int finish_free_shout_id_;		//免费次数传闻id
		int treasure_shout_id_; 		//贵重物品传闻
		Int64 finish_all_times_;	//全服通关次数
		int server_record_count_;	//全服记录条数
		int person_record_count_;	//个人记录条数

		ActivityDetail(void);
		void reset(void);
		void new_act_reset(void);
		void combine_reset(void);	//合服重置
		void test_reset(void);		//命令重置活动
		void check_in_role_mail(Int64 role_id, int rank);
		void erase_role_mail_reward(Int64 role_id, int reward_id);
		int check_is_open_word(int num);
		void update_rank_info(Int64 role_id, string name, int num, int rank_type = RANK_TYPE_NUM);
		void sort_player_rank(int rank_type = RANK_TYPE_NUM);
		void push_rank_vec(OneRankInfo &rank_info, int rank_type = RANK_TYPE_NUM);
		void add_rank(OneRankInfo &rank_info, Int64 role_id, string name, int num);
		void add_weight(ChangeReward &obj, int &weight, int type);

		OneRankInfo *fetch_rank_info(Int64 role_id, int rank_type = RANK_TYPE_NUM);
		ItemObj fetch_lucky_egg_slot_reward(SlotInfo* slot_info, int is_color = false, int type = false);
	};
	typedef std::map<int, ActivityDetail> ActivityDetailMap; //act_type作为唯一索引
	ActivityDetailMap act_detail_map_;

	void add_new_item(ActivityDetail& act_detail, const BSONObj& res);

	void update_item(ActivityDetail* act_detail, const Json::Value& conf);
	void update_item(ActivityDetail* act_detail, const BSONObj& res);

	int fetch_act_base_conf(ActivityDetail& act_detail);
	ActivityDetail* fetch_activity_detail(int activity_id, int agent_code = -1);
	ActivityDetail* fetch_act_detail_by_type(int act_type);
	SlotInfoMap* fetch_slot_map(ActivityDetail* act_detail, int day);
	SlotInfoVec* const fetch_slot_map_by_group(ActivityDetail* act_detail, int group_id, int day);
	int fetch_slot_map_weight_by_group(ActivityDetail* act_detail, int group_id);
	SlotInfo* rand_get_slot(int activity_id, Int64 wheel_times = 0);
	SlotInfo* gold_box_rand_get_slot(int activity_id, int type = 0);
	SlotInfo* fetch_slot_info(ActivityDetail* act_detail, int day, int slot_id);

	int is_activity_time(ActivityDetail* act_detail);
	int is_activity_time(int activity_id);
	int is_activity_time(int activity_id, int date_type, Int64 first_date, Int64 last_date);

	int fetch_cur_day(int activity_id);
	int fetch_left_day(int activity_id);
	int fetch_left_tick(int activity_id);

	int cal_day_time(int time_point);
	int fecth_now_time_point(ActivityDetail* act_detail);
	int fetch_next_time_point(ActivityDetail* act_detail);
	int fetch_limit_end_tick(ActivityDetail* act_detail); //本轮秒所剩余时间
	int fetch_start_tick(ActivityDetail* act_detail); //下轮秒杀开始时间
	SlotInfo* fetch_limit_time_slot(ActivityDetail* act_detail, int time_point, int slot_id);

	int is_must_appear_slot(SlotInfo* slot_info, Int64 wheel_time = 0);
	int fetch_slot_wehght(SlotInfo* slot_info, Int64 wheel_time = 0);
	ItemObj fetch_slot_reward(SlotInfo* slot_info, Int64 wheel_time = 0);
	int fetch_rand_amount(SlotInfo* slot_info);

	void rand_get_slot_map(ActivityDetail* act_detail);

	static bool comp_by_time_desc(const ServerItemInfo &first, const ServerItemInfo &second);
	void record_serialize(ProtoServerRecord* server_record, ServerItemInfo& record);
	void act_reset_every_day(ActivityDetail* act_detail);
	void act_end_send_mail(ActivityDetail* act_detail);
	void gem_act_end_send_mail(ActivityDetail* act_detail);

	LuckyWheelActivity(void);
	void reset(void);
};

struct WheelPlayerInfo
{
	struct ItemRecord
	{
		int item_id_;
		int amount_;
		int item_bind_;
		Int64 get_time_;
		int reward_mult_;
		int sub_value_;

		ItemRecord();
		void reset();
	};
	typedef std::vector<ItemRecord> ItemRecordSet;

	struct PersonSlot
	{
		int time_point_;
		int slot_id_;
		int buy_times_;
		int is_color_;
		IntMap nine_slot_;
		ItemObj item_;

		PersonSlot();
		void reset();
		int is_find_word();
		int open_num();
	};
	typedef std::vector<PersonSlot> PersonSlotSet;

	struct ShopSlot
	{
		int is_rarity_;
		int is_cast_;
		int slot_id_;
		int is_buy_;
		int item_price_;
		int item_price_pre_;
		int group_id_;
		int day_;
		ItemObj item_;

		ShopSlot();
		void reset();
	};

	typedef std::map<int , ShopSlot> ShopSlotMap;


	struct PlayerDetail
	{
		int activity_id_;	//活动id
		int act_score_;		//个人积分
		int wheel_times_;	//(幸运大轮盘：抽奖次数 进阶宝匣：记录到达的格子)
		int reset_times_;	//每日重置次数
		int login_tick_;	//每日登录时间
		int use_free_;		//使用免费次数
		int label_get_;		//结婚活动称号领取情况
		int rank_get_;		//结婚活动排名领取情况
		int reward_get_;	//奖励领取情况
		int nine_word_reward_; //真言奖励
		int is_first_;		//是否第一次
		int open_times_;	//开启次数(手动不重置)
		Int64 reset_tick_;	//记录重置时间（与后台活动记录重置时间对比来重置玩家数据）
		int combine_reset_;	//是否已经合服重置

		IntMap rebate_map_;	//充值返元宝次数

		ItemRecordSet item_record_; //个人记录(物品id, 数量, 获得时间)
		PersonSlotSet person_slot_set_;	//个人格子记录
		IntVec reward_location_; 	//限时秒杀/幸运砸蛋 获得物品位置

		//商店信息
		IntMap group_refresh_times_map_; 	//刷新次数
		ShopSlotMap	shop_slot_map_;			//玩家格子信息
		Int64 refresh_tick_;				//刷新时间戳
		IntMap refresh_reward_map_;			//玩家刷新次数奖励记录

		//迷宫信息
		int slot_index_;					//进度序号
		int slot_scale_;					//奖励倍数
		IntMap	group_show_times_map_;		//普通出现次数数组	//鉴宝鉴宝保底次数
		IntMap 	group_show_times_map_fina_;	//最终出现次数数组
		int reward_scale_;					//奖励倍数
		IntMap free_times_map_;				//是否领取免费次数
		int	maze_free_;						//迷宫免费次数
		int bless_;							//祝福值

		//神仙鉴宝
		IntMap two_same_show_times_map_;	//换宝必出俩个次数
		IntMap three_same_show_times_map_;	//换宝必出三个次数
		IntMap now_slot_map_;				//鉴宝格子集合
		IntMap fina_slot_map_;				//最终格子集合

		IntMap slot_item_id_;				//格子奖励id
		IntMap slot_item_num_;				//格子奖励数量
		FishInfoVec fish_info_vec_;			//各种鱼信息
		int refresh_fish_flag_;				//当鱼全空的时候免费刷新标志
		IntMap fish_reward_map_;			//兑换奖励记录 用于判断上限

		//女神赐福
		int bless_value;			//祝福值
		IntMap reward_record_map;	//个人奖励记录  (item_id, amount)
		IntMap exchange_item_frequency;	//已经兑换的次数 (item_id, frequency)
		IntMap bless_reward_frequency;	//赐福奖励物品参与随机次数 (item_id, frequency)
		IntMap bless_reward_possess;	//奖励的物品列表 (item_id)

		PlayerDetail();
		void reset();
		void reset_every_day();
		void restart_reset();
		void request_reset();	//手动重置
		int check_in_reward_location(int location);
		int fetch_buy_times();
		int fetch_nine_word_num();	//获取玩家已经开启的九字个数
		int fetch_total_open_num();	//获取玩家总开启的格子个数
		int fetch_lucky_egg_open(int type); //1:获取已经开启的蛋蛋个数 2:获取彩蛋个数
		PersonSlot* fetch_person_slot(int time_point, int slot_id);
	};
	typedef std::map<int, PlayerDetail> PlayerDetailMap;
	PlayerDetailMap player_detail_map_;

	static bool comp_by_time_desc(const ItemRecord &first, const ItemRecord &second);
	void record_serialize(ProtoPersonRecord* person_record, ItemRecord& record);

	WheelPlayerInfo();
	void reset();
};

struct DailyActivity
{
	//活动索引枚举
	enum
	{
		ACT_TOTAL_DOUBLE 	= 70101,	//全民双倍

		DOUBLE_ADVANCE		= 1,	//进阶副本双倍
		DOUBLE_RAMA			= 2,	//罗摩副本双倍
		DOUBLE_STORY 		= 3,	//剧情副本双倍
		DOUBLE_SWORD		= 4,	//论剑武林双倍
		DOUBLE_XUANJI		= 5,	//玄机宝匣双倍
		DOUBLE_COUPLE		= 6,	//夫妻副本双倍

		END
	};

	struct DailyDetail
	{
		int activity_id_;			//活动id
		int act_type_;				//活动编号(唯一索引)

		int back_date_type_;		//后台日期类型（1为根据开服天数，2为根据实际时间）
		Int64 back_first_date_;		//后台第一天日期
		Int64 back_last_date_;		//后台最后日期
		Int64 refresh_tick_;		//配置写入mongo时间
		int value1_;				//值1
		int value2_;				//值2
		int open_flag_;				//活动状态标识： 1开启 0：关闭
		int sort_;					//后台活动排行
		IntMap agent_;				//渠道列表，指定哪些渠道可以看到这些活动信息，如果为空：表示所有渠道都可以看到
		string act_content_;		//活动内容

		Int64 save_refresh_tick_;	//保存的后台刷新时间
		Int64 reset_tick_;			//记录活动重置时间（与玩家自身记录时间对比来重置玩家数据）
		Int64 save_first_date_;		//保存第一天日期
		Int64 save_last_date_;		//保存最后日期

		DailyDetail(void);
		void reset(void);
		void new_act_reset();

		int is_activity_time(int use_save = false);
		int fetch_left_day();
		int fetch_left_tick();
	};
	typedef std::map<int, DailyDetail> DailyDetailMap; //act_type作为唯一索引
	DailyDetailMap daily_detail_map_;

	void add_new_item(DailyDetail& daily_detail, const BSONObj& res);
	void update_item(DailyDetail* daily_detail, const Json::Value& conf);
	void update_item(DailyDetail* daily_detail, const BSONObj& res);

	int fetch_act_base_conf(DailyDetail& daily_detail);
	DailyDetail* fetch_daily_detail_by_type(int act_type);
	DailyDetail* fetch_daily_detail(int activity_id, int agent_code = -1);

	DailyActivity(void);
	void reset(void);
};

struct TravelTeamInfo
{
    Int64 __team_id;            // 队伍ID
    std::string __team_name;     // 队名
    Int64 __leader_id;          // 队长ID
    int __auto_signup;			// 是否自动报名
    int __auto_accept;			// 是否自动接收
    int __need_force;			// 进队需要战力
    int __is_signup;            // 是否已报名

    Time_Value __refresh_signup_tick;   // 刷新报名信息
    Time_Value __create_tick;	// 创建时间
    Time_Value __last_logout_tick;	//最近队员离线时间

    struct TravelTeamer
    {
        Int64 __teamer_id;
        std::string __teamer_name;
        int __teamer_sex;
        int __teamer_career;
        int __teamer_level;
        int __teamer_force;
        Time_Value __logout_tick;
        Time_Value __join_tick;

        TravelTeamer(void);
        void reset(void);
        void serialize(ProtoRoleInfo *msg);
        void serialize(ProtoTeamer *msg);
    };

    typedef std::map<Int64, TravelTeamer> TeamerMap;
    TeamerMap __teamer_map; // key: role_id
    TeamerMap __apply_map;  // key: role_id

    LongSet __invite_role_set;

    void reset(void);
    TravelTeamer *trvl_teamer(Int64 role_id);
    bool team_is_pass_miss_day();
    bool member_is_pass_miss_day(Int64 role_id);
};

struct WeightedRandomInfo
{
	struct IdWeightDATA
	{
		int __id;
		int __weight;

		IdWeightDATA(void);
		void reset(void);
	};

	struct Compare{
		bool operator()(const IdWeightDATA& obj_one,const IdWeightDATA& obj_two){
			return obj_one.__weight < obj_two.__weight;				//从小到大排序
		}
	};

	typedef std::set<IdWeightDATA, Compare> IdWeightSET;
	IdWeightSET __id_weight_set;

	int reward_item_to_weighted_set(IntMap &reward_item, GameConfig::ConfigMap& reward_item_conf);
	int weighted_random_operator(void);

	WeightedRandomInfo(void);
	void reset(void);
};

#endif //_LOGICSTRUCT_H_
