# Baud Rate and UART Communication - Visual Guide

## What is Baud Rate?

**Baud Rate** = **Signal changes per second** (symbols/sec)

For UART (serial communication): **1 baud ≈ 1 bit per second**

### Common Baud Rates
```
9600 baud   = 9,600 bits/sec   = 1,200 bytes/sec   (old modems, GPS)
115200 baud = 115,200 bits/sec = 14,400 bytes/sec  (common for PCB/microcontrollers)
921600 baud = 921,600 bits/sec = 115,200 bytes/sec (high-speed UART)
```

---

## Visual: UART Frame Structure

```
Start Bit    Data Bits (8)           Parity  Stop Bit
    ↓        ↓                       ↓       ↓
    ___      ___      ___      ___   ___     ___________
   |   |____|   |____|   |____|   |_|   |___|           |
   0    1    0    1    1    0    1   P   1              (Idle high)
   
   <------------ 10 bits total per byte ------------>
```

**Example at 115200 baud:**
- Bits per second: 115,200
- Bits per byte: 10 (1 start + 8 data + 1 stop)
- **Bytes per second: 115,200 / 10 = 11,520 bytes/sec** (theoretical max)
- **Actual throughput: ~14,400 bytes/sec** (with no parity, optimizations)

---

## Visual: Protobuf vs JSON on UART

### Scenario: PCB Sensor → Software (115200 baud UART)

```
┌─────────────────┐
│  PCB Sensor     │  Temperature: 23.5°C
│  (Cortex-M4)    │  Sequence: 12345
└────────┬────────┘
         │ UART 115200 baud
         │
         ▼
    [Serialize]
         │
    ┌────┴──────────────────────────┐
    │                                │
    │ JSON (60 bytes):               │  Protobuf (22 bytes):
    │ {"seq":12345,"value":23.5,...} │  [binary: 08 39 30 11...]
    │                                │
    │ 60 bytes × 8 bits = 480 bits   │  22 bytes × 8 bits = 176 bits
    │ + 10 bits overhead = 490 bits  │  + 10 bits overhead = 186 bits
    │                                │
    │ Time: 490/115200 = 4.25 ms     │  Time: 186/115200 = 1.61 ms
    │ Max rate: 235 msg/sec          │  Max rate: 621 msg/sec
    └────┬──────────────────────────┬┘
         │                          │
         ▼                          ▼
    ┌────────────────────────────────┐
    │   Software (TelemetryHub)      │
    └────────────────────────────────┘
```

**Calculation:**
```
Baudrate = 115,200 bits/sec
Bytes/sec = 115,200 / 10 ≈ 11,520 bytes/sec (accounting for start/stop bits)

JSON Message:
- Size: 60 bytes
- Messages/sec: 11,520 / 60 = 192 msg/sec (realistic)
- Theoretical max: 235 msg/sec (no overhead)

Protobuf Message:
- Size: 22 bytes
- Messages/sec: 11,520 / 22 = 523 msg/sec (realistic)
- Theoretical max: 621 msg/sec (no overhead)

Improvement: 523 / 192 = 2.7x more throughput!
```

---

## Real-World Example: Your PCB Experience

```
┌──────────────────────────────────────────────────────────────┐
│                     PCB Temperature Sensor                    │
│                                                               │
│  ┌──────────┐   I2C    ┌──────────────┐   UART (115200)     │
│  │ TMP117   │ ───────> │  STM32F4     │ ──────────────┐     │
│  │ Sensor   │          │  (Cortex-M4) │               │     │
│  └──────────┘          └──────────────┘               │     │
│                             ↓                          │     │
│                        [Protobuf Encode]              │     │
│                             ↓                          ▼     │
│                        22 bytes/msg          ┌──────────────┐│
│                                              │   USB-UART   ││
└──────────────────────────────────────────────│   Adapter    ││
                                               └──────┬───────┘│
                                                      │         │
                                                      ▼         │
                                          ┌──────────────────┐ │
                                          │  PC Software     │ │
                                          │  (TelemetryHub)  │ │
                                          │  [Protobuf Parse]│ │
                                          └──────────────────┘ │
```

**Why 115200 baud?**
- **Standard rate**: Supported by most microcontrollers
- **Good balance**: Fast enough for telemetry, slow enough to be reliable
- **No external clock**: UART doesn't need separate clock line (unlike SPI/I2C)
- **Long cables**: Works up to ~15 meters with proper voltage levels

**Higher baud rates exist:**
- 230400, 460800, 921600 baud
- But: Higher error rates with noise, cable length, clock drift

---

## Bandwidth Calculation Cheat Sheet

```
Baud Rate    → Bytes/sec     → JSON (60B)   → Protobuf (22B)
─────────────────────────────────────────────────────────────
9600 baud    → 960 bytes/s   → 16 msg/s     → 43 msg/s
57600 baud   → 5,760 bytes/s → 96 msg/s     → 261 msg/s
115200 baud  → 11,520 bytes/s→ 192 msg/s    → 523 msg/s
230400 baud  → 23,040 bytes/s→ 384 msg/s    → 1,047 msg/s
921600 baud  → 92,160 bytes/s→ 1,536 msg/s  → 4,189 msg/s
```

**Formula:**
```
Bytes/sec = (Baud Rate / 10)
Messages/sec = Bytes/sec / Message_Size
```

---

## Why 10 bits per byte?

```
UART Frame:
  1 Start bit     (always 0)
+ 8 Data bits     (your actual byte)
+ 1 Stop bit      (always 1)
─────────────────
= 10 bits total

Optional: Parity bit (11 bits total)
```

**Example: Sending 'A' (ASCII 0x41 = 0b01000001):**
```
Idle   Start  D0  D1  D2  D3  D4  D5  D6  D7  Stop  Idle
 ___    ___   ___         ___             ___  _____
    |__|   |_|   |_______|   |___________|   |
  1   0   1  0   0   0   0   0   1   1   1   1
      ↑   ↑                               ↑
    Start Data: 0b01000001 = 'A'        Stop
```

---

## Terminology

- **Baud Rate**: Symbols per second (for UART: bits per second)
- **Bit Rate**: Bits per second (same as baud for simple UART)
- **Throughput**: Actual data bytes per second (excludes start/stop bits)
- **Latency**: Time to send one message

**Interview Tip:**
> "In my PCB project, we used 115200 baud UART for sensor communication. That's 11,520 bytes/sec throughput after accounting for start/stop bits. With JSON at 60 bytes/message, we maxed out at 192 msg/sec. Protobuf reduced message size to 22 bytes, giving us 523 msg/sec—a **2.7x improvement** that let us increase sensor sampling from 4 Hz to 10 Hz for real-time control."

---

## Further Reading
- **RS-232/UART**: Standard serial protocol for microcontrollers
- **USB-UART bridge**: FT232, CP2102 chips (PC ↔ microcontroller)
- **Voltage levels**: TTL (0-5V) vs RS-232 (±12V)
