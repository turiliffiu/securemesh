# 📝 Changelog

Tutte le modifiche rilevanti al progetto SecureMesh.

Il formato è basato su [Keep a Changelog](https://keepachangelog.com/it/1.0.0/),
e questo progetto aderisce al [Semantic Versioning](https://semver.org/).

---

## [3.1.0] - 2026-02-17

### 🎉 Release Stabile - RAM Fix

**GOLDEN VERSION** - Punto di partenza per future feature.

### ✅ Aggiunto
- **CSS/JS in PROGMEM**: serviti come file separati `/dash.css` e `/dash.js`
  - Zero occupazione heap durante rendering
  - Cache-Control headers per performance browser
- **Fix chat privata utente-to-utente**
  - Logica filtro corretta: `fromUser===nickname` invece di `isOwn`
  - Messaggi privati ora visibili correttamente al destinatario

### 🔧 Modificato
- **Dashboard HTML ridotta a ~600 byte** (era ~5KB con CSS inline)
- **Gestione memoria ottimizzata:**
  - Buffer messaggi: 30 → 15
  - LocalUsers: 8 → 6
  - GlobalUsers: 20 → 12

### 🐛 Risolto
- **CRITICO:** RAM insufficiente (7KB → 21KB liberi)
  - Causa: costruzione stringa HTML troppo grande su heap
  - Fix: CSS/JS serviti direttamente da Flash memory
- **CRITICO:** CSS mostrato come testo corrotto
  - Causa: heap overflow durante `html += FPSTR(DASH_CSS)`
  - Fix: route separate `/dash.css` con `beginResponse_P()`
- **Chat privata non funzionante:**
  - Mittente vedeva messaggio, destinatario no
  - Fix: logica filtro JS basata su `fromUser` invece di `isOwn`

### 📊 Performance
- RAM libera: **21KB** (vs 7KB in v3.0)
- Flash: **390KB** (37% utilizzo)
- Latenza mesh: **100-400ms** (invariata)
- Uptime testato: **48+ ore** senza crash

---

## [3.0.0] - 2026-02-17

### 🎉 Major Release - Sistema Nickname

**BREAKING CHANGE:** Rimosso sistema room/canali, introdotti nickname utente.

### ✅ Aggiunto
- **Sistema nickname utente:**
  - Modal all'apertura dashboard per scelta nickname
  - LocalStorage salva nickname tra sessioni
  - Validazione: 2-20 caratteri alfanumerici + `_`, `-`, `.`
- **Chat pubblica + privata 1-to-1:**
  - Chat pubblica visibile a tutti gli utenti
  - Chat privata per nickname specifico (non più nodo-to-nodo)
  - Badge messaggi non letti per chat e utenti
- **Sidebar utenti:**
  - Lista live utenti online (locali + remoti)
  - Indicatore stato (pallino verde)
  - Click utente → switch a chat privata
- **API nuove:**
  - `POST /api/join` - Registra nickname
  - `POST /api/ping` - Keepalive (ogni 20s)
  - `GET /api/users` - Lista utenti mesh
- **Gestione utenti:**
  - `LocalUser[]` - utenti su questo nodo (max 6)
  - `GlobalUser[]` - utenti rete mesh (max 12)
  - Broadcast lista utenti ogni 15s via mesh
  - Timeout automatico utenti inattivi (60s locale, 90s remoto)

### ❌ Rimosso
- **Sistema room/canali:**
  - Canali broadcast/generale/lavoro/random
  - Campo `channel` in Message struct
  - Campo `toNodeId` (sostituito da `toUser`)
  - Sidebar canali
  - Switch canale

### 🔧 Modificato
- **Struct Message:**
  - Aggiunto: `fromUser` (nickname mittente)
  - Aggiunto: `fromNode` (nome nodo origine)
  - Rimosso: `channel`, `toNodeId`
  - `toUser`: `""` = pubblico, `"nickname"` = privato
- **Mesh protocol:**
  - Messaggi includono `fromUser` e `toUser`
  - Nuovo messaggio `type: "users"` con lista nickname
  - Filtro ricezione: privati solo se destinatario locale

### ⚠️ Problemi Noti
- RAM critica: solo 7KB liberi (risolto in v3.1)
- CSS corrotto su alcuni device (risolto in v3.1)

---

## [2.0.0] - 2026-02-16

### 🎉 Release GOLD - Multi-Room Funzionante

**GOLDEN BASELINE** - Salvato come `main_STABLE_v2.0_GOLDEN.cpp`

### ✅ Aggiunto
- **Sistema multi-room (4 canali):**
  - Broadcast (tutti i nodi)
  - Generale (conversazione libera)
  - Lavoro (coordinamento team)
  - Random (off-topic)
- **Chat privata nodo-a-nodo:**
  - Selezione nodo destinatario
  - Messaggi cifrati con badge 🔒
  - LED feedback 3 flash
- **Portale migliorato:**
  - Lista nodi con IP
  - Badge "TU SEI QUI"
  - Auto-refresh ogni 15s
  - Statistiche real-time
- **Captive portal completo:**
  - DNS wildcard `*` → 192.168.4.1
  - Redirect detection: `/generate_204`, `/hotspot-detect.html`
  - Compatibilità Android/iOS/Windows

### 🔧 Modificato
- Dashboard con sidebar canali
- Filtro messaggi per canale attivo
- Badge unread per canale
- Cache nodi con `NodeInfo[]` struct

### 📊 Performance
- RAM libera: **27KB**
- Flash: **385KB** (37%)
- Supporto 4 nodi testato
- Uptime: 24+ ore

---

## [1.0.0] - 2026-02-15

### 🎉 Release Iniziale - PoC Funzionante

### ✅ Aggiunto
- **Rete mesh base:**
  - painlessMesh networking
  - Auto-discovery nodi
  - Broadcast messaggi
- **Web dashboard:**
  - AsyncWebServer HTTP
  - Chat UI basic
  - Monitor seriale integrato
- **API REST:**
  - `GET /api/messages`
  - `POST /api/send`
  - `GET /api/stats`
- **Captive portal:**
  - DNSServer basic
  - Redirect automatico

### ⚠️ Limitazioni
- Chat globale unica (no room)
- No gestione utenti (solo nodi)
- RAM: ~25KB liberi
- Bug: portale non sempre auto-apre

---

## [Unreleased]

### 🚧 In Sviluppo

#### v3.2 - Prossima Release
- [ ] Emoji picker integrato
- [ ] Timestamp assoluto (NTP)
- [ ] Notifica sonora nuovi messaggi
- [ ] Tema scuro

#### v4.0 - Sicurezza & Gruppi
- [ ] Cifratura E2E messaggi privati
- [ ] Gruppi custom oltre chat pubblica
- [ ] Storage persistente SPIFFS
- [ ] File transfer (immagini)

#### v5.0 - Advanced Features
- [ ] Voice messages
- [ ] Network topology viz
- [ ] LoRa mesh support
- [ ] Multi-hop routing

---

## Legenda

- **✅ Aggiunto** - Nuove feature
- **🔧 Modificato** - Modifiche a feature esistenti
- **🐛 Risolto** - Bug fix
- **❌ Rimosso** - Feature deprecate
- **⚠️ Deprecated** - Feature che verranno rimosse
- **🔒 Sicurezza** - Vulnerabilità risolte
- **📊 Performance** - Miglioramenti performance

---

## Versioning

Questo progetto usa [SemVer](https://semver.org/):
- **MAJOR** - Breaking changes (es. 2.0 → 3.0)
- **MINOR** - Nuove feature backward-compatible (es. 3.0 → 3.1)
- **PATCH** - Bug fix (es. 3.1.0 → 3.1.1)

---

**Versione corrente:** v3.1.0  
**Data release:** 2026-02-17  
**Autore:** Salvo (turiliffiu)
