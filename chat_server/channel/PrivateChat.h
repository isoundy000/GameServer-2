/*
 * PrivateChat.h
 *
 *  Created on: 2013-7-1
 *      Author: root
 */

#ifndef PRIVATECHAT_H_
#define PRIVATECHAT_H_
#include "BaseChannel.h"
#include <list>
#include <map>

class PrivateChat:public BaseChannel
{
public:
	friend class MMOChatPrivate;
	typedef std::list<ChatRecord*> RecordList;
	typedef std::map<int64_t,RecordList> RecordMap;
	typedef std::map<int64_t,int64_t> IntMap;

	PrivateChat();
	virtual ~PrivateChat();

	int init(ChannelAgency *agency,int64_t channel_id=0,bool offline=false);
	int stop(void);
	int suspend(void);
	virtual int channel_type(void);
	virtual int notify(void);
	virtual void reset(void);

	int notify_offline(void);
	int recv_record(ChatRecord* record);
	void import_offline_record(void);

	int add_black_list(int64_t role_id);
	int remove_black_list(int64_t role_id);
	bool is_in_black_list(int64_t role_id);

	int add_friend_list(Int64 role_id);
	int remove_friend_list(Int64 role_id);
	bool is_in_friend_list(Int64 role_id);

	int restart(void);
	bool is_suspend(void);
	int get_history(RecordList& list,int64_t role_id,int time_offset);

	virtual int push_voice(ChatRecord* record,bool history=false);
	virtual ChatRecord* get_voice(int64_t id);

private:
	int push_history(int64_t role_id,ChatRecord *record);
	int clear_history(void);
	int load_record(ChatRecord* record);
	int save_record(void);

	RecordMap record_map_;
	RecordList record_load_list_;//用来加载数据(数据库线程存，逻辑线程取)
	RecordList record_save_list_;//用来保存数据（逻辑线程存，数据库线程取）

	bool data_change_;
	Time_Value last_save_;
	bool is_supend_;

	IntMap black_map_;//黑名单
	IntMap friend_map_;

//	RecordList voice_list_;
};

#endif /* PRIVATECHAT_H_ */
