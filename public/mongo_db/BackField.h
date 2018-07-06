/*
 * BackField.h
 *
 * Created on: 2013-07-31 15:02
 *     Author: lyz
 */

#ifndef _BACKFIELD_H_
#define _BACKFIELD_H_

#include <string>

////////serial/////////////
//{{{
struct DBBackSerial
{
    static const std::string COLLECTION;

    static const std::string ID;
    static const std::string MONEY_SERIAL;
    struct MoneySerial
    {
        static const std::string SERIAL;
        static const std::string FRESH_TICK;
        static const std::string COPPER;
        static const std::string GOLD;
        static const std::string BIND_GOLD;
        static const std::string BIND_COPPER;
    };

    static const std::string ITEM_SERIAL;
    struct ItemSerial
    {
        static const std::string SERIAL;
        static const std::string ITEM_ID;
        static const std::string FRESH_TICK;
        static const std::string VALUE;
    };

    static const std::string EXP_SERIAL;
    struct ExpSerial
    {
        static const std::string SERIAL;
        static const std::string FRESH_TICK;
        static const std::string VALUE;
    };
};
//}}}

///////////////////FlowControl
// {{{
struct DBFlowControl
{
	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string SERVER_INDEX_SET;
	static const std::string NEED_LOAD_DATA;

	static const std::string IS_FORBIT_LOGIN;
	static const std::string FORBIT_CHANNEL;
	static const std::string SERIAL_RECORD;
	static const std::string MONEY_SERIAL_RECORD;
	static const std::string ITEM_SERIAL_RECORD;
	static const std::string EQUIP_SERIAL_RECORD;
	static const std::string MOUNT_SERIAL_RECORD;
	static const std::string PET_SERIAL_RECORD;
	static const std::string SKILL_SERIAL_RECORD;
	static const std::string MAIL_SERIAL_RECORD;
	static const std::string MARKET_SERIAL_RECORD;
	static const std::string ACHIEVE_SERIAL_RECORD;
	static const std::string FORCE_REFRSH_RANK_TPYE;
	static const std::string PLAYER_LEVEL_SERIAL;
	static const std::string OTHER_SERIAL;
	static const std::string ONLINE_USER_SERIAL;
	static const std::string LOGIN_SERIAL;
	static const std::string TASK_SERIAL;
	static const std::string RANK_SERIAL;
	static const std::string CHAT_SERIAL;

	struct ForceRefreshRankType
	{
		static const std::string RANK_FIGHT_LEVEL;
		static const std::string RANK_FIGHT_FORCE;
		static const std::string RANK_KILL_VALUE;
		static const std::string RANK_KILL_NUM;
		static const std::string RANK_KILL_NORMAL;
		static const std::string RANK_KILL_EVIL;
		static const std::string RANK_PET;
		static const std::string RANK_MOUNT;
		static const std::string RANK_FUN_MOUNT;
		static const std::string RANK_FUN_GOD_SOLIDER;
		static const std::string RANK_FUN_MAGIC_EQUIP;
		static const std::string RANK_FUN_XIAN_WING;
		static const std::string RANK_FUN_LING_BEAST;
		static const std::string RANK_FUN_BEAST_EQUIP;
		static const std::string RANK_FUN_BEAST_MOUNT;
		static const std::string RANK_FUN_BEAST_WING;
		static const std::string RANK_FUN_BEAST_MAO;
		static const std::string RANK_FUN_TIAN_GANG;
	};
};
//}}}

// DBSceneLine////////////////
// {{{
struct DBSceneLine
{
    static const std::string COLLECTION;
    static const std::string FLAG;
    static const std::string SERVER_INDEX_SET;
    static const std::string SERVER_INDEX;
    static const std::string SCENE;
    struct Scene
    {
        static const std::string SCENE_ID;
        static const std::string TYPE;
        static const std::string MAX_LINE;
        static const std::string PER_PLAYER;
    };
};
// }}}

// DBBackBroDetail////////////////
// {{{
struct DBBackBroDetail
{
	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string DB_OP_TYPE;
	static const std::string DATA_CHANGE;
	static const std::string BRO_RECORD;

	struct Bro_record
	{   static const std::string BRO_TYPE;
		static const std::string BRO_TICK;
		static const std::string BRO_TIMES;
		static const std::string REPEAT_TIMES;
		static const std::string INTERVAL_SEC;
		static const std::string CONTENT;
	};
};
// }}}

// DBBackNotice////////////////
struct DBBackNotice
{
    static const std::string COLLECTION;
    static const std::string ID;
    static const std::string NOTIFY;

    static const std::string TITLE;
    static const std::string CONTENT;
    static const std::string TICK;

    static const std::string START_TICK;
    static const std::string ITEM_SET;
};

// DBBackRole////////////////////
struct DBBackRole
{
    static const std::string COLLECTION;
    static const std::string SERVER_FLAG;
    static const std::string MARKET;
    static const std::string PLATFORM;
    static const std::string AGENT;
    static const std::string PLATFORM_CODE;
    static const std::string AGENT_CODE;
    static const std::string NET_TYPE;
    static const std::string SYS_VERSION;
    static const std::string SYS_MODEL;
    static const std::string MAC;
    static const std::string IP;
    static const std::string IMEI;
    static const std::string CREATE_MARKET_CODE;
    static const std::string CREATE_AGENT;
    static const std::string CREATE_AGENT_CODE;
    static const std::string CREATE_NET_TYPE;
    static const std::string CREATE_SYS_VERSION;
    static const std::string CREATE_SYS_MODEL;
    static const std::string CREATE_MAC;
    static const std::string CREATE_IP;
    static const std::string CREATE_IMEI;
    static const std::string IS_NEW_MAC;
    static const std::string ROLE_ID;
    static const std::string ACCOUNT;
    static const std::string ROLE_NAME;
    static const std::string CAREER;
    static const std::string SEX;
    static const std::string LEVEL;
    static const std::string FIGHT_FORCE;
    static const std::string SCENE_ID;
    static const std::string COORD_X;
    static const std::string COORD_Y;
    static const std::string CREATE_TIME;
    static const std::string EXPERIENCE;
    static const std::string LEAGUE_ID;
    static const std::string LEAGUE_NAME;
    static const std::string FIGHTER_PROP;

    static const std::string BIND_COPPER;
    static const std::string BIND_GOLD;
    static const std::string COPPER;
    static const std::string GOLD;
    static const std::string GOLD_USE;
    static const std::string COUPON_USE;
    static const std::string COPPER_USE;
    static const std::string BIND_COPPER_USE;

    static const std::string LAST_SIGN_IN_TIME;
    static const std::string LAST_SIGN_OUT_TIME;
    static const std::string LOGIN_COUNT;
    static const std::string ON_HOOK;
    static const std::string ONLINE;
    static const std::string PERMISSION;
    static const std::string RECHARGE_FIRST;
    static const std::string RECHARGE_GOLD;
    static const std::string VIP;
    static const std::string VIP_DEADLINE;
    static const std::string VIP_START_TIME;
    static const std::string KILL_NUM;
    static const std::string KILL_VALUE;
};

// DBBackAccount/////////////////
struct DBBackAccount
{
    static const std::string COLLECTION;
    static const std::string ACCOUNT;
    static const std::string PERMISSION;
    static const std::string CREATE;
    static const std::string CREATE_TIME;
};


// DBBackRecharge////////////////
struct DBBackRecharge
{
	static const std::string COLLECTION;
	static const std::string ID; /*订单的唯一索引，一般是订单号*/
	static const std::string FLAG;
	static const std::string ORDER_NUM;
	static const std::string RECHARGE_CHANNEL;
	static const std::string RECHARGE_MONEY;
	static const std::string RECHANGE_GOLD;
	static const std::string ACCOUNT;/*充值：按照玩家的账户给角色充值*/
	static const std::string ROLE_ID;/*无用的字段*/
	static const std::string RECHARGE_TICK;
	static const std::string RECHARGE_RANK_TAG;
};

// DBBackMail////////////////
struct DBBackMail
{
	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string READ;
	static const std::string TIME;
	static const std::string RECEIVER_SET;
	static const std::string SENDER;
	static const std::string TITLE;
	static const std::string CONTENT;

	static const std::string MONEY;
	struct Money
	{
		static const std::string GOLD;
		static const std::string BIND_GOLD;
		static const std::string COPPER;
		static const std::string BIND_COPPER;
	};

	static const std::string ITEM;
	struct Item
	{
		static const std::string ITEM_ID;
		static const std::string ITEM_AMOUNT;
		static const std::string ITEM_BIND;
		static const std::string ITEM_NAME;
	};

};

struct DBBackActivity
{
	static const std::string COLLECTION;
	static const std::string AGENT;
	static const std::string ACT_INDEX;
	static const std::string OPEN_FLAG;
	static const std::string COND_TYPE;
	static const std::string START_COND;
	static const std::string MAIL_ID;
	static const std::string RED_EVENT;
	static const std::string SPECIAL_NOTIFY;

	static const std::string SORT;
	static const std::string REWARD_TYPE;
	static const std::string FIRST_TYPE;
	static const std::string SECOND_TYPE;
	static const std::string START_TICK;
	static const std::string STOP_TICK;
	static const std::string UPDATE_TICK;
	static const std::string REWARD_START;
	static const std::string REWARD_END;
	static const std::string CYCLE_TIMES;
	static const std::string OPEN_TIME;
	static const std::string LIMIT;
	static const std::string REDRAW;
	static const std::string DAY_CLEAR;
	static const std::string RECORD_VALUE;

	static const std::string ACT_TITLE;
	static const std::string ACT_CONTENT;
	static const std::string REWARD;
	static const std::string DRAWED;
    static const std::string T_SUB_MAP;
	static const std::string F_RANK_INFO;

	static const std::string MAIL_TITLE;
	static const std::string MAIL_CONTENT;
	static const std::string PRIORITY;
	static const std::string ICON_TYPE;

	static const std::string CORNUCOPIA_RECHARGE;
	struct CornucopiaRecharge
	{
		static const std::string ROLE_ID;
		static const std::string PLAYER_NAME;
		static const std::string GET_TIME;
		static const std::string CORNUCOPIA_GOLD;
		static const std::string REWARD_MULT;
	};

	struct Reward
	{
		static const std::string TYPE;
		static const std::string COND;
		static const std::string COST_ITEM;
		static const std::string PRE_COST;
		static const std::string CONTENT;
		static const std::string ITEM;
		static const std::string BROCAST;
		static const std::string TIMES;
		static const std::string MUST_RESET;
		static const std::string HANDLE_TYPE;
		static const std::string CASH_COUPON;
		static const std::string REWARD_ID;
		static const std::string REWARD_TYPE;
		static const std::string REWARD_START_COND;
		static const std::string EXCHANGE_TYPE;
		static const std::string EXCHANGE_ITEM_NAME;
		static const std::string SUB_MAP;
		static const std::string RECHARGE_MAP;
		static const std::string DRAWED_MAP;
	};
};

struct DBBackWonderfulActivity
{
	static const std::string COLLECTION;
	static const std::string ACTIVITY_ID;
	static const std::string DATE_TYPE;
	static const std::string FIRST_DATE;
	static const std::string LAST_DATE;
	static const std::string OPEN_FLAG;
	static const std::string REFRESH_RICK;
	static const std::string AGENT;
	static const std::string SORT;
	static const std::string ACTIVITY_TYPE;
	static const std::string ACT_CONTENT;
	static const std::string VALUE1;
	static const std::string VALUE2;
};

struct DBFestActivity
{
	static const std::string COLLECTION;
	static const std::string ID;

	static const std::string ICON_TYPE;
	static const std::string STATCK_TICK;
	static const std::string END_TICK;
	static const std::string UPDATE_TICK;
};

struct DBBackMayActivity
{
	static const std::string COLLECTION;
	static const std::string ID;

	static const std::string OPEN_FLAG;
	static const std::string REFRESH_TICK;
	static const std::string BEGIN_DATE;
	static const std::string END_DATE;
	static const std::string ACT_TYPE;
	static const std::string ACT_SET;
	static const std::string AGENT;
};

// ActiCode////////////////////
struct DBBackActiCode
{
	static const std::string COLLECTION;
	static const std::string BACKUP_COLLECTION;
	static const std::string ID;
	static const std::string USER_ID;
	static const std::string GIFT_SORT;
	static const std::string AMOUNT;
	static const std::string START_TIME;
	static const std::string END_TIME;
	static const std::string USED_TIME;
	static const std::string BATCH_ID;
	static const std::string USE_ONLY_VIP;
	static const std::string ACTI_CODE;
};

// MediaGiftDetail////////////////////
// {{{
struct DBBackMediaGiftDef
{
	static const std::string COLLECTION;

	static const std::string UPDATE_STATUS;
	static const std::string UPDATE_TICK;

	static const std::string GIFT_SORT;
	static const std::string GIFT_TYPE;
	static const std::string GIFT_TAG;
	static const std::string USE_TIMES;
	static const std::string SHOW_ICON;
	static const std::string HIDE_USED;
	static const std::string IS_SHARE;
	static const std::string EXPIRE_TIME;
	static const std::string GIFT_NAME;
	static const std::string GIFT_DESC;
	static const std::string VALUE_EXTS;

	static const std::string FONT_COLOR;
	struct ValueExts
	{
		static const std::string VALUE_EXT;
	};

	static const std::string GIFT_ITEMS;

	struct ItemObj
	{
	static const std::string ITEM_ID;
	static const std::string ITEM_AMOUNT;
	static const std::string ITEM_BIND;
	static const std::string ITEM_INDEX;
	};
};

// }}}

//49you-luoshenyu-box//////////////////////
//{{{
struct DBBackDownLoadBoxGift{
	static const std::string COLLECTION;

	static const std::string AGENT_CODE;
	static const std::string DOWNLOAD_URL;
	static const std::string ITEM_LIST;

	struct ItemObj
	{
		static const std::string ITEM_ID;
		static const std::string ITEM_AMOUNT;
		static const std::string ITEM_BIND;
	};
};

//}}}

// BackCustomerSVCRecord////////////////
// {{{
struct BackCustomerSVCRecord
{
	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string SENDER_ID;
	static const std::string SEND_TICK;
	static const std::string RECORD_TYPE;
	static const std::string HAS_READ;
	static const std::string HAS_REPLAY;
	static const std::string NEED_LOAD_DATA;
	static const std::string SENDER_NAME;
	static const std::string CONTENT;
	static const std::string TITLE;
	static const std::string REPLAY_CONTENT;
	static const std::string SENDER_LEVEL;
	static const std::string SERVER_CODE;
	static const std::string PLATFORM;
	static const std::string AGENT;
	static const std::string RECHARGE_GOLD;
	static const std::string REMOVE_FLAG;
	static const std::string EVALUATE_TICK;
	static const std::string EVALUATE_LEVEL;
	static const std::string EVALUATE_STAR;
	static const std::string OPINION_INDEX;
};

struct DBBackDailyRecharge
{
	static const int OPEN_TIME_ID;

	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string START_TIME;
	static const std::string END_TIME;
};

// }}}

// BackRestriction 后台限制/////////////////////////
// {{{

struct DBBackRestriction
{
	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string ACCOUNT;
	static const std::string ROLE_NAME;
	static const std::string ROLE_ID;
	static const std::string IP_ADDR;
	static const std::string DESC;
	static const std::string MANAGER;
	static const std::string OPERATION;
	static const std::string OPER_TYPE;
	static const std::string FLAG;
	static const std::string EXPIRED_TIME;
	static const std::string CREATED_TIME;
	static const std::string MAC;
};

struct DBBanIpInfo
{
	static const std::string COLLECTION;
	static const std::string IP_UINT;
	static const std::string IP_STRING;
	static const std::string EXPIRED_TIME;
};

struct DBBanMacInfo
{
	static const std::string COLLECTION;
	static const std::string MAC_STRING;
	static const std::string MAC;
	static const std::string EXPIRED_TIME;
};

struct DBWhiteIpInfo
{
	static const std::string COLLECTION;
	static const std::string IP_UINT;
	static const std::string IP_STRING;
};

struct DBBackSwitcher
{
	static const std::string COLLECTION;
	static const std::string NAME;
	static const std::string VALUE;
};

struct DBBackModify
{
	static const std::string COLLECTION;
	static const std::string NAME;
	static const std::string IS_UPDATE;
	static const std::string ROLE_ID;
	static const std::string LEAGUE_ID;
	static const std::string VALUE;

	struct ValueEle
	{
		static const std::string QQ_NUM;
		static const std::string DES_CONTENT;
		static const std::string DES_MAIL;
		static const std::string VIP_LEVEL_LIMIT;
		static const std::string RECHARGE;
	};

	static const std::string START_TICK;
	static const std::string END_TICK;
	static const std::string OPEN_STATE;
	static const std::string ACTIVITY_ID;
};

// 后台活动时间开关
struct DBBackDraw
{
    static const std::string COLLECTION;
    static const std::string FLAG;
    static const std::string SERVER_INDEX;
    static const std::string ACTIVITY_ID;
    static const std::string S_TICK;
    static const std::string E_TICK;
};

struct DBBackContactWay
{
	static const std::string COLLECTION;
	static const std::string MARKET_CODE;
	static const std::string CONTACT_WAY;
};

struct DBServerInfo
{
    static const std::string COLLECTION;
    static const std::string INDEX;	//index == 0 表示本服的服务器信息
    static const std::string SERVER_ID;
    static const std::string IS_IN_USE;
    static const std::string SERVER_FLAG;
    static const std::string CUR_SERVER_FLAG;
    static const std::string SERVER_PREV;
    static const std::string SERVER_NAME;
    static const std::string FRONT_SERVER_NAME;
    static const std::string OPEN_SERVER;
    static const std::string COMBINE_SERVER_SET;
    static const std::string COMBINE_TO_SERVER_ID;
};

struct DBCombineServer
{
	static const std::string COLLECTION;
	static const std::string ID;

	static const std::string SERVER_FLAG;
	static const std::string IP;
	static const std::string PORT;
	static const std::string SPECIAL_FILE;
	static const std::string UPDATE_TICK;
};

struct DBChatLimit
{
	static const std::string COLLECTION;
	static const std::string ID;	// channel_type
	static const std::string LIMIT_LEVEL;
	static const std::string CHAT_INTREVAL;
	static const std::string UPDATE_TICK;
};

struct DBWordCheck
{
	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string LIST;
	static const std::string TIME;
};

struct DBVipChat
{
	static const std::string COLLECTION;
	static const std::string ID;
	static const std::string UPDATE_TICK;
	static const std::string TIME;

	static const std::string DETAIL;
	struct VipLimit
	{
		static const std::string VIP_LV;
		static const std::string INFO;
	};
};

struct DBJYBackActivity
{
    static const std::string COLLECTION;
    static const std::string ACT_ID;
    static const std::string UPDATE_FLAG;
    static const std::string UPDATE_TICK;
    static const std::string FIRST_TYPE;
    static const std::string SECOND_TYPE;
    static const std::string ACT_TITLE;
    static const std::string ACT_CONTENT;
    static const std::string ACT_START;
    static const std::string ACT_END;
    static const std::string IS_OPEN;
    static const std::string ORDER;
    static const std::string REWARD_MAIL_TITLE;
    static const std::string REWARD_MAIL_CONTENT;
    static const std::string NEED_GOLD;
    static const std::string REWARD;
    struct Reward
    {
    	static const std::string COND_TYPE;
        static const std::string COND;
        static const std::string REWARD_TYPE;
        static const std::string REWARD_ITEM;
        static const std::string RETURN_GOLD_RATE;
    };
};

struct DBCorrectTrvlRank
{
    static const std::string COLLECTION;
    static const std::string UPDATE_FLAG;
    static const std::string ID;
    static const std::string TYPE;
    static const std::string OP_TYPE;
    static const std::string AMOUNT;
    static const std::string TICK;
    static const std::string ACTIVITY_ID;

    static const std::string SERVER;
    static const std::string ROLE;
};

#endif //_BACKFIELD_H_
