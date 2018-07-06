/*
 * ChatUnit.cpp
 *
 * Created on: 2013-01-19 14:49
 *     Author: glendy
 */

#include "ChatUnit.h"
#include "ChatMonitor.h"
#include "PoolMonitor.h"
#include "TransactionMonitor.h"
#include "DaemonServer.h"
#include "ChannelAgency.h"

int ChatUnit::type(void)
{
    return CHAT_UNIT;
}

UnitMessage *ChatUnit::pop_unit_message(void)
{
    return CHAT_MONITOR->unit_msg_pool()->pop();
}

int ChatUnit::push_unit_message(UnitMessage *msg)
{
    return CHAT_MONITOR->unit_msg_pool()->push(msg);
}

int ChatUnit::process_block(UnitMessage *unit_msg)
{
	int32_t sid = unit_msg->__sid, recogn = unit_msg->__msg_head.__recogn,
			trans_id = unit_msg->__msg_head.__trans_id;
	int64_t role_id = unit_msg->__msg_head.__role_id;

	Message *msg_proto = unit_msg->proto_msg();
	int ivalue = unit_msg->ivalue();

	Transaction *transaction = 0;
	if (trans_id > 0)
	{
		TRANSACTION_MONITOR->find_transaction(trans_id, transaction);
	}

	switch (recogn)
	{
	case CLIENT_SERVER_KEEP_ALIVE:
	{
		return 0;
	}
	case INNER_TIMER_TIMEOUT:
	{
		return POOL_MONITOR->game_timer_timeout(ivalue);
	}
	case INNER_UPDATE_CONFIG:
	{
		return DAEMON_SERVER->request_update_config();
	}
	case INNER_CHAT_LOGIN:
	{
		MSG_USER("CHAT sign in");
		ChatPlayer *player = 0;
		if (CHAT_MONITOR->find_player(role_id, player) == 0)
		{
			return player->resign(role_id, msg_proto);
		}
		else
		{
			player = CHAT_MONITOR->player_pool()->pop();
			if (player == 0)
			{
				MSG_USER("ERROR memory null");
				return -1;
			}
			if (player->sign_in(role_id, msg_proto) != 0)
			{
				CHAT_MONITOR->player_pool()->push(player);
				return -1;
			}
			return 0;
		}
	}
	case CLIENT_CHAT_LOGIN:
	{
		return CHAT_MONITOR->client_login_chat(sid, msg_proto);
	}
	case TRANS_CHAT_LOAD_LEAGUE_HISTORY:
	{
		return CHAT_MONITOR->channel_agency()->load_league_history(role_id);
	}
	case TRANS_CHAT_LOAD_PRIVATE_HISTORY:
	{
		return CHAT_MONITOR->channel_agency()->load_private_history(role_id);
	}
	case TRANS_CHAT_LOAD_LOUDSPEAKER:
	{
		return CHAT_MONITOR->channel_agency()->load_loudspeaker();
	}
	case TRANS_CHAT_SAVE_LOUDSPEAKER:
	{
		return CHAT_MONITOR->channel_agency()->save_loudspeaker();
	}
	case TRANS_LOAD_SHOP_MODE:
	{
		return CHAT_MONITOR->db_load_mode_done(transaction);
	}
	case INNER_CHAT_NEW_ANNOUNCE_WORLD:
	{
		return CHAT_MONITOR->announce_world(msg_proto);
	}
	case INNER_CHAT_BACKSTAGE_ANNOUNCE:
	{
		return CHAT_MONITOR->back_stage_push_system_announce(msg_proto);
	}

	case TRANS_CHAT_SAVE_PRIVATE_HISTORY:
	{
		CHAT_MONITOR->after_db_opera_reset_base_channel(CHANNEL_PRIVATE, role_id);
		return 0;
	}
	case TRANS_CHAT_SAVE_LEAGUE_HISTORY:
	{
		CHAT_MONITOR->after_db_opera_reset_base_channel(CHANNEL_LEAGUE, role_id);
		return 0;
	}
	default:
		break;
	}

	ChatPlayer *player = 0;
	if ((recogn / 10000000) == 1)
	{
		if (CHAT_MONITOR->find_sid_player(sid, player) != 0
				|| player->is_active() == false)
		{
			MSG_USER("ERROR chat sid player no login %d %d", recogn, role_id);
			return -1;
		}
	}
	else
	{
		if (CHAT_MONITOR->find_player(role_id, player) != 0
				|| player->is_active() == false)
		{
			MSG_USER("ERROR chat player no login %d %ld", recogn, role_id);
			return -1;
		}
	}

	switch (recogn)
	{
		case CLIENT_CHAT_NORMAL:
			return player->chat_normal(msg_proto);
		case CLIENT_CHAT_PRIVATE:
			return player->chat_private(msg_proto);
		case CLIENT_CHAT_GET_LEAGUE_HISTORY:
			return player->chat_get_league_history(msg_proto);
		case CLIENT_CHAT_GET_PRIVATE_HISTORY:
			return player->chat_get_private_history(msg_proto);
		case CLIENT_CHAT_GET_VOICE:
			return player->chat_get_voice(msg_proto);
		case CLIENT_CHAT_GET_FLAUNT_RECORD:
			return player->chat_get_flaunt_record(msg_proto);

////////////////////////////////////////////////////////////////
		case INNER_CHAT_LOGOUT:
			return player->sign_out();
		case CLIENT_CHAT_CLIENT_CLOSE:
			return player->client_close();
		case INNER_CHAT_LOUDSPEAKER:
			return player->chat_loudspeaker(msg_proto);
		case INNER_CHAT_ESTABLISH_TEAM:
			return player->establish_team(msg_proto);
		case INNER_CHAT_DIMISS_TEAM:
			return player->dismiss_team(msg_proto);
		case INNER_CHAT_JOIN_TEAM:
			return player->join_team(msg_proto);
		case INNER_CHAT_LEAVE_TEAM:
			return player->leave_team(msg_proto);
		case INNER_CHAT_ESTABLISH_LEAGUE:
			return player->establish_league(msg_proto);
		case INNER_CHAT_JOIN_LEAGUE:
			return player->join_league(msg_proto);
		case INNER_CHAT_DIMISS_LEAGUE:
			return player->dismiss_league(msg_proto);
		case INNER_CHAT_LEAVE_LEAGUE:
			return player->leave_league(msg_proto);
		case INNER_CHAT_ADD_BLACK_LIST:
			return player->add_black_list(msg_proto);
		case INNER_CHAT_REMOVE_BLACK_LIST:
			return player->remove_black_list(msg_proto);
		case INNER_CHAT_ADD_FRIEND_LIST:
			return player->add_friend_list(msg_proto);
		case INNER_CHAT_REFRESH_VIP_STATUS:
			return player->refrsh_vip_status(msg_proto);
		case INNER_CHAT_REMOVE_FRIEND_LIST:
			return player->remove_friend_list(msg_proto);
		case INNER_CHAT_SYNC_ROLE_LEVEL:
			return player->sync_role_level(msg_proto);
		case INNER_CHAT_SYNC_SPEAK_STATE:
			return player->sync_role_speak_state(msg_proto);
		case INNER_CHAT_NEW_ANNOUNCE_PLAYER:
			return player->announce_with_player(msg_proto);

		case INNER_CHAT_FLAUNT:
			return player->flaunt_dispatch(msg_proto);

        case INNER_SYNC_ROLE_NAME:
            return player->sync_update_player_name(msg_proto);
        case INNER_SYNC_ROLE_SEX:
        	return player->sync_update_player_sex();

		default:
		{
			MSG_USER("ERROR can't recognize recogn %d %d %ld", sid, recogn,role_id);
			return -1;
		}
	}

	return 0;
}

int ChatUnit::process_stop(void)
{
    CHAT_MONITOR->logout_all_player();
    return BaseUnit::process_stop();
}


