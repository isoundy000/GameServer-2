/*
 * LogicLeaguer.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: peizhibi
 */

#include "LogicLeaguer.h"

#include "GameFont.h"
#include "MMOLeague.h"
#include "ProtoDefine.h"
#include "LeagueSystem.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"

#include "GameField.h"
#include "GameCommon.h"
#include "OpenActivitySys.h"
#include "RankSystem.h"

LogicLeaguer::LogicLeaguer()
{
	// TODO Auto-generated constructor stub
}

LogicLeaguer::~LogicLeaguer()
{
	// TODO Auto-generated destructor stub
}

void LogicLeaguer::reset()
{
	this->leaguer_info_.reset();
}

void LogicLeaguer::reset_every_day()
{
	this->leaguer_info_.buy_map_.clear();
	this->leaguer_info_.region_draw_.clear();

	this->leaguer_info_.salary_flag_ = 0;
	this->leaguer_info_.store_times_ = 0;
	this->leaguer_info_.draw_welfare_ = false;

	this->leaguer_info_.gold_donate_ = 0;
	this->leaguer_info_.wand_donate_ = 0;

	this->reset_lfb_wave_reward();
}

void LogicLeaguer::reset_lfb_wave_reward()
{
	this->leaguer_info_.wave_reward_map_.clear();

	GameConfig::ConfigMap &conf_map = CONFIG_INSTANCE->lfb_wave_reward_map();
	for (GameConfig::ConfigMap::iterator iter = conf_map.begin();
			iter != conf_map.end(); ++iter)
	{
		const Json::Value &conf = *(iter->second);
		const Json::Value &wave_reward_json = conf["wave_reward"];
		IntMap wave_reward;
		for (uint i = 0; i < wave_reward_json.size(); ++i)
		{
			int amount = wave_reward_json[i][0u].asInt();
			wave_reward[amount] = false;
		}
		this->leaguer_info_.wave_reward_map_.insert(LeaguerInfo::TypeItemNumberMap::value_type(iter->first, wave_reward));
	}
}

void LogicLeaguer::update_contri_to_client()
{
	Proto80400390 active;
	active.set_cur_contri(this->leaguer_info_.cur_contri_);
	this->respond_to_client(ACTIVE_UPDATE_CONTRI, &active);
}

int LogicLeaguer::league_handle_player_task()
{
	LeaguerInfo &leaguer = this->leaguer_info_;
	leaguer.open_ = true;

	return 0;
}

void LogicLeaguer::league_handle_player_levelup()
{
	LeaguerInfo &leaguer = this->leaguer_info_;
	JUDGE_RETURN(leaguer.open_ == false, ;);

	int cur_level = this->role_level();
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_level(
			"fun_league", cur_level) == true, ;);

	leaguer.open_ = true;
}

void LogicLeaguer::leaguer_check_open()
{
	LeaguerInfo &leaguer = this->leaguer_info_;
	JUDGE_RETURN(leaguer.open_ == false, ;);

	int ckeck_level = CONFIG_INSTANCE->league("check_open_level").asInt();
	JUDGE_RETURN(this->role_level() >= ckeck_level, ;);

	leaguer.open_ = true;
}

void LogicLeaguer::upgrade_cur_contri(LeagueMember* member, int contri_num, int add_num)
{
	this->leaguer_info_.cur_contri_ += add_num;
	member->cur_contribute_ = this->leaguer_info_.cur_contri_;

	//技能红点
	this->check_skill_red_point();

	JUDGE_RETURN(add_num > 0, ;);

	member->total_contribute_ += add_num;
	member->today_contribute_ += add_num;

	this->notify_one_tips_info(GameEnum::TIPS_ITEM,
			GameEnum::ITEM_ID_LEAGUE_CONTRI, add_num);

	League* league = this->league();
	JUDGE_RETURN(league != NULL, ;);

	league->sort_rank();
//	//update achievement
//	Proto31401901 info;
//	info.set_cur_value(value);
//	info.set_achieve_index(GameEnum::LEAGUE);
//	LOGIC_MONITOR->dispatch_to_scene(this, &info);
}

Int64 LogicLeaguer::league_index()
{
	return this->role_detail().__league_id;
}

void LogicLeaguer::leaguer_sign_in()
{
	League* league = this->league();
	if (league == NULL && this->league_index() > 0)
	{
		GameCommon::request_save_role_long(this->role_id(), Role::LEAGUE_ID, 0);
	}

	JUDGE_RETURN(league != NULL, ;);

	LeagueMember* member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, ;);

//	this->upgrade_cur_contri(member, member->offline_contri_);
	this->leaguer_info_.apply_list_.clear();

	member->offline_contri_ = 0;
	member->role_lvl_ = this->role_level();
	league->last_login_tick_ = ::time(NULL);

	//变性bug检测
	member->role_sex_ = this->role_detail().__sex;
	member->role_career_ = this->role_detail().__career;

	this->handle_leader_sign();
}

void LogicLeaguer::leaguer_sign_out()
{
	LeagueMember* member = this->league_member();
	JUDGE_RETURN(member != NULL, ;);

	member->offline_tick_ = ::time(NULL);
}

League* LogicLeaguer::league()
{
	JUDGE_RETURN(this->league_index() > 0, NULL);
	League* league = this->find_league(this->league_index());

	JUDGE_RETURN(league != NULL, NULL);
	if(league->validate_member(this->role_id()) == false)
	{
		this->role_detail().__league_id = 0;
		return NULL;
	}

	return league;
}

LeagueMember* LogicLeaguer::league_member()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, NULL);

	return league->league_member(this->role_id());
}

League* LogicLeaguer::find_league(Int64 league_index)
{
	return LEAGUE_PACKAGE->find_object(league_index);
}

LeaguerInfo& LogicLeaguer::leaguer_info()
{
	return this->leaguer_info_;
}

bool LogicLeaguer::is_league_leader()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, false);

	return league->is_leader(this->role_id());
}

int LogicLeaguer::open_league()
{
	Proto50100622 league_info;
	league_info.set_league_flag(this->league() != NULL ? true : false);
	FINER_PROCESS_RETURN(RETURN_LEAGUE_OPEN, &league_info);
}

int LogicLeaguer::fetch_league_list_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100601*, request, RETURN_LEAGUE_LIST_INFO);

	LongVec league_set;
	LEAGUE_SYSTEM->fetch_league_set(request->show_join(), league_set);

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page_index(),
			league_set.size(), GameEnum::LEAGUE_LIST_PAGE_COUNT);

	Proto50100601 list_info;
	list_info.set_page_index(page_info.cur_page_);
	list_info.set_total_page(page_info.total_page_);

	int rank_index = page_info.start_index_ + 1;
	for (LongVec::iterator iter = league_set.begin() + page_info.start_index_;
			iter != league_set.end(); ++iter)
	{
		League* league = this->find_league(*iter);
		JUDGE_CONTINUE(league != NULL);

		ProtoLeagueItem* league_item = list_info.add_league_set();
		JUDGE_CONTINUE(league_item != NULL);

		league->make_up_league_list_info(this->role_id(), league_item);
		league_item->set_rank_index(rank_index);

		rank_index += 1;
		page_info.add_count_ += 1;

		JUDGE_BREAK(page_info.add_count_ < GameEnum::LEAGUE_LIST_PAGE_COUNT);
	}

	FINER_PROCESS_RETURN(RETURN_LEAGUE_LIST_INFO, &list_info);
}

int LogicLeaguer::create_league(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100602*, request, RETURN_LEAGUE_CREATE);

	CONDITION_NOTIFY_RETURN(this->leaguer_info_.open_ == true,
			RETURN_LEAGUE_CREATE, ERROR_BACK_GAME_SWITCH_CLOSE);
	CONDITION_NOTIFY_RETURN(this->league() == NULL, RETURN_LEAGUE_CREATE,
			ERROR_HAVE_LEAGUE);

    char buffer[GameEnum::MAX_LEAGUE_NAME_LENGTH * 6 + 1] = {0,};
    string_remove_black_char(buffer, GameEnum::MAX_LEAGUE_NAME_LENGTH * 6,
    		request->league_name().c_str(), request->league_name().length());

	string league_name = buffer;
	CONDITION_NOTIFY_RETURN(GameCommon::validate_chinese_name_length(league_name,
			GameEnum::MAX_LEAGUE_NAME_LENGTH) == true,
			RETURN_LEAGUE_CREATE, ERROR_LEAGUE_NAME_LENGTH);

	CONDITION_NOTIFY_RETURN(LEAGUE_SYSTEM->is_league_name_same(league_name) == false,
			RETURN_LEAGUE_CREATE, ERROR_LEAGUE_NAME_SAME);

	string league_intro = request->league_intro();
	CONDITION_NOTIFY_RETURN(GameCommon::validate_name_length(league_intro,
			GameEnum::MAX_LEAGUE_INTRO_LENGTH) == true,
			RETURN_LEAGUE_CREATE, ERROR_LEAGUE_INTRO_LENGTH);

	Proto31400302 create_info;
	create_info.set_league_name(league_name);
	create_info.set_league_intro(request->league_intro());
	create_info.set_create_type(request->create_type());
	return LOGIC_MONITOR->dispatch_to_scene(this, &create_info);
}

int LogicLeaguer::map_create_league(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400302*, request, RETURN_LEAGUE_CREATE);
	CONDITION_NOTIFY_RETURN(this->league() == NULL, RETURN_LEAGUE_CREATE,
			ERROR_HAVE_LEAGUE);

	Int64 league_index = LEAGUE_SYSTEM->fetch_new_league_index();
	CONDITION_NOTIFY_RETURN(league_index > 0, RETURN_LEAGUE_LIST_INFO,
			ERROR_SERVER_INNER);
	CONDITION_NOTIFY_RETURN(LEAGUE_PACKAGE->find_object(league_index) == NULL,
			RETURN_LEAGUE_LIST_INFO, ERROR_SERVER_INNER);

	League* league = LEAGUE_PACKAGE->pop_object();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_LIST_INFO,
			ERROR_SERVER_INNER);

	// initial
	league->league_index_ = league_index;
	LEAGUE_PACKAGE->bind_object(league->league_index_, league);

	league->create_tick_ = ::time(NULL);
	league->last_login_tick_ = ::time(NULL);
	league->leader_index_ = this->role_id();

	league->league_name_ = request->league_name();
	league->league_intro_ = request->league_intro();
	league->flag_lvl_ = 1;

	league->league_lvl_ = LEAGUE_SYSTEM->creat_league_lvl(request->create_type());
	league->handle_join_league(this->role_id(), GameEnum::LEAGUE_POS_LEADER);

	// sort and save
	LEAGUE_SYSTEM->sort_league();
	LEAGUE_SYSTEM->erase_all_apply(this->role_id());
	// 创建帮派boss(帮派驻地)场景
	LEAGUE_SYSTEM->sync_league_boss_info(league);

//	BrocastParaVec para_vec;
//	GameCommon::push_brocast_para_string(para_vec, this->name());
//	GameCommon::push_brocast_para_int(para_vec, request->create_type());
//	this->monitor()->announce_world(SHOUT_ALL_CREATE_LEAGUE, para_vec);

	//chat transactions
	this->logic_player()->chat_establish_league(league_index);
	this->logic_player()->chat_join_league(league_index, league->league_name_);
	MMOLeague::save_league(league);

	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_CREATE);
}

int LogicLeaguer::request_rename_league(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400024*, request, -1);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_RENAME_LEAGUE,
			ERROR_LEAGUE_NO_EXIST);

	if (league->is_leader(this->role_id()) == false)
	{
		LOGIC_MONITOR->dispatch_to_scene(this, request);
		return this->respond_to_client_error(RETURN_RENAME_LEAGUE, ERROR_LEAGE_LEADER_CAN_RENAME);
	}

	if (LEAGUE_SYSTEM->is_league_name_same(request->name()) == true)
	{
		LOGIC_MONITOR->dispatch_to_scene(this, request);
		return this->respond_to_client_error(RETURN_RENAME_LEAGUE, ERROR_LEAGUE_NAME_SAME);
	}

	request->set_operate(true);
	string pre_name = league->league_name_;
	league->league_name_ = request->name();
	league->change_name_log(GameEnum::LEAGUE_LOG_CHANGENAME,this->role_detail().name());

	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->send_map_league_info();
	}

	LEAGUE_SYSTEM->save_league(league->league_index_);
	return LOGIC_MONITOR->dispatch_to_scene(this, request);
}

int LogicLeaguer::handle_resex_leaguer()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	LeagueMember *member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, 0);

	member->role_sex_ = this->role_detail().__sex;
	member->role_career_ = this->role_detail().__career;

	LFbPlayer *lfb_player = league->lfb_player(this->role_id());
	JUDGE_RETURN(lfb_player != NULL, 0);

	lfb_player->sex_ = this->role_detail().__sex;

	return 0;
}

int LogicLeaguer::handle_rename_leaguer()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	LeagueMember *member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, 0);

	member->role_name_ = this->name();

	LFbPlayer *lfb_player = league->lfb_player(this->role_id());
	JUDGE_RETURN(lfb_player != NULL, 0);
	lfb_player->name_ = this->name();

	JUDGE_RETURN(this->role_id() == league->leader_index_, 0);
	LEAGUE_SYSTEM->sync_leader_to_map(league);

	return 0;
}

int LogicLeaguer::handle_leader_sign()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);
	JUDGE_RETURN(league->is_leader(this->role_id()) == true, 0);
	JUDGE_RETURN(league->is_in_impeach() == true, 0);

	LeagueImpeach &league_impeach = league->league_impeach_;
	league_impeach.reset();

	return 0;
}

int LogicLeaguer::request_check_same_league(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100648*, request, RETURN_LEAGUE_IS_SAME);

	Proto50100648 same_info;
	League* league = this->league();

	if (league != NULL && league->validate_member(request->role_id()) == true)
	{
		same_info.set_same_flag(true);
	}
	else
	{
		same_info.set_same_flag(false);
	}

	FINER_PROCESS_RETURN(RETURN_LEAGUE_IS_SAME, &same_info);
}

int LogicLeaguer::apply_join_league(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100604*, request, RETURN_LEAGUE_APPLY_JOIN);

	CONDITION_NOTIFY_RETURN(this->leaguer_info_.open_ == true,
			RETURN_LEAGUE_APPLY_JOIN, ERROR_BACK_GAME_SWITCH_CLOSE);
	CONDITION_NOTIFY_RETURN(this->league() == NULL, RETURN_LEAGUE_APPLY_JOIN,
			ERROR_HAVE_LEAGUE);

	int join_level = CONFIG_INSTANCE->league("join_level").asInt();
	CONDITION_NOTIFY_RETURN(this->role_level() >= join_level,
			RETURN_LEAGUE_APPLY_JOIN, ERROR_PLAYER_LEVEL);

	Proto50100604 respond;
	for (int i = 0; i < request->apply_set_size(); ++i)
	{
		Int64 league_index = request->apply_set(i);
		League* league = this->find_league(league_index);
		JUDGE_CONTINUE(league != NULL);
		JUDGE_CONTINUE(league->league_full() == false);

		respond.add_apply_set(league_index);

		league->push_applier(this->role_id());
		this->leaguer_info_.apply_list_[league_index] = true;
		this->handle_league_apply(league);

		JUDGE_CONTINUE(league->arrive_auto_accept(this->role_id()) == true);

		league->handle_join_league(this->role_id(), GameEnum::LEAGUE_POS_NONE);
		this->notify_join_league(this->role_id());

		FINER_PROCESS_NOTIFY(RETURN_LEAGUE_APPLY_JOIN);
	}
	FINER_PROCESS_RETURN(RETURN_LEAGUE_APPLY_JOIN, &respond);
}

int LogicLeaguer::cancel_join_league(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100605*, request, RETURN_LEAGUE_CANCEL_APPLY);

	League* league = this->find_league(request->league_index());
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_CANCEL_APPLY,
			ERROR_LEAGUE_NO_EXIST);

	league->erase_applier(this->role_id());
	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_CANCEL_APPLY);
}

int LogicLeaguer::handle_league_apply(League* league)
{
	JUDGE_RETURN(league != NULL, 0);

	League::MemberMap member_map = league->member_map_;
	for (League::MemberMap::iterator iter = member_map.begin();
			iter != member_map.end(); ++iter)
	{
		JUDGE_CONTINUE(league->can_view_apply(iter->first) == true);
		this->send_apply_red_point(league, iter->first);
	}

	return 0;
}

int LogicLeaguer::send_apply_red_point(League* league, Int64 role_id)
{
	JUDGE_RETURN(league != NULL, 0);

	LogicPlayer *player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, 0);

	if (league->can_view_apply(role_id) == true)
	{
		if (league->applier_map_.size() > 0)
			player->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_ACCEPT, 1);
		else
			player->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_ACCEPT, 0);
	}

	return 0;
}

int LogicLeaguer::fetch_leader_inpeach()
{
	// 检测是否弹劾帮主
	this->check_impeach_time();

	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	LeagueImpeach &league_impeach = league->league_impeach_;
	int left = league_impeach.impeach_tick_ - ::time(NULL);
	left = left > 0 ? left : 0;

	Proto50100636 respond;
	respond.set_left_tick(left);
	FINER_PROCESS_RETURN(RETURN_REQUEST_LEADER_IMPEACH, &respond);
}

int LogicLeaguer::fetch_league_info()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_INFO,
			ERROR_LEAGUE_NO_EXIST);
//	JUDGE_RETURN(league != NULL, 0);

	LeagueMember* league_member = league->league_member(this->role_id());
	CONDITION_NOTIFY_RETURN(league_member != NULL, RETURN_LEAGUE_INFO,
			ERROR_CLIENT_OPERATE);

	const Json::Value &league_item = league->set_up();
	CONDITION_NOTIFY_RETURN(league_item != Json::Value::null,
			RETURN_LEAGUE_INFO,	ERROR_CLIENT_OPERATE);

	//福利红点
	if (this->leaguer_info_.draw_welfare_ == false)
	{
		this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_WELFARE, 1);
	}

	//申请列表红点
	this->send_apply_red_point(league, this->role_id());

	//技能红点
	this->check_skill_red_point();

	//帮派旗帜升级红点
	int flag_lvl = league->flag_lvl_;
	int flag_exp = league->flag_exp_;
	const Json::Value &next_flag_info = CONFIG_INSTANCE->league_flag(flag_lvl+1);
	if (next_flag_info != Json::Value::null)
	{
		const Json::Value &flag_info = CONFIG_INSTANCE->league_flag(flag_lvl);
		int need_exp = flag_info["upgrade_exp"].asInt();
		if (flag_exp >= need_exp)
			this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_FLAG, 1);
		else
			this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_FLAG, 0);
	}
	else
	{
		this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_FLAG, 0);
	}

	Proto50100606 league_info;
	league_info.set_league_index(league->league_index_);
	league_info.set_league_name(league->league_name_);
	league_info.set_league_count(league->member_map_.size());
	league_info.set_league_rank(league->league_rank_);
	league_info.set_league_pos(league_member->league_pos_);
	league_info.set_auto_accept(league->auto_accept_);

	int max_member = LEAGUE_SYSTEM->max_member(league->league_lvl_);
	league_info.set_max_role(max_member);

	league_info.set_leader_id(league->leader_index_);
	league_info.set_leader_name(league->leader_name());

	league_info.set_league_lvl(league->league_lvl_);
	league_info.set_league_intro(league->league_intro_);
	league_info.set_league_resource(league->league_resource_);
	league_info.set_total_resource(league_item["upgrade_resource"].asInt());

//	int need_contri = CONFIG_INSTANCE->league_store()["need_contribute"].asInt();
//	if(this->league_member()->league_pos_ == GameEnum::LEAGUE_POS_NONE &&
//			this->league_member()->total_contribute_ < need_contri)
//	{
//		league_info.set_is_lstore(0);
//	}
//	else
//	{
//		league_info.set_is_lstore(1);
//	}
	LeagueMember* league_leader = league->league_member(league->leader_index_);
	league_info.set_leader_career(league_leader->role_career_);
	league_info.set_leader_sex(league_leader->role_sex_);

	LogicPlayer* player = this->find_player(league->leader_index_);
	if (player == NULL)
	{
		RankRecord *god_solider = RANK_SYS->fetch_rank_record(RANK_FUN_GOD_SOLIDER, league->leader_index_);
		if (god_solider != NULL)
			league_info.set_leader_weapons(god_solider->__single_player_info.__rank_value);

		RankRecord *wings = RANK_SYS->fetch_rank_record(RANK_FUN_XIAN_WING, league->leader_index_);
		if (wings != NULL)
			league_info.set_leader_wings(wings->__single_player_info.__rank_value);

		league_info.set_fashion_id(league_leader->fashion_id_);
		league_info.set_fashion_color(league_leader->fashion_color_);
	}
	else
	{
		if (player->role_detail().mount_info_.count(GameEnum::FUN_GOD_SOLIDER) > 0)
		{
			ThreeObj &obj = player->role_detail().mount_info_[GameEnum::FUN_GOD_SOLIDER];
			league_info.set_leader_weapons(obj.tick_);
		}
		else
		{
			league_info.set_leader_weapons(0);
		}

		if (player->role_detail().mount_info_.count(GameEnum::FUN_XIAN_WING) > 0)
		{
			ThreeObj &obj = player->role_detail().mount_info_[GameEnum::FUN_XIAN_WING];
			league_info.set_leader_wings(obj.tick_);
		}
		else
		{
			league_info.set_leader_wings(0);
		}

		league_info.set_fashion_id(player->role_detail().fashion_id_);
		league_info.set_fashion_color(player->role_detail().fashion_color_);
	}

	FINER_PROCESS_RETURN(RETURN_LEAGUE_INFO, &league_info);
}

int LogicLeaguer::fetch_other_league_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100653*, request, CLIENT_OTHER_LEAGUE_INFO);

	League* league = this->find_league(request->league_index());

	CONDITION_NOTIFY_RETURN(league != NULL, CLIENT_OTHER_LEAGUE_INFO,
			ERROR_LEAGUE_NO_EXIST);

	Proto50100653 league_info;
	league_info.set_league_index(league->league_index_);
	league_info.set_league_name(league->league_name_);
	league_info.set_leader_name(league->leader_name());
	league_info.set_league_count(league->member_map_.size());
	league_info.set_league_rank(league->league_rank_);
	league_info.set_league_lvl(league->league_lvl_);
	league_info.set_league_force(league->league_force_);
	league_info.set_flag_lvl(league->flag_lvl_);

	PairObjVec role_set;
	league->fetch_sort_member(role_set);

	for (PairObjVec::iterator iter = role_set.begin(); iter != role_set.end();	++iter)
	{
		LeagueMember* league_member = league->league_member(iter->id_);
		JUDGE_CONTINUE(league_member != NULL);

		ProtoLeagueMember* member_info = league_info.add_member_set();
		JUDGE_CONTINUE(member_info != NULL);

		member_info->set_left_time(iter->value_);
		member_info->set_role_index(league_member->role_index_);
		member_info->set_role_name(league_member->role_name_);
		member_info->set_league_pos(league_member->league_pos_);
		member_info->set_role_lvl(league_member->role_lvl_);
		member_info->set_sex(league_member->role_sex_);
		member_info->set_vip(league_member->vip_type_);
		member_info->set_role_force(league_member->new_role_force_);
		member_info->set_cur_contri(league_member->cur_contribute_);
		member_info->set_total_contri(league_member->total_contribute_);
	}

	FINER_PROCESS_RETURN(CLIENT_OTHER_LEAGUE_INFO, &league_info);
}

int LogicLeaguer::fetch_league_boss_info()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_BOSS_INFO,
				ERROR_LEAGUE_NO_EXIST);

	Int64 reset_tick = league->league_boss_.reset_tick_;
	league->league_boss_.reset_everyday();

	if (reset_tick != league->league_boss_.reset_tick_)
	{
		MMOLeague::save_league(league);
	}

	Proto50100649 boss_info;
	boss_info.set_boss_index(league->league_boss_.boss_index_);
	boss_info.set_boss_exp(league->league_boss_.boss_exp_);
	boss_info.set_super_summon_role(league->league_boss_.super_summon_role_);
	boss_info.set_normal_summon_type(league->league_boss_.normal_summon_type_);
	boss_info.set_super_summon_type(league->league_boss_.super_summon_type_);
	boss_info.set_normal_summon_tick(league->league_boss_.normal_summon_tick_);
	boss_info.set_super_summon_tick(league->league_boss_.super_summon_tick_);

	FINER_PROCESS_RETURN(RETURN_LEAGUE_BOSS_INFO, &boss_info);
}

int LogicLeaguer::feed_league_boss(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100650*, request, RETURN_FEED_LEAGUE_BOSS);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_FEED_LEAGUE_BOSS,
					ERROR_LEAGUE_NO_EXIST);

	JUDGE_RETURN(request->item_id() > 0 && request->feed_num() > 0, 0);

	CONDITION_NOTIFY_RETURN(request->item_id() == CONFIG_INSTANCE->league_boss("feed_id").asInt(),
			RETURN_FEED_LEAGUE_BOSS, ERROR_FEED_BOOS_ITEM);

	LeagueBossInfo &league_boss = league->league_boss_;
	CONDITION_NOTIFY_RETURN(league_boss.normal_summon_tick_ == 0 && league_boss.super_summon_tick_ == 0,
			RETURN_FEED_LEAGUE_BOSS, ERROR_BOSS_HAS_SUMMON);

	int boss_index = league_boss.boss_index_;
	const Json::Value& boss_info 	  = CONFIG_INSTANCE->boss_info(boss_index);
	const Json::Value& next_boss_info = CONFIG_INSTANCE->boss_info(boss_index + 1);
	CONDITION_NOTIFY_RETURN(boss_info != Json::Value::null && next_boss_info != Json::Value::null,
			RETURN_FEED_LEAGUE_BOSS, ERROR_CONFIG_NOT_EXIST);

	CONDITION_NOTIFY_RETURN(next_boss_info["is_highest"].asInt() == 0,
			RETURN_FEED_LEAGUE_BOSS, ERROR_BOSS_LV_MAX);

	Proto31400321 feed_info;
	feed_info.set_item_id(request->item_id());
	feed_info.set_feed_num(request->feed_num());
	return LOGIC_MONITOR->dispatch_to_scene(this, &feed_info);
}

int LogicLeaguer::map_feed_league_boss(Message* msg)
{
	League* league = this->league();
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400321*, request, RETURN_FEED_LEAGUE_BOSS);
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_FEED_LEAGUE_BOSS,
				ERROR_LEAGUE_NO_EXIST);

	int boss_exp  = CONFIG_INSTANCE->boss_attr(league->league_boss_.boss_index_, "exp");
	JUDGE_RETURN(boss_exp != -1, 0);

	int feed_num  = request->feed_num();
	int basic_exp = CONFIG_INSTANCE->league_boss("add_exp").asInt();
	int add_exp   = basic_exp * feed_num;
	int total_exp = league->league_boss_.boss_exp_ + add_exp;
	while(total_exp >= boss_exp)
	{
		total_exp -= boss_exp;
		++league->league_boss_.boss_index_;
		int is_highest = CONFIG_INSTANCE->boss_attr(league->league_boss_.boss_index_, "is_highest");
		JUDGE_RETURN(is_highest != -1, 0);

		if (is_highest >= 1)
		{
			--league->league_boss_.boss_index_;
			total_exp = 0;
			break;
		}
		else
		{
			boss_exp  = CONFIG_INSTANCE->boss_attr(league->league_boss_.boss_index_, "exp");
			JUDGE_RETURN(boss_exp != -1, 0);
		}
	}
	league->league_boss_.boss_exp_ = total_exp;
	MMOLeague::save_league(league);

	this->fetch_league_boss_info();

	FINER_PROCESS_NOTIFY(RETURN_FEED_LEAGUE_BOSS);
}

int LogicLeaguer::summon_boss(Message* msg)
{
	League* league = this->league();
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100651*, request, RETURN_SUMMON_BOSS);
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_SUMMON_BOSS,
				ERROR_LEAGUE_NO_EXIST);

	JUDGE_RETURN(this->add_validate_operate_tick() == true, 0);

	Int64 cur_time = ::time(0);
	CONDITION_NOTIFY_RETURN((cur_time <= league->league_boss_.end_tick_)
			&&(cur_time >= league->league_boss_.start_tick_),
			RETURN_SUMMON_BOSS, ERROR_NOT_IN_SUMMON_TIME);

	int summon_type = request->summon_type();
	CONDITION_NOTIFY_RETURN((summon_type == 1)||(summon_type == 2), RETURN_SUMMON_BOSS,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN((league->league_boss_.normal_summon_type_ == false)
			&&(league->league_boss_.super_summon_type_ == false),
			RETURN_SUMMON_BOSS, ERROR_HAS_LEAGUE_BOSS);

	int boss_index = league->league_boss_.boss_index_;
	if (summon_type == 1)
	{
		CONDITION_NOTIFY_RETURN(league->can_operate_league(this->role_id()),
				RETURN_SUMMON_BOSS, ERROR_POSITION_NOT_ENOUGH);
		CONDITION_NOTIFY_RETURN(league->league_boss_.normal_summon_tick_ == 0, RETURN_SUMMON_BOSS,
				ERROR_BOSS_HAS_SUMMON);

		const Json::Value& boss_info = CONFIG_INSTANCE->boss_info(boss_index);
		CONDITION_NOTIFY_RETURN(boss_info != Json::Value::null, RETURN_SUMMON_BOSS,
				ERROR_CONFIG_NOT_EXIST);
	}
	else
	{
		CONDITION_NOTIFY_RETURN(league->league_boss_.super_summon_tick_ == 0, RETURN_SUMMON_BOSS,
				ERROR_BOSS_HAS_SUMMON);

		boss_index += 1;
		const Json::Value& next_boss_info = CONFIG_INSTANCE->boss_info(boss_index);
		CONDITION_NOTIFY_RETURN(next_boss_info != Json::Value::null, RETURN_SUMMON_BOSS,
				ERROR_CONFIG_NOT_EXIST);
	}

	Proto31400322 boss_summon;
	boss_summon.set_summon_type(request->summon_type());
//	boss_summon.set_role_id(request->role_id());
	boss_summon.set_boss_index(boss_index);
	boss_summon.set_league_index(league->league_index_);
	boss_summon.set_flag_lv(league->flag_lvl_);
	return LOGIC_MONITOR->dispatch_to_scene(this, &boss_summon);
}

int LogicLeaguer::map_summon_boss(Message* msg)
{
	League* league = this->league();
	MSG_DYNAMIC_CAST_NOTIFY(Proto30400451*, request, RETURN_SUMMON_BOSS);
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_SUMMON_BOSS,
					ERROR_LEAGUE_NO_EXIST);

	int summon_type = request->summon_type();
	if (summon_type == 1)
	{
		league->league_boss_.normal_summon_tick_ = ::time(0);
		league->league_boss_.normal_summon_type_ = true;
	}
	else
	{
		league->league_boss_.super_summon_role_ = this->name();
		league->league_boss_.super_summon_tick_ = ::time(0);
		league->league_boss_.super_summon_type_ = true;
	}
	MMOLeague::save_league(league);

	this->fetch_league_boss_info();

	FINER_PROCESS_NOTIFY(RETURN_SUMMON_BOSS);
}

int LogicLeaguer::request_enter_league_boss()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_ENTER_LEAGUE_BOSS,
			ERROR_LEAGUE_NO_EXIST);

	if (this->leaguer_info_.leave_type_ > 0)
	{
		int leave_time = ::time(NULL) - this->leaguer_info_.leave_tick_;
		int span_tick = CONFIG_INSTANCE->league("enter_lboss_span").asInt();
		CONDITION_NOTIFY_RETURN(leave_time > span_tick, RETURN_ENTER_LEAGUE_BOSS, ERROR_LIMIT_TIME_TO_LBOSS);
	}

	return LOGIC_MONITOR->dispatch_to_scene(this->gate_sid(), this->role_id(),
			this->scene_id(), INNER_MAP_LEAGUE_BOSS_ENTER);
}

int LogicLeaguer::test_reset_boss(int boss_lvl)
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, -1);

	if (boss_lvl <= 0)
		boss_lvl = 1;

	while (true)
	{
		const Json::Value& boss_info = CONFIG_INSTANCE->boss_info(boss_lvl + 1);
		if (boss_info != Json::Value::null)
			break;
		boss_lvl--;
	}

	Int64 cur_time = ::time(0);
	league->league_boss_.start_tick_ = cur_time;
	league->league_boss_.end_tick_   = cur_time + 1800;
	league->league_boss_.boss_index_ = boss_lvl;
	league->league_boss_.boss_exp_   = 0;
	league->league_boss_.normal_summon_type_= false;
	league->league_boss_.super_summon_type_ = false;
	league->league_boss_.normal_summon_tick_= 0;
	league->league_boss_.super_summon_tick_ = 0;
	league->league_boss_.normal_die_tick_   = 0;
	league->league_boss_.super_die_tick_    = 0;

	return 0;
}

int LogicLeaguer::refresh_escort_quality(Message* msg)
{
	return 0;
}

int LogicLeaguer::open_escort_info()
{
	return 0;
}

int LogicLeaguer::select_escort_car_type(Message* msg)
{
	return 0;
}

int LogicLeaguer::quit_league()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_QUIT,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(LEAGUE_SYSTEM->validate_quit_league(this->scene_id())
			== true, RETURN_LEAGUE_QUIT, ERROR_LEAGUE_QUIT_SCENE);

	if (league->member_map_.size() > 1)
	{
		CONDITION_NOTIFY_RETURN(league->is_leader(this->role_id()) == false,
				RETURN_LEAGUE_QUIT, ERROR_LEADER_CAN_NOT_LEAVE);
	}

	Proto30400445 request;
	request.set_league_id(this->league_index());
	request.set_role_id(this->role_id());
	return LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::LBOSS_SCENE_ID, &request);
}

int LogicLeaguer::map_quit_league(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400445*, request, -1);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_QUIT,
			ERROR_LEAGUE_NO_EXIST);

	league->handle_quit_league(this->role_id(), GameEnum::LEAGUE_LOG_MEMBER_QUIT);
	LEAGUE_SYSTEM->check_and_dismiss_league(league);

	//取消所有帮派红点
	this->quit_league_miss_red_point();
	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_QUIT);
}

int LogicLeaguer::fetch_league_shop_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100624*, request, RETURN_LEAGUE_SHOP_INFO);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_SHOP_INFO,
			ERROR_LEAGUE_NO_EXIST);

	return this->fetch_league_shop_info(request->shop_type());
}

int LogicLeaguer::league_shop_buy(Message* msg)
{
	return 0;
}

int LogicLeaguer::map_league_shop_buy(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400303*, request, -1);

	if (request->oper_result() == true)
	{
		this->leaguer_info_.buy_map_[request->buy_item()] += request->buy_num();
		this->fetch_league_shop_info(request->shop_type());
	}
	else
	{
		LeagueMember* member = this->league_member();
		JUDGE_RETURN(member != NULL, -1);

		JUDGE_RETURN(request->need_type() == GameEnum::MONEY_CONTRIBUTE, -1);
//		this->upgrade_cur_contri(member, request->need_amount());
	}

	return 0;
}

int LogicLeaguer::modify_league_intro(Message* msg)
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_MODIFY_INTRO,
			ERROR_LEAGUE_NO_EXIST);
	CONDITION_NOTIFY_RETURN(league->can_operate_league(this->role_id()) == true,
			RETURN_LEAGUE_MODIFY_INTRO, ERROR_NO_LEAGUE_RIGHT);

	MSG_DYNAMIC_CAST_NOTIFY(Proto10100611*, request, RETURN_LEAGUE_MODIFY_INTRO);

	league->league_intro_ = request->league_intro();
	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_MODIFY_INTRO);
}

int LogicLeaguer::fetch_league_welfare()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_OBTAIN_WELFARE,
			ERROR_LEAGUE_NO_EXIST);

	LeagueMember* member = league->league_member(this->role_id());
	CONDITION_NOTIFY_RETURN(member != NULL, RETURN_LEAGUE_OBTAIN_WELFARE,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->leaguer_info_.draw_welfare_ == false,
			RETURN_LEAGUE_OBTAIN_WELFARE, ERROR_REWARD_DRAWED);

	const Json::Value& league_item = LEAGUE_SYSTEM->league_set_up(league->league_lvl_);
	CONDITION_NOTIFY_RETURN(league_item != Json::Value::null,
			RETURN_LEAGUE_OBTAIN_WELFARE, ERROR_SERVER_INNER);

	this->leaguer_info_.draw_welfare_ = true;
	this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_WELFARE, 0);

	int welfare_id = league_item["welfare"].asInt();
	this->request_add_reward(welfare_id, ADD_FROM_LEAGUE_WELFARE);

	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_OBTAIN_WELFARE);
}

int LogicLeaguer::open_league_donate()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_OPEN_DONATE,
			ERROR_LEAGUE_NO_EXIST);

	Proto50100626 donate_info;
	donate_info.set_wand( this->leaguer_info_.wand_donate_);
	donate_info.set_gold( this->leaguer_info_.gold_donate_);
	donate_info.set_send_flag(this->leaguer_info_.send_flag_);
	donate_info.set_my_contri(this->leaguer_info_.cur_contri_);

	league->sort_rank();
	for (ThreeObjVec::iterator iter = league->today_contri_rank_.begin();
			iter != league->today_contri_rank_.end(); ++iter)
	{
		LeagueMember *member = league->league_member(iter->id_);
		JUDGE_CONTINUE(member != NULL);
		JUDGE_CONTINUE(member->today_contribute_ > 0);

		ProtoDonateRank *today_rank = donate_info.add_today_rank();
		today_rank->set_rank(member->today_rank_);
		today_rank->set_vip(member->vip_type_);
		today_rank->set_role_name(member->role_name_);
		today_rank->set_contri(member->today_contribute_);
	}

	for (ThreeObjVec::iterator iter = league->total_contri_rank_.begin();
			iter != league->total_contri_rank_.end(); ++iter)
	{
		LeagueMember *member = league->league_member(iter->id_);
		JUDGE_CONTINUE(member != NULL);
		JUDGE_CONTINUE(member->total_contribute_ > 0);

		ProtoDonateRank *total_rank = donate_info.add_total_rank();
		total_rank->set_rank(member->total_rank_);
		total_rank->set_vip(member->vip_type_);
		total_rank->set_role_name(member->role_name_);
		total_rank->set_contri(member->total_contribute_);
	}

//	for (League::MemberMap::iterator iter = league->member_map_.begin();
//			iter != league->member_map_.end(); ++iter)
//	{
//		LeagueMember &member = iter->second;
//		if (member.today_contribute_ > 0)
//		{
//			ProtoDonateRank *today_rank = donate_info.add_today_rank();
//			today_rank->set_rank(member.today_rank_);
//			today_rank->set_vip(member.vip_type_);
//			today_rank->set_role_name(member.role_name_);
//			today_rank->set_contri(member.today_contribute_);
//		}
//		if (member.total_contribute_ >0)
//		{
//			ProtoDonateRank *total_rank = donate_info.add_total_rank();
//			total_rank->set_rank(member.total_rank_);
//			total_rank->set_vip(member.vip_type_);
//			total_rank->set_role_name(member.role_name_);
//			total_rank->set_contri(member.total_contribute_);
//		}
//	}

	FINER_PROCESS_RETURN(RETURN_LEAGUE_OPEN_DONATE, &donate_info);
}

int LogicLeaguer::league_donate(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100613*, request, RETURN_LEAGUE_DONATE);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_DONATE, ERROR_LEAGUE_NO_EXIST);

	int donate_number = request->donate_number();
	CONDITION_NOTIFY_RETURN(donate_number > 0, RETURN_LEAGUE_DONATE, ERROR_CLIENT_OPERATE);

	this->leaguer_info_.send_flag_ = request->send_flag();

	Proto31400301 donate_info;
	donate_info.set_donate_type(request->donate_type());
	donate_info.set_donate_number(donate_number);
	donate_info.set_send_flag(request->send_flag());

	return LOGIC_MONITOR->dispatch_to_scene(this, &donate_info);
}

int LogicLeaguer::map_league_donate(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400301*, request, RETURN_LEAGUE_DONATE);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_DONATE, ERROR_LEAGUE_NO_EXIST);

	// add donate and broadcast
	this->add_league_donate(request);
	this->check_and_brocast_league_donate(request);

	//this->fetch_league_info();
	this->open_league_donate();
	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_DONATE);
}

int LogicLeaguer::fetch_league_member_list(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100614*, request, RETURN_LEAGUE_MEMBER_LIST);

	League* league = this->league();
//	JUDGE_RETURN(league != NULL, 0);
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_MEMBER_LIST,
			ERROR_LEAGUE_NO_EXIST);

	PairObjVec role_set;
	league->fetch_sort_member(role_set);

//	PageInfo page_info;
//	GameCommon::game_page_info(page_info, request->page_index(),
//			role_set.size(), GameEnum::LEAGUE_MEMBER_PAGE_COUNT);

	Proto50100614 list_info;
//	list_info.set_page_index(page_info.page_index_);
//	list_info.set_total_page(page_info.total_page_);

//	for (PairObjSet::iterator iter = role_set.begin() + page_info.start_index_;
	for (PairObjVec::iterator iter = role_set.begin(); iter != role_set.end();	++iter)
	{
		LeagueMember* league_member = league->league_member(iter->id_);
		JUDGE_CONTINUE(league_member != NULL);

		ProtoLeagueMember* member_info = list_info.add_member_set();
		JUDGE_CONTINUE(member_info != NULL);

		member_info->set_left_time(iter->value_);
		member_info->set_role_index(league_member->role_index_);
		member_info->set_role_name(league_member->role_name_);
		member_info->set_league_pos(league_member->league_pos_);
		member_info->set_role_lvl(league_member->role_lvl_);
		member_info->set_sex(league_member->role_sex_);
		member_info->set_vip(league_member->vip_type_);
		member_info->set_role_force(league_member->new_role_force_);
		member_info->set_cur_contri(league_member->cur_contribute_);
		member_info->set_total_contri(league_member->total_contribute_);

//		page_info.add_count_ += 1;
//		JUDGE_BREAK(page_info.add_count_ < GameEnum::LEAGUE_MEMBER_PAGE_COUNT);
	}

//	MSG_USER("Proto50100614, list_info: %s", list_info.Utf8DebugString().c_str());

	FINER_PROCESS_RETURN(RETURN_LEAGUE_MEMBER_LIST, &list_info);
}

int LogicLeaguer::league_appoint(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100616*, request, RETURN_LEAGUE_APPOINT);
	CONDITION_NOTIFY_RETURN(this->role_id() != request->role_index(),
			RETURN_LEAGUE_APPOINT, ERROR_CLIENT_OPERATE);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_APPOINT,
			ERROR_LEAGUE_NO_EXIST);

	LeagueMember* member = league->league_member(request->role_index());
	CONDITION_NOTIFY_RETURN(member != NULL, RETURN_LEAGUE_APPOINT,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->league_member()->league_pos_ > member->league_pos_, RETURN_LEAGUE_APPOINT,
			ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(this->league_member()->league_pos_ >= GameEnum::LEAGUE_POS_ELDER, RETURN_LEAGUE_APPOINT,
			ERROR_CLIENT_OPERATE);

//	int league_pos = LEAGUE_SYSTEM->fetch_league_pos(member->league_pos_);
	int target_pos = request->target_pos();
	CONDITION_NOTIFY_RETURN(this->league_member()->league_pos_ > target_pos &&
			target_pos != member->league_pos_, RETURN_LEAGUE_APPOINT, ERROR_CLIENT_OPERATE);

	int loop = 0;
	bool break_flag = false;
	while (true)
	{
		++loop;
		switch (target_pos)
		{
		case GameEnum::LEAGUE_POS_NONE:
		{
			break_flag = true;
			break;
		}
		case GameEnum::LEAGUE_POS_EXCELLENT:
		{
			int excellent_count = league->league_pos_count(GameEnum::LEAGUE_POS_EXCELLENT);
			int max_count = LEAGUE_SYSTEM->max_league_pos_count(league->league_lvl_,
					GameEnum::LEAGUE_POS_EXCELLENT);
			if (member->league_pos_ > target_pos)
			{
				if (excellent_count < max_count)
					break_flag = true;
				else
					target_pos = GameEnum::LEAGUE_POS_NONE;
			}
			else
			{
				CONDITION_NOTIFY_RETURN(excellent_count < max_count, RETURN_LEAGUE_APPOINT,
						ERROR_LPOS_MAX_COUNT);
				break_flag = true;
			}
			break;
		}
		case GameEnum::LEAGUE_POS_ELDER:
		{
			int elder_count = league->league_pos_count(GameEnum::LEAGUE_POS_ELDER);
			int max_count = LEAGUE_SYSTEM->max_league_pos_count(league->league_lvl_,
					GameEnum::LEAGUE_POS_ELDER);
			if (member->league_pos_ > target_pos)
			{
				if (elder_count < max_count)
					break_flag = true;
				else
					target_pos = GameEnum::LEAGUE_POS_EXCELLENT;
			}
			else
			{
				CONDITION_NOTIFY_RETURN(elder_count < max_count, RETURN_LEAGUE_APPOINT,
						ERROR_LPOS_MAX_COUNT);
				break_flag = true;
			}
			break;
		}
		case GameEnum::LEAGUE_POS_DEPUTY:
		{
//			CONDITION_NOTIFY_RETURN(league->is_leader(this->role_id()) == true,
//					RETURN_LEAGUE_APPOINT, ERROR_CLIENT_OPERATE);
			int deputy_count = league->league_pos_count(GameEnum::LEAGUE_POS_DEPUTY);
			int max_count = LEAGUE_SYSTEM->max_league_pos_count(league->league_lvl_,
					GameEnum::LEAGUE_POS_DEPUTY);

			CONDITION_NOTIFY_RETURN(deputy_count < max_count, RETURN_LEAGUE_APPOINT,
					ERROR_LPOS_MAX_COUNT);

			league->add_league_member_log(GameEnum::LEAGUE_LOG_APPOINT_DEPUTY,
					member->role_name_);

			break_flag = true;
			break;
		}

		default:
		{
			return this->respond_to_client_error(RETURN_LEAGUE_APPOINT,
					ERROR_CLIENT_OPERATE);
		}
		}

		if (break_flag == true)
			break;

		if (loop >= 50)
			break;
	}
	member->league_pos_ = target_pos;

	int mail_id = 0;
	std::string pos_name = CONFIG_INSTANCE->league_pos_name(target_pos);
	if (target_pos > member->league_pos_)
	{
		mail_id = CONFIG_INSTANCE->league("appoint_mail_id").asInt();
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);

		std::string my_pos_name = CONFIG_INSTANCE->league_pos_name(this->league_member()->league_pos_);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), this->name(), my_pos_name.c_str(), pos_name.c_str());

		GameCommon::request_save_mail_content(request->role_index(), mail_info);
	}
	else
	{
		mail_id = CONFIG_INSTANCE->league("demotion_mail_id").asInt();
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);

		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), pos_name.c_str());

		GameCommon::request_save_mail_content(request->role_index(), mail_info);
	}

	// 更新在线成员帮派成员信息
	Proto10100614 proto_info;
	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		LogicPlayer *member_player = NULL;
		if(LOGIC_MONITOR->find_player(iter->first, member_player) == 0)
		{
			member_player->fetch_league_member_list(&proto_info);
		}
	}

	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_APPOINT);
}

int LogicLeaguer::league_kick(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100617*, request, RETURN_LEAGUE_KICK);

	CONDITION_NOTIFY_RETURN(this->role_id() != request->role_index(),
			RETURN_LEAGUE_KICK, ERROR_CLIENT_OPERATE);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_KICK,
			ERROR_LEAGUE_NO_EXIST);

	LeagueMember* self = league->league_member(this->role_id());
	LeagueMember* kicked_mem = league->league_member(request->role_index());
	CONDITION_NOTIFY_RETURN(self != NULL && kicked_mem != NULL && self->league_pos_ >= GameEnum::LEAGUE_POS_DEPUTY,
			RETURN_LEAGUE_KICK, ERROR_DEPUTY_CAN_KICK);
	if(self->league_pos_ == GameEnum::LEAGUE_POS_DEPUTY)
	{
		CONDITION_NOTIFY_RETURN(kicked_mem->league_pos_ == GameEnum::LEAGUE_POS_NONE,
				RETURN_LEAGUE_KICK, ERROR_CLIENT_OPERATE);
	}

	league->handle_quit_league(request->role_index(),
			GameEnum::LEAGUE_LOG_MEMBER_QUITED);

	this->notify_league_state(request->role_index(), GameEnum::LEAGUE_STATE_KICK);

	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_KICK);
}

int LogicLeaguer::league_leader_transfer(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100618*, request, RETURN_LEAGUE_LEADER_TRANSFER);
	CONDITION_NOTIFY_RETURN(this->role_id() != request->role_index(),
			RETURN_LEAGUE_LEADER_TRANSFER, ERROR_CLIENT_OPERATE);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_LEADER_TRANSFER,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(league->is_leader(this->role_id()) == true,
			RETURN_LEAGUE_LEADER_TRANSFER, ERROR_CLIENT_OPERATE);

	league->transfer_leader(request->role_index());
	this->notify_change_leader(request->role_index());

	int mail_id = CONFIG_INSTANCE->league("leader_mail_id").asInt();
	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);

	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str(), this->name());
	GameCommon::request_save_mail_content(request->role_index(), mail_info);

	// 更新在线成员帮派成员信息
	Proto10100614 proto_info;
	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		LogicPlayer *member_player = NULL;
		if(LOGIC_MONITOR->find_player(iter->first, member_player) == 0)
		{
			member_player->fetch_league_info();
			member_player->fetch_league_member_list(&proto_info);
		}
	}

	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_LEADER_TRANSFER);
}

int LogicLeaguer::impeach_league_leader()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_IMPEACH_LEAGUE_LEADER,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(league->is_leader(this->role_id()) == false,
			RETURN_IMPEACH_LEAGUE_LEADER, ERROR_IMPEACH_MYSELF);
	CONDITION_NOTIFY_RETURN(league->can_view_apply(this->role_id()) == true,
			RETURN_IMPEACH_LEAGUE_LEADER, ERROR_ELDER_CAN_IMPEACH_LEADER);
	CONDITION_NOTIFY_RETURN(league->is_in_impeach() == false,
			RETURN_IMPEACH_LEAGUE_LEADER, ERROR_IS_IN_IMPEACH);

	LeagueMember* leader_info = league->league_member(league->leader_index_);
	CONDITION_NOTIFY_RETURN(leader_info != NULL, RETURN_IMPEACH_LEAGUE_LEADER,
			ERROR_LEAGUE_NO_EXIST);

	LogicPlayer* leader = this->find_player(league->leader_index_);
	CONDITION_NOTIFY_RETURN(leader == NULL, RETURN_IMPEACH_LEAGUE_LEADER,
			ERROR_CANNOT_IMPEACH_LEADER);

	Int64 cur_tick = ::time(NULL);
	int pass_tick = cur_tick - leader_info->offline_tick_;
	int need_tick = CONFIG_INSTANCE->league("transfer_tick").asInt();
	CONDITION_NOTIFY_RETURN(pass_tick >= need_tick, RETURN_IMPEACH_LEAGUE_LEADER,
			ERROR_CANNOT_IMPEACH_LEADER);

	league->impeach_leader(this->role_id());

	FINER_PROCESS_NOTIFY(RETURN_IMPEACH_LEAGUE_LEADER);
}

int LogicLeaguer::impeach_vote(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100635*, request, RETURN_MEMBER_IMPEACH_VOTE);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_MEMBER_IMPEACH_VOTE,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(league->is_leader(this->role_id()) == false,
			RETURN_MEMBER_IMPEACH_VOTE, ERROR_IMPEACH_MYSELF);
	CONDITION_NOTIFY_RETURN(league->is_in_impeach() == true,
			RETURN_MEMBER_IMPEACH_VOTE, ERROR_NOT_IN_IMPEACH);
	CONDITION_NOTIFY_RETURN(league->is_vote(this->role_id()) == false,
				RETURN_MEMBER_IMPEACH_VOTE, ERROR_HAS_VOTE);

	int vote_type = request->vote_type();
	league->impeach_voter(this->role_id(), vote_type);

	FINER_PROCESS_NOTIFY(RETURN_MEMBER_IMPEACH_VOTE);
}

int LogicLeaguer::impeach_timeout()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

//	this->fetch_league_info();
	this->notify_change_leader(this->role_id());

	return 0;
}

int LogicLeaguer::check_impeach_time()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);
	JUDGE_RETURN(this->role_id() != league->leader_index_, 0);
	JUDGE_RETURN(league->is_in_impeach() == true, 0);
	JUDGE_RETURN(league->is_vote(this->role_id()) == false, 0);
	JUDGE_RETURN(league->check_impeach_role_leave() == true, 0);

	Int64 impeach_role = league->league_impeach_.impeach_role_;
	LeagueMember* member_info = league->league_member(impeach_role);
	JUDGE_RETURN(member_info != NULL, 0);

	Proto80400387 answer;
	answer.set_leader_name(league->leader_name());
	answer.set_impeach_name(member_info->role_name_);
	answer.set_need_num(league->impeach_need_num());
	answer.set_now_num(league->impeach_now_num());

	return this->respond_to_client(ACTIVE_IMPEACH_LEADER_INFO, &answer);
}

int LogicLeaguer::fetch_league_apply_list(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100615*, request, RETURN_LEAGUE_APPLY_LIST);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_APPLY_LIST,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(league->can_view_apply(this->role_id()) == true,
			RETURN_LEAGUE_APPLY_LIST, ERROR_NO_LEAGUE_RIGHT);

	ThreeObjVec applier_set;
	league->fetch_sort_applier(applier_set);

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page_index(),
			applier_set.size(), GameEnum::LEAGUE_MAX_APPLY_LIST);

	Proto50100615 list_info;
	list_info.set_page_index(page_info.cur_page_);
	list_info.set_total_page(page_info.total_page_);

	for (int i = page_info.start_index_; i < page_info.total_count_; ++i)
	{
		Int64 league_index = applier_set[i].id_;

		League::ApplierMap::iterator iter = league->applier_map_.find(league_index);
		JUDGE_CONTINUE(iter != league->applier_map_.end());

		ProtoLeagueApply* apply_item = list_info.add_apply_set();
		JUDGE_CONTINUE(apply_item != NULL);

		apply_item->set_role_index(iter->second.role_index_);
		apply_item->set_vip_type(iter->second.vip_type_);
		apply_item->set_role_sex(iter->second.role_sex_);
		apply_item->set_role_name(iter->second.role_name_);

		apply_item->set_role_lvl(iter->second.role_lvl_);
		apply_item->set_role_force(iter->second.role_force_);
		apply_item->set_role_career(iter->second.role_career_);

		bool online_flag = this->online_flag(iter->second.role_index_);
		apply_item->set_online_flag(online_flag);

		page_info.add_count_ += 1;
		JUDGE_BREAK(page_info.add_count_ < GameEnum::LEAGUE_MAX_APPLY_LIST);
	}

	list_info.set_auto_flag(league->auto_accept());
	list_info.set_accept_force(league->accept_force_);
	FINER_PROCESS_RETURN(RETURN_LEAGUE_APPLY_LIST, &list_info);
}

int LogicLeaguer::accept_league_apply(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100619*, request, RETURN_LEAGUE_ACCEPT_APPLY);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_ACCEPT_APPLY,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(league->can_view_apply(this->role_id()) == true,
			RETURN_LEAGUE_ACCEPT_APPLY, ERROR_NO_LEAGUE_RIGHT);

	int role_set_size = request->role_set_size();
	bool single_flag = role_set_size > 1 ? false : true;

	for (int i = 0; i < role_set_size; ++i)
	{
		CONDITION_NOTIFY_RETURN(league->league_full() == false,
				RETURN_LEAGUE_ACCEPT_APPLY,	ERROR_LEAGUE_MEMBER_FULL);

		Int64 role_index = request->role_set(i);
		JUDGE_CONTINUE(this->validate_league_join(role_index, single_flag) == true);

		league->handle_join_league(role_index, GameEnum::LEAGUE_POS_NONE);
		this->notify_join_league(role_index);

#ifdef LOCAL_DEBUG
		MMOLeague::save_league(league);
#endif
		LogicPlayer* player = NULL;
		if (LOGIC_MONITOR->find_player(role_index, player) != 0) continue;

		Proto31400324 info;
		info.set_player_id(role_index);
		info.set_league_id(league->league_index_);
		LOGIC_MONITOR->dispatch_to_scene(player, &info);
	}
	//申请列表红点
	if (league->applier_map_.size() <= 0)
	{
		for (League::MemberMap::iterator iter = league->member_map_.begin();
				iter != league->member_map_.end(); ++iter)
		{
			LogicPlayer* player = this->find_player(iter->first);
			JUDGE_CONTINUE(player != NULL);
			JUDGE_CONTINUE(league->can_view_apply(iter->first) == true);

			player->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_ACCEPT, 0);
		}
	}
	this->fetch_league_info();
	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_ACCEPT_APPLY);
}

int LogicLeaguer::reject_league_apply(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100620*, request, RETURN_LEAGUE_REJECT_APPLY);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_REJECT_APPLY,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(league->can_view_apply(this->role_id()) == true,
			RETURN_LEAGUE_REJECT_APPLY, ERROR_NO_LEAGUE_RIGHT);

	int mail_id = CONFIG_INSTANCE->league("reject_mail_id").asInt();
	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);

	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str(), league->league_name_.c_str());

	for (int i = 0; i < request->role_set_size(); ++i)
	{
		Int64 role_id = request->role_set(i);
		league->erase_applier(role_id);
		GameCommon::request_save_mail_content(role_id, mail_info);
	}
	//申请列表红点
	if (league->applier_map_.size() <= 0)
	{
		for (League::MemberMap::iterator iter = league->member_map_.begin();
				iter != league->member_map_.end(); ++iter)
		{
			LogicPlayer* player = this->find_player(iter->first);
			JUDGE_CONTINUE(player != NULL);
			JUDGE_CONTINUE(league->can_view_apply(iter->first) == true);

			player->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_ACCEPT, 0);
		}
	}
	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_REJECT_APPLY);
}

int LogicLeaguer::set_league_auto_accept(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100632*, request, RETURN_LEAGUE_AUTO_ACCEPT);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_AUTO_ACCEPT,
			ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(league->is_leader(this->role_id()) == true,
			RETURN_LEAGUE_AUTO_ACCEPT, ERROR_NO_LEAGUE_RIGHT);

//	int accept_force = request->accept_force();
//	CONDITION_NOTIFY_RETURN(accept_force >= League::MIN_FORCE,
//			RETURN_LEAGUE_AUTO_ACCEPT, ERROR_LEAGUE_MIN_FORCE);

	league->auto_accept_ = request->auto_accept();
//	league->accept_force_ = accept_force;

	FINER_PROCESS_NOTIFY(RETURN_LEAGUE_AUTO_ACCEPT);
}

int LogicLeaguer::fetch_league_log(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100621*, request, RETURN_LEAGUE_LOG);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_LOG,
			ERROR_LEAGUE_NO_EXIST);

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page_index(),
			league->league_log_set_.size(), GameEnum::LEAGUE_LOG_PAGE_COUNT);

	Proto50100621 log_info;
	log_info.set_cur_page(page_info.cur_page_);
	log_info.set_total_page(page_info.total_page_);

	League::LeagueLogSet::iterator iter = league->league_log_set_.begin();
	for (int i = 0; i < page_info.start_index_ ; ++i)
	{
		++iter;
	}

	Int64 now_tick = ::time(NULL);
	for (; iter != league->league_log_set_.end(); ++iter)
	{
		ProtoLeagueLog* member_log = log_info.add_log_set();
		JUDGE_CONTINUE(member_log != NULL);

		member_log->set_log_tick(now_tick - iter->log_tick_);
		member_log->set_log_content(iter->log_conent_);

		page_info.add_count_ += 1;
		JUDGE_BREAK(page_info.add_count_ < GameEnum::LEAGUE_LOG_PAGE_COUNT);
	}

	FINER_PROCESS_RETURN(RETURN_LEAGUE_LOG, &log_info);
}

int LogicLeaguer::add_league_member_contri(int add_contri)
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, -1);

	LeagueMember* member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, -1);

	this->upgrade_cur_contri(member, 0, add_contri);
	this->update_contri_to_client();
	return 0;
}

int LogicLeaguer::sync_league_member_contri(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101802*, request, -1);
	return this->add_league_member_contri(request->add_num());
}

int LogicLeaguer::fetch_league_skill_info()
{
	League* league = this->league();
//	JUDGE_RETURN(league != NULL, 0);
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_SKILL, ERROR_LEAGUE_NO_EXIST);

	Proto50100627 skill_info;
	skill_info.set_cur_contri(this->leaguer_info_.cur_contri_);

	for (IntMap::iterator iter = this->leaguer_info_.skill_map_.begin();
			iter != this->leaguer_info_.skill_map_.end(); ++iter)
	{
		ProtoPairObj* pair_obj = skill_info.add_skill_set();
		JUDGE_CONTINUE(pair_obj != NULL);

		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}

	FINER_PROCESS_RETURN(RETURN_LEAGUE_SKILL, &skill_info);
}

int LogicLeaguer::upgrade_league_skill(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100628*, request, RETURN_LEAGUE_SKILL_UPGRADE);

	int skill_id = request->skill_id();
	CONDITION_NOTIFY_RETURN(this->leaguer_info_.skill_map_.count(skill_id) > 0,
			RETURN_LEAGUE_SKILL_UPGRADE, ERROR_CLIENT_OPERATE);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_SKILL_UPGRADE,
			ERROR_LEAGUE_NO_EXIST);

	int cur_lvl = this->leaguer_info_.skill_map_[skill_id];
	const Json::Value &league_lv_info = CONFIG_INSTANCE->league("set_up")[league->league_lvl_-1];
	int skill_lv_max = league_lv_info["skill_lv_max"].asInt();
	CONDITION_NOTIFY_RETURN(cur_lvl < skill_lv_max, RETURN_LEAGUE_SKILL_UPGRADE, ERROR_LEAGUE_LEVEL);

	const Json::Value &league_skill = CONFIG_INSTANCE->league_skill();
	CONDITION_NOTIFY_RETURN((uint)cur_lvl < (league_skill.size() - 1),
			RETURN_LEAGUE_SKILL_UPGRADE, ERROR_MAX_LEAGUE_SKILL);

	int skill_index = cur_lvl + 1;
	const Json::Value &skill_info = CONFIG_INSTANCE->league_skill_info(skill_index);
	CONDITION_NOTIFY_RETURN(skill_info != Json::Value::null, RETURN_LEAGUE_SKILL_UPGRADE,
			ERROR_MAX_LEAGUE_SKILL);

	LeagueMember* member = league->league_member(this->role_id());
	CONDITION_NOTIFY_RETURN(member != NULL, CLIENT_LEAGUE_SKILL_UPGRADE,
			ERROR_CLIENT_OPERATE);

	int need_contri = skill_info["contribute"].asInt();
	CONDITION_NOTIFY_RETURN(need_contri <= this->leaguer_info_.cur_contri_,
			RETURN_LEAGUE_SKILL_UPGRADE, ERROR_SHORT_CONTRIBUTION);

	this->leaguer_info_.skill_map_[skill_id] += 1;
	this->add_league_member_contri(-1 * need_contri);

	this->send_map_skill_info(true);

	Proto50100628 upgrade_info;
	upgrade_info.set_skill_id(skill_id);
	upgrade_info.set_skill_level(cur_lvl + 1);
	upgrade_info.set_cur_contri(this->leaguer_info_.cur_contri_);
	FINER_PROCESS_RETURN(RETURN_LEAGUE_SKILL_UPGRADE, &upgrade_info);
}

int LogicLeaguer::check_skill_red_point()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	int cur_contri = this->leaguer_info_.cur_contri_;
	const Json::Value &league_skill = CONFIG_INSTANCE->league_skill();
	const Json::Value &league_lv_info = CONFIG_INSTANCE->league("set_up")[league->league_lvl_-1];
	int skill_lv_max = league_lv_info["skill_lv_max"].asInt();
	for (IntMap::iterator iter = this->leaguer_info_.skill_map_.begin();
			iter != this->leaguer_info_.skill_map_.end(); ++iter)
	{
		int skill_id = iter->first;
		int skill_lv = iter->second;
		if ((uint)skill_lv < (league_skill.size() - 1) && skill_lv < skill_lv_max)
		{
			const Json::Value &skill_info = CONFIG_INSTANCE->league_skill_info(skill_lv + 1);

			int need_contri = skill_info["contribute"].asInt();
			if (cur_contri < need_contri)
			{
				this->send_skill_red_point(skill_id, 0);
			}
			else
			{
				this->send_skill_red_point(skill_id, 1);
			}
		}
		else
		{
			this->send_skill_red_point(skill_id, 0);
		}
	}
	return 0;
}

int LogicLeaguer::send_skill_red_point(int skill_id, int value)
{
	int event_id = 0;
	switch(skill_id)
	{
	case GameEnum::ATTACK:
	{
		event_id = GameEnum::PA_EVENT_ATTACK_LEAGUE_SKILL;
		break;
	}
	case GameEnum::DEFENSE:
	{
		event_id = GameEnum::PA_EVENT_DEFENCE_LEAGUE_SKILL;
		break;
	}
//	case GameEnum::CRIT:
//	{
//		event_id = GameEnum::PA_EVENT_CRIT_LEAGUE_SKILL;
//		break;
//	}
//	case GameEnum::TOUGHNESS:
//	{
//		event_id = GameEnum::PA_EVENT_TOUGHNESS_LEAGUE_SKILL;
//		break;
//	}
	case GameEnum::HIT:
	{
		event_id = GameEnum::PA_EVENT_HIT_LEAGUE_SKILL;
		break;
	}
	case GameEnum::AVOID:
	{
		event_id = GameEnum::PA_EVENT_AVOID_LEAGUE_SKILL;
		break;
	}
	case GameEnum::BLOOD_MAX:
	{
		event_id = GameEnum::PA_EVENT_BLOOD_MAX_LEAGUE_SKILL;
		break;
	}
	default:
		return 0;
	}
	this->inner_notify_assist_event(event_id, value);
	return 0;
}

int LogicLeaguer::fetch_league_flag_info()
{
	League* league = this->league();
//	JUDGE_RETURN(league != NULL, 0);
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_FETCH_LEAGUE_FLAG_INFO, ERROR_LEAGUE_NO_EXIST);

	Proto50100654 respond;
	respond.set_flag_lvl(league->flag_lvl_);
	respond.set_flag_exp(league->flag_exp_);

	FINER_PROCESS_RETURN(RETURN_FETCH_LEAGUE_FLAG_INFO, &respond);
}

int LogicLeaguer::upgrade_league_flag()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_UPGRADE_LEAGUE_FLAG, ERROR_LEAGUE_NO_EXIST);
	CONDITION_NOTIFY_RETURN(league->can_operate_league(this->role_id()),
			RETURN_UPGRADE_LEAGUE_FLAG, ERROR_LEAGUE_NOT_LEADER);

	int cur_flag_lvl = league->flag_lvl_;
	int cur_exp = league->flag_exp_;

	const Json::Value &next_flag_info = CONFIG_INSTANCE->league_flag(cur_flag_lvl+1);
	CONDITION_NOTIFY_RETURN(next_flag_info != Json::Value::null, RETURN_UPGRADE_LEAGUE_FLAG,
			ERROR_FLAG_IS_HIGHEST);

	const Json::Value &flag_info = CONFIG_INSTANCE->league_flag(cur_flag_lvl);
	int need_exp = flag_info["upgrade_exp"].asInt();

	CONDITION_NOTIFY_RETURN(cur_exp >= need_exp, RETURN_UPGRADE_LEAGUE_FLAG, ERROR_FLAG_EXP_NOT_ENOUGH);

	league->flag_lvl_ += 1;
	league->flag_exp_ -= need_exp;

	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->send_map_flag_info();

		//帮旗升级红点
		JUDGE_CONTINUE(league->can_operate_league(iter->first));
		const Json::Value &next_flag = CONFIG_INSTANCE->league_flag(league->flag_lvl_+1);
		if (next_flag != Json::Value::null)
		{
			int need = next_flag["upgrade_exp"].asInt();
			if (league->flag_exp_ < need)
				player->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_FLAG, 0);
		}
		else
		{
			player->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_FLAG, 0);
		}
	}

	this->sync_flag_lvl_to_map();

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_int(para_vec, league->flag_lvl_);
	int shout_id = CONFIG_INSTANCE->league("up_flag_shout").asInt();
	GameCommon::announce(league->league_index_, shout_id, &para_vec);

	Proto50100655 respond;
	respond.set_flag_lvl(league->flag_lvl_);
	respond.set_flag_exp(league->flag_exp_);

	FINER_PROCESS_RETURN(RETURN_UPGRADE_LEAGUE_FLAG, &respond);
}

int LogicLeaguer::sync_flag_lvl_to_map()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_UPGRADE_LEAGUE_FLAG, ERROR_LEAGUE_NO_EXIST);

	Proto30400454 inner;
	inner.set_flag_lvl(league->flag_lvl_);
	inner.set_league_index(league->league_index_);

	return LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::LBOSS_SCENE_ID, &inner);
}

bool LogicLeaguer::can_apply_join_league()
{
	JUDGE_RETURN(this->leaguer_info_.leave_type_ > 0, true);
	int span_tick = 0;
	switch(this->leaguer_info_.leave_type_)
	{
	case GameEnum::LEAGUE_LOG_MEMBER_QUIT:
	{
		span_tick = CONFIG_INSTANCE->league("quick_time_span").asInt();
		break;
	}

	case GameEnum::LEAGUE_LOG_MEMBER_QUITED:
	{
		span_tick = CONFIG_INSTANCE->league("kick_time_span").asInt();
		break;
	}

	default:
	{
		return true;
	}
	}

	double left_time = ::time(NULL) - this->leaguer_info_.leave_tick_ - span_tick;

	if(left_time < 0)
	{
		left_time = ::ceil(::abs(left_time / 60));
		char nofity_cstr[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH];
		::snprintf(nofity_cstr,GameEnum::DEFAULT_MAX_CONTENT_LEGNTH,CONFIG_INSTANCE->league("error_tips").asCString(),int(left_time));
		nofity_cstr[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';

		TipsPlayer tips(this);
		tips.push_tips_str(nofity_cstr);
		return false;
	}
	return true;
}

bool LogicLeaguer::validate_league_join(Int64 role_index, bool single_flag)
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, false);

	int ret = league->validate_join(role_index);
	JUDGE_RETURN(ret != 0, true);

	JUDGE_RETURN(single_flag == true, false);
	this->respond_to_client_error(RETURN_LEAGUE_ACCEPT_APPLY, ret);

	return false;
}

void LogicLeaguer::handle_join_league(Int64 league_index)
{
	League *league = this->find_league(league_index);
	JUDGE_RETURN(league != NULL, ;);

	this->leaguer_info_.apply_list_.clear();
	this->role_detail().__league_id = league_index;
	this->logic_player()->chat_join_league(league_index, league->league_name_);
}

void LogicLeaguer::handle_quit_league(int leave_type)
{
	this->logic_player()->chat_leave_league(this->league_index());

	this->leaguer_info_.leave_type_ = leave_type;
	this->leaguer_info_.leave_tick_ = ::time(NULL);

	this->role_detail().__league_id = 0;
	this->inner_notify_assist_event(GameEnum::PA_EVNET_LEAGUE_SALARY, 0);
}

void LogicLeaguer::notify_join_league(Int64 role_index)
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, ;);

	LeagueMember* member = league->league_member(role_index);
	JUDGE_RETURN(member != NULL, ;);

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, member->role_name_);
	int shout_id = CONFIG_INSTANCE->league("add_player_shout").asInt();
	GameCommon::announce(league->league_index_, shout_id, &para_vec);

	this->notify_league_state(role_index, GameEnum::LEAGUE_STATE_JOIN);
}

void LogicLeaguer::notify_quit_league(Int64 league_index, string role_name)
{
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, role_name);
	int shout_id = CONFIG_INSTANCE->league("leave_player_shout").asInt();
	GameCommon::announce(league_index, shout_id, &para_vec);
}

void LogicLeaguer::notify_change_leader(Int64 role_index)
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, ;);

	LeagueMember* member = league->league_member(role_index);
	JUDGE_RETURN(member != NULL, ;);

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, member->role_name_);
	int shout_id = CONFIG_INSTANCE->league("change_leader_shout").asInt();
	GameCommon::announce(league->league_index_, shout_id, &para_vec);
}

int LogicLeaguer::fetch_league_shop_info(int shop_type)
{
//	const Json::Value& league_shop = LEAGUE_SYSTEM->league_shop(shop_type);
//	JUDGE_RETURN(league_shop != Json::Value::null, -1);
//
//	Proto50100624 shop_info;
//	shop_info.set_shop_type(shop_type);
//	shop_info.set_cur_contri(this->leaguer_info_.cur_contri_);
//
//	for (uint i = 0; i < league_shop.size(); ++i)
//	{
//		ProtoLeagueShop* shop_item = shop_info.add_shop_set();
//		JUDGE_CONTINUE(shop_item != NULL);
//
//		int item_id = league_shop[i]["item_id"].asInt();
//		shop_item->set_item_id(item_id);
//
//		int max_num = league_shop[i]["max_number"].asInt();
//
//		IntMap::iterator iter = this->leaguer_info_.buy_map_.find(item_id);
//		if (iter != this->leaguer_info_.buy_map_.end())
//		{
//			shop_item->set_buy_num(max_num - iter->second);
//		}
//		else
//		{
//			shop_item->set_buy_num(max_num);
//		}
//	}
//
//	FINER_PROCESS_RETURN(RETURN_LEAGUE_SHOP_INFO, &shop_info);
	return 0;
}

int LogicLeaguer::send_map_league_info()
{
	Proto30400423 league_info;
	League* self_leauge = this->league();

	if (self_leauge != NULL)
	{
		league_info.set_league_index(self_leauge->league_index_);
		league_info.set_league_name(self_leauge->league_name_);
	}
	else
	{
		league_info.set_league_index(0);
		league_info.set_league_name(GameCommon::NullString);
	}

	return LOGIC_MONITOR->dispatch_to_scene(this, &league_info);
}

int LogicLeaguer::send_map_skill_info(const int enter_type)
{
	int send_flag = false;
	int is_add = false;
	League* league = this->league();
	if (league != NULL)
		is_add = true;

	Proto30400803 inner;

	Proto30400011 prop_info;
	prop_info.set_offset(BasicElement::LEAGUE_SKILL);
	prop_info.set_enter_type(enter_type);

	for (uint i = 0; i < this->leaguer_info_.skill_prop_set_.size(); ++i)
	{
		int prop_index = this->leaguer_info_.skill_prop_set_[i];
		int skill_lvl = this->leaguer_info_.skill_map_[prop_index];

		ProtoPairObj *obj = inner.add_skill_map();
		obj->set_obj_id(prop_index);
		obj->set_obj_value(skill_lvl);

		JUDGE_CONTINUE(skill_lvl > 0);

		ProtoPairObj* pair_obj = prop_info.add_prop_set();
		JUDGE_CONTINUE(pair_obj != NULL);

		pair_obj->set_obj_id(prop_index);
		if (is_add == true)
		{
			int add_value = this->fetch_league_skill_add_attr(prop_index,
					skill_lvl);
			pair_obj->set_obj_value(add_value);
		}
		else
		{
			pair_obj->set_obj_value(0);
		}

		send_flag = true;
	}

	LOGIC_MONITOR->dispatch_to_scene(this, &inner);

	JUDGE_RETURN(send_flag == true, -1);
	return LOGIC_MONITOR->dispatch_to_scene(this, &prop_info);
}

int LogicLeaguer::fetch_league_skill_add_attr(int prop_index, int skill_lvl)
{
	int skill_index = skill_lvl + 1;
	const Json::Value &skill_info = CONFIG_INSTANCE->league_skill_info(skill_index);
	switch(prop_index)
	{
	case GameEnum::ATTACK:
	{
		 return skill_info["attack"].asInt();
	}
	case GameEnum::DEFENSE:
	{
		return skill_info["defence"].asInt();
	}
	case GameEnum::HIT:
	{
		return skill_info["hit"].asInt();
	}
	case GameEnum::AVOID:
	{
		return skill_info["avoid"].asInt();
	}
	case GameEnum::BLOOD_MAX:
	{
		return skill_info["blood_max"].asInt();
	}
	default:
		break;
	};

	return 0;
}

void LogicLeaguer::add_flag_attr(Proto30400011* request, int add_index, int attr, int is_add)
{
	JUDGE_RETURN(request != NULL, ;);

	ProtoPairObj* pair_obj = request->add_prop_set();
	pair_obj->set_obj_id(add_index);
	if (is_add == true)
		pair_obj->set_obj_value(attr);
	else
		pair_obj->set_obj_value(0);
}

void LogicLeaguer::quit_league_miss_red_point()
{
	//福利红点
	this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_WELFARE, 0);
	//申请列表红点
	this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_ACCEPT, 0);
	//技能红点
	for (IntMap::iterator iter = this->leaguer_info_.skill_map_.begin();
			iter != this->leaguer_info_.skill_map_.end(); ++iter)
	{
			int skill_id = iter->first;
			this->send_skill_red_point(skill_id, 0);
	}
	//帮派旗帜升级红点
	this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_FLAG, 0);
	//神兽喂养红点
	this->inner_notify_assist_event(GameEnum::PA_EVENT_LEAGUE_BOSS_FEED, 0);
}

int LogicLeaguer::fetch_wave_reward(const Json::Value& conf, int amount)
{
	for (uint i = 0; i < conf.size(); ++i)
	{
		int need_amount = conf[i][0u].asInt();
		JUDGE_CONTINUE(need_amount == amount);

		return conf[i][1u].asInt();
	}

	return 0;
}

int LogicLeaguer::add_wave_reward(League* league, int wave, IntMap* reward_map,
		IntMap& get_reward, ProtoWaveReward *wave_record)
{
	JUDGE_RETURN(league != NULL && reward_map != NULL, ERROR_CLIENT_OPERATE);

	int wave_amount = league->fetch_wave_player(wave);

	const Json::Value &wave_reward_json = CONFIG_INSTANCE->lfb_wave_reward(wave);
	JUDGE_RETURN(wave_reward_json != Json::Value::null, ERROR_CLIENT_OPERATE);

	const Json::Value &wave_reward = wave_reward_json["wave_reward"];

	wave_record->set_wave(wave);
	wave_record->set_amount(wave_amount);

	for (IntMap::iterator it = reward_map->begin(); it != reward_map->end(); ++it)
	{
		ProtoPairObj *obj = wave_record->add_obj();
		obj->set_obj_id(it->first);

		if (it->first > wave_amount || it->second == true)
		{
			obj->set_obj_value(it->second);
			continue;
		}

		int reward_id = this->fetch_wave_reward(wave_reward, it->first);
		if (reward_id > 0)
			++(get_reward[reward_id]);

		it->second = true;
		obj->set_obj_value(it->second);
	}

	return 0;
}

void LogicLeaguer::check_create_lfb_player(League* league)
{
	if (league->lfb_player_map_.count(this->role_id()) <= 0)
	{
		LFbPlayer &my_lfb = league->lfb_player_map_[this->role_id()];
		my_lfb.role_id_ = this->role_id();
		my_lfb.name_ = this->name();
		my_lfb.sex_ = this->role_detail().__sex;
	}

	//兼容线上缺性别问题
	LFbPlayer &my_lfb = league->lfb_player_map_[this->role_id()];
	if (my_lfb.sex_ != 1 || my_lfb.sex_ != 2)
	{
		my_lfb.sex_ = this->role_detail().__sex;
	}
}

int LogicLeaguer::send_map_flag_info(const int enter_type)
{
	int is_add = false;
	int flag_lvl = 1;
	League* league = this->league();
	if (league != NULL)
	{
		is_add = true;
		flag_lvl = league->flag_lvl_;
	}

	Proto30400803 inner;
	inner.set_flag_lvl(flag_lvl);
	LOGIC_MONITOR->dispatch_to_scene(this, &inner);

	const Json::Value &flag_info = CONFIG_INSTANCE->league_flag(flag_lvl);

	Proto30400011 prop_info;
	prop_info.set_offset(BasicElement::LEAGUE_FLAG);
	prop_info.set_enter_type(enter_type);
	this->add_flag_attr(&prop_info, GameEnum::ATTACK, flag_info["attack"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::DEFENSE, flag_info["defence"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::CRIT, flag_info["crit"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::TOUGHNESS, flag_info["toughness"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::HIT, flag_info["hit"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::AVOID, flag_info["dodge"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::BLOOD_MAX, flag_info["health"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::DAMAGE_MULTI, flag_info["damage_multi"].asInt(), is_add);
	this->add_flag_attr(&prop_info, GameEnum::REDUCTION_MULTI, flag_info["decrease_hurt"].asInt(), is_add);


	int leader_attack = flag_info["leader_attack"].asInt();
	if (this->is_league_leader() == true && leader_attack > 0)
	{
		leader_attack += flag_info["attack"].asInt();
		this->add_flag_attr(&prop_info, GameEnum::ATTACK, leader_attack, is_add);
	}

	return LOGIC_MONITOR->dispatch_to_scene(this, &prop_info);
}

int LogicLeaguer::update_apply_league_info()
{
	Int64 role_id = this->role_id();
	for (LongMap::iterator iter = this->leaguer_info_.apply_list_.begin();
			iter != this->leaguer_info_.apply_list_.end(); ++iter)
	{
		League* league = this->find_league(iter->first);

		JUDGE_CONTINUE(league != NULL);
		JUDGE_CONTINUE(league->applier_map_.count(role_id) > 0);

		LeagueApplier& applier = league->applier_map_[role_id];
		applier.role_lvl_ = this->role_level();
		applier.role_force_ = this->role_detail().__fight_force;
	}

	return 0;
}

int LogicLeaguer::notify_league_state(Int64 role_id, int league_state)
{
	LogicPlayer* player = this->find_player(role_id);
	JUDGE_RETURN(player != NULL, -1);

	Proto80400344 state_info;
	state_info.set_league_state(league_state);
	return player->respond_to_client(ACTIVE_LEAGUE_STATE_INFO, &state_info);
}

void LogicLeaguer::add_league_donate(int donate_type, int donate_number)
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, ;);

	LeagueMember* member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, ;);

	switch (donate_type)
	{
	case GameEnum::LEAGUE_WAND_DONATE:
	{
		this->leaguer_info_.wand_donate_ += donate_number;
		league->add_league_donate_log(GameEnum::LEAGUE_LOG_DONATE_WAND,
				donate_number * GameEnum::LEAGUE_WAND_TIMES, this->name());
		break;
	}

	case GameEnum::LEAGUE_GOLD_DONATE:
	{
		this->leaguer_info_.gold_donate_ += donate_number;
		league->add_league_donate_log(GameEnum::LEAGUE_LOG_DONATE_GOLD,
				donate_number, this->name());
		break;
	}

	default:
	{
		return;
	}
	}

	// add league resource
	const Json::Value& donate_item = CONFIG_INSTANCE->league("donate")[donate_type];
	int add_contir = donate_number * donate_item["contribute"].asDouble();
	int add_resource = donate_number * donate_item["resource"].asDouble();
	this->add_league_member_contri(add_contir);
	league->add_league_resource(this->role_id(), add_resource, CONTRI_FROM_DONATE);
}

void LogicLeaguer::add_league_donate(Proto31400301* request)
{
	JUDGE_RETURN(request != NULL, ;);
	this->add_league_donate(request->donate_type(), request->donate_number());
}

void LogicLeaguer::check_and_brocast_league_donate(Proto31400301* request)
{
//	JUDGE_RETURN(request != NULL, ;);
//	JUDGE_RETURN(request->send_flag() == true, ;);
//
//	BrocastParaVec para_vec;
//	GameCommon::push_brocast_para_role_detail(para_vec,	this->role_id(), this->name(), false);
//
//	if (request->donate_type() == GameEnum::LEAGUE_WAND_DONATE)
//	{
//		int wand = request->donate_number() * GameEnum::LEAGUE_WAND_TIMES;
//		GameCommon::push_brocast_para_int(para_vec, wand);
//		this->announce_with_player(SHOUT_LEAGUE_DONATE_COPPER, para_vec);
//	}
//	else
//	{
//		GameCommon::push_brocast_para_int(para_vec, request->donate_number());
//		this->announce_with_player(SHOUT_LEAGUE_DONATE, para_vec);
//	}
}

int LogicLeaguer::fetch_store()
{
//	League* league = this->league();
//	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_FETCH_INFO, ERROR_CLIENT_OPERATE);
//	int total_times = CONFIG_INSTANCE->league_store()["apply_times"].asInt();
//	GamePackage& package = league->__lstore.package;
//
//	Proto50100801 respond;
//	respond.set_is_leader(this->league_member()->league_pos_);
//	respond.set_left_times(total_times - this->leaguer_info_.store_times_);
//	respond.set_total_times(total_times);
//	respond.set_pack_size(package.pack_size_);
//	for (ItemListMap::iterator iter = package.item_list_map_.begin();
//				iter != package.item_list_map_.end(); ++iter)
//	{
//		PackageItem* pack_item = iter->second;
//		JUDGE_CONTINUE(pack_item != NULL);
//
//		ProtoItem* proto_item = respond.add_item_list();
//		pack_item->serialize(proto_item);
//	}
//	FINER_PROCESS_RETURN(RETURN_LSTORE_FETCH_INFO, &respond);
	return 0;
}


int LogicLeaguer::fetch_apply_list(Message* msg)
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_FETCH_APPLY, ERROR_CLIENT_OPERATE);
	DYNAMIC_CAST(Proto10100802*, request, msg);

	std::map<Int64,ApplyInfo>& apply_map = league->__lstore.apply_map;
	std::vector<Int64>& apply_vec = league->__lstore.apply_vec;
	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page_index(),
			apply_vec.size(), GameEnum::LSTORE_PAGE_APPLY_COUNT);

	Proto50100802 list_info;
	list_info.set_page_index(page_info.cur_page_);
	list_info.set_total_page(page_info.total_page_);

	for (std::vector<Int64>::reverse_iterator iter = apply_vec.rbegin() + page_info.start_index_;
			iter != apply_vec.rend(); ++iter)
	{
		ProtoLeagueStoreApply* apply_item = list_info.add_apply_list();
		JUDGE_CONTINUE(apply_item != NULL);

		apply_item->set_item_id(apply_map[*iter].item_id);
		apply_item->set_item_num(apply_map[*iter].item_num);
		apply_item->set_apply_id(*iter);

		LeagueMember* mem = this->league()->league_member(apply_map[*iter].role_id);
		JUDGE_CONTINUE(mem != NULL);
		apply_item->set_contri(mem->total_contribute_);
		apply_item->set_role_level(mem->role_lvl_);
		apply_item->set_role_name(mem->role_name_);

		PackageItem* item = league->__lstore.package.find_by_unique_id(apply_map[*iter].item_unique_id);
		JUDGE_CONTINUE(item != NULL);
		item->serialize(apply_item->mutable_item());

		page_info.add_count_ += 1;

		JUDGE_BREAK(page_info.add_count_ < GameEnum::LSTORE_PAGE_APPLY_COUNT);
	}
	FINER_PROCESS_RETURN(RETURN_LSTORE_FETCH_APPLY, &list_info);
}

int LogicLeaguer::sort_store()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_SORT, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(this->league_member()->league_pos_ >= GameEnum::LEAGUE_POS_DEPUTY,
			RETURN_LSTORE_SORT, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(league->__lstore.next_refresh_tick < ::time(0),RETURN_LSTORE_SORT, ERROR_OPERATE_TOO_FAST);
	league->__lstore.next_refresh_tick = ::time(0) + 3;

	GamePackage& package = league->__lstore.package;
	package.sort_and_merge();
	league->__lstore.lock_map.clear();
	for(ItemListMap::iterator iter = package.item_list_map_.begin(); iter != package.item_list_map_.end(); ++iter)
	{
		league->__lstore.lock_map[iter->first] = -1;
	}
	league->notify_store();
	return 0;
}

int LogicLeaguer::insert_store(Message* msg)
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_INSERT, ERROR_CLIENT_OPERATE);
	DYNAMIC_CAST(Proto10100804*, request, msg);
	int item_id = request->item_id();
	if(!GameCommon::item_is_equipment(item_id))
	{
		const Json::Value& itemjson = CONFIG_INSTANCE->item(item_id);
		CONDITION_NOTIFY_RETURN(itemjson != Json::nullValue && itemjson.isMember("can_lstore")
				&& itemjson["can_lstore"].asInt() == 1,RETURN_LSTORE_INSERT, ERROR_CLIENT_OPERATE);
	}

	int index =league->__lstore.search_empty_index_i();
	CONDITION_NOTIFY_RETURN(index >= 0,RETURN_LSTORE_INSERT, index);
	this->league()->__lstore.luck_block(index);

	Proto31400316 req;
	req.set_item_id(item_id);
	req.set_item_num(request->item_num());
	req.set_item_index(request->item_index());
	req.set_package_index(index);

	this->monitor()->dispatch_to_scene(this,&req);
	return 0;
}

int LogicLeaguer::insert_store_after(Message* msg)
{
	League* league = this->league();
	if(league == NULL)
	{
		this->respond_to_client_error(RETURN_LSTORE_INSERT,ERROR_CLIENT_OPERATE);
		return this->lstore_insert_failed(msg);
	}
	DYNAMIC_CAST(Proto30100237*, request, msg);
	int package_index = request->package_index();
	const ProtoItem& proto_item = request->item();

	GamePackage& package = league->__lstore.package;
	if(package.find_by_index(package_index) != NULL)
	{
		this->respond_to_client_error(RETURN_LSTORE_INSERT,ERROR_NO_ENOUGH_SPACE);
		return this->lstore_insert_failed(msg);
	}

	PackageItem* item = GamePackage::pop_item(proto_item.id());
	item->unserialize(proto_item);
	item->__index = package_index;
    int64_t item_unique_id = 0;
    this->monitor()->fetch_global_value(Global::LSTORE_ITEM_ID, item_unique_id);
    item->__unique_id = item_unique_id;

	int ret = package.insert_by_index(item);
	if(ret != 0)
	{
		this->respond_to_client_error(RETURN_LSTORE_INSERT,ret);
		return this->lstore_insert_failed(msg);
	}

	this->league()->__lstore.lock_map[package_index] = -1;
	league->notify_store();

	ret = this->record_apply_his(item->__id,item->__amount,this->role_id(),GameEnum::LSTORE_IN);
	CONDITION_NOTIFY_RETURN(ret == 0,RETURN_LSTORE_INSERT,ret);

	return 0;
}

int LogicLeaguer::lstore_insert_failed(Message* msg)
{
	DYNAMIC_CAST(Proto30100237*, request, msg);
	const ProtoItem& proto_item = request->item();

	Proto31400320 req;
	ProtoItem* failed_item = req.mutable_item();
	(*failed_item) = proto_item;
	req.set_pack_index(-1);

	this->monitor()->dispatch_to_scene(this,&req);
	return 0;
}

int LogicLeaguer::apply_item(Message* msg)
{
//	League* league = this->league();
//	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_GET_ITEM, ERROR_CLIENT_OPERATE);
//
//	int total_times = CONFIG_INSTANCE->league_store()["apply_times"].asInt();
//
//	if(total_times > 0 && this->leaguer_info_.store_times_ >= total_times)
//	{
//		this->set_last_error(ERROR_TIMES_SHORT);
//		return this->respond_to_client_error(RETURN_LSTORE_GET_ITEM, ERROR_TIMES_SHORT);
//	}
//
//	DYNAMIC_CAST(Proto10100805*, request, msg);
//	int item_id = request->item_id(),item_num = request->item_num(),package_index = request->item_index();
//
//	CONDITION_NOTIFY_RETURN(item_num <= 99,RETURN_LSTORE_GET_ITEM,ERROR_SET_ITEM_AMOUNT);
//	GamePackage& package = league->__lstore.package;
//
//	PackageItem* item = package.find_by_index(package_index);
//	if(item == NULL)
//	{
//		//this->sort_store();
//		return this->respond_to_client_error(RETURN_LSTORE_GET_ITEM, ERROR_NO_THIS_ITEM);
//	}
//	int have_num = item->__amount;
//	CONDITION_NOTIFY_RETURN(item->__id == item_id && have_num >= item_num,RETURN_LSTORE_GET_ITEM,ERROR_SET_ITEM_AMOUNT);
//	CONDITION_NOTIFY_RETURN(this->league()->__lstore.lock_map[package_index] == -1,
//			RETURN_LSTORE_GET_ITEM,ERROR_NO_THIS_ITEM);
//
//	if(this->league_member()->league_pos_ >= GameEnum::LEAGUE_POS_DEPUTY)
//	{
//		this->league()->__lstore.lock_map[package_index] = -2;
//		league->__lstore.next_refresh_tick = ::time(0) + 3;
//		Proto31400317 req;
//		ProtoItem* proto_item = req.mutable_item();
//		item->serialize(proto_item,-1,item_num);
//		//插入背包
//		return this->monitor()->dispatch_to_scene(this,&req);
//	}
//
//	std::map< Int64,LongSet >& item_role_map = this->league()->__lstore.item_role_map;
//	if(item_role_map[item->__unique_id].count(this->role_id()))
//	{
//		this->set_last_error(ERROR_APPLY_REPEATED);
//		return this->respond_to_client_error(RETURN_LSTORE_GET_ITEM, ERROR_APPLY_REPEATED);
//
//	}
//	int ret = this->record_apply(item_id,item_num,item->__unique_id);
//	CONDITION_NOTIFY_RETURN(ret == 0,RETURN_LSTORE_GET_ITEM,ret);
//	++this->leaguer_info_.store_times_;
//	Proto50100805 respond;
//	respond.set_left_times(total_times - this->leaguer_info_.store_times_);
//	FINER_PROCESS_RETURN(RETURN_LSTORE_GET_ITEM,&respond);
	return 0;
}

int LogicLeaguer::record_apply(int item_id,int item_num, Int64 item_unique_id)
{
    int64_t apply_id = 0;
    this->monitor()->fetch_global_value(Global::LSTORE_APPLY, apply_id);
	ApplyInfo info;
	info.item_id = item_id;
	info.item_num = item_num;
	info.item_unique_id = item_unique_id;
	info.role_id = this->role_id();
	if (apply_id <= 0)
    {
        return ERROR_SERVER_INNER;
    }
	this->league()->__lstore.apply_map[apply_id] = info;
	this->league()->__lstore.item_role_map[item_unique_id].insert(info.role_id);
	this->league()->__lstore.apply_vec.push_back(apply_id);
	return  0;
}

int LogicLeaguer::check_apply(Message* msg)
{
//	League* league = this->league();
//	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_CHECK_APPLY, ERROR_CLIENT_OPERATE);
//	CONDITION_NOTIFY_RETURN(this->league_member()->league_pos_ >= GameEnum::LEAGUE_POS_DEPUTY,
//			RETURN_LEAGUE_KICK, ERROR_CLIENT_OPERATE);
//	DYNAMIC_CAST(Proto10100806*, request, msg);
//	int64_t apply_id = request->apply_id();
//	int opt = request->opt();
//
//	if(opt == 1)
//	{
//		CONDITION_NOTIFY_RETURN(this->league()->__lstore.apply_map.count(apply_id) > 0,
//				RETURN_LSTORE_CHECK_APPLY,ERROR_GOODS_NO_EXIST);
//		ApplyInfo& info = league->__lstore.apply_map[apply_id];
//		int item_id = info.item_id,item_num = info.item_num;
//
//		Int64 item_unique_id = info.item_unique_id;
//		Int64 role_id = info.role_id;
//		string role_name = info.role_name;
//		GamePackage& package = this->league()->__lstore.package;
//		PackageItem* item = package.find_by_unique_id(item_unique_id);
//		if(item == NULL || item->__amount < item_num)
//		{
//			if(item == NULL)
//				this->remove_apply(item_unique_id,0);
//			else
//				this->remove_apply(item_unique_id,item->__amount);
//			this->set_last_error(ERROR_APPLY_NOT_VALID);
//			Proto10100802 req;
//			req.set_page_index(0);
//			this->fetch_apply_list(&req);
//			return this->respond_to_client_error(RETURN_LSTORE_CHECK_APPLY,ERROR_APPLY_NOT_VALID);
//		}
//
//		int ret = this->record_apply_his(item_id,item_num,role_id,GameEnum::LSTORE_APPLY,apply_id);
//		CONDITION_NOTIFY_RETURN(ret == 0,RETURN_LSTORE_CHECK_APPLY,ret);
//
//		//广播
//		int old_amount = item->__amount;
//		item->__amount = item_num;
//		BrocastParaVec para_vec;
//		GameCommon::push_brocast_para_role_detail(para_vec,
//				this->role_id(), this->name(), this->logic_player()->teamer_state());
//		GameCommon::push_brocast_para_string(para_vec,role_name);
//		GameCommon::push_brocast_para_item(para_vec, item);
//		this->announce_with_player(SHOUT_LEAGUE_LSTORE_CHECK, para_vec);
//
//		item->__amount = old_amount;
//		int item_index = item->__index;
//		this->send_lsrote_mail(item,role_id,item_num);
//		package.remove_item(item,item_num);
//		league->notify_store();
//
//		league->__lstore.apply_map.erase(apply_id);
//		std::vector<Int64>& apply_vec = this->league()->__lstore.apply_vec;
//		for(std::vector<Int64>::iterator iter = apply_vec.begin();iter != apply_vec.end();++iter)
//		{
//			if(*iter != apply_id)
//				continue;
//			apply_vec.erase(iter);
//			break;
//		}
//
//		int left_num = 0;
//		item = package.find_by_unique_id(item_unique_id);
//		if(item != NULL )
//		{
//			left_num = item->__amount;
//		}
//		else
//		{
//			league->__lstore.lock_map[item_index] = 0;
//		}
//		this->remove_apply(item_unique_id,left_num);
//	}
//
//	Proto10100802 req;
//	req.set_page_index(0);
//	return this->fetch_apply_list(&req);
	return 0;
}

int LogicLeaguer::remove_apply(Int64 item_unique_id,int left_amount)
{
	std::vector<Int64>& apply_vec = this->league()->__lstore.apply_vec;
	std::map<Int64,int> remove_map;
	std::map<Int64,ApplyInfo>& apply_map = this->league()->__lstore.apply_map;
	std::map< Int64,LongSet >& item_role_map = this->league()->__lstore.item_role_map;

	for(std::map<Int64,ApplyInfo>::iterator iter = apply_map.begin(); iter != apply_map.end(); )
	{
		if((iter->second.item_unique_id == item_unique_id)&&(iter->second.item_num > left_amount))
		{
			if(left_amount == 0)
			{
				item_role_map.erase(item_unique_id);
			}
			else
			{
				item_role_map[item_unique_id].erase(iter->second.role_id);
				if(item_role_map[item_unique_id].empty())
				{
					item_role_map.erase(item_unique_id);
				}
			}
			remove_map[iter->first] = 1;
			apply_map.erase(iter++);
			continue;
		}
		++iter;
	}
	if(!remove_map.empty())
	{
		for(std::vector<Int64>::iterator iter2 = apply_vec.begin(); iter2 != apply_vec.end();)
		{
			if(remove_map.count(*iter2) <= 0)
			{
				++iter2;
				continue;
			}
			iter2 = apply_vec.erase(iter2);
		}
	}
	return  0;
}

int LogicLeaguer::send_lsrote_mail(PackageItem* item,Int64 role_id,int item_num)
{
	int front = FONT_LSTORE_GET;
	if( this->league()->is_leader(this->role_id()))
	{
		front = FONT_LSTORE_GET_LEADER;
	}
    FontPair mail_font = FONT2(front);
    MailInformation *mail_info = GameCommon::create_sys_mail(mail_font,front);

	PackageItem* pack_item = GamePackage::pop_item(item->__id);
	*pack_item = *item;
	pack_item->__amount = item_num;
	int item_index = mail_info->goods_map_.size();
	mail_info->goods_map_[item_index] = pack_item;

	char mail_content[GameEnum::MAX_MAIL_CONTENT_LENGTH + 1];
    ::snprintf(mail_content, GameEnum::MAX_MAIL_CONTENT_LENGTH, mail_info->mail_content_.c_str(),this->name());
    mail_content[GameEnum::MAX_MAIL_CONTENT_LENGTH] = '\0';

    mail_info->mail_content_ = mail_content;
    GameCommon::request_save_mail(role_id, mail_info);
	return 0;
}

int LogicLeaguer::record_apply_his(int item_id,int item_num,Int64 role_id,int opt,Int64 apply_id)
{
	LeagueMember* mem = this->league()->league_member(role_id);
	if(mem == NULL)
	{
		this->remove_apply_by_id(apply_id);
		Proto10100802 req;
		req.set_page_index(0);
		this->fetch_apply_list(&req);
		return ERROR_PLAYER_NOT_EXIST;
	}

	ApplyHistory history;
	history.tick = ::time(0);
	history.item_id = item_id;
	history.item_num = item_num;
	history.opt = opt;
	history.role_name = mem->role_name_;
	if(opt == GameEnum::LSTORE_APPLY)
	{
		history.checker_name = this->role_detail().name();
	}
	std::list<ApplyHistory>& apply_his = this->league()->__lstore.apply_history;
	while( apply_his.size() >= LSTORE_MAX_APPLY_HISTORY )
	{
		apply_his.pop_front();
	}
	apply_his.push_back(history);
	return 0;
}

int LogicLeaguer::fetch_apply_history(Message* msg)
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_FETCH_APPLY_HIS, ERROR_CLIENT_OPERATE);
	std::list<ApplyHistory>& apply_history = league->__lstore.apply_history;

	Proto50100807 list_info;
	for(std::list<ApplyHistory>::iterator iter = apply_history.begin();
			iter != apply_history.end(); ++iter)
	{
		ProtoLeagueStoreApplyHis* apply_his = list_info.add_apply_his();
		JUDGE_CONTINUE(apply_his != NULL);

		apply_his->set_item_id(iter->item_id);
		apply_his->set_item_num(iter->item_num);
		apply_his->set_exec(iter->opt);
		apply_his->set_checker_name(iter->checker_name);
		apply_his->set_exec_tick(iter->tick);
		apply_his->set_exec_name(iter->role_name);

	}
	FINER_PROCESS_RETURN(RETURN_LSTORE_FETCH_APPLY_HIS,&list_info);
}

int LogicLeaguer::sync_league_info()
{
//	if(this->league() == NULL)
//	{
//		return 0;
//	}
//
//	Proto31400315 send_to_map_logic;
//	send_to_map_logic.set_join_tick(this->league_member()->join_tick_);
//	return LOGIC_MONITOR->dispatch_to_scene(this, &send_to_map_logic);
	return 0;
}

void LogicLeaguer::update_leaguer_fashion()
{
	League *league = this->league();
	JUDGE_RETURN(league != NULL, ;);

	LeagueMember* member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, ;);

	member->fashion_id_ = this->role_detail().fashion_id_;
	member->fashion_color_ = this->role_detail().fashion_color_;
}

int LogicLeaguer::remove_apply_by_id(Int64 apply_id)
{
	std::vector<Int64>& apply_vec = this->league()->__lstore.apply_vec;
	std::map<Int64,ApplyInfo>& apply_map = this->league()->__lstore.apply_map;
	std::map< Int64,LongSet >& item_role_map = this->league()->__lstore.item_role_map;

	for(std::vector<Int64>::iterator iter = apply_vec.begin(); iter != apply_vec.end(); ++iter)
	{
		if(*iter != apply_id)
			continue;
		ApplyInfo& info = apply_map[apply_id];
		item_role_map[info.item_unique_id].erase(info.role_id);
		if(item_role_map[info.item_unique_id].empty())
		{
			item_role_map.erase(info.item_unique_id);
		}
		apply_vec.erase(iter);
		break;
	}
	apply_map.erase(apply_id);
	return 0;
}

int LogicLeaguer::apply_item_result(Message *msg)
{
	DYNAMIC_CAST(Proto31400318 *, request, msg);
	int result = request->result();
	const ProtoItem& proto_item = request->item();
	if( result == 1)
	{
		//失败
		int erroe_code = request->error_code();
		this->set_last_error(erroe_code);
		this->respond_to_client_error(RETURN_LSTORE_GET_ITEM, erroe_code);
		this->league()->__lstore.lock_map[proto_item.index()] = -1;
	}
	else
	{
		int ret = this->record_apply_his(proto_item.id(),proto_item.amount(),this->role_id(),GameEnum::LSTORE_OUT);
		if(ret != 0)
		{
			MSG_USER("record_apply_his ERROR:league_id:%ld,role_id:%ld,item_id:%d,errno:%d",
					this->league_index(),this->role_id(),proto_item.id(),ret);
			this->set_last_error(RETURN_LSTORE_GET_ITEM);
			this->respond_to_client_error(RETURN_LSTORE_GET_ITEM, ret);
			Proto31400320 req;
			req.set_pack_index(request->pack_index());
			ProtoItem* failed_item = req.mutable_item();
			(*failed_item) = proto_item;
			return this->monitor()->dispatch_to_scene(this,&req);
		}

		ret = this->league()->__lstore.package.remove_item(proto_item.index(),proto_item.amount());
		if(ret != 0)
		{
			MSG_USER("lstore.package.remove_item ERROR:league_id:%ld,role_id:%ld,item_id:%d,errno:%d",
					this->league_index(),this->role_id(),proto_item.id(),ret);
			this->set_last_error(RETURN_LSTORE_GET_ITEM);
			this->respond_to_client_error(RETURN_LSTORE_GET_ITEM, ret);
			Proto31400320 req;
			req.set_pack_index(request->pack_index());
			ProtoItem* failed_item = req.mutable_item();
			(*failed_item) = proto_item;
			return this->monitor()->dispatch_to_scene(this,&req);
		}

		PackageItem* item = this->league()->__lstore.package.find_by_index(proto_item.index());
		if(NULL == item)
		{
			this->league()->__lstore.lock_map[proto_item.index()] = 0;
			this->remove_apply(proto_item.unique_id(),0);
		}
		else
		{
			this->league()->__lstore.lock_map[proto_item.index()] = -1;
			this->remove_apply(proto_item.unique_id(),item->__amount);
		}

		this->league()->notify_store();
		return 0;
	}
	return 0;
}


int LogicLeaguer::open_store()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_FETCH_INFO, ERROR_CLIENT_OPERATE);
	league->__lstore.view_list.insert(this->role_id());
	return 0;
}
int LogicLeaguer::close_store()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL,RETURN_LSTORE_FETCH_INFO, ERROR_CLIENT_OPERATE);
	league->__lstore.view_list.erase(this->role_id());
	return 0;
}

int LogicLeaguer::request_lfb_wave_reward()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_FB_REWARD_INFO, ERROR_LEAGUE_NO_EXIST);

	if (this->leaguer_info_.wave_reward_map_.size() <= 0)
		this->reset_lfb_wave_reward();

	Proto50102101 respond;

	LeaguerInfo::WaveRewardMap &wave_reward_map = this->leaguer_info_.wave_reward_map_;
	for (LeaguerInfo::WaveRewardMap::iterator iter = wave_reward_map.begin();
			iter != wave_reward_map.end(); ++iter)
	{
		int wave_amount = league->fetch_wave_player(iter->first);

		ProtoWaveReward *wave_reward = respond.add_wave_reward();
		wave_reward->set_wave(iter->first);
		wave_reward->set_amount(wave_amount);

		IntMap &reward_map = iter->second;
		for (IntMap::iterator it = reward_map.begin(); it != reward_map.end(); ++it)
		{
			ProtoPairObj *obj = wave_reward->add_obj();
			obj->set_obj_id(it->first);
			obj->set_obj_value(it->second);
		}
	}

	if (league->last_wave_player_ > 0)
	{
		LFbPlayer *lfb_player = league->lfb_player(league->last_wave_player_);
		if (lfb_player != NULL)
		{
			ProtoWavePlayer *wave_player = respond.mutable_wave_player();
			wave_player->set_role_id(lfb_player->role_id_);
			wave_player->set_name(lfb_player->name_);
			wave_player->set_sex(lfb_player->sex_);
			wave_player->set_wave(lfb_player->last_wave_);
			wave_player->set_tick(lfb_player->tick_);
		}
	}

	FINER_PROCESS_RETURN(RETURN_LEAGUE_FB_REWARD_INFO, &respond);
}

int LogicLeaguer::fetch_lfb_wave_reward(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102102 *, request, RETURN_FETCH_LEAGUE_FB_REWARD);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_FETCH_LEAGUE_FB_REWARD, ERROR_LEAGUE_NO_EXIST);

	Proto50102102 respond;
	respond.set_wave(request->wave());
	respond.set_is_total(request->is_total());

	IntMap get_reward;
	int is_total = request->is_total();
	if (is_total == true)
	{
		LeaguerInfo::WaveRewardMap &wave_reward_map = this->leaguer_info_.wave_reward_map_;
		for (LeaguerInfo::WaveRewardMap::iterator iter = wave_reward_map.begin();
				iter != wave_reward_map.end(); ++iter)
		{
			ProtoWaveReward *wave_reward = respond.add_wave_info();
			this->add_wave_reward(league, iter->first, &(iter->second), get_reward, wave_reward);
		}
	}
	else
	{
		int wave = request->wave();
		IntMap *reward_map = this->leaguer_info_.reward_map(wave);
		ProtoWaveReward *wave_reward = respond.add_wave_info();
		int ret = this->add_wave_reward(league, wave, reward_map, get_reward, wave_reward);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_FETCH_LEAGUE_FB_REWARD, ret);
	}

	CONDITION_NOTIFY_RETURN(get_reward.size() > 0, RETURN_FETCH_LEAGUE_FB_REWARD,
			ERROR_ACT_NO_REWARD);

	for (IntMap::iterator iter = get_reward.begin(); iter != get_reward.end(); ++iter)
	{
		for (int i = 0; i < iter->second; ++i)
			this->request_add_reward(iter->first, ADD_FROM_LEAGUE_FB_BOX);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_LEAGUE_FB_REWARD, &respond);
}

int LogicLeaguer::request_lfb_cheer_info()
{
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_FETCH_LEAGUE_FB_CHEER_INFO, ERROR_LEAGUE_NO_EXIST);

	this->check_create_lfb_player(league);
	LFbPlayer *my_lfb = league->lfb_player(this->role_id());
	CONDITION_NOTIFY_RETURN(my_lfb != NULL, RETURN_FETCH_LEAGUE_FB_CHEER_INFO, ERROR_SERVER_INNER);

	Proto50102104 respond;
	respond.set_cheer(my_lfb->cheer_);
	respond.set_be_cheer(my_lfb->be_cheer_);
	respond.set_encourage(my_lfb->encourage_);
	respond.set_be_encourage(my_lfb->be_encourage_);

	ProtoPlayerCheer *my_cheer = respond.mutable_my_cheer();
	my_cheer->set_role_id(my_lfb->role_id_);
	my_cheer->set_role_name(my_lfb->name_);
	my_cheer->set_wave(my_lfb->wave_);

	for (ThreeObjVec::iterator iter = league->lfb_rank_vec_.begin();
			iter != league->lfb_rank_vec_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->role_id() != iter->id_);

		LFbPlayer *lfb_player = league->lfb_player(iter->id_);
		int wave = lfb_player->wave_;
		JUDGE_CONTINUE(wave > 0);

		ProtoPlayerCheer *player_info = respond.add_player_info();
		player_info->set_role_id(lfb_player->role_id_);
		player_info->set_role_name(lfb_player->name_);
		player_info->set_wave(wave);

		if (my_lfb->fetch_in_record(iter->id_, 1, true) == true)
			player_info->set_is_cheer(true);
		else
			player_info->set_is_cheer(false);

		if (my_lfb->fetch_in_record(iter->id_, 2, true) == true)
			player_info->set_is_encourage(true);
		else
			player_info->set_is_encourage(false);
	}

	for (LFbPlayer::CheerRecordVec::iterator iter = my_lfb->record_vec_.begin();
			iter != my_lfb->record_vec_.end(); ++iter)
	{
		CheerRecord &record = *iter;
		ProtoCheerRecord *proto_record = respond.add_cheer_record();
		record.serialize(proto_record);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_LEAGUE_FB_CHEER_INFO, &respond);
}

int LogicLeaguer::lfb_cheer_leaguer(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102103 *, request, RETURN_LEAGUE_FB_CHEER);

	Int64 role_id = request->role_id();
	int type = request->type();
	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_LEAGUE_FB_CHEER, ERROR_LEAGUE_NO_EXIST);
	CONDITION_NOTIFY_RETURN(role_id != this->role_id(), RETURN_LEAGUE_FB_CHEER, ERROR_CHEER_SELF);
	CONDITION_NOTIFY_RETURN(role_id > 0 && (type == 1 || type == 2),
			RETURN_LEAGUE_FB_CHEER, ERROR_CLIENT_OPERATE);

	LFbPlayer *lfb_player = league->lfb_player(role_id);
	CONDITION_NOTIFY_RETURN(lfb_player != NULL, RETURN_LEAGUE_FB_CHEER, ERROR_CHEER_PLAYER);

	this->check_create_lfb_player(league);
	LFbPlayer *my_lfb = league->lfb_player(this->role_id());
	CONDITION_NOTIFY_RETURN(my_lfb != NULL, RETURN_LEAGUE_FB_CHEER, ERROR_SERVER_INNER);

	int player_wave = lfb_player->wave_;
	int my_wave = my_lfb->wave_;

	const Json::Value &lfb_base = CONFIG_INSTANCE->lfb_base();
	CONDITION_NOTIFY_RETURN(lfb_base != Json::Value::null, RETURN_LEAGUE_FB_CHEER, ERROR_CONFIG_NOT_EXIST);

	int ret = my_lfb->fetch_in_record(role_id, type, true);
	CONDITION_NOTIFY_RETURN(ret == false, RETURN_LEAGUE_FB_CHEER, ERROR_CHEER_SAME_PLAYER);

	if (type == 1)
	{
		int max_cheer = lfb_base["cheer"].asInt();
		int max_be_cheer = lfb_base["be_cheer"].asInt();
		int cheer_limit = lfb_base["cheer_limit"].asInt();
		CONDITION_NOTIFY_RETURN(my_lfb->cheer_ < max_cheer, RETURN_LEAGUE_FB_CHEER,
				ERROR_CHEER_TIMES_LIMIT);
		CONDITION_NOTIFY_RETURN(lfb_player->be_cheer_ < max_be_cheer,
				RETURN_LEAGUE_FB_CHEER, ERROR_BE_CHEER_TIMES_LIMIT);
		CONDITION_NOTIFY_RETURN(player_wave >= my_wave + cheer_limit,
				RETURN_LEAGUE_FB_CHEER, ERROR_CHEER_WAVE_LIMIT);

		my_lfb->cheer_ += 1;
		lfb_player->be_cheer_ += 1;
	}
	else
	{
		int max_encourage = lfb_base["encourage"].asInt();
		int max_be_encourage = lfb_base["be_encourage"].asInt();
		int encourage_limit = lfb_base["encourage_limit"].asInt();
		CONDITION_NOTIFY_RETURN(my_lfb->encourage_ < max_encourage, RETURN_LEAGUE_FB_CHEER,
				ERROR_CHEER_TIMES_LIMIT);
		CONDITION_NOTIFY_RETURN(lfb_player->be_encourage_ < max_be_encourage,
				RETURN_LEAGUE_FB_CHEER, ERROR_BE_CHEER_TIMES_LIMIT);
		CONDITION_NOTIFY_RETURN(player_wave <= my_wave - encourage_limit,
				RETURN_LEAGUE_FB_CHEER, ERROR_ENCOURAGE_WAVE_LIMIT);

		my_lfb->encourage_ += 1;
		lfb_player->be_encourage_ += 1;
	}

	CheerRecord record = my_lfb->create_record(role_id, type, true, lfb_player->name_);
	lfb_player->create_record(this->role_id(), type, false, this->name());

	Proto50102103 respond;
	respond.set_role_id(role_id);
	respond.set_type(type);
	respond.set_cheer(my_lfb->cheer_);
	respond.set_encourage(my_lfb->encourage_);
	record.serialize(respond.mutable_cheer_record());
	FINER_PROCESS_RETURN(RETURN_LEAGUE_FB_CHEER, &respond);
}

int LogicLeaguer::update_lfb_wave(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30100951 *, request, -1);

	League* league = this->league();
	JUDGE_RETURN(league != NULL, -1);

	this->check_create_lfb_player(league);
	LFbPlayer *my_lfb = league->lfb_player(this->role_id());
	JUDGE_RETURN(my_lfb != NULL, -1);

	my_lfb->wave_ = request->wave();
	my_lfb->tick_ = ::time(NULL);
	league->sort_lfb_player();
	this->request_lfb_wave_reward();

	return 0;
}

int LogicLeaguer::request_enter_lfb(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102105 *, request, RETURN_REQUEST_ENTER_LFB);

	League* league = this->league();
	CONDITION_NOTIFY_RETURN(league != NULL, RETURN_REQUEST_ENTER_LFB, ERROR_LEAGUE_NO_EXIST);

	this->check_create_lfb_player(league);
	LFbPlayer *my_lfb = league->lfb_player(this->role_id());

	Proto10400901 inner_request;
	inner_request.set_script_sort(request->script_sort());
	inner_request.set_cheer_num(my_lfb->be_cheer_);
	inner_request.set_encourage_num(my_lfb->be_encourage_);
	this->monitor()->dispatch_to_scene(this, &inner_request);

	FINER_PROCESS_NOTIFY(RETURN_REQUEST_ENTER_LFB);
}

int LogicLeaguer::test_reset_lfb()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	league->lfb_rank_vec_.clear();
	for (League::LFbPlayerMap::iterator iter = league->lfb_player_map_.begin();
			iter != league->lfb_player_map_.end(); ++iter)
	{
		LFbPlayer &lfb_player = iter->second;
		lfb_player.reset_every_day();
	}
	league->set_max_last_wave_player();

	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		LogicPlayer *player = NULL;
		JUDGE_CONTINUE(LOGIC_MONITOR->find_player(iter->first, player) == 0);

		player->reset_lfb_wave_reward();
		LOGIC_MONITOR->dispatch_to_scene(player->gate_sid(), player->role_id(),
				player->scene_id(), INNER_TEST_RESET_LFB);

		player->request_lfb_wave_reward();
	}

	return 0;
}

int LogicLeaguer::fetch_league_region_info()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	LeagueMember* member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, 0);

	Proto50100657 region_info;
	region_info.set_today_contri(member->today_contribute_);

	if (league->have_region_reward() == true)
	{
		region_info.set_rank(league->region_rank_);
		region_info.set_leader_draw(league->region_leader_reward_);
	}
	else
	{
		region_info.set_rank(0);
		region_info.set_leader_draw(0);
	}

	for (IntMap::iterator iter = this->leaguer_info_.region_draw_.begin();
			iter != this->leaguer_info_.region_draw_.end(); ++iter)
	{
		ProtoPairObj* pair = region_info.add_draw_map();
		pair->set_obj_id(iter->first);
		pair->set_obj_value(iter->second);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_REGION_WELFARE, &region_info);
}

int LogicLeaguer::leader_draw_region_reward()
{
	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	CONDITION_NOTIFY_RETURN(league->have_region_reward() == true,
			RETURN_LEADER_DRAW_REGION_REWARD, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(league->is_leader(this->role_id()) == true,
			RETURN_LEADER_DRAW_REGION_REWARD, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(league->region_leader_reward_ == 0,
			RETURN_LEADER_DRAW_REGION_REWARD, ERROR_CLIENT_OPERATE);

	const Json::Value& conf = CONFIG_INSTANCE->league_region(league->region_rank_);
	CONDITION_NOTIFY_RETURN(conf.empty() == false, RETURN_LEADER_DRAW_REGION_REWARD,
			ERROR_CLIENT_OPERATE);

	league->region_leader_reward_ = 1;
	this->request_add_reward(conf["leader_reward"].asInt(), ADD_FROM_LRF_LEADER);

	this->fetch_league_region_info();
	FINER_PROCESS_NOTIFY(RETURN_LEADER_DRAW_REGION_REWARD);
}

int LogicLeaguer::draw_league_region_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100659*, request, RETURN_DRAW_REGION_REWARD);

	League* league = this->league();
	JUDGE_RETURN(league != NULL, 0);

	LeagueMember* member = league->league_member(this->role_id());
	JUDGE_RETURN(member != NULL, 0);

	int rank = 0;
	if (league->have_region_reward() == true)
	{
		rank = league->region_rank_;
	}

	const Json::Value& conf = CONFIG_INSTANCE->league_region(rank);
	CONDITION_NOTIFY_RETURN(conf.empty() == false, RETURN_DRAW_REGION_REWARD,
			ERROR_CLIENT_OPERATE);

	uint index = request->index();
	CONDITION_NOTIFY_RETURN(index <= conf["daily_reward"].size(),
			RETURN_DRAW_REGION_REWARD, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->leaguer_info_.region_draw_.count(
			index) == 0, RETURN_DRAW_REGION_REWARD, ERROR_CLIENT_OPERATE);

	int need_contri = conf["daily_reward"][index - 1][0u].asInt();
	CONDITION_NOTIFY_RETURN(member->today_contribute_ >= need_contri,
			RETURN_DRAW_REGION_REWARD, ERROR_CLIENT_OPERATE);

	this->request_add_reward(conf["daily_reward"][index - 1][1u].asInt(), ADD_FROM_LRF_DAILY);
	this->leaguer_info_.region_draw_[request->index()] = true;

	this->fetch_league_region_info();
	FINER_PROCESS_NOTIFY(RETURN_DRAW_REGION_REWARD);
}

