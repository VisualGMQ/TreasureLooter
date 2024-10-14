#pragma once
#include "pch.hpp"

namespace tl {

class Audio {
public:
    friend class AudioManager;

    Audio(const std::string& filename);
    Audio(Audio&&);
    Audio& operator=(Audio&&);
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;
    ~Audio();

    void Play(int loops);
    void Pause();
    void Resume();
    void Stop();
    bool IsPlaying();
    void FadeIn(int loops, int ms);
    void FadeOut(int loops, int ms);
    void ChangeVolume(float volume);

    operator bool() const { return chunk_; }

private:
    Mix_Chunk* chunk_ = nullptr;

    constexpr static int InvalidChannel = -2;
    int channel_ = InvalidChannel;
};

class Music {
public:
    friend class AudioManager;

    Music(const std::string& filename);
    Music(Music&&);
    Music& operator=(Music&&);
    Music(const Music&) = delete;
    Music& operator=(const Music&) = delete;
    ~Music();

    operator bool() const { return music_; }

private:
    Mix_Music* music_ = nullptr;
};

class AudioManager {
public:
    Audio* LoadSound(const std::string& filename, const std::string& name);
    Music* LoadMusic(const std::string& filename, const std::string& name);
    const Audio* FindSound(const std::string& name) const;
    Audio* FindSound(const std::string& name);
    void Destroy(const std::string& name);
    void Clear();

    void PlayMusic(const std::string& name, int loops);
    bool IsMusicPlaying() const;
    void PauseMusic();
    void ResumeMusic();
    void HaltMusic();
    void FadeInMusic(const std::string& name, int loops, int ms);
    void FadeOutMusic(int ms);
    void ChangeMusicVolume(float);

private:
    std::unordered_map<std::string, Audio> audios_;
    std::unordered_map<std::string, Music> musics_;

    Music* findMusic(const std::string&);
};

}