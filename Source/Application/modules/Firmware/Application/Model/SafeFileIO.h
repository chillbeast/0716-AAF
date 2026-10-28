// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <system_error>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Ignite::SafeFileIO
{
    namespace fs = std::filesystem;

    inline bool isLeafName(const std::string& name)
    {
        return !name.empty() && name != "." && name != ".." &&
               name.find_first_of("/\\:") == std::string::npos;
    }

    inline std::string uniqueSuffix()
    {
        static std::atomic<uint64_t> sequence{0};
        return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
               "-" + std::to_string(++sequence);
    }

    template <class Writer>
    bool writeFile(const fs::path& target, Writer writer)
    {
        auto temporary = target;
        temporary += ".pending-" + uniqueSuffix();
        std::error_code ignored;
        try
        {
            std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
            if (!stream) return false;
            writer(stream);
            stream.flush();
            const bool written = stream.good();
            stream.close();
            if (written && !stream.fail())
            {
#ifdef _WIN32
                if (MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                    return true;
#else
                fs::rename(temporary, target);
                return true;
#endif
            }
        }
        catch (...) {}
        fs::remove(temporary, ignored);
        return false;
    }

    // Stage a complete directory before moving the old version to a recoverable backup.
    // Keep the most recent backup until the next successful staging operation.
    template <class Writer>
    bool writeDirectory(const fs::path& target, const fs::path& recoveryRoot, Writer writer)
    {
        const auto backup = recoveryRoot / "backups" / target.filename();
        const auto pending = recoveryRoot / "staging" / uniqueSuffix();
        std::error_code ignored;
        try
        {
            fs::create_directories(backup.parent_path());
            fs::create_directories(pending);
            if (fs::exists(backup) && !fs::exists(target)) fs::rename(backup, target);
            if (!writer(pending))
            {
                fs::remove_all(pending, ignored);
                return false;
            }
            if (fs::exists(target))
            {
                fs::remove_all(backup);
                fs::rename(target, backup);
            }
            try { fs::rename(pending, target); }
            catch (...)
            {
                if (!fs::exists(target) && fs::exists(backup)) fs::rename(backup, target);
                throw;
            }
            return true;
        }
        catch (...)
        {
            fs::remove_all(pending, ignored);
            return false;
        }
    }
}
