/*
 * Copyright (c) - 2022 - MUSIC Tribe, MY
 * This file is part of IGNITE.
 *
 * IGNITE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * IGNITE is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with IGNITE. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
 /*
   ==============================================================================

	 This file contains the basic startup code for a JUCE application.

   ==============================================================================
 */
#include <JuceHeader.h>

using namespace Ignite;
using namespace Ignite::Midi;
std::unique_ptr<IListener> _PUiInterface;
std::unique_ptr<IDevice> _PUiVirtualEngine;
extern IDevice* EngineInit(void);
#include "DesktopPanel.h"
#include "DesktopWorkflowTest.h"
#include "DesktopServices.h"
#include "DesktopFeatureTest.h"

//==============================================================================
#define SHOW_BOOT_IMAGE
#if defined SHOW_BOOT_IMAGE
#define ENGINE_READY_WAIT_TIME_SEC 5

class waitpopup : public juce::Component
{
public:
	waitpopup()
	{
		Component::setSize(ui::layout::screen.getWidth(), ui::layout::screen.getHeight());
		Component::setTopLeftPosition(0, 0);
		Component::setFocusContainerType(FocusContainerType::focusContainer);
	};
	~waitpopup() = default;
	void mouseUp(const MouseEvent& event)override { return; };
	void mouseDown(const MouseEvent& event)override { return; };
	void paint(juce::Graphics& g)
	{
		Image dropImage = juce::ImageCache::getFromFile(Theme::getAssetsDirectory().getChildFile("boot-screen.png"));
		g.drawImageAt(dropImage, 0, 0);
	}
};
waitpopup* _waitpopup;
void showWaitForBootComplete(bool isClose)
{
	if (isClose)
	{
		if (_waitpopup)
		{
			_waitpopup->removeFromDesktop();
			delete _waitpopup;
		}
	}
	else // create
	{
		_waitpopup = new waitpopup;
		_waitpopup->setVisible(true);
		_waitpopup->addToDesktop(ComponentPeer::windowHasDropShadow);
	}
}
#endif //SHOW_BOOT_IMAGE
void UiEngineInterfaceInit()
{
	//Engine init, Interface init and linking interface end points
	_PUiVirtualEngine = (std::unique_ptr<IDevice>) EngineInit();
	_PUiInterface = std::make_unique<UiInterface>(_PUiVirtualEngine.get());
}

class IgniteApplication : public juce::JUCEApplication
{
public:
	//==============================================================================
	IgniteApplication()
	{
	}

	const juce::String getApplicationName() override { return ProjectInfo::projectName; }
	const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
	bool moreThanOneInstanceAllowed() override { return true; }

	//==============================================================================
	void initialise(const juce::String& commandLine) override
	{
		auto logPath = SystemStats::getEnvironmentVariable("IGNITE_LOG", "");
		if (logPath.isNotEmpty())
		{
			fileLogger = std::make_unique<FileLogger>(File(logPath), "IGNITE desktop " + getApplicationVersion(), 0);
			Logger::setCurrentLogger(fileLogger.get());
		}
#if defined SHOW_BOOT_IMAGE
		int waitCount = ENGINE_READY_WAIT_TIME_SEC;
#ifdef _WIN64
		showWaitForBootComplete(false);
#endif
		std::thread engineThread(UiEngineInterfaceInit);

		while ((!_PUiVirtualEngine || (_PUiVirtualEngine && _PUiVirtualEngine->isIDeviceReady() == false) )&& waitCount-- > 0)
			Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 1000);
		//if ((!_PUiVirtualEngine || (_PUiVirtualEngine && _PUiVirtualEngine->isIDeviceReady() == false)) && waitCount <= 0)
		// TODO: Do if any recovery required if engine not ready in time

		if (engineThread.joinable()) //this block the UI to load
			engineThread.join();
		mainWindow.reset(new MainWindow(getApplicationName(), !commandLine.contains("--touchscreen")));
#ifdef _WIN64
		showWaitForBootComplete(true);// pop-up exits
#endif
#else //SHOW_BOOT_IMAGE
		std::thread engineThread(UiEngineInterfaceInit);
		engineThread.join();
		mainWindow.reset(new MainWindow(getApplicationName(), !commandLine.contains("--touchscreen")));
#endif//SHOW_BOOT_IMAGE
#ifdef _WIN32
        midiRouter = std::make_unique<IgniteDesktop::MidiRouter>(*static_cast<IgniteEngine*>(_PUiVirtualEngine.get()));
        midiRouter->restore();
        mainWindow->showDesktopSettings = [this] { openDesktopSettings(); };
        if (auto* panel = dynamic_cast<DesktopPanel*>(mainWindow->getContentComponent()))
            panel->showDesktopSettings = [this] { openDesktopSettings(); };
#endif
		Logger::writeToLog("Engine ready: " + String(_PUiVirtualEngine && _PUiVirtualEngine->isIDeviceReady() ? "yes" : "no"));
		if (commandLine.contains("--self-test"))
			Timer::callAfterDelay(2500, [this] { runDesktopSelfTest(0); });
		if (commandLine.contains("--workflow-test") || commandLine.contains("--workflow-verify"))
			Timer::callAfterDelay(2500, [this, commandLine]
			{
				const auto failures = DesktopWorkflowTest::run(*static_cast<IgniteEngine*>(_PUiVirtualEngine.get()), commandLine.contains("--workflow-verify"), commandLine.contains("--node-edit"), commandLine.contains("--audio-device"));
				setApplicationReturnValue(failures == 0 ? 0 : 1);
				quit();
			});
        if (commandLine.contains("--feature-test") || commandLine.contains("--feature-verify"))
            Timer::callAfterDelay(2500,[this,commandLine]
            {
                auto& engine=*static_cast<IgniteEngine*>(_PUiVirtualEngine.get());
                const bool verify=commandLine.contains("--feature-verify");
                int failures=verify ? 0 : DesktopWorkflowTest::run(engine,false,true,false);
                if (!failures) failures+=DesktopFeatureTest::run(engine,*midiRouter,verify);
                openDesktopSettings();
                setApplicationReturnValue(failures?1:0);
                Timer::callAfterDelay(800,[this] {captureSettingsTabs(0);});
            });
        if (commandLine.contains("--desktop-settings")) Timer::callAfterDelay(1000,[this] {openDesktopSettings();});
		auto snapshotPath = SystemStats::getEnvironmentVariable("IGNITE_SNAPSHOT", "");
		if (snapshotPath.isNotEmpty())
			Timer::callAfterDelay(2000, [this, snapshotPath]
			{
				auto* content = mainWindow->getContentComponent();
				auto image = content->createComponentSnapshot(content->getLocalBounds());
				FileOutputStream stream{File(snapshotPath)};
				if (stream.openedOk())
				{
					PNGImageFormat().writeImageToStream(image, stream);
					Logger::writeToLog("UI snapshot saved: " + snapshotPath);
				}
			});
	}

	void shutdown() override
	{
        if (desktopSettings) delete desktopSettings.getComponent();
        midiRouter.reset();
#ifdef _WIN32
		if (_PUiVirtualEngine)
			static_cast<IgniteEngine*>(_PUiVirtualEngine.get())->shutdownDesktop();
#endif
		// Add your application's shutdown code here..
		mainWindow = nullptr; // (deletes our window)
		Logger::writeToLog("Shutdown: window closed");
		_PUiInterface.reset();
		Logger::writeToLog("Shutdown: UI publisher stopped");
		_PUiVirtualEngine.release(); // The firmware's _PIgniteEngine owns this instance.
		Logger::setCurrentLogger(nullptr);
		fileLogger.reset();
	}

	//==============================================================================
	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore this
		// request and let the app carry on running, or call quit() to allow the app to close.
		quit();
	}

	void anotherInstanceStarted(const juce::String& commandLine) override
	{
		// When another instance of the app is launched while this one is running,
		// this method is invoked, and the commandLine parameter tells you what
		// the other instance's command-line arguments were.
	}



	//==============================================================================
	/*
		This class implements the desktop window that contains an instance of
		our MainComponent class.
	*/
	class MainWindow : public juce::DocumentWindow
	{
	public:
		MainWindow(juce::String name, bool simulator)
			: DocumentWindow(name,
				juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
				DocumentWindow::allButtons)
		{
#if JUCE_WINDOWS || JUCE_MAC
			if (simulator)
			{
				setName("IGNITE Simulator");
				setContentOwned(new DesktopPanel(), true);
				setContentComponentSize(1346, 860);
				setResizable(true, true);
				setResizeLimits(748, 500, 2244, 1460);
				centreWithSize(getWidth(), getHeight());
			}
			else
#endif
			{
				setContentNonOwned(&ui::getAppHome(), false);
				setContentComponentSize(ui::layout::screen.getWidth(), ui::layout::screen.getHeight());
				setResizable(false, false);
			}
#if JUCE_WINDOWS || JUCE_MAC
			setUsingNativeTitleBar(true);
#else
			setUsingNativeTitleBar(false);
			setTitleBarHeight(0);
#endif
#if (JUCE_IOS || JUCE_ANDROID || JUCE_LINUX || __linux__)
			setFullScreen(true);
#endif
			setVisible(true);
		}

        std::function<void()> showDesktopSettings;
        bool keyPressed(const juce::KeyPress& key) override
        {
            if (key.getModifiers().isCommandDown() && key.getKeyCode()==',' && showDesktopSettings)
            { showDesktopSettings(); return true; }
            return DocumentWindow::keyPressed(key);
        }
		void closeButtonPressed() override
		{
			// This is called when the user tries to close this window. Here, we'll just
			// ask the app to quit when this happens, but you can change this to do
			// whatever you need.
			JUCEApplication::getInstance()->systemRequestedQuit();
		}

		/* Note: Be careful if you override any DocumentWindow methods - the base
		   class uses a lot of them, so by overriding you might break its functionality.
		   It's best to do all your work in your content component instead, but if
		   you really have to override any DocumentWindow methods, make sure your
		   subclass also calls the superclass's method.
		*/

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

private:
	void runDesktopSelfTest(int stage)
	{
		auto check = [this](bool pass, const String& name)
		{
			Logger::writeToLog(String(pass ? "PASS: " : "FAIL: ") + name);
			if (!pass) ++selfTestFailures;
		};
		auto* panel = dynamic_cast<DesktopPanel*>(mainWindow->getContentComponent());
		auto* front = ui::FrountPannel::GetFrountPannelInstance();
		using P = FrontPannelParam;
		if (stage == 0)
		{
			check(panel && panel->faceplateLoaded(), "simulator faceplate and panel loaded");
			check(_PUiVirtualEngine->isIDeviceReady(), "firmware engine ready");
			if (!panel) { setApplicationReturnValue(1); quit(); return; }
			panel->tap(P::paramKey_SONG);
		}
		else if (stage == 1)
		{
			check(ui::AppMainmode::getMainModeInstance()->GetActiveTab() == mt::Ignite::tabs::SONG, "hardware SONG button navigates touchscreen");
			panel->tap(P::paramKey_MENU);
		}
		else if (stage == 2)
		{
			check(ui::AppMainmode::getMainModeInstance()->GetActiveTab() == mt::Ignite::tabs::MENU, "hardware MENU button navigates touchscreen");
			panel->tap(P::paramKey_PAGESHIFT);
		}
		else if (stage == 3)
		{
			check(front->getShiftKeyStatus(), "hardware SHIFT engages");
			panel->tap(P::paramKey_PAGESHIFT);
		}
		else
		{
			check(!front->getShiftKeyStatus(), "hardware SHIFT releases");
			auto* engine = static_cast<IgniteEngine*>(_PUiVirtualEngine.get());
			Track track(0, "Desktop smoke", false, engine->songManager);
			track.AuditioningTrackInit();
			track.sfNoteOn(0, 60, 0.8f);
			float peak = 0;
			for (int block = 0; block < 10; ++block)
			{
				track.sfSynthesizeAudio();
				for (int i = 0; i < track.audioBuff.getNumSamples(); ++i)
					peak = std::max(peak, std::abs(track.audioBuff.getSample(0, i)));
			}
			check(engine->songManager->get_tsf_instance() == nullptr ? peak == 0 : peak > 0.0001f,
				"firmware track renders with SoundFont and stays silent without one");
			Logger::writeToLog("Desktop self-test failures: " + String(selfTestFailures));
			setApplicationReturnValue(selfTestFailures == 0 ? 0 : 1);
			quit(); return;
		}
		Timer::callAfterDelay(200, [this, stage] { runDesktopSelfTest(stage + 1); });
	}
	int selfTestFailures = 0;
    void captureSettingsTabs(int index)
    {
        auto path=SystemStats::getEnvironmentVariable("IGNITE_SETTINGS_SNAPSHOT","");
        if (!desktopSettings || index>=4 || path.isEmpty()) {quit();return;}
        if (auto* settings=dynamic_cast<IgniteDesktop::Settings*>(desktopSettings->getContentComponent())) settings->showTab(index);
        Timer::callAfterDelay(100,[this,index,path]
        {
            if (desktopSettings)
            {
                auto snapshot=desktopSettings->createComponentSnapshot(desktopSettings->getLocalBounds());
                auto file=File(path).getSiblingFile(File(path).getFileNameWithoutExtension()+"-"+String(index)+".png");
                FileOutputStream output(file);
                if(output.openedOk()) PNGImageFormat().writeImageToStream(snapshot,output);
            }
            captureSettingsTabs(index+1);
        });
    }
    void openDesktopSettings()
    {
        if (desktopSettings) { desktopSettings->toFront(true); return; }
        auto* engine = static_cast<IgniteEngine*>(_PUiVirtualEngine.get());
        if (!engine || !engine->songManager->dm || !midiRouter) return;
        juce::DialogWindow::LaunchOptions options;
        options.dialogTitle = "IGNITE Desktop Settings";
        options.content.setOwned(new IgniteDesktop::Settings(*engine,*midiRouter));
        options.dialogBackgroundColour = juce::Colour(0xff25272b);
        options.useNativeTitleBar = true;
        options.resizable = true;
        options.escapeKeyTriggersCloseButton = true;
        desktopSettings = options.launchAsync();
    }
    std::unique_ptr<IgniteDesktop::MidiRouter> midiRouter;
    juce::Component::SafePointer<juce::DialogWindow> desktopSettings;
	std::unique_ptr<MainWindow> mainWindow;
	std::unique_ptr<FileLogger> fileLogger;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(IgniteApplication)
