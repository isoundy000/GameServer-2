/*
 * AIDropPack.h
 *
 *  Created on: Jan 16, 2014
 *      Author: peizhibi
 */

#ifndef AIDROPPACK_H_
#define AIDROPPACK_H_

#include "AIStruct.h"
#include "GameMover.h"

class AIDropPack : public GameMover
{
public:
	class AIDropTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);

		AIDropTimer();
		AIDropPack* drop_pack_;
	};

	class RecycleTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);

		AIDropPack* drop_pack_;
	};

public:
	AIDropPack();
	virtual ~AIDropPack();

    virtual int client_sid(void);
    virtual int64_t entity_id(void);

    virtual int enter_scene(const int type = ENTER_SCENE_LOGIN);
    virtual int exit_scene(const int type = EXIT_SCENE_LOGOUT);

    virtual int make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate = false);

public:
    int drop_id();

	int reset();
	int recycle_self();

	int is_have_goods();
	int pick_up_shout_drop_goods(MapPlayer *player);

	int validate_pick_up(Int64 role_id);
	int set_pick_up_prep(Int64 role_id);

	int pick_up_suceess();
	int pick_up_failure(Int64 role_id);

	int make_up_money_info(Proto31400013* pickup_info);
	int make_up_item_info(Proto31400013* pickup_info);

	int sign_and_enter_scene(const MoverCoord& ai_coord, Scene* scene);
	AIDropDetail& drop_detail();

    void calc_pickup_protect_tick(const Json::Value &json);
    void calc_recycle_tick(const Json::Value &json, Scene *scene);

private:
	AIDropTimer drop_timer_;
	RecycleTimer recycle_timer_;

	AIDropDetail drop_detail_;
};

#endif /* AIDROPPACK_H_ */
