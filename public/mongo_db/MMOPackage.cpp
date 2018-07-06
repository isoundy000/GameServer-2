/*
 * MMOPackage.cpp
 *
 * Created on: 2013-05-04 10:19
 *     Author: lyz
 */

#include "GameField.h"
#include "MMOPackage.h"
#include "GameCommon.h"
#include "MongoConnector.h"
#include "MapLogicPlayer.h"

#include "PubStruct.h"
#include "MapMonitor.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOPackage::~MMOPackage(void)
{ /*NULL*/ }

int MMOPackage::load_player_package(MapLogicPlayer *player)
{
    PackageDetail &pack_detail = player->pack_detail();
    BSONObj res = this->conection().findOne(Package::COLLECTION,
    		QUERY(Package::ID << player->role_id()));

    if (res.isEmpty() == true)
    {
#ifdef TEST_COMMAND
    	player->test_add_pack_item();
#endif
        return 0;
    }

    {
        // money
	    BSONObj money_obj = res.getObjectField(Package::MONEY.c_str());
	    GameCommon::bson_to_money(pack_detail.__money, money_obj);

	    //game resource
	    BSONObj resource_obj = res.getObjectField(Package::GAME_RESOURCE.c_str());
	    GameCommon::bson_to_map(pack_detail.resource_map_, resource_obj);

	    //use game resource
	    BSONObj use_resource_obj = res.getObjectField(Package::USER_GAME_RESOURCE.c_str());
	    GameCommon::bson_to_map(pack_detail.use_resource_map_, use_resource_obj);
    }

	BSONObjIterator iter(res.getObjectField(Package::PACK.c_str()));
	while (iter.more())
	{
		BSONObj pack_obj = iter.next().embeddedObject();

		int pack_type = pack_obj[Package::Pack::PACK_TYPE].numberInt();
		int pack_size = pack_obj[Package::Pack::PACK_SIZE].numberInt();
		player->add_pack_type(pack_type, pack_size);

		GamePackage* package = player->find_package(pack_type);
		JUDGE_CONTINUE(package != NULL);

		int sublime_level = pack_obj[Package::Pack::SUBLIME_LEVEL].numberInt();
		int is_open_sublime = pack_obj[Package::Pack::IS_OPEN_SUBLIME].numberInt();
		package->set_sublime_info(is_open_sublime, sublime_level);

		BSONObjIterator item_iter(pack_obj.getObjectField(Package::Pack::PACK_ITEM.c_str()));
		while (item_iter.more())
		{
			BSONObj item_obj = item_iter.next().embeddedObject();
			int item_id = item_obj[Package::PackItem::ID].numberInt();
			if (item_obj[Package::PackItem::AMOUNT].numberInt() <= 0)
			{
				MSG_USER("ERROR package has 0 amount item %ld %s %d %d %d", player->role_id(), player->name(),
						item_obj[Package::PackItem::INDEX].numberInt(), item_id, item_obj[Package::PackItem::AMOUNT].numberInt());
				continue;
			}

			PackageItem* pack_item = GamePackage::pop_item(item_id);
			JUDGE_CONTINUE(pack_item != NULL);

			GameCommon::bson_to_item(pack_item, item_obj);
			package->insert_by_index(pack_item);
		}

		int cur_index = 0;
		BSONObjIterator str_iter(pack_obj.getObjectField(Package::Pack::STRENGTHEN.c_str()));
		while (str_iter.more())
		{
			BSONObj str_obj = str_iter.next().embeddedObject();

			PackGridInfo* grid_info = package->grid_info(cur_index);
			JUDGE_CONTINUE(grid_info != NULL);

			grid_info->strengthen_lvl_ = str_obj[DBPairObj::KEY].numberInt();
			grid_info->strengthen_bless_ = str_obj[DBPairObj::VALUE].numberInt();

			++cur_index;
		}
	}

	player->calc_equip_property();
    return 0;
}

int MMOPackage::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
    PackageDetail &pack_detail = player->pack_detail();

    BSONVec resource_vc;
    GameCommon::map_to_bson(resource_vc, pack_detail.resource_map_);

    BSONVec use_resource_vc;
    GameCommon::map_to_bson(use_resource_vc, pack_detail.use_resource_map_);

    BSONVec pack_vc;
    pack_vc.reserve(pack_detail.__pack_map.size());

	for (PackageMap::iterator iter = pack_detail.__pack_map.begin();
			iter != pack_detail.__pack_map.end(); ++iter)
	{
		GamePackage* package = iter->second;
		JUDGE_CONTINUE(package != NULL);

		BSONVec item_vec;
		item_vec.reserve(package->item_list_map_.size());

		for (ItemListMap::iterator iter = package->item_list_map_.begin();
				iter != package->item_list_map_.end(); ++iter)
		{
			PackageItem* pack_item = iter->second;
			JUDGE_CONTINUE(pack_item != NULL);
			item_vec.push_back(GameCommon::item_to_bson(pack_item));
		}

		BSONVec strengthen_vec;
		strengthen_vec.reserve(package->grid_vec().size());

		PackGridVec& grid_vec = package->grid_vec();
		for (PackGridVec::iterator iter = grid_vec.begin(); iter != grid_vec.end(); ++iter)
		{
			strengthen_vec.push_back(BSON(DBPairObj::KEY << iter->strengthen_lvl_
					<< DBPairObj::VALUE << iter->strengthen_bless_));
		}

		BSONObjBuilder builder;
		builder << Package::Pack::PACK_TYPE << package->type()
			<< Package::Pack::PACK_SIZE << package->size()
			<< Package::Pack::PACK_ITEM << item_vec
			<< Package::Pack::STRENGTHEN << strengthen_vec
			<< Package::Pack::SUBLIME_LEVEL << package->sublime_level()
			<< Package::Pack::IS_OPEN_SUBLIME << package->is_open_sublime();

		pack_vc.push_back(builder.obj());
	}

	{
		BSONObjBuilder builder;
		builder << Package::MONEY << GameCommon::money_to_bson(pack_detail.__money)
			<< Package::PACK << pack_vc
			<< Package::GAME_RESOURCE << resource_vc
			<< Package::USER_GAME_RESOURCE << use_resource_vc;

		data_map->push_update(Package::COLLECTION, BSON(Package::ID << player->role_id()),
				builder.obj(), true);
	}

    return 0;
}



void MMOPackage::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(Package::COLLECTION, BSON(Package::ID << 1), true);
END_CATCH
}
