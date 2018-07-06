/*
 * MLOnceRewards.cpp
 *
 *  Created on: Aug 18, 2014
 *      Author: jinxing
 */
#include "MLOnceRewards.h"
#include <mongo/client/dbclient.h>
#include "MMOLabel.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"

void OnceRewardRecords::reset(void)
{
	this->__update_res_flag = 0;
}

MLOnceRewards::MLOnceRewards(void)
{
}

MLOnceRewards::~MLOnceRewards(void)
{
}

void MLOnceRewards::reset(void)
{
	this->records_.reset();
}

OnceRewardRecords &MLOnceRewards::once_reward_records(void)
{
	return this->records_;
}

int MLOnceRewards::process_query_update_res_rewards(Message *msg)
{
	Proto51401441 respond;
	respond.set_has_reward(this->records_.__update_res_flag == 0);
	FINER_PROCESS_RETURN(RETURN_QUERY_UPDATE_RES_AWARDS, &respond);
}

int MLOnceRewards::process_get_update_res_rewards(Message *msg)
{
	CONDITION_NOTIFY_RETURN(this->records_.__update_res_flag == 0,
			RETURN_QUERY_UPDATE_RES_AWARDS, ERROR_CLIENT_OPERATE);

	int ret = this->add_reward(CONFIG_INSTANCE->const_set("down_load_reward"),
			ADD_FROM_UPDAET_RES);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_QUERY_UPDATE_RES_AWARDS, ret);

	this->records_.__update_res_flag = 1;
	FINER_PROCESS_NOTIFY(RETURN_QUERY_UPDATE_RES_AWARDS);
}

int MLOnceRewards::sync_transfer_once_rewards(int scene_id)
{
	BSONObj bson_obj;
	MMOWelfare::mmo_once_rewards_dump_to_bson(this->map_logic_player(), bson_obj);

	Proto31401404 msg;
	msg.set_obj_type(GameEnum::MAP_LOGIC_ONCE_REWARDS);
	msg.set_bson_raw_data(bson_obj.objdata(), bson_obj.objsize());

	return this->send_to_other_logic_thread(scene_id, msg);
}

int MLOnceRewards::read_transfer_once_rewards(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401404*, request, -1);

	BSONObj bson_obj(request->bson_raw_data().c_str());
	return MMOWelfare::mmo_once_rewards_load_from_bson(bson_obj, this->map_logic_player());
}
