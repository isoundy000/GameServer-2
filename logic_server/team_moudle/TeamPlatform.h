/*
 * TeamPlatform.h
 *
 *  Created on: Jul 22, 2013
 *      Author: peizhibi
 */

#ifndef TEAMPLATFORM_H_
#define TEAMPLATFORM_H_

#include "TeamPanel.h"

class TeamPlatform
{
public:
	TeamPlatform();
	~TeamPlatform();

	bool can_create_team(int scene_id);
	bool can_dismiss_team(int team_count);

	int offline_handle_time_out();
	int team_request_time_out();
	int sign_view_time_out();

	int update_rep_id_info(Message* msg);

	int push_offline_teamer(int64_t role_id, int team_index);
	int pop_offline_teamer(int64_t role_id);

	void set_team_view(int64_t role_id, int view_index);
	void erase_team_view(int64_t role_id);

	void brocast_team_view(int view_index);
	void remove_offline_team(int team_index);

	bool validate_team_view(int64_t role_id, int open_team = true);

	void add_replier(Int64 role_id);
	void remove_replier(Int64 role_id);

	PoolPackage<TeamPanel>* team_panel_package();

private:
	int offline_handle_time_out(TeamPanel* team_panel, bool &is_erase);
	int notify_offline_handle(TeamPanel* team_panel);

private:
	PoolPackage<TeamPanel> panel_package_;	// TeamPanel内存池
	LongMap view_team_map_;					// 玩家ID --- 副本ID

	IntMap team_index_map_;					// 队伍ID(有队员离线)
	LongList team_replier_list_;			// 需要自动处理请求的玩家
	LongMap offline_teamers_map_;			// 存放 离线玩家ID --- 队伍ID
};

typedef Singleton<TeamPlatform> TeamPlatformSingle;
#define TEAM_PLANTFORM   			TeamPlatformSingle::instance()
#define TEAM_PANEL_PACKAGE			TEAM_PLANTFORM->team_panel_package()
#define TEAM_PLANTFORM_INFO   		TEAM_PLANTFORM->platform_info()

#endif /* TEAMPLATFORM_H_ */
