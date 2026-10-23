#include "../src/OrientationPolicy.h"
#include <cassert>
#include <cstdint>
int main(){
  struct Pose{float x,y;int rotation;};
  Pose poses[]={{1,0,3},{-1,0,1},{0,1,2},{0,-1,0}};
  for(auto pose:poses){
    OrientationPolicy p;int current=(pose.rotation+1)&3;
    assert(p.update(0,0,1,0,1000,current)==current);
    assert(p.update(pose.x,pose.y,0,0,1100,current)==current);
    assert(p.update(pose.x,pose.y,0,0,1600,current)==current);
    assert(p.update(pose.x,pose.y,0,0,1700,current)==pose.rotation);
  }
  OrientationPolicy p;
  assert(p.update(0,1,0,120,2000,1)==1);
  assert(p.update(.7,.7,0,0,3000,1)==1);
  assert(p.update(0,2,0,0,4000,1)==1);
  assert(p.update(0,1,0,0,5000,1)==1);
  assert(p.update(0,1,0,0,5600,1)==2);
  assert(p.update(0,-1,0,0,5700,2)==2);
  assert(p.update(0,-1,0,0,6300,2)==2);
  assert(p.update(0,-1,0,0,6600,2)==0);
  OrientationPolicy wrap;
  assert(wrap.update(0,1,0,0,UINT32_MAX-300,1)==1);
  assert(wrap.update(0,1,0,0,500,1)==2);
}
