#include "StateStorage.h"
#include "../libutilities/Error.h"
#include <tbb/parallel_sort.h>
#include <oneapi/tbb/parallel_for_each.h>
#include <tbb/spin_mutex.h>
#include <boost/algorithm/hex.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/crc.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <cctype>
#include <iomanip>
#include <ios>
#include <iterator>
#include <optional>

using namespace bcos;
using namespace bcos::storage;

void StateStorage::asyncGetPrimaryKeys(std::string_view table,
    const std::optional<Condition const>& _condition,
    std::function<void(Error::UniquePtr, std::vector<std::string>)> _callback)
{
    std::map<std::string_view, storage::Entry::Status> localKeys;

    if (m_enableTraverse)
    {
        for (auto& it : m_data)
        {
            if (it.first.table() == table)
            {
                if (!_condition || _condition->isValid(it.first.key()))
                {
                    localKeys.emplace(it.first.key(), it.second.status());
                }
            }
        }
    }

    std::shared_ptr<StorageInterface> prev = getPrev();
    if (!prev)
    {
        std::vector<std::string> resultKeys;
        for (auto& localIt : localKeys)
        {
            if (localIt.second == Entry::NORMAL)
            {
                resultKeys.push_back(std::string(localIt.first));
            }
        }

        _callback(nullptr, std::move(resultKeys));
        return;
    }

    prev->asyncGetPrimaryKeys(table, _condition,
        [localKeys = std::move(localKeys), callback = std::move(_callback)](
            auto&& error, auto&& remoteKeys) mutable {
            if (error)
            {
                callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(
                             StorageError::ReadError, "Get primary keys from prev failed!", *error),
                    std::vector<std::string>());
                return;
            }

            for (auto it = remoteKeys.begin(); it != remoteKeys.end();)
            {
                bool deleted = false;

                auto localIt = localKeys.find(*it);
                if (localIt != localKeys.end())
                {
                    if (localIt->second != Entry::NORMAL)
                    {
                        it = remoteKeys.erase(it);
                        deleted = true;
                    }

                    localKeys.erase(localIt);
                }

                if (!deleted)
                {
                    ++it;
                }
            }

            for (auto& localIt : localKeys)
            {
                if (localIt.second == Entry::NORMAL)
                {
                    remoteKeys.push_back(std::string(localIt.first));
                }
            }

            callback(nullptr, std::move(remoteKeys));
        });
}

void StateStorage::asyncGetRow(std::string_view tableView, std::string_view keyView,
    std::function<void(Error::UniquePtr, std::optional<Entry>)> _callback)
{
    decltype(m_data)::const_accessor entryIt;
    if (m_data.find(entryIt, EntryKey(tableView, keyView)))
    {
        auto& entry = entryIt->second;

        if (entry.status() != Entry::NORMAL)
        {
            entryIt.release();

            STORAGE_REPORT_GET(tableView, keyView, std::nullopt, "DELETED");
            _callback(nullptr, std::nullopt);
        }
        else
        {
            auto optionalEntry = std::make_optional(entry);
            entryIt.release();

            STORAGE_REPORT_GET(tableView, keyView, optionalEntry, "FOUND");
            _callback(nullptr, std::move(optionalEntry));
        }
        return;
    }
    else
    {
        STORAGE_REPORT_GET(tableView, keyView, std::nullopt, "NO ENTRY");
    }

    auto prev = getPrev();
    if (prev)
    {
        prev->asyncGetRow(tableView, keyView,
            [this, prev, table = std::string(tableView), key = std::string(keyView), _callback](
                Error::UniquePtr error, std::optional<Entry> entry) {
                if (error)
                {
                    _callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(
                                  StorageError::ReadError, "Get row from storage failed!", *error),
                        {});
                    return;
                }

                if (entry)
                {
                    STORAGE_REPORT_GET(table, key, entry, "PREV FOUND");
                    _callback(nullptr,
                        std::make_optional(importExistingEntry(table, key, std::move(*entry))));
                }
                else
                {
                    STORAGE_REPORT_GET(table, key, std::nullopt, "PREV NOT FOUND");
                    _callback(nullptr, std::nullopt);
                }
            });
    }
    else
    {
        _callback(nullptr, std::nullopt);
    }
}

void StateStorage::asyncGetRows(std::string_view tableView,
    const std::variant<const gsl::span<std::string_view const>, const gsl::span<std::string const>>&
        _keys,
    std::function<void(Error::UniquePtr, std::vector<std::optional<Entry>>)> _callback)
{
    std::visit(
        [this, &tableView, &_callback](auto&& _keys) {
            std::vector<std::optional<Entry>> results(_keys.size());
            auto missinges = std::tuple<std::vector<std::string_view>,
                std::vector<std::tuple<std::string, size_t>>>();

            long existsCount = 0;

            size_t i = 0;
            for (auto& key : _keys)
            {
                decltype(m_data)::const_accessor entryIt;
                std::string_view keyView(key);
                if (m_data.find(entryIt, EntryKey(tableView, keyView)))
                {
                    auto& entry = entryIt->second;
                    if (entry.status() == Entry::NORMAL)
                    {
                        results[i].emplace(entry);
                    }
                    else
                    {
                        results[i] = std::nullopt;
                    }
                    ++existsCount;
                }
                else
                {
                    std::get<1>(missinges).emplace_back(std::string(key), i);
                    std::get<0>(missinges).emplace_back(key);
                }

                ++i;
            }

            auto prev = getPrev();
            if (existsCount < _keys.size() && prev)
            {
                prev->asyncGetRows(tableView, std::get<0>(missinges),
                    [this, table = std::string(tableView), callback = std::move(_callback),
                        missingIndexes = std::move(std::get<1>(missinges)),
                        results = std::move(results)](
                        auto&& error, std::vector<std::optional<Entry>>&& entries) mutable {
                        if (error)
                        {
                            callback(BCOS_ERROR_WITH_PREV_UNIQUE_PTR(StorageError::ReadError,
                                         "async get perv rows failed!", *error),
                                std::vector<std::optional<Entry>>());
                            return;
                        }

                        for (size_t i = 0; i < entries.size(); ++i)
                        {
                            auto& entry = entries[i];

                            if (entry)
                            {
                                results[std::get<1>(missingIndexes[i])].emplace(importExistingEntry(
                                    table, std::move(std::get<0>(missingIndexes[i])),
                                    std::move(*entry)));
                            }
                        }

                        callback(nullptr, std::move(results));
                    });
            }
            else
            {
                _callback(nullptr, std::move(results));
            }
        },
        _keys);
}

void StateStorage::asyncSetRow(std::string_view tableNameView, std::string_view keyView,
    Entry entry, std::function<void(Error::UniquePtr)> callback)
{
    auto updatedCapacity = entry.capacityOfHashField();
    std::optional<Entry> entryOld;

    decltype(m_data)::accessor entryIt;
    if (m_data.find(entryIt, EntryKey(tableNameView, keyView)))
    {
        auto& existsEntry = entryIt->second;
        entryOld.emplace(std::move(existsEntry));
        entryIt->second = std::move(entry);

        updatedCapacity -= entryOld->capacityOfHashField();

        if (entryIt->second.status() == Entry::PURGED)
        {
            m_data.erase(entryIt);
            STORAGE_REPORT_SET(tableNameView, keyView, std::nullopt, "PURGED");
        }
        else
        {
            STORAGE_REPORT_SET(tableNameView, keyView, entryIt->second, "UPDATE");
        }
        entryIt.release();
    }
    else
    {
        decltype(m_data)::const_accessor constEntryIt;
        if (m_data.emplace(constEntryIt, EntryKey(std::string(tableNameView), std::string(keyView)),
                std::move(entry)))
        {
            STORAGE_REPORT_SET(constEntryIt->first.table(), constEntryIt->first.key(),
                constEntryIt->second, "INSERT");
        }
        else
        {
            STORAGE_LOG(WARNING) << "Set row failed because row exists";

            STORAGE_REPORT_SET(constEntryIt->first.table(), constEntryIt->first.key(),
                constEntryIt->second, "EXISTS");
        }
    }


    if (m_recoder.local())
    {
        m_recoder.local()->log(
            Recoder::Change(std::string(tableNameView), std::string(keyView), std::move(entryOld)));
    }

    m_capacity += updatedCapacity;

    callback(nullptr);
}

void StateStorage::parallelTraverse(bool onlyDirty,
    std::function<bool(
        const std::string_view& table, const std::string_view& key, const Entry& entry)>
        callback) const
{
    oneapi::tbb::parallel_for_each(m_data.begin(), m_data.end(), [&](const std::pair<const EntryKey, Entry>& it) {
        auto& entry = it.second;
        if (!onlyDirty || entry.dirty())
        {
            callback(it.first.table(), it.first.key(), entry);
        }
    });
}

std::optional<Table> StateStorage::openTable(const std::string_view& tableName)
{
    std::promise<std::tuple<Error::UniquePtr, std::optional<Table>>> openPromise;
    asyncOpenTable(tableName, [&](auto&& error, auto&& table) {
        openPromise.set_value({std::move(error), std::move(table)});
    });

    auto [error, table] = openPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(*error);
    }

    return table;
}

std::optional<Table> StateStorage::createTable(std::string _tableName, std::string _valueFields)
{
    std::promise<std::tuple<Error::UniquePtr, std::optional<Table>>> createPromise;
    asyncCreateTable(
        _tableName, _valueFields, [&](Error::UniquePtr&& error, std::optional<Table>&& table) {
            createPromise.set_value({std::move(error), std::move(table)});
        });
    auto [error, table] = createPromise.get_future().get();
    if (error)
    {
        BOOST_THROW_EXCEPTION(*error);
    }

    return table;
}

crypto::HashType StateStorage::hash(const bcos::crypto::Hash::Ptr& hashImpl)
{
    bcos::crypto::HashType totalHash;

    tbb::spin_mutex mutex;
    tbb::parallel_for(m_data.range(),
        [&hashImpl, &mutex, &totalHash](decltype(m_data)::range_type& range) {
            for (auto& it : range)
            {
                auto& entry = it.second;
                if (entry.dirty())
                {
                    bytes data;
                    data.reserve(entry.capacityOfHashField() + entry.fieldCount());

                    for (auto field : entry)
                    {
                        data.insert(data.end(), field.begin(), field.end());
                        data.insert(data.end(), (bcos::byte)entry.status());
                    }

                    auto result = hashImpl->hash(data);

                    tbb::spin_mutex::scoped_lock lock(mutex);
                    totalHash ^= result;
                }
            }
        });

    return totalHash;
}

void StateStorage::rollback(const Recoder& recoder)
{
    for (auto& change : recoder)
    {
        if (change.entry)
        {
            decltype(m_data)::accessor entryIt;
            if (m_data.find(entryIt, EntryKey(change.table, std::string_view(change.key))))
            {
                entryIt->second = std::move(*(change.entry));
            }
            else
            {
                m_data.emplace(entryIt,
                    EntryKey(std::string(change.table), std::string(change.key)),
                    std::move(*(change.entry)));
            }
        }
        else
        {  // nullopt means the key is not exist in m_cache
            decltype(m_data)::const_accessor entryIt;
            if (m_data.find(entryIt, EntryKey(change.table, std::string_view(change.key))))
            {
                m_data.erase(entryIt);
            }
            else
            {
                auto message =
                    (boost::format("Not found rollback entry: %s:%s") % change.table % change.key)
                        .str();

                BOOST_THROW_EXCEPTION(BCOS_ERROR(StorageError::UnknownError, message));
            }
        }
    }
}

Entry StateStorage::importExistingEntry(std::string_view table, std::string_view key, Entry entry)
{
    entry.setDirty(false);

    if (!m_cachePrev)
    {
        return entry;
    }

    decltype(m_data)::const_accessor entryIt;
    if (m_data.find(entryIt, EntryKey(table, key)))
    {
        STORAGE_REPORT_SET(
            entryIt->first.table(), key, std::make_optional(entry), "IMPORT REJECTED");
    }
    else
    {
        if (!m_data.emplace(
                entryIt, EntryKey(std::string(table), std::string(key)), std::move(entry)))
        {
            if (table != StorageInterface::SYS_TABLES)
            {
                STORAGE_REPORT_SET(
                    entryIt->first.table(), key, entryIt->second, "IMPORT EXISTS FAILED");

                auto fmt = boost::format("Import existsing entry failed! Table: %s, key: %s") %
                           entryIt->first.table() % key;
                STORAGE_LOG(ERROR) << fmt;
                BOOST_THROW_EXCEPTION(BCOS_ERROR(StorageError::UnknownError, fmt.str()));
            }
            STORAGE_REPORT_SET(entryIt->first.table(), key, entryIt->second, "IMPORT SYS_TABLES");
        }
        STORAGE_REPORT_SET(
            entryIt->first.table(), key, std::make_optional(entryIt->second), "IMPORT");
    }

    return entryIt->second;
}
