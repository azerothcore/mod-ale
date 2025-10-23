/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef LOOTMETHODS_H
#define LOOTMETHODS_H

/***
 * Represents loot that can be obtained from various sources like creatures, gameobjects, or items.
 * 
 * Contains information about items that can be looted, their quantities, money, and loot state.
 *
 * Inherits all methods from: none
 */
namespace LuaLoot
{
    /**
     * Returns `true` if all loot has been taken from this [Loot], returns `false` otherwise.
     *
     * @return bool isLooted
     */
    int IsLooted(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->isLooted());
        return 1;
    }

    /**
     * Adds an item to the [Loot] with the specified parameters.
     *
     * If an item with the same ID already exists and its count is less than 255, the count will be increased instead of adding a new entry.
     *
     * @param uint32 itemId : the ID of the item to add
     * @param uint8 minCount : minimum count of the item
     * @param uint8 maxCount : maximum count of the item
     * @param float chance : chance for the item to drop (0-100)
     * @param uint16 lootMode : loot mode for the item
     * @param bool needsQuest = false : if `true`, the item requires a quest to be looted
     * @param bool allowStacking = true : if `true`, allow items to stack in the loot window
     */
    int AddItem(lua_State* L, Loot* loot)
    {
        uint32 itemid = ALE::CHECKVAL<uint32>(L, 2);
        uint8 min_count = ALE::CHECKVAL<uint8>(L, 3);
        uint8 max_count = ALE::CHECKVAL<uint8>(L, 4);
        float chance = ALE::CHECKVAL<float>(L, 5);
        uint16 loot_mode = ALE::CHECKVAL<uint16>(L, 6);
        bool needs_quest = ALE::CHECKVAL<bool>(L, 7, false);
        bool allow_stacking = ALE::CHECKVAL<bool>(L, 8, true);

        if (allow_stacking)
        {
            auto& container = needs_quest ? loot->quest_items : loot->items;

            for (LootItem& lootitem : container)
            {
                if (lootitem.itemid == itemid && lootitem.count < 255)
                {
                    uint32 add = std::max<uint32>(1u, min_count);
                    uint32 newCount = std::min<uint32>(255u, lootitem.count + add);
                    lootitem.count = static_cast<uint8>(newCount);
                    return 0;
                }
            }
        }

        LootStoreItem newLootStoreItem(itemid, 0, chance, needs_quest, loot_mode, 0, min_count, max_count);
        loot->AddItem(newLootStoreItem);

        return 0;
    }

    /**
     * Returns `true` if the [Loot] contains the specified item, and returns `false` otherwise.
     *
     * @param uint32 itemId = 0 : the ID of the item to check for. If 0, checks if any item exists
     * @param uint32 count = 0 : specific count to check for. If 0, ignores count
     * @return bool hasItem
     */
    int HasItem(lua_State* L, Loot* loot)
    {
        uint32 itemid = ALE::CHECKVAL<uint32>(L, 2, false);
        uint32 count = ALE::CHECKVAL<uint32>(L, 3, false);
        bool has_item = false;

        if (itemid)
        {
            for (const LootItem &lootitem : loot->items)
            {
                if (lootitem.itemid == itemid && (count == 0 || lootitem.count == count))
                {
                    has_item = true;
                    break;
                }
            }
        }
        else
        {
            for (const LootItem &lootitem : loot->items)
            {
                if (lootitem.itemid != 0)
                {
                    has_item = true;
                    break;
                }
            }
        }

        ALE::Push(L, has_item);
        return 1;
    }

    /**
     * Removes the specified item from the [Loot].
     *
     * If count is specified, removes only that amount. Otherwise removes all items with the ID.
     *
     * @param uint32 itemId : the ID of the item to remove
     * @param bool isCountSpecified = false : if `true`, only removes the specified count
     * @param uint32 count = 0 : amount to remove when isCountSpecified is true
     */
    int RemoveItem(lua_State* L, Loot* loot)
    {
        uint32 itemid = ALE::CHECKVAL<uint32>(L, 2);
        bool isCountSpecified = ALE::CHECKVAL<bool>(L, 3, false);
        uint32 count = isCountSpecified ? ALE::CHECKVAL<uint32>(L, 4) : 0;

        auto removeFromContainer = [&](auto& container, uint32& remaining)
        {
            for (auto it = container.begin(); it != container.end(); )
            {
                if (it->itemid == itemid)
                {
                    if (isCountSpecified)
                    {
                        if (it->count > remaining)
                        {
                            it->count -= static_cast<uint8>(remaining);
                            remaining = 0;
                            break;
                        }
                        else
                        {
                            remaining -= it->count;
                            it = container.erase(it);
                            if (remaining == 0)
                                break;
                            continue;
                        }
                    }
                    else
                    {
                        it = container.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        };

        // Remove from regular items
        removeFromContainer(loot->items, count);

        // Remove from quest items as well
        if (!isCountSpecified || count > 0)
            removeFromContainer(loot->quest_items, count);

        return 0;
    }

    /**
     * Returns the amount of money in this [Loot].
     *
     * @return uint32 money : the amount of money in copper
     */
    int GetMoney(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->gold);
        return 1;
    }

    /**
     * Sets the amount of money in this [Loot].
     *
     * @param uint32 money : the amount of money to set in copper
     */
    int SetMoney(lua_State* L, Loot* loot)
    {
        uint32 gold = ALE::CHECKVAL<uint32>(L, 2);

        loot->gold = gold;
        return 0;
    }

    /**
     * Generates a random amount of money for this [Loot] within the specified range.
     *
     * @param uint32 minGold : minimum amount of money in copper
     * @param uint32 maxGold : maximum amount of money in copper
     */
    int GenerateMoney(lua_State* L, Loot* loot)
    {
        uint32 min_gold = ALE::CHECKVAL<uint32>(L, 2);
        uint32 max_gold = ALE::CHECKVAL<uint32>(L, 3);

        loot->generateMoneyLoot(min_gold, max_gold);
        return 0;
    }

    /**
     * Clears all items and money from this [Loot].
     */
    int Clear(lua_State* /*L*/, Loot* loot)
    {
        loot->clear();
        return 0;
    }

    /**
     * Sets the number of unlooted items in this [Loot].
     *
     * @param uint32 count : the number of unlooted items
     */
    int SetUnlootedCount(lua_State* L, Loot* loot)
    {
        uint32 count = ALE::CHECKVAL<uint32>(L, 2);

        loot->unlootedCount = count;
        return 0;
    }

    /**
     * Returns the number of unlooted items in this [Loot].
     *
     * @return uint32 unlootedCount
     */
    int GetUnlootedCount(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->unlootedCount);
        return 1;
    }

    /**
     * Returns a table containing all items in this [Loot].
     *
     * Each item is represented as a table with the following fields:
     *   - id: item ID
     *   - index: item index in the loot list
     *   - count: quantity of the item
     *   - needs_quest: whether the item requires a quest
     *   - is_looted: whether the item has already been looted
     *   - roll_winner_guid: GUID of the player who won the item roll
     *
     * @return table items : array of item tables
     */
    int GetItems(lua_State* L, Loot* loot)
    {
        lua_createtable(L, loot->items.size(), 0);
        int tbl = lua_gettop(L);

        for (unsigned int i = 0; i < loot->items.size(); i++)
        {
            lua_newtable(L);

            ALE::Push(L, loot->items[i].itemid);
            lua_setfield(L, -2, "id");

            ALE::Push(L, loot->items[i].itemIndex);
            lua_setfield(L, -2, "index");

            ALE::Push(L, loot->items[i].count);
            lua_setfield(L, -2, "count");

            ALE::Push(L, loot->items[i].needs_quest);
            lua_setfield(L, -2, "needs_quest");

            ALE::Push(L, loot->items[i].is_looted);
            lua_setfield(L, -2, "is_looted");

            ALE::Push(L, loot->items[i].rollWinnerGUID);
            lua_setfield(L, -2, "roll_winner_guid");

            lua_rawseti(L, tbl, i + 1);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns a table containing all quest items in this [Loot].
     *
     * Each quest item is represented as a table with the following fields:
     *   - id: item ID
     *   - index: item index in the quest loot list
     *   - count: quantity of the item
     *   - needs_quest: whether the item requires a quest
     *   - is_looted: whether the item has already been looted
     *   - roll_winner_guid: GUID of the player who won the item roll
     *
     * @return table quest_items : array of quest item tables
     */
    int GetQuestItems(lua_State* L, Loot* loot)
    {
        lua_createtable(L, loot->quest_items.size(), 0);
        int tbl = lua_gettop(L);

        for (unsigned int i = 0; i < loot->quest_items.size(); i++)
        {
            lua_newtable(L);

            ALE::Push(L, loot->quest_items[i].itemid);
            lua_setfield(L, -2, "id");

            ALE::Push(L, loot->quest_items[i].itemIndex);
            lua_setfield(L, -2, "index");

            ALE::Push(L, loot->quest_items[i].count);
            lua_setfield(L, -2, "count");

            ALE::Push(L, loot->quest_items[i].needs_quest);
            lua_setfield(L, -2, "needs_quest");

            ALE::Push(L, loot->quest_items[i].is_looted);
            lua_setfield(L, -2, "is_looted");

            ALE::Push(L, loot->quest_items[i].rollWinnerGUID);
            lua_setfield(L, -2, "roll_winner_guid");

            lua_rawseti(L, tbl, i + 1);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Updates the index of all items in this [Loot] to match their position in the list.
     *
     * This should be called after removing items to ensure indices are sequential.
     */
    int UpdateItemIndex(lua_State* /*L*/, Loot* loot)
    {
        uint32 index = 0;

        for (unsigned int i = 0; i < loot->items.size(); ++i)
            loot->items[i].itemIndex = index++;

        for (unsigned int i = 0; i < loot->quest_items.size(); ++i)
            loot->quest_items[i].itemIndex = index++;

        return 0;
    }

    /**
     * Sets the looted status of a specific item in this [Loot].
     *
     * @param uint32 itemId : the ID of the item
     * @param uint32 count : specific count to match. If 0, ignores count
     * @param bool looted = true : `true` to mark as looted, `false` to mark as unlooted
     */
    int SetItemLooted(lua_State* L, Loot* loot)
    {
        uint32 itemid = ALE::CHECKVAL<uint32>(L, 2);
        uint32 count = ALE::CHECKVAL<uint32>(L, 3);
        bool looted = ALE::CHECKVAL<bool>(L, 4, true);

        for (auto &lootItem : loot->items)
        {
            if (lootItem.itemid == itemid && (count == 0 || lootItem.count == count))
            {
                lootItem.is_looted = looted;
                break;
            }
        }
        return 0;
    }

    /**
     * Returns `true` if the [Loot] is completely empty (no items and no money), returns `false` otherwise.
     *
     * @return bool isEmpty
     */
    int IsEmpty(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->empty());
        return 1;
    }

    /**
     * Returns the [Loot] type.
     *
     * @return [LootType] lootType
     */
    int GetLootType(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->loot_type);
        return 1;
    }

    /**
     * Sets the [Loot] type.
     *
     * <pre>
     * enum LootType
     * {
     *     LOOT_NONE                           = 0,
     *     LOOT_CORPSE                         = 1,
     *     LOOT_PICKPOCKETING                  = 2,
     *     LOOT_FISHING                        = 3,
     *     LOOT_DISENCHANTING                  = 4,
     *     LOOT_SKINNING                       = 6,
     *     LOOT_PROSPECTING                    = 7,
     *     LOOT_MILLING                        = 8,
     *     LOOT_FISHINGHOLE                    = 20,
     *     LOOT_INSIGNIA                       = 21,
     *     LOOT_FISHING_JUNK                   = 22
     * };
     * </pre>
     *
     * @param [LootType] lootType : the loot type to set
     */
    int SetLootType(lua_State* L, Loot* loot)
    {
        uint32 lootType = ALE::CHECKVAL<uint32>(L, 2);
        loot->loot_type = static_cast<LootType>(lootType);
        return 0;
    }

    /**
     * Returns the [Player] GUID that owns this loot for round robin distribution.
     *
     * @return ObjectGuid roundRobinPlayer : the player GUID
     */
    int GetRoundRobinPlayer(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->roundRobinPlayer);
        return 1;
    }

    /**
     * Sets the [Player] GUID for round robin loot distribution.
     *
     * @param ObjectGuid playerGUID : the player GUID
     */
    int SetRoundRobinPlayer(lua_State* L, Loot* loot)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        loot->roundRobinPlayer = guid;
        return 0;
    }

    /**
     * Returns the [Player] GUID that owns this loot.
     *
     * @return ObjectGuid lootOwner : the player GUID
     */
    int GetLootOwner(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->lootOwnerGUID);
        return 1;
    }

    /**
     * Sets the [Player] GUID that owns this loot.
     *
     * @param ObjectGuid playerGUID : the player GUID
     */
    int SetLootOwner(lua_State* L, Loot* loot)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        loot->lootOwnerGUID = guid;
        return 0;
    }

    /**
     * Returns the container GUID that holds this loot.
     *
     * @return ObjectGuid containerGUID : the container GUID
     */
    int GetContainer(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->containerGUID);
        return 1;
    }

    /**
     * Sets the container GUID that holds this loot.
     *
     * @param ObjectGuid containerGUID : the container GUID
     */
    int SetContainer(lua_State* L, Loot* loot)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        loot->containerGUID = guid;
        return 0;
    }

    /**
     * Returns the source [WorldObject] GUID for this loot.
     *
     * @return ObjectGuid sourceGUID : the source [WorldObject] GUID
     */
    int GetSourceWorldObject(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->sourceWorldObjectGUID);
        return 1;
    }

    /**
     * Sets the source [WorldObject] GUID for this loot.
     *
     * @param ObjectGuid sourceGUID : the source [WorldObject] GUID
     */
    int SetSourceWorldObject(lua_State* L, Loot* loot)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        loot->sourceWorldObjectGUID = guid;
        return 0;
    }

    /**
     * Returns `true` if the [Loot] contains quest items and returns `false` otherwise.
     *
     * @return bool hasQuestItems
     */
    int HasQuestItems(lua_State* L, Loot* loot)
    {
        ALE::Push(L, !loot->quest_items.empty());
        return 1;
    }

    /**
     * Returns `true` if the [Loot] has items available for all players and returns `false` otherwise.
     *
     * @return bool hasItemForAll
     */
    int HasItemForAll(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->hasItemForAll());
        return 1;
    }

    /**
     * Returns `true` if the [Loot] has items that are over the group loot threshold and returns `false` otherwise.
     *
     * @return bool hasOverThresholdItem
     */
    int HasOverThresholdItem(lua_State* L, Loot* loot)
    {
        ALE::Push(L, loot->hasOverThresholdItem());
        return 1;
    }

    /**
     * Returns the total number of items (regular + quest items) in this [Loot].
     *
     * @return uint32 itemCount
     */
    int GetItemCount(lua_State* L, Loot* loot)
    {
        ALE::Push(L, static_cast<uint32>(loot->items.size() + loot->quest_items.size()));
        return 1;
    }

    /**
     * Returns the maximum loot slot index available for the specified [Player].
     *
     * @param [Player] player : the player to check slots for
     * @return uint32 maxSlot
     */
    int GetMaxSlotForPlayer(lua_State* L, Loot* loot)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, loot->GetMaxSlotInLootFor(player));
        return 1;
    }

    /**
     * Adds a [Player] to the list of players currently looting this [Loot].
     *
     * @param [Player] player : the player to add as a looter
     */
    int AddLooter(lua_State* L, Loot* loot)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        loot->AddLooter(player->GetGUID());
        return 0;
    }

    /**
     * Removes a [Player] from the list of players currently looting this [Loot].
     *
     * @param [Player] player : the player to remove from looters
     */
    int RemoveLooter(lua_State* L, Loot* loot)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        loot->RemoveLooter(player->GetGUID());
        return 0;
    }
};
#endif // LOOTMETHODS_H
