/*
 * TQueryCursor.cpp
 *
 * Created on: 2014-03-08 16:45
 *     Author: lyz
 */

#include "TQueryCursor.h"

#include "GameHeader.h"
#include <mongo/client/dbclient.h>

TQueryCursor::TQueryCursor(void) :
    read_index_(0)
{ /*NULL*/ }

TQueryCursor::~TQueryCursor(void)
{ /*NULL*/ }

void TQueryCursor::set_data(std::auto_ptr<DBClientCursor> cursor)
{
    this->obj_list_.clear();
    while (cursor->more())
    {
    	BSONObj obj = cursor->next().copy();
        this->obj_list_.push_back(obj);
    }
    this->read_index_ = 0;
}

bool TQueryCursor::more(void)
{
    return this->read_index_ < this->obj_list_.size();
}

BSONObj TQueryCursor::next(void)
{
    if (this->more() == false)
        return BSONObj();

    return this->obj_list_[this->read_index_++];
}

