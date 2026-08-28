# ESP32 External Flashing Reference Guide

This document outlines the procedure and memory offsets required to flash compiled binaries onto the ESP-WROOM-32D using external tools (such as Espressif Flash Download Tool or `esptool.py`) when direct USB flashing from the development container is unavailable.

## 1. Required Binaries
When compiling the project via PlatformIO, the following build artifacts are generated under `.pio/build/esp32dev/`:
- **Bootloader:** `bootloader.bin`
- **Partition Table:** `partitions.bin`
- **Application Firmware:** `firmware.bin`

## 2. Flash Address Map (ESP32 / 4MB Flash)

| Component | Flash Address | Path in Workspace |
| :--- | :--- | :--- |
| **Bootloader** | `0x1000` | `.pio/build/esp32dev/bootloader.bin` |
| **Partition Table** | `0x8000` | `.pio/build/esp32dev/partitions.bin` |
| **Application** | `0x10000` | `.pio/build/esp32dev/firmware.bin` |

## 3. Flashing via `esptool.py` (Command Line)

```bash
esptool.py --chip esp32 --port <PORT> --baud 921600 \
  --before default_reset --after hard_reset \
  write_flash -z \
  --flash_mode dio --flash_freq 40m --flash_size 4mb \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

## 4. Flashing via Espressif Flash Download Tool (GUI)
1. Launch **ESP32 DownloadTool**.
2. Select the three binary paths and assign their respective addresses (`0x1000`, `0x8000`, `0x10000`).
3. Set SPI Speed to `40MHz`, SPI Mode to `DIO`, and Flash Size to `32Mbit`.
4. Select the target COM port and click **START**.
