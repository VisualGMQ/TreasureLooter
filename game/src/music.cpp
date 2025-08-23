#include "music.hpp"
#include "storage.hpp"
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"

Sound::Sound(const Path& filename) {
    auto file = IOStream::CreateFromFile(filename, IOMode::Read, true);
    auto content = file->Read();

    int channels, sample_rate;
    short* data;

    int samples = stb_vorbis_decode_memory((const unsigned char*)content.data(), content.size(), &channels, &sample_rate, &data);
}

Sound::~Sound() {
    delete m_data;
}
