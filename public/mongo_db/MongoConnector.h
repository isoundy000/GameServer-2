/*
 * MongoConnector.h
 *
 * Created on: 2013-01-24 11:41
 *     Author: glendy
 */

#ifndef _MONGOCONNECTOR_H_
#define _MONGOCONNECTOR_H_

#include "GameHeader.h"

class MMORole;
class MMORoleEx;
class MMOGlobal;
class MMOFight;
class MMOPackage;
class MMOSkill;
class MMOEscort;
class MMOStatus;
class MMOOnline;
class MMOTrade;
class MMOLeague;
class MMOMail;
class MMOMailOffline;
class MMOTask;
class MMOShop;
class MMODivine;
class MMOVip;
class MMOActivityTips;
class MMOOnlineRewards;
class MMOOfflineRewards;
class MMOTreasures;
class MMOEquipSmelt;
class MMOCollectChests;
class MMOExpRestore;
class MMOChatLeague;
class MMOChatPrivate;
class MMOCombineServer;

class MMOSocialer;
class MailInformation;
class MailBox;
class MMOBeast;
class MMOMarket;
class BackSerial;
class BackFlowControl;
class BackBrocast;
class BackMailRequest;
class MMOLabel;
class MMOScript;
class MMOAchievement;
class MMOHiddenTreasure;
class MMOFashion;
class MMOTransfer;
class MMORankPannel;
class MMOInvestSystem;
class MMOScriptProgress;
class ScriptPlayerRel;
class MMOScriptClean;
class MMOPlayerTip;
class MMOSysSetting;
class MMOScriptHistory;
class RpmRecommandMap;
class BackSceneLine;
class BackCustomerSVC;
class BackMediaGift;
class BackRestriction;
class BackRecharge;
class MMOMediaGift;
class MMOWelfare;
class MMOIllustration;
class RpmRecomandInfo;
class MMORechargeRewards;
class MMOWorldBoss;
class MMOActivityTipsSystem;

class BackDraw;
class MMOTravel;
class MMOWedding;
class MMOMagicWeapon;
class MMOServerInfo;
class MMOOpenActivity;
class MMOSwordPool;
class MMOLuckyWheel;
class MMOOfflineHook;
class BackJYBackActivity;
class MMOTravTeam;

template<class Key, class Value, class HSMUTEX> class HashMap;
class MongoDataMap;

class MongoConnector
{
    friend class MongoUnit;
public:
    MongoConnector(void);
    ~MongoConnector(void);

    DBClientConnection &conection(void);
    int connect_mongo(void);
    void disconnect(void);

    int check_role_auto_run_mongo(Int64 role, MongoDataMap *mongo_data_map);
    int auto_run_mongo(MongoDataMap *mongo_data_map);

    int load_gate_player(GatePlayer *player);
    int load_global_key(HashMap<std::string, int64_t, NULL_MUTEX> *global_key_map);
    int64_t load_global_voice_id(void);
    int save_global_voice_id(int64_t id);
    int update_back_role_offline(void);

    int save_new_role(GatePlayer *player);

    int load_player(LogicPlayer *player);
    int load_player(MapPlayerEx *player);
    int load_player(MapLogicPlayer *player);

    int load_mail_offline(int64_t role_id, MailBox *mail_box);
    int save_mail_offline(MailInformation *info);

    Int64 search_player_name(const string& role_name);

    int load_mmo_db(DBShopMode* shop_mode);

    int request_save_mmo_done(int recogn, DBTradeMode* trade_mode);
    int request_remove_mmo_done(int recogn, DBTradeMode* trade_mode);

    int get_rpm_recomand_info(RpmRecomandInfo* rpm_recomand_info);
    int get_rpm_introduction_info(RpmRecomandInfo* rpm_recomand_info);
    int init_rpm_introduction_info(void);

    int64_t update_global_key(const std::string &key);
    int load_global_script_progress(HashMap<int, Int64, NULL_MUTEX> *progress_map, const std::vector<int> &script_set);
    int update_global_script_progress(const int script_sort, const Int64 progress);

    int update_offline_tick(void);
    int update_special_FB_achievement(MongoDataMap* data_map);

    int load_script_progress(ScriptPlayerRel *player_rel);

    int load_panic_shop(void);
    int load_activity_tick(MongoDataMap *data_map);

    MMOGlobal *mmo_global(void);
    MMOChatLeague *chat_league(void);
    MMOChatPrivate *chat_private(void);
    MMORole *mmo_role(void);
    MMORoleEx *mmo_role_ex(void);

    MMOSocialer *mmo_socialer(void);
    MMOMarket *market(void);
    MMORankPannel *mmo_rank_pannel(void);
    MMOBeast *mmo_beast(void);
    MMOScriptHistory *mmo_script_history(void);
    BackFlowControl* back_flow_control(void);
    BackSceneLine *back_scene_line(void);
    BackBrocast *back_brocast(void);
    BackCustomerSVC *back_customer_svc(void);
    BackMediaGift *back_media_gift(void);
    MMOInvestSystem *mmo_invest_system(void);

public:
    DBClientConnection *conn_;

    MMOGlobal *mmo_global_;
    MMORole *mmo_role_;
    MMORoleEx *mmo_role_ex_;
    MMOFight *mmo_fight_;
    MMOPackage *mmo_package_;
    MMOSkill *mmo_skill_;
    MMOEscort *mmo_escort_;
    MMOStatus *mmo_status_;
    MMOSocialer *mmo_socialer_;
    MMOTask *mmo_task_;
    
    MMOOnline *mmo_online_;
	MMOMail *mmo_mail_;
	MMOMailOffline *mmo_mail_offline_;
	MMOTrade *mmo_trade_;
	MMOLeague *mmo_league_;
	MMOVip *mmo_vip_;
	MMOActivityTips *mmo_activity_tips_;
	MMOOnlineRewards *mmo_online_rewards_;
	MMOOfflineRewards *mmo_offline_rewards_;
	MMOTreasures *mmo_treasures_info_;
	MMOCollectChests *mmo_collect_chests_;
	MMOEquipSmelt *mmo_equip_smelt_;
	MMOWelfare *mmo_welfare_;
	MMOIllustration *mmo_illus_;
	MMOExpRestore *mmo_exp_restore_;
    MMOChatLeague *mmo_chat_league_;
    MMOChatPrivate *mmo_chat_private_;
    
    MMOMarket *market_;
    MMOShop* mmo_shop_;
    MMOBeast* mmo_beast_;
    BackSerial *back_serial_;
    BackFlowControl *back_flow_control_;
    BackSceneLine *back_scene_line_;
    BackBrocast *back_brocast_;
    BackMailRequest *back_mail_request_;
    BackCustomerSVC *back_customer_svc_;
    BackMediaGift	*back_media_gift_;
    BackRestriction *back_restriction_;
    BackRecharge *back_recharge_;

    MMOLabel *mmo_label_;
    MMOScript *mmo_script_;
    MMOAchievement *mmo_achievement_;
    MMOHiddenTreasure *mmo_hidden_treasure_;
    MMOFashion *mmo_fashion_;
    MMOTransfer *mmo_transfer_;
    MMORankPannel *mmo_rank_pannel_;
    MMOWorldBoss* mmo_world_boss_;
    MMOInvestSystem *mmo_invest_system_;

    MMOScriptProgress *mmo_script_progress_;
    MMOScriptClean *mmo_script_clean_;
    MMOPlayerTip *mmo_player_tip_;
    MMOSysSetting *mmo_sys_setting_;
    MMOScriptHistory *mmo_script_history_;
    MMOMediaGift	*mmo_media_gift_;
    MMOOpenActivity *mmo_open_activity_;
    MMOLuckyWheel *mmo_lucky_wheel_;
    MMORechargeRewards *mmo_recharge_rewards_;
    MMOTravel* mmo_travel_;
    BackDraw *back_draw_;
    MMOWedding *mmo_wedding_;
    MMOMagicWeapon *mmo_magicw_;
    MMOServerInfo *mmo_server_info_;
    MMOCombineServer *mmo_combine_server_;
    MMOSwordPool* mmo_sword_pool_;
    MMOOfflineHook* mmo_offlinehook;
    BackJYBackActivity* back_jyback_activity_;
    MMOTravTeam *mmo_trav_team_;
};

#endif //_MONGOCONNECTOR_H_
