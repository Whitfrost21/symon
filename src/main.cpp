#include "../external/nlohmann/json.hpp"
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
using json = nlohmann::json;
// stdbuf -oL ./a.out | head -n 5 | jq .

class Metric {
public:
  virtual ~Metric() = default;

  virtual void update() = 0;
  virtual json snapshot() const = 0;
};

class CpuMetric : public Metric {
private:
  struct CPUtimes {
    long long user, nice, system, idle, iowait, irq, softirq;
  };
  using CPUSet = std::vector<CPUtimes>;
  struct CPUusage {
    double total;
    std::vector<double> cores;
  };
  CPUSet curr, prev;
  CPUusage usage;
  bool initialized = false;

  // read all cpu times
  void readallcpus() {
    std::ifstream file("/proc/stat");
    std::string line;
    curr.clear();
    while (std::getline(file, line)) {
      if (line.rfind("cpu", 0) != 0) {
        break;
      }
      std::istringstream iss(line);
      std::string cpu;
      CPUtimes t{};

      iss >> cpu >> t.user >> t.nice >> t.system >> t.idle >> t.iowait >>
          t.irq >> t.softirq;

      curr.push_back(t);
    }
  }

  long long total(CPUtimes t) {
    return t.user + t.nice + t.system + t.idle + t.iowait + t.irq + t.softirq;
  }

  double cpuUsage(const CPUtimes &prev, const CPUtimes &curr) {
    long long totald = total(curr) - total(prev);
    long long idled = (curr.idle + curr.iowait) - (prev.idle + prev.iowait);
    return totald > 0 ? (totald - idled) * 100.0 / totald : 0.0;
  }

  void computeUsage() {
    if (prev.size() != curr.size()) {
      prev = curr;
    }
    usage.total = cpuUsage(prev[0], curr[0]);
    usage.cores.clear();

    for (size_t i = 1; i < curr.size(); ++i) {
      usage.cores.push_back(cpuUsage(prev[i], curr[i]));
    }
  }

public:
  void update() override {
    readallcpus();
    if (!initialized) {
      prev = curr;
      initialized = true;
      return;
    }
    computeUsage();
    prev = curr;
  }

  // std::cout << "{\"cpu\": " << cpu.total << ", "
  //           << "\"cpu_cores\": [";
  // for (size_t i = 0; i < cpu.cores.size(); ++i) {
  //   if (i)
  //     std::cout << ", ";
  //   std::cout << cpu.cores[i];
  // }
  json snapshot() const override {
    json j;
    j["cpu"]["total"] = usage.total;
    j["cpu"]["cores"] = usage.cores;
    return j;
  }
};

class MemoryMetric : public Metric {
private:
  struct Meminfo {
    long long totalkb = 0;
    long long freekb = 0;
    long long bufferkb = 0;
    long long cachekb = 0;
  };
  Meminfo mem;
  long long usedmb;
  long long totalmb;
  void readmemory() {
    std::ifstream file("/proc/meminfo");
    std::string key;
    long long value;
    std::string unit;

    while (file >> key >> value >> unit) {
      if (key == "MemTotal:") {
        mem.totalkb = value;
      } else if (key == "MemFree:") {
        mem.freekb = value;
      } else if (key == "Buffers:") {
        mem.bufferkb = value;
      } else if (key == "Cached:") {
        mem.cachekb = value;
      }
    }
  }

public:
  void update() override {
    readmemory();
    long long usedkb = mem.totalkb - mem.freekb - mem.bufferkb - mem.cachekb;
    usedmb = usedkb / 1024;
    totalmb = mem.totalkb / 1024;
  }

  json snapshot() const override {
    return {
        {"mem_used", usedmb},
        {"mem_total", totalmb},
    };
  }
};

class Engine {
private:
  std::vector<std::unique_ptr<Metric>> metrics;

public:
  void add(std::unique_ptr<Metric> m) { metrics.push_back(std::move(m)); }

  void tick() {
    for (auto &m : metrics) {
      m->update();
    }
  }

  json collect() {
    json result;
    for (const auto &m : metrics) {
      auto snap = m->snapshot();
      result.update(snap);
    }
    return result;
  }
};

int main() {

  Engine engine;
  engine.add(std::make_unique<CpuMetric>());
  engine.add(std::make_unique<MemoryMetric>());
  while (true) {
    engine.tick();
    json out = engine.collect();

    std::cout << out.dump(4) << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
