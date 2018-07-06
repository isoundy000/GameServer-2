/*
 * AIBTCName.cpp
 *
 *  Created on: Jun 24, 2013
 *      Author: peizhibi
 */

#include "AIBTCName.h"

/*
 * ai precondition
 * */
std::string AIBTCName::AutoPre = "auto_pre";
std::string AIBTCName::DiePre = "die_pre";
std::string AIBTCName::SleepPre = "sleep_pre";
std::string AIBTCName::PatrolPre = "patrol_pre";
std::string AIBTCName::NoAimPre = "no_aim_pre";
std::string AIBTCName::HaveAimPre = "hava_aim_pre";
std::string AIBTCName::SelectModeA = "select_mode_a";
std::string AIBTCName::SelectModeB = "select_mode_b";
std::string AIBTCName::SelectModeC = "select_mode_c";
std::string AIBTCName::SelectModeD = "select_mode_d";
std::string AIBTCName::SelectModeNpc = "select_mode_npc";
std::string AIBTCName::SelectModeE = "select_mode_e";
std::string AIBTCName::SelectModeF = "select_mode_f";
std::string AIBTCName::SelectModeG = "select_mode_g";
std::string AIBTCName::SelectModeOutside = "select_mode_outside";
std::string AIBTCName::SelectModeMonsterLowBlood = "select_mode_low_blood_monster";
std::string AIBTCName::BackPre = "back_pre";
std::string AIBTCName::IdleBackPre = "idle_back_pre";
std::string AIBTCName::ChasePre = "chase_pre";
std::string AIBTCName::AttackPre = "attack_pre";
std::string AIBTCName::RecycleTickPre = "recycle_tick_pre";
std::string AIBTCName::CircleChasePre = "circle_chase_pre";
std::string AIBTCName::AttackIntervalPre = "attack_interavel_pre";
std::string AIBTCName::FollowPre = "follow_pre";
std::string AIBTCName::AimDistancePre = "aim_distance_pre";
std::string AIBTCName::NearInnerPre = "near_inner_pre";
std::string AIBTCName::PausePre = "pause_pre";
std::string AIBTCName::SelectModeMonster = "select_mode_monster";
std::string AIBTCName::AreaRecyclePre = "area_recycle_pre";
std::string AIBTCName::CheckNearLeaguePre = "check_nearleague_pre";
std::string AIBTCName::CheckNearPlayerPre = "check_nearplayer_pre";
std::string AIBTCName::NotSinglePre = "not_single_monster_pre";
std::string AIBTCName::PlayerAlivePre = "player_alive_pre";
std::string AIBTCName::AIDistancePre = "ai_distance_pre";
std::string AIBTCName::AIIntervalPre = "ai_interval_pre";
std::string AIBTCName::AIWithinDistancePre = "ai_within_dist_pre";
std::string AIBTCName::AIBeyondDistancePre = "ai_beyond_dist_pre";
std::string AIBTCName::AllRoleDistancePre = "all_role_distance_pre";
std::string AIBTCName::RelAiDiePre = "rel_ai_die_pre";
std::string AIBTCName::ChaseBeyondDistancePre = "chase_beyond_dist_pre";
std::string AIBTCName::QIXIRUNAWAYPre  = "qixi_runaway_pre";
/*
 * ai action
 * */
std::string AIBTCName::AutoAction = "auto_action";
std::string AIBTCName::DieAction = "die_action";
std::string AIBTCName::SleepAction = "sleep_action";
std::string AIBTCName::RecycleAction = "recycle_action";
std::string AIBTCName::AliveRecycleAction = "alive_recycle_action";
std::string AIBTCName::PatrolAction = "patrol_action";
std::string AIBTCName::BackAction = "back_action";
std::string AIBTCName::IdleBackAction = "idle_back_action";
std::string AIBTCName::AttackAction = "attack_action";
std::string AIBTCName::ChaseAction = "chase_action";
std::string AIBTCName::SelectAction = "select_action";
std::string AIBTCName::PatrolRouteAction = "patrol_route_action";
std::string AIBTCName::PatrolRouteBAction = "patrol_route_b_action";
std::string AIBTCName::SelectNpcAction = "select_npc_action";
std::string AIBTCName::FollowAction = "follow_action";
std::string AIBTCName::KeepAwayAction = "keep_away_action";
std::string AIBTCName::MoveAction = "move_action";
std::string AIBTCName::PauseAction = "pause_action";
std::string AIBTCName::MovetoAction = "moveto_action";
std::string AIBTCName::ComboAttackAction = "combo_attack_action";
std::string AIBTCName::NormalAttackAction = "normal_attack_action";
std::string AIBTCName::IntervalFinAction = "interval_fin_action";
std::string AIBTCName::RemoveBuffAction = "remove_buff_action";
std::string AIBTCName::AddBuffAction = "add_buff_action";
std::string AIBTCName::RelAddSkillAction = "reladdskill_action";
std::string AIBTCName::AddSkillAction = "addskill_action";
std::string AIBTCName::DirectChaseAction = "direct_chase_action";
std::string AIBTCName::GenGiftboxAction = "gen_giftbox_action";
std::string AIBTCName::LimitDisChaseAction = "limit_dis_chase_action";
std::string AIBTCName::ReturnPatrolAction = "return_patrol_action";
std::string AIBTCName::NotifyBossRemoveAttr = "notify_boss_remove_attr_action";
std::string AIBTCName::NotifyShowBoss = "notify_show_boss_action";
std::string AIBTCName::QIXIRUNAWAY = "qixi_run_away_action";
std::string AIBTCName::QIXIPatrolAction = "qixi_patrol_action";
/*
 * ai condition flag
 * */
std::string AIBTCName::NO_MOVE = "no_move";
std::string AIBTCName::NO_ATTACK = "no_attack";
std::string AIBTCName::NO_BE_ATTACKED = "no_be_attacked";
std::string AIBTCName::NO_AIM = "no_aim";
std::string AIBTCName::RECYCLE_ON_ATTACKED = "recycle_on_attacked";

/*
 * ai field name
 * */
std::string AIBTCName::RECYCLE_TICK_FIELD = "recycle_tick";
std::string AIBTCName::NPC_SORT_FIELD = "npc_sort";
std::string AIBTCName::PATROL_PATH_FIELD = "patrol_path";
std::string AIBTCName::ATTACK_INTERVAL_FIELD = "auto_attack_tick";
std::string AIBTCName::SELECT_C_RADIUS_FIELD = "select_c_radius";
std::string AIBTCName::CIRCLE_CHASE_FIELD = "circle_chase_raius";
std::string AIBTCName::MAX_CHASE_RADIUS_FIELD = "max_chase_radius";
std::string AIBTCName::ATTACK_DISTANCE_FIELD = "attack_distance";
std::string AIBTCName::AI_DISTANCE_FIELD = "ai_distance";
std::string AIBTCName::AI_NORMAL_SKILL = "normal_skill";
std::string AIBTCName::AI_COMBO_SKILL_LIST = "combo_skill_list";
std::string AIBTCName::AI_RANDOM_COMBO_SKILL_LIST = "random_combo_skill";
std::string AIBTCName::AI_ADD_BUFF = "add_buff";
std::string AIBTCName::AI_REMOVE_BUFF = "remove_buff";

