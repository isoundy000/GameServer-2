/*
 * DoubleQueue.h
 *
 * Created on: 2012-05-26 19:51
 *     Author: lyz
 */

#ifndef _DOUBLEQUEUE_H_
#define _DOUBLEQUEUE_H_

#include <queue>
#include "Thread_Mutex.h"
#include "Mutex_Guard.h"

class DoubleQueue
{
public:
    typedef std::queue<void *> ItemQueue;

    DoubleQueue(void);
    ~DoubleQueue(void);

    void clear(void);
    bool empty(void);
    size_t size(void);
    int push(void *item);
    int pop(void *&item);

protected:
    void swap(void);

protected:
    ItemQueue one_queue_;
    ItemQueue two_queue_;
    ItemQueue *read_queue_;
    ItemQueue *write_queue_;

    RW_Mutex mutex_;

    RE_MUTEX write_mutex_;
    size_t read_size_;
    size_t write_size_;
};

inline DoubleQueue::DoubleQueue(void)
{
	this->read_queue_ = &(this->one_queue_);
	this->read_size_ = 0;
	this->write_queue_ = &(this->two_queue_);
	this->write_size_ = 0;
}

inline DoubleQueue::~DoubleQueue(void)
{ /*NULL*/ }

inline void DoubleQueue::clear(void)
{
    GUARD_WRITE(RW_MUTEX, mon_1, this->mutex_);
    GUARD(RE_MUTEX, mon_2, this->write_mutex_);
	while (this->one_queue_.empty() == false)
		this->one_queue_.pop();
	while (this->two_queue_.empty() == false)
		this->two_queue_.pop();
}

inline bool DoubleQueue::empty(void)
{
	size_t read_size = this->read_size_, write_size = this->write_size_;
    if (read_size > 0 || write_size > 0)
        return false;

    return true;
}

inline size_t DoubleQueue::size(void)
{
	size_t read_size = this->read_size_, write_size = this->write_size_;
    return (read_size + write_size);
}

inline int DoubleQueue::push(void *item)
{
    GUARD_READ(RW_MUTEX, mon_1, this->mutex_);
    GUARD(RE_MUTEX, mon_2, this->write_mutex_);

    this->write_queue_->push(item);
    ++(this->write_size_);

    return 0;
}

inline int DoubleQueue::pop(void *&item)
{
    if (this->empty() == true)
        return -1;

    {
        GUARD_READ(RW_MUTEX, mon, this->mutex_);

        if (this->read_queue_->empty() == false)
        {
            item = this->read_queue_->front();
            this->read_queue_->pop();
            --this->read_size_;

            return 0;
        }
    }

    if (this->size() <= 0)
        return -1;

    {
        GUARD_WRITE(RW_MUTEX, mon_1, this->mutex_);
        GUARD(RE_MUTEX, mon_2, this->write_mutex_);
        this->swap();
    }

    return -1;
}

inline void DoubleQueue::swap(void)
{
    ItemQueue *tmp_queue = this->write_queue_;
    this->write_queue_ = this->read_queue_;
    this->read_queue_ = tmp_queue;

    this->write_size_ = this->write_queue_->size();
    this->read_size_ = this->read_queue_->size();
}

#endif //_DOUBLEQUEUE_H_
