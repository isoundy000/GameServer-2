/*
 * Transaction.h
 *
 * Created on: 2013-01-26 10:37
 *     Author: glendy
 */

#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include "PubStruct.h"

class LeagueChannel;
class LoudSpeaker;
class PrivateChat;
class ChannelAgency;
struct MailInformation;
struct MailBox;
class MongoDataMap;
class MarketSystem;
class ScriptPlayerRel;
class ActiCodeDetail;
class RechargeTimeRankDetail;

struct TransactionData
{
    int __data_type;
    union
    {
        Block_Buffer *__block_buff;
        LogicPlayer *__logic_player;
        MapPlayerEx *__map_player;
        GatePlayer *__gate_player;
        MapLogicPlayer *__map_logic_player;
        LeagueChannel *__league_channel;
        LoudSpeaker *__loudspeaker;
        PrivateChat *__privatechat;
        ChannelAgency *__channel_agency;
        DBShopMode *__shop_mode;
        DBTradeMode *__trade_mode;
        MailInformation *__mail_info;
        MailBox *__mail_box;
        MongoDataMap *__mongo_data_map;
        MarketSystem *__market;
        DBFriendInfo *__friend_info_detail;
        ShopItem *__shop_item;
        ScriptPlayerRel *__script_player_rel;
        RpmRecomandInfo *__rpm_recomand_info;
        RechargeOrder *__back_recharge_order;
        ActiCodeDetail* __acti_code_detail;
        RechargeTimeRankDetail* __recharge_time_rank_detail;
    } __data;
    union
    {
        Block_Pool_Group *__buff_pool;
        ObjectPoolEx<LogicPlayer> *__logic_player_pool;
        ObjectPoolEx<MapPlayerEx> *__map_player_pool;
        ObjectPoolEx<GatePlayer> *__gate_player_pool;
        ObjectPoolEx<MapLogicPlayer> *__map_logic_player_pool;
        ObjectPoolEx<LeagueChannel> *__league_channel_pool;
        ObjectPoolEx<LoudSpeaker> *__loud_speaker_pool;
        ObjectPoolEx<PrivateChat> *__private_chat_pool;
        ObjectPoolEx<ChannelAgency> *__channel_agency_pool;
        ObjectPoolEx<DBShopMode> *__shop_mode_pool;
        ObjectPoolEx<DBTradeMode> *__trade_mode_pool;
        ObjectPoolEx<MailInformation> *__mail_info_pool;
        ObjectPoolEx<MailBox> *__mail_box_pool;
        ObjectPoolEx<MongoDataMap> *__mongo_data_map_pool;
        ObjectPoolEx<MarketSystem> *__market_map_pool;
        ObjectPoolEx<DBFriendInfo> *__friend_info_detail_pool;
        ObjectPoolEx<ShopItem> *__shop_item_pool;
        ObjectPoolEx<ScriptPlayerRel> *__script_playerrel_pool;
        ObjectPoolEx<RpmRecomandInfo> * __rpm_recomand_info_pool;
        ObjectPoolEx<RechargeOrder> *__back_recharge_order_pool;
        ObjectPoolEx<ActiCodeDetail> *__acti_code_detail_pool;
    } __data_pool;

    void reset(void);
};

struct TransactionDetail
{
    int __trans_id;
    int __scene_id;
    Time_Value __interval_tick;
    Time_Value __check_tick;
    int __src_recogn;
    int __res_recogn;
    BaseUnit *__unit;
    int __trans_status;

    int __sid;
    int64_t __role_id;
    int __error;

    typedef boost::unordered_map<int, TransactionData> DataMap;
    DataMap __data_map;    // key: __data_type;

    void reset(void);
};

class Transaction
{
public:
    // transaction status {{{
    enum TRANS_STATUS
    {
        TRANS_NULL          = 0,
        TRANS_PROCESSING    = 1,
        TRANS_SUCCESS       = 2,
        TRANS_FAILURE       = 3,
        TRANS_ROLLBACK      = 4,
        TRANS_SUMMIT        = 5,
        TRANS_STATUS_END
    };
    // }}}
public:
    virtual ~Transaction(void);
    void reset(void);

    int trans_id(void);
    int scene_id(void);
    BaseUnit *unit(void);
    Time_Value &check_tick(void);
    Time_Value &interval_tick(void);
    int src_recogn(void);
    int res_recogn(void);
    int sid(void);
    int64_t role_id(void);

    int status(void);
    void update_status(const int status);
    bool is_processing(void);
    bool is_failure(void);
    bool is_success(void);
    bool is_summit(void);
    bool is_rollback(void);

    TransactionDetail &detail(void);
    void update_tick(void);

    int push_data(const TransactionData &data);
    int push_data(const int type, const void *data, const void *data_pool);
    TransactionData *fetch_data(const int data_type);
    MongoDataMap *fetch_mongo_data_map(void);

    virtual int begin(BaseUnit *trans_unit = 0);
    // if trans->__role_id != 0, can only use in logic unit;
    virtual int summit(void);
    // if trans->__role_id != 0, can only use in logic unit;
    virtual int rollback(void);
    virtual int respond_source_unit(void);

    virtual void recycle_self(void);

protected:
    virtual void recycle_data(void);
    virtual int process_rollback(void);
 
protected:
    TransactionDetail trans_detail_;
};

#endif //_TRANSACTION_H_
