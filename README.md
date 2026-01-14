# 🛰️ Sony Spresense Evaluation & Project Hub

[![Project Kanban](https://img.shields.io/badge/Project-Kanban_Board-blueviolet?style=flat-square&logo=github)](https://github.com/baqwas/spresence/projects)
[![Wiki](https://img.shields.io/badge/Documentation-Wiki-brightgreen?style=flat-square&logo=wikipedia)](https://github.com/baqwas/spresence/wiki)
[![YouTube](https://img.shields.io/badge/YouTube-Watch_Tutorials-red?style=flat-square&logo=youtube)](https://www.youtube.com/@baqwas)

Welcome to the **Sony Spresense** development repository. This project is a comprehensive evaluation of the Spresence ecosystem, transitioning from beginner "Hello World" basics to intermediate multicore AI and DSP solutions.

---

## 📺 Project Context
This repository accompanies the Spresense series on the **[Baqwas YouTube Channel]**. We explore the unique capabilities of the Sony CXD5602, including:
* **6-Core ARM® Cortex®-M4F** architecture.
* **Integrated GNSS** (GPS/GLONASS).
* **High-Resolution Audio** (192kHz/24-bit).
* **Edge AI** with camera and microphone arrays.

---

## 📂 Repository Structure

| Directory | Description |
| :--- | :--- |
| **[`/src`](./src)** | All source code and sketches, organized by difficulty level. |
| **[`/docs`](./docs)** | Hardware datasheets, pinout diagrams, and local documentation. |
| **[`/tools`](./tools)** | Helper scripts for Ubuntu/Linux developers. |

---

## 🛠️ Quick Start (Ubuntu/Linux)

### 1. Permissions
Before uploading code, ensure your user has access to the serial port:
```bash
sudo usermod -a -G dialout $USER
>>>>>>> 69282e1 (Initial commit: Repository structure, Wiki assets, and READMEs)
