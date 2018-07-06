/*
 * MongoConnector.cpp
 *
 * Created on: 2013-01-24 11:47
 *     Author: glendy
 */

#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MongoData.h"
#include "MMORole.h"
#include "MMOGlobal.h"
#include "MMOFight.h"
#include "MMOPackage.h"
#include "MMOSkill.h"
#include "MMOStatus.h"
#include "GatePlayer.h"
#include "LogicPlayer.h"
#include "MapPlayerEx.h"
#include "MapLogicPlayer.h"
#include "MMOSocialer.h"
#include "MMOOnline.h"
#include "MMOTrade.h"
#include "MMOMailOffline.h"
#include "MMOFriendshipValue.h"
#include "BackFlowControl.h"
#include "BackBrocast.h"
#include "BackMailRequest.h"
#include "MMOLeague.h"
#include "MMOShop.h"
#include "MMOBeast.h"
#include "MMOVip.h"
#include "MMOOnlineRewards.h"
#include "MMOOfflineRewards.h"
#include "MMOTreasures.h"
#include "MMOCollectChests.h"
#include "MMOEquipSmelt.h"
#include "MMOActivityTipsSystem.h"
#include "MMOChatLeague.h"
#include "MMOChatPrivate.h"
#include "MMOMarket.h"
#include "MMOWorldBoss.h"
#include "MMOLabel.h"
#include "MMOScript.h"
#include "MMORankPannel.h"
#include "MMOInvestSystem.h"
#include "MMOScriptProgress.h"
#include "MMOScriptClean.h"
#include "MMOPlayerTip.h"
#include "MMOScriptHistory.h"
#include "BackSceneLine.h"
#include "BackMediaGift.h"
#include "MMORechargeRankConfig.h"
#include "MMOMediaGift.h"
#include "MMOIllustration.h"
#include "MMOWedding.h"
#include "MMOMagicWeapon.h"
#include "MMOServerInfo.h"
#include "MMOOpenActivity.h"
#include "GameField.h"
#include "DaemonServer.h"
#include "MMOSwordPool.h"
#include "MMOEscort.h"
#include "MMOTravel.h"
#include "MMOLuckyWheel.h"
#include "MMOOfflineHook.h"
#include "BackJYBackActivity.h"
#include "MMOTravTeam.h"

#include <mongo/util/net/sock.h>
#include <mongo/client/dbclient.h>
using namespace mongo;

MongoConnector::MongoConnector(void) :
    conn_(0)
{
    this->mmo_role_ = new MMORole;
    this->mmo_role_->set_connection(this);

    this->mmo_role_ex_ = new MMORoleEx;
    this->mmo_role_ex_->set_connection(this);

    this->mmo_global_ = new MMOGlobal;
    this->mmo_global_->set_connection(this);

    this->mmo_fight_ = new MMOFight;
    this->mmo_fight_->set_connection(this);

    this->mmo_package_ = new MMOPackage;
    this->mmo_package_->set_connection(this);

    this->mmo_skill_ = new MMOSkill;
    this->mmo_skill_->set_connection(this);

    this->mmo_escort_ = new MMOEscort;
    this->mmo_escort_->set_connection(this);

    this->mmo_status_ = new MMOStatus;
    this->mmo_status_->set_connection(this);

    this->mmo_trade_ = new MMOTrade;
    this->mmo_trade_->set_connection(this);

    this->mmo_league_ = new MMOLeague;
    this->mmo_league_->set_connection(this);

    this->mmo_online_ = new MMOOnline;
    this->mmo_online_->set_connection(this);

    this->mmo_chat_league_ = new MMOChatLeague;
    this->mmo_chat_league_->set_connection(this);

    this->mmo_chat_private_ = new MMOChatPrivate;
    this->mmo_chat_private_->set_connection(this);

    this->mmo_mail_ = new MMOMail;
    this->mmo_mail_->set_connection(this);

    this->mmo_mail_offline_ = new MMOMailOffline;
    this->mmo_mail_offline_->set_connection(this);

    this->mmo_socialer_ = new MMOSocialer;
    this->mmo_socialer_->set_connection(this);

    this->mmo_task_ = new MMOTask;
    this->mmo_task_->set_connection(this);

    this->market_ = new MMOMarket;
    this->market_->set_connection(this);

    this->mmo_shop_ = new MMOShop;
    this->mmo_shop_->set_connection(this);

    this->mmo_beast_ = new MMOBeast;
    this->mmo_beast_->set_connection(this);

    this->back_serial_ = new BackSerial;
    this->back_serial_->set_connection(this);

    this->mmo_vip_ = new MMOVip();
    this->mmo_vip_->set_connection(this);

    this->mmo_activity_tips_ = new MMOActivityTips;
    this->mmo_activity_tips_->set_connection(this);

    this->mmo_online_rewards_ = new MMOOnlineRewards;
    this->mmo_online_rewards_->set_connection(this);

    this->mmo_offline_rewards_ = new MMOOfflineRewards;
    this->mmo_offline_rewards_->set_connection(this);

    this->mmo_treasures_info_ = new MMOTreasures;
    this->mmo_treasures_info_->set_connection(this);

    this->mmo_equip_smelt_ = new MMOEquipSmelt;
    this->mmo_equip_smelt_->set_connection(this);

    this->mmo_collect_chests_ = new MMOCollectChests;
    this->mmo_collect_chests_->set_connection(this);

    this->mmo_welfare_ = new MMOWelfare;
    this->mmo_welfare_->set_connection(this);

    this->mmo_exp_restore_ = new MMOExpRestore;
    this->mmo_exp_restore_->set_connection(this);

    this->mmo_label_ = new MMOLabel;
    this->mmo_label_->set_connection(this);

    this->mmo_illus_ = new MMOIllustration;
    this->mmo_illus_->set_connection(this);

    this->mmo_script_ = new MMOScript;
    this->mmo_script_->set_connection(this);

    this->mmo_achievement_ = new MMOAchievement;
    this->mmo_achievement_->set_connection(this);

    this->mmo_hidden_treasure_ = new MMOHiddenTreasure;
    this->mmo_hidden_treasure_->set_connection(this);

    this->mmo_fashion_ = new MMOFashion;
    this->mmo_fashion_->set_connection(this);

    this->mmo_transfer_ = new MMOTransfer;
    this->mmo_transfer_->set_connection(this);

    this->mmo_rank_pannel_ = new MMORankPannel;
    this->mmo_rank_pannel_->set_connection(this);

    this->mmo_invest_system_ = new MMOInvestSystem;
    this->mmo_invest_system_->set_connection(this);

    this->mmo_script_progress_ = new MMOScriptProgress;
    this->mmo_script_progress_->set_connection(this);

    this->mmo_script_clean_ = new MMOScriptClean;
    this->mmo_script_clean_->set_connection(this);

    this->mmo_player_tip_ = new MMOPlayerTip;
    this->mmo_player_tip_->set_connection(this);

    this->mmo_sys_setting_ = new MMOSysSetting;
    this->mmo_sys_setting_->set_connection(this);

    this->mmo_script_history_ = new MMOScriptHistory;
    this->mmo_script_history_->set_connection(this);

    this->mmo_media_gift_ = new MMOMediaGift;
    this->mmo_media_gift_->set_connection(this);

    this->mmo_open_activity_ = new MMOOpenActivity;
    this->mmo_open_activity_->set_connection(this);

    this->mmo_lucky_wheel_ = new MMOLuckyWheel;
    this->mmo_lucky_wheel_->set_connection(this);

    this->back_flow_control_ = new BackFlowControl;
    this->back_flow_control_->set_connection(this);

    this->back_scene_line_ = new BackSceneLine;
    this->back_scene_line_->set_connection(this);

    this->back_brocast_ = new BackBrocast;
    this->back_brocast_->set_connection(this);

    this->back_mail_request_ = new BackMailRequest;
    this->back_mail_request_->set_connection(this);

    this->back_media_gift_ = new BackMediaGift;
    this->back_media_gift_->set_connection(this);

    this->back_customer_svc_ = new BackCustomerSVC;
    this->back_customer_svc_->set_connection(this);

    this->back_restriction_ = new BackRestriction;
    this->back_restriction_->set_connection(this);

    this->back_recharge_ = new BackRecharge;
    this->back_recharge_->set_connection(this);

    this->mmo_recharge_rewards_ = new MMORechargeRewards;
    this->mmo_recharge_rewards_->set_connection(this);

    this->mmo_world_boss_ = new MMOWorldBoss;
    this->mmo_world_boss_->set_connection(this);

    this->mmo_travel_ = new MMOTravel;
    this->mmo_travel_->set_connection(this);

    this->back_draw_ = new BackDraw;
    this->back_draw_->set_connection(this);

    this->mmo_wedding_ = new MMOWedding;
    this->mmo_wedding_->set_connection(this);

    this->mmo_magicw_ = new MMOMagicWeapon;
    this->mmo_magicw_->set_connection(this);

    this->mmo_server_info_ = new MMOServerInfo;
    this->mmo_server_info_->set_connection(this);

    this->mmo_combine_server_ = new MMOCombineServer;
    this->mmo_combine_server_->set_connection(this);

    this->mmo_sword_pool_ = new MMOSwordPool;
    this->mmo_sword_pool_->set_connection(this);

    this->mmo_offlinehook = new MMOOfflineHook;
    this->mmo_offlinehook->set_connection(this);
    
    this->back_jyback_activity_ = new BackJYBackActivity();
    this->back_jyback_activity_->set_connection(this);

    this->mmo_trav_team_ = new MMOTravTeam();
    this->mmo_trav_team_->set_connection(this);
}

MongoConnector::~MongoConnector(void)
{
	SAFE_DELETE(this->mmo_role_);
	SAFE_DELETE(this->mmo_role_ex_);
	SAFE_DELETE(this->mmo_global_);
	SAFE_DELETE(this->mmo_fight_);
	SAFE_DELETE(this->mmo_package_);
	SAFE_DELETE(this->mmo_skill_);
	SAFE_DELETE(this->mmo_escort_);
	SAFE_DELETE(this->mmo_status_);
	SAFE_DELETE(this->mmo_online_);
	SAFE_DELETE(this->mmo_mail_);
	SAFE_DELETE(this->mmo_mail_offline_);
	SAFE_DELETE(this->mmo_socialer_);
    SAFE_DELETE(this->mmo_task_);
    SAFE_DELETE(this->mmo_vip_);
    SAFE_DELETE(this->mmo_activity_tips_);
    SAFE_DELETE(this->mmo_online_rewards_);
    SAFE_DELETE(this->mmo_offline_rewards_);
    SAFE_DELETE(this->mmo_treasures_info_);
    SAFE_DELETE(this->mmo_collect_chests_);
    SAFE_DELETE(this->mmo_equip_smelt_);
    SAFE_DELETE(this->mmo_welfare_);
    SAFE_DELETE(this->mmo_exp_restore_);

	SAFE_DELETE(this->mmo_league_);
	SAFE_DELETE(this->mmo_trade_);
	SAFE_DELETE(this->market_);
	SAFE_DELETE(this->back_serial_);
	SAFE_DELETE(this->mmo_chat_league_);
	SAFE_DELETE(this->mmo_chat_private_);
	SAFE_DELETE(this->mmo_shop_);
	SAFE_DELETE(this->mmo_beast_);
	SAFE_DELETE(this->mmo_label_);
	SAFE_DELETE(this->mmo_illus_);
    SAFE_DELETE(this->mmo_script_);
    SAFE_DELETE(this->mmo_achievement_);
    SAFE_DELETE(this->mmo_hidden_treasure_);
    SAFE_DELETE(this->mmo_fashion_);
    SAFE_DELETE(this->mmo_transfer_);
    SAFE_DELETE(this->mmo_rank_pannel_);
    SAFE_DELETE(this->mmo_script_progress_);
    SAFE_DELETE(this->mmo_script_clean_);
    SAFE_DELETE(this->mmo_player_tip_);
    SAFE_DELETE(this->mmo_sys_setting_);
    SAFE_DELETE(this->mmo_script_history_);
    SAFE_DELETE(this->mmo_media_gift_);
    SAFE_DELETE(this->mmo_open_activity_);
    SAFE_DELETE(this->mmo_lucky_wheel_);

    SAFE_DELETE(this->back_flow_control_);
    SAFE_DELETE(this->back_scene_line_);
    SAFE_DELETE(this->back_brocast_);
    SAFE_DELETE(this->back_mail_request_);
    SAFE_DELETE(this->back_media_gift_);
    SAFE_DELETE(this->back_customer_svc_);
    SAFE_DELETE(this->back_restriction_);
    SAFE_DELETE(this->back_recharge_);
    SAFE_DELETE(this->mmo_recharge_rewards_);
    SAFE_DELETE(this->mmo_world_boss_);
    SAFE_DELETE(this->back_draw_);
    SAFE_DELETE(this->mmo_travel_);
    SAFE_DELETE(this->mmo_wedding_);
    SAFE_DELETE(this->mmo_magicw_);
    SAFE_DELETE(this->mmo_server_info_);
    SAFE_DELETE(this->mmo_combine_server_);
    SAFE_DELETE(this->mmo_sword_pool_);
    SAFE_DELETE(this->mmo_offlinehook);
    SAFE_DELETE(this->back_jyback_activity_);
	SAFE_DELETE(this->conn_);
	SAFE_DELETE(this->mmo_trav_team_);
}

mongo::DBClientConnection &MongoConnector::conection(void)
{
    if (this->conn_ == 0)
    {
        this->conn_ = new DBClientConnection(true);
        this->connect_mongo();
    }
    else
    {
        if (this->conn_->isFailed() == true)
            this->connect_mongo();
    }
    return *(this->conn_);
}

int MongoConnector::connect_mongo(void)
{
    std::string errmsg;

    const Json::Value &mongo_json = CONFIG_INSTANCE->global()["mongo_machine"];
    std::string mongo_host = mongo_json["host"].asString();
    int mongo_port = mongo_json["port"].asInt();

    if (mongo_port > 0)
    {
        char sz_host_port[64];
        ::sprintf(sz_host_port, ":%d", mongo_port);
        mongo_host += sz_host_port;
    }

    if (this->conn_->connect(mongo_host, errmsg) == true)
    {
        MSG_USER("connected mongo %s", this->conn_->getServerAddress().c_str());
        return 0;
    }
    return -1;
}

void MongoConnector::disconnect(void)
{
	if (this->conn_ != NULL)
	{
		delete this->conn_;
		this->conn_ = NULL;
	}
}

int MongoConnector::check_role_auto_run_mongo(Int64 role, MongoDataMap *mongo_data_map)
{
	if (mongo_data_map->check_role_ == true && role > 0)
	{
		JUDGE_RETURN(this->mmo_role_->check_role(role) == true, -1);
		return this->auto_run_mongo(mongo_data_map);
	}
	else
	{
		return this->auto_run_mongo(mongo_data_map);
	}
}

int MongoConnector::auto_run_mongo(MongoDataMap *mongo_data_map)
{
    MongoDataMap::DataMap &data_map = mongo_data_map->data_map();
    for (MongoDataMap::DataMap::iterator iter = data_map.begin();
            iter != data_map.end(); ++iter)
    {
        MongoData *data = iter->second;
//        JUDGE_CONTINUE(data != NULL);

		try {
			if (data->is_update())
			{
				this->conection().update(data->table(),
						mongo::Query(data->condition()),
						BSON("$set" << data->data_bson()),
						data->is_auto_insert());
			}
			else if (data->is_find())
			{
				data->set_data_bson(
						this->conection().findOne(data->table(),
								mongo::Query(data->condition())));
			}
			else if (data->is_query())
			{
				data->set_data_cursor(
						this->conection().query(data->table(),
								mongo::Query(data->condition())));
			}
			else if (data->is_multithread_query())
			{
				data->set_multithread_cursor(
						this->conection().query(data->table(),
								mongo::Query(data->condition()),
								GameEnum::RANK_RECORD_DEFAULT_LIMIT_NUM));
			}
			else if (data->is_multithread_query_with_sort())
			{
				data->set_multithread_cursor(
						this->conection().query(data->table(),
								mongo::Query(data->condition()).sort(data->sort_condition()),
								GameEnum::RANK_RECORD_DEFAULT_LIMIT_NUM));
			}
            else if (data->is_remove())
            {
                this->conection().remove(data->table(),
                        mongo::Query(data->condition()));
            }
		} catch (mongo::SocketException &ex) {
			MSG_USER("ERROR mongo[%s %d], %s", data->table().c_str(), data->op_type(), ex.toString().c_str());
			this->connect_mongo();
			return -1;
		} catch (...) {
			MSG_USER("ERROR mongo error %s %d", data->table().c_str(), data->op_type());
			return -1;
		}
    }
    return 0;
}

int MongoConnector::load_gate_player(GatePlayer *player)
{
    int ret = this->mmo_role_->load_player(player);
    this->mmo_online_->load_player_online(player->role_id(), &(player->online()));
    return ret;
}

int MongoConnector::load_global_key(HashMap<std::string, int64_t, NULL_MUTEX> *global_key_map)
{
    return this->mmo_global_->load_global_key(global_key_map);
}
int64_t MongoConnector::load_global_voice_id(void)
{
	return this->mmo_global_->load_global_voice_id();
}
int MongoConnector::save_global_voice_id(int64_t id)
{
	return this->mmo_global_->update_global_voice_id(id);
}

int MongoConnector::update_back_role_offline(void)
{
	return this->mmo_role_->update_back_role_offline();
}

int MongoConnector::save_new_role(GatePlayer *player)
{
    return this->mmo_role_->save_new_role(player);
}

int MongoConnector::load_player(LogicPlayer *player)
{
	if (this->mmo_role_->load_player_base(player) != 0)
    {
        MSG_USER("ERROR load player base %ld", player->role_id());
        return -1;
    }

    this->mmo_role_ex_->load_role_ex(player);
    this->mmo_online_->load_player_online(player->role_id(), &(player->online()));
    this->mmo_online_->update_player_login_tick(player->role_id());
    this->mmo_open_activity_->load_open_activity(player);
    this->mmo_open_activity_->load_may_activityer(player);
    this->mmo_lucky_wheel_->load_lucky_wheel_activity(player);
    this->mmo_league_->load_leaguer_info(player);
    this->mmo_socialer_->load_player_socialer(player);
    this->mmo_vip_->load_player_vip(player);
    this->mmo_activity_tips_->load_player_activity_tips(player);
    this->back_customer_svc_->load_player_customer_service_detail_when_init(player);
    this->back_customer_svc_->load_customer_way_when_init(player);
    this->mmo_wedding_->load_player_wedding(player);
    this->back_jyback_activity_->load_player_back_act_rec(player);

    player->check_adjust_wedding_id();
    return 0;
}

int MongoConnector::load_player(MapPlayerEx *player)
{
	if (this->mmo_role_->load_player_base(player) != 0)
    {
        MSG_USER("ERROR load player base %ld", player->role_id());
        return -1;
    }

    this->mmo_status_->load_player_status(player);
    this->mmo_fight_->load_player_fight(player);
    this->mmo_skill_->load_player_skill(player);
    this->mmo_escort_->load_player_escort(player);

    player->init_level_property();
	player->init_all_player_skill();
	player->init_skill_property();
    player->adjust_load_db_info();
    player->adjust_map_skiller();
	player->refresh_all_status_property(ENTER_SCENE_TRANSFER);

    this->mmo_online_->load_player_online(player->role_id(), &(player->online()));
    this->back_serial_->load_map_serial(player);
    this->mmo_script_->load_player_script(player);
    this->mmo_league_->load_leaguer_skill_prop(player);
    this->mmo_league_->load_league_flag_prop(player);

    player->init_league_flag_property();
    player->init_league_skill_property();

    this->mmo_vip_->load_player_vip(player);
    this->mmo_fight_->load_tiny_player(player);
    this->mmo_travel_->load_player_tarena(player);
    this->mmo_role_->check_and_modify(player);
    this->mmo_magicw_->load_player_magic_weapon(player);

    if (player->is_new_role() == true)
    {
    	FightDetail &fight_detail = player->fight_detail();
        fight_detail.__blood = fight_detail.__blood_total_i(player);
        fight_detail.__magic = fight_detail.__magic_total_i(player);
    }

    if (player->is_death() == true)
    {
    	FightDetail &fight_detail = player->fight_detail();
    	fight_detail.__blood = 100;
    }

    if (player->tarena_detail().adjust_tick_ == 0)
    {
    	player->tarena_detail().adjust_tick_ = GameCommon::next_month_start_zero();
    }

    return 0;
}

int MongoConnector::load_player(MapLogicPlayer *player)
{
    if (this->mmo_role_->load_player_base(player) != 0)
    {
        MSG_USER("ERROR load player base %ld", player->role_id());
        return -1;
    }

    this->mmo_role_ex_->load_role_ex(player);
    this->mmo_package_->load_player_package(player);
    this->mmo_mail_->load_player_mail(player);
    this->mmo_online_->load_player_online(player->role_id(), &(player->online()));
    this->mmo_trade_->load_mmo_trade(player);
    this->mmo_vip_->load_player_vip(player);
    this->mmo_online_rewards_->load_player_online_rewards(player);
    this->mmo_offline_rewards_->load_player_offline_rewards(player);
    this->mmo_treasures_info_->load_player_treasures_info(player);
    this->mmo_collect_chests_->load_player_collect_chests(player);
    this->mmo_equip_smelt_->load_player_equip_smelt(player);
    this->mmo_welfare_->load_player_welfare(player);
    this->mmo_invest_system_->load_player(player);
    this->mmo_illus_->load_player_illus(player);
    this->mmo_exp_restore_->load_player_exp_restore(player);
    this->mmo_beast_->load_master(player);

    this->back_serial_->load_map_logic_serial(player);
    this->mmo_task_->load_player_task(player);
    this->mmo_label_->load_player_label(player);
    this->mmo_achievement_->load_player_achievement(player);
    this->mmo_hidden_treasure_->load_player_hi_treasure(player);
    this->mmo_fashion_->load_player_fashion(player);
    this->mmo_transfer_->load_player_transfer(player);
    this->mmo_script_clean_->load_player_script_clean(player);
    this->mmo_player_tip_->load_player_tip_pannel(player);
    this->mmo_media_gift_->load_player_media_gift(player);
    this->mmo_recharge_rewards_->load_player_recharge_rewards(player);
    this->mmo_fight_->load_tiny_player(player);
    this->mmo_magicw_->load_player_magic_weapon(player);
    this->mmo_sword_pool_->load_player_spool(player);
    this->mmo_offlinehook->load_player_offlinehook(player);
    //adjust
    player->adjust_mount_open();
	player->caculate_all_mount_prop();

    return 0;
}

int MongoConnector::load_mail_offline(int64_t role_id, MailBox *mail_box)
{
	return this->mmo_mail_offline_->load_mail_offline(role_id, mail_box);
}

int MongoConnector::save_mail_offline(MailInformation *info)
{
	return this->mmo_mail_offline_->save_mail_offline(info);
}

Int64 MongoConnector::search_player_name(const string& role_name)
{
	return this->mmo_role_->search_player_name(role_name);
}

int MongoConnector::load_mmo_db(DBShopMode* shop_mode)
{
	switch(shop_mode->recogn_)
	{
	case TRANS_LOAD_SHOP_INFO:
	{
		this->mmo_shop_->load_shop_by_config(shop_mode);
		this->mmo_shop_->load_shop(shop_mode);
		break;
	}
	case TRANS_LOAD_GAME_NOTICE:
	{
		this->back_mail_request_->load_game_notice(shop_mode);
		break;
	}
	case TRANS_LOAD_CHAT_LIMIT:
	{
		this->back_mail_request_->load_chat_limit(shop_mode);
		break;
	}
	case TRANS_LOAD_WORDS_CHECK:
	{
		this->back_mail_request_->load_word_check(shop_mode);
		break;
	}
	case TRANS_LOAD_VIP_CHAT_LIMIT:
	{
		this->back_mail_request_->load_vip_chat_limit(shop_mode);
		break;
	}
	case TRANS_LOAD_LUCKY_WHEEL:
	{
		this->mmo_lucky_wheel_->load_back_activity_info(shop_mode);
		break;
	}
	case TRANS_LOAD_FESTIVAL_TIME:
	{
		this->mmo_open_activity_->load_fest_act_time(shop_mode);
		break;
	}
	case TRANS_LOAD_MAY_ACTIVITY:
	{
		this->mmo_open_activity_->load_back_may_activity(shop_mode);
		break;
	}
	case TRANS_LOAD_COMBINE_SERVER:
	{
		this->mmo_combine_server_->load_combine_server(shop_mode);
		break;
	}
	case TRANS_LOAD_BACK_ACTIVITY:
	{
		this->back_mail_request_->load_back_activity(shop_mode);
		break;
	}
	case TRANS_LOAD_ALL_ROLE_ID:
	{
		this->mmo_role_->load_all_role_id(shop_mode);
		break;
	}
	case TRANS_GET_RANK_ROLE_CARRER:
	{
		this->mmo_role_->load_rank_roleid_carrer(shop_mode);
		break;
	}
	case TRANS_PLAYER_ROLE_RENAME:
	{
		this->mmo_role_->update_player_name(shop_mode);
		break;
	}
	case TRANS_LOAD_PLAYER_FOR_MALL_DONATION:
	{
		const char* player_name = shop_mode->input_argv_.type_string_.c_str();
		shop_mode->output_argv_.type_int64_ = this->mmo_role_->search_player_name(player_name);
		break;
	}
	case TRANS_LOAD_COPY_PLAYER:
	case TRANS_LOAD_AREA_COPY_PLAYER:
	case TRANS_LOAD_COPY_TRAV_TEAMER:
	{
		this->mmo_role_->load_copy_player(shop_mode);
		break;
	}
	case TRANS_DB_SAVE_PET_RANK_DETAIL:
	{
		this->mmo_rank_pannel_->save_beast_detail_on_pet_rank(shop_mode);
		break;
	}
	case TRANS_LOAD_LFR_WAR_INFO:
	{
		this->mmo_league_->load_lwar_info(shop_mode);
		break;
	}
	}

	return 0;
}


int MongoConnector::request_save_mmo_done(int recogn, DBTradeMode* trade_mode)
{
	switch (recogn)
	{
	case TRANS_SAVE_MAIL_OFFLINE:
	{
		Int64 mail_id = this->update_global_key(Global::MAIL);
		*trade_mode->query_ = BSON(MailOffline::MAIL_ID << mail_id);
		break;
	}

	default:
	{
		break;
	}
	}

	this->conection().update(trade_mode->table_name_, *(trade_mode->query_),
			*(trade_mode->content_), trade_mode->operate_flag_);
	return 0;
}

int MongoConnector::request_remove_mmo_done(int recogn, DBTradeMode* trade_mode)
{
	switch (recogn)
	{
	default:
	{
		this->conection().remove(trade_mode->table_name_, *(trade_mode->query_),
				trade_mode->operate_flag_);
	}
	}
	return 0;
}

int MongoConnector::get_rpm_recomand_info(RpmRecomandInfo* rpm_recomand_info)
{
	return this->mmo_role_->get_rpm_recomand_info(rpm_recomand_info);
}

int MongoConnector::get_rpm_introduction_info(RpmRecomandInfo* rpm_recomand_info)
{
	return this->mmo_role_->get_rpm_introduction_info(rpm_recomand_info);
}

int MongoConnector::init_rpm_introduction_info(void)
{
	return MMORole::init_introduction_info();
}

int64_t MongoConnector::update_global_key(const std::string &key)
{
    return this->mmo_global_->update_global_key(key);
}

int MongoConnector::load_global_script_progress(HashMap<int, Int64, NULL_MUTEX> *progress_map, const std::vector<int> &script_set)
{
    return this->mmo_global_->load_global_script_progress(progress_map, script_set);
}

int MongoConnector::update_global_script_progress(const int script_sort, const Int64 progress)
{
    return this->mmo_global_->update_global_script_progress(script_sort, progress);
}

int MongoConnector::update_offline_tick(void)
{
	return this->mmo_online_->update_all_player_offline();
}

int MongoConnector::update_special_FB_achievement(MongoDataMap* data_map)
{
	return this->mmo_achievement_->update_special_FB_achievement(data_map);
}

int MongoConnector::load_script_progress(ScriptPlayerRel *player_rel)
{
    return this->mmo_script_progress_->load_script_progress(player_rel);
}

int MongoConnector::load_activity_tick(MongoDataMap *data_map)
{
    return this->back_draw_->load_activity_tick(data_map);
}

MMOGlobal *MongoConnector::mmo_global(void)
{
	return this->mmo_global_;
}

MMOChatLeague *MongoConnector::chat_league(void)
{
	return this->mmo_chat_league_;
}

MMOChatPrivate *MongoConnector::chat_private(void)
{
	return this->mmo_chat_private_;
}

MMOMarket *MongoConnector::market(void)
{
	return this->market_;
}

MMORole *MongoConnector::mmo_role(void)
{
	return this->mmo_role_;
}

MMORoleEx *MongoConnector::mmo_role_ex(void)
{
	return this->mmo_role_ex_;
}

MMOSocialer *MongoConnector::mmo_socialer(void)
{
	return this->mmo_socialer_;
}

MMORankPannel *MongoConnector::mmo_rank_pannel(void)
{
	return this->mmo_rank_pannel_;
}

MMOInvestSystem *MongoConnector::mmo_invest_system(void)
{
	return this->mmo_invest_system_;
}

MMOBeast *MongoConnector::mmo_beast(void)
{
	return this->mmo_beast_;
}

MMOScriptHistory *MongoConnector::mmo_script_history(void)
{
    return this->mmo_script_history_;
}

BackFlowControl* MongoConnector::back_flow_control(void)
{
	return this->back_flow_control_;
}

BackSceneLine *MongoConnector::back_scene_line(void)
{
    return this->back_scene_line_;
}

BackBrocast *MongoConnector::back_brocast(void)
{
	return this->back_brocast_;
}

BackMediaGift *MongoConnector::back_media_gift(void)
{
	return this->back_media_gift_;
}

BackCustomerSVC *MongoConnector::back_customer_svc(void)
{
	return this->back_customer_svc_;
}
