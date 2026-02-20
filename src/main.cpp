/*
 * SecureMesh v3.1 - NICKNAME + CHAT (RAM FIX)
 * Fix: CSS/JS in PROGMEM, HTML minificato, strutture ridotte
 * Base: firmware GOLD funzionante
 */

#include <Arduino.h>
#include "painlessMesh.h"
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <DNSServer.h>

#define MESH_PREFIX      "SecureMesh_PoC"
#define MESH_PASSWORD    "mesh2026secure"
#define MESH_PORT        5555

#ifndef NODE_NAME
#define NODE_NAME        "NODE_DEFAULT"
#endif

#define LED_PIN          D4
#define MAX_MESSAGES     15     // ridotto da 30
#define MAX_LOCAL_USERS  6      // ridotto da 8
#define MAX_GLOBAL_USERS 12     // ridotto da 20
#define DNS_PORT         53

Scheduler userScheduler;
painlessMesh mesh;
AsyncWebServer server(80);
DNSServer dnsServer;

uint32_t messagesSent = 0;
uint32_t messagesReceived = 0;

// ============================================
// CSS + JS IN PROGMEM (non occupa RAM heap)
// ============================================

// CSS dashboard (statico, va in Flash)
static const char DASH_CSS[] PROGMEM = R"css(
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);height:100vh;overflow:hidden}
.app{display:flex;height:100vh;max-width:960px;margin:0 auto;background:#fff}
.sidebar{width:200px;background:#2c3e50;color:#fff;display:flex;flex-direction:column;flex-shrink:0}
.sb-head{padding:12px;background:#1a252f;border-bottom:1px solid #34495e}
.sb-title{font-size:.9em;font-weight:700}
.sb-nick{font-size:.75em;color:#10b981;margin-top:3px}
.sb-sec{padding:8px 12px 4px;font-size:.6em;text-transform:uppercase;opacity:.6;letter-spacing:1px}
.cb{padding:7px 10px;border-radius:6px;cursor:pointer;margin:2px 6px;display:flex;align-items:center;gap:7px;font-size:.82em}
.cb:hover{background:#34495e}
.cb.active{background:#667eea}
.cb .dot{width:7px;height:7px;border-radius:50%;background:#10b981;flex-shrink:0}
.cb .bx{margin-left:auto;background:#e74c3c;color:#fff;font-size:.6em;padding:1px 5px;border-radius:8px;min-width:16px;text-align:center;display:none}
.main{flex:1;display:flex;flex-direction:column;min-width:0}
.hdr{background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;padding:10px 14px;display:flex;justify-content:space-between;align-items:center;box-shadow:0 2px 8px rgba(0,0,0,.2)}
.bk{background:rgba(255,255,255,.2);color:#fff;text-decoration:none;padding:5px 11px;border-radius:7px;font-size:.82em;border:none;cursor:pointer}
.hi{text-align:center;flex:1}
.ht{font-size:.95em;font-weight:600}
.hs{font-size:.68em;opacity:.85;margin-top:1px}
.hst{display:flex;gap:10px}
.sv{font-size:1.1em;font-weight:700}
.sl{font-size:.6em;opacity:.9;text-transform:uppercase}
.msgs{flex:1;padding:10px;background:#f5f5f5;overflow-y:auto}
.msg{margin-bottom:9px;animation:si .25s}
@keyframes si{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:translateY(0)}}
.bbl{display:inline-block;max-width:76%;padding:8px 12px;border-radius:13px;box-shadow:0 1px 3px rgba(0,0,0,.1);word-wrap:break-word}
.rx .bbl{background:#fff;border-bottom-left-radius:3px}
.tx{text-align:right}
.tx .bbl{background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;border-bottom-right-radius:3px}
.fn{font-weight:600;font-size:.76em;margin-bottom:3px;color:#667eea}
.tx .fn{color:rgba(255,255,255,.9)}
.ft{font-size:.88em;line-height:1.4}
.fm{font-size:.66em;margin-top:3px;opacity:.7;display:flex;align-items:center;gap:4px}
.tx .fm{justify-content:flex-end}
.pt{background:rgba(231,76,60,.15);color:#e74c3c;padding:1px 5px;border-radius:4px;font-weight:600;font-size:.88em}
.tx .pt{background:rgba(255,255,255,.2);color:#fff}
.empty{text-align:center;padding:30px;color:#999;font-style:italic}
.inp{padding:10px 13px;background:#fff;border-top:1px solid #e0e0e0;display:flex;gap:8px;align-items:center}
#mi{flex:1;padding:9px 15px;border:2px solid #e0e0e0;border-radius:20px;outline:none;font-size:.88em}
#mi:focus{border-color:#667eea}
#sb{width:40px;height:40px;border-radius:50%;background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;border:none;cursor:pointer;font-size:1.1em}
#sb:active{transform:scale(.95)}
.mo{position:fixed;inset:0;background:rgba(0,0,0,.6);display:flex;align-items:center;justify-content:center;z-index:999}
.md{background:#fff;border-radius:18px;padding:35px;max-width:360px;width:90%;text-align:center}
.md h2{font-size:1.7em;margin-bottom:8px;background:linear-gradient(135deg,#667eea,#764ba2);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.md p{color:#666;margin-bottom:22px;font-size:.92em}
.md input{width:100%;padding:12px 16px;border:2px solid #e0e0e0;border-radius:11px;font-size:1em;outline:none;text-align:center;margin-bottom:12px}
.md input:focus{border-color:#667eea}
.mdb{width:100%;padding:13px;background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;border:none;border-radius:11px;font-size:.95em;font-weight:600;cursor:pointer}
.err{color:#e74c3c;font-size:.82em;margin-top:6px;display:none}
@media(max-width:500px){.sidebar{width:170px}.hst{display:none}}
)css";

// JS dashboard (statico, va in Flash)
static const char DASH_JS[] PROGMEM = R"js(
var nk='',cm='public',ct='',lm=0,am=[],ur={};
var sn=localStorage.getItem('sm_nick');
if(sn&&sn.length>=2){nk=sn;hide()}
function hide(){
  document.getElementById('mo').style.display='none';
  document.getElementById('mi').disabled=false;
  document.getElementById('sb').disabled=false;
  document.getElementById('sn2').textContent='👤 '+nk;
  document.getElementById('mi').focus();
  join();
}
function go(){
  var v=document.getElementById('ni').value.trim();
  if(v.length<2||v.length>20||!/^[\w\-\.]+$/.test(v)){document.getElementById('er').style.display='block';return;}
  nk=v;localStorage.setItem('sm_nick',nk);hide();
}
document.getElementById('ni').addEventListener('keypress',function(e){if(e.key==='Enter')go()});
function join(){
  fetch('/api/join',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({nickname:nk})}).catch(()=>{});
  setInterval(function(){fetch('/api/ping',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({nickname:nk})}).catch(()=>{})},20000);
}
function sw(m,t){
  cm=m;ct=t;
  document.querySelectorAll('.cb').forEach(function(b){b.classList.remove('active')});
  var id=m==='public'?'bp':'bu'+t;
  var b=document.getElementById(id);if(b)b.classList.add('active');
  if(m==='public'){
    document.getElementById('cht').textContent='💬 Chat Pubblica';
    document.getElementById('chs').textContent='Visibile a tutti';
    ur['public']=0;upbg('public');
  }else{
    document.getElementById('cht').textContent='🔒 '+t;
    document.getElementById('chs').textContent='Chat privata';
    ur[t]=0;upbg(t);
  }
  flt();
}
function upbg(k){
  var c=ur[k]||0;
  var id=k==='public'?'bxp':'bx'+k;
  var b=document.getElementById(id);
  if(b){b.textContent=c;b.style.display=c>0?'inline-block':'none';}
}
function flt(){
  var c=document.getElementById('msgs');
  var f;
  if(cm==='public'){f=am.filter(function(m){return !m.toUser||m.toUser===''});}
  else{f=am.filter(function(m){
    // Messaggio tra me e ct: io ho mandato a ct, oppure ct ha mandato a me
    return (m.fromUser===nk&&m.toUser===ct)||(m.fromUser===ct&&m.toUser===nk);
  });}
  if(!f.length){c.innerHTML='<div class="empty">'+(cm==='public'?'Nessun messaggio. Inizia a chattare!':'Nessun messaggio privato con '+esc(ct)+'.')+'</div>';return;}
  c.innerHTML='';
  f.forEach(function(m){
    var d=document.createElement('div');
    var own=m.fromUser===nk;  // sono il mittente se il mio nick è fromUser
    d.className='msg '+(own?'tx':'rx');
    var meta='<span>'+m.time+'</span>';
    if(m.toUser&&m.toUser!=='')meta+='<span class="pt">🔒</span>';
    var fl=own?'Tu':esc(m.fromUser)+(m.fromNode?'<small style="opacity:.7"> @'+esc(m.fromNode)+'</small>':'');
    d.innerHTML='<div class="bbl">'+(own?'':'<div class="fn">'+fl+'</div>')+'<div class="ft">'+esc(m.text)+'</div><div class="fm">'+meta+'</div></div>';
    c.appendChild(d);
  });
  c.scrollTop=c.scrollHeight;
}
function updM(){
  if(!nk)return;
  fetch('/api/messages').then(function(r){return r.json()}).then(function(d){
    if(d.messages.length===lm)return;
    var nw=d.messages.slice(lm);
    lm=d.messages.length;am=d.messages;
    nw.forEach(function(m){
      if(m.fromUser===nk)return;  // messaggi miei: non contare come unread
      if(!m.toUser||m.toUser===''){if(cm!=='public'){ur['public']=(ur['public']||0)+1;upbg('public');}}
      else if(m.toUser===nk){var k=m.fromUser;if(!(cm==='private'&&ct===k)){ur[k]=(ur[k]||0)+1;upbg(k);}}
    });
    flt();
  }).catch(function(){});
}
function updU(){
  if(!nk)return;
  fetch('/api/users').then(function(r){return r.json()}).then(function(d){
    document.getElementById('snd').textContent=d.nodes||'-';
    document.getElementById('snu').textContent=d.users.length;
    var ul=document.getElementById('ul');ul.innerHTML='';
    d.users.forEach(function(u){
      if(u.nickname===nk)return;
      var div=document.createElement('div');
      div.className='cb'+(cm==='private'&&ct===u.nickname?' active':'');
      div.id='bu'+u.nickname;
      div.onclick=function(){sw('private',u.nickname)};
      var c=ur[u.nickname]||0;
      div.innerHTML='<span class="dot"></span><span style="flex:1;overflow:hidden;text-overflow:ellipsis">'+esc(u.nickname)+'</span><span class="bx" id="bx'+u.nickname+'">'+(c||'')+'</span>';
      if(c>0)div.querySelector('.bx').style.display='inline-block';
      ul.appendChild(div);
    });
  }).catch(function(){});
}
function snd(){
  if(!nk)return;
  var i=document.getElementById('mi');var t=i.value.trim();if(!t)return;
  fetch('/api/send',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({nickname:nk,text:t,toUser:cm==='private'?ct:''})})
  .then(function(r){return r.json()}).then(function(d){if(d.success){i.value='';lm=0;setTimeout(updM,200)}}).catch(function(){});
}
function esc(t){if(!t)return'';var d=document.createElement('div');d.textContent=t;return d.innerHTML}
setInterval(updM,2000);setInterval(updU,5000);
if(nk){updM();updU();}
)js";

// ============================================
// STRUTTURE DATI
// ============================================

struct Message {
    char fromUser[21];
    char fromNode[16];
    char text[201];
    char toUser[21];
    unsigned long timestamp;
    bool isOwn;
};

struct LocalUser {
    char nickname[21];
    unsigned long lastSeen;
};

struct GlobalUser {
    char nickname[21];
    char nodeName[16];
    unsigned long lastSeen;
};

struct NodeInfo {
    uint32_t nodeId;
    char name[16];
    IPAddress ip;
    unsigned long lastSeen;
};

Message messages[MAX_MESSAGES];
int messageIndex = 0;
int messageCount = 0;

LocalUser localUsers[MAX_LOCAL_USERS];
int localUsersCount = 0;

GlobalUser globalUsers[MAX_GLOBAL_USERS];
int globalUsersCount = 0;

NodeInfo knownNodes[10];
int knownNodesCount = 0;

// ============================================
// GESTIONE UTENTI
// ============================================

void upsertLocalUser(const char* nickname) {
    for (int i = 0; i < localUsersCount; i++) {
        if (strcmp(localUsers[i].nickname, nickname) == 0) {
            localUsers[i].lastSeen = millis();
            return;
        }
    }
    if (localUsersCount < MAX_LOCAL_USERS) {
        strncpy(localUsers[localUsersCount].nickname, nickname, 20);
        localUsers[localUsersCount].nickname[20] = 0;
        localUsers[localUsersCount].lastSeen = millis();
        localUsersCount++;
        Serial.printf("👤 Utente: %s\n", nickname);
    }
}

void upsertGlobalUser(const char* nickname, const char* nodeName) {
    if (!nickname || strlen(nickname) == 0) return;
    for (int i = 0; i < globalUsersCount; i++) {
        if (strcmp(globalUsers[i].nickname, nickname) == 0) {
            strncpy(globalUsers[i].nodeName, nodeName, 15);
            globalUsers[i].lastSeen = millis();
            return;
        }
    }
    if (globalUsersCount < MAX_GLOBAL_USERS) {
        strncpy(globalUsers[globalUsersCount].nickname, nickname, 20);
        strncpy(globalUsers[globalUsersCount].nodeName, nodeName, 15);
        globalUsers[globalUsersCount].lastSeen = millis();
        globalUsersCount++;
    }
}

void pruneInactiveUsers() {
    unsigned long now = millis();
    for (int i = 0; i < localUsersCount; i++) {
        if (now - localUsers[i].lastSeen > 60000) {
            localUsers[i] = localUsers[--localUsersCount];
            i--;
        }
    }
    for (int i = 0; i < globalUsersCount; i++) {
        if (now - globalUsers[i].lastSeen > 90000) {
            globalUsers[i] = globalUsers[--globalUsersCount];
            i--;
        }
    }
}

void broadcastUserList() {
    if (localUsersCount == 0) return;
    DynamicJsonDocument doc(256);
    doc["type"] = "users";
    doc["node"] = NODE_NAME;
    JsonArray arr = doc.createNestedArray("u");
    for (int i = 0; i < localUsersCount; i++) arr.add(localUsers[i].nickname);
    String msg; serializeJson(doc, msg);
    mesh.sendBroadcast(msg);
}

// ============================================
// TASKS
// ============================================

void sendHeartbeat();
void requestNodesInfo();
void broadcastUsers();

Task taskHeartbeat(TASK_SECOND * 30, TASK_FOREVER, &sendHeartbeat);
Task taskInfoRequest(TASK_SECOND * 10, TASK_FOREVER, &requestNodesInfo);
Task taskBroadcastUsers(TASK_SECOND * 15, TASK_FOREVER, &broadcastUsers);

void sendHeartbeat() {
    DynamicJsonDocument doc(128);
    doc["from"] = NODE_NAME;
    doc["type"] = "heartbeat";
    String msg; serializeJson(doc, msg);
    mesh.sendBroadcast(msg);
}

void requestNodesInfo() {
    DynamicJsonDocument doc(128);
    doc["type"] = "info_request";
    doc["from"] = NODE_NAME;
    String msg; serializeJson(doc, msg);
    mesh.sendBroadcast(msg);
}

void broadcastUsers() {
    pruneInactiveUsers();
    broadcastUserList();
}

// ============================================
// PORTALE (minimizzato, costruzione leggera)
// ============================================

void sendPortal(AsyncWebServerRequest *request) {
    // CSS inline minimo - tutto inline per risparmiare RAM
    String html = F("<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
        "<meta http-equiv='refresh' content='15'>"
        "<title>SecureMesh</title><style>"
        "*{margin:0;padding:0;box-sizing:border-box}"
        "body{font-family:sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);min-height:100vh;padding:15px}"
        ".box{max-width:600px;margin:0 auto;background:#fff;border-radius:16px;padding:20px}"
        "h1{text-align:center;color:#667eea;margin-bottom:5px}"
        ".sub{text-align:center;color:#999;font-size:.85em;margin-bottom:15px}"
        ".card{border:2px solid #e0e0e0;border-radius:12px;padding:15px;margin-bottom:12px;display:flex;justify-content:space-between;align-items:center}"
        ".card.me{border-color:#10b981;background:#f0fdf4}"
        ".name{font-size:1.1em;font-weight:700;color:#667eea}"
        ".badge{background:#10b981;color:#fff;font-size:.65em;padding:2px 8px;border-radius:10px;margin-left:8px}"
        ".detail{font-size:.75em;color:#888;margin-top:3px}"
        ".online{color:#10b981;font-size:.78em;font-weight:600;margin-top:5px}"
        ".btn{padding:10px 20px;background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;text-decoration:none;border-radius:9px;font-size:.88em;font-weight:700;white-space:nowrap}"
        ".stats{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;margin-top:15px}"
        ".sc{text-align:center;background:#f8f9fa;padding:12px;border-radius:10px}"
        ".sv{font-size:1.8em;font-weight:700;color:#667eea}"
        ".sl{font-size:.7em;color:#666;text-transform:uppercase}"
        ".empty{text-align:center;color:#999;padding:20px;background:#f5f5f5;border-radius:10px;margin-bottom:12px}"
        "</style></head><body><div class='box'>"
        "<h1>🔒 SecureMesh</h1>"
        "<div class='sub'>Rete Mesh v3.1 • Auto-aggiornamento ogni 15s</div>");

    // Questo nodo
    html += F("<div class='card me'><div>");
    html += "<div class='name'>" + String(NODE_NAME);
    html += F("<span class='badge'>TU SEI QUI</span></div>");
    html += "<div class='detail'>ID: " + String(mesh.getNodeId()) + "</div>";
    IPAddress myIP = mesh.getStationIP();
    if (myIP) html += "<div class='detail'>IP: " + myIP.toString() + "</div>";
    if (localUsersCount > 0) {
        html += F("<div class='detail'>Utenti: ");
        for (int i = 0; i < localUsersCount; i++) {
            if (i) html += F(", ");
            html += localUsers[i].nickname;
        }
        html += F("</div>");
    }
    html += F("<div class='online'>● Online - Locale</div></div>");
    html += F("<a href='/dashboard' class='btn'>Apri Chat</a></div>");

    // Nodi remoti
    int others = 0;
    for (int i = 0; i < knownNodesCount; i++) {
        NodeInfo& n = knownNodes[i];
        if (n.nodeId == mesh.getNodeId()) continue;
        if (millis() - n.lastSeen > 60000) continue;
        others++;
        html += F("<div class='card'><div>");
        html += "<div class='name'>" + String(n.name) + "</div>";
        html += "<div class='detail'>ID: " + String(n.nodeId) + "</div>";
        if (n.ip) html += "<div class='detail'>IP: " + n.ip.toString() + "</div>";
        // Utenti remoti su questo nodo
        String ru;
        for (int j = 0; j < globalUsersCount; j++) {
            if (strcmp(globalUsers[j].nodeName, n.name) == 0) {
                if (ru.length()) ru += ", ";
                ru += globalUsers[j].nickname;
            }
        }
        if (ru.length()) html += "<div class='detail'>Utenti: " + ru + "</div>";
        html += F("<div class='online'>● Online - Remoto</div></div>");
        if (n.ip) {
            html += "<a href='http://" + n.ip.toString() + "/dashboard' class='btn'>Apri Chat</a>";
        } else {
            html += F("<span style='color:#999;font-size:.8em'>IP in attesa...</span>");
        }
        html += F("</div>");
    }

    if (others == 0) {
        html += F("<div class='empty'>📡 Nessun altro nodo trovato<br><small>Avvicina altri dispositivi SecureMesh</small></div>");
    }

    html += F("<div class='stats'>"
        "<div class='sc'><div class='sv' id='sn'>-</div><div class='sl'>Nodi</div></div>"
        "<div class='sc'><div class='sv' id='su'>-</div><div class='sl'>Utenti</div></div>"
        "<div class='sc'><div class='sv'>");
    html += String(ESP.getFreeHeap() / 1024);
    html += F("K</div><div class='sl'>RAM</div></div></div>"
        "<div style='text-align:center;margin-top:12px;color:#aaa;font-size:.75em'>"
        "SecureMesh v3.1 • ");
    html += NODE_NAME;
    html += F("</div></div>"
        "<script>"
        "fetch('/api/stats').then(r=>r.json()).then(d=>{document.getElementById('sn').textContent=d.nodes;document.getElementById('su').textContent=d.users});"
        "</script></body></html>");

    request->send(200, "text/html", html);
}

// ============================================
// DASHBOARD - HTML minimale (~600 byte heap)
// CSS e JS serviti come file separati
// ============================================

void sendDashboard(AsyncWebServerRequest *request) {
    // Nodo dinamico: costruiamo solo la parte che cambia
    String nodeName = String(NODE_NAME);

    // HTML puro: ~600 byte, CSS e JS linkati esternamente
    String html = F("<!DOCTYPE html><html><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
        "<title>");
    html += nodeName;
    html += F("</title>"
        "<link rel='stylesheet' href='/dash.css'>"
        "</head><body>"
        "<div class='mo' id='mo'><div class='md'>"
          "<h2>&#x1F464; Nickname</h2>"
          "<p>Scegli il nickname per <strong>");
    html += nodeName;
    html += F("</strong></p>"
          "<input type='text' id='ni' placeholder='Il tuo nome...' maxlength='20' autocomplete='off'>"
          "<div class='err' id='er'>Usa 2-20 caratteri (lettere, numeri, _ - .)</div>"
          "<button class='mdb' onclick='go()'>Entra in Chat</button>"
        "</div></div>"
        "<div class='app'>"
          "<div class='sidebar'>"
            "<div class='sb-head'>"
              "<div class='sb-title'>");
    html += nodeName;
    html += F("</div><div class='sb-nick' id='sn2'></div></div>"
            "<div class='sb-sec'>Chat</div>"
            "<div id='chatlist'>"
              "<div class='cb active' id='bp' onclick='sw(\"public\",\"\")'>"
                "<span>&#x1F4E2;</span><span>Pubblica</span>"
                "<span class='bx' id='bxp'></span>"
              "</div>"
            "</div>"
            "<div class='sb-sec'>Utenti</div>"
            "<div id='ul'></div>"
          "</div>"
          "<div class='main'>"
            "<div class='hdr'>"
              "<button class='bk' onclick='location=\"/\"'>&#8592; Portale</button>"
              "<div class='hi'>"
                "<div class='ht' id='cht'>Chat Pubblica</div>"
                "<div class='hs' id='chs'>Visibile a tutti</div>"
              "</div>"
              "<div class='hst'>"
                "<div><div class='sv' id='snd'>-</div><div class='sl'>Nodi</div></div>"
                "<div><div class='sv' id='snu'>-</div><div class='sl'>Utenti</div></div>"
              "</div>"
            "</div>"
            "<div class='msgs' id='msgs'>"
              "<div class='empty'>Entra nella chat per vedere i messaggi!</div>"
            "</div>"
            "<div class='inp'>"
              "<input type='text' id='mi' placeholder='Scrivi...' "
                "onkeypress='if(event.key===\"Enter\")snd()' autocomplete='off' disabled>"
              "<button id='sb' onclick='snd()' disabled>&#x27A4;</button>"
            "</div>"
          "</div>"
        "</div>"
        "<script src='/dash.js'></script>"
        "</body></html>");

    request->send(200, "text/html", html);
}

// ============================================
// MESH CALLBACKS
// ============================================

void updateNodeCache(uint32_t nodeId, const char* name, IPAddress ip) {
    for (int i = 0; i < knownNodesCount; i++) {
        if (knownNodes[i].nodeId == nodeId) {
            strncpy(knownNodes[i].name, name, 15);
            knownNodes[i].ip = ip;
            knownNodes[i].lastSeen = millis();
            return;
        }
    }
    if (knownNodesCount < 10) {
        knownNodes[knownNodesCount].nodeId = nodeId;
        strncpy(knownNodes[knownNodesCount].name, name, 15);
        knownNodes[knownNodesCount].ip = ip;
        knownNodes[knownNodesCount].lastSeen = millis();
        knownNodesCount++;
    }
}

void receivedCallback(uint32_t from, String &msg) {
    messagesReceived++;
    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, msg)) return;

    const char* msgType = doc["type"] | "unknown";
    const char* fromNode = doc["from"] | "UNKNOWN";

    // info_request (INVARIATO)
    if (strcmp(msgType, "info_request") == 0) {
        DynamicJsonDocument resp(256);
        resp["type"] = "info_response";
        resp["from"] = NODE_NAME;
        resp["nodeId"] = mesh.getNodeId();
        IPAddress myIP = mesh.getStationIP();
        if (myIP) resp["ip"] = myIP.toString();
        String response; serializeJson(resp, response);
        mesh.sendBroadcast(response);
        return;
    }

    // info_response (INVARIATO)
    if (strcmp(msgType, "info_response") == 0) {
        uint32_t nodeId = doc["nodeId"] | 0;
        const char* ip = doc["ip"] | "0.0.0.0";
        IPAddress nodeIP; nodeIP.fromString(ip);
        updateNodeCache(nodeId, fromNode, nodeIP);
        return;
    }

    if (strcmp(msgType, "heartbeat") == 0) return;

    // Lista utenti da altro nodo
    if (strcmp(msgType, "users") == 0) {
        const char* nodeName = doc["node"] | fromNode;
        JsonArray users = doc["u"].as<JsonArray>();
        for (JsonVariant u : users) upsertGlobalUser(u.as<const char*>(), nodeName);
        return;
    }

    // Messaggio chat
    if (strcmp(msgType, "chat") == 0) {
        const char* fromUser = doc["fu"] | fromNode;
        const char* text = doc["t"] | "";
        const char* toUser = doc["tu"] | "";

        // Filtra privati non per questo nodo
        if (strlen(toUser) > 0) {
            bool forme = false;
            for (int i = 0; i < localUsersCount; i++) {
                if (strcmp(localUsers[i].nickname, toUser) == 0) { forme = true; break; }
            }
            if (!forme) return;
        }

        Serial.printf("📨 %s@%s→%s: %s\n", fromUser, fromNode,
            strlen(toUser) > 0 ? toUser : "tutti", text);

        strncpy(messages[messageIndex].fromUser, fromUser, 20);
        strncpy(messages[messageIndex].fromNode, fromNode, 15);
        strncpy(messages[messageIndex].text, text, 200);
        strncpy(messages[messageIndex].toUser, toUser, 20);
        messages[messageIndex].timestamp = millis();
        messages[messageIndex].isOwn = false;
        messageIndex = (messageIndex + 1) % MAX_MESSAGES;
        if (messageCount < MAX_MESSAGES) messageCount++;

        int fl = strlen(toUser) > 0 ? 3 : 2;
        for (int i = 0; i < fl; i++) {
            digitalWrite(LED_PIN, LOW); delay(80);
            digitalWrite(LED_PIN, HIGH); delay(80);
        }
    }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("✅ Nuovo nodo: %u (Tot: %d)\n", nodeId, mesh.getNodeList().size());
    requestNodesInfo();
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, LOW); delay(100);
        digitalWrite(LED_PIN, HIGH); delay(100);
    }
}

void changedConnectionCallback() {
    Serial.printf("⚠️ Topologia cambiata (Nodi: %d)\n", mesh.getNodeList().size());
}

void nodeTimeAdjustedCallback(int32_t offset) {}

// ============================================
// MESSAGING
// ============================================

void sendChatMessage(const char* fromUser, const char* text, const char* toUser) {
    messagesSent++;
    DynamicJsonDocument doc(512);
    doc["type"] = "chat";
    doc["from"] = NODE_NAME;
    doc["fu"] = fromUser;   // fromUser (compatto)
    doc["t"]  = text;       // text
    doc["tu"] = toUser;     // toUser
    String msg; serializeJson(doc, msg);
    mesh.sendBroadcast(msg);

    Serial.printf("📤 %s→%s: %s\n", fromUser,
        strlen(toUser) > 0 ? toUser : "tutti", text);

    strncpy(messages[messageIndex].fromUser, fromUser, 20);
    strncpy(messages[messageIndex].fromNode, NODE_NAME, 15);
    strncpy(messages[messageIndex].text, text, 200);
    strncpy(messages[messageIndex].toUser, toUser, 20);
    messages[messageIndex].timestamp = millis();
    messages[messageIndex].isOwn = true;
    messageIndex = (messageIndex + 1) % MAX_MESSAGES;
    if (messageCount < MAX_MESSAGES) messageCount++;

    digitalWrite(LED_PIN, LOW); delay(50);
    digitalWrite(LED_PIN, HIGH);
}

String formatTime(unsigned long ts) {
    char t[12];
    unsigned long s = ts / 1000, m = s / 60, h = m / 60;
    snprintf(t, sizeof(t), "%02lu:%02lu:%02lu", h % 24, m % 60, s % 60);
    return String(t);
}

// ============================================
// SETUP
// ============================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    Serial.println(F("\n╔══════════════════════════════════╗"));
    Serial.println(F("║  SECUREMESH v3.1 - NICK+CHAT    ║"));
    Serial.println(F("╚══════════════════════════════════╝"));
    Serial.printf("Nodo: %s | Heap: %uK\n\n", NODE_NAME, ESP.getFreeHeap() / 1024);

    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

    userScheduler.addTask(taskHeartbeat);
    userScheduler.addTask(taskInfoRequest);
    userScheduler.addTask(taskBroadcastUsers);
    taskHeartbeat.enable();
    taskInfoRequest.enable();
    taskBroadcastUsers.enable();

    Serial.println(F("✅ Mesh OK!"));
    Serial.printf("   Node ID: %u\n", mesh.getNodeId());

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    Serial.println(F("✅ DNS OK! Captive portal attivo"));

    // Routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){ sendPortal(r); });
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *r){ sendDashboard(r); });

    // CSS e JS serviti direttamente da PROGMEM - zero heap aggiuntivo
    server.on("/dash.css", HTTP_GET, [](AsyncWebServerRequest *r){
        AsyncWebServerResponse *resp = r->beginResponse_P(200, "text/css", DASH_CSS);
        resp->addHeader("Cache-Control", "max-age=300");
        r->send(resp);
    });

    server.on("/dash.js", HTTP_GET, [](AsyncWebServerRequest *r){
        AsyncWebServerResponse *resp = r->beginResponse_P(200, "application/javascript", DASH_JS);
        resp->addHeader("Cache-Control", "max-age=300");
        r->send(resp);
    });

    // Captive portal detection
    server.on("/generate_204",        HTTP_GET, [](AsyncWebServerRequest *r){ r->redirect("/"); });
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *r){ r->redirect("/"); });
    server.on("/canonical.html",      HTTP_GET, [](AsyncWebServerRequest *r){ r->redirect("/"); });
    server.on("/success.txt",         HTTP_GET, [](AsyncWebServerRequest *r){ r->send(200, "text/plain", "success"); });

    // API stats
    server.on("/api/stats", HTTP_GET, [](AsyncWebServerRequest *r){
        char buf[64];
        int users = localUsersCount + globalUsersCount;
        snprintf(buf, sizeof(buf), "{\"nodes\":%d,\"total\":%d,\"users\":%d}",
            (int)mesh.getNodeList().size() + 1, messageCount, users);
        r->send(200, "application/json", buf);
    });

    // API join
    server.on("/api/join", HTTP_POST, [](AsyncWebServerRequest *r){}, NULL,
        [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t, size_t){
            DynamicJsonDocument doc(128);
            if (deserializeJson(doc, (char*)data)) { r->send(400); return; }
            const char* nick = doc["nickname"] | "";
            if (strlen(nick) < 2 || strlen(nick) > 20) { r->send(400); return; }
            upsertLocalUser(nick);
            broadcastUserList();
            r->send(200, "application/json", "{\"ok\":true}");
        }
    );

    // API ping
    server.on("/api/ping", HTTP_POST, [](AsyncWebServerRequest *r){}, NULL,
        [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t, size_t){
            DynamicJsonDocument doc(128);
            if (!deserializeJson(doc, (char*)data)) {
                const char* nick = doc["nickname"] | "";
                if (strlen(nick) >= 2) upsertLocalUser(nick);
            }
            r->send(200, "application/json", "{\"ok\":true}");
        }
    );

    // API users
    server.on("/api/users", HTTP_GET, [](AsyncWebServerRequest *r){
        DynamicJsonDocument doc(1024);
        doc["nodes"] = (int)mesh.getNodeList().size() + 1;
        JsonArray arr = doc.createNestedArray("users");
        for (int i = 0; i < localUsersCount; i++) {
            JsonObject u = arr.createNestedObject();
            u["nickname"] = localUsers[i].nickname;
            u["node"] = NODE_NAME;
        }
        for (int i = 0; i < globalUsersCount; i++) {
            JsonObject u = arr.createNestedObject();
            u["nickname"] = globalUsers[i].nickname;
            u["node"] = globalUsers[i].nodeName;
        }
        String resp; serializeJson(doc, resp);
        r->send(200, "application/json", resp);
    });

    // API messages
    server.on("/api/messages", HTTP_GET, [](AsyncWebServerRequest *r){
        DynamicJsonDocument doc(3072);
        JsonArray arr = doc.createNestedArray("messages");
        int start = (messageCount < MAX_MESSAGES) ? 0 : messageIndex;
        for (int i = 0; i < messageCount; i++) {
            int idx = (start + i) % MAX_MESSAGES;
            JsonObject m = arr.createNestedObject();
            m["fromUser"] = messages[idx].fromUser;
            m["fromNode"] = messages[idx].fromNode;
            m["text"]     = messages[idx].text;
            m["toUser"]   = messages[idx].toUser;
            m["time"]     = formatTime(messages[idx].timestamp);
            m["isOwn"]    = messages[idx].isOwn;
        }
        String resp; serializeJson(doc, resp);
        r->send(200, "application/json", resp);
    });

    // API send
    server.on("/api/send", HTTP_POST, [](AsyncWebServerRequest *r){}, NULL,
        [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t, size_t){
            DynamicJsonDocument doc(512);
            if (deserializeJson(doc, (char*)data)) { r->send(400); return; }
            const char* nick   = doc["nickname"] | "";
            const char* text   = doc["text"] | "";
            const char* toUser = doc["toUser"] | "";
            if (strlen(nick) < 2 || strlen(text) == 0 || strlen(text) > 200) {
                r->send(400, "application/json", "{\"success\":false}");
                return;
            }
            upsertLocalUser(nick);
            sendChatMessage(nick, text, toUser);
            r->send(200, "application/json", "{\"success\":true}");
        }
    );

    server.onNotFound([](AsyncWebServerRequest *r){ r->redirect("/"); });
    server.begin();

    Serial.println(F("✅ Web server OK!\n"));
    Serial.println(F("1. WiFi: SecureMesh_PoC / mesh2026secure"));
    Serial.println(F("2. Portale apre automaticamente"));
    Serial.println(F("3. Scegli nodo → nickname → chatta!\n"));

    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, LOW); delay(200);
        digitalWrite(LED_PIN, HIGH); delay(200);
    }
    Serial.println(F("✅ Pronto!\n"));
}

// ============================================
// LOOP (IDENTICO al gold)
// ============================================

void loop() {
    dnsServer.processNextRequest();
    mesh.update();

    static unsigned long lastD = 0;
    if (millis() - lastD > 15000) {
        Serial.printf("[%s] Nodi:%d Cache:%d Lu:%d Gu:%d Msg:%d RAM:%uK\n",
            NODE_NAME, (int)mesh.getNodeList().size(), knownNodesCount,
            localUsersCount, globalUsersCount, messageCount,
            ESP.getFreeHeap() / 1024);
        lastD = millis();
    }
}
