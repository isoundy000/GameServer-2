/*
 * BaseUnit.cpp
 *
 * Created on: 2012-12-31 18:56
 *     Author: glendy
 */

#include "BaseUnit.h"
#include "PoolMonitor.h"

void BaseUnit::MessagePerformInfo::reset(void)
{
    this->__recogn = 0;
    this->__process_num = 0;
}

BaseUnit::BaseUnit(void) : is_running_(false), is_ready_stop_(false),
		recogn_msgperf_map_(5000), msg_perf_set_(5000),
		less_info_(0), total_msg_num_(0)
{ /*NULL*/ }

void BaseUnit::run_handler(void)
{
    this->prev_loop_init();
    this->total_msg_num_ = 0;
    this->less_info_ = 0;
    this->is_running_ = true;
    this->is_ready_stop_ = false;
    this->process_request();
    this->is_running_ = false;
}

int BaseUnit::push_request(const UnitMessage &msg)
{
    if (this->is_ready_stop_ == true)
        return -1;

    UnitMessage *n_msg = 0;
    if ((n_msg = this->pop_unit_message()) == 0)
        return -1;

    *n_msg = msg;
    return this->unit_msg_queue_.push(n_msg);
}

bool BaseUnit::is_running(void)
{
    return this->is_running_;
}

void BaseUnit::stop_wait(void)
{
    this->is_ready_stop_ = true;
    this->thr_join();
}

int BaseUnit::process_request(void)
{
    UnitMessage *msg = 0;
    timespec ts = {0, 1000000};
    while (this->is_ready_stop_ == false || this->unit_msg_queue_.size() > 0)
    {
        this->interval_run();
        if (this->unit_msg_queue_.size() > 5000 && this->type() != LOG_UNIT)
            MSG_USER("WARNING unit[%d] queue too big %d", this->type(), this->unit_msg_queue_.size());

    	if (this->unit_msg_queue_.size() <= 0 ||
            this->unit_msg_queue_.pop((void *&) msg) != 0)
        {
    		ts.tv_nsec = 1000000;
    		if (this->type() != GATE_BROAD_UNIT && this->type() != MAP_UNIT && this->type() != GATE_UNIT && this->type() != MAP_LOGIC_UNIT)
    			ts.tv_nsec = 50000000;
    		::nanosleep (&ts, 0);
    		continue;
        }

#ifndef LOCAL_DEBUG
        Time_Value begin = Time_Value::gettimeofday();
#endif

        int ret = this->process_block(msg);
        if (ret != 0)
        	MSG_USER("ERROR process recogn %d %d %d %ld", this->type(), msg->__msg_head.__recogn, ret, msg->__msg_head.__role_id);

#ifndef LOCAL_DEBUG
        Time_Value diff = Time_Value::gettimeofday() - begin;
        if ((diff.usec() + diff.sec() * 1000000L) > 10000)
        	MSG_USER("[WARNING] process too long %d %d %ld.%06d", this->type(), msg->__msg_head.__recogn, diff.sec(), diff.usec());
#endif

        this->push_message_data(*msg);
        this->push_unit_message(msg);
    }

    this->process_stop();

    return 0;
}

int BaseUnit::push_message_data(UnitMessage &msg)
{
    switch (msg.__type)
    {
        case UnitMessage::TYPE_BLOCK_BUFF:
            if (msg.__data.__buf != 0)
                POOL_MONITOR->push_buf_block(msg.__data.__buf, msg.__sid);
            msg.__data.__buf = 0;
            return 0;
        case UnitMessage::TYPE_PROTO_MSG:
        {
            if (msg.__data.__proto_msg != 0)
                delete msg.__data.__proto_msg;
            msg.__data.__proto_msg = 0;
            return 0;
        }
        default:
            return -1;
    }

    return 0;

}

int BaseUnit::prev_loop_init(void)
{
    return 0;
}

int BaseUnit::interval_run(void)
{
    return 0;
}

int BaseUnit::process_stop(void)
{
	this->unit_msg_queue_.clear();
    for (RecognMsgPerfMap::iterator iter = this->recogn_msgperf_map_.begin();
            iter != this->recogn_msgperf_map_.end(); ++iter)
    {
        this->msg_perf_pool_.push(iter->second);
    }
    this->recogn_msgperf_map_.clear();
    this->msg_perf_set_.clear();
    this->msg_perf_pool_.clear();
    return 0;
}

int BaseUnit::report_recogn_number(void)
{
    std::ostringstream msg_stream;
    for (MsgPerfSet::iterator iter = this->msg_perf_set_.begin();
            iter != this->msg_perf_set_.end(); ++iter)
    {
        MessagePerformInfo *info = *iter;
        msg_stream << "unit info:" << this->type() << " " << info->__recogn << " " << info->__process_num << std::endl;
    }
    MSG_USER("%s", msg_stream.str().c_str());
    return 0;
}

