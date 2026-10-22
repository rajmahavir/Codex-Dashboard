#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <esp_crt_bundle.h>
#include <mbedtls/gcm.h>
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>
#include <esp_system.h>
#include <time.h>

static const char *AP_PREFIX = "CodexSetup-";
static const char *USAGE_URL = "https://chatgpt.com/backend-api/wham/usage";
static const uint32_t POLL_MS = 180000;
static const uint32_t PBKDF2_ITERATIONS = 120000;

WebServer server(80);
DNSServer dnsServer;
Preferences prefs;
TFT_eSPI tft;

String apName, apPassword, statusText = "Booting";
String wifiSsid, wifiPassword;
String accessToken, accountId, planType = "ChatGPT";
bool authUnlocked = false;
bool haveShort = false, haveWeekly = false;
float shortUsed = 0, weeklyUsed = 0;
uint32_t shortResetAt = 0, weeklyResetAt = 0;
uint32_t lastPoll = 0, lastGood = 0;
uint8_t encSalt[16], encNonce[12], encTag[16];
String encryptedBlob;

String htmlEscape(const String &s) {
  String o; o.reserve(s.length() + 8);
  for (char c : s) {
    if (c == '&') o += F("&amp;");
    else if (c == '<') o += F("&lt;");
    else if (c == '>') o += F("&gt;");
    else if (c == '"') o += F("&quot;");
    else if (c == '\'') o += F("&#39;");
    else o += c;
  }
  return o;
}

String pageStart(const String &title) {
  String s = F("<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
               "<meta charset='utf-8'><title>");
  s += htmlEscape(title);
  s += F("</title><style>"
         "body{font:16px system-ui;background:#10141c;color:#eef2ff;margin:0;padding:20px}"
         "main{max-width:560px;margin:auto} .card{background:#1b2330;padding:18px;border-radius:14px;margin:12px 0}"
         "input,button{box-sizing:border-box;width:100%;padding:12px;margin:7px 0;border-radius:8px;border:1px solid #526174}"
         "input{background:#0f1722;color:white}button{background:#57c7ff;color:#07121b;font-weight:700;border:0}"
         "small,.muted{color:#aab6c8} .bar{height:12px;background:#344152;border-radius:8px;overflow:hidden}"
         ".fill{height:100%;background:#53d6a5}.warn{background:#ffb454}.danger{background:#ff667d}"
         "a{color:#70d2ff} code{overflow-wrap:anywhere}</style></head><body><main>");
  return s;
}
String pageEnd() { return F("</main></body></html>"); }

void drawStatusScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.setTextSize(2);
  tft.setCursor(8, 8); tft.print("CODEX USAGE");
  tft.drawFastHLine(8, 34, 156, TFT_DARKGREY);
  tft.setTextSize(1); tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setCursor(8, 42); tft.print("Wi-Fi: ");
  tft.println(WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "Setup AP");
  tft.setCursor(8, 56); tft.print("Web: ");
  if (WiFi.status() == WL_CONNECTED) tft.println(WiFi.localIP());
  else tft.println("192.168.4.1");

  auto drawWindow = [&](const char *label, bool present, float used, uint32_t resetAt, int y) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setTextSize(2); tft.setCursor(8, y);
    tft.print(label);
    if (!present) {
      tft.setTextSize(1); tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.setCursor(8, y + 24); tft.print("Not returned by API"); return;
    }
    tft.setCursor(8, y + 23); tft.printf("%.0f%% used", used);
    tft.drawRoundRect(8, y + 48, 156, 13, 4, TFT_DARKGREY);
    int fw = (int)(152 * constrain(used, 0.0f, 100.0f) / 100.0f);
    uint16_t col = used >= 90 ? TFT_RED : (used >= 70 ? TFT_ORANGE : TFT_CYAN);
    if (fw > 0) tft.fillRoundRect(10, y + 50, fw, 9, 3, col);
    tft.setTextSize(1); tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setCursor(8, y + 67);
    if (resetAt > (uint32_t)time(nullptr)) {
      uint32_t secs = resetAt - (uint32_t)time(nullptr);
      tft.printf("Reset in %ud %uh %um", secs / 86400, (secs % 86400) / 3600, (secs % 3600) / 60);
    } else tft.print("Reset time unavailable");
  };
  drawWindow("Short window", haveShort, shortUsed, shortResetAt, 78);
  drawWindow("Weekly window", haveWeekly, weeklyUsed, weeklyResetAt, 178);
  tft.drawFastHLine(8, 276, 156, TFT_DARKGREY);
  tft.setTextSize(1); tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(8, 284); tft.println(statusText.substring(0, 24));
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.setCursor(8, 301);
  tft.print("Refresh: 3 min");
}

String bytesToHex(const uint8_t *data, size_t len) {
  static const char hex[] = "0123456789abcdef";
  String out; out.reserve(len * 2);
  for (size_t i = 0; i < len; i++) { out += hex[data[i] >> 4]; out += hex[data[i] & 0x0f]; }
  return out;
}
bool hexToBytes(const String &hex, uint8_t *out, size_t outLen) {
  if (hex.length() != outLen * 2) return false;
  auto nibble = [](char c)->int { if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; };
  for (size_t i=0; i<outLen; i++) {
    int a=nibble(hex[i*2]), b=nibble(hex[i*2+1]);
    if(a<0||b<0)return false; out[i]=(uint8_t)((a<<4)|b);
  }
  return true;
}
bool deriveKey(const String &passphrase, const uint8_t *salt, uint8_t key[32]) {
  const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  if (!info) return false;
  mbedtls_md_context_t ctx; mbedtls_md_init(&ctx);
  int rc = mbedtls_md_setup(&ctx, info, 1);
  if (rc == 0) rc = mbedtls_pkcs5_pbkdf2_hmac(&ctx,
      (const unsigned char*)passphrase.c_str(), passphrase.length(),
      salt, 16, PBKDF2_ITERATIONS, 32, key);
  mbedtls_md_free(&ctx);
  return rc == 0;
}
bool encryptCredentials(const String &plain, const String &passphrase) {
  esp_fill_random(encSalt, sizeof(encSalt)); esp_fill_random(encNonce, sizeof(encNonce));
  uint8_t key[32]; if (!deriveKey(passphrase, encSalt, key)) return false;
  std::unique_ptr<uint8_t[]> cipher(new uint8_t[plain.length() + 1]);
  mbedtls_gcm_context gcm; mbedtls_gcm_init(&gcm);
  int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
  if (rc == 0) rc = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT,
      plain.length(), encNonce, sizeof(encNonce), nullptr, 0,
      (const unsigned char*)plain.c_str(), cipher.get(), sizeof(encTag), encTag);
  mbedtls_gcm_free(&gcm); memset(key, 0, sizeof(key));
  if (rc != 0) return false;
  encryptedBlob = bytesToHex(cipher.get(), plain.length());
  prefs.begin("codexmon", false);
  prefs.putString("blob", encryptedBlob);
  prefs.putString("salt", bytesToHex(encSalt, sizeof(encSalt)));
  prefs.putString("nonce", bytesToHex(encNonce, sizeof(encNonce)));
  prefs.putString("tag", bytesToHex(encTag, sizeof(encTag)));
  prefs.end();
  return true;
}
bool decryptCredentials(const String &passphrase) {
  prefs.begin("codexmon", true);
  String blob = prefs.getString("blob", "");
  String saltHex = prefs.getString("salt", "");
  String nonceHex = prefs.getString("nonce", "");
  String tagHex = prefs.getString("tag", "");
  prefs.end();
  if (!blob.length() || blob.length() % 2) return false;
  if (!hexToBytes(saltHex, encSalt, sizeof(encSalt)) ||
      !hexToBytes(nonceHex, encNonce, sizeof(encNonce)) ||
      !hexToBytes(tagHex, encTag, sizeof(encTag))) return false;
  size_t n = blob.length() / 2;
  std::unique_ptr<uint8_t[]> cipher(new uint8_t[n + 1]);
  std::unique_ptr<uint8_t[]> plain(new uint8_t[n + 1]);
  if (!hexToBytes(blob, cipher.get(), n)) return false;
  uint8_t key[32]; if (!deriveKey(passphrase, encSalt, key)) return false;
  mbedtls_gcm_context gcm; mbedtls_gcm_init(&gcm);
  int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
  if (rc == 0) rc = mbedtls_gcm_auth_decrypt(&gcm, n, encNonce, sizeof(encNonce),
      nullptr, 0, encTag, sizeof(encTag), cipher.get(), plain.get());
  mbedtls_gcm_free(&gcm); memset(key, 0, sizeof(key));
  if (rc != 0) return false;
  plain[n] = 0;
  JsonDocument doc;
  if (deserializeJson(doc, (char*)plain.get()) != DeserializationError::Ok) return false;
  accessToken = String((const char*)(doc["access_token"] | ""));
  accountId = String((const char*)(doc["account_id"] | ""));
  memset(plain.get(), 0, n);
  authUnlocked = accessToken.length() > 20;
  return authUnlocked;
}

String makeDashboard() {
  String s = pageStart("Codex Usage Monitor");
  s += F("<h1>Codex Usage Monitor</h1><p class='muted'>ESP32 standalone monitor · no computer-side service</p>");
  if (!authUnlocked) {
    s += F("<div class='card'><h2>Unlock saved credentials</h2><p class='muted'>Enter the encryption passphrase you created during setup. This is not your ChatGPT password.</p>"
           "<form method='post' action='/unlock'><input type='password' name='passphrase' minlength='10' required placeholder='Encryption passphrase'>"
           "<button>Unlock</button></form></div>");
    s += F("<div class='card'><h3>First-time setup?</h3><a href='/setup-auth'>Configure Codex credentials</a></div>");
  } else {
    s += F("<div class='card'><div class='muted'>Plan</div><h2>");
    s += htmlEscape(planType); s += F("</h2>");
    auto card = [&](const char *name, bool present, float used, uint32_t reset) {
      s += F("<div class='card'><h3>"); s += name; s += F("</h3>");
      if (!present) { s += F("<p>Not reported by the endpoint.</p>"); }
      else {
        s += F("<h2>"); s += String(used, 0); s += F("% used</h2><div class='bar'><div class='fill ");
        if (used >= 90) s += F("danger"); else if (used >= 70) s += F("warn");
        s += F("' style='width:"); s += String(constrain(used,0.0f,100.0f), 1); s += F("%'></div></div><p>");
        s += F("Resets: "); s += (reset ? String((unsigned long)reset) : String("unknown"));
        s += F(" (Unix time)</p>");
      }
      s += F("</div>");
    };
    card("Short usage window", haveShort, shortUsed, shortResetAt);
    card("Weekly usage window", haveWeekly, weeklyUsed, weeklyResetAt);
    s += F("<p class='muted'>Status: "); s += htmlEscape(statusText); s += F("</p>");
    s += F("<form method='post' action='/refresh'><button>Refresh now</button></form>"
           "<form method='post' action='/lock'><button>Lock credentials in RAM</button></form>"
           "<form method='post' action='/clear'><button>Erase saved Codex credentials</button></form>");
  }
  s += F("<div class='card'><h3>Network</h3><p>Wi-Fi: "); s += htmlEscape(WiFi.SSID());
  s += F("</p><p>Device IP: "); s += WiFi.localIP().toString();
  s += F("</p><p class='muted'>This page uses HTTP on your local network. Do not use it on an untrusted Wi-Fi network.</p></div>");
  s += pageEnd(); return s;
}

String setupAuthPage() {
  String s = pageStart("Configure Codex");
  s += F("<h1>Connect Codex usage</h1><div class='card'><p><b>Do not enter your ChatGPT account password.</b> This device cannot use an official supported embedded ChatGPT sign-in flow. Paste a Codex OAuth access token obtained locally from your own Codex login, plus its account ID.</p>"
         "<p class='muted'>The endpoint is undocumented and may change. Credentials are encrypted at rest with AES-256-GCM and a passphrase-derived key. You must re-enter the passphrase after reboot.</p>"
         "<form method='post' action='/save-auth'>"
         "<label>OAuth access token</label><input name='token' type='password' autocomplete='off' minlength='20' required>"
         "<label>ChatGPT account ID</label><input name='account' autocomplete='off' required>"
         "<label>Encryption passphrase (10+ characters)</label><input name='passphrase' type='password' minlength='10' required>"
         "<label>Confirm passphrase</label><input name='confirm' type='password' minlength='10' required>"
         "<button>Encrypt and save</button></form></div><p><a href='/'>Back to dashboard</a></p>");
  s += pageEnd(); return s;
}

void startSetupAP() {
  WiFi.mode(WIFI_AP_STA);
  uint64_t mac = ESP.getEfuseMac();
  char suffix[7]; snprintf(suffix, sizeof(suffix), "%06llX", (unsigned long long)(mac & 0xFFFFFF));
  apName = String(AP_PREFIX) + suffix;
  apPassword = String("Setup-") + suffix;
  WiFi.softAP(apName.c_str(), apPassword.c_str());
  IPAddress ip(192,168,4,1);
  dnsServer.start(53, "*", ip);
  statusText = "Setup AP active";
  Serial.printf("\nSetup AP: %s\nPassword: %s\nOpen http://192.168.4.1\n", apName.c_str(), apPassword.c_str());
  tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.setTextSize(2);
  tft.setCursor(8, 8); tft.println("WIFI SETUP");
  tft.setTextSize(1); tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(8, 50); tft.println("Connect phone to:");
  tft.setCursor(8, 65); tft.println(apName);
  tft.setCursor(8, 88); tft.println("Password:");
  tft.setCursor(8, 103); tft.println(apPassword);
  tft.setCursor(8, 135); tft.println("Open browser:");
  tft.setCursor(8, 150); tft.println("192.168.4.1");
}
void saveWifi() {
  String ssid = server.arg("ssid"), pass = server.arg("password");
  if (!ssid.length()) { server.send(400, "text/plain", "SSID required"); return; }
  prefs.begin("codexmon", false); prefs.putString("ssid", ssid); prefs.putString("wpass", pass); prefs.end();
  server.send(200, "text/html", pageStart("Wi-Fi saved") + "<h2>Wi-Fi saved</h2><p>Restarting and connecting to the selected network.</p>" + pageEnd());
  delay(800); ESP.restart();
}
void handleRoot() {
  if (WiFi.getMode() & WIFI_AP) {
    String s = pageStart("Wi-Fi Setup");
    s += F("<h1>Wi-Fi setup</h1><div class='card'><p>Choose the 2.4 GHz Wi-Fi network the monitor should use.</p>"
           "<form method='post' action='/save-wifi'><label>Wi-Fi name (SSID)</label><input name='ssid' required>"
           "<label>Wi-Fi password</label><input type='password' name='password' autocomplete='new-password'>"
           "<button>Save Wi-Fi and restart</button></form></div>");
    s += pageEnd(); server.send(200, "text/html", s); return;
  }
  server.send(200, "text/html", makeDashboard());
}
void handleSaveAuth() {
  String token = server.arg("token"), acct = server.arg("account");
  String pass = server.arg("passphrase"), confirm = server.arg("confirm");
  if (token.length() < 20 || !acct.length() || pass.length() < 10 || pass != confirm) {
    server.send(400, "text/html", pageStart("Invalid input") + "<h2>Invalid input</h2><p>Check token, account ID, matching passphrases and minimum passphrase length.</p><a href='/setup-auth'>Try again</a>" + pageEnd()); return;
  }
  JsonDocument doc; doc["access_token"] = token; doc["account_id"] = acct;
  String plain; serializeJson(doc, plain);
  bool ok = encryptCredentials(plain, pass);
  token = ""; pass = ""; confirm = "";
  if (!ok) { server.send(500, "text/plain", "Encryption failed"); return; }
  // Keep credentials available only in RAM for this session; ciphertext is stored in NVS.
  accountId = acct; accessToken = String((const char*)(doc["access_token"] | ""));
  authUnlocked = accessToken.length() > 20;
  statusText = "Credentials saved";
  server.send(200, "text/html", pageStart("Saved") + "<h2>Credentials encrypted and saved.</h2><p>Fetching usage now. After a reboot, enter your encryption passphrase to unlock them.</p><a href='/' >Open dashboard</a>" + pageEnd());
  if (authUnlocked) { lastPoll = millis(); fetchUsage(); }
}
void handleUnlock() {
  String pass = server.arg("passphrase");
  bool ok = decryptCredentials(pass);
  pass = "";
  if (!ok) { statusText = "Unlock failed"; server.send(401, "text/html", pageStart("Unlock failed") + "<h2>Could not unlock</h2><p>Wrong passphrase or damaged saved data.</p><a href='/'>Try again</a>" + pageEnd()); return; }
  server.send(200, "text/html", pageStart("Unlocked") + "<h2>Unlocked</h2><p>Fetching usage now…</p><a href='/refresh'>Continue</a>" + pageEnd());
  fetchUsage();
}
void handleLock() { accessToken = ""; accountId = ""; authUnlocked = false; statusText = "Locked"; server.sendHeader("Location", "/"); server.send(303); }
void handleClear() {
  accessToken = ""; accountId = ""; authUnlocked = false;
  prefs.begin("codexmon", false); prefs.remove("blob"); prefs.remove("salt"); prefs.remove("nonce"); prefs.remove("tag"); prefs.end();
  statusText = "Credentials erased"; server.sendHeader("Location", "/setup-auth"); server.send(303);
}
void handleRefresh() { fetchUsage(); drawStatusScreen(); server.sendHeader("Location", "/"); server.send(303); }

bool parseWindow(JsonVariantConst w, float &used, uint32_t &resetAt, uint32_t &duration) {
  if (w.isNull() || !w.is<JsonObjectConst>()) return false;
  JsonObjectConst o = w.as<JsonObjectConst>();
  if (o["used_percent"].is<float>() || o["used_percent"].is<int>()) used = o["used_percent"].as<float>();
  else if (o["percent_left"].is<float>() || o["percent_left"].is<int>()) used = 100.0f - o["percent_left"].as<float>();
  else return false;
  used = constrain(used, 0.0f, 100.0f);
  duration = o["limit_window_seconds"] | 0U;
  resetAt = o["reset_at"] | 0U;
  if (!resetAt && o["reset_time_ms"].is<uint64_t>()) resetAt = (uint32_t)(o["reset_time_ms"].as<uint64_t>() / 1000ULL);
  if (!resetAt) {
    uint32_t after = o["reset_after_seconds"] | 0U;
    if (after) resetAt = (uint32_t)time(nullptr) + after;
  }
  return true;
}
bool fetchUsage() {
  if (!authUnlocked || accessToken.length() < 20) { statusText = "Unlock credentials first"; drawStatusScreen(); return false; }
  if (WiFi.status() != WL_CONNECTED) { statusText = "Wi-Fi disconnected"; drawStatusScreen(); return false; }
  WiFiClientSecure client;
  client.setCACertBundle(x509_crt_bundle);
  HTTPClient http; http.setTimeout(15000);
  if (!http.begin(client, USAGE_URL)) { statusText = "HTTPS init failed"; drawStatusScreen(); return false; }
  http.addHeader("Authorization", "Bearer " + accessToken);
  http.addHeader("ChatGPT-Account-Id", accountId);
  http.addHeader("Accept", "application/json");
  http.addHeader("Origin", "https://chatgpt.com");
  http.addHeader("Referer", "https://chatgpt.com/");
  http.addHeader("User-Agent", "Mozilla/5.0");
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    statusText = "API HTTP " + String(code);
    if (code == 401 || code == 403) statusText = "Token expired/rejected";
    http.end(); drawStatusScreen(); return false;
  }
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) { statusText = "Bad JSON response"; drawStatusScreen(); return false; }
  JsonVariantConst rl = doc["rate_limit"];
  if (rl.isNull()) rl = doc["rate_limits"];
  if (rl.isNull()) { statusText = "No rate_limit field"; drawStatusScreen(); return false; }

  haveShort = haveWeekly = false;
  JsonVariantConst win1 = rl["primary_window"];
  if (win1.isNull()) win1 = rl["five_hour"];
  if (win1.isNull()) win1 = rl["primary"];
  JsonVariantConst win2 = rl["secondary_window"];
  if (win2.isNull()) win2 = rl["weekly"];
  if (win2.isNull()) win2 = rl["secondary"];

  float u; uint32_t r, d;
  if (parseWindow(win1, u, r, d)) {
    if (d >= 172800) { haveWeekly=true; weeklyUsed=u; weeklyResetAt=r; }
    else { haveShort=true; shortUsed=u; shortResetAt=r; }
  }
  if (parseWindow(win2, u, r, d)) {
    if (d >= 172800) { haveWeekly=true; weeklyUsed=u; weeklyResetAt=r; }
    else { haveShort=true; shortUsed=u; shortResetAt=r; }
  }
  planType = String((const char*)(doc["plan_type"] | "ChatGPT"));
  if (!haveShort && !haveWeekly) { statusText = "No usage windows returned"; drawStatusScreen(); return false; }
  statusText = "Updated OK"; lastGood = millis(); drawStatusScreen(); return true;
}

void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save-wifi", HTTP_POST, saveWifi);
  server.on("/setup-auth", HTTP_GET, [](){ server.send(200, "text/html", setupAuthPage()); });
  server.on("/save-auth", HTTP_POST, handleSaveAuth);
  server.on("/unlock", HTTP_POST, handleUnlock);
  server.on("/lock", HTTP_POST, handleLock);
  server.on("/clear", HTTP_POST, handleClear);
  server.on("/refresh", HTTP_GET, handleRefresh);
  server.on("/refresh", HTTP_POST, handleRefresh);
  server.onNotFound([]() {
    if (WiFi.getMode() & WIFI_AP) { server.sendHeader("Location", "http://192.168.4.1/"); server.send(302); }
    else server.send(404, "text/plain", "Not found");
  });
  server.begin();
}
void setup() {
  Serial.begin(115200);
  pinMode(46, OUTPUT); digitalWrite(46, HIGH);
  tft.init(); tft.setRotation(0); tft.fillScreen(TFT_BLACK);
  prefs.begin("codexmon", true);
  wifiSsid = prefs.getString("ssid", ""); wifiPassword = prefs.getString("wpass", "");
  bool hasSavedAuth = prefs.getString("blob", "").length() > 0;
  prefs.end();

  if (!wifiSsid.length()) {
    startSetupAP();
  } else {
    WiFi.mode(WIFI_STA); WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
    statusText = "Connecting Wi-Fi";
    uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started < 20000) delay(250);
    if (WiFi.status() != WL_CONNECTED) startSetupAP();
    else {
      configTime(0, 0, "pool.ntp.org", "time.google.com");
      statusText = hasSavedAuth ? "Locked - open webpage" : "Configure credentials";
      Serial.printf("Dashboard: http://%s/\n", WiFi.localIP().toString().c_str());
    }
  }
  setupRoutes();
  drawStatusScreen();
}
void loop() {
  if (WiFi.getMode() & WIFI_AP) dnsServer.processNextRequest();
  server.handleClient();
  if (authUnlocked && WiFi.status() == WL_CONNECTED && millis() - lastPoll >= POLL_MS) {
    lastPoll = millis(); fetchUsage();
  }
  delay(2);
}
