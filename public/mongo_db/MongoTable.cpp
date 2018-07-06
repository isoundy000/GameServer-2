/*
 * MongoTable.cpp
 *
 * Created on: 2013-03-21 16:30
 *     Author: lyz
 */

#include "MongoTable.h"
#include "MongoConnector.h"
#include "DaemonServer.h"

#include <mongo/client/dbclient.h>
#include "GameHeader.h"

MongoTable::MongoTable() : conector_(0)
{ /*NULL*/ }

MongoTable::~MongoTable(void)
{ /*NULL*/ }

void MongoTable::set_connection(MongoConnector *conect)
{
	this->conector_ = conect;
#ifdef LOCAL_DEBUG
	this->ensure_all_index();
#else
	if (DAEMON_SERVER->is_server_daemon() == true)
	{
	    this->ensure_all_index();
	}
#endif
}

mongo::DBClientConnection &MongoTable::conection(void)
{
    return this->conector_->conection();
}

