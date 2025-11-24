# ESP32 Drain Maintenance & Flow Monitoring System

This project implements a real-time drain monitoring system using an ESP32.  
It measures flow rate from a Hall-effect flow sensor, tracks drainage conditions,
and publishes sensor data using MQTT for downstream analytics and automation.

The repository includes two firmware variants:
1. **Standard Arduino loop-based implementation**
2. **Modular FreeRTOS implementation** with dedicated system tasks


##  Prototype Hardware

<img src="Prototype/project_prototype.jpg" width="350">

---

## 🚀 Features

- Clean separation of logic (FreeRTOS tasks, modular `.cpp/.h`)  
- Expandable sensor architecture for future water-level,water flow or gas sensor modules  
- Designed for low-power IoT environments  
- Allows benchmarking of:
  

---

## Firmware Versions

- **Standard Arduino Version:**  
  [drain_maintenance_system_firmware.ino](code/drain_maintenance_system_firmware.ino)

- **FreeRTOS Version:**  
  [main.ino](code/FreeRTOS_Version/main.ino)



## Next Steps
- Add database logging
- Add OTA updates
