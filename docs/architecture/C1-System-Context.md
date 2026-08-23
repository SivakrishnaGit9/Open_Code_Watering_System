# C1: System Context - Automated Plant Watering System

## Overview
The C1 System Context diagram outlines the highest-level view of the Plant Watering System, showing its interaction with external entities (Users, Water Bucket Reservoir, Plant Pots, and Mains Power).

```mermaid
graph TD
    user["User / Plant Owner<br/><i>(Sets up system & fills reservoir)</i>"]
    wateringSystem["<b>Plant Watering System</b><br/><i>Autonomous 12V ESP32 Controller</i>"]
    powerSource["12V DC Wall Adapter<br/><i>FEDUS 12V 3A SMPS</i>"]
    waterSource["Water Reservoir<br/><i>Bucket Source</i>"]
    plants["4 Plant Pots<br/><i>2 GPH Drippers</i>"]

    user -->|Configures & Refills| wateringSystem
    powerSource -->|Supplies 12V DC| wateringSystem
    waterSource -->|Provides Water| wateringSystem
    wateringSystem -->|Delivers ~500ml every 48h| plants
```

## Boundaries & Relationships
- **User:** Interacts during setup and occasional reservoir refills.
- **Power Source:** Mains-powered 12V 3A adapter ensuring 100% uptime reliability.
- **Water Reservoir:** Static bucket source equipped with low-level monitoring or visual checks.
- **Plant Pots:** 4 target plants irrigated equally via pressure-compensating drippers and check valves.
