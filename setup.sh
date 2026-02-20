#!/bin/bash

# SecureMesh - Setup Automatico
# Configurazione ambiente sviluppo ESP8266

set -e  # Exit on error

# Colori output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔══════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  🚀 SecureMesh - Setup Automatico       ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════╝${NC}"
echo ""

# ============================================
# 1. Verifica Python 3
# ============================================

echo -e "${BLUE}[1/5]${NC} Verifica Python 3..."

if ! command -v python3 &> /dev/null; then
    echo -e "${RED}❌ Python 3 non trovato!${NC}"
    echo -e "   Installa con: ${YELLOW}sudo apt install python3 python3-venv${NC}"
    exit 1
fi

PYTHON_VERSION=$(python3 --version | cut -d' ' -f2)
echo -e "${GREEN}✅ Python $PYTHON_VERSION${NC}"

# ============================================
# 2. Virtual Environment
# ============================================

echo -e "\n${BLUE}[2/5]${NC} Setup virtual environment..."

VENV_DIR="venv"

if [ -d "$VENV_DIR" ]; then
    echo -e "${GREEN}✅ Virtual environment trovato: $VENV_DIR/${NC}"
else
    echo -e "${YELLOW}⚙️  Creazione virtual environment...${NC}"
    python3 -m venv $VENV_DIR
    echo -e "${GREEN}✅ Virtual environment creato${NC}"
fi

# Attiva virtual environment
source $VENV_DIR/bin/activate

# Verifica attivazione
if [ -z "$VIRTUAL_ENV" ]; then
    echo -e "${RED}❌ Errore attivazione virtual environment${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Virtual environment attivo: $VIRTUAL_ENV${NC}"

# ============================================
# 3. Upgrade pip
# ============================================

echo -e "\n${BLUE}[3/5]${NC} Aggiornamento pip..."

pip install --upgrade pip > /dev/null 2>&1
PIP_VERSION=$(pip --version | cut -d' ' -f2)
echo -e "${GREEN}✅ pip $PIP_VERSION${NC}"

# ============================================
# 4. Installa PlatformIO
# ============================================

echo -e "\n${BLUE}[4/5]${NC} Verifica PlatformIO..."

if command -v pio &> /dev/null; then
    PIO_VERSION=$(pio --version | grep -oP '\d+\.\d+\.\d+' | head -1)
    echo -e "${GREEN}✅ PlatformIO $PIO_VERSION già installato${NC}"
else
    echo -e "${YELLOW}⚙️  Installazione PlatformIO...${NC}"
    pip install platformio
    PIO_VERSION=$(pio --version | grep -oP '\d+\.\d+\.\d+' | head -1)
    echo -e "${GREEN}✅ PlatformIO $PIO_VERSION installato${NC}"
fi

# ============================================
# 5. Verifica Librerie
# ============================================

echo -e "\n${BLUE}[5/5]${NC} Verifica librerie progetto..."

# Controlla se platformio.ini esiste
if [ ! -f "platformio.ini" ]; then
    echo -e "${RED}❌ platformio.ini non trovato!${NC}"
    echo -e "   Assicurati di essere nella directory del progetto"
    exit 1
fi

# Installa/aggiorna dipendenze
echo -e "${YELLOW}⚙️  Installazione dipendenze...${NC}"
pio pkg install > /dev/null 2>&1

# Verifica librerie installate
LIBS_INSTALLED=$(pio pkg list | grep -c "painlessMesh\|ArduinoJson\|ESPAsyncTCP\|ESPAsyncWebServer" || true)

if [ "$LIBS_INSTALLED" -eq 4 ]; then
    echo -e "${GREEN}✅ Tutte le librerie verificate:${NC}"
    echo -e "   - painlessMesh"
    echo -e "   - ArduinoJson"
    echo -e "   - ESPAsyncTCP"
    echo -e "   - ESPAsyncWebServer"
else
    echo -e "${YELLOW}⚠️  Alcune librerie potrebbero mancare${NC}"
    echo -e "   Esegui manualmente: ${YELLOW}pio pkg install${NC}"
fi

# ============================================
# Setup Completato
# ============================================

echo ""
echo -e "${GREEN}╔══════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║  ✅ Setup completato con successo!       ║${NC}"
echo -e "${GREEN}╚══════════════════════════════════════════╝${NC}"
echo ""

echo -e "${BLUE}📋 Informazioni ambiente:${NC}"
echo -e "   Python:      $PYTHON_VERSION"
echo -e "   pip:         $PIP_VERSION"
echo -e "   PlatformIO:  $PIO_VERSION"
echo -e "   VirtualEnv:  $VIRTUAL_ENV"
echo ""

echo -e "${BLUE}🎯 Prossimi passi:${NC}"
echo ""
echo -e "1️⃣  ${YELLOW}Attiva virtual environment${NC} (se non già attivo):"
echo -e "   ${BLUE}source venv/bin/activate${NC}"
echo ""
echo -e "2️⃣  ${YELLOW}Connetti ESP8266${NC} via USB"
echo ""
echo -e "3️⃣  ${YELLOW}Build firmware${NC}:"
echo -e "   ${BLUE}pio run -e node1${NC}"
echo ""
echo -e "4️⃣  ${YELLOW}Upload su ESP8266${NC}:"
echo -e "   ${BLUE}pio run -e node1 --target upload${NC}"
echo ""
echo -e "   Oppure specifica porta:"
echo -e "   ${BLUE}pio run -e node1 --target upload --upload-port /dev/ttyUSB0${NC}"
echo ""
echo -e "5️⃣  ${YELLOW}Monitor seriale${NC}:"
echo -e "   ${BLUE}pio device monitor --baud 115200${NC}"
echo ""

echo -e "${BLUE}📚 Documentazione:${NC}"
echo -e "   README.md        - Panoramica progetto"
echo -e "   INSTALL.md       - Guida installazione dettagliata"
echo -e "   ARCHITECTURE.md  - Documentazione tecnica"
echo ""

echo -e "${GREEN}✨ Buon sviluppo con SecureMesh!${NC}"
echo ""

# Rimani nel virtual environment
exec $SHELL
