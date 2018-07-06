/*
 * FloatAI.h
 *
 * Created on: 2015-06-09 10:29
 *     Author: lyz
 */

#ifndef _FLOATAI_H_
#define _FLOATAI_H_

#include "GameAI.h"

class FloatAI : public GameAI
{
public:
    enum {
        FTYPE_FLOAT = 1,        //花车
        FTYPE_GIFTBOX = 2,      //礼盒
        FTYPE_END
    };
public:
    virtual ~FloatAI(void);
    virtual void reset(void);

    void set_float_type(void);
    void set_giftbox_type(void);
    bool is_float(void);
    bool is_giftbox(void);

    void set_wedding_id(const Int64 id);
    Int64 wedding_id(void);

    void set_wedding_type(const int type);
    int wedding_type(void);

    void set_partner_id_1(const Int64 role_id);
    Int64 partner_id_1(void);

    void set_partner_career_1(const int career);
    int partner_career_1(void);

    void set_partner_id_2(const Int64 role_id);
    Int64 partner_id_2(void);

    void set_partner_career_2(const int career);
    int partner_career_2(void);

    bool is_float_owner(const Int64 role_id);

    virtual void generate_gift_box(void);

    // 检查不是所有者才可以拾取
    virtual int gather_limit_collect_begin(Int64 role_id);
    virtual int gather_limit_collect_done(Int64 role_id, int result, ItemObj &gather_item);

    virtual int enter_scene(const int type = ENTER_SCENE_LOGIN);
    virtual int exit_scene(const int type = EXIT_SCENE_LOGOUT);
    virtual int schedule_move_fighter(void);
    virtual int make_up_appear_other_info(Block_Buffer *buff, const bool send_by_gate = false);

    void remove_giftbox_id(const Int64 id);

protected:
    virtual int recycle_self(void);

protected:
    Int64 wedding_id_;
    Int64 wedding_type_;
    Int64 partner_id_1_;
    int partner_career_1_;
    Int64 partner_id_2_;
    int partner_career_2_;
    int float_type_;    // 1 
    Time_Value cruise_timeout_tick_;
    LongSet giftbox_set_;
};

#endif //_FLOATAI_H_
