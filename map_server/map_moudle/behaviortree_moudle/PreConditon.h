/*
 * PreConditon.h
 *
 * Created on: 2013-05-17 18:19
 *     Author: lyz
 */

#ifndef _PRECONDITON_H_
#define _PRECONDITON_H_

#include "BehaviorNode.h"

class BevNodePreconditionNOT : public BevNodePrecondition
{
public:
    BevNodePreconditionNOT(void);
    virtual ~BevNodePreconditionNOT(void);

    virtual void reset(void);
    virtual bool ExternalCondition(NodeInputParam &input);

    void set_operand(BevNodePrecondition *condition);

protected:
    BevNodePrecondition *operand_;
};

class BevNodePreconditionAND : public BevNodePrecondition
{
public:
    BevNodePreconditionAND(void);
    virtual ~BevNodePreconditionAND(void);

    virtual void reset(void);
    virtual bool ExternalCondition(NodeInputParam &input);

    void set_operand_left(BevNodePrecondition *condition);
    void set_operand_right(BevNodePrecondition *condition);

protected:
    BevNodePrecondition *operand_left_;
    BevNodePrecondition *operand_right_;
};

class BevNodePreconditionOR : public BevNodePrecondition
{
public:
    BevNodePreconditionOR(void);
    virtual ~BevNodePreconditionOR(void);

    virtual void reset(void);
    virtual bool ExternalCondition(NodeInputParam &input);

    void set_operand_left(BevNodePrecondition *condition);
    void set_operand_right(BevNodePrecondition *condition);

protected:
    BevNodePrecondition *operand_left_;
    BevNodePrecondition *operand_right_;
};

class BevNodePreconditionXOR : public BevNodePrecondition
{
public:
    BevNodePreconditionXOR(void);
    virtual ~BevNodePreconditionXOR(void);

    virtual void reset(void);
    virtual bool ExternalCondition(NodeInputParam &input);

    void set_operand_left(BevNodePrecondition *condition);
    void set_operand_right(BevNodePrecondition *condition);

protected:
    BevNodePrecondition *operand_left_;
    BevNodePrecondition *operand_right_;
};

#endif //_PRECONDITON_H_
