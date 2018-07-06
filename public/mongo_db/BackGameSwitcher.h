/*
 * BackGameSwitcher.h
 *
 *  Created on: Dec 20, 2014
 *      Author: jinxing
 */

#ifndef BACKGAMESWITCHER_H_
#define BACKGAMESWITCHER_H_

#include "PubStruct.h"

class BackGameSwitcher
{
public:
	BackGameSwitcher();
	virtual ~BackGameSwitcher();

	static int direct_load_data(GameSwitcherDetail& detail);
	static int load_data_request(MongoDataMap* data_map);

	static int save_game_modify(BackGameModifyVec& update_vec);
	static string get_49you_qq();

	static string get_fashion_box();

	static int set_detail_value_by_bson(BSONObj &res, GameSwitcherDetail& detail);

	static int get_lucky_table(BackGameModify& modify);

protected:
	virtual void ensure_all_index(void);
};

#endif /* BACKGAMESWITCHER_H_ */
