/*
 * ChatStruct.h
 *
 * Created on: 2013-01-18 14:21
 *     Author: glendy
 */

#ifndef _CHATSTRUCT_H_
#define _CHATSTRUCT_H_

#include "PubStruct.h"

struct ChatRoleDetail:public BaseRoleInfo
{
    void reset(void);
    int __forbid_type;
    int __forbid_time;

    Int64 __sign_in;
    Int64 __last_sign_out;

    int __travel_area_id;

    string __league_name;
};

//聊天屏蔽
struct WordCheck
{
	StringVec words_;
	Int64 update_tick_;

	WordCheck(void);
	void reset(void);
};

// 后台配置的限制等级
struct ChatLimit
{
	int channel_type_;
	int limit_level_;
	int chat_interval_;
	Int64 update_tick_;

	ChatLimit(void);
	void reset(void);
};

//后台配置的vip等级限制聊天次数
struct VipLimit
{
	Int64 update_tick_;
	int time_;
	struct VipTimes
	{
		int vip_lv_;
		IntMap channel_times_map_;

		VipTimes();
		void reset();
	};
	typedef std::map<int, VipTimes> VipTimesMap;
	VipTimesMap vip_map_;

	VipLimit();
	void reset();
	VipTimes *fetch_vip_times(int vip_lv);
};

struct ChatTimes
{
	Int64 role_id_;
	IntMap channel_times_;

	ChatTimes();
	void reset();
};

class ChatRecord
{
public:
	ChatRecord(void);
	~ChatRecord(void);
	void reset(void);
	void copy(ChatRecord *record);
	int64_t __src_role_id;
	int64_t __dst_role_id;
	int __time;
	int __type;//聊天类型：1 文字 2：语音（没有语音内容） 3：语音内容
	int64_t __voice_id;
	int __voice_len;//语音时长
	int __sid;
//	int __is_offline;
	Block_Buffer* __buffer;
};

class FlauntRecord
{
public:
	Int64 __flaunt_id;
	int __flaunt_type;
	int __len;
	Block_Buffer* __buffer;

	FlauntRecord(void);
	~FlauntRecord(void);
	void reset(void);
};

#endif //_CHATSTRUCT_H_
