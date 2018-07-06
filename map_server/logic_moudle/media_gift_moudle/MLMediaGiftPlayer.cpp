/*
 * MLMediaGiftPlayer.cpp
 *
 *  Created on: Aug 9, 2014
 *      Author: root
 */

#include "MLMediaGiftPlayer.h"
#include "ProtoDefine.h"
#include "Transaction.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "MediaGiftConfig.h"
#include "BackMediaGift.h"
#include "GameFont.h"
#include "MongoDataMap.h"
#include "MLGameSwither.h"

#define ACTI_CODE_SHIFT 		4
#define ACTI_CODE_MASK 			0xf
#define CHECK_SUM_LENGTH		4

#define GIFT_TIME_LIMIT_MASK 	0x1
#define GIFT_LEVEL_LIMIT_MASK	0x2
#define GIFT_SPAN_LIMIT_MASK	0x4

MLMediaGiftPlayer::MLMediaGiftPlayer() {
	// TODO Auto-generated constructor stub
}

MLMediaGiftPlayer::~MLMediaGiftPlayer() {
	// TODO Auto-generated destructor stub
}

void MLMediaGiftPlayer::reset(void)
{
	ActiCodeDetailMap::iterator iter = this->media_gift_detail_.__acti_code_map.begin();
	for( ; iter != this->media_gift_detail_.__acti_code_map.end(); ++iter )
	{
		JUDGE_CONTINUE(iter->second != 0);
		this->monitor()->acti_code_detail_pool()->push(iter->second);
	}

	this->media_gift_detail_.reset();
}

PlayerMediaGiftDetail& MLMediaGiftPlayer::media_gift_detail(void)
{
	return this->media_gift_detail_;
}

int MLMediaGiftPlayer::use_acti_code_begin(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11401431*, request, RETURN_USE_ACTI_CODE);

	CONDITION_NOTIFY_RETURN(GameCommon::validate_time_span(
			this->media_gift_detail_.__last_query_tick, Time_Value::SECOND),
			RETURN_USE_ACTI_CODE, ERROR_OPERATE_TOO_FAST);

	this->media_gift_detail_.__last_query_tick = ::time(0);

	char acti_code[MAX_ACTI_CODE_LENGTH + 1] = {0};
	::strncpy(acti_code, request->acti_code().c_str(), MAX_ACTI_CODE_LENGTH);

	GameCommon::string_to_uper_case(acti_code);

	//校验激活码
	int ret = this->validate_acti_code(acti_code);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_USE_ACTI_CODE, ret);

	ActiCodeDetail* acti_code_detail = this->monitor()->acti_code_detail_pool()->pop();
	CONDITION_NOTIFY_RETURN(0 != acti_code_detail, RETURN_USE_ACTI_CODE, ERROR_SERVER_INNER);

	Int64 code_num = this->acti_code_str_to_num(acti_code);
	acti_code_detail->__id = code_num;

	::strncpy(acti_code_detail->__acti_code, acti_code, MAX_ACTI_CODE_LENGTH);
	acti_code_detail->__acti_code[MAX_ACTI_CODE_LENGTH] = '\0';

	acti_code_detail->__use_only_vip_client = request->super_vip_use();

	 if (0 != TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			 TRANS_FETCH_ACTI_CODE_DETAIL, DB_BACK_ACTI_CODE, acti_code_detail,
			 this->monitor()->acti_code_detail_pool(), this->monitor()->logic_unit()))
	{
		this->monitor()->acti_code_detail_pool()->push(acti_code_detail);
		this->respond_to_client_error(RETURN_USE_ACTI_CODE, ERROR_SERVER_INNER);
		return -1;
	}

	return 0;
}

int MLMediaGiftPlayer::use_acti_code_after(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return this->respond_to_client_error(RETURN_USE_ACTI_CODE,ERROR_ACTI_CODE_UNVALID);
	}

	TransactionData* trans_data = trans->fetch_data(DB_BACK_ACTI_CODE);
	if(trans_data == 0)
	{
		trans->rollback();
		return this->respond_to_client_error(RETURN_USE_ACTI_CODE, ERROR_ACTI_CODE_UNVALID);
	}

	ActiCodeDetail* acti_code_detail = trans_data->__data.__acti_code_detail;
	trans_data->__data.__acti_code_detail = NULL;
	trans->summit();

	int only_super_vip_client = acti_code_detail->__use_only_vip_client;
	int only_super_vip_config = acti_code_detail->__use_only_vip;
	CONDITION_NOTIFY_RETURN(only_super_vip_config == only_super_vip_client,
			RETURN_USE_ACTI_CODE,
			ERROR_SERVER_INNER);

	CONDITION_NOTIFY_RETURN(0 != acti_code_detail, RETURN_USE_ACTI_CODE,
			ERROR_SERVER_INNER);

	int ret = 0;
	if ((ret = this->valida_media_gift(acti_code_detail)) != 0)
	{
		this->monitor()->acti_code_detail_pool()->push(acti_code_detail);
		return this->respond_to_client_error(RETURN_USE_ACTI_CODE, ret);
	}

	Int64 code_id = acti_code_detail->__id;

	ActiCodeDetailMap::iterator iter = this->media_gift_detail_.__acti_code_map.find(code_id);
	if((iter != this->media_gift_detail_.__acti_code_map.end()) && (0 != iter->second))
		this->monitor()->acti_code_detail_pool()->push(iter->second);

	this->media_gift_detail_.__acti_code_map[code_id] = acti_code_detail;

	if((ret = this->route_media_gift(acti_code_detail)) != 0)
	{
		this->media_gift_detail_.__acti_code_map.erase(code_id);
		this->monitor()->acti_code_detail_pool()->push(acti_code_detail);
		return this->respond_to_client_error(RETURN_USE_ACTI_CODE, ret);
	}

	return 0;
}

int MLMediaGiftPlayer::fetch_media_gift_config(void)
{
	Proto51401432 respond;
	const MediaGiftDefMap& gift_map = MEDIA_GIFT_CONFIG->media_gift_def_map();

	for(MediaGiftDefMap::const_iterator iter = gift_map.begin(); iter != gift_map.end(); ++iter)
	{
		this->make_up_gift_config_info(&respond, iter->second);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_MEDIA_GIFT_DEF, &respond);
}

int MLMediaGiftPlayer::sync_transfer_media_gift(int scene_id)
{
	Proto31400127 request;
	for(IntMap::iterator iter = this->media_gift_detail().__gift_use_times.begin();
			iter != this->media_gift_detail().__gift_use_times.end(); ++iter)
	{
		ProtoPairObj *obj = request.add_gift_use_times();
		obj->set_obj_id(iter->first);
		obj->set_obj_value(iter->second);
	}

	for(IntMap::iterator iter = this->media_gift_detail().__gift_use_tags.begin();
			iter != this->media_gift_detail().__gift_use_tags.end(); ++iter)
	{
		ProtoPairObj *obj = request.add_gift_use_tags();
		obj->set_obj_id(iter->first);
		obj->set_obj_value(iter->second);
	}

	for(IntMap::iterator iter = this->media_gift_detail().__gift_use_tick.begin();
			iter != this->media_gift_detail().__gift_use_tick.end(); ++iter)
	{
		ProtoPairObj *obj = request.add_gift_use_tick();
		obj->set_obj_id(iter->first);
		obj->set_obj_value(iter->second);
	}

	return this->send_to_other_logic_thread(scene_id, request);
}

int MLMediaGiftPlayer::read_transfer_media_gift(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400127*, request, -1);

	for(int i=0; i < request->gift_use_times_size(); ++i)
	{
		ProtoPairObj obj = request->gift_use_times(i);
		this->media_gift_detail().__gift_use_times[obj.obj_id()] = obj.obj_value();
	}

	for(int i=0; i < request->gift_use_tags_size(); ++i)
	{
		ProtoPairObj obj = request->gift_use_tags(i);
		this->media_gift_detail().__gift_use_tags[obj.obj_id()] = obj.obj_value();
	}

	for(int i=0; i < request->gift_use_tick_size(); ++i)
	{
		ProtoPairObj obj = request->gift_use_tick(i);
		this->media_gift_detail().__gift_use_tick[obj.obj_id()] = obj.obj_value();
	}

	return 0;
}

int MLMediaGiftPlayer::begin_query_center_acti_code(Int64 code_id, const char* acti_code)
{
	Proto30101101 request;
	request.set_code_id(code_id);
	request.set_acti_code(acti_code);
	return this->monitor()->dispatch_to_logic(this, &request);
}

int MLMediaGiftPlayer::after_query_center_acti_code(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31402002*, request, -1);

	int query_ret = request->query_ret();
	Int64 code_id = request->code_id();

	CONDITION_NOTIFY_RETURN(query_ret == 0, RETURN_USE_ACTI_CODE, ERROR_ACTI_CODE_UNVALID);

	ActiCodeDetailMap::iterator iter = this->media_gift_detail_.__acti_code_map.find(code_id);
	CONDITION_NOTIFY_RETURN(iter != this->media_gift_detail_.__acti_code_map.end(),
			RETURN_USE_ACTI_CODE, ERROR_SERVER_INNER);

	CONDITION_NOTIFY_RETURN(iter->second != 0, RETURN_USE_ACTI_CODE, ERROR_ACTI_CODE_UNVALID);
	ActiCodeDetail *acti_code_detail = iter->second;

	this->record_other_serial(MAIN_MEDIA_GIFT, SUB_USE_MEDIA_GIFT, acti_code_detail->__gift_sort, code_id);

	int ret = 0;
	if ((ret = this->fetch_media_gift(acti_code_detail)) != 0)
	{
		this->respond_to_client_error(RETURN_USE_ACTI_CODE, ret);
	}
	else
	{
		Proto51401431 respond;
		respond.set_gift_sort(acti_code_detail->__gift_sort);

		MediaGiftDef *gift_config = 0;
		MEDIA_GIFT_CONFIG->fetch_gift_config(respond.gift_sort(), gift_config);
		if (0 != gift_config)
		{
			respond.set_gift_name(gift_config->__gift_name);
		}

		this->respond_to_client(RETURN_USE_ACTI_CODE, &respond);

	    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
		BackMediaGift::update_acti_code(data_map, acti_code_detail);

	    if (TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
	    		TRANS_UPDATE_ACTI_CODE_DETAIL, data_map) != 0)
	    {
	        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
	    }
	}

	this->media_gift_detail_.__acti_code_map.erase(code_id);
	this->monitor()->acti_code_detail_pool()->push(acti_code_detail);
    return 0;
}

void MLMediaGiftPlayer::make_up_gift_config_info(Message *msg, const MediaGiftDef& gift_def)
{
	MSG_DYNAMIC_CAST_RETURN(Proto51401432*, info, ;);
	JUDGE_RETURN(gift_def.__show_icon != 0, ;);

	int used_times = this->media_gift_detail().__gift_use_times[gift_def.__gift_sort];
	JUDGE_RETURN((gift_def.__hide_used == 0) || (gift_def.__use_times - used_times > 0), ;);

	if(gift_def.__expire_time > 0)
	{
		JUDGE_RETURN(gift_def.__expire_time > ::time(0), ;);
	}

	ProtoMediaGiftDef* proto_gift_def = info->add_gift_def();
	JUDGE_RETURN(0 != proto_gift_def, ;);

	proto_gift_def->set_gift_sort(gift_def.__gift_sort);
	proto_gift_def->set_gift_type(gift_def.__gift_type);
	proto_gift_def->set_gift_name(gift_def.__gift_name);
	proto_gift_def->set_gift_desc(gift_def.__gift_desc);
	proto_gift_def->set_icon_id(gift_def.__show_icon);

	if(gift_def.__gift_type & GIFT_TIME_LIMIT_MASK)
	{//有次数限制的礼包
		proto_gift_def->set_use_times(gift_def.__use_times);
		proto_gift_def->set_left_use_times(MAX(0, gift_def.__use_times - used_times));
	}
	else
	{
		proto_gift_def->set_left_use_times(10);
		proto_gift_def->set_use_times(10);
	}

	for(ItemObjVec::const_iterator iter = gift_def.__gift_list.begin();
			iter != gift_def.__gift_list.end(); ++ iter)
	{
		ProtoItem* proto_item = proto_gift_def->add_gift_items();

		proto_item->set_id(iter->id_);
		proto_item->set_amount(iter->amount_);
		proto_item->set_bind(iter->bind_);
		proto_item->set_index(iter->index_);
	}

	for(IntMap::const_iterator iter = gift_def.__value_ext.begin();
			iter != gift_def.__value_ext.end(); ++iter)
	{
		ProtoPairObj* pair_obj = proto_gift_def->add_value_exts();
		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}

	for(IntVec::const_iterator iter = gift_def.__font_color.begin();
			iter != gift_def.__font_color.end(); ++iter)
	{
		proto_gift_def->add_font_color(*iter);
	}
}

int MLMediaGiftPlayer::validate_acti_code(const char* acti_code)
{
	JUDGE_RETURN(0 != acti_code, ERROR_SERVER_INNER);

	int code_len = ::strlen(acti_code);
	JUDGE_RETURN(code_len >= ACTI_CODE_LENGTH, ERROR_ACTI_CODE_UNVALID);
	JUDGE_RETURN(code_len <= MAX_ACTI_CODE_LENGTH, ERROR_ACTI_CODE_UNVALID);

//	int64_t code_num = acti_code_str_to_num(acti_code + CHECK_SUM_LENGTH);
//	JUDGE_RETURN(code_num >= 0, ERROR_ACTI_CODE_UNVALID);
//
//	char crc_str[8] = {0};
//	::sprintf(crc_str, acti_code, CHECK_SUM_LENGTH);
//	crc_str[CHECK_SUM_LENGTH]='\0';
//
//	uint16_t crc_check = 0xffff;
//	crc_check = GameCommon::crc16(crc_check, (uint8_t *)acti_code + CHECK_SUM_LENGTH,
//			strlen(acti_code) - CHECK_SUM_LENGTH);
//
//	uint16_t crc_value = 0xffff & this->character_str_to_num(crc_str);
//	MSG_USER([%s] = 0x%04x [%s] => 0x%04x, crc_str, crc_value,
//			acti_code + CHECK_SUM_LENGTH, crc_check);
//
//	JUDGE_RETURN(crc_check = crc_value, ERROR_ACTI_CODE_UNVALID);

	return 0;
}

int64_t MLMediaGiftPlayer::acti_code_str_to_num(const char* acti_code)
{
	JUDGE_RETURN(0 != acti_code, -1);
	const char* p_char = acti_code;

	int64_t add_num = 0;
	for( ; *p_char != '\0'; ++p_char )
	{
		int i_value = 0;
		if(*p_char >= '0' && *p_char <= '9')
			i_value = *p_char - '0';
		else if(*p_char >= 'A' && *p_char <= 'Z')
			i_value = *p_char - 'A' + 10;
		else
			return -1;

		add_num = add_num * 36 + i_value;
	}

	return add_num;
}

int MLMediaGiftPlayer::character_str_to_num(const char* str)
{
	JUDGE_RETURN(0 != str, -1);
	const char* p_char = str;

	int add_num = 0;
	for( ; *p_char != '\0'; ++p_char )
	{
		int i_value = 0;
		if(*p_char >= '0' && *p_char <= '9')
			i_value = *p_char - '0';
		else if(*p_char >= 'A' && *p_char <= 'Z')
			i_value = *p_char - 'A' + 10;
		else
			return -1;

		add_num <<= ACTI_CODE_SHIFT;
		add_num += (i_value & ACTI_CODE_MASK);
	}

	return add_num;
}

int MLMediaGiftPlayer::valida_media_gift(ActiCodeDetail* acti_code_detail)
{
	JUDGE_RETURN(acti_code_detail != 0, ERROR_ACTI_CODE_UNVALID);
	JUDGE_RETURN(acti_code_detail->__amount > 0, ERROR_ACTI_CODE_USED);

	Int64 cur_time = time(0);

	if (acti_code_detail->__start_time > 0)
	{
		JUDGE_RETURN(cur_time >= acti_code_detail->__start_time, ERROR_ACTI_CODE_UNVALID);
	}

	if (acti_code_detail->__end_time > 0)
	{
		JUDGE_RETURN(cur_time <= acti_code_detail->__end_time, ERROR_ACTI_CODE_UNVALID);
	}

	// 领取礼物
	MediaGiftDef *gift_config = 0;
	MEDIA_GIFT_CONFIG->fetch_gift_config(acti_code_detail->__gift_sort, gift_config);
	JUDGE_RETURN(0 != gift_config, ERROR_ACTI_CODE_UNVALID);

	if(gift_config->__expire_time > 0)
	{
		JUDGE_RETURN(cur_time <= gift_config->__expire_time, ERROR_ACTI_CODE_UNVALID);
	}

	if(gift_config->__gift_tag != 0)
	{
		int use_tag = this->media_gift_detail().__gift_use_tags[gift_config->__gift_tag];
		JUDGE_RETURN(use_tag <= 0, ERROR_MEDIA_GIFT_USE_TIMES);
	}

	int gift_times_limit = gift_config->__gift_type & GIFT_TIME_LIMIT_MASK;
	int gift_level_limit = gift_config->__gift_type & GIFT_LEVEL_LIMIT_MASK;
	int gift_span_limit = gift_config->__gift_type & GIFT_SPAN_LIMIT_MASK;

	if(gift_times_limit)
	{// 次数有限制的礼包
		int used_times = this->media_gift_detail().__gift_use_times[gift_config->__gift_sort];
		JUDGE_RETURN(used_times < gift_config->__use_times, ERROR_MEDIA_GIFT_USE_TIMES);
	}

	if(gift_level_limit)
	{// 等级有限制的礼包
		int low_level = gift_config->__value_ext[1];
		int top_level = gift_config->__value_ext[2];

		JUDGE_RETURN(this->role_level() >= low_level, ERROR_MEDIA_GIFT_LVL_LIMIT);
		JUDGE_RETURN(this->role_level() <= top_level, ERROR_MEDIA_GIFT_LVL_LIMIT);
	}

	if(gift_span_limit)
	{// 要隔一段时间才能领取的礼包
		int time_span = gift_config->__value_ext[3];
		int used_tick = this->media_gift_detail().__gift_use_tick[gift_config->__gift_sort];
		bool valid_span = GameCommon::validate_time_span(used_tick, time_span);
		JUDGE_RETURN(true == valid_span, ERROR_MEDIA_GIFT_TIME_SPAN);
	}

	return 0;
}

int MLMediaGiftPlayer::fetch_media_gift(ActiCodeDetail* acti_code_detail)
{
	JUDGE_RETURN(acti_code_detail != 0, ERROR_ACTI_CODE_UNVALID);

	MediaGiftDef *gift_config = 0;
	MEDIA_GIFT_CONFIG->fetch_gift_config(acti_code_detail->__gift_sort, gift_config);
	JUDGE_RETURN(gift_config != NULL, ERROR_ACTI_CODE_UNVALID);

//	int ret = this->insert_package(ADD_FROM_GIFT_DRAW, gift_config->__gift_list);
//	if(ret != 0)// 无法插入背包,使用邮件发送
//	{
//		FontPair mail_font = FONT2(FONT_MEDIA_GIFT_AWARDS);
//		char font_str[MAX_COMMON_NAME_LENGTH + 1] = {0};
//		::snprintf(font_str, MAX_COMMON_NAME_LENGTH, mail_font.second.c_str(), gift_config->__gift_name);
//		mail_font.second = font_str;
//
//		MailInformation* gift_mail = GameCommon::create_sys_mail(mail_font,FONT_MEDIA_GIFT_AWARDS);
//		gift_mail->receiver_id_ = this->role_id();
//
//		for(ItemObjVec::iterator iter = gift_config->__gift_list.begin();
//				iter != gift_config->__gift_list.end(); ++iter)
//		{
//			GameCommon::insert_mail_item(gift_mail->goods_map_, *iter);
//		}
//		GameCommon::request_save_mail(gift_mail);
//		this->respond_to_client_error(RETURN_USE_ACTI_CODE, ERROR_MEDIA_GIFT_PACKAGE);
//	}

	if (acti_code_detail->__use_only_vip > 0)
	{
		JUDGE_RETURN(this->vip_detail().__super_get_type == false, ERROR_CLIENT_OPERATE);

		int mail_id = CONFIG_INSTANCE->const_set("super_vip_mail");
		JUDGE_RETURN(mail_id > 0, ERROR_CONFIG_ERROR);

		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		mail_info->mail_content_ = this->vip_detail().__des_mail;

		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str());

		for (ItemObjVec::iterator iter = gift_config->__gift_list.begin();
				iter != gift_config->__gift_list.end(); ++iter)
		{
			ItemObj& obj = *iter;
			mail_info->add_goods(obj.id_, obj.amount_, obj.bind_);
		}

		GameCommon::request_save_mail_content(this->role_id(), mail_info);

		this->vip_detail().__super_get_type = true;
		acti_code_detail->__amount -= 1;

		MapLogicPlayer* player = this->map_logic_player();
		player->fetch_super_vip_info_begin();
	}
	else
	{
		this->insert_package(ADD_FROM_GIFT_DRAW, gift_config->__gift_list);

		acti_code_detail->__user_id = this->role_id();
		acti_code_detail->__used_time = ::time(0);
		acti_code_detail->__amount -= 1;

		this->media_gift_detail().__gift_use_tags[gift_config->__gift_tag] += 1;
		this->media_gift_detail().__gift_use_times[gift_config->__gift_sort] += 1;
		this->media_gift_detail().__gift_use_tick[gift_config->__gift_sort] = ::time(0);
	}

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_MEDIA_GIFT);
	return 0;
}

int MLMediaGiftPlayer::route_media_gift(ActiCodeDetail* acti_code_detail)
{
	JUDGE_RETURN(acti_code_detail != 0, ERROR_ACTI_CODE_UNVALID);

	MediaGiftDef *gift_config = 0;
	MEDIA_GIFT_CONFIG->fetch_gift_config(acti_code_detail->__gift_sort, gift_config);
	JUDGE_RETURN(0 != gift_config, ERROR_ACTI_CODE_UNVALID);

	Int64 code_id = acti_code_detail->__id;
	if (0 == gift_config->__is_share) // 单服的激活码不需要查询
	{
		Proto31402002 request;
		request.set_code_id(code_id);
		request.set_query_ret(0);
		return this->after_query_center_acti_code(&request);
	}
	else
	{
		return this->begin_query_center_acti_code(code_id, acti_code_detail->__acti_code);
	}
}

int MLMediaGiftPlayer::fetch_download_box_info()
{
//	CONDITION_NOTIFY_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::download_box_gift),
//			RETURN_DOWNLOAD_BOX_GIFT_INFO, ERROR_DOWNLOAD_BOX_GIFT_NOT_OPEN);
//	MapLogicPlayer *map_logic_player = this->map_logic_player();
//
//	int agent_code = map_logic_player->agent_code();
//	MSG_USER("local agent:%d",agent_code);
//
//	std::vector<ItemObj> item_list;
//	string download_url;
//	BackMediaGift::load_download_box_gift(agent_code,item_list,download_url);
//
//	Proto51401462 respond;
//	respond.set_download_url(download_url);
//	if(item_list.empty() == false)
//	{
//		std::vector<ItemObj>::iterator iter = item_list.begin();
//		for(;iter != item_list.end();++iter)
//		{
//			JUDGE_CONTINUE(&(*iter) != NULL);
//			ProtoItem *proto_item = respond.add_item_list();
//			proto_item->set_id(iter->item_id_);
//			proto_item->set_amount(iter->item_amount_);
//			proto_item->set_bind(iter->item_bind_);
//		}
//	}
//
//	FINER_PROCESS_RETURN(RETURN_DOWNLOAD_BOX_GIFT_INFO, &respond);
	return 0;
}
