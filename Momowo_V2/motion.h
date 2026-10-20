// Momowo motion sensor (optional, on the display's I2C pins): an MPU6050 (GY-521) accelerometer
// and gyro for pats, shakes, tilts and drops. A small register-level driver, so no extra library
// is needed. Only loop() calls these.
#pragma once
#include <Wire.h>
#include <math.h>

namespace motion {

// ---- I2C helpers ----
static bool wr(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

static bool rd(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t n) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(addr, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

static bool present(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

// ---- MPU6050 ----
static uint8_t mpuAddr = 0;                 // 0 = not fitted

// Is there an MPU6050 (or a close clone) at addr? Its WHO_AM_I register tells it apart from a
// DS3231 clock chip, which shares address 0x68.
static bool isMpu(uint8_t addr) {
  uint8_t who = 0;
  if (!present(addr) || !rd(addr, 0x75, &who, 1)) return false;
  return who == 0x68 || (who >= 0x70 && who <= 0x74) || who == 0x98;   // 0x74: MPU-6515 style clones
}

// The I2C addresses that answer, as "3C 68", and WHO_AM_I at 0x68/0x69 as "68:68 69:--"
static void i2cScan(char* found, size_t n, char* who, size_t wn) {
  size_t k = 0;
  found[0] = 0;
  for (uint8_t a = 1; a < 127 && k + 4 < n; a++)
    if (present(a)) k += snprintf(found + k, n - k, k ? " %02X" : "%02X", a);
  if (!k) snprintf(found, n, "nothing");
  uint8_t w8 = 0, w9 = 0;
  bool h8 = present(0x68) && rd(0x68, 0x75, &w8, 1), h9 = present(0x69) && rd(0x69, 0x75, &w9, 1);
  char a8[3] = "--", a9[3] = "--";
  if (h8) snprintf(a8, sizeof(a8), "%02X", w8);
  if (h9) snprintf(a9, sizeof(a9), "%02X", w9);
  snprintf(who, wn, "68:%s 69:%s", a8, a9);
}

// Not found: list what is on the I2C bus, and what answers at 0x68/0x69, on the Serial Monitor
static void mpuDiagnose() {
  Serial.print("[motion] I2C devices:");
  for (uint8_t a = 1; a < 127; a++)
    if (present(a)) Serial.printf(" 0x%02X", a);
  Serial.println(" (0x3C is the screen, 0x68 a DS3231 clock or the MPU6050, 0x69 the MPU6050 with AD0 high)");
  for (uint8_t a : {(uint8_t)0x68, (uint8_t)0x69}) {
    uint8_t who = 0;
    if (present(a) && rd(a, 0x75, &who, 1)) Serial.printf("[motion] 0x%02X answers WHO_AM_I = 0x%02X\n", a, who);
  }
}

static bool mpuBegin() {
  for (uint8_t a : {(uint8_t)0x69, (uint8_t)0x68}) {
    if (!isMpu(a)) continue;
    wr(a, 0x6B, 0x80);                      // reset
    delay(60);
    wr(a, 0x6B, 0x01);                      // wake, gyro clock
    wr(a, 0x1A, 0x03);                      // 44 Hz low-pass filter
    wr(a, 0x19, 0x09);                      // 100 samples a second
    wr(a, 0x1B, 0x08);                      // gyro +-500 deg/s
    wr(a, 0x1C, 0x08);                      // accel +-4 g
    mpuAddr = a;
    return true;
  }
  return false;
}

// Acceleration in g and rotation in deg/s, in the sensor's own axes
static bool mpuRead(float a[3], float g[3]) {
  uint8_t b[14];
  if (!mpuAddr || !rd(mpuAddr, 0x3B, b, 14)) return false;
  for (int i = 0; i < 3; i++) {
    a[i] = (int16_t)(b[i * 2] << 8 | b[i * 2 + 1]) / 8192.0f;
    g[i] = (int16_t)(b[8 + i * 2] << 8 | b[9 + i * 2]) / 65.5f;
  }
  return true;
}

// ---- Gestures from the accelerometer ----
enum Event : uint8_t { EV_NONE, EV_PAT, EV_SHAKE, EV_DROP, EV_TILT };

struct State {
  float grav[3] = {0, 0, 1};               // slow average: which way is down (sensor reads "up" as +1 g)
  float ref[3] = {0, 0, 1};                // Momowo's usual resting position
  bool refSet = false;
  float tiltX = 0, tiltY = 0;               // -1..1: tilt away from the resting position, for the eyes
  float tiltDeg = 0;
  uint32_t stillSince = 0, lastPeak = 0, bumpAt = 0, shakeStart = 0, dropStart = 0, tiltSince = 0;
  uint8_t peaks = 0;
  bool tiltWarned = false;
  float jerk = 0;                           // short-term movement energy
  int8_t lastSign = 0;
};

static float dot3(const float* a, const float* b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
static float norm3(const float* a) { return sqrtf(dot3(a, a)); }

// Tilt of the current "down" (s.grav) away from a reference pose, split into the screen's
// left-right and up-down, each -1..1
static void tiltFrom(const State& s, const float ref[3], float& tx, float& ty) {
  float rn = norm3(ref), gn = norm3(s.grav);
  float r[3] = {ref[0] / rn, ref[1] / rn, ref[2] / rn};
  float q[3] = {s.grav[0] / gn, s.grav[1] / gn, s.grav[2] / gn};
  // screen axes: sensor X (or Y if X points down) made level, then the one across it
  float ax[3] = {1, 0, 0};
  if (fabsf(r[0]) > 0.8f) { ax[0] = 0; ax[1] = 1; }
  float k = dot3(ax, r);
  for (int i = 0; i < 3; i++) ax[i] -= k * r[i];
  float an = norm3(ax);
  for (int i = 0; i < 3; i++) ax[i] /= an;
  float ay[3] = {r[1] * ax[2] - r[2] * ax[1], r[2] * ax[0] - r[0] * ax[2], r[0] * ax[1] - r[1] * ax[0]};
  tx = fmaxf(-1, fminf(1, dot3(q, ax) * 2.0f));
  ty = fmaxf(-1, fminf(1, dot3(q, ay) * 2.0f));
}

// Feed one reading (about 50 a second). sens: 1 gentle .. 3 sensitive. Returns an event, if any.
static Event update(State& s, const float a[3], const float g[3], uint32_t now, uint8_t sens) {
  float mag = norm3(a);
  if (mag < 0.05f) return EV_NONE;
  float dyn[3];
  for (int i = 0; i < 3; i++) {
    s.grav[i] += (a[i] - s.grav[i]) * 0.08f;
    dyn[i] = a[i] - s.grav[i];
  }
  float d = norm3(dyn);
  float rot = fabsf(g[0]) + fabsf(g[1]) + fabsf(g[2]);
  s.jerk = s.jerk * 0.9f + d * 0.1f;

  // resting position: learned once still at start-up, and again whenever Momowo is moved
  // somewhere and left there for a minute
  bool still = d < 0.04f && rot < 6;
  if (!still) s.stillSince = now;
  uint32_t settle = s.refSet ? 60000 : 1500;
  if (now - s.stillSince > settle) {
    float gn = norm3(s.grav);
    float ang = acosf(fmaxf(-1.0f, fminf(1.0f, dot3(s.grav, s.ref) / (gn * norm3(s.ref) + 1e-6f)))) * 57.3f;
    if (!s.refSet || ang > 8) {
      for (int i = 0; i < 3; i++) s.ref[i] = s.grav[i];
      s.refSet = true;
      s.tiltWarned = false;
    }
    s.stillSince = now;
  }

  // tilt relative to the resting position, split into the screen's left-right and up-down
  if (s.refSet) {
    float rn = norm3(s.ref), gn = norm3(s.grav);
    float r[3] = {s.ref[0] / rn, s.ref[1] / rn, s.ref[2] / rn};
    float q[3] = {s.grav[0] / gn, s.grav[1] / gn, s.grav[2] / gn};
    s.tiltDeg = acosf(fmaxf(-1.0f, fminf(1.0f, dot3(q, r)))) * 57.3f;
    tiltFrom(s, s.ref, s.tiltX, s.tiltY);
  }

  // dropped: nearly weightless for a moment
  if (mag < 0.35f) {
    if (!s.dropStart) s.dropStart = now;
    else if (now - s.dropStart > 70) { s.dropStart = 0; return EV_DROP; }
  } else {
    s.dropStart = 0;
  }

  // shake: several strong swings back and forth within a second
  float shakeThr = 1.3f - 0.25f * sens;     // g
  if (d > shakeThr) {
    int8_t sign = dot3(dyn, s.grav) >= 0 ? 1 : -1;
    if (now - s.lastPeak > 60 && sign != s.lastSign) {
      if (now - s.shakeStart > 1200) { s.shakeStart = now; s.peaks = 0; }
      s.lastPeak = now;
      s.lastSign = sign;
      if (++s.peaks >= 4) { s.peaks = 0; s.shakeStart = 0; return EV_SHAKE; }
    }
  }

  // pat: a short, light bump, then calm again (not part of a shake)
  float patThr = 0.42f - 0.1f * sens;
  if (d > patThr && d < shakeThr && !s.bumpAt && now - s.lastPeak > 800) s.bumpAt = now;
  if (s.bumpAt && now - s.bumpAt > 180) {
    bool calm = s.jerk < patThr * 0.5f && now - s.lastPeak > 800;
    s.bumpAt = 0;
    if (calm) return EV_PAT;
  }

  // held at a steep angle for two seconds
  if (s.refSet && s.tiltDeg > 55) {
    if (!s.tiltSince) s.tiltSince = now;
    if (!s.tiltWarned && now - s.tiltSince > 2000) { s.tiltWarned = true; return EV_TILT; }
  } else {
    s.tiltSince = 0;
    if (s.tiltDeg < 30) s.tiltWarned = false;
  }
  return EV_NONE;
}

}  // namespace motion
