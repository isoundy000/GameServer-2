/*
 * LeagueFbScript.h
 *
 *  Created on: 2017年2月28日
 *      Author: lyw
 */

#ifndef LEAGUEFBSCRIPT_H_
#define LEAGUEFBSCRIPT_H_

#include "BaseScript.h"

//帮派副本
class LeagueFbScript : public BaseScript
{
public:
	LeagueFbScript();
	virtual ~LeagueFbScript();
	virtual void reset(void);

	virtual int check_add_player_buff();
	virtual int fetch_first_reward(ThreeObjVec& reward_vec);
	virtual int fetch_normal_reward();
	virtual int fetch_wave_reward(ThreeObjVec& reward_vec);
	virtual int script_finish_flag();
	virtual int process_script_stage_finish();	//阶段完成处理

protected:
	virtual int process_script_player_finish(MapPlayerEx* player);
	virtual void recycle_self_to_pool(void);

	void update_reward_map(int reward_id);
	void add_buff_map(const Json::Value &json, IntMap &buff_map);

private:
	int is_add_;
};

#endif /* LEAGUEFBSCRIPT_H_ */
