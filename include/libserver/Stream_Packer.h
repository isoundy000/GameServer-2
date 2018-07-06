/*
 * Packer.h
 *
 *  Created on: Oct 26, 2012
 *      Author: ChenLong
 */

#ifndef STREAM_PACKER_H_
#define STREAM_PACKER_H_

#include "Thread.h"
#include "List.h"
#include "Svc.h"

class Stream_Packer: public Thread {
public:
	typedef List<int, Thread_Mutex> Cid_List;
	typedef std::vector<Block_Buffer *> Block_Vector;

	Stream_Packer(void);
	virtual ~Stream_Packer(void);

    virtual int thr_cancel_join(void);

	int push_packing_cid(int cid);

	virtual Svc *find_svc(int cid);

	virtual Block_Buffer *pop_block(int cid);

	virtual int push_block(int cid, Block_Buffer *block);

	virtual int packed_data_handler(Block_Vector &block_vec);

	virtual int drop_handler(int cid);

	int push_drop(int cid);

	int process_drop(void);

	virtual void run_handler(void);


private:
	int process(void);

	int process_packing_list(void);


private:
	Cid_List packing_list_;
	Cid_List drop_list_;
    bool is_running_;
};

#endif /* STREAM_PACKER_H_ */
