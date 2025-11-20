# XP12AutoPause

![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)
![X-Plane](https://img.shields.io/badge/Simulator-X--Plane%2012-orange)
![License](https://img.shields.io/badge/License-MIT-green)

**XP12AutoPause** is a high-performance, native C++ plugin for X-Plane 12. It automatically pauses the simulator when your aircraft reaches a specific distance (radius) from a target coordinate.

This tool is essential for long-haul flights, allowing you to step away from the computer while ensuring you never miss your Top of Descent (TOD) or a critical waypoint.

---

## ✨ Features

* **🚀 Native Performance:** Written in C++ using the X-Plane SDK. Zero impact on FPS (no background scripts).
* **🖥️ Modern UI:** Features a clean, dark-themed interface built with Dear ImGui.
* **🌐 Cross-Platform:** Runs natively on **Windows**, **macOS** (Intel & Apple Silicon), and **Linux**.
* **Background Operation:** You can close the plugin window, and the logic remains active in the background.
* **💾 Persistence:** Automatically saves your target coordinates, radius, and settings for your next flight.

---

## 📥 Installation

### 1. Download
Go to the **[Releases](../../releases)** page and download the latest `XP12AutoPause.zip`.

### 2. Install
1.  Open the downloaded ZIP file.
2.  You will see a folder named **`XP12AutoPause`**.
3.  Drag and drop this **entire folder** into your X-Plane plugins directory:
    
    `X-Plane 12/Resources/plugins/`

**Your folder structure should look like this:**
```text
X-Plane 12/
 └── Resources/
      └── plugins/
           └── XP12AutoPause/       <-- Drop this folder
                ├── win_x64/
                ├── mac_x64/
                └── lin_x64/

```            
##🍎 macOS Users (Important)
Since this plugin is not notarized by Apple (because I am an independent developer), macOS will likely block it for security reasons upon first launch.
To allow the plugin to run:
1. Open your Terminal app.
2. Copy and paste the following command (adjust the path if your X-Plane is installed elsewhere):
```xattr -cr "~/X-Plane 12/Resources/plugins/XP12AutoPause"```
(If you installed X-Plane in a different location, replace ~/X-Plane 12 with your actual path).

##📖 How to Use
* **Open: Start X-Plane and go to the top menu: Plugins -> XP12AutoPause -> Toggle Window.
* **Set Target: Enter the Latitude and Longitude of your destination or Top of Descent (TOD).
* **Tip: You can copy these coordinates directly from SimBrief or map tools.
* **Set Radius: Enter the distance in Nautical Miles (NM) at which you want the pause to trigger (e.g., 150 for TOD).
* **Activate: Click the "Set & Activate" button.
* **Fly: You can now close the window using the "X" or the menu. The plugin will monitor your distance in the background and pause the simulator automatically when you reach the radius.
