/*
 * GamePackage.cpp
 *
 * Created on: 2013-05-02 14:11
 *     Author: lyz
 */

#include "GamePackage.h"
#include "PoolMonitor.h"
#include "MLPacker.h"
#include "ProtoDefine.h"

PackOpenInfo::PackOpenInfo()
{
	this->add_exp_ = 0;
	this->left_second_ = 0;

	this->online_tick_ = 0;
	this->total_second_ = 0;
}

PackGridInfo::PackGridInfo()
{
	this->strengthen_lvl_ = 0;
	this->strengthen_bless_ = 0;
}

int ResultInfo::set_error_no(int error_no)
{
	this->error_no_ = error_no;
	return this->error_no_;
}

PackInsertResult::PackInsertResult(void)
{ /*NULL*/ }

void PackInsertResult::reset(void)
{
	this->error_no_ = 0;
	this->obj_map_.clear();
}

int PackInsertResult::first_index() const
{
	JUDGE_RETURN(this->obj_map_.empty() == false, -1);

	ItemObjMap::const_iterator iter = this->obj_map_.begin();
	return iter->first;
}

PackRemoveResult::PackRemoveResult(void)
{
	PackRemoveResult::reset();
}

void PackRemoveResult::reset(void)
{
	this->item_id_ = 0;
	this->bind_count_ = 0;
	this->unbind_count_ = 0;

	this->error_no_ = 0;
	this->remove_map_.clear();
}

GamePackage::GamePackage(void)
{
	GamePackage::reset();
}

void GamePackage::reset(void)
{
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		GamePackage::push_item(iter->second);
	}

	this->pack_type_ = 0;
	this->pack_size_ = 0;
	this->sublime_level_ = 0;
	this->is_open_sublime_ = 0;
	this->last_open_tick_ = 0;

	this->grid_vec_.clear();
	this->item_list_map_.clear();

	this->lock_type_ = 0;
	this->lock_map_.clear();

	this->insert_res_.reset();
	this->remove_result_.reset();

	this->owner_ = NULL;
}

void GamePackage::set_owner(MLPacker* owner)
{
	this->owner_ = owner;
}

void GamePackage::set_type(int type, int size)
{
	this->pack_type_ = type;
	this->pack_size_ = GamePackage::fetch_max_pack_size(type, size);

	this->init_equip_pack();
}

void GamePackage::set_sublime_info(int is_open_sublime, int sublime_level)
{
	JUDGE_RETURN(this->is_equip() == true, ;);
	this->is_open_sublime_ = is_open_sublime;
	this->sublime_level_ = sublime_level;
}

void GamePackage::set_insert_result(const PackInsertResult& insert_result)
{
	this->insert_res_ = insert_result;
}

void GamePackage::init_equip_pack()
{
	JUDGE_RETURN(this->is_equip() == true, ;);
	this->grid_vec_.resize(this->size());
}

void GamePackage::caculate_equip_force(int index)
{
	JUDGE_RETURN(GamePackage::pack_type_has_force(this->type()) == true, ;);

	PackageItem* pack_item = this->find_by_index(index);
	JUDGE_RETURN(pack_item != NULL, ;);

	int role_level = this->owner_->role_level();
	int strengthen_level = this->strengthen_level(index);

	pack_item->caculate_fight_prop(role_level, strengthen_level);
}

void GamePackage::notify_item_info(int index)
{
	PackageItem* pack_item = this->find_by_index(index);
	JUDGE_RETURN(pack_item != NULL, ;);

	Proto81400101 respond;
	respond.set_pack_type(this->type());

	ProtoItem* proto_item = respond.add_pack_item_list();
	this->serialize(proto_item, pack_item);

	this->owner_->respond_to_client(ACTIVE_ADD_GOODS, &respond);
}

int GamePackage::type()
{
	return this->pack_type_;
}

int GamePackage::size()
{
	return this->pack_size_;
}

int GamePackage::sublime_level()
{
	return this->sublime_level_;
}

int GamePackage::is_open_sublime()
{
	return this->is_open_sublime_;
}

int GamePackage::strengthen_level(int index)
{
	JUDGE_RETURN(this->validate_pack_index(index) == true, 0);
	return this->grid_vec_[index].strengthen_lvl_;
}

PackGridInfo* GamePackage::grid_info(int index)
{
	JUDGE_RETURN(this->validate_pack_index(index) == true, NULL);
	return &this->grid_vec_[index];
}

int GamePackage::left_capacity(void)
{
	int cur_use_size = this->item_list_map_.size() + this->lock_map_.size();
    return this->size() - cur_use_size;
}

int GamePackage::pack_max_size(void)
{
//	if(this->is_package()) return CONFIG_INSTANCE->tiny("max_pack_grid").asInt();
//	if(this->is_store()) return CONFIG_INSTANCE->tiny("max_store_grid").asInt();
	return 0;
}

int GamePackage::pack_base_index(void)
{
	return 0;
}

int GamePackage::validate_pack_index(int index)
{
	return index >= 0 && index < this->pack_size_;
}

PackGridVec &GamePackage::grid_vec(void)
{
	return this->grid_vec_;
}

ItemListMap &GamePackage::item_map(void)
{
    return this->item_list_map_;
}

int GamePackage::insert_with_res(const ItemObj& obj)
{
	ItemObjVec obj_vec;
	obj_vec.push_back(obj);
	return this->insert_with_res(obj_vec);
}

int GamePackage::insert_with_res(const ItemObjVec& item_obj_vec)
{
	this->insert_res_.reset();

	for (ItemObjVec::const_iterator iter = item_obj_vec.begin();
			iter != item_obj_vec.end(); ++iter)
	{
		int ret = this->check_insert(*iter);
		JUDGE_CONTINUE(ret != 0);

		return this->insert_res_.set_error_no(ret);
	}

	this->insert_operate();
	return this->insert_res_.set_error_no(0);
}

int GamePackage::insert_with_res(int pack_index, PackageItem* pack_item)
{
	this->insert_res_.reset();

	if (pack_item == NULL)
	{
		return this->insert_res_.set_error_no(ERROR_INVALID_PARAM);
	}

	if (this->validate_pack_index(pack_index) == false)
	{
		return this->insert_res_.set_error_no(ERROR_INVALID_PARAM);
	}

	if (this->item_list_map_.count(pack_index) > 0)
	{
		return this->insert_res_.set_error_no(ERROR_INVALID_PARAM);
	}

	ItemObj obj;
	obj.unserialize(pack_item);

	this->insert_res_.obj_map_[pack_index] = obj;
	this->insert_by_index(pack_index, pack_item);
	this->caculate_equip_force(pack_index);

	return this->insert_res_.set_error_no(0);
}

int GamePackage::insert_with_res(int pack_index, const ItemObj& item_obj)
{
	this->insert_res_.reset();

	if (this->validate_pack_index(pack_index) == false)
	{
		return this->insert_res_.set_error_no(ERROR_INVALID_PARAM);
	}

	if (this->item_list_map_.count(pack_index) > 0)
	{
		return this->insert_res_.set_error_no(ERROR_INVALID_PARAM);
	}

	PackageItem* pack_item = GamePackage::pop_item(item_obj.id_);
	pack_item->set(item_obj.id_, item_obj.amount_, item_obj.bind_);

	this->insert_res_.obj_map_[pack_index] = item_obj;
	this->insert_by_index(pack_index, pack_item);

	return this->insert_res_.set_error_no(0);
}

int GamePackage::insert_by_index(PackageItem* pack_item)
{
	return this->insert_by_index(pack_item->__index, pack_item);
}

int GamePackage::insert_by_index(int pack_index, PackageItem* pack_item)
{
	pack_item->__index = pack_index;
	this->item_list_map_[pack_index] = pack_item;
	return 0;
}

int GamePackage::remove_item(int pack_index, int item_amount)
{
	PackageItem* pack_item = this->find_by_index(pack_index);
	JUDGE_RETURN(pack_item != NULL, ERROR_CLIENT_OPERATE);

	return this->remove_item(pack_item, item_amount);
}

int GamePackage::remove_item(PackageItem* pack_item, int item_amount)
{
	this->remove_result_.reset();

	if (pack_item == NULL || item_amount <= 0)
	{
		this->remove_result_.error_no_ = ERROR_CLIENT_OPERATE;
		return this->remove_result_.error_no_;
	}

	if (this->is_index_lock(pack_item->__index) == true)
	{
		this->remove_result_.error_no_ = ERROR_PACKAGE_LOCKED;
		return this->remove_result_.error_no_;
	}

	if (item_amount > pack_item->__amount)
	{
		this->remove_result_.error_no_ = ERROR_PACKAGE_ITEM_AMOUNT;
		return this->remove_result_.error_no_;
	}

	return this->remove_item_i(pack_item, item_amount);
}

int GamePackage::remove_all_by_id(int item_id)
{
	this->remove_result_.reset();

	ItemVector remove_set;
	JUDGE_RETURN(this->find_all_without_lock(remove_set, item_id) == true,
			ERROR_PACKAGE_ITEM_AMOUNT);

	for (ItemVector::iterator iter = remove_set.begin();
			iter != remove_set.end(); ++iter)
	{
		this->remove_item_i(*iter);
	}

	return 0;
}

int GamePackage::remove_by_id(int item_id, int item_amount)
{
	this->remove_result_.reset();
	if (item_amount <= 0)
	{
		this->remove_result_.error_no_ = ERROR_CLIENT_OPERATE;
		return this->remove_result_.error_no_;
	}

	ItemVector bind_set, unbind_set;
	int bind_count = 0, unbind_count = 0;

	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);
		JUDGE_CONTINUE(pack_item->expire_item() == false);
//		JUDGE_CONTINUE(this->is_index_lock(pack_item->__index) == false);

		if (pack_item->__bind == GameEnum::ITEM_BIND)
		{
			bind_set.push_back(pack_item);
			bind_count += pack_item->__amount;
		}
		else
		{
			unbind_set.push_back(pack_item);
			unbind_count += pack_item->__amount;
		}
	}

	if (bind_count + unbind_count < item_amount)
	{
		this->remove_result_.error_no_ = ERROR_PACKAGE_GOODS_AMOUNT;
		return this->remove_result_.error_no_;
	}

	for (ItemVector::iterator iter = bind_set.begin(); iter != bind_set.end();
			++iter)
	{
		// bind
		PackageItem* pack_item = *iter;
		JUDGE_CONTINUE(pack_item != NULL);

		int remove_amount = std::min(item_amount, pack_item->__amount);
		this->remove_item_i(pack_item, remove_amount);

		item_amount -= remove_amount;
		JUDGE_CONTINUE(item_amount <= 0);

		return 0;
	}

	for (ItemVector::iterator iter = unbind_set.begin(); iter != unbind_set.end();
			++iter)
	{
		// no bind
		PackageItem* pack_item = *iter;
		JUDGE_CONTINUE(pack_item != NULL);

		int remove_amount = std::min(item_amount, pack_item->__amount);
		this->remove_item_i(pack_item, remove_amount);

		item_amount -= remove_amount;
		JUDGE_CONTINUE(item_amount <= 0);

		return 0;
	}

	return 0;
}

int GamePackage::remove_item_index(PackageItem* pack_item)
{
	JUDGE_RETURN(pack_item != NULL, -1);
	JUDGE_RETURN(this->item_list_map_.count(pack_item->__index) > 0, -1);

	this->item_list_map_.erase(pack_item->__index);
	return 0;
}

int GamePackage::pack_transaction(const ItemObj& remove_item, const ItemObj& add_item)
{
	// remove first
	int remove_ret = this->remove_by_id(remove_item.id_,
			remove_item.amount_);
	JUDGE_RETURN(remove_ret == 0, remove_ret);

	int add_ret = this->insert_with_res(add_item);
	JUDGE_RETURN(add_ret != 0, add_ret);

	// if add fail, restore remove
	PackRemoveResult& remove_result = this->remove_result_;
	for (IntMap::iterator iter = remove_result.remove_map_.begin();
			iter != remove_result.remove_map_.end(); ++iter)
	{
		int item_bind = GameEnum::ITEM_BIND;

		if (remove_result.unbind_count_ >= remove_result.unbind_count_)
		{
			item_bind = GameEnum::ITEM_NO_BIND;
			remove_result.unbind_count_ -= remove_result.unbind_count_;
		}

		this->insert_with_res(iter->first, ItemObj(remove_result.item_id_,
				iter->second, item_bind));
	}

	return add_ret;
}

PackageItem* GamePackage::find_by_index(int pack_index)
{
	ItemListMap::iterator iter = this->item_list_map_.find(pack_index);
	return iter != this->item_list_map_.end() ? iter->second : NULL;
}

PackageItem* GamePackage::find_by_id(int item_id)
{
	PackageItem* pack_item = NULL;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
//		PackageItem* pack_item = iter->second;
//		JUDGE_CONTINUE(pack_item->__id == item_id);

//		return pack_item;
		JUDGE_CONTINUE(iter->second->__id == item_id);

		if (pack_item == NULL)
		{
			pack_item = iter->second;
		}
		else
		{
			if (iter->second->__amount > pack_item->__amount)
				pack_item = iter->second;
		}
	}

	return pack_item;
}

PackageItem* GamePackage::find_by_bind_id(int item_id)
{
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id
				&& pack_item->__bind == GameEnum::ITEM_BIND);

		return pack_item;
	}

	return NULL;
}

PackageItem* GamePackage::find_by_unbind_id(int item_id)
{
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id
				&& pack_item->__bind != GameEnum::ITEM_BIND);

		return pack_item;
	}

	return NULL;
}

PackageItem* GamePackage::find_by_unique_id(Int64 item_unique_id)
{
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__unique_id == item_unique_id);

		return pack_item;
	}

	return NULL;
}

int GamePackage::find_all(IntVec& item_set, int item_id)
{
	int find_flag = false;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);

		find_flag = true;
		item_set.push_back(iter->first);
	}

	return find_flag;
}

int GamePackage::find_all(ItemVector& item_set, int item_id)
{
	int find_flag = false;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);

		find_flag = true;
		item_set.push_back(pack_item);
	}

	return find_flag;
}

int GamePackage::find_all(ItemListMap& item_map, int item_id)
{
	int find_flag = false;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);

		find_flag = true;
		item_map[iter->first] = pack_item;
	}

	return find_flag;
}

int GamePackage::find_all_without_lock(ItemVector& item_set, int item_id)
{
	int find_flag = false;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);
		JUDGE_CONTINUE(this->is_index_lock(pack_item->__index) == false);

		find_flag = true;
		item_set.push_back(pack_item);
	}

	return find_flag;
}

int GamePackage::count_by_id(int item_id)
{
	int find_count = 0;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);
		JUDGE_CONTINUE(pack_item->expire_item() == false);
		find_count += pack_item->__amount;
	}

	return find_count;
}

int GamePackage::count_by_bind_id(int item_id)
{
	int find_count = 0;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);
		JUDGE_CONTINUE(pack_item->__bind == GameEnum::ITEM_BIND);
		JUDGE_CONTINUE(pack_item->expire_item() == false);
		find_count += pack_item->__amount;
	}

	return find_count;
}

int GamePackage::count_by_unbind_id(int item_id)
{
	int find_count = 0;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		JUDGE_CONTINUE(pack_item->__id == item_id);
		JUDGE_CONTINUE(pack_item->__bind != GameEnum::ITEM_BIND);
		JUDGE_CONTINUE(pack_item->expire_item() == false);
		find_count += pack_item->__amount;
	}

	return find_count;
}

const PackInsertResult& GamePackage::get_insert_result()
{
	return this->insert_res_;
}

const PackRemoveResult& GamePackage::get_remove_result()
{
	return this->remove_result_;
}

bool GamePackage::finish_open_grid(void)
{
	return this->size() >= this->pack_max_size();
}

bool GamePackage::validate_open_index(int pack_index)
{
	return pack_index >= this->size() && pack_index < this->pack_max_size();
}

void GamePackage::open_pack_grid(int pack_index, Int64 online_tick)
{
	this->pack_size_ = pack_index + 1;
	this->last_open_tick_ = online_tick;
}

int GamePackage::fetch_open_grid_info(PackOpenInfo& open_info, int pack_index,double rate)
{
//	JUDGE_RETURN(this->finish_open_grid() == false, ERROR_CLIENT_OPERATE);
//	JUDGE_RETURN(pack_index >= this->pack_size_, ERROR_CLIENT_OPERATE);
//
//	const Json::Value& exp_cfg = CONFIG_INSTANCE->tiny("pack_grid_exp");
//	const Json::Value& time_cfg = CONFIG_INSTANCE->tiny("pack_grid_time");
//	JUDGE_RETURN(this->pack_max_size() == (int)time_cfg.size(), ERROR_CONFIG_ERROR);
//
//	// first grid
//	open_info.add_exp_ += exp_cfg[this->pack_size_].asInt();
//	open_info.total_second_ += time_cfg[this->pack_size_].asInt()*(1.00 - rate);
//
//	Int64 diff_online_tick = ::time(NULL) - this->last_open_tick_;
//	if (diff_online_tick >= open_info.total_second_)
//	{
//		open_info.left_second_ = 0;
//	}
//	else
//	{
//		open_info.left_second_ = open_info.total_second_ - diff_online_tick;
//	}
//
//	// next total grid
//	for (int i = this->pack_size_ + 1; i <= pack_index; ++i)
//	{
//		int cur_time = time_cfg[i].asInt();
//		open_info.add_exp_ += exp_cfg[i].asInt();
//
//		open_info.left_second_ += cur_time;
//		open_info.total_second_ += cur_time;
//	}

	return 0;
}

int GamePackage::fetch_open_grid_money(int left_second)
{
//	JUDGE_RETURN(left_second > 0, 0);
//
//	static int pack_time_money = CONFIG_INSTANCE->tiny("pack_time_money")[1u].asInt();
//	static int pack_grid_unit_second = CONFIG_INSTANCE->tiny("pack_time_money")[0u].asInt();
//
//	return GAME_PAGE_SIZE(left_second, pack_grid_unit_second) * pack_time_money;
	return 0;
}

int GamePackage::pack_lock(int pack_index, int lock_type)
{
	JUDGE_RETURN(this->check_and_lock(lock_type) == false, -1);
	JUDGE_RETURN(this->is_index_lock(pack_index) == false, -1);

	this->lock_map_[pack_index] = true;
	return 0;
}

int GamePackage::pack_unlock(int pack_index, int lock_type)
{
	JUDGE_RETURN(this->check_lock_type(lock_type) == true, -1);
	JUDGE_RETURN(this->is_index_lock(pack_index) == true, -1);

	this->lock_map_.erase(pack_index);
	JUDGE_RETURN(this->lock_map_.empty() == true, -1);

	this->lock_type_ = 0;
	return 0;
}

int GamePackage::check_and_lock(int lock_type)
{
	JUDGE_RETURN(this->check_lock_type(lock_type) == false, true);
	JUDGE_RETURN(this->check_lock_type(0) == true, false);
	this->lock_type_ = lock_type;
	return true;
}

int GamePackage::pack_unlock_all(int lock_type)
{
	JUDGE_RETURN(this->check_lock_type(lock_type) == true, -1);
	this->lock_type_ = 0;
	this->lock_map_.clear();
	return 0;
}

int GamePackage::is_index_lock(int pack_index)
{
	return this->lock_map_.count(pack_index) > 0;
}

int GamePackage::check_lock_type(int lock_type)
{
	return this->lock_type_ == lock_type;
}

void GamePackage::serialize(ProtoPackage* proto_package)
{
	JUDGE_RETURN(proto_package != NULL, ;);

	proto_package->set_pack_type(this->type());
	proto_package->set_pack_size(this->size());

	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		ProtoItem* proto_item = proto_package->add_item_list();
		iter->second->serialize(proto_item);
	}

	for (PackGridVec::iterator iter = this->grid_vec_.begin();
			iter != this->grid_vec_.end(); ++iter)
	{
		ProtoPairObj* pair = proto_package->add_grid_vec();
		pair->set_obj_id(iter->strengthen_lvl_);
		pair->set_obj_value(iter->strengthen_bless_);
	}

	proto_package->set_is_open_sublime(this->is_open_sublime());
	proto_package->set_sublime_level(this->sublime_level());
}

void GamePackage::unserialize(const ProtoPackage& proto_package)
{
	for (int i = 0; i < proto_package.item_list_size(); ++i)
	{
		const ProtoItem& proto_item = proto_package.item_list(i);

		PackageItem* pack_item = GamePackage::pop_item(proto_item.id());
		JUDGE_CONTINUE(pack_item != NULL);

		pack_item->unserialize(proto_package.item_list(i));
		this->insert_by_index(pack_item);
	}

	for (int i = 0; i < proto_package.grid_vec_size(); ++i)
	{
		PackGridInfo* grid_info = this->grid_info(i);
		JUDGE_CONTINUE(grid_info != NULL);

		const ProtoPairObj& pair = proto_package.grid_vec(i);
		grid_info->strengthen_lvl_ = pair.obj_id();
		grid_info->strengthen_bless_ = pair.obj_value();
	}

	this->set_sublime_info(proto_package.is_open_sublime(), proto_package.sublime_level());
}

void GamePackage::serialize(ProtoItem* proto_item, int pack_index)
{
	JUDGE_RETURN(this->item_list_map_.count(pack_index) > 0, ;);
	return this->serialize(proto_item, this->item_list_map_[pack_index]);
}

void GamePackage::serialize(ProtoItem* proto_item, PackageItem* pack_item)
{
	pack_item->serialize(proto_item);
	this->make_up_item_force(proto_item, pack_item);
}

int GamePackage::sort_and_merge(int remove)
{
	ItemVector item_vec;
	ItemVector remove_vec;
	item_vec.reserve(this->item_list_map_.size());
	remove_vec.reserve(this->item_list_map_.size());

	for (ItemListMap::iterator iter = this->item_list_map_.begin();
				iter != this->item_list_map_.end(); ++iter)
	{
	    PackageItem* pack_item = iter->second;
	    JUDGE_CONTINUE(pack_item != NULL);

	    if (remove == false)
	    {
	    	item_vec.push_back(pack_item);
	    	continue;
	    }

	    if (pack_item->expire_item() == true)
	    {
	    	remove_vec.push_back(pack_item);
	    	continue;
	    }

	    item_vec.push_back(pack_item);
	}

	this->item_list_map_.clear();
	for (ItemVector::iterator iter = remove_vec.begin(); iter != remove_vec.end(); ++iter)
	{
		GamePackage::push_item(*iter);
	}

	JUDGE_RETURN(item_vec.empty() == false, -1);

	PackageItem* prev_item = NULL;
	std::stable_sort(item_vec.begin(), item_vec.end(), GamePackage::pack_sort_cmp);
	for (ItemVector::iterator iter = item_vec.begin(); iter != item_vec.end(); ++iter)
	{
		PackageItem* cur_item = *iter;
		JUDGE_CONTINUE(GamePackage::merge_and_push(cur_item, prev_item) == false);

		if (prev_item == NULL)
		{
			cur_item->__index = this->pack_base_index();
		}
		else
		{
			cur_item->__index = prev_item->__index + 1;
		}

		this->item_list_map_[cur_item->__index] = cur_item;
		prev_item = cur_item;
	}

	return 0;
}

int GamePackage::fetch_require_space(const ItemObj& obj, int max_amount)
{
	ItemObj insertObj = obj;

	insertObj.amount_ = this->insert_old_space(insertObj, max_amount);
	JUDGE_RETURN(insertObj.amount_ > 0, true);

	insertObj.amount_ = this->insert_new_space(insertObj, max_amount);
	JUDGE_RETURN(insertObj.amount_ > 0, true);

	return false;
}

bool GamePackage::pack_check(int pack_type)
{
//	switch(pack_type)
//	{
//	case GameEnum::INDEX_EQUIP:
//	case GameEnum::INDEX_PACKAGE:
//	case GameEnum::INDEX_WARDROBE:
//	case GameEnum::INDEX_STORE:
//	case GameEnum::INDEX_BOX_STORE:
//	case GameEnum::INDEX_MOUNT:
//		return true;
//	default:
//		return false;
//	}
	return false;
}

bool GamePackage::merge_and_push(PackageItem* cur_item, PackageItem* prev_item)
{
	JUDGE_RETURN(cur_item != NULL && prev_item != NULL, false);

	if (cur_item->time_item() == true)
	{
		return false;
	}

	int overlap_count = GameCommon::item_overlap_count(prev_item->__id);
	if (overlap_count == 1)
	{
		return false;
	}

	if (cur_item->__id != prev_item->__id)
	{
		return false;
	}

	if (cur_item->__bind != prev_item->__bind)
	{
		return false;
	}

	int total_amount = cur_item->__amount + prev_item->__amount;
	if (total_amount > overlap_count)
	{
		cur_item->__amount = total_amount - overlap_count;
		prev_item->__amount = overlap_count;

		return false;
	}
	else
	{
		cur_item->__amount = 0;
		GamePackage::push_item(cur_item);

		prev_item->__amount = total_amount;
		return true;
	}
}

bool GamePackage::pack_sort_cmp(const PackageItem* left, const PackageItem* right)
{
	if (left->__id == right->__id)
	{
	    // 物品绑定
	    if (left->__bind != right->__bind)
	    {
	        return left->__bind > right->__bind;
	    }

	    // 物品数量
	    if (left->__amount != right->__amount)
	    {
	    	return left->__amount > right->__amount;
	    }

	    return true;
	}

    const Json::Value& left_json = CONFIG_INSTANCE->item(left->__id);
    const Json::Value& right_json = CONFIG_INSTANCE->item(right->__id);

    // 排序字段
    int left_sort = GameCommon::json_value(left_json, "sort");
    int right_sort = GameCommon::json_value(right_json, "sort");
    if (left_sort != right_sort)
    {
    	return left_sort > right_sort;
    }

    // 物品类型
    int left_type = left_json["type"].asInt();
    int right_type = right_json["type"].asInt();
    if (left_type != right_type)
    {
    	return left_type < right_type;
    }

	// 品质
	int left_color = GameCommon::json_value(left_json, "color");
	int right_color = GameCommon::json_value(right_json, "color");
	if (left_color != right_color)
	{
		return left_color > right_color;
	}

	// 等级
	int left_lvl = GameCommon::json_value(left_json, "lvl");
	int right_lvl = GameCommon::json_value(right_json, "lvl");
	if (left_lvl != right_lvl)
	{
		return left_lvl > right_lvl;
	}

	// 部位
	int left_part = GameCommon::json_value(left_json, "part");
	int right_part = GameCommon::json_value(right_json, "part");
	if (left_part != right_part)
	{
		return left_part < right_part;
	}

    return left->__id < right->__id;
}

int GamePackage::pack_type_has_force(int pack_type)
{
	switch(pack_type)
	{
	case GameEnum::INDEX_EQUIP:
		return true;
	}

	return false;
}

int GamePackage::item_has_force(PackageItem* pack_item)
{
	return pack_item->__force_flag == true;
}

int GamePackage::check_insert(const ItemObj& obj)
{
	JUDGE_RETURN(obj.amount_ > 0, ERROR_INVALID_PARAM);

	const Json::Value& item_conf = CONFIG_INSTANCE->item(obj.id_);
	JUDGE_RETURN(item_conf.empty() == false, ERROR_CONFIG_ERROR);

	int overlap_count = GameCommon::item_overlap_count(obj.id_);
	JUDGE_RETURN(this->fetch_require_space(obj, overlap_count) == true,
			ERROR_PACKAGE_NO_CAPACITY);

	return 0;
}

int GamePackage::insert_old_space(const ItemObj& obj, int max_amount)
{
	JUDGE_RETURN(max_amount > 1, obj.amount_);

	//可以叠加，优先插入原来的格子
	ItemObj add_obj = obj;
	int cur_amount = obj.amount_;

	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter)
	{
		PackageItem* item = iter->second;
		JUDGE_CONTINUE(item->__id == obj.id_);
		JUDGE_CONTINUE(item->__bind == obj.bind_);
		JUDGE_CONTINUE(item->time_item() == false);	//时间物品，不可二次叠加

		int left_amount = max_amount - iter->second->__amount;
		JUDGE_CONTINUE(left_amount > 0);

		if (cur_amount > left_amount)
		{
			add_obj.amount_ = left_amount;
			this->insert_res_.obj_map_[iter->first] = add_obj;
			cur_amount -= left_amount;
		}
		else
		{
			add_obj.amount_ = cur_amount;
			this->insert_res_.obj_map_[iter->first] = add_obj;
			cur_amount = 0;
			break;
		}
	}

	return cur_amount;
}

int GamePackage::insert_new_space(const ItemObj& obj, int max_amount)
{
	JUDGE_RETURN(obj.amount_ > 0, 0);

	//插入新格子
	ItemObj add_obj = obj;
	int cur_amount = obj.amount_;

	for (int i = 0; i < this->size(); ++i)
	{
		JUDGE_CONTINUE(this->insert_res_.obj_map_.count(i) == 0);

		PackageItem* item = this->find_by_index(i);
		JUDGE_CONTINUE(item == NULL);

		if (cur_amount > max_amount)
		{
			add_obj.amount_ = max_amount;
			this->insert_res_.obj_map_[i] = add_obj;
			cur_amount -= max_amount;
		}
		else
		{
			add_obj.amount_ = cur_amount;
			this->insert_res_.obj_map_[i] = add_obj;
			cur_amount = 0;
			break;
		}
	}

	return cur_amount;
}

void GamePackage::insert_operate()
{
	for (ItemObjMap::iterator iter = this->insert_res_.obj_map_.begin();
			iter != this->insert_res_.obj_map_.end(); ++iter)
	{
		PackageItem* pack_item = this->find_by_index(iter->first);
		if (pack_item != NULL)
		{
			pack_item->__amount += iter->second.amount_;
		}
		else
		{
			const ItemObj& obj= iter->second;

			pack_item = GamePackage::pop_item(obj.id_);
			pack_item->set(obj.id_, obj.amount_, obj.bind_);

			this->insert_by_index(iter->first, pack_item);
		}
	}
}

int GamePackage::search_empty_index_i(void)
{
	JUDGE_RETURN(this->left_capacity() > 0, ERROR_PACKAGE_NO_CAPACITY);

	for (int i = 0; i < this->size(); ++i)
	{
		int pack_index = this->fetch_pack_index(i);

		JUDGE_CONTINUE(this->is_index_lock(pack_index) == false);
		JUDGE_CONTINUE(this->find_by_index(pack_index) == NULL);

		return pack_index;
	}

	return ERROR_NO_ENOUGH_SPACE;
}

bool GamePackage::is_equip(void)
{
	return this->type() == GameEnum::INDEX_EQUIP;
}

bool GamePackage::is_package(void)
{
	return this->type() == GameEnum::INDEX_PACKAGE;
}

bool GamePackage::is_box_store(void)
{
	return this->type() == GameEnum::INDEX_BOX_STORE;
}

PackageItem* GamePackage::pop_item(int item_id, int item_amount)
{
	JUDGE_RETURN(item_id == 0 || GameCommon::validate_item_id(item_id) == true, NULL);

    PackageItem* pack_item = POOL_MONITOR->pack_item_pool()->pop();
    JUDGE_RETURN(pack_item != NULL, NULL);

    pack_item->__id = item_id;
    pack_item->__amount = item_amount;

    GamePackage::init_item_info(pack_item);
    GamePackage::init_prop_info(pack_item);

    return pack_item;
}

int GamePackage::push_item(PackageItem *pack_item)
{
	JUDGE_RETURN(pack_item != NULL, -1);
    return POOL_MONITOR->pack_item_pool()->push(pack_item);
}

int GamePackage::init_item_info(PackageItem* pack_item)
{
	JUDGE_RETURN(pack_item->validate_item() == true, -1);

	const Json::Value& conf = pack_item->conf();

	int sub_type = conf["sub_type"].asInt();
	if (sub_type >= 18 && sub_type <= 27)
	{
		//战骑类
		pack_item->__force_flag = true;
		pack_item->caculate_fight_prop();
	}

	int expire_time = conf["expire_time"].asInt();
	if (expire_time > 0)
	{
		pack_item->__timeout = ::time(NULL) + expire_time;
	}

	int tips_time_list_size = conf["tips_time_list"].size();
	for (int i = 0; i < tips_time_list_size; ++i)
	{
		pack_item->__tips_time_map[i] = conf["tips_time_list"][i].asInt();
		pack_item->__tips_status_map[i] = 1;
	}

	pack_item->__tips_level = conf["tips_level"].asInt();

	return 0;
}

int GamePackage::init_prop_info(PackageItem* pack_item)
{
	return 0;
}

int GamePackage::fetch_max_pack_size(int pack_type, int pack_size)
{
	switch (pack_type)
	{
	case GameEnum::INDEX_EQUIP:
	{
		return GameEnum::EQUIP_MAX_INDEX;
	}
	case GameEnum::INDEX_PACKAGE:
	{
		return std::max<int>(GameEnum::PACK_MIN_INDEX, pack_size);
	}
	case GameEnum::INDEX_STORE:
	{
		return std::max<int>(GameEnum::STORE_MIN_INDEX,pack_size);
	}
	case GameEnum::INDEX_BOX_STORE:
	{
		return GameEnum::BOX_STORE_MIN_INDEX;
	}
	case GameEnum::INDEX_MOUNT:
	{
		return GameEnum::MOUNT_STORE_MAX_INDEX;
	}
	default:
	{
		return pack_size;
	}
	}
}

int GamePackage::remove_item_i(PackageItem* pack_item)
{
	return this->remove_item_i(pack_item, pack_item->__amount);
}

int GamePackage::remove_item_i(PackageItem* pack_item, int item_amount)
{
	this->remove_result_.item_id_ = pack_item->__id;
	this->remove_result_.remove_map_[pack_item->__index] = item_amount;
	if (pack_item->__bind == GameEnum::ITEM_BIND)
	{
		this->remove_result_.bind_count_ += item_amount;
	}
	else
	{
		this->remove_result_.unbind_count_ += item_amount;
	}

	pack_item->__amount -= item_amount;
	if (pack_item->__amount <= 0)
	{
		this->item_list_map_.erase(pack_item->__index);
		GamePackage::push_item(pack_item);
	}

	return 0;
}

int GamePackage::fetch_pack_index(int index)
{
//	return this->pack_base_index() + index + 1;
	return index;
}

int GamePackage::check_pack_is_order(ItemVector& item_set)
{
	JUDGE_RETURN(this->item_list_map_.empty() == false, true);
	item_set.reserve(this->item_list_map_.size());

	bool is_order = true;
	int cur_index = this->pack_type_ + 1;

	PackageItem* last_item = NULL;
	for (ItemListMap::iterator iter = this->item_list_map_.begin();
			iter != this->item_list_map_.end(); ++iter, ++cur_index)
	{
		PackageItem* pack_item = iter->second;
		if (pack_item == NULL)
		{
			is_order = false;
			continue;
		}

		item_set.push_back(iter->second);
		JUDGE_CONTINUE(is_order == true);

		if (iter->first != cur_index)
		{
			is_order = false;
			continue;
		}

		if (last_item == NULL)
		{
			last_item = pack_item;
			continue;
		}

		is_order = GamePackage::pack_sort_cmp(last_item, pack_item);
	}

	return is_order;
}

void GamePackage::make_up_item_force(ProtoItem* proto_item, PackageItem* pack_item)
{
	JUDGE_RETURN(GamePackage::pack_type_has_force(this->type()) == true
			|| GamePackage::item_has_force(pack_item) == true, ;);
	proto_item->set_force(pack_item->__prop_info.force_);
}

