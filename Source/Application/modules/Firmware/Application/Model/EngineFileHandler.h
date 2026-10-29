#include "common_defs/source/EngineDataStruct.h"
#include <filesystem>

namespace Ignite
{
	class IgniteEngine;
	class EngineFileHandler
	{
	public:
		EngineFileHandler (IgniteEngine& model);
		EngineFileHandler (const EngineFileHandler&) = delete;
		EngineFileHandler& operator= (const EngineFileHandler&) = delete;
		EngineFileHandler (EngineFileHandler&&) = delete;
		EngineFileHandler& operator= (EngineFileHandler&&) = delete;

		~EngineFileHandler () = default;

		uint8_t securePowerOffStatus = 0;
		uint8_t isBackupToBeDone = 0;
		uint8_t failBootCount = 0;

		void SetSystemStatus(uint8_t _securePowerOffStatus, uint8_t _isBackupToBeDone, uint8_t _failBootCount);
		bool CreateBackUP(uint8_t DataFolderId);
		bool RestoreBackUP(uint8_t DataFolderId);
		FileHandlerError updateMainInfo (mainInfoData info);
		int32_t getMainInfo (mainInfoData& info);

		FileHandlerError CreateSong (SongData songData);
		FileHandlerError DeleteSong (std::string SongName);
		FileHandlerError UpdateSongInfo (SongData& songData);
		SongData GetSongInfo (std::string SongName);
		uint16_t GetNumberofSongs ();
		FileHandlerError RenameSong (std::string SongName, std::string SongNewName);
		std::vector<std::string> GetSongNameList (uint16_t StartPos, uint16_t Count);
		FileHandlerError DeleteAllSongs ();

		FileHandlerError CreateScene (std::string SongName, SceneData sceneData);
		FileHandlerError DeleteScene (std::string SongName, std::string SceneName);
		FileHandlerError UpdateSceneInfo (std::string SongName, SceneData sceneData);
		SceneData GetSceneInfo (std::string SongName, std::string SceneName);
		uint16_t GetNumberofScene (std::string SongName);
		FileHandlerError RenameScene (std::string SongName, std::string SceneName, std::string SceneNewName);
		std::vector<std::string> GetSceneList (std::string SongName, uint16_t StartPos, uint16_t Count);

		FileHandlerError CreateTracksWithList (std::string SongName, std::string SceneName, std::vector<TrackData> TrackDataList);
		FileHandlerError DeleteTrackList (std::string SongName, std::string SceneName);
		FileHandlerError DeleteTrack (std::string SongName, std::string SceneName, std::string TrackName);
		std::vector<TrackData> GetTrackDataList (std::string SongName, std::string SceneName, int8_t numberOfTracks);
		uint16_t GetNumberofTracks (std::string SongName, std::string SceneName);
		FileHandlerError RenameTrack (std::string SongName, std::string SceneName, std::string TrackName, std::string TrackNewName);
		std::vector<std::string> GetTrackList (std::string SongName, std::string SceneName, uint16_t StartPos, uint16_t Count);

		FileHandlerError Createpattern (std::string PatternName, PatternData patternData);
		void GetpatternDataHeader (std::string PatternName, PatternData* patternData);
		PatternData ReadPatternFile (std::string PatternName, bool headerOnly, Step* stepClass);
		std::list<StepData> GetpatternBarStepData (std::string PatternName, uint16_t BarId, uint16_t StepCount);
		std::list<std::string> GetPatternNameList ();
		std::list<BarData> GetPatternsBarList (std::string PatternName);
		std::list<std::string> GetPatternsStepList (std::string PatternName, uint16_t BarId);
		FileHandlerError RenamePattern (std::string PatternName, std::string PatternNewName);
		FileHandlerError DeletePattern (std::string PatternName);
		FileHandlerError updatePatternInfo (std::string patternName, PatternData patternData);
		FileHandlerError UpdatePatternBarInfo (std::string PatternName, BarData& _barData);
		FileHandlerError DeleteAllPattern ();

		FileHandlerError saveInstrument (std::string InstrumentName, InstrumentData instrumentData);
		InstrumentData GetInstrumentData (std::string InstrumentName);
		FileHandlerError DeleteInstrument (std::string InstrumentName);
		uint16_t GetNumberofInstruments ();
		std::vector<std::string> getInstrumentNameList ();
		FileHandlerError RenameInstrument (std::string instrumentName, std::string instrumentNewName);
		FileHandlerError DeleteAllInstrument ();

		// Settings -->
		FileHandlerError UpdateMidiCnfgSetting (SettingsMidiCnfgData settingsMidiCnfgData, std::string SongName);
		std::list <MidiConfig> GetMidiCnfgSetting (std::string SongName);
		FileHandlerError UpdateCvioCnfgSetting(SettingsCvioCnfgData settingsCvioCnfgData, std::string SongName);
		FileHandlerError UpdateCalibData (Array <CalibrationData>*);
		SettingsCvioCnfgData GetCvioCnfgSetting(std::string SongName);
		Array <CalibrationData> * GetCalibData();

		FileHandlerError UpdateSettingsGeneralCnfgData (SettingsGeneralCnfgData settingsCvioCnfgData);
		bool GetGeneralCnfgDataSetting (SettingsGeneralCnfgData* settingsGeneralCnfgData);
		FileHandlerError UpdateSettingsSongCnfgData (SettingsSongCnfgData settingsSongCnfgData, std::string SongName);
		bool GetSettingsSongCnfgDataSetting (SettingsSongCnfgData* settingsSongCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsSceneCnfgData (SettingsSceneCnfgData settingsSceneCnfgData, std::string SongName);
		bool GetSettingsSceneCnfgDataSetting (SettingsSceneCnfgData* settingsSceneCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsTrackCnfgData (SettingsTrackCnfgData settingTrackCnfgData, std::string SongName);
		bool GetSettingsTrackCnfgDataSetting (SettingsTrackCnfgData* settingTrackCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsTrackInstrumentMixerData (SettingsTrackInstrumentMixerData settingsTrackInstrumentMixerData, std::string SongName);
		bool GetSettingsTrackInstrumentMixerDataSetting (SettingsTrackInstrumentMixerData* settingsTrackInstrumentMixerData, std::string SongName);
		FileHandlerError UpdateSettingsPatternCnfgData (SettingsPatternCnfgData settingsPatternCnfgData, std::string SongName);
		bool GetSettingsPatternCnfgDataSetting (SettingsPatternCnfgData* settingsPatternCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsRecordCnfgData (SettingsRecordCnfgData settingsRecordCnfgData, std::string SongName);
		bool GetSettingsRecordCnfgDataSetting (SettingsRecordCnfgData* settingsRecordCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsBRayCnfgData (SettingsBRayCnfgData settingsBRayCnfgData, std::string SongName);
		bool GetSettingsBRayCnfgDataSetting (SettingsBRayCnfgData* settingsBRayCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsRemoteControlCnfgData (SettingsRemoteControlCnfgData settingsRemoteControlCnfgData, std::string SongName);
		bool GetSettingsRemoteControlCnfgDataSetting (SettingsRemoteControlCnfgData* settingsRemoteControlCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsMidiMachineControlCnfgData (SettingsMidiMachineControlCnfgData settingsMidiMachineControlCnfgData, std::string SongName);
		bool GetSettingsMidiMachineControlCnfgDataSetting (SettingsMidiMachineControlCnfgData* settingsMidiMachineControlCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsSculptModeCnfgData (SettingsSculptModeCnfgData settingsSculptModeCnfgData, std::string SongName);
		bool GetSettingsSculptModeCnfgDataSetting (SettingsSculptModeCnfgData* settingsSculptModeCnfgData, std::string SongName);
		FileHandlerError UpdateSettingsUdsCnfgData (SettingsUdsCnfgData settingsUdsCnfgData, std::string SongName);
		bool GetSettingsUdsCnfgDataSetting (SettingsUdsCnfgData* settingsUdsCnfgData, std::string SongName);
		FileHandlerError updateSettingsMidiClkCnfgData (SettingsMidiClkCnfgData settingsMidiClkCnfgData, std::string SongName);
		bool getSettingsMidiClkCnfgDataSetting (SettingsMidiClkCnfgData* settingsMidiClkCnfgData, std::string SongName);
		FileHandlerError updateSettingsTransposeControlCnfgData (TransposeControlCnfgData* data, std::string songName);
		bool getSettingsTransposeControlCnfgData (TransposeControlCnfgData* data, std::string songName);
		void mountDataPart(bool isReadOnly);
		// Settings <--

		bool isGetDataCorrupted() { return isDataCorrupted;};

	private:
		bool writeXmlAtomic(const std::filesystem::path&,const juce::XmlElement&);
		FileHandlerError writeSongInfo(const std::filesystem::path&, const SongData&);
		FileHandlerError writeScene(const std::filesystem::path&, const SceneData&);
		FileHandlerError writeSceneInfo(const std::filesystem::path&, const SceneData&);
		FileHandlerError writeTracks(const std::filesystem::path&, const std::vector<TrackData>&);
		FileHandlerError writePattern(const std::filesystem::path&, const PatternData&);
		IgniteEngine& engine;
		bool isDataCorrupted = false;
		bool GetSystemStatus(void);
		std::string getBackUPPathFromId(uint8_t DataFolderId);
	};
}
