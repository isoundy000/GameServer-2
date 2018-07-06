/*
 * TeamPanel.h
 *
 *  Created on: Jul 22, 2013
 *      Author: peizhibi
 */

#ifndef TEAMPANEL_H_
#define TEAMPANEL_H_

#include "LogicStruct.h"

class ProtoTeamer;
class ProtoSignInfo;
class Proto80400306;

struct TeamSetInfo
{
	TeamSetInfo(void);
	void reset(void);

	bool validate_applier(int64_t role_id);
	bool validate_inviter(int64_t role_id);
	bool validate_borcast_recuit(void);

	BLongMap team_applier_map_;
	BLongMap team_inviter_map_;
	int fb_confirm_tick_;
	int borcast_recuit_tick_;

	int auto_invite_;
	int auto_accept_;
};

struct OfflineTeamerInfo
{
	int make_up_teamer_info(ProtoTeamer* teamer_info);

	int64_t	role_id_;
	long offline_tick_;
	BaseRoleInfo teamer_info_;
};

struct ReplacementInfo
{
	int make_up_teamer_info(ProtoTeamer* teamer_info);
	void set_replacement_info(ReplacementRoleInfo* rpm_role_info);

	ReplacementRoleInfo teamer_info_;
};

struct TeamPanel
{
	TeamPanel(void);
	void reset(void);
	void notify_all_teamer(int recogn, Message* msg);

	int online_count();
	int teamer_count();

	bool teamer_full();
	bool is_leader(int64_t role_id);
	bool validate_teamer(int64_t role_id);
	bool validate_replacement(int64_t role_id);

	bool push_teamer(int64_t role_id, int push_type);
	bool erase_teamer(int64_t role_id);
	bool erase_replacement(int64_t role_id);

	RpmRecomandInfo* rpm_recomand_info();
	bool check_and_handle_offline();	//移除离线时间过长的队员
	int save_offline_teamer_info(int64_t role_id, bool is_leave = false);

	LogicPlayer* find_online_teamer(bool include_leader = true);

	int team_index_;
	int64_t leader_id_;
	int team_state_;

	int fb_id_;//as script sort
	int no_enter_nums_; //当前未进入副本的人数

	std::string leader_name_;
	LongList teamer_set_;
	LongList replacement_set_;

	LongMap fb_wait_confirm_set_;
	LongMap fb_ready_set_;
	LongMap use_times_set_;

	RpmRecomandInfo* rpm_recomand_info_;

	typedef std::map<int64_t, OfflineTeamerInfo> OfflineTeamerMap;
	OfflineTeamerMap offline_teamer_info_;

	typedef std::map<int64_t, ReplacementInfo> RpmTeamerMap;
	RpmTeamerMap rpm_teamer_info_;
};

#endif /* TEAMPANEL_H_ */
