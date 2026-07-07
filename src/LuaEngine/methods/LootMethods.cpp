/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "LootMgr.h"
#include "Player.h"

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
    bool IsLooted(Loot* loot)
    {
        return loot->isLooted();
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
    void AddItem(Loot* loot, uint32 itemid, uint8 min_count, uint8 max_count, float chance, uint16 loot_mode, sol::optional<bool> needsQuest, sol::optional<bool> allowStacking)
    {
        bool needs_quest = needsQuest.value_or(false);
        bool allow_stacking = allowStacking.value_or(true);

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
                    return;
                }
            }
        }

        LootStoreItem newLootStoreItem(itemid, 0, chance, needs_quest, loot_mode, 0, min_count, max_count);
        loot->AddItem(newLootStoreItem);
    }

    /**
     * Returns `true` if the [Loot] contains the specified item, and returns `false` otherwise.
     *
     * @param uint32 itemId = 0 : the ID of the item to check for. If 0, checks if any item exists
     * @param uint32 count = 0 : specific count to check for. If 0, ignores count
     * @return bool hasItem
     */
    bool HasItem(Loot* loot, sol::optional<uint32> itemId, sol::optional<uint32> countArg)
    {
        uint32 itemid = itemId.value_or(0);
        uint32 count = countArg.value_or(0);
        bool has_item = false;

        if (itemid)
        {
            for (LootItem const& lootitem : loot->items)
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
            for (LootItem const& lootitem : loot->items)
            {
                if (lootitem.itemid != 0)
                {
                    has_item = true;
                    break;
                }
            }
        }

        return has_item;
    }

    // Helper for RemoveItem: erases (or decrements) matching items in one loot container.
    void RemoveItemFromContainer(std::vector<LootItem>& container, uint32 itemid, bool isCountSpecified, uint32& remaining)
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
    void RemoveItem(Loot* loot, uint32 itemid, sol::optional<bool> isCountSpecifiedArg, sol::optional<uint32> countArg)
    {
        bool isCountSpecified = isCountSpecifiedArg.value_or(false);
        uint32 count = 0;
        if (isCountSpecified)
        {
            if (!countArg)
                throw std::invalid_argument("count expected when isCountSpecified is true");

            count = *countArg;
        }

        // Remove from regular items
        RemoveItemFromContainer(loot->items, itemid, isCountSpecified, count);

        // Remove from quest items as well
        if (!isCountSpecified || count > 0)
            RemoveItemFromContainer(loot->quest_items, itemid, isCountSpecified, count);
    }

    /**
     * Returns the amount of money in this [Loot].
     *
     * @return uint32 money : the amount of money in copper
     */
    uint32 GetMoney(Loot* loot)
    {
        return loot->gold;
    }

    /**
     * Sets the amount of money in this [Loot].
     *
     * @param uint32 money : the amount of money to set in copper
     */
    void SetMoney(Loot* loot, uint32 gold)
    {
        loot->gold = gold;
    }

    /**
     * Generates a random amount of money for this [Loot] within the specified range.
     *
     * @param uint32 minGold : minimum amount of money in copper
     * @param uint32 maxGold : maximum amount of money in copper
     */
    void GenerateMoney(Loot* loot, uint32 min_gold, uint32 max_gold)
    {
        loot->generateMoneyLoot(min_gold, max_gold);
    }

    /**
     * Clears all items and money from this [Loot].
     */
    void Clear(Loot* loot)
    {
        loot->clear();
    }

    /**
     * Sets the number of unlooted items in this [Loot].
     *
     * @param uint32 count : the number of unlooted items
     */
    void SetUnlootedCount(Loot* loot, uint32 count)
    {
        loot->unlootedCount = count;
    }

    /**
     * Returns the number of unlooted items in this [Loot].
     *
     * @return uint32 unlootedCount
     */
    uint32 GetUnlootedCount(Loot* loot)
    {
        return loot->unlootedCount;
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
    sol::table GetItems(Loot* loot, sol::this_state s)
    {
        sol::state_view lua(s);
        sol::table tbl = lua.create_table();

        for (unsigned int i = 0; i < loot->items.size(); i++)
        {
            sol::table item = lua.create_table();

            item["id"] = loot->items[i].itemid;
            item["index"] = loot->items[i].itemIndex;
            item["count"] = static_cast<uint8>(loot->items[i].count);
            item["needs_quest"] = static_cast<bool>(loot->items[i].needs_quest);
            item["is_looted"] = static_cast<bool>(loot->items[i].is_looted);
            item["roll_winner_guid"] = loot->items[i].rollWinnerGUID;

            tbl[i + 1] = item;
        }

        return tbl;
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
    sol::table GetQuestItems(Loot* loot, sol::this_state s)
    {
        sol::state_view lua(s);
        sol::table tbl = lua.create_table();

        for (unsigned int i = 0; i < loot->quest_items.size(); i++)
        {
            sol::table item = lua.create_table();

            item["id"] = loot->quest_items[i].itemid;
            item["index"] = loot->quest_items[i].itemIndex;
            item["count"] = static_cast<uint8>(loot->quest_items[i].count);
            item["needs_quest"] = static_cast<bool>(loot->quest_items[i].needs_quest);
            item["is_looted"] = static_cast<bool>(loot->quest_items[i].is_looted);
            item["roll_winner_guid"] = loot->quest_items[i].rollWinnerGUID;

            tbl[i + 1] = item;
        }

        return tbl;
    }

    /**
     * Updates the index of all items in this [Loot] to match their position in the list.
     *
     * This should be called after removing items to ensure indices are sequential.
     */
    void UpdateItemIndex(Loot* loot)
    {
        uint32 index = 0;

        for (unsigned int i = 0; i < loot->items.size(); ++i)
            loot->items[i].itemIndex = index++;

        for (unsigned int i = 0; i < loot->quest_items.size(); ++i)
            loot->quest_items[i].itemIndex = index++;
    }

    /**
     * Sets the looted status of a specific item in this [Loot].
     *
     * @param uint32 itemId : the ID of the item
     * @param uint32 count : specific count to match. If 0, ignores count
     * @param bool looted = true : `true` to mark as looted, `false` to mark as unlooted
     */
    void SetItemLooted(Loot* loot, uint32 itemid, uint32 count, sol::optional<bool> lootedArg)
    {
        bool looted = lootedArg.value_or(true);

        for (auto& lootItem : loot->items)
        {
            if (lootItem.itemid == itemid && (count == 0 || lootItem.count == count))
            {
                lootItem.is_looted = looted;
                break;
            }
        }
    }

    /**
     * Returns `true` if the [Loot] is completely empty (no items and no money), returns `false` otherwise.
     *
     * @return bool isEmpty
     */
    bool IsEmpty(Loot* loot)
    {
        return loot->empty();
    }

    /**
     * Returns the [Loot] type.
     *
     * @return [LootType] lootType
     */
    LootType GetLootType(Loot* loot)
    {
        return loot->loot_type;
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
    void SetLootType(Loot* loot, uint32 lootType)
    {
        loot->loot_type = static_cast<LootType>(lootType);
    }

    /**
     * Returns the [Player] GUID that owns this loot for round robin distribution.
     *
     * @return ObjectGuid roundRobinPlayer : the player GUID
     */
    ObjectGuid GetRoundRobinPlayer(Loot* loot)
    {
        return loot->roundRobinPlayer;
    }

    /**
     * Sets the [Player] GUID for round robin loot distribution.
     *
     * @param ObjectGuid playerGUID : the player GUID
     */
    void SetRoundRobinPlayer(Loot* loot, ObjectGuid guid)
    {
        loot->roundRobinPlayer = guid;
    }

    /**
     * Returns the [Player] GUID that owns this loot.
     *
     * @return ObjectGuid lootOwner : the player GUID
     */
    ObjectGuid GetLootOwner(Loot* loot)
    {
        return loot->lootOwnerGUID;
    }

    /**
     * Sets the [Player] GUID that owns this loot.
     *
     * @param ObjectGuid playerGUID : the player GUID
     */
    void SetLootOwner(Loot* loot, ObjectGuid guid)
    {
        loot->lootOwnerGUID = guid;
    }

    /**
     * Returns the container GUID that holds this loot.
     *
     * @return ObjectGuid containerGUID : the container GUID
     */
    ObjectGuid GetContainer(Loot* loot)
    {
        return loot->containerGUID;
    }

    /**
     * Sets the container GUID that holds this loot.
     *
     * @param ObjectGuid containerGUID : the container GUID
     */
    void SetContainer(Loot* loot, ObjectGuid guid)
    {
        loot->containerGUID = guid;
    }

    /**
     * Returns the source [WorldObject] GUID for this loot.
     *
     * @return ObjectGuid sourceGUID : the source [WorldObject] GUID
     */
    ObjectGuid GetSourceWorldObject(Loot* loot)
    {
        return loot->sourceWorldObjectGUID;
    }

    /**
     * Sets the source [WorldObject] GUID for this loot.
     *
     * @param ObjectGuid sourceGUID : the source [WorldObject] GUID
     */
    void SetSourceWorldObject(Loot* loot, ObjectGuid guid)
    {
        loot->sourceWorldObjectGUID = guid;
    }

    /**
     * Returns `true` if the [Loot] contains quest items and returns `false` otherwise.
     *
     * @return bool hasQuestItems
     */
    bool HasQuestItems(Loot* loot)
    {
        return !loot->quest_items.empty();
    }

    /**
     * Returns `true` if the [Loot] has items available for all players and returns `false` otherwise.
     *
     * @return bool hasItemForAll
     */
    bool HasItemForAll(Loot* loot)
    {
        return loot->hasItemForAll();
    }

    /**
     * Returns `true` if the [Loot] has items that are over the group loot threshold and returns `false` otherwise.
     *
     * @return bool hasOverThresholdItem
     */
    bool HasOverThresholdItem(Loot* loot)
    {
        return loot->hasOverThresholdItem();
    }

    /**
     * Returns the total number of items (regular + quest items) in this [Loot].
     *
     * @return uint32 itemCount
     */
    uint32 GetItemCount(Loot* loot)
    {
        return static_cast<uint32>(loot->items.size() + loot->quest_items.size());
    }

    /**
     * Returns the maximum loot slot index available for the specified [Player].
     *
     * @param [Player] player : the player to check slots for
     * @return uint32 maxSlot
     */
    uint32 GetMaxSlotForPlayer(Loot* loot, Player* player)
    {
        return loot->GetMaxSlotInLootFor(player);
    }

    /**
     * Adds a [Player] to the list of players currently looting this [Loot].
     *
     * @param [Player] player : the player to add as a looter
     */
    void AddLooter(Loot* loot, Player* player)
    {
        loot->AddLooter(player->GetGUID());
    }

    /**
     * Removes a [Player] from the list of players currently looting this [Loot].
     *
     * @param [Player] player : the player to remove from looters
     */
    void RemoveLooter(Loot* loot, Player* player)
    {
        loot->RemoveLooter(player->GetGUID());
    }
}

void RegisterLootMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<Loot>> type = ALEBind::NewHandleType<ScopedRef<Loot>>(lua, "Loot");

    type["IsLooted"]             = ALEBind::Method(&LuaLoot::IsLooted);
    type["AddItem"]              = ALEBind::Method(&LuaLoot::AddItem);
    type["HasItem"]              = ALEBind::Method(&LuaLoot::HasItem);
    type["RemoveItem"]           = ALEBind::Method(&LuaLoot::RemoveItem);
    type["GetMoney"]             = ALEBind::Method(&LuaLoot::GetMoney);
    type["SetMoney"]             = ALEBind::Method(&LuaLoot::SetMoney);
    type["GenerateMoney"]        = ALEBind::Method(&LuaLoot::GenerateMoney);
    type["Clear"]                = ALEBind::Method(&LuaLoot::Clear);
    type["SetUnlootedCount"]     = ALEBind::Method(&LuaLoot::SetUnlootedCount);
    type["GetUnlootedCount"]     = ALEBind::Method(&LuaLoot::GetUnlootedCount);
    type["GetItems"]             = ALEBind::Method(&LuaLoot::GetItems);
    type["GetQuestItems"]        = ALEBind::Method(&LuaLoot::GetQuestItems);
    type["UpdateItemIndex"]      = ALEBind::Method(&LuaLoot::UpdateItemIndex);
    type["SetItemLooted"]        = ALEBind::Method(&LuaLoot::SetItemLooted);
    type["IsEmpty"]              = ALEBind::Method(&LuaLoot::IsEmpty);
    type["GetLootType"]          = ALEBind::Method(&LuaLoot::GetLootType);
    type["SetLootType"]          = ALEBind::Method(&LuaLoot::SetLootType);
    type["GetRoundRobinPlayer"]  = ALEBind::Method(&LuaLoot::GetRoundRobinPlayer);
    type["SetRoundRobinPlayer"]  = ALEBind::Method(&LuaLoot::SetRoundRobinPlayer);
    type["GetLootOwner"]         = ALEBind::Method(&LuaLoot::GetLootOwner);
    type["SetLootOwner"]         = ALEBind::Method(&LuaLoot::SetLootOwner);
    type["GetContainer"]         = ALEBind::Method(&LuaLoot::GetContainer);
    type["SetContainer"]         = ALEBind::Method(&LuaLoot::SetContainer);
    type["GetSourceWorldObject"] = ALEBind::Method(&LuaLoot::GetSourceWorldObject);
    type["SetSourceWorldObject"] = ALEBind::Method(&LuaLoot::SetSourceWorldObject);
    type["HasQuestItems"]        = ALEBind::Method(&LuaLoot::HasQuestItems);
    type["HasItemForAll"]        = ALEBind::Method(&LuaLoot::HasItemForAll);
    type["HasOverThresholdItem"] = ALEBind::Method(&LuaLoot::HasOverThresholdItem);
    type["GetItemCount"]         = ALEBind::Method(&LuaLoot::GetItemCount);
    type["GetMaxSlotForPlayer"]  = ALEBind::Method(&LuaLoot::GetMaxSlotForPlayer);
    type["AddLooter"]            = ALEBind::Method(&LuaLoot::AddLooter);
    type["RemoveLooter"]         = ALEBind::Method(&LuaLoot::RemoveLooter);
}
