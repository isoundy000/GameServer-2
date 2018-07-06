/*
 * FightStruct.h
 *
 * Created on: 2013-12-06 10:26
 *     Author: lyz
 */

#ifndef _FIGHTSTRUCT_H_
#define _FIGHTSTRUCT_H_

#include "MapStruct.h"
#include "Heap.h"
#include <stdint.h>
#include <memory>
#include <list>

class Proto10400202;
class ProtoStatus;

namespace GameEnum
{
	enum MAP_COMMON_ENUM
	{
		MAX_ANGRY				= 100,

		FIGHTER_MONSTER 		= 1,	//小怪
		FIGHTER_BOSS			= 2,	//BOSS
		FIGHTER_GATHER			= 3,	//采集物

		MAP_COMMON_ENUM_END
	};

	enum SKILL_TARGET_TYPE
	{
		SKILL_TARGET_ENEMY = 0,     		// 技能施放给玩家，玩家是敌方
		SKILL_TARGET_ALLY,          		// 技能施放给玩家, 玩家是友方
		SKILL_TARGET_SELF,          		// 技能施放给自己
		SKILL_TARGET_MONSTER,				// 技能施放给小怪
		SKILL_TARGET_BOSS,          		// 技能施放给BOSS
		SKILL_TARGET_SELF_OWNER,			// 技能施放给自己主人
		SKILL_TARGET_NEUTRALITY,			// 技能施放给中立

		SKILL_TARGET_END
	};
	typedef std::bitset<SKILL_TARGET_END> SkillTargetFlagSet;

	enum SKILL_AOE_TARGET_SELECT_TYPE
	{
		SKILL_AOE_CUR_AIM_TARGET = 0,		// 当目标对象
		SKILL_AOE_TARGET_CIRCLE = 1,    	// 以目标为圆心
		SKILL_AOE_SELF_CIRCLE = 2,      	// 以自身为圆心
		SKILL_AOE_SELF_SECTOR = 3,      	// 以自身为扇形
		SKILL_AOE_SELF_RECT = 4,        	// 以自身前面为矩形
		SKILL_AOE_SELF_OWNER_TARGET = 5,	// 自身主人
		SKILL_AOE_SELF_TARGET = 6,			// 自身
		SKILL_AOE_TARGET_NEUTRALITY = 7,	// 中立
		SKILL_AOE_SCENE_LEFT_SIDE = 8,		// 自身场景左侧对象
		SKILL_AOE_SCENE_RIGHT_SIDE = 9,		// 自身场景右侧对象
		SKILL_AOE_SELF_RING = 10,			// 以自身为环形圆心
		SKILL_AOE_BOSS = 11,                // 以BOSS为对象
		SKILL_AOE_ALL_PLAYER = 12,			// 全部玩家
		SKILL_AOE_TARGET_RECT = 13,			// 以目标为矩形中心
		SKILL_AOE_RANK_HURT_LIST = 14,      // 随机选择伤害列表中的一个玩家
		SKILL_AOE_TARGET_POINT_CIRCLE = 15,	// 以目标点为原心
		SKILL_AOE_END
	};

	enum SKILL_COORD_TYPE
	{
		SKILL_COORD_T_SELF = 0,     // 以自己坐标为技能坐标点
		SKILL_COORD_T_FIXED = 1,    // 以初始触发时的技能点作为固定的技能坐标点
		SKILL_COORD_T_END
	};

	enum STATUS_UPDATE_FLAG
	{
		STATUS_U_PROP = 0,
		STATUS_U_SPEED,

		STATUS_U_END
	};
}

struct HistoryStatus
{
	double __value1;
	double __value2;
	double __value3;
	double __value4;
	double __value5;

	void reset(void);
};

struct BasicStatus: public HeapNode
{
	enum
	{
		VALUE1				= 1,
		VALUE2				= 2,
		VALUE3				= 3,
		VALUE4				= 4,
		VALUE5				= 5,

		BLOOD 				= 101,	//修改当前血，具体总值
		DIRECTBLOOD 		= 102,	//修改当前血，按当前血量万分比
		DIRECTMAXBLOOD 		= 103,	//修改当前血，按最大血量万分比
		MAXBLOOD 			= 104,  //修改最大血量，并根据剩余比例,修改当前血量
		SPEED 				= 105,  //修改当前速度
		ATTACK 				= 106,  //攻击
		DEFENCE 			= 107,	//防御
		HIT 				= 108,  //命中
		AVOID 				= 109,  //闪避
		CRIT 				= 110, 	//暴击
		TOUGHNESS 			= 111,  //坚韧
		HURTDEEP 			= 112,  //伤害加深
        CRIT_RATE 			= 113,  //暴击率
        CRIT_HURT			= 114,	//暴击伤害
        DIRECTHURT 			= 115,  //根据攻击力扣血
        DAMAGE				= 116,	//无视防御伤害
		JUMPING				= 121,	//跳跃
        CRAZY_ANGRY 		= 122,	//狂怒
        EXEMPT 				= 123,  //免伤
		SILENCE 			= 124,  //沉默
		STAY 				= 125, 	//定身
		DIZZY 				= 126,  //眩晕
        SHIELD 				= 127,	//护盾
		RELIVE_PROTECT 		= 128, 	//复活保护
		ROLE_SHIELD			= 129,	//霸体
		INSIDE_ICE			= 130,	//冰封
		JIAN_DROP			= 131,	//剑落
		MULTI_PROP			= 132,	//多个属性
		FIGHT_THROUGH		= 133,	//致命穿透
		SUPPERMAN 			= 134,  //无敌
		EXCEPT_DIZZY		= 135,	//免疫眩晕
		EXCEPT_MAX_BLOOD	= 136,	//免疫流血
		FORCE_DIZZY 		= 137,	//强力眩晕
		FIX_SHIELD			= 138,	//固定护盾
		IMMUNE_DIZZY		= 139,	//百分比免疫眩晕

        EVENTCUT 			= 1101,	//连斩BUFF
        PROP_ADD_EXP 		= 1102,	//道具加经验
        QUINTUPLE_ADD_EXP 	= 1103, //五倍挂机
        TRANSFER_ADD_EXP 	= 1104,	//变身增加经验
        TBATTLE_TREASURE    = 70001, //华山秘宝BUFF

		/******************no use*************************/
		// 死亡时会清除的BUFF
		MAGIC = 5,            // 天山仙露效果的BUFF
		MAXMAGIC = 6,           // 修改最大法值
		DIRECTMAGIC = 7,        // 修改法，按当前法值计算百分比
		DIRECTMAXMAGIC = 8,     // 修改法，按最大法值计算百分比
		REBOUNDHURT = 22,       // 反弹伤害,按伤害值计算百分比
		REBOUNDDEFENCE = 23,    // 反弹伤害,按防御计算百分比
		SKILLDISTANCE = 24,     // 修改被动技能的攻击距离
		PEASANT = 38,          // 农民状态(只能用三连击)
		REBOUNDMAXBLOOD = 40, 	// 反弹伤害,按造成伤害者的最大血量计算百分比
		IMMUNE = 41, 			// 伤害免疫,同时反弹伤害
        REPEATHURT = 44,        // 增加一次伤害
        BLOOD_BACK_REDUCE = 45, // 回血减少一定比例
        INFMAGIC = 46,          // 技能不消耗能量的BUFF
        STIFF = 49,             // 技能受击僵直状态
        MREDUCE_HURT = 50,      // 受到伤害时用法力抵消伤害
        FLASH_AVOID = 56,       // 疾闪
        STONE_PLAYER = 57,      // 被石化
		STATUS_TYPE_END
	};

	int __status;           //实际唯一BUFF ID
	int __client_status;		//
	int __buff_type;
	int __show_type;		//1表示周围，2表示自己
	int __flag;
	int __client_msg;		//消息方式

	IntMap __avoid_buff;	//免疫BUFF
	std::vector<BasicStatus*> __after_status;	// 后续触发的 buff

	IntMap __inoperative_scene;	//不起效果的场景

	/*
	 * 在构建 __after_status 时会使用 __value1 来保存配置中的 last 时间
	 * 请确保 __value1 为 double 类型
	 * */
	double __value1;	//一般为数值
	double __value2;	//一般万分比，保留原值 //还用于护盾的固定伤害值
	double __value3;
	double __value4;
	double __value5;
	IntVec __value6;

	Time_Value __check_tick;
	Time_Value __interval;
	Time_Value __last_tick;

	int __skill_id;
	int __level;
	Int64 __attacker;
	int __accumulate_tims;	// 叠加次数

	int status_type() const;

	int client_status();
	int left_time(int type = 0);

	double fetch_value(int type);
	const Json::Value& conf() const;

	BasicStatus();
	BasicStatus(int status);
	BasicStatus(double value, double percent, Int64 attackor = 0);

	void reset();
	void recyle_after_status();
	void set_value(int type, double val);
	void set_normal_value(double value, double percent);

	void set_status(int status);
	void set_client_status(int client_status);
	void set_normal(int status, double last, double interval = 0, double val1 = 0, double val2 = 0);
	void set_all(const Time_Value &last, const Time_Value &interval = Time_Value::zero,
			Int64 attackor = 0, int skill_id = 0,
			int level = 0, double val1 = 0, double val2 = 0,
			double val3 = 0, double val4 = 0, double val5 = 0);

    void serialize(ProtoStatus *proto);
    void serialize_b(int i, ProtoStatus *proto);
};

typedef std::vector<BasicStatus*> BasicStatusVec;

class StatusValueCmp
{
public:
	bool operator()(BasicStatus *&left, BasicStatus *&right);
};

typedef Heap<BasicStatus, StatusValueCmp> StatusValueQueue;
struct StatusQueueNode: public HeapNode
{
	enum
	{
		DIE_REMOVE 			= 0,
		EXIT_SCENE_REMOVE	= 1,
		EXIT_GAME_REMOVE	= 2,
		TOTAL_REMOVE_TYPE 	= 3,
		END
	};

	int __status;
	int __remove_type[TOTAL_REMOVE_TYPE];

	Time_Value __check_tick;
	StatusValueQueue __status_list;

	StatusQueueNode();
	void reset();
	void set_status(const BasicStatus &status);
};
class StatusQueueNodeCmp
{
public:
	bool operator()(StatusQueueNode *&left, StatusQueueNode *&right);
};

class StatusTickCmp
{
public:
	bool operator()(BasicStatus *&left, BasicStatus *&right);
};

class TraceForceInfo
{
public:
	TraceForceInfo();
	~TraceForceInfo();

	SubObj& sub_info();
	void set_trace_info(GameFighter* fighter, const SubObj& sub = SubObj());

	bool trace_dump_a();
	bool trace_dump_b();

private:
	GameFighter* fighter_;
	SubObj sub_;
	int prev_force_;
};

class FighterSkillCmp
{
public:
	bool operator()(FighterSkill *&left, FighterSkill *&right);
};

struct DelaySkillInfo: public HeapNode
{
	Time_Value __launch_tick;
	std::auto_ptr<Proto10400202> __request;

	void reset(void);
};

class DelaySkillCmp
{
public:
	bool operator()(DelaySkillInfo *&left, DelaySkillInfo *&right);
};

struct PassiveSkillInfo : public HeapNode
{
	Time_Value __launch_tick;
	FighterSkill *__skill;
	void reset(void);
};

class PassiveSkillCmp
{
public:
	bool operator()(PassiveSkillInfo *&left, PassiveSkillInfo *&right);
};

typedef std::vector<PassiveSkillInfo *>	 			PassiveSkillInfoVec;
typedef Heap<PassiveSkillInfo, PassiveSkillCmp> 	FightPassiveSkillQueue;

struct DefenderHurt
{
	int64_t __defender_id;
	int __hurt_blood;
	int __hurt_magic;

	DefenderHurt(void);
	void reset(void);
};


struct FightDetail
{
	int __pk_state;
	int __camp_id;              // 阵营ID, 在强PK场景时判断对象是否可攻击
	int __last_skill;			// 最近使用的技能
	int __fight_state;			// 战斗状态类型：0非战斗状态,１主动攻击,2被动攻击
	Time_Value __fight_tick;	// 战斗状态到期时间
	Int64 __death_tick;
	int __relive_request_two;	// 复活请求了两次
	Time_Value __relive_tick;
	Time_Value __pk_tick;       // PK状态剩余时间
	Time_Value __gather_tick;	// 采集状态到期时间
    int gather_state_;			// 采集状态0:不在采集/1:正在采集

	int __level;				//等级
	int __auto_recover_blood;	//是否自动回血 AI
	int __recover_blood_span;	//回血时间间隔
	int __recover_blood; 		//每次恢复最大血值
	int __recover_blood_per;	//每次恢复最大万分比
	int __jump;					//跳跃值
	int __jump_times;			//跳跃次数
	int __angry;				//怒气值
	int __glamour;				//魅力值
	int __pk_value;
	Int64 __experience;		//经验
	Int64 __next_exp;		//升级所需要的经验

	BasicElement __attack_lower;	// 攻击下限
	BasicElement __attack_upper;	// 攻击上限
	BasicElement __defence_lower;	// 防御下限
	BasicElement __defence_upper;	// 防御上限
	BasicElement __hit;         // 命中
	BasicElement __avoid;       // 闪避
	BasicElement __crit;   		// 暴击
	BasicElement __toughness;   // 韧性
	BasicElement __lucky;		// 幸运
    BasicElement __damage;      // 伤害加成值
    BasicElement __reduction;   // 伤害减免值

	BasicElement __blood_multi;     // 血量提升倍率
	BasicElement __magic_multi;     // 蓝提升倍率
	BasicElement __attack_lower_multi;	// 攻击下限变化倍率
	BasicElement __attack_upper_multi;	// 攻击上限变化倍率
	BasicElement __defence_lower_multi;	// 防御下限变化倍率
	BasicElement __defence_upper_multi;	// 防御上限变化倍率
	BasicElement __hit_multi;       // 命中提升倍率
	BasicElement __avoid_multi;     // 闪避提升倍率
	BasicElement __crit_hurt_multi; // 暴击伤害倍率
	BasicElement __crit_value_multi;// 暴击值倍率
	BasicElement __toughness_multi; // 韧性提升倍率
	BasicElement __lucky_multi;		// 幸运变化倍率
    BasicElement __damage_multi;    // 伤害加成倍率
    BasicElement __reduction_multi; // 伤害减免倍率

	int __blood;                // 当前血量
	BasicElement __blood_max;   // 最大血量
	int __magic;                // 当前能量
	BasicElement __magic_max;   // 能量最大值
	int __color_all_per;

	int hit_klv_;		//命中KLV
	int avoid_klv_;		//闪避KLV
	int crit_klv_;		//暴击KLV
	int toughness_klv_;	//韧性KLV

	typedef HashMap<int64_t, DefenderHurt, NULL_MUTEX> AttackorMap;
	AttackorMap __attackor_map;	// 同一个战斗状态内所受到的实际伤害统计

	Int64 __last_defender_id;
	BLongSet __history_defender_set; // 记录历史攻击过的对象数目
	BIntSet __passive_skill_set;   	// 由基本技能才可触发的被动技能列表

	SkillMap __skill_map;	//技能总表
	LongMap __passive_skill_use_time;	//被动技能记录使用时间
	FightPassiveSkillQueue __fight_passive_skill_queue;	// 战斗中概率触发的被动技能列表
    FightPassiveSkillQueue __hurt_passive_skill_queue;  // 战斗中受到伤害时触发的被动技能列表
//    FightPassiveSkillQueue __hurt_can_avoid_passive_skill_quue; // 战斗中没有闪避的伤害时才触发的被动技能列表

	int __skill_id_for_step;			// 当前的技能步数
	int __skill_step;					// 技能步数记录
	Time_Value __skill_step_tick;		// 技能步数刷新时间

	void reset(void);
	void set_level(int level);
	void clear_all_fight_property(int offset);
	void add_fighter_property(int offset, const FightProperty& prop);

	double blood_percent(GameFighter *fighter, int coefficient = 100);
	double magic_percent(GameFighter *fighter, int coefficient = 100);

	int cur_blood();
	int cur_magic();

	void set_cur_blood(double percent);
	void set_cur_magic(double percent);

	int enough_exp_upgrade();
	int left_relive_time(int delay = 0);

	bool validate_offset(int offset);

	// 生命
	double __blood_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __blood_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	// 能量
	double __magic_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __magic_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __attack_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __attack_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	// 攻击下限
	double __attack_lower_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	// 攻击上限
	double __attack_upper_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __defence_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __defence_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	// 防御下限
	double __defence_lower_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	// 防御上限
	double __defence_upper_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __crit_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __crit_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __crit_hurt_multi_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __toughness_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __toughness_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __hit_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __hit_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __avoid_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __avoid_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

	double __lucky_total(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;
	int __lucky_total_i(GameFighter *fighter, const int buff =
			BasicElement::ELEM_ALL) const;

    double __damage_total(GameFighter *fighter, const int buff = BasicElement::ELEM_ALL) const;
    double __damage_rate_total(GameFighter *fighter, const int buff = BasicElement::ELEM_ALL) const;

    double __reduction_total(GameFighter *fighter, const int buff = BasicElement::ELEM_ALL) const;
    double __reduction_rate_total(GameFighter *fighter, const int buff = BasicElement::ELEM_ALL) const;

	double __force_total(GameFighter *fighter);
	int __force_total_i(GameFighter *fighter);

	double __element_force(int offset);
};

struct CurrentSkill
{
	enum
	{
		SKILL_EFFECT_HURT = 0,
		SKILL_EFFECT_AID,
		SKILL_EFFECT_END
	};

	typedef std::bitset<SKILL_EFFECT_END> SkillEffectFlag;
	SkillEffectFlag __effect_flag;

	int __skill_id;
	int __display_skill;		// 通知给客户端的技能ID
	int __skill_level;
	MoverCoord __skill_coord;
    MoverCoord __play_coord;    // 客户端发过来的坐标点

	double __radian;         //朝向：单位弧度
	double __angle;			//朝向：单位角度

	int __skill_step;		// 技能步数,从1开始
	Time_Value __prev_skill_launch_tick;
	int __is_full_screen;	// 是否全屏技能
    double __add_effect_tick;   // 被动技能增加特效时间(旋风斩等)
    bool __is_passive_effect; // 是否处理被动效果

    LongSet __client_target_set;	//客户端目标
	LongSet __defender_set;
	int64_t __defender_id;
    int __has_defender_buff; // 闪避没有效果时不插入特效BUFF

    int __hurt_flag;	//是否有技能伤害
	int __skill_hurt;    // 用于统计技能的伤害
	int __skill_hurt_percent;	// 用于统计技能的伤害万分比
    int __skill_blood;      // 技能回血，给被攻击者一次回血的技能
    int __skill_self_blood; // 技能给自己回血的技能
    int __skill_hurt_deep;  // 对单体一次伤害加深的值
    double __skill_hurt_deep_percent;   // 对单体一次伤害加深百分比

    bool __from_pool;
    FighterSkill* __skill;
	GameFighter *__defender;
    int __avoid_hurt;   // 是否产生闪避

    bool __in_safe_area;

	typedef HashMap<Int64, DefenderHurt, NULL_MUTEX> DefenderMap;
	DefenderMap __defender_map;		// 玩家技能对被攻击者的实际伤害统计；

    typedef std::set<string> EffectNameSet;
    EffectNameSet __passive_effect_set; // 被动跟基础之间互斥的效果

    Int64 __client_target;	// 客户端当前选中的攻击对象ＩＤ not reset
    Int64 __sel_defender;	// 服务端当前选中的攻击对象ＩＤ

    int save_skill_;
    Int64 save_defender_;

    CurrentSkill();

	void reset(void);
	void set_angle(double angle);	//设置角度
	void set_radian(double radian);	//设置弧度
	void set_skill_id(GameFighter* fighter, int skill_id, int skill_level = 1);

	void save_info();
	void restore_info();

	int aoe_type();
	int is_base_skill();
	int is_jump_skill();
	int need_check_distance();

	void effect_flag_reset(void);
	void effect_flag_reset(const int flag);
	void effect_flag_set(const int flag);
	bool effect_flag_test(const int flag);
};

struct LoopSkillDetail
{
	int __skill;
	Time_Value __next;
	Time_Value __interval;
	Time_Value __timeout;
	int __cnt; // 计数
    int __skill_coord_type; // 0 自身，1 固定攻击点
    MoverCoord __fixed_skill_coord;
    double __radian;	//弧度
    Int64 __client_target;	//客户端选中的对象

	void reset(void);
};

struct SkillFreezeDetail
{
	int __trigger_skill;	// 触发技能冻结效果的技能ID
	Time_Value __timeout;
	BIntSet __skills;
};


#endif //_FIGHTSTRUCT_H_
