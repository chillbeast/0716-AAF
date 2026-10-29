#pragma once
#include "Common/Modules/Instrument.h"
#include "Firmware/Application/Model/ModelParams.h"
#include <stdint.h>
#include <string>
#include <unordered_set>

class IInstrumentListener
{
public:
	virtual ~IInstrumentListener() { }

	virtual void handleInternalPgmChange(int32_t instrId, int programId)
	{
	}
	virtual void handleMidiSettingChange(int32_t instrId)
	{
	}
};

namespace Ignite
{
	class IgniteEngine;

	class InstrumentManager : public Module
	{
	public:
		InstrumentManager(IParamChangeListener* listener, IgniteEngine& model);
		InstrumentManager(const InstrumentManager&) = delete;
		InstrumentManager& operator= (const InstrumentManager&) = delete;
		InstrumentManager(InstrumentManager&&) = delete;
		InstrumentManager& operator= (InstrumentManager&&) = delete;

		~InstrumentManager() = default;

		std::string addInstrument(void); //call at create instrument page, then call set param for any parameter which are set by user
		void clearAllInstrument(void);
		void removeInstrument(std::string instrumentName);

		uint8_t getInstrumentCount(void) const;

		int8_t getInstrumentPort(std::string instrumentName);
		int8_t getInstrumentChannel(std::string instrumentName);
		Instrument* getInstrument(std::string instrumentName);

		void setInstrumentPort(std::string instrumentName, uint8_t val);
		void setInstrumentChannel(std::string instrumentName, uint8_t val);
		void setInstrumentName(std::string instrumentName, std::string Newname);
		void setInstrumentDispName(std::string instrumentName, std::string Newname);
		void setActiveInstrument(std::string name);
		void setParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist, bool isPlayMode)override;
		void onParameterChange(moduleId_t moduleSrc, ParameterBase* param, bool isPlayMode) override;
		void SetCommandParam(moduleId_t moduleSrc, paramId_t param, bool isPlayMode) override;
		void getParam(moduleId_t moduleSrc, paramId_t param)override;
		void ParamHandler(moduleId_t moduleSrc, ParameterBase* param, bool SendNotification);
		std::vector<IgniteParamList> getParamList(moduleId_t moduleSrc, std::vector<IgniteParamList>& paramlist)override;
		bool SaveInstrument(std::string name);
		void modifyInstrumentNameIfDuplicate(std::string& name);
		bool copyInstrument(std::string name);
		void moveInstrument(uint64_t positionData);
		void sortInstrumentsById(void);
		void reAssignSequentialIdsForInstrumentsAndSave(void);
		void updateInstrumentData(std::string name, InstrumentData& data);
		void SaveEditTrackMixerInstrument (int8_t trackId);
		void setTrackMixerTrack(int8_t trackId);
		void setTrackMixerPanValue(int8_t pan);
		void setTrackMixerVolumeValue(int8_t volume);
		int8_t getTrackMixerTrack (void);
		int8_t getTrackMixerPanValue (void);
		int8_t getTrackMixerVolumeValue (void);
		Instrument* getInstrumentFromListIndex (int8_t index);
		Instrument* getRxInstrumentFromTrackId(int8_t trackId);
		Instrument* getTxInstrumentFromTrackId(int8_t trackId);
		void InstrumentMngrPowerOnHousekeeping ();
		void addInstrumentListener (IInstrumentListener* listener);

		std::list<Instrument> instrumentList;
		const std::string& desktopActiveName() const {return currentInstrumentName;}
		IgniteEngine& engine;
		int8_t _tmTrackId;
	private:
		IParamChangeListener* paramlistener;
		bool isUndilistNeedUpdate = true;

		std::string currentInstrumentName;
		uint8_t numberOfInstruments;

		InstrumentData getInstrumentData(std::string InstrumentName);
		void SetDefaultUndoListData();
		void SaveData();

		std::unordered_set<IInstrumentListener *> _instrumentListeners;
	};
}

