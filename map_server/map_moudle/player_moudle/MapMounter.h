/*
 * MapMounter.h
 *
 *  Created on: Nov 26, 2013
 *      Author: peizhibi
 */

#ifndef MAPMOUNTER_H_
#define MAPMOUNTER_H_

#include "GameFighter.h"

class Proto30400406;

class MapMounter : virtual public GameFighter
{
public:
	MapMounter();
	virtual ~MapMounter();
	void reset_mount(void);

	MountDetail &mount_detail(int type = 1);

	int update_mount_info(Message* msg);
	int refresh_mount_skill(int type, Proto30400406* request);
	int refresh_mount_shape(MountDetail& mount_detail);
	int refresh_mount_prop(int type);

	int mount_grade(int type = GameEnum::FUN_MOUNT);
	int is_on_mount(int type = GameEnum::FUN_MOUNT);
	int fetch_mount_id(int type = GameEnum::FUN_MOUNT);

	int mount_speed();
	int fetch_mount_special_force();

	int refresh_fun_mount_prop();
	int refresh_fun_beast_prop();

protected:
	MountDetail mount_detail_[GameEnum::FUN_TOTAL_MOUNT_TYPE];
};

#endif /* MAPMOUNTER_H_ */
