#pragma once

#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <optional>


using result = std::array<int, 5>;
static constexpr result defaultResult = result{ 0,0,0,0,0 };



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

void print(result res) {
    for (int i = 0; i < 4; i++) {
        std::cout << res[i] << " ";
    }
    std::cout << res[4] << '\n';
}

std::optional<result> stringToResult(std::string_view s) {
    std::cout << s << '\n';
    if (s.length() >= 5) {
        std::optional<result> res = result{ 0,0,0,0,0 };
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
