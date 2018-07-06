/*
 * EffectAI.h
 *
 * Created on: 2014-02-12 17:23
 *     Author: lyz
 */

#ifndef _EFFECTAI_H_
#define _EFFECTAI_H_

#include "GameAI.h"

class EffectAI: public GameAI {
public:
	enum EFFECT_MONSTER_TYPE {
		EMT_AREA_HURT_EFFECT = 0,   // 默认是区域伤害特效
		EMT_SAFE_AREA_EFFECT = 1,   // 安全区域特效（进行安全区域特殊处理）
		EMT_END
	};
public:
	EffectAI(void);
	virtual ~EffectAI(void);
	virtual void reset(void);

	bool is_area_hurt_effect(void);
	bool is_safe_area_effect(void);

	virtual void set_effect_sort(const int effect_sort);
	virtual int effect_sort(void);

	virtual int make_up_appear_info_base(Block_Buffer *buff,
			const bool send_by_gate = false);
	virtual int make_up_appear_other_info(Block_Buffer *buff,
			const bool send_by_gate = false);
	virtual int make_up_disappear_info_base(Block_Buffer *buff,
			const bool send_by_gate = false);
	virtual int make_up_disappear_other_info(Block_Buffer *buff,
			const bool send_by_gate = false);

	bool validate_point_not_overlap(CoordVec &coord_set, MoverCoord &move_coord,
			int radius);

protected:
	virtual int recycle_self(void);

	virtual void init_base_property(void);
	virtual void init_safe_area(const Json::Value &prop_monster);
	virtual int validate_safe_area(GameFighter *defender);

protected:
	int effect_sort_;
	int safe_point_effect_sort_;
	int effect_monster_type_;
};

#endif //_EFFECTAI_H_
