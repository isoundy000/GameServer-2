/*
 * ControlNode.h
 *
 * Created on: 2013-05-17 15:51
 *     Author: lyz
 */

#ifndef _CONTROLNODE_H_
#define _CONTROLNODE_H_

#include "BehaviorNode.h"

/*
 * 从上一次遍历的节点开始,找到第一个为true的子节点;
 * 条件判断：有一个子节点为true则返回true;
 * */
class SelectNode : public BehaviorNode
{
public:
    virtual void reset(void);

protected:
    virtual bool DoEvaluate(NodeInputParam &input);
    virtual void DoTransition(NodeInputParam &input);
    virtual int DoTick(NodeOutputParam &output, NodeInputParam &input);

private:
    int set_next_index();

private:
    int cur_index_;
};

/*
 * 每次从头开始，遍历找到第一个为true的子节点;
 * 条件判断：有一个子节点为true则返回true;
 * */
class PrioritySelectNode : public BehaviorNode
{
public:
	PrioritySelectNode();

protected:
    virtual bool DoEvaluate(NodeInputParam &input);
    virtual void DoTransition(NodeInputParam &input);
    virtual int DoTick(NodeOutputParam &output, NodeInputParam &input);

private:
    int cur_index_;
};

/*
 * 从第一个结点开始执行至结束
 * 最后循环
 * */
class SequenceNode : public BehaviorNode
{
public:
    virtual void reset(void);

protected:
    virtual bool DoEvaluate(NodeInputParam &input);
    virtual void DoTransition(NodeInputParam &input);
    virtual int DoTick(NodeOutputParam &output, NodeInputParam &input);

private:
    int set_next_index();

private:
    int cur_index_;
};

/*
 * 执行所有的子节点
 * 条件判断：有一个子节点为true则返回true;
 * */
class ParallelSelectNode : public BehaviorNode
{
public:
    typedef std::vector<BehaviorNode *> ChildExecutingList;

public:
    virtual void reset(void);

protected:
    virtual bool DoEvaluate(NodeInputParam &input);
    virtual void DoTransition(NodeInputParam &input);
    virtual int DoTick(NodeOutputParam &output, NodeInputParam &input);

private:
    ChildExecutingList child_executing_list_;
};

/*
 * 执行所有的子节点
 * 条件判断：有一个子节点为false则返回false;
 * */
class ParallelSequenceNode : public BehaviorNode
{
public:
    typedef std::vector<BehaviorNode *> ChildExecutingList;
public:
    virtual void reset(void);

protected:
    virtual bool DoEvaluate(NodeInputParam &input);
    virtual void DoTransition(NodeInputParam &input);
    virtual int DoTick(NodeOutputParam &output, NodeInputParam &input);

private:
    ChildExecutingList child_executing_list_;
};

#endif //_CONTROLNODE_H_
