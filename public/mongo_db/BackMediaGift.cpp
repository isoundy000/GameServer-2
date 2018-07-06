/*
 * BackMediaGift.cpp
 *
 *  Created on: Aug 9, 2014
 *      Author: root
 */

#include "BackField.h"
#include "BackMediaGift.h"

#include "DBCommon.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

BackRestriction::BackRestriction() {
	// TODO Auto-generated constructor stub

}

BackRestriction::~BackRestriction() {
	// TODO Auto-generated destructor stub
}

void BackRestriction::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBBackRestriction::COLLECTION, BSON(DBBackRestriction::ID << 1), true);
	this->conection().ensureIndex(DBBackRestriction::COLLECTION, BSON(DBBackRestriction::FLAG << 1));

	this->conection().ensureIndex(DBBanIpInfo::COLLECTION, BSON(DBBanIpInfo::IP_UINT << 1), true);
	this->conection().ensureIndex(DBWhiteIpInfo::COLLECTION, BSON(DBWhiteIpInfo::IP_UINT << 1), true);
	this->conection().ensureIndex(DBBanMacInfo::COLLECTION, BSON(DBBanMacInfo::MAC_STRING << 1), true);

END_CATCH
}


BackMediaGift::BackMediaGift() {
	// TODO Auto-generated constructor stub

}

BackMediaGift::~BackMediaGift() {
	// TODO Auto-generated destructor stub
}

int BackMediaGift::load_acti_code(ActiCodeDetail* acti_code_detail)
{
	JUDGE_RETURN(0 != acti_code_detail, -1);

	BSONObj res = this->conection().findOne(DBBackActiCode::COLLECTION,
			QUERY(DBBackActiCode::ACTI_CODE << acti_code_detail->__acti_code));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	acti_code_detail->__amount = res[DBBackActiCode::AMOUNT].numberInt();
	acti_code_detail->__gift_sort = res[DBBackActiCode::GIFT_SORT].numberInt();
	acti_code_detail->__start_time = res[DBBackActiCode::START_TIME].numberInt();
	acti_code_detail->__end_time = res[DBBackActiCode::END_TIME].numberInt();
	acti_code_detail->__batch_id = res[DBBackActiCode::BATCH_ID].numberInt();
	acti_code_detail->__use_only_vip = res[DBBackActiCode::USE_ONLY_VIP].numberInt();
	return 0;

}

int BackMediaGift::update_gift_config(MediaGiftDefMap& gift_map, int &last_update_tick)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBBackMediaGiftDef::COLLECTION,
			QUERY(DBBackMediaGiftDef::UPDATE_STATUS << 1));

	if(res.isEmpty() == false)
	{
		int db_update_tick = res[DBBackMediaGiftDef::UPDATE_TICK].numberInt();
		JUDGE_RETURN(last_update_tick < db_update_tick, -1);

		last_update_tick = db_update_tick;
	}

	gift_map.clear();

	BSONVec bson_set;
	CACHED_CONNECTION.findN(bson_set, DBBackMediaGiftDef::COLLECTION,
			QUERY(DBBackMediaGiftDef::GIFT_ITEMS << BSON("$exists"<< true)), 100);

	for(BSONVec::iterator iter = bson_set.begin(); iter != bson_set.end(); ++iter)
	{
		BSONObj &obj = *iter;
		MSG_USER("%s", obj.toString().c_str());

		int gift_sort = obj[DBBackMediaGiftDef::GIFT_SORT].numberInt();

		MediaGiftDef &gift_def = gift_map[gift_sort];
		gift_def.__gift_sort = gift_sort;
		gift_def.__gift_type = obj[DBBackMediaGiftDef::GIFT_TYPE].numberInt();
		gift_def.__gift_tag = obj[DBBackMediaGiftDef::GIFT_TAG].numberInt();
		gift_def.__use_times = obj[DBBackMediaGiftDef::USE_TIMES].numberInt();
		gift_def.__show_icon = obj[DBBackMediaGiftDef::SHOW_ICON].numberInt();
		gift_def.__hide_used = obj[DBBackMediaGiftDef::HIDE_USED].numberInt();
		gift_def.__is_share = obj[DBBackMediaGiftDef::IS_SHARE].numberInt();
		gift_def.__expire_time = obj[DBBackMediaGiftDef::EXPIRE_TIME].numberInt();

		::strncpy(gift_def.__gift_name, obj[DBBackMediaGiftDef::GIFT_NAME].str().c_str(), MAX_COMMON_NAME_LENGTH);
		gift_def.__gift_name[MAX_COMMON_NAME_LENGTH] = '\0';

		::strncpy(gift_def.__gift_desc, obj[DBBackMediaGiftDef::GIFT_DESC].str().c_str(), MAX_MEDIA_GIFT_DESC_LENGTH);
		gift_def.__gift_desc[MAX_MEDIA_GIFT_DESC_LENGTH] = '\0';

		DBCommon::bson_to_item_vec(gift_def.__gift_list,
	    		obj.getObjectField(DBBackMediaGiftDef::GIFT_ITEMS.c_str()));

	    DBCommon::bson_to_int_vec(gift_def.__font_color,
	    		obj.getObjectField(DBBackMediaGiftDef::FONT_COLOR.c_str()));

	    GameCommon::bson_to_map(gift_def.__value_ext,
	    		obj.getObjectField(DBBackMediaGiftDef::VALUE_EXTS.c_str()));
	}

	return 0;
}

int BackMediaGift::update_acti_code(MongoDataMap* data_map, ActiCodeDetail* acti_code_detail)
{
	BSONObjBuilder builder;

	builder	<< DBBackActiCode::ID << acti_code_detail->__id
			<< DBBackActiCode::USER_ID << acti_code_detail->__user_id
			<< DBBackActiCode::AMOUNT << acti_code_detail->__amount
			<< DBBackActiCode::USED_TIME << acti_code_detail->__used_time;

	data_map->push_update(DBBackActiCode::COLLECTION, BSON(DBBackActiCode::ACTI_CODE
			<< acti_code_detail->__acti_code), builder.obj(), true);

	return 0;
}

int BackMediaGift::load_download_box_gift(int agent_code,std::vector<ItemObj>& item_list,string& url)
{
	item_list.clear();
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBBackDownLoadBoxGift::COLLECTION);
	while(cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		if(res.hasField(DBBackDownLoadBoxGift::AGENT_CODE.c_str()))
		{
			if(res.hasField(DBBackDownLoadBoxGift::DOWNLOAD_URL.c_str()))
			{
				url = res[DBBackDownLoadBoxGift::DOWNLOAD_URL].String();
			}
			int agent = res[DBBackDownLoadBoxGift::AGENT_CODE].numberInt();
			if(agent == agent_code)
			{
				BSONObjIterator iter(res.getObjectField(DBBackDownLoadBoxGift::ITEM_LIST.c_str()));
				while (iter.more())
				{
				 BSONObj it_obj = iter.next().embeddedObject();
				 ItemObj item;
				 item.id_ = it_obj[DBBackDownLoadBoxGift::ItemObj::ITEM_ID].numberInt();
				 item.amount_ = it_obj[DBBackDownLoadBoxGift::ItemObj::ITEM_AMOUNT].numberInt();
				 item.bind_ = it_obj[DBBackDownLoadBoxGift::ItemObj::ITEM_BIND].numberInt();
				 item_list.push_back(item);
			    }
				return 0;
			}
		}
		else
		{
			continue;
		}
	}
	return -1;
}

void BackMediaGift::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBBackActiCode::COLLECTION, BSON(DBBackActiCode::ACTI_CODE << 1), true);
	this->conection().ensureIndex(DBBackActiCode::COLLECTION, BSON(DBBackActiCode::ID << 1));
	this->conection().ensureIndex(DBBackActiCode::BACKUP_COLLECTION, BSON(DBBackActiCode::ID << 1));


	this->conection().ensureIndex(DBBackMediaGiftDef::COLLECTION, BSON(DBBackMediaGiftDef::GIFT_SORT << 1), true);
	this->conection().ensureIndex(DBBackMediaGiftDef::COLLECTION, BSON(DBBackMediaGiftDef::UPDATE_STATUS << 1));
	this->conection().ensureIndex(DBBackDownLoadBoxGift::COLLECTION, BSON(DBBackDownLoadBoxGift::AGENT_CODE << 1));
END_CATCH
}
