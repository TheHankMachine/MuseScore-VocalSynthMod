#pragma once

#include "../abstractsynthesizer.h"
#include "wavfile.h"
#include "wavgenerator.h"

namespace muse::audio::synth {
class WavSynth : public AbstractSynthesizer {
public:
    WavSynth(const audio::AudioSourceParams& params, const modularity::ContextPtr& iocCtx);
    ~WavSynth();

    Ret init(const OutputSpec& spec);
    void addGenerator(IWavGeneratorPtr generator);
    /*Ret addSoundFonts(const std::vector<io::path_t>& sfonts);
    void setPreset(const std::optional<midi::Program>& preset);*/

    std::string name() const override;
    AudioSourceType type() const override;

    void setupSound(const mpe::PlaybackSetupData& setupData) override;
    void setupEvents(const mpe::PlaybackData& playbackData) override;
    const mpe::PlaybackData& playbackData() const override;

    void flushSound() override;

    bool isActive() const override;
    void setIsActive(const bool isActive) override;

    msecs_t playbackPosition() const override;
    void setPlaybackPosition(const msecs_t newPosition) override;

    void revokePlayingNotes() override; // all channels

    unsigned int audioChannelsCount() const override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    void setOutputSpec(const OutputSpec& spec) override;

    bool isValid() const override;

    float getSample(int32_t outspecSample) const;
    void generateWav();
    
private:

    void updateSampleRateRatio();

    IWavGeneratorPtr m_generator;

    mpe::PlaybackData m_playbackData;
    OutputSpec m_spec;

    WavFilePtr m_wav = nullptr;
    //int16_t* m_wavSamples;
    uint32_t m_sampleIndex = 0; //mixer sample rate

    double m_sampleRateRatio = 1;

    bool m_active = true;

    bool m_changed = true;

};

using WavSynthPtr = std::shared_ptr<WavSynth>;
}