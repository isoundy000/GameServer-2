/*
 * GameHeader.h
 *
 *  Created on: Jun 27, 2013
 *      Author: peizhibi
 */

#ifndef GAMEHEADER_H_
#define GAMEHEADER_H_

#include <math.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <google/protobuf/message.h>

#include <cstring>

#include <map>
#include <list>
#include <vector>
#include <set>
#include <bitset>
#include <memory>
#include <queue>

#include "json/json.h"

#include "Lib_Log.h"
#include "Singleton.h"
#include "Date_Time.h"
#include "Time_Value.h"
#include "Block_Buffer.h"

#include "Thread.h"
#include "Mutex_Guard.h"
#include "Thread_Mutex.h"
#include "Block_Pool_Group.h"

using ::google::protobuf::Message;

using std::string;
using std::auto_ptr;

namespace mongo
{
	class BSONObj;
	class BSONObjBuilder;
	class BSONObjIterator;
	BSONObj fromjson(const std::string &str);
	BSONObj fromjson(const char *str, int* len);
	class DBClientConnection;
	class DBClientCursor;
}

using mongo::BSONObj;
using mongo::BSONObjBuilder;
using mongo::BSONObjIterator;
using mongo::fromjson;
using mongo::DBClientConnection;
using mongo::DBClientCursor;

namespace GameEnum
{
	enum HEADER_ENUM
	{
		EQUIP_MAX_INDEX				= 8,	//身上容量
		EQUIP_JEWEL_MAX_INDEX		= 3,	//镶嵌的宝石最大数量

		FUN_MOUNT					= 1,	//战骑
		FUN_GOD_SOLIDER				= 2,	//神兵
		FUN_MAGIC_EQUIP				= 3,	//法器
		FUN_XIAN_WING				= 4,	//神羽
		FUN_LING_BEAST				= 5,	//仙童
		FUN_BEAST_EQUIP				= 6,	//仙武
		FUN_BEAST_MOUNT				= 7,	//仙兽
		FUN_BEAST_WING				= 8,	//仙翼
		FUN_BEAST_MAO				= 9,	//灵兽
		FUN_TIAN_GANG				= 10,	//元神
		FUN_TOTAL_MOUNT_TYPE		= FUN_TIAN_GANG,

        //operate
        NORMAL_OPERATE 				= 0,	//普通
        ESCORT_OPERATE				= 1,	//护送
        TASK_LEVLE_OPERATE			= 2,	//任务
        MAIN_TOWN_OPERATE			= 3,	//回城
        TOTAL_OPERATE,


		HEADER_ENUM_END
	};
}

class Epoll_Watcher;
class BaseUnit;
class ProtoHead;
class ProtoClientHead;

class MongoUnit;
class UnitMessage;
class Transaction;
class MongoDataMap;

class ProtoItem;
class ProtoEquip;
class ProtoMoney;
class ProtoCoord;
class ProtoSerialObj;
class ProtoPackage;
class ProtoPackageSet;
class ProtoBeastSkill;
class ProtoMagicalInfo;
class ProtoCustomerSVCRecord;

class ProtoFashion;
class ProtoPairObj;

class Proto50100156;
class Proto51401505;
class Proto51401421;

class Money;
class DBShopMode;
class DBTradeMode;
class PackageItem;
class GamePackage;
class ArrTimeManager;
class GameTimerHandler;

class MapMonitor;
class LogicMonitor;
class PlayerManager;
class MongoConnector;

class MapBeast;
class MasterDetial;
class ServerInfo;

class MLPacker;
class GameMover;
class GameFighter;

class GatePlayer;
class LogicPlayer;
class MapLogicPlayer;

class MapPlayer;
class MapPlayerEx;

class SessionManager;
class FighterSkill;
class BrocastPara;
class RankRecord;
class FlowControlDetail;
class FlowControl;
class ShopItem;

class ProtoMallItem;
class ProtoMallList;
class ProtoLimitTimeLabel;
class ProtoAchieveDetail;
class ProtoRankRecord;
class ProtoMailInfo;
class ProtoBrocastNewInfo;
class ProtoFashionInfo;
class ProtoFashionTips;

typedef int64_t BlockIndexType;
typedef int64_t GridIndexType;

typedef long long int Int64;

typedef std::vector<int> 				IntVec;
typedef std::vector<Int64>				LongVec;
typedef std::vector<BSONObj>			BSONVec;
typedef std::vector<string> 			StringVec;
typedef std::vector<FighterSkill*> 		SkillVec;
typedef std::vector<BrocastPara> 		BrocastParaVec;

typedef std::vector<void*> VoidVec;
typedef std::vector<PackageItem*> ItemVector;
typedef std::vector<MapPlayerEx*> MapPlayerExVec;
typedef std::vector<PackageItem> NPItemVector;

typedef std::list<int>	IntList;
typedef std::list<Int64> LongList;
typedef std::list<PackageItem*> ItemList;
typedef std::list<string> StringList;

typedef std::pair<int, int> IntPair;
typedef std::pair<Int64, Int64> LongPair;
typedef std::pair<string, string> FontPair;
typedef std::pair<double, double> DoublePair;
typedef std::pair<int, string>	IntStrPair;

typedef std::vector<IntPair> IntPairVec;
typedef std::vector<LongPair> LongPairVec;
typedef std::list<LongPair> LongPairList;

typedef std::map<int, int> IntMap;
typedef std::map<Int64, Int64> LongMap;
typedef std::map<int, IntPair> IntPairMap;

typedef std::map<int, double> 			IDMap;
typedef std::map<int, PackageItem*> 	ItemListMap;
typedef std::map<int, FighterSkill*> 	SkillMap;
typedef std::map<Int64, string> 		LongStrMap;
typedef std::map<string, int>			StrIntMap;

typedef boost::unordered_map<int, int> 		BIntMap;
typedef boost::unordered_map<int, Int64> 	BIntLongMap;
typedef boost::unordered_map<Int64, Int64> 	BLongMap;
typedef boost::unordered_map<string, int>	BStrIntMap;

typedef std::set<int>						IntSet;
typedef std::set<Int64>						LongSet;

typedef boost::unordered_set<int> 			BIntSet;
typedef boost::unordered_set<Int64>			BLongSet;

/*
 * fill the memory with 0
 * */
#define ZeroMemory(DES, LENGTH) ::memset(DES, 0, LENGTH)

/*
 * calculate array length
 * */
#define ARRAY_LENGTH(ARRAY)	(sizeof(ARRAY) / sizeof(ARRAY[0]))

/*
 * safe delete
 * */
#define SAFE_DELETE(P)	\
	if (P != 0) \
	{ \
		delete P; \
		P = 0; \
	}

/*
 * calculate page size
 */
#define GAME_PAGE_SIZE(TOTAL, PAGE) 		\
	((TOTAL) % (PAGE) == 0 ? (TOTAL) / (PAGE) : ((TOTAL) / (PAGE) + 1))

/*
 * fetch the different one
 * */
#define FETCH_DIFFERENT_ONE(INPUT, SELECTA, SELECTB) \
	((INPUT == SELECTA) ? SELECTB : SELECTA)

/*
 * adjust no zero
 * */
#define ADJUST_NO_ZERO(SELECTA, SELECTB) \
	(SELECTA == 0 ? SELECTB : SELECTA)

/*
 * dynamic cast
 * */
#define DYNAMIC_CAST(TYPE, DES_VAR, SRC_VAR) 	\
		TYPE DES_VAR = dynamic_cast<TYPE>(SRC_VAR)

/*
 * return finer process result without message
 */
#define FINER_PROCESS_NOTIFY(RECOGN) \
    return this->respond_to_client(RECOGN)

/*
 * return finer process result with message
 */
#define FINER_PROCESS_RETURN(RECOGN, MB) \
    return this->respond_to_client(RECOGN, MB)

/*
 * return finer process result without message
 */
#define FINER_PLAYER_PROCESS_NOTIFY(RECOGN) \
    return player->respond_to_client(RECOGN)

/*
 * return finer process result with message
 */
#define FINER_PLAYER_PROCESS_RETURN(RECOGN, MB) \
    return player->respond_to_client(RECOGN, MB)

/*
 * if CONDITION false then return
 */
#define JUDGE_RETURN(CONDITION, RETURN) \
    if (!(CONDITION))\
    {\
        return RETURN;\
    }

/*
 * if CONDITION false then continue
 */
#define JUDGE_CONTINUE(CONDITION) \
    if (!(CONDITION))\
    {\
        continue;\
    }

/*
 * if CONDITION false then break
 */
#define JUDGE_BREAK(CONDITION) \
    if (!(CONDITION))\
    {\
        break;\
    }


/*
 * check and notify self
 * */
#define CONDITION_NOTIFY_RETURN(CONDITION, RECOGN, RET_CODE) \
	if (!(CONDITION)) \
	{ \
		this->set_last_error(RET_CODE);	\
		return this->respond_to_client_error(RECOGN, RET_CODE); \
	}

#define CONDITION_NOTIFY_RETURN_B(CONDITION, RECOGN, RET_CODE, WELL_CODE) \
	if (RET_CODE == WELL_CODE) { \
		return this->respond_to_client(RECOGN); \
	} \
	CONDITION_NOTIFY_RETURN(CONDTION, RECOGN, RET_CODE)

/*
 * check and notify player
 * */
#define CONDITION_PLAYER_NOTIFY_RETURN(CONDITION, RECOGN, RET_CODE) \
	if (!(CONDITION)) \
	{ \
		return player->respond_to_client_error(RECOGN, RET_CODE); \
	}

#define CONDITION_PLAYER_NOTIFY_RETURN_B(CONDITION, RECOGN, RET_CODE, WELL_CODE) \
	if (RET_CODE == WELL_CODE) { \
		return player->respond_to_client(RECOGN); \
	} \
	CONDITION_PLAYER_NOTIFY_RETURN(CONDITION, RECOGN, RET_CODE)
/*
 * src_var dynamic_cast, if des_var is null, return
 * */
#define DYNAMIC_CAST_RETURN(TYPE, DES_VAR, SRC_VAR, RETURN) \
		DYNAMIC_CAST(TYPE, DES_VAR, SRC_VAR); \
		JUDGE_RETURN(DES_VAR != NULL, RETURN)

/*
 * msg dynamic_cast, if des_var is null, return
 * */
#define MSG_DYNAMIC_CAST_RETURN(TYPE, DES_VAR, RETURN) \
		DYNAMIC_CAST_RETURN(TYPE, DES_VAR, msg, RETURN)

/*
 * src_var dynamic_cast, if des_var is null, notify self
 * */
#define DYNAMIC_CAST_NOTIFY(TYPE, DES_VAR, SRC_VAR, RETURN) 	\
	DYNAMIC_CAST(TYPE, DES_VAR, SRC_VAR); \
	CONDITION_NOTIFY_RETURN(DES_VAR != NULL, RETURN, ERROR_CLIENT_OPERATE)

/*
 * msg dynamic_cast, if des_var is null, notify self
 * */
#define MSG_DYNAMIC_CAST_NOTIFY(TYPE, DES_VAR, RETURN) 	\
		DYNAMIC_CAST_NOTIFY(TYPE, DES_VAR, msg, RETURN)


#define CHECK_TRANSACTION_ERROR(P_TRANS, RECOGN) \
do { \
    if (P_TRANS == 0) return -1; \
    if (P_TRANS->detail().__error != 0) \
    { \
        this->respond_to_client_error(RECOGN, P_TRANS->detail().__error); \
        P_TRANS->rollback(); \
        return -1; \
    } \
} while(0)

// 检查是否满足兑换，进行兑换或通知客户端确认兑换 金钱不足会让函数在此返回 要保证NEED_MONEY为需要扣的所有金钱
// 最好保证是操作最后一个验证，否则后续验证失败了，可能会兑换了铜币
#define PROCESS_BIND_COPPER_ADJUST_EXCHANGE_AUTO_REPEAT(NEED_MONEY, RECOGN, MSG) \
do {\
	int ret = this->bind_copper_exchange_or_notify(NEED_MONEY, RECOGN, MSG);\
	CONDITION_NOTIFY_RETURN(ret >= 0, RECOGN, ret);\
	JUDGE_RETURN(0 == ret, 0);\
} while(0)

#define PROCESS_AUTO_BIND_COPPER_EXCHANGE_AUTO_REPEAT(NEED_MONEY, RECOGN, AUTO_BUY, MSG) \
do {\
	int ret = this->bind_copper_exchange_or_notify(NEED_MONEY, RECOGN, MSG, AUTO_BUY);\
	CONDITION_NOTIFY_RETURN(ret >= 0, RECOGN, ret);\
	JUDGE_RETURN(0 == ret, 0);\
} while(0)

#define PROCESS_BIND_COPPER_ADJUST_EXCHANGE(NEED_MONEY, RECOGN) \
do {\
	int ret = this->bind_copper_exchange_or_notify(NEED_MONEY, RECOGN);\
	CONDITION_NOTIFY_RETURN(ret >= 0, RECOGN, ret);\
	JUDGE_RETURN(0 == ret, 0);\
} while(0)

#define PROCESS_BIND_COPPER_ADJUST_EXCHANGE_WITH_ERROR(NEED_MONEY, RECOGN) \
do {\
	int ret = this->bind_copper_exchange_or_notify(NEED_MONEY, RECOGN);\
	CONDITION_NOTIFY_RETURN(ret >= 0, RECOGN, ret);\
	CONDITION_NOTIFY_RETURN(ret == 0, RECOGN, ERROR_PACKAGE_COPPER_AMOUNT);\
} while(0)

struct SubObj
{
	SubObj(int val1 = 0, int val2 = 0, int val3 = 0)
	{
		this->val1_ = val1;
		this->val2_ = val2;
		this->val3_ = val3;
	}

	int val1_;
	int val2_;
	int val3_;
};

/*
 * two sort item
 */
struct PairObj
{
	PairObj(Int64 id = 0, int value = 0)
	{
		this->id_ = id;
		this->value_ = value;
	}

	Int64 id_;
	int value_;
};

/*
 * three sort item
 */
struct ThreeObj
{
	ThreeObj(Int64 id = 0, int value = 0, Int64 tick = 0)
	{
		this->id_ = id;
		this->value_ = value;
		this->tick_ = tick;
		this->sub_ = 0;
	}

	Int64 id_;

	int value_;
	int sub_;
	IntMap sub_map_;

	Int64 tick_;
	string name_;
};

/*
 * four sort item
 * */
struct FourObj
{
	FourObj(Int64 id = 0, Int64 tick = 0,
			int first_value = 0, int second_value = 0)
	{
		this->id_ = id;
		this->tick_ = tick;

		this->first_value_ = first_value;
		this->second_value_ = second_value;

		this->first_id_ = 0;
		this->second_id_ = 0;
	}

	Int64 id_;
	Int64 tick_;

	int first_value_;
	int second_value_;

	Int64 first_id_;
	Int64 second_id_;
};

typedef std::vector<PairObj> 		PairObjVec;
typedef std::vector<ThreeObj> 		ThreeObjVec;
typedef std::vector<FourObj> 		FourObjVec;

typedef std::list<PairObj>			PairObjList;
typedef std::list<ThreeObj>			ThreeObjList;

typedef std::map<Int64, ThreeObj> 	ThreeObjMap;

struct ArrTimeItem
{
	Int64 arr_tick_;
	virtual ~ArrTimeItem() {}
};

class MoverCoord
{
public:
	MoverCoord(int pos_x = 0, int pos_y = 0);
    void reset(void);

    int pos_x(void) const;
    int pos_y(void) const;
    int pixel_x(void) const;
    int pixel_y(void) const;

    void serialize(ProtoCoord* proto_coord) const ;
    void unserialize(ProtoCoord* proto_coord);

    Int64 coord_index();

    void set_pos(const int pos_x, const int pos_y,
    		const int cell_width = 30, const int cell_height = 30);
    void set_pixel(const int pixel_x, const int pixel_y,
    		const int cell_width = 30, const int cell_height = 30);

    static int pixel_to_pos(const int pixel, const int cell = 30);
    static int pos_to_pixel(const int pos, const int cell = 30);

protected:
    int __pos_x;
    int __pos_y;
    int __pixel_x;
    int __pixel_y;
};

typedef std::vector<MoverCoord> CoordVec;

struct CreateDaysInfo
{
	bool init_;
	Int64 create_tick_;		//创建时间
	int first_left_sec_;	//第一天剩余秒数

	CreateDaysInfo();

	void reset();
	void set_tick(time_t create_tick);

	int passed_days();	//从1开始
	Int64 passed_time();
	Int64 create_day_tick();
};

#endif /* GAMEHEADER_H_ */
