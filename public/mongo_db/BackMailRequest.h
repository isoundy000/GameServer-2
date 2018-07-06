/*
 * BackMailRequest.h
 *
 *  Created on: Jun 30, 2014
 *      Author: louis
 */

#ifndef BACKMAILREQUEST_H_
#define BACKMAILREQUEST_H_

#include "MongoTable.h"

class BackMailRequest : public MongoTable
{
public:
	BackMailRequest();
	virtual ~BackMailRequest();

	static int update_data(MongoDataMap* data_map, const int mail_request_id);
	static int load_mail_request(MongoDataMap* data_map);
    static int process_back_mail(void);

	int load_game_notice(DBShopMode* shop_mode);
	int load_back_activity(DBShopMode* shop_mode);
	int load_chat_limit(DBShopMode* shop_mode);
	int load_word_check(DBShopMode* shop_mode);
	int load_vip_chat_limit(DBShopMode* shop_mode);
protected:
	virtual void ensure_all_index(void);
};

#endif /* BACKMAILREQUEST_H_ */
