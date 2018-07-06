/*
 * MMOWorldBoss.h
 *
 *  Created on: Apr 8, 2014
 *      Author: jinxing
 */

#ifndef MMO_WORLD_BOSS_H_
#define MMO_WORLD_BOSS_H_

#include "MongoTable.h"
#include "HashMap.h"
#include <vector>
#include <map>

class SceneBossDetail;
class BossDetail;
class ShusanLayout;
class WorldBossInfo;
class MAttackLabelRecord;

class MMOWorldBoss: public MongoTable
{
public:
    typedef std::map<int, BossDetail> BossMap;

public:
	MMOWorldBoss();
	virtual ~MMOWorldBoss();

	static int load_wboss_info(WorldBossInfo* wboss_info);
	static int update_wboss_info(WorldBossInfo* wboss_info, int direct_save);

	// 怪物攻城
	static int load_mattack_info(MAttackLabelRecord* label_record);
	static int update_mattack_info(MAttackLabelRecord* label_record);

protected:
    virtual void ensure_all_index(void);
};

#endif /* MMO_WORLD_BOSS_H_ */

