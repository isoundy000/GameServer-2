/*
 * MongoException.h
 *
 * Created on: 2013-02-20 16:36
 *     Author: glendy
 */

#ifndef _MONGOEXCEPTION_H_
#define _MONGOEXCEPTION_H_

#include <mongo/util/net/sock.h>
#include "Log.h"

#define BEGIN_CATCH \
    try {

#define END_CATCH \
	} \
	catch(mongo::SocketException &ex) \
	{ \
		MSG_USER("ERROR %s", ex.toString().c_str()); \
		this->conector_->connect_mongo(); \
	} \
	catch(...) \
	{ \
		MSG_USER("ERROR mongo error"); \
	};

#define END_CACHE_CATCH\
	}\
	catch(mongo::SocketException &ex)\
	{\
		MSG_USER(ERROR %s, ex.toString().c_str());\
	}\
	catch(...)\
	{\
		MSG_USER(ERROR mongo error);\
	};

#endif //_MONGOEXCEPTION_H_
