// Momowo astronomy: sun and moon positions, Islamic prayer times and the Hindu
// lunar calendar (panchang). Pure maths, no hardware: it compiles and can be tested on a PC.
// Accuracy is about a minute for prayer times and sunrise, and a few minutes for tithi
// boundaries, which is plenty for a desk display (local mosques and almanacs have the final say).
#pragma once
#include <math.h>
#include <stdint.h>

namespace astro {

static const double D2R = M_PI / 180.0, R2D = 180.0 / M_PI;

static double fixAngle(double a) { a = fmod(a, 360.0); return a < 0 ? a + 360.0 : a; }
static double fixHour(double h)  { h = fmod(h, 24.0); return h < 0 ? h + 24.0 : h; }

// Julian day at 0h UT of a Gregorian date
static double julianDay(int y, int m, int d) {
  if (m <= 2) { y -= 1; m += 12; }
  int a = y / 100, b = 2 - a + a / 4;
  return floor(365.25 * (y + 4716)) + floor(30.6001 * (m + 1)) + d + b - 1524.5;
}

// ---- Sun (Meeus, low precision: about 0.01 degree) ----
struct SunPos { double decl, eqt, lon; };   // declination (deg), equation of time (hours), apparent longitude (deg)

static SunPos sunPosition(double jd) {
  double D = jd - 2451545.0;
  double g = fixAngle(357.529 + 0.98560028 * D);
  double q = fixAngle(280.459 + 0.98564736 * D);
  double L = fixAngle(q + 1.915 * sin(g * D2R) + 0.020 * sin(2 * g * D2R));
  double e = 23.439 - 0.00000036 * D;
  double RA = atan2(cos(e * D2R) * sin(L * D2R), cos(L * D2R)) * R2D / 15.0;
  SunPos s;
  s.decl = asin(sin(e * D2R) * sin(L * D2R)) * R2D;
  s.eqt = q / 15.0 - fixHour(RA);
  if (s.eqt > 12) s.eqt -= 24;
  if (s.eqt < -12) s.eqt += 24;
  // apparent longitude, a little more precise (for the Hindu calendar)
  double T = D / 36525.0;
  double L0 = fixAngle(280.46646 + 36000.76983 * T);
  double M = fixAngle(357.52911 + 35999.05029 * T) * D2R;
  double C = (1.914602 - 0.004817 * T) * sin(M) + 0.019993 * sin(2 * M) + 0.000289 * sin(3 * M);
  double om = (125.04 - 1934.136 * T) * D2R;
  s.lon = fixAngle(L0 + C - 0.00569 - 0.00478 * sin(om));
  return s;
}

// ---- Moon longitude (Meeus ch. 47, main periodic terms: about 0.01 degree) ----
static double moonLongitude(double jd) {
  double T = (jd - 2451545.0) / 36525.0;
  double Lp = fixAngle(218.3164477 + 481267.88123421 * T);
  double D  = fixAngle(297.8501921 + 445267.1114034 * T) * D2R;
  double M  = fixAngle(357.5291092 + 35999.0502909 * T) * D2R;
  double Mp = fixAngle(134.9633964 + 477198.8675055 * T) * D2R;
  double F  = fixAngle(93.2720950 + 483202.0175233 * T) * D2R;
  double E  = 1 - 0.002516 * T;
  // {D, M, M', F, coefficient in 1e-6 degree}
  static const int16_t A[][4] = {
    {0,0,1,0},{2,0,-1,0},{2,0,0,0},{0,0,2,0},{0,1,0,0},{0,0,0,2},{2,0,-2,0},{2,-1,-1,0},
    {2,0,1,0},{2,-1,0,0},{0,1,-1,0},{1,0,0,0},{0,1,1,0},{2,0,0,-2},{0,0,1,2},{0,0,1,-2},
    {4,0,-1,0},{0,0,3,0},{4,0,-2,0},{2,1,-1,0},{2,1,0,0},{1,0,-1,0},{1,1,0,0},{2,-1,1,0},
    {2,0,2,0},{4,0,0,0},{2,0,-3,0},{0,1,-2,0},{2,0,-1,2},{2,-1,-2,0},{1,0,1,0},{2,-2,0,0}};
  static const int32_t C[] = {
    6288774, 1274027, 658314, 213618, -185116, -114332, 58793, 57066,
    53322, 45758, -40923, -34720, -30383, 15327, -12528, 10980,
    10675, 10034, 8548, -7888, -6766, -5163, 4987, 4036,
    3994, 3861, 3665, -2689, -2602, 2390, -2348, 2236};
  double sum = 0;
  for (unsigned i = 0; i < sizeof(C) / sizeof(C[0]); i++) {
    double arg = A[i][0] * D + A[i][1] * M + A[i][2] * Mp + A[i][3] * F;
    double c = C[i];
    if (abs(A[i][1]) == 1) c *= E; else if (abs(A[i][1]) == 2) c *= E * E;
    sum += c * sin(arg);
  }
  double om = (125.04452 - 1934.136261 * T) * D2R;
  return fixAngle(Lp + sum / 1e6 - 0.00478 * sin(om));   // apparent (nutation in longitude, main term)
}

// ---- Prayer times (the PrayTimes.org method) ----
enum Prayer { FAJR, SUNRISE, DHUHR, ASR, MAGHRIB, ISHA, PRAYER_COUNT };

struct Method {
  const char* name;
  float fajr;        // sun angle below the horizon
  float isha;        // angle, or minutes after maghrib when ishaMin is true
  bool ishaMin;
  uint8_t ihtiyat;   // safety minutes added (taken off sunrise), as local authorities publish
};

// Same order as on the phone page
static const Method METHODS[] = {
  {"Muslim World League", 18.0f, 17.0f, false, 0},
  {"ISNA (North America)", 15.0f, 15.0f, false, 0},
  {"Egyptian Authority", 19.5f, 17.5f, false, 0},
  {"Umm al-Qura (Makkah)", 18.5f, 90.0f, true, 0},
  {"Karachi", 18.0f, 18.0f, false, 0},
  {"MUIS (Singapore)", 20.0f, 18.0f, false, 2},
  {"Kemenag (Indonesia)", 20.0f, 18.0f, false, 2},
  {"JAKIM (Malaysia)", 20.0f, 18.0f, false, 2},
  {"Diyanet (Turkey)", 18.0f, 17.0f, false, 0},
  {"UOIF (France)", 12.0f, 12.0f, false, 0},
  {"Gulf / Dubai", 18.2f, 18.2f, false, 0},
};
static const int METHOD_COUNT = sizeof(METHODS) / sizeof(METHODS[0]);

// A sensible method for a country (ISO code), used when the owner leaves it on "automatic"
static int methodForCountry(const char* cc) {
  if (!cc || !cc[0]) return 0;
  struct { const char* cc; uint8_t m; } T[] = {
    {"SG", 5}, {"ID", 6}, {"MY", 7}, {"BN", 7}, {"SA", 3}, {"TR", 8}, {"FR", 9},
    {"US", 1}, {"CA", 1}, {"EG", 2}, {"PK", 4}, {"IN", 4}, {"BD", 4}, {"AE", 10},
    {"QA", 10}, {"KW", 10}, {"OM", 10}, {"BH", 10}};
  for (auto& e : T) if (cc[0] == e.cc[0] && cc[1] == e.cc[1]) return e.m;
  return 0;
}

// Minutes after local midnight for each prayer (NAN if the sun never reaches the angle).
// tz: local UTC offset in hours. hanafi: Asr when shadows are twice the length. ramadan: Umm
// al-Qura Isha is 120 minutes after Maghrib during Ramadan.
static void prayerTimes(int y, int m, int d, double lat, double lon, double tz, int method, bool hanafi,
                        bool ramadan, double out[PRAYER_COUNT]) {
  if (method < 0 || method >= METHOD_COUNT) method = 0;
  const Method& M = METHODS[method];
  double jd = julianDay(y, m, d) - lon / (15.0 * 24.0);
  auto midDay = [&](double t) { return fixHour(12 - sunPosition(jd + t).eqt); };
  auto angleTime = [&](double angle, double t, bool ccw) {
    SunPos s = sunPosition(jd + t);
    double noon = fixHour(12 - s.eqt);
    double c = (-sin(angle * D2R) - sin(s.decl * D2R) * sin(lat * D2R)) /
               (cos(s.decl * D2R) * cos(lat * D2R));
    if (c < -1 || c > 1) return (double)NAN;
    double T = acos(c) * R2D / 15.0;
    return noon + (ccw ? -T : T);
  };
  auto asrTime = [&](double factor, double t) {
    double decl = sunPosition(jd + t).decl;
    double angle = -atan(1.0 / (factor + tan(fabs(lat - decl) * D2R))) * R2D;
    return angleTime(angle, t, false);
  };
  // first guesses (hours), then one refinement with the sun's position at those times
  double t[PRAYER_COUNT] = {5, 6, 12, 13, 18, 18};
  for (int iter = 0; iter < 2; iter++) {
    double p[PRAYER_COUNT];
    for (int i = 0; i < PRAYER_COUNT; i++) p[i] = (isnan(t[i]) ? (i < 2 ? 5.0 : 18.0) : t[i]) / 24.0;
    t[FAJR]    = angleTime(M.fajr, p[FAJR], true);
    t[SUNRISE] = angleTime(0.833, p[SUNRISE], true);
    t[DHUHR]   = midDay(p[DHUHR]);
    t[ASR]     = asrTime(hanafi ? 2 : 1, p[ASR]);
    t[MAGHRIB] = angleTime(0.833, p[MAGHRIB], false);
    t[ISHA]    = M.ishaMin ? t[MAGHRIB] + (ramadan && method == 3 ? 120 : M.isha) / 60.0
                           : angleTime(M.isha, p[ISHA], false);
  }
  // high latitudes: Fajr / Isha no later (earlier) than the "angle-based" share of the night
  double night = fixHour(t[SUNRISE] - t[MAGHRIB]);
  if (!isnan(night)) {
    double fajrMax = M.fajr / 60.0 * night;
    if (isnan(t[FAJR]) || fixHour(t[SUNRISE] - t[FAJR]) > fajrMax) t[FAJR] = t[SUNRISE] - fajrMax;
    if (!M.ishaMin) {
      double ishaMax = M.isha / 60.0 * night;
      if (isnan(t[ISHA]) || fixHour(t[ISHA] - t[MAGHRIB]) > ishaMax) t[ISHA] = t[MAGHRIB] + ishaMax;
    }
  }
  for (int i = 0; i < PRAYER_COUNT; i++) {
    double h = t[i] + tz - lon / 15.0;
    double ih = M.ihtiyat / 60.0;
    h += i == SUNRISE ? -ih : ih;                           // safety margin: later, sunrise earlier
    if (i == DHUHR && !M.ihtiyat) h += 1 / 60.0;            // Dhuhr a minute after the zenith
    out[i] = isnan(h) ? NAN : fixHour(h) * 60.0;
  }
}

// ---- Hindu lunar calendar (amanta months, Lahiri ayanamsa) ----
static double ayanamsa(double jd) { return 23.853 + (jd - 2451545.0) / 365.25 * (50.2788 / 3600.0); }
static double siderealSun(double jd) { return fixAngle(sunPosition(jd).lon - ayanamsa(jd)); }
static double elongation(double jd) { return fixAngle(moonLongitude(jd) - sunPosition(jd).lon); }

// The new moon (off = 0) or full moon (off = 180) just before (dir = -1) or after (dir = +1) jd
static double moonPhase(double jd, int dir, double off) {
  auto e = [off](double x) { return fixAngle(elongation(x) + off); };
  // step a day at a time until the (shifted) elongation wraps through 360 -> 0
  double a = jd, ea = e(a);
  for (int i = 0; i < 35; i++) {
    double b = a + dir, eb = e(b);
    bool wrapped = dir < 0 ? eb > ea + 180 : ea > eb + 180;
    if (wrapped) {
      double lo = dir < 0 ? b : a, hi = dir < 0 ? a : b;   // it wraps between lo and hi
      for (int k = 0; k < 30; k++) {
        double mid = (lo + hi) / 2;
        if (e(mid) > 180) lo = mid; else hi = mid;
      }
      return (lo + hi) / 2;
    }
    a = b; ea = eb;
  }
  return jd;
}
static double newMoon(double jd, int dir) { return moonPhase(jd, dir, 0); }
static double fullMoon(double jd, int dir) { return moonPhase(jd, dir, 180); }

struct Panchang {
  int tithi;         // 1-30: 1-15 Shukla (waxing), 16-30 Krishna (waning); 15 Purnima, 30 Amavasya
  int month;         // 0 Chaitra ... 11 Phalguna (amanta: the month ends at the new moon)
  bool adhika;       // leap month
  int shaka, vikram; // Shaka Samvat and Vikram Samvat years
  int rasi;          // sidereal sign of the sun (0 Mesha ... 11 Meena)
};

// jdUT: the instant to describe, normally local sunrise. gYear / gMonth: its Gregorian date.
static Panchang panchang(double jdUT, int gYear, int gMonth) {
  Panchang p;
  p.tithi = (int)(elongation(jdUT) / 12.0) + 1;
  if (p.tithi > 30) p.tithi = 30;
  double nm0 = newMoon(jdUT, -1), nm1 = newMoon(jdUT + 0.5, +1);
  int r0 = (int)(siderealSun(nm0) / 30.0), r1 = (int)(siderealSun(nm1) / 30.0);
  p.month = (r0 + 1) % 12;
  p.adhika = (r0 == r1);
  p.rasi = (int)(siderealSun(jdUT) / 30.0);
  p.shaka = (gMonth <= 6 && p.month >= 8) ? gYear - 79 : gYear - 78;
  p.vikram = p.shaka + 135;
  return p;
}

static const char* const HINDU_MONTH[12] = {"Chaitra", "Vaishakha", "Jyeshtha", "Ashadha", "Shravana",
  "Bhadrapada", "Ashvin", "Kartika", "Margashirsha", "Pausha", "Magha", "Phalguna"};

static int tithiAt(double jd) {
  int t = (int)(elongation(jd) / 12.0) + 1;
  return t > 30 ? 30 : t;
}

// Major festivals (pan-Indian), or nullptr. Each is kept on the day its tithi prevails at the time
// tradition uses for it (sunrise, midday, afternoon, evening or midnight); when that tithi falls
// entirely between two such moments (a "lost" tithi), the day it begins on takes it.
// jdRise / jdSet: today's local sunrise and sunset (UT Julian days).
static const char* hinduFestival(const Panchang& p, double jdRise, double jdSet) {
  double day = jdSet - jdRise;
  if (day <= 0 || day > 1) day = 0.5;
  // Makar Sankranti: the sun entered sidereal Capricorn after yesterday's sunset, by today's
  int r0 = (int)(siderealSun(jdSet - 1) / 30.0), r1 = (int)(siderealSun(jdSet) / 30.0);
  if (r0 == 8 && r1 == 9) return "Makar Sankranti / Pongal";
  if (p.adhika) return nullptr;
  enum { RISE, NOON, AFTERNOON, EVENING, MIDNIGHT };
  struct F { uint8_t month, tithi, when; const char* name; };
  static const F FEST[] = {
    {0, 1, RISE, "Ugadi / Gudi Padwa"}, {0, 9, NOON, "Ram Navami"},
    {4, 15, RISE, "Raksha Bandhan"}, {4, 23, RISE, "Janmashtami"},
    {5, 4, NOON, "Ganesh Chaturthi"}, {6, 1, RISE, "Navaratri"}, {6, 10, AFTERNOON, "Dussehra"},
    {6, 30, EVENING, "Diwali / Deepavali"}, {10, 29, MIDNIGHT, "Maha Shivaratri"},
    {11, 16, AFTERNOON, "Holi"}};
  auto at = [&](uint8_t w, double shift) {
    double jd = w == RISE ? jdRise : w == NOON ? jdRise + day / 2 : w == AFTERNOON ? jdRise + day * 0.7
              : w == EVENING ? jdSet + 0.03 : jdRise + day / 2 + 0.5;
    return tithiAt(jd + shift);
  };
  for (const F& f : FEST) {
    int t = at(f.when, 0), next = at(f.when, 1);
    int want = f.tithi;
    bool today = t == want && at(f.when, -1) != want;   // a tithi covering the moment two days running: day one
    bool lost = t == (want + 28) % 30 + 1 && next == want % 30 + 1;   // today want-1, tomorrow want+1
    if (!today && !lost) continue;
    // the month is checked at the festival's own moment (a festival just after a new moon)
    int m = p.month;
    if (f.tithi - p.tithi > 15) m = (m + 11) % 12;          // still the old month's end (Diwali on a tithi-1 sunrise)
    else if (p.tithi - f.tithi > 15) m = (m + 1) % 12;      // the new month begins today (Ugadi on a tithi-30 sunrise)
    if (m == f.month) return f.name;
  }
  // Thaipusam: the moon in Pusam (Pushya nakshatra) at midday, by the full moon of Thai (sun in sidereal Capricorn)
  auto pusamNoon = [&](double s) {
    double noon = jdRise + day / 2 + s, ms = fixAngle(moonLongitude(noon) - ayanamsa(noon));
    if (ms < 93.3333 || ms >= 106.6667) return false;
    double a = fullMoon(noon, -1), b = fullMoon(noon, +1), fm = noon - a < b - noon ? a : b;
    return fabs(fm - noon) < 1.5 && (int)(siderealSun(fm) / 30.0) == 9;
  };
  if (pusamNoon(0) && !pusamNoon(-1)) return "Thaipusam";
  return nullptr;
}

// ---- Hebrew calendar (the fixed arithmetic calendar, Dershowitz & Reingold) ----
// Months are numbered from Nisan = 1; the year starts at Tishrei = 7. Adar II = 13 in leap years.
// rd: days counted with 1 = 1 January of year 1 (Gregorian), i.e. Julian day number - 1721425.
static bool hebLeap(int32_t y) { return (7 * y + 1) % 19 < 7; }

static int32_t hebElapsed(int32_t y) {      // days from the epoch to 1 Tishrei of year y
  int32_t months = 235 * ((y - 1) / 19) + 12 * ((y - 1) % 19) + (7 * ((y - 1) % 19) + 1) / 19;
  int32_t parts = 204 + 793 * (months % 1080);
  int32_t hours = 5 + 12 * months + 793 * (months / 1080) + parts / 1080;
  int32_t day = 1 + 29 * months + hours / 24;
  parts = 1080 * (hours % 24) + parts % 1080;
  if (parts >= 19440 || (day % 7 == 2 && parts >= 9924 && !hebLeap(y)) ||
      (day % 7 == 1 && parts >= 16789 && hebLeap(y - 1)))
    day++;                                  // molad zaken and the GaTaRaD / BeTUTaKPaT rules
  if (day % 7 == 0 || day % 7 == 3 || day % 7 == 5) day++;   // Rosh Hashanah never Sun, Wed, Fri
  return day;
}

static int hebMonthDays(int m, int32_t y) {
  int yearDays = hebElapsed(y + 1) - hebElapsed(y);
  if (m == 2 || m == 4 || m == 6 || m == 10 || m == 13) return 29;
  if (m == 12 && !hebLeap(y)) return 29;
  if (m == 8 && yearDays % 10 != 5) return 29;   // Cheshvan is long only in "complete" years
  if (m == 9 && yearDays % 10 == 3) return 29;   // Kislev is short in "deficient" years
  return 30;
}

static const int32_t HEB_EPOCH = -1373429;

static int32_t hebToRd(int m, int d, int32_t y) {
  int32_t days = d;
  int last = hebLeap(y) ? 13 : 12;
  if (m < 7) {
    for (int i = 7; i <= last; i++) days += hebMonthDays(i, y);
    for (int i = 1; i < m; i++) days += hebMonthDays(i, y);
  } else {
    for (int i = 7; i < m; i++) days += hebMonthDays(i, y);
  }
  return days + hebElapsed(y) + HEB_EPOCH;
}

static void hebFromRd(int32_t rd, int32_t& y, int& m, int& d) {
  y = (rd - HEB_EPOCH) / 366;               // a little low; walk up
  while (rd >= hebToRd(7, 1, y + 1)) y++;
  m = rd < hebToRd(1, 1, y) ? 7 : 1;
  while (rd > hebToRd(m, hebMonthDays(m, y), y)) m++;
  d = rd - hebToRd(m, 1, y) + 1;
}

// Jewish holidays on a Hebrew date. dow: 0 Sunday .. 6 Saturday (of that Hebrew day).
// israel: Israel's one-day festivals instead of the diaspora's two. *n: day of a multi-day festival.
enum HebFest : uint8_t { HF_NONE, HF_ROSH, HF_YOM_KIPPUR, HF_SUKKOT, HF_SHEMINI, HF_SIMCHAT,
                         HF_SHEMINI_SIMCHAT, HF_HANUKKAH, HF_TU_BISHVAT, HF_PURIM, HF_PESACH,
                         HF_SHAVUOT, HF_TISHA_BAV };

static HebFest hebFestival(int32_t y, int m, int d, int dow, bool israel, int* n) {
  *n = 0;
  if (m == 7) {
    if (d == 1 || d == 2) { *n = d; return HF_ROSH; }
    if (d == 10) return HF_YOM_KIPPUR;
    if (d >= 15 && d <= 21) { *n = d - 14; return HF_SUKKOT; }
    if (d == 22) return israel ? HF_SHEMINI_SIMCHAT : HF_SHEMINI;
    if (d == 23 && !israel) return HF_SIMCHAT;
  }
  int32_t k = hebToRd(m, d, y) - hebToRd(9, 25, y);         // Hanukkah: 25 Kislev, eight days
  if (k >= 0 && k < 8) { *n = k + 1; return HF_HANUKKAH; }
  if (m == 11 && d == 15) return HF_TU_BISHVAT;
  if (d == 14 && m == (hebLeap(y) ? 13 : 12)) return HF_PURIM;
  if (m == 1 && d >= 15 && d <= (israel ? 21 : 22)) { *n = d - 14; return HF_PESACH; }
  if (m == 3 && (d == 6 || (d == 7 && !israel))) { *n = d - 5; return HF_SHAVUOT; }
  if (m == 5 && ((d == 9 && dow != 6) || (d == 10 && dow == 0))) return HF_TISHA_BAV;   // moved off Shabbat
  return HF_NONE;
}

}  // namespace astro
