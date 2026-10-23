#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Passwords stay in NVS and are never included in status responses or HTML.
class WifiProfiles {
public:
  explicit WifiProfiles(const char *space="codexwifi"):space_(space){}
  struct Profile { String ssid, password; } items[5];
  size_t count=0;
  int find(const String &ssid) const { for(size_t i=0;i<count;i++)if(items[i].ssid==ssid)return i;return -1; }
  bool load(bool importLegacy=true) {
    count=0;
    Preferences storage;
    String data;
    if(storage.begin(space_,true)){data=storage.getString("profiles","");storage.end();}
    if(data.length()) {
      JsonDocument doc;
      if(deserializeJson(doc,data)||!doc.is<JsonArray>()||doc.size()>5)return false;
      for(JsonObject item:doc.as<JsonArray>()) {
        String ssid=item["ssid"]|"", password=item["password"]|"";
        if(!valid(ssid,password)||find(ssid)>=0){count=0;return false;}
        items[count++]={ssid,password};
      }
      return true;
    }
    if(!importLegacy)return true;
    // Import the previous firmware's single network without deleting its backup.
    if(storage.begin("codexmon",true)) {
      String ssid=storage.getString("ssid",""), password=storage.getString("wpass","");storage.end();
      if(ssid.length()&&valid(ssid,password)){items[0]={ssid,password};count=1;return persist();}
    }
    return true;
  }
  static bool valid(const String &ssid,const String &password) {
    if(!ssid.length()||ssid.length()>32)return false;
    if(password.length()==64){for(char c:password)if(!isxdigit((unsigned char)c))return false;return true;}
    return password.isEmpty()||(password.length()>=8&&password.length()<=63);
  }
  bool put(const String &ssid,const String &password) {
    if(!valid(ssid,password))return false;
    int index=find(ssid);if(index<0&&count==5)return false;
    size_t oldCount=count; if(index<0)index=count++;
    Profile old=items[index];items[index]={ssid,password};
    if(persist())return true;
    items[index]=old;count=oldCount;return false;
  }
  bool remove(const String &ssid) {
    int index=find(ssid);if(index<0)return false;
    Profile old[5];for(size_t i=0;i<count;i++)old[i]=items[i];
    size_t oldCount=count;for(size_t i=index;i+1<count;i++)items[i]=items[i+1];count--;
    if(persist()){items[count]={};return true;}
    count=oldCount;for(size_t i=0;i<count;i++)items[i]=old[i];return false;
  }
  bool prefer(const String &ssid) {
    int index=find(ssid);if(index<0)return false;
    Profile selected=items[index];for(int i=index;i>0;i--)items[i]=items[i-1];items[0]=selected;
    if(persist())return true;
    for(int i=0;i<index;i++)items[i]=items[i+1];items[index]=selected;return false;
  }
private:
  const char *space_;
  bool persist() {
    JsonDocument doc;JsonArray list=doc.to<JsonArray>();
    for(size_t i=0;i<count;i++){JsonObject p=list.add<JsonObject>();p["ssid"]=items[i].ssid;p["password"]=items[i].password;}
    String data;serializeJson(doc,data);Preferences storage;
    if(!storage.begin(space_,false))return false;
    bool ok=storage.putString("profiles",data)==data.length();storage.end();return ok;
  }
};
