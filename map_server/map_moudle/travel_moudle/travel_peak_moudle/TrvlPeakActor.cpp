/*
 * TrvlPeakActor.cpp
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#include "TrvlPeakActor.h"
#include "TrvlPeakMonitor.h"
#include "ProtoDefine.h"
#include "TrvlPeakScene.h"
#include "TrvlPeakPreScene.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"

#include <mongo/client/dbclient.h>

TrvlPeakActor::TrvlPeakActor() {
	// TODO Auto-generated constructor stub

}

TrvlPeakActor::~TrvlPeakActor() {
	// TODO Auto-generated destructor stub
}

void TrvlPeakActor::reset()
{

}

int TrvlPeakActor::enter_scene(const int type)
{
	switch (this->scene_id())
	{
	case GameEnum::TRVL_PEAK_SCENE_ID:
	{
		return this->on_enter_trvl_peak_scene(type);
	}
	case GameEnum::TRVL_PEAK_PRE_SCENE_ID:
	{
		return this->on_enter_prep_peak_scene(type);
	}
	}

	return 0;
}

int TrvlPeakActor::exit_scene(const int type)
{
	switch (this->scene_id())
	{
	case GameEnum::TRVL_PEAK_SCENE_ID:
	{
		return this->on_exit_trvl_peak_scene(type);
	}
	case GameEnum::TRVL_PEAK_PRE_SCENE_ID:
	{
		return this->on_exit_prep_peak_scene(type);
	}
	}

	return 0;
}

int TrvlPeakActor::die_process(const int64_t fighter_id)
{
	MapPlayer::die_process(fighter_id);

	TrvlPeakScene* scene = TRVL_PEAK_MONITOR->find_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, 0);

	scene->insert_die_player(this->real_role_id());

	TravPeakTeam *travel_team = TRVL_PEAK_MONITOR->find_travel_team_by_role_id(this->real_role_id());
	JUDGE_RETURN(travel_team != NULL, 0);

	bool is_all_teamer_die = true;
	for (TravPeakTeam::TeamerMap::iterator iter = travel_team->__teamer_map.begin();
			iter != travel_team->__teamer_map.end(); ++iter)
	{
		TravPeakTeam::TravPeakTeamer &teamer_info = iter->second;
		if (scene->is_has_die_player(teamer_info.__teamer_id) == false)
		{
			is_all_teamer_die = false;
			break;
		}
	}

	if (is_all_teamer_die == true)
	{
		scene->finish_fight_earlier();
	}

	return 0;
}

int TrvlPeakActor::trvl_peak_scene()
{
	return GameEnum::TRVL_PEAK_SCENE_ID;
}

int TrvlPeakActor::trvl_peak_prep_scene()
{
	return GameEnum::TRVL_PEAK_PRE_SCENE_ID;
}

int TrvlPeakActor::on_enter_prep_peak_scene(int type)
{
	TravPeakTeam *trvl_team = TRVL_PEAK_MONITOR->find_travel_team_by_role_id(this->role_id());
	JUDGE_RETURN(trvl_team != NULL, ERROR_NO_TRAVEL_TEAM);

	int ret = TRVL_PEAK_MONITOR->check_teamer_can_enter(trvl_team->__team_id);
	JUDGE_RETURN(ret == 0, ret);

	TrvlPeakPreScene* peak_pre_scene = TRVL_PEAK_MONITOR->fetch_trvl_peak_pre_scene();
	JUDGE_RETURN(peak_pre_scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(peak_pre_scene);
	return MapPlayer::enter_scene(type);
}

int TrvlPeakActor::on_exit_prep_peak_scene(int type)
{
	return 0;
}

int TrvlPeakActor::on_enter_trvl_peak_scene(int type)
{
	TravPeakTeam *trvl_team = TRVL_PEAK_MONITOR->find_travel_team_by_role_id(this->role_id());
	JUDGE_RETURN(trvl_team != NULL, ERROR_NO_TRAVEL_TEAM);

	TrvlPeakScene* scene = TRVL_PEAK_MONITOR->find_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(scene);
	MapPlayer::enter_scene(type);

	this->set_camp_id(trvl_team->__camp_id + 1);

	return 0;
}

int TrvlPeakActor::on_exit_trvl_peak_scene(int type)
{
	return 0;
}

int TrvlPeakActor::request_enter_trvl_peak_scene()
{
	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_TRVL_PEAK);
	enter_info.set_main_version(CONFIG_INSTANCE->main_version());

	int ret = this->send_request_enter_info(this->trvl_peak_prep_scene(), enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRVL_PEAK_ENTER, ret);

	return 0;
}

int TrvlPeakActor::request_match_trvl_peak_quality()
{
	CONDITION_NOTIFY_RETURN(this->scene_id() == this->trvl_peak_prep_scene(), RETURN_MATCH_TRVL_PEAK_QUALITY, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(TRVL_PEAK_MONITOR->is_fighting_time() == true && TRVL_PEAK_MONITOR->is_quality_act_time() == true,
			RETURN_MATCH_TRVL_PEAK_QUALITY, ERROR_GUARD_TIME);

	TrvlPeakPreScene* peak_pre_scene = TRVL_PEAK_MONITOR->fetch_trvl_peak_pre_scene();
	JUDGE_RETURN(peak_pre_scene != NULL, -1);

	TravPeakTeam *trvl_team = TRVL_PEAK_MONITOR->find_travel_team_by_role_id(this->role_id());
	CONDITION_NOTIFY_RETURN(trvl_team != NULL, RETURN_MATCH_TRVL_PEAK_QUALITY, ERROR_NO_TRAVEL_TEAM);
	CONDITION_NOTIFY_RETURN(this->role_id() == trvl_team->__leader_id, RETURN_MATCH_TRVL_PEAK_QUALITY, ERROR_LEADER_CAN_MATCH);
	CONDITION_NOTIFY_RETURN(trvl_team->__start_tick <= ::time(NULL), RETURN_MATCH_TRVL_PEAK_QUALITY, ERROR_OPERATE_TOO_FAST);
	CONDITION_NOTIFY_RETURN(trvl_team->__state == TravPeakTeam::STATE_NONE, RETURN_MATCH_TRVL_PEAK_QUALITY, ERROR_TARENA_FIGHTING);

//	for (TravPeakTeam::TeamerMap::iterator iter = trvl_team->__teamer_map.begin();
//			iter != trvl_team->__teamer_map.end(); ++iter)
//	{
//		MapPlayerEx* player = peak_pre_scene->find_player(iter->first);
//		CONDITION_NOTIFY_RETURN(player != NULL, RETURN_MATCH_TRVL_PEAK_QUALITY, ERROR_TEAMER_NOT_IN_SCENE);
//	}

	TRVL_PEAK_MONITOR->register_travel_peak_team(trvl_team->__team_id);

	for (TravPeakTeam::TeamerMap::iterator iter = trvl_team->__teamer_map.begin();
			iter != trvl_team->__teamer_map.end(); ++iter)
	{
		MapPlayerEx* player = peak_pre_scene->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(RETURN_MATCH_TRVL_PEAK_QUALITY);
	}

	return 0;
}

int TrvlPeakActor::request_unmatch_trvl_peak_quality()
{
	JUDGE_RETURN(this->trvl_peak_prep_scene() == this->scene_id(), -1);

	TravPeakTeam *trvl_team = TRVL_PEAK_MONITOR->find_travel_team_by_role_id(this->role_id());
	JUDGE_RETURN(trvl_team != NULL, -1);
	CONDITION_NOTIFY_RETURN(this->role_id() == trvl_team->__leader_id, RETURN_UNMATCH_TRVL_PEAK_QUALITY, ERROR_LEADER_CAN_CANCLE_MATCH);

	TrvlPeakPreScene* peak_pre_scene = TRVL_PEAK_MONITOR->fetch_trvl_peak_pre_scene();
	JUDGE_RETURN(peak_pre_scene != NULL, -1);

	TRVL_PEAK_MONITOR->unregister_travel_peak_team(trvl_team->__team_id);

	for (TravPeakTeam::TeamerMap::iterator iter = trvl_team->__teamer_map.begin();
			iter != trvl_team->__teamer_map.end(); ++iter)
	{
		MapPlayerEx* player = peak_pre_scene->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(RETURN_UNMATCH_TRVL_PEAK_QUALITY);
	}

	return 0;
}

int TrvlPeakActor::request_trvl_peak_scene_info()
{
	JUDGE_RETURN(this->trvl_peak_prep_scene() == this->scene_id(), -1);

	TravPeakTeam *trvl_team = TRVL_PEAK_MONITOR->find_travel_team_by_role_id(this->role_id());
	JUDGE_RETURN(trvl_team != NULL, -1);

	int fight_type = TRVL_PEAK_MONITOR->fight_type();

	Proto50401704 respond;
	respond.set_fight_type(fight_type);
	respond.set_team_id(trvl_team->__team_id);
	respond.set_team_name(trvl_team->__team_name);
	respond.set_leader_id(trvl_team->__leader_id);
	respond.set_add_exp(TRVL_PEAK_MONITOR->player_exp(this->role_id()));

	IntPair pair = TRVL_PEAK_MONITOR->fetch_act_left_tick();
	respond.set_left_tick(pair.second);
	if (pair.first > 0)
	{
		respond.set_is_prep(0);
	}
	else
	{
		respond.set_is_prep(1);
	}

	for (TravPeakTeam::TeamerMap::iterator iter = trvl_team->__teamer_map.begin();
			iter != trvl_team->__teamer_map.end(); ++iter)
	{
		TravPeakTeam::TravPeakTeamer &teamer = iter->second;
		ProtoTeamer *proto_teamer = respond.add_teamer();
		teamer.serialize(proto_teamer);
	}

	if (fight_type == TrvlPeakMonitor::TIMER_QUALITY)
	{
		ProtoPeakQualityInfo *quality_info = respond.mutable_quality_info();
		quality_info->set_score(trvl_team->__score);
		quality_info->set_rank(trvl_team->__rank);

		const Json::Value& conf = TRVL_PEAK_MONITOR->scene_conf();
		int left_fight_times = conf["no_score_times"].asInt() - trvl_team->__quality_times;
		left_fight_times = left_fight_times >= 0 ? left_fight_times : 0;
		quality_info->set_left_fight_times(left_fight_times);
	}
	else if (fight_type == TrvlPeakMonitor::TIMER_KNOCKOUT)
	{
//		ProtoPeakKnockoutInfo *knockout_info = respond.mutable_knockout_info();
	}
	else
	{
		return -1;
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_TRVL_PEAK_SCENE_INFO, &respond);
}

int TrvlPeakActor::request_trvl_peak_rank_info(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10401705 *, request, msg, -1);

	Proto30402015 inner_req;
	inner_req.set_page(request->page());
	return MAP_MONITOR->dispatch_to_scene(GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);
}

int TrvlPeakActor::handle_exit_trvl_peak_scene(void)
{
	switch (this->scene_id())
	{
	case GameEnum::TRVL_PEAK_SCENE_ID:
	{
		return this->transfer_to_prev_scene();
	}
	default:
	{
		return this->transfer_to_save_scene();
	}
	}
}

int TrvlPeakActor::sync_offline_hook_to_travel_scene(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402003 *, request, msg, -1);

	Int64 team_id = request->team_id(), role_id = request->role_id();

	MapPlayerEx *player = this->find_player(role_id);
	if (player != NULL)
	{
		OfflineRoleDetail role_detail;
		GameCommon::copy_player(&role_detail, player);

		Proto30400432 inner_msg;
		GameCommon::copy_player(&inner_msg, role_detail);

		inner_msg.set_fashion_id(player->fashion_detail().select_id_);
		inner_msg.set_fashion_color(player->fashion_detail().sel_color_id_);
		inner_msg.set_wing_level(player->mount_detail(GameEnum::FUN_XIAN_WING).mount_grade_);
		inner_msg.set_solider_level(player->mount_detail(GameEnum::FUN_GOD_SOLIDER).mount_grade_);
		inner_msg.set_magic_level(player->mount_detail(GameEnum::FUN_MAGIC_EQUIP).mount_grade_);
		inner_msg.set_beast_level(player->mount_detail(GameEnum::FUN_LING_BEAST).mount_grade_);
		inner_msg.set_mount_level(player->mount_detail(GameEnum::FUN_MOUNT).mount_grade_);

		Proto30402002 inner_req;
		inner_req.set_team_id(team_id);
		inner_req.set_src_role_id(role_id);
		inner_req.set_offline_role_recogn(INNER_MAP_LMARTIAL_OFFTRANS);
		inner_req.set_msg_body(inner_msg.SerializeAsString());
		return MAP_MONITOR->dispatch_to_scene(GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);
	}
	else
	{
		DBShopMode *shop_mode = GameCommon::pop_shop_mode();
		shop_mode->recogn_ = TRANS_LOAD_COPY_TRAV_TEAMER;
		shop_mode->input_argv_.type_int64_ = role_id;
		shop_mode->output_argv_.type_void_ = this->monitor()->player_pool()->pop();

		return this->monitor()->db_map_load_mode_begin(shop_mode);
	}

	return 0;
}

int TrvlPeakActor::process_redirect_to_trvl_scene(DBShopMode *shop_mode)
{
	MapPlayerEx *player = (MapPlayerEx *)shop_mode->output_argv_.type_void_;

	JUDGE_RETURN(player != NULL, 0);
	JUDGE_RETURN(shop_mode->output_argv_.bson_obj_->isEmpty() == false, 0);

	player->set_offline_copy_by_bson(*(shop_mode->output_argv_.bson_obj_));

	Int64 src_role_id = shop_mode->input_argv_.type_int64_;

	OfflineRoleDetail role_detail;
	GameCommon::copy_player(&role_detail, player);

	Proto30400432 inner_msg;
	GameCommon::copy_player(&inner_msg, role_detail);

	inner_msg.set_fashion_id(player->fashion_detail().select_id_);
	inner_msg.set_fashion_color(player->fashion_detail().sel_color_id_);
	inner_msg.set_wing_level(player->mount_detail(GameEnum::FUN_XIAN_WING).mount_grade_);
	inner_msg.set_solider_level(player->mount_detail(GameEnum::FUN_GOD_SOLIDER).mount_grade_);
	inner_msg.set_magic_level(player->mount_detail(GameEnum::FUN_MAGIC_EQUIP).mount_grade_);
	inner_msg.set_beast_level(player->mount_detail(GameEnum::FUN_LING_BEAST).mount_grade_);
	inner_msg.set_mount_level(player->mount_detail(GameEnum::FUN_MOUNT).mount_grade_);

	Proto30402002 inner_req;
	inner_req.set_team_id(0);
	inner_req.set_src_role_id(src_role_id);
	inner_req.set_offline_role_recogn(INNER_MAP_LMARTIAL_OFFTRANS);
	inner_req.set_msg_body(inner_msg.SerializeAsString());
	return MAP_MONITOR->dispatch_to_scene(GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);

}

