/**
 * @file
 */

#include "UserStockMgr.h"
#include "BackendModels.h"
#include "../User.h"

namespace backend {

UserStockMgr::UserStockMgr(User* user, const stock::ItemProviderPtr& itemProvider, const persistence::DBHandlerPtr& dbHandler) :
		_user(user), _dbHandler(dbHandler) {
}

void UserStockMgr::update(long dt) {
}

void UserStockMgr::init() {
	const EntityId userId = _user->id();
	if (!_dbHandler->select(db::StockModel(), db::DBConditionStockUserid(userId), [this] (db::StockModel&& model) {
		const stock::ItemId itemId = model.itemid();
		const stock::ItemPtr& item = _itemProvider->createItem(itemId);
		if (!item) {
			return;
		}
		item->changeAmount(model.amount());
		_stock.add(item);
	})) {
		Log::warn("Could not load stock for user %" PRIEntId, userId);
	}
	if (!_dbHandler->select(db::InventoryModel(), db::DBConditionInventoryUserid(userId), [this] (db::InventoryModel&& model) {
		// TODO: load inventory
		//_stock.inventory().add();
	})) {
		Log::warn("Could not load inventory for user %" PRIEntId, userId);
	}
}

void UserStockMgr::shutdown() {
}

}
