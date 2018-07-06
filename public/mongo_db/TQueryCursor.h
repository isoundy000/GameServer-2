/*
 * TQueryCursor.h
 *
 * Created on: 2014-03-08 16:33
 *     Author: lyz
 */

#ifndef _TQUERYCURSOR_H_
#define _TQUERYCURSOR_H_

#include <vector>
#include <memory>

namespace mongo
{
	class BSONObj;
	class DBClientCursor;
}
using mongo::BSONObj;
using mongo::DBClientCursor;

class TQueryCursor
{
public:
    typedef std::vector<BSONObj> BSONObjList;

public:
    TQueryCursor(void);
    ~TQueryCursor(void);

    void set_data(std::auto_ptr<DBClientCursor> cursor);
    bool more(void);
    BSONObj next(void);

protected:
    BSONObjList obj_list_;
    size_t read_index_;
};

#endif //_TQUERYCURSOR_H_
