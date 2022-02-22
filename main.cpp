
#include <iostream>
#include <stdlib.h>
#include <numeric>

#include "ThreadPool.h"
#include "Result.h"
#include "Knowledge.h"
#include "DictManager.h"

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

    size_t threadCount = std::thread::hardware_concurrency();
    thread_pool::ThreadPool tp{threadCount};
    const size_t wordsPerThread = wordCount / threadCount;

    std::vector<std::future<std::vector<int>>> res;
    res.reserve(threadCount);

    std::shared_ptr<std::ofstream> outTxt = (threadCount == 1) ? std::make_shared<std::ofstream>() : nullptr;
    if (outTxt) outTxt->open("out_results.txt");

    for (size_t i = 0; i <threadCount; i++) {
        const size_t startIndex = i * wordsPerThread;
        const size_t remainingWords = (i == threadCount - 1) ? wordCount % wordsPerThread : 0;
        const size_t endIndex = (i+1) * wordsPerThread + remainingWords;

        const auto& job = [knowledge, dict, outTxt, initialGuess, startIndex, endIndex]() mutable {
            std::vector<int> iterationCounts;
            iterationCounts.reserve(endIndex - startIndex);
            for (size_t i = startIndex; i < endIndex; i++) {
                auto iterations = iterationCount(knowledge, initialGuess, dict.word(i), outTxt);
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
    auto duration = duration_cast<std::chrono::seconds>(stop - start);
    if (outTxt) outTxt->close();

    std::cout << "The average is: " << average(averageVec) << ", duration: " << duration.count() << "(s)" << '\n';
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
    knowledge->load(dict->words());

    baseLineAlgorithm(*dict, *knowledge);
    //runAgainstWebsite(*dict, *knowledge);

    return 0;
}