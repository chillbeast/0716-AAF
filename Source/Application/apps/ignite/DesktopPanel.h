// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <JuceHeader.h>
#include <cmath>

class DesktopPanel : public juce::Component, private juce::Timer
{
    using Param = Ignite::FrontPannelParam;
    using Status = Ignite::FrontPannelKeyStatus;

    class Control : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
    {
    public:
        Control(DesktopPanel& panel, Param key, Param turn, bool isEncoder, const juce::String& name)
            : owner(panel), keyId(key), turnId(turn), encoder(isEncoder)
        {
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
            const bool lit = led > 1 && led != uint8_t(Ignite::fpLEDPattern::EN_LED_PATT_OFF)
                && led < uint8_t(Ignite::fpLEDPattern::EN_RGB_LED_OFF);
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
    private:
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
    };

    class Surface : public juce::Component
    {
    public:
        Surface() { background = juce::Drawable::createFromSVGFile(ui::Theme::getAssetsDirectory().getChildFile("ignite-panel.svg")); }
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xff202124));
            if (background) background->drawWithin(g, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit, 1.0f);
        }
        std::unique_ptr<juce::Drawable> background;
    };

public:
    DesktopPanel()
    {
        addAndMakeVisible(surface);
        surface.setSize(748, 478);
        auto& screen = ui::getAppHome();
        surface.addAndMakeVisible(screen);
        screen.setBounds(0, 0, 1024, 600);
        screen.setTransform(juce::AffineTransform::scale(280.0f / 1024.0f, 161.0f / 600.0f).translated(90, 35));
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
        surface.addAndMakeVisible(settingsButton);
        settingsButton.setBounds(580, 451, 148, 20);
        settingsButton.onClick = [this] { if (showDesktopSettings) showDesktopSettings(); };
        startTimerHz(20);
    }
    ~DesktopPanel() override
    {
        stopTimer();
        ui::getAppHome().setTransform({});
        surface.removeChildComponent(&ui::getAppHome());
    }
    void paint(juce::Graphics& g) override { g.fillAll(juce::Colour(0xff111215)); }
    void resized() override
    {
        const float scale = juce::jmin(float(getWidth()) / 748, float(getHeight()) / 478);
        surface.setTransform(juce::AffineTransform::scale(scale).translated((getWidth() - 748 * scale) / 2,
                                                                         (getHeight() - 478 * scale) / 2));
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
    void timerCallback() override { surface.repaint(); }
    void add(const juce::String& name, Param key, Param turn, bool encoder, juce::Rectangle<int> bounds)
    {
        auto control = std::make_unique<Control>(*this, key, turn, encoder, name);
        control->setBounds(bounds); surface.addAndMakeVisible(*control); controls.push_back(std::move(control));
    }
    Surface surface;
    juce::TextButton settingsButton{"DESKTOP SETTINGS"};
    std::vector<std::unique_ptr<Control>> controls;
    juce::TooltipWindow tooltips{this, 500};
};
