/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "DBCStores.h"
#include "DBCStructure.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "SharedDefines.h"

/***
 * Represents item data defined in the database and DBCs, such as stats, quality, class restrictions, and display info.
 *
 * Used to access read-only metadata about items (not specific item instances in bags or equipment).
 *
 * Inherits all methods from: none
 */
namespace LuaItemTemplate
{
    /**
     * Returns the [ItemTemplate]'s ID.
     *
     * @return uint32 itemId
     */
    uint32 GetItemId(ItemTemplate* itemTemplate)
    {
        return itemTemplate->ItemId;
    }

    /**
     * Returns the [ItemTemplate]'s class.
     *
     * @return uint32 class
     */
    uint32 GetClass(ItemTemplate* itemTemplate)
    {
        return itemTemplate->Class;
    }

    /**
     * Returns the [ItemTemplate]'s subclass.
     *
     * @return uint32 subClass
     */
    uint32 GetSubClass(ItemTemplate* itemTemplate)
    {
        return itemTemplate->SubClass;
    }

    /**
     * Returns the [ItemTemplate]'s name in the [Player]'s locale.
     *
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the [ItemTemplate] name in (it's optional default: LOCALE_enUS)
     *
     * @return string name
     */
    std::string GetName(ItemTemplate* itemTemplate, sol::optional<uint32> locale)
    {
        uint32 loc_idx = locale.value_or(LocaleConstant::LOCALE_enUS);

        ItemLocale const* itemLocale = sObjectMgr->GetItemLocale(itemTemplate->ItemId);
        std::string name = itemTemplate->Name1;

        if (itemLocale && !itemLocale->Name[loc_idx].empty())
            name = itemLocale->Name[loc_idx];

        return name;
    }

    /**
     * Returns the [ItemTemplate]'s display ID.
     *
     * @return uint32 displayId
     */
    uint32 GetDisplayId(ItemTemplate* itemTemplate)
    {
        return itemTemplate->DisplayInfoID;
    }

    /**
     * Returns the [ItemTemplate]'s quality.
     *
     * @return uint32 quality
     */
    uint32 GetQuality(ItemTemplate* itemTemplate)
    {
        return itemTemplate->Quality;
    }

    /**
     * Returns the [ItemTemplate]'s flags.
     *
     * @return uint32 flags
     */
    uint32 GetFlags(ItemTemplate* itemTemplate)
    {
        return itemTemplate->Flags;
    }

    /**
     * Returns the [ItemTemplate]'s extra flags.
     *
     * @return uint32 flags
     */
    uint32 GetExtraFlags(ItemTemplate* itemTemplate)
    {
        return itemTemplate->Flags2;
    }

    /**
     * Returns the [ItemTemplate]'s default purchase count.
     *
     * @return uint32 buyCount
     */
    uint32 GetBuyCount(ItemTemplate* itemTemplate)
    {
        return itemTemplate->BuyCount;
    }

    /**
     * Returns the [ItemTemplate]'s purchase price.
     *
     * @return int32 buyPrice
     */
    int32 GetBuyPrice(ItemTemplate* itemTemplate)
    {
        return itemTemplate->BuyPrice;
    }

    /**
     * Returns the [ItemTemplate]'s sell price.
     *
     * @return uint32 sellPrice
     */
    uint32 GetSellPrice(ItemTemplate* itemTemplate)
    {
        return itemTemplate->SellPrice;
    }

    /**
     * Returns the [ItemTemplate]'s inventory type.
     *
     * @return uint32 inventoryType
     */
    uint32 GetInventoryType(ItemTemplate* itemTemplate)
    {
        return itemTemplate->InventoryType;
    }

    /**
     * Returns the [Player] classes allowed to use this [ItemTemplate].
     *
     * @return uint32 allowableClass
     */
    uint32 GetAllowableClass(ItemTemplate* itemTemplate)
    {
        return itemTemplate->AllowableClass;
    }

    /**
     * Returns the [Player] races allowed to use this [ItemTemplate].
     *
     * @return uint32 allowableRace
     */
    uint32 GetAllowableRace(ItemTemplate* itemTemplate)
    {
        return itemTemplate->AllowableRace;
    }

    /**
     * Returns the [ItemTemplate]'s item level.
     *
     * @return uint32 itemLevel
     */
    uint32 GetItemLevel(ItemTemplate* itemTemplate)
    {
        return itemTemplate->ItemLevel;
    }

    /**
     * Returns the minimum level required to use this [ItemTemplate].
     *
     * @return uint32 requiredLevel
     */
    uint32 GetRequiredLevel(ItemTemplate* itemTemplate)
    {
        return itemTemplate->RequiredLevel;
    }

    /**
     * Returns the icon is used by this [ItemTemplate].
     *
     * @return string itemIcon
     */
    char const* GetIcon(ItemTemplate* itemTemplate)
    {
        uint32 display_id = itemTemplate->DisplayInfoID;

        ItemDisplayInfoEntry const* displayInfo = sItemDisplayInfoStore.LookupEntry(display_id);
        char const* icon = displayInfo->inventoryIcon;

        return icon;
    }
}

void RegisterItemTemplateMethods(sol::state& lua)
{
    sol::usertype<ItemTemplate> type = lua.new_usertype<ItemTemplate>("ItemTemplate", sol::no_constructor);

    type["GetItemId"]         = &LuaItemTemplate::GetItemId;
    type["GetClass"]          = &LuaItemTemplate::GetClass;
    type["GetSubClass"]       = &LuaItemTemplate::GetSubClass;
    type["GetName"]           = &LuaItemTemplate::GetName;
    type["GetDisplayId"]      = &LuaItemTemplate::GetDisplayId;
    type["GetQuality"]        = &LuaItemTemplate::GetQuality;
    type["GetFlags"]          = &LuaItemTemplate::GetFlags;
    type["GetExtraFlags"]     = &LuaItemTemplate::GetExtraFlags;
    type["GetBuyCount"]       = &LuaItemTemplate::GetBuyCount;
    type["GetBuyPrice"]       = &LuaItemTemplate::GetBuyPrice;
    type["GetSellPrice"]      = &LuaItemTemplate::GetSellPrice;
    type["GetInventoryType"]  = &LuaItemTemplate::GetInventoryType;
    type["GetAllowableClass"] = &LuaItemTemplate::GetAllowableClass;
    type["GetAllowableRace"]  = &LuaItemTemplate::GetAllowableRace;
    type["GetItemLevel"]      = &LuaItemTemplate::GetItemLevel;
    type["GetRequiredLevel"]  = &LuaItemTemplate::GetRequiredLevel;
    type["GetIcon"]           = &LuaItemTemplate::GetIcon;
}
