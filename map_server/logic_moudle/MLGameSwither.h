/*
 * MLGameSwither.h
 *
 *  Created on: Dec 20, 2014
 *      Author: jinxing
 */

#ifndef MLGAMESWITHER_H_
#define MLGAMESWITHER_H_

#include "PubStruct.h"
#include <google/protobuf/message.h>

class MLGameSwither
{
public:
	MLGameSwither();
	virtual ~MLGameSwither();
	void init(void);
	GameSwitcherDetail& detail(void);

	int request_sync_from_logic();
	int sync_from_logic(Message *msg);

	int process_detail_has_update(GameSwitcherDetail& old);	// 数据有更新

	bool map_check_switcher(const std::string& name);	// 后台开关判断（仅用于地图进程）

private:
	GameSwitcherDetail detail_;
};

typedef Singleton<MLGameSwither> MLGameSwitcherSysSingle;
#define ML_SWITCHER_SYS	MLGameSwitcherSysSingle::instance()

#endif /* MLGAMESWITHER_H_ */
