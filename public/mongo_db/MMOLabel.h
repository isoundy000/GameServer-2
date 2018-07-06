/*
 * MMOLabel.h
 *
 *  Created on: 2013-12-3
 *      Author: louis
 */

#ifndef MMOLABEL_H_
#define MMOLABEL_H_

#include "MongoTable.h"

class MMOLabel : public MongoTable
{
public:
	MMOLabel();
	virtual ~MMOLabel();

	int load_player_label(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

	static void save_player_label(long role_id, int label_id, int source = 0);

protected:
    virtual void ensure_all_index(void);
};

class MMOAchievement : public MongoTable
{
public:
	MMOAchievement();
	virtual ~MMOAchievement();

	int load_player_achievement(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

	static int request_update_special_FB_achievement(MongoDataMap* data_map, MapLogicPlayer* player);
	int update_special_FB_achievement(MongoDataMap* data_map);

protected:
    virtual void ensure_all_index(void);
};

class MMOTask : public MongoTable
{
public:
    MMOTask(void);
    virtual ~MMOTask(void);

    int load_player_task(MapLogicPlayer *player);

    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
    virtual void ensure_all_index(void);
};

class MMOSysSetting : public MongoTable
{
public:
	MMOSysSetting();
	virtual ~MMOSysSetting();

	int load_player_system_setting(MapLogicPlayer* player);
	static int update_data(MapLogicPlayer* player, MongoDataMap* data_map);

	static int sync_setting_info_to_other_role(std::string account, long long int  role_id, BSONObj& set_detail);

protected:
	virtual void ensure_all_index(void);
};

class MMOWelfare: public MongoTable
{
public:
	MMOWelfare();
	virtual ~MMOWelfare();

    int load_player_welfare(MapLogicPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

public:
    static int mmo_once_rewards_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj);
    static int mmo_once_rewards_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player);

protected:
    virtual void ensure_all_index(void);

};


#endif /* MMOLABEL_H_ */
