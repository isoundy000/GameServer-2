/*
 * AIBTCFactory.h
 *
 *  Created on: Jun 21, 2013
 *      Author: peizhibi
 */

#ifndef AIBTCFACTORY_H_
#define AIBTCFACTORY_H_

#include "BTCFactory.h"

class AIBTCFactory : public BTCFactory
{
public:
	AIBTCFactory();
	virtual ~AIBTCFactory();

private:
	virtual BehaviorNode* create_action_node(const std::string& action_name,
			const Json::Value &node_json);
	virtual BevNodePrecondition* create_ext_precond(const std::string& precond_name,
			const Json::Value &node_json);
};

#endif /* AIBTCFACTORY_H_ */
