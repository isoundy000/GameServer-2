/*
 * MMOGlobal.h
 *
 * Created on: 2013-03-21 17:25
 *     Author: lyz
 */

#ifndef _MMOGLOBAL_H_
#define _MMOGLOBAL_H_

#include "MongoTable.h"

class MMOGlobal : public MongoTable
{
public:
    virtual ~MMOGlobal(void);

    int load_global_key(HashMap<std::string, int64_t, NULL_MUTEX> *global_key_map);
    int64_t update_global_key(const std::string &key);
    static int update_global_key(const std::string &key, const int64_t value, MongoDataMap *data_map);

    int64_t load_global_voice_id(void);
    int update_global_voice_id(int64_t id);

    int load_global_script_progress(HashMap<int, Int64, NULL_MUTEX> *progress_map, const std::vector<int> &script_set);
    int update_global_script_progress(const int script_sort, const Int64 progress);

    static int load_global_key_value(const string& key, int& value);
    static int save_global_key_to_mongo_unit(const std::string &key, const int value);

    static int load_combine_first_value(int& value);
    static int save_comnbine_first_value(int value);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOGLOBAL_H_
