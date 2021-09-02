#include "../interfaces/storage/StorageInterface.h"
#include "../interfaces/storage/Table.h"

using namespace bcos::storage;

TableInfo::Ptr StorageInterface::getSysTableInfo(const std::string& tableName) const
{
    if (tableName == SYS_TABLES)
    {
        return std::make_shared<storage::TableInfo>(tableName, SYS_TABLE_KEY,
            std::string(SYS_TABLE_KEY_FIELDS) + "," + SYS_TABLE_VALUE_FIELDS);
    }
    return nullptr;
}

void StorageInterface::asyncCreateTable(const std::string& _tableName, const std::string& _keyField,
    const std::string& _valueFields, std::function<void(Error::Ptr&&, bool)> callback) noexcept
{
    asyncOpenTable(SYS_TABLES, [_tableName, callback, _keyField, _valueFields](
                                   Error::Ptr&& error, std::shared_ptr<Table>&& sysTable) {
        if (error)
        {
            callback(BCOS_ERROR_WITH_PREV_PTR(-1, "Open sys_tables failed!", error), false);
            return;
        }

        sysTable->asyncGetRow(_tableName, [_tableName, callback, sysTable, _keyField, _valueFields](
                                              Error::Ptr&& error, storage::Entry::Ptr&& entry) {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV_PTR(-1, "Get table info row failed!", error), false);
                return;
            }

            if (entry)
            {
                callback(nullptr, false);
                return;
            }

            auto tableEntry = sysTable->newEntry();
            tableEntry->setField(SYS_TABLE_KEY_FIELDS, _keyField);
            tableEntry->setField(SYS_TABLE_VALUE_FIELDS, _valueFields);

            sysTable->asyncSetRow(_tableName, tableEntry, [callback](Error::Ptr&& error, bool) {
                if (error)
                {
                    callback(BCOS_ERROR_WITH_PREV_PTR(
                                 -1, "Put table info into sys_tables failed!", error),
                        false);
                    return;
                }

                callback(nullptr, true);
            });
        });
    });
}

void StorageInterface::asyncOpenTable(const std::string& tableName,
    std::function<void(Error::Ptr&&, std::shared_ptr<Table>&&)> callback) noexcept
{
    auto sysTableInfo = getSysTableInfo(tableName);
    if (sysTableInfo)
    {
        auto table = std::make_shared<storage::Table>(this, sysTableInfo, 0);
        callback(nullptr, std::move(table));

        return;
    }
    else
    {
        asyncOpenTable(SYS_TABLES, [this, callback, tableName](
                                       Error::Ptr&& error, storage::Table::Ptr&& sysTable) {
            if (error)
            {
                callback(std::move(error), nullptr);
                return;
            }

            sysTable->asyncGetRow(tableName, [this, tableName, callback](
                                                 Error::Ptr&& error, storage::Entry::Ptr&& entry) {
                if (error)
                {
                    callback(std::move(error), nullptr);
                    return;
                }

                if (!entry)
                {
                    callback(nullptr, nullptr);
                    return;
                }

                auto tableInfo = std::make_shared<storage::TableInfo>(tableName,
                    entry->getField(SYS_TABLE_KEY_FIELDS), entry->getField(SYS_TABLE_VALUE_FIELDS));
                auto table = std::make_shared<storage::Table>(this, tableInfo, 0);

                callback(nullptr, std::move(table));
            });
        });
    }
}