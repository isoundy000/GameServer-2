/*
 * GameField.h
 *
 * Created on: 2013-02-20 16:27
 *     Author: glendy
 */

#ifndef _GAMEFIELD_H_
#define _GAMEFIELD_H_

#include <string>

typedef const std::string DBField;

struct DBPairObj
{
	static DBField KEY;
	static DBField VALUE;
	static DBField TICK;
	static DBField NAME;
};

struct DBItemObj
{
	static DBField ID;
	static DBField AMOUNT;
	static DBField BIND;
	static DBField INDEX;
};

struct Global
{
    static DBField COLLECTION;
    static DBField KEY;
    static DBField ROLE;
    static DBField GID;
    static DBField CONTENT;
    static DBField MAIL;
    static DBField FRIENDSHIP;
    static DBField LEAGUE;
    static DBField VOICE_ID;
    static DBField BEAST;
    static DBField CUSTOMER_SERVICE;
    static DBField SCRIPT_PROGRESS;
    static DBField WEDDING;
    static DBField LSTORE_APPLY;
    static DBField LSTORE_ITEM_ID;
	static DBField BROTHER;
    static DBField TRAVEL_TEAM;
    static DBField AVERAGE_ROLE_LEVEL;
};

struct Role
{
    static DBField COLLECTION;
    static DBField IS_ACTIVE;
    static DBField ACCOUNT;
    static DBField SERVER_FLAG;
    static DBField AGENT;
    static DBField AGENT_CODE;
    static DBField PLATFORM;
    static DBField PLATFORM_CODE;
    static DBField MARKET_CODE;
    static DBField IS_NEW;
    static DBField IS_FIRST_RENAME;
    static DBField BAN_TYPE;
    static DBField BAN_EXPIRED;
    static DBField ID;
    static DBField NAME;
    static DBField SRC_NAME;
    static DBField FULL_NAEM;
    static DBField LEVEL;
    static DBField EXP;
    static DBField SEX;
    static DBField CAREER;
    static DBField VIP_TYPE;
    static DBField FORCE;
    static DBField PREV_FORCE_MAP;
    static DBField WATCH_STATUS;
    static DBField IS_TRANSFER_WATCH;
    static DBField FIRST_WBOSS_TIME;
    static DBField ALREADY_LEAGUE;
    static DBField LEAGUE_ID;
    static DBField LEAGUE_NAME;
    static DBField PARTNER_ID;
    static DBField PARTNER_NAME;
    static DBField WEDDING_ID;
    static DBField WEDDING_TYPE;
    static DBField SAVE_TRVL_SCENE;
    static DBField DAY_RESET_TICK;
    static DBField WEEK_RESET_TICK;
    static DBField ML_DAY_RESET_TICK;
    static DBField VIEW_TICK;
    static DBField NOTICE_DRAW_TICK;
    static DBField SHAPE_DETAIL;
    static DBField LABEL_INFO;
    static DBField BUY_MAP;
    static DBField BUY_TOTAL_MAP;
    static DBField PANIC_BUY_NOTIFY;
    static DBField MOUNT_INFO;

    static DBField WEDDING_SELF;
    static DBField WEDDING_SIDE;

    static DBField SERVER_TICK;
    static DBField COMBINE_TICK;
    static DBField IS_YELLOW;
    static DBField KILL_NUM;
    static DBField KILL_NORMAL;
    static DBField KILL_EVIL;
    static DBField IS_BROCAST;
    static DBField ONLINE_TICKS;
    static DBField KILL_VALUE;
    static DBField BROTHER_REWARD_INDEX;
    static DBField TRANSLATE_TO_ENEMY_TIMES;
    static DBField IS_WORSHIP;
    static DBField TODAY_RECHARGE_GOLD;
    static DBField TODAY_CONSUME_GOLD;
    static DBField TODAY_BUY_TIMES;
    static DBField TODAY_CAN_BUY_TIMES;
    static DBField GASHAPON_BUY_TIMES;
    static DBField CONTINUITY_LOGIN_DAY;
    static DBField CONTINUITY_LOGIN_FLAG;

    static DBField CREATE_TIME;
    static DBField IP;
    static DBField LAST_SIGN_IN;
    static DBField LAST_SIGN_OUT;
    static DBField LOGIN_TICK;
    static DBField LOGIN_DAYS;
    static DBField LOGIN_COUNT;
    static DBField PERMISSION;
    static DBField DRAW_DAY;
    static DBField DRAW_GIFT;
    static DBField DRAW_VIP;
    static DBField RAND_USE_TIMES;
    static DBField RECHARGE_TOTAL_GOLD;
    static DBField SCENE_PK_STATE;
    static DBField WEDDING_GIFTBOX_TICK;
    static DBField WEDDING_GIFTBOX_TIMES;
    static DBField FRESH_FREE_RELIVE_TICK;
    static DBField USED_FREE_RELIVE;
    static DBField SACREDSTONE_END_TICK;
    static DBField COLLECT_CHEST_AMOUNT;
    static DBField SACREDSTONE_EXP;
    static DBField CHANGE_NAME_TICK;
    static DBField CHANGE_SEX_TICK;
    static DBField OPEN_GIFT_CLOSE;
    static DBField LAST_ACT_TYPE;
    static DBField LAST_ACT_END_TIME;

    static DBField LOCATION;
    struct Location
    {
        static DBField SCENE_ID;
        static DBField PIXEL_X;
        static DBField PIXEL_Y;
        static DBField TOWARD;
        static DBField MODE;
        static DBField SPACE_ID;

        static DBField TEMP_PIXEL_X;
        static DBField TEMP_PIXEL_Y;
        static DBField PREV_SCENE_ID;
        static DBField PREV_PIXEL_X;
        static DBField PREV_PIXEL_Y;
        static DBField PREV_MODE;
        static DBField PREV_SPACE_ID;

        static DBField PREV_TOWN_SCENE;
        static DBField PREV_TOWN_X;
        static DBField PREV_TOWN_Y;

        static DBField SCENE_HISTORY;
    };

    static DBField SAVE_INFO;
    struct SaveInfo
    {
    	static DBField SCENE_ID;
    	static DBField PK_STATE;
    	static DBField BLOOD;
    	static DBField MAGIC;
    };

	static DBField PRACTICE_TIMES;
	static DBField PRACTICE_GOLD_TIMES;
	static DBField PRACTICE_CLOUDS;
	static DBField ESCORT_TIMES;
	static DBField PROTECT_TIMES;
	static DBField ROB_TIMES;

	static DBField SPECIAL_BOX_INFO;
	struct SpecialBoxInfo
	{
		static DBField BUY_TIMES;
		static DBField SCORE;
		static DBField REFRESH_TIMES;
	};
};

struct RoleEx
{
    static DBField COLLECTION;
    static DBField ID;

    static DBField BOX_OPEN_COUNT_ONE;//藏宝库-打开次数统计
    static DBField BOX_OPEN_COUNT_TEN;//藏宝库-打开次数统计
    static DBField BOX_OPEN_COUNT_FIFTY;//藏宝库-打开次数统计
    static DBField SAVVY;	// 悟性
    static DBField BOX_IS_OPEN;	// 藏宝库是否开启
    static DBField SECOND_DECOMPOSE;	// 装备非第一次分解
    static DBField LTABLE_LEFT_TIMES;	// 幸运转盘剩余次数
    static DBField LTABLE_EXEC_TIMES;	// 幸运转盘抽取次数
    static DBField LTABLE_GOLD;	// 幸运转盘累计充值消耗
    static DBField LTABLE_KEY;	// 幸运转盘活动索引
};

struct DBCopyPlayer
{
	static DBField COLLECTION;

	static DBField ID;
	static DBField NAME;
	static DBField LEVEL;
	static DBField VIP_TYPE;
	static DBField SEX;
	static DBField FORCE;
	static DBField CAREER;
	static DBField LEAGUE_ID;
	static DBField LEAGUE_NAME;
	static DBField LEAGUE_POS;

	static DBField PARTNER_ID;
	static DBField PARTNER_NAME;
	static DBField WEDDING_ID;
	static DBField WEDDING_TYPE;
	static DBField FASHION_ID;
	static DBField FASHION_COLOR;

	static DBField ATTACK_LOWER;
	static DBField ATTACK_UPPER;
	static DBField DEFENCE_LOWER;
	static DBField DEFENCE_UPPER;
	static DBField HIT;
	static DBField DODGE;
	static DBField CRIT;
	static DBField TOUGHNESS;
    static DBField LUCKY;
	static DBField BLOOD;
	static DBField MAGIC;
	static DBField SPEED;
	static DBField DAMAGE;	//伤害加成
	static DBField REDUCTION;	//伤害减免
	static DBField WING_LEVEL;
	static DBField SOLIDER_LEVEL;
	static DBField WEAPON_LEVEL;
	static DBField MAGIC_LEVEL;
	static DBField BEAST_LEVEL;
	static DBField MOUNT_LEVEL;
	static DBField EQUIP_REFINE_LVL;

	static DBField SKILL_SET;
	static DBField SHAPE_SET;
};

struct Vip
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField TYPE;
	static DBField EXPIRED_TIME;
    static DBField START_TIME;
    static DBField CHECK_FLAG;
	static DBField IS_GIVEN;
	static DBField WEEKLY_GIVEN;
	static DBField IS_GIVEN_WEEKLY;
	static DBField WEEKLY_TICK;
	static DBField SUPER_VIP_TYPE;
};

struct Fight
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField PK;
    static DBField SAVE_PK;
    static DBField CAMP_ID;
    static DBField LEVEL;
    static DBField EXPERIENCE;
    static DBField PROP_POINT;
    static DBField BLOOD_BASIC;
    static DBField ATTACK_LOWER_BASIC;
    static DBField ATTACK_UPPER_BASIC;
    static DBField DEFENCE_LOWER_BASIC;
    static DBField DEFENCE_UPPER_BASIC;
    static DBField HIT_BASIC;
    static DBField AVOID_BASIC;
    static DBField CRIT_BASIC;
    static DBField TOUGHNESS_BASIC;
    static DBField MAGIC_BASIC;
    static DBField LUCKY_BASIC;

    static DBField BLOOD;
    static DBField MAGIC;
    static DBField ANGRY;
    static DBField GLAMOUR;
    static DBField JUMP;

    static DBField PK_TICK;
};

struct DBMapTiny
{
    static DBField COLLECTION;
    static DBField ID;

    static DBField CUR_BLOOD;
    static DBField NON_TIPS;
    static DBField CLIENT_GUIDE;
    static DBField EVERYDAY_TICK;
    static DBField LAST_RECHARGE_TICK;
    static DBField DAILY_TOTAL_RECHARGE;
};

struct Package
{
    static DBField COLLECTION;
    static DBField ID;

    static DBField RECHARGE_FIRST_TICK;
    static DBField RECHARGE_GOLD;
    static DBField GAME_RESOURCE;
    static DBField USER_GAME_RESOURCE;


    static DBField MONEY;
    struct Money
    {
        static DBField GOLD;
        static DBField COPPER;
        static DBField BIND_GOLD;
        static DBField BIND_COPPER;
    };

    static DBField PACK;
    struct Pack
    {
        static DBField PACK_TYPE;
        static DBField PACK_SIZE;
        static DBField PACK_ITEM;
        static DBField PACK_EQUIP;
    	static DBField LAST_TICK;
    	static DBField STRENGTHEN;
    	static DBField SUBLIME_LEVEL;
    	static DBField IS_OPEN_SUBLIME;
    };

    struct PackItem
    {
        static DBField INDEX;
        static DBField ID;
        static DBField AMOUNT;
        static DBField BIND;
        static DBField USE_TICK;
        static DBField USE_TIMES;
        static DBField TIME_OUT;
        static DBField NEW_TAG;
        static DBField UNIQUE_ID;
        static DBField TIPS_LEVEL;
        static DBField TIPS_TIME_MAP;
        static DBField TIPS_STATUS_MAP;
        static DBField OUT_TIME_ITEM_ID;
        static DBField OUT_TIME_ITEM_AMOUNT;
        static DBField OUT_TIME_ITEM_BIND;

    };
    struct PackEquip
    {
    	static DBField REFINE_LEVEL;
    	static DBField REFINE_DEGREE;
    	static DBField BRIGHT_FLAG;
    	static DBField FASHION;
    	static DBField LUCK_VALUE;
    	static DBField JEWEL_SETS;	//一旦更新好了之后就全部使用这个字段
    	static DBField REFINE_SETS;
    	static DBField BASE_POLISH_ATTR;
    	static DBField EXTRAS_ATTR;
    	static DBField SPECIAL_JEWELS;	//一旦更新好了之后就全部使用这个字段
    	static DBField MOLDING_ATTACK_LEVEL;
    	static DBField MOLDING_DEFENCE_LEVEL;
    	static DBField MOLDING_HEALTH_LEVEL;
    	static DBField MOLDING_ALL_LEVEL;
    	static DBField MOLDING_ATTACK_SCHEDULE;
    	static DBField MOLDING_DEFENCE_SCHEDULE;
    	static DBField MOLDING_HEALTH_SCHEDULE;
    };

    struct Fashion
    {
    	static DBField USE_TYPE;
    	static DBField USE_TICK;
    	static DBField EXPIRE_TICK;
    	static DBField NOTIFED_MAP;
    	static DBField IS_IN_USE;
    };

    struct PolishAttrDetial		//对应PubStruct.h中的struct EquipPolishAttrDetial	结构体
    {
    	static DBField ATTR_DETIAL_LOCK_INDEX;
    	static DBField ATTR_DETIAL_ATTR_TYPE;
    	static DBField ATTR_DETIAL_CUR_VALUE;
    	static DBField ATTR_DETIAL_COLOR;
    	static DBField ATTR_DETIAL_MAX_VALUE;
    };
    struct BasePolishAttr	//对应PubStruct.h中的struct EquipPolish	结构体
    {
    	static DBField PROCESS_VALUE;
    	static DBField CUR_POLISH_INFO;
    	static DBField SINGLE_POLISH_INFO;
    	static DBField BATCH_POLISH_INFO;
    };
    struct JewelMapInfo		//宝石信息，id、绑定状态
    {
    	static DBField ID;
    	static DBField BIND_STATUS;
    };
};

struct Escort
{
    static DBField COLLECTION;
    static DBField ID;

    static DBField CAR_INDEX;
    static DBField PROTECT_ID;
    static DBField ESCORT_TYPE;
    static DBField ESCORT_TIMES;
    static DBField TOTAL_EXP;
    static DBField START_TICK;
    static DBField TILL;
    static DBField TARGET_LEVEL;
    static DBField PROTECT_LIST;
	struct Protect_list
	{
		static DBField PROTECT_PLAYER;
	};
};

struct Skill
{
    static DBField COLLECTION;
    static DBField ID;

    static DBField SKILL;
    struct SSkill
    {
        static DBField SKILL_ID;
        static DBField LEVEL;
        static DBField USED_TIMES;

        static DBField USETICK;
        struct Tick
        {
            static DBField SEC;
            static DBField USEC;
        };
    };

    static DBField CUR_SCHEME;
    static DBField SCHEME_LIST;

    static DBField CUR_RAMA;
    static DBField RAMA_LIST;
};

struct Status
{
    static DBField COLLECTION;
    static DBField ID;

    static DBField STATUS;
    struct SStatus
    {
        static DBField STATUS_TYPE;
        static DBField VALUE1;
        static DBField VALUE2;
        static DBField VALUE3;
        static DBField VALUE4;
        static DBField VALUE5;

        static DBField VIEW_TYPE;
        static DBField VIEW1;
        static DBField VIEW2;
        static DBField VIEW3;

        static DBField SKILL_ID;
        static DBField SKILL_LEVEL;
        static DBField ATTACK_ID;
        static DBField ACCUMULATE;

        static DBField CHECKTICK;
        static DBField INTERVAL;
        static DBField LASTTICK;
        struct Tick
        {
            static DBField SEC;
            static DBField USEC;
        };
    };
};

struct DBTrade
{
    static DBField COLLECTION;

    static DBField ID;
    static DBField GOODS;
    static DBField DBSTATE;
};

struct DBShopItem
{
	static DBField COLLECTION;

	static DBField SHOP_TYPE;
	static DBField CONTENT;
	static DBField ITEM_ID;
	static DBField ITEM_POS;
	static DBField ITEM_BIND;
	static DBField ITEM_TYPE;
	static DBField MAX_ITEM;
	static DBField MAX_TOTAL;

	static DBField MONEY_TYPE;
	static DBField SRC_PRICE;
	static DBField CUR_PRICE;
	static DBField VIP_PRICE;

	static DBField START_TICK;
	static DBField END_TICK;
};

struct DBBaseRole
{
	static DBField INDEX;
	static DBField VIP;
	static DBField SEX;
	static DBField NAME;
	static DBField LVL;
	static DBField FORCE;
	static DBField CAREER;
};

struct DBLeague
{
	static DBField COLLECTION;

	static DBField ID;
	static DBField LEAGUE_NAME;
	static DBField LEAGUE_INTRO;
	static DBField LEAGUE_LVL;
	static DBField LEAGUE_RESOURCE;
	static DBField REGION_RANK;
	static DBField REGION_TICK;
	static DBField REGION_LEADER_REWARD;

	static DBField CREATE_TICK;
	static DBField LAST_LOGIN;
	static DBField LEADER_INDEX;
	static DBField AUTO_ACCEPT;
	static DBField ACCEPT_FORCE;
	static DBField FLAG_LVL;
	static DBField FLAG_EXP;
	static DBField LEAGUE_MEMBER;
	static DBField LEAGUE_APPLIER;
	static DBField LEAGUE_LOG;

	struct Member
	{
		static DBField ROLE_INDEX;
		static DBField JOIN_TICK;
		static DBField LEAGUE_POS;

		static DBField CUR_CONTRI;
		static DBField TODAY_CONTRI;
		static DBField TOTAL_CONTRI;
		static DBField OFFLINE_CONTRI;

		static DBField TODAY_RANK;
		static DBField TOTAL_RANK;
		static DBField LRF_BET_LEAGUE_ID;
	};

	struct Log
	{
		static DBField LOG_TICK;
		static DBField LOG_CONTENT;
	};

	static DBField LEAUGE_FB;
	struct LeagueFB
	{
		static DBField OPEN_TICK;
		static DBField OPEN_MODE;
		static DBField OPEN_STATE;
		static DBField FINISH_STATE;
	};

	static DBField LEAGUE_BOSS;
	struct LeagueBoss
	{
		static DBField BOSS_INDEX;
		static DBField BOSS_EXP;
		static DBField SUPER_SUMMON_ROLE;
		static DBField RESET_TICK;
		static DBField NORMAL_SUMMON_TICK;
		static DBField SUPER_SUMMON_TICK;
		static DBField NORMAL_SUMMON_TYPE;
		static DBField SUPER_SUMMON_TYPE;
		static DBField NORMAL_DIE_TICK;
		static DBField SUPER_DIE_TICK;
	};

	static DBField LEAGUE_IMPEACH;
	struct LeagueImpeach
	{
		static DBField IMPEACH_ROLE;
		static DBField IMPEACH_TICK;
	};
	static DBField VOTE_MAP;

	static DBField LFB_PLAYER_SET;
	struct LFbPlayerSet
	{
		static DBField PLAYER_ID;
		static DBField TICK;
		static DBField NAME;
		static DBField SEX;
		static DBField WAVE;
		static DBField LAST_WAVE;
		static DBField CHEER;
		static DBField ENCOURAGE;
		static DBField BE_CHEER;
		static DBField BE_ENCOURAGE;

		static DBField RECORD_VEC;
		struct RecordSet
		{
			static DBField ROLE_ID;
			static DBField TYPE;
			static DBField IS_ACTIVE;
			static DBField TIME;
			static DBField NAME;
		};
	};
};

struct DBLeaguer
{
	static DBField COLLECTION;

	static DBField ID;
	static DBField SHOP_BUY;
	static DBField LEAGUE_SKILL;
	static DBField OPEN;
	static DBField DRAW_WELFARE;
	static DBField WAND_DONATE;
	static DBField GOLD_DONATE;
	static DBField SEND_FLAG;

	static DBField SIEGE_SHOP;
	static DBField SIEGE_SHOP_REFRESH;
	static DBField DAY_ADMIRE_TIMES;

	static DBField LEAVE_TYPE;
	static DBField LEAVE_TICK;
	static DBField CUR_CONTRI;
	static DBField SALARY_FLAG;
	static DBField FB_FLAG;
	static DBField STORE_TIMES;
	static DBField APPLY_LIST;
	static DBField WAVE_REWARD_MAP;
	static DBField REGION_DRAW;

	static DBField LV_REFRESH_TICK;
	static DBField LV_TASK_SET;
	static DBField TID;
	static DBField MID;
	static DBField NEED;
	static DBField FINISH;
};

struct DBLeagueWarInfo
{
	static DBField COLLECTION;

	static DBField ID;
	static DBField TOTAL_NUM;
	static DBField LAST_TICK;

	static DBField RANK_SET;
	struct LeagueWarRank
	{
		static DBField ID;
		static DBField LEAGUE_INDEX;
		static DBField LEAGUE_NAME;
		static DBField LEADER;
		static DBField FORCE;
		static DBField SCORE;
		static DBField FLAG_LVL;
	};
};

struct DBLWTicker
{
	static DBField COLLECTION;
	static DBField ID;

	//江湖榜重排序 id == 1
	struct Arena
	{
		static DBField TIMEOUT_TICK;
		static DBField RE_RANK;
	};

	//商店限制 id == 2
	struct Shop
	{
		static DBField LIMITED_SET;
		static DBField LIMITED_TOTAL_SET;
	};

	//合服第一次执行 id == 3
	struct CombineFirst
	{
		static DBField COMB_FIRST;
	};
};

struct DBArenaRole
{
	static DBField COLLECTION;

	static DBField ID;
	static DBField NAME;
	static DBField SEX;
	static DBField CAREER;
	static DBField FORCE;
	static DBField LEVEL;
	static DBField REWARD_LEVEL;
	static DBField WING_LEVEL;
	static DBField SOLIDER_LEVEL;

	static DBField REFRESH_TICK;
	static DBField LEFT_TIMES;
	static DBField BUY_TIMES;

	static DBField RANK;
	static DBField IS_SKIP;
	static DBField IS_OVER_LIMIT;
	static DBField OPEN_FLAG;
	static DBField FIGHT_SET;

	static DBField LAST_RANK;
	static DBField ADD_ANIMA;
	static DBField ADD_MONEY;
	static DBField CONTINUE_WIN;

	static DBField HIS_RECORD;
	static DBField FIGHT_TICK;
	static DBField FIGHT_TYPE;
	static DBField FIGHT_STATE;
	static DBField FIGHT_NAME;
	static DBField FIGHT_RANK;
	static DBField RANK_CHANGE;
};

struct DBChatRecord
{
	 static DBField SRC_ROLE_ID;
	 static DBField DST_ROLE_ID;
	 static DBField TIME;
     static DBField TYPE;
     static DBField VOICE_ID;
     static DBField VOICE_LEN;
	 static DBField CONTENT;
};


struct DBChatLeague
{
	 static DBField COLLECTION;
	 static DBField LEAGUE_ID;
	 static DBField CHAT_LIST;
};


struct DBOnline
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField IS_ONLINE;
    static DBField SIGN_IN_TICK;
    static DBField SIGN_OUT_TICK;
    static DBField TOTAL_ONLINE;
    static DBField DAY_ONLINE;
    static DBField WEEK_ONLINE;
    static DBField MONTH_ONLINE;
    static DBField YEAR_ONLINE;

    static DBField DAY_REFRESH;
    static DBField WEEK_REFRESH;
    static DBField MONTH_REFRESH;
    static DBField YEAR_REFRESH;
};

struct SocialerInfo
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField OPEN;

	static DBField APPLY_LIST;
	struct ApplyInfo
	{
		static DBField FRIEND_ID;
		static DBField LEAGUE_ID;
		static DBField FRIEND_NAME;
		static DBField LEAGUE_NAME;
		static DBField LEVEL;
		static DBField SEX;
		static DBField TICK;
	};

	static DBField FRIEND_LIST;
	static DBField STRANGER_LIST;
	static DBField BLACK_LIST;
	static DBField NEARBY_LIST;
	static DBField ENEMY_LIST;
	static DBField TICK;
	static DBField STRENGTH_LIST;
	static DBField STRENGTH_GIVE_LIST;
	static DBField STRENGTH;
	static DBField GIVE_TIMES;
	static DBField GET_TIMES;
	static DBField VIP_BUY;

	static DBField FRIEND_VALUE;
	static DBField FRIENDSHIP_ID;
	static DBField REMOVE_LIST;
	static DBField RECENT_CHAT_LIST;
	static DBField FRIEND_VALUE_LIST;
	static DBField FRIEND_SHIP_LIST;
};

struct DBFriendPair
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField PAIR_SET;
	struct PairSet
	{
		static DBField NUMBER;
		static DBField PLAYER_ONE;
		static DBField PLAYER_TWO;
	};
};

struct MailInfo
{
	static DBField COLLECTION;
    static DBField ID;
	static DBField SEND_MAIL_COUNT;
	static DBField SEND_MAIL_COOL_TICK;
	static DBField COUNT;
    static DBField INFO;
    struct Info
    {
    	static DBField MAIL_ID;
    	static DBField TYPE;
    	static DBField FORMAT;
        static DBField HAS_READ;
        static DBField TIME;
        static DBField READ_TICK;
        static DBField SENDER_NAME;
        static DBField TITLE;
        static DBField CONTENT;
        static DBField GOODS;
        static DBField SENDER_ID;
        static DBField LABEL;
        static DBField EXPLOIT;
        static DBField SENDER_VIP;
        static DBField ST_SCORE;
        static DBField RESOURCE;
    };
};

struct MailOffline
{
	static DBField COLLECTION;
	static DBField MAIL_ID;
	static DBField ROLE_ID; // target id
	static DBField SENDER_ID;	  // sender id
	static DBField FLAG;
	static DBField TYPE;
	static DBField FORMAT;
	static DBField TIME;
	static DBField SENDER_NAME;
	static DBField TITLE;
	static DBField CONTENT;
	static DBField GOODS;
    static DBField LABEL;
    static DBField EXPLOIT;
    static DBField SENDER_VIP;
    static DBField ST_SCORE;
};

struct DBMarket
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField LAST_GOLD;
	static DBField LAST_COPPER;
};

struct DBMarketItem
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField ROLE_ID;

	static DBField ON_TICK;
	static DBField OFF_TICK;

	static DBField ITEM_OBJ;
	static DBField ITEM_ID;
	static DBField ITEM_AMOUNT;

	static DBField MONEY_TYPE;
	static DBField PRICE;
};

struct DBBeastSkill
{
	static DBField INDEX;
	static DBField ID;
	static DBField LOCK;
	static DBField EXP;
	static DBField LEVEL;
	static DBField TRANSFORM;
};

struct DBMaster
{
	static DBField COLLECTION;

	static DBField ID;
	static DBField MASTER_NAME;
	static DBField SKILL_SET;
	static DBField BEAST_SET;
	static DBField PACK_SIZE;
	static DBField CUR_BESAT;
	static DBField CUR_BEAST_SORT;
	static DBField SAVE_BESAT;
	static DBField LAST_BEAST;
	static DBField LAST_BEAST_SORT;
	static DBField GEN_SKILL_LUCKY;
	static DBField GEN_SKILL_BOOK;
	static DBField IS_OPEN_SKILL;

	static DBField MOUNT_SET;
	static DBField MOUNT_GRADE;
	static DBField OPEN;
	static DBField BLESS;
	static DBField FAIL_TIMES;
	static DBField FINISH_BLESS;
	static DBField ON_MOUNT;
	static DBField MOUNT_SHAPE;
	static DBField ABILITY;
	static DBField GROWTH;
	static DBField ACT_SHAPE;
	static DBField SPOOL_LEVEL;
	static DBField SKILL;
};


struct FriendshipValue
{
	static DBField COLLECTION;
	static DBField FRIEND_ID;
	static DBField FriendValueDetail;

	struct FriendValueDetail
	{
		static DBField FIRST_ID;
		static DBField SECOND_ID;
		static DBField FRIEND_VALUE;
	};
};

struct DBTask
{
	static DBField COLLECTION;
	static DBField ID;
    static DBField SUBMITED_TASK;
    static DBField NOVICE_STEP;
    static DBField LATEST_MAIN_TASK;
    static DBField UIOPEN_STEP;

    static DBField ROUTINE_REFRESH_TICK;
    static DBField ROUTINE_TASK_INDEX;
    static DBField ROUTINE_TOTAL_NUM;
    static DBField ROUTINE_CONSUME_HISTORY;
    static DBField IS_FINISH_ALL_ROUTINE;
    static DBField IS_ROUTINE_TASK;
    static DBField IS_SECOND_ROUTINE;

    static DBField OFFER_ROUTINE_TASK_INDEX;
    static DBField IS_FINISH_ALL_OFFER_ROUTINE;
    static DBField OFFER_ROUTINE_TOTAL_NUM;
    static DBField IS_OFFER_ROUTINE_TASK;
    static DBField IS_SECOND_OFFER_ROUTINE;

    static DBField LEAGUE_ROUTINE_REFRESH_TICK;
    static DBField LEAGUE_ROUTINE_TASK_INDEX;
    static DBField LEAGUE_ROUTINE_TOTAL_NUM;
    static DBField IS_FINISH_ALL_LEAGUE_ROUTINE;
    static DBField IS_LEAGUE_ROUTINE_TASK;
    static DBField IS_SECOND_LEAGUE_ROUTINE;

    static DBField LCONTRI_DAY_TICK;
    static DBField LCONTRI_DAY;
    static DBField OPEN_UI;
    static DBField UI_VERSION;
    static DBField TRIAL_FRESH_TICK;
    static DBField USED_TRIAL_TIMES;
    static DBField TRIAL_TASK_SET;
    static DBField FINISH_TASK;
    static DBField TASK;
	struct Task
	{
		static DBField TASK_ID;
        static DBField GAME_TYPE;
        static DBField ACCEPT_TICK_SEC;
        static DBField ACCEPT_TICK_USEC;
        static DBField REFRESH_TICK_SEC;
        static DBField REFRESH_TICK_USEC;
        static DBField PREV_TASK;
        static DBField POST_TASK;
        static DBField STATUS;
        static DBField LOGIC_TYPE;
        static DBField TASK_STAR;
        static DBField FAST_FINISH_RATE;
        static DBField FRESH_STAR_TIMES;

        static DBField COND_LIST;
        struct CondList
        {
            static DBField TYPE;
            static DBField CURRENT_VALUE;
            static DBField COND_INDEX;
            static DBField ID_LIST_INDEX;
            static DBField COND_ID;
            static DBField FINAL_VALUE;
            static DBField KILL_TYPE;
            static DBField RANGE_LEVEL;
        };
	};
};


struct MallActivityInfo
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField OPEN;
	static DBField REFRESH_TYPE;
	static DBField REFRESH_TICK;
	static DBField LIMIT_TYPE;
	static DBField NAME;
	static DBField MEMO;
	static DBField TOTAL_LIMIT_AMOUNT;
	static DBField SINGLE_LIMIT_AMOUNT;
	static DBField PLAYER_RECORD;
	static DBField TOTAL_BUY_RECORD;
	static DBField SINGLE_BUY_RECORD;
	static DBField GOODS_ID;
	static DBField GOODS_AMOUNT;
	static DBField IS_CONFIG;
	static DBField VISIBLE_START_TICK;
	static DBField VISIBLE_END_TICK;
	static DBField ACTIVITY_START_TICK;
	static DBField ACTIVITY_END_TICK;
};

struct LabelInfo
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField LABEL_ID;
	static DBField PRE_LABEL_ID;

	static DBField WAR_LABEL;
	static DBField WAR_LABEL_TICK;
	static DBField MATRIAL_LABEL;
	static DBField MARTIAL_LABEL_TICK;

	static DBField EXPIRE_TICK;
	static DBField PERMANT_LIST;
	static DBField NEW_LIST;
	static DBField LIMIT_TIME_LIST;
	static DBField EXPIRE_UNSHOWN_LIST;
};

struct DBActivityTipsInfo
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField NOTIFY_REC;

	struct NotifyTipsRec
	{
		static DBField ACTIVITY_ID;
        static DBField START_TICK;
        static DBField END_TICK;
        static DBField REFRESH_TICK;
		static DBField FINISH_COUNT;
		static DBField IS_TOUCH;
	};
};

struct DBScript
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField SCRIPT_ID;
    static DBField SCRIPT_SORT;
    static DBField PREV_SCENE;
    static DBField PREV_PIXEL_X;
    static DBField PREV_PIXEL_Y;
    static DBField PREV_BLOOD;
    static DBField PREV_MAGIC;
    static DBField TRVL_TOTAL_PASS;
    static DBField FIRST_SCRIPT;
    static DBField SKILL_ID;

    static DBField TYPE_RECORD;
    struct TypeRecord
    {
    	static DBField SCRIPT_TYPE;
    	static DBField MAX_SCRIPT_SORT;
    	static DBField PASS_WAVE;
    	static DBField PASS_CHAPTER;
    	static DBField START_WAVE;
    	static DBField START_CHAPTER;
    	static DBField NOTIFY_CHAPTER;
    	static DBField IS_SWEEP;
    	static DBField USED_TIMES_TICK_SEC;
    	static DBField USED_TIMES_TICK_USEC;
    };

    static DBField SCRIPT_WAVE_RECORD;
    struct ScriptWaveInfo
    {
    	static DBField SCRIPT_WAVE_ID;
    	static DBField IS_GET;
    };

    static DBField RECORD;
    struct Record
    {
        static DBField SCRIPT_SORT;
        static DBField USED_TIMES;
        static DBField BUY_TIMES;
        static DBField COUPLE_BUY_TIME;
        static DBField USED_TIMES_TICK_SEC;
        static DBField USED_TIMES_TICK_USEC;
        static DBField ENTER_SCRIPT_TICK;
        static DBField PROGRESS_ID;
        static DBField BEST_USE_TICK;
        static DBField IS_FIRST_PASS;
        static DBField DAY_PASS_TIMES;
        static DBField IS_EVEN_ENTER;
        static DBField PROTECT_BEAST_INDEX;
    };

    static DBField PIECE_RECORD;
    struct PieceRecord
    {
        static DBField PASS_PIECE;
        static DBField PASS_CHAPTER;
        static DBField PIECE_STAR_AWARD;

        static DBField PIECE_CHAPTER;
        struct PieceChapter
        {
            static DBField CHAPTER_KEY;
            static DBField USED_SEC;
            static DBField USED_TIMES;
            static DBField TODAY_PASS_FLAG;
            static DBField AWARD_FLAG;
            static DBField CHAPTER_ITEM;
        };
    };

    static DBField LEGEND_TOP_INFO;
    static DBField SWORD_TOP_INFO;
    struct LegendTopInfo
    {
    	static DBField PASS_FLOOR;
    	static DBField TODAY_RANK;
    	static DBField IS_SWEEP;

    	static DBField FLOOR_INFO;
    	struct FloorInfo
    	{
    		static DBField FLOOR_ID;
    		static DBField PASS_TICK;
    		static DBField TODAY_PASS_FLAG;
		};
    };
};

struct DBSwordPool
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField LEVEL;
	static DBField EXP;
	static DBField OPEN;
	static DBField STYLE_LV;
	static DBField REFRESH_TICK;

	static DBField LAST_POOL_TASK_INFO;
	static DBField TODAY_POOL_TASK_INFO;
	struct PoolTaskInfo
	{
		static DBField TASK_ID;
		static DBField TOTAL_NUM;
		static DBField LEFT_NUM;
	};
};

struct DBHiddenTreasure
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField DAY;
	static DBField OPEN;
	static DBField GET_STATUS;
	static DBField REFRESH_TICK;
	static DBField BUY_MAP;
};

struct DBFashion
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField LEVEL;
	static DBField EXP;
	static DBField OPEN;
	static DBField SELECT_ID;
	static DBField SEL_COLOR_ID;
	static DBField SEND_SET;

	static DBField FASHION_SET;
	struct FashionInfo
	{
		static DBField FASHION_ID;
		static DBField COLOR_ID;
		static DBField ACTIVE_TYPE;
		static DBField IS_PERMANENT;
		static DBField ACTIVE_TICK;
		static DBField END_TICK;
		static DBField COLOR_SET;
	};
};

struct DBTransfer
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField LEVEL;
	static DBField EXP;
	static DBField OPEN;
	static DBField STAGE;
	static DBField TRANSFER_TICK;
	static DBField LAST;
	static DBField ACTIVE_ID;
	static DBField OPEN_REWARD;
	static DBField GOLD_TIMES;
	static DBField REFRESH_TICK;

	static DBField TRANSFER_SET;
	struct TransferSet
	{
		static DBField TRANSFER_ID;
		static DBField TRANSFER_LV;
		static DBField IS_PERMANENT;
		static DBField IS_ACTIVE;
		static DBField ACTIVE_TICK;
		static DBField END_TICK;
		static DBField TRANSFER_SKILL;
		static DBField SKILL_SET;
	};
};

struct DBAchieve
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField ACH_INDEX;
	static DBField BASE_TYPE;
	static DBField CHILD_TYPE;
	static DBField COMPARE;
	static DBField SORT;
	static DBField RED_EVENT;

	static DBField DETAIL;
	struct AchieveDetail
	{
		static DBField ACHIEVE_ID;
		static DBField NEED_AMOUNT;
		static DBField SORT;
		static DBField ACHIEVE_TYPE;
		static DBField NUMBER_TYPE;
		static DBField REWARD_ID;
		static DBField ACH_AMOUNT;
	};
};

struct AchieveInfo
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField ACHIEVE_LEVEL;
	static DBField POINT_MAP;

	static DBField ACHIEVE_LIST;
	struct AchieveList
	{
		static DBField ACHIEVE_ID;
		static DBField ACH_INDEX;
		static DBField FINISH_TICK;
		static DBField GET_STATUS;
		static DBField FINISH_NUM;
		static DBField SPECIAL_VALUE;
	};
//	static DBField DRAWED_LIST;
//	static DBField UNCHECK_LIST;
//	static DBField ACHIEVE_PROP;
//	static DBField OTHER_ACHIEVE_RECORD;
//	static DBField OTHER_CUR_VALUE;
//	static DBField TYPE;
//	static DBField SUB_TYPE;
//	static DBField IS_FINISH;
//	static DBField REWARD_STATUS;
//	static DBField PROP_ID;
//	static DBField PROP_VALUE;
};

struct DBEquipSmelt
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField SMELT_LEVEL;
	static DBField SMELT_EXP;
	static DBField RECOMMEND;
	static DBField OPEN;

};

struct DBOfflineRewards
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField RECEIVED_MARK;
	static DBField LOGOUT_TIME;
	static DBField OFFLINE_SUM;
	static DBField LONGEST_TIME;
};

struct DBTreasures
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField FREE_TIMES;
	static DBField RESET_TICK;
	static DBField RESET_TIMES;
	static DBField GAME_INDEX;
	static DBField GAME_LENGTH;

	static DBField ITEM_LIST;
	struct Item_list
	{
		static DBField ID;
		static DBField AMOUNT;
		static DBField BIND;
		static DBField INDEX;
	};
};

struct DBOnlineRewards
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField STAGE;
	static DBField PRE_STAGE;
	static DBField LOGIN_TIME;
	static DBField ONLINE_SUM;
	static DBField RECEIVED_MARK;
	static DBField AWARD_LIST;
	struct Award_List
	{
		static DBField AWARD_NUM;
	};

};

struct DBCollectChests
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField COLLECT_NUM;
	static DBField NEXT_TICK;
};

struct DBIllus
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField OPEN;
	static DBField ILLUS_GROUP;
	struct Illus_group
	{
		static DBField GROUP_ID;
		static DBField GROUP_TYPE;
		static DBField GROUP_NUM;
	};
	static DBField ILLUS;
	struct Illus
	{
		static DBField ILLUS_ID;
		static DBField ILLUS_LEVEL;
		static DBField ILLUS_NUM;
	};
};

struct DBWelfare
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField CHECK_IN;
	struct CheckIn
	{
		static DBField AWARD_INDEX;
		static DBField CHECK_IN_POINT;
		static DBField LAST_TIME;
		static DBField SHOW_POINT;

		static DBField CHECK_TOTAL_INDEX;
		static DBField CHARGE_MONEY;
		static DBField TOTAL_LAST_TIME;
		static DBField TOTAL_POPUP;
	};

	static DBField LIVENESS;
	struct Liveness
	{
		static DBField LIVENESS_POINT;
		static DBField RECEIVED_AWARD_INDEX;

		static DBField FINISH_RECORD;
		struct FinishRecord
		{
			static DBField ID;
			static DBField RECORDS;
			struct Record{
				static DBField NUM;
				static DBField IS_PASSED;
			};
		};

		static DBField FINISH_TIMES;
		struct FinishTimes
		{
			static DBField ID;
			static DBField TIMES;
		};
		static DBField LAST_TIME;
	};

	static DBField ONCE_REWARDS;
	struct OnceRewards
	{
		static DBField UPDATE_RES_FLAG;
	};

	static DBField DAILY_LIVENESS;
	struct DailyLiveness
	{
		static DBField RECORDS;
		struct Record
		{
			static DBField ID;
			static DBField TYPE;
			static DBField VALUE;
			static DBField REWARD_VALUE;
			static DBField IS_OVER;
		};
	};
};

struct DBExpRestore
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField CHECK_TIMESTAMP;
	static DBField VIP_TYPE_REC;
	static DBField VIP_START_TIME;
	static DBField VIP_EXPRIED_TIME;

	static DBField PRE_ACT_MAP;
	struct PreActMap
	{
		static DBField ACT_ID;
		static DBField EXT_TYPE;
		static DBField TIMES;
	};

	static DBField NOW_ACT_MAP;
	struct NowActMap
	{
		static DBField ACT_ID;
		static DBField EXT_TYPE;
		static DBField TIMES;
	};

	static DBField LEVEL_RECORD;
	struct LevelRecord
	{
		static DBField REC_TIMESTAMP;
		static DBField LEVEL;
	};

	static DBField VIP_RECORD;
	struct VipRecord
	{
		static DBField REC_TIMESTAMP;
		static DBField VIP_TYPE;

	};

	static DBField STORAGE_RECORD;
	struct ActivityExpRestore
	{
		static DBField STORAGE_ID;
		static DBField REC_TIMESTAMP;
		static DBField FINISH_COUNT;
		static DBField STORAGE_VALID;

	};

	static DBField STORAGE_STAGE_INFO;
	struct StorageStageInfo
	{
		static DBField STORAGE_ID;
		static DBField TIMESTAMP_STAGE;
		struct TimestampStage
		{
			static DBField REC_TIMESTAMP;
			static DBField STAGE;
		};
	};
};


struct DBOfflineData
{
	static DBField COLLECTION;
	static DBField ID;
};

struct DBRank
{
	static DBField COLLECTION;

	static DBField ID;
	//static DBField RANK_DETAIL;
    static DBField TOTAL_VALUE;
    static DBField JANE_ITEM;
	static DBField RANK_RECORD;
	struct RankDetail
	{
		static DBField RANK_TYPE;
		static DBField RANK_NAME;
		static DBField LAST_REFRESH_TICK;

		static DBField CUR_RANK;
		static DBField LAST_RANK;
		static DBField ACHIEVE_TICK;
		static DBField ADDITIONAL_ID;
		static DBField PLAYER_ID;
		static DBField DISPLAYER_CONTENT;
		static DBField LEAGUE_NAME;
		static DBField RANK_VALUE;
		static DBField ADDIITION_ID;
		static DBField RANK_FORCE;
		static DBField EXT_VALUE;
		static DBField VIP_TYPE;
		static DBField WORSHIP_NUM;
		static DBField IS_WORSHIP;
	};
};

struct DBRanker
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField NAME;
	static DBField EXPERIENCE;
	static DBField FIGHT_LEVEL;
	static DBField FIGHT_FORCE;
	static DBField KILL_VALUE;
	static DBField KILL_NUM;
	static DBField KILL_NORMAL;
	static DBField WEEK_TICK;
	static DBField KILL_EVIL;
	static DBField LABEL;
	static DBField WEAPON;
	static DBField CLOTHES;
	static DBField FASHION_WEAPON;
	static DBField FASHION_CLOTHES;
	static DBField VIP_STATUS;
	static DBField SEX;
	static DBField CAREER;
	static DBField LEAGUE_ID;
	static DBField LEAGUE_NAME;
	static DBField VIP_TYPE;
	static DBField WORSHIP_NUM;
	static DBField IS_WORSHIP;

	static DBField ATTACK;
	static DBField DEFENCE;
	static DBField MAX_BLOOD;
	static DBField ATTACK_UPPER;
	static DBField ATTACK_LOWER;
	static DBField DEFENCE_UPPER;
	static DBField DEFENCE_LOWER;
	static DBField CRIT;
	static DBField TOUGHNESS;
	static DBField HIT;
	static DBField DODGE;
	static DBField LUCKY;
	static DBField CRIT_HIT;
	static DBField DAMAGE;
	static DBField REDUCTION;
	static DBField GLAMOUR;

	static DBField FUN_MOUNT_GRADE;
	static DBField FUN_GOD_SOLIDER_GRADE;
	static DBField FUN_MAGIC_EQUIP_GRADE;
	static DBField FUN_XIAN_WING_GRADE;
	static DBField FUN_LING_BEAST_GRADE;
	static DBField FUN_BEAST_EQUIP_GRADE;
	static DBField FUN_BEAST_MOUNT_GRADE;
	static DBField FUN_BEAST_WING_GRADE;
	static DBField FUN_BEAST_MAO_GRADE;
	static DBField FUN_TIAN_GANG_GRADE;

	static DBField FUN_MOUNT_FORCE;
	static DBField FUN_GOD_SOLIDER_FORCE;
	static DBField FUN_MAGIC_EQUIP_FORCE;
	static DBField FUN_XIAN_WING_FORCE;
	static DBField FUN_LING_BEAST_FORCE;
	static DBField FUN_BEAST_EQUIP_FORCE;
	static DBField FUN_BEAST_MOUNT_FORCE;
	static DBField FUN_BEAST_WING_FORCE;
	static DBField FUN_BEAST_MAO_FORCE;
	static DBField FUN_TIAN_GANG_FORCE;

	static DBField MOUNT_SET;
	static DBField OFF_MOUNT_TYPE;
	static DBField OFF_MOUNT_OPEN;
	static DBField OFF_MOUNT_GRADE;
	static DBField OFF_MOUNT_SHAPE;
	static DBField OFF_ACT_SHAPE;
	static DBField OFF_MOUNT_SKILL;
	static DBField OFF_MOUNT_FORCE;
	static DBField OFF_MOUNT_PROP;
	static DBField OFF_MOUNT_TEMP;

	static DBField MOUNT_SORT;
	static DBField MOUNT_GRADE;
	static DBField MOUNT_SHAPE;
	static DBField IS_ON_MOUNT;

	static DBField ZYFM_PASS_KEY;
	static DBField ZYFM_PASS_TICK;

    static DBField WING_LEVEL;

    static DBField SEND_FLOWER;
    static DBField RECV_FLOWER;
    static DBField ACT_SEND_FLOWER;
    static DBField ACT_RECV_FLOWER;
    static DBField OP_TYPE;

	static DBField SKILL_LIST;
	struct Skill_list
	{
		static DBField skill_id;
		static DBField skill_level;
	};

	static DBField PET_DETAIL;
	static DBField PET;
	struct Pet
	{
		static DBField PET_IS_REMOVE;
		static DBField PET_INDEX;
		static DBField PET_SORT;
		static DBField PET_NAME;
		static DBField PET_FORCE;
		static DBField PET_CUR_SORT;
		static DBField PET_LEVEL;
	};

	static DBField EQUIP_LIST;
	static DBField EQUIP_SET;
	static DBField FAIRY_ACT;
	struct Fairy_act
	{
		static DBField ID;
		static DBField SELECT_ICON;
	};
};

struct DBWorldBossNew
{
	static DBField COLLECTION ;
	static DBField KEY;   	// scene_id
	static DBField SCENE_ID;
	static DBField STATUS;
	static DBField DIE_TICK;
};

struct DBTrvlWboss
{
	static DBField COLLECTION ;
	static DBField KEY;   	// scene_id
	static DBField STATUS;
	static DBField KILLER;
	static DBField KILLER_NAME;
};

struct DBMAttackLabelRecord
{
	static DBField COLLECTION;
	static DBField KEY;		//label_id
	static DBField ROLE_ID;
	static DBField ROLE_NAME;
	static DBField ROLE_SEX;
};

struct DBScriptProgress
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField SCRIPT_SORT;
    static DBField IS_FINISH;
    static DBField OWNER;
    static DBField SAVE_TICK;
};

struct DBScriptClean
{
    static DBField COLLECTION;
    static DBField ID;

    static DBField CURRENT_SCRIPT;
    static DBField FINISH_SCRIPT;
    struct ScriptInfo
    {
        static DBField SCRIPT_SORT;
        static DBField CHAPTER_KEY;
        static DBField USE_TIMES;
        static DBField USE_TICK;
    };

    static DBField EXP;
    static DBField SAVVY;
    static DBField MONEY_COPPER;
    static DBField MONEY_BIND_COPPER;
    static DBField MONEY_GOLD;
    static DBField MONEY_BIND_GOLD;
    static DBField STATE;
    static DBField BEGIN_TICK;
    static DBField END_TICK;
    static DBField TERMINATE_TICK;
    static DBField NEXT_CHECK_TICK;

    static DBField SCRIPT_COMPACT;
    struct ScriptCompact
    {
        static DBField COMPACT_TYPE;
        static DBField START_TICK;
        static DBField EXPIRED_TICK;
        static DBField SYS_NOTIFY;
    };
};

struct DBPlayerTip
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField TIP_DETAIL;
	struct TipDetail
	{
		static DBField EVENT_ID;
		static DBField FASHION_ID;
		static DBField FASHION_FORCE;
		static DBField FASHION_LEFT_SEC;
		static DBField MARTIAL_ID;
		static DBField VIP_TYPE;
		static DBField VIP_LEFT_SEC;
		static DBField TIPS_FLAG;
	};

};

struct DBScriptHistory
{
    static DBField COLLECTION;
    static DBField SCRIPT_SORT;

    static DBField CHAPTER_REC;
    struct ChapterRec
    {
        static DBField FIRST_ID;
        static DBField FIRST_NAME;
        static DBField CHAPTER_KEY;
        static DBField BEST_USE_TICK;
    }; 
};

struct DBLegendTopPlayer
{
	static DBField COLLECTION;
	static DBField COLLECTION2;
	static DBField ID;
	static DBField REFRESH_TICK;

	static DBField PLAYER_SET;
	struct PlayerSet
	{
		static DBField PLAYER_ID;
		static DBField NAME;
		static DBField FIGHT_SCORE;
		static DBField FLOOR;
		static DBField RANK;
		static DBField TICK;
	};
};

struct DBCouplePlayer
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField ROLE_MAP;
};

struct DBSysSetting
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField ACCOUNT;
	static DBField SET_DETAIL;

	struct SetDetail
	{
		static DBField IS_SHOCK;
		static DBField SCREEN_TYPE;
		static DBField FLUENCY_TYPE;
		static DBField SHIELD_TYPE;
		static DBField TURNOFF_ACT_NOTIFY;
		static DBField AUTO_ADJUST_EXPRESS;

		static DBField MUSIC_EFFECT;
		struct MusicEffect
		{
			static DBField MUSIC_ON;
			static DBField MUSIC_VOLUME;
		};

		static DBField SOUND_EFFECT;
		struct SoundEffect
		{
			static DBField SOUND_ON;
			static DBField SOUND_VOLUME;
		};
	};
};

struct DBCustomerSVCDetail
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField LAST_SUMMIT_TYPE;
	static DBField CONTENT;
	static DBField TITLE;

	static DBField OPINION_REWARD;
	struct RewardInfo
	{
		static DBField OPINION_INDEX;
		static DBField REWARD_STATUS;
	};
};

struct DBMediaGiftPlayer
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField USE_TIMES_LIST;
	static DBField USE_TAGS_LIST;
	static DBField USE_TICK_LIST;
};

struct DBRpmFakeInfo
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField NAME;
	static DBField FORCE;
	static DBField LEVEL;
	static DBField SEX;
	static DBField CAREER;
	static DBField VIP_TYPE;
	static DBField CONFIG_ID;
	static DBField WING_LEVEL;
	static DBField SOLIDER_LEVEL;
};

struct DBOpenActivity
{
	static DBField COLLECTION;
	static DBField ID;

    static DBField OPEN_ACT;
    struct Act
    {
    	static DBField SUB_VALUE;
    	static DBField UPDATE_TICK;
    	static DBField DRAWED_SET;
    	static DBField ID;
    	static DBField ARRIVE;
    	static DBField ARRIVE_MAP;
    	static DBField DRAWED;
    	static DBField SECOND_SUB;
    	static DBField START_TICK;
    	static DBField STOP_TICK;
    };

    static DBField ACCU_MAP;
    static DBField COMBINE_TICK;
    struct ReLogin
    {
    	static DBField SINGLE;
    	static DBField TEN;
    	static DBField HUNDRED;
    	static DBField MULTIPLE;

    	static DBField SINGLE_STATE;
    	static DBField TEN_STATE;
    	static DBField HUNDRED_STATE;
    	static DBField MULTIPLE_STATE;

    	static DBField CUMULATIVE_DAY;
    };

	static const std::string CORNUCOPIA_TASK;
	static const std::string CORNUCOPIA_STAGE;

	static const std::string RED_PACKET_GROUP;

	static const std::string ACT_DATA_RESET_FLAG;
	static const std::string LAST_ACT_TIME;
	static const std::string LAST_ACT_TYPE;

	struct CornucopiaStage
	{
		static const std::string STAGEID;
		static const std::string FLAG;
	};
	struct CornucopiaTask
	{
		static const std::string TASKID;
		static const std::string COMPLETION_TIMES;
		static const std::string TOTAL_TIMES;
	};

};

struct DBLuckyWheelSys
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField ACT_TYPE;
	static DBField SAVE_GOLD;
	static DBField DATE_TYPE;
	static DBField FIRST_DATE;
	static DBField LAST_DATE;
	static DBField IS_COMBINE_RESET;
	static DBField RESET_TICK;
	static DBField SAVE_DAY;
	static DBField REFRESH_TIMES;
	static DBField ALL_FINISH_TIMES;

	static DBField ITEM_SET;
	struct ServerItemSet
	{
		static DBField PLAYER_ID;
		static DBField PLAYER_NAME;
		static DBField GET_TIME;
		static DBField ITEM_BIND;
		static DBField AMOUNT;
		static DBField ITEM_ID;
		static DBField REWARD_MULT;
		static DBField SUB_VALUE;
	};

	static DBField RANK_NUM_SET;
	static DBField RANK_LUCKY_SET;
	struct OneRankSet
	{
		static DBField RANK;
		static DBField TICK;
		static DBField NUM;
		static DBField ROLE_ID;
		static DBField NAME;
	};

	static DBField ROLE_MAIL_SET;
	struct RoleMailSet
	{
		static DBField ROLE_ID;
		static DBField REWARD_SET;
	};

	static DBField GEM_ROLE_MAIL_SET;
	struct GemRoleMailSet
	{
		static DBField ROLE_ID;
		static DBField REWARD_SET;
	};

	static DBField BLESS_REWARD_SET;

	static DBField SLOT_SET;
	struct SlotSet
	{
		static DBField TIME_POINT;
		static DBField SLOT_ID;
		static DBField SERVER_BUY;
	};
};

struct DBLuckyWheel
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField ACTIVITY_DETAIL;
	struct ActivityDetail
	{
		static DBField ACTIVITY_ID;
		static DBField ACT_SCORE;
		static DBField WHEEL_TIMES;
		static DBField RESET_TIMES;
		static DBField LOGIN_TICK;
		static DBField USE_FREE;
		static DBField LABEL_GET;
		static DBField RANK_GET;
		static DBField REWARD_GET;
		static DBField NINE_WORD_REWARD;
		static DBField IS_FIRST;
		static DBField OPEN_TIMES;
		static DBField RESET_TICK;
		static DBField COMBINE_RESET;
		static DBField REWARD_LOCATION;
		static DBField REBATE_MAP;

		static DBField ITEM_RECORD;
		struct ItemRecord
		{
			static DBField ITEM_BIND;
			static DBField AMOUNT;
			static DBField GET_TIME;
			static DBField ITEM_ID;
			static DBField REWARD_MULT;
			static DBField SUB_VALUE;
		};

		static DBField PERSON_SLOT_SET;
		struct PersonSlotSet
		{
			static DBField TIME_POINT;
			static DBField SLOT_ID;
			static DBField BUY_TIMES;
			static DBField IS_COLOR;
			static DBField NINE_SLOT;
		};

		static DBField SHOP_SLOT_MAP;
		struct ShopSlot
		{
			static DBField IS_RARITY;
			static DBField IS_CAST;
			static DBField SLOT_ID;
			static DBField IS_BUY;
			static DBField ITEM_PRICE;
			static DBField GROUP_ID;
			static DBField DAY;
			static DBField ITEM_ID;
			static DBField ITEM_AMOUNT;
			static DBField ITEM_BIND;
			static DBField ITEM_PRICE_PRE;
		};
		static DBField REFRESH_TIMES_MAP;
		static DBField REFRESH_TICK;
		static DBField REFRESH_REWARD_MAP;

		static DBField SLOT_INDEX;
		static DBField SLOT_SCALE;
		static DBField SHOW_TIMES_MAP;
		static DBField SHOW_TIMES_FINA_MAP;
		static DBField REWARD_SCALE;
		static DBField MAZE_FREE;
		static DBField BLESS;
		static DBField FREE_TIMES_MAP;
		static DBField SLOT_ITEM_ID;
		static DBField SLOT_ITEM_NUM;
		static DBField REWARD_GEM_MAP;
		static DBField TWO_SHOW_MAP;
		static DBField THREE_SHOW_MAP;
		static DBField NOW_SLOT_MAP;
		static DBField FINA_SLOT_MAP;

		static DBField FISH_INFO_VEC;
		struct FISH_INFO
		{
			static DBField TYPE;
			static DBField LAYER;
			static DBField FLAG;
			static DBField POS_X;
			static DBField POS_Y;
		};
		static DBField FISH_REWARD;

		static DBField BLESS_VALUE;
		static DBField REWARD_RECORD_MAP;
		static DBField EXCHANGE_ITEM_FREQUENCY;
		static DBField BLESS_REWARD_FREQUENCY;
		static DBField BLESS_REWARD_POSSESS;
	};
};

struct DBDailyActSys
{
	static DBField COLLECTION;
	static DBField ACT_TYPE;
	static DBField ACTIVITY_ID;
	static DBField REFRESH_TICK;
	static DBField RESET_TICK;
	static DBField FIRST_DATE;
	static DBField LAST_DATE;
};

struct InvestRecharge
{
	static DBField COLLECTION;
	static DBField VERSION;
	static DBField INVEST_RECHARGE;
	struct InvestMap
	{
		static DBField ID;
		static DBField IS_BUY;
		static DBField BUY_TIME;
		static DBField REWARD_TIME;
		static DBField GET_REWARD;
		static DBField INVEST_INDEX;
		static DBField INVEST_REWARDS;
		static DBField VIP_REWARDS;
		static DBField VIP_LEVEL;
		static DBField SAVE_FLAG;
	};

	static DBField SERIAL_DAILY;
	static DBField SERIAL_REBATE;
	static DBField SERIAL_INVEST;
};

struct InvestRecharger
{
	static DBField COLLECTION;
	static DBField ID;
};

struct DBDailyRechargeRoleMail
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField MAIL_MAP;
	static DBField ROLE_ID;
	static DBField FIRST_RECHARGE_TIMES;
	static DBField DAILY_RECHARGE;
};

struct DBRechargeRewards
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField RECHARGE_MONEY;
	static DBField RECHARGE_TIMES;
	static DBField RECHARGE_AWARDS;
	static DBField FEEDBACK_AWARDS;
	static DBField RECHARGE_MAP;
	static DBField RECHARGE_TYPE;
	static DBField LAST_RECHARGE_TIME;
	static DBField FIRST_RECHARGE_TIME;
	static DBField LOVE_GIFT_INDEX;
	static DBField LOVE_RECHARGE_MONEY;

	static DBField DAILY_RECHARGE;
	struct DailyRecharge
	{
		static DBField TODAY_RECHARGE_GOLD;
		static DBField LAST_RECHARGE_TIME;
		static DBField DAILY_RECHARGE_REWARDS;
		static DBField FIRST_RECHARGE_GOLD;
		static DBField ACT_RECHARGE_TIMES;
		static DBField ACT_HAS_MAIL;
	};

	static DBField INVEST_RECHARGE;
	struct InvestRecharge
	{
		static DBField IS_BUY;
		static DBField BUY_TIME;
		static DBField REWARD_TIME;
		static DBField GET_REWARD;
		static DBField INVEST_INDEX;
		static DBField INVEST_REWARDS;
		static DBField VIP_REWARDS;
	};

	static DBField REBATE_RECHARGE;
	struct RebateRecharge
	{
		static DBField REBATE_TIMES;
		static DBField LAST_BUY_TIME;
	};

	static DBField TOTAL_RECHARGE;
	struct TotalRecharge
	{
		static DBField REWARD_STATES;
	};
};

struct DBGWedding
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField WEDDING_TICK;
    static DBField PARTNER_ONE;
    static DBField PARTNER_TWO;
    static DBField DAY_WEDDING_TIMES;
    static DBField DAY_REFRESH_TICK;
    static DBField INTIMACY;
    static DBField HISTORY_INTIMACY;
    static DBField WEDDING_TYPE;
    static DBField KEEPSAKE_ID;
    static DBField KEEPSAKE_LEVEL;
    static DBField KEEPSAKE_SUBLEVEL;
    static DBField KEEPSAKE_PROGRESS;

    static DBField SWEET_DEGREE_1;
    static DBField RING_LEVEL_1;
    static DBField SYS_LEVEL_1;
    static DBField TREE_LEVEL_1;
    static DBField TICK_1;
    static DBField FETCH_TICK_1;
    static DBField ONCE_REWARD_1;
    static DBField LEFT_TIMES_1;

    static DBField SWEET_DEGREE_2;
    static DBField RING_LEVEL_2;
    static DBField SYS_LEVEL_2;
    static DBField TREE_LEVEL_2;
    static DBField TICK_2;
    static DBField FETCH_TICK_2;
    static DBField ONCE_REWARD_2;
    static DBField LEFT_TIMES_2;

};

struct DBPlayerWedding
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField INTIMACY;
    static DBField WEDDING_ID;
    static DBField RECV_FLOWER;
    static DBField SEND_FLOWER;
    static DBField IS_HAS_RING;
    static DBField SIDE_FASHION_ID;
    static DBField SIDE_FASHION_COLOR;

    static DBField WEDDING_PROPERTY;
    struct	Wedding_detail
    {
    	static DBField LEVEL;
    	static DBField EXP;
    	static DBField SIDE_LEVEL;
    	static DBField SIDE_ORDER;
    };
    static DBField WEDDING_LABEL;


};

struct DBBrother
{
	static DBField COLLECTION;
    static DBField ID;
    static DBField EMOTION;
    static DBField PROMISE;
	static DBField BROTHRES;
	static DBField TASK_ID;
	static DBField MONSTER_ID;
	static DBField MONSTER_NUM;
	static DBField FINISH_INFO;
	static DBField TASK_LIST;
	static DBField CUR_SCORE;
	static DBField BROTHER_TASK;
};

struct DBMagicWeapon
{
    static DBField COLLECTION;
    static DBField OPEN;
    static DBField ROLEID;
    static DBField MAGICW_LIST;
    static DBField MAGICW_ID;
    static DBField MAGICW_SKILL_ID;
    static DBField MAGICW_SKILL_LVL;
    static DBField ACTIVED_STATE;
    static DBField IS_ADORN;
    static DBField MAGICW_RANK_GRADE;
    static DBField MAGICW_RANK_PROGRESS;
	static DBField MAGICW_QUALITY_GRADE;
	static DBField MAGICW_QUALITY_PROGRESS;
};


struct DBRankHide
{
	static DBField COLLECTION;
	static DBField ROLE_ID;
};

struct DBBattler
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField TARENA;
	static DBField TARENA_OFFLINE;
	struct Tarena
	{
		static DBField STAGE;
		static DBField SCORE;
		static DBField ADJUST_TICK;
		static DBField DRAW;
		static DBField WIN_TIMES;
		static DBField GET_EXPLOIT;
		static DBField ATTEND_TIMES;
		static DBField DRAW_WIN;
	};
};

struct DBTarenaRole
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField SERVER;
	static DBField ROLE;

	static DBField INIT;
	static DBField SYNC;
	static DBField STAGE;
	static DBField SCORE;
	static DBField UPDATE_TICK;
};

struct DBWeddingRank
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField TICK;
	static DBField SERVER;
	static DBField PLAYER1;
	static DBField PLAYER2;
};

struct DBRechargeRank
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField GET_RANK;
	static DBField COST_RANK;
	static DBField RECHARGE_BACK_RANK;

	static DBField RANK;
	static DBField ROLE_ID;
	static DBField SERVER;
	static DBField ROLE;
	static DBField AMOUNT;
	static DBField TICK;
	static DBField ACTIVITY_ID;
};

struct DBPlayerOfflineHook
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField last_start_time;
	static DBField last_end_time;

	static DBField surplus_time;
	static DBField last_reward_exp;
	static DBField last_reward;
	static DBField last_costProp;
	static DBField last_offhook_time;

	static DBField one_point_five_plue_time;
	static DBField two_plus_time;
	static DBField vip_plus_time;
	static DBField today_reward;
};

struct DBLeagueRegionFight
{
	static DBField ID;
	static DBField COLLECTION;
	static DBField TOTAL_NUM;
	static DBField LAST_TICK;
	static DBField FINISH;

	static DBField LRF_RESULT;
	static DBField SUPP_RESULT;

	struct LRF_RES
	{
		static DBField LEAGUE_ID;
		static DBField LEAGUE_NAME;
		static DBField LEAGUE_RANK;
		static DBField LEADER;
		static DBField FORCE;
		static DBField WIN_OR_STATE;
		static DBField RESOURCE_LAND_ID;
		static DBField ENEMY_LEAGUE_ID;
		static DBField ENEMY_LEAGUE_NAME;
		static DBField ENEMY_RESOURCE_LAND_ID;
	};

	struct SUPP_RES
	{
		static DBField ROLE_ID;
		static DBField SUPPORT_LEAGUE;
		static DBField RESULT;
	};
};

struct DBJYBackActPlayerRec
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField ACT_RECORD;
    struct ActRecord
    {
        static DBField ACT_ID;
        static DBField ACT_START;
        static DBField ACT_END;
        static DBField MAIL_TITLE;
        static DBField MAIL_CONTENT;
        static DBField DAILY_REFRESH_TICK;
        static DBField DAILY_VALUE;
        static DBField SINGLE_COND_VALUE;
        static DBField REWARD_VALUE;
        struct RewardValue
        {
            static DBField REWARD_ID;
            static DBField REWARD_FLAG;
        };
        static DBField UPDATE_TICK;
        static DBField REWARD_ITEM;
        struct RewardItem
        {
            static DBField REWARD_ID;
            static DBField ITEM_LIST;
        };
    };
};

struct DBMayActivity
{
	static DBField COLLECTION;
	static DBField ID;			//活动索引
	static DBField ACT_TYPE;

	static DBField REWARD_SET;
	struct RewardSet
	{
		static DBField SUB_MAP;
		static DBField DRAW_MAP;
		static DBField ACT_DRAW_MAP;
	};

	static DBField COUPLE_MAP;
	struct CoupleMap
	{
		static DBField WEDDING_ID;
		static DBField ONLINE_TICK;
		static DBField BUY_TICK;
	};

	static DBField RUN_MAP;
	struct RunInfo
	{
		static DBField ROLE_ID;
		static DBField NAME;
		static DBField SEX;
		static DBField VALUE;
		static DBField TICK;
	};
};

struct DBMayActivityer
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField ACT_TYPE;

    static DBField MAY_ACT;
    struct Act
    {
    	static DBField ID;
    	static DBField SUB_VALUE;
    	static DBField DRAWED_SET;
    	static DBField ARRIVE;
    	static DBField ARRIVE_MAP;
    	static DBField DRAWED;
    	static DBField SECOND_SUB;
    	static DBField SEND_MAP;
    	static DBField ROLE_MAP;
    	static DBField CUMULATIVE_TIMES;
    	static DBField REFRESH_TICK;
    	static DBField REFRESH_TIMES_MAP;
    };

	static DBField SHOP_SLOT_MAP;
	struct ShopSlot
	{
		static DBField IS_RARITY;
		static DBField IS_CAST;
		static DBField SLOT_ID;
		static DBField IS_BUY;
		static DBField ITEM_PRICE;
		static DBField GROUP_ID;
		static DBField DAY;
		static DBField ITEM_ID;
		static DBField ITEM_AMOUNT;
		static DBField ITEM_BIND;
		static DBField ITEM_PRICE_PRE;
	};



	struct FashionLivenessMap
	{
		static DBField TIMES;
		static DBField LIVENESS_REWARD_MAP;
	};

	struct LivenessRewardMap
	{
		static DBField SLOT_ID;
		static DBField STATE;
	};

	static DBField FASHION_ACT_INFO;
	struct FashionActInfo
	{
		static DBField LIVENESS;
		static DBField CUR_FASHION_TIMES;
		static DBField FASHION_LIVENESS_MAP;
	};

	static const std::string CORNUCOPIA_TASK;
	static const std::string CORNUCOPIA_STAGE;

	struct CornucopiaStage
	{
		static const std::string STAGEID;
		static const std::string FLAG;
	};
	struct CornucopiaTask
	{
		static const std::string TASKID;
		static const std::string COMPLETION_TIMES;
		static const std::string TOTAL_TIMES;
		static const std::string TASK_NAME;
	};
};

/////DBTrvlBattle
struct DBTrvlBattle
{
    static DBField COLLECTION;
    static DBField ID;

    // id == 0 的信息
    static DBField FIRST_TOP_ID;
    static DBField FIRST_TOP_NAME;
    
    // id == 1 的当前排行信息
    static DBField CUR_RANK_LIST;
    
    // id == 2 的当前奖励层数信息
    static DBField CUR_ROLE_LIST;

    // id == 3 的上一轮的战场日志信息
    static DBField LAST_RANK_LIST;

    // id == 4 的名人堂列表
    static DBField HISTORY_FAME_LIST;

    struct BattleRole
    {
        static DBField ROLE_ID;
        static DBField LEAGUE_ID;
        static DBField ROLE_NAME;
        static DBField MAX_FLOOR;
        static DBField LAST_REWARD_SCORE;
        static DBField NEXT_REWARD_SCORE;
        static DBField NEXT_SCORE_REWARD_ID;
        static DBField SERVER;
    };

    struct BattleRank
    {
        static DBField RANK;
        static DBField SCORE;
        static DBField TOTAL_KILL_AMOUNT;
        static DBField TICK;
        static DBField ROLE_ID;
        static DBField ROLE_NAME;
        static DBField PREV;
        static DBField SERVER_FLAG;
        static DBField SEX;
        static DBField CAREER;
        static DBField LEVEL;
        static DBField FORCE;
        static DBField WEAPON;
        static DBField CLOTHES;
        static DBField WING_LEVEL;
        static DBField SOLIDER_LEVEL;
        static DBField VIP_TYPE;
        static DBField MOUNT_SORT;
        static DBField SWORD_POOL;
        static DBField TIAN_GANG;
        static DBField FASHION_ID;
        static DBField FASHION_COLOR;
    };

};

struct DBLocalTravTeam
{
	static DBField COLLECTION;
	static DBField ID;

	static DBField TEAM_NAME;
	static DBField LEADER_ID;
	static DBField AUTO_SIGNUP;
	static DBField AUTO_ACCEPT;
	static DBField NEED_FORCE;
	static DBField IS_SIGNUP;
	static DBField REFRESH_SIGNUP_TICK;
	static DBField CREATE_TICK;
	static DBField LAST_LOGOUT_TICK;

	static DBField TRAV_TEAMER;
	static DBField APPLY_MAP;
	struct TravTeamer
	{
		static DBField TEAMER_ID;
		static DBField TEAMER_NAME;
		static DBField TEAMER_LEVEl;
		static DBField TEAMER_SEX;
		static DBField TEAMER_CAREER;
		static DBField TEAMER_FORCE;
		static DBField LOGOUT_TICK;
		static DBField JOIN_TICK;
	};
};

struct DBRemoteTravTeam
{
    static DBField COLLECTION;
    static DBField ID;
    static DBField TEAM_NAME;
    static DBField LEADER_ID;
    static DBField SERVER;

    static DBField QUALITY_TIMES;
    static DBField SCORE;
    static DBField CONTINUE_WIN;
    static DBField UPDATE_TICK;

    static DBField TRAV_TEAMER;
    struct TravTeamer
    {
        static DBField TEAMER_ID;
        static DBField TEAMER_NAME;
        static DBField TEAMER_SEX;
        static DBField TEAMER_CAREER;
        static DBField TEAMER_LEVEL;
        static DBField TEAMER_FORCE;
    };
};

struct DBQualityInfo
{
	static DBField COLLECTION;
	static DBField ID;
	static DBField SIGNUP_SET;
};

#endif //_GAMEFIELD_H_
