#pragma once

#include <fstream>
#include <vector>
#include <filesystem>

struct WavHeader {
    // RIFF Chunk
    uint8_t  riff[4];
    uint32_t chunkSize;
    uint8_t  wave[4];

    // fmt Chunk
    uint8_t  fmt[4];
    uint32_t fmtChunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    // data Chunk
    uint8_t data[4];
    uint32_t dataChunkSize;
};

struct WavFile {
    WavHeader header;
    std::vector<int16_t> samples;

    WavFile(WavHeader header, std::vector<int16_t> samples) : header(header), samples(samples) {}
    ~WavFile() {
        samples.clear();
        samples.shrink_to_fit();
    }
};

using WavFilePtr = std::shared_ptr<WavFile>;

WavFilePtr loadWavFile(const std::string& filename) {
    WavHeader header{};

    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return nullptr;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

    if (!(
        std::string(reinterpret_cast<char*>(header.riff), 4) == "RIFF"
        && std::string(reinterpret_cast<char*>(header.wave), 4) == "WAVE"
        && std::string(reinterpret_cast<char*>(header.fmt), 4) == "fmt "
        && std::string(reinterpret_cast<char*>(header.data), 4) == "data"
    )) {
        return nullptr;
    }

    // assert(header.blockAlign == header.numChannels * header.bitsPerSample/8);
    // assert(header.byteRate == header.sampleRate * header.blockAlign);


    WavFilePtr wav  = std::make_shared<WavFile>( header, std::vector<int16_t> {} );

    //WavFilePtr wav = std::make_shared<WavFile>( header, std::vector<int16_t> {} );

    wav->samples.resize(header.dataChunkSize / sizeof(int16_t));
    file.read(reinterpret_cast<char*>(wav->samples.data()), header.dataChunkSize);

    file.close();

    return wav;
}

bool fileExists(const std::string filePath) {
    std::ifstream infile(filePath);
    return infile.good();
}

bool deleteFile(const std::string filePath) {
    return std::filesystem::remove(filePath);
}