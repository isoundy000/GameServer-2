/*
 * BehaviorNode.cpp
 *
 * Created on: 2013-05-17 16:03
 *     Author: lyz
 */

#include "BehaviorNode.h"

void BevNodePrecondition::set_pre_name(const std::string& pre_name)
{
	this->pre_name_ = pre_name;
}

void BevNodePrecondition::set_field_name_1(const std::string &name)
{
    this->field_name_1_ = name;
}

std::string &BevNodePrecondition::field_name_1(void)
{
    return this->field_name_1_;
}

/*
 * BehaviorNode
 * */
BehaviorNode::BehaviorNode(void)
{
    this->child_list_count_ = 0;

    this->create_tick_ = ::time(0);
    this->current_status_ = 0;
	this->parent_ = 0;
	this->precondition_ = 0;
    this->action_type_flag_ = new AIActionTypeFlag();
}

BehaviorNode::~BehaviorNode(void)
{
    SAFE_DELETE(this->action_type_flag_);
}

void BehaviorNode::set_parent(BehaviorNode *parent)
{
    this->parent_ = parent;
}

void BehaviorNode::set_precondition(BevNodePrecondition *condition)
{
    this->precondition_ = condition;
}

void BehaviorNode::set_status(int status)
{
    this->current_status_ = status;
}

void BehaviorNode::set_node_name(const std::string& node_name)
{
	this->node_name_ = node_name;
}

int BehaviorNode::status(void)
{
    return this->current_status_;
}

bool BehaviorNode::is_executing(void)
{
	return this->check_bev_run_status(BT::BEV_RS_EXECUTING);
}

bool BehaviorNode::check_bev_run_status(int run_status)
{
	return this->current_status_ == run_status;
}

bool BehaviorNode::DoEvaluate(NodeInputParam &input)
{
    return true;
}

void BehaviorNode::Transition(NodeInputParam &input)
{
    this->DoTransition(input);
}

void BehaviorNode::DoTransition(NodeInputParam &input)
{
}

int BehaviorNode::Tick(NodeOutputParam &output, NodeInputParam &input)
{
    return this->DoTick(output, input);
}

int BehaviorNode::DoTick(NodeOutputParam &output, NodeInputParam &input)
{
    return BT::BEV_RS_FINISH;
}

bool BehaviorNode::is_validate_index(int child_index)
{
	return child_index >= 0 && child_index < this->child_list_count_;
}

bool BehaviorNode::is_validate_node(long update_tick)
{
	return this->create_tick_ > update_tick;
}

BehaviorNode* BehaviorNode::fetch_child_node(int child_index)
{
	JUDGE_RETURN(this->is_validate_index(child_index) == true, NULL);
	return this->child_list_[child_index].__child;
}

void BehaviorNode::push_child_node(BehaviorNode *child, int priority)
{
    this->child_list_.push_back(BevChildNode(priority, child));
    this->child_list_count_ += 1;
}

void BehaviorNode::reset(void)
{
    if (this->precondition_ != NULL)
	{
    	this->precondition_->reset();
	}

    this->current_status_ = BT::BEV_RS_IDL;
    this->child_list_count_ = this->child_list_.size();
    this->action_type_flag_->reset();

    for (ChildNodeList::iterator iter = this->child_list_.begin();
    		iter != this->child_list_.end(); ++iter)
    {
    	BehaviorNode* node = iter->__child;
    	node->reset();
    }
}

bool BehaviorNode::Evaluate(NodeInputParam &input)
{
    int ext_ret = true;
    if (this->precondition_ != NULL)
    {
    	ext_ret =  this->precondition_->ExternalCondition(input);
    }

    return ext_ret && this->DoEvaluate(input);
}

void BehaviorNode::set_field_name_1(const std::string &name)
{
    this->field_name_1_ = name;
}

std::string &BehaviorNode::field_name_1(void)
{
    return this->field_name_1_;
}

void BehaviorNode::set_action_type_flag(const int flag)
{
    JUDGE_RETURN(GameEnum::AAT_NO_FLAG < flag && flag < GameEnum::AI_ACTION_TYPE_END, ;);
    this->action_type_flag_->set(flag);
}

void BehaviorNode::reset_action_type_flag(const int flag)
{
    JUDGE_RETURN(GameEnum::AAT_NO_FLAG < flag && flag < GameEnum::AI_ACTION_TYPE_END, ;);
    this->action_type_flag_->reset(flag);
}

bool BehaviorNode::test_ation_type_flag(const int flag)
{
    JUDGE_RETURN(GameEnum::AAT_NO_FLAG < flag && flag < GameEnum::AI_ACTION_TYPE_END, false);
    return this->action_type_flag_->test(flag);
}

void BehaviorNode::copy_action_type_flag(BehaviorNode *node)
{
    for (int i = GameEnum::AAT_NO_FLAG; i < GameEnum::AI_ACTION_TYPE_END; ++i)
        (*(this->action_type_flag_))[i] = (node->test_ation_type_flag(i) ? 1 : 0);
}

/*
 * BevActionNode
 * */
int BevActionNode::DoTick(NodeOutputParam &output, NodeInputParam &input)
{
    this->DoEnter(input);

    int exit_status = this->DoExecute(output, input);
    this->DoExit(input, exit_status);

    return exit_status;
}

void BevActionNode::DoEnter(NodeInputParam &input)
{
    return;
}

void BevActionNode::DoExit(NodeInputParam &input, int exit_status)
{
    return;
}

int BevActionNode::DoExecute(NodeOutputParam &output, NodeInputParam &input)
{
    return BT::BEV_RS_FINISH;
}



