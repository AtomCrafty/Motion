#ifndef MOTION_DATASOURCE_HPP
#define MOTION_DATASOURCE_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <queue>
#include <vector>
#include <iostream>

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <Motion/Export.h>
#include <Motion/priv/AudioPacket.hpp>
#include <Motion/AudioPlayback.hpp>
#include <Motion/priv/VideoPacket.hpp>
#include <Motion/VideoPlayback.hpp>
#include <Motion/State.hpp>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

namespace mt
{
    class VideoPlayback;
    class AudioPlayback;

    class MOTION_CXX_API DataSource : private sf::NonCopyable
    {
        friend class VideoPlayback;
        friend class AudioPlayback;

    private:
        int m_videostreamid;
        int m_audiostreamid;
        sf::Clock m_updateclock;
        sf::Time m_playingoffset;
        sf::Time m_filelength;
        sf::Vector2i m_videosize;
        int m_audiochannelcount;
        float m_playbackspeed;
        AVFormatContext* m_formatcontext;
        AVCodecContext* m_videocontext;
        AVCodecContext* m_audiocontext;
        AVCodec* m_videocodec;
        AVCodec* m_audiocodec;
        AVFrame* m_videorawframe;
        AVFrame* m_videorgbaframe;
        AVFrame* m_audiorawbuffer;
        uint8_t* m_videorawbuffer;
        uint8_t* m_videorgbabuffer;
        uint8_t* m_audiopcmbuffer;
        SwsContext* m_videoswcontext;
        SwrContext* m_audioswcontext;
        State m_state;
        std::unique_ptr<std::thread> m_decodethread;
        std::atomic<bool> m_shouldthreadrun;
        std::atomic<bool> m_eofreached;
        std::atomic<bool> m_playingtoeof;
        sf::Mutex m_playbacklock;
        std::vector<mt::VideoPlayback*> m_videoplaybacks;
        std::vector<mt::AudioPlayback*> m_audioplaybacks;

        AVFrame* CreatePictureFrame(enum PixelFormat SelectedPixelFormat, int Width, int Height, unsigned char*& PictureBuffer);
        void DestroyPictureFrame(AVFrame*& PictureFrame, unsigned char*& PictureBuffer);
        void Cleanup();
        void StartDecodeThread();
        void StopDecodeThread();
        void DecodeThreadRun();
        bool IsFull();
        void NotifyStateChanged(State NewState);

    public:
        DataSource();
        ~DataSource();
        bool LoadFromFile(const std::string& Filename, bool EnableVideo = true, bool EnableAudio = true);
        void Play();
        void Pause();
        void Stop();
        const bool HasVideo();
        const bool HasAudio();
        const sf::Vector2i GetVideoSize();
        const State GetState();
        const sf::Time GetVideoFrameTime();
        const int GetAudioChannelCount();
        const int GetAudioSampleRate();
        const sf::Time GetFileLength();
        const sf::Time GetPlayingOffset();
        void SetPlayingOffset(sf::Time PlayingOffset);
        void Update();
        const float GetPlaybackSpeed();
        void SetPlaybackSpeed(float PlaybackSpeed);
        const bool IsEndofFileReached();
    };
}

#endif
