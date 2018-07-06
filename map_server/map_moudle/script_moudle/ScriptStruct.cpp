/*
 * ScriptStruct.cpp
 *
 * Created on: 2013-12-27 16:03
 *     Author: lyz
 */

#include "ScriptStruct.h"
#include "ProtoDefine.h"

#include "GameHeader.h"
#include <mongo/client/dbclient.h>

ScriptPlayerRel::ScriptPlayerRel(void) :
    __gate_sid(0), __role_id(0), __script_sort(0), __script_id(0), __progress_id(0), __scene_id(0),
    __team_id(0), __level(0), __cheer_num(0), __encourage_num(0), __used_times(0), __buy_times(0),
    __fresh_tick(0, 0), __pass_piece(0), __pass_chapter(0), __piece(0), __chapter(0), __vip_type(0),
    __day_online_sec(0), __progress_obj(0)
{
	this->__progress_obj = new BSONObj();
}

ScriptPlayerRel::~ScriptPlayerRel(void)
{
	SAFE_DELETE(this->__progress_obj);
}

void ScriptPlayerRel::reset(void)
{
    this->__gate_sid = 0;
    this->__role_id = 0;
    this->__script_sort = 0;
    this->__script_id = 0;
    this->__progress_id = 0;
    this->__scene_id = 0;
    this->__team_id = 0;
    this->__level = 0;
    this->__cheer_num = 0;
    this->__encourage_num = 0;

    this->__used_times = 0;
    this->__buy_times = 0;
    this->__fresh_tick = Time_Value::zero;

    this->__pass_piece = 0;
    this->__pass_chapter = 0;
    this->__piece = 0;
    this->__chapter = 0;
    this->__vip_type = 0;
    this->__day_online_sec = 0;
    this->teamer_set_.clear();
    this->replacement_set_.clear();

    *(this->__progress_obj) = BSONObj();
}

ScriptPlayerRel &ScriptPlayerRel::operator=(const ScriptPlayerRel &obj)
{
    this->__gate_sid = obj.__gate_sid;
    this->__role_id = obj.__role_id;
    this->__script_sort = obj.__script_sort;
//    this->__script_id = obj.__script_id;
    this->__progress_id = obj.__progress_id;
    this->__scene_id = obj.__scene_id;
    this->__team_id = obj.__team_id;
    this->__level = obj.__level;

    this->__used_times = obj.__used_times;
    this->__buy_times = obj.__buy_times;
    this->__fresh_tick = obj.__fresh_tick;

    this->__pass_piece = obj.__pass_piece;
    this->__pass_chapter = obj.__pass_chapter;
    this->__piece = obj.__piece;
    this->__chapter = obj.__chapter;
    this->__vip_type = obj.__vip_type;
    this->__day_online_sec = obj.__day_online_sec;

    this->teamer_set_ = obj.teamer_set_;
    this->replacement_set_ = obj.replacement_set_;
    *(this->__progress_obj) = *(obj.__progress_obj);

    return *this;
}

void ScriptDetail::Team::reset(void)
{
    this->__team_id = 0;
    this->__teamer_map.clear();
    this->__history_role.clear();
    this->__used_times_tick_map.clear();
}

void ScriptDetail::Scene::reset(void)
{
    this->__cur_scene = 0;
    this->__cur_floor = 0;

    this->__last_scene = 0;
    this->__last_floor = 0;

    this->__next_scene = 0;
    this->__next_floor = 0;

    this->__passed_scene = 0;
    this->__passed_floor = 0;

    this->__started_scene.clear();
    this->__finish_scene.clear();

    this->__scene_flag_set.reset();
    this->__notify_finish_tick = 0;

    this->__monster_level_index = -1;

    this->__monster_fail_notify_tick = Time_Value::zero;
    this->__monster_failure_tick = Time_Value::zero;
}

void ScriptDetail::Tick::reset(void)
{
    this->__ready_sec = 0;
    this->__used_sec = 0;
    this->__total_sec = 0;
    this->__inc_sec = 0;

    this->__ready_tick = 0;
    this->__begin_tick = 0;
    this->__end_tick = 0;
}

void ScriptDetail::Monster::reset(void)
{
	this->reset_everytime();

    this->__evencut = 0;
    this->__max_evencut = 0;
    this->__clear_evencut_tick = Time_Value::zero;

    this->__boss_hurt = 0;
    this->__player_hurt = 0;
    this->__magic_monster.reset();
}

void ScriptDetail::Monster::reset_everytime()
{
	this->__total_monster = 0;
	this->__left_monster = 0;

	this->__config_monster_map.clear();
	this->__monster_left_map.clear();
	this->__monster_killed_map.clear();
	this->__monster_total_map.clear();
	this->__poem_text.clear();

	this->__text_size = 0;
	this->__appear_text_max = 0;
	this->__appear_text_set.clear();
	this->__special_sort_set.clear();
}

void ScriptDetail::MagicMonster::reset()
{
	this->_boss_ai_id = 0;
	this->_monster_ai_id = 0;
	this->_role_id = 0;
	this->_flag = 0;
	this->_name.clear();
}

void ScriptDetail::Wave::reset(void)
{
	this->reset_everytime();

    this->__pass_wave = 0;
    this->__spirit_value = 0;
    this->__matrix_level_map.clear();
    this->__active_puppet_flag.clear();
    this->__active_puppet.clear();
}

void ScriptDetail::Wave::reset_everytime()
{
	this->__cur_wave = 0;
    this->__total_wave = 0;
	this->__finish_wave = 0;
	this->__begin_wave = 0;
	this->__wave_monster_map.clear();
	this->__wave_killed_map.clear();
    this->__wave_total_map.clear();
}

void ScriptDetail::Relive::reset(void)
{
    this->__total_relive = 0;
    this->__left_relive = 0;
    this->__used_relive = 0;
    this->__used_item_relive = 0;
}

void ScriptDetail::Piece::reset(void)
{
    this->__piece = 0;
    this->__chapter = 0;
    this->__piece_chapter_map.clear();
}

ScriptDetail::PlayerAward::PlayerAward(void)
{
	PlayerAward::reset();
}

void ScriptDetail::PlayerAward::reset(void)
{
    this->__dps = 0;
    this->__exp = 0;
    this->__copper = 0;
	this->__additional_exp = 0;
    this->__item_map.clear();
}

void ScriptDetail::reset(void)
{
    this->__script_sort = 0;
    this->__progress_id = 0;
    this->__stop_times = 0;
    this->__exit_script_state = 0;
    this->__cheer_num = 0;
    this->__encourage_num = 0;

    this->__total_used_tick = 0;
    this->__star_lvl.clear();
    this->__player_set.clear();
    this->__recycle_tick = Time_Value::zero;
    this->__max_monster_num = 0;
    this->__is_single = false;
    this->__protect_npc_sort = 0;
    this->__owner_id = 0;

    this->__team.reset();
    this->__scene.reset();
    this->__tick.reset();
    this->__monster.reset();
    this->__wave.reset();
    this->__relive.reset();
    this->__piece.reset();
    this->__player_award_map.clear();

    this->couple_sel_.clear();
    this->teamer_set_.clear();
    this->replacements_set_.clear();
    this->src_replacements_set_.clear();
}

void ScriptTeamDetail::reset(void)
{
    this->__team_id = 0;
    this->__teamer_list.clear();
    this->__script_list.clear();
    this->__replacements_map.clear();
}

ScriptPlayerDetail::TypeRecord::TypeRecord(void)
{
	TypeRecord::reset();
}

void ScriptPlayerDetail::TypeRecord::reset(void)
{
	this->__script_type = 0;
	this->__max_script_sort = 0;
	this->__pass_wave = 0;
	this->__pass_chapter = 0;
	this->__notify_wave = 0;
	this->__notify_chapter = 0;
	this->__start_wave = 0;
	this->__start_chapter = 0;
	this->__used_times_tick = Time_Value::zero;
	this->__is_sweep = 0;
	this->__reward_map.clear();
}

ScriptPlayerDetail::ScriptWaveRecord::ScriptWaveRecord(void)
{
	ScriptWaveRecord::reset();
}

void ScriptPlayerDetail::ScriptWaveRecord::reset(void)
{
	this->__script_wave_id = 0;
	this->__is_get = 0;
}

ScriptPlayerDetail::ScriptRecord::ScriptRecord(void)
{
	ScriptRecord::reset();
}

void ScriptPlayerDetail::ScriptRecord::reset(void)
{
    this->__script_sort = 0;
    this->__used_times = 0;
    this->__buy_times = 0;
    this->__couple_buy_times = 0;
    this->__used_times_tick = Time_Value::zero;
    this->__enter_script_tick = Time_Value::zero;
    this->__progress_id = 0;
    this->__best_use_tick = Time_Value::DAY;
    this->__award_star = 0;
    this->__is_first_pass = 0;
    this->__day_pass_times = 0;
    this->__is_even_enter = 0;
    this->__protect_beast_index = -1;
}

void ScriptPlayerDetail::PieceRecord::PieceChapterInfo::reset(void)
{
    this->__chapter_key = 0;
    this->__used_sec = 0;
    this->__used_times = 0;
    this->__totay_pass_flag = 0;
}

void ScriptPlayerDetail::LegendTopInfo::FloorInfo::reset(void)
{
	this->__floor_id = 0;
	this->__pass_tick = 0;
	this->__totay_pass_flag = 0;
}

void ScriptPlayerDetail::LegendTopInfo::reset(void)
{
	this->__pass_floor = 0;
	this->__today_rank = 0;
	this->__is_sweep = 0;
	this->__piece_map.clear();
}

void ScriptPlayerDetail::PieceRecord::reset(void)
{
    this->__pass_piece = 0;
    this->__pass_chapter = 0;
    this->__pass_chapter_map.clear();
    this->__piece_star_award_map.clear();
    this->__pass_chapter_item.clear();
}

void ScriptPlayerDetail::reset(void)
{
    this->__script_id = 0;
    this->__script_sort = 0;
    this->__task_listen = false;
    this->__trvl_total_pass = 0;

    this->__prev_scene = 0;
    this->__prev_coord.reset();
    this->__prev_blood = 0;
    this->__prev_magic = 0;

    this->__skill_id = 0;

    this->__piece_record.reset();
    this->__legend_top_info.reset();
    this->__sword_top_info.reset();

    this->__type_map.clear();
    this->__record_map.clear();
    this->__script_wave_map.clear();
    this->__first_pass_item.clear();
    this->__first_script_vc.clear();

    this->__max_trvl_pass = CONFIG_INSTANCE->const_set("max_trvl_pass");
}

bool ScriptPlayerDetail::is_have_trvl_red()
{
	return this->__trvl_total_pass < this->__max_trvl_pass;
}

void LegendTopPlayer::reset(void)
{
	this->refresh_tick_ = 0;
	this->player_map_.clear();
}

void LegendTopPlayer::PlayerInfo::reset(void)
{
	this->player_id_ = 0;
	this->name_.clear();
	this->fight_score_ = 0;
	this->floor_ = 0;
	this->rank_ = 0;
	this->tick_ = 0;
}

void LegendTopPlayer::PlayerInfo::serialize(ProtoLegendTopRank *proto_rank)
{
	proto_rank->set_role_id(this->player_id_);
	proto_rank->set_role_name(this->name_);
	proto_rank->set_rank(this->rank_);
	proto_rank->set_floor(this->floor_);
	proto_rank->set_fight_score(this->fight_score_);
}

void HistoryChapterRecord::reset(void)
{
    this->__first_top_level_player = 0;
    this->__first_top_level_role_name.clear();
    this->__chapter_key = 0;
    this->__best_use_tick = 0;
}

void ScriptSceneDetail::reset(void)
{
    this->__left_monster = 0;
    this->__killed_monster = 0;
    this->__total_monster = 0;
}

FourAltarDetail::EffectDetail::EffectDetail(void) :
    __target(0), __effect_type(0), __effect_value(0.0), __effect_percent(0.0),
    __effect_interval(0.0), __effect_last(0.0), __skill_interval(0.0), 
    __text_id(0)
{ /*NULL*/ }

void FourAltarDetail::EffectDetail::reset(void)
{
    this->__target = 0;
    this->__effect_type = 0;
    this->__effect_value = 0.0;
    this->__effect_percent = 0.0;
    this->__effect_interval = 0.0;
    this->__effect_last = 0.0;
    this->__skill_interval = 0.0;
    this->__text_id = 0;
}

void FourAltarDetail::SceneSkillDetail::reset(void)
{
    this->__once_buff_list.clear();
    this->__scene_skill.reset();
    this->__skill_check_tick = Time_Value::zero;
    this->__tip_id = 0;
}

void FourAltarDetail::reset(void)
{
    this->__boss_id_set.clear();
    this->__scene_skill_map.clear();
    this->__min_skill_check_tick = Time_Value::zero;
    this->__min_interval_check_tick = Time_Value::zero;
    this->__active_boss_set.clear();
}

DropDragonHole_Role::DropDragonHole_Role()
{
	this->role_id_ = 0;
	this->scirpt_skill_id_ = 0;
	this->skill_buff_id_ = 0;
	this->gather_sort_ = 0;
}
void DropDragonHole_Role::reset()
{
	this->role_id_ = 0;
	this->scirpt_skill_id_ = 0;
	this->skill_buff_id_ = 0;
	this->gather_sort_ = 0;
}
