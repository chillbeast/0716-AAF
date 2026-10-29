// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "DesktopPanel.h"

class DesktopLayoutTest : private juce::Timer
{
public:
    DesktopLayoutTest(DesktopPanel& p,Ignite::IgniteEngine& e,bool verifyOnly,
                      std::function<void(int,int)> resize,std::function<void(int)> done)
        : panel(p),engine(e),verify(verifyOnly),resizeWindow(std::move(resize)),finished(std::move(done))
    { startTimer(250); }
    ~DesktopLayoutTest() override { stopTimer(); }
private:
    void check(bool ok,const juce::String& name)
    {
        ++checks; if (!ok) ++failures;
        juce::Logger::writeToLog(juce::String(ok?"PASS: ":"FAIL: ")+name);
    }
    void choose(bool hardware){choose(hardware?DesktopPanel::Layout::Hardware:DesktopPanel::Layout::Compact);}
    void choose(DesktopPanel::Layout value)
    {
        auto* button=dynamic_cast<juce::Button*>(panel.findChildWithID(value==DesktopPanel::Layout::Hardware?"layout-hardware":value==DesktopPanel::Layout::Touchscreen?"layout-touchscreen":"layout-compact"));
        check(button!=nullptr,"layout selector is an accessible button");
        if (button)
        {
            waitingForLayout=true; requestedLayout=value;
            layoutDeadline=juce::Time::getMillisecondCounterHiRes()+3000;
            button->triggerClick();
        }
    }
    void geometry()
    {
        auto& screen=ui::getAppHome();
        auto* surface=screen.getParentComponent();
        check(surface && surface->getParentComponent()==&panel,"the original live touchscreen stays attached to the simulator");
        if (!surface) return;
        const auto screenBounds=screen.getBoundsInParent();
        check(surface->getLocalBounds().contains(screenBounds),"touchscreen fits within the current layout");
        if (panel.isHardwareLayout()) return;
        if(panel.isTouchscreenLayout())
        {
            int total=0,visible=0;
            for(auto* child:surface->getChildren())if(child->getComponentID().startsWith("ignite-control-"))
            {++total;if(child->isVisible())++visible;}
            check(total==69 && visible==0,"touchscreen mode hides every hardware control without destroying them");
            check(std::abs(float(screenBounds.getWidth())/screenBounds.getHeight()-1024.0f/600)<0.01f,"touchscreen mode preserves the display aspect ratio");
            const float available=juce::jmin(float(surface->getWidth())/1024,float(surface->getHeight())/600);
            check(screenBounds.getWidth()>=1024*available-2 && screenBounds.getHeight()>=600*available-2,"touchscreen mode uses the full available display area");
            return;
        }
        bool inside=true,separate=true;
        std::vector<juce::Rectangle<int>> bounds;
        for (auto* child : surface->getChildren())
            if (child->getComponentID().startsWith("ignite-control-"))
            {
                inside &= child->isVisible() && surface->getLocalBounds().contains(child->getBounds());
                separate &= !screenBounds.intersects(child->getBounds());
                for (const auto& other : bounds) separate &= !other.intersects(child->getBounds());
                bounds.push_back(child->getBounds());
            }
        check(bounds.size()==69 && inside,"all 69 original controls remain visible and fit the compact panel");
        check(separate,"compact controls do not overlap each other or the touchscreen");
        check(std::abs(float(screenBounds.getWidth())/screenBounds.getHeight()-1024.0f/600)<0.01f,
              "compact touchscreen preserves its original aspect ratio");
    }
    void snapshot(const juce::String& name)
    {
        const auto folder=juce::SystemStats::getEnvironmentVariable("IGNITE_LAYOUT_SNAPSHOTS","");
        if (folder.isEmpty()) return;
        auto root=juce::File(folder); root.createDirectory();
        juce::FileOutputStream stream(root.getChildFile(name+".png"));
        if (stream.openedOk()) juce::PNGImageFormat().writeImageToStream(panel.createComponentSnapshot(panel.getLocalBounds()),stream);
    }
    void sameState()
    {
        check(&ui::getAppHome()==originalScreen && engine.songManager->activeSong.get()==originalSong,
              "changing layout preserves the live screen and selected song");
    }
    void finish()
    {
        stopTimer();
        juce::Logger::writeToLog("Layout checks: "+juce::String(checks)+", failures: "+juce::String(failures));
        finished(failures);
    }
    void timerCallback() override
    {
        if (waitingForLayout)
        {
            if (panel.getLayout()!=requestedLayout && juce::Time::getMillisecondCounterHiRes()<layoutDeadline) return;
            waitingForLayout=false;
        }
        using P=Ignite::FrontPannelParam;
        switch (stage++)
        {
            case 0:
            {
                const auto expected=juce::SystemStats::getEnvironmentVariable("IGNITE_EXPECT_LAYOUT","compact");
                check(panel.getLayout()==(expected=="hardware"?DesktopPanel::Layout::Hardware:expected=="touchscreen"?DesktopPanel::Layout::Touchscreen:DesktopPanel::Layout::Compact),"startup honors the saved layout or explicit override");
                auto preferences=IgniteDesktop::readPreferences();
                check(preferences.getProperty("LayoutSentinel","").toString()=="preserve-other-settings","layout changes preserve other desktop preferences");
                auto* window=panel.getTopLevelComponent();
                auto* display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(window->getScreenBounds());
                check(display && display->userArea.contains(window->getScreenBounds()),"initial window fits the monitor work area");
                if (verify)
                {
                    check(preferences.getProperty("PanelLayout","").toString()=="touchscreen","saved touchscreen choice survives restart and temporary overrides");
                    geometry(); finish(); return;
                }
                auto command=[&](Ignite::ModuleName module,auto parameter,Ignite::AnyType value)
                {engine.SetCommand(Ignite::moduleId_t(Ignite::ModuleName::moduleUI),Ignite::moduleId_t(module),Ignite::paramId_t(parameter),value);};
                command(Ignite::ModuleName::moduleSong,Ignite::SongParam::paramAddSongCmd,uint8_t(0));
                command(Ignite::ModuleName::moduleSong,Ignite::SongParam::paramActiveSong,int32_t(0));
                originalSong=engine.songManager->activeSong.get();
                originalScreen=&ui::getAppHome();
                check(originalSong!=nullptr,"layout test creates a selected song");
                panel.tap(P::paramKey_SONG); choose(false); break;
            }
            case 1:
                check(!panel.isHardwareLayout(),"Compact toolbar button changes the layout");
                sameState(); geometry(); snapshot("compact"); choose(true); break;
            case 2:
                check(panel.isHardwareLayout(),"Hardware toolbar button restores the faceplate");
                sameState(); geometry(); snapshot("hardware");
                for (int i=0;i<16;++i) panel.setHardwareLayout(i%2==0,false);
                sameState(); panel.setHardwareLayout(true,false); choose(false); break;
            case 3:
                resizeWindow(960,600); geometry(); sameState(); snapshot("compact-960x600"); break;
            case 4:
                resizeWindow(1280,720); geometry(); snapshot("compact-1280x720"); break;
            case 5:
            {
                resizeWindow(1366,768); geometry(); snapshot("compact-1366x768");
                const auto compactArea=ui::getAppHome().getBoundsInParent().getWidth()*ui::getAppHome().getBoundsInParent().getHeight();
                panel.setHardwareLayout(true,false);
                const auto hardwareArea=ui::getAppHome().getBoundsInParent().getWidth()*ui::getAppHome().getBoundsInParent().getHeight();
                panel.setHardwareLayout(false,false);
                check(compactArea>hardwareArea*2,"compact layout provides more than twice the touchscreen area");
                break;
            }
            case 6:
                resizeWindow(960,540); geometry(); snapshot("compact-scaled-desktop");
                panel.tap(P::paramKey_PAGESHIFT); break;
            case 7:
                check(ui::FrountPannel::GetFrountPannelInstance()->getShiftKeyStatus(),"compact SHIFT control engages");
                panel.tap(P::paramKey_PAGESHIFT); panel.tap(P::paramKey_MENU); break;
            case 8:
                check(!ui::FrountPannel::GetFrountPannelInstance()->getShiftKeyStatus(),"compact SHIFT control releases");
                check(ui::AppMainmode::getMainModeInstance()->GetActiveTab()==mt::Ignite::tabs::MENU,"compact navigation reaches the firmware menu");
                sameState();
                check(IgniteDesktop::readPreferences().getProperty("PanelLayout","").toString()=="compact","toolbar choice is saved for the next launch");
                compactArea=ui::getAppHome().getBoundsInParent().getWidth()*ui::getAppHome().getBoundsInParent().getHeight();
                choose(DesktopPanel::Layout::Touchscreen);break;
            case 9:
                check(panel.isTouchscreenLayout(),"Touchscreen toolbar button selects the third mode");
                sameState();geometry();snapshot("touchscreen-960x540");
                check(ui::getAppHome().getBoundsInParent().getWidth()*ui::getAppHome().getBoundsInParent().getHeight()>compactArea*1.5,"touchscreen-only mode gives the display more area than compact mode");
                break;
            case 10:
                resizeWindow(1280,720);geometry();snapshot("touchscreen-1280x720");
                layoutDeadline=juce::Time::getMillisecondCounterHiRes()+3000;
                check(panel.keyPressed(juce::KeyPress(juce::KeyPress::F2Key)),"keyboard navigation remains available without panel controls");break;
            case 11:
                if(ui::AppMainmode::getMainModeInstance()->GetActiveTab()!=mt::Ignite::tabs::SONG && juce::Time::getMillisecondCounterHiRes()<layoutDeadline){--stage;return;}
                check(ui::AppMainmode::getMainModeInstance()->GetActiveTab()==mt::Ignite::tabs::SONG,"touchscreen-mode keyboard navigation reaches Song");
                choose(true);break;
            case 12:
                check(panel.isHardwareLayout(),"Hardware mode can be restored from Touchscreen");
                sameState();geometry();choose(DesktopPanel::Layout::Touchscreen);break;
            case 13:
                for(int i=0;i<18;++i)panel.setLayout(DesktopPanel::Layout(i%3),false);
                panel.setLayout(DesktopPanel::Layout::Touchscreen,false);sameState();geometry();
                panel.tap(P::paramKey_PAGESHIFT);break;
            case 14:
                check(ui::FrountPannel::GetFrountPannelInstance()->getShiftKeyStatus(),"mapped panel input still engages SHIFT in touchscreen mode");
                panel.tap(P::paramKey_PAGESHIFT);break;
            default:
                check(!ui::FrountPannel::GetFrountPannelInstance()->getShiftKeyStatus(),"mapped panel input releases SHIFT in touchscreen mode");
                check(IgniteDesktop::readPreferences().getProperty("PanelLayout","").toString()=="touchscreen","touchscreen choice is saved for the next launch");
                sameState();finish();break;
        }
    }
    DesktopPanel& panel;
    Ignite::IgniteEngine& engine;
    bool verify;
    std::function<void(int,int)> resizeWindow;
    std::function<void(int)> finished;
    const void* originalScreen=nullptr;
    const void* originalSong=nullptr;
    int stage=0,checks=0,failures=0;
    bool waitingForLayout=false;
    DesktopPanel::Layout requestedLayout=DesktopPanel::Layout::Compact;
    int compactArea=0;
    double layoutDeadline=0;
};
