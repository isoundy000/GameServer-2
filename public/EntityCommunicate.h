/*
 * EntityCommunicate.h
 *
 * Created on: 2013-02-20 15:10
 *     Author: glendy
 */

#ifndef _ENTITYCOMMUNICATE_H_
#define _ENTITYCOMMUNICATE_H_

#include "GameHeader.h"

class EntityCommunicate
{
public:
	virtual ~EntityCommunicate(void);

//    virtual int respond_to_client(const int recogn, Block_Buffer *buff);
//    virtual int respond_to_client_error(const int recogn, const int error, Block_Buffer *buff);

    virtual int respond_to_client(const int recogn, const Message *msg_proto = 0);
    virtual int respond_to_client_error(const int recogn, const int error, const Message *msg_proto = 0);

    virtual int respond_to_client(Block_Buffer *buff);

    virtual int64_t entity_id(void) = 0;
    virtual int gate_sid(void);

    virtual int entry_id_low(void);
    virtual int entry_id_high(void);

    virtual const char* name();

    int set_last_error(int last_error);
    int get_last_error();

    int notify_player_skill(FighterSkill* skill);
    int validate_operate_tick(int type = GameEnum::NORMAL_OPERATE);
    int add_validate_operate_tick(double add_tick = 1, int type = GameEnum::NORMAL_OPERATE);

    int make_up_client_block(Block_Buffer *buff, const ProtoClientHead *head, const Message *msg_proto);
    int make_up_broad_block(Block_Buffer *buffer, const ProtoHead *head, const Message *msg_proto);
    int make_up_gate_block(Block_Buffer *buff, const ProtoHead *head, const Message *msg_proto);

    void notify_tips_info(int msg_id, ...);
    void notify_one_tips_info(int type, int id, int amount = 1);
    void notify_red_point(int even_id, int even_value = 0);

    void reset_entity();
    void announce(int shout_id, BrocastParaVec& para_vec);

    void notify_server_activity_info();
    void sync_open_activity_info(int first_type, const SubObj& sub = SubObj());

private:
    int last_error_;
    Time_Value operate_tick_[GameEnum::TOTAL_OPERATE];
};

#endif //_ENTITYCOMMUNICATE_H_
