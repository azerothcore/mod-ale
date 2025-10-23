/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEFileWatcher.h"
#include "LuaEngine.h"
#include "ALEUtility.h"
#include <boost/filesystem.hpp>

ALEFileWatcher::ALEFileWatcher() : running(false), checkInterval(1)
{
}

ALEFileWatcher::~ALEFileWatcher()
{
    StopWatching();
}

void ALEFileWatcher::StartWatching(const std::string& scriptPath, uint32 intervalSeconds)
{
    if (running.load())
    {
        ALE_LOG_DEBUG("[ALEFileWatcher]: Already watching files");
        return;
    }

    if (scriptPath.empty())
    {
        ALE_LOG_ERROR("[ALEFileWatcher]: Cannot start watching - script path is empty");
        return;
    }

    watchPath = scriptPath;
    checkInterval = intervalSeconds;
    running.store(true);

    ScanDirectory(watchPath);

    watcherThread = std::thread(&ALEFileWatcher::WatchLoop, this);
    
    ALE_LOG_INFO("[ALEFileWatcher]: Started watching '{}' (interval: {}s)", watchPath, checkInterval);
}

void ALEFileWatcher::StopWatching()
{
    if (!running.load())
        return;

    running.store(false);

    if (watcherThread.joinable())
        watcherThread.join();

    fileTimestamps.clear();
    
    ALE_LOG_INFO("[ALEFileWatcher]: Stopped watching files");
}

void ALEFileWatcher::WatchLoop()
{
    while (running.load())
    {
        try
        {
            CheckForChanges();
        }
        catch (const std::exception& e)
        {
            ALE_LOG_ERROR("[ALEFileWatcher]: Error during file watching: {}", e.what());
        }

        std::this_thread::sleep_for(std::chrono::seconds(checkInterval));
    }
}

bool ALEFileWatcher::IsWatchedFileType(const std::string& filename) {
    return (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".lua") ||
        (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".ext") ||
        (filename.length() >= 5 && filename.substr(filename.length() - 5) == ".moon");
}

void ALEFileWatcher::ScanDirectory(const std::string& path)
{
    try
    {
        boost::filesystem::path dir(path);
        
        if (!boost::filesystem::exists(dir) || !boost::filesystem::is_directory(dir))
            return;

        boost::filesystem::directory_iterator end_iter;
        
        for (boost::filesystem::directory_iterator dir_iter(dir); dir_iter != end_iter; ++dir_iter)
        {
            std::string fullpath = dir_iter->path().generic_string();
            
            if (boost::filesystem::is_directory(dir_iter->status()))
            {
                ScanDirectory(fullpath);
            }
            else if (boost::filesystem::is_regular_file(dir_iter->status()))
            {
                std::string filename = dir_iter->path().filename().generic_string();
                
                if (IsWatchedFileType(filename))
                {
                    fileTimestamps[fullpath] = boost::filesystem::last_write_time(dir_iter->path());
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        ALE_LOG_ERROR("[ALEFileWatcher]: Error scanning directory '{}': {}", path, e.what());
    }
}

void ALEFileWatcher::CheckForChanges()
{
    bool hasChanges = false;
    
    try
    {
        boost::filesystem::path dir(watchPath);
        
        if (!boost::filesystem::exists(dir) || !boost::filesystem::is_directory(dir))
            return;

        boost::filesystem::directory_iterator end_iter;
        
        for (boost::filesystem::directory_iterator dir_iter(dir); dir_iter != end_iter; ++dir_iter)
        {
            if (ShouldReloadFile(dir_iter->path().generic_string()))
                hasChanges = true;
        }
        
        for (auto it = fileTimestamps.begin(); it != fileTimestamps.end();)
        {
            if (!boost::filesystem::exists(it->first))
            {
                ALE_LOG_DEBUG("[ALEFileWatcher]: File deleted: {}", it->first);
                it = fileTimestamps.erase(it);
                hasChanges = true;
            }
            else
            {
                ++it;
            }
        }
    }
    catch (const std::exception& e)
    {
        ALE_LOG_ERROR("[ALEFileWatcher]: Error checking for changes: {}", e.what());
        return;
    }

    if (hasChanges)
    {
        ALE_LOG_INFO("[ALEFileWatcher]: Lua script changes detected - triggering reload");
        ALE::ReloadALE();
        
        ScanDirectory(watchPath);
    }
}

bool ALEFileWatcher::ShouldReloadFile(const std::string& filepath)
{
    try
    {
        boost::filesystem::path file(filepath);
        
        if (boost::filesystem::is_directory(file))
        {
            boost::filesystem::directory_iterator end_iter;
            
            for (boost::filesystem::directory_iterator dir_iter(file); dir_iter != end_iter; ++dir_iter)
            {
                if (ShouldReloadFile(dir_iter->path().generic_string()))
                    return true;
            }
            return false;
        }
        
        if (!boost::filesystem::is_regular_file(file))
            return false;
            
        std::string filename = file.filename().generic_string();

        if (!IsWatchedFileType(filename)) return false;
            
        auto currentTime = boost::filesystem::last_write_time(file);
        auto it = fileTimestamps.find(filepath);
        
        if (it == fileTimestamps.end())
        {
            ALE_LOG_DEBUG("[ALEFileWatcher]: New file detected: {}", filepath);
            fileTimestamps[filepath] = currentTime;
            return true;
        }
        
        if (it->second != currentTime)
        {
            ALE_LOG_DEBUG("[ALEFileWatcher]: File modified: {}", filepath);
            it->second = currentTime;
            return true;
        }
    }
    catch (const std::exception& e)
    {
        ALE_LOG_ERROR("[ALEFileWatcher]: Error checking file '{}': {}", filepath, e.what());
    }
    
    return false;
}
