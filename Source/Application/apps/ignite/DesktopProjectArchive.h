// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <JuceHeader.h>
#include "Firmware/Application/Model/SafeFileIO.h"
#include "Firmware/Application/Model/SavedDataValidation.h"

namespace IgniteDesktop
{
    class ProjectArchive
    {
    public:
        static juce::File dataRoot() { return Ignite::SongManager::desktopSettingsFile().getParentDirectory().getChildFile("Ignite_folder"); }
        static juce::Result exportTo(const juce::File& root, const juce::File& target)
        {
            if (!root.isDirectory()) return juce::Result::fail("There is no project data to export.");
            juce::ZipFile::Builder archive;
            for (const auto& library : libraries())
            {
                auto folder=root.getChildFile(library);
                if (!folder.isDirectory()) return juce::Result::fail("Missing library: "+library);
                for (const auto& file : folder.findChildFiles(juce::File::findFiles,true))
                    archive.addFile(file,6,file.getRelativePathFrom(root).replaceCharacter('\\','/'));
            }
            auto info=root.getChildFile("info.txt");
            if (info.existsAsFile()) archive.addFile(info,6,"info.txt");
            const juce::String manifest="{\"format\":1,\"product\":\"IGNITE\"}";
            archive.addEntry(new juce::MemoryInputStream(manifest.toRawUTF8(),size_t(manifest.getNumBytesAsUTF8()),true),6,"ignite-backup.json",juce::Time::getCurrentTime());
            juce::TemporaryFile temporary(target);
            {
                juce::FileOutputStream stream(temporary.getFile());
                if (!stream.openedOk() || !archive.writeToStream(stream,nullptr)) return juce::Result::fail("Could not write the archive.");
                stream.flush();
                if (stream.getStatus().failed()) return stream.getStatus();
            }
            return temporary.overwriteTargetFileWithTemporary() ? juce::Result::ok() : juce::Result::fail("Could not replace the destination archive.");
        }
        static juce::Result restoreFrom(const juce::File& source, const juce::File& root)
        {
            juce::ZipFile archive(source);
            const int count=archive.getNumEntries();
            if (count == 0 || count > 200000) return juce::Result::fail("The file is not a supported Ignite backup.");
            juce::StringArray names;
            int manifest=-1;
            juce::int64 total=0;
            for (int i=0;i<count;++i)
            {
                const auto* entry=archive.getEntry(i);
                const auto name=entry->filename.replaceCharacter('\\','/');
                const auto parts=juce::StringArray::fromTokens(name,"/","");
                if (name.isEmpty() || name.startsWithChar('/') || name.containsChar(':') || parts.contains("..") || parts.contains(".") ||
                    names.contains(name,true) || entry->isSymbolicLink)
                    return juce::Result::fail("The archive contains an unsafe or duplicate path.");
                for (const auto& part : parts)
                    if (part.isNotEmpty() && (part != part.trim() || part.endsWithChar('.') || part.containsAnyOf("<>\"|?*")))
                        return juce::Result::fail("The archive contains an invalid path component.");
                if (entry->uncompressedSize < 0 || entry->uncompressedSize > juce::int64(8)*1024*1024*1024-total)
                    return juce::Result::fail("The backup exceeds the supported size.");
                names.add(name);
                total+=entry->uncompressedSize;
                if (total > juce::int64(8)*1024*1024*1024) return juce::Result::fail("The backup exceeds the supported size.");
                if (name == "ignite-backup.json") manifest=i;
                else if (name != "info.txt" && !libraries().contains(parts[0])) return juce::Result::fail("The archive contains an unknown library.");
            }
            if (manifest < 0) return juce::Result::fail("This is not an Ignite desktop backup.");
            std::unique_ptr<juce::InputStream> metadata(archive.createStreamForEntry(manifest));
            if (!metadata) return juce::Result::fail("Missing backup metadata.");
            auto description=juce::JSON::parse(metadata->readEntireStreamAsString());
            if (int(description.getProperty("format",0)) != 1 || description.getProperty("product","").toString() != "IGNITE")
                return juce::Result::fail("Unsupported backup format.");
            auto pending=root.getParentDirectory().getChildFile(".import-"+juce::Uuid().toString());
            if (pending.createDirectory().failed()) return juce::Result::fail("Cannot create the restore directory.");
            struct Cleanup { juce::File directory; ~Cleanup() { directory.deleteRecursively(); } } cleanup{pending};
            auto extracted=archive.uncompressTo(pending,false);
            if (extracted.failed()) return extracted;
            for (const auto& library : libraries())
            {
                auto folder=pending.getChildFile(library);
                // Empty libraries have no ZIP entries.
                if (folder.createDirectory().failed()) return juce::Result::fail("Cannot prepare a library directory.");
                for (const auto& file : folder.findChildFiles(juce::File::findFiles,true,"*.txt"))
                    if (!Ignite::validSavedXml(file)) return juce::Result::fail("A saved data file is not valid XML: "+file.getFileName());
            }
            auto mainInfo=pending.getChildFile("info.txt");
            if (mainInfo.existsAsFile() && !Ignite::validSavedXml(mainInfo)) return juce::Result::fail("The saved project index is not valid XML.");
            namespace fs=std::filesystem;
            const bool restored=Ignite::SafeFileIO::writeDirectory(fs::u8path(root.getFullPathName().toStdString()),
                fs::u8path(root.getParentDirectory().getChildFile(".desktop-restore").getFullPathName().toStdString()),
                [&](const fs::path& destination)
                {
                    for (const auto& library : libraries())
                        fs::copy(fs::u8path(pending.getChildFile(library).getFullPathName().toStdString()),destination/library.toStdString(),fs::copy_options::recursive);
                    auto file=pending.getChildFile("info.txt");
                    if (file.existsAsFile()) fs::copy_file(fs::u8path(file.getFullPathName().toStdString()),destination/"info.txt");
                    return true;
                });
            return restored ? juce::Result::ok() : juce::Result::fail("Restore failed; the previous project data has been retained.");
        }
        static juce::StringArray libraries() { return {"Songs","Instruments","Patterns","Settings"}; }
    };
}
