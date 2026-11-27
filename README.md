# Raspberry Pi Alarm Clock (`libgpiod`)

C++ alarm clock for Raspberry Pi using `libgpiod` with a 4-digit 7-segment display (shift register), two buttons, and a buzzer.

## Breadboard Setup
<img width="3024" height="2679" alt="PXL_20261127_164515853~2" src="https://github.com/user-attachments/assets/17ef2c0d-4ce6-4801-bfc2-5c02e6f1bd85" />

## Components

* 1x Raspberry Pi
* 1x Breadboard & jumper wires
* 1x 4-digit 7-segment display
* 1x 8-bit shift register (74HC595)
* 2x Push buttons
* 1x Active Buzzer
* 1x NPN transistor (2N3904 / PN2222)
* 8x Resistors (100 Ω, 7-segment current limiting)
* 2x Resistors (330 Ω, button pull-down)
* 1x Resistor (1000 Ω, transistor base)[cite: 1, 2, 3]

## GPIO Pinout

### Shift Register (`QA..QH` -> `A, B, C, D, E, F, G, DP`)

| Signal | BCM GPIO | Direction | Role |
| :--- | :--- | :--- | :--- |
| `SER` | GPIO `17` | `OUTPUT` | Serial data input |
| `SRCLK` | GPIO `27` | `OUTPUT` | Shift clock |
| `RCLK` | GPIO `22` | `OUTPUT` | Latch clock |

### Digits & Peripherals

| Component | BCM GPIO | Direction | Role |
| :--- | :--- | :--- | :--- |
| Digit 1 | GPIO `23` | `OUTPUT` | Digit 1 select |
| Digit 2 | GPIO `24` | `OUTPUT` | Digit 2 select |
| Digit 3 | GPIO `25` | `OUTPUT` | Digit 3 select |
| Digit 4 | GPIO `6` | `OUTPUT` | Digit 4 select |
| Button 1 | GPIO `21` | `INPUT` | Button 1 |
| Button 2 | GPIO `20` | `INPUT` | Button 2 |
| Buzzer | GPIO `19` | `OUTPUT` | Buzzer |
| Common Ground | `GND` | - | Shared ground |

## Usage

In **clock mode**:
* **Button 1:**
  * **Short press:** Toggle the alarm on/off and view the alarm
  * **Long press:** Edit the alarm
* **Button 2:**
  * **Short press:** View the volume setting
  * **Long press:** Edit the volume setting
  
## Dependencies

```bash
sudo apt update
sudo apt install -y g++ make libgpiod-dev gpiod
```

## Build & Run

```bash
make
./alarm_clock
```
