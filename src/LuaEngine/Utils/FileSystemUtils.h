/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _FILESYSTEM_UTILS_H
#define _FILESYSTEM_UTILS_H

#include <string>
#include <ctime>
#include "Common.h"

namespace ALE::Utils
{
    /**
     * @class FileSystemUtils
     * @brief Centralized file system utilities for script handling
     */
    class FileSystemUtils
    {
    public:
        /**
         * @brief Get file modification timestamp
         * @param filepath Path to file
         * @return Unix timestamp, 0 if file doesn't exist
         */
        static std::time_t GetFileModTime(const std::string& filepath);

        /**
         * @brief Check if file has .ext extension
         * @param filepath File path to check
         * @return true if extension is .ext
         */
        static bool IsExtFile(const std::string& filepath);

        /**
         * @brief Check if file has .moon extension
         * @param filepath File path to check
         * @return true if extension is .moon
         */
        static bool IsMoonScriptFile(const std::string& filepath);

        /**
         * @brief Check if file has .cout extension
         * @param filepath File path to check
         * @return true if extension is .cout
         */
        static bool IsCoutFile(const std::string& filepath);

        /**
         * @brief Check if file has valid script extension
         * @param filepath File path to check
         * @return true if extension is .ext, .lua, .moon, or .cout
         */
        static bool IsScriptFile(const std::string& filepath);

        /**
         * @brief Get file extension including the dot
         * @param filepath File path
         * @return Extension string (e.g., ".lua"), empty if no extension
         */
        static std::string GetExtension(const std::string& filepath);

    private:
        /**
         * @brief Check if file has specific extension
         * @param filepath File path to check
         * @param extension Extension to match (e.g., ".moon")
         * @return true if extension matches
         */
        static bool HasExtension(const std::string& filepath, const std::string& extension);
    };

} // namespace ALE::Utils

#endif // _FILESYSTEM_UTILS_H
