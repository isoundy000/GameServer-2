/*
 * Heap.h
 *
 * Created on: 2013-10-09 10:48
 *     Author: lyz
 */

#ifndef _HEAP_H_
#define _HEAP_H_

#include <functional>
#include <vector>

struct HeapNode
{
    int __heap_index;

    HeapNode(void);
    void set_heap_index(const int index);
    int heap_index(void) const;
};

inline HeapNode::HeapNode(void) :
    __heap_index(-1)
{ /*NULL*/ }

inline void HeapNode::set_heap_index(const int index)
{
    this->__heap_index = index;
}

inline int HeapNode::heap_index(void) const
{
    return this->__heap_index;
}

// index start from zero
#define HEAP_PARENT(X) \
        (((X) - 1) >> 1)
#define HEAP_LEFT_CHILD(X) \
        (((X) << 1) + 1)
#define HEAP_RIGHT_CHILD(X) \
        (((X) << 1) + 2)

template <class Obj, class Compare = std::less<Obj> >
class Heap
{
    typedef std::vector<Obj *> ObjList;
public:
    typedef typename ObjList::iterator iterator;
    typedef typename ObjList::const_iterator const_iterator;
public:
    Heap(void);
    virtual ~Heap(void);
    void clear(void);

    size_t size(void);
    int push(Obj *obj);
    Obj *top(void);
    Obj *pop(void);

    int remove(Obj *obj);
    Obj *remove(const int index);

    Obj *node(const int index);

    bool is_in_heap(const Obj *obj);

    /*尽量不要用迭代器遍历，用的话要判断指针是否为空*/
//    iterator begin(void) { return this->heap_list_.begin(); }
//    iterator end(void) { return this->heap_list_.end(); }
//    const_iterator begin(void) const { return this->heap_list_.begin(); }
//    const_iterator end(void) const { return this->heap_list_.end(); }

protected:
    int adjust_from_top(const int index);
    int adjust_from_bottom(const int index);

protected:
    Compare compare_func_;
    ObjList heap_list_;
    int current_size_;
};

template <class Obj, class Compare>
Heap<Obj, Compare>::Heap(void) : current_size_(0)
{ /*NULL*/ }

template <class Obj, class Compare>
Heap<Obj, Compare>::~Heap(void)
{ /*NULL*/ }

template <class Obj, class Compare>
void Heap<Obj, Compare>::clear(void)
{
    this->heap_list_.clear();
    this->current_size_ = 0;
}

template <class Obj, class Compare>
size_t Heap<Obj, Compare>::size(void)
{
    return this->current_size_;
}

template <class Obj, class Compare>
int Heap<Obj, Compare>::push(Obj *obj)
{
    int index = this->current_size_;
    obj->set_heap_index(index);

    if (this->current_size_ >= int(this->heap_list_.size()))
    {
        this->heap_list_.push_back(obj);
    }
    else
    {
        this->heap_list_[index] = obj;
    }

    ++this->current_size_;

    this->adjust_from_bottom(index);
    return 0;
}

template <class Obj, class Compare>
Obj *Heap<Obj, Compare>::top(void)
{
    if (this->current_size_ <= 0)
        return 0;

    return this->heap_list_[0];
}

template <class Obj, class Compare>
Obj *Heap<Obj, Compare>::pop(void)
{
    if (this->current_size_ <= 0)
        return 0;

    Obj *obj = this->heap_list_[0];
    this->heap_list_[0] = this->heap_list_[--this->current_size_];
    this->heap_list_[0]->set_heap_index(0);
    this->heap_list_[this->current_size_] = 0;

    this->adjust_from_top(0);

    obj->set_heap_index(-1);
    return obj;
}

template <class Obj, class Compare>
int Heap<Obj, Compare>::remove(Obj *obj)
{
    int cur_index = obj->heap_index();
    if (cur_index < 0 || this->current_size_ <= cur_index)
        return -1;

    Obj *remove_obj = this->heap_list_[cur_index];
    if (remove_obj != obj)
        return -1;

    if (this->remove(cur_index) == 0)
        return -1;
    return 0;
}

template <class Obj, class Compare>
Obj *Heap<Obj, Compare>::remove(const int index)
{
    int cur_index = index;
    if (cur_index < 0 || this->current_size_ <= cur_index)
        return 0;

    Obj *remove_obj = this->heap_list_[cur_index];

    Obj *adjust_obj = this->heap_list_[--this->current_size_];
    this->heap_list_[cur_index] = adjust_obj;
    adjust_obj->set_heap_index(cur_index);

    int parent_index = HEAP_PARENT(cur_index);
    if (parent_index >= 0 && this->compare_func_(adjust_obj, this->heap_list_[parent_index]) == true)
    {
        this->adjust_from_bottom(cur_index);
    }
    else
    {
        this->adjust_from_top(cur_index);
    }

    if (remove_obj != 0)
        remove_obj->set_heap_index(-1);

    return remove_obj;
}

template <class Object, class Compare>
Object *Heap<Object, Compare>::node(const int index)
{
    if (index < 0 || index >= this->current_size_)
        return 0;
    return this->heap_list_[index];
}

template <class Object, class Compare>
int Heap<Object, Compare>::adjust_from_top(const int index)
{
    if (index < 0 || index >= this->current_size_)
        return -1;

    Object *cur_node = 0, *left_node = 0, *right_node = 0;
    int cur_index = index, 
        left_index = HEAP_LEFT_CHILD(index),
        right_index = HEAP_RIGHT_CHILD(index);

    cur_node = this->heap_list_[cur_index];
    while (left_index < this->current_size_ || right_index < this->current_size_)
    {
        left_node = this->heap_list_[left_index];
        right_node = 0;
        if (right_index < this->current_size_)
            right_node = this->heap_list_[right_index];

        if (this->compare_func_(cur_node, left_node) == false)
        {
            if (right_node != 0 && this->compare_func_(cur_node, right_node) == false)
            {
                if (this->compare_func_(left_node, right_node) == true)
                {
                    this->heap_list_[cur_index] = left_node;
                    left_node->set_heap_index(cur_index);
                    cur_index = left_index;
                }
                else
                {
                    this->heap_list_[cur_index] = right_node;
                    right_node->set_heap_index(cur_index);
                    cur_index = right_index;
                }
            }
            else
            {
                this->heap_list_[cur_index] = left_node;
                left_node->set_heap_index(cur_index);
                cur_index = left_index;
            }

        }
        else if (right_node != 0 && this->compare_func_(cur_node, right_node) == false)
        {
            this->heap_list_[cur_index] = right_node;
            right_node->set_heap_index(cur_index);
            cur_index = right_index;
        }
        else
        {
            break;
        }

        left_index = HEAP_LEFT_CHILD(cur_index);
        right_index = HEAP_RIGHT_CHILD(cur_index);
    }

    this->heap_list_[cur_index] = cur_node;
    cur_node->set_heap_index(cur_index);

    return 0;
}

template <class Object, class Compare>
int Heap<Object, Compare>::adjust_from_bottom(const int index)
{
    if (index <= 0 || index >= this->current_size_)
        return -1;

    Object *cur_node = 0, *parent_node = 0;
    int cur_index = index;
    int parent_index = HEAP_PARENT(cur_index);

    cur_node = this->heap_list_[cur_index];
    while(parent_index >= 0)
    {
        parent_node = this->heap_list_[parent_index];
        if (this->compare_func_(cur_node, parent_node) == false)
        {
            break;
        }

        //this->heap_list_[parent_index-1].__object = cur_node;
        this->heap_list_[cur_index] = parent_node;
        parent_node->set_heap_index(cur_index);

        cur_index = parent_index;
        parent_index = HEAP_PARENT(cur_index);
    }

    this->heap_list_[cur_index] = cur_node;
    cur_node->set_heap_index(cur_index);

    return 0;
}

template <class Obj, class Compare>
bool Heap<Obj, Compare>::is_in_heap(const Obj *obj)
{
    int cur_index = obj->heap_index();
    if (cur_index < 0 || this->current_size_ <= cur_index)
        return false;

    Obj *heap_obj = this->heap_list_[cur_index];
    return (heap_obj == obj);
}

#endif //_HEAP_H_
