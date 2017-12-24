// Copyright (C) 2017 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <assert.h>
#include <vector>
#include <functional>
#include "Hash.h"

template <typename T1, typename T2>
class Dict
{
public:
    struct Entry
    {
        Entry*      next;
        uint32_t    hash;
        T1          key;
        T2          value;
    };
    typedef std::function<void(Entry*)> VisitProc;
public:
    Dict()
    {
        count_ = 0;
        Resize();
    }

    ~Dict()
    {
        Clear();
    }

    uint32_t Size() const
    {
        return count_;
    }

    T2 Lookup(const T1& key)
    {
        Entry* entry = *FindPointer(key, hash_value(key));
        if (entry != NULL)
        {
            return entry->value;
        }
        return T2();
    }

    Entry* InsertOrReplace(const T1& key, const T2& value)
    {
        uint32_t hash = hash_value(key);
        Entry** ptr = FindPointer(key, hash);
        Entry* old = *ptr;
        if (old != NULL)
        {
            old->value = value;
        }
        else
        {
            Entry* entry = new Entry();
            entry->hash = hash;
            entry->key = key;
            entry->value = value;
            entry->next = NULL;
            *ptr = entry;
            ++count_;
            if (count_ > buckets_.size())
            {
                Resize();
            }
        }
        return old;
    }

    void Remove(const T1& key)
    {
        Entry** ptr = FindPointer(key, hash_value(key));
        Entry* result = *ptr;
        if (result != NULL)
        {
            *ptr = result->next;
            --count_;
            delete result;
        }
    }

    void Visit(VisitProc func)
    {
        for (uint32_t i = 0; i < buckets_.size(); i++)
        {
            Entry* h = buckets_[i];
            while (h != NULL)
            {
                func(h);
                h = h->next;
            }
        }
    }

private:
    Entry** FindPointer(const T1& key, uint32_t hash)
    {
        assert(!buckets_.empty());
        Entry** ptr = &buckets_[hash & (buckets_.size() - 1)];
        while (*ptr != NULL && 
            ((*ptr)->hash != hash || (*ptr)->key != key))
        {
            ptr = &(*ptr)->next;
        }
        return ptr;
    }

    void Clear()
    {
        for (uint32_t i = 0; i < buckets_.size(); i++)
        {
            Entry* h = buckets_[i];
            while (h != NULL)
            {
                Entry* next = h->next;
                delete h;
                h = next;
            }
        }
    }

    void Resize()
    {
        uint32_t new_length = 8;
        while (new_length < count_)
        {
            new_length *= 2;
        }
        std::vector<Entry*> new_buckets;
        new_buckets.resize(new_length);
        uint32_t count = 0;
        for (uint32_t i = 0; i < buckets_.size(); i++)
        {
            Entry* h = buckets_[i];
            while (h != NULL)
            {
                Entry* next = h->next;
                Entry** ptr = &new_buckets[h->hash & (new_length-1)];
                h->next = *ptr;
                *ptr = h;
                h = next;
                count++;
            }
        }
        assert(count == count_);
        buckets_.swap(new_buckets);
    }

private:
    uint32_t    count_;
    std::vector<Entry*> buckets_;
};
