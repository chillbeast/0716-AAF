// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <set>
#include <string>

namespace Ignite
{
    // Accessed under the engine's desktop state mutex.
    class DesktopChanges
    {
    public:
        void begin() { enabled=true; pending.clear(); }
        void edited(const std::string& key) { if(enabled && !key.empty()) pending.insert(key); }
        void saved(const std::string& key) { pending.erase(key); }
        bool dirty() const { return !pending.empty(); }
        void beginSave() { failed=false; }
        void writeFailed() { failed=true; }
        bool finishSave(bool success)
        {
            if(!success || failed) {pending.insert("save-error"); return false;}
            pending.clear(); return true;
        }
    private:
        bool enabled=false,failed=false;
        std::set<std::string> pending;
    };
}
