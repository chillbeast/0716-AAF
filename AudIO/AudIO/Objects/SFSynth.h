#pragma once

#if !defined( JUCE_VERSION )
#include "../Includes/AudioObject.h"
#endif
#include "../Objects/SoundFontSynthesis/tsf.h"

class TSFLoader;
struct IStream;
struct tsf;

namespace AudIO
{
    class SFSynth : public AudIO::Object
    {
    public:
        enum
        {
            kParamBypass = 0,
            kParamGain,
            kParamNoteOn,
            kParamNoteOff,
            kParamProgramChange,
            kParamControlChange,
            kParamMax
        };
        SFSynth();

        void setSoundFont( const char* filepath );
        void setSoundFont( const uint8_t* memory, size_t size );
        void setDefaultSoundFont();

        void        prepareToPlay( uint32_t samplerate, uint32_t blocksize ) override;
        void        setParameter( int index, float value ) override;
        float       getParameter( int index ) override;
        const char* getType() const override;
        void        getParameterLabel( int index, char* text ) override;
        void        getParameterName( int index, char* text ) override;
        int         getNumParameters() override { return kParamMax; }
        float       getMin( uint32_t index ) const override;
        float       getMax( uint32_t index ) const override;
        void        channelNoteOn( uint8_t pitch, uint8_t velocity, uint8_t channel );
        void        channelNoteOff( uint8_t pitch, uint8_t channel );
        void        noteOffAll( bool quick = true );
        void        programChange( uint8_t program, uint8_t channel );
        void        controlChange( uint8_t control, uint8_t value, uint8_t channel );
        void        pitchBend( int16_t value, uint8_t channel );
        void        setGlobalGainDb( float value );
        void        setLowPassCutoff( uint8_t val, uint8_t channel );
        void        setLowPassResonance( uint8_t val, uint8_t channel );
        void        setLowPassAttack( uint8_t val, uint8_t channel );
        void        setLowPassRelease( uint8_t val, uint8_t channel );
        void        setReverb( uint8_t val, uint8_t channel );
        void        setChorus( uint8_t val, uint8_t channel );
        void        setEqLow( uint8_t val, uint8_t channel );
        void        setEqHigh( uint8_t val, uint8_t channel );

    private:
        void setupAudioSpecs( void );
        void process( AudIO::Buffer<float>& buffer ) override;

        bool                       m_bypass = false;
        float                      m_gain = 0.0;
        std::unique_ptr<TSFLoader> _loader;
        IStream* _stream = nullptr;
        tsf* _tsf = nullptr;
    };

    REGISTER_OBJECT( "SFSynth", SFSynth );
};
