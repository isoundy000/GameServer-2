/*
 * MMOChatPrivate.cpp
 *
 *  Created on: 2013-7-2
 *      Author: root
 */

#include "MMOChatPrivate.h"
#include "GameField.h"
#include "PrivateChat.h"
#include "ChannelAgency.h"
#include "Block_Buffer.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOChatPrivate::MMOChatPrivate()
{

}

MMOChatPrivate::~MMOChatPrivate()
{

}

void MMOChatPrivate::ensure_all_index(void)
{
}

int MMOChatPrivate::load_private_record(PrivateChat *pChat)
{
	{//加载黑名单
		BSONObj res = this->conection().findOne(SocialerInfo::COLLECTION,
				QUERY(SocialerInfo::ID << (long long int)pChat->channel_id()));

		int64_t player_id = 0;
		if(res.hasField(SocialerInfo::BLACK_LIST.c_str()))
		{
			BSONObj friend_list = res.getObjectField(SocialerInfo::BLACK_LIST.c_str());
			BSONObjIterator it(friend_list);
			while(it.more())
			{
				BSONObj obj = it.next().embeddedObject();
				player_id = obj[SocialerInfo::ID].numberLong();
				pChat->add_black_list(player_id);
			}
		}
	}

	return 0;
}

int MMOChatPrivate::save_private_record(PrivateChat *pChat)
{
	return 0;
}
