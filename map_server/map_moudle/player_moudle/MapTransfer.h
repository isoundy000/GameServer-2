/*
 * MapTransfer.h
 *
 *  Created on: 2017年4月7日
 *      Author: lyw
 */

#ifndef MAPTRANSFER_H_
#define MAPTRANSFER_H_

#include "GameFighter.h"

class MapTransfer : virtual public GameFighter
{
public:
	MapTransfer();
	virtual ~MapTransfer();
	void reset_transfer(void);

	virtual MapPlayer *transfer_player(void);

	virtual double fetch_sub_skill_use_rate(FighterSkill* skill);

	int update_transfer_info(Message* msg);
	int player_use_transfer();
	int create_transfer_skill(int type);
	int delete_last_transfer(int type);

	int use_transfer(int id);
	int remove_transfer(int id);

	int refresh_transfer_shape();
	int trans_refresh_transfer_shape();
	int fetch_transfer_id();

	SkillMap *fetch_transfer_skill();

	bool check_is_in_transfer_time();
	bool check_is_in_cool_time();

	TransferDetail& transfer_detail();

private:
	TransferDetail transfer_detail_;

};

#endif /* MAPTRANSFER_H_ */
