#include "../interfaces/storage/StorageInterface.h"
#include "../interfaces/storage/Table.h"

using namespace bcos::storage;

std::optional<TableInfo> StorageInterface::getSysTableInfo(const std::string& tableName)
{
    if (tableName == SYS_TABLES)
    {
        return TableInfo(
            tableName, "table_name", {std::string("key_field"), std::string("value_fields")});
    }
    return {};
}

void StorageInterface::asyncCreateTable(const std::string& _tableName, const std::string& _keyField,
    const std::string& _valueFields, std::function<void(Error::Ptr&&, bool)> callback) noexcept
{
    asyncOpenTable(SYS_TABLES, [_tableName, callback, _keyField, _valueFields](
                                   auto&& error, auto&& sysTable) {
        if (error)
        {
            callback(BCOS_ERROR_WITH_PREV_PTR(-1, "Open sys_tables failed!", error), false);
            return;
        }

        sysTable->asyncGetRow(_tableName, [_tableName, callback, &sysTable, _keyField,
                                              _valueFields](auto&& error, auto&& entry) {
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
            tableEntry.setField(SYS_TABLE_KEY_FIELDS, _keyField);
            tableEntry.setField(SYS_TABLE_VALUE_FIELDS, _valueFields);

            sysTable->asyncSetRow(_tableName, tableEntry, [callback](auto&& error, auto success) {
                if (error)
                {
                    callback(BCOS_ERROR_WITH_PREV_PTR(
                                 -1, "Put table info into sys_tables failed!", error),
                        false);
                    return;
                }

                if (!success)
                {
                    callback(BCOS_ERROR_PTR(-1, "Create table failed! table exists"), false);
                    return;
                }

                callback(nullptr, success);
            });
        });
    });
}

void StorageInterface::asyncOpenTable(const std::string& tableName,
    std::function<void(Error::Ptr&&, std::optional<Table>&&)> callback) noexcept
{
    auto sysTableInfo = getSysTableInfo(tableName);
    if (sysTableInfo)
    {
        auto table = Table(this, std::make_shared<TableInfo>(std::move(*sysTableInfo)), 0);
        callback(nullptr, std::move(table));

        return;
    }
    else
    {
        asyncOpenTable(SYS_TABLES,
            [this, callback, tableName](Error::Ptr&& error, std::optional<Table>&& sysTable) {
                if (error)
                {
                    callback(std::move(error), {});
                    return;
                }

                sysTable->asyncGetRow(
                    tableName, [this, tableName, callback](auto&& error, auto&& entry) {
                        if (error)
                        {
                            callback(std::move(error), {});
                            return;
                        }

                        if (!entry)
                        {
                            callback(nullptr, {});
                            return;
                        }

                        auto tableInfo = std::make_shared<storage::TableInfo>(tableName,
                            std::string(entry->getField(SYS_TABLE_KEY_FIELDS)),
                            entry->getField(SYS_TABLE_VALUE_FIELDS));
                        auto table = Table(this, tableInfo, 0);

                        callback(nullptr, std::move(table));
                    });
            });
    }
}