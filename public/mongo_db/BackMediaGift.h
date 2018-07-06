/*
 * BackMediaGift.h
 *
 *  Created on: Aug 9, 2014
 *      Author: root
 */

#ifndef BACKMEDIAGIFT_H_
#define BACKMEDIAGIFT_H_

#include "MongoTable.h"
#include "MapLogicStruct.h"

class BackRestriction : public MongoTable
{
public:
	BackRestriction();
	virtual ~BackRestriction();

protected:
    virtual void ensure_all_index(void);
};

class BackMediaGift : public MongoTable
{
public:
	BackMediaGift();
	virtual ~BackMediaGift();

	int load_acti_code(ActiCodeDetail* acti_code_detail);
	static int update_acti_code(MongoDataMap* data_map, ActiCodeDetail* acti_code_detail);
	static int update_gift_config(MediaGiftDefMap& gift_map, int &last_update_tick);
	static int load_download_box_gift(int agent_code,std::vector<ItemObj>& item_list,string& url);

protected:
	void ensure_all_index(void);
};

#endif /* BACKMEDIAGIFT_H_ */
