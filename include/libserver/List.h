// -*- C++ -*-
/*
 * List.h
 *
 *  Created on: Apr 12, 2012
 *      Author: ChenLong
 */

#ifndef LIST_H_
#define LIST_H_

#include "Thread_Mutex.h"
#include "Mutex_Guard.h"
#include <list>

template <typename Obj, typename LOCK = NULL_MUTEX>
class List {
public:
	typedef std::list<Obj> TList;
	typedef typename TList::const_iterator const_iterator;

	List(void);
	~List(void);

	void push_back(const Obj &v);
	Obj &front();
	Obj pop_front();

	bool empty();
	void clear();
	int size() const;

	const_iterator begin() const;
	const_iterator end() const;

	const std::list<Obj> &get_list();
	const std::list<Obj> &get_list() const;

private:
	TList list_;
	int size_;
	LOCK lock_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename Obj, typename LOCK>
List<Obj, LOCK>::List(void) : size_(0) { }

template <typename Obj, typename LOCK>
List<Obj, LOCK>::~List(void) { }

template <typename Obj, typename LOCK>
void List<Obj, LOCK>::push_back(const Obj &v) {
	GUARD(LOCK, mon, this->lock_);
	++size_;
	list_.push_back(v);
	return ;
}

template <typename Obj, typename LOCK>
Obj &List<Obj, LOCK>::front() {
	GUARD(LOCK, mon, this->lock_);
	return list_.front();
}

template <typename Obj, typename LOCK>
Obj List<Obj, LOCK>::pop_front() {
	GUARD(LOCK, mon, this->lock_);
	Obj val = list_.front();
	--size_;
	list_.pop_front();
	return val;
}

template <typename Obj, typename LOCK>
bool List<Obj, LOCK>::empty() {
//	GUARD(LOCK, mon, this->lock_);
	return size_ <= 0;
}

template <typename Obj, typename LOCK>
void List<Obj, LOCK>::clear() {
	GUARD(LOCK, mon, this->lock_);
	size_ = 0;
	list_.clear();
	return ;
}

template <typename Obj, typename LOCK>
int List<Obj, LOCK>::size() const {
//	GUARD(LOCK, mon, this->lock_);
	return size_;
}

template <typename Obj, typename LOCK>
const std::list<Obj> &List<Obj, LOCK>::get_list() {
	GUARD(LOCK, mon, this->lock_);
	return list_;
}

template <typename Obj, typename LOCK>
const std::list<Obj> &List<Obj, LOCK>::get_list() const {
	GUARD(LOCK, mon, this->lock_);
	return list_;
}

template <typename Obj, typename LOCK>
typename List<Obj, LOCK>::const_iterator List<Obj, LOCK>::begin() const {
	return list_.begin();
}

template <typename Obj, typename LOCK>
typename List<Obj, LOCK>::const_iterator List<Obj, LOCK>::end() const {
	return list_.end();
}

#endif /* LIST_H_ */
