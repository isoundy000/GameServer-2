/*
 * MLLeaguer.h
 *
 *  Created on: Dec 10, 2013
 *      Author: peizhibi
 */

#ifndef MLLEAGUER_H_
#define MLLEAGUER_H_

#include "MLPacker.h"

class MLLeaguer : virtual public MLPacker
{
public:
	MLLeaguer();
	virtual ~MLLeaguer();

    void reset(void);

	int sync_league_info(Message* msg);

	int logic_league_donate(Message* msg);
	int logic_create_league(Message* msg);
	int logic_league_shop_buy(Message* msg);
    int logic_sync_league_id(Message *msg);
    int logic_feed_league_boss(Message* msg);
    int logic_summon_boss(Message* msg);

    int lstore_insert_failed(Message *msg);
    int lstore_insert(Message *msg);
    int lstore_get(Message *msg);

    int check_league_open_task(int task_id);

protected:
    int logic_lsiege_shop_buy(Message *msg);
public:
    Int64 join_tick_;
};

#endif /* MLLEAGUER_H_ */
