#pragma once

#include "Result.h"

#include <vector>
#include <string_view>
#include <map>
#include <set>



class Knowledge {
public:
    void load(const std::vector<std::string_view>& words) {
        m_words = words;
    };

    std::string_view generateGuess(result res) {
        int maxScore = 0;
        std::string_view currentGuess = "";
        for (const auto& word : m_words) {
            int currScore = generateWordScore(word, res);
            if (currScore > maxScore) {
                maxScore = currScore;
                currentGuess = word;
            }
        }
        return currentGuess;
    };

    void ween(std::string_view guess, result res) {
        if (m_words.size() == 1) return;
        for (auto iter = m_words.begin(); iter != m_words.end();) {
            auto word = *iter;
            bool wordKept = keepWord(word, guess, res);
            if (!wordKept || word == guess) {
                iter = m_words.erase(iter);
            }
            else {
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
            int reduceFoundLetterCount = (m_popularity[c] > m_words.size()) ? m_words.size() : 0;
            int charsWeightedPop = (chars.find(c) != chars.end()) ? 1 : (m_popularity[c] - reduceFoundLetterCount);
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
                    }
                    else if (!found && gc != g && rc != 2 && w == g && oneIndexLocations.find(j) == oneIndexLocations.end()) {
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
    std::map<char, int> m_popularity;
    std::map<char, std::array<int, 5>> m_positionPopularity;

};
