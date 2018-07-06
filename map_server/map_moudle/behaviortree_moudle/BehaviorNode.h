/*
 * BehaviorNode.h
 *
 * Created on: 2013-05-17 15:52
 *     Author: lyz
 */

#ifndef _BEHAVIORNODE_H_
#define _BEHAVIORNODE_H_

#include "BehaviorStruct.h"

/*
 * BevNodePrecondition
 * */
class BevNodePrecondition {
public:
	virtual ~BevNodePrecondition() {
	}

	virtual void reset(void) { /*NULL*/
	}
	;
	virtual bool ExternalCondition(NodeInputParam &input) = 0;

	void set_pre_name(const std::string& pre_name);
	virtual void set_field_name_1(const std::string &name);
	virtual std::string &field_name_1(void);

private:
	std::string pre_name_;
	std::string field_name_1_;
};

/*
 * BehaviorNode
 * */
class BehaviorNode {
public:
	typedef std::vector<BevChildNode> ChildNodeList;
	typedef std::bitset<GameEnum::AI_ACTION_TYPE_END> AIActionTypeFlag;

public:
	BehaviorNode(void);
	virtual ~BehaviorNode(void);
	virtual void reset(void);

	void set_parent(BehaviorNode *parent);
	void set_precondition(BevNodePrecondition *condition);

	BehaviorNode* fetch_child_node(int child_index);
	void push_child_node(BehaviorNode *child, int priority = 0);

	int status(void);
	void set_status(int status);
	void set_node_name(const std::string& node_name);

	bool is_executing(void);
	bool check_bev_run_status(int run_status);
	bool is_validate_index(int child_index);
	bool is_validate_node(long update_tick);

	// 节点的选择条件: DoEvaluate是内部条件，precondition是外部条件
	bool Evaluate(NodeInputParam &input);
	// 当前节点未处理完成，但需要切换到其他同级节点调用
	void Transition(NodeInputParam &input);
	// 当Evaluate返回true时调用此接口执行当前节点的逻辑
	int Tick(NodeOutputParam &output, NodeInputParam &input);

	void set_field_name_1(const std::string &name);
	std::string &field_name_1(void);

	void set_action_type_flag(const int flag);
	void reset_action_type_flag(const int flag);
	bool test_ation_type_flag(const int flag);
	void copy_action_type_flag(BehaviorNode *node);

protected:
	virtual bool DoEvaluate(NodeInputParam &input);
	virtual void DoTransition(NodeInputParam &input);
	virtual int DoTick(NodeOutputParam &output, NodeInputParam &input);

protected:
	std::string node_name_;
	std::string field_name_1_;

	int child_list_count_;
	ChildNodeList child_list_;

private:
	long create_tick_;
	int current_status_;

	BehaviorNode *parent_;
	BevNodePrecondition *precondition_;
	AIActionTypeFlag *action_type_flag_;
};

/*
 * BevActionNode
 * */
class BevActionNode: public BehaviorNode {
protected:
	virtual int DoTick(NodeOutputParam &output, NodeInputParam &input);

	virtual void DoEnter(NodeInputParam &input);
	virtual void DoExit(NodeInputParam &input, int exit_status);
	virtual int DoExecute(NodeOutputParam &output, NodeInputParam &input);
};

#endif //_BEHAVIORNODE_H_
