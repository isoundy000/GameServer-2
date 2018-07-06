/*
 * MMOSwordPool.h
 *
 *  Created on: 2016年10月12日
 *      Author: lyw
 */

#ifndef MMOSWORDPOOL_H_
#define MMOSWORDPOOL_H_

#include "MongoTable.h"
#include "LeagueStruct.h"

class MMOSwordPool : public MongoTable
{
public:
	MMOSwordPool();
	virtual ~MMOSwordPool();

	int load_player_spool(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

protected:
	virtual void ensure_all_index(void);
};

class MMOHiddenTreasure : public MongoTable
{
public:
	MMOHiddenTreasure();
	virtual ~MMOHiddenTreasure();

	int load_player_hi_treasure(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

protected:
	virtual void ensure_all_index(void);
};

class MMOFashion : public MongoTable
{
public:
	MMOFashion();
	virtual ~MMOFashion();

	int load_player_fashion(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);
	static void fetch_player_fashion(LeagueMember &member);

protected:
	virtual void ensure_all_index(void);
};

class MMOTransfer : public MongoTable
{
public:
	MMOTransfer();
	virtual ~MMOTransfer();

	int load_player_transfer(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

protected:
	virtual void ensure_all_index(void);
};

#endif /* MMOSWORDPOOL_H_ */
