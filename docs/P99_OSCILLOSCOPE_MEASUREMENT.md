# P99 Latency Measurement with Oscilloscope - Visual Guide

## What is P99 (99th Percentile)?

**P99 latency** = The latency value below which **99% of measurements fall**.

**Example:**
- You send 1000 messages
- Sort latencies: [1ms, 1ms, 1.2ms, ..., 5ms, 8ms, 150ms]
- P99 = 990th value (99% of 1000)
- If P99 = 5ms → **99% of messages took ≤ 5ms, 1% took longer**

### Why P99 Matters More Than Average

```
Scenario: 1000 messages sent

Latencies:
- 990 messages: 1-2 ms (fast path)
- 10 messages: 100-200 ms (slow path - maybe GC pause, network retransmit)

Average: (990 × 1.5ms + 10 × 150ms) / 1000 = 2.985ms
P99: 5ms
P99.9: 150ms

User experience:
- Average says "3ms - looks great!"
- P99 says "5ms - still pretty good"
- P99.9 says "150ms - uh oh, users notice lag!"
```

**Interview Answer:**
> "P99 is critical because averages hide outliers. In production, that 1% of slow requests might be your most important customers or during peak load. At Google scale, 1% of 100 million requests = 1 million angry users!"

---

## Measuring P99 with Oscilloscope

### Why Oscilloscope for PCB/Embedded?

**Software logging is too slow and invasive!**
- `printf()` takes ~1ms → distorts measurements
- Logging affects cache, interrupts
- **Solution**: Use GPIO pins + oscilloscope

### Setup Diagram

```
┌────────────────────────────────────────────────────────────┐
│                    Microcontroller (STM32)                  │
│                                                             │
│  ┌──────────────────────────────────────────────────┐     │
│  │  main() {                                         │     │
│  │    while(1) {                                     │     │
│  │      // Measure: Sensor Read → Serialize → Send  │     │
│  │                                                   │     │
│  │      GPIO_SET_HIGH(PIN_PA0);   // ───┐           │     │
│  │      sensor_read();             //    │           │     │
│  │      protobuf_encode();         //    │ Measure   │     │
│  │      uart_send();               //    │ Time      │     │
│  │      GPIO_SET_LOW(PIN_PA0);    // ───┘           │     │
│  │                                                   │     │
│  │      delay(100ms);                                │     │
│  │    }                                              │     │
│  │  }                                                │     │
│  └──────────┬───────────────────────────────────────┘     │
│             │                                              │
│             │ PA0 (GPIO pin)                              │
└─────────────┼──────────────────────────────────────────────┘
              │
              │ Probe connected here
              ▼
       ┌──────────────┐
       │ Oscilloscope │ ← Captures 1000 pulses
       │              │
       │  [Measure]   │
       │  Statistics  │
       └──────────────┘
```

### Oscilloscope Waveform

```
PIN_PA0 signal:

     ┌────┐           ┌────┐         ┌────┐         ┌──────┐
     │    │           │    │         │    │         │      │  ← Outlier!
─────┘    └───────────┘    └─────────┘    └─────────┘      └─────
     
     │<─>│           │<─>│         │<─>│         │<────>│
     1.5ms           1.8ms         2.0ms          6.5ms
     
Capture 1000 pulses → Oscilloscope statistics:
  Min:   1.2 ms
  Max:   150 ms  (anomaly - maybe interrupt fired)
  Mean:  1.8 ms
  Std:   2.5 ms
  P99:   5.0 ms  ← 99th percentile
```

### Oscilloscope Setup (Tektronix/Rigol/Siglent)

```
1. Connect probe to GPIO pin (PA0)
2. Set trigger: Rising edge, single shot
3. Set timebase: 10ms/div (to see full pulse)
4. Capture mode: Statistics
5. Run 1000 acquisitions
6. Export CSV: pulse_widths.csv

CSV Output:
Measurement, Width
1,          1.523 ms
2,          1.678 ms
3,          1.892 ms
...
999,        2.123 ms
1000,       1.567 ms
```

---

## Python Analysis Script

```python
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Load oscilloscope CSV
df = pd.read_csv('pulse_widths.csv')
latencies = df['Width'].values * 1000  # Convert to microseconds

# Calculate percentiles
p50 = np.percentile(latencies, 50)   # Median
p95 = np.percentile(latencies, 95)
p99 = np.percentile(latencies, 99)
p99_9 = np.percentile(latencies, 99.9)

print(f"P50:   {p50:.2f} μs")
print(f"P95:   {p95:.2f} μs")
print(f"P99:   {p99:.2f} μs")
print(f"P99.9: {p99_9:.2f} μs")

# Plot histogram
plt.hist(latencies, bins=50, alpha=0.7, label='Latency Distribution')
plt.axvline(p99, color='r', linestyle='--', label=f'P99: {p99:.2f}μs')
plt.xlabel('Latency (μs)')
plt.ylabel('Count')
plt.title('Telemetry Latency Distribution (Oscilloscope Measurement)')
plt.legend()
plt.savefig('p99_latency.png')
```

---

## Real Example: JSON vs Protobuf Measurement

### Test Setup
```
Microcontroller: STM32F407 @ 168 MHz
Sensor: TMP117 (I2C temperature sensor)
UART: 115200 baud

Test: Read sensor → Serialize → Send over UART
Iterations: 1000
```

### Results (Oscilloscope Capture)

```
┌─────────────┬──────────┬──────────┬─────────┐
│ Metric      │ JSON     │ Protobuf │ Improve │
├─────────────┼──────────┼──────────┼─────────┤
│ P50 (median)│ 3.2 ms   │ 2.1 ms   │ 34%     │
│ P95         │ 4.8 ms   │ 2.8 ms   │ 42%     │
│ P99         │ 6.5 ms   │ 3.2 ms   │ 51%     │  ← Key metric!
│ P99.9       │ 150 ms   │ 145 ms   │ 3%      │  ← Outliers same (interrupt latency)
│ Max         │ 180 ms   │ 175 ms   │ 3%      │
└─────────────┴──────────┴──────────┴─────────┘

Interpretation:
- P99 improved 51%: Most requests faster with Protobuf
- P99.9 similar: Outliers dominated by system interrupts, not serialization
```

### Waveform Comparison

```
JSON (60 bytes → 4.2ms UART time + 1ms encode = ~5ms typical)
     ┌────────────┐      ┌────────────┐      ┌────────────┐
─────┘            └──────┘            └──────┘            └─────
     │<────5ms──>│      │<────5ms──>│      │<────5ms──>│

Protobuf (22 bytes → 1.5ms UART time + 0.3ms encode = ~2ms typical)
     ┌─────┐            ┌─────┐            ┌─────┐
─────┘     └────────────┘     └────────────┘     └────────────
     │<2ms>│            │<2ms>│            │<2ms>│

P99 tells us: "In worst case (excluding rare interrupts), Protobuf is 3.2ms vs JSON's 6.5ms"
```

---

## Why Oscilloscope vs Software Timers?

### Software Timer Problems

```cpp
// ❌ BAD: Software timer (introduces overhead)
uint32_t start = HAL_GetTick();  // ← Takes ~500 CPU cycles
sensor_read();
protobuf_encode();
uart_send();
uint32_t end = HAL_GetTick();    // ← Takes ~500 CPU cycles
printf("Latency: %lu ms", end - start);  // ← Takes ~10,000 CPU cycles!
```

**Problems:**
1. **Timer resolution**: HAL_GetTick() often 1ms resolution (too coarse)
2. **Logging overhead**: `printf()` takes 1-2ms → distorts measurement
3. **Cache effects**: Timing code pollutes cache
4. **Heisenberg principle**: Measurement changes behavior

### Oscilloscope Advantages

```cpp
// ✅ GOOD: GPIO toggle (< 10 CPU cycles, ~60 ns)
GPIO_SET_HIGH(PIN_PA0);  // ← Instant, no overhead
sensor_read();
protobuf_encode();
uart_send();
GPIO_SET_LOW(PIN_PA0);   // ← Instant, no overhead
// No printf, no logging!
```

**Advantages:**
1. **Non-invasive**: GPIO toggle takes ~60 ns (negligible)
2. **High resolution**: Oscilloscope has ns precision
3. **No code changes**: Just add 2 lines
4. **Statistical analysis**: Oscilloscope built-in percentile calculations

---

## Alternative: Logic Analyzer

For **digital protocols** (I2C, SPI, UART), use **logic analyzer**:

```
┌─────────────────────────────────────────────────┐
│          Logic Analyzer (Saleae, DSLogic)       │
│                                                  │
│  Channel 0: UART TX  ───┐                       │
│  Channel 1: UART RX  ───┼─ Decode               │
│  Channel 2: I2C SCL  ───┤  Protocol             │
│  Channel 3: I2C SDA  ───┘                       │
│                                                  │
│  [Timing Statistics]                            │
│  UART packet time: 1.523 ms (JSON)              │
│  UART packet time: 0.556 ms (Protobuf)          │
└─────────────────────────────────────────────────┘
```

**Logic analyzer** decodes protocols automatically:
- See actual UART bytes
- Measure packet times
- Cheaper than oscilloscope ($50 vs $2000)

---

## Interview Story

**Question: "How did you measure Protobuf performance improvement?"**

> "I used an oscilloscope to measure end-to-end latency non-invasively. I toggled a GPIO pin before sensor read and after UART transmission. The oscilloscope captured 1000 samples and calculated percentiles automatically.
>
> **Results:**
> - **P99 latency**: 6.5ms (JSON) → 3.2ms (Protobuf) = **51% improvement**
> - **P50 latency**: 3.2ms (JSON) → 2.1ms (Protobuf) = **34% improvement**
>
> The oscilloscope was critical because software timers would add 1-2ms overhead from `printf()`, distorting the measurements. GPIO toggling takes only 60 nanoseconds—completely non-invasive.
>
> This validated our migration: Protobuf not only reduced message size (62%) but also improved worst-case latency by 50%, meeting our real-time control requirements for the PCB sensor system."

---

## Tools Summary

| Tool | Use Case | Precision | Cost |
|------|----------|-----------|------|
| **Oscilloscope** | Analog signals, timing | 1 ns | $500-$5000 |
| **Logic Analyzer** | Digital protocols (I2C, SPI, UART) | 10 ns | $50-$500 |
| **Software Timer** | High-level profiling | 1 ms | Free |
| **GPIO + Scope** | Non-invasive latency | 60 ns | $500+ |

**Pro Tip:** For embedded work, a $100 logic analyzer (Saleae Logic 8) is more useful than a $2000 oscilloscope for most debugging tasks!
