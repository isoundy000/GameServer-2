/*
 * RestrictionSystem.h
 *
 *  Created on: Nov 17, 2014
 *      Author: LiJin
 */

#ifndef RESTRICTIONSYSTEM_H_
#define RESTRICTIONSYSTEM_H_

#include "GameHeader.h"

class RestrictionSystem
{

typedef std::vector<LogicPlayer*> LogicPlayerSet;

public:
	RestrictionSystem(void);
	virtual ~RestrictionSystem(void);

	int request_update_restriction_info(void);
	int request_mark_restriction_info_fin(int id, int retsult);
	int respond_update_restirction_info(Transaction* trans);

	int sync_player_speak_state(int64_t role_id, int state, int expired_time);

	int force_logout(int64_t role_id, int type = 0);
	int force_logout(LogicPlayer* player, const IntStrPair& pair = IntStrPair(0, ""));

private:
	int ip_ban_state_change(int type, const string& ip_addr, int64_t expired);
	int account_ban_state_change(int type, const string& account, int64_t expired);
	int role_id_ban_state_change(int type, int64_t role_id, int64_t expired);
	int role_name_ban_state_change(int type, const string& role_name, int64_t expired);
	int role_id_speak_state_change(int type, int64_t role_id, int64_t expired);
	int role_name_speak_state_change(int type, const string& role_name, int64_t expired);
	int role_kick_offline(int64_t role_id, const string& role_name, const string& account);

	int process_restrion_operation(const BSONObj* bson_obj);
	int query_online_player(int type, const string& match, LogicPlayerSet &player_set);
	int save_restrion_to_db(const BSONObj* querry, int ban_type, int64_t ban_expired);
	int save_ban_ip_to_db(uint ip_value, const string& ip_str, int64_t expired);
	int drop_ban_ip_from_db(uint ip_value);

	void make_query_bson_role_id(BSONObj* bson_obj, int64_t role_id);
	void make_query_bson_role_name(BSONObj* bson_obj, const string& role_name);
	void make_query_bson_account(BSONObj* bson_obj, const string& account);

	int force_logout_online_player(int type, const string& match);

	int mac_ban_state_change(int type,const string& mac,int64_t expired);
	int save_ban_mac_to_db(const string& mac_str, int64_t expired);
	int drop_ban_mac_from_db(const string& mac_str);

	LogicMonitor *monitor(void);
};

typedef Singleton<RestrictionSystem> RestrictionSystemSingle;
#define RESTRI_SYSTEM	RestrictionSystemSingle::instance()

#endif /* RESTRICTIONSYSTEM_H_ */
