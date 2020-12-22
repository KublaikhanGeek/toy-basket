/******************************************************************************
 * File name     : WeakCallback.h
 * Description   :
 * Version       : v1.0
 * Create Time   : 2019/8/29
 * Author        : andy
 * Modify history:
 *******************************************************************************
 * Modify Time   Modify person  Modification
 * ------------------------------------------------------------------------------
 *
 *******************************************************************************/

#ifndef _WEAKCALLBACK_H
#define _WEAKCALLBACK_H

#include <functional>
#include <memory>

namespace toyBasket
{

// A barely usable WeakCallback

template <typename CLASS, typename... ARGS>
class WeakCallback
{
public:
    WeakCallback(const std::weak_ptr<CLASS>& object, const std::function<void(CLASS*, ARGS...)>& function)
        : object_(object)
        , function_(function)
    {
    }

    // Default dtor, copy ctor and assignment are okay

    void operator()(ARGS&&... args) const
    {
        std::shared_ptr<CLASS> ptr(object_.lock());
        if (ptr)
        {
            function_(ptr.get(), std::forward<ARGS>(args)...);
        }
        // else
        // {
        //   LOG_TRACE << "expired";
        // }
    }

private:
    std::weak_ptr<CLASS> object_;
    std::function<void(CLASS*, ARGS...)> function_;
};

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object, void (CLASS::*function)(ARGS...))
{
    return WeakCallback<CLASS, ARGS...>(object, function);
}

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...) const)
{
    return WeakCallback<CLASS, ARGS...>(object, function);
}

} // namespace toyBasket

#endif // _WEAKCALLBACK_H
