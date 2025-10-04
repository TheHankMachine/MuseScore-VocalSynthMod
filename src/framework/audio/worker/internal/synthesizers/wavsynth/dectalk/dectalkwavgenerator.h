#include "../wavgenerator.h"

namespace muse::audio::synth {

struct DECTalkNote {
	std::string lyric;
	mpe::usecs_t duration;
	uint8_t pitch;
};

class DECTalkWavGenerator : public IWavGenerator {
public:
	DECTalkWavGenerator(std::string name);

	bool generate(std::string filePath, const mpe::PlaybackData& playbackData);
	bool generate(std::string filePath, const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics, bool flush);

private:

	void initCommand();
	void quantizeNotes(std::vector<DECTalkNote> notes);

	std::string m_command = "";
	std::string m_name;

};
}