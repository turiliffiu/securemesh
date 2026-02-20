/*
 * SecureMesh - MULTI-ROOM COMPATTO
 * Dashboard ottimizzata per ESP8266 (RAM limitata)
 */

#include <Arduino.h>
#include "painlessMesh.h"
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <DNSServer.h>

#define MESH_PREFIX     "SecureMesh_PoC"
#define MESH_PASSWORD   "mesh2026secure"
#define MESH_PORT       5555

#ifndef NODE_NAME
#define NODE_NAME       "NODE_DEFAULT"
#endif

#define LED_PIN         D4
#define MAX_MESSAGES    30
#define DNS_PORT        53

Scheduler userScheduler;
painlessMesh mesh;
AsyncWebServer server(80);
DNSServer dnsServer;

uint32_t messagesSent = 0;
uint32_t messagesReceived = 0;

struct Message {
    String from;
    String text;
    String channel;
    uint32_t toNodeId;
    unsigned long timestamp;
    bool isOwn;
};

Message messages[MAX_MESSAGES];
int messageIndex = 0;
int messageCount = 0;

struct NodeInfo {
    uint32_t nodeId;
    String name;
    IPAddress ip;
    unsigned long lastSeen;
};

NodeInfo knownNodes[10];
int knownNodesCount = 0;

void sendHeartbeat();
void requestNodesInfo();

Task taskHeartbeat(TASK_SECOND * 30, TASK_FOREVER, &sendHeartbeat);
Task taskInfoRequest(TASK_SECOND * 10, TASK_FOREVER, &requestNodesInfo);

void sendHeartbeat() {
    DynamicJsonDocument doc(128);
    doc["from"] = NODE_NAME;
    doc["type"] = "heartbeat";
    String msg;
    serializeJson(doc, msg);
    mesh.sendBroadcast(msg);
}

void requestNodesInfo() {
    DynamicJsonDocument doc(128);
    doc["type"] = "info_request";
    doc["from"] = NODE_NAME;
    String msg;
    serializeJson(doc, msg);
    mesh.sendBroadcast(msg);
}

// ============================================
// PORTALE (MANTENUTO IDENTICO)
// ============================================

void sendPortal(AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='15'>";
    html += "<title>SecureMesh Portal</title><style>";
    html += "*{margin:0;padding:0;box-sizing:border-box}";
    html += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);min-height:100vh;padding:20px}";
    html += ".container{max-width:700px;margin:0 auto;background:#fff;border-radius:20px;padding:30px;box-shadow:0 20px 60px rgba(0,0,0,0.3)}";
    html += "h1{background:linear-gradient(135deg,#667eea,#764ba2);-webkit-background-clip:text;-webkit-text-fill-color:transparent;margin-bottom:10px;text-align:center;font-size:2.5em}";
    html += ".subtitle{text-align:center;color:#999;margin-bottom:20px;font-size:1em}";
    html += ".welcome{text-align:center;background:linear-gradient(135deg,#e3f2fd,#f3e5f5);padding:20px;border-radius:15px;margin-bottom:25px;border-left:4px solid #667eea}";
    html += ".welcome h2{color:#667eea;margin-bottom:10px;font-size:1.3em}";
    html += ".welcome p{color:#666;line-height:1.6}";
    html += ".nodes{display:flex;flex-direction:column;gap:15px;margin-bottom:25px}";
    html += ".node-card{background:linear-gradient(135deg,#f8f9fa,#fff);padding:20px;border-radius:15px;display:flex;justify-content:space-between;align-items:center;transition:all 0.3s;border:2px solid #e0e0e0}";
    html += ".node-card:hover{transform:translateY(-3px);box-shadow:0 8px 20px rgba(102,126,234,0.3);border-color:#667eea}";
    html += ".node-card.current{border-color:#10b981;background:linear-gradient(135deg,#d1fae5,#fff)}";
    html += ".node-info{flex:1}";
    html += ".node-name{font-size:1.4em;font-weight:bold;color:#667eea;margin-bottom:5px}";
    html += ".node-badge{display:inline-block;background:#10b981;color:#fff;padding:3px 10px;border-radius:12px;font-size:0.7em;margin-left:10px}";
    html += ".node-id{font-size:0.75em;color:#999;margin:5px 0}";
    html += ".node-status{display:flex;align-items:center;gap:5px;margin-top:8px}";
    html += ".status-dot{width:10px;height:10px;border-radius:50%;background:#10b981;animation:pulse 2s infinite}";
    html += "@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.5}}";
    html += ".btn{padding:12px 25px;background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;text-decoration:none;border-radius:10px;font-weight:bold;display:inline-block;transition:all 0.3s;border:none;cursor:pointer}";
    html += ".btn:hover{transform:scale(1.05);box-shadow:0 5px 15px rgba(102,126,234,0.4)}";
    html += ".stats{display:grid;grid-template-columns:1fr 1fr 1fr;gap:15px;margin-top:25px}";
    html += ".stat-card{background:#f8f9fa;padding:20px;border-radius:12px;text-align:center;border:2px solid #e0e0e0}";
    html += ".stat-value{font-size:2.2em;font-weight:bold;background:linear-gradient(135deg,#667eea,#764ba2);-webkit-background-clip:text;-webkit-text-fill-color:transparent}";
    html += ".stat-label{color:#666;margin-top:5px;font-size:0.85em;text-transform:uppercase;letter-spacing:1px}";
    html += ".footer{text-align:center;margin-top:25px;padding-top:20px;border-top:2px solid #e0e0e0;color:#999;font-size:0.85em}";
    html += ".info-box{background:#fff3cd;border-left:4px solid #ffc107;padding:15px;border-radius:8px;margin-bottom:20px}";
    html += ".info-box p{color:#856404;margin:0;font-size:0.9em}";
    html += ".empty{text-align:center;padding:40px;color:#999;background:#f5f5f5;border-radius:10px}";
    html += "</style></head><body><div class='container'>";
    html += "<h1>🔒 SecureMesh</h1>";
    html += "<div class='subtitle'>Rete Mesh Decentralizzata • Multi-Room</div>";
    
    html += "<div class='welcome'>";
    html += "<h2>Benvenuto nel Portale!</h2>";
    html += "<p>Dashboard Multi-Room: Broadcast + 4 Canali + Chat Private</p>";
    html += "</div>";
    
    html += "<div class='info-box'>";
    html += "<p>⟳ Auto-refresh 15s • Click su nodo per accedere</p>";
    html += "</div>";
    
    html += "<div class='nodes'>";
    
    html += "<div class='node-card current'>";
    html += "<div class='node-info'>";
    html += "<div class='node-name'>" + String(NODE_NAME);
    html += "<span class='node-badge'>TU SEI QUI</span></div>";
    html += "<div class='node-id'>Node ID: " + String(mesh.getNodeId()) + "</div>";
    IPAddress myIP = mesh.getStationIP();
    if (myIP) {
        html += "<div class='node-id'>IP: " + myIP.toString() + "</div>";
    }
    html += "<div class='node-status'><div class='status-dot'></div><span style='font-size:0.85em;color:#10b981;font-weight:bold'>Online</span></div>";
    html += "</div>";
    html += "<a href='/dashboard' class='btn'>Apri Dashboard</a>";
    html += "</div>";
    
    int otherNodes = 0;
    for (int i = 0; i < knownNodesCount; i++) {
        if (knownNodes[i].nodeId != mesh.getNodeId() && (millis() - knownNodes[i].lastSeen) < 60000) {
            otherNodes++;
            html += "<div class='node-card'>";
            html += "<div class='node-info'>";
            html += "<div class='node-name'>" + knownNodes[i].name + "</div>";
            html += "<div class='node-id'>Node ID: " + String(knownNodes[i].nodeId) + "</div>";
            if (knownNodes[i].ip) {
                html += "<div class='node-id'>IP: " + knownNodes[i].ip.toString() + "</div>";
            }
            html += "<div class='node-status'><div class='status-dot'></div><span style='font-size:0.85em;color:#666'>Online</span></div>";
            html += "</div>";
            
            if (knownNodes[i].ip) {
                html += "<a href='http://" + knownNodes[i].ip.toString() + "/dashboard' class='btn'>Apri Dashboard</a>";
            } else {
                html += "<button class='btn' disabled style='opacity:0.5'>In attesa...</button>";
            }
            html += "</div>";
        }
    }
    
    if (otherNodes == 0) {
        html += "<div class='empty'>";
        html += "<p style='font-size:1.1em;margin-bottom:10px'>📡 Nessun altro nodo</p>";
        html += "<p>Aspetta qualche secondo...</p>";
        html += "</div>";
    }
    
    html += "</div>";
    
    html += "<div class='stats'>";
    html += "<div class='stat-card'><div class='stat-value'>" + String(mesh.getNodeList().size() + 1) + "</div><div class='stat-label'>Nodi</div></div>";
    html += "<div class='stat-card'><div class='stat-value'>" + String(messageCount) + "</div><div class='stat-label'>Messaggi</div></div>";
    html += "<div class='stat-card'><div class='stat-value'>" + String(ESP.getFreeHeap() / 1024) + "K</div><div class='stat-label'>RAM</div></div>";
    html += "</div>";
    
    html += "<div class='footer'>";
    html += "<p>SecureMesh v2.0 • " + String(NODE_NAME) + "</p>";
    html += "</div>";
    
    html += "</div></body></html>";
    
    request->send(200, "text/html", html);
}

// ============================================
// DASHBOARD COMPATTA (HTML MINIMO!)
// ============================================

void sendDashboard(AsyncWebServerRequest *request) {
    // Invia HTML in chunk per evitare overflow!
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    
    response->print("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
    response->print("<meta name='viewport' content='width=device-width,initial-scale=1.0'>");
    response->print("<title>");
    response->print(NODE_NAME);
    response->print("</title><style>");
    response->print("*{margin:0;padding:0;box-sizing:border-box}");
    response->print("body{font-family:Arial,sans-serif;background:#f5f5f5;height:100vh;overflow:hidden;display:flex}");
    response->print(".side{width:200px;background:#2c3e50;color:#fff;display:flex;flex-direction:column}");
    response->print(".hd{padding:15px;background:#1a252f;font-size:1em;font-weight:bold}");
    response->print(".ch{padding:8px 10px;border-radius:5px;cursor:pointer;margin:3px 5px;font-size:0.8em}");
    response->print(".ch:hover{background:#34495e}");
    response->print(".ch.on{background:#667eea}");
    response->print(".main{flex:1;display:flex;flex-direction:column;background:#fff}");
    response->print(".top{padding:10px 15px;background:#fff;border-bottom:1px solid #e0e0e0;display:flex;justify-content:space-between}");
    response->print(".btn{padding:6px 12px;background:#f5f5f5;border:none;border-radius:5px;cursor:pointer;font-size:0.8em}");
    response->print(".msgs{flex:1;padding:15px;overflow-y:auto;background:#f9f9f9}");
    response->print(".msg{margin-bottom:10px}");
    response->print(".bub{display:inline-block;max-width:70%;padding:8px 12px;border-radius:10px;background:#fff}");
    response->print(".sent{text-align:right}");
    response->print(".sent .bub{background:linear-gradient(135deg,#667eea,#764ba2);color:#fff}");
    response->print(".from{font-weight:600;font-size:0.75em;margin-bottom:3px;color:#667eea}");
    response->print(".sent .from{color:rgba(255,255,255,0.95)}");
    response->print(".txt{font-size:0.85em}");
    response->print(".inp{padding:12px 15px;background:#fff;border-top:1px solid #e0e0e0;display:flex;gap:8px}");
    response->print("#i{flex:1;padding:10px 15px;border:2px solid #e0e0e0;border-radius:18px;outline:none}");
    response->print("#s{width:40px;height:40px;border-radius:50%;background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;border:none;cursor:pointer}");
    response->print("</style></head><body>");
    
    response->print("<div class='side'><div class='hd'>");
    response->print(NODE_NAME);
    response->print("</div>");
    response->print("<div class='ch on' onclick='sw(\"broadcast\")'>📢 Tutti</div>");
    response->print("<div class='ch' onclick='sw(\"generale\")'># Generale</div>");
    response->print("<div class='ch' onclick='sw(\"lavoro\")'>💼 Lavoro</div>");
    response->print("<div class='ch' onclick='sw(\"random\")'>🎲 Random</div>");
    response->print("<div style='padding:15px 10px;font-size:0.7em;opacity:0.6'>UTENTI</div>");
    response->print("<div id='u'></div></div>");
    
    response->print("<div class='main'>");
    response->print("<div class='top'><div id='t'>📢 Tutti</div>");
    response->print("<button class='btn' onclick='location=\"/\"'>← Portale</button></div>");
    response->print("<div class='msgs' id='m'></div>");
    response->print("<div class='inp'><input id='i' placeholder='Messaggio...' onkeypress=\"if(event.key==='Enter')send()\">");
    response->print("<button id='s' onclick='send()'>➤</button></div></div>");
    
    response->print("<script>");
    response->print("let ch='broadcast',tgt=0,lst=0,all=[];");
    response->print("function sw(c,t){ch=c;tgt=t||0;document.querySelectorAll('.ch').forEach(e=>e.classList.remove('on'));");
    response->print("const e=document.querySelector('.ch');if(e)e.classList.add('on');");
    response->print("document.getElementById('t').textContent=tgt>0?'💬 Chat':(c==='broadcast'?'📢 Tutti':'# '+c);flt()}");
    response->print("function flt(){const m=document.getElementById('m');");
    response->print("const f=all.filter(x=>{if(tgt>0)return(x.toNodeId==tgt&&x.isOwn)||(x.fromId==tgt&&!x.isOwn);");
    response->print("return x.channel===ch&&x.toNodeId===0});");
    response->print("if(!f.length){m.innerHTML='<div style=\"text-align:center;padding:30px;color:#999\">Nessun messaggio</div>';return}");
    response->print("m.innerHTML='';f.forEach(x=>{const d=document.createElement('div');");
    response->print("d.className='msg '+(x.isOwn?'sent':'');");
    response->print("d.innerHTML='<div class=\"bub\">'+(x.isOwn?'':'<div class=\"from\">'+esc(x.from)+'</div>')+'<div class=\"txt\">'+esc(x.text)+'</div></div>';");
    response->print("m.appendChild(d)});m.scrollTop=m.scrollHeight}");
    response->print("function updm(){fetch('/api/messages').then(r=>r.json()).then(d=>{");
    response->print("if(d.messages.length===lst)return;lst=d.messages.length;all=d.messages;flt()}).catch(e=>{})}");
    response->print("function updu(){fetch('/api/nodes').then(r=>r.json()).then(d=>{");
    response->print("const u=document.getElementById('u');u.innerHTML='';");
    response->print("d.nodes.forEach(n=>{if(n.id!=");
    response->print(mesh.getNodeId());
    response->print("){const v=document.createElement('div');v.className='ch';v.onclick=()=>sw('p',n.id);");
    response->print("v.textContent='● '+n.name;u.appendChild(v)}})}).catch(e=>{})}");
    response->print("function send(){const i=document.getElementById('i'),t=i.value.trim();if(!t)return;");
    response->print("fetch('/api/send',{method:'POST',headers:{'Content-Type':'application/json'},");
    response->print("body:JSON.stringify({text:t,channel:ch,targetId:tgt})})");
    response->print(".then(r=>r.json()).then(d=>{if(d.success){i.value='';setTimeout(updm,200)}}).catch(e=>{})}");
    response->print("function esc(t){const d=document.createElement('div');d.textContent=t;return d.innerHTML}");
    response->print("setInterval(updm,2000);setInterval(updu,5000);updm();updu();");
    response->print("</script></body></html>");
    
    request->send(response);
}

// ============================================
// CALLBACKS (IDENTICI)
// ============================================

void receivedCallback(uint32_t from, String &msg) {
    messagesReceived++;
    
    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, msg)) return;
    
    String msgType = doc["type"] | "unknown";
    
    if (msgType == "heartbeat") return;
    
    if (msgType == "info_request") {
        DynamicJsonDocument resp(256);
        resp["type"] = "info_response";
        resp["from"] = NODE_NAME;
        resp["fromId"] = mesh.getNodeId();
        IPAddress myIP = mesh.getStationIP();
        if (myIP) resp["ip"] = myIP.toString();
        String r;
        serializeJson(resp, r);
        mesh.sendSingle(from, r);
        return;
    }
    
    if (msgType == "info_response") {
        String fromName = doc["from"] | "UNKNOWN";
        uint32_t fromId = doc["fromId"] | from;
        String ipStr = doc["ip"] | "";
        
        bool found = false;
        for (int i = 0; i < knownNodesCount; i++) {
            if (knownNodes[i].nodeId == fromId) {
                knownNodes[i].name = fromName;
                if (ipStr.length() > 0) knownNodes[i].ip.fromString(ipStr);
                knownNodes[i].lastSeen = millis();
                found = true;
                break;
            }
        }
        
        if (!found && knownNodesCount < 10) {
            knownNodes[knownNodesCount].nodeId = fromId;
            knownNodes[knownNodesCount].name = fromName;
            if (ipStr.length() > 0) knownNodes[knownNodesCount].ip.fromString(ipStr);
            knownNodes[knownNodesCount].lastSeen = millis();
            knownNodesCount++;
        }
        return;
    }
    
    if (msgType == "chat") {
        String fromNode = doc["from"] | "UNKNOWN";
        String text = doc["text"] | "";
        String channel = doc["channel"] | "broadcast";
        uint32_t toNodeId = doc["to"] | 0;
        
        if (toNodeId != 0 && toNodeId != mesh.getNodeId()) return;
        
        messages[messageIndex].from = fromNode;
        messages[messageIndex].text = text;
        messages[messageIndex].channel = channel;
        messages[messageIndex].toNodeId = toNodeId;
        messages[messageIndex].timestamp = millis();
        messages[messageIndex].isOwn = false;
        
        messageIndex = (messageIndex + 1) % MAX_MESSAGES;
        if (messageCount < MAX_MESSAGES) messageCount++;
        
        int flashes = (toNodeId > 0) ? 3 : 2;
        for(int i=0; i<flashes; i++) {
            digitalWrite(LED_PIN, LOW);
            delay(80);
            digitalWrite(LED_PIN, HIGH);
            delay(80);
        }
    }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("\n✅ NUOVO NODO: %u\n\n", nodeId);
    for(int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
    }
}

void changedConnectionCallback() {
    Serial.printf("\n⚠️  Topologia: %d nodi\n\n", mesh.getNodeList().size());
}

void nodeTimeAdjustedCallback(int32_t offset) {}

void sendChatMessage(String text, String channel, uint32_t targetId) {
    messagesSent++;
    
    DynamicJsonDocument doc(512);
    doc["from"] = NODE_NAME;
    doc["fromId"] = mesh.getNodeId();
    doc["type"] = "chat";
    doc["text"] = text;
    doc["channel"] = channel;
    
    if (targetId > 0) {
        doc["to"] = targetId;
        mesh.sendSingle(targetId, doc.as<String>());
    } else {
        doc["to"] = 0;
        mesh.sendBroadcast(doc.as<String>());
    }
    
    messages[messageIndex].from = NODE_NAME;
    messages[messageIndex].text = text;
    messages[messageIndex].channel = channel;
    messages[messageIndex].toNodeId = targetId;
    messages[messageIndex].timestamp = millis();
    messages[messageIndex].isOwn = true;
    
    messageIndex = (messageIndex + 1) % MAX_MESSAGES;
    if (messageCount < MAX_MESSAGES) messageCount++;
    
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
}

String formatTime(unsigned long timestamp) {
    unsigned long s = timestamp / 1000;
    unsigned long m = s / 60;
    unsigned long h = m / 60;
    char t[16];
    snprintf(t, sizeof(t), "%02lu:%02lu:%02lu", h % 24, m % 60, s % 60);
    return String(t);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║   SECUREMESH - MULTI-ROOM COMPATTO   ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    Serial.printf("Nodo: %s\n", NODE_NAME);
    Serial.printf("Chip ID: %08X\n\n", ESP.getChipId());
    
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
    
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    
    userScheduler.addTask(taskHeartbeat);
    userScheduler.addTask(taskInfoRequest);
    taskHeartbeat.enable();
    taskInfoRequest.enable();
    
    Serial.println("✅ Mesh OK!");
    Serial.printf("   Node ID: %u\n\n", mesh.getNodeId());
    
    Serial.println("🌐 Init DNS Captive Portal...");
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    Serial.println("✅ DNS OK!\n");
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
        sendPortal(req);
    });
    
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *req){
        sendDashboard(req);
    });
    
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *req){
        req->redirect("/");
    });
    
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *req){
        req->redirect("/");
    });
    
    server.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *req){
        req->redirect("/");
    });
    
    server.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *req){
        req->send(200, "text/plain", "success");
    });
    
    server.on("/api/nodes", HTTP_GET, [](AsyncWebServerRequest *req){
        DynamicJsonDocument doc(1024);
        JsonArray arr = doc.createNestedArray("nodes");
        
        JsonObject self = arr.createNestedObject();
        self["id"] = mesh.getNodeId();
        self["name"] = NODE_NAME;
        
        for (int i = 0; i < knownNodesCount; i++) {
            if ((millis() - knownNodes[i].lastSeen) < 60000) {
                JsonObject n = arr.createNestedObject();
                n["id"] = knownNodes[i].nodeId;
                n["name"] = knownNodes[i].name;
            }
        }
        
        String r;
        serializeJson(doc, r);
        req->send(200, "application/json", r);
    });
    
    server.on("/api/stats", HTTP_GET, [](AsyncWebServerRequest *req){
        DynamicJsonDocument doc(256);
        doc["nodes"] = mesh.getNodeList().size();
        doc["total"] = messageCount;
        String r;
        serializeJson(doc, r);
        req->send(200, "application/json", r);
    });
    
    server.on("/api/messages", HTTP_GET, [](AsyncWebServerRequest *req){
        DynamicJsonDocument doc(4096);
        JsonArray arr = doc.createNestedArray("messages");
        int start = (messageCount < MAX_MESSAGES) ? 0 : messageIndex;
        for(int i = 0; i < messageCount; i++) {
            int idx = (start + i) % MAX_MESSAGES;
            JsonObject m = arr.createNestedObject();
            m["from"] = messages[idx].from;
            m["text"] = messages[idx].text;
            m["channel"] = messages[idx].channel;
            m["toNodeId"] = messages[idx].toNodeId;
            m["fromId"] = 0;
            m["time"] = formatTime(messages[idx].timestamp);
            m["isOwn"] = messages[idx].isOwn;
        }
        String r;
        serializeJson(doc, r);
        req->send(200, "application/json", r);
    });
    
    server.on("/api/send", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL,
        [](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t idx, size_t tot){
            DynamicJsonDocument doc(512);
            if (deserializeJson(doc, (char*)data)) {
                req->send(400, "application/json", "{\"success\":false}");
                return;
            }
            String text = doc["text"] | "";
            String channel = doc["channel"] | "broadcast";
            uint32_t targetId = doc["targetId"] | 0;
            
            if (text.length() == 0 || text.length() > 200) {
                req->send(400, "application/json", "{\"success\":false}");
                return;
            }
            sendChatMessage(text, channel, targetId);
            req->send(200, "application/json", "{\"success\":true}");
        }
    );
    
    server.onNotFound([](AsyncWebServerRequest *req){
        req->redirect("/");
    });
    
    server.begin();
    Serial.println("🌐 Web server OK!\n");
    
    Serial.println("✅ PRONTO! Multi-Room attivo\n");
    
    for(int i=0; i<3; i++) {
        digitalWrite(LED_PIN, LOW);
        delay(200);
        digitalWrite(LED_PIN, HIGH);
        delay(200);
    }
}

void loop() {
    dnsServer.processNextRequest();
    mesh.update();
    
    static unsigned long lastD = 0;
    if (millis() - lastD > 15000) {
        Serial.printf("[%s] Nodi:%d Cache:%d Msg:%d RAM:%uK\n", 
            NODE_NAME, mesh.getNodeList().size(), knownNodesCount, 
            messageCount, ESP.getFreeHeap() / 1024);
        lastD = millis();
    }
}
