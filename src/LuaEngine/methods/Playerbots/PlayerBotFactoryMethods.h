/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef PLAYERBOTFACTORYMETHODS_H
#define PLAYERBOTFACTORYMETHODS_H

#ifdef MOD_PLAYERBOTS
#include "PlayerbotAI.h"
#include "PlayerbotFactory.h"

namespace LuaPlayerBotFactory
{
    /**
     * Rerolls everything on the bot: level, gear, talents, spells and consumables.
     *
     * @param bool incremental = false : keep what the bot already has when possible
     */
    int Randomize(lua_State* L, PlayerbotFactory* factory)
    {
        bool incremental = ALE::CHECKVAL<bool>(L, 2, false);
        factory->Randomize(incremental);
        return 0;
    }

    /**
     * Restores the consumables, the ammo and the durability of the bot.
     */
    int Refresh(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->Refresh();
        return 0;
    }

    /**
     * Strips the bot of everything it owns.
     */
    int ClearEverything(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->ClearEverything();
        return 0;
    }

    /**
     * Grants the bot the skills matching its class and level.
     */
    int InitSkills(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitSkills();
        return 0;
    }

    /**
     * Spends the talent points of the bot and returns the chosen talent tree.
     *
     * @param bool incremental = false
     * @param bool useTemplate = true : follow a talent template instead of picking freely
     * @param bool reset = false : reset the talents first
     * @return uint32 talentTree
     */
    int InitTalentsTree(lua_State* L, PlayerbotFactory* factory)
    {
        bool incremental = ALE::CHECKVAL<bool>(L, 2, false);
        bool useTemplate = ALE::CHECKVAL<bool>(L, 3, true);
        bool reset = ALE::CHECKVAL<bool>(L, 4, false);

        ALE::Push(L, factory->InitTalentsTree(incremental, useTemplate, reset));
        return 1;
    }

    /**
     * Teaches the bot every spell available to it.
     */
    int InitAvailableSpells(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitAvailableSpells();
        return 0;
    }

    /**
     * Teaches the bot the spells of its class.
     */
    int InitClassSpells(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitClassSpells();
        return 0;
    }

    /**
     * Teaches the bot the special spells it needs, riding and weapon skills for example.
     */
    int InitSpecialSpells(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitSpecialSpells();
        return 0;
    }

    /**
     * Equips the bot with gear matching its level and the quality of the factory.
     *
     * @param bool incremental = false : only replace what is worse than the new gear
     * @param bool secondChance = true : retry the slots left empty with looser rules
     */
    int InitEquipment(lua_State* L, PlayerbotFactory* factory)
    {
        bool incremental = ALE::CHECKVAL<bool>(L, 2, false);
        bool secondChance = ALE::CHECKVAL<bool>(L, 3, true);

        factory->InitEquipment(incremental, secondChance);
        return 0;
    }

    /**
     * Applies the enchants and the gems the bot gear should carry.
     *
     * @param bool destroyOld = true
     */
    int ApplyEnchantAndGems(lua_State* L, PlayerbotFactory* factory)
    {
        bool destroyOld = ALE::CHECKVAL<bool>(L, 2, true);
        factory->ApplyEnchantAndGemsNew(destroyOld);
        return 0;
    }

    /**
     * Gives the bot the bags it should carry.
     *
     * @param bool destroyOld = true
     */
    int InitBags(lua_State* L, PlayerbotFactory* factory)
    {
        bool destroyOld = ALE::CHECKVAL<bool>(L, 2, true);
        factory->InitBags(destroyOld);
        return 0;
    }

    /**
     * Summons and levels the pet of the bot.
     */
    int InitPet(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitPet();
        return 0;
    }

    /**
     * Spends the talent points of the bot pet.
     */
    int InitPetTalents(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitPetTalents();
        return 0;
    }

    /**
     * Fills the quiver of the bot with the ammo matching its ranged weapon.
     */
    int InitAmmo(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitAmmo();
        return 0;
    }

    /**
     * Gives the bot the glyphs matching its talents.
     *
     * @param bool incremental = false
     */
    int InitGlyphs(lua_State* L, PlayerbotFactory* factory)
    {
        bool incremental = ALE::CHECKVAL<bool>(L, 2, false);
        factory->InitGlyphs(incremental);
        return 0;
    }

    /**
     * Destroys the consumables the bot no longer needs.
     */
    int CleanupConsumables(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->CleanupConsumables();
        return 0;
    }

    /**
     * Gives the bot the reagents its spells consume.
     */
    int InitReagents(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitReagents();
        return 0;
    }

    /**
     * Gives the bot its consumables.
     */
    int InitConsumables(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitConsumables();
        return 0;
    }

    /**
     * Gives the bot the potions matching its level.
     */
    int InitPotions(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitPotions();
        return 0;
    }

    /**
     * Gives the bot the food and the drinks matching its level.
     */
    int InitFood(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitFood();
        return 0;
    }

    /**
     * Teaches the bot the mounts matching its riding skill.
     */
    int InitMounts(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitMounts();
        return 0;
    }

    /**
     * Fills the keyring of the bot with the keys its level grants.
     */
    int InitKeyring(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitKeyring();
        return 0;
    }

    /**
     * Grants the bot the reputations its level grants.
     */
    int InitReputation(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitReputation();
        return 0;
    }

    /**
     * Completes the instance quests of the bot.
     */
    int InitInstanceQuests(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitInstanceQuests();
        return 0;
    }

    /**
     * Completes the attunement quests of the bot.
     */
    int InitAttunementQuests(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitAttunementQuests();
        return 0;
    }

    /**
     * Puts the bot in a guild.
     */
    int InitGuild(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->InitGuild();
        return 0;
    }

    /**
     * Releases every instance the bot is bound to.
     */
    int UnbindInstance(lua_State* /*L*/, PlayerbotFactory* factory)
    {
        factory->UnbindInstance();
        return 0;
    }

    /**
     * Returns the GUID of a random character usable as a bot.
     *
     * @return ObjectGuid guid
     */
    int GetRandomBot(lua_State* L, PlayerbotFactory* /*factory*/)
    {
        ALE::Push(L, PlayerbotFactory::GetRandomBot());
        return 1;
    }

    /**
     * Loads the static data the factory needs, quest and enchant tables for example.
     */
    int Init(lua_State* /*L*/, PlayerbotFactory* /*factory*/)
    {
        PlayerbotFactory::Init();
        return 0;
    }

    /**
     * Returns the gear score weighted by the given item quality.
     *
     * @param uint32 gearScore
     * @param uint32 quality
     * @return uint32 mixedGearScore
     */
    int CalcMixedGearScore(lua_State* L, PlayerbotFactory* /*factory*/)
    {
        uint32 gearScore = ALE::CHECKVAL<uint32>(L, 2);
        uint32 quality = ALE::CHECKVAL<uint32>(L, 3);

        ALE::Push(L, PlayerbotFactory::CalcMixedGearScore(gearScore, quality));
        return 1;
    }

    /**
     * Spends the talent points of the given [Player] following a numbered spec.
     *
     * @param [Player] bot
     * @param int32 specNo
     * @param bool reset = false
     */
    int InitTalentsBySpecNo(lua_State* L, PlayerbotFactory* /*factory*/)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        int specNo = ALE::CHECKVAL<int>(L, 3);
        bool reset = ALE::CHECKVAL<bool>(L, 4, false);

        PlayerbotFactory::InitTalentsBySpecNo(bot, specNo, reset);
        return 0;
    }

    /**
     * Spends the talent points of the given [Player] following a parsed spec link.
     *
     * @param [Player] bot
     * @param table specLink
     * @param bool reset = false
     */
    int InitTalentsByParsedSpecLink(lua_State* L, PlayerbotFactory* /*factory*/)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        luaL_checktype(L, 3, LUA_TTABLE);
        bool reset = ALE::CHECKVAL<bool>(L, 4, false);

        std::vector<std::vector<uint32>> parsed;

        lua_pushnil(L);
        while (lua_next(L, 3))
        {
            if (lua_istable(L, -1))
            {
                std::vector<uint32> tree;
                lua_pushnil(L);
                while (lua_next(L, -2))
                {
                    tree.push_back(static_cast<uint32>(lua_tonumber(L, -1)));
                    lua_pop(L, 1);
                }
                parsed.push_back(tree);
            }
            lua_pop(L, 1);
        }

        PlayerbotFactory::InitTalentsByParsedSpecLink(bot, parsed, reset);
        return 0;
    }

    /**
     * Destroys every item equipped by the given [Player].
     *
     * @param [Player] bot
     */
    int DestroyEquippedGear(lua_State* L, PlayerbotFactory* /*factory*/)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        PlayerbotFactory::DestroyEquippedGear(bot);
        return 0;
    }

    /**
     * Regears the given [Player] without building a factory first.
     *
     * @param [Player] bot
     * @param uint32 itemQuality
     * @param uint32 itemLevel
     * @param bool incremental = false
     * @param bool secondChance = false
     * @param bool applyFinishers = true : also apply the enchants and the gems
     */
    int AutoGear(lua_State* L, PlayerbotFactory* /*factory*/)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        uint32 itemQuality = ALE::CHECKVAL<uint32>(L, 3);
        uint32 itemLevel = ALE::CHECKVAL<uint32>(L, 4);
        bool incremental = ALE::CHECKVAL<bool>(L, 5, false);
        bool secondChance = ALE::CHECKVAL<bool>(L, 6, false);
        bool applyFinishers = ALE::CHECKVAL<bool>(L, 7, true);

        PlayerbotFactory::AutoGear(bot, itemQuality, itemLevel, incremental, secondChance, applyFinishers);
        return 0;
    }
};

namespace LuaPlayerBotFactoryGlobal
{
    /**
     * Builds a [PlayerbotFactory] for the given bot.
     *
     * @param [Player] bot
     * @param uint32 level
     * @param uint32 itemQuality = 0 : 0 to let the configuration decide
     * @param uint32 gearScoreLimit = 0 : 0 for no limit
     * @return [PlayerbotFactory] factory
     */
    int CreatePlayerbotFactory(lua_State* L)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 1);
        uint32 level = ALE::CHECKVAL<uint32>(L, 2);
        uint32 itemQuality = ALE::CHECKVAL<uint32>(L, 3, 0);
        uint32 gearScoreLimit = ALE::CHECKVAL<uint32>(L, 4, 0);

        ALE::Push(L, new PlayerbotFactory(bot, level, itemQuality, gearScoreLimit));
        return 1;
    }
};
#endif // MOD_PLAYERBOTS
#endif // PLAYERBOTFACTORYMETHODS_H
