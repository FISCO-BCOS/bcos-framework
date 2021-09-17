#include "../interfaces/storage/StorageInterface.h"
#include "../interfaces/storage/Table.h"
#include <boost/range/detail/implementation_help.hpp>
#include <optional>

using namespace bcos::storage;

TableInfo::ConstPtr StorageInterface::getSysTableInfo(const std::string_view& tableName)
{
    struct SystemTables
    {
        SystemTables()
        {
            tables.push_back(std::make_shared<TableInfo>(
                std::string(SYS_TABLES), std::vector<std::string>{SYS_TABLE_VALUE_FIELDS}));
        }

        std::vector<TableInfo::ConstPtr> tables;
    } static m_systemTables;

    if (tableName == SYS_TABLES)
    {
        return m_systemTables.tables[0];
    }

    return nullptr;
}

void StorageInterface::asyncCreateTable(std::string _tableName, std::string _valueFields,
    std::function<void(Error::UniquePtr&&, std::optional<Table>&&)> callback) noexcept
{
    asyncOpenTable(SYS_TABLES, [this, tableName = std::move(_tableName),
                                   callback = std::move(callback),
                                   valueFields = std::move(_valueFields)](
                                   auto&& error, auto&& sysTable) {
        if (error)
        {
            callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(-1, "Open sys_tables failed!", *error), {});
            return;
        }

        sysTable->asyncGetRow(tableName, [this, tableName, callback, &sysTable,
                                             valueFields = std::move(valueFields)](
                                             auto&& error, auto&& entry) {
            if (error)
            {
                callback(
                    BCOS_ERROR_WITH_PREV_UNIQUE_PTR(-1, "Get table info row failed!", *error), {});
                return;
            }

            if (entry)
            {
                callback(nullptr, {});
                return;
            }

            auto tableEntry = sysTable->newEntry();
            tableEntry.setField(SYS_TABLE_VALUE_FIELDS, std::string(valueFields));

            sysTable->asyncSetRow(tableName, tableEntry,
                [this, callback, tableName, valueFields = std::move(valueFields)](auto&& error) {
                    if (error)
                    {
                        callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(
                                     -1, "Put table info into sys_tables failed!", *error),
                            {});
                        return;
                    }

                    std::vector<std::string> fields;
                    boost::split(fields, valueFields, boost::is_any_of(","));

                    auto tableInfo = std::make_shared<storage::TableInfo>(
                        std::move(tableName), std::move(fields));
                    auto table = Table(this, tableInfo);

                    callback(nullptr, std::make_optional(std::move(table)));
                });
        });
    });
}

void StorageInterface::asyncOpenTable(std::string_view tableName,
    std::function<void(Error::UniquePtr&&, std::optional<Table>&&)> callback) noexcept
{
    auto sysTableInfo = getSysTableInfo(tableName);
    if (sysTableInfo)
    {
        auto table = Table(this, sysTableInfo);
        callback(nullptr, std::move(table));

        return;
    }
    else
    {
        asyncOpenTable(
            SYS_TABLES, [this, callback = std::move(callback), tableName = std::string(tableName)](
                            Error::UniquePtr&& error, std::optional<Table>&& sysTable) {
                if (error)
                {
                    callback(std::move(error), {});
                    return;
                }

                if (!sysTable)
                {
                    callback(BCOS_ERROR_UNIQUE_PTR(
                                 -1, "System table: " + std::string(SYS_TABLES) + " not found!"),
                        {});
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
                        auto table = Table(this, tableInfo);

                        callback(nullptr, std::make_optional(std::move(table)));
                    });
            });
    }
}