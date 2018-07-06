/*
 * ConfigStruct.h
 *
 * Created on: 2013-04-18 15:52
 *     Author: lyz
 */

#ifndef _CONFIGSTRUCT_H_
#define _CONFIGSTRUCT_H_

#include "GameHeader.h"
#include "HashMap.h"
#include "DoubleKeyMap.h"


extern const unsigned char *skipBOM(const unsigned char *str, int *size);

struct ServerDetail
{
	int __index;
    int __server_type;
    int __is_travel;

    string __service_name;
    string __service_machine;
    BIntSet __scene_list;

    string __address;
    string __domain;
    int __inner_port;
    int __outer_port;

    BIntMap __scene_convert_to_map;

    ServerDetail(void);
    void reset();

    bool need_connect() const;
    bool need_reconnect_travel();
    bool need_reconnect_channel();
};

struct BlockConfigDetail
{
    int __block_width;
    int __block_height;
};

struct MapBlockDetail
{
    int __max_step;
    double __monster_step_tick;
    BlockConfigDetail __base_block_config;
    IntVec __flush_tick_vc;

    typedef HashMap<int, BlockConfigDetail, NULL_MUTEX> BlockConfigMap;
    BlockConfigMap __scene_block_config_map;

    int __script_stop_times;
    int __script_monster_max;

    void reset(void);
};

struct MptDetail
{
    int __cell_width;
    int __cell_height;
    int __x_len;
    int __y_len;
    int __map_width;
    int __map_height;
    int __i_point_y;

    int __block_width;
    int __block_height;

    MptDetail(void);
    void reset(void);
};

struct MergeItem
{
    int __npc_shop_type;
    std::string __item_id_str;
};
typedef std::vector<MergeItem> MergeItemVec;
typedef std::map<int, MergeItemVec> MergeItemVecMap;

#endif //_CONFIGSTRUCT_H_
