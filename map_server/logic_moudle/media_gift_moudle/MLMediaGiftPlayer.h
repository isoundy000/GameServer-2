/*
 * MLMediaGiftPlayer.h
 *
 *  Created on: Aug 9, 2014
 *      Author: root
 */

#include "MLPacker.h"

#ifndef MLMEDIAGIFTPLAYER_H_
#define MLMEDIAGIFTPLAYER_H_

class ProtoMediaGiftDef;
class MediaGiftDef;
class ActiCodeDetail;

class MLMediaGiftPlayer : virtual public MLPacker
{
public:
	MLMediaGiftPlayer();
	virtual ~MLMediaGiftPlayer();

	PlayerMediaGiftDetail& media_gift_detail(void);
	void reset(void);

	int use_acti_code_begin(Message *msg);
	int use_acti_code_after(Transaction* trans);
	int fetch_media_gift_config(void);

	int sync_transfer_media_gift(int scene_id);
	int read_transfer_media_gift(Message *msg);

	int begin_query_center_acti_code(Int64 code_id, const char* acti_code);
	int after_query_center_acti_code(Message *msg);

	int fetch_download_box_info(); //49洛神域游戏盒子

private:
	void make_up_gift_config_info(Message *msg, const MediaGiftDef& gift_def);
	int validate_acti_code(const char* acti_code);
	int64_t acti_code_str_to_num(const char* acti_code);
	int character_str_to_num(const char* str);
	int valida_media_gift(ActiCodeDetail* acti_code_detail);
	int fetch_media_gift(ActiCodeDetail* acti_code_detail);
	int route_media_gift(ActiCodeDetail* acti_code_detail);

	PlayerMediaGiftDetail media_gift_detail_;
};

#endif /* MLMEDIAGIFTPLAYER_H_ */
