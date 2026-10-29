// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cstddef>

namespace Ignite
{
// The sequencer renders fixed 20 ms blocks; desktop drivers choose their own size.
template <std::size_t BlockSize, std::size_t SourceChannels = 1>
class FixedAudioBlockAdapter
{
public:
    void reset() { cursor = BlockSize; }

    template <typename Render>
    void process(float* const* outputs, int channels, int samples, Render render)
    {
        int offset = 0;
        while (offset < samples)
        {
            if (cursor == BlockSize)
            {
                block.fill(0.0f);
                render(block.data());
                cursor = 0;
            }
            const auto count = std::min(BlockSize - cursor, static_cast<std::size_t>(samples - offset));
            for (int channel = 0; channel < channels; ++channel)
                if (outputs[channel] != nullptr)
                {
                    if constexpr (SourceChannels == 2)
                    {
                        if (channels == 1)
                            for (std::size_t i=0;i<count;++i)
                                outputs[channel][offset+i]=(block[cursor+i]+block[BlockSize+cursor+i])*0.5f;
                        else
                            std::copy_n(block.data()+(channel%SourceChannels)*BlockSize+cursor,count,outputs[channel]+offset);
                    }
                    else std::copy_n(block.data()+cursor,count,outputs[channel]+offset);
                }
            cursor += count;
            offset += static_cast<int>(count);
        }
    }

private:
    std::array<float, BlockSize * SourceChannels> block{};
    std::size_t cursor = BlockSize;
};
}
