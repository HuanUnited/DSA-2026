#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
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
constexpr int32_t SENTINEL = INT32_MIN;

void distributePolyphase(const std::string &filename,
                         std::vector<std::unique_ptr<std::fstream>> &files,
                         std::vector<int> &ip, std::vector<int> &ms,
                         int &levels) {
  int n = files.size();
  for (int k = 0; k < n - 1; ++k) {
    ip[k] = 1;
    ms[k] = 1;
  }
  ip[n - 1] = 0;
  ms[n - 1] = 0;

  levels = 1;
  int i = 0;

  std::ifstream source(filename);
  int32_t current;

  if (!(source >> current))
    return;

  bool hasData = true;
  while (hasData) {
    *files[i] << current << " ";
    int32_t prev = current;
    bool runContinues = false;

    while (source >> current) {
      if (current < prev) {
        runContinues = true;
        break;
      }
      *files[i] << current << " ";
      prev = current;
    }

    *files[i] << SENTINEL << " ";
    ms[i]--;

    if (i < n - 2 && ms[i] < ms[i + 1]) {
      i++;
    } else if (ms[i] == 0) {
      levels++;
      int ip0 = ip[0];
      i = 0;
      for (int k = 0; k < n - 1; ++k) {
        ms[k] = ip[k + 1] - ip[k] + ip0;
        ip[k] = ip[k + 1] + ip0;
      }
    } else {
      i = 0;
    }

    hasData = runContinues;
  }
}

void polyphaseSort(const std::string &filename, int auxFilesCount = 3) {
  int n = auxFilesCount + 1;
  std::vector<std::string> fileNames(n);
  std::vector<std::unique_ptr<std::fstream>> files(n);

  for (int i = 0; i < n; ++i) {
    fileNames[i] = "poly_tape_" + std::to_string(i) + ".txt";
    files[i] = std::make_unique<std::fstream>(fileNames[i],
                                              std::ios::out | std::ios::trunc);
  }

  std::vector<int> ip(n, 0);
  std::vector<int> ms(n, 0);
  int L = 0;

  distributePolyphase(filename, files, ip, ms, L);

  if (L <= 1)
    return;

  for (int i = 0; i < n - 1; ++i) {
    files[i]->close();
    files[i]->open(fileNames[i], std::ios::in);
  }
  files[n - 1]->close();
  files[n - 1]->open(fileNames[n - 1], std::ios::out | std::ios::trunc);

  std::vector<int32_t> currentVals(n - 1, SENTINEL);
  std::vector<bool> active(n - 1, false);

  while (L > 0) {
    while (ip[n - 2] > 0) {
      bool allDummy = true;
      for (int i = 0; i < n - 1; ++i) {
        if (ms[i] == 0)
          allDummy = false;
      }

      if (allDummy) {
        for (int i = 0; i < n - 1; ++i)
          ms[i]--;
        ms[n - 1]++;
      } else {
        for (int i = 0; i < n - 1; ++i) {
          active[i] = false;
          if (ms[i] > 0) {
            ms[i]--;
          } else {
            *files[i] >> currentVals[i];
            if (currentVals[i] != SENTINEL)
              active[i] = true;
          }
        }

        bool runActive = true;
        while (runActive) {
          int minIdx = -1;
          int32_t minVal = std::numeric_limits<int32_t>::max();

          for (int i = 0; i < n - 1; ++i) {
            if (active[i] && currentVals[i] < minVal) {
              minVal = currentVals[i];
              minIdx = i;
            }
          }

          if (minIdx == -1)
            break;

          *files[n - 1] << minVal << " ";

          *files[minIdx] >> currentVals[minIdx];
          if (currentVals[minIdx] == SENTINEL) {
            active[minIdx] = false;
          }

          runActive = false;
          for (int i = 0; i < n - 1; ++i) {
            if (active[i])
              runActive = true;
          }
        }
        *files[n - 1] << SENTINEL << " ";
      }

      for (int i = 0; i < n - 1; ++i)
        ip[i]--;
      ip[n - 1]++;
    }

    L--;

    files[n - 2]->close();
    files[n - 1]->close();

    auto tmpFile = std::move(files[n - 1]);
    std::string tmpName = fileNames[n - 1];
    int tmpIp = ip[n - 1];
    int tmpMs = ms[n - 1];

    for (int i = n - 1; i > 0; --i) {
      files[i] = std::move(files[i - 1]);
      fileNames[i] = fileNames[i - 1];
      ip[i] = ip[i - 1];
      ms[i] = ms[i - 1];
    }

    files[0] = std::move(tmpFile);
    fileNames[0] = tmpName;
    ip[0] = tmpIp;
    ms[0] = tmpMs;

    files[0]->open(fileNames[0], std::ios::in);
    files[n - 1]->open(fileNames[n - 1], std::ios::out | std::ios::trunc);
  }

  files[0]->close();
  std::ifstream finalIn(fileNames[0]);
  std::ofstream finalOut(filename, std::ios::trunc);

  int32_t val;
  while (finalIn >> val) {
    if (val != SENTINEL) {
      finalOut << val << " ";
    }
  }

  finalIn.close();
  finalOut.close();

  for (int i = 0; i < n; ++i) {
    std::remove(fileNames[i].c_str());
  }
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
  constexpr int kFilesUsed = 8;

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