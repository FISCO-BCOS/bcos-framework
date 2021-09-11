#pragma once

#include <memory>
#include <utility>

namespace bcos
{
template <class T>
class ConcurrentCOW
{
public:
    ConcurrentCOW(){};
    explicit ConcurrentCOW(T&& obj) { reset(std::forward<T>(obj)); };
    explicit ConcurrentCOW(std::shared_ptr<T>&& obj) noexcept { m_obj = std::move(obj); }
    ConcurrentCOW(const ConcurrentCOW& cow) noexcept { m_obj = cow.m_obj; }
    ConcurrentCOW(ConcurrentCOW&& cow) noexcept { m_obj = std::move(cow.m_obj); }
    void operator=(const ConcurrentCOW& cow) noexcept { m_obj = cow.m_obj; }
    void operator=(ConcurrentCOW&& cow) noexcept { m_obj = std::move(cow.m_obj); }
    ~ConcurrentCOW() noexcept {}

    void reset(T&& obj)
    {
        if (!m_obj.unique())
        {
            m_obj = std::make_shared<T>(std::forward<T>(obj));
        }
        else
        {
            *m_obj = std::forward<T>(obj);
        }
    }

    void clear() noexcept { m_obj.reset(); }

    bool empty() const noexcept { return m_obj.get() == nullptr; }

    const T& operator*() const noexcept { return get(); }
    const T* operator->() const noexcept { return m_obj.get(); }

    const T* get() const noexcept { return m_obj.get(); }

    long refCount() const noexcept { return m_obj.use_count(); }

    T* mutableGet()
    {
        if (!m_obj.unique())
        {
            m_obj = std::make_shared<T>(*m_obj);
        }

        return m_obj.get();
    }

private:
    std::shared_ptr<T> m_obj;
};

}  // namespace bcos
