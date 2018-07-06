/*
 * MMOMailOffline.h
 *
 *  Created on: 2013年7月15日
 *      Author: xie
 */

#ifndef MMOMAILOFFLINE_H_
#define MMOMAILOFFLINE_H_

#include "MongoTable.h"
#include "GameHeader.h"

class MailBox;
class MongoDataMap;
class MailInformation;

class MMOMailOffline : public MongoTable
{
public:
	MMOMailOffline(void);
	~MMOMailOffline(void);

	virtual int load_mail_offline(Int64 role_id, MailBox* mail_box);
	virtual int save_mail_offline(MailInformation* mail_info);

	static int remove_loaded_mail_offline(const Int64 role_id, LongVec &id_list, MongoDataMap *data_map);

protected:
	virtual void ensure_all_index(void);
};

class MMOMail : public MongoTable
{
public:
	MMOMail(void);
	virtual ~MMOMail();
	virtual int load_player_mail(MapLogicPlayer * player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
	virtual void ensure_all_index(void);

};


#endif /* MMOMAILOFFLINE_H_ */
