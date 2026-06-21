# Symon

A lightweight Linux system monitoring engine written in modern C++.

SysMon collects system metrics directly from the Linux `/proc` filesystem and exposes them as structured JSON output. The project is focused on learning low-level Linux internals, systems programming, performance monitoring, and clean software architecture.

---

## Features

### CPU Monitoring

Reads CPU statistics from:

```text
/proc/stat
```

Calculates:

- Total CPU utilization
- Per-core CPU utilization
- Delta-based sampling between consecutive snapshots

Formula:

```text
CPU Usage = (Total Delta - Idle Delta) / Total Delta * 100
```

---

### Memory Monitoring

Reads memory information from:

```text
/proc/meminfo
```

Uses:

- MemTotal
- MemFree
- Buffers
- Cached

Calculation:

```text
Used Memory =
MemTotal - MemFree - Buffers - Cached
```

Outputs:

- Total memory (MB)
- Used memory (MB)

---

### JSON Output

Example:

```json
{
    "cpu": {
        "total": 4.00,
        "cores": [
            3.0,
            5.0,
            8.0,
            4.0,
            5.0,
            3.0,
            2.0,
            5.0
        ]
    },
    "mem_total": 7779,
    "mem_used": 2234
}
```

---

## Architecture

The project uses an abstraction-based metric system.

### Metric Interface

```cpp
class Metric {
public:
    virtual ~Metric() = default;

    virtual void update() = 0;
    virtual json snapshot() const = 0;
};
```

Every metric must:

1. Collect data
2. Update internal state
3. Produce a JSON snapshot

---

### Current Metrics

#### CpuMetric

Responsible for:

- Reading `/proc/stat`
- Maintaining previous and current snapshots
- Computing utilization percentages

State:

```text
prev CPU snapshot
curr CPU snapshot
usage statistics
```

---

#### MemoryMetric

Responsible for:

- Reading `/proc/meminfo`
- Computing used memory
- Producing memory statistics

---

### Engine

The engine orchestrates all metrics.

Responsibilities:

```text
tick()
 ├─ update CPU metric
 ├─ update Memory metric
 └─ update future metrics
```

```text
collect()
 ├─ gather snapshots
 └─ merge into JSON output
```

Example:

```cpp
Engine engine;

engine.add(std::make_unique<CpuMetric>());
engine.add(std::make_unique<MemoryMetric>());
```

The engine knows nothing about CPU or memory internals.

It only knows how to manage Metrics.

---

## Design Concepts Used

### Abstraction

Metrics expose:

```cpp
update()
snapshot()
```

without exposing implementation details.

The engine does not know:

- How CPU usage is calculated
- How memory is parsed
- Where data comes from

---

### Encapsulation

Each metric owns its internal state.

Examples:

```cpp
CPUSet prev;
CPUSet curr;
```

are hidden inside `CpuMetric`.

No external code can modify them directly.

---

### Stateful Sampling

CPU utilization requires two samples.

Workflow:

```text
Tick 0
 └─ Store initial snapshot

Tick 1
 └─ Compute delta

Tick 2+
 └─ Continue sliding window calculations
```

---

## Current Project Structure

```text
sysmon/
│
├── src/
│   └── main.cpp
│
├── external/
│   └── nlohmann/
│       └── json.hpp
│
└── build.sh
```

---

## Build

Requirements:

- Linux
- g++
- C++20

Build:

```bash
./build.sh
```

Run:

```bash
./sysmon
```

---

## Why This Project Exists

Goals:

- Learn Linux internals
- Understand `/proc`
- Practice modern C++
- Explore systems programming concepts
- Build monitoring software from scratch
- Learn software architecture through real projects

---

# Roadmap

## Phase 1 — Core Engine ✅

- [x] CPU monitoring
- [x] Memory monitoring
- [x] JSON output
- [x] Metric abstraction
- [x] Engine orchestration
- [x] Per-core CPU usage

---

## Phase 2 — Stability Improvements

- [ ] Metric readiness API
- [ ] CPU hotplug protection
- [ ] Better error handling
- [ ] Unit tests
- [ ] Logging system
- [ ] Configuration support

---

## Phase 3 — Additional Metrics

### Disk

- [ ] Disk usage
- [ ] Disk I/O throughput
- [ ] Read/write statistics

Sources:

```text
/proc/diskstats
```

---

### Network

- [ ] Upload speed
- [ ] Download speed
- [ ] Interface statistics

Sources:

```text
/proc/net/dev
```

---

### Processes

- [ ] Process count
- [ ] Top CPU consumers
- [ ] Top memory consumers

Sources:

```text
/proc/<pid>
```

---

### System Information

- [ ] Uptime
- [ ] Load average
- [ ] Kernel version
- [ ] Host information

Sources:

```text
/proc/uptime
/proc/loadavg
```

---

## Phase 4 — Exporters

### HTTP API

```text
GET /metrics
```

Returns JSON metrics.

---

### Prometheus Exporter

Expose metrics in Prometheus format.

Example:

```text
sysmon_cpu_usage 5.2
sysmon_memory_used_mb 2234
```

---

### WebSocket Streaming

Real-time metric streaming.

---

## Phase 5 — User Interfaces

### CLI Dashboard

```text
CPU: 5.3%
MEM: 2234 / 7779 MB
```

---

### TUI Dashboard

Inspired by:

- btop
- htop

Potential libraries:

- FTXUI
- ncurses

---

### Desktop GUI

Future exploration:

- Qt
- Dear ImGui

---

## Phase 6 — Advanced Features

### Historical Storage

- [ ] Metric persistence
- [ ] Time-series storage
- [ ] Metric replay

---

### Alerts

Examples:

```text
CPU > 90%
Memory > 80%
Disk Full
```

---

### Plugin System

Allow loading metrics dynamically.

Example:

```text
plugins/
 ├─ gpu.so
 ├─ battery.so
 └─ sensors.so
```

---

## Long-Term Vision

SysMon aims to evolve from a simple monitoring utility into a modular monitoring engine capable of:

- Collecting system metrics
- Exporting metrics
- Driving dashboards
- Supporting plugins
- Serving as a learning platform for systems programming

---



