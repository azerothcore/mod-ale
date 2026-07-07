/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "QueryResult.h"
#include "StringFormat.h"

/***
 * The result of a database query.
 *
 * E.g. the return value of [Global:WorldDBQuery].
 *
 * Inherits all methods from: none
 */
namespace LuaALEQuery
{
    static void CheckFields(ResultSet* resultset, uint32 field)
    {
        uint32 count = resultset->GetFieldCount();
        if (field >= count)
            throw std::invalid_argument(Acore::StringFormat("trying to access invalid field index {}. There are {} fields available and the indexes start from 0", field, count));
    }

    /**
     * Returns `true` if the specified column of the current row is `NULL`, otherwise `false`.
     *
     * @param uint32 column
     * @return bool isNull
     */
    bool IsNull(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);

        return resultset->Fetch()[col].IsNull();
    }

    /**
     * Returns the number of columns in the result set.
     *
     * @return uint32 columnCount
     */
    uint32 GetColumnCount(ResultSet* resultset)
    {
        return resultset->GetFieldCount();
    }

    /**
     * Returns the number of rows in the result set.
     *
     * @return uint32 rowCount
     */
    uint32 GetRowCount(ResultSet* resultset)
    {
        if (resultset->GetRowCount() > (uint32)-1)
            return (uint32)-1;

        return (uint32)(resultset->GetRowCount());
    }

    /**
     * Returns the data in the specified column of the current row, casted to a boolean.
     *
     * @param uint32 column
     * @return bool data
     */
    bool GetBool(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<bool>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to an unsigned 8-bit integer.
     *
     * @param uint32 column
     * @return uint8 data
     */
    uint8 GetUInt8(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<uint8>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to an unsigned 16-bit integer.
     *
     * @param uint32 column
     * @return uint16 data
     */
    uint16 GetUInt16(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<uint16>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to an unsigned 32-bit integer.
     *
     * @param uint32 column
     * @return uint32 data
     */
    uint32 GetUInt32(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<uint32>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to an unsigned 64-bit integer.
     *
     * @param uint32 column
     * @return uint64 data
     */
    uint64 GetUInt64(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<uint64>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to a signed 8-bit integer.
     *
     * @param uint32 column
     * @return int8 data
     */
    int8 GetInt8(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<int8>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to a signed 16-bit integer.
     *
     * @param uint32 column
     * @return int16 data
     */
    int16 GetInt16(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<int16>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to a signed 32-bit integer.
     *
     * @param uint32 column
     * @return int32 data
     */
    int32 GetInt32(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<int32>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to a signed 64-bit integer.
     *
     * @param uint32 column
     * @return int64 data
     */
    int64 GetInt64(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<int64>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to a 32-bit floating point value.
     *
     * @param uint32 column
     * @return float data
     */
    float GetFloat(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<float>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to a 64-bit floating point value.
     *
     * @param uint32 column
     * @return double data
     */
    double GetDouble(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<double>();
    }

    /**
     * Returns the data in the specified column of the current row, casted to a string.
     *
     * @param uint32 column
     * @return string data
     */
    std::string GetString(ResultSet* resultset, uint32 col)
    {
        CheckFields(resultset, col);
        return resultset->Fetch()[col].Get<std::string>();
    }

    /**
     * Advances the [ALEQuery] to the next row in the result set.
     *
     * *Do not* call this immediately after a query, or you'll skip the first row.
     *
     * Returns `false` if there was no new row, otherwise `true`.
     *
     * @return bool hadNextRow
     */
    bool NextRow(ResultSet* resultset)
    {
        return resultset->NextRow();
    }

    /**
     * Returns a table from the current row where keys are field names and values are the row's values.
     *
     * All numerical values will be numbers and everything else is returned as a string.
     *
     * **For example,** the query:
     *
     *     SELECT entry, name FROM creature_template
     *
     * would result in a table like:
     *
     *     { entry = 123, name = "some creature name" }
     *
     * To move to next row use [ALEQuery:NextRow].
     *
     * @return table rowData : table filled with row columns and data where `T[column] = data`
     */
    sol::table GetRow(ResultSet* resultset, sol::this_state s)
    {
        uint32 col = resultset->GetFieldCount();
        Field* row = resultset->Fetch();

        sol::table tbl = sol::state_view(s).create_table(0, col);

        for (uint32 i = 0; i < col; ++i)
        {
            std::string fieldName = resultset->GetFieldName(i);

            std::string _str = row[i].Get<std::string>();
            char const* str = _str.c_str();
            if (row[i].IsNull() || !str)
                tbl[fieldName] = sol::lua_nil;
            else
            {
                // MYSQL_TYPE_LONGLONG Interpreted as string for lua
                switch (row[i].GetType())
                {
                    case DatabaseFieldTypes::Int8:
                    case DatabaseFieldTypes::Int16:
                    case DatabaseFieldTypes::Int32:
                    case DatabaseFieldTypes::Int64:
                    case DatabaseFieldTypes::Float:
                    case DatabaseFieldTypes::Double:
                        tbl[fieldName] = strtod(str, NULL);
                        break;
                    default:
                        tbl[fieldName] = str;
                        break;
                }
            }
        }

        return tbl;
    }
}

void RegisterALEQueryMethods(sol::state& lua)
{
    sol::usertype<ResultSet> type = lua.new_usertype<ResultSet>("ALEQuery", sol::no_constructor);

    type["IsNull"]         = &LuaALEQuery::IsNull;
    type["GetColumnCount"] = &LuaALEQuery::GetColumnCount;
    type["GetRowCount"]    = &LuaALEQuery::GetRowCount;
    type["GetBool"]        = &LuaALEQuery::GetBool;
    type["GetUInt8"]       = &LuaALEQuery::GetUInt8;
    type["GetUInt16"]      = &LuaALEQuery::GetUInt16;
    type["GetUInt32"]      = &LuaALEQuery::GetUInt32;
    type["GetUInt64"]      = &LuaALEQuery::GetUInt64;
    type["GetInt8"]        = &LuaALEQuery::GetInt8;
    type["GetInt16"]       = &LuaALEQuery::GetInt16;
    type["GetInt32"]       = &LuaALEQuery::GetInt32;
    type["GetInt64"]       = &LuaALEQuery::GetInt64;
    type["GetFloat"]       = &LuaALEQuery::GetFloat;
    type["GetDouble"]      = &LuaALEQuery::GetDouble;
    type["GetString"]      = &LuaALEQuery::GetString;
    type["NextRow"]        = &LuaALEQuery::NextRow;
    type["GetRow"]         = &LuaALEQuery::GetRow;
}
