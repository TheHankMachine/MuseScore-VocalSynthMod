#pragma once

#include <algorithm>
#include "../abstractsynthesizer.h"

#include "audio/common/audiosanitizer.h"

#include "wavsynth.h"

namespace muse::audio::synth {

WavSynth::WavSynth(const AudioSourceParams& params, const modularity::ContextPtr& iocCtx)
    : AbstractSynthesizer(params, iocCtx) {
}

WavSynth::~WavSynth() {
    m_wav = nullptr;
    m_generator = nullptr;
}

void WavSynth::addGenerator(IWavGeneratorPtr generator) {
    m_generator = generator;
}

std::string WavSynth::name() const {
    return "Wav";
}

AudioSourceType WavSynth::type() const {
    return AudioSourceType::WavFile;
}


void WavSynth::setupSound(const mpe::PlaybackSetupData& setupData) {
    updateSampleRateRatio();
}

void WavSynth::updateSampleRateRatio() {

    if (m_wav == nullptr) {
        return;
    }
    
    // I guess I could replace this with the unsigned long long computation if
    // double is not percise enough.
    m_sampleRateRatio = (double) m_wav->header.sampleRate / m_spec.sampleRate;

}

void WavSynth::setupEvents(const mpe::PlaybackData& playbackData) {

    ONLY_AUDIO_WORKER_THREAD;

    m_playbackData = playbackData;

    m_playbackData.mainStream.onReceive(this, [this](const mpe::PlaybackEventsMap& events,
        const mpe::DynamicLevelLayers& dynamics) {

            m_changed = true;

            m_playbackData.originEvents = events;
            m_playbackData.dynamics = dynamics;
            //m_shouldUpdateMainStreamEvents = true;

//            if (m_isActive || m_updateMainStreamWhenInactive) {
//                updateMainStream();
//            }
           

            /*m_playbackData.originEvents = events;
            m_playbackData.dynamics = dynamics;
            m_shouldUpdateMainStreamEvents = true;

            if (m_isActive || m_updateMainStreamWhenInactive) {
                updateMainStream();
            }*/
        }
    );



    //m_playbackData.offStream.onReceive(this, [this](const mpe::PlaybackEventsMap& events,
    //    const mpe::DynamicLevelLayers& dynamics,
    //    bool flush) {
    //        for (const auto& pair : events) {
    //            for (const mpe::PlaybackEvent& event : pair.second) {
    //                if (std::holds_alternative<mpe::NoteEvent>(event)) {
    //                    mpe::NoteEvent e = std::get<mpe::NoteEvent>(event)
    //                } 
    //            }
    //        }
    //    });

}

const mpe::PlaybackData& WavSynth::playbackData() const {
    return m_playbackData;
}

void WavSynth::flushSound() {
    //m_playing = false;
    // stops all sound
}

bool WavSynth::isActive() const {
    return m_active;
}

void WavSynth::setIsActive(const bool isActive) {
    m_active = isActive;

    if (isActive && m_changed) {
        generateWav();
    }
}

msecs_t WavSynth::playbackPosition() const {   
    return m_sampleIndex * 1000000 / m_spec.sampleRate;
}

void WavSynth::setPlaybackPosition(const msecs_t newPosition) {

    m_sampleIndex = newPosition * m_spec.sampleRate / 1000000;

    return;
}

void WavSynth::revokePlayingNotes() {
    //"gracefully" stops all notes
    flushSound();
    return;
}

unsigned int WavSynth::audioChannelsCount() const {
    return 2;
}

//TODO:
// [ ] - Incorperate stereo
samples_t WavSynth::process(float* buffer, samples_t samplesPerChannel) {

    if (!buffer) {
        return 0;
    }

    if (!m_active) {
        return 0;
    }

    if (m_wav == nullptr) {
        return 0;
    }


    int maxSamplesToWrite = std::min(
        (samples_t) (m_wav->samples.size() / m_sampleRateRatio) - m_sampleIndex,
        samplesPerChannel
    );
    
    if (maxSamplesToWrite <= 0) {
        return 0;
    }
        
    if (m_sampleIndex < 0) {
        return 0;
    }

    for (uint i = 0; i < maxSamplesToWrite; ++i) {
  
        //l = ((long) m_sampleIndex * m_wav->header.sampleRate) / m_spec.sampleRate;

        //float sample = m_wavSamples[(uint32_t) (m_sampleIndex * m_sampleRateRatio)] * (0.25f / 32768.f);
        float sample = 0;// getSample(m_sampleIndex);

        const int N = 10;

        for (uint j = 0; j < N; j++) {
            sample += getSample(m_sampleIndex - j) / N;
        }

        *buffer++ = sample;
        *buffer++ = sample;

        m_sampleIndex++;
    }

    return maxSamplesToWrite;
}

float WavSynth::getSample(int32_t outspecSample) const {
    if (outspecSample < 0) {
        return 0;
    }
    if (m_wav == nullptr) {
        return 0;
    }
    return m_wav->samples[(uint32_t)(outspecSample * m_sampleRateRatio)] / 32768.f;
}

void WavSynth::generateWav() {
    m_changed = false;

    const char* temp_path = std::getenv("TEMP");
    std::string path = std::string(temp_path) + "\\musescore_vocal_out.wav";

    m_generator->generate(path, m_playbackData);
    m_wav = loadWavFile(path);

    updateSampleRateRatio();
}

bool WavSynth::isValid() const {
    return true;
}

void WavSynth::setOutputSpec(const OutputSpec& spec) {
    m_spec = spec;

    updateSampleRateRatio();
}

async::Channel<unsigned int> WavSynth::audioChannelsCountChanged() const {
    async::Channel<unsigned int> idk_what_im_doing;

    return idk_what_im_doing;
}


};