# Raspberry Pi Alarm Clock (`libgpiod`)

C++ alarm clock for Raspberry Pi using `libgpiod` with a 4-digit 7-segment display (shift register), two buttons, and a buzzer.

## Breadboard Setup

<img width="2048" height="1750" alt="IMG-20261127-WA0000" src="https://github.com/user-attachments/assets/e42c54ca-39d4-458a-a9eb-d6f517249caf" />

## GPIO Pinout (BCM)

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
