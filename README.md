# NUCLEO-G474RE Zephyr I2C Target Device

Dette projekt viser, hvordan et **ST NUCLEO-G474RE** board kan bruges som et **I2C device / I2C target** med Zephyr RTOS.

Boardet opfører sig som en lille I2C-enhed med et simpelt register-map. Via I2C kan en ekstern controller/master tænde og slukke **LED2** på NUCLEO-boardet.

## Funktionalitet

- Board: `nucleo_g474re`
- RTOS: Zephyr
- I2C-mode: target/device
- I2C-adresse: `0x42`
- I2C-interface: `I2C1`
- Pins:
  - `PB8` = I2C1 SCL
  - `PB9` = I2C1 SDA
- LED2 styres via et I2C-register

## Register-map

| Register | Navn | Beskrivelse |
|---:|---|---|
| `0x00` | `REG_LED2_CONTROL` | Skriv `0x00` for at slukke LED2, skriv `0x01` for at tænde LED2 |
| `0x01` | `REG_LED2_STATUS` | Læs LED2-status. `0x00` = slukket, `0x01` = tændt |
| `0x02` | `REG_DEVICE_ID` | Fast device-ID. Eksempel: `0x47` |

## Hardware-forbindelse

Forbind I2C-controller/master til NUCLEO-G474RE:

| Controller/master | NUCLEO-G474RE |
|---|---|
| SCL | PB8 |
| SDA | PB9 |
| GND | GND |

I2C-bussen skal have pull-up-modstande.

Typisk:

```text
SCL -> 3.3 V via 4.7 kΩ
SDA -> 3.3 V via 4.7 kΩ
```

Brug **3.3 V pull-ups**. Brug ikke 5 V pull-ups direkte på STM32 I2C-linjerne.

## Projektstruktur

```text
nucleo_g474re_i2c_target/
├── CMakeLists.txt
├── prj.conf
├── boards/
│   └── nucleo_g474re.overlay
└── src/
    └── main.c
```

## Krav

På Windows/WSL anbefales følgende:

- Windows 10/11
- WSL2 med Ubuntu
- Zephyr SDK installeret
- Zephyr workspace oprettet
- Python virtual environment
- `west`
- `usbipd-win`, hvis ST-LINK skal bruges direkte fra WSL

## Aktivér Python virtual environment

Hvis dit virtual environment ligger i projektmappen:

```bash
cd ~/zephyrproject/apps/nucleo_g474re_i2c_target
source .venv/bin/activate
```

Hvis dit virtual environment ligger i Zephyr-workspace:

```bash
source ~/zephyrproject/.venv/bin/activate
```

Kontroller at `west` virker:

```bash
west --version
```

Hvis `west` mangler:

```bash
pip install west
```

Installer Zephyr Python requirements:

```bash
pip install -r ~/zephyrproject/zephyr/scripts/requirements.txt
```

## Kompilér projektet fra WSL

Gå til projektmappen:

```bash
cd ~/zephyrproject/apps/nucleo_g474re_i2c_target
```

Byg projektet til NUCLEO-G474RE:

```bash
west build -p always -b nucleo_g474re .
```

Forklaring:

```text
west build        Starter Zephyr build
-p always         Rydder gammel CMake-cache før build
-b nucleo_g474re  Vælger board
.                 Bruger nuværende mappe som applikation
```

Hvis build lykkes, ligger firmwarefilen typisk her:

```text
build/zephyr/zephyr.elf
build/zephyr/zephyr.hex
build/zephyr/zephyr.bin
```

## Flash programmet fra WSL

Når ST-LINK er tilgængelig i WSL, kan du flashe med:

```bash
west flash
```

Hvis du vil bygge og flashe i én arbejdsgang:

```bash
west build -p always -b nucleo_g474re .
west flash
```

## Se printk-output

Zephyr `printk()` kommer normalt ud på boardets console UART via ST-LINK Virtual COM Port.

I WSL kan den typisk ses som:

```bash
/dev/ttyACM0
```

Find porten:

```bash
ls /dev/ttyACM*
```

Åbn serial monitor med `screen`:

```bash
sudo apt install -y screen
screen /dev/ttyACM0 115200
```

Afslut `screen` med:

```text
Ctrl + A
K
Y
```

Alternativt kan du bruge Windows-programmer som PuTTY, Tera Term eller VS Code Serial Monitor.

Indstillinger:

```text
Baudrate: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

## Del ST-LINK USB fra Windows host til WSL

WSL2 har normalt ikke direkte adgang til USB-enheder. For at bruge ST-LINK fra WSL skal USB-enheden deles fra Windows til WSL med `usbipd-win`.

### 1. Installer usbipd-win i Windows

Åbn PowerShell som administrator og kør:

```powershell
winget install --exact dorssel.usbipd-win
```

Genstart eventuelt Windows efter installationen.

### 2. Opdater WSL

Åbn PowerShell:

```powershell
wsl --update
wsl --shutdown
```

Start derefter din Ubuntu/WSL igen.

### 3. Tilslut NUCLEO-boardet

Sæt NUCLEO-G474RE i USB-porten via ST-LINK USB-stikket.

### 4. Find ST-LINK busid i Windows

Åbn PowerShell og kør:

```powershell
usbipd list
```

Find linjen med ST-LINK. Den kan fx ligne:

```text
BUSID  VID:PID    DEVICE
2-3    0483:374b  STMicroelectronics STLink dongle
```

Notér `BUSID`, fx:

```text
2-3
```

### 5. Bind USB-enheden

Dette skal normalt køres i PowerShell som administrator:

```powershell
usbipd bind --busid 2-3
```

Erstat `2-3` med dit eget `BUSID`.

Hvis Windows holder fast i enheden, kan du prøve:

```powershell
usbipd bind --force --busid 2-3
```

### 6. Attach ST-LINK til WSL

Sørg for at en WSL-terminal er åben. Det holder WSL2 VM'en aktiv.

Kør derefter i PowerShell:

```powershell
usbipd attach --wsl --busid 2-3
```

Erstat igen `2-3` med dit eget `BUSID`.

Når enheden er attached til WSL, kan Windows normalt ikke bruge den samtidig. Det betyder fx, at STM32CubeProgrammer i Windows ikke bør være forbundet til ST-LINK samtidig.

### 7. Kontroller i WSL at ST-LINK er synlig

I Ubuntu/WSL:

```bash
sudo apt install -y usbutils
lsusb
```

Du bør se noget i stil med:

```text
STMicroelectronics ST-LINK
```

Du kan også se efter seriel port:

```bash
ls /dev/ttyACM*
```

### 8. Flash fra WSL

Når ST-LINK er synlig i WSL:

```bash
cd ~/zephyrproject/apps/nucleo_g474re_i2c_target
west flash
```

## Frigiv ST-LINK fra WSL igen

Hvis du vil give USB-enheden tilbage til Windows:

```powershell
usbipd detach --busid 2-3
```

Erstat `2-3` med dit eget `BUSID`.

## Typiske fejl og løsninger

### `west: command not found`

Aktivér dit Python virtual environment:

```bash
source ~/zephyrproject/.venv/bin/activate
```

Eller installer `west`:

```bash
pip install west
```

### `No ST-LINK detected`

Kontroller:

```bash
lsusb
```

Hvis ST-LINK ikke vises, er den ikke delt korrekt til WSL. Kør igen i PowerShell:

```powershell
usbipd list
usbipd attach --wsl --busid <BUSID>
```

### `Permission denied` ved adgang til USB/serial

Prøv midlertidigt med `sudo`, fx:

```bash
sudo west flash
```

Mere permanent kan brugeren tilføjes til relevante grupper:

```bash
sudo usermod -aG dialout $USER
```

Log ud og ind igen bagefter.

### `/dev/ttyACM0` findes ikke

Tjek om boardet er attached til WSL:

```bash
lsusb
```

Hvis `lsusb` ikke viser ST-LINK, skal USB-enheden attaches igen fra Windows:

```powershell
usbipd attach --wsl --busid <BUSID>
```

### Windows bruger ST-LINK samtidig

Luk programmer som kan holde ST-LINK åben:

- STM32CubeProgrammer
- STM32CubeIDE
- PuTTY/Tera Term
- VS Code debug-sessioner

Attach derefter igen:

```powershell
usbipd attach --wsl --busid <BUSID>
```

## Test med Linux I2C-controller

Eksempel fra Raspberry Pi eller anden Linux-maskine:

Scan I2C-bus:

```bash
i2cdetect -y 1
```

Forventet I2C-adresse:

```text
0x42
```

Tænd LED2:

```bash
i2cset -y 1 0x42 0x00 0x01
```

Sluk LED2:

```bash
i2cset -y 1 0x42 0x00 0x00
```

Læs LED2-status:

```bash
i2cset -y 1 0x42 0x01
i2cget -y 1 0x42
```

Læs device-ID:

```bash
i2cset -y 1 0x42 0x02
i2cget -y 1 0x42
```

## Test med i2c_scan.py
Med filen i2c_scan.py kan man via en ESP32 lave en test der aktivere LD2 og som læser ID samt status på LD2. 

## Relevante filer

### `prj.conf`

Projektet skal mindst have:

```conf
CONFIG_I2C=y
CONFIG_I2C_TARGET=y
CONFIG_GPIO=y

CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y

CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3

CONFIG_MAIN_STACK_SIZE=2048
```

### `boards/nucleo_g474re.overlay`

```dts
&i2c1 {
	status = "okay";
	clock-frequency = <I2C_BITRATE_STANDARD>;
};
```

## Referencer

- Microsoft: Connect USB devices to WSL  
  https://learn.microsoft.com/windows/wsl/connect-usb

- usbipd-win WSL support  
  https://github.com/dorssel/usbipd-win/wiki/WSL-support

- Zephyr documentation  
  https://docs.zephyrproject.org/
