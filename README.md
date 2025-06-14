# BLISS: Blinking Low-Power Infrared Sensing System

> A wearable eye health monitoring device to help prevent Computer Vision Syndrome and fatigue-related incidents through real-time blink detection and feedback.

---

## 👁️ Overview

**BLISS** (Blinking Low-power Infrared Sensing System) is a smart wearable device that monitors a user’s blinking behavior using infrared sensing and provides real-time alerts via vibration when signs of fatigue or CVS are detected.

It is:

* Low-power and lightweight
* BLE-enabled
* > 90% accurate in static use cases (e.g., reading, driving)
* Packaged in a compact 3D-printed form

---

## 🧑‍💻 Team

| Name               | Role                           |
| ------------------ | ------------------------------ |
| Taisuke Miyamoto   | Algorithm and App Development  |
| Tergel Molom-Ochir | Hardware & Logistics           |
| Sashank Rao        | Embedded Systems, BLE, Finance |
| Heta Shah          | PCB Design                     |
| **Advisor**        | Prof. Qiangfei Xia             |

---

## ❓ Problem Statement

* 60M+ Americans suffer from chronic sleep deprivation
* 81.9% of engineering students experience Computer Vision Syndrome (CVS)
* \~700 drowsy-driving deaths occurred in 2020
* Lack of discreet, low-cost, user-friendly solutions to address blink-related fatigue

---

## 🎯 Goals

* Detect user blinks in real-time
* Measure:

  * Blink frequency (±1 blink/min)
  * Blink duration (±0.1s)
* Alert users if:

  * Blink frequency < 12 blinks/min
  * Blink duration > 0.5s
* Send alerts via vibration and BLE smartphone notifications

---

## ⚙️ System Features

* **BLE connectivity** for mobile app integration
* **Onboard blink detection** with IR sensor
* **Real-time alerting** with haptic feedback
* **Compact form factor** using custom PCB and 3D-printed housing
* **Safe IR emission** (720nm compliant)

---

## 🧪 Testing Results

| Condition            | Actual Blinks | Detected Blinks | Accuracy |
| -------------------- | ------------- | --------------- | -------- |
| Stationary Head      | 34            | 37              | 91%      |
| Constant Head Motion | 24            | 28              | 67%      |

➡️ Stationary use cases (e.g., working on a computer or highway driving) are ideal for device use.

---

## 🧱 Hardware Components

* **IR Sensor** (720nm, safe & effective)

  * \$2.93/unit, 100mA forward current
* **Adafruit Feather M0 Basic Proto**

  * ATSAMD21 Cortex-M0
  * BLE via NRF51822
  * USB charging
  * \$29.95/unit
* **LiPo Battery**
* **Custom PCB**
* **3D Printed Enclosure**

---

## 🧠 Software Architecture

* IR data sampling @ 75 Hz
* Data smoothing and moving average
* Blink detection using:

  * Signal slope thresholding
  * STD-weighted comparison
* BLE data updates sent once/second

---

## 📱 App Features

* Displays blink frequency and duration
* Triggers vibration alerts on low frequency or long duration
* (Planned) Reset functionality from app

---

## 📦 Deliverables

* ✅ Functional prototype of smart glasses
* ✅ Final PCB with 3D-printed enclosure
* ✅ BLE-enabled mobile app
* ✅ >90% accuracy in controlled conditions

---

## 🔄 Future Work

* Add full BLE app reset functionality
* Improve app UI
* Further miniaturize the hardware
* Reduce component costs

---

## 💰 Budget Summary

| Item            | Cost     |
| --------------- | -------- |
| MCU             | \$29.99  |
| IR Sensors      | \$3.88   |
| PCB             | \$5.13   |
| Battery         | \$7.95   |
| **Total Spent** | \$421.14 |
| **Remaining**   | \$78.86  |

---

## 🔐 Safety

* IR emission wavelength: **720 nm**
* Verified safe under guidance from Radiation Safety Officer

---

## 📚 References

Key research topics include:

* Blink frequency and duration analysis
* IR safety standards
* Computer Vision Syndrome epidemiology
* Fatigue detection via eye behavior

📖 Full report available here: [FPR Report](./BLISS/FPR%20Report.docx)

---

## 🙏 Acknowledgments

We thank:

* **Professor Qiangfei Xia** for guidance and support
* **UMass ECE Department** for project resources and infrastructure

---

## 📸 Demo & Images

![image](https://github.com/user-attachments/assets/4448f57e-00b9-4159-9d12-2c8b4236c0ef)

*Labeled view showing sensor, PCB, battery, and wiring inside glasses arm*

![image](https://github.com/user-attachments/assets/352694e2-e2be-456b-a589-4dce9e23ec12)

*Final Product with 3D printed frame

> Live demo of system can be viewed in the Team4-FPR-Official.pptx file.

---

## 📝 License

This project is for academic demonstration purposes. Contact the team for reuse or collaboration inquiries.


