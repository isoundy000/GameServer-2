/*
 * MMOPlayerTip.h
 *
 *  Created on: May 7, 2014
 *      Author: louis
 */

#ifndef MMOPLAYERTIP_H_
#define MMOPLAYERTIP_H_

#include "MongoTable.h"

class MMOPlayerTip : public MongoTable
{
public:
	MMOPlayerTip();
	virtual ~MMOPlayerTip();

    int load_player_tip_pannel(MapLogicPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
    virtual void ensure_all_index(void);

};

#endif /* MMOPLAYERTIP_H_ */
