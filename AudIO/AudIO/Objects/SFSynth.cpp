
#include "SoundFontSynthesis/tsf.h"
#include "SoundFontSynthesis/AWE32_1MB.h"
#include "SFSynth.h"

namespace AudIO
{
    SFSynth::SFSynth()
        : Object( 0, 2, kParamMax ), _loader( new TSFLoader )
    {
        setDefaultSoundFont();
    }

    void SFSynth::setSoundFont( const char* filepath )
    {
        if( _stream )
            delete _stream;
        _loader->unload();
        _stream = new FileStream( filepath );
        if( _stream )
            _tsf = _loader->load( _stream );

        setupAudioSpecs();
    }

    void SFSynth::setSoundFont( const uint8_t* memory, size_t size )
    {
        if( _stream )
            delete _stream;
        _loader->unload();
        _stream = new MemoryStream( memory, size );
        if( _stream )
            _tsf = _loader->load( _stream );

        setupAudioSpecs();
    }

    void SFSynth::setDefaultSoundFont()
    {
        setSoundFont( soundfont, soundfontsize );
        //setSoundFont( "D:\\florestan-subset.SF2");
    }

    void SFSynth::setupAudioSpecs( void )
    {
        if( !_tsf )
            return;
        _tsf->setChannelBankPreset( 9, 128, 0 );
        _tsf->setMaxVoices(48); //TODO Causes a crash
    }

    void SFSynth::setParameter( int index, float value )
    {
        switch( index )
        {
            case kParamBypass:
            m_bypass = value >= 1.0 ? 1.0 : 0.0;
            break;
            default:
            throw; // unknown parameter
        }
    }

    float SFSynth::getParameter( int index )
    {
        switch( index )
        {
            case kParamBypass:
            return m_bypass ? 1.0f : 0.0f;
            default:
            break; // unknown parameter
        }
        return 0.0;
    }

    float SFSynth::getMin( uint32_t index ) const
    {
        switch( index )
        {
            case kParamBypass:
            return 0.0;
            default:
            break; // unknown parameter
        }
        return 0.0;
    }

    float SFSynth::getMax( uint32_t index ) const
    {
        switch( index )
        {
            case kParamBypass:
            return 1.0;
            default:
            break; // unknown parameter
        }
        return 0.0;
    }

    const char* SFSynth::getType() const
    {
        return "SFSynth";
    }

    void SFSynth::getParameterLabel( int index, char* text )
    {
        if( text )
            strcpy( text, utils::emptyString );
    }

    void SFSynth::getParameterName( int index, char* text )
    {
        if( text )
        {
            switch( index )
            {
                case kParamBypass:
                strcpy( text, "Bypass" );
                break;
            }
            strcpy( text, utils::emptyString );
        }
    }

    void SFSynth::prepareToPlay( uint32_t samplerate, uint32_t blocksize )
    {
        Object::prepareToPlay(samplerate, blocksize);
        if (!_tsf) return;
        _tsf->setOutputSpecs( OutputMode::StereoUnweaved, blocksize, samplerate, m_gain );


        _tsf->_chorus.init( samplerate );
        _tsf->_eq.init( samplerate, 400.f, 8000.f );
        _tsf->_reverb.init( samplerate );

        setLowPassAttack( 127, 1 );
        setLowPassRelease( 0, 1 );
        setLowPassCutoff( 70, 1 );
        setLowPassResonance( 100, 1 );

    }

    void SFSynth::process( AudIO::Buffer<float>& buffer )
    {
        if( !m_bypass && _tsf )
        {
            _tsf->renderMono( buffer.getOutputBuffer(0), blockSize() );
            if( buffer.getNumOutputChannels() > 1 )
            {
                memcpy( buffer.getOutputBuffer( 1 ), buffer.getOutputBuffer( 0 ), blockSize() * sizeof( float ) );
            }
        }
    }

    void SFSynth::channelNoteOn( uint8_t pitch, uint8_t velocity, uint8_t channel )
    {
        if( !_tsf || channel == 0 || channel > 16 )
            return;
        _tsf->channelNoteOn( channel - 1, pitch, (float)(velocity / 128.0f) );
    }

    void SFSynth::channelNoteOff( uint8_t pitch, uint8_t channel )
    {
        if( !_tsf || channel == 0 || channel > 16 )
            return;
        _tsf->channelNoteOff( channel - 1, pitch );
    }

    void SFSynth::noteOffAll( bool quick )
    {
        if( !_tsf )
            return;
        _tsf->noteOffAll( quick );
    }

    void SFSynth::programChange( uint8_t program, uint8_t channel )
    {
        if( !_tsf || channel == 0 || channel > 16 )
            return;
        _tsf->channelNoteOffAll( channel - 1 );
        _tsf->setChannelPresetNumber( channel - 1, program, (channel == 10) );
    }

    void SFSynth::controlChange( uint8_t control, uint8_t value, uint8_t channel )
    {
        if( !_tsf || channel == 0 || channel > 16 )
            return;
        _tsf->channelMidiControl( channel - 1, control, value );
    }

    void SFSynth::pitchBend( int16_t value, uint8_t channel )
    {
        if( !_tsf || channel == 0 || channel > 16 )
            return;
        _tsf->setPitchWheel( channel - 1, value );
    }

    void SFSynth::setGlobalGainDb( float value )
    {
        m_gain = value;

        if( !_tsf )
            return;

        auto volume = (m_gain > -100.f) ? powf( 10.0f, m_gain * 0.05f ) : 0;

        _tsf->setVolume( volume );
    }

    void SFSynth::setLowPassCutoff( uint8_t val, uint8_t channel )
    {
        _tsf->setLowPassCutoff( val );
    }

    void SFSynth::setLowPassResonance( uint8_t val, uint8_t channel )
    {
        _tsf->setLowPassResonance( val );
    }

    void SFSynth::setLowPassAttack( uint8_t val, uint8_t channel )
    {
        _tsf->setLowPassAttack( val );
    }

    void SFSynth::setLowPassRelease( uint8_t val, uint8_t channel )
    {
        _tsf->setLowPassRelease( val );
    }

    void SFSynth::setReverb( uint8_t val, uint8_t channel )
    {
        _tsf->setReverb( val );
    }

    void SFSynth::setChorus( uint8_t val, uint8_t channel )
    {
        _tsf->setChorus( val );
    }

    void SFSynth::setEqLow( uint8_t val, uint8_t channel )
    {
        _tsf->setEqLow( val );
    }

    void SFSynth::setEqHigh( uint8_t val, uint8_t channel )
    {
        _tsf->setEqHigh( val );
    }
}
