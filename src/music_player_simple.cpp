#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <comdef.h>
#include <functiondiscoverykeys_devpkey.h>
#endif

// Simplified Music Player v0.5.0 - Direct WASAPI Implementation
// Based on analyst report recommendation: Use proven components

class SimpleMusicPlayer {
private:
    IMMDeviceEnumerator* enumerator_;
    IMMDevice* device_;
    IAudioClient* client_;
    IAudioRenderClient* render_client_;
    HANDLE audio_event_;
    std::vector<float> audio_buffer_;
    bool is_playing_;
    WAVEFORMATEXTENSIBLE wave_format_;

public:
    SimpleMusicPlayer() : enumerator_(nullptr), device_(nullptr), client_(nullptr),
                          render_client_(nullptr), audio_event_(nullptr), is_playing_(false) {
        COMInitializer();
    }

    ~SimpleMusicPlayer() {
        stop();
        cleanup();
    }

    struct COMInitializer {
        COMInitializer() { CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); }
        ~COMInitializer() { CoUninitialize(); }
    };

    bool load_wav_file(const std::string& filename) {
        std::cout << "Loading WAV file: " << filename << std::endl;

        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        // Read WAV header
        char header[44];
        file.read(header, 44);

        // Validate RIFF format
        if (std::memcmp(header, "RIFF", 4) != 0 || std::memcmp(header + 8, "WAVE", 4) != 0) {
            std::cerr << "Invalid WAV file format" << std::endl;
            return false;
        }

        // Extract audio info
        uint16_t channels = *reinterpret_cast<uint16_t*>(header + 22);
        uint32_t sample_rate = *reinterpret_cast<uint32_t*>(header + 24);
        uint16_t bits_per_sample = *reinterpret_cast<uint16_t*>(header + 34);

        std::cout << "Audio info: " << sample_rate << " Hz, " << channels
                  << " channels, " << bits_per_sample << " bits" << std::endl;

        // Find data chunk
        file.seekg(12);
        while (true) {
            char chunk_id[4];
            uint32_t chunk_size;
            file.read(chunk_id, 4);
            file.read(reinterpret_cast<char*>(&chunk_size), 4);

            if (std::memcmp(chunk_id, "data", 4) == 0) {
                break;
            }
            file.seekg(chunk_size, std::ios::cur);
        }

        // Read audio data
        size_t data_size = file.tellg();
        file.seekg(0, std::ios::end);
        data_size = static_cast<size_t>(file.tellg()) - data_size;
        file.seekg(-static_cast<ptrdiff_t>(data_size), std::ios::end);

        std::vector<int16_t> pcm_data(data_size / 2);
        file.read(reinterpret_cast<char*>(pcm_data.data()), data_size);

        // Convert to float and resample to 48kHz if needed
        double resample_ratio = 48000.0 / sample_rate;
        size_t output_frames = static_cast<size_t>(pcm_data.size() / channels * resample_ratio);

        audio_buffer_.resize(output_frames * 2); // Always stereo

        for (size_t i = 0; i < output_frames; ++i) {
            double source_index = i / resample_ratio;
            size_t source_frame = static_cast<size_t>(source_index);

            if (source_frame * channels < pcm_data.size()) {
                if (channels == 1) {
                    // Mono to stereo
                    float sample = static_cast<float>(pcm_data[source_frame]) / 32768.0f;
                    audio_buffer_[i * 2] = sample;
                    audio_buffer_[i * 2 + 1] = sample;
                } else if (channels == 2) {
                    // Stereo
                    audio_buffer_[i * 2] = static_cast<float>(pcm_data[source_frame * 2]) / 32768.0f;
                    audio_buffer_[i * 2 + 1] = static_cast<float>(pcm_data[source_frame * 2 + 1]) / 32768.0f;
                }
            } else {
                audio_buffer_[i * 2] = 0.0f;
                audio_buffer_[i * 2 + 1] = 0.0f;
            }
        }

        std::cout << "Loaded " << output_frames << " frames, resampled to 48kHz" << std::endl;
        return true;
    }

    bool initialize_audio() {
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                     CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                     reinterpret_cast<void**>(&enumerator_));
        if (FAILED(hr)) return false;

        hr = enumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &device_);
        if (FAILED(hr)) return false;

        hr = device_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void**>(&client_));
        if (FAILED(hr)) return false;

        // Set up WAVE_FORMAT_EXTENSIBLE (stage 1 fix from analyst report)
        wave_format_.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        wave_format_.Format.nChannels = 2;
        wave_format_.Format.nSamplesPerSec = 48000;
        wave_format_.Format.wBitsPerSample = 32;
        wave_format_.Format.nBlockAlign = (wave_format_.Format.nChannels * wave_format_.Format.wBitsPerSample) / 8;
        wave_format_.Format.nAvgBytesPerSec = wave_format_.Format.nSamplesPerSec * wave_format_.Format.nBlockAlign;
        wave_format_.Format.cbSize = sizeof(wave_format_) - sizeof(WAVEFORMATEX);
        wave_format_.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        wave_format_.Samples.wValidBitsPerSample = 32;
        wave_format_.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;

        // Initialize with REFERENCE_TIME buffer
        REFERENCE_TIME buffer_duration = 100000; // 10ms in 100-nanosecond units
        hr = client_->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                buffer_duration, 0, reinterpret_cast<WAVEFORMATEX*>(&wave_format_), nullptr);
        if (FAILED(hr)) {
            std::cerr << "Failed to initialize audio client: 0x" << std::hex << hr << std::endl;
            return false;
        }

        // Create event handle
        audio_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!audio_event_) return false;

        hr = client_->SetEventHandle(audio_event_);
        if (FAILED(hr)) return false;

        hr = client_->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&render_client_));
        if (FAILED(hr)) return false;

        std::cout << "Audio initialized successfully: 48kHz/32-bit float stereo" << std::endl;
        return true;
    }

    bool play() {
        if (audio_buffer_.empty()) {
            std::cerr << "No audio data to play" << std::endl;
            return false;
        }

        if (!client_) {
            if (!initialize_audio()) {
                return false;
            }
        }

        HRESULT hr = client_->Start();
        if (FAILED(hr)) return false;

        is_playing_ = true;
        std::cout << "✓ Playback started successfully" << std::endl;

        // Audio playback thread
        static size_t current_frame = 0;
        const size_t total_frames = audio_buffer_.size() / 2;

        while (is_playing_) {
            // Wait for audio event
            DWORD wait_result = WaitForSingleObject(audio_event_, 2000);
            if (wait_result != WAIT_OBJECT_0) {
                break;
            }

            UINT32 buffer_frames;
            hr = client_->GetBufferSize(&buffer_frames);
            if (FAILED(hr)) break;

            UINT32 padding;
            hr = client_->GetCurrentPadding(&padding);
            if (FAILED(hr)) break;

            UINT32 frames_available = buffer_frames - padding;
            if (frames_available == 0) continue;

            BYTE* buffer;
            hr = render_client_->GetBuffer(frames_available, &buffer);
            if (FAILED(hr)) break;

            // Fill buffer with audio data
            float* float_buffer = reinterpret_cast<float*>(buffer);
            for (UINT32 i = 0; i < frames_available; ++i) {
                if (current_frame < total_frames) {
                    float_buffer[i * 2] = audio_buffer_[current_frame * 2];
                    float_buffer[i * 2 + 1] = audio_buffer_[current_frame * 2 + 1];
                    current_frame++;
                } else {
                    // Loop the audio
                    current_frame = 0;
                    float_buffer[i * 2] = audio_buffer_[current_frame * 2];
                    float_buffer[i * 2 + 1] = audio_buffer_[current_frame * 2 + 1];
                    current_frame++;
                }
            }

            hr = render_client_->ReleaseBuffer(frames_available, 0);
            if (FAILED(hr)) break;
        }

        return true;
    }

    void stop() {
        is_playing_ = false;
        if (client_) {
            client_->Stop();
        }
    }

private:
    void cleanup() {
        if (render_client_) {
            render_client_->Release();
            render_client_ = nullptr;
        }
        if (client_) {
            client_->Release();
            client_ = nullptr;
        }
        if (device_) {
            device_->Release();
            device_ = nullptr;
        }
        if (enumerator_) {
            enumerator_->Release();
            enumerator_ = nullptr;
        }
        if (audio_event_) {
            CloseHandle(audio_event_);
            audio_event_ = nullptr;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <wav_file>" << std::endl;
        return 1;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "   Professional Music Player v0.5.0" << std::endl;
    std::cout << "   Simplified Direct WASAPI Architecture" << std::endl;
    std::cout << "========================================" << std::endl;

    SimpleMusicPlayer player;

    if (!player.load_wav_file(argv[1])) {
        std::cerr << "Failed to load audio file" << std::endl;
        return 1;
    }

    if (!player.play()) {
        std::cerr << "Failed to start playback" << std::endl;
        return 1;
    }

    std::cout << "Playing for 10 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    player.stop();
    std::cout << "Playback completed" << std::endl;

    return 0;
}