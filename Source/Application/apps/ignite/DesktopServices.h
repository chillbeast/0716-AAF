// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <JuceHeader.h>
#include <map>
#include <memory>
#include <atomic>
#include <functional>
#include "Firmware/Application/Model/SoundFontProgram.h"
#include "DesktopProjectArchive.h"
#include "DesktopPreferences.h"

namespace IgniteDesktop
{
    class MidiRouter
    {
        struct Input : juce::MidiInputCallback
        {
            Input(MidiRouter& r, int p) : router(r), port(p) {}
            void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage& message) override
            {
                if (!router.stopping) router.receive(port, message);
            }
            MidiRouter& router;
            int port;
            std::unique_ptr<juce::MidiInput> device;
        };
    public:
        explicit MidiRouter(Ignite::IgniteEngine& e) : engine(e)
        {
            auto lock = engine.lockDesktopState();
            engine.songManager->desktopMidiOutput = [this](const juce::MidiMessage& msg, uint8_t port)
            {
                std::lock_guard<std::mutex> guard(outputMutex);
                auto found = outputs.find(port);
                if (found == outputs.end()) return false;
                found->second->sendMessageNow(msg);
                return true;
            };
        }
        ~MidiRouter()
        {
            stopping = true;
            {
                auto lock = engine.lockDesktopState();
                engine.songManager->desktopMidiOutput = {};
            }
            // Closing an input joins its callback; never hold the model lock here.
            inputs.clear();
            outputs.clear();
        }
        void receive(int port, const juce::MidiMessage& message)
        {
            if (!stopping) engine.songManager->receiveDesktopMidi(uint8_t(port), message);
        }
        juce::Result bindInput(int port, const juce::String& identifier)
        {
            if (identifier == inputId(port)) return juce::Result::ok();
            std::unique_ptr<Input> replacement;
            if (identifier.isNotEmpty())
            {
                replacement = std::make_unique<Input>(*this, port);
                replacement->device = juce::MidiInput::openDevice(identifier, replacement.get());
                if (!replacement->device) return juce::Result::fail("Cannot open this MIDI input. Check whether another app is using it.");
            }
            inputs.erase(port);
            if (replacement)
            {
                replacement->device->start();
                inputs[port] = std::move(replacement);
            }
            inputIds[port] = identifier;
            return juce::Result::ok();
        }
        juce::Result bindOutput(int port, const juce::String& identifier)
        {
            if (identifier == outputId(port)) return juce::Result::ok();
            std::unique_ptr<juce::MidiOutput> replacement;
            if (identifier.isNotEmpty())
            {
                replacement = juce::MidiOutput::openDevice(identifier);
                if (!replacement) return juce::Result::fail("Cannot open this MIDI output.");
            }
            {
                std::lock_guard<std::mutex> guard(outputMutex);
                outputs.erase(port);
                if (replacement) outputs[port] = std::move(replacement);
            }
            outputIds[port] = identifier;
            return juce::Result::ok();
        }
        juce::String inputId(int port) const { auto it = inputIds.find(port); return it == inputIds.end() ? juce::String{} : it->second; }
        juce::String outputId(int port) const { auto it = outputIds.find(port); return it == outputIds.end() ? juce::String{} : it->second; }
        void restore()
        {
            auto settings = readPreferences();
            engine.songManager->desktopMonitorEnabled = bool(settings.getProperty("MonitorInput", true));
            if (auto* routes = settings.getProperty("MidiRoutes", {}).getArray())
                for (const auto& route : *routes)
                {
                    const int port = int(route.getProperty("port", -1));
                    if (!validPort(port)) continue;
                    auto in = bindInput(port, route.getProperty("input", "").toString());
                    auto out = bindOutput(port, route.getProperty("output", "").toString());
                    if (in.failed()) juce::Logger::writeToLog(in.getErrorMessage());
                    if (out.failed()) juce::Logger::writeToLog(out.getErrorMessage());
                }
        }
        bool save()
        {
            auto settings = readPreferences();
            juce::Array<juce::var> routes;
            for (int port : ports())
            {
                auto* route = new juce::DynamicObject();
                route->setProperty("port", port);
                route->setProperty("input", inputId(port));
                route->setProperty("output", outputId(port));
                routes.add(juce::var(route));
            }
            settings.getDynamicObject()->setProperty("MidiRoutes", routes);
            settings.getDynamicObject()->setProperty("MonitorInput", engine.songManager->desktopMonitorEnabled);
            return writePreferences(settings);
        }
        static std::vector<int> ports() { return {0,1,2,3,4,13,14,15,16,17,18,19,20,21,22,23}; }
        static bool validPort(int port) { return (port >= 0 && port <= 4) || (port >= 13 && port <= 23); }
        static juce::String portName(int port)
        {
            if (port < 5) return "DIN " + juce::String(port + 1);
            if (port < 19) return "USB device " + juce::String(port - 12);
            return "USB host " + juce::String(port - 18);
        }
        Ignite::IgniteEngine& engine;
    private:
        std::atomic<bool> stopping{false};
        std::map<int, std::unique_ptr<Input>> inputs;
        std::map<int, std::unique_ptr<juce::MidiOutput>> outputs;
        std::map<int, juce::String> inputIds, outputIds;
        std::mutex outputMutex;
    };

    class MidiSettings : public juce::Component
    {
        struct Row
        {
            int port;
            juce::Label label;
            juce::ComboBox input, output;
            juce::StringArray inputIds, outputIds;
        };
    public:
        explicit MidiSettings(MidiRouter& r) : router(r)
        {
            addAndMakeVisible(title); title.setText("Map your Windows devices to the ports used by instrument and clock settings.", juce::dontSendNotification);
            addAndMakeVisible(monitor); monitor.setToggleState(router.engine.songManager->desktopMonitorEnabled, juce::dontSendNotification);
            monitor.onClick = [this] { auto lock = router.engine.lockDesktopState(); router.engine.songManager->desktopMonitorEnabled = monitor.getToggleState(); router.engine.songManager->desktopPanic(); router.save(); };
            addAndMakeVisible(refresh); refresh.onClick = [this] { refreshDevices(); };
            addAndMakeVisible(status);
            addAndMakeVisible(inputHeading); addAndMakeVisible(outputHeading);
            inputHeading.setText("Windows input",juce::dontSendNotification); outputHeading.setText("Windows output",juce::dontSendNotification);
            for (int port : MidiRouter::ports())
            {
                auto row = std::make_unique<Row>(); row->port = port;
                addAndMakeVisible(row->label); row->label.setText(MidiRouter::portName(port), juce::dontSendNotification);
                addAndMakeVisible(row->input); addAndMakeVisible(row->output);
                auto* p = row.get();
                row->input.onChange = [this,p]
                {
                    auto result = router.bindInput(p->port, p->inputIds[p->input.getSelectedId() - 1]);
                    status.setText(result.wasOk() ? "Input mapping saved." : result.getErrorMessage(), juce::dontSendNotification);
                    if (result.wasOk()) router.save(); else refreshDevices();
                };
                row->output.onChange = [this,p]
                {
                    auto result = router.bindOutput(p->port, p->outputIds[p->output.getSelectedId() - 1]);
                    status.setText(result.wasOk() ? "Output mapping saved." : result.getErrorMessage(), juce::dontSendNotification);
                    if (result.wasOk()) router.save(); else refreshDevices();
                };
                rows.push_back(std::move(row));
            }
            refreshDevices();
            setSize(720, 680);
        }
        void resized() override
        {
            title.setBounds(12, 8, getWidth()-24, 38); monitor.setBounds(12, 48, 440, 26); refresh.setBounds(getWidth()-166, 48, 150, 26);
            inputHeading.setBounds(140,76,260,20); outputHeading.setBounds(410,76,getWidth()-426,20);
            int y=98;
            for (auto& row : rows) { row->label.setBounds(12,y,126,28); row->input.setBounds(140,y,260,28); row->output.setBounds(410,y,getWidth()-426,28); y+=32; }
            status.setBounds(12,y+8,getWidth()-24,50);
        }
    private:
        void refreshDevices()
        {
            const auto inputs = juce::MidiInput::getAvailableDevices(), outputs = juce::MidiOutput::getAvailableDevices();
            for (auto& row : rows)
            {
                auto fill = [](juce::ComboBox& combo, juce::StringArray& ids, const auto& devices, const juce::String& current)
                {
                    combo.clear(juce::dontSendNotification); ids.clear(); ids.add(""); combo.addItem("None",1);
                    for (const auto& device : devices) { ids.add(device.identifier); combo.addItem(device.name,ids.size()); }
                    if (current.isNotEmpty() && !ids.contains(current)) { ids.add(current); combo.addItem("Disconnected device",ids.size()); }
                    combo.setSelectedId(ids.indexOf(current)+1,juce::dontSendNotification);
                };
                fill(row->input,row->inputIds,inputs,router.inputId(row->port)); fill(row->output,row->outputIds,outputs,router.outputId(row->port));
            }
            if (inputs.isEmpty()) status.setText("No MIDI inputs connected. Use Refresh after connecting a controller or loopback port.",juce::dontSendNotification);
        }
        MidiRouter& router;
        juce::Label title, status, inputHeading, outputHeading;
        juce::ToggleButton monitor{"Hear incoming notes through the selected track"};
        juce::TextButton refresh{"Refresh devices"};
        std::vector<std::unique_ptr<Row>> rows;
    };

    class SoundFontSettings : public juce::Component, private juce::ListBoxModel
    {
    public:
        explicit SoundFontSettings(Ignite::IgniteEngine& e) : engine(e), list("Presets",this)
        {
            for (auto* child : std::initializer_list<juce::Component*>{&bank,&load,&search,&list,&instruments,&assign,&preview,&status}) addAndMakeVisible(child);
            load.onClick = [this] { chooseBank(); };
            search.setTextToShowWhenEmpty("Search preset name, bank or program",juce::Colours::grey);
            search.onTextChange = [this] { filter(); };
            assign.onClick = [this] { assignPreset(); };
            preview.onClick = [this] { previewPreset(); };
            refresh(); setSize(720,580);
        }
        ~SoundFontSettings() override { stopPreview(); }
        void resized() override
        {
            bank.setBounds(12,8,getWidth()-180,42); load.setBounds(getWidth()-160,14,146,28);
            search.setBounds(12,56,getWidth()-180,28); preview.setBounds(getWidth()-158,56,144,28); list.setBounds(12,94,getWidth()-24,getHeight()-204);
            instruments.setBounds(12,getHeight()-100,getWidth()-220,30); assign.setBounds(getWidth()-198,getHeight()-100,184,30);
            status.setBounds(12,getHeight()-60,getWidth()-24,50);
        }
        void refresh()
        {
            presets = engine.songManager->getDesktopSoundFontPresets();
            bank.setText(juce::String::fromUTF8(engine.songManager->desktopSoundFontPath.c_str()),juce::dontSendNotification);
            instruments.clear(juce::dontSendNotification); instrumentNames.clear();
            auto lock = engine.lockDesktopState();
            for (auto& instrument : engine.instrumentManager->instrumentList)
            {
                instrumentNames.add(instrument.getName()); instruments.addItem(instrument.getDispName(),instrumentNames.size());
            }
            if (!instrumentNames.isEmpty()) instruments.setSelectedId(1,juce::dontSendNotification);
            instruments.setTextWhenNothingSelected("Create an instrument in the firmware UI first"); filter();
        }
    private:
        int getNumRows() override { return int(filtered.size()); }
        void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override
        {
            if (!juce::isPositiveAndBelow(row,filtered.size())) return;
            if (selected) g.fillAll(juce::Colour(0xff683747));
            const auto& preset = presets[size_t(filtered[size_t(row)])];
            g.setColour(juce::Colours::white); g.setFont(14);
            g.drawText(juce::String(preset.bank)+":"+juce::String(preset.program)+"  "+juce::String(preset.name),8,0,width-16,height,juce::Justification::centredLeft);
        }
        void listBoxItemDoubleClicked(int,const juce::MouseEvent&) override { previewPreset(); }
        void stopPreview()
        {
            ++previewGeneration;
            if (!previewing) return;
            auto lock=engine.lockDesktopState();
            engine.songManager->desktopStopPreview();
            previewing=false;
        }
        void previewPreset()
        {
            const int row=list.getSelectedRow();
            if (!juce::isPositiveAndBelow(row,filtered.size())) return;
            if (engine.songManager->getSystemState()!=SYSTEM_STATE::IDLE)
            { status.setText("Stop playback and recording before previewing a preset.",juce::dontSendNotification); return; }
            stopPreview();
            auto lock=engine.lockDesktopState(); auto& songs=*engine.songManager;
            const auto& preset=presets[size_t(filtered[size_t(row)])];
            if (!songs.desktopPreviewPreset(preset.index)) return;
            previewing=true;
            const auto generation=++previewGeneration;
            juce::Component::SafePointer<SoundFontSettings> safe(this);
            juce::Timer::callAfterDelay(500,[safe,generation]
            {
                if (!safe || safe->previewGeneration!=generation) return;
                auto guard=safe->engine.lockDesktopState();
                safe->engine.songManager->desktopPreviewNoteOff();
            });
            juce::Timer::callAfterDelay(1200,[safe,generation] {if (safe && safe->previewGeneration==generation) safe->stopPreview();});
            status.setText("Previewing "+juce::String(preset.name),juce::dontSendNotification);
        }
        void filter()
        {
            filtered.clear(); const auto query=search.getText();
            for (int i=0;i<int(presets.size());++i)
            {
                const auto& p=presets[size_t(i)];
                if ((juce::String(p.name)+" "+juce::String(p.bank)+":"+juce::String(p.program)).containsIgnoreCase(query)) filtered.push_back(i);
            }
            list.updateContent(); list.deselectAllRows();
            status.setText(juce::String(int(filtered.size()))+" presets. This bank applies to all internal instruments; loading stops playback.",juce::dontSendNotification);
        }
        void chooseBank()
        {
            stopPreview();
            chooser=std::make_unique<juce::FileChooser>("Load SoundFont",juce::File("D:\\Old Samples"),"*.sf2");
            juce::Component::SafePointer<SoundFontSettings> safe(this);
            chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser& choice)
            {
                if (!safe || !choice.getResult().existsAsFile()) return;
                std::string error;
                if (safe->engine.songManager->loadDesktopSoundFont(choice.getResult().getFullPathName().toStdString(),error))
                {
                    auto preferences=readPreferences(); preferences.getDynamicObject()->setProperty("SoundFont",choice.getResult().getFullPathName());
                    const bool saved=writePreferences(preferences); safe->refresh();
                    if (!saved) safe->status.setText("Bank loaded; could not save the startup preference.",juce::dontSendNotification);
                }
                else safe->status.setText(juce::String(error)+" Previous bank retained.",juce::dontSendNotification);
            });
        }
        void assignPreset()
        {
            const int row=list.getSelectedRow(), instrumentIndex=instruments.getSelectedId()-1;
            if (!juce::isPositiveAndBelow(row,filtered.size()) || !juce::isPositiveAndBelow(instrumentIndex,instrumentNames.size())) return;
            const auto& preset=presets[size_t(filtered[size_t(row)])]; const auto name=instrumentNames[instrumentIndex].toStdString();
            using namespace Ignite;
            engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleInstrument),paramId_t(InstrumentParam::paramSetActiveInstrument),name);
            engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleInstrument),paramId_t(InstrumentParam::paramInternalSound),true);
            engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleInstrument),paramId_t(InstrumentParam::paramInternalSoundPgm),SoundFontProgram::encode(preset.bank,preset.program));
            engine.instrumentManager->SaveInstrument(name);
            status.setText("Assigned and saved: "+juce::String(preset.name),juce::dontSendNotification);
        }
        Ignite::IgniteEngine& engine;
        juce::Label bank,status;
        juce::TextButton load{"Load SoundFont..."},assign{"Assign to instrument"},preview{"Preview C4"};
        juce::TextEditor search;
        juce::ListBox list;
        juce::ComboBox instruments;
        juce::StringArray instrumentNames;
        std::unique_ptr<juce::FileChooser> chooser;
        std::vector<Ignite::SongManager::SoundFontPreset> presets;
        std::vector<int> filtered;
        bool previewing=false;
        int previewGeneration=0;
    };

    class ProjectSettings : public juce::Component
    {
    public:
        explicit ProjectSettings(Ignite::IgniteEngine& e) : engine(e)
        {
            for (auto* child : std::initializer_list<juce::Component*>{&description,&exportButton,&restoreButton,&status}) addAndMakeVisible(child);
            description.setText("Export songs, patterns, instruments and settings as an Ignite backup ZIP. SoundFont files stay in your sample library. Restoring a backup requires restarting Ignite.",juce::dontSendNotification);
            exportButton.onClick=[this] { choose(false); }; restoreButton.onClick=[this] { choose(true); };
            setSize(720,580);
        }
        void resized() override {description.setBounds(16,16,getWidth()-32,90);exportButton.setBounds(16,124,210,34);restoreButton.setBounds(240,124,210,34);status.setBounds(16,174,getWidth()-32,100);}
    private:
        void choose(bool restoring)
        {
            chooser=std::make_unique<juce::FileChooser>(restoring?"Restore Ignite backup":"Export Ignite backup",
                juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Ignite-backup.zip"),"*.zip");
            juce::Component::SafePointer<ProjectSettings> safe(this);
            const int browserFlags=restoring ? juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles :
                juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting;
            chooser->launchAsync(browserFlags,[safe,restoring](const juce::FileChooser& chooser)
            {
                if (!safe || chooser.getResult()==juce::File{}) return;
                auto* self=safe.getComponent(); auto lock=self->engine.lockDesktopState();
                auto* songs=self->engine.songManager;
                if (restoring)
                {
                    songs->stopRecord(); songs->stopSong(); songs->setSystemState(KEY_PRESSED::STOP_KEY);
                    auto result=ProjectArchive::restoreFrom(chooser.getResult(),ProjectArchive::dataRoot());
                    if (result.failed()) self->status.setText(result.getErrorMessage(),juce::dontSendNotification);
                    else
                    {
                        self->status.setText("Restored. Close and restart Ignite to load the restored libraries.",juce::dontSendNotification);
                        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Restore complete",
                            "The backup has been restored. Ignite will now close; open it again using the Desktop shortcut.","Close Ignite",nullptr,
                            juce::ModalCallbackFunction::create([](int) { juce::JUCEApplication::getInstance()->quit(); }));
                    }
                }
                else
                {
                    if (!self->engine.SaveAllData())
                    {
                        self->status.setText("Save failed. Backup export stopped; your edits remain open.",juce::dontSendNotification);
                        return;
                    }
                    auto result=ProjectArchive::exportTo(ProjectArchive::dataRoot(),chooser.getResult());
                    self->status.setText(result.wasOk()?"Backup exported: "+chooser.getResult().getFullPathName():result.getErrorMessage(),juce::dontSendNotification);
                }
            });
        }
        Ignite::IgniteEngine& engine;
        juce::Label description,status;
        juce::TextButton exportButton{"Export backup..."},restoreButton{"Restore backup..."};
        std::unique_ptr<juce::FileChooser> chooser;
    };

    class Settings : public juce::Component
    {
    public:
        Settings(Ignite::IgniteEngine& e,MidiRouter& r) : engine(e), midi(r),fonts(e),projects(e),tabs(juce::TabbedButtonBar::TabsAtTop)
        {
            audio=std::make_unique<juce::AudioDeviceSelectorComponent>(*engine.songManager->dm,0,0,0,2,false,false,true,false);
            audio->setSize(720,580);
            audioView.setViewedComponent(audio.get(),false); midiView.setViewedComponent(&midi,false);
            tabs.addTab("Audio",juce::Colour(0xff25272b),&audioView,false);
            tabs.addTab("MIDI",juce::Colour(0xff25272b),&midiView,false);
            tabs.addTab("SoundFonts",juce::Colour(0xff25272b),&fonts,false);
            tabs.addTab("Projects",juce::Colour(0xff25272b),&projects,false);
            addAndMakeVisible(tabs); addAndMakeVisible(save); addAndMakeVisible(panic); addAndMakeVisible(status);
            save.onClick=[this]
            {
                auto value=readPreferences(); auto xml=engine.songManager->dm->createStateXml();
                if (xml) value.getDynamicObject()->setProperty("AudioDevice",xml->toString());
                status.setText(writePreferences(value)?"Audio settings saved.":"Could not save audio settings.",juce::dontSendNotification);
            };
            panic.onClick=[this] {engine.songManager->desktopPanic();status.setText("Sounding notes cleared.",juce::dontSendNotification);};
            setSize(760,690);
        }
        void resized() override {tabs.setBounds(8,8,getWidth()-16,getHeight()-66);save.setBounds(12,getHeight()-48,150,30);panic.setBounds(174,getHeight()-48,110,30);status.setBounds(294,getHeight()-48,getWidth()-306,32);}
        void showTab(int index) { tabs.setCurrentTabIndex(index); }
    private:
        Ignite::IgniteEngine& engine;
        MidiSettings midi;
        SoundFontSettings fonts;
        ProjectSettings projects;
        std::unique_ptr<juce::AudioDeviceSelectorComponent> audio;
        juce::Viewport audioView,midiView;
        juce::TabbedComponent tabs;
        juce::TextButton save{"Save audio settings"},panic{"Panic"};
        juce::Label status;
    };
}
