/*
 * MongoUnit.cpp
 *
 * Created on: 2013-01-25 16:46
 *     Author: glendy
 */

#include "MongoUnit.h"
#include "Transaction.h"
#include "PoolMonitor.h"
#include "MongoConnector.h"
#include "TransactionMonitor.h"
#include "MMOChatLeague.h"
#include "MMOChatPrivate.h"
#include "MMORole.h"
#include "MMOMarket.h"
#include "MapStruct.h"
#include "ChannelAgency.h"
#include "MMOBeast.h"
#include "MMORankPannel.h"
#include "MMOPackage.h"
#include "BackFlowControl.h"
#include "BackBrocast.h"
#include "BackSceneLine.h"
#include "BackMediaGift.h"
#include "MMORechargeRankConfig.h"

MongoUnit::MongoUnit(void)
{
    this->conn_ = new MongoConnector();
}

int MongoUnit::type(void)
{
    return MONGO_UNIT;
}

UnitMessage *MongoUnit::pop_unit_message(void)
{
    return POOL_MONITOR->unit_msg_pool()->pop();
}

int MongoUnit::push_unit_message(UnitMessage *msg)
{
    return POOL_MONITOR->unit_msg_pool()->push(msg);
}

int MongoUnit::process_block(UnitMessage *unit_msg)
{
    int32_t trans_id = unit_msg->__msg_head.__trans_id;

    Transaction *trans = 0;
    if (TRANSACTION_MONITOR->find_transaction(trans_id, trans) != 0)
        return -1;
   
    // here can't use submit, rollback function;
    int32_t recogn = trans->res_recogn(), ret = 0,
    		is_submit_trans = 0; // 是否立即在当前线程回收数据库事务对象（不需要回调的事务）
    switch (recogn)
    {
        case TRANS_LOGOUT_MAP_PLAYER:
        case TRANS_LOGOUT_MAP_LOGIC_PLAYER:
        case TRANS_SAVE_MAP_PLAYER:
        case TRANS_SAVE_MAP_LOGIC_PLAYER:
        {
        	is_submit_trans = 1;	// 立即回收事务对象, 下面是不立即回收的
        	TransactionData *trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
        	ret = this->conn_->check_role_auto_run_mongo(trans->detail().__role_id,
        			trans_data->__data.__mongo_data_map);
			break;
        }
        case TRANS_LOGOUT_LOGIC_PLAYER:
        case TRANS_SAVE_LOGIC_PLAYER:
        case TRANS_MARKET_SAVE:
        case TRANS_TASK_SAVE:
        case TRANS_UPDATE_GLOBAL_KEY:
        case TRANS_SAVE_RANK_PANEL_DATA:
        case TRANS_SAVE_PLAYER_RANK_BEAST_DETAIL:
        case TRANS_UPDATE_SCRIPT_PROGRESS:
        case TRANS_UPDATE_SCRIPT_HISTORY:
        case TRANS_UPDATE_OFFLINE_TEAM_INFO:
        case TRANS_UPDATE_BACK_BROCAST_INFO:
        case TRANS_UPDATE_BACK_RECHARGE_ORDER_FLAG:
        case TRANS_UPDATE_BACK_MAIL_REQUEST_FLAG:
        case TRANS_UPDATE_CUSTOMER_SVC_RECORD:
        case TRANS_UPDATE_ACTI_CODE_DETAIL:
        case TRANS_SAVE_PLAYER_SCRIPT_ZYFM:
        case TRANS_MARK_RESTRICTION_INFO:
        case TRANS_SAVE_BAN_IP_INFO:
        case TRANS_REMOVE_BAN_IP_INFO:
        case TRANS_SAVE_COPPER_BOWL_RANK:
        case TRANS_SAVE_RANK_LAST_TICK:
        case TRANS_SAVE_INVEST_SYSTEM:
        case TRANS_SAVE_PEAK_WAR_RANK:
        case TRANS_DELETE_LOAD_MAIL_OFFLINE:
        case TRANS_SAVE_WEDDING_INFO:
        case TRANS_REMOVE_WEDDING_INFO:
        case TRANS_SAVE_LSIEGE_INFO:
        case TRANS_SAVE_SYSTEM_TIPS:
        case TRANS_SAVE_BROTHER_INFO:
        case TRANS_TRAVEL_SAVE_LOGIC_BEAST:
        case TRANS_TRAVEL_SAVE_CUREENT_BEAST:
        case TRANS_SAVE_LOCAL_TRAV_TEAM:
        case TRANS_DEL_LOCAL_TRAV_TEAM:
        case TRANS_SAVE_REMOTE_TRAV_TEAM:
        case TRANS_DEL_REMOTE_TRAV_TEAM:
        case TRANS_SAVE_COPY_TRAV_TEAMER:
        case TRANS_SAVE_TRAVEL_PEAK_KNOCK:
        case TRANS_SAVE_TRAVEL_PEAK_PROMOT:
        case TRANS_SAVE_TRAVPEAK_WORTH_RANK:
        case TRANS_SAVE_TRAVPEAK_BET:
        case TRANS_SAVE_STRENGTH_GET:
        case TRANS_SAVE_BAN_MAC_INFO:
        case TRANS_REMOVE_BAN_MAC_INFO:
        case TRANS_LOAD_BACK_ACTIVITY_UPDATE_FLAG:
        {
        	is_submit_trans = 1;	// 立即回收事务对象, 下面是不立即回收的
        	TransactionData *trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
			ret = this->conn_->auto_run_mongo(trans_data->__data.__mongo_data_map);
			break;
        }
        case TRANS_LOAD_BACK_CUSTOMER_SVC_RECORD:
        case TRANS_LOAD_BACK_MAIL_REQUEST:
        case TRANS_LOAD_BACK_RECHARGE_ORDER:
        case TRANS_LOAD_SINGLE_PLAYER_ALL_INFO:
        case TRANS_LOAD_MASTER_OFFLINE:
        case TRANS_LOAD_RANKER_INFO:
        case TRANS_LOAD_RANK_PET_INFO:
        case TRANS_LOAD_RANK_MOUNT_INFO:
        case TRANS_LOAD_RANKER_PET_MOUNT_INFO:
        case TRANS_LOAD_RANKER_WING_INFO:
        	//rank pannel data
        case TRANS_LOAD_FIGHT_FORCE_RANK_DATA:
        case TRANS_LOAD_KILL_VALUE_RANK_DATA:
        case TRANS_LOAD_HERO_RANK_DATA:
        case TRANS_LOAD_FIGHT_LEVEL_RANK_DATA:
        case TRANS_LOAD_PET_RANK_DATA:
        case TRANS_LOAD_MOUNT_RANK_DATA:
        case TRANS_LOAD_SCRIPT_ZYFM_RANK_DATA:
        case TRANS_SAVE_PLAYER_RANK_WING:
        case TRANS_LOAD_DAILY_RECHARGE_OPEN_TIME:
        case TRANS_LOAD_RESTRICTION_INFO:
        case TRANS_LOAD_GAME_SWITCHER_INFO:
        case TRANS_LOAD_SEND_FLOWER_RANK:
        case TRANS_LOAD_RECV_FLOWER_RANK:
        case TRANS_GET_GETSIEGE_LEADER:
        case TRANS_LOAD_GAME_MODIFY_INFO:
        case TRANS_RANK_HIDE_ROLE:
        case TRANS_RESET_ACT_FLOWER_RANK:
        case TRANS_LOAD_RANK_FUN_INFO:
        case TRANS_LOAD_BACK_ACTIVITY_UPDATE:
        case TRANS_CORRECT_TRVL_RANK:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
			ret = this->conn_->auto_run_mongo(trans_data->__data.__mongo_data_map);
			break;
        }
        case TRANS_LOAD_GATE_PLAYER_PHP:
        case TRANS_LOAD_GATE_PLAYER:
		{
			TransactionData *trans_data = trans->fetch_data(DB_GATE_PLAYER);
			ret = this->conn_->load_gate_player(trans_data->__data.__gate_player);
			break;
		}
		case TRANS_SAVE_NEW_ROLE:
		{
			TransactionData *trans_data = trans->fetch_data(DB_GATE_PLAYER);
			ret = this->conn_->save_new_role(trans_data->__data.__gate_player);
			break;
		}
		case TRANS_LOAD_LOGIC_PLAYER:
		{
			TransactionData *trans_data = trans->fetch_data(DB_LOGIC_PLAYER);
			ret = this->conn_->load_player(trans_data->__data.__logic_player);
			break;
		}
        case TRANS_LOAD_MAP_PLAYER:
        {
            TransactionData *trans_data = trans->fetch_data(DB_MAP_PLAYER);
            ret = this->conn_->load_player(trans_data->__data.__map_player);
            break;
        }
        case TRANS_LOAD_MAP_LOGIC_PLAYER:
        {
            TransactionData *trans_data = trans->fetch_data(DB_MAP_LOGIC_PLAYER);
            ret = this->conn_->load_player(trans_data->__data.__map_logic_player);
            break;
        }
        case TRANS_CHAT_LOAD_LEAGUE_HISTORY:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_CHAT_LEAGUE);
        	this->conn_->chat_league()->load_league_record(trans_data->__data.__league_channel);
        	break;
        }
        case TRANS_CHAT_SAVE_LEAGUE_HISTORY:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_CHAT_LEAGUE);
        	this->conn_->chat_league()->save_league_record(trans_data->__data.__league_channel);
        	break;
        }
        case TRANS_CHAT_LOAD_PRIVATE_HISTORY:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_CHAT_PRIVATE);
        	this->conn_->chat_private()->load_private_record(trans_data->__data.__privatechat);
        	break;
        }
        case TRANS_CHAT_SAVE_PRIVATE_HISTORY:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_CHAT_PRIVATE);
        	this->conn_->chat_private()->save_private_record(trans_data->__data.__privatechat);
        	break;
        }
        case TRANS_CHAT_SAVE_VOICE_ID:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_CHAT_AGENCY);
        	this->conn_->save_global_voice_id(trans_data->__data.__channel_agency->max_voice_id());
//        	this->conn_->chat_private()->load_all_record(trans_data->__data.__channel_agency);
        	is_submit_trans = 1;
        	break;
        }
        case TRANS_CHECK_LOAD_BACK_FLOW_CONTROL:
        {
        	this->conn_->back_flow_control()->request_load_flow_detail();
        	is_submit_trans = 1;
        	break;
        }
        case TRANS_CHECK_LOAD_BACK_BRO_CONTROL:
        {
        	this->conn_->back_brocast()->request_load_data();
        	break;
        }
        case TRANS_LOAD_SHOP_MODE:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_SHOP_LOAD_MODE);
        	this->conn_->load_mmo_db(trans_data->__data.__shop_mode);
        	break;
        }
        case TRANS_TRADE_MODE_DEFAULT:
        case TRANS_SAVE_MAIL_OFFLINE:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_TRADE_MODE);
        	this->conn_->request_save_mmo_done(recogn, trans_data->__data.__trade_mode);
        	is_submit_trans = 1;
        	break;
        }
        case TRANS_TRADE_MODE_REMOVE:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_TRADE_MODE);
        	this->conn_->request_remove_mmo_done(recogn, trans_data->__data.__trade_mode);
        	is_submit_trans = 1;
        	break;
        }
        case TRANS_LOAD_MAIL_OFFLINE:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_MAIL_BOX);
        	// 如果没有邮件，则不需要回调了
        	if (this->conn_->load_mail_offline(trans->detail().__role_id, trans_data->__data.__mail_box) < 0)
        	{
        		return trans->rollback();
        	}
        	break;
        }
        case TRANS_LOAD_LOGIC_SOCIALER_INFO:
        case TRANS_LOAD_LOGIC_APPLY_INFO:
        {
            TransactionData *trans_data = trans->fetch_data(DB_FRIEND_INFO_DETAIL);
            this->conn_->mmo_role()->load_offline_friend_info(trans_data->__data.__friend_info_detail);
            break;
        }
        case TRANS_LOAD_SINGLE_LOGIC_SOCIALER_INFO:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_FRIEND_INFO_DETAIL);
        	this->conn_->mmo_role()->load_offline_friend_info(trans_data->__data.__friend_info_detail);
        	break;
        }
        case TRANS_LOAD_PLAYER_TO_SEND_MAIL:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_MAIL_INFO);
        	this->conn_->mmo_role()->load_player_to_send_mail(trans_data->__data.__mail_info);
        	break;
        }
        case TRANS_UPDATE_PLAYER_NAME:
        {
            ret = this->update_player_name(trans);
            break;
        }
        case TRANS_LOAD_SCRIPT_PROGRESS:
        {
            TransactionData *trans_data = trans->fetch_data(DB_SCRIPT_PLAYER_REL);
            ret = this->conn_->load_script_progress(trans_data->__data.__script_player_rel);
            break;
        }
        case TRANS_GET_RPM_RECOMAND_INFO:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_RPM_RECOMAND_INFO);
        	ret = this->conn_->get_rpm_recomand_info(trans_data->__data.__rpm_recomand_info);
        	break;
        }
        case TRANS_GET_RPM_INTORDUCTION_INFO:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_RPM_RECOMAND_INFO);
        	ret = this->conn_->get_rpm_introduction_info(trans_data->__data.__rpm_recomand_info);
        	break;
        }
        case TRANS_LOAD_SCENE_LINE_CONFIG:
        {
            TransactionData *trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
            ret = this->conn_->back_scene_line()->load_scene_line_config(trans_data->__data.__mongo_data_map);
            break;
        }
        case TRANS_FETCH_RAND_NAME:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
        	ret = this->conn_->mmo_role()->fetch_random_name(trans_data->__data.__mongo_data_map);
        	break;
        }
        case TRANS_SYNC_OTHER_ROLE_RECHARGE_INFO:
        {
        	TransactionData *trans_data = trans->fetch_data(DB_BACK_RECHARGE_ORDER);
        	ret = this->conn_->mmo_role()->sync_recharge_info_to_all_role(trans_data->__data.__back_recharge_order);
        	is_submit_trans = 1;
        	break;
        }
        case TRANS_UPDATE_SPECIAL_FB_ACHIEVEMENT:
        {
        	TransactionData* trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
        	this->conn_->update_special_FB_achievement(trans_data->__data.__mongo_data_map);
        	is_submit_trans = 1;
        	break;
        }
        case TRANS_FETCH_ACTI_CODE_DETAIL:
        {
        	TransactionData* trans_date = trans->fetch_data(DB_BACK_ACTI_CODE);
        	ret = this->conn_->back_media_gift()->load_acti_code(trans_date->__data.__acti_code_detail);
        	break;
        }
        case TRANS_LOAD_BACK_ACTIVITY_TICK:
        {
            TransactionData *trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
            ret = this->conn_->load_activity_tick(trans_data->__data.__mongo_data_map);
            break;
        }
        default:
        {
            MSG_USER("ERROR can't recognize mongo request %d %d", trans->src_recogn(), recogn);
            trans->update_status(Transaction::TRANS_FAILURE);
            return -1;
        }
    }

    if (ret != 0)
    {
    	trans->detail().__error = ret;
        trans->update_status(Transaction::TRANS_FAILURE);
    }

    if (trans->unit() == 0)
    {
    	is_submit_trans = 1;
    }

    if (is_submit_trans == 0)
    {
    	trans->respond_source_unit();
    }
    else
    {
    	// 默认是不立即提交，回调时才由调用者提交
    	trans->summit();
    }
    return 0;
}

int MongoUnit::process_stop(void)
{
	SAFE_DELETE(this->conn_);
	return BaseUnit::process_stop();
}

int MongoUnit::interval_run(void)
{
	return 0;
}

int MongoUnit::update_player_name(Transaction *trans)
{
    TransactionData *trans_data = trans->fetch_data(DB_MONGO_DATA_MAP);
    MongoDataMap *data_map = trans_data->__data.__mongo_data_map;

    return this->conn_->mmo_role()->update_player_name(data_map);
}

