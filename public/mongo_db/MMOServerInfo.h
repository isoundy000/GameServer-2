/*
 * MMOServerInfo.h
 *
 * Created on: 2016-01-27 17:10
 *     Author: lyz
 */

#ifndef _MMOSERVERINFO_H_
#define _MMOSERVERINFO_H_

#include "MongoTable.h"

class ServerInfo;

class MMOServerInfo : public MongoTable
{
public:
    static int update_server_info(ServerInfo *server_info, int index = -1);
    static int update_server_info(ServerInfo *server_info, int index,
    		mongo::DBClientConnection& conn);

    static int load_server_info(ServerInfo *server_info);
    static int remove_server_info(int server_id);

protected:
    virtual void ensure_all_index(void);
};

class MMOCombineServer : public MongoTable
{
public:
	int load_combine_server(DBShopMode* shop_mode);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOSERVERINFO_H_
