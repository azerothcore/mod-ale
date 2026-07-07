/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Bag.h"
#include "Common.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SharedDefines.h"

#include <sstream>

/***
 * Represents an instance of an item in the game world.
 *
 * Inherits all methods from: [Object]
 */
namespace LuaItem
{
    /**
     * Returns 'true' if the [Item] is soulbound, 'false' otherwise
     *
     * @return bool isSoulBound
     */
    bool IsSoulBound(Item* item)
    {
        return item->IsSoulBound();
    }

    /**
     * Returns 'true' if the [Item] is account bound, 'false' otherwise
     *
     * @return bool isAccountBound
     */
    bool IsBoundAccountWide(Item* item)
    {
        return item->IsBoundAccountWide();
    }

    /**
     * Returns 'true' if the [Item] is bound to a [Player] by an enchant, 'false' otehrwise
     *
     * @return bool isBoundByEnchant
     */
    bool IsBoundByEnchant(Item* item)
    {
        return item->IsBoundByEnchant();
    }

    /**
     * Returns 'true' if the [Item] is not bound to the [Player] specified, 'false' otherwise
     *
     * @param [Player] player : the [Player] object to check the item against
     * @return bool isNotBound
     */
    bool IsNotBoundToPlayer(Item* item, Player* player)
    {
        return item->IsBindedNotWith(player);
    }

    /**
     * Returns 'true' if the [Item] is locked, 'false' otherwise
     *
     * @return bool isLocked
     */
    bool IsLocked(Item* item)
    {
        return item->IsLocked();
    }

    /**
     * Returns 'true' if the [Item] is a bag, 'false' otherwise
     *
     * @return bool isBag
     */
    bool IsBag(Item* item)
    {
        return item->IsBag();
    }

    /**
     * Returns 'true' if the [Item] is a currency token, 'false' otherwise
     *
     * @return bool isCurrencyToken
     */
    bool IsCurrencyToken(Item* item)
    {
        return item->IsCurrencyToken();
    }

    /**
     * Returns 'true' if the [Item] is a not an empty bag, 'false' otherwise
     *
     * @return bool isNotEmptyBag
     */
    bool IsNotEmptyBag(Item* item)
    {
        return item->IsNotEmptyBag();
    }

    /**
     * Returns 'true' if the [Item] is broken, 'false' otherwise
     *
     * @return bool isBroken
     */
    bool IsBroken(Item* item)
    {
        return item->IsBroken();
    }

    /**
     * Returns 'true' if the [Item] can be traded, 'false' otherwise
     *
     * @return bool isTradeable
     */
    bool CanBeTraded(Item* item, sol::optional<bool> mail)
    {
        return item->CanBeTraded(mail.value_or(false));
    }

    /**
     * Returns 'true' if the [Item] is currently in a trade window, 'false' otherwise
     *
     * @return bool isInTrade
     */
    bool IsInTrade(Item* item)
    {
        return item->IsInTrade();
    }

    /**
     * Returns 'true' if the [Item] is currently in a bag, 'false' otherwise
     *
     * @return bool isInBag
     */
    bool IsInBag(Item* item)
    {
        return item->IsInBag();
    }

    /**
     * Returns 'true' if the [Item] is currently equipped, 'false' otherwise
     *
     * @return bool isEquipped
     */
    bool IsEquipped(Item* item)
    {
        return item->IsEquipped();
    }

    /**
     * Returns 'true' if the [Item] has the [Quest] specified tied to it, 'false' otherwise
     *
     * @param uint32 questId : the [Quest] id to be checked
     * @return bool hasQuest
     */
    bool HasQuest(Item* item, uint32 quest)
    {
        return item->hasQuest(quest);
    }

    /**
     * Returns 'true' if the [Item] is a potion, 'false' otherwise
     *
     * @return bool isPotion
     */
    bool IsPotion(Item* item)
    {
        return item->IsPotion();
    }

    /**
     * Returns 'true' if the [Item] is a weapon vellum, 'false' otherwise
     *
     * @return bool isWeaponVellum
     */
    bool IsWeaponVellum(Item* item)
    {
        return item->IsWeaponVellum();
    }

    /**
     * Returns 'true' if the [Item] is an armor vellum, 'false' otherwise
     *
     * @return bool isArmorVellum
     */
    bool IsArmorVellum(Item* item)
    {
        return item->IsArmorVellum();
    }

    /**
     * Returns 'true' if the [Item] is a conjured consumable, 'false' otherwise
     *
     * @return bool isConjuredConsumable
     */
    bool IsConjuredConsumable(Item* item)
    {
        return item->IsConjuredConsumable();
    }

    /*bool IsRefundExpired(Item* item)// TODO: Implement core support
    {
        return item->IsRefundExpired();
    }*/

    /**
     * Returns the chat link of the [Item]
     *
     * <pre>
     * enum LocaleConstant
     * {
     *     LOCALE_enUS = 0,
     *     LOCALE_koKR = 1,
     *     LOCALE_frFR = 2,
     *     LOCALE_deDE = 3,
     *     LOCALE_zhCN = 4,
     *     LOCALE_zhTW = 5,
     *     LOCALE_esES = 6,
     *     LOCALE_esMX = 7,
     *     LOCALE_ruRU = 8
     * };
     * </pre>
     *
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the [Item]'s name in
     * @return string itemLink
     */
    std::string GetItemLink(Item* item, sol::optional<uint8> localeArg)
    {
        uint8 locale = localeArg.value_or(DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            throw std::invalid_argument("valid LocaleConstant expected");

        ItemTemplate const* temp = item->GetTemplate();
        std::string name = temp->Name1;
        if (ItemLocale const* il = sObjectMgr->GetItemLocale(temp->ItemId))
        {
            ObjectMgr::GetLocaleString(il->Name, static_cast<LocaleConstant>(locale), name);
        }

        if (int32 itemRandPropId = item->GetItemRandomPropertyId())
        {
            std::array<char const*, 16> const* suffix = NULL;
            if (itemRandPropId < 0)
            {
                ItemRandomSuffixEntry const* itemRandEntry = sItemRandomSuffixStore.LookupEntry(-item->GetItemRandomPropertyId());
                if (itemRandEntry)
                    suffix = &itemRandEntry->Name;
            }
            else
            {
                ItemRandomPropertiesEntry const* itemRandEntry = sItemRandomPropertiesStore.LookupEntry(item->GetItemRandomPropertyId());
                if (itemRandEntry)
                    suffix = &itemRandEntry->Name;
            }
            if (suffix)
            {
                char const* suffixName = (*suffix)[(name != temp->Name1) ? locale : uint8(DEFAULT_LOCALE)];
                if (strcmp(suffixName, "") != 0)
                {
                    name += ' ';
                    name += suffixName;
                }
            }
        }

        Player* owner = item->GetOwner();
        std::ostringstream oss;
        oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
            "|Hitem:" << temp->ItemId << ":" <<
            item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) << ":" <<
            item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT) << ":" <<
            item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2) << ":" <<
            item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3) << ":" <<
            item->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT) << ":" <<
            item->GetItemRandomPropertyId() << ":" << item->GetItemSuffixFactor() << ":" <<
            (uint32)(owner ? owner->GetLevel() : 0) << "|h[" << name << "]|h|r";

        return oss.str();
    }

    /**
     * Returns the GUID of the [Player] who owns the specified [Item].
     *
     * @param [Item] item
     * @return uint64 ownerGUID
     */
    ObjectGuid GetOwnerGUID(Item* item)
    {
        return item->GetOwnerGUID();
    }

    /**
     * Returns the [Player] who currently owns the [Item]
     *
     * @return [Player] player : the [Player] who owns the [Item]
     */
    Player* GetOwner(Item* item)
    {
        return item->GetOwner();
    }

    /**
     * Returns the [Item]s stack count
     *
     * @return uint32 count
     */
    uint32 GetCount(Item* item)
    {
        return item->GetCount();
    }

    /**
     * Returns the [Item]s max stack count
     *
     * @return uint32 maxCount
     */
    uint32 GetMaxStackCount(Item* item)
    {
        return item->GetMaxStackCount();
    }

    /**
     * Returns the [Item]s current slot
     *
     * @return uint8 slot
     */
    uint8 GetSlot(Item* item)
    {
        return item->GetSlot();
    }

    /**
     * Returns the [Item]s current bag slot
     *
     * @return uint8 bagSlot
     */
    uint8 GetBagSlot(Item* item)
    {
        return item->GetBagSlot();
    }

    /**
     * Returns the [Item]s enchantment ID by enchant slot specified
     *
     * @param [EnchantmentSlot] enchantSlot : the enchant slot specified
     * @return uint32 enchantId : the id of the enchant slot specified
     */
    uint32 GetEnchantmentId(Item* item, uint32 enchant_slot)
    {
        if (enchant_slot >= MAX_INSPECTED_ENCHANTMENT_SLOT)
            throw std::invalid_argument("valid EnchantmentSlot expected");

        return item->GetEnchantmentId(EnchantmentSlot(enchant_slot));
    }

    /**
     * Returns the spell ID tied to the [Item] by spell index
     *
     * @param uint32 spellIndex : the spell index specified
     * @return uint32 spellId : the id of the spell
     */
    int32 GetSpellId(Item* item, uint32 index)
    {
        if (index >= MAX_ITEM_PROTO_SPELLS)
            throw std::invalid_argument("valid SpellIndex expected");

        return item->GetTemplate()->Spells[index].SpellId;
    }

    /**
     * Returns the spell trigger tied to the [Item] by spell index
     *
     * @param uint32 spellIndex : the spell index specified
     * @return uint32 spellTrigger : the spell trigger of the specified index
     */
    uint32 GetSpellTrigger(Item* item, uint32 index)
    {
        if (index >= MAX_ITEM_PROTO_SPELLS)
            throw std::invalid_argument("valid SpellIndex expected");

        return item->GetTemplate()->Spells[index].SpellTrigger;
    }

    /**
     * Returns class of the [Item]
     *
     * @return uint32 class
     */
    uint32 GetClass(Item* item)
    {
        return item->GetTemplate()->Class;
    }

    /**
     * Returns subclass of the [Item]
     *
     * @return uint32 subClass
     */
    uint32 GetSubClass(Item* item)
    {
        return item->GetTemplate()->SubClass;
    }

    /**
     * Returns the name of the [Item]
     *
     * @return string name
     */
    std::string GetName(Item* item)
    {
        return item->GetTemplate()->Name1;
    }

    /**
     * Returns the display ID of the [Item]
     *
     * @return uint32 displayId
     */
    uint32 GetDisplayId(Item* item)
    {
        return item->GetTemplate()->DisplayInfoID;
    }

    /**
     * Returns the quality of the [Item]
     *
     * @return uint32 quality
     */
    uint32 GetQuality(Item* item)
    {
        return item->GetTemplate()->Quality;
    }

    /**
     * Returns the default purchase count of the [Item]
     *
     * @return uint32 count
     */
    uint32 GetBuyCount(Item* item)
    {
        return item->GetTemplate()->BuyCount;
    }

    /**
     * Returns the purchase price of the [Item]
     *
     * @return uint32 price
     */
    uint32 GetBuyPrice(Item* item)
    {
        return item->GetTemplate()->BuyPrice;
    }

    /**
     * Returns the sell price of the [Item]
     *
     * @return uint32 price
     */
    uint32 GetSellPrice(Item* item)
    {
        return item->GetTemplate()->SellPrice;
    }

    /**
     * Returns the inventory type of the [Item]
     *
     * @return uint32 inventoryType
     */
    uint32 GetInventoryType(Item* item)
    {
        return item->GetTemplate()->InventoryType;
    }

    /**
     * Returns the [Player] classes allowed to use this [Item]
     *
     * @return uint32 allowableClass
     */
    uint32 GetAllowableClass(Item* item)
    {
        return item->GetTemplate()->AllowableClass;
    }

    /**
     * Returns the [Player] races allowed to use this [Item]
     *
     * @return uint32 allowableRace
     */
    uint32 GetAllowableRace(Item* item)
    {
        return item->GetTemplate()->AllowableRace;
    }

    /**
     * Returns the [Item]s level
     *
     * @return uint32 itemLevel
     */
    uint32 GetItemLevel(Item* item)
    {
        return item->GetTemplate()->ItemLevel;
    }

    /**
     * Returns the minimum level required to use this [Item]
     *
     * @return uint32 requiredLevel
     */
    uint32 GetRequiredLevel(Item* item)
    {
        return item->GetTemplate()->RequiredLevel;
    }

    /**
     * Returns the number of stat entries defined on the [Item]'s [ItemTemplate].  This reflects how many stat slots (e.g., Strength, Stamina, etc.) are defined for the item.
     *
     * @param [Item] item
     * @return uint32 statsCount
     */
    uint32 GetStatsCount(Item* item)
    {
        return item->GetTemplate()->StatsCount;
    }

    /**
     * Returns the random property ID of this [Item]
     *
     * @return uint32 randomPropertyId
     */
    uint32 GetRandomProperty(Item* item)
    {
        return item->GetTemplate()->RandomProperty;
    }

    /**
     * Returns the random suffix ID of the specified [Item].  This corresponds to the `RandomSuffix` field from the item's [ItemTemplate], which controls the applied suffix (e.g., "of the Bear", "of the Eagle").
     *
     * @param [Item] item
     * @return uint32 randomSuffixId
     */
    uint32 GetRandomSuffix(Item* item)
    {
        return item->GetTemplate()->RandomSuffix;
    }

    /**
     * Returns the item set ID of this [Item]
     *
     * @return uint32 itemSetId
     */
    uint32 GetItemSet(Item* item)
    {
        return item->GetTemplate()->ItemSet;
    }

    /**
     * Returns the bag size of this [Item], 0 if [Item] is not a bag
     *
     * @return uint32 bagSize
     */
    uint32 GetBagSize(Item* item)
    {
        if (Bag* bag = item->ToBag())
            return bag->GetBagSize();

        return 0;
    }

    /**
     * Returns the [ItemTemplate] for this [Item].
     *
     * @return [ItemTemplate] itemTemplate
     */
    ItemTemplate const* GetItemTemplate(Item* item)
    {
        return item->GetTemplate();
    }

    /**
     * Sets the [Player] specified as the owner of the [Item]
     *
     * @param [Player] player : the [Player] specified
     */
    void SetOwner(Item* item, Player* player)
    {
        item->SetOwnerGUID(player->GetGUID());
    }

    /**
     * Sets the binding of the [Item] to 'true' or 'false'
     *
     * @param bool setBinding
     */
    void SetBinding(Item* item, bool soulbound)
    {
        item->SetBinding(soulbound);
        item->SetState(ITEM_CHANGED, item->GetOwner());
    }

    /**
     * Sets the stack count of the [Item]
     *
     * @param uint32 count
     */
    void SetCount(Item* item, uint32 count)
    {
        item->SetCount(count);
    }

    /**
     * Sets the specified enchantment of the [Item] to the specified slot
     *
     * @param uint32 enchantId : the ID of the enchant to be applied
     * @param uint32 enchantSlot : the slot for the enchant to be applied to
     * @return bool enchantmentSuccess : if enchantment is successfully set to specified slot, returns 'true', otherwise 'false'
     */
    bool SetEnchantment(Item* item, uint32 enchant, uint32 enchantSlot)
    {
        Player* owner = item->GetOwner();
        if (!owner)
        {
            return false;
        }

        if (!sSpellItemEnchantmentStore.LookupEntry(enchant))
        {
            return false;
        }

        EnchantmentSlot slot = (EnchantmentSlot)enchantSlot;
        if (slot >= MAX_INSPECTED_ENCHANTMENT_SLOT)
            throw std::invalid_argument("valid EnchantmentSlot expected");

        owner->ApplyEnchantment(item, slot, false);
        item->SetEnchantment(slot, enchant, 0, 0);
        owner->ApplyEnchantment(item, slot, true);
        return true;
    }


    /**
     * Sets the random properties for the [Item] from a given random property ID.
     *
     * @param uint32 randomPropId : The ID of the random property to be applied.
     */
    void SetRandomProperty(Item* item, uint32 randomPropId)
    {
        item->SetItemRandomProperties(randomPropId);
    }

    /**
     * Sets the random suffix for the [Item] from a given random suffix ID.
     *
     * @param uint32 randomSuffixId : The ID of the random suffix to be applied.
     */
    void SetRandomSuffix(Item* item, uint32 randomPropId)
    {
        item->SetItemRandomProperties(-randomPropId);
    }


    /* OTHER */
    /**
     * Removes an enchant from the [Item] by the specified slot
     *
     * @param uint32 enchantSlot : the slot for the enchant to be removed from
     * @return bool enchantmentRemoved : if enchantment is successfully removed from specified slot, returns 'true', otherwise 'false'
     */
    bool ClearEnchantment(Item* item, uint32 enchantSlot)
    {
        Player* owner = item->GetOwner();
        if (!owner)
        {
            return false;
        }

        EnchantmentSlot slot = (EnchantmentSlot)enchantSlot;
        if (slot >= MAX_INSPECTED_ENCHANTMENT_SLOT)
            throw std::invalid_argument("valid EnchantmentSlot expected");

        if (!item->GetEnchantmentId(slot))
        {
            return false;
        }

        owner->ApplyEnchantment(item, slot, false);
        item->ClearEnchantment(slot);
        return true;
    }

    /**
     * Saves the [Item] to the database
     */
    void SaveToDB(Item* item)
    {
        CharacterDatabaseTransaction trans = CharacterDatabaseTransaction(nullptr);
        item->SaveToDB(trans);
    }
}

void RegisterItemMethods(sol::state& lua)
{
    sol::usertype<ItemRef> type = ALEBind::NewHandleType<ItemRef, ObjectRef>(lua, "Item");

    type["IsSoulBound"]          = ALEBind::Method(&LuaItem::IsSoulBound);
    type["IsBoundAccountWide"]   = ALEBind::Method(&LuaItem::IsBoundAccountWide);
    type["IsBoundByEnchant"]     = ALEBind::Method(&LuaItem::IsBoundByEnchant);
    type["IsNotBoundToPlayer"]   = ALEBind::Method(&LuaItem::IsNotBoundToPlayer);
    type["IsLocked"]             = ALEBind::Method(&LuaItem::IsLocked);
    type["IsBag"]                = ALEBind::Method(&LuaItem::IsBag);
    type["IsCurrencyToken"]      = ALEBind::Method(&LuaItem::IsCurrencyToken);
    type["IsNotEmptyBag"]        = ALEBind::Method(&LuaItem::IsNotEmptyBag);
    type["IsBroken"]             = ALEBind::Method(&LuaItem::IsBroken);
    type["CanBeTraded"]          = ALEBind::Method(&LuaItem::CanBeTraded);
    type["IsInTrade"]            = ALEBind::Method(&LuaItem::IsInTrade);
    type["IsInBag"]              = ALEBind::Method(&LuaItem::IsInBag);
    type["IsEquipped"]           = ALEBind::Method(&LuaItem::IsEquipped);
    type["HasQuest"]             = ALEBind::Method(&LuaItem::HasQuest);
    type["IsPotion"]             = ALEBind::Method(&LuaItem::IsPotion);
    type["IsWeaponVellum"]       = ALEBind::Method(&LuaItem::IsWeaponVellum);
    type["IsArmorVellum"]        = ALEBind::Method(&LuaItem::IsArmorVellum);
    type["IsConjuredConsumable"] = ALEBind::Method(&LuaItem::IsConjuredConsumable);
    type["GetItemLink"]          = ALEBind::Method(&LuaItem::GetItemLink);
    type["GetOwnerGUID"]         = ALEBind::Method(&LuaItem::GetOwnerGUID);
    type["GetOwner"]             = ALEBind::Method(&LuaItem::GetOwner);
    type["GetCount"]             = ALEBind::Method(&LuaItem::GetCount);
    type["GetMaxStackCount"]     = ALEBind::Method(&LuaItem::GetMaxStackCount);
    type["GetSlot"]              = ALEBind::Method(&LuaItem::GetSlot);
    type["GetBagSlot"]           = ALEBind::Method(&LuaItem::GetBagSlot);
    type["GetEnchantmentId"]     = ALEBind::Method(&LuaItem::GetEnchantmentId);
    type["GetSpellId"]           = ALEBind::Method(&LuaItem::GetSpellId);
    type["GetSpellTrigger"]      = ALEBind::Method(&LuaItem::GetSpellTrigger);
    type["GetClass"]             = ALEBind::Method(&LuaItem::GetClass);
    type["GetSubClass"]          = ALEBind::Method(&LuaItem::GetSubClass);
    type["GetName"]              = ALEBind::Method(&LuaItem::GetName);
    type["GetDisplayId"]         = ALEBind::Method(&LuaItem::GetDisplayId);
    type["GetQuality"]           = ALEBind::Method(&LuaItem::GetQuality);
    type["GetBuyCount"]          = ALEBind::Method(&LuaItem::GetBuyCount);
    type["GetBuyPrice"]          = ALEBind::Method(&LuaItem::GetBuyPrice);
    type["GetSellPrice"]         = ALEBind::Method(&LuaItem::GetSellPrice);
    type["GetInventoryType"]     = ALEBind::Method(&LuaItem::GetInventoryType);
    type["GetAllowableClass"]    = ALEBind::Method(&LuaItem::GetAllowableClass);
    type["GetAllowableRace"]     = ALEBind::Method(&LuaItem::GetAllowableRace);
    type["GetItemLevel"]         = ALEBind::Method(&LuaItem::GetItemLevel);
    type["GetRequiredLevel"]     = ALEBind::Method(&LuaItem::GetRequiredLevel);
    type["GetStatsCount"]        = ALEBind::Method(&LuaItem::GetStatsCount);
    type["GetRandomProperty"]    = ALEBind::Method(&LuaItem::GetRandomProperty);
    type["GetRandomSuffix"]      = ALEBind::Method(&LuaItem::GetRandomSuffix);
    type["GetItemSet"]           = ALEBind::Method(&LuaItem::GetItemSet);
    type["GetBagSize"]           = ALEBind::Method(&LuaItem::GetBagSize);
    type["GetItemTemplate"]      = ALEBind::Method(&LuaItem::GetItemTemplate);
    type["SetOwner"]             = ALEBind::Method(&LuaItem::SetOwner);
    type["SetBinding"]           = ALEBind::Method(&LuaItem::SetBinding);
    type["SetCount"]             = ALEBind::Method(&LuaItem::SetCount);
    type["SetEnchantment"]       = ALEBind::Method(&LuaItem::SetEnchantment);
    type["SetRandomProperty"]    = ALEBind::Method(&LuaItem::SetRandomProperty);
    type["SetRandomSuffix"]      = ALEBind::Method(&LuaItem::SetRandomSuffix);
    type["ClearEnchantment"]     = ALEBind::Method(&LuaItem::ClearEnchantment);
    type["SaveToDB"]             = ALEBind::Method(&LuaItem::SaveToDB);
}
