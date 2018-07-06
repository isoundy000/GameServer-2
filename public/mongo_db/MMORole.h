/*
 * MMORole.h
 *
 * Created on: 2013-03-21 11:30
 *     Author: lyz
 */

#ifndef _MMOROLE_H_
#define _MMOROLE_H_

#include "MongoTable.h"
#include "LeagueStruct.h"
struct FriendInfo;
struct DBFriendInfo;

class MongoDataMap;

class MMORole : public MongoTable
{
public:
    virtual ~MMORole(void);

    int save_new_back_role(GatePlayer *player);
    int save_account_phone_info(GatePlayer *player);
    int save_new_role(GatePlayer *player);
    int load_player(GatePlayer *player);
    int is_can_create_role(const string agent_code,const string ip,const string mac);
    int fetch_player(BSONObj& res, GatePlayer* player, const BSONObj& fields);

    void set_role_permission_info(BaseRoleInfo& role_info);
    void set_gm_permission(GatePlayer *player);
    void set_ban_ip_info(GatePlayer *player);
    void set_mac_ban_info(GatePlayer *player);

    int load_player_base(LogicPlayer *player);
    static int update_data(LogicPlayer *player, MongoDataMap *data_map, const int recogn);

    int load_player_base(MapPlayerEx *player);
    static int update_data(MapPlayerEx *player, MongoDataMap *data_map);
    static int update_back_role(MapPlayerEx *player, MongoDataMap *data_map);

    int load_player_wedding_info_by_wedding_id(Int64 wedding_id, Int64 role_id, IntMap& self_map, IntMap& side_map);
    int load_player_wedding_info_by_role_id(Int64 role_id, IntMap& self_map, IntMap& side_map);

    int load_player_base(MapLogicPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);
    static int update_back_role(MapLogicPlayer *player, MongoDataMap *data_map);
    
    int load_offline_friend_info(DBFriendInfo *friend_detail);
//    int load_offline_friend_info_single(DBFriendInfo *friend_detail);
    int load_offline_player_info(Int64 role_id, FriendInfo& info);
    int load_player_to_send_mail(MailInformation *mail_info);

    int get_rpm_recomand_info(RpmRecomandInfo* rpm_recomand_info);
    int get_rpm_introduction_info(RpmRecomandInfo* rpm_recomand_info);

    static int init_introduction_info(void);
    static int init_introduction_info(const Json::Value &rpm_json);

    Int64 search_player_name(const string& role_name);
    string search_player_account(const string& role_name);

    int check_and_modify(MapPlayerEx *player);
    int check_role(Int64 role_id);
    int check_validate_create_role(Int64 role_id);

    static bool validate_player(Int64 role_id);
    static bool validate_league_member(Int64 role_id, Int64 league_index);
    static void load_league_member(LeagueMember& league_member);

    static void init_base_role_info(BaseRoleInfo& detail, const BSONObj& res);
    static void update_role_name(MapLogicPlayer *player, const char *src_name, MongoDataMap *data_map);

    int update_player_name(MongoDataMap *data_map);
    static std::string fetch_role_name(BSONObj &obj);
    static std::string fetch_player_name(Int64 role_id);
    std::string fetch_league_name(Int64 league_id);

    int update_player_name(DBShopMode* shop_mode);

    void load_copy_player(DBShopMode* shop_mode);
    static int update_copy_player(MapPlayerEx *player, MongoDataMap *data_map, const Int64 role_id = 0);
//    static int update_copy_player(MapLogicPlayer *player, MongoDataMap *data_map);

    static int add_test_copy_player(LongMap& add_map, const LongSet& exist_map, uint differ);

    int fetch_random_name(MongoDataMap *data_map);
    static void update_sex_condition(MongoDataMap *data_map, int sex, const string& account);

    int sync_recharge_info_to_all_role(RechargeOrder* recharge_order);

    int update_back_role_offline(void);

    static int fetch_role_info_for_wedding(const Int64 role_id, std::string &name, int &sex, int &career);
    static int fetch_role_info_for_brother(const Int64 role_id, std::string &name, int &sex, int &career, int& level);
    int load_all_role_id(DBShopMode* shop_mode);
    int load_rank_roleid_carrer(DBShopMode* shop_mode);
    static void load_sys_model(const Int64 role_id ,string& out_sys_model,string& market_code);
protected:
    virtual void ensure_all_index(void);
};

class MMORoleEx : public MongoTable
{
public:
	virtual ~MMORoleEx();

    int load_role_ex(MapLogicPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

    int load_role_ex(LogicPlayer *player);
    static int update_data(LogicPlayer *player, MongoDataMap *mongo_data);
protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOROLE_H_
