/*
 * PubStruct.h
 *
 * Created on: 2012-12-29 17:47
 *     Author: glendy
 */

#ifndef _PUBSTRUCT_H_
#define _PUBSTRUCT_H_

#include "GameHeader.h"

#include "GameEnum.h"
#include "GameError.h"
#include "GameDefine.h"
#include "SerialDefine.h"
#include "MessageNumber.h"

#include "Log.h"
#include "HashMap.h"
#include "GameTimer.h"
#include "ObjectPoolEx.h"

#include "GameCache.h"
#include "GameConfig.h"
#include "PoolPackage.h"
#include "FlowControl.h"
#include "EntityCommunicate.h"
#include "TransactionMonitor.h"

#ifdef LOCAL_DEBUG
#define L_ASSERT(X) assert(X)
#else	// no define LOCAL_DEBUG
#define L_ASSERT(X)
#include "Log.h"
#undef LOG
#define LOG     (Log::instance())
#endif	// def LOCAL_DEBUG

extern int string_utf8_len(const char *str, int *p_asccii_amount = NULL, int *p_chinese_amount = NULL);
extern size_t string_utf8_len_to_raw_size(const char *str, const int limit_len);
extern int chinese_utf8_len(const char *str);
extern int string_remove_black_char(char *dest, const int max_len, const char *src, const int src_len);
extern uint64_t des_encrypt(const uint64_t data, const char *key);
extern uint64_t des_decrypt(const uint64_t data, const char *key);
extern int generate_md5(const char *src, std::string &str_md5);
extern int generate_md5(const char *src, const int len, std::string &str_md5);
extern time_t convert_to_whole_hour(const Time_Value &tick);
extern time_t convert_to_midnight(const Time_Value &tick);
extern Message* create_message(const std::string& type_name);
extern Message* parse_message(const std::string &type_name, Block_Buffer *data_buff);
extern Message* parse_message(int recogn, Block_Buffer *data_buff);
extern int string_to_block_len(const std::string &data);
extern void split_long_to_int(const int64_t value, int32_t &i_low, int32_t &i_high);
extern int64_t merge_int_to_long(const int32_t i_low, const int32_t i_high);
extern int type_name_to_recogn(const std::string &type_name);
extern pthread_t gettid(void);
typedef void (*SigHandler)(int);
extern int register_signal(int sign, SigHandler handler);

extern Time_Value current_day(const int hour, const int minute, const Time_Value &nowtime = Time_Value::gettimeofday());
extern Time_Value current_week(const int week, const int hour, const int minute, const Time_Value &nowtime = Time_Value::gettimeofday());
extern Time_Value current_month(const int day, const int hour, const int minute, const Time_Value &nowtime = Time_Value::gettimeofday());
extern Time_Value current_year(const int month, const int day, const Time_Value &nowtime = Time_Value::gettimeofday());
extern int diff_day(const Time_Value &tick, const Time_Value &nowtime = Time_Value::gettimeofday());

extern Time_Value next_day(const int hour, const int minute, const Time_Value &nowtime = Time_Value::gettimeofday());
extern Time_Value next_week(const int week, const int hour, const int minute, const Time_Value &nowtime = Time_Value::gettimeofday());
extern Time_Value next_month(const int day, const int hour, const int minute, const Time_Value &nowtime = Time_Value::gettimeofday());
extern Time_Value next_year(const int month, const int day, const Time_Value &nowtime = Time_Value::gettimeofday());

class ProtoFightPro;
class BaseRoleInfo;
class OfflineRoleDetail;
class OfflineBeastDetail;

class Proto31400012;
class Proto51400601;
class Proto31401504;
class Proto80400216;

class ProtoSession;
class ProtoRoleInfo;
class ProtoBrocastRole;
class ProtoOfflineBeast;
class ProtoItem;
class ProtoSkill;
class ProtoThreeObj;
class ProtoSerialObj;
class ProtoMoney;
class ProtoFashionInfo;
class ProtoEquip;
class ProtoMailInfo;
class ProtoCustomerSVCRecord;

class ProtoEquipPolish;
class ProtoEquipPolishAttrInfo;
class ProtoEquipPolishStruct;
class ProtoShoutItem;
class PActTypeItem;
class ProtoMagicActBase;
class ProtoTeamer;
class ProtoServer;

enum SERVER_SCENE
{
    SCENE_LOGIC     = 101,
    SCENE_CHAT      = 201,
    SCENE_LOG       = 301,
    SCENE_SAND      = 501,
    SCENE_GATE      = 601,
    SCENE_AUTH      = 701,

    SCENE_END
};

enum ROUTE_TYPE
{
    BT_DIRECT_TARGET_SCENE = 1,
    BT_DIRECT_CLIENT = 2,
    BT_NOPLAYER_TARGET_SCENE = 3,
    BT_BROAD_CLIENT = 4,
    BT_BROAD_IN_GATE = 5,
    BT_DIRECT_TARGET_SCENE_BACK = 6,
    BT_DIRECT_TARGET_LINE_SCENE = 7,
    
    BT_END
};

enum SERVER_TYPE
{
    SERVER_DAEMON   = 0,
    SERVER_LOGIC    = 1,
    SERVER_CHAT     = 2,
    SERVER_LOG      = 3,
    SERVER_MAP      = 4,
    SERVER_SAND     = 5,
    SERVER_GATE     = 6,
    SERVER_AUTH     = 7,
    
    SERVER_TYPE_END
};

// 必须按所在模块，用连续编号来定义类型
enum GAME_TIMER_TYPE
{
	//逻辑服
    GTT_LOGIC_TYPE_BEG      = 100,
    GTT_LOGIC_PLAYER        = 101,		//间隔0.5秒
    GTT_LOGIC_TRANS         = 102,		//间隔10秒
    GTT_LOGIC_MONITOR       = 103,		//间隔20秒
    GTT_LOGIC_ONE_SEC		= 104,		//间隔1秒钟
    GTT_LOGIC_ONE_MINUTE	= 105,		//间隔1分钟
    GTT_LOGIC_ONE_HOUR		= 106,		//间隔1小时
    GTT_LOGIC_TYPE_END,

    //聊天服
    GTT_CHAT_TYPE_BEG       = 200,
    GTT_CHAT_PLAYER         = 201,
    GTT_CHAT_TRANS          = 202,
    GTT_CHAT_CHANNEL        = 203,
    GTT_CHAT_ONE_SEC 		= 204,
    GTT_CHAT_TEN_SEC		= 205,
    GTT_CHAT_ONE_MINUTE		= 206,
    GTT_CHAT_TYPE_END,

    //地图服
    GTT_MAP_TYPE_BEG        = 400,
    GTT_MAP_PLAYER          = 401,		//战斗线程间隔0.1秒
    GTT_MAP_MONSTER         = 402,		//战斗线程间隔0.3秒
    GTT_MAP_TRANS           = 403,		//战斗线程间隔10秒
    GTT_MAP_BROAD           = 404,		//战斗线程间隔0.01秒
    GTT_ML_ONE_SECOND       = 405,		//逻辑线程间隔1秒
    GTT_MAP_ONE_SECOND		= 406,		//战斗线程间隔1秒
    GTT_ML_ONE_MINUTE       = 407,		//逻辑线程间隔1分钟
    GTT_MAP_ONE_MINUTE		= 408,		//战斗线程间隔1分钟
    GTT_MAP_TYPE_END,

    //网关
    GTT_GATE_TYPE_BEG       = 500,
    GTT_GATE_PLAYER         = 501,		//1秒
    GTT_GATE_TRANS          = 502,		//10秒
    GTT_GATE_TYPE_END,

    GTT_END
};

enum MSG_TYPE
{
	/*消息来自客户端*/
    MSG_TYPE_CLIENT_TO_LOGIC    	= 101,
    MSG_TYPE_CLIENT_TO_CHAT     	= 102,
    MSG_TYPE_CLIENT_TO_MAP      	= 104,
    MSG_TYPE_CLIENT_TO_GATE			= 106,
    MSG_TYPE_CLIENT_TO_AUTH			= 107,
    MSG_TYPE_CLIENT_TO_MAP_LOGIC 	= 114,

    /*内部消息*/
    MSG_TYPE_INNER_TO_LOGIC     	= 301,
    MSG_TYPE_INNER_TO_CHAT      	= 302,
    MSG_TYPE_INNER_TO_LOG_LOG   	= 3031,
    MSG_TYPE_INNER_TO_LOG_MYSQL 	= 3033,
    MSG_TYPE_INNER_TO_MAP       	= 304,
    MSG_TYPE_INNER_TO_MAP_LOGIC 	= 314,
    MSG_TYPE_PHP_TO_MAP				= 308,
    MSG_TYPE_PHP_TO_MAP_LOGIC		= 318,
    
    MSG_TYPE_END
};

enum TRANS_DATA_TYPE
{
    // rollback data type {{{
    ROLL_ITEM           = 1,
    // }}}

    // database communicate data type {{{
    DB_BLOCK_BUFF           = 10001,
    DB_LOGIC_PLAYER         = 10002,
    DB_MAP_PLAYER           = 10003,
    DB_GATE_PLAYER			= 10004,
    DB_MAP_LOGIC_PLAYER     = 10005,

    DB_CHAT_LEAGUE     		= 10006,
    DB_CHAT_LOUDSPEAKER     = 10007,
    DB_CHAT_PRIVATE     	= 10008,
    DB_CHAT_AGENCY     		= 10009,
    DB_TRADE_MODE			= 10010,

    DB_MAIL_INFO			= 10011,
    DB_MAIL_BOX				= 10012,
    DB_MONGO_DATA_MAP       = 10013,
    DB_SHOP_LOAD_MODE		= 10014,
    DB_FRIEND_INFO_DETAIL   = 10015,
    DB_MALL_ITEM_DETAIL     = 10016,
    DB_SCRIPT_PLAYER_REL    = 10017,
    DB_RPM_RECOMAND_INFO	= 10018,
    DB_BACK_RECHARGE_ORDER  = 10019,
    DB_BACK_ACTI_CODE		= 10020,
    DB_RECHARGE_TIME_RANK	= 10021,
    // }}}

    TRANS_DATA_TYPE_END
};

enum CHAT_ENUM
{
	R_WORD 				= 1,			//文字
	R_VOICE 			= 2,			//语音消息
	R_VOICE_SAVE 		= 3,			//语音内容
	R_BROCAST_INFO		= 4,			//广播内容
	R_FLAUNT            = 5,            //炫耀
	R_BACK_STAGE_ANNOUNCE = 6,          //后台广播
	R_POSITION			= 7,			//坐标广播

	CHANNEL_WORLD 		= 1,			//世界
	CHANNEL_LEAGUE 		= 2,			//帮派
	CHANNEL_TEAM 		= 3,			//队伍
	CHANNEL_SYSTEM		= 4,			//系统
	CHANNEL_TRAVEL 		= 5,			//跨服
	CHANNEL_PRIVATE 	= 6,			//私聊
	CHANNEL_SCENE 		= 7,			//场景
	CHANNEL_LOUDSPEAKER	= 8,			//喇叭

	// 顶部滚动广播/后台广播
	BANNER_BROCAST_JUST_TOP 	= 0,		// 只在顶部显示
	BANNER_BROCAST_JUST_SYS		= 1,		// 只在系统广播里显示
	BANNER_BROCAST_TOP_AND_SYS	= 2,		// 同时在顶部和系统广播里显示

	CHANNEL_END
};


enum VIP_TYPE
{
	VIP_NOT_VIP = 0,			//非VIP
	VIP_1		= 100,			//VIP1
	VIP_2		= 101,			//VIP2
	VIP_3		= 102,			//VIP3
	VIP_4		= 103,			//VIP4
	VIP_5		= 104,			//VIP5
	VIP_6		= 105,			//VIP6
	VIP_7		= 106,			//VIP7
	VIP_8		= 107,			//VIP8
	VIP_9       = 108,			//VIP9
	VIP_10      = 109,			//VIP10
	VIP_11      = 110,			//VIP11
	VIP_12      = 111,			//VIP12
	VIP_MAX		= VIP_12
};

enum RewardState{
	REWARD_NONE = 0,
	REWARD_HAVE = 1,
	REWARD_GONE = 2	// 已领取
};

enum RewardType{
	DAILY_1 = 1,
	DAILY_2 = 2,
	DAILY_3 = 3,
	DAILY_4 = 4,
	DAILY_5 = 5,
	DAILY_6 = 6,
	DAILY_7 = 7,
	INVEST_TYPE_NUM
};

enum ENTER_SCENE_TYPE
{
    ENTER_SCENE_LOGIN     = 1,
    ENTER_SCENE_TRANSFER  = 2,
    ENTER_SCENE_RELIVE    = 3,
    ENTER_SCENE_JUMP	  = 4,
    ENTER_SCENE_EXIST	  = 5,
    ENTER_SCENE_PEAK_WAR_JUMP = 6,

    ENTER_SCENE_END
};

enum WEDDING_SYS_TYPE
{
	WED_RING	= 1,	//婚戒
	WED_SYS		= 2,	//姻缘
	WED_TREE	= 3,	//爱情树
	WED_TYPE_END
};

struct InnerRouteHead
{
    int32_t __recogn;
    int64_t __role_id;
    int32_t __broad_type;
    int32_t __scene_id;
    int32_t __inner_req;
    int32_t __line_id;

    InnerRouteHead(const int recogn = 0, const Int64 role_id = 0, const int broad_type = 0, const int scene_id = 0, const int inner_req = 0);
    void reset(void);
};

struct ProtoClientHead
{
    int32_t __recogn;
    int32_t __error;

    ProtoClientHead(void);
    void reset(void);
};

struct ProtoHead
{
    int32_t __recogn;
    int32_t __error;
    int32_t __trans_id;
    int32_t __scene_id;
    int64_t __role_id;

    int __src_line_id;
    int __src_scene_id;

    ProtoHead(const int recogn = 0, const int error = 0, const int trans_id = 0, const int scene_id = 0, const Int64 role_id = 0);
    void reset(void);
};

struct UnitMessage
{
    enum UNIT_MSG_TYPE
    {
        TYPE_BLOCK_BUFF     = 1,
        TYPE_IVALUE   		= 2,
        TYPE_PROTO_MSG      = 3,

        TYPE_END
    };

    int32_t __type;
    int32_t __sid;
    uint32_t __len;

    ProtoHead __msg_head;
    InnerRouteHead __route_head;

    union {
    	int __i_val;
        void* __argv;
        Block_Buffer *__buf;
        Message *__proto_msg;
    } __data;

    void set_msg_proto(Message *msg_proto);
    void set_data_buff(Block_Buffer *buff);
    void set_ivalue(const int value);

    bool need_adjust_player_scene();

    UnitMessage(void);
    void reset(void);

    int ivalue(void);
    Message *proto_msg(void);
    Block_Buffer *data_buff(void);
};

struct BaseServerInfo
{
	BaseServerInfo();

	void reset();
	void set_cur_server_flag();
	void set_server_flag(const string& server_flag);
	void serialize(ProtoServer* server_info, int include_cur = false);
	void unserialize(const ProtoServer& proto_server);
	void unserialize(const BaseServerInfo& server_info);

	string fetch_flag_by_trvl();

	int __server_id;			// 服务器标识
	string __server_flag;		// 服务器标识字符串格式
	string __server_prev;  		// 服务器标识前缀
	string __server_name;		// 服务器名字
	string __cur_server_flag;	// 特殊用途
};

struct ServerInfo : public BaseServerInfo
{
    int __is_in_use;    // 是否在用, 默认是true
    int __combine_to_server_id; // 合服后所在的服务器标识

    string __back_server_name;	// 后台显示的服务器名字
    Time_Value __open_server;   // 开服时间

    IntVec __combine_set; // 合服源标识
    bool __has_collect_combine_set; // 用于递归收集合服源时使用

    ServerInfo(int in_use = 1);
    void reset(void);
};

struct GlobalIndex
{
	static int global_team_index_;
	static int global_market_index_;
	static int global_scene_hash_id_;
	static int global_item_hash_id_;
};

struct GameName
{
	static std::string SPEED;		//速度

	//一级属性
	static std::string POWER;		// 力量
	static std::string STAMINA;		// 筋骨
	static std::string PHYSICA;		// 体质
	static std::string AGILITY;		// 敏捷
	static std::string BODYACT;		// 身法

	//二级属性
	static std::string BLOOD_MAX;			//血量上限
	static std::string MAGIC_MAX;			//魔法上限
	static std::string ATTACK;				//攻击
	static std::string DEFENSE;				//防御
	static std::string HIT;					//命中
	static std::string AVOID;				//闪避
	static std::string CRIT;				//暴击
	static std::string TOUGHNESS;			//韧性
	static std::string PK_VALUE;			//pk值
	static std::string GLAMOUR;				//魅力值
	static std::string LUCK;				//幸运值
	static std::string ATTACK_MAX;			//攻击上限
	static std::string ATTACK_MIN;			//攻击下限
	static std::string DEFENCE_MAX;			//防御上限
	static std::string DEFENCE_MIN;			//防御下限
	static std::string CRIT_HURT_MULTI;		//暴击伤害倍数(万分比)
	static std::string DAMAGE_MULTI;		//伤害加成倍数(万分比)
	static std::string REDUCTION_MULTI;		//伤害减免倍数(万分比)
	static std::string DAMAGE;				//伤害加成
	static std::string REDUCTION;			//伤害减免
	static std::string HIT_KLV;				//命中KLV
	static std::string AVOID_KLV;			//闪避KLV
	static std::string CRIT_KLV;			//暴击KLV
	static std::string TOUGHNESS_KLV;		//韧性KLV

	//福利
	static std::string WELCONTRI;	// 宗派福利--贡献
	static std::string WELGOODS;	// 宗派福利--物品
	static std::string WELCOPPER;	// 宗派福利--铜钱

	// 怪物产生方式
	static std::string CENTER_BORN;		//中心点模式
	static std::string FIXED_BORN;		//固定点模式
	static std::string AREA_BORN;		//Area in SPT
	static std::string UNLIMITED_BORN;	//不限次数

	// 经验找回阶段类型
	static std::string ER_STAGE_PASS_THROUGHT; // 通关一次后即可找回经验
	static std::string ER_STAGE_PASS_CHAPTER; // 根据通关的章计算可找回的次数
	static std::string ER_STAGE_PASS_FLOOR;	  // 根据最高通过的层数计算可找回的经验
};

struct PageInfo
{
	PageInfo(void);
	void reset(void);

	//当前请求页
	int cur_page_;
	//总页数
	int total_page_;

	//开始索引，从0开始
	int start_index_;
	//总数
	int total_count_;

	uint add_count_;
};

struct ActivityTimeInfo
{
	ActivityTimeInfo(int day_check = true);

	void reset(void);
	void set_freq_type(int type);
	void set_week_day(const IntVec& week_day);

	void set_next_stage();
	void set_next_one_state();
	void set_next_two_stage();
	void set_next_three_stage();

	int fetch_time_group();
	int fetch_last_index();
	int fetch_next_cycle_time();

	int fetch_left_time();
	int fetch_two_left_time();
	int fetch_three_left_time();

	int fetch_start_time();
	int fetch_two_start_time();
	int fetch_three_start_time();

	int fetch_end_time();	//上次结束时间
	int fetch_after_end_left_time();//结束之后，到下一次未开始，还有多久

	int client_state();
	int finish_type();
	int today_activity();

	int cur_state_;
	int freq_type_;
	int day_check_;		//初始默认为true
	int active_time_;

	int time_index_;	// start from 0
	int time_span_;		//1 or 2 or 3
	int time_cycle_;
	int refresh_time_;
	IntVec time_set_;
	IntMap week_day_;
};
typedef std::map<int, ActivityTimeInfo> TimeInfoMap;

struct SerialObj
{
	SerialObj(const ProtoSerialObj& proto_serial);
	SerialObj(int type = 0, int sub = 0, int value = 0);

	void serialize(ProtoSerialObj* proto_serial) const;
	void unserialize(const ProtoSerialObj& proto_serial);

	int type_;
	int sub_;
	int value_;
};

struct ItemObj
{
	ItemObj(const ProtoItem& proto_item);
	ItemObj(int id = 0, int amount = 0,
			int bind = false, int index = 0);

	void reset(void);

	void serialize(ProtoItem* proto_item, int type  = 0) const;
	void unserialize(const ProtoItem& proto_item);
	void unserialize(PackageItem* item);

	bool validate() const;

	int id_;
	int amount_;
	int bind_;

	int index_;
	int rand_;		// 概率
	int rand_start_times_;	//多少次后开始概率
	int no_rand_times_;		//多少次后必定出现
    int refine_level_; 	// 装备强化等级
};

typedef std::vector<ItemObj> 	ItemObjVec;
typedef std::map<int ,ItemObj> 	ItemObjMap;

struct ItemIndexObj
{
	ItemIndexObj(int index = 0, int amount = 0)
	{
		this->index_ = index;
		this->amount_ = amount;
	}

	int index_;
	int amount_;
};

struct SessionDetail : public GameTimer
{
    std::string __account;
    std::string __address;
    int __port;
    Time_Value __timeout_tick;
    int64_t __role_id;
    std::string __session;
    int __client_sid;

    SessionManager *__manager;

    void reset(void);
    virtual int type(void);
    virtual int handle_timeout(const Time_Value &nowtime);
};

struct Money
{
    Int64 __gold;		//元宝
    Int64 __bind_gold;	//绑定元宝
    Int64 __copper;
    Int64 __bind_copper;

    void serialize(ProtoMoney *proto_money) const;
    void unserialize(const ProtoMoney *proto_money);
    void unserialize(const ProtoMoney& proto_money);

    explicit Money(const Int64 gold = 0, const Int64 bind_gold = 0,
    	const Int64 copper = 0, const Int64 bind_copper = 0);
    explicit Money(const ProtoMoney& proto_money);

    Money &operator -=(const Money &money);
    Money &operator +=(const Money &money);

    void reset(void);
};
extern bool operator > (const Money &a, const Money &b);
extern bool operator >= (const Money &a, const Money &b);
extern bool operator < (const Money &a, const Money &b);
extern bool operator <= (const Money &a, const Money &b);
extern bool operator == (const Money &a, const Money &b);
extern bool operator != (const Money &a, const Money &b);
extern Money operator + (const Money &a, const Money &b);
extern Money operator - (const Money &a, const Money &b);

struct MoldingSpiritDetail
{
	MoldingSpiritDetail();
	int fetch_nature_level(int type);
	int update_nature_level(int type, int level, bool is_set = false);
	int fetch_nature_schedule(int type);
	int update_nature_schedule(int type, int schedule, bool is_set = false);
	int check_up_all_nature();

	void reset();
	MoldingSpiritDetail* copy(const MoldingSpiritDetail &detail);
	MoldingSpiritDetail* operator=(const MoldingSpiritDetail& rhs);

	int __red_grade;
	IntMap __nature_level_map;		//属性等级列表
	IntMap __nature_schedule_map;	//属性进度		达到当前等级最大进度后，提升一级，然后从0开始
};


typedef std::multimap<int, int>	JewelMap;	//<宝石id，绑定状态>,使用multimap而不是map主要是为了扩展以后策划修改需求（修改需求为同一装备可以镶嵌id相同的宝石（绑定和非绑定之分））

struct Equipment
{
	int strengthen_level_;
	int __refine_degree;

	ItemObjMap __good_refine;	//精炼
	IntMap __jewel_detail;		//装备镶嵌宝石的详情 key: type, value: jewel id

	JewelMap __special_jewel;	//first宝石id，second宝石绑定状态，装备镶嵌特殊宝石详情（强化24级的装备有个孔可以镶嵌任意类型的宝石）
	int __luck_value;	//装备的幸运属性值；

	MoldingSpiritDetail __molding_detail;		//铸魂

	Equipment(void);
    Equipment* operator=(const Equipment& rhs);

	void reset(void);
	void fetch_refine(IntMap& refine_map);
	void set_refine(IntMap& refine_map);
	void serialize(ProtoEquip *msg_proto, PackageItem *package_item);
    void unserialize(const ProtoEquip *msg_proto, PackageItem *package_item);
};

struct RecordEquipObj
{
	RecordEquipObj(void);
	RecordEquipObj(int type, PackageItem* item);

	void reset(void);
	void set_attr_data(PackageItem* item);

	int pack_type_;
	int equip_id_;
	int equip_index_;
	int equip_bind_;
	int strengthen_level_;	//强化等级
	int refine_degree_;
	int luck_value;//幸运值（武器才有）

	IntMap jewel_list;	//镶嵌宝石列表
	IntMap good_refine;	//精炼

	MoldingSpiritDetail molding_detail_;

//	EquipPolishInfo cur_polish;		//当前洗练属性
//	EquipPolishInfo extern_attr;	//极品属性
};

struct RotaryTableInfo
{
	int prop_index_;
	void reset();
};

struct FightProperty
{
	FightProperty(int type = 0);

	void reset();
	void set_type(int type);
	void add_multi_times(int multi_times, int type = 1);

	int caculate_force(int sub = 0);

	void serialize(IntMap& prop_map);
	void serialize(ProtoFightPro* proto);
    void unserialize(const ProtoFightPro& proto);
    void unserialize(IntMap& prop_map);

	//属性名
	void make_up_name_prop(const Json::Value& prop_json, int times = 1, int value = 0);
	//属性ID
	void make_up_id_prop(const Json::Value& prop_json);
	//单个属性
	void make_up_one_prop(const string& prop_name, int prop_add, int times = 1, int value = 0);

	void add_fight_property(const FightProperty& fight_prop);

	static void add_fight_property(int& cur_prop, int conf_value, const IntPair& pair);

	int type_;
	int force_;

	int blood_max_;
	int magic_max_;
	int blood_;
	int magic_;

	int attack_;
	int defence_;
	int avoid_;
	int hit_;
	int crit_;
	int toughness_;
	int damage_;		// 伤害加成
	int reduction_;		// 伤害减免

	int crit_hurt_multi_;	//暴击伤害倍数
	int damage_multi_;		//伤害加成倍数
	int reduction_multi_;	//伤害减免倍数
};

struct PackageItem
{
    int __index;		//GamePackage位置, 从0开始
    int __amount;
    int __hash_id;		//唯一自动分配ID

    int __id;			//物品种类ID
    int __bind;
    int __force_flag;

    int __use_times;
    int __new_tag;

    Int64 __use_tick;
    Int64 __unique_id;
    Int64 __timeout;	//失效时间

    IntMap	__tips_time_map;	//提示时间
    IntMap	__tips_status_map;	//该时间段是否提示
    int __tips_level;			//神接提示等级

    Equipment __equipment;
	FightProperty __prop_info; //装备属性
    RotaryTableInfo __rotary_table;

    bool validate_item();
    bool time_item();
    bool expire_item();
    bool is_equipment();

	void set(int id, int amount, int bind, int index = 0);
	void caculate_fight_prop();	//战骑类装备
	void caculate_fight_prop(int role_lvl, int strength_lvl);	//身上装备
    void serialize(ProtoItem *msg_proto, int new_index = -1, int set_num = 0);

    void unserialize(const ProtoItem *msg_proto);
    void unserialize(const ProtoItem& msg_proto);
    void unserialize(const ItemObj& obj, int index = -1);

    int red_grade();
    IntPair equip_part();

    const Json::Value& conf();
    PackageItem &operator=(const PackageItem &item);

    PackageItem(void);
    void reset(void);
};

struct ShopItem
{
	int item_id_;
	int item_pos_;
	int item_bind_;

	IntVec item_type_;
	int shop_type_;

	int money_type_;
	int src_price_;
	int cur_price_;

	Int64 start_tick_;
	Int64 end_tick_;
	IntPair need_item_;

	int max_item_; //每天限购总量
	int max_total_; //总共限购量

	string content_;

	ShopItem(void);
	void reset(void);
	bool check_time();
};

struct SerialInfo
{
    int64_t __fresh_tick;
    int64_t __value;
    int64_t __copper;
    int64_t __gold;
    int64_t __bind_gold;
    int64_t __bind_copper;

    SerialInfo(void);
};

struct DBInputArgv
{
	int type_int_;
	void* type_void_;

	Int64 type_int64_;
	string type_string_;

	IntVec extra_int_vec_;
	LongVec extra_long_vec_;

	void reset();
};

struct DBOutPutArgv
{
	int type_int_;
	void* type_void_;

	Int64 type_int64_;
	string type_string_;

	IntVec int_vec_;
	VoidVec void_vec_;
	LongVec long_vec_;
	StringVec str_vec_;

	BSONObj* bson_obj_;
	BSONVec* bson_vec_;

	DBOutPutArgv(void);
	~DBOutPutArgv(void);
	void reset();
};

struct DBShopMode
{
	int recogn_;
	int sub_value_;

	DBInputArgv input_argv_;
	DBOutPutArgv output_argv_;

	DBShopMode(void);
	void reset(void);
};

struct DBTradeMode
{
	BSONObj* query_;
	BSONObj* content_;

	std::string table_name_;
	bool operate_flag_;

	DBTradeMode(void);
	~DBTradeMode(void);
	void reset();
};

struct BaseMember
{
	enum
	{
		TRAVEL_SCRIPT_TEAMER	= 1,
		END
	};

	BaseMember(int type = 0);
	void reset();
	void serialize(ProtoTeamer* proto);
	void unserialize(const ProtoTeamer& proto);
	void unserialize(const BaseRoleInfo& role, int fashion = 0);

	Int64 id_;			//玩家ID
	string name_;		//玩家名字
	string prev_;		//区服

	int type_;
	int sex_;			//玩家性别
	int force_;			//战力
	int level_;			//等级
	int vip_type_;		//VIP等级
	int fashion_;		//时装
	int wing_;			//翅膀
	int solider_;		//神兵
};

struct BaseRoleInfo : public BaseServerInfo
{
	BaseRoleInfo(void);

	void reset(void);
	void set_name(const string& src_name);
	void set_sex();
	void set_create_tick(time_t create_tick);
	void make_up_teamer_info(Int64 role_id, ProtoTeamer* teamer_info);
	void inc_adjust_login_days();

    void set_open_tick(Int64 tick);
    void set_combine_tick(Int64 tick);

	const char* name() const;
	bool is_validate_permi(int permi);
	bool validate_server_open_day(int need_day);
	bool validate_max_combine_day(int max_day);

    string __name;
    string __src_name;

    string __account;
    string __agent;
    int __agent_code;
    int __market_code;
    int __permission;	// 权限类型； GameEnum::PermissionType

    int __sex;
    int __career;
    int __level;
    int __vip_type;
    int __fight_force;
    int __scene_id;
    int __recharge_total_gold;

    Int64 __id;	//角色id
    Int64 __league_id;//宗派id

    Int64 __partner_id;	//结婚伴侣ID
    string __partner_name; //结婚伴侣名字
    int	__wedding_type;		//结婚类型
    Int64 __wedding_id;		//结婚id 没有为0

    int __label_id;//称号id
    int __team_id;//队伍id

    Int64 __login_tick;	// 最近登录时间
    Int64 __login_days; 	// 登录总天数
    Int64 __last_logout_tick;	// 最近登出时间

    Int64 server_tick_;	//开服时间
    Int64 combine_tick_;//合服时间

    CreateDaysInfo create_info_;	//创号时间
};

struct ReplacementRoleInfo //化身信息
{
	ReplacementRoleInfo(void);
	void reset(void);

	int64_t __role_id;
    char __name[MAX_NAME_LENGTH + 1];
    int __sex;
    int __career;
    int __level;
    int __vip_type;
    int __fight_force;
};

struct RpmRecomandInfo
{
	int __leader_force;
	LongMap __teamates_id;

	int __info_count;
	ReplacementRoleInfo __role_info_list[RPM_LIST_LENGTH];

	RpmRecomandInfo();
	void reset(void);
};

struct RewardInfo
{
	int id_;				//配置奖励ID
	int adjust_;			//是否对item_id进行转化
	int exp_;				//经验
	int contri_;			//帮贡
	int select_index_;		//选择的index

    Money money_;			//金钱
    IntMap resource_map_;	//特殊资源
    ItemObjVec item_vec_;	//道具
    MapPlayer* player_;		//玩家
    MapLogicPlayer* logic_player_;	//玩家

    RewardInfo(int adjust = true, MapPlayer* player = NULL, MapLogicPlayer* logic_player = NULL);
    const Json::Value& conf() const;

    int has_player();
    int history_use_times(int id);
    int add_use_times(int id);
    int remve_use_times(int id);

    void reset();
    void set_player(MapLogicPlayer* player);
    void add_rewards(const ItemObjVec& obj_vec);
    void add_rewards(const ProtoItem& proto_obj);

    void add_rewards(const ItemObj& obj);
    void add_adjust_rewards(const ItemObj& obj);
    void add_nonadjust_rewards(const ItemObj& obj);
    void add_pack_items(const ItemObj& obj);
};

struct MailInformation
{
    Int64 mail_index_;
    Int64 send_time_; // 发件时间
    Int64 read_tick_; // 读邮件时间

    int mail_type_; // 邮件类型
    int mail_format_; // 邮件格式
    int has_read_; // 未读为0，已读为1

    Int64 sender_id_;
    string sender_name_; //发件人角色名
    int sender_vip_;	//发件人vip等级，-1表示系统邮件

    Int64 receiver_id_; // 收件人id
    string receiver_name_; // 收件人姓名

    string mail_title_;
    string mail_content_;

    //构造邮件内容
    char makeup_content_[GameEnum::MAX_MAIL_CONTENT_LENGTH + 1];

    int label_id_;			//称号
    Money money_;			//金钱
    IntMap resource_map_;	//特殊资源

    ItemListMap goods_map_; //key : mail_index , value : PackageItem*
    IntMap attach_map_; 	//key : mail_index , value : amount

    MailInformation(void);

    void reset(void);
    void fetch_goods_index(IntMap& goods_index);

    void add_goods(const ItemObjVec& obj_vec);
    void add_goods(const ItemObj& obj);
    void add_goods(int item_id, int item_amount, int item_bind = false);
    void add_money(int money_type, int amount);
    RewardInfo add_goods(int reward_id);

    void serilize(ProtoMailInfo* proto_mail);
    void unserilize(const ProtoMailInfo* proto_mail);

    void recycle_self();
    void recycle_goods();
};

class FixedTimer : public GameTimer
{
public:
	FixedTimer(int timer_type, int schedule_sec);

	int schedule_timer();
	virtual int type(void);

private:
	int timer_type_;
	int schedule_sec_;
};

struct FriendInfo
{
	int64_t __role_id;
	int64_t __icon_id;
	int64_t __league_id;
	int __friend_type;
	int __vip_status;
	int __is_online;
	int __sex;
	int __scene_id;
	int __career;
	int __level;
	int __team_status;
	string __name;
	int __stranger_tick;
	int __black_tick;
	int __enemy_tick;
	int __nearby_tick;
	int __force;
	int __intimacy;
	int __name_color;

	FriendInfo(void);
	void reset(void);
};

typedef std::vector<FriendInfo> FriendInfoSet;
struct DBFriendInfo
{
	int __friend_type;
	FriendInfoSet __friend_info_vec;
	LongSet __offine_set;

	DBFriendInfo(void);
	void reset(void);
};


struct MallActivityDetail
{
	int __activity_id;
	int __open_activity;//1:yes 0:no
	int __daily_refresh_type; //1:每日刷新  0:非每日刷新
	int __refresh_tick;
	int __last_save_tick;//save db interval
	int __limit_type; //101 全服限量, 201 个人限量, 301 全服限量+个人限量
	int __server_limit_amount;//全服限量
	int __single_limit_amount;//个人限量
	bool __data_change;//1:yes 0:no
	std::string __activity_name;
	std::string __activity_memo;

	IntMap __server_buy_map;

	typedef std::map<Int64, IntMap> MallBuyRecord; //key: role_id, value: (left:goods_id, right:goods_amount)
	MallBuyRecord __player_record;

	MallActivityDetail(void);
	void reset(void);
};

typedef std::map<int, int> ShapeDetail;

struct AchieveDetail
{
	int achieve_id_;	//成就id
	int ach_index_;		//成就父id(无用，改为不记录)
	int get_status_;	//获取状态：0不可获取，1：可获取，2：已获取
	int finish_num_;	//完成数量
	Int64 finish_tick_; //完成时间
	Int64 special_value_; //特殊值，部分成就会用到

	AchieveDetail(void);
	void reset();
};

struct InvestRechargeDetail
{
	InvestRechargeDetail();

	void reset();
	int cur_max_days();
	int is_passed_max_days();

	int __is_buy; 			//是否购买
	int __vip_level;

	CreateDaysInfo __buy_time;		//购买时间
	IntMap __invest_rewards; //普通领奖状态；
	IntMap __vip_rewards; 	//vip领奖状态;

};
typedef std::map<Int64, InvestRechargeDetail> InvestMap;

struct BaseVipDetail
{
	int __vip_type;
	int __vip_level;
	Int64 __expired_time;
    Int64 __start_time;

	BaseVipDetail();
	const Json::Value& conf();

	bool is_vip();
	bool is_max_vip();

	void set_vip_type(int vip_type);
	void reset();
};

struct ShoutInfo
{
	int id_;

	Int64 role_;
	Int64 group_;

	ShoutInfo(int id, Int64 role = 0)
	{
		this->id_ 	= id;
		this->role_ = role;
		this->group_= 0;
	}
};

struct BrocastRole
{
	Int64 __role_id;
	char __role_name[MAX_COMMON_NAME_LENGTH + 1];
	int __team_status;

	void reset(void);
};

struct BrocastPara
{
	int __parse_type;
	union
	{
		int __parse_int;
		Int64 __parse_int_64;
		char __parse_string[MAX_COMMON_NAME_LENGTH + 1];
		BrocastRole __parse_role_info;
	}__parse_data;

    ProtoShoutItem *__shout_item;

    BrocastPara(void);
    ~BrocastPara(void);
	void reset();
    BrocastPara &operator=(const BrocastPara &para);
    BrocastPara(const BrocastPara &para);
};

struct PlayerAssistTip
{
	int __event_id; 	//系统提示事件ID
	int	__tips_flag;	//提示值

    PlayerAssistTip(void);
	void reset(void);
	void serilize(ProtoPairObj* proto);
};

struct TipsPlayer
{
	TipsPlayer(EntityCommunicate* player);
	~TipsPlayer();

	void push_tips(int type, int id, int amount = 1);
	void push_goods(const ItemObjVec& item_set);
	void push_goods(int id, int amount);
	void push_money(const Money& money);
	void push_tips_str(const char* str);
	void push_tips_msg_id(int msg_id, ...);

	std::auto_ptr<Proto80400216> tips_info_;
	EntityCommunicate* player_;
};

struct SysSetting
{
	enum
	{
		SCREEN_SMOOTH        =         1,
		SCREEN_BALANCE,
		SCREEN_PERFECT,

		SCREEN_TYPE_END
	};
/*
 *  shield_type:
	1、屏蔽其他玩家的仙羽
	2、屏蔽其他玩家的技能特效
	3、屏蔽其他玩家的称号
	4、屏蔽其他玩家的宠物
	5、屏蔽怪物名称
	6、屏蔽掉落道具名称
	7、屏蔽同屏玩家模型5
	8、屏蔽同屏玩家模型10
	9、屏蔽同屏玩家模型20
	10、屏蔽所有玩家
	11、仅显示可攻击对象
*/

	int __is_shock;              //是否振动
	int __screen_type;           //智能模式
	int __fluency_type;           //流畅模式
	IntVec __shield_type;           //屏蔽类型
	int __turnoff_act_notify;    //是否关闭活动提示，1关闭，0开启
	int __auto_adjust_express;	 //是否自动调节游戏表现 1关闭 0开启
	PairObj __music_effect;      //音乐: obj_id：是否勾选，obj_value：音量
	PairObj __sound_effect;      //音效: obj_id：是否勾选，obj_value：音量

	SysSetting();
	void reset();
};

struct RechargeOrder
{
	int __order_id;
	std::string __order_num;
	int __channel_id;
	int __money;
	int __gold;
	std::string __account;
	Int64 __tick;
	Int64 __role_id;

	RechargeOrder(void);
	void reset(void);
};

struct MailDetailSerialObj
{
	typedef std::vector<ThreeObj> AttachItem;

	std::string __title;
	std::string __content;
	std::string __sender_name;
	std::string __receiver_name;

	Int64 __mail_index;
	int __mail_type;
    int __mail_format_;
	Int64 __receiver_id;
	Int64 __sender_id;
	Int64 __send_tick;
	Int64 __read_tick;
	int __has_read;
	Money __attach_money;
	AttachItem __attach_item;

	MailDetailSerialObj();
	void reset();
};

struct FestivalInfo
{
	FestivalInfo()
	{
		this->icon_type_ = 0;
		this->act_state_ = 0;

		this->start_tick_ = 0;
		this->end_tick_ = 0;
		this->update_tick_ = 0;
	}

	int icon_type_;
	int act_state_;
	Int64 start_tick_;
	Int64 end_tick_;
	Int64 update_tick_;
};

struct BigActInfo
{
	BigActInfo()
	{
		this->type_ = 0;
		this->start_tick_ = 0;
		this->end_tick_ = 0;
	}

	int type_;
	Int64 start_tick_;
	Int64 end_tick_;
};

struct ScoreInfo
{
	Int64 id_;
	string name_;

	double score_;
	Int64 tick_;

	ScoreInfo();
};

typedef std::map<Int64, ScoreInfo> ScoreInfoMap;

struct GameSwitcherDetail
{
	// true  表示开放相应功能
	BStrIntMap switcher_map_;	//key: function, value: 0=close, 1=open

	struct Names
	{
		const static std::string market;	//市场
		const static std::string shop;		//商店
		const static std::string mail;
		const static std::string mall_gift;
		const static std::string first_recharge;
		const static std::string trade;
		const static std::string box;
		const static std::string rank;
		const static std::string first_double_return;	//删档测试双倍
		const static std::string download_box_gift;
		const static std::string love_gift;
		const static std::string equip_red; //红装
		const static std::string treasures;	//宝匣
		const static std::string gift;		//激活码礼包
		const static std::string transfer;	//变身
		const static std::string molding_spirit;	//铸魂
		const static std::string jewel_sublime;		//宝石升华
		const static std::string special_box;		//神秘宝箱
	};

    void reset(void);
    int serialize(Message *msg);
    int unserialize(Message *msg);
    bool has_update(GameSwitcherDetail &other, const char* name=NULL);
};
typedef GameSwitcherDetail::Names GameSwitcherName;

struct BackGameModify
{
	struct LuckyTable
	{
		Int64 start_tick[2];
		Int64 end_tick[2];
		int open_state;
		int activity_id;
		void reset();
	};
	struct Names
	{
		const static std::string league_fb;//仙盟副本活动状态
		const static std::string gongcheng;//怪物攻城活动
		const static std::string wildqixi;//荒野奇袭活动
		const static std::string you49_qq;//49游qq
		const static std::string fashion_box;//时装宝盒
		const static std::string lucky_table;//幸运转盘
	};
	struct SuperVipInfo
	{
		std::string qq_num;
		std::string des_content;
		std::string des_mail;
		int vip_level_limit;
		int need_recharge;
		void reset();
	};
	typedef std::map<int, SuperVipInfo> SuperVipInfoMap;

	LuckyTable ltable;
	std::string name;
	int is_update;
	Int64 role_id;
	Int64 league_id;
	Int64 value;
	std::string value_str;
	SuperVipInfoMap value_map;
	void reset();
};

typedef std::vector<BackGameModify> BackGameModifyVec;
typedef BackGameModify::Names GameModifyName;

struct FighterSkill: public HeapNode
{
	int __skill_id;		//ID
	int __level;		//等级
	int __used_times;	//升级使用次数
	int __fight_use_times;	//战斗使用次数

	int __use_level;
	int __use_rate;
	int __lend_skill;
	int __object;
	int __distance;
	int __radius;
	int __aoe_type;
	int __launch_way;
	int __rand_step;
	int __max_step;
	int __full_screen;
	int __server_force;		//特殊技能战力

    int __skill_type;
    int __transfer_no_release;
    int __del_buff;
    int __is_launch_once;
    int __no_object_limit;
    int __object_from_server;
    int __db_flag;
    int __check_flag;
    int __level_type;

    int __is_loop;
    int __is_mutual;
    int __effect_ai_skill;
    int __sub_rate_skill;
    int __sub_rate_skill_2;
    int __need_used_times;
    int __passive_trigger;
    int __max_times;

    string __target_type;

	Time_Value __cool;
	Time_Value __use_tick;
//	Time_Value __select_tick;

	FighterSkill(int skill = 0, int level = 1);

	void reset(int skill = 0, int level = 1);
	void add_use_tick();

	void serialize(ProtoSkill* proto, int client = false);
    void unserialize(const ProtoSkill& proto);

    void serialize(ProtoPairObj* proto);
    void unserialize(const ProtoPairObj& proto);

    bool is_cool_finish();
    bool is_active_skill();
    bool is_passive_skill();
    bool is_passive_prop_skill();
    bool is_passive_buff_skill();
    bool arrive_fight_max_times();

	int fun_bit();
	int skill_id();
	int skill_level(int client);
	int level_conf(const Json::Value& json);

	const Json::Value& conf();
	const Json::Value& detail();
};

struct SecondTimeout
{
	int start_;
	int going_;
	int interval_;

	int sub1_;
	int sub2_;
	Int64 index_;

	SecondTimeout();

	void stop_time();
	void reset_time();
	void start_interval(int interval);

	bool update_time();
	bool is_timeout();
};

struct LeftTimeOut
{
	int left_time_;

	LeftTimeOut();

	void set_time(int left_time);
	bool reduce_time();
	bool is_zero();
};

struct BasicElement
{
    enum ELEMADD
    {
        ELEM_ALL    = 0,
	    ELEM_MULTI_BASE = 1, // 用于计算属性的百分比加成的基数
        ELEM_OTHER_BASE = 2, // 用于返回百分比基数外的加成值
        ELEM_FORCE = 3, // 计算战力
        ELEM_NO_BASE = 4,	//BASIC外

        ELEM_ADD_END
    };

    /*宠物*/
    enum BEASTOFFSET
    {
    	BEAST_BASIC	= 0,
        BEAST_SOUL,			//炼魂 1
        BEAST_SKILL,		//宠物技能 2
        BEAST_GROWTH,		//宠物成长值 3
        TOTAL_PROP,			//总属性 4
        BEAST_END
    };

    /*人物*/
    enum OFFSET
    {
        BASIC       = 0,
        EQUIP,              //装备 1
		PLAYER_SKILL,		//玩家技能 2
        STATUS,             //状态加成 3
        LABEL,              //称号 4
        ACHIEVEMENT,        //成就 5
        LEAGUE_SKILL,		//仙盟技能 6
        PROP_ITEM,			//道具 7
        TIAN_GANG,			//天罡 8
        FLY,                //飞升 9
        WEDDING,            //结婚 10
        VIP,				//VIP加成 11
        GODER,				//化神 12
        MAGICWEAPON,		//法宝 13
		ILLUSTRATION,       //图鉴 14
        MOUNT,				//战骑 15
        GOD_SOLIDER,		//神兵 16
        MAGIC_EQUIP,		//法器 17
        XIAN_WING,			//仙羽 18
        LING_BEAST,			//灵宠 19
        BEAST_EQUIP,		//灵器 20
        BEAST_MOUNT,		//灵骑 21
        EQUIP_SMELT,		//装备熔炼 22
        LEAGUE_FLAG,		//帮帮旗帜 23
        SWORD_POOL,			//剑池 24
        BEAST_WING,			//灵羽 25
        BEAST_MAO,			//灵猫 26
        FASHION,			//时装 27
        WED_RING,			//戒指 28
        WED_SYS,			//姻缘 29
        WED_TREE,			//爱情树	30
        TRANSFER,			//变身 31
        JEWEL_SUBLIME,		//宝石升华 32
        OFFSET_END
    };

    double __elem[OFFSET_END];

    BasicElement();

    double __total(int buff = BasicElement::ELEM_ALL, bool multi = false) const;
    int __total_i(int buff = BasicElement::ELEM_ALL, bool multi = false) const;

    static bool validate_offset(int offset);

    void reset(void);
    void reset_single(int offset = BasicElement::BASIC);

    void set_single(double value, int offset = BasicElement::BASIC);
    void add_single(double value, int offset = BasicElement::BASIC);
    void reduce_single(double value, int offset = BasicElement::BASIC);

    double basic() const;
    double single(int offset = BasicElement::BASIC);
};

// 剑池任务编号
namespace GameEnum
{
	enum
	{
		SPOOL_TASK_BEGIN = 0,
		SPOOL_TASK_ADVANCE_SCRIPT,		//进阶副本
		SPOOL_TASK_STORY_SCRIPT,		//剧情副本
		SPOOL_TASK_EXP_SCRIPT,			//经验副本
		SPOOL_TASK_DAILY,				//日常任务
		SPOOL_TASK_LEAGUE,				//帮会任务
		SPOOL_TASK_LEGEND,				//挑战江湖榜
		SPOOL_TASK_TRVL_SCRIPT,			//跨服副本
		SPOOL_TASK_WBOSS,				//世界boss
		SPOOL_TASK_ESCORT,				//护送
		SPOOL_TASK_REWARD,				//悬赏任务
		SPOOL_TASK_END
	};
};

enum MOVE_SCENE_MODE
{
    SCENE_MODE_NORMAL   		= 0,
    SCENE_MODE_LEAGUE			= 1,
    SCENE_MODE_SCRIPT   		= 2,
    SCENE_MODE_WORLD_BOSS 		= 4,
    SCENE_MODE_BATTLE_GROUND	= 5,
    SCENE_MODE_LEAGUE_WAR 		= 14,	//帮派争霸
    SCENE_MODE_MONSTER_ATTACK 	= 15,	//怪物攻城
    SCENE_MODE_REGION_FIGHT		= 16,   //城池战
    SCENE_MODE_TRVL_PEAK		= 17,	//巅峰对决
    SCENE_MODE_END
};


#endif //_PUBSTRUCT_H_
