#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "BoardDisplay.h"
#include "BoardMotion.h"
#include "CodexUi.h"
#include <atomic>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "WebAssets.h"
#include "WebFonts.h"
#include "WifiProfiles.h"
#include <driver/temperature_sensor.h>
#include <math.h>
#include <esp_ota_ops.h>
#include <esp_app_desc.h>
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
BoardDisplay lcd;
LGFX_Sprite tft(&lcd);

String apName, apPassword, statusText = "Booting";
WifiProfiles wifiProfiles;
bool migrationRequired=false;
bool wifiAttempt=false, wifiConnected=false, wifiRestart=false;
size_t wifiNext=0;
uint32_t wifiAttemptAt=0,wifiRetryAt=0;
volatile uint8_t wifiFailureReason=0;
String accessToken, accountId, planType = "ChatGPT";
bool accountReady = false;
bool haveShort = false, haveWeekly = false;
float shortUsed = 0, weeklyUsed = 0;
uint32_t shortResetAt = 0, weeklyResetAt = 0;
uint32_t lastPoll = 0, lastGood = 0;
uint8_t encSalt[16], encNonce[12], encTag[16];
String encryptedBlob;
BoardMotion motion;
int screenRotation=1;
std::atomic<bool> detailsPage{false};
bool screenPreview=false;
uint32_t lastDraw=0, lastTemperature=0, previewUntil=0;
float chipTemperature=NAN;
temperature_sensor_handle_t temperatureSensor=nullptr;


bool fetchUsage();
void bootSlotA();

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
  String s=F("<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><meta name='theme-color' content='#090c10'><title>");
  s+=htmlEscape(title);s+=F("</title><style>");s+=FPSTR(WEB_CSS);
  s+=F("</style></head><body><main>");return s;
}
String pageEnd(){return F("</main></body></html>");}

String resetIn(uint32_t reset) {
  uint32_t now=(uint32_t)time(nullptr);
  if(!reset)return "Awaiting data";
  if(reset<=now)return "Reset due";
  uint32_t m=(reset-now)/60,h=m/60,d=h/24;
  if(d)return String(d)+"d "+String(h%24)+"h";
  if(h)return String(h)+"h "+String(m%60)+"m";
  return m?String(m)+"m":"<1m";
}
// LVGL, LCD and IMU have one owner. Network requests can block without freezing
// tilt or the brief page transition. Only fixed-size snapshots cross tasks.
struct DisplayMessage { CodexUiModel ui; uint32_t previewUntil; int rotation; };
QueueHandle_t displayQueue=nullptr;
SemaphoreHandle_t frameMutex=nullptr;
static lv_disp_draw_buf_t lvDrawBuffer;
static lv_disp_drv_t lvDriver;
static lv_disp_t *lvDisplay=nullptr;
static lv_color_t *lvPixels=nullptr;
static int requestedRotation=1;
static volatile uint32_t uiFrames=0;
static bool displayReady=false;
void lvFlush(lv_disp_drv_t *driver,const lv_area_t *area,lv_color_t *pixels){
  // LVGL uses native RGB565; Lovyan's typed overload handles wire byte order.
  int w=area->x2-area->x1+1,h=area->y2-area->y1+1;
  tft.pushImage(area->x1,area->y1,w,h,(lgfx::rgb565_t*)pixels);
  lcd.pushImage(area->x1,area->y1,w,h,(lgfx::rgb565_t*)pixels);
  uiFrames++;lv_disp_flush_ready(driver);
}
void setScreenRotation(int rotation){requestedRotation=rotation&3;}
void displayTask(void*){
  DisplayMessage current={};current.ui.short_left=current.ui.weekly_left=-1;
  strcpy(current.ui.badge,"SETUP");strcpy(current.ui.freshness,"Starting");
  uint32_t tick=millis(),tempAt=0,pressed=0;int page=-1;
  bool released=false,wasDown=false,dirty=true;
  for(;;){
    uint32_t now=millis();lv_tick_inc(now-tick);tick=now;
    DisplayMessage next;bool changed=xQueueReceive(displayQueue,&next,0)==pdTRUE;
    if(changed)current=next;
    bool down=digitalRead(0)==LOW;
    if(!released){if(!down)released=true;}
    else{
      if(down&&!wasDown)pressed=now;
      if(!down&&wasDown&&now-pressed>=40&&now-pressed<1000)detailsPage=!detailsPage.load();
      if(down&&now-pressed>=3000){bootSlotA();pressed=now;}
      wasDown=down;
    }
    if(current.ui.page!=2){int desired=detailsPage.load()?1:0;if(desired!=current.ui.page){current.ui.page=desired;changed=true;}}
    bool preview=current.previewUntil&&int32_t(current.previewUntil-now)>0;
    xSemaphoreTake(frameMutex,portMAX_DELAY);
    int rotation=preview?current.rotation:motion.poll(now,screenRotation);
    bool rotated=rotation!=screenRotation;
    // LVGL 8.3 can invalidate scr_to_load when an active screen slide is
    // interrupted. Coalesce updates and finish the 192 ms slide first.
    bool idle=lv_anim_count_running()==0;
    if(rotated&&idle){
      screenRotation=rotation;tft.deleteSprite();lcd.setRotation(rotation);
      tft.createSprite(lcd.width(),lcd.height());
      lvDriver.hor_res=lcd.width();lvDriver.ver_res=lcd.height();lv_disp_drv_update(lvDisplay,&lvDriver);
    }
    if(temperatureSensor&&now-tempAt>=5000){
      tempAt=now;
      if(temperature_sensor_get_celsius(temperatureSensor,&chipTemperature)!=ESP_OK)chipTemperature=NAN;
      if(current.ui.page==1)changed=true;
    }
    dirty=dirty||changed||rotated;
    if(dirty&&idle){
      if(isfinite(chipTemperature))snprintf(current.ui.temperature,sizeof(current.ui.temperature),"%.1f C",chipTemperature);
      else strlcpy(current.ui.temperature,"Unavailable",sizeof(current.ui.temperature));
      strlcpy(current.ui.rotation,motion.available?"Automatic":"Landscape",sizeof(current.ui.rotation));
      lv_obj_t *screen=codex_screen(lcd.width(),lcd.height(),&current.ui);
      bool transition=page>=0&&page!=current.ui.page&&!rotated;
      lv_scr_load_anim(screen,transition?(current.ui.page?LV_SCR_LOAD_ANIM_MOVE_LEFT:LV_SCR_LOAD_ANIM_MOVE_RIGHT):LV_SCR_LOAD_ANIM_NONE,transition?192:0,0,true);
      page=current.ui.page;dirty=false;
    }
    lv_timer_handler();
    xSemaphoreGive(frameMutex);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
bool beginDisplay(){
  lcd.setRotation(1);tft.setPsram(true);tft.setColorDepth(16);
  if(!tft.createSprite(lcd.width(),lcd.height()))return false;
  lvPixels=(lv_color_t*)heap_caps_malloc(320*24*sizeof(lv_color_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if(!lvPixels)return false;
  frameMutex=xSemaphoreCreateMutex();displayQueue=xQueueCreate(1,sizeof(DisplayMessage));
  if(!frameMutex||!displayQueue)return false;
  lv_init();lv_disp_draw_buf_init(&lvDrawBuffer,lvPixels,nullptr,320*24);
  lv_disp_drv_init(&lvDriver);lvDriver.hor_res=320;lvDriver.ver_res=172;
  lvDriver.draw_buf=&lvDrawBuffer;lvDriver.flush_cb=lvFlush;
  lvDisplay=lv_disp_drv_register(&lvDriver);
  displayReady=xTaskCreatePinnedToCore(displayTask,"codex-ui",8192,nullptr,1,nullptr,1)==pdPASS;
  return displayReady;
}
void drawStatusScreen(){
  lastDraw=millis();if(!displayReady)return;
  DisplayMessage message={};CodexUiModel &m=message.ui;
  bool preview=screenPreview,ap=WiFi.getMode()&WIFI_AP;
  m.page=ap?2:detailsPage?1:0;
  m.short_left=preview?62:haveShort?(int)roundf(constrain(100.f-shortUsed,0.f,100.f)):-1;
  m.weekly_left=preview?13:haveWeekly?(int)roundf(constrain(100.f-weeklyUsed,0.f,100.f)):-1;
  String badge=preview?"PREVIEW":accountReady?planType:"SETUP";badge.toUpperCase();
  strlcpy(m.badge,badge.c_str(),sizeof(m.badge));
  strlcpy(m.short_reset,preview?"Resets 3h 34m":haveShort?("Resets "+resetIn(shortResetAt)).c_str():"Awaiting data",sizeof(m.short_reset));
  strlcpy(m.weekly_reset,preview?"Resets 4d 17h":haveWeekly?("Resets "+resetIn(weeklyResetAt)).c_str():"Awaiting data",sizeof(m.weekly_reset));
  m.online=WiFi.status()==WL_CONNECTED;
  strlcpy(m.ssid,(ap?apName:WiFi.SSID()).c_str(),sizeof(m.ssid));
  strlcpy(m.address,WiFi.localIP().toString().c_str(),sizeof(m.address));
  strlcpy(m.setup_password,apPassword.c_str(),sizeof(m.setup_password));
  String network=m.online?WiFi.SSID()+" / Online":"Connecting Wi-Fi";
  // Reserve the right half of the landscape footer for freshness.
  if(network.length()>20)network=network.substring(0,17)+"...";
  strlcpy(m.network,network.c_str(),sizeof(m.network));
  String fresh=preview?"Preview":!accountReady?"Setup on web":statusText=="Updated OK"?"Updated "+String((millis()-lastGood)/60000)+"m":lastGood?"Saved / retrying":"Connecting";
  strlcpy(m.freshness,fresh.c_str(),sizeof(m.freshness));
  message.previewUntil=preview?previewUntil:0;message.rotation=requestedRotation;
  xQueueOverwrite(displayQueue,&message);
}
void serviceDisplay(){
  uint32_t now=millis();
  if(screenPreview&&int32_t(now-previewUntil)>0){screenPreview=false;drawStatusScreen();}
  if(uint32_t(now-lastDraw)>15000)drawStatusScreen();
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
  return mbedtls_pkcs5_pbkdf2_hmac_ext(
      MBEDTLS_MD_SHA256,
      (const unsigned char*)passphrase.c_str(), passphrase.length(),
      salt, 16, PBKDF2_ITERATIONS, 32, key) == 0;
}
bool encryptCredentials(const String &plain, const String &passphrase, const char *space = "codexmon", const char *recordKey = "auth_v1", bool deviceKey = false) {
  esp_fill_random(encSalt, sizeof(encSalt)); esp_fill_random(encNonce, sizeof(encNonce));
  uint8_t key[32]; if (!(deviceKey ? hexToBytes(passphrase,key,32) : deriveKey(passphrase, encSalt, key))) return false;
  std::unique_ptr<uint8_t[]> cipher(new uint8_t[plain.length() + 1]);
  mbedtls_gcm_context gcm; mbedtls_gcm_init(&gcm);
  int rc = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256);
  if (rc == 0) rc = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT,
      plain.length(), encNonce, sizeof(encNonce), nullptr, 0,
      (const unsigned char*)plain.c_str(), cipher.get(), sizeof(encTag), encTag);
  mbedtls_gcm_free(&gcm); memset(key, 0, sizeof(key));
  if (rc != 0) return false;
  // One binary NVS blob is atomic and avoids hex-string size/fragmentation limits.
  size_t size = 48 + plain.length();
  std::unique_ptr<uint8_t[]> record(new uint8_t[size]);
  memcpy(record.get(), "CDA1", 4);
  memcpy(record.get()+4, encSalt, 16);
  memcpy(record.get()+20, encNonce, 12);
  memcpy(record.get()+32, encTag, 16);
  memcpy(record.get()+48, cipher.get(), plain.length());
  if (!prefs.begin(space, false)) return false;
  bool saved = prefs.putBytes(recordKey, record.get(), size) == size;
  if (saved && !deviceKey) { prefs.remove("blob"); prefs.remove("salt"); prefs.remove("nonce"); prefs.remove("tag"); }
  prefs.end();
  if (!saved) return false;
  return true;
}
bool decryptCredentials(const String &passphrase, const char *space = "codexmon", const char *recordKey = "auth_v1", bool deviceKey = false) {
  if (!prefs.begin(space, true)) return false;
  String blob;
  bool valid = false;
  size_t savedSize = prefs.getBytesLength(recordKey);
  if (savedSize >= 48 && savedSize <= 16384) {
    std::unique_ptr<uint8_t[]> record(new uint8_t[savedSize]);
    valid = prefs.getBytes(recordKey, record.get(), savedSize) == savedSize && memcmp(record.get(), "CDA1", 4) == 0;
    if (valid) {
      memcpy(encSalt, record.get()+4, 16); memcpy(encNonce, record.get()+20, 12); memcpy(encTag, record.get()+32, 16);
      blob = bytesToHex(record.get()+48, savedSize-48);
    }
  } else if (!savedSize && !deviceKey) {
    // Read earlier hex records without requiring users to replace valid credentials.
    blob = prefs.getString("blob", "");
    valid = hexToBytes(prefs.getString("salt", ""), encSalt, sizeof(encSalt)) &&
            hexToBytes(prefs.getString("nonce", ""), encNonce, sizeof(encNonce)) &&
            hexToBytes(prefs.getString("tag", ""), encTag, sizeof(encTag));
  }
  prefs.end();
  if (!valid || !blob.length() || blob.length() % 2) return false;
  size_t n = blob.length() / 2;
  std::unique_ptr<uint8_t[]> cipher(new uint8_t[n + 1]);
  std::unique_ptr<uint8_t[]> plain(new uint8_t[n + 1]);
  if (!hexToBytes(blob, cipher.get(), n)) return false;
  uint8_t key[32]; if (!(deviceKey ? hexToBytes(passphrase,key,32) : deriveKey(passphrase, encSalt, key))) return false;
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
  accountReady = accessToken.length() > 20;
  return accountReady;
}

String deviceSecret(bool create=false,const char *space="codexmon") {
  if(!prefs.begin(space,!create))return "";
  uint8_t key[32];bool ok=prefs.getBytesLength("auto_key")==sizeof(key)&&prefs.getBytes("auto_key",key,sizeof(key))==sizeof(key);
  if(!ok&&create){esp_fill_random(key,sizeof(key));ok=prefs.putBytes("auto_key",key,sizeof(key))==sizeof(key);}
  prefs.end();String secret=ok?bytesToHex(key,sizeof(key)):String("");memset(key,0,sizeof(key));return secret;
}
bool loadAutomaticAuth(const char *space="codexmon") {
  String key=deviceSecret(false,space);
  return key.length()&&decryptCredentials(key,space,"auto_auth",true);
}
bool saveAutomaticAuth(const String &plain,const char *space="codexmon") {
  String key=deviceSecret(true,space);
  if(!key.length()||!encryptCredentials(plain,key,space,"auto_auth",true)||!loadAutomaticAuth(space))return false;
  // Keep the original passphrase record until the automatic record is verified.
  if(prefs.begin(space,false)){
    prefs.remove("auth_v1");prefs.remove("blob");prefs.remove("salt");prefs.remove("nonce");prefs.remove("tag");prefs.end();
  }
  return true;
}
bool haveLegacyCredentials() {
  if(!prefs.begin("codexmon",true))return false;
  bool have=prefs.getBytesLength("auth_v1")>0||prefs.getString("blob","").length()>0;
  prefs.end();return have;
}
bool haveSavedCredentials() {
  if (!prefs.begin("codexmon", true)) return false;
  bool have = prefs.getBytesLength("auto_auth") > 0 || prefs.getBytesLength("auth_v1") > 0 || prefs.getString("blob", "").length() > 0;
  prefs.end(); return have;
}

String dashboardState(){
  JsonDocument doc;doc["plan"]=planType;doc["ready"]=accountReady;doc["migrationRequired"]=migrationRequired;doc["savedCredentials"]=haveSavedCredentials();
  doc["status"]=statusText;doc["ssid"]=WiFi.SSID();doc["ip"]=WiFi.localIP().toString();
  doc["imu"]=motion.available;doc["landscape"]=(screenRotation&1)!=0;
  if(isfinite(chipTemperature))doc["temperature"]=chipTemperature;else doc["temperature"]=nullptr;
  if(lastGood)doc["ageSeconds"]=(millis()-lastGood)/1000;else doc["ageSeconds"]=nullptr;
  doc["short"]["present"]=haveShort;doc["short"]["left"]=constrain(100.f-shortUsed,0.f,100.f);doc["short"]["resetAt"]=shortResetAt;
  doc["weekly"]["present"]=haveWeekly;doc["weekly"]["left"]=constrain(100.f-weeklyUsed,0.f,100.f);doc["weekly"]["resetAt"]=weeklyResetAt;
  String data;serializeJson(doc,data);data.replace("<","\\u003c");return data;
}
String makeDashboard(){
  String body=FPSTR(WEB_DASHBOARD);body.replace("{{DATA}}",dashboardState());
  return pageStart("Codex · Usage remaining")+body+pageEnd();
}

String setupAuthPage() {
  String s = pageStart("Configure Codex");
  s += F("<h1>Connect Codex usage</h1><div class='card'><p><b>Do not enter your ChatGPT account password.</b> This device cannot use an official supported embedded ChatGPT sign-in flow. Paste a Codex OAuth access token obtained locally from your own Codex login, plus its account ID.</p>"
         "<p class='muted'>Credentials are encrypted on this device with a stored device key. The dashboard resumes automatically after restart. Anyone with access to the device's flash can recover that key.</p>"
         "<form method='post' action='/save-auth'>"
         "<label>OAuth access token</label><input name='token' type='password' autocomplete='off' minlength='20' required>"
         "<label>ChatGPT account ID</label><input name='account' autocomplete='off' required>"
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
  drawStatusScreen();
}
void serviceWifi() {
  uint32_t now=millis();
  if(wifiRestart){WiFi.disconnect(false,false,1000);wifiAttempt=false;wifiConnected=false;wifiNext=0;wifiRetryAt=now;wifiRestart=false;}
  if(WiFi.status()==WL_CONNECTED){
    if(!wifiConnected){
      wifiConnected=true;wifiAttempt=false;wifiNext=0;
      if(WiFi.getMode()&WIFI_AP){dnsServer.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_STA);}
      WiFi.setSleep(true);configTime(0,0,"pool.ntp.org","time.google.com");
      statusText=accountReady?"Connecting to Codex":migrationRequired?"Finish account upgrade":"Connect Codex account";
      lastPoll=millis()-POLL_MS;drawStatusScreen();
      Serial.printf("Dashboard: http://%s/\n",WiFi.localIP().toString().c_str());
    }
    return;
  }
  if(wifiConnected){wifiConnected=false;wifiAttempt=false;wifiNext=0;wifiRetryAt=now;statusText="Reconnecting Wi-Fi";drawStatusScreen();}
  if(wifiAttempt){
    // Authentication/association and DHCP are separate stages. Some routers need
    // a DHCP retry before assigning an address after moving between subnets.
    bool associated=WiFi.STA.connected();
    uint32_t budget=associated?90000:25000;
    if(associated&&statusText!="Getting network address"){statusText="Getting network address";drawStatusScreen();}
    if(uint32_t(now-wifiAttemptAt)<budget)return;
    WiFi.disconnect(false,false,1000);wifiAttempt=false;
  }
  if(int32_t(now-wifiRetryAt)<0)return;
  if(wifiNext>=wifiProfiles.count){
    if(!(WiFi.getMode()&WIFI_AP))startSetupAP();
    wifiNext=0;wifiRetryAt=now+30000;return;
  }
  const auto &profile=wifiProfiles.items[wifiNext++];
  // Recreate the station interface between routers so DHCP does not carry
  // state from a different subnet. Keep the radio awake until it has an IP;
  // serviceWifi restores modem sleep immediately after a successful connection.
  WiFi.STA.end();WiFi.STA.begin();WiFi.setAutoReconnect(false);WiFi.setSleep(false);
  WiFi.begin(profile.ssid.c_str(),profile.password.c_str());
  wifiAttempt=true;wifiAttemptAt=now;statusText="Connecting Wi-Fi";drawStatusScreen();
}
String wifiPage(const String &message="") {
  String s=pageStart("Wi-Fi networks");
  s+=F("<header class='topbar'><div class='brand'>CODEX <span class='badge'>WI-FI</span></div><a class='button' href='/'>Dashboard</a></header><section class='hero'><div><div class='eyebrow'>Connected wherever you work</div><h1>Your networks.</h1><p>Save up to five 2.4 GHz networks. The monitor tries them in priority order when it starts or loses connection.</p></div></section>");
  if(message.length())s+="<p role='status'>"+htmlEscape(message)+"</p>";
  s+="<div class='card'><h3>Saved networks · "+String(wifiProfiles.count)+" / 5</h3>";
  if(!wifiProfiles.count)s+="<p>No networks saved yet.</p>";
  for(size_t i=0;i<wifiProfiles.count;i++){
    String name=htmlEscape(wifiProfiles.items[i].ssid);
    s+="<div class='network-row'><div><strong>"+name+"</strong><p class='help'>"+String(i==0?"Preferred · ":"")+String(WiFi.status()==WL_CONNECTED&&WiFi.SSID()==wifiProfiles.items[i].ssid?"Connected":"Saved")+"</p></div><div class='settings-actions'>";
    for(const char *action:{"prefer","connect","remove"}){
      s+="<form method='post' action='/wifi-action'><input type='hidden' name='ssid' value='"+name+"'><input type='hidden' name='action' value='"+action+"'><button"+String(String(action)=="remove"?" class='danger'":"")+">"+String(String(action)=="prefer"?"Make preferred":String(action)=="connect"?"Connect":"Remove")+"</button></form>";
    }
    s+="</div></div>";
  }
  s+=F("</div><div class='card'><h3>Add or update a network</h3><p class='help'>Adding a network keeps your current connection. A blank password keeps an existing password; for a new network it means an open network.</p><form method='post' action='/save-wifi'><label for='ssid-input'>Network name (SSID)</label><input id='ssid-input' name='ssid' maxlength='32' required autocomplete='off'><label for='wifi-password'>Password</label><input id='wifi-password' type='password' name='password' maxlength='64' autocomplete='new-password'><label><input type='checkbox' name='open' value='1'> Use an open network (no password)</label><button class='primary'>Save network</button></form></div><p class='help'>If no saved network is reachable, join the setup network shown on the board and open 192.168.4.1. Saved networks are retried automatically.</p>");
  return s+pageEnd();
}
void saveWifi() {
  String ssid=server.arg("ssid"),pass=server.arg("password");int index=wifiProfiles.find(ssid);
  if(!pass.length()&&index>=0&&server.arg("open")!="1")pass=wifiProfiles.items[index].password;
  if(server.arg("open")=="1")pass="";
  if(!WifiProfiles::valid(ssid,pass)){server.send(400,"text/html",wifiPage("Use a 1–32 byte SSID and an 8–63 character password, a 64-digit hexadecimal key, or an open network."));return;}
  if(!wifiProfiles.put(ssid,pass)){server.send(500,"text/html",wifiPage("Could not save. Check that fewer than five networks are saved, then retry."));return;}
  server.send(200,"text/html",wifiPage("Network saved. Your current connection stays active."));
  if(WiFi.status()!=WL_CONNECTED){wifiNext=0;wifiRetryAt=millis();wifiAttempt=false;}
}
void wifiAction() {
  String ssid=server.arg("ssid"),action=server.arg("action");
  if(wifiProfiles.find(ssid)<0){server.send(404,"text/html",wifiPage("Network not found."));return;}
  bool ok=action=="remove"?wifiProfiles.remove(ssid):(action=="prefer"||action=="connect")?wifiProfiles.prefer(ssid):false;
  if(!ok){server.send(500,"text/html",wifiPage("The change could not be saved."));return;}
  String message=action=="remove"?"Network removed from saved networks. Any current connection continues until disconnected.":action=="connect"?"Switching networks. If the address changes, tap BOOT on the board to see its new address.":"Preferred network saved. It will be tried first on the next connection.";
  server.send(200,"text/html",wifiPage(message));
  if(action=="connect")wifiRestart=true;
}
void handleRoot() {
  server.send(200,"text/html",(WiFi.getMode()&WIFI_AP)?wifiPage():makeDashboard());
}
void handleSaveAuth() {
  String token = server.arg("token"), acct = server.arg("account");
  if (token.length() < 20 || !acct.length()) {
    server.send(400, "text/html", pageStart("Invalid input") + "<h2>Check the token and account ID.</h2><a href='/setup-auth'>Try again</a>" + pageEnd()); return;
  }
  JsonDocument doc; doc["access_token"] = token; doc["account_id"] = acct;
  String plain; serializeJson(doc, plain);
  bool ok = saveAutomaticAuth(plain);
  token = ""; plain = "";
  if (!ok) { server.send(500, "text/plain", "Credentials could not be encrypted or saved; nothing new was activated"); return; }
  // Automatic startup loads the device-key encrypted NVS record.
  accountId = acct; accessToken = String((const char*)(doc["access_token"] | ""));
  accountReady = accessToken.length() > 20;
  migrationRequired=false;statusText = "Credentials saved";
  server.send(200, "text/html", pageStart("Saved") + "<h2>Credentials encrypted and saved.</h2><p>Fetching usage now. Updates resume automatically after restart.</p><a href='/' >Open dashboard</a>" + pageEnd());
  if (accountReady) { lastPoll = millis(); fetchUsage(); }
}
void handleMigration() {
  if(!migrationRequired){server.sendHeader("Location","/");server.send(303);return;}
  String pass=server.arg("passphrase");bool ok=decryptCredentials(pass);pass="";
  if(!ok){server.send(401,"text/html",pageStart("Try again")+"<h2>Could not read the previous account setup.</h2><p>Check your previous passphrase and try again.</p><a href='/'>Back</a>"+pageEnd());return;}
  JsonDocument doc;doc["access_token"]=accessToken;doc["account_id"]=accountId;String plain;serializeJson(doc,plain);
  if(!saveAutomaticAuth(plain)){accountReady=false;accessToken="";accountId="";server.send(500,"text/plain","Could not save automatic startup. The previous credentials were preserved; please retry.");return;}
  migrationRequired=false;server.send(200,"text/html",pageStart("Ready")+"<h2>All set.</h2><p>Your dashboard will now start automatically.</p><a href='/'>Open dashboard</a>"+pageEnd());
  lastPoll=millis();fetchUsage();
}
void handleClear() {
  accessToken = ""; accountId = ""; accountReady = false;
  prefs.begin("codexmon", false); prefs.remove("auto_auth"); prefs.remove("auto_key"); prefs.remove("auth_v1"); prefs.remove("blob"); prefs.remove("salt"); prefs.remove("nonce"); prefs.remove("tag"); prefs.end();
  migrationRequired=false;haveShort=haveWeekly=false;lastGood=0;statusText = "Credentials erased"; server.sendHeader("Location", "/setup-auth"); server.send(303);
}
void handleRefresh() { lastPoll=millis();fetchUsage(); drawStatusScreen(); server.sendHeader("Location", "/"); server.send(303); }

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
bool ensureClock() {
  if (time(nullptr) >= 1700000000) return true;
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  uint32_t started = millis();
  while (time(nullptr) < 1700000000 && millis() - started < 10000) delay(50);
  return time(nullptr) >= 1700000000;
}

bool fetchUsage() {
  if (!accountReady || accessToken.length() < 20) { statusText = "Connect Codex account"; drawStatusScreen(); return false; }
  if (WiFi.status() != WL_CONNECTED) { statusText = "Wi-Fi disconnected"; drawStatusScreen(); return false; }
  if (!ensureClock()) { statusText = "Clock sync failed; check NTP"; drawStatusScreen(); return false; }
  WiFiClientSecure client;
  client.useBuiltinCACertBundle();
  client.setHandshakeTimeout(15);
  HTTPClient http; http.setTimeout(15000); http.setConnectTimeout(15000);
  if (!http.begin(client, USAGE_URL)) { statusText = "HTTPS init failed"; drawStatusScreen(); return false; }
  http.addHeader("Authorization", "Bearer " + accessToken);
  http.addHeader("ChatGPT-Account-Id", accountId);
  http.addHeader("Accept", "application/json");
  http.addHeader("Origin", "https://chatgpt.com");
  http.addHeader("Referer", "https://chatgpt.com/");
  http.addHeader("User-Agent", "Mozilla/5.0");
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    statusText = code < 0 ? "HTTPS: " + HTTPClient::errorToString(code) : "API HTTP " + String(code);
    if (code < 0) {
      char error[128] = {};
      int tlsError = client.lastError(error, sizeof(error));
      if (tlsError) statusText += " / " + String(error);
    }
    if (code == 401) statusText = "Token expired/rejected (401)";
    if (code == 403) statusText = "Access denied (403)";
    http.end(); drawStatusScreen(); return false;
  }
  JsonDocument doc;
  // HTTPClient removes transfer chunk framing; its raw stream does not.
  DeserializationError err = deserializeJson(doc, http.getString());
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
  server.on("/fonts/agency-regular-v1.woff2",HTTP_GET,[](){
    server.sendHeader("Cache-Control","public, max-age=31536000, immutable");
    server.send_P(200,"font/woff2",(const char*)WEB_FONT_REGULAR,sizeof(WEB_FONT_REGULAR));
  });
  server.on("/fonts/agency-bold-v1.woff2",HTTP_GET,[](){
    server.sendHeader("Cache-Control","public, max-age=31536000, immutable");
    server.send_P(200,"font/woff2",(const char*)WEB_FONT_BOLD,sizeof(WEB_FONT_BOLD));
  });
  server.on("/api/status",HTTP_GET,[](){server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",dashboardState());});
  server.on("/api/refresh",HTTP_POST,[](){lastPoll=millis();fetchUsage();server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",dashboardState());});
  server.on("/boot-a", HTTP_POST, [](){ server.send(200, "text/plain", "Booting ESPINFO Slot A"); delay(200); bootSlotA(); });
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save-wifi", HTTP_POST, saveWifi);
  server.on("/wifi", HTTP_GET, [](){server.send(200,"text/html",wifiPage());});
  server.on("/wifi-action", HTTP_POST, wifiAction);
  server.on("/setup-auth", HTTP_GET, [](){ server.send(200, "text/html", setupAuthPage()); });
  server.on("/save-auth", HTTP_POST, handleSaveAuth);
  server.on("/migrate-auth", HTTP_POST, handleMigration);
  server.on("/clear", HTTP_POST, handleClear);
  server.on("/refresh", HTTP_GET, handleRefresh);
  server.on("/refresh", HTTP_POST, handleRefresh);
  server.onNotFound([]() {
    if (WiFi.getMode() & WIFI_AP) { server.sendHeader("Location", "http://192.168.4.1/"); server.send(302); }
    else server.send(404, "text/plain", "Not found");
  });
  server.begin();
}
void bootSlotA() {
  const esp_partition_t *slot = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, "app0");
  esp_app_desc_t desc;
  if (!slot || esp_ota_get_partition_description(slot, &desc) != ESP_OK || esp_ota_set_boot_partition(slot) != ESP_OK) {
    Serial.println("Slot A validation/selection failed"); return;
  }
  Serial.println("Booting ESPINFO Slot A"); Serial.flush(); delay(100); ESP.restart();
}
void printStatus() {
  const esp_partition_t *running = esp_ota_get_running_partition();
  JsonDocument doc;
  doc["firmware"]="codex-dashboard-slot-b-v3.3-web-instrument";
  doc["partition"]=running->label; doc["address"]=running->address;
  doc["flash_bytes"]=ESP.getFlashChipSize(); doc["psram_bytes"]=ESP.getPsramSize();doc["free_psram"]=ESP.getFreePsram();doc["free_heap"]=ESP.getFreeHeap();
  doc["setup_ap"]=(WiFi.getMode() & WIFI_AP)!=0; doc["ip"]=WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():WiFi.softAPIP().toString();
  doc["saved_credentials"]=haveSavedCredentials();
  doc["account_ready"]=accountReady;doc["migration_required"]=migrationRequired;doc["wifi_profiles"]=wifiProfiles.count;doc["ssid"]=WiFi.SSID();doc["wifi_status"]=(int)WiFi.status();doc["wifi_failure_reason"]=wifiFailureReason; doc["status"]=statusText;
  doc["unix_time"]=(uint32_t)time(nullptr);
  if(displayReady)xSemaphoreTake(frameMutex,portMAX_DELAY);
  doc["ui_engine"]="LVGL 8.3.11";doc["ui_frames"]=uiFrames;doc["ui_ready"]=displayReady;
  doc["cpu_mhz"]=getCpuFrequencyMhz();doc["rotation"]=screenRotation;doc["width"]=tft.width();doc["height"]=tft.height();
  doc["imu_present"]=motion.available;doc["imu_samples"]=motion.samples;doc["imu_errors"]=motion.errors;
  doc["accel_x"]=motion.x;doc["accel_y"]=motion.y;doc["accel_z"]=motion.z;doc["gyro_dps"]=motion.speed;
  if(isfinite(chipTemperature))doc["chip_temperature_c"]=chipTemperature;else doc["chip_temperature_c"]=nullptr;
  if(displayReady)xSemaphoreGive(frameMutex);
  serializeJson(doc,Serial); Serial.println();
}
void probeTLS() {
  JsonDocument result;
  result["clock_synced"]=ensureClock();
  WiFiClientSecure client;
  result["trust_store"]="ESP-IDF built-in";
  client.useBuiltinCACertBundle();
  client.setHandshakeTimeout(15);
  HTTPClient http; http.setTimeout(15000); http.setConnectTimeout(15000);
  if (time(nullptr)>=1700000000 && http.begin(client, USAGE_URL)) {
    int code=http.GET();  // No Authorization header or account data.
    result["http_status"]=code;
    char error[128]={};
    result["tls_error"]=code>0 ? 0 : client.lastError(error,sizeof(error));
    if(result["tls_error"].as<int>()) result["tls_detail"]=error;
    http.end();
  }
  serializeJson(result,Serial); Serial.println();
}
void testCredentialStorage() {
  // Use an isolated temporary namespace and synthetic data, never user credentials.
  String previousToken=accessToken, previousAccount=accountId;
  bool wasReady=accountReady;
  JsonDocument sample; String dummy; dummy.reserve(2048);
  for(int i=0;i<2048;i++) dummy+='T';
  sample["access_token"]=dummy; sample["account_id"]="synthetic-account";
  String plain; serializeJson(sample,plain);
  bool saved=encryptCredentials(plain,"temporary-storage-test","cdselftest");
  bool roundtrip=saved && decryptCredentials("temporary-storage-test","cdselftest") && accessToken==dummy && accountId=="synthetic-account";
  bool wrongRejected=saved && !decryptCredentials("wrong-test-passphrase","cdselftest");
  bool automaticSaved=saveAutomaticAuth(plain,"cdselftest");
  accessToken="";accountId="";accountReady=false;
  bool automaticReload=automaticSaved&&loadAutomaticAuth("cdselftest")&&accessToken==dummy&&accountId=="synthetic-account";
  bool legacyRemoved=false;
  if(prefs.begin("cdselftest",true)){legacyRemoved=prefs.getBytesLength("auth_v1")==0;prefs.end();}
  String badKey;for(int i=0;i<64;i++)badKey+='0';
  bool wrongDeviceKeyRejected=!decryptCredentials(badKey,"cdselftest","auto_auth",true);
  bool cleared=false;
  if(prefs.begin("cdselftest",false)) { cleared=prefs.clear(); prefs.end(); }
  accessToken=previousToken; accountId=previousAccount; accountReady=wasReady;
  // Profile persistence and priority tests use a separate namespace and fake networks.
  if(prefs.begin("cdwifitest",false)){prefs.clear();prefs.end();}
  WifiProfiles networks("cdwifitest");bool profilesOk=networks.load(false);
  for(int i=0;i<5;i++)profilesOk=networks.put("Test "+String(i),"synthetic-password")&&profilesOk;
  profilesOk=profilesOk&&!networks.put("Sixth","synthetic-password")&&networks.count==5;
  profilesOk=profilesOk&&networks.put("Test 2","updated-password")&&networks.count==5&&networks.prefer("Test 2");
  WifiProfiles restored("cdwifitest");
  profilesOk=profilesOk&&restored.load(false)&&restored.count==5&&restored.items[0].ssid=="Test 2"&&restored.items[0].password=="updated-password";
  profilesOk=profilesOk&&restored.remove("Test 2")&&restored.count==4&&restored.find("Test 2")==-1;
  profilesOk=profilesOk&&!restored.put("","synthetic-password")&&!restored.put("Invalid password","short")&&restored.put("Open network","");
  WifiProfiles finalProfiles("cdwifitest");profilesOk=profilesOk&&finalProfiles.load(false)&&finalProfiles.count==5&&finalProfiles.find("Open network")>=0;
  bool wifiCleared=false;if(prefs.begin("cdwifitest",false)){wifiCleared=prefs.clear();prefs.end();}
  JsonDocument result; result["saved"]=saved; result["roundtrip"]=roundtrip;
  result["wrong_passphrase_rejected"]=wrongRejected; result["temporary_namespace_cleared"]=cleared;
  result["automatic_saved"]=automaticSaved;result["automatic_reload"]=automaticReload;result["legacy_removed_after_migration"]=legacyRemoved;
  result["wrong_device_key_rejected"]=wrongDeviceKeyRejected;result["wifi_profile_tests"]=profilesOk;result["wifi_test_namespace_cleared"]=wifiCleared;
  serializeJson(result,Serial); Serial.println();
}
void testWifiVisibility() {
  JsonDocument doc;
  int found=WiFi.scanNetworks(false,false,false,150);
  doc["scan_ok"]=found>=0;
  JsonArray list=doc["networks"].to<JsonArray>();
  for(size_t i=0;i<wifiProfiles.count;i++){
    JsonObject item=list.add<JsonObject>();item["ssid"]=wifiProfiles.items[i].ssid;bool visible=false;
    for(int j=0;j<found;j++)if(WiFi.SSID(j)==wifiProfiles.items[i].ssid){visible=true;item["rssi"]=WiFi.RSSI(j);item["channel"]=WiFi.channel(j);break;}
    item["visible"]=visible;
  }
  WiFi.scanDelete();
  if(prefs.begin("codexmon",true)){
    String oldSsid=prefs.getString("ssid","");int index=wifiProfiles.find(oldSsid);
    doc["legacy_profile_unchanged"]=index>=0&&wifiProfiles.items[index].password==prefs.getString("wpass","");prefs.end();
  }
  doc["failure_reason"]=wifiFailureReason;serializeJson(doc,Serial);Serial.println();
}
void handleSlotControls() {
  serviceDisplay();
  static String command;
  while (Serial.available()) {
    char c=Serial.read();
    if(c=='\n') {
      command.trim();
      if(command=="STATUS") printStatus();
      else if(command=="DETAILS") {detailsPage=!detailsPage;drawStatusScreen();Serial.println(detailsPage?"Device page":"Usage page");}
      else if(command=="TLS_TEST") probeTLS();
      else if(command=="STORAGE_TEST") testCredentialStorage();
      else if(command=="WIFI_DIAG") testWifiVisibility();
      else if(command.startsWith("WIFI_CONNECT ")) {
        String ssid=command.substring(13);int index=wifiProfiles.find(ssid);
        if(index<0)Serial.println("Saved network not found");
        else{
          // USB testing changes only the RAM order; saved priorities stay intact.
          WifiProfiles::Profile selected=wifiProfiles.items[index];
          for(int i=index;i>0;i--)wifiProfiles.items[i]=wifiProfiles.items[i-1];
          wifiProfiles.items[0]=selected;wifiRestart=true;
          Serial.println("Testing saved network: "+ssid);
        }
      }
      else if(command=="WIFI_RESTORE") {
        WifiProfiles restored;
        if(restored.load()&&restored.count){wifiProfiles=restored;wifiRestart=true;Serial.println("Restoring saved Wi-Fi priorities");}
        else Serial.println("Could not reload saved Wi-Fi priorities");
      }
      else if(command=="BOOT_A") bootSlotA();
      else if(command=="RESTART") {Serial.println("Restarting Dashboard");Serial.flush();delay(100);ESP.restart();}
      else if(command.startsWith("PREVIEW ")) {screenPreview=true;previewUntil=millis()+30000;detailsPage=false;setScreenRotation(command.substring(8).toInt());drawStatusScreen();Serial.println("Design preview: synthetic allowance for 30 seconds");}
      else if(command=="LIVE") {screenPreview=false;drawStatusScreen();Serial.println("Live display restored");}
      else if(command=="SCREEN") {
        if(!displayReady){Serial.println("Display unavailable");command="";continue;}
        xSemaphoreTake(frameMutex,portMAX_DELAY);
        size_t size=tft.width()*tft.height()*2,offset=0;
        uint8_t *pixels=(uint8_t*)heap_caps_malloc(size,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
        if(pixels)memcpy(pixels,tft.getBuffer(),size);
        xSemaphoreGive(frameMutex);
        if(!pixels){Serial.println("Snapshot unavailable");command="";continue;}
        Serial.setTxTimeoutMs(1000);Serial.printf("FRAME %u\n",(unsigned)size);
        while(offset<size){size_t n=Serial.write(pixels+offset,min(size-offset,size_t(2048)));if(!n)break;offset+=n;delay(1);}
        Serial.setTxTimeoutMs(250);free(pixels);
      }
      command="";
    } else if(command.length()<64) command+=c;
  }
}

void setup() {
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  temperature_sensor_config_t temperatureConfig=TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10,80);
  if(temperature_sensor_install(&temperatureConfig,&temperatureSensor)==ESP_OK)temperature_sensor_enable(temperatureSensor);
  pinMode(0, INPUT_PULLUP);
  lcd.init();lcd.setBrightness(130);
  motion.begin();beginDisplay();drawStatusScreen();
  esp_ota_mark_app_valid_cancel_rollback();
  accountReady=loadAutomaticAuth();migrationRequired=!accountReady&&haveLegacyCredentials();
  bool wifiLoaded=wifiProfiles.load();
  WiFi.onEvent([](arduino_event_id_t event,arduino_event_info_t info){if(event==ARDUINO_EVENT_WIFI_STA_DISCONNECTED&&info.wifi_sta_disconnected.reason!=8)wifiFailureReason=info.wifi_sta_disconnected.reason;});
  WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(false);WiFi.setSleep(true);
  setupRoutes();
  if(!wifiLoaded)statusText="Wi-Fi storage needs attention";
  if(!wifiProfiles.count)startSetupAP();else serviceWifi();
  Serial.println("Codex Dashboard Slot B ready. STATUS / SCREEN / BOOT_A");
}
void loop() {
  handleSlotControls();
  if (WiFi.getMode() & WIFI_AP) dnsServer.processNextRequest();
  server.handleClient();
  serviceWifi();
  if (accountReady && WiFi.status() == WL_CONNECTED && millis() - lastPoll >= POLL_MS) {
    lastPoll = millis(); fetchUsage();
  }
  delay(2);
}
