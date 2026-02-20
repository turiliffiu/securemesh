# 🚀 Comandi Git per Primo Commit GitHub

# ============================================
# PREPARAZIONE REPOSITORY LOCALE
# ============================================

# 1. Vai nella directory del progetto
cd ~/esp-rete-mesh

# 2. Copia tutti i file documentazione scaricati
# (assumendo che siano in ~/Downloads/)

# README principale
cp ~/Downloads/README.md ./

# Guide installazione e architettura
cp ~/Downloads/INSTALL.md ./
cp ~/Downloads/ARCHITECTURE.md ./

# Changelog e licenza
cp ~/Downloads/CHANGELOG.md ./
cp ~/Downloads/LICENSE ./

# Setup script
cp ~/Downloads/setup.sh ./
chmod +x setup.sh

# Aggiorna .gitignore (fai backup prima)
cp .gitignore .gitignore.bak
cp ~/Downloads/.gitignore ./

# 3. Verifica file copiati
ls -la *.md *.sh LICENSE .gitignore

# Output atteso:
# README.md
# INSTALL.md
# ARCHITECTURE.md
# CHANGELOG.md
# LICENSE
# setup.sh
# .gitignore

# ============================================
# INIZIALIZZAZIONE GIT
# ============================================

# 4. Inizializza repository Git
git init

# 5. Configura identità Git (se non già fatto)
git config user.name "Salvo"
git config user.email "tuo.email@example.com"

# Verifica config
git config --list | grep user

# ============================================
# PRIMO COMMIT LOCALE
# ============================================

# 6. Aggiungi tutti i file
git add .

# 7. Verifica cosa verrà committato
git status

# Output atteso:
# Changes to be committed:
#   new file:   README.md
#   new file:   INSTALL.md
#   new file:   ARCHITECTURE.md
#   new file:   CHANGELOG.md
#   new file:   LICENSE
#   new file:   .gitignore
#   new file:   setup.sh
#   new file:   platformio.ini
#   new file:   src/main.cpp
#   new file:   funzionanti/firmware_v3.1_ram_fix.cpp
#   new file:   funzionanti/main_STABLE_v2.0_GOLDEN.cpp
#   ...

# NOTA: .pio/ e venv/ NON devono apparire (sono in .gitignore)

# 8. Primo commit
git commit -m "Initial commit: SecureMesh v3.1 - Rete mesh ESP8266 con chat nickname"

# 9. Verifica commit
git log --oneline

# Output atteso:
# a1b2c3d (HEAD -> master) Initial commit: SecureMesh v3.1...

# ============================================
# CREAZIONE REPOSITORY GITHUB
# ============================================

# 10. Vai su GitHub (browser)
#     https://github.com/new

# 11. Compila form:
#     Repository name: securemesh
#     Description: Rete mesh decentralizzata con chat multi-utente su ESP8266
#     Public/Private: [tua scelta]
#     ❌ NON selezionare "Initialize with README" (ce l'hai già!)
#     ❌ NON selezionare .gitignore (ce l'hai già!)
#     ❌ NON selezionare licenza (ce l'hai già!)

# 12. Click "Create repository"

# ============================================
# CONNESSIONE A GITHUB
# ============================================

# 13. Aggiungi remote (sostituisci TUO_USERNAME)
git remote add origin https://github.com/TUO_USERNAME/securemesh.git

# Verifica remote
git remote -v

# Output atteso:
# origin  https://github.com/TUO_USERNAME/securemesh.git (fetch)
# origin  https://github.com/TUO_USERNAME/securemesh.git (push)

# 14. Rinomina branch master -> main (opzionale, GitHub standard)
git branch -M main

# ============================================
# PUSH SU GITHUB
# ============================================

# 15. Push iniziale
git push -u origin main

# Ti chiederà username e password GitHub
# ⚠️ IMPORTANTE: usa Personal Access Token, non password account!
#
# Come creare token:
# 1. GitHub → Settings → Developer settings → Personal access tokens
# 2. Generate new token (classic)
# 3. Seleziona scope: repo (full control)
# 4. Copy token (salvalo in password manager!)

# Username: TUO_USERNAME
# Password: ghp_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX (token)

# Output atteso:
# Enumerating objects: 25, done.
# Counting objects: 100% (25/25), done.
# Delta compression using up to 4 threads
# Compressing objects: 100% (20/20), done.
# Writing objects: 100% (25/25), 45.67 KiB | 2.28 MiB/s, done.
# Total 25 (delta 3), reused 0 (delta 0)
# To https://github.com/TUO_USERNAME/securemesh.git
#  * [new branch]      main -> main
# Branch 'main' set up to track remote branch 'main' from 'origin'.

# ============================================
# VERIFICA SU GITHUB
# ============================================

# 16. Apri browser
#     https://github.com/TUO_USERNAME/securemesh

# Dovresti vedere:
# ✅ README.md visualizzato nella homepage
# ✅ File tree: src/, funzionanti/, docs
# ✅ Badge "MIT License"
# ✅ setup.sh con icona "executable"

# ============================================
# COMANDI UTILI POST-SETUP
# ============================================

# Verifica stato repository
git status

# Vedi cronologia commit
git log --oneline --graph --all

# Verifica branch
git branch -a

# Pull (sincronizza da GitHub)
git pull origin main

# ============================================
# WORKFLOW SVILUPPO FUTURO
# ============================================

# Per ogni modifica:

# 1. Modifica file
vim src/main.cpp

# 2. Verifica modifiche
git status
git diff

# 3. Aggiungi modifiche
git add src/main.cpp
# oppure tutto:
git add .

# 4. Commit con messaggio descrittivo
git commit -m "Fix: risolto bug chat privata badge unread"

# 5. Push su GitHub
git push origin main

# ============================================
# BRANCH PER FEATURE
# ============================================

# Per feature complesse, crea branch:

# Crea e switcha a nuovo branch
git checkout -b feature/emoji-picker

# Lavora sulla feature
# ... modifiche ...

# Commit sulla feature
git add .
git commit -m "Add: emoji picker integrato in chat"

# Push branch su GitHub
git push -u origin feature/emoji-picker

# Su GitHub: crea Pull Request
# Dopo merge: torna a main
git checkout main
git pull origin main

# Cancella branch locale (se merged)
git branch -d feature/emoji-picker

# ============================================
# TAGS PER RELEASE
# ============================================

# Crea tag per release
git tag -a v3.1.0 -m "Release v3.1.0 - RAM fix & chat privata"

# Push tag su GitHub
git push origin v3.1.0

# Lista tag
git tag -l

# ============================================
# TROUBLESHOOTING
# ============================================

# Errore: "fatal: remote origin already exists"
git remote remove origin
git remote add origin https://github.com/TUO_USERNAME/securemesh.git

# Errore: "Authentication failed"
# → Usa Personal Access Token, non password

# Errore: file troppo grande (>100MB)
# → Aggiungi a .gitignore e rimuovi da staging
git rm --cached file_grande.bin

# Vedere cosa verrà pushato
git log origin/main..HEAD

# Annullare ultimo commit (locale, prima di push)
git reset --soft HEAD~1

# Annullare modifiche non committate
git restore src/main.cpp

# ============================================
# FINE
# ============================================

echo "✅ Repository GitHub configurato!"
echo "🔗 https://github.com/TUO_USERNAME/securemesh"
