/*
 * LogicUnit.cpp
 *
 * Created on: 2013-01-08 11:34
 *     Author: glendy
 */

#include "BackGameModifySys.h"
#include "LogicUnit.h"
#include "ProtoDefine.h"
#include "LogicPlayer.h"
#include "Transaction.h"
#include "LogicMonitor.h"
#include "PoolMonitor.h"
#include "TransactionMonitor.h"

#include "ArenaSys.h"
#include "SendActReward.h"
#include "LogicPhp.h"
#include "TeamPlatform.h"
#include "DaemonServer.h"
#include "MarketSystem.h"

#include "ShopMonitor.h"
#include "RankSystem.h"
#include "LeagueSystem.h"
#include "FestActivitySys.h"
#include "ActivityTipsSystem.h"
#include "BackstageBrocastControl.h"
#include "BackstageMailSystem.h"
#include "OpenActivitySys.h"
#include "ShopMonitor.h"
#include "RestrictionSystem.h"
#include "LogicGameSwitcherSys.h"
#include "BackActivityTick.h"
#include "WeddingMonitor.h"
#include "InvestRechargeSys.h"
#include "LuckyWheelSys.h"
#include "DailyActSys.h"
#include "JYBackActivitySys.h"
#include "TrvlTeamSystem.h"

int LogicUnit::type(void)
{
    return LOGIC_UNIT;
}

UnitMessage *LogicUnit::pop_unit_message(void)
{
    return LOGIC_MONITOR->unit_msg_pool()->pop();
}

int LogicUnit::push_unit_message(UnitMessage *msg)
{
    return LOGIC_MONITOR->unit_msg_pool()->push(msg);
}

int LogicUnit::process_block(UnitMessage *unit_msg)
{
    int sid = unit_msg->__sid, recogn = unit_msg->__msg_head.__recogn,
            trans_id = unit_msg->__msg_head.__trans_id;
    int64_t role_id = unit_msg->__msg_head.__role_id;

    Message *msg_proto = unit_msg->proto_msg();
    int ivalue = unit_msg->ivalue();

    Transaction *transaction = 0;
    if (trans_id > 0)
    {
    	TRANSACTION_MONITOR->find_transaction(trans_id, transaction);
    }

    if (recogn / 100000 == 308 || recogn / 100000 == 318)
    {
        return this->process_php_request(sid, recogn, unit_msg);
    }

    // 不需要经过玩家的操作
    switch (recogn)
    {
        case INNER_TIMER_TIMEOUT:
            return POOL_MONITOR->game_timer_timeout(ivalue);
        case INNER_LOGIC_SYNC_LEAGUE_BOSS:
        	return LEAGUE_SYSTEM->load_league_boss_scene();
        case INNER_MAP_LEAGUE_CREATE_BOSS:
        	return LEAGUE_SYSTEM->check_boss_scene_start(msg_proto);
        case IL_LEAGUE_FB_FINISH:
        	return LEAGUE_SYSTEM->handle_league_fb_finish(msg_proto);
        case IL_LEAGUE_BOSS_FINISH:
        	return LEAGUE_SYSTEM->handle_league_boss_finish(msg_proto);
        case INNER_LOGIC_NOTIFY_ALL_PLAYER:
        	return LOGIC_MONITOR->notify_all_player(msg_proto);
        case INNER_SYNIC_LEAGUE_RENAME:
        	return LEAGUE_SYSTEM->handle_rename_league_finish(msg_proto);
        case INNER_SEND_LWAR_FIRST_WIN:
        	return LEAGUE_SYSTEM->inner_send_lwar_first_win(msg_proto);

        case TRANS_LOAD_LOGIC_PLAYER:
            return LOGIC_MONITOR->after_load_player(transaction);
        case TRANS_LOAD_GATE_PLAYER_PHP:
        	return LOGIC_PHP->after_load_role_info(transaction);
        case INNER_UPDATE_CONFIG:
        {
            DAEMON_SERVER->request_update_config();
            ARENA_SYS->check_area_need_sort();
            return 0;
        }
        case TRANS_LOAD_SHOP_MODE:
        	return LOGIC_MONITOR->db_load_mode_done(transaction);

        	//rank system : load data from db
        case TRANS_LOAD_FIGHT_FORCE_RANK_DATA:
        case TRANS_LOAD_KILL_VALUE_RANK_DATA:
        case TRANS_LOAD_HERO_RANK_DATA:
        case TRANS_LOAD_FIGHT_LEVEL_RANK_DATA:
        case TRANS_LOAD_PET_RANK_DATA:
        case TRANS_LOAD_MOUNT_RANK_DATA:
        case TRANS_LOAD_SCRIPT_ZYFM_RANK_DATA:
        case TRANS_SAVE_PLAYER_RANK_WING:
        case TRANS_LOAD_SEND_FLOWER_RANK:
        case TRANS_LOAD_RECV_FLOWER_RANK:
        case TRANS_LOAD_ACT_SEND_FLOWER_RANK:
        case TRANS_LOAD_ACT_RECV_FLOWER_RANK:
        case TRANS_LOAD_RANK_FUN_INFO:
        case TRANS_LOAD_RANK_MOUNT_INFO:
//        case TRANS_LOAD_RANK_FUN_MOUNT_INFO:
//        case TRANS_LOAD_RANK_FUN_GOD_SOLIDER_INFO:
//        case TRANS_LOAD_RANK_FUN_MAGIC_EQUIP_INFO:
//        case TRANS_LOAD_RANK_FUN_XIAN_WING_INFO:
//        case TRANS_LOAD_RANK_FUN_LING_BEAST_INFO:
//        case TRANS_LOAD_RANK_FUN_BEAST_EQUIP_INFO:
//        case TRANS_LOAD_RANK_FUN_BEAST_MOUNT_INFO:
        	return RANK_SYS->after_db_refresh_rank_data(transaction);
        	//rank system : load hide special role_id from db
        case TRANS_RANK_HIDE_ROLE:
        	return RANK_SYS->after_get_hide_player(transaction);
        case INNER_LOGIC_ACT_FLOWER_RANK:
        	return RANK_SYS->update_act_flower_rank(msg_proto);
        case TRANS_RESET_ACT_FLOWER_RANK:
        	return RANK_SYS->reset_act_flower_rank(transaction);
        	//backstage brocast
        case TRANS_CHECK_LOAD_BACK_BRO_CONTROL:
        	return BBC_INSTANCE->after_load_data_from_db(transaction);
        case INNER_MAP_MARKET_BUY:
        	return LogicMarketer::map_market_buy_goods(role_id, msg_proto);

        	//backstage mail
        case TRANS_LOAD_BACK_MAIL_REQUEST:
        	return BACK_MAIL_SYS->after_load_back_mail_request(transaction);

        case IL_LEAGUE_ESCORT_FINISH_REWARD:
        	return LEAGUE_SYSTEM->send_league_escort_reward(msg_proto);
        case INNER_LOGIC_ADD_LEAGUE_FLAG_EXP:
        	return LEAGUE_SYSTEM->add_league_flag_exp(msg_proto);
        case INNER_LOGIC_ADD_MARTIAL_LABEL:
        	return LEAGUE_SYSTEM->update_martial_role_label(msg_proto);
        case INNER_LOGIC_ADD_CONTRI:
        	return LEAGUE_SYSTEM->add_league_member_contri(msg_proto);
        case INNER_LOGIC_LEGION_RESULT:
        	return LEAGUE_SYSTEM->set_region_result(msg_proto);
        case IL_NOTIGY_ESCORT_STATE:
        	return LEAGUE_SYSTEM->notify_lescort_state(msg_proto);
        case IL_REQUEST_RANK_INFO:
        	return RANK_SYS->map_fetch_rank_info(unit_msg->__msg_head, msg_proto);
        case INNER_LOGIC_REP_ID_INFO:
        	return TEAM_PLANTFORM->update_rep_id_info(msg_proto);
        case IL_ARENA_FIGHT_FINISH:
        	return ARENA_SYS->finish_challenge(msg_proto);
        case IL_ARENA_UPDATE_SHAPE:
        	return ARENA_SYS->update_arena_role_shape(role_id, msg_proto);
        case INNER_LOGIC_RANK_LEVEL_FIGHT:
        	return RANK_SYS->map_act_fetch_level_fight_rank_info(msg_proto);
        case INNER_LOGIC_RANK_LEAGUE:
        	return LEAGUE_SYSTEM->map_act_fetch_league_rank_info(msg_proto);

        case INNER_LOGIC_SET_OFFLINE_INFO:
        	return RANK_SYS->update_player_offline(msg_proto);
        case INNER_LOGIC_SCENE_SYNC_ACTIVITY:
        	return ACTIVITY_TIPS_SYSTEM->scene_sync_activity_state(msg_proto);
        case INNER_LOGIC_JOIN_ACTIVITY_SINGLE:
        	return ACTIVITY_TIPS_SYSTEM->sync_single_player_join_activity(msg_proto);
        case INNER_LOGIC_JOIN_ACTIVITY_BATCH:
			return ACTIVITY_TIPS_SYSTEM->sync_batch_player_join_activity(msg_proto);
        case INNER_LOGIC_REQ_SYNC_ONSALE_ITEM:
        	return SHOP_MONITOR->respond_sync_shop_item(msg_proto);

        case INNER_LOGIC_SYNC_TRVL_ANNOUNCE:
        	return ACTIVITY_TIPS_SYSTEM->trvl_sync_announce(msg_proto);
        case INNER_ML_UPDATE_SCRIPT_RANK:
        	return RANK_SYS->update_rank_data(msg_proto);
        case IL_TMARENA_FINISH_MAIL:
        	return LOGIC_MONITOR->send_tmarena_rank_reward(msg_proto);

        case INNER_REQ_LOGIC_SEND_MSG_TO_ALL:
        	return LOGIC_MONITOR->parse_msg_to_all_player(msg_proto);
        case INNER_REQ_LOGIC_SEND_MSG_TO_ONE:
        	return LOGIC_MONITOR->parse_msg_to_one_player(msg_proto);
        case INNER_LOGIC_NOTIFY_PLAYER_FROM_MAP:
        	return LOGIC_MONITOR->notify_player_from_map(msg_proto);
        case TRANS_LOAD_RESTRICTION_INFO:
        	return RESTRI_SYSTEM->respond_update_restirction_info(transaction);
        case INNER_MAP_REQ_SYNC_GAME_SWITCHER_INFO:
        	return LOGIC_SWITCHER_SYS->push_detail_to_ml();
        case TRANS_LOAD_GAME_SWITCHER_INFO:
        	return LOGIC_SWITCHER_SYS->after_load_data(transaction);
        case TRANS_LOAD_GAME_MODIFY_INFO:
        	return BACK_GAME_MODIFY_SYS->after_load_data(transaction);
        case INNER_FETCH_SUPER_VIP_INFO:
        	return BACK_GAME_MODIFY_SYS->fetch_super_vip_info(msg_proto);
        case TRANS_LOAD_BACK_ACTIVITY_TICK:
            return BACK_ACTIVITY_TICK->update_activity_tick_in_logic_server(transaction);
        case IL_OPEN_ACTIVITY_LWAR_INFO:
        	return LOGIC_OPEN_ACT_SYS->update_lwar_activity_rank_info(msg_proto);
        case TRANS_LOAD_BACK_ACTIVITY_UPDATE:
        	return BACK_ACTIVITY_SYS->process_after_load_back_act_update(transaction);

        case INNER_MAP_UPDATE_TRVL_WEDDING_REWARD:
        	return LUCKY_WHEEL_SYSTEM->update_couple_rank_info(msg_proto);
        case INNER_MAP_FETCH_WEDDING_ACT_REWARD:
        	return LUCKY_WHEEL_SYSTEM->trvl_wedding_reward_info(msg_proto);
        case INNER_MAP_SEND_RECHARGE_RANK_MAIL:
        	return LUCKY_WHEEL_SYSTEM->trvl_recharge_rank_mail(msg_proto);

        case INNER_UPDATE_DOUBLE_SCRIPT:
        	return DAILY_ACT_SYSTEM->check_total_double_is_update(msg_proto);

        case INNER_MAP_SEND_TRVL_WBOSS_MAIL:
        	return LUCKY_WHEEL_SYSTEM->trvl_wboss_send_mail(msg_proto);

		// wedding
		case INNER_OTHER_UPDATE_INTIMACY:
			return WEDDING_MONITOR->process_other_update_intimacy(msg_proto);
		case INNER_CRUISE_FINISH:
			return WEDDING_MONITOR->process_cruise_finish(msg_proto);
		case INNER_LOGIC_SEND_ACT_REWARD:
			return SEND_ACTREWARD->send_player_act_reward(msg_proto);
		case INNER_LOGIC_FESTIVAL_BOSS_HURT:
			return FEST_ACTIVITY_SYS->handle_scene_boss_info(msg_proto);

		case INNER_SYNC_TRVL_TEAM_SIGNUP_STATE:
			return TRAVEL_TEAM_SYS->update_trvl_team_signup_state(msg_proto);
		case INNER_SYNC_TRAVEL_PEAK_TICK:
			return TRAVEL_TEAM_SYS->process_sync_trvl_peak_activity_tick(msg_proto);

        // 历练奖励
        case INNER_TRVL_BATTLE_PRACTICE:
            return LogicPlayer::process_tbattle_practice_mail(msg_proto);

		//开服活动
		case INNER_ML_DELETE_RETURN_RECHARGE:
		{	//充值返还
			Proto31400263*  request = dynamic_cast<Proto31400263*>(msg_proto);
			JUDGE_RETURN(request != NULL, -1)

			Proto32101105 send;
			send.set_role_id(request->role_id());
			send.set_account(request->account());
			MSG_USER("logic trans request delete return recharge to centerUnit!");
			return LOGIC_MONITOR->process_inner_center_thread(send);
		}
        default:
            break;
    }

    // role operate ;
    LogicPlayer *player = 0;
    if (LOGIC_MONITOR->find_player(role_id, player) != 0)
    {
        if (recogn != INNER_LOGIC_LOGIN)
        {
            MSG_USER("ERROR client no role login %d %ld %d", sid, role_id, recogn);
            if (transaction != NULL)
            {
            	transaction->rollback();
            	transaction = NULL;
            }
            return -1;
        }
        return LOGIC_MONITOR->request_login_player(sid, role_id, dynamic_cast<Proto30100101 *>(msg_proto));
    }
    else
    {
        if (recogn == INNER_LOGIC_LOGIN)
        {
            return player->resign(sid, dynamic_cast<Proto30100101 *>(msg_proto));
        }
    }

    switch (recogn)
    {
		case INNER_LOGIC_MAP_ENTER_SCENE:
			return player->map_enter_scene(msg_proto);
    	case INNER_LOGIC_MAP_OBTAIN_AREA:
    		return player->map_obtain_area(msg_proto);
    	case INNER_ML_ADD_GOODS:
    		return player->map_add_goods_result(msg_proto);
        case INNER_LOGIC_OWN_ITEM:
        	return player->after_request_mall_info(msg_proto);
        case CLIENT_MALL_FETCH_INFO:
        	return player->request_mall_info(msg_proto);
        case CLIENT_MALL_DONATE_GOODS:
        	return player->mall_donate_goods_begin(msg_proto);
        case INNER_MAP_MALL_DONATE:
        	return player->mall_donate_goods_done(msg_proto);
        case CLIENT_MALL_BUY_GOODS:
        	return player->mall_buy_goods_begin(msg_proto);
        case CLIENT_FETCH_MALL_ITEM_INFO:
        	return player->fetch_mall_good_detail(msg_proto);
        case INNER_MAP_MALL_BUY:
        	return player->mall_buy_goods_done(msg_proto);
        case INNER_ML_LEAGUE_RENAME:
        	return player->request_rename_league(msg_proto);
        case INNER_LSTORE_INSERT_TO_LOGIC:
        	return player->insert_store_after(msg_proto);
        case INNER_LSTORE_GET_TO_LOGIC:
        	return player->apply_item_result(msg_proto);
        case INNER_RETURN_ACTIVITY_COST_ITEM:
        	return player->request_activity_buy_done(msg_proto);
        case INNER_MAY_ACT_BUY:
        	return player->inner_act_buy_operate(msg_proto);
        case INNER_ACT_BUY_ITEM:
        	return player->fetch_change_buy_end(msg_proto);
        case INNER_GEM_SYNTHESIS_PACK_OP:
        	return player->inner_gem_synthesis_pack_operate(msg_proto);
        case INNER_FETCH_ITEM_AMOUNT_INFO:
        	return player->goddess_bless_exchange_end(msg_proto);
        case INNER_GODDESS_BLESS_REWARD:
        	return player->goddess_bless_operator_end(msg_proto);
        case INNER_ML_LUCKY_WHEEL_COST:
        	return player->draw_award_done(msg_proto);
        case INNER_ML_LUCKY_WHEEL_RESET:
        	return player->request_activity_reset_done(msg_proto);
        case INNER_LOGIC_CONSUME_GOLD:
        	return player->map_consume_gold(msg_proto);
        case CLIENT_OTHER_MASTER_INFO:
        	return player->fetch_other_player_info(msg_proto, RETURN_OTHER_MASTER_INFO);
        case INNER_ML_LOOKUP_OTHER_INFO:
        	return player->respond_other_player_info(msg_proto);
        case CLIENT_FETCH_NOTICE_INFO:
        	return player->fetch_game_notice_info();
        case CLIENT_DRAW_NOTICE_REWARD:
        	return player->draw_game_notice_reward_begin();
        case INNER_ML_NOTICE_REWARD:
        	return player->draw_game_notice_reward_done(msg_proto);
        case ACTIVE_POPUP_INFO:
        	return player->notify_client_popup_info(msg_proto);
        case INNER_SYNC_LEAGUE_FB_WAVE:
        	return player->update_lfb_wave(msg_proto);

        	// team
        case INNER_ML_TEAM_TASK_MONSTER_INFO:
        	return player->team_monster_info_update(msg_proto);

        case CLIENT_TEAM_OPEN_TEAM_PANEL:
        	return player->logic_fetch_team_info();
        case CLIENT_TEAM_CREATE_TEAM:
        	return player->logic_create_team();
        case CLIENT_TEAM_APPOINT_LEADER:
        	return player->logic_appoint_leader(msg_proto);
        case CLIENT_TEAM_KICK_TEAMER:
        	return player->logic_kick_teamer(msg_proto);
        case CLIENT_TEAM_QUIT_TEAM:
        	return player->logic_quit_team();
        case CLIENT_TEAM_ORG_TEAM:
        	return player->logic_request_org_team(msg_proto);
        case CLIENT_TEAM_ACCEPT_INVITE:
        	return player->accept_invite_into_team(msg_proto);
        case CLIENT_TEAM_ACCPET_APPLY:
        	return player->accept_apply_into_team(msg_proto);
        case CLIENT_TEAM_CLOSE_TEAM_PANEL:
        	return player->logic_close_team_info();

        case CLIENT_TEAM_FAST_JOIN:
        	return player->logic_fast_join_team(msg_proto);
        case CLIENT_TEAM_NEAR_TEAM_INFO:
        	return player->logic_near_team_begin(msg_proto);
        case CLIENT_TEAM_NEAR_PLAYER:
        	return player->logic_near_player(msg_proto);
        case CLIENT_TEAM_AUTO_INVITE:
        	return player->logic_auto_invite(msg_proto);
        case CLIENT_TEAM_AUTO_ACCEPT:
        	return player->logic_auto_accept(msg_proto);

        // fb team
        case CLIENT_TEAM_FB_ORGANIZE:
        	return player->team_fb_organize(msg_proto);
        case CLIENT_TEAM_FB_BROADCAST_RECRUIT:
        	return player->team_fb_broadcast_recruit();
        case CLIENT_TEAM_FB_FRIEND_LIST:
        	return player->team_fb_friend_list();
        case CLIENT_TEAM_FB_LEAGUER_LIST:
        	return player->team_fb_leaguer_list();
        case CLIENT_TEAM_FB_INVITE_INTOTEAM:
        	return player->team_fb_invite_into_team(msg_proto);
        case CLIENT_TEAM_FB_ENTER_COUPLE_FB:
        	return player->team_fb_request_enter_couple_fb();
        case CLIENT_TEAM_FB_ENTER_COUPLE_FB_RESPOND:
        	return player->team_fb_respond_enter_couple_fb(msg_proto);
        case CLIENT_TEAM_FB_REQUEST_GET_TIMES:
        	return player->request_couple_fb_send_times();
        case CLIENT_TEAM_FB_APPLY_INTO_TEAM:
        	return player->team_fb_apply_into_team(msg_proto);
        case CLIENT_TEAM_FB_RESPOND_INVITE:
        	return player->team_fb_respond_invite(msg_proto);
        case CLIENT_TEAM_FB_RESPOND_APPLY:
        	return player->team_fb_respond_apply(msg_proto);
        case CLIENT_TEAM_FB_GET_READY:
        	return player->team_fb_get_ready(msg_proto);
        case CLIENT_TEAM_FB_CONFIRM_SWITCH:
        	return player->team_fb_respond_team_switch(msg_proto);
        case CLIENT_TEAM_FB_CHANGE_FB:
        	return player->team_fb_change_fb(msg_proto);
        case CLIENT_TEAM_FB_ENTER_FB:
        	return player->team_fb_enter_fb(msg_proto);
        case CLIENT_TEAM_FB_DISMISS_TEAM:
        	return player->team_fb_dismiss_team();
        case CLIENT_TEAM_FB_REPLACEMENT_LIST:
        	return player->begin_get_rpm_recomand_info(msg_proto);
        case CLIENT_TEAM_FB_REPLACEMENT_RECRUIT:
        	return player->team_fb_recruit_replacement(msg_proto);
        case INNER_LOGIC_TEAM_SCRIPT_USE_TIMES:
        	return player->finish_sync_fb_use_times(msg_proto);
        case INNER_LOGIC_TEAM_LEAVE_SCRIPT:
        	return player->team_fb_leave_fb(msg_proto);
        case INNER_LOGIC_TEAM_VALIDATE:
        	return player->validate_map_team_info(msg_proto);
        case INNER_LOGIC_NEAR_TEAM_INFO:
        	return player->logic_near_team_done(msg_proto);
        case CLIENT_TEAM_FB_ORG_CANCEL:
        	return player->team_fb_organize_cancel();
        case INNER_LOGIC_TEAM_SYNC_ENTER_RESULT:
        	return player->sync_player_enter_result(msg_proto);

        case INNER_MAP_TEAM_REQ_ENTER:
        	return player->team_fb_enter_fb_from_scene(msg_proto);
        case INNER_MAP_TEAM_REQ_READY:
        	return player->team_fb_get_ready_from_scene(msg_proto);

        //league
        case CLIENT_LEAGUE_LIST_INFO:
        	return player->fetch_league_list_info(msg_proto);
        case CLIENT_LEAGUE_CREATE:
        	return player->create_league(msg_proto);
        case CLIENT_LEAGUE_IS_SAME:
        	return player->request_check_same_league(msg_proto);
        case CLIENT_LEAGUE_APPLY_JOIN:
        	return player->apply_join_league(msg_proto);
        case CLIENT_LEAGUE_CANCEL_APPLY:
        	return player->cancel_join_league(msg_proto);
        case CLIENT_LEAGUE_INFO:
        	return player->fetch_league_info();
        case CLIENT_REQUEST_LEADER_IMPEACH:
        	return player->fetch_leader_inpeach();
        case CLIENT_OTHER_LEAGUE_INFO:
        	return player->fetch_other_league_info(msg_proto);
        case CLIENT_LEAGUE_QUIT:
        	return player->quit_league();
        case IM_QUIT_LEAGUE:
        	return player->map_quit_league(msg_proto);
        case CLIENT_LEAGUE_MODIFY_INTRO:
        	return player->modify_league_intro(msg_proto);
        case CLIENT_LEAGUE_FETCH_WELFARE:
        	return player->fetch_league_welfare();
        case CLIENT_LEAGUE_DONATE:
        	return player->league_donate(msg_proto);
        case CLIENT_LEAGUE_MEMBER_LIST:
        	return player->fetch_league_member_list(msg_proto);
        case CLIENT_LEAGUE_APPLY_LIST:
        	return player->fetch_league_apply_list(msg_proto);
        case CLIENT_LEAGUE_APPOINT:
        	return player->league_appoint(msg_proto);
        case CLIENT_LEAGUE_KICK:
        	return player->league_kick(msg_proto);
        case CLIENT_LEAGUE_LEADER_TRANSFER:
        	return player->league_leader_transfer(msg_proto);
        case CLIENT_IMPEACH_LEAGUE_LEADER:
        	return player->impeach_league_leader();
        case CLIENT_MEMBER_IMPEACH_VOTE:
        	return player->impeach_vote(msg_proto);
        case CLIENT_LEAGUE_ACCEPT_APPLY:
        	return player->accept_league_apply(msg_proto);
        case CLIENT_LEAGUE_REJECT_APPLY:
        	return player->reject_league_apply(msg_proto);
        case CLIENT_LEAGUE_AUTO_ACCEPT:
        	return player->set_league_auto_accept(msg_proto);
        case CLIENT_LEAGUE_LOG:
        	return player->fetch_league_log(msg_proto);
        case CLIENT_LEAGUE_OPEN:
        	return player->open_league();
        case CLIENT_LEAGUE_OPEN_DONATE:
        	return player->open_league_donate();
        case CLIENT_LEAGUE_SHOP_INFO:
        	return player->fetch_league_shop_info(msg_proto);
        case CLIENT_LEAGUE_SHOP_BUY:
        	return player->league_shop_buy(msg_proto);
        case INNER_MAP_LEAGUE_DONATE:
        	return player->map_league_donate(msg_proto);
        case INNER_MAP_LEAGUE_CREATE:
        	return player->map_create_league(msg_proto);
        case INNER_MAP_LEAGUE_BUY:
        	return player->map_league_shop_buy(msg_proto);
        case CLIENT_LEAGUE_SKILL:
        	return player->fetch_league_skill_info();
        case CLIENT_LEAGUE_SKILL_UPGRADE:
        	return player->upgrade_league_skill(msg_proto);
        case CLIENT_FETCH_LEAGUE_FLAG_INFO:
        	return player->fetch_league_flag_info();
        case CLIENT_UPGRADE_LEAGUE_FLAG:
        	return player->upgrade_league_flag();
        case CLIENT_LESCORT_SELECT_CAR:
        	return player->select_escort_car_type(msg_proto);
        case CLIENT_LESCORT_OPEN_INFO:
        	return player->open_escort_info();
        case INNER_LOGIC_SYNC_PERSION_INFO:
        	return player->sync_league_info();
        case INNER_LOGIC_SYNC_LEAGUE_CONTRI:
        	return player->sync_league_member_contri(msg_proto);
        case CLIENT_LEAGUE_BOSS_INFO:
            return player->fetch_league_boss_info();
        case CLIENT_FEED_LEAGUE_BOSS:
        	return player->feed_league_boss(msg_proto);
        case INNER_ML_MAP_FEED_LEAGUE_BOSS:
        	return player->map_feed_league_boss(msg_proto);
        case CLIENT_SUMMON_BOSS:
        	return player->summon_boss(msg_proto);
        case CLIENT_ENTER_LEAGUE_BOSS:
        	return player->request_enter_league_boss();
        case INNER_LEAGUE_BOSS_SUMMON:
        	return player->map_summon_boss(msg_proto);
        case INNER_CHECK_LEAGUE_OPEN_TASK:
        	return player->league_handle_player_task();
		case INNER_LOGIC_ACT_CORNUCOPIA_REWARD:
			return player->update_cornucopia_activity_recharge(msg_proto);
		case CLIENT_FETCH_REGION_WELFARE:
			return player->fetch_league_region_info();
		case CLIENT_LEADER_DRAW_REGION_REWARD:
			return player->leader_draw_region_reward();
		case CLIENT_DRAW_REGION_REWARD:
			return player->draw_league_region_reward(msg_proto);

        case CLIENT_REQUEST_FRIEND_LIST:
        	return player->fetch_friend_list(msg_proto);
        case CLIENT_REQUEST_SEND_FRIEND_APPLY:
        	return player->send_friend_apply(msg_proto);
        case CLIENT_REQUEST_ACCEPT_FRIEND_APPLY:
        	return player->accept_friend_apply(msg_proto);
        case CLIENT_REQUEST_DELETE_FRIEND_APPLY:
        	return player->remove_apply_info(msg_proto);
        case CLIENT_REQUEST_FETCH_APPLY_LIST:
        	return player->request_fetch_apply_list();
        case CLIENT_SEND_FRIEND_TO_BLACK:
        	return player->send_friend_to_black(msg_proto);
        case CLIENT_REMOVE_BLACK_FRIEND:
        	return player->remove_black_friend(msg_proto);
        case INNER_FETCH_NEARBY_PLAYER:
        	return player->fetch_nearby_player(msg_proto);
        case CLIENT_TRANSALTE_TO_ENEMY_POSTION:
            return player->translate_to_enemy_position(msg_proto);
        case INNER_FETCH_ENEMY_POSITION:
        	return player->after_fetch_to_enemy_position(msg_proto);
        case CLIENT_APPEND_TO_FRIEND_LIST:
        	return player->append_to_friend_list(msg_proto);
        case CLIENT_REMOVE_FROM_FRIEND_LIST:
        	return player->remove_from_friend_list(msg_proto);
        case CLIENT_SEARCH_FRIEND_BY_NAME:
        	return player->search_friend_by_name(msg_proto);
        case CLIENT_RECOMMEND_FRIEND:
        	return player->recommend_friend(msg_proto);
        case CLIENT_FETCH_SINGLE_PLAYER_INFO:
        	return player->fetch_friend_info_by_role_id(msg_proto);
        case CLIENT_FETCH_SINGLE_PLAYER_DETAIL:
        	return player->fetch_single_player_all_by_role_id(msg_proto);
        case TRANS_LOAD_LOGIC_SOCIALER_INFO:
        	return player->after_fetch_friend_list(transaction);
        case TRANS_LOAD_LOGIC_APPLY_INFO:
        	return player->after_accept_friend_apply(transaction);
        case TRANS_LOAD_SINGLE_LOGIC_SOCIALER_INFO:
        	return player->after_fetch_friend_info_by_role_id(transaction);
        case TRANS_LOAD_SINGLE_PLAYER_ALL_INFO:
        	return player->after_fetch_single_player_all_info(transaction);
        case TRANS_LOAD_MASTER_OFFLINE:
        	return player->after_fetch_other_master(transaction);
        case TRANS_GET_RPM_RECOMAND_INFO:
        case TRANS_GET_RPM_INTORDUCTION_INFO:
        	return player->after_get_rpm_recomand_info(transaction);
        case RETURN_FETCH_SINGLE_PLAYER_DETAIL:
        	return player->respond_single_player_all_info(msg_proto);
        case RETURN_REQUEST_FETCH_RANKER_DETAIL:
        	return player->respond_ranker_detail(msg_proto);

        case CLIENT_OPEN_LUCKY_TABLE:
            return player->open_ltable(msg_proto);
        case CLIENT_CLOSE_LUCKY_TABLE:
            return player->close_ltable();
        case CLIENT_OPERATOR_LUCKY_TABLE:
            return player->exec_ltable(msg_proto);
        case LOGIC_ADD_LUCKY_TABLE_TIMES:
            return player->add_ltable_times(msg_proto);
        case INNER_ML_LUCKY_TABLE_COST:
            return player->exec_ltable_after(msg_proto);
      /*  case NOTIFY_OFFLINE_HOOK_INFO:
        	return OFFLINEHOOK_MONITOR->sync_offline_hook(msg_proto) ;
        case NOTIFY_OFFLINE_HOOK_APPLY:
        	return OFFLINEHOOK_MONITOR->player_offlinehook_apply(msg_proto) ; */

        // 巅峰对决
        case CLIENT_SELF_TRAVEL_TEAM_INFO:
        	return player->request_myself_travel_team_info();
        case CLIENT_CREATE_TRAVEL_TEAM:
        	return player->request_create_travel_team_begin(msg_proto);
        case CLIENT_LOCAL_TRAVEL_TEAM_LIST:
        	return player->request_local_travel_team_list(msg_proto);
        case CLIENT_APPLY_TRAVEL_TEAM:
        	return player->request_apply_travel_team(msg_proto);
        case CLIENT_TRAVEL_TEAM_APPLY_LIST:
        	return player->request_travel_team_apply_list();
        case CLIENT_REPLY_TRAVEL_TEAM_APPLY:
        	return player->request_reply_travel_team_apply(msg_proto);
        case CLIENT_CHANGE_TRAVEL_TEAM_LEADER:
        	return player->request_change_travel_team_leader(msg_proto);
        case CLIENT_KICK_TRAVEL_TEAMER:
        	return player->request_kick_travel_teamer(msg_proto);
        case CLIENT_OTHER_TRAVEL_TEAM_INFO:
        	return player->request_other_travel_team_info(msg_proto);
        case CLIENT_INVITE_TRAVEL_TEAM:
        	return player->request_invite_to_travel_team(msg_proto);
        case CLIENT_REPLY_TRAVEL_TEAM_INVITE:
        	return player->request_reply_invite_travel_team(msg_proto);
        case CLIENT_DIMISS_TRAVEL_TEAM:
        	return player->request_dimiss_travel_team();
        case CLIENT_LEAVE_TRAVEL_TEAM:
        	return player->request_leave_travel_team();
        case CLIENT_SET_TEAM_FORCE:
        	return player->request_set_team_force(msg_proto);
        case CLIENT_SET_TEAM_AUTO_TYPE:
        	return player->request_set_team_auto_type(msg_proto);
        case CLIENT_SIGNUP_TRAVEL_PEAK:
        	return player->request_signup_travel_peak();

        case INNER_SET_SIGNUP_TRAVPEAK_FLAG:
        	return player->process_set_signup_travpeak_flag(msg_proto);
        case INNER_CHECK_MONEY_CREATE_TRAVTEAM:
        	return player->process_create_travel_team_end(msg_proto);

        case INNER_LOGIC_LOGOUT:
        {
        	if (player->gate_sid() != sid)
        	{
        		MSG_USER("ERROR logic_player gate_sid diff %ld %s %d %d", player->role_id(),
        				player->name(), player->gate_sid(), sid);
        		return 0;
        	}
            return player->sign_out();
        }
        case INNER_SYNC_ROLE_INFO:
            return player->sync_role_info(msg_proto);
        case CLIENT_ML_TEST_COMMAND:
        	return player->logic_test_command(msg_proto);

        case CLIENT_MARKET_INFO:
        	return player->fetch_market_info(msg_proto);
        case CLIENT_MARKET_ONSELL:
        	return player->market_onsell(msg_proto);
        case INNER_MAP_MARKET_ONSELL:
        	return player->map_market_onsell(msg_proto);
        case CLIENT_MARKET_BUY:
        	return player->market_buy_goods(msg_proto);
        case CLIENT_MARKET_SELF:
        	return player->fetch_self_market_info(msg_proto);
        case CLIENT_MARKET_GETBACK:
        	return player->market_get_back(msg_proto);
        case CLIENT_MARKET_CONSELL:
        	return player->market_continue_sell(msg_proto);
        case CLIENT_MARKET_LOW_PRICE:
        	return player->fetch_market_low_price(msg_proto);
        case CLIENT_MARKET_SELL_LOG:
        	return player->fetch_sell_log(msg_proto);
        case CLIENT_MARKET_SHOURT:
        	return player->shout_market_item(msg_proto);

        case INNER_LOGIC_ACT_SERIAL_INFO:
        	return player->sync_update_act_serial_info(msg_proto);

        case INNER_ESCORT_SERIAL_GOLD_INFO:
        	return player->sync_update_escort_info(msg_proto);
        case INNER_QUINTUPLE_MONSTER_EXP_INFO:
        	return player->sync_update_quintuple_exp_info(msg_proto);
        case INNER_MAP_FASHION_STYLE:
        	return player->sync_uddate_fashion_info(msg_proto);

        //	escort
        case INNER_ESCORT_ACTIVITY_INFO:
        	return player->refresh_escort_info(msg_proto);

        case INNER_LOGIC_SYNC_PERMISSION_INFO:
        	return player->refresh_permission_info(msg_proto);

        case INNER_ML_RESEX_OPERATE:
        	return player->handle_resex(msg_proto);
            // vip
        case INNER_LOGIC_SYNC_VIP_INFO:
            return player->sync_vip_info(msg_proto);

        case INNER_LOGIC_SYNC_HOOKING_INFO:
        	return player->sync_hook_info(msg_proto);

            //rank system
        case CLIENT_REQUEST_FETCH_RANK_DATA:
        	return player->fetch_rank_data(msg_proto);
        case CLIENT_REQUEST_FETCH_RANKER_DETAIL:
        	return player->fetch_ranker_detail_info(msg_proto);
        case CLIENT_REQUEST_WORSHIP_RANK:
        	return player->request_worship_rank(msg_proto);
        case INNER_ML_FETCH_SELF_RANKER_DATA:
            return player->process_fetch_rank_data_after_fetch_self_data(msg_proto);
        case TRANS_LOAD_RANKER_INFO:
        case TRANS_LOAD_RANK_PET_INFO:
        case TRANS_LOAD_RANK_MOUNT_INFO:
        case TRANS_LOAD_RANKER_WING_INFO:
        case TRANS_LOAD_KILL_VALUE_RANK_DATA:
        case TRANS_LOAD_HERO_RANK_DATA:
//        case TRANS_LOAD_PLAYER_FUN_MOUNT_INFO:
//        case TRANS_LOAD_PLAYER_FUN_GOD_SOLIDER_INFO:
//        case TRANS_LOAD_PLAYER_FUN_MAGIC_EQUIP_INFO:
//        case TRANS_LOAD_PLAYER_FUN_XIAN_WING_INFO:
//        case TRANS_LOAD_PLAYER_FUN_LING_BEAST_INFO:
//        case TRANS_LOAD_PLAYER_FUN_BEAST_EQUIP_INFO:
//        case TRANS_LOAD_PLAYER_FUN_BEAST_MOUNT_INFO:
        	return player->after_fetch_ranker_detail(transaction);
        case TRANS_LOAD_RANKER_PET_MOUNT_INFO:
        	return player->respond_fetch_rank_beast_info(transaction);

        	//chat system
        case INNER_CHAT_ADD_STRANGE_FRIEND:
        	return player->record_be_added_as_stranger_friend(msg_proto);

        	// arena
        case CLIENT_ARENA_FETCH_INFO:
        	return player->fetch_arena_info();
        case CLIENT_ARENA_DRAW_REWARD:
        	return player->draw_area_reward();
        case CLIENT_ARENA_CHALLENGE:
        	return player->start_area_challenge(msg_proto);
        case CLIENT_ARENA_CHANGE_SKIP:
        	return player->change_arena_skip(msg_proto);
        case CLIENT_ARENA_BUY_TIMES:
        	return player->buy_arena_times_begin();
        case CLIENT_ARENA_CLEAR_COOL:
        	return player->clear_arena_cool_begin();
        case CLIENT_ARENA_BUY_INFO:
        	return player->fetch_arena_times_money();
        case INNER_ML_ARENA_BUY_TIMES:
        	return player->buy_arena_times_done(msg_proto);
        case INNER_ML_ARENA_BUY_COOL:
        	return player->clear_arena_cool_done(msg_proto);
        case CLIENT_ARENA_CLOSE_INFO:
        	return player->close_arena_info();
        case CLIENT_ARENA_REFRESH_PLAYER:
        	return player->refresh_arena_player();
        case CLIENT_ARENA_RANK_LIST:
        	return player->fetch_arena_rank_list();
        case CLIENT_CHANGE_REWARD:
        	return player->fetch_change_buy_begin(msg_proto);

        	//activity notification
        case CLIENT_ACTIVITY_TIPS_FETCH_ALL:
        	return player->fetch_all_tips_info();
        case CLIENT_ACTIVITY_TIPS_TOUCH:
        	return player->touch_tips_icon(msg_proto);

        	//logic customer serivce system
        case CLIENT_REQUEST_OPEN_CUSTOMER_SVC_PANNEL:
        	return player->request_open_customer_service_pannel(msg_proto);
        case CLIENT_REQUEST_OPEN_REPLAY_PANNEL:
        	return player->request_open_replay_pannel(msg_proto);
        case CLIENT_SUMMIT_CUSTOMER_SERVICE_RECORD:
        	return player->summit_customer_service_record(msg_proto);
        case CLIENT_REQUEST_READ_CUSTOMER_SVC_RECORD:
        	return player->request_read_customer_service_record(msg_proto);
        case CLIENT_REQUEST_DEL_CUSTOMER_SVC_RECORD:
        	return player->request_delete_customer_service_record(msg_proto);
        case CLIENT_REQUEST_EVALUATE_CUSTOMER_REPLAY:
        	return player->request_evaluate_customer_service_replay(msg_proto);
        case TRANS_LOAD_BACK_CUSTOMER_SVC_RECORD:
        	return player->after_customer_svc_timeup_load_data(transaction);
        
        case INNER_SYNC_ROLE_NAME:
            return player->sync_update_player_name(msg_proto);
        case INNER_SYNC_ROLE_SEX:
        	return player->sync_update_player_sex();
        case INNER_LOGIC_SYNC_FIGHT_DETAIL:
        	return player->sync_update_fight_detail(msg_proto);
        case IL_SYNC_MOUNT_INFO:
        	return player->sync_update_mount_info(msg_proto);
        case INNER_LOGIC_SYNC_RECHARGE_TOTAL_GOLD:
        	return player->sync_update_player_total_recharge_gold(msg_proto);
        case  INNER_LOGIC_KILL_PLAYER:
        	return player->sync_update_kill_player(msg_proto);

        case INNER_LOGIC_REQ_QUERY_ACTI_CODE:
        	return player->request_query_center_acti_code(msg_proto);
        case INNER_LOGIC_RET_QUERY_ACTI_CODE:
        	return player->return_query_center_acti_code(msg_proto);

        case CLIENT_OPEN_ACT_FETCH_INFO:
        	return player->fetch_open_activity(msg_proto);
        case CLIENT_OPEN_ACT_DRAW:
        	return player->draw_open_activity_reward(msg_proto);
        case CLIENT_OPEN_ACT_LIST:
        	return player->fetch_open_activity_list();
        case CLIENT_OPEN_ACT_BUY:
        	return player->request_activity_buy_begin(msg_proto);
        case CLIENT_FEST_ACT_INFO:
        	return player->fetch_festival_activity_list();
        case IL_OPEN_ACTIVITY_PLAYER_INFO:
        	return player->update_open_activity_info(msg_proto);
        case IL_SYNC_COUPLE_FB_TIMES:
        	return player->update_couple_fb_times();
        case CLIENT_FETCH_CUMULATIVE_INFO_STATE:
        	return player->update_cumulative_logic_info_state(msg_proto);
        case CLIENT_FETCH_CORNUCOPIA_INFO:
        	return player->fetch_cornucopia_msg(msg_proto);
        case CLIENT_FETCH_CORNUCOPIA_REWARD:
        	return player->fetch_cornucopia_reward(msg_proto);
        case INNER_UPDATE_CORNUCOPIA_TASK:
        	return player->update_cornucopia_activity_value(msg_proto);
        case INNER_UPDATE_LABOUR_TASK:
        	return player->update_labour_activity_value(msg_proto);
        case CLIENT_SYNA_FISH_INFO:
        	return player->syna_fish_info(msg_proto);
        case CLIENT_GET_FISH:
        	return player->fetch_fish_begin(msg_proto);
        case CLIENT_FETCH_FISH_SCORE_REWARD:
        	return player->fetch_fish_score_reward(msg_proto);
        case CLIENT_FETCH_FISH_SCORE_INFO:
        	return player->fetch_fish_score_info(msg_proto);
        case CLIENT_FETCH_FISH_REWARD_INFO:
        	return player->fetch_fish_tips_info(msg_proto);
        case INNER_USE_PROP_ADD_FISH_SCORE:
        	return player->use_prop_add_score(msg_proto);

        // 女神赐福
//        case CLIENT_FETCH_GODDESS_BLESS_INFO:
//        	return player->fetch_bless_activity_info();
//        case CLIENT_GODDESS_BLESS_OPERATOR:
//        	return player->goddess_bless_operator_begin(msg_proto);
//        case CLIENT_GODDESS_BLESS_EXCHANGE:
//        	return player->goddess_bless_exchange_begin(msg_proto);

        //神秘宝箱
        case CLIENT_SPECIAL_BOX_INFO:
        	return player->fetch_special_box_info(NULL);
        case CLIENT_SPECIAL_BOX_BUY_KEY:
        	return player->fetch_special_box_buy_key_begin(msg_proto);
        case CLIENT_SPECIAL_BOX_COST_ITEM:
        	return player->fetch_special_box_reward_check_item(msg_proto);
        case CLIENT_SPECIAL_BOX_COST_MONEY:
        	return player->fetch_special_box_reward_check_money(msg_proto);
        case INNER_LUCKY_WHEEL_ALL_COST:
        	return player->cost_item_or_money_return(msg_proto);
        case CLIENT_SPECIAL_BOX_CHANGE_INFO:
        	return player->fetch_special_box_change_info(msg_proto);
        case CLIENT_SPECIAL_BOX_CHANGE_REWARD:
        	return player->fetch_special_box_change_reward_begin(msg_proto);

        // may activity
        case CLIENT_MAY_ACTIVITY_INFO:
        	return player->fetch_may_activity_list();
        case CLIENT_ONE_MAY_ACTIVITY:
        	return player->fetch_may_activity(msg_proto);
        case CLIENT_FETCH_MAY_ACT_REWARD:
        	return player->draw_may_activity_reward(msg_proto);
        case CLIENT_MAY_ACT_BUY:
        	return player->request_may_activity_buy_begin(msg_proto);
        case CLIENT_ADD_COUPLE_TICK:
        	return player->request_add_couple_tick_begin(msg_proto);
        case CLIENT_DAILY_RUN_RUN:
        	return player->request_start_run_begin(msg_proto);
        case CLIENT_DAILY_RUN_BUY:
        	return player->request_daily_run_buy_begin(msg_proto);
        case CLIENT_DAILY_RUN_FRIEND:
        	return player->request_daily_run_friend_list(msg_proto);
        case CLIENT_DAILY_RUN_SEND:
        	return player->requrst_daily_run_send(msg_proto);
        case CLIENT_DAILY_RUN_JUMP:
        	return player->request_daily_run_jump_begin(msg_proto);

        //抢红包活动
        case CLIENT_RED_PACKET_REWARD_INFO:
        	return player->fetch_red_packet_reward_info(msg_proto);
        case CLIENT_FETCH_RED_PACKET_REWARD:
        	return player->fetch_red_packet_reward(msg_proto);
        case CLIENT_ACT_OPEN_TIME_STATE:
        	return player->fetch_red_packet_state_by_group(msg_proto);

        case CLIENT_FASHION_ACT_TAKE_REWARD:
        	return player->fetch_fashion_buy_begin(msg_proto);
        case CLIENT_FASHION_ACT_LIVENESS_REWARD:
        	return player->fetch_fashion_reward(msg_proto);

        //lucky_wheel activity
        case CLIENT_LUCKY_WHEEL_LIST_NEW:
        	return player->fetch_wheel_act_list();
        case CLIENT_ONE_LUCKY_WHEEL_ACTIVITY:
        	return player->fetch_one_wheel_activity(msg_proto);
        case CLIENT_DRAW_LUCKY_WHEEL_REWARD:
        	return player->draw_award_begin(msg_proto);
        case CLIENT_LUCKY_WHEEL_EXCHANGE:
        	return player->lucky_wheel_exchange(msg_proto);
        case CLIENT_RESET_LUCKY_WHEEL:
        	return player->request_activity_reset_begin(msg_proto);
        case CLIENT_REQUEST_TIME_LIMIT:
        	return player->fetch_time_limit_info();
        case CLIENT_TIME_LIMIT_BUY:
        	return player->time_limit_item_buy_begin(msg_proto);
        case CLIENT_TIME_LIMIT_REWARD:
        	return player->fetch_activity_reward(msg_proto);
        case CLIENT_REQUEST_COUPLE_ACT:
        	return player->fetch_wedding_rank_info(msg_proto);
        case CLIENT_FETCH_COUPLE_ACT_REWARD:
        	return player->fetch_wedding_act_reward_begin(msg_proto);
        case CLIENT_OPEN_NINE_WORD:
        	return player->request_open_nine_word_begin(msg_proto);
        case CLIENT_FETCH_NINE_WORD_REWARD:
        	return player->fetch_nine_word_reward(msg_proto);
        case CLIENT_FETCH_ACT_RANK_INFO:
        	return player->fetch_act_rank_info(msg_proto);
        case CLIENT_OPEN_LUCKY_EGG:
        	return player->request_open_lucky_egg_begin(msg_proto);
        case CLIENT_FETCH_RECHARGE_RANK:
        	return player->fetch_recharge_rank_info(msg_proto);
        case CLIENT_FETCH_RECHARGE_REBATE:
        	return player->fetch_recharge_rebate_info();
        case INNER_LOGIC_SYNC_REBATE_RECHARGE:
        	return player->recharge_rebate(msg_proto);
        case CLIENT_FETCH_GASHAPON_INFO:
        	return player->fetch_gashapon_info(msg_proto);
        case CLIENT_DRAW_GASHAPON_REWARD:
        	return player->fetch_gashapon_reward(msg_proto);

        //宝石合成
        case CLIENT_GEM_SYNTHESIS_INFO:
        	return player->fetch_gem_activity_info();
        case CLIENT_GEM_SYNTHESIS_OP:
        	return player->gem_synthesis(msg_proto);
        case CLIENT_GEM_SYNTHESIS_REWARD:
        	return player->draw_gem_synthesis_reward();

        case CLIENT_CABINET_INFO:
        	return player->fetch_cabinet_info(msg_proto);
        case CLIENT_CABINET_BUY:
        	return player->cabinet_buy_begin(msg_proto);
        case CLIENT_CABINET_REFRESH:
        	return player->cabinet_refresh_begin(msg_proto);
        case CLIENT_CABINET_REWARDS:
        	return player->cabinet_refresh_reward(msg_proto);

        	//迷宫寻宝
        case CLIENT_MAZE_TREASURE_INFO:
        	return player->fetch_maze_treasures(msg_proto);
        case CLIENT_MAZE_TREASURE_DRAW:
        	return player->draw_maze_begin(msg_proto);

        	//神仙鉴宝
        case CLIENT_IMMORTAL_TREASURE_INFO:
        	return player->fetch_immortal_treasures();
        case CLIENT_IMMORTAL_TREASURE_DRAW:
        	return player->draw_immortal_treasures_begin();
        case CLIENT_IMMORTAL_TREASURE_RAND:
        	return player->rand_immortal_treasures_begin(msg_proto);
        case CLIENT_IMMORTAL_TREASURE_REWARD:
        	return player->fetch_immortal_treasures_reward();

        //每日活动
        case CLIENT_DAILY_ACT_LIST:
        	return player->fetch_daily_act_list();
        case CLIENT_TOTAL_DOUBLE_ACT:
        	return player->request_total_double_info();
        case INNER_PLAY_TREASURES_DICE:
        	return player->fetch_player_dice_mult(msg_proto);

        // 后台活动
        case CLIENT_BACK_ACT_LIST:
            return player->fetch_back_activity_list(msg_proto);
        case CLIENT_SINGLE_BACK_ACT_INFO:
            return player->fetch_single_back_activity_info(msg_proto);
        case CLIENT_DRAW_SINGLE_BACK_ACT:
            return player->draw_single_back_activity_reward(msg_proto);
        case INNER_BACK_ACT_REWARD_ITEM:
            return player->process_back_act_reward_state_after_insert(msg_proto);
        case CLIENT_FETCH_BACK_TRAV_RECHARGE_RANK:
            return player->fetch_back_travel_rank_info(msg_proto);
            
		// 结婚系统
        case INNER_LOGIC_FIGHT_PROP_INFO:
        	return player->notify_fight_prop_info(msg_proto);
		case CLIENT_WEDDING_BEFORE:
			return player->request_wedding_before(msg_proto);
		case CLIENT_WEDDING:
			return player->request_wedding(msg_proto);
		case CLIENT_WEDING_INVEST:
			return player->request_wedding_invest(msg_proto);
		case CLIENT_WEDING_REPLY:
			return player->request_wedding_reply(msg_proto);
		case CLIENT_WEDDING_MAKE_UP:
			return player->request_wedding_make_up(msg_proto);
		case CLIENT_DIVORCE:
			return player->request_divorce(msg_proto);
		case CLIENT_KEEPSAKE_UPGRADE:
			return player->request_keepsake_upgrade(msg_proto);
		case CLIENT_PRESENT_FLOWER:
			return player->request_present_flower(msg_proto);
		case CLIENT_WEDDING_PANNEL:
			return player->request_wedding_pannel();
		case CLIENT_REPLY_WEDDING:
			return player->request_reply_wedding(msg_proto);
		case CLIENT_UPDATE_RING:
			return player->request_update_ring_begin(msg_proto);
		case CLIENT_UPDATE_SYS:
			return player->request_update_sys(msg_proto);
		case CLIENT_UPDATE_TREE:
			return player->request_update_tree_begin(msg_proto);
		case CLIENT_BUY_WEDDING_TREASURES:
			return player->request_buy_wedding_treasures_begin(msg_proto);
		case CLIENT_FETCH_WEDDING_TREASURES:
			return player->request_fetch_wedding_treasures(msg_proto);

		case CLIENT_WEDDING_LABEL_INFO:
			return player->request_wedding_label_info();
		case CLIENT_WEDDING_GET_LABEL:
			return player->request_wedding_get_label(msg_proto);

		case INNER_ML_WEDDING_BUY_TREASURES:
			return player->request_buy_wedding_treasures_end(msg_proto);
		case INNER_ML_WEDDING_UPDATE:
			return player->request_update_ring_or_tree_end(msg_proto);

		case INNER_WEDDING_CHECK_PACK:
			return player->process_wedding_after_pack_check(msg_proto);
		case INNER_DIVORCE_CHECK_PACK:
			return player->process_divorce_after_pack_check(msg_proto);
		case INNER_KEEPSAKE_UPGRADE:
			return player->process_keepsake_upgrade_after_pack_check(msg_proto);
		case INNER_PRESENT_FLOWER:
			return player->process_present_flower_after_pack_check(msg_proto);
		case CLIENT_SERVER_FLAG:
			return player->request_sever_flag();

	    // league store
		case CLIENT_LSTORE_FETCH_INFO:
			return player->fetch_store();
		case CLIENT_LSTORE_FETCH_APPLY:
			return player->fetch_apply_list(msg_proto);
		case CLIENT_LSTORE_SORT:
			return player->sort_store();
		case CLIENT_LSTORE_INSERT:
			return player->insert_store(msg_proto);
		case CLIENT_LSTORE_GET_ITEM:
			return player->apply_item(msg_proto);
		case CLIENT_LSTORE_CHECK_APPLY:
			return player->check_apply(msg_proto);
		case CLIENT_LSTORE_FETCH_APPLY_HIS:
			return player->fetch_apply_history(msg_proto);
		case CLIENT_LSTORE_OPEN_LSTORE:
			return player->open_store();
		case CLIENT_LSTORE_CLOSE_LSTORE:
			return player->close_store();
			//精彩活动（开服活动）：充值返还，向地图服发送请求更新玩家领奖信息

		//帮派副本
		case CLIENT_LEAGUE_FB_REWARD_INFO:
			return player->request_lfb_wave_reward();
		case CLIENT_FETCH_LEAGUE_FB_REWARD:
			return player->fetch_lfb_wave_reward(msg_proto);
		case CLIENT_LEAGUE_FB_CHEER:
			return player->lfb_cheer_leaguer(msg_proto);
		case CLIENT_FETCH_LEAGUE_FB_CHEER_INFO:
			return player->request_lfb_cheer_info();
		case CLIENT_REQUEST_ENTER_LFB:
			return player->request_enter_lfb(msg_proto);

		//trade
		case INNER_TRADE_FETCH_ON_LINE_STATE:
			return player->trade_fetch_on_line_state(msg_proto);
        
        case INNER_TRVL_REWARD_ID:
            return player->process_travel_reward_by_mail(msg_proto);

        case INNER_CENTER_PLAYER_INCOME_LOG:
        {
            DYNAMIC_CAST_RETURN(Proto32101107 *, request, msg_proto, -1);
            request->set_sid(player->role_detail().__server_id);
            return LOGIC_MONITOR->process_inner_center_thread(*msg_proto);
        }
        case INNER_LOGIC_DELETE_RETURN_RECHARGE:
        {
        	MSG_USER("logic get delete_return_recharge result and trans it to map");
        	return LOGIC_MONITOR->dispatch_to_scene(player, msg_proto);
        }
        default:
        {
            MSG_USER("ERROR can't recognize recogn %d %d", sid, recogn);
            if (transaction != NULL)
            {
            	transaction->rollback();
            	transaction = NULL;
            }
            return -1;
        }
    }

    return 0;
}

int LogicUnit::interval_run(void)
{
	return 0;
}

int LogicUnit::process_stop(void)
{
    LOGIC_MONITOR->logout_all_player();
    
    return BaseUnit::process_stop();
}

int LogicUnit::process_php_request(const int sid, const int recogn, UnitMessage *unit_msg)
{
	Message *proto_msg = unit_msg->proto_msg();
	Block_Buffer *msg = unit_msg->data_buff();

	int64_t role_id = 0;
	int role_size = 0;
	std::vector<int64_t> role_list;

	if(unit_msg->__type == UnitMessage::TYPE_BLOCK_BUFF)
	{
		msg->read_int32(role_size);
		for (int i = 0; i < role_size; ++i)
		{
			role_id = 0;
			msg->read_int64(role_id);
			if (role_id == 0)
			{
				continue;
			}
			role_list.push_back(role_id);
		}
	}

	switch(recogn)
	{
		case PHP_REQUEST_ROLE_INFO:
			return LOGIC_PHP->request_role_info(sid, proto_msg);

		case INNER_PHP_TEST_COMMAND:
			return LOGIC_PHP->request_test_command(sid, role_list, msg);

		case INNER_PHP_INSERT_ITEM:
			return LOGIC_PHP->request_insert_item(sid, role_list, msg);
		case INNER_PHP_REMOVE_ITEM:
			return LOGIC_PHP->request_remove_item(sid, role_list, msg);

		default:
			MSG_USER("error recogn %d", recogn);
			break;
	}

	return 0;
}


