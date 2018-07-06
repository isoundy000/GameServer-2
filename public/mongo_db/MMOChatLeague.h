/*
 * MMOChatLeague.h
 *
 *  Created on: 2013-7-1
 *      Author: root
 */

#ifndef MMOCHATLEAGUE_H_
#define MMOCHATLEAGUE_H_

#include "MongoTable.h"

class LeagueChannel;
class ChannelAgency;

class MMOChatLeague: public MongoTable
{
public:
	MMOChatLeague();
	virtual ~MMOChatLeague();

	int load_league_record(ChannelAgency *agency);
	int load_league_record(LeagueChannel *channel);
	int save_league_record(LeagueChannel *channel);

protected:
	virtual void ensure_all_index(void);
};

#endif /* MMOCHATLEAGUE_H_ */
