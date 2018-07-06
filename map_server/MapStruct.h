/*
 * MapStruct.h
 *
 * Created on: 2013-01-17 20:02
 *     Author: glendy
 */

#ifndef _MAPSTRUCT_H_
#define _MAPSTRUCT_H_

#include "PubStruct.h"
#include "GameCommon.h"
#include "PlayerManager.h"

class ProtoBeast;
class ProtoSyncLeague;
class ProtoSkillCombine;
class ProtoSysSetting;
class ProtoTravelTeam;
class ProtoTransferInfo;

class Proto10400201;
class Proto10400202;
class Proto50100031;

class Proto30400105;
class Proto30400350;
class Proto30400051;
class Proto30400052;
class Proto31400013;
class ProtoHookDetail;

class Scene;
class GameAI;
class MapBeast;
class AIDropPack;

class SceneFactory;
class LeagueMonitor;
class SessionManager;

typedef std::list<Time_Value> TimeValueList;
typedef boost::unordered_map<int, GamePackage*> PackageMap;
typedef HashMap<int64_t, GameMover*, NULL_MUTEX> MoverMap;

enum EXIT_SCENE_TYPE
{
    EXIT_SCENE_LOGOUT     = 1,
    EXIT_SCENE_TRANSFER   = 2,
    EXIT_SCENE_RELIVE     = 3,
    EXIT_SCENE_JUMP		  = 4,
    EXIT_SCENE_ERROR 	  = 7,
    EXIT_SCENE_END
};

enum PK_STATE_TYPE
{
	PK_STATE_NONE = -1,

    PK_PEACE    	= 0,    //和平
    PK_PLENARY  	= 1,    //全体PK
    PK_TEAM     	= 2,    //队伍PK
    PK_LEAGUE   	= 3,    //帮派PK
    PK_DITHESISM	= 4,	//善恶
    PK_CAMP			= 5,	//阵营

    PK_STATE_END
};

enum FIGHT_TIPS_TYPE
{
	FIGHT_TIPS_SCRIPT_AUTO	= -2,	// 副本进入时自动回复
    FIGHT_TIPS_NOTHING      = -1,
    FIGHT_TIPS_BLANK        = 0,    // 空白
    FIGHT_TIPS_NORMAL       = 1,    // 普通攻击
    FIGHT_TIPS_CRIT         = 2,    // 暴击
    FIGHT_TIPS_MISS         = 3,    // 闪避
    FIGHT_TIPS_STATUS		= 4,	// 状态更新

    FIGHT_TIPS_SYSTEM_AUTO  = 5,    // 系统自动修改
    FIGHT_TIPS_USE_PROPS    = 6,    // 使用道具
    FIGHT_TIPS_RELIVE       = 7,    // 复活
    FIGHT_TIPS_LIGTHING		= 8,	// 雷击
    FIGHT_TIPS_LUCKY        = 9,    // 幸运一击
    FIGHT_TIPS_ABSORB       = 10,   // 护盾吸收
    FIGHT_TIPS_JUMP_MISS	= 11,	// 跳闪
    FIGHT_TIPS_SKILL		= 12,	// 技能

    FIGHT_TIPS_TYPE_END
};

enum FIGHT_UPDATE_TYPE
{
    FIGHT_UPDATE_BLOOD  		= 1,		// 通知更新血量
    FIGHT_UPDATE_MAGIC  		= 2,		// 通知更新蓝
    FIGHT_UPDATE_EXP    		= 3,		// 通知更新经验
    FIGHT_UPDATE_FIGHT_STATE 	= 4,		// 通知更新战斗状态
    FIGHT_UPDATE_DISTANCE 		= 5,		// 通知更新战斗距离
    FIGHT_UPDATE_SPEED  		= 6,    	// 通知更新移动速度
    FIGHT_UPDATE_PK				= 7,		// 通知更新PK状态
    FIGHT_UPDATE_ANGRY			= 8,		// 通知怒气值
    FIGHT_UPDATE_SKILL			= 9,		// 通知技能使用次数
    FIGHT_UPDATE_CAMP			= 10,		// 通知阵营
    FIGHT_UPDATE_JUMP			= 11,		// 更新跳跃值

    FIGHT_UPDATE_TYPE_END
};

/**
 * +---+---+---+
 * | 8 | 1 | 2 |
 * +---+---+---+
 * | 7 | 0 | 3 |
 * +---+---+---+
 * | 6 | 5 | 4 |
 * +---+---+---+
 */
enum MOVE_TOWARD_MODE
{
    MOVE_TOWARD_NULL        = 0,
    MOVE_TOWARD_UP          = 1,
    MOVE_TOWARD_RIGHT_UP    = 2,
    MOVE_TOWARD_RIGHT       = 3,
    MOVE_TOWARD_RIGHT_DOWN  = 4,
    MOVE_TOWARD_DOWN        = 5,
    MOVE_TOWARD_LEFT_DOWN   = 6,
    MOVE_TOWARD_LEFT        = 7,
    MOVE_TOWARD_LEFT_UP     = 8,
    MOVE_TOWARD_END
};


struct SceneBlock
{
    typedef HashMap<int64_t, size_t, NULL_MUTEX> MoverOffsetMap;

    BlockIndexType __block_index;   // 区块在地图的区块列表中的索引
    Block_Buffer __data_buff;   // 待广播的消息内容,由多个消息拼接成的数据块

    MoverMap __mover_map;

    MoverMap __player_map;
    MoverMap __other_mover_map;		// 用于获取周围玩家时先发玩家后发其他信息
    // 每个玩家需要广播数据的偏移点（后进入的玩家不需要广播进入前产生的消息）
    MoverOffsetMap __mover_offset_map;

    SceneBlock(void);
    SceneBlock(const SceneBlock &obj);
    SceneBlock &operator=(const SceneBlock &obj);
    void reset(void);
    int push_data(Block_Buffer &buff);
    int flush_to_mover(GameMover *mover);
    int make_up_all_disappear_info(Block_Buffer &buff, GameMover *mover = 0);
    int make_up_all_appear_info(Block_Buffer &buff, GameMover *mover = 0, bool send_by_gate = false);
};

struct SceneDetail
{
    typedef std::vector<SceneBlock> SceneBlockList;
    typedef std::vector<size_t> BlockMoverAmountList;

    int __scene_id;
    int __space_id;

    int __block_width;      // 九宫格一个区块的横向宽度，单位：格子
    int __block_height;     // 九宫格一个区块的竖向高度，单位：格子
    int __mpt_x_len;        // 地图MPT横向的最大格子数
    int __mpt_y_len;        // 地图MPT竖向的最大格子数
    int __block_x_amount;   // 整张地图的九宫格横向的区块数
    int __block_y_amount;   // 整张地图的九宫格竖向的区块数

    SceneBlockList __scene_block_list;  // 区块列表
    BlockMoverAmountList __mover_amount_list;
    BlockMoverAmountList __player_amount_list;
    BLongSet __player_block_set;   // 有玩家区块的ID集合
    BLongSet __has_block_set;		// 有广播包的区块

    int __dynamic_grid_pixel;   // 怪物寻路时的一个格子的像素
    int __dynamic_x_len;        // 寻路用MPT横向的最大格子数
    int __dynamic_y_len;        // 寻路用MPT竖向的最大格子数
    typedef std::vector<int> MptCoordList;
    MptCoordList __dynamic_mpt; // 动态MPT

    typedef boost::unordered_map<int, BLongSet> SortAIMap;
    typedef SortAIMap GroupAIMap;
    SortAIMap ai_sort_map_;
    GroupAIMap ai_group_map_;
    BLongSet boss_sort_set_;
    BLongSet safe_area_set_;

    typedef boost::unordered_map<Int64, SceneBlock *> TeamSceneBlockMap;
    TeamSceneBlockMap team_scene_block_map_;

    int __near_block_width;
    int __near_block_height;
    int __near_block_x_amount;
    int __near_block_y_amount;
    typedef boost::unordered_set<GameMover *> MoverSet;
    typedef std::vector<MoverSet> NearBlockList;
    NearBlockList __near_block_list;
    BLongSet __near_player_block_set;	// 有玩家的处理附近区块的ＩＤ
    Time_Value __near_notify_tick;	// 定时更新附近玩家列表

    int __room_amount;    // 当前正在使用的房间数
    SceneBlockList __scene_block_vec[MAX_ROOM_SCENE_SIZE];
    BlockMoverAmountList __mover_amount_vec[MAX_ROOM_SCENE_SIZE];
    BlockMoverAmountList __player_amount_vec[MAX_ROOM_SCENE_SIZE];
    int __room_player_amount_list[MAX_ROOM_SCENE_SIZE];
    MptCoordList __dynamic_mpt_vec[MAX_ROOM_SCENE_SIZE];

    MoverMap __player_map_vec[MAX_ROOM_SCENE_SIZE];

    void reset(void);
};

extern bool operator==(const MoverCoord &left, const MoverCoord &right);
extern bool operator!=(const MoverCoord &left, const MoverCoord &right);
extern bool operator<(const MoverCoord &left, const MoverCoord &right);
extern int coord_offset_grid(const MoverCoord &mover_coord, const MoverCoord &target_coord);
extern int coord_offset_grid_min(const MoverCoord &mover_coord, const MoverCoord &target_coord);
extern bool check_coord_distance(const MoverCoord &mover_coord, const MoverCoord &target_coord,
		const int distance, const int mapping_factor=1, const int grid_dis = 30);
extern bool check_coord_pixel_distance(const MoverCoord &mover_coord, const MoverCoord &target_coord,
        const int pixel_radius, const double mapping_factor = 1);
extern int vector_to_angle(const MoverCoord &start, const MoverCoord &end);
extern int toward_to_angle(const int toward_index);
extern double vector_to_radian(const MoverCoord &start, const MoverCoord &end);
extern int center_to_rect(MoverCoord &pointA, MoverCoord &pointB, MoverCoord &pointC, MoverCoord &pointD,
        const MoverCoord &center, const double angle, const double width, const double height);


struct SceneInfo
{
	int id_;
	int mode_;
	int space_;

	SceneInfo(int scene_id, int scene_mode = 0, int sapce_id = 0)
	{
		this->id_	= scene_id;
		this->mode_	= scene_mode;
		this->space_= sapce_id;
	}
};

struct SaveSceneInfo
{
	int scene_id_;
	int scene_mode_;
	int space_id_;
	MoverCoord coord_;

	int blood_;
	int magic_;
	int pk_state_;

	SaveSceneInfo();
	void reset();
};

struct MoverDetail
{
	// cur location
    int __scene_id;
    MoverCoord __location;
    int __scene_mode;
    int __space_id;

    // prev location
    int __prev_scene_id;
    MoverCoord __prev_location;
    int __prev_scene_mode;
    int __prev_space_id;

    // 在同一场景中临时切换到另一坐标时保存当前坐标(观战)
    MoverCoord __temp_location;

    // use for make up move info {{{
    Time_Value __step_arrive_tick;
    Time_Value __step_time;
    int __toward;		//角度
    typedef std::vector<MoverCoord> MoverStepList;
    MoverStepList __step_list;

    typedef std::list<MoverCoord> MovePath;
    MovePath __move_path;
    // }}}
    BlockIndexType __cur_block_index;

    BasicElement __speed;
    BasicElement __speed_multi;

    void reset(void);
};

struct MapTeamInfo
{
	int team_index_;
	Int64 leader_id_;
	LongMap teamer_set_;
	LongMap replacement_set_;

	MapTeamInfo(void);
	void reset(void);

	void serialize(ProtoTravelTeam* team);
	void unserialize(const ProtoTravelTeam& team);

	int team_index();
};

struct HookDetail
{
	enum AUTO_DRUG_STATUS
	{
		AUTO_DRUG_NULL = 0,
		AUTO_DRUG_LOW = 1,   //自动吃药：回复量从小到大
		AUTO_DRUG_HIGH = 2,  //自动吃药：回复量从大到小
		AUTO_DRUG_STATUS_END
	};

	enum AUTO_BUY_DRUG_STATUS
	{
		AUTO_BUY_DRUG_NULL = 0,
		AUTO_BUY_DRUG_PRIMARY = 1,//自动购买小药品
		AUTO_BUY_DRUG_SENIOR = 2, //自动购买大药品
		AUTO_BUY_DRUG_STATUS_END
	};

	enum AUTO_RELIVE_TYPE
	{
		AUTO_RELIVE_NULL = 0,
		AUTO_RELIVE_ON_SITE = 1,
		AUTO_RELIVE_ON_TOWN = 2,
		AUTO_RELIVE_END
	};

	int __is_hooking;

	int __kill_task_monster;
	int __kill_nearby_monster;
	int __kill_other;

	int __auto_pickup;
	int __auto_call_beast;
	int __auto_drug;			// 0 不自动吃药, 1 优先使用小瓶药，2 优先使用大瓶药

	int __auto_drug_blood;		// 触发自动吃药的血量
	int __auto_drug_magic;		// 触发自动吃蓝的血量
	int __auto_drug_blood_beast;
	int __auto_drug_magic_beast;

	int __auto_buy_drug;		// 自动买药, VIPX时默认勾选
	int __stop_hook;
	int __relive_type;

	int __hook_skill_list_index;
	int __hook_skill[MAX_HOOK_SKILL_LIST][MAX_HOOK_SKILL_NUMBER];

	int __auto_avoid_boss;	// 1 自动避开BOSS

    int __drug_blood_tick;   // 吃血药时间
    int __drug_magic_tick;   // 吃法力时间
    int __drug_blood_notify_buy;	// 血药提示购买
    int __drug_magic_notify_buy;	// 蓝药提示购买

    int __auto_back_main_tick;	// 自动回城时间
    int __auto_random_tick;		// 随机卷时间
    int __auto_back_main;   // 是否自动使用回城卷
    int __auto_back_main_blood; // 使用回城卷的血量
    int __auto_random;      // 是否自动使用随机卷
    int __auto_random_blood;    // 使用随机卷的血量

	void reset(void);
};

//变身系统
struct TransferDetail
{
	enum
	{
		TYPE_NORMAL  	= 1,	//普通聚精
		TYPE_GOLD		= 2,	//元宝聚精

		CHANGE_TRANSFER = 1,	//切换变身
		USE_TRANSFER	= 2,	//使用变身
		CD_TRANSFER 	= 3,	//变身CD
		SYNC_TRANSFER	= 4,	//同步变身

		END
	};

	int level_;	//英魂等级
	int exp_;	//经验(英魂值)
	int open_;	//是否开启
	int stage_;	//等阶
	int gold_times_;	//元宝聚精次数

	Int64 transfer_tick_; //变身开始时间
	int last_;		   	//变身持续时间
	int active_id_;	   	//正在使用的变身id
	int open_reward_;	//是否领取开启奖励

	Time_Value refresh_tick_;

	//不需要保存的字段
	int reduce_cool_;	//变身cd减少
	int add_time_;		//变身时间增加(百分比)

	//单个变身数据
	struct TransferInfo
	{
		int transfer_id_;	//变身id
		int transfer_lv_;	//变身等级
		int is_permanent_;	//1为永久性,0为限时
		int is_active_;		//是否激活中
		Int64 active_tick_;	//激活时间,永久性为-1
		Int64 end_tick_;	//失效时间,永久性为-1
		int transfer_skill_;//变身技能
		IntMap skill_map_;	//技能

		SkillMap skill_info_;		//技能
		FightProperty one_prop_;	//单个变身属性

		TransferInfo(int id = 0);
		void reset();

		const Json::Value& conf();
		const Json::Value& detail_conf();

		bool is_active_time();

		void add_skill();
		void create_skill();
		void delete_skill();

		void serialize(ProtoTransferInfo *proto);
		void unserialize(ProtoTransferInfo proto);
	};
	typedef std::map<int, TransferInfo> TransferInfoMap;
	TransferInfoMap transfer_map_;

	FightProperty spirit_prop_;	//英魂属性
	FightProperty total_prop_;	//总属性

	void reset();
	void reset_every_day();

	TransferInfo* fetch_transfer_info();
	TransferInfo* fetch_transfer_info(int id);

	bool is_in_cool();
	bool is_in_transfer();
	int transfer_cool();	//变身cd剩余时间
	const Json::Value& spirit_level_json();
	const Json::Value& spirit_stage_json();
	const Json::Value& transfer_base_json();
};

//时装
struct RoleFashion
{
	//时装补领条件
	enum
	{
		COND_LEVEL 	= 1, 	//等级
		COND_ONLINE_DAY	= 2,	//登录天数

		COND_END
	};

	struct FashionInfo
	{
		int fashion_id_; 	//时装id
		int color_id_;		//选择的染色id
		int active_type_;	//激活状态(现改为使用物品就激活)
		int is_permanent_;	//时装类型（1为永久性,0为限时）
		Int64 active_tick_;	//激活时间,永久性时装为-1
		Int64 end_tick_;	//失效时间,永久性时装为0

		IntVec color_set_;	//已经染色的id集
		FightProperty fight_prop_;		//单件时装属性(基础属性和染色属性，不包括染色数量加成)
		FightProperty color_num_prop_;	//时装染色数量奖励属性

		FashionInfo();
		void reset();
		const Json::Value& fashion_json();
		int check_color_set_has_color(int color_id);
		int check_json_has_color(int color_id);
	};
	typedef std::map<int, FashionInfo> FashionInfoMap;
	FashionInfoMap fashion_map_;

	int level_;		//当前等级
	int exp_;		//当前经验
	int select_id_;	//选择的时装id/外观id
	int sel_color_id_; //选择的染色id
	int open_;		//开启状态
	IntVec send_set_;	//时装补领集合

	FightProperty total_prop_;	//总属性

	RoleFashion();
	void reset();
	int permanent_fashion_amount();		//已经激活的永久时装数量
	int check_in_send_set(int fashion_id);
	FashionInfo* fetch_fashion_info(int fashion_id);
	const Json::Value& fashion_level();
	const Json::Value& fashion_num_conf();
};

// 藏宝阁
struct HiddenTreasureDetail
{
	int day_;				//天数
	int open_;				//激活状态 0未激活 1已激活 2已失效
	int get_status_;		//领取状态 0未领取 1已领取
	Int64 refresh_tick_;	//刷新时间
	IntMap buy_map_;		//购买状态

	HiddenTreasureDetail(void);
	void reset(void);
	void set_next_day_info();
};

// 剑池细节
struct SwordPoolDetail
{
	int level_;
	int exp_;
	int open_;
	int stype_lv_; // 幻化等级
	Time_Value refresh_tick_;

	struct PoolTaskInfo
	{
		int task_id_;		// 任务id
		int total_num_;		// 总次数
		int left_num_;		// 剩余次数

		PoolTaskInfo(void);
	};
	typedef std::map<int, PoolTaskInfo> TodayTaskInfoMap;
	typedef std::map<int, PoolTaskInfo> LastTaskInfoMap;

	TodayTaskInfoMap today_task_map_;	// 今日任务
	LastTaskInfoMap last_task_map_;		// 昨日任务

	SwordPoolDetail(void);
	void reset(void);
};

struct MountDetail
{
	int open_;				//是否开启
	int type_;				//类型
	int mount_grade_;		//战骑阶数
	int bless_;				//祝福值
	int fail_times_;		//失败次数
	int limit_flag_;		//是否有时间限制
	Int64 finish_bless_;	//祝福值结束时间

	int on_mount_;			//是否乘上战骑
	int mount_shape_;		//外形等级
	int act_shape_;			//活动外形

	int ability_amount_;	//资质进度
	int growth_amount_;		//成长进度
	int sword_pool_level_;	//剑池等级

	SkillMap skill_map_;		//技能
	FightProperty fight_prop_;	//属性
	FightProperty temp_prop_;	//临时属性
	FightProperty total_prop_;	//总属性

	//配置
	int prop_index_;
	int equip_index_;
	int try_task_id_;
	int open_activity_;
	int skill_fun_;
	int shout_id_;
	int shout_start_;
	int evoluate_red_;
	int ability_red_;
	int growth_red_;
	int skill_red_;
	int equip_red_;
	int no_tips_;
	int tips_task_;
	int ach_grade_;
	int ach_skill_;
	int force_open_;
	string fun_str_;

	const Json::Value& conf();
	const Json::Value& set_conf();
	FighterSkill* find_skill(int id);

	int is_all_skill_open();
	int is_one_level_skill(int skill_id);
	double left_blood_value(int skill_id, int left_blood_per, int type);

	void set_grade(int grade, int add = 0);
	void add_new_skill(int id, int lvl = 1);
	void reset(int type = 1, int flag = 0);
	void set_bless_time();
	void check_adjust_open(int level);

	int shape_id();
	int left_time();
	int skill_force();
	int hidden_flag();
	int prop_open();
	int sword_pool_multi();
};

struct BeastDetail
{
	Int64 beast_id_;
	Int64 src_beast_id_;
	Int64 master_id_;

	int type_;
	int beast_sort_;
	string beast_name_;

	BeastDetail(void);
	void reset(void);
};

/*
 * 战斗线程，每秒定时器
 * */
class MapOneSecTimer : public FixedTimer
{
public:
	MapOneSecTimer();
    virtual int handle_timeout(const Time_Value &tv);
};

/*
 * 战斗线程，整小时定时器，由于时间间隔，可能会出现最长不超过1分钟的误差
 * */
class MapIntHourTimer : public GameTimer
{
public:
	int schedule_timer();

	virtual int type(void);
	virtual int handle_timeout(const Time_Value &tv);
};

/*
 * 战斗线程，每天晚上12点定时器，由于时间间隔，可能会出现最长不超过1分钟的误差
 * */
class MapMidNightTimer: public GameTimer
{
public:
	int schedule_timer();

	virtual int type(void);
	virtual int handle_timeout(const Time_Value &tv);
};

struct TrvlKeepAliveTimer : public GameTimer
{
public:
	virtual int type();
	virtual int handle_timeout(const Time_Value &tv);
};

struct MLVipDetail : public BaseVipDetail
{
	int __check_flag;
    IntMap __is_given;
    IntMap __weekly_given;
    Time_Value __weekly_tick;

    int __super_get_type;	//超级vip领取状态
    string __des_mail;		//超级vip邮件内容

	MLVipDetail(void);
	void reset(void);

	bool is_has_reward(int vip_type, int reward_type);
};

struct OfflineRewardsDetail
{
	int __received_mark;  //领取标记
	long long __logout_time;
	long long __offline_sum;
	long long __exp_num;
	int __longest_time;

	void reset(void);
	OfflineRewardsDetail(void);
};

struct OnlineRewardsDetail
{
	int __stage; //在线奖 可领阶段
	int __pre_stage; // 上次领奖阶段
	int __received_mark;  //领取标记
	Int64 __login_time; 	//登录时间

	Int64 __online_sum; //今日累计在线时间
	IntVec award_list;
	RewardInfo __reward;
	void reset(void);
	OnlineRewardsDetail(void);
};

struct CheckInDetail
{
	int __month_day; // 当月多少天
	int __award_index; // 下一次可领取的奖励的索引(也可理解为已经签到的次数)
	Int64 __check_in_point; // 获得的签到积分
	Int64 __last_time; // 最后一次签到的时间
	int __show_point; // 是否需要显示顶部签到积分信息
	int __popup;	// 是否需要弹出签到窗口

//	int __total_cycle_id;  //累计签到周期 ID
	int __charge_money; // 当天是否充值 -1 没充值 0 充值没有再领一次 1 充值并且再领一次了
    int __check_total_index; // 当前累计签到所需天数 ID （ 在JSON里获得） 0 表示 还没有领过 1 表示领了第一个（2天的累计奖励 以此递推
    Int64 __total_last_time;
    int __total_popup;

	void reset(void);
	CheckInDetail(void);
};

struct BlessDetail
{
	time_t __buff_start;	// buff 的开始时间
	time_t __buff_left;		// buff 剩余时间
	time_t __next_reward_time;	// 下一次发放奖励的时间
	int    __reward_times;	// 发放奖励的次数
	int    __reward_amount; // 累计发放的奖励数

	void reset(void);
};

struct SkillCoolInfo
{
	Int64 role_;
	std::map<int, Time_Value> skill_cool_;

	void reset();
	Time_Value fetch_time(int skill);
};



#endif //_MAPSTRUCT_H_
