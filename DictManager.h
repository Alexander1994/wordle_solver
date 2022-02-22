#pragma once

#include <filesystem>
#include <time.h>
#include <fstream>
#include <chrono>


std::vector<std::string_view> split(std::string_view str, std::string_view token) {
    std::vector<std::string_view> result;
    for (size_t i = 0; i < str.size(); ) {
        const char& c = str[i];
        int diff = c - 'a';
        if (diff < 26 && diff >= 0) {
            result.push_back(str.substr(i, 5));
            i += 5;
        }
        else {
            i++;
        }
    }
    return result;
}


class DictManager {

public:
    static constexpr size_t wordSize = 5;
    static constexpr size_t invisibleLetterCount = 2;

    void load() {
        srand(time(NULL));
        std::ifstream ifs("dict.txt");
        m_data.assign((std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()));
        m_words = split(m_data, "\r\n");
        size_t offset = rand() % m_words.size();
        m_secret = m_words[offset];
    };

    constexpr std::vector<std::string_view> words() const {
        return m_words;
    };
    constexpr std::string_view word(size_t i) const {
        return m_words[i];
    };
    constexpr void secret(std::string_view s) {
        m_secret = s;
    };
private:
    const std::string m_fileName = "dict.cpp";
    std::string m_data;
    std::vector<std::string_view> m_words;
    std::string_view m_secret;
};