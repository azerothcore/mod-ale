/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "FileSystemUtils.h"
#include <filesystem>
#include <sys/stat.h>

namespace ALE::Utils
{
    std::time_t FileSystemUtils::GetFileModTime(const std::string& filepath)
    {
        struct stat fileInfo;
        if (stat(filepath.c_str(), &fileInfo) == 0)
            return fileInfo.st_mtime;
        return 0;
    }

    bool FileSystemUtils::IsExtFile(const std::string& filepath)
    {
        return HasExtension(filepath, ".ext");
    }

    bool FileSystemUtils::IsMoonScriptFile(const std::string& filepath)
    {
        return HasExtension(filepath, ".moon");
    }

    bool FileSystemUtils::IsCoutFile(const std::string& filepath)
    {
        return HasExtension(filepath, ".cout");
    }

    bool FileSystemUtils::IsScriptFile(const std::string& filepath)
    {
        std::string ext = GetExtension(filepath);
        return ext == ".ext" || ext == ".lua" || ext == ".moon" || ext == ".cout";
    }

    std::string FileSystemUtils::GetExtension(const std::string& filepath)
    {
        std::filesystem::path path(filepath);
        return path.extension().string();
    }

    bool FileSystemUtils::HasExtension(const std::string& filepath, const std::string& extension)
    {
        return GetExtension(filepath) == extension;
    }

} // namespace ALE::Utils
