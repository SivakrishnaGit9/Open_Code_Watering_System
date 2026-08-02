# AGENTS.md — Plant Watering System

## Source of truth

- `FSD_V_1_0.md` is the sole authoritative spec (v2.0, design-finalised). All implementation must conform to it.

## Project state

- Design phase **complete**; no firmware or hardware implementation exists yet.
- The repository contains only the functional specification document.

## Key design anchors (from FSD)

| Aspect | Decision |
|---|---|
| MCU | ESP32 (NodeMCU / DevKit) |
| Power | 12V DC wall adapter (2A–3A) — no solar/battery |
| Pump switch | IRLZ44N logic-level MOSFET + 10kΩ pulldown on gate |
| Watering | 60s every 48h (~500ml total via 4× 2 GPH drippers) |
| State persistence | NVS (Preferences API) every 5 min during countdown |
| Debug web server | `#define DEBUG_WEB_SERVER` — disabled in production |
| Strapping pins | GPIO 0, 2, 12, 15 avoided for pump control |
| GPIO map | Pump=25, Float=33, LED=2 |

## Conventions

- No code changes without explicit user approval.
- No modifications to `FSD_V_1_0.md` without prior FSD change approval from the user.
- When implementing firmware: use Arduino framework (de facto for ESP32), non-blocking `millis()` or FreeRTX tasks, and Preferences API for NVS.
