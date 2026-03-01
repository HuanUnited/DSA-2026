#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

enum class MergeMode {
  Direct, // Fixed run lengths p=1, 2, 4...
  Natural // Natural non-decreasing sequences
};

enum class SortAlgorithm { BalancedMultiway, Polyphase };

// -----------------------------------------------------------------------------
// Utilities
// -----------------------------------------------------------------------------

bool createFileWithRandomNumbers(const std::string &fileName,
                                 const int numbersCount,
                                 const int maxNumberValue) {
  std::ofstream outFile(fileName, std::ios::trunc);
  if (!outFile.is_open())
    return false;

  std::mt19937_64 rng(12345);
  std::uniform_int_distribution<int> dist(-maxNumberValue, maxNumberValue);

  for (int i = 0; i < numbersCount; ++i) {
    outFile << dist(rng) << (i == numbersCount - 1 ? "" : " ");
  }
  return true;
}

bool isFileContainsSortedArray(const std::string &fileName) {
  std::ifstream inFile(fileName);
  if (!inFile.is_open())
    return false;

  int current, previous;
  if (!(inFile >> previous))
    return true;

  while (inFile >> current) {
    if (current < previous)
      return false;
    previous = current;
  }
  return true;
}

// -----------------------------------------------------------------------------
// Balanced Multiway Sort Components
// -----------------------------------------------------------------------------

int distributeMultiway(const std::string &filename,
                       const std::vector<std::string> &outNames, int n,
                       MergeMode mode, int p) {
  std::ifstream inMain(filename);
  std::vector<std::ofstream> outFiles(n);
  for (int i = 0; i < n; ++i)
    outFiles[i].open(outNames[i], std::ios::trunc);

  int current;
  if (!(inMain >> current))
    return 0;

  int fileIdx = 0;
  int countInRun = 1;
  outFiles[fileIdx] << current << " ";
  int prev = current;
  int totalRuns = 1;

  while (inMain >> current) {
    bool endOfRun = false;
    if (mode == MergeMode::Direct && countInRun >= p)
      endOfRun = true;
    if (mode == MergeMode::Natural && current < prev)
      endOfRun = true;

    if (endOfRun) {
      fileIdx = (fileIdx + 1) % n;
      countInRun = 0;
      totalRuns++;
    }
    outFiles[fileIdx] << current << " ";
    countInRun++;
    prev = current;
  }

  for (int i = 0; i < n; ++i)
    outFiles[i].close();
  return totalRuns;
}

int executeMultiwayMergePass(const std::vector<std::string> &inNames,
                             const std::vector<std::string> &outNames, int n,
                             MergeMode mode, int p) {
  std::vector<std::ifstream> inStreams(n);
  std::vector<std::ofstream> outStreams(n);

  for (int i = 0; i < n; ++i) {
    inStreams[i].open(inNames[i]);
    outStreams[i].open(outNames[i], std::ios::trunc);
  }

  int outIdx = 0;
  int activeFilesCount = 0;
  int totalRunsProduced = 0;

  std::vector<int> segmentNumbers(n);
  std::vector<bool> activeStreams(n, false);
  std::vector<int> currentRunCount(n, 0);

  // Initial read
  for (int i = 0; i < n; ++i) {
    if (inStreams[i] >> segmentNumbers[i]) {
      activeStreams[i] = true;
      currentRunCount[i] = 1;
      activeFilesCount++;
    }
  }

  while (activeFilesCount > 0) {
    int activeRunsCount = activeFilesCount;
    std::vector<int> runPrev(n, std::numeric_limits<int>::min());
    totalRunsProduced++;

    while (activeRunsCount > 0) {
      int minIndex = -1;
      int minValue = std::numeric_limits<int>::max();

      for (int i = 0; i < n; ++i) {
        if (activeStreams[i] && segmentNumbers[i] < minValue) {
          minValue = segmentNumbers[i];
          minIndex = i;
        }
      }

      if (minIndex == -1)
        break;

      outStreams[outIdx] << segmentNumbers[minIndex] << " ";
      runPrev[minIndex] = segmentNumbers[minIndex];

      if (inStreams[minIndex] >> segmentNumbers[minIndex]) {
        currentRunCount[minIndex]++;
        bool runEnded = false;

        if (mode == MergeMode::Direct && currentRunCount[minIndex] > p)
          runEnded = true;
        if (mode == MergeMode::Natural &&
            segmentNumbers[minIndex] < runPrev[minIndex])
          runEnded = true;

        if (runEnded) {
          activeStreams[minIndex] = false;
          activeRunsCount--;
          currentRunCount[minIndex] = 1;
        }
      } else {
        activeStreams[minIndex] = false;
        activeRunsCount--;
        activeFilesCount--;
      }
    }

    // Reactivate streams for the next run distribution
    for (int i = 0; i < n; ++i) {
      if (!inStreams[i].eof() && !inStreams[i].fail()) {
        activeStreams[i] = true;
      }
    }
    outIdx = (outIdx + 1) % n;
  }

  for (int i = 0; i < n; ++i) {
    inStreams[i].close();
    outStreams[i].close();
  }

  return totalRunsProduced;
}

void balancedMultiwaySort(const std::string &filename, int n, MergeMode mode) {
  std::vector<std::string> f_names(n), g_names(n);
  for (int i = 0; i < n; ++i) {
    f_names[i] = "f_" + std::to_string(i) + ".txt";
    g_names[i] = "g_" + std::to_string(i) + ".txt";
  }

  int p = 1;
  bool isSorted = false;
  bool directionFtoG = true;

  int initialRuns = distributeMultiway(filename, f_names, n, mode, p);
  if (initialRuns <= 1) {
    for (int i = 0; i < n; ++i) {
      std::remove(f_names[i].c_str());
      std::remove(g_names[i].c_str());
    }
    return;
  }

  while (!isSorted) {
    const auto &inNames = directionFtoG ? f_names : g_names;
    const auto &outNames = directionFtoG ? g_names : f_names;

    int runsThisPass = executeMultiwayMergePass(inNames, outNames, n, mode, p);

    if (mode == MergeMode::Direct)
      p *= n;
    directionFtoG = !directionFtoG;

    if (runsThisPass <= 1)
      isSorted = true;
  }

  const auto &finalNames = directionFtoG ? f_names : g_names;
  std::ifstream resIn(finalNames[0]);
  std::ofstream resOut(filename, std::ios::trunc);
  resOut << resIn.rdbuf();

  for (int i = 0; i < n; ++i) {
    std::remove(f_names[i].c_str());
    std::remove(g_names[i].c_str());
  }
}

// -----------------------------------------------------------------------------
// Polyphase Sort Components
// -----------------------------------------------------------------------------

// Phase 1: Fibonnaci distribution across K-1 tapes
int distributePolyphase(const std::string &filename,
                        const std::vector<std::string> &tapes,
                        std::vector<int> &a, std::vector<int> &d, int K) {
  std::vector<int> t(K - 1, 0);
  t[0] = 1;

  std::vector<std::unique_ptr<std::ofstream>> outs(K - 1);
  for (int i = 0; i < K - 1; ++i)
    outs[i] = std::make_unique<std::ofstream>(tapes[i], std::ios::trunc);

  std::ifstream source(filename);
  int current;
  bool hasData = static_cast<bool>(source >> current);
  int tapeIdx = 0;

  while (hasData) {
    *outs[tapeIdx] << current << " ";
    int prev = current;
    bool runContinues = true;

    while (source >> current) {
      if (current < prev) {
        runContinues = false;
        break;
      }
      *outs[tapeIdx] << current << " ";
      prev = current;
    }
    a[tapeIdx]++;

    if (a[tapeIdx] == t[tapeIdx]) {
      tapeIdx++;
      if (tapeIdx == K - 1) {
        int t0 = t[0];
        for (int i = 0; i < K - 2; ++i)
          t[i] = t0 + t[i + 1];
        t[K - 2] = t0;
        tapeIdx = 0;
      }
    }
    hasData = !runContinues;
  }

  int total_actual = 0;
  for (int i = 0; i < K - 1; ++i) {
    d[i] = t[i] - a[i];
    total_actual += a[i];
  }
  return total_actual;
}

// Phase 2: Single polyphase merge step into the designated empty tape
void executePolyphaseMergePass(const std::vector<std::string> &tapes,
                               std::vector<int> &a, std::vector<int> &d,
                               std::vector<int> &tapeMap, int K,
                               int mergePasses) {
  int outIdx = tapeMap[K - 1];
  std::ofstream outStream(tapes[outIdx], std::ios::trunc);

  std::vector<std::unique_ptr<std::ifstream>> inStreams(K - 1);
  std::vector<int> currentVals(K - 1);
  std::vector<bool> active(K - 1, false);

  for (int i = 0; i < K - 1; ++i) {
    inStreams[i] = std::make_unique<std::ifstream>(tapes[tapeMap[i]]);
  }

  for (int p = 0; p < mergePasses; ++p) {
    bool allDummy = true;
    for (int i = 0; i < K - 1; ++i) {
      if (d[tapeMap[i]] > 0) {
        d[tapeMap[i]]--;
      } else {
        allDummy = false;
        a[tapeMap[i]]--;
        if (*inStreams[i] >> currentVals[i])
          active[i] = true;
      }
    }

    if (allDummy) {
      d[outIdx]++;
    } else {
      std::vector<int> prevVals(K - 1, std::numeric_limits<int>::min());
      bool runsActive = true;

      while (runsActive) {
        int minIdx = -1;
        int minVal = std::numeric_limits<int>::max();

        for (int i = 0; i < K - 1; ++i) {
          if (active[i] && currentVals[i] < minVal) {
            minVal = currentVals[i];
            minIdx = i;
          }
        }

        if (minIdx == -1)
          break;

        outStream << minVal << " ";
        prevVals[minIdx] = minVal;

        if (*inStreams[minIdx] >> currentVals[minIdx]) {
          if (currentVals[minIdx] < prevVals[minIdx])
            active[minIdx] = false;
        } else {
          active[minIdx] = false;
        }

        runsActive = false;
        for (int i = 0; i < K - 1; ++i)
          if (active[i])
            runsActive = true;
      }
      a[outIdx]++;
    }
  }
}

void polyphaseSort(const std::string &filename, MergeMode mode,
                   int auxFilesCount = 3) {
  mode = MergeMode::Natural;

  int K = auxFilesCount + 1;
  std::vector<std::string> tapes(K);
  for (int i = 0; i < K; ++i)
    tapes[i] = "poly_tape_" + std::to_string(i) + ".txt";

  std::vector<int> a(K, 0);
  std::vector<int> d(K, 0);
  std::vector<int> tapeMap(K);
  for (int i = 0; i < K; ++i)
    tapeMap[i] = i;

  int total_actual = distributePolyphase(filename, tapes, a, d, K);

  if (total_actual <= 1) {
    if (total_actual == 1) {
      for (int i = 0; i < K - 1; ++i) {
        if (a[i] == 1) {
          std::ifstream fin(tapes[i]);
          std::ofstream fout(filename, std::ios::trunc);
          fout << fin.rdbuf();
          break;
        }
      }
    }
    for (int i = 0; i < K; ++i)
      std::remove(tapes[i].c_str());
    return;
  }

  while (true) {
    int mergePasses = a[tapeMap[K - 2]] + d[tapeMap[K - 2]];
    if (mergePasses == 0)
      break;

    executePolyphaseMergePass(tapes, a, d, tapeMap, K, mergePasses);

    // Rotate Tapes
    int oldEmpty = tapeMap[K - 2];
    for (int i = K - 2; i > 0; --i)
      tapeMap[i] = tapeMap[i - 1];
    tapeMap[0] = tapeMap[K - 1];
    tapeMap[K - 1] = oldEmpty;

    int total_runs_left = 0;
    for (int i = 0; i < K; ++i)
      total_runs_left += a[i];
    if (total_runs_left <= 1)
      break;
  }

  for (int i = 0; i < K; ++i) {
    if (a[tapeMap[i]] == 1) {
      std::ifstream fin(tapes[tapeMap[i]]);
      std::ofstream fout(filename, std::ios::trunc);
      fout << fin.rdbuf();
      break;
    }
  }
  for (int i = 0; i < K; ++i)
    std::remove(tapes[i].c_str());
}

// -----------------------------------------------------------------------------
// Execution Hooks
// -----------------------------------------------------------------------------

void sortFile(const std::string &fileName, SortAlgorithm algo, MergeMode mode,
              int filesCount) {
  if (algo == SortAlgorithm::BalancedMultiway) {
    balancedMultiwaySort(fileName, filesCount, mode);
  } else {
    polyphaseSort(fileName, mode, filesCount);
  }
}

int createAndSortFile(const std::string &fileName, const int numbersCount,
                      const int maxNumberValue, SortAlgorithm algo,
                      MergeMode mode, int filesCount) {
  if (!createFileWithRandomNumbers(fileName, numbersCount, maxNumberValue))
    return -1;
  sortFile(fileName, algo, mode, filesCount);
  if (!isFileContainsSortedArray(fileName))
    return -2;
  return 1;
}

int main() {
  std::string fileName = "test_data.txt";
  const int numbersCount = 500000;
  const int maxNumberValue = 100000;

  std::cout << "Testing Balanced Multiway (Direct, n=3)...\n";
  for (int i = 0; i < 3; i++) {
    int result = createAndSortFile(fileName, numbersCount, maxNumberValue,
                                   SortAlgorithm::BalancedMultiway,
                                   MergeMode::Direct, 3);
    std::cout << "  Pass " << i + 1 << ": " << (result == 1 ? "OK" : "FAIL")
              << "\n";
  }

  std::cout << "Testing Balanced Multiway (Natural, n=3)...\n";
  for (int i = 0; i < 3; i++) {
    int result = createAndSortFile(fileName, numbersCount, maxNumberValue,
                                   SortAlgorithm::BalancedMultiway,
                                   MergeMode::Natural, 3);
    std::cout << "  Pass " << i + 1 << ": " << (result == 1 ? "OK" : "FAIL")
              << "\n";
  }

  std::cout << "Testing Polyphase Sort (Natural, aux=3)...\n";
  for (int i = 0; i < 3; i++) {
    int result =
        createAndSortFile(fileName, numbersCount, maxNumberValue,
                          SortAlgorithm::Polyphase, MergeMode::Natural, 3);
    std::cout << "  Pass " << i + 1 << ": " << (result == 1 ? "OK" : "FAIL")
              << "\n";
  }

  return 0;
}