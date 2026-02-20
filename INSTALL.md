# 📦 Guida Installazione SecureMesh

Istruzioni complete per installare e configurare SecureMesh su ESP8266.

---

## 📋 Indice

1. [Prerequisiti](#1-prerequisiti)
2. [Setup Ambiente](#2-setup-ambiente)
3. [Clone Repository](#3-clone-repository)
4. [Installazione Automatica](#4-installazione-automatica)
5. [Installazione Manuale](#5-installazione-manuale)
6. [Upload Firmware](#6-upload-firmware)
7. [Verifica Funzionamento](#7-verifica-funzionamento)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. Prerequisiti

### 🐧 Linux (Ubuntu/Debian)

```bash
# Aggiorna pacchetti
sudo apt update && sudo apt upgrade -y

# Installa dipendenze base
sudo apt install -y \
    python3 \
    python3-pip \
    python3-venv \
    git \
    usbutils

# Aggiungi utente a gruppo dialout (accesso seriale)
sudo usermod -a -G dialout $USER

# IMPORTANTE: Logout e login per applicare gruppo
# Oppure: newgrp dialout
```

### 🍎 macOS

```bash
# Installa Homebrew (se non presente)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Installa dipendenze
brew install python3 git

# Driver USB-Serial (CH340/CP2102)
# Scarica da: https://www.wch.cn/downloads/CH341SER_MAC_ZIP.html
```

### 🪟 Windows

1. **Python 3:**
   - Download: https://www.python.org/downloads/
   - ✅ Spunta "Add Python to PATH" durante installazione

2. **Git:**
   - Download: https://git-scm.com/download/win
   - Usa impostazioni default

3. **Driver USB:**
   - CH340: https://sparks.gogo.co.nz/ch340.html
   - CP2102: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

4. **PowerShell o Git Bash** per eseguire comandi

---

## 2. Setup Ambiente

### Verifica Installazione Python

```bash
python3 --version
# Output atteso: Python 3.8.x o superiore

pip3 --version
# Output atteso: pip 21.x o superiore
```

### Identifica Porta Seriale ESP8266

**Linux:**
```bash
# Prima di connettere ESP8266
ls /dev/ttyUSB*

# Connetti ESP8266 via USB

# Dopo connessione (nuova porta = tua ESP8266)
ls /dev/ttyUSB*
# Output tipico: /dev/ttyUSB0
```

**macOS:**
```bash
ls /dev/cu.*
# Output tipico: /dev/cu.usbserial-1410
```

**Windows (PowerShell):**
```powershell
Get-WmiObject Win32_SerialPort | Select-Object Name,DeviceID
# Output tipico: COM3, COM4, etc.
```

---

## 3. Clone Repository

```bash
# Clone da GitHub
git clone https://github.com/turiliffiu/securemesh.git
cd securemesh

# Verifica contenuto
ls -la
```

Output atteso:
```
.
├── funzionanti/
├── include/
├── platformio.ini
├── setup.sh
├── src/
├── test/
├── README.md
├── INSTALL.md
├── ARCHITECTURE.md
└── CHANGELOG.md
```

---

## 4. Installazione Automatica

### Script Setup (Consigliato)

```bash
# Dai permessi esecuzione
chmod +x setup.sh

# Esegui setup
./setup.sh
```

Lo script:
1. ✅ Crea virtual environment Python (se non esiste)
2. ✅ Attiva il venv
3. ✅ Installa PlatformIO
4. ✅ Verifica dipendenze librerie
5. ✅ Mostra istruzioni upload

Output atteso:
```
🚀 SecureMesh - Setup Automatico
================================

✅ Virtual environment trovato: venv/
✅ Virtual environment attivo
✅ PlatformIO installato: 6.x.x
✅ Librerie verificate

🎯 Setup completato!

Prossimi passi:
1. Connetti ESP8266 via USB
2. pio run -e node1 --target upload
3. pio device monitor --baud 115200
```

### Attivazione Manuale Virtual Environment

**Linux/macOS:**
```bash
source venv/bin/activate
```

**Windows PowerShell:**
```powershell
.\venv\Scripts\Activate.ps1
```

**Windows CMD:**
```cmd
venv\Scripts\activate.bat
```

Dopo attivazione, il prompt mostra `(venv)`:
```
(venv) user@host:~/securemesh$
```

---

## 5. Installazione Manuale

Se `setup.sh` non funziona o preferisci passo-passo:

### Step 1: Crea Virtual Environment

```bash
cd ~/securemesh
python3 -m venv venv
```

### Step 2: Attiva Virtual Environment

```bash
source venv/bin/activate  # Linux/macOS
# oppure
.\venv\Scripts\Activate.ps1  # Windows
```

### Step 3: Installa PlatformIO

```bash
pip install platformio

# Verifica
pio --version
# Output: PlatformIO Core, version 6.x.x
```

### Step 4: Installa Dipendenze Librerie

```bash
cd ~/securemesh
pio pkg install
```

Output atteso:
```
Installing painlessMesh/painlessMesh @ ^1.5.0
Installing bblanchon/ArduinoJson @ ^6.21.0
Installing me-no-dev/ESPAsyncTCP @ ^1.2.2
Installing ottowinter/ESPAsyncWebServer-esphome @ ^3.2.2
```

---

## 6. Upload Firmware

### Build Firmware

```bash
# Build per NODE_ALICE (environment node1)
pio run -e node1

# Build per NODE_BOB (environment node2)
pio run -e node2

# Build per NODE_CHARLIE (environment node3)
pio run -e node3
```

Output successo:
```
Building in release mode
...
RAM:   [======    ]  58.8% (used 48240 bytes from 81920 bytes)
Flash: [===       ]  37.8% (used 394536 bytes from 1044464 bytes)
```

### Upload via USB

**Auto-detect porta:**
```bash
pio run -e node1 --target upload
```

**Porta specifica:**
```bash
# Linux
pio run -e node1 --target upload --upload-port /dev/ttyUSB0

# macOS
pio run -e node1 --target upload --upload-port /dev/cu.usbserial-1410

# Windows
pio run -e node1 --target upload --upload-port COM3
```

Output atteso durante upload:
```
Uploading .pio/build/node1/firmware.bin
esptool.py v4.5.1
Serial port /dev/ttyUSB0
Connecting....
Chip is ESP8266EX
...
Writing at 0x00000000... (100 %)
Wrote 394536 bytes (285412 compressed) at 0x00000000 in 25.3 seconds
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

---

## 7. Verifica Funzionamento

### Monitor Seriale

```bash
pio device monitor --baud 115200
```

**Output corretto al boot:**
```
╔══════════════════════════════════╗
║  SECUREMESH v3.1 - NICK+CHAT    ║
╚══════════════════════════════════╝
Nodo: NODE_ALICE | Heap: 24K

✅ Mesh OK!
   Node ID: 2919469548
✅ DNS OK! Captive portal attivo
✅ Pronto!

1. WiFi: SecureMesh_PoC / mesh2026secure
2. Portale apre automaticamente
3. Scegli nodo → nickname → chatta!

[NODE_ALICE] Nodi:1 Cache:0 Lu:0 Gu:0 Msg:0 RAM:21K
```

**Interrompi monitor:** `Ctrl+C`

### Test Connessione WiFi

**Smartphone/PC:**
1. WiFi → `SecureMesh_PoC` (password: `mesh2026secure`)
2. Portale captive si apre automaticamente
3. Se non si apre: browser → `http://192.168.4.1`

**Verifica portale:**
```
✅ Vedi NODE_ALICE con badge "TU SEI QUI"
✅ Button "Apri Chat" funzionante
✅ Stats: 1 Nodo, 0 Utenti, XX K RAM
```

### Test Chat

1. Click **Apri Chat**
2. Modal nickname → inserisci `TestUser` → Enter
3. Input messaggio → scrivi `Hello world` → Invio
4. ✅ Messaggio appare in chat pubblica
5. ✅ Timestamp e badge "Tu"

---

## 8. Troubleshooting

### ❌ Problema: "Permission denied" su porta seriale

**Linux:**
```bash
# Aggiungi utente a gruppo dialout
sudo usermod -a -G dialout $USER

# Logout e login
# Oppure applica subito:
newgrp dialout
```

### ❌ Problema: "Serial port not found"

**Verifica connessione USB:**
```bash
# Linux
dmesg | grep tty
# Cerca: ttyUSB0 attached

# Test porta manuale
pio device list
```

**Controlla driver USB:**
- CH340: https://github.com/juliagoda/CH341SER
- CP2102: da Silicon Labs

### ❌ Problema: "Failed to connect to ESP8266"

**1. Hard reset durante upload:**
- Tieni premuto **BOOT/FLASH** button su ESP8266
- Mentre tieni premuto, premi **RESET**
- Rilascia RESET (tieni ancora BOOT)
- Esegui `pio run -e node1 --target upload`
- Rilascia BOOT quando vedi "Connecting...."

**2. Verifica baudrate:**
```bash
# Prova velocità più bassa
pio run -e node1 --target upload --upload-speed 115200
```

### ❌ Problema: RAM troppo bassa (<18KB)

**Nel monitor vedi `RAM: 7K` invece di `RAM: 21K`?**

Controlla che `src/main.cpp` sia la versione v3.1:
```bash
head -5 src/main.cpp
```

Deve mostrare:
```cpp
/*
 * SecureMesh v3.1 - NICKNAME + CHAT (RAM FIX)
```

Se diverso:
```bash
cp funzionanti/firmware_v3.1_ram_fix.cpp src/main.cpp
pio run -e node1 --target upload
```

### ❌ Problema: "Multiple definition of setup/loop"

**Causa:** file `.cpp` extra in `src/`

**Soluzione:**
```bash
# Sposta backup fuori da src/
mv src/main_*.cpp funzionanti/

# Verifica solo main.cpp in src/
ls -la src/
# Deve contenere SOLO main.cpp
```

### ❌ Problema: Portale non si apre automaticamente

**Android:**
- Disattiva "Rete mobile" temporaneamente
- Attiva solo WiFi SecureMesh_PoC
- Apri browser → `192.168.4.1`

**iOS:**
- Ignora avviso "Nessuna connessione Internet"
- Click notifica "Accedi alla rete"

**PC:**
- Apri manualmente `http://192.168.4.1`

### ❌ Problema: CSS/JS mostrati come testo corrotto

**Causa:** RAM insufficiente (vecchia versione firmware)

**Soluzione:** usa firmware v3.1 con CSS/JS in PROGMEM
```bash
cp funzionanti/firmware_v3.1_ram_fix.cpp src/main.cpp
pio run --target clean
pio run -e node1 --target upload
```

### ❌ Problema: Chat privata non funziona

**Sintomo:** messaggi privati non appaiono al destinatario

**Verifica versione firmware:**
```bash
grep "v3.1" src/main.cpp
```

Se manca `v3.1`, copia versione corretta:
```bash
cp funzionanti/firmware_v3.1_ram_fix.cpp src/main.cpp
```

**Svuota cache browser:**
- Android Chrome: Menu → Impostazioni → Privacy → Cancella dati
- PC Chrome: `Ctrl+Shift+R` (hard reload)

---

## 🎯 Installazione Multi-Nodo

Per rete mesh con 2+ ESP8266:

### 1. Prepara Hardware

- **2-4× ESP8266 Wemos D1 Mini**
- **2-4× Cavi USB** (oppure 1 cavo + alimentatori USB)

### 2. Flash Nodi Diversi

```bash
# Nodo 1 - NODE_ALICE
pio run -e node1 --target upload --upload-port /dev/ttyUSB0

# Scollega, connetti Nodo 2
# Nodo 2 - NODE_BOB
pio run -e node2 --target upload --upload-port /dev/ttyUSB0

# Scollega, connetti Nodo 3
# Nodo 3 - NODE_CHARLIE
pio run -e node3 --target upload --upload-port /dev/ttyUSB0
```

### 3. Alimenta Tutti i Nodi

- Collega alimentazione (USB o batteria)
- **Attendi 10-15 secondi** per mesh auto-discovery
- LED lampeggia 5 volte quando trova nuovo nodo

### 4. Verifica Mesh

**Monitor su NODE_ALICE:**
```
[NODE_ALICE] Nodi:3 Cache:2 Lu:0 Gu:0 Msg:0 RAM:21K
```
`Nodi:3` = mesh funzionante! ✅

**Dal portale:**
- Vedi **3 card** (NODE_ALICE, NODE_BOB, NODE_CHARLIE)
- Ogni nodo mostra **● Online - Remoto**
- Click **Apri Chat** su qualsiasi nodo

---

## 📞 Supporto

Problemi non risolti?

1. **Controlla:** [README.md](README.md) → sezione FAQ
2. **Issues:** [GitHub Issues](https://github.com/TUO_USERNAME/securemesh/issues)
3. **Log completo:** `pio device monitor --baud 115200 > debug.log`

Allega sempre:
- Output `pio run -e node1 -v` (verbose build)
- Log monitor seriale completo
- Sistema operativo e versione
- Modello ESP8266

---

**🎉 Installazione completata! Buon meshing!**
