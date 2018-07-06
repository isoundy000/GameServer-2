/*
 * MapUnit.cpp
 *
 * Created on: 2013-01-18 10:23
 *     Author: glendy
 */

#include "MapUnit.h"
#include "MapPlayerEx.h"
#include "Transaction.h"
#include "MapMonitor.h"
#include "PoolMonitor.h"
#include "LeagueMonitor.h"
#include "TransactionMonitor.h"
#include "LeagueReginFightSystem.h"
#include "TMarenaMonitor.h"

#include "DaemonServer.h"
#include "ProtoDefine.h"
#include "BaseScript.h"
#include "SMBattleSystem.h"
#include "LeagueWarSystem.h"
#include "GlobalScriptHistory.h"

#include "AreaMonitor.h"
#include "SceneLineManager.h"
#include "MLGameSwither.h"
#include "AIManager.h"
#include "FloatAI.h"
#include "GameAI.h"

#include "MapLogicPlayer.h"
#include "TrvlScriptMonitor.h"
#include "TrvlArenaMonitor.h"
#include "WorldBossSystem.h"
#include "ScriptSystem.h"
#include "MonsterAttackSystem.h"
#include "TrvlWeddingMonitor.h"
#include "TrvlRechargeMonitor.h"
#include "TrvlWbossMonitor.h"
#include "TrvlBattleMonitor.h"
#include "TrvlPeakMonitor.h"

int MapUnit::type(void)
{
    return MAP_UNIT;
}

UnitMessage *MapUnit::pop_unit_message(void)
{
    return MAP_MONITOR->unit_msg_pool()->pop();
}

int MapUnit::push_unit_message(UnitMessage *msg)
{
    return MAP_MONITOR->unit_msg_pool()->push(msg);
}

int MapUnit::process_block(UnitMessage *unit_msg)
{
    uint32_t len = unit_msg->__len;
    int32_t sid = unit_msg->__sid, recogn = unit_msg->__msg_head.__recogn,
            trans_id = unit_msg->__msg_head.__trans_id;
    int64_t role_id = unit_msg->__msg_head.__role_id;

    Message *msg_proto = unit_msg->proto_msg();
//    Block_Buffer *msg_buff = unit_msg->data_buff();
    int ivalue = unit_msg->ivalue();

    Transaction *transaction = 0;
    if (trans_id > 0)
    	TRANSACTION_MONITOR->find_transaction(trans_id, transaction);

    /*非玩家时处理*/
    switch (recogn)
    {
        case INNER_TIMER_TIMEOUT:
            return POOL_MONITOR->game_timer_timeout(ivalue);
        case INNER_UPDATE_CONFIG:
        {
            DAEMON_SERVER->request_update_config();
            TRVL_ARENA_MONITOR->clear_history_db();
            return 0;
        }
        case INNER_MAP_LOGIN:
            return MAP_MONITOR->request_map_player_login(sid, role_id, msg_proto);
        case TRANS_LOAD_MAP_PLAYER:
            return MAP_MONITOR->after_load_player(transaction);
        case INNER_MAP_LOGOUT:
            return PLAYER_MANAGER->process_map_player_logout(sid, role_id);
//        case CLIENT_MAP_CONNECT:
//            return MAP_MONITOR->connect_map_broad(sid, msg_proto);
        case INNER_MAP_CLOSE:
            return MAP_MONITOR->disconnect_map_broad(sid);
        case INNER_MAP_UPDATE_SID:
        	return MAP_MONITOR->update_map_player_sid(sid, role_id, NULL);
        case INNER_TRANSFER_SCENE_BEGIN:
            return PLAYER_MANAGER->update_transfer_base(sid, role_id, msg_proto);
        case INNER_TRANSFER_SCENE_END:
            return PLAYER_MANAGER->finish_sync_transfer(sid, msg_proto);
        case INNER_SYNC_PLAYER_MOVE:
        case INNER_SYNC_PLAYER_FIGHT:
        case INNER_SYNC_PLAYER_SCRIPT:
        case INNER_SYNC_PLAYER_VIP:
        case INNER_SYNC_PLAYER_MAPTEAM:
        case INNER_SYNC_PLAYER_KILL:
        case INNER_SYNC_PLAYER_TINY:
        case INNER_SYNC_PLAYER_ESCORT:
        case INNER_SYNC_PLAYER_SHAPE_AND_LABEL:
        case INNER_SYNC_PLAYER_BATTLEGRUOND:
        case INNER_SYNC_LEAGUER_INFO:
            return PLAYER_MANAGER->sync_transfer_map(role_id, recogn, msg_proto);
        case INNER_SYNC_PLAYER_ONLINE:
            return PLAYER_MANAGER->sync_transfer_online(sid, dynamic_cast<Proto30400105 *>(msg_proto));
        case INNER_SYNC_MAP_OBJ_BY_BSON:
        	return PLAYER_MANAGER->sync_transfer_map_by_bson(role_id, msg_proto);

        case IM_QUIT_LEAGUE:
        	return LEAGUE_MONITOR->handle_quit_league(sid, msg_proto);
        case INNER_RENAME_LEAGUE:
        	return LEAGUE_MONITOR->handle_rename_league(sid, role_id, msg_proto);
        case INNER_MAP_LEAGUE_CREATE_BOSS:
        {
        	LEAGUE_MONITOR->create_map_league(msg_proto);
        	LEAGUE_MONITOR->create_league_boss_scene(msg_proto);
        	return 0;
        }
        case INNER_SYNC_LEADER_INFO:
        {
        	return LEAGUE_MONITOR->update_league_leader(msg_proto);
        }
		case INNER_MAP_LEAGUE_DISMISS_BOSS:
		{
			LEAGUE_MONITOR->recycle_map_league(msg_proto);
			LEAGUE_MONITOR->recycle_league_boss(msg_proto);
			return 0;
		}
        case INNER_MAP_REQ_ENTER_SCENE:
        	return MAP_MONITOR->request_enter_scene_begin(sid, role_id, msg_proto);
		case INNER_MAP_AVERAGE_LEVEL:
			return MAP_MONITOR->fetch_average_level_done(msg_proto);
        case INNER_MAP_GENERATE_BOSS:
        	return MAP_MONITOR->festival_generate_boss(msg_proto);
        case INNER_MAP_FESTIVAL_INFO:
        	return MAP_MONITOR->set_festival_activity_info(msg_proto);
        case INNER_MAP_BIG_ACT_INFO:
        	return MAP_MONITOR->set_big_activity_info(msg_proto);
        case INNER_MAP_LEAGUE_REGION_INFO:
        	return LRF_MONITOR->apply_lrf_operate(sid, role_id, msg_proto);
        case INNER_MAP_LOAD_LEAGUE_REGION:
        	return LRF_MONITOR->load_league_war_info_begin(true);

		case INNER_REQUEST_WORLD_BOSS_INFO:
			return WORLD_BOSS_SYSTEM->request_fetch_wboss_info(sid, role_id, msg_proto);
		case INNER_LOGIN_GET_WORLD_BOSS_INFO:
			return WORLD_BOSS_SYSTEM->login_send_red_point(sid, role_id, msg_proto);
		case INNER_REQUEST_TRVL_WBOSS_INFO:
			return TRVL_WBOSS_MONITOR->request_fetch_wboss_info(sid, role_id, msg_proto);

		case INNER_UPDATE_MATTACK_LABEL_INFO:
			return MONSTER_ATTACK_SYSTEM->update_label_record(msg_proto);
        case IM_AREA_CREATE_FIELD:
        	return AREA_MONITOR->create_area_field(msg_proto);
		case IM_BACK_CLOSE_ACTIVITY:
			return MAP_MONITOR->back_force_close_activity(msg_proto);

		case INNER_LEAGUE_BOSS_SUMMON:
			return LEAGUE_MONITOR->summon_league_boss(sid, role_id, msg_proto);
		case INNER_SYNC_LEAGUE_FLAG_LVL:
			return LEAGUE_MONITOR->fetch_league_flag(msg_proto);
        
        case INNER_MAP_SCRIPT_ENTER_CHECK:
            return BaseScript::check_enter_script(sid, role_id, msg_proto);
        case TRANS_LOAD_SCRIPT_PROGRESS:
            return BaseScript::after_load_script_progress(transaction);
        case INNER_MAP_SCRIPT_HISTORY:
            return GLOBAL_SCRIPT_HISTORY->request_script_history_info(sid, role_id, msg_proto);
        case TRANS_LOAD_SCENE_LINE_CONFIG:
            return MAP_MONITOR->scene_line_manager()->sync_scene_line_config(transaction);
        case TRANS_LOAD_SHOP_MODE:
        	return MAP_MONITOR->db_map_load_mode_done(transaction);
		case INNER_LOGIC_SYNC_GAME_SWITCHER_TO_MAP:
			return ML_SWITCHER_SYS->sync_from_logic(msg_proto);
		case INNER_FETCH_LEGEND_TOP_RANK:
			return SCRIPT_SYSTEM->request_fetch_rank_info(sid, role_id, msg_proto);
		case INNER_UPDATE_LEGEND_TOP_RANK:
			return SCRIPT_SYSTEM->sweep_update_top_rank(sid, msg_proto);
		case INNER_HANDLE_COUPLE_FB_OFFLINE:
			return SCRIPT_SYSTEM->add_couple_script_times(msg_proto);
		case INNER_LOGIN_UPDATE_COUPLE_FB:
			return SCRIPT_SYSTEM->login_fetch_couple_script_times(sid, role_id, msg_proto);
		case INNER_WEDDING_CRUISE:
			return this->process_wedding_cruise_begin(msg_proto);
		case INNER_UPDATE_DOUBLE_SCRIPT:
			return SCRIPT_SYSTEM->set_double_script(sid, msg_proto);
		case INNER_FETCH_DOUBLE_SCRIPT:
			return SCRIPT_SYSTEM->sweep_fetch_script_mult(sid, role_id, msg_proto);

		//travel script
		case INNER_MAP_TRVL_SCRIPT_CREATE_TEAM:
			return TRVL_SCRIPT_MONITOR->modify_team_operate(sid, role_id, msg_proto);
		case INNER_MAP_TRVL_SCRIPT_TEAM_LIST:
			return TRVL_SCRIPT_MONITOR->inner_team_operate(sid, role_id, msg_proto);

		//travel arena
		case INNER_MAP_TRVL_ARENA_SIGN:
			return TRVL_ARENA_MONITOR->sign(sid, role_id, msg_proto);
		case INNER_MAP_TRVL_ARENA_OPERATE:
			return TRVL_ARENA_MONITOR->operate(sid, role_id, msg_proto);
		case INNER_MAP_TMARENA_RANK_INFO:
			return TRVL_MARENA_MONITOR->fetch_rank_info(sid, role_id);

		//travel wedding
		case INNER_MAP_TRVL_WEDDING_INFO:
			return TRVL_WEDDING_MONITOR->wedding_rank_info(sid, role_id, msg_proto);
		case INNER_MAP_FETCH_TRVL_WEDDING_INFO:
			return TRVL_WEDDING_MONITOR->fetch_wedding_rank_info(sid, role_id, msg_proto);
		case INNER_MAP_FETCH_WEDDING_ACT_REWARD:
			return TRVL_WEDDING_MONITOR->fetch_wedding_rank_reward(sid, role_id, msg_proto);

		//travel recharge rank
		case INNER_MAP_UPDATE_RECHARGE_RANK:
			return TRVL_RECHARGE_MONITOR->add_recharge_rank_info(sid, role_id, msg_proto);
		case INNER_MAP_FETCH_RECHARGE_RANK:
			return TRVL_RECHARGE_MONITOR->fetch_recharge_rank_info(sid, role_id, msg_proto);
		case INNER_MAP_TEST_SEND_RECHARGE_MAIL:
			return TRVL_RECHARGE_MONITOR->test_send_recharge_mail(sid, role_id, msg_proto);
        case INNER_MAP_BACK_TRAV_RECHARGE_RANK:
            return TRVL_RECHARGE_MONITOR->fetch_back_trvl_recharge_rank_info(sid, role_id, msg_proto);
        case INNER_MAP_UPDATE_TRVL_RECHARGE_RANK:
            return TRVL_RECHARGE_MONITOR->add_back_recharge_rank_info(sid, role_id, msg_proto);
        case TRANS_CORRECT_TRVL_RANK:
            return TRVL_RECHARGE_MONITOR->correct_trvl_rank(transaction);

        // travel battle
        case INNER_TRVL_BATTLE_EVERY_LIST:
            return TRVL_BATTLE_MONITOR->process_fetch_every_list(sid, role_id, msg_proto);
        case INNER_TRVL_BATTLE_VIEW_PLAYER:
            return TRVL_BATTLE_MONITOR->process_fetch_view_player_info(sid, role_id, msg_proto);
        case INNER_TRVL_BATTLE_TEST_CMD:
        	return TRVL_BATTLE_MONITOR->process_test_activity(sid, role_id, msg_proto);
        case INNER_TRVL_BATTLE_MAIN_PANNEL:
            return TRVL_BATTLE_MONITOR->process_fetch_main_pannel(sid, role_id, msg_proto);

        // travel peak
        case INNER_SYNC_TRVL_TEAM_INFO:
        	return TRVL_PEAK_MONITOR->sync_travel_team_info(sid, role_id, msg_proto);
        case INNER_SYNC_OFFLINE_HOOK:
        	return TRVL_PEAK_MONITOR->sync_offline_player_info(sid, role_id, msg_proto);
        case INNER_SYNC_TRVL_PEAK_SIGNUP:
        	return TRVL_PEAK_MONITOR->process_signup_travel_team(sid, role_id, msg_proto);
        case INNER_FETCH_TRAVEL_PEAK_TICK:
        	return TRVL_PEAK_MONITOR->process_fetch_travel_peak_tick(sid, role_id, msg_proto);
        case INNER_AREA_OTHER_TEAM_DETAIL:
        	return TRVL_PEAK_MONITOR->process_fetch_trvl_team_detail(sid, role_id, msg_proto);
        case INNER_SYNC_TEAMER_FORCE_INFO:
        	return TRVL_PEAK_MONITOR->sync_update_trvl_teamer_force_info(sid, role_id, msg_proto);
        case INNER_FETCH_TRVL_PEAK_RANK:
        	return TRVL_PEAK_MONITOR->process_fetch_trvl_peak_rank(sid, role_id, msg_proto);

        // all travel
        case INNER_FETCH_TRAVEL_AREA:
            return MAP_MONITOR->process_fetch_travel_area(sid, msg_proto);
        case INNER_MAP_TRVL_KEEP_ALIVE:
        	return MAP_MONITOR->keep_alive_msg_done(sid);

        default:
            break;
    }

    MapPlayerEx *player = 0;
    if (MAP_MONITOR->find_player(role_id, player) != 0)
    {
        MSG_USER("ERROR can't found map player %d %ld", recogn, role_id);

        if (transaction != NULL)
        {
        	transaction->rollback();
        	transaction = NULL;
        }
        return -1;
    }

    /*有玩家时，任何情况处理*/
    switch (recogn)
    {
    case INNER_TRANSFER_LOGIC_INFO_END:
        return player->start_map_sync_transfer();
    default:
    	break;
    }

    /*非传送时，才处理*/
    if (player->transfer_flag() == true)
    {
    	// TODO: send force logout to logic thread
    	Time_Value nowtime = Time_Value::gettimeofday();
    	if (player->transfer_timeout_tick() < nowtime)
    	{
    		Proto31400005 req;
    		req.set_role_id(player->role_id());
    		player->send_to_logic_thread(req);
    		player->sign_out(false);
    	}

        MSG_USER("ERROR player transfer[will force map logic exit] map sign out %d %ld %d timeout[%ld.%06ld  <-> %ld.%06d]",
        		recogn, role_id, player->transfer_flag(), player->transfer_timeout_tick().sec(), player->transfer_timeout_tick().usec(),
        		nowtime.sec(), nowtime.usec());

        if (transaction != NULL)
        {
        	transaction->rollback();
        	transaction = NULL;
        }

        return -1;
    }

    if (player->role_detail().__name.empty() == true)
    {
    	MSG_USER("ERROR empty role info unbind %ld", role_id);
    	MAP_MONITOR->unbind_player(role_id);
    	return -1;
    }

    switch (recogn)
    {
    	case CLIENT_ML_TEST_COMMAND:
    		return player->map_test_command(msg_proto);
    	case CLIENT_PICKUP_GOODS:
    		return player->pick_up_drop_goods_begin(msg_proto);
    	case INNER_MAP_PICK_UP_RESULT:
    		return player->pick_up_drop_goods_done(msg_proto);
    	case CLIENT_GATHER_STATE_B:
    		return player->gather_state_begin(msg_proto);
    	case CLIENT_GATHER_STATE_E:
    		return player->gather_state_end();
    	case CLIENT_GATHER_GOODS:
    		return player->gather_goods_begin(msg_proto);
    	case INNER_ML_GATHER_GOODS:
    		return player->gather_goods_done(msg_proto);
    	case CLIENT_AUTO_USE_BLOOD:
    		return player->set_auto_cont_blood(msg_proto);
    	case INNER_MAP_ADD_CONT_BLOOD:
    		return player->check_and_add_cur_blood(player->fetch_buy_blood_value());
    	case CLIENT_BLOOD_CONTAIN_NOTIPS:
    		return player->set_cont_blood_notips(msg_proto);
    	case CLIENT_FETCH_ROLE_DETAIL:
    		return player->fetch_role_detail();

        case INNER_MAP_ML_LOGIN_DONE:
        	return player->finish_login_map_logic(msg_proto);
        case INNER_MAP_ACCELERATE_FORBIT:
            return player->process_accelerate_forbit(msg_proto);
        case INNER_MAP_EXIT_SYSTEM:
        	return player->request_force_exit_system(msg_proto);

        case CLIENT_OBTAIN_AREA:
            return player->client_obtain_area_info();
        case CLIENT_SCHEDULE_MOVE:
            return player->schedule_move_action(msg_proto);
        case CLIENT_TRANSFER_SCENE:
            return player->transfer_to_other_scene(msg_proto);
        case CLIENT_TRANSFER_POINT:
        	return player->transfer_to_point(msg_proto, true);
        case CLIENT_TRANSFER_MAIN_TOWN:
        	return player->transfer_to_main_town();
        case CLIENT_TRANSFER_ESCORT:
        	return player->transfer_to_escort_npc(msg_proto);

        case CLIENT_PLAYER_GLIDE:
            return player->request_glide(msg_proto);
        case INNER_MAP_TRANSFER_FEE_FINISH:
        	return player->transfer_fee_deduct_finish(msg_proto);
        case CLIENT_COLLECT_CHESTS_ENTER:
        	return player->request_enter_collect_chests();

        case CLIENT_ANSWER_ACTIVITY_ENTER:
        	return player->request_enter_answer_activity();
        case CLIENT_HOTSPRING_ACTIVITY_ENTER:
        	return player->request_enter_hotspring_activity();
        case CLIENT_PREPARE_FIGHT:
            return player->prepare_fight_skill(msg_proto);
//        case CLIENT_LAUNCH_FIGHT:
//            return player->launch_fight_skill(msg_proto);
        case CLIENT_RELIVE:
            return player->request_relive(msg_proto);
        case CLIENT_PK_STATE:
            return player->request_pk_state(msg_proto);
        case INNER_MAP_UPDATE_SKILL:
        	return player->sync_update_skill(msg_proto);
        case INNER_RES_PACK_ITEM:
            return player->respond_pack_item(msg_proto);
        case CLIENT_DART_FORWARD:
            return player->request_dart_forward(msg_proto);
        case INNER_ML_SYNC_LEVEL_INFO:
        	return player->finish_ml_level_upgrade();

        case INNER_MAP_REFRESH_PRO:
        	return player->logic_refresh_fight_property(msg_proto);
        case INNER_MAP_ADD_EXP:
            return player->sync_add_exp(msg_proto);
        case INNER_MAP_EXP_PERCENT:
        	return player->sync_add_exp_percent(msg_proto);
        case INNER_LOGIC_GET_QUINTUPLE_ONLINE_BUFF:
        	return player->add_quintuple_exp_percent(msg_proto);
        case INNER_LOGIC_REFRESH_BUFF_INFO:
        	return player->refresh_buff_status(msg_proto);
        case INNER_LOGIC_RM_QUINTUPLE_ONLINE_BUFF:
        	return player->remove_quintuple_exp_percent(msg_proto);
        case INNER_MAP_TASK_CHECK_NPC:
        	return player->check_is_near_npc(msg_proto);
        case INNER_ML_FINISHED_TASK:
            return player->check_is_near_finish_npc(msg_proto);
        case INNER_MAP_ADD_BLOOD:
        	return player->sync_direct_add_blood(msg_proto);
        case INNER_MAP_DIRECT_ADD_MAGIC:
        	return player->sync_direct_add_magic(msg_proto);
        case INNER_MAP_PROP_LEVELUP:
        	return player->prop_item_level_up();
        case INNER_MAP_USE_GOODS:
        	return player->map_use_pack_goods(msg_proto);
        case INNER_MAP_SYNC_ARENA_SHAPE:
        	return player->sync_arena_shape_info();
        case INNER_SYNC_ROLE_NAME:
            return player->sync_update_player_name(msg_proto);
        case INNER_SYNC_ROLE_SEX:
        	return player->sync_update_player_sex();

        case INNER_MAP_BRANCH_TASK_INFO:
        	return player->update_branch_task_info(msg_proto);
        case INNER_MAP_BEAST_ADD:
        	return player->logic_upsert_beast(msg_proto);
        case INNER_MAP_BEAST_REMOVE:
        	return player->callback_cur_beast(msg_proto);
        case INNER_MAP_BEAST_REFRESH_PROP:
        	return player->refresh_beast_fight_prop(msg_proto);
        case INNER_MAP_MOUNT_REFRESH_SHAPE:
        	return player->update_mount_info(msg_proto);
        case INNER_MAP_SWORD_POOL_STYLE_LVL:
        	return player->update_spool_info(msg_proto);
        case INNER_MAP_FASHION_STYLE:
        	return player->update_fashion_info(msg_proto);
        case INNER_MAP_TRANSFER_INFO:
        	return player->update_transfer_info(msg_proto);
        case INNER_MAP_EQUIP_REFRESH_SHAPE:
        	return player->refresh_player_equip_shape(msg_proto);
        case INNER_MAP_UPDATE_REFINE_VIEW:
        	return player->refresh_equip_refine_lvl(msg_proto);
        case INNER_ML_REFRESH_FAIRY_ACT:
        	return player->refresh_fairy_act(msg_proto);
        case CLIENT_BEAST_MOVE:
            return player->beast_schedule_move(msg_proto);

        case INNER_MAP_LEAGUE_SELF_INFO:
        	return player->set_self_league_info(msg_proto);
        case INNER_MAP_LEAGUE_BOSS_ENTER:
        	return player->request_enter_league_boss_begin();
        case INNER_MAP_RES_ENTER_SCENE:
        	return player->request_enter_scene_done(msg_proto);
        case IM_CHECK_USE_GOODS:
        	return player->check_and_use_goods(msg_proto);
        case CLIENT_ESCORT_OPEN_INFO:
        	return player->open_pescort_car_info();
        case CLIENT_ESCORT_UPGRADE:
        	return player->upgrade_escort_level(msg_proto);
        case CLIENT_ESCORT_SELECT_CAR:
        	return player->select_pescort_car_begin(msg_proto);
        case CLIENT_ESCORT_SELECT_CAR_DONE:
        	return player->select_pescort_car_done();
        case CLIENT_ESCORT_PROTECT:
        	return player->protect_escort(msg_proto);
        case CLIENT_ESCORT_WISH:
        	return player->wish_escort();
        case CLIENT_ESCORT_STOP_PROTECT:
        	return player->escort_stop_protect();
        case CLIENT_ESCORT_SEEK_HELP:
        	return player->escort_seek_help();
        case IM_SYNC_LEAGUE_FB_FLAG:
        	return player->sync_league_fb_flag(msg_proto);
        case INNER_MAP_UPDATE_LEAGUER_INFO:
        	return player->update_leaguer_info(msg_proto);

        //flower glamour
        case INNER_UPDATE_GLAMOUR_INFO:
        	return player->update_glamour(msg_proto);

        // league war
        case CLIENT_ENTER_LEAGUE_WAR:
        	return player->request_join_league_war();
        case CLIENT_FETCH_LEAGUE_WAR_INFO:
        	return player->request_league_war_info();
        case CLIENT_CHANGE_SPACE:
        	return player->request_change_space(msg_proto);
        case CLIENT_FETCH_ALL_LWAR_SCORE:
        	return player->request_league_war_score();

        // skill
        case CLIENT_FETCH_SKILL_LIST:
        	return player->fetch_skill_list(msg_proto);
        case CLIENT_FETCH_SKILL_SCHEME:
        	return player->fetch_skill_scheme(msg_proto);
        case CLIENT_FETCH_ALL_SKILL_SCHEME:
        	return player->fetch_all_skill_scheme();
        case CLIENT_UPDATE_SKILL_SCHEME:
        	return player->update_skill_scheme(msg_proto);
        case CLIENT_SET_CUR_RAMA:
        	return player->set_current_rama(msg_proto);
        case CLIENT_SKILL_SHOTCUT:
        	return player->fetch_skill_shortcut();
        case CLIENT_EXCHANGE_SKILL_SHOTCUT:
        	return player->exchange_skill_shortcut(msg_proto);
        case CLIENT_SKILL_LEVEL_UP:
            return player->request_skill_level_up(msg_proto);

        case INNER_MAP_SYNC_LABEL:
        	return player->refresh_player_cur_label(msg_proto);
		case INNER_MAP_SYNC_VIP_INFO:
			return player->sync_vip_info(msg_proto);
		case ACTIVE_NOTIFY_FASHION_ADD_PROP:
			return player->notify_update_fashion_add_prop(msg_proto);

		case INNER_MAP_SYNC_PERMISSION_INFO:
			return player->sync_permission_info(msg_proto);
			
		case CLIENT_REQUEST_ROLE_PANEL_INFO: // 人物属性面板
			return player->fetch_role_panel_info(msg_proto);

		case RETURN_FETCH_SINGLE_PLAYER_DETAIL:
			return player->fetch_single_player_all_detail(msg_proto);
		case RETURN_REQUEST_FETCH_RANKER_DETAIL:
			return player->fetch_ranker_detail(msg_proto);
        case INNER_MAP_FETCH_SELF_RANKER_DATA:
            return player->process_fetch_self_ranker_info(msg_proto);
	
        // script
        case CLIENT_REQUEST_ENTER_SCRIPT:
            return player->request_enter_script(msg_proto);
        case INNER_MAP_ENTER_SCRIPT:
        	return player->sync_enter_script(msg_proto);
        case INNER_SEND_SCRIPT_RESET:
        	return player->logic_reset_script();
        case CLIENT_SCRIPT_DETAIL_PROGRESS:
            return player->request_script_detail_progress();
        case CLIENT_REQUEST_EXIT_SYSTEM:
        	return player->request_exit_cur_system();
        case CLIENT_SPECIAL_SCRIPT_EXIT:
        	return player->request_exit_special();
        case CLIENT_REQUEST_EXIT_ANSWER_ACTIVITY:
        	return player->request_exit_cur_system();
        case CLIENT_GET_PLAYER_WAIT_TIME:
        	return player->request_player_wait_time();
        case CLIENT_FETCH_HOTSPRING_INFO:
        	return player->request_Hotspring_info();
        case CLIENT_HOTSPRING_DOUBLE_MAJOR:
        	return player->request_double_major(msg_proto);
        case CLIENT_HOTSPRING_GUESS:
        	return player->request_player_guess(msg_proto);
        case CLIENT_HOTSPRING_NEAR_PLAYER:
            return player->request_hotspring_near_player(msg_proto);
        case CLIENT_SCRIPT_STOP_SCRIPT:
        	return player->request_stop_script(msg_proto);
        case CLIENT_SCRIPT_RUN_SCRIPT:
        	return player->request_run_script(msg_proto);
        case INNER_GET_SCRIPT_CLEAN_TIMES:
        	return player->inner_fetch_script_clean_times(msg_proto);
        case CLIENT_EXTRACT_SCRIPT_CARD:
            return player->request_extract_script_card(msg_proto);
        case CLIENT_SCRIPT_PIECE_DETAIL:
            return player->request_script_piece_detail();
        case INNER_FETCH_TEAM_SCRIPT_USE_TIMES:
            return player->sync_team_script_use_times(msg_proto);
        case CLIENT_SCRIPT_PLAYER_INFO:
            return player->request_script_player_info(msg_proto);
        case CLIENT_SCRIPT_FIRST_START:
        	return player->request_first_start_script(msg_proto);
        case INNER_ML_CLIMB_TOWER_TASK_CHECK:
        	return player->set_script_wave_task(msg_proto);
        case CLIENT_SCRIPT_LIST_INFO:
            return player->request_script_list_info(msg_proto);
        case CLIENT_SCRIPT_SPECIAL_AWARD:
        	return player->request_fetch_special_award(msg_proto);
        case CLIENT_SCRIPT_ADD_TIMES_GOLD:
            return player->request_script_add_times_gold(msg_proto);
        case CLIENT_SCRIPT_ADD_TIMES:
            return player->request_script_add_times(msg_proto);
        case CLIENT_SCRIPT_TYPE_RESET:
        	return player->request_script_type_reset_begin(msg_proto);
        case INNER_SCRIPT_ADD_TIMES_USE_GOLD:
//            return player->process_add_script_after_use_gold(msg_proto);
        	return player->process_after_reset_script(msg_proto);
        case INNER_SCRIPT_LIST_INFO:
            return player->process_script_list_info(msg_proto);
        case INNER_RETURN_UPDATE_COUPLE_FB:
        	return player->login_add_couple_fb_times(msg_proto);
//        case CLIENT_SCRIPT_PIECE_TOTAL_STAR:
//            return player->request_piece_total_star(msg_proto);
//        case CLIENT_SCRIPT_PIECE_TOTAL_STAR_DRAW:
//            return player->request_piece_total_star_award(msg_proto);
        case INNER_SCRIPT_DRAW_PIECE_TOTAL_STAR:
            return player->process_piece_total_star_after_award(msg_proto);
//        case INNER_ML_CLIMB_TOWER_NOVICE_INFO:
//        	return player->process_fetch_climb_tower_info(msg_proto);
        case INNER_SCRIPT_COMPACT_INFO:
            return player->read_script_compact_info(msg_proto);
        case RETURN_WEDDING_PANNEL:
            return player->process_fetch_couple_script_times(msg_proto);
        case CLIENT_REMOVE_STONE_STATE:
            return player->request_remove_stone_state(msg_proto);
        case CLIENT_COUPLE_FB_SELECT_KEY:
        	return player->couple_fb_select_key(msg_proto);
        case CLIENT_SELECT_SWORD_SKILL:
        	return player->sword_top_select_skill(msg_proto);
        case CLIENT_LEGEND_TOP_RANK:
        	return player->fetch_legend_top_rank(msg_proto);
        case CLIENT_SWORD_TOP_RANK:
        	return player->fetch_sword_top_rank(msg_proto);
        case INNER_UPDATE_SCRIPT_RED_POINT:
        	return player->script_red_point_check_uplvl();
        case INNER_TEST_RESET_LFB:
        	return player->test_reset_lfb_script();

        // world boss
        case CLIENT_ENTER_WORLD_BOSS:
        	return player->request_join_wboss(msg_proto);
        case INNER_ENTER_WORLD_USE_FLY:
        	return player->request_join_wboss_done(msg_proto);
        case CLIENT_GET_WORLD_BOSS_INFO:
        	return player->request_wboss_info();
        case CLIENT_GET_POCKET_AWARD:
        	return player->request_get_wboss_pocket_award(msg_proto);
        case CLIENT_CREATE_DICE_NUM:
        	return player->request_create_dice_num(msg_proto);
        case CLIENT_MY_RANK_INFO:
        	return player->request_my_rank_info(msg_proto);
        case CLIENT_FETCH_RED_POINT:
        	return player->request_wboss_red_point();
        case CLIENT_GET_WORLD_BOSS_SCENE_INFO:
        	return player->request_wboss_scene_info();

        //trvl world boss
        case CLIENT_GET_TRVL_WBOSS_INFO:
        	return player->request_trvl_wboss_info();
        case CLIENT_ENTER_TRVL_WBOSS_PRE:
        	return player->request_join_trvl_pre_scene();
        case CLIENT_ENTER_TRVL_WBOSS:
        	return player->request_join_trvl_wboss(msg_proto);
        case CLIENT_LEAVE_TRVL_WBOSS:
        	return player->request_exit_trvl_wboss();

        //trvl peak
        case CLIENT_TRVL_PEAK_ENTER:
        	return player->request_enter_trvl_peak_scene();
        case CLIENT_UNMATCH_TRVL_PEAK_QUALITY:
        	return player->request_unmatch_trvl_peak_quality();
        case CLIENT_MATCH_TRVL_PEAK_QUALITY:
        	return player->request_match_trvl_peak_quality();
        case CLIENT_FETCH_TRVL_PEAK_SCENE_INFO:
        	return player->request_trvl_peak_scene_info();
        case CLIENT_FETCH_TRVL_PEAK_RANK_INFO:
        	return player->request_trvl_peak_rank_info(msg_proto);
        case INNER_SYNC_FETCH_OFFLINE_HOOK:
        	return player->sync_offline_hook_to_travel_scene(msg_proto);

        //	escort
        case INNER_ESCORT_ACTIVITY_INFO:
        	return player->request_get_activity_status(msg_proto);

        // monster attack
        case CLIENT_MATTACK_REQUEST_ENTER:
        	return player->request_enter_player();
        case CLIENT_FIGHTING_WILL_INFO:
        	return player->request_get_fighting_will();

        case INNER_MAP_TEAM_SYNC_TEAM_INFO:
        	return player->read_team_info(msg_proto);
        case INNER_MAP_TEAM_UPDATE_BLOOD_INFO:
        	return player->update_team_blood_info();
        case INNER_MAP_TEAM_REQ_ENTER:
        	return player->req_team_enter_script(msg_proto);
        case INNER_MAP_TEAM_REQ_READY:
        	return player->req_team_get_ready(msg_proto);
        case INNER_MAP_TEAM_NEAR_INFO:
        	return player->fetch_teamer_near_info(msg_proto);
        case CLIENT_OPEN_MINI_MAP:
            return player->request_open_mini_map_pannel();
        case CLIENT_CLOSE_MINI_MAP:
            return player->request_close_mini_map_pannel();

        // 杀戮战场
        case CLIENT_JOIN_SM_BATTLE:
        	return player->request_join_sm_battle();
        case CLIENT_SCAN_SM_BATTLE_RANK:
        	return player->scan_sm_battle_rank(msg_proto);
        case CLIENT_REQUEST_INIT_SM_INFO:
        	return player->request_init_sm_battle_info();

        // 跨服副本
        case CLIENT_TRVL_SCRIPT_CREATE_TEAM:
        	return player->create_trvl_script_team(msg_proto);
        case CLIENT_TRVL_SCRIPT_FETCH_LIST:
        	return player->fetch_trvl_script_team_list(msg_proto);
        case CLIENT_TRVL_SCRIPT_QUIT_TEAM:
        	return player->quit_trvl_script_team(msg_proto);
        case CLIENT_TRVL_SCRIPT_ADD_TEAM:
        	return player->add_trvl_script_team(msg_proto);
        case CLIENT_TRVL_SCRIPT_TEAM_INFO:
        	return player->fetch_trvl_script_team_info(msg_proto);
        case CLIENT_TRVL_SCRIPT_START:
        	return player->start_trvl_script_team(msg_proto);
        case CLIENT_TRVL_SCRIPT_PREV_OPER:
        	return player->prep_trvl_script_team(msg_proto);
        case CLIENT_TRVL_SCRIPT_KICK:
        	return player->kick_trvl_script_team(msg_proto);
        case CLIENT_TRVL_ARENA_INFO:
        	return player->fetch_travel_arena_info();
        case CLIENT_TRVL_ARENA_ENTER:
        	return player->request_enter_travel_arena();
        case CLIENT_TRVL_ARENA_RANK:
        	return player->travel_arena_rank(msg_proto);
        case CLIENT_TRVL_ARENA_DRAW:
        	return player->travel_arena_draw();
        case CLIENT_TRVL_ARENA_SIGN:
        	return player->travel_arena_sign();
        case CLIENT_TRVL_ARENA_UNSIGN:
        	return player->travel_arena_unsign();
        case CLEINT_TRVL_MARENA_ENTER:
        	return player->request_enter_travel_marena();
        case CLIENT_TRVL_MARENA_SIGN:
        	return player->travel_marena_sign();
        case CLIENT_TRVL_MARENA_UNGISN:
        	return player->travel_marena_unsign();
        case CLIENT_TRVL_MAREAN_INFO:
        	return player->travel_marena_info();
        case CLIENT_TRVL_MARENA_RNAK:
        	return player->travel_marena_rank_begin();

        case CLIENT_TRVL_ARENA_WIN_DRAW:
        	return player->travel_arena_draw_win(msg_proto);
        case CLIENT_FETCH_TRANSALTE:
             return player->fetch_enemy_translate(msg_proto);
        case INNER_FETCH_ENEMY_POSITION:
             return player->fetch_enemy_position(msg_proto);
        case INNER_FETCH_NEARBY_PLAYER:
             return player->fetch_nearby_player(msg_proto);

        case CLIENT_TRVL_BATTLE_ENTER:
             return player->request_enter_travel_battle();
        case CLIENT_TRVL_BATTLE_LAST_RANK:
             return player->request_tbattle_last_rank_list(msg_proto);
        case CLIENT_TRVL_BATTLE_HIS_TOP:
             return player->request_tbattle_history_top_list(msg_proto);
        case CLIENT_TRVL_BATTLE_VIEW_PLAYER:
             return player->request_tbattle_view_player_info(msg_proto);
        case CLIENT_TRVL_BATTLE_CUR_RANK:
             return player->request_tbattle_cur_rank_list(msg_proto);
        case CLIENT_TRVL_BATTLE_INDIANA_LIST:
             return player->request_tbattle_indiana_list(msg_proto);

		//=============wedding==================
		case INNER_WEDDING_ROLE_COORD:
			return player->process_get_wedding_role_coord(msg_proto);
		case INNER_SYNC_MAP_WEDDING_INFO:
			return player->process_sync_wedding_info(msg_proto);
        case ACTIVE_SEND_FLOWER:
            return player->notify_send_flower_effect(msg_proto);

        //=============trade===================
        case   INNER_REQUEST_IS_LIMIT_TO_MAP:
        	return player->handle_inner_trade_distance(msg_proto);
        case INNER_NOTIFY_UPDATE_TRADE_STATE:
        	return player->notify_trade_to_mapplayer(msg_proto);
        case INNER_TRADE_FETCH_ON_LINE_STATE:
        	return player->trade_get_player_online_state(msg_proto);
        case IM_ROLE_PASSIVE_SKILL_PA_EVENT:
            return player->check_passive_skill_pa_event(msg_proto);

		case INNER_SYNC_MAGICW_LVLUP_INFO:
        	return player->refresh_player_magic_weapon_skill(msg_proto);
		case INNER_SYNC_MAGICW_RANK_INFO:
			return player->refresh_player_magic_weapon_shape(msg_proto);
		case INNER_NOTIFY_GETIN_WILDQIXI:
			return player->enter_rand_scene(msg_proto);
		case INNER_CHAT_FLAUNT:
        	return player->process_flaunt_info(msg_proto);

        //领地战
		case CLIENT_GET_LRF_INFO:
			return player->get_last_region_info(msg_proto);
		case CLIENT_GET_LRF_BATSCENEINFO:
			return player->get_region_bet_info(msg_proto);
		case CLIENT_GET_LRF_BATSUPPORT:
			return player->get_bet_support_apply(msg_proto);
		case CLIENT_REQUEST_ENTER_WAR:
			return player->player_request_join_war(msg_proto);
		case CLIENT_LRF_CHANGE_MODE:
			return player->player_request_change_mode(msg_proto);
		case INNER_LRF_BUY_HICKTY:
			return player->lrf_request_change_mode_done(msg_proto);
		case CLIENT_LRF_WAR_RANK:
			return player->fetch_lrf_war_rank();

        default:
            MSG_USER("ERROR can't recongize map message %d %d %d", sid, recogn, len);

            if (transaction != NULL)
            {
            	transaction->rollback();
            	transaction = NULL;
            }
            return -1;
    }

    return 0;
}

int MapUnit::process_stop(void)
{
    MAP_MONITOR->logout_map_all_player();
    return BaseUnit::process_stop();
}

int MapUnit::process_wedding_cruise_begin(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30101608 *, request, msg, -1);

    int monster_sort = request->monster_sort(), scene_id = request->scene_id();

    Scene *scene = NULL;
    JUDGE_RETURN(MAP_MONITOR->find_scene(0, scene_id, scene) == 0, -1);

    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    const Json::Value &npc_loc_json = wedding_json["npc_locate"];
	int npc_sort = npc_loc_json[1u].asInt();
	const Json::Value &npc_json = CONFIG_INSTANCE->npc(scene_id, npc_sort);
	MoverCoord coord;
    if (npc_json.isMember("pos"))
    {
    	coord.set_pixel(npc_json["pos"][0u].asInt(), npc_json["pos"][1u].asInt());
    }
    else
    {
    	coord.set_pixel(npc_json["posX"].asInt(), npc_json["posY"].asInt());
    }

    Int64 ai_id = AIMANAGER->generate_float_ai(monster_sort, coord, scene);

    FloatAI *float_ai = AIMANAGER->float_ai_package()->find_object(ai_id);
    JUDGE_RETURN(float_ai != NULL, -1);

    float_ai->set_wedding_id(request->wedding_id());
    float_ai->set_wedding_type(request->wedding_type());
    float_ai->set_partner_id_1(request->partner_id_1());
    float_ai->set_partner_career_1(request->partner_career_1());
    float_ai->set_partner_career_2(request->partner_career_2());
    float_ai->set_partner_id_2(request->partner_id_2());
    float_ai->set_float_type();
    float_ai->enter_scene();

    MSG_USER("float ai enter %ld %d %ld %ld %d %ld %d(%d,%d) from(%d,%d)", float_ai->wedding_id(),
            float_ai->wedding_type(), float_ai->partner_id_1(), float_ai->partner_id_2(),
            float_ai->ai_sort(), float_ai->ai_id(), float_ai->scene_id(),
            float_ai->location().pixel_x(), float_ai->location().pixel_y(),
            coord.pixel_x(), coord.pixel_y());

    return 0;
}
