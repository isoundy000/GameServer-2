/*
 * GameEnum.h
 *
 *  Created on: Jun 26, 2013
 *      Author: peizhibi
 */

#ifndef GAMEENUM_H_
#define GAMEENUM_H_

namespace GameEnum
{
	/*
	 * Common Enum
	 * */
	enum COMMON_ENUM
	{
		DEFAULT_VECTOR_SIZE			= 128,		//vector默认大小
		DEFAULT_HASH_MAP_SIZE		= 2048,		//哈希默认大小
		DEFAULT_TIME_OUT 			= 10,		//默认超时时间
		DEFAULT_MAX_NAME_LENGTH		= 128,		//名称默认最大长度
		DEFAULT_MAX_CONTENT_LEGNTH	= 512,		//内容默认最大长度
		DEFAULT_TOTAL_PAGE			= 1,		//默认总页数
		DEFAULT_MAX_RAND_TIMES		= 50,		//默认最大随机次数
		DEFAULT_MAX_RT_TIMES	 	= 20,		//默认最大随机方向的次数

		MAX_BEAST_NAME_LEN			= 12,		//宠物名字长度
		MAX_PLAYER_NAME_LEN 		= 12,		//人物名字长度
		MAX_MSG_NAME_LEN			= 32,		//消息名最大长度
		MAX_LEAGUE_NAME_LENGTH 		= 7,		//最长宗派名

		MAX_TEAMER_COUNT			= 3, 		//最多队伍成员个数

		TEAMER_STATE_NONE			= 0,		// 没有队伍
		TEAMER_STATE_LEADER			= 1, 		// 队长
		TEAMER_STATE_TEAMER			= 2, 		// 队员

		LEAGUE_WAND_DONATE			= 0,		//帮派令牌捐献
		LEAGUE_GOLD_DONATE			= 1,		//元宝捐献
		LEAGUE_WAND_TIMES			= 1,		//帮派令牌倍数

		DEFAULT_USE_SKILL_DISTANCE	= 5,		//默认施法距离
		DEFAULT_AI_PATH_GRID		= 30,		//默认AI的寻路格子大小
		DEFUALT_MAPPING_FACTOR		= 1,		//默认y轴投射比例
		CLIENT_MAPPING_FACTOR		= 2,		//客户端y轴投射比例
        CLIMB_TOWER_SCRIPT_AI_GRID  = 80,       //斩妖的怪物间隔增大
		DEFAULT_COPY_PLAYER_RADIUS	= 10,		//默认周围
		DEFAULT_CHASE_MAX_DISTANCE	= 20,		//追击最大的距离

		DEFAUL_OWNER_DISTANCE		= 10,			//离主人距离
//		BASE_MAGICW_SKILL_LHZ		= 420000011,	//法宝技能轮回珠
//		BASE_MAGICW_SKILL_TDJ		= 420000013,	//法宝技能天遁剑


		ET_LEAGUE_WAR				= 3,		//帮派战
		ET_SM_BATTLE				= 4,		//玄武战场
		ET_TM_ARENA					= 5,		//争霸天下
		ET_ARENA_FIELD				= 6,		//江湖榜
		ET_LEAGUE_BOSS 				= 13,		//帮派boss
		ET_COLLECT_CHESTS			= 14,		//豪门斗宝
		ET_ANSWER_ACTIVITY			= 15,		//答题活动
		ET_HOTSPRING_ACTIVITY		= 16,		//温泉玩法
		ET_MONSTER_ATTACK 			= 17,		//怪物攻城
		ET_WORLD_BOSS 				= 18,		//世界boss
		ET_TRAVEL_ARENA				= 19,		//跨服竞技
		ET_LEAGUE_REGION_WAR 		= 20,  		//帮派领地
		ET_TRVL_WBOSS				= 21,		//跨服世界boss
		ET_TRVL_BATTLE				= 22,		//华山论剑
		ET_TRVL_PEAK				= 23,		//巅峰对决

		SEX_UNISEX					= 0,		// 不分性别
		SEX_MALE					= 1,		//男
		SEX_FEMALE					= 2,		//女

		CAREER_NO_LIMIT				= 0,		//所有职业，即没有职业限制
		CAREER_WARRIOR              = 1,        // 战士职业
        CAREER_MASTER               = 2,        // 法师职业
        CAREER_ASSASSIN             = 3,        // 刺客职业
        CAREER_TAOIST               = 4,        // 道士职业
		
        SORT_DESC					= 1,		//降序
        SORT_ASC					= 2,		//降序

    	SPEED						= 1,		// 速度
    	SPEED_MULTI					= 2,		// 速度倍数
    	SPEED_B_MULTI				= 3,		// 基础速度的倍数

    	PLAYER_STATUS_NONE			= 0,
    	PLAYER_ONLINE				= 1,		// 在线
    	PLAYER_OFFLINE				= 2,		// 离线

    	//一级属性
    	POWER						= 101,		// 力量
    	STAMINA						= 102,		// 筋骨
    	PHYSICA						= 103,		// 体质
    	AGILITY						= 104,		// 敏捷
    	BODYACT						= 105,		// 身法

    	POWER_MULTI					= 111,		// 力量倍数
    	STAMINA_MULTI				= 112,		// 筋骨倍数
    	PHYSICA_MULTI				= 113,		// 体质倍数
    	AGILITY_MULTI				= 114,		// 敏捷倍数
    	BODYACT_MULTI				= 115,		// 身法倍数

    	BEAST_FORCE					= 221,		// 宠物战力
    	PROP_EXP_PERCENT			= 222,		// 经验倍数

    	//二级属性
    	BLOOD_MAX					= 2001,		// 血量上限
    	MAGIC_MAX					= 2002,		// 魔法上限
    	HIT							= 2003,		// 命中
    	AVOID						= 2004,		// 闪避
    	CRIT						= 2005,		// 暴击
    	TOUGHNESS					= 2006,		// 韧性
    	LUCKY						= 2007,		// 幸运值
    	ATTACK						= 2008,		// 攻击
    	DEFENSE						= 2009,		// 防御
        DAMAGE               		= 2010,     // 伤害加成数值
        REDUCTION           		= 2011,     // 伤害减免数值
    	ATTACK_UPPER				= 2012,		// 攻击上限
    	ATTACK_LOWER 				= 2013,		// 攻击下限
    	DEFENCE_UPPER				= 2014,		// 防御上限
    	DEFENCE_LOWER				= 2015,		// 防御下限

    	BLOOD						= 2101,		// 增加血量
    	MAGIC						= 2102,		// 增加魔法
    	HIT_MULTI					= 2103,		// 命中倍数
    	AVOID_MULTI					= 2104,		// 闪避倍数
    	CRIT_HURT_MULTI				= 2105,		// 暴击伤害倍数
    	TOUGHNESS_MULTI				= 2106,		// 韧性倍数
    	LUCKY_MULTI					= 2107,		// 幸运倍数
    	BLOOD_MULTI					= 2108,		// 血倍数
    	MAGIC_MULTI					= 2109,		// 魔法倍数
    	ATTACK_MULTI				= 2110,		// 攻击倍数
    	DEFENCE_MULTI				= 2111,		// 防御倍数
        DAMAGE_MULTI               	= 2112,     // 伤害加成万分比
        REDUCTION_MULTI           	= 2113,     // 伤害减免万分比
    	ATTACK_UPPER_MULTI			= 2114,		// 攻击上限倍数
    	ATTACK_LOWER_MULTI			= 2115,		// 攻击下限倍数
    	DEFENCE_UPPER_MULTI			= 2116,		// 防御上限倍数
    	DEFENCE_LOWER_MULTI			= 2117,		// 防御下限倍数
    	CRIT_VALUE_MULTI			= 2118,		// 暴击值倍数

    	//DAMAGE_ATTR_PERCENT，伤害加成或伤害减免是一个百分比（策划也不确定具体的位数精度，暂定为2位），
    	//而且是在战斗时才具体使用其值，为了以后方便操作此处使用一个枚举值（防止策划更改百分比位数而丢失精度）
    	DAMAGE_ATTR_PERCENT			= 10000,

    	//注意：由于策划会不定时增加属性，在计算战斗属性值时会遍历当前已有的属性，每次增添属性时都得更改代码中所有遍历属性的地方，
    	//麻烦且容易出错，所以在此增加ATTR_BEGINM和ATTR_END作为遍历的起始和结束，每次只需修改这个两值即可
    	ATTR_BEGIN					= BLOOD_MAX,		//遍历属性时的起始值
    	ATTR_END					= DEFENCE_LOWER,	//遍历属性时的结束值

    	ATTR_MULTI_BEGIN			= BLOOD,					// 遍历属性倍数时的起始值
    	ATTR_MULTI_END				= CRIT_VALUE_MULTI,			// 遍历属性倍数时的结束值

    	FSTATE_DRAW					= 0,		// 平局
    	FSTATE_WIN					= 1,		// 赢
    	FSTATE_LOSE					= 2,		// 输

		NORMAL_TEAM 				= 0,		//普通组队
		TRAVEL_TEAM 				= 1,		//跨服副本组队
		TOTAL_TEAM 					= 2,

    	MAX_LEAGUE_WAR_COUNT		= 9,
    	LEAGUE_SYNC_TIME			= 10,

    	EXIT_TYPE_PREVE				= 0,		// prev scene
    	EXIT_TYPE_SAVE				= 1,		// save scene
    	EXIT_TYPE_BORN				= 2,		// born scene

    	PLAYER_TYPE_BOTH			= 0,		//offline and online
    	PLAYER_TYPE_OFFLINE			= 1,		//offline
    	PLAYER_TYPE_ONLINE			= 2,		//online


    	GATHER_ITEM					= 1,		// 采集物品
    	GATHER_PROCESS				= 2,		// 采集进度
    	GATHER_RANDOM_ONE			= 3,		// 采集随机获得
    	GATHER_NONE					= 4,		// 采集动作
    	GATHER_CHESTS				= 6,		// 采集宝箱 获得奖励包
    	GATHER_REWARD 				= 8,		// 宝箱 奖励ID掉落

    	OFFLINE_DT_BEAST_MOUNT		= 1,		// 宠物坐骑

    	USE_GOODS_WELL				= 0,		// 使用正常
    	CHECK_AND_USE_IN_MAP		= 1,		// 检测并在地图线程使用
    	USE_GOODS_MUCH_TIMES		= 2,		// 多次使用

        TIPS_ITEM 				    = 0, 		//物品
        TIPS_CONFIG_STR				= 10,		//配置里的字符串

        UIOPEN_STEP_SIZE			= 20,		//UI界面开放步骤

		MAIN_ACT_OPEN 				= 1,	//开服活动
		MAIN_ACT_RETURN				= 2,	//返利活动
		MAIN_ACT_COMBINE			= 3,	//合服活动
		MAIN_ACT_FEST				= 4,	//节日活动
		MAIN_ACT_C_RETURN			= 6,	//合服返利活动

		COMMON_ENUM_END
	};

	/*场景ID*/
	enum GAME_SCENE_ID
	{
		ESCORT_SCENE_ID				= 15000,	//个人运镖
		RECHARGE_BASE_SCENE_ID		= 10100,	//每日充值存储数据场景
		ANSWER_ACTIVITY_SCENE_ID	= 40101,	//答题活动
        LBOSS_SCENE_ID				= 40201, 	//帮派驻地(帮派boss)
		LWAR_SCENE_ID				= 40202,	//帮派争霸
		LEAGUE_REGION_FIGHT_ID		= 40204,    //帮派领地
		MATTACK_SCENE_ID 			= 40301,	//怪物攻城
		HOTSPRING_SCENE_ID			= 40501,	//温泉玩法
		ARENA_SCENE_ID				= 50201,	//江湖榜
		SUN_MOON_BATTLE_ID			= 50301,	//玄武战场
		TRVL_SCRIPT_SCENE_ID		= 70101,	//跨服副本 70101 - 70109
		TRVL_ARENA_SCENE_ID			= 70201,	//跨服竟技
		TRVL_ARENA_PREP_SCENE_ID	= 70202,	//跨服竞技准备
		TRVL_MARENA_PREP_SCENE_ID	= 70401,	//争霸天下准备
		TRVL_MARENA_SCENE_ID		= 70402,	//争霸天下竞技
		TRVL_WEDDING_SCENE_ID 		= 70501,	//跨服结婚活动特殊场景
		TRVL_RECHARGE_SCENE_ID		= 70601,	//跨服每日冲榜特殊场景
        TRVL_BATTLE_SCENE_ID        = 70701,    //华山论剑
        TRVL_PEAK_SCENE_ID			= 70801,	//跨服巅峰对决场景
        TRVL_PEAK_PRE_SCENE_ID		= 70802,	//跨服巅峰对决准备场景
		GAME_SCENE_ID_END
	};

	enum WORLD_BOSS_SCENE_ID
	{
		WORLD_BOSS_SCENE_ID_READY		= 50100,	//世界BOSS场景开始
		WORLD_BOSS_SCENE_ID_1			= 50101,	//世界BOSS场景-乱花宫
		WORLD_BOSS_SCENE_ID_2			= 50102,
		WORLD_BOSS_SCENE_ID_3			= 50103,
		WORLD_BOSS_SCENE_ID_4			= 50104,
		WORLD_BOSS_SCENE_ID_5			= 50105,
		WORLD_BOSS_SCENE_ID_6			= 50106,
		WORLD_BOSS_SCENE_ID_7			= 50107,
		WORLD_BOSS_SCENE_ID_8			= 50108,
		WORLD_BOSS_SCENE_ID_9			= 50109,
		WORLD_BOSS_SCENE_ID_10			= 50110,
		WORLD_BOSS_SCENE_ID_11			= 50111,
		WORLD_BOSS_SCENE_ID_12			= 50112,
		WORLD_BOSS_SCENE_ID_13			= 50113,
		WORLD_BOSS_SCENE_ID_14			= 50114,
		WORLD_BOSS_SCENE_ID_15			= 50115,
		WORLD_BOSS_SCENE_ID_16			= 50116,
		WORLD_BOSS_SCENE_ID_17			= 50117,
		WORLD_BOSS_SCENE_ID_18			= 50118,	//世界BOSS场景-极寒境
		WORLD_BOSS_SCENE_ID_END
	};

	enum TRVL_WBOSS_SCENE_ID
	{
		TRVL_WBOSS_SCENE_ID_READY		= 70300,	//跨服世界BOSS中转场景
		TRVL_WBOSS_SCENE_ID_1			= 70301,	//跨服世界BOSS场景1
		TRVL_WBOSS_SCENE_ID_2			= 70302,	//跨服世界BOSS场景2
		TRVL_WBOSS_SCENE_ID_3			= 70303,	//跨服世界BOSS场景3
		TRVL_WBOSS_SCENE_ID_4			= 70304,	//跨服世界BOSS场景4
		TRVL_WBOSS_SCENE_ID_5			= 70305,	//跨服世界BOSS场景5

		TRVL_WBOSS_SCENE_ID_END
	};

	enum MAIL_TYPE
	{
		MAIL_PRIVATE 				= 301,
		MAIL_SYSTEM  				= 302,
		MAIL_GM						= 303,
		MAIL_SHOP					= 304,

		MAX_MAIL_TITLE_LENGTH		= 100,
		MAX_MAIL_CONTENT_LENGTH		= 1000,
		MAX_MAIL_COUNT				= 100,
		MAX_MAIL_GOODS_COUNT		= 4,

		MAIL_PAGE_COUNT				= 20,

		MAIL_DELETE_ALL             = 1,		//删除所有
		MAIL_DELETE_READ            = 2,		//删除已读
		MAIL_DELETE_SINGLE          = 3,		//删除单一
		MAIL_DELETE_NO_ATTACHED		= 4,		//删除没有附件

		DEFAULT_MAIL_BOX_SIZE          = 50,
		DEFAULT_MAIL_SEND_LIMIT_AMOUNT = 20,
		DEFAULT_MAIL_INTERVAL_HOUR     = 1,

		MAIL_END
	};

	enum GAME_UPDATE_PLAYER_TYPE
	{
		PLAYER_INFO_MOUNT 			= 1,		//玩家战骑
		PLAYER_INFO_VIP				= 2,		//玩家VIP状态
		PLAYER_INFO_EQUIP_SHAPE     = 3,        //玩家装备外形：武器衣服，时装武器，时装衣服
		PLAYER_INFO_LABEL           = 4,        //玩家称号
		PLAYER_INFO_LEAGUE			= 5,		//玩家帮派信息
		PLAYER_INFO_ID				= 7,		//玩家ID
		PLAYER_INFO_LEVEL           = 8,        //玩家等级
		PLAYER_INFO_PROTECT			= 9,		//是否在保护状态中
		PLAYER_INFO_COLOR			= 11,		//颜色
		PLAYER_INFO_MAGICWEAPON		= 13,		//罗摩
		PLAYER_INFO_GOLD_SOLIDER	= 14,		//神兵
		PLAYER_INFO_WING			= 15,		//仙羽
		PLAYER_INFO_SWORD_POOL		= 16,		//剑池
		PLAYER_INFO_HOTSRING		= 17,		//温泉
		PLAYER_INFO_TIAN_GANG		= 18,		//天罡
		PLAYER_INFO_FASHION			= 19,		//时装
		PLAYER_INFO_WEDDING			= 20,		//结婚
		PLAYER_INFO_TRANSFER		= 21,		//变身
		PLAYER_INFO_END
	};

	enum GAME_PACKAGE
	{
        ITEM_NO_BIND    			= 0,		//未绑定
        ITEM_BIND       			= 1,		//绑定
        ITEM_UNKNOW					= 2,		//绑定状态未知，待绑定（合成系统中新道具的绑定状态视子道具绑定状态而定）

        ITEM_TYPE_GOODS				= 1001,		//药品
        ITEM_TYPE_OTHER				= 2001,		//其他
        ITEM_TYPE_LEAGUE_TREASURE	= 2002,		//宗派藏宝图
        ITEM_TYPE_TRANSFER			= 2003,		//传送卡
        ITEM_TYPE_EQUIP_GOD_REFINE	= 2004,		//装备神炼
        ITEM_TYPE_ROLE_RENAME		= 2005,		//角色命名
        ITEM_TYPE_LEAGUE_RENAME		= 2006,		//仙盟命名
        ITEM_TYPE_ROLE_SEX			= 2007,		//角色变性
        ITEM_TYPE_MATRIAL			= 3001,		//材料
        ITEM_TYPE_LABEL				= 16,		//自动使用称号

        DEFAULT_OVERLAP_SIZE		= 100,		//默认叠加数量
        MAX_OVERLAP_SIZE   			= 999,		//最大叠加数量
        ID_NAME_EQUIPMENT 			= 100000000,

        PACK_MIN_INDEX				= 800,		//背包最小容量
        STORE_MIN_INDEX				= 40,		//仓库最小容量
        BOX_STORE_MIN_INDEX			= 20,		//藏宝库最小容量
        MOUNT_STORE_MAX_INDEX		= 3,		//坐骑装备最大容量
        GODER_STORE_MAX_INDEX 		= 4,		//化神装备最大容量

        INDEX_EQUIP     			= 1,		//身上装备
        INDEX_PACKAGE   			= 2,		//背包
        INDEX_GODER					= 3,		//化神
        INDEX_STORE                 = 5,        //仓库
        INDEX_BOX_STORE             = 6,        //藏宝库
		INDEX_MOUNT					= 10,       //坐骑装备
		INDEX_GOD_SOLIDER			= 20,		//神兵
		INDEX_MAGIC_WEAPON			= 30,		//法器
		INDEX_XIANYU				= 40,		//仙羽
		INDEX_BEAST					= 50,		//灵宠
		INDEX_BEAST_WU				= 60,		//灵武
		INDEX_BEAST_WEAPON			= 70,		//灵器
		INDEX_BEAST_WING			= 80,		//灵羽
		INDEX_BEAST_MAO				= 90,		//灵猫
		INDEX_TIAN_GANG				= 100,		//天罡

        MONEY_BIND_COPPER			= 0,		//绑铜
        MONEY_UNBIND_COPPER			= 1,		//非绑铜
        MONEY_BIND_GOLD				= 2,		//礼金
        MONEY_UNBIND_GOLD			= 3,		//元宝
        MONEY_CONTRIBUTE			= 4,		//贡献
        MONEY_GOLD = 8,							//gold or bind_gold
        MONEY_COPPER = 9, 						//copper or bind_copper

        GOLD_EXCHANGE_BIND_COPPER   = 100,
        GOLD_EXCHANGE_COPPER        = 10,

        DROP_TYPE_PERSON			= 1,	//最后一击
        DROP_TYPE_TEAM				= 2,	//队伍，每人一份
        DROP_TYPE_ALL				= 3,	//全部看得到，但只一份，先捡先得
        DROP_TYPE_ALL_HAVE			= 4,	//当前场景每人一份，通过邮件发送
        DROP_TYPE_MAX_HURT_PERSON   = 5,    //最大伤害
        DROP_TYPE_RANDOM_HURT       = 6,    //伤害排行前几位中随机
        DROP_TYPE_ALL_HAVE_PICKUP	= 7,	//当前场景每人一份，捡起放入背包

        GAME_PACKAGE_END
	};

	enum GAME_SHOP
	{
		DEFAULT_SHOP_ITEM_SIZE		= 20,

		MONEY_ADJUST_DEFAUL			= 0,	//默认，两者都调整
		MONEY_ADJUST_COPPER			= 1,	//只调整铜钱，不足时不调整
		MONEY_ADJUST_GOLD			= 2,	//只调整元宝，不足时不调整
		MONEY_ADJUST_COPPER_FORCE	= 3,	//强制调整铜钱

		SHOP_SELL_BUYBACK			= 100,	//回购
		SHOP_SELL_DRUG				= 101,	//药品
		SHOP_SELL_STRENGTHEN		= 102,	//强化

		SHOP_ITEM_NORMAL			= 1,	//普通
		SHOP_ITEM_HOT				= 2,	//热卖
		SHOP_ITEM_DISCOUNT			= 3,	//打折
		SHOP_ITEM_TIME_LIMITED		= 4,	//限时

		MAX_KEEP_SELL_OUT 			= 18,

		//mall activity
		MALL_BUY_TOTAL_LIMIT        = 101,  //101 全服限量
		MALL_BUY_SINGLE_LIMIT       = 201,  //201 个人限量
		MALL_BUY_BOTH_LIMIT         = 301,  //301 全服限量+个人限量

		MALL_ACTIIVTY_NON_REFRESH   = 0,   //商城活动结束时刷新
		MALL_ACTIIVTY_DAILY_REFRESH = 1,   //商城活动每日刷新

		//mall type
		MALL_TYPE_VIP_PLAYER        = 1,  //限购
		MALL_TYPE_PLAYER            = 2,  //变强
		MALL_TYPE_PET               = 3,  //骑宠
		MALL_TYPE_NORMAL            = 4,  //日常
	    MALL_TYPE_FASHION           = 5,  //时装
		MALL_TYPE_ACTIVITY          = 6,  //商城活动
		MALL_TYPE_PAY_BY_BIND_GOLD  = 7,  //礼金商城

		//mall goods' item_type_ : ShopItem
		MALL_ITEM_NOT_TAP		    = 1000,	// 不显示标签
		MALL_ITEM_HOT_SALE			= 1001,	// 显示推荐热销标签
		MALL_ITEM_DISCOUNT			= 1002,	// 显示推荐打折标签
		MALL_ITEM_RECOMMAND			= 1003,	// 显示推荐推荐标签
		MALL_ITEM_ONLY_MALE			= 1004,	// 仅男性角色可购买
		MALL_ITEM_ONLY_FEMALE		= 1005,	// 仅女性角色可购买
		MALL_ITEM_LIMITED			= 1006,	// 每天限购
		MALL_TOTAL_LIMITED			= 1007,	// 限购总量
		MALL_ITEM_VIP_BUY_1       	= 1008, // VIP BUY_1
		MALL_ITEM_VIP_BUY_2       	= 1009, // VIP BUY_2
		MALL_ITEM_VIP_BUY_3       	= 1010, // VIP BUY_3
		MALL_ITEM_VIP_BUY_4      	= 1011, // VIP BUY_4
		MALL_ITEM_VIP_BUY_5       	= 1012, // VIP BUY_5
		MALL_ITEM_VIP_BUY_6       	= 1013, // VIP BUY_6
		MALL_ITEM_VIP_BUY_7       	= 1014, // VIP BUY_7
		MALL_ITEM_VIP_BUY_8       	= 1015, // VIP BUY_8
		MALL_ITEM_VIP_BUY_9       	= 1016, // VIP BUY_9
		MALL_ITEM_VIP_BUY_10      	= 1017, // VIP BUY_10
		MALL_ITEM_ONLY_WARRIOR    	= 1018, // 仅战士角色可购买
		MALL_ITEM_ONLY_ASSASSINT  	= 1019, // 仅法刺客色可购买
		MALL_ITEM_ONLY_MASTER     	= 1020, // 仅刺法师色可购买


		GAME_SHOP_END
	};

	enum GAEM_BEAST
	{
		BEAST_SKY_SOUL				= 1,	//天魂
		BEAST_EARTH_SOUL			= 2,	//地魂
		BEAST_LIFE_SOUL				= 3,	//命魂
		TOTAL_BEAST_SOUL			= 4,

		UPDATE_BEAST_ID				= 0,	//ID
		UPDATE_BEAST_SORT			= 1,	//灵宠
		UPDATE_BEAST_EQUIP			= 2,	//灵武
		UPDATE_BEAST_MOUNT			= 3,	//灵骑
		UPDATE_BEAST_WING			= 4,	//灵羽

		GAME_BEAST_END
	};

	enum GAME_LABEL
	{
		LABEL_TYPE_PERMANT             =  0,//永久
		LABEL_TYPE_LIMIT_TIME          =  1,//限时

		LABEL_INSERT                   =  1,//
		LABEL_DELETE                   =  2,//

		LABEL_END
	};

	enum GAME_EQUIPMENT
	{
		MAGICAL_POLISH_RECORD_NUM      = 4,   //神兵洗练条数
		MAGICAL_POLISH_ATTR_TYPE       = 7,   //神兵属性种类

		FASHION_TYPE_LIMIT_TIME_USED   = 2,   //限时时装(未过期)已使用
		FASHION_TYPE_EXPIRE            = 3,   //过期时装
		FASHION_TYPE_PERMANENT         = 4,   //永久时装
		FASHION_TYPE_VIP_ONLY		   = 5,	  //VIP专属时装

		FASHION_PART_CAPACITY          = 1000,//衣橱(时装挂载点)的容量

		EQUIP_WEAPON                   	=  1,  	//01武器
		EQUIP_XIANGLIAN                	=  2,  	//02项链
		EQUIP_JIEZHI                   	=  3,  	//03戒指
		EQUIP_HUWAN                    	=  4,  	//04护腕
		EQUIP_TOUKUI                   	=  5,  	//05头盔
		EQUIP_YIFU                  	=  6,  	//06衣服
		EQUIP_HUTUI                   	=  7,  	//07护腿
		EQUIP_ZHANXUE                	=  8,  	//08战靴

		FASHION_YIFU                   =  2000,  //2000时装衣服
		FASHION_WEAPON                 =  11000,  //11000时装武器

		EQUIP_TEMPERED_NUM					= 4,	//装备淬练需要子装备的数量
		EQUIP_END
	};

	enum NAME_COLOR
	{
		NONE,
		NAME_RED		= 98,	//红色
		NAME_YELLOW		= 99,	//黄色
		NAME_WHITE		= 100,	//白色
		NAME_BLUE		= 101,	//蓝色
		NAME_PURPLE		= 102,	//紫色
		END
	};

	enum NewAchieveType
	{
		NEW_ACHIEVE_TYPE_BEGIN 	= 100000,

		ROLE_LEVEL 				= 101000,	//角色等级
		MOUNT_LEVEL 			= 102000,	//战骑等阶
		WING_LEVEL 				= 103000,	//仙羽等阶
		PET_LEVEL 				= 104000,	//灵宠等阶
		MAGIC_EQUIP_LEVEL 		= 105000,	//法器等阶
		GOD_SOLIDER_LEVEL 		= 106000,	//神兵等阶
		BEAST_EQUIP_LEVEL 		= 107000,	//灵武等阶
		BEAST_MOUNT_LEVEL 		= 108000,	//灵骑等阶
		BEAST_WING_LEVEL 		= 109000,	//灵羽等阶
		BEAST_CAT_LEVEL 		= 110000,	//灵猫等阶
		PLOUGH_LEVEL 			= 111000,	//天罡等阶
		MOUNT_SKILL 			= 112000,	//战骑技能
		WING_SKILL 				= 113000,	//仙羽技能
		PET_SKILL 				= 114000,	//灵宠技能
		MAGIC_EQUIP_SKILL 		= 115000,	//法器技能
		GOD_SOLIDER_SKILL 		= 116000,	//神兵技能
		BEAST_EQUIP_SKILL 		= 117000,	//灵武技能
		BEAST_MOUNT_SKILL 		= 118000,	//灵骑技能
		BEAST_WING_SKILL 		= 119000,	//灵羽技能
		BEAST_CAT_SKILL 		= 120000,	//灵猫技能
		PLOUGH_SKILL 			= 121000,	//天罡技能

		DAILY_TASK 				= 201000,	//日常任务
		REWARD_TASK 			= 202000,	//悬赏任务
		LEAGUE_TASK 			= 203000,	//帮派任务
		LEGEND_TOP_SCRIPT 		= 204000, 	//问鼎江湖
		EXP_SCRIPT 				= 205000,	//经验副本
		SWORD_FIGHT 			= 206000,	//问剑江湖

		LEGEND_RANK 			= 301000,	//江湖榜
		SM_BATTLE_KILL 			= 302000,	//杀戮战场
		ESCORT_ACH 				= 303000,	//护送美人
		WORLD_BOSS 				= 304000,	//世界boss

		GET_STATUS_NULL	= 0,   //不能领奖
		GET_STATUS_ABLE = 1,   //能领奖
		GET_STATUS_DONE = 2,   //已领奖

		// 成就细分类型
		ACHIEVE_TYPE_1 	= 1,
		ACHIEVE_TYPE_2 	= 2,
		ACHIEVE_TYPE_3 	= 3,
		ACHIEVE_TYPE_4 	= 4,
		ACHIEVE_TYPE_5 	= 5,

		NEW_ACHIEVE_TYPE_END
	};

	enum FIGHT_STATE
	{
		FIGHT_STATE_NO = 0,		// 非攻击状态
		FIGHT_STATE_ACTIVE = 1,	// 主动攻击状态
		FIGHT_STATE_PASSIVE = 2	// 被动攻击状态
	};

	enum RELIVE_MODE
	{
	    RELIVE_CONFIG_POINT     = 1,    // 复活点复活
	    RELIVE_LOCATE           = 2,    // 原地复活
	    RELIVE_SPECIAL			= 3,	// 特殊点复活

	    RELIVE_MODE_END
	};

	enum FLAUNT_TYPE
	{
		FLAUNT_EQUIP           =  10001,
		FLAUNT_ITEM            =  10002,
		FLAUNT_FASHION         =  10003,
		FLAUNT_GAMEGIFT        =  10004,
		FLAUNT_PET             =  10005,
		FLAUNT_POSITION		   =  10006,

		FLAUNT_TYPE_END
	};

	// 特殊道具
	enum SPECIAL_ITEM
	{
        ITEM_MONEY_UNBIND_GOLD			= 216500001,		//金钱-元宝
        ITEM_MONEY_BIND_GOLD			= 216500002,		//金钱-绑定元宝
        ITEM_ID_PLAYER_EXP				= 216500003,		//经验
        ITEM_ID_REPUTATION				= 216500004,		//声望
        ITEM_ID_HONOUR					= 216500005,		//荣誉
        ITEM_ID_EXPLOIT					= 216500006,		//功勋
        ITEM_ID_LEAGUE_CONTRI			= 216500007,		//帮贡
        ITEM_ID_REIKI					= 216500008,		//灵气
        ITEM_ID_PRACTICE             	= 216500010,        //历练
        ITEM_ID_SPIRIT					= 216500011,		//精华
        ITEM_TODAY_UNBIND_GOLD			= 216500101,		//今天元宝
        ITEM_TODAY_BIND_GOLD			= 216500102,		//今天绑元
        GOODS_UBNID_GOLD				= 213100001,		//物品-元宝
        GOODS_BIND_GOLD					= 213110001,		//物品-绑定元宝
        SPECIAL_ITEM_END
	};

	enum RANK_SYSTEM
	{
		RANK_RECORD_DEFAULT_LIMIT_NUM   =  100,

		RANK_DATA_SEND_FIRST            =   20,
		RANK_DATA_SEND_SECOND           =   10,

		RANK_QUERY_PET_DETAIL           =    1,
		RANK_QUERY_OTHER_DETAIL         =    2,
		RANK_QUERY_TRAVEL_PET			= 	 3,

		RANK_SYSTEM_END
	};

	enum ObjectType
	{
		MAP_LOGIC_OBJECT_TYPE_ERROR = 0,

		/////// MAP LOGIC
		MAP_LOGIC_MINIGAME_OBJECT = 10001,
		MAP_LOGIC_ONCE_REWARDS = 10003,

		MAP_LOGIC_OPEN_ACT_DETAIL = 10004,
		MAP_LOGIC_OPEN_ACT_GLOBAL_DETAIL = 10005,

		///////// MAP
		MAP_OBJ_HOME_CITY_BLESS = 20001,

		MAP_LOGIC_OBJECT_TYPE_END
	};

	enum CampType
	{
		CAMP_NONE	= 0,
		CAMP_ONE	= 1,
		CAMP_TWO	= 2,
		CAMP_THREE	= 3,

		CAMP_END
	};

	enum BrocastParseType
	{
		PARSE_TYPE_INT               =  1,
		PARSE_TYPE_INT_64            =  2,
		PARSE_TYPE_STRING            =  3,
		PARSE_TYPE_PROTOBROCASTROLE  =  4,
		PARSE_TYPE_ITEM_ID			 =  5,
        PARSE_TYPE_ITEM_TIPS         =  6,

		PARSE_TYPE_END
	};

    enum ScriptCleanState
    {
        SCRIPT_CS_STOP     = 0,    // 未开始
        SCRIPT_CS_START    = 1,    // 扫荡中
        SCRIPT_CS_END
    };

    enum ScriptCleanType
    {
        SCRIPT_CT_ALL       = 0,    // 全部扫荡
        SCRIPT_CT_SINGLE    = 1,    // 扫荡指定副本
        SCRIPT_CT_END
    };

    enum PlayerAssistEvent
    {
    	PA_EVENT_MAIL               		= 10001,	//邮件
    	PA_EVENT_TRVL_TOTAL_TIMES			= 10002,	//跨服副本
    	PA_EVENT_SWORD_POOL_LEVEL_UP		= 10004,	//剑池升级
    	PA_EVENT_WORLD_BOSS					= 10005,	//世界boss
    	PA_EVENT_LEAGUE_WAR					= 10006,	//帮派争霸
    	PA_EVENT_SEVEN_DAY					= 10007,	//七天登录
    	PA_EVENT_FRIEND_ACCEPT				= 10008,	//好友申请列表
    	PA_EVENT_TARENA_STAGE_REWARD		= 10012,	//跨服竞技段位奖励
    	PA_EVENT_TARENA_WIN_REWARD			= 10013,	//跨服竞技赢奖励

    	PA_EVENT_LABEL						= 10180, 	//称号
    	PA_EVENT_SCRIPT_ADVANCE_FIGHT		= 10201,	//进阶副本挑战
    	PA_ENENT_SCRIPT_EXP_SWEEP			= 10211,	//经验副本扫荡
    	PA_EVENT_SCRIPT_EXP_BOX				= 10212,	//经验副本宝箱
    	PA_EVENT_SCRIPT_STORY_FIGHT			= 10221,	//剧情副本挑战
    	PA_EVENT_SCRIPT_STORY_SWEEP			= 10222,	//剧情副本扫荡
    	PA_EVENT_SCRIPT_LEGEND_SWEEP		= 10231,	//问鼎江湖扫荡
    	PA_EVENT_SCRIPT_VIP_FIGHT			= 10241,	//VIP副本挑战
    	PA_EVENT_SCRIPT_RAMA_SWEEP			= 10251,	//罗摩密境扫荡
    	PA_EVENT_SCRIPT_RAMA_FIGHT			= 10252,	//罗摩密境挑战
    	PA_EVENT_SCRIPT_SWORD_SWEEP			= 10261,	//论剑武林扫荡

    	PA_EVENT_CHAT_PRIVATE				= 10301,	//聊天--私聊
    	PA_EVENT_CHAT_LEAGUE				= 10311,	//聊天--帮派

    	PA_EVENT_LEAGUE_WELFARE				= 10501,	//可领取帮派福利
    	PA_EVENT_LEAGUE_ACCEPT				= 10502,	//帮派申请列表
    	PA_EVENT_LEAGUE_FLAG				= 10503,	//帮派旗帜
    	PA_EVENT_LEAGUE_BOSS_FEED			= 10504,	//帮派神兽喂养
    	PA_EVENT_ATTACK_LEAGUE_SKILL		= 10511,	//帮派攻击技能升级
    	PA_EVENT_DEFENCE_LEAGUE_SKILL		= 10512,	//帮派防御技能升级
    	PA_EVENT_CRIT_LEAGUE_SKILL			= 10513,	//帮派暴击技能升级
    	PA_EVENT_TOUGHNESS_LEAGUE_SKILL		= 10514,	//帮派抗暴技能升级
    	PA_EVENT_HIT_LEAGUE_SKILL			= 10515,	//帮派命中技能升级
    	PA_EVENT_AVOID_LEAGUE_SKILL			= 10516,	//帮派闪避技能升级
    	PA_EVENT_BLOOD_MAX_LEAGUE_SKILL		= 10517,	//帮派生命技能升级

    	PA_EVENT_ARENA_CHALLENGE_NEW		= 10601,	//江湖帮挑战
//    	PA_EVENT_ARENA_SHOP					= 10602,	//江湖帮商店
    	PA_EVENT_ARENA_AWARD				= 10603,	//江湖榜奖励
    	PA_EVENT_ARENA_OPEN					= 10604, 	//江湖帮开启

    	PA_EVENT_ONLINE_AWRAD				= 10701,	//在线奖励
    	PA_EVENT_CHECK_IN_AWARD				= 10702,	//签到奖励
    	PA_EVENT_OFFLINE_AWARD				= 10703,	//离线奖励

    	PA_EVENT_SMELT_OPEN					= 10801, 	//锻造开启
        PA_EVENT_EQUIP_STRENGTHEN 			= 10802,	//装备强化
        PA_EVENT_EQUIP_INSERT_JEWEL         = 10803,	//宝石镶嵌
        PA_EVENT_ITEM_MERGE                 = 10804,	//宝石合成
        PA_EVENT_SMELT						= 10805,	//锻造熔炼
        PA_EVENT_EQUIP_GOOD_REFINE			= 10806,	//锻造精炼
        PA_EVENT_EQUIP_RED_UPRISING       	= 10807,	//锻造传说
		PA_EVENT_EQUIP_MOLDING_SPIRIT		= 10808,	//装备铸魂

    	PA_EVENT_RAMA_OPEN					= 10902,	//罗摩开启
    	PA_EVENT_RAMA_GET					= 10901,	//罗摩获得

    	PA_EVENT_FIELD_ILLUS				= 11001,	//野外图鉴升级
    	PA_EVENT_LEGEND_ILLUS				= 11002,	//跨服图鉴升级
    	PA_EVENT_RAMA_ILLUS					= 11003,	//罗摩图鉴升级
    	PA_EVENT_ILLUS_OPEN					= 11004,	//图鉴开启

    	/*************************no use****************************/
    	PLAYER_ASSIST_EVENT_LIMIT_FASHION      =      1002,//限时时装
    	PLAYER_ASSIST_EVENT_FRONTREA           =      1003,//修炼[去掉]
    	PLAYER_ASSIST_EVENT_VIP                =      1004,//vip奖励
    	PLAYER_ASSIST_EVENT_ACHIEVEMENT        =      1005,//成就
    	PLAYER_ASSIST_EVENT_VIP_TIME           =      1006,//vip到期提醒
    	PLAYER_ASSIST_EVENT_EXPIRE_FASHION     =      1007,//时装过期提醒
    	PLAYER_ASSIST_EVENT_VIP_EXPIRE         =      1008,//vip过期提醒
    	PLAYER_ASSIST_EVENT_PACK_GRID_OPEN     =      1009,//可以免费开启背包格子
    	PLAYER_ASSIST_EVENT_ARENA_REWARD	   =      1010,//竞技场奖励
    	PLAYER_ASSIST_EVENT_MAGICAL_ARMS	   = 	  1011,//神兵可激活
    	PLAYER_ASSIST_EVENT_DRAGON_ROAD_CD	   =	  1012,//宠物副本可进行
    	PA_EVNET_LEAGUE_SALARY				   = 	  1013,//仙盟工资
    	PA_EVENT_SCRIPT_CLEAN				   = 	  1014,//副本扫荡奖励提醒
    	PA_EVENT_SCRIPT_COMPACT				   = 	  1015,//副本契约到期提醒
    	PA_EVENT_SKILL_LVLUP				   = 	  1016,//技能升级提醒
    	PA_EVENT_REINCAMATION				   =      1017,//转生提醒
        PA_EVENT_EQUIP_TEMPERED                =      1021,//装备淬炼提醒
        PA_EVENT_LEAGUE_NEW_APPLY              =      1023,//仙盟新申请提醒
        PA_EVENT_BOSS_CHALLENGE                =      1024,//BOSS可挑战提醒
        PA_EVENT_EQUIP_POLISH                  =      1025,//装备洗炼提醒
        PA_EVENT_ARENA_CHALLENGE               =      1026,//竞技场可挑战提醒
        PA_EVENT_SCRIPT						   = 	  1028,//副本可打提醒
        PA_EVENT_SACRIFICE					   =	  1029,//祭祀提醒
        PA_EVENT_PANIC_SHOP					   = 	  1030,//开服抢购
        PA_EVENT_BOX_FREE					   = 	  1031,//藏宝库免费提醒
        PA_EVENT_BOX_NEW_ITEM				   = 	  1032,//藏宝库新加物品提醒
        PA_EVENT_FUN_ACTIVITY_MERGE			   = 	  1034,	//合服活动有奖励通知
        PA_EVENT_FUN_ACTIVITY_MORE			   = 	  1035,	//跨服活动有奖励通知
        PA_EVENT_FUN_ACTIVITY_DAILY			   = 	  1036,	//日常活动有奖励通知
        PA_EVENT_FUN_ACTIVITY_FEAST			   = 	  1037,	//节日活动有奖励通知
        PA_EVENT_BEAST_GROWTH                  =      1038, //宠物成长提醒
        PA_EVENT_BEAST_REFINE_SOUL             =      1039, //宠物灵修提醒
        PA_EVENT_BEAST_COMBINE                 =      1040, //宠物融合提醒
        PA_EVENT_MOUNT_EVOLUATE                =      1041, //坐骑进货提醒
        PA_EVENT_WJSL_SCRIPT				   = 	  1043,	//无尽试炼提醒
        PA_EVENT_PRACTICE					   =	  1044, //修行提醒
        PA_EVENT_BROTHER_TASK				   =	  1045, //结义任务提醒
        PA_EVENT_49_VIP						   =	  1046, //49游至尊VIP
        PA_EVENT_STRENGTH					   =	  1047, //可领取体力
        PA_EVENT_MAGIC_ACT				   	   = 	  1048,	//神器
        PA_EVENT_LOVE_GIFT				   	   = 	  1049,	//爱心礼包
		PA_EVENT_DROP_DRAGON_HOLE			   =      1058, //坠龙窟通知


    	PLAYER_ASSIST_EVENT_END
    };


	enum FlowControlDetail
	{
		SERIAL_RECORD               =           1, // 是否开启流水总开关
		MONEY_SERIAL_RECORD         =           2, // 是否开启金钱流水
		ITEM_SERIAL_RECORD          =           3, // 是否开启道具流水
		EQUIP_SERIAL_RECORD         =           4, // 是否开启装备流水
		MOUNT_SERIAL_RECORD         =           5, // 是否战骑类流水
		PET_SERIAL_RECORD           =           6,
		SKILL_SERIAL_RECORD         =           7,
		MAIL_SERIAL_RECORD          =           8, // 是否开启邮件流水
		MARKET_SERIAL_RECORD        =           9,
		ACHIEVEMENT_SERIAL_RECORD   =          10,
		IS_FORBIT_LOGIN				=		   11, // 是否禁止登录
		PLAYER_LEVEL_RECORD         =          12, // 是否开启玩家等级流水
		OTHER_SERIAL_RECORD         =          13, // 是否开启其他流水
		ONLINE_USER_RECORD          =          14, // 是否开启玩家在线情况流水
		LOGIN_RECORD                =          15, // 是否开启玩家登录情况流水
		TASK_RECORD                 =          16, // 是否开启任务流水
		RANK_RECORD                 =          17, // 是否开启排行流水
		CHAT_RECORD					=		   18, // 是否开启聊天流水

		FLOW_CONTROL_DETAIL_TYPE_END
	};

	enum ExpRestoreType		//资源找回
	{
		ER_TYPE_FREE		= 1,
		ER_TYPE_MONEY		= 2,

		ERTYPE_END
	};

	enum RestoreActType 	//资源找回活动大类
	{
		//进阶副本
		ES_ACT_FUN_MOUNT		= 1,	//战骑
		ES_ACT_FUN_MAGIC_EQUIP	= 2,	//法器
		ES_ACT_FUN_LING_BEAST	= 3,	//灵宠
		ES_ACT_FUN_XIAN_WING	= 4,	//仙羽
		ES_ACT_FUN_GOD_SOLIDER	= 5,	//神兵
		ES_ACT_FUN_BEAST_EQUIP	= 6,	//灵武
		ES_ACT_FUN_BEAST_MOUNT	= 7,	//灵骑
		ES_ACT_FUN_BEAST_WING	= 8,	//灵羽
		ES_ACT_FUN_BEAST_MAO	= 9,	//灵猫
		ES_ACT_FUN_TIAN_GANG	= 10,	//天罡

		ES_ACT_EXP_FB 			= 11,	//经验副本

		//剧情副本
		ES_ACT_STORY_FB_1		= 12,
		ES_ACT_STORY_FB_2		= 13,
		ES_ACT_STORY_FB_3		= 14,
		ES_ACT_STORY_FB_4		= 15,
		ES_ACT_STORY_FB_5		= 16,
		ES_ACT_STORY_FB_6		= 17,
		ES_ACT_STORY_FB_7		= 18,

		ES_ACT_LEGEND_TOP		= 22,	//问鼎

		//vip副本
		ES_ACT_VIP_1_FB			= 23,
		ES_ACT_VIP_3_FB			= 24,
		ES_ACT_VIP_6_FB			= 25,
		ES_ACT_VIP_9_FB			= 26,

		//环式任务
		ES_ACT_DAILY_ROUTINE	= 30,
		ES_ACT_LEAGUE_ROUTINE	= 31,
		ES_ACT_OFFER_ROUTINE	= 32,

		ES_ACT_ARENA			= 35,	//江湖榜
		ES_ACT_TRVL_ARENA		= 36,	//跨服
		ES_ACT_SM_BATTLE		= 37,	//杀戮

		ES_ACT_TYPE_BEGIN 		= ES_ACT_FUN_MOUNT,
		ES_ACT_TYPE_END 		= ES_ACT_SM_BATTLE,

		ES_ACT_BASE_NUM = 10000
	};

	enum TipsSysCondition
	{
		JOIN_CONDITION_MATCH = 0,
		JOIN_LEVEL_UNMATCH = 1,
		JOIN_HAS_NO_LEAGUE = 2,
		JOIN_NO_BLIND_ICON = 3,
        JOIN_SHOW_LEVEL_LIMIT = 4,
        JOIN_SIGNUP_TICK = 5,		// 巅峰对决报名阶段
        JOIN_BET_TICK = 6,			// 巅峰对决下注阶段
		SHOW_LEVEL_UNMATCH = 1001,

		JOIN_FAIL_CONDITION_END
	};

	enum SyncLogicInfoType
	{// 和MapPlayer中定义相同
		SYNC_LOGIC_FORCE = 1,
		SYNC_LOGIC_END
	};

	enum PermissionType
	{
		PERT_NORMAL_PLAYER	= 1,
		PERT_GM				= 2,
		PERT_DEVELOPER		= 4,
		PERT_NOVICE_GUIDE	= 8,
		PERT_INNER_PLAYER	= 16,
		PERT_END
	};

	enum ActivityType
	{
		DAILY_ACTIVITY		= 1,	//每天
		WEEKLY_ACTIVITY		= 2		//每周
	};

	enum ActivityTipsState
	{
		ACTIVITY_STATE_NO_START	= 0,	//未开始，结束
		ACTIVITY_STATE_AHEAD	= 1,	//预告
		ACTIVITY_STATE_START	= 2,	//开始
		ACTIVITY_STATE_END		= 3,
		ACTIVITY_STATE_ONGOING	= 4,
		ACTIVITY_STATE_NEXT		= 5,

		ACTIVITY_STATE_ENUM
	};

    enum CommercialSys
    {
    	COMM_SYS_START = 0,
		COMM_SYS_BEAST_GROWTH,
		COMM_SYS_BEAST_LEARN_SKILL,
		COMM_SYS_BEAST_REFINE_SOUL,
    	COMM_SYS_MOUNT,
    	COMM_SYS_DG,
    	COMM_SYS_EQ_REFINE,
    	COMM_SYS_EQ_UP,
    	COMM_SYS_DIVINE,
    	COMM_SYS_MAIL,
    	COMM_SYS_AUCTION,
    	COMM_SYS_MALL,
    	COMM_SYS_SHOP,
    	COMM_SYS_LEAGUE,
    	COMM_SYS_SKILL_STRENTHEN,

    	COMM_SYS_END
    };

    enum BanOperation
    {
    	OPER_BAN_NONE = 0,
    	OPER_BAN_ACCOUNT = 1,
    	OPER_BAN_ROLE = 2,
    	OPER_BAN_IP = 3,
    	OPER_BAN_SPEAK = 4,
    	OPER_BAN_SPEAK_TRICK = 5,
    	OPER_BNA_KICK = 6,
		OPER_BAN_MAC = 7,
		OPER_BAN_END
    };

    enum BanOperationType
    {
    	BAN_OPT_RESTRIC = 1,		// 封禁
    	BAN_OPT_DERESTRIC = 2,		// 解封
    };

    enum LvlRewardsCond
    {
    	LR_COND_NOT_REACH = 0,
    	LR_COND_REACH,
    	LR_COND_HAS_GET,
    };

    enum DailyLivenessCondID
    {
		DL_COND_ID_SIGN_IN = 1,	// 每天登陆
		DL_COND_ID_VIP_SIGN_IN = 2,	// VIP每天登陆
    	DL_COND_ID_EQUIP_TEMPERED = 4,	//装备分解
    	DL_COND_ID_BEAST_MERGE = 8,	//宠物融合
    	DL_COND_ID_BOX = 10,	//藏宝库抽奖
		DL_COND_ID_MALL_GOOD_TIMES = 11,	// 商城购买物品次数

    	DL_COND_ID_TRIAL_TASK = 15, //试炼任务
    	DL_COND_ID_RING_TASK = 16, //环式任务
    	DL_COND_ID_LEAGUE_DONATE = 28, // 仙盟贡献

    	DL_COND_ID_END
    };

    // 观战状态
    enum WatchStatus
    {
    	WATCH_STATUS_NULL 		= 0,
    	WATCH_STATUS_ENTERING 	= 1,
    	WATCH_STATUS_WATCHING 	= 2,

    	WATCH_STATUS_END
    };

    enum MatchResults
    {
    	MATCH_NULL = -99,

    	MATCH_LOSE = -1,
    	MATCH_DRAW = 0,
    	MATCH_WIN  = 1
    };

    enum ValidTeamCondition
    {
    	EXTRA_CONDITION_NONE				= 0x00,
    	EXTRA_CONDITION_SEVEN_PROGRESS		= 0x01,
    	EXTRA_CONDITION_TEAM_EXISTS			= 0x02,
    	EXTRA_CONDITION_SEVEN_TIME			= 0x04,
    	EXTRA_CONDITION_ALL					= 0xff
    };

    enum PRACTICE
    {
    	PRACTICE_COPPER						= 0,
    	PRACTICE_GOLD						= 1,
    	PRACTICE_HIGH_DIRAECT				= 0,
    	PRACTICE_GOLD_DIRAECT				= 1,
    	PRACTICE_NIMBUS_CLOUD				= 0,
    	PRACTICE_GOD_CLOUD					= 1,
    	PRACTICE_CLOUD_NUM					= 10
    };

    enum Color
    {
    	WHITE			= 1,	//白
    	GREEN			= 2,	//绿
    	BLUE			= 3,	//蓝
    	VIOLET			= 4,	//紫
    	ORANGE			= 5,	//橙
    	RED				= 6,	//红1
    	RED2			= 7,	//红2
    	RED3			= 8,	//红3
    	RED4			= 9,	//红4
    	RED5 			= 10,	//红5
    	RED6			= 11,	//红6
    	RED7			= 12,	//红7
    	RED8			= 13,	//红8
    	EQUIP_COLOR_END
    };

    enum BoxFreeState
    {
    	BOX_FREE_TIME = 1,	//到免费时间
    	BOX_FREE_MUST = 2,	//必出
    	BOX_FREE_LUCKY = 4,	//幸运值满
    };
    enum ExternAttrType
    {
    	COME_FROM_LUCK = 0,	//抽奖
    	COME_FROM_EQUIP_TEMPERED = 1,	//装备淬练或橙炼
    	COME_FROM_ACTIVITY = 2,		//活动（打boss掉落等）
    	COME_FROM_EQUIP_UPRISE = 3,		//装备升阶
    };
    enum AI_ACTION_TYPE
    {
        AAT_NO_FLAG = 0,
        AAT_BACK,           // 往回走的行为
        AAT_IDLE_BACK,      // 空闲时往回走的行为
        AI_ACTION_TYPE_END
    };
    enum LSTORE_OPT
    {
    	LSTORE_APPLY = 0,	//申请
    	LSTORE_OUT = 1,     //取出
    	LSTORE_IN = 2,    	//放入
    	LSTORE_END
    };


    enum
	{
    	ENTER_ADORN_DEFAULT,
    	ENTER_DEMOUNT,
    	ENTER_LEVELUP,
        ENTER_END
    };

    //休闲玩法相关
    enum
	{
    	FIRST_COLLECT_CHESTS_ID = 20041,
    	SECOND_COLLECT_CHESTS_ID = 20042,
    	COLLECT_CHESTS_SCENE_ID = 10200,
    	PLAYER_COLLECT_MAX_NUM = 20,
    	ANSWER_ACTIVITY_ID = 20031,
        COLLECT_END
    };

    //罗摩相关信息
    enum
	{
    	RAMA_ID_BASE_NUM = 10000,
    	RAMA_BASE_RAND_NUM = 10000,
    	RAMA_BASS_LEVEL = 1,
    	RAMA_CLASS_ID_BEGIN = 1,
    	RAMA_CLASS_ID_END = 9,
    	RAMA_GET_FROM_ONLINE = 1,
    	RAMA_GET_FROM_VIP = 2,
    	RAMA_GET_FROM_TASK = 3,
    	RAMA_GET_FROM_RECHARGE = 4,
    	RAMA_INFO_END
    };

    //五倍挂机
    enum
	{
    	QUINTUPLE_ACTIVITY_ID = 20061,
    	QUINTUPLE_START_SHOUT = 70601,
    	QUINTUPLE_END_SHOUT = 70602,
    	QUINTUPLE_BUFF_PERCENT = 40000,
    	QUINTUPLE_INFO_END
    };

    //江湖榜
    enum
    {
    	ARENA_RANK_START = 1,
    	ARENA_RANK_END = 50,
    	ARENA_RANK_NUM = 100,
//    	AREA_LIMIT_TIME = 1800,
    	ARENA_BASE_RANK_NUM = 310,
    	ARENA_BASE_ID_NUM = 1000,
    	ARENA_RANK_INFO_END
    };

    enum
    {
    	ESCORT_ACTIVITY_FIRST_ID	= 20051,
    	ESCORT_ACTIVITY_SECOND_ID	= 20052,
    	ESCORT_INFO_OPEN = 1,
    	ESCORT_INFO_UPGRADE = 2,
    	ESCORT_INFO_SELECT = 3,
    	ESCORT_END
    };

    enum BRANCH_TYPE //支线任务类型
    {
    	BRANCH_EQUIP = 1,			//锻造强化
    	BRANCH_EXP_FB = 10,			//经验
    	BRANCH_LEGEND_TOP		= 80,	//问鼎
    	BRANCH_LEAGUE			= 90, 	//加入帮派
    	BRANCH_FIRST_RECHARGE	= 91,	//首次充值
    	BRANCH_LEVEL 			= 100,	//等级
    	BRANCH_WEDDING			= 101,	//婚姻

    	BRANCH_MOUNT		= 20101,	//战骑
    	BRANCH_MAGIC_EQUIP	= 20102,	//法器
    	BRANCH_LING_BEAST	= 20103,	//灵宠
    	BRANCH_XIAN_WING	= 20104,	//仙羽
    	BRANCH_GOD_SOLIDER	= 20105,	//神兵
    	BRANCH_BEAST_EQUIP	= 20106,	//灵武
    	BRANCH_BEAST_MOUNT	= 20107,	//灵骑
    	BRANCH_BEAST_WING	= 20108,	//灵羽
    	BRANCH_BEAST_MAO	= 20109,	//灵猫
    	BRANCH_TIAN_GANG	= 20110,	//天罡

		//剧情副本
    	BRANCH_STORY_FB_1		= 20301,
    	BRANCH_STORY_FB_2		= 20302,
    	BRANCH_STORY_FB_3		= 20303,
    	BRANCH_STORY_FB_4		= 20304,
    	BRANCH_STORY_FB_5		= 20305,
    	BRANCH_STORY_FB_6		= 20306,
    	BRANCH_STORY_FB_7		= 20307,

    	BRANCH_FB_BEGIN 		= BRANCH_MOUNT,
    	BRANCH_FB_END			= BRANCH_STORY_FB_7,
    	BRANCH_TYPE_END
    };

    enum PROPERTY_TYPE
    {
    	WEDDING_TYPE			= 1, //结婚属性

    	TYPE_END
    };

    enum CUMULATIVE_LOGIN_STATE
    {
    	ALL = 0,
    	MULTIPLY,
    	SINGLE,
    	TEN,
    	HUNDRED
    };

	//聚宝盆任务
	enum
	{
		CORNUCOPIA_TASK_BEGIN = 1,
		CORNUCOPIA_TASK_FIRE_RIDING_PROGRESS = CORNUCOPIA_TASK_BEGIN,		//战骑进阶
		CORNUCOPIA_TASK_WING_PROGRESS,										//仙羽进阶(神翼进阶)
		CORNUCOPIA_TASK_TROOPS_PROGRESS,									//神兵进阶(魔刃进阶)
		CORNUCOPIA_TASK_MAGIC_PROGRESS,										//法器进阶(宝器进阶)
		CORNUCOPIA_TASK_PET_PROGRESS,										//灵宠进阶(仙童进阶)
		CORNUCOPIA_TASK_WEAPON_PROGRESS,									//灵武进阶(仙武进阶)
		CORNUCOPIA_TASK_NIMBUS_RIDING_PROGRESS,								//灵骑进阶(仙兽进阶)
		CORNUCOPIA_TASK_DAILY,												//日常任务
		CORNUCOPIA_TASK_LEAGUE,												//帮会任务
		CORNUCOPIA_TASK_WBOSS,												//世界boss
		CORNUCOPIA_TASK_TRVL_SCRIPT,										//跨服副本
		CORNUCOPIA_TASK_EQUIP_REFINE,										//装备强化
		CORNUCOPIA_TASK_ESCORT,												//护送
		CORNUCOPIA_TASK_LEGEND,												//挑战江湖榜
		CORNUCOPIA_TASK_FLOWERS,											//好友送花
		CORNUCOPIA_TASK_ADVANCE_SCRIPT,										//进阶副本
		CORNUCOPIA_TASK_END
	};

	enum
	{
		LABOUR_TASK_BEGIN = 1,
		LABOUR_TASK_WBOSS = LABOUR_TASK_BEGIN,								//世界boss
		LABOUR_TASK_KILL_MONSTER,											//击杀怪物
		LABOUR_TASK_ESCORT,													//护送
		LABOUR_TASK_BROCAST_BATTLEGROUND,									//玄武战场
		LABOUR_TASK_TRVL_SCRIPT,											//跨服副本
		LABOUR_TASK_EXPEND_BIND_MONEY,										//消耗绑定元宝
		LABOUR_TASK_EXPEND_MONEY,											//消耗元宝
		LABOUR_TASK_END
	};

	enum
	{
		MOLDING_SPIRIT_BEGIN = 0,
		MOLDING_SPIRIT_ATTACK = MOLDING_SPIRIT_BEGIN,					//攻击
		MOLDING_SPIRIT_DEFENSE,											//防御
		MOLDING_SPIRIT_BLOOD,											//血量
		MOLDING_SPIRIT_ALL_NATURE,										//所有属性
		MOLDING_SPIRIT_END
	};
};

#endif /* GAMEENUM_H_ */
