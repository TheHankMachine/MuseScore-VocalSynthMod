#pragma once

#include "../abstractsynthesizer.h"

#include "audio/common/audiosanitizer.h"

namespace muse::audio::synth {
class IWavGenerator {
public:
	virtual ~IWavGenerator() = default;

	virtual bool generate(std::string filePath, const mpe::PlaybackData& playbackData) = 0;
	virtual bool generate(std::string filePath, const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics, bool flush) = 0;
};

using IWavGeneratorPtr = std::shared_ptr<IWavGenerator>;
}