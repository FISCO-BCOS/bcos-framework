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
    ConcurrentCOW(const ConcurrentCOW& cow) noexcept { m_obj = cow.m_obj; }
    ConcurrentCOW(ConcurrentCOW&& cow) noexcept { m_obj = std::move(cow.m_obj); }
    void operator=(const ConcurrentCOW& cow) noexcept { m_obj = cow.m_obj; }
    void operator=(ConcurrentCOW&& cow) noexcept { m_obj = std::move(cow.m_obj); }
    ~ConcurrentCOW() noexcept {}

    void reset(T&& obj) { m_obj = std::make_shared<T>(std::forward<T>(obj)); }

    bool empty() const noexcept { return m_obj.get() == nullptr; }

    const T& operator*() const noexcept { return get(); }
    const T* operator->() const noexcept { return m_obj.get(); }

    const T& get() const noexcept { return *m_obj; }

    long refCount() const noexcept { return m_obj.use_count(); }

    T& mutableGet()
    {
        if (refCount() > 1)
        {
            std::shared_ptr<T> obj = std::make_shared<T>(*m_obj);
            m_obj = obj;
        }

        return *m_obj;
    }

private:
    std::shared_ptr<T> m_obj;
};

}  // namespace bcos
