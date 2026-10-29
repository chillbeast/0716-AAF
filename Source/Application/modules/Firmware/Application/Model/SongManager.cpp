#include <string>
#include <iostream>
#include <iterator>
#include <list>
#include <assert.h>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <execution>
#include "Firmware/Application/Model/SongManager.h"
#include "Firmware/Application/Model/IgniteEngine.h"
#include "Common/Modules/IModule.h"
#include "common_defs/source/EngineDataStruct.h"
#include "Common/Modules/Track.h"

#include <iomanip>
#include <stdlib.h>

#ifdef __linux__
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#endif

using namespace Ignite;
using namespace Ignite;
using namespace juce::dsp;



#ifdef _WIN32
#define NOMINMAX /* avoid conflicts with std lib function min & max*/
#include <windows.h>
#include <mmeapi.h>
#include <mmreg.h>

WAVEFORMATEX windowsAudioFormat;
HWAVEOUT windowsAudioHandle = NULL;
WAVEHDR windowsAudioHeader;

#endif

#ifdef TEST_ENABLE_SAMPLE
#include <juce_audio_formats/juce_audio_formats.h>
juce::AudioBuffer<float> audioBuffer;
uint32_t lengthInSamples = 0;
uint32_t readaudioBuffer = 0;
uint32_t readaudiofile = 0;
uint32_t adaudioRepeatcount = 100;// number of time the sample should repeat, it may play without a delay
uint32_t IndexReadaudiofile = 0;
void SongManager::readAudioFromFile()
{
	File resourceFile("C:/untitled.wav");
	juce::AudioFormatManager formatManager;
	formatManager.registerBasicFormats();

	auto* reader = formatManager.createReaderFor(resourceFile.createInputStream());
	audioBuffer.setSize(reader->numChannels, reader->lengthInSamples);
	reader->read(&audioBuffer, 0, reader->lengthInSamples, 0, true, true);
	lengthInSamples = reader->lengthInSamples;
	delete reader;
	return;
}
#endif

SongManager * SongManager::instance = nullptr;
#ifdef __linux__
//static var definition 
bool SongManager::usbHostMidiDevConnectSigReceived = false;
bool SongManager::usbHostMidiDevDisconnectSigReceived = false;

void *SongManager::handleUsbHostMidiDevices (void *arg) 
{
	while (!CThread::desktopShutdownRequested())
	{
		if (SongManager::usbHostMidiDevConnectSigReceived)
		{
			//reset
			SongManager::usbHostMidiDevConnectSigReceived = false;

			if (SongManager::instance != nullptr)
			{
				auto devList = SongManager::instance->getUsbHostMidiDeviceList();
				auto newDevs = SongManager::instance->findNewlyAddedUsbMidiDevices(devList);
				SongManager::instance->initUsbMidiDevPorts(newDevs);
				
				if (newDevs.size() > 0)
				{
					SongManager::instance->publishUsbHostMidiChangeNotificationToUI();
				}
			}
		}

		if (SongManager::usbHostMidiDevDisconnectSigReceived)
		{
			//reset
			SongManager::usbHostMidiDevDisconnectSigReceived = false;

			auto devList = SongManager::instance->getUsbHostMidiDeviceList();
			auto removedDevs = SongManager::instance->findRecentlyRemovedUsbMidiDevices(devList);
			SongManager::instance->deinitUsbMidiDevPorts(removedDevs);

			if (removedDevs.size() > 0)
			{
				SongManager::instance->publishUsbHostMidiChangeNotificationToUI();
			}
		}

		sleep(1);
		
	}
}

void SongManager::handle_signal(int sig)
{
	//used to inform USB host device connection
    if (sig == SIGUSR1)
	{
		SongManager::usbHostMidiDevConnectSigReceived = true;
    }

    if (sig == SIGUSR2)
	{
		SongManager::usbHostMidiDevDisconnectSigReceived = true;
	}
}

#endif /* __linux__ */

#ifdef USE_INTERNAL_20MS_TIMER

#ifdef __linux__

void audioGenTimer (union sigval dwUser)

#elif defined _WIN32

void CALLBACK audioGenTimer (UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)

#endif /* _WIN32 */

{
	SongManager* pSongManager;
	AudioDataType TwentyMsBuffer[INTERNAL_AUDIO_20MS_BLOCK_SIZE] = { 0 }; //synthesize 20 ms seconds

#ifdef _WIN32
	pSongManager = reinterpret_cast<SongManager*>(dwUser);
#elif defined __linux__
	pSongManager = reinterpret_cast<SongManager*>(dwUser.sival_ptr);
#endif /* __linux__ */

#ifdef TEST_ENABLE_SAMPLE
	if (!readaudioBuffer && readaudiofile)
	{
		SongManager::readAudioFromFile();
		readaudioBuffer = 1;
	}
	if (readaudiofile && readaudioBuffer)
	{
		if (readaudiofile <= adaudioRepeatcount)
		{
			for (auto i = 0; i < INTERNAL_AUDIO_20MS_BLOCK_SIZE; i++)
			{
				TwentyMsBuffer[i] = audioBuffer.getSample(0, IndexReadaudiofile);
				IndexReadaudiofile = (IndexReadaudiofile + 1) % lengthInSamples;
				if (IndexReadaudiofile == 0)
					readaudiofile++;
			}
		}
	}
	else
#endif // TEST_ENABLE_SAMPLE
	{
		pSongManager->renderTracksAndMix(TwentyMsBuffer);
	}
#ifdef USE_JUCE_AUDIO

	SongManager::writeToBuffer(pSongManager, TwentyMsBuffer);

#else /* USE_JUCE_AUDIO */

	SongManager::directPlatformPlay(TwentyMsBuffer, sizeof(TwentyMsBuffer));

#endif /* ! USE_JUCE_AUDIO */

#ifdef LOG_PCM_TO_FILE

	//SongManager::logToFile_3(pSongManager, TwentyMsBuffer, sizeof(TwentyMsBuffer));

#endif /* LOG_PCM_TO_FILE */
}


struct buffer
{
	float buffer[INTERNAL_AUDIO_20MS_BLOCK_SIZE];
	bool dataFull;
}outData[3];

unsigned int writeIndex = 0;
unsigned int readIndex = 0;

void SongManager::writeToBuffer(SongManager* songManagerP, AudioDataType* data)
{
	if (outData[writeIndex].dataFull == true)
	{
		printf("DATA OVERFLOW\n");
	}
	memcpy(outData[writeIndex].buffer, data, INTERNAL_AUDIO_20MS_BLOCK_SIZE * sizeof(AudioDataType));
	outData[writeIndex].dataFull = true;

#ifdef LOG_PCM_TO_FILE
		//songManagerP->logToFile_1(outData[writeIndex].buffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE * sizeof(AudioDataType));
#endif

	writeIndex++;
	if (writeIndex > 2)
	{
		writeIndex = 0;
	}
}

void SongManager::readFromBuffer(AudioDataType* data)
{
	if (outData[writeIndex].dataFull == true)
	{
		memcpy(data, outData[readIndex].buffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE * sizeof(AudioDataType));
		outData[readIndex].dataFull = false;
		readIndex++;
		if (readIndex > 2)
		{
			readIndex = 0;
		}
	}
	else
	{
		printf("DATA UNDER FLOW\n");
	}

#ifdef LOG_PCM_TO_FILE
	//logToFile_2(data, INTERNAL_AUDIO_20MS_BLOCK_SIZE*sizeof(AudioDataType));
#endif

}
#endif //USE_INTERNAL_20MS_TIMER

void midiClkOutTimerFunc(SongManager* songMgr)
{
	MidiMessage msg = MidiMessage::midiClock();

	while (songMgr->_clkOutTimerRunning)
	{
		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnDin1Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::dinMidi1);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnDin2Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::dinMidi2);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnDin3Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::dinMidi3);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnDin4Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::dinMidi4);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnDin5Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::dinMidi5);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbD1Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbDevMidi1);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbD2Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbDevMidi2);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbD3Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbDevMidi3);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbD4Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbDevMidi4);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbD5Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbDevMidi5);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbD6Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbDevMidi6);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbH1Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbHostMidi1);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbH2Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbHostMidi2);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbH3Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbHostMidi3);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbH4Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbHostMidi4);
		}

		if (songMgr->engine.settingsMidiClkCnfg->isClkOutEnabledOnUsbH5Port())
		{
			songMgr->sendMidiExternal(msg, midiDataIOPorts::usbHostMidi5);
		}

		std::this_thread::sleep_for(songMgr->_clockOutDurationUs);
	}
}

void midiClockTimerFunc(SongManager* songMgr)
{
	std::chrono::milliseconds waitDurationMs{ 1000 };
	uint32 midiClockAbsentIntervalCount = 0;
	std::chrono::high_resolution_clock::time_point time1;
	uint32_t count1 = 0;

	while (songMgr->_midiClkSlaveModeActive && songMgr->_midiClkRxInProgress)
	{
		if (midiClockAbsentIntervalCount == 0)
		{
			//get current time
			time1 = std::chrono::high_resolution_clock::now();

			//get current counter
			count1 = songMgr->_midiClockCounter.load(std::memory_order_acquire);
		}

		//wait for duration
		std::this_thread::sleep_for(waitDurationMs);

		//get current count
		auto count2 = songMgr->_midiClockCounter.load(std::memory_order_acquire);

		//get current time
		auto time2 = std::chrono::high_resolution_clock::now();

		uint32_t countDiff = 0;
		if (count2 > count1)
		{
			countDiff = count2 - count1;
		}
		else
		{
			countDiff = 4294967295 - count1 + count2;
		}

		if (countDiff == 0)
		{
			//no clock recived for last interval (1 sec)
			++midiClockAbsentIntervalCount;

			//max interval expected is at 1PP4Q at 20 BPM - 12 seconds
			if (midiClockAbsentIntervalCount >= 12)
			{
				midiClockAbsentIntervalCount = 0;
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "No midi clock / Unsupported tempo");
			}
		}
		else
		{
			midiClockAbsentIntervalCount = 0;
			//calculate frequency
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time2 - time1).count();
			float freqHz = ((float)(countDiff * 1000000) / duration);

			songMgr->handleMidiClkIn (freqHz);
		}
	}
}

void timerFunc (uint8_t& count, std::chrono::milliseconds waitDuration)
{
	std::this_thread::sleep_for (waitDuration);
	count = 0;//reset
}

void reStartTimerFunc (SongManager* songMgr, std::atomic<uint8_t>& count, int32_t sceneId, std::chrono::milliseconds waitDuration)
{
	int32_t newSceneId = 0;

	std::this_thread::sleep_for (waitDuration);

	//check count at expiry
	switch (count)
	{
		case 1://reStart scene
			newSceneId = sceneId;//reStart same scene
			break;

		case 2://restart prev scene
			newSceneId = (sceneId == 0) ? 0 : (sceneId - 1);
			break;

		case 3://restart song
			newSceneId = 0;
			break;
	}

	count.store(0);//reset
	songMgr->stopSong ();
	songMgr->playSong (newSceneId);
}

std::unordered_map <ScaleList, ScaleParams> ftsList =
{
	{	ScaleList::ScaleAllNotes,			{"FTS-Off", 	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }}		},
	{	ScaleList::ScaleAlgerian,			{"Algerian", 	{ 2, 1, 2, 1, 1, 1, 3, 1 }}					},
	{	ScaleList::ScaleDiminished,			{"Diminished", 	{ 2, 1, 2, 1, 2, 1, 2, 1 } }				},

	{	ScaleList::ScaleOriental,			{"Oriental", 			{ 1, 3, 1, 1, 3, 1, 2 } }			},
	{	ScaleList::ScaleByzantine,			{"Byzantine", 			{ 1, 3, 1, 2, 1, 3, 1 } }			},
	{	ScaleList::ScaleAeolian,			{"Aeolian", 			{ 2, 1, 2, 2, 1, 2, 2 } }			},
	{	ScaleList::ScaleHarmonicMinor,		{"HarmonicMinor", 		{ 2, 1, 2, 2, 1, 3, 1 } }			},
	{	ScaleList::ScaleMelodicMinor,		{"MelodicMinor", 		{ 2, 1, 2, 2, 2, 2, 1 } }			},
	{	ScaleList::ScaleHungarian1,			{"Hungarian1", 			{ 2, 1, 3, 1, 1, 3, 1 } }			},
	{	ScaleList::ScaleLydianDiminished, 	{"LydianDiminished",	{ 2, 1, 3, 1, 2, 2, 1 } }			},
	{	ScaleList::ScaleArabian,			{"Arabian", 			{ 2, 2, 1, 1, 2, 2, 2 } }			},
	{	ScaleList::ScaleIndian2,			{"Indian2", 			{ 2, 2, 1, 2, 1, 2, 2 } }			},
	{	ScaleList::ScaleMajor,				{"Major", 				{ 2, 2, 1, 2, 2, 2, 1 } }			},
	{	ScaleList::ScaleMixoLydian,			{"MixoLydian", 			{ 2, 2, 2, 1, 2, 1, 2 } }			},
	{	ScaleList::ScaleHungarian2,			{"Hungarian2", 			{ 3, 1, 2, 1, 2, 1, 2 } }			},

	{	ScaleList::ScalePersian,			{"Persian", 	{ 1, 3, 1, 2, 1, 3 } }						},
	{	ScaleList::ScaleBlues2,				{"Blues2", 		{ 2, 1, 1, 3, 2, 3 } }						},
	{	ScaleList::ScalePrometheus,			{"Prometheus",	{ 2, 2, 2, 3, 1, 2 } }						},
	{	ScaleList::ScaleWholeNote,			{"WholeNote", 	{ 2, 2, 2, 2, 2, 2 } }						},
	{	ScaleList::ScaleAugmented,			{"Augmented", 	{ 3, 1, 3, 1, 3, 1 } }						},
	{	ScaleList::ScaleBlues1,				{"Blues1", 		{ 3, 2, 1, 1, 3, 2 } }						},

	{	ScaleList::ScaleIndian1,			{"Indian1", 		{ 1, 2, 2, 3, 2 } }						},
	{	ScaleList::ScaleBalinese,			{"Balinese", 		{ 1, 2, 4, 1, 4 } }						},
	{	ScaleList::ScaleJapanese3,			{"Japanese3", 		{ 1, 4, 1, 4, 2 } }						},
	{	ScaleList::ScaleJapanese1,			{"Japanese1", 		{ 1, 4, 2, 3, 2 } }						},
	{	ScaleList::ScaleJapanese4,			{"Japanese4", 		{ 2, 1, 4, 1, 4 } }						},
	{	ScaleList::ScaleAncientChinese,		{"AncientChinese", 	{ 2, 2, 2, 1, 2 } }						},
	{	ScaleList::ScaleMongolian,			{"Mongolian", 		{ 2, 2, 3, 2, 3 } }						},
	{	ScaleList::ScaleJapanese2,			{"Japanese2", 		{ 2, 3, 2, 2, 3 } }						},
	{	ScaleList::ScaleMinorPentatonic,  	{"MinorPentatonic",	{ 3, 2, 2, 3, 2 } }						},
	{	ScaleList::ScaleChinese,			{"Chinese", 		{ 4, 2, 1, 4, 1 } }						},

	{	ScaleList::ScaleUD1,				{"UD1", 	{  } }											},
	{	ScaleList::ScaleUD2,				{"UD2", 	{  } }											},
	{	ScaleList::ScaleUD3,				{"UD3", 	{  } }											},
	{	ScaleList::ScaleUD4,				{"UD4", 	{  } }											},
	{	ScaleList::ScaleUD5,				{"UD5", 	{  } }											},
	{	ScaleList::ScaleUD6,				{"UD6", 	{  } }											},
	{	ScaleList::ScaleUD7,				{"UD7", 	{  } }											},
	{	ScaleList::ScaleUD8,				{"UD8", 	{  } }											},
	{	ScaleList::ScaleUD9,				{"UD9", 	{  } }											},
	{	ScaleList::ScaleUD10,				{"UD10",	{  } }											},
};
//TODO : these names are not final.
//Currently added as place holder, to implement the midi remote control
//These may get updated, once midi part is implemented
//And USB midi wil also get added to this list
//Assuming HW midi ports are numbered from 0 to 4
std::map <int8_t, std::string> gMidiPortNameList =
{
	//Indices are mapped exactly as how GUI uses.
	{(int8_t)midiDataIOPorts::dinMidi1, IGNITE_DIN_MIDI_PORT_1_NAME},
	{(int8_t)midiDataIOPorts::dinMidi2, IGNITE_DIN_MIDI_PORT_2_NAME},
	{(int8_t)midiDataIOPorts::dinMidi3, IGNITE_DIN_MIDI_PORT_3_NAME},
	{(int8_t)midiDataIOPorts::dinMidi4, IGNITE_DIN_MIDI_PORT_4_NAME},
	{(int8_t)midiDataIOPorts::dinMidi5, IGNITE_DIN_MIDI_PORT_5_NAME},


	//Added USB DEVICE MIDI ports, as per the names listed in Linux aplaymidi -l
	{(int8_t)midiDataIOPorts::usbDevMidi1, IGNITE_USB_D_MIDI_PORT_1_NAME},
	{(int8_t)midiDataIOPorts::usbDevMidi2, IGNITE_USB_D_MIDI_PORT_2_NAME},
	{(int8_t)midiDataIOPorts::usbDevMidi3, IGNITE_USB_D_MIDI_PORT_3_NAME},
	{(int8_t)midiDataIOPorts::usbDevMidi4, IGNITE_USB_D_MIDI_PORT_4_NAME},
	{(int8_t)midiDataIOPorts::usbDevMidi5, IGNITE_USB_D_MIDI_PORT_5_NAME},
	{(int8_t)midiDataIOPorts::usbDevMidi6, IGNITE_USB_D_MIDI_PORT_6_NAME},

	{(int8_t)midiDataIOPorts::usbHostMidi1, IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME},
	{(int8_t)midiDataIOPorts::usbHostMidi2, IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME},
	{(int8_t)midiDataIOPorts::usbHostMidi3, IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME},
	{(int8_t)midiDataIOPorts::usbHostMidi4, IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME},
	{(int8_t)midiDataIOPorts::usbHostMidi5, IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME},

	//These are not Midi ports
	//{(int8_t)IGNITE_SYNC_PORT, IGNITE_SYNC_PORT_NAME},
	//{(int8_t)IGNITE_INTERNAL_CLK, IGNITE_INTERNAL_CLK_NAME},
};
#include "tsf.h"
#include "SoundFontProgram.h"
/* init and load TSF and sound font and use the copy in all song,scene tracks */
void SongManager::init_tsf()
{
    const int savedVoices=int(juce::JSON::parse(desktopSettingsFile()).getProperty("SynthVoices",32));
    if(savedVoices==8 || savedVoices==16 || savedVoices==32 || savedVoices==64 || savedVoices==128)desktopVoiceLimit=savedVoices;
    auto path = juce::SystemStats::getEnvironmentVariable("IGNITE_SOUNDFONT", "");
    if (path.isEmpty())
        path = juce::JSON::parse(desktopSettingsFile()).getProperty("SoundFont", "").toString();
    if (path.isEmpty())
    {
        auto local = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getSiblingFile("Ignite.sf2");
        path = local.existsAsFile() ? local.getFullPathName() : juce::String(SOUND_FONT_FILE);
    }
    std::string error;
    if (!loadDesktopSoundFont(path.toStdString(), error))
        juce::Logger::writeToLog("No usable SoundFont at " + path + "; internal sound disabled. " + error);
}
void* SongManager::get_tsf_instance() {
	return (_songTSf);
}
class Track;
SongManager::SongManager(IParamChangeListener* listener, IgniteEngine& model) : paramlistener(listener), engine(model), Module("SongManager", (moduleId_t)ModuleName::moduleSong, (unsigned int)SongParam::paramSongMax, listener)

#ifdef ENABLE_THREADS_TO_PLAY
        ,
#ifndef __linux__
		TrackPlayThread_0("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
		TrackPlayThread_1("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
		TrackPlayThread_2("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
		TrackPlayThread_3("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
		TrackPlayThread_4("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
		TrackPlayThread_5("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
		TrackPlayThread_6("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
		TrackPlayThread_7("TrackPlay", IgniteConstants::threadStackSize, osPriorityHigh),
#endif /* ! __linux __ */
		TrackPlayQ_0(IgniteConstants::msgQueueSize),
		TrackPlayQ_1(IgniteConstants::msgQueueSize),
		TrackPlayQ_2(IgniteConstants::msgQueueSize),
		TrackPlayQ_3(IgniteConstants::msgQueueSize),
		TrackPlayQ_4(IgniteConstants::msgQueueSize),
		TrackPlayQ_5(IgniteConstants::msgQueueSize),
		TrackPlayQ_6(IgniteConstants::msgQueueSize),
		TrackPlayQ_7(IgniteConstants::msgQueueSize)
#endif
{
	init_tsf();
	AddParameter<uint8_t>((paramId_t)SongParam::paramAddSongCmd, 0, 127, 0);// no arg
	AddParameter<int32_t>((paramId_t)SongParam::paramDeleteSongCmd, 0, 1000, 0);
	AddParameter<int32_t>((paramId_t)SongParam::paramActiveSong, 0, 1000, 0);
	AddParameter<uint8_t>((paramId_t)SongParam::paramPlaySongCmd, 0, 127, 0);//Support 127 scenes max
	AddParameter<uint8_t>((paramId_t)SongParam::paramPauseSongCmd, 0, 127, 0);
	AddParameter<float>((paramId_t)SongParam::paramSongTempo, 20.0, 360.0, 130.0);
	AddParameter<uint8_t>((paramId_t)SongParam::paramSongSwing, 0, 16, 0);
	AddParameter<std::string>((paramId_t)SongParam::paramSongName, "0", "0", "0");
	AddParameter<uint8_t>((paramId_t)SongParam::paramSaveSongCmd, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)SongParam::paramUndoCmd, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)SongParam::paramSongNameList, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)SongParam::paramUndoListCmd, 0, 127, 0);
	AddParameter<int8_t>((paramId_t)SongParam::paramClearUndoList, 0, 127, 0);
	AddParameter<uint8_t>((paramId_t)SongParam::paramStopSongCmd, 0, 127, 0);
	//AddParameter<bool>((paramId_t)SongParam::paramFillMode, 0, 1, 0);
	AddParameter<bool>((paramId_t)SongParam::paramStartRecordCmd, 0, 1, 0);
	AddParameter<bool>((paramId_t)SongParam::paramStopRecordCmd, 0, 1, 0);
	AddParameter<int32_t>((paramId_t)SongParam::paramMoveSong, 0, 1000, 0);
	AddParameter<std::string>((paramId_t)SongParam::paramCopySong, "0", "0", "0");
	AddParameter<uint16_t>((paramId_t)SongParam::paramRecordEventFilter, 0, 64, 0);
	AddParameter<int8_t>((paramId_t)SongParam::paramTrackCountCmd, 0, 64, 16);
	AddParameter<bool>((paramId_t)SongParam::paramTrackSoloMode, 0, 1, 0);
	AddParameter<bool>((paramId_t)SongParam::paramFillModeSwitchOn, 0, 1, 0);
	AddParameter<int8_t>((paramId_t)SongParam::paramFillModeType, 0, 3, 0);
	AddParameter<int8_t>((paramId_t)SongParam::paramFillModeRelease, 0, 2, 0);
	AddParameter<bool>((paramId_t)SongParam::paramApplySongTempoToScenes, 0, 1, 0);
	AddParameter<bool>((paramId_t)SongParam::paramApplySongSwingToScenes, 0, 1, 0);
	AddParameter<bool> ((paramId_t)SongParam::paramReStartSongCmd, 0, 1, 0);
	AddParameter<bool> ((paramId_t)SongParam::paramFtsNameList, 0, 1, 0);//used for get, arg not used

	AddParameter<uint8_t> ((paramId_t)SongParam::ParamTestNumCreateSong, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamTestSetNumScene, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamTestSetNumTrack, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamTestSetNumPattern, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamTestSetNumPatternBars, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamTestSetNumInstrument, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamDeleteAllSong, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamDeleteAllSongInstPattern, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamEngineReady, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamEngineCreateDataBackup, 0, 255, 0);
	AddParameter<uint8_t> ((paramId_t)SongParam::ParamEngineRestoreDataBackup, 0, 255, 0);

	AddParameter<bool> ((paramId_t)SongParam::paramActiveTrackTxInstrumentAvailability, 0, 1, 0);
	AddParameter<bool> ((paramId_t)SongParam::paramAuditioningState, 0, 1, 0);

	AddParameter<float>((paramId_t)SongParam::paramSyncIn, 0, 288, 0);
	AddParameter<float>((paramId_t)SongParam::paramSyncOut, 0, 288, 0);
	AddParameter<int8_t>((paramId_t)SongParam::paramSaveAll, 0, 1, 0);

	AddParameter<bool>((paramId_t)SongParam::paramUsbHostMidiChangeNotification, 0, 1, 0);// value not used currently
	AddParameter<bool>((paramId_t)SongParam::paramUsbHostMidiDeviceNameList, 0, 1, 0);// value not used currently
	AddParameter<int>((paramId_t)SongParam::paramSystemState, SYSTEM_STATE::IDLE, SYSTEM_STATE::IDLE, SYSTEM_STATE::RECORD_PAUSE);
	//Read from file system and update the Song manager data
	//update numberOfSongs before calling populateSongInfoListFromFileSystem
	numberOfSongs = engine.fileHandler->GetNumberofSongs();
	populateSongInfoListFromFileSystem();

	//UI will read the last saved active song ID and will set it as active Song
	mainInfoData info;
	auto valid = engine.fileHandler->getMainInfo (info);
	if (valid >= 0)
	{
		savedActiveSongId = info.activeSongId;
		_masterVolume = info.masterVolume;
	}
	else
	{
		savedActiveSongId = -1;
		_masterVolume = 0.7f;
	}

	activeSongId = -1;
	desktopOffline=juce::SystemStats::getEnvironmentVariable("IGNITE_HEADLESS_RENDER","")=="1";
    if(!desktopOffline)openMidiInterfaces();
	for (uint8_t i = 0; i < AUDITIONING_MAX_PORT; i++)
	{
		std::string TrackName = "AuditioningTrack" + std::to_string(i);
		AuditioningTrack[i] = std::make_unique<Track>((int8_t)(0), TrackName, false, this);//Track to play internal sound for Midi in
		AuditioningTrack[i]->AuditioningTrackInit();
		isAuditioningTrackEnabled[i] = false;
	}

#ifdef __linux__

	//create thread for usb host midi device
	int ret = pthread_attr_init(&usbHostMidiDeviceThreadAttr);
	if (ret == 0)
	{
		ret = pthread_attr_setdetachstate(&usbHostMidiDeviceThreadAttr, PTHREAD_CREATE_DETACHED);
		if(ret 	== 0)
		{
			ret = pthread_create(&usbHostMidiDeviceThreadId, &usbHostMidiDeviceThreadAttr, handleUsbHostMidiDevices, NULL);
			if(ret != 0)
			{
				printf("unable to create USB HOST MIDI thread\n");
			}
		}
		else
		{
			printf("Unable to set detach state for USB HOST MIDI thread\n");
		}
	}
	else
	{
		printf("Unable to init attr for USB HOST MIDI thread\n");
	}
#endif
	engine.updateModuleReadyStatus((moduleId_t)ModuleName::moduleSong, true);
}

SongManager::~SongManager()
{
}
void SongManager::shutdownDesktop()
{
#ifdef _WIN32
    if (dm != nullptr)
    {
        dm->removeAudioCallback(this);
        dm->closeAudioDevice();
    }
    _clkOutTimerRunning = false;
    _midiClkSlaveModeActive = false;

#ifdef ENABLE_THREADS_TO_PLAY
    TrackPlayThread_0.stopThread();
    TrackPlayThread_1.stopThread();
    TrackPlayThread_2.stopThread();
    TrackPlayThread_3.stopThread();
    TrackPlayThread_4.stopThread();
    TrackPlayThread_5.stopThread();
    TrackPlayThread_6.stopThread();
    TrackPlayThread_7.stopThread();
#endif
#endif
}

bool SongManager::populateSongFromFileSystem(int32_t songId)
{
	bool activeSongSet = false;

	if (!engine.isIDeviceReady())
		return false;

	std::string songName = getSongNameFromId(songId);

	// Read the Song data of UI selected song
	struct SongData songInfo = engine.fileHandler->GetSongInfo(songName);

	if (songInfo.songId == songId)
	{
		// create song skeleton - scene and track data will be populated from their respective data structures
		// create the new active song with data from filesystem
		// create with 0 scenes, populate scenes as we read it from filesystem
		// update un-initialised song fields
		activeSong = std::make_unique<CSong> ((int32_t)(songInfo.songId), songName, songInfo.songTempo, songInfo.songSwing, 0, this/*,songInfo.isFillMode*/);
		activeSong->_songData->gTrackCount = songInfo.gTrackCount;
		activeSong->_songData->songId = songInfo.songId;
		activeSong->_songData->songSwing = songInfo.songSwing;
		activeSong->_songData->applySongTempoToScenes = songInfo.applySongTempoToScenes;
		activeSong->_songData->applySongSwingToScenes = songInfo.applySongSwingToScenes;

		// SCENE
		// Read scene names
		std::vector<std::string> savedSceneNameList = engine.fileHandler->GetSceneList(songName, 1, (uint16_t)songInfo.numberOfScenes);
		if (savedSceneNameList.size() != songInfo.numberOfScenes)
		{
			songInfo.numberOfScenes = (uint32_t)savedSceneNameList.size();
		}

		// read Scene data
#ifdef ENABLE_PARALLEL_EXE
		std::for_each (
			//std::execution::par,
			std::execution::par_unseq,
			savedSceneNameList.begin (),
			savedSceneNameList.end (),
			[this, songName, savedSceneNameList](auto&& item)
			{
				std::string savedSceneName = item;
#else
		for (int32_t i = 0; i < songInfo.numberOfScenes; i++)
		{
			std::string savedSceneName = savedSceneNameList[i];
#endif
				struct SceneData sceneInfo = engine.fileHandler->GetSceneInfo (songName, savedSceneName);
				// create scene skeleton
				// create with 0 tracks, populate tracka s we read track info
				Scene tempScene (sceneInfo.sceneId, savedSceneName, sceneInfo.sceneTempo, sceneInfo.sceneSwing, 0, sceneInfo.trackMuteStatus, this);
				// update un-initialised fileds
				tempScene._sceneData->gBarLenInSteps = sceneInfo.gBarLenInSteps;
                tempScene._sceneData->desktopMeterNumerator=sceneInfo.desktopMeterNumerator;
                tempScene._sceneData->desktopMeterDenominator=sceneInfo.desktopMeterDenominator;
				tempScene._sceneData->numBars = sceneInfo.numBars;
				tempScene._sceneData->mode = sceneInfo.mode;
				tempScene._sceneData->transposeAfterFTS = sceneInfo.transposeAfterFTS;
				tempScene._sceneData->scaleFTS = sceneInfo.scaleFTS;
				tempScene._sceneData->noteFTS = sceneInfo.noteFTS;
				tempScene._sceneData->transposeValue = sceneInfo.transposeValue;

				// TRACK
				// get tracks info for this scene
				std::vector<TrackData> TrackDataList = engine.fileHandler->GetTrackDataList (songName, savedSceneName, sceneInfo.numberOfTracks);

#if 0 //ENABLE_PARALLEL_EXE
				std::for_each (
					std::execution::par_unseq,
					TrackDataList.begin (),
					TrackDataList.end (),
					[this, songName, &tempScene] (auto&& trackInfo)
#else
					for (auto trackInfo : TrackDataList)
#endif
					{
						Track tempTrack (trackInfo.trackId, trackInfo.trackName, trackInfo.trackMute, this);
						if (!trackInfo.trackMute)
							tempScene._sceneData->trackMuteStatus &= ~((uint64_t)((uint64_t)1 << trackInfo.trackId));

						// update un-initalised params
						tempTrack._trackData->trackSolo = trackInfo.trackSolo;
						tempTrack._trackData->trackRecordEnable = trackInfo.trackRecordEnable;
						tempTrack._trackData->fillPattern = nullptr;
						tempTrack._trackData->trackRxInstrument = nullptr;
						tempTrack._trackData->trackTxInstrument = nullptr;
						tempTrack._trackData->tvTrackDataCount = trackInfo.tvTrackDataCount;
						tempTrack._trackData->tvTrackData = trackInfo.tvTrackData;
                        tempTrack._trackData->hasDesktopFx=trackInfo.hasDesktopFx;tempTrack._trackData->desktopFxState=trackInfo.desktopFxState;
						tempTrack.setTrackMainPattern (nullptr);

						if (!trackInfo.trackRxInstrumentName.empty () && (trackInfo.trackRxInstrumentName != " "))
						{
							Instrument* val = _PInstrumentMgr->getInstrument(trackInfo.trackRxInstrumentName);
							if (val != nullptr)
							{
								tempTrack.setTrackRxInstrument(val);
								val->UpdateUsedByList(false, songName, tempScene._sceneData->sceneName, trackInfo.trackName);
							}
						}
						if (!trackInfo.trackTxInstrumentName.empty() && (trackInfo.trackTxInstrumentName != " "))
						{
							Instrument* val = _PInstrumentMgr->getInstrument(trackInfo.trackTxInstrumentName);
							if (val != nullptr)
							{
								tempTrack.setTrackTxInstrument(val);
								val->UpdateUsedByList(false, songName, tempScene._sceneData->sceneName, trackInfo.trackName);
							}
						}
						if (!trackInfo.mainPatternName.empty () && (trackInfo.mainPatternName != " "))
						{
							Pattern* newPattern = _PPatternMgr->GetPattern(trackInfo.mainPatternName);
							if (newPattern != nullptr)
							{
								tempTrack.setTrackMainPattern(newPattern);
								newPattern->UpdateUsedByList(false, songName, tempScene._sceneData->sceneName, trackInfo.trackName);
							}
						}
						if (!trackInfo.fillPatternName.empty () && (trackInfo.fillPatternName != " "))
						{
							Pattern* newPattern = _PPatternMgr->GetPattern(trackInfo.fillPatternName);
							if (newPattern != nullptr)
							{
								tempTrack.setTrackFillPattern(newPattern);
								newPattern->UpdateUsedByList(true, songName, tempScene._sceneData->sceneName, trackInfo.trackName);
							}
						}
						// push filled tracks to Scene's trackList
						tempScene.addTrack (tempTrack);
					}
#if 0 //ENABLE_PARALLEL_EXE
					);
#endif
				// push filled scene to Song's sceneList
				activeSong->addScene (tempScene, false, false);
			}

#ifdef ENABLE_PARALLEL_EXE
		);
#endif
		sortScenesInSong(*activeSong.get ());
		reAssignSequentialIdsForScenes(*activeSong.get ());

#ifdef ENABLE_PARALLEL_EXE
			std::for_each(
				std::execution::par_unseq,
				activeSong->_songData->sceneList.begin(),
				activeSong->_songData->sceneList.end(),
				[this](auto&& sceneIt)
#else
				for (auto sceneIt : activeSong->_songData->sceneList)
#endif
				{
					activeSong->sortTracksInScene(sceneIt);
				}

#ifdef ENABLE_PARALLEL_EXE
		);
#endif
		activeSongSet = true;
	}
	SaveUsedbyLisrtDataForActiveSong();
	return activeSongSet;
}

void SongManager::setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)
{
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	//set value for each, this calls onParameterChange
	FindParameter((*it).param)->setValue(moduleSrc, (*it).MsgValue, isPlayMode);
}

void SongManager::SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode)
{
	//commands not added to Undo list, and no notification
	ParamHandler(moduleSrc, FindParameter(param), false);
}

void SongManager::onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode)
{
	if (!isPlayMode &&
		(moduleSrc != (moduleId_t)ModuleName::moduleUndo) &&
		((SongParam)param->GetParamID() != SongParam::paramActiveSong) &&
		((SongParam)param->GetParamID() != SongParam::paramPlaySongCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramPauseSongCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramUndoCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramAddSongCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramDeleteSongCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramSaveAll) &&
		((SongParam)param->GetParamID() != SongParam::paramSaveSongCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramUndoListCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramClearUndoList) &&
		((SongParam)param->GetParamID() != SongParam::paramStopSongCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramStartRecordCmd) &&
		((SongParam)param->GetParamID() != SongParam::paramStopRecordCmd) &&
		//((SongParam)param->GetParamID() != SongParam::paramFillMode) &&
		((SongParam)param->GetParamID() != SongParam::paramFillModeSwitchOn) &&
		((SongParam)param->GetParamID() != SongParam::paramFillModeType) &&
		((SongParam)param->GetParamID() != SongParam::paramFillModeRelease) &&
		((SongParam)param->GetParamID() != SongParam::paramMoveSong) &&
		((SongParam)param->GetParamID() != SongParam::ParamTestNumCreateSong) &&
		((SongParam)param->GetParamID() != SongParam::ParamTestSetNumScene) &&
		((SongParam)param->GetParamID() != SongParam::ParamTestSetNumTrack) &&
		((SongParam)param->GetParamID() != SongParam::ParamTestSetNumPattern) &&
		((SongParam)param->GetParamID() != SongParam::ParamTestSetNumPatternBars) &&
		((SongParam)param->GetParamID() != SongParam::ParamTestSetNumInstrument) &&
		((SongParam)param->GetParamID() != SongParam::ParamDeleteAllSong) &&
		((SongParam)param->GetParamID() != SongParam::ParamDeleteAllSongInstPattern) &&
		((SongParam)param->GetParamID() != SongParam::ParamEngineReady) &&
		((SongParam)param->GetParamID() != SongParam::ParamEngineCreateDataBackup) &&
		((SongParam)param->GetParamID() != SongParam::ParamEngineRestoreDataBackup) &&
		((SongParam)param->GetParamID() != SongParam::paramAuditioningState)
		)
	{
		if ((SongParam)param->GetParamID() == SongParam::paramSongName)
		{
			if (SongManager::getName() != (std::string)(param->getValue().get<std::string>().c_str()))
			{
				AddToUndoList(param); // TODO correct it later, UI sending song name along with active song
			}
		}
		else
		{
			AddToUndoList(param);
		}
	}
	ParamHandler(moduleSrc, param, true);
}

void SongManager::ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification)
{
	if (isUndilistNeedUpdate && (activeSong != nullptr))
	{
		isUndilistNeedUpdate = false;
		SetDefaultUndoListData();
	}

	if (moduleSrc != (moduleId_t)ModuleName::moduleSong)
	{
		switch ((SongParam)param->GetParamID())
		{
			case SongParam::paramUndoCmd:
				doParamUndo();
				break;

			case SongParam::paramClearUndoList:
			{
				//Clear undo list is called when user switches from one song to another.
				//If user has not saved the song, before switching to new song
				//Set song name back to original name (should match with FS name)
				//Get original song name from undo list
				//Set that name in songInfoList
				//Subsequently if the same song is accessed at a later point of
				//time, song will be accessed from FS, with correct name.
				std::vector <SongParam> tmpParamList;
				//only name is reset to default(saved)
				tmpParamList.push_back (SongParam::paramSongName);
				resetToDefaultParamValues (tmpParamList);

				releaseSongResources (activeSongId, false);
				ClearUndoList ();
				break;
			}

			case SongParam::paramSongName:
				SongManager::setName((std::string)(param->getValue().get<std::string>().c_str()));
				break;

			case SongParam::paramSongSwing:
				SongManager::setSwing(param->getValue().get<uint8_t>());
				break;

			case SongParam::paramSongTempo:
				SongManager::setTempo(param->getValue().get<float>());
				break;

			case SongParam::ParamTestNumCreateSong:
			{
				uint8_t val = param->getValue ().get<uint8_t> ();
				if (val > 0)
					createSongWithRandomData (val, TestCreateNumScene, TestCreateNumTrack, TestCreateNumInst, TestCreateNumPattern, TestCreateNumPatternBars);
			}
				break;

			case SongParam::ParamTestSetNumScene:
					TestCreateNumScene = param->getValue ().get<uint8_t> ();
				break;

			case SongParam::ParamTestSetNumTrack:
					TestCreateNumTrack = param->getValue ().get<uint8_t> ();
				break;

			case SongParam::ParamTestSetNumPattern:
					TestCreateNumPattern = param->getValue ().get<uint8_t> ();
				break;

			case SongParam::ParamTestSetNumPatternBars:
					TestCreateNumPatternBars = param->getValue ().get<uint8_t> ();
				break;

			case SongParam::ParamTestSetNumInstrument:
					TestCreateNumInst = param->getValue ().get<uint8_t> ();
				break;

			case SongParam::ParamDeleteAllSong:
				clearAllSongs ();
				break;

			case SongParam::ParamDeleteAllSongInstPattern:
				clearAllSongsInstrumentPattern ();
				break;

			case SongParam::paramAddSongCmd:
					SongManager::createAndAddSong ();
				break;

			case SongParam::paramActiveSong:
				SongManager::setActiveSong (param->getValue().get<int32_t>());
				break;

			case SongParam::paramDeleteSongCmd:
				SongManager::removeSong();
				break;

			case SongParam::paramPlaySongCmd:
			{
				setSystemState(KEY_PRESSED::PLAY_KEY);

				auto sceneId = param->getValue ().get<uint8_t> ();
				if ((sceneId >= 0) && (sceneId < activeSong->_songData->numberOfScenes))
				{
					SongManager::playSong (sceneId);
				}
				else
				{
					SongManager::playSong (0);
				}

				/* Send the play command if MMC send is Enabled*/
				if (_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcSendStatus)
				{
					MidiMessage message;
					sendMidiExternal (message.midiMachineControlCommand (message.mmc_play), (uint8_t)_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcOutputPort);
				}
				break;
			}

			case SongParam::paramPauseSongCmd:
				SongManager::pauseSong();
				setSystemState(KEY_PRESSED::PAUSE_KEY);

				/* Send the pause command if MMC send is Enabled*/
				if (_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcSendStatus)
				{
					MidiMessage message;
					sendMidiExternal (message.midiMachineControlCommand (message.mmc_pause), (uint8_t)_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcOutputPort);
				}
				break;

            case SongParam::paramSaveAll:
                // Scene/track edits need persistence even when song parameters are clean.
                if (saveSong()) ClearUndoList();
                break;
			case SongParam::paramSaveSongCmd:
				SaveData();
				break;

			case SongParam::paramStopSongCmd:
			{
				/* Send the stop command if MMC send is Enabled*/
				if (_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcSendStatus)
				{
					MidiMessage message;
					sendMidiExternal (message.midiMachineControlCommand (message.mmc_stop), (uint8_t)_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcOutputPort);
				}

				std::vector<IgniteParamList> params;
				IgniteParamList temp = { (paramId_t)SettingsGeneralCnfgParam::SettingDeviceCnfgSafeStopParam };
				params.push_back (temp);

				params = engine.settingsGeneralCnfg->getParamList ((moduleId_t)(ModuleName::moduleSong), params);
				int8_t safeStopStatus = params[0].MsgValue.get<int8_t> ();

				if (safeStopStatus == 1)
				{
					//start timer when first stop is received
					if (_stopCount == 0)
					{
						_stopCount = 1;
						std::chrono::milliseconds duration (1000);//user should press 2nd stop, within 1 second
						std::thread timerFunctionThread (timerFunc, std::ref (_stopCount), duration);//wait for second stop
						timerFunctionThread.detach ();//no resources allocated, detach from caller thread and allow it to return
						return;
					}
					else if (_stopCount == 1)
					{
						//second stop received within 1 second
						_stopCount = 0;//reset
						SongManager::stopSong ();
						setSystemState(KEY_PRESSED::STOP_KEY);
					}
					else
					{
						assert (0);
					}
				}
				else
				{
					//no safe stop - stop the play with first stop
					SongManager::stopSong ();
					setSystemState(KEY_PRESSED::STOP_KEY);
				}

				break;
			}

			case SongParam::paramReStartSongCmd:
			{
				int32_t sceneId = getCurrentPlaySceneId ();

				//Press once restart scene
				//Press twice within 1 second, restart at previous scene
				//Press thrice within 1 second, restart song
				//start timer when first stop is received
				if (_reStartCount == 0)
				{
					_reStartCount = 1;
					std::chrono::milliseconds duration (1000);//user to press 2nd and 3rd time, within 1 second from first press
					std::thread reStarttimerThread (reStartTimerFunc, this, std::ref(_reStartCount), sceneId, duration);//wait for second stop
					reStarttimerThread.detach ();//no resources allocated, detach from caller thread and allow it to return
				}
				else
				{
					++_reStartCount;
				}
				break;
			}

			case SongParam::paramStartRecordCmd:
				setSystemState(KEY_PRESSED::RECORD_KEY);
				SongManager::initRecord();
				SongManager::setRecordTrigger(true);

				/* Send the record start command if MMC send is Enabled*/
				if (_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcSendStatus)
				{
					MidiMessage message;
					sendMidiExternal (message.midiMachineControlCommand (message.mmc_recordStart), (uint8_t)_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcOutputPort);
				}
				break;

			case SongParam::paramStopRecordCmd:
				SongManager::stopRecord();
				setSystemState(KEY_PRESSED::RECORD_STOP_KEY);
				SongManager::setRecordTrigger(false);
				/* Send the record stop command if MMC send is Enabled*/
				if (_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcSendStatus)
				{
					MidiMessage message;
					sendMidiExternal (message.midiMachineControlCommand (message.mmc_recordStop), (uint8_t)_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcOutputPort);
				}
				break;
				/*
							case SongParam::paramFillMode:
								SongManager::setFillMode(param->getValue().get<bool>());
								break;
								*/
			case SongParam::paramMoveSong:
				SongManager::moveSong(activeSong->_songData->songId, param->getValue().get<int32_t>());
				break;

			case SongParam::paramCopySong:
				SongManager::copySong(param->getValue().get<std::string>());
				break;

			case SongParam::paramRecordEventFilter:
				SongManager::setRecordEventFilter(param->getValue().get<uint16_t>());
				break;
			case SongParam::paramTrackCountCmd:
				SongManager::modifyTrackCount(param->getValue().get<int8_t>());
				break;

			case SongParam::paramTrackSoloMode:
				//SongManager::setTrackSoloMode(param->getValue().get<bool>()); moved to scene settings
				break;

			case SongParam::paramFillModeSwitchOn:
				SongManager::setFillModeSwitchOnState(param->getValue().get<bool>());
				break;

			case SongParam::paramFillModeType:
				SongManager::setFillModeType(param->getValue().get<int8_t>());
				break;

			case SongParam::paramFillModeRelease:
				SongManager::setFillModeRelease(param->getValue().get<int8_t>());
				break;

			case SongParam::paramApplySongTempoToScenes:
				SongManager::setGlobalTempo(param->getValue().get<bool>());
				break;

			case SongParam::paramApplySongSwingToScenes:
				SongManager::setGlobalSwing(param->getValue().get<bool>());
				break;

			case SongParam::ParamEngineCreateDataBackup:
				SongManager::GetEngineDataBackup(param->getValue().get<int8_t>());
				break;
			case SongParam::ParamEngineRestoreDataBackup:
				SongManager::RestoreEngineDataBackup(param->getValue().get<int8_t>());
				break;

			case SongParam::paramSyncIn:
				SongManager::handleSyncIn(param->getValue().get<float>());
				break;

			case SongParam::paramAuditioningState:
				SongManager::setAuditioningState(param->getValue().get<bool>());
				break;

			default:
				//TODO
				assert(0);
				break;
		}
	}
	if (engine.getAutoSaveStatus() || engine.settingsSongCnfg->_SettingsSongCnfgData.SongAutoSaveParam)
	{
		SaveData();
	}
	if (SendNotification)
	{
		/* Send notification to the subscribed modules via engine */
		paramlistener->onParameterChanged(moduleSrc, param);
	}
}
void SongManager::SaveData()
{
	if (GetUndoParamsInList().empty())
		return;
	if (SongManager::saveSong()) ClearUndoList();
}
// Save the selected song ID and master volume.
bool SongManager::saveSongData(void)
{
	mainInfoData data;
	engine.fileHandler->getMainInfo(data);
	data.activeSongId = activeSongId;
	data.masterVolume = _masterVolume;
	return engine.fileHandler->updateMainInfo(data)==FILE_OK;
}
bool SongManager::saveSong(void)
{
    auto stateLock = engine.lockDesktopState();
    if (activeSong == nullptr) return false;
    // Publish the replacement only after the entire new song has been written.
    if (engine.get_fileHandler()->CreateSong(*activeSong->_songData) != FILE_OK) return false;
    if (activeSong->songNameDirtyFlag)
    {
        const auto oldName = GetDefaultDataFromList((paramId_t)SongParam::paramSongName).get<std::string>();
        if (oldName != activeSong->getName()) engine.get_fileHandler()->DeleteSong(oldName);
    }
    if(!saveSongData()) return false;
    engine.desktopSaved("song");
    activeSong->songNameDirtyFlag = false;
    engine.sceneManager->markSaved();
    SetDefaultUndoListData();
    SaveUsedbyLisrtDataForActiveSong();
    return true;
}

bool SongManager::saveSong(CSong& song)
{
    auto stateLock = engine.lockDesktopState();
    if (engine.get_fileHandler()->CreateSong(*song._songData) != FILE_OK) return false;
    SetDefaultUndoListData();
    return true;
}

void SongManager::saveSongInfo (SongData& songInfo)
{
	//filesystem save
	engine.get_fileHandler()->UpdateSongInfo(songInfo);

	SetDefaultUndoListData();
}

void SongManager::saveSongInfo (int32 id, std::string songName)
{
	//get data from FS
	auto songData = engine.get_fileHandler ()->GetSongInfo(songName);

	//update new id
	songData.songId = id;

	//filesystem save
	engine.get_fileHandler ()->UpdateSongInfo (songData);

	SetDefaultUndoListData ();
}

void SongManager::SetDefaultUndoListData()
{
	std::vector<IgniteParamList> paramlist;
	std::vector<IgniteParamList> retParam;
	IgniteParamList paramlistdata = {};

	ClearUndoDefaultList();

	for (auto i = (paramId_t)SongParam::paramall + 1; i < (paramId_t)SongParam::paramSongMax; ++i)
	{
		if ((i == (paramId_t)SongParam::paramSongNameList) ||
			(i == (paramId_t)SongParam::ParamEngineReady) ||
			(i == (paramId_t)SongParam::ParamEngineCreateDataBackup) ||
			(i == (paramId_t)SongParam::ParamEngineRestoreDataBackup) ||
			(i == (paramId_t)SongParam::paramUndoListCmd) ||
			//(i == (paramId_t)SongParam::paramFillMode)   ||
			(i == (paramId_t)SongParam::paramCopySong) ||
			(i == (paramId_t)SongParam::paramMoveSong) ||
			(i == (paramId_t)SongParam::paramFillModeSwitchOn) ||
			(i == (paramId_t)SongParam::paramFillModeType) ||
			(i == (paramId_t)SongParam::paramFillModeRelease) || 
			(i == (paramId_t)SongParam::paramUsbHostMidiChangeNotification) ||
			(i == (paramId_t)SongParam::paramUsbHostMidiDeviceNameList)
			)
		{
			//ignore command params, used in get param getParamList
		}
		else
		{
			AddToUndoDefaultList ( (paramId_t)i, FindParameter ( (paramId_t)i)->getValue());
			paramlistdata.param = (paramId_t)i;
			paramlist.push_back (paramlistdata);
		}
	}

	//TODO Add a comment for below section
	retParam = getParamList((moduleId_t)ModuleName::moduleSong, paramlist);

	for (std::vector<IgniteParamList>::iterator it = retParam.begin(); it != retParam.end() && isUndilistNeedUpdate == false; ++it)
	{
		AddToUndoDefaultList(it->param, it->MsgValue);
	}
}

/** Create a new song
 * New song created with a default name (checked for duplicates)
 * Auto saved in FS
 */
void SongManager::createAndAddSong(void)
{
	float tempo = DEFAULT_SONG_TEMPO; //set some default value
	uint8_t swing = DEFAULT_SONG_SWING;//set some default value
	uint8_t numscene = 0;//TODO
	std::string name;

	name = "SONG-" + std::to_string(numberOfSongs + 1);

	modifySongNameIfDuplicate(name);

	CSong tempSong(numberOfSongs, name, tempo, swing, numscene, this/*, false*/);

	addSong(tempSong, false, false);

	engine.get_fileHandler()->DeleteSong(name);
	engine.get_fileHandler()->CreateSong(*tempSong._songData);

	++numberOfSongs;
}

/** Request to select a new song as the active song.
 * New song data (if it exists) would be read from filesystem
 * activeSOng pointer would point to this (new) song.
 * All subsequent song releated operations, would be applied to the active song
 */
void SongManager::setActiveSong (int32_t index)
{
	if ((index < 0) || (index >= numberOfSongs))
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN index out of range " + String (index));
		return;
	}

	if (index == activeSongId)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "INFO Active song already set to " + String (index));
		return;
		//Active song is already set to the requsted value
	}

	if (activeSong)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "INFO Switching to song :" + String (index) + ": " + String (getSongNameFromId (index)) + " From song :" + String (activeSong->getId ()) + ": " + activeSong->getName ());
		//if current active song is in play/paused state and new song is selected
		handleRunningSongOnSwitch ();
	}

	activeSong.reset ();
	//handleMidiOnSongExit ();

	//check in filesystem and set as active song
	//internally sets activeSong
	populateSongFromFileSystem (index); //engine song id

	if (activeSong)
	{
		activeSongId = index;
		savedActiveSongId = index;


		//Do this before first play.
		//This is a good place, since we have control at each song change
		this->engine.sequenceManager->initSequence();
#if defined(__linux__) && defined(TIMER_TEST_PLAY)
		for(auto& scene : activeSong->_songData->sceneList)
		{
			for(auto& track : scene._sceneData->trackList)
			{
				track.timerClear();
			}
		}
#endif
	}
	else // song not found in filesystem
	{
		activeSongId = -1;
	}

	mainInfoData data;
	engine.fileHandler->getMainInfo(data);
	data.activeSongId = activeSongId;
	data.masterVolume = _masterVolume;
	engine.fileHandler->updateMainInfo(data);

	if (activeSong != nullptr)
	{
		SetDefaultUndoListData();
		ClearUndoList();
	}

}
//Function to save updated used by data to FS; this to avoid multiple R/W to FS
void SongManager::SaveUsedbyLisrtDataForActiveSong()
{
	std::list <Pattern*> PatternList;
	std::list <Instrument*> InstrumentList;
	std::list <Instrument*> InstrumentOutList;
	if (activeSong)
	{
		for (int i = 0; i < activeSong->getNumOfScenes(); i++)
		{
			Scene* scene = getSceneWithId(i);
			std::string sceneName = scene->getName();
			auto& trackList = scene->_sceneData->trackList;
			for (auto& track : trackList)
			{
				PatternList.push_back(track.getTrackMainPattern());
				PatternList.push_back(track.getTrackFillPattern());
				InstrumentList.push_back(track.getTrackRxInstrument());
				InstrumentList.push_back(track.getTrackTxInstrument());
			}
		}
		PatternList.unique();
		InstrumentList.unique();
		InstrumentOutList.unique();
		for (auto it : PatternList)
		{
			if(it != NULL)
				it->savePatternUsedByList();
		}
		for (auto it : InstrumentList)
		{
			if (it != NULL)
				it->saveInstrumentUsedByList();
		}
		for (auto it : InstrumentOutList)
		{
			if (it != NULL)
				it->saveInstrumentUsedByList();
		}
	}
}
void SongManager::releaseSongResources (int32_t songId, bool isDelete)
{
	ResetTrackUsedByList (songId, isDelete);
}
void SongManager::ResetTrackUsedByList (int32_t songId, bool isDelete)
{
#if defined DISABLE_USED_BY_LIST
	std::string songName = getSongNameFromId (songId);
	// Clear all current instrument and Pattern
	if (activeSong)
	{
		int32_t i = 0;
#ifdef ENABLE_PARALLEL_EXE
		//parallel_for (i, (activeSong->getNumOfScenes () - 1), [this, songName](int n)
		for (i = 0; i < activeSong->getNumOfScenes (); i++)
		{
			Scene* scene = getSceneWithId (i);
			std::string sceneName = scene->getName ();
			auto& trackList = scene->_sceneData->trackList;
			std::for_each (
				std::execution::par_unseq,
				trackList.begin (),
				trackList.end (),
				[this, songName, sceneName, isDelete](auto&& track)
			{
				track.ResetUsedbyInfo (songName, sceneName, isDelete);
			});
		}
#else
		for (i = 0; i < activeSong->getNumOfScenes (); i++)
		{
			Scene* scene = getSceneWithId (i);
			std::string sceneName = scene->getName ();
			auto& trackList = scene->_sceneData->trackList;
			for (auto& track : trackList)
			{
				track.ResetUsedbyInfo (songName, sceneName, true);
			}
		}
#endif
		engine.trackManager->saveTrack ();
	}
#endif//DISABLE_USED_BY_LIST
}
/*
void SongManager::setFillMode(bool mode)
{
	if (activeSong == nullptr)
	{
		return;
	}

	activeSong->setFillMode(mode);
}
*/
void SongManager::setName(std::string name)
{
	if (activeSong == nullptr)
	{
		return;
	}

	activeSong->setName(name);
	activeSong->updateSongInfoList ();
}

void SongManager::setTempo(float val)
{
	auto stateLock = engine.lockDesktopState();
	if (activeSong == nullptr)
	{
		return;
	}

	activeSong->setTempo(val);
	//engine.setParam(moduleId_t::Storage, (paramId_t)Store::SongProperties, activeSongId);
}

void SongManager::setSwing(uint8_t val)
{
	if (activeSong == nullptr)
	{
		return;
	}

	activeSong->setSwing(val);
	//engine.setParam(moduleId_t::Storage, (paramId_t)Store::SongProperties, activeSongId);
}

std::vector<IgniteParamList> SongManager::getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)
{
	std::vector<IgniteParamList> retParam = {};
	IgniteParamList paramData;

	//loop to fill all data and return the value
	for (std::vector<IgniteParamList>::iterator it = paramlist.begin(); it != paramlist.end(); ++it)
	{
		switch ((SongParam)(*it).param)
		{
			case SongParam::ParamEngineReady:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = engine.isIDeviceReady();
				retParam.push_back(tempParam);
				break;
			}
			case SongParam::paramSongName: //return active song's swing value
			{
				//read value from active song
				std::string val = SongManager::getName();

				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}

			case SongParam::paramSongNameList:// return list of names from filesystem
			{
				int32_t count = (*it).Count;
				int32_t startSongPosition = (*it).startPos;
				std::vector<std::string> savedSongNameList;

				// Get name list from engines list
				savedSongNameList = getSongNameListSorted(startSongPosition, count);

				// Populate the return vector
				int32_t index = 0;
				struct IgniteParamList tempParam;
				for (auto nameIter = savedSongNameList.begin(); nameIter != savedSongNameList.end(); ++nameIter)
				{
					tempParam.param = (*it).param;
					tempParam.MsgValue = (*nameIter);
					tempParam.startPos = index;
					retParam.push_back(tempParam);
					++index;
				}
				break;
			}

			case SongParam::paramSongSwing: //return active song's swing value
			{
				//read value from active song
				uint8_t val = SongManager::getSwing();

				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}

			case SongParam::paramSongTempo: //return active song's tempo value
			{
				float val = SongManager::getTempo();

				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}

			case SongParam::paramUndoListCmd:
			{
				uint8_t Count = 1;
				paramData = { 0 };
				std::vector<paramId_t> undoList = GetUndoParamsInList();

				paramData.param = (paramId_t)SongParam::paramUndoListCmd;
				paramData.Count = (uint32_t)undoList.size(); //set the number of params
				retParam.push_back(paramData);

				for (auto itr = undoList.begin(); itr < undoList.end(); ++itr)
				{
					paramData.param = *itr;
					paramData.Count = Count;
					Count++;
					retParam.push_back(paramData);
				}
				break;
			}
			/*
						case SongParam::paramFillMode:
						{
							bool mode = SongManager::getFillMode();
							struct IgniteParamList tempParam;
							tempParam.param = (*it).param;
							tempParam.MsgValue = mode;
							retParam.push_back(tempParam);
							break;
						}
			*/
			case SongParam::paramRecordEventFilter:
			{
				auto val = SongManager::getRecordEventFilter();

				//fill return structure
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = val;
				retParam.push_back(tempParam);
				break;
			}
			case SongParam::paramTrackCountCmd://get the track count for current edit scene
				if (activeSong != nullptr)
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;//copy from get request
					tempParam.startPos = 0;//not used
					if (!activeSong->_songData->sceneList.empty())
					{
						tempParam.MsgValue = activeSong->_songData->sceneList.begin()->getNumOfTracks();
					}
					else
						tempParam.MsgValue = activeSong->_songData->gTrackCount;

					retParam.push_back(tempParam);
				}
				else
				{
					DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN Active song not set");
				}
				break;
#if 0 //moved to scene settings
			case SongParam::paramTrackSoloMode:
			{
				bool mode = SongManager::getTrackSoloMode();
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = mode;
				retParam.push_back(tempParam);
				break;
			}
#endif
			case SongParam::paramFillModeSwitchOn:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getFillModeSwitchOnState();
				retParam.push_back(tempParam);
			}
			break;

			case SongParam::paramFillModeType:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getFillModeType();
				retParam.push_back(tempParam);
				break;
			}

			case SongParam::paramFillModeRelease:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getFillModeRelease();
				retParam.push_back(tempParam);
				break;
			}

			case SongParam::paramApplySongTempoToScenes:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getGlobalTempoFlag();
				retParam.push_back(tempParam);
				break;
			}

			case SongParam::paramApplySongSwingToScenes:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getGlobalSwingFlag();
				retParam.push_back(tempParam);
				break;
			}

			case SongParam::paramActiveSong:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getSavedActiveSongId ();
				retParam.push_back (tempParam);
				break;
			}

			case SongParam::paramFtsNameList:// return list of names from filesystem
			{
				std::vector<std::string> savedFtsNameList;

				int8_t scaleIndex = (int8_t)ScaleList::ScaleAllNotes;

				if (ftsList.size() < (int8_t)ScaleList::ScaleMax)
				{
					// Get name list from engines list
					for (auto& scale : ftsList)
					{
						(void)scale;
						if (ftsList[(ScaleList)scaleIndex].scaleIntervalList.size() > 0)
						{
							savedFtsNameList.push_back(ftsList[(ScaleList)scaleIndex].name);
						}
						else
						{
							//break; //In Linux it giving 0 for 1st
						}
						scaleIndex++;
					}
				}

				// Populate the return vector
				int32_t index = 0;
				struct IgniteParamList tempParam;
				for (auto nameIter = savedFtsNameList.begin (); nameIter != savedFtsNameList.end (); ++nameIter)
				{
					tempParam.param = (*it).param;
					tempParam.MsgValue = (*nameIter);
					tempParam.startPos = index;
					retParam.push_back (tempParam);
					++index;
				}
				break;
			}

			case SongParam::paramActiveTrackTxInstrumentAvailability:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = getActiveTrackTxInstrumentAvailability ();
				retParam.push_back (tempParam);
			}
			break;

			case SongParam::paramAuditioningState:
			{
				struct IgniteParamList tempParam;
				tempParam.param = (*it).param;
				tempParam.MsgValue = _isAuditioningOn;
				retParam.push_back(tempParam);
			}
			break;

			case SongParam::paramUsbHostMidiDeviceNameList://return list of usb Host Midi Devices
			{
				std::map<int8_t, std::string> nameListWithIndex;

				nameListWithIndex = getUsbHostPortMidiDeviceNames();
			
				// Populate the return vector
				struct IgniteParamList tempParam;
				for (auto &nameIter : nameListWithIndex)
				{
					tempParam.param = (*it).param;
					tempParam.MsgValue = nameIter.second;
					tempParam.startPos = nameIter.first;
					retParam.push_back(tempParam);
				}
				break;
			}
			case SongParam::paramSystemState: //return system state
				{
					struct IgniteParamList tempParam;
					tempParam.param = (*it).param;
					tempParam.MsgValue = (int)getSystemState();
					retParam.push_back(tempParam);
					break;
				}

			default:
				//assert(0);
				break;

		}
	}
	return retParam;
}

void SongManager::getParam(moduleId_t moduleSrc, paramId_t param)
{
	switch ((SongParam)param)
	{
		case SongParam::ParamEngineReady:
		{
			engine.SendResponse((moduleId_t)ModuleName::moduleSong, moduleSrc, param, engine.isIDeviceReady());
			break;
		}
		case SongParam::paramSongName:
		{
			std::string name = SongManager::getName();
			engine.SendResponse((moduleId_t)ModuleName::moduleSong, moduleSrc, param, name);
			break;
		}

		case SongParam::paramSongSwing:
		{
			uint8_t val = SongManager::getSwing();
			engine.SendResponse((moduleId_t)ModuleName::moduleSong, moduleSrc, param, val);
			break;
		}

		case SongParam::paramSongTempo:
		{
			float val = SongManager::getTempo();
			engine.SendResponse((moduleId_t)ModuleName::moduleSong, moduleSrc, param, val);
			break;
		}
		/*
		case SongParam::paramFillMode:
		{
			bool mode = SongManager::getFillMode();
			engine.SendResponse((moduleId_t)ModuleName::moduleSong, moduleSrc, param, mode);
			break;
		}
		*/
		default:
			//TODO WARN
			break;
	}
}

CSong* SongManager::getActiveSong(void)
{
	return activeSong.get ();
}

float SongManager::getTempo(void)
{
	if (activeSong == nullptr)
	{
		return -1;
	}

	return activeSong->getTempo();
}

int8_t SongManager::getSwing(void)
{
	if (activeSong == nullptr)
	{
		return -1;
	}

	return activeSong->getSwing();
}
/*
bool SongManager::getFillMode(void)
{
	if (activeSong == nullptr)
	{
		//TODO warn
		return false;
	}

	return activeSong->getFillMode();
}
*/

std::string SongManager::getName(void)
{
	if (activeSong == nullptr)
	{
		//TODO warn
		return "invalid";
	}

	return activeSong->getName();
}

int32_t SongManager::getSongCount(void) const
{
	return numberOfSongs;
}

void SongManager::clearAllSongs()
{
	ClearUndoList ();
	songInfoList.clear ();
	activeSongId = -1;
	activeSong = nullptr;
	numberOfSongs = 0;
	engine.get_fileHandler ()->DeleteAllSongs ();
}
void SongManager::clearAllSongsInstrumentPattern ()
{
	clearAllSongs ();
	engine.patternManager->clearAllPattern ();
	engine.instrumentManager->clearAllInstrument ();
}

//Will delete the current active song
void SongManager::removeSong(void)//id starts from 0
{
	if (activeSong == nullptr)
	{
		return;
	}

	// de-init song play settings - if removing a paused song
	// play song removal is restricted from UI - not checked here
	if (_isSongPaused == true)
	{
		deInitSongPlaySettings();
	}
	releaseSongResources(activeSongId, true);
	activeSongId = -1;

	// Delete in filesystem:
	// Current active song's name may have been changed by user, but not yet saved to FS
	// FS entry would be having the last-saved name, so delete FS entry using last-saved name
	if (activeSong->songNameDirtyFlag == true)
	{
		std::string DefaultSongName = (std::string)(GetDefaultDataFromList ((paramId_t)SongParam::paramSongName).get<std::string> ().c_str ());
		if (DefaultSongName != activeSong->getName ())
		{
			engine.get_fileHandler ()->DeleteSong (DefaultSongName);
		}
		//user did undo[s] to reach back to the last-saved song name
		else
		{
			engine.get_fileHandler ()->DeleteSong (activeSong->getName ());
		}
	}
	else
	{
		engine.get_fileHandler ()->DeleteSong (activeSong->getName ());
	}

	//delete in engine's songInfoList
	auto id = activeSong->getId();
	auto songInfo = songInfoList.find(id);
	if (songInfo != songInfoList.end ())
	{
		songInfoList.erase (id);
		--numberOfSongs;
		//Set sequential ids for remaing songs and Save
		reAssignSequentialIdsForSongsAndSave ();
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN song not found in info list");
	}
}

void SongManager::playSong(int32_t sceneId)
{
	auto stateLock = engine.lockDesktopState();
	if (activeSong == nullptr)
	{
		//TODO warn
		return;
	}

	// check if already playing
	if (_isSongPlaying == false)
	{
#ifdef __linux__
		//startPulseProcessingTimers();
#endif

#ifdef USE_JUCE_AUDIO
		if (_isSongPaused == true)
		{
			setUpPlatformAudio();
		}
		//Start JUCE call backs
		startJuceAudioCallback();


#else /* USE_JUCE_AUDIO */

	#ifdef _WIN32
		#ifdef USE_INTERNAL_20MS_TIMER
		startWindowsAudioDirect();
		#endif
	#endif /* _WIN32 */

	#ifdef __linux__

	#endif /* __linux__ */

#endif /* USE_JUCE_AUDIO */

		if (_isSongPaused == false)
		{
			initPlayScene(sceneId);
			sendMidiClk();

			// Send MIDI things to connected instrument @song start
			sendTrackValuesAtSongLevel();
		}

		// This section is used by resume from pause and fresh start
		CMidiClock::getInstance().start();
		//_timeStamp = std::chrono::high_resolution_clock::now();
		_isSongPlaying = true;
		_isSongPaused = false;
#ifdef USE_INTERNAL_20MS_TIMER
		start20msAudioProcessingTimer();
#endif

#ifdef LOG_PCM_TO_FILE
		openLogFiles();
#endif /* LOG_PCM_TO_FILE */

	}
}

void SongManager::initPlayScene(int32_t sceneId)
{
	auto stateLock = engine.lockDesktopState();
	uint64_t muteStateOfTracksToUse = 0xFFFFFFFFFFFFFFFF;
	bool muteHoldInEffect = false;

	if (activeSong == nullptr)
	{
		//TODO WARN
		return;
	}


	//get mute hold settings from completed scene, before switching to next scene
	auto prevScene = activeSong->getPlayScene();
	if (prevScene != nullptr)
	{
		auto prevSceneMuteHoldType = prevScene->getSceneMuteHold();

		if ((prevSceneMuteHoldType == MuteHoldType::Next) || (prevSceneMuteHoldType == MuteHoldType::Lock))
		{
			auto _muteHoldSceneId = activeSong->getPlaySceneId();

			//Keep information outside scene level for multi-scene mute hold (LOCK)
			setGlobalMuteHold(prevSceneMuteHoldType, _muteHoldSceneId);

			muteStateOfTracksToUse = getSceneAllTrackMuteStatus(_muteHoldSceneId);
			muteHoldInEffect = true;
		}
		//prev scene does NOT have mute hold, but some other scene has set MuteHoldType::lock
		//find that scene and use its track mute states
		else if (getGlobalMuteHoldType() == MuteHoldType::Lock)
		{
			// check if the last scene to set global LOCK, is still in LOCK state
			auto lastSceneToSetMuteHoldLock = getSceneWithId(_gMuteHoldLockedSceneId);
			if ((lastSceneToSetMuteHoldLock != nullptr) && (lastSceneToSetMuteHoldLock->getSceneMuteHold() == MuteHoldType::Lock))
			{
				muteStateOfTracksToUse = getSceneAllTrackMuteStatus(_gMuteHoldLockedSceneId);
				muteHoldInEffect = true;
			}
			// last scene to set global mute hold LOCK is now OFF
			// check if any other scene has mute hold LOCK an use it
			else
			{
				auto firstSceneToSetMuteHoldLock = findTheFirstSceneWithMuteHoldLock();
				if (firstSceneToSetMuteHoldLock != nullptr)
				{
					auto scene_id = firstSceneToSetMuteHoldLock->getId();
					muteStateOfTracksToUse = getSceneAllTrackMuteStatus(scene_id);
					setGlobalMuteHold(MuteHoldType::Lock, scene_id);//update new sceneId
					muteHoldInEffect = true;
				}
				//no scene has Mute hold LOCK
				//so clear global state
				else
				{
					setGlobalMuteHold(MuteHoldType::Off, -1);
					muteHoldInEffect = false;
				}
			}
		}
		//prev scene does NOT have mute hold, but the scene before that has set MuteHoldType::next
		else if (getGlobalMuteHoldType() == MuteHoldType::Next)
		{
			// MuteHoldType::next is only valid for immediate next scene, clear the mute hold after that.
			setGlobalMuteHold(MuteHoldType::Off, -1);
			muteHoldInEffect = false;
		}
	}
	//set next scene as current play scene
	bool sceneSet = activeSong->setPlayScene(sceneId);

	//proceed to track setting if scene is set
	if (sceneSet == true)
	{
		//sendTrackValuesAtSceneLevel();
		publishSceneInfoToUi();
		activeSong->playScene->gBarId = 0;
		activeSong->playScene->gStepId = 0;
		publishSceneBarInfoToUi();//call only after setting Scene gBarId/gStepId
		CMidiClock::getInstance().setTempo(activeSong->playScene->getTempo());

		int8_t trackId = 0;

		for (auto& trackIter : activeSong->playScene->_sceneData->trackList)
		{
			trackIter.initTrackPlaySetting();

			if (muteHoldInEffect == true)
			{
				trackIter.setTrackMuteStatusInMuteHoldMode((muteStateOfTracksToUse >> trackId) & 0x1 ? true : false);
				++trackId;
			}
			else //no mute hold for the next scene, reset mute hold mode mute status to regular mute status
			{
				trackIter.setTrackMuteStatusInMuteHoldMode(trackIter.getTrackMuteStatus());
			}
		}
		UpdateplaySceneTrackPortMap();
		setSceneTimeBaseTick();
	}
}

void SongManager::pauseSong()
{
	auto stateLock = engine.lockDesktopState();
	if (_isSongPlaying == true)
	{
#ifdef __linux__
		//stopPulseProcessingThreads();
#endif

#ifdef USE_JUCE_AUDIO
		stopJuceAudioCallback();

#else /* USE_JUCE_AUDIO */

	#ifdef USE_INTERNAL_20MS_TIMER
		#ifdef _WIN32
		stopWindowsAudioDirect();
		#endif /* _WIN32 */

		#ifdef __linux__

		#endif /* __linux__ */

	#endif /* USE_INTERNAL_20MS_TIMER */

#endif /* USE_JUCE_AUDIO */

		CMidiClock::getInstance().pause();
		_isSongPlaying = false;
		_isSongPaused = true;

#ifdef USE_INTERNAL_20MS_TIMER
		stopAudioProcessingTimer();
#endif

#ifdef LOG_PCM_TO_FILE
		closeLogFiles();
#endif
	}
}

void SongManager::stopSong()
{
	auto stateLock = engine.lockDesktopState();
	if ((_isSongPlaying == true) || (_isSongPaused == true))
	{

#ifdef __linux__
		//stopPulseProcessingThreads();
#endif

#ifdef USE_JUCE_AUDIO

		stopJuceAudioCallback();

#else /* USE_JUCE_AUDIO */

	#ifdef USE_INTERNAL_20MS_TIMER

		#ifdef _WIN32
		stopWindowsAudioDirect();
		#endif /* _WIN32 */

		#ifdef __linux__

		#endif /* __linux__ */

	#endif /* USE_INTERNAL_20MS_TIMER */ 


#endif /* USE_JUCE_AUDIO */

		CMidiClock::getInstance().stop();
		_isSongPaused = false;
		_isSongPlaying = false;

		_scenePulsesPerStep = 0;


		// stop play config at scene & track level of active song
		deInitSongPlaySettings();
		activeSong->playScene = nullptr;

#ifdef USE_INTERNAL_20MS_TIMER
		stopAudioProcessingTimer();
#endif

#ifdef LOG_PCM_TO_FILE
		closeLogFiles();
#endif

	}
}

bool SongManager::applyFtsAndTranspose(uint8_t& note, bool isTransposeDefeat, bool isFtsDefeat)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr))
	{
		//TODO warn
		return false;
	}

	if (activeSong->playScene->getTransposeFTSPriority() == (int8_t)TransposeFTSPriority::TransposePostFTS)
	{
		if (isFtsDefeat == false)
		{
			note = scaleNote(note);
		}
		if (isTransposeDefeat == false)
		{
			note = transposeNote(note);
		}
	}
	else if (activeSong->playScene->getTransposeFTSPriority() == (int8_t)TransposeFTSPriority::TransposePreFTS)
	{
		if (isTransposeDefeat == false)
		{
			note = transposeNote(note);
		}
		if (isFtsDefeat == false)
		{
			note = scaleNote(note);
		}
	}

	return true;
}

Scene* SongManager::getPlayScene(void)
{
	Scene* ret = nullptr;

	if (activeSong != nullptr)
	{
		ret = activeSong->playScene;
	}

	return ret;
}

uint8_t SongManager::getPlaySceneNumTracks(void)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr))
	{
		//TODO warn
		return 0;
	}

	//uint8_t sceneId = activeSong->getPlaySceneId();
	uint8_t numOfTracks = activeSong->playScene->getNumOfTracks();

	return numOfTracks;
}

//Scene change will happen at bar boundary
//This function is called every time a bar finishes.
void SongManager::advancePlayScene(void)
{
    if(desktopOffline)return;
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr))
	{
		//TODO warn
		return;
	}

	bool sceneLoopEnabled = ((activeSong->getEndSceneId() >= 0) && (activeSong->getStartSceneId() >= 0));

	int32_t currentSceneId = activeSong->playSceneId;

	if (activeSong->playScene->_sceneData->mode == (uint8_t)SceneAdvanceMode::SceneAdvanceManual)
	{
		//Check if scene change is required
		if ((manualSwitchToNextScene == true) || (manualJumpToNewScene == true))
		{
			//we are moving to a new scene
			// reset back to start bar, for next scene
			activeSong->playScene->gBarId = 0;
			activeSong->playScene->gStepId = 0;
			_sceneChangedFlag = true;

			//set next scene id
			int32_t nextSceneId = activeSong->playSceneId + 1;
			if (manualJumpToNewScene == true)
			{
				nextSceneId = activeSong->jumpSceneId;
			}

			// check for end
			// once it reaches the end, repeat last scene indefinetely
			if (nextSceneId > (activeSong->getNumOfScenes() - 1)) // scene id starts from 0
			{
				nextSceneId = activeSong->getNumOfScenes() - 1;
			}

			// stay within loop, provided user did not jump to a specific new scene
			if ((manualJumpToNewScene == false) && (sceneLoopEnabled == true))
			{
				if ((nextSceneId > activeSong->getEndSceneId()) || (nextSceneId < activeSong->getStartSceneId()))
				{
					// restart the loop from begining
					nextSceneId = activeSong->getStartSceneId();
				}
			}

			//TODO check if really required. Pattern should only play for scene length, restart pattern
			//If same scene is repeated, let pattern continue, with control on gbar
			if (nextSceneId != currentSceneId)
			{
				clearLastPlayedScene (currentSceneId);
				initPlayScene (nextSceneId);
			}

			//clear flag
			if (manualSwitchToNextScene == true) manualSwitchToNextScene = false;
			if (manualJumpToNewScene == true) manualJumpToNewScene = false;
		}
		else // no manual change in scene, continue in current scene
		{
			// check if scene is completed
			if (activeSong->playScene->gBarId >= activeSong->playScene->getBarCount())
			{
				// reset back to start bar, for next scene
				activeSong->playScene->gBarId = 0;
				activeSong->playScene->gStepId = 0;
				_sceneChangedFlag = true;
			}
		}
	}
	else if (activeSong->playScene->_sceneData->mode == (uint8_t)SceneAdvanceMode::SceneAdvanceAuto)
	{
		//If manual next or jump cmd recievd wait till end of bar, to switch
		if ((manualSwitchToNextScene == true) || (manualJumpToNewScene == true))
		{
			//reset back to start bar, as we are moving to a new scene
			activeSong->playScene->gBarId = 0;
			activeSong->playScene->gStepId = 0;
			_sceneChangedFlag = true;

			//set next scene id
			int32_t nextSceneId = activeSong->playSceneId + 1;
			if (manualJumpToNewScene == true)
			{
				nextSceneId = activeSong->jumpSceneId;

			}

			// check for end
			// once it reaches the end, repeat last scene indefinetely
			if (nextSceneId > (activeSong->getNumOfScenes() - 1)) // scene id starts from 0
			{
				nextSceneId = activeSong->getNumOfScenes() - 1;
			}

			// stay within loop, provided user did not jump to a specific new scene
			if ((manualJumpToNewScene == false) && (sceneLoopEnabled == true))
			{
				if ((nextSceneId > activeSong->getEndSceneId()) || (nextSceneId < activeSong->getStartSceneId()))
				{
					// restart the loop from begining
					nextSceneId = activeSong->getStartSceneId();
				}
			}

			// TODO check if really required. Pattern should only play for scene length, restart pattern
			//If same scene is repeated, let pattern continue, with control on gbar
			if (nextSceneId != currentSceneId)
			{
				// move to next scene
				clearLastPlayedScene (currentSceneId);
				initPlayScene (nextSceneId);
			}


			//clear flag
			if (manualSwitchToNextScene == true) manualSwitchToNextScene = false;
			if (manualJumpToNewScene == true) manualJumpToNewScene = false;
		}
		else //no manual switch
		{
			// check if scene is completed
			if (activeSong->playScene->gBarId >= activeSong->playScene->getBarCount())
			{
				// reset back to start bar
				activeSong->playScene->gBarId = 0;
				activeSong->playScene->gStepId = 0;
				_sceneChangedFlag = true;

				int32_t nextSceneId = activeSong->playSceneId + 1;

				// check for end
				// once it reaches the end, repeat last scene indefinetely
				if (nextSceneId > (activeSong->getNumOfScenes() - 1)) // scene id starts from 0
				{
					nextSceneId = activeSong->getNumOfScenes() - 1;
				}

				if (sceneLoopEnabled == true)
				{
					if ((nextSceneId > activeSong->getEndSceneId()) || (nextSceneId < activeSong->getStartSceneId()))
					{
						// restart the loop from begining
						nextSceneId = activeSong->getStartSceneId();
					}
				}

				//TODO check if really required. Pattern should only play for scene length, restart pattern
				//If same scene is repeated, let pattern continue, with control on gbar
				if (nextSceneId != currentSceneId)
				{
					// move to next scene
					clearLastPlayedScene (currentSceneId);
					initPlayScene (nextSceneId);
				}
			}
		}
	}
}

/*
bool SongManager::getSongTrackMuteStatus(void)
{
	if (activeSong == nullptr)
	{
		//TODO warn
		return true;
	}

	//uint8_t sceneId = activeSong->getPlaySceneId();
	bool muteStatus = activeSong->playScene->playTrack->getTrackMuteStatus();

	return muteStatus;
}
*/

//TODO: remove below as we using JUCE MIDI
//void SongManager::transmitCb(uint8_t* data, uint8_t length, MidiType type)
//{
////	if (midiOut.isPortOpen())
//	{
//		if (type == MidiType::SystemExclusive)
//		{
//	//		midiOut.sendMessage(data, length);
//		}
//		else
//		{
////			midiOut.sendMessage(&data[1], length - 1);//midi transport layer adds one byte at start
//		}
//	}
//}

std::map<int,std::vector<int>> SongManager::desktopScaleIntervals() const
{
    std::map<int,std::vector<int>> result;
    for(const auto& item:ftsList)result[int(item.first)]={item.second.scaleIntervalList.begin(),item.second.scaleIntervalList.end()};
    return result;
}
void SongManager::setDesktopScaleIntervals(const std::map<int,std::vector<int>>& values)
{
    for(const auto& item:values)if(item.first>=int(ScaleList::ScaleMin) && item.first<int(ScaleList::ScaleMax))
        ftsList[ScaleList(item.first)].scaleIntervalList={item.second.begin(),item.second.end()};
}

std::vector<int8_t> SongManager::generateScaleNotes(int8_t rootNote, int8_t scale)
{

	//Default to C, if out of range
	if ((rootNote < (int8_t)FTSRootNote::NoteMin) || (rootNote > (int8_t)FTSRootNote::NoteMax))
	{
		rootNote = (int8_t)FTSRootNote::NoteC;
	}

	//Default to no FTS, if out of range
	if ((scale <= (int8_t)ScaleList::ScaleMin) || (scale >= (int8_t)ScaleList::ScaleMax))
	{
		scale = (int8_t)ScaleList::ScaleAllNotes;
	}

	std::vector<int8_t> scaleNoteList;
	std::vector<int8_t>::iterator noteIntervalStartIter;
	std::vector<int8_t>::iterator noteIntervalEndIter;

	noteIntervalStartIter = ftsList[(ScaleList)scale].scaleIntervalList.begin();
	noteIntervalEndIter = ftsList[(ScaleList)scale].scaleIntervalList.end();

	scaleNoteList.clear();
	uint8_t noteCount = 0;
	for (auto& noteIntervalIter = noteIntervalStartIter; noteIntervalIter != noteIntervalEndIter; ++noteIntervalIter)
	{
		int8_t offset = 0;
		// ensure offsets are within octave range
		if (*noteIntervalIter > (int8_t)FTSRootNote::NoteMax)
		{
			offset = *noteIntervalIter % NOTES_PER_OCTAVE;
		}
		else if (*noteIntervalIter < (int8_t)FTSRootNote::NoteMin)
		{
			offset = 0;
		}
		else //within range
		{
			offset = *noteIntervalIter;
		}

		int8_t note = 0;
		if (noteCount == 0) //first note = root note
		{
			note = rootNote;
			scaleNoteList.push_back(note);
			++noteCount;
		}

		if (noteCount > 0) // remaing notes
		{
			note = scaleNoteList[noteCount - 1] + offset;
			//note range from C -> B, check if it has crossed B and restart at C
			if (note > (int8_t)FTSRootNote::NoteMax)
			{
				note = (int8_t)FTSRootNote::NoteMin + (note % NOTES_PER_OCTAVE);
			}
			scaleNoteList.push_back(note);
			++noteCount;
		}
	}

	return scaleNoteList;
}

int8_t SongManager::scaleNote(int8_t note)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr))
	{
		assert(0);
		return -1;
	}

	// If FTS is not enabled, don't modify note
	if ((activeSong->playScene->getFTSRootNote() == -1) || (activeSong->playScene->getFTSScale() == -1))
	{
		return note;
	}

	std::vector<int8_t>scale = generateScaleNotes(activeSong->playScene->getFTSRootNote(), activeSong->playScene->getFTSScale());

	int8_t offset = 0;
	int8_t normalisedNote = note % NOTES_PER_OCTAVE;

	std::vector<int8_t>::iterator noteLocation = std::find(scale.begin(), scale.end(), (int8_t)normalisedNote);

	//calculate offset, if note not part of scale
	if (noteLocation == scale.end())
	{
		int16_t i = 0;
		int8_t checkNote = normalisedNote;
		// check all other notes in both direction, till we find a match
		// higher freq has priority
		for (i = 1; i < NOTES_PER_OCTAVE; ++i)
		{
			//if pattern note + offset crossed max, then nearest note is downward, don't search upward
			if ((int16_t)(note + i) <= (int16_t)Note::noteMax)
			{
				//check upward dircetion (C -> B)
				checkNote = normalisedNote + (int8_t)i;
				//crossed B, so continue at C
				if (checkNote > (int8_t)FTSRootNote::NoteMax)
				{
					checkNote = (normalisedNote + i) % NOTES_PER_OCTAVE;
				}
				noteLocation = std::find(scale.begin(), scale.end(), (int8_t)checkNote);
				if (noteLocation != scale.end())
				{
					offset = (int8_t)i;
					break;
				}
			}

			//if pattern note - offset crossed min, then nearset note is upward, don't search downward
			if ((int8_t)(note - i) >= (int8_t)Note::noteMin)
			{
				//check downward direction(C <- B)
				checkNote = normalisedNote - (int8_t)i;
				//crossed C, so continue at B
				if (checkNote < (int8_t)FTSRootNote::NoteMin)
				{
					checkNote = (normalisedNote - i + NOTES_PER_OCTAVE) % NOTES_PER_OCTAVE;
				}
				noteLocation = std::find(scale.begin(), scale.end(), (int8_t)checkNote);
				if (noteLocation != scale.end())
				{
					offset = (int8_t)(-i);
					break;
				}
			}
		}
	}

	int16_t scaledNote = note + offset;

	//check for overflow beyond g10
	if (scaledNote > (int8_t)Note::noteMax)
	{
		scaledNote = (int8_t)Note::noteMax;
	}


	if (scaledNote < (int8_t)Note::noteMin)
	{
		scaledNote = (int8_t)Note::noteMin;
	}
	return scaledNote;
}

int8_t SongManager::transposeNote(int8_t note)
{
	int8_t transposeval = activeSong->playScene->getTransposeValue();

	int16_t transposedNote = note + transposeval;

	//check for overflow beyond g10
	if (transposedNote > (int8_t)Note::noteMax)
	{
		transposedNote = (int8_t)Note::noteMax;
	}
	//check for overflow beyond c0
	if (transposedNote < (int8_t)Note::noteMin)
	{
		transposedNote = (int8_t)Note::noteMin;
	}

	return (int8_t)transposedNote;
}

//at every scene change
void SongManager::publishSceneInfoToUi(void)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr) || (activeSong->playScene->_sceneData == nullptr))
	{
		//TODO WARN
		return;
	}

	//current play scene id
	engine.SendResponse((moduleId_t)ModuleName::moduleScene, (moduleId_t)ModuleName::moduleUI, (paramId_t)SceneParam::paramCurrentPlaySceneId, activeSong->playScene->getId());

	//current play scene name
	engine.SendResponse((moduleId_t)ModuleName::moduleScene, (moduleId_t)ModuleName::moduleUI, (paramId_t)SceneParam::paramCurrentPlaySceneName, activeSong->playScene->getName());

	//currrent play scene unmuted tracks
	auto trackMute = activeSong->playScene->_sceneData->trackMuteStatus;
	int unmutedTrackCount = 0;
	for (int i = 0; i < activeSong->playScene->getNumOfTracks(); ++i)
	{
		if (((trackMute >> i) & 0x1) == 0)
		{
			++unmutedTrackCount;
		}
	}
	engine.SendResponse((moduleId_t)ModuleName::moduleScene, (moduleId_t)ModuleName::moduleUI, (paramId_t)SceneParam::paramCurrentPlaySceneUnmutedTracks, unmutedTrackCount);

	//NEXT PLAY SCENE
	bool nextSceneFound = false;
	int32_t id = identifyNextPlayScene();
	auto start = activeSong->_songData->sceneList.begin();
	auto end = activeSong->_songData->sceneList.end();
	auto i = start;

	//locate the scene in our scene list
	if (id >= activeSong->getNumOfScenes())
	{
		nextSceneFound = false;
	}
	else
	{
		for (i = start; i != end; ++i)
		{
			if ((*i).getId() == id)
			{
				nextSceneFound = true;
				break;
			}
		}

		if (i == end)
		{
			nextSceneFound = false;
		}
	}

	int32_t nextSceneId = -1;
	std::string nextScenename = "invalid";
	int8_t nextSceneUnmutedTracks = 0;

	//if found in list - get the details from the scene
	if (nextSceneFound == true)
	{
		nextSceneId = (*i).getId();
		nextScenename = (*i).getName();

		trackMute = (*i)._sceneData->trackMuteStatus;

		for (int x = 0; x < (*i).getNumOfTracks(); ++x)
		{
			if (((trackMute >> x) & 0x1) == 0)
			{
				++nextSceneUnmutedTracks;
			}
		}
	}

	engine.SendResponse((moduleId_t)ModuleName::moduleScene, (moduleId_t)ModuleName::moduleUI, (paramId_t)SceneParam::paramNextPlaySceneId, nextSceneId);
	//next play scene name
	engine.SendResponse((moduleId_t)ModuleName::moduleScene, (moduleId_t)ModuleName::moduleUI, (paramId_t)SceneParam::paramNextPlaySceneName, nextScenename);
	//next play scene unmuted tracks
	engine.SendResponse((moduleId_t)ModuleName::moduleScene, (moduleId_t)ModuleName::moduleUI, (paramId_t)SceneParam::paramNextPlaySceneUnmutedTracks, nextSceneUnmutedTracks);
}

//at every bar transition
void SongManager::publishSceneBarInfoToUi(void)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr) || (activeSong->playScene->_sceneData == nullptr))
	{
		//TODO WARN
		return;
	}

	int16_t barId = getCurrentPlayingGBarId();
	engine.SendResponse((moduleId_t)ModuleName::moduleScene, (moduleId_t)ModuleName::moduleUI, (paramId_t)SceneParam::paramSceneBarId, barId);
}

int32_t SongManager::identifyNextPlayScene(void)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr) || (activeSong->playScene->_sceneData == nullptr))
	{
		//TODO warn
		return -1;
	}

	int32_t nextSceneId = -1;

	bool sceneLoopEnabled = ((activeSong->getStartSceneId() >= 0) && (activeSong->getEndSceneId() >= 0));

	if (activeSong->playScene->_sceneData->mode == (uint8_t)SceneAdvanceMode::SceneAdvanceManual)
	{
		//Check if scene change is required
		if ((manualSwitchToNextScene == true) || (manualJumpToNewScene == true))
		{
			//set next scene id
			nextSceneId = activeSong->playSceneId + 1;
			if (manualJumpToNewScene == true)
			{
				nextSceneId = activeSong->jumpSceneId;
			}

			if (nextSceneId > (activeSong->getNumOfScenes() - 1))
			{
				nextSceneId = 0;
			}

			// stay within loop, provided user did not jump to a specific new scene
			if ((manualJumpToNewScene == false) && (sceneLoopEnabled == true))
			{
				if ((nextSceneId > activeSong->getEndSceneId()) || (nextSceneId < activeSong->getStartSceneId()))
				{
					// restart the loop from begining
					nextSceneId = activeSong->getStartSceneId();
				}
			}
		}
		else // no manual change in scene, continue in current scene
		{
			nextSceneId = activeSong->playSceneId;
		}
	}
	else if (activeSong->playScene->_sceneData->mode == (uint8_t)SceneAdvanceMode::SceneAdvanceAuto)
	{
		if ((manualSwitchToNextScene == true) || (manualJumpToNewScene == true))
		{
			//set next scene id
			nextSceneId = activeSong->playSceneId + 1;
			if (manualJumpToNewScene == true)
			{
				nextSceneId = activeSong->jumpSceneId;
			}

			if (nextSceneId > (activeSong->getNumOfScenes() - 1))
			{
				nextSceneId = 0;
			}

			// stay within loop, provided user did not jump to a specific new scene
			if ((manualJumpToNewScene == false) && (sceneLoopEnabled == true))
			{
				if ((nextSceneId > activeSong->getEndSceneId()) || (nextSceneId < activeSong->getStartSceneId()))
				{
					// restart the loop from begining
					nextSceneId = activeSong->getStartSceneId();
				}
			}
		}
		else //no manual switch
		{
			nextSceneId = activeSong->playSceneId + 1;

			if (nextSceneId > (activeSong->getNumOfScenes() - 1))
			{
				nextSceneId = 0;
			}

			if (sceneLoopEnabled == true)
			{
				if ((nextSceneId > activeSong->getEndSceneId()) || (nextSceneId < activeSong->getStartSceneId()))
				{
					// restart the loop from begining
					nextSceneId = activeSong->getStartSceneId();
				}
			}
		}
	}
	return nextSceneId;
}

void SongManager::installDesktopSong(std::unique_ptr<CSong> song)
{
    const int id=numberOfSongs;
    song->setId(id);song->setEditScene(0);song->editScene->setEditTrack(0);
    song->setStartSceneId(0);song->setEndSceneId(song->getNumOfScenes()-1);
    songInfoList.emplace(id,song->getName());++numberOfSongs;
    activeSong=std::move(song);activeSongId=id;savedActiveSongId=id;
    ClearUndoList();SetDefaultUndoListData();
    engine.sceneManager->markSaved();engine.trackManager->resetDesktopHistory();
    engine.sequenceManager->initSequence();
    engine.desktopEdited("song");
}

void SongManager::addSong(CSong& newSong, bool sortScenes, bool reAssignSceneIds)
{
	if (sortScenes == true)
	{
		//sort scenes within song as per their id
		sortScenesInSong(newSong);
	}

	if (reAssignSceneIds == true)
	{
		//re-assign sequential ids to all scenes in the list
		reAssignSequentialIdsForScenes(newSong);
	}

	//add in engine's songInfoList
	//this is called from create and add song, so the new song info will be added at the end
	songInfoList.emplace (newSong.getId (), newSong.getName());

	//check the config - where to insert the new song
	//std::vector<IgniteParamList> parameterListToRead;
	//std::vector<IgniteParamList> parameterListReturned;
	//IgniteParamList paramToRead;
	//paramToRead.param = (paramId_t)SettingsSongCnfgParam::settingInsertPosition;
	//parameterListToRead.push_back(paramToRead);
	//parameterListReturned = engine.get_settingsSongCnfg()->getParamList((moduleId_t)ModuleName::moduleSong, parameterListToRead);

	//auto newPosition = InsertPosition::listEnd;//default add at end

	//if (parameterListReturned.size() > 0)
	//{
	//	newPosition = static_cast<InsertPosition> (parameterListReturned[0].MsgValue.get<int8_t>());
	//}

	//if (newPosition == InsertPosition::nextToActiveRow)
	//{
	//	moveSong(newSong.getId(), activeSong->getId() + 1);
	//}
	//else //if (newPosition == atTheEnd) // default to end
	//{
	//	//nothing to do
	//}
}

struct sortSceneCompareFunction
{
	bool operator()(Scene& first, Scene& second)
	{
		return (first.getId() < second.getId());
	}
};

void SongManager::sortScenesInSong(CSong& song)
{
	song._songData->sceneList.sort(sortSceneCompareFunction());
}

void SongManager::reAssignSequentialIdsForScenes(CSong& song)
{
	auto sceneStart = song._songData->sceneList.begin();
	auto sceneEnd = song._songData->sceneList.end();

	//Re-assign ids starting from 0
	int32_t count = 0;
	for (auto& sceneIter = sceneStart; sceneIter != sceneEnd; ++sceneIter)
	{
		if ((*sceneIter).getId() != count)
		{
			(*sceneIter).setId(count);
		}
		++count;
	}
}
/*
void SongManager::reAssignSequentialIdsForScenesAndSave(CSong& song)
{
	auto sceneStart = song._songData->sceneList.begin();
	auto sceneEnd = song._songData->sceneList.end();

	//Re-assign ids starting from 0
	int32_t count = 0;
	for (auto sceneIter = sceneStart; sceneIter != sceneEnd; ++sceneIter)
	{
		if ((*sceneIter).getId() != count)
		{
			(*sceneIter).setId(count);
			//save modifed scenes to filesystem
			saveScene(song, *sceneIter);
		}
		++count;
	}
}
*/


//TODO check timings for this function with diffrent(10, 20, 30) songs list sizes
void SongManager::reAssignSequentialIdsForSongsAndSave (void)
{
	//Re-assign ids starting from 0
	int32_t count = 0;
	auto tmpSongInfoList = songInfoList;
	songInfoList.clear ();

	for (auto songIter : tmpSongInfoList)
	{
		songInfoList.emplace (count, songIter.second);

		//save modified song id to filesystem
		saveSongInfo (count, songIter.second);

		++count;
	}
}

/*
* Re-assigns sequential ids
* Returns the new id (after re-assigning ids), of one (monitored) song
*/
int32_t SongManager::reAssignSequentialIdsForSongsAndSave (int32_t monitorIndex, bool isDirectionUp)
{
	bool monitorIndexMapped = false;
	//Re-assign ids starting from 0
	int32_t count = 0, newIndex =  -1;
	auto tmpSongInfoList = songInfoList;
	songInfoList.clear();
	uint8_t monitorIndexCount = 0;

	for (auto songIter : tmpSongInfoList)
	{
		if ( (monitorIndexMapped == false) && (monitorIndex == songIter.first))
		{
			++monitorIndexCount;
			//if movement direction is up, moved song will be the first item at monitor index
			//else it would be the second item at monitor index
			if ( (isDirectionUp == true) || (monitorIndexCount == 2))
			{
				monitorIndexMapped = true;
				newIndex = count;
			}
		}
		songInfoList.emplace (count, songIter.second);

		//save modified songInfo to filesystem
		saveSongInfo (count, songIter.second);

		++count;
	}

	return newIndex;
}

void SongManager::modifySongNameIfDuplicate(std::string& name)
{
	auto start = songInfoList.begin();
	auto end = songInfoList.end();

	for (auto& songIter = start; songIter != end; ++songIter)
	{
		if (songIter->second == name)
		{
			std::string tagString;
#if 0
			time_t now = time(0);
			struct tm  tstruct;
			char timeStampStr[50] = { 0 };
			tstruct = *localtime(&now);
			strftime(timeStampStr, sizeof(timeStampStr), "_%H%M%S_%d%h%Y", &tstruct);
			tagString = timeStampStr;
#else
			srand((uint32_t)time(NULL));
			int16_t randomTag = rand() % 9999;
			tagString = "_" + std::to_string(randomTag);
#endif
			name = name + tagString;
		}
	}
}
void SongManager::pulse(uint16_t pulse)
{
	auto stateLock = engine.lockDesktopState();
	if (_pulseHandlingInProgress.load() == true)
	{
		return;//Not completed prev req
	}
	_pulseHandlingInProgress.store(true);

	if ((activeSong == nullptr) || (activeSong->playScene == nullptr))
	{
		_pulseHandlingInProgress.store(false);
		return;
	}

	if (_scenePulsesPerStep < 1)
	{
		_pulseHandlingInProgress.store(false);
		return;
	}

	uint16_t _scenePulse = pulse % _scenePulsesPerStep;

	//Check for step boundary
	if (_scenePulse >= (_scenePulsesPerStep - 1))
	{
		//increment gbar step
		++(activeSong->playScene->gStepId);

		///Check for Bar change
		//gBarLenInSteps 1 - 16
		//gstepId 0 - 15
		if (activeSong->playScene->gStepId >= activeSong->playScene->getStepsPerBar())
		{
			++(activeSong->playScene->gBarId);
			activeSong->playScene->gStepId = 0;

			if(activeSong->playScene->gBarId>=activeSong->playScene->getBarCount() || manualSwitchToNextScene || manualJumpToNewScene)
				activeSong->playScene->stopPlayingNotes();
			advancePlayScene();//check and advance

			publishSceneBarInfoToUi();
			//sendMidiClk();
		}
	}

	//int16_t sceneBarId = getCurrentPlayingGBarId();
	if (activeSong->getPlayScene() == nullptr)
	{
		_pulseHandlingInProgress.store(false);
		return;
	}

	auto& trackList = activeSong->getPlayScene()->_sceneData->trackList;
	int16_t sceneBarId = activeSong->getPlayScene()->gBarId;
	int16_t sceneStepId = activeSong->getPlayScene()->gStepId;
    if(desktopOffline)
    {
        for(auto& track:trackList)track.pulse(sceneBarId,sceneStepId);
        _pulseHandlingInProgress.store(false);return;
    }
#ifdef ENABLE_THREADS_TO_PLAY
	std::list<TrackAndPosition> listTohandle;
	int count = 0;

	for (auto& track : trackList)
	{
		TrackAndPosition data = { &track, sceneBarId, sceneStepId };
		listTohandle.push_back(data);

		if ((count + 1) % 8 == 0 || count + 1 == static_cast<int>(trackList.size()))
		{
			int queueIndex = count / 8; //each Q has 8 track to handle
			switch (queueIndex)
			{
			case 0: TrackPlayQ_0.put(listTohandle); break;
			case 1: TrackPlayQ_1.put(listTohandle); break;
			case 2: TrackPlayQ_2.put(listTohandle); break;
			case 3: TrackPlayQ_3.put(listTohandle); break;
			case 4: TrackPlayQ_4.put(listTohandle); break;
			case 5: TrackPlayQ_5.put(listTohandle); break;
			case 6: TrackPlayQ_6.put(listTohandle); break;
			case 7: TrackPlayQ_7.put(listTohandle); break;
			}
			listTohandle.clear();
		}
		count++;
	}

#else
	for (auto& track : trackList)
	{
		if ((activeSong != nullptr) && (activeSong->playScene != nullptr))
		{
			track.pulse(sceneBarId, sceneStepId);
		}
	}
#endif
	_pulseHandlingInProgress.store(false);
}

void SongManager::clearLastPlayedScene(int32_t SceneId)
{
	if (activeSong == nullptr)
	{
		//TODO WARN
		return;
	}

	for (auto& sceneIter : activeSong->_songData->sceneList)
	{
		if (sceneIter._sceneData->sceneId == (uint32_t)SceneId)
		{
			sceneIter.deInitScenePlaySettings();
		}
	}
}


void SongManager::deInitSongPlaySettings(void)
{
	for (auto& sceneIter : activeSong->_songData->sceneList)
	{
		sceneIter.deInitScenePlaySettings();
	}
}


void SongManager::sendTrackValuesAtSongLevel(void)
{
#ifdef TRACK_VALUE_ENABLE
	if (activeSong != nullptr)
	{
		for (auto& sceneIter : activeSong->_songData->sceneList)
		{
			for (auto& trackIter : sceneIter._sceneData->trackList)
			{
				if (trackIter._trackData->trackInstrument != nullptr)
				{
					for (auto& tvIter : trackIter._trackData->trackInstrument->_instrumentData->tv)
					{
						//send only if scope is at song level and it is midi CC
						if ((tvIter.second.savedAt == TrackValueScope::TVAtSongLevel) && (tvIter.second.paramType == TrackValueType::TVTypeMidiCC))
						{
							//TODO SENING MIDI CC over JUCE
							//midiCCSend(tvIter.second.midiCCValue);
						}
					}
				}
			}
		}
	}
#endif
}

void SongManager::sendTrackValuesAtSceneLevel(void)
{
#ifdef TRACK_VALUE_ENABLE
	for (auto& trackIter : activeSong->playScene->_sceneData->trackList)
	{
		if (trackIter._trackData->trackInstrument != nullptr)
		{
			for (auto& tvIter : trackIter._trackData->trackInstrument->_instrumentData->tv)
			{
				//send only if scope is at scene level and it is midi CC
				if ((tvIter.second.savedAt == TrackValueScope::TVAtSceneLevel) && (tvIter.second.paramType == TrackValueType::TVTypeMidiCC))
				{
					//TODO SENING MIDI CC over JUCE
					//midiCCSend(tvIter.second.midiCCValue);
				}
			}
		}
	}
#endif
}

/**
* All USB MIDI and Virtual MIDI port messages will be handled here.
* DIN MIDI port messages will be first received at the MCU and then routed to IMX
*/
void IgniteMidi::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message)
{
	if (_PIgniteEngine != nullptr)		//Don't receive the midi message until IgniteEngine is loaded
	{
		//check if src message port is enabled for midi remote control
		if (_PSongMgr->midiRemoteControlEnabled (source->getName ()) == true)
		{
			_PSongMgr->handleMidiRemoteControlMessage (message);
		}
		else if (_PSongMgr->midiTransposeControlEnabled (source->getName ()) == true)
		{
			_PSongMgr->handleMidiTransposeControlMessage (message);
		}
		else if (_PSongMgr->midiMachineControlEnabled (source->getName ()) == true)
		{
			_PSongMgr->handleMidiMachineControlMessage (message);
		}
		else
		{
			_PSongMgr->handleInMidiMessage (message, source->getName ().toStdString ());
		}
	}
}


void SongManager::handleMcuSpecialKey(uint8_t KeyId, uint8_t value)
{
	auto stateLock = engine.lockDesktopState();
	switch ((paramId_t)KeyId)
	{
	case (paramId_t)FrontPannelParam::paramKey_INTERNALSOUNDS:
		isInternelKeyHeld = (value == 0 ? false : true);
		break;
	/*case (paramId_t)FrontPannelParam::paramKey_STOP: //TODO : Handle here may create issue as UI also handling, check and enable later: added to stop ASAP
		if (_state != SYSTEM_STATE::IDLE)
			engine.setParam((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramStopSongCmd, 0);
		break;
	case (paramId_t)FrontPannelParam::paramKey_PLAY:
		if ((_state == SYSTEM_STATE::PLAY) || (_state == SYSTEM_STATE::LIVE_RECORD))
			engine.setParam((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleSong, (paramId_t)SongParam::paramPauseSongCmd, 0);
		break;*/
	}
}

/*
 * Function handling MIDI messages recived at DIN ports (fowarded by MCU to IMX)
 */
void SongManager::handleIncomingMcuMidiMessage (String msgPortName, const MidiMessage& message)
{
	auto stateLock = engine.lockDesktopState();
	handleMidiThru(message, msgPortName.toStdString());
	//check if src message port is enabled for midi remote control
	if (message.isProgramChange() && midiRemoteControlEnabled (msgPortName) == true)
	{
		_PSongMgr->handleMidiRemoteControlMessage (message);
	}
	else if (midiTransposeControlEnabled (msgPortName) == true)
	{
		_PSongMgr->handleMidiTransposeControlMessage (message);
	}
	// Handle all messages
	_PSongMgr->handleInMidiMessage (message, msgPortName.toStdString ());
}

void SongManager::handleInMidiMessage(const MidiMessage& message, const std::string portName, bool isSendMidiThru)
{
	auto stateLock = engine.lockDesktopState();
	if(isSendMidiThru)
		handleMidiThru(message, portName);

	//DBG(message.getDescription());
	//Handle clock - in clock slave mode
	auto msgPort = getPortIdFromName(portName);
	if (message.isMidiClock() /* || message.isMidiStart() || message.isMidiContinue() || message.isMidiStop()*/)
	{
		//check if external clock from Midi ports (USB/DIN) is enabled
		if ( (engine.settingsMidiClkCnfg->isClkInFromDinMidiPort()) || (engine.settingsMidiClkCnfg->isClkInFromUsbMidiPort()))
		{
			auto clockSrc = engine.settingsMidiClkCnfg->getParamSingle(SettingsMidiClkCnfgParam::clkInPort);
			auto configPort = clockSrc.get<int8_t>();

			//check if it is enabled for message's port
			if (configPort == msgPort)
			{
				/*
				if (message.isMidiStart() || message.isMidiContinue())
				{

				}
				else if (message.isMidiStop())
				{

				}
				else //F8
				*/
				{
					//start thread to measure frequency - if not already started
					if (_midiClockCalculationStarted == false)
					{
						startMidiClockCalculation();
					}
					++_midiClockCounter;
				}
			}

		}
	}

	//Currently recording is set to start on first Note ON/Midi CC/ Pitch Bend/Program Change message
	//Change if condition below to set new start behaviour
	if ((_recordTrigger == true) && (_isRecording == false))
	{
		startRecord(message);
	}

	if (_isRecording == true)
	{
		//DBG (message.getDescription ());
		//DBG (message.getTimeStamp ());

		if (
			(_recordEventFilter & static_cast<uint16_t>(RecordEvents::Note)) && (message.isNoteOn ()) ||
			(_recordEventFilter & static_cast<uint16_t>(RecordEvents::Note)) && (message.isNoteOff()) ||
			(_recordEventFilter & static_cast<uint16_t>(RecordEvents::AfterTouchPP)) && (message.isAftertouch()) ||
			(_recordEventFilter & static_cast<uint16_t>(RecordEvents::AfterTouchCP)) && (message.isChannelPressure()) ||
			(_recordEventFilter & static_cast<uint16_t>(RecordEvents::ProgramChange)) && (message.isProgramChange()) ||
			(_recordEventFilter & static_cast<uint16_t>(RecordEvents::PitchBend)) && (message.isPitchWheel()) ||
			(_recordEventFilter & static_cast<uint16_t>(RecordEvents::MidiCC)) && (message.isController())
			)
		{
		if (_state == SYSTEM_STATE::STEP_RECORD)
		{
			if (activeSong->getEditScene() && activeSong->getEditScene()->editTrack &&
				activeSong->getEditScene()->editTrack->getRxInstrumentPort() == msgPort)
			{
				activeSong->getEditScene()->editTrack->stepRecordNote(message);
				patternRecMap[activeSong->getEditScene()->editTrack->getTrackMainPattern()->getName()] = activeSong->getEditScene()->editTrack->getTrackMainPattern();
			}
		}
		else if (_state == SYSTEM_STATE::LIVE_RECORD)
		{
			//get track list from current play scene - associated with this port
			auto trackIdList = playSceneTrackPortMap.equal_range(msgPort);
			for (auto it = trackIdList.first; it != trackIdList.second; ++it)
			{
				auto track = getPlayTrackFromId(it->second);
				if ((track != nullptr) && (track->_trackData->trackMute == false) && track->_trackData->trackRecordEnable)
				{
					track->liveRecord(message);
					patternRecMap[track->getTrackMainPattern()->getName()] = track->getTrackMainPattern();
				}
			}
		}
			//juce::Logger::outputDebugString ("INPUT :" + (std::to_string (tmp.statusByte)) + ":" + (std::to_string (tmp.dataByte1)));
		}
	}

#if 0
	//when not in record state
	else
	{
		if (message.isController ())
		{
			switch ((MidiCCMsgID)message.getControllerNumber ())
			{
				case MidiCCMsgID::SustainPedalOn_Off:
				{
					isSustainPedalOn = (message.getControllerValue () > 63); //Values of 0-63 indicate OFF ; else On
					if (!isSustainPedalOn && !PendingMidiMsgList.empty ())
					{
						SendPendingMidiMsg ();
					}
				}
				break;
			}
		}
	}
#endif
}

#if 0
void SongManager::SendPendingMidiMsg (void)
{
	for (auto& it : PendingMidiMsgList)
	{
		if (it.isInternalSound)
		{
			sendMidiMessageInternal (it.msg);
		}
		else
		{
			sendMidiExternal (it.msg, it.midiport);
		}
	}
	PendingMidiMsgList.clear ();
}
#endif

#if 0
void SongManager::handleMidiOnSongExit (void)
{
	if (!PendingMidiMsgList.empty ())
	{
		SendPendingMidiMsg ();
	}
}
#endif

void SongManager::initRecord(void)
{
	if (activeSong == nullptr)
	{
		//TODO warn
		return;
	}

	//Default record start to first scene of the song
	activeSong->setPlayScene(0);

	getRecordParams ();

	for (auto& track : activeSong->getPlayScene ()->_sceneData->trackList)
	{
		if (_state == SYSTEM_STATE::STEP_RECORD)
		{
				track.initStepRecord();
		}

		//Modify patterns associated with those tracks, which are not muted
		//This allows user to to filter out track which need not be part of record
		if (track._trackData->trackMute == false)
		{
			auto pattern = track.getTrackPattern();
			if (pattern != nullptr)
			{
				// If number of bars to record is more than current bar num in pattern , add bars
				if (_recordSetting.count ((paramId_t)SettingsRecordCnfgParam::SettingRecordNumOfBarsParam) != 0)
				{
					auto recordBars = _recordSetting.find ((paramId_t)SettingsRecordCnfgParam::SettingRecordNumOfBarsParam)->second;
					auto patternBars = pattern->getTotalBars ();
					if (patternBars != recordBars)
					{
						if (recordBars > patternBars)
						{
							pattern->addBars (recordBars - patternBars);
						}
					}
				}

				//if user has selected to overwrite the pattern, clear it before start
				if (_recordSetting.count((paramId_t)SettingsRecordCnfgParam::SettingRecordBarOverWriteParam) != 0)
				{
					auto overWrite = _recordSetting.find((paramId_t)SettingsRecordCnfgParam::SettingRecordBarOverWriteParam)->second;
					if (overWrite)
					{
						pattern->clearPattern();
					}
				}
			}
			else
			{
				DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN Unable to get Track's pattern");
			}
		}//track mute check
	}// task list loop

	//set record events filter at start of record
	if (_recordSetting.count((paramId_t)SettingsRecordCnfgParam::SettingRecordAfterTouchEventParam) != 0)
	{
		_recordEventFilter = (_recordSetting [(paramId_t)SettingsRecordCnfgParam::SettingRecordAfterTouchEventParam] == 0) ? (_recordEventFilter & ~ ( (uint16_t)RecordEvents::AfterTouchCP)) : _recordEventFilter | ( (uint16_t)RecordEvents::AfterTouchCP);
	}

	if (_recordSetting.count((paramId_t)SettingsRecordCnfgParam::SettingRecordMidiCCEventParam) != 0)
	{
		_recordEventFilter = (_recordSetting[(paramId_t)SettingsRecordCnfgParam::SettingRecordMidiCCEventParam] == 0) ? (_recordEventFilter & ~( (uint16_t)RecordEvents::MidiCC)) : (_recordEventFilter | ( (uint16_t)RecordEvents::MidiCC));
	}

	if (_recordSetting.count((paramId_t)SettingsRecordCnfgParam::SettingRecordNoteEventParam) != 0)
	{
		_recordEventFilter = (_recordSetting[(paramId_t)SettingsRecordCnfgParam::SettingRecordNoteEventParam] == 0) ? (_recordEventFilter & ~( (uint16_t)RecordEvents::Note)) : (_recordEventFilter | ( (uint16_t)RecordEvents::Note));
	}

	if (_recordSetting.count((paramId_t)SettingsRecordCnfgParam::SettingRecordPgmChangeEventParam) != 0)
	{
		_recordEventFilter = (_recordSetting[(paramId_t)SettingsRecordCnfgParam::SettingRecordPgmChangeEventParam] == 0) ? (_recordEventFilter & ~( (uint16_t)RecordEvents::ProgramChange)) : (_recordEventFilter | ( (uint16_t)RecordEvents::ProgramChange));
	}

	if (_recordSetting.count((paramId_t)SettingsRecordCnfgParam::SettingRecordPitchBendEventParam) != 0)
	{
		_recordEventFilter = (_recordSetting[(paramId_t)SettingsRecordCnfgParam::SettingRecordPitchBendEventParam] == 0) ? (_recordEventFilter & ~( (uint16_t)RecordEvents::PitchBend)) : (_recordEventFilter | ((uint16_t)RecordEvents::PitchBend));
	}

	if (_recordSetting.count((paramId_t)SettingsRecordCnfgParam::SettingRecordPolyPEventParam) != 0)
	{
		_recordEventFilter = (_recordSetting[(paramId_t)SettingsRecordCnfgParam::SettingRecordPolyPEventParam] == 0) ? (_recordEventFilter & ~((uint16_t)RecordEvents::AfterTouchPP)) : (_recordEventFilter | ((uint16_t)RecordEvents::AfterTouchPP));
	}
}

/* Start the record function
 * Internal clock - record start once we rx first midi packet to start record
 * Extrnal clock  - record start if we have external clock or get a transport start
 */

void SongManager::startRecord(const MidiMessage& message)
{
	if (_midiClkSlaveModeActive)
	{
		if (engine.settingsMidiClkCnfg->isClkInFromSyncPort())
		{
			//if external clock (SYNC IN ), start recording if clock is present (temp will be > 0)
			if (activeSong->getTempo() >= 20)
			{
				_isRecording = true;
			}
		}
		else
		{
			//if external clock (DIN MIDI/ USB MIDI), start recording at clock start or transport start
			if (message.isMidiClock() || message.isMidiStart())
			{
				_isRecording = true;
			}
		}
	}
	else
	{
		if (message.isNoteOn() || message.isController() || message.isProgramChange() || message.isPitchWheel())
		{
			//if internal clock, start recording at first midi packet
			_isRecording = true;
		}
	}
}

/* Stop the record function */
void SongManager::stopRecord(void)
{
    auto stateLock = engine.lockDesktopState();
	if (_isRecording == true)
	{
        if (_state == SYSTEM_STATE::LIVE_RECORD && activeSong && activeSong->playScene)
            for (auto& track : activeSong->playScene->_sceneData->trackList) track.recordStepEvents();
		_isRecording = false;
		_recordTrigger = false;

		for (auto& it : patternRecMap)
		{
			engine.patternManager->SavePattern(it.second, true);
		}
		patternRecMap.clear();
	}
}

// Called after each power cycle
void SongManager::populateSongInfoListFromFileSystem()
{
	songInfoList.clear();

	if (numberOfSongs <= 0)
	{
		return;
	}

	// Read all song names
	auto songNameList = engine.fileHandler->GetSongNameList(0, static_cast<uint16_t>(numberOfSongs));

	// Get info for all songs
	for (const auto& songName : songNameList)
	{
		SongData songInfo = engine.fileHandler->GetSongInfo(songName);

		// Check for duplicate IDs and assign a new unique ID if necessary
		while (songInfoList.find(songInfo.songId) != songInfoList.end())
		{
			++songInfo.songId; // Increment the ID until a unique one is found
		}

		songInfoList.insert({ songInfo.songId, songInfo.songName });
	}

	// Keep in order
	int32_t index = 0;
	for (auto it = songInfoList.begin(); it != songInfoList.end(); /* no increment here */)
	{
		if (it->first != index)
		{
			// Move it to its correct place if not present, so that UI index and song ID will be in sync
			SongData songInfo = engine.fileHandler->GetSongInfo(it->second);
			songInfo.songId = index;
			engine.fileHandler->UpdateSongInfo(songInfo);

			// Remove the old entry and insert the updated one
			it = songInfoList.erase(it); // Erase returns the next iterator
			songInfoList.insert({ index, songInfo.songName });
		}
		else
		{
			++it; // Only increment if no erase
		}
		++index;
	}
}


std::string SongManager::getSongNameFromId(int32_t id)
{
	std::string name = "invalid";

	auto songInfo = songInfoList.find (id);
	if (songInfo != songInfoList.end())
	{
		name = songInfo->second;
	}
	else
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR song does not exist");
	}

	return name;
}

std::vector<std::string> SongManager::getSongNameListSorted(int32_t start, int32_t count)
{
	std::vector<std::string> songNameListSorted;
	int32_t position = 0;
	int32_t readCount = 0;

	for (auto& song : songInfoList)
	{
		if (position >= start)
		{
			songNameListSorted.push_back(song.second);
			++readCount;
		}

		++position;

		if (readCount == count)
		{
			break;
		}
	}

	return songNameListSorted;
}
void SongManager::modifyTrackCount(int8_t newTrackCount)
{
	if (activeSong == nullptr)
	{
		//TODO WARN
		return;
	}
	else if (activeSong->editScene == nullptr)
	{
		activeSong->_songData->gTrackCount = newTrackCount;
		return;
	}
	activeSong->_songData->gTrackCount = newTrackCount;
	// Set track count for all scene
	auto sceneStart = activeSong->_songData->sceneList.begin();
	auto sceneEnd = activeSong->_songData->sceneList.end();
	for (auto& sceneIter = sceneStart; sceneIter != sceneEnd; ++sceneIter)
	{
		(*sceneIter).modifyTrackCount(newTrackCount, activeSong->_songData->songName);
	}
}

bool SongManager::moveSong(int32_t curId, int32_t newId)
{
	bool ret = false;
	if ((activeSong != nullptr) && (activeSong->getId() == curId))
	{
		ret = moveActiveSong (curId, newId);
	}
	else
	{
		ret = moveNonActiveSong (curId, newId);
	}
	return ret;
}

bool SongManager::moveActiveSong(int32_t curId, int32_t newId)
{
	// If a activeSong position is changed by user, check if song name has unsaved name changes
	// If name change is un-saved, files system entry would be having a different (last saved) name.
	// Auto save the current song (with new name & other data), before switching to a new song
	if ((activeSong) && (activeSong->getId () != newId) && (activeSong->songNameDirtyFlag == true))
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "INFO Changing active song ID to : " + String (newId) + " Autosaving song : " + String (activeSong->getId ()) + ": " + activeSong->getName ());
		saveSong ();
	}

	bool moveUp  = false;
	std::pair <int32_t, std::string> tmpCopyDst, tmpCopySrc;

	// Sanity check value from UI
	if (newId >= numberOfSongs)
	{
		newId = numberOfSongs - 1;
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN rcd index greater than max index, corrcetd to " + String (newId));
	}
	if (newId < 0)
	{
		newId = 0;
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN rcd index less than zero, corrected to 0");
	}

	if (newId == curId)
	{
		//nothing to do, already in position
		return true;
	}

	// find existing song at new dest id
	// copy it in tmp
	auto destRangeBounds = songInfoList.equal_range (newId);

	// Destination is empty  - NOT EXPECTED
	if (destRangeBounds.first == destRangeBounds.second)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN new index not valid, aborting move op");
		return false;
	}
	//Destination song found
	else
	{
		moveUp = (newId - curId) > 0 ? false : true;

		//when we are moving up, remove and re-add existing song at dest, so that it is placed after moved song
		if (moveUp == true)
		{
			//expecting only one entry at index, due to re-sequencing
			tmpCopyDst = *(destRangeBounds.first);

			//erase destination
			songInfoList.erase (destRangeBounds.first, destRangeBounds.second);
		}
	}

	//find src song
	auto srcRangeBounds = songInfoList.equal_range (curId);

	// source song not found in list - return - no further processing
	if (srcRangeBounds.first == srcRangeBounds.second)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR src song not found");
		return false;
	}
	//source song exists
	else
	{
		//expecting only one entry, due to re-sequencing
		tmpCopySrc = *(srcRangeBounds.first);

		//erase source
		songInfoList.erase (srcRangeBounds.first, srcRangeBounds.second);

		//Update id to dest
		songInfoList.emplace (newId, tmpCopySrc.second);

		//save the song name locally
		//from here to end of this function song will remain same
		//use it to identify the song
	}

	//If dest song was deleted in list, re-add it
	//Only when moving up, dest song entry is deleted and re-added
	if (moveUp == true)
	{
		songInfoList.emplace (tmpCopyDst.first, tmpCopyDst.second);
	}

	auto reAssignedId = reAssignSequentialIdsForSongsAndSave (newId, moveUp);

	//Active song has been moved to a new location.
	//This has to be set, to update active song's ID
	setActiveSong (reAssignedId);

	return true;
}

/* Current use case is to move a newly created (NOT active) song, to the position right after the active song.
* In this case active song position is not changed, so not handling that.
* In future, if that functionality (where active song position change) is also required, refer moveActiveSong(), to handle that scenario.
*/
bool SongManager::moveNonActiveSong(int32_t curId, int32_t newId)
{
	bool moveUp = false;
	std::pair <int32_t, std::string> tmpCopyDst, tmpCopySrc;

	// Sanity check
	if (newId >= numberOfSongs)
	{
		newId = numberOfSongs - 1;
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN rcd index greater than max index, corrcetd to " + String(newId));
	}
	if (newId < 0)
	{
		newId = 0;
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN rcd index less than zero, corrected to 0");
	}

	if (newId == curId)
	{
		//nothing to do, already in position
		return true;
	}

	// find existing song at new dest id
	// copy it in tmp
	auto destRangeBounds = songInfoList.equal_range(newId);

	// Destination is empty  - NOT EXPECTED
	if (destRangeBounds.first == destRangeBounds.second)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN new index not valid, aborting move op");
		return false;
	}
	//Destination song found
	else
	{
		moveUp = (newId - curId) > 0 ? false : true;

		//when we are moving up, remove and re-add existing song at dest, so that it is placed after moved song
		if (moveUp == true)
		{
			//expecting only one entry at index, due to re-sequencing
			tmpCopyDst = *(destRangeBounds.first);

			//erase destination
			songInfoList.erase(destRangeBounds.first, destRangeBounds.second);
		}
	}

	//find src song
	auto srcRangeBounds = songInfoList.equal_range(curId);

	// source song not found in list - return - no further processing
	if (srcRangeBounds.first == srcRangeBounds.second)
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR src song not found");
		return false;
	}
	//source song exists
	else
	{
		//expecting only one entry, due to re-sequencing
		tmpCopySrc = *(srcRangeBounds.first);

		//erase source
		songInfoList.erase(srcRangeBounds.first, srcRangeBounds.second);

		//Update id to dest
		songInfoList.emplace(newId, tmpCopySrc.second);

		//save the song name locally
		//from here to end of this function song will remain same
		//use it to identify the song
	}

	//If dest song was deleted in list, re-add it
	//Only when moving up, dest song entry is deleted and re-added
	if (moveUp == true)
	{
		songInfoList.emplace(tmpCopyDst.first, tmpCopyDst.second);
	}

	reAssignSequentialIdsForSongsAndSave();
	return true;
}

bool SongManager::copySong(std::string songName)
{
	CSong tmpSong(*(activeSong));

	modifySongNameIfDuplicate(songName);

	tmpSong._songData->songName = songName;
	tmpSong._songData->songId = numberOfSongs;
	++numberOfSongs;

	//Newly created copy is added at the end of the list
	songInfoList.emplace (tmpSong.getId() , tmpSong.getName());

	saveSong(tmpSong);

	return true;
}

//ui will first read the current event filter value
// then set/clear appropriate bits(events)
// then call this function
void SongManager::setRecordEventFilter(uint16_t val)
{
	_recordEventFilter = val;
}

uint16_t SongManager::getRecordEventFilter(void)
{
	return _recordEventFilter;
}

void SongManager::setRecordTrigger(bool val)
{
	_recordTrigger = val;
}

TimeBase SongManager::getSceneLargestTimeBase(Scene& scene)
{
	TimeBase savedTb = TimeBase::Base_32T;//initialise to smallest duration timebase

	for (auto& track : scene._sceneData->trackList)
	{
		Pattern* pattern = track.getTrackPattern();
		if (pattern != nullptr)
		{
			TimeBase tmpTb = pattern->getTimeBase();
			//checking for largest TimeBase
			//Higher timebases have lower number
			if (tmpTb > savedTb)
			{
				savedTb = tmpTb;
			}
		}
	}

	return savedTb;
}

void SongManager::handleTVMidiMessageOnRecord(MidiMessage msg, int8_t midiport, bool isInternalSound, Track* track)
{
	/* In record state send a copy of cc message which needs to be recorded */
	if (_state == SYSTEM_STATE::STEP_RECORD || _state == SYSTEM_STATE::LIVE_RECORD)
		handleInMidiMessage(msg, getPortNameFromId(midiport), false);
}

void SongManager::sendMidiMessage (MidiMessage msg, int8_t midiport, bool isInternalSound, Track* track)
{
    if(desktopOffline)
    {
        if(desktopOfflineMidi)desktopOfflineMidi(msg,track);
        if(isInternalSound)sendMidiMessageInternal(msg,track);
        return;
    }
	//DBG("OUT : " + msg.getDescription());
#if 0
	if (isSustainPedalOn && msg.isNoteOff ())
	{
		PendingMidiMsg _PendingMidiMsg;
		_PendingMidiMsg.msg = msg;
		_PendingMidiMsg.midiport = midiport;
		_PendingMidiMsg.isInternalSound = isInternalSound;
		PendingMidiMsgList.push_back(_PendingMidiMsg);
	}
	else
#endif
	{
		// Sound Data is send to both internal and external ouputs , when Internal sound key is pressed
		// TODO Remove isInternalSound, after testing
		if (isInternalSound || isInternelKeyHeld)
		{
			sendMidiMessageInternal (msg, track);
		}

		sendMidiExternal(msg, midiport);

	}
}
#if 0 //moved to scene settings
void SongManager::setTrackSoloMode(bool state)
{
	_trackSoloModeOn = state;
}

bool SongManager::getTrackSoloMode(void)
{
	return _trackSoloModeOn;
}
#endif
uint64_t SongManager::getSceneAllTrackMuteStatus(int32_t sceneId)
{
	uint64_t ret = 0xFFFFFFFFFFFFFFFF;
	for (auto& scene : activeSong->_songData->sceneList)
	{
		if (scene.getId() == sceneId)
		{
			ret = scene._sceneData->trackMuteStatus;
		}
	}
	return ret;
}

void SongManager::setGlobalMuteHold(MuteHoldType val, int32_t sceneId)
{
	_gSceneMuteHold = static_cast <MuteHoldType> (val);

	if (_gSceneMuteHold == MuteHoldType::Lock)
	{
		_gMuteHoldLockedSceneId = (int16_t)sceneId;
	}
}

MuteHoldType SongManager::getGlobalMuteHoldType(void)
{
	return _gSceneMuteHold;
}

Scene* SongManager::getSceneWithId(int32_t sceneId)
{
	Scene* retScene = nullptr;

	if (activeSong != nullptr)
	{
		for (auto& scene : activeSong->_songData->sceneList)
		{
			if (scene.getId() == sceneId)
			{
				retScene = &scene;
				break;
			}
		}
	}
	return retScene;
}

Scene* SongManager::findTheFirstSceneWithMuteHoldLock(void)
{
	Scene* retScene = nullptr;

	if (activeSong != nullptr)
	{
		for (auto& scene : activeSong->_songData->sceneList)
		{
			if (scene.getSceneMuteHold() == MuteHoldType::Lock)
			{
				retScene = &scene;
				break;
			}
		}
	}
	return retScene;
}

void SongManager::setFillModeSwitchOnState(bool state)
{
	_fillModeSwitchOn = state;
}

void SongManager::setFillModeType(int8_t type)
{
	_fillModeType = type;
}

void SongManager::setFillModeRelease(int8_t releasetype)
{
	_fillModeReleaseType = releasetype;

	if (releasetype == static_cast<int8_t> (FillReleaseMode::bar))
	{
		auto playSceneId = getCurrentPlaySceneId();
		if (playSceneId < 0) { playSceneId = 0; }
		_fillModeSceneId = playSceneId;

		auto playBarId = getCurrentPlayingGBarId();
		if (playBarId < 0) { playBarId = 0; }
		_fillModeBarId = playBarId;
	}
	else if (releasetype == static_cast<int8_t> (FillReleaseMode::scene))
	{
		auto playSceneId = getCurrentPlaySceneId();
		if (playSceneId < 0) { playSceneId = 0; }
		_fillModeSceneId = playSceneId;
	}
}

bool SongManager::getFillModeSwitchOnState(void)
{
	return _fillModeSwitchOn;
}

int8_t SongManager::getFillModeType(void)
{
	return _fillModeType;
}

int8_t SongManager::getFillModeRelease(void)
{
	return _fillModeReleaseType;
}

bool SongManager::getFillModeTriggerState(void)
{
	bool ret = false;

	if (_fillModeReleaseType == static_cast <int8_t> (FillReleaseMode::key))
	{
		if (_fillModeSwitchOn == true)
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
	}
	else if (_fillModeReleaseType == static_cast <int8_t> (FillReleaseMode::bar))
	{
		if ((getCurrentPlaySceneId() == _fillModeSceneId) && (getCurrentPlayingGBarId() == _fillModeBarId))
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
	}
	else if (_fillModeReleaseType == static_cast <int8_t> (FillReleaseMode::scene))
	{
		if (getCurrentPlaySceneId() == _fillModeSceneId)
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
	}

	return ret;
}

int32_t SongManager::getCurrentPlaySceneId(void)
{
	if ((activeSong == nullptr) || (activeSong->getPlayScene() == nullptr))
	{
		//assert(0);
		return -1;
	}

	return activeSong->getPlaySceneId();
}

int16_t SongManager::getCurrentPlayingGBarId(void)
{
	if ((activeSong == nullptr) || (activeSong->getPlayScene() == nullptr))
	{
		return -1;
	}

	return activeSong->playScene->gBarId;
}

void SongManager::setGlobalTempo(bool state)
{
	if (activeSong == nullptr)
	{
		return;
	}

	activeSong->setApplySongTempoStatus(state);
}

void SongManager::setGlobalSwing(bool state)
{
	if (activeSong == nullptr)
	{
		return;
	}

	activeSong->setApplySongSwingStatus(state);
}

bool SongManager::getGlobalTempoFlag(void)
{
	if (activeSong == nullptr)
	{
		return false;
	}

	return activeSong->getApplySongTempoStatus();
}

bool SongManager::getGlobalSwingFlag(void)
{
	if (activeSong == nullptr)
	{
		return false;
	}

	return activeSong->getApplySongSwingStatus();
}


bool SongManager::midiRemoteControlEnabled (String msgPortName)
{
	if (_settingsRemoteControlCnfg->_SettingsRemoteControlCnfgData.RemoteControlStatus)
	{
		auto midiPort = _settingsRemoteControlCnfg->_SettingsRemoteControlCnfgData.RemoteControlMidiPort;

		auto remoteControlEnabledPortName = getPortNameFromId (midiPort);

		if (msgPortName.toStdString () == remoteControlEnabledPortName)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void SongManager::handleMidiRemoteControlMessage (const MidiMessage& message)
{
	auto stateLock = engine.lockDesktopState();
	//check if src message channel is enabled for midi remote control
	auto remoteControlEnabledChannel = _settingsRemoteControlCnfg->_SettingsRemoteControlCnfgData.RemoteControlMidiChannel;
	auto msgChannel = message.getChannel ();

	if (msgChannel == remoteControlEnabledChannel)
	{
		//Only use program change message for controls.
		//Other messages types are ignored, if received on remote control midi port
		if (message.isProgramChange () == true)
		{
			auto controlValue = message.getProgramChangeNumber ();
			if (_settingsRemoteControlCnfg->_SettingsRemoteControlCnfgData.RemoteControlSceneChange > 0)
			{
				//if remote control value is out of range,it will be ignored at scene manager
				_PSceneMgr->manualPlayNewScene (controlValue);
			}
			else //TODO Font change not handled at the moment. Will add after discussion with Aman
			{
			}
		}
	}
}

bool SongManager::midiMachineControlEnabled (String msgPortName)
{
	/*Check whether MMC receive is enabled*/
	if (_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcReceiveStatus)
	{
		auto midiPort = _settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcInputPort;

		auto midiMachineControlEnabledPortName = getPortNameFromId (midiPort);

		if (msgPortName.toStdString () == midiMachineControlEnabledPortName)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void SongManager::handleMidiMachineControlMessage (const MidiMessage& message)
{
	auto stateLock = engine.lockDesktopState();
	/* Extract the Device Id(3rd byte) from the Midi Machine Message*/
	auto rawData = message.getRawData ();
	uint8_t deviceId = rawData[2];

	/* Check whether this is an MMC message. */
	/* In mmc commands are received we are calling the front pannel function instead of the engine functions like
		instead of recordStart() invoking engine.SendResponse() with (paramId_t)FrontPannelParam::paramKey_REC as parameter
		The problem when recordStart() engine call was made, engine needs to create new Track,Pattern and update the glocal structure wherein
		UI structure also has to be updated with these new TRack and Pattern information, but there is no call which can be made from engine to UI in regard
		of this and all the states handled in UI couldn't be updated from engine

		Hence front panel call was made by passing respective parameter, which is a software call to trigger the front panel button. So the states will be updated
		properly in the structure*/
	if ((message.isMidiMachineControlMessage () == true) && ((_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcDeviceIdAll) ||
			(_settingsMidiMachineControlCnfg->_SettingsMidiMachineControlCnfgData.mmcDeviceId == deviceId)))
	{
		uint8_t command = (uint8_t)message.getMidiMachineControlCommand ();
		switch (command)
		{
		case MidiMessage::MidiMachineControlCommand::mmc_play:
				engine.SendResponse ((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_PLAY, 1);
				break;
		case MidiMessage::MidiMachineControlCommand::mmc_stop:
				engine.SendResponse ((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_STOP, 1);
			break;
		case MidiMessage::MidiMachineControlCommand::mmc_pause:
				if (_isSongPlaying == true)
				{
					engine.SendResponse ((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_PLAY, 1);
				}
				break;
		case MidiMessage::MidiMachineControlCommand::mmc_recordStart:
			{
				/* If there is a song playing, stop the song*/
				if (_isSongPlaying == true)
				{
					/*This call triggers the frontpanel stop button */
					engine.SendResponse ((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_STOP, 1);
				}
				/*This call triggers the frontpanel record button to start the record */
				engine.SendResponse ((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_REC, 1);
				/*To select the option create new pattern for record */
				engine.SendResponse ((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_RSELECT, 1);
			}
			break;
		case MidiMessage::MidiMachineControlCommand::mmc_recordStop:
			if (_recordTrigger == true)
			{
				engine.SendResponse((moduleId_t)ModuleName::moduleFrontPannel, (moduleId_t)ModuleName::moduleUI, (paramId_t)FrontPannelParam::paramKey_REC, 1);
			}
			break;
		default:
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN CC command not handled: " + String(command));
			break;
		}
	}
}

std::string SongManager::getPortNameFromId (int8_t id)
{
	if (gMidiPortNameList.count (id) != 0)
	{
		return gMidiPortNameList[id];
	}
	else
	{
		return "INVALID";
	}
}

//Implement only get method. Set id should be part of setActiveSong and not done independently
//To be used after a power cycle, to identfy whcih song to highlight
int32_t SongManager::getSavedActiveSongId (void)
{
	return savedActiveSongId;
}

bool SongManager::midiTransposeControlEnabled (String msgPortName)
{
	if (_settingsTransposeControlCnfg->_transposeControlCnfgData.transposeControlStatus)
	{
		auto midiPort = _settingsTransposeControlCnfg->_transposeControlCnfgData.transposeControlMidiPort;

		auto transposeControlEnabledPortName = getPortNameFromId (midiPort);

		if (msgPortName.toStdString () == transposeControlEnabledPortName)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void SongManager::handleMidiTransposeControlMessage (const MidiMessage& message)
{
	auto stateLock = engine.lockDesktopState();
	//check if src message channel is enabled for midi remote control
	auto transposeControlEnabledChannel = _settingsTransposeControlCnfg->_transposeControlCnfgData.transposeControlMidiChannel;

	auto msgChannel = message.getChannel ();

	if (msgChannel == transposeControlEnabledChannel)
	{
		//Only use midinote message for controls.
		//Other messages types are ignored, if received on remote control midi port
		if (message.isNoteOnOrOff () == true)
		{
			auto latchState = _settingsTransposeControlCnfg->_transposeControlCnfgData.transposeControlLatch;

			if ( message.isNoteOn() == true)
			{
				_settingsTransposeControlCnfg->_transposeControlCnfgData.inputNote = (int8_t)message.getNoteNumber();
				auto input = _settingsTransposeControlCnfg->_transposeControlCnfgData.inputNote;
				auto center = _settingsTransposeControlCnfg->_transposeControlCnfgData.transposeControlCenterPitch;
				_settingsTransposeControlCnfg->_transposeControlCnfgData.transposeDelta = center - input;
				_settingsTransposeControlCnfg->_transposeControlCnfgData.tranposeUpdatedFlag.store (true);
			}
			else if (latchState == false) //if latch is enabled ignore note off
			{
				_settingsTransposeControlCnfg->_transposeControlCnfgData.inputNote = _settingsTransposeControlCnfg->_transposeControlCnfgData.transposeControlCenterPitch;
				_settingsTransposeControlCnfg->_transposeControlCnfgData.transposeDelta = 0;
				_settingsTransposeControlCnfg->_transposeControlCnfgData.tranposeUpdatedFlag.store (true);
			}
		}
	}
}

void SongManager::createSongWithRandomData (uint8_t NumOfSongs, uint8_t NumOfScene, uint8_t NumOfTracks, uint8_t numInstrument, uint8_t numPattern, uint8_t numPatternBars)
{
	float tempo = DEFAULT_SONG_TEMPO; //set some default value
	uint8_t swing = DEFAULT_SONG_SWING;//set some default value
	uint8_t numscene = 0;//TODO
	std::string name;
	bool createInstrument = (numInstrument > 0 ? true : false);
	bool createPattern = (numPattern > 0 ? true : false);

	while ((NumOfSongs--) > 0)
	{
		name = "SONG-" + std::to_string (numberOfSongs);
		CSong tempSong (numberOfSongs, name, tempo, swing, numscene, this/*, false*/);
		modifySongNameIfDuplicate (name);
		tempSong._songData->songName = name;

		ClientPatternData clientPatternData;
		//add bar

		for (int32_t i = 0; i < NumOfScene; i++)
		{
			std::string sceneName = SCENE_NAME_PREFIX + std::to_string (i);
			uint64_t trackMute = 0x0; // enable all track
			Scene tempScene (i, sceneName, tempo, swing, NumOfTracks, trackMute, this);
			++(tempSong._songData->numberOfScenes);

			for (auto& trackIt : tempScene._sceneData->trackList)
			{
				if (createInstrument)
				{
					Instrument* instru;
					if (_PInstrumentMgr->getInstrumentCount () < numInstrument)
					{
						instru = _PInstrumentMgr->getInstrument (_PInstrumentMgr->addInstrument ());
						instru->setInternalSoundPgm ((rand () % 255));
					}
					else
						instru = _PInstrumentMgr->getInstrumentFromListIndex ((rand () % numInstrument));

					if (instru != nullptr)
						trackIt.setTrackRxInstrument (instru);

					Instrument* instruOut = nullptr;
					if (_PInstrumentMgr->getInstrumentCount() < numInstrument)
					{
						instruOut = _PInstrumentMgr->getInstrument(_PInstrumentMgr->addInstrument());
						instruOut->setInternalSoundPgm((rand() % 255));
					}

					if (instruOut != nullptr)
						trackIt.setTrackTxInstrument(instruOut);
				}
				else
				{
					Instrument* instru = _PInstrumentMgr->getInstrumentFromListIndex ((rand () % numInstrument));
					if (instru != nullptr)
						trackIt.setTrackRxInstrument (instru);

					Instrument* instruOut = _PInstrumentMgr->getInstrumentFromListIndex((rand() % numInstrument));//TODO may go wrong - check the out port
					if (instruOut != nullptr)
						trackIt.setTrackTxInstrument(instruOut);
				}
				if (createPattern)
				{
					Pattern* pattern;
					if (_PPatternMgr->getPatternCount () < numPattern)
						pattern = _PPatternMgr->GetPattern (_PPatternMgr->addPatternWithRandomData (numPatternBars));
					else
						pattern = _PPatternMgr->GetPatternFromIndex ((rand () % numPattern));

					if (pattern != nullptr)
						trackIt.setTrackMainPattern (pattern);
				}
				else
				{
					Pattern* pattern = _PPatternMgr->GetPatternFromIndex ((rand () % numPattern));
					if (pattern != nullptr)
						trackIt.setTrackMainPattern (pattern);
				}
			}
			tempSong._songData->sceneList.push_back (tempScene);
		}
		engine.get_fileHandler ()->DeleteSong (name);
		engine.get_fileHandler ()->CreateSong (*tempSong._songData);

		songInfoList.emplace (tempSong.getId(), tempSong.getName());

		++numberOfSongs;
	}
}

void SongManager::sendMidiMessageInternal (MidiMessage msg, Track* track)
{
    auto stateLock = engine.lockDesktopState();
    if (track) track->sfHandleMidi(msg);
}

void SongManager::handleRunningSongOnSwitch (void)
{
	if ( (_isSongPaused == true) || (_isSongPlaying == true))
	{
		stopSong();
	}
}

/** Get list of track ids (from current play scene) associated with midi port an update list
 * Same midi port can be used with multiple instrument definitions
 * So get all the tracks using the specified midi port
 */
bool SongManager::UpdateplaySceneTrackPortMap()
{
	Scene* playScene;
	playSceneTrackPortMap.clear();
	if ((activeSong != nullptr) && ((playScene = activeSong->getPlayScene()) != nullptr))
	{
		for (auto& track : playScene->_sceneData->trackList)
		{
			if (track._trackData->trackRxInstrument != nullptr)
			{
				playSceneTrackPortMap.emplace(track._trackData->trackRxInstrument->getMidiPort(), track.getTrackId());
			}
		}
	}
	return (playSceneTrackPortMap.size() > 0);
}

//This is done for each packet, so limit the simultaneous track recorded to min num
std::vector <Instrument*> SongManager::getInstrumentsFromPortName (std::string portName)
{
	std::vector <Instrument *> instrumentList;

	for (auto iter = engine.instrumentManager->instrumentList.begin (); iter != engine.instrumentManager->instrumentList.end (); ++iter)
	{
		if (iter->getMidiPort() == getPortIdFromName (portName))
		{
			instrumentList.push_back (&(*iter));
		}
	}

	if (instrumentList.size () == 0)
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "WARN Instrument not found for port: " + portName);
	}
	return instrumentList;
}

int32_t SongManager::getSongIdFromName (std::string songName)
{
	int32_t id = -1;

	auto iter = songInfoList.begin ();

	for (; iter != songInfoList.end (); ++iter)
	{
		if (iter->second == songName)
		{
			id = iter->first;
			break;
		}
	}

	if (iter == songInfoList.end ())
	{
		DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR song not found in list");
	}

	return id;
}

int32_t SongManager::getSceneIdFromName (std::string sceneName)
{
	for (auto iter = activeSong->_songData->sceneList.begin (); iter != activeSong->_songData->sceneList.end (); ++iter)
	{
		if (iter->getName() == sceneName)
		{
			return iter->getId();
		}
	}
	return -1;
}

uint8_t SongManager::getTrackIdFromName (std::string trackName)
{
	for (auto iter = activeSong->getEditScene()->_sceneData->trackList.begin(); iter != activeSong->getEditScene ()->_sceneData->trackList.end (); ++iter)
	{
		if (iter->getTrackName() == trackName)
		{
			return iter->getTrackId();
		}
	}
	return 0;
}

int8_t SongManager::getPortIdFromName (std::string name)
{
	for (auto iter = gMidiPortNameList.begin (); iter != gMidiPortNameList.end (); ++iter)
	{
		if (iter->second == name)
		{
			return iter->first;
		}
	}
	return -1;
}

/** Sets the song's parameter[s] to last saved value.
 * Current requirement is only for resetting song NAME
 * Any new params can be added in switch, if required in future.
 */
void SongManager::resetToDefaultParamValues (std::vector<SongParam> paramList)
{
	for (auto param : paramList)
	{
		switch (param)
		{
			case SongParam::paramSongName:
			{
				auto name = (std::string)(GetDefaultDataFromList ((paramId_t)SongParam::paramSongName).get<std::string> ().c_str ());
				if (name.size () != 0)
				{
					SongManager::setName (name);
					activeSong->songNameDirtyFlag = false;

				}
				else
				{
					DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "Unable to reset to saved song name");
				}
				break;
			}

			default:
				DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "ERR unexpected song param");
				break;
		}
	}
}

uint8 SongManager::initialByte (const int type, const int channel) noexcept
{
	return (uint8)(type | setBounds (0, 15, channel));
}

MidiMessage SongManager::controllerEvent (const int channel, const int controllerType, const int value) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0xb0, channel),
		controllerType & 127, value & 127);
}

MidiMessage SongManager::programChange (const int channel, const int programNumber) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0xc0, channel), programNumber & 0x7f);
}

MidiMessage SongManager::noteOn (const int channel, const int noteNumber, const uint8 velocity) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0x90, channel), noteNumber & 127, velocity);
}

MidiMessage SongManager::noteOff (const int channel, const int noteNumber, uint8 velocity) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0x80, channel), noteNumber & 127, velocity);
}

MidiMessage SongManager::allNotesOff (const int channel) noexcept
{
	return SongManager::controllerEvent (channel, 123, 0);
}

MidiMessage SongManager::allSoundOff (const int channel) noexcept
{
	return SongManager::controllerEvent (channel, 120, 0);
}

MidiMessage SongManager::pitchWheel (const int channel, const int position) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0xe0, channel), position & 127, (position >> 7) & 127);
}

MidiMessage SongManager::pitchWheel (const int channel, const int data1, const int data2) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0xe0, channel), data1 & 127, data2 & 127);
}

MidiMessage SongManager::channelPressureChange (const int channel, const int pressure) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0xd0, channel), pressure & 0x7f);
}

MidiMessage SongManager::aftertouchChange (const int channel, const int noteNum, const int aftertouchValue) noexcept
{
	// the channel must be between 0 and 15 inclusive
	assert (channel >= 0 && channel <= 15);

	return MidiMessage (SongManager::initialByte (0xa0, channel), noteNum & 0x7f, aftertouchValue & 0x7f);
}


void SongManager::getRecordParams (void)
{
	IgniteParamList paramInfo;
	std::vector<IgniteParamList> paramList;

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordQuantizationParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordMthruParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordFTSParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordAutoAddBarParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordBarOverWriteParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordNumOfBarsParam;
	paramList.push_back (paramInfo);


	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordNoteEventParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordMidiCCEventParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordAfterTouchEventParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordPitchBendEventParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordPolyPEventParam;
	paramList.push_back (paramInfo);

	paramInfo.param = (paramId_t)SettingsRecordCnfgParam::SettingRecordPgmChangeEventParam;
	paramList.push_back (paramInfo);


	paramList = engine.get_settingsRecordCnfg ()->getParamList ((moduleId_t)(ModuleName::moduleSong), paramList);

	for (auto& param : paramList)
	{
		if (_recordSetting.count (param.param) != 0)
		{
			_recordSetting.erase (param.param);
		}
		_recordSetting.emplace (param.param, param.MsgValue.get<int8_t>());
	}
}

void SongManager::GetEngineDataBackup (uint8_t DataFolderId)
{
	engine.get_fileHandler()->CreateBackUP(DataFolderId);
}

void SongManager::RestoreEngineDataBackup(uint8_t DataFolderId)
{
	engine.get_fileHandler()->RestoreBackUP(DataFolderId);
}
void SongManager::openMidiInterfaces (void)
{
	// Init midi interface list
	// Place holders for all the midi ports are added
	// Input and output interfaces are initiliased to nullptr
	// Input and output interfaces will be set up in the below code, based on
	// the midi ports identified by JUCE midi framework
	for (auto& midiEntry : gMidiPortNameList)
	{
		IgniteMidi midiDev;
		midiDev.deviceName = midiEntry.second;
		int8_t id = midiEntry.first;
		midiDevList.emplace (id, midiDev);
	}

#ifdef _WIN32
    for (int port = int(midiDataIOPorts::usbHostMidi1); port <= int(midiDataIOPorts::usbHostMidi5); ++port)
        gMidiPortNameList[int8_t(port)] = "Desktop USB host " + std::to_string(port - int(midiDataIOPorts::usbHostMidi1) + 1);
#endif
	// OUTPUT INTERFACE
	auto availableOutDevices = MidiOutput::getAvailableDevices ();
	for (auto& midiOutDevice : availableOutDevices)
	{
#if 0 //AudIo not used.
		// TODO : Bug with AudIO init.
		// Windows synth has to be opened before the devmgr.start (SAMPLE_RATE, BLOCK_SIZE);
		if (midiOutDevice.name == "Microsoft GS Wavetable Synth")
		{
			MidiOutput::openDevice (midiOutDevice.identifier);
		}
#endif
		if
		(
			(midiOutDevice.name == IGNITE_USB_D_MIDI_PORT_1_NAME) ||
			(midiOutDevice.name == IGNITE_USB_D_MIDI_PORT_2_NAME) ||
			(midiOutDevice.name == IGNITE_USB_D_MIDI_PORT_3_NAME) ||
			(midiOutDevice.name == IGNITE_USB_D_MIDI_PORT_4_NAME) ||
			(midiOutDevice.name == IGNITE_USB_D_MIDI_PORT_5_NAME) ||
			(midiOutDevice.name == IGNITE_USB_D_MIDI_PORT_6_NAME)
		)

		{
			auto outDeviceHandle = &((midiDevList [getPortIdFromName (midiOutDevice.name.toStdString())]).outputInterface);
			*outDeviceHandle = MidiOutput::openDevice (midiOutDevice.identifier);
			if (outDeviceHandle->get () == nullptr)
			{
				DBG (String (__FUNCTION__) + "::" + String(__LINE__) + "::" + "Unable to open Midi Out " + midiOutDevice.name);
			}
		}
	}


	// INPUT INTERFACE
	auto availableInDevices = MidiInput::getAvailableDevices ();
	for (auto& midiInDevice : availableInDevices)
	{
		juce::Logger::outputDebugString (midiInDevice.name);

		if
		(
			(midiInDevice.name == IGNITE_USB_D_MIDI_PORT_1_NAME) ||
			(midiInDevice.name == IGNITE_USB_D_MIDI_PORT_2_NAME) ||
			(midiInDevice.name == IGNITE_USB_D_MIDI_PORT_3_NAME) ||
			(midiInDevice.name == IGNITE_USB_D_MIDI_PORT_4_NAME) ||
			(midiInDevice.name == IGNITE_USB_D_MIDI_PORT_5_NAME) ||
			(midiInDevice.name == IGNITE_USB_D_MIDI_PORT_6_NAME)
		)
		{
			auto midiDeviceHandle = &(midiDevList [getPortIdFromName (midiInDevice.name.toStdString ())]);
			midiDeviceHandle->inputInterface = MidiInput::openDevice (midiInDevice.identifier, (MidiInputCallback*)midiDeviceHandle);
			if (midiDeviceHandle->inputInterface != nullptr)
			{
				midiDeviceHandle->inputInterface->start ();
			}
			else
			{
				DBG (String (__FUNCTION__) + "::" + String (__LINE__) + "::" + "Unable to open Midi In " + midiInDevice.name);
			}
		}
	}

#ifdef __linux__
	//Host USB MIDI device

	//Open Alreday connected
	checkAndOpenMidiHostDevice();

	//dynamic handling for device connected in future
	struct sigaction sa;
	sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 
	sigaction(SIGUSR1, &sa, NULL);//add 
	sigaction(SIGUSR2, &sa, NULL);//remove
#endif
}

void SongManager::setSceneTimeBaseTick (void)
{
	Scene* playScene = activeSong->getPlayScene ();

	if (playScene != nullptr)
	{
		TimeBase tb = getSceneLargestTimeBase(*playScene);

		_scenePulsesPerStep = getPulsesPerStep(tb);
		_sceneTimeBase = tb;
	}
}

void SongManager::clearSceneTimeBaseTick(void)
{
	Scene* playScene = activeSong->getPlayScene();

	if (playScene != nullptr)
	{
	}
}

/**
* When pattern's TB is changed, Scene TB needs to be updated,
* if new pattern TB is bigger (in duration) than current screen TB.
* This function is used to set it on-the-go, if it is in playing (play,pause) state.
* If it is not playing, then it would be taken care at start of next play.
**/
void SongManager::checkAndApplyNewTimeBaseForScene (void)
{
	if ((_isSongPlaying) || (_isSongPaused))
	{
		setSceneTimeBaseTick();
	}
}

/**
* Returns true if an Active Edit Track is present, with Tx Instrument assigned.
*/
bool SongManager::getActiveTrackTxInstrumentAvailability(void)
{
	if ((activeSong != nullptr) &&
		(activeSong->editScene != nullptr) &&
		(activeSong->editScene->editTrack != nullptr) &&
		(activeSong->editScene->editTrack->getTrackTxInstrument() != nullptr)
		)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SongManager::setUpPlatformAudio(void)
{
#ifdef USE_JUCE_AUDIO

	//JUCE call back setup
	if (dm != nullptr)
		dm->addAudioCallback(this);

#else /* USE_JUCE_AUDIO */

#ifdef _WIN32
	//FORMAT
	windowsAudioFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT; // For 32-bit float
	windowsAudioFormat.nChannels = INTERNAL_AUDIO_CHANNEL_COUNT; // TSF renders mono audio
	windowsAudioFormat.nSamplesPerSec = INTERNAL_AUDIO_SAMPLE_RATE; // 48 kHz
	windowsAudioFormat.wBitsPerSample = INTERNAL_AUDIO_SAMPLE_SIZE_BITS; // 32 bits
	windowsAudioFormat.nBlockAlign = (windowsAudioFormat.nChannels * windowsAudioFormat.wBitsPerSample) / 8;
	windowsAudioFormat.nAvgBytesPerSec = windowsAudioFormat.nSamplesPerSec * windowsAudioFormat.nBlockAlign;

#elif defined __linux__

//TODO direct platform audio for linux (experimental)

#endif /* __linux__ */

#endif /* USE_JUCE_AUDIO */


}

void SongManager::audioDeviceAboutToStart(AudioIODevice* device)
{
    const auto rate = device->getCurrentSampleRate();
    desktopHostSampleRate = rate;
    audioResampler.setResamplingRatio(double(INTERNAL_AUDIO_SAMPLE_RATE) / rate);
    audioResampler.prepareToPlay(device->getCurrentBufferSizeSamples(), rate);
	juce::Logger::writeToLog("Audio device: " + device->getName() + ", " +
		juce::String(device->getCurrentSampleRate()) + " Hz, block " + juce::String(device->getCurrentBufferSizeSamples()));
}

void SongManager::audioDeviceStopped()
{
    audioResampler.releaseResources();
}

void SongManager::prepareToPlay(int, double) { audioBlockAdapter.reset(); }
void SongManager::releaseResources() {}
void SongManager::getNextAudioBlock(const juce::AudioSourceChannelInfo& info)
{
    float* outputs[2]{};
    const int count=std::min(2,info.buffer->getNumChannels());
    for(int channel=0;channel<count;++channel) outputs[channel]=info.buffer->getWritePointer(channel,info.startSample);
    audioBlockAdapter.process(outputs,count,info.numSamples,[this](float* block){renderDesktopStereo(block);});
    desktopSourceSamples+=info.numSamples;
}


#if (JUCE_VERSION>=8) && (JUCE_BUILDNUMBER>=0)

void SongManager::audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int deviceInputChannels,
	float* const* outputChannelData, int deviceOutputChannels,
	int numSamples, const juce::AudioIODeviceCallbackContext& /*context*/)

#else

void SongManager::audioDeviceIOCallbackWithContext(const float** inputChannelData, int numInputChannels,
	float** outputChannelData, int numOutputChannels,
	int numSamples, const juce::AudioIODeviceCallbackContext& context)

#endif
{

    juce::ScopedNoDenormals noDenormals;
    float* outputs[2]{};int count=0;
    for(int ch=0;ch<deviceOutputChannels;++ch)
        if(outputChannelData[ch])
        {
            if(count<2) outputs[count++]=outputChannelData[ch];
            else std::fill_n(outputChannelData[ch],numSamples,0.0f);
        }
    if(count==0 || numSamples<=0) return;
    juce::AudioBuffer<float> output(outputs,count,numSamples);
    audioResampler.getNextAudioBlock({&output,0,numSamples});
    ++desktopAudioCallbacks;desktopOutputSamples+=numSamples;
    float peak=desktopAudioPeak.load(std::memory_order_relaxed);
    for(int ch=0;ch<count;++ch) for(int i=0;i<numSamples;++i) peak=std::max(peak,std::abs(outputs[ch][i]));
    desktopAudioPeak.store(peak,std::memory_order_relaxed);
}

void SongManager::startJuceAudioCallback (void)
{
	if (_isAudioCallbackEnabled)
		return;
	if (dm == nullptr)
	{
		juce::Logger::writeToLog("Audio device manager is not ready; playback can be retried.");
		return;
	}
	juce::AudioDeviceManager::AudioDeviceSetup setup;
	setup.sampleRate = (double)INTERNAL_AUDIO_SAMPLE_RATE;
	setup.bufferSize = (int)INTERNAL_AUDIO_20MS_BLOCK_SIZE;
    auto saved = juce::JSON::parse(desktopSettingsFile()).getProperty("AudioDevice", "").toString();
    auto xml = juce::XmlDocument::parse(saved);
    const auto error = dm->initialise(0, 2, xml.get(), true, {}, xml ? nullptr : &setup);
	auto* device = dm->getCurrentAudioDevice();
	_isAudioCallbackEnabled = error.isEmpty() && device != nullptr && device->isOpen();
	if (!_isAudioCallbackEnabled)
		juce::Logger::writeToLog("Cannot start audio: " + (error.isEmpty() ? juce::String("no open output device") : error));
}

void SongManager::stopJuceAudioCallback (void)
{
	if (!_isAudioCallbackEnabled)
		return;
	_isAudioCallbackEnabled = false;
	if (dm != nullptr) dm->closeAudioDevice();
}

/* Handles play-time internal sound program change, done at instrument level*/
void SongManager::handleInternalPgmChange (int32_t instrId, int programId)
{
	auto stateLock = engine.lockDesktopState();
	Scene* playScene;

	(void) programId;

	if ((activeSong != nullptr) && ((playScene = activeSong->getPlayScene()) != nullptr))
	{
		for (auto& track : playScene->_sceneData->trackList)
		{
			if (track._trackData->trackTxInstrument != nullptr)
			{
				if (track._trackData->trackTxInstrument->getId() == instrId)
				{
					track.sfChangeProgram();
				}
			}
		}
	}
}
void SongManager::handleMidiSettingChange(int32_t instrId)
{
	auto stateLock = engine.lockDesktopState();
	Scene* playScene;
	if ((activeSong != nullptr) && ((playScene = activeSong->getPlayScene()) != nullptr))
	{
		for (auto& track : playScene->_sceneData->trackList)
		{
			if (track._trackData->trackTxInstrument != nullptr)
			{
				if (track._trackData->trackTxInstrument->getId() == instrId)
				{
					track.setTrackTxMidiPortAndChannel();
				}
			}
			if (track._trackData->trackRxInstrument != nullptr)
			{
				if (track._trackData->trackRxInstrument->getId() == instrId)
				{
					track.setTrackRxMidiPortAndChannel();
				}
			}
		}
		UpdateplaySceneTrackPortMap();
	}
}
#ifdef USE_INTERNAL_20MS_TIMER
#ifdef _WIN32
void SongManager::startWindowsAudioDirect(void)
{
	MMRESULT result = waveOutOpen(&windowsAudioHandle, WAVE_MAPPER, &windowsAudioFormat, 0, 0, CALLBACK_NULL);
	if(result != MMSYSERR_NOERROR)
	{
		// Handle error - internal audio will not work
		DBG("Unable to open direct Windows Audio : Result = " + String(result));
	}
}

void SongManager::playWindowsAudioDirect (AudioDataType* audioBuffer, int sizeInBytes)
{
	//HEADER
	windowsAudioHeader.lpData = (LPSTR)audioBuffer;
	windowsAudioHeader.dwBufferLength = sizeInBytes;
	windowsAudioHeader.dwFlags = 0;

	if (waveOutPrepareHeader(windowsAudioHandle, &windowsAudioHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		DBG("Windows audio prepare error");
	}

	// Play the audio
	if (waveOutWrite(windowsAudioHandle, &windowsAudioHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		DBG("Windows audio output error");
	}
}

void SongManager::stopWindowsAudioDirect(void)
{
	waveOutUnprepareHeader(windowsAudioHandle, &windowsAudioHeader, sizeof(WAVEHDR));
	waveOutClose(windowsAudioHandle);
}

#endif /* _WIN32 */

void SongManager::start20msAudioProcessingTimer(void)
{
	if (_isAudioTimerEnabled)
		return;
	_isAudioTimerEnabled = true;
#ifdef _WIN32

#define AUDIO_TIMER_PERIOD_MS	20 // 20 milliseconds period
	audioGentimerId = timeSetEvent(AUDIO_TIMER_PERIOD_MS, 1, audioGenTimer, reinterpret_cast<DWORD_PTR>(this), TIME_PERIODIC);

#elif defined __linux__

#define AUDIO_TIMER_PERIOD_NS	20000000 //20 milliseconds
	int ret = 0;
	struct sigevent sEvent = { 0 };

	sEvent.sigev_notify = SIGEV_THREAD;
	sEvent.sigev_notify_function = audioGenTimer;
	sEvent.sigev_value.sival_ptr = reinterpret_cast<void *>(this);

	ret = timer_create(CLOCK_MONOTONIC, &sEvent, &audioGentimerId);
	if (ret < 0)
	{
		DBG("Timer creation failed");
		return;
	}

	struct itimerspec timeData;

	//Reload value
	timeData.it_interval.tv_nsec = AUDIO_TIMER_PERIOD_NS;
	timeData.it_interval.tv_sec = 0;

	//First expiry - using relative
	timeData.it_value.tv_nsec = AUDIO_TIMER_PERIOD_NS;
	timeData.it_value.tv_sec = 0;

	ret = timer_settime(audioGentimerId, 0, &timeData, NULL);
	if (ret < 0)
	{
		DBG("Timer start failed");
		return;
	}

#endif /* __linux__ */

}
#endif //USE_INTERNAL_20MS_TIMER

#ifdef LOG_PCM_TO_FILE

void SongManager::openLogFiles (void)
{
	openLogFile_1 ();
	openLogFile_2 ();
	openLogFile_3 ();
}
void SongManager::openLogFile_1 (void)
{
#ifdef _WIN32

	//open file
	fd_1 = CreateFile(PCM_LOG_FILE_1, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,	FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd_1 == INVALID_HANDLE_VALUE)
	{
		return;
	}
	else
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// The file existed
			DWORD ret = SetFilePointer(fd_1, 0, NULL, FILE_END);
			if (ret == INVALID_SET_FILE_POINTER)
			{
				// not able to seek, it will overwrite
			}
		}
	}

#elif defined __linux__

	fd_1 = open(PCM_LOG_FILE_1, O_RDWR|O_CREAT, 0777);
	if (fd_1 < 0)
	{
		DBG("File open failed\n");
		return;
	}

	int ret = lseek(fd_1, 0, SEEK_END);
	if(ret < 0)
	{
		DBG("Seek failed\n");
	}

#endif /* __linux__ */

}

void SongManager::openLogFile_2 (void)
{
#ifdef _WIN32

	//open file
	fd_2 = CreateFile(PCM_LOG_FILE_2, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,	FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd_2 == INVALID_HANDLE_VALUE)
	{
		return;
	}
	else
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// The file existed
			DWORD ret = SetFilePointer(fd_2, 0, NULL, FILE_END);
			if (ret == INVALID_SET_FILE_POINTER)
			{
				// not able to seek, it will overwrite
			}
		}
	}

#elif defined __linux__

	fd_2 = open(PCM_LOG_FILE_2, O_RDWR|O_CREAT, 0777);
	if (fd_2 < 0)
	{
		DBG("File open failed\n");
		return;
	}

	int ret = lseek(fd_2, 0, SEEK_END);
	if(ret < 0)
	{
		DBG("Seek failed\n");
	}

#endif /* __linux__ */
}

void SongManager::openLogFile_3 (void)
{
#ifdef _WIN32

	//open file
	fd_3 = CreateFile(PCM_LOG_FILE_3, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,	FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd_3 == INVALID_HANDLE_VALUE)
	{
		return;
	}
	else
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// The file existed
			DWORD ret = SetFilePointer(fd_3, 0, NULL, FILE_END);
			if (ret == INVALID_SET_FILE_POINTER)
			{
				// not able to seek, it will overwrite
			}
		}
	}

#elif defined __linux__

	fd_3 = open(PCM_LOG_FILE_3, O_RDWR|O_CREAT, 0777);
	if (fd_3 < 0)
	{
		DBG("File open failed\n");
		return;
	}

	int ret = lseek(fd_3, 0, SEEK_END);
	if(ret < 0)
	{
		DBG("Seek failed\n");
	}

#endif /* __linux__ */
}

void SongManager::logToFile_3(SongManager *context, AudioDataType *buffer, int sizeInBytes)
{
#ifdef _WIN32

	LPDWORD writtenBytes = 0;
	WriteFile(context->fd_3 , buffer, sizeInBytes, writtenBytes, NULL);

#elif defined __linux__

	int ret;
	ret = write(context->fd_3, buffer, sizeInBytes);

#endif /* __linux__ */
}

void SongManager::logToFile_2(AudioDataType *buffer, int sizeInBytes)
{
#ifdef _WIN32

	LPDWORD writtenBytes = 0;
	WriteFile(fd_2, buffer, sizeInBytes, writtenBytes, NULL);

#elif defined __linux__

	int ret;
	ret = write(fd_2, buffer, sizeInBytes);

#endif /* __linux__ */
}

void SongManager::logToFile_1(AudioDataType *buffer, int sizeInBytes)
{
#ifdef _WIN32

	LPDWORD writtenBytes = 0;
	WriteFile(fd_1, buffer, sizeInBytes, writtenBytes, NULL);

#elif defined __linux__

	int ret;
	ret = write(fd_1, buffer, sizeInBytes);

#endif /* __linux__ */
}

void SongManager::closeLogFiles(void)
{
#ifdef _WIN32

	CloseHandle(fd_1);
	CloseHandle(fd_2);
	CloseHandle(fd_3);

#elif defined __linux__

	close(fd_1);
	close(fd_2);
	close(fd_3);

#endif
}

#endif /* LOG_PCM_TO_FILE */


#ifdef USE_INTERNAL_20MS_TIMER
void SongManager::directPlatformPlay (AudioDataType *TwentyMsBuffer, int numBytes)
{
#ifdef _WIN32

	SongManager::playWindowsAudioDirect(TwentyMsBuffer, numBytes);

#elif defined __linux__

	//TODO direct linux playback

#endif /* __linux__ */
}

void SongManager::stopAudioProcessingTimer(void)
{
	if (!_isAudioTimerEnabled)
		return;
	_isAudioTimerEnabled = false;
#ifdef _WIN32

	timeKillEvent(audioGentimerId);
	timeEndPeriod(1);

#elif defined __linux__

	timer_delete(audioGentimerId);

#endif /* __linux__ */
}
#endif /* USE_INTERNAL_20MS_TIMER */


#ifdef _WIN32
void SongManager::renderDesktopStereo(AudioDataType* buffer,int frames)
{
    if(frames<1 || frames>INTERNAL_AUDIO_20MS_BLOCK_SIZE)return;
    std::fill_n(buffer,2*frames,AudioDataType{});
    std::unique_lock<std::recursive_mutex> lock(engine.stateMutex,std::try_to_lock);
    if(!lock.owns_lock()) return;
    float* channels[]{buffer,buffer+frames};
    juce::AudioBuffer<float> output(channels,2,frames);
    auto mix=[&](Track& track)
    {
        track.sfSynthesizeAudio(frames);
        for(int ch=0;ch<2;++ch) output.addFrom(ch,0,track.audioBuff,ch,0,frames,_masterVolume);
    };
    auto* scene=activeSong?activeSong->getPlayScene():nullptr;
    if(scene) for(auto& track:scene->_sceneData->trackList) if(!track.trackMuted()) mix(track);
    if(_isAuditioningOn && (_state==SYSTEM_STATE::IDLE || _state==SYSTEM_STATE::PAUSE))
        for(int i=0;i<AUDITIONING_MAX_PORT;++i) if(isAuditioningTrackEnabled[i] && AuditioningTrack[i]) mix(*AuditioningTrack[i]);
    if(desktopMonitorEnabled && !_isAuditioningOn && _state!=SYSTEM_STATE::PLAY && _state!=SYSTEM_STATE::LIVE_RECORD &&
       activeSong && activeSong->editScene && activeSong->editScene!=scene && activeSong->editScene->editTrack)
    {
        auto* track=activeSong->editScene->editTrack;
        if(track->getTrackTxInstrument() && track->getTrackTxInstrument()->getInternalSound()) mix(*track);
    }
}

// Kept for existing firmware/offline callers that explicitly request a mono block.
void SongManager::renderTracksAndMix(AudioDataType* buffer)
{
    AudioDataType stereo[2*INTERNAL_AUDIO_20MS_BLOCK_SIZE]{};
    renderDesktopStereo(stereo);
    for(int i=0;i<INTERNAL_AUDIO_20MS_BLOCK_SIZE;++i)buffer[i]=(stereo[i]+stereo[INTERNAL_AUDIO_20MS_BLOCK_SIZE+i])*0.5f;
}
#else
#ifdef ENABLE_PARALLEL_EXE
void SongManager::renderTracksAndMix(AudioDataType* buffer)
{
#ifdef _WIN32
    std::unique_lock<std::recursive_mutex> stateLock(engine.stateMutex, std::try_to_lock);
    if (!stateLock.owns_lock())
    {
        std::fill_n(buffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE, AudioDataType{});
        return;
    }
#endif
#ifdef USE_INTERNAL_EFFECTS
	juce::AudioBuffer<float> audioOutBuff(&buffer, 1, INTERNAL_AUDIO_20MS_BLOCK_SIZE);
	audioOutBuff.clear();
#endif
	std::mutex audioMutex;
	if (activeSong != nullptr)
	{
		auto playScene = activeSong->getPlayScene();

		if (playScene != nullptr)
		{
			auto& trackList = playScene->_sceneData->trackList;

			// Use a mutex to protect audioOutBuff
			std::for_each(
				std::execution::par_unseq,
				trackList.begin(), trackList.end(),
				[this, &audioMutex, &audioOutBuff, buffer]
				(auto&& track)
				{
					if (!track.trackMuted())
					{
						track.sfSynthesizeAudio(); //independently in parallel
#ifdef ENABLE_TRACK_MIXING
						if (track._trackData->trackMute == false)
						{
							std::lock_guard<std::mutex> lock(audioMutex);// make it run sequentially
							audioOutBuff.addFrom(0, 0, track.audioBuff, 0, 0, track.audioBuff.getNumSamples(), _masterVolume);
						}
#else
						memcpy(buffer, track._twentyMsBuffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE * sizeof(AudioDataType));
#endif
					}
					});
			}
		}
	if ((_isAuditioningOn) && ((_state == SYSTEM_STATE::IDLE || _state == SYSTEM_STATE::PAUSE))) // Auditioning not enabled on play/rec
	{
		for (uint8_t i = 0; i < AUDITIONING_MAX_PORT; i++)
		{
			if (isAuditioningTrackEnabled[i])
			{
				AuditioningTrack[i]->sfSynthesizeAudio();
#ifdef ENABLE_TRACK_MIXING
				std::lock_guard<std::mutex> lock(audioMutex);
				audioOutBuff.addFrom(0, 0, AuditioningTrack[i]->audioBuff, 0, 0, AuditioningTrack[i]->audioBuff.getNumSamples(), _masterVolume);
#endif
			}
		}
	}
    renderDesktopMonitor(buffer);
}
#else
void SongManager::renderTracksAndMix(AudioDataType* buffer)
{
#ifdef _WIN32
    std::unique_lock<std::recursive_mutex> stateLock(engine.stateMutex, std::try_to_lock);
    if (!stateLock.owns_lock())
    {
        std::fill_n(buffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE, AudioDataType{});
        return;
    }
#endif
#ifdef USE_INTERNAL_EFFECTS
	juce::AudioBuffer<float>audioOutBuff(&buffer, 1, INTERNAL_AUDIO_20MS_BLOCK_SIZE);
	audioOutBuff.clear();
#endif
	if (activeSong != nullptr)
	{
		auto playScene = activeSong->getPlayScene();

		if (playScene != nullptr)
		{
			auto& trackList = playScene->_sceneData->trackList;

			for (auto& track : trackList)
			{
				if (!track.trackMuted())
				{
					track.sfSynthesizeAudio();
#ifdef ENABLE_TRACK_MIXING
					if (track._trackData->trackMute == false)
						audioOutBuff.addFrom(0, 0, track.audioBuff, 0, 0, track.audioBuff.getNumSamples(), _masterVolume);
#else
					memcpy(buffer, track._twentyMsBuffer, INTERNAL_AUDIO_20MS_BLOCK_SIZE * sizeof(AudioDataType));
#endif
				}
			}
		}
	}
	if ((_isAuditioningOn) && ((_state == SYSTEM_STATE::IDLE || _state == SYSTEM_STATE::PAUSE)))//Auditioning not enabled on play/rec
	{
		for (uint8_t i = 0; i < AUDITIONING_MAX_PORT; i++)
		{
			if (isAuditioningTrackEnabled[i])
			{
				AuditioningTrack[i]->sfSynthesizeAudio();
#ifdef ENABLE_TRACK_MIXING
				audioOutBuff.addFrom(0, 0, AuditioningTrack[i]->audioBuff, 0, 0, AuditioningTrack[i]->audioBuff.getNumSamples(), _masterVolume);
#endif
			}
		}
	}
    renderDesktopMonitor(buffer);
}
#endif//ENABLE_PARALLEL_EXE

void SongManager::renderDesktopStereo(AudioDataType* buffer,int frames)
{
    if(frames!=INTERNAL_AUDIO_20MS_BLOCK_SIZE)return;
    renderTracksAndMix(buffer);
    std::copy_n(buffer,INTERNAL_AUDIO_20MS_BLOCK_SIZE,buffer+INTERNAL_AUDIO_20MS_BLOCK_SIZE);
}
#endif

bool SongManager::setDesktopVoiceLimit(int voices)
{
    if(voices!=8 && voices!=16 && voices!=32 && voices!=64 && voices!=128) return false;
    auto lock=engine.lockDesktopState();
    if(voices==desktopVoiceLimit) return true;
    if(activeSong){stopRecord();stopSong();setSystemState(KEY_PRESSED::STOP_KEY);}
    desktopPanic();
    if(activeSong)for(auto& scene:activeSong->_songData->sceneList)for(auto& track:scene._sceneData->trackList)track.unloadDesktopSoundFont();
    for(auto& track:AuditioningTrack)if(track)track->unloadDesktopSoundFont();
    desktopVoiceLimit=voices;
    for(auto& track:AuditioningTrack)if(track)track->AuditioningTrackInit();
    return true;
}

void SongManager::setTempoFromFrequency(float freqHz)
{
	auto ret = engine.settingsMidiClkCnfg->getParamSingle(SettingsMidiClkCnfgParam::inputPpqn);

	auto configPpqn = ret.get<int8_t>();

	auto ppqn = mapConfigPpqnToPpqn(configPpqn);

	auto tempo = ((float)(freqHz * 60)/ ppqn);

	setTempo(tempo);
}

//Set tempo if Ignite is in clock slave mode, with midi(DIN/USB) port selected as source
void SongManager::handleMidiClkIn(float freqHz)
{
	setTempoFromFrequency (freqHz);
}

void SongManager::handleSyncIn(float frequencyHz)
{
	auto stateLock = engine.lockDesktopState();
	//check if it is in clock slave mode(external sync-in clock source)
	if (engine.settingsMidiClkCnfg->isClkInFromSyncPort())
	{
		setTempoFromFrequency(frequencyHz);
	}
}

float SongManager::mapConfigPpqnToPpqn(int8_t configPpqn)
{
	float ret = 24;//default

	switch (configPpqn)
	{
	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_1PP16:
		ret = 4;
		break;

	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_1PP2Q:
		ret = 0.5;
		break;

	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_1PP4Q:
		ret = 0.25;
		break;

	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_1PP8:
		ret = 2;
		break;

	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_1PPQ:
		ret = 1;
		break;

	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_24PPQ:
		ret = 24;
		break;

	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_2PPQ:
		ret = 2;
		break;

	case (int)SettingsMidiClkCnfgInputClkRate::inClkRate_48PPQ:
		ret = 48;
		break;

	default:
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "ERR Unknown clock ppqn config, default to 24 ppqn");
		break;
	}

	return ret;
}


int8_t SongManager::getChannelFromMidiMessage(MidiMessage msg)
{
	//JUCE code return 1 to 16
	//Midi std follows 0 to 15
	return static_cast<int8_t>((msg.getChannel())) - 1;
}

/*
 * Init midi clock calculation, when clock slave mode is selected
 */
void SongManager::initMidiClkCalculator(void)
{
	_midiClockCounter.store ( 0 );
	_midiClockCalculationStarted = false;
	_midiClkSlaveModeActive = true;
	_midiClkRxInProgress = false;
}

/*
 * De-Init midi clock calculation, when clock master mode is selected
 */
void SongManager::deInitMidiClkCalculator(void)
{
	_midiClkSlaveModeActive = false;
}

/*
 * Start thread to monitor the incoming midi clock and update the tempo
 */
void SongManager::startMidiClockCalculation(void)
{
	_midiClockCalculationStarted = true;
	_midiClkRxInProgress = true;

	//std::chrono::milliseconds duration(1000);//check every second
	std::thread midiClkCounterThread(midiClockTimerFunc, this);
	midiClkCounterThread.detach();//no resources allocated, detach from caller thread and allow it to return
}
/*
MidiOutput* SongManager::getUsbMidiOutInterface(int8_t portId)
{
	if ((portId >= midiDataIOPorts::usbDevMidi1) && (portId <= midiDataIOPorts::usbDevMidi6))
	{
		auto& outDevice = midiDevList[portId].outputInterface;
		if (outDevice)
		{
			return outDevice.get();
		}
	}
	return nullptr;
}
*/

void SongManager::handleCommandMessage(int id)
{
	if (dm == nullptr)
	{
		dm = new(juce::AudioDeviceManager);
		setUpPlatformAudio();
	}
    if (id == 1) startJuceAudioCallback();
}

void SongManager::init(void)
{
	this->postCommandMessage(0);
	this->engine.instrumentManager->addInstrumentListener(this);//(Dependencies : SongManager and InstrumentManager)
	initAuditioningPreset();
#ifdef ENABLE_THREADS_TO_PLAY
#ifdef __linux__
	/* Start at begining and keep running. Same behaviour as Windows code.
	   Later sync with play start and stop
		- Code already in place, commented.
		- Unblock queue before setting exit flag.
		- Also check on re-distributing cores when play/record, is not happening
	*/
	startPulseProcessingTimers();
#else
	for (int i = 0; i < PLAY_THREAD_TRACK_SIZE; i++)
	{
		switch (i)
		{
			case 0:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_0(); };
				TrackPlayThread_0.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			case 1:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_1(); };
				TrackPlayThread_1.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			case 2:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_2(); };
				TrackPlayThread_2.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			case 3:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_3(); };
				TrackPlayThread_3.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			case 4:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_4(); };
				TrackPlayThread_4.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			case 5:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_5(); };
				TrackPlayThread_5.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			case 6:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_6(); };
				TrackPlayThread_6.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			case 7:
				TrackPlayThreadFunctionCB[i] = [](void* context) { static_cast<SongManager*> (context)->TrackPlayThreadFunction_7(); };
				TrackPlayThread_7.startThread(TrackPlayThreadFunctionCB[i], this);
				break;
			default:
				break;
		}
	}
#endif /* ! __linux__ */
#endif
}
void SongManager::setCvGateVoltage(int cv_channel, float cvVolt, int gate_channel, float gateVolt, bool allcvOff, bool allGateOff)
{
	/* This function may got call from Track threads safe check with mutex*/
	std::lock_guard<std::mutex> lock(cvset_mutex);

	if (allcvOff)
	{
		CVControl::getCVInterface()->setArrayGateVoltage(0.0f);
		CVControl::getCVInterface()->setArrayCvVoltage(0.0f);
	}
	else if (allGateOff)
	{
		CVControl::getCVInterface()->setArrayGateVoltage(0.0f);
	}
	else
	{
		if (cv_channel != -1)
		{
			CVControl::getCVInterface()->setCvVoltage(cv_channel, cvVolt);
		}
		else if (gate_channel != -1)
		{
			CVControl::getCVInterface()->setGateVoltage(gate_channel, gateVolt);
		}
	}
}
#ifdef ENABLE_THREADS_TO_PLAY
#ifdef __linux__
void printSchedData(int thread_id)
{
	int pol=0;
	struct sched_param sp;

	int ret = pthread_getschedparam(pthread_self(), &pol, &sp);
	if(ret != 0)
	{
		printf("Unable to get sched param %d\n", thread_id);
		fflush(stdout);
	}
	else
	{
		printf("Thread %d policy %s prio %d\n", thread_id, (pol == SCHED_FIFO) ? "SCHED_FIFO" : (pol == SCHED_RR) ? "SCHED_RR" : (pol == SCHED_OTHER) ? "SCHED_OTHER" : "UNDEFINED" , sp.sched_priority);
		fflush(stdout); 
	}
}
#endif

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_0(void * arg)
{
	printSchedData(0);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_0.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for (auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_0(void)
{
	TrackPlayThread_0.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_0.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for (auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_1(void * arg)
{
	printSchedData(1);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_1.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for(auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_1(void)
{
	TrackPlayThread_1.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_1.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for(auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_2(void * arg)
{
	printSchedData(2);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_2.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for (auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_2(void)
{
	TrackPlayThread_2.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_2.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for (auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_3(void * arg)
{
	printSchedData(3);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_3.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for (auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_3(void)
{
	TrackPlayThread_3.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_3.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for (auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_4(void * arg)
{
	printSchedData(4);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_4.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for (auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_4(void)
{
	TrackPlayThread_4.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_4.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for (auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_5(void * arg)
{
	printSchedData(5);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_5.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for (auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_5(void)
{
	TrackPlayThread_5.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_5.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for (auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_6(void * arg)
{
	printSchedData(6);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_6.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for (auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_6(void)
{
	TrackPlayThread_6.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_6.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for (auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */

#ifdef __linux__
void *SongManager::TrackPlayThreadFunction_7(void * arg)
{
	printSchedData(7);
	if(SongManager::instance != NULL)
	{
		std::list <TrackAndPosition> trackListToHandle;
		while (SongManager::instance->runPulseProcessingThreads == true)
		{
			SongManager::instance->TrackPlayQ_7.get(trackListToHandle, TimeOut::kWaitForever);
			if (trackListToHandle.size())
			{
				for (auto& it : trackListToHandle)
				{
					static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
				}
			}
		}
	}
}
#else /* __linux__ */
void SongManager::TrackPlayThreadFunction_7(void)
{
	TrackPlayThread_7.delayMs(1000);
	std::list <TrackAndPosition> trackListToHandle;
	while (!CThread::desktopShutdownRequested())
	{
		if (!TrackPlayQ_7.get(trackListToHandle, TimeOut::kWaitForever)) break;
		if (trackListToHandle.size())
		{
			for (auto& it : trackListToHandle)
			{
				static_cast<Track*>(it.trackPtr)->pulse(it.sceneBarId, it.sceneStepId);
			}
		}
	}
}
#endif /* ! __linux__ */
#endif//ENABLE_THREADS_TO_PLAY

void SongManager::setSystemState(KEY_PRESSED key)
{
	auto stateLock = engine.lockDesktopState();
	MidiMessage msg;

	switch (key)
	{
		case KEY_PRESSED::PLAY_KEY:
			if (_state == SYSTEM_STATE::IDLE)
			{
				_state = SYSTEM_STATE::PLAY;
				msg = MidiMessage::midiStart();
			}
			else if (_state == SYSTEM_STATE::PAUSE)
			{
				_state = SYSTEM_STATE::PLAY;
				msg = MidiMessage::midiContinue();
			}
			else if (_state == STEP_RECORD)
			{
				_state = SYSTEM_STATE::LIVE_RECORD;
			}
			else if (_state == RECORD_PAUSE)
			{
				_state = SYSTEM_STATE::LIVE_RECORD;
			}
			//not expected and no action in PLAY / LIVE_RECORD
			break;

		case KEY_PRESSED::PAUSE_KEY:
			if (_state == SYSTEM_STATE::PLAY)
			{
				_state = SYSTEM_STATE::PAUSE;
				msg = MidiMessage::midiStop();
			}
			else if (_state == SYSTEM_STATE::LIVE_RECORD)
			{
				_state = SYSTEM_STATE::RECORD_PAUSE;
			}
			//not expected and no action in IDLE / PAUSE / STEP_RECORD / RECORD_PAUSE
			break;

		case KEY_PRESSED::RECORD_KEY:
			if (_state == SYSTEM_STATE::IDLE)
			{
				_state = SYSTEM_STATE::STEP_RECORD;
			}
			else if (_state == SYSTEM_STATE::PLAY)
			{
				_state = SYSTEM_STATE::LIVE_RECORD;
			}
			else if (_state == SYSTEM_STATE::PAUSE)
			{
				_state = SYSTEM_STATE::RECORD_PAUSE;
			}
			//not expected and no action  STEP_RECORD/ LIVE_RECORD / RECORD_PAUSE
			break;

		case KEY_PRESSED::RECORD_STOP_KEY:
			if (_state == SYSTEM_STATE::STEP_RECORD)
			{
				_state = SYSTEM_STATE::IDLE;
			}
			else if (_state == SYSTEM_STATE::LIVE_RECORD)
			{
				_state = SYSTEM_STATE::PLAY;
			}
			else if (_state == SYSTEM_STATE::RECORD_PAUSE)
			{
				_state = SYSTEM_STATE::PAUSE;
			}
			// not expected and no action  IDLE / PLAY/ PAUSE
			break;

		case KEY_PRESSED::STOP_KEY:

			if (_state == SYSTEM_STATE::PLAY)
			{
				_state = SYSTEM_STATE::IDLE;
				msg = MidiMessage::midiStop();
			}
			else if (_state == SYSTEM_STATE::PAUSE)
			{
				_state = SYSTEM_STATE::IDLE;
			}
			else if (_state == SYSTEM_STATE::STEP_RECORD)
			{
				_state = SYSTEM_STATE::IDLE;
			}
			else if (_state == SYSTEM_STATE::LIVE_RECORD)
			{
				_state = SYSTEM_STATE::IDLE;
			}
			else if (_state == SYSTEM_STATE::RECORD_PAUSE)
			{
				_state = SYSTEM_STATE::IDLE;
			}
			// not expected and no action  IDLE
			break;
	}

	
	sendTransportControlMsg(msg);
	
}
void SongManager::timerCallback()//TODO: not used and invoked - can be removed
{
	//stopTimer();
	if (_handleMidiIn && _isAuditioningOn)//Restart as we have more message recived;
	{
		_handleMidiIn = false;
		startTimer(InternalTimeIntervalMs);
	}
	else if (!_isSongPlaying &&!_isRecording)//stop timers
	{
		stopJuceAudioCallback();
#ifdef USE_INTERNAL_20MS_TIMER
		stopAudioProcessingTimer();
#endif
		for (uint8_t i = 0; i < AUDITIONING_MAX_PORT; i++)
			AuditioningTrack[i]->AuditioningTrackClear();
	}
	// song playing stop only Auditioning timer
}

void SongManager::setAuditioningState(bool state)
{
	_isAuditioningOn = state;
	if (!state)
	{
		if (!_isSongPlaying &&!_isRecording)//stop all timer
		{
			stopJuceAudioCallback();
#ifdef USE_INTERNAL_20MS_TIMER
			stopAudioProcessingTimer();
#endif
			for (uint8_t i = 0; i < AUDITIONING_MAX_PORT; i++)
				AuditioningTrack[i]->AuditioningTrackClear();
		}
		//else// song playing stop only Auditioning timer
			//stopTimer();
	}
	else
	{
#ifdef USE_INTERNAL_20MS_TIMER
		start20msAudioProcessingTimer();
#endif
		startJuceAudioCallback();
		//startTimer(InternalTimeIntervalMs);// this to simulate play and stop
	}
}
void SongManager::initAuditioningPreset()
{
	for (uint8_t i = 0; i < AUDITIONING_MAX_PORT; i++)
	{
		int preset = engine.settingsGeneralCnfg->getAuditioningPortPreset(i);
		setAuditioningPreset(i, preset);
	}
}
void SongManager::setAuditioningPreset(uint8_t portID, int presetindex)
{
	if (presetindex == PRESET_INVALID_ID)
		isAuditioningTrackEnabled[portID] = false;
	else
	{
		AuditioningTrack[portID]->AuditioningTrackSetPreset(presetindex);
		isAuditioningTrackEnabled[portID] = true;
	}
}
void SongManager::HandleMidiInAuditioning(uint8_t portID, const MidiMessage msg)
{
	if (_isAuditioningOn)
	{
		if (!_handleMidiIn)
			setAuditioningState(_isAuditioningOn);//check and enable timers
		if (portID < AUDITIONING_MAX_PORT)
		{
			_handleMidiIn = true;
			sendMidiMessageInternal(msg, AuditioningTrack[portID].get());
		}
	}
}

void SongManager::handleMidiThru(const MidiMessage& message, const std::string portName)
{
	int8_t portId = getPortIdFromName(portName);
	if (portId < 0)
		return;
	MidiConfig _midicnfg = engine.settingsMidiCnfg->getMidiConfigForPortId(portId);

	HandleMidiInAuditioning(portId, message);


	if (_midicnfg.type != SoftThruType::off && _midicnfg.sourcePort == portId)
	{
		MidiMessage msgTosend = message;
		switch (_midicnfg.type)
		{
			case SoftThruType::loopBack:
				sendMidiExternal(msgTosend, portId);
				break;

			case SoftThruType::differentChannelDifferentPort:
				msgTosend.setChannel(_midicnfg.destChannel);
			case SoftThruType::differentPortSameChannel://fallthrough
				portId = _midicnfg.destPort;
				sendMidiExternal(msgTosend, portId);//MCU port ID starts from 1
				break;

			case SoftThruType::off:

			default:
				break;
		}
	}
}

Ignite::Track* SongManager::getPlayTrackFromId (uint8_t trackId)
{
	Track* ret = nullptr;
	auto scene = getPlayScene();

	if (scene != nullptr)
	{
		//It is called only during Step time record
		//Step time recording is not cpu intensive
		//In this case it is okay to loop through all tracks
		//In future if you need to use it somewhere else, consider the loop
		for (auto& track : scene->_sceneData->trackList)
		{
			if (track._trackData->trackId == trackId)
			{
				ret = &track;
				break;
			}
		}
	}

	return ret;
}

void SongManager::setMasterVolume(float volume)
{
	_masterVolume = volume;
}
float SongManager::getMasterVolume()
{
	return _masterVolume;
}
void SongManager::setStepReSelectedStep(uint8_t stepId)
{
	if (_state == SYSTEM_STATE::STEP_RECORD)
	{
		if (activeSong->getEditScene() && activeSong->getEditScene()->editTrack)
			activeSong->getEditScene()->editTrack->setStepReSelectedStep(stepId);
	}
}

/* Send at Play start, Tempo change, scene change, clk ppqn change , clk port change*/
void SongManager::sendMidiClk(void)
{
	sendSyncClkPulses();
	sendMidiClkPackets();
}

void SongManager::sendMidiClkPackets(void)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr))
	{
		return;
	}
	
	//Calculatethe interval
	auto bpm =  activeSong->playScene->getTempo();
	
	auto ret = engine.settingsMidiClkCnfg->getParamSingle(SettingsMidiClkCnfgParam::outputPpqn);
	auto configPpqn = ret.get<int8_t>();
	auto outPpq = mapConfigPpqnToPpqn(configPpqn);

	double duration_ms = (double)60 * 1000 / (bpm * outPpq);

	//Update the interval, used by timer thread
	_clockOutDurationUs = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double, std::milli>(duration_ms));

	//create the thread, if it is not running
	if (_clkOutTimerRunning == false)
	{
		_clkOutTimerRunning = true;
		std::thread midiClkOutTimerThread(midiClkOutTimerFunc, this);
		midiClkOutTimerThread.detach();//no resources allocated, detach from caller thread and allow it to return
	}
}

void SongManager::sendSyncClkPulses(void)
{
	if ((activeSong == nullptr) || (activeSong->playScene == nullptr))
	{
		return;
	}

	//check if midi clk out is enabled
	uint32_t clkOutPorts = (engine.settingsMidiClkCnfg->getParamSingle(SettingsMidiClkCnfgParam::clkOutPort)).get<uint32_t>();
	if (clkOutPorts != 0)
	{
		auto tempo = activeSong->playScene->getTempo();

		auto ret = engine.settingsMidiClkCnfg->getParamSingle(SettingsMidiClkCnfgParam::outputPpqn);

		auto configPpqn = ret.get<int8_t>();

		auto ppqn = mapConfigPpqnToPpqn(configPpqn);

		auto freq = (tempo * ppqn) / 60;

		uint16_t freqTruncated = static_cast<uint16_t>(freq);

		if (engine.settingsMidiClkCnfg->isClkOutEnabledOnSyncPort())
		{
			engine.frontPannelManager->SendSyncOutFreq(freqTruncated);
		}
	}
	else
	{
		stopMidiClk();
	}
}

void SongManager::handleClkOutPpqnChange(void)
{
	sendMidiClk();
}

void SongManager::handleClkOutPortChange(void)
{
	sendMidiClk();
}

void SongManager::stopMidiClk(void)
{

}
/* Other external modules can query the engine system state*/
SYSTEM_STATE SongManager::getSystemState(void)
 {
	return _state;
	}

/* Engine internal use only */

void SongManager::sendTransportControlMsg(MidiMessage msg)
{
	if (msg.isMidiStart() || msg.isMidiContinue() || msg.isMidiStop())
	{
		//broadcasting to all ports
		//TODO : add user controls if required
		for (uint8_t port = midiDataIOPorts::dinMidi1; port < midiDataIOPorts::maxMidiIOPorts; ++port)
		{
			sendMidiExternal(msg, port);
		}
	}
}

void SongManager::setInstance(SongManager *inst)
{
	SongManager::instance = inst;
}

#ifdef __linux__
void SongManager::checkAndOpenMidiHostDevice(void)
{
	//simulate signal
	SongManager::usbHostMidiDevConnectSigReceived = true;
}
#endif /* __linux__ */


void SongManager::sendMidiExternal (MidiMessage msg, uint8_t port)
{
    if(desktopOffline)return;
    auto stateLock = engine.lockDesktopState();
#ifdef _WIN32
    if (desktopMidiOutput && desktopMidiOutput(msg, port)) return;
#endif
	//Imx handles the USB-MIDIs, Virtual-MIDI(test only)

	// DIN MIDI 0 - 4 -> MCU

	// CV 5  - 12

	// USB DEV MIDI 13 - 18 -> IMX

	// USB HOST MIDI 19 -> 23 IMX

	//USB MIDIs
	if ( isUsbMidiPort(port))
	{
		auto& outDevice = midiDevList[port].outputInterface;
		if (outDevice)
		{
			outDevice->sendMessageNow (msg);
		}
	}
	// MCU handles DIN MIDI
	else if (isDinMidiPort(port))
	{
		engine.frontPannelManager->sendMidiMessageExternal (msg, port);
	}
}

bool SongManager::isDinMidiPort(uint8_t port)
{
	if ((port >= midiDataIOPorts::dinMidi1) && (port <= midiDataIOPorts::dinMidi5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SongManager::isUsbMidiPort(uint8_t port)
{
	if (isUsbDeviceMidiPort(port) || isUsbHostMidiPort(port))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SongManager::isUsbDeviceMidiPort(uint8_t port)
{
	if ((port >= midiDataIOPorts::usbDevMidi1) && (port <= midiDataIOPorts::usbDevMidi6))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SongManager::isUsbHostMidiPort(uint8_t port)
{
	if ((port >= midiDataIOPorts::usbHostMidi1) && (port <= midiDataIOPorts::usbHostMidi5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::vector <std::string> SongManager::getUsbHostMidiDeviceList(void)
{
	std::vector <std::string> outDevList;
	std::vector <std::string> inDevList;
	std::vector <std::string> devList;

	auto availableOutDevices = MidiOutput::getAvailableDevices();
	for (auto& midiOutDevice : availableOutDevices)
	{
		//Skip the 6 permanent  usb device port midi devices
		if (strncmp(midiOutDevice.name.toRawUTF8(), IGNITE_USB_D_MIDI_PORT_PREFIX, strlen(IGNITE_USB_D_MIDI_PORT_PREFIX)) != 0)
		{
			outDevList.emplace_back(midiOutDevice.name.toRawUTF8());
		}
	}
	
	auto availableInDevices = MidiInput::getAvailableDevices();
	for (auto& midiInDevice : availableInDevices)
	{
		//Skip the 6 permanent  usb device port midi devices
		if (strncmp(midiInDevice.name.toRawUTF8(), IGNITE_USB_D_MIDI_PORT_PREFIX, strlen(IGNITE_USB_D_MIDI_PORT_PREFIX)) != 0)
		{
			inDevList.emplace_back(midiInDevice.name.toRawUTF8());
		}
	}
	
	//Filter-in devices having both in and out interfaces
	for (int i = 0; i < outDevList.size(); i++)
	{
		for (int j = 0; j < inDevList.size(); j++)
		{
			if (outDevList[i] == inDevList[j])
			{
				devList.push_back(outDevList[i]);
				break;
			}
			else
			{
				continue;
			}
		}
	}

	return devList;
}

std::vector <std::string> SongManager::findNewlyAddedUsbMidiDevices(std::vector <std::string> extDevList)
{
	std::vector <std::string> newDevs;

	for (int i = 0; i < extDevList.size(); i++)
	{
		bool newDevice = true;
		for (auto &savedDevice : midiDevList)
		{
			if (extDevList[i] == savedDevice.second.deviceName)
			{
				newDevice = false;
				break;
			}
			else
			{
				continue;
			}
		}

		if (newDevice == true)
		{
			newDevs.push_back(extDevList[i]);
		}
	}

	return newDevs;
}

std::vector <std::string> SongManager::findRecentlyRemovedUsbMidiDevices(std::vector <std::string> extDevList)
{
	std::vector <std::string> removedDevs;

	for (auto& savedDevice : midiDevList)
	{
		//Ignore permament DIN midi, usb device midi, and empty usb host midi ports
		if 	(	(strncmp (savedDevice.second.deviceName.c_str(), IGNITE_USB_D_MIDI_PORT_PREFIX, strlen(IGNITE_USB_D_MIDI_PORT_PREFIX)) == 0) || 
				(strncmp (savedDevice.second.deviceName.c_str(), IGNITE_DIN_MIDI_PORT_PREFIX, strlen(IGNITE_DIN_MIDI_PORT_PREFIX)) == 0) ||
				(strncmp (savedDevice.second.deviceName.c_str(), IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME, strlen(IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME)) == 0) 
			)
		{
			continue;
		}

		bool removedDevice = true;

		for (int i = 0; i < extDevList.size(); i++)
		{
			if (savedDevice.second.deviceName == extDevList[i])
			{
				removedDevice = false;
				break;
			}
			else
			{
				continue;
			}
		}

		if (removedDevice == true)
		{
			printf("removing device %s\n", savedDevice.second.deviceName.c_str());
			fflush(stdout);
			removedDevs.push_back(savedDevice.second.deviceName);
		}
	}

	return removedDevs;
}

void SongManager::initUsbMidiDevPorts(std::vector <std::string> devNameList)
{
	if (devNameList.size() > 0)
	{
		for (int8_t i = 0; i < IGNITE_MAX_USB_HOST_MIDI_DEVICES; i++)
		{
			int8_t index = static_cast<int8_t>(midiDataIOPorts::usbHostMidi1 + i);

			//check if slot is empty
			if (gMidiPortNameList[index] == IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME)
			{

				gMidiPortNameList[index] = devNameList[0];

				midiDevList[index].deviceName = devNameList[0];

				openUsbMidiPort(index);

				//remove added device from input list
				devNameList.erase(devNameList.begin());

				if (devNameList.size() == 0)
				{
					break;
				}
			}
		}
	}
}

void SongManager::deinitUsbMidiDevPorts(std::vector <std::string> devNameList)
{
	if(devNameList.size() > 0)
	{
		for (int8_t i = 0; i < 6; i++)
		{
			int8_t index = static_cast<int8_t>(midiDataIOPorts::usbHostMidi1 + i);

			if (gMidiPortNameList[index] == devNameList[0])
			{
				gMidiPortNameList[index] = IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME;

				midiDevList.erase(index);

				//remove processed device from input list
				devNameList.erase(devNameList.begin());

				if (devNameList.size() == 0)
				{
					break;
				}
			}
		}
	}
}

//Open both input and output interface
void SongManager::openUsbMidiPort(uint8_t port)
{
	auto availableOutDevices = MidiOutput::getAvailableDevices();
	for (auto& midiOutDevice : availableOutDevices)
	{
		if (strncmp (midiOutDevice.name.toRawUTF8(), midiDevList[port].deviceName.c_str(), strlen(midiDevList[port].deviceName.c_str())) != 0) continue;

		auto outDeviceHandle = &((midiDevList[port]).outputInterface);
		*outDeviceHandle = MidiOutput::openDevice(midiOutDevice.identifier);
		if (outDeviceHandle->get() == nullptr)
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Unable to open Midi Out " + midiOutDevice.name);
			printf("FAILED TO Open MIDI OUT ->%s\n", midiOutDevice.name.toRawUTF8());
			fflush(stdout);
		}
		else
		{
			printf("Open MIDI OUT ->%s\n", midiOutDevice.name.toRawUTF8());
			fflush(stdout);
		}
	}

	auto availableInDevices = MidiInput::getAvailableDevices();
	for (auto& midiInDevice : availableInDevices)
	{
		if (strncmp(midiInDevice.name.toRawUTF8(), midiDevList[port].deviceName.c_str(), strlen(midiDevList[port].deviceName.c_str())) != 0) continue;

		auto midiDeviceHandle = &(midiDevList[port]);
		midiDeviceHandle->inputInterface = MidiInput::openDevice(midiInDevice.identifier, (MidiInputCallback*)midiDeviceHandle);
		if (midiDeviceHandle->inputInterface != nullptr)
		{
			midiDeviceHandle->inputInterface->start();

			printf("Open MIDI IN ->%s\n", midiInDevice.name.toRawUTF8());
			fflush(stdout);
		}
		else
		{
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Unable to open Midi In " + midiInDevice.name);
			printf("FAILED TO Open MIDI IN ->%s\n", midiInDevice.name.toRawUTF8());
			fflush(stdout);
		}
	}
}

void SongManager::closeUsbMidiPort(uint8_t port)
{
	//no JUCE operations found for close
}

std::map<int8_t, std::string> SongManager::getUsbHostPortMidiDeviceNames(void)
{
	std::map<int8_t, std::string> nameListWithIndex;

	for (int8_t i = 0; i < IGNITE_MAX_USB_HOST_MIDI_DEVICES; i++)
	{
		int8_t index = static_cast<int8_t>(midiDataIOPorts::usbHostMidi1 + i);

		//check if slot is empty
		if (gMidiPortNameList[index] != IGNITE_USB_HOST_MIDI_PORT_PLACE_HOLDER_NAME)
		{
			nameListWithIndex.emplace(index, gMidiPortNameList[index]);
		}
	}

	return nameListWithIndex;
}

void SongManager::publishUsbHostMidiChangeNotificationToUI(void)
{
	engine.SendResponse((moduleId_t)ModuleName::moduleSong, (moduleId_t)ModuleName::moduleUI, (paramId_t)SongParam::paramUsbHostMidiChangeNotification, 0);//bool value not used for info exchange
}

#ifdef __linux__
void SongManager::startPulseProcessingTimers(void)
{
	if(runPulseProcessingThreads == true) 
	{
		DBG("Attempting to run multiple instaances of pulse processing threads");
		return;
	}

	runPulseProcessingThreads = true;

	pthread_attr_t attr;
	cpu_set_t cpus;

	int ret = pthread_attr_init(&attr);
	if(ret != 0)
	{
		printf("Attr not init\n");
		fflush(stdout);
	}
	else
	{
		printf("Attr init\n");
		fflush(stdout);
	}

	ret = pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	if(ret != 0)
	{
		printf("POlicy not set\n");
		fflush(stdout);
	}
	else
	{
		printf("Pol init\n");
		fflush(stdout);
	}
/*
	struct sched_param sp = {99};
	ret = pthread_attr_setschedparam(&attr, &sp);
	if(ret != 0)
	{
		printf("Prio not set\n");
		fflush(stdout);
	}
	else
	{
		printf("Prio init\n");
		fflush(stdout);
	}
*/
	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if(ret != 0)
	{
		printf("Explicit sched not set\n");
		fflush(stdout);
	}
	else
	{
		printf("EXP sched init\n");
		fflush(stdout);
	}

	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(ret != 0)
	{
		printf("Detach state not set\n");
		fflush(stdout);
	}
	else
	{
		printf("det state init\n");
		fflush(stdout);
	}

	CPU_ZERO(&cpus);
	CPU_SET(2, &cpus);
	CPU_SET(3, &cpus);
	ret = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	if(ret != 0)
	{
		printf("Affin not set\n");
		fflush(stdout);
	}
	else
	{
		printf("Affin state init\n");
		fflush(stdout);
	}


	ret = pthread_create(&TrackPlayThread_0 , &attr, TrackPlayThreadFunction_0, NULL);
	if(ret != 0)
	{
		printf("Thread 0 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 0 created\n");
		fflush(stdout);
	}

	ret = pthread_create(&TrackPlayThread_1 , &attr, TrackPlayThreadFunction_1, NULL);
	if(ret != 0)
	{
		printf("Thread 1 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 1 created\n");
		fflush(stdout);
	}

	ret = pthread_create(&TrackPlayThread_2 , &attr, TrackPlayThreadFunction_2, NULL);
	if(ret != 0)
	{
		printf("Thread 2 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 2 created\n");
		fflush(stdout);
	}

	ret = pthread_create(&TrackPlayThread_3 , &attr, TrackPlayThreadFunction_3, NULL);
	if(ret != 0)
	{
		printf("Thread 3 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 3 created\n");
		fflush(stdout);
	}

	ret = pthread_create(&TrackPlayThread_4 , &attr, TrackPlayThreadFunction_4, NULL);
	if(ret != 0)
	{
		printf("Thread 4 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 4 created\n");
		fflush(stdout);
	}

	ret = pthread_create(&TrackPlayThread_5 , &attr, TrackPlayThreadFunction_5, NULL);
	if(ret != 0)
	{
		printf("Thread 5 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 5 created\n");
		fflush(stdout);
	}

	ret = pthread_create(&TrackPlayThread_6 , &attr, TrackPlayThreadFunction_6, NULL);
	if(ret != 0)
	{
		printf("Thread 6 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 6 created\n");
		fflush(stdout);
	}

	ret = pthread_create(&TrackPlayThread_7 , &attr, TrackPlayThreadFunction_7, NULL);
	if(ret != 0)
	{
		printf("Thread 7 create failed : %s\n", strerror(ret));
		fflush(stdout);
	}
	else
	{
		printf("Thread 7 created\n");
		fflush(stdout);
	}

	pthread_attr_destroy(&attr);
}

void SongManager::stopPulseProcessingThreads(void)
{
	printf("Stop pulse processing threads\n");
	runPulseProcessingThreads = false;
}
#endif /* __linux__ */

float SongManager::getTickDivider (TimeBase tb)
{
	const float base1divider = 1 / 4.0f;
	float divider = base1divider;

	switch (tb)
	{
		case TimeBase::Base_1:
			divider = base1divider;
			break;
		case TimeBase::Base_2:
			divider = base1divider * 2;
			break;
		case TimeBase::Base_4:
			divider = base1divider * 4;
			break;
		case TimeBase::Base_8:
			divider = base1divider * 8;
			break;
		case TimeBase::Base_16:
			divider = base1divider * 16;
			break;
		case TimeBase::Base_32:
			divider = base1divider * 32;
			break;
		/*
		case TimeBase::Base_64:
			divider = base1divider * 64;
			break;
		*/
		case TimeBase::Base_2T:
			divider = base1divider * 2 * (3.0f / 2);
			break;
		case TimeBase::Base_4T:
			divider = base1divider * 4 * (3.0f / 2);
			break;
		case TimeBase::Base_8T:
			divider = base1divider * 8 * (3.0f / 2);
			break;
		case TimeBase::Base_16T:
			divider = base1divider * 16 * (3.0f / 2);
			break;
		case TimeBase::Base_32T:
			divider = base1divider * 32 * (3.0f / 2);
			break;
		/*
		case TimeBase::Base_64T:
			divider = base1divider * 64 * (3.0f / 2);
			break;
		*/
		default:
			break;
	}
	return divider;
}

uint16_t SongManager::getPulsesPerStep(TimeBase tb)
{
	auto divider = getTickDivider (tb);
	return static_cast<uint16_t> ((SET_PPQN / divider));
}

juce::File SongManager::desktopSettingsFile()
{
    auto root = juce::SystemStats::getEnvironmentVariable("IGNITE_DATA_DIR", "");
    if (root.isEmpty()) root = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("BehringerIgnite").getFullPathName();
    return juce::File(root).getChildFile("desktop.json");
}
std::shared_ptr<void> SongManager::acquireDesktopSoundFont(const std::string& path,std::string& error)
{
    auto lock=engine.lockDesktopState();
    const juce::File file(juce::String::fromUTF8(path.c_str()));
    const auto key=file.getFullPathName().toStdString();
    if(auto found=desktopFontCache[key].lock()) {error.clear();return found;}
    juce::FileInputStream input(file);
    if(!input.openedOk()){error="Cannot open bank: "+path;return {};}
    tsf_stream stream{};stream.data=&input;
    stream.read=[](void* data,void* out,unsigned int count)->int{return static_cast<juce::FileInputStream*>(data)->read(out,int(count));};
    stream.skip=[](void* data,unsigned int count)->int{auto& input=*static_cast<juce::FileInputStream*>(data);return input.setPosition(input.getPosition()+count)?1:0;};
    auto* font=tsf_load(&stream);
    if(!font || tsf_get_presetcount(font)==0){tsf_close(font);error="The file contains no usable SoundFont presets.";return {};}
    tsf_set_output(font,Track::audioChannels==2?TSF_STEREO_UNWEAVED:TSF_MONO,INTERNAL_AUDIO_SAMPLE_RATE,0);
    auto result=std::shared_ptr<void>(font,[](void* data){tsf_close(static_cast<tsf*>(data));});
    desktopFontCache[key]=result;error.clear();return result;
}

std::string SongManager::soundFontPathForInstrument(Instrument* instrument) const
{
    return instrument && !instrument->_instrumentData->soundFontPath.empty()?instrument->_instrumentData->soundFontPath:desktopSoundFontPath;
}

std::shared_ptr<void> SongManager::soundFontForInstrument(Instrument* instrument,std::string& error)
{
    if(!instrument || instrument->_instrumentData->soundFontPath.empty()) {error=desktopDefaultFont?"":"No default SoundFont is loaded.";return desktopDefaultFont;}
    return acquireDesktopSoundFont(instrument->_instrumentData->soundFontPath,error);
}

std::vector<SongManager::SoundFontPreset> SongManager::getDesktopSoundFontPresets()
{
    auto lock=engine.lockDesktopState();std::vector<SoundFontPreset> presets;
    auto* font=static_cast<tsf*>((desktopBrowserFont?desktopBrowserFont:desktopDefaultFont).get());
    if(font)for(int index=0;index<tsf_get_presetcount(font);++index)
        presets.push_back({tsf_get_presetbank(font,index),tsf_get_presetnumber(font,index),index,tsf_get_presetname(font,index)});
    return presets;
}

std::string SongManager::desktopPresetName(int value,const std::string& instrumentName)
{
    auto lock=engine.lockDesktopState();if(!SoundFontProgram::isCustom(value))return {};
    auto* instrument=engine.instrumentManager->getInstrument(instrumentName.empty()?engine.instrumentManager->desktopActiveName():instrumentName);
    std::string error;auto handle=soundFontForInstrument(instrument,error);auto* font=static_cast<tsf*>(handle.get());
    const auto* name=font?tsf_bank_get_presetname(font,SoundFontProgram::bank(value),SoundFontProgram::program(value)):nullptr;
    return name?name:"Preset unavailable in assigned bank";
}

bool SongManager::browseDesktopSoundFont(const std::string& path,std::string& error)
{
    auto font=acquireDesktopSoundFont(path,error);if(!font)return false;
    auto lock=engine.lockDesktopState();desktopBrowserFont=std::move(font);
    desktopBrowserFontPath=juce::File(juce::String::fromUTF8(path.c_str())).getFullPathName().toStdString();
    return true;
}

bool SongManager::loadDesktopSoundFont(const std::string& path,std::string& error)
{
    auto font=acquireDesktopSoundFont(path,error);if(!font)return false;
    auto lock=engine.lockDesktopState();
    if(engine.songManager==this)
    {
        if(activeSong){stopRecord();stopSong();setSystemState(KEY_PRESSED::STOP_KEY);}
        stopJuceAudioCallback();
    }
    if(activeSong)for(auto& scene:activeSong->_songData->sceneList)for(auto& track:scene._sceneData->trackList)track.unloadDesktopSoundFont();
    for(auto& track:AuditioningTrack)if(track)track->unloadDesktopSoundFont();
    desktopDefaultFont=font;desktopBrowserFont=std::move(font);_songTSf=desktopDefaultFont.get();
    desktopSoundFontPath=desktopBrowserFontPath=juce::File(juce::String::fromUTF8(path.c_str())).getFullPathName().toStdString();
    for(auto& track:AuditioningTrack)if(track)track->AuditioningTrackInit();
    juce::Logger::writeToLog("SoundFont loaded: "+juce::String::fromUTF8(desktopSoundFontPath.c_str())+" ("+juce::String(tsf_get_presetcount(static_cast<tsf*>(_songTSf)))+" presets)");
    return true;
}

void SongManager::desktopInstrumentFontChanged(const std::string& name)
{
    auto lock=engine.lockDesktopState();
    if(activeSong)for(auto& scene:activeSong->_songData->sceneList)for(auto& track:scene._sceneData->trackList)
        if(track.getTrackTxInstrument() && track.getTrackTxInstrument()->getName()==name)track.unloadDesktopSoundFont();
}

bool SongManager::assignDesktopSoundFont(const std::string& name,const std::string& path,int program,std::string& error)
{
    auto lock=engine.lockDesktopState();auto* instrument=engine.instrumentManager->getInstrument(name);
    if(!instrument){error="Select an existing instrument.";return false;}
    auto font=path.empty()?desktopDefaultFont:acquireDesktopSoundFont(path,error);
    if(!font){if(error.empty())error="No default bank is loaded.";return false;}
    const int bank=SoundFontProgram::isCustom(program)?SoundFontProgram::bank(program):(program>127?128:0);
    const int number=SoundFontProgram::isCustom(program)?SoundFontProgram::program(program):(program>127?Track::getTsfPresetFromIgniteIndex(program):program);
    if(tsf_get_presetindex(static_cast<tsf*>(font.get()),bank,number)<0){error="That preset is unavailable in the selected bank.";return false;}
    if(activeSong){stopRecord();stopSong();setSystemState(KEY_PRESSED::STOP_KEY);}
    auto command=[&](InstrumentParam param,AnyType value){return engine.SetCommand(moduleId_t(ModuleName::moduleUI),moduleId_t(ModuleName::moduleInstrument),paramId_t(param),value);};
    command(InstrumentParam::paramSetActiveInstrument,name);
    const auto canonical=path.empty()?std::string{}:juce::File(juce::String::fromUTF8(path.c_str())).getFullPathName().toStdString();
    command(InstrumentParam::paramDesktopSoundFontPath,canonical);
    command(InstrumentParam::paramInternalSound,true);command(InstrumentParam::paramInternalSoundPgm,program);
    if(!engine.instrumentManager->SaveInstrument(name)){error="Assignment changed, but saving failed. Keep Ignite open and retry Save.";return false;}
    error.clear();return true;
}

void SongManager::receiveDesktopMidi(uint8_t port, const MidiMessage& message)
{
    auto lock = engine.lockDesktopState();
    if (CThread::desktopShutdownRequested() || gMidiPortNameList.find(int8_t(port)) == gMidiPortNameList.end()) return;
    ++desktopMidiReceived;
    const auto name = getPortNameFromId(int8_t(port));
    if (midiRemoteControlEnabled(name)) handleMidiRemoteControlMessage(message);
    else if (midiTransposeControlEnabled(name)) handleMidiTransposeControlMessage(message);
    else if (midiMachineControlEnabled(name)) handleMidiMachineControlMessage(message);
    else handleInMidiMessage(message, name);
    if (desktopMonitorEnabled && !_isAuditioningOn && activeSong && activeSong->editScene && activeSong->editScene->editTrack)
    {
        auto* track = activeSong->editScene->editTrack;
        auto* input = track->getTrackRxInstrument();
        auto* output = track->getTrackTxInstrument();
        if (input && output && output->getInternalSound() && input->getMidiPort() == int8_t(port) &&
            message.getChannel() == input->getMidiChannel() + 1)
        {
            track->prepareDesktopMonitor();
            sendMidiMessageInternal(message, track);
            if (message.isNoteOn()) postCommandMessage(1);
        }
    }
}
void SongManager::renderDesktopMonitor(AudioDataType* buffer)
{
#ifdef _WIN32
    if (!desktopMonitorEnabled || _isAuditioningOn || _state == SYSTEM_STATE::PLAY || _state == SYSTEM_STATE::LIVE_RECORD ||
        !activeSong || !activeSong->editScene || !activeSong->editScene->editTrack) return;
    auto* track = activeSong->editScene->editTrack;
    if (activeSong->playScene == activeSong->editScene) return; // already rendered by the normal mix
    if (!track->getTrackTxInstrument() || !track->getTrackTxInstrument()->getInternalSound()) return;
    track->sfSynthesizeAudio();
    juce::AudioBuffer<float> output(&buffer, 1, INTERNAL_AUDIO_20MS_BLOCK_SIZE);
    output.addFrom(0, 0, track->audioBuff, 0, 0, INTERNAL_AUDIO_20MS_BLOCK_SIZE, _masterVolume);
#endif
}
void SongManager::desktopPanic()
{
    auto lock = engine.lockDesktopState();
    if (activeSong)
        for (auto& scene : activeSong->_songData->sceneList)
            for (auto& track : scene._sceneData->trackList) track.sfReset();
    for (auto& track : AuditioningTrack) if (track) track->sfReset();
}
bool SongManager::desktopPreviewPreset(int presetIndex)
{
    auto lock = engine.lockDesktopState();
    if (_state != SYSTEM_STATE::IDLE || !(desktopBrowserFont?desktopBrowserFont:desktopDefaultFont) || presetIndex < 0 ||
        presetIndex >= tsf_get_presetcount(static_cast<tsf*>((desktopBrowserFont?desktopBrowserFont:desktopDefaultFont).get()))) return false;
    AuditioningTrack[0]->useDesktopSoundFont(desktopBrowserFont?desktopBrowserFont:desktopDefaultFont);
    setAuditioningPreset(0, presetIndex);
    setAuditioningState(true);
    HandleMidiInAuditioning(0, juce::MidiMessage::noteOn(1, 60, uint8_t(100)));
    return true;
}
void SongManager::desktopPreviewNoteOff()
{
    auto lock = engine.lockDesktopState();
    HandleMidiInAuditioning(0, juce::MidiMessage::noteOff(1, 60));
}
void SongManager::desktopStopPreview()
{
    auto lock = engine.lockDesktopState();
    setAuditioningState(false);
}
