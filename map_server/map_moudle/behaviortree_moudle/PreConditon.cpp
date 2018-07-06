/*
 * PreConditon.cpp
 *
 * Created on: 2013-05-17 18:41
 *     Author: lyz
 */

#include "PreConditon.h"

/*
 * BevNodePreconditionNOT
 * */
BevNodePreconditionNOT::BevNodePreconditionNOT(void) :
		operand_(0) { /*NULL*/
}

BevNodePreconditionNOT::~BevNodePreconditionNOT(void) { /*NULL*/
}

void BevNodePreconditionNOT::reset(void) {
	this->operand_->reset();
}

void BevNodePreconditionNOT::set_operand(BevNodePrecondition *condition) {
	this->operand_ = condition;
}

bool BevNodePreconditionNOT::ExternalCondition(NodeInputParam &input) {
	return !(this->operand_ != 0
			&& this->operand_->ExternalCondition(input) == true);
}

/*
 * BevNodePreconditionAND
 * */
BevNodePreconditionAND::BevNodePreconditionAND(void) :
		operand_left_(0), operand_right_(0) { /*NULL*/
}
BevNodePreconditionAND::~BevNodePreconditionAND(void) { /*NULL*/
}

void BevNodePreconditionAND::reset(void) {
	if (this->operand_left_ != NULL) {
		this->operand_left_->reset();
	}

	if (this->operand_right_ != NULL) {
		this->operand_right_->reset();
	}
}

void BevNodePreconditionAND::set_operand_left(BevNodePrecondition *condition) {
	this->operand_left_ = condition;
}

void BevNodePreconditionAND::set_operand_right(BevNodePrecondition *condition) {
	this->operand_right_ = condition;
}

bool BevNodePreconditionAND::ExternalCondition(NodeInputParam &input) {
	return ((this->operand_left_ == 0
			|| this->operand_left_->ExternalCondition(input))
			&& (this->operand_right_ == 0
					|| this->operand_right_->ExternalCondition(input)));
}

/*
 * BevNodePreconditionOR
 * */
BevNodePreconditionOR::BevNodePreconditionOR(void) :
		operand_left_(0), operand_right_(0) { /*NULL*/
}

BevNodePreconditionOR::~BevNodePreconditionOR(void) { /*NULL*/
}

void BevNodePreconditionOR::reset(void) {
	if (this->operand_left_ != NULL) {
		this->operand_left_->reset();
	}

	if (this->operand_right_ != NULL) {
		this->operand_right_->reset();
	}
}

void BevNodePreconditionOR::set_operand_left(BevNodePrecondition *condition) {
	this->operand_left_ = condition;
}

void BevNodePreconditionOR::set_operand_right(BevNodePrecondition *condition) {
	this->operand_right_ = condition;
}

bool BevNodePreconditionOR::ExternalCondition(NodeInputParam &input) {
	return ((this->operand_left_ == 0
			|| this->operand_left_->ExternalCondition(input))
			|| (this->operand_right_ == 0
					|| this->operand_right_->ExternalCondition(input)));
}

/*
 * BevNodePreconditionXOR
 * */
BevNodePreconditionXOR::BevNodePreconditionXOR(void) :
		operand_left_(0), operand_right_(0) { /*NULL*/
}

BevNodePreconditionXOR::~BevNodePreconditionXOR(void) { /*NULL*/
}

void BevNodePreconditionXOR::reset(void) {
	if (this->operand_left_ != NULL) {
		this->operand_left_->reset();
	}

	if (this->operand_right_ != NULL) {
		this->operand_right_->reset();
	}
}

void BevNodePreconditionXOR::set_operand_left(BevNodePrecondition *condition) {
	this->operand_left_ = condition;
}

void BevNodePreconditionXOR::set_operand_right(BevNodePrecondition *condition) {
	this->operand_right_ = condition;
}

bool BevNodePreconditionXOR::ExternalCondition(NodeInputParam &input) {
	return ((this->operand_left_ == 0
			|| this->operand_left_->ExternalCondition(input))
			^ (this->operand_right_ == 0
					|| this->operand_right_->ExternalCondition(input)));
}
