/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _BINDING_MAP_H
#define _BINDING_MAP_H

#include "Common.h"
#include "ObjectGuid.h"

#include <sol/sol.hpp>

#include <mutex>
#include <unordered_map>
#include <vector>

/*
 * A set of Lua event handlers registered against keys of type `K`.
 *
 * Scripts register handlers with RegisterPlayerEvent & co; hooks fetch them
 * back by key when the matching game event fires. Handlers are stored as
 * sol::protected_function, so their lifetime in the Lua registry is managed
 * automatically.
 */
template<typename K>
class BindingMap
{
private:
    struct Binding
    {
        uint64 id;
        // Number of times the handler still fires before expiring; 0 means never expires.
        uint32 remainingShots;
        sol::protected_function callback;
    };

    std::unordered_map<K, std::vector<Binding>> bindings;
    // Maps a binding id back to its key, so Remove doesn't have to scan every list.
    std::unordered_map<uint64, K> keysById;
    uint64 maxBindingID = 0;
    std::mutex mutex;

public:
    /*
     * Inserts a new handler for `key` that fires `shots` times.
     *
     * If `shots` is 0 the handler never expires on its own, but can still be
     * removed with `Clear` or `Remove`. Returns an id usable with `Remove`.
     */
    uint64 Insert(K const& key, sol::protected_function callback, uint32 shots)
    {
        std::lock_guard<std::mutex> guard(mutex);

        uint64 id = ++maxBindingID;
        bindings[key].push_back({ id, shots, std::move(callback) });
        keysById.emplace(id, key);
        return id;
    }

    // Removes all handlers for `key`.
    void Clear(K const& key)
    {
        std::lock_guard<std::mutex> guard(mutex);

        auto iter = bindings.find(key);
        if (iter == bindings.end())
            return;

        for (Binding const& binding : iter->second)
            keysById.erase(binding.id);

        bindings.erase(iter);
    }

    // Removes all handlers for all keys.
    void Clear()
    {
        std::lock_guard<std::mutex> guard(mutex);

        keysById.clear();
        bindings.clear();
    }

    // Removes the single handler identified by `id`, if it still exists.
    void Remove(uint64 id)
    {
        std::lock_guard<std::mutex> guard(mutex);

        auto keyIter = keysById.find(id);
        if (keyIter == keysById.end())
            return;

        auto listIter = bindings.find(keyIter->second);
        if (listIter != bindings.end())
        {
            std::vector<Binding>& list = listIter->second;
            std::erase_if(list, [id](Binding const& binding) { return binding.id == id; });
            if (list.empty())
                bindings.erase(listIter);
        }

        keysById.erase(keyIter);
    }

    // Returns whether `key` has any handlers.
    bool HasBindingsFor(K const& key)
    {
        std::lock_guard<std::mutex> guard(mutex);

        auto iter = bindings.find(key);
        return iter != bindings.end() && !iter->second.empty();
    }

    /*
     * Returns a snapshot of the handlers registered for `key`, consuming one
     * shot from each.
     *
     * Dispatch iterates the snapshot, so a handler that registers or removes
     * handlers while running cannot corrupt the iteration.
     */
    std::vector<sol::protected_function> GetCallbacksFor(K const& key)
    {
        std::lock_guard<std::mutex> guard(mutex);

        std::vector<sol::protected_function> callbacks;

        auto iter = bindings.find(key);
        if (iter == bindings.end())
            return callbacks;

        std::vector<Binding>& list = iter->second;
        callbacks.reserve(list.size());

        for (Binding& binding : list)
            callbacks.push_back(binding.callback);

        // Expire handlers that just used up their last shot.
        std::erase_if(list, [this](Binding& binding)
        {
            if (binding.remainingShots == 0)
                return false;

            --binding.remainingShots;
            if (binding.remainingShots > 0)
                return false;

            keysById.erase(binding.id);
            return true;
        });

        if (list.empty())
            bindings.erase(iter);

        return callbacks;
    }
};

/*
 * A `BindingMap` key for global event bindings (ServerEvents, GuildEvents, ...).
 */
template<typename T>
struct EventKey
{
    T event_id;

    EventKey(T event_id) :
        event_id(event_id)
    {
    }

    bool operator==(EventKey const& other) const
    {
        return event_id == other.event_id;
    }
};

/*
 * A `BindingMap` key for event ID + entry bindings (CreatureEvents, GameObjectEvents, ...).
 */
template<typename T>
struct EntryKey
{
    T event_id;
    uint32 entry;

    EntryKey(T event_id, uint32 entry) :
        event_id(event_id),
        entry(entry)
    {
    }

    bool operator==(EntryKey const& other) const
    {
        return event_id == other.event_id && entry == other.entry;
    }
};

/*
 * A `BindingMap` key for event ID + specific object bindings
 * (currently just CreatureEvents on one spawned creature).
 */
template<typename T>
struct UniqueObjectKey
{
    T event_id;
    ObjectGuid guid;
    uint32 instance_id;

    UniqueObjectKey(T event_id, ObjectGuid guid, uint32 instance_id) :
        event_id(event_id),
        guid(guid),
        instance_id(instance_id)
    {
    }

    bool operator==(UniqueObjectKey const& other) const
    {
        return event_id == other.event_id && guid == other.guid && instance_id == other.instance_id;
    }
};

namespace ALEKeyHash
{
    inline void Combine(std::size_t& seed, std::size_t value)
    {
        // from boost::hash_combine
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template<typename T>
    std::size_t HashEnum(T value)
    {
        return std::hash<std::underlying_type_t<T>>()(static_cast<std::underlying_type_t<T>>(value));
    }
}

/*
 * std::hash implementations so the key types can be used in unordered_map.
 */
namespace std
{
    template<typename T>
    struct hash<EventKey<T>>
    {
        std::size_t operator()(EventKey<T> const& key) const
        {
            return ALEKeyHash::HashEnum(key.event_id);
        }
    };

    template<typename T>
    struct hash<EntryKey<T>>
    {
        std::size_t operator()(EntryKey<T> const& key) const
        {
            std::size_t seed = ALEKeyHash::HashEnum(key.event_id);
            ALEKeyHash::Combine(seed, std::hash<uint32>()(key.entry));
            return seed;
        }
    };

    template<typename T>
    struct hash<UniqueObjectKey<T>>
    {
        std::size_t operator()(UniqueObjectKey<T> const& key) const
        {
            std::size_t seed = ALEKeyHash::HashEnum(key.event_id);
            ALEKeyHash::Combine(seed, std::hash<uint32>()(key.instance_id));
            ALEKeyHash::Combine(seed, std::hash<uint64>()(key.guid.GetRawValue()));
            return seed;
        }
    };
}

#endif // _BINDING_MAP_H
