/*
 * MediaGiftConfig.h
 *
 *  Created on: Aug 11, 2014
 *      Author: root
 */

#ifndef MEDIAGIFTCONFIG_H_
#define MEDIAGIFTCONFIG_H_

#include "Singleton.h"
#include "MapLogicStruct.h"

class MediaGiftConfig
{
public:
	class UpdateConfigTimer: public GameTimer
	{
	public:
		UpdateConfigTimer(void);
		~UpdateConfigTimer(void);

		int type(void);
		int handle_timeout(const Time_Value &tv);

		void set_parent(MediaGiftConfig *parent);
		MediaGiftConfig *parent_;
	};

	MediaGiftConfig();
	virtual ~MediaGiftConfig();

	void media_gift_start(void);
	void media_gift_end(void);

	void update_gift_config(void);
	int fetch_gift_config(int gift_sort, MediaGiftDef* &gift_conf);

	const MediaGiftDefMap& media_gift_def_map(void);

private:
	int last_update_tick_;
	UpdateConfigTimer update_timer_;
	MediaGiftDefMap gift_map_;
};

typedef Singleton<MediaGiftConfig> MediaGiftConfigSingle;
#define MEDIA_GIFT_CONFIG MediaGiftConfigSingle::instance()

#endif /* MEDIAGIFTCONFIG_H_ */
