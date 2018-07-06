/*
 * MapLogicUnit.cpp
 *
 * Created on: 2013-11-28 11:03
 *     Author: lyz
 */

#include "MapLogicUnit.h"
#include "MapLogicPlayer.h"
#include "Transaction.h"
#include "MapMonitor.h"
#include "PoolMonitor.h"
#include "TransactionMonitor.h"
#include "DaemonServer.h"
#include "ProtoDefine.h"
#include "MLBackDailyRechargeSys.h"
#include "BackActivityTick.h"

int MapLogicUnit::type(void)
{
    return MAP_LOGIC_UNIT;
}

UnitMessage *MapLogicUnit::pop_unit_message(void)
{
    return MAP_MONITOR->unit_msg_pool()->pop();
}

int MapLogicUnit::push_unit_message(UnitMessage *msg)
{
    return MAP_MONITOR->unit_msg_pool()->push(msg);
}

int MapLogicUnit::process_block(UnitMessage *unit_msg)
{
    uint32_t len = unit_msg->__len;
    int32_t sid = unit_msg->__sid, recogn = unit_msg->__msg_head.__recogn,
            trans_id = unit_msg->__msg_head.__trans_id;
    int64_t role_id = unit_msg->__msg_head.__role_id;

    Message *msg_proto = unit_msg->proto_msg();
    int ivalue = unit_msg->ivalue();

    Transaction *transaction = 0;
    if (trans_id > 0)
    	TRANSACTION_MONITOR->find_transaction(trans_id, transaction);

    switch (recogn)
    {
        case INNER_TIMER_TIMEOUT:
            return POOL_MONITOR->game_timer_timeout(ivalue);
        case INNER_MAP_LOGIC_LOGIN:
        {        
            DYNAMIC_CAST(Proto31400001*, request, msg_proto);
            return PLAYER_MANAGER->request_load_logic_player(request->gate_sid(), role_id);
        }
        case TRANS_LOAD_MAP_LOGIC_PLAYER:
            return PLAYER_MANAGER->after_load_logic_player(transaction);
        case INNER_MAP_LOGIC_LOGOUT:
            return PLAYER_MANAGER->process_ml_player_logout(sid, role_id);
        case INNER_ML_UPDATE_SID:
        	return MAP_MONITOR->update_logic_player_sid(sid, role_id, NULL);
        case INNER_MAP_SYNC_LOGIC_BASE:
            return PLAYER_MANAGER->update_transfer_logic_base(sid, dynamic_cast<Proto31400103 *>(msg_proto));
        case INNER_ML_SYNC_PACKAGE:
        case INNER_ML_SYNC_MOUNT:
        case INNER_ML_SYNC_DIVNEGON:
        case INNER_ML_SYNC_BEAST:
        case INNER_MAP_SYNC_CHECK_IN_INFO:
        case INNER_MAP_SYNC_LIVENESS_INFO:
        case INNER_MAP_SYNC_COPPER_BOWL_INFO:
        case INNER_ML_SYNC_VIP:
        case INNER_ML_SYNC_EXP_RESOTRE:
        case INNER_ML_SYNC_ACTIVITY_NOTIFY:
        case INNER_ML_SYNC_TEAM_INFO:
        case INNER_ML_SYNC_ONLINE_AWARD:
        case INNER_ML_SYNC_MAGICAL_POLISH:
        case INNER_ML_SYNC_LABEL_INFO:
        case INNER_ML_SYNC_OFFLINE_AWARD:
        case INNER_ML_SYNC_TREASURES_INFO:
        case INNER_ML_SYNC_COLLECT_CHESTS:
        case INNER_ML_SYNC_ILLUS:
        case INNER_ML_SYNC_ACHIEVEMENT:
        case INNER_ML_SYNC_MAIL_BOX_INFO:
        case INNER_ML_SYNC_PLAYER_TIP_PANNEL:
        case INNER_ML_SYNC_HOOK_DETAIL:
        case INNER_ML_SYNC_MEDIA_GIFT:
        case INNER_ML_SYNC_PROPER:
        case INNER_ML_SYNC_RECHARGE_REWARDS:
        case INNER_ML_SYNC_SCRIPT_CLEAN:
        case INNER_ML_SYNC_DAILY_RECHARGE:
        case INNER_ML_SYNC_REBATE_RECHARGE:
        case INNER_ML_SYNC_INVEST_RECHARGE:
        case INNER_ML_SYNC_TINY:
        case INNER_ML_SYNC_TOTAL_REWARDS:
        case INNER_ML_SYNC_BACK_ACT:
        case INNER_ML_SYNC_MAGICWEAPON:
        case INNER_ML_SYNC_SWORD_POOL_INFO:
        case INNER_ML_SYNC_FASHION_INFO:
        case INNER_ML_SYNC_TRANSFER_INFO:
        case INNER_ML_SYNC_HIDDEN_TREASURE_INFO:
        case INNER_MAP_SYNC_LOGIC_TASK:
        case SYNC_OFFLINE_HOOK_INFO:
        	return PLAYER_MANAGER->update_transfer_logic(role_id, recogn, msg_proto);
        case INNER_MAP_SYNC_LOGIC_ONLINE:
            return PLAYER_MANAGER->update_transfer_logic_online(sid, dynamic_cast<Proto31400105 *>(msg_proto));
        case INNER_TRANSFER_LOGIC_INFO_END:
            return PLAYER_MANAGER->finish_sync_transfer_logic_info(sid, dynamic_cast<Proto31400102 *>(msg_proto));
        case INNER_TRANSFER_SCENE_END:
            return PLAYER_MANAGER->finish_sync_transfer_start_logic(role_id);
        case INNER_SYNC_ML_OBJ_BY_BSON:
        	return PLAYER_MANAGER->update_transfer_logic_by_bson(role_id, msg_proto);
        case TRANS_LOAD_SHOP_MODE:
        	return MAP_MONITOR->db_load_mode_done(transaction);

        case TRANS_LOAD_DAILY_RECHARGE_OPEN_TIME:
        	return BackDR_SYS->after_load_DR_open_time(transaction);
        case INNER_LOGIC_SYNC_ACTIVITY_TICK:
            return BACK_ACTIVITY_TICK->sync_activity_tick(msg_proto);

        case INNER_ML_FORCE_LOGOUT:
        {
        	// 处理登录状态错误，清除对象残留
        	Proto31400005 *req = dynamic_cast<Proto31400005 *>(msg_proto);
        	MapLogicPlayer *player = 0;
        	if (MAP_MONITOR->player_manager()->find_logic_player(req->role_id(), player) == 0)
        	{
        		MSG_USER("force logout logic sign out %ld %s", player->role_id(), player->role_name().c_str());

        		player->sign_out(EXIT_SCENE_ERROR, false);
        	}
        	return 0;
        }
    	case CLIENT_ML_TEST_COMMAND:
    	{
    		MAP_MONITOR->map_test_command(msg_proto);
    		break;
    	}
        default:
            break;
    }

    MapLogicPlayer *player = 0;
    int find_flag = MAP_MONITOR->player_manager()->find_logic_player(role_id, player);

    /*处理玩家可能不在线的*/
    switch (recogn)
    {
    case INNER_MAP_MARKET_BUY:
    	return this->handle_with_noplayer_online(player, unit_msg);
    }

    if (find_flag != 0 || player->is_active() == false)
    {
        MSG_USER("ml player no active %ld %d %d %d", role_id, find_flag, sid, recogn);
        if (transaction != NULL)
        {
        	transaction->rollback();
        	transaction = NULL;
        }

        return -1;
    }

    switch (recogn)
    {
        case INNER_ML_LOOKUP_OTHER_INFO:
        	return player->fetch_other_player_info(msg_proto);
        case RETURN_FETCH_SINGLE_PLAYER_DETAIL:
        	return player->fetch_single_player_all_detail(msg_proto);
		case RETURN_REQUEST_FETCH_RANKER_DETAIL:
			return player->fetch_ranker_detail(msg_proto);
        case INNER_ML_FETCH_SELF_RANKER_DATA:
            return player->process_fetch_self_ranker_info(msg_proto);
        case INNER_ML_MAP_OBTAIN_AREA_DONE:
        	return player->map_obtain_area_info_done();
        case INNER_TRANSFER_LOGIC_INFO_BEGIN:
            return player->transfer_scene(msg_proto);

        case INNER_ML_ADD_MONEY:
        	return player->logic_request_add_money(msg_proto);
        case INNER_SUB_TRANSLATE_ENEMY:
               	return player->enemy_transalye_sub_money(msg_proto);
        case INNER_ML_ADD_GOODS:
        	return player->logic_request_add_goods(msg_proto);
        case INNER_ML_REMOVE_GOODS:
        	return player->logic_request_remove_goods(msg_proto);
        case INNER_ML_BATCH_ADD_ITEM:
        	return player->logic_request_add_batch_goods(msg_proto);
        case INNER_ML_PICK_UP:
        	return player->map_request_pickup_goods(msg_proto);
        case INNER_ML_GATHER_GOODS:
        	return player->map_request_gather_goods(msg_proto);
        case INNER_ML_RESET_PLAYER_CHESTS_INFO:
        	return player->reset_player_chests_info(msg_proto);
        case INNER_MAP_LOGIC_KILL_MONSTER:
        	return player->kill_monster(msg_proto);
        case INNER_MAP_LOGIC_COLLECT_ITEM:
        	return player->collect_item(msg_proto);
        case INNER_ML_TASK_COL_MONSTER:
            return player->task_collect_monster(msg_proto);
        case INNER_ML_TEAM_TASK_MONSTER_INFO:
        	return player->team_monster_info_update(msg_proto);
        case INNER_ML_TASK_BRANCH_UPDATE:
        	return player->task_branch_update(msg_proto);
        case INNER_MAP_LOGIC_DEATH_STATE:
        	return player->sync_death_state(msg_proto);
        case INNER_MAP_LOGIC_FIGHT_STATE:
        	return player->sync_fight_state(msg_proto);
        case INNER_ML_FINISH_ACTIVITY:
        	return player->sync_attend_activity(msg_proto);
        case INNER_ML_LEAGUE_RENAME:
        	return player->request_rename_league_done(msg_proto);
        case INNER_ML_RESEX_OPERATE:
        	return player->request_resex_role_done(msg_proto);
        case INNER_MAP_KILL_REDUCE_LUCKY:
            return player->process_kill_reduce_lucky(msg_proto);
        case INNER_LSTORE_INSERT_FAILED_TO_ML:
            return player->lstore_insert_failed(msg_proto);
        case INNER_LSTORE_INSERT_TO_ML:
            return player->lstore_insert(msg_proto);
        case INNER_LSTORE_GET_TO_ML:
            return player->lstore_get(msg_proto);
        case INNER_LRF_BUY_HICKTY:
        	return player->lrf_buy_hickty(msg_proto);

        case INNER_ML_SYNC_LEVEL_INFO:
        	return player->sync_info_from_map(msg_proto);
        case CLIENT_ML_TEST_COMMAND:
        	return player->logic_test_command(msg_proto);
        case INNER_MAP_TEAM_SYNC_INFO:
        	return player->read_logic_team_info(msg_proto);

        case INNER_MAP_LEAGUE_DONATE:
        	return player->logic_league_donate(msg_proto);
        case INNER_MAP_LEAGUE_CREATE:
        	return player->logic_create_league(msg_proto);
        case INNER_MAP_LEAGUE_ACCEPT:
        	return player->open_league_task(msg_proto);
        case INNER_MAP_LEAGUE_BUY:
        	return player->logic_league_shop_buy(msg_proto);
        case INNER_ML_FEE_DEDUCT:
        	return player->transfer_fee_deduct(msg_proto);
        case INNER_MAP_LEAGUE_SELF_INFO:
        	return player->logic_sync_league_id(msg_proto);
        case INNER_ML_MAP_FEED_LEAGUE_BOSS:
        	return player->logic_feed_league_boss(msg_proto);
        case INNER_ML_MAP_SUMMON_BOSS:
        	return player->logic_summon_boss(msg_proto);
        case INNER_ML_MAP_ADD_AWARD:
        	return player->add_reward(msg_proto);

        	// package
        case CLIENT_PACKAGE_INFO:
        	return player->fetch_pack_info(msg_proto);
        case CLIENT_OPEN_PACKAGE_GRID:
        	return player->open_pack_grid(msg_proto);
        case CLIENT_SORT_PACKAGE:
        	return player->pack_sort_and_merge(msg_proto);
        case CLIENT_ITEM_TIP:
        	return player->tip_pack_goods(msg_proto);
        case CLIENT_USE_GOODS:
            return player->use_pack_goods(msg_proto);
        case IM_CHECK_USE_GOODS:
        	return player->after_check_use_pack_goods(msg_proto);
        case INNER_MAP_USE_GOODS:
        	return player->map_use_pack_goods_done(msg_proto);
        case CLIENT_FETCH_ROTARY_TABLE:
            return player->fetch_rotary_table_result(msg_proto);
        case CLIENT_USE_ROTARY_TABLE:
            return player->use_rotary_table_goods(msg_proto);
        case CLIENT_LOGIN_REWARD_INFO:
        	return player->fetch_login_reward_info();
        case CLIENT_DRAW_LOGIN_REWARD:
        	return player->draw_login_reward(msg_proto);
        case CLIENT_LOGIN_REWARD_FLAG:
        	return player->fetch_login_reward_flag();
        case CLIENT_OPEN_GIFT_INFO:
        	return player->fetch_open_gift_info();
        case CLIENT_OPEN_GIFT_REWARD:
        	return player->draw_open_gift_reward(msg_proto);
        case CLIENT_AUTO_USE_BLOOD:
        	return player->request_add_blood_container(msg_proto);
        case CLIENT_DROP_GOODS:
        	return player->drop_goods(msg_proto);
        case CLIENT_REQUEST_SELL_GOODS:
        	return player->sell_goods(msg_proto);
        case CLIENT_REQUEST_batch_SELL_GOODS:
        	return player->batch_sell_goods(msg_proto);
        case CLIENT_REQUEST_RED_CLOTHES_EXCHANGE:
        	return player->red_clothes_exchange(msg_proto);
        case CLIENT_LEGEND_SECRET_EXCHANGE:
        	return player->request_exchange(msg_proto);
        case CLIENT_PACK_GRID_CLICK:
        	return player->fetch_item_detail_info(msg_proto);
        case CLIENT_PACK_OPEN:
        	return player->open_package();
        case CLIENT_SHOP_BUY_BACK:
        	return player->buy_goods_back(msg_proto);
        case CLIENT_SELLOUT_INFO:
        	return player->sell_out_info();
        case CLIENT_REQUEST_EXCHANGE_MONEY_WITH_GOLD:
        	return player->pack_money_exchange_with_gold(msg_proto);
        case INNER_MAP_MALL_BUY:
        	return player->buy_mall_goods(msg_proto);
        case INNER_SYCN_OWN_ITEM:
              	return player->get_own_goods(msg_proto);
        case INNER_ML_ADD_GAME_RES:
            return player->add_game_resource(msg_proto);
        case INNER_ML_ADD_MULT_GOODS:
        	return player->add_mult_item(msg_proto);
        case INNER_ML_ADD_SINGLE_GOODS:
        	return player->add_single_item(msg_proto);
        case INNER_ENTER_WORLD_USE_FLY:
        	return player->request_world_boss_enter(msg_proto);

        	//back recharge event
        case TRANS_LOAD_BACK_RECHARGE_ORDER:
        	return player->after_load_recharge_order(transaction);
        case CLIENT_ENTER_MAIN_TOWN:
            return player->request_enter_main_town(msg_proto);
        case CLIENT_FUNCTION_NEED_GOLD:
            return player->request_function_need_gold(msg_proto);
        case CLIENT_RENAME_ROLE:
        	return player->request_rename_role_begin(msg_proto);
        case CLIENT_RENAME_LEAGUE:
        	return player->request_rename_league_begin(msg_proto);
        case CLIENT_RESEX_ROLE:
        	return player->request_resex_role_begin(msg_proto);

        // mount
        case CLIENT_FETCH_MOUNT_INFO:
        	return player->fetch_mount_info(msg_proto);
        case CLIENT_MOUNT_EVALUATE:
        	return player->mount_evoluate(msg_proto);
        case CLIENT_TAKEON_MOUNT:
        	return player->takeon_mount(msg_proto);
        case CLIENT_TAKEOFF_MOUNT:
        	return player->takeoff_mount(msg_proto);
        case CLIENT_SELECT_MOUNT_SHAPE:
        	return player->select_mount_shape(msg_proto);
        case CLIENT_MOUNT_USE_GOODS:
        	return player->use_mount_goods(msg_proto);

        case CLIENT_FETCH_GODER_INFO:
        	return player->fetch_goder_info();
        case CLIENT_GODER_UPGRADE:
        	return player->goder_operate();

        case INNER_MAP_LOGIC_WEDDING_INFO:
        	return player->update_player_wedding_info(msg_proto);
        case INNER_ML_ARENA_BUY_TIMES:
          	return player->request_buy_arena_times(msg_proto);
        case INNER_ML_ARENA_BUY_COOL:
          	return player->request_buy_clear_cool(msg_proto);
        case INNER_ML_NOTICE_REWARD:
        	return player->request_notice_add_goods(msg_proto);
        case CLIENT_FETCH_COLLECT_CHESTS_INFO:
        	return player->fetch_collect_chests_info(msg_proto);

        case CLIENT_FETCH_GUIDE_INFO:
            return player->fetch_guide_info();
        case CLIENT_SAVE_GUIDE_INFO:
            return player->save_guide_info(msg_proto);
        case CLIENT_OBTAIN_MAIL_LIST:
        	return player->obtain_mail_list(msg_proto);
        case CLIENT_OBTAIN_MAIL_INFO:
        	return player->obtain_mail_info(msg_proto);
        case CLIENT_PICK_UP_ATTACH_A:
        	return player->pick_up_single_mail(msg_proto);
        case CLIENT_PICK_UP_ALL_ATTACH:
        	return player->pick_up_all_mail();
        case CLIENT_DELETE_MAIL:
        	return player->delete_mail(msg_proto);
        case CLIENT_SEND_MAIL:
        	return player->send_mail(sid, msg_proto);
        case TRANS_SAVE_MAIL_OFFLINE:
        	return player->after_send_mail(transaction);
        case TRANS_LOAD_MAIL_OFFLINE:
        	return player->after_load_mail_offline(transaction);
        case TRANS_LOAD_PLAYER_TO_SEND_MAIL:
        	return player->after_load_player_to_send_mail(transaction);

        case CLIENT_REQUEST_ACHIEVE_PANEL_INFO:
        	return player->request_fetch_achieve_info();
        case CLIENT_DRAW_ACHIEVE_REWARD:
        	return player->get_achieve_reward(msg_proto);

        	//achievement
        case INNER_ML_FINISH_MAP_ACHIEVEMENT:
        	return player->finish_map_achievement(msg_proto);
        case INNER_MAP_MARKET_ONSELL:
        	return player->request_add_onsell_goods(msg_proto);
        case INNER_REQ_PACK_ITEM:
            return player->request_pack_item(msg_proto);

            //藏宝阁
        case CLIENT_FETCH_HIDDEN_TREASURE_INFO:
        	return player->fetch_hi_treasure_info();
        case CLIENT_FETCH_HIDDEN_TREASURE_REWARD:
        	return player->fetch_hi_treasure_reward();
        case CLIENT_HIDDEN_TREASURE_BUY:
        	return player->buy_hi_treasure_item(msg_proto);

        	//task
        case CLIENT_TASK_ABANDON:
        	return player->task_abandon(msg_proto);
        case CLIENT_TASK_ACCEPT:
        	return player->task_accept(msg_proto);
        case CLIENT_TASK_FAST_FINISH:
        	return player->task_fast_finish(msg_proto);
        case CLIENT_TASK_NPC_LIST:
        	return player->task_npc_list(msg_proto);
        case CLIENT_TASK_SUBMIT:
        	return player->task_submit(msg_proto);
        case CLIENT_TASK_TRACK_LIST:
        	return player->task_get_list(msg_proto);
        case CLIENT_TASK_TALK:
        	return player->task_talk(msg_proto);
        case INNER_MAP_TASK_CHECK_NPC:
        	return player->task_after_check_npc(msg_proto);
        case TRANS_UPDATE_PLAYER_NAME:
            return player->after_update_player_name(transaction);
        case CLIENT_FETCH_NOVICE_STEP:
        	return player->request_fetch_novice_step();
        case CLIENT_SET_NOVICE_STEP:
        	return player->request_set_novice_step(msg_proto);
        case INNER_ML_CLIMB_TOWER_TASK_CHECK:
        	return player->check_script_wave_task_finish(msg_proto);
        case CLIENT_FETCH_UIOPEN_STEP:
            return player->request_fetch_uiopen_step();
        case CLIENT_SET_UIOPEN_STEP:
            return player->request_set_uiopen_step(msg_proto);
        case CLIENT_ROUTINE_WORLD_CHAT:
            return player->request_routine_world_chat();
        case CLIENT_ROUTINE_OPEN_MARKET:
            return player->request_routine_open_market();
        case CLIENT_ROUTINE_FRESH_STAR:
            return player->request_refresh_task_star(msg_proto);
        case CLIENT_ROUTINE_FINISH_ALL:
            return player->request_routine_finish_all(msg_proto);

        	//equipment
        case CLIENT_REQUEST_EQUIP_REFINE_INFO:
        	return player->fetch_equip_refine_panel_info(msg_proto);
        case CLIENT_EQUIPMENT_REFINE:
        	return player->equip_strengthen(msg_proto);
        case CLIENT_EQUIPMENT_RED_UPRISING:
        	return player->equip_red_uprising(msg_proto);
        case CLIENT_EQUIP_ORANGE_UPRISING:
            return player->equip_orange_uprising(msg_proto);
        case CLIENT_EQUIPMENT_BRIGHTEN:
        	return player->equip_brighten(msg_proto);
        case CLIENT_EQUIP_GOD_REFINE:
        	return player->equip_god_refine(msg_proto);
        case CLIENT_EQUIP_GOD_REFINE_DIFF:
        	return player->equip_god_refine_diff(msg_proto);
        case CLIENT_PUT_ON_EQUIPMENT:
        	return player->put_on_equip(msg_proto);
        case CLIENT_TAKE_OFF_EQUIPMENT:
        	return player->take_off_equip(msg_proto);
        case CLIENT_REQUEST_MAGICAL_PANEL_INFO:
        	return player->fetch_magical_panel_info();
        case CLIENT_ACTIVATE_MAGICAL_ITEM:
        	return player->activate_magical_item(msg_proto);
        case CLIENT_REQUEST_MAGICAL_DETAIL_INFO:
        	return player->fetch_magical_detail_info(msg_proto);
        case CLIENT_MAGICAL_POLISH:
        	return player->magical_polish(msg_proto);
        case CLIENT_SELECT_MAGICAL_POLISH_RESULT:
        	return player->magical_polish_select_result(msg_proto);
        case CLIENT_MAGPOLISH_CLEAR_SINGLE_RECORD:
        	return player->magical_polish_clear_single_record(msg_proto);
        case CLIENT_EQUIP_INHERIT:	//装备继承
        	return player->equip_inherit(msg_proto);
        case CLIENT_EQUIP_TEMPERED:		//装备淬练
        	return player->equip_tempered(msg_proto);
        case CLIENT_EQUIP_INSERT_JEWEL:		//装备镶嵌宝石
        	return player->equip_insert_jewel(msg_proto);
        case CLIENT_EQUIP_GOOD_REFINE:
        	return player->equip_good_refine(msg_proto);
        case CLIENT_EQUIP_POLISH:			//装备洗练
        	return player->equip_polish(msg_proto);
        case CLIENT_EQUIP_FETCH_POLISH_ATTR:		//装备洗练时点击装备获取其洗练属性
        	return player->equip_polish_fetch_attr_list(msg_proto);
        case CLIENT_EQUIP_POLISH_REPLACE:		//装备洗练时替换洗练属性
        	return player->equip_polish_replace_attr(msg_proto);
        case CLIENT_EQUIP_COMPSE:
        	return player->equip_compose(msg_proto);
        case CLIENT_EQUIP_DECOMPOSE:
        	return player->equip_decompose(msg_proto);
        case CLIENT_EQUIP_REMOVE_JEWEL:
        	return player->equip_remove_jewel(msg_proto);
        case CLIENT_EQUIP_UPGRADE_JEWEL:
        	return player->equip_upgrade_jewel(msg_proto);
        case CLIENT_EQUIP_SUBLIME_JEWEL_INFO:
        	return player->fetch_equip_sublime_jewel_info();
        case CLIENT_EQUIP_SUBLIME_JEWEL:
        	return player->equip_sublime_jewel();
        case CLIENT_FETCH_EQUIP_SMELT_INFO:
        	return player->fetch_equip_smelt_panel_info();
        case CLIENT_CHANGE_SMELT_RECOMMEND:
        	return player->change_smelt_recommend(msg_proto);
        case CLIENT_EQUIP_SMELT:
        	return player->equip_smelt(msg_proto);
		case CLIENT_MOLDING_SPIRIT:
			return player->equip_molding_spirit(msg_proto);
		case CLIENT_MOLDING_SPIRIT_INFO:
			return player->fetch_molding_spirit_info(msg_proto);
		case CLIENT_MOLDING_SPIRIT_ALL_EQUIP_INFO:
			return player->fetch_molding_spirit_all_equip_info();
        	//vip
        case CLIENT_FETCH_VIP_INFO:
        	return player->fetch_vip_info();
		case CLIENT_VIP_GAIN_GIFT:
			return player->gain_vip_gift(msg_proto);
		case CLIENT_FETCH_SUPER_VIP_INFO:
			return player->fetch_super_vip_info_begin();
		case INNER_FETCH_SUPER_VIP_INFO:
			return player->fetch_super_vip_info_end(msg_proto);

		case INNER_UPDATE_DAILY_RECHARGE_INFO:
			return BackDR_SYS->update_player_info(msg_proto);

        	//mall
        case INNER_MAP_MALL_DONATE:
        	return player->map_process_donate_mall_goods(msg_proto);

        	//label
        case CLIENT_REQUEST_LABEL_PANEL_INFO:
        	return player->fetch_label_panel_info(msg_proto);
        case CLIENT_SELECT_LABEL:
        	return player->select_label(msg_proto);
        case CLIENT_CANCEL_LABEL:
        	return player->cancel_label(msg_proto);
        case CLIENT_REQUEST_SINGLE_LABEL_INFO:
        	return player->fetch_single_label_info(msg_proto);
        case CLIENT_CHECK_NEW_LABEL:
            return player->check_new_label(msg_proto);
        case INNER_ML_SYNC_ADD_LABEL:
        	return player->sync_add_label(msg_proto);


        	//illustration
        case CLIENT_REQUEST_ILLUS_PANEL_INFO:
        	return player->fetch_illus_panel_info(msg_proto);
        case CLIENT_SELECT_ILLUS_CLASS:
        	return player->select_illus_class(msg_proto);
        case CLIENT_SELECT_ILLUS_GROUP:
        	return player->select_illus_group(msg_proto);
        case CLIENT_REQUEST_SINGLE_ILLUS_INFO:
        	return player->fetch_single_illus_info(msg_proto);
        case CLIENT_UPGRADE_ILLUS:
        	return player->upgrade_illus_info(msg_proto);
        case CLIENT_SELECT_ILLUS:
        	return player->select_single_illus(msg_proto);
        case INNER_ML_SYNC_ADD_ILLUS:
        	return player->sync_add_illus(msg_proto);

        //offline_rewards
        case CLIENT_FETCH_OFFLINE_INFO:
        	return player->offline_rewards_info();
        case CLIENT_FETCH_OFFLINE_AWARD:
        	return player->fetch_offline_rewards(msg_proto);

        	//online rewards
        case CLIENT_ONLINE_REWARDS_FETCH_AWARDS:
        	return player->fetch_online_rewards();
        case CLIENT_ONLINE_REWARDS_GET_INFO:
        	return player->get_online_rewards_info();

        /*********************** Welfard 福利 */
        // check in, 签到
        case CLIENT_FETCH_CHECK_IN_INFO:
        	return player->fetch_check_in_info(msg_proto);
        case CLIENT_REQUEST_CHECK_IN:
        	return player->request_check_in(msg_proto);
        case CLIENT_REQUEST_CHECK_IN_AGAIN:
        	return player->request_check_in_again(msg_proto);
        case CLIENT_REQUEST_TOTAL_CHECK_IN:
        	return player->request_total_check_in(msg_proto);

        // exp restore 经验找回
        case CLIENT_FETCH_EXP_RESTORE_INFO:
        	return player->fetch_exp_restore_info();
        case CLIENT_RESTORE_EXP_SINGLE:
        	return player->exp_restore_single(msg_proto);
        case CLIENT_RESTORE_EXP_ALL:
        	return player->exp_restore_all(msg_proto);
        case CLIENT_RESTORE_GOODS_INFO:
        	return player->fetch_restore_goods_info(msg_proto);
        case INNER_ML_ER_EVENT_DONE:
        	return player->exp_restore_event_done(msg_proto);
        case INNER_ML_ER_SYNC_STAGE_INFO:
        	return player->sync_storage_stage_info(msg_proto);

        // media gift 媒体礼包
        case CLIENT_USE_ACTI_CODE:
        	return player->use_acti_code_begin(msg_proto);
        case TRANS_FETCH_ACTI_CODE_DETAIL:
        	return player->use_acti_code_after(transaction);
        case CLIENT_FETCH_MEDIA_GIFT_DEF:
        	return player->fetch_media_gift_config();
        case INNER_ML_REQUEST_ACTI_CODE_RETURN:
        	return player->after_query_center_acti_code(msg_proto);

        case CLIENT_QUERY_UPDATE_RES_AWARDS:
        	return player->process_query_update_res_rewards(msg_proto);
        case CLIENT_GET_UPDATE_RES_AWARDS:
        	return player->process_get_update_res_rewards(msg_proto);

        case CLIENT_DISPLAY_WELFARE_ELEMENTS:
        {
        	Proto51401421 respond;
        	player->fetch_display_welfare_elements(msg_proto, &respond);
        	return player->respond_to_client(RETURN_DISPLAY_WELFARE_ELEMENTS, &respond);
        }

        case CLIENT_DOWNLOAD_BOX_GIFT_INFO:
        	return player->fetch_download_box_info();

        	//player assist
        case CLIENT_CANCEL_RED_POINT:
        	return player->cancel_player_assist(msg_proto);
        case CLIENT_FETCH_PLAYER_ASSIST_PANNEL:
        	return player->fetch_player_assist_pannel(msg_proto);
        case CLIENT_FETCH_PLAYER_ASSIST_TIPS:
        	return player->fetch_player_assist_tips(msg_proto);
        case CLIENT_REQUEST_INGORE_SHOW_PANNEL:
        	return player->request_ingore_show_pannel(msg_proto);
        case INNER_NOTIFY_PLAYER_ASSIST_TIPS_EVENT:
        	return player->process_inner_player_assist_tips_event(msg_proto);

        // script
        case CLIENT_CLEAN_SINGLE_SCRIPT:
            return player->request_start_clean_single_script(msg_proto);
        case INNER_SCRIPT_CLEAN_TIMES_INFO:
            return player->process_start_script_clean(msg_proto);
        case INNER_SCRIPT_FIRST_PASS_AWARD:
            return player->process_script_first_pass_award(msg_proto);
        case INNER_SCRIPT_ADD_TIMES_USE_GOLD:
            return player->process_script_add_times_use_gold(msg_proto);
        case INNER_SCRIPT_LIST_INFO:
            return player->process_script_list_info(msg_proto);
        case INNER_SCRIPT_DRAW_PIECE_TOTAL_STAR:
            return player->process_piece_total_star_award(msg_proto);
        case INNER_SCRIPT_OTHER_AWARD:
            return player->process_script_other_award(msg_proto);
        case INNER_MAP_SCRIPT_ENTER_CHECK:
            return player->process_script_enter_check_pack(msg_proto);
        case INNER_MINISCRIPT_PLAYER_LEVEL:
            return player->process_novice_level_up(msg_proto);

        // treasures
        case CLIENT_FETCH_TREASURES_INFO:
        	return player->fetch_treasures_info();
        case CLIENT_FETCH_TREASURES_DICE:
        	return player->player_dice_begin(msg_proto);
        case INNER_PLAY_TREASURES_DICE:
        	return player->fetch_act_mult(msg_proto);
        case INNER_SEND_ACT_DROP:
        	return player->sync_festival_icon(msg_proto);
        case CLIENT_FETCH_TREASURES_RESET:
        	return player->player_reset_treasures_game_begin();
        case INNER_SEND_BIG_ACT_STATE:
        	return player->sync_big_act_state(msg_proto);

        // sword_pool
        case CLIENT_FETCH_SWORD_POOL_INFO:
        	return player->get_spool_info();
        case CLIENT_UPLEVEL_SWORD_POOL:
        	return player->uplevel_sword_pool();
        case CLIENT_FIND_BACK_ONE_TASK:
        	return player->find_back_task_for_one(msg_proto);
        case CLIENT_FIND_BACK_ALL_TASK:
        	return player->find_back_all_task();
        case CLIENT_CHANGE_STYLE_LV:
        	return player->change_spool_style_lv(msg_proto);
        case INNER_UPDATE_SWORD_POOL_TASK:
        	return player->update_task_info(msg_proto);

        // fashion
        case CLIENT_FETCH_FASHION_INFO:
        	return player->request_fetch_fashion_info();
        case CLIENT_FASHION_ADD_COLOR:
        	return player->request_fashion_add_color(msg_proto);
        case CLIENT_CHANGE_FASHION_ID:
        	return player->request_change_fashion_id(msg_proto);
        case CLIENT_CHANGE_FASHION_STYLE:
        	return player->request_change_fashion_style(msg_proto);

        // transfer
        case CLIENT_FETCH_SPIRIT_INFO:
        	return player->request_spirit_info();
        case CLIENT_MAKE_SPIRIT:
        	return player->request_make_spirit(msg_proto);
        case CLIENT_UP_STAGE:
        	return player->request_up_stage();
        case CLIENT_FETCH_TRANSFER_INFO:
        	return player->request_transfer_info();
        case CLIENT_CHANGE_TRANSFER_ID:
        	return player->request_change_transfer_id(msg_proto);
        case CLIENT_USER_TRANSFER:
        	return player->request_use_transfer();
        case CLIENT_GET_OPEN_REWARD:
        	return player->fetch_open_reward();
        case CLIENT_BUY_TRANSFER:
        	return player->request_buy_transfer(msg_proto);

        case CLIENT_FETCH_RECHARGE_ACTI_INFO:
        	return player->fetch_recharge_awards_info();
        case CLIENT_FETCH_RECHARGE_AWARDS:
        	return player->fetch_recharge_awards();
        case CLIENT_FETCH_DAILY_RECHARGE_INFO:
        	return player->fetch_daily_recharge_info();
        case CLIENT_FETCH_DAILY_RECHARGE_REWARDS:
        	return player->fetch_daily_recharge_rewards(msg_proto);
        case CLIENT_FETCH_REBATE_RECHARGE_INFO:
        	return player->fetch_rebate_recharge_info();
        case CLIENT_FETCH_REBATE_RECHARGE_REWARDS:
        	return player->fetch_rebate_recharge_rewards();
        case CLIENT_FETCH_INVEST_RECHARGE_INFO:
        	return player->fetch_invest_recharge_info();
        case CLIENT_FETCH_INVEST_RECHARGE_REWARDS:
        	return player->fetch_invest_recharge_rewards(msg_proto);
        case CLEINT_FETCH_RECHARGE_INFO:
        	return player->fetch_recharge_info(msg_proto);

        case INNER_SKILL_LEVEL_UP:
            return player->process_skill_level_up(msg_proto);
        case CLIENT_FETCH_TOTAL_RECHARGE_REWARDS:
        	return player->get_total_recharge_reward(msg_proto);

		// 结婚系统
		case INNER_WEDDING_CHECK_PACK:
			return player->process_wedding_check_pack(msg_proto);
		case INNER_DIVORCE_CHECK_PACK:
			return player->process_divorce_check_pack(msg_proto);
		case INNER_KEEPSAKE_UPGRADE:
			return player->process_keepsake_upgrade_check_pack(msg_proto);
		case INNER_INTIMACY_AWARD_CHECK:
			return player->process_intimacy_award_check(msg_proto);
		case INNER_SYNC_DIVORCE_LABEL:
			return player->process_divorce_clear_label();
		case INNER_PRESENT_FLOWER:
			return player->process_send_flower(msg_proto);

		//返利活动购买/兑换扣除物品
		case INNER_RETURN_ACTIVITY_COST_ITEM:
			return player->return_activity_cost_item(msg_proto);

		case INNER_ML_LUCKY_WHEEL_COST:
			return player->lucky_wheel_activity_cost(msg_proto);
		case INNER_ML_LUCKY_WHEEL_RESET:
			return player->lucky_wheel_activity_reset(msg_proto);

		case INNER_ML_WEDDING_UPDATE:
			return player->wedding_update_work(msg_proto);
		case INNER_ML_WEDDING_BUY_TREASURES:
			return player->wedding_buy_treasures(msg_proto);

		case INNER_MAY_ACT_BUY:
			return player->may_activity_buy(msg_proto);
		case INNER_FETCH_ITEM_AMOUNT_INFO:
        	return player->fetch_item_amount_info(msg_proto);
		case INNER_GODDESS_BLESS_REWARD:
			return player->goddess_bless_reward(msg_proto);

		case INNER_ACT_BUY_ITEM:
			return player->may_act_buy_many_item(msg_proto);
		case INNER_LUCKY_WHEEL_ALL_COST:
			return player->special_box_cost(msg_proto);
		//交易
		case CLIENT_TRADE_SPONSOR:
			return player->request_org_trade(msg_proto);
		case INNER_REQUEST_IS_LIMIT_TO_LOGIC:
			return player->trade_condition_satisfy(msg_proto);
		case CLIENT_TRADE_INVITE_RESPOND:
			return player->trade_respond_invite(msg_proto);
		case CLIENT_TRADE_CLOSE_PANEL:
		    return player->trade_close_panel();
		case CLIENT_TRADE_PUSH_ITEMS:
			return player->trade_push_items(msg_proto);
		case CLIENT_TRADE_POP_ITEMS:
			return player->trade_pop_items(msg_proto);
		case CLIENT_TRADE_LOCK_PANEL:
			return player->trade_lock_panel();
		case CLIENT_TRADE_START:
			return player->trade_start();
		case INNER_NOTIFY_TRADE_CANCEL:
			return player->inner_notify_trade_cancel(CANCEL_NOT_DEFAULT);

		//法宝
		case CLIENT_REQUEST_MAGIC_WEAPON:
			return player->magic_weapon_fetch_info();
		case CLIENT_ACTIVE_MAGIC_WEAPON:
			return player->magic_weapon_active(msg_proto);
		case CLIENT_PROMOTE_RANK_MAGIC_WEAPON:
			return player->magic_weapon_promote_rank(msg_proto);
		case CLIENT_PROMOTE_QUALITY_MAGIC_WEAPON:
			return player->magic_weapon_promote_quality(msg_proto);
		case CLIENT_ADORN_MAGIC_WEAPON:
			return player->magic_weapon_adorn(msg_proto);

        case INNER_ML_LUCKY_TABLE_COST:
            return player->lucky_table_cost(msg_proto);

        // 离线挂机
		case CLIENT_GET_OFFLINEHOOK_INFO:
			return player->player_request_offlineHook(msg_proto);
		case CLIENT_OFFLINEHOOK_DRUG_USE:
			return player->player_request_applyofflineHook(msg_proto);
		case CLIENT_OFFLINEHOOK_REWARDGET:
			return player->player_replay_offlinereward(msg_proto);
		case CLIENT_OFFLINE_PLUS_REWARD:
			return player->use_offline_plus_item(msg_proto);
		case CLIENT_OFFLINE_PLUS_INFO:
			return player->fetch_offline_plus_info();

        // 后台活动
        case INNER_BACK_ACT_REWARD_ITEM:
            return player->process_back_act_reward_item(msg_proto);

        // 华山论剑
        case CLIENT_TRVL_BATTLE_MAIN_PANNEL:
            return player->request_tbattle_main_pannel(msg_proto);
        case INNER_TRVL_REWARD_ID:
            return player->process_trvl_reward(msg_proto);

        case INNER_CHECK_MONEY_CREATE_TRAVTEAM:
        	return player->process_check_money_create_trav_team(msg_proto);

        default:
            MSG_USER("ERROR can't recongize map logic message %d %d %d", sid, recogn, len);
            if (transaction != NULL)
            {
            	transaction->rollback();
            	transaction = NULL;
            }
            return -1;
    }

    return 0;
}

int MapLogicUnit::process_stop(void)
{
    MAP_MONITOR->logout_logic_all_player();
    return BaseUnit::process_stop();
}

int MapLogicUnit::handle_with_noplayer_online(MapLogicPlayer* player, UnitMessage *unit_msg)
{
	if (player != NULL)
	{
		/*玩家在线*/
		switch(unit_msg->__msg_head.__recogn)
		{
        case INNER_MAP_MARKET_BUY:
        	return player->market_buy_goods(unit_msg->proto_msg());
		}
	}
	else
	{
		MAP_MONITOR->dispatch_to_logic(unit_msg->proto_msg(), unit_msg->__sid,
				unit_msg->__msg_head.__role_id);
	}

	return 0;
}

