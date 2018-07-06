/*
 * Client_Svc.h
 *
 *  Created on: Oct 10, 2012
 *      Author: ChenLong
 */

#ifndef SVC_H_
#define SVC_H_

#include "Event_Handler.h"
#include "Block_List.h"
#include "Thread_Mutex.h"

class Block_Buffer;

class Svc: public Event_Handler {
public:
	typedef Block_List<Thread_Mutex> Data_Block_List;
	typedef std::vector<Block_Buffer *> Block_Vector;

	const static int MAX_LIST_SIZE = 1000;
	const static int MAX_PACK_SIZE = 60 * 1024;

	const static char req_flash_policy[];
	const static char res_flash_policy[];
	const static int MAX_TRY_RES_FLASH_POLICY = 10;

	Svc(void);

	virtual ~Svc(void);

	void reset(void);

	void clear_send_block(void);

	virtual Block_Buffer *pop_block(int cid);

	virtual int push_block(int cid, Block_Buffer *block);

	virtual int register_recv_handler(void);

	virtual int unregister_recv_handler(void);

	virtual int register_send_handler(void);

	virtual int unregister_send_handler(void);

	virtual int recv_handler(int cid);

	virtual int close_handler(int cid);

	virtual int handle_input(void);

	virtual int handle_close(void);

	int close_fd(void);

	int recv_data(void);

	int send_data(void);

	int check_flash_policy(Block_Buffer *buf);

	int pack_recv_data(Block_Vector &block_vec);

	int push_recv_block(Block_Buffer *buf);

	int push_send_block(Block_Buffer *buf);

	void set_cid(int cid);

	int get_cid(void);

	bool get_reg_recv(void);

	void set_reg_recv(bool val);

	bool get_reg_send(void);

	void set_reg_send(bool val);

	bool is_closed(void);

	void set_closed(bool v);

	int get_peer_addr(std::string &ip, int &port);

	int get_local_addr(std::string &ip, int &port);

	void set_max_list_size(size_t max_size);
    void set_max_recv_list_size(size_t max_size);
    void set_max_send_list_size(size_t max_size);

	void set_max_pack_size(size_t max_size);

	void set_flash_policy(bool v);

	bool get_flash_policy(void);

	void set_peer_addr(void);
	virtual void reset_max_recv_list_size(void);

	bool is_send_list_empty(void);

protected:
	int cid_;
	int restore_;

	Data_Block_List recv_block_list_;
	Data_Block_List send_block_list_;

    size_t max_recv_list_size_;
    size_t max_send_list_size_;
	size_t max_pack_size_;

	bool is_closed_;
	bool is_reg_recv_, is_reg_send_;

	std::string peer_ip_;
	int peer_port_;

	bool writeable_;
	bool validate_flash_policy_; /// 是否启用flash安全沙箱验证
	
	Block_Buffer *cur_buf_;
    Block_Buffer *front_buf_;
    int front_cid_;

    Thread_Mutex close_mutex_;	//关闭锁
};

////////////////////////////////////////////////////////////////////////////////

inline void Svc::reset(void) {
	Data_Block_List::BList blist;

	blist.clear();
	recv_block_list_.swap(blist);
	for (Data_Block_List::BList::iterator it = blist.begin(); it != blist.end(); ++it) {
		push_block(cid_, *it);
	}

	blist.clear();
	send_block_list_.swap(blist);
	for (Data_Block_List::BList::iterator it = blist.begin(); it != blist.end(); ++it) {
		push_block(cid_, *it);
	}

    if (this->front_buf_ != 0)
    {
        if (this->front_cid_ != 0)
            this->push_block(this->front_cid_, this->front_buf_);
        else
        	this->push_block(this->cid_, this->front_buf_);
        this->front_buf_ = 0;
        this->front_cid_ = 0;
    }

    if (this->cur_buf_ != 0)
    {
        this->push_block(this->cid_, this->cur_buf_);
        this->cur_buf_ = 0;
    }

	cid_ = 0;
	restore_ = 0;
	is_closed_ = false;
	is_reg_recv_ = false;
	is_reg_send_ = false;

	max_recv_list_size_ = MAX_LIST_SIZE;
    max_send_list_size_ = MAX_LIST_SIZE;
	max_pack_size_ = MAX_PACK_SIZE;

	peer_ip_.clear();
	peer_port_ = 0;

	writeable_ = true;
	validate_flash_policy_ = false;

	Event_Handler::reset();
}

inline void Svc::clear_send_block(void) {
	Data_Block_List::BList blist;
	send_block_list_.swap(blist);

	for (Data_Block_List::BList::iterator it = blist.begin(); it != blist.end(); ++it) {
		push_block(cid_, *it);
	}
}

inline int Svc::push_recv_block(Block_Buffer *buf) {
	if (is_closed_)
		return -1;

	if (recv_block_list_.size() >= max_recv_list_size_) {
		LOG_USER_INFO("recv_block_list_ has full. %d %d", recv_block_list_.size(), max_recv_list_size_);
		return -1;
	}
	reset_max_recv_list_size();
	recv_block_list_.push_back(buf);
	return 0;
}

inline int Svc::push_send_block(Block_Buffer *buf) {
	if (is_closed_)
		return -1;

	if (send_block_list_.size() >= max_send_list_size_) {
		LOG_USER_INFO("ERROR send_block_list_ has full send_block_list_.size() = %d, max_send_list_size = %d, fd[%d]", send_block_list_.size(), max_send_list_size_, this->get_fd());
		handle_close();
		return -1;
	}
	send_block_list_.push_back(buf);
	return 0;
}

inline void Svc::set_cid(int cid) {
	cid_ = cid;
}

inline int Svc::get_cid(void) {
	return cid_;
}

inline bool Svc::get_reg_recv(void) {
	return is_reg_recv_;
}

inline void Svc::set_reg_recv(bool val) {
	is_reg_recv_ = val;
}

inline bool Svc::get_reg_send(void) {
	return is_reg_send_;
}

inline void Svc::set_reg_send(bool val) {
	is_reg_send_ = val;
}

inline bool Svc::is_closed(void) {
	return is_closed_;
}

inline void Svc::set_closed(bool v) {
	is_closed_ = v;
}

inline void Svc::set_max_list_size(size_t max_size) {
	max_send_list_size_ = max_size;
    max_recv_list_size_ = max_size;
}

inline void Svc::set_max_recv_list_size(size_t max_size) {
    max_recv_list_size_ = max_size;
}

inline void Svc::set_max_send_list_size(size_t max_size) {
    max_send_list_size_ = max_size;
}

inline void Svc::set_max_pack_size(size_t max_size) {
	max_pack_size_ = max_size;
}

inline void Svc::set_flash_policy(bool v) {
	validate_flash_policy_ = v;
}

inline bool Svc::get_flash_policy(void) {
	return validate_flash_policy_;
}

inline void Svc::set_peer_addr(void) {
	get_peer_addr(peer_ip_, peer_port_);
}

inline void Svc::reset_max_recv_list_size(void) {
	return;
}

inline bool Svc::is_send_list_empty(void) {
	return this->send_block_list_.empty();
}

#endif /* SVC_H_ */
