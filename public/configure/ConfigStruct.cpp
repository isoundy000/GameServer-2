/*
 * ConfigStruct.cpp
 *
 * Created on: 2013-04-18 15:55
 *     Author: lyz
 */

#include "ConfigStruct.h"
#include "GameConfig.h"
#include <cstring>

const unsigned char *skipBOM(const unsigned char *str, int *size)
{
    const char *p = "\xEF\xBB\xBF"; /* Utf8 BOM mark */
    const unsigned char *c = (const unsigned char *)str;
    int i = 0;
    do {
        if (i > *size)
            break;
        if (*c != *(const unsigned char *)p++)
        {
            *size -= i;
            return c;
        }
        ++i;
        ++c;
    } while (*p != '\0');
    *size -= i;
    return c;
}

ServerDetail::ServerDetail(void)
{
	ServerDetail::reset();
}

void ServerDetail::reset()
{
	this->__index = 0;
    this->__server_type = 0;
    this->__service_name.clear();
    this->__service_machine.clear();
    this->__scene_list.clear();
    this->__address.clear();
    this->__domain.clear();
    this->__inner_port = 0;
    this->__outer_port = 0;
    this->__is_travel = false;
    this->__scene_convert_to_map.clear();
}

bool ServerDetail::need_connect() const
{
	if (this->__inner_port <= 0)
	{
		return false;
	}

	if (this->__address.empty() == true)
	{
		return false;
	}

	return true;
}

bool ServerDetail::need_reconnect_travel()
{
	string new_address = CONFIG_INSTANCE->machine_address(this->__service_machine);
	int new_port = CONFIG_INSTANCE->travel_server_port(this->__service_machine);
	JUDGE_RETURN(new_address != this->__address || new_port != this->__inner_port, false);

	this->__address = new_address;
	this->__inner_port = new_port;
	return true;
}

bool ServerDetail::need_reconnect_channel()
{
	string new_address = CONFIG_INSTANCE->machine_address(this->__service_machine,
			NULL, this->__is_travel);
	int new_port = CONFIG_INSTANCE->travel_server_port_channel(this->__service_machine);
	JUDGE_RETURN(new_address != this->__address || new_port != this->__inner_port, false);

	this->__address = new_address;
	this->__inner_port = new_port;
	return true;
}

void MapBlockDetail::reset(void)
{
    this->__max_step = 0;
    this->__monster_step_tick = 0;
    this->__base_block_config.__block_width = 0;
    this->__base_block_config.__block_height = 0;
    this->__flush_tick_vc.clear();

    this->__scene_block_config_map.unbind_all();

    this->__script_stop_times = 0;
    this->__script_monster_max = 0;
}

MptDetail::MptDetail(void) :
    __cell_width(0), __cell_height(0), __x_len(0), __y_len(0),
    __map_width(0), __map_height(0), __i_point_y(0),
    __block_width(0), __block_height(0)
{ /*NULL*/ }

void MptDetail::reset(void)
{
    ::memset(this, 0, sizeof(MptDetail));
}

