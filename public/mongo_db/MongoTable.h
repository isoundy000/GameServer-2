/*
 * MongoTable.h
 *
 * Created on: 2013-03-21 11:26
 *     Author: lyz
 */

#ifndef _MONGOTABLE_H_
#define _MONGOTABLE_H_

#include <stdint.h>

template<class Key, class Value, class HSMUTEX> class HashMap;
class MongoConnector;
class MongoDataMap;

class MapPlayerEx;
class LogicPlayer;
class MapLogicPlayer;
class DBShopMode;

namespace mongo
{
	class DBClientConnection;
    class BSONObj;
}
using mongo::DBClientConnection;
using mongo::BSONObj;

class MongoTable
{
public:
    MongoTable(void);
    virtual ~MongoTable(void);

    void set_connection(MongoConnector *conect);
    mongo::DBClientConnection &conection(void);

protected:
    virtual void ensure_all_index(void) = 0;

protected:
    MongoConnector *conector_;
};

#endif //_MONGOTABLE_H_
