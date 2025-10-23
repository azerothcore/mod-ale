#include "ALEDBCRegistry.h"

std::vector<DBCDefinition> dbcRegistry = {
    REGISTER_DBC(GemProperties, GemPropertiesEntry, sGemPropertiesStore),
    REGISTER_DBC(Spell,         SpellEntry,         sSpellStore),
};

