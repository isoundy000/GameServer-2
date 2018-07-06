/*
 * MapLogicStruct.h
 *
 *  Created on: Dec 23, 2013
 *      Author: peizhibi
 */

#ifndef MAPLOGICSTRUCT_H_
#define MAPLOGICSTRUCT_H_

#include "Heap.h"
#include "MapStruct.h"

class ProtoBuffStatus;

/*
 * prop name
 * */
struct PropName
{
	const static string ADD_EXP;		//增加经验
	const static string ADD_BLOOD;		//直接增加当前血量
	const static string ADD_CULTIVATION;//增加修为
	const static string REDUCE_KILL;	//减少杀戮值
	const static string ADD_LUCKY;		//增加幸运值
	const static string ADD_GOLD;		//增加元宝
	const static string ADD_BIND_GOLD;	//增加绑定元宝
	const static string ADD_REPUTATION;	//增加声望
	const static string ADD_EXPLOIT;	//增加功勋
	const static string ADD_CONTRI; 	//增加帮贡
	const static string ADD_HONOR;		//增加荣誉
	const static string ADD_REIKI;		//增加灵气
	const static string ADD_PRACTICE;	//增加历练
	const static string ADD_SPIRIT;		//增加精华

	const static string HEALTH;			//增加血池
	const static string BEAST_EGG;		//灵宠蛋
	const static string EXP_PERCENT;	//经验卡
	const static string MOUNT_LEVEL;	//战骑升阶丹
	const static string XIANYU_LEVEL;	//仙羽升阶丹
	const static string MAGIC_LEVEL;	//法器升阶丹
	const static string BEAST_LEVEL;	//灵宠升阶丹
	const static string GODSOLIDER_LEVEL;	//神兵升阶丹
	const static string BEAST_EQUIP_LEVEL;	//灵武升阶丹
	const static string BEAST_MOUNT_LEVEL;	//灵骑升阶丹
	const static string BEAST_WING_LEVEL;	//灵羽升阶丹
	const static string BEAST_WING_ACT;		//灵羽活动翅膀
	const static string SWORD_POOL_EXP;		//剑池经验卡

	const static string ONE_OFFLINE_PERCENT;	//1.5倍离线经验符
	const static string TWO_OFFLINE_PERCENT;	//2倍离线经验符
	const static string VIP_OFFLINE_PERCENT;	//vip离线经验符

	const static string GIFT_PACK;			//奖励礼包
	const static string FIX_GIF_PACK;		//固定礼包
	const static string RESP_GIF_PACK;		//个概率礼包
	const static string TOTAL_GIF_PACK;		//总概率礼包
	const static string RAND_GIF_PACK;		//随机礼包
	const static string ROTARY_TABLE;
	const static string MAGIC_ACT_PACK;

	const static string VIP_CARD;		//VIP卡
	const static string LEVEL_UP;		//玩家升级丹
	const static string TRANSFER;		//传送卡
	const static string TRANSFER_RANDOM;	//随机传送
    const static string ROUTINE_FINISH;
	const static string OPEN_BOX_BY_KEY;
	const static string ADD_LABEL;		//增加称号
	const static string CHANGE_PERMISSION;	//改变权限
	const static string ADD_FASHION;	//增加时装
	const static string ADD_TRANSFER;	//增加变身

    const static string RECHARGE;		//充值卡
    const static string SCRIPT_COMPACT;
    const static string ADD_BEAST_SKILL;	//增加灵宠技能

    const static string ADD_SAVVY;		//增加悟性
    const static string MAGIC;			//增加能量
    const static string ACTIVE_WING;		//激活翅膀

    const static string OFFLINEHOOK_DRUG;    //增加离线经验

    const static string ADD_FISH_SCORE; 	//增加积分
};

struct PackageDetail
{
	PackageMap __pack_map;

	Money __money;					//金钱
    IntMap resource_map_;			//资源, key:类型, value：数值
    IntMap use_resource_map_;		//记录使用资源数量，包含金钱
    IntMap __auto_exchange_copper;

    void reset(void);
};

/*
 * 一般用于升级时，需要的单种物品计算
 * */
struct UpgradeAmountInfo
{
	//拥有数量
	int have_amount_;
	//需要数量
	int need_amount_;

	//使用数量
	int total_use_amount_;
	//需要购买数量
	int buy_amount_;

	//物品ID
	IntVec item_id_;
	IntVec use_amount_;

	//购买物品ID
	int buy_item_;
	//配置单价
	Money item_price_;
	//类型
	int type_;
	//是否完成
	int init_flag_;

	UpgradeAmountInfo(int type = 0);
	UpgradeAmountInfo(MLPacker* packer, const Json::Value& conf);
	UpgradeAmountInfo(MLPacker* packer, int item_id, int need_amount);
	UpgradeAmountInfo(int item_id, int need_amount, int have_amount = 0);

	void reset();
	void set_item_price(int item_id);
	void set_amount_info(int have_amount, int need_amount);
	void set_amount_info(GamePackage* package, const Json::Value& conf);

	int total_money(Money& need_money, const Money& own_money);
	int total_money();
};

struct MapLogicRoleDetail : public BaseRoleInfo
{
	int __camp_id;
    int __is_death;
    int __death_tick;
    int __fight_state;
    int __open_gift_close;		//开服豪礼关闭标识
    int __drop_act;
    int __is_big_act_time;

    Int64 change_name_tick_;
    Int64 change_sex_tick_;
    Int64 __ml_day_reset_tick;

    IntMap draw_days_;
    IntMap draw_vips_;
    IntMap draw_gift_;	//开服豪礼
    IntMap rand_use_times_;	//使用次数

	std::string __league_name;

	IntMap	wedding_self_;	//婚姻系统个人信息
	IntMap  wedding_side_;	//婚姻系统他人信息

    void reset(void);
};

//trade
struct TradeInfo
{
	int  is_trade_open; //是否开启交易
	int  trade_state;
	int  is_locked;
	int  is_trading;
	Int64 curr_role_id; //当前交易对方玩家id
	Int64 requ_role_id; //设置发起交易后，等待对方响应的玩家id
	LongList accept_role_id_list; //接收响应玩家的列表
	NPItemVector ItemTempVector;
	TradeInfo();
	void reset();
};


struct Aillusion
{
	 int id_;//幻化外形id
	 bool unlock_; //是否解锁
	 int unlocktype_; //解锁条件
	 int type_; //0无限时,1有限时
	 Time_Value shape_expire_; //到期时间
	 int expire_sec();
	 Aillusion();
	 void reset();
};
typedef std::vector<Aillusion> AillusionVec;

/*
 * 逻辑线程，整分钟定时器
 * */
class MLIntMinTimer : public GameTimer
{
public:
	int schedule_timer();

	virtual int type(void);
	virtual int handle_timeout(const Time_Value &tv);
	int notify_first_recharge_acti(void);
};

/*
 * 每天晚上12点定时器，由于时间间隔，可能会出现最长不超过1分钟的相差
 * */
class MLMidNightTimer : public GameTimer
{
public:
	int schedule_timer();

	virtual int type(void);
	virtual int handle_timeout(const Time_Value &tv);

	int midnight_refresh(void);
};

struct MasterDetial
{
	LongVec beast_set_;

	Int64 cur_beast_id_;
	Int64 last_beast_id_;
	Int64 save_beast_id_; 	// 保存宠物ID,需要强制回收宠物的场景使用

	int cur_beast_sort_;
	int last_beast_sort_;

	int gen_skill_lucky_;
	IntMap gen_skill_book_;
	int beast_skill_open_;

	void reset();
	bool validate_beast(Int64 beast_id);
};

/*
 * logic beast fight detail
 * */
struct LBFightDetail
{
	BasicElement attack_;
	BasicElement crit_;
	BasicElement hit_;
	BasicElement attack_mul_;
	BasicElement crit_mul_;
	BasicElement hit_mul_;

	void reset();
};

struct RealmDetail
{
 struct RealmInfo
   {
		int __step;///当前阶
		int __property_index;//属性索引
		int __cur_xianqi;//当前仙气
		int __cur_lingqi;//当前灵气
		/*本系统总共获得的属性加成属性*/
		int __total_blood;
		int __total_defence;//当前
		int __total_crit;//当前
		int __total_toughness;//当前
		int __total_hit;//当前
		int __total_avoid;//当前
		int __total_attack;//当前
		RealmInfo(void);
		void reset(void);
   }realmInfo;
   int __realm_id;//当前境界
	void reset(void);
	RealmDetail(void);
};

struct SellOutDetail
{
	SellOutDetail(void);

	void reset(void);
	void add_sell_out(PackageItem* pack_item, int sell_amount);

	ItemList sell_out_;
};

struct SkillCombineResult
{
	SkillCombineResult(void);
	void serialize(ProtoSkillCombine* proto_combine);

	int combine_exp_;
	int combined_exp_;
};

struct StorageRecord
{
	StorageRecord(void);
	void reset(void);

	int __storage_id;
	int __finish_count;
	bool __storage_valid;
	Time_Value __record_timestamp;
};

typedef std::vector<StorageRecord> StorageRecordSet;
typedef std::map<int64_t, int> TimestampStageMap;
typedef std::map<int, TimestampStageMap> StorageStageInfo;

struct RestoreActInfo
{
	RestoreActInfo(void)
	{
		this->__ext_type = 0;
		this->__times = 0;
	}
	void reset(void)
	{
		this->__ext_type = 0;
		this->__times = 0;
	}

	int __ext_type;
	int __times;
};

typedef std::map<int, RestoreActInfo> RestoreMap;

struct TreasuresDetail
{
	TreasuresDetail()
	{
		TreasuresDetail::reset();
	}
	void reset(void)
	{
		this->free_times_ = 0;
		this->game_index_ = 1;
		this->game_length_ = 0;
		this->reset_tick_ = 0;
		this->reset_times_ = 0;
		this->item_list_.clear();
		this->req_tick_ = Time_Value::zero;
	}

	int free_times_;
	int reset_times_;
	int game_length_;
	int game_index_;

	Int64 reset_tick_;
	ItemObjVec item_list_;
    Time_Value req_tick_;
};

struct ExpRestoreDetail
{
	ExpRestoreDetail(void);
	void reset(void);

	int __vip_type_record;				// VIP类型-过期不重置用来推导经验
	int __vip_start_time;				// VIP开始时间
	int __vip_expried_time;				// VIP过期时间-过期后不重置用来推导经验
	Time_Value __check_timestamp;		// 时间超过此值时要更新
	LongMap __timestamp_level_map;		// 对应日期的等级
	LongMap __timestamp_vip_map;		// 对应日期的VIP类型
	StorageRecordSet __storage_record_set;
	StorageStageInfo __storage_stage_info;

	RestoreMap __pre_act_map;		//昨日未参加活动数据	//活动大类 和 活动关键字段（等级或者层数 之类）
	RestoreMap __now_act_map;		//玩家参加活动数据
};

struct ScriptCleanDetail
{
    struct ScriptInfo
    {
        int __script_sort;
        int __chapter_key;  // 章节KEY，篇 * 1000 + 章节
        int __use_times;    // 副本次数
        int __use_tick;     // 时间
        int __reset_times;	// 重置次数
        int __protect_beast_index;  // 灵兽岛保护记录

        ScriptInfo(void);
        void reset(void);
    };
    typedef std::vector<ScriptInfo> ScriptInfoVec;
    ScriptInfoVec __current_script_vc;
    ScriptInfoVec __finish_script_vc;

    struct CleanGetItem
    {
    	CleanGetItem()
    	{
    	    this->__item_id = 0;
    	    this->__item_unbind_amount = 0;
    	    this->__item_bind_amount = 0;
    	}

    	int __item_id;
    	int __item_unbind_amount;
    	int __item_bind_amount;
    };
    typedef boost::unordered_map<int, CleanGetItem> GetItemMap;
    GetItemMap __get_item_map;	//旧的奖励数据
    GetItemMap __get_drop_map;	//旧的奖励数据

    struct CleanItemInfo
    {
    	CleanItemInfo()
    	{
    		this->__item_id = 0;
    		this->__amount = 0;
    	}

    	int __item_id;				//新的奖励表id
    	int __amount;				//新的奖励表数量
    };
    typedef boost::unordered_map<int, CleanItemInfo> ItemMap;
    ItemMap __item_map;		//旧的奖励数据
    ItemMap __drop_map;		//旧的奖励数据

    //新的奖励数据
    struct ScriptItemInfo
    {
    	ScriptItemInfo()
    	{
    		this->__id = 0;
    		this->__item_map.clear();
    		this->__drop_map.clear();
    		this->__get_item_map.clear();
    		this->__get_drop_map.clear();
    	}

    	int __id;
    	ItemMap __item_map;
    	ItemMap __drop_map;
    	ItemObjMap __get_item_map;
    	ItemObjMap __get_drop_map;
    };
    typedef std::map<int, ScriptItemInfo> ScriptItemMap;
    ScriptItemMap __script_item_map;

    int __drop_dragon_script_sort;
    int __drop_dragon_enter_teamers;

    int __exp;
    int __savvy;
    Money __money;
    IntVec __clean_sort_list;
    int __reset_times;
    int __top_floor;
    int __pass_wave;
    int __pass_chapter;
    int __start_wave;
    int __start_chapter;
    int __mult;	//奖励倍数

    int __state;    // ScriptCleanState
    Time_Value __begin_tick;
    Time_Value __end_tick;
    Time_Value __terminate_tick;
    Time_Value __next_check_tick;

    void reset(void);
    void reset_award(void);
};

struct ScriptCompactDetail
{
    int __compact_type;
    Time_Value __start_tick;
    Time_Value __expired_tick;
    int __sys_notify;

    void reset(void);
};


struct SmeltInfo
{
	enum
	{
		MAX_LEVEL = 100
	};
	int __smelt_level;
	int __smelt_exp;
	int __recommend;
	int open_;				//是否开启

	SmeltInfo();
	void reset();
};

struct RamaDetail
{
	struct RamaInfo
	{
		int cur_rama_level;
		int cur_rama_layer;
	};

	typedef std::map<int, RamaInfo> RamaMap;
	int cur_reiki_num;

	IntMap cur_rama_prop;
	RamaMap player_rama_map;

	void reset();
};

struct ReincarnationDetail
{
	int cur_cultivation;//当前修为
	int round;//当前转
	Int64 extra_exp; //额外经验；
	int exchange_times; //兑换修为次数
	 /*本系统获得的各属性*/
	int total_attack;
	int total_defence;
	int total_attack_low;
	int total_defence_low;
	int total_crit;
	int total_toughness;
	int total_hit;
	int total_avoid;
	int total_magic;
	int total_health;
	Time_Value tick;
    void reset(void);
};

struct ActiCodeDetail
{
	ActiCodeDetail(void);
	void reset(void);

	Int64 __id;
	Int64 __user_id;
	int __gift_sort;
	int __amount;
	Int64 __start_time;
	Int64 __end_time;
	int __used_time;
	int __batch_id;
	int __use_only_vip_client;
	int __use_only_vip;
	string __des_mail;
	char __acti_code[MAX_ACTI_CODE_LENGTH + 1];
};

struct MediaGiftBatchDetail
{
	MediaGiftBatchDetail(void);
	void reset(void);
	int __batch_id;
	int __gen_num;
	int __gen_time;
	int __gift_sort;
	int __agent_id;
	int __platform_id;
};

struct MediaGiftDef
{
	MediaGiftDef(void);
	void reset(void);

	int __gift_sort;
	int __gift_type;
	int __gift_tag;
	int __use_times;
	int __show_icon;
	int __hide_used;
	int __is_share;
	int __expire_time;

	IntVec __font_color;
	IntMap __value_ext;
	ItemObjVec __gift_list;
	char __gift_name[MAX_COMMON_NAME_LENGTH + 1];
	char __gift_desc[MAX_MEDIA_GIFT_DESC_LENGTH + 1];
};
typedef std::vector<MediaGiftDef> MediaGiftDefVc;
typedef std::map<int, MediaGiftDef> MediaGiftDefMap;
typedef std::map<Int64, ActiCodeDetail*> ActiCodeDetailMap;

struct PlayerMediaGiftDetail
{
	PlayerMediaGiftDetail(void);
	void reset(void);

	int __last_query_tick;
	IntMap __gift_use_times;
	IntMap __gift_use_tags;
	IntMap __gift_use_tick;
	ActiCodeDetailMap __acti_code_map;
};

struct RechargeDetail
{
	RechargeDetail(void);
	void reset(void);
	void record_recharge(int gold);

	int has_recharge(int index);

	int __recharge_money;
	int __recharge_times;
	int __feedback_awards;
	int __recharge_type;

	Int64 __first_recharge_time;
	Int64 __last_recharge_time;
	IntVec __recharge_awards;

	uint __love_gift_index;
	int __love_gift_recharge;

	IntMap __recharge_map;	//已充值过 key: 配置ID, value: TRUE
	BIntSet __latest_order_set;
	BIntSet __prev_order_set;

	Time_Value __order_fresh_tick;
};

struct DailyRechargeDetail
{
	DailyRechargeDetail(void);
	void reset(void);
	int __today_recharge_gold;
	Int64 __last_recharge_time;
	int __first_recharge_gold; //玩家是否首充
	int __act_recharge_times;  //开服七天充值过万天数
	int __act_has_mail;			//是否获得称号奖励
	IntMap __daily_recharge_rewards;
};

struct RebateRechargeDetail
{
	RebateRechargeDetail(void);
	void reset(void);
	int __rebate_times; //玩家购买次数
	Int64 __last_buy_time; //上次购买时间
};

struct LevelRewardsDetail
{
	LevelRewardsDetail(void);
	void reset(void);

	IntMap __rewards_map;
	int __awards_num;
};


struct TinyDetail
{
	enum
	{
		TOTAL_FUND 	= 2,
		BUY 		= 0,
		DRAW		= 1,
		DRAWED		= 2,
		END
	};

	struct FundItem
	{
		FundItem();
		void reset();

		int buy_flag_;	//是否拥有标志
		int draw_flag_;	//当天领取标志
		int draw_times_;	//已经领取次数
		int total_times_;	//总的可领取次数
		Int64 get_tick_;	//上一次获取该基金的时间
	};

	IntMap guide_map_;	//客户端引导
	FundItem fund_set_[TOTAL_FUND];

	void reset();

};

struct WingDetail
{
    struct SingleWingInfo
    {
        int __wing_id;
        int __wing_level;
        int __wing_process;

        SingleWingInfo(void);
        void reset(void);
    };
    typedef std::map<int, SingleWingInfo> WingMap;
    WingMap __wing_map;
    int __curr_wing_id;
    int __curr_wing_skill;
    int __curr_wing_skill_level;
    int __is_first_active_wing;

    void reset(void);
};

struct TotalRechargeDetail
{
	IntVec __reward_states;

	void reset();
};

struct Box
{
	int __box_open_count_one;
	int __box_open_count_ten;
	int __box_open_count_fifty;
    int __box_is_open;/*是否开启藏宝库*/
    Box();
    void reset();
    ~Box();
};

struct RoleExDetail
{
    int __savvy;
    Box __box;
    int __is_second_equip_decompose;
	RoleExDetail(void);
	void reset(void);
};

struct ItemColor
{
    int __item_id;
    int __item_amount;
    int __item_color;

    ItemColor(void);
};

extern bool ItemColorCmp(const ItemColor &left, const ItemColor &right);

struct BaseAchieveInfo
{
	struct AchieveInfo
	{
		int achieve_id_;	//子成就目标id
		int need_amount_;	//达成条件
		int sort_;			//目标排序
		int achieve_type_;	//子类细分
		int number_type_;	//类型（1：完成类成就，2：数量类成就）
		int reward_id_;		//奖励表id
		int ach_amount_;	//成就点数

		AchieveInfo(void);
		void reset(void);
	};
	typedef std::vector<AchieveInfo> AchieveInfoSet;

	struct ChildAchieve
	{
		int ach_index_;		//子成就id
		int base_type_;		//成就大类
		int child_type_;	//成就子类
		int compare_;		//比较类型（1：向前比较，2：向后比较）
		int sort_;			//子类排序
		int red_point_;		//红点

		AchieveInfoSet ach_info_set_;

		ChildAchieve(void);
		void reset(void);
	};
	typedef std::map<int, ChildAchieve> ChildAchieveMap;
	ChildAchieveMap child_achieve_map_;

	int achieve_level_;
	IntMap achieve_point_map_;

	int add_new_achieve(int index, const Json::Value& conf);
	int update_achieve(ChildAchieve* achieve, const Json::Value& conf);

	BaseAchieveInfo::ChildAchieve* find_child_achieve(int ach_index);
	BaseAchieveInfo::AchieveInfo* find_achieve_info(int ach_index, int achieve_id);
	BaseAchieveInfo::AchieveInfo* find_achieve_info(BaseAchieveInfo::ChildAchieve* child_achieve, int achieve_id);
	BaseAchieveInfo::AchieveInfo* find_achieve_info(int achieve_id);

	BaseAchieveInfo(void);
	void reset(void);
};

#endif /* MAPLOGICSTRUCT_H_ */
