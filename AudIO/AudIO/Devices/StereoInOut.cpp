
#include "StereoInOut.h"
#include "minisdl_audio.h"
#include <iostream>

SDL_AudioSpec _specStereoInOut;

void AudioCallbackStereoInOut( void* data, uint8_t* stream, int len )
{
    if( data )
        static_cast<AudIO::StereoInOut*>( data )->process( (float*)stream );
}

namespace AudIO
{
    StereoInOut::StereoInOut()
        : IAudioDevice( 2, 2 )
    {
        _specStereoInOut.channels = _buffer.getNumOutputChannels();
        _specStereoInOut.format   = AUDIO_F32;
        _specStereoInOut.callback = AudioCallbackStereoInOut;
        _specStereoInOut.userdata = this;
    }

    void StereoInOut::start( uint32_t samplerate, uint32_t blocksize )
    {
        _specStereoInOut.freq    = samplerate;
        _specStereoInOut.samples = blocksize;
        // Initialize the audio system
        if( SDL_AudioInit( NULL ) < 0 )
        {
            std::cout << "Audio Initialization Failed" << std::endl;
            return;
        }
        if( SDL_OpenAudio( &_specStereoInOut, NULL ) < 0 )
        {
            std::cout << "Could not open the audio hardware or the desired audio output format" << std::endl;
            return;
        }

        prepareToPlay( samplerate, blocksize );
    }

    void StereoInOut::pause()
    {
        _state.store( Stopped );
        SDL_PauseAudio( 1 );
    }

    void StereoInOut::resume()
    {
        _state.store( Playing );
        SDL_PauseAudio( 0 );
    }

    void StereoInOut::stop()
    {
        _state.store( Stopped );
        _buffer.clearOutputs();
        SDL_CloseAudio();
        teardown();
    }

    void StereoInOut::process( float* data )
    {
        // Copy random input data to buffer
        for( int ch = 0; ch < _buffer.getNumInputChannels(); ++ch )
        {
            if( auto ptr = _buffer.getInputBuffer( ch ) )
            {
                for( int sample = 0; sample < _specStereoInOut.samples; sample++ )
                {
                    ptr[sample] = utils::random<float>( 0.0f, 1.f );
                }
            }
        }

        IAudioDevice::process(_buffer);

        // Copy to actual audio device buffer
        for( int ch = 0; ch < _buffer.getNumOutputChannels(); ++ch )
        {
            if( auto ptr = _buffer.getOutputBuffer( ch ) )
            {
                for( uint16_t sample = 0; sample < _specStereoInOut.samples; sample++ )
                {
                    data[2 * sample + ch] = ptr[sample];
                }
            }
            else // clear Buffer
            {
                for( uint16_t sample = 0; sample < _specStereoInOut.samples; sample++ )
                {
                    data[2 * sample + ch] = 0;
                }
            }
        }
    }
};
