// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <JuceHeader.h>
#include <cmath>
#include "DesktopPreferences.h"

class DesktopPanel : public juce::Component, private juce::Timer
{
    using Param = Ignite::FrontPannelParam;
    using Status = Ignite::FrontPannelKeyStatus;

    class Control : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
    {
    public:
        Control(DesktopPanel& panel, Param key, Param turn, bool isEncoder, const juce::String& name)
            : owner(panel), keyId(key), turnId(turn), encoder(isEncoder), caption(name.upToFirstOccurrenceOf(" (",false,false))
        {
            setComponentID("ignite-control-" + juce::String(int(key)));
            if (int(key)>=30 && int(key)<=61) caption=juce::String(int(key)-(int(key)<46?30:46)+1);
            setTooltip(name + (encoder ? (key == turn ? " — drag vertically or scroll to turn" : " — drag vertically or scroll to turn; click to press") : " — click and hold"));
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }
        void mouseDown(const juce::MouseEvent& event) override
        {
            down = true; dragged = false; lastY = event.y;
            if (!encoder) { owner.input(keyId, uint8_t(Status::paramKeyStatusPressed)); startTimer(500); }
            repaint();
        }
        void mouseDrag(const juce::MouseEvent& event) override
        {
            if (!encoder) return;
            int ticks = (lastY - event.y) / 4;
            if (ticks != 0) { dragged = true; lastY -= ticks * 4; rotate(ticks); }
        }
        void mouseUp(const juce::MouseEvent&) override
        {
            stopTimer(); down = false;
            if (encoder) { if (!dragged && keyId != turnId) owner.tap(keyId); }
            else owner.input(keyId, uint8_t(Status::paramKeyStatusReleased));
            repaint();
        }
        void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
        {
            if (encoder && wheel.deltaY != 0) rotate(wheel.deltaY > 0 ? 1 : -1);
        }
        void mouseEnter(const juce::MouseEvent&) override { repaint(); }
        void mouseExit(const juce::MouseEvent&) override { repaint(); }
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(1);
            auto led = owner.led(keyId);
            const bool lit = (keyId == Param::paramKey_PAGESHIFT && ui::FrountPannel::GetFrountPannelInstance()->getShiftKeyStatus()) ||
                (led > 1 && led != uint8_t(Ignite::fpLEDPattern::EN_LED_PATT_OFF)
                && led < uint8_t(Ignite::fpLEDPattern::EN_RGB_LED_OFF));
            if (!owner.isHardwareLayout())
            {
                paintCompact(g,bounds,lit);
                return;
            }
            if (down || isMouseOver() || lit)
            {
                g.setColour((down ? juce::Colours::white : juce::Colour(0xfff17183)).withAlpha(down ? 0.40f : 0.18f));
                if (encoder) g.fillEllipse(bounds); else g.fillRoundedRectangle(bounds, 2);
                g.setColour(down ? juce::Colours::white : juce::Colour(0xffef536f));
                if (encoder) g.drawEllipse(bounds, 0.8f); else g.drawRoundedRectangle(bounds, 2, 0.8f);
            }
            if (encoder && turns != 0)
            {
                float angle = float(turns) * 0.12f;
                auto centre = bounds.getCentre();
                g.setColour(juce::Colours::white.withAlpha(0.85f));
                g.drawLine(centre.x, centre.y, centre.x + std::sin(angle) * bounds.getWidth() * 0.35f,
                           centre.y - std::cos(angle) * bounds.getHeight() * 0.35f, 1.1f);
            }
        }
        Param key() const { return keyId; }
        juce::Rectangle<int> hardwareBounds;
    private:
        void paintCompact(juce::Graphics& g, juce::Rectangle<float> bounds, bool lit)
        {
            const auto accent = keyId == Param::paramKey_PLAY ? juce::Colour(0xff54c992) : juce::Colour(0xffed718a);
            g.setColour((down || lit) ? accent.withAlpha(0.36f) : juce::Colour(isMouseOver() ? 0xff38414c : 0xff282f38));
            g.fillRoundedRectangle(bounds,4);
            g.setColour((down || lit) ? accent : juce::Colour(0xff4a5562));
            g.drawRoundedRectangle(bounds,4,1);
            if (encoder)
            {
                const float diameter = juce::jmin(bounds.getWidth()-8,bounds.getHeight()-19);
                auto dial = juce::Rectangle<float>(diameter,diameter).withCentre({bounds.getCentreX(),bounds.getY()+diameter/2+3});
                g.setColour(juce::Colour(0xff14191f)); g.fillEllipse(dial);
                g.setColour(down ? accent : juce::Colour(0xff9ca9b8)); g.drawEllipse(dial,1);
                const float angle=float(turns)*0.12f;
                const auto centre=dial.getCentre();
                g.setColour(juce::Colour(0xfff0f3f6));
                g.drawLine(centre.x,centre.y,centre.x+std::sin(angle)*diameter*0.36f,centre.y-std::cos(angle)*diameter*0.36f,1.8f);
                g.setFont(10.5f);
                g.drawFittedText(caption,getLocalBounds().withTrimmedTop(getHeight()-17).reduced(2,0),juce::Justification::centred,1);
            }
            else
            {
                g.setColour(juce::Colour(0xffedf1f5)); g.setFont(11.5f);
                g.drawFittedText(caption,getLocalBounds().reduced(3,2),juce::Justification::centred,2);
            }
        }
        void timerCallback() override
        {
            owner.input(keyId, uint8_t(Status::paramKeyStatusOnHold)); startTimer(120);
        }
        void rotate(int ticks)
        {
            ticks = juce::jlimit(-16, 16, ticks);
            turns += ticks;
            for (int i = 0; i < std::abs(ticks); ++i)
            {
                if (turnId == Param::paramKey_ROTARY_NAV)
                    owner.tap(ticks > 0 ? Param::paramKey_RMOVERIGHT : Param::paramKey_RMOVELEFT);
                else owner.input(turnId, ticks > 0 ? ROTARY_RIGHT : ROTARY_LEFT);
            }
            repaint();
        }
        DesktopPanel& owner;
        Param keyId, turnId;
        bool encoder = false, down = false, dragged = false;
        int lastY = 0, turns = 0;
        juce::String caption;
    };

    class Surface : public juce::Component
    {
    public:
        Surface() { background = juce::Drawable::createFromSVGFile(ui::Theme::getAssetsDirectory().getChildFile("ignite-panel.svg")); }
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xff202124));
            if (hardware && background) background->drawWithin(g, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit, 1.0f);
            if (!hardware)
            {
                g.fillAll(juce::Colour(0xff171c22));
                for (const auto& group : groups)
                {
                    g.setColour(juce::Colour(0xffa6b1bd)); g.setFont(11);
                    g.drawText(group.first,group.second,juce::Justification::centredLeft);
                }
            }
        }
        std::unique_ptr<juce::Drawable> background;
        bool hardware=true;
        std::vector<std::pair<juce::String,juce::Rectangle<int>>> groups;
    };

public:
    DesktopPanel()
    {
        addAndMakeVisible(surface);
        surface.setSize(748, 478);
        auto& screen = ui::getAppHome();
        surface.addAndMakeVisible(screen);
        screen.setBounds(0, 0, 1024, 600);
        auto key = [this](const char* name, Param id, int x, int y, int w = 35, int h = 20)
        { add(name, id, id, false, {x, y, w, h}); };
        auto knob = [this](const char* name, Param push, Param turn, int x, int y, int d = 35)
        { add(name, push, turn, true, {x - d/2, y - d/2, d, d}); };
        key("USB STOP", Param::paramKey_USB_STOP, 27, 116, 28, 21);
        key("MENU (F1)", Param::paramKey_MENU, 27, 167, 28, 19);
        knob("ROW", Param::paramKey_ROTARYROW, Param::paramRot_ROW, 47, 270, 42);
        knob("BAR", Param::paramKey_ROTARYBAR, Param::paramRot_BAR, 700, 270, 42);
        knob("JOG / ENTER", Param::paramKey_RSELECT, Param::paramKey_ROTARY_NAV, 442, 126, 53);
        key("RETURN", Param::paramKey_RETURN_NAV, 404, 90, 20, 19);
        key("COPY", Param::paramKey_COPY_NAV, 461, 90, 20, 19);
        key("MINUS", Param::paramKey_MINUS_NAV, 404, 147, 20, 18);
        key("PLUS", Param::paramKey_PLUS_NAV, 461, 147, 20, 18);
        const char* names[]{"A / RELEASE", "B / CUTOFF", "C / RESONANCE", "D / VOLUME", "E / REVERB", "F / CHORUS", "G / EQ HIGH", "H / EQ LOW"};
        for (int i = 0; i < 8; ++i)
            knob(names[i], Param(63 + i), Param(63 + i), 523 + (i % 4) * 60, i < 4 ? 88 : 166);
        const Param upper[]{Param::paramKey_PAGESHIFT, Param::paramKey_LAST, Param::paramKey_SAVE, Param::paramKey_REC,
            Param::paramKey_BACKFORWARD, Param::paramKey_STOP, Param::paramKey_PLAY, Param::paramKey_SCULPT,
            Param::paramKey_DELETE, Param::paramKey_INSERT};
        const Param lower[]{Param::paramKey_SONG, Param::paramKey_SCENE, Param::paramKey_TRACK, Param::paramKey_NEXT_TIE,
            Param::paramKey_FILL_X, Param::paramKey_SOLO_SKIP, Param::paramKey_GANG, Param::paramKey_PATTERN,
            Param::paramKey_TEMPO, Param::paramKey_INTERNALSOUNDS};
        const char* upperNames[]{"SHIFT", "LAST", "SAVE (Ctrl+S)", "REC", "REWIND", "STOP (Esc)", "PLAY (Space)", "SCULPT", "DELETE", "INSERT"};
        const char* lowerNames[]{"SONG (F2)", "SCENE (F3)", "TRACK (F4)", "NEXT / TIE", "FILL / X", "SOLO / SKIP", "GANG", "PATTERN (F5)", "TEMPO", "INTERNAL SOUNDS"};
        const int columns[]{96, 152, 209, 271, 328, 385, 442, 503, 560, 617};
        for (int i = 0; i < 10; ++i)
        {
            key(upperNames[i], upper[i], columns[i], i >= 3 && i <= 6 ? 238 : 240, 35, i >= 3 && i <= 6 ? 35 : 20);
            key(lowerNames[i], lower[i], columns[i], 285, 35, 20);
        }
        for (int i = 0; i < 16; ++i)
        {
            int x = 34 + int(float(i) * 45.2f);
            knob(("STEP ENCODER " + juce::String(i + 1)).toRawUTF8(), Param(46 + i), Param(72 + i), x, 356, 35);
            key(("PAD " + juce::String(i + 1)).toRawUTF8(), Param(30 + i), x - 13, 398, 26, 43);
        }
        setSize(1346, 860);
        for (auto* component : std::initializer_list<juce::Component*>{&title,&hardwareButton,&compactButton,&settingsButton}) addAndMakeVisible(component);
        title.setText("IGNITE",juce::dontSendNotification); title.setFont(juce::Font(17,juce::Font::bold));
        hardwareButton.setComponentID("layout-hardware"); compactButton.setComponentID("layout-compact");
        for (auto* button : {&hardwareButton,&compactButton})
        {
            button->setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff963f58));
            button->setColour(juce::TextButton::textColourOnId,juce::Colours::white);
        }
        for (auto* button : {&hardwareButton,&compactButton,&settingsButton})
        {
            button->setWantsKeyboardFocus(false);
            button->setMouseClickGrabsKeyboardFocus(false);
        }
        hardwareButton.setTooltip("Use the original hardware arrangement");
        compactButton.setTooltip("Larger touchscreen with closely spaced controls for computer monitors");
        hardwareButton.onClick=[this] {setHardwareLayout(true);};
        compactButton.onClick=[this] {setHardwareLayout(false);};
        settingsButton.onClick = [this] { if (showDesktopSettings) showDesktopSettings(); };
        setHardwareLayout(IgniteDesktop::readPreferences().getProperty("PanelLayout","compact").toString()=="hardware",false);
        startTimerHz(20);
    }
    ~DesktopPanel() override
    {
        stopTimer();
        ui::getAppHome().setTransform({});
        surface.removeChildComponent(&ui::getAppHome());
    }
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff11161c));
        g.setColour(juce::Colour(0xff2b333d)); g.fillRect(0,toolbarHeight-1,getWidth(),1);
    }
    void resized() override
    {
        auto bar=getLocalBounds().removeFromTop(toolbarHeight).reduced(8,5);
        title.setBounds(bar.removeFromLeft(92));
        hardwareButton.setBounds(bar.removeFromLeft(96).reduced(2,0));
        compactButton.setBounds(bar.removeFromLeft(96).reduced(2,0));
        settingsButton.setBounds(bar.removeFromRight(148));
        auto content=getLocalBounds().withTrimmedTop(toolbarHeight);
        surface.groups.clear();
        if (hardwareLayout)
        {
            surface.setBounds(0,0,748,478);
            const float scale=juce::jmin(float(content.getWidth())/748,float(content.getHeight())/478);
            surface.setTransform(juce::AffineTransform::scale(scale).translated(content.getX()+(content.getWidth()-748*scale)/2,
                                                                                          content.getY()+(content.getHeight()-478*scale)/2));
            ui::getAppHome().setTransform(juce::AffineTransform::scale(280.0f/1024,161.0f/600).translated(90,35));
            for (auto& control : controls) control->setBounds(control->hardwareBounds);
        }
        else
        {
            surface.setTransform({}); surface.setBounds(content);
            layoutCompact();
        }
        surface.repaint();
    }
    bool isHardwareLayout() const { return hardwareLayout; }
    void setHardwareLayout(bool enabled,bool persist=true)
    {
        hardwareLayout=enabled; surface.hardware=enabled;
        hardwareButton.setToggleState(enabled,juce::dontSendNotification);
        compactButton.setToggleState(!enabled,juce::dontSendNotification);
        if (persist)
        {
            auto preferences=IgniteDesktop::readPreferences();
            preferences.getDynamicObject()->setProperty("PanelLayout",enabled?"hardware":"compact");
            if (!IgniteDesktop::writePreferences(preferences)) juce::Logger::writeToLog("Could not save the simulator layout preference.");
        }
        resized();
    }
    bool keyPressed(const juce::KeyPress& key) override
    {
        auto* focused = juce::Component::getCurrentlyFocusedComponent();
        if (dynamic_cast<juce::TextEditor*>(focused) != nullptr) return false;
        const int code = key.getKeyCode();
        if (code >= juce::KeyPress::F1Key && code <= juce::KeyPress::F5Key)
        {
            const Param ids[]{Param::paramKey_MENU, Param::paramKey_SONG, Param::paramKey_SCENE, Param::paramKey_TRACK, Param::paramKey_PATTERN};
            tap(ids[code - juce::KeyPress::F1Key]); return true;
        }
        if (code == juce::KeyPress::spaceKey) { tap(Param::paramKey_PLAY); return true; }
        if (code == juce::KeyPress::escapeKey) { tap(Param::paramKey_STOP); return true; }
        if (key.getModifiers().isCommandDown() && (code == 'S' || code == 's')) { tap(Param::paramKey_SAVE); return true; }
        return false;
    }
    void tap(Param id)
    {
        input(id, uint8_t(Status::paramKeyStatusPressed));
        juce::Component::SafePointer<DesktopPanel> safe(this);
        juce::Timer::callAfterDelay(60, [safe, id] { if (safe) safe->input(id, uint8_t(Status::paramKeyStatusReleased)); });
    }
    void input(Param id, uint8_t value)
    {
        auto* engine = static_cast<Ignite::IgniteEngine*>(_PUiVirtualEngine.get());
        if (!engine || Ignite::CThread::desktopShutdownRequested()) return;
        if (!engine->songManager->activeSong && (id == Param::paramKey_PLAY || id == Param::paramKey_REC || id == Param::paramKey_STOP)) return;
        ui::FrountPannel::GetFrountPannelInstance()->simulatorInput(id, value);
    }
    uint8_t led(Param id) const
    {
        auto* engine = static_cast<Ignite::IgniteEngine*>(_PUiVirtualEngine.get());
        return engine ? engine->frontPannelManager->simulatorLed(uint8_t(id)) : 0;
    }
    bool faceplateLoaded() const { return backgroundLoaded(); }
    std::function<void()> showDesktopSettings;
    bool backgroundLoaded() const { return surface.background != nullptr; }
private:
    void place(Param key,juce::Rectangle<int> bounds)
    {
        for (auto& control : controls) if (control->key()==key) { control->setBounds(bounds); return; }
    }
    void layoutCompact()
    {
        auto area=surface.getLocalBounds().reduced(8);
        auto steps=area.removeFromBottom(94); area.removeFromBottom(8);
        auto side=area.removeFromRight(244); area.removeFromRight(10);
        const float scale=juce::jmin(float(area.getWidth())/1024,float(area.getHeight())/600);
        ui::getAppHome().setTransform(juce::AffineTransform::scale(scale).translated(area.getCentreX()-512*scale,area.getCentreY()-300*scale));
        const float verticalScale=juce::jmin(1.0f,float(side.getHeight())/464);
        const int x=side.getX(), width=side.getWidth();
        float y=float(side.getY());
        auto heading=[&](const juce::String& name)
        {
            surface.groups.push_back({name,{x,juce::roundToInt(y),width,juce::roundToInt(16*verticalScale)}});
            y+=16*verticalScale;
        };
        auto row=[&](std::initializer_list<Param> keys,int height)
        {
            int i=0; const int count=int(keys.size());
            for (auto key : keys)
            {
                const int left=x+i*width/count,right=x+(i+1)*width/count;
                place(key,{left,juce::roundToInt(y),right-left-4,juce::roundToInt(height*verticalScale)-3}); ++i;
            }
            y+=height*verticalScale;
        };
        heading("SOUND");
        row({Param(63),Param(64),Param(65),Param(66)},50);
        row({Param(67),Param(68),Param(69),Param(70)},50); y+=8*verticalScale;
        heading("NAVIGATION");
        row({Param::paramKey_ROTARYROW,Param::paramKey_RSELECT,Param::paramKey_ROTARYBAR},52);
        row({Param::paramKey_RETURN_NAV,Param::paramKey_COPY_NAV,Param::paramKey_MINUS_NAV,Param::paramKey_PLUS_NAV},28); y+=8*verticalScale;
        heading("MODES");
        row({Param::paramKey_MENU,Param::paramKey_SONG,Param::paramKey_SCENE,Param::paramKey_TRACK},28);
        row({Param::paramKey_PATTERN,Param::paramKey_INTERNALSOUNDS,Param::paramKey_TEMPO,Param::paramKey_USB_STOP},28); y+=8*verticalScale;
        heading("TRANSPORT");
        row({Param::paramKey_BACKFORWARD,Param::paramKey_REC,Param::paramKey_STOP,Param::paramKey_PLAY},32); y+=8*verticalScale;
        heading("EDIT / PERFORMANCE");
        row({Param::paramKey_PAGESHIFT,Param::paramKey_LAST,Param::paramKey_SAVE,Param::paramKey_NEXT_TIE},28);
        row({Param::paramKey_FILL_X,Param::paramKey_SOLO_SKIP,Param::paramKey_GANG,Param::paramKey_SCULPT},28);
        row({Param::paramKey_DELETE,Param::paramKey_INSERT},28);
        surface.groups.push_back({"STEPS / PADS",steps.removeFromTop(16)});
        const int stepWidth=steps.getWidth();
        for (int i=0;i<16;++i)
        {
            const int left=steps.getX()+i*stepWidth/16,right=steps.getX()+(i+1)*stepWidth/16;
            place(Param(46+i),{left,steps.getY(),right-left-4,48});
            place(Param(30+i),{left,steps.getY()+51,right-left-4,27});
        }
    }
    void timerCallback() override { surface.repaint(); }
    void add(const juce::String& name, Param key, Param turn, bool encoder, juce::Rectangle<int> bounds)
    {
        auto control = std::make_unique<Control>(*this, key, turn, encoder, name);
        control->hardwareBounds=bounds;
        control->setBounds(bounds); surface.addAndMakeVisible(*control); controls.push_back(std::move(control));
    }
    Surface surface;
    static constexpr int toolbarHeight=40;
    bool hardwareLayout=true;
    juce::Label title;
    juce::TextButton hardwareButton{"Hardware"},compactButton{"Compact"},settingsButton{"Desktop settings"};
    std::vector<std::unique_ptr<Control>> controls;
    juce::TooltipWindow tooltips{this, 500};
};
