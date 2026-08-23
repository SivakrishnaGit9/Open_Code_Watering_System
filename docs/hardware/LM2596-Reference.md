# LM2596 12V-to-5V Step-Down Buck Converter Reference

## Overview
The LM2596S DC-DC step-down (buck) switching regulator steps down the incoming 12V DC wall adapter power to a stable 5V DC supply to power the ESP32 NodeMCU logic board.

## Pinout & Connections
| Pin | Connection | Description |
| :--- | :--- | :--- |
| **IN+** | 12V DC Supply (+) | Connected to 12V wall power adapter |
| **IN-** | System Ground (-) | Connected to Common Ground |
| **OUT+** | 5V Regulated Output (+) | Connected to ESP32 VIN / 5V pin |
| **OUT-** | System Ground (-) | Connected to Common Ground |

## Specifications & Electrical Limits
- **Input Voltage Range:** 4.5V to 40V DC (operated at 12V DC in this project)
- **Output Voltage:** Regulated 5V DC
- **Maximum Output Current:** Up to 3A
- **Switching Frequency:** 150 kHz
