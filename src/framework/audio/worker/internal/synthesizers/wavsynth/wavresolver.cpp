/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "wavresolver.h"
#include "wavsynth.h"

#include "audio/common/audiosanitizer.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::synth;

WavResolver::WavResolver(const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
    ONLY_AUDIO_WORKER_THREAD;

    refresh();
}

ISynthesizerPtr WavResolver::resolveSynth(const TrackId trackId, const AudioInputParams& params, const OutputSpec& spec) const
{
    ONLY_AUDIO_WORKER_THREAD;

//    auto search = m_resourcesCache.find(params.resourceMeta.id);
//    if (search == m_resourcesCache.end()) {
//        LOGE() << "Not found: " << params.resourceMeta.id;
//        return nullptr;
//    }

    WavSynthPtr synth = std::make_shared<WavSynth>(params, iocContext());
    synth->addGenerator(
        std::make_shared<DECTalkWavGenerator>(params.resourceMeta.id)
    );
    //synth->init(spec);
    //synth->addSoundFonts({ search->second.path });
    //synth->setPreset(search->second.preset);

    return synth;
}

bool WavResolver::hasCompatibleResources(const PlaybackSetupData& setup) const
{
    return true;
}

AudioResourceMetaList WavResolver::resolveResources() const
{
    ONLY_AUDIO_WORKER_THREAD;

    const std::vector<std::string> names = {
        "Perfect Paul",
        "Beautiful Betty",
        "Huge Harry",
        "Doctor Dennis",
        "Whispering Wendy",
        "Frail Frank",
        "Rough Rita",
        "Uppity Ursula",
        "Kit the Kid",
        "Variable Val"
    };

    AudioResourceMetaList result;
    result.reserve(names.size());

    for (std::string name : names) {
         
        AudioResourceMeta meta;
        meta.id = name;
        meta.type = AudioResourceType::WavFile;
        meta.vendor = "DECTalk";

        meta.attributes = {
            //{ PLAYBACK_SETUP_DATA_ATTRIBUTE, muse::mpe::GENERIC_SETUP_DATA_STRING },
            //{ PRESET_BANK_ATTRIBUTE, String::number(preset.program.bank) },
            //{ PRESET_NAME_ATTRIBUTE, String::fromStdString(preset.name) },
            { SOUNDFONT_NAME_ATTRIBUTE, String::fromStdString(name)},
        };
        meta.hasNativeEditorSupport = true;

        result.push_back(meta);

    }

    //AudioResourceMeta meta;
    //meta.id = "Perfect Paul";
    //meta.type = AudioResourceType::WavFile;
    //meta.vendor = "DECTalk";
    //meta.attributes = {
    //    //{ PLAYBACK_SETUP_DATA_ATTRIBUTE, muse::mpe::GENERIC_SETUP_DATA_STRING },
    //    { SOUNDFONT_NAME_ATTRIBUTE, String::fromStdString("paul")}
    //};
    //meta.hasNativeEditorSupport = true;

    //result.push_back(meta);

    /*result.reserve(m_resourcesCache.size());
    * 
    for (const auto& pair : m_resourcesCache) {
        result.push_back(pair.second.meta);
    }*/

    return result;
}

SoundPresetList WavResolver::resolveSoundPresets(const audio::AudioResourceMeta&) const
{
    return SoundPresetList();
}

//static std::string makeId(const std::string& name, int bank, int program)
//{
//    return name + "\\" + std::to_string(bank) + "\\" + std::to_string(program);
//}

void WavResolver::refresh()
{
    ONLY_AUDIO_WORKER_THREAD;

    //m_resourcesCache.clear();

    /*for (const auto& pair : soundFontRepository()->soundFonts()) {
        const SoundFontMeta& soundFont = pair.second;

        std::string name = io::completeBasename(soundFont.path).toStdString();

        {
            AudioResourceId id = name;

            AudioResourceMeta chooseAutomaticMeta;
            chooseAutomaticMeta.id = id;
            chooseAutomaticMeta.type = AudioResourceType::FluidSoundfont;
            chooseAutomaticMeta.vendor = FLUID_VENDOR_NAME;
            chooseAutomaticMeta.attributes = {
                { PLAYBACK_SETUP_DATA_ATTRIBUTE, muse::mpe::GENERIC_SETUP_DATA_STRING },
                { SOUNDFONT_NAME_ATTRIBUTE, String::fromStdString(name) }
            };
            chooseAutomaticMeta.hasNativeEditorSupport = false;

            m_resourcesCache.emplace(id, SoundFontResource{ soundFont.path, std::nullopt, std::move(chooseAutomaticMeta) });
        }

        for (const SoundFontPreset& preset : soundFont.presets) {
            AudioResourceId id = makeId(name, preset.program.bank, preset.program.program);

            AudioResourceMeta meta;
            meta.id = id;
            meta.type = AudioResourceType::FluidSoundfont;
            meta.vendor = FLUID_VENDOR_NAME;
            meta.attributes = {
                { PLAYBACK_SETUP_DATA_ATTRIBUTE, muse::mpe::GENERIC_SETUP_DATA_STRING },
                { SOUNDFONT_NAME_ATTRIBUTE, String::fromStdString(name) },
                { PRESET_NAME_ATTRIBUTE, String::fromStdString(preset.name) },
                { PRESET_BANK_ATTRIBUTE, String::number(preset.program.bank) },
                { PRESET_PROGRAM_ATTRIBUTE, String::number(preset.program.program) },
            };
            meta.hasNativeEditorSupport = false;

            m_resourcesCache.emplace(id, SoundFontResource{ soundFont.path, preset.program, std::move(meta) });
        }
    }*/
}

void WavResolver::clearSources()
{
}
