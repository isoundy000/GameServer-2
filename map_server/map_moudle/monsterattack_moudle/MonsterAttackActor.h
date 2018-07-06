/*
 * MonsterAttackActor.h
 *
 *  Created on: 2016年9月27日
 *      Author: lyw
 */

#ifndef MONSTERATTACKACTOR_H_
#define MONSTERATTACKACTOR_H_

#include "MapPlayer.h"

class MonsterAttackActor: virtual public MapPlayer {
public:
	MonsterAttackActor();
	virtual ~MonsterAttackActor();

	void reset();
	virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
	virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
	virtual int die_process(const int64_t fighter_id);

	int mattack_scene();
	int mattack_enter_scene_type();
	int request_enter_player(void);
	int request_get_fighting_will();
	int handle_exit_mattack_scene(void);

	int on_enter_mattack_scene(const int type);
	int on_exit_mattack_scene(const int type);

private:
	int mattack_die_process(Int64 fighter_id);
};

#endif /* MONSTERATTACKACTOR_H_ */
