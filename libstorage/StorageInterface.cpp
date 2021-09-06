#include "../interfaces/storage/StorageInterface.h"
#include "../interfaces/storage/Table.h"
#include <boost/range/detail/implementation_help.hpp>

using namespace bcos::storage;

const TableInfo* StorageInterface::getSysTableInfo(const std::string_view& tableName)
{
    struct SystemTables
    {
        SystemTables() { tables.push_back({SYS_TABLES, {SYS_TABLE_VALUE_FIELDS}}); }

        std::vector<TableInfo> tables;
    } static m_systemTables;

    if (tableName == SYS_TABLES)
    {
        return &m_systemTables.tables[0];
    }

    return nullptr;
}

void StorageInterface::asyncCreateTable(std::string _tableName, std::string _valueFields,
    std::function<void(Error::Ptr&&, bool)> callback) noexcept
{
    asyncOpenTable(SYS_TABLES, [tableName = std::move(_tableName), callback,
                                   valueFields = std::move(_valueFields)](
                                   auto&& error, auto&& sysTable) {
        if (error)
        {
            callback(BCOS_ERROR_WITH_PREV_PTR(-1, "Open sys_tables failed!", error), false);
            return;
        }

        sysTable->asyncGetRow(tableName, [tableName, callback, &sysTable,
                                             valueFields = std::move(valueFields)](
                                             auto&& error, auto&& entry) {
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
            tableEntry.setField(SYS_TABLE_VALUE_FIELDS, std::string(valueFields));

            sysTable->asyncSetRow(tableName, tableEntry, [callback](auto&& error, auto success) {
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

void StorageInterface::asyncOpenTable(std::string_view tableName,
    std::function<void(Error::Ptr&&, std::optional<Table>&&)> callback) noexcept
{
    auto sysTableInfo = getSysTableInfo(tableName);
    if (sysTableInfo)
    {
        auto table = Table(this, std::make_shared<TableInfo>(*sysTableInfo), 0);
        callback(nullptr, std::move(table));

        return;
    }
    else
    {
        asyncOpenTable(
            SYS_TABLES, [this, callback = std::move(callback), tableName = std::string(tableName)](
                            Error::Ptr&& error, std::optional<Table>&& sysTable) {
                if (error)
                {
                    callback(std::move(error), {});
                    return;
                }

                sysTable->asyncGetRow(tableName,
                    [this, tableName, callback = std::move(callback)](auto&& error, auto&& entry) {
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

                        std::vector<std::string> fields;
                        boost::split(
                            fields, entry->getField(SYS_TABLE_VALUE_FIELDS), boost::is_any_of(","));

                        auto tableInfo = std::make_shared<storage::TableInfo>(
                            std::move(tableName), std::move(fields));
                        auto table = Table(this, tableInfo, 0);

                        callback(nullptr, std::move(table));
                    });
            });
    }
}