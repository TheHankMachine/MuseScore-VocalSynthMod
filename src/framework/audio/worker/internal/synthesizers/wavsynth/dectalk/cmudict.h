#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>
#include <filesystem>
#include <map>

class CMUDict {
public:
    static constexpr unsigned int numEntries = 143'298 + 2; // 2 extra for safety
    std::vector<std::string> entries;

    std::string operator[](std::string word) const {

        std::transform(word.begin(), word.end(), word.begin(), ::toupper);



        int l = 0;
        int r = entries.size() - 1;

        while (l <= r) {
            int m = l + ((r - l) / 2);

            std::string entryWord = entries[m].substr(0, entries[m].find("  "));

            if (entryWord < word) {
                l = m + 1;
            }
            else if (entryWord > word) {
                r = m - 1;
            }
            else {
                return entries[m].substr(entries[m].find("  ") + 2);
            }
        }

        return "_";
    }
};

std::once_flag CMULoadedFlag;

CMUDict& getCMUInstance() {
    static CMUDict instance;

    std::call_once(CMULoadedFlag, []() {
        instance.entries = {};

        std::filesystem::path path = std::filesystem::current_path() / "dectalk\\cmudict-mod";
        //std::string path = "C:\\Users\\20241427\\Source\\Repos\\MuseScore-VocalSynthMod\\src\\framework\\audio\\worker\\internal\\synthesizers\\wavsynth\\dectalk\\cmudict-mod";
        std::ifstream fin(path);

        if (!fin.is_open()) {
            std::cout << "file not found: " << path << std::endl;
        }
        //else {
            //std::cout << "yay" << std::endl;
        //}

        instance.entries.reserve(CMUDict::numEntries);

        std::string line;
        while (std::getline(fin, line)) {
            instance.entries.push_back(line);
        }
        });

    return instance;
}