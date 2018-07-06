/*
 * ActivityStruct.h
 *
 *  Created on: Dec 16, 2016
 *      Author: peizhibi
 */

#ifndef ACTIVITYSTRUCT_H_
#define ACTIVITYSTRUCT_H_

#include "LogicStruct.h"

class LogicActivityer;
class ProtoLimitValue;

struct BackSetActDetail
{
	enum
	{
//		ACT_RANK				= 1007,	//排行榜
//		ACT_DAILY_RECHARGE      = 1013,	//每日充值（首次充值为准）
//        ACT_MULTILE_RATE		= 1028,	//多倍奖励活动

		//开服活动
        F_ACT_AIM_CHASE			= 1,	//目标追求
        F_ACT_GROWTH_MOUNT		= 2,	//成长线进阶
        F_ACT_FIRST_REC_ALL		= 3,	//首充团购
        F_ACT_ACCU_RECHARGE		= 4,	//累计充值
        F_ACT_ALL_PEOPLE_GROWTH	= 5,	//全民总动员
        F_ACT_ALL_PEOPLE_RANK	= 6,	//全民冲榜
        F_ACT_LEAGUE_WAR		= 7,	//首次帮派争霸
        F_ACT_CUMULATIVE_LOGIN  = 8,	//累计登录
        F_ACT_CORNUCOPIA		= 9, 	//聚宝盘
        F_ACT_OPEN_BUY			= 10,	//开服抢购

        //非合服返利活动
        F_ACT_R_GROWTH_MOUNT	= 11,	//返利进阶活动
        F_ACT_R_ACCU_RECHARE	= 12,	//返利累充
        F_ACT_R_QUICK_BUY		= 13,	//返利抢购
        F_ACT_R_TUPU			= 15,	//返利图谱

        //合服活动
        C_ACT_LOGIN				= 31,	//登录奖励
        C_ACT_FIRST_REC_ALL		= 32,	//首冲团购
        C_ACT_ACCU_RECHARGE		= 33,	//累充奖励
        C_ACT_CONSUM_RANK		= 34,	//消费排行
        C_ACT_GROWTH			= 35,	//成长进阶
        C_ACT_EXCHANGE			= 36,	//合服兑换
        C_ACT_SHOP				= 37,	//合服商城

        //合服返利活动
        C_ACT_R_GROWTH_MOUNT	= 61,	//合服返利进阶活动
        C_ACT_R_ACCU_RECHARE	= 62,	//合服返利累充
        C_ACT_R_QUICK_BUY		= 63,	//合服返利抢购
        C_ACT_R_TUPU			= 65,	//合服返利图谱

        F_ACT_FEST_LOGIN		= 101,	//登入活动
        F_ACT_FEST_BOSS			= 102,	//BOSS活动
        F_ACT_FEST_CONSUM		= 103,	//消费活动
        F_ACT_FEST_DROP			= 104,	//掉落活动

		LOGIN_DRAW_ONCE			= 1,
		LOGIC_DRAW_DAYLIY		= 2,

		NONE_START				= 0,
		HAVE_START				= 1,
		ALL_FINISH				= 2,
		NONE_ACT				= 3,

		UNDRAW					= 0,		//未领取
		DRAWED					= 1,		//已领取
		CNDRAW					= 2,		//不能领取

		//多倍奖励活动需要加倍的类型
		REWARD_TYPE_CONTRIBUTE			= 1,	//贡献
		REWARD_TYPE_SALARY				= 2,	//俸禄
		REWARD_TYPE_JOIN_IN				= 3,	//参与奖
		REWARD_TYPE_CITY_SALARY			= 4,	//城主方俸禄
		REWARD_TYPE_RANK				= 5,	//排名奖励
		REWARD_TYPE_WINNER				= 6,	//胜利方奖励
		REWARD_TYPE_EXPLOIT				= 7,	//功勋奖励

		RECIVE_FLOWER = 1,	//收花
		SEND_FLOWER = 2,	//送花


		COND_TYPE_0 		= 0,	//count = 1, 只有一个条件值
		COND_TYPE_1			= 1,	//count = 2, 排行榜条件，第一个为起始等级，第二个为结束等级, 到达起始值所有人奖励
		COND_TYPE_2			= 2,	//count = 2, 一个条件值，一个到达值(arrive)
		COND_TYPE_3			= 3,	//count = 3, 一个条件值，一个到达值(arrive) 2的特殊类型
		COND_TYPE_4			= 4,	//count = 2, 特殊排行榜条件，第一个为起始等级，第二个为结束等级, 到达起始值所有人奖励

		HANDLE_TYPE_1		= 1,	//累计

		D_REWARD_T_0		= 0,	// 表示达到即可领奖, 凌晨自动发邮件
		D_REWARD_T_1 		= 1, 	// 表示活动结束时凌晨自动发邮件
		D_REWARD_T_2 		= 2,	// 帮派争霸排行奖励处理方式
		D_REWARD_T_3 		= 3,	// 每天不清除，必须手动领取
		END
	};

	struct ActRewardRate
	{
		ActRewardRate();
		void reset();
		void set(int type);
		int contribute;		//贡献
		int salary;			//俸禄
		int join_in;		//参与奖
		int city_salary;	//城主方俸禄
		int rank;			//排名奖励
		int wanner;			//胜利方奖励
		int exploit;		//功勋奖励
		int rate;  //倍率
	};

	struct ActItem
	{
		ActItem();

		void set_cond(const string& cond_str);
		void set_reward(const string& reward_str);

		bool arrive();
		bool drawed(Int64 role);

		int cur_index_;		//每个奖励的位置索引，0开始
		string content_;	//每个奖励的描述信息
		string exchange_item_name_; //兑换物品名称

		LongMap sub_map_;		//记录
		LongMap drawed_map_;	//key: role_id, value: 是否领取

		int brocast_;	//该奖励领取时是否广播
		int times_;		//奖励限制领取次数，0为无限次
		int must_reset_;	//图谱兑换重置
		int handle_type_;	//处理方式
		int cash_coupon_;	//可使用代金券
		int sub_value_;
		LongVec cond_value_;		//领取奖励的条件值

		ItemObjVec cost_item_; // 兑换/购买 消耗物品
		ItemObjVec pre_cost_;  // 兑换/购买 原价消耗

		int reward_id_;			//统一奖励配置
		int reward_type_;		//奖励类型 0表示达到即可领奖，1表示活动结束时邮件发送, 3 帮派争霸排行奖励处理方式, 4 每天不清除，必须手动领取
		int reward_start_cond_;	//该名次奖励的最小值
		int exchange_type_;		//兑换类型
		ItemObjVec reward_;		//奖励数组，每个数据项为{道具id，道具绑定状态bind，道具数量amount}
	};

	typedef std::vector<ActItem>	ActItemSet;

	struct ActTypeItem
	{
		ActTypeItem()
		{
			this->act_index_ = 0;
			this->sort_ = 0;
			this->first_type_ = 0;
			this->second_type_ = 0;
			this->start_tick_ = 0;
			this->stop_tick_ = 0;
			this->update_tick_ = 0;
			this->reward_start_ = 0;
			this->reward_end_ = 0;
			this->sync_flag_ = false;
			this->act_stop_and_send_mail_ = 0;
			this->priority_ = 0;
			this->icon_type_ = 0;
			this->cond_type_ = 0;
			this->start_cond_ = 0;
			this->mail_id_ = 0;
			this->red_event_ = 0;
			this->force_red_ = 0;
			this->redraw_ = 0;
			this->record_value_ = 0;
			this->limit_ = 0;
			this->special_notify_ = 0;
			this->day_clear_ = 0;
			this->cornucopia_server_gold_ = 0;
		}

		void update_reset()
		{
			//后台配置更新时需要重置的一些字段，主要是一些数组类型的
			this->agent_.clear();
			this->act_item_set_.clear();
		}

		bool validate_time() const
		{
			Int64 now_tick = ::time(NULL);
			return now_tick >= this->start_tick_ && now_tick < this->stop_tick_;
		}

		bool is_need_save() const;

		int passed_day()
		{
			Int64 now_tick = ::time(NULL);
			Int64 next_day_tick = ::next_day(0, 0, Time_Value(this->start_tick_)).sec();

			if (now_tick < next_day_tick)
			{
				return 0;
			}

			return (now_tick - next_day_tick) / Time_Value::DAY + 1;
		}

		void serialize(PActTypeItem* act_item) const;
		void unserialize(const PActTypeItem* act_item);

		void sort_t_sub_map();	//冲榜排序
		void sort_t_sub_map_b();	//消费榜排序
		void reset_everyday();

		IntMap agent_;	//渠道列表，指定哪些渠道可以看到这些活动信息，如果为空：表示所有渠道都可以看到

		int act_index_;	//活动的索引（主要用做本地代码和后台数据库配置文件匹配）
		int sort_;		//各个活动排序（数值越大优先级越高）
		int sync_flag_;		//排行同步标志
		int redraw_;		//重复领取
		int cond_type_;		//条件类型
		int start_cond_;	//起始条件
		int red_event_;		//红点事件
		int limit_;			//限制次数
		int special_notify_; //特殊字段（用于帮派争霸）
		int day_clear_;		//玩家记录每天清除
		int record_value_;	//记录达到条件值
		int force_red_;

		int first_type_;	//活动编号，就算上面enum的枚举类型（如ACT_LOGI = 1001,	//登入活动），必须和后台数据库配置文件对应一致
		int second_type_;	//活动编号（排行榜有这个类型，其他的为0）
		int priority_;		//活动优先级（当有多个相同的活动时（活动第一、第二类型相同），先显示优先级高的）
		int icon_type_;		//活动图标类型，1开服，2合服，3跨服，4日常，51节日：愚人节，52节日：劳动节，53节日：端午，54节日：情人节，55节日：中秋，56节日：国庆，57节日：万圣节，58节日：圣诞，59节日：元旦，60节日：春节，61节日：元宵，
		string act_title_;	//活动标题
		string act_content_;//活动描述

		IntMap open_time_;	//开启时间

		int mail_id_;			//邮件ID
		string mail_title_;		//邮件标题
		string mail_content_;	//邮件内容

		Int64 start_tick_;	//活动开始时间（1970到活动开始时经过的秒数）
		Int64 stop_tick_;	//活动结束时间（1970到活动结束时经过的秒数）
		Int64 update_tick_;		//活动创建时间（PHP将后台配置信息写入到后台数据库的时间）,用来检测后台配置是否有重新改动过

		ServerRecordMap server_record_map_; 		//全服记录(玩家id 获得时间 金额)
		int cornucopia_server_gold_;				//聚宝盆全服金额
		ThreeObjMap t_sub_map_;	//记录信息
		ThreeObjVec t_sub_rank_;  //排行记录
		BaseMember first_info_;	//第一名

		Int64 reward_start_;	//奖励领取的起始条件（主要用于排行榜）
		Int64 reward_end_;		//奖励领取的起始条件（主要用于排行榜）
		std::map<Int64, FontPair> id_name;	//key：玩家id， value：pair.first玩家名字,pair.second仙盟名字（不是仙盟排行就为空）， 主要用于排行榜

		int act_stop_and_send_mail_;		//活动结束时发送邮件的标志（主要因为延时导致不能根据活动结束时间准确判断活动结束）
		ActItemSet act_item_set_;	//每个活动奖励列表（同个活动有多个奖励，每个奖励设有不同的限制条件、奖励描述、奖励道具列表（道具id，道具绑定状态bind，道具数量amount））
	};

	typedef std::vector<ActTypeItem> ActTypeItemSet;  //每个活动详情
	ActTypeItemSet act_type_item_set_;   //总的活动数组（如单笔充值，累计充值，每天登录等等）
	ActTypeItemSet end_act_set_;   //已经结束的活动数组

	BackSetActDetail(void);
	virtual ~BackSetActDetail(void);
	virtual int after_load_activity(DBShopMode* shop_mode) = 0;

	void reset();
	void fetch_all_index(IntMap& index_map);

	int add_new_item(const BSONObj& res);
	int add_new_item(int index, const Json::Value& conf);
	int update_item(ActTypeItem* item, const BSONObj& res);
	int update_item(ActTypeItem* item, const Json::Value& conf);
	int get_multile_rate_act(int agent_code, int type, BackSetActDetail::ActRewardRate& rate_info);

	//注意：find_item是根据活动的唯一id索引查找的，而find_item_by_type是根据活动的第一、第二类型进行查找的（服务器会存在多个第一、第二类型相同但活动id不同活动）
	BackSetActDetail::ActTypeItem* find_item(int act_index, int agent_code = -1, bool validate_time = false);
	BackSetActDetail::ActTypeItem* find_item_by_day(const PairObj& type, int day, int agent_code = -1);
	BackSetActDetail::ActTypeItem* find_end_item(int act_index);
	BackSetActDetail::ActTypeItem* find_end_item_by_type(int act_type, int second_type = 0);

	void remove_long_end_act();
	BackSetActDetail& act_detail();

	static ItemObj str_to_itemobj(const string& reward_str);
	static bool compare_act_type_item(const BackSetActDetail::ActTypeItem& first,
				const BackSetActDetail::ActTypeItem& second);
};		//后台数据库配置文件信息

struct MLActivityerDetial
{
	struct ActItem
	{
		ActItem()
		{
			this->arrive_ = 0;
			this->drawed_ = 0;
		}

		bool have_reward()
		{
			return this->arrive_ > this->drawed_;
		}

		int arrive_;
		int drawed_;	//arrive_和drawed_共同组成奖励的领取状态（例如在循环充值和重复消费里面，arrive_表示奖励的总的可领取次数，而drawed_表示奖励的已领取次数），具体情况，具体分析；
		IntMap arrive_map_;
	};

	typedef std::map<int, ActItem> ActItemMap;	//map<key, value>其中key和cur_index_对应奖励下标，value表示奖励的领取状态

	struct ActTypeItem
	{
		ActTypeItem()
		{
			ActTypeItem::reset();
		}

		void reset()
		{
			this->cur_index_ = -1;
			this->sub_value_ = 0;
			this->second_sub_ = 0;
			this->update_tick_ = 0;
			this->star_tick_ = 0;
			this->stop_tick_ = 0;
			this->act_item_map_.clear();
		}

		int cur_index_;		//这里的cur_index_和BackSetActDetail::cur_index_对应一致，表示每个奖励的位置索引（同一活动可能有多个奖励，如单笔充值10、50...元宝等），从0开始

		Int64 sub_value_;	//每种奖励的附加值，具体情况，具体分析，（例如：累计充值时代表总的充值数量，宠物成才或宠物灵修时代表宠物id，仙羽提升时代表仙羽类型,删档充值返回时表示是否已经返还过（0未返还，非0删档前充值元宝数））
		Int64 update_tick_;	//在充值天数活动中表示上次充值时间
		Int64 second_sub_;	//第二附加值，在充值天数活动中表示累计充值天数
		Int64 star_tick_;	//该活动开始时间
		Int64 stop_tick_;	//该活动结束时间
		ActItemMap act_item_map_;	//每笔奖励的领取状态，map<key, value>其中key是上面的cur_index_对应，value是struct ActItem表示奖励的领取状态
	};

	typedef std::map<int, ActTypeItem>	ActTypeItemMap;		//这里的key是BackSetActDetail::act_index_的值，表示当前奖励是哪个活动的,value是奖励的情况

	int last_request_index_;
	Int64 combine_tick_;
	IntMap accu_value_map_;
	ActTypeItemMap act_type_item_set_;

    void reset(void)
    {
    	this->combine_tick_ = 0;
    	this->last_request_index_ = -1;
    	this->accu_value_map_.clear();
        this->act_type_item_set_.clear();
    }
};

// 后台活动全配置
struct JYBackActivityItem
{
    enum
    {
        FTYPE_OPEN_ACTIVITY = 1,        // 开服活动
        FTYPE_RETURN_ACTIVITY = 2,      // 返利活动
        FTYPE_COMBINE_ACTIVITY = 3,     // 合服活动
        FTYPE_FESTIVAL_ACTIVITY = 4,    // 节日活动
        FTYPE_DAILY_ACTIVITY = 5,       // 日常活动
        FTYPE_TRAVEL_ACTIVITY = 6,      // 跨服活动
        FTYPE_TRAVEL_RANK = 7,			// 跨服排行
        FTYPE_ACTIVITY_END,

        STYPE_ACCU_RECHARGE = 1,        // 累积充值
        STYPE_REPEAT_RECHARGE = 2,      // 重复充值
        STYPE_ACCU_CONSUM = 3,          // 累积消费
        STYPE_SINGLE_RECHARGE_RANK = 4, // 单服充值排行
        STYPE_TRAVEL_RECHARGE_RANK = 5, // 跨服充值排行
        STYPE_ACTIVITY_END,

        CONDTYPE_SINGLE_EQUAL = 0,      // 单值相等
        CONDTYPE_SINGLE_GTE = 1,        // 单值大于等于
        CONDTYPE_SINGLE_LTE = 2,        // 单值小于等于
        CONDTYPE_RANGE = 3,             // 区间范围
        CONDTYPE_SINGLE_LIST = 4,       // 单值列表相等
        CONDTYPE_SINGLE_DIVID = 5,      // 单值整除
        CONDTYPE_END,

        REWARDTYPE_DAILY    = 1,        // 每天邮件自动发放未领取的
        REWARDTYPE_ACT_END  = 2,        // 活动结束时邮件自动发放未领取的

		UNDRAW					= 0,		//未领取
		DRAWED					= 1,		//已领取
		CNDRAW					= 2,		//不能领取

        END
    };
    int __act_id;
    int __first_type;   // 用于区分不同的活动界面，1 开服活动,2 返利活动, 3 合服活动, 4 节日活动, 5 日常活动, 7 跨服活动
    int __second_type;  // 用于区分不同的条件判断源, 1 累积充值, 2 重复充值, 3 累积消费, 4 单服充值排行, 5 跨服充值排行
    std::string __act_title;    // 活动标题
    std::string __act_content;  // 活动内容
    Time_Value __act_start;     // 活动开始时间
    Time_Value __act_end;       // 活动结束时间
    Time_Value __show_end;      // 活动面板消失时间
    int __is_open;              // 活动开启标识
    int __order;                // 显示顺序
    std::string __reward_mail_title;    // 奖励的邮件标题
    std::string __reward_mail_content;  // 奖励的邮件内容
    int __need_gold;			// 排行上榜限制

    Time_Value __update_tick;   // 更新时间

    typedef std::vector<ItemObj> ItemList;
    struct Reward
    {
        int __cond_type;            // 【后台暂不用配置，程序控制】条件类型: 0 单值相等,  1 单值大于等于, 2 单值小于等于, 3 区间范围, 4 单值列表相等, 5 单值整除
        IntVec __cond_list;     // 根据需要的条件值数量填写

        int __reward_type;      // 1 每天领取, 每天邮件发放未领取的, 条件值每天清除；
                                // 2 活动时间内可领取，结束时条件源值清除，未领取的邮件发放；
        ItemList __reward_item;  // 奖励道具列表

        int __restore_gold_rate;

        void reset(void);
    };
    typedef std::vector<Reward> RewardList;
    RewardList __reward_list;

    void reset(void);
    void serialize(Message *msg);
    bool is_recharge_activity(void);
    bool is_consum_activity(void);
};

extern bool JYBackActivityItemCmp(JYBackActivityItem * const &left, JYBackActivityItem * const &right);

// 玩家身上的后台活动领取记录
struct JYBackActivityPlayerRecord
{
    int __act_id;
    Int64 __act_start;      // 活动开始时间
    Int64 __act_end;        // 活动结束时间, 用于判断活动是否已结束需要自动发放奖
    std::string __mail_title;
    std::string __mail_content;

    Int64 __daily_refresh_tick; // 每日值清0时间
    int __daily_value;      // 每日增加的值，零晨清0
    int __single_cond_value;    // 活动时间内累加的值

    typedef std::map<int, int> RewardMap;
    typedef std::map<int, RewardMap> IndexRewardMap;
    IndexRewardMap __reward_value_map; // key1: reward index, key2: __cond_list 的值, value: 0 未领取，1 已领取, 2 不可领取奖励

    Time_Value __update_tick;   // 用于判断是否需要重新加载活动的奖励
    typedef std::vector<ItemObj> ItemVec;
    typedef std::map<int, ItemVec> IndexRewardItemMap;
    IndexRewardItemMap __index_reward_item_map;

    void reset(void);
    bool is_has_reward(void);
};

// 五一活动
struct MayActDetail
{
	struct PlayerInfo
	{
		PlayerInfo();
		void reset();
		Int64 role_id_;
		string name_;
		uint money_;
	};

	typedef std::map<Int64, PlayerInfo> LUMap;                        //一个红包保存抢了元宝的玩家信息
	typedef std::map<int, LUMap> IndexMap;						//各红包对应保存玩家信息，不同红包也会出现相同玩家信息
	typedef std::map<int, IndexMap> GroupMap;						//记录该组玩家获得的红包信息

	typedef std::vector<IntVec> RedPacketVec;                     //当前轮数某组随机红包集合
	typedef std::map<int, RedPacketVec> GroupPacketMap;              //该轮红包集合, IntVec总金额分配集合
	typedef std::map<int, GroupPacketMap> TimePacketMap;    //所有轮数红包集合

	enum
	{
		DAILY_LOGIN		= 1, 	//每日登录
		SEND_POCKET		= 2,	//红包派送
		DAILY_BUY		= 3,	//每日限购
		DAILY_RUN		= 4,	//天天跑酷
		NICE_FASHION	= 5,	//绝版时装
		DAILY_RECHARGE	= 6,	//每日充值
		COUPLE_HEART	= 7,	//夫妻同心
		LABOR_HONOUR	= 8,	//劳动光荣
		CHANGE_ITEM		= 9,	//神秘兑换

		ACT_START_DAY 	= 0501, //活动开启时间
		ACT_LAST_DAY	= 7,	//活动持续时间
		FRIEND_PAGE		= 5,	//好友助跑每页数量

		UNDRAW			= 0,	//未领取
		DRAWED			= 1,	//已领取
		CNDRAW			= 2,	//不能领取

		END
	};

	int open_flag_;			//开启状态
	Int64 refresh_tick_;	//配置写入mongo时间
	Int64 begin_date_;		//开始时间
	Int64 end_date_;		//结束时间
	int act_type_;			//后台配置的活动版本
	IntMap back_act_map_;	//后台开启的活动
	IntMap agent_;			//渠道列表，指定哪些渠道可以看到这些活动信息，如果为空：表示所有渠道都可以看到

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
		int pre_cost_;		//原价
		int now_cost_;		//现价
		int group_id_;		//组id
		ItemObj item_obj_;	//奖励
	};
	typedef std::map<int, SlotInfo> SlotInfoMap;
	typedef std::vector<SlotInfo> SlotInfoVec;

	struct ActReward
	{
		int cur_id_;		//索引id
		string name_;		//名字
		string content_;	//内容
		LongVec cond_;		//领取奖励的条件值

		int pre_cost_;		//原价
		int now_cost_;		//现价
		int times_;			//限制次数
		int show_id_;		//传闻id

		int task_id_; 		//任务id
		int reward_id_;		//奖励
		ItemObjVec change_item_;	//兑换物品集合
		int every_change_times_; 	//每天兑换次数
		int act_change_times_;		//活动期间兑换次数

		int open_day_;		//天数
		int group_weight_; 	//奖励权重

		int packet_count_; 	//红包总数
		int packet_money_;  //红包总金额
		int get_times_; 	//可抢人数
		int min_money_;		//红包最小金额
		int money_limit_;	//红包随机值控制范围
		int red_act_interval_;  //红包活动间隔时间

		//保存的数据
		LongMap sub_map_;					//记录
		LongMap drawed_map_;				//key: role_id, value: 是否领取
		LongMap act_drawed_map_;			//神秘兑换 活动期间兑换物品数量

		ActReward();
		void reset();
		void request_reset();
		bool arrive();
	};
	typedef std::vector<ActReward> ActRewardSet;

	struct LimitTime
	{
		int group_;
		int state_;
		int up_time_;
		int down_time_;
		LimitTime();
		void reset();
	};
	typedef std::vector<LimitTime> LimitTimeVec;

	struct RedActInfo
	{
		enum{ CLOSE, BEGIN, END, READY, FINISH };
		RedActInfo();
		int state_;		//红包活动状态 未开启，开启，结束，准备开启, 抢完
		int tick_;		//准备开启倒计时
		int money_;
		void reset();
	};
	typedef std::vector<RedActInfo> RedActInfoVec;

	struct CoupleInfo
	{
		Int64 wedding_id_;
		int online_tick_;
		int buy_tick_;

		CoupleInfo();
		void reset();
	};
	typedef std::map<Int64, CoupleInfo> CoupleInfoMap;

	struct RunInfo
	{
		Int64 role_id_;
		string name_;
		int sex_;
		Int64 tick_;
		int location_;

		RunInfo();
		void reset();
	};
	typedef std::map<Int64, RunInfo> RunRoleMap;

	typedef std::map<int, SlotInfoMap> DaySlotInfoMap;
	typedef std::map<int, SlotInfoVec> GroupSlotInfoMap;

	struct ActInfo
	{
		int act_index_;		//活动的索引
		string act_name_;	//活动名字
		string act_title_;	//标题

		int first_type_;	//活动编号
		int second_type_;	//活动第二类型
		int start_cond_;	//起始条件
		int mail_id_;		//邮件id
		int red_point_;		//红点id
		int send_reward_;	//赠送奖励
		int day_reset_;		//每天重置

		IntMap open_time_;	//开启时间
		IntVec limit_cond_;	//限制值
		ThreeObjVec rand_vec; //随机值
		int reward_show_id_;  //奖励展示id

		LimitTimeVec limit_time_vec_;			//活动开启时间段
		int red_packet_max_;					//红包最大数量
		GroupMap player_reward_info_;			//玩家红包信息

		TimePacketMap all_red_packet_map_;		//所有随机的红包组
		int cur_packet_act_times_;				//当前活动时间段索引 1开始
		int cur_time_act_open_state_;			//当前时间段活动开启状态记录
		RedActInfoVec red_act_info_vec_;		//各红包开启活动信息
		int cur_open_red_act_tick_;				//当前开启红包的时间

		GroupSlotInfoMap group_slot_map_;		//按组的格子信息表格
		IntMap group_weight_map_;				//整个奖励对应各格子权重（已计算各权重）
		IntMap group_pro_list_;		//分组权重
		IntMap group_may_be_list_;	//可出组出现的概率，个数为0，1，索引引用来原来的从0开始为第一组
		IntMap group_show_list_;	//必出
		IntMap group_no_show_list_;	//必不出
		IntMap group_limit_list_;	//出现个数上限

		ActRewardSet reward_set_;	//奖励详情

		//保存的数据
		int act_type_;		//活动版本

		CoupleInfoMap couple_info_map_;	//夫妻同心数据
		RunRoleMap run_role_map_;		//跑酷玩家数据

		ActInfo();
		void reset();
		void request_reset();
		void serialize_limit_cond(ProtoLimitValue *proto);
	};
	typedef std::vector<ActInfo> ActInfoSet;	//活动列表
	ActInfoSet act_info_set_;

	MayActDetail();
	void reset();

	void add_new_item(int index, const Json::Value& conf);
	void update_item(ActInfo* act_info, const Json::Value& conf);

	ActInfo* find_act_info(int act_index, int agent_code = -1);
	ActInfo* find_item_by_day(const PairObj& type, int day, int agent_code = -1);
	void refresh_act_open_time(int act_index);
	bool is_act_open_time(int act_type, int day);
	bool is_act_open_time_by_id(int act_index);

	SlotInfoVec* const fetch_slot_map_by_group(ActInfo* act_detail, int group_id);
	int fetch_slot_map_weight_by_group(ActInfo* act_detail, int group_id);
	SlotInfo* fetch_slot_info(ActInfo* act_detail, int slot_id, int group_id = 0);

	int act_left_tick();
	int act_total_day();
	int act_cur_day();
	int act_type();
};

//五一活动玩家数据
struct MayActivityer
{
	struct ShopSlot
	{
		MayActDetail::SlotInfo slot_info_;
		int is_buy_;
	};
	struct ActItem
	{
		ActItem()
		{
			this->arrive_ = 0;
			this->drawed_ = 0;
		}

		bool have_reward()
		{
			return this->arrive_ > this->drawed_;
		}

		int arrive_;
		int drawed_;	//arrive_和drawed_共同组成奖励的领取状态（例如在循环充值和重复消费里面，arrive_表示奖励的总的可领取次数，而drawed_表示奖励的已领取次数），具体情况，具体分析；
		IntMap arrive_map_;
	};
	typedef std::map<int, ActItem> ActItemMap;	//map<key, value>其中key和cur_index_对应奖励下标，value表示奖励的领取状态
	typedef std::map<int, ShopSlot> ShopSlotMap;
	typedef std::vector<ShopSlot> ShopSlotVec;
	struct ActTypeItem
	{
		ActTypeItem();
		void reset();
		void reset_every();
		int fetch_role_size(int type);

		int cur_index_;		//这里的cur_index_和MayActDetail::act_index_对应一致

		Int64 sub_value_;	//第一附加值，在天天跑酷中表示助跑剩余次数	在绝版时装中表示变更时装格子id 在劳动光荣中表示今天是否累计
		Int64 second_sub_;	//第二附加值，在天天跑酷中表示好友助跑剩余次数 在绝版时装中表示变更时装id奖励

		ActItemMap act_item_map_;	//每笔奖励的领取状态，map<key, value>其中key是上面的cur_index_对应，value是struct ActItem表示奖励的领取状态
		LongMap send_map_;			//好友赠送状态
		LongMap role_map_;			//显示角色，1：超前 0：落后

		IntMap group_refresh_times_map_; 	//刷新次数
		ShopSlotMap shop_slot_map_;			//玩家保存的格子信息
		ShopSlotVec shop_slot_vec_temp_;	//临时保存格子信息
		int refresh_tick_;					//刷新时间戳

		int liveness_;						//在绝版时装中表示活跃度
		int cur_fashion_times_;					//当前活动轮数
		int max_fashion_times_;				//最大轮数
		int max_cond_count_;				//第一轮最后一个索引

		std::map<int, IntMap> liveness_map_;	//活跃度奖励领取状态

		CornucopiaTaskMap labour_task_map_;		//劳动光荣任务列表
	};
	typedef std::map<int, ActTypeItem>	ActTypeItemMap;		//这里的key是MayActDetail::act_index_的值，表示当前奖励是哪个活动的,value是奖励的情况
	ActTypeItemMap act_type_item_set_;

	int act_type_;

	void reset(void)
	{
		this->act_type_ = 0;
	    this->act_type_item_set_.clear();
	}

};

#endif /* ACTIVITYSTRUCT_H_ */
