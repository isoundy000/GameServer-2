/*
 * BackGameModifySys.h
 *
 *  Created on: 2015年12月30日
 *      Author: sky
 */

#ifndef LOGIC_SERVER_BACKGAMEMODIFYSYS_H_
#define LOGIC_SERVER_BACKGAMEMODIFYSYS_H_

#include "PubStruct.h"
class BaseLogicPlayer;

class BackGameModifySys
{
public:
	BackGameModifySys();
	virtual ~BackGameModifySys();

	int interval_run(void);
	int request_load_date_from_db(void);
	int after_load_data(Transaction* trans);	// 通过消息号加载数据后调用
	int fetch_super_vip_info(Message *msg);		// 超级VIP的功能要用这里的数据

private:
	void handle_game_modify(BackGameModify& game_modify);
	int handle_49youqq(BackGameModify& game_modify);
	int handle_fashion_box(BackGameModify& game_modify);
	int handle_lucky_table(BackGameModify& game_modify);
private:
	void notify_49_qq(LongMap& qq_set);
	BackGameModifyVec update_vec;
};

typedef Singleton<BackGameModifySys> BackGameModifySysSingle;
#define BACK_GAME_MODIFY_SYS	BackGameModifySysSingle::instance()



#endif /* LOGIC_SERVER_BACKGAMEMODIFYSYS_H_ */
