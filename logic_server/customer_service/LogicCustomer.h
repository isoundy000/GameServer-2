/*
 * LogicCustomer.h
 *
 *  Created on: Aug 11, 2014
 *      Author: louis
 */

#ifndef LOGICCUSTOMER_H_
#define LOGICCUSTOMER_H_

#include "BaseLogicPlayer.h"

class TQueryCursor;

class LogicCustomer : virtual public BaseLogicPlayer
{
public:
	LogicCustomer();
	virtual ~LogicCustomer();

	virtual int request_open_customer_service_pannel(Message* msg);
	virtual int request_open_replay_pannel(Message* msg);
	virtual int request_evaluate_customer_service_replay(Message*msg);
	virtual int summit_customer_service_record(Message*msg);
	virtual int request_read_customer_service_record(Message*msg);
	virtual int request_delete_customer_service_record(Message*msg);

	CustomerServiceDetail& customer_service_detial();
	void set_contact_way(std::string string);
	void reset();

	int customer_svc_time_up(void);
	int after_customer_svc_timeup_load_data(Transaction* trans);

	static Int64 fetch_global_customer_svc_record_index(void);
	static bool cmp_customer_svc_record(CustomerServiceRecord* lhs, CustomerServiceRecord* rhs);
	static bool need_delete(CustomerServiceRecord* record);

private:
	int notify_update_replay_pannel();
	int calc_unread_replay_record_amount(void);
	int auto_check_and_delete_expire_time_customer_svc_record(void);
	int sort_customer_service_replay_record(void);
	int sumit_customer_service_record(const std::string& title, const std::string& content, const int record_type,
			const int opinion_index = 0, const int evaluate_star = 5);
	int sumit_customer_service_record_to_backstage(CustomerServiceRecord* record);
	int delete_customer_service_record(const Int64 record_id);
	int delete_customer_service_record_to_backstage(CustomerServiceRecord* record);
	int bson2customer_service_record(auto_ptr<TQueryCursor> cursor, CustomerRecordVec& modify_set);

private:
	CustomerServiceDetail customer_service_detial_;
	std::string  contact_way_;
};

#endif /* LOGICCUSTOMER_H_ */
