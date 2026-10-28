// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_core/juce_core.h>
namespace Ignite
{
    inline bool validSavedXml(const juce::File& file)
    {
        const auto text=file.loadFileAsString().trimEnd();
        if (text.containsIgnoreCase("<!DOCTYPE")) return false;
        juce::XmlDocument document(text);
        const auto root=document.getDocumentElement();
        if (!root || document.getLastParseError().isNotEmpty()) return false;
        static const juce::StringArray knownRoots{"BRayCnfgData","CalibOffset","CvioCnfgS","GeneralCnfg","Instrument",
            "MidiClkCnfgData","MidiCnfg","MidiMachineControlCnfgData","PatternCnfgData","RecordCnfgData",
            "RemoteControlCnfgData","SceneCnfData","SceneInfo","SculptModeCnfgData","SongCnfgData","Song_Info",
            "SyStemStatus","Track","TrackCnfData","UdsCnfgData","mainInfoData","pattern","pattern_bar","step"};
        if (!knownRoots.contains(root->getTagName())) return false;
        // JUCE's parser can return an element for an unterminated opening tag.
        // Files produced by the firmware always have a complete closing tag.
        return text.endsWith("/>") || text.endsWith("</"+root->getTagName()+">");
    }
}
