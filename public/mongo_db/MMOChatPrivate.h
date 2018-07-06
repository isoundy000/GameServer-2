/*
 * MMOChatPrivate.h
 *
 *  Created on: 2013-7-2
 *      Author: root
 */

#ifndef MMOCHATPRIVATE_H_
#define MMOCHATPRIVATE_H_

#include "MongoTable.h"

class PrivateChat;
class ChatRecord;

class MMOChatPrivate: public MongoTable
{
public:
	MMOChatPrivate();
	virtual ~MMOChatPrivate();

	virtual void ensure_all_index(void);

	int load_private_record(PrivateChat *pChat);
	int save_private_record(PrivateChat *pChat);
};

#endif /* MMOCHATPRIVATE_H_ */
