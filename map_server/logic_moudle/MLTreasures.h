/*
 * MLTreasures.h
 *
 *  Created on: 2016年11月30日
 *      Author: lzy
 */

#ifndef MLTREASURES_H_
#define MLTREASURES_H_

#include "MLPacker.h"

class MLTreasures : virtual public MLPacker
{
public:
	enum
	{
		RAND_BASE_NUM = 10000
	};

	MLTreasures();
	~MLTreasures();

	int treasures_info_reset();
	int player_dice_begin(Message* msg);
	int player_dice_end(Message* msg);
	int rand_dice();
	int fetch_act_mult(Message* msg);
	int player_reset_treasures_game_begin();
	int player_reset_treasures_game_end(Message* msg);
	int fetch_treasures_info();
	int notify_treasures_info();
	int notify_treasures_all_reward();
	int check_treasures_reward(int index, int mult);
	TreasuresDetail &treasures_detail();

	int sync_transfer_treasures_info(int scene_id);
	int read_transfer_treasures_info(Message* msg);

private:
	TreasuresDetail treasures_detail_;
};


#endif /* MLTREASURES_H_ */
