/*
 * LogicGameSwitcherSys.h
 *
 *  Created on: Dec 20, 2014
 *      Author: jinxing
 */

#ifndef LOGICGAMESWITCHERSYS_H_
#define LOGICGAMESWITCHERSYS_H_

#include "PubStruct.h"
class BaseLogicPlayer;

class LogicGameSwitcherSys
{
public:
	LogicGameSwitcherSys();
	virtual ~LogicGameSwitcherSys();
	GameSwitcherDetail& detail(void);

	int start(void);
	int stop(void);

	int interval_run(void);
	int direct_load_date_from_db(void);
	int request_load_date_from_db(void);
	int after_load_data(Transaction* trans);	// 通过消息号加载数据后调用

	int process_detail_has_update(GameSwitcherDetail& old);	// 数据有更新
	int push_detail_to_ml(void);
	int notify_client(BaseLogicPlayer *player = 0);

	bool logic_check_switcher(const std::string& name); // 后台开关判断（仅用于逻辑进程）

private:
	GameSwitcherDetail detail_;
};

typedef Singleton<LogicGameSwitcherSys> LogicGameSwitcherSysSingle;
#define LOGIC_SWITCHER_SYS	LogicGameSwitcherSysSingle::instance()

#endif /* LOGICGAMESWITCHERSYS_H_ */
