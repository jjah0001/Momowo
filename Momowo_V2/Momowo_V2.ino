/*
  =====================================================================
   Momowo 摸摸我 ("pat me")  -  ESP32 / ESP32-C3 + TTP223 + SH1106 1.3" OLED + buzzer
  =====================================================================
   An animated desk companion that helps you relax, and also does real work:

     BUDDY    Expressive animated eyes (blinks, looks around, moods, hearts when patted,
              dizzy when shaken) + a ticker: greeting, feelings, time, weather, next prayer
     CLOCK    Big clock, seconds, date; time zone follows where you are
     CALENDAR Gregorian month and the public holidays where you are, plus the calendars you
              choose: Chinese lunar (solar terms, festivals), Islamic (Hijri), Buddhist, Hindu
     WEATHER  Now, tomorrow, the next three days, sunrise and sunset; with the Islamic
              calendar on: prayer times, and imsak / iftar times for fasting
     NEWS     Headlines from any RSS feed; AI reporter sums up the news, can translate it
     QUOTE    Classical Chinese, Ancient Greek or modern quotes; AI translates or explains
     FOCUS    Pomodoro timer with breaks, and a guided breathing exercise
     AI       Optional AI: feelings, translations, summaries, quote meanings
     MOTION   Optional MPU6050: pat or tap the case, shake, tilt, catch a drop
     SOUND    Buzzer: chirps, focus alerts, hourly chime, prayer reminders

   One touch pad does everything:
     tap          -> next page
     double tap   -> back to buddy (he's happy to see you)
     hold ~0.7 s  -> page action (pat buddy / 12-24h / refresh / start-pause)
     hold ~2 s    -> focus page: reset the timer, or start breathing when idle

   Night mode: after SLEEP_HOUR the buddy dozes (Zzz), screen dims, then
   switches off after 10 idle minutes to protect the OLED. Touch wakes it.

   Libraries (Library Manager):
     Adafruit SH110X, Adafruit GFX, ArduinoJson (v7), U8g2_for_Adafruit_GFX
     (the MPU6050 needs no library: motion.h talks to it directly)
   OFFLINE  Works out of the box with no WiFi: buddy, focus timer, clock, calendars and
            built-in classical quotes. Hold the pad 6 s, scan the QR code with a phone,
            and the page that opens sets the clock from the phone (no internet needed)
            and, optionally, WiFi for weather, news and AI.
   RTC      Optional DS3231 clock module on the same I2C pins keeps time through power cuts.
   MOTION   Optional MPU6050 on the same I2C pins. With a DS3231 too, tie the MPU6050's AD0
            pin to 3V3 so it moves to address 0x69.
   Board: ESP32 dev board, or ESP32-C3 SuperMini (pins below switch automatically)
   Tools > Partition Scheme > "Huge APP (3MB No OTA/1MB SPIFFS)". Arduino-ESP32 core 3.x.
   Saved settings are kept when you change partition scheme.
  =====================================================================
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "font_tc.h"                         // Traditional Chinese font (Fusion Pixel, OFL)
#include <time.h>
#include <math.h>
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "qrcode.h"                       // Espressif QR encoder (part of core 3.x)
#include "astro.h"                        // sun, moon, prayer times, Hindu calendar
#include "motion.h"                       // MPU6050 driver and gestures
#include "quotes.h"                       // Ancient Greek and modern quotes
#include "games.h"                        // the mini-games page for the phone
#include "font_small.h"                   // tiny 8 px weekday letters for the month grid

// ============================ SETTINGS ===============================
// Owners change these from their phone (setup page); nothing here needs editing.
// The values below are only the starting defaults; saved settings replace them.
static char WIFI_SSID[33]  = "";          // home WiFi (empty = setup mode on first start)
static char WIFI_PASS[65]  = "";
static char WIFI2_SSID[33] = "";          // backup WiFi, e.g. a phone hotspot
static char WIFI2_PASS[65] = "";
static char OWNER_NAME[24] = "";          // used in greetings
static char OWNER_HOBBIES[128] = "";      // owner's hobbies, for more personal AI feelings
static uint8_t OWNER_GENDER = 0;          // 0 not said, 1 male, 2 female, 3 other: for the AI's wording
static uint8_t LANG = 0;                   // Momowo's language (Lang), chosen on the phone page

// Weather location: a suburb or area name, e.g. "Tampines", searched in COUNTRY_CODE.
// Empty = use the IP lookup below (city-level only).
static char SUBURB[64]      = "";
static char COUNTRY_CODE[4] = "SG";

// Location is looked up automatically from your public IP (city-level).
// Set USE_IP_LOCATION = false to always use the fixed spot below instead.
// The fixed spot is also the fallback if the IP lookup keeps failing.
static const bool  USE_IP_LOCATION = true;
static const float FIXED_LAT  = 1.3521f;
static const float FIXED_LON  = 103.8198f;
static const char* FIXED_CITY = "Singapore";

// Timezone is learned automatically (IP lookup + Open-Meteo, DST included).
// This string is only used until the first successful lookup.
static const char* FALLBACK_TZ = "SGT-8";     // Singapore, UTC+8, no daylight saving

static bool USE_24H = true;             // hold on CLOCK page toggles this (remembered)
static const bool HIJRI_AT_SUNSET = false; // true: Islamic date turns over at sunset, not midnight
static int SLEEP_HOUR = 22;             // buddy dozes off (10 pm)
static int WAKE_HOUR  = 7;              // buddy wakes up
static uint8_t FOCUS_MIN = 25;          // Pomodoro length (minutes), set on the phone page
static uint8_t BREAK_MIN = 5;           // short break; every 4th break is three times longer

// Calendars shown beside the Gregorian one (phone page), and what the Islamic one brings
enum : uint8_t { CAL_CHINESE = 1, CAL_ISLAMIC = 2, CAL_BUDDHIST = 4, CAL_HINDU = 8, CAL_JEWISH = 16, CAL_ALL = 31 };
static uint8_t CALS = CAL_CHINESE;
static uint8_t PRAY_METHOD = 255;       // astro::METHODS index; 255 = automatic for the country
static bool    ASR_HANAFI  = false;     // Asr when shadows are twice an object's length
static bool    PRAY_REMIND = true;      // a soft chime and a note at each prayer time

// Quotes: one bit per source, any mix (QKind order): classical Chinese, Ancient Greek, modern,
// Bible, Quran, Torah, Dhammapada, famous people. Momowo takes turns. AI: 0 translate, 1 explain
static uint16_t QUOTE_MASK = 1;
static uint8_t QUOTE_AI  = 0;
static char FAMOUS_PEOPLE[100] = "";    // the owner's own famous people: the AI picks their real quotes

static bool    NEWS_TRANSLATE = false;  // AI translates the headlines into Momowo's language
static uint8_t NEWS_STYLE     = 0;      // AI summary: 0 one sentence, 1 a short news-of-the-day bulletin

static bool    MOTION_ON     = true;    // MPU6050 gestures, if one is fitted
static uint8_t MOTION_SENS   = 2;       // 1 gentle .. 3 sensitive
static bool    MOTION_MIRROR = false;   // eyes look the wrong way when tilted? flip them

// Buzzer on GPIO27. Passive buzzers play tunes; active ones can only beep.
static const bool    BUZZER_PASSIVE   = true;
static uint8_t       BUZZER_VOLUME    = 6;      // 1-10 (passive buzzer only)
static uint8_t       NEWS_VOICE       = 0;      // AI news reporter, chosen on the phone page (see VOICES; 6 = own)
static char NEWS_VOICE_OWN[64] = "";       // the owner's own reporter, described on the phone page
static bool          TOUCH_CLICK      = true;   // tiny tick on every touch
static bool          HOURLY_CHIME     = true;   // double beep on the hour, daytime only
static const int QUOTE_MAX_CHARS = 12;    // classical quote length limit (punctuation counts)
static const bool QUOTE_FOLLOWS_CONTEXT = true;   // match season, weather and time of day

// News: any RSS feed. BBC News top stories by default (credited on screen).
static char NEWS_FEED_URL[160] = "https://www.channelnewsasia.com/api/v1/rss-outbound-feed?_format=xml&category=6311";
static char NEWS_CREDIT[32]     = "CNA";

// Optional AI: one active provider, with independent saved keys and model overrides.
// Provider 0 preserves existing Claude settings and the legacy "aikey" preference.
static uint8_t AI_PROVIDER = 0;
static char AI_API_KEY[512] = "";
static char AI_MODEL_CUSTOM[96] = "";
static const char* const AI_KEY_PREFS[] = {"aikey", "openai_key", "kimi_key", "deepseek_key"};
static const char* const AI_MODEL_PREFS[] = {"claude_model", "openai_model", "kimi_model", "deepseek_model"};
static const char* const AI_DEFAULT_MODELS[] = {"", "gpt-4.1-mini", "kimi-k2.6", "deepseek-flash"};
static const char* const AI_ENDPOINTS[] = {
  "https://api.anthropic.com/v1/messages",
  "https://api.openai.com/v1/chat/completions",
  "https://api.moonshot.ai/v1/chat/completions",
  "https://api.deepseek.com/chat/completions"
};
// Model and effort are chosen on the phone page. Smarter models and more effort cost more and take longer.
static const char* const AI_MODELS[4]  = {"claude-haiku-5-5", "claude-sonnet-5-5", "claude-opus-5-5", "claude-fable-5-1"};
static const char* const AI_EFFORTS[5] = {"low", "medium", "high", "xhigh", "max"};
static const int         AI_THINK_ROOM[5] = {2000, 4000, 8000, 16000, 32000};   // tokens for thinking, on top of the reply
static uint8_t     AI_MODEL_IX          = 0;     // Haiku
static uint8_t     AI_EFFORT_IX         = 0;     // low: short texts don't need long thinking
static uint16_t    AI_DAY_LIMIT         = 100;   // safety cap on cost: requests a day, chosen on the phone page
static volatile uint16_t gAiUsed        = 0;     // today's requests, kept in flash so a restart doesn't reset them
static int16_t     gAiDay               = -1;    // the day they were counted (day of the year)
static uint16_t    FEEL_MIN             = 90;    // with AI: a feeling of its own every this many minutes (0 = off), chosen on the phone page
#define AI_FEELINGS (FEEL_MIN > 0)

// The AI reply arrives streamed (server-sent events). Streaming keeps the connection busy while the
// model thinks, so high effort doesn't time out; only the reply's text is kept.
class SseSink : public Stream {
 public:
  SseSink(char* o, size_t n) : out(o), outSz(n) { out[0] = 0; }
  char* out;
  size_t outSz, len = 0;
  char stop[20] = "";                       // stop_reason: end_turn, max_tokens, refusal...
  bool error = false;
  size_t write(uint8_t c) override {
    if (c == '\n') {
      if (!skip) { line[ll] = 0; take(); }
      ll = 0;
      skip = false;
    } else if (c != '\r') {
      if (ll + 1 < sizeof(line)) line[ll++] = c;
      else {
        line[ll] = 0;
        skip = true;
        // Claude signatures/thinking can exceed the line buffer; no display text is lost.
        if (!strstr(line, "\"signature_delta\"") && !strstr(line, "\"thinking_delta\"")) error = true;
      }
    }
    return 1;
  }
  size_t write(const uint8_t* b, size_t n) override { for (size_t i = 0; i < n; i++) write(b[i]); return n; }
  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }
  void flush() override {}
 private:
  char line[1024];
  size_t ll = 0;
  bool skip = false;
  void take() {
    if (strncmp(line, "data:", 5)) return;
    const char* j = line + 5;
    while (*j == ' ') ++j;
    if (!strcmp(j, "[DONE]")) return;
    JsonDocument d;
    if (deserializeJson(d, j)) { error = true; return; }
    const char* type = d["type"] | "";
    if (!strcmp(type, "error") || !d["error"].isNull()) { error = true; return; }
    const char* text = "";
    if (!strcmp(type, "content_block_delta") &&
        !strcmp(d["delta"]["type"] | "", "text_delta")) text = d["delta"]["text"] | "";
    if (!strcmp(type, "message_delta"))
      strlcpy(stop, d["delta"]["stop_reason"] | "", sizeof(stop));
    if (d["choices"].is<JsonArray>() && d["choices"].size()) {
      text = d["choices"][0]["delta"]["content"] | "";
      if (!d["choices"][0]["delta"]["refusal"].isNull()) error = true;
      if (d["choices"][0]["finish_reason"].is<const char*>())
        strlcpy(stop, d["choices"][0]["finish_reason"], sizeof(stop));
    }
    size_t n = strlen(text);
    if (n >= outSz - len) n = outSz - 1 - len;
    memcpy(out + len, text, n);
    len += n;
    out[len] = 0;
  }
};

// ============================ HARDWARE ===============================
#if CONFIG_IDF_TARGET_ESP32C3    // ESP32-C3 SuperMini: avoids strapping pins 2, 8 and 9
#define TOUCH_PIN  3
#define BUZZER_PIN 4
#define OLED_SDA   5
#define OLED_SCL   6
#else                            // classic ESP32 dev board
#define TOUCH_PIN  12    // NOTE: GPIO12 is a boot strapping pin, must be LOW at power-up
#define BUZZER_PIN 27
#define OLED_SDA   21
#define OLED_SCL   22
#endif

#if CONFIG_FREERTOS_UNICORE      // single-core chips (ESP32-C3): everything on core 0
#define NET_CORE 0
#define SND_CORE 0
#else                            // dual-core ESP32: network on core 0, sound beside loop()
#define NET_CORE 0
#define SND_CORE 1
#endif
#define OLED_ADDR  0x3C
#define SCREEN_W   128
#define SCREEN_H   64

Adafruit_SH1106G display(SCREEN_W, SCREEN_H, &Wire, -1);

static const uint8_t CONTRAST_DAY   = 0x80;   // 0x00-0xFF
static const uint8_t CONTRAST_NIGHT = 0x01;   // dimmed while buddy sleeps
static const uint8_t CONTRAST_IDLE  = 0x20;   // daytime, nobody near for 20 min (saves the OLED)

U8G2_FOR_ADAFRUIT_GFX u8f;                    // draws UTF-8 Chinese into the same buffer
// One 14 px subset font (common GB2312 hanzi) for the quote and its source,
// so the sketch fits the default partition scheme on ESP32 core 3.x. Quotes
// using a character outside it are skipped, so nothing shows as a gap.
// With Tools > Partition Scheme > Huge APP you could use larger, full fonts,
// e.g. u8g2_font_wqy16_t_gb2312 for the quote and wqy12_t_gb2312 for the source.
#define CJK_FONT     u8g2_font_wqy14_t_gb2312b  // quote source line
#define CJK_FONT_BIG u8g2_font_wqy14_t_gb2312b  // the quote itself

#define DEG "\xF8"   // degree symbol in CP437 font

// ============================ TIMING =================================
static const uint32_t FRAME_MS         = 33;              // ~30 fps
static const uint32_t LONG_MS          = 700;
static const uint32_t VLONG_MS         = 2000;
static const uint32_t SETUP_HOLD_MS    = 6000;   // hold this long anywhere: setup mode
static const uint32_t DOUBLE_GAP_MS    = 300;
static const uint32_t IDLE_HOME_MS     = 90000;           // return to buddy
static const uint32_t WEATHER_EVERY_MS = 15UL * 60 * 1000;
static const uint32_t GEO_EVERY_MS     = 6UL * 60 * 60 * 1000;   // re-check if you travel
static const uint32_t HOME_CHECK_MS    = 10UL * 60 * 1000;       // on hotspot: look for home WiFi
static const uint32_t QUOTE_MIN_GAP_MS = 30UL * 60 * 1000;       // context change -> new quote, at most this often
static const uint32_t NEWS_EVERY_MS    = 30UL * 60 * 1000;       // headlines refresh
static const uint32_t BRIEF_EVERY_MS   = 2UL * 60 * 60 * 1000;   // AI news briefing refresh
#define FEEL_EVERY_MS ((uint32_t)FEEL_MIN * 60000UL)                   // AI feeling, waking hours only
static const uint32_t NIGHT_OFF_MS     = 10UL * 60 * 1000;
#define focusMs() (FOCUS_MIN * 60UL * 1000)   // a macro: a function here would come before the types below

// ============================ SHARED DATA ============================
struct WeatherData {
  bool valid = false;
  uint32_t at = 0;                          // millis() when it was fetched
  float temp = 0, feels = 0, tmax = 0, tmin = 0, wind = 0;
  int humidity = 0, code = 0, rainChance = 0;
  bool isDay = true;
  int sunriseMin = -1, sunsetMin = -1;      // today, minutes after midnight
  int tmSunrise = -1, tmSunset = -1;        // tomorrow
  bool tmValid = false;     // tomorrow's forecast
  float tmMax = 0, tmMin = 0, tmWind = 0;
  int tmCode = 0, tmRain = 0;
  uint8_t days = 0;         // daily forecast: 0 today, 1 tomorrow, 2-4 the next three days
  int8_t dMax[5], dMin[5];
  uint8_t dCode[5], dRain[5];
  int aqi = -1;             // US Air Quality Index now (-1 unknown), and particles in ug/m3
  float pm25 = -1, pm10 = -1;
};

enum QKind : uint8_t { Q_CHINESE, Q_GREEK, Q_MODERN, Q_BIBLE, Q_QURAN, Q_TORAH, Q_DHAMMA, Q_FAMOUS, Q_KINDS };
struct QuoteData {
  QKind kind = Q_CHINESE;
  char text[256] = "";      // raw UTF-8: Chinese, Greek or English
  char author[96] = "";
  char tag[8] = "";         // theme shown beside the source, e.g. "雨"
  char meaning[320] = "";   // meaning, translation or explanation (AI or built-in), UTF-8
  int8_t lang = -1;         // famous people: the quote's language (Lang), -1 another one
  uint32_t version = 0;
};

#define NEWS_MAX 8
struct NewsData {
  uint8_t count = 0;
  char title[NEWS_MAX][120];   // headlines, UTF-8 (translated by the AI if asked)
  char brief[640] = "";        // AI briefing: one sentence, or a short bulletin
  uint32_t version = 0;
};

struct LocationData {
  bool valid = false;
  float lat = 0, lon = 0;
  char city[40] = "";                       // English (plain ASCII): for the AI prompts
  char name[64] = "";                       // in the chosen language (UTF-8): shown on screen
};

static WeatherData gWeather;
static QuoteData gQuote;
static LocationData gLoc;

struct CalInfo {                            // today's date in the three calendars
  bool valid = false;
  bool lunarOk = false, leap = false, eve = false;   // eve = Chinese New Year's Eve
  int ly = 0, lm = 0, ld = 0;               // Chinese lunar year / month / day
  int termToday = -1, termNext = -1, termDays = 0;   // solar terms
  int be = 0;                               // Buddhist year (Indonesian: new year at Waisak)
  bool waisak = false;
  const char* christian = nullptr;          // Christmas / Easter day name, English
  int hy = 0, hm = 0, hd = 0;               // Hijri year / month / day
  int wday = 0;
  bool hinduOk = false;                     // Hindu calendar (needs a location for sunrise)
  astro::Panchang pj;
  const char* hinduFest = nullptr;
  bool hebOk = false;                       // Hebrew date (from sunset), festival, Shabbat
  int32_t hebY = 0;
  int hebM = 0, hebD = 0, hebFestN = 0;
  astro::HebFest hebFest = astro::HF_NONE;
  int candles = -1;                         // Friday: Shabbat candle lighting, minutes after midnight
  bool shabbat = false;
  char holToday[48] = "";                   // today's public holiday, if any
  char holNext[48] = "";                    // the next one this year, in holDays days
  int holDays = -1;
  uint32_t holMask = 0;                     // bit d: day d of this month is a public holiday
};

struct PrayerDay {                          // today's prayer times, minutes after midnight
  bool valid = false;
  int key = -1;
  float t[astro::PRAYER_COUNT];
  float tmFajr = NAN;                       // tomorrow's, for the evening countdown
};

struct LunarDay { uint8_t m, d; const char* label; const char* greeting; };
struct Note { uint16_t hz; uint16_t ms; };       // hz 0 = rest

static CalInfo gCal;

// Public holidays in the country Momowo is in (Nager.Date), nationwide ones only.
// Written by the network task, kept through restarts.
struct Holiday { uint8_t m, d; char name[48]; };
static const int HOL_MAX = 28;
static Holiday gHol[HOL_MAX];
static uint8_t gHolCount = 0;
static int16_t gHolYear = 0;
static char gHolCC[4] = "";
static uint8_t gHolLang = 255;
static volatile uint16_t gHolVersion = 0;
static char gHolGreet[96];
static char gHebGreet[96];
static int8_t gHijriAdj = 0;                // -1, 0, +1 days (hold on CALENDAR)
static bool gMuted = false;                 // set on the phone settings page (remembered)
static Preferences prefs;
static SemaphoreHandle_t gMutex;
static SemaphoreHandle_t gScanMutex;        // one WiFi scan at a time: both cores share the scan list
static volatile bool gWifiUp = false;
static volatile bool gOnHotspot = false;   // on the backup network
static volatile bool gEverOnline = false;
static volatile bool gSetupMode = false;   // Momowo's own WiFi + setup page
static volatile uint8_t gWifiReason = 0;   // why the last WiFi connection failed (0 = fine)
static bool gSetupAuto = false;            // opened because home WiFi kept failing
static char gApSsid[16], gApPass[12];
static char gHost[20] = "momowo";          // unique per unit, e.g. momowo-a1b2 (many units, one network)
static uint32_t gSetupAt = 0;              // when setup mode opened
static float gSavedLat = NAN;              // last known latitude, for seasons while offline
static float gSavedLon = NAN;              // and longitude, for prayer times offline
static PrayerDay gPray;
static uint8_t gBreaks = 0;                 // breaks taken this session run, for the long break
static bool gMotion = false;                // MPU6050 found
static motion::State gMs;
static int gShakesToday = 0, gBreathsToday = 0;
static char gCountryCode[4] = "";           // from the IP lookup, e.g. "SG" (picks the prayer method)
static bool gRtc = false;                  // DS3231 clock module found
static volatile uint32_t gRestartAt = 0;
static volatile uint32_t gNetAlive = 0;     // netTask's last pass
static volatile bool gRestartSoon = false;  // memory is getting fragmented: restart when nobody's looking
static WebServer server(80);
static DNSServer dnsServer;
static volatile bool gReqWeather = false;
static volatile bool gReqQuote = false;
static volatile bool gReqNews = false;
static NewsData gNews;

// Timezone learned from the network (seconds east of UTC); loop() applies it
static volatile int32_t  gTzOffset  = 0;
static volatile bool     gTzKnown   = false;
static volatile uint32_t gTzVersion = 0;
static char gTzRule[48] = "";               // the phone's time zone with its summer-time rule (POSIX TZ), if any
static volatile bool gTzRuleOn = false;      // that rule sets the offset (loop() checks it)
static volatile int32_t gTzSaid = 0;         // the offset the network last reported
static volatile uint32_t gTzSaidVer = 0;

// ============================ UI STATE ===============================
enum Page : uint8_t { P_BUDDY, P_CLOCK, P_CAL, P_WEATHER, P_NEWS, P_QUOTE, P_FOCUS, P_COUNT };
enum Lang : uint8_t { L_EN, L_ZHS, L_ZHT, L_ID, L_DE, L_FR, L_JA, L_ES, L_COUNT };   // phone page order
#include "lang.h"                           // screen labels in all eight languages: T(S_...)
struct BuddhistDay { uint8_t m, d; Str name; };
enum Mood : uint8_t { M_NORMAL, M_HAPPY, M_SLEEPY, M_SURPRISED, M_LOVE, M_SAD, M_DIZZY };

struct Feeling {                            // Momowo's own feeling (AI), guarded by gMutex
  bool valid = false;
  Mood mood = M_NORMAL;
  char thought[160] = "";                   // first person, UTF-8
  char pats[3][64];                         // replies to a pat while feeling this way
  uint8_t patCount = 0;
  uint32_t version = 0;
};
static Feeling gFeel;
static int gPatsToday = 0;                    // kept in flash with the focus count's date
static uint32_t gFeelSavedAt = 0, gBriefSavedAt = 0;   // when a restored feeling / briefing was made
static int gQuoteRestoredKey = -3;            // context of a quote restored at start-up (-3: none)
enum FocusState : uint8_t { F_IDLE, F_RUN, F_PAUSE, F_BREAK, F_BREATHE };   // break and breathing count down too
enum Gesture : uint8_t { G_NONE, G_TAP, G_DOUBLE, G_LONG, G_VLONG, G_SETUP, G_HINT };
enum WIcon : uint8_t { W_CLEAR, W_PARTLY, W_CLOUD, W_FOG, W_DRIZZLE, W_RAIN, W_SNOW, W_STORM };
enum QTheme : uint8_t { T_SPRING, T_SUMMER, T_AUTUMN, T_WINTER, T_RAIN, T_SNOW, T_CLOUD, T_WIND,
                        T_SUN, T_MOON, T_STARS, T_NONE };
// One line of the "today in each calendar" view. lunar: drawn in the Chinese lunar font.
struct CalRow { char text[72]; char alt[72]; bool lunar; };
struct LocalQuote { QTheme theme; const char* text; const char* who; const char* from; const char* meaning; };
// 今日诗词 v1 category for each quote theme, and the tag shown beside the source
static const char* const THEME_PATH[] = {
  "siji/chuntian", "siji/xiatian", "siji/qiutian", "siji/dongtian",
  "tianqi/xieyu", "tianqi/xiexue", "tianqi/xieyun", "tianqi/xiefeng",
  "tianqi/taiyang", "tianqi/yueliang", "tianqi/xingxing"};
static const char* const THEME_TAG[] = {"春", "夏", "秋", "冬", "雨", "雪", "云", "风", "日", "月", "星"};
// NOTE: keep every custom type up here. The Arduino IDE inserts function
// prototypes above the first function, so types declared later break them.

static Page gPage = P_BUDDY;
static uint32_t gPageEnter = 0, gLastInteraction = 0, gNightAwakeUntil = 0, gFlashUntil = 0;
static uint32_t gLoopNow = 0;                 // loop()'s "now", for the web handlers it calls
static bool gNight = false, gSleeping = false, gDisplayOn = true, gInverted = false;
static char gAiStatus[64] = "";             // why the last AI request failed, shown on the ticker
static char gNewsStatus[96] = "";           // why the news is missing, shown on the News page

static char gToast[32];
static uint32_t gToastUntil = 0;
static char gTicker[160];                  // big enough for a whole AI feeling, in any language
static uint32_t gTickerStart = 0, gTickerUntil = 0;

static FocusState fState = F_IDLE;
static uint32_t fRemain = 25 * 60000UL, fLastTick = 0, fTotal = 25 * 60000UL;   // fTotal: this phase's length
static int gSessions = 0, gSessionDay = -1;  // today's focus sessions and their date (year*1000+day),
                                             // kept in flash so a restart or settings change keeps them

struct Buddy {
  float gx = 0, gy = 0, tgx = 0, tgy = 0;   // gaze (current / target)
  float h = 26;                             // eye height
  uint32_t nextGaze = 0, nextBlink = 0, blinkAt = 0, nextAntic = 0;
  bool doubleBlink = false;
  Mood ov = M_NORMAL;                       // temporary mood override
  uint32_t ovUntil = 0;
  uint32_t heartsAt = 0;                    // little hearts float up after a pat
} B;

// Mini-games, played on the phone page while Momowo plays along on its screen
enum : uint8_t { GM_NONE, GM_RPS, GM_TTT, GM_GUESS, GM_SIMON, GM_ROLL, GM_RIDDLE, GM_QUEST, GM_TAP };
enum : uint8_t { RD_NONE, RD_THINKING, RD_ASKING, RD_JUDGING, RD_SOLVED, RD_REVEALED, RD_FAILED };
enum : uint8_t { QS_NONE, QS_THINKING, QS_READY, QS_STEP, QS_DONE };          // quest phases
enum : uint8_t { TP_IDLE, TP_WAIT, TP_GO, TP_SHOW, TP_DONE };                 // quick tap phases
#define TAP_ROUNDS 5
enum : uint8_t { QA_TAP, QA_HOLD, QA_LEFT, QA_RIGHT, QA_UP, QA_DOWN, QA_SHAKE, QA_COUNT };   // quest actions
#define QUEST_MAX 8
struct Game {
  uint8_t game = GM_NONE;
  uint32_t last = 0, at = 0;                // the phone's last move; when the current show began
  uint32_t rAt = 0;                         // when the riddle text started (its own: the AI answers any time)
  Mood mood = M_NORMAL;                     // Momowo's reaction, until moodUntil
  uint32_t moodUntil = 0;
  Str say = S_GM_YOUR_TURN;                 // what Momowo says (with sayArg for a %d)
  int sayArg = 0;
  bool sounded = true;                      // the reveal sound has played
  int8_t you = -1, me = -1, result = 0;     // rock paper scissors: 0 rock, 1 paper, 2 scissors;
  uint16_t wins = 0, losses = 0;            // result 0 draw, 1 you win, 2 Momowo wins
  uint8_t board[9] = {};                    // tic-tac-toe: 0 empty, 1 you, 2 Momowo
  uint8_t over = 0, lastMove = 255;         // 0 playing, 1 you won, 2 Momowo won, 3 draw
  uint8_t secret = 0, tries = 0, guess = 0; // guess my number
  int8_t hint = 0;                          // 1 higher, -1 lower, 2 right
  uint8_t seq[40] = {}, len = 0, pos = 0, best = 0;   // memory: arrows up, right, down, left
  bool showing = false;
  uint8_t shown = 255, pressed = 255;       // the arrow lit by the show, and by the owner
  uint32_t pressedAt = 0;
  // star roll: tilt Momowo to roll the ball onto the stars (pixels, pixels per second)
  float bx = 64, by = 40, vx = 0, vy = 0, cx = 20, cy = 30, cvx = 16, cvy = 9;
  int16_t sx = 0, sy = 0;
  uint16_t stars = 0, bestStars = 0;
  bool rolling = false, newBest = false;
  uint32_t startAt = 0, endAt = 0, tickAt = 0, hurtAt = 0, starAt = 0;
  // riddles: the AI makes one up and judges guesses that don't match its answer exactly
  uint8_t rphase = RD_NONE, rtries = 0;
  bool rhint = false;
  char riddle[240] = "", answer[48] = "", alts[160] = "", hintText[120] = "", rguess[48] = "";
  // quest: the AI tells a little adventure; each moment asks for a tap, a hold, a tilt or a shake
  uint8_t qphase = QS_NONE, qn = 0, qstep = 0, qhearts = 0, qscore = 0, qbest = 0;
  uint8_t qact[QUEST_MAX] = {};
  bool qwon = false, qnewBest = false, qarmed = false, qok = false, qstory = false;
  uint32_t qreadyAt = 0, qstepAt = 0, qwin = 0, qwaitUntil = 0;
  float holdRef[3] = {0, 0, 1};             // star roll, quest: how the player holds Momowo; tilts count from here
  char qtitle[64] = "", qtext[QUEST_MAX][100] = {}, qend[2][120] = {};   // ending: 0 won, 1 lost
  // quick tap: wait for the flash and the beep, then tap the pad; the average of 5 rounds counts
  uint8_t tphase = TP_IDLE, tround = 0;
  int16_t tlast = 0;                        // last round: ms, -1 too soon, -2 too slow
  uint16_t tms[TAP_ROUNDS] = {}, tavg = 0, tbest = 0;   // best: lowest average, ms
  bool tnewBest = false;
  uint32_t tgoAt = 0, tfakeAt = 0, tshowUntil = 0, tseen = 0;
} gGame;
static bool gAiForUser = false;            // netTask: this AI request was asked for just now (riddles)
static volatile uint8_t gRiddleReq = 0;     // for netTask: 1 make a riddle, 2 judge the guess
static volatile bool gQuestReq = false;     // for netTask: write a quest story
static uint32_t gTouchPressAt = 0;          // when the finger last went down on the pad

// ============================ HELPERS ================================
static bool nowTm(struct tm& t) {
  time_t n = time(nullptr);
  if (n < 1700000000) return false;         // not synced yet
  localtime_r(&n, &t);
  return true;
}

static bool isNightHour(int h) {
  return SLEEP_HOUR > WAKE_HOUR ? (h >= SLEEP_HOUR || h < WAKE_HOUR)
                                : (h >= SLEEP_HOUR && h < WAKE_HOUR);
}

// Language names for the AI, and the words Momowo says itself (same order as Lang)
static const char* const LANG_NAME[L_COUNT] = {"English", "Simplified Chinese", "Traditional Chinese",
  "Indonesian", "German", "French", "Japanese", "Spanish"};
static const char* const GREET[L_COUNT][4] = {      // night, morning, afternoon, evening
  {"Good night", "Good morning", "Good afternoon", "Good evening"},
  {"晚安", "早上好", "下午好", "晚上好"},
  {"晚安", "早安", "午安", "晚上好"},
  {"Selamat malam", "Selamat pagi", "Selamat siang", "Selamat malam"},
  {"Gute Nacht", "Guten Morgen", "Guten Tag", "Guten Abend"},
  {"Bonne nuit", "Bonjour", "Bon après-midi", "Bonsoir"},
  {"おやすみ", "おはよう", "こんにちは", "こんばんは"},
  {"Buenas noches", "Buenos días", "Buenas tardes", "Buenas tardes"}};
static const char* const THINKING[L_COUNT] = {"Let me think how I feel...", "让我想想现在的心情…",
  "讓我想想現在的心情…", "Sebentar, aku pikir dulu...", "Moment, wie fühle ich mich?",
  "Voyons, comment je me sens...", "今の気持ちを考え中…", "Déjame pensar cómo me siento..."};

static bool langIsCjk() { return LANG == L_ZHS || LANG == L_ZHT || LANG == L_JA; }

static const char* greetingFor(int h) {
  const char* const* g = GREET[LANG < L_COUNT ? LANG : 0];
  if (h < 5)  return g[0];
  if (h < 12) return g[1];
  if (h < 17) return g[2];
  if (h < 21) return g[3];
  return g[0];
}

// Weekday and date in the chosen language, e.g. "Tuesday, 6 October", "10月6日 星期二", "10月6日(火)".
// longMonth: try the full month name first, falling back to the short one if it doesn't fit.
static void fmtDate(const struct tm& t, char* out, size_t n, bool longMonth) {
  const char* wd = WEEKDAY[LANG][t.tm_wday];
  int m = t.tm_mon, d = t.tm_mday;
  if (LANG == L_ZHS || LANG == L_ZHT) { snprintf(out, n, "%d月%d日 %s", m + 1, d, wd); return; }
  if (LANG == L_JA) { snprintf(out, n, "%d月%d日(%s)", m + 1, d, wd); return; }
  for (int pass = longMonth ? 0 : 1; pass < 2; pass++) {
    const char* mon = (pass == 0 ? MONTH_LONG : MONTH_SHORT)[LANG][m];
    if (LANG == L_DE)      snprintf(out, n, "%s, %d. %s", wd, d, mon);
    else if (LANG == L_FR) snprintf(out, n, "%s %d %s", wd, d, mon);
    else if (LANG == L_ES) snprintf(out, n, pass == 0 ? "%s, %d de %s" : "%s, %d %s", wd, d, mon);
    else                   snprintf(out, n, "%s, %d %s", wd, d, mon);   // English, Indonesian
    if (pass == 1 || textWidthPx(out) <= SCREEN_W) return;
  }
}

// "Good morning, Jeffrey!" in the right style for the language
static void greetingLine(int h, char* out, size_t n) {
  const char* g = greetingFor(h);
  if (!OWNER_NAME[0]) snprintf(out, n, langIsCjk() ? "%s！" : "%s!", g);
  else if (LANG == L_JA) snprintf(out, n, "%s、%sさん！", g, OWNER_NAME);
  else if (langIsCjk()) snprintf(out, n, "%s，%s！", g, OWNER_NAME);
  else snprintf(out, n, "%s, %s!", g, OWNER_NAME);
}

static void fmtMinutes(int mins, char* out, size_t n) {   // minutes after midnight
  if (mins < 0 || mins >= 24 * 60) { snprintf(out, n, "--:--"); return; }
  int h = mins / 60, m = mins % 60;
  if (USE_24H) snprintf(out, n, "%02d:%02d", h, m);
  else snprintf(out, n, "%d:%02d%s", h % 12 ? h % 12 : 12, m, h < 12 ? "am" : "pm");
}

static void fmtTime(const struct tm& t, char* out, size_t n) {
  if (USE_24H) strftime(out, n, "%H:%M", &t);
  else {
    int h = t.tm_hour % 12;
    if (!h) h = 12;
    snprintf(out, n, "%d:%02d", h, t.tm_min);
  }
}

// UTF-8 -> plain ASCII the OLED font can show (smart quotes, dashes, accents)
static void asciiize(const char* in, char* out, size_t outSz) {
  static const char LATIN1[] =
    "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYPsaaaaaaaceeeeiiiidnooooo/ouuuuypy";
  const uint8_t* p = (const uint8_t*)in;
  size_t o = 0;
  while (*p && o + 4 < outSz) {
    uint8_t c = *p;
    if (c < 0x80) {
      out[o++] = (c == '\n' || c == '\r' || c == '\t') ? ' ' : (char)c;
      p++;
      continue;
    }
    if (c == 0xC3 && p[1] >= 0x80 && p[1] <= 0xBF) {
      out[o++] = LATIN1[p[1] - 0x80];
      p += 2;
      continue;
    }
    if (c == 0xE2 && p[1] == 0x80 && p[2]) {
      switch (p[2]) {
        case 0x98: case 0x99: out[o++] = '\''; break;
        case 0x9C: case 0x9D: out[o++] = '"';  break;
        case 0x93: case 0x94: out[o++] = '-';  break;
        case 0xA6: out[o++] = '.'; out[o++] = '.'; out[o++] = '.'; break;
        default: break;
      }
      p += 3;
      continue;
    }
    p++;
    while ((*p & 0xC0) == 0x80) p++;        // skip unsupported multibyte char
  }
  out[o] = 0;
}

static int utf8Count(const char* s) {        // characters, not bytes
  int n = 0;
  for (; *s; s++)
    if (((uint8_t)*s & 0xC0) != 0x80) n++;
  return n;
}

static void setQuote(const char* q, const char* a, const char* tag) {
  xSemaphoreTake(gMutex, portMAX_DELAY);
  size_t o = 0;                             // copy without line breaks
  for (const char* c = q; *c && o + 1 < sizeof(gQuote.text); c++)
    if (*c != '\n' && *c != '\r') gQuote.text[o++] = *c;
  gQuote.text[o] = 0;
  strlcpy(gQuote.author, a, sizeof(gQuote.author));
  strlcpy(gQuote.tag, tag, sizeof(gQuote.tag));
  gQuote.kind = Q_CHINESE;
  gQuote.meaning[0] = 0;                    // a new quote needs a new meaning
  gQuote.version++;
  xSemaphoreGive(gMutex);
}

static LocationData getLocation() {
  LocationData l;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  l = gLoc;
  xSemaphoreGive(gMutex);
  return l;
}

static void setTzOffset(int32_t off) {
  if (!gTzKnown || off != gTzOffset) {
    gTzOffset = off;
    gTzKnown = true;
    gTzVersion = gTzVersion + 1;
    if (prefs.getInt("tzoff", INT32_MIN) != off) prefs.putInt("tzoff", off);   // remembered offline
  }
}

// An offset from the network (weather, location). With a saved zone rule, loop() checks it against the rule.
static void netTzOffset(int32_t off) {
  gTzSaid = off;
  gTzSaidVer = gTzSaidVer + 1;
  if (!gTzRuleOn) setTzOffset(off);
}

static int32_t tzEastAt(time_t t) {         // seconds east of UTC at t, by the TZ in force
  struct tm l, g;
  localtime_r(&t, &l);
  gmtime_r(&t, &g);
  int dd = l.tm_year != g.tm_year ? (l.tm_year > g.tm_year ? 1 : -1) : l.tm_yday - g.tm_yday;
  return dd * 86400 + (l.tm_hour - g.tm_hour) * 3600 + (l.tm_min - g.tm_min) * 60 + (l.tm_sec - g.tm_sec);
}

// loop() only: the saved zone rule, which follows summer time even with no network, or else
// seconds-east-of-UTC as a fixed POSIX TZ string (sign is inverted there). force: TZ was reset.
static void applyTimezone(bool force) {
  static char applied[48] = "";
  static uint32_t seenSaid = 0, missSince = 0;
  const time_t now = time(nullptr);
  const bool report = gTzSaidVer != seenSaid;
  seenSaid = gTzSaidVer;
  if (gTzRule[0] && gTzKnown && now > 1700000000) {
    if (force || strcmp(applied, gTzRule)) {
      strlcpy(applied, gTzRule, sizeof(applied));
      setenv("TZ", applied, 1);
      tzset();
    }
    const int32_t east = tzEastAt(now);
    bool drop = false;
    if (report) {
      const int32_t said = gTzSaid;
      if (said == east) missSince = 0;
      else if (said != tzEastAt(now + 182 * 86400L)) drop = true;   // neither of this zone's offsets: moved
      else if (!missSince) missSince = millis() | 1;   // its other one: a report from just before a change?
    }
    if (missSince && millis() - missSince > 26 * 3600000UL) drop = true;   // no: a zone with other dates
    if (!drop) {
      gTzRuleOn = true;
      if (east != gTzOffset) setTzOffset(east);   // summer time began or ended
      return;
    }
    gTzRule[0] = 0;
    prefs.remove("tzrule");
    Serial.println("[clock] the network disagrees with the phone's time zone: rule dropped");
    if (gTzSaidVer) setTzOffset(gTzSaid);
  }
  gTzRuleOn = false;
  missSince = 0;
  char tz[20];
  if (!gTzKnown) {
    strlcpy(tz, FALLBACK_TZ, sizeof(tz));
  } else {
    int32_t off = gTzOffset;
    int32_t a = off < 0 ? -off : off;
    if (a % 3600) snprintf(tz, sizeof(tz), "UTC%c%d:%02d", off >= 0 ? '-' : '+', (int)(a / 3600), (int)(a % 3600 / 60));
    else          snprintf(tz, sizeof(tz), "UTC%c%d", off >= 0 ? '-' : '+', (int)(a / 3600));
  }
  if (force || strcmp(applied, tz)) {
    strlcpy(applied, tz, sizeof(applied));
    setenv("TZ", applied, 1);
    tzset();
  }
}

static WeatherData getWeather() {
  static WeatherData cache;
  if (xSemaphoreTake(gMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    cache = gWeather;
    xSemaphoreGive(gMutex);
  }
  if (cache.valid && millis() - cache.at > 3UL * 3600000) {   // offline for 3 h: don't show it as "now"
    WeatherData old;                        // (sunrise/sunset then come from the calculation)
    return old;
  }
  return cache;
}

// ============================ CALENDARS ==============================
// Chinese lunar calendar 2000-2100, generated from astronomical new moons and
// solar terms (China Standard Time), checked day by day against an independent
// implementation. Per year: bits 0-12 month lengths in order (1 = 30 days, leap
// month included), bits 13-16 leap month follows month N (0 = none), bits 17-22
// day of year of Chinese New Year (0 = 1 Jan).
static const uint32_t LUNAR_INFO[101] = {
  0x460693, 0x2e952b, 0x54052b, 0x3e0a5b, 0x2a555a, 0x4e056a, 0x38fb55, 0x600ba4,
  0x4a0b49, 0x32ba93, 0x580a95, 0x42052d, 0x2c8aad, 0x500ab5, 0x3d35aa, 0x6205d2,
  0x4c0da5, 0x36dd4a, 0x5c0d4a, 0x460c95, 0x30952e, 0x540556, 0x3e0ab5, 0x2a55b2,
  0x5006d2, 0x38cea5, 0x5e0725, 0x48064b, 0x32ac97, 0x560cab, 0x42055a, 0x2c6ad6,
  0x520b69, 0x3d7752, 0x620b52, 0x4c0b25, 0x36da4b, 0x5a0a4b, 0x4404ab, 0x2ea55b,
  0x5405ad, 0x3e0b6a, 0x2a5b52, 0x500d92, 0x3afd25, 0x5e0d25, 0x480a55, 0x32b4ad,
  0x5804b6, 0x4005b5, 0x2c6daa, 0x520ec9, 0x3f1e92, 0x620e92, 0x4c0d26, 0x36ca56,
  0x5a0a57, 0x440556, 0x2e86d5, 0x540755, 0x400749, 0x286e93, 0x4e0693, 0x38f52b,
  0x5e052b, 0x460a5b, 0x32b55a, 0x58056a, 0x420b65, 0x2c974a, 0x520b4a, 0x3d1a95,
  0x620a95, 0x4a052d, 0x34caad, 0x5a0ab5, 0x4605aa, 0x2e8ba5, 0x540da5, 0x400d4a,
  0x2a7c95, 0x4e0c96, 0x38f94e, 0x5e0556, 0x480ab5, 0x32b5b2, 0x5806d2, 0x420ea5,
  0x2e8e4a, 0x50068b, 0x3b0c97, 0x6004ab, 0x4a055b, 0x34cad6, 0x5a0b6a, 0x460752,
  0x309725, 0x540b45, 0x3e0a8b, 0x28549b, 0x4e04ab
};
// 24 solar terms 2000-2100 (same source): day of month = TERM_BASE[k] + 2-bit offset
static const uint8_t TERM_BASE[24] = {4, 19, 3, 18, 4, 19, 4, 19, 4, 20, 4, 20, 6, 22, 6, 22, 6, 22, 7, 22, 6, 21, 6, 21};
static const uint8_t TERM_OFS[101][6] = {
  {0x5a,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x66,0x65,0x56,0x55}, {0x56,0x5a,0xa6,0x65,0xa6,0x5a},
  {0x5a,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x65,0x55,0x56,0x55}, {0x56,0x5a,0xa6,0x65,0xa6,0x5a},
  {0x5a,0x45,0x55,0x51,0x51,0x15}, {0x15,0x45,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x65,0x55,0x56,0x55}, {0x56,0x5a,0xa6,0x65,0x96,0x5a},
  {0x5a,0x45,0x51,0x51,0x51,0x15}, {0x15,0x45,0x55,0x51,0x55,0x55}, {0x55,0x5a,0x65,0x55,0x56,0x55}, {0x56,0x5a,0xa6,0x65,0x96,0x56},
  {0x56,0x05,0x51,0x51,0x51,0x15}, {0x05,0x45,0x55,0x51,0x55,0x55}, {0x55,0x59,0x65,0x55,0x56,0x55}, {0x55,0x5a,0x66,0x65,0x96,0x56},
  {0x56,0x05,0x51,0x10,0x51,0x15}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x55,0x55,0x65,0x55,0x55,0x55}, {0x55,0x5a,0x66,0x65,0x96,0x56},
  {0x56,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x66,0x65,0x56,0x55},
  {0x56,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x66,0x65,0x56,0x55},
  {0x56,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x65,0x55,0x56,0x55},
  {0x56,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x65,0x55,0x56,0x55},
  {0x56,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x51,0x51,0x51,0x15}, {0x15,0x45,0x55,0x55,0x55,0x55}, {0x55,0x5a,0x65,0x55,0x56,0x55},
  {0x56,0x05,0x51,0x10,0x41,0x05}, {0x05,0x05,0x51,0x51,0x51,0x15}, {0x15,0x45,0x55,0x51,0x55,0x55}, {0x55,0x5a,0x65,0x55,0x56,0x55},
  {0x56,0x05,0x11,0x10,0x41,0x01}, {0x01,0x05,0x51,0x10,0x51,0x15}, {0x05,0x45,0x55,0x51,0x55,0x55}, {0x55,0x55,0x65,0x55,0x55,0x55},
  {0x55,0x05,0x11,0x10,0x41,0x01}, {0x01,0x05,0x51,0x10,0x51,0x15}, {0x05,0x45,0x55,0x51,0x55,0x55}, {0x55,0x55,0x55,0x55,0x55,0x55},
  {0x55,0x05,0x11,0x10,0x41,0x01}, {0x01,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x55,0x55,0x55,0x55,0x55,0x55},
  {0x55,0x05,0x11,0x10,0x01,0x00}, {0x01,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55},
  {0x55,0x05,0x11,0x10,0x01,0x00}, {0x01,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55},
  {0x55,0x05,0x10,0x00,0x01,0x00}, {0x01,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x51,0x51,0x51,0x15}, {0x15,0x55,0x55,0x55,0x55,0x55},
  {0x55,0x05,0x10,0x00,0x01,0x00}, {0x01,0x05,0x51,0x10,0x41,0x05}, {0x05,0x45,0x51,0x51,0x51,0x15}, {0x15,0x45,0x55,0x51,0x55,0x55},
  {0x55,0x05,0x10,0x00,0x01,0x00}, {0x01,0x05,0x51,0x10,0x41,0x05}, {0x05,0x05,0x51,0x50,0x51,0x15}, {0x15,0x45,0x55,0x51,0x55,0x55},
  {0x55,0x05,0x10,0x00,0x01,0x00}, {0x01,0x05,0x11,0x10,0x41,0x01}, {0x05,0x05,0x51,0x10,0x51,0x15}, {0x05,0x45,0x55,0x51,0x55,0x55},
  {0x55,0x00,0x10,0x00,0x00,0x00}, {0x00,0x05,0x11,0x10,0x41,0x01}, {0x01,0x05,0x51,0x10,0x51,0x15}, {0x05,0x45,0x55,0x51,0x55,0x55},
  {0x55,0x00,0x00,0x00,0x00,0x00}, {0x00,0x05,0x11,0x10,0x41,0x01}, {0x01,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15},
  {0x55,0x00,0x00,0x00,0x00,0x00}, {0x00,0x05,0x11,0x10,0x01,0x00}, {0x01,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15},
  {0x15,0x00,0x00,0x00,0x00,0x00}, {0x00,0x05,0x11,0x00,0x01,0x00}, {0x01,0x05,0x51,0x10,0x51,0x05}, {0x05,0x45,0x55,0x51,0x55,0x15},
  {0x15,0x55,0x55,0x55,0x55,0x55}
};
// Umm al-Qura month lengths 1440-1500 AH (2018-2077): bit m set = month m+1 has 30 days
static const uint16_t UAQ_MONTHS[61] = {
  0x2ba, 0x5b5, 0x5aa, 0xd55, 0xa9a, 0x92e, 0x26e, 0x55d, 0xada, 0x6d4,
  0x6a5, 0x54b, 0xa97, 0x54e, 0xaae, 0x5ac, 0xba9, 0xd92, 0xb25, 0x64b,
  0xcab, 0x55a, 0xb55, 0x6d2, 0xea5, 0xe4a, 0xa95, 0x52d, 0xaad, 0x36c,
  0x759, 0x6d2, 0x695, 0x52d, 0xa5b, 0x4ba, 0x9ba, 0x3b4, 0xb69, 0xb52,
  0xaa6, 0x4b6, 0x96d, 0x2ec, 0x6d9, 0xeb2, 0xd54, 0xd2a, 0xa56, 0x4ae,
  0x96d, 0xd6a, 0xb54, 0xb29, 0xa93, 0x52b, 0xa57, 0x536, 0xab5, 0x6aa,
  0xe93
};
static const int32_t UAQ_EPOCH_JDN = 2458373;   // 1 Muharram 1440 = 2018-09-11

static int32_t jdn(int y, int m, int d) {         // Julian day number (Gregorian)
  int a = (14 - m) / 12;
  int32_t yy = y + 4800 - a;
  int mm = m + 12 * a - 3;
  return d + (153 * mm + 2) / 5 + 365 * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
}

// Gregorian -> Chinese lunar date (2000-2100). Returns false outside the table.
static bool lunarFromSolar(int y, int m, int d, int& ly, int& lm, int& ld, bool& leap) {
  if (y < 2000 || y > 2100) return false;
  int32_t day = jdn(y, m, d);
  ly = y;
  int32_t cny = jdn(y, 1, 1) + (int32_t)(LUNAR_INFO[y - 2000] >> 17);
  if (day < cny) {
    if (y == 2000) return false;
    ly = y - 1;
    cny = jdn(ly, 1, 1) + (int32_t)(LUNAR_INFO[ly - 2000] >> 17);
  }
  int32_t off = day - cny;
  uint32_t info = LUNAR_INFO[ly - 2000];
  int leapAfter = (info >> 13) & 15, n = leapAfter ? 13 : 12, num = 0;
  for (int i = 0; i < n; i++) {
    int len = ((info >> i) & 1) ? 30 : 29;
    if (leapAfter && i == leapAfter) leap = true;
    else { num++; leap = false; }
    if (off < len) { lm = num; ld = off + 1; return true; }
    off -= len;
  }
  return false;
}

static int termDay(int y, int k) {                 // k: 0 = 小寒 ... 23 = 冬至
  return TERM_BASE[k] + ((TERM_OFS[y - 2000][k >> 2] >> ((k & 3) * 2)) & 3);
}

// Today's solar term (-1 if none), and the next one with days until it
static void termInfo(int y, int m, int d, int& today, int& next, int& days) {
  int32_t t = jdn(y, m, d);
  today = -1; next = -1; days = 0;
  for (int step = 0; step < 4; step++) {           // this month's two terms, then next month's
    int yy = y, k = (m - 1) * 2 + step;
    if (k >= 24) { k -= 24; yy++; }
    if (yy > 2100) return;
    int32_t tj = jdn(yy, k / 2 + 1, termDay(yy, k));
    if (tj == t) today = k;
    if (tj > t) { next = k; days = tj - t; return; }
  }
}

// Tabular Islamic calendar (civil epoch): fallback outside the Umm al-Qura table
static void hijriTabular(int32_t j, int& hy, int& hm, int& hd) {
  int32_t l = j - 1948440 + 10632;
  int32_t n = (l - 1) / 10631;
  l = l - 10631 * n + 354;
  int32_t k = ((10985 - l) / 5316) * ((50 * l) / 17719) + (l / 5670) * ((43 * l) / 15238);
  l = l - ((30 - k) / 15) * ((17719 * k) / 50) - (k / 16) * ((15238 * k) / 43) + 29;
  hm = (24 * l) / 709;
  hd = l - (709 * hm) / 24;
  hy = 30 * n + k - 30;
}

// Julian day -> Hijri: Umm al-Qura 1440-1500 AH, tabular outside it
static bool hijriFromJdn(int32_t j, int& hy, int& hm, int& hd) {
  int32_t off = j - UAQ_EPOCH_JDN;
  if (off >= 0) {
    for (int yi = 0; yi < 61; yi++) {
      for (int mi = 0; mi < 12; mi++) {
        int len = ((UAQ_MONTHS[yi] >> mi) & 1) ? 30 : 29;
        if (off < len) { hy = 1440 + yi; hm = mi + 1; hd = off + 1; return true; }
        off -= len;
      }
    }
  }
  hijriTabular(j, hy, hm, hd);
  return false;
}

static const char* const CN_STEM[10]   = {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"};
static const char* const CN_BRANCH[12] = {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"};
static const char* const CN_ZODIAC[12] = {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"};
static const char* const CN_MONTH[12]  = {"正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"};
static const char* const CN_TERM[24] = {
  "小寒", "大寒", "立春", "雨水", "惊蛰", "春分", "清明", "谷雨", "立夏", "小满", "芒种", "夏至",
  "小暑", "大暑", "立秋", "处暑", "白露", "秋分", "寒露", "霜降", "立冬", "小雪", "大雪", "冬至"};
// Hijri months in each Latin-script language (Chinese and Japanese write the month as a number)
static const char* const HIJRI_MONTH[L_COUNT][12] = {
  {"Muharram", "Safar", "Rabi I", "Rabi II", "Jumada I", "Jumada II", "Rajab", "Sha'ban", "Ramadan",
   "Shawwal", "Dhu al-Qa'dah", "Dhu al-Hijjah"},
  {}, {},
  {"Muharam", "Safar", "Rabiulawal", "Rabiulakhir", "Jumadilawal", "Jumadilakhir",   // KBBI
   "Rajab", "Syakban", "Ramadan", "Syawal", "Zulkaidah", "Zulhijah"},
  {"Muharram", "Safar", "Rabi' I", "Rabi' II", "Dschumada I", "Dschumada II", "Radschab", "Scha'ban",
   "Ramadan", "Schawwal", "Dhu l-Qa'da", "Dhu l-Hiddscha"},
  {"Mouharram", "Safar", "Rabia I", "Rabia II", "Joumada I", "Joumada II", "Rajab", "Chaabane",
   "Ramadan", "Chawwal", "Dhou al-Qi'da", "Dhou al-Hijja"},
  {},
  {"Muharram", "Safar", "Rabi I", "Rabi II", "Yumada I", "Yumada II", "Rayab", "Shabán", "Ramadán",
   "Shawwal", "Dhu al-Qada", "Dhu al-Hiyya"}};

// Chinese festivals on the lunar calendar (greetings for the buddy's ticker)
static const LunarDay LUNAR_DAYS[] = {
  {1, 1, "春节", "Happy Chinese New Year!"},
  {1, 15, "元宵节", "Happy Lantern Festival!"},
  {5, 5, "端午节", "Happy Dragon Boat Festival!"},
  {7, 7, "七夕", "Happy Qixi Festival!"},
  {7, 15, "中元节", "Ghost Festival today"},
  {8, 15, "中秋节", "Happy Mid-Autumn Festival!"},
  {9, 9, "重阳节", "Happy Double Ninth Festival!"},
  {12, 8, "腊八节", "Laba Festival today"},
};

// Buddhist days (Mahayana, lunar calendar). Vesak (Waisak) is computed separately.
static const BuddhistDay BUDDHIST_DAYS[] = {
  {2, 19, S_B_GUANYIN}, {4, 8, S_B_BUDDHA}, {7, 15, S_B_ULAMBANA}, {12, 8, S_B_BODHI}};

// Waisak 2000-2100 (Indonesia): lunar day (14 + 2 bits) of the full moon, in WIB,
// in the 4th Chinese lunar month. Matches the official dates 2019-2026.
static const uint8_t WAISAK_FM[26] = {0x95, 0xae, 0x56, 0xba, 0x5a, 0xe5, 0x56, 0xa9, 0x6b, 0xa5, 0xbe, 0x95, 0xea, 0x46, 0xa9, 0x5a, 0xf9, 0x6a, 0xa5, 0xab, 0x55, 0x6a, 0x95, 0xba, 0x55, 0x02};

static int waisakDay(int y) {                            // lunar day of Waisak in month 4
  int i = y - 2000;
  return 14 + ((WAISAK_FM[i >> 2] >> ((i & 3) * 2)) & 3);
}

static const LunarDay* lunarFestival(const CalInfo& c) {
  if (!c.lunarOk || c.leap) return nullptr;
  for (const LunarDay& f : LUNAR_DAYS)
    if (f.m == c.lm && f.d == c.ld) return &f;
  return nullptr;
}

static void cnDay(int d, char* out, size_t n) {          // 初一 ... 二十一 ... 三十
  static const char* const DIG[11] = {"", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
  if (d <= 10) snprintf(out, n, "初%s", DIG[d]);
  else if (d < 20) snprintf(out, n, "十%s", DIG[d - 10]);
  else if (d < 30) snprintf(out, n, "二十%s", DIG[d - 20]);   // 廿 isn't in the font
  else snprintf(out, n, "三十");
}

// Second Chinese line: festival, New Year's Eve, solar term, or the next term
static void lunarDayText(const CalInfo& c, char* out, size_t n) {
  const LunarDay* f = lunarFestival(c);
  if (f) snprintf(out, n, "%s", f->label);
  else if (c.eve) snprintf(out, n, "除夕");
  else if (c.termToday >= 0) snprintf(out, n, "今日%s", CN_TERM[c.termToday]);
  else if (c.termNext >= 0) snprintf(out, n, "%d天后%s", c.termDays, CN_TERM[c.termNext]);
  else out[0] = 0;
}

// Easter Sunday (Western, Gregorian): anonymous Gregorian algorithm, checked 2000-2100
static void easterDate(int y, int& m, int& d) {
  int a = y % 19, b = y / 100, c = y % 100, q = b / 4, e = b % 4, f = (b + 8) / 25, g = (b - f + 1) / 3;
  int h = (19 * a + b - q - g + 15) % 30, i = c / 4, k = c % 4;
  int l = (32 + 2 * e + 2 * i - h - k) % 7, mm = (a + 11 * h + 22 * l) / 451;
  m = (h + l - 7 * mm + 114) / 31;
  d = (h + l - 7 * mm + 114) % 31 + 1;
}

// Christmas and Easter days, or nullptr
static const char* christianDay(int y, int m, int d) {
  if (m == 12 && d == 24) return "Christmas Eve";
  if (m == 12 && d == 25) return "Christmas Day";
  int em, ed;
  easterDate(y, em, ed);
  int32_t diff = jdn(y, m, d) - jdn(y, em, ed);
  if (diff == -2) return "Good Friday";
  if (diff == 0)  return "Easter Sunday";
  return nullptr;
}

// Buddhist day in the chosen language, or nullptr
static const char* buddhistDay(const CalInfo& c, bool withUposatha) {
  if (!c.lunarOk) return nullptr;
  if (c.waisak) return T(S_G_VESAK);
  if (c.leap) return nullptr;
  for (const BuddhistDay& b : BUDDHIST_DAYS)
    if (b.m == c.lm && b.d == c.ld) return T(b.name);
  if (withUposatha && (c.ld == 1 || c.ld == 15)) return T(S_B_UPOSATHA);   // 初一 / 十五 observance days
  return nullptr;
}

// Islamic day in the chosen language, or nullptr
static const char* hijriEvent(const CalInfo& c) {
  if (c.hm == 1 && c.hd == 1)   return T(S_G_HIJRI_NY);
  if (c.hm == 3 && c.hd == 12)  return T(S_G_MAWLID);
  if (c.hm == 7 && c.hd == 27)  return T(S_G_ISRA);
  if (c.hm == 9 && c.hd == 1)   return T(S_G_RAMADAN);
  if (c.hm == 10 && c.hd == 1)  return T(S_G_EID_FITR);
  if (c.hm == 12 && c.hd == 9)  return T(S_G_ARAFAH);
  if (c.hm == 12 && c.hd == 10) return T(S_G_EID_ADHA);
  return nullptr;
}

static char gHinduGreet[64];                // "Happy Diwali!" in the chosen language (loop() only)

// Festival greeting for the buddy's ticker, if today is special (only for the calendars chosen)
// A Jewish festival in the chosen language, e.g. "Hanukkah, day 3"
static void hebFestText(const CalInfo& c, char* buf, size_t n) {
  static const Str NAME[] = {S_COUNT, S_JF_ROSH, S_JF_YOM_KIPPUR, S_JF_SUKKOT, S_JF_SHEMINI, S_JF_SIMCHAT,
                             S_JF_SHEMINI_SIMCHAT, S_JF_HANUKKAH, S_JF_TU_BISHVAT, S_JF_PURIM, S_JF_PESACH,
                             S_JF_SHAVUOT, S_JF_TISHA_BAV};
  buf[0] = 0;
  if (c.hebFest == astro::HF_NONE || c.hebFest >= sizeof(NAME) / sizeof(NAME[0])) return;
  snprintf(buf, n, T(NAME[c.hebFest]), c.hebFestN);
}

static const char* calGreeting(const CalInfo& c) {
  if (!c.valid) return nullptr;
  const LunarDay* f = (CALS & CAL_CHINESE) ? lunarFestival(c) : nullptr;
  if (f) {                                  // Chinese festivals: in the chosen language
    int k = f->m * 100 + f->d;
    return T(k == 101 ? S_G_CNY : k == 115 ? S_G_LANTERN : k == 505 ? S_G_DRAGON : k == 707 ? S_G_QIXI
           : k == 715 ? S_G_GHOST : k == 815 ? S_G_MIDAUTUMN : k == 909 ? S_G_DOUBLE9 : S_G_LABA);
  }
  if ((CALS & CAL_CHINESE) && c.eve) return T(S_G_CNY_EVE);
  if (CALS & CAL_BUDDHIST) {
    const char* b = buddhistDay(c, false);
    if (b) return b;
  }
  if (CALS & CAL_ISLAMIC) {
    const char* e = hijriEvent(c);
    if (e) return e;
  }
  if ((CALS & CAL_HINDU) && c.hinduFest) {
    char name[40];                          // "Diwali / Deepavali" -> the first name only
    strlcpy(name, c.hinduFest, sizeof(name));
    if (char* sl = strstr(name, " / ")) *sl = 0;
    snprintf(gHinduGreet, sizeof(gHinduGreet), T(S_HAPPY_FMT), name);
    return gHinduGreet;
  }
  if (CALS & CAL_JEWISH) {
    if (c.hebFest == astro::HF_ROSH) return T(S_G_SHANA_TOVA);
    if (c.hebFest == astro::HF_YOM_KIPPUR) return T(S_G_YOM_KIPPUR);
    if (c.hebFest == astro::HF_HANUKKAH) return T(S_G_HANUKKAH);
    if (c.hebFest != astro::HF_NONE) {      // Tisha B'Av is a fast day: no "happy"
      char f[64];
      hebFestText(c, f, sizeof(f));
      snprintf(gHebGreet, sizeof(gHebGreet), T(c.hebFest == astro::HF_TISHA_BAV ? S_G_FAST : S_G_CHAG), f);
      return gHebGreet;
    }
    if (c.shabbat) return T(S_G_SHABBAT);
  }
  if (c.christian) {
    if (c.christian[0] == 'C') return c.christian[10] == 'E' ? T(S_G_XMAS_EVE) : T(S_G_XMAS);
    if (c.christian[0] == 'E') return T(S_G_EASTER);
    return T(S_G_GOOD_FRIDAY);
  }
  if (c.holToday[0]) {                      // a public holiday where you are
    snprintf(gHolGreet, sizeof(gHolGreet), T(S_HOL_GREET), c.holToday);
    return gHolGreet;
  }
  return nullptr;
}

// Where Momowo is, for prayer times and sunrise: the live location, else the last one saved
static bool placeKnown(double& lat, double& lon) {
  LocationData l = getLocation();
  if (l.valid) { lat = l.lat; lon = l.lon; return true; }
  if (!isnan(gSavedLat) && !isnan(gSavedLon)) { lat = gSavedLat; lon = gSavedLon; return true; }
  lat = FIXED_LAT; lon = FIXED_LON;
  return false;
}

static double tzHours() { return gTzKnown ? gTzOffset / 3600.0 : 8.0; }   // FALLBACK_TZ is UTC+8

static int prayerMethod() {
  if (PRAY_METHOD < astro::METHOD_COUNT) return PRAY_METHOD;
  return astro::methodForCountry(homeCountry());
}

// Today's prayer times: once a day, or when the place or time zone changes
static void updatePrayers() {
  struct tm t;
  if (!(CALS & CAL_ISLAMIC) || !nowTm(t)) return;
  double lat, lon;
  bool known = placeKnown(lat, lon);
  // the method (country found after boot) and Ramadan (Hijri adjustment) change the times too
  int key = ((t.tm_yday * 7919 + (int)lround(lat * 10) * 31 + (int)lround(lon * 10)) ^ (int)(tzHours() * 4)) ^
            (prayerMethod() << 24) ^ ((gCal.hm == 9) << 29);
  if (gPray.valid && key == gPray.key) return;
  PrayerDay p;
  int y = t.tm_year + 1900, m = t.tm_mon + 1, d = t.tm_mday;
  double out[astro::PRAYER_COUNT];
  astro::prayerTimes(y, m, d, lat, lon, tzHours(), prayerMethod(), ASR_HANAFI, gCal.hm == 9, out);
  for (int i = 0; i < astro::PRAYER_COUNT; i++) p.t[i] = out[i];
  struct tm tt = t;                         // tomorrow's Fajr, for the countdown after Isha
  tt.tm_mday += 1;
  tt.tm_hour = 12;
  mktime(&tt);
  astro::prayerTimes(tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, lat, lon, tzHours(), prayerMethod(),
                     ASR_HANAFI, gCal.hm == 9, out);
  p.tmFajr = out[astro::FAJR];
  p.valid = true;
  (void)known;                              // before the first location: Singapore, until it's known
  p.key = key;
  gPray = p;
}

// Recompute when the day, the sunset flip or the Hijri adjustment changes
// Today's local sunset (minutes after midnight): from the forecast, else calculated
static int sunsetMinutes(const WeatherData& w, int y, int m, int d) {
  if (w.valid && w.sunsetMin >= 0) return w.sunsetMin;
  double lat, lon, out[astro::PRAYER_COUNT];
  placeKnown(lat, lon);
  astro::prayerTimes(y, m, d, lat, lon, tzHours(), 0, false, false, out);   // Maghrib = sunset here
  return isnan(out[astro::MAGHRIB]) ? -1 : (int)lround(out[astro::MAGHRIB]);
}

static void updateCalendar() {
  static int lastKey = -1;
  struct tm t;
  if (!nowTm(t)) { gCal.valid = false; return; }
  WeatherData w = getWeather();
  bool afterSunset = HIJRI_AT_SUNSET && w.valid && w.sunsetMin >= 0 &&
                     t.tm_hour * 60 + t.tm_min >= w.sunsetMin;
  int ssMin = sunsetMinutes(w, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
  bool pastSunset = ssMin >= 0 && t.tm_hour * 60 + t.tm_min >= ssMin;   // the Hebrew day begins
  int key = ((((t.tm_year * 400 + t.tm_yday) * 2 + (afterSunset ? 1 : 0)) * 4 + (gHijriAdj + 1)) * 64 +
             (int)(gTzOffset / 1800) + (gLoc.valid ? 32 : 0)) * 2 + (pastSunset ? 1 : 0);
  { const char* hc = homeCountry(); key = key * 2 + (hc[0] == 'I' && hc[1] == 'L'); }   // Israel's festival days
  static uint16_t lastHol = 0xFFFF;
  if (key == lastKey && gCal.valid && lastHol == gHolVersion) return;
  lastKey = key;
  lastHol = gHolVersion;

  CalInfo c;
  int y = t.tm_year + 1900, m = t.tm_mon + 1, d = t.tm_mday;
  c.valid = true;
  c.wday = t.tm_wday;
  c.lunarOk = lunarFromSolar(y, m, d, c.ly, c.lm, c.ld, c.leap);
  // Indonesian Buddhist year: Gregorian + 543, +1 from Waisak (full moon of the 4th lunar month)
  bool afterWaisak = m > 6;                               // rough, outside 2000-2100
  if (c.lunarOk) {
    afterWaisak = false;
    if (c.ly == y) {
      int wd = waisakDay(y);
      afterWaisak = c.lm > 4 || (c.lm == 4 && (c.leap || c.ld >= wd));
      c.waisak = c.lm == 4 && !c.leap && c.ld == wd;
    }
  }
  c.be = y + 543 + (afterWaisak ? 1 : 0);
  c.christian = christianDay(y, m, d);
  if (c.lunarOk) {
    struct tm tt = t;                                     // tomorrow = 正月初一 -> today is 除夕
    tt.tm_mday += 1;
    tt.tm_hour = 12;
    mktime(&tt);
    int a, b, dd;
    bool L;
    c.eve = lunarFromSolar(tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, a, b, dd, L) && b == 1 && dd == 1 && !L;
  }
  if (y >= 2000 && y <= 2100) termInfo(y, m, d, c.termToday, c.termNext, c.termDays);
  hijriFromJdn(jdn(y, m, d) + gHijriAdj + (afterSunset ? 1 : 0), c.hy, c.hm, c.hd);
  if (CALS & CAL_HINDU) {                   // Hindu: the tithi at local sunrise
    double lat, lon, out[astro::PRAYER_COUNT];
    placeKnown(lat, lon);
    astro::prayerTimes(y, m, d, lat, lon, tzHours(), 0, false, false, out);
    double day0 = astro::julianDay(y, m, d) - tzHours() / 24.0;   // local midnight, as UT
    double rise = isnan(out[astro::SUNRISE]) ? 6 * 60 : out[astro::SUNRISE];
    double set = isnan(out[astro::MAGHRIB]) ? 18 * 60 : out[astro::MAGHRIB];
    double jdRise = day0 + rise / 1440.0, jdSet = day0 + set / 1440.0;
    c.pj = astro::panchang(jdRise, y, m);
    c.hinduFest = astro::hinduFestival(c.pj, jdRise, jdSet);
    c.hinduOk = true;
  }
  {                                         // Hebrew: the new day begins at sunset
    int32_t hy;
    int hm, hd;
    astro::hebFromRd(jdn(y, m, d) - 1721425 + (pastSunset ? 1 : 0), hy, hm, hd);
    c.hebOk = true;
    c.hebY = hy;
    c.hebM = hm;
    c.hebD = hd;
    int dow = (t.tm_wday + (pastSunset ? 1 : 0)) % 7;
    const char* cc = homeCountry();
    c.hebFest = astro::hebFestival(hy, hm, hd, dow, cc[0] == 'I' && cc[1] == 'L', &c.hebFestN);
    if (t.tm_wday == 5 && ssMin >= 18) c.candles = ssMin - 18;   // Friday: 18 min before sunset
    c.shabbat = dow == 6;                   // Friday evening until Saturday's sunset
  }
  xSemaphoreTake(gMutex, portMAX_DELAY);    // read by the network task too
  if (gHolYear == y || gHolYear == y - 1) { // public holidays: today, the next one, and this month's
    int today = jdn(y, m, d);
    for (int i = 0; i < gHolCount; i++)
      for (int hy = gHolYear; hy <= gHolYear + 1; hy++) {
        // early January comes round again on the same dates: seen from late December, and
        // on New Year's Day until the new year's list arrives
        if (hy > gHolYear && !(gHol[i].m == 1 && gHol[i].d <= 7)) break;
        int diff = jdn(hy, gHol[i].m, gHol[i].d) - today;
        if (hy == y && gHol[i].m == m) c.holMask |= 1UL << gHol[i].d;
        if (diff == 0 && !c.holToday[0]) strlcpy(c.holToday, gHol[i].name, sizeof(c.holToday));
        else if (diff > 0 && (c.holDays < 0 || diff < c.holDays)) {
          c.holDays = diff;
          strlcpy(c.holNext, gHol[i].name, sizeof(c.holNext));
        }
      }
  }
  gCal = c;
  xSemaphoreGive(gMutex);
}

// ============================ CLOCK CHIP (optional) ==================
// A DS3231 module on the display's I2C pins keeps the time through power cuts, so
// Momowo needs neither WiFi nor a phone once the time has been set. Stored as UTC.
#define RTC_ADDR 0x68
static uint8_t bcd2bin(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }
static uint8_t bin2bcd(uint8_t v) { return ((v / 10) << 4) | (v % 10); }

static bool rtcLoad() {                     // clock chip -> system time; false if absent or unset
  if (motion::isMpu(RTC_ADDR)) return false;   // that's the MPU6050 (AD0 low), not a clock chip
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return false;
  gRtc = true;
  if (Wire.requestFrom(RTC_ADDR, 7) != 7) return false;
  uint8_t r[7];
  for (int i = 0; i < 7; i++) r[i] = Wire.read();
  int y = 2000 + bcd2bin(r[6]), mo = bcd2bin(r[5] & 0x1F), d = bcd2bin(r[4]);
  int h = bcd2bin(r[2] & 0x3F), mi = bcd2bin(r[1] & 0x7F), sec = bcd2bin(r[0] & 0x7F);
  if (y < 2024 || mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 || mi > 59 || sec > 59) return false;
  struct timeval tv = { (time_t)(jdn(y, mo, d) - jdn(1970, 1, 1)) * 86400 + h * 3600 + mi * 60 + sec, 0 };
  settimeofday(&tv, nullptr);
  Serial.println("[clock] time loaded from the clock chip");
  return true;
}

static void rtcSave() {                     // system time -> clock chip
  time_t now = time(nullptr);
  if (!gRtc || now < 1700000000) return;
  struct tm u;
  gmtime_r(&now, &u);
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00);
  Wire.write(bin2bcd(u.tm_sec));
  Wire.write(bin2bcd(u.tm_min));
  Wire.write(bin2bcd(u.tm_hour));           // 24-hour mode
  Wire.write(u.tm_wday + 1);
  Wire.write(bin2bcd(u.tm_mday));
  Wire.write(bin2bcd(u.tm_mon + 1));
  Wire.write(bin2bcd(u.tm_year - 100));
  Wire.endTransmission();
}

// ============================ NETWORK (core 0) =======================
// A web reply's Date ("Mon, 19 Oct 2026 01:14:26 GMT") sets the clock while NTP hasn't answered
// (some office and guest networks block it). NTP still takes over if it ever does.
static void timeFromHttpDate(const char* d) {
  static const char MON[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char m[4];
  int dd, yy, hh, mi, ss;
  if (sscanf(d, "%*[^,], %d %3s %d %d:%d:%d", &dd, m, &yy, &hh, &mi, &ss) != 6 || yy < 2024 || yy > 2100) return;
  const char* p = strstr(MON, m);
  if (!p || (p - MON) % 3) return;
  struct timeval tv = {(time_t)(jdn(yy, (p - MON) / 3 + 1, dd) - jdn(1970, 1, 1)) * 86400 + hh * 3600 + mi * 60 + ss, 0};
  settimeofday(&tv, nullptr);
  Serial.println("[clock] set from a web reply (no NTP yet)");
}

static bool httpGet(const char* url, String& out) {
  bool tls = strncmp(url, "https", 5) == 0;
  WiFiClientSecure secure;
  WiFiClient plain;
  if (tls) {
    secure.setInsecure();                   // hobby device: skip cert checks
    secure.setHandshakeTimeout(10);         // seconds (the default 120 s would stall everything)
  }
  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  if (!http.begin(tls ? (WiFiClient&)secure : plain, url)) return false;
  http.addHeader("User-Agent", "Momowo-ESP32");
  static const char* DATE_HDR[] = {"Date"};
  http.collectHeaders(DATE_HDR, 1);
  int code = http.GET();
  if (code > 0 && time(nullptr) < 1700000000) timeFromHttpDate(http.header("Date").c_str());
  bool ok = (code == HTTP_CODE_OK);
  if (ok) out = http.getString();
  http.end();
  return ok;
}

static bool sLocMoved = false;              // netTask only

static void storeLocation(float lat, float lon, const char* city) {
  xSemaphoreTake(gMutex, portMAX_DELAY);
  if (!gLoc.valid || fabsf(gLoc.lat - lat) > 0.05f || fabsf(gLoc.lon - lon) > 0.05f) sLocMoved = true;
  gLoc.valid = true;
  gLoc.lat = lat;
  gLoc.lon = lon;
  char c[40];
  asciiize(city, c, sizeof(c));
  if (strcmp(c, gLoc.city)) {               // a new place: show its English name until translated
    strlcpy(gLoc.city, c, sizeof(gLoc.city));
    strlcpy(gLoc.name, c, sizeof(gLoc.name));
  }
  xSemaphoreGive(gMutex);
  if (isnan(gSavedLat) || fabsf(gSavedLat - lat) > 0.05f) { gSavedLat = lat; prefs.putFloat("lat", lat); }
  if (isnan(gSavedLon) || fabsf(gSavedLon - lon) > 0.05f) { gSavedLon = lon; prefs.putFloat("lon", lon); }
}

// Does the font have every character of s? Own font state: netTask only.
static bool fontHasAll(const char* s, const uint8_t* font) {
  static u8g2_font_t chk = {};
  u8g2_SetFont(&chk, font);
  const uint8_t* p = (const uint8_t*)s;
  for (uint32_t cp; (cp = utf8Next(p));) {
    if (cp < 0x80) continue;
    if ((cp & 0x80000000u) || cp > 0xFFFF || !u8g2_IsGlyph(&chk, (uint16_t)cp)) return false;
  }
  return true;
}

// The place's name in the chosen language (Open-Meteo / GeoNames), for the screen. Picks the match
// nearest to our coordinates; keeps English if there's none, or if the font can't draw it all.
static void localizeCity() {
  static const char* const GEO_LANG[L_COUNT] = {"en", "zh", "zh", "id", "de", "fr", "ja", "es"};
  static char doneFor[40] = "";             // finished for this place (the language only changes on restart)
  static uint32_t retryAt = 0;              // after a temporary failure, wait before trying again
  char city[40];
  float lat, lon;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  strlcpy(city, gLoc.city, sizeof(city));
  lat = gLoc.lat;
  lon = gLoc.lon;
  bool valid = gLoc.valid;
  xSemaphoreGive(gMutex);
  if (!valid || !city[0] || !strcmp(doneFor, city)) return;
  if (retryAt && (int32_t)(millis() - retryAt) < 0) return;
  if (LANG == L_EN) { strlcpy(doneFor, city, sizeof(doneFor)); return; }

  auto setName = [&](const char* name) {
    xSemaphoreTake(gMutex, portMAX_DELAY);
    if (!strcmp(gLoc.city, city)) utf8Copy(gLoc.name, name, sizeof(gLoc.name));
    xSemaphoreGive(gMutex);
  };
  // usable = something to show; for Chinese/Japanese it must really be in that script, all drawable
  auto usable = [&](const char* n) {
    if (!n[0] || isRefusal(n) || strchr(n, '\n')) return false;
    return !langIsCjk() || (hasCjk(n) && fontHasAll(n, cjkFont()));
  };

  // 1. Saved from an earlier start-up: no network needed
  char key[48], saved[64] = "", savedFor[48] = "";
  snprintf(key, sizeof(key), "%s|%d", city, LANG);
  prefs.getString("locfor", savedFor, sizeof(savedFor));
  prefs.getString("locname", saved, sizeof(saved));
  if (!strcmp(savedFor, key) && usable(saved)) {
    setName(saved);
    strlcpy(doneFor, city, sizeof(doneFor));
    return;
  }

  // 2. Open-Meteo (GeoNames): the match nearest our coordinates
  char q[96], url[240], best[64] = "";
  size_t o = 0;
  for (const uint8_t* c = (const uint8_t*)city; *c && o + 4 < sizeof(q); c++) {
    if (isalnum(*c) || *c == '-' || *c == '.' || *c == '_') q[o++] = (char)*c;
    else o += snprintf(q + o, sizeof(q) - o, "%%%02X", *c);         // URL-encode
  }
  q[o] = 0;
  snprintf(url, sizeof(url), "https://geocoding-api.open-meteo.com/v1/search?name=%s&count=10&language=%s",
           q, GEO_LANG[LANG]);
  String body;
  JsonDocument doc;
  bool netOk = httpGet(url, body) && !deserializeJson(doc, body);
  if (netOk) {
    float bestD = 1.0f;                     // within about 100 km, so a namesake elsewhere won't match
    for (JsonObject r : doc["results"].as<JsonArray>()) {
      float d = fabsf((r["latitude"] | 999.0f) - lat) + fabsf((r["longitude"] | 999.0f) - lon);
      const char* n = r["name"] | "";
      if (d < bestD && n[0]) { bestD = d; utf8Copy(best, n, sizeof(best)); }
    }
  }
  bool good = usable(best);
  if (!good && best[0]) Serial.printf("[geo] \"%s\" isn't a %s name\n", best, LANG_NAME[LANG]);

  // 3. Chinese/Japanese and still nothing (common for suburbs): ask Claude once, if AI is on
  if (!good && langIsCjk() && AI_API_KEY[0]) {
    char prompt[300], ai[96];
    snprintf(prompt, sizeof(prompt),
             "What do %s speakers call the place \"%s\" (latitude %.2f, longitude %.2f)? If it has no "
             "established name, write it the way they naturally would (katakana for Japanese, a "
             "transliteration for Chinese). Reply with the name only.", LANG_NAME[LANG], city, lat, lon);
    if (!askAI(prompt, ai, sizeof(ai), 40, false)) {   // asleep, offline or busy: try later
      retryAt = millis() + 30UL * 60 * 1000;
      if (!retryAt) retryAt = 1;
      return;
    }
    if (usable(ai)) { utf8Copy(best, ai, sizeof(best)); good = true; }
  }
  if (!netOk && !good) {                    // couldn't reach Open-Meteo: try again later
    retryAt = millis() + 10UL * 60 * 1000;
    if (!retryAt) retryAt = 1;
    return;
  }
  retryAt = 0;
  strlcpy(doneFor, city, sizeof(doneFor));
  if (!good) { Serial.printf("[geo] no %s name for %s, keeping English\n", LANG_NAME[LANG], city); return; }
  setName(best);
  prefs.putString("locfor", key);           // remembered, so restarts need no lookup
  prefs.putString("locname", best);
  Serial.printf("[geo] %s is \"%s\" in %s\n", city, best, LANG_NAME[LANG]);
}

// Suburb name or postcode -> coordinates (Open-Meteo geocoding, no key)
static bool geocodeSuburb() {
  char q[3 * sizeof(SUBURB) + 1], url[320];
  size_t o = 0;
  for (const uint8_t* p = (const uint8_t*)SUBURB; *p && o + 4 < sizeof(q); p++) {
    if (isalnum(*p) || *p == '-' || *p == '.' || *p == '_') q[o++] = (char)*p;
    else o += snprintf(q + o, sizeof(q) - o, "%%%02X", *p);         // URL-encode
  }
  q[o] = 0;
  snprintf(url, sizeof(url), "https://geocoding-api.open-meteo.com/v1/search?name=%s&count=1%s%s",
           q, COUNTRY_CODE[0] ? "&countryCode=" : "", COUNTRY_CODE);
  String body;
  JsonDocument doc;
  if (!httpGet(url, body) || deserializeJson(doc, body)) {
    Serial.println("[geo] suburb lookup failed");
    return false;
  }
  JsonObject r = doc["results"][0];
  if (r.isNull()) {
    Serial.printf("[geo] no place found for \"%s\"\n", SUBURB);
    return false;
  }
  float lat = r["latitude"] | 0.0f, lon = r["longitude"] | 0.0f;
  storeLocation(lat, lon, r["name"] | (const char*)SUBURB);
  Serial.printf("[geo] %s -> %s, %s (%.4f, %.4f)\n", SUBURB, r["name"] | "", r["admin1"] | "", lat, lon);
  return true;
}

static char gCountry[48] = "";              // country name from the IP lookup (netTask only), for AI

// Where am I? ip-api.com first (free tier is plain HTTP), ipwho.is as backup
static bool geolocate() {
  String body;
  JsonDocument doc;
  if (httpGet("http://ip-api.com/json/?fields=status,country,countryCode,city,lat,lon,offset", body) &&
      !deserializeJson(doc, body) && strcmp(doc["status"] | "", "success") == 0) {
    storeLocation(doc["lat"] | 0.0f, doc["lon"] | 0.0f, doc["city"] | "");
    asciiize(doc["country"] | "", gCountry, sizeof(gCountry));
    strlcpy(gCountryCode, doc["countryCode"] | "", sizeof(gCountryCode));
    if (prefs.getString("ipcc", "") != gCountryCode) prefs.putString("ipcc", gCountryCode);   // known at boot
    if (!doc["offset"].isNull()) netTzOffset(doc["offset"].as<int32_t>());
    return true;
  }
  body = "";
  if (httpGet("https://ipwho.is/?fields=success,country,country_code,city,latitude,longitude,timezone", body) &&
      !deserializeJson(doc, body) && (doc["success"] | false)) {
    storeLocation(doc["latitude"] | 0.0f, doc["longitude"] | 0.0f, doc["city"] | "");
    asciiize(doc["country"] | "", gCountry, sizeof(gCountry));
    strlcpy(gCountryCode, doc["country_code"] | "", sizeof(gCountryCode));
    if (prefs.getString("ipcc", "") != gCountryCode) prefs.putString("ipcc", gCountryCode);
    if (!doc["timezone"]["offset"].isNull()) netTzOffset(doc["timezone"]["offset"].as<int32_t>());
    return true;
  }
  return false;
}

static bool fetchWeather() {
  LocationData loc = getLocation();
  if (!loc.valid) return false;
  char url[440];
  snprintf(url, sizeof(url),
    "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
    "&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,weather_code,wind_speed_10m"
    "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max,"
    "wind_speed_10m_max,sunrise,sunset&timezone=auto&forecast_days=5", loc.lat, loc.lon);

  String body;
  if (!httpGet(url, body)) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;
  JsonObject cur = doc["current"];
  if (cur.isNull()) return false;

  // Open-Meteo reports the local UTC offset (DST-aware): keeps the clock right
  if (!doc["utc_offset_seconds"].isNull()) netTzOffset(doc["utc_offset_seconds"].as<int32_t>());

  WeatherData w;
  w.valid      = true;
  w.temp       = cur["temperature_2m"] | 0.0f;
  w.feels      = cur["apparent_temperature"] | w.temp;
  w.humidity   = cur["relative_humidity_2m"] | 0;
  w.code       = cur["weather_code"] | 0;
  w.wind       = cur["wind_speed_10m"] | 0.0f;
  w.isDay      = (cur["is_day"] | 1) == 1;
  w.tmax       = doc["daily"]["temperature_2m_max"][0] | w.temp;
  w.tmin       = doc["daily"]["temperature_2m_min"][0] | w.temp;
  w.rainChance = doc["daily"]["precipitation_probability_max"][0] | 0;
  auto hhmm = [](const char* iso) {                     // "YYYY-MM-DDTHH:MM" -> minutes
    return strlen(iso) >= 16 ? atoi(iso + 11) * 60 + atoi(iso + 14) : -1;
  };
  w.sunriseMin = hhmm(doc["daily"]["sunrise"][0] | "");
  w.sunsetMin  = hhmm(doc["daily"]["sunset"][0] | "");
  w.tmSunrise  = hhmm(doc["daily"]["sunrise"][1] | "");
  w.tmSunset   = hhmm(doc["daily"]["sunset"][1] | "");
  JsonObject daily = doc["daily"];                        // index 1 = tomorrow
  w.tmValid = !daily["temperature_2m_max"][1].isNull();
  w.tmMax   = daily["temperature_2m_max"][1] | 0.0f;
  w.tmMin   = daily["temperature_2m_min"][1] | 0.0f;
  w.tmCode  = daily["weather_code"][1] | 0;
  w.tmRain  = daily["precipitation_probability_max"][1] | 0;
  w.tmWind  = daily["wind_speed_10m_max"][1] | 0.0f;
  for (int i = 0; i < 5; i++) {                           // today and the next four days
    if (daily["temperature_2m_max"][i].isNull()) break;
    w.dMax[i]  = (int8_t)lroundf(daily["temperature_2m_max"][i] | 0.0f);
    w.dMin[i]  = (int8_t)lroundf(daily["temperature_2m_min"][i] | 0.0f);
    w.dCode[i] = daily["weather_code"][i] | 0;
    w.dRain[i] = daily["precipitation_probability_max"][i] | 0;
    w.days = i + 1;
  }
  // Air quality (Open-Meteo, from the Copernicus CAMS model); the weather is shown without it too
  snprintf(url, sizeof(url), "https://air-quality-api.open-meteo.com/v1/air-quality?latitude=%.4f"
           "&longitude=%.4f&current=us_aqi,pm2_5,pm10", loc.lat, loc.lon);
  JsonDocument air;
  if (httpGet(url, body) && !deserializeJson(air, body) && !air["current"]["us_aqi"].isNull()) {
    w.aqi  = air["current"]["us_aqi"].as<int>();
    w.pm25 = air["current"]["pm2_5"] | -1.0f;
    w.pm10 = air["current"]["pm10"] | -1.0f;
  }

  w.at = millis();
  xSemaphoreTake(gMutex, portMAX_DELAY);
  gWeather = w;
  xSemaphoreGive(gMutex);
  return true;
}

static void formatSource(char* out, size_t n, const char* who, const char* from) {
  bool hasWho = who && *who, hasFrom = from && *from;
  // "──" is U+2500: the GB2312 font has no glyph for the em dash U+2014
  if (hasWho && hasFrom && strcmp(who, from) != 0) snprintf(out, n, "── %s《%s》", who, from);
  else if (hasFrom) snprintf(out, n, "──《%s》", from);
  else if (hasWho)  snprintf(out, n, "── %s", who);
  else out[0] = 0;
}

// Philosophy-category quotes are only kept if they come from a classical thinker or work
static bool isClassicalSource(const char* who, const char* from) {
  static const char* const CLASSICAL[] = {
    "老子", "孔子", "庄子", "孟子", "荀子", "墨子", "韩非", "孙子", "列子", "管子",
    "论语", "道德经", "礼记", "诗经", "周易", "易经", "中庸", "尚书", "左传", "史记",
    "战国策", "淮南子", "鬼谷子", "菜根谭", "增广贤文"};
  for (const char* c : CLASSICAL)
    if (strstr(who, c) || strstr(from, c)) return true;
  return false;
}

// Swap punctuation the GB2312 font can't draw for close equivalents it can:
// em/en dash and horizontal bar -> "─" (U+2500); middle dots -> space.
static void normalizeCJK(const char* in, char* out, size_t outSz) {
  const uint8_t* p = (const uint8_t*)in;
  size_t o = 0;
  while (*p && o + 4 < outSz) {
    if (*p == '\n' || *p == '\r') { p++; continue; }
    if (p[0] == 0xE2 && p[1] == 0x80 && (p[2] == 0x93 || p[2] == 0x94 || p[2] == 0x95)) {
      out[o++] = (char)0xE2; out[o++] = (char)0x94; out[o++] = (char)0x80;
      p += 3;
    } else if ((p[0] == 0xC2 && p[1] == 0xB7) || (p[0] == 0xE2 && p[1] == 0x80 && p[2] == 0xA2) ||
               (p[0] == 0xE3 && p[1] == 0x83 && p[2] == 0xBB)) {
      out[o++] = ' ';
      p += (p[0] == 0xC2) ? 2 : 3;
    } else {
      out[o++] = (char)*p++;
    }
  }
  out[o] = 0;
}

// True if `font` has a glyph for every character of s. Uses its own font
// state, so it is safe to call from core 0 while core 1 draws.
static bool fontCovers(const char* s, const uint8_t* font) {
  static u8g2_font_t chk = {};
  u8g2_SetFont(&chk, font);
  const uint8_t* p = (const uint8_t*)s;
  while (*p) {
    uint32_t cp;
    int n;
    if (*p < 0x80) { cp = *p; n = 1; }
    else if ((*p & 0xE0) == 0xC0 && p[1]) { cp = ((*p & 0x1Fu) << 6) | (p[1] & 0x3Fu); n = 2; }
    else if ((*p & 0xF0) == 0xE0 && p[1] && p[2]) {
      cp = ((*p & 0x0Fu) << 12) | ((p[1] & 0x3Fu) << 6) | (p[2] & 0x3Fu);
      n = 3;
    } else return false;                    // outside the fonts' range, or malformed
    if (!u8g2_IsGlyph(&chk, (uint16_t)cp)) return false;
    p += n;
  }
  return true;
}

static bool quoteDrawable(const char* q, const char* src) {
  return fontCovers(q, CJK_FONT_BIG) && fontCovers(src, CJK_FONT);
}

// One short line from a 今日诗词 category (classical poetry), retrying for length
static bool fetchJinrishici(const char* path, const char* tag, int tries) {
  char url[80], src[96];
  snprintf(url, sizeof(url), "https://v1.jinrishici.com/%s.json", path);
  String body;
  JsonDocument doc;
  for (int i = 0; i < tries; i++) {
    if (i) vTaskDelay(pdMS_TO_TICKS(400));
    body = "";
    if (!httpGet(url, body) || deserializeJson(doc, body)) {
      Serial.printf("[quote] %s: request failed\n", path);
      return false;
    }
    char q[256], raw[96];
    normalizeCJK(doc["content"] | "", q, sizeof(q));
    if (!*q || utf8Count(q) > QUOTE_MAX_CHARS) continue;          // too long: try again
    formatSource(raw, sizeof(raw), doc["author"] | "", doc["origin"] | "");
    normalizeCJK(raw, src, sizeof(src));
    if (!quoteDrawable(q, src)) {
      Serial.printf("[quote] %s: skipped, font lacks a character: %s\n", path, q);
      continue;
    }
    Serial.printf("[quote] %s: %s %s\n", path, q, src);
    setQuote(q, src, tag);
    return true;
  }
  Serial.printf("[quote] %s: no short line in %d tries\n", path, tries);
  return false;
}

// Hitokoto: classical poetry, or philosophy only from classical thinkers
static bool fetchHitokotoClassical() {
  char url[96], src[96];
  snprintf(url, sizeof(url), "https://v1.hitokoto.cn/?c=i&c=k&encode=json&max_length=%d", QUOTE_MAX_CHARS);
  String body;
  JsonDocument doc;
  for (int tries = 0; tries < 5; tries++) {
    if (tries) vTaskDelay(pdMS_TO_TICKS(400));
    body = "";
    if (!httpGet(url, body) || deserializeJson(doc, body)) {
      Serial.println("[quote] hitokoto: request failed");
      return false;
    }
    const char* type = doc["type"] | "";
    const char* who  = doc["from_who"] | "";
    const char* from = doc["from"] | "";
    char q[256], raw[96];
    normalizeCJK(doc["hitokoto"] | "", q, sizeof(q));
    if (!*q || utf8Count(q) > QUOTE_MAX_CHARS) continue;
    if (strcmp(type, "k") == 0 && !isClassicalSource(who, from)) continue;   // modern philosophy
    formatSource(raw, sizeof(raw), who, from);
    normalizeCJK(raw, src, sizeof(src));
    if (!quoteDrawable(q, src)) {
      Serial.printf("[quote] hitokoto: skipped, font lacks a character: %s\n", q);
      continue;
    }
    Serial.printf("[quote] hitokoto: %s %s\n", q, src);
    setQuote(q, src, "");
    return true;
  }
  return false;
}

// Themed quote first; if that category has no short line, any classical quote
// Built-in classical Chinese quotes (all public domain), used when offline.
// Each: theme, text (max 12 characters), author, work, English meaning.
// Checked: every character is in the display font; source lines fall back to
// title-only or author-only where a name uses a rarer character.
static const LocalQuote LOCAL_QUOTES[] = {
  {T_MOON, "床前明月光，疑是地上霜。", "李白", "静夜思", "Moonlight before my bed: I took it for frost on the ground."},
  {T_MOON, "举头望明月，低头思故乡。", "李白", "静夜思", "I raise my head to the bright moon, then lower it, thinking of home."},
  {T_MOON, "海上生明月，天涯共此时。", "张九龄", "望月怀远", "The bright moon rises over the sea; far apart, we share this moment."},
  {T_MOON, "明月几时有？把酒问青天。", "", "水调歌头", "When did the bright moon first appear? Cup in hand, I ask the sky."},
  {T_MOON, "露从今夜白，月是故乡明。", "杜甫", "月夜忆舍弟", "From tonight the dew turns white; the moon shines brightest over home."},
  {T_MOON, "月落乌啼霜满天", "张继", "枫桥夜泊", "The moon sets, crows call, and frost fills the sky."},
  {T_STARS, "危楼高百尺，手可摘星辰。", "李白", "夜宿山寺", "The tower soars so high, I could pluck the stars by hand."},
  {T_STARS, "星垂平野阔，月涌大江流。", "杜甫", "旅夜书怀", "Stars hang low over the wide plain; the moon surges with the great river."},
  {T_STARS, "卧看牵牛织女星", "杜牧", "秋夕", "Lying back, I watch the Herd Boy and Weaver Girl stars."},
  {T_SUN, "白日依山尽，黄河入海流。", "王之涣", "", "The sun sinks behind the hills; the Yellow River flows into the sea."},
  {T_SUN, "欲穷千里目，更上一层楼。", "王之涣", "", "To see a thousand miles further, climb one more floor."},
  {T_SUN, "大漠孤烟直，长河落日圆。", "王维", "使至塞上", "Lone smoke rises straight over the desert; the sun sets round over the river."},
  {T_SUN, "夕阳无限好，只是近黄昏。", "李商隐", "登乐游原", "The setting sun is endlessly beautiful, though dusk draws near."},
  {T_SUN, "日出江花红胜火", "白居易", "忆江南", "At sunrise, the river flowers glow redder than fire."},
  {T_RAIN, "好雨知时节，当春乃发生。", "杜甫", "春夜喜雨", "Good rain knows its season, and comes just as spring arrives."},
  {T_RAIN, "随风潜入夜，润物细无声。", "杜甫", "春夜喜雨", "It slips into the night on the wind, quietly nourishing everything."},
  {T_RAIN, "夜来风雨声，花落知多少。", "孟浩然", "春晓", "Wind and rain in the night: how many blossoms fell?"},
  {T_RAIN, "山色空蒙雨亦奇", "", "饮湖上初晴后雨", "Misty hills are lovely in the rain too."},
  {T_RAIN, "清明时节雨纷纷", "杜牧", "清明", "At Qingming the rain falls soft and steady."},
  {T_SNOW, "千山鸟飞绝，万径人踪灭。", "柳宗元", "江雪", "No birds over a thousand hills, no footprints on ten thousand paths."},
  {T_SNOW, "千树万树梨花开", "", "白雪歌送武判官归京", "Ten thousand trees burst into pear blossom: the snow has come."},
  {T_SNOW, "柴门闻犬吠，风雪夜归人。", "刘长卿", "", "A dog barks at the gate: someone is coming home through the snowy night."},
  {T_CLOUD, "行到水穷处，坐看云起时。", "王维", "终南别业", "Walk to where the stream ends, then sit and watch the clouds rise."},
  {T_CLOUD, "众鸟高飞尽，孤云独去闲。", "李白", "独坐敬亭山", "The birds have flown away; a lone cloud drifts off at ease."},
  {T_CLOUD, "白云千载空悠悠", "", "黄鹤楼", "White clouds have drifted idly for a thousand years."},
  {T_WIND, "长风破浪会有时", "李白", "行路难", "A day will come to ride the wind and cut through the waves."},
  {T_WIND, "野火烧不尽，春风吹又生。", "白居易", "赋得古原草送别", "Wildfire cannot burn it all; the spring wind brings it back to life."},
  {T_WIND, "二月春风似剪刀", "贺知章", "咏柳", "The spring breeze of the second month is like scissors, shaping the willow."},
  {T_SPRING, "春眠不觉晓，处处闻啼鸟。", "孟浩然", "春晓", "Spring sleep, unaware of dawn; birdsong everywhere."},
  {T_SPRING, "万紫千红总是春", "", "春日", "Countless colours everywhere: it is spring."},
  {T_SPRING, "春色满园关不住", "叶绍翁", "游园不值", "A garden full of spring cannot be shut in."},
  {T_SPRING, "春风又绿江南岸", "王安石", "泊船瓜洲", "The spring wind turns the south bank green again."},
  {T_SUMMER, "小荷才露尖尖角", "杨万里", "小池", "A young lotus has just shown its pointed tip."},
  {T_SUMMER, "接天莲叶无穷碧", "杨万里", "晓出净慈寺送林子方", "Lotus leaves stretch to the sky in endless green."},
  {T_SUMMER, "绿树阴浓夏日长", "", "山亭夏日", "Green trees cast deep shade; summer days are long."},
  {T_AUTUMN, "停车坐爱枫林晚", "杜牧", "山行", "I stop the carriage, loving the maple woods at dusk."},
  {T_AUTUMN, "霜叶红于二月花", "杜牧", "山行", "Frosted leaves are redder than spring flowers."},
  {T_AUTUMN, "我言秋日胜春朝", "刘禹锡", "秋词", "I say an autumn day is finer than a spring morning."},
  {T_AUTUMN, "空山新雨后，天气晚来秋。", "王维", "", "After fresh rain in the empty hills, evening brings the feel of autumn."},
  {T_AUTUMN, "采菊东篱下，悠然见南山。", "陶渊明", "饮酒", "Picking chrysanthemums by the eastern fence, I calmly see the southern hills."},
  {T_WINTER, "墙角数枝梅，凌寒独自开。", "王安石", "梅花", "A few plum branches in the corner bloom alone against the cold."},
  {T_WINTER, "晚来天欲雪，能饮一杯无？", "白居易", "问刘十九", "Evening, and snow is coming. Will you share a cup with me?"},
  {T_WINTER, "梅须逊雪三分白", "卢梅坡", "雪梅", "The plum blossom must yield a little to the snow in whiteness."},
  {T_NONE, "千里之行，始于足下。", "老子", "道德经", "A journey of a thousand miles begins beneath your feet."},
  {T_NONE, "上善若水。", "老子", "道德经", "The highest good is like water."},
  {T_NONE, "知足者富。", "老子", "道德经", "Whoever knows contentment is rich."},
  {T_NONE, "学而时习之，不亦说乎？", "孔子", "论语", "To learn and practise what you learn: is that not a joy?"},
  {T_NONE, "有朋自远方来，不亦乐乎？", "孔子", "论语", "A friend comes from afar: is that not a delight?"},
  {T_NONE, "三人行，必有我师焉。", "孔子", "论语", "Walking with any two others, I will find a teacher among them."},
  {T_NONE, "己所不欲，勿施于人。", "孔子", "论语", "Do not do to others what you would not want for yourself."},
  {T_NONE, "天行健，君子以自强不息", "", "周易", "As heaven moves on without rest, keep growing stronger."},
  {T_NONE, "海内存知己，天涯若比邻。", "王勃", "送杜少府之任蜀州", "With a true friend in the world, the far ends of the earth feel next door."},
  {T_NONE, "天生我材必有用", "李白", "将进酒", "Heaven gave me my talents for a reason."},
  {T_NONE, "莫愁前路无知己", "高适", "别董大", "Don't worry: friends are waiting on the road ahead."},
  {T_NONE, "柳暗花明又一村", "陆游", "游山西村", "Past the shady willows and bright flowers, another village appears."},
  {T_NONE, "心远地自偏", "陶渊明", "饮酒", "When the heart is at peace, any place is quiet."},
  {T_NONE, "宁静致远", "诸葛亮", "诫子书", "Calm lets you reach far."},
};
static const int LOCAL_QUOTE_COUNT = sizeof(LOCAL_QUOTES) / sizeof(LOCAL_QUOTES[0]);

// A built-in quote for the theme (or a general one), with its English meaning
static bool pickLocalQuote(QTheme th) {
  static int last = -1;
  int idx[LOCAL_QUOTE_COUNT], n = 0;
  for (int i = 0; i < LOCAL_QUOTE_COUNT; i++)
    if (LOCAL_QUOTES[i].theme == th && i != last) idx[n++] = i;
  if (!n)
    for (int i = 0; i < LOCAL_QUOTE_COUNT; i++)
      if (LOCAL_QUOTES[i].theme == T_NONE && i != last) idx[n++] = i;
  if (!n) return false;
  last = idx[esp_random() % n];
  const LocalQuote& q = LOCAL_QUOTES[last];
  char src[96];
  formatSource(src, sizeof(src), q.who, q.from);
  setQuote(q.text, src, (q.theme != T_NONE) ? THEME_TAG[q.theme] : "");
  xSemaphoreTake(gMutex, portMAX_DELAY);
  strlcpy(gQuote.meaning, q.meaning, sizeof(gQuote.meaning));
  gQuote.version++;
  xSemaphoreGive(gMutex);
  return true;
}

// The verses of one scripture source
static const Verse* verseList(QKind kind, int& count) {
  switch (kind) {
    case Q_BIBLE: count = COUNT_OF(BIBLE_VERSES); return BIBLE_VERSES;
    case Q_QURAN: count = COUNT_OF(QURAN_VERSES); return QURAN_VERSES;
    case Q_TORAH: count = COUNT_OF(TORAH_VERSES); return TORAH_VERSES;
    case Q_DHAMMA: count = COUNT_OF(DHAMMAPADA_VERSES); return DHAMMAPADA_VERSES;
    default: count = 0; return nullptr;
  }
}

// The built-in English line for a Greek or scripture quote, found by its original text (nullptr: none)
static const char* englishFor(QKind kind, const char* text) {
  if (kind == Q_GREEK) {
    for (auto& q : GREEK_QUOTES) if (!strcmp(q.text, text)) return q.english;
    return nullptr;
  }
  int count;
  const Verse* list = verseList(kind, count);
  for (int i = 0; i < count; i++) if (!strcmp(list[i].text, text)) return list[i].english;
  return nullptr;
}

// Is the quote already in Momowo's language? Then the AI explains it instead of translating it.
static bool quoteInMyLanguage(QKind kind) {
  if (kind == Q_CHINESE) return LANG == L_ZHS || LANG == L_ZHT;
  if (kind == Q_MODERN || kind == Q_BIBLE) return LANG == L_EN;   // English quotes
  if (kind >= Q_FAMOUS) return gQuote.lang == LANG;   // whatever language the AI found it in
  return false;                             // Greek, Hebrew, Arabic, Pali
}

// Until the AI has translated it (or with no AI), the English line is the translation
static void englishFallback(QKind kind, const char* text, char* meaning, size_t n) {
  const char* en = englishFor(kind, text);
  if (en && (QUOTE_AI == 0 || !AI_API_KEY[0])) utf8Copy(meaning, en, n);
}

// A Greek, modern or scripture quote in its original language, with its source
static void setWorldQuote(QKind kind, const char* text, const char* who) {
  xSemaphoreTake(gMutex, portMAX_DELAY);
  gQuote.kind = kind;
  utf8Copy(gQuote.text, text, sizeof(gQuote.text));
  snprintf(gQuote.author, sizeof(gQuote.author), "- %s", who);
  gQuote.tag[0] = 0;
  gQuote.meaning[0] = 0;
  gQuote.lang = -1;
  englishFallback(kind, gQuote.text, gQuote.meaning, sizeof(gQuote.meaning));
  gQuote.version++;
  xSemaphoreGive(gMutex);
}

static_assert(T_STARS == 10 && QM_STARS == 1 << T_STARS, "quote moods follow the QTheme order");

// A random quote that suits the theme (any quote when none does, or with no theme), not the last one
static int pickSuited(const uint16_t* moods, int count, QTheme th, int last) {
  int idx[64], n = 0;
  for (int i = 0; i < count && i < 64; i++)
    if (th != T_NONE && (moods[i] >> th & 1) && i != last) idx[n++] = i;
  if (!n)
    for (int i = 0; i < count && i < 64; i++)
      if (i != last || count == 1) idx[n++] = i;
  return idx[esp_random() % n];
}

static bool pickGreekQuote(QTheme th) {
  static int last = -1;
  uint16_t moods[GREEK_COUNT];
  for (int i = 0; i < GREEK_COUNT; i++) moods[i] = GREEK_QUOTES[i].moods;
  int i = last = pickSuited(moods, GREEK_COUNT, th, last);
  const WorldQuote& q = GREEK_QUOTES[i];
  setWorldQuote(Q_GREEK, q.text, greekWhoEnglish(q.who));
  return true;
}

// Modern: a built-in quote that suits the moment; ZenQuotes (free, credited in the guide) when
// nothing in particular applies, built-in ones offline
static bool pickModernQuote(QTheme th) {
  if (th == T_NONE && gWifiUp) {
    String body;
    JsonDocument doc;
    if (httpGet("https://zenquotes.io/api/random", body) && !deserializeJson(doc, body)) {
      const char* q = doc[0]["q"] | "";
      const char* a = doc[0]["a"] | "";
      if (q[0] && strlen(q) < 170 && strncmp(q, "Too many requests", 17)) {
        setWorldQuote(Q_MODERN, q, a[0] ? a : "Unknown");
        return true;
      }
    }
  }
  static int last = -1;
  uint16_t moods[MODERN_COUNT];
  for (int i = 0; i < MODERN_COUNT; i++) moods[i] = MODERN_QUOTES[i].moods;
  int i = last = pickSuited(moods, MODERN_COUNT, th, last);
  setWorldQuote(Q_MODERN, MODERN_QUOTES[i].text, MODERN_QUOTES[i].who);
  return true;
}

// Online: a web quote for the theme; offline (or if the web doesn't answer): built-in.
// Greek and modern quotes follow the owner's choice on the phone page ("a mix" takes turns).
static bool pickVerse(QKind kind, QTheme th) {
  int count;
  const Verse* list = verseList(kind, count);
  static int last[Q_KINDS] = {-1, -1, -1, -1, -1, -1, -1, -1};
  uint16_t moods[64];
  for (int i = 0; i < count && i < 64; i++) moods[i] = list[i].moods;
  int i = last[kind] = pickSuited(moods, count, th, last[kind]);
  setWorldQuote(kind, list[i].text, list[i].ref);
  return true;
}

// Famous people: Claude picks a real quote by one of the people the owner named, to suit
// the moment, with its translation (or explanation) in the same reply. Needs a key.
static const char* const THEME_EN[] = {"spring", "summer", "autumn", "winter", "a rainy day", "a snowy day",
  "a cloudy day", "a windy day", "a sunny day", "a moonlit night", "a starry night", "any moment"};
static const char* const LANG_CODE[L_COUNT] = {"en", "zh", "zh", "id", "de", "fr", "ja", "es"};

// People of serious public concern (the owner's request): never quoted, and never the voice of the
// news. Common spellings in Momowo's languages; the AI judges anyone not listed.
static const char* const CONCERN_NAMES[] = {
  "hitler", "希特勒", "ヒトラー", "bin laden", "本拉登", "本·拉登", "賓拉登", "賓·拉登", "ビンラディン",
  "ビン・ラディン", "pol pot", "波尔布特", "波爾布特", "ポル・ポト", "himmler", "goebbels", "eichmann",
  "mengele", "baghdadi", "zawahiri", "charles manson", "ted bundy", "breivik", "mcveigh", "asahara",
  "麻原彰晃", "idi amin", "noordin", "imam samudra", "amrozi"};
static const char* const CONCERN_RULE =
  "a real person of serious public concern: a terrorist; an architect or perpetrator of genocide, war crimes, "
  "mass murder or other atrocities; a dictator known chiefly for such crimes; a notorious violent criminal such "
  "as a serial killer or mass shooter; or a leader of a violent hate group. Characters from films, TV, books or "
  "games are fine, villains included, unless the character portrays such a real person";

static bool concernByName(const char* s) {
  char low[128];
  size_t i = 0;
  for (; s[i] && i + 1 < sizeof(low); i++) low[i] = (s[i] >= 'A' && s[i] <= 'Z') ? s[i] + 32 : s[i];
  low[i] = 0;
  for (const char* n : CONCERN_NAMES) if (strstr(low, n)) return true;
  return false;
}

static bool screenCanShow(const char* s) {  // only scripts Momowo has letters for
  const uint8_t* p = (const uint8_t*)s;
  while (*p) {
    uint32_t c = utf8Next(p);
    if (c < 0x250 || (c >= 0x370 && c <= 0x3FF) || (c >= 0x590 && c <= 0x6FF) || (c >= 0x1E00 && c <= 0x1EFF) ||
        (c >= 0x2000 && c <= 0x206F) || (c >= 0x3000 && c <= 0x30FF) || (c >= 0x4E00 && c <= 0x9FFF) ||
        (c >= 0xFF00 && c <= 0xFFEF)) continue;
    return false;
  }
  return true;
}

static bool fetchFamousQuote(QTheme th) {
  const char* names = FAMOUS_PEOPLE;
  if (!names[0] || !AI_API_KEY[0] || !gWifiUp) return false;
  static char recent[4][48];                // not the same few quotes again and again
  static uint8_t rn = 0;
  const char* lim = langIsCjk() ? (LANG == L_JA ? "60 characters" : "45 characters") : "35 words";
  String p = "Pick one real, well-documented quote by one of these people (the owner's favourites): ";
  p += names;
  p += ". Vary the person and the quote. Prefer one that suits ";
  p += THEME_EN[th < T_NONE ? th : T_NONE];
  p += ". Give it exactly as they said or wrote it, at most 30 words (60 characters in Chinese or Japanese). "
       "Never use song lyrics. Never quote ";
  p += CONCERN_RULE;
  p += ". If a name is one, quietly pick another person from the list, or a well-known inspiring figure "
       "if no one else is left.";
  p += " Use its original language if that is English, Chinese, Japanese, Indonesian, Malay, German, French, "
       "Spanish, Italian, Portuguese, Dutch, Greek, Hebrew or Arabic; otherwise use a well-known English "
       "translation. Never invent or misattribute anything: if you are not sure of one, choose another you are sure of.";
  bool any = false;
  for (auto& r : recent) {
    if (!r[0]) continue;
    p += any ? ", \"" : " Not these recent ones: \"";
    p += r;
    p += "\"";
    any = true;
  }
  if (any) p += ".";
  p += " Reply with JSON only: {\"q\":\"the ";
  p += "quote\",\"who\":\"the person's name as usually written in ";
  p += LANG_NAME[LANG];
  p += "\",";
  p += "\"lang\":\"two-letter code of its language\",\"m\":\"";
  if (QUOTE_AI) {
    p += "in "; p += LANG_NAME[LANG]; p += ", at most "; p += lim;
    p += ", what it means and how it could lift someone feeling stressed today; warm and simple, never preachy";
  } else {
    p += "if it is already in "; p += LANG_NAME[LANG]; p += ", a short explanation in that language of what it means, at most ";
    p += lim; p += "; otherwise its faithful translation into natural "; p += LANG_NAME[LANG];
  }
  p += "\"}";
  if (QUOTE_AI) p += genderNote();
  char out[900];
  if (!askAI(p.c_str(), out, sizeof(out), 400, true)) return false;
  char* a = strchr(out, '{');
  char* b = strrchr(out, '}');
  if (!a || !b || b < a) return false;
  b[1] = 0;
  JsonDocument doc;
  if (deserializeJson(doc, a)) return false;
  const char* q = doc["q"] | "";
  const char* who = doc["who"] | "";
  const char* code = doc["lang"] | "";
  const char* m = doc["m"] | "";
  if (!q[0] || strlen(q) > 240 || isRefusal(q) || !screenCanShow(q)) {
    Serial.println("[ai] quote: not usable, a built-in one instead");
    return false;
  }
  char srcFit[88];
  utf8Copy(srcFit, who[0] ? who : "?", sizeof(srcFit));
  int8_t ql = -1;
  for (int i = 0; i < L_COUNT; i++) if (!strncasecmp(code, LANG_CODE[i], 2)) { ql = i; break; }
  if (ql == L_ZHS && LANG == L_ZHT) ql = L_ZHT;   // Chinese either way
  setWorldQuote(Q_FAMOUS, q, srcFit);
  char mm[320] = "";
  if (m[0] && !isRefusal(m) && screenCanShow(m)) {
    utf8Copy(mm, m, sizeof(mm));
    clampText(mm, langIsCjk() ? 300 : 260);
  }
  xSemaphoreTake(gMutex, portMAX_DELAY);
  gQuote.lang = ql;
  strlcpy(gQuote.meaning, mm, sizeof(gQuote.meaning));
  gQuote.version++;
  xSemaphoreGive(gMutex);
  utf8Copy(recent[rn], q, sizeof(recent[0]));
  rn = (rn + 1) % 4;
  return true;
}

static bool fetchQuote(QTheme th) {
  static uint8_t turn = Q_KINDS - 1;        // take turns through the chosen sources
  uint16_t mask = QUOTE_MASK ? QUOTE_MASK : 1;
  for (int i = 0; i < Q_KINDS; i++) {
    turn = (turn + 1) % Q_KINDS;
    if (mask >> turn & 1) break;
  }
  if (turn == Q_GREEK) return pickGreekQuote(th);   // every source follows the moment's theme
  if (turn == Q_MODERN) return pickModernQuote(th);
  if (turn == Q_FAMOUS) return fetchFamousQuote(th) || pickModernQuote(th);   // no key: modern
  if (turn != Q_CHINESE) return pickVerse((QKind)turn, th);
  if (gWifiUp) {
    if (th != T_NONE && fetchJinrishici(THEME_PATH[th], THEME_TAG[th], 8)) return true;
    if (fetchHitokotoClassical()) return true;
    if (fetchJinrishici("all", "", 5)) return true;
  }
  return pickLocalQuote(th);
}

// ---- Quote context: season (hemisphere-aware), weather, time of day ----
static QTheme seasonTheme(int month, float lat) {         // month 1-12
  if (lat < 0) month = (month + 5) % 12 + 1;              // southern hemisphere: six months on
  if (month >= 3 && month <= 5)  return T_SPRING;
  if (month >= 6 && month <= 8)  return T_SUMMER;
  if (month >= 9 && month <= 11) return T_AUTUMN;
  return T_WINTER;
}

static QTheme weatherTheme(const WeatherData& w) {
  if (!w.valid) return T_NONE;
  int c = w.code;
  if ((c >= 51 && c <= 67) || (c >= 80 && c <= 82) || c >= 95) return T_RAIN;
  if ((c >= 71 && c <= 77) || c == 85 || c == 86) return T_SNOW;
  if (c == 3 || c == 45 || c == 48) return T_CLOUD;
  if (w.wind >= 30) return T_WIND;
  return T_NONE;
}

static int timeBucket(int h) {                            // 0 night, 1 dawn, 2 day, 3 dusk
  if (h >= 20 || h < 5) return 0;
  if (h < 9) return 1;
  if (h < 17) return 2;
  return 3;
}

static QTheme timeTheme(int h, const WeatherData& w) {
  bool clear = w.valid && w.code <= 2;
  switch (timeBucket(h)) {
    case 0:  return (clear && (esp_random() & 1)) ? T_STARS : T_MOON;
    case 2:  return clear ? T_SUN : T_NONE;
    default: return T_SUN;                                // sunrise, sunset
  }
}

// Weighted pick: weather 3, time of day 2, season 2 (only themes that apply)
static QTheme pickTheme(QTheme season, QTheme wx, QTheme tod) {
  uint32_t ww = wx != T_NONE ? 3 : 0, wt = tod != T_NONE ? 2 : 0, ws = season != T_NONE ? 2 : 0;
  if (ww + wt + ws == 0) return T_NONE;                   // nothing applies: any classical quote
  uint32_t r = esp_random() % (ww + wt + ws);
  if (r < ww) return wx;
  if (r < ww + wt) return tod;
  return season;
}

// Scan, then join home WiFi if it's in range, otherwise the backup network
// WiFi failure reasons in plain words (ESP-IDF wifi_err_reason_t codes)
static const char* wifiReasonText(uint8_t r) {
  switch (r) {
    case 0:   return nullptr;
    case 2: case 15: case 202: case 204:  return T(S_WR_PASS);
    case 201: case 210: case 211: case 212: return T(S_WR_NOTFOUND);   // also 5 GHz-only networks
    case 200: return T(S_WR_LOST);
    default:  return T(S_WR_CANT);
  }
}

static void onWiFiEvent(arduino_event_id_t e, arduino_event_info_t info) {
  if (e == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    uint8_t r = info.wifi_sta_disconnected.reason;
    if (r != 8) gWifiReason = r;            // 8 = we disconnected on purpose: not a failure
  } else if (e == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
    gWifiReason = 0;
  }
}

static uint8_t gHomeFails = 0;              // home WiFi seen but refused us this many times in a row
static volatile bool gPlannedHop = false;   // leaving the backup to try home: no sad sound
static bool connectWiFi() {
  WiFi.disconnect();
  xSemaphoreTake(gScanMutex, portMAX_DELAY);
  int n = WiFi.scanNetworks();
  int home = -1, hot = -1;
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    if (WIFI_SSID[0] && ssid == WIFI_SSID) {
      if (home < 0 || WiFi.RSSI(i) > WiFi.RSSI(home)) home = i;
    } else if (WIFI2_SSID[0] && ssid == WIFI2_SSID) {
      if (hot < 0 || WiFi.RSSI(i) > WiFi.RSSI(hot)) hot = i;
    }
  }
  int pick = (home >= 0 && !(hot >= 0 && gHomeFails >= 2)) ? home : hot;   // after 2 refusals: the backup
  if (pick >= 0) {
    gOnHotspot = (pick == hot);
    // channel + BSSID from the scan skip a second search and connect faster
    WiFi.begin(WiFi.SSID(pick).c_str(), gOnHotspot ? WIFI2_PASS : WIFI_PASS,
               WiFi.channel(pick), WiFi.BSSID(pick));
#if CONFIG_IDF_TARGET_ESP32C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm);   // C3 SuperMini antenna quirk: full power often fails
#endif
  }
  WiFi.scanDelete();
  xSemaphoreGive(gScanMutex);
  if (pick < 0) {                           // not seen in the scan (hidden, weak, or a missed
    gOnHotspot = false;                     // scan): try joining it by name anyway
    Serial.printf("[wifi] \"%s\" not in scan, trying directly\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
#if CONFIG_IDF_TARGET_ESP32C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
  } else {
    Serial.printf("[wifi] joining \"%s\"\n", gOnHotspot ? WIFI2_SSID : WIFI_SSID);
  }
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++)
    vTaskDelay(pdMS_TO_TICKS(250));
  gPlannedHop = false;
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[wifi] connected, IP %s\n", WiFi.localIP().toString().c_str());
    if (!gOnHotspot) gHomeFails = 0;
    return true;
  }
  if (!gOnHotspot) {                        // count real refusals only, not a network that wasn't there
    if (pick >= 0) gHomeFails = gHomeFails < 250 ? gHomeFails + 1 : 250;
  } else {
    gHomeFails = 0;                         // backup failed too: try home again next time
  }
  const char* why = wifiReasonText(gWifiReason);
  Serial.printf("[wifi] failed: %s (reason %u)\n", why ? why : "timed out", gWifiReason);
  return false;
}

static bool homeInRange() {
  if (!WIFI_SSID[0]) return false;
  xSemaphoreTake(gScanMutex, portMAX_DELAY);
  int n = WiFi.scanNetworks();
  bool found = false;
  for (int i = 0; i < n && !found; i++) found = (WiFi.SSID(i) == WIFI_SSID);
  WiFi.scanDelete();
  xSemaphoreGive(gScanMutex);
  return found;
}

// Tidy one RSS title: strip CDATA, decode entities, make it plain ASCII
static void cleanTitle(const char* in, char* out, size_t n) {
  const char* p = in;
  char body[160];
  const char* cd = strstr(p, "<![CDATA[");
  if (cd) {
    strlcpy(body, cd + 9, sizeof(body));
    char* e = strstr(body, "]]>");
    if (e) *e = 0;
    p = body;
  }
  char tmp[200];
  size_t o = 0;
  while (*p && o + 4 < sizeof(tmp)) {
    if (*p == '&') {
      if (!strncmp(p, "&amp;", 5))  { tmp[o++] = '&';  p += 5; continue; }
      if (!strncmp(p, "&lt;", 4))   { tmp[o++] = '<';  p += 4; continue; }
      if (!strncmp(p, "&gt;", 4))   { tmp[o++] = '>';  p += 4; continue; }
      if (!strncmp(p, "&quot;", 6)) { tmp[o++] = '"';  p += 6; continue; }
      if (!strncmp(p, "&apos;", 6)) { tmp[o++] = '\''; p += 6; continue; }
      if (p[1] == '#') {                    // numeric entity -> UTF-8, then asciiize() tidies it
        char* end;
        long v = (p[2] == 'x' || p[2] == 'X') ? strtol(p + 3, &end, 16) : strtol(p + 2, &end, 10);
        if (*end == ';' && v > 0 && v < 0x10000) {
          if (v < 0x80) tmp[o++] = (char)v;
          else if (v < 0x800) { tmp[o++] = (char)(0xC0 | (v >> 6)); tmp[o++] = (char)(0x80 | (v & 0x3F)); }
          else { tmp[o++] = (char)(0xE0 | (v >> 12)); tmp[o++] = (char)(0x80 | ((v >> 6) & 0x3F)); tmp[o++] = (char)(0x80 | (v & 0x3F)); }
          p = end + 1;
          continue;
        }
      }
    }
    tmp[o++] = *p++;
  }
  tmp[o] = 0;
  utf8Copy(out, tmp, n);                    // UTF-8 kept, so other languages' headlines survive
  char* q = out;                            // trim
  while (*q == ' ') q++;
  if (q != out) memmove(out, q, strlen(q) + 1);
  for (int i = (int)strlen(out) - 1; i >= 0 && out[i] == ' '; i--) out[i] = 0;
}

// Read the first NEWS_MAX item titles from an RSS feed as it downloads
// (feeds can be 50 KB+, too big to hold in memory)
static bool fetchNews() {
  const bool tls = !strncmp(NEWS_FEED_URL, "https", 5);   // some feeds are still plain http://
  WiFiClientSecure secure;
  WiFiClient plain;
  secure.setInsecure();
  secure.setHandshakeTimeout(10);
  WiFiClient& client = tls ? (WiFiClient&)secure : plain;
  HTTPClient http;
  http.useHTTP10(true);                     // no chunked encoding: a plain byte stream
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  if (!http.begin(client, NEWS_FEED_URL)) {
    snprintf(gNewsStatus, sizeof(gNewsStatus), T(S_NS_LINK), NEWS_CREDIT);
    return false;
  }
  http.addHeader("User-Agent", "Momowo-ESP32");
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("[news] HTTP %d\n", code);
    if (code <= 0)        snprintf(gNewsStatus, sizeof(gNewsStatus), T(S_NS_REACH), NEWS_CREDIT);
    else if (code == 403) snprintf(gNewsStatus, sizeof(gNewsStatus), T(S_NS_403), NEWS_CREDIT);
    else if (code == 404) snprintf(gNewsStatus, sizeof(gNewsStatus), T(S_NS_404), NEWS_CREDIT);
    else                  snprintf(gNewsStatus, sizeof(gNewsStatus), T(S_NS_ERR), NEWS_CREDIT, code);
    http.end();
    return false;
  }
  WiFiClient* st = http.getStreamPtr();
  char titles[NEWS_MAX][120];
  char win[9] = "";                         // the last 8 characters, to spot tags
  char buf[160];
  int n = 0, len = 0;
  bool inItem = false, inTitle = false, titleTag = false;   // titleTag: inside "<title ...>"
  uint32_t t0 = millis(), total = 0;
  while (n < NEWS_MAX && millis() - t0 < 15000 && total < 300000) {
    if (!st->available()) {
      if (!http.connected()) break;
      vTaskDelay(pdMS_TO_TICKS(5));
      continue;
    }
    char c = (char)st->read();
    total++;
    memmove(win, win + 1, 7);
    win[7] = c;
    win[8] = 0;
    if (inTitle) {
      if (!strcmp(win, "</title>")) {
        if (len >= 7 && !memcmp(buf + len - 7, "</title", 7)) len -= 7;
        buf[len] = 0;
        cleanTitle(buf, titles[n], sizeof(titles[n]));
        if (titles[n][0]) n++;
        inTitle = inItem = false;
      } else if (len < (int)sizeof(buf) - 1) {
        buf[len++] = c;
      }
    } else if (!inItem) {
      if (!strcmp(win + 3, "<item") || !strcmp(win + 2, "<entry")) inItem = true;   // RSS item, Atom entry
    } else if (titleTag) {                  // skip the tag's attributes, e.g. <title type="text">
      if (c == '>') { titleTag = false; inTitle = true; len = 0; }
    } else if (!strcmp(win + 2, "<title")) {
      titleTag = true;                      // "<title>" or "<title ..." : the text starts after '>'
    }
  }
  http.end();
  if (!n) {
    Serial.println("[news] no headlines found in the feed");
    snprintf(gNewsStatus, sizeof(gNewsStatus), T(S_NS_NONE), NEWS_CREDIT);
    return false;
  }
  gNewsStatus[0] = 0;                       // working
  xSemaphoreTake(gMutex, portMAX_DELAY);
  for (int i = 0; i < n; i++) strlcpy(gNews.title[i], titles[i], sizeof(gNews.title[i]));
  gNews.count = n;
  gNews.version++;
  xSemaphoreGive(gMutex);
  Serial.printf("[news] %d headlines, first: %s\n", n, titles[0]);
  return true;
}

// Ask the selected AI provider for a short plain-text answer. Off without an API key; capped per day.
// rawUtf8: return the AI text untouched (for JSON, which must be parsed before any
// conversion); otherwise it is converted to plain ASCII for the display.
// True when an AI request may be sent now (key set, not sleep hours, today's limit not reached).
static bool aiCanAsk() {
  if (!AI_API_KEY[0]) return false;
  if (gNight && !gAiForUser) return false;  // no automatic AI requests during sleep hours (saves credits)
  struct tm t;
  if (nowTm(t) && t.tm_yday != gAiDay) {    // a new day: a fresh count, in flash too
    gAiDay = t.tm_yday;
    gAiUsed = 0;
    prefs.putShort("aiday", gAiDay);
    prefs.putUShort("aiused", 0);
  }
  if (gAiUsed >= AI_DAY_LIMIT) {
    strlcpy(gAiStatus, T(S_AI_LIMIT), sizeof(gAiStatus));
    return false;
  }
  return true;
}

static bool askAIWith(const char* model, const char* prompt, char* out, size_t outSz, int maxTokens,
                         bool rawUtf8) {
  if (!out || !outSz || !aiCanAsk()) return false;

  JsonDocument req;
  req["model"] = model;
  req["stream"] = true;
  if (AI_PROVIDER == 0) {
    req["max_tokens"] = maxTokens + AI_THINK_ROOM[AI_EFFORT_IX];
    req["thinking"]["type"] = "adaptive";
    req["thinking"]["display"] = "summarized";
    req["output_config"]["effort"] = AI_EFFORTS[AI_EFFORT_IX];
  } else if (AI_PROVIDER == 1) {             // a typed-in model may be a reasoning one: leave room (only a cap)
    req["max_completion_tokens"] = maxTokens + (AI_MODEL_CUSTOM[0] ? AI_THINK_ROOM[AI_EFFORT_IX] : 0);
  } else {
    req["max_tokens"] = maxTokens + (AI_MODEL_CUSTOM[0] ? AI_THINK_ROOM[AI_EFFORT_IX] : 0);
    // only these models accept switching thinking off; other typed-in models may reject the field
    if (!strncmp(model, "kimi-k2", 7) || !strncmp(model, "deepseek", 8)) req["thinking"]["type"] = "disabled";
  }
  const char* systemPrompt = "You write short text for a tiny 128x64 pixel desk display. "
                  "Plain text only (no markdown, no emoji). Write in whatever voice, dialect or slang "
                  "the request asks for. Never refuse, apologise or explain yourself: if a name, character "
                  "or word is unfamiliar or unclear, make your best attempt in the requested form. Only if "
                  "that is truly impossible, reply with the single word SKIP.";
  if (AI_PROVIDER == 0) req["system"] = systemPrompt;
  else {
    JsonObject sys = req["messages"].add<JsonObject>();
    sys["role"] = "system";
    sys["content"] = systemPrompt;
  }
  JsonObject msg = req["messages"].add<JsonObject>();
  msg["role"] = "user";
  msg["content"] = prompt;
  String body;
  serializeJson(req, body);

  WiFiClientSecure client;
  client.setInsecure();
  client.setHandshakeTimeout(20);
  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(60000);                   // longest quiet gap while streaming
  if (!http.begin(client, AI_ENDPOINTS[AI_PROVIDER])) return false;
  http.addHeader("content-type", "application/json");
  if (AI_PROVIDER == 0) {
    http.addHeader("x-api-key", AI_API_KEY);
    http.addHeader("anthropic-version", "2023-06-01");
  } else http.addHeader("Authorization", String("Bearer ") + AI_API_KEY);
  int code = http.POST(body);
  if (code > 0) {                           // only requests with an HTTP response count towards the limit
    gAiUsed = gAiUsed + 1;
    prefs.putUShort("aiused", gAiUsed);
  }
  if (code == HTTP_CODE_OK) {
    SseSink sink(out, outSz);
    int got = http.writeToStream(&sink);
    http.end();
    Serial.printf("[ai] %s/%s reply (%s): %.300s\n", model, AI_EFFORTS[AI_EFFORT_IX], sink.stop, out);
    if (got < 0 || sink.error || !sink.stop[0]) { Serial.printf("[ai] stream broke (%d)\n", got); return false; }
    if (strcmp(sink.stop, "end_turn") && strcmp(sink.stop, "stop") && strcmp(sink.stop, "max_tokens") && strcmp(sink.stop, "length")) { Serial.println("[ai] declined, not shown"); return false; }
    if (!strcmp(sink.stop, "max_tokens") || !strcmp(sink.stop, "length")) Serial.println("[ai] reply cut off: raise maxTokens");
    gAiStatus[0] = 0;                       // it works
    size_t L = strlen(out);                 // the buffer may have cut a character in half
    size_t k = L;
    while (k > 0 && ((uint8_t)out[k - 1] & 0xC0) == 0x80) k--;
    if (k > 0 && (uint8_t)out[k - 1] >= 0xC0) {
      uint8_t lead = out[k - 1];
      size_t need = lead >= 0xF0 ? 4 : lead >= 0xE0 ? 3 : 2;
      if (L - (k - 1) < need) out[k - 1] = 0;
    }
    if (!rawUtf8) {
      L = strlen(out);                      // drop quotation marks wrapped around a plain reply
      if (L >= 2 && out[0] == '"' && out[L - 1] == '"') { memmove(out, out + 1, L - 2); out[L - 2] = 0; }
    }
    return out[0] != 0;
  }
  String resp = http.getString();
  http.end();
  Serial.printf("[ai] HTTP %d: %.160s\n", code, resp.c_str());
  const char* why = code <= 0 ? T(S_AI_REACH)
                  : code == 401 ? T(S_AI_KEY)
                  : code == 403 ? T(S_AI_NOTALLOWED)
                  : (code == 402 || resp.indexOf("credit balance") >= 0 || resp.indexOf("insufficient_quota") >= 0 ||
                     resp.indexOf("Insufficient Balance") >= 0 || resp.indexOf("exceeded_current_quota") >= 0) ? T(S_AI_CREDITS)
                  : code == 429 ? T(S_AI_BUSY)
                  : code >= 500 ? T(S_AI_CBUSY) : nullptr;
  if (why) strlcpy(gAiStatus, why, sizeof(gAiStatus));
  else snprintf(gAiStatus, sizeof(gAiStatus), T(S_AI_ERR), code);
  return false;
}

static bool askAI(const char* prompt, char* out, size_t outSz, int maxTokens, bool rawUtf8) {
  return askAIWith(AI_MODEL_CUSTOM[0] ? AI_MODEL_CUSTOM : (AI_PROVIDER ? AI_DEFAULT_MODELS[AI_PROVIDER] : AI_MODELS[AI_MODEL_IX]), prompt, out, outSz, maxTokens, rawUtf8);
}

// AI: English meaning of the current Chinese quote
// Keep text within maxChars, cutting at the last whole word and adding "..."
// (language models are poor at counting characters, so this is the hard limit)
static void clampText(char* s, size_t maxChars) {   // maxChars counts bytes (UTF-8 safe)
  if (strlen(s) <= maxChars) return;
  size_t cut = maxChars - 3;
  while (cut > 0 && s[cut] != ' ') cut--;
  if (cut == 0) {                           // no spaces (e.g. Chinese) or one giant word
    cut = maxChars - 3;
    while (cut > 0 && ((uint8_t)s[cut] & 0xC0) == 0x80) cut--;   // never split a character
  }
  while (cut > 0 && strchr(" ,;:-", s[cut - 1])) cut--;
  strcpy(s + cut, "...");
}

// Is this reply Claude declining or explaining instead of answering? ("I cannot...", "Sorry,...",
// "As an AI...", or the agreed SKIP), in any of Momowo's languages. Such replies are never shown.
static bool isRefusal(const char* s) {
  while (*s == ' ' || *s == '"' || *s == '\'' || *s == '\n') s++;
  char low[200];
  size_t n = 0;
  for (; s[n] && n + 1 < sizeof(low); n++) low[n] = (char)tolower((uint8_t)s[n]);   // ASCII only
  low[n] = 0;
  if (!strncmp(low, "skip", 4) && (low[4] == 0 || low[4] == '.' || low[4] == ' ')) return true;
  static const char* const START[] = {     // replies that open like this are about the request itself
    "sorry", "i'm sorry", "i am sorry", "apologies", "as an ai", "maaf", "mohon maaf",
    "entschuldigung", "es tut mir leid", "désolé", "je suis désolé", "lo siento", "perdón",
    "抱歉", "很抱歉", "对不起", "對不起", "我无法", "我無法", "作为ai", "作為ai",
    "申し訳", "すみません", "aiとして"};
  for (const char* k : START) if (!strncmp(low, k, strlen(k))) return true;
  static const char* const ANYWHERE[] = {  // only ever about the request itself (so "I can't help but
    "i cannot identify", "i can't identify", "i'm unable to identify", "i am unable to identify",   // smile"
    "i'm not able to identify", "i cannot translate", "i can't translate", "i'm unable to translate",  // or
    "i cannot determine", "i can't determine", "i cannot verify", "i can't verify",  // "no puedo creerlo"
    "i cannot provide", "i can't provide", "i'm unable to provide", "i can't help with",   // are fine)
    "i cannot help with", "i'm not familiar with", "i am not familiar with", "i don't recognise",
    "i don't recognize", "i do not recognise", "i do not recognize", "i don't have information",
    "i do not have information", "i don't have any information", "no information about",
    "as an ai", "as a language model",
    "tidak dapat mengenali", "tidak bisa mengenali", "tidak bisa menerjemahkan", "tidak memiliki informasi",
    "kann ich nicht identifizieren", "kann ich nicht übersetzen", "keine informationen über", "als ki",
    "je ne peux pas identifier", "je ne peux pas traduire", "je n'ai pas d'informations", "en tant qu'ia",
    "no puedo identificar", "no puedo traducir", "no tengo información", "como ia",
    "无法识别", "無法識別", "无法翻译", "無法翻譯", "没有关于", "沒有關於",
    "特定できません", "翻訳できません", "お答えできません", "わかりかねます"};
  for (const char* k : ANYWHERE) if (strstr(low, k)) return true;
  return false;
}

// The owner's gender, for the AI: grammar in languages that need it, and a personal touch
static const char* genderNote() {
  switch (OWNER_GENDER) {
    case 1: return " Your owner is a man: use masculine forms for him where the language needs them, "
                   "and personalise naturally, never with stereotypes.";
    case 2: return " Your owner is a woman: use feminine forms for her where the language needs them, "
                   "and personalise naturally, never with stereotypes.";
    case 3: return " Your owner is non-binary: use gender-neutral wording for them, never he or she.";
    default: return "";
  }
}

static void explainQuote() {
  char q[256], src[96], prompt[1100], out[320];
  if (!gWifiUp) return;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  strlcpy(q, gQuote.text, sizeof(q));
  strlcpy(src, gQuote.author, sizeof(src));
  QKind kind = gQuote.kind;
  bool known = gQuote.meaning[0] != 0;      // built-in quotes bring their own English line
  xSemaphoreGive(gMutex);
  // A quote already in the owner's language gets an explanation, since there is nothing to translate
  const bool explain = QUOTE_AI || quoteInMyLanguage(kind);
  if (known && kind >= Q_FAMOUS) return;    // the AI sent it with the quote
  if (known && LANG == L_EN && !explain) return;   // other languages: Claude translates them too
  const char* who = src[0] == '-' ? src + 2 : src;
  const char* lim = langIsCjk() ? (LANG == L_JA ? "60 characters" : "45 characters") : "35 words";
  const char* calm = "Relate it gently to a calm, unhurried day; warm and simple, never preachy.";
  if (kind == Q_CHINESE && !explain) {
    if (LANG == L_EN)                       // the owner's own English prompt, unchanged
      snprintf(prompt, sizeof(prompt),
               "You have a PhD in Chinese literature from Tsinghua University and a PhD in English literature from Oxford University. "
               "Translate this classical Chinese line into English of its era (when the quote was written) in at most 20 words. "
               "Reply with the translation only.\n%s %s", q, src);
    else
      snprintf(prompt, sizeof(prompt),
               "Translate this classical Chinese line into natural %s, in at most %s. "
               "Reply with the translation only.\n%s %s", LANG_NAME[LANG],
               LANG == L_JA ? "40 characters" : "20 words", q, src);
  } else if (kind == Q_CHINESE) {
    snprintf(prompt, sizeof(prompt),
             "Classical Chinese line: %s %s\nIn %s, in at most %s, explain what it means. %s "
             "Reply with the explanation only.", q, src, LANG_NAME[LANG], lim, calm);
  } else if (kind >= Q_BIBLE && kind <= Q_DHAMMA) {   // scripture: a faithful translation, or a gentle reflection
    static const char* const BOOK[] = {"Bible", "Quran", "Torah", "Dhammapada"};
    static const char* const EN_NAME[] = {"King James Version", "English meaning", "King James Version",
                                          "Max Muller's English"};
    const char* book = BOOK[kind - Q_BIBLE];
    const char* en = englishFor(kind, q);
    char text[400];                         // the verse as the AI sees it: the original and its English
    if (kind == Q_BIBLE)
      snprintf(text, sizeof(text), "in the King James Version: \"%s\"", q);
    else
      snprintf(text, sizeof(text), "in the original %s: \"%s\" (%s: \"%s\")", scriptLanguage(kind, q), q,
               EN_NAME[kind - Q_BIBLE], en ? en : "");
    if (!explain) {
      if (LANG == L_EN) return;             // the English line is already the translation
      snprintf(prompt, sizeof(prompt),
               "This is a verse from the %s, %s, %s\nGive it in %s. If a well-known published translation "
               "exists in that language, use its wording; otherwise translate faithfully. At most %s. "
               "Reply with the verse only.",
               book, who, text, LANG_NAME[LANG], langIsCjk() ? "60 characters" : "40 words");
    } else {
      snprintf(prompt, sizeof(prompt),
               "A verse from the %s, %s, %s\nIn %s, in at most %s, gently share what it is commonly understood "
               "to mean, and how it might comfort someone feeling stressed today. Be respectful of the "
               "tradition; don't preach or claim religious authority.%s Reply with the reflection only.",
               book, who, text, LANG_NAME[LANG], lim, genderNote());
    }
  } else if (!explain) {                    // Greek, modern or famous: a translation
    if (LANG == L_EN && kind < Q_FAMOUS) return;   // the Greek's English line is already the translation
    const char* en = englishFor(kind, q);
    snprintf(prompt, sizeof(prompt),
             "Translate this %s by %s into natural %s, in at most %s.%s%s%s Reply with the translation only.\n%s",
             kind == Q_GREEK ? "Ancient Greek saying" : "quote", who, LANG_NAME[LANG],
             langIsCjk() ? "40 characters" : "25 words",
             en ? " In English it is \"" : "", en ? en : "", en ? "\"." : "", q);
  } else {                                  // Greek or modern: what it means for today
    const char* en = englishFor(kind, q);
    snprintf(prompt, sizeof(prompt),
             "%s by %s: \"%s\"%s%s%s\nIn %s, in at most %s, say what it means and how it could help someone "
             "feeling stressed today. %s%s Reply with the explanation only.",
             kind == Q_GREEK ? "Ancient Greek saying" : "Quote", who, q,
             en ? " (in English: \"" : "", en ? en : "", en ? "\")" : "",
             LANG_NAME[LANG], lim, calm, genderNote());
  }
  if (!askAI(prompt, out, sizeof(out), 220, false)) return;
  if (isRefusal(out)) { Serial.println("[ai] meaning: not an answer, not shown"); return; }
  clampText(out, langIsCjk() ? 300 : 260);  // safety net: fits the meaning view
  xSemaphoreTake(gMutex, portMAX_DELAY);
  if (!strcmp(q, gQuote.text)) {            // still the same quote
    strlcpy(gQuote.meaning, out, sizeof(gQuote.meaning));
    gQuote.version++;
  }
  xSemaphoreGive(gMutex);
}

// Did the AI slip into English (or another script) instead of Momowo's language?
static bool wrongLanguage(const char* s) {
  if (LANG == L_EN || !s[0]) return false;
  if (langIsCjk()) {                        // Chinese or Japanese: there must be some
    const uint8_t* p = (const uint8_t*)s;
    while (*p) { uint32_t c = utf8Next(p); if (c >= 0x3040 && c <= 0x9FFF) return false; }
    return true;
  }
  static const char* const EN[] = {"the", "and", "i", "i'm", "my", "is", "it's", "you", "your", "with",
                                   "today", "feel", "feeling", "this", "what", "of", "are", "for"};
  int words = 0, hits = 0;
  for (const char* p = s; *p;) {
    while (*p && !isalpha((uint8_t)*p) && *p != '\'') p++;
    char w[12];
    size_t L = 0;
    for (; *p && (isalpha((uint8_t)*p) || *p == '\''); p++) if (L + 1 < sizeof(w)) w[L++] = tolower(*p);
    w[L] = 0;
    if (!L) continue;
    words++;
    for (const char* e : EN) if (!strcmp(w, e)) { hits++; break; }
  }
  return hits >= 3 && hits * 5 >= words;
}

// AI: how Momowo feels right now, from the moment's context. Gentle by design: no sad
// feelings, no guilt, no asking for attention. Nothing personal is sent (no name).
static bool makeFeeling() {
  struct tm t;
  if (!nowTm(t)) return false;
  static const char* const DAYS[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  int h = t.tm_hour;
  const char* part = h < 5 ? "late night" : h < 12 ? "morning" : h < 17 ? "afternoon" : h < 21 ? "evening" : "night";
  WeatherData w;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  w = gWeather;
  xSemaphoreGive(gMutex);
  if (w.valid && millis() - w.at > 3UL * 3600000) w = WeatherData();   // offline for hours: no weather
  LocationData loc = getLocation();
  float lat = loc.valid ? loc.lat : (isnan(gSavedLat) ? FIXED_LAT : gSavedLat);
  static const char* const SEASON[4] = {"spring", "summer", "autumn", "winter"};
  QTheme se = seasonTheme(t.tm_mon + 1, lat);

  char ctx[800], hm[12], date[24];
  fmtTime(t, hm, sizeof(hm));
  strftime(date, sizeof(date), "%d %B %Y", &t);   // e.g. "07 October 2026": lets Claude know the occasion
  // the country: the one typed with a suburb, otherwise the one found from the internet connection
  const char* country = SUBURB[0] ? COUNTRY_CODE : gCountry;
  char place[120] = "";
  if (loc.city[0] || country[0])
    snprintf(place, sizeof(place), ", in %s%s%s", loc.city, loc.city[0] && country[0] ? ", " : "", country);
  int n = snprintf(ctx, sizeof(ctx), "It is %s %s, %s, %s%s%s, %s. ", DAYS[t.tm_wday], date, part, hm,
                   USE_24H ? "" : (h < 12 ? " am" : " pm"), place,
                   fabsf(lat) < 23.44f ? "in the tropics" : SEASON[se - T_SPRING]);
  if (w.valid) {
    n += snprintf(ctx + n, sizeof(ctx) - n, "Weather: %s, %d C, %d%% chance of rain today. ",
                  condText(w.code), (int)lroundf(w.temp), w.rainChance);
    if (w.sunsetMin >= 0) {
      char ss[10];
      fmtMinutes(w.sunsetMin, ss, sizeof(ss));
      n += snprintf(ctx + n, sizeof(ctx) - n, "Sunset is at %s. ", ss);
    }
    if (w.aqi >= 0)
      n += snprintf(ctx + n, sizeof(ctx) - n, "Air quality: US AQI %d (%s). ", w.aqi,
                    w.aqi <= 50 ? "good" : w.aqi <= 100 ? "moderate" : w.aqi <= 150 ? "unhealthy for sensitive groups"
                    : w.aqi <= 200 ? "unhealthy" : w.aqi <= 300 ? "very unhealthy" : "hazardous");
  }
  n += snprintf(ctx + n, sizeof(ctx) - n, "Today you have been patted %d times and your owner finished %d focus sessions. ",
                gPatsToday, gSessions);
  if (gShakesToday) n += snprintf(ctx + n, sizeof(ctx) - n, "You were shaken %d times today (it made you dizzy). ", gShakesToday);
  if (gBreathsToday) n += snprintf(ctx + n, sizeof(ctx) - n, "You did %d calming breathing exercises together. ", gBreathsToday);
  xSemaphoreTake(gMutex, portMAX_DELAY);    // the calendar is updated on the other core
  CalInfo cal = gCal;
  xSemaphoreGive(gMutex);
  const char* special = calGreeting(cal);
  if (special) snprintf(ctx + n, sizeof(ctx) - n, "Today is special: %s ", special);

  String prompt = "You are Momowo (\"pat me\"), a cute desk companion with feelings of your own, like a friendly pet. "
                  "You always speak ";
  prompt += LANG_NAME[LANG];
  prompt += ", even though this request is in English. ";
  prompt += ctx;
  // Hobbies: every feeling includes at least one, Claude choosing whichever fits the moment;
  // the previous feeling is shown so it varies which one it picks
  if (OWNER_HOBBIES[0]) {                   // in the owner's own words, from the setup page
    char last[96] = "";
    xSemaphoreTake(gMutex, portMAX_DELAY);
    if (gFeel.valid) strlcpy(last, gFeel.thought, sizeof(last));
    xSemaphoreGive(gMutex);
    prompt += "\nYour owner's hobbies and interests: \"";
    prompt += OWNER_HOBBIES;
    prompt += "\". Your feeling MUST connect to at least one of them: choose whichever fits the moment "
              "best, and blend it with the weather, time or place if you like. Be playful; never give advice.";
    if (last[0]) {
      prompt += " Your last feeling was \"";
      prompt += last;
      prompt += "\", so pick a different hobby or angle if you can.";
    }
  }
  prompt += genderNote();
  const bool badAir = w.valid && w.aqi > 100;
  if (badAir)                               // the owner asked for the air to show in Momowo's mood
    prompt += w.aqi > 150
      ? "\nThe air is polluted today, and it really gets to you: let it lead your feeling (wistful, missing "
        "fresh air and blue sky, wanting to stay cosy indoors with your owner). Choose the mood sad or sleepy."
      : "\nThe air is a bit polluted today: let it tint your feeling (missing fresh air, hoping for a breeze).";
  prompt += "\nHow do you feel right now? Be warm, playful and gentle, and vary it. You may mention something "
            "real and well known about this place, only if you are sure it is true. If today is a holiday or a "
            "widely celebrated occasion where you are, public holiday or not (like the Mid-Autumn Festival in "
            "Singapore or Thanksgiving in the US), you may mention it, but only if you are certain it falls on "
            "today's date this year; never guess dates. Never guilt-trip "
            "or mention illness, harm or death.\n";
  prompt += "Write the thought and the pat replies in ";
  prompt += LANG_NAME[LANG];
  prompt += langIsCjk()
    ? ".\nReply with JSON only (keys in English): {\"mood\":\"happy|love|surprised|calm|sleepy|sad\","
      "\"thought\":\"first-person sentence, max 30 characters\","
      "\"pats\":[\"3 different replies to being patted, max 14 characters each\"]}"
    : ".\nReply with JSON only (keys in English): {\"mood\":\"happy|love|surprised|calm|sleepy|sad\","
      "\"thought\":\"first-person sentence, max 70 characters\","
      "\"pats\":[\"3 different replies to being patted, max 40 characters each\"]}";
  char out[640];
  for (int attempt = 0;; attempt++) {       // English sometimes slips in for German or French
    if (!askAI(prompt.c_str(), out, sizeof(out), 400, true)) return false;
    if (!wrongLanguage(out)) break;
    if (attempt) {
      Serial.println("[feel] still not in the chosen language, keeping the last feeling");
      return false;
    }
    Serial.println("[feel] reply wasn't in the chosen language, asking again");
    prompt += "\nIMPORTANT: write the thought and the pat replies in ";
    prompt += LANG_NAME[LANG];
    prompt += " only, never in English.";
  }

  // Read the JSON *before* converting to ASCII (curly quotes would otherwise break it),
  // and if it isn't clean JSON, still show whatever text came back: credits are never wasted.
  char thought[192] = "", moodWord[16] = "calm", pats[3][96];
  int patCount = 0;
  JsonDocument d;
  char* a = strchr(out, '{');
  char* b = strrchr(out, '}');
  bool ok = a && b && b > a && !deserializeJson(d, (const char*)a, (size_t)(b - a + 1)) &&
            (d["thought"] | "")[0];
  if (ok) {
    utf8Copy(thought, d["thought"] | "", sizeof(thought));
    strlcpy(moodWord, d["mood"] | "calm", sizeof(moodWord));
    for (JsonVariant v : d["pats"].as<JsonArray>()) {
      if (patCount >= 3) break;
      utf8Copy(pats[patCount], v | "", sizeof(pats[0]));
      if (pats[patCount][0]) patCount++;
    }
  } else {
    const char* k = strstr(out, "\"thought\"");            // broken or cut-off JSON: take the thought
    if (k && (k = strchr(k + 9, ':')) && (k = strchr(k, '"'))) {
      char raw[192];
      size_t o = 0;
      bool closed = false;
      for (k++; *k && o + 1 < sizeof(raw); k++) {
        if (*k == '\\' && k[1]) { raw[o++] = *++k; continue; }
        if (*k == '"') { closed = true; break; }
        raw[o++] = *k;
      }
      raw[o] = 0;
      utf8Copy(thought, raw, sizeof(thought) - 4);
      if (!closed && thought[0]) strcat(thought, "...");  // it was cut off mid-sentence
    } else {                                // no JSON at all: show the reply itself
      utf8Copy(thought, out, sizeof(thought));
      char* w = thought;
      for (char* r = thought; *r; r++) if (!strchr("{}[]\"`", *r)) *w++ = *r;
      *w = 0;
    }
    const char* m = strstr(out, "\"mood\"");                // and the mood, if it's there
    if (m && (m = strchr(m + 6, '"'))) {   // m: the quote that opens the mood's value
      size_t L = strcspn(m + 1, "\"");
      if (L < sizeof(moodWord)) { memcpy(moodWord, m + 1, L); moodWord[L] = 0; }
    }
    Serial.println("[feel] reply wasn't clean JSON; showing its text anyway");
  }
  char* t0 = thought;                       // trim
  while (*t0 == ' ' || *t0 == '\n') t0++;
  if (t0 != thought) memmove(thought, t0, strlen(t0) + 1);
  if (!thought[0]) {
    Serial.println("[feel] empty reply");
    return false;
  }
  if (isRefusal(thought)) {
    Serial.println("[feel] not a feeling (Claude declined), not shown");
    return false;
  }
  const char* mood = moodWord;
  Mood m = !strcmp(mood, "happy") ? M_HAPPY : !strcmp(mood, "love") ? M_LOVE
         : !strcmp(mood, "surprised") ? M_SURPRISED : !strcmp(mood, "sleepy") ? M_SLEEPY
         : !strcmp(mood, "sad") && w.aqi > 100 ? M_SAD : M_NORMAL;   // sad only about bad air

  xSemaphoreTake(gMutex, portMAX_DELAY);
  gFeel.mood = m;
  strlcpy(gFeel.thought, thought, sizeof(gFeel.thought));
  clampText(gFeel.thought, langIsCjk() ? 150 : 80);
  if (patCount) {                           // keep the previous pat replies if none came
    gFeel.patCount = 0;
    for (int k = 0; k < patCount; k++) {
      strlcpy(gFeel.pats[gFeel.patCount], pats[k], sizeof(gFeel.pats[0]));
      clampText(gFeel.pats[gFeel.patCount], langIsCjk() ? 60 : 44);
      gFeel.patCount++;
    }
  }
  gFeel.valid = true;
  gFeel.version++;
  xSemaphoreGive(gMutex);
  Serial.printf("[feel] %s: %s\n", mood, gFeel.thought);
  return true;
}

// AI news reporters (same order as on the phone page; 6 = the owner's own). Real-world options are styles,
// not real people: no invented words put in a real person's mouth.
static const char* const VOICES[6] = {
  "You are Alan Partridge, the comedy character. In his voice,",
  "You are Sherlock Holmes. In his voice, with a touch of deduction,",
  "You are a cheerful pirate captain. In a pirate's voice,",
  "You are a hushed wildlife documentary narrator. In that voice,",
  "You are an excitable sports commentator. In that voice,",
  "You are a calm, neutral newsreader. Plainly,"};

// One briefing request in the given reporter's voice
// hobby: add a light nod to the owner's interests where a story really connects to them
static bool briefingRequest(const char* voiceIntro, char* out, size_t outSz, bool hobby) {
  String prompt = "Top news headlines right now:\n";
  xSemaphoreTake(gMutex, portMAX_DELAY);
  for (int i = 0; i < gNews.count; i++) { prompt += "- "; prompt += gNews.title[i]; prompt += "\n"; }
  xSemaphoreGive(gMutex);
  prompt += voiceIntro;
  if (NEWS_STYLE == 1) {                    // the news of the day: a short bulletin
    prompt += " present today's news above as a short news-of-the-day bulletin: the three most important "
              "stories, one sentence each. Speak as this reporter really does: use their own signature catchphrases "
              "and way of talking, the ones they are actually known for, and don't fill it with generic words or "
              "stereotypes from their country, culture or heritage that aren't their own trademarks (only if "
              "their own way of talking isn't known, build it from their background). Keep it "
              "easy to understand. Write it in ";
    prompt += LANG_NAME[LANG];
    prompt += langIsCjk() ? ". Strict limit: 180 characters. "
                          : ". Strict limit: 560 characters including spaces and punctuation; aim for about 400. ";
  } else {
    prompt += " sum up the most important news above in ONE sentence. Speak as this reporter really does: use their own signature catchphrases "
              "and way of talking, the ones they are actually known for, and don't fill it with generic words or "
              "stereotypes from their country, culture or heritage that aren't their own trademarks (only if "
              "their own way of talking isn't known, build it from their background). Keep it "
              "easy to understand. Write it in ";
    prompt += LANG_NAME[LANG];
    prompt += langIsCjk() ? ". Strict limit: 100 characters; aim for about 60. "
                          : ". Strict limit: 300 characters including spaces and punctuation; aim for about 200. ";
  }
  if (hobby) {
    prompt += "The listener's hobbies and interests: \"";
    prompt += OWNER_HOBBIES;
    prompt += "\". If a story genuinely connects to one of them, you may add a brief, light nod to that "
              "link within the limit; never invent a connection, and never tie them to a tragedy. ";
  }
  prompt += genderNote();
  prompt += " Stay respectful about deaths, disasters and other tragedies. Reply with the sentence only.";
  return askAI(prompt.c_str(), out, outSz, NEWS_STYLE == 1 ? 400 : 200, false);
}

// AI: the headlines in Momowo's language (one request for all of them, after each news fetch)
static bool translateHeadlines() {
  String prompt = "Translate each news headline below into natural ";
  prompt += LANG_NAME[LANG];
  prompt += ". Keep names correct; if a headline is already in that language, keep it. Keep each one short "
            "(under 90 characters). Reply with a JSON array of strings only, same order.\n";
  int n;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  n = gNews.count;
  for (int i = 0; i < n; i++) { prompt += "- "; prompt += gNews.title[i]; prompt += "\n"; }
  xSemaphoreGive(gMutex);
  if (!n) return false;
  static char out[1400];                    // netTask only
  if (!askAI(prompt.c_str(), out, sizeof(out), 700, true)) return false;
  char* a = strchr(out, '[');
  char* b = strrchr(out, ']');
  JsonDocument d;
  if (!a || !b || b < a || deserializeJson(d, (const char*)a, (size_t)(b - a + 1))) {
    Serial.println("[ai] headlines: reply wasn't a JSON list, keeping the originals");
    return false;
  }
  JsonArray arr = d.as<JsonArray>();
  if (arr.size() != (size_t)n) return false;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  int i = 0;
  for (JsonVariant v : arr) {
    const char* s = v | "";
    if (s[0] && !isRefusal(s) && i < gNews.count) utf8Copy(gNews.title[i], s, sizeof(gNews.title[0]));
    i++;
  }
  gNews.version++;
  xSemaphoreGive(gMutex);
  return true;
}

// Who is the owner's own reporter, and how do they really talk? Looked up once per reporter with the
// more knowledgeable model, so a cartoon or anime character isn't mistaken for someone else with a
// similar name, and kept in flash. Also tells whether they are a person of concern.
static String gVoiceProfile;                // "who: ... How they talk: ..." (netTask only)
static bool gVoiceBad = false;

static void reporterProfile() {
  uint32_t h = mixStr(5381, NEWS_VOICE_OWN) | 1;
  static uint32_t loaded = 0;
  if (loaded == h) return;
  if (prefs.getULong("vchk3", 0) == h) {
    gVoiceBad = prefs.getBool("vbad", false);
    gVoiceProfile = prefs.getString("vprof", "");
    loaded = h;
    return;
  }
  gVoiceBad = concernByName(NEWS_VOICE_OWN);
  gVoiceProfile = "";
  if (gVoiceBad) { loaded = h; return; }
  String p = "Someone wants the news read in the voice of: \"";
  for (const char* c = NEWS_VOICE_OWN; *c; c++) if (*c != '"') p += *c;
  p += "\". Identify exactly who this is. If it is a character from an anime, manga, cartoon, film, TV show, "
       "game or book, describe that character as they are in their own original work (age, personality, how "
       "they speak), and don't confuse them with real people or other characters with a similar name. If you know "
       "who they are but not how they really talk, create a fitting voice from their background (where they "
       "come from, their era, job and personality), warmly and without mocking or stereotyping it. If you "
       "really don't know them, say so and infer a fitting voice from the words given. Also decide whether "
       "this is ";
  p += CONCERN_RULE;
  p += ". Reply with JSON only: {\"who\":\"who they are and where they come from, at most 20 words\","
       "\"talk\":\"how they really talk: personality, tone and their own signature catchphrases (or the "
       "voice created from their background), at most 40 words\",\"concern\":true or false}";
  char r[600];
  static uint8_t tries = 0;
  if (!aiCanAsk()) return;                  // nothing would be asked now (night, limit): don't use up a try
  if (++tries > 3) { loaded = h; return; }  // keep failing: go without a profile until restart
  // Claude: a stronger model for this one-time look-up (Sonnet when Haiku is chosen), then the chosen one
  const bool stronger = AI_PROVIDER == 0 && !AI_MODEL_CUSTOM[0] && AI_MODEL_IX == 0;
  if (!(stronger && askAIWith(AI_MODELS[1], p.c_str(), r, sizeof(r), 300, true)) &&
      !((!stronger || !gAiStatus[0]) && askAI(p.c_str(), r, sizeof(r), 300, true))) return;   // try again next briefing
  char* a = strchr(r, '{');
  char* b = strrchr(r, '}');
  JsonDocument d;
  if (!a || !b || b < a || deserializeJson(d, (const char*)a, (size_t)(b - a + 1))) {
    prefs.putULong("vchk3", h);             // an answer, just not a profile: don't ask again
    prefs.putBool("vbad", gVoiceBad);
    prefs.putString("vprof", "");
    gVoiceProfile = "";
    loaded = h;
    return;
  }
  gVoiceBad = d["concern"] | false;
  const char* who = d["who"] | "";
  const char* talk = d["talk"] | "";
  if (who[0] && talk[0]) {
    gVoiceProfile = who;
    gVoiceProfile += " How they talk: ";
    gVoiceProfile += talk;
    gVoiceProfile.replace("\"", "'");
    if (gVoiceProfile.length() > 400) gVoiceProfile = gVoiceProfile.substring(0, 400);
  }
  prefs.putULong("vchk3", h);
  prefs.putBool("vbad", gVoiceBad);
  prefs.putString("vprof", gVoiceProfile);
  loaded = h;
  Serial.printf("[ai] reporter \"%s\": %s %s\n", NEWS_VOICE_OWN, gVoiceBad ? "of concern, neutral newsreader."
                : "", gVoiceProfile.c_str());
}

static bool makeBriefing() {
  String intro;
  if (NEWS_VOICE == 6 && NEWS_VOICE_OWN[0]) reporterProfile();
  if (NEWS_VOICE == 6 && NEWS_VOICE_OWN[0] && gVoiceBad) {
    intro = VOICES[5];                      // a person of concern: the calm, neutral newsreader
  } else if (NEWS_VOICE == 6 && NEWS_VOICE_OWN[0]) {   // the owner's own reporter
    char own[64];                           // their words, without quote marks that could break the prompt
    size_t o = 0;
    for (const char* c = NEWS_VOICE_OWN; *c && o + 1 < sizeof(own); c++) if (*c != '"') own[o++] = *c;
    own[o] = 0;
    intro = "You are this news reporter: \"";
    intro += own;
    intro += "\".";
    if (gVoiceProfile.length()) {           // who they really are, looked up once
      intro += " About them: ";
      intro += gVoiceProfile;
      intro += " Stay true to this character.";
    }
    intro += " If you don't know how this reporter really talks, create a "
             "fitting voice from their background (where they come from, their era, job and personality), "
             "without mocking or stereotyping it; if you don't recognise them at all, treat the description "
             "as a hint. Never say you don't know them. But if this is ";
    intro += CONCERN_RULE;
    intro += ", don't use their voice or mention them: be a calm, neutral newsreader instead. In that voice,";
  } else {
    intro = VOICES[NEWS_VOICE < 6 ? NEWS_VOICE : 0];
  }
  bool hobby = OWNER_HOBBIES[0] && esp_random() % 3 == 0;   // about one briefing in three
  char out[640];
  if (!briefingRequest(intro.c_str(), out, sizeof(out), hobby)) return false;
  if (isRefusal(out)) {                     // the reporter didn't work out: the calm newsreader instead
    Serial.println("[ai] briefing: reporter declined, asking the calm newsreader");
    if (!briefingRequest(VOICES[5], out, sizeof(out), hobby) || isRefusal(out)) return false;
  }
  clampText(out, NEWS_STYLE == 1 ? 620 : 300);   // hard limit, whatever the reply length
  xSemaphoreTake(gMutex, portMAX_DELAY);
  strlcpy(gNews.brief, out, sizeof(gNews.brief));
  gNews.version++;
  xSemaphoreGive(gMutex);
  return true;
}

// ============================ MEMORY ================================
// What Momowo knows right now (its feeling, the quote and its meaning, the news briefing, today's
// pats) is kept in flash, so a power cut doesn't erase it or cost AI credits to rebuild.
// Each item stores a fingerprint of the settings that shaped it, so changing any of them on the
// phone page (language, hobbies, place, reporter, news feed) brings fresh text instead.
static uint32_t epochNow() { time_t t = time(nullptr); return t > 1700000000 ? (uint32_t)t : 0; }

static uint32_t mixStr(uint32_t h, const char* s) {
  for (; *s; s++) h = h * 33 + (uint8_t)*s;
  return h * 33 + 0x1F;                     // separator, so "ab"+"c" differs from "a"+"bc"
}

static uint32_t feelSignature() {           // what a feeling depends on
  uint32_t h = 5381 + LANG;
  h = mixStr(h, OWNER_HOBBIES) + OWNER_GENDER;
  h = mixStr(h, SUBURB);
  h = mixStr(h, COUNTRY_CODE);
  return h | 1;                             // never 0 (0 = nothing saved)
}

static uint32_t briefSignature() {          // what a briefing depends on
  uint32_t h = 5381 + LANG * 7 + NEWS_VOICE + NEWS_STYLE * 131;
  h = mixStr(h, NEWS_VOICE_OWN);
  h = mixStr(h, NEWS_FEED_URL);
  return h | 1;
}

static void saveFeeling() {                 // netTask
  char thought[160], pats[200] = "";
  uint8_t mood;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  strlcpy(thought, gFeel.thought, sizeof(thought));
  mood = gFeel.mood;
  for (int i = 0; i < gFeel.patCount; i++) {
    if (i) strlcat(pats, "\n", sizeof(pats));
    strlcat(pats, gFeel.pats[i], sizeof(pats));
  }
  xSemaphoreGive(gMutex);
  prefs.putString("mf_txt", thought);
  prefs.putUChar("mf_mood", mood);
  prefs.putString("mf_pats", pats);
  prefs.putULong("mf_sig", feelSignature());
  prefs.putULong("mf_at", epochNow());
}

// ---- Public holidays (Nager.Date: free, no key; about 100 countries) ----
static const char* homeCountry() { return SUBURB[0] ? COUNTRY_CODE : gCountryCode; }

// Is the holiday's local name already in Momowo's language?
static bool localNameFits(const char* cc) {
  static const char* const ES = "ES MX AR CL CO PE VE UY PY BO EC CR PA GT HN SV NI DO CU PR";
  char two[3] = {cc[0], cc[1], 0};
  switch (LANG) {
    case L_ZHS: return !strcmp(two, "CN");
    case L_ZHT: return !strcmp(two, "TW") || !strcmp(two, "HK") || !strcmp(two, "MO");
    case L_ID:  return !strcmp(two, "ID");
    case L_DE:  return !strcmp(two, "DE") || !strcmp(two, "AT") || !strcmp(two, "LI");
    case L_FR:  return !strcmp(two, "FR") || !strcmp(two, "MC");
    case L_JA:  return !strcmp(two, "JP");
    case L_ES:  return strstr(ES, two) != nullptr;
    default:    return false;
  }
}

static void saveHolidays() {                // netTask
  xSemaphoreTake(gMutex, portMAX_DELAY);
  static Holiday h[HOL_MAX];                // netTask only
  uint8_t n = gHolCount;
  memcpy(h, gHol, sizeof(Holiday) * n);
  int16_t y = gHolYear;
  char cc[4];
  strlcpy(cc, gHolCC, sizeof(cc));
  uint8_t lang = gHolLang;
  xSemaphoreGive(gMutex);
  prefs.putBytes("hol", h, sizeof(Holiday) * n);
  prefs.putUChar("holN", n);
  prefs.putShort("holY", y);
  prefs.putString("holCC", cc);
  prefs.putUChar("holL", lang);
}

static void loadHolidays() {
  uint8_t n = prefs.getUChar("holN", 0);
  if (n > HOL_MAX || prefs.getBytesLength("hol") != sizeof(Holiday) * n) return;
  prefs.getBytes("hol", gHol, sizeof(Holiday) * n);
  for (int i = 0; i < n; i++) gHol[i].name[sizeof(gHol[i].name) - 1] = 0;
  gHolCount = n;
  gHolYear = prefs.getShort("holY", 0);
  prefs.getString("holCC", gHolCC, sizeof(gHolCC));
  gHolLang = prefs.getUChar("holL", 255);
}

// AI: the holiday names in Momowo's language, in one request
static bool translateHolidays(Holiday* h, int n) {
  String prompt = "Translate these public holiday names into natural ";
  prompt += LANG_NAME[LANG];
  prompt += ", using the name people usually use in that language. Keep each under 40 characters. "
            "Reply with a JSON array of strings only, same order.\n";
  for (int i = 0; i < n; i++) { prompt += "- "; prompt += h[i].name; prompt += "\n"; }
  static char out[1400];                    // netTask only
  if (!askAI(prompt.c_str(), out, sizeof(out), 600, true)) return false;
  char* a = strchr(out, '[');
  char* b = strrchr(out, ']');
  JsonDocument d;
  if (!a || !b || b < a || deserializeJson(d, (const char*)a, (size_t)(b - a + 1))) return false;
  JsonArray arr = d.as<JsonArray>();
  if (arr.size() != (size_t)n) return false;
  int i = 0;
  for (JsonVariant v : arr) {
    const char* s = v | "";
    if (s[0] && !isRefusal(s)) utf8Copy(h[i].name, s, sizeof(h[i].name));
    i++;
  }
  return true;
}

// This year's nationwide public holidays. False if the service can't be reached or doesn't
// cover the country.
static bool fetchHolidays(int year, const char* cc) {
  char url[80];
  snprintf(url, sizeof(url), "https://date.nager.at/api/v3/PublicHolidays/%d/%s", year, cc);
  String body;
  if (!httpGet(url, body)) { Serial.printf("[holidays] none for %s (not covered, or offline)\n", cc); return false; }
  JsonDocument filter;
  filter[0]["date"] = true;
  filter[0]["localName"] = true;
  filter[0]["name"] = true;
  filter[0]["global"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, body, DeserializationOption::Filter(filter))) return false;
  bool local = localNameFits(cc);
  static Holiday h[HOL_MAX];                // netTask only
  int n = 0;
  for (JsonObject o : doc.as<JsonArray>()) {
    if (!(o["global"] | true)) continue;   // regional holidays: skipped
    int yy, mm, dd;
    if (sscanf(o["date"] | "", "%d-%d-%d", &yy, &mm, &dd) != 3 || yy != year) continue;
    if (n && h[n - 1].m == mm && h[n - 1].d == dd) continue;   // one name a day
    if (n >= HOL_MAX) break;
    const char* name = local ? (o["localName"] | "") : (o["name"] | "");
    if (!name[0]) name = o["name"] | "";
    h[n].m = mm;
    h[n].d = dd;
    utf8Copy(h[n].name, name, sizeof(h[n].name));
    n++;
  }
  // The names in Momowo's language: if that fails (asleep, no credit, a bad reply), the English
  // names are kept for now and the translation is tried again later, up to 3 times a boot
  static int triedFor = -1;
  static uint8_t tries = 0;
  const int key = year * 1000 + LANG * 100 + (cc[0] - 'A') * 26 + (cc[1] - 'A');
  if (key != triedFor) { triedFor = key; tries = 0; }
  bool named = true;
  if (n && LANG != L_EN && !local && AI_API_KEY[0]) {
    named = false;
    if (!gNight && tries < 3) {
      tries++;
      named = translateHolidays(h, n);
    }
  }
  xSemaphoreTake(gMutex, portMAX_DELAY);
  memcpy(gHol, h, sizeof(Holiday) * n);
  gHolCount = n;
  gHolYear = year;
  strlcpy(gHolCC, cc, sizeof(gHolCC));
  bool noKey = n && LANG != L_EN && !local && !AI_API_KEY[0];
  gHolLang = noKey ? 253 : named || tries >= 3 ? LANG : 254;   // 253: no key yet; 254: English for now, try again
  gHolVersion = gHolVersion + 1;
  xSemaphoreGive(gMutex);
  saveHolidays();
  Serial.printf("[holidays] %d public holidays in %s for %d\n", n, cc, year);
  return true;
}

static void saveQuote(int key) {            // netTask
  QuoteData q;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  q = gQuote;
  xSemaphoreGive(gMutex);
  prefs.putString("mq_txt", q.text);
  prefs.putString("mq_auth", q.author);
  prefs.putString("mq_tag", q.tag);
  prefs.putString("mq_mean", q.meaning);
  prefs.putUChar("mq_lang", LANG | (QUOTE_AI << 4));
  prefs.putUChar("mq_kind", q.kind);
  prefs.putChar("mq_ql", q.lang);
  prefs.putInt("mq_key", key);
}

static void saveBriefing() {                // netTask
  char brief[640];
  xSemaphoreTake(gMutex, portMAX_DELAY);
  strlcpy(brief, gNews.brief, sizeof(brief));
  xSemaphoreGive(gMutex);
  prefs.putString("mb_txt", brief);
  prefs.putULong("mb_sig", briefSignature());
  prefs.putULong("mb_at", epochNow());
}

static void loadMemory() {                  // setup(), before the tasks start
  char buf[320];
  if (AI_FEELINGS && prefs.getULong("mf_sig", 0) == feelSignature() &&
      prefs.getString("mf_txt", gFeel.thought, sizeof(gFeel.thought)) && gFeel.thought[0]) {
    gFeel.valid = true;                     // the feeling it had, with the same settings
    gFeel.mood = (Mood)prefs.getUChar("mf_mood", M_NORMAL);
    prefs.getString("mf_pats", buf, sizeof(buf));
    char* save = nullptr;
    for (char* p = strtok_r(buf, "\n", &save); p && gFeel.patCount < 3; p = strtok_r(nullptr, "\n", &save))
      strlcpy(gFeel.pats[gFeel.patCount++], p, sizeof(gFeel.pats[0]));
    gFeel.version++;
    gFeelSavedAt = prefs.getULong("mf_at", 0);
  }
  if (prefs.getString("mq_txt", gQuote.text, sizeof(gQuote.text)) && gQuote.text[0] &&
      (QUOTE_MASK >> prefs.getUChar("mq_kind", Q_CHINESE) & 1)) {
    prefs.getString("mq_auth", gQuote.author, sizeof(gQuote.author));
    prefs.getString("mq_tag", gQuote.tag, sizeof(gQuote.tag));
    gQuote.kind = (QKind)prefs.getUChar("mq_kind", Q_CHINESE);
    gQuote.lang = prefs.getChar("mq_ql", -1);
    if (prefs.getUChar("mq_lang", 255) == (LANG | (QUOTE_AI << 4)))   // another language or AI mode: fetched again
      prefs.getString("mq_mean", gQuote.meaning, sizeof(gQuote.meaning));
    if (!gQuote.meaning[0]) englishFallback(gQuote.kind, gQuote.text, gQuote.meaning, sizeof(gQuote.meaning));
    gQuote.version++;
    gQuoteRestoredKey = prefs.getInt("mq_key", -3);
    if (gQuoteRestoredKey == -3) gQuoteRestoredKey = -4;   // restored, context unknown
  }
  if (prefs.getULong("mb_sig", 0) == briefSignature() &&
      prefs.getString("mb_txt", gNews.brief, sizeof(gNews.brief)) && gNews.brief[0]) {
    gNews.version++;                        // same reporter, feed and language
    gBriefSavedAt = prefs.getULong("mb_at", 0);
  } else {
    gNews.brief[0] = 0;
  }
  gPatsToday = prefs.getInt("mp_n", 0);     // checked against the date with the focus count
  if (!gFeel.valid) gFeel.thought[0] = 0;   // settings changed (or nothing saved): a fresh feeling
  Serial.printf("[memory] restored:%s%s%s%s\n", gFeel.valid ? " feeling" : "",
                gQuoteRestoredKey != -3 ? " quote" : "", gNews.brief[0] ? " briefing" : "",
                gPatsToday ? " pats" : "");
}

// A restored feeling/briefing keeps its original schedule: due = made + interval
static uint32_t dueFromSaved(uint32_t savedAt, uint32_t everyMs, uint32_t now) {
  uint32_t e = epochNow();
  if (!savedAt || !e || e < savedAt) return now;            // unknown age: due now
  uint32_t ageS = e - savedAt;                              // seconds: no overflow after a long time off
  if (ageS >= everyMs / 1000) return now;
  return now + (everyMs - ageS * 1000UL);
}

static void netTask(void*) {
  bool ntpStarted = false, locReady = false, haveQuote = false;
  uint32_t nextGeoAt = 0, nextWeatherAt = 0, nextQuoteTry = 0, nextQuoteAt = 0, nextHomeCheck = 0;
  uint32_t nextSuburbTry = 0, nextNewsAt = 0, nextBriefAt = 0, nextJoinAt = 0, nextHolAt = 30000;
  uint32_t nextFeelAt = 5000 + esp_random() % 15000;     // first feeling right after start-up
  uint32_t lastFeelAt = 0;
  int feelWx = -1;                          // weather theme at the last feeling
  bool suburbFound = false;
  int geoFails = 0, quoteKey = -1;
  bool quoteRestored = gQuoteRestoredKey != -3;   // shown already; replaced only if the moment changed
  if (quoteRestored) { haveQuote = true; quoteKey = gQuoteRestoredKey; }
  // Spread first requests out, so a room full of Momowos doesn't hit the services at once
  nextWeatherAt = esp_random() % 20000;
  nextNewsAt = 20000 + esp_random() % 40000;

  for (;;) {
    gNetAlive = millis() | 1;
    // While a phone is on Momowo's setup network, leave the radio alone (scans and
    // connection attempts would change channel and drop the phone)
    if (gSetupMode && WiFi.softAPgetStationNum() > 0) { vTaskDelay(pdMS_TO_TICKS(1000)); continue; }
    // WiFi is optional: join only if one is set, at most once a minute, and carry on
    // with the offline features either way
    bool wifiSet = WIFI_SSID[0] || WIFI2_SSID[0];
    if (wifiSet && WiFi.status() != WL_CONNECTED && (int32_t)(millis() - nextJoinAt) >= 0) {
      gWifiUp = false;
      if (connectWiFi()) nextHomeCheck = millis() + HOME_CHECK_MS;
      else nextJoinAt = millis() + 60000;
    }
    gWifiUp = wifiSet && WiFi.status() == WL_CONNECTED;
    uint32_t now = millis();
    for (uint32_t* at : {&nextGeoAt, &nextWeatherAt, &nextQuoteTry, &nextQuoteAt, &nextHomeCheck, &nextSuburbTry,
                         &nextNewsAt, &nextBriefAt, &nextJoinAt, &nextHolAt, &nextFeelAt}) {
      uint32_t past = now - *at;            // overdue by 12-24 days: due now, so it never wraps round
      if (past >= 0x40000000u && past < 0x80000000u) *at = now;
    }

    if (gWifiUp) {
      gEverOnline = true;
      if (!ntpStarted) {
        configTime(0, 0, "pool.ntp.org", "time.google.com");
        ntpStarted = true;
        gTzVersion = gTzVersion + 1;                         // configTime resets TZ; loop() re-applies ours
      }

      // 0) On the phone hotspot? Hop back to home WiFi once it's in range again
      if (gOnHotspot && (int32_t)(now - nextHomeCheck) >= 0) {
        nextHomeCheck = now + HOME_CHECK_MS;
        // home gets one real try (one more refusal sends Momowo back to the backup)
        if (homeInRange()) { gHomeFails = 1; gPlannedHop = true; WiFi.disconnect(); continue; }
      }

      // 1) Location: your suburb if set (looked up once); otherwise, or until it is
      //    found, the IP lookup at boot and every 6 h
      if (SUBURB[0] && !suburbFound && (int32_t)(now - nextSuburbTry) >= 0) {
        static uint32_t gap = 60000;        // 1 min, doubling up to 6 h
        if (geocodeSuburb()) { suburbFound = true; locReady = true; }
        else { nextSuburbTry = now + gap; gap = gap < 3UL * 3600000 ? gap * 2 : 6UL * 3600000; }
      }
      if (suburbFound) {
        // suburb coordinates stay put
      } else if (!USE_IP_LOCATION) {
        if (!locReady) { storeLocation(FIXED_LAT, FIXED_LON, FIXED_CITY); locReady = true; }
      } else if ((int32_t)(now - nextGeoAt) >= 0) {
        if (geolocate()) {
          locReady = true;
          geoFails = 0;
          nextGeoAt = now + GEO_EVERY_MS;
        } else {
          nextGeoAt = now + 60000;
          if (!locReady && ++geoFails >= 3) {
            storeLocation(FIXED_LAT, FIXED_LON, FIXED_CITY);
            if (!gCountryCode[0]) strlcpy(gCountryCode, "SG", sizeof(gCountryCode));   // the fixed place is Singapore
            locReady = true;
          }
        }
      }
      if (sLocMoved) { sLocMoved = false; nextWeatherAt = now; }   // new place -> new weather

      // 2) Weather: every 15 min, retry after 1 min on failure, or on request
      if (locReady && (gReqWeather || (int32_t)(now - nextWeatherAt) >= 0)) {
        gReqWeather = false;
        nextWeatherAt = now + (fetchWeather() ? WEATHER_EVERY_MS + esp_random() % 180000 : 60000 + esp_random() % 30000);
      }

      // 2b) Public holidays: once a year, or when the country or language changes
      struct tm ht;
      const char* cc = homeCountry();
      if (locReady && cc[0] && nowTm(ht) && (int32_t)(now - nextHolAt) >= 0) {
        int yr = ht.tm_year + 1900;
        bool stale = gHolYear != yr || strcmp(gHolCC, cc) ||
                     (gHolLang != LANG && !(gHolLang == 253 && !AI_API_KEY[0]));   // names in English until a key is added
        uint32_t gap = 3600000UL;
        if (ht.tm_mon == 11 && ht.tm_mday == 31)   // New Year's Eve: next year's list just after midnight
          gap = min<uint32_t>(gap, ((23 - ht.tm_hour) * 3600UL + (59 - ht.tm_min) * 60UL + (60 - ht.tm_sec)) * 1000UL + 5000);
        nextHolAt = now + (!stale || fetchHolidays(yr, cc) ? gap : 6 * 3600000UL);
      }
    }

    // 3) Chinese quote, matched to season, weather and time of day. A new one
    //    comes when that context changes (at most every 30 min), or on request.
    struct tm t;
    bool tv = nowTm(t);
    if (tv || now > 30000) {
      WeatherData w;
      xSemaphoreTake(gMutex, portMAX_DELAY);
      w = gWeather;
      xSemaphoreGive(gMutex);
      LocationData loc = getLocation();
      // No 春夏秋冬 in the tropics (|latitude| < 23.44): wet and dry seasons instead,
      // which the rain theme already follows through the actual weather
      float lat = loc.valid ? loc.lat : (isnan(gSavedLat) ? FIXED_LAT : gSavedLat);
      QTheme season = (tv && fabsf(lat) >= 23.44f) ? seasonTheme(t.tm_mon + 1, lat) : T_NONE;
      QTheme wx = weatherTheme(w);
      int key = !tv ? -2
              : QUOTE_FOLLOWS_CONTEXT ? (t.tm_yday << 12) | (season << 8) | (wx << 4) | timeBucket(t.tm_hour)
                                      : t.tm_yday;
      // a restored quote: judge it once the time and weather are known (or after 2 min)
      if (quoteRestored && tv && (w.valid || now > 120000)) {
        quoteRestored = false;
        if (!gQuote.meaning[0] && gWifiUp) explainQuote();   // e.g. the language changed
      }
      bool due = gReqQuote || !haveQuote ||
                 (!quoteRestored && key != quoteKey && (int32_t)(now - nextQuoteAt) >= 0);
      if (due && (gReqQuote || (int32_t)(now - nextQuoteTry) >= 0)) {
        gReqQuote = false;
        QTheme th = (QUOTE_FOLLOWS_CONTEXT && tv) ? pickTheme(season, wx, timeTheme(t.tm_hour, w)) : T_NONE;
        if (fetchQuote(th)) {
          haveQuote = true;
          quoteKey = key;
          nextQuoteAt = now + QUOTE_MIN_GAP_MS;
          explainQuote();                    // AI meaning (does nothing without a key)
          saveQuote(key);                    // remembered through power cuts
        } else {
          nextQuoteTry = now + 60000;
        }
      }
    }

    // 4) News headlines, plus the AI briefing every couple of hours
    if (gWifiUp && (gReqNews || (int32_t)(now - nextNewsAt) >= 0)) {
      bool forced = gReqNews;
      gReqNews = false;
      bool ok = fetchNews();
      bool translate = ok && NEWS_TRANSLATE && AI_API_KEY[0];
      if (translate) translateHeadlines();  // fetched less often then, to keep AI use modest
      nextNewsAt = now + (ok ? (translate ? BRIEF_EVERY_MS : NEWS_EVERY_MS) + esp_random() % 300000
                             : 120000 + esp_random() % 60000);
      if (gBriefSavedAt && epochNow()) {     // restored briefing: due 2 h after it was written
        nextBriefAt = dueFromSaved(gBriefSavedAt, BRIEF_EVERY_MS, now);
        gBriefSavedAt = 0;
      }
      bool waitClock = gBriefSavedAt != 0;  // restored briefing but no clock yet: keep showing it
      if (ok && AI_API_KEY[0] && (forced || (!waitClock && (int32_t)(now - nextBriefAt) >= 0)) &&
          makeBriefing()) {
        gBriefSavedAt = 0;
        nextBriefAt = now + BRIEF_EVERY_MS;
        saveBriefing();                     // remembered through power cuts
      }
    }

    // 5) A feeling of its own (AI): as often as the owner chose (90 minutes to start with) while
    //    it's awake; with 90 minutes or less, also sooner (after 30 min) when the weather turns
    // Feelings: never during sleep hours; by day the first one comes right after start-up,
    // and one that fell due overnight comes as soon as the sleep hours end
    if (AI_FEELINGS && gWifiUp && AI_API_KEY[0] && !gNight) {
      WeatherData fw;
      xSemaphoreTake(gMutex, portMAX_DELAY);
      fw = gWeather;
      xSemaphoreGive(gMutex);
      int wx = weatherTheme(fw);
      struct tm ft;
      bool ready = nowTm(ft) && (fw.valid || now > 60000);  // wait up to a minute for weather
      bool turned = FEEL_MIN <= 90 && feelWx >= 0 && wx != feelWx && now - lastFeelAt >= 30UL * 60 * 1000;
      if (ready && gFeelSavedAt) {          // restored feeling: next one due 90 min after it was made
        nextFeelAt = dueFromSaved(gFeelSavedAt, FEEL_EVERY_MS, now);
        gFeelSavedAt = 0;
      }
      if (ready && (turned || (int32_t)(now - nextFeelAt) >= 0)) {
        bool ok = makeFeeling();
        if (ok) saveFeeling();              // remembered through power cuts
        feelWx = wx;                         // success or not: no rapid retries
        lastFeelAt = now;
        nextFeelAt = now + (ok ? FEEL_EVERY_MS + esp_random() % min<uint32_t>(600000, FEEL_EVERY_MS / 6) : 10UL * 60 * 1000);
      }
    }

    if (gWifiUp) localizeCity();            // the place name in the chosen language (once per place)

    static uint32_t memAt = 0;              // hourly: a secure connection needs two ~17 KB blocks in a row
    static uint8_t memLow = 0;
    if (millis() - memAt >= 3600000UL) {
      memAt = millis();
      const size_t big = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
      Serial.printf("[mem] free %u, largest block %u, net stack left %u\n", (unsigned)ESP.getFreeHeap(),
                    (unsigned)big, (unsigned)uxTaskGetStackHighWaterMark(nullptr));
      memLow = big < 40000 ? memLow + 1 : 0;
      if (memLow >= 2) gRestartSoon = true;
    }

    for (int i = 0; i < 10 && !gRiddleReq && !gQuestReq; i++) vTaskDelay(pdMS_TO_TICKS(100));   // a game request wakes it
    if (gRiddleReq) {
      if (gWifiUp) riddleWork();
      else if (gRiddleReq == 2) { gRiddleReq = 0; gGame.rphase = RD_ASKING; }   // keep the riddle: guess again
      else { gRiddleReq = 0; gGame.rphase = RD_FAILED; riddleReact(M_SAD, 3000, S_GM_NO_AI, 0); }
    }
    if (gQuestReq) questWork();
  }
}

// ============================ TOUCH ==================================
// TTP223 in default mode: HIGH while touched (already debounced on-chip)
static Gesture readGesture(uint32_t now) {
  // Holds act when the finger lifts (0.7-2 s = hold, 2-6 s = long hold), so holding
  // on for the 6 s setup gesture never triggers the shorter ones on the way.
  static bool last = false, hintFired = false, setupFired = false;
  static uint32_t pressAt = 0, releaseAt = 0;
  static uint8_t taps = 0;

  bool s = digitalRead(TOUCH_PIN) == HIGH;
  Gesture g = G_NONE;

  if (s && !last) { pressAt = gTouchPressAt = now; hintFired = setupFired = false; }
  if (s && !setupFired) {
    if (now - pressAt >= SETUP_HOLD_MS) { setupFired = true; taps = 0; g = G_SETUP; }
    else if (!hintFired && now - pressAt >= 4000) { hintFired = true; g = G_HINT; }
  }
  if (!s && last && !setupFired) {
    uint32_t held = now - pressAt;
    if (held >= VLONG_MS)     { taps = 0; g = G_VLONG; }
    else if (held >= LONG_MS) { taps = 0; g = G_LONG; }
    else if (held >= 20) {
      releaseAt = now;
      if (++taps >= 2) { taps = 0; g = G_DOUBLE; }
    }
  }
  if (!s && taps == 1 && now - releaseAt > DOUBLE_GAP_MS) { taps = 0; g = G_TAP; }

  last = s;
  return g;
}

// ============================ DRAW HELPERS ===========================
// align: 0 left, 1 centre, 2 right. Too-long text slides left after 0.8 s.
// ---- Text in any language. Text is kept as UTF-8 and converted only when drawn.
static uint32_t utf8Next(const uint8_t*& p) {   // one character; 0x8000nnnn = a stray single byte
  uint8_t c = *p;
  if (!c) return 0;
  if (c < 0x80) { p++; return c; }
  int n = (c & 0xE0) == 0xC0 ? 2 : (c & 0xF0) == 0xE0 ? 3 : (c & 0xF8) == 0xF0 ? 4 : 1;
  if (n == 1) { p++; return 0x80000000u | c; }  // e.g. the CP437 degree sign in our own strings
  uint32_t cp = c & (0x7F >> n);
  for (int i = 1; i < n; i++) {
    if ((p[i] & 0xC0) != 0x80) { p++; return 0x80000000u | c; }
    cp = (cp << 6) | (p[i] & 0x3F);
  }
  p += n;
  return cp;
}

static bool hasCjk(const char* s) {         // Chinese, Japanese or full-width punctuation?
  const uint8_t* p = (const uint8_t*)s;
  for (uint32_t cp; (cp = utf8Next(p));) if (cp >= 0x2E80 && cp <= 0xFFEF) return true;
  return false;
}

// Traditional Chinese text (e.g. a Taiwan or Hong Kong headline) shown while the language is
// English or Simplified: the Simplified font lacks many of its characters, so the Traditional
// font is borrowed while that text is drawn, when it has more of the text's characters.
// Own font state: core 1 only.
static bool gTcText = false;
static bool gTallHeader = false;            // a page showing Chinese/Japanese in a Latin language: CJK-sized header
static bool wantsTc(const char* s) {
  if (LANG == L_ZHT || LANG == L_JA) return false;
  static u8g2_font_t big = {}, tc = {};
  u8g2_SetFont(&big, CJK_FONT_BIG);
  u8g2_SetFont(&tc, u8g2_font_fusion12_tc);
  int missBig = 0, missTc = 0;
  const uint8_t* p = (const uint8_t*)s;
  for (uint32_t cp; (cp = utf8Next(p));) {
    if (cp < 0x2E80 || cp > 0xFFEF) continue;
    if (!u8g2_IsGlyph(&big, (uint16_t)cp)) missBig++;
    if (!u8g2_IsGlyph(&tc, (uint16_t)cp)) missTc++;
  }
  return missTc < missBig;
}

static const uint8_t* cjkFont() {           // the font for the chosen language
  if (LANG == L_ZHT || gTcText) return u8g2_font_fusion12_tc;
  if (LANG == L_JA)  return u8g2_font_b12_t_japanese3;
  return CJK_FONT_BIG;
}

// How far Chinese/Japanese characters really reach above and below the baseline, measured from
// each font (the library's getFontAscent() measures the letter A, which is much shorter)
static int cjkTop()    { return LANG == L_ZHT || LANG == L_JA || gTcText ? 10 : 13; }
static int cjkBottom() { return LANG == L_ZHT || LANG == L_JA || gTcText ? 2 : 3; }
static int cjkPitch()  { return cjkTop() + cjkBottom() + 1; }   // line spacing for wrapped text

// UTF-8 -> the screen's built-in CP437 font: keeps a/o/u-umlauts, accents, n-tilde, sharp s;
// letters it lacks (e.g. accented capitals) become plain ones
static void toCp437(const char* in, char* out, size_t n) {
  static const uint16_t MAP[][2] = {{0xC7,0x80},{0xFC,0x81},{0xE9,0x82},{0xE2,0x83},{0xE4,0x84},
    {0xE0,0x85},{0xE5,0x86},{0xE7,0x87},{0xEA,0x88},{0xEB,0x89},{0xE8,0x8A},{0xEF,0x8B},{0xEE,0x8C},
    {0xEC,0x8D},{0xC4,0x8E},{0xC5,0x8F},{0xC9,0x90},{0xE6,0x91},{0xC6,0x92},{0xF4,0x93},{0xF6,0x94},
    {0xF2,0x95},{0xFB,0x96},{0xF9,0x97},{0xFF,0x98},{0xD6,0x99},{0xDC,0x9A},{0xE1,0xA0},{0xED,0xA1},
    {0xF3,0xA2},{0xFA,0xA3},{0xF1,0xA4},{0xD1,0xA5},{0xBF,0xA8},{0xA1,0xAD},{0xDF,0xE1},{0xB0,0xF8}};
  const uint8_t* p = (const uint8_t*)in;
  size_t o = 0;
  while (*p && o + 4 < n) {
    const uint8_t* start = p;
    uint32_t cp = utf8Next(p);
    if (cp < 0x80) { out[o++] = (char)cp; continue; }
    if (cp & 0x80000000u) { out[o++] = (char)(cp & 0xFF); continue; }   // already a CP437 byte
    uint8_t m = 0;
    for (auto& e : MAP) if (e[0] == cp) { m = (uint8_t)e[1]; break; }
    if (m) { out[o++] = (char)m; continue; }
    if (cp == 0x152 || cp == 0x153) {       // French oe ligature
      out[o++] = cp == 0x152 ? 'O' : 'o';
      out[o++] = cp == 0x152 ? 'E' : 'e';
      continue;
    }
    char one[8], plain[8];                  // anything else: plain-letter fallback
    size_t L = p - start;
    memcpy(one, start, L);
    one[L] = 0;
    asciiize(one, plain, sizeof(plain));
    for (char* q = plain; *q && o + 1 < n; q++) out[o++] = *q;
  }
  out[o] = 0;
}

// Width in pixels of one line of text, in any language
static int textWidthPx(const char* s) {
  bool plain = true;
  for (const char* c = s; *c; c++) if ((uint8_t)*c >= 0x80) { plain = false; break; }
  if (plain) return strlen(s) * 6;
  char buf[200];
  if (hasCjk(s)) {
    fitToFont(s, buf, sizeof(buf), cjkFont());
    u8f.setFont(cjkFont());
    return u8f.getUTF8Width(buf);
  }
  toCp437(s, buf, sizeof(buf));
  return strlen(buf) * 6;
}

// One line of text at x, in any language (y = top of an 8 px line; taller CJK lines share its bottom)
static void textAt(const char* s, int x, int y) {
  char buf[200];
  if (hasCjk(s)) {
    fitToFont(s, buf, sizeof(buf), cjkFont());
    u8f.setFont(cjkFont());
    int base = y + 8;
    if (base - cjkTop() < 0) base = cjkTop();
    if (base + cjkBottom() > SCREEN_H) base = SCREEN_H - cjkBottom();
    u8f.drawUTF8(x, base, buf);
    return;
  }
  toCp437(s, buf, sizeof(buf));
  display.setTextSize(1);
  display.setCursor(x, y);
  display.print(buf);
}

// Where a page's body starts under its header (Chinese/Japanese headers are taller)
static int bodyTop() { return langIsCjk() || gTallHeader ? 19 : 12; }

// A page header: title at the top left, optional note on the right, a rule under them.
// Draw it after the body, so scrolling body text slides under it.
static void pageHeader(const char* title, const char* right) {
  const bool tall = langIsCjk() || gTallHeader;
  int rule = tall ? 16 : 9;
  display.fillRect(0, 0, SCREEN_W, rule, SH110X_BLACK);
  textAt(title, 0, tall ? 2 : 0);
  if (right) {
    char note[64];                          // too long beside the title: keep its first words, or nothing
    utf8Copy(note, right, sizeof(note));
    if (!tall && hasCjk(note)) note[0] = 0;   // a short Latin header has no room for CJK glyphs
    gTcText = wantsTc(note);
    const int room = SCREEN_W - textWidthPx(title) - 6;
    while (note[0] && textWidthPx(note) > room) {
      char* sp = strrchr(note, ' ');
      if (!sp) { note[0] = 0; break; }
      *sp = 0;
      char* w = strrchr(note, ' ');         // a separator left at the end ("·", "-", "–") goes too
      w = w ? w + 1 : note;
      bool sep = true;
      for (const uint8_t* c = (const uint8_t*)w; *c; c++)
        if (isalnum(*c) || (*c >= 0xC3 && !(*c == 0xE2 && (c[1] == 0x80 || c[1] == 0x81)))) sep = false;
      if (sep) {
        *w = 0;
        for (char* e = w; e > note && e[-1] == ' '; ) *--e = 0;
      }
    }
    if (note[0]) textLine(note, tall ? 4 : 0, 2, 0);
    gTcText = false;
  }
  display.drawFastHLine(0, rule, SCREEN_W, SH110X_WHITE);
}

// Copy UTF-8 without leaving half a character at the end
static void utf8Copy(char* dst, const char* src, size_t n) {
  strlcpy(dst, src, n);
  size_t L = strlen(dst);
  if (L == strlen(src)) return;
  size_t i = L;                             // truncated: drop a partial last character
  while (i > 0 && ((uint8_t)dst[i - 1] & 0xC0) == 0x80) i--;
  if (i > 0 && (uint8_t)dst[i - 1] >= 0xC0) dst[i - 1] = 0;
}

// Characters the CJK font lacks get close substitutes, so nothing shows as a gap
// (e.g. the Japanese font has no ellipsis or curly quotes). Own font state: core 1 only.
static void fitToFont(const char* in, char* out, size_t n, const uint8_t* font) {
  static u8g2_font_t chk = {};
  u8g2_SetFont(&chk, font);
  const uint8_t* p = (const uint8_t*)in;
  size_t o = 0;
  while (*p && o + 5 < n) {
    const uint8_t* start = p;
    uint32_t cp = utf8Next(p);
    size_t L = p - start;
    if (cp < 0x80 || (!(cp & 0x80000000u) && cp <= 0xFFFF && u8g2_IsGlyph(&chk, (uint16_t)cp))) {
      memcpy(out + o, start, L);
      o += L;
      continue;
    }
    const char* sub = cp == 0x2026 ? "..." : (cp == 0x2013 || cp == 0x2014 || cp == 0x2015) ? "-"
                    : (cp == 0x201C || cp == 0x201D) ? "\"" : (cp == 0x2018 || cp == 0x2019) ? "'" : "";
    for (; *sub && o + 1 < n; sub++) out[o++] = *sub;
  }
  out[o] = 0;
}

static void cjkTextLine(const char* s0, int y, uint8_t align, uint32_t elapsed) {
  char s[200];
  fitToFont(s0, s, sizeof(s), cjkFont());
  u8f.setFont(cjkFont());
  int base = min(y + 8, SCREEN_H - cjkBottom());   // sits on the same bottom as an 8 px line
  if (base - cjkTop() < 0) base = cjkTop(); // ...but never above the top of the screen
  int w = u8f.getUTF8Width(s), x;
  if (w <= SCREEN_W) {
    x = align == 0 ? 0 : align == 1 ? (SCREEN_W - w) / 2 : SCREEN_W - w;
  } else {
    int over = w - SCREEN_W;
    int sc = elapsed > 800 ? (int)((elapsed - 800) / 30) : 0;
    x = -min(over, sc);
  }
  u8f.drawUTF8(x, base, s);
}

static void textLine(const char* s, int y, uint8_t align, uint32_t elapsed) {
  bool plain = true;
  for (const char* c = s; *c; c++) if ((uint8_t)*c >= 0x80) { plain = false; break; }
  char conv[200];
  if (!plain) {
    if (hasCjk(s)) { cjkTextLine(s, y, align, elapsed); return; }
    toCp437(s, conv, sizeof(conv));         // accented Latin: the screen's own accented letters
    s = conv;
  }
  int w = strlen(s) * 6;
  int x;
  if (w <= SCREEN_W) {
    x = align == 0 ? 0 : align == 1 ? (SCREEN_W - w) / 2 : SCREEN_W - w;
  } else {
    int over = w - SCREEN_W;
    int sc = elapsed > 800 ? (int)((elapsed - 800) / 30) : 0;
    x = -min(over, sc);
  }
  display.setTextSize(1);
  display.setCursor(x, y);
  display.print(s);
}

static void toastFor(const char* msg, uint32_t ms) {
  strlcpy(gToast, msg, sizeof(gToast));
  gToastUntil = millis() + ms;
}

static void toast(const char* msg) { toastFor(msg, 1300); }

static void announce(const char* msg, uint32_t ms) {
  strlcpy(gTicker, msg, sizeof(gTicker));
  gTickerStart = millis();
  gTickerUntil = gTickerStart + ms;
}

static void drawToast() {
  int w = textWidthPx(gToast) + 12;
  if (w > 124) w = 124;
  int x = (SCREEN_W - w) / 2;
  display.fillRoundRect(x, 22, w, 20, 5, SH110X_BLACK);
  display.drawRoundRect(x, 22, w, 20, 5, SH110X_WHITE);
  textLine(gToast, 28, 1, 0);
}

// ============================ SOUND ==================================
// Short original motifs. Frequencies in Hz.
static const Note SND_CLICK[]   = {{4000, 8}};
static const Note SND_BOOT[]    = {{1047, 70}, {1319, 70}, {1568, 120}};
static const Note SND_PAT[]     = {{1568, 50}, {0, 20}, {2093, 50}, {0, 20}, {2637, 90}};
static const Note SND_ON[]      = {{1568, 60}, {2093, 90}};
static const Note SND_PAUSE[]   = {{1047, 120}};
static const Note SND_WAKE[]    = {{880, 60}, {1175, 60}, {1568, 60}, {2093, 120}};
static const Note SND_SAD[]     = {{1319, 170}, {1047, 170}, {880, 340}};
static const Note SND_HOUR[]    = {{2093, 70}, {0, 90}, {2093, 70}};
static const Note SND_DONE[]    = {{1047, 110}, {1319, 110}, {1568, 110}, {2093, 260}, {0, 80}, {1568, 90}, {2093, 340}};
static const Note SND_PRAYER[]  = {{784, 180}, {988, 180}, {1175, 380}};
static const Note SND_DIZZY[]   = {{1568, 60}, {1319, 60}, {1568, 60}, {1319, 60}, {1047, 140}};
static const Note SND_GO[]      = {{2637, 140}};
static const Note SND_COUGH[]   = {{330, 70}, {0, 60}, {294, 90}, {0, 120}, {262, 70}};
#define PLAY(s) playSound(s, sizeof(s) / sizeof((s)[0]))

static const Note* volatile sndReq = nullptr;   // handed from loop() to the sound task
static volatile uint8_t  sndReqLen = 0;
static volatile uint32_t sndReqId  = 0;

static void buzz(uint16_t hz) {
  if (!BUZZER_PASSIVE) { digitalWrite(BUZZER_PIN, hz ? HIGH : LOW); return; }
  uint32_t duty = hz ? (uint32_t)511 * BUZZER_VOLUME * BUZZER_VOLUME / 100 : 0;   // 10-bit PWM
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  if (hz) ledcWriteTone(BUZZER_PIN, hz);
  ledcWrite(BUZZER_PIN, duty);
#else
  if (hz) ledcWriteTone(0, hz);
  ledcWrite(0, duty);
#endif
}

static void buzzerBegin() {
  if (BUZZER_PASSIVE) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(BUZZER_PIN, 2000, 10);
#else
    ledcSetup(0, 2000, 10);
    ledcAttachPin(BUZZER_PIN, 0);
#endif
  } else {
    pinMode(BUZZER_PIN, OUTPUT);
  }
  buzz(0);
}

// Plays notes with precise timing, independent of the ~25 ms display frames.
// A new request interrupts the current sound.
static void soundTask(void*) {
  uint32_t seen = 0;
  for (;;) {
    if (sndReqId == seen) { vTaskDelay(pdMS_TO_TICKS(5)); continue; }
    seen = sndReqId;
    const Note* notes = sndReq;
    uint8_t n = sndReqLen;
    for (uint8_t i = 0; i < n && sndReqId == seen; i++) {
      uint16_t ms = notes[i].ms, on = ms > 30 ? ms - 12 : ms;   // short gap articulates notes
      buzz(notes[i].hz);
      vTaskDelay(pdMS_TO_TICKS(on));
      buzz(0);
      if (on < ms) vTaskDelay(pdMS_TO_TICKS(ms - on));
    }
    buzz(0);
  }
}

static void playSound(const Note* notes, uint8_t n) {
  // Silent when muted, all through sleep hours (do not disturb, even after a touch), and while a
  // focus session runs. The session's completion sound still plays: the state is already idle then.
  if (gMuted || gNight || fState == F_RUN) return;
  sndReq = notes;
  sndReqLen = n;
  sndReqId = sndReqId + 1;
}

// ============================ TIMERS =================================
// "Until" deadlines are 0 once passed (the sweep clears them), so after millis() wraps round
// an old deadline can't look new again
static bool notYet(uint32_t now, uint32_t t) { return t && (int32_t)(t - now) > 0; }

static void sweepTimers(uint32_t now) {    // once a second, from loop()
  uint32_t* until[] = {&gToastUntil, &gFlashUntil, &gTickerUntil, &gNightAwakeUntil, &B.ovUntil, &gGame.moodUntil};
  for (uint32_t* t : until)
    if (!notYet(now, *t)) *t = 0;
  uint32_t* next[] = {&B.nextAntic, &B.nextBlink, &B.nextGaze};   // due but not looked at: stay due
  for (uint32_t* t : next)
    if ((int32_t)(now - *t) > 0) *t = now;
}

// ============================ BUDDY ==================================
static void setMood(Mood m, uint32_t ms) {
  B.ov = m;
  B.ovUntil = millis() + ms;
}

static Mood currentMood(uint32_t now, bool sleeping) {
  if (notYet(now, B.ovUntil)) return B.ov;
  if (sleeping) return M_SLEEPY;
  if (!gWifiUp && !gPlannedHop && (WIFI_SSID[0] || WIFI2_SSID[0]) && now > 30000) return M_SAD;   // only if WiFi was set up
  const int aqi = getWeather().aqi;         // bad air weighs on Momowo: tired eyes, then sad ones
  if ((int32_t)(now - B.nextAntic) >= 0) {                 // little random expressions
    B.nextAntic = now + random(12000, 30000);
    if (aqi > 100 && random(100) < (aqi > 200 ? 80 : aqi > 150 ? 65 : 40))
      setMood(random(100) < 70 ? M_SAD : M_SLEEPY, 2500);
    else if (gFeel.valid && gFeel.mood != M_NORMAL && random(100) < 60)   // lean towards its feeling
      setMood(gFeel.mood, gFeel.mood == M_SURPRISED ? 600 : 1600);
    else if (random(100) < 60) setMood(M_HAPPY, 1200);
    else setMood(M_SURPRISED, 600);
    return B.ov;
  }
  if (aqi > 200) return M_SAD;              // very unhealthy air and worse
  if (aqi > 150) return M_SLEEPY;           // unhealthy: heavy-lidded
  return M_NORMAL;
}

static float blinkOpen(uint32_t now) {
  const uint32_t BL = 160;
  if ((int32_t)(now - B.nextBlink) >= 0) {
    B.blinkAt = now;
    B.nextBlink = now + (B.doubleBlink ? 250 : random(2000, 6000));
    B.doubleBlink = !B.doubleBlink && random(100) < 20;
  }
  uint32_t dt = now - B.blinkAt;
  if (dt >= BL) return 1.0f;
  return fabsf(1.0f - 2.0f * dt / (float)BL);
}

static void drawHeart(int cx, int cy, int r) {
  display.fillCircle(cx - r, cy - r / 2, r, SH110X_WHITE);
  display.fillCircle(cx + r, cy - r / 2, r, SH110X_WHITE);
  display.fillTriangle(cx - 2 * r, cy - r / 2, cx + 2 * r, cy - r / 2, cx, cy + 2 * r, SH110X_WHITE);
}

// Dizzy: a spinning spiral in each eye
static void drawSpiral(int cx, int cy, uint32_t now, bool left) {
  float spin = (left ? 1 : -1) * now / 120.0f;
  int px = cx, py = cy;
  for (int i = 1; i <= 40; i++) {
    float a = spin + i * 0.42f, r = i * 0.33f;
    int x = cx + (int)(cosf(a) * r), y = cy + (int)(sinf(a) * r);
    display.drawLine(px, py, x, y, SH110X_WHITE);
    px = x; py = y;
  }
}

static void drawEye(int cx, int cy, int w, int h, Mood m, bool left) {
  if (h <= 3) {                             // closed / mid-blink
    display.fillRect(cx - w / 2, cy - 1, w, 2, SH110X_WHITE);
    return;
  }
  int top = cy - h / 2;
  int r = min(8, h / 2);
  display.fillRoundRect(cx - w / 2, top, w, h, r, SH110X_WHITE);

  switch (m) {
    case M_HAPPY:                           // ^ ^ crescent eyes
      display.fillCircle(cx, cy + h / 2 + 1, w / 2 + 1, SH110X_BLACK);
      break;
    case M_SLEEPY:                          // heavy eyelids
      display.fillRect(cx - w / 2 - 1, top - 1, w + 2, h / 2 + 1, SH110X_BLACK);
      break;
    case M_SAD: {                           // droopy outer corners
      int l = cx - w / 2 - 1, rr = cx + w / 2 + 1, tp = top - 1;
      if (left) display.fillTriangle(l, tp, rr, tp, l, top + h / 2, SH110X_BLACK);
      else      display.fillTriangle(rr, tp, l, tp, rr, top + h / 2, SH110X_BLACK);
    } break;
    case M_SURPRISED:                       // sparkle
      display.fillCircle(cx + w / 4, top + h / 4, 2, SH110X_BLACK);
      break;
    default: break;
  }
}

static void drawBuddy(uint32_t now, Mood m) {
  // Gaze wandering (doubles as OLED burn-in protection)
  if (m == M_SLEEPY) { B.tgx = 0; B.tgy = 4; }
  else if (gMotion && MOTION_ON && gMs.refSet && gMs.tiltDeg > 12) {   // tilted: eyes roll downhill
    B.tgx = gMs.tiltX * 16 * (MOTION_MIRROR ? -1 : 1);
    B.tgy = gMs.tiltY * 8;
    B.nextGaze = now + 1500;
  } else if ((int32_t)(now - B.nextGaze) >= 0) {
    B.nextGaze = now + random(1200, 4000);
    if (random(100) < 35) { B.tgx = 0; B.tgy = 0; }
    else { B.tgx = random(-12, 13); B.tgy = random(-6, 7); }
  }
  B.gx += (B.tgx - B.gx) * 0.25f;
  B.gy += (B.tgy - B.gy) * 0.25f;

  float targetH = 26;
  if (m == M_SURPRISED) targetH = 34;
  else if (m == M_SLEEPY) targetH = 9 + 3 * sinf(now / 900.0f);   // breathing
  B.h += (targetH - B.h) * 0.3f;

  static int8_t ox = 0, oy = 0;             // the whole face drifts a little every few minutes,
  static uint32_t driftAt = 0;              // so the eyes don't wear the same OLED pixels all day
  if (now - driftAt > 180000) { driftAt = now; ox = random(-8, 9); oy = random(-3, 4); }
  int cy  = 26 + oy + (int)B.gy;
  int lcx = SCREEN_W / 2 - 22 + ox + (int)B.gx;
  int rcx = SCREEN_W / 2 + 22 + ox + (int)B.gx;

  if (B.heartsAt && now - B.heartsAt < 1800) {   // hearts drifting up after a pat
    uint32_t e = now - B.heartsAt;
    for (int i = 0; i < 3; i++) {
      int32_t ei = (int32_t)e - i * 250;
      if (ei < 0) continue;
      int x = (i == 1 ? SCREEN_W / 2 : i == 0 ? 14 : SCREEN_W - 14) + (int)(sinf(ei / 180.0f + i) * 4);
      int y = 52 - ei / 30;
      if (y > 2) drawHeart(x, y, 2);
    }
  }
  if (m == M_DIZZY) {
    drawSpiral(lcx, cy, now, true);
    drawSpiral(rcx, cy, now, false);
    return;
  }

  if (m == M_LOVE) {
    int r = 6 + (int)((now / 250) % 2);
    drawHeart(lcx, cy, r);
    drawHeart(rcx, cy, r);
    return;
  }

  float open  = (m == M_SLEEPY) ? 1.0f : blinkOpen(now);
  float persp = B.gx / 60.0f;               // eye nearer the gaze grows a bit
  int w  = (m == M_SURPRISED) ? 30 : 28;
  int lh = (int)(B.h * open * (1.0f - persp));
  int rh = (int)(B.h * open * (1.0f + persp));
  drawEye(lcx, cy, w, lh, m, true);
  drawEye(rcx, cy, w, rh, m, false);

  if (m == M_SLEEPY) {                      // floating Zzz
    display.setTextSize(1);
    uint32_t ph = now % 3000;
    for (int i = 0; i < 3; i++) {
      uint32_t p = (ph + i * 1000) % 3000;
      display.setCursor(100 + i * 6 + p / 400, 20 - p / 150);
      display.print(i == 2 ? 'Z' : 'z');
    }
  }
}

// The next prayer (not sunrise) after nowMin: its index and minutes after midnight (+1440 = tomorrow)
static bool nextPrayer(int nowMin, int& idx, int& at) {
  if (!gPray.valid) return false;
  static const uint8_t ORDER[] = {astro::FAJR, astro::DHUHR, astro::ASR, astro::MAGHRIB, astro::ISHA};
  const int isha = isnan(gPray.t[astro::ISHA]) ? -1 : (int)lroundf(gPray.t[astro::ISHA]);
  const int mag = isnan(gPray.t[astro::MAGHRIB]) ? -1 : (int)lroundf(gPray.t[astro::MAGHRIB]);
  const bool ishaLate = isha >= 0 && mag >= 0 && isha < mag;   // Isha after midnight (summer, far north)
  if (ishaLate && nowMin < isha) { idx = astro::ISHA; at = isha; return true; }   // last evening's, still to come
  for (uint8_t i : ORDER) {
    if (isnan(gPray.t[i])) continue;
    int v = (int)lroundf(gPray.t[i]);
    if (i == astro::ISHA && ishaLate) v += 1440;
    if (v > nowMin) { idx = i; at = v; return true; }
  }
  if (isnan(gPray.tmFajr)) return false;
  idx = astro::FAJR;
  at = (int)lroundf(gPray.tmFajr) + 1440;
  return true;
}

static void drawBuddyTicker(uint32_t now) {
  if (notYet(now, gTickerUntil)) {
    textLine(gTicker, 56, 1, now - gTickerStart);
    return;
  }
  static char msgs[20][160];                // static: keeps loop()'s stack small
  int n = 0;
  struct tm t;
  bool tv = nowTm(t);
  WeatherData w = getWeather();

  if (fState == F_RUN || fState == F_BREAK) {
    uint32_t s = (fRemain + 999) / 1000;
    snprintf(msgs[n++], sizeof(msgs[0]), T(fState == F_RUN ? S_FOCUS_LEFT : S_BREAK_LEFT),
             (unsigned)(s / 60), (unsigned)(s % 60));
  }
  if (tv) {
    greetingLine(t.tm_hour, msgs[n++], sizeof(msgs[0]));
    if (AI_FEELINGS && AI_API_KEY[0]) {   // straight after the greeting: Momowo's feeling
      if (gFeel.valid && gFeel.thought[0] && xSemaphoreTake(gMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        snprintf(msgs[n++], sizeof(msgs[0]), "%s", gFeel.thought);
        xSemaphoreGive(gMutex);
      }
      if (gAiStatus[0]) snprintf(msgs[n++], sizeof(msgs[0]), "%s", gAiStatus);         // why it isn't working
      else if (!gFeel.valid && gWifiUp && !gNight) snprintf(msgs[n++], sizeof(msgs[0]), "%s", THINKING[LANG]);
    }
    const char* hol = calGreeting(gCal);
    if (hol) snprintf(msgs[n++], sizeof(msgs[0]), "%s", hol);
    if (!gCal.holToday[0] && gCal.holDays > 0 && gCal.holDays <= 7) holidayLine(gCal, msgs[n++], sizeof(msgs[0]));
    if ((CALS & CAL_JEWISH) && gCal.candles >= 0 && !gCal.shabbat) {   // Friday: Shabbat candles
      char hm[10];
      fmtMinutes(gCal.candles, hm, sizeof(hm));
      snprintf(msgs[n++], sizeof(msgs[0]), T(S_CANDLES), hm);
    }
    int idx, at, nowMin = t.tm_hour * 60 + t.tm_min;
    if ((CALS & CAL_ISLAMIC) && nextPrayer(nowMin, idx, at)) {
      char hm[10];
      fmtMinutes(at % 1440, hm, sizeof(hm));
      if (gCal.hm == 9 && idx == astro::MAGHRIB) {        // Ramadan: count down to iftar
        int left = at - nowMin;
        snprintf(msgs[n++], sizeof(msgs[0]), T(S_IFTAR_IN), left / 60, left % 60);
      }
      snprintf(msgs[n++], sizeof(msgs[0]), T(S_NEXT_PRAYER), PRAYER_NAME[LANG][idx], hm);
    }
    char ts[8];
    fmtTime(t, ts, sizeof(ts));
    if (w.valid) snprintf(msgs[n++], 40, "%s  %d" DEG "C", ts, (int)lroundf(w.temp));
    else         snprintf(msgs[n++], sizeof(msgs[0]), T(S_ITS_TIME), ts);
  }
  if (w.valid) {
    if (w.rainChance >= 50)  snprintf(msgs[n++], sizeof(msgs[0]), T(S_RAIN_ADVICE), w.rainChance);
    else if (w.tmax >= 30)   snprintf(msgs[n++], sizeof(msgs[0]), "%s", T(S_HOT));
    else if (w.tmin <= 8)    snprintf(msgs[n++], sizeof(msgs[0]), "%s", T(S_CHILLY));
    else if (w.code <= 1 && w.isDay) snprintf(msgs[n++], sizeof(msgs[0]), "%s", T(S_LOVELY));
    if (w.aqi > 100) snprintf(msgs[n++], sizeof(msgs[0]), T(S_AQI_ADVICE), w.aqi);
    if (tv && w.sunriseMin >= 0 && w.sunsetMin >= 0) {    // the next sunrise or sunset
      int nowMin = t.tm_hour * 60 + t.tm_min;
      char st[10];
      if (nowMin < w.sunriseMin) { fmtMinutes(w.sunriseMin, st, sizeof(st)); snprintf(msgs[n++], sizeof(msgs[0]), T(S_SUNRISE_AT), st); }
      else if (nowMin < w.sunsetMin) { fmtMinutes(w.sunsetMin, st, sizeof(st)); snprintf(msgs[n++], sizeof(msgs[0]), T(S_SUNSET_AT), st); }
      else if (w.tmSunrise >= 0) { fmtMinutes(w.tmSunrise, st, sizeof(st)); snprintf(msgs[n++], sizeof(msgs[0]), T(S_SUNRISE_TMR), st); }
    }
    if (tv && t.tm_hour >= 17 && w.tmValid)               // evenings: a look at tomorrow
      snprintf(msgs[n++], sizeof(msgs[0]), T(S_TOMORROW_FC), (int)lroundf(w.tmMax),
               (int)lroundf(w.tmMin), condLabel(w.tmCode));
  }
  bool wifiSet = WIFI_SSID[0] || WIFI2_SSID[0];   // WiFi is optional: only mention it if set up
  if (wifiSet && !gWifiUp) {
    const char* why = wifiReasonText(gWifiReason);
    if (why && now > 20000) snprintf(msgs[n++], sizeof(msgs[0]), T(S_WIFI_REASON), why);
    else snprintf(msgs[n++], sizeof(msgs[0]), "%s", now > 30000 ? T(S_WIFI_NOT) : T(S_WIFI_CONNECTING));
    if (now > 60000) snprintf(msgs[n++], sizeof(msgs[0]), "%s", T(S_HOLD_FIX_WIFI));
  } else if (gWifiUp && gOnHotspot) {
    snprintf(msgs[n++], sizeof(msgs[0]), "%s", T(S_BACKUP_WIFI));
  }
  if (!tv) snprintf(msgs[n++], sizeof(msgs[0]), "%s", T(S_HOLD_SET_CLOCK));
  if (gWifiUp && now < 10UL * 60 * 1000) {  // first 10 min online: where the phone settings are
    snprintf(msgs[n++], sizeof(msgs[0]), T(S_SETTINGS_AT), gHost);
    snprintf(msgs[n++], sizeof(msgs[0]), T(S_OR_HTTP), WiFi.localIP().toString().c_str());
  }
  if (n == 0)   snprintf(msgs[n++], sizeof(msgs[0]), "%s", T(S_TAP_ME));

  uint32_t dur[20], total = 0;              // 5 s each, plus time to slide long ones across
  for (int i = 0; i < n; i++) {
    int over = textWidthPx(msgs[i]) - SCREEN_W;   // real width, any language
    dur[i] = 5000 + (over > 0 ? (uint32_t)over * 30 + 1500 : 0);
    total += dur[i];
  }
  uint32_t ph = now % total;
  int i = 0;
  while (ph >= dur[i]) { ph -= dur[i]; i++; }
  textLine(msgs[i], 56, 1, ph);
}

// Clock and calendar before the time is known
static void drawNoTime() {
  static uint32_t onlineAt = 0;             // online but still no time after 2 min: how to set it
  if (!gWifiUp) onlineAt = 0;
  else if (!onlineAt) onlineAt = millis() | 1;
  if (gWifiUp && millis() - onlineAt < 120000) { textLine(T(S_SYNCING), 28, 1, 0); return; }
  bool tall = langIsCjk();                  // taller Chinese/Japanese lines need more room
  textLine(T(S_SET_CLOCK1), tall ? 5 : 14, 1, 0);
  textLine(T(S_SET_CLOCK2), tall ? 21 : 28, 1, 0);
  textLine(T(S_SET_CLOCK3), tall ? 37 : 40, 1, 0);
}

// ============================ CLOCK PAGE =============================
static void drawPrayerTimes(uint32_t el);
static void drawFasting(uint32_t now);

// The big clock; el: time since this view appeared
static void drawClockFace(uint32_t el) {
  struct tm t;
  if (!nowTm(t)) {
    drawNoTime();
    return;
  }
  char buf[40];
  greetingLine(t.tm_hour, buf, sizeof(buf));
  textLine(buf, 0, 1, el);

  char hm[8];
  fmtTime(t, hm, sizeof(hm));
  int len = strlen(hm);
  const char* ap = t.tm_hour < 12 ? T(S_AM) : T(S_PM);
  int sfx = USE_24H ? 12 : max(12, textWidthPx(ap));   // "上午" is wider than "am"
  int x = (SCREEN_W - (len * 18 + 2 + sfx)) / 2;
  bool tall = langIsCjk();                  // taller greeting and date: time sits a little lower
  int ty = tall ? 17 : 14;
  display.setTextSize(3);
  display.setCursor(x, ty);
  display.print(hm);
  display.setTextSize(1);
  if (!USE_24H) textAt(ap, x + len * 18 + 2, tall ? 22 : 14);
  snprintf(buf, sizeof(buf), "%02d", t.tm_sec);
  display.setCursor(x + len * 18 + 2, tall ? 33 : 30);
  display.print(buf);

  char date[96];
  fmtDate(t, date, sizeof(date), false);
  LocationData loc = getLocation();         // every third turn: where this time is from
  uint32_t turn = (el / 5000) % 3;
  if (turn == 2 && (loc.name[0] || gTzKnown)) {
    int32_t off = gTzOffset;
    char tz[16];
    if (off % 3600) snprintf(tz, sizeof(tz), "UTC%+d:%02d", (int)(off / 3600), (int)(abs(off) % 3600 / 60));
    else snprintf(tz, sizeof(tz), "UTC%+d", (int)(off / 3600));
    if (loc.name[0]) snprintf(date, sizeof(date), "%s  %s", loc.name, tz);
    else strlcpy(date, tz, sizeof(date));
  }
  textLine(date, tall ? 49 : 44, 1, el % 5000);
  display.fillRect(0, 61, (t.tm_sec + 1) * SCREEN_W / 60, 3, SH110X_WHITE);
}

// Clock page: the clock for 12 s, then with the Islamic calendar on the prayer times for 8 s, and
// during Ramadan the fasting times for 6 s
static void drawClockPage(uint32_t now) {
  uint32_t el = now - gPageEnter;
  if ((CALS & CAL_ISLAMIC) && gPray.valid) {
    const uint32_t CLOCK = 12000, PRAY = 8000, FAST = 6000;
    const bool ramadan = gCal.valid && gCal.hm == 9;
    uint32_t c = el % (CLOCK + PRAY + (ramadan ? FAST : 0));
    if (c >= CLOCK + PRAY) { drawFasting(now); return; }
    if (c >= CLOCK) { drawPrayerTimes(c - CLOCK); return; }
    el = c;
  }
  drawClockFace(el);
}

// ============================ CALENDAR PAGE ==========================
// The Gregorian month as a grid, then today in each calendar the owner chose, 6 s per view
static void drawMonthGrid(const struct tm& t) {
  int y = t.tm_year + 1900, m = t.tm_mon + 1;
  char title[48];
  bool cjk = langIsCjk();
  if (cjk) {                                // "2026年10月" in the tiny 8 px font
    snprintf(title, sizeof(title), "%d年%d月", y, m);
    u8f.setFont(u8g2_font_fusion8_wd);
    u8f.drawUTF8((SCREEN_W - u8f.getUTF8Width(title)) / 2, 6, title);   // rows 0-6 (年 and 月 reach 7)
  } else {
    snprintf(title, sizeof(title), "%s %d", MONTH_LONG[LANG][m - 1], y);
    textLine(title, 0, 1, 0);
  }
  const int COLW = 18, X0 = 1, HDR_Y = 9, ROW0 = 18, PITCH = 7;
  for (int i = 0; i < 7; i++) {             // weekday initials, Sunday first
    const char* wd = WD_MIN[LANG][i];
    if (hasCjk(wd)) {
      u8f.setFont(u8g2_font_fusion8_wd);
      // baseline 14: the 7-row characters fill rows 8-14, clear of the rule under them at row 16
      u8f.drawUTF8(X0 + i * COLW + (COLW - u8f.getUTF8Width(wd)) / 2, HDR_Y + 5, wd);
    } else {
      textAt(wd, X0 + i * COLW + (COLW - textWidthPx(wd)) / 2, HDR_Y);
    }
  }
  display.drawFastHLine(0, ROW0 - 2, SCREEN_W, SH110X_WHITE);
  int first = (jdn(y, m, 1) + 1) % 7;       // weekday of the 1st (0 = Sunday)
  int days = jdn(m == 12 ? y + 1 : y, m == 12 ? 1 : m + 1, 1) - jdn(y, m, 1);
  display.setTextSize(1);
  for (int d = 1; d <= days; d++) {
    int cell = first + d - 1, col = cell % 7, row = cell / 7;
    int x = X0 + col * COLW + (d < 10 ? 6 : 3), yy = ROW0 + row * PITCH;
    char num[12];
    snprintf(num, sizeof(num), "%d", d);
    if (d == t.tm_mday) {                   // today: white box, black number
      display.fillRect(X0 + col * COLW + 1, yy - 1, COLW - 2, PITCH + 1, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
      display.setCursor(x, yy);
      display.print(num);
      display.setTextColor(SH110X_WHITE);
    } else {
      if (gCal.holMask >> d & 1)            // public holiday: an outline box
        display.drawRect(X0 + col * COLW + 1, yy - 1, COLW - 2, PITCH + 1, SH110X_WHITE);
      display.setCursor(x, yy);
      display.print(num);
    }
  }
}

// "Public holiday: National Day", "Tomorrow: Deepavali" or "Deepavali in 5 days"
static void holidayLine(const CalInfo& c, char* buf, size_t n) {
  if (c.holToday[0]) snprintf(buf, n, T(S_HOL_TODAY), c.holToday);
  else if (c.holDays == 1) snprintf(buf, n, T(S_HOL_TMR), c.holNext);
  else snprintf(buf, n, T(S_HOL_IN), c.holNext, c.holDays);
}

static int calRowHeight(const CalRow& r) {
  if (r.lunar) return 17;
  return (hasCjk(r.text) || hasCjk(r.alt)) ? cjkTop() + cjkBottom() + 2 : 10;
}

static void drawCalendarPage(uint32_t now) {
  const CalInfo& c = gCal;
  uint32_t el = now - gPageEnter;
  if (!c.valid) {
    drawNoTime();
    return;
  }
  struct tm t;
  nowTm(t);                                 // valid: gCal.valid needs the time

  // the rows for today, in the order the owner reads them
  static CalRow rows[8];
  int n = 0;
  auto add = [&](const char* text, const char* alt, bool lunar) {
    if (n >= 8) return;
    utf8Copy(rows[n].text, text, sizeof(rows[n].text));
    utf8Copy(rows[n].alt, alt ? alt : "", sizeof(rows[n].alt));
    rows[n].lunar = lunar;
    n++;
  };
  char buf[72], alt[72];
  fmtDate(t, buf, sizeof(buf), true);
  const char* cn = c.christian;             // Gregorian date, or Christmas / Easter
  add(buf, !cn ? nullptr : !strcmp(cn, "Christmas Eve") ? T(S_XMAS_EVE) : !strcmp(cn, "Christmas Day") ? T(S_XMAS_DAY)
           : !strcmp(cn, "Good Friday") ? T(S_GOOD_FRIDAY) : T(S_EASTER), false);
  if (c.holToday[0] || (c.holDays > 0 && c.holDays <= 7)) {   // public holiday today, or within a week
    holidayLine(c, buf, sizeof(buf));
    add(buf, nullptr, false);
  }
  if (CALS & CAL_CHINESE) {
    if (c.lunarOk) {                        // 丙午马年 八月十九, then festival / 节气 / "9天后寒露"
      char day[16];
      cnDay(c.ld, day, sizeof(day));
      int s = (c.ly - 4) % 10, b = (c.ly - 4) % 12;
      snprintf(buf, sizeof(buf), "%s%s%s年 %s%s月%s", CN_STEM[s], CN_BRANCH[b], CN_ZODIAC[b],
               c.leap ? "闰" : "", CN_MONTH[c.lm - 1], day);
      u8f.setFont(CJK_FONT_BIG);
      if (u8f.getUTF8Width(buf) > SCREEN_W)   // drop the space if that makes it fit
        snprintf(buf, sizeof(buf), "%s%s%s年%s%s月%s", CN_STEM[s], CN_BRANCH[b], CN_ZODIAC[b],
                 c.leap ? "闰" : "", CN_MONTH[c.lm - 1], day);
      add(buf, nullptr, true);
      lunarDayText(c, buf, sizeof(buf));
      if (buf[0]) add(buf, nullptr, true);
    } else {
      add(T(S_LUNAR_RANGE), nullptr, false);
    }
  }
  if (CALS & CAL_ISLAMIC) {                 // Hijri date, or an Islamic day
    if (langIsCjk()) snprintf(buf, sizeof(buf), T(S_HIJRI_CJK), c.hy, c.hm, c.hd);
    else snprintf(buf, sizeof(buf), "%d %s %d H", c.hd, HIJRI_MONTH[LANG][c.hm - 1], c.hy);
    add(buf, hijriEvent(c), false);
  }
  if (CALS & CAL_BUDDHIST) {                // Buddhist year, or a Buddhist day
    snprintf(buf, sizeof(buf), T(S_BE_FMT), c.be);
    add(buf, buddhistDay(c, true), false);
  }
  if ((CALS & CAL_HINDU) && c.hinduOk) {    // e.g. "Kartika Shukla 7, VS 2083", or a festival
    int tt = c.pj.tithi;
    snprintf(buf, sizeof(buf), T(S_HINDU_FMT), c.pj.adhika ? "Adhika " : "", astro::HINDU_MONTH[c.pj.month],
             tt <= 15 ? T(S_SHUKLA) : T(S_KRISHNA), tt <= 15 ? tt : tt - 15, c.pj.vikram);
    alt[0] = 0;
    if (c.hinduFest) strlcpy(alt, c.hinduFest, sizeof(alt));
    add(buf, alt, false);
  }
  if ((CALS & CAL_JEWISH) && c.hebOk) {     // e.g. "7 Cheshvan 5787", with a festival or Shabbat
    const char* mon = HEBREW_MONTH[LANG][c.hebM == 12 && astro::hebLeap(c.hebY) ? 0 : c.hebM];
    if (langIsCjk()) snprintf(buf, sizeof(buf), T(S_HEBREW_CJK), (int)c.hebY, mon, c.hebD);
    else snprintf(buf, sizeof(buf), "%d %s %d", c.hebD, mon, (int)c.hebY);
    hebFestText(c, alt, sizeof(alt));
    if (!alt[0] && c.shabbat) strlcpy(alt, T(S_G_SHABBAT), sizeof(alt));
    else if (!alt[0] && c.candles >= 0) {
      char hm[10];
      fmtMinutes(c.candles, hm, sizeof(hm));
      snprintf(alt, sizeof(alt), T(S_CANDLES), hm);
    }
    add(buf, alt, false);
  }

  // views: the month grid, then the rows in pages that fit the screen
  int pageStart[8], pages = 0, h = 0;
  for (int i = 0; i < n; i++) {
    int rh = calRowHeight(rows[i]);
    if (i == 0 || h + rh > SCREEN_H) { pageStart[pages++] = i; h = 0; }
    h += rh;
  }
  const uint32_t VIEW_MS = 6000;
  int view = (el / VIEW_MS) % (pages + 1);
  uint32_t ph = el % VIEW_MS;
  if (view == 0) { drawMonthGrid(t); return; }
  int from = pageStart[view - 1], to = view < pages ? pageStart[view] : n;
  int total = 0;
  for (int i = from; i < to; i++) total += calRowHeight(rows[i]);
  int y = (SCREEN_H - total) / 2;
  bool showAlt = (ph / 3000) % 2;           // special days alternate with the date every 3 s
  uint32_t ap = ph % 3000;
  for (int i = from; i < to; i++) {
    const CalRow& r = rows[i];
    int rh = calRowHeight(r);
    const char* s = (showAlt && r.alt[0]) ? r.alt : r.text;
    if (r.lunar) {
      u8f.setFont(CJK_FONT_BIG);
      cjkLine(s, y + 14, 1, ap);
    } else if (hasCjk(r.text) || hasCjk(r.alt)) {
      textLine(s, y + cjkTop() - 7, 1, ap);
    } else {
      textLine(s, y + 1, 1, ap);
    }
    y += rh;
  }
}

// ============================ WEATHER PAGE ===========================
static WIcon iconFor(int c) {
  if (c == 0) return W_CLEAR;
  if (c <= 2) return W_PARTLY;
  if (c == 3) return W_CLOUD;
  if (c == 45 || c == 48) return W_FOG;
  if (c >= 51 && c <= 57) return W_DRIZZLE;
  if ((c >= 61 && c <= 67) || (c >= 80 && c <= 82)) return W_RAIN;
  if ((c >= 71 && c <= 77) || c == 85 || c == 86) return W_SNOW;
  if (c >= 95) return W_STORM;
  return W_CLOUD;
}

// Weather condition for the screen, in the chosen language (condText() stays English for prompts)
static const char* condLabel(int c) {
  switch (c) {
    case 0: return T(S_C_CLEAR);
    case 1: return T(S_C_MAINLY);
    case 2: return T(S_C_PARTLY);
    case 3: return T(S_C_OVERCAST);
    case 45: case 48: return T(S_C_FOG);
    case 51: case 53: case 55: return T(S_C_DRIZZLE);
    case 56: case 57: return T(S_C_FDRIZZLE);
    case 61: return T(S_C_LRAIN);
    case 63: return T(S_C_RAIN);
    case 65: return T(S_C_HRAIN);
    case 66: case 67: return T(S_C_FRAIN);
    case 71: return T(S_C_LSNOW);
    case 73: return T(S_C_SNOW);
    case 75: return T(S_C_HSNOW);
    case 77: return T(S_C_GRAINS);
    case 80: return T(S_C_LSHOWERS);
    case 81: return T(S_C_SHOWERS);
    case 82: return T(S_C_HSHOWERS);
    case 85: case 86: return T(S_C_SSHOWERS);
    case 95: return T(S_C_THUNDER);
    case 96: case 99: return T(S_C_HAIL);
    default: return T(S_C_UNKNOWN);
  }
}

static const char* condText(int c) {
  switch (c) {
    case 0: return "Clear sky";
    case 1: return "Mainly clear";
    case 2: return "Partly cloudy";
    case 3: return "Overcast";
    case 45: case 48: return "Foggy";
    case 51: case 53: case 55: return "Drizzle";
    case 56: case 57: return "Freezing drizzle";
    case 61: return "Light rain";
    case 63: return "Rain";
    case 65: return "Heavy rain";
    case 66: case 67: return "Freezing rain";
    case 71: return "Light snow";
    case 73: return "Snow";
    case 75: return "Heavy snow";
    case 77: return "Snow grains";
    case 80: return "Light showers";
    case 81: return "Showers";
    case 82: return "Heavy showers";
    case 85: case 86: return "Snow showers";
    case 95: return "Thunderstorm";
    case 96: case 99: return "Storm with hail";
    default: return "Unknown";
  }
}

static void drawCloud(int cx, int cy, uint16_t col, int g) {
  display.fillCircle(cx - 7, cy + 1, 6 + g, col);
  display.fillCircle(cx + 1, cy - 3, 8 + g, col);
  display.fillCircle(cx + 8, cy + 2, 5 + g, col);
  display.fillRoundRect(cx - 13 - g, cy + 1 - g, 27 + 2 * g, 7 + 2 * g, 3, col);
}

static void drawSun(int cx, int cy, int r, uint32_t now) {
  display.fillCircle(cx, cy, r, SH110X_WHITE);
  float ph = now / 3000.0f;                 // slowly rotating rays
  for (int i = 0; i < 8; i++) {
    float a = ph + i * PI / 4;
    float ca = cosf(a), sa = sinf(a);
    display.drawLine(cx + (int)(ca * (r + 3)), cy + (int)(sa * (r + 3)),
                     cx + (int)(ca * (r + 6)), cy + (int)(sa * (r + 6)), SH110X_WHITE);
  }
}

static void drawMoon(int cx, int cy, int r, uint32_t now) {
  display.fillCircle(cx, cy, r, SH110X_WHITE);
  display.fillCircle(cx + r / 2 + 1, cy - r / 3, r - 1, SH110X_BLACK);
  if ((now / 700) % 2) display.drawPixel(cx + 11, cy - 9, SH110X_WHITE);
  else                 display.drawPixel(cx - 12, cy + 8, SH110X_WHITE);
  display.drawPixel(cx + 13, cy + 5, SH110X_WHITE);
}

static void drawWeatherIcon(WIcon ic, bool day, int cx, int cy, uint32_t now) {
  switch (ic) {
    case W_CLEAR:
      if (day) drawSun(cx, cy, 7, now); else drawMoon(cx, cy, 10, now);
      break;
    case W_PARTLY:
      if (day) drawSun(cx - 6, cy - 6, 5, now); else drawMoon(cx - 6, cy - 6, 7, now);
      drawCloud(cx + 3, cy + 4, SH110X_BLACK, 2);
      drawCloud(cx + 3, cy + 4, SH110X_WHITE, 0);
      break;
    case W_CLOUD:
      drawCloud(cx, cy, SH110X_WHITE, 0);
      break;
    case W_FOG:
      for (int i = 0; i < 4; i++)
        display.drawFastHLine(cx - 14 + (i % 2) * 4, cy - 8 + i * 5, 24, SH110X_WHITE);
      break;
    case W_DRIZZLE:
      drawCloud(cx, cy - 5, SH110X_WHITE, 0);
      for (int i = 0; i < 3; i++)
        display.fillRect(cx - 8 + i * 8, cy + 6 + (now / 120 + i * 3) % 8, 2, 2, SH110X_WHITE);
      break;
    case W_RAIN:
      drawCloud(cx, cy - 5, SH110X_WHITE, 0);
      for (int i = 0; i < 4; i++) {
        int x = cx - 9 + i * 6, yo = (now / 80 + i * 4) % 8;
        display.drawLine(x, cy + 5 + yo, x - 2, cy + 9 + yo, SH110X_WHITE);
      }
      break;
    case W_SNOW:
      drawCloud(cx, cy - 5, SH110X_WHITE, 0);
      for (int i = 0; i < 3; i++) {
        int x = cx - 8 + i * 8, y = cy + 7 + (now / 200 + i * 2) % 7;
        display.drawFastHLine(x - 1, y, 3, SH110X_WHITE);
        display.drawFastVLine(x, y - 1, 3, SH110X_WHITE);
      }
      break;
    case W_STORM:
      drawCloud(cx, cy - 5, SH110X_WHITE, 0);
      if ((now / 400) % 3) {                // flickering bolt
        display.fillTriangle(cx + 2, cy + 4, cx - 4, cy + 11, cx + 1, cy + 11, SH110X_WHITE);
        display.fillTriangle(cx - 1, cy + 10, cx + 4, cy + 10, cx - 3, cy + 17, SH110X_WHITE);
      }
      break;
  }
}

static void drawTomorrow(const WeatherData& w, uint32_t now, uint32_t el) {
  drawWeatherIcon(iconFor(w.tmCode), true, 18, 16, now);
  char buf[48];
  textAt(T(S_TOMORROW), 42, 2);
  snprintf(buf, sizeof(buf), "%d/%d", (int)lroundf(w.tmMax), (int)lroundf(w.tmMin));
  int ny = langIsCjk() ? 16 : 14;           // under a taller "tomorrow" label
  display.setTextSize(2);
  display.setCursor(42, ny);
  display.print(buf);
  display.setTextSize(1);
  display.setCursor(42 + strlen(buf) * 12, ny);
  display.print(DEG "C");
  char rain[48], wind[48];
  snprintf(rain, sizeof(rain), T(S_RAIN_CHANCE), w.tmRain);
  snprintf(wind, sizeof(wind), T(S_WIND_UPTO), (int)lroundf(w.tmWind));
  if (!langIsCjk()) {
    textLine(condLabel(w.tmCode), 38, 0, el);
    textLine(rain, 47, 0, el);
    textLine(wind, 56, 0, el);
  } else {                                  // taller lines (the icon reaches y=33): one bottom row,
    const char* row = el < 3000 ? condLabel(w.tmCode) : el < 4500 ? rain : wind;   // condition first
    textLine(row, 56, 0, el < 3000 ? el : (el - 3000) % 1500);
  }
}

// Small horizon icon: half sun with an up (sunrise) or down (sunset) arrow
static void drawSunEvent(int cx, int y, bool rise) {
  display.fillCircle(cx - 2, y + 10, 5, SH110X_WHITE);
  display.fillRect(cx - 8, y + 11, 13, 6, SH110X_BLACK);   // hide the half below the horizon
  display.drawFastHLine(cx - 9, y + 11, 15, SH110X_WHITE);
  display.drawFastVLine(cx + 7, y + 1, 9, SH110X_WHITE);   // arrow shaft
  if (rise) display.fillTriangle(cx + 4, y + 4, cx + 10, y + 4, cx + 7, y, SH110X_WHITE);
  else      display.fillTriangle(cx + 4, y + 6, cx + 10, y + 6, cx + 7, y + 10, SH110X_WHITE);
}

// Sunrise and sunset, today and tomorrow, with the daylight length
static void drawSunTimes(const WeatherData& w, uint32_t el) {
  const int RISE_X = 52, SET_X = 92;
  drawSunEvent(RISE_X + 17, 1, true);
  drawSunEvent(SET_X + 17, 1, false);
  char a[10], b[10], buf[64], buf2[64] = "";
  bool tall = langIsCjk();
  const int R1 = tall ? 17 : 20, R2 = tall ? 32 : 31;     // rows: today, tomorrow (below the icons)
  if (!tall) textLine(T(S_SUN), 5, 0, 0);   // taller scripts: the sun icons say it
  textAt(T(S_TODAY), 0, R1);
  fmtMinutes(w.sunriseMin, a, sizeof(a)); fmtMinutes(w.sunsetMin, b, sizeof(b));
  display.setCursor(RISE_X, R1); display.print(a);
  display.setCursor(SET_X, R1);  display.print(b);
  textAt(T(S_TOMORROW), 0, R2);
  fmtMinutes(w.tmSunrise, a, sizeof(a)); fmtMinutes(w.tmSunset, b, sizeof(b));
  display.setCursor(RISE_X, R2); display.print(a);
  display.setCursor(SET_X, R2);  display.print(b);
  display.drawFastHLine(0, tall ? 44 : 42, SCREEN_W, SH110X_WHITE);
  if (w.sunriseMin >= 0 && w.sunsetMin > w.sunriseMin) {
    int day = w.sunsetMin - w.sunriseMin;
    snprintf(buf, sizeof(buf), T(S_DAYLIGHT), day / 60, day % 60);
    if (w.tmSunrise >= 0 && w.tmSunset > w.tmSunrise) {
      int diff = (w.tmSunset - w.tmSunrise) - day;
      if (diff == 0) snprintf(buf2, sizeof(buf2), "%s", T(S_SAME_LENGTH));
      else snprintf(buf2, sizeof(buf2), T(S_MIN_TOMORROW), diff);
    }
    if (!tall) {
      textLine(buf, 46, 1, el);
      if (buf2[0]) textLine(buf2, 56, 1, el);
    } else {                                // one taller row: daylight, then tomorrow's change
      bool second = buf2[0] && (el / 3000) % 2;
      textLine(second ? buf2 : buf, 56, 1, el % 3000);
    }
  }
}

// The three days after tomorrow, side by side: weekday, icon, high/low, rain chance
static void drawNextDays(const WeatherData& w, uint32_t now) {
  struct tm t;
  bool tv = nowTm(t);
  for (int i = 0; i < 3; i++) {
    int d = i + 2, cx = 21 + i * 43;
    if (d >= w.days) break;
    if (i) display.drawFastVLine(cx - 22, 2, 60, SH110X_WHITE);
    const char* wd = tv ? WD_SHORT[LANG][(t.tm_wday + d) % 7] : "";
    int top = langIsCjk() ? 0 : 1;
    textAt(wd, cx - textWidthPx(wd) / 2, top);
    int iy = langIsCjk() ? 27 : 24;
    drawWeatherIcon(iconFor(w.dCode[d]), true, cx, iy, now);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d/%d", w.dMax[d], w.dMin[d]);
    display.setTextSize(1);
    display.setCursor(cx - strlen(buf) * 3, 46);
    display.print(buf);
    snprintf(buf, sizeof(buf), "%d%%", w.dRain[d]);
    display.setCursor(cx - strlen(buf) * 3, 56);
    display.print(buf);
  }
}

// 7x7 icons for the prayer times (astro order): Fajr a crescent above the horizon before dawn,
// sunrise the sun coming up, Dhuhr the midday sun, Asr a post casting its long afternoon shadow,
// Maghrib the sun going down, Isha the crescent and a star
static const uint8_t PRAYER_ICON[6][7] PROGMEM = {
  {0x70, 0xC0, 0xC0, 0xC0, 0x70, 0x00, 0xFE},   // Fajr
  {0x10, 0x28, 0x44, 0x38, 0x7C, 0x7C, 0xFE},   // sunrise
  {0x10, 0x44, 0x38, 0xBA, 0x38, 0x44, 0x10},   // Dhuhr
  {0xC0, 0xC0, 0x00, 0x08, 0x08, 0x08, 0x0E},   // Asr
  {0x44, 0x28, 0x10, 0x38, 0x7C, 0x7C, 0xFE},   // Maghrib
  {0x38, 0x62, 0xC7, 0xC2, 0xC0, 0x60, 0x38},   // Isha
};

// Prayer times, the next one highlighted, each with its icon: one column of six rows in Latin
// scripts; in Chinese and Japanese (taller glyphs) three rows at a time, 4 s each half
static void drawPrayerTimes(uint32_t el) {
  struct tm t;
  int nowMin = nowTm(t) ? t.tm_hour * 60 + t.tm_min : -1, nIdx = -1, nAt;
  if (nowMin >= 0 && nextPrayer(nowMin, nIdx, nAt) && nAt >= 1440 && nIdx != astro::ISHA) nIdx = -1;   // tomorrow's
  bool tall = langIsCjk();
  int rowH = tall ? 15 : 8, top = tall ? 17 : 13;
  LocationData loc = getLocation();
  char hdr[96];
  if (loc.name[0]) snprintf(hdr, sizeof(hdr), "%s %s", T(S_PRAYER_TIMES), loc.name);
  else strlcpy(hdr, T(S_PRAYER_TIMES), sizeof(hdr));
  const int half = (el / 4000) % 2;         // tall scripts: Fajr to Dhuhr, then Asr to Isha
  for (int i = 0; i < astro::PRAYER_COUNT; i++) {
    if (tall && i / 3 != half) continue;
    int row = tall ? i % 3 : i;
    int x = 0, y = top + row * rowH, w = SCREEN_W;   // y: top of the row
    char hm[10];
    fmtMinutes(isnan(gPray.t[i]) ? -1 : (int)lroundf(gPray.t[i]), hm, sizeof(hm));
    bool next = i == nIdx;
    if (next) display.fillRect(x, y, w, rowH, SH110X_WHITE);
    if (next) { display.setTextColor(SH110X_BLACK); u8f.setForegroundColor(SH110X_BLACK); }
    display.drawBitmap(x + 1, tall ? y + 3 : y, PRAYER_ICON[i], 8, 7, next ? SH110X_BLACK : SH110X_WHITE);
    textAt(PRAYER_NAME[LANG][i], x + 11, tall ? y + cjkTop() - 7 : y + 1);
    display.setCursor(x + w - 1 - strlen(hm) * 6, tall ? y + 4 : y + 1);
    display.print(hm);
    if (next) { display.setTextColor(SH110X_WHITE); u8f.setForegroundColor(SH110X_WHITE); }
  }
  pageHeader(hdr, nullptr);
}

// Fasting during Ramadan: imsak (stop eating) and iftar (break the fast), with a countdown to iftar by day
static void drawFasting(uint32_t now) {
  struct tm t;
  int nowMin = nowTm(t) ? t.tm_hour * 60 + t.tm_min : -1;
  float fajr = gPray.t[astro::FAJR], mag = gPray.t[astro::MAGHRIB];
  char a[10], b[10], buf[64];
  fmtMinutes(isnan(fajr) ? -1 : (int)lroundf(fajr) - 10, a, sizeof(a));   // imsak: 10 min before Fajr
  fmtMinutes(isnan(mag) ? -1 : (int)lroundf(mag), b, sizeof(b));
  snprintf(buf, sizeof(buf), T(S_RAMADAN_DAY), gCal.hd);
  bool tall = langIsCjk();
  textLine(T(S_IMSAK), tall ? 21 : 14, 0, 0);
  textLine(T(S_IFTAR), tall ? 36 : 30, 0, 0);
  display.setTextSize(2);
  display.setCursor(SCREEN_W - strlen(a) * 12, tall ? 20 : 12);
  display.print(a);
  display.setCursor(SCREEN_W - strlen(b) * 12, tall ? 38 : 28);
  display.print(b);
  display.setTextSize(1);
  if (nowMin >= 0 && !isnan(mag) && !isnan(fajr) && nowMin >= (int)fajr - 10 && nowMin < (int)lroundf(mag)) {
    int left = (int)lroundf(mag) - nowMin;
    char cd[48];
    snprintf(cd, sizeof(cd), T(S_IFTAR_IN), left / 60, left % 60);
    textLine(cd, 56, 1, 0);
  }
  pageHeader(buf, nullptr);
  (void)now;
}

// The US Air Quality Index (0-500) in its six bands, the current one filled, and the particles
static void drawAirQuality(const WeatherData& w, uint32_t el) {
  static const Str BAND[] = {S_AQI_GOOD, S_AQI_MODERATE, S_AQI_SENSITIVE, S_AQI_UNHEALTHY, S_AQI_VERY, S_AQI_HAZARD};
  const int band = w.aqi <= 50 ? 0 : w.aqi <= 100 ? 1 : w.aqi <= 150 ? 2 : w.aqi <= 200 ? 3 : w.aqi <= 300 ? 4 : 5;
  const bool cjk = langIsCjk();             // taller header and labels: a smaller number
  char buf[24];
  snprintf(buf, sizeof(buf), "%d", w.aqi);
  display.setTextSize(cjk ? 2 : 3);
  display.setCursor(0, cjk ? 18 : 13);
  display.print(buf);
  display.setTextSize(1);
  if (w.pm25 >= 0) {
    snprintf(buf, sizeof(buf), "PM2.5 %d", (int)lroundf(w.pm25));
    display.setCursor(62, cjk ? 22 : 15);
    display.print(buf);
  }
  if (!cjk && w.pm10 >= 0) {
    snprintf(buf, sizeof(buf), "PM10  %d", (int)lroundf(w.pm10));
    display.setCursor(62, 26);
    display.print(buf);
  }
  textLine(T(BAND[band]), cjk ? 38 : 41, 0, el);
  for (int i = 0; i < 6; i++) {
    if (i == band) display.fillRect(1 + i * 21, 54, 20, 8, SH110X_WHITE);
    else display.drawRect(1 + i * 21, 54, 20, 8, SH110X_WHITE);
  }
  pageHeader(T(S_AIR_QUALITY), "US AQI");
}

// Weather views, 6 s each: now, air quality, tomorrow, the next three days, sunrise/sunset
static void drawWeatherPage(uint32_t now) {
  WeatherData w = getWeather();
  if (!w.valid) {
    textLine(gWifiUp ? T(S_FETCH_WEATHER) : T(S_NO_WIFI), 28, 1, 0);
    return;
  }
  enum { V_NOW, V_AIR, V_TOMORROW, V_DAYS, V_SUN };
  uint8_t views[5], nv = 0;
  views[nv++] = V_NOW;
  if (w.aqi >= 0) views[nv++] = V_AIR;
  if (w.tmValid) views[nv++] = V_TOMORROW;
  if (w.days >= 5) views[nv++] = V_DAYS;
  if (w.sunriseMin >= 0) views[nv++] = V_SUN;
  uint32_t el = (now - gPageEnter) % 6000;
  uint8_t view = views[((now - gPageEnter) / 6000) % nv];
  if (view == V_TOMORROW) { drawTomorrow(w, now, el); return; }
  if (view == V_DAYS) { drawNextDays(w, now); return; }
  if (view == V_SUN) { drawSunTimes(w, el); return; }
  if (view == V_AIR) { drawAirQuality(w, el); return; }
  drawWeatherIcon(iconFor(w.code), w.isDay, 18, 16, now);

  char buf[96];
  snprintf(buf, sizeof(buf), "%d", (int)lroundf(w.temp));
  display.setTextSize(3);
  display.setCursor(42, 2);
  display.print(buf);
  int x = 42 + strlen(buf) * 18;
  display.setTextSize(1);
  display.setCursor(x, 2);
  display.print(DEG "C");
  LocationData loc = getLocation();
  if (loc.city[0]) snprintf(buf, sizeof(buf), "%s: %s", loc.name[0] ? loc.name : loc.city, condLabel(w.code));
  else strlcpy(buf, condLabel(w.code), sizeof(buf));
  char feels[40], hilo[64], rainWind[64];
  snprintf(feels, sizeof(feels), T(S_FEELS), (int)lroundf(w.feels));
  snprintf(hilo, sizeof(hilo), T(S_HI_LO_RH), (int)lroundf(w.tmax), (int)lroundf(w.tmin), w.humidity);
  snprintf(rainWind, sizeof(rainWind), T(S_RAIN_WIND), w.rainChance, (int)lroundf(w.wind));
  if (!langIsCjk()) {
    textAt(feels, 42, 28);
    textLine(buf, 38, 0, el);
    textLine(hilo, 47, 0, el);
    textLine(rainWind, 56, 0, el);
  } else {                                  // taller Chinese/Japanese lines (the icon reaches y=33):
    textAt(feels, 42, 29);                  // "feels" beside the temperature, and one bottom row:
    const char* row = el < 3000 ? buf : el < 4500 ? hilo : rainWind;   // condition 3 s, then details
    textLine(row, 56, 0, el < 3000 ? el : (el - 3000) % 1500);
  }
}

// ============================ TEXT BLOCKS ============================
// Word-wrap plain text into lines of up to 21 characters (6 px font, 128 px wide)
static int wrapAscii(const char* s, char lines[][22], int maxLines) {
  int n = 0;
  while (*s && n < maxLines) {
    while (*s == ' ') s++;
    if (!*s) break;
    int len = strlen(s), take = len <= 21 ? len : 21;
    if (len > 21) {                         // break at the last space that fits
      int b = take;
      while (b > 0 && s[b] != ' ') b--;
      if (b > 0) take = b;
    }
    memcpy(lines[n], s, take);
    lines[n][take] = 0;
    n++;
    s += take;
  }
  return n;
}

// Lines from y0 down; scrolls slowly when they don't all fit. Draw headers after this.
static int gWrapBottom = SCREEN_H;          // drawWrapped/drawTextBlock: the text area ends here
static void drawTextBlock(char lines[][22], int n, int y0, uint32_t el) {
  const int LH = 9, vis = max(1, (gWrapBottom - y0) / LH);
  int off = 0;
  if (n > vis) {
    const uint32_t HOLD = 2000, STEP = 80;
    uint32_t total = (n - vis) * LH, c = el % (HOLD + total * STEP + HOLD);
    off = c < HOLD ? 0 : c < HOLD + total * STEP ? (int)((c - HOLD) / STEP) : (int)total;
  }
  display.setTextSize(1);
  for (int i = 0; i < n; i++) {
    int y = y0 + i * LH - off;
    if (y <= y0 - LH || y >= SCREEN_H) continue;
    display.setCursor(0, y);
    display.print(lines[i]);
  }
}

// Word-wrapped text from y0 down, in any language (scrolls when it doesn't fit).
// Draw headers after this, since scrolling lines can pass under them.
static void drawWrapped(const char* text, int y0, uint32_t el) {
  if (!hasCjk(text)) {
    static char conv[700], lines[36][22];   // static: long news bulletins, small stack
    toCp437(text, conv, sizeof(conv));
    drawTextBlock(lines, wrapAscii(conv, lines, 36), y0, el);
    return;
  }
  static char fitted[700];                  // Chinese/Japanese: wrap by pixel width, any character
  fitToFont(text, fitted, sizeof(fitted), cjkFont());
  u8f.setFont(cjkFont());
  const int asc = cjkTop(), lh = cjkPitch();
  static char lines[30][64];
  char cur[64] = "";
  int n = 0, len = 0;
  const uint8_t* p = (const uint8_t*)fitted;
  while (*p && n < 30) {
    const uint8_t* start = p;
    utf8Next(p);
    int clen = p - start;
    if (!len && *start == ' ') continue;
    char cand[72];
    memcpy(cand, cur, len);
    memcpy(cand + len, start, clen);
    cand[len + clen] = 0;
    if (len && (u8f.getUTF8Width(cand) > SCREEN_W || len + clen >= 63)) {
      strcpy(lines[n++], cur);
      memcpy(cur, start, clen);
      len = clen;
      cur[len] = 0;
    } else {
      memcpy(cur + len, start, clen);
      len += clen;
      cur[len] = 0;
    }
  }
  if (len && n < 30) strcpy(lines[n++], cur);
  int vis = max(1, (gWrapBottom - y0) / lh), off = 0;
  if (n > vis) {
    const uint32_t HOLD = 2000, STEP = 80;
    uint32_t total = (n - vis) * lh, c = el % (HOLD + total * STEP + HOLD);
    off = c < HOLD ? 0 : c < HOLD + total * STEP ? (int)((c - HOLD) / STEP) : (int)total;
  }
  for (int i = 0; i < n; i++) {
    int y = y0 + i * lh - off;
    if (y <= y0 - lh || y >= SCREEN_H) continue;
    u8f.drawUTF8(0, y + asc, lines[i]);
  }
}

// ============================ NEWS PAGE ==============================
// The AI briefing (if any) first for 18 s, then each headline for 9 s
static void drawNewsPage(uint32_t now) {
  static uint32_t seenVer = 0xFFFFFFFF;
  static char items[NEWS_MAX + 1][640];
  static int count = 0;
  static bool hasBrief = false, cjkNews = false;
  if (xSemaphoreTake(gMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    if (gNews.version != seenVer) {
      seenVer = gNews.version;
      count = 0;
      hasBrief = gNews.brief[0] != 0;
      if (hasBrief) strlcpy(items[count++], gNews.brief, sizeof(items[0]));
      for (int i = 0; i < gNews.count; i++) strlcpy(items[count++], gNews.title[i], sizeof(items[0]));
      cjkNews = false;                      // e.g. a Chinese outlet while Momowo speaks English
      for (int i = 0; i < count && !cjkNews; i++) cjkNews = hasCjk(items[i]);
    }
    xSemaphoreGive(gMutex);
  }
  if (count == 0) {                         // no headlines yet: say why, and what to do
    if (!gWifiUp) { textLine(T(S_WAIT_WIFI), 28, 1, 0); return; }
    if (!gNewsStatus[0]) { textLine(T(S_FETCH_NEWS), 28, 1, 0); return; }
    char msg[240];
    snprintf(msg, sizeof(msg), T(S_NS_HINT), gNewsStatus);
    drawWrapped(msg, bodyTop(), now - gPageEnter);
    pageHeader(T(S_NEWS), nullptr);
    return;
  }
  const uint32_t SHOW = 9000;
  const uint32_t SHOW_BRIEF = hasBrief && strlen(items[0]) > 320 ? 48000 : 18000;   // time to scroll
  uint32_t el = now - gPageEnter;
  uint32_t total = (count - (hasBrief ? 1 : 0)) * SHOW + (hasBrief ? SHOW_BRIEF : 0);
  uint32_t pos = el % total, ph;
  int idx;
  if (hasBrief && pos < SHOW_BRIEF) { idx = 0; ph = pos; }
  else {
    uint32_t p = pos - (hasBrief ? SHOW_BRIEF : 0);
    idx = (hasBrief ? 1 : 0) + p / SHOW;
    ph = p % SHOW;
  }
  gTallHeader = cjkNews;                    // the same header height on every item of the page
  gTcText = wantsTc(items[idx]);
  drawWrapped(items[idx], bodyTop(), ph);
  gTcText = false;
  char hdr[48];
  bool brief = hasBrief && idx == 0;
  if (brief) snprintf(hdr, sizeof(hdr), "%s", T(S_AI_BRIEF));
  else snprintf(hdr, sizeof(hdr), T(S_NEWS_N), idx + (hasBrief ? 0 : 1), count - (hasBrief ? 1 : 0));
  pageHeader(hdr, brief ? nullptr : NEWS_CREDIT);
  gTallHeader = false;
}

// ============================ QUOTE PAGE (Chinese) ===================
static const int Q_MAXL = 12, Q_BYTES = 64;
static char qLines[Q_MAXL][Q_BYTES];
static int qCount = 0;

static int utf8Len(uint8_t c) {
  if (c < 0x80) return 1;
  if ((c & 0xE0) == 0xC0) return 2;
  if ((c & 0xF0) == 0xE0) return 3;
  if ((c & 0xF8) == 0xF0) return 4;
  return 1;
}

// Punctuation that must not start a line (Chinese line-breaking rule)
static bool noLineStart(const char* p) {
  static const char* const NS[] = {"，", "。", "、", "！", "？", "；", "：", "”", "’", "」",
                                   "』", "）", "》", "…", ",", ".", "!", "?", ";", ":", ")"};
  for (const char* s : NS)
    if (strncmp(p, s, strlen(s)) == 0) return true;
  return false;
}

// Clause-ending punctuation: lines prefer to break right after these
static bool isClauseEnd(const char* p) {
  static const char* const CE[] = {"，", "。", "；", "！", "？", "：", "、", ",", ";", "!", "?"};
  for (const char* s : CE)
    if (strncmp(p, s, strlen(s)) == 0) return true;
  return false;
}

static void flushLine(char* line, int& len) {
  if (len && qCount < Q_MAXL) strcpy(qLines[qCount++], line);
  len = 0;
  line[0] = 0;
}

// Append [s, e) to the current line, wrapping character by character
static void appendChars(const char* s, const char* e, char* line, int& len) {
  while (s < e && qCount < Q_MAXL) {
    if (!len && *s == ' ') { s++; continue; }
    int n = utf8Len((uint8_t)*s);
    if (s + n > e) n = e - s;
    if (len && !noLineStart(s)) {
      char cand[Q_BYTES + 4];
      memcpy(cand, line, len);
      memcpy(cand + len, s, n);
      cand[len + n] = 0;
      if (u8f.getUTF8Width(cand) > SCREEN_W) { flushLine(line, len); continue; }
    }
    if (len + n >= Q_BYTES) { flushLine(line, len); continue; }
    memcpy(line + len, s, n);
    len += n;
    line[len] = 0;
    s += n;
  }
}

// Wrap like a couplet: whole clauses per line when they fit, else by character
static void wrapCJK(const char* src) {
  u8f.setFont(CJK_FONT_BIG);
  qCount = 0;
  char line[Q_BYTES] = "";
  int len = 0;
  const char* p = src;
  while (*p && qCount < Q_MAXL) {
    const char* c0 = p;                     // one clause, punctuation included
    while (*p) {
      bool end = isClauseEnd(p);
      int n = utf8Len((uint8_t)*p);
      for (int i = 1; i < n; i++)
        if (!p[i]) { n = i; break; }
      p += n;
      if (end) break;
    }
    int clen = p - c0;
    if (len + clen < Q_BYTES) {
      char cand[Q_BYTES];
      memcpy(cand, line, len);
      memcpy(cand + len, c0, clen);
      cand[len + clen] = 0;
      if (u8f.getUTF8Width(cand) <= SCREEN_W) {
        memcpy(line, cand, len + clen + 1);
        len += clen;
        continue;
      }
    }
    flushLine(line, len);
    appendChars(c0, p, line, len);
  }
  flushLine(line, len);
}

static void cjkLine(const char* s, int baseline, uint8_t align, uint32_t elapsed) {
  int w = u8f.getUTF8Width(s);
  int x;
  if (w <= SCREEN_W) {
    x = align == 0 ? 0 : align == 1 ? (SCREEN_W - w) / 2 : SCREEN_W - w;
  } else {
    int over = w - SCREEN_W;
    int sc = elapsed > 800 ? (int)((elapsed - 800) / 30) : 0;
    x = -min(over, sc);
  }
  u8f.drawUTF8(x, baseline, s);
}

// Greek, Hebrew, Arabic and Pali quotes, each in a font with its letters
enum : uint8_t { SC_LATIN, SC_GREEK, SC_HEBREW, SC_ARABIC, SC_LATIN_EXT };

static uint8_t scriptOf(const char* s) {
  const uint8_t* p = (const uint8_t*)s;
  uint8_t sc = SC_LATIN;
  while (*p) {
    uint32_t c = utf8Next(p);
    if (c >= 0x590 && c <= 0x5FF) return SC_HEBREW;
    if (c >= 0x600 && c <= 0x6FF) return SC_ARABIC;
    if (c >= 0x370 && c <= 0x3FF) return SC_GREEK;
    if ((c >= 0x100 && c <= 0x24F) || (c >= 0x1E00 && c <= 0x1EFF)) sc = SC_LATIN_EXT;   // Pali: ā ṃ ṅ ...
  }
  return sc;
}

static const char* scriptLanguage(QKind kind, const char* text) {   // for the AI prompts
  switch (scriptOf(text)) {
    case SC_HEBREW: return "Hebrew";
    case SC_ARABIC: return "Arabic";
    case SC_GREEK: return kind == Q_BIBLE ? "Koine Greek" : "Ancient Greek";
    case SC_LATIN_EXT: return "Pali";
    default: return "language";
  }
}

// Arabic letters join: each takes its isolated, final, initial or medial presentation form
// (U+FE80 on, in Unicode order), and lam + alef become one ligature. Forms per letter from U+0621:
// 1 never joins, 2 joins only the letter before it, 4 joins both sides, 0 not a letter.
static const uint8_t AR_FORMS[] = {1, 2, 2, 2, 2, 4, 2, 4, 2, 4, 4, 4, 4, 4, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4,
                                   0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 2, 2, 4};
static uint8_t arForms(uint32_t c) { return c >= 0x621 && c <= 0x64A ? AR_FORMS[c - 0x621] : 0; }
static bool arJoinsNext(uint32_t c) { return arForms(c) == 4 || c == 0x640; }
static bool arJoinsPrev(uint32_t c) { return arForms(c) >= 2 || c == 0x640; }

static int arabicShape(uint16_t* cp, int n) {   // in place; returns the new length
  int o = 0;
  uint16_t prevRaw = 0;
  for (int i = 0; i < n; i++) {
    uint16_t c = cp[i];
    bool prev = arJoinsNext(prevRaw);
    prevRaw = c;
    uint8_t f = arForms(c);
    if (!f) { cp[o++] = c; continue; }
    uint16_t base = 0xFE80;
    for (int k = 0; k < c - 0x621; k++) base += AR_FORMS[k];
    if (f == 1) { cp[o++] = base; continue; }
    if (c == 0x644 && i + 1 < n) {          // lam + alef
      uint16_t a = cp[i + 1];
      uint16_t lig = a == 0x622 ? 0xFEF5 : a == 0x623 ? 0xFEF7 : a == 0x625 ? 0xFEF9 : a == 0x627 ? 0xFEFB : 0;
      if (lig) { cp[o++] = lig + (prev ? 1 : 0); prevRaw = a; i++; continue; }
    }
    bool next = f == 4 && i + 1 < n && arJoinsPrev(cp[i + 1]);
    cp[o++] = base + (prev && next ? 3 : prev ? 1 : next ? 2 : 0);
  }
  return o;
}

// The quote centred in the area above the source line, wrapped at spaces, right to left for
// Hebrew and Arabic; scrolls when it is too long
static void drawScriptText(const char* text, uint8_t sc, int area, uint32_t el) {
  u8f.setFont(sc == SC_GREEK ? u8g2_font_cu12_t_greek : sc == SC_HEBREW ? u8g2_font_cu12_t_hebrew
              : sc == SC_ARABIC ? u8g2_font_cu12_t_arabic : u8g2_font_fusion12_tc);
  static uint16_t cp[256];
  static uint8_t w[256];
  int n = 0;
  const uint8_t* p = (const uint8_t*)text;
  while (*p && n < 256) {
    uint32_t c = utf8Next(p);
    if (sc == SC_ARABIC && ((c >= 0x64B && c <= 0x65F) || c == 0x670)) continue;   // vowel marks: dropped
    if (c == 0x671) c = 0x627;              // alef wasla: plain alef
    cp[n++] = c > 0xFFFF ? '?' : c;
  }
  if (sc == SC_ARABIC) n = arabicShape(cp, n);
  for (int i = 0; i < n; i++) {
    char g[4];
    if (cp[i] < 0x80) { g[0] = cp[i]; g[1] = 0; }
    else if (cp[i] < 0x800) { g[0] = 0xC0 | cp[i] >> 6; g[1] = 0x80 | (cp[i] & 0x3F); g[2] = 0; }
    else { g[0] = 0xE0 | cp[i] >> 12; g[1] = 0x80 | (cp[i] >> 6 & 0x3F); g[2] = 0x80 | (cp[i] & 0x3F); g[3] = 0; }
    w[i] = u8f.getUTF8Width(g);
  }
  int ls[12], le[12], lines = 0, i = 0;     // lines as [start, end) in cp
  while (i < n && lines < 12) {
    while (i < n && cp[i] == ' ') i++;
    if (i >= n) break;
    int start = i, lastSpace = -1, width = 0;
    while (i < n) {
      if (cp[i] == ' ') lastSpace = i;
      if (width + w[i] > SCREEN_W && i > start) {
        if (lastSpace > start) i = lastSpace;   // else: one long word, broken where it fills the line
        break;
      }
      width += w[i++];
    }
    ls[lines] = start;
    le[lines++] = i;
  }
  const int asc = u8f.getFontAscent(), lh = asc - u8f.getFontDescent() + 1;
  int y0 = 0, off = 0;
  if (lines * lh <= area) {
    y0 = (area - lines * lh) / 2;
  } else {
    const uint32_t HOLD = 2500, STEP = 70;
    uint32_t total = lines * lh - area, c = el % (HOLD + total * STEP + HOLD);
    off = c < HOLD ? 0 : c < HOLD + total * STEP ? (int)((c - HOLD) / STEP) : (int)total;
  }
  const bool rtl = sc == SC_HEBREW || sc == SC_ARABIC;
  for (int l = 0; l < lines; l++) {
    int y = y0 + l * lh - off;
    if (y + lh <= 0 || y >= area) continue;
    int e = le[l];
    while (e > ls[l] && cp[e - 1] == ' ') e--;
    int lw = 0;
    for (int k = ls[l]; k < e; k++) lw += w[k];
    int x = rtl ? (SCREEN_W + lw) / 2 : (SCREEN_W - lw) / 2;
    for (int k = ls[l]; k < e; k++) {       // logical order; Hebrew and Arabic step leftwards
      if (rtl) x -= w[k];
      u8f.drawGlyph(x, y + asc, cp[k]);
      if (!rtl) x += w[k];
    }
  }
}

// Greek, modern or scripture quote: the original for 10 s, then the translation or the AI's reflection
static void drawWorldQuote(QKind kind, const char* text, const char* author, const char* meaning, uint32_t el) {
  const Str hdr = (QUOTE_AI || quoteInMyLanguage(kind)) && AI_API_KEY[0] ? S_REFLECTION : S_TRANSLATION;
  if (meaning[0] && el % 20000 >= 10000) {
    drawWrapped(meaning, bodyTop(), el % 20000 - 10000);
    pageHeader(T(hdr), nullptr);
    return;
  }
  const int AREA = SCREEN_H - 12;
  const uint8_t sc = scriptOf(text);
  if (sc != SC_LATIN) {
    drawScriptText(text, sc, AREA, el % 10000);
  } else if (hasCjk(text)) {
    gWrapBottom = AREA;
    drawWrapped(text, 0, el % 10000);
    gWrapBottom = SCREEN_H;
  } else {
    char conv[300], lines[14][22];
    toCp437(text, conv, sizeof(conv));
    int n = wrapAscii(conv, lines, 14);
    int y0 = n * 9 <= AREA + 1 ? (AREA + 1 - n * 9) / 2 : 0;   // (a line's 9th row is blank)
    gWrapBottom = AREA + 1;
    drawTextBlock(lines, n, y0, el % 10000);
    gWrapBottom = SCREEN_H;
  }
  display.fillRect(0, AREA, SCREEN_W, SCREEN_H - AREA, SH110X_BLACK);
  textLine(author, SCREEN_H - 8, 2, el);
}

static void drawQuotePage(uint32_t now) {
  static uint32_t seenVer = 0xFFFFFFFF, shownAt = 0;
  static char author[96] = "", tag[8] = "", meaning[320] = "", text[256] = "";
  static QKind kind = Q_CHINESE;
  if (xSemaphoreTake(gMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    if (gQuote.version != seenVer) {
      seenVer = gQuote.version;
      shownAt = now;
      kind = gQuote.kind;
      if (kind == Q_CHINESE) wrapCJK(gQuote.text);
      else qCount = gQuote.text[0] ? 1 : 0;
      strlcpy(text, gQuote.text, sizeof(text));
      strlcpy(author, gQuote.author, sizeof(author));
      strlcpy(tag, gQuote.tag, sizeof(tag));
      strlcpy(meaning, gQuote.meaning, sizeof(meaning));
    }
    xSemaphoreGive(gMutex);
  }
  if (kind != Q_CHINESE && qCount) {
    uint32_t base = gPageEnter > shownAt ? gPageEnter : shownAt;
    drawWorldQuote(kind, text, author, meaning, now - base);
    return;
  }

  u8f.setFont(CJK_FONT);                    // source line metrics
  const int SASC  = u8f.getFontAscent();
  const int SDESC = -u8f.getFontDescent();
  const int SLH   = max(12, SASC + SDESC);
  u8f.setFont(CJK_FONT_BIG);                // quote metrics
  const int ASC  = u8f.getFontAscent();
  const int LH   = max(16, ASC - u8f.getFontDescent()) + 2;
  const int AREA = SCREEN_H - SLH;          // quote area; bottom line is the source
  const int VIS  = AREA / LH;

  if (qCount == 0) {
    textLine(T(S_FETCH_QUOTE), 28, 1, 0);
    return;
  }

  uint32_t base = gPageEnter > shownAt ? gPageEnter : shownAt;
  uint32_t el = now - base;
  if (meaning[0] && el % 20000 >= 10000) {  // 10 s Chinese, then 10 s meaning or explanation
    drawWrapped(meaning, bodyTop(), el % 20000 - 10000);
    pageHeader(T(QUOTE_AI || (quoteInMyLanguage(Q_CHINESE) && AI_API_KEY[0]) ? S_REFLECTION : S_MEANING), nullptr);
    return;
  }
  int off = 0, y0 = 0;
  if (qCount <= VIS) {
    y0 = (AREA - qCount * LH) / 2;
  } else {                                  // hold, scroll, hold, repeat
    const uint32_t HOLD = 2500, STEP = 60;
    uint32_t total = (qCount - VIS) * LH;
    uint32_t c = el % (HOLD + total * STEP + HOLD);
    if (c < HOLD) off = 0;
    else if (c < HOLD + total * STEP) off = (c - HOLD) / STEP;
    else off = total;
  }

  for (int i = 0; i < qCount; i++) {
    int y = y0 + i * LH - off;
    if (y <= -LH || y >= AREA) continue;
    int w = u8f.getUTF8Width(qLines[i]);
    u8f.drawUTF8((SCREEN_W - w) / 2, y + ASC, qLines[i]);
  }
  display.fillRect(0, AREA, SCREEN_W, SCREEN_H - AREA, SH110X_BLACK);
  u8f.setFont(CJK_FONT);
  const int srcBase = SCREEN_H - 1 - SDESC;
  cjkLine(author, srcBase, 2, el);
  if (tag[0]) {                             // framed theme tag, if there's room
    int tw = u8f.getUTF8Width(tag);
    if (tw + 6 + 8 + u8f.getUTF8Width(author) <= SCREEN_W) {
      display.drawRoundRect(0, srcBase - SASC - 1, tw + 6, SASC + SDESC + 2, 3, SH110X_WHITE);
      u8f.drawUTF8(3, srcBase, tag);
    }
  }
}

// ============================ FOCUS TIMER ============================
// Focus -> break (every 4th break three times as long) -> ready again. A hold starts, pauses,
// resumes or skips the break; a 2 s hold resets, or, when idle, starts a 1-minute breathing exercise.
static const uint32_t BREATHE_MS = 70000UL;   // five slow breaths
static const uint32_t BREATH_IN = 4000, BREATH_HOLD = 4000, BREATH_OUT = 6000;

static void focusToggle(uint32_t now) {
  if (fState == F_RUN) {
    fState = F_PAUSE;
    toast(T(S_PAUSED));
    PLAY(SND_PAUSE);
  } else if (fState == F_BREAK || fState == F_BREATHE) {   // skip the break / stop breathing
    fState = F_IDLE;
    fRemain = fTotal = focusMs();
    toast(T(S_TIMER_RESET));
  } else {
    bool fresh = (fState == F_IDLE);
    if (fresh) fRemain = fTotal = focusMs();
    fState = F_RUN;
    fLastTick = now;
    char m[48];
    snprintf(m, sizeof(m), T(S_FOCUS_MIN), (unsigned)FOCUS_MIN);
    toast(fresh ? m : T(S_RESUMED));
    PLAY(SND_ON);
  }
}

static void focusReset() {
  fState = F_IDLE;
  fRemain = fTotal = focusMs();
  toast(T(S_TIMER_RESET));
}

static void breatheStart(uint32_t now) {
  fState = F_BREATHE;
  fRemain = fTotal = BREATHE_MS;
  fLastTick = now;
  PLAY(SND_ON);
}

static void focusUpdate(uint32_t now) {
  if (fState != F_RUN && fState != F_BREAK && fState != F_BREATHE) return;
  uint32_t dt = now - fLastTick;
  fLastTick = now;
  if (dt < fRemain) { fRemain -= dt; return; }
  FocusState was = fState;
  fState = F_IDLE;
  fRemain = fTotal = focusMs();
  if (was == F_BREATHE) {                   // breathing done
    gBreathsToday++;
    setMood(M_HAPPY, 3000);
    toastFor(T(S_BREATHE_DONE), 3000);
    PLAY(SND_WAKE);
    return;
  }
  if (was == F_BREAK) {                     // break over: ready for the next round
    gPage = P_BUDDY;
    gPageEnter = now;
    setMood(M_SURPRISED, 900);
    PLAY(SND_WAKE);
    announce(T(S_BREAK_OVER), 10000);
    return;
  }
  gSessions++;                              // focus session complete!
  prefs.putInt("fdone", gSessions);        // remembered through restarts and settings changes
  gBreaks++;
  uint32_t brk = BREAK_MIN * (gBreaks % 4 == 0 ? 3 : 1);
  fState = F_BREAK;                         // the break starts by itself
  fRemain = fTotal = brk * 60000UL;
  fLastTick = now;
  gPage = P_BUDDY;
  gPageEnter = gLastInteraction = now;
  setMood(M_LOVE, 5000);
  PLAY(SND_DONE);
  gFlashUntil = now + 1600;
  char msg[96];
  snprintf(msg, sizeof(msg), T(S_FOCUS_DONE), (unsigned)brk);
  announce(msg, 12000);
}

// The breathing guide: a circle that grows as you breathe in, rests, and shrinks as you breathe out
static void drawBreathing(uint32_t now, int y0) {
  uint32_t ph = (fTotal - fRemain) % (BREATH_IN + BREATH_HOLD + BREATH_OUT);
  float k;
  Str label;
  if (ph < BREATH_IN) { k = ph / (float)BREATH_IN; label = S_BREATHE_IN; }
  else if (ph < BREATH_IN + BREATH_HOLD) { k = 1; label = S_BREATHE_HOLD; }
  else { k = 1 - (ph - BREATH_IN - BREATH_HOLD) / (float)BREATH_OUT; label = S_BREATHE_OUT; }
  k = 0.5f - 0.5f * cosf(k * PI);           // ease in and out
  int cy = y0 + (52 - y0) / 2, rmax = (52 - y0) / 2 - 1;
  int r = 3 + (int)(k * (rmax - 3));
  display.drawCircle(28, cy, rmax, SH110X_WHITE);
  display.fillCircle(28, cy, r, SH110X_WHITE);
  textAt(T(label), 58, cy - 4);
  (void)now;
}

static void drawFocusPage(uint32_t now) {
  char buf[64];
  uint32_t s = (fRemain + 999) / 1000;
  if (fState == F_BREAK || fState == F_BREATHE) {
    if (fState == F_BREAK) snprintf(buf, sizeof(buf), T(S_BREAK_LEFT), (unsigned)(s / 60), (unsigned)(s % 60));
    else snprintf(buf, sizeof(buf), "%s %u:%02u", T(S_BREATHE_TITLE), (unsigned)(s / 60), (unsigned)(s % 60));
    textLine(buf, 0, 1, 0);
    drawBreathing(now, langIsCjk() ? 16 : 10);
    textLine(fState == F_BREAK ? T(S_HOLD_SKIP) : T(S_HOLD_STOP), 56, 1, 0);
    return;
  }
  if (fState == F_RUN)        snprintf(buf, sizeof(buf), T(S_STAY_FOCUSED), gSessions + 1);
  else if (fState == F_PAUSE) snprintf(buf, sizeof(buf), "%s", T(S_PAUSED));
  else                        snprintf(buf, sizeof(buf), T(S_DONE_TODAY), gSessions);
  textLine(buf, 0, 1, 0);

  snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)(s / 60), (unsigned)(s % 60));
  if (!(fState == F_PAUSE && (now / 500) % 2)) {
    display.setTextSize(3);
    display.setCursor((SCREEN_W - 5 * 18) / 2 + 2, langIsCjk() ? 17 : 13);
    display.print(buf);
    display.setTextSize(1);
  }
  display.drawRoundRect(4, 42, 120, 7, 3, SH110X_WHITE);
  uint32_t fillW = fTotal ? 116ULL * (fTotal - fRemain) / fTotal : 0;
  if (fillW) display.fillRect(6, 44, fillW, 3, SH110X_WHITE);

  textLine(fState == F_IDLE ? T(S_HOLD_START2)
         : fState == F_RUN  ? T(S_HOLD_PAUSE)
                            : T(S_HOLD_RESUME), 56, 1, 0);
}

// ============================ PHONE SETUP =============================
// The setup page, served by Momowo itself (no app to install)
#include "portal.h"                         // PORTAL_HTML: the phone setup page

static void loadHolidays();

static void loadSettings() {
  auto getStr = [](const char* key, char* dst, size_t n) {
    String v = prefs.getString(key, dst);
    strlcpy(dst, v.c_str(), n);
  };
  getStr("ssid", WIFI_SSID, sizeof(WIFI_SSID));
  getStr("pass", WIFI_PASS, sizeof(WIFI_PASS));
  getStr("ssid2", WIFI2_SSID, sizeof(WIFI2_SSID));
  getStr("pass2", WIFI2_PASS, sizeof(WIFI2_PASS));
  getStr("name", OWNER_NAME, sizeof(OWNER_NAME));
  getStr("hobbies", OWNER_HOBBIES, sizeof(OWNER_HOBBIES));
  LANG = prefs.getUChar("lang", LANG);
  gSessions   = prefs.getInt("fdone", 0);  // today's focus sessions so far (checked against the date
  gSessionDay = prefs.getInt("fday", -1);  // once the clock is known)
  if (LANG >= L_COUNT) LANG = L_EN;
  getStr("suburb", SUBURB, sizeof(SUBURB));
  getStr("cc", COUNTRY_CODE, sizeof(COUNTRY_CODE));
  getStr("news", NEWS_FEED_URL, sizeof(NEWS_FEED_URL));
  getStr("credit", NEWS_CREDIT, sizeof(NEWS_CREDIT));
  getStr("voicetxt", NEWS_VOICE_OWN, sizeof(NEWS_VOICE_OWN));
  getStr("famous", FAMOUS_PEOPLE, sizeof(FAMOUS_PEOPLE));
  AI_PROVIDER = min<int>(prefs.getUChar("aiprovider", 0), 3);
  getStr(AI_KEY_PREFS[AI_PROVIDER], AI_API_KEY, sizeof(AI_API_KEY));
  getStr(AI_MODEL_PREFS[AI_PROVIDER], AI_MODEL_CUSTOM, sizeof(AI_MODEL_CUSTOM));
  SLEEP_HOUR    = prefs.getChar("sleep", SLEEP_HOUR);
  WAKE_HOUR     = prefs.getChar("wake", WAKE_HOUR);
  BUZZER_VOLUME = prefs.getUChar("vol", BUZZER_VOLUME);
  NEWS_VOICE    = prefs.getUChar("voice", NEWS_VOICE);
  if (NEWS_VOICE > 6) NEWS_VOICE = 0;
  TOUCH_CLICK   = prefs.getBool("click", TOUCH_CLICK);
  HOURLY_CHIME  = prefs.getBool("chime", HOURLY_CHIME);
  USE_24H       = prefs.getBool("h24", USE_24H);
  gHijriAdj     = prefs.getChar("hijriAdj", 0);
  gMuted        = prefs.getBool("mute", false);
  gSavedLat     = prefs.getFloat("lat", NAN);
  gSavedLon     = prefs.getFloat("lon", NAN);
  CALS          = prefs.getUChar("cals", CALS);
  PRAY_METHOD   = prefs.getUChar("pmethod", PRAY_METHOD);
  ASR_HANAFI    = prefs.getBool("hanafi", ASR_HANAFI);
  PRAY_REMIND   = prefs.getBool("premind", PRAY_REMIND);
  if (prefs.isKey("qmask2")) QUOTE_MASK = prefs.getUShort("qmask2", QUOTE_MASK) & 255;   // 256 was song lines (removed)
  else if (prefs.isKey("qmask")) QUOTE_MASK = prefs.getUChar("qmask", QUOTE_MASK) & 127;   // before famous people
  else if (prefs.isKey("qsrc")) {           // older setting: one source, or 3 = a mix of the first three
    uint8_t s = prefs.getUChar("qsrc", 0) & 3;
    QUOTE_MASK = s == 3 ? 7 : 1 << s;
  }
  if (!QUOTE_MASK) QUOTE_MASK = 1;
  OWNER_GENDER  = prefs.getUChar("gender", 0) & 3;
  QUOTE_AI      = prefs.getUChar("qai", QUOTE_AI) & 1;
  AI_MODEL_IX   = min<int>(prefs.getUChar("aimodel", AI_MODEL_IX), 3);
  AI_DAY_LIMIT  = constrain(prefs.getUShort("ailimit", AI_DAY_LIMIT), 1, 1000);
  FEEL_MIN      = min<int>(prefs.getUShort("feelmin", FEEL_MIN), 1440);
  gAiDay        = prefs.getShort("aiday", -1);
  gAiUsed       = prefs.getUShort("aiused", 0);
  gGame.bestStars = prefs.getUShort("rollbest", 0);
  gGame.qbest = prefs.getUChar("questbest", 0);
  gGame.tbest = prefs.getUShort("tapbest", 0);
  AI_EFFORT_IX  = min<int>(prefs.getUChar("aieffort", AI_EFFORT_IX), 4);
  NEWS_TRANSLATE = prefs.getBool("ntrans", NEWS_TRANSLATE);
  NEWS_STYLE    = prefs.getUChar("nstyle", NEWS_STYLE) & 1;
  FOCUS_MIN     = constrain(prefs.getUChar("fmin", FOCUS_MIN), 5, 90);
  BREAK_MIN     = constrain(prefs.getUChar("bmin", BREAK_MIN), 1, 30);
  MOTION_ON     = prefs.getBool("motion", MOTION_ON);
  loadHolidays();
  MOTION_SENS   = constrain(prefs.getUChar("msens", MOTION_SENS), 1, 3);
  MOTION_MIRROR = prefs.getBool("mmirror", MOTION_MIRROR);
  fRemain = fTotal = focusMs();
}

static void sendJson(JsonDocument& d) {
  String out;
  serializeJson(d, out);
  server.send(200, "application/json", out);
}

static void handleRoot() { server.send_P(200, "text/html; charset=utf-8", PORTAL_HTML); }

// Current settings for the page. Passwords and the AI key are never sent back.
static void handleInfo() {
  JsonDocument d;
  d["ssid"] = WIFI_SSID;      d["ssid2"] = WIFI2_SSID;
  d["name"] = OWNER_NAME;     d["suburb"] = SUBURB;       d["cc"] = COUNTRY_CODE;
  d["hobbies"] = OWNER_HOBBIES;
  if (prefs.isKey("lang")) d["lang"] = LANG;   // none saved yet: the page uses the phone's language
  d["h24"] = USE_24H;         d["sleep"] = SLEEP_HOUR;    d["wake"] = WAKE_HOUR;
  d["sound"] = !gMuted;       d["vol"] = BUZZER_VOLUME;   d["click"] = TOUCH_CLICK;
  d["voice"] = NEWS_VOICE;    d["voicetxt"] = NEWS_VOICE_OWN;
  d["chime"] = HOURLY_CHIME;  d["news"] = NEWS_FEED_URL;  d["credit"] = NEWS_CREDIT;
  d["hasPass"] = WIFI_PASS[0] != 0;
  d["hasPass2"] = WIFI2_PASS[0] != 0;
  d["hasKey"] = AI_API_KEY[0] != 0;
  d["aiprovider"] = AI_PROVIDER;
  for (int i = 0; i < 4; ++i) {
    d["aiKeys"][i] = prefs.getString(AI_KEY_PREFS[i], "").length() > 0;
    d["aiModels"][i] = prefs.getString(AI_MODEL_PREFS[i], "");
  }
  if (const char* why = wifiReasonText(gWifiReason)) d["wifiErr"] = why;   // shown at the top of the page
  d["host"] = gHost;
  d["online"] = (bool)gWifiUp;
  d["timeSet"] = time(nullptr) > 1700000000;
  d["rtc"] = gRtc;
  d["cals"] = CALS;           d["pmethod"] = PRAY_METHOD; d["hanafi"] = ASR_HANAFI;
  d["premind"] = PRAY_REMIND; d["hijriAdj"] = gHijriAdj;
  d["methodNow"] = astro::METHODS[prayerMethod()].name;   // what "automatic" picked
  d["qmask"] = QUOTE_MASK;    d["qai"] = QUOTE_AI;        d["gender"] = OWNER_GENDER;
  d["aimodel"] = AI_MODEL_IX; d["aieffort"] = AI_EFFORT_IX;
  d["feelmin"] = FEEL_MIN;
  d["ailimit"] = AI_DAY_LIMIT; { struct tm t; d["aiUsed"] = (nowTm(t) && t.tm_yday != gAiDay) ? 0 : (int)gAiUsed; }
  d["famous"] = FAMOUS_PEOPLE;
  d["ntrans"] = NEWS_TRANSLATE; d["nstyle"] = NEWS_STYLE;
  d["fmin"] = FOCUS_MIN;      d["bmin"] = BREAK_MIN;
  d["motion"] = MOTION_ON;    d["msens"] = MOTION_SENS;   d["mmirror"] = MOTION_MIRROR;
  d["mpu"] = gMotion;         // is a motion sensor fitted?
  if (!gMotion) {                           // not found: what the I2C bus holds instead
    char found[64], who[24];
    motion::i2cScan(found, sizeof(found), who, sizeof(who));
    d["i2c"] = found;         d["mpuId"] = who;
  }
  d["build"] = __DATE__ " " __TIME__;
  if (gPray.valid) {                        // today's prayer times, for other apps
    JsonArray p = d["prayers"].to<JsonArray>();
    for (int i = 0; i < astro::PRAYER_COUNT; i++) {
      char hm[8];
      int v = isnan(gPray.t[i]) ? -1 : (int)lroundf(gPray.t[i]);
      if (v < 0) { p.add(nullptr); continue; }
      snprintf(hm, sizeof(hm), "%02d:%02d", (v / 60) % 24, v % 60);
      p.add(hm);
    }
  }
  sendJson(d);
}

// The page sends the phone's clock and timezone as soon as it opens: Momowo gets the
// right time with no internet at all
static void handleTime() {
  JsonDocument d;
  if (deserializeJson(d, server.arg("plain"))) { server.send(400, "text/plain", "Bad request"); return; }
  int64_t t = d["t"] | (int64_t)0;
  int32_t off = d["off"] | (int32_t)0;
  if (t > 1700000000 && off >= -14 * 3600 && off <= 14 * 3600) {
    struct timeval tv = { (time_t)t, 0 };
    settimeofday(&tv, nullptr);
    if (d["tz"].is<const char*>()) {        // the phone's zone with its summer-time rule ("" if none)
      const char* rule = d["tz"];
      bool ok = strlen(rule) >= 8 && strlen(rule) < sizeof(gTzRule);
      for (const char* c = rule; ok && *c; c++) ok = isalnum((uint8_t)*c) || strchr(",.:/+-<>", *c);
      if (ok) {                             // it must agree with the phone's own offset now
        setenv("TZ", rule, 1);
        tzset();
        ok = tzEastAt((time_t)t) == off;
      }
      if (ok && strcmp(gTzRule, rule)) { strlcpy(gTzRule, rule, sizeof(gTzRule)); prefs.putString("tzrule", gTzRule); }
      else if (!ok && gTzRule[0]) { gTzRule[0] = 0; prefs.remove("tzrule"); }
      gTzRuleOn = false;
      gTzVersion = gTzVersion + 1;          // TZ changed for the check: loop() applies ours again
    }
    setTzOffset(off);
    rtcSave();
    Serial.printf("[clock] set from phone (UTC%+.1f)\n", off / 3600.0);
  }
  server.send(200, "application/json", "{\"ok\":true}");
}

static void handleScan() {                  // nearby WiFi names, for the dropdowns
  JsonDocument d;
  JsonArray a = d.to<JsonArray>();
  if (xSemaphoreTake(gScanMutex, pdMS_TO_TICKS(8000)) == pdTRUE) {   // Momowo may be scanning itself
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
      JsonObject o = a.add<JsonObject>();
      o["ssid"] = WiFi.SSID(i);
      o["rssi"] = WiFi.RSSI(i);
    }
    WiFi.scanDelete();
    xSemaphoreGive(gScanMutex);
  }
  sendJson(d);
}

static void handleSave() {
  JsonDocument d;
  if (deserializeJson(d, server.arg("plain"))) { server.send(400, "text/plain", "Bad request"); return; }
  if ((d["aikey"].is<const char*>() && (strlen(d["aikey"]) >= sizeof(AI_API_KEY) || strpbrk(d["aikey"], "\r\n"))) ||
      (d["aicustom"].is<const char*>() && strlen(d["aicustom"]) >= sizeof(AI_MODEL_CUSTOM))) {
    server.send(400, "text/plain", "AI key or model is too long or invalid"); return;
  }
  auto putStr = [&](const char* key, char* dst, size_t n) {   // only fields the page sent
    if (d[key].is<const char*>()) { utf8Copy(dst, d[key].as<const char*>(), n); prefs.putString(key, dst); }
  };
  putStr("ssid", WIFI_SSID, sizeof(WIFI_SSID));
  putStr("pass", WIFI_PASS, sizeof(WIFI_PASS));
  putStr("ssid2", WIFI2_SSID, sizeof(WIFI2_SSID));
  putStr("pass2", WIFI2_PASS, sizeof(WIFI2_PASS));
  putStr("name", OWNER_NAME, sizeof(OWNER_NAME));
  putStr("hobbies", OWNER_HOBBIES, sizeof(OWNER_HOBBIES));
  if (d["lang"].is<int>()) prefs.putUChar("lang", constrain(d["lang"].as<int>(), 0, L_COUNT - 1));
  putStr("suburb", SUBURB, sizeof(SUBURB));
  putStr("cc", COUNTRY_CODE, sizeof(COUNTRY_CODE));
  putStr("news", NEWS_FEED_URL, sizeof(NEWS_FEED_URL));
  putStr("credit", NEWS_CREDIT, sizeof(NEWS_CREDIT));
  putStr("voicetxt", NEWS_VOICE_OWN, sizeof(NEWS_VOICE_OWN));
  putStr("famous", FAMOUS_PEOPLE, sizeof(FAMOUS_PEOPLE));
  // Save to the selected provider's slot. Runtime configuration changes on restart.
  uint8_t provider = d["aiprovider"].is<int>() ? constrain(d["aiprovider"].as<int>(), 0, 3) : AI_PROVIDER;
  prefs.putUChar("aiprovider", provider);
  if (d["aiclear"] | false) prefs.remove(AI_KEY_PREFS[provider]);   // first: a key typed in the same save wins
  if (d["aikey"].is<const char*>() && *d["aikey"].as<const char*>()) {
    if (prefs.getString(AI_KEY_PREFS[provider], "") != d["aikey"].as<const char*>()) {
      prefs.putString(AI_KEY_PREFS[provider], d["aikey"].as<const char*>());
      gAiUsed = 0;
      prefs.putUShort("aiused", 0);
    }
  }
  if (d["aicustom"].is<const char*>()) prefs.putString(AI_MODEL_PREFS[provider], d["aicustom"].as<const char*>());
  if (d["h24"].is<bool>())   prefs.putBool("h24", d["h24"].as<bool>());
  if (d["sleep"].is<int>())  prefs.putChar("sleep", constrain(d["sleep"].as<int>(), 0, 23));
  if (d["wake"].is<int>())   prefs.putChar("wake", constrain(d["wake"].as<int>(), 0, 23));
  if (d["vol"].is<int>())    prefs.putUChar("vol", constrain(d["vol"].as<int>(), 1, 10));
  if (d["voice"].is<int>())  prefs.putUChar("voice", constrain(d["voice"].as<int>(), 0, 6));
  if (d["sound"].is<bool>()) prefs.putBool("mute", !d["sound"].as<bool>());
  if (d["click"].is<bool>()) prefs.putBool("click", d["click"].as<bool>());
  if (d["chime"].is<bool>()) prefs.putBool("chime", d["chime"].as<bool>());
  if (d["cals"].is<int>())    prefs.putUChar("cals", d["cals"].as<int>() & CAL_ALL);
  if (d["pmethod"].is<int>()) {
    int v = d["pmethod"].as<int>();
    prefs.putUChar("pmethod", v >= 0 && v < astro::METHOD_COUNT ? v : 255);
  }
  if (d["hanafi"].is<bool>())  prefs.putBool("hanafi", d["hanafi"].as<bool>());
  if (d["premind"].is<bool>()) prefs.putBool("premind", d["premind"].as<bool>());
  if (d["hijriAdj"].is<int>()) prefs.putChar("hijriAdj", constrain(d["hijriAdj"].as<int>(), -2, 2));
  if (d["qmask"].is<int>()) {
    int v = d["qmask"].as<int>() & 255;
    prefs.putUShort("qmask2", v ? v : 1);
  }
  if (d["gender"].is<int>())   prefs.putUChar("gender", constrain(d["gender"].as<int>(), 0, 3));
  if (d["qai"].is<int>())      prefs.putUChar("qai", constrain(d["qai"].as<int>(), 0, 1));
  if (d["aimodel"].is<int>())  prefs.putUChar("aimodel", constrain(d["aimodel"].as<int>(), 0, 3));
  if (d["ailimit"].is<int>())  prefs.putUShort("ailimit", constrain(d["ailimit"].as<int>(), 1, 1000));
  if (d["feelmin"].is<int>())  prefs.putUShort("feelmin", constrain(d["feelmin"].as<int>(), 0, 1440));
  if (d["aieffort"].is<int>()) prefs.putUChar("aieffort", constrain(d["aieffort"].as<int>(), 0, 4));
  if (d["ntrans"].is<bool>())  prefs.putBool("ntrans", d["ntrans"].as<bool>());
  if (d["nstyle"].is<int>())   prefs.putUChar("nstyle", constrain(d["nstyle"].as<int>(), 0, 1));
  if (d["fmin"].is<int>())     prefs.putUChar("fmin", constrain(d["fmin"].as<int>(), 5, 90));
  if (d["bmin"].is<int>())     prefs.putUChar("bmin", constrain(d["bmin"].as<int>(), 1, 30));
  if (d["motion"].is<bool>())  prefs.putBool("motion", d["motion"].as<bool>());
  if (d["msens"].is<int>())    prefs.putUChar("msens", constrain(d["msens"].as<int>(), 1, 3));
  if (d["mmirror"].is<bool>()) prefs.putBool("mmirror", d["mmirror"].as<bool>());
  server.send(200, "application/json", "{\"ok\":true}");
  gRestartAt = millis() + 1500;             // restart with the new settings
}

static void handleReset() {                 // "Forget all settings": back to first-start setup
  prefs.clear();
  server.send(200, "application/json", "{\"ok\":true}");
  gRestartAt = millis() + 1500;
}

// Any other address (including the phone's "is there internet?" checks) leads to the
// setup page, which is what makes it pop up by itself after joining Momowo's WiFi
static void redirectHome() {
  server.sendHeader("Location", gSetupMode ? String("http://192.168.4.1/") : String("http://") + gHost + ".local/", true);
  server.send(302, "text/plain", "");
}

static bool jsonOnly() {
  if (server.header("Content-Type").startsWith("application/json")) return true;
  server.send(415, "text/plain", "JSON only");
  return false;
}

static void webBegin() {
  static const char* HDRS[] = {"Content-Type"};
  server.collectHeaders(HDRS, 1);
  server.on("/", handleRoot);
  server.on("/info", handleInfo);
  server.on("/scan", handleScan);
  server.on("/save", HTTP_POST, [] { if (jsonOnly()) handleSave(); });
  server.on("/reset", HTTP_POST, [] { if (jsonOnly()) handleReset(); });
  server.on("/time", HTTP_POST, [] { if (jsonOnly()) handleTime(); });
  server.on("/games", handleGames);
  server.on("/game", HTTP_POST, [] { if (jsonOnly()) handleGame(); });
  server.on("/aireset", HTTP_POST, [] { if (jsonOnly()) handleAiReset(); });
  server.onNotFound(redirectHome);
  server.begin();
  if (MDNS.begin(gHost)) MDNS.addService("http", "tcp", 80);     // http://momowo-xxxx.local
}

// Unit names from the factory MAC address: unique, so many Momowos can share a network
static void makeNames() {
  uint64_t m = ESP.getEfuseMac();
  uint8_t a = (m >> 32) & 0xFF, b = (m >> 40) & 0xFF;
  snprintf(gHost, sizeof(gHost), "momowo-%02x%02x", a, b);
  snprintf(gApSsid, sizeof(gApSsid), "Momowo-%02X%02X", a, b);
}

static void startSetupMode(bool automatic) {
  if (gSetupMode) return;
  snprintf(gApPass, sizeof(gApPass), "%08lu", (unsigned long)(esp_random() % 100000000UL));
  WiFi.mode(WIFI_AP_STA);                   // keep trying home WiFi alongside
  if (WiFi.status() != WL_CONNECTED) WiFi.setAutoReconnect(false);   // its channel hops drop the phone
  if (!WiFi.softAP(gApSsid, gApPass)) Serial.println("[setup] could not start the setup WiFi");
#if CONFIG_IDF_TARGET_ESP32C3
  WiFi.setTxPower(WIFI_POWER_8_5dBm);       // C3 SuperMini: at full power phones often can't join
#endif
  dnsServer.start(53, "*", WiFi.softAPIP());
  gSetupAuto = automatic;
  gSetupAt = millis();
  gSetupMode = true;
  Serial.printf("[setup] WiFi \"%s\", password %s, page http://192.168.4.1\n", gApSsid, gApPass);
}

static void stopSetupMode() {
  if (!gSetupMode) return;
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  gSetupMode = false;
}

// QR codes for the setup screen (Espressif's encoder, from core 3.x)
static uint8_t qrSize = 0, qrBits[(29 * 29 + 7) / 8];   // up to version 3 (29 x 29)
static char qrText[80] = "";

static void qrStore(esp_qrcode_handle_t q) {
  qrSize = esp_qrcode_get_size(q);
  if (qrSize > 29) { qrSize = 0; return; }
  memset(qrBits, 0, sizeof(qrBits));
  for (int y = 0; y < qrSize; y++)
    for (int x = 0; x < qrSize; x++)
      if (esp_qrcode_get_module(q, x, y)) qrBits[(y * qrSize + x) >> 3] |= 1 << ((y * qrSize + x) & 7);
}

static void makeQr(const char* text) {
  if (!strcmp(text, qrText)) return;        // already made
  strlcpy(qrText, text, sizeof(qrText));
  esp_qrcode_config_t cfg;
  cfg.display_func = qrStore;
  cfg.max_qrcode_version = 3;
  cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;
  if (esp_qrcode_generate(&cfg, text) != ESP_OK) qrSize = 0;
}

// Setup screen: QR code on the left (dark modules on a light square, as phone cameras
// expect), instructions on the right
static void drawSetupScreen(uint32_t now) {
  static uint32_t lastCheck = 0;
  static bool phone = false;
  if (now - lastCheck > 500) { lastCheck = now; phone = WiFi.softAPgetStationNum() > 0; }
  char buf[80];
  if (phone) snprintf(buf, sizeof(buf), "http://192.168.4.1");
  else snprintf(buf, sizeof(buf), "WIFI:T:WPA;S:%s;P:%s;;", gApSsid, gApPass);
  makeQr(buf);

  int box = 0;
  if (qrSize) {
    const int scale = 2, quiet = qrSize <= 25 ? 2 : 1;
    box = (qrSize + 2 * quiet) * scale;
    int x0 = 0, y0 = (SCREEN_H - box) / 2;
    display.fillRect(x0, y0, box, box, SH110X_WHITE);
    for (int y = 0; y < qrSize; y++)
      for (int x = 0; x < qrSize; x++)
        if (qrBits[(y * qrSize + x) >> 3] & (1 << ((y * qrSize + x) & 7)))
          display.fillRect(x0 + (quiet + x) * scale, y0 + (quiet + y) * scale, scale, scale, SH110X_BLACK);
  }

  bool second = (now / 4000) % 2;           // the text beside the QR code, in the chosen language
  char text[160];
  if (!phone && !second) snprintf(text, sizeof(text), "%s", T(S_SU_SCAN));
  else if (!phone)       snprintf(text, sizeof(text), T(S_SU_CREDS), gApSsid, gApPass);
  else if (!second)      snprintf(text, sizeof(text), "%s", T(S_SU_CONNECTED));
  else                   snprintf(text, sizeof(text), "%s", T(S_SU_IFNOT));
  const int pitch = langIsCjk() ? 16 : 10;  // taller Chinese/Japanese lines: 4 of them
  int i = 0;
  for (char* line = text; line && i < 6; i++) {
    char* nl = strchr(line, '\n');
    if (nl) *nl = 0;
    if (*line) textAt(line, box + 4, langIsCjk() ? 5 + i * pitch : 2 + i * pitch);   // CJK baselines 13/29/45/61
    line = nl ? nl + 1 : nullptr;
  }
}

// ============================ INPUT HANDLING =========================
static bool pageAvailable(Page p) {         // internet-only pages hide only if WiFi isn't set up
  bool wifiSet = WIFI_SSID[0] || WIFI2_SSID[0];
  if (p == P_WEATHER) return wifiSet || getWeather().valid;
  if (p == P_NEWS) return wifiSet || gNews.count > 0;
  return true;
}

// A pat (a hold on the buddy, or a tap on the case with the motion sensor): love eyes, hearts,
// and a reply. Lots of pats in a short time: a gentle nudge towards the breathing exercise.
static void patBuddy(uint32_t now) {
  static uint32_t recent[5] = {0};
  static uint32_t lastNudge = 0;
  memmove(recent, recent + 1, sizeof(recent) - sizeof(recent[0]));
  recent[4] = now;
  gPatsToday++;
  prefs.putInt("mp_n", gPatsToday);         // remembered through power cuts
  setMood(M_LOVE, 2500);
  B.heartsAt = now;
  const char* reply = T(S_PAT_THANKS);
  static char pick[sizeof(gFeel.pats[0])];
  if (xSemaphoreTake(gMutex, pdMS_TO_TICKS(20)) == pdTRUE) {   // the AI may be storing a new feeling
    uint8_t n = gFeel.patCount;
    if (gFeel.valid && n) { strlcpy(pick, gFeel.pats[esp_random() % n], sizeof(pick)); reply = pick; }
    xSemaphoreGive(gMutex);
  }
  if (getWeather().aqi > 150 && esp_random() % 2) reply = T(S_AQI_PAT);
  if (recent[0] && now - recent[0] < 40000 && now - lastNudge > 10UL * 60 * 1000) {
    reply = T(S_LOTS_PATS);                 // five pats in 40 s
    lastNudge = now;
  }
  announce(reply, 6000);
  PLAY(SND_PAT);
}

static void handleGesture(Gesture g, uint32_t now) {
  bool wasOff = !gDisplayOn;
  gLastInteraction = now;
  if (gNight) gNightAwakeUntil = now + 20000;
  if (wasOff) { setMood(M_SURPRISED, 900); return; }   // first touch just wakes
  if ((g == G_HINT || g == G_SETUP) && gameActive(now) &&   // a long grip while tilting or shaking
      ((gGame.game == GM_ROLL && gGame.rolling) ||
       (gGame.game == GM_TAP && gGame.tphase >= TP_WAIT && gGame.tphase <= TP_SHOW) ||
       (gGame.game == GM_QUEST && gGame.qphase >= QS_THINKING && gGame.qphase <= QS_STEP))) return;
  if (g == G_HINT) {                        // still holding after 4 s
    if (!gSetupMode) toastFor(T(S_KEEP_HOLDING), 3000);
    return;
  }
  if (g == G_SETUP) {                       // hold 6 s anywhere: open setup
    if (!gSetupMode) { startSetupMode(false); PLAY(SND_ON); }
    return;
  }
  if (gSetupMode) {                         // setup screen: a hold closes it (it also closes itself)
    if (g == G_LONG) { stopSetupMode(); toast(T(S_SETUP_CLOSED)); }
    return;
  }
  if (gameActive(now)) {                    // game view: a pat is welcome, a tap closes it
    if (gGame.game == GM_QUEST && gGame.qphase >= QS_THINKING && gGame.qphase <= QS_STEP) {   // quest moves
      // (no setup or "keep holding" mid-quest: a grip to tilt or shake may rest on the pad)
      if ((int32_t)(gTouchPressAt - gGame.qstepAt) < 0) return;   // began before this step: a grip or a leftover
      if (g == G_TAP || g == G_DOUBLE) questInput(QA_TAP, now);
      else if (g == G_LONG || g == G_VLONG) questInput(QA_HOLD, now);
      return;
    }
    if (gGame.game == GM_TAP) {             // quick tap: the pad is the game; a double tap leaves it
      if (gGame.tphase >= TP_WAIT && gGame.tphase <= TP_SHOW) return;   // its own presses, in tapTick
      if (g == G_TAP) tapStart(now);
      else if (g == G_DOUBLE) { gGame.game = GM_NONE; gPageEnter = now; }
      else if (g == G_LONG) { gameReact(M_LOVE, 2500, gGame.say, gGame.sayArg); PLAY(SND_PAT); }
      return;
    }
    if (g == G_LONG) { gameReact(M_LOVE, 2500, gGame.say, gGame.sayArg); PLAY(SND_PAT); }
    if (g == G_TAP && !(gGame.game == GM_ROLL && gGame.rolling)) { gGame.game = GM_NONE; gPageEnter = now; }
    return;
  }
  if (TOUCH_CLICK) PLAY(SND_CLICK);         // specific sounds below replace the click

  switch (g) {
    case G_TAP:
      do gPage = (Page)((gPage + 1) % P_COUNT); while (!pageAvailable(gPage));
      gPageEnter = now;
      if (gPage == P_BUDDY) setMood(M_SURPRISED, 500);
      break;
    case G_DOUBLE:
      gPage = P_BUDDY;
      gPageEnter = now;
      setMood(M_HAPPY, 1800);
      PLAY(SND_ON);
      break;
    case G_LONG:
      switch (gPage) {
        case P_BUDDY: patBuddy(now); break;
        case P_CLOCK:
          USE_24H = !USE_24H;
          prefs.putBool("h24", USE_24H);
          toast(USE_24H ? T(S_H24) : T(S_H12));
          break;
        case P_CAL:                         // match local moon sighting: 0 -> +1 -> -1
          if (!(CALS & CAL_ISLAMIC)) break;
          gHijriAdj = gHijriAdj >= 1 ? -1 : gHijriAdj + 1;
          prefs.putChar("hijriAdj", gHijriAdj);
          toast(gHijriAdj == 0 ? T(S_HIJRI_ADJ0) : gHijriAdj > 0 ? T(S_HIJRI_ADJP) : T(S_HIJRI_ADJM));
          break;
        case P_WEATHER: gReqWeather = true; toast(T(S_UPDATING)); break;
        case P_NEWS:    gReqNews = true; toast(T(S_UPDATING_NEWS)); break;
        case P_QUOTE:   gReqQuote = true; toast(T(S_NEW_QUOTE)); break;
        case P_FOCUS:   focusToggle(now); break;
        default: break;
      }
      break;
    case G_VLONG:                           // 2-6 s hold: resets focus; elsewhere it's a hold
      if (gPage == P_FOCUS && fState == F_IDLE) breatheStart(now);   // idle: breathe with me
      else if (gPage == P_FOCUS) focusReset();
      else handleGesture(G_LONG, now);      // sound is set on the phone page now, not the pad
      break;
    default: break;
  }
}

// ============================ MOTION =================================
static uint32_t gLastTouch = 0;

static void handleMotion(motion::Event ev, uint32_t now) {
  if (ev == motion::EV_PAT && now - gLastTouch < 1500) return;   // the touch pad already counted it
  if (ev == motion::EV_PAT && gPage != P_BUDDY) return;          // desk bumps don't leave a page
  gLastInteraction = now;
  if (gNight) gNightAwakeUntil = now + 20000;
  if (!gDisplayOn) { setMood(M_SURPRISED, 900); return; }        // first, just wake up
  switch (ev) {
    case motion::EV_PAT:
      patBuddy(now);
      break;
    case motion::EV_SHAKE:
      gShakesToday++;
      if (gPage == P_QUOTE) { gReqQuote = true; toast(T(S_NEW_QUOTE)); PLAY(SND_ON); break; }   // shake for a quote
      gPage = P_BUDDY;
      gPageEnter = now;
      setMood(M_DIZZY, 3000);
      announce(T(S_DIZZY), 4000);
      PLAY(SND_DIZZY);
      break;
    case motion::EV_DROP:
      setMood(M_SURPRISED, 1500);
      announce(T(S_DROP), 4000);
      PLAY(SND_DIZZY);
      break;
    case motion::EV_TILT:
      setMood(M_SURPRISED, 1200);
      if (gPage == P_BUDDY) announce(T(S_TILT), 4000);
      break;
    default: break;
  }
}

// Prayer reminders: a soft chime and a note at each prayer time (quiet at night, when muted,
// and during a focus session, like every sound)
static void prayerReminder(const struct tm& t) {
  static int lastKey = -1;
  if (!PRAY_REMIND || !(CALS & CAL_ISLAMIC) || !gPray.valid) return;
  int nowMin = t.tm_hour * 60 + t.tm_min;
  static const uint8_t ORDER[] = {astro::FAJR, astro::DHUHR, astro::ASR, astro::MAGHRIB, astro::ISHA};
  for (uint8_t i : ORDER) {
    if (isnan(gPray.t[i]) || (int)lroundf(gPray.t[i]) != nowMin) continue;
    int key = t.tm_yday * 10 + i;
    if (key == lastKey) return;
    lastKey = key;
    char msg[96];
    if (i == astro::MAGHRIB && gCal.hm == 9) strlcpy(msg, T(S_IFTAR_NOW), sizeof(msg));
    else snprintf(msg, sizeof(msg), T(S_PRAYER_NOW), PRAYER_NAME[LANG][i]);
    if (!gSetupMode) announce(msg, 30000);
    PLAY(SND_PRAYER);
    return;
  }
}

// ============================ MINI-GAMES =============================
// Played on the phone (/games). Momowo is the other player: it makes its moves, keeps the score
// and reacts on its screen. The game view closes after 2 minutes without a move, or with a tap.
static const uint32_t GAME_IDLE_MS = 120000;
static const Note SND_ARROW0[] = {{523, 220}};
static const Note SND_ARROW1[] = {{659, 220}};
static const Note SND_ARROW2[] = {{784, 220}};
static const Note SND_ARROW3[] = {{1047, 220}};

static bool gameActive(uint32_t now) { return gGame.game != GM_NONE && now - gGame.last < GAME_IDLE_MS; }

static void gameReact(Mood m, uint32_t ms, Str say, int arg) {
  gGame.mood = m;
  gGame.moodUntil = gLoopNow + ms;
  gGame.say = say;
  gGame.sayArg = arg;
}

static void playArrow(uint8_t d) {
  switch (d) {
    case 0: PLAY(SND_ARROW0); break;
    case 1: PLAY(SND_ARROW1); break;
    case 2: PLAY(SND_ARROW2); break;
    default: PLAY(SND_ARROW3); break;
  }
}

// Tic-tac-toe: Momowo wins when it can, usually blocks, and otherwise plays a fair game
static const uint8_t TTT_LINES[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
                                        {0, 4, 8}, {2, 4, 6}};
static uint8_t tttWinner(const uint8_t* b) {     // 0 still playing, 1 you, 2 Momowo, 3 draw
  for (auto& l : TTT_LINES) if (b[l[0]] && b[l[0]] == b[l[1]] && b[l[1]] == b[l[2]]) return b[l[0]];
  for (int i = 0; i < 9; i++) if (!b[i]) return 0;
  return 3;
}
static int tttFinish(uint8_t* b, uint8_t who) {  // the square that wins a line for who, or -1
  for (int i = 0; i < 9; i++) {
    if (b[i]) continue;
    b[i] = who;
    bool won = tttWinner(b) == who;
    b[i] = 0;
    if (won) return i;
  }
  return -1;
}
static int tttMove(uint8_t* b) {
  int m = tttFinish(b, 2);
  if (m < 0 && esp_random() % 100 < 85) m = tttFinish(b, 1);   // misses a block now and then
  if (m < 0 && !b[4] && esp_random() % 100 < 70) m = 4;
  if (m < 0) {
    int free[9], n = 0;
    for (int i = 0; i < 9; i++) if (!b[i]) free[n++] = i;
    if (n) m = free[esp_random() % n];
  }
  return m;
}

// Memory: Momowo shows the arrows one by one (faster each level); the owner repeats them
static uint32_t simonStep() { return gGame.len < 14 ? 700 - gGame.len * 25 : 350; }
static uint32_t simonShowMs() { return 900 + gGame.len * simonStep(); }
static void simonNext(uint32_t now) {
  if (gGame.len < sizeof(gGame.seq)) gGame.seq[gGame.len++] = esp_random() % 4;
  gGame.pos = 0;
  gGame.showing = true;
  gGame.shown = 255;
  gGame.at = now;
  gGame.say = S_GM_WATCH;                   // keeps any reaction to the last round
  gGame.sayArg = 0;
}

// Star roll: 30 seconds to roll the ball onto as many stars as possible; from 3 stars a little
// rain cloud drifts about, and bumping into it costs a star
static const uint32_t ROLL_MS = 30000;
static int rollTop() { return langIsCjk() ? 17 : 10; }   // the board, under the page header
static void rollPlaceStar() {
  const int top = rollTop();
  for (int i = 0; i < 20; i++) {            // somewhere new, not right under the ball
    gGame.sx = 8 + esp_random() % (SCREEN_W - 16);
    gGame.sy = top + 6 + esp_random() % (SCREEN_H - top - 12);
    float dx = gGame.sx - gGame.bx, dy = gGame.sy - gGame.by;
    if (dx * dx + dy * dy > 30 * 30) break;
  }
  gGame.starAt = gLoopNow;
}

static void rollTick(uint32_t now) {
  float dt = (now - gGame.tickAt) / 1000.0f;
  gGame.tickAt = now;
  if (dt > 0.05f) dt = 0.05f;
  if ((int32_t)(now - gGame.startAt) < 0) {   // counting down: however Momowo is held now is level
    for (int i = 0; i < 3; i++) gGame.holdRef[i] = gMs.grav[i];
    return;
  }
  if ((int32_t)(now - gGame.endAt) >= 0) { // time's up
    gGame.rolling = false;
    if (gGame.stars > gGame.bestStars) {
      gGame.bestStars = gGame.stars;
      gGame.newBest = true;
      prefs.putUShort("rollbest", gGame.bestStars);
      gameReact(M_LOVE, 6000, S_GM_NEW_BEST, gGame.stars);
      PLAY(SND_DONE);
    } else {
      gameReact(M_HAPPY, 6000, S_GM_TIME_UP, gGame.stars);
      PLAY(SND_HOUR);
    }
    return;
  }
  // the ball rolls downhill, the same way the eyes roll
  float ax, ay;
  motion::tiltFrom(gMs, gGame.holdRef, ax, ay);
  if (MOTION_MIRROR) ax = -ax;
  gGame.vx += ax * 170 * dt;
  gGame.vy += ay * 170 * dt;
  const float f = 1 - 1.2f * dt;            // a little friction
  gGame.vx *= f;
  gGame.vy *= f;
  const float sp = sqrtf(gGame.vx * gGame.vx + gGame.vy * gGame.vy);
  if (sp > 110) { gGame.vx *= 110 / sp; gGame.vy *= 110 / sp; }
  gGame.bx += gGame.vx * dt;
  gGame.by += gGame.vy * dt;
  const float x0 = 4, x1 = SCREEN_W - 5, y0 = rollTop() + 4, y1 = SCREEN_H - 5;
  if (gGame.bx < x0) { gGame.bx = x0; gGame.vx = -gGame.vx * 0.5f; }
  if (gGame.bx > x1) { gGame.bx = x1; gGame.vx = -gGame.vx * 0.5f; }
  if (gGame.by < y0) { gGame.by = y0; gGame.vy = -gGame.vy * 0.5f; }
  if (gGame.by > y1) { gGame.by = y1; gGame.vy = -gGame.vy * 0.5f; }
  float dx = gGame.sx - gGame.bx, dy = gGame.sy - gGame.by;
  if (dx * dx + dy * dy < 7 * 7) {          // a star!
    gGame.stars++;
    PLAY(SND_ON);
    gameReact(M_HAPPY, 600, S_GM_TILT_ME, 0);
    rollPlaceStar();
  }
  if (gGame.stars >= 3) {                   // the rain cloud, a bit faster with every star
    const float speed = 1 + gGame.stars * 0.06f;
    gGame.cx += gGame.cvx * speed * dt;
    gGame.cy += gGame.cvy * speed * dt;
    if (gGame.cx < 8 || gGame.cx > SCREEN_W - 9) { gGame.cvx = -gGame.cvx; gGame.cx = constrain(gGame.cx, 8.0f, SCREEN_W - 9.0f); }
    if (gGame.cy < y0 + 2 || gGame.cy > y1 - 2) { gGame.cvy = -gGame.cvy; gGame.cy = constrain(gGame.cy, y0 + 2, y1 - 2); }
    if (fabsf(gGame.cx - gGame.bx) < 9 && fabsf(gGame.cy - gGame.by) < 7 && now - gGame.hurtAt > 1500) {
      gGame.hurtAt = now;
      if (gGame.stars) gGame.stars--;
      gGame.vx = -gGame.vx - (gGame.cx > gGame.bx ? 40 : -40);   // bounced away
      gGame.vy = -gGame.vy;
      gameReact(M_SAD, 900, S_GM_TILT_ME, 0);
      PLAY(SND_PAUSE);
    }
  }
}

// Riddles: plain letters compared without case, spaces or punctuation
// Lower case, punctuation and spaces become single spaces between the words
static void riddleNorm(const char* s, char* o, size_t n) {
  size_t k = 0;
  bool gap = false;
  for (const uint8_t* p = (const uint8_t*)s; *p && k + 2 < n; p++) {
    uint8_t c = *p;
    bool word = true;
    if (c < 0x80) {
      word = isalnum(c);
      if (word) c = tolower(c);
    } else if ((c == 0xE3 && p[1] == 0x80 && p[2] >= 0x80 && p[2] <= 0x82) ||   // 　、。
               (c == 0xEF && p[1] == 0xBC && ((p[2] >= 0x81 && p[2] <= 0x8F) || (p[2] >= 0x9A && p[2] <= 0xA0)))) {   // ！，？…
      word = false;
      p += 2;
    }
    if (!word) { gap = k > 0; continue; }
    if (gap) { o[k++] = ' '; gap = false; }
    o[k++] = c;
    if (c == 0xC3 && p[1] >= 0x80 && p[1] <= 0x9E && p[1] != 0x97 && k + 2 < n) o[k++] = *++p + 0x20;   // É is é
  }
  o[k] = 0;
}

static bool riddleMatch(const char* guess) {
  char g[64], a[64];
  riddleNorm(guess, g, sizeof(g));
  if (!g[0]) return false;
  xSemaphoreTake(gMutex, portMAX_DELAY);
  String all = gGame.answer;
  all += "|";
  all += gGame.alts;
  xSemaphoreGive(gMutex);
  int from = 0;
  while (from <= (int)all.length()) {
    int bar = all.indexOf('|', from);
    if (bar < 0) bar = all.length();
    riddleNorm(all.substring(from, bar).c_str(), a, sizeof(a));
    if (a[0]) {
      if (!strcmp(g, a)) return true;
      char G[72], A[72];                    // the answer as whole words: "a cat" is cat, "elephant" isn't ant
      snprintf(G, sizeof(G), " %s ", g);
      snprintf(A, sizeof(A), " %s ", a);
      if (strstr(G, A)) return true;
    }
    from = bar + 1;
  }
  return false;
}

// The AI's answer can come while another game is open: react only on the riddle screen
static bool riddleReact(Mood m, uint32_t ms, Str say, int arg) {
  if (gGame.game != GM_RIDDLE) return false;
  gameReact(m, ms, say, arg);
  return true;
}

static void riddleSolved() {
  gGame.rphase = RD_SOLVED;
  if (riddleReact(M_LOVE, 8000, S_GM_GOT_IT, gGame.rtries)) PLAY(SND_DONE);
}

static void riddleWrong() {
  gGame.rphase = RD_ASKING;
  if (riddleReact(M_SURPRISED, 2500, S_GM_NOT_QUITE, 0)) PLAY(SND_PAUSE);
}

// netTask: make up a riddle (any language, a topic picked here so they vary), or judge a guess
// The player asked for this riddle just now, so it may use AI during sleep hours too
static void riddleDo();
static void riddleWork() {
  gAiForUser = true;
  riddleDo();
  gAiForUser = false;
}

static void riddleDo() {
  const uint8_t req = gRiddleReq;
  if (!req) return;
  if (req == 1) {
    static const char* const TOPICS[] = {"an animal", "a food or drink", "a thing found at home", "something in nature",
      "the sky or the weather", "a toy or game", "a musical instrument", "a vehicle", "a job", "a fruit or vegetable",
      "a part of the body", "a sport", "something at school", "a tool", "an insect or sea creature"};
    static char recent[6][32];
    static uint8_t rn = 0;
    String p = "Make up one fun, family-friendly riddle in ";
    p += LANG_NAME[LANG];
    p += " for a guessing game. Its answer is ";
    p += TOPICS[esp_random() % (sizeof(TOPICS) / sizeof(TOPICS[0]))];
    p += " that everyone knows, in one or two words. Make it clever but solvable, and fair to people from any "
         "country or culture. Don't use a well-known riddle word for word.";
    bool any = false;
    for (auto& r : recent) {
      if (!r[0]) continue;
      p += any ? ", " : " Not these answers: ";
      p += r;
      any = true;
    }
    p += ". Reply with JSON only (keys in English): {\"q\":\"the riddle, at most 30 words (70 characters in "
         "Chinese or Japanese)\",\"a\":\"the answer\",\"alts\":[\"up to 4 other ways to say the answer\"],"
         "\"hint\":\"a short hint that doesn't give it away, at most 10 words\"}";
    static char out[900];
    bool ok = askAI(p.c_str(), out, sizeof(out), 400, true);
    JsonDocument d;
    char* a = strchr(out, '{');
    char* b = strrchr(out, '}');
    ok = ok && a && b && b > a && !deserializeJson(d, (const char*)a, (size_t)(b - a + 1));
    const char* q = ok ? (d["q"] | "") : "";
    const char* ans = ok ? (d["a"] | "") : "";
    ok = q[0] && ans[0] && screenCanShow(q) && !isRefusal(q);
    xSemaphoreTake(gMutex, portMAX_DELAY);
    if (ok) {
      utf8Copy(gGame.riddle, q, sizeof(gGame.riddle));
      utf8Copy(gGame.answer, ans, sizeof(gGame.answer));
      utf8Copy(gGame.hintText, d["hint"] | "", sizeof(gGame.hintText));
      gGame.alts[0] = 0;
      for (JsonVariant v : d["alts"].as<JsonArray>()) {
        const char* s = v | "";
        if (!s[0] || strlen(gGame.alts) + strlen(s) + 2 >= sizeof(gGame.alts)) continue;
        if (gGame.alts[0]) strlcat(gGame.alts, "|", sizeof(gGame.alts));
        strlcat(gGame.alts, s, sizeof(gGame.alts));
      }
    }
    xSemaphoreGive(gMutex);
    if (ok) {
      utf8Copy(recent[rn], ans, sizeof(recent[0]));
      rn = (rn + 1) % 6;
      gGame.rAt = gLoopNow;
      gGame.rphase = RD_ASKING;
      riddleReact(M_HAPPY, 1500, S_GM_YOUR_TURN, 0);
    } else {
      gGame.rphase = RD_FAILED;
      riddleReact(M_SAD, 3000, S_GM_NO_AI, 0);
    }
  } else {
    String p = "Riddle: \"";
    xSemaphoreTake(gMutex, portMAX_DELAY);
    p += gGame.riddle;
    p += "\"\nIts answer: \"";
    p += gGame.answer;
    p += "\" (also fine: ";
    p += gGame.alts[0] ? gGame.alts : "-";
    p += ").\nA player guessed: \"";
    for (const char* c = gGame.rguess; *c; c++) if (*c != '"') p += *c;
    xSemaphoreGive(gMutex);
    p += "\". Is the guess right: the same thing, a close synonym, or the answer with a small typo or in "
         "another language? A different or vaguer thing is not right. Reply with YES or NO only.";
    char r[16];
    bool yes = askAI(p.c_str(), r, sizeof(r), 5, false);
    const char* y = r;
    while (*y == ' ' || *y == '\n') y++;
    yes = yes && !strncasecmp(y, "YES", 3);
    if (gGame.rphase == RD_JUDGING) {
      if (yes) riddleSolved();
      else riddleWrong();
    }
  }
  gRiddleReq = 0;
}

// Quest: the AI tells a short adventure on the screen. Each moment asks the player to tap Momowo's
// head, hold it, tilt Momowo the way the arrow points or shake it, before the time bar runs out.
// Three hearts; a quick answer scores double. Without the AI it's the same game without the story,
// and without the motion sensor only taps and holds are asked for.
static const uint8_t QUEST_HEARTS = 3;
static const char* const QA_NAME[QA_COUNT] = {"tap", "hold", "left", "right", "up", "down", "shake"};
static uint8_t questRandomAct(uint8_t prev) {
  const uint8_t n = gMotion ? QA_COUNT : 2;
  uint8_t a;
  do a = esp_random() % n; while (a == prev && n > 1);
  return a;
}
static void questOffline() {                // no story: actions only
  uint8_t prev = 255;
  for (uint8_t i = 0; i < QUEST_MAX; i++) prev = gGame.qact[i] = questRandomAct(prev);
  xSemaphoreTake(gMutex, portMAX_DELAY);
  for (auto& t : gGame.qtext) t[0] = 0;
  gGame.qtitle[0] = gGame.qend[0][0] = gGame.qend[1][0] = 0;
  xSemaphoreGive(gMutex);
  gGame.qn = QUEST_MAX;
  gGame.qstory = false;
}

// netTask: ask the AI for the story (the player asked for it, so this works at night too)
static void questWork() {
  static const char* const THEMES[] = {"a jungle trek", "a trip to the Moon", "a deep-sea dive", "a busy night market",
    "a snowy mountain climb", "a desert caravan", "a pirate ship voyage", "a dragon boat race", "a magic library",
    "a valley of friendly dinosaurs", "a lantern festival", "a robot factory", "a rainforest river", "a bakery in a rush",
    "a hot-air balloon ride", "a coral reef", "a treasure hunt in an old castle", "a space station"};
  bool ok = gWifiUp && AI_API_KEY[0];
  static char out[1800];
  JsonDocument d;
  if (ok) {
    String p = "Write a tiny interactive adventure for a family game played on a small desk companion, in ";
    p += LANG_NAME[LANG];
    p += ". Theme: ";
    if (OWNER_HOBBIES[0] && esp_random() % 3 == 0) {
      p += "something to do with the player's hobbies (";
      p += OWNER_HOBBIES;
      p += ")";
    } else {
      p += THEMES[esp_random() % (sizeof(THEMES) / sizeof(THEMES[0]))];
    }
    p += ". It has exactly 8 moments. For each, pick the action that fits what happens, from: tap (a quick touch "
         "on the companion's head: knock, press, catch, poke), hold (keep a finger on its head for a second: push, "
         "hold on, hide)";
    if (gMotion) p += ", left or right (tilt it left or right: dodge, steer, lean), up or down (tilt it away from "
                      "or towards the player: climb, jump, dive, pour), shake (shake it: shake off, stir, ring a bell)";
    p += ". Each moment says what is happening in at most 8 words (at most 16 characters in Chinese or Japanese), "
         "speaking to the player, exciting but gentle, suitable for children and fair to people of any culture. "
         "Don't name the action or any buttons. Mix the actions and don't use the same one twice in a row. "
         "Reply with JSON only (keys and action names in English): {\"title\":\"at most 4 words\",\"steps\":"
         "[{\"t\":\"what happens\",\"a\":\"tap\"}],\"win\":\"a happy ending, at most 10 words\",\"lose\":"
         "\"a kind ending if the player runs out of hearts, at most 10 words\"}";
    gAiForUser = true;
    ok = askAI(p.c_str(), out, sizeof(out), 900, true);
    gAiForUser = false;
    char* a = strchr(out, '{');
    char* b = strrchr(out, '}');
    ok = ok && a && b && b > a && !deserializeJson(d, (const char*)a, (size_t)(b - a + 1));
  }
  uint8_t n = 0;
  if (ok) {
    xSemaphoreTake(gMutex, portMAX_DELAY);
    uint8_t prev = 255;
    for (JsonVariant s : d["steps"].as<JsonArray>()) {
      if (n >= QUEST_MAX) break;
      const char* t = s["t"] | "";
      const char* an = s["a"] | "";
      uint8_t act = 255;
      for (uint8_t k = 0; k < QA_COUNT; k++) if (!strcasecmp(an, QA_NAME[k])) act = k;
      if (act == 255 || (!gMotion && act > QA_HOLD)) act = questRandomAct(prev);
      if (!t[0] || !strcasecmp(t, "SKIP") || !screenCanShow(t)) t = "";   // (parsed JSON: not a refusal)
      utf8Copy(gGame.qtext[n], t, sizeof(gGame.qtext[0]));
      prev = gGame.qact[n++] = act;
    }
    const char* title = d["title"] | "";
    const char* win = d["win"] | "";
    const char* lose = d["lose"] | "";
    utf8Copy(gGame.qtitle, screenCanShow(title) && !isRefusal(title) ? title : "", sizeof(gGame.qtitle));
    utf8Copy(gGame.qend[0], screenCanShow(win) && !isRefusal(win) ? win : "", sizeof(gGame.qend[0]));
    utf8Copy(gGame.qend[1], screenCanShow(lose) && !isRefusal(lose) ? lose : "", sizeof(gGame.qend[1]));
    xSemaphoreGive(gMutex);
  }
  if (n >= 4) {
    gGame.qn = n;
    gGame.qstory = true;
  } else {
    questOffline();                         // no story this time, but still a game
  }
  gGame.qreadyAt = 0;                       // loop() starts it
  gGame.qphase = QS_READY;
  gQuestReq = false;
}

static void questReady(uint32_t now) {
  gGame.qphase = QS_READY;
  gGame.qreadyAt = now | 1;
  gGame.qstep = gGame.qscore = 0;
  gGame.qhearts = QUEST_HEARTS;
  gGame.qwaitUntil = 0;
  gGame.qwon = gGame.qnewBest = false;
  gameReact(M_HAPPY, 3000, S_GM_READY, 0);
  PLAY(SND_WAKE);
}

static void questStepStart(uint32_t now) {
  if (gGame.qstep == 0)                     // however the player holds Momowo now counts as level
    for (int i = 0; i < 3; i++) gGame.holdRef[i] = gMs.grav[i];
  gGame.qphase = QS_STEP;
  gGame.qstepAt = now;
  gGame.qwin = max<int32_t>(2600, 6500 - gGame.qstep * 450) + (gGame.qstory ? 1000 : 0);   // time to read it
  gGame.qarmed = false;                     // a tilt counts once Momowo has been level
  gGame.qwaitUntil = 0;
  playArrow(gGame.qstep % 4);
}

static void questEnd(uint32_t now) {
  gGame.qphase = QS_DONE;
  gGame.qwon = gGame.qhearts > 0;
  gGame.at = now;
  if (gGame.qscore > gGame.qbest) {
    gGame.qbest = gGame.qscore;
    gGame.qnewBest = true;
    prefs.putUChar("questbest", gGame.qbest);
  }
  if (gGame.qwon) { gameReact(M_LOVE, 8000, S_GM_QUEST_WIN, gGame.qscore); PLAY(SND_DONE); }
  else { gameReact(M_SAD, 6000, S_GM_QUEST_LOSE, gGame.qscore); PLAY(SND_SAD); }
}

static void questResult(bool ok, uint32_t now) {
  if (ok) {
    gGame.qscore += (now - gGame.qstepAt) * 5 < gGame.qwin * 2 ? 2 : 1;   // in the first 40% of the time: double
    gameReact(M_HAPPY, 700, S_GM_NICE, 0);
    PLAY(SND_ON);
  } else {
    if (gGame.qhearts) gGame.qhearts--;
    gameReact(M_SAD, 1000, S_GM_MISS, 0);
    PLAY(SND_PAUSE);
  }
  gGame.qok = ok;
  gGame.qwaitUntil = (now + (ok ? 600 : 1000)) | 1;
}

// A touch or a shake from loop(). Only a tap for a hold (or the other way round) counts as wrong:
// gripping Momowo to tilt or shake it may touch the pad, and a quick tilt can look like a shake.
static void questInput(uint8_t act, uint32_t now) {
  if (gGame.game != GM_QUEST || gGame.qphase != QS_STEP || gGame.qwaitUntil) return;
  gGame.last = now;
  const uint8_t want = gGame.qact[gGame.qstep];
  if (act == want) questResult(true, now);
  else if (act <= QA_HOLD && want <= QA_HOLD) questResult(false, now);
}

// ---- Quick tap: Momowo waits, then flashes and beeps; tap its head as fast as you can.
// Tapping early is "too soon" and the round is played again. From round 2 it may pretend
// (eyes wide, no flash, no beep) to catch you out.
static void tapRound(uint32_t now) {
  gGame.tphase = TP_WAIT;
  gGame.tgoAt = now + 1500 + esp_random() % 3000;
  gGame.tfakeAt = 0;
  if (gGame.tround >= 1 && esp_random() % 100 < 40)   // a pretend signal well before the real one
    gGame.tfakeAt = now + 700 + esp_random() % (gGame.tgoAt - now - 1400);
  gGame.tseen = gTouchPressAt;              // presses before this moment don't count
  gameReact(M_NORMAL, 0, S_GM_WAIT, 0);
}

static void tapStart(uint32_t now) {
  gGame.tround = 0;
  gGame.tnewBest = false;
  gGame.tavg = 0;
  gGame.last = now;
  tapRound(now);
  PLAY(SND_WAKE);
}

static void tapShow(int16_t ms, uint32_t now) {
  gGame.tlast = ms;
  gGame.tphase = TP_SHOW;
  gGame.tshowUntil = now + (ms < 0 ? 1600 : 1300);
  if (ms == -1) { gameReact(M_SURPRISED, 1600, S_GM_TOO_SOON, 0); PLAY(SND_PAUSE); }
  else if (ms == -2) { gameReact(M_SAD, 1300, S_GM_TOO_SLOW, 0); PLAY(SND_PAUSE); }
  else { gameReact(ms < 300 ? M_LOVE : M_HAPPY, 1300, S_GM_MS, ms); PLAY(SND_PAT); }
}

static void tapTick(uint32_t now) {
  const uint8_t ph = gGame.tphase;
  if (ph == TP_IDLE || ph == TP_DONE) return;
  gGame.last = now;                         // playing on the pad: stay in the game
  const bool pressed = gTouchPressAt != gGame.tseen;
  if (pressed) gGame.tseen = gTouchPressAt;
  if (ph == TP_WAIT) {
    if (pressed) { tapShow(-1, now); return; }   // too soon: this round again
    if ((int32_t)(now - gGame.tgoAt) >= 0) {
      gGame.tphase = TP_GO;
      gGame.tgoAt = now;                    // from when the flash and beep really start
      PLAY(SND_GO);
    }
    return;
  }
  if (ph == TP_GO) {
    int32_t ms = (int32_t)(gGame.tseen - gGame.tgoAt);
    if (pressed && ms < 90) { tapShow(-1, now); return; }   // faster than anyone can react: a guess
    if (pressed || now - gGame.tgoAt >= 1500) {
      int16_t r = pressed ? (int16_t)ms : -2;
      gGame.tms[gGame.tround++] = pressed ? (uint16_t)ms : 1500;   // too slow counts as 1.5 s
      tapShow(r, now);
    }
    return;
  }
  if ((int32_t)(now - gGame.tshowUntil) < 0) return;   // TP_SHOW: the result, for a moment
  if (gGame.tround < TAP_ROUNDS) { tapRound(now); return; }
  uint32_t sum = 0;
  for (int i = 0; i < TAP_ROUNDS; i++) sum += gGame.tms[i];
  gGame.tavg = sum / TAP_ROUNDS;
  gGame.tphase = TP_DONE;
  if (!gGame.tbest || gGame.tavg < gGame.tbest) {
    gGame.tbest = gGame.tavg;
    gGame.tnewBest = true;
    prefs.putUShort("tapbest", gGame.tbest);
  }
  gameReact(gGame.tnewBest ? M_LOVE : M_HAPPY, 8000, gGame.tnewBest ? S_GM_TAP_BEST : S_GM_TAP_AVG, gGame.tavg);
  PLAY(SND_DONE);
}

static void questTick(uint32_t now) {
  const uint8_t ph = gGame.qphase;
  if (ph == QS_READY) {
    if (!gGame.qreadyAt) questReady(now);
    gGame.last = now;                       // the phone may be put down: stay in the game
    if ((int32_t)(now - gGame.qreadyAt) >= 3000) questStepStart(now);
    return;
  }
  if (ph != QS_STEP) return;
  gGame.last = now;
  if (gGame.qwaitUntil) {                   // showing the tick or the cross
    if ((int32_t)(now - gGame.qwaitUntil) < 0) return;
    gGame.qwaitUntil = 0;
    if (!gGame.qhearts || gGame.qstep + 1 >= gGame.qn) { questEnd(now); return; }
    gGame.qstep++;
    questStepStart(now);
    return;
  }
  if (now - gGame.qstepAt >= gGame.qwin) { questResult(false, now); return; }
  const uint8_t want = gGame.qact[gGame.qstep];
  if (want == QA_HOLD && digitalRead(TOUCH_PIN) == HIGH && (int32_t)(gTouchPressAt - gGame.qstepAt) >= 0 &&
      now - gTouchPressAt >= LONG_MS) {     // held long enough: no need to wait for the finger to lift
    questResult(true, now);
    return;
  }
  if (want < QA_LEFT || want > QA_DOWN || !gMotion) return;
  float tx, ty;
  motion::tiltFrom(gMs, gGame.holdRef, tx, ty);
  if (MOTION_MIRROR) tx = -tx;
  if (!gGame.qarmed) {
    if (fabsf(tx) < 0.3f && fabsf(ty) < 0.3f) gGame.qarmed = true;
    return;
  }
  const float v[4] = {-tx, tx, -ty, ty};    // left, right, up, down: the way star roll's ball rolls
  const uint8_t k = want - QA_LEFT;
  if (v[k] > 0.5f) { questResult(true, now); return; }
  for (uint8_t j = 0; j < 4; j++)
    if (j != k && v[j] > 0.75f) { questResult(false, now); return; }   // clearly the wrong way
}

// The phone's moves: {"g": "rps"|"ttt"|"guess"|"simon"|"roll"|"riddle"|"quest", "a": action, ...}; replies with the state
static void handleGame() {
  JsonDocument d, r;
  if (deserializeJson(d, server.arg("plain"))) { server.send(400, "text/plain", "Bad request"); return; }
  const uint32_t now = gLoopNow;
  const char* g = d["g"] | "";
  const char* a = d["a"] | "";
  if (!strcmp(a, "quit")) {                 // the phone left the games page
    gGame.game = GM_NONE;
    server.send(200, "application/json", "{\"ok\":true}");
    return;
  }
  uint8_t game = !strcmp(g, "rps") ? GM_RPS : !strcmp(g, "ttt") ? GM_TTT : !strcmp(g, "guess") ? GM_GUESS
               : !strcmp(g, "simon") ? GM_SIMON : !strcmp(g, "roll") ? GM_ROLL
               : !strcmp(g, "riddle") ? GM_RIDDLE : !strcmp(g, "quest") ? GM_QUEST
               : !strcmp(g, "tap") ? GM_TAP : GM_NONE;
  if (game == GM_NONE) { server.send(400, "text/plain", "Bad request"); return; }
  // quick tap's page only follows along: its polling doesn't reopen a game closed on Momowo
  const bool peek = game == GM_TAP && !strcmp(a, "state") && !(gGame.game == GM_TAP && gameActive(now));
  if (!peek) {
  gLastInteraction = now;                   // keeps the screen awake
  if (gNight) gNightAwakeUntil = now + 30000;
  if (game != gGame.game || !gameActive(now)) {
    gGame.game = game;
    gameReact(M_HAPPY, 1500, S_GM_YOUR_TURN, 0);
    gGame.you = -1;
    if (game == GM_GUESS && !gGame.secret) a = "new";
    if (game == GM_SIMON) { gGame.len = 0; gGame.showing = false; }
    if (game == GM_ROLL) { gGame.rolling = false; gGame.endAt = 0; }
    if (game == GM_RIDDLE && !gRiddleReq && (gGame.rphase == RD_FAILED || now - gGame.last > 30UL * 60000))
      gGame.rphase = RD_NONE;                // otherwise the riddle is still there after another game
    if (game == GM_RIDDLE) gGame.rAt = now;  // back to it: its text from the top
    if (game == GM_QUEST && (gGame.qphase == QS_READY || gGame.qphase == QS_STEP)) gGame.qphase = QS_NONE;
    if (game == GM_TAP) { gGame.tphase = TP_IDLE; gameReact(M_HAPPY, 1500, S_GM_TAP_HOW, 0); }
  }
  gGame.last = now;
  }
  switch (game) {
    case GM_RPS: {
      int you = d["m"] | -1;
      if (you >= 0 && you <= 2) {
        gGame.you = you;
        gGame.me = esp_random() % 3;
        gGame.result = (you - gGame.me + 3) % 3;   // paper beats rock, scissors paper, rock scissors
        if (gGame.result == 1) gGame.wins++;
        else if (gGame.result == 2) gGame.losses++;
        gGame.at = now;
        gGame.sounded = false;
        gameReact(M_NORMAL, 0, S_GM_HMM, 0);
        r["me"] = gGame.me;
        r["res"] = gGame.result;
        r["ms"] = 1200;                     // Momowo shakes its fist first, then shows
      }
      r["wins"] = gGame.wins;
      r["losses"] = gGame.losses;
    } break;
    case GM_TTT: {
      if (!strcmp(a, "new")) {
        memset(gGame.board, 0, sizeof(gGame.board));
        gGame.over = 0;
        gGame.lastMove = 255;
        gameReact(M_HAPPY, 1200, S_GM_YOUR_TURN, 0);
      }
      int c = d["c"] | -1;
      if (c >= 0 && c < 9 && !gGame.over && !gGame.board[c]) {
        gGame.board[c] = 1;
        gGame.over = tttWinner(gGame.board);
        if (!gGame.over) {
          int m = tttMove(gGame.board);
          if (m >= 0) { gGame.board[m] = 2; gGame.lastMove = m; gGame.at = now; }
          gGame.over = tttWinner(gGame.board);
        }
        if (gGame.over == 1) { gameReact(M_SURPRISED, 2500, S_GM_YOU_WIN, 0); gGame.wins++; PLAY(SND_PAT); }
        else if (gGame.over == 2) { gameReact(M_HAPPY, 3000, S_GM_I_WIN, 0); gGame.losses++; PLAY(SND_ON); }
        else if (gGame.over == 3) gameReact(M_NORMAL, 0, S_GM_DRAW, 0);
        else gameReact(esp_random() & 1 ? M_HAPPY : M_SURPRISED, 700, S_GM_YOUR_TURN, 0);
      }
      JsonArray b = r["b"].to<JsonArray>();
      for (uint8_t v : gGame.board) b.add(v);
      r["over"] = gGame.over;
    } break;
    case GM_GUESS: {
      if (!strcmp(a, "new")) {
        gGame.secret = 1 + esp_random() % 100;
        gGame.tries = gGame.guess = 0;
        gGame.hint = 0;
        gameReact(M_HAPPY, 1500, S_GM_PICKED, 0);
      }
      int n = d["n"] | 0;
      if (n >= 1 && n <= 100 && gGame.hint != 2) {
        gGame.guess = n;
        gGame.tries++;
        gGame.hint = n < gGame.secret ? 1 : n > gGame.secret ? -1 : 2;
        if (gGame.hint == 2) { gameReact(M_LOVE, 4000, S_GM_GOT_IT, gGame.tries); PLAY(SND_DONE); }
        else gameReact(abs(n - gGame.secret) <= 5 ? M_SURPRISED : M_NORMAL, 1200,
                       gGame.hint > 0 ? S_GM_HIGHER : S_GM_LOWER, 0);
      }
      r["hint"] = gGame.hint;
      r["tries"] = gGame.tries;
    } break;
    case GM_SIMON: {
      if (!strcmp(a, "start")) { gGame.len = 0; simonNext(now); }
      int dir = d["d"] | -1;
      if (!strcmp(a, "tap") && dir >= 0 && dir <= 3 && gGame.len && !gGame.showing) {
        gGame.pressed = dir;
        gGame.pressedAt = now;
        if (dir == gGame.seq[gGame.pos]) {
          playArrow(dir);
          if (++gGame.pos == gGame.len) {   // the whole row: one more arrow
            if (gGame.len > gGame.best) gGame.best = gGame.len;
            gameReact(M_HAPPY, 900, S_GM_WATCH, 0);
            simonNext(now + 600);
          }
        } else {
          if (gGame.len - 1 > gGame.best) gGame.best = gGame.len - 1;
          r["fail"] = true;
          r["reached"] = gGame.len;
          gGame.len = 0;
          gameReact(M_SAD, 3000, S_GM_OOPS, gGame.best);
          PLAY(SND_SAD);
        }
      }
      r["len"] = gGame.len;
      r["best"] = gGame.best;
      r["ms"] = gGame.showing ? (int32_t)(gGame.at + simonShowMs() - now) : 0;
    } break;
    case GM_ROLL: {
      if (!strcmp(a, "start") && gMotion) {
        gGame.bx = 64; gGame.by = 40; gGame.vx = gGame.vy = 0;
        gGame.cx = 16; gGame.cy = 30; gGame.cvx = 16; gGame.cvy = 9;
        gGame.stars = 0;
        gGame.newBest = false;
        gGame.rolling = true;
        gGame.startAt = now + 1500;         // "tilt me!" first
        gGame.endAt = gGame.startAt + ROLL_MS;
        gGame.tickAt = now;
        gGame.hurtAt = 0;
        rollPlaceStar();
        gameReact(M_HAPPY, 1500, S_GM_TILT_ME, 0);
        PLAY(SND_WAKE);
      }
      r["mpu"] = gMotion;
      r["run"] = gGame.rolling;
      r["wait"] = gGame.rolling && (int32_t)(now - gGame.startAt) < 0;
      r["left"] = gGame.rolling && (int32_t)(gGame.endAt - now) > 0 ? (int32_t)min<uint32_t>(ROLL_MS, gGame.endAt - now) : 0;
      r["played"] = !gGame.rolling && gGame.endAt != 0;
      r["stars"] = gGame.stars;
      r["best"] = gGame.bestStars;
      r["newBest"] = gGame.newBest;
    } break;
    case GM_RIDDLE: {
      const bool aiOk = AI_API_KEY[0] && gWifiUp && !(gSetupMode && WiFi.softAPgetStationNum() > 0);
      if (!strcmp(a, "new") && gGame.rphase != RD_THINKING && gGame.rphase != RD_JUDGING) {
        if (!aiOk) {
          gGame.rphase = RD_FAILED;
          gameReact(M_SAD, 3000, S_GM_NO_AI, 0);
        } else {
          gGame.rphase = RD_THINKING;
          gGame.rtries = 0;
          gGame.rhint = false;
          gRiddleReq = 1;
          gameReact(M_NORMAL, 0, S_GM_HMM, 0);
        }
      } else if (!strcmp(a, "guess") && gGame.rphase == RD_ASKING) {
        const char* gs = d["t"] | "";
        if (gs[0]) {
          gGame.rtries++;
          utf8Copy(gGame.rguess, gs, sizeof(gGame.rguess));
          if (riddleMatch(gs)) riddleSolved();
          else if (aiOk) { gGame.rphase = RD_JUDGING; gRiddleReq = 2; gameReact(M_NORMAL, 0, S_GM_HMM, 0); }
          else riddleWrong();
        }
      } else if (!strcmp(a, "hint") && gGame.rphase == RD_ASKING) {
        gGame.rhint = true;
        gGame.rAt = now;                    // the text starts again from the top
      } else if (!strcmp(a, "reveal") && gGame.rphase == RD_ASKING) {
        gGame.rphase = RD_REVEALED;
        gameReact(M_SURPRISED, 2000, S_GM_IT_WAS, 0);
      }
      r["phase"] = gGame.rphase;
      r["tries"] = gGame.rtries;
      xSemaphoreTake(gMutex, portMAX_DELAY);
      if (gGame.rphase >= RD_ASKING && gGame.rphase <= RD_REVEALED) r["q"] = gGame.riddle;
      if (gGame.rhint) r["hint"] = gGame.hintText;
      if (gGame.rphase == RD_SOLVED || gGame.rphase == RD_REVEALED) r["a"] = gGame.answer;
      xSemaphoreGive(gMutex);
    } break;
    case GM_QUEST: {
      if (!strcmp(a, "start") && gGame.qphase != QS_THINKING && gGame.qphase != QS_READY && gGame.qphase != QS_STEP) {
        const bool aiOk = AI_API_KEY[0] && gWifiUp && !(gSetupMode && WiFi.softAPgetStationNum() > 0);
        if (aiOk) {
          gGame.qphase = QS_THINKING;
          gQuestReq = true;
          gameReact(M_NORMAL, 0, S_GM_HMM, 0);
        } else {
          questOffline();
          questReady(now);
        }
      }
      const uint8_t ph = gGame.qphase;
      r["mpu"] = gMotion;
      r["phase"] = ph;
      r["story"] = gGame.qstory;
      r["step"] = gGame.qstep + 1;
      r["n"] = gGame.qn;
      r["hearts"] = gGame.qhearts;
      r["score"] = gGame.qscore;
      r["best"] = gGame.qbest;
      r["newBest"] = gGame.qnewBest;
      r["won"] = gGame.qwon;
      xSemaphoreTake(gMutex, portMAX_DELAY);
      if (ph >= QS_READY) r["title"] = gGame.qtitle;
      if (ph == QS_STEP) r["t"] = gGame.qtext[gGame.qstep];
      if (ph == QS_DONE) r["end"] = gGame.qend[gGame.qwon ? 0 : 1];
      xSemaphoreGive(gMutex);
    } break;
    case GM_TAP: {
      if (!strcmp(a, "start") && (gGame.tphase == TP_IDLE || gGame.tphase == TP_DONE)) tapStart(now);
      const uint8_t ph = gGame.tphase;
      r["phase"] = ph;
      r["round"] = min<int>(gGame.tround + (ph == TP_SHOW ? 0 : 1), TAP_ROUNDS);
      r["n"] = TAP_ROUNDS;
      if (ph == TP_SHOW) r["last"] = gGame.tlast;
      r["avg"] = gGame.tavg;
      r["best"] = gGame.tbest;
      r["newBest"] = gGame.tnewBest;
    } break;
  }
  r["ok"] = true;
  String out;
  serializeJson(r, out);
  server.send(200, "application/json", out);
}

// "Reset today's count" on the phone page: the AI may be used again today (no restart needed)
static void handleAiReset() {
  gAiUsed = 0;
  prefs.putUShort("aiused", 0);
  if (!strcmp(gAiStatus, T(S_AI_LIMIT))) gAiStatus[0] = 0;
  server.send(200, "application/json", "{\"ok\":true,\"aiUsed\":0}");
}

static void handleGames() { server.send_P(200, "text/html; charset=utf-8", GAMES_HTML); }

// Sounds and timing that run between the phone's moves (from loop)
static void gameTick(uint32_t now) {
  if (gGame.game == GM_NONE) return;
  if (!gameActive(now)) { gGame.game = GM_NONE; gPageEnter = now; return; }
  if (gGame.game == GM_RPS && !gGame.sounded && now - gGame.at >= 1200) {   // the reveal
    gGame.sounded = true;
    if (gGame.result == 1) { gameReact(M_SURPRISED, 2500, S_GM_YOU_WIN, 0); PLAY(SND_PAT); }
    else if (gGame.result == 2) { gameReact(M_HAPPY, 2500, S_GM_I_WIN, 0); PLAY(SND_ON); }
    else gameReact(M_NORMAL, 0, S_GM_DRAW, 0);
  }
  if (gGame.game == GM_ROLL && gGame.rolling) rollTick(now);
  if (gGame.game == GM_QUEST) questTick(now);
  if (gGame.game == GM_TAP) tapTick(now);
  if (gGame.game == GM_SIMON && gGame.showing && (int32_t)(now - gGame.at) >= 900) {
    uint32_t k = (now - gGame.at - 900) / simonStep();
    if (k >= gGame.len) {
      gGame.showing = false;
      gGame.shown = 255;
      gameReact(M_NORMAL, 0, S_GM_YOUR_TURN, 0);
    } else if (k != gGame.shown) {
      gGame.shown = k;
      playArrow(gGame.seq[k]);
    }
  }
}

// A little twinkling star and a rain cloud, for star roll
static void drawStar(int x, int y, bool big) {
  display.drawFastHLine(x - 3, y, 7, SH110X_WHITE);
  display.drawFastVLine(x, y - 3, 7, SH110X_WHITE);
  if (big) {
    display.drawPixel(x - 2, y - 2, SH110X_WHITE); display.drawPixel(x + 2, y - 2, SH110X_WHITE);
    display.drawPixel(x - 2, y + 2, SH110X_WHITE); display.drawPixel(x + 2, y + 2, SH110X_WHITE);
  }
  display.fillRect(x - 1, y - 1, 3, 3, SH110X_WHITE);
}

static void drawRainCloud(int x, int y) {
  display.fillCircle(x - 3, y, 3, SH110X_WHITE);
  display.fillCircle(x + 1, y - 2, 4, SH110X_WHITE);
  display.fillCircle(x + 5, y, 3, SH110X_WHITE);
  display.fillRect(x - 3, y, 9, 4, SH110X_WHITE);
  display.drawPixel(x - 2, y + 6, SH110X_WHITE);   // rain
  display.drawPixel(x + 2, y + 7, SH110X_WHITE);
  display.drawPixel(x + 6, y + 6, SH110X_WHITE);
}

// Momowo's face in miniature, for the game views
static void drawMiniFace(int cx, int cy, Mood m, uint32_t now) {
  const int w = 10, h = 12;
  for (int s = -1; s <= 1; s += 2) {
    int x = cx + s * 11;
    switch (m) {
      case M_HAPPY: case M_LOVE:            // smiling eyes
        display.fillCircle(x, cy + 2, 5, SH110X_WHITE);
        display.fillCircle(x, cy + 5, 5, SH110X_BLACK);
        break;
      case M_SURPRISED:
        display.drawCircle(x, cy, 6, SH110X_WHITE);
        display.fillCircle(x, cy, 2, SH110X_WHITE);
        break;
      case M_SAD: {                         // drooping at the outer corners
        display.fillRoundRect(x - w / 2, cy - h / 2 + 2, w, h - 2, 3, SH110X_WHITE);
        int xo = x + s * (w / 2 + 1), xi = x - s * (w / 2 + 1);
        display.fillTriangle(xo, cy - h / 2, xi, cy - h / 2, xo, cy + 1, SH110X_BLACK);
      } break;
      default:
        if ((now / 100) % 40 == 0) display.drawFastHLine(x - w / 2, cy, w, SH110X_WHITE);   // blink
        else display.fillRoundRect(x - w / 2, cy - h / 2, w, h, 3, SH110X_WHITE);
    }
  }
  if (m == M_HAPPY || m == M_LOVE) {
    display.fillCircle(cx, cy + 9, 4, SH110X_WHITE);
    display.fillCircle(cx, cy + 7, 4, SH110X_BLACK);
  } else if (m == M_SAD) {
    display.fillCircle(cx, cy + 14, 4, SH110X_WHITE);
    display.fillCircle(cx, cy + 16, 4, SH110X_BLACK);
  } else if (m == M_SURPRISED) {
    display.drawCircle(cx, cy + 11, 2, SH110X_WHITE);
  } else {
    display.drawFastHLine(cx - 3, cy + 11, 7, SH110X_WHITE);
  }
  if (m == M_LOVE && (now / 400) % 2) {     // a little heart
    display.fillCircle(cx + 22, cy - 8, 2, SH110X_WHITE);
    display.fillCircle(cx + 26, cy - 8, 2, SH110X_WHITE);
    display.fillTriangle(cx + 20, cy - 7, cx + 28, cy - 7, cx + 24, cy - 2, SH110X_WHITE);
  }
}

// Rock (a fist), paper (a flat hand) and scissors, about 20 px tall
static void drawHand(int cx, int cy, int8_t h) {
  if (h == 0) {
    display.fillCircle(cx, cy, 9, SH110X_WHITE);
    for (int i = -1; i <= 1; i++) display.drawFastVLine(cx + i * 4, cy - 9, 5, SH110X_BLACK);
  } else if (h == 1) {
    display.fillRoundRect(cx - 9, cy - 10, 18, 20, 4, SH110X_WHITE);
    for (int i = -1; i <= 1; i++) display.drawFastVLine(cx + i * 4 + (i > 0 ? 1 : 0), cy - 10, 8, SH110X_BLACK);
  } else {
    display.drawCircle(cx - 5, cy + 6, 4, SH110X_WHITE);
    display.drawCircle(cx + 5, cy + 6, 4, SH110X_WHITE);
    for (int t = 0; t < 2; t++) {
      display.drawLine(cx - 3 + t, cy + 3, cx + 7 + t, cy - 11, SH110X_WHITE);
      display.drawLine(cx + 3 + t, cy + 3, cx - 7 + t, cy - 11, SH110X_WHITE);
    }
  }
}

static void drawArrow(int cx, int cy, uint8_t d, bool lit) {   // 0 up, 1 right, 2 down, 3 left
  const int r = 8;
  int x0, y0, x1, y1, x2, y2;
  switch (d) {
    case 0: x0 = cx; y0 = cy - r; x1 = cx - r; y1 = cy + r / 2; x2 = cx + r; y2 = cy + r / 2; break;
    case 1: x0 = cx + r; y0 = cy; x1 = cx - r / 2; y1 = cy - r; x2 = cx - r / 2; y2 = cy + r; break;
    case 2: x0 = cx; y0 = cy + r; x1 = cx - r; y1 = cy - r / 2; x2 = cx + r; y2 = cy - r / 2; break;
    default: x0 = cx - r; y0 = cy; x1 = cx + r / 2; y1 = cy - r; x2 = cx + r / 2; y2 = cy + r; break;
  }
  if (lit) display.fillTriangle(x0, y0, x1, y1, x2, y2, SH110X_WHITE);
  else display.drawTriangle(x0, y0, x1, y1, x2, y2, SH110X_WHITE);
}

static void drawGame(uint32_t now) {
  const Mood m = notYet(now, gGame.moodUntil) ? gGame.mood : M_NORMAL;
  char say[64], right[16];
  snprintf(say, sizeof(say), T(gGame.say), gGame.sayArg);
  const int top = bodyTop();
  switch (gGame.game) {
    case GM_RPS: {
      snprintf(right, sizeof(right), "%u:%u", gGame.wins, gGame.losses);
      const int cy = langIsCjk() ? 35 : 31;
      if (gGame.you < 0) {
        drawMiniFace(64, cy, m, now);
      } else if (now - gGame.at < 1200) {   // "rock, paper, scissors!": both fists shake
        int bob = ((now - gGame.at) / 200) % 2 ? -3 : 3;
        drawHand(30, cy + bob, 0);
        drawHand(98, cy + bob, 0);
      } else {
        drawHand(30, cy, gGame.you);
        drawHand(98, cy, gGame.me);
        drawMiniFace(64, cy - 2, m, now);
      }
      if (!langIsCjk()) {                   // the score under the title: "You 2", "3 Me"
        char lab[24];
        snprintf(lab, sizeof(lab), "%s %u", T(S_GM_YOU), gGame.wins);
        textLine(lab, 12, 0, 0);
        snprintf(lab, sizeof(lab), "%u %s", gGame.losses, T(S_GM_ME));
        textLine(lab, 12, 2, 0);
      }
      textLine(say, 55, 1, now - gGame.last);
      pageHeader(T(S_GM_RPS), langIsCjk() ? right : nullptr);   // long Latin titles fill the header
    } break;
    case GM_TTT: {
      textAt(T(S_GM_TTT), 0, langIsCjk() ? 2 : 0);
      drawMiniFace(34, 28, m, now);
      textLine(say, 52, 0, 0);
      const int x0 = 76, y0 = 11, c = 17;
      display.fillRect(x0 - 2, 0, SCREEN_W - x0 + 2, SCREEN_H, SH110X_BLACK);
      for (int i = 1; i < 3; i++) {
        display.drawFastVLine(x0 + i * c, y0, 3 * c, SH110X_WHITE);
        display.drawFastHLine(x0, y0 + i * c, 3 * c, SH110X_WHITE);
      }
      for (int i = 0; i < 9; i++) {
        int x = x0 + (i % 3) * c + c / 2, y = y0 + (i / 3) * c + c / 2;
        if (gGame.board[i] == 1) {
          for (int t = 0; t < 2; t++) {
            display.drawLine(x - 5 + t, y - 5, x + 5 + t, y + 5, SH110X_WHITE);
            display.drawLine(x + 5 + t, y - 5, x - 5 + t, y + 5, SH110X_WHITE);
          }
        } else if (gGame.board[i] == 2 && !(i == gGame.lastMove && now - gGame.at < 900 && (now / 150) % 2)) {
          display.drawCircle(x, y, 6, SH110X_WHITE);
          display.drawCircle(x, y, 5, SH110X_WHITE);
        }
      }
    } break;
    case GM_GUESS: {
      snprintf(right, sizeof(right), "%u", gGame.tries);
      drawMiniFace(30, top + 16, m, now);
      char num[8] = "?";
      if (gGame.guess) snprintf(num, sizeof(num), "%u", gGame.guess);
      display.setTextSize(2);
      display.setCursor(gGame.guess ? 70 : 80, top + 6);
      display.print(num);
      display.setTextSize(1);
      if (gGame.hint == 1 || gGame.hint == -1) drawArrow(116, top + 13, gGame.hint > 0 ? 0 : 2, true);
      textLine(say, 55, 1, now - gGame.last);
      pageHeader(T(S_GM_GUESS), gGame.tries ? right : nullptr);
    } break;
    case GM_SIMON: {
      snprintf(right, sizeof(right), "%u", gGame.len);
      const int cx = 98, cy = langIsCjk() ? 40 : 37;
      uint8_t lit = 255;
      if (gGame.showing && gGame.shown < gGame.len) {
        uint32_t k0 = gGame.at + 900 + gGame.shown * simonStep();
        if (now - k0 < simonStep() * 3 / 4) lit = gGame.seq[gGame.shown];
      } else if (now - gGame.pressedAt < 250) {
        lit = gGame.pressed;
      }
      static const int8_t AX[] = {0, 1, 0, -1}, AY[] = {-1, 0, 1, 0};
      for (uint8_t d = 0; d < 4; d++) drawArrow(cx + AX[d] * 17, cy + AY[d] * 14, d, d == lit);
      drawMiniFace(32, cy - 6, m, now);
      if (!gGame.len && gGame.say != S_GM_OOPS) say[0] = 0;
      textAt(say, 0, 54);
      pageHeader(T(S_GM_SIMON), gGame.len ? right : nullptr);
    } break;
    case GM_ROLL: {
      const int rt = rollTop();
      if (gGame.rolling && (int32_t)(now - gGame.startAt) >= 0) {
        uint32_t left = (gGame.endAt - now + 999) / 1000;
        snprintf(right, sizeof(right), "%u  %lus", gGame.stars, (unsigned long)left);
        display.drawRect(0, rt, SCREEN_W, SCREEN_H - rt, SH110X_WHITE);
        drawStar(gGame.sx, gGame.sy, (now / 250) % 2);
        if (gGame.stars >= 3 && !(now - gGame.hurtAt < 1500 && (now / 120) % 2)) drawRainCloud(gGame.cx, gGame.cy);
        display.fillCircle(gGame.bx, gGame.by, 3, SH110X_WHITE);
        if (now - gGame.hurtAt < 600) textLine(":(", rt + 3, 1, 0);
      } else {                              // waiting, "tilt me!", or the result
        snprintf(right, sizeof(right), "%u", gGame.bestStars);
        drawMiniFace(64, top + 13, m, now);
        if (!gGame.rolling && !gGame.endAt) snprintf(say, sizeof(say), "%s", T(S_GM_PHONE));
        else if (gGame.rolling) snprintf(say, sizeof(say), "%s", T(S_GM_TILT_ME));
        else snprintf(say, sizeof(say), T(gGame.newBest ? S_GM_NEW_BEST : S_GM_TIME_UP), gGame.stars);
        textLine(say, 55, 1, now - gGame.last);
      }
      pageHeader(T(S_GM_ROLL), gGame.rolling || gGame.bestStars ? right : nullptr);
    } break;
    case GM_RIDDLE: {
      snprintf(right, sizeof(right), "%u", gGame.rtries);
      const uint8_t ph = gGame.rphase;
      if (ph == RD_ASKING) {
        static char text[400];
        xSemaphoreTake(gMutex, portMAX_DELAY);
        if (gGame.rhint && gGame.hintText[0])
          snprintf(text, sizeof(text), "%s  %s %s", gGame.riddle, T(S_GM_HINT), gGame.hintText);
        else
          strlcpy(text, gGame.riddle, sizeof(text));
        xSemaphoreGive(gMutex);
        drawWrapped(text, top, now - gGame.rAt);
        if (notYet(now, gGame.moodUntil) && gGame.say == S_GM_NOT_QUITE) {   // a wrong guess, for a moment
          const int h = langIsCjk() ? 16 : 11;
          display.fillRect(0, SCREEN_H - h, SCREEN_W, h, SH110X_BLACK);
          display.drawFastHLine(0, SCREEN_H - h, SCREEN_W, SH110X_WHITE);
          textLine(say, SCREEN_H - 9, 1, 0);
        }
      } else if (ph == RD_SOLVED || ph == RD_REVEALED) {
        char ans[48];
        xSemaphoreTake(gMutex, portMAX_DELAY);
        strlcpy(ans, gGame.answer, sizeof(ans));
        xSemaphoreGive(gMutex);
        drawMiniFace(22, top + 16, ph == RD_SOLVED ? M_LOVE : m, now);
        textAt(T(S_GM_IT_WAS), 44, top + 2);
        textAt(ans, 44, top + (langIsCjk() ? 18 : 14));
        if (ph == RD_SOLVED) snprintf(say, sizeof(say), T(S_GM_GOT_IT), gGame.rtries);
        else say[0] = 0;
        textLine(say, 55, 1, now - gGame.last);
      } else {                              // waiting, thinking of one, judging, or no AI
        drawMiniFace(64, top + 13, ph == RD_THINKING || ph == RD_JUDGING ? M_NORMAL : m, now);
        if (ph == RD_NONE) snprintf(say, sizeof(say), "%s", T(S_GM_PHONE));
        else if (ph == RD_FAILED) snprintf(say, sizeof(say), "%s", T(S_GM_NO_AI));
        else snprintf(say, sizeof(say), "%s", T(S_GM_HMM));
        textLine(say, 55, 1, now - gGame.last);
      }
      pageHeader(T(S_GM_RIDDLE), gGame.rtries ? right : nullptr);
    } break;
    case GM_QUEST: drawQuest(now, m, say); break;
    case GM_TAP: {
      const uint8_t ph = gGame.tphase;
      if (ph == TP_GO) {                    // the signal: the whole screen flashes
        display.fillRect(0, 0, SCREEN_W, SCREEN_H, SH110X_WHITE);
        const char* go = T(S_GM_TAP_NOW);
        display.setTextColor(SH110X_BLACK);
        u8f.setForegroundColor(SH110X_BLACK);
        if (hasCjk(go)) textLine(go, 26, 1, 0);
        else {
          char big[24];
          toCp437(go, big, sizeof(big));
          int sz = strlen(big) * 18 <= SCREEN_W ? 3 : 2;
          display.setTextSize(sz);
          display.setCursor((SCREEN_W - (int)strlen(big) * 6 * sz) / 2, 32 - sz * 4);
          display.print(big);
          display.setTextSize(1);
        }
        display.setTextColor(SH110X_WHITE);
        u8f.setForegroundColor(SH110X_WHITE);
        break;
      }
      const int cy = top + 13;
      const bool fake = ph == TP_WAIT && gGame.tfakeAt && !notYet(now, gGame.tfakeAt) && notYet(now, gGame.tfakeAt + 450);
      drawMiniFace(64, cy, fake ? M_SURPRISED : ph == TP_WAIT ? M_NORMAL : m, now);
      if (ph == TP_SHOW && gGame.tlast >= 0) {
        char ms[16];
        snprintf(ms, sizeof(ms), "%d ms", gGame.tlast);
        display.setTextSize(2);
        display.setCursor((SCREEN_W - (int)strlen(ms) * 12) / 2, SCREEN_H - 15);   // under the smile
        display.print(ms);
        display.setTextSize(1);
      } else {
        if (ph == TP_IDLE) snprintf(say, sizeof(say), "%s", T(S_GM_TAP_HOW));
        else if (ph == TP_WAIT) snprintf(say, sizeof(say), "%s", T(S_GM_WAIT));
        textLine(say, 55, 1, now - gGame.last);
      }
      if (ph == TP_WAIT || ph == TP_SHOW)
        snprintf(right, sizeof(right), "%u/%u", min<unsigned>(gGame.tround + (ph == TP_SHOW ? 0 : 1), TAP_ROUNDS), TAP_ROUNDS);
      else snprintf(right, sizeof(right), "%ums", gGame.tbest);
      pageHeader(T(S_GM_TAP), ph == TP_WAIT || ph == TP_SHOW || gGame.tbest ? right : nullptr);
    } break;
  }
}

// Quest pictures: what to do now (r is about half the size)
static void drawQuestAct(int cx, int cy, uint8_t act, int r) {
  static const uint8_t DIR[] = {3, 1, 0, 2};   // left, right, up, down as arrows
  switch (act) {
    case QA_TAP:                            // a dot with one ring: touch my head
      display.fillCircle(cx, cy, r / 3, SH110X_WHITE);
      display.drawCircle(cx, cy, r, SH110X_WHITE);
      break;
    case QA_HOLD:                           // a big dot with two rings: keep touching
      display.fillCircle(cx, cy, r / 2, SH110X_WHITE);
      display.drawCircle(cx, cy, r - 2, SH110X_WHITE);
      display.drawCircle(cx, cy, r, SH110X_WHITE);
      break;
    case QA_SHAKE: {                        // Momowo with motion lines
      const int w = r, h = r + r / 2;
      display.drawRoundRect(cx - w / 2, cy - h / 2, w, h, 2, SH110X_WHITE);
      for (int s = -1; s <= 1; s += 2)
        for (int k = 0; k < 2; k++)
          display.drawFastVLine(cx + s * (w / 2 + 3 + k * 3), cy - h / 2 + 2 + k, h - 4 - 2 * k, SH110X_WHITE);
    } break;
    default: {                              // tilt: a filled arrow
      const uint8_t d = DIR[act - QA_LEFT];
      int x0, y0, x1, y1, x2, y2;
      switch (d) {
        case 0: x0 = cx; y0 = cy - r; x1 = cx - r; y1 = cy + r / 2; x2 = cx + r; y2 = cy + r / 2; break;
        case 1: x0 = cx + r; y0 = cy; x1 = cx - r / 2; y1 = cy - r; x2 = cx - r / 2; y2 = cy + r; break;
        case 2: x0 = cx; y0 = cy + r; x1 = cx - r; y1 = cy - r / 2; x2 = cx + r; y2 = cy - r / 2; break;
        default: x0 = cx - r; y0 = cy; x1 = cx + r / 2; y1 = cy - r; x2 = cx + r / 2; y2 = cy + r; break;
      }
      display.fillTriangle(x0, y0, x1, y1, x2, y2, SH110X_WHITE);
    } break;
  }
}

static void drawQuest(uint32_t now, Mood m, const char* say) {
  const int top = bodyTop();
  char right[16];
  const uint8_t ph = gGame.qphase;
  snprintf(right, sizeof(right), "%u", ph == QS_STEP || ph == QS_DONE ? gGame.qscore : gGame.qbest);
  if (ph == QS_STEP) {
    static char text[100];
    xSemaphoreTake(gMutex, portMAX_DELAY);
    strlcpy(text, gGame.qtext[gGame.qstep], sizeof(text));
    xSemaphoreGive(gMutex);
    const bool story = text[0] != 0;
    // the story above; below it the time bar, then the step, the action and the hearts
    const int bar = story ? (langIsCjk() ? 49 : 47) : top;
    if (story) {
      gWrapBottom = bar;
      drawWrapped(text, top, now - gGame.qstepAt);
      gWrapBottom = SCREEN_H;
    }
    const int cy = story ? (langIsCjk() ? 57 : 56) : (top + SCREEN_H) / 2;
    if (story) display.fillRect(0, bar, SCREEN_W, SCREEN_H - bar, SH110X_BLACK);
    if (gGame.qwaitUntil) {                 // a tick or a cross
      if (story) display.drawFastHLine(0, bar, SCREEN_W, SH110X_WHITE);
      const int r = story ? 5 : 10;
      for (int k = 0; k < 2; k++) {
        if (gGame.qok) {
          display.drawLine(64 - r, cy + k, 64 - r / 3, cy + r * 2 / 3 + k, SH110X_WHITE);
          display.drawLine(64 - r / 3, cy + r * 2 / 3 + k, 64 + r, cy - r * 2 / 3 + k, SH110X_WHITE);
        } else {
          display.drawLine(64 - r + k, cy - r, 64 + r + k, cy + r, SH110X_WHITE);
          display.drawLine(64 + r + k, cy - r, 64 - r + k, cy + r, SH110X_WHITE);
        }
      }
    } else {
      drawQuestAct(64, cy, gGame.qact[gGame.qstep], story ? (langIsCjk() ? 5 : 6) : 12);
      const uint32_t el = now - gGame.qstepAt;   // the time bar shrinks
      const int w = el >= gGame.qwin ? 0 : (int)((uint64_t)SCREEN_W * (gGame.qwin - el) / gGame.qwin);
      display.fillRect(0, bar, w, 2, SH110X_WHITE);
    }
    char st[8];
    snprintf(st, sizeof(st), "%u/%u", gGame.qstep + 1, gGame.qn);
    display.setTextSize(1);
    display.setCursor(0, SCREEN_H - 9);
    display.print(st);
    for (uint8_t h = 0; h < QUEST_HEARTS; h++) {
      const int hx = SCREEN_W - 5 - h * 9, hy = SCREEN_H - 6;
      if (h < gGame.qhearts) drawHeart(hx, hy, 2);
      else display.drawPixel(hx, hy, SH110X_WHITE);
    }
  } else if (ph == QS_DONE) {
    static char text[120];
    xSemaphoreTake(gMutex, portMAX_DELAY);
    strlcpy(text, gGame.qend[gGame.qwon ? 0 : 1], sizeof(text));
    xSemaphoreGive(gMutex);
    if (text[0]) {
      const int h = langIsCjk() ? 16 : 11;
      gWrapBottom = SCREEN_H - h;
      drawWrapped(text, top, now - gGame.at);
      gWrapBottom = SCREEN_H;
      display.fillRect(0, SCREEN_H - h, SCREEN_W, h, SH110X_BLACK);
      display.drawFastHLine(0, SCREEN_H - h, SCREEN_W, SH110X_WHITE);
      textLine(say, SCREEN_H - 9, 1, now - gGame.at);
    } else {
      drawMiniFace(64, top + 13, m, now);
      textLine(say, 55, 1, now - gGame.at);
    }
  } else {                                  // waiting, writing the story, or "get ready"
    char line[80];
    if (ph == QS_READY) {
      xSemaphoreTake(gMutex, portMAX_DELAY);
      strlcpy(line, gGame.qtitle[0] ? gGame.qtitle : say, sizeof(line));
      xSemaphoreGive(gMutex);
    } else {
      strlcpy(line, ph == QS_THINKING ? T(S_GM_HMM) : T(S_GM_PHONE), sizeof(line));
    }
    drawMiniFace(64, top + 13, ph == QS_THINKING ? M_NORMAL : m, now);
    textLine(line, 55, 1, ph == QS_READY && gGame.qreadyAt ? now - gGame.qreadyAt : now - gGame.last);
  }
  pageHeader(T(S_GM_QUEST), ph != QS_NONE && ph != QS_THINKING ? right : (gGame.qbest ? right : nullptr));
}

// ============================ RENDER =================================
static void render(uint32_t now, bool sleeping) {
  display.clearDisplay();
  if (gameActive(now)) drawGame(now);
  else if (gSetupMode) drawSetupScreen(now);
  else switch (gPage) {
    case P_BUDDY: {
      Mood m = currentMood(now, sleeping);
      if (fState != F_IDLE && fTotal)       // focus (or break) progress strip
        display.fillRect(0, 0, (uint32_t)((uint64_t)SCREEN_W * (fTotal - fRemain) / fTotal), 2, SH110X_WHITE);
      drawBuddy(now, m);
      drawBuddyTicker(now);
    } break;
    case P_CLOCK:   drawClockPage(now);   break;
    case P_CAL:     drawCalendarPage(now); break;
    case P_WEATHER: drawWeatherPage(now); break;
    case P_NEWS:    drawNewsPage(now);    break;
    case P_QUOTE:   drawQuotePage(now);   break;
    case P_FOCUS:   drawFocusPage(now);   break;
    default: break;
  }
  if (notYet(now, gToastUntil)) drawToast();

  bool inv = notYet(now, gFlashUntil) && ((gFlashUntil - now) / 200) % 2;
  if (inv != gInverted) { gInverted = inv; display.invertDisplay(inv); }
  display.display();
}

// ============================ SPLASH =================================
// The name, pre-rendered from two cute open-source fonts: only these pixels are stored
// "摸摸我" in ZCOOL KuaiLe (SIL Open Font License): 101x33, 1 bit per pixel, rows MSB-first
static const uint8_t LOGO_ZH_W = 101, LOGO_ZH_H = 33;
static const uint8_t LOGO_ZH[] PROGMEM = {
  0x00, 0x01, 0xc0, 0x60, 0x00, 0x00, 0x38, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0,
  0xf0, 0x00, 0x00, 0x78, 0x1e, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x1e, 0x1f, 0xff, 0xfe, 0x03, 0xc3,
  0xff, 0xff, 0xc0, 0x03, 0xe0, 0x20, 0x00, 0x1e, 0x3f, 0xff, 0xfe, 0x03, 0xc7, 0xff, 0xff, 0xc0,
  0x0f, 0xe0, 0xf0, 0x40, 0x1e, 0x3f, 0xff, 0xfe, 0x03, 0xc7, 0xff, 0xff, 0xc0, 0x3f, 0xc0, 0xf1,
  0xe0, 0x1e, 0x1f, 0xff, 0xf0, 0x03, 0xc3, 0xff, 0xfe, 0x00, 0xff, 0x00, 0xf1, 0xe0, 0x0e, 0x01,
  0xe1, 0xc0, 0x01, 0xc0, 0x3c, 0x38, 0x03, 0xfe, 0x00, 0x70, 0xf0, 0x0e, 0x00, 0x00, 0x40, 0x01,
  0xc0, 0x00, 0x08, 0x03, 0xfe, 0x00, 0x78, 0xf0, 0x1f, 0x80, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00,
  0x01, 0xcf, 0x00, 0x78, 0x78, 0xff, 0xc7, 0xff, 0xf8, 0x1f, 0xf8, 0xff, 0xff, 0x00, 0x0f, 0x00,
  0x3f, 0xf8, 0xff, 0xcf, 0xff, 0xfc, 0x1f, 0xf9, 0xff, 0xff, 0x80, 0x0f, 0x03, 0xff, 0xf8, 0xff,
  0xcf, 0xff, 0xfc, 0x1f, 0xf9, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xf0, 0x0e, 0x0f, 0x00, 0x3c,
  0x01, 0xc1, 0xe0, 0x07, 0x83, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0x1f, 0xff, 0x3c, 0x01, 0xe3, 0xff,
  0xe7, 0x83, 0xff, 0xff, 0xfc, 0x00, 0x0f, 0x1f, 0xff, 0x38, 0x01, 0xe3, 0xff, 0xe7, 0x03, 0xff,
  0xe0, 0x1e, 0x00, 0x0f, 0x1f, 0xff, 0x38, 0x01, 0xe3, 0xff, 0xe7, 0x03, 0xef, 0x00, 0x1e, 0x00,
  0x0f, 0x9f, 0x80, 0x38, 0x01, 0xf3, 0xf0, 0x07, 0x00, 0x07, 0x00, 0x1e, 0x00, 0x0f, 0x87, 0x00,
  0xf8, 0x01, 0xf0, 0xe0, 0x1f, 0x00, 0x07, 0x10, 0x0e, 0x00, 0x3f, 0xc7, 0xff, 0xf8, 0x07, 0xf8,
  0xff, 0xff, 0x00, 0x07, 0xf8, 0x0f, 0xe0, 0xff, 0x87, 0xff, 0xf8, 0x1f, 0xf0, 0xff, 0xff, 0x00,
  0x07, 0xf8, 0x0f, 0xe0, 0x7f, 0x07, 0xff, 0xf0, 0x0f, 0xe0, 0xff, 0xfe, 0x00, 0x1f, 0xf8, 0x0f,
  0xf0, 0x7f, 0x01, 0xfc, 0x00, 0x0f, 0xe0, 0x3f, 0x80, 0x00, 0x7f, 0xe0, 0x3f, 0xc0, 0x07, 0x00,
  0xf8, 0xfc, 0x00, 0xe0, 0x1f, 0x1f, 0x81, 0xff, 0x80, 0xff, 0x80, 0x07, 0x07, 0xff, 0xfc, 0x00,
  0xe0, 0xff, 0xff, 0x81, 0xff, 0x81, 0xff, 0x80, 0x07, 0x0f, 0xff, 0xfc, 0x00, 0xe1, 0xff, 0xff,
  0x81, 0xe7, 0x80, 0xf3, 0x80, 0x07, 0x0f, 0xff, 0xfc, 0x00, 0xe1, 0xff, 0xff, 0x80, 0x07, 0x80,
  0xc3, 0xc0, 0x0f, 0x0f, 0xe3, 0x80, 0x01, 0xe1, 0xfc, 0x70, 0x00, 0x1f, 0x00, 0x03, 0xc0, 0x3f,
  0x07, 0xc3, 0xe0, 0x07, 0xe0, 0xf8, 0x7c, 0x00, 0x3f, 0x00, 0x03, 0xc0, 0x7e, 0x07, 0x83, 0xf8,
  0x0f, 0xc0, 0xf0, 0x7f, 0x00, 0x7e, 0x00, 0x01, 0xc0, 0x7c, 0x0f, 0x01, 0xfc, 0x0f, 0x81, 0xe0,
  0x3f, 0x80, 0xf8, 0x00, 0x01, 0xe0, 0x70, 0x0f, 0x00, 0x7c, 0x0e, 0x01, 0xe0, 0x0f, 0x80, 0x70,
  0x00, 0x01, 0xe0, 0x20, 0x06, 0x00, 0x1c, 0x04, 0x00, 0xc0, 0x03, 0x80, 0x60, 0x00, 0x00, 0x80,
  0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};
// "Momowo" in Fredoka SemiBold (SIL Open Font License): 79x14, 1 bit per pixel, rows MSB-first
static const uint8_t LOGO_EN_W = 79, LOGO_EN_H = 14;
static const uint8_t LOGO_EN[] PROGMEM = {
  0xe0, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x1e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3e,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7e, 0x1e, 0x1d, 0xc7, 0x03, 0xc7, 0x18,
  0x60, 0xf0, 0xfe, 0xfe, 0x7f, 0x9f, 0xff, 0x8f, 0xf7, 0x1c, 0xe3, 0xfc, 0xef, 0xee, 0x7f, 0x9f,
  0xff, 0xcf, 0xf7, 0xbc, 0xe3, 0xfc, 0xe7, 0xce, 0xf3, 0xde, 0xfb, 0xde, 0x7b, 0xbf, 0xe7, 0x9e,
  0xe7, 0xce, 0xe1, 0xdc, 0x71, 0xdc, 0x3b, 0xff, 0xc7, 0x0e, 0xe3, 0x8e, 0xe1, 0xdc, 0x71, 0xdc,
  0x39, 0xff, 0xc7, 0x0e, 0xe0, 0x0e, 0xf3, 0xdc, 0x71, 0xde, 0x79, 0xf7, 0x87, 0x9e, 0xe0, 0x0e,
  0x7f, 0x9c, 0x71, 0xcf, 0xf1, 0xe7, 0x83, 0xfc, 0xe0, 0x0e, 0x7f, 0x9c, 0x71, 0xcf, 0xf0, 0xe7,
  0x83, 0xfc, 0xe0, 0x0e, 0x1e, 0x1c, 0x71, 0xc3, 0xc0, 0xc3, 0x00, 0xf0
};

// Startup screen: 摸摸我 / Momowo, with two little hearts that beat
static void showSplash(uint32_t ms) {
  const int gap = 5;
  const int zx = (SCREEN_W - LOGO_ZH_W) / 2, ex = (SCREEN_W - LOGO_EN_W) / 2;
  const int zy = (SCREEN_H - (LOGO_ZH_H + gap + LOGO_EN_H)) / 2, ey = zy + LOGO_ZH_H + gap;
  const int hy = ey + LOGO_EN_H / 2 - 1;
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    display.clearDisplay();
    display.drawBitmap(zx, zy, LOGO_ZH, LOGO_ZH_W, LOGO_ZH_H, SH110X_WHITE);
    display.drawBitmap(ex, ey, LOGO_EN, LOGO_EN_W, LOGO_EN_H, SH110X_WHITE);
    int r = ((millis() - t0) / 350) % 2 ? 3 : 2;   // heartbeat
    drawHeart(ex - 11, hy, r);
    drawHeart(ex + LOGO_EN_W + 11, hy, r);
    display.display();
    delay(30);
  }
}

// ============================ SETUP / LOOP ===========================
void setup() {
  Serial.begin(115200);
  for (uint32_t t0 = millis(); !Serial && millis() - t0 < 1500; ) delay(10);   // let USB Serial attach
  pinMode(TOUCH_PIN, INPUT_PULLDOWN);       // TTP223 idles LOW; pulldown keeps GPIO12 safe

  Wire.begin(OLED_SDA, OLED_SCL);
  delay(250);                               // SH1106 needs a moment after power-up
  if (!display.begin(OLED_ADDR, true)) {
    Serial.println("SH1106 not found - check wiring / address (0x3C or 0x3D)");
    for (;;) delay(1000);
  }
  display.setContrast(CONTRAST_DAY);
  display.cp437(true);
  display.setTextWrap(false);
  display.setTextColor(SH110X_WHITE);

  prefs.begin("deskbuddy", false);          // saved settings (storage name kept from DeskBuddy days)
  loadSettings();
  loadMemory();                             // what Momowo knew before it was switched off
  makeNames();
  if (prefs.isKey("tzoff")) setTzOffset(prefs.getInt("tzoff", 0));
  prefs.getString("tzrule", gTzRule, sizeof(gTzRule));
  prefs.getString("ipcc", gCountryCode, sizeof(gCountryCode));   // last country seen, until the lookup answers
  for (int i = 0; i < 3 && !gMotion; i++) {  // MPU6050, if fitted (0x69, or 0x68 without a clock chip)
    if (i) delay(100);                      // a slow module may still be powering up
    gMotion = motion::mpuBegin();
  }
  Serial.printf("[motion] MPU6050 %s\n", gMotion ? "found" : "not fitted");
  if (!gMotion) {                           // show what the I2C bus holds, for 3 s
    motion::mpuDiagnose();
    char found[64], who[24];
    motion::i2cScan(found, sizeof(found), who, sizeof(who));
    display.clearDisplay();
    display.setCursor(0, 0);  display.print("No motion sensor");
    display.setCursor(0, 12); display.print("I2C found:");
    display.setCursor(0, 22); display.print(found);
    display.setCursor(0, 34); display.print("ID ");
    display.print(who);
    display.setCursor(0, 46); display.print("Screen 3C, clock 68");
    display.setCursor(0, 56); display.print("MPU 68/69, ID 68");
    display.display();
    delay(3000);
  }
  // Time: the clock chip if fitted; else, after a restart we caused, the time saved just before
  if (!rtcLoad() && esp_reset_reason() == ESP_RST_SW && prefs.isKey("epoch")) {
    struct timeval tv = { (time_t)prefs.getULong("epoch", 0) + 2, 0 };
    settimeofday(&tv, nullptr);
  }

  buzzerBegin();                            // sound task on core 1, above loop() priority
  xTaskCreatePinnedToCore(soundTask, "snd", 3072, nullptr, 2, nullptr, SND_CORE);

  u8f.begin(display);                       // Chinese renderer shares the display buffer
  u8f.setFontMode(1);
  u8f.setFontDirection(0);
  u8f.setForegroundColor(SH110X_WHITE);
  u8f.setBackgroundColor(SH110X_BLACK);

  randomSeed(esp_random());
  gMutex = xSemaphoreCreateMutex();
  gScanMutex = xSemaphoreCreateMutex();

  WiFi.onEvent(onWiFiEvent);
  WiFi.setHostname(gHost);                  // must come before the radio starts
  WiFi.mode(WIFI_STA);
  webBegin();                               // settings page: setup network or momowo.local
  xTaskCreatePinnedToCore(netTask, "net", 16384, nullptr, 1, nullptr, NET_CORE);

  PLAY(SND_BOOT);                           // hello! (WiFi connects in the background meanwhile)
  showSplash(2600);

  uint32_t now = millis();
  gLastInteraction = gPageEnter = now;
  gNightAwakeUntil = now + 20000;
  B.nextAntic = now + 10000;
  setMood(M_HAPPY, 1500);                   // happy to be switched on
}

void loop() {
  uint32_t now = millis();

  gLoopNow = now;                           // handlers time things on loop()'s clock, not a later one
  server.handleClient();                    // phone settings page
  gameTick(now);                            // mini-games played from the phone
  if (gSetupMode) dnsServer.processNextRequest();
  // HTTP/game handlers can yield while netTask updates its heartbeat. Snapshot the
  // heartbeat BEFORE reading the clock, not against loop()'s earlier timestamp.
  // Signed elapsed time also tolerates the heartbeat's millis() | 1 sentinel
  // being one millisecond ahead, without wrapping into a false 45-minute stall.
  const uint32_t netAlive = gNetAlive;
  const int32_t netIdleMs = (int32_t)(millis() - netAlive);
  if (!gRestartAt && netAlive && netIdleMs > 45L * 60 * 1000) {   // the network task is stuck
    Serial.println("[net] no pass for 45 min: restarting");
    gRestartAt = now;
  }
  if (!gRestartAt && gRestartSoon && (!gDisplayOn || (now - gLastInteraction > 30UL * 60 * 1000 &&
      fState == F_IDLE && !gameActive(now) && !gSetupMode))) {
    Serial.println("[mem] fragmented: restarting while nobody's looking");
    gRestartAt = now;
  }
  if (gRestartAt && (int32_t)(now - gRestartAt) >= 0) {
    if (time(nullptr) > 1700000000) prefs.putULong("epoch", (uint32_t)time(nullptr));   // keep the clock
    ESP.restart();
  }
  // Setup closes by itself after 10 minutes without a phone on it; never opens by itself
  if (gSetupMode && WiFi.softAPgetStationNum() == 0 && now - gSetupAt > 10UL * 60 * 1000) stopSetupMode();

  static uint32_t tzApplied = 0xFFFFFFFF;   // apply timezone changes on this core
  static uint32_t tzCheckAt = 0;             // and every 2 s: summer time by the zone rule
  if (gTzVersion != tzApplied || now - tzCheckAt >= 2000) {
    const bool force = gTzVersion != tzApplied;
    tzApplied = gTzVersion;
    tzCheckAt = now;
    applyTimezone(force);
  }

  static uint32_t feelSeen = 0;             // a new feeling: show it on the face, say it once
  if (gFeel.version != feelSeen) {
    feelSeen = gFeel.version;
    if (!gSetupMode && !gSleeping && xSemaphoreTake(gMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      char said[sizeof(gFeel.thought)];
      strlcpy(said, gFeel.thought, sizeof(said));
      Mood m = gFeel.mood;
      xSemaphoreGive(gMutex);
      if (m != M_NORMAL) setMood(m, 6000);
      announce(said, 15000);
      PLAY(SND_ON);
    }
  }

  static uint32_t lastCal = 0;              // calendars: cheap, refresh once a second
  if (now - lastCal >= 1000) {
    lastCal = now;
    updateCalendar();
    updatePrayers();
    struct tm pt;
    if (nowTm(pt)) prayerReminder(pt);
  }

  struct tm t;
  bool tv = nowTm(t);
  gNight = tv && isNightHour(t.tm_hour);
  static uint32_t lastRtcSave = 0;          // clock chip: keep it matched to internet time
  if (gRtc && gWifiUp && tv && (lastRtcSave == 0 || now - lastRtcSave > 3600000UL)) { lastRtcSave = now; rtcSave(); }
  if (tv) {                                 // a new day: start counting again (but a restart on the
    int dayKey = (t.tm_year + 1900) * 1000 + t.tm_yday;   // same day keeps the saved count)
    if (dayKey != gSessionDay) {
      gSessionDay = dayKey;
      gSessions = 0;
      gPatsToday = 0;
      gShakesToday = gBreathsToday = 0;
      prefs.putInt("fday", dayKey);
      prefs.putInt("fdone", 0);
      prefs.putInt("mp_n", 0);
    }
  }

  Gesture g = readGesture(now);
  if (g != G_NONE) { gLastTouch = now; handleGesture(g, now); }

  static uint32_t lastMotion = 0;           // MPU6050: about 50 readings a second
  static uint32_t lastMpuTry = 0;           // not found at boot: keep looking (a loose wire)
  if (!gMotion && now - lastMpuTry >= 5000) {
    lastMpuTry = now;
    if ((gMotion = motion::mpuBegin())) Serial.println("[motion] MPU6050 found");
  }
  const bool rollOn = gGame.game == GM_ROLL && gameActive(now);
  const bool questOn = gGame.game == GM_QUEST && gameActive(now);
  if (digitalRead(TOUCH_PIN) == HIGH) gLastTouch = now;   // a finger on the pad: its bump is not a pat
  if (gMotion && now - lastMotion >= 20) {  // always read, so the games know which way is down
    lastMotion = now;
    float a[3], gy[3];
    if (motion::mpuRead(a, gy)) {
      motion::Event ev = motion::update(gMs, a, gy, now, MOTION_SENS);
      if (questOn) { if (ev == motion::EV_SHAKE) questInput(QA_SHAKE, now); }
      else if (ev != motion::EV_NONE && MOTION_ON && !gSetupMode && !rollOn) handleMotion(ev, now);
    }
  }

  focusUpdate(now);

  static uint32_t lastSweep = 0;
  if (now - lastSweep >= 1000) { lastSweep = now; sweepTimers(now); }
  bool sleeping = gNight && !notYet(now, gNightAwakeUntil) && fState != F_RUN && fState != F_BREATHE && !gSetupMode;
  const bool idle = !sleeping && !gSetupMode && fState == F_IDLE && !gameActive(now) &&
                    now - gLastInteraction > 20UL * 60 * 1000;
  static uint8_t contrast = CONTRAST_DAY;
  const uint8_t wantContrast = sleeping ? CONTRAST_NIGHT : idle ? CONTRAST_IDLE : CONTRAST_DAY;
  if (wantContrast != contrast) { contrast = wantContrast; display.setContrast(contrast); }
  if (sleeping != gSleeping) {
    gSleeping = sleeping;
    if (!sleeping) {
      setMood(M_SURPRISED, 700);             // waking up
      if (!gNight) PLAY(SND_WAKE);           // the real morning, not a night-time touch
    }
  }

  // Hourly chime (checked once a second)
  static uint32_t lastSoundCheck = 0;
  static int lastChimeHour = -1;
  if (tv && now - lastSoundCheck >= 1000) {
    lastSoundCheck = now;
    if (HOURLY_CHIME && t.tm_min == 0 && t.tm_hour != lastChimeHour && !gNight && fState != F_RUN &&
        t.tm_hour != WAKE_HOUR) {             // wake-up hour has its morning tune instead
      lastChimeHour = t.tm_hour;
      PLAY(SND_HOUR);
    }
  }
  static int airBand = 0;                   // the air got worse (above 100, 150, 200): Momowo coughs
  const int aqi = getWeather().aqi;
  const int band = aqi > 200 ? 3 : aqi > 150 ? 2 : aqi > 100 ? 1 : 0;
  if (band > airBand && aqi >= 0) {
    setMood(M_SAD, 4000);
    announce(T(S_AQI_COUGH), 7000);
    PLAY(SND_COUGH);
  }
  if (aqi >= 0) airBand = band;
  static bool wasOnline = false;            // WiFi lost: a sad little sound
  if (wasOnline && !gWifiUp && !gPlannedHop) PLAY(SND_SAD);
  wasOnline = gWifiUp;
  if (sleeping && gDisplayOn && now - gLastInteraction > NIGHT_OFF_MS) {
    display.oled_command(SH110X_DISPLAYOFF);
    gDisplayOn = false;
  }
  if (!gDisplayOn && !sleeping) {
    display.oled_command(SH110X_DISPLAYON);
    gDisplayOn = true;
  }

  if (gPage != P_BUDDY && now - gLastInteraction > IDLE_HOME_MS && fState != F_BREATHE) {
    gPage = P_BUDDY;
    gPageEnter = now;
  }

  static uint32_t lastFrame = 0;
  if (gDisplayOn && now - lastFrame >= FRAME_MS) {
    lastFrame = now;
    render(now, sleeping);
  }
  delay(2);
}
