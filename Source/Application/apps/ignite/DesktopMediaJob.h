// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopMediaSnapshot.h"
#include <windows.h>

namespace IgniteDesktop::Media
{
    // Each export runs in a child with its own data root, engine, MIDI state and voices.
    class Job : private juce::Timer
    {
    public:
        ~Job() override { stop(); }
        juce::Result start(std::unique_ptr<juce::XmlElement> snapshot,const juce::File& target,std::function<void(juce::Result)> callback)
        {
            if(process)return juce::Result::fail("An export is already running.");
            directory=juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("IgniteExport-"+juce::Uuid().toString());
            if(directory.createDirectory().failed())return juce::Result::fail("Cannot create the export workspace.");
            destination=target;pending=target.getSiblingFile("."+target.getFileName()+".pending-"+juce::Uuid().toString());
            {auto stream=pending.createOutputStream();if(!stream){cleanup();return juce::Result::fail("The output folder is not writable.");}}
            snapshot->setAttribute("output",pending.getFullPathName());const auto job=directory.getChildFile("job.xml");
            if(!snapshot->writeTo(job)){cleanup();return juce::Result::fail("Cannot write the export snapshot.");}
            std::map<std::wstring,std::wstring> overrides{
                {L"IGNITE_DATA_DIR",directory.getChildFile("data").getFullPathName().toWideCharPointer()},
                {L"IGNITE_LOG",directory.getChildFile("render.log").getFullPathName().toWideCharPointer()},
                {L"IGNITE_SOUNDFONT",snapshot->getStringAttribute("kind")!="midi" && snapshot->getStringAttribute("font").isNotEmpty()?std::wstring(snapshot->getStringAttribute("font").toWideCharPointer()):std::wstring(directory.getChildFile("missing-default.sf2").getFullPathName().toWideCharPointer())},
                {L"IGNITE_HEADLESS_RENDER",L"1"}};
            std::vector<std::wstring> entries;
            if(auto* environment=GetEnvironmentStringsW())
            {
                for(auto* item=environment;*item;item+=wcslen(item)+1)
                {std::wstring value(item);bool replaced=false;for(const auto& entry:overrides)if(_wcsnicmp(value.c_str(),(entry.first+L"=").c_str(),entry.first.size()+1)==0)replaced=true;if(!replaced)entries.push_back(std::move(value));}
                FreeEnvironmentStringsW(environment);
            }
            for(const auto& entry:overrides)entries.push_back(entry.first+L"="+entry.second);
            std::sort(entries.begin(),entries.end(),[](const auto& a,const auto& b){return _wcsicmp(a.c_str(),b.c_str())<0;});
            std::vector<wchar_t> environment;for(const auto& entry:entries){environment.insert(environment.end(),entry.begin(),entry.end());environment.push_back(0);}environment.push_back(0);
            const auto executable=juce::File::getSpecialLocation(juce::File::currentExecutableFile);
            std::wstring command((executable.getFullPathName().quoted()+" --render-job "+job.getFullPathName().quoted()).toWideCharPointer());
            STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION info{};
            if(!CreateProcessW(executable.getFullPathName().toWideCharPointer(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW|CREATE_UNICODE_ENVIRONMENT,environment.data(),executable.getParentDirectory().getFullPathName().toWideCharPointer(),&startup,&info))
            {cleanup();return juce::Result::fail("Cannot start the isolated export process.");}
            CloseHandle(info.hThread);process=info.hProcess;completion=std::move(callback);cancelled=false;startTimer(100);return juce::Result::ok();
        }
        bool running() const {return process!=nullptr;}
        void cancel(){if(process){cancelled=true;TerminateProcess(process,2);}}
        juce::String progress() const {return directory.getChildFile("progress.txt").loadFileAsString();}
        const juce::File& workspace() const {return directory;}
    private:
        void stop()
        {
            stopTimer();if(process){TerminateProcess(process,2);WaitForSingleObject(process,5000);CloseHandle(process);process=nullptr;}cleanup();
        }
        void cleanup()
        {
            if(pending!=juce::File{})pending.deleteFile();
            if(directory.getFileName().startsWith("IgniteExport-"))directory.deleteRecursively();
        }
        void timerCallback() override
        {
            if(!process || WaitForSingleObject(process,0)!=WAIT_OBJECT_0)return;
            stopTimer();DWORD code=1;GetExitCodeProcess(process,&code);CloseHandle(process);process=nullptr;
            auto result=juce::XmlDocument::parse(directory.getChildFile("result.xml"));
            juce::Result outcome=juce::Result::fail(cancelled?"Export cancelled. The previous file was kept.":result?result->getStringAttribute("error","Export failed; the previous file was kept."):"Export process failed; the previous file was kept.");
            if(!cancelled && code==0 && result && result->getBoolAttribute("success") && pending.existsAsFile())
                outcome=MoveFileExW(pending.getFullPathName().toWideCharPointer(),destination.getFullPathName().toWideCharPointer(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)?juce::Result::ok():juce::Result::fail("Cannot replace the destination file. The previous file was kept.");
            cleanup();auto callback=std::move(completion);if(callback)callback(outcome);
        }
        HANDLE process=nullptr;
        bool cancelled=false;
        juce::File directory,pending,destination;
        std::function<void(juce::Result)> completion;
    };
}
