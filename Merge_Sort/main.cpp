#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
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
// Balanced Multiway Sort Components
// -----------------------------------------------------------------------------

int distributeMultiway(const std::string &filename,
                       const std::vector<std::string> &outNames,
                       const int filesUsed, const MergeMode mode,
                       const int directParitionSize) {
  std::ifstream inMain(filename);
  std::vector<std::ofstream> outFiles(filesUsed);
  for (int i = 0; i < filesUsed; ++i)
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
    if (mode == MergeMode::Direct && countInRun >= directParitionSize)
      endOfRun = true;
    if (mode == MergeMode::Natural && current < prev)
      endOfRun = true;

    if (endOfRun) {
      fileIdx = (fileIdx + 1) % filesUsed;
      countInRun = 0;
      totalRuns++;
    }
    outFiles[fileIdx] << current << " ";
    countInRun++;
    prev = current;
  }

  for (int i = 0; i < filesUsed; ++i)
    outFiles[i].close();
  return totalRuns;
}

int executeMultiwayMergePass(const std::vector<std::string> &inNames,
                             const std::vector<std::string> &outNames,
                             const int filesUsed, const MergeMode mode,
                             const int directParitionSize) {
  std::vector<std::ifstream> inStreams(filesUsed);
  std::vector<std::ofstream> outStreams(filesUsed);

  for (int i = 0; i < filesUsed; ++i) {
    inStreams[i].open(inNames[i]);
    outStreams[i].open(outNames[i], std::ios::trunc);
  }

  int outIdx = 0;
  int activeFilesCount = 0;
  int totalRunsProduced = 0;

  // Массив segmentNumbers сохраняет самое первое значение в файле
  // это для того чтобы сравнить и выбирать
  std::vector<int> segmentNumbers(filesUsed);
  std::vector<bool> activeStreams(filesUsed, false);
  std::vector<int> currentRunCount(filesUsed, 0);

  // Initial read
  for (int i = 0; i < filesUsed; ++i) {
    if (inStreams[i] >> segmentNumbers[i]) {
      activeStreams[i] = true;
      currentRunCount[i] = 1;
      activeFilesCount++;
    }
  }

  while (activeFilesCount > 0) {
    int activeRunsCount = activeFilesCount;
    std::vector<int> runPrev(filesUsed, std::numeric_limits<int>::min());
    totalRunsProduced++;

    while (activeRunsCount > 0) {
      int minIndex = -1;
      int minValue = std::numeric_limits<int>::max();

      for (int i = 0; i < filesUsed; ++i) {
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

        if (mode == MergeMode::Direct &&
            currentRunCount[minIndex] > directParitionSize)
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
    for (int i = 0; i < filesUsed; ++i) {
      if (!inStreams[i].eof() && !inStreams[i].fail()) {
        activeStreams[i] = true;
      }
    }
    outIdx = (outIdx + 1) % filesUsed;
  }

  for (int i = 0; i < filesUsed; ++i) {
    inStreams[i].close();
    outStreams[i].close();
  }

  return totalRunsProduced;
}

void balancedMultiwaySort(const std::string &filename, const int filesUsed,
                          const MergeMode mode) {
  std::vector<std::string> f_names(filesUsed), g_names(filesUsed);
  for (int i = 0; i < filesUsed; ++i) {
    f_names[i] = "f_" + std::to_string(i) + ".txt";
    g_names[i] = "g_" + std::to_string(i) + ".txt";
  }

  int p = 1;
  bool isSorted = false;
  bool directionFtoG = true;

  int initialRuns = distributeMultiway(filename, f_names, filesUsed, mode, p);
  if (initialRuns <= 1) {
    for (int i = 0; i < filesUsed; ++i) {
      std::remove(f_names[i].c_str());
      std::remove(g_names[i].c_str());
    }
    return;
  }

  while (!isSorted) {
    const auto &inNames = directionFtoG ? f_names : g_names;
    const auto &outNames = directionFtoG ? g_names : f_names;

    int runsThisPass =
        executeMultiwayMergePass(inNames, outNames, filesUsed, mode, p);

    if (mode == MergeMode::Direct)
      p *= filesUsed;
    directionFtoG = !directionFtoG;

    if (runsThisPass <= 1)
      isSorted = true;
  }

  const auto &finalNames = directionFtoG ? f_names : g_names;
  std::ifstream resIn(finalNames[0]);
  std::ofstream resOut(filename, std::ios::trunc);
  resOut << resIn.rdbuf();

  for (int i = 0; i < filesUsed; ++i) {
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
                        std::vector<int> &idealPartition,
                        std::vector<int> &missingPartition,
                        const int filesUsed) {
  std::vector<int> t(filesUsed - 1, 0);
  t[0] = 1;

  std::vector<std::unique_ptr<std::ofstream>> outs(filesUsed - 1);
  for (int i = 0; i < filesUsed - 1; ++i)
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
    idealPartition[tapeIdx]++;

    if (idealPartition[tapeIdx] == t[tapeIdx]) {
      tapeIdx++;
      if (tapeIdx == filesUsed - 1) {
        int t0 = t[0];
        for (int i = 0; i < filesUsed - 2; ++i)
          t[i] = t0 + t[i + 1];
        t[filesUsed - 2] = t0;
        tapeIdx = 0;
      }
    }
    hasData = !runContinues;
  }

  int total_actual = 0;
  for (int i = 0; i < filesUsed - 1; ++i) {
    missingPartition[i] = t[i] - idealPartition[i];
    total_actual += idealPartition[i];
  }
  return total_actual;
}

// Phase 2: Single polyphase merge step into the designated empty tape
void executePolyphaseMergePass(const std::vector<std::string> &tapes,
                               std::vector<int> &idealPartition,
                               std::vector<int> &missingPartition,
                               std::vector<int> &tapeMap, int filesUsed,
                               int mergePasses) {
  int outIdx = tapeMap[filesUsed - 1];
  std::ofstream outStream(tapes[outIdx], std::ios::trunc);

  std::vector<std::unique_ptr<std::ifstream>> inStreams(filesUsed - 1);
  std::vector<int> currentVals(filesUsed - 1);
  std::vector<bool> active(filesUsed - 1, false);

  for (int i = 0; i < filesUsed - 1; ++i) {
    inStreams[i] = std::make_unique<std::ifstream>(tapes[tapeMap[i]]);
  }

  for (int p = 0; p < mergePasses; ++p) {
    bool allDummy = true;
    for (int i = 0; i < filesUsed - 1; ++i) {
      if (missingPartition[tapeMap[i]] > 0) {
        missingPartition[tapeMap[i]]--;
      } else {
        allDummy = false;
        idealPartition[tapeMap[i]]--;
        if (*inStreams[i] >> currentVals[i])
          active[i] = true;
      }
    }

    if (allDummy) {
      missingPartition[outIdx]++;
    } else {
      std::vector<int> prevVals(filesUsed - 1, std::numeric_limits<int>::min());
      bool runsActive = true;

      while (runsActive) {
        int minIdx = -1;
        int minVal = std::numeric_limits<int>::max();

        for (int i = 0; i < filesUsed - 1; ++i) {
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
        for (int i = 0; i < filesUsed - 1; ++i)
          if (active[i])
            runsActive = true;
      }
      idealPartition[outIdx]++;
    }
  }
}

void polyphaseSort(const std::string &filename, int auxFilesCount = 3) {

  int filesUsed = auxFilesCount + 1;
  std::vector<std::string> tapes(filesUsed);
  for (int i = 0; i < filesUsed; ++i)
    tapes[i] = "poly_tape_" + std::to_string(i) + ".txt";

  std::vector<int> idealPartitions(filesUsed, 0);
  std::vector<int> missingPartitions(filesUsed, 0);
  std::vector<int> tapeMap(filesUsed);
  for (int i = 0; i < filesUsed; ++i)
    tapeMap[i] = i;

  int total_actual = distributePolyphase(filename, tapes, idealPartitions,
                                         missingPartitions, filesUsed);

  if (total_actual <= 1) {
    if (total_actual == 1) {
      for (int i = 0; i < filesUsed - 1; ++i) {
        if (idealPartitions[i] == 1) {
          std::ifstream fin(tapes[i]);
          std::ofstream fout(filename, std::ios::trunc);
          fout << fin.rdbuf();
          break;
        }
      }
    }
    for (int i = 0; i < filesUsed; ++i)
      std::remove(tapes[i].c_str());
    return;
  }

  while (true) {
    int mergePasses = idealPartitions[tapeMap[filesUsed - 2]] +
                      missingPartitions[tapeMap[filesUsed - 2]];
    if (mergePasses == 0)
      break;

    executePolyphaseMergePass(tapes, idealPartitions, missingPartitions,
                              tapeMap, filesUsed, mergePasses);

    // Rotate Tapes
    int oldEmpty = tapeMap[filesUsed - 2];
    for (int i = filesUsed - 2; i > 0; --i)
      tapeMap[i] = tapeMap[i - 1];
    tapeMap[0] = tapeMap[filesUsed - 1];
    tapeMap[filesUsed - 1] = oldEmpty;

    int total_runs_left = 0;
    for (int i = 0; i < filesUsed; ++i)
      total_runs_left += idealPartitions[i];
    if (total_runs_left <= 1)
      break;
  }

  for (int i = 0; i < filesUsed; ++i) {
    if (idealPartitions[tapeMap[i]] == 1) {
      std::ifstream fin(tapes[tapeMap[i]]);
      std::ofstream fout(filename, std::ios::trunc);
      fout << fin.rdbuf();
      break;
    }
  }
  for (int i = 0; i < filesUsed; ++i)
    std::remove(tapes[i].c_str());
}

// --------- Помощники -----------
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

double sortFilewithTime(const std::string &fileName, SortAlgorithm algo,
                        MergeMode mode, int filesCount) {

  auto start = std::chrono::steady_clock::now();

  if (algo == SortAlgorithm::BalancedMultiway) {
    balancedMultiwaySort(fileName, filesCount, mode);
  } else {
    polyphaseSort(fileName, filesCount);
  }

  auto stop = std::chrono::steady_clock::now();

  if (!isFileContainsSortedArray(fileName))
    return -2;

  return std::chrono::duration<double, std::milli>(stop - start).count();
}

// deterministic dataset seed
unsigned int datasetSeed(size_t n, int lo, int hi) {
  uint64_t s = 1469598103934665603ull;
  s ^= n;
  s *= 1099511628211ull;
  s ^= static_cast<uint64_t>(lo + 0x9e3779b1);
  s *= 1099511628211ull;
  s ^= static_cast<uint64_t>(hi ^ 0x9e3779b1);
  s *= 1099511628211ull;
  return static_cast<unsigned int>(s & 0xFFFFFFFFu);
}

void genRandomtoFile(const std::string &fname, const size_t n, const int lo,
                     const int hi, const unsigned int seed) {
  std::ofstream os(fname);
  if (!os)
    throw std::runtime_error("Failed to open " + fname);

  std::mt19937_64 rng(seed);
  std::uniform_int_distribution<int> dist(lo, hi);
  int randomNumber{};
  os << n << '\n';

  for (size_t i = 0; i < n; ++i) {
    randomNumber = dist(rng);
    os << randomNumber << ((i + 1 == n) ? '\n' : ' ');
  }
}

bool fileExists(const std::string &filename) {
  std::ifstream file(filename);
  bool exists = file.good();
  file.close();
  return exists;
}

void copyFile(const std::string &source, const std::string &destination) {
  std::ifstream src(source, std::ios::binary);
  std::ofstream dst(destination, std::ios::binary);

  dst << src.rdbuf();
}

int main() {

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  const std::vector<size_t> sizes = {1000u, 10000u, 100000u};
  const std::vector<std::pair<int, int>> ranges = {
      {-10, 10}, {-1000, 1000}, {-100000, 100000}};

  std::vector<std::pair<MergeMode, SortAlgorithm>> modes{
      {MergeMode::Direct, SortAlgorithm::BalancedMultiway},
      {MergeMode::Natural, SortAlgorithm::BalancedMultiway},
      {MergeMode::Natural, SortAlgorithm::Polyphase}};
  constexpr int kFilesUsed = 3;

  std::ofstream csv_append("timings_External.csv", std::ios::app);
  csv_append << "Algorithm" << ',' << "Sorting Mode" << ',' << "Array Size"
             << ',' << "Lowest" << ',' << "Highest"
             << "Files" << std::fixed << std::setprecision(3)
             << "First run time" << ',' << "Second run time" << ','
             << "Third run time" << ',' << "Mean" << "\n";

  for (auto [mode, algo] : modes) {
    std::string s_mode = (mode == MergeMode::Direct) ? "Direct" : "Natural";
    std::string s_algo =
        (algo == SortAlgorithm::Polyphase) ? "Polyphase" : "BalancedMultiway";
    std::cout << "Executing " << s_algo << " " << s_mode << "\n";

    for (size_t n : sizes) {
      for (auto [lo, hi] : ranges) {
        // Dataset generation
        std::string fname = "data_" + std::to_string(n) + "_" +
                            std::to_string(lo) + "_" + std::to_string(hi) +
                            ".txt";

        std::vector<int> data;
        std::string data_dupe = "dupe.txt";

        if (!fileExists(fname)) {
          unsigned int seed = datasetSeed(n, lo, hi);
          std::cout << "Generating dataset n=" << n << " range=[" << lo << ','
                    << hi << "] seed=" << seed << " ...\n";
          genRandomtoFile(fname, n, lo, hi, seed);
        } else {
          std::cout << "Found dataset n=" << n << " range=[" << lo << ',' << hi
                    << "]\n";
        }

        // Duplicate dataset
        std::vector<double> runs;
        runs.reserve(3);

        for (int r = 0; r < 3; ++r) {
          copyFile(fname, data_dupe);

          double tms = sortFilewithTime(data_dupe, algo, mode, kFilesUsed);
          if (tms == -2) {
            std::cerr << "ERROR: " << s_algo << " " << s_mode
                      << " produced unsorted result"
                      << "\n";
            std::terminate();
          } else if (tms == -1) {
            std::cerr << "ERROR: " << s_algo << " " << s_mode
                      << " unable to create file" << "\n";
            std::terminate();
          }

          runs.push_back(tms);
          std::cout << std::fixed << std::setprecision(2) << tms << " ms"
                    << (r < 2 ? ", " : "");
        }

        double mean = (runs[0] + runs[1] + runs[2]) / 3.0;
        std::cout << "  mean=" << std::fixed << std::setprecision(2) << mean
                  << " ms\n";

        std::ofstream csv_append("timings_External.csv", std::ios::app);
        csv_append << s_algo << ',' << s_mode << ',' << n << ',' << lo << ','
                   << hi << "," << kFilesUsed << "," << std::fixed
                   << std::setprecision(3) << runs[0] << ',' << runs[1] << ','
                   << runs[2] << ',' << mean << "\n";
        csv_append.close();
      }
    }
  }

  std::cout << "All experiments complete. See timings_count.csv and data_*.txt "
               "files.\n";
  return 0;
}