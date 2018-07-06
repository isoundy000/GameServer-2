/*
 * CenterPostCache.h
 *
 *  Created on: 2014年1月20日
 *      Author: xing
 *      同步信息到 中央服
 */

#ifndef CENTERPOSTCACHE_H_
#define CENTERPOSTCACHE_H_

#include <SimpleHTTPClient.h>
#include "GameHeader.h"
#include "ObjectPoolEx.h"

typedef struct CenterRoleDetail_t_{
	std::string __name;
	std::string __account;
	std::string __agent;
	std::string __platform;
	Int64 __id;
	int __level;
	int __sex;
    std::string __server_flag;

	void reset(void);
}CenterRoleDetail;


class CenterPostCache {
public:

	typedef std::map<Int64, CenterRoleDetail *> ID_Detail_map;
	typedef ObjectPoolEx<CenterRoleDetail> Detail_pool;
//	typedef std::vector<ID_Detail_map* > MapList;
//	typedef ObjectPoolEx<ID_Detail_map> Map_pool;

	CenterPostCache();
	virtual ~CenterPostCache();
	int init();

	ID_Detail_map *detail_map(void);

	int insert_role_detail(Message *proto_msg);
	int insert_role_detail(CenterRoleDetail *detail);
	int post2center_role_all(bool retry=true);
	int post2center_role_once(bool retry); // retry: 如果失败是否下次重试
	bool is_need_post(void);
	std::string& php_url(void);

	int query_center_acti_code(const std::string &acti_code);
	int query_center_del_return_recharge(Int64 role_id, const std::string& account);
    int sync_sdk_info(Message *msg);

private:
//	MapList map_list_;
//	Map_pool map_pool_;
	ID_Detail_map role_detail_map_;
	Detail_pool role_detail_pool_;

	SimpleHTTPClient httpcli_;
//	std::string post_url_;
	std::string post_sign_;
	std::string server_flag_;
	std::string php_url_;
	uint post_once_limit_;

	bool initialize_;

};

typedef Singleton<CenterPostCache> CenterPostCacheSingle;
#define CENTER_POST_CACHE	CenterPostCacheSingle::instance()

#endif /* CENTERPOSTCACHE_H_ */
