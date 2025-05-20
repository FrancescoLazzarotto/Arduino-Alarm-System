# Arduino-Alarm-System
Arduino-based security alarm system with keypad PIN entry, LCD display, ultrasonic motion detection, and PIN change functionality.

# Arduino PIN-Protected Alarm System with Motion Detection

An Arduino project implementing a security alarm system featuring PIN code access via a 4x4 keypad, status updates on a 16x2 LCD, motion detection using an HC-SR04 ultrasonic sensor, and audible/visual alerts. Users can arm/disarm the system and change the PIN.

## Table of Contents

* [Features](#features)
* [Hardware Requirements](#hardware-requirements)
* [Software Dependencies](#software-dependencies)
* [Schematic/Wiring](#schematicwiring)
* [Installation & Setup](#installation--setup)
* [How to Use](#how-to-use)


## Features

* **PIN Code Access:** Secure the system with a 6-digit PIN.
* **LCD Display:** 16x2 LCD for user interaction and status updates (e.g., "Enter Pin", "Alarm Active", "Motion Detected").
* **Keypad Input:** A 4x4 matrix keypad for PIN entry and command input.
* **Motion Detection:** Utilizes an HC-SR04 ultrasonic sensor to detect movement when the alarm is armed.
* **Audible Alarm:** A piezo buzzer sounds when motion is detected or for feedback.
* **Visual Alarm Indicator:** An LED indicates when the system is armed or an alarm is triggered.
* **PIN Change Functionality:** Allows the user to change the access PIN after authenticating with the current PIN.
* **Input Masking:** PIN entries are displayed as '*' on the LCD for privacy.

## Hardware Requirements

* Arduino Board (e.g., Uno, Nano, or compatible)
* 16x2 LCD Display (HD44780 compatible, with I2C module or direct pins)
* 4x4 Matrix Keypad
* HC-SR04 Ultrasonic Sensor
* Piezo Buzzer
* 1x LED (for alarm status)
* Resistors:
    * 1x ~220-330 Ohm resistor for the LED
    * 1x 10k Ohm potentiometer for LCD contrast (if not using an I2C backpack with one)
* Jumper Wires
* Breadboard

## Software Dependencies

* Arduino IDE (version 1.8.x or newer recommended)
* [`Keypad.h`](https://playground.arduino.cc/Code/Keypad/): For interfacing with the matrix keypad.
* [`LiquidCrystal.h`](https://www.arduino.cc/en/Reference/LiquidCrystal): For controlling the LCD display.

These libraries are typically included with the Arduino IDE or can be installed via the Library Manager (`Sketch > Include Library > Manage Libraries...`).

## Schematic/Wiring

The pin connections are defined in the `.ino` sketch:

* **LCD Display (as per `LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);`)**
    * RS: Arduino Pin A0
    * E:  Arduino Pin A1
    * D4: Arduino Pin A2
    * D5: Arduino Pin A3
    * D6: Arduino Pin A4
    * D7: Arduino Pin A5
    * VSS: GND
    * VDD: +5V
    * VO: Wiper of 10k potentiometer (other ends to +5V and GND for contrast) or fixed resistor.
    * R/W: GND
    * A (Anode for backlight): +5V (via current-limiting resistor if needed, e.g., 220 Ohm)
    * K (Kathode for backlight): GND

* **Keypad (4x4)**
    * Row Pins (R1-R4): Arduino Pins 11, 10, 9, 8
    * Column Pins (C1-C4): Arduino Pins 7, 6, 5, 4

* **Ultrasonic Sensor (HC-SR04)**
    * `trigPin`: Arduino Pin 12
    * `echoPin`: Arduino Pin 13
    * VCC: +5V
    * GND: GND

* **Buzzer**
    * Positive Pin: Arduino Pin 3
    * Negative Pin: GND

* **Alarm LED**
    * Anode (Longer Leg): Arduino Pin 2 (via ~220-330 Ohm resistor)
    * Cathode (Shorter Leg): GND

*(It's highly recommended to include a simple Fritzing diagram or a hand-drawn schematic image in your repository and link it here for clarity).*

## Installation & Setup

1.  **Clone the Repository (Optional):**
    ```bash
    git clone <your-repository-url>
    cd <repository-name>
    ```
2.  **Gather Hardware:** Collect all the components listed under [Hardware Requirements](#hardware-requirements).
3.  **Wire the Circuit:** Connect the components to your Arduino board as per the [Schematic/Wiring](#schematicwiring) section. Double-check all connections.
4.  **Install Libraries:** Open the Arduino IDE. If `Keypad.h` or `LiquidCrystal.h` are not already installed, go to `Sketch > Include Library > Manage Libraries...` and install them.
5.  **Open the Sketch:** Open the `.ino` file (e.g., `alarm_system.ino`) in the Arduino IDE.
6.  **Select Board & Port:** In the Arduino IDE, select your Arduino board type (e.g., Arduino Uno) under `Tools > Board` and the correct COM port under `Tools > Port`.
7.  **Upload the Code:** Click the "Upload" button (right arrow icon) in the Arduino IDE.

## How to Use

1.  **Power On:** After successfully uploading the sketch and powering the Arduino, the system will boot up. The LCD will display:
    ```
    Inserisci Pin
    [      ]
    ```
2.  **Enter PIN:**
    * Use the numeric keys (0-9) to enter the 6-digit PIN.
    * Each digit entered will be shown as an `*` on the LCD (e.g., `[*** ]`).
    * Press `#` to clear the last entered digit (backspace).
    * Press `*` to submit the entered PIN for verification.

3.  **Arming/Disarming the System:**
    * **If the correct PIN is entered:**
        * If the system was **disarmed**, it will briefly display "Pin corretto" (Correct Pin), then "Allarme Attivo" (Alarm Active). The `alarm` LED (connected to pin 2) will turn ON.
        * If the system was **armed**, it will briefly display "Pin corretto" (Correct Pin), then "Allarme Disattivo" (Alarm Inactive). The `alarm` LED will turn OFF, and the buzzer (if sounding) will stop.
    * **If an incorrect PIN is entered:** The LCD will display "Pin Errato" (Incorrect Pin) then "Riprova" (Try Again) for a short period, then return to the PIN entry screen.

4.  **Motion Detection (When System is Armed):**
    * While the system is armed, the ultrasonic sensor actively monitors for motion.
    * If motion is detected within the sensor's range (approx. < 100 cm in the code), the LCD will display:
        ```
        Movimento rilevato
        Inserisci Pin
        ```
        The piezo buzzer will sound, and the `alarm` LED will remain ON (or flash, if you modify the code).
    * To silence the alarm and disarm the system, enter the correct PIN and press `*`.

5.  **Change PIN:**
    * This function is only accessible when the system is **disarmed**.
    * Press the `A` key on the keypad.
    * The LCD will prompt "Vecchio PIN:" (Old PIN:). Enter the current 6-digit PIN. After the 6th digit is entered, it will automatically proceed.
    * **If the old PIN is correct:**
        * The LCD will prompt "Nuovo PIN:" (New PIN:). Enter your desired new 6-digit PIN.
        * The LCD will then prompt "Conferma PIN:" (Confirm PIN:). Re-enter the new PIN.
        * If both new PIN entries match, the LCD will display "PIN Modificato!" (PIN Changed!) and the new PIN will be saved in the Arduino's volatile memory (it will reset to the hardcoded default on power loss). The system then returns to the "Inserisci Pin" screen.
    * **If the old PIN is incorrect or the new PIN confirmation fails:** An error message will be displayed (e.g., "Vecchio PIN Errato" or "Conferma Errata"), and the PIN change process will either restart from the "Old PIN" step or abort, returning to the main PIN entry screen.

