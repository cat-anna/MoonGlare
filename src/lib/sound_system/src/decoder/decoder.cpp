#include "decoder.hpp"
#include <orbit_logger.h>

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> GetLibFlacDecoderInfo();
std::vector<DecoderInfo> GetLibModPlugDecoderInfo();
std::vector<DecoderInfo> GetLibMpg123DecoderInfo();
std::vector<DecoderInfo> GetLibVorbisDecoderInfo();
std::vector<DecoderInfo> GetWaveDecoderInfo();

std::vector<DecoderInfo> iDecoderFactory::GetDecoders() {
    std::vector<DecoderInfo> r;

    for (auto &it : GetLibModPlugDecoderInfo())
        r.push_back(it);
    for (auto &it : GetWaveDecoderInfo())
        r.push_back(it);
    for (auto &it : GetLibMpg123DecoderInfo())
        r.push_back(it);
    for (auto &it : GetLibVorbisDecoderInfo())
        r.push_back(it);
    for (auto &it : GetLibFlacDecoderInfo())
        r.push_back(it);

    return std::move(r);
}

} // namespace MoonGlare::SoundSystem::Decoder
