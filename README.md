# 🔒 SecureMesh

**Rete mesh decentralizzata con chat multi-utente su ESP8266**

[![Platform](https://img.shields.io/badge/platform-ESP8266-blue.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ready-orange.svg)](https://platformio.org/)

---

## 📖 Indice

- [Caratteristiche](#-caratteristiche)
- [Demo Screenshot](#-demo-screenshot)
- [Hardware Richiesto](#-hardware-richiesto)
- [Installazione Rapida](#-installazione-rapida)
- [Utilizzo](#-utilizzo)
- [Architettura](#-architettura)
- [Sviluppo](#-sviluppo)
- [Roadmap](#-roadmap)
- [Licenza](#-licenza)

---

## ✨ Caratteristiche

### 🌐 Rete Mesh Decentralizzata
- Rete **painlessMesh** auto-configurante
- **Zero configurazione** di rete: i nodi si trovano automaticamente
- Supporto fino a **10+ nodi** simultanei
- **Auto-discovery** con cache IP dei nodi
- Heartbeat periodico per rilevamento connessioni/disconnessioni

### 💬 Chat Multi-Utente Avanzata
- **Nickname personalizzati**: ogni utente sceglie il proprio nome
- **Chat pubblica**: messaggi visibili a tutti gli utenti connessi
- **Chat privata 1-to-1**: conversazioni private tra due utenti specifici
- **Multi-utente per nodo**: più utenti possono connettersi allo stesso ESP8266
- **Badge messaggi non letti**: contatori visivi per chat pubbliche e private
- **Memoria nickname**: localStorage salva il nickname tra le sessioni

### 🎨 Interfaccia Utente Professionale
- **Captive Portal automatico**: il portale si apre automaticamente alla connessione WiFi
- **Dashboard responsive**: ottimizzata per mobile e desktop
- **Sidebar utenti**: lista live degli utenti online con stato
- **Design moderno**: gradient purple/blue, animazioni fluide
- **Zero configurazione utente**: tutto funziona out-of-the-box

### ⚡ Performance Ottimizzate
- **RAM libera: ~21KB** su ESP8266 (80KB totali)
- **Flash: ~38%** utilizzata (390KB/1044KB)
- **CSS/JS in PROGMEM**: serviti da Flash memory (zero heap)
- **Latency: 100-400ms** per broadcast mesh
- **Uptime: 24h+** senza crash o memory leak

---

## 📸 Demo Screenshot

### Portale Nodi
Il portale mostra tutti i nodi disponibili con IP, utenti connessi e stato online/offline.

```
┌─────────────────────────────────────┐
│     🔒 SecureMesh                   │
│     Rete Mesh v3.1                  │
├─────────────────────────────────────┤
│ NODE_ALICE         [TU SEI QUI]     │
│ ID: 2919469548                      │
│ IP: 192.168.1.45                    │
│ Utenti: Mario, Luigi                │
│ ● Online - Locale    [Apri Chat]    │
├─────────────────────────────────────┤
│ NODE_BOB                            │
│ ID: 2919469549                      │
│ IP: 192.168.1.46                    │
│ Utenti: Anna                        │
│ ● Online - Remoto    [Apri Chat]    │
└─────────────────────────────────────┘
```

### Dashboard Chat
Sidebar con chat pubblica + lista utenti, area messaggi, input.

```
┌──────────┬────────────────────────────┐
│ NODE_    │  ← Portale   💬 Chat       │
│ ALICE    │     Pubblica               │
│ 👤 Mario ├────────────────────────────┤
│          │                            │
│ CHAT     │  Luigi: Ciao a tutti!      │
│ 📢 Pubb. │  00:05:23                  │
│          │                            │
│ UTENTI   │         Hello 👋           │
│ ● Luigi  │         00:05:45           │
│ ● Anna   │                            │
│          │  [Scrivi un messaggio...] ➤│
└──────────┴────────────────────────────┘
```

---

## 🛠 Hardware Richiesto

### Hardware Minimo
- **1× ESP8266** (Wemos D1 Mini consigliato)
- **1× Cavo micro-USB** (per programmazione)
- **1× Alimentatore USB** 5V 500mA

### Hardware Consigliato (rete mesh)
- **2-4× ESP8266 Wemos D1 Mini**
- **LED integrato** su pin D4 (feedback visivo)
- Opzionale: batterie 18650 + pannelli solari per nodi autonomi

### Specifiche ESP8266
- **Flash:** 4MB
- **RAM:** 80KB (21KB liberi con SecureMesh)
- **CPU:** 80MHz (overclock a 160MHz opzionale)
- **WiFi:** 802.11 b/g/n 2.4GHz

---

## 🚀 Installazione Rapida

### Prerequisiti
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install python3 python3-venv git

# macOS
brew install python3 git

# Windows
# Installa Python da python.org e Git da git-scm.com
```

### Setup Automatico (consigliato)
```bash
# Clone repository
git clone https://github.com/turiliffiu/securemesh.git
cd securemesh

# Esegui setup automatico
chmod +x setup.sh
./setup.sh

# Upload su ESP8266 (porta auto-detect)
source venv/bin/activate
pio run -e node1 --target upload
```

### Setup Manuale
Vedi [INSTALL.md](INSTALL.md) per istruzioni dettagliate.

---

## 📱 Utilizzo

### 1️⃣ Prima Accensione

1. **Alimenta l'ESP8266** (USB o batteria)
2. Attendi **LED lampeggiante** (3 flash = pronto)
3. Monitor seriale mostra:
   ```
   ╔═════════════════════════════════╗
   ║  SECUREMESH v3.1 - NICK+CHAT    ║
   ╚═════════════════════════════════╝
   ✅ Mesh OK!
   ✅ DNS OK! Captive portal attivo
   ✅ Pronto!
   ```

### 2️⃣ Connessione da Smartphone/PC

1. **WiFi Settings** → connettiti a:
   - SSID: `SecureMesh_PoC`
   - Password: `mesh2026secure`

2. **Portale si apre automaticamente**
   - Android: portale captive automatico
   - iOS: notifica "Accedi alla rete"
   - PC: apri manualmente `http://192.168.4.1`

3. **Scegli nodo** → click `Apri Chat`

### 3️⃣ Chat

1. **Modal nickname**: inserisci nome (es. `Mario`)
2. **Chat pubblica**: scrivi messaggi visibili a tutti
3. **Chat privata**: click su utente in sidebar → chat 1-to-1

### 4️⃣ Più Nodi (Mesh Network)

1. Alimenta **2+ ESP8266** con firmware SecureMesh
2. I nodi **si trovano automaticamente** in ~5-10 secondi
3. Nel portale vedi **lista nodi disponibili**
4. Utenti su nodi diversi possono chattare tra loro

---

## 🏗 Architettura

### Livelli del Sistema

```
┌─────────────────────────────────────────┐
│  Browser (WiFi client)                  │
│  - Captive Portal / Dashboard           │
│  - JavaScript (polling API ogni 2s)     │
└────────────┬────────────────────────────┘
             │ HTTP / WebSocket
┌────────────▼────────────────────────────┐
│  ESP8266 (AsyncWebServer)               │
│  - API REST (/api/*)                    │
│  - CSS/JS serviti da PROGMEM            │
│  - Gestione utenti locali               │
└────────────┬────────────────────────────┘
             │ painlessMesh
┌────────────▼────────────────────────────┐
│  Mesh Network (altri ESP8266)           │
│  - Broadcast messaggi                   │
│  - Discovery automatico                 │
│  - Sync utenti globali                  │
└─────────────────────────────────────────┘
```

### Flusso Messaggio Privato

```
Mario@NODE_ALICE → beffo@NODE_BOB

1. Mario scrive "Hello" e invia
2. NODE_ALICE → mesh.sendBroadcast({
     type: "chat",
     fromUser: "Mario",
     toUser: "beffo",
     text: "Hello"
   })
3. NODE_BOB riceve broadcast
4. NODE_BOB filtra: beffo è utente locale? ✅
5. Salva messaggio in buffer locale
6. beffo fa polling /api/messages → vede "Hello"
```

### Strutture Dati Principali

```cpp
// Messaggio (15 max, circular buffer)
struct Message {
    char fromUser[21];   // nickname mittente
    char fromNode[16];   // nodo origine
    char text[201];      // testo messaggio
    char toUser[21];     // "" = pubblico, "nick" = privato
    unsigned long timestamp;
    bool isOwn;          // true = inviato da questo nodo
};

// Utente locale (6 max per nodo)
struct LocalUser {
    char nickname[21];
    unsigned long lastSeen;  // keepalive ogni 20s
};

// Utente globale (12 max rete mesh)
struct GlobalUser {
    char nickname[21];
    char nodeName[16];
    unsigned long lastSeen;  // timeout 90s
};
```

Vedi [ARCHITECTURE.md](ARCHITECTURE.md) per dettagli completi.

---

## 👨‍💻 Sviluppo

### Struttura Progetto

```
securemesh/
├── src/
│   └── main.cpp              # Firmware attivo
├── funzionanti/
│   └── firmware_v3.1_ram_fix.cpp  # Golden backup
├── platformio.ini            # Config PlatformIO
├── setup.sh                  # Setup automatico
├── README.md                 # Questo file
├── INSTALL.md                # Guida installazione
├── ARCHITECTURE.md           # Documentazione tecnica
└── CHANGELOG.md              # Storia versioni
```

### Build e Flash

```bash
# Attiva virtual environment
source venv/bin/activate

# Build per nodo specifico
pio run -e node1

# Upload via USB (auto-detect porta)
pio run -e node1 --target upload

# Monitor seriale
pio device monitor --baud 115200

# Build + Upload + Monitor (one-liner)
pio run -e node1 --target upload && pio device monitor
```

### Configurazione Nodi

Modifica `platformio.ini`:

```ini
[env:node1]
build_flags = 
    ${common.build_flags}
    -DNODE_NAME=\"NODE_ALICE\"

[env:node2]
build_flags = 
    ${common.build_flags}
    -DNODE_NAME=\"NODE_BOB\"
```

### Debug

```cpp
// Nel firmware, livello di log:
mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);

// Nel loop, ogni 15s:
Serial.printf("Nodi:%d Lu:%d Gu:%d Msg:%d RAM:%uK\n",
    mesh.getNodeList().size(), localUsersCount, 
    globalUsersCount, messageCount, ESP.getFreeHeap() / 1024);
```

---

## 🗺 Roadmap

### v3.2 (Prossima Release)
- [ ] Emoji picker integrato
- [ ] Timestamp assoluto (RTC/NTP)
- [ ] Notifica sonora nuovi messaggi
- [ ] Tema scuro

### v4.0 (Future)
- [ ] Gruppi custom (oltre chat pubblica)
- [ ] Storage persistente messaggi (SPIFFS)
- [ ] Cifratura E2E (AES-256)
- [ ] File transfer (immagini)
- [ ] PWA + push notifications

### v5.0 (Advanced)
- [ ] Voice messages
- [ ] Network topology visualization
- [ ] LoRa mesh support
- [ ] Multi-hop routing intelligente

---

## 🤝 Contribuire

I contributi sono benvenuti! Per favore:

1. Fork del repository
2. Crea feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit con messaggi chiari (`git commit -m 'Add: emoji picker'`)
4. Push al branch (`git push origin feature/AmazingFeature`)
5. Apri Pull Request

### Guidelines
- Testa sempre su hardware reale prima del PR
- Mantieni RAM libera >= 18KB
- Documenta nuove API in `ARCHITECTURE.md`
- Aggiungi entry in `CHANGELOG.md`

---

## 📄 Licenza

Questo progetto è rilasciato sotto licenza **MIT**.  
Vedi [LICENSE](LICENSE) per dettagli completi.

```
Copyright (c) 2026 Salvo (turiliffiu)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software...
```

---

## 🙏 Crediti

### Librerie Utilizzate
- [painlessMesh](https://gitlab.com/painlessMesh/painlessMesh) - Mesh networking
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Async web server
- [ArduinoJson](https://arduinojson.org/) - JSON parsing
- [DNSServer](https://github.com/esp8266/Arduino) - Captive portal

### Ispirazione
- [ESP-NOW](https://www.espressif.com/en/products/software/esp-now/overview) - Espressif protocol
- [Meshtastic](https://meshtastic.org/) - Long-range mesh communication

---

## 📞 Supporto

- **Issues:** [GitHub Issues](https://github.com/TUO_USERNAME/securemesh/issues)
- **Discussions:** [GitHub Discussions](https://github.com/TUO_USERNAME/securemesh/discussions)
- **Email:** tuo.email@example.com

---

## ⭐ Statistiche Progetto

- **Linee di codice:** ~1000 (firmware + docs)
- **RAM utilizzata:** ~59KB/80KB (26% overhead)
- **Flash utilizzata:** ~390KB/1044KB (37%)
- **Latenza mesh:** 100-400ms
- **Uptime testato:** 48+ ore
- **Nodi testati:** 4 simultanei

---

**Realizzato con ❤️ per l'IoT decentralizzato**

*SecureMesh - Comunicazione libera, senza server, senza censura.*
