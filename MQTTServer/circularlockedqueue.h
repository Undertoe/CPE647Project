#ifndef CIRCULARLOCKEDQUEUE_H
#define CIRCULARLOCKEDQUEUE_H

#include <deque>
#include <optional>

#include "semaphorefix.h"

template<typename Type>
struct CircularLockedQueue
{
    std::deque<Type> data;
    SemaFix sema;

    CircularLockedQueue()
    {
        sema.Init(0);
    }

    void AddData(const Type &d)
    {
        data.push_back(d);
        sema.Post();
    }

    void AddData(Type &&d)
    {
        data.emplace_back(d);
        sema.Post();
    }

    std::optional<Type> PopData()
    {
        sema.Wait();
        if(data.size() > 0)
        {
            Type newData = data.front();
            data.pop_front();
            return newData;
        }
        return std::nullopt;
    }

};

#endif // CIRCULARLOCKEDQUEUE_H
