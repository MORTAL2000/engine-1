/**
 * @file
 */

#pragma once

#include <gmock/gmock.h>
#include "persistence/PersistenceMgr.h"

namespace persistence {
class DBHandlerMock : public DBHandler {
public:
	MOCK_METHOD(bool, init, ());
	MOCK_METHOD(void, shutdown, ());

	MOCK_METHOD(Connection*, connection, (), (const));
	MOCK_METHOD(bool, createTable, (Model&&), (const));
	MOCK_METHOD(bool, createOrUpdateTable, (Model&&), (const));

	MOCK_METHOD(bool, exec, (const core::String&), (const));
};

class PersistenceMgrMock : public PersistenceMgr {
public:
	PersistenceMgrMock() :
			PersistenceMgr(std::make_shared<DBHandlerMock>()) {
	}
	MOCK_METHOD0(init, bool());
	MOCK_METHOD0(shutdown, void());

	MOCK_METHOD2(registerSavable, bool(uint32_t, ISavable *));
	MOCK_METHOD2(unregisterSavable, bool(uint32_t, ISavable *));
};

inline std::shared_ptr<DBHandlerMock> createDbHandlerMock() {
	auto dbHandler = std::make_shared<persistence::DBHandlerMock>();
	EXPECT_CALL(*dbHandler, connection()).WillRepeatedly(testing::ReturnNull());
	EXPECT_CALL(*dbHandler, exec(testing::_)).WillRepeatedly(testing::Return(true));
	EXPECT_CALL(*dbHandler, createTable(testing::_)).WillRepeatedly(testing::Return(true));
	EXPECT_CALL(*dbHandler, createOrUpdateTable(testing::_)).WillRepeatedly(testing::Return(true));
	return dbHandler;
}

inline std::shared_ptr<persistence::PersistenceMgrMock> createPersistenceMgrMock() {
	auto persistenceMgr = std::make_shared<persistence::PersistenceMgrMock>();
	EXPECT_CALL(*persistenceMgr, registerSavable(testing::_, testing::_)).WillRepeatedly(testing::Return(true));
	EXPECT_CALL(*persistenceMgr, unregisterSavable(testing::_, testing::_)).WillRepeatedly(testing::Return(true));
	return persistenceMgr;
}

}
