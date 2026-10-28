#include "Firmware/Application/Model/FrontPannelManager.h"
#include "Firmware/Application/Model/FrontPannelUtility.h"
#include "Firmware/Application/Model/FrontPannelLed.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <system_error> // for std::system_error
#include <regex>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

#if defined FP_UART_LINUX
#include <sys/sysinfo.h>
#if defined FP_SIM_LINUX_REMOTE_SERVER
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
struct sockaddr_in server_addr;
#endif
#endif

#if defined FP_REMOTE_CLIENT_RECIVE_WIN
#include <Windows.h>
#include <SetupAPI.h>
#include <socketapi.h>
struct sockaddr_in server_addr;
#endif

#if defined FP_UART_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <linux/input.h>
#endif //FP_UART_LINUX


#if defined FP_UART_LINUX
#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttymxc0"
#define USB_DEVICE "/dev/ttyVS0"
#if defined FP_USB_SIM_LINUX
#define USB_HOST_DEVICE "/dev/ttyVS0"
#endif
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#endif //FP_UART_LINUX

#ifdef __APPLE__
typedef unsigned long DWORD; // Define DWORD for macOS
#endif

#define HARDWARETEST 0
using namespace Ignite;

std::string LCD_BRIGHTNESS_SYS_PATH = "/sys/class/backlight/backlight/brightness";
const char* USB_UNMOUNT_CMD = "/bin/sh /usr/bin/hostUsbMount.sh 0";
const char* FIRMWARE_UPDATE_CMD = "/bin/swupdate -i /tmp/fwupdate/Ignit*.swu -v  &";
const char* FACTORY_RESET_CMD = "/bin/rm -rf /home/ignite/* /data/ignite/*";
//const char* MCU_RESETPIN_EXPORT = "echo 07 > /sys/class/gpio/export";
//const char* MCU_RESETPIN_DIR_SET = "echo out > /sys/class/gpio/gpio7/direction";
//const char* MCU_RESETPIN_VAL_HIGH = "echo 1 > /sys/class/gpio/gpio7/value";
//const char* MCU_RESETPIN_VAL_LOW = "echo 0 > /sys/class/gpio/gpio7/value";
//const char* MCU_RESETPIN_UNEXPORT = "echo 07 > /sys/class/gpio/unexport";
const char* MCU_MCUTXPIN_EXPORT = "echo 119 > /sys/class/gpio/export";
const char* MCU_MCUTXPIN_DIR_SET = "echo out > /sys/class/gpio/gpio119/direction";
const char* MCU_MCUTXPIN_VAL_HIGH = "echo 1 > /sys/class/gpio/gpio119/value";
const char* MCU_MCUTXPIN_VAL_LOW = "echo 0 > /sys/class/gpio/gpio119/value";
const char* MCU_MCUTXPIN_UNEXPORT = "echo 119 > /sys/class/gpio/unexport";
const char* FW_DECRYPTION_DIR = "/home/ignite/Firmware_Update/";
const char* FW_EXTRACTION_DIR_CMD = "mkdir -p /tmp/fwupdate";
const char* FW_MCU_PATH = "/usr/lib/firmware/mcu/Ignite_MCU.hex";

#define MAX_RETRIES 50 // can't fail :: TODO MIDI send may block song
int8_t mcu_soft_version = 0;
uint32 fwtxcount = 0;

#if defined FP_TEST_KEYBRD  // Test Code
static bool initDone = false;


juce::String LedDsp1_string[61] = {
		"USB", "MENU", "BRAY", "RET", "COPY", "'-'_NAV", "'+'_NAV", 	/* End of Line */
		"RROW", "SAVE", "LAST", "SHIFT", "REC", "BCKFWD", "STOP", "PLAY", "SCULPT", "DELETE", "INSERT", "RBAR",/* End of Line */
		"SONG", "SCENE", "TRACK", "NXT/TIE", "FILL_X", "SLO/SKP", "GANG", "PATTERN", "TEMPO", "INT_SND",/* End of Line */
		"TR1", "TR2", "TR3", "TR4", "TR5", "TR6", "TR7", "TR8", "TR9", "TR10", "TR11", "TR12", "TR13", "TR14", "TR15", "TR16",/* End of Line */
		"TRSL1", "TRSL2", "TRSL3", "TRSL4", "TRSL5", "TRSL6", "TRSL7", "TRSL8", "TRSL9", "TRSL10", "TRSL11", "TRSL12", "TRSL13", "TRSL14", "TRSL15", "TRSL16",/* End of Line */
};
juce::ToggleButton* _toggleButtonArray[61];
uint8_t _toggleButtonArrayVal[61];
bool _toggleButtonArrayVal_needupdate = false;
DialogWindow::LaunchOptions Diaoption;
juce::DialogWindow* Dwind;
bool readwait = true;

class FPKeys : public juce::Component,
	public juce::KeyListener,
	public juce::MouseListener
{
public:
	FPKeys()
	{
		Component::setSize(500, 500);
		Component::setTopLeftPosition(0, 0);
		Component::setFocusContainer(true);
		Component::addKeyListener(this);
	};
	~FPKeys();
	int SetKeys()
	{
		addAndMakeVisible(FPInfo = new Label);
		FPInfo->setText("Use mapped keys from Keyboard", juce::dontSendNotification);
		FPInfo->setFont(juce::Font(16.0f, juce::Font::bold));
		FPInfo->setColour(juce::Label::textColourId, Colours::black);
		FPInfo->setBounds(10, 10, 400, 50);

		int j = 0, k = 1;
		for (int i = 0; i < 61; i++)
		{
			_toggleButtonArrayVal[i] = 0;
			addAndMakeVisible(_toggleButtonArray[i] = new juce::ToggleButton);
			_toggleButtonArray[i]->setButtonText(LedDsp1_string[i]);
			_toggleButtonArray[i]->setToggleState(_toggleButtonArrayVal[i], false);
			_toggleButtonArray[i]->setBounds(10 * k, 50 + j, 400, 50);
			_toggleButtonArray[i]->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
			_toggleButtonArray[i]->setColour(juce::ToggleButton::tickColourId, juce::Colours::darkred);
			_toggleButtonArray[i]->setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);
			k += 8;
			// ( 7 || 2 || 10 || 16 || 16) // number of LED in each line
			if (i == 6 || i == 18 || i == 28 || i == 44 || i == 60)
			{
				j += 20;
				k = 1;
			}
		}

		/*
		juce::String infoText;
		addAndMakeVisible(FPLog = new Label);
		FPLog->setFont(juce::Font(16.0f, juce::Font::italic));
		FPLog->setColour(juce::Label::textColourId, Colours::brown);
		infoText << " Key Mapping \n";
		infoText << " Left/Right arrow = Rotary Left/Right \n";
		infoText << " Down Arrow = Enter/Rotary Push \n";
		infoText << " Key Z = Song \n";
		infoText << " Key X = Scene \n";
		infoText << " Key C = Track \n";
		infoText << " Key V = Pattern \n";
		infoText << " Key M = Menu \n";
		infoText << " Delete & Page Down = '+' & '-' Key  \n";
		infoText << " Insert  = Return/back \n";
		infoText << " Page UP = Copy Key \n";
		infoText << " Number 1 -8  = TRACK 1-8 \n";
		infoText << " SHIFT + Number 1 -8  = TRACK 9-16 \n";
		FPLog->setText(infoText, juce::dontSendNotification);
		FPLog->setBounds(10, 20, 400, 400);
		*/

		return 1;
	}
	SysMessage getKey()
	{
		SysMessage _SysMessage = { 0, };
		if (!readwait && !sysMessageList.empty())
		{
			_SysMessage = (sysMessageList.front());
			sysMessageList.pop_front();
		}
		this->postCommandMessage(1);
		return _SysMessage;
	}
	void paint(Graphics& g)
	{
		//		g.fillAll(Colours::burlywood);
	}
	bool keyPressed(const KeyPress& key, Component* originatingComponent)
	{
		bool MessageKeyCode = false;
		uint8_t Keyinput = 1;
		KeyCodeValue keyCodeValue = { 0, };
		keyCodeValue.KeyValue = 1;
		readwait = true;
		int keyTmp = key.getKeyCode();
		switch (keyTmp)
		{
		case 65575: //leftKey
			if (key.getModifiers().isShiftDown())
				keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_PAGESHIFT;
			else
				keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_RMOVELEFT;
			break;
		case 65573: //rightKey
			if (key.getModifiers().isShiftDown())
				keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_PAGESHIFT;
			else
				keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_RMOVERIGHT;
			break;
		case 65576: //Down
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_RSELECT;
			break;
		case 65582: //Del
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_MINUS_NAV;
			break;
		case 65570: //Page Down
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_PLUS_NAV;
			break;
		case 65581: //Insert
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_RETURN_NAV;
			break;
		case 65569: //Page up
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_COPY_NAV;
			break;
		case 65: //A
		case 97:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_SAVE;
			break;
		case 66: //b
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_ROTARYBAR;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 83: //S
		case 115:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_LAST;
			break;
		case 68: //D
		case 100:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_SHIFT;
			break;
		case 90: //z
		case 122:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_SONG;
			break;
		case 88: //x
		case 120:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_SCENE;
			break;
		case 67: //c
		case 99:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_TRACK;
			break;
		case 82: //r
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_ROTARYROW;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 69: //e
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_RESONANCE;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 73: //i
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_EQHI;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 81: //q
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_RELEASE;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 84: //t
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_VOLUME;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 85: //u
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_CHORUS;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 87: //w
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_FILTER;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 89: //y
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_REVERBE;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 79: //o
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_EQLO;
			(key.getModifiers().isShiftDown()) ? keyCodeValue.KeyValue = 2 : keyCodeValue.KeyValue = 1;
			break;
		case 49: //num 1 or shift + num 1
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL9 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL1;
			break;
		case 50:
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL10 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL2;
			break;
		case 51:
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL11 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL3;
			break;
		case 52:
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL12 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL4;
			break;
		case 53:
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL13 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL5;
			break;
		case 54:
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL14 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL6;
			break;
		case 55:
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL15 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL7;
			break;
		case 56://num 8
			keyCodeValue.KeyParam = (key.getModifiers().isShiftDown()) ? (paramId_t)FrontPannelParam::paramKey_TRACKSEL16 : (paramId_t)FrontPannelParam::paramKey_TRACKSEL8;
			break;
		case 86:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_PATTERN;
			break;
		case 77:
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_MENU;
			break;
		case 93:// ]
		{
			MessageKeyCode = true;
			SysMessage _sysMessage;
			_sysMessage.Header = SYS_MIDI_MSG;
			_sysMessage.Direction = DIRECTION_IN;
			_sysMessage.Id = 0;//Port ID
			_sysMessage.size = 1;
			_sysMessage.Data[0] = 0xF8;
			sysMessageList.push_back(_sysMessage); //Press
		}
		case 91:// [
		{
			MessageKeyCode = true; // To avoid sending the key code

			SysMessage _sysMessage;
			_sysMessage.Header = SYS_KEY_MSG;
			_sysMessage.Direction = DIRECTION_IN;
			_sysMessage.Data[0] = 1;

			_sysMessage.Id = (paramId_t)FrontPannelParam::paramKey_SONG;
			sysMessageList.push_back(_sysMessage);

			_sysMessage.Id = TEST_DELAY_ID; // delay 2+ sec
			sysMessageList.push_back(_sysMessage);

			_sysMessage.Id = (paramId_t)FrontPannelParam::paramKey_SONG;
			sysMessageList.push_back(_sysMessage);

			_sysMessage.Id = TEST_DELAY_ID; // delay 2+ sec
			sysMessageList.push_back(_sysMessage);

			_sysMessage.Id = TEST_DELAY_ID; // delay 2+ sec // nested delay for 4+ sec
			sysMessageList.push_back(_sysMessage);

			_sysMessage.Id = (paramId_t)FrontPannelParam::paramKey_SCENE;
			sysMessageList.push_back(_sysMessage);
			_sysMessage.Id = TEST_DELAY_ID; // delay 2+ sec
			sysMessageList.push_back(_sysMessage);

			_sysMessage.Id = (paramId_t)FrontPannelParam::paramKey_TRACK;
			sysMessageList.push_back(_sysMessage);
		}
		break;
		case 70://f
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_REC;
			break;
		case 71://g
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_BACKFORWARD;
			break;
		case 72://h
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_STOP;
			break;
		case 74://j
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_PLAY;
			break;
		case 75://k
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_SAVE;
			break;
		case 76://l
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_DELETE;
			break;
		case 80://p
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_GANG;
			break;
		case 78://n
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_SOLO_SKIP;
			break;
			//case 66://b
			//	keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_FILL_X;
			//	break;
		case 44://<
			keyCodeValue.KeyParam = (paramId_t)FrontPannelParam::paramKey_NEXT_TIE;
			break;
		default:
			//Key not used
			FPInfo->setText("Use mapped keys from Keyboard : Key Not Mapped", juce::dontSendNotification);
			FPInfo->setColour(juce::Label::outlineColourId, Colours::red);
			break;
		}
		if (keyCodeValue.KeyParam != 0 && (MessageKeyCode == false))
		{
			SysMessage _sysMessage;
			_sysMessage.Header = SYS_KEY_MSG;
			_sysMessage.Direction = DIRECTION_IN;
			_sysMessage.Id = keyCodeValue.KeyParam;
			_sysMessage.Data[0] = keyCodeValue.KeyValue;
			sysMessageList.push_back(_sysMessage);


			FPInfo->setText("Use mapped keys from Keyboard : Pressed Valid Key", juce::dontSendNotification);
			FPInfo->setColour(juce::Label::outlineColourId, Colours::darkblue);
		}
		readwait = false;
		return true;
	}
	bool keyStateChanged(bool isKeyDown, Component* originatingComponent)
	{
		return true;
	}
	void handleCommandMessage(int id)
	{
		Component* comp = getCurrentlyModalComponent();

		if (comp != this && comp != nullptr)
		{
			if (GetFocusagain && isCurrentlyBlockedByAnotherModalComponent())
			{
				comp->addKeyListener(this); //over ride
				Dwind->grabKeyboardFocus();
				TopLevelWindow* top = TopLevelWindow::getActiveTopLevelWindow();
				this->setFocusContainerType(FocusContainerType::keyboardFocusContainer);
				GetFocusagain = false;
			}
		}
		else
			GetFocusagain = true;
	}
private:
	bool GetFocusagain = true;
	Label* FPInfo;
	Label* FPLog;
	std::list<SysMessage> sysMessageList;
};
FPKeys::~FPKeys()
{}
FPKeys* fpKeys;

void FrontPannelManager::handleCommandMessage(int id)
{
	if (id == 1)
	{
		fpKeys = new FPKeys;
		fpKeys->SetKeys();
	}
	else if (id == 2)
	{
		Diaoption.content.setOwned(fpKeys);
		Diaoption.content->setSize(1300, 200); // may get crash if the size is less than other Dialog window
		Diaoption.dialogTitle = "Font pannel Key simulator ";
		//Diaoption.dialogBackgroundColour = juce::Colour(0xff00AAEE);
		Diaoption.dialogBackgroundColour = Colours::burlywood;
		Diaoption.escapeKeyTriggersCloseButton = true;
		Diaoption.useNativeTitleBar = false;
		Diaoption.resizable = true;
		//Diaoption.create()->setVisible(true);
		Dwind = Diaoption.create();
		Dwind->setVisible(true);
		Dwind->grabKeyboardFocus();
		Dwind->addToDesktop();
		Dwind->addKeyListener(fpKeys);
		Dwind->setTopLeftPosition(10, 600);
	}
	else if (3)
	{
		_toggleButtonArrayVal_needupdate = false;
		for (int i = 0; i < 61; i++)
		{
			_toggleButtonArray[i]->setToggleState(_toggleButtonArrayVal[i], false);
		}
	}
}
#else
void FrontPannelManager::handleCommandMessage(int id)
{
}
#endif//#if defined FP_TEST_KEYBRD
FrontPannelManager::FrontPannelManager(IParamChangeListener* listener, IgniteEngine& model) : paramlistener(listener), engine(model),
FrontPannelExternalReadThread("FrontPanneIN", IgniteConstants::threadStackSize, osPriorityHigh),
FrontPannelExternalWriteThread("FrontPanneOUT", IgniteConstants::threadStackSize, osPriorityHigh),
FrontPannelInternalProcessThread("FrontPannelHandle", IgniteConstants::threadStackSize, osPriorityHigh),
FrontPannelLinuxWatchThread("FrontPannelLinuxWatch", IgniteConstants::threadStackSize, osPriorityHigh),
FrontPanneEthernetWatchThread("FrontPannelEthernetWatch", IgniteConstants::threadStackSize, osPriorityHigh),
FrontPanneExternalReadQ(IgniteConstants::FPmsgQueueSize),
FrontPanneExternalWriteQ(IgniteConstants::FPmsgQueueSize),
juce::Thread("FrontPannelDecodeThread"),

Module("FrontPannelManager", (moduleId_t)ModuleName::moduleFrontPannel, (unsigned int)FrontPannelParam::paramFrontPannelMax, listener)
{

	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_MENU, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_BRAY, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_ENTER_NAV, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_MINUS_NAV, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_PLUS_NAV, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_RETURN_NAV, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_COPY_NAV, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_ROTARYROW, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACK, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_SCENE, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_USB_STOP, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_SAVE, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_SONG, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_PATTERN, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TEMPO, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_INTERNALSOUNDS, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_LAST, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_REC, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_STOP, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_SCULPT, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_INSERT, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_SHIFT, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_BACKFORWARD, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_PLAY, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_DELETE, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_FILL_X, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_NEXT_TIE, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_SOLO_SKIP, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_GANG, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_ROTARYBAR, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL1, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL2, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL3, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL4, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL5, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL6, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL7, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL8, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL9, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL10, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL11, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL12, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL13, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL14, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL15, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKSEL16, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY1, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY2, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY3, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY4, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY5, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY6, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY7, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY8, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY9, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY10, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY11, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY12, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY13, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY14, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY15, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_TRACKROTARY16, 0, 127, 0);
	//Key Internal values, check ROTARY Navi keys ?
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_RMOVELEFT, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_RMOVERIGHT, 0, 255, 0);

	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_RELEASE, 0, 255, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_FILTER, 0, 255, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_RESONANCE, 0, 255, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_VOLUME, 0, 255, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_REVERBE, 0, 255, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_CHORUS, 0, 255, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_EQHI, 0, 255, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::paramKey_EQLO, 0, 255, 0);

	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_RSELECT, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_PAGESHIFT, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_MouseClick, 0, 255, 0); //not a FP Key; this to handle mouse click in UI

	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_ROTARY_NAV, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_ROW, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT1, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT2, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT3, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT4, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT5, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT6, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT7, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT8, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT9, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT10, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT11, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT12, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT13, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT14, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT15, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_PADROT16, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramRot_BAR, 0, 255, 0);

	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP1, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP2, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP3, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP4, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP5, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP6, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP7, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP8, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP9, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP10, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP11, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP12, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP13, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP14, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP15, 0, 255, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::param_RGB_DISP16, 0, 255, 0);

	AddParameter<float>((paramId_t)FrontPannelParam::paramKey_TempoValue, 20.0, 300.0, 130.0);

	//System Events
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_LCD_Brightness, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_FPLED_Brightness, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_PowerKey_Event, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_USBStop, 0, 127, 0);

	AddParameter<std::string>((uint8_t)FrontPannelParam::paramKey_getSerialNumber, "0", "0", "0");
	AddParameter<std::string>((uint8_t)FrontPannelParam::paramKey_getFirmwareVersion, "0", "0", "0");
	AddParameter<std::string>((uint8_t)FrontPannelParam::paramKey_getFirmwareBuildInfo, "0", "0", "0");
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_Firmware_Update_Source, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_Firmware_Update, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)FrontPannelParam::paramKey_Valid_Firmware_check, -10, 10, 0);

	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_Valid_Firmware_check, 0, 127, 0);
	AddParameter<int>((paramId_t)FrontPannelParam::paramKey_EmmcGetTotalSize, 0, 127, 0);
	AddParameter<int>((paramId_t)FrontPannelParam::paramKey_EmmcGetFreeSize, 0, 127, 0);
	AddParameter<int>((paramId_t)FrontPannelParam::paramKey_UsbGetTotalSize, 0, 127, 0);
	AddParameter<int>((paramId_t)FrontPannelParam::paramKey_UsbGetFreeSize, 0, 127, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::paramKey_UsbGetConnectionStatus, 0, 1, 0);
	AddParameter<int>((paramId_t)FrontPannelParam::paramKey_RamGetTotalSize, 0, 127, 0);
	AddParameter<int>((paramId_t)FrontPannelParam::paramKey_RamGetFreeSize, 0, 127, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::paramKey_FactoryReset, 0, 1, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_HwTestMode, 0, 10, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::paramKey_KeyDataReady, 0, 1, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_FwUpgrade, 0, 10, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::paramKey_DataCorrupted, 0, 1, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_McuSleepMode, 0, 10, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_decryptFirmwareImage, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)FrontPannelParam::paramKey_ExtractFirmwareImage, 0, 127, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::param_SyncOut, 0, 288, 0);
	AddParameter<uint16_t>((paramId_t)FrontPannelParam::param_SyncIn, 0, 288, 0);
	AddParameter<int>((paramId_t)FrontPannelParam::paramKey_DevStatus, -1, 10, -1);
	AddParameter<bool>((paramId_t)FrontPannelParam::paramKey_RebootDevice, 0, 1, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::param_removeFirmwarePacakge, 0, 1, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::param_startPlayTribeUsbHandler, 0, 1, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::param_SOMFwUpdate, 0, 1, 0);
	AddParameter<int8_t>((paramId_t)FrontPannelParam::paramSaveAll, 0, 1, 0);
	AddParameter<int8_t>((paramId_t)FrontPannelParam::param_MCU_FWVERSION, 0, 1, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::param_startPlayTribeWifiHandler, 0, 1, 0);
	AddParameter<bool>((paramId_t)FrontPannelParam::param_stopPlayTribeWifiHandler, 0, 1, 0);

	FpUtility = new FrontPannelUtility(this);
	FpLedHandler = new FrontPannelLed(this);
//#if defined PLAYTRIBE 
	usbFunctionFS = new USBFunctionFS(this,engine);
//#endif
	playTribeConMngr = new PlayTribeConnectionManager(this);

	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleFrontPannel, true);
	startThread();

	for (uint8_t i = 0; i < (uint8_t)FrontPannelParam::paramKey_End_KeyMapping; i++)
	{
		KeyStatus[i] = (uint8_t)FrontPannelKeyStatus::paramKeyStatusIdle;
	}
	/*
		SW Release		: MCU <---> iMX <---> Engine <--> UI

		Debug			: MCU <---> iMX <---> Engine <---> UI and Engine <---> Windows
		Debug Simulator	: iMX <---> USB <---> Simulator

		FP_USB_SIM_LINUX      -- enable linux code to send/recive to Windows

		FP_UART_LINUX         -- iMX to MCU communication
	*/
#ifdef FP_REMOTE_CLIENT_RECIVE_WIN//wait and connect to device or script
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) == SOCKET_ERROR) {
		DBG(" FP_WIN_ETHERNET_CLIENT : WSAStartup  failed ");
	}
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket.load() == -1) {
		DBG("FP_WIN_ETHERNET_CLIENT: clientSocket  failed");
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080); // Port number
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.1"); // Listen on all available IPs
	int status = -1;
	while (status != 0)
	{
		status = connect(clientSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));//wait in loop till connect
	}
#endif // FP_WIN_ETHERNET_CLIENT

#if defined FP_UART_LINUX
	{
		int c, res;
		struct termios ttyMcu;

		ttyMcuFileHandler = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
		if (ttyMcuFileHandler < 0)
		{
			//TODO: reset MCU  OR reset imx tty
			DBG("MCU - ttymxc0: setup failed ---!");
		}
		bzero(&ttyMcu, sizeof(ttyMcu));

		//cc_flag
		ttyMcu.c_cflag = (BAUDRATE | CLOCAL | CREAD);
		ttyMcu.c_cflag &= ~PARENB;
		ttyMcu.c_cflag &= ~CSTOPB;
		ttyMcu.c_cflag &= ~CSIZE;
		ttyMcu.c_cflag |= CS8;
		ttyMcu.c_cflag &= ~CRTSCTS;

		ttyMcu.c_lflag &= ~ICANON;
		ttyMcu.c_lflag &= ~ECHO;
		ttyMcu.c_lflag &= ~ECHOE;
		ttyMcu.c_lflag &= ~ECHONL;
		ttyMcu.c_lflag &= ~ISIG;

		//c_iflag
		ttyMcu.c_iflag &= ~(IXON | IXOFF | IXANY | ISIG);
		ttyMcu.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
		ttyMcu.c_iflag = IGNPAR;

		//c_oflag
		ttyMcu.c_oflag = 0;
		ttyMcu.c_oflag &= ~OPOST;
		ttyMcu.c_oflag &= ~ONLCR;

		/* set input mode (non−canonical, no echo,...) */
		ttyMcu.c_cc[VTIME] = 10; /* inter−character timer unused */
		ttyMcu.c_cc[VMIN] = 0; /* blocking read until each chars received */

		cfmakeraw(&ttyMcu);
		cfsetispeed(&ttyMcu, BAUDRATE);
		cfsetospeed(&ttyMcu, BAUDRATE);

		tcflush(ttyMcuFileHandler, TCIOFLUSH);
		tcsetattr(ttyMcuFileHandler, TCSANOW, &ttyMcu);
#if defined FP_SIM_LINUX_REMOTE_SERVER
		int flags = fcntl(ttyMcuFileHandler, F_GETFL, 0);
		if (fcntl(ttyMcuFileHandler, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			std::cout << "ttyMcuFileHandler: fcntl set O_NONBLOCK failed" << std::endl;
		}
#endif //FP_SIM_LINUX_REMOTE_SERVER
		DBG("MCU - ttymxc0: setup done");
	}
#endif// FP_UART_LINUX
}

#if defined FP_SIM_LINUX_REMOTE_SERVER
void FrontPannelManager::FrontPanneEthernetWatchThreadFunction(void)
{
	FrontPanneEthernetWatchThread.delayMs(FRONT_PANEL_THREAD_INIT_WAIT); //Wait till Init Complete
	int serverSocket;
	bool connected = false;
	struct sockaddr_in server_addr;

	while (!connected)
	{
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == -1) {
			std::cerr << "FP_SIM_LINUX_REMOTE_SERVER: serverSocket failed" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		int opt = 1;
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(8080); // Port number
		server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all available IPs

		if (bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
			std::cerr << "FP_SIM_LINUX_REMOTE_SERVER: bind failed (wait or restart App)" << std::endl;
			close(serverSocket);
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		if (listen(serverSocket, 5) == -1) {
			std::cerr << "FP_SIM_LINUX_REMOTE_SERVER: listen failed (wait or restart App)" << std::endl;
			close(serverSocket);
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}
		connected = true;
		break;
	}

	sockaddr_in clientAddress;
	socklen_t clientAddrLen = sizeof(clientAddress);

	while (!CThread::desktopShutdownRequested())
	{
		if (clientSocket.load() == -1)
		{
			int socket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrLen);
			if (socket == -1)
			{
				//std::cerr << "FP_SIM_LINUX_REMOTE_SERVER: accept failed" << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			else
			{
				clientSocket.store(socket);
				int flags = fcntl(clientSocket, F_GETFL, 0);
				fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
			}
			std::cout << "FP_SIM_LINUX_REMOTE_SERVER: Client connected @ : " << clientSocket << std::endl;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}

	close(serverSocket);
}
#endif //FP_SIM_LINUX_REMOTE_SERVER

void FrontPannelManager::FrontPannelStartThreads(void)
{
	FrontPannelExternalReadThreadFunctionCB = [](void* context) { static_cast<FrontPannelManager*> (context)->FrontPannelExternalReadThreadFunction(); };
	FrontPannelExternalReadThread.startThread(FrontPannelExternalReadThreadFunctionCB, this);
	FrontPannelExternalWriteThreadFunctionCB = [](void* context) { static_cast<FrontPannelManager*> (context)->FrontPannelExternalWriteThreadFunction(); };
	FrontPannelExternalWriteThread.startThread(FrontPannelExternalWriteThreadFunctionCB, this);
	FrontPannelInternalProcessThreadFunctionCB = [](void* context) { static_cast<FrontPannelManager*> (context)->FrontPannelInternalProcessThreadFunction(); };
	FrontPannelInternalProcessThread.startThread(FrontPannelInternalProcessThreadFunctionCB, this);
	FrontPannelLinuxWatchThreadFunctionCB = [](void* context) { static_cast<FrontPannelManager*> (context)->FrontPannelLinuxWatchThreadFunction(); };
	FrontPannelLinuxWatchThread.startThread(FrontPannelLinuxWatchThreadFunctionCB, this);

#if defined FP_SIM_LINUX_REMOTE_SERVER
	FrontPanneEthernetWatchThreadFunctionCB = [](void* context) { static_cast<FrontPannelManager*> (context)->FrontPanneEthernetWatchThreadFunction(); };
	FrontPanneEthernetWatchThread.startThread(FrontPanneEthernetWatchThreadFunctionCB, this);
#endif
}
void FrontPannelManager::shutdownDesktop()
{
#ifdef _WIN32
    stopTimer();
    signalThreadShouldExit();
    juce::Thread::stopThread(3000);
    FrontPannelExternalReadThread.stopThread();
    FrontPannelExternalWriteThread.stopThread();
    FrontPannelInternalProcessThread.stopThread();
    FrontPannelLinuxWatchThread.stopThread();
    FpLedHandler->FrontPannelLedExecuteThread.stopThread();
#endif
}

//Note: Get data from the given position, calling code need to check the boundary
uint8_t FrontPannelManager::FrontPannelGetDataAt(uint32_t pos, bool incRxPos)
{
	uint32_t tmpPos = (McuRxRead.load() + pos) % MCU_RX_BUFF_MAX_SIZE;
	uint8_t tmp = UartMcuRxBuffer[tmpPos];

	if (incRxPos)
		McuRxRead.store(tmpPos);

	return tmp;
}
void FrontPannelManager::HandleSpecialKey()
{
	if (KeyStatus[(uint8_t)FrontPannelParam::paramKey_STOP] == (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed ||
		KeyStatus[(uint8_t)FrontPannelParam::paramKey_STOP] == (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
	{
		engine.songManager->handleMcuSpecialKey((uint8_t)FrontPannelParam::paramKey_STOP, 1);
	}
	if (KeyStatus[(uint8_t)FrontPannelParam::paramKey_PLAY] == (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed ||
		KeyStatus[(uint8_t)FrontPannelParam::paramKey_PLAY] == (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
	{
		engine.songManager->handleMcuSpecialKey((uint8_t)FrontPannelParam::paramKey_PLAY, 1);
	}
}
//void FrontPannelManager::FrontPannelDecodeMessage(void)
void FrontPannelManager::run()
{
	uint8_t data = 0;
	uint32_t count = 0;
	while (!threadShouldExit())
	{
		uint32_t DataToProcess = (McuRxWrite.load() == McuRxRead.load()) ? 0 :
			(McuRxWrite.load() > McuRxRead.load()) ?
			(McuRxWrite.load() - McuRxRead.load() + 1) :
			((MCU_RX_BUFF_MAX_SIZE - McuRxRead.load()) + McuRxWrite.load() + 1);

		if ((McuRxWrite.load() + 1) % MCU_RX_BUFF_MAX_SIZE == McuRxRead.load()) // Buffer is full
		{
			if (errorSkipCount == 0)
			{
				uint32_t tmpPos = (McuRxRead.load() + 50) % MCU_RX_BUFF_MAX_SIZE;
				McuRxRead.store(tmpPos);
			}
			else
			{
				uint32_t tmpPos = (McuRxRead.load() + errorSkipCount) % MCU_RX_BUFF_MAX_SIZE;
				McuRxRead.store(tmpPos);
			}
			std::cout << "FP: ERROR: Buffer is full, skipped data" << std::endl;
			//wait(10);
			continue;
		}
		if (DataToProcess < 4)
		{
			wait(1);
			continue;
		}

		SysMessage _sysMessage;
		_sysMessage.Direction = DIRECTION_IN;

		switch (UartMcuRxBuffer[McuRxRead.load()])
		{
		case SYS_KEY_MSG: //Key Handling TODO: Change to define or enum
			if (DataToProcess > 4)
			{
				data = FrontPannelGetDataAt(MCU_DATA_SIZE_POS, false);
				if ((data) && DataToProcess > (uint32_t)(data + MCU_HEADRD_SIZE)) // we recived enough data to process
				{
					FrontPannelGetDataAt(MCU_DATA_SIZE_POS, true); // Move to pos
					KeyReady = false;//Make ui wait till it complete
					uint8_t keycount = 0;
					bool statusChanged = false;
					char szBuff[100] = { 0 };
					multiKeyPressed = false;
					std::unique_lock<std::mutex> lock(keyReadmutex);//Lock and use as UI may request data from Keyparamlist, engine could clear it if multiple key pressed at fast rate.
					Keyparamlist.clear();
					for (auto i = 0; i < data; i++)
					{
						szBuff[i] = FrontPannelGetDataAt(1, true); //read 1 data
					}
					for (auto bytesIndex = 0; bytesIndex < 12; bytesIndex++) //12 address location, for 8 bit data, each bit denote a KEY which mapped as param
					{
						for (auto bitIndex = 0; bitIndex < 8; bitIndex++)
						{
							if (McuToUiMap.find(keycount) != McuToUiMap.end())// has Mapping, some GPIO not used in MCU
							{
								uint8_t value = ((szBuff[bytesIndex] >> bitIndex) & 1);//each bit map to a key/GPIO
								count += value;
								IgniteParamList tempParam;
								tempParam.param = (uint8_t)McuToUiMap.at(keycount); // MCU keys reg value mapped to param, check FrontPannelMcuMap
								uint8_t tmpStatus = KeyStatus[tempParam.param];
								uint8_t& keyStatus = KeyStatus[tempParam.param];

								switch (keyStatus) {
								case (uint8_t)FrontPannelKeyStatus::paramKeyStatusIdle:
									if (value) {
										keyStatus = (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed;
										KeypressCount[tempParam.param] = 1; // Initialize counter
									}
									break;
								case (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed:
									if (value) {
										KeypressCount[tempParam.param]++;
										if (KeypressCount[tempParam.param] >= FP_KEYPRESS_TO_HOLD) {
											keyStatus = (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold;
											KeypressCount[tempParam.param] = 0; // Reset counter to handle next hold to UI
										}
									}
									else {
										keyStatus = (uint8_t)FrontPannelKeyStatus::paramKeyStatusReleased;
										KeypressCount[tempParam.param] = 0; // Reset counter
									}
									break;
								case (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold:
									keyStatus = value ? (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold : (uint8_t)FrontPannelKeyStatus::paramKeyStatusReleased;
									KeypressCount[tempParam.param]++;
									if (KeypressCount[tempParam.param] >= FP_KEYPRESS_TO_HOLD)// HOLD need to be sent again
									{
										statusChanged = value;
										KeypressCount[tempParam.param] = 0; // Reset counter to handle next hold to UI
									}
									break;
								case (uint8_t)FrontPannelKeyStatus::paramKeyStatusReleased:
									if (value) {
										keyStatus = (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed;
										KeypressCount[tempParam.param] = 1; // Reset counter on new press
									}
									else {
										keyStatus = (uint8_t)FrontPannelKeyStatus::paramKeyStatusIdle;
										KeypressCount[tempParam.param] = 0; // Ensure counter is reset
									}
									break;
								default:
									keyStatus = (uint8_t)FrontPannelKeyStatus::paramKeyStatusIdle;
									KeypressCount[tempParam.param] = 0; // Ensure counter is reset
									break;
								}
								multiKeyPressed = (count > 1);
								tempParam.startPos = KeyStatus[(uint8_t)tempParam.param]; //Not using Anytype to avoid multiple calls, #improve performance
								tempParam.Count = multiKeyPressed; //more than one key detected; will update the 1st param ; which is used in UI
								tempParam.MsgValue = 0;
								Keyparamlist.push_back(tempParam);

								if (((paramId_t)tempParam.param == ((paramId_t)FrontPannelParam::paramKey_TEMPO)) && (tempParam.startPos == (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed))
								{
									FpUtility->MidiClockDetect(MCU_KEY_TEMP_KEY_ID);
								}
								else if ((paramId_t)tempParam.param == ((paramId_t)FrontPannelParam::paramKey_PAGESHIFT))
								{
									if (KeyStatus[(uint8_t)tempParam.param] == (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed || KeyStatus[(uint8_t)tempParam.param] == (uint8_t)FrontPannelKeyStatus::paramKeyStatusOnHold)
										ShiftKeyPressed = true;
								}
								if (tmpStatus != KeyStatus[tempParam.param])
								{
									statusChanged = true;
								}
								//if ((szBuff[bytesIndex] >> bitIndex) & 1)
								//{
								//	_sysMessage.Id = (uint8_t)McuToUiMap.at(keycount); // MCU keys reg value mapped to param, check FrontPannelMcuMap
								//	_sysMessage.Data[0] = 1;
								//	FrontPanneExternalReadQ.put(_sysMessage, TimeOut::kNoWait);
								//	//DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Key code to UI " + String(keycount));
								//}
							}
							keycount++;
						}
						HBState = false;
					}
					KeyReady = true;
					KeyReady_cv.notify_one();
					if (statusChanged)
					{
						Keyparamlist.begin()->Count = multiKeyPressed;
						_sysMessage.Header = SYS_KEY_MSG;
						_sysMessage.Id = (paramId_t)FrontPannelParam::paramKey_KeyDataReady;
						_sysMessage.Data[0] = 1;
						FrontPanneExternalReadQ.put(_sysMessage, TimeOut::kNoWait);
						HandleSpecialKey();
					}
					FrontPannelGetDataAt(1, true); //Handled move to next hearder
				}
			}
			break;
		case SYS_KNOB_MSG:
			if (DataToProcess > 4)
			{
				data = FrontPannelGetDataAt(MCU_DATA_SIZE_POS, false);// we recived enough data to process ?
				if (data && DataToProcess > (uint32_t)(data + MCU_HEADRD_SIZE))
				{
					data = FrontPannelGetDataAt(MCU_DATA_SIZE_POS, true); // Move to pos
					char szBuff[100] = { 0 };
					for (auto i = 0; i < data; i++)
					{
						szBuff[i] = FrontPannelGetDataAt(1, true);//read 1 data
					}
					_sysMessage.Header = SYS_KNOB_MSG;
					_sysMessage.Id = (uint8_t)McuToUiMap.at(szBuff[0]); // MCU keys reg value mapped to param, check FrontPannelMcuMap
					_sysMessage.Data[0] = szBuff[1];
					DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Rottary Knob to UI " + String(szBuff[0]) + "Value" + String(szBuff[1]));

					auto now = std::chrono::system_clock::now();
					auto it = messageTimestamps.find(_sysMessage.Id);

					if (it == messageTimestamps.end() ||
						std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count() > 100)
					{
						messageTimestamps[_sysMessage.Id] = now;
						FrontPanneExternalReadQ.put(_sysMessage, TimeOut::kNoWait);
					}
					FrontPannelGetDataAt(1, true); //Handled move to next hearder
					HBState = false;
				}
			}
			break;
		case SYS_MIDI_MSG:
			if (DataToProcess > 4)
			{
				_sysMessage.Header = SYS_MIDI_MSG;

				data = FrontPannelGetDataAt(MCU_DATA_SIZE_POS, false);
				if (data && DataToProcess > (uint32_t)(data + MCU_HEADRD_SIZE))// we recived enough data to process ?
				{
					_sysMessage.Id = (uint8_t)FrontPannelGetDataAt(2, false); //Port_ID - not moving position
					_sysMessage.size = data = FrontPannelGetDataAt(3, true); // Move to pos
					for (auto i = 0; ((i < data) && (i < SYSMESSAGE_MAX_DAT_SIZE)); i++)
					{
						_sysMessage.Data[i] = FrontPannelGetDataAt(1, true);//read 1 data
					}
					DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "MIDI Data from - " + String(_sysMessage.Id));
					FrontPanneExternalReadQ.put(_sysMessage, TimeOut::kNoWait);
					FrontPannelGetDataAt(1, true); //Handled move to next hearder
					HBState = false;
				}
			}
			break;
		case SYS_SYTM_MSG:
			if (DataToProcess > 4)
			{
				_sysMessage.Header = SYS_SYTM_MSG;
				data = FrontPannelGetDataAt(MCU_DATA_SIZE_POS, false);
				if (data && DataToProcess > (uint32_t)(data + MCU_HEADRD_SIZE))// we recived enough data to process ?
				{
					if ((uint8_t)FrontPannelGetDataAt(2, false) == SYS_SYTM_SW_UPGRADE)
					{
						HandleFwUpgrade((uint8_t)FrontPannelGetDataAt(4, true));
						FrontPannelGetDataAt(1, true); //Handled move to next hearder
					}
					else if ((uint8_t)FrontPannelGetDataAt(2, false) == SYS_MCU_HB_STATE)
					{
						(uint8_t)FrontPannelGetDataAt(4, true);
						FrontPannelGetDataAt(1, true); //Handled move to next hearder
					}
					else if ((uint8_t)FrontPannelGetDataAt(2, false) == SYS_MCU_FW_VERSION)
					{
						mcu_soft_version=(int8_t)FrontPannelGetDataAt(4, true);
						std::cout << "MCU FIRMWARE VERSION: " << mcu_soft_version << std::endl;
						FrontPannelGetDataAt(1, true); //Handled move to next hearder
						
					}
					else if ((uint8_t)FrontPannelGetDataAt(2, false) == SYS_MCU_READY)
					{
						(int8_t)FrontPannelGetDataAt(4, true);
						HandleSystemMsg(SYS_MCU_SYNC_STATE, 0x00); //MCU SYNC MSG SEND
						std::cout << "MCU FW VERSION REQESTED" << std::endl;
						HandleSystemMsg(SYS_MCU_FW_VERSION, 1); //
						FpLedHandler->startLedExecuteThread();
						std::cout << "LED DATA UPDATED" << std::endl;
						FrontPannelGetDataAt(1, true); //Handled move to next hearder
					}

					HBState = false;
				}
			}
			break;
		case SYS_SYNC_MSG:
			if (DataToProcess > 4)
			{
				_sysMessage.Header = SYS_SYTM_MSG;
				data = FrontPannelGetDataAt(MCU_DATA_SIZE_POS, false);
				if (data && DataToProcess > (uint32_t)(data + MCU_HEADRD_SIZE))// we recived enough data to process ?
				{
					if ((uint8_t)FrontPannelGetDataAt(2, false) == SYNC_IN)
					{
						//send to Engine
						engine.setParam((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSyncIn, (FrontPannelGetDataAt(5, false) << 8) | FrontPannelGetDataAt(4, false));
						float freq = static_cast<float>(((FrontPannelGetDataAt(5, false) << 8) | FrontPannelGetDataAt(4, false)));
						float decn = static_cast<float>(FrontPannelGetDataAt(6, false));
						freq += (decn / 100);
						engine.setParam((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramSyncIn, freq);
						FrontPannelGetDataAt(7, true);//Handled move to next header
					}
				}
			}
			break;
		case SYS_BRAY_MSG:
			if (DataToProcess > 4)
			{
				IgniteParamList tempParam;
				_sysMessage.Header = SYS_KEY_MSG;
				_sysMessage.Direction = DIRECTION_IN;
				Keyparamlist.clear();
				data = FrontPannelGetDataAt(MCU_DATA_SIZE_POS, false);
				if (data && DataToProcess > (uint32_t)(data + MCU_HEADRD_SIZE))// we recived enough data to process ?
				{
					if ((uint8_t)FrontPannelGetDataAt(4, false))
					{
						tempParam.param = (uint8_t)FrontPannelParam::paramKey_PLUS_NAV;
						tempParam.startPos = (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed;
						tempParam.Count = 1;
						tempParam.MsgValue = 0;
						Keyparamlist.push_back(tempParam);
						Keyparamlist.begin()->Count = multiKeyPressed;
						_sysMessage.Id = (paramId_t)FrontPannelParam::paramKey_KeyDataReady;
						_sysMessage.Data[0] = 1;
						FrontPanneExternalReadQ.put(_sysMessage, TimeOut::kNoWait);
						DBG("PLUS.............");

					}
					else
					{
						tempParam.param = (uint8_t)FrontPannelParam::paramKey_MINUS_NAV;
						tempParam.startPos = (uint8_t)FrontPannelKeyStatus::paramKeyStatusPressed;
						tempParam.Count = 1;
						tempParam.MsgValue = 0;
						Keyparamlist.push_back(tempParam);
						Keyparamlist.begin()->Count = multiKeyPressed;
						_sysMessage.Id = (paramId_t)FrontPannelParam::paramKey_KeyDataReady;
						_sysMessage.Data[0] = 0;
						FrontPanneExternalReadQ.put(_sysMessage, TimeOut::kNoWait);
						DBG("MINUS.............");
					}
					FrontPannelGetDataAt(1, true); //Handled move to next hearder
				}
			}
			break;
		default:
			uint8_t tmp = FrontPannelGetDataAt(errorSkipCount, false);
			std::cout << "ERROR: unknown ID received from MCU: " << " Error count: " << errorSkipCount << String(tmp) << std::endl;
			if ((tmp >= SYS_SYTM_MSG && tmp <= SYS_SYNC_MSG) || (errorSkipCount >= MCU_RX_BUFF_MAX_SIZE -1))
			{
				// Detected valid header - wild guess
				FrontPannelGetDataAt(errorSkipCount, true);
				errorSkipCount = 0;
			}
			else
				errorSkipCount++;
			break;
		}
	}
}
bool FrontPannelManager::sendAsMcuMsg(std::vector<uint8_t> data)
{
#if 0
	//{//add this part in handleIncomingMidiMessage(disable other calls) in songmanager to rute the midimsg recived from USB port to MCU port --- only for testing
	//	std::vector<uint8_t> data;
	//	data.push_back(0xE5);
	//	data.push_back(0x01);
	//	data.push_back(0x01);
	//	data.push_back(static_cast<uint8_t>(message.getRawDataSize()));
	//	const uint8_t* rawData = message.getRawData();
	//	data.insert(data.end(), rawData, rawData + message.getRawDataSize()); // Insert raw data into the vector
	//	_PSongMgr->engine.frontPannelManager->sendAsMcuMsg(data);
	//}

	for (auto it : data)
	{
		uint32_t writePos = (McuRxWrite + 1) % MCU_RX_BUFF_MAX_SIZE;
		if (writePos != McuRxRead)
		{
			UartMcuRxBuffer[McuRxWrite] = static_cast<uint8_t>(it);
			McuRxWrite = writePos;
		}
	}
	FrontPannelDecodeMessage();
#endif
	return true;
}

void FrontPannelManager::FrontPannelExternalReadThreadFunction(void)
{
#if defined(_WIN32) && !defined(FP_TEST_KEYBRD) && !defined(FP_REMOTE_CLIENT_RECIVE_WIN)
    return; // No physical MCU transport is present in the desktop application.
#endif
	FrontPannelExternalReadThread.delayMs(FRONT_PANEL_THREAD_INIT_WAIT); //Wait till Init Complete
	const int bufferSize = 256;
	char szBuff[bufferSize] = { 0 };
	while (!CThread::desktopShutdownRequested())
	{
		SysMessage _sysMessage;
		_sysMessage.Header = SYS_KEY_MSG;
		_sysMessage.Direction = DIRECTION_IN;
		int dwBytesRead = 0;
		bool isDataFromEth = false;
#if defined FP_UART_LINUX
		dwBytesRead = read(ttyMcuFileHandler, szBuff, bufferSize);
#endif
#if defined FP_REMOTE_CLIENT_RECIVE || defined FP_REMOTE_CLIENT_RECIVE_WIN
		if (dwBytesRead <= 0)
		{
			dwBytesRead = 0;
			int socket = clientSocket.load();
			if (socket != -1)
			{
				auto tmpRead = recv(socket, szBuff, bufferSize, 0);
				if (tmpRead > 0)
				{
					if (tmpRead > 0 && tmpRead < bufferSize)
					{
						dwBytesRead = tmpRead;
						isDataFromEth = true;
						//std::cout << "FP: got message " << dwBytesRead << std::endl;
					}
				}
				else if (tmpRead == 0)
				{
					// Connection closed
					std::cout << "FP: Connection closed by the peer" << std::endl;
					clientSocket.store(-1);
				}
				else if (tmpRead == -1)
				{
					// Error occurred
					switch (errno)
					{
					case EAGAIN:
						break;
					case EBADF:
					case ECONNRESET:
					case ECONNABORTED:
					case ENETUNREACH:
					case EHOSTUNREACH:
						std::cout << "FP:ETH ERROR: recv failed Socket reset" << std::endl;
						clientSocket.store(-1);
						break;
					default:
						std::cout << "FP:ETH ERROR: recv failed with unknown error " << errno << std::endl;
						break;
					}
				}
			}
		}
#endif
		if (dwBytesRead > 0)
		{
			for (auto i = 0; i < (int)dwBytesRead; i++)
			{
				// Debug code
				//char test[100];
				//sprintf(test, "  data recived 0x%02X ", szBuff[i]);
				//std::cout << "Data" << i << test << ":::" << szBuff[i] << std::endl;

				uint32_t writePos = (McuRxWrite.load() + 1) % MCU_RX_BUFF_MAX_SIZE;
				if (writePos != McuRxRead.load()) // Ensure buffer is not full
				{
					UartMcuRxBuffer[McuRxWrite.load()] = static_cast<uint8_t>(szBuff[i]);
					McuRxWrite.store(writePos);
				}
				else
				{
					std::cout << "FP:ERROR: overflow :: writePos == McuRxRead " << std::endl;
				}
			}
#if defined FP_SIM_LINUX_REMOTE_SEND // send a copy to windows simulator
			{
				int socket = clientSocket.load();
				if (socket != -1 && !isDataFromEth) {
					ssize_t bytesSent = send(socket, szBuff, dwBytesRead, 0);
					if (bytesSent == -1) {
						std::cout << "FP:ETH ERROR: Send failed Socket reset" << errno << std::endl;
						if(errno != EAGAIN)
							clientSocket.store(-1);
					}
					/*else
						std::cout << "FP:ETH INFO: Sent " << bytesSent << " bytes" << std::endl;
					*/
				}
			}
#endif
			isDataFromEth = false;
		}
#if defined FP_TEST_KEYBRD  // Test Code
		if (1)
		{
			SysMessage sysMessage = { 0, };
			if (!initDone)
			{
				FrontPannelExternalReadThread.delayMs(2000);
				this->postCommandMessage(1);
				FrontPannelExternalReadThread.delayMs(100);
				this->postCommandMessage(2);
				initDone = true;
				FrontPannelExternalReadThread.delayMs(1000);
			}
			while (initDone)
			{
				if (fpKeys == nullptr)
				{
					FrontPannelExternalReadThread.delayMs(500);
					continue;
				}
				FrontPannelExternalReadThread.delayMs(1000);
				sysMessage = fpKeys->getKey();

				if (sysMessage.Id == TEST_DELAY_ID) // internal to insert delay for 2 sec for test
				{
					FrontPannelExternalReadThread.delayMs(TEST_DELAY_TIME);
				}
				else if (sysMessage.Data[0] != 0)
				{
					FrontPanneExternalReadQ.put(sysMessage, TimeOut::kNoWait);
					FrontPannelExternalReadThread.delayMs(20);
				}
				else if (_toggleButtonArrayVal_needupdate)
				{
					this->postCommandMessage(3);
				}
			}//while loop
		}
#endif
	}
}
inline int FrontPannelManager::uart_write_with_retry(int ttyMcuFileHandler, const void* data, size_t size)
{
#if defined FP_UART_LINUX
	int retries = 0;
	ssize_t bytes_written;

	while (retries < MAX_RETRIES) {
		bytes_written = write(ttyMcuFileHandler, data, size);
		if (bytes_written == size) {
			// Write successful
			return 0;
		}
		else if (bytes_written == -1 && errno == EAGAIN) {
			// Write would block, retry
			retries++;
			perror("UART write failed, retrying");
		}
		else {
			// Other errors
			perror("UART write failed");
			return -1;
		}
	}

	// If we reach here, all retries have failed
#endif
	return -1;
}
void FrontPannelManager::FrontPannelExternalWriteThreadFunction(void)
{
	FrontPannelExternalWriteThread.delayMs(FRONT_PANEL_THREAD_INIT_WAIT); //Wait till Init Complete
	SysMessage fpQData = { 0, };

	while (!CThread::desktopShutdownRequested())
	{
		fpQData = { 0, };
		if (!FrontPanneExternalWriteQ.get(fpQData, TimeOut::kWaitForever)) break;
		std::unique_lock<std::mutex> lock(midiSendMutx); //SYS_MIDI_MSG - is priority not using FrontPanneExternalWriteQ
		while (!midiSendDone) { // Manually check the condition
			midiSendWait.wait(lock);
		}
		if (fpQData.Direction == DIRECTION_OUT)
		{
			switch (fpQData.Header)
			{
			case SYS_MIDI_MSG:
				// MIDI send out not using Q, see sendMidiMessageExternal
				break;
			case SYS_PAD_MSG:
			case SYS_LED_MSG:
			case SYS_KNOB_MSG:
			case SYS_SYTM_MSG:
#if defined FP_UART_LINUX
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Header, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Direction, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Id, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.size, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Data, ((fpQData.size) - MCU_HEADRD_SIZE));
#endif
				if (0)//if needed more data
				{
					DBG(" LED MSG to MCU Header  " + String(fpQData.Header));
					DBG(" LED MSG to MCU Direction  " + String(fpQData.Direction));
					DBG(" LED MSG to MCU Id  " + String(fpQData.Id));
					DBG(" LED MSG to MCU size  " + String(fpQData.size));
					for (auto i = 0; i < ((fpQData.size) - MCU_HEADRD_SIZE); i++)
					{
						DBG(" LED MSG to MCU Data index [" + String(i) + "] " + String(fpQData.Data[i]));
					}
				}
				else
					//DBG(" LED MSG to MCU Header  " + String(fpQData.Header));
					break;

			case SYS_SYNC_MSG:
#if defined FP_UART_LINUX
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Header, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Direction, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Id, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.size, 1);
				uart_write_with_retry(ttyMcuFileHandler, &fpQData.Data, ((fpQData.size) - MCU_HEADRD_SIZE));
#endif
				break;
			}

		}
	}
}

void FrontPannelManager::FrontPannelInternalProcessThreadFunction(void)
{
	FrontPannelInternalProcessThread.delayMs(FRONT_PANEL_THREAD_INIT_WAIT); //Wait till Init Complete
	SysMessage fpQData = { 0, };
	while (!CThread::desktopShutdownRequested())
	{
		fpQData = { 0, };
		if (!FrontPanneExternalReadQ.get(fpQData, TimeOut::kWaitForever)) break;
		if (fpQData.Direction == DIRECTION_IN)
		{
			FpUtility->SysMessageHandlerFromMCU(fpQData);
		}
	}
}
void FrontPannelManager::FrontPannelLinuxWatchThreadFunction(void)
{
#if defined FP_UART_LINUX
	FrontPannelInternalProcessThread.delayMs(FRONT_PANEL_THREAD_INIT_WAIT); //Wait till Init Complete
	int PowerKeyFd;
	struct input_event PowerKeyEvent;
	PowerKeyFd = open("/dev/input/event2", O_RDONLY);
	if (PowerKeyFd == -1) {
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN setup event failed");
		//TODO: implement recovery?
	}
	while (!CThread::desktopShutdownRequested())
	{
		FrontPannelInternalProcessThread.delayMs(FRONT_PANEL_THREAD_WAIT_1_SEC); //Wait 1 sec
		read(PowerKeyFd, &PowerKeyEvent, sizeof(struct input_event));
		//printf("event1 Read value Event type: %d, code: %d, value: %d\n", PowerKeyEvent.type, PowerKeyEvent.code, PowerKeyEvent.value); //TODO After test change to DBG
		if (PowerKeyEvent.value == 1)
		{
			auto now = std::chrono::steady_clock::now();
			if (isScheduled)
			{
				lastKeyPress = now;
				return;
			}

			isScheduled = true;
			lastKeyPress = now;

			std::thread([this]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(debounceDurationMs));

				auto now = std::chrono::steady_clock::now();
				if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastKeyPress).count() >= debounceDurationMs)
				{
					engine.SendResponse((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_PowerKey_Event, 1);
				}

				isScheduled = false;
			}).detach();
		}
	}
#endif //FP_UART_LINUX
}
bool FrontPannelManager::isVirtualPort(uint8_t midiport)
{
	if (midiport > 5) //TODO - define port ID
	{
		return true;
	}
	return false;
}
bool FrontPannelManager::isPortAvailable(uint8_t midiport)
{
	//Linux API check the Virtual port present
	return false;
}
void FrontPannelManager::sendKnobLedData()
{
	SysMessage _xternalData;
	_xternalData.Header = SYS_KNOB_MSG;
	_xternalData.Direction = DIRECTION_OUT;
	_xternalData.Id = 0;
	_xternalData.size = (CV_KNOB_COUNT + MCU_HEADRD_SIZE);

	for (auto i = 0; i < CV_KNOB_COUNT; i++)
	{
		_xternalData.Data[i] = CV_Led_Data[i];
	}
	KnobLedDataUpdated = false;
	FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
}
void FrontPannelManager::sendRGBLedData()
{
	SysMessage _xternalData;
	_xternalData.Header = SYS_PAD_MSG;
	_xternalData.Direction = DIRECTION_OUT;
	_xternalData.Id = 0;
	_xternalData.size = (RGB_LED_COUNT + MCU_HEADRD_SIZE);

	for (auto i = 0; i < RGB_LED_COUNT; i++)
	{
		_xternalData.Data[i] = RGB_Led_Data[i];
	}
	RgbLedDataUpdated = false;
	FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
}
void FrontPannelManager::updateLedData(uint8_t leddata[], uint16_t arraySize, uint8_t dispID)
{
	SysMessage _xternalData;
	_xternalData.Header = SYS_LED_MSG;
	_xternalData.Direction = DIRECTION_OUT;
	_xternalData.Id = dispID;
	_xternalData.size = static_cast<uint8_t>(arraySize + MCU_HEADRD_SIZE);
	for (auto i = 0; i < arraySize; i++)
	{
		_xternalData.Data[i] = leddata[i];
	}
	FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
}

void FrontPannelManager::sendMidiMessageExternal(MidiMessage msg, uint8_t midiport)
{
	midiSendDone = false;
	std::lock_guard<std::mutex> lock(midiSendMutx);//lock the FrontPanneExternalWriteQ - not to send any data to MCU
#if defined FP_UART_LINUX
	uint8_t header = SYS_MIDI_MSG;
	uint8_t direction = DIRECTION_OUT;
	uint8_t id = midiport;
	uint8_t size = static_cast<uint8_t>(msg.getRawDataSize() + MCU_HEADRD_SIZE);
	const uint8_t* data = msg.getRawData();

	uart_write_with_retry(ttyMcuFileHandler, &header, 1);
	uart_write_with_retry(ttyMcuFileHandler, &direction, 1);
	uart_write_with_retry(ttyMcuFileHandler, &id, 1);
	uart_write_with_retry(ttyMcuFileHandler, &size, 1);
	uart_write_with_retry(ttyMcuFileHandler, data, msg.getRawDataSize());
#endif
	midiSendDone = true;
	midiSendWait.notify_one(); //unlock
}

void FrontPannelManager::sendLedMessageToMCU(uint8_t LediD, uint8_t LedNum, uint8_t LedColor)
{
	if ((RGB_Led_Data[LedNum] != LedColor) && (LedNum < RGB_LED_COUNT))
	{
		RGB_Led_Data[LedNum] = LedColor;
		RgbLedDataUpdated = true;
	}
}
void FrontPannelManager::setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		//FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
		setParam(moduleSrc, (*it).param, (*it).MsgValue, isPlayMode);
	}
}
std::vector<IgniteParamList> FrontPannelManager::getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam;

	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		switch ((FrontPannelParam)(*it).param)
		{
		case FrontPannelParam::paramKey_KeyDataReady:
		{
			std::unique_lock<std::mutex> lock(keyReadmutex);
			while (!KeyReady)//wait if Key decode in progress
			{
				KeyReady_cv.wait(lock);
			}
			retParam = Keyparamlist;
			//KeyReady = false; -- NOTE *** In some use case UI may ask again, return current data
			return(retParam); // Keyparamlist has fey data; UI not to mix with other req: one param req will provide all key status
		}
		break;
		case FrontPannelParam::paramKey_Valid_Firmware_check:
		{
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = (int8_t)isValidFirmware();
			retParam.push_back(tempParam);
			break;
		}
		case FrontPannelParam::paramKey_EmmcGetTotalSize:
		{
			getIntStorageStatus();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = (int)intTotalSize;
			retParam.push_back(tempParam);
			break;
		}
		case FrontPannelParam::paramKey_EmmcGetFreeSize:
		{
			getIntStorageStatus();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = (int)intFreeSize;
			retParam.push_back(tempParam);
			break;
		}
		case FrontPannelParam::paramKey_UsbGetTotalSize:
		{
			getUsbStorageStatus();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = (int)usbTotalSize;
			retParam.push_back(tempParam);
			break;
		}
		case FrontPannelParam::paramKey_UsbGetFreeSize:
		{
			getUsbStorageStatus();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = (int)usbFreeSize;
			retParam.push_back(tempParam);
			break;
		}
		case FrontPannelParam::paramKey_RamGetTotalSize:
		{
			getMemoryStatus();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = (int)ramTotalSize;
			retParam.push_back(tempParam);
			break;
		}
		case FrontPannelParam::paramKey_RamGetFreeSize:
		{
			getMemoryStatus();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = (int)ramFreeSize;
			retParam.push_back(tempParam);
			break;
		}

		case FrontPannelParam::paramKey_getFirmwareVersion:
		{
			getFirmwareInfo();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = imxFirmwareVersion;
			retParam.push_back(tempParam);
			break;
		}

		case FrontPannelParam::paramKey_getFirmwareBuildInfo:
		{
			getFirmwareInfo();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = firmwareBuildDate;
			retParam.push_back(tempParam);
			break;
		}

		case FrontPannelParam::paramKey_getSerialNumber:
		{
			getSerialNumber();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = serialNumber;
			retParam.push_back(tempParam);
			break;
		}

		case FrontPannelParam::paramKey_UsbGetConnectionStatus:
		{
			getSerialNumber();
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = isUSBStorageConnected();
			retParam.push_back(tempParam);
			break;
		}

		case FrontPannelParam::paramKey_DataCorrupted:
		{
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = engine.fileHandler->isGetDataCorrupted();
			retParam.push_back(tempParam);
			break;
		}
		case FrontPannelParam::paramKey_DevStatus:
		{
			struct IgniteParamList tempParam;
			tempParam.param = (*it).param;
			tempParam.MsgValue = fwUpdateStatus;
			retParam.push_back(tempParam);
			break;
		}
		default:
			//assert(0);
			break;

		}
	}
	return(retParam);
}
void FrontPannelManager::SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{

}
void FrontPannelManager::getParam(moduleId_t moduleSrc, paramId_t param)
{
}
void FrontPannelManager::onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	ParamHandler(moduleSrc, param, false);//Not sending any Notification about LEDs
}
void FrontPannelManager::ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	paramId_t paramId = param->GetParamID();
	if (paramId < simulatorLeds.size()) simulatorLeds[paramId] = param->getValue().get<uint8_t>();
	uint8_t inPlace = 0xFF;
#ifdef FP_TEST_KEYBRD
	if (initDone && _toggleButtonArray[0] != nullptr)
	{
		for (int i = 0; i < 61; i++)
		{
			if (FpLedHandler->KetToLedPositionMapping[i] == paramId)
			{
				uint8_t val = (param->getValue().get<uint8_t>());
				_toggleButtonArrayVal_needupdate = true;
				//not handling here as it required to modify FP code, will be handled in handlemessage (command 3)
				switch ((FrontPannelParam)paramId)
				{
				case FrontPannelParam::paramKey_MENU:
				case FrontPannelParam::paramKey_TRACK:
				case FrontPannelParam::paramKey_SCENE:
				case FrontPannelParam::paramKey_SONG:
				case FrontPannelParam::paramKey_PATTERN:

					_toggleButtonArrayVal[1] = 0;//(uint8_t)FrontPannelParam::paramKey_MENU
					_toggleButtonArrayVal[21] = 0;//(uint8_t)FrontPannelParam::paramKey_TRACK
					_toggleButtonArrayVal[20] = 0;//(uint8_t)FrontPannelParam::paramKey_SCENE
					_toggleButtonArrayVal[19] = 0;//(uint8_t)FrontPannelParam::paramKey_SONG
					_toggleButtonArrayVal[26] = 0;//(uint8_t)FrontPannelParam::paramKey_PATTERN
					break;
				}

				_toggleButtonArrayVal[i] = ((param->getValue().get<uint8_t>() == fpLEDPattern::EN_LED_PATT_OFF) ? 0 : 1);
			}
		}
	}
	return;
#endif

	if ((FrontPannelParam)paramId < FrontPannelParam::paramKey_Internals) //Handle LEDs
	{
		//sendLedMessageToMCU (McuLedMapping[paramId], (param->getValue ().get<uint8_t> ()), fpLEDPattern::EN_RGB_LED_OFF);//TODO -- convert to LED Data
		//FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, (param->getValue().get<uint8_t>()), fpLEDPattern::EN_RGB_LED_OFF);
	}

	if (isHwTestOn && (FrontPannelParam)paramId != FrontPannelParam::paramKey_HwTestMode)
	{
		return;
	}

	switch ((FrontPannelParam)paramId)
	{
	case FrontPannelParam::paramKey_BRAY:
	case FrontPannelParam::paramKey_ENTER_NAV:
	case FrontPannelParam::paramKey_MINUS_NAV:
	case FrontPannelParam::paramKey_PLUS_NAV:
	case FrontPannelParam::paramKey_RETURN_NAV:
	case FrontPannelParam::paramKey_COPY_NAV:
	case FrontPannelParam::paramKey_USB_STOP:
	case FrontPannelParam::paramKey_LAST:
	case FrontPannelParam::paramKey_SHIFT:
	case FrontPannelParam::paramKey_INSERT:
	case FrontPannelParam::paramKey_DELETE:
	case FrontPannelParam::paramKey_RMOVELEFT:
	case FrontPannelParam::paramKey_RMOVERIGHT:
	case FrontPannelParam::paramKey_RSELECT:
		// Invalid or blink
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, param->getValue().get<uint8_t>(), fpLEDPattern::EN_RGB_LED_OFF);
		break;
	case FrontPannelParam::paramKey_INTERNALSOUNDS:
	{
		engine.songManager->handleMcuSpecialKey(paramId, (uint8_t)(param->getValue().get<uint8_t>() == fpLEDPattern::EN_LED_PATT_ON)); // set On/Off to Songmanager
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, param->getValue().get<uint8_t>(), fpLEDPattern::EN_RGB_LED_OFF);
	}
	break;
	case FrontPannelParam::paramKey_MENU:
	case FrontPannelParam::paramKey_TRACK:
	case FrontPannelParam::paramKey_SCENE:
	case FrontPannelParam::paramKey_SONG:
	case FrontPannelParam::paramKey_PATTERN:
		//On or blink till Stop requested - one of it can be on MCU will off the other
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, param->getValue().get<uint8_t>(), fpLEDPattern::EN_RGB_LED_OFF);
		break;

	case FrontPannelParam::paramKey_PLAY:
	case FrontPannelParam::paramKey_STOP:
		//On or blink till Stop requested - one of it can be on MCU will off the other
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, (param->getValue().get<uint8_t>()), fpLEDPattern::EN_RGB_LED_OFF);
		break;

	case FrontPannelParam::paramKey_SAVE:
	case FrontPannelParam::paramKey_TEMPO:
	case FrontPannelParam::paramKey_FILL_X:
	case FrontPannelParam::paramKey_NEXT_TIE:
	case FrontPannelParam::paramKey_SOLO_SKIP:
		//blink Stop
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, (param->getValue().get<uint8_t>()), fpLEDPattern::EN_RGB_LED_OFF);
		break;

	case FrontPannelParam::paramKey_ROTARYROW:
	case FrontPannelParam::paramKey_BACKFORWARD:
	case FrontPannelParam::paramKey_ROTARYBAR:
		//Blink
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, (param->getValue().get<uint8_t>()), fpLEDPattern::EN_RGB_LED_OFF);
		break;

	case FrontPannelParam::paramKey_SCULPT:
	case FrontPannelParam::paramKey_REC:
	case FrontPannelParam::paramKey_GANG:
	case FrontPannelParam::paramKey_TRACKROTARY1:
	case FrontPannelParam::paramKey_TRACKROTARY2:
	case FrontPannelParam::paramKey_TRACKROTARY3:
	case FrontPannelParam::paramKey_TRACKROTARY4:
	case FrontPannelParam::paramKey_TRACKROTARY5:
	case FrontPannelParam::paramKey_TRACKROTARY6:
	case FrontPannelParam::paramKey_TRACKROTARY7:
	case FrontPannelParam::paramKey_TRACKROTARY8:
	case FrontPannelParam::paramKey_TRACKROTARY9:
	case FrontPannelParam::paramKey_TRACKROTARY10:
	case FrontPannelParam::paramKey_TRACKROTARY11:
	case FrontPannelParam::paramKey_TRACKROTARY12:
	case FrontPannelParam::paramKey_TRACKROTARY13:
	case FrontPannelParam::paramKey_TRACKROTARY14:
	case FrontPannelParam::paramKey_TRACKROTARY15:
	case FrontPannelParam::paramKey_TRACKROTARY16:
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, (param->getValue().get<uint8_t>()), fpLEDPattern::EN_RGB_LED_OFF);
		break;

	case FrontPannelParam::paramKey_RELEASE:
	case FrontPannelParam::paramKey_FILTER:
	case FrontPannelParam::paramKey_RESONANCE:
	case FrontPannelParam::paramKey_VOLUME:
	case FrontPannelParam::paramKey_REVERBE:
	case FrontPannelParam::paramKey_CHORUS:
	case FrontPannelParam::paramKey_EQHI:
	case FrontPannelParam::paramKey_EQLO:
	{
		uint16_t value = 0xFF;
		inPlace = ((uint8_t)paramId - (uint8_t)FrontPannelParam::paramKey_RELEASE);
		value = param->getValue().get<uint16_t>();
		 
		if (param->getValue().get<uint16_t>() && value == 0)
			value = 1;
		else if (value > 16)
			value = 16;

		DBG("-------------- KNOB Value ------ " + String(value) + "::" + String(inPlace));


		if (CV_Led_Data[inPlace] != value)
		{
			KnobLedDataUpdated = true;
			CV_Led_Data[inPlace] = static_cast<uint8_t>(value);
		}
	}
	break;
	case FrontPannelParam::paramRot_PADROT1:
	case FrontPannelParam::paramRot_PADROT2:
	case FrontPannelParam::paramRot_PADROT3:
	case FrontPannelParam::paramRot_PADROT4:
	case FrontPannelParam::paramRot_PADROT5:
	case FrontPannelParam::paramRot_PADROT6:
	case FrontPannelParam::paramRot_PADROT7:
	case FrontPannelParam::paramRot_PADROT8:
	case FrontPannelParam::paramRot_PADROT9:
	case FrontPannelParam::paramRot_PADROT10:
	case FrontPannelParam::paramRot_PADROT11:
	case FrontPannelParam::paramRot_PADROT12:
	case FrontPannelParam::paramRot_PADROT13:
	case FrontPannelParam::paramRot_PADROT14:
	case FrontPannelParam::paramRot_PADROT15:
	case FrontPannelParam::paramRot_PADROT16:
		// Not used -- use paramKey_TRACKROTARY to set LED
		break;

	case FrontPannelParam::paramKey_TRACKSEL1:
	case FrontPannelParam::paramKey_TRACKSEL2:
	case FrontPannelParam::paramKey_TRACKSEL3:
	case FrontPannelParam::paramKey_TRACKSEL4:
	case FrontPannelParam::paramKey_TRACKSEL5:
	case FrontPannelParam::paramKey_TRACKSEL6:
	case FrontPannelParam::paramKey_TRACKSEL7:
	case FrontPannelParam::paramKey_TRACKSEL8:
	case FrontPannelParam::paramKey_TRACKSEL9:
	case FrontPannelParam::paramKey_TRACKSEL10:
	case FrontPannelParam::paramKey_TRACKSEL11:
	case FrontPannelParam::paramKey_TRACKSEL12:
	case FrontPannelParam::paramKey_TRACKSEL13:
	case FrontPannelParam::paramKey_TRACKSEL14:
	case FrontPannelParam::paramKey_TRACKSEL15:
	case FrontPannelParam::paramKey_TRACKSEL16:
		FpLedHandler->HandleLedMessageFromUI((uint8_t)paramId, (param->getValue().get<uint8_t>()), fpLEDPattern::EN_RGB_LED_OFF);
		break;

	case FrontPannelParam::param_RGB_DISP1:
	case FrontPannelParam::param_RGB_DISP2:
	case FrontPannelParam::param_RGB_DISP3:
	case FrontPannelParam::param_RGB_DISP4:
	case FrontPannelParam::param_RGB_DISP5:
	case FrontPannelParam::param_RGB_DISP6:
	case FrontPannelParam::param_RGB_DISP7:
	case FrontPannelParam::param_RGB_DISP8:
	case FrontPannelParam::param_RGB_DISP9:
	case FrontPannelParam::param_RGB_DISP10:
	case FrontPannelParam::param_RGB_DISP11:
	case FrontPannelParam::param_RGB_DISP12:
	case FrontPannelParam::param_RGB_DISP13:
	case FrontPannelParam::param_RGB_DISP14:
	case FrontPannelParam::param_RGB_DISP15:
	case FrontPannelParam::param_RGB_DISP16:
		inPlace = ((uint8_t)paramId - (uint8_t)FrontPannelParam::param_RGB_DISP1);
		sendLedMessageToMCU(inPlace, inPlace, param->getValue().get<uint8_t>());
		break;

	case FrontPannelParam::paramKey_LCD_Brightness:
		setLCDDisplayBrightness(param->getValue().get<uint8_t>());
		break;
	case FrontPannelParam::paramKey_FPLED_Brightness:
		setMcuLEDBrightness(param->getValue().get<uint8_t>());
		break;
	case FrontPannelParam::paramKey_USBStop:
		stopUsb(param->getValue().get<uint8_t>());
		break;
	case FrontPannelParam::paramKey_decryptFirmwareImage:
		decryptFirmwareImage();
		break;
	case FrontPannelParam::paramKey_Firmware_Update:
#if defined PLAYTRIBE 
		usbFunctionFS->setFirmwareUpdateStatus(DEV_OP_ONGOING);
#endif
		playTribeConMngr->setFirmwareUpdateStatus(DEV_OP_ONGOING);
		fwUpdateStatus = FW_UPDATE_ONGOING;
		startFirmwareUpdate();
		break;
	case FrontPannelParam::paramKey_Firmware_Update_Source:
		fw_source = (SwUdateSource_t)param->getValue().get<uint8_t>();
		isFirmwareImagePresent();
		break;
	case FrontPannelParam::paramKey_ExtractFirmwareImage:
		extractImage();
		break;
	case FrontPannelParam::paramKey_DevStatus:
		fwUpdateStatus = (DEV_STATUS)param->getValue().get<int>();
#if defined PLAYTRIBE 
		usbFunctionFS->setFirmwareUpdateStatus(DEV_OP_ONGOING);
#endif
		playTribeConMngr->setFirmwareUpdateStatus(DEV_OP_ONGOING);
		break;
	case FrontPannelParam::param_removeFirmwarePacakge:
		removeFirmwareFile();
		break;
	case FrontPannelParam::paramKey_FactoryReset:
		startFactoryReset();
		break;
	case FrontPannelParam::paramKey_RebootDevice:
		rebootDevice();
		break;
	case FrontPannelParam::paramKey_HwTestMode:
		HandleSystemMsg(SYS_SYTM_HW_TEST, (uint8_t)param->getValue().get<uint8_t>());
		isHwTestOn = ((uint8_t)param->getValue().get<uint8_t>() > 0);
		if (isHwTestOn)
			startTimer(1000);
		else
			stopTimer();
		break;
	case FrontPannelParam::paramKey_FwUpgrade:
		HandleFwUpgrade(SYS_SW_UPGRADE_REQ);
		break;
	case FrontPannelParam::paramKey_DataCorrupted:
		engine.SendResponse((moduleId_t)ModuleName::moduleSong, moduleSrc, (paramId_t)FrontPannelParam::paramKey_DataCorrupted, engine.fileHandler->isGetDataCorrupted());
	case FrontPannelParam::paramKey_McuSleepMode:
		HandleSystemMsg(SYS_MCU_SLEEP, (uint8_t)param->getValue().get<uint8_t>());//TODO test and enable?
		engine.songManager->saveSongData();//as of now reboot handled in UI
		break;
	case FrontPannelParam::param_SyncOut:
		SendSyncOutFreq((uint8_t)param->getValue().get<uint16_t>());
		break;
	case FrontPannelParam::param_SyncIn:
		RequestSyncIn();
		break;
	case FrontPannelParam::param_startPlayTribeUsbHandler:
//#if defined PLAYTRIBE 
		usbFunctionFS->stop();
		usbFunctionFS->start();
//#endif
		break;
	case FrontPannelParam::param_SOMFwUpdate:
		HandleFwUpgrade(SYS_SW_UPDATE_SOM);
		break;
	case FrontPannelParam::param_MCU_FWVERSION:
		HandleSystemMsg(SYS_MCU_FW_VERSION, (int8_t)param->getValue().get<int8_t>());
		break;
	case FrontPannelParam::param_startPlayTribeWifiHandler:
//#if defined PLAYTRIBE
		playTribeConMngr->PlayTribeConnectionManagerStartThreads();
//#endif
		break;
	case FrontPannelParam::param_stopPlayTribeWifiHandler:
//#if defined PLAYTRIBE 
		playTribeConMngr->disconnectClient();
//#endif
		break;
	}
}
SongManager* FrontPannelManager::getSongManager()
{
	return engine.songManager;
}
void FrontPannelManager::timerCallback()
{
	if (!HBState)
	{
		HBMissingCount = 0;
		HBState = true;
	}
	else if (HBState)
	{
		HBMissingCount++;
		HBState = true;
	}

	if (McuSoftReset)
	{
#if 0
		std::system(MCU_MCUTXPIN_EXPORT);
		std::system(MCU_MCUTXPIN_DIR_SET);
		std::system(MCU_MCUTXPIN_VAL_HIGH);
		for (int i = 0; i < 10000; i++)
			for (int j = 0; j < 500; j++);
		std::system(MCU_MCUTXPIN_VAL_LOW);
		std::system(MCU_MCUTXPIN_VAL_HIGH);
		std::system(MCU_MCUTXPIN_UNEXPORT);
		startTimer(3000);
#endif
		HandleSystemMsg(SYS_MCU_SYNC_STATE, 0x00);//MCU SYNC MSG SEND
		McuSoftReset = 0;
		HBMissingCount = 0;
	}
	if (HBMissingCount > 5)
	{
		//commenting below body for mcu restart
//#ifdef FP_UART_LINUX //commenting blo
		//std::system(MCU_RESETPIN_EXPORT);
		//std::system(MCU_RESETPIN_DIR_SET);
		//std::system(MCU_RESETPIN_VAL_HIGH);
		//std::system(MCU_RESETPIN_VAL_LOW);
		////std::system(MCU_RESETPIN_VAL_HIGH);
		//std::system(MCU_RESETPIN_UNEXPORT);
		//DBG("mcu restarted");
//#endif
		McuSoftReset = 1;
		HBMissingCount = 0;
	}
	//restart MCU

#if HARDWARETEST
	MidiMessage msg = juce::MidiMessage::noteOn(1, 10, (juce::uint8)100);
	sendMidiMessageExternal(msg, testPortNum);
	testPortNum++;
	if (testPortNum > 4)
		testPortNum = 0;
	if (isHwTestOn)
		startTimer(400);
#endif
}
void FrontPannelManager::HandleSystemMsg(uint8_t id, uint8_t value)
{
	SysMessage _xternalData;
	_xternalData.Header = SYS_SYTM_MSG;
	_xternalData.Direction = DIRECTION_OUT;
	_xternalData.Id = id;
	_xternalData.size = (MCU_HEADRD_SIZE + 1);
	_xternalData.Data[0] = value;
	FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
}
void FrontPannelManager::setMcuLEDBrightness(uint8_t value)
{
	DBG(value);
	if (value <= 0)
	{
		HandleSystemMsg(SYS_MCU_LED_BRIGHTNESS, LED_BRIGHTNESS_0);
		
	}
	else if(value <= 25)
	{
		HandleSystemMsg(SYS_MCU_LED_BRIGHTNESS, LED_BRIGHTNESS_25);

	}
	else if (value <= 50)
	{
		HandleSystemMsg(SYS_MCU_LED_BRIGHTNESS, LED_BRIGHTNESS_50);

	}
	else if (value <= 75)
	{
		HandleSystemMsg(SYS_MCU_LED_BRIGHTNESS, LED_BRIGHTNESS_75);

	}
	else if (value <= 100)
	{
		HandleSystemMsg(SYS_MCU_LED_BRIGHTNESS, LED_BRIGHTNESS_100);
	}
	//todo Send message to MCU
}
void FrontPannelManager::setLCDDisplayBrightness(uint8_t value)
{
#if defined FP_UART_LINUX
	if (value >= 0 && value <= 100)
	{
		char cmd[100];
		if (value < 25)
			value = 10;
		sprintf(cmd, "echo %d > /sys/class/backlight/backlight/brightness", value);
		std::system(cmd);
	}
#endif //FP_UART_LINUX
	DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Set LCD Brightness Value = " + LCD_BRIGHTNESS_SYS_PATH + "  " + String(value));
}
void FrontPannelManager::stopUsb(uint8_t value)
{
#if defined FP_UART_LINUX
	std::system(USB_UNMOUNT_CMD);
#endif //FP_UART_LINUX
	DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "USB Unmount command: " + USB_UNMOUNT_CMD);
}

bool FrontPannelManager::getVersionfromFile(std::string filename)
{
	char versionBytes[20];
	fw_ver_index = -1;

	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error: Unable to open image file." << std::endl;
		return false;
	}

	file.read(versionBytes, 20);
	newImxFirmwareVersion = std::string(versionBytes, 20);
	file.close();

	for (auto i = 0; i < 20; i++)
		if ((versionBytes[i] == 'E') || (versionBytes[i] == 'R'))
			fw_ver_index = i;

	if (fw_ver_index < 0)
	{
		std::cerr << "Version is not correct" << std::endl;
		return "";
	}
	versionBytes[fw_ver_index] = '\0';
	fw_ver_index += 2;
	newImxFirmwareVersion = versionBytes;
	std::regex pattern("\\d+\\.\\d+\\.\\d+");
	if (!std::regex_match(newImxFirmwareVersion, pattern))
		return false;

	std::cerr << "New FW Version: " << newImxFirmwareVersion << std::endl;
	return true;
}

void FrontPannelManager::removeHeaderFromImage(std::string fwfilepath)
{
#if defined FP_UART_LINUX
	std::string cmd = "mkdir -p " + decrptionDir;
	system(cmd.c_str());

	cmd = "tail -c +" + std::to_string(fw_ver_index) + " " + fwfilepath + " > " + decrptionDir + fwHeaderFile;
	DBG("removing Header from image" << cmd);
	system(cmd.c_str());
#endif
}


int8_t FrontPannelManager::decryptFirmwareImage()
{
	bool status = false;
	std::string outFullpath = decrptionDir + fwFileName;
	std::string cmd = "mkdir -p " + decrptionDir;
	fwFileFullpath = decrptionDir + fwHeaderFile;

#if defined FP_UART_LINUX
	system(cmd.c_str()); //creating temp Dir
	if (fs::is_directory(decrptionDir))
	{
		std::string command = "openssl enc -aes-256-cbc -pbkdf2 -d -salt -in " + fwFileFullpath + " -out " + outFullpath + " -k '" + decrptKey + "'";

		// Execute the command using system call
		int result = system(command.c_str());

		// Check the result of the system call
		if (result == 0)
		{
			DBG("FWUpdate Decryption successful");
			return true;
		}
		else
		{
			printf("FWUpdate -> Incorrect password or decryption failed\n");
			return false;
		}
	}
	else
	{
		DBG("FwUpdate -> decrptionDir= " << decrptionDir << " NOT found");
	}
#endif
	return status;
}

std::vector<std::string> FrontPannelManager::getSWUFiles(const std::string& folderPath, const std::string& imageFormat)
{
	std::vector<std::string> swuFiles;

	if (fs::exists(folderPath) && fs::is_directory(folderPath))
	{
		try
		{
			for (const auto& entry : fs::directory_iterator(folderPath))
			{
				if (entry.path().extension() == imageFormat)
				{
					swuFiles.push_back(entry.path().filename().string());
					DBG("FwUpate files -> " << entry.path().filename().string());
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			DBG("Error accessing directory: " << e.what());
			(void)e;
		}
	}
	else
	{
		DBG("The path is not valid or does not exist.");
	}
	return swuFiles;
}

bool FrontPannelManager::extractImage()
{
	bool status = true;
	std::string imageFormat = ".swu";
	std::string cmd = "mkdir -p " + extractDir;

	system(cmd.c_str());

	std::string outFullpath = decrptionDir + fwFileName;
	cmd = "mount -o loop -t ext4  " + outFullpath + " " + extractDir;
	DBG("decrptionDir: " + String(decrptionDir) + " fwFileName " + String(fwFileName) + " outFullpath " + String(outFullpath));

	system(cmd.c_str());

	std::vector<std::string> swuFiles = getSWUFiles(extractDir, imageFormat);

	if (swuFiles.empty())
	{
		DBG("FwUpdate -> After exraction image file In " << extractDir << " Is not Found");
		return false;
	}
	else
	{
		DBG("FwUpdate -> After exraction image file In " << extractDir << " Is Found");
		for (const auto& swuFile : swuFiles)
		{
			DBG("swu file = " + String(swuFile));
			fwFileName = (swuFile);
		}
	}
	return status;
}

void FrontPannelManager::SendSyncOutFreq(uint16_t freq)
{
	SysMessage _xternalData;
	_xternalData.Header = SYS_SYNC_MSG;
	_xternalData.Direction = DIRECTION_OUT;
	_xternalData.Id = SYNC_OUT;
	_xternalData.Data[0] = (uint8_t)SyncOutMap[freq + 1];
	if (SyncOutMap[freq + 1] < 255)
	{
		_xternalData.size = (MCU_HEADRD_SIZE + 1);
	}
	else
	{
		_xternalData.size = (MCU_HEADRD_SIZE + 2);
		_xternalData.Data[1] = (uint8_t)(SyncOutMap[freq + 1] >> 8);
	}
	
	FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
}

void FrontPannelManager::RequestSyncIn()
{
	SysMessage _xternalData;
	_xternalData.Header = SYS_SYNC_MSG;
	_xternalData.Direction = DIRECTION_OUT;
	_xternalData.Id = SYNC_IN;
	_xternalData.size = (MCU_HEADRD_SIZE + 1);
	_xternalData.Data[0] = 0x01;
	FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
}

int FrontPannelManager::compareSWVersion(String newVersion, String currentVersion)
{
	int verNum1 = 0, verNum2 = 0;

	for (int i = 0, j = 0; (i < newVersion.length()
		|| j < currentVersion.length());) {
		while (i < newVersion.length() && newVersion[i] != '.') {
			verNum1 = verNum1 * 10 + (newVersion[i] - '0');
			i++;
		}

		while (j < currentVersion.length() && currentVersion[j] != '.') {
			verNum2 = verNum2 * 10 + (currentVersion[j] - '0');
			j++;
		}

		if (verNum1 > verNum2)
			return 1;
		if (verNum2 > verNum1)
			return -1;

		verNum1 = verNum2 = 0;
		i++;
		j++;
	}
	return 0;
}

bool FrontPannelManager::getFWUpdatePkgFirmwareVersion()
{
	std::fstream versionFile;
	versionFile.open("/tmp/fwupdate/version.txt", std::ios::in);
	if (versionFile.is_open())
	{
		std::string str;
		while (getline(versionFile, str))
		{
			if (!str.find("DATE"))
			{
				str.erase(0, 5);
				newFirmwareBuildDate = str;
			}
			else if (!str.find("IMX"))
			{
				str.erase(0, 8);
				newImxFirmwareVersion = str;
			}
			else if (!str.find("CTL"))
			{
				str.erase(0, 8);
				newGdFirmwareVersion = str;
			}
		}
		versionFile.close();
		return true;
	}
	else
		DBG("File not found");
	return false;
}

void FrontPannelManager::startFirmwareUpdate()
{
#if defined FP_UART_LINUX
	DBG("Start iMX Firmware Update");
	std::string command = FIRMWARE_UPDATE_CMD;
	std::future<void> future = std::async(std::launch::async, [&command]
		{
			// Execute the command
			std::system(FIRMWARE_UPDATE_CMD);
		});

#endif //FP_UART_LINUX
}

void FrontPannelManager::removeFirmwareFile()
{
#if defined FP_UART_LINUX
	std::string command = "rm -rf " + encryptedFwFileFullpath;
	std::system(command.c_str());
#endif
}

std::vector<int> extractImgFileVersion(const std::string& filename)
{
	std::regex regexVersion(R"(Ignite_(\d+)\.(\d+)\.(\d+)E\.img)");
	std::smatch key;
	std::vector<int> fversion;

	if (std::regex_match(filename, key, regexVersion)) {
		for (size_t i = 1; i < key.size(); ++i) {
			fversion.push_back(std::stoi(key[i]));
		}
	}
	return fversion;
}

bool compareVersions(const std::string & file1, const std::string & file2)
{
	auto version1 = extractImgFileVersion(file1);
	auto version2 = extractImgFileVersion(file2);
	return version1 > version2;
}

int8_t FrontPannelManager::isFirmwareImagePresent()
{
	std::string folderPath = "";
	std::string imageFormat = ".img";

	if ((fw_source == SwUdateSource_t::PC) || (fw_source == SwUdateSource_t::INTERNAL)) // PlayTribe or Internal upgrade
		folderPath = "/home/ignite/Firmware_Update";
	else if (fw_source == SwUdateSource_t::USBDISK) //From USB Mass Storage
		folderPath = "/mnt/flash";
	else
		return false;
	std::vector<std::string> swuFiles = getSWUFiles(folderPath, imageFormat);

	if (swuFiles.empty())
	{
		DBG("******************************No upgarde image files found in the folder.");
		return false;
	}
	else
	{
		for (const auto& swuFile : swuFiles)
		{
			DBG("UPGARED IMAGE FILE" + String(swuFile));
			(void)swuFile;
		}
	}
	std::sort(swuFiles.begin(), swuFiles.end(), compareVersions);
	fwFileName = swuFiles.front(); // taking latest version
	fwFileFullpath = folderPath + "/" + swuFiles.front();
	encryptedFwFileFullpath = fwFileFullpath;
	DBG("************************* FWFile Name -> " + String(fwFileName));
	DBG("***********************FWFile Full path -> " + String(fwFileFullpath));
	return true;
}

FwStatus_t FrontPannelManager::isValidFirmware()
{
#if defined FP_UART_LINUX
	if (juce::File(fwFileFullpath).exists())
	{
		if(getVersionfromFile(fwFileFullpath))
		{
			removeHeaderFromImage(fwFileFullpath);
			getFirmwareInfo();
			DBG("FW update pkg IMX software version: " + String(newImxFirmwareVersion));
			DBG("FW update pkg MCU software version: " + String(newGdFirmwareVersion));
			DBG("FW update pkg software version date: " + String(newFirmwareBuildDate));
			DBG("Current IMX software version: " + String(imxFirmwareVersion));
			DBG("Current MCU software version: " + String(gdFirmwareVersion));
			DBG("Current software version date: " + String(firmwareBuildDate));

			auto isValidFW = compareSWVersion(newImxFirmwareVersion, imxFirmwareVersion);
			if (isValidFW > 0)
			{
				DBG("New Firmware found");
				return FwStatus_t::NEW_FW;
			}
			else if (isValidFW == 0)
			{
				DBG("Firmware version is same");
				return FwStatus_t::SAME_FW;
			}
			else
			{
				DBG("Firmware version is old");
				return FwStatus_t::OLD_FW;
			}
		}
	}
	else
	{
		DBG("Firmware file  " + String(fwFileName) +  " does not exits");
		return FwStatus_t::FW_NOT_PRESENT;
	}
#endif
	return FwStatus_t::INVALID_FW;
}

bool FrontPannelManager::startFactoryReset()
{
#if defined FP_UART_LINUX
	std::system(FACTORY_RESET_CMD);
	std::system("mkdir -p /home/ignite/Firmware_Update");
#endif
	engine.SetCommand((moduleId_t)ModuleName::moduleUI, (moduleId_t)ModuleName::moduleFrontPannel, (paramId_t)FrontPannelParam::paramKey_DevStatus, (int)FACTORY_RESET_COMPLETED, false);
	return true;
}

void FrontPannelManager::rebootDevice()
{
#if defined FP_UART_LINUX
	std::system("fw_setenv fwupdate_status 1");
	std::system("reboot");
#endif
}

void FrontPannelManager::getSerialNumber()
{
#if defined FP_UART_LINUX
	std::fstream versionFile;
	versionFile.open("/sys/devices/soc0/serial_number", std::ios::in);
	if (versionFile.is_open())
	{
		getline(versionFile, serialNumber);
		DBG("Serial number: " + String(serialNumber));
	}
#else
	serialNumber = "1234567891234567";
#endif
}

void FrontPannelManager::getFirmwareInfo()
{
#if defined FP_UART_LINUX
	std::fstream versionFile;
	versionFile.open("/etc/software_version", std::ios::in);
	if (versionFile.is_open())
	{
		std::string str;
		while (getline(versionFile, str))
		{
			if (!str.find("DATE"))
			{
				str.erase(0, 5);
				firmwareBuildDate = str;
			}
			else if (!str.find("IMX"))
			{
				str.erase(0, 18);
				imxFirmwareVersion = str;
			}
			else if (!str.find("CTL"))
			{
				str.erase(0, 8);
				gdFirmwareVersion = str;
			}
		}

		versionFile.close();
	}
	imxFirmwareVersion[2] =(mcu_soft_version+0x30);
	std::cout <<"updated firmware version: "<< imxFirmwareVersion << std::endl;
#else
	firmwareBuildDate = "2023-10-25";
	imxFirmwareVersion = "Ignite_SW_v0.1.1";
	gdFirmwareVersion = "Control_SW_v0.1.1";
#endif
}

void FrontPannelManager::getMemoryStatus()
{
#if defined FP_UART_LINUX
	struct sysinfo memInfo;

	sysinfo(&memInfo);
	ramTotalSize = (memInfo.totalram * memInfo.mem_unit) / (1024 * 1024);
	ramFreeSize = (memInfo.freeram * memInfo.mem_unit) / (1024 * 1024);
#else
	ramTotalSize = 1000;
	ramFreeSize = 10;
#endif
}

void FrontPannelManager::getIntStorageStatus()
{
#if defined FP_UART_LINUX
	auto intStorage = fs::space("/data/ignite/");
#else
	auto intStorage = fs::space("C:/");
#endif
	intTotalSize = static_cast<int>((intStorage.capacity / (1024 * 1024)));
	intFreeSize = static_cast<int>((intStorage.free / (1024 * 1024)));
}

bool FrontPannelManager::isUSBStorageConnected()
{
#if defined FP_UART_LINUX
	std::fstream versionFile;
	versionFile.open("/proc/mounts", std::ios::in);

	if (versionFile.is_open())
	{
		std::string str;
		while (getline(versionFile, str))
		{
			if (str.find("flash") != std::string::npos)
			{
				DBG("USB Storage is connected");
				return true;
			}
		}
	}
#endif
	DBG("USB Storage is not connected");
	return false;
}

void FrontPannelManager::getUsbStorageStatus()
{
	if (isUSBStorageConnected())
	{
#if defined FP_UART_LINUX
		auto usbStorage = fs::space("/mnt/flash/");
#else
		auto usbStorage = fs::space("C:/");
#endif
		usbTotalSize = static_cast<int>((usbStorage.capacity / (1024 * 1024)));
		usbFreeSize = static_cast<int>((usbStorage.free / (1024 * 1024)));
	}
	else
	{
		usbTotalSize = -1;
		usbFreeSize = -1;
	}
}

void FrontPannelManager::HandleFwUpgrade(uint8_t id)
{
	SysMessage _xternalData;
	_xternalData.Header = SYS_SYTM_MSG;
	_xternalData.Direction = DIRECTION_OUT;
	_xternalData.Id = 0xEE;

	switch (id)
	{
	case SYS_SW_UPGRADE_READY://read the file
		if (openHexFile(&fileptr, FW_MCU_PATH))
		{
			_xternalData.Data[0] = SYS_SW_UPGRADE_DATA;
			_xternalData.size = (MCU_HEADRD_SIZE + getDataFromHexFile(&_xternalData.Data[1], 16) + 1);
			fwtxcount += 16;
			FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
		}

		break;
	case SYS_SW_UPGRADE_REQ:
		_xternalData.size = (MCU_HEADRD_SIZE + 1);
		_xternalData.Data[0] = SYS_SW_UPGRADE_REQ;
		FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
		fwtxcount = 0;
		break;
	case SYS_SW_UPGRADE_ACK:// request from mcu

		if (fileptr.eof())
		{
			DBG("ACK RECEIVED EOP");
			_xternalData.size = (MCU_HEADRD_SIZE + 1);
			_xternalData.Data[0] = SYS_SW_UPGARDE_DONE;
			engine.SendResponse((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_FwUpgrade, (bool)false);
		}
		else
		{
			DBG("ACK RECEIVED");
			_xternalData.Data[0] = SYS_SW_UPGRADE_DATA;
			_xternalData.size = (MCU_HEADRD_SIZE + getDataFromHexFile(&_xternalData.Data[1], 16) + 1);
			fwtxcount += 16;
			DBG("bytes recieved ");
			DBG(std::to_string(fwtxcount));
		}
		FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
		break;
	case SYS_SW_UPGARDE_DONE:
		fileptr.close();
		break;
	case SYS_SW_UPDATE_SOM:
		_xternalData.size = (MCU_HEADRD_SIZE + 1);
		_xternalData.Data[0] = SYS_SW_UPDATE_SOM;
		FrontPanneExternalWriteQ.put(_xternalData, TimeOut::kNoWait);
		DBG("INFORMED MCU ABOUT SOM UPATE");
		break;
	}
}


uint8_t FrontPannelManager::openHexFile(std::ifstream* filept, const char* file_path)
{
#if defined FP_UART_LINUX
	fs::path filepath = (const char*)file_path;
#else
	fs::path filepath = "../../../Tool/Release/IgniteRelease.hex";
#endif

#if 0
	fileptr.exceptions(std::ifstream::failbit); // enable throwing an exception on open failure
	try {
		fileptr.open(filename, std::ios::in); // try to open test.txt for reading // try to open test.txt for reading
	}
	catch (const std::system_error& e) {
		// catch the system error
		DBG(e.what());
		//	cout << "System error: " << e.code() << " - " << e.what() << endl; // print the error code and message
	}
	return 0;
#endif
#if 1
	fileptr = std::ifstream(filepath, std::ios::in);

	if (fileptr.is_open())
	{
		DBG("Hex file opened successfully!");
		return 1;
	}
	else
	{
		DBG("Error in opening hex file");
		return 0;
	}
#endif
}

uint8 FrontPannelManager::getDataFromHexFile(uint8* data, uint8_t bytes)
{
	uint8 i;

	for (i = 0; i < bytes; i++)
	{
		if (fileptr.eof())
		{
			fileptr.close();
			DBG("end of file");
			break;
		}
		else
		{
			(fileptr) >> std::hex >> data[i];
			//DBG(data[i]); //data will be transfered to MCU for future use
		}
	}
	DBG("end of 16 bytes");
	return i;
}

DeviceInfo FrontPannelManager::fillDeviceStatusInfo()
{
	DBG(" Filling DeviceInfo struct data from device variables");
	getSerialNumber();
	getFirmwareInfo();
	getMemoryStatus();
	getIntStorageStatus();
	getUsbStorageStatus();
	struct DeviceInfo info = { 0 };

	memcpy(info.serialNumber, (char*)serialNumber.c_str(), sizeof(info.serialNumber));
	memcpy(info.firmwareVersion, imxFirmwareVersion.c_str(), sizeof(info.firmwareVersion));
	memcpy(info.firmwareBuildDate, firmwareBuildDate.c_str(), sizeof(info.firmwareBuildDate));
	// fill memory status
	memcpy(info.memoryStatus.total, (std::to_string(ramTotalSize) + " MB").c_str(), sizeof(info.memoryStatus.total));
	memcpy(info.memoryStatus.available, (std::to_string(ramFreeSize) + " MB").c_str(), sizeof(info.memoryStatus.available));
	int used = ramTotalSize - ramFreeSize;
	memcpy(info.memoryStatus.used, (std::to_string(used) + " MB").c_str(), sizeof(info.memoryStatus.used));
	//fill internal stoatge status
	memcpy(info.internalStorageStatus.totalSpace, (std::to_string(intTotalSize) + " MB").c_str(), sizeof(info.internalStorageStatus.totalSpace));
	memcpy(info.internalStorageStatus.availableSpace, (std::to_string(intFreeSize) + " MB").c_str(), sizeof(info.internalStorageStatus.availableSpace));
	int usedspace = intTotalSize - intFreeSize;
	memcpy(info.internalStorageStatus.usedSpace, (std::to_string(usedspace) + " MB").c_str(), sizeof(info.internalStorageStatus.usedSpace));
	//fill usb storage status
	if (isUSBStorageConnected())
	{
		std::string totalSpaceStr = std::to_string(usbTotalSize) + " MB";
		std::string availableSpaceStr = std::to_string(usbFreeSize) + " MB";

		// Copy the resulting strings to the respective arrays
		memcpy(info.usbStorageStatus.totalSpace, totalSpaceStr.c_str(), sizeof(info.usbStorageStatus.totalSpace));
		memcpy(info.usbStorageStatus.availableSpace, availableSpaceStr.c_str(), sizeof(info.usbStorageStatus.availableSpace));
	}
	else
	{
		memcpy(info.usbStorageStatus.totalSpace, "Not Connected", sizeof(info.usbStorageStatus.totalSpace));
		memcpy(info.usbStorageStatus.availableSpace, "Not Connected", sizeof(info.usbStorageStatus.availableSpace));
	}
	return info;
}
