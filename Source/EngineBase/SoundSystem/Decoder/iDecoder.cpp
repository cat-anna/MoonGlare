#include "iDecoder.h"

#include "libModPlugDecoder.h"
#include "WaveDecoder.h"
#include "libMpg123Decoder.h"
#include "libVorbisDecoder.h"
#include "libFlacDecoder.h"

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> iDecoderFactory::GetDecoders() {
    std::vector<DecoderInfo> r;

    for(auto &it: GetLibModPlugDecoderInfo())
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

}
