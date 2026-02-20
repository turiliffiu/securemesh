# 🏗 Architettura SecureMesh

Documentazione tecnica completa dell'architettura del sistema.

---

## 📋 Indice

1. [Overview](#overview)
2. [Stack Tecnologico](#stack-tecnologico)
3. [Architettura Hardware](#architettura-hardware)
4. [Architettura Software](#architettura-software)
5. [Strutture Dati](#strutture-dati)
6. [Flussi di Comunicazione](#flussi-di-comunicazione)
7. [API REST](#api-rest)
8. [Gestione Memoria](#gestione-memoria)
9. [Protocollo Mesh](#protocollo-mesh)
10. [Performance](#performance)

---

## Overview

SecureMesh è un sistema di comunicazione mesh decentralizzato basato su ESP8266 che permette chat multi-utente con nickname personalizzati, chat pubblica e privata 1-to-1.

### Caratteristiche Chiave
- **Zero configurazione**: captive portal automatico
- **Decentralizzato**: nessun server centrale
- **Multi-utente**: più utenti per nodo ESP8266
- **Ottimizzato RAM**: ~21KB liberi su 80KB totali
- **Scalabile**: testato fino a 4 nodi / 12 utenti

---

## Stack Tecnologico

### Hardware
- **MCU:** ESP8266 (80MHz, 80KB RAM, 4MB Flash)
- **Board:** Wemos D1 Mini
- **WiFi:** 802.11 b/g/n 2.4GHz

### Software
- **Framework:** Arduino (PlatformIO)
- **Linguaggio:** C++ (firmware), JavaScript (frontend)
- **Librerie Core:**
  - `painlessMesh` v1.5.0 - Networking mesh
  - `ESPAsyncWebServer` v3.2.2 - HTTP server asincrono
  - `ArduinoJson` v6.21.0 - JSON parsing
  - `DNSServer` - Captive portal

---

## Architettura Hardware

### Pinout ESP8266 Wemos D1 Mini

```
         ┌─────────────┐
   RST ──┤ •         • ├── TX
   A0  ──┤           • ├── RX
   D0  ──┤           • ├── D1 (I2C SCL)
   D5  ──┤   WEMOS   • ├── D2 (I2C SDA)
   D6  ──┤   D1 MINI • ├── D3
   D7  ──┤           • ├── D4 (LED) ◄── Usato
   D8  ──┤           • ├── GND
  3V3  ──┤ •       • • ├── 5V
         └─────────────┘
```

**Pin utilizzati:**
- `D4` (GPIO2): LED integrato per feedback visivo
  - 2 flash = messaggio pubblico ricevuto
  - 3 flash = messaggio privato ricevuto
  - 5 flash = nuovo nodo connesso

---

## Architettura Software

### Diagramma Livelli

```
┌───────────────────────────────────────────────┐
│           Browser (Client)                    │
│  ┌──────────────────────────────────────────┐ │
│  │  HTML + CSS + JavaScript                 │ │
│  │  - Modal nickname (localStorage)         │ │
│  │  - Sidebar utenti                        │ │
│  │  - Chat pubblica/privata                 │ │
│  │  - Polling API (/api/messages ogni 2s)   │ │
│  └──────────────────────────────────────────┘ │
└────────────────┬──────────────────────────────┘
                 │ HTTP GET/POST
┌────────────────▼──────────────────────────────┐
│        ESP8266 - AsyncWebServer               │
│  ┌──────────────────────────────────────────┐ │
│  │  Routes & API                            │ │
│  │  - GET /          → Portale              │ │
│  │  - GET /dashboard → Chat UI              │ │
│  │  - GET /dash.css  → CSS (PROGMEM)        │ │
│  │  - GET /dash.js   → JS (PROGMEM)         │ │
│  │  - POST /api/join → Registra nickname    │ │
│  │  - POST /api/ping → Keepalive            │ │
│  │  - GET /api/users → Lista utenti         │ │
│  │  - GET /api/messages → Messaggi          │ │
│  │  - POST /api/send → Invia messaggio      │ │
│  └──────────────────────────────────────────┘ │
│  ┌──────────────────────────────────────────┐ │
│  │  Gestione Stato                          │ │
│  │  - LocalUser[6]  (utenti su questo nodo) │ │
│  │  - GlobalUser[12] (utenti rete mesh)     │ │
│  │  - Message[15]   (buffer circolare)      │ │
│  │  - NodeInfo[10]  (cache nodi + IP)       │ │
│  └──────────────────────────────────────────┘ │
└────────────────┬──────────────────────────────┘
                 │ painlessMesh broadcast
┌────────────────▼──────────────────────────────┐
│         painlessMesh - P2P Network            │
│  ┌──────────────────────────────────────────┐ │
│  │  Messaggi Mesh (JSON)                    │ │
│  │  - type: "chat"    → Messaggio utente    │ │
│  │  - type: "users"   → Lista utenti nodo   │ │
│  │  - type: "info_request"  → Discovery     │ │
│  │  - type: "info_response" → IP + NodeID   │ │
│  │  - type: "heartbeat"     → Keepalive     │ │
│  └──────────────────────────────────────────┘ │
└────────────────┬──────────────────────────────┘
                 │ WiFi mesh (broadcast)
┌────────────────▼──────────────────────────────┐
│         Altri ESP8266 nella rete mesh         │
└───────────────────────────────────────────────┘
```

---

## Strutture Dati

### Message (Buffer Circolare 15 elementi)

```cpp
struct Message {
    char fromUser[21];      // Nickname mittente (max 20 char)
    char fromNode[16];      // Nome nodo origine (NODE_ALICE, etc)
    char text[201];         // Testo messaggio (max 200 char)
    char toUser[21];        // "" = pubblico, "nickname" = privato
    unsigned long timestamp;// Millisecondi dal boot
    bool isOwn;             // true = inviato da questo nodo
};
```

**Dimensione:** ~260 byte/messaggio  
**Totale array:** 260 × 15 = ~3.9 KB

**Logica Buffer Circolare:**
```cpp
messages[messageIndex] = nuovoMessaggio;
messageIndex = (messageIndex + 1) % MAX_MESSAGES;  // Wrap-around
if (messageCount < MAX_MESSAGES) messageCount++;
```

### LocalUser (Array 6 elementi)

```cpp
struct LocalUser {
    char nickname[21];      // Nickname utente su questo nodo
    unsigned long lastSeen; // Ultimo ping ricevuto
};
```

**Timeout:** 60 secondi senza `POST /api/ping` → rimosso

**Dimensione:** ~25 byte/utente × 6 = ~150 byte

### GlobalUser (Array 12 elementi)

```cpp
struct GlobalUser {
    char nickname[21];      // Nickname utente
    char nodeName[16];      // Nodo dove è connesso
    unsigned long lastSeen; // Ultimo broadcast ricevuto
};
```

**Timeout:** 90 secondi senza broadcast `type: "users"` → rimosso

**Dimensione:** ~37 byte/utente × 12 = ~450 byte

### NodeInfo (Array 10 elementi)

```cpp
struct NodeInfo {
    uint32_t nodeId;        // ID univoco mesh (auto-generato)
    char name[16];          // Nome nodo (NODE_ALICE, etc)
    IPAddress ip;           // IP locale (192.168.x.x)
    unsigned long lastSeen; // Ultimo heartbeat/discovery
};
```

**Timeout:** 60 secondi senza attività → rimosso

**Dimensione:** ~28 byte/nodo × 10 = ~280 byte

---

## Flussi di Comunicazione

### 1. Primo Accesso Utente

```
┌──────────┐                 ┌──────────┐
│ Browser  │                 │ ESP8266  │
└─────┬────┘                 └─────┬────┘
      │                            │
      │  WiFi: SecureMesh_PoC      │
      ├───────────────────────────►│
      │                            │
      │  GET /                     │
      ├───────────────────────────►│
      │  200 OK (HTML portale)     │
      │◄───────────────────────────┤
      │                            │
      │  Click "Apri Chat"         │
      │  GET /dashboard            │
      ├───────────────────────────►│
      │  200 OK (HTML minimal)     │
      │◄───────────────────────────┤
      │                            │
      │  GET /dash.css             │
      ├───────────────────────────►│
      │  200 OK (CSS da PROGMEM)   │
      │◄───────────────────────────┤
      │                            │
      │  GET /dash.js              │
      ├───────────────────────────►│
      │  200 OK (JS da PROGMEM)    │
      │◄───────────────────────────┤
      │                            │
      │  JS: localStorage['sm_nick']?  │
      │  No → modal nickname           │
      │                                │
      │  Utente inserisce "Mario"      │
      │  POST /api/join                │
      │  {"nickname": "Mario"}         │
      ├───────────────────────────────►│
      │                                │
      │  upsertLocalUser("Mario")      │
      │  broadcastUserList()           │
      │                                │
      │  200 OK {"ok": true}           │
      │◄───────────────────────────────┤
      │                                │
      │  localStorage.set('sm_nick', 'Mario')
      │  Avvia polling:                │
      │  - POST /api/ping ogni 20s     │
      │  - GET /api/messages ogni 2s   │
      │  - GET /api/users ogni 5s      │
      │                                │
```

### 2. Invio Messaggio Pubblico

```
Mario scrive "Ciao a tutti" in chat pubblica

┌──────────┐         ┌──────────┐         ┌──────────┐
│ Browser  │         │ NODE_A   │         │ NODE_B   │
│  Mario   │         │ (Alice)  │         │ (Bob)    │
└─────┬────┘         └─────┬────┘         └─────┬────┘
      │                    │                    │
      │ POST /api/send     │                    │
      │ {nickname:"Mario", │                    │
      │  text:"Ciao",      │                    │
      │  toUser:""}        │                    │
      ├───────────────────►│                    │
      │                    │                    │
      │  Salva in buffer:  │                    │
      │  messages[i] = {   │                    │
      │   fromUser:"Mario",│                    │
      │   text:"Ciao",     │                    │
      │   toUser:"",       │                    │
      │   isOwn:true       │                    │
      │  }                 │                    │
      │                    │                    │
      │  mesh.sendBroadcast({                   │
      │   type:"chat",     │                    │
      │   from:"NODE_A",   │                    │
      │   fu:"Mario",      │                    │
      │   t:"Ciao",        │                    │
      │   tu:""            │                    │
      │  })                │                    │
      │                    ├───────────────────►│
      │                    │                    │
      │                    │  Riceve broadcast  │
      │                    │  toUser="" (pubbl.)│
      │                    │  Salva in buffer   │
      │                    │                    │
      │ 200 OK             │                    │
      │ {success:true}     │                    │
      │◄───────────────────┤                    │
      │                    │                    │
      │ Poll /api/messages │                    │
      │ ogni 2s            │                    │
      ├───────────────────►│                    │
      │ [{fromUser:"Mario",│                    │
      │   text:"Ciao",...}]│                    │
      │◄───────────────────┤                    │
      │                    │                    │
      │ Mostra in UI       │                    │
      │                    │                    │
```

### 3. Invio Messaggio Privato

```
Mario scrive "Hey Luigi" a Luigi (su NODE_B)

┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
│ Browser  │     │ NODE_A   │     │ NODE_B   │     │ Browser  │
│  Mario   │     │          │     │          │     │  Luigi   │
└─────┬────┘     └─────┬────┘     └─────┬────┘     └─────┬────┘
      │                │                │                │
      │ POST /api/send │                │                │
      │ {nickname:"Mario",              │                │
      │  text:"Hey Luigi",              │                │
      │  toUser:"Luigi"}                │                │
      ├───────────────►│                │                │
      │                │                │                │
      │  Salva buffer: │                │                │
      │  toUser="Luigi"│                │                │
      │                │                │                │
      │  mesh.sendBroadcast({           │                │
      │   type:"chat",  │                │                │
      │   fu:"Mario",   │                │                │
      │   tu:"Luigi",   │                │                │
      │   t:"Hey Luigi" │                │                │
      │  })             │                │                │
      │                ├───────────────►│                │
      │                │  Riceve:       │                │
      │                │  toUser="Luigi"│                │
      │                │  Verifica:     │                │
      │                │  "Luigi" in    │                │
      │                │  localUsers[]? │                │
      │                │  ✅ SÌ         │                │
      │                │  Salva buffer  │                │
      │                │                │                │
      │                │                │ Poll /api/messages
      │                │                │◄───────────────┤
      │                │ Filtra:        │                │
      │                │ (fromUser="Mario" && toUser="Luigi")
      │                │                ├───────────────►│
      │                │                │ Mostra msg     │
      │                │                │                │
```

**Nodo NON destinatario ignora messaggio:**
```
NODE_C riceve broadcast:
- toUser = "Luigi"
- "Luigi" in localUsers[]? ❌ NO
- Ignora (non salva)
```

### 4. Broadcast Lista Utenti

```
Ogni 15 secondi ogni nodo manda lista utenti locali

┌──────────┐              ┌──────────┐
│ NODE_A   │              │ NODE_B   │
│ Users:   │              │ Users:   │
│ - Mario  │              │ - Luigi  │
│ - Anna   │              │          │
└─────┬────┘              └─────┬────┘
      │                         │
      │  broadcastUserList()    │
      │  ogni 15s               │
      │                         │
      │  mesh.sendBroadcast({   │
      │   type: "users",        │
      │   node: "NODE_A",       │
      │   u: ["Mario", "Anna"]  │
      │  })                     │
      ├────────────────────────►│
      │                         │
      │                    upsertGlobalUser("Mario", "NODE_A")
      │                    upsertGlobalUser("Anna", "NODE_A")
      │                         │
      │◄────────────────────────┤
      │   {type: "users",       │
      │    node: "NODE_B",      │
      │    u: ["Luigi"]}        │
      │                         │
      │  upsertGlobalUser("Luigi", "NODE_B")
      │                         │
```

---

## API REST

### Portale

#### `GET /`
Portale con lista nodi e statistiche.

**Response:** HTML
```html
<div class='card me'>
  <div class='name'>NODE_ALICE <span class='badge'>TU SEI QUI</span></div>
  <div class='detail'>ID: 2919469548</div>
  <a href='/dashboard' class='btn'>Apri Chat</a>
</div>
```

#### `GET /dashboard`
Dashboard chat (HTML minimal ~600B).

**Response:** HTML con link esterni
```html
<link rel='stylesheet' href='/dash.css'>
<script src='/dash.js'></script>
```

#### `GET /dash.css`
CSS servito da PROGMEM (Flash memory).

**Response:** `text/css` (~2KB, zero heap)

#### `GET /dash.js`
JavaScript servito da PROGMEM (Flash memory).

**Response:** `application/javascript` (~2.5KB, zero heap)

### API Utenti

#### `POST /api/join`
Registra nickname su nodo corrente.

**Request:**
```json
{
  "nickname": "Mario"
}
```

**Response:**
```json
{
  "ok": true
}
```

**Side effects:**
- Aggiunge/aggiorna `LocalUser`
- Chiama `broadcastUserList()` → mesh
- Errore 400 se nickname < 2 o > 20 caratteri

#### `POST /api/ping`
Keepalive per mantenere utente attivo.

**Request:**
```json
{
  "nickname": "Mario"
}
```

**Response:**
```json
{
  "ok": true
}
```

**Side effects:**
- Aggiorna `lastSeen` in `LocalUser`
- Se 60s senza ping → rimosso da `pruneInactiveUsers()`

**Frequenza client:** ogni 20 secondi

#### `GET /api/users`
Lista tutti gli utenti (locali + remoti).

**Response:**
```json
{
  "nodes": 2,
  "users": [
    {
      "nickname": "Mario",
      "node": "NODE_ALICE"
    },
    {
      "nickname": "Luigi",
      "node": "NODE_BOB"
    }
  ]
}
```

**Frequenza polling client:** ogni 5 secondi

### API Messaggi

#### `GET /api/messages`
Recupera tutti i messaggi nel buffer.

**Response:**
```json
{
  "messages": [
    {
      "fromUser": "Mario",
      "fromNode": "NODE_ALICE",
      "text": "Ciao!",
      "toUser": "",
      "time": "00:05:23",
      "isOwn": true
    },
    {
      "fromUser": "Luigi",
      "fromNode": "NODE_BOB",
      "text": "Hey",
      "toUser": "Mario",
      "time": "00:05:45",
      "isOwn": false
    }
  ]
}
```

**Frequenza polling client:** ogni 2 secondi

#### `POST /api/send`
Invia messaggio pubblico o privato.

**Request (pubblico):**
```json
{
  "nickname": "Mario",
  "text": "Ciao a tutti!",
  "toUser": ""
}
```

**Request (privato):**
```json
{
  "nickname": "Mario",
  "text": "Messaggio privato",
  "toUser": "Luigi"
}
```

**Response:**
```json
{
  "success": true
}
```

**Side effects:**
- Salva in `messages[]` buffer locale
- `mesh.sendBroadcast()` a tutta la rete
- LED flash (2× pubblico, 3× privato)
- Errore 400 se testo vuoto o > 200 caratteri

### API Stats

#### `GET /api/stats`
Statistiche rete mesh (per portale).

**Response:**
```json
{
  "nodes": 2,
  "total": 5,
  "users": 3
}
```

---

## Gestione Memoria

### Layout RAM ESP8266 (80KB totali)

```
┌─────────────────────────────────────┐ 0x3FFF0000
│  Stack (8KB)                        │
├─────────────────────────────────────┤
│  Heap (dinamico)                    │
│  └── Variabili runtime ~18KB        │ ◄── Qui viviamo
│      - WiFi buffers                 │
│      - Mesh networking              │
│      - HTTP requests/responses      │
├─────────────────────────────────────┤
│  BSS/Data (globali)                 │ ◄── Nostre struct
│  └── messages[15]      ~3.9KB       │
│  └── localUsers[6]     ~150B        │
│  └── globalUsers[12]   ~450B        │
│  └── knownNodes[10]    ~280B        │
│  └── Altri globals     ~500B        │
│  Totale:               ~5.3KB       │
├─────────────────────────────────────┤
│  IRAM (istruzioni)                  │
└─────────────────────────────────────┘ 0x40100000
```

**Heap libera tipica:** ~21KB dopo boot

### Ottimizzazioni Memoria v3.1

**1. CSS/JS in PROGMEM (Flash memory)**

Prima (v3.0):
```cpp
String html = "<style>";
html += "body{font-family:...}";  // 2KB+ in heap ❌
```

Dopo (v3.1):
```cpp
static const char DASH_CSS[] PROGMEM = R"css(
body{font-family:...}
)css";

// Server con beginResponse_P (P = PROGMEM)
server.on("/dash.css", [](AsyncWebServerRequest *r){
    r->beginResponse_P(200, "text/css", DASH_CSS);
});
```

**Risparmio:** ~4.5KB heap (CSS + JS)

**2. `char[]` invece di `String`**

Prima:
```cpp
struct Message {
    String fromUser;  // heap allocation ❌
    String text;      // heap allocation ❌
};
```

Dopo:
```cpp
struct Message {
    char fromUser[21];  // stack/BSS ✅
    char text[201];     // stack/BSS ✅
};
```

**Risparmio:** ~1.5KB heap per 15 messaggi

**3. Buffer ridimensionati**

| Versione | Messages | LocalUsers | GlobalUsers | RAM libera |
|----------|----------|------------|-------------|------------|
| v2.0     | 30       | 8          | 20          | ~27KB      |
| v3.0     | 30       | 8          | 20          | ~7KB ❌    |
| v3.1     | 15       | 6          | 12          | ~21KB ✅   |

**4. JSON compatto su mesh**

Prima:
```json
{
  "type": "chat",
  "fromUser": "Mario",
  "text": "Hello",
  "toUser": "Luigi"
}
```

Dopo:
```json
{
  "type": "chat",
  "fu": "Mario",
  "t": "Hello",
  "tu": "Luigi"
}
```

**Risparmio:** ~20% dimensione payload mesh

---

## Protocollo Mesh

### Messaggi painlessMesh

Tutti i messaggi mesh sono JSON broadcast (UDP-like).

#### 1. Chat Message

```json
{
  "type": "chat",
  "from": "NODE_ALICE",
  "fu": "Mario",
  "t": "Ciao!",
  "tu": ""
}
```

**Filtro ricezione:**
```cpp
if (toUser != "") {
    // Messaggio privato: verifica se destinatario è locale
    bool isForMe = false;
    for (LocalUser& u : localUsers) {
        if (strcmp(u.nickname, toUser) == 0) {
            isForMe = true;
            break;
        }
    }
    if (!isForMe) return;  // Ignora
}
// Salva nel buffer
```

#### 2. Users Broadcast

```json
{
  "type": "users",
  "node": "NODE_ALICE",
  "u": ["Mario", "Anna", "Giuseppe"]
}
```

**Frequenza:** ogni 15 secondi  
**Scopo:** sincronizzare lista utenti globali

#### 3. Info Request (Discovery)

```json
{
  "type": "info_request",
  "from": "NODE_ALICE"
}
```

**Frequenza:** ogni 10 secondi  
**Scopo:** trovare nuovi nodi e ottenere IP

#### 4. Info Response

```json
{
  "type": "info_response",
  "from": "NODE_BOB",
  "nodeId": 2919469549,
  "ip": "192.168.1.46"
}
```

**Scopo:** popolare `knownNodes[]` cache

#### 5. Heartbeat

```json
{
  "type": "heartbeat",
  "from": "NODE_ALICE"
}
```

**Frequenza:** ogni 30 secondi  
**Scopo:** keepalive nodi (evita timeout cache)

---

## Performance

### Latenza

| Operazione | Latenza | Note |
|------------|---------|------|
| Messaggio pubblico (stesso nodo) | 50-100ms | HTTP POST + salvataggio |
| Messaggio pubblico (nodo remoto) | 150-400ms | + broadcast mesh |
| Messaggio privato (stesso nodo) | 50-100ms | Come pubblico |
| Messaggio privato (nodo remoto) | 150-400ms | + broadcast + filtro |
| Poll /api/messages | 20-50ms | Lettura buffer locale |
| Poll /api/users | 30-60ms | Merge locale + globale |

### Throughput

| Metrica | Valore | Limite |
|---------|--------|--------|
| Messaggi/secondo (stesso nodo) | ~10 | Limitato da client polling |
| Messaggi/secondo (mesh) | ~5 | Limitato da broadcast mesh |
| Utenti simultanei per nodo | 6 | `MAX_LOCAL_USERS` |
| Utenti totali rete mesh | 12 | `MAX_GLOBAL_USERS` |
| Nodi simultanei | 10+ | Testato 4, teorico >10 |

### Memoria

| Componente | RAM (byte) | Flash (byte) |
|------------|------------|--------------|
| Firmware core | ~3KB | 390KB |
| Struct dati | ~5KB | - |
| WiFi stack | ~15KB | - |
| Heap libera | ~21KB | - |
| **Totale usato** | **~59KB/80KB** | **390KB/1044KB** |

### Uptime

- **Test 24h:** ✅ Nessun crash, RAM stabile
- **Test 48h:** ✅ Nessun memory leak rilevato
- **Max messaggi:** 100+ senza degrado performance

---

## Sicurezza

### Attuale (v3.1)

⚠️ **NESSUNA CIFRATURA** - PoC non production-ready

- Messaggi mesh: plaintext
- WiFi: WPA2-PSK (`mesh2026secure`)
- Nessuna autenticazione utente
- Nessuna validazione input lato server (XSS possibile)

### Roadmap Sicurezza (v4.0+)

- [ ] Cifratura E2E messaggi privati (AES-256)
- [ ] HMAC per integrità messaggi mesh
- [ ] Nonce anti-replay attack
- [ ] Sanitizzazione HTML input utente
- [ ] Rate limiting API
- [ ] Password room (opzionale)

---

## Troubleshooting Tecnico

### Debug RAM

```cpp
// In loop(), ogni 15s
Serial.printf("[%s] RAM:%uK Heap:%u Frag:%u%%\n",
    NODE_NAME,
    ESP.getFreeHeap() / 1024,
    ESP.getFreeHeap(),
    ESP.getHeapFragmentation());
```

**Output normale:**
```
[NODE_ALICE] RAM:21K Heap:21504 Frag:5%
```

**Problemi:**
- `RAM < 15K` → leak o buffer troppo grandi
- `Frag > 30%` → troppe alloc/free (usa char[] invece String)

### Debug Mesh

```cpp
mesh.setDebugMsgTypes(ERROR | CONNECTION | COMMUNICATION);
```

**Output:**
```
myAP: STATION_CONNECTED (MAC: 24:0a:c4:12:34:56)
New connection from 2919469549
receivedCallback from 2919469549: {"type":"chat"...}
```

### Dump Struct

```cpp
void dumpMessages() {
    Serial.println("=== MESSAGES ===");
    for (int i = 0; i < messageCount; i++) {
        int idx = (messageIndex - messageCount + i + MAX_MESSAGES) % MAX_MESSAGES;
        Serial.printf("%d. %s@%s → %s: \"%s\"\n",
            i, messages[idx].fromUser, messages[idx].fromNode,
            messages[idx].toUser[0] ? messages[idx].toUser : "ALL",
            messages[idx].text);
    }
}
```

---

**Documentazione completa! Per domande tecniche apri una issue su GitHub.**
