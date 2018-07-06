/*
 * BTCFactory.cpp
 *
 *  Created on: May 23, 2013
 *      Author: peizhibi
 */

#include "BTCFactory.h"

#include "BTCName.h"
#include "ControlNode.h"
#include "GameConfig.h"
#include "PreConditon.h"
#include "AIBTCFactory.h"

BTCFactory* BTCFactory::instance_ = NULL;

int BTCFactory::init()
{
	const Json::Value& bt_factory = CONFIG_INSTANCE->bt_factory();

	for (uint i = 0; i < bt_factory["tree_type"].size(); ++i)
	{
		std::string tree_name = bt_factory["tree_type"][i].asString();
		JUDGE_CONTINUE(this->tree_type_map_.count(tree_name) == 0);

		this->tree_type_map_[tree_name] = i;

		BTTreePool* tree_pool = new BTTreePool(tree_name);
		this->bt_tree_pool_.push_back(tree_pool);
	}

	return 0;
}

int BTCFactory::fina()
{
	return 0;
}

BTCFactory* BTCFactory::instance()
{
	if (BTCFactory::instance_ == NULL)
	{
		BTCFactory::instance_ = new AIBTCFactory;
//		BTCFactory::instance_->init();
	}

	return BTCFactory::instance_;
}

BehaviorNode* BTCFactory::pop_ai_tree(const std::string& tree_name)
{
	JUDGE_RETURN(this->tree_type_map_.count(tree_name) > 0, NULL);

	int tree_type_index = this->tree_type_map_[tree_name];
	BehaviorNode* bev_tree = this->bt_tree_pool_[tree_type_index]->pop();

	bev_tree->reset();
	return bev_tree;
}

int BTCFactory::push_ai_tree(const std::string& tree_name, BehaviorNode* ai_tree)
{
	JUDGE_RETURN(ai_tree != NULL, -1);
	JUDGE_RETURN(this->tree_type_map_.count(tree_name) > 0, -1);

	int tree_type_index = this->tree_type_map_[tree_name];
	return this->bt_tree_pool_[tree_type_index]->push(ai_tree);
}

BehaviorNode* BTCFactory::create_ai_tree(const std::string &tree_name)
{
	const Json::Value& ai_tree = CONFIG_INSTANCE->behavior(tree_name);
	JUDGE_RETURN(ai_tree != Json::Value::null, NULL);
	return this->create_ai_tree(ai_tree);
}

BehaviorNode* BTCFactory::create_ai_tree(const Json::Value &ai_tree)
{
	std::string node_type = ai_tree[BTCName::NodeType].asString();
	if (node_type == BTCName::ControlNode)
	{
		// control node
		return this->create_control_node(ai_tree);
	}

	if (node_type == BTCName::ActionNode)
	{
		// action node
		return this->create_action_node(ai_tree);
	}

	MSG_USER("ERROR NULL node_type");

	return NULL;
}

BehaviorNode* BTCFactory::create_control_node(const Json::Value &ai_tree)
{
	std::string node_action = ai_tree[BTCName::NodeAction].asString();

	// create control node
	BehaviorNode* node = this->create_control_node(node_action);
	JUDGE_RETURN(node != NULL, NULL);

	// add ext precondition
	this->add_ext_precond(node, ai_tree);

	// add sub node
	int node_count = ai_tree[BTCName::NodeTree].size();
	for (int i = 0; i < node_count; ++i)
	{
		BehaviorNode* sub_node = this->create_ai_tree(
				ai_tree[BTCName::NodeTree][i]);

		node->push_child_node(sub_node);
		sub_node->set_parent(node);
		node->copy_action_type_flag(sub_node);
	}

	return node;
}

BehaviorNode* BTCFactory::create_action_node(const Json::Value &ai_tree)
{
	std::string node_action = ai_tree[BTCName::NodeAction].asString();

	// create action node
	BehaviorNode* node = this->create_action_node(node_action, ai_tree);
	JUDGE_RETURN(node != NULL, NULL);

    if (ai_tree.isMember(BTCName::ActionFieldName1))
    {
        node->set_field_name_1(ai_tree[BTCName::ActionFieldName1].asString());
    }
	// add ext precondition
	this->add_ext_precond(node, ai_tree);
	return node;
}

int BTCFactory::add_ext_precond(BehaviorNode* node, const Json::Value &ai_tree)
{
	JUDGE_RETURN(node != NULL, -1);
	JUDGE_RETURN(ai_tree.isMember(BTCName::EXTPRECOND) == true, -1);

	BevNodePrecondition* ext_precond = this->create_ext_precond(
			ai_tree[BTCName::EXTPRECOND], ai_tree); 
	JUDGE_RETURN(ext_precond != NULL, -1);

    if (ai_tree.isMember(BTCName::ExtFieldName1))
    {
        ext_precond->set_field_name_1(ai_tree[BTCName::ExtFieldName1].asString());
    }
	node->set_precondition(ext_precond);

	return 0;
}

BevNodePrecondition* BTCFactory::create_ext_precond(const Json::Value &precond,
		const Json::Value &node_json)
{
	if (precond.isString() == true)
	{
		return this->create_ext_precond(precond.asString(), node_json);
	}

	if (precond.isArray() == true)
	{
		return this->create_logic_precond(precond, node_json);
	}

	return NULL;
}

BevNodePrecondition* BTCFactory::create_logic_precond(const Json::Value &precond,
		const Json::Value &node_json)
{
	std::string logic_relate = precond[0u].asString();
	if (logic_relate == BTCName::NodeLogicAnd)
	{
		BevNodePreconditionAND* cond_and = new BevNodePreconditionAND;

		BevNodePrecondition* operand_left = this->create_ext_precond(precond[1u], node_json);
		BevNodePrecondition* operand_right = this->create_ext_precond(precond[2u], node_json);

		cond_and->set_operand_left(operand_left);
		cond_and->set_operand_right(operand_right);

		return cond_and;
	}

	if (logic_relate == BTCName::NodeLogicOr)
	{
		BevNodePreconditionOR* cond_or = new BevNodePreconditionOR;

		BevNodePrecondition* operand_left = this->create_ext_precond(precond[1u], node_json);
		BevNodePrecondition* operand_right = this->create_ext_precond(precond[2u], node_json);

		cond_or->set_operand_left(operand_left);
		cond_or->set_operand_right(operand_right);

		return cond_or;
	}

	if (logic_relate == BTCName::NodeLogicNot)
	{
		BevNodePreconditionNOT* cond_not = new BevNodePreconditionNOT;

		BevNodePrecondition* operand_left = this->create_ext_precond(precond[1u], node_json);
		cond_not->set_operand(operand_left);

		return cond_not;
	}

	return NULL;
}

BehaviorNode* BTCFactory::create_control_node(const std::string& control_name)
{
	BehaviorNode* control_node = NULL;

	if (control_name == BTCName::PrioSelNode)
	{
		control_node = new PrioritySelectNode;
	}
	else if (control_name == BTCName::SelNode)
	{
		control_node = new SelectNode;
	}
	else if (control_name == BTCName::ParaSelNode)
	{
		control_node = new ParallelSelectNode;
	}
	else if (control_name == BTCName::SequenceNode)
	{
		control_node = new SequenceNode;
	}
	else
	{
		MSG_USER("Error Control Name %s", control_name.c_str());
	}

	control_node->set_node_name(control_name);
	return control_node;
}


