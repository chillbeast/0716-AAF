
#include "StereoOut.h"
#include "minisdl_audio.h"
#include <iostream>

float*        dest;
SDL_AudioSpec OutputAudioSpec;

void AudioCallback( void* data, uint8_t* stream, int len )
{
    if( data )
        static_cast<AudIO::StereoOut*>( data )->process( (float*)stream );
}

namespace AudIO
{
    StereoOut::StereoOut()
        : IAudioDevice( 0, 2 )
    {
        OutputAudioSpec.channels = _buffer.getNumOutputChannels();
        OutputAudioSpec.format   = AUDIO_F32;
        OutputAudioSpec.callback = AudioCallback;
        OutputAudioSpec.userdata = this;
    }

    void StereoOut::start( uint32_t samplerate, uint32_t blocksize )
    {
        OutputAudioSpec.freq    = samplerate;
        OutputAudioSpec.samples = blocksize;
        // Initialize the audio system
        if( SDL_AudioInit( NULL ) < 0 )
        {
            std::cout << "Audio Initialization Failed" << std::endl;
            return;
        }
        if( SDL_OpenAudio( &OutputAudioSpec, NULL ) < 0 )
        {
            std::cout << "Could not open the audio hardware or the desired audio output format" << std::endl;
            return;
        }

        prepareToPlay( samplerate, blocksize );
    }

    void StereoOut::pause()
    {
        _state.store( Stopped );
        SDL_PauseAudio( 1 );
    }

    void StereoOut::resume()
    {
        _state.store( Playing );
        SDL_PauseAudio( 0 );
    }

    void StereoOut::stop()
    {
        _state.store( Stopped );
        _buffer.clearOutputs();
        SDL_CloseAudio();
        teardown();
    }

    void StereoOut::process( float* data )
    {
        IAudioDevice::process(_buffer);
        // Copy to actual audio device buffer
        for( int ch = 0; ch < _buffer.getNumOutputChannels(); ++ch )
        {
            if( auto ptr = _buffer.getOutputBuffer( ch ) )
            {
                for( uint16_t sample = 0; sample < OutputAudioSpec.samples; sample++ )
                {
                    data[2 * sample + ch] = ptr[sample];
                }
            }
            else // clear Buffer
            {
                for( uint16_t sample = 0; sample < OutputAudioSpec.samples; sample++ )
                {
                    data[2 * sample + ch] = 0;
                }
            }
        }
    }
};
