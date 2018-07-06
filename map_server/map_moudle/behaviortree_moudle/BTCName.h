/*
 * BTCName.h
 *
 *  Created on: May 23, 2013
 *      Author: peizhibi
 */

#ifndef BTCNAME_H_
#define BTCNAME_H_

#include <string>

struct BTCName
{
	/*
	 * node type
	 * */
	static std::string ControlNode;
	static std::string ActionNode;

	/*
	 * control node
	 * */
	static std::string PrioSelNode;
	static std::string SelNode;
	static std::string ParaSelNode;
	static std::string SequenceNode;

	/*
	 * tree property info
	 * */
	static std::string NodeName;
	static std::string NodeType;
	static std::string NodeAction;
	static std::string EXTPRECOND;
	static std::string NodeTree;
	static std::string LinkFile;
    static std::string ExtFieldName1;
    static std::string ActionFieldName1;

	/*
	 * logic relation
	 * */
	static std::string NodeLogicAnd;
	static std::string NodeLogicOr;
    static std::string NodeLogicNot;
};

#endif /* BTCNAME_H_ */
