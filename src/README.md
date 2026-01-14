# 📂 Spresence Project Source Code

Welcome to the source code repository for our Sony Spresence evaluation series. This directory is organized by difficulty level to help you progress from basic hardware tests to advanced multicore applications.

---

## 🏗️ Folder Structure

| Category | Description | Key Modules Covered |
| :--- | :--- | :--- |
| **[level-01-basics](./level-01-basics)** | Entry-level sketches. | GPIO, LEDs, Serial Debugging |
| **[level-02-core](./level-02-core)** | Leveraging Sony's unique hardware. | GNSS (GPS), Audio, Camera, SD Card |
| **[level-03-advanced](./level-03-advanced)** | Pro-level implementations. | Multicore (ASMP), Edge AI, DSP |

---

## 🚀 How to Use These Examples

### 1. Prerequisites
Before running any code, ensure you have:
* Installed the **Spresence Board Manager** in Arduino IDE.
* Set the **Core Selection** to `MainCore` (unless specified in the project folder).
* Connected your board via the **Main USB** port.

### 2. Opening a Project
Arduino IDE requires the folder name to match the `.ino` file. 
To open the GPS Tracker:
1. Navigate to `src/level-02-core/01-gps-tracker/`.
2. Open `01-gps-tracker.ino`.

### 3. Logic Voltage Warning ⚠️
> **STOP:** Always check the `README.md` inside each project folder before wiring. Most projects in Level 02 and 03 require the **Extension Board** set to **3.3V**. Connecting 3.3V or 5V sensors directly to the Main Board will damage it.

---

## 📥 Downloading the Code

If you are comfortable with Git, clone this repository to your Ubuntu machine:
```bash
cd ~/Arduino/Projects
git clone [https://github.com/baqwas/spresence.git](https://github.com/baqwas/spresence.git)

