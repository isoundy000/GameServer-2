/*
 * TowerDefenseScript.h
 *
 * Created on: 2014-02-13 15:42
 *     Author: lyz
 */

#ifndef _TOWERDEFENSESCRIPT_H_
#define _TOWERDEFENSESCRIPT_H_

#include "BaseScript.h"

// 塔防副本
class TowerDefenseScript : public BaseScript
{
public:
    TowerDefenseScript(void);
    virtual ~TowerDefenseScript(void);
    virtual void reset(void);

    void set_spirit_value(const int value);
    int spirit_value(void);
    void update_matrix_spirit_level(const int matrix_id, const int level);
    int matrix_spirit_level(const int matrix_id);

    int check_matrix_status(GameFighter *fighter);

    int check_call_puppet(const int puppet, const int scene_id);
    int update_puppet_call_flag(const int puppet);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif //_TOWERDEFENSESCRIPT_H_
