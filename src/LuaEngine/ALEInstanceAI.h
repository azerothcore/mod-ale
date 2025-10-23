/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_INSTANCE_DATA_H
#define _ALE_INSTANCE_DATA_H

#include "LuaEngine.h"
#include "InstanceScript.h"

/*
 * This class is a small wrapper around `InstanceData`,
 *   allowing instances to be scripted with ALE.
 *
 *
 * Note 1
 * ======
 *
 * Instances of `ALEInstanceAI` are owned by the core, so they
 *   are not deleted when ALE is reloaded. Thus `Load` is only called
 *   by the core once, no matter how many times ALE is reloaded.
 *
 * However, when ALE reloads, all instance data in ALE is lost.
 * So the solution is as follows:
 *
 *   1. Store the last save data in the member var `lastSaveData`.
 *
 *      At first this is just the data given to us by the core when it calls `Load`,
 *        but later on once we start saving new data this is from ALE.
 *
 *   2. When retrieving instance data from ALE, check if it's missing.
 *
 *      The data will be missing if ALE is reloaded, since a new Lua state is created.
 *
 *   3. If it *is* missing, call `Reload`.
 *
 *      This reloads the last known instance save data into ALE, and calls the appropriate hooks.
 *
 *
 * Note 2
 * ======
 *
 * CMaNGOS expects some of these methods to be `const`. However, any of these
 *   methods are free to call `Save`, resulting in mutation of `lastSaveData`.
 *
 * Therefore, none of the hooks are `const`-safe, and `const_cast` is used
 *   to escape from these restrictions.
 */
class ALEInstanceAI : public InstanceData
{
private:
    // The last save data to pass through this class,
    //   either through `Load` or `Save`.
    std::string lastSaveData;

public:
    ALEInstanceAI(Map* map) : InstanceData(map)
    {
    }

    void Initialize() override;

    /*
     * These are responsible for serializing/deserializing the instance's
     *   data table to/from the core.
     */
    void Load(const char* data) override;
    // Simply calls Save, since the functions are a bit different in name and data types on different cores
    std::string GetSaveData() override
    {
        return Save();
    }
    const char* Save() const;


    /*
     * Calls `Load` with the last save data that was passed to
     * or from ALE.
     *
     * See: big documentation blurb at the top of this class.
     */
    void Reload()
    {
        Load(NULL);
    }

    /*
     * These methods allow non-Lua scripts (e.g. DB, C++) to get/set instance data.
     */
    uint32 GetData(uint32 key) const override;
    void SetData(uint32 key, uint32 value) override;

    uint64 GetData64(uint32 key) const override;
    void SetData64(uint32 key, uint64 value) override;

    /*
     * These methods are just thin wrappers around ALE.
     */
    void Update(uint32 diff) override
    {
        // If ALE is reloaded, it will be missing our instance data.
        // Reload here instead of waiting for the next hook call (possibly never).
        // This avoids having to have an empty Update hook handler just to trigger the reload.
        if (!sALE->HasInstanceData(instance))
            Reload();

        sALE->OnUpdateInstance(this, diff);
    }

    bool IsEncounterInProgress() const override
    {
        return sALE->OnCheckEncounterInProgress(const_cast<ALEInstanceAI*>(this));
    }

    void OnPlayerEnter(Player* player) override
    {
        sALE->OnPlayerEnterInstance(this, player);
    }

    void OnGameObjectCreate(GameObject* gameobject) override
    {
        sALE->OnGameObjectCreate(this, gameobject);
    }

    void OnCreatureCreate(Creature* creature) override
    {
        sALE->OnCreatureCreate(this, creature);
    }
};

#endif // _ALE_INSTANCE_DATA_H
