/*
 * ControlNode.cpp
 *
 * Created on: 2013-05-17 19:24
 *     Author: lyz
 */

#include "ControlNode.h"

/*
 * SelectNode
 * */
void SelectNode::reset(void)
{
    this->cur_index_ = 0;
    BehaviorNode::reset();
}

bool SelectNode::DoEvaluate(NodeInputParam &input)
{
	JUDGE_RETURN(this->child_list_.empty() == false, false);

    int next_index = this->cur_index_;
    do
    {
    	BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
    	JUDGE_RETURN(child_node != NULL, false);

        if (child_node->Evaluate(input) == true)
        {
            this->cur_index_ = next_index;
            return true;
        }

        next_index = (next_index + 1) % this->child_list_count_;
    } while (next_index != this->cur_index_);

    return false;
}

void SelectNode::DoTransition(NodeInputParam &input)
{
	BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
	child_node->Transition(input);
	this->set_status(child_node->status());
}

int SelectNode::DoTick(NodeOutputParam &output, NodeInputParam &input)
{
	BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
    int status = child_node->Tick(output, input);

    this->set_status(status);
    this->set_next_index();

    return status;
}

int SelectNode::set_next_index()
{
    JUDGE_RETURN(this->status() == BT::BEV_RS_FINISH, -1);
	this->cur_index_ = (this->cur_index_ + 1) % this->child_list_count_;
	return 0;
}

/*
 * PrioritySelectNode
 * */
PrioritySelectNode::PrioritySelectNode()
{
	this->cur_index_ = -1;
}

bool PrioritySelectNode::DoEvaluate(NodeInputParam &input)
{
    for (ChildNodeList::iterator iter = this->child_list_.begin();
    		iter != this->child_list_.end(); ++iter)
    {
        JUDGE_CONTINUE((*iter).__child->Evaluate(input) == true);

		this->cur_index_ = iter - this->child_list_.begin();
		return true;
    }

    return false;
}

void PrioritySelectNode::DoTransition(NodeInputParam &input)
{
	BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
	child_node->Transition(input);
	this->set_status(child_node->status());
}

int PrioritySelectNode::DoTick(NodeOutputParam &output, NodeInputParam &input)
{
	BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
    int status = child_node->Tick(output, input);

    this->set_status(status);
    return status;
}

/*
 * SequenceNode
 * */
void SequenceNode::reset(void)
{
    this->cur_index_ = 0;
    BehaviorNode::reset();
}

bool SequenceNode::DoEvaluate(NodeInputParam &input)
{
	JUDGE_RETURN(this->child_list_.empty() == false, false);

	BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
	return child_node->Evaluate(input);
}

void SequenceNode::DoTransition(NodeInputParam &input)
{
	BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
	child_node->Transition(input);

	this->set_status(child_node->status());
	this->set_next_index();
}

int SequenceNode::DoTick(NodeOutputParam &output, NodeInputParam &input)
{
    BehaviorNode *child_node = this->fetch_child_node(this->cur_index_);
    int status = child_node->Tick(output, input);

    this->set_status(status);
    this->set_next_index();

    return status;
}

int SequenceNode::set_next_index()
{
    JUDGE_RETURN(this->status() == BT::BEV_RS_FINISH, -1);
	this->cur_index_ = (this->cur_index_ + 1) % this->child_list_count_;
	return 0;
}

/*
 * ParallelSelectNode
 * */
void ParallelSelectNode::reset(void)
{
    this->child_executing_list_.clear();
    BehaviorNode::reset();
}

bool ParallelSelectNode::DoEvaluate(NodeInputParam &input)
{
	return true;
}

void ParallelSelectNode::DoTransition(NodeInputParam &input)
{
    this->set_status(BT::BEV_RS_FINISH);
}

int ParallelSelectNode::DoTick(NodeOutputParam &output, NodeInputParam &input)
{
	for (ChildNodeList::iterator iter = this->child_list_.begin();
			iter != this->child_list_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->__child->Evaluate(input) == true);
		iter->__child->Tick(output, input);
	}

    this->set_status(BT::BEV_RS_FINISH);
    return BT::BEV_RS_FINISH;
}

/*
 * ParallelSequenceNode
 * */
void ParallelSequenceNode::reset(void)
{
    this->child_executing_list_.clear();
}

bool ParallelSequenceNode::DoEvaluate(NodeInputParam &input)
{
    this->child_executing_list_.clear();
    bool evaluate = true;
    for (ChildNodeList::iterator iter = this->child_list_.begin();
    		iter != this->child_list_.end(); ++iter)
    {
        if (iter->__child->Evaluate(input) == true)
            this->child_executing_list_.push_back(iter->__child);
        else
            evaluate = false;
    }
    return evaluate;
}

void ParallelSequenceNode::DoTransition(NodeInputParam &input)
{
    int status = BT::BEV_RS_FINISH;
    for (ChildExecutingList::iterator iter = this->child_executing_list_.begin();
            iter != this->child_executing_list_.end(); ++iter)
    {
        if ((*iter)->is_executing() == false)
            continue;

        (*iter)->Transition(input);
        if ((*iter)->is_executing() == true)
        {
            status = BT::BEV_RS_EXECUTING;
        }
    }
    this->set_status(status);
}

int ParallelSequenceNode::DoTick(NodeOutputParam &output, NodeInputParam &input)
{
    int status = BT::BEV_RS_FINISH, tmp_status = 0;
    for (ChildExecutingList::iterator iter = this->child_executing_list_.begin(); 
            iter != this->child_executing_list_.end(); ++iter)
    {
        if ((*iter)->is_executing() == true)
        {
            tmp_status = (*iter)->Tick(output, input);
            if (tmp_status == BT::BEV_RS_EXECUTING)
            {
                status = tmp_status;
            }
        }
    }
    this->set_status(status);
    return status;
}

