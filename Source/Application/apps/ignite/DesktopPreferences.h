// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <JuceHeader.h>

namespace IgniteDesktop
{
    inline juce::var readPreferences()
    {
        auto value = juce::JSON::parse(Ignite::SongManager::desktopSettingsFile());
        return value.isObject() ? value : juce::var(new juce::DynamicObject());
    }
    inline bool writePreferences(juce::var value)
    {
        auto file = Ignite::SongManager::desktopSettingsFile();
        return file.getParentDirectory().createDirectory().wasOk() && file.replaceWithText(juce::JSON::toString(value));
    }
}
