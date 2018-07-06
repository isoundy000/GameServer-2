/*
 * GamePackage.h
 *
 * Created on: 2013-05-02 10:50
 *     Author: lyz
 */

#ifndef _GAMEPACKAGE_H_
#define _GAMEPACKAGE_H_

#include "PubStruct.h"

struct PackOpenInfo
{
	PackOpenInfo();

	int online_tick_;
	int left_second_;

	int add_exp_;
	int total_second_;
};

struct PackGridInfo
{
	PackGridInfo();

	int strengthen_lvl_;		//强化等级
	int strengthen_bless_;		//强化祝福
};

typedef std::vector<PackGridInfo> PackGridVec;

struct ResultInfo
{
	int error_no_;
	int set_error_no(int error_no);
};

struct PackInsertResult : public ResultInfo
{
	PackInsertResult(void);

	void reset(void);
	int first_index() const;

	ItemObjMap obj_map_;	//key: index
};

struct PackRemoveResult : public ResultInfo
{
	PackRemoveResult(void);
	void reset(void);

	int item_id_;
	int bind_count_;
	int unbind_count_;

	IntMap remove_map_;
};

class GamePackage
{
public:
    GamePackage(void);
    void reset(void);

    /*
     * 基本信息
     * */
    void set_owner(MLPacker* owner);
    void set_type(int type, int size);
    void set_sublime_info(int is_open_sublime, int sublime_level);
    void set_insert_result(const PackInsertResult& insert_result);

    void init_equip_pack();
    void caculate_equip_force(int index);
    void notify_item_info(int index);

    int type();
    int size();
    int sublime_level();
    int is_open_sublime();
    int strengthen_level(int index);
    PackGridInfo* grid_info(int index);

    int left_capacity(void);
    int pack_max_size(void);

    int pack_base_index(void);
    int validate_pack_index(int index);

    PackGridVec &grid_vec(void);
    ItemListMap &item_map(void);

    /*
     * 背包插入
     * */
    int insert_with_res(const ItemObj& obj);
    int insert_with_res(const ItemObjVec& item_obj_vec);
    int insert_with_res(int pack_index, PackageItem* pack_item);
    int insert_with_res(int pack_index, const ItemObj& item_obj);

    int insert_by_index(PackageItem* pack_item);
    int insert_by_index(int pack_index, PackageItem* pack_item);

    /*
     * 背包删除
     * */
    int remove_item(int pack_index, int item_amount);
    int remove_item(PackageItem* pack_item, int item_amount);

    int remove_all_by_id(int item_id);
    int remove_by_id(int item_id, int item_amount);

    int remove_item_index(PackageItem* pack_item);

    /*
     * 背包事务操作
     * */
    int pack_transaction(const ItemObj& remove_item, const ItemObj& add_item);

    /*
     * 背包查找
     * */
    PackageItem* find_by_index(int pack_index);
    PackageItem* find_by_id(int item_id);
    PackageItem* find_by_bind_id(int item_id);
    PackageItem* find_by_unbind_id(int item_id);
    PackageItem* find_by_unique_id(Int64 item_unique_id);

    /*
     * 背包查找
     * */
    int find_all(IntVec& item_set, int item_id);
    int find_all(ItemVector& item_set, int item_id);
    int find_all(ItemListMap& item_map, int item_id);
    int find_all_without_lock(ItemVector& item_set, int item_id);

    /*
     * 计算个数
     * */
    int count_by_id(int item_id);
    int count_by_bind_id(int item_id);
    int count_by_unbind_id(int item_id);

    /*
     * 获取操作情况
     * */
    const PackInsertResult& get_insert_result(void);
    const PackRemoveResult& get_remove_result(void);

    /*
     * 开启格子
     * */
    bool finish_open_grid(void);
    bool validate_open_index(int pack_index);

    void open_pack_grid(int pack_index, Int64 online_tick);
    int fetch_open_grid_info(PackOpenInfo& open_info, int pack_index,double rate = 0);
    int fetch_open_grid_money(int left_second);

    /*
     * 锁定格子
     * */
    int pack_lock(int pack_index, int lock_type);
    int pack_unlock(int pack_index, int lock_type);

    int check_and_lock(int lock_type);
    int pack_unlock_all(int lock_type);

    int is_index_lock(int pack_index);
    int check_lock_type(int lock_type);

    /*
     * 系列化
     * */
    void serialize(ProtoPackage* proto_package);
    void unserialize(const ProtoPackage& proto_package);

    void serialize(ProtoItem* proto_item, int pack_index);
    void serialize(ProtoItem* proto_item, PackageItem* item);

    /*
     * 其他
     * */
    int sort_and_merge(int remove = 0);
    int fetch_require_space(int item_id, int item_num, int item_bind);

    /*
     * only search first one empty index, return ERROR_NO_ENOUGH_SPACE if fail
     * */
    int search_empty_index_i(void);

    /*
     * 背包类型
     */
    bool is_equip(void);
    bool is_package(void);
    bool is_box_store(void);

public:
    static PackageItem *pop_item(int item_id = 0, int item_amount = 0);
    static int push_item(PackageItem* pack_item);

    static int init_item_info(PackageItem* pack_item);
    static int init_prop_info(PackageItem* pack_item);
    static int fetch_max_pack_size(int pack_type, int pack_size);

    static bool pack_check(int pack_type);
    static bool merge_and_push(PackageItem* cur_item, PackageItem* prev_item);
    static bool pack_sort_cmp(const PackageItem* left, const PackageItem* right);

    static int pack_type_has_force(int pack_type);
    static int item_has_force(PackageItem* pack_item);

private:
    int check_insert(const ItemObj& obj);
    int fetch_require_space(const ItemObj& obj, int max_amount);
    int insert_old_space(const ItemObj& obj, int max_amount);
    int insert_new_space(const ItemObj& obj, int max_amount);

    void insert_operate();

    int remove_item_i(PackageItem* pack_item);
    int remove_item_i(PackageItem* pack_item, int item_amount);

    int fetch_pack_index(int index);
    int check_pack_is_order(ItemVector& item_set);

    void make_up_item_force(ProtoItem* proto_item, PackageItem* pack_item);

public:
	int pack_size_;
	int pack_type_;
	int sublime_level_;
	int is_open_sublime_;

	Int64 last_open_tick_;

	PackGridVec grid_vec_;
	ItemListMap item_list_map_;

private:
	MLPacker* owner_;

	int lock_type_;
	IntMap lock_map_;

	PackInsertResult insert_res_;
	PackRemoveResult remove_result_;
};

#endif //_GAMEPACKAGE_H_
