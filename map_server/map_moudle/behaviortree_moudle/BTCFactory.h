/*
 * BTCFactory.h
 *
 *  Created on: May 23, 2013
 *      Author: peizhibi
 */

#ifndef BTCFACTORY_H_
#define BTCFACTORY_H_

#include "BehaviorNode.h"
#include "BTTreePool.h"
#include "json/json.h"

/*
 * Behavior Tree Component Factory
 * */
class BTCFactory
{
public:
	virtual ~BTCFactory(){}

	int init();
	int fina();

	static BTCFactory* instance();

	BehaviorNode* pop_ai_tree(const std::string& tree_name);
	int push_ai_tree(const std::string& tree_name, BehaviorNode* ai_tree);

	BehaviorNode* create_ai_tree(const std::string &tree_name);

private:
	BehaviorNode* create_ai_tree(const Json::Value &ai_tree);

	BehaviorNode* create_control_node(const Json::Value &ai_tree);
	BehaviorNode* create_action_node(const Json::Value &ai_tree);
	int add_ext_precond(BehaviorNode* node, const Json::Value &ai_tree);

	BehaviorNode* create_control_node(const std::string& control_name);
	virtual BehaviorNode* create_action_node(const std::string& action_name,
			const Json::Value &node_json) = 0;

	BevNodePrecondition* create_ext_precond(const Json::Value &precond,
			const Json::Value &node_json);
	BevNodePrecondition* create_logic_precond(const Json::Value &precond,
			const Json::Value &node_json);
	virtual BevNodePrecondition* create_ext_precond(const std::string& precond_name,
			const Json::Value &node_json) = 0;

private:
	static BTCFactory* instance_;

	std::map<std::string, int> tree_type_map_;
	std::vector<BTTreePool*> bt_tree_pool_;
};

#define 	BTCFACTORY		BTCFactory::instance()

#endif /* BTCFACTORY_H_ */
