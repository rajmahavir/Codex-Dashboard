#pragma once
#include <cmath>
#include <cstdint>

// Gravity establishes orientation; angular speed rejects movement in transit.
// Preserve the last orientation on a flat desk and around diagonal boundaries.
class OrientationPolicy {
  int candidate = -1;
  bool hasChanged = false;
  uint32_t since = 0, changedAt = 0;
public:
  int update(float x, float y, float z, float speed, uint32_t now, int current) {
    float length = std::sqrt(x*x+y*y+z*z);
    if (length < .75f || length > 1.25f || std::fabs(z) > .82f || speed > 80) {
      candidate = -1; return current;
    }
    int next = -1;
    if (std::fabs(x) > .55f && std::fabs(x) > std::fabs(y)*1.3f) next = x > 0 ? 1 : 3;
    else if (std::fabs(y) > .55f && std::fabs(y) > std::fabs(x)*1.3f) next = y > 0 ? 0 : 2;
    // Sensor axes are mounted 180 degrees from the panel coordinate system.
    if(next>=0) next=(next+2)&3;
    if (next < 0 || next == current) { candidate = -1; return current; }
    if (candidate != next) { candidate = next; since = now; return current; }
    if (uint32_t(now-since) >= 550 && (!hasChanged || uint32_t(now-changedAt) >= 900)) {
      hasChanged = true; changedAt = now; candidate = -1; return next;
    }
    return current;
  }
};
