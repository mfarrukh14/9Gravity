#pragma once

//#include <SDL3/SDL_mixer.h>  // Commented out since not available
#include <string>
#include <unordered_map>
#include <memory>

class Sound {
public:
    Sound();
    ~Sound();
    
    bool LoadFromFile(const std::string& path);
    void Play(int loops = 0);
    void Stop();
    
private:
    // Mix_Chunk* m_chunk;  // Commented out
    int m_channel;
};

class Music {
public:
    Music();
    ~Music();
    
    bool LoadFromFile(const std::string& path);
    void Play(int loops = -1);
    void Stop();
    void Pause();
    void Resume();
    
    bool IsPlaying() const;
    
private:
    // Mix_Music* m_music;  // Commented out
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    bool Initialize();
    void Shutdown();
    
    std::shared_ptr<Sound> LoadSound(const std::string& name, const std::string& path);
    std::shared_ptr<Music> LoadMusic(const std::string& name, const std::string& path);
    
    void PlaySound(const std::string& name, int loops = 0);
    void PlayMusic(const std::string& name, int loops = -1);
    void StopMusic();
    
    void SetSoundVolume(int volume); // 0-128
    void SetMusicVolume(int volume); // 0-128
    
private:
    std::unordered_map<std::string, std::shared_ptr<Sound>> m_sounds;
    std::unordered_map<std::string, std::shared_ptr<Music>> m_music;
    bool m_initialized;
};
