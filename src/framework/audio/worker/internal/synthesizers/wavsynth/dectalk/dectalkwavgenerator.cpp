#pragma once

#include "../wavfile.h"
#include "audio/common/audiosanitizer.h"
#include "dectalkwavgenerator.h"
#include "cmudict.h"

#include <format>
#include <regex>
#include <iostream>
#include <fstream>
#include <deque>


namespace muse::audio::synth {

const mpe::usecs_t MAX_REST_DURATION = 27'500'000; // a little less than 30 seconds
const mpe::usecs_t MIN_NOTE_DOWNTIME = 20'000; // 10 ms
const mpe::usecs_t PLOSIVE_DURATION = 20'000; // 20 ms
const mpe::usecs_t MAX_SUSTAINED_DURATION = 90'000; // 90ms
const double SYLLABIC_PERCENTAGE = 0.85;

const msecs_t MIN_INPUT_DURATION = 5; // 5 ms

const int SAMPLE_RATE = 10'000; //10 kHz
const int FRAME_SIZE = 64; // 64 samples
const int MAX_PHONEMES_PER_CLAUSE = 100;
const int DUMMY_VOWEL_FRAMES = 4;
const int COMMA_PAUSE_FRAMES = 2;

// most of these are not needed, however, it might be nice to have later.
const std::set<std::string> PLOSIVE_PHONEMES		= { "P", "B", "T", "D", "K", "G" };
const std::set<std::string> NASAL_PHONEMES			= { "M", "N", "NX" };
const std::set<std::string> FRICATIVE_PHONEMES		= { "F", "V", "TH", "DH", "S", "Z", "SH", "ZH", "HX" };
const std::set<std::string> AFFRICATIVE_PHONEMES	= { "CH", "JH" };
const std::set<std::string> LIQUID_PHONEMES			= { "L", "R" };
const std::set<std::string> GLIDE_PHONEMES			= { "W", "Y" };
const std::set<std::string> SYLLABIC_PHONEMES		= {
	"IY", "IH", "EY", "EH", "AE", "AA", "AY", "AW", "AH", "AO", "OW", "OY",
	"UH", "UW", "RR", "YU", "AX", "IX", "IR", "ER", "AR", "OR", "UR"
};


DECTalkWavGenerator::DECTalkWavGenerator(std::string name) {
	m_name = name;
}

std::string seperatePhonemesWithSpace(std::string word) {
	//std::set<std::string> phonemes = {
	//	"iy", "ih", "ey", "eh", "ae", "aa", "ay", "aw", "ah", "ao", "ow", "oy", "uh", "uw", "rr", "yu",
	//	"ax", "ix", "ir", "er", "ar", "or", "ur", "yx", "ll", "hx", "rx", "lx", "nx", "el", "dz", "en",
	//	"th", "dh", "ch", "jh", "df", "sh", "zh", "dx", "tx"  "r", "w", "m", "n", "f", "v", "s", "z",
	//	"k", "g", "p", "b", "t", "d", "q", "_"
	//};

	std::set<std::string> phonemes = {
		"IY", "IH", "EY", "EH", "AE", "AA", "AY", "AW",
		"AH", "AO", "OW", "OY", "UH", "UW", "RR", "YU",
		"AX", "IX", "IT", "ER", "AR", "OR", "UR", "YX",
		"HX", "RX", "LX", "NX", "ER", "DZ", "EN", "TH",
		"DH", "CH", "JH", "DF", "SH", "ZH", "DX", "TX",
		"L", "R", "W", "M", "N", "F", "V", "S",
		"Z", "K", "G", "P", "B", "T", "D", "Q",
		"_"
	};

	std::transform(word.begin(), word.end(), word.begin(), ::toupper);
	word = std::regex_replace(word, std::regex("[^A-Z_]"), "");

	std::string result = "";
	std::string remaining = word;

	int searchLength = 2 + 1;
	while (--searchLength > 0) {
		if (phonemes.count(remaining.substr(0, searchLength))) {
			result += " " + remaining.substr(0, searchLength);

			if (remaining.size() > searchLength) {
				remaining = remaining.substr(searchLength);
			}
			else {
				break;
			}

			searchLength = 3;
		}
	}

	return result.substr(1);
}

std::deque<std::string> split(std::string input, std::string delimiter) {
	std::deque<std::string> result = {};
	std::string s = input;

	while (true) {
		size_t pos = s.find(delimiter);
		if (pos == std::string::npos) {
			result.push_back(s);
			return result;
		}
		
		result.push_back(s.substr(0, pos));
		s = s.substr(pos + delimiter.size());
	}
}


std::vector<DECTalkNote> getNotesFromScore(const mpe::PlaybackData& playbackData) {

	std::vector<DECTalkNote> notes = {};
	int lastWordStartIndex;
	bool priorHyphenated = false;
	bool phonemicMode = false;

	int s = playbackData.originEvents.size();
	for (auto const& events : playbackData.originEvents) {
		for (mpe::PlaybackEvent event : events.second) {

			DECTalkNote note;
			mpe::usecs_t nominalDuration;

			if (std::holds_alternative<mpe::RestEvent>(event)) {
				mpe::RestEvent restEvent = std::get<mpe::RestEvent>(event);

				if (notes.size() > 0 && notes[notes.size() - 1].lyric == "_") {
					if (notes[notes.size() - 1].duration + restEvent.arrangementCtx().actualDuration < MAX_REST_DURATION) {
						notes[notes.size() - 1].duration += restEvent.arrangementCtx().actualDuration;
						continue;
					}
				}

				nominalDuration = restEvent.arrangementCtx().nominalDuration;
				note = DECTalkNote{ "_", restEvent.arrangementCtx().actualDuration, 1 };
			}
			else if (std::holds_alternative<mpe::NoteEvent>(event)) {
				mpe::NoteEvent noteEvent = std::get<mpe::NoteEvent>(event);

				nominalDuration = noteEvent.arrangementCtx().nominalDuration;
				note = DECTalkNote{ "XX", noteEvent.arrangementCtx().actualDuration, (uint8_t)(noteEvent.pitchCtx().nominalPitchLevel / 50 - 20) };

				// look for a lyric for the note
				for (mpe::PlaybackEvent event : events.second) {
					if (std::holds_alternative<mpe::SyllableEvent>(event)) {
						mpe::SyllableEvent lyricEvent = std::get<mpe::SyllableEvent>(event);


						if (!phonemicMode && lyricEvent.text.toStdString().front() == '[') {
							phonemicMode = true;
						}

						if (!priorHyphenated) {
							note.lyric = lyricEvent.text.toStdString();
							lastWordStartIndex = notes.size();
						}
						else {

							if (phonemicMode) {
								notes[lastWordStartIndex].lyric += " | " + lyricEvent.text.toStdString();
							}
							else {
								notes[lastWordStartIndex].lyric += lyricEvent.text.toStdString();
							}

							note.lyric = "|";
						}

						priorHyphenated = lyricEvent.flags == mpe::SyllableEvent::FlagType::HyphenedToNext;

						if (phonemicMode && lyricEvent.text.toStdString().back() == ']') {
							phonemicMode = false;
						}
					}
				}
			}
			else {
				continue;
			}


			if (nominalDuration - note.duration < MIN_NOTE_DOWNTIME) {
				note.duration = nominalDuration;

				notes.push_back(note);
			}
			else {
				notes.push_back(note);
				notes.push_back(DECTalkNote{ "_", nominalDuration - note.duration, 1 });
			}


			
		}
	}

	return notes;
}

void spreadLyrics(std::vector<DECTalkNote>& notes) {
	std::vector<std::string> result = {};

	//fuck
	bool phonemic = false;

	for (int i = 0; i < notes.size(); ++i) {

		std::string lyric = notes[i].lyric;

		if (lyric == "_" || lyric == "XX") {
			notes[i].lyric = "_";
			continue;
		}

		if (lyric.front() == '[' || lyric.back() == ']') {
			if (lyric.front() == '[') lyric = lyric.substr(1);
			if (lyric.back() == ']')  lyric = lyric.substr(0, lyric.size() - 1);
		}
		else {
			lyric = getCMUInstance()[lyric];
		}


		std::deque<std::string> syllables = split(lyric, " | ");

		for (std::string syllable : syllables) {

			if (i > notes.size()) {
				break;
			}
			
			std::deque<std::string> phonemes = split(syllable, " ");
			std::string primaryPhoneme = phonemes.front();
			for (std::string phoneme : phonemes) {
				if (SYLLABIC_PHONEMES.count(phoneme)) {
					primaryPhoneme = phoneme;
				}
			}
			size_t pos = syllable.find(primaryPhoneme);

			int l = i;
			int r = i;
			notes[i].lyric = primaryPhoneme;

			i++;
			while (i < notes.size()) {
				if (notes[i].lyric == "XX") {
					notes[i].lyric = primaryPhoneme;
					r = i;
				}
				else if (notes[i].lyric != "_") {
					if (notes[i].lyric == "|") {
						i++;
					}

					break;
				}
				i++;
			}
			i--;

			notes[l].lyric = syllable.substr(0, pos) + notes[l].lyric;
			notes[r].lyric = notes[r].lyric + syllable.substr(pos + primaryPhoneme.size());
		
		}

	}
}

std::vector<DECTalkNote> spreadPhonemes(std::vector<DECTalkNote>& notes) {
	std::vector<DECTalkNote> result = {};
	std::string lastPhoneme = "_";

	for (int i = 0; i < notes.size(); i++) {

		if (notes[i].lyric == "_") {
			result.push_back( DECTalkNote{ notes[i].lyric, notes[i].duration, notes[i].pitch} );
			continue;
		}

		std::deque<std::string> phonemes = split(notes[i].lyric, " ");

		mpe::usecs_t duration = notes[i].duration;

		int numPlosives = 0;
		for (int j = 0; j < phonemes.size(); ++j) {
			if (PLOSIVE_PHONEMES.count(phonemes[j])) {
				numPlosives++;
			}
		}
		int num_sustain = phonemes.size() - 1 - numPlosives;

		mpe::usecs_t variable_duration = duration - numPlosives * PLOSIVE_DURATION;

		mpe::usecs_t sustained_duration = 0;
		if (num_sustain > 0) {
			sustained_duration = variable_duration * (1.0 - SYLLABIC_PERCENTAGE) / num_sustain;
			if (sustained_duration > MAX_SUSTAINED_DURATION) {
				sustained_duration = MAX_SUSTAINED_DURATION;
			}
		}

		variable_duration -= sustained_duration * num_sustain;

		mpe::usecs_t syllabic_duration = variable_duration;

		for (int j = 0; j < phonemes.size(); ++j) {

			msecs_t local_duration = 0;

			if (j == phonemes.size() - 1) {
				local_duration = duration;
			}
			else {
				if (SYLLABIC_PHONEMES.count(phonemes[j])) {
					local_duration = syllabic_duration;
				}
				else if (PLOSIVE_PHONEMES.count(phonemes[j])) {
					local_duration = PLOSIVE_DURATION;
				}
				else {
					local_duration = sustained_duration;
				}
			}

			duration -= local_duration;


			result.push_back(DECTalkNote{ phonemes[j], local_duration, notes[i].pitch });


		}
	}

	return result;

}

mpe::usecs_t framesToUsecs(uint64_t frames) {
	return 1'000'000 * (frames * FRAME_SIZE) / SAMPLE_RATE;
}

void DECTalkWavGenerator::initCommand() {
	m_command.erase();

	m_command = "[:phoneme on]";
	m_command += "[:n" + m_name.substr(0, 1) + "]";
}


void DECTalkWavGenerator::quantizeNotes(std::vector<DECTalkNote> notes) {
	m_command += "[";

	mpe::usecs_t accumulateiveError = 0;
	int nPhonemesInClause = 0;

	for(int i = 0; i < notes.size(); ++i) {

		bool addComma = false;
		int extraFrames = 0;
		DECTalkNote note = notes[i];

		bool isPlosive = PLOSIVE_PHONEMES.count(note.lyric);

		nPhonemesInClause++;
		// account for dummy vowel phoneme
		if (isPlosive) {
			nPhonemesInClause++;
		}

		if (i + 1 < notes.size() && isPlosive && notes[i + 1].lyric == "_") {
			extraFrames += DUMMY_VOWEL_FRAMES;
		}

		if (note.lyric == "_" && nPhonemesInClause > MAX_PHONEMES_PER_CLAUSE) {
			addComma = true;
			extraFrames += COMMA_PAUSE_FRAMES;
			nPhonemesInClause = 0;
		}

		msecs_t inputMs = (note.duration - accumulateiveError - framesToUsecs(extraFrames)) / 1000 - 5;

		if (inputMs < MIN_INPUT_DURATION) {
			inputMs = MIN_INPUT_DURATION;
		}

		int frames = ((inputMs + 4) * 10) / FRAME_SIZE;
		mpe::usecs_t out_us = framesToUsecs(frames + extraFrames); // 10000.0;
		//double outms = 1000.0 * (frames * 71) / 11025.0;

		accumulateiveError += (out_us - note.duration);

		m_command += note.lyric + "<" + std::to_string(inputMs);
		if (note.lyric == "_") {
			m_command += ">";
		}
		else {
			m_command += "," + std::to_string(note.pitch) + ">";
		}

		if (addComma) {
			m_command += "],[";
		}


	}

	m_command += "]";
}

bool DECTalkWavGenerator::generate(std::string filePath, const mpe::PlaybackData& playbackData) {

	if (fileExists(filePath)) {
		deleteFile(filePath);
	}

	initCommand();

	std::vector<DECTalkNote> notes = getNotesFromScore(playbackData);
	
	spreadLyrics(notes);

	notes = spreadPhonemes(notes);

	quantizeNotes(notes);

	std::ofstream myfile;
	myfile.open(filePath + ".txt");
	myfile << m_command;
	myfile.close();

	std::filesystem::path path = std::filesystem::current_path() / "dectalk\\say.exe";
	//std::string path = "dectalk\\say.exe";

	// holy fuck this fucking sucks so goddamn much
	std::string shellCommand = "cmd.exe /C \"\"" + path.string()  + "\" -w ";
	shellCommand += "\"" + filePath + "\" < \"" + filePath + ".txt\"\"";

	notes.clear();
	notes.shrink_to_fit();

	system(shellCommand.c_str());

	return true;
}

bool DECTalkWavGenerator::generate(std::string filePath, const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics, bool flush) {
	return false;
}

}