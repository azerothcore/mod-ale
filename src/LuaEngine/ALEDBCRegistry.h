#ifndef ALEDBCREGISTRY_H
#define ALEDBCREGISTRY_H

#include "ALEBind.h"
#include "DBCStores.h"

#include <functional>
#include <string>
#include <vector>

/*
 * Registry of DBC stores exposed to Lua through the global LookupEntry(name, id).
 *
 * Each entry knows how to look an id up in its store and how to wrap the found
 * record into a Lua value of the matching usertype. DBC records live for the
 * whole server uptime, so handing their address to Lua is safe.
 */
struct DBCDefinition
{
    std::string name;
    std::function<void const* (uint32)> lookupFunction;
    std::function<sol::object(sol::state_view, void const*)> makeObject;
};

extern std::vector<DBCDefinition> dbcRegistry;

#define REGISTER_DBC(dbcName, entryType, store)                             \
    {                                                                       \
        #dbcName,                                                           \
        [](uint32 id) -> void const*                                        \
        {                                                                   \
            return store.LookupEntry(id);                                   \
        },                                                                  \
        [](sol::state_view lua, void const* entry) -> sol::object          \
        {                                                                   \
            /* Setters exist on some DBC usertypes, so expose non-const. */ \
            return sol::make_object(lua,                                    \
                const_cast<entryType*>(static_cast<entryType const*>(entry))); \
        }                                                                   \
    }

#endif // ALEDBCREGISTRY_H
