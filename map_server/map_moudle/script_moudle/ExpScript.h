/*
 * ExpScript.h
 *
 * Created on: 2013-12-26 19:07
 *     Author: lyz
 */

#ifndef _EXPSCRIPT_H_
#define _EXPSCRIPT_H_

#include "BaseScript.h"

// 经验副本
class ExpScript : public BaseScript
{
public:
    ExpScript(void);
    virtual ~ExpScript(void);
    virtual void reset(void);

    virtual int fetch_first_reward(ThreeObjVec& reward_vec);
    virtual int fetch_normal_reward();
    virtual int fetch_wave_reward(ThreeObjVec& reward_vec);
    virtual int sync_restore_pass(MapPlayerScript* player);//更新资源找回
    virtual int script_pass_chapter();
    virtual int script_finish_flag();
    virtual int process_script_stage_finish();	//阶段完成处理

protected:
    virtual int process_script_player_finish(MapPlayerEx* player);
    virtual void recycle_self_to_pool(void);

    void update_reward_map(int reward_id);

};

#endif //_EXPSCRIPT_H_
