/*
 * GateUnit.cpp
 *
 * Created on: 2013-04-15 16:11
 *     Author: lyz
 */

#include "GateUnit.h"
#include "GateMonitor.h"

#include "TransactionMonitor.h"
#include "Transaction.h"

#include "DaemonServer.h"
#include "GameCommon.h"

int GateUnit::type(void)
{
    return GATE_UNIT;
}

UnitMessage *GateUnit::pop_unit_message(void)
{
    return GATE_MONITOR->unit_msg_pool()->pop();
}

int GateUnit::push_unit_message(UnitMessage *msg)
{
    return GATE_MONITOR->unit_msg_pool()->push(msg);
}

int GateUnit::process_block(UnitMessage *unit_msg)
{
	JUDGE_RETURN(this->dispatch_by_noplayer(unit_msg) == false, 0);

    int sid = unit_msg->__sid, recogn = unit_msg->__msg_head.__recogn,
        trans_id = unit_msg->__msg_head.__trans_id;
    int64_t role_id = unit_msg->__msg_head.__role_id;

    Block_Buffer *msg = unit_msg->data_buff();
    int ivalue = unit_msg->ivalue();
    Message *msg_proto = unit_msg->proto_msg();

    Transaction *transaction = 0;
    if (trans_id > 0)
    	TRANSACTION_MONITOR->find_transaction(trans_id, transaction);

    // 一般不能在此处进行网关消息处理,不然会话验证无效
    switch (recogn)
    {
        case CLIENT_SERVER_KEEP_ALIVE:
            return GATE_MONITOR->check_client_accelerate(sid);
        case INNER_TIMER_TIMEOUT:
            return POOL_MONITOR->game_timer_timeout(ivalue);
        case INNER_UPDATE_CONFIG:
        {
            DAEMON_SERVER->request_update_config();
            GATE_MONITOR->check_update_trvl_url();
            return 0;
        }
        case INNER_RELOGIN_LOGIC:
            return GATE_MONITOR->update_relogin_tick();
        case INNER_SYNC_SEESION:
            return GATE_MONITOR->sync_session(sid, msg);
        case INNER_GATE_ROLEAMOUNT:
            return GATE_MONITOR->fetch_role_amount(sid, msg);
        case INNER_FORCE_GATE_LOGOUT:
            return GATE_MONITOR->force_player_logout(msg);
        case INNER_CONNECT_SVC_CLOSE:
            return GATE_MONITOR->process_connect_svc_close(msg);
        case INNER_CONNECT_SERVER:
            return GATE_MONITOR->process_connected_svc(msg_proto);
#ifndef NO_USE_PROTO
        case CLIENT_GATE_SESSION:
            return GATE_MONITOR->validate_gate_session(sid, msg_proto);
        case CLIENT_CREATE_ROLE:
            return GATE_MONITOR->create_new_role(sid, msg_proto);
#else
        case CLIENT_GATE_SESSION:
            return GATE_MONITOR->validate_gate_session(sid, msg);
        case CLIENT_CREATE_ROLE:
            return GATE_MONITOR->create_new_role(sid, msg);
#endif
        case CLIENT_RANDOM_NAME:
            return GATE_MONITOR->select_random_name(sid, msg_proto);
        case TRANS_FETCH_RAND_NAME:
            return GATE_MONITOR->after_select_random_name(transaction);
        case TRANS_LOAD_GATE_PLAYER:
            return GATE_MONITOR->after_load_player(transaction);
        case TRANS_SAVE_NEW_ROLE:
            return GATE_MONITOR->after_save_new_role(transaction);
        case INNER_MAP_TRAVEL_MONGO_SAVE:
            return GATE_MONITOR->process_travel_mongo_save(sid, msg_proto);
        case INNER_GATE_SAVE_SERIAL_LOG:
        	return GATE_MONITOR->process_travel_serial_save(sid, msg_proto);
        case INNER_FETCH_TRAVEL_AREA:
            return GATE_MONITOR->process_set_travel_area(sid, msg_proto);
        case INNER_MAP_TRVL_KEEP_ALIVE:
        	return GATE_MONITOR->process_travel_map_keep_alive(sid, msg_proto);
        case TRANS_LOAD_SHOP_MODE:
        	return GATE_MONITOR->db_load_mode_done(transaction);
        default:
            break;
    }

    GatePlayer *player = 0;
    if (recogn / 10000000 == 1 && unit_msg->__route_head.__inner_req == 0)
    {
		if (GATE_MONITOR->find_player_by_sid(sid, player) != 0)
		{
			MSG_USER("ERROR sid no gate player %d %d", sid, recogn);

            if (transaction != NULL)
            {
            	transaction->rollback();
            	transaction = NULL;
            }
			return -1;
		}
		else
		{
			unit_msg->__msg_head.__role_id = player->role_id();
		}
    }
    else
    {
		if (GATE_MONITOR->find_player(role_id, player) != 0)
		{
			if (recogn != ACTIVE_MOVE_DISAPPEAR)
			{
				MSG_USER("ERROR no gate player %ld %d", role_id, recogn);
			}

            if (transaction != NULL)
            {
            	transaction->rollback();
            	transaction = NULL;
            }

			this->process_back(unit_msg);
			return -1;
		}
    }

    switch (recogn)
    {
        case CLIENT_START_GAME:
        	return 0;
//            return player->start_game();
        case CLIENT_STOP_GAME:
            return player->stop_game(GateMonitor::SOCK_DISCOUNT);
        case INNER_SYNC_ROLE_NAME:
            if (unit_msg->__route_head.__broad_type > 0)
                return GATE_MONITOR->dispatch_to_server(unit_msg);
            return player->sync_update_player_name(msg_proto);
        case INNER_SYNC_ROLE_INFO:
            if (unit_msg->__route_head.__broad_type > 0)
                return GATE_MONITOR->dispatch_to_server(unit_msg);
            else
                return player->sync_role_info(msg_proto);
        default:
            return GATE_MONITOR->dispatch_to_server(unit_msg);
    }

    return 0;
}

bool GateUnit::dispatch_by_noplayer(UnitMessage *unit_msg)
{
	// 优化内存复制的次数
	JUDGE_RETURN(unit_msg->__route_head.__broad_type == BT_NOPLAYER_TARGET_SCENE, false);

	if (unit_msg->__route_head.__line_id != -1)
	{
		int sid = GATE_MONITOR->connect_scene_to_sid(unit_msg->__route_head.__scene_id,
				unit_msg->__route_head.__line_id);

		MSG_USER("%d, %d, %d", sid, unit_msg->__route_head.__scene_id,
				unit_msg->__route_head.__line_id);

		if (sid != -1)
		{
			int32_t *client_sid = (int32_t *)(unit_msg->data_buff()->get_read_ptr());
			*client_sid = sid;
			if (GATE_MONITOR->connect_sender()->push_pool_block_with_len(unit_msg->data_buff()) == 0)
				unit_msg->reset();
		}

		return true;
	}

	if (GameCommon::is_travel_scene(unit_msg->__route_head.__scene_id))
	{
		int line_id = GATE_MONITOR->fetch_line_id_by_scene(0, unit_msg->__route_head.__scene_id);

		int sid = GATE_MONITOR->connect_scene_to_sid(unit_msg->__route_head.__scene_id, line_id);
		if (sid > 0)
		{
			int32_t *client_sid = (int32_t *)(unit_msg->data_buff()->get_read_ptr());
			*client_sid = sid;
			if (GATE_MONITOR->connect_sender()->push_pool_block_with_len(unit_msg->data_buff()) == 0)
				unit_msg->reset();
		}
		return true;
	}

	IntVec sid_vc;
	if (GATE_MONITOR->fetch_sid_of_all_line(unit_msg->__msg_head.__scene_id, sid_vc) == 0
			&& sid_vc.empty() == false)
	{
		if (sid_vc.size() == 1)
		{
			int32_t *client_sid = (int32_t *)(unit_msg->data_buff()->get_read_ptr());
			*client_sid = *(sid_vc.begin());
			if (GATE_MONITOR->connect_sender()->push_pool_block_with_len(unit_msg->data_buff()) == 0)
				unit_msg->reset();
		}
		else
		{
			for (IntVec::iterator iter = sid_vc.begin(); iter != sid_vc.end(); ++iter)
			{
				Block_Buffer *buff = GATE_MONITOR->pop_block(0);
				buff->copy(unit_msg->data_buff());
				int32_t *client_sid = (int32_t *)(buff->get_read_ptr());
				*client_sid = *iter;
				if (GATE_MONITOR->connect_sender()->push_pool_block_with_len(buff) != 0)
					GATE_MONITOR->push_block(buff, 0);
			}
		}
	}

	return true;
}

bool GateUnit::process_back(UnitMessage* unit_msg)
{
	JUDGE_RETURN(unit_msg->__route_head.__broad_type == BT_DIRECT_TARGET_SCENE_BACK, false);

	ProtoHead head = unit_msg->__msg_head;

	head.__scene_id = SCENE_LOGIC;
	GATE_MONITOR->dispatch_to_scene(&head, unit_msg->data_buff());

	return true;
}

int GateUnit::process_stop(void)
{
    GATE_MONITOR->logout_all_player();
    return BaseUnit::process_stop();
}


int ConnectUnit::type(void)
{
    return GATE_CONNECT_UNIT;
}

void ConnectUnit::stop_wait(void)
{
    // 清除连接请求，否则可能会导致停止时进程卡住
    UnitMessage *msg = NULL;
    while (this->unit_msg_queue_.size() > 0)
    {
        this->unit_msg_queue_.pop((void *&) msg);
        this->push_message_data(*msg);
        this->push_unit_message(msg);
        msg = NULL;
    }
    BaseUnit::stop_wait();
}

UnitMessage *ConnectUnit::pop_unit_message(void)
{
    return GATE_MONITOR->unit_msg_pool()->pop();
}

int ConnectUnit::push_unit_message(UnitMessage *msg)
{
    return GATE_MONITOR->unit_msg_pool()->push(msg);
}

int ConnectUnit::process_block(UnitMessage *unit_msg)
{
    int recogn = unit_msg->__msg_head.__recogn;
//    Block_Buffer *msg = unit_msg->data_buff();
//    int ivalue = unit_msg->ivalue();
    Message *msg_proto = unit_msg->proto_msg();

    switch (recogn)
    {
        case INNER_CONNECT_SERVER:
            return GATE_MONITOR->process_connect_server(msg_proto);
        default:
            break;
    }

    return 0;
}

int ConnectUnit::dispatch_to_gate_unit(const Message &msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto.GetTypeName());
    Message *request = create_message(msg_proto.GetTypeName());
    JUDGE_RETURN(request != NULL, 0);

    request->CopyFrom(msg_proto);

    UnitMessage unit_msg;
    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = 0;
    unit_msg.set_msg_proto(request);

    if (GATE_MONITOR->logic_unit()->push_request(unit_msg) != 0)
    {
    	if (request != 0)
    		delete request;
    }
    return 0;
}

