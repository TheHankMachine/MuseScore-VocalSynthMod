#pragma once

#include <optional>
#include <unordered_map>

#include "../../../isynthresolver.h"

#include "global/async/asyncable.h"
#include "global/modularity/ioc.h"
#include "../../../isoundfontrepository.h"

#include "fluidsynth.h"

namespace muse::audio::synth {
class WavResolver : public ISynthResolver::IResolver, public muse::Injectable, public async::Asyncable {

public:
    explicit WavResolver(const muse::modularity::ContextPtr& iocCtx = nullptr);

    ISynthesizerPtr resolveSynth(const audio::TrackId trackId, const audio::AudioInputParams& params,
        const OutputSpec& spec) const override;
    bool hasCompatibleResources(const audio::PlaybackSetupData& setup) const override;

    audio::AudioResourceMetaList resolveResources() const override;
    audio::SoundPresetList resolveSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    void refresh() override;
    void clearSources() override;

private:
    //WavSynthPtr createSynth(const audio::AudioResourceId& resourceId) const;

    /*struct SoundFontResource {
        io::path_t path;
        std::optional<midi::Program> preset = std::nullopt;
        AudioResourceMeta meta;
    };*/

    //std::unordered_map<AudioResourceId, SoundFontResource> m_resourcesCache;
};
}