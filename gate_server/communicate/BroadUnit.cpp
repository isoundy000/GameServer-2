/*
 * BroadUnit.cpp
 *
 * Created on: 2014-09-26 10:50
 *     Author: lyz
 */

#include "BroadUnit.h"
#include "GateMonitor.h"

int BroadUnit::type(void)
{
    return GATE_BROAD_UNIT;
}

UnitMessage *BroadUnit::pop_unit_message(void)
{
    return GATE_MONITOR->unit_msg_pool()->pop();
}

int BroadUnit::push_unit_message(UnitMessage *msg)
{
    return GATE_MONITOR->unit_msg_pool()->push(msg);
}

int BroadUnit::process_block(UnitMessage *unit_msg)
{
    JUDGE_RETURN(this->dispatch_by_broad_in_gate(unit_msg) == false, 0);
    JUDGE_RETURN(this->dispatch_by_broad_client(unit_msg) == false, 0);

    return -1;
}

bool BroadUnit::dispatch_by_broad_in_gate(UnitMessage *unit_msg)
{
    JUDGE_RETURN(unit_msg->__route_head.__broad_type == BT_BROAD_IN_GATE, false);

	Block_Buffer *buff = unit_msg->data_buff();
	int *data_len = 0, *mover_size = 0;

	data_len = (int *)(buff->get_read_ptr());
	mover_size = (int *)(buff->get_read_ptr() + sizeof(int32_t) + *data_len);

	int body_index = buff->get_read_idx() + sizeof(int32_t);
	int body_end_index = body_index + *data_len;

	buff->set_read_idx(buff->get_read_idx() + sizeof(int32_t) * 2 + *data_len);

    int conf_size = CONFIG_INSTANCE->const_set("broad_size");
    if (conf_size <= 0)
    {
    	conf_size = 150;
    }

	int64_t mover_id = 0;
	int offset = 0;
    int client_sid = 0;

	int max_size = std::min<int>(conf_size, *mover_size);
	for (int i = 0; i < max_size; ++i)
	{
        buff->read_int64(mover_id);
        buff->read_int32(offset);
        client_sid = GATE_MONITOR->find_sid_by_role_id(mover_id);
        if (client_sid >= 0)
        {
            int org_read_idx = buff->get_read_idx(), org_write_idx = buff->get_write_idx();
            buff->set_read_idx(body_index + offset);
            buff->set_write_idx(body_end_index);
            if (buff->readable_bytes() > 0)
            	GATE_MONITOR->client_sender(client_sid)->push_data_block_with_len(client_sid, *buff);
            buff->set_write_idx(org_write_idx);
            buff->set_read_idx(org_read_idx);
        }
        else
        {
        	MSG_USER("ERROR no broad role %ld", mover_id);
        }
	}
	return true;
}


bool BroadUnit::dispatch_by_broad_client(UnitMessage *unit_msg)
{
	JUDGE_RETURN(unit_msg->__route_head.__broad_type == BT_BROAD_CLIENT, false);

	int role_sid = GATE_MONITOR->find_sid_by_role_id(unit_msg->__route_head.__role_id);
	if (role_sid >= 0)
	{
		if (unit_msg->data_buff() != 0)
		{
			int32_t *client_sid = (int32_t *)(unit_msg->data_buff()->get_read_ptr());
			*client_sid = role_sid;
			if (GATE_MONITOR->client_sender(role_sid)->push_pool_block_with_len(unit_msg->data_buff()) == 0)
				unit_msg->reset();
		}
	}
	else
	{
		MSG_USER("ERROR no broad role %ld", unit_msg->__route_head.__role_id);
	}

	return true;
}
