
#include <fstream>
#include <string>
#include <iostream>
#include <array>
#include <stdlib.h>
#include <time.h>
#include <filesystem>
#include <map>
#include <vector>
#include <optional>
#include <numeric>
#include <chrono>
#include <set>

#include "ThreadPool.h"

using result = std::array<int, 5>;
static constexpr result defaultResult = result{ 0,0,0,0,0 };

class DictManager {

public:
    static constexpr size_t wordSize = 5;
    static constexpr size_t invisibleLetterCount = 2;
    
    void load();
    result check(std::string_view guess);

    constexpr std::vector<std::string_view> words() const;
    constexpr std::string_view word(size_t i) const;
    constexpr void secret(std::string_view s);
private:
    const std::string m_fileName = "dict.cpp";
    std::string m_data;
    std::vector<std::string_view> m_words;
    std::string_view m_secret;
};

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

class Knowledge {
public:
    void load(DictManager& dm) {
        m_words = dm.words();
        m_totalWords = dm.words();
    };

    std::string_view generateGuess(result res) {
        int maxScore = 0;
        std::string_view currentGuess = "";
        for (const auto& word : m_totalWords) {
            int currScore = generateWordScore(word, res);
            if (currScore > maxScore) {
                maxScore = currScore;
                currentGuess = word;
            }
        }
        return currentGuess;
    };

    bool guessableWord(std::string_view w, const std::set<char>& charSet) {
        int count = 0;
        for (const char& c : w) {
            if (charSet.find(c) != charSet.end()) {
                count++;
            }
            if (count >= 2) return true;
        }
        return false;
    }

    void ween(std::string_view guess, result res) {
        if (m_words.size() == 1) return;
        std::set<char> charSet;
        for (auto iter = m_words.begin(); iter != m_words.end();) {
            auto word = *iter;
            bool wordKept = keepWord(word, guess, res);
            if (!wordKept || (word == guess)) {
                iter = m_words.erase(iter);
            }
            else {
                for (const char& c : word) charSet.insert(c);
                ++iter;
            }
        }
        for (auto iter = m_totalWords.begin(); iter != m_totalWords.end();) {
            auto word = *iter;
            bool wordToGuess = guessableWord(word, charSet);
            if (!wordToGuess) {
                iter = m_totalWords.erase(iter);
            }
            else {
                for (const char& c : word) charSet.insert(c);
                ++iter;
            }
        }
    };


    void updatePopularity() {
        m_popularity.clear();
        m_positionPopularity.clear();

        for (const auto& word : m_words) {
            for (int i = 0; i < 5; i++) {
                const char& c = word[i];
                m_popularity[c]++;
                m_positionPopularity[c][i]++;
            }
        }
    }

    void printWords() {
        for (const auto& word : m_words) {
            std::cout << word << '\n';
        }
    }

    size_t wordCount() {
        return m_words.size();
    }

private:
    int generateWordScore(std::string_view word, result res) {
        int score = 0;
        std::set<char> chars;
        for (int i = 0; i < 5; i++) {
            const char& c = word[i];
            int charsWeightedPop = (chars.find(c) != chars.end()) ? 1 : m_popularity[c];
            chars.insert(c);
            score += charsWeightedPop * m_positionPopularity[c][i];
        }
        return score;
    }

    bool keepWord(std::string_view word, std::string_view guess, result res) {
        std::map<size_t, char> oneIndexLocations; // indexes where res[i] = 1

        for (int i = 0; i < 5; i++) {
            char r = res[i];
            char g = guess[i];
            if (r == 0) {
                // false: if letter exists, skipping 2's and skipping letters that are 1 locations
                for (size_t j = 0; j < 5; j++) {
                    char w = word[j];
                    char rc = res[j];
                    if (rc != 2 && oneIndexLocations.find(j) == oneIndexLocations.end() && w == g) {
                        return false;
                    }
                }
            }
            else if (r == 1) {
                // false: if letter doesn't exist, skipping locations where guess[i] is the letter and using unique locations. 
                bool found = false;
                for (size_t j = 0; j < 5; j++) {
                    char w = word[j];
                    char rc = res[j];
                    char gc = guess[j];
                    if (gc == w && i == j) {
                        return false;
                    } else if (!found && gc != g && rc != 2 && w == g && oneIndexLocations.find(j) == oneIndexLocations.end()) {
                        oneIndexLocations[j] = w;
                        found = true;
                    }
                    if (i >= j && found) break; // this could be written more cleanly in the for loop
                }
                if (!found) return false;
            }
            else if (r == 2) { // remove all that dont have g at the index
                if (word[i] != g) {
                    return false;
                }
            }
        }
        return true;
    }

    std::vector<std::string_view> m_words;
    std::vector<std::string_view> m_totalWords;
    std::map<char, int> m_popularity;
    std::map<char, std::array<int, 5>> m_positionPopularity;

};

void DictManager::load() {
    srand(time(NULL));
    std::ifstream ifs("dict.txt");
    m_data.assign((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    m_words = split(m_data, "\r\n");
    size_t offset = rand() % m_words.size();
    m_secret = m_words[offset];
}

constexpr void DictManager::secret(std::string_view s) {
    m_secret = s;
}

constexpr std::string_view DictManager::word(size_t i) const {
    return m_words[i];
};

constexpr std::vector<std::string_view> DictManager::words() const {
    return m_words;
};


result checkWithSecret(std::string_view guess, std::string_view secret) {
    result res = { 0,0,0,0,0 };
    std::vector<size_t> usedIndexes;
    usedIndexes.reserve(5);

    for (size_t i = 0; i < 5; i++) {
        char g = guess[i];
        if (g == secret[i]) {
            res[i] = 2;
            usedIndexes.push_back(i);
        }
    }

    for (size_t i = 0; i < 5; i++) {
        char g = guess[i];
        if (res[i] != 2) {
            for (size_t j = 0; j < 5; j++) {
                char s = secret[j];
                bool indexIsUsed = std::find(usedIndexes.begin(), usedIndexes.end(), j) != usedIndexes.end();
                if (!indexIsUsed && g == s) {
                    res[i] = 1;
                    usedIndexes.push_back(j);
                    break;
                }
            }
        }
    }
    return res;
};

result DictManager::check(std::string_view guess) {
    return checkWithSecret(guess, m_secret);
}

void print(result res) {
    for (int i = 0; i < 4; i++) {
        std::cout << res[i] << " ";
    }
    std::cout << res[4] << '\n';
}

std::optional<result> stringToResult(std::string_view s) {
    std::cout << s << '\n';
    if (s.length() >= 5) {
        std::optional<result> res = result{0,0,0,0,0};
        for (int i = 0; i < 5; i++) {
            char c = s[i];
            if (c == '0' || c == '1' || c == '2') {
                (*res)[i] = c - '0';
            }
            else {
                return std::nullopt;
            }
        }
        return res;
    }
    return std::nullopt;
}

bool isComplete(result res) {
    for (int i = 0; i < 5; i++) {
        if (res[i] == 0 || res[i] == 1) return false;
    }
    return true;
}

int iterationCount(Knowledge knowledge, std::string_view initialGuess, std::string_view secret, std::shared_ptr<std::ofstream> outTxt) {
    result res = defaultResult;
    std::string_view guess = initialGuess;
    int i = 1;

    if (outTxt) (*outTxt) << secret;
    for (; true; i++) {
        if (outTxt) (* outTxt) << " " << guess;
        res = checkWithSecret(guess, secret);

        if (isComplete(res)) {
            break;
        }
        else {
            knowledge.ween(guess, res);
            knowledge.updatePopularity();
            guess = knowledge.generateGuess(res);
        }
        if (i == 6) {
            if (outTxt) (*outTxt) << '\n';
            return 6;
        }
    }
    if (outTxt) (*outTxt) << '\n';
    return i;
}

float average(std::vector<float> const& v) {
    if (v.empty()) {
        return 0;
    }

    auto const count = static_cast<float>(v.size());
    return std::reduce(v.begin(), v.end()) / count;
}

void baseLineAlgorithm(DictManager& dict, Knowledge& knowledge) {
    auto start = std::chrono::high_resolution_clock::now();
    const size_t wordCount = dict.words().size();
    knowledge.updatePopularity();
    auto initialGuess = knowledge.generateGuess(defaultResult);

    size_t threadCount = 1ull; //std::thread::hardware_concurrency();
    thread_pool::ThreadPool tp{threadCount};
    const size_t wordsPerThread = wordCount / threadCount;

    std::vector<std::future<std::vector<int>>> res;
    res.reserve(threadCount);

    std::shared_ptr<std::ofstream> outTxt = (threadCount == 1) ? std::make_shared<std::ofstream>() : nullptr;
    if (outTxt) outTxt->open("out_results.txt");

    for (size_t i = 0; i <threadCount; i++) {
        const size_t startIndex = i * wordsPerThread;
        const size_t endIndex = (i+1) * wordsPerThread + ((i == threadCount-1) ? wordCount % wordsPerThread : 0);

        const auto& job = [knowledge, dict, outTxt, initialGuess, wordsPerThread, startIndex, endIndex]() mutable {
            std::vector<int> iterationCounts;
            iterationCounts.reserve(wordsPerThread);
            for (size_t i = startIndex; i < endIndex; i++) {
                Knowledge currKnowledge = knowledge;
                auto iterations = iterationCount(currKnowledge, initialGuess, dict.word(i), outTxt);
                iterationCounts.push_back(iterations);
            }
            return iterationCounts;
        };
        res.push_back(tp.submit(job));
    }
    std::vector<float> averageVec;
    averageVec.reserve(wordCount);

    for (auto& r : res) {
        for (const auto& count : r.get()) {
            averageVec.push_back(count);
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::minutes>(stop - start);
    if (outTxt) outTxt->close();

    std::cout << "The average is: " << average(averageVec) << ", duration: " << duration.count() << '\n';
}

void runAgainstWebsite(DictManager& dict, Knowledge& knowledge) {
    std::optional<result> res = defaultResult;
    std::string resStr;
    std::string guess;
    int i = 0;
    for (; i < 20 && knowledge.wordCount() != 0; i++) {
        knowledge.updatePopularity();
        guess = knowledge.generateGuess(*res);

        std::cout << "guess:" << guess << '\n';
        do {
            std::cout << "Enter Result:";
            std::cin >> resStr;
            res = stringToResult(resStr);
        } while (res == std::nullopt);

        if (isComplete(*res)) {
            std::cout << i + 1 << '\n';
            break;
        }
        else {
            knowledge.ween(guess, *res);
            std::cout << "word count: " << knowledge.wordCount() << '\n';
        }
    }
}

int main() {
    std::unique_ptr<DictManager> dict = std::make_unique<DictManager>();
    std::unique_ptr<Knowledge> knowledge = std::make_unique<Knowledge>();
    dict->load();
    knowledge->load(*dict);

    baseLineAlgorithm(*dict, *knowledge);
    //runAgainstWebsite(*dict, *knowledge);

    return 0;
}