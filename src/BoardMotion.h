#pragma once
#include <Wire.h>
#include "OrientationPolicy.h"

// Register map and I2C pins verified against Waveshare's 1.47B Arduino demo.
class BoardMotion {
  uint8_t address=0;
  uint32_t lastSample=0;
  bool filtered=false;
  OrientationPolicy policy;
  bool read(uint8_t reg, uint8_t *buffer, size_t count) {
    Wire.beginTransmission(address); Wire.write(reg);
    if(Wire.endTransmission(false)!=0) return false;
    if(Wire.requestFrom(address, uint8_t(count))!=count) return false;
    for(size_t i=0;i<count;i++) buffer[i]=Wire.read();
    return true;
  }
  bool write(uint8_t reg,uint8_t value) {
    Wire.beginTransmission(address); Wire.write(reg); Wire.write(value);
    return Wire.endTransmission()==0;
  }
public:
  bool available=false;
  float x=0,y=0,z=0,speed=0;
  uint32_t samples=0,errors=0;
  bool begin() {
    Wire.begin(48,47,400000); Wire.setTimeOut(20);
    for(uint8_t a: {uint8_t(0x6b),uint8_t(0x6a)}) {
      address=a; uint8_t id=0;
      if(read(0,&id,1) && id==5) {
        // Disable sensors during configuration. ±4g / ±512 dps, 60 Hz.
        available=write(8,0) && write(2,0x40) && write(3,0x17) &&
                  write(4,0x57) && write(6,0x11) && write(7,0) && write(8,0x43);
        return available;
      }
    }
    return false;
  }
  int poll(uint32_t now,int rotation) {
    if(!available || uint32_t(now-lastSample)<50) return rotation;
    lastSample=now;
    uint8_t b[12];
    if(!read(0x35,b,12)) { errors++; return rotation; }
    auto value=[&](int i){return int16_t(uint16_t(b[i])|(uint16_t(b[i+1])<<8));};
    float ax=value(0)/8192.f,ay=value(2)/8192.f,az=value(4)/8192.f;
    float gx=value(6)/64.f,gy=value(8)/64.f,gz=value(10)/64.f;
    speed=sqrtf(gx*gx+gy*gy+gz*gz);
    if(!filtered) {x=ax;y=ay;z=az;filtered=true;}
    else {x=.8f*x+.2f*ax;y=.8f*y+.2f*ay;z=.8f*z+.2f*az;}
    samples++;
    return policy.update(x,y,z,speed,now,rotation);
  }
};
