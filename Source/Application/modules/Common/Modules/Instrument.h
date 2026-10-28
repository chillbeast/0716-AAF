#pragma once
#include <stdint.h>
#include <vector>
#include <string>

namespace Ignite
{
	struct InstrumentData;
	struct TrackValue;
	struct TVMidiCCParams;
	struct TVPgmChangeParams;
	struct TVScaleVelocityByPercentParams;
	struct TVScaleLengthByPercent;
	struct TVTransposeNoteByPercent;
	struct TVTransposeNoteBySemitone;
	struct TVTransposeNoteByOctave;
	struct TVFTS;
	class InstrumentManager;
	struct TrackMixer;

	class Instrument
	{
	public:
		explicit Instrument (int32_t id, std::string name, int8_t port, int8_t chan, InstrumentManager* instMgr);
		Instrument (InstrumentData instrumentData, InstrumentManager* instMgr);
		Instrument (const Instrument& source);
		Instrument& operator= (const Instrument& source);
		~Instrument ();

		void setMidiPort (int8_t val);
		void setMidiChannel (int8_t val);
		void setName (std::string name);

		int8_t getMidiPort (void) const;
		int8_t getMidiChannel (void) const;
		std::string getName (void) const;
		std::string getDispName (void) const;
		void setDefaultNote (uint8_t val);
		void setInternalSoundPgm (int val);
		void setInternalSound (bool val);
		void setMultiTimbralStatus (bool val);
		void setPolySpreadStatus (bool val);
		void setTransposeDefeatStatus (bool val);
		void setFtsDefeatStatus (bool val);
		void setEditTrackThruStatus (bool val);
		void setCc0BankMStatus (bool val);
		void setCc0BankLStatus (bool val);
		void setDisplayNoteNum (bool val);
		void setPreSendPgmChange (bool val);

		uint8_t getDefaultNote ();
		int getInternalSoundPgm ();
		bool getInternalSound ();
		bool getMultiTimbralStatus ();
		bool getPolySpreadStatus ();
		bool getTransposeDefeatStatus ();
		bool getFtsDefeatStatus ();
		bool getEditTrackThruStatus ();
		bool getCc0BankMStatus ();
		bool getCc0BankLStatus ();
		bool getDisplayNoteNum ();
		bool getPreSendPgmChange ();

		bool UpdateUsedByList (bool isFillPattern, std::string SongName, std::string SceneName, std::string TrackName);
		bool RemoveFromUsedByList (bool isFillPattern, std::string SongName, std::string SceneName, std::string TrackName);

		void addTVKnob (int8_t knobId, TrackValue tvData);
		void deleteTVKnob (int8_t knobId);
		uint8_t getTrackValueCount (void);

		void setTVActiveKnob (int8_t knobId);
		void setTVMidiCCNum (int8_t num);
		void setTVName (std::string name);
		void setTVMidiCCValue (float val);
		void setTVMidiCCInitValue (int8_t val);
		void setTVMidiCCMinValue (int8_t val);
		void setTVMidiCCMaxValue (int8_t val);

		void setTrackValueScope (uint8_t saveMode);
		void setTVParamType (TrackValueType type);
		void setTVEffectStatus(bool status); //mute or unmute
		bool getTrackValuePresenceStatus (int8_t knobId);

		int8_t getTVActiveKnob (void);
		int8_t getTVMidiCCNum (void);
		std::string getTVName (void);
		float getTVMidiCCValue (void);
		int8_t getTVMidiCCInitValue (void);
		int8_t getTVMidiCCMinValue (void);
		int8_t getTVMidiCCMaxValue (void);

		uint8_t getTrackValueScope (void);
		int8_t getTVParamType (void);
		void setId (int32_t val);
		int32_t getId (void);


		bool getTrackValueMidiCC (std::vector <TVMidiCCParams>& ccData);
		void sendTVMidiMessage (TrackValue tvToSend, int8_t knobId);

		int8_t getTrackMixerPan (void);
		int8_t getTrackMixerVolume (void);
		void setTrackMixerVolume (int8_t value);
		void setTrackMixerPan (int8_t value);
		void sendTMMidiMessage (TrackMixer tmToSend, TrackMixerType type);


		void setTVMidiPgmNumber (int8_t val, TvLocation location);
		void setTVMidiPgmBankLsb (int8_t val, TvLocation location);
		void setTVMidiPgmBankMsb (int8_t val, TvLocation location);
		int8_t getTVMidiPgmNumber (TvLocation location);
		int8_t getTVMidiPgmBankMsb (TvLocation location);
		int8_t getTVMidiPgmBankLsb (TvLocation location);

		void saveInstrumentUsedByList (void);
		void saveInstrumentTV (void);

		bool isValidPgmChangeData (TrackValue tv);
		void loadInstrumentLevelPgmChangeData (int8_t knobId);

		InstrumentManager* _instrumentMgr;
		InstrumentData* _instrumentData;
		int8_t _activeKnobId = -1;
	};
}
