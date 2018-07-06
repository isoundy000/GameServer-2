/*
 * Transaction.cpp
 *
 * Created on: 2013-01-26 14:17
 *     Author: glendy
 */

#include "MongoDataMap.h"
#include "Transaction.h"
#include "BaseUnit.h"
#include "PoolMonitor.h"
#include "TransactionMonitor.h"
#include "LogicPlayer.h"
#include "MapPlayerEx.h"
#include "MapLogicPlayer.h"
#include "GatePlayer.h"
#include "LeagueChannel.h"
#include "PrivateChat.h"
#include "LoudSpeaker.h"
#include "ChannelAgency.h"

void TransactionData::reset(void)
{
    ::memset(this, 0, sizeof(TransactionData));
}

void TransactionDetail::reset(void)
{
    this->__trans_id = 0;
    this->__scene_id = 0;
    this->__interval_tick = Time_Value::zero;
    this->__check_tick = Time_Value::zero;
    this->__src_recogn = 0;
    this->__res_recogn = 0;
    this->__unit = 0;
    this->__trans_status = 0;
    this->__role_id = 0;
    this->__sid = 0;
    this->__error = 0;
    this->__data_map.clear();
}

Transaction::~Transaction(void)
{ /*NULL*/ }

void Transaction::reset(void)
{
    this->trans_detail_.reset();
}

int Transaction::trans_id(void)
{
    return this->trans_detail_.__trans_id;
}

int Transaction::scene_id(void)
{
    return this->trans_detail_.__scene_id;
}

BaseUnit *Transaction::unit(void)
{
    return this->trans_detail_.__unit;
}

Time_Value &Transaction::check_tick(void)
{
    return this->trans_detail_.__check_tick;
}

Time_Value &Transaction::interval_tick(void)
{
    return this->trans_detail_.__interval_tick;
}

int Transaction::src_recogn(void)
{
    return this->trans_detail_.__src_recogn;
}

int Transaction::res_recogn(void)
{
    return this->trans_detail_.__res_recogn;
}

int Transaction::sid(void)
{
    return this->trans_detail_.__sid;
}

int64_t Transaction::role_id(void)
{
    return this->trans_detail_.__role_id;
}

int Transaction::status(void)
{
    return this->trans_detail_.__trans_status;
}

void Transaction::update_status(const int status)
{
    this->trans_detail_.__trans_status = status;
    if (status == Transaction::TRANS_FAILURE)
    	this->trans_detail_.__check_tick = Time_Value::gettimeofday() + Time_Value(3);
}

bool Transaction::is_processing(void)
{
    return this->trans_detail_.__trans_status == TRANS_PROCESSING;
}

bool Transaction::is_failure(void)
{
    return this->trans_detail_.__trans_status == TRANS_FAILURE;
}

bool Transaction::is_success(void)
{
    return this->trans_detail_.__trans_status == TRANS_SUCCESS;
}

bool Transaction::is_summit(void)
{
    return this->trans_detail_.__trans_status == TRANS_SUMMIT;
}

bool Transaction::is_rollback(void)
{
    return this->trans_detail_.__trans_status == TRANS_ROLLBACK;
}

TransactionDetail &Transaction::detail(void)
{
    return this->trans_detail_;
}

void Transaction::update_tick(void)
{
    this->trans_detail_.__check_tick = Time_Value::gettimeofday() + this->trans_detail_.__interval_tick;
}

int Transaction::push_data(const TransactionData &data)
{
    this->trans_detail_.__data_map[data.__data_type] = data;
    return 0;
}

int Transaction::push_data(const int type, const void *data, const void *data_pool)
{
    if (data == 0)
    {
        MSG_USER("ERROR data null");
        return -1;
    }

    TransactionData trans_data;
    trans_data.__data_type = type;
    switch (type)
    {
        case DB_BLOCK_BUFF:
        {
            trans_data.__data.__block_buff = (Block_Buffer *)data;
            trans_data.__data_pool.__buff_pool = (Block_Pool_Group *)data_pool;
            return this->push_data(trans_data);
            break;
        }
        case DB_LOGIC_PLAYER:
        {
            trans_data.__data.__logic_player = (LogicPlayer *)data;
            trans_data.__data_pool.__logic_player_pool = (ObjectPoolEx<LogicPlayer> *)data_pool;
            return this->push_data(trans_data);
            break;
        }
        case DB_MAP_PLAYER:
        {
            trans_data.__data.__map_player = (MapPlayerEx *)data;
            trans_data.__data_pool.__map_player_pool = (ObjectPoolEx<MapPlayerEx> *)data_pool;
            return this->push_data(trans_data);
            break;
        }
        case DB_GATE_PLAYER:
        {
            trans_data.__data.__gate_player = (GatePlayer *)data;
            trans_data.__data_pool.__gate_player_pool = (ObjectPoolEx<GatePlayer> *)data_pool;
            return this->push_data(trans_data);
            break;
        }
        case DB_MAP_LOGIC_PLAYER:
        {
            trans_data.__data.__map_logic_player = (MapLogicPlayer *)data;
            trans_data.__data_pool.__map_logic_player_pool = (ObjectPoolEx<MapLogicPlayer> *)data_pool;
            return this->push_data(trans_data);
            break;
        }
        case DB_CHAT_LEAGUE:
        {
        	trans_data.__data.__league_channel = (LeagueChannel *)data;
        	trans_data.__data_pool.__league_channel_pool = (ObjectPoolEx<LeagueChannel> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_CHAT_LOUDSPEAKER:
        {
        	trans_data.__data.__loudspeaker = (LoudSpeaker *)data;
        	trans_data.__data_pool.__loud_speaker_pool = (ObjectPoolEx<LoudSpeaker> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_CHAT_PRIVATE:
        {
        	trans_data.__data.__privatechat = (PrivateChat *)data;
        	trans_data.__data_pool.__private_chat_pool = (ObjectPoolEx<PrivateChat> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_CHAT_AGENCY:
        {
        	trans_data.__data.__channel_agency= (ChannelAgency *)data;
        	trans_data.__data_pool.__channel_agency_pool = (ObjectPoolEx<ChannelAgency> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_TRADE_MODE:
        {
        	trans_data.__data.__trade_mode= (DBTradeMode *)data;
        	trans_data.__data_pool.__trade_mode_pool = (ObjectPoolEx<DBTradeMode> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_MAIL_INFO:
        {
        	trans_data.__data.__mail_info = (MailInformation *)data;
        	trans_data.__data_pool.__mail_info_pool = (ObjectPoolEx<MailInformation> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_MAIL_BOX:
        {
        	trans_data.__data.__mail_box = (MailBox *)data;
        	trans_data.__data_pool.__mail_box_pool = (ObjectPoolEx<MailBox> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_MONGO_DATA_MAP:
        {
            trans_data.__data.__mongo_data_map = (MongoDataMap *)data;
            trans_data.__data_pool.__mongo_data_map_pool = (ObjectPoolEx<MongoDataMap> *)data_pool;
            return this->push_data(trans_data);
        }
        case DB_SHOP_LOAD_MODE:
        {
            trans_data.__data.__shop_mode = (DBShopMode *)data;
            trans_data.__data_pool.__shop_mode_pool = (ObjectPoolEx<DBShopMode> *)data_pool;
            return this->push_data(trans_data);
        }
        case DB_FRIEND_INFO_DETAIL:
        {
        	trans_data.__data.__friend_info_detail = (DBFriendInfo *)data;
        	trans_data.__data_pool.__friend_info_detail_pool = (ObjectPoolEx<DBFriendInfo> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_SCRIPT_PLAYER_REL:
        {
            trans_data.__data.__script_player_rel = (ScriptPlayerRel *)data;
            trans_data.__data_pool.__script_playerrel_pool = (ObjectPoolEx<ScriptPlayerRel> *)data_pool;
            return this->push_data(trans_data);
        }
        case DB_RPM_RECOMAND_INFO:
        {
        	trans_data.__data.__rpm_recomand_info = (RpmRecomandInfo*)data;
        	trans_data.__data_pool.__rpm_recomand_info_pool = (ObjectPoolEx<RpmRecomandInfo> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_BACK_RECHARGE_ORDER:
        {
        	trans_data.__data.__back_recharge_order = (RechargeOrder*)data;
        	trans_data.__data_pool.__back_recharge_order_pool = (ObjectPoolEx<RechargeOrder> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_BACK_ACTI_CODE:
        {
        	trans_data.__data.__acti_code_detail = (ActiCodeDetail*)data;
        	trans_data.__data_pool.__acti_code_detail_pool = (ObjectPoolEx<ActiCodeDetail> *)data_pool;
        	return this->push_data(trans_data);
        }
        case DB_RECHARGE_TIME_RANK:
        {
        	trans_data.__data.__recharge_time_rank_detail = (RechargeTimeRankDetail*)data;
        	return this->push_data(trans_data);
        }

        default:
            MSG_USER("ERROR push error data type %d", type);
            return -1;
    }
    return 0;
}

TransactionData *Transaction::fetch_data(const int data_type)
{
    TransactionDetail::DataMap::iterator iter = this->trans_detail_.__data_map.find(data_type);
    if (iter == this->trans_detail_.__data_map.end())
        return 0;
    return &(iter->second);
}

MongoDataMap *Transaction::fetch_mongo_data_map(void)
{
	TransactionData *trans_data = this->fetch_data(DB_MONGO_DATA_MAP);
	if (trans_data != 0)
		return trans_data->__data.__mongo_data_map;
	return 0;
}

int Transaction::begin(BaseUnit *trans_unit)
{
    this->update_status(TRANS_PROCESSING);
    this->update_tick();
    if (TRANSACTION_MONITOR->bind_transaction(this) != 0)
    {
    	MSG_USER("ERROR bind transaction %d %d", this->detail().__src_recogn, this->detail().__res_recogn);
        return -1;
    }

    if (trans_unit != 0)
    {
        UnitMessage unit_msg;
        unit_msg.__type = UnitMessage::TYPE_IVALUE;
        unit_msg.__msg_head.__recogn = this->res_recogn();
        unit_msg.__msg_head.__role_id = this->role_id();
        unit_msg.__msg_head.__trans_id = this->trans_id();
        unit_msg.__data.__i_val = this->trans_id();
        return trans_unit->push_request(unit_msg);
    }
    return 0;
}

int Transaction::summit(void)
{
    TRANSACTION_MONITOR->unbind_transaction(this);

    this->update_status(TRANS_SUMMIT);
    this->recycle_self();

    return 0;
}

int Transaction::rollback(void)
{
    TRANSACTION_MONITOR->unbind_transaction(this);
    if (this->process_rollback() != 0)
    {
    	MSG_USER("rollback failure %d %d", this->detail().__src_recogn, this->detail().__res_recogn);

        this->update_tick();
        TRANSACTION_MONITOR->bind_transaction(this);
        return -1;
    }

    this->update_status(TRANS_ROLLBACK);
    this->recycle_self();
    return 0;
}

int Transaction::respond_source_unit(void)
{
    if (this->unit() == 0)
        return 0;

    this->update_tick();
    UnitMessage unit_msg;

    unit_msg.__type = UnitMessage::TYPE_IVALUE;
    unit_msg.__sid = this->sid();
    unit_msg.__msg_head.__recogn = this->res_recogn();
    unit_msg.__msg_head.__role_id = this->role_id();
    unit_msg.__msg_head.__trans_id = this->trans_id();
    unit_msg.__data.__i_val = this->trans_id();
    return this->unit()->push_request(unit_msg);

}

void Transaction::recycle_self(void)
{
    this->recycle_data();
    POOL_MONITOR->transaction_pool()->push(this);
}

void Transaction::recycle_data(void)
{
    TransactionDetail::DataMap &data_map = this->trans_detail_.__data_map;
    for (TransactionDetail::DataMap::iterator iter = data_map.begin();
            iter != data_map.end(); ++iter)
    {
        TransactionData &data = iter->second;
        switch (data.__data_type)
        {
            case DB_BLOCK_BUFF:
            {
                if (data.__data.__block_buff != 0 && data.__data_pool.__buff_pool != 0)
                    data.__data_pool.__buff_pool->push_block(0, data.__data.__block_buff);

                data.__data.__block_buff = 0;
                data.__data_pool.__buff_pool = 0;
                break;
            }
            case DB_LOGIC_PLAYER:
            {
                if (data.__data.__logic_player != 0 && data.__data_pool.__logic_player_pool != 0)
                    data.__data_pool.__logic_player_pool->push(data.__data.__logic_player);

                data.__data.__logic_player = 0;
                data.__data_pool.__logic_player_pool = 0;
                break;
            }
            case DB_MAP_PLAYER:
            {
                if (data.__data.__map_player != 0 && data.__data_pool.__map_player_pool != 0)
                    data.__data_pool.__map_player_pool->push(data.__data.__map_player);

                data.__data.__map_player = 0;
                data.__data_pool.__map_player_pool = 0;
                break;
            }
            case DB_GATE_PLAYER:
            {
                if (data.__data.__gate_player != 0 && data.__data_pool.__gate_player_pool != 0)
                    data.__data_pool.__gate_player_pool->push(data.__data.__gate_player);

                data.__data.__gate_player = 0;
                data.__data_pool.__gate_player_pool = 0;
                break;
            }
            case DB_MAP_LOGIC_PLAYER:
            {
                if (data.__data.__map_logic_player != 0 && data.__data_pool.__map_logic_player_pool != 0)
                    data.__data_pool.__map_logic_player_pool->push(data.__data.__map_logic_player);

                data.__data.__map_logic_player = 0;
                data.__data_pool.__map_logic_player_pool = 0;
                break;
            }
            case DB_MAIL_INFO:
            {
            	if (data.__data.__mail_info != 0 && data.__data_pool.__mail_info_pool != 0)
            		data.__data_pool.__mail_info_pool->push(data.__data.__mail_info);
            	data.__data.__mail_info = 0;
            	data.__data_pool.__mail_info_pool = 0;
            	break;
            }
            case DB_MAIL_BOX:
            {
            	if (data.__data.__mail_box != 0 && data.__data_pool.__mail_info_pool != 0)
            		data.__data_pool.__mail_box_pool->push(data.__data.__mail_box);
            	data.__data.__mail_box = 0;
            	data.__data_pool.__mail_box_pool = 0;
            	break;
            }
            case DB_MONGO_DATA_MAP:
            {
                if (data.__data.__mongo_data_map != 0 && data.__data_pool.__mongo_data_map_pool != 0)
                    data.__data_pool.__mongo_data_map_pool->push(data.__data.__mongo_data_map);
                data.__data.__mongo_data_map = 0;
                data.__data_pool.__mongo_data_map_pool = 0;
                break;
            }
            case DB_TRADE_MODE:
            {
            	if (data.__data.__trade_mode != 0 && data.__data_pool.__trade_mode_pool != 0)
            	{
                    data.__data_pool.__trade_mode_pool->push(data.__data.__trade_mode);
            	}

                data.__data.__trade_mode = 0;
                data.__data_pool.__trade_mode_pool = 0;
            	break;
            }
            case DB_SHOP_LOAD_MODE:
            {
            	if (data.__data.__shop_mode != 0 && data.__data_pool.__shop_mode_pool != 0)
            	{
                    data.__data_pool.__shop_mode_pool->push(data.__data.__shop_mode);
            	}

                data.__data.__shop_mode = 0;
                data.__data_pool.__shop_mode_pool = 0;
            	break;
            }
            case DB_FRIEND_INFO_DETAIL:
            {
            	if(data.__data.__friend_info_detail != 0 && data.__data_pool.__friend_info_detail_pool != 0)
            	{
            		data.__data_pool.__friend_info_detail_pool->push(data.__data.__friend_info_detail);
            	}

            	data.__data.__friend_info_detail = 0;
            	data.__data_pool.__friend_info_detail_pool = 0;
            	break;
            }
            case DB_MALL_ITEM_DETAIL:
            {
            	if(data.__data.__shop_item != 0 && data.__data_pool.__shop_item_pool != 0)
            	{
            		data.__data_pool.__shop_item_pool->push(data.__data.__shop_item);
            	}

            	data.__data.__shop_item = 0;
            	data.__data_pool.__shop_item_pool = 0;
            	break;
            }
            case DB_SCRIPT_PLAYER_REL:
            {
                if (data.__data.__script_player_rel != 0 && data.__data_pool.__script_playerrel_pool != 0)
                    data.__data_pool.__script_playerrel_pool->push(data.__data.__script_player_rel);
                data.__data.__script_player_rel = 0;
                data.__data_pool.__script_playerrel_pool = 0;
                break;
            }
            case DB_RPM_RECOMAND_INFO:
            {
            	if(data.__data.__rpm_recomand_info != 0 && data.__data_pool.__rpm_recomand_info_pool != 0)
            		data.__data_pool.__rpm_recomand_info_pool->push(data.__data.__rpm_recomand_info);

				data.__data.__rpm_recomand_info = 0;
				data.__data_pool.__rpm_recomand_info_pool = 0;
            	break;
            }
            case DB_BACK_RECHARGE_ORDER:
            {
            	if(data.__data.__back_recharge_order != 0 && data.__data_pool.__back_recharge_order_pool != 0)
            		data.__data_pool.__back_recharge_order_pool->push(data.__data.__back_recharge_order);

            	data.__data.__back_recharge_order = 0;
            	data.__data_pool.__back_recharge_order_pool = 0;
            	break;
            }
            case DB_BACK_ACTI_CODE:
            {
            	if(data.__data.__acti_code_detail != 0 && data.__data_pool.__acti_code_detail_pool != 0 )
            		data.__data_pool.__acti_code_detail_pool->push(data.__data.__acti_code_detail);

            	data.__data.__acti_code_detail = 0;
            	data.__data_pool.__acti_code_detail_pool = 0;
            	break;
            }

            case DB_CHAT_LEAGUE:
            case DB_CHAT_LOUDSPEAKER:
            case DB_CHAT_PRIVATE:
            case DB_CHAT_AGENCY:
            case DB_RECHARGE_TIME_RANK:
                // no need to push back pool;
                break;
            default:
            {
                if (data.__data_type != 0)
                    MSG_USER("ERROR can't reconigze transaction data type %d", data.__data_type);
                break;
            }
        }
    }
}

int Transaction::process_rollback(void)
{
	// TODO;
    return 0;
}

