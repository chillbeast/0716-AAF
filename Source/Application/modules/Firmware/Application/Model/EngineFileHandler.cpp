#include <fstream>
#include "SafeFileIO.h"
#include "SavedDataValidation.h"
#include <filesystem>
#include <vector>

using namespace Ignite;
namespace fs = std::filesystem;

//std::string PATH_TO_ROOT_FD = "/root_path";
#ifdef _WIN64
std::string PATH_TO_ROOT_FD = juce::SystemStats::getEnvironmentVariable("IGNITE_DATA_DIR",
	juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
		.getChildFile("BehringerIgnite").getFullPathName()).toStdString();
std::string PATH_TO_CALIB_ROOT_FD = (fs::path(PATH_TO_ROOT_FD) / "calibration").string();
#elif __APPLE__
std::string PATH_TO_ROOT_FD = "/tmp/Ignite_Files";
std::string PATH_TO_CALIB_ROOT_FD = "/tmp/calibration";
const fs::path root_path_backup_external{ "/tmp/flash" };
#else
std::string PATH_TO_ROOT_FD = "/data/ignite/Ignite_Files";
std::string PATH_TO_CALIB_ROOT_FD = "/factory/calibration";
const fs::path root_path_backup_external{ "/mnt/flash" };
#endif

#define FILE_VERSION 0 // Current file version

std::string INFO_FILE_NAME = "info";
std::string FILE_FORMAT = ".txt";

std::string SONG_FOLDER_NAME = "Songs";
std::string INSTRUMENTS_FOLDER_NAME = "Instruments";
std::string PATTERNS_FOLDER_NAME = "Patterns";
std::string SETTINGS_FOLDER_NAME = "Settings";

std::string SYM_STATUS_NAME = "symstatus";
std::string MIDICNFG_FILE_NAME = "MidiCnfg";
std::string CVIOCNFG_FILE_NAME = "CvioCnfg";
std::string SETTING_FILE_NAME = "SongSettings";
std::string SETTINGCNFG_FILE_NAME = "SettingCnfg";
std::string SETTING_SONG_CNFG_FILE_NAME = "SettingSongCnfg";
std::string SETTING_SCENE_CNFG_FILE_NAME = "SettingSceneCnfg";
std::string SETTING_TRACK_CNFG_FILE_NAME = "SettingTrackCnfg";
std::string SETTING_TRACK_MIXER_CNFG_FILE_NAME = "SettingTrackMixerCnfg";
std::string SETTING_PATTERN_CNFG_FILE_NAME = "SettingPatternCnfg";
std::string SETTING_RECORD_CNFG_FILE_NAME = "SettingRecordCnfg";
std::string SETTING_BRAY_CNFG_FILE_NAME = "SettingBRayCnfg";
std::string SETTING_REMOTE_SETTING_CNFG_FILE_NAME = "SettingRemoteCntrlCnfg";
std::string SETTING_REMOTE_SETTING_SCLUPT_FILE_NAME = "SettingSculptCntrlCnfg";
std::string SETTING_UDS_SETTING_FILE_NAME = "SettingsUdsCnfg";
std::string SETTING_MIDI_CLK_CNFG_FILE_NAME = "SettingsMidiClkCnfg";
std::string SETTING_MIDI_MACHINE_CNFG_FILE_NAME = "SettingMidiMachineCntrlCnfg";
std::string SETTING_MIDI_XPOSE_CNFG_FILE_NAME = "SettingMidiXposeCnfg";
std::string CALIBRATION_OFFSET_FILE_NAME = "CalibrationOffset";

const fs::path root_path{ PATH_TO_ROOT_FD };
const fs::path calib_root_path{ PATH_TO_CALIB_ROOT_FD };
const fs::path root_work_path{ root_path / "Ignite_folder" };
const fs::path root_path_backup_internal{ root_path / "Ignite_folder_backup" };

const fs::path songs_path{ root_work_path / SONG_FOLDER_NAME };
const fs::path instruments_path{ root_work_path / INSTRUMENTS_FOLDER_NAME };
const fs::path patterns_path{ root_work_path / PATTERNS_FOLDER_NAME };
const fs::path settings_path{ root_work_path / SETTINGS_FOLDER_NAME };

bool EngineFileHandler::writeXmlAtomic(const fs::path& path,const juce::XmlElement& xml)
{
    auto stateLock=engine.lockDesktopState();
    const auto data=xml.toString().toStdString();
    const bool saved=SafeFileIO::writeFile(path,[&](std::ostream& stream) {stream<<data;});
    if(!saved) engine.desktopChanges.writeFailed();
    else
    {
        const std::pair<std::string,int> settings[]{
            {MIDICNFG_FILE_NAME,9},{CVIOCNFG_FILE_NAME,10},{SETTINGCNFG_FILE_NAME,11},
            {SETTING_SONG_CNFG_FILE_NAME,12},{SETTING_SCENE_CNFG_FILE_NAME,13},{SETTING_TRACK_CNFG_FILE_NAME,14},
            {SETTING_PATTERN_CNFG_FILE_NAME,16},{SETTING_RECORD_CNFG_FILE_NAME,17},{SETTING_BRAY_CNFG_FILE_NAME,18},
            {SETTING_REMOTE_SETTING_CNFG_FILE_NAME,19},{SETTING_REMOTE_SETTING_SCLUPT_FILE_NAME,20},
            {SETTING_UDS_SETTING_FILE_NAME,21},{SETTING_MIDI_CLK_CNFG_FILE_NAME,22},
            {SETTING_MIDI_XPOSE_CNFG_FILE_NAME,23},{SETTING_MIDI_MACHINE_CNFG_FILE_NAME,24}};
        for(const auto& setting:settings) if(path.filename()==setting.first+FILE_FORMAT)
            engine.desktopSaved("settings:"+std::to_string(setting.second));
    }
    return saved;
}

#ifdef _WIN64
const fs::path root_path_backup_external{ root_path / "Ignite_folder_backup" };// external and internal same for windows to avoid error
#endif

Array <CalibrationData> newCalibData;

EngineFileHandler::EngineFileHandler(IgniteEngine& model) :engine(model)
{
	exists(root_path) ? 0 : fs::create_directories(root_path);
#ifdef _WIN32
    for (const auto& recovery : {root_path/".restore-save",root_path/".desktop-restore"})
    {
        const auto previous=recovery/"backups"/root_work_path.filename();
        if (!fs::exists(root_work_path) && fs::is_directory(previous)) fs::rename(previous,root_work_path);
    }
#endif
	exists(root_work_path) ? 0 : fs::create_directory(root_work_path);
	exists(songs_path) ? 0 : fs::create_directory(songs_path);
    for (const auto& library : {std::make_pair(songs_path, root_work_path / ".song-save"),
                                std::make_pair(patterns_path, root_work_path / ".pattern-save")})
    {
        fs::create_directories(library.first);
        const auto recovery = library.second / "backups";
        if (fs::exists(recovery))
            for (const auto& entry : fs::directory_iterator(recovery))
                if (entry.is_directory() && !fs::exists(library.first / entry.path().filename()))
                    fs::rename(entry.path(), library.first / entry.path().filename());
    }
    const auto sceneRecovery = root_work_path / ".scene-save";
    if (fs::exists(sceneRecovery))
        for (const auto& song : fs::directory_iterator(sceneRecovery))
        {
            const auto target = songs_path / song.path().filename();
            const auto backups = song.path() / "backups";
            if (fs::is_directory(target) && fs::is_directory(backups))
                for (const auto& scene : fs::directory_iterator(backups))
                    if (scene.is_directory() && !fs::exists(target / scene.path().filename()))
                        fs::rename(scene.path(), target / scene.path().filename());
        }
	exists(instruments_path) ? 0 : fs::create_directory(instruments_path);
	exists(patterns_path) ? 0 : fs::create_directory(patterns_path);
	exists(settings_path) ? 0 : fs::create_directory(settings_path);
	newCalibData.add({});
	newCalibData.add({});
	newCalibData.add({});
	newCalibData.add({});
	newCalibData.add({});
	newCalibData.add({});
	newCalibData.add({});
	newCalibData.add({});
	if (!exists(calib_root_path))
	{
		mountDataPart(false);
		fs::create_directory(calib_root_path);
		UpdateCalibData(&newCalibData);
		mountDataPart(true);
	}
	GetSystemStatus();//Read and update the data

	if (isBackupToBeDone)
		CreateBackUP(0);
}

void EngineFileHandler::mountDataPart(bool isReadOnly)
{
#if defined( __linux__ ) || defined( __linux ) || defined( linux )
	isReadOnly ? system("mount -o remount,ro /factory") : system("mount -o remount,rw /factory");
	system("sync");
#endif
}

bool EngineFileHandler::GetSystemStatus(void)
{
	int8_t version = 0;
	bool handleFailure = true;
	if (exists(root_path / (SYM_STATUS_NAME + FILE_FORMAT)))
	{
		fs::path filepath = { root_path / (SYM_STATUS_NAME + FILE_FORMAT) };
		juce::File infofile(filepath.string());

		if (!infofile.exists())
			return false;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml)
			{
				//handleFailure
				// struct has default data
			}
			else if (DataXml->hasTagName("SyStemStatus"))
			{
				for (auto* e : DataXml->getChildIterator())
				{
					if (e->hasTagName("Info"))
					{
						version = (uint8_t)e->getIntAttribute("FILE_VERSION");
						securePowerOffStatus = (uint8_t)e->getIntAttribute("securePowerOffStatus");
						isBackupToBeDone = (uint8_t)e->getIntAttribute("isBackupToBeDone");
						failBootCount = (uint8_t)e->getIntAttribute("failBootCount");
					}
				}
				handleFailure = false;
			}
		}
		catch (...) {
			//handleFailure
		}
	}
	if (handleFailure)
	{
		isDataCorrupted = true;
		SetSystemStatus(0, 0, 0); //Create with default data
	}
	return 0;
}
void EngineFileHandler::SetSystemStatus(uint8_t _securePowerOffStatus, uint8_t _isBackupToBeDone, uint8_t _failBootCount)
{
	if (exists(root_work_path / (SYM_STATUS_NAME + FILE_FORMAT)))
	{
		const auto savePath=root_path / (SYM_STATUS_NAME + FILE_FORMAT);
		XmlElement DataXml("SyStemStatus");
		XmlElement* DataInfo = new XmlElement("Info");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("securePowerOffStatus", _securePowerOffStatus);
		DataInfo->setAttribute("isBackupToBeDone", _isBackupToBeDone);
		DataInfo->setAttribute("failBootCount", _failBootCount);
		DataXml.addChildElement(DataInfo);
		writeXmlAtomic(savePath,DataXml);
	}
}
std::string EngineFileHandler::getBackUPPathFromId(uint8_t DataFolderId)
{
	fs::path backupPath;
	switch (EngineDataFolderId(DataFolderId))
	{
	case EngineDataFolderId::AllDataInternalStorage:
	case EngineDataFolderId::SongsInternalStorage:
	case EngineDataFolderId::PatternInternalStorage:
	case EngineDataFolderId::InstrumentInternalStorage:
	case EngineDataFolderId::SettingsInternalStorage:
		backupPath = root_path_backup_internal;
		break;

	case EngineDataFolderId::AllDataExternalStorage:
	case EngineDataFolderId::SongsExternalStorage:
	case EngineDataFolderId::PatternExternalStorage:
	case EngineDataFolderId::InstrumentExternalStorage:
	case EngineDataFolderId::SettingsExternalStorage:
		backupPath = root_path_backup_external;
		break;

	case EngineDataFolderId::MaxValue:
	default:
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN FolderId out of range " + String(DataFolderId));
		break;
	}
	return (backupPath.string());
}
#ifdef _WIN32
static std::vector<std::string> desktopBackupLibraries(uint8_t folder)
{
    switch (EngineDataFolderId(folder))
    {
        case EngineDataFolderId::AllDataInternalStorage: case EngineDataFolderId::AllDataExternalStorage:
            return {SONG_FOLDER_NAME, INSTRUMENTS_FOLDER_NAME, PATTERNS_FOLDER_NAME, SETTINGS_FOLDER_NAME};
        case EngineDataFolderId::SongsInternalStorage: case EngineDataFolderId::SongsExternalStorage: return {SONG_FOLDER_NAME};
        case EngineDataFolderId::PatternInternalStorage: case EngineDataFolderId::PatternExternalStorage: return {PATTERNS_FOLDER_NAME};
        case EngineDataFolderId::InstrumentInternalStorage: case EngineDataFolderId::InstrumentExternalStorage: return {INSTRUMENTS_FOLDER_NAME};
        case EngineDataFolderId::SettingsInternalStorage: case EngineDataFolderId::SettingsExternalStorage: return {SETTINGS_FOLDER_NAME};
        default: return {};
    }
}
static bool desktopCopyBackup(const fs::path& source, const fs::path& destination,
                              const std::vector<std::string>& libraries, bool restoring)
{
    if (libraries.empty() || !fs::is_directory(source)) return false;
    for (const auto& name : libraries)
    {
        const auto folder=source/name;
        if (!fs::is_directory(folder)) return false;
        for (const auto& entry : fs::recursive_directory_iterator(folder))
        {
            if (entry.is_symlink()) return false;
            if (restoring && entry.is_regular_file() && entry.path().extension()==FILE_FORMAT &&
                !validSavedXml(juce::File(juce::String(entry.path().u8string())))) return false;
        }
    }
    if (restoring && libraries.size()==4 && fs::exists(source/(INFO_FILE_NAME+FILE_FORMAT)) &&
        !validSavedXml(juce::File(juce::String((source/(INFO_FILE_NAME+FILE_FORMAT)).u8string())))) return false;
    const auto recovery=destination.parent_path()/(restoring ? ".restore-save" : ".backup-save");
    return SafeFileIO::writeDirectory(destination,recovery,[&](const fs::path& pending)
    {
        if (fs::is_directory(destination))
            for (const auto& entry : fs::directory_iterator(destination))
                fs::copy(entry.path(),pending/entry.path().filename(),fs::copy_options::recursive);
        for (const auto& name : libraries)
        {
            fs::remove_all(pending/name);
            fs::copy(source/name,pending/name,fs::copy_options::recursive);
        }
        if (libraries.size()==4 && fs::exists(source/(INFO_FILE_NAME+FILE_FORMAT)))
            fs::copy_file(source/(INFO_FILE_NAME+FILE_FORMAT),pending/(INFO_FILE_NAME+FILE_FORMAT),fs::copy_options::overwrite_existing);
        if (restoring)
        {
            if (std::find(libraries.begin(),libraries.end(),SONG_FOLDER_NAME)!=libraries.end())
            { fs::remove_all(pending/".song-save");fs::remove_all(pending/".scene-save"); }
            if (std::find(libraries.begin(),libraries.end(),PATTERNS_FOLDER_NAME)!=libraries.end()) fs::remove_all(pending/".pattern-save");
        }
        return true;
    });
}
#endif

bool EngineFileHandler::CreateBackUP(uint8_t DataFolderId)
{
#ifdef _WIN32
    auto lock=engine.lockDesktopState();
    try { return desktopCopyBackup(root_work_path,fs::path(getBackUPPathFromId(DataFolderId)),desktopBackupLibraries(DataFolderId),false); }
    catch (...) { return false; }
#else

	bool ret = true;
	fs::path backupPath = getBackUPPathFromId(DataFolderId);

	if (exists(root_work_path))
	{
		if (!exists(backupPath))
		{
			if (!fs::create_directory(backupPath))
			{
				DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + " create backup bath failed  " + backupPath.string());
				return false;
			}
		}
		switch (EngineDataFolderId(DataFolderId))
		{
		case EngineDataFolderId::AllDataInternalStorage:
		case EngineDataFolderId::AllDataExternalStorage: //intentional fallthrough, backupPath updated above
			fs::remove_all(backupPath / SONG_FOLDER_NAME);
			fs::remove_all(backupPath / INSTRUMENTS_FOLDER_NAME);
			fs::remove_all(backupPath / PATTERNS_FOLDER_NAME);
			fs::remove_all(backupPath / SETTINGS_FOLDER_NAME);

			fs::copy(songs_path, (backupPath / SONG_FOLDER_NAME), fs::copy_options::recursive);
			fs::copy(instruments_path, backupPath / INSTRUMENTS_FOLDER_NAME, fs::copy_options::recursive);
			fs::copy(patterns_path, backupPath / PATTERNS_FOLDER_NAME, fs::copy_options::recursive);
			fs::copy(settings_path, backupPath / SETTINGS_FOLDER_NAME, fs::copy_options::recursive);
			break;
		case EngineDataFolderId::SongsInternalStorage:
		case EngineDataFolderId::SongsExternalStorage:
			fs::remove_all(backupPath / SONG_FOLDER_NAME);
			fs::copy(songs_path, (backupPath / SONG_FOLDER_NAME), fs::copy_options::recursive);
			break;
		case EngineDataFolderId::PatternInternalStorage:
		case EngineDataFolderId::PatternExternalStorage:
			fs::remove_all(backupPath / PATTERNS_FOLDER_NAME);
			fs::copy(patterns_path, backupPath / PATTERNS_FOLDER_NAME, fs::copy_options::recursive);
			break;
		case EngineDataFolderId::InstrumentInternalStorage:
		case EngineDataFolderId::InstrumentExternalStorage:
			fs::remove_all(backupPath / INSTRUMENTS_FOLDER_NAME);
			fs::copy(instruments_path, backupPath / INSTRUMENTS_FOLDER_NAME, fs::copy_options::recursive);
			break;
		case EngineDataFolderId::SettingsInternalStorage:
		case EngineDataFolderId::SettingsExternalStorage:
			fs::remove_all(backupPath / SETTINGS_FOLDER_NAME);
			fs::copy(settings_path, backupPath / SETTINGS_FOLDER_NAME, fs::copy_options::recursive);
			break;
		case EngineDataFolderId::MaxValue:
		default:
			ret = false;
			break;
		}
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "FolderId ID " + String(DataFolderId) + "Path : " + (backupPath.string()));
	}
	else
	{
		ret = false;
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN FolderId out of range/Path invalid " + String(DataFolderId));
	}
	return ret;
#endif
}

bool EngineFileHandler::RestoreBackUP(uint8_t DataFolderId)
{
#ifdef _WIN32
    auto lock=engine.lockDesktopState();
    try { return desktopCopyBackup(fs::path(getBackUPPathFromId(DataFolderId)),root_work_path,desktopBackupLibraries(DataFolderId),true); }
    catch (...) { return false; }
#else

	bool ret = true;
	fs::path backupPath = getBackUPPathFromId(DataFolderId);

	if (exists(root_work_path) && exists(backupPath))
	{
		try {
			switch (EngineDataFolderId(DataFolderId))
			{
			case EngineDataFolderId::AllDataInternalStorage:
			case EngineDataFolderId::AllDataExternalStorage://intentional fallthrough, backupPath updated above
			{
				if (exists(backupPath / SONG_FOLDER_NAME))
				{
					fs::remove_all(songs_path);
					fs::copy(backupPath / SONG_FOLDER_NAME, songs_path, fs::copy_options::recursive);
				}
				if (exists(backupPath / INSTRUMENTS_FOLDER_NAME))
				{
					fs::remove_all(instruments_path);
					fs::copy(backupPath / INSTRUMENTS_FOLDER_NAME, instruments_path, fs::copy_options::recursive);
				}
				if (exists(backupPath / PATTERNS_FOLDER_NAME))
				{
					fs::remove_all(patterns_path);
					fs::copy(backupPath / PATTERNS_FOLDER_NAME, patterns_path, fs::copy_options::recursive);
				}
				if (exists(backupPath / SETTINGS_FOLDER_NAME))
				{
					fs::remove_all(settings_path);
					fs::copy(backupPath / SETTINGS_FOLDER_NAME, settings_path, fs::copy_options::recursive);
				}
			}
			break;
			case EngineDataFolderId::SongsInternalStorage:
			case EngineDataFolderId::SongsExternalStorage:
				if (exists(backupPath / SONG_FOLDER_NAME))
				{
					fs::remove_all(songs_path);
					fs::copy(backupPath / SONG_FOLDER_NAME, songs_path, fs::copy_options::recursive);
				}
				break;
			case EngineDataFolderId::PatternInternalStorage:
			case EngineDataFolderId::PatternExternalStorage:
				if (exists(backupPath / PATTERNS_FOLDER_NAME))
				{
					fs::remove_all(patterns_path);
					fs::copy(backupPath / PATTERNS_FOLDER_NAME, patterns_path, fs::copy_options::recursive);
				}
				break;
			case EngineDataFolderId::InstrumentInternalStorage:
			case EngineDataFolderId::InstrumentExternalStorage:
				if (exists(backupPath / INSTRUMENTS_FOLDER_NAME))
				{
					fs::remove_all(instruments_path);
					fs::copy(backupPath / INSTRUMENTS_FOLDER_NAME, instruments_path, fs::copy_options::recursive);
				}
				break;
			case EngineDataFolderId::SettingsInternalStorage:
			case EngineDataFolderId::SettingsExternalStorage:
				if (exists(backupPath / SETTINGS_FOLDER_NAME))
				{
					fs::remove_all(settings_path);
					fs::copy(backupPath / SETTINGS_FOLDER_NAME, settings_path, fs::copy_options::recursive);
				}
				break;
			case EngineDataFolderId::MaxValue:
			default:
				ret = false;
				break;
			}
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "FolderId ID " + String(DataFolderId) + "Path : " + (backupPath.string()));
		}
		catch (...) {
			DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "Error: exception ");
		}
	}
	else
	{
		DBG(String(__FUNCTION__) + "::" + String(__LINE__) + "::" + "WARN FolderId out of range/Path invalid " + String(DataFolderId));
	}
	return ret;
#endif
}

//Add multiple params/struct if required in future, update the argument list as required
FileHandlerError EngineFileHandler::updateMainInfo(mainInfoData info)
{
	const auto savePath=root_work_path / (INFO_FILE_NAME + FILE_FORMAT);

	XmlElement DataXml("mainInfoData");
	XmlElement* DataInfo = new XmlElement("Info");
	DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
	DataInfo->setAttribute("activeSongId", info.activeSongId);
	DataInfo->setAttribute("masterVolume", info.masterVolume);
	DataXml.addChildElement(DataInfo);
	if(!writeXmlAtomic(savePath,DataXml)) return FILE_ERROR;
	return FILE_OK;
}

int32_t EngineFileHandler::getMainInfo(mainInfoData& info)
{
	uint8_t version = 0;
	bool handleFailure = true;
	if (exists(root_work_path / (INFO_FILE_NAME + FILE_FORMAT)))
	{
		fs::path filepath = { root_work_path / (INFO_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(filepath.string());

		if (!infofile.exists())
			return -1;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml)
			{
				//handleFailure
			}
			else if (DataXml->hasTagName("mainInfoData"))
			{
				for (auto* e : DataXml->getChildIterator())
				{
					if (e->hasTagName("Info"))
					{
						version = (uint8_t)e->getIntAttribute("FILE_VERSION");
						info.activeSongId = e->getIntAttribute("activeSongId");
						info.masterVolume = static_cast<float>(e->getDoubleAttribute("masterVolume", 0.7f));
					}
				}
				handleFailure = false;
			}
		}
		catch (...) {
			//handleFailure
		}
	}
	if (handleFailure)
	{
		isDataCorrupted = true;
		mainInfoData temp;
		updateMainInfo(temp); //Create with default data
		info.activeSongId = 0;
	}
	return 0;
}

FileHandlerError EngineFileHandler::CreateSong(SongData songData)
{
    if (!SafeFileIO::isLeafName(songData.songName)) return FILE_ERROR;
    const bool saved = SafeFileIO::writeDirectory(songs_path / songData.songName, root_work_path / ".song-save",
        [&](const fs::path& pending)
        {
            const auto previous = songs_path / songData.songName;
            if (fs::exists(previous))
                for (const auto& entry : fs::directory_iterator(previous))
                    if (entry.is_regular_file()) fs::copy_file(entry.path(), pending / entry.path().filename());
            if (writeSongInfo(pending, songData) != FILE_OK) return false;
            for (const auto& scene : songData.sceneList)
                if (!SafeFileIO::isLeafName(scene._sceneData->sceneName) ||
                    writeScene(pending / scene._sceneData->sceneName, *scene._sceneData) != FILE_OK) return false;
            return true;
        });
    if (!saved) juce::Logger::writeToLog("Song save failed; previous data retained: " + juce::String(songData.songName));
    else
    {
        std::error_code ignored;
        fs::remove_all(root_work_path / ".scene-save" / songData.songName, ignored);
    }
    return saved ? FILE_OK : FILE_ERROR;
}
FileHandlerError EngineFileHandler::DeleteSong(std::string SongName)
{
    if (!SafeFileIO::isLeafName(SongName)) return FILE_ERROR;
    std::error_code error;
    fs::remove_all(root_work_path / ".scene-save" / SongName, error);
    if (error) return FILE_ERROR;
    fs::remove_all(root_work_path / ".song-save" / "backups" / SongName, error);
    if (error) return FILE_ERROR;
    const auto removed = fs::remove_all(songs_path / SongName, error);
    return !error && removed ? FILE_OK : FILE_ERROR;
}
FileHandlerError EngineFileHandler::DeleteAllSongs()
{
    fs::remove_all(root_work_path / ".scene-save");
	fs::remove_all(root_work_path / ".song-save");
	FileHandlerError ret = (fs::remove_all(songs_path)) ? FILE_OK : FILE_ERROR;
	exists(songs_path) ? 0 : fs::create_directory(songs_path);
	return ret;
}

FileHandlerError EngineFileHandler::UpdateSongInfo(SongData& songData)
{
    return writeSongInfo(songs_path / songData.songName, songData);
}
FileHandlerError EngineFileHandler::writeSongInfo(const fs::path& folder, const SongData& songData)
{
	if (exists(folder))
	{

		XmlElement DataXml("Song_Info");
		XmlElement* DataInfo = new XmlElement("Info");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("songName", songData.songName);
		DataInfo->setAttribute("gTrackCount", songData.gTrackCount);
		DataInfo->setAttribute("songSwing", songData.songSwing);
		DataInfo->setAttribute("numberOfScenes", songData.numberOfScenes);
		DataInfo->setAttribute("songId", songData.songId);
		DataInfo->setAttribute("songTempo", (std::round(songData.songTempo * 10.0) / 10.0));
		DataInfo->setAttribute("applySongTempoToScenes", songData.applySongTempoToScenes);
		DataInfo->setAttribute("applySongSwingToScenes", songData.applySongSwingToScenes);
		DataXml.addChildElement(DataInfo);
		return SafeFileIO::writeFile(folder / (INFO_FILE_NAME + FILE_FORMAT), [&](std::ostream& output) { output << DataXml.toString(); }) ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}

SongData EngineFileHandler::GetSongInfo(std::string SongName)
{
	SongData songData = {};
	std::string buf = { 0 };
	uint8_t version = 0;
	bool handleFailure = true;
	if (exists(songs_path / SongName / (INFO_FILE_NAME + FILE_FORMAT)))
	{
		fs::path filepath = { songs_path / SongName / (INFO_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(filepath.string());
		if (!infofile.exists())
			return songData;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };

			if (!DataXml)
			{
				//handleFailure
			}
			else if (DataXml->hasTagName("Song_Info"))
			{
				for (auto* e : DataXml->getChildIterator())
				{
					if (e->hasTagName("Info"))
					{
						version = (uint8_t)e->getIntAttribute("FILE_VERSION");
						songData.songName = e->getStringAttribute("songName").toStdString();
						songData.gTrackCount = (uint8_t)e->getIntAttribute("gTrackCount");
						songData.songSwing = (uint8_t)e->getIntAttribute("songSwing");
						songData.numberOfScenes = e->getIntAttribute("numberOfScenes");
						songData.songId = e->getIntAttribute("songId");
						songData.songTempo = (float)e->getDoubleAttribute("songTempo");
						songData.applySongTempoToScenes = e->getIntAttribute("applySongTempoToScenes");
						songData.applySongSwingToScenes = e->getIntAttribute("applySongSwingToScenes");
					}
				}
				handleFailure = false;
			}
		}
		catch (...) {
			//handleFailure
		}
	}
	if (handleFailure)
	{
		isDataCorrupted = true;
		songData.songName = SongName;
		UpdateSongInfo(songData);//default data
	}
	return songData;
}

FileHandlerError EngineFileHandler::RenameSong(std::string SongName, std::string SongNewName)
{
	if ((exists(songs_path / SongName)) && !exists(songs_path / SongNewName))
	{
		fs::rename(songs_path / SongName, songs_path / SongNewName);
	}
	else
		return FILE_EXISTS;

	return(exists(songs_path / SongNewName) ? FILE_OK : FILE_ERROR);
}

uint16_t EngineFileHandler::GetNumberofSongs()
{
	const fs::path songPath{ songs_path };
	uint16_t FolderCount = 0;

	for (const auto& file : fs::directory_iterator(songPath))
		file.is_directory() ? ++FolderCount : 0;

	return FolderCount;
}

//filesytem count starts from 1
//this will return the list sorted by name
std::vector<std::string> EngineFileHandler::GetSongNameList(uint16_t StartPos, uint16_t Count)
{
	std::vector<std::string> SongList;
	const fs::path songPath{ songs_path };
	uint16_t FileCount = 0;

	for (const auto& file : fs::directory_iterator(songPath))
	{
		if (file.is_directory())
		{
			++FileCount;
			if (FileCount >= StartPos && FileCount <= (StartPos + Count))
				SongList.push_back(file.path().stem().string());

			if (FileCount >= (StartPos + Count))
				break;
		}
	}
	return SongList;
}

FileHandlerError EngineFileHandler::CreateScene(std::string SongName, SceneData sceneData)
{
    if (!SafeFileIO::isLeafName(SongName) || !SafeFileIO::isLeafName(sceneData.sceneName) ||
        !fs::is_directory(songs_path / SongName)) return FILE_ERROR;
    const bool saved = SafeFileIO::writeDirectory(songs_path / SongName / sceneData.sceneName,
        root_work_path / ".scene-save" / SongName,
        [&](const fs::path& pending) { return writeScene(pending, sceneData) == FILE_OK; });
    if (!saved) juce::Logger::writeToLog("Scene save failed; previous data retained: " + juce::String(sceneData.sceneName));
    return saved ? FILE_OK : FILE_ERROR;
}
FileHandlerError EngineFileHandler::writeScene(const fs::path& folder, const SceneData& sceneData)
{
    if (!fs::exists(folder)) fs::create_directory(folder);
    else if (!fs::is_empty(folder)) return FILE_ERROR;
    if (writeSceneInfo(folder, sceneData) != FILE_OK) return FILE_ERROR;
    std::vector<TrackData> tracks;
    for (const auto& track : sceneData.trackList) tracks.push_back(*track._trackData);
    return writeTracks(folder, tracks);
}

FileHandlerError EngineFileHandler::DeleteScene(std::string SongName, std::string SceneName)
{
    if (!SafeFileIO::isLeafName(SongName) || !SafeFileIO::isLeafName(SceneName)) return FILE_ERROR;
    fs::remove_all(root_work_path / ".scene-save" / SongName / "backups" / SceneName);
	if (exists(songs_path / SongName))
		return (fs::remove_all(songs_path / SongName / SceneName) ? FILE_OK : FILE_ERROR);
	else
		return FILE_EXISTS;
}

FileHandlerError EngineFileHandler::UpdateSceneInfo(std::string SongName, SceneData sceneData)
{
    return writeSceneInfo(songs_path / SongName / sceneData.sceneName, sceneData);
}
FileHandlerError EngineFileHandler::writeSceneInfo(const fs::path& folder, const SceneData& sceneData)
{
	if (exists(folder))
	{

		XmlElement DataXml("SceneInfo");
		XmlElement* DataInfo = new XmlElement("Info");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("transposeAfterFTS", sceneData.transposeAfterFTS);
		DataInfo->setAttribute("sceneSwing", sceneData.sceneSwing);
		DataInfo->setAttribute("gBarLenInSteps", sceneData.gBarLenInSteps);
		DataInfo->setAttribute("numBars", sceneData.numBars);
		DataInfo->setAttribute("mode", sceneData.mode);
		DataInfo->setAttribute("scaleFTS", sceneData.scaleFTS);
		DataInfo->setAttribute("noteFTS", sceneData.noteFTS);
		DataInfo->setAttribute("transposeValue", sceneData.transposeValue);
		DataInfo->setAttribute("numberOfTracks", sceneData.numberOfTracks);
		DataInfo->setAttribute("sceneId", (int)sceneData.sceneId);
		DataInfo->setAttribute("trackMuteStatus", (int)sceneData.trackMuteStatus);
		DataInfo->setAttribute("sceneTempo", (std::round(sceneData.sceneTempo * 10.0) / 10.0));
		DataXml.addChildElement(DataInfo);
		return SafeFileIO::writeFile(folder / (INFO_FILE_NAME + FILE_FORMAT), [&](std::ostream& output) { output << DataXml.toString(); }) ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}

SceneData EngineFileHandler::GetSceneInfo(std::string SongName, std::string SceneName)
{
	SceneData sceneData = {};
	uint8_t version = 0;
	std::string buf{ 0 };
	bool handleFailure = true;
	if (exists(songs_path / SongName / SceneName / (INFO_FILE_NAME + FILE_FORMAT)))
	{
		fs::path filepath = { songs_path / SongName / SceneName / (INFO_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(filepath.string());
		if (!infofile.exists())
			return sceneData;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml)
			{
				//handleFailure
			}
			else if (DataXml->hasTagName("SceneInfo"))
			{
				for (auto* e : DataXml->getChildIterator())
				{
					if (e->hasTagName("Info"))
					{
						version = (uint8_t)e->getIntAttribute("FILE_VERSION");
						sceneData.transposeAfterFTS = e->getBoolAttribute("transposeAfterFTS");
						sceneData.sceneSwing = (int8_t)e->getIntAttribute("sceneSwing");
						sceneData.gBarLenInSteps = (int8_t)e->getIntAttribute("gBarLenInSteps");
						sceneData.numBars = (uint16_t)e->getIntAttribute("numBars");
						sceneData.mode = (uint8_t)e->getIntAttribute("mode");
						sceneData.scaleFTS = (uint8_t)e->getIntAttribute("scaleFTS");
						sceneData.noteFTS = (uint8_t)e->getIntAttribute("noteFTS");
						sceneData.transposeValue = (uint8_t)e->getIntAttribute("transposeValue");
						sceneData.numberOfTracks = (uint8_t)e->getIntAttribute("numberOfTracks");
						sceneData.sceneId = e->getIntAttribute("sceneId");
						sceneData.trackMuteStatus = e->getIntAttribute("trackMuteStatus");
						sceneData.sceneTempo = (float)e->getIntAttribute("sceneTempo");
					}
				}
				handleFailure = false;
			}
		}
		catch (...) {
			//handleFailure
		}
	}
	if (handleFailure)
	{
		isDataCorrupted = true;
		sceneData.sceneName = SceneName;
		UpdateSceneInfo(SongName, sceneData);
	}
	return sceneData;
}

uint16_t EngineFileHandler::GetNumberofScene(std::string SongName)
{
	const fs::path scenePath{ songs_path / SongName };
	uint16_t FolderCount = 0;

	for (const auto& file : fs::directory_iterator(scenePath))
		file.is_directory() ? ++FolderCount : 0;

	return FolderCount;
}

FileHandlerError EngineFileHandler::RenameScene(std::string SongName, std::string SceneName, std::string SceneNewName)
{
	if ((exists(songs_path / SongName / SceneName)) && !exists(songs_path / SongName / SceneNewName))
	{
		fs::rename(songs_path / SongName / SceneName, songs_path / SongName / SceneNewName);
	}
	else
		return FILE_EXISTS;

	return((exists(songs_path / SongName / SceneNewName)) ? FILE_OK : FILE_ERROR);
}

std::vector<std::string> EngineFileHandler::GetSceneList(std::string SongName, uint16_t StartPos, uint16_t Count)
{
	std::vector<std::string> SceneList;
	const fs::path scenePath{ songs_path / SongName };
	uint16_t FileCount = 0;

	for (const auto& file : fs::directory_iterator(scenePath))
	{
		if (file.is_directory())
		{
			++FileCount;
			if (FileCount >= StartPos && FileCount <= (StartPos + Count))
				SceneList.push_back(file.path().stem().string());

			if (FileCount >= (StartPos + Count))
				break;
		}
	}
	return SceneList;
}

FileHandlerError EngineFileHandler::CreateTracksWithList(std::string SongName, std::string SceneName, std::vector<TrackData> TrackDataList)
{
    return writeTracks(songs_path / SongName / SceneName, TrackDataList);
}
FileHandlerError EngineFileHandler::writeTracks(const fs::path& folder, const std::vector<TrackData>& TrackDataList)
{
	FileHandlerError ret = FILE_EXISTS;

	if (exists(folder))
	{


		XmlElement DataXml("Track");
		for (auto TrackData : TrackDataList)
		{
			XmlElement* DataInfo = new XmlElement("TrackData");

			uint8_t tvTrackDatasize = static_cast<uint8_t>(TrackData.tvTrackData.size());
			DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
			DataInfo->setAttribute("trackMute", TrackData.trackMute);
			DataInfo->setAttribute("trackRecordEnable", TrackData.trackRecordEnable);
			DataInfo->setAttribute("trackSolo", TrackData.trackSolo);
			DataInfo->setAttribute("trackId", TrackData.trackId);
			DataInfo->setAttribute("trackName", TrackData.trackName);
			DataInfo->setAttribute("trackRxInstrumentName", TrackData.trackRxInstrumentName);
			DataInfo->setAttribute("trackTxInstrumentName", TrackData.trackTxInstrumentName);
			DataInfo->setAttribute("mainPatternName", TrackData.mainPatternName);
			DataInfo->setAttribute("fillPatternName", TrackData.fillPatternName);
			DataInfo->setAttribute("tvTrackDatasize", tvTrackDatasize);
            DataInfo->setAttribute("desktopFx",TrackData.hasDesktopFx);
            if(TrackData.hasDesktopFx)for(int key=0;key<16;++key)DataInfo->setAttribute("fx"+juce::String(key),TrackData.desktopFxState[size_t(key)]);

			if (TrackData.tvTrackData.size() > 0)
			{
				for (auto& tvData : TrackData.tvTrackData)
				{
					XmlElement* DataInfoTv = new XmlElement("tvTrackData");
					if (tvData.second.paramType == TrackValueType::TVTypeMidiCC)
					{
						DataInfoTv->setAttribute("first", tvData.first);//knobid
						DataInfoTv->setAttribute("paramType", static_cast <uint8_t>(tvData.second.paramType));//type
						DataInfoTv->setAttribute("tvMidiCC", tvData.second.param.tvMidiCC.value);//value
					}
					else if (tvData.second.paramType == TrackValueType::TVTypePgmChange)
					{
						DataInfoTv->setAttribute("first", tvData.first);//knobid
						DataInfoTv->setAttribute("paramType", static_cast <uint8_t>(tvData.second.paramType));//type

						DataInfoTv->setAttribute("bankLSB", tvData.second.param.tvPgmChange.bankLSB);//lsb
						DataInfoTv->setAttribute("bankMSB", tvData.second.param.tvPgmChange.bankMSB);//lsb
						DataInfoTv->setAttribute("midiPgmNum", tvData.second.param.tvPgmChange.midiPgmNum);//lsb
					}
					else
					{
						//TODO : Log error
					}
					DataInfo->addChildElement(DataInfoTv); // TODO: do we need to add else also ?
				}
			}
			DataXml.addChildElement(DataInfo);
		}
		return SafeFileIO::writeFile(folder / ("TrackV1" + FILE_FORMAT), [&](std::ostream& output) { output << DataXml.toString(); }) ? FILE_OK : FILE_ERROR;
	}
	return ret;
}

std::vector<TrackData> EngineFileHandler::GetTrackDataList(std::string SongName, std::string SceneName, int8_t numberOfTracks)
{
	std::vector<TrackData> TrackDataList;
	TrackData TrackData = {};
	std::string buf = { 0 };
	int8_t version = 0;
	bool handleFailure = true;

	if (exists(songs_path / SongName / SceneName))
	{
		fs::path filepath = { songs_path / SongName / SceneName / ("TrackV1" + FILE_FORMAT) };
		juce::File infofile(filepath.string());

		if (!infofile.exists())
			return TrackDataList;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || (!DataXml->hasTagName("Track")))
			{
				//handleFailure
			}
			else
			{
				for (auto* e : DataXml->getChildWithTagNameIterator("TrackData"))
				{
					version = (int8_t)e->getIntAttribute("FILE_VERSION");
					TrackData.trackMute = e->getBoolAttribute("trackMute");
					TrackData.trackRecordEnable = e->getBoolAttribute("trackRecordEnable");
					TrackData.trackSolo = e->getBoolAttribute("trackSolo");
					TrackData.trackId = (uint8_t)e->getIntAttribute("trackId");
					TrackData.trackName = e->getStringAttribute("trackName").toStdString();
					TrackData.trackRxInstrumentName = e->getStringAttribute("trackRxInstrumentName").toStdString();
					TrackData.trackTxInstrumentName = e->getStringAttribute("trackTxInstrumentName").toStdString();
					TrackData.mainPatternName = e->getStringAttribute("mainPatternName").toStdString();
					TrackData.fillPatternName = e->getStringAttribute("fillPatternName").toStdString();
					TrackData.tvTrackDataCount = (uint8_t)e->getIntAttribute("tvTrackDatasize");
                    TrackData.hasDesktopFx=e->getBoolAttribute("desktopFx",false);
                    const Ignite::TrackData defaults{};
                    for(int key=0;key<16;++key)TrackData.desktopFxState[size_t(key)]=float(e->getDoubleAttribute("fx"+juce::String(key),defaults.desktopFxState[size_t(key)]));
					TrackData.tvTrackData.clear();

					if (TrackData.tvTrackDataCount > 0)
					{
						TrackData.tvTrackData.clear();
						for (auto* f : e->getChildWithTagNameIterator("tvTrackData"))
						{
							std::pair <int8_t, TrackValue> tmp;

							tmp.first = static_cast<int8_t>(f->getIntAttribute("first"));//knobid
							tmp.second.paramType = static_cast <TrackValueType>(f->getIntAttribute("paramType"));//type

							if (tmp.second.paramType == TrackValueType::TVTypeMidiCC)
							{
								tmp.second.param.tvMidiCC.value = static_cast<float>(f->getDoubleAttribute("tvMidiCC"));//value
							}
							else if (tmp.second.paramType == TrackValueType::TVTypePgmChange)
							{
								tmp.second.param.tvPgmChange.bankLSB = (int8_t)f->getIntAttribute("bankLSB");//lsb
								tmp.second.param.tvPgmChange.bankMSB = (int8_t)f->getIntAttribute("bankMSB");//lsb
								tmp.second.param.tvPgmChange.midiPgmNum = (int8_t)f->getIntAttribute("midiPgmNum");//lsb
							}
							else
							{
								//TODO : Log error
							}
							TrackData.tvTrackData.insert(tmp);
						}
					}
					TrackDataList.push_back(TrackData);
				}
				handleFailure = false;
			}
		}
		catch (...) {
			//handleFailure
		}

		if (handleFailure)
		{
			isDataCorrupted = true;
			TrackDataList.push_back(TrackData);//create 1 track as the file is valid and caller expect a data
			CreateTracksWithList(SongName, SceneName, TrackDataList);//create empty default
		}
	}
	return TrackDataList;
}

FileHandlerError EngineFileHandler::DeleteTrackList(std::string SongName, std::string SceneName)
{
	if (exists(songs_path / SongName / SceneName))
	{
		return ((fs::remove(songs_path / SongName / SceneName / ("TrackV1" + FILE_FORMAT))) ? FILE_OK : FILE_ERROR);
	}
	return FILE_ERROR;
}
FileHandlerError EngineFileHandler::DeleteTrack(std::string SongName, std::string SceneName, std::string TrackName)
{
	if (exists(songs_path / SongName / SceneName))
	{
		return ((fs::remove(songs_path / SongName / SceneName / (TrackName + FILE_FORMAT))) ? FILE_OK : FILE_ERROR);
	}
	return FILE_ERROR;
}

uint16_t EngineFileHandler::GetNumberofTracks(std::string SongName, std::string SceneName)
{
	const fs::path trackpath{ songs_path / SongName / SceneName };
	uint16_t Count = 0;

	for (const auto& file : fs::directory_iterator(trackpath))
	{
		if (file.path().extension() == FILE_FORMAT &&
			file.path().stem().string() != INFO_FILE_NAME)
		{
			++Count;
		}
	}
	return Count;
}

FileHandlerError EngineFileHandler::RenameTrack(std::string SongName, std::string SceneName, std::string TrackName, std::string TrackNewName)
{
	if ((exists(songs_path / SongName / SceneName / TrackName)) && !exists(songs_path / SongName / SceneName / TrackNewName))
	{
		fs::rename(songs_path / SongName / SceneName / TrackName, songs_path / SongName / SceneName / TrackNewName);
	}
	else
		return FILE_EXISTS;

	return((exists(songs_path / SongName / SceneName / TrackNewName)) ? FILE_OK : FILE_ERROR);
}

std::vector<std::string> EngineFileHandler::GetTrackList(std::string SongName, std::string SceneName, uint16_t StartPos, uint16_t Count)
{
	std::vector<std::string> trackNameList;
	const fs::path scenePath{ songs_path / SongName / SceneName };
	uint16_t FileCount = 0;

	for (const auto& file : fs::directory_iterator(scenePath))
	{
		if (file.path().extension() == FILE_FORMAT &&
			file.path().stem().string() != INFO_FILE_NAME)
		{
			++FileCount;

			if (FileCount >= StartPos && FileCount <= Count)
				trackNameList.push_back(file.path().stem().string());

			if (FileCount >= Count)
				break;
		}
	}
	return trackNameList;
}

FileHandlerError EngineFileHandler::Createpattern(std::string PatternName, PatternData patternData)
{
    if (!SafeFileIO::isLeafName(PatternName)) return FILE_ERROR;
    const bool saved = SafeFileIO::writeDirectory(patterns_path / PatternName, root_work_path / ".pattern-save",
        [&](const fs::path& pending) { return writePattern(pending, patternData) == FILE_OK; });
    if (!saved) juce::Logger::writeToLog("Pattern save failed; previous data retained: " + juce::String(PatternName));
    return saved ? FILE_OK : FILE_ERROR;
}
FileHandlerError EngineFileHandler::writePattern(const fs::path& folder, const PatternData& patternData)
{
	if (fs::exists(folder))
	{
		std::ofstream infofile(folder / (INFO_FILE_NAME + FILE_FORMAT), std::ofstream::binary);
		if (!infofile)
			return FILE_ERROR;

		XmlElement DataPatternXml("pattern");
		XmlElement* DataInfo = new XmlElement("pattern_info");

		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("id", patternData.id);
		DataInfo->setAttribute("patternDispName", patternData.patternDispName);
		DataInfo->setAttribute("numBars", patternData.numBars);
		DataInfo->setAttribute("timeBase", (uint8_t)patternData.timeBase);
		DataInfo->setAttribute("patternLastStepId", patternData.patternLastStepId);
		DataInfo->setAttribute("_barLoopOn", patternData._barLoopOn);
		DataInfo->setAttribute("_barLoopStartId", patternData._barLoopStartId);
		DataInfo->setAttribute("_barLoopEndId", patternData._barLoopEndId);
		DataInfo->setAttribute("UsedByCount", (int)patternData.UsedByCount);

		if (patternData.UsedByCount && !patternData.usedByList.empty())
		{
			for (auto UsedByIt = patternData.usedByList.begin(); UsedByIt != patternData.usedByList.end(); ++UsedByIt)
			{
				XmlElement* DataUsedby = new XmlElement("usedby");
				DataUsedby->setAttribute("isFillPattern", UsedByIt->isFillPattern);
				DataUsedby->setAttribute("songName", UsedByIt->songName);
				DataUsedby->setAttribute("sceneName", UsedByIt->sceneName);
				DataUsedby->setAttribute("trackName", UsedByIt->trackName);
				DataInfo->addChildElement(DataUsedby);
			}
		}
		DataPatternXml.addChildElement(DataInfo);
		infofile << DataPatternXml.toString();
		infofile.flush();
		infofile.close();
                    if (!infofile) return FILE_ERROR;

		if (patternData.numBars && !patternData.barList.empty())
		{
			for (auto& barIt : patternData.barList)
			{
				std::string barFD = ("BAR-" + std::to_string(barIt.second._BarData->BarId));
				if (!fs::create_directory(folder / barFD))
					return FILE_ERROR;

				std::ofstream barInfofile(folder / barFD / (INFO_FILE_NAME + FILE_FORMAT), std::ofstream::binary);
				if (!barInfofile)
					return FILE_ERROR;

				XmlElement DataBarXml("pattern_bar");
				XmlElement* DataBarInfo = new XmlElement("bar_info");
				DataBarInfo->setAttribute("FILE_VERSION", FILE_VERSION);
				DataBarInfo->setAttribute("id", barIt.second._BarData->BarId);
				DataBarInfo->setAttribute("stepCount", barIt.second._BarData->stepCount);
				DataBarInfo->setAttribute("barRepeatCount", barIt.second._BarData->barRepeatCount);
				DataBarInfo->setAttribute("barTransposeDefeat", barIt.second._BarData->barTransposeDefeat);
				DataBarInfo->setAttribute("gBarSyncOn", barIt.second._BarData->gBarSyncOn);
				DataBarInfo->setAttribute("direction",  static_cast<int>(barIt.second._BarData->direction));
				DataBarXml.addChildElement(DataBarInfo);
				barInfofile << DataBarXml.toString();
				barInfofile.flush();
				barInfofile.close();
                    if (!barInfofile) return FILE_ERROR;

				if (barIt.second._BarData->stepCount && !barIt.second._BarData->stepList.empty())
				{
					std::string stepName = ("step" + FILE_FORMAT);
					std::ofstream stepfile(folder / barFD / stepName, std::ofstream::binary);
					if (!stepfile)
						return FILE_ERROR;

					XmlElement DataStepXml("step");
					for (auto StepIt = barIt.second._BarData->stepList.begin(); StepIt != barIt.second._BarData->stepList.end(); ++StepIt)
					{
						XmlElement* DataStepInfo = new XmlElement("step_data");
						DataStepInfo->setAttribute("FILE_VERSION", FILE_VERSION);
						DataStepInfo->setAttribute("stepId", StepIt->second._stepData->stepId);
						DataStepInfo->setAttribute("nodeCount_size", (int)StepIt->second._stepData->nodeList.size());
						DataStepInfo->setAttribute("enable", StepIt->second._stepData->enable);
						DataStepInfo->setAttribute("skipStatus", StepIt->second._stepData->skipStatus);
						DataStepInfo->setAttribute("transposeDefeatStatus", StepIt->second._stepData->transposeDefeatStatus);
						DataStepInfo->setAttribute("tieStatus", StepIt->second._stepData->tieStatus);

						if (StepIt->second._stepData->nodeCount && !StepIt->second._stepData->nodeList.empty())
						{
							for (auto itNode = StepIt->second._stepData->nodeList.begin(); itNode != StepIt->second._stepData->nodeList.end(); ++itNode)
							{
								XmlElement* StepNodeInfo = new XmlElement("step_node_data");
								for (auto& nodeData : itNode->second)
								{
									XmlElement* DataNodeInfo = new XmlElement("note_data");
									DataNodeInfo->setAttribute("nodeId", nodeData.second.nodeId);
									DataNodeInfo->setAttribute("note", nodeData.second.note);
									DataNodeInfo->setAttribute("velocity", nodeData.second.velocity);
									DataNodeInfo->setAttribute("lenInSteps", nodeData.second.lenInSteps);
									DataNodeInfo->setAttribute("lenInTicks", nodeData.second.lenInTicks);
									DataNodeInfo->setAttribute("delay", nodeData.second.offset);
									DataNodeInfo->setAttribute("step_id", StepIt->second._stepData->stepId);
									DataNodeInfo->setAttribute("noteCount", nodeData.second.noteCount);
									StepNodeInfo->addChildElement(DataNodeInfo);
								}
								DataStepInfo->addChildElement(StepNodeInfo);
							}
						}
						if (StepIt->second._stepData->auxList.size() != 0)
						{
							for (auto iter = StepIt->second._stepData->auxList.begin(); iter != StepIt->second._stepData->auxList.end(); ++iter)
							{
								XmlElement* DataNodeInfo = new XmlElement("aux_data");

								DataNodeInfo->setAttribute("state", iter->enabled);
								DataNodeInfo->setAttribute("statusByte", iter->type);
								DataNodeInfo->setAttribute("dataByte1", iter->data1);
								DataNodeInfo->setAttribute("dataByte2", iter->data2);
								DataNodeInfo->setAttribute("offset", iter->offset);

								DataStepInfo->addChildElement(DataNodeInfo);
							}
						}
						DataStepXml.addChildElement(DataStepInfo);
					}
					stepfile << DataStepXml.toString();
					stepfile.flush();
					stepfile.close();
                    if (!stepfile) return FILE_ERROR;
				}
			}
		}
	}
	return FILE_OK;
}

void EngineFileHandler::GetpatternDataHeader(std::string PatternName, PatternData* patternData)
{
	std::string buf = { 0 };
	int8_t version = 0;
	bool handleFailure = true;

	if (exists(patterns_path) && (exists(patterns_path / PatternName)))
	{
		fs::path filepath = { patterns_path / PatternName / (INFO_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(filepath.string());
		if (!infofile.exists())
		{
			//handleFailure
		}
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("pattern"))
			{
				//handleFailure
			}
			else
			{
				for (auto* e : DataXml->getChildWithTagNameIterator("pattern_info"))
				{
					version = (int8_t)e->getIntAttribute("FILE_VERSION");
					patternData->id = e->getIntAttribute("id");
					patternData->patternDispName = e->getStringAttribute("patternDispName").toStdString();;
					patternData->numBars = (uint8_t)e->getIntAttribute("numBars");
					patternData->timeBase = (TimeBase)e->getIntAttribute("timeBase");
					patternData->patternLastStepId = (int16_t)e->getIntAttribute("patternLastStepId");
					patternData->_barLoopOn = e->getIntAttribute("_barLoopOn");
					patternData->_barLoopStartId = (int8_t)e->getIntAttribute("_barLoopStartId");
					patternData->_barLoopEndId = (int8_t)e->getIntAttribute("_barLoopEndId");
					patternData->UsedByCount = e->getIntAttribute("UsedByCount");
					patternData->patternName = PatternName;

					if (patternData->UsedByCount)
					{
						UsedByData usedByData{};
						for (auto* f : e->getChildWithTagNameIterator("usedby"))
						{
							usedByData.isFillPattern = f->getBoolAttribute("isFillPattern");
							usedByData.songName = f->getStringAttribute("songName").toStdString();
							usedByData.sceneName = f->getStringAttribute("sceneName").toStdString();
							usedByData.trackName = f->getStringAttribute("trackName").toStdString();
							patternData->usedByList.push_back(usedByData);
						}
					}
				}
				handleFailure = false;
			}
		}
		catch (...) {
			//handleFailure
		}
		if (handleFailure)
		{
			isDataCorrupted = true;
			patternData->patternName = PatternName;
			patternData->patternDispName = PatternName;
			return;
		}
	}
	return;
}

FileHandlerError EngineFileHandler::updatePatternInfo(std::string patternName, PatternData patternData)
{
	if (exists(patterns_path / patternName))
	{
		const auto savePath=patterns_path / patternName / (INFO_FILE_NAME + FILE_FORMAT);
		XmlElement DataPatternXml("pattern");
		XmlElement* DataInfo = new XmlElement("pattern_info");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("id", patternData.id);
		DataInfo->setAttribute("patternDispName", patternData.patternDispName);
		DataInfo->setAttribute("numBars", patternData.numBars);
		DataInfo->setAttribute("timeBase", (uint8_t)patternData.timeBase);
		DataInfo->setAttribute("patternLastStepId", patternData.patternLastStepId);
		DataInfo->setAttribute("_barLoopOn", patternData._barLoopOn);
		DataInfo->setAttribute("_barLoopStartId", patternData._barLoopStartId);
		DataInfo->setAttribute("_barLoopEndId", patternData._barLoopEndId);
		DataInfo->setAttribute("UsedByCount", (int)patternData.UsedByCount);

		if (patternData.UsedByCount && !patternData.usedByList.empty())
		{
			for (auto UsedByIt = patternData.usedByList.begin(); UsedByIt != patternData.usedByList.end(); ++UsedByIt)
			{
				XmlElement* DataUsedby = new XmlElement("usedby");
				DataUsedby->setAttribute("isFillPattern", UsedByIt->isFillPattern);
				DataUsedby->setAttribute("songName", UsedByIt->songName);
				DataUsedby->setAttribute("sceneName", UsedByIt->sceneName);
				DataUsedby->setAttribute("trackName", UsedByIt->trackName);
				DataInfo->addChildElement(DataUsedby);
			}
		}
		DataPatternXml.addChildElement(DataInfo);
		if(!writeXmlAtomic(savePath,DataPatternXml)) return FILE_ERROR;
		return FILE_OK;
	}
	return FILE_EXISTS;
}

FileHandlerError EngineFileHandler::UpdatePatternBarInfo(std::string PatternName, BarData& _barData)
{
	BarData bardata = { 0 };
	const fs::path PatternPath{ patterns_path / PatternName };
	for (const auto& file : fs::directory_iterator(PatternPath))
	{
		if (file.is_directory())
		{
			std::string InfoName = (file.path().stem().string() + "/" + (INFO_FILE_NAME + FILE_FORMAT));
			fs::path filepath = { PatternPath / InfoName };
			juce::File barInfoFile(filepath.string());

			auto DataXml{ juce::XmlDocument::parse(barInfoFile) };
			if (!DataXml || !DataXml->hasTagName("pattern_bar"))
			{
				return FILE_ERROR;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("bar_info"))
			{
				bardata.BarId = (uint8_t)e->getIntAttribute("id");
			}

			if (_barData.BarId == bardata.BarId)
			{
				std::ofstream barInfoFile1(PatternPath / InfoName, std::ofstream::trunc | std::ofstream::binary);
				if (!barInfoFile1)
					return FILE_ERROR;

				XmlElement DataBarXml("pattern_bar");
				XmlElement* DataBarInfo = new XmlElement("bar_info");
				DataBarInfo->setAttribute("FILE_VERSION", FILE_VERSION);
				DataBarInfo->setAttribute("id", _barData.BarId);
				DataBarInfo->setAttribute("stepCount", _barData.stepCount);
				DataBarInfo->setAttribute("barRepeatCount", _barData.barRepeatCount);
				DataBarInfo->setAttribute("barTransposeDefeat", _barData.barTransposeDefeat);
				DataBarInfo->setAttribute("gBarSyncOn", _barData.gBarSyncOn);
				DataBarInfo->setAttribute("direction",  static_cast <int> (_barData.direction));
				DataBarXml.addChildElement(DataBarInfo);
				barInfoFile1 << DataBarXml.toString();
				barInfoFile1.flush();
				barInfoFile1.close();
				return FILE_OK;
			}
		}
	}
	return FILE_ERROR;
}

std::list<StepData> EngineFileHandler::GetpatternBarStepData(std::string PatternName, uint16_t BarId, uint16_t StepCount)
{
	std::string buf = { 0 };
	StepData stepData = {};
	std::list<StepData> StepDataList;
	std::string barFD = ("BAR-" + std::to_string(BarId));
	std::string stepName = ("step" + FILE_FORMAT);
	int8_t version = 0;
	fs::path filepath = { patterns_path / PatternName / barFD / stepName };
	juce::File stepfile(filepath.string());
	if (!stepfile.exists())
		return(StepDataList);

	try {
		auto DataXml{ juce::XmlDocument::parse(stepfile) };

		if (!DataXml || !DataXml->hasTagName("step"))
		{
			isDataCorrupted = true;
			return StepDataList;
		}

		for (auto* e : DataXml->getChildWithTagNameIterator("step_data"))
		{
			stepData.nodeList.clear();
			version = (int8_t)e->getIntAttribute("FILE_VERSION");
			stepData.stepId = (uint16_t)e->getIntAttribute("stepId");
			stepData.nodeCount = (uint8_t)e->getIntAttribute("nodeCount_size");
			stepData.enable = e->getBoolAttribute("enable");
			stepData.skipStatus = e->getBoolAttribute("skipStatus");
			stepData.transposeDefeatStatus = e->getBoolAttribute("transposeDefeatStatus");
			stepData.tieStatus = e->getBoolAttribute("tieStatus");


			for (auto* f : e->getChildWithTagNameIterator("step_node_data"))
			{
				for (auto* g : f->getChildWithTagNameIterator("note_data"))
				{
					NodeData node{};
					node.nodeId = (uint16_t)g->getIntAttribute("nodeId");
					node.note = (uint8_t)g->getIntAttribute("note");
					node.velocity = (uint8_t)g->getIntAttribute("velocity");
					node.lenInSteps = (uint8_t)g->getIntAttribute("lenInSteps");
					node.lenInTicks = (TickLength_t)g->getIntAttribute("lenInTicks");
					node.offset = (TickOffset_t)g->getIntAttribute("delay");
					node.step_id = (int8_t)g->getIntAttribute("step_id");
					node.noteCount = (int8_t)g->getIntAttribute("noteCount");
					stepData.nodeList[node.note][node.offset] = node;
				}
			}

			stepData.auxList.clear();
			for (auto* f : e->getChildWithTagNameIterator("aux_data"))
			{
				Aux auxEvent;
				auxEvent.enabled = f->getIntAttribute("state");
				auxEvent.type = (uint16_t)f->getIntAttribute("statusByte");
				auxEvent.data1 = (int8_t)f->getIntAttribute("dataByte1");
				auxEvent.data2 = (int8_t)f->getIntAttribute("dataByte2");
				auxEvent.offset = (int8_t)f->getIntAttribute("offset");

				stepData.auxList.push_back(auxEvent);
			}
			StepDataList.push_back(stepData);
		}
	}
	catch (...) {
		return(StepDataList);
	}
	return(StepDataList);
}

std::list<Ignite::BarData> EngineFileHandler::GetPatternsBarList(std::string PatternName)
{
	std::list<BarData> barDataList;
	int8_t version = 0;
	const fs::path PatternPath{ patterns_path / PatternName };
	for (const auto& file : fs::directory_iterator(PatternPath))
	{
		BarData bardata = { 0 };

		if (file.is_directory())
		{
			std::string InfoName = (file.path().stem().string() + "/" + (INFO_FILE_NAME + FILE_FORMAT));
			fs::path filepath = { PatternPath / InfoName };
			juce::File barInfoFile(filepath.string());
			if (!barInfoFile.exists())
				return(barDataList);
			try {
				auto DataXml{ juce::XmlDocument::parse(barInfoFile) };
				if (!DataXml || (!DataXml->hasTagName("pattern_bar")))
				{
					isDataCorrupted = true;
					return barDataList;
				}
				for (auto* e : DataXml->getChildWithTagNameIterator("bar_info"))
				{
					version = (int8_t)e->getIntAttribute("FILE_VERSION");
					bardata.BarId = (int8_t)e->getIntAttribute("id");
					bardata.stepCount = (uint16_t)e->getIntAttribute("stepCount");
					bardata.barRepeatCount = (uint8_t)e->getIntAttribute("barRepeatCount");
					bardata.barTransposeDefeat = e->getBoolAttribute("barTransposeDefeat");
					bardata.gBarSyncOn = e->getBoolAttribute("gBarSyncOn");
					bardata.direction = static_cast<PatternDirection>(e->getIntAttribute("direction"));
					barDataList.push_back(bardata);
				}
			}
			catch (...) {
				return(barDataList);
			}
		}
	}
	return barDataList;
}

std::list<std::string> EngineFileHandler::GetPatternsStepList(std::string PatternName, uint16_t BarId)
{
	std::list<std::string> StepList;
	std::string barFD = ("BAR-" + std::to_string(BarId));

	const fs::path PatternStepPath{ patterns_path / PatternName / barFD };
	for (const auto& file : fs::directory_iterator(PatternStepPath))
	{
		if ((!file.is_directory()) && (file.path().stem().string().find("STEP-") != std::string::npos))
		{
			StepList.push_back(file.path().stem().string());
		}
	}
	return StepList;
}

std::list<std::string> EngineFileHandler::GetPatternNameList()
{
	std::list<std::string> patternNameList;
	const fs::path PatternPath{ patterns_path };
	std::string PatternFileData;

	for (const auto& file : fs::directory_iterator(PatternPath))
	{
		if (file.is_directory())
		{
			patternNameList.push_back(file.path().stem().string());
		}
	}
	return patternNameList;
}

FileHandlerError EngineFileHandler::RenamePattern(std::string PatternName, std::string PatternNewName)
{
	if ((exists(patterns_path / PatternName)) && !exists(patterns_path / PatternNewName))
	{
		fs::rename(patterns_path / PatternName, patterns_path / PatternNewName);
		return FILE_OK;
	}
	else
		return FILE_EXISTS;
}

FileHandlerError EngineFileHandler::DeletePattern(std::string PatternName)
{
    if (!SafeFileIO::isLeafName(PatternName)) return FILE_ERROR;
    fs::remove_all(root_work_path / ".pattern-save" / "backups" / PatternName);
    return ((fs::remove_all(patterns_path / PatternName)) ? FILE_OK : FILE_ERROR);
}

FileHandlerError EngineFileHandler::DeleteAllPattern()
{
    fs::remove_all(root_work_path / ".pattern-save");
    FileHandlerError ret = (fs::remove_all(patterns_path)) ? FILE_OK : FILE_ERROR;
	exists(patterns_path) ? 0 : fs::create_directory(patterns_path);
	return ret;
}

FileHandlerError EngineFileHandler::saveInstrument(std::string InstrumentName, InstrumentData instrumentData)
{
	if (!exists(instruments_path))
		return FILE_ERROR;

    if (!SafeFileIO::isLeafName(instrumentData.instrumentName)) return FILE_ERROR;

	XmlElement DataXml("Instrument");
	XmlElement* DataInfo = new XmlElement("Data");

	DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
	DataInfo->setAttribute("id", instrumentData.id);
	DataInfo->setAttribute("instrumentName", instrumentData.instrumentName);
	DataInfo->setAttribute("instrumentDispName", instrumentData.instrumentDispName);
	DataInfo->setAttribute("midiPort", instrumentData.midiPort);
	DataInfo->setAttribute("midiChannel", instrumentData.midiChannel);
	DataInfo->setAttribute("defaultNote", instrumentData.defaultNote);
	DataInfo->setAttribute("internalSoundPgm", instrumentData.internalSoundPgm);
    DataInfo->setAttribute("soundFontPath",juce::String::fromUTF8(instrumentData.soundFontPath.c_str()));
	DataInfo->setAttribute("internalSound", instrumentData.internalSound);
	DataInfo->setAttribute("multiTimbralStatus", instrumentData.multiTimbralStatus);
	DataInfo->setAttribute("polySpreadStatus", instrumentData.polySpreadStatus);
	DataInfo->setAttribute("transposeDefeatStatus", instrumentData.transposeDefeatStatus);
	DataInfo->setAttribute("ftsDefeatStatus", instrumentData.ftsDefeatStatus);
	DataInfo->setAttribute("editTrackThruStatus", instrumentData.editTrackThruStatus);
	DataInfo->setAttribute("cc0BankMStatus", instrumentData.cc0BankMStatus);
	DataInfo->setAttribute("cc0BankLStatus", instrumentData.cc0BankLStatus);
	DataInfo->setAttribute("displayNoteNum", instrumentData.displayNoteNum);
	DataInfo->setAttribute("preSendPgmChange", instrumentData.preSendPgmChange);
	DataInfo->setAttribute("UsedByCount", (int)instrumentData.UsedByCount);
	DataInfo->setAttribute("tm_pan", instrumentData.tm.pan);
	DataInfo->setAttribute("tm_volume", instrumentData.tm.volume);
	DataInfo->setAttribute("tvCount", instrumentData.tvCount);
	DataInfo->setAttribute("pgmChangeData_bankLSB", instrumentData.pgmChangeData.bankLSB);
	DataInfo->setAttribute("pgmChangeData_bankMSB", instrumentData.pgmChangeData.bankMSB);
	DataInfo->setAttribute("pgmChangeData_midiPgmNum", instrumentData.pgmChangeData.midiPgmNum);

	if ((instrumentData.tvCount > 0) && (instrumentData.tv.size() > 0))
	{
		for (auto& tvIter : instrumentData.tv)
		{
			XmlElement* tvData = new XmlElement("tv");
			tvData->setAttribute("knobId", tvIter.second.knobId);
			tvData->setAttribute("name", tvIter.second.name);
			tvData->setAttribute("paramType", static_cast <uint8_t>(tvIter.second.paramType));
			tvData->setAttribute("savedAt", static_cast <uint8_t>(tvIter.second.savedAt));

			switch (tvIter.second.paramType)
			{
			case TrackValueType::TVTypeMidiCC:
				tvData->setAttribute("tvMidiCC_minValue", tvIter.second.param.tvMidiCC.minValue);
				tvData->setAttribute("tvMidiCC_maxValue", tvIter.second.param.tvMidiCC.maxValue);
				tvData->setAttribute("tvMidiCC_num", tvIter.second.param.tvMidiCC.num);
				tvData->setAttribute("tvMidiCC_initValue", tvIter.second.param.tvMidiCC.initValue);
				tvData->setAttribute("tvMidiCCvalue", tvIter.second.param.tvMidiCC.value);//value
				break;

			case TrackValueType::TVTypePgmChange:
				//tvData->setAttribute("tvPgmChange_bankLSB", tvIter.second.param.tvPgmChange.bankLSB);
				//tvData->setAttribute("tvPgmChange_bankMSB", tvIter.second.param.tvPgmChange.bankMSB);
				//tvData->setAttribute("tvPgmChange_midiPgmNum", tvIter.second.param.tvPgmChange.midiPgmNum);
				break;
			}
			DataInfo->addChildElement(tvData);
		}
	}
	if (instrumentData.UsedByCount && !instrumentData.usedByList.empty())
	{
		for (auto UsedByIt = instrumentData.usedByList.begin(); UsedByIt != instrumentData.usedByList.end(); ++UsedByIt)
		{
			XmlElement* usedbyData = new XmlElement("usedby");
			usedbyData->setAttribute("isFillPattern", UsedByIt->isFillPattern);
			usedbyData->setAttribute("songName", UsedByIt->songName);
			usedbyData->setAttribute("sceneName", UsedByIt->sceneName);
			usedbyData->setAttribute("trackName", UsedByIt->trackName);
			DataInfo->addChildElement(usedbyData);
		}
	}
	DataXml.addChildElement(DataInfo);
    return SafeFileIO::writeFile(instruments_path / (instrumentData.instrumentName + FILE_FORMAT),
        [&](std::ostream& output) { output << DataXml.toString(); }) ? FILE_OK : FILE_ERROR;
}

InstrumentData EngineFileHandler::GetInstrumentData(std::string InstrumentName)
{
	InstrumentData instrumentData = { {}, };
	std::string buf = { 0 };
	int8_t version = 0;
	fs::path InstrumentFile{ instruments_path / InstrumentName += FILE_FORMAT };

	if (!exists(InstrumentFile))
	{
		isDataCorrupted = true;
		return instrumentData;
	}
	instrumentData.instrumentName = InstrumentName;
	juce::File infofile(InstrumentFile.string());

	if (!infofile.exists())
		return instrumentData;
	try {
		auto DataXml{ juce::XmlDocument::parse(infofile) };

		if (!DataXml || (!DataXml->hasTagName("Instrument")))
		{
			isDataCorrupted = true;
			return instrumentData;
		}

		for (auto* e : DataXml->getChildIterator())
		{
			if (e->hasTagName("Data"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				instrumentData.id = e->getIntAttribute("id");
				instrumentData.instrumentName = e->getStringAttribute("instrumentName").toStdString();
				instrumentData.instrumentDispName = e->getStringAttribute("instrumentDispName").toStdString();
				instrumentData.midiPort = (int8_t)e->getIntAttribute("midiPort");
				instrumentData.midiChannel = (int8_t)e->getIntAttribute("midiChannel");
				instrumentData.defaultNote = (uint8_t)e->getIntAttribute("defaultNote");
				instrumentData.internalSoundPgm = e->getIntAttribute("internalSoundPgm");
                instrumentData.soundFontPath=e->getStringAttribute("soundFontPath").toStdString();
				instrumentData.internalSound = e->getBoolAttribute("internalSound");
				instrumentData.multiTimbralStatus = e->getBoolAttribute("multiTimbralStatus");
				instrumentData.polySpreadStatus = e->getBoolAttribute("polySpreadStatus");
				instrumentData.transposeDefeatStatus = e->getBoolAttribute("transposeDefeatStatus");
				instrumentData.ftsDefeatStatus = e->getBoolAttribute("ftsDefeatStatus");
				instrumentData.editTrackThruStatus = e->getBoolAttribute("editTrackThruStatus");
				instrumentData.cc0BankMStatus = e->getBoolAttribute("cc0BankMStatus");
				instrumentData.cc0BankLStatus = e->getBoolAttribute("cc0BankLStatus");
				instrumentData.displayNoteNum = e->getBoolAttribute("displayNoteNum");
				instrumentData.preSendPgmChange = e->getBoolAttribute("preSendPgmChange");
				instrumentData.UsedByCount = e->getIntAttribute("UsedByCount");
				instrumentData.tm.pan = (int8_t)e->getIntAttribute("tm_pan");
				instrumentData.tm.volume = (int8_t)e->getIntAttribute("tm_volume");
				instrumentData.tvCount = (uint8_t)e->getIntAttribute("tvCount");
				instrumentData.pgmChangeData.bankLSB = (int8_t)e->getIntAttribute("pgmChangeData_bankLSB");
				instrumentData.pgmChangeData.bankMSB = (int8_t)e->getIntAttribute("pgmChangeData_bankMSB");
				instrumentData.pgmChangeData.midiPgmNum = (int8_t)e->getIntAttribute("pgmChangeData_midiPgmNum");
			}

			if (instrumentData.tvCount > 0)
			{
				instrumentData.tv.clear();
				TrackValue tvTemp;
				for (auto* f : e->getChildWithTagNameIterator("tv"))
				{
					tvTemp.knobId = (int8_t)f->getIntAttribute("knobId");
					tvTemp.name = f->getStringAttribute("name").toStdString();
					tvTemp.paramType = static_cast<TrackValueType>(f->getIntAttribute("paramType"));
					tvTemp.savedAt = static_cast<TrackValueScope>(f->getIntAttribute("savedAt"));

					switch (tvTemp.paramType)
					{
					case TrackValueType::TVTypeMidiCC:

						tvTemp.param.tvMidiCC.minValue = (int8_t)f->getIntAttribute("tvMidiCC_minValue");
						tvTemp.param.tvMidiCC.maxValue = (int8_t)f->getIntAttribute("tvMidiCC_maxValue");
						tvTemp.param.tvMidiCC.num = (int8_t)f->getIntAttribute("tvMidiCC_num");
						tvTemp.param.tvMidiCC.initValue = (int8_t)f->getIntAttribute("tvMidiCC_initValue");
						tvTemp.param.tvMidiCC.value = static_cast<float>(f->getDoubleAttribute("tvMidiCCvalue"));//value
						break;

					case TrackValueType::TVTypePgmChange:
						//tvTemp.param.tvPgmChange.bankLSB = std::stoi (buf);
						//tvTemp.param.tvPgmChange.bankMSB = std::stoi (buf);
						//tvTemp.param.tvPgmChange.midiPgmNum = std::stoi (buf);
						break;
					}
					instrumentData.tv.emplace(tvTemp.knobId, tvTemp);
				}
			}

			if (instrumentData.UsedByCount)
			{
				UsedByData usedByData{};
				for (auto* g : e->getChildWithTagNameIterator("usedby"))
				{
					usedByData.isFillPattern = g->getIntAttribute("isFillPattern");
					usedByData.songName = g->getStringAttribute("songName").toStdString();
					usedByData.sceneName = g->getStringAttribute("sceneName").toStdString();
					usedByData.trackName = g->getStringAttribute("trackName").toStdString();
					instrumentData.usedByList.push_back(usedByData);
				}
			}
		}
	}
	catch (...) {
		return instrumentData;
	}
	return instrumentData;
}

FileHandlerError EngineFileHandler::DeleteInstrument(std::string InstrumentName)
{
	if (fs::remove(instruments_path / (InstrumentName + FILE_FORMAT)))
	{
		return FILE_OK;
	}
	return FILE_ERROR;
}

uint16_t EngineFileHandler::GetNumberofInstruments()
{
	const fs::path instrumentsPath{ instruments_path };
	uint16_t Count = 0;

	for (const auto& file : fs::directory_iterator(instrumentsPath))
	{
		if (file.path().extension() == FILE_FORMAT &&
			file.path().stem().string() != INFO_FILE_NAME)
		{
			++Count;
		}
	}
	return Count;
}

std::vector<std::string>  EngineFileHandler::getInstrumentNameList()
{
	std::vector<std::string>  instrumentNameList;
	const fs::path instrumentsPath{ instruments_path };
	for (const auto& file : fs::directory_iterator(instrumentsPath))
	{
		if (file.path().extension() == FILE_FORMAT &&
			file.path().stem().string() != INFO_FILE_NAME)
		{
			instrumentNameList.push_back((file.path().stem().string()));
		}
	}
	return instrumentNameList;
}

FileHandlerError EngineFileHandler::RenameInstrument(std::string instrumentName, std::string instrumentNewName)
{
	if ((exists(instruments_path / (instrumentName + FILE_FORMAT))) && !exists(instruments_path / (instrumentNewName + FILE_FORMAT)))
	{
		fs::rename((instruments_path / (instrumentName + FILE_FORMAT)),
			(instruments_path / (instrumentNewName + FILE_FORMAT)));
		return FILE_OK;
	}
	else
		return FILE_EXISTS;
}

FileHandlerError EngineFileHandler::DeleteAllInstrument()
{
	FileHandlerError ret = (fs::remove_all(instruments_path)) ? FILE_OK : FILE_ERROR;
	exists(instruments_path) ? 0 : fs::create_directory(instruments_path);
	return ret;
}
//------------------ Setting Files

FileHandlerError EngineFileHandler::UpdateMidiCnfgSetting(SettingsMidiCnfgData settingsMidiCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (MIDICNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("MidiCnfg");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);

		for (int8_t i = 0; i < MAX_MIDICONFIG_PORT; i++)
		{
			XmlElement* DataMidi = new XmlElement("DataMidiCnfg");
			DataMidi->setAttribute("sourcePort", settingsMidiCnfgData.MidiCnfg[i].sourcePort);
			DataMidi->setAttribute("type", static_cast <uint8_t>(settingsMidiCnfgData.MidiCnfg[i].type));
			DataMidi->setAttribute("destPort", settingsMidiCnfgData.MidiCnfg[i].destPort);
			DataMidi->setAttribute("destChannel", settingsMidiCnfgData.MidiCnfg[i].destChannel);
			DataMidi->setAttribute("thruFTSEnabled", settingsMidiCnfgData.MidiCnfg[i].thruFTSEnabled);
			DataMidi->setAttribute("midiClkTxEnabled", settingsMidiCnfgData.MidiCnfg[i].midiClkTxEnabled);
			DataInfo->addChildElement(DataMidi);
		}
		DataInfo->setAttribute("DataLocation", settingsMidiCnfgData.DataLocation);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
std::list <MidiConfig> EngineFileHandler::GetMidiCnfgSetting(std::string SongName)
{
	std::list <MidiConfig> _midiCnfg;
	MidiConfig _midiCnfgData = { 0, };
	std::string buf = { 0 };

	int8_t version = 0;

	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (MIDICNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());

		if (!infofile.exists())
			return _midiCnfg;

		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("MidiCnfg"))
			{
				isDataCorrupted = true;
				return _midiCnfg;
			}

			for (auto* f : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)f->getIntAttribute("FILE_VERSION");

				for (auto* e : f->getChildWithTagNameIterator("DataMidiCnfg"))
				{
					_midiCnfgData = { 0, };
					_midiCnfgData.sourcePort = (int8_t)e->getIntAttribute("sourcePort");
					_midiCnfgData.type = (SoftThruType)e->getIntAttribute("type");
					_midiCnfgData.destPort = (int8_t)e->getIntAttribute("destPort");
					_midiCnfgData.destChannel = (int8_t)e->getIntAttribute("destChannel");
					_midiCnfgData.thruFTSEnabled = (int8_t)e->getIntAttribute("thruFTSEnabled");
					_midiCnfgData.midiClkTxEnabled = (int8_t)e->getIntAttribute("midiClkTxEnabled");
					_midiCnfg.push_back(_midiCnfgData);
				}
			}
			return _midiCnfg;
		}
		catch (...) {
			return _midiCnfg;
		}
	}
	return _midiCnfg;
}

FileHandlerError EngineFileHandler::UpdateCvioCnfgSetting(SettingsCvioCnfgData settingsCvioCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (CVIOCNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("CvioCnfgS");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);

		for (int8_t i = 0; i < MAX_CVIO_PORTS; ++i)
		{
			XmlElement* DataCvio = new XmlElement("DataCvioCnfg");
			DataCvio->setAttribute("portNum", (settingsCvioCnfgData.cvConfig[i].portNum));
			//DataCvio->setAttribute("channelNum", (settingsCvioCnfgData.cvConfig[i].channelNum));
			DataCvio->setAttribute("law", (int8_t)(settingsCvioCnfgData.cvConfig[i].law));
			//DataCvio->setAttribute("gateType", (int8_t)(settingsCvioCnfgData.cvConfig[i].gateType));
			/*DataCvio->setAttribute("slideType", (int8_t)(settingsCvioCnfgData.cvConfig[i].slideType));
			DataCvio->setAttribute("slideTime", (settingsCvioCnfgData.cvConfig[i].slideTime));*/
			DataCvio->setAttribute("noteRangeStart", (settingsCvioCnfgData.cvConfig[i].noteRangeStart));
			DataCvio->setAttribute("noteRangeEnd", (settingsCvioCnfgData.cvConfig[i].noteRangeEnd));
			DataCvio->setAttribute("notePercent", (settingsCvioCnfgData.cvConfig[i].notePercent));
			DataCvio->setAttribute("velocityPercent", (settingsCvioCnfgData.cvConfig[i].velocityPercent));
			DataCvio->setAttribute("ccNum", (settingsCvioCnfgData.cvConfig[i].ccNum));
			DataCvio->setAttribute("ccPercent", (settingsCvioCnfgData.cvConfig[i].ccPercent));
			DataCvio->setAttribute("pitchBend", (settingsCvioCnfgData.cvConfig[i].pitchBend));
			DataCvio->setAttribute("voltage", (int8_t)(settingsCvioCnfgData.cvConfig[i].voltage));
			DataCvio->setAttribute("EnvAttack", (settingsCvioCnfgData.cvConfig[i].EnvAttack));
			DataCvio->setAttribute("EnvAttackMin", (settingsCvioCnfgData.cvConfig[i].EnvAttackMin));
			DataCvio->setAttribute("EnvAttackMax", (settingsCvioCnfgData.cvConfig[i].EnvAttackMax));
			DataCvio->setAttribute("EnvAttackName", settingsCvioCnfgData.cvConfig[i].EnvAttackName);
			DataCvio->setAttribute("EnvDecay", (settingsCvioCnfgData.cvConfig[i].EnvDecay));
			DataCvio->setAttribute("EnvDecayMin", (settingsCvioCnfgData.cvConfig[i].EnvDecayMin));
			DataCvio->setAttribute("EnvDecayMax", (settingsCvioCnfgData.cvConfig[i].EnvDecayMax));
			DataCvio->setAttribute("EnvDecayName", settingsCvioCnfgData.cvConfig[i].EnvDecayName);
			DataCvio->setAttribute("EnvSustain", (settingsCvioCnfgData.cvConfig[i].EnvSustain));
			DataCvio->setAttribute("EnvSustainMin", (settingsCvioCnfgData.cvConfig[i].EnvSustainMin));
			DataCvio->setAttribute("EnvSustainMax", (settingsCvioCnfgData.cvConfig[i].EnvSustainMax));
			DataCvio->setAttribute("EnvSustainName", settingsCvioCnfgData.cvConfig[i].EnvSustainName);
			DataCvio->setAttribute("EnvRelease", (settingsCvioCnfgData.cvConfig[i].EnvRelease));
			DataCvio->setAttribute("EnvReleaseMin", (settingsCvioCnfgData.cvConfig[i].EnvReleaseMin));
			DataCvio->setAttribute("EnvReleaseMax", (settingsCvioCnfgData.cvConfig[i].EnvReleaseMax));
			DataCvio->setAttribute("EnvReleaseName", settingsCvioCnfgData.cvConfig[i].EnvReleaseName);
			DataCvio->setAttribute("EnvAmount", (settingsCvioCnfgData.cvConfig[i].EnvAmount));
			DataCvio->setAttribute("EnvAmountMin", (settingsCvioCnfgData.cvConfig[i].EnvAmountMin));
			DataCvio->setAttribute("EnvAmountMax", (settingsCvioCnfgData.cvConfig[i].EnvAmountMax));
			DataCvio->setAttribute("EnvAmountName", settingsCvioCnfgData.cvConfig[i].EnvAmountName);
			DataCvio->setAttribute("EnvVelocity", (settingsCvioCnfgData.cvConfig[i].EnvVelocity));
			DataCvio->setAttribute("EnvVelocityMin", (settingsCvioCnfgData.cvConfig[i].EnvVelocityMin));
			DataCvio->setAttribute("EnvVelocityMax", (settingsCvioCnfgData.cvConfig[i].EnvVelocityMax));
			DataCvio->setAttribute("EnvVelocityName", settingsCvioCnfgData.cvConfig[i].EnvVelocityName);
			DataCvio->setAttribute("cvNoteType", (settingsCvioCnfgData.cvConfig[i].cvNoteType));
			DataInfo->addChildElement(DataCvio);
		}
		for (int8_t i = 0; i < MAX_GATE_PORTS; ++i)
		{
			XmlElement* DataGate = new XmlElement("DataGateCnfg");
			DataGate->setAttribute("portNum", (settingsCvioCnfgData.gateConfig[i].portNum));
			DataGate->setAttribute("channelNum", (settingsCvioCnfgData.gateConfig[i].channelNum));
			DataGate->setAttribute("type", (int8_t)(settingsCvioCnfgData.gateConfig[i].type));
			DataGate->setAttribute("mode", (int8_t)(settingsCvioCnfgData.gateConfig[i].mode));
			DataGate->setAttribute("output", (int8_t)(settingsCvioCnfgData.gateConfig[i].output));
			DataInfo->addChildElement(DataGate);
		}
		DataInfo->setAttribute("DataLocation", settingsCvioCnfgData.DataLocation);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}

SettingsCvioCnfgData EngineFileHandler::GetCvioCnfgSetting(std::string SongName)
{
	std::string buf = { 0 };
	CV _cv;
	Gate _gate;
	SettingsCvioCnfgData _settingsCvioCnfgData;
	int8_t version = 0;

	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}

		fs::path Settingfilepath = (filepath / (CVIOCNFG_FILE_NAME + FILE_FORMAT));
		juce::File infofile(Settingfilepath.string());

		if (!infofile.exists())
			return _settingsCvioCnfgData;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("CvioCnfgS"))
			{
				isDataCorrupted = true;
				return _settingsCvioCnfgData;
			}

			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				_settingsCvioCnfgData.DataLocation = (int8_t)e->getIntAttribute("DataLocation");

				for (auto* f : e->getChildWithTagNameIterator("DataCvioCnfg"))
				{
					_cv = { 0, };
					_cv.portNum = (int8_t)f->getIntAttribute("portNum");
					_cv.channelNum = (int8_t)f->getIntAttribute("channelNum");
					_cv.law = (CvLaw)e->getIntAttribute("law");
					_cv.gateType = (CvGateType)e->getIntAttribute("gateType");
					_cv.slideType = (CvSlideType)e->getIntAttribute("slideType");
					_cv.slideTime = f->getIntAttribute("slideTime");
					_cv.noteRangeStart = (int8_t)f->getIntAttribute("noteRangeStart");
					_cv.noteRangeEnd = (int8_t)f->getIntAttribute("noteRangeEnd");
					_cv.notePercent = (int8_t)f->getIntAttribute("notePercent");
					_cv.velocityPercent = (int8_t)f->getIntAttribute("velocityPercent");
					_cv.ccNum = (int8_t)f->getIntAttribute("ccNum");
					_cv.ccPercent = (int8_t)f->getIntAttribute("ccPercent");
					_cv.pitchBend = (int16_t)f->getIntAttribute("pitchBend");
					_cv.voltage = (CVPeakVoltage)f->getIntAttribute("voltage");
					_cv.EnvAttack = (int8_t)f->getIntAttribute("EnvAttack");
					_cv.EnvAttackMin = (int8_t)f->getIntAttribute("EnvAttackMin");
					_cv.EnvAttackMax = (int8_t)f->getIntAttribute("EnvAttackMax");
					_cv.EnvAttackName = f->getStringAttribute("EnvAttackName").toStdString();
					_cv.EnvDecay = (int8_t)f->getIntAttribute("EnvDecay");
					_cv.EnvDecayMin = (int8_t)f->getIntAttribute("EnvDecayMin");
					_cv.EnvDecayMax = (int8_t)f->getIntAttribute("EnvDecayMax");
					_cv.EnvDecayName = f->getStringAttribute("EnvDecayName").toStdString();
					_cv.EnvSustain = (int8_t)f->getIntAttribute("EnvSustain");
					_cv.EnvSustainMin = (int8_t)f->getIntAttribute("EnvSustainMin");
					_cv.EnvSustainMax = (int8_t)f->getIntAttribute("EnvSustainMax");
					_cv.EnvSustainName = f->getStringAttribute("EnvSustainName").toStdString();
					_cv.EnvRelease = (int8_t)f->getIntAttribute("EnvRelease");
					_cv.EnvReleaseMin = (int8_t)f->getIntAttribute("EnvReleaseMin");
					_cv.EnvReleaseMax = (int8_t)f->getIntAttribute("EnvReleaseMax");
					_cv.EnvReleaseName = f->getStringAttribute("EnvReleaseName").toStdString();
					_cv.EnvAmount = (int8_t)f->getIntAttribute("EnvAmount");
					_cv.EnvAmountMin = (int8_t)f->getIntAttribute("EnvAmountMin");
					_cv.EnvAmountMax = (int8_t)f->getIntAttribute("EnvAmountMax");
					_cv.EnvAmountName = f->getStringAttribute("EnvAmountName").toStdString();
					_cv.EnvVelocity = (int8_t)f->getIntAttribute("EnvVelocity");
					_cv.EnvVelocityMin = (int8_t)f->getIntAttribute("EnvVelocityMin");
					_cv.EnvVelocityMax = (int8_t)f->getIntAttribute("EnvVelocityMax");
					_cv.EnvVelocityName = f->getStringAttribute("EnvVelocityName").toStdString();
					_cv.cvNoteType = (uint8_t)f->getIntAttribute("cvNoteType");
					_settingsCvioCnfgData.cvConfig.insert({ _cv.portNum , _cv });
				}
				for (auto* g : e->getChildWithTagNameIterator("DataGateCnfg"))
				{
					_gate = { 0, };
					_gate.portNum = (int8_t)g->getIntAttribute("portNum");
					_gate.channelNum = (int8_t)g->getIntAttribute("channelNum");
					_gate.type = (GateType)g->getIntAttribute("type");
					_gate.mode = (CvGateType)g->getIntAttribute("mode");
					_gate.output = (gatePeakVoltage)g->getIntAttribute("output");
					_settingsCvioCnfgData.gateConfig.insert({ _gate.portNum , _gate });
				}
			}
		}
		catch (...) {
			UpdateCvioCnfgSetting(_settingsCvioCnfgData, SongName);
			return _settingsCvioCnfgData;
		}
	}
	return _settingsCvioCnfgData;
}

FileHandlerError EngineFileHandler::UpdateCalibData(Array <CalibrationData>* calibData)
{
	if (exists(calib_root_path))
	{
		mountDataPart(false);

        const auto savePath=calib_root_path / (CALIBRATION_OFFSET_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("CalibOffset");
		XmlElement* DataInfo = new XmlElement("offset");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);

		DataInfo->setAttribute("CV0CalibOffset1", calibData->getReference(0).calibOffset[0]);
		DataInfo->setAttribute("CV0CalibOffset2", calibData->getReference(0).calibOffset[1]);
		DataInfo->setAttribute("CV0CalibOffset3", calibData->getReference(0).calibOffset[2]);
		DataInfo->setAttribute("CV0CalibOffset4", calibData->getReference(0).calibOffset[3]);
		DataInfo->setAttribute("CV0CalibOffset5", calibData->getReference(0).calibOffset[4]);
		DataInfo->setAttribute("CV0CalibOffset6", calibData->getReference(0).calibOffset[5]);
		DataInfo->setAttribute("CV0CalibOffset7", calibData->getReference(0).calibOffset[6]);
		DataInfo->setAttribute("CV0CalibOffset8", calibData->getReference(0).calibOffset[7]);
		DataInfo->setAttribute("CV0CalibOffset9", calibData->getReference(0).calibOffset[8]);
		DataInfo->setAttribute("CV0CalibOffset10", calibData->getReference(0).calibOffset[9]);

		DataInfo->setAttribute("CV1CalibOffset1", calibData->getReference(1).calibOffset[0]);
		DataInfo->setAttribute("CV1CalibOffset2", calibData->getReference(1).calibOffset[1]);
		DataInfo->setAttribute("CV1CalibOffset3", calibData->getReference(1).calibOffset[2]);
		DataInfo->setAttribute("CV1CalibOffset4", calibData->getReference(1).calibOffset[3]);
		DataInfo->setAttribute("CV1CalibOffset5", calibData->getReference(1).calibOffset[4]);
		DataInfo->setAttribute("CV1CalibOffset6", calibData->getReference(1).calibOffset[5]);
		DataInfo->setAttribute("CV1CalibOffset7", calibData->getReference(1).calibOffset[6]);
		DataInfo->setAttribute("CV1CalibOffset8", calibData->getReference(1).calibOffset[7]);
		DataInfo->setAttribute("CV1CalibOffset9", calibData->getReference(1).calibOffset[8]);
		DataInfo->setAttribute("CV1CalibOffset10", calibData->getReference(1).calibOffset[9]);

		DataInfo->setAttribute("CV2CalibOffset1", calibData->getReference(2).calibOffset[0]);
		DataInfo->setAttribute("CV2CalibOffset2", calibData->getReference(2).calibOffset[1]);
		DataInfo->setAttribute("CV2CalibOffset3", calibData->getReference(2).calibOffset[2]);
		DataInfo->setAttribute("CV2CalibOffset4", calibData->getReference(2).calibOffset[3]);
		DataInfo->setAttribute("CV2CalibOffset5", calibData->getReference(2).calibOffset[4]);
		DataInfo->setAttribute("CV2CalibOffset6", calibData->getReference(2).calibOffset[5]);
		DataInfo->setAttribute("CV2CalibOffset7", calibData->getReference(2).calibOffset[6]);
		DataInfo->setAttribute("CV2CalibOffset8", calibData->getReference(2).calibOffset[7]);
		DataInfo->setAttribute("CV2CalibOffset9", calibData->getReference(2).calibOffset[8]);
		DataInfo->setAttribute("CV2CalibOffset10", calibData->getReference(2).calibOffset[9]);

		DataInfo->setAttribute("CV3CalibOffset1", calibData->getReference(3).calibOffset[0]);
		DataInfo->setAttribute("CV3CalibOffset2", calibData->getReference(3).calibOffset[1]);
		DataInfo->setAttribute("CV3CalibOffset3", calibData->getReference(3).calibOffset[2]);
		DataInfo->setAttribute("CV3CalibOffset4", calibData->getReference(3).calibOffset[3]);
		DataInfo->setAttribute("CV3CalibOffset5", calibData->getReference(3).calibOffset[4]);
		DataInfo->setAttribute("CV3CalibOffset6", calibData->getReference(3).calibOffset[5]);
		DataInfo->setAttribute("CV3CalibOffset7", calibData->getReference(3).calibOffset[6]);
		DataInfo->setAttribute("CV3CalibOffset8", calibData->getReference(3).calibOffset[7]);
		DataInfo->setAttribute("CV3CalibOffset9", calibData->getReference(3).calibOffset[8]);
		DataInfo->setAttribute("CV3CalibOffset10", calibData->getReference(3).calibOffset[9]);

		DataInfo->setAttribute("CV4CalibOffset1", calibData->getReference(4).calibOffset[0]);
		DataInfo->setAttribute("CV4CalibOffset2", calibData->getReference(4).calibOffset[1]);
		DataInfo->setAttribute("CV4CalibOffset3", calibData->getReference(4).calibOffset[2]);
		DataInfo->setAttribute("CV4CalibOffset4", calibData->getReference(4).calibOffset[3]);
		DataInfo->setAttribute("CV4CalibOffset5", calibData->getReference(4).calibOffset[4]);
		DataInfo->setAttribute("CV4CalibOffset6", calibData->getReference(4).calibOffset[5]);
		DataInfo->setAttribute("CV4CalibOffset7", calibData->getReference(4).calibOffset[6]);
		DataInfo->setAttribute("CV4CalibOffset8", calibData->getReference(4).calibOffset[7]);
		DataInfo->setAttribute("CV4CalibOffset9", calibData->getReference(4).calibOffset[8]);
		DataInfo->setAttribute("CV4CalibOffset10", calibData->getReference(4).calibOffset[9]);

		DataInfo->setAttribute("CV5CalibOffset1", calibData->getReference(5).calibOffset[0]);
		DataInfo->setAttribute("CV5CalibOffset2", calibData->getReference(5).calibOffset[1]);
		DataInfo->setAttribute("CV5CalibOffset3", calibData->getReference(5).calibOffset[2]);
		DataInfo->setAttribute("CV5CalibOffset4", calibData->getReference(5).calibOffset[3]);
		DataInfo->setAttribute("CV5CalibOffset5", calibData->getReference(5).calibOffset[4]);
		DataInfo->setAttribute("CV5CalibOffset6", calibData->getReference(5).calibOffset[5]);
		DataInfo->setAttribute("CV5CalibOffset7", calibData->getReference(5).calibOffset[6]);
		DataInfo->setAttribute("CV5CalibOffset8", calibData->getReference(5).calibOffset[7]);
		DataInfo->setAttribute("CV5CalibOffset9", calibData->getReference(5).calibOffset[8]);
		DataInfo->setAttribute("CV5CalibOffset10", calibData->getReference(5).calibOffset[9]);

		DataInfo->setAttribute("CV6CalibOffset1", calibData->getReference(6).calibOffset[0]);
		DataInfo->setAttribute("CV6CalibOffset2", calibData->getReference(6).calibOffset[1]);
		DataInfo->setAttribute("CV6CalibOffset3", calibData->getReference(6).calibOffset[2]);
		DataInfo->setAttribute("CV6CalibOffset4", calibData->getReference(6).calibOffset[3]);
		DataInfo->setAttribute("CV6CalibOffset5", calibData->getReference(6).calibOffset[4]);
		DataInfo->setAttribute("CV6CalibOffset6", calibData->getReference(6).calibOffset[5]);
		DataInfo->setAttribute("CV6CalibOffset7", calibData->getReference(6).calibOffset[6]);
		DataInfo->setAttribute("CV6CalibOffset8", calibData->getReference(6).calibOffset[7]);
		DataInfo->setAttribute("CV6CalibOffset9", calibData->getReference(6).calibOffset[8]);
		DataInfo->setAttribute("CV6CalibOffset10", calibData->getReference(6).calibOffset[9]);

		DataInfo->setAttribute("CV7CalibOffset1", calibData->getReference(7).calibOffset[0]);
		DataInfo->setAttribute("CV7CalibOffset2", calibData->getReference(7).calibOffset[1]);
		DataInfo->setAttribute("CV7CalibOffset3", calibData->getReference(7).calibOffset[2]);
		DataInfo->setAttribute("CV7CalibOffset4", calibData->getReference(7).calibOffset[3]);
		DataInfo->setAttribute("CV7CalibOffset5", calibData->getReference(7).calibOffset[4]);
		DataInfo->setAttribute("CV7CalibOffset6", calibData->getReference(7).calibOffset[5]);
		DataInfo->setAttribute("CV7CalibOffset7", calibData->getReference(7).calibOffset[6]);
		DataInfo->setAttribute("CV7CalibOffset8", calibData->getReference(7).calibOffset[7]);
		DataInfo->setAttribute("CV7CalibOffset9", calibData->getReference(7).calibOffset[8]);
		DataInfo->setAttribute("CV7CalibOffset10", calibData->getReference(7).calibOffset[9]);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);

		mountDataPart(true);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}

Array <CalibrationData>* EngineFileHandler::GetCalibData()
{
	std::string buf = { 0 };
	int8_t version = 0;

	if (exists(calib_root_path))
	{
		fs::path Settingfilepath = (calib_root_path / (CALIBRATION_OFFSET_FILE_NAME + FILE_FORMAT));
		juce::File infofile(Settingfilepath.string());

		if (!infofile.exists())
			return &newCalibData;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("CalibOffset"))
			{
				isDataCorrupted = true;
				return &newCalibData;
			}

			for (auto* e : DataXml->getChildWithTagNameIterator("offset"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(0).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(0).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset0"));
				newCalibData.getReference(0).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset1"));
				newCalibData.getReference(0).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset2"));
				newCalibData.getReference(0).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset3"));
				newCalibData.getReference(0).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset4"));
				newCalibData.getReference(0).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset5"));
				newCalibData.getReference(0).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset6"));
				newCalibData.getReference(0).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset7"));
				newCalibData.getReference(0).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset8"));
				newCalibData.getReference(0).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV0CalibOffset9"));

				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(1).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(1).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset0"));
				newCalibData.getReference(1).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset1"));
				newCalibData.getReference(1).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset2"));
				newCalibData.getReference(1).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset3"));
				newCalibData.getReference(1).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset4"));
				newCalibData.getReference(1).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset5"));
				newCalibData.getReference(1).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset6"));
				newCalibData.getReference(1).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset7"));
				newCalibData.getReference(1).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset8"));
				newCalibData.getReference(1).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV1CalibOffset9"));

				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(2).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(2).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset0"));
				newCalibData.getReference(2).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset1"));
				newCalibData.getReference(2).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset2"));
				newCalibData.getReference(2).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset3"));
				newCalibData.getReference(2).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset4"));
				newCalibData.getReference(2).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset5"));
				newCalibData.getReference(2).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset6"));
				newCalibData.getReference(2).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset7"));
				newCalibData.getReference(2).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset8"));
				newCalibData.getReference(2).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV2CalibOffset9"));

				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(3).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(3).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset0"));
				newCalibData.getReference(3).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset1"));
				newCalibData.getReference(3).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset2"));
				newCalibData.getReference(3).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset3"));
				newCalibData.getReference(3).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset4"));
				newCalibData.getReference(3).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset5"));
				newCalibData.getReference(3).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset6"));
				newCalibData.getReference(3).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset7"));
				newCalibData.getReference(3).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset8"));
				newCalibData.getReference(3).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV3CalibOffset9"));

				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(4).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(4).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset0"));
				newCalibData.getReference(4).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset1"));
				newCalibData.getReference(4).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset2"));
				newCalibData.getReference(4).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset3"));
				newCalibData.getReference(4).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset4"));
				newCalibData.getReference(4).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset5"));
				newCalibData.getReference(4).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset6"));
				newCalibData.getReference(4).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset7"));
				newCalibData.getReference(4).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset8"));
				newCalibData.getReference(4).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV4CalibOffset9"));

				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(5).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(5).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset0"));
				newCalibData.getReference(5).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset1"));
				newCalibData.getReference(5).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset2"));
				newCalibData.getReference(5).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset3"));
				newCalibData.getReference(5).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset4"));
				newCalibData.getReference(5).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset5"));
				newCalibData.getReference(5).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset6"));
				newCalibData.getReference(5).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset7"));
				newCalibData.getReference(5).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset8"));
				newCalibData.getReference(5).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV5CalibOffset9"));

				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(6).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(6).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset0"));
				newCalibData.getReference(6).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset1"));
				newCalibData.getReference(6).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset2"));
				newCalibData.getReference(6).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset3"));
				newCalibData.getReference(6).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset4"));
				newCalibData.getReference(6).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset5"));
				newCalibData.getReference(6).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset6"));
				newCalibData.getReference(6).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset7"));
				newCalibData.getReference(6).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset8"));
				newCalibData.getReference(6).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV6CalibOffset9"));

				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				newCalibData.getReference(7).DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				newCalibData.getReference(7).calibOffset[0] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset0"));
				newCalibData.getReference(7).calibOffset[1] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset1"));
				newCalibData.getReference(7).calibOffset[2] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset2"));
				newCalibData.getReference(7).calibOffset[3] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset3"));
				newCalibData.getReference(7).calibOffset[4] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset4"));
				newCalibData.getReference(7).calibOffset[5] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset5"));
				newCalibData.getReference(7).calibOffset[6] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset6"));
				newCalibData.getReference(7).calibOffset[7] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset7"));
				newCalibData.getReference(7).calibOffset[8] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset8"));
				newCalibData.getReference(7).calibOffset[9] = static_cast<float>(e->getDoubleAttribute("CV7CalibOffset9"));
			}
		}
		catch (...) {
			UpdateCalibData(&newCalibData);
			return &newCalibData;
		}
	}
	return &newCalibData;
}

FileHandlerError EngineFileHandler::UpdateSettingsGeneralCnfgData(SettingsGeneralCnfgData settingsGeneralCnfgData)
{
	if (exists(settings_path))
	{

        const auto savePath=settings_path / (SETTINGCNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("GeneralCnfg");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("DeviceCnfgUSBParam", settingsGeneralCnfgData.DeviceCnfgUSBParam);
		DataInfo->setAttribute("DeviceCnfgSafeStopParam", settingsGeneralCnfgData.DeviceCnfgSafeStopParam);
		DataInfo->setAttribute("DeviceCnfgBrightnessParam", settingsGeneralCnfgData.DeviceCnfgBrightnessParam);
		DataInfo->setAttribute("DeviceCnfgAutoSaveAll", settingsGeneralCnfgData.DeviceCnfgAutoSaveAll);
		DataInfo->setAttribute("InstrumentAutoSave", settingsGeneralCnfgData.InstrumentAutoSave);
		DataInfo->setAttribute("auditioningMaxPort", settingsGeneralCnfgData.auditioningMaxPort);
		DataInfo->setAttribute("WifiState", settingsGeneralCnfgData.WifiState);
		DataInfo->setAttribute("LastConnectedWiFiSsid", settingsGeneralCnfgData.LastWifiConSsid);

		XmlElement* DataPreset = new XmlElement("auditioningPortPresetInfo");
		DataPreset->setAttribute("auditioningPortPreset1", settingsGeneralCnfgData.auditioningPortPreset[0]);
		DataPreset->setAttribute("auditioningPortPreset2", settingsGeneralCnfgData.auditioningPortPreset[1]);
		DataPreset->setAttribute("auditioningPortPreset3", settingsGeneralCnfgData.auditioningPortPreset[2]);
		DataPreset->setAttribute("auditioningPortPreset4", settingsGeneralCnfgData.auditioningPortPreset[3]);
		DataPreset->setAttribute("auditioningPortPreset5", settingsGeneralCnfgData.auditioningPortPreset[4]);
		DataPreset->setAttribute("auditioningPortPreset6", settingsGeneralCnfgData.auditioningPortPreset[5]);
		DataPreset->setAttribute("auditioningPortPreset7", settingsGeneralCnfgData.auditioningPortPreset[6]);
		DataPreset->setAttribute("auditioningPortPreset8", settingsGeneralCnfgData.auditioningPortPreset[7]);
		DataPreset->setAttribute("auditioningPortPreset9", settingsGeneralCnfgData.auditioningPortPreset[8]);
		DataPreset->setAttribute("auditioningPortPreset10", settingsGeneralCnfgData.auditioningPortPreset[9]);
		DataPreset->setAttribute("auditioningPortPreset11", settingsGeneralCnfgData.auditioningPortPreset[10]);
		DataPreset->setAttribute("auditioningPortPreset12", settingsGeneralCnfgData.auditioningPortPreset[11]);
		DataPreset->setAttribute("auditioningPortPreset13", settingsGeneralCnfgData.auditioningPortPreset[12]);
		DataPreset->setAttribute("auditioningPortPreset14", settingsGeneralCnfgData.auditioningPortPreset[13]);
		DataPreset->setAttribute("auditioningPortPreset15", settingsGeneralCnfgData.auditioningPortPreset[14]);
		DataPreset->setAttribute("auditioningPortPreset16", settingsGeneralCnfgData.auditioningPortPreset[15]);
		DataPreset->setAttribute("auditioningPortPreset17", settingsGeneralCnfgData.auditioningPortPreset[16]);
		DataInfo->addChildElement(DataPreset);

		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetGeneralCnfgDataSetting(SettingsGeneralCnfgData* settingsGeneralCnfgData)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path Settingfilepath = { settings_path / (SETTINGCNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;

		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("GeneralCnfg"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsGeneralCnfgData->DeviceCnfgUSBParam = (int8_t)e->getIntAttribute("DeviceCnfgUSBParam");
				settingsGeneralCnfgData->DeviceCnfgSafeStopParam = (int8_t)e->getIntAttribute("DeviceCnfgSafeStopParam");
				settingsGeneralCnfgData->DeviceCnfgBrightnessParam = (int8_t)e->getIntAttribute("DeviceCnfgBrightnessParam");
				settingsGeneralCnfgData->DeviceCnfgAutoSaveAll = (int8_t)e->getIntAttribute("DeviceCnfgAutoSaveAll");
				settingsGeneralCnfgData->InstrumentAutoSave = (int8_t)e->getIntAttribute("InstrumentAutoSave");
				settingsGeneralCnfgData->auditioningMaxPort = (int8_t)e->getIntAttribute("auditioningMaxPort");
				settingsGeneralCnfgData->WifiState = (bool)e->getIntAttribute("WifiState");
				settingsGeneralCnfgData->LastWifiConSsid = e->getStringAttribute("LastConnectedWiFiSsid").toStdString();

				for (auto* f : e->getChildWithTagNameIterator("auditioningPortPresetInfo"))
				{
					settingsGeneralCnfgData->auditioningPortPreset[0] = f->getIntAttribute("auditioningPortPreset1");
					settingsGeneralCnfgData->auditioningPortPreset[1] = f->getIntAttribute("auditioningPortPreset2");
					settingsGeneralCnfgData->auditioningPortPreset[2] = f->getIntAttribute("auditioningPortPreset3");
					settingsGeneralCnfgData->auditioningPortPreset[3] = f->getIntAttribute("auditioningPortPreset4");
					settingsGeneralCnfgData->auditioningPortPreset[4] = f->getIntAttribute("auditioningPortPreset5");
					settingsGeneralCnfgData->auditioningPortPreset[5] = f->getIntAttribute("auditioningPortPreset6");
					settingsGeneralCnfgData->auditioningPortPreset[6] = f->getIntAttribute("auditioningPortPreset7");
					settingsGeneralCnfgData->auditioningPortPreset[7] = f->getIntAttribute("auditioningPortPreset8");
					settingsGeneralCnfgData->auditioningPortPreset[8] = f->getIntAttribute("auditioningPortPreset9");
					settingsGeneralCnfgData->auditioningPortPreset[9] = f->getIntAttribute("auditioningPortPreset10");
					settingsGeneralCnfgData->auditioningPortPreset[10] = f->getIntAttribute("auditioningPortPreset11");
					settingsGeneralCnfgData->auditioningPortPreset[11] = f->getIntAttribute("auditioningPortPreset12");
					settingsGeneralCnfgData->auditioningPortPreset[12] = f->getIntAttribute("auditioningPortPreset13");
					settingsGeneralCnfgData->auditioningPortPreset[13] = f->getIntAttribute("auditioningPortPreset14");
					settingsGeneralCnfgData->auditioningPortPreset[14] = f->getIntAttribute("auditioningPortPreset15");
					settingsGeneralCnfgData->auditioningPortPreset[15] = f->getIntAttribute("auditioningPortPreset16");
					settingsGeneralCnfgData->auditioningPortPreset[16] = f->getIntAttribute("auditioningPortPreset17");
				}
			}
			return true;
		}
		catch (...) {
			SettingsGeneralCnfgData _settingsGeneralCnfgData;
			settingsGeneralCnfgData->DeviceCnfgUSBParam = 0;
			settingsGeneralCnfgData->DeviceCnfgSafeStopParam = 0;
			settingsGeneralCnfgData->DeviceCnfgBrightnessParam = 80;
			UpdateSettingsGeneralCnfgData(_settingsGeneralCnfgData);
		}
	}
	return false;
}
FileHandlerError EngineFileHandler::UpdateSettingsSongCnfgData(SettingsSongCnfgData SettingsSongCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_SONG_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("SongCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("SongAutoSaveParam", SettingsSongCnfgData.SongAutoSaveParam);
		DataInfo->setAttribute("SongNoteNameDispParam", SettingsSongCnfgData.SongNoteNameDispParam);
		DataInfo->setAttribute("SongTapTempoParam", SettingsSongCnfgData.SongTapTempoParam);
		DataInfo->setAttribute("DataLocation", SettingsSongCnfgData.DataLocation);
		DataInfo->setAttribute("SongInsertPosition", SettingsSongCnfgData.SongInsertPosition);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsSongCnfgDataSetting(SettingsSongCnfgData* _SettingsSongCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_SONG_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());

		if (!infofile.exists())
			return false;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("SongCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				_SettingsSongCnfgData->SongAutoSaveParam = (int8_t)e->getIntAttribute("SongAutoSaveParam");
				_SettingsSongCnfgData->SongNoteNameDispParam = (int8_t)e->getIntAttribute("SongNoteNameDispParam");
				_SettingsSongCnfgData->SongTapTempoParam = (int8_t)e->getIntAttribute("SongTapTempoParam");
				_SettingsSongCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				_SettingsSongCnfgData->SongInsertPosition = (int8_t)e->getIntAttribute("SongInsertPosition");
			}
		}
		catch (...) {
			SettingsSongCnfgData _data;
			_SettingsSongCnfgData->SongAutoSaveParam = _data.SongAutoSaveParam;
			_SettingsSongCnfgData->SongNoteNameDispParam = _data.SongNoteNameDispParam;
			_SettingsSongCnfgData->SongTapTempoParam = _data.SongTapTempoParam;
			_SettingsSongCnfgData->DataLocation = _data.DataLocation;
			_SettingsSongCnfgData->SongInsertPosition = _data.SongInsertPosition;
			UpdateSettingsSongCnfgData(_data, SongName);
		}
		return true;
	}
	return false;
}
FileHandlerError EngineFileHandler::UpdateSettingsSceneCnfgData(SettingsSceneCnfgData settingsSceneCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_SCENE_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("SceneCnfData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("SceneAutoSaveParam", settingsSceneCnfgData.SceneAutoSaveParam);
		DataInfo->setAttribute("SceneTapTempoParam", settingsSceneCnfgData.SceneTapTempoParam);
		DataInfo->setAttribute("SceneSoloModeParam", settingsSceneCnfgData.SceneSoloModeParam);
		DataInfo->setAttribute("SceneFillModeParam", settingsSceneCnfgData.SceneFillModeParam);
		DataInfo->setAttribute("SceneFillActiveCnfgParam", settingsSceneCnfgData.SceneFillActiveCnfgParam);
		DataInfo->setAttribute("SceneFillKeyCnfgParam", settingsSceneCnfgData.SceneFillKeyCnfgParam);
		DataInfo->setAttribute("DataLocation", settingsSceneCnfgData.DataLocation);
		DataInfo->setAttribute("udsCount", settingsSceneCnfgData.udsCount);
		DataInfo->setAttribute("activeUds", settingsSceneCnfgData.activeUds);
		DataInfo->setAttribute("SceneInsertPosition", settingsSceneCnfgData.SceneInsertPosition);

		for (auto iCount = 0; iCount < settingsSceneCnfgData.udsCount; iCount++)
		{
			XmlElement* DataMidi = new XmlElement("DataUdsCnfg");
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[0]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[1]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[2]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[3]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[4]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[5]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[6]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[7]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[8]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[9]));
			DataMidi->setAttribute("udsConfig", (settingsSceneCnfgData.udsConfig[iCount].udsNoteList[10]));
			DataMidi->setAttribute("udsName", settingsSceneCnfgData.udsConfig[iCount].udsName);
			DataInfo->addChildElement(DataMidi);
		}
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsSceneCnfgDataSetting(SettingsSceneCnfgData* settingsSceneCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_SCENE_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("SceneCnfData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsSceneCnfgData->SceneAutoSaveParam = (int8_t)e->getIntAttribute("SceneAutoSaveParam");
				settingsSceneCnfgData->SceneTapTempoParam = (int8_t)e->getIntAttribute("SceneTapTempoParam");
				settingsSceneCnfgData->SceneSoloModeParam = (int8_t)e->getIntAttribute("SceneSoloModeParam");
				settingsSceneCnfgData->SceneFillModeParam = (int8_t)e->getIntAttribute("SceneFillModeParam");
				settingsSceneCnfgData->SceneFillActiveCnfgParam = (int8_t)e->getIntAttribute("SceneFillActiveCnfgParam");
				settingsSceneCnfgData->SceneFillKeyCnfgParam = (int8_t)e->getIntAttribute("SceneFillKeyCnfgParam");
				settingsSceneCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				settingsSceneCnfgData->udsCount = (int8_t)e->getIntAttribute("udsCount");
				settingsSceneCnfgData->activeUds = (int8_t)e->getIntAttribute("activeUds");
				settingsSceneCnfgData->SceneInsertPosition = (int8_t)e->getIntAttribute("SceneInsertPosition");

				auto iCount = (int8_t)0;
				for (auto* f : e->getChildWithTagNameIterator("DataUdsCnfg"))
				{
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[0] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[1] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[2] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[3] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[4] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[5] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[6] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[7] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[8] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[9] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsNoteList[10] = (int8_t)f->getIntAttribute("SongAutoSaveParam");
					settingsSceneCnfgData->udsConfig[iCount].udsName = f->getStringAttribute("udsName").toStdString();
					iCount++;
				}
			}
		}
		catch (...) {
			SettingsSceneCnfgData _SettingsSceneCnfgData;
			UpdateSettingsSceneCnfgData(_SettingsSceneCnfgData, SongName);
		}
		return true;
	}
	return false;
}
FileHandlerError EngineFileHandler::UpdateSettingsTrackCnfgData(SettingsTrackCnfgData settingsTrackCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_TRACK_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("TrackCnfData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("TrackAutoSaveParam", settingsTrackCnfgData.TrackAutoSaveParam);
		DataInfo->setAttribute("DataLocation", settingsTrackCnfgData.DataLocation);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsTrackCnfgDataSetting(SettingsTrackCnfgData* settingsTrackCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_TRACK_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("TrackCnfData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsTrackCnfgData->TrackAutoSaveParam = (int8_t)e->getIntAttribute("TrackAutoSaveParam");
				settingsTrackCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
			}
		}
		catch (...) {
			SettingsTrackCnfgData _settingsTrackCnfgData;
			UpdateSettingsTrackCnfgData(_settingsTrackCnfgData, SongName);
		}
		return true;
	}
	return false;
}
FileHandlerError EngineFileHandler::UpdateSettingsTrackInstrumentMixerData(SettingsTrackInstrumentMixerData settingsTrackInstrumentMixerData, std::string SongName)
{
	// TODO - Not used keeping as place holder ; as Mixer data part of instrument
	//if (exists (settings_path))
	//{		fs::path filepath;
	//if (!SongName.empty ())
	//	filepath = { songs_path / SongName };
	//else
	//	filepath = { settings_path };
	//	std::ofstream infofile (filepath / (SETTING_TRACK_MIXER_CNFG_FILE_NAME + FILE_FORMAT), std::ofstream::binary);
	//	if (!infofile)
	//		return FILE_ERROR;
	//	infofile << std::to_string ((int8_t)settingsTrackInstrumentMixerData.MixerTrack) << '\n'
	// << std::to_string ((int8_t)settingsTrackInstrumentMixerData.MixerPanValue) << '\n'
	// << std::to_string ((int8_t)settingsTrackInstrumentMixerData.MixerVolumeValue) << '\n';
	//	infofile.close ();
	//	return FILE_OK;
	//}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsTrackInstrumentMixerDataSetting(SettingsTrackInstrumentMixerData* settingsTrackInstrumentMixerData, std::string SongName)
{
	// TODO - Not used keeping as place holder ; as Mixer data part of instrument
	//std::string buf = { 0 };
	//if (exists (settings_path))
	//{		fs::path filepath;
	//if (!SongName.empty ())
	//	filepath = { songs_path / SongName };
	//else
	//	filepath = { settings_path };
	//	std::ifstream infofile (filepath / (SETTING_TRACK_MIXER_CNFG_FILE_NAME + FILE_FORMAT), std::ifstream::binary);
	//	if (!infofile)
	//		return false;

	//	infofile >> buf; settingsTrackInstrumentMixerData->MixerTrack = std::stoi (buf);
	//	infofile >> buf; settingsTrackInstrumentMixerData->MixerPanValue = std::stoi (buf);
	//	infofile >> buf; settingsTrackInstrumentMixerData->MixerVolumeValue = std::stoi (buf);
	//	return true;
	//}
	return false;
}

FileHandlerError EngineFileHandler::UpdateSettingsPatternCnfgData(SettingsPatternCnfgData settingsPatternCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;

		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_PATTERN_CNFG_FILE_NAME + FILE_FORMAT);
		XmlElement DataXml("PatternCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");

		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("PatternAutoSaveParam", settingsPatternCnfgData.PatternAutoSaveParam);
		DataInfo->setAttribute("PatternBarFollowState", settingsPatternCnfgData.PatternBarFollowState);
		DataInfo->setAttribute("DataLocation", settingsPatternCnfgData.DataLocation);
		DataInfo->setAttribute("patternDefaultNote", settingsPatternCnfgData.patternDefaultNote);
		DataInfo->setAttribute("patternInsertPosition", settingsPatternCnfgData.patternInsertPosition);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);

		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}

bool EngineFileHandler::GetSettingsPatternCnfgDataSetting(SettingsPatternCnfgData* settingsPatternCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };

		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };

			if (!exists(filepath))
				filepath = { settings_path };
		}

		fs::path Settingfilepath = { filepath / (SETTING_PATTERN_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());

		if (!infofile.exists())
			return FILE_ERROR;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("PatternCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsPatternCnfgData->PatternAutoSaveParam = (int8_t)e->getIntAttribute("PatternAutoSaveParam");
				settingsPatternCnfgData->PatternBarFollowState = e->getIntAttribute("PatternBarFollowState");
				settingsPatternCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				settingsPatternCnfgData->patternDefaultNote = (int16_t)e->getIntAttribute("patternDefaultNote");
				settingsPatternCnfgData->patternInsertPosition = (int8_t)e->getIntAttribute("patternInsertPosition");
			}
		}
		catch (...) {
			SettingsPatternCnfgData _settingsPatternCnfgData;
			UpdateSettingsPatternCnfgData(_settingsPatternCnfgData, SongName);
			//caller has default data
		}
		return true;
	}
	return false;
}

FileHandlerError EngineFileHandler::UpdateSettingsRecordCnfgData(SettingsRecordCnfgData settingsRecordCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_RECORD_CNFG_FILE_NAME + FILE_FORMAT);
		XmlElement DataXml("RecordCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);

		DataInfo->setAttribute("RecordQuantizationParam", settingsRecordCnfgData.RecordQuantizationParam);
		DataInfo->setAttribute("RecordMthryParam", settingsRecordCnfgData.RecordMthryParam);
		DataInfo->setAttribute("RecordFTSParam", settingsRecordCnfgData.RecordFTSParam);
		DataInfo->setAttribute("RecordAutoAddBarParam", settingsRecordCnfgData.RecordAutoAddBarParam);
		DataInfo->setAttribute("RecordBarOverWriteParam", settingsRecordCnfgData.RecordBarOverWriteParam);
		DataInfo->setAttribute("RecordNumOfBarsParam", settingsRecordCnfgData.RecordNumOfBarsParam);
		DataInfo->setAttribute("RecordNoteEventParam", settingsRecordCnfgData.RecordNoteEventParam);
		DataInfo->setAttribute("RecordMidiCCEventParam", settingsRecordCnfgData.RecordMidiCCEventParam);
		DataInfo->setAttribute("RecordAfterTouchEventParam", settingsRecordCnfgData.RecordAfterTouchEventParam);
		DataInfo->setAttribute("RecordPitchBendEventParam", settingsRecordCnfgData.RecordPitchBendEventParam);
		DataInfo->setAttribute("RecordPolyPEventParam", settingsRecordCnfgData.RecordPolyPEventParam);
		DataInfo->setAttribute("RecordPgmChangeEventParam", settingsRecordCnfgData.RecordPgmChangeEventParam);
		DataInfo->setAttribute("DataLocation", settingsRecordCnfgData.DataLocation);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsRecordCnfgDataSetting(SettingsRecordCnfgData* settingsRecordCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_RECORD_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("RecordCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsRecordCnfgData->RecordQuantizationParam = (int8_t)e->getIntAttribute("RecordQuantizationParam");
				settingsRecordCnfgData->RecordMthryParam = (int8_t)e->getIntAttribute("RecordMthryParam");
				settingsRecordCnfgData->RecordFTSParam = (int8_t)e->getIntAttribute("RecordFTSParam");
				settingsRecordCnfgData->RecordAutoAddBarParam = (int8_t)e->getIntAttribute("RecordAutoAddBarParam");
				settingsRecordCnfgData->RecordBarOverWriteParam = (int8_t)e->getIntAttribute("RecordBarOverWriteParam");
				settingsRecordCnfgData->RecordNumOfBarsParam = (int8_t)e->getIntAttribute("RecordNumOfBarsParam");
				settingsRecordCnfgData->RecordNoteEventParam = (int8_t)e->getIntAttribute("RecordNoteEventParam");
				settingsRecordCnfgData->RecordMidiCCEventParam = (int8_t)e->getIntAttribute("RecordMidiCCEventParam");
				settingsRecordCnfgData->RecordAfterTouchEventParam = (int8_t)e->getIntAttribute("RecordAfterTouchEventParam");
				settingsRecordCnfgData->RecordPitchBendEventParam = (int8_t)e->getIntAttribute("RecordPitchBendEventParam");
				settingsRecordCnfgData->RecordPolyPEventParam = (int8_t)e->getIntAttribute("RecordPolyPEventParam");
				settingsRecordCnfgData->RecordPgmChangeEventParam = (int8_t)e->getIntAttribute("RecordPgmChangeEventParam");
				settingsRecordCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
			}
			return true;
		}
		catch (...) {
			SettingsRecordCnfgData _settingsRecordCnfgData;
			UpdateSettingsRecordCnfgData(_settingsRecordCnfgData, SongName);
			//caller has default data
		}
	}
	return false;
}
FileHandlerError EngineFileHandler::UpdateSettingsBRayCnfgData(SettingsBRayCnfgData settingsBRayCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_BRAY_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("BRayCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("brayPowerStatus", settingsBRayCnfgData.brayPowerStatus);
		DataInfo->setAttribute("brayControlledParam", (int8_t)settingsBRayCnfgData.brayControlledParam);
		DataInfo->setAttribute("DataLocation", settingsBRayCnfgData.DataLocation);
		DataInfo->setAttribute("brayCcNum", settingsBRayCnfgData.brayCcNum);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsBRayCnfgDataSetting(SettingsBRayCnfgData* settingsBRayCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_BRAY_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());

		if (!infofile.exists())
			return false;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("BRayCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsBRayCnfgData->brayPowerStatus = (int8_t)e->getIntAttribute("brayPowerStatus");
				settingsBRayCnfgData->brayControlledParam = (BRayControllSetting)e->getIntAttribute("brayControlledParam");
				settingsBRayCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				settingsBRayCnfgData->brayCcNum = (int8_t)e->getIntAttribute("brayCcNum");
			}
		}
		catch (...) {
			SettingsBRayCnfgData _settingsBRayCnfgData;
			UpdateSettingsBRayCnfgData(_settingsBRayCnfgData, SongName);
			//caller has default data
		}
		return true;
	}
	return false;
}

FileHandlerError EngineFileHandler::UpdateSettingsRemoteControlCnfgData(SettingsRemoteControlCnfgData settingsRemoteControlCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_REMOTE_SETTING_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("RemoteControlCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");

		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("RemoteControlStatus", settingsRemoteControlCnfgData.RemoteControlStatus);
		DataInfo->setAttribute("RemoteControlLatch", settingsRemoteControlCnfgData.RemoteControlLatch);
		DataInfo->setAttribute("RemoteControlMidiPort", settingsRemoteControlCnfgData.RemoteControlMidiPort);
		DataInfo->setAttribute("RemoteControlMidiChannel", settingsRemoteControlCnfgData.RemoteControlMidiChannel);
		DataInfo->setAttribute("RemoteControlSceneChange", settingsRemoteControlCnfgData.RemoteControlSceneChange);
		DataInfo->setAttribute("RemoteControlFontChange", settingsRemoteControlCnfgData.RemoteControlFontChange);
		DataInfo->setAttribute("DataLocation", settingsRemoteControlCnfgData.DataLocation);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsRemoteControlCnfgDataSetting(SettingsRemoteControlCnfgData* settingsRemoteControlCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_REMOTE_SETTING_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("RemoteControlCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsRemoteControlCnfgData->RemoteControlStatus = (int8_t)e->getIntAttribute("RemoteControlStatus");
				settingsRemoteControlCnfgData->RemoteControlLatch = (int8_t)e->getIntAttribute("RemoteControlLatch");
				settingsRemoteControlCnfgData->RemoteControlMidiPort = (int8_t)e->getIntAttribute("RemoteControlMidiPort");
				settingsRemoteControlCnfgData->RemoteControlMidiChannel = (int8_t)e->getIntAttribute("RemoteControlMidiChannel");
				settingsRemoteControlCnfgData->RemoteControlSceneChange = (int8_t)e->getIntAttribute("RemoteControlSceneChange");
				settingsRemoteControlCnfgData->RemoteControlFontChange = (int8_t)e->getIntAttribute("RemoteControlFontChange");
				settingsRemoteControlCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
			}
		}
		catch (...) {
			SettingsRemoteControlCnfgData _settingsRemoteControlCnfgData;
			UpdateSettingsRemoteControlCnfgData(_settingsRemoteControlCnfgData, SongName);
		}
		return true;
	}
	return false;
}

FileHandlerError EngineFileHandler::UpdateSettingsMidiMachineControlCnfgData(SettingsMidiMachineControlCnfgData settingsMidiMachineControlCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_MIDI_MACHINE_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("MidiMachineControlCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("DataLocation", settingsMidiMachineControlCnfgData.DataLocation);
		DataInfo->setAttribute("mmcDeviceId", settingsMidiMachineControlCnfgData.mmcDeviceId);
		DataInfo->setAttribute("mmcDeviceIdAll", settingsMidiMachineControlCnfgData.mmcDeviceIdAll);
		DataInfo->setAttribute("mmcInputPort", settingsMidiMachineControlCnfgData.mmcInputPort);
		DataInfo->setAttribute("mmcOutputPort", settingsMidiMachineControlCnfgData.mmcOutputPort);
		DataInfo->setAttribute("mmcReceiveStatus", settingsMidiMachineControlCnfgData.mmcReceiveStatus);
		DataInfo->setAttribute("mmcSendStatus", settingsMidiMachineControlCnfgData.mmcSendStatus);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsMidiMachineControlCnfgDataSetting(SettingsMidiMachineControlCnfgData* settingsMidiMachineControlCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_MIDI_MACHINE_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("MidiMachineControlCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsMidiMachineControlCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				settingsMidiMachineControlCnfgData->mmcDeviceId = (int8_t)e->getIntAttribute("mmcDeviceId");
				settingsMidiMachineControlCnfgData->mmcDeviceIdAll = (int8_t)e->getIntAttribute("mmcDeviceIdAll");
				settingsMidiMachineControlCnfgData->mmcInputPort = (int8_t)e->getIntAttribute("mmcInputPort");
				settingsMidiMachineControlCnfgData->mmcOutputPort = (int8_t)e->getIntAttribute("mmcOutputPort");
				settingsMidiMachineControlCnfgData->mmcReceiveStatus = (int8_t)e->getIntAttribute("mmcReceiveStatus");
				settingsMidiMachineControlCnfgData->mmcSendStatus = (int8_t)e->getIntAttribute("mmcSendStatus");
			}
		}
		catch (...) {
			SettingsMidiMachineControlCnfgData _settingsMidiMachineControlCnfgData;
			UpdateSettingsMidiMachineControlCnfgData(_settingsMidiMachineControlCnfgData, SongName);
		}
		return true;
	}
	return false;
}

FileHandlerError EngineFileHandler::UpdateSettingsSculptModeCnfgData(SettingsSculptModeCnfgData settingsSculptModeCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };
		const auto savePath=filepath / (SETTING_REMOTE_SETTING_SCLUPT_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("SculptModeCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("sculptMode", (int8_t)settingsSculptModeCnfgData.sculptMode);
		DataInfo->setAttribute("sculptParameter", (int8_t)settingsSculptModeCnfgData.sculptParameter);
		DataInfo->setAttribute("sculptRelativeVelocity", settingsSculptModeCnfgData.sculptRelativeVelocity);
		//DataInfo->setAttribute("sculptRelativeLengthSteps", settingsSculptModeCnfgData.sculptRelativeLengthSteps);
		//DataInfo->setAttribute("sculptRelativeLengthTicks", settingsSculptModeCnfgData.sculptRelativeLengthTicks);
		DataInfo->setAttribute("sculptRelativeNote", settingsSculptModeCnfgData.sculptRelativeNote);
		DataInfo->setAttribute("sculptAbsoluteVelocity", settingsSculptModeCnfgData.sculptAbsoluteVelocity);
		//DataInfo->setAttribute("sculptAbsoluteLengthSteps", settingsSculptModeCnfgData.sculptAbsoluteLengthSteps);
		//DataInfo->setAttribute("sculptAbsoluteLengthTicks", settingsSculptModeCnfgData.sculptAbsoluteLengthTicks);
		DataInfo->setAttribute("sculptAbsoluteNote", settingsSculptModeCnfgData.sculptAbsoluteNote);
		DataInfo->setAttribute("sculptRandomRelativeVelocityStart", settingsSculptModeCnfgData.sculptRandomRelativeVelocityStart);
		DataInfo->setAttribute("sculptRandomRelativeVelocityEnd", settingsSculptModeCnfgData.sculptRandomRelativeVelocityEnd);
		//DataInfo->setAttribute("sculptRandomRelativeLengthStepStart", settingsSculptModeCnfgData.sculptRandomRelativeLengthStepStart);
		//DataInfo->setAttribute("sculptRandomRelativeLengthStepEnd", settingsSculptModeCnfgData.sculptRandomRelativeLengthStepEnd);
		//DataInfo->setAttribute("sculptRandomRelativeLengthTickStart", settingsSculptModeCnfgData.sculptRandomRelativeLengthTickStart);
		//DataInfo->setAttribute("sculptRandomRelativeLengthTickEnd", settingsSculptModeCnfgData.sculptRandomRelativeLengthTickEnd);
		DataInfo->setAttribute("sculptRandomRelativeNoteStart", settingsSculptModeCnfgData.sculptRandomRelativeNoteStart);
		DataInfo->setAttribute("sculptRandomRelativeNoteEnd", settingsSculptModeCnfgData.sculptRandomRelativeNoteEnd);
		DataInfo->setAttribute("sculptRandomAbsoluteVelocityStart", settingsSculptModeCnfgData.sculptRandomAbsoluteVelocityStart);
		DataInfo->setAttribute("sculptRandomAbsoluteVelocityEnd", settingsSculptModeCnfgData.sculptRandomAbsoluteVelocityEnd);
		/*DataInfo->setAttribute("sculptRandomAbsoluteLengthStepStart", settingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepStart);
		DataInfo->setAttribute("sculptRandomAbsoluteLengthStepEnd", settingsSculptModeCnfgData.sculptRandomAbsoluteLengthStepEnd);
		DataInfo->setAttribute("sculptRandomAbsoluteLengthTickStart", settingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickStart);
		DataInfo->setAttribute("sculptRandomAbsoluteLengthTickEnd", settingsSculptModeCnfgData.sculptRandomAbsoluteLengthTickEnd);*/
		DataInfo->setAttribute("sculptRandomAbsoluteNoteStart", settingsSculptModeCnfgData.sculptRandomAbsoluteNoteStart);
		DataInfo->setAttribute("sculptRandomAbsoluteNoteEnd", settingsSculptModeCnfgData.sculptRandomAbsoluteNoteEnd);
		DataInfo->setAttribute("DataLocation", settingsSculptModeCnfgData.DataLocation);
		DataXml.addChildElement(DataInfo);
		return writeXmlAtomic(savePath,DataXml) ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsSculptModeCnfgDataSetting(SettingsSculptModeCnfgData* settingsSculptModeCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}
		fs::path Settingfilepath = { filepath / (SETTING_REMOTE_SETTING_SCLUPT_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try
		{
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("SculptModeCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}

			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsSculptModeCnfgData->sculptMode = static_cast <SculptMode>(e->getIntAttribute("sculptMode"));
				settingsSculptModeCnfgData->sculptParameter = static_cast <SculptParameterSelect>(e->getIntAttribute("sculptParameter"));
				settingsSculptModeCnfgData->sculptRelativeVelocity = (int8_t)e->getIntAttribute("sculptRelativeVelocity");
				//settingsSculptModeCnfgData->sculptRelativeLengthSteps = e->getIntAttribute("sculptRelativeLengthSteps");
				//settingsSculptModeCnfgData->sculptRelativeLengthTicks = e->getIntAttribute("sculptRelativeLengthTicks");
				settingsSculptModeCnfgData->sculptRelativeNote = (int8_t)e->getIntAttribute("sculptRelativeNote");
				settingsSculptModeCnfgData->sculptAbsoluteVelocity = (int8_t)e->getIntAttribute("sculptAbsoluteVelocity");
				//settingsSculptModeCnfgData->sculptAbsoluteLengthSteps = e->getIntAttribute("sculptAbsoluteLengthSteps");
				//settingsSculptModeCnfgData->sculptAbsoluteLengthTicks = e->getIntAttribute("sculptAbsoluteLengthTicks");
				settingsSculptModeCnfgData->sculptAbsoluteNote = (int8_t)e->getIntAttribute("sculptAbsoluteNote");
				settingsSculptModeCnfgData->sculptRandomRelativeVelocityStart = (int8_t)e->getIntAttribute("sculptRandomRelativeVelocityStart");
				settingsSculptModeCnfgData->sculptRandomRelativeVelocityEnd = (int8_t)e->getIntAttribute("sculptRandomRelativeVelocityEnd");
				//settingsSculptModeCnfgData->sculptRandomRelativeLengthStepStart = e->getIntAttribute("sculptRandomRelativeLengthStepStart");
				//settingsSculptModeCnfgData->sculptRandomRelativeLengthStepEnd = e->getIntAttribute("sculptRandomRelativeLengthStepEnd");
				//settingsSculptModeCnfgData->sculptRandomRelativeLengthTickStart = e->getIntAttribute("sculptRandomRelativeLengthTickStart");
				//settingsSculptModeCnfgData->sculptRandomRelativeLengthTickEnd = e->getIntAttribute("sculptRandomRelativeLengthTickEnd");
				settingsSculptModeCnfgData->sculptRandomRelativeNoteStart = (int8_t)e->getIntAttribute("sculptRandomRelativeNoteStart");
				settingsSculptModeCnfgData->sculptRandomRelativeNoteEnd = (int8_t)e->getIntAttribute("sculptRandomRelativeNoteEnd");
				settingsSculptModeCnfgData->sculptRandomAbsoluteVelocityStart = (int8_t)e->getIntAttribute("sculptRandomAbsoluteVelocityStart");
				settingsSculptModeCnfgData->sculptRandomAbsoluteVelocityEnd = (int8_t)e->getIntAttribute("sculptRandomAbsoluteVelocityEnd");
				//settingsSculptModeCnfgData->sculptRandomAbsoluteLengthStepStart = e->getIntAttribute("sculptRandomAbsoluteLengthStepStart");
				//settingsSculptModeCnfgData->sculptRandomAbsoluteLengthStepEnd = e->getIntAttribute("sculptRandomAbsoluteLengthStepEnd");
				//settingsSculptModeCnfgData->sculptRandomAbsoluteLengthTickStart = e->getIntAttribute("sculptRandomAbsoluteLengthTickStart");
				//settingsSculptModeCnfgData->sculptRandomAbsoluteLengthTickEnd = e->getIntAttribute("sculptRandomAbsoluteLengthTickEnd");
				settingsSculptModeCnfgData->sculptRandomAbsoluteNoteStart = (int8_t)e->getIntAttribute("sculptRandomAbsoluteNoteStart");
				settingsSculptModeCnfgData->sculptRandomAbsoluteNoteEnd = (int8_t)e->getIntAttribute("sculptRandomAbsoluteNoteEnd");
				settingsSculptModeCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
			}
			return true;
		}
		catch (...) {
			SettingsSculptModeCnfgData _settingsSculptModeCnfgData;
			UpdateSettingsSculptModeCnfgData(_settingsSculptModeCnfgData, SongName);
		}
	}
	return false;
}
FileHandlerError EngineFileHandler::UpdateSettingsUdsCnfgData(SettingsUdsCnfgData settingsUdsCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;
		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_UDS_SETTING_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("UdsCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("DataLocation", settingsUdsCnfgData.DataLocation);
		DataInfo->setAttribute("udsCount", settingsUdsCnfgData.udsCount);
		for (auto& it : settingsUdsCnfgData.udsConfig)
		{
			XmlElement* DataUdsName = new XmlElement("uds_info");
			DataUdsName->setAttribute("udsName", it.udsName);
			for (auto& itNode : it.udsNoteList)
			{
				XmlElement* NodeInfo = new XmlElement("NodeInfo");
				NodeInfo->setAttribute("itNode", itNode);
				DataUdsName->addChildElement(NodeInfo);
			}
			DataInfo->addChildElement(DataUdsName);
		}
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}
bool EngineFileHandler::GetSettingsUdsCnfgDataSetting(SettingsUdsCnfgData* settingsUdsCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	UDS udsConfig;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };
		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };
			if (!exists(filepath))
				filepath = { settings_path };
		}

		fs::path Settingfilepath = { filepath / (SETTING_UDS_SETTING_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try {
			settingsUdsCnfgData->udsConfig.clear();
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("UdsCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsUdsCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
				settingsUdsCnfgData->udsCount = (int8_t)e->getIntAttribute("udsCount");

				for (auto* f : e->getChildWithTagNameIterator("uds_info"))
				{
					udsConfig.udsName = f->getStringAttribute("udsName").toStdString();
					for (auto* g : f->getChildWithTagNameIterator("NodeInfo"))
					{
						int8_t node = (int8_t)g->getIntAttribute("itNode");
						udsConfig.udsNoteList.push_back((int8_t)node);
					}
					settingsUdsCnfgData->udsConfig.push_back(udsConfig);
					udsConfig.udsNoteList.clear();
				}
			}
		}
		catch (...) {
			SettingsUdsCnfgData _settingsUdsCnfgData;
			_settingsUdsCnfgData.udsConfig.clear();
			UpdateSettingsUdsCnfgData(_settingsUdsCnfgData, SongName);
		}
		return true;
	}
	return false;
}

FileHandlerError EngineFileHandler::updateSettingsMidiClkCnfgData(SettingsMidiClkCnfgData settingsMidiClkCnfgData, std::string SongName)
{
	if (exists(settings_path))
	{
		fs::path filepath;

		if (!SongName.empty())
			filepath = { songs_path / SongName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_MIDI_CLK_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("MidiClkCnfgData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("midiClkInPort", settingsMidiClkCnfgData.midiClkInPort);
		DataInfo->setAttribute("midiClkOutPorts", static_cast<int>(settingsMidiClkCnfgData.midiClkOutPorts));//TODO Check with Xavier for other API's, rather than casting
		DataInfo->setAttribute("MidiClkInputClkRate", settingsMidiClkCnfgData.MidiClkInputClkRate);
		DataInfo->setAttribute("MidiClkOutPutClkRate", settingsMidiClkCnfgData.MidiClkOutPutClkRate);
		DataInfo->setAttribute("SyncTriggerForIn", settingsMidiClkCnfgData.SyncTriggerForIn);
		DataInfo->setAttribute("SyncTriggerForOut", settingsMidiClkCnfgData.SyncTriggerForOut);
		DataInfo->setAttribute("DataLocation", settingsMidiClkCnfgData.DataLocation);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}

	return FILE_EXISTS;
}

bool EngineFileHandler::getSettingsMidiClkCnfgDataSetting(SettingsMidiClkCnfgData* settingsMidiClkCnfgData, std::string SongName)
{
	std::string buf = { 0 };
	int8_t version = 0;

	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };

		if (!SongName.empty())
		{
			filepath = { songs_path / SongName };

			if (!exists(filepath))
				filepath = { settings_path };
		}

		fs::path Settingfilepath = { filepath / (SETTING_MIDI_CLK_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("MidiClkCnfgData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				settingsMidiClkCnfgData->midiClkInPort = (int8_t)e->getIntAttribute("midiClkInPort");
				settingsMidiClkCnfgData->midiClkOutPorts = (uint32_t)e->getIntAttribute("midiClkOutPorts");
				settingsMidiClkCnfgData->MidiClkInputClkRate = (int8_t)e->getIntAttribute("MidiClkInputClkRate");
				settingsMidiClkCnfgData->MidiClkOutPutClkRate = (int8_t)e->getIntAttribute("MidiClkOutPutClkRate");
				settingsMidiClkCnfgData->SyncTriggerForIn = (int8_t)e->getIntAttribute("SyncTriggerForIn");
				settingsMidiClkCnfgData->SyncTriggerForOut = (int8_t)e->getIntAttribute("SyncTriggerForOut");
				settingsMidiClkCnfgData->DataLocation = (int8_t)e->getIntAttribute("DataLocation");
			}
		}
		catch (...) {
			SettingsMidiClkCnfgData _settingsMidiClkCnfgData;
			updateSettingsMidiClkCnfgData(_settingsMidiClkCnfgData, SongName);
		}
		return true;
	}
	return false;
}

FileHandlerError EngineFileHandler::updateSettingsTransposeControlCnfgData(TransposeControlCnfgData* data, std::string songName)
{
	if (exists(settings_path))
	{
		fs::path filepath;

		if (!songName.empty())
			filepath = { songs_path / songName };
		else
			filepath = { settings_path };

        const auto savePath=filepath / (SETTING_MIDI_XPOSE_CNFG_FILE_NAME + FILE_FORMAT);

		XmlElement DataXml("TrackCnfData");
		XmlElement* DataInfo = new XmlElement("settings");
		DataInfo->setAttribute("FILE_VERSION", FILE_VERSION);
		DataInfo->setAttribute("transposeControlStatus", data->transposeControlStatus);
		DataInfo->setAttribute("transposeControlLatch", data->transposeControlLatch);
		DataInfo->setAttribute("transposeControlMidiPort", data->transposeControlMidiPort);
		DataInfo->setAttribute("transposeControlMidiChannel", data->transposeControlMidiChannel);
		DataInfo->setAttribute("transposeControlCenterPitch", data->transposeControlCenterPitch);
		DataInfo->setAttribute("dataLocation", data->dataLocation);
		DataXml.addChildElement(DataInfo);
		const bool written=writeXmlAtomic(savePath,DataXml);
		return written ? FILE_OK : FILE_ERROR;
	}
	return FILE_EXISTS;
}

bool EngineFileHandler::getSettingsTransposeControlCnfgData(TransposeControlCnfgData* data, std::string songName)
{
	std::string buf = { 0 };
	int8_t version = 0;
	if (exists(settings_path))
	{
		fs::path filepath = { settings_path };

		if (!songName.empty())
		{
			filepath = { songs_path / songName };

			if (!exists(filepath))
				filepath = { settings_path };
		}

		fs::path Settingfilepath = { filepath / (SETTING_MIDI_XPOSE_CNFG_FILE_NAME + FILE_FORMAT) };
		juce::File infofile(Settingfilepath.string());
		if (!infofile.exists())
			return false;
		try {
			auto DataXml{ juce::XmlDocument::parse(infofile) };
			if (!DataXml || !DataXml->hasTagName("TrackCnfData"))
			{
				isDataCorrupted = true;
				return false;
			}
			for (auto* e : DataXml->getChildWithTagNameIterator("settings"))
			{
				version = (int8_t)e->getIntAttribute("FILE_VERSION");
				data->transposeControlStatus = e->getIntAttribute("transposeControlStatus");
				data->transposeControlLatch = e->getIntAttribute("transposeControlLatch");
				data->transposeControlMidiPort = (int8_t)e->getIntAttribute("transposeControlMidiPort");
				data->transposeControlMidiChannel = (int8_t)e->getIntAttribute("transposeControlMidiChannel");
				data->transposeControlCenterPitch = (int8_t)e->getIntAttribute("transposeControlCenterPitch");
				data->dataLocation = (int8_t)e->getIntAttribute("dataLocation");
			}
		}
		catch (...) {
			TransposeControlCnfgData _data;
			updateSettingsTransposeControlCnfgData(&_data, songName);
		}
		return true;
	}
	return false;
}
