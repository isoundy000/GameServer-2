/*
 * RankStruct.h
 *
 *  Created on: Feb 26, 2014
 *      Author: louis
 */

#ifndef RANKSTRUCT_H_
#define RANKSTRUCT_H_

#include "LogicStruct.h"
class TQueryCursor;

////only for record
//static const std::string RANK_NAME[] = {"", "FIGHT_FORCE", "FIGHT_LEVEL",
//		"MAGICAL_POLISH", "EQUIPMENT", "PET", "MOUNT", "RANK_SINGLE_SCRIPT_ZYFM"};

enum RANK_TYPE
{
	RANK_FIGHT_FORCE         =	1,	// 战力排行
	RANK_FIGHT_LEVEL         =	2,	// 等级排行
	RANK_PET                 =	3,	// 宠物排行
	RANK_MOUNT               =	4,	// 坐骑排行
	RANK_SINGLE_SCRIPT_ZYFM  =	5,	// 斩妖伏魔录排行
	RANK_LEAGUE				 =	6,	// 帮派排行
	RANK_MAGICAL_POLISH      =  7, 
	RANK_EQUIPMENT           =  8,
    RANK_SEND_FLOWER         = 10,  // 送花排行
    RANK_RECV_FLOWER         = 11,  // 收花排行
    RANK_KILL_VALUE          = 12,  // 恶人榜
    RANK_HERO                = 13,  // 英雄榜

    RANK_FUN_MOUNT			 = 14,	// 战骑
    RANK_FUN_GOD_SOLIDER	 = 15,	// 神兵
    RANK_FUN_MAGIC_EQUIP	 = 16,	// 法器
    RANK_FUN_XIAN_WING		 = 17,	// 仙羽
    RANK_FUN_LING_BEAST		 = 18,	// 灵宠
    RANK_FUN_BEAST_EQUIP	 = 19,	// 灵武
    RANK_FUN_BEAST_MOUNT	 = 20,	// 灵骑
    RANK_FUN_BEAST_WING		 = 21,	// 灵羽
    RANK_FUN_BEAST_MAO		 = 22,	// 灵猫
    RANK_FUN_TIAN_GANG		 = 23,	// 天罡

    //====下面是实时排行ID > 10000==========================
    RANK_LOTTERY             =  10001,	// 限时时装抽奖排行
    RANK_LOTTERY_BEAST		 =  10002,	// 限时神兽抽奖排行
    RANK_ACT_SEND_FLOWER     = 	10003,  // 后台活动送花排行
    RANK_ACT_RECV_FLOWER     = 	10004,  // 后台活动收花排行

    //===活动排行 ID 》 20000
    RANK_COMBINE_CONSUM_RANK =	20001,	// 合服活动消费排行

	RANK_TYPE_END
};

struct RankerShapeInfo
{
	RankerShapeInfo(void);
	void reset(void);

	int __sex;
	int __weapon;
	int __clothes;
	int __label;
	int __mount;
	int __pet;
};

struct BaseRankInfo
{
	BaseRankInfo(void);
	void reset(void);

	int __cur_rank;
	int __last_rank;
	Int64 __achive_tick;
	Int64 __role_id;
	std::string __role_name;
	std::string __league_name;
	int __rank_type;
	int __rank_value;
	int __fight_force;
	Int64 __ext_value;
	Int64 __additional_id;

	int __vip_type;
	Int64 __worship_num;
	int __is_worship;

//	RankerShapeInfo __shaper_info;
};

struct RankRecord
{
	RankRecord(int p = 0);
	void reset(void);

    /*
     * 系列化
     * */
	void serialize(ProtoRankRecord* proto_record);
	void unserialize(const ProtoRankRecord* proto_record);

    int __vec_index;
	BaseRankInfo __single_player_info;
};

struct RankRefreshDetail
{
	RankRefreshDetail(void);
	void reset(void);

	int __refresh_type;		// 刷新类型 0日刷 1按配置时间刷
	int __rank_type;		// 排名类型
	int __refresh_interval;		//刷新间隔
	int __last_refresh_tick;	//上次刷新时间
	int __next_refresh_tick;	//下次刷新时间
	IntVec __refresh_tick_set;	//总刷新时间点集合
};

struct PlayerOfflineData
{
	Int64 role_id_;

	Int64 mount_beast_tick_;
	string mount_beast_info_;

	void reset(void);
};

typedef ObjectPoolEx<RankRecord> RankRecordPool;
typedef std::vector<RankRecord*> RankRecordVec;
typedef std::map<int, RankRecordVec> RankShowPannel; // key : rank_type

typedef std::map<int, RankRefreshDetail> RankRefreshManager;

typedef std::map<Int64, RankRecord*> RankRecordMap;// key : player_id
typedef std::map<int, RankRecordMap> RankPannel; // key : rank_type

extern bool rank_record_cmp(const RankRecord *left, const RankRecord *right);

#endif /* RANKSTRUCT_H_ */
