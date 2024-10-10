#include "audio.hpp"
#include "macro.hpp"

namespace tl {

Audio::Audio(const std::string& filename) {
    chunk_ = Mix_LoadWAV(filename.c_str());
    TL_RETURN_IF_FALSE_LOGE(chunk_, "audio %s load failed: %s", filename.c_str(),
                      Mix_GetError());
}

Audio::Audio(Audio&& audio) : chunk_{audio.chunk_} {
    audio.chunk_ = nullptr;
}

Audio& Audio::operator=(Audio&& audio) {
    if (&audio != this) {
        chunk_ = audio.chunk_;
        audio.chunk_ = nullptr;
    }
    return *this;
}

void Audio::Play(int loops) {
    channel_ = Mix_PlayChannel(-1, chunk_, loops + (loops > 0 ? 1 : 0));
    if (channel_ == -1) {
        channel_ = InvalidChannel;
    }
}

void Audio::Pause() {
    TL_RETURN_IF_FALSE(channel_ != InvalidChannel);
    Mix_Pause(channel_);
}

void Audio::Resume() {
    TL_RETURN_IF_FALSE(channel_ != InvalidChannel);
    Mix_ReserveChannels(channel_);
}

void Audio::Stop() {
    TL_RETURN_IF_FALSE(channel_ != InvalidChannel);
    Mix_HaltChannel(channel_);
    channel_ = InvalidChannel;
}

bool Audio::IsPlaying() {
    TL_RETURN_FALSE_IF_FALSE(channel_ != InvalidChannel);
    return Mix_Playing(channel_);
}

void Audio::FadeIn(int loops, int ms) {
    channel_ = Mix_FadeInChannel(-1, chunk_, loops, ms);
    if (channel_ == -1) {
        channel_ = InvalidChannel;
    }
}

void Audio::FadeOut(int loops, int ms) {
    channel_ = Mix_FadeInChannel(-1, chunk_, loops, ms);
    if (channel_ == -1) {
        channel_ = InvalidChannel;
    }
}

void Audio::ChangeVolume(float volume) {
    TL_RETURN_IF_FALSE(channel_ != InvalidChannel && volume >= 0);
    Mix_Volume(channel_, MIX_MAX_VOLUME * volume);
}

Audio::~Audio() {
    Mix_FreeChunk(chunk_);
}

Music::Music(const std::string& filename) {
    music_ = Mix_LoadMUS(filename.c_str());
    TL_RETURN_IF_FALSE_LOGE(music_, "music %s load faild: %s", filename.c_str(),
                      Mix_GetError());
}

Music::Music(Music&& o) : music_{o.music_} {
    o.music_ = nullptr;
}

Music& Music::operator=(Music&& o) {
    if (&o != this) {
        music_ = o.music_;
        o.music_ = nullptr;
    }
    return *this;
}

Music::~Music() {
    Mix_FreeMusic(music_);
}

Audio* AudioManager::LoadSound(const std::string& filename,
                               const std::string& name) {
    Audio audio{filename};
    TL_RETURN_NULL_IF_FALSE(audio);

    auto result = audios_.emplace(name, std::move(audio));
    TL_RETURN_NULL_IF_FALSE(result.second);

    return &result.first->second;
}

const Audio* AudioManager::FindSound(const std::string& name) const {
    auto it = audios_.find(name);
    TL_RETURN_NULL_IF_FALSE(it != audios_.end());

    return &it->second;
}

Audio* AudioManager::FindSound(const std::string& name) {
    return const_cast<Audio*>(std::as_const(*this).FindSound(name));
}

Music* AudioManager::LoadMusic(const std::string& filename,
                               const std::string& name) {
    Music audio{filename};
    TL_RETURN_NULL_IF_FALSE(audio);

    auto result = musics_.emplace(name, std::move(audio));
    TL_RETURN_NULL_IF_FALSE(result.second);

    return &result.first->second;
}

void AudioManager::Destroy(const std::string& name) {
    audios_.erase(name);
}

void AudioManager::Clear() {
    audios_.clear();
}

void AudioManager::PlayMusic(const std::string& name, int loops) {
    Music* music = findMusic(name);
    Mix_PlayMusic(music->music_, loops + (loops > 0 ? 1 : 0));
}

bool AudioManager::IsMusicPlaying() const {
    return Mix_PlayingMusic() == 1;
}

void AudioManager::PauseMusic() {
    Mix_PauseMusic();
}

void AudioManager::ResumeMusic() {
    Mix_ResumeMusic();
}

void AudioManager::HaltMusic() {
    Mix_HaltMusic();
}

void AudioManager::FadeInMusic(const std::string& name, int loops, int ms) {
    Music* music = findMusic(name);
    Mix_FadeInMusic(music->music_, loops, ms);
}

void AudioManager::FadeOutMusic(int ms) {
    Mix_FadeOutMusic(ms);
}

void AudioManager::ChangeMusicVolume(float volume) {
    TL_RETURN_IF_FALSE(volume >= 0);
    Mix_VolumeMusic(volume * MIX_MAX_VOLUME);
}

Music* AudioManager::findMusic(const std::string& name) {
    auto it = musics_.find(name);
    TL_RETURN_NULL_IF_FALSE(it != musics_.end());
    return &it->second;
}

}  // namespace tl