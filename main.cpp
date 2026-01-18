#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>
#include <sndcore2/voice.h>
#include <sndcore2/core.h>
#include <padscore/wpad.h>
#include <sysapp/launch.h>

#include <snd/internal/RemoteSpeakerManager.h>

constexpr uint32_t AX_VOICE_MAX_PRIORITY = 31;

/**
 * Enable generating samples for Wiimotes
 * \param voice voice to generate samples from
 * \param on set to TRUE to generate samples for Wiimotes
 *
 * \see AXRmtGetSamples
 */
extern "C" void AXSetVoiceRmtOn(AXVoice *voice, uint16_t on);

extern "C" int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output);

int main() {
    WHBProcInit();

    WHBLogCafeInit();
    WHBLogUdpInit();

    AXInitParams initParams = {AX_INIT_RENDERER_48KHZ, {}, AX_INIT_PIPELINE_SINGLE};
    AXInitWithParams(&initParams);

    WPADInit();

    short *out = nullptr;
    int channels = 0;
    int sampleRate = 0;
    auto const nSamples = stb_vorbis_decode_filename("sample.ogg", &channels, &sampleRate, &out);
    if (out) {
        WHBLogPrintf("%d samples, %d Hz, %d channels ", nSamples, sampleRate, channels);
        auto * const voice = AXAcquireVoice(AX_VOICE_MAX_PRIORITY, nullptr, nullptr);

        AXVoiceOffsets bufferInfo{
            .dataType = AX_VOICE_FORMAT_LPCM16,
            .loopingEnabled = AX_VOICE_LOOP_DISABLED,
            .loopOffset = 0,
            .endOffset = static_cast<uint32_t>(nSamples),
            .currentOffset = 0,
            .data = out
        };
        AXSetVoiceOffsets(voice, &bufferInfo);

        const auto srcRatio = static_cast<float>(sampleRate) / static_cast<float>(AXGetInputSamplesPerSec());
        AXSetVoiceSrcType(voice, AX_VOICE_SRC_TYPE_LINEAR);
        AXSetVoiceSrcRatio(voice, srcRatio);

        AXVoiceVeData volume = {0x1000, 0};
        AXSetVoiceVe(voice, &volume);

        AXSetVoiceRmtOn(voice, TRUE);

        AXVoiceDeviceMixData mixData = {};
        mixData.bus[0].volume = 0x800;
        AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_CONTROLLER, 0, &mixData);

        nw::snd::internal::RemoteSpeakerManager * const speakerManager = nw::snd::internal::RemoteSpeakerManager::GetInstance();
        speakerManager->Initialize();
        nw::snd::RemoteSpeaker * const speaker = speakerManager->GetRemoteSpeaker(WPAD_CHAN_0);
        speaker->InitParam();
        speaker->Initialize(nullptr);
        speaker->EnableOutput(true);

        AXSetVoiceState(voice, AX_VOICE_STATE_PLAYING);

        // When a playing voice completes its last sample, its state is set to AX_VOICE_STOPPED
        while (WHBProcIsRunning()) {
            if (!AXIsVoiceRunning(voice))
                SYSLaunchMenu();
        }
        AXSetVoiceState(voice, AX_VOICE_STATE_STOPPED);
        AXFreeVoice(voice);

        speaker->Finalize(nullptr);
        speakerManager->Finalize();

    }
    else {
        WHBLogPrint("Failed to open and decode vorbis file");
        SYSLaunchMenu();
        while (WHBProcIsRunning()) {

        }
    }

    WPADShutdown();

    AXQuit();

    WHBLogUdpDeinit();
    WHBLogCafeDeinit();

    WHBProcShutdown();
}
