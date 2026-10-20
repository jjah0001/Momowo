// Momowo screen labels in eight languages, one row per label:
// English, Simplified Chinese, Traditional Chinese, Indonesian, German, French, Japanese, Spanish.
// When editing: keep the same %d / %s placeholders, in the same order, in every language,
// and keep labels that sit in fixed spaces (setup screen, toasts, focus hints) about as short.
#pragma once

enum Str : uint16_t {
  S_FOCUS_LEFT,
  S_ITS_TIME,
  S_RAIN_ADVICE,
  S_HOT,
  S_CHILLY,
  S_LOVELY,
  S_SUNRISE_AT,
  S_SUNSET_AT,
  S_SUNRISE_TMR,
  S_TOMORROW_FC,
  S_WIFI_REASON,
  S_WIFI_NOT,
  S_WIFI_CONNECTING,
  S_HOLD_FIX_WIFI,
  S_BACKUP_WIFI,
  S_HOLD_SET_CLOCK,
  S_SETTINGS_AT,
  S_OR_HTTP,
  S_TAP_ME,
  S_WR_PASS,
  S_WR_NOTFOUND,
  S_WR_LOST,
  S_WR_CANT,
  S_AM,
  S_PM,
  S_SYNCING,
  S_SET_CLOCK1,
  S_SET_CLOCK2,
  S_SET_CLOCK3,
  S_FETCH_WEATHER,
  S_NO_WIFI,
  S_FEELS,
  S_HI_LO_RH,
  S_RAIN_WIND,
  S_TOMORROW,
  S_RAIN_CHANCE,
  S_WIND_UPTO,
  S_SUN,
  S_TODAY,
  S_DAYLIGHT,
  S_SAME_LENGTH,
  S_MIN_TOMORROW,
  S_C_CLEAR,
  S_C_MAINLY,
  S_C_PARTLY,
  S_C_OVERCAST,
  S_C_FOG,
  S_C_DRIZZLE,
  S_C_FDRIZZLE,
  S_C_LRAIN,
  S_C_RAIN,
  S_C_HRAIN,
  S_C_FRAIN,
  S_C_LSNOW,
  S_C_SNOW,
  S_C_HSNOW,
  S_C_GRAINS,
  S_C_LSHOWERS,
  S_C_SHOWERS,
  S_C_HSHOWERS,
  S_C_SSHOWERS,
  S_C_THUNDER,
  S_C_HAIL,
  S_C_UNKNOWN,
  S_WAIT_WIFI,
  S_FETCH_NEWS,
  S_NEWS_N,
  S_NEWS,
  S_AI_BRIEF,
  S_NS_LINK,
  S_NS_REACH,
  S_NS_403,
  S_NS_404,
  S_NS_ERR,
  S_NS_NONE,
  S_NS_HINT,
  S_MEANING,
  S_STAY_FOCUSED,
  S_PAUSED,
  S_DONE_TODAY,
  S_HOLD_START,
  S_HOLD_PAUSE,
  S_HOLD_RESUME,
  S_FOCUS_MIN,
  S_RESUMED,
  S_TIMER_RESET,
  S_FOCUS_DONE,
  S_KEEP_HOLDING,
  S_SETUP_CLOSED,
  S_PAT_THANKS,
  S_H24,
  S_H12,
  S_UPDATING,
  S_UPDATING_NEWS,
  S_NEW_QUOTE,
  S_AI_LIMIT,
  S_AI_REACH,
  S_AI_KEY,
  S_AI_NOTALLOWED,
  S_AI_CREDITS,
  S_AI_BUSY,
  S_AI_CBUSY,
  S_AI_ERR,
  S_SU_SCAN,
  S_SU_CREDS,
  S_SU_CONNECTED,
  S_SU_IFNOT,
  S_LUNAR_RANGE,
  S_XMAS_EVE,
  S_XMAS_DAY,
  S_GOOD_FRIDAY,
  S_EASTER,
  S_G_XMAS,
  S_G_XMAS_EVE,
  S_G_EASTER,
  S_G_GOOD_FRIDAY,
  S_G_CNY,
  S_G_LANTERN,
  S_G_DRAGON,
  S_G_QIXI,
  S_G_GHOST,
  S_G_MIDAUTUMN,
  S_G_DOUBLE9,
  S_G_LABA,
  S_G_CNY_EVE,
  S_G_VESAK,
  S_B_GUANYIN,
  S_B_BUDDHA,
  S_B_ULAMBANA,
  S_B_BODHI,
  S_B_UPOSATHA,
  S_G_HIJRI_NY,
  S_G_MAWLID,
  S_G_ISRA,
  S_G_RAMADAN,
  S_G_EID_FITR,
  S_G_ARAFAH,
  S_G_EID_ADHA,
  S_HAPPY_FMT,
  S_HIJRI_CJK,
  S_BE_FMT,
  S_HINDU_FMT,
  S_SHUKLA,
  S_KRISHNA,
  S_IFTAR_IN,
  S_NEXT_PRAYER,
  S_PRAYER_TIMES,
  S_IMSAK,
  S_IFTAR,
  S_FASTING,
  S_RAMADAN_DAY,
  S_PRAYER_NOW,
  S_IFTAR_NOW,
  S_FETCH_QUOTE,
  S_TRANSLATION,
  S_REFLECTION,
  S_BREAK_LEFT,
  S_BREAK_OVER,
  S_BREATHE_IN,
  S_BREATHE_HOLD,
  S_BREATHE_OUT,
  S_BREATHE_TITLE,
  S_BREATHE_DONE,
  S_HOLD_SKIP,
  S_HOLD_STOP,
  S_HOLD_START2,
  S_LOTS_PATS,
  S_DIZZY,
  S_DROP,
  S_TILT,
  S_HIJRI_ADJ0,
  S_HIJRI_ADJP,
  S_HIJRI_ADJM,
  S_HOL_TODAY,
  S_HOL_TMR,
  S_HOL_IN,
  S_HOL_GREET,
  S_HEBREW_CJK,
  S_JF_ROSH,
  S_JF_YOM_KIPPUR,
  S_JF_SUKKOT,
  S_JF_SHEMINI,
  S_JF_SIMCHAT,
  S_JF_SHEMINI_SIMCHAT,
  S_JF_HANUKKAH,
  S_JF_TU_BISHVAT,
  S_JF_PURIM,
  S_JF_PESACH,
  S_JF_SHAVUOT,
  S_JF_TISHA_BAV,
  S_G_SHANA_TOVA,
  S_G_YOM_KIPPUR,
  S_G_CHAG,
  S_G_HANUKKAH,
  S_G_FAST,
  S_G_SHABBAT,
  S_CANDLES,
  S_AIR_QUALITY,
  S_AQI_GOOD,
  S_AQI_MODERATE,
  S_AQI_SENSITIVE,
  S_AQI_UNHEALTHY,
  S_AQI_VERY,
  S_AQI_HAZARD,
  S_AQI_ADVICE,
  S_GM_RPS,
  S_GM_TTT,
  S_GM_GUESS,
  S_GM_SIMON,
  S_GM_YOU_WIN,
  S_GM_I_WIN,
  S_GM_DRAW,
  S_GM_YOUR_TURN,
  S_GM_HMM,
  S_GM_HIGHER,
  S_GM_LOWER,
  S_GM_GOT_IT,
  S_GM_PICKED,
  S_GM_WATCH,
  S_GM_LEVEL,
  S_GM_OOPS,
  S_GM_SCORE,
  S_GM_YOU,
  S_GM_ME,
  S_AQI_COUGH,
  S_AQI_PAT,
  S_GM_ROLL,
  S_GM_TILT_ME,
  S_GM_TIME_UP,
  S_GM_NEW_BEST,
  S_GM_PHONE,
  S_GM_RIDDLE,
  S_GM_NOT_QUITE,
  S_GM_HINT,
  S_GM_IT_WAS,
  S_GM_NO_AI,
  S_GM_QUEST,
  S_GM_READY,
  S_GM_NICE,
  S_GM_MISS,
  S_GM_QUEST_WIN,
  S_GM_QUEST_LOSE,
  S_GM_TAP,
  S_GM_TAP_HOW,
  S_GM_WAIT,
  S_GM_TAP_NOW,
  S_GM_TOO_SOON,
  S_GM_TOO_SLOW,
  S_GM_MS,
  S_GM_TAP_AVG,
  S_GM_TAP_BEST,
  S_COUNT
};

static const char* const STR[S_COUNT][L_COUNT] = {
  {"Focus: %u:%02u left", "专注：还剩 %u:%02u", "專注：還剩 %u:%02u", "Fokus: sisa %u:%02u", "Fokus: noch %u:%02u", "Focus : encore %u:%02u", "集中：残り %u:%02u", "Enfoque: quedan %u:%02u"},   // FOCUS_LEFT
  {"It's %s", "现在 %s", "現在 %s", "Sekarang %s", "Es ist %s", "Il est %s", "今 %s", "Son las %s"},   // ITS_TIME
  {"Rain %d%% - grab a brolly!", "降雨 %d%%，记得带伞！", "降雨 %d%%，記得帶傘！", "Hujan %d%% - bawa payung!", "Regen %d%% - Schirm mitnehmen!", "Pluie %d%% - prends un parapluie !", "雨 %d%%、傘を忘れずに！", "Lluvia %d%%: ¡lleva paraguas!"},   // RAIN_ADVICE
  {"Hot one! Stay hydrated", "好热！记得多喝水", "好熱！記得多喝水", "Panas! Jangan lupa minum", "Heiß heute! Viel trinken", "Il fait chaud ! Bois de l'eau", "暑いね！水分補給を", "¡Hace calor! Bebe agua"},   // HOT
  {"Chilly today - rug up!", "今天有点冷，多穿点！", "今天有點冷，多穿點！", "Dingin hari ini, pakai jaket!", "Kühl heute - warm anziehen!", "Frais aujourd'hui, couvre-toi !", "今日は肌寒いね、暖かくして！", "Hace fresco, ¡abrígate!"},   // CHILLY
  {"Lovely day out there", "今天天气真好", "今天天氣真好", "Cuaca cerah hari ini", "Herrliches Wetter heute", "Belle journée dehors", "いい天気だね", "¡Qué día tan bonito!"},   // LOVELY
  {"Sunrise at %s", "日出 %s", "日出 %s", "Matahari terbit %s", "Sonnenaufgang %s", "Lever du soleil %s", "日の出 %s", "Amanece a las %s"},   // SUNRISE_AT
  {"Sunset at %s", "日落 %s", "日落 %s", "Matahari terbenam %s", "Sonnenuntergang %s", "Coucher du soleil %s", "日の入り %s", "Atardece a las %s"},   // SUNSET_AT
  {"Sunrise tomorrow %s", "明天日出 %s", "明天日出 %s", "Besok terbit %s", "Morgen Sonnenaufgang %s", "Lever demain %s", "明日の日の出 %s", "Mañana amanece a las %s"},   // SUNRISE_TMR
  {"Tomorrow %d/%d° %s", "明天 %d/%d° %s", "明天 %d/%d° %s", "Besok %d/%d° %s", "Morgen %d/%d° %s", "Demain %d/%d° %s", "明日 %d/%d度 %s", "Mañana %d/%d° %s"},   // TOMORROW_FC
  {"WiFi: %s", "WiFi：%s", "WiFi：%s", "WiFi: %s", "WLAN: %s", "Wi-Fi : %s", "WiFi：%s", "WiFi: %s"},   // WIFI_REASON
  {"WiFi not connected", "WiFi 未连接", "WiFi 未連線", "WiFi tidak terhubung", "WLAN nicht verbunden", "Wi-Fi non connecté", "WiFi 未接続", "WiFi sin conexión"},   // WIFI_NOT
  {"Connecting to WiFi...", "正在连接 WiFi…", "正在連線 WiFi…", "Menghubungkan WiFi...", "Verbinde mit WLAN...", "Connexion au Wi-Fi...", "WiFi に接続中…", "Conectando al WiFi..."},   // WIFI_CONNECTING
  {"Hold me 6 s to fix WiFi", "按住我 6 秒来修复 WiFi", "按住我 6 秒來修復 WiFi", "Tahan aku 6 detik untuk mengatur WiFi", "Halte mich 6 s fürs WLAN", "Maintiens-moi 6 s pour le Wi-Fi", "6秒押してWiFiを設定", "Mantenme 6 s para el WiFi"},   // HOLD_FIX_WIFI
  {"On backup WiFi", "正在使用备用 WiFi", "正在使用備用 WiFi", "Pakai WiFi cadangan", "Ersatz-WLAN aktiv", "Sur le Wi-Fi de secours", "予備の WiFi に接続中", "Usando el WiFi de reserva"},   // BACKUP_WIFI
  {"Hold me 6 s to set my clock", "按住我 6 秒来设置时间", "按住我 6 秒來設定時間", "Tahan aku 6 detik untuk mengatur jam", "Halte mich 6 s, um die Uhr zu stellen", "Maintiens-moi 6 s pour régler l'heure", "6秒押して時計を設定", "Mantenme 6 s para poner la hora"},   // HOLD_SET_CLOCK
  {"Settings: %s.local", "设置：%s.local", "設定：%s.local", "Pengaturan: %s.local", "Einstellungen: %s.local", "Réglages : %s.local", "設定：%s.local", "Ajustes: %s.local"},   // SETTINGS_AT
  {"or http://%s", "或 http://%s", "或 http://%s", "atau http://%s", "oder http://%s", "ou http://%s", "または http://%s", "o http://%s"},   // OR_HTTP
  {"Hi! Tap me :)", "你好！摸摸我 :)", "嗨！摸摸我 :)", "Hai! Sentuh aku :)", "Hallo! Tipp mich an :)", "Coucou ! Touche-moi :)", "やあ！タッチしてね :)", "¡Hola! Tócame :)"},   // TAP_ME
  {"wrong password?", "密码错误？", "密碼錯誤？", "kata sandi salah?", "falsches Passwort?", "mot de passe erroné ?", "パスワード違い？", "¿contraseña incorrecta?"},   // WR_PASS
  {"network not found", "找不到网络", "找不到網路", "jaringan tidak ditemukan", "Netz nicht gefunden", "réseau introuvable", "ネットワークが見つからない", "red no encontrada"},   // WR_NOTFOUND
  {"signal lost", "信号中断", "訊號中斷", "sinyal hilang", "Signal verloren", "signal perdu", "電波が切れた", "señal perdida"},   // WR_LOST
  {"can't connect", "无法连接", "無法連線", "tidak bisa terhubung", "keine Verbindung", "connexion impossible", "接続できない", "no se puede conectar"},   // WR_CANT
  {"am", "上午", "上午", "am", "am", "am", "午前", "am"},   // AM
  {"pm", "下午", "下午", "pm", "pm", "pm", "午後", "pm"},   // PM
  {"Syncing time...", "正在同步时间…", "正在同步時間…", "Menyinkronkan jam...", "Hole die Uhrzeit...", "Synchro de l'heure...", "時刻を同期中…", "Sincronizando hora..."},   // SYNCING
  {"Set my clock:", "设置时间：", "設定時間：", "Atur jamku:", "Uhr stellen:", "Règle mon heure :", "時計の設定：", "Pon mi hora:"},   // SET_CLOCK1
  {"hold me for 6 s,", "按住我 6 秒，", "按住我 6 秒，", "tahan aku 6 detik,", "halte mich 6 s,", "maintiens-moi 6 s,", "6秒押して、", "mantenme 6 s,"},   // SET_CLOCK2
  {"then scan my code", "再扫描二维码", "再掃描 QR 碼", "lalu pindai kodeku", "dann Code scannen", "puis scanne mon code", "コードを読み取って", "y escanea mi código"},   // SET_CLOCK3
  {"Fetching weather...", "正在获取天气…", "正在取得天氣…", "Mengambil cuaca...", "Hole das Wetter...", "Récup. de la météo...", "天気を取得中…", "Cargando el tiempo..."},   // FETCH_WEATHER
  {"No WiFi :(", "没有 WiFi :(", "沒有 WiFi :(", "Tidak ada WiFi :(", "Kein WLAN :(", "Pas de Wi-Fi :(", "WiFi なし :(", "Sin WiFi :("},   // NO_WIFI
  {"feels %d°", "体感 %d°", "體感 %d°", "serasa %d°", "gefühlt %d°", "ressenti %d°", "体感 %d度", "sensación %d°"},   // FEELS
  {"H %d°  L %d°  RH %d%%", "高%d° 低%d° 湿度%d%%", "高%d° 低%d° 濕度%d%%", "Maks %d° Min %d° RH %d%%", "Max %d° Min %d° LF %d%%", "Max %d° Min %d° Hum %d%%", "最高%d度 最低%d度 湿度%d%%", "Máx %d° Mín %d° Hum %d%%"},   // HI_LO_RH
  {"Rain %d%%  Wind %dkm/h", "降雨%d%% 风%dkm/h", "降雨%d%% 風%dkm/h", "Hujan %d%% Angin %dkm/j", "Regen %d%% Wind %dkm/h", "Pluie %d%% Vent %dkm/h", "雨%d%% 風%dkm/h", "Lluvia %d%% Viento %dkm/h"},   // RAIN_WIND
  {"Tomorrow", "明天", "明天", "Besok", "Morgen", "Demain", "明日", "Mañana"},   // TOMORROW
  {"Rain chance %d%%", "降雨概率 %d%%", "降雨機率 %d%%", "Peluang hujan %d%%", "Regenrisiko %d%%", "Risque de pluie %d%%", "降水確率 %d%%", "Prob. de lluvia %d%%"},   // RAIN_CHANCE
  {"Wind up to %d km/h", "风速最高 %d km/h", "風速最高 %d km/h", "Angin hingga %d km/j", "Wind bis %d km/h", "Vent jusqu'à %d km/h", "風速最大 %d km/h", "Viento hasta %d km/h"},   // WIND_UPTO
  {"Sun", "太阳", "太陽", "Matahari", "Sonne", "Soleil", "太陽", "Sol"},   // SUN
  {"Today", "今天", "今天", "Hari ini", "Heute", "Auj.", "今日", "Hoy"},   // TODAY
  {"Daylight %dh %02dm", "白昼 %d小时%02d分", "白晝 %d小時%02d分", "Siang %dj %02dm", "Tageslicht %dh %02dm", "Jour %dh %02dmin", "日中 %d時間%02d分", "Luz del día %dh %02dm"},   // DAYLIGHT
  {"Same length tomorrow", "明天一样长", "明天一樣長", "Besok sama panjang", "Morgen gleich lang", "Demain, même durée", "明日も同じ長さ", "Mañana, igual"},   // SAME_LENGTH
  {"%+d min tomorrow", "明天 %+d 分钟", "明天 %+d 分鐘", "Besok %+d menit", "Morgen %+d Min.", "Demain %+d min", "明日 %+d 分", "Mañana %+d min"},   // MIN_TOMORROW
  {"Clear sky", "晴朗", "晴朗", "Cerah", "Klarer Himmel", "Ciel dégagé", "快晴", "Despejado"},   // C_CLEAR
  {"Mainly clear", "大部晴朗", "大致晴朗", "Sebagian besar cerah", "Überwiegend klar", "Plutôt dégagé", "ほぼ晴れ", "Mayormente despejado"},   // C_MAINLY
  {"Partly cloudy", "多云间晴", "晴時多雲", "Berawan sebagian", "Teilweise bewölkt", "Partiellement nuageux", "晴れ時々曇り", "Parcialmente nublado"},   // C_PARTLY
  {"Overcast", "阴天", "陰天", "Mendung", "Bedeckt", "Couvert", "曇り", "Cubierto"},   // C_OVERCAST
  {"Foggy", "有雾", "有霧", "Berkabut", "Neblig", "Brouillard", "霧", "Niebla"},   // C_FOG
  {"Drizzle", "毛毛雨", "毛毛雨", "Gerimis", "Nieselregen", "Bruine", "霧雨", "Llovizna"},   // C_DRIZZLE
  {"Freezing drizzle", "冻毛毛雨", "凍毛毛雨", "Gerimis beku", "Gefrierender Niesel", "Bruine verglaçante", "着氷性の霧雨", "Llovizna helada"},   // C_FDRIZZLE
  {"Light rain", "小雨", "小雨", "Hujan ringan", "Leichter Regen", "Pluie légère", "小雨", "Lluvia ligera"},   // C_LRAIN
  {"Rain", "中雨", "中雨", "Hujan", "Regen", "Pluie", "雨", "Lluvia"},   // C_RAIN
  {"Heavy rain", "大雨", "大雨", "Hujan lebat", "Starker Regen", "Forte pluie", "大雨", "Lluvia fuerte"},   // C_HRAIN
  {"Freezing rain", "冻雨", "凍雨", "Hujan beku", "Gefrierender Regen", "Pluie verglaçante", "着氷性の雨", "Lluvia helada"},   // C_FRAIN
  {"Light snow", "小雪", "小雪", "Salju ringan", "Leichter Schnee", "Neige légère", "小雪", "Nieve ligera"},   // C_LSNOW
  {"Snow", "中雪", "中雪", "Salju", "Schnee", "Neige", "雪", "Nieve"},   // C_SNOW
  {"Heavy snow", "大雪", "大雪", "Salju lebat", "Starker Schnee", "Forte neige", "大雪", "Nevada fuerte"},   // C_HSNOW
  {"Snow grains", "米雪", "米雪", "Butiran salju", "Schneegriesel", "Neige en grains", "霧雪", "Granos de nieve"},   // C_GRAINS
  {"Light showers", "小阵雨", "小陣雨", "Hujan sebentar", "Leichte Schauer", "Averses légères", "弱いにわか雨", "Chubascos ligeros"},   // C_LSHOWERS
  {"Showers", "阵雨", "陣雨", "Hujan lokal", "Schauer", "Averses", "にわか雨", "Chubascos"},   // C_SHOWERS
  {"Heavy showers", "强阵雨", "強陣雨", "Hujan deras", "Starke Schauer", "Fortes averses", "強いにわか雨", "Chubascos fuertes"},   // C_HSHOWERS
  {"Snow showers", "阵雪", "陣雪", "Hujan salju", "Schneeschauer", "Averses de neige", "にわか雪", "Chubascos de nieve"},   // C_SSHOWERS
  {"Thunderstorm", "雷阵雨", "雷陣雨", "Badai petir", "Gewitter", "Orage", "雷雨", "Tormenta"},   // C_THUNDER
  {"Storm with hail", "雷阵雨伴冰雹", "雷陣雨伴冰雹", "Badai dan hujan es", "Gewitter mit Hagel", "Orage avec grêle", "ひょうを伴う雷雨", "Tormenta con granizo"},   // C_HAIL
  {"Unknown", "未知", "未知", "Tidak diketahui", "Unbekannt", "Inconnu", "不明", "Desconocido"},   // C_UNKNOWN
  {"Waiting for WiFi...", "等待 WiFi…", "等待 WiFi…", "Menunggu WiFi...", "Warte auf WLAN...", "Attente du Wi-Fi...", "WiFi 待ち…", "Esperando WiFi..."},   // WAIT_WIFI
  {"Fetching news...", "正在获取新闻…", "正在取得新聞…", "Mengambil berita...", "Hole Nachrichten...", "Récup. des infos...", "ニュース取得中…", "Cargando noticias..."},   // FETCH_NEWS
  {"News %d/%d", "新闻 %d/%d", "新聞 %d/%d", "Berita %d/%d", "News %d/%d", "Infos %d/%d", "ニュース %d/%d", "Noticias %d/%d"},   // NEWS_N
  {"News", "新闻", "新聞", "Berita", "News", "Infos", "ニュース", "Noticias"},   // NEWS
  {"AI briefing", "AI 简报", "AI 簡報", "Ringkasan AI", "KI-Überblick", "Résumé IA", "AI まとめ", "Resumen IA"},   // AI_BRIEF
  {"The %s link doesn't work", "%s 链接无效", "%s 連結無效", "Tautan %s tidak berfungsi", "Der %s-Link geht nicht", "Le lien %s ne marche pas", "%s のリンクが無効", "El enlace de %s no funciona"},   // NS_LINK
  {"Can't reach %s", "无法连接 %s", "無法連線 %s", "Tidak bisa menjangkau %s", "%s nicht erreichbar", "%s injoignable", "%s に接続できない", "No se puede llegar a %s"},   // NS_REACH
  {"%s blocked me (HTTP 403)", "%s 拒绝了我 (HTTP 403)", "%s 拒絕了我 (HTTP 403)", "%s memblokirku (HTTP 403)", "%s blockiert mich (HTTP 403)", "%s me bloque (HTTP 403)", "%s に拒否された (HTTP 403)", "%s me bloqueó (HTTP 403)"},   // NS_403
  {"%s feed not found (HTTP 404)", "找不到 %s 的订阅源 (HTTP 404)", "找不到 %s 的訂閱源 (HTTP 404)", "Feed %s tidak ada (HTTP 404)", "%s-Feed nicht gefunden (HTTP 404)", "Flux %s introuvable (HTTP 404)", "%s のフィードがない (HTTP 404)", "Feed de %s no encontrado (HTTP 404)"},   // NS_404
  {"%s error (HTTP %d)", "%s 出错 (HTTP %d)", "%s 出錯 (HTTP %d)", "%s galat (HTTP %d)", "%s-Fehler (HTTP %d)", "Erreur %s (HTTP %d)", "%s エラー (HTTP %d)", "Error de %s (HTTP %d)"},   // NS_ERR
  {"No headlines in the %s feed", "%s 订阅源里没有新闻", "%s 訂閱源裡沒有新聞", "Tidak ada berita di feed %s", "Keine Schlagzeilen im %s-Feed", "Aucun titre dans le flux %s", "%s のフィードに記事なし", "No hay titulares en %s"},   // NS_NONE
  {"%s. Hold me to try again, or pick another news source on the phone page.", "%s。按住我再试一次，或在手机页面换个新闻来源。", "%s。按住我再試一次，或在手機頁面換個新聞來源。", "%s. Tahan aku untuk coba lagi, atau pilih sumber berita lain di halaman ponsel.", "%s. Halte mich für einen neuen Versuch, oder wähle auf der Handyseite eine andere Quelle.", "%s. Maintiens-moi pour réessayer, ou choisis une autre source sur la page du téléphone.", "%s。長押しで再試行、またはスマホのページで別のニュースを選んでね。", "%s. Mantenme pulsado para reintentar o elige otra fuente en la página del móvil."},   // NS_HINT
  {"Meaning", "释义", "釋義", "Makna", "Bedeutung", "Sens", "意味", "Significado"},   // MEANING
  {"Stay focused! #%d", "专注中！第%d轮", "專注中！第%d輪", "Tetap fokus! #%d", "Bleib dran! #%d", "Concentration ! #%d", "集中！%d回目", "¡Concéntrate! #%d"},   // STAY_FOCUSED
  {"Paused", "已暂停", "已暫停", "Dijeda", "Pausiert", "En pause", "一時停止", "En pausa"},   // PAUSED
  {"Done today: %d", "今天完成：%d", "今天完成：%d", "Selesai hari ini: %d", "Heute geschafft: %d", "Faits auj. : %d", "今日の完了：%d", "Hechas hoy: %d"},   // DONE_TODAY
  {"hold to start", "按住开始", "按住開始", "tahan untuk mulai", "halten zum Starten", "maintiens pour lancer", "長押しで開始", "mantén para empezar"},   // HOLD_START
  {"hold:pause 2s:reset", "按住暂停 2秒重置", "按住暫停 2秒重設", "tahan:jeda 2d:ulang", "halten:Pause 2s:Reset", "maint.:pause 2s:raz", "押す:停止 2秒:初期化", "mant.:pausa 2s:borrar"},   // HOLD_PAUSE
  {"hold:resume 2s:reset", "按住继续 2秒重置", "按住繼續 2秒重設", "tahan:lanjut 2d:ulang", "halten:weiter 2s:neu", "maint.:reprise 2s:raz", "押す:再開 2秒:初期化", "mant.:sigue 2s:borrar"},   // HOLD_RESUME
  {"Focus! %u min", "专注！%u 分钟", "專注！%u 分鐘", "Fokus! %u menit", "Fokus! %u Min.", "Focus ! %u min", "集中！%u分", "¡Enfoque! %u min"},   // FOCUS_MIN
  {"Resumed", "继续", "繼續", "Lanjut", "Weiter", "Reprise", "再開", "Reanudado"},   // RESUMED
  {"Timer reset", "计时已重置", "計時已重設", "Timer diulang", "Timer zurück", "Minuteur à zéro", "タイマーをリセット", "Reloj reiniciado"},   // TIMER_RESET
  {"Focus done! Take a %u min break", "专注完成！休息 %u 分钟吧", "專注完成！休息 %u 分鐘吧", "Fokus selesai! Istirahat %u menit", "Geschafft! Jetzt %u Min. Pause", "Bravo ! Pause de %u min", "集中完了！%u分休憩しよう", "¡Hecho! Descansa %u min"},   // FOCUS_DONE
  {"Hold on for setup", "继续按住：设置", "繼續按住：設定", "Tahan: pengaturan", "Halten für Setup", "Garde le doigt...", "そのまま：設定", "Sigue: configurar"},   // KEEP_HOLDING
  {"Setup closed", "已关闭设置", "已關閉設定", "Pengaturan ditutup", "Setup beendet", "Réglages fermés", "設定を終了", "Ajustes cerrados"},   // SETUP_CLOSED
  {"Aww, thanks for the pat!", "嘿嘿，谢谢摸摸！", "嘿嘿，謝謝摸摸！", "Hehe, makasih elusannya!", "Oh, danke fürs Streicheln!", "Merci pour la caresse !", "なでてくれてありがとう！", "¡Gracias por la caricia!"},   // PAT_THANKS
  {"24-hour clock", "24 小时制", "24 小時制", "Jam 24 jam", "24-Stunden", "Format 24 h", "24時間表示", "Reloj 24 h"},   // H24
  {"12-hour clock", "12 小时制", "12 小時制", "Jam 12 jam", "12-Stunden", "Format 12 h", "12時間表示", "Reloj 12 h"},   // H12
  {"Updating...", "正在更新…", "正在更新…", "Memperbarui...", "Aktualisiere...", "Mise à jour...", "更新中…", "Actualizando..."},   // UPDATING
  {"Updating news...", "正在更新新闻…", "正在更新新聞…", "Memuat berita...", "Hole News...", "MàJ des infos...", "ニュース更新中…", "Nuevas noticias..."},   // UPDATING_NEWS
  {"New quote...", "换一句…", "換一句…", "Kutipan baru...", "Neues Zitat...", "Autre citation...", "新しい名言…", "Nueva cita..."},   // NEW_QUOTE
  {"AI: daily limit reached", "AI：已达每日上限", "AI：已達每日上限", "AI: batas harian tercapai", "KI: Tageslimit erreicht", "IA : limite du jour atteinte", "AI：今日の上限です", "IA: límite diario alcanzado"},   // AI_LIMIT
  {"AI: can't reach AI provider", "AI：无法连接 AI provider", "AI：無法連線 AI provider", "AI: AI provider tak terjangkau", "KI: AI provider nicht erreichbar", "IA : AI provider injoignable", "AI：AI provider に接続できない", "IA: no llego a AI provider"},   // AI_REACH
  {"AI: API key not accepted", "AI：API 密钥无效", "AI：API 金鑰無效", "AI: kunci API ditolak", "KI: API-Schlüssel abgelehnt", "IA : clé API refusée", "AI：APIキーが無効", "IA: clave API rechazada"},   // AI_KEY
  {"AI: key not allowed", "AI：密钥没有权限", "AI：金鑰沒有權限", "AI: kunci tidak diizinkan", "KI: Schlüssel nicht erlaubt", "IA : clé non autorisée", "AI：キーに権限がない", "IA: clave no permitida"},   // AI_NOTALLOWED
  {"AI: out of credits", "AI：额度已用完", "AI：額度已用完", "AI: kredit habis", "KI: Guthaben aufgebraucht", "IA : plus de crédits", "AI：クレジット切れ", "IA: sin créditos"},   // AI_CREDITS
  {"AI: busy, will retry", "AI：繁忙，稍后重试", "AI：忙碌，稍後重試", "AI: sibuk, coba lagi nanti", "KI: ausgelastet, neuer Versuch", "IA : occupée, nouvel essai", "AI：混雑中、再試行します", "IA: ocupada, reintentaré"},   // AI_BUSY
  {"AI: AI provider busy, will retry", "AI：AI provider 繁忙，稍后重试", "AI：AI provider 忙碌，稍後重試", "AI: AI provider sibuk, coba lagi", "KI: AI provider ausgelastet", "IA : AI provider occupé, nouvel essai", "AI：AI provider 混雑中", "IA: AI provider ocupado, reintentaré"},   // AI_CBUSY
  {"AI error %d", "AI 错误 %d", "AI 錯誤 %d", "Galat AI %d", "KI-Fehler %d", "Erreur IA %d", "AI エラー %d", "Error de IA %d"},   // AI_ERR
  {"Set me up!\n\nScan this\nwith your\nphone's\ncamera", "设置我吧\n用手机\n扫描这个\n二维码", "設定我吧！\n用手機相機\n掃描這個\nQR碼", "Atur aku!\n\nPindai ini\ndengan\nkamera\nponselmu", "Richte\nmich ein!\nScanne das\nmit der\nHandy-\nkamera", "Configure-\nmoi !\nScanne ça\navec ton\ntéléphone", "設定してね\nスマホの\nカメラで\nスキャン", "¡Ajústame!\n\nEscanea\nesto con\nla cámara\ndel móvil"},   // SU_SCAN
  {"WiFi name:\n%s\n\nPassword:\n%s\nhold=exit", "WiFi名\n%s\n密码\n%s", "WiFi名稱\n%s\n密碼\n%s", "Nama WiFi:\n%s\n\nSandi:\n%s\ntahan=stop", "WLAN-Name:\n%s\n\nPasswort:\n%s\nhalten=aus", "Nom Wi-Fi:\n%s\n\nMdp :\n%s\nmaint.=fin", "WiFi名\n%s\nパスワード\n%s", "Red WiFi:\n%s\n\nClave:\n%s\nmantén=fin"},   // SU_CREDS
  {"Connected!\n\nThe setup\npage opens\nby itself.", "已连接\n设置页\n会自动\n打开", "已連線！\n設定頁面\n會自動\n開啟。", "Terhubung!\n\nHalaman\npengaturan\nterbuka\nsendiri.", "Verbunden!\n\nDie Seite\nöffnet\nsich von\nselbst.", "Connecté !\n\nLa page\ns'ouvre\ntoute\nseule.", "接続OK！\n設定画面が\n自動で\n開きます", "¡Conectado\n\nLa página\nse abrirá\nsola."},   // SU_CONNECTED
  {"If not,\nscan again\nor open:\n\n192.168.4.1", "没打开？\n再扫一次\n或打开\n192.168.4.1", "如果沒有，\n再掃一次\n或開啟：\n192.168.4.1", "Belum?\n\nPindai\nulang atau\nbuka:\n192.168.4.1", "Sonst neu\nscannen\noder:\n\n192.168.4.1", "Sinon,\nrescanne\nou ouvre :\n\n192.168.4.1", "開かない時\n再スキャン\nか次を開く\n192.168.4.1", "Si no,\nescanea\nde nuevo o\nabre:\n192.168.4.1"},   // SU_IFNOT
  {"Lunar dates: 2000-2100", "农历：2000-2100", "農曆：2000-2100", "Imlek: 2000-2100", "Mondkalender: 2000-2100", "Lunaire : 2000-2100", "旧暦：2000-2100", "Lunar: 2000-2100"},   // LUNAR_RANGE
  {"Christmas Eve", "平安夜", "平安夜", "Malam Natal", "Heiligabend", "Réveillon de Noël", "クリスマスイブ", "Nochebuena"},   // XMAS_EVE
  {"Christmas Day", "圣诞节", "聖誕節", "Hari Natal", "Weihnachten", "Noël", "クリスマス", "Navidad"},   // XMAS_DAY
  {"Good Friday", "受难节", "耶穌受難日", "Jumat Agung", "Karfreitag", "Vendredi saint", "聖金曜日", "Viernes Santo"},   // GOOD_FRIDAY
  {"Easter Sunday", "复活节", "復活節", "Minggu Paskah", "Ostersonntag", "Pâques", "イースター", "Domingo de Pascua"},   // EASTER
  {"Merry Christmas!", "圣诞快乐！", "聖誕快樂！", "Selamat Natal!", "Frohe Weihnachten!", "Joyeux Noël !", "メリークリスマス！", "¡Feliz Navidad!"},   // G_XMAS
  {"It's Christmas Eve!", "今天是平安夜！", "今天是平安夜！", "Malam Natal tiba!", "Heute ist Heiligabend!", "C'est le réveillon !", "今日はクリスマスイブ！", "¡Es Nochebuena!"},   // G_XMAS_EVE
  {"Happy Easter!", "复活节快乐！", "復活節快樂！", "Selamat Paskah!", "Frohe Ostern!", "Joyeuses Pâques !", "ハッピーイースター！", "¡Felices Pascuas!"},   // G_EASTER
  {"Good Friday today", "今天是受难节", "今天是耶穌受難日", "Hari ini Jumat Agung", "Heute ist Karfreitag", "C'est le Vendredi saint", "今日は聖金曜日", "Hoy es Viernes Santo"},   // G_GOOD_FRIDAY
  {"Happy Chinese New Year!", "新年快乐！", "新年快樂！", "Selamat Tahun Baru Imlek!", "Frohes chinesisches Neujahr!", "Bonne année chinoise !", "春節おめでとう！", "¡Feliz Año Nuevo chino!"},   // G_CNY
  {"Happy Lantern Festival!", "元宵节快乐！", "元宵節快樂！", "Selamat Cap Go Meh!", "Frohes Laternenfest!", "Bonne fête des lanternes !", "元宵節おめでとう！", "¡Feliz Festival de los Faroles!"},   // G_LANTERN
  {"Happy Dragon Boat Festival!", "端午节快乐！", "端午節快樂！", "Selamat Peh Cun!", "Frohes Drachenbootfest!", "Bonne fête des bateaux-dragons !", "端午節おめでとう！", "¡Feliz Festival del Bote del Dragón!"},   // G_DRAGON
  {"Happy Qixi Festival!", "七夕快乐！", "七夕快樂！", "Selamat Hari Qixi!", "Frohes Qixi-Fest!", "Bonne fête de Qixi !", "七夕おめでとう！", "¡Feliz Qixi!"},   // G_QIXI
  {"Ghost Festival today", "今天是中元节", "今天是中元節", "Hari ini Cioko", "Heute ist Geisterfest", "C'est la fête des fantômes", "今日は中元節", "Hoy es el Festival de los Fantasmas"},   // G_GHOST
  {"Happy Mid-Autumn Festival!", "中秋节快乐！", "中秋節快樂！", "Selamat Festival Kue Bulan!", "Frohes Mondfest!", "Bonne fête de la mi-automne !", "中秋節おめでとう！", "¡Feliz Festival de Medio Otoño!"},   // G_MIDAUTUMN
  {"Happy Double Ninth Festival!", "重阳节快乐！", "重陽節快樂！", "Selamat Festival Ciong Yang!", "Frohes Doppel-Neun-Fest!", "Bonne fête du Double Neuf !", "重陽節おめでとう！", "¡Feliz Festival del Doble Nueve!"},   // G_DOUBLE9
  {"Laba Festival today", "今天是腊八节", "今天是臘八節", "Hari ini Festival Laba", "Heute ist Laba-Fest", "C'est la fête de Laba", "今日はろうはち節", "Hoy es el Festival Laba"},   // G_LABA
  {"Chinese New Year's Eve!", "除夕快乐！", "除夕快樂！", "Malam Tahun Baru Imlek!", "Chinesischer Silvester!", "Réveillon du Nouvel An chinois !", "旧暦の大晦日！", "¡Nochevieja china!"},   // G_CNY_EVE
  {"Happy Vesak!", "卫塞节快乐！", "衛塞節快樂！", "Selamat Hari Waisak!", "Frohes Vesakfest!", "Joyeux Vesak !", "ウェーサーカ祭おめでとう！", "¡Feliz Vesak!"},   // G_VESAK
  {"Guanyin's birthday", "观音诞", "觀音誕", "Hari lahir Kwan Im", "Guanyins Geburtstag", "Anniversaire de Guanyin", "観音様の誕生日", "Cumpleaños de Guanyin"},   // B_GUANYIN
  {"Buddha's birthday", "佛诞", "佛誕", "Hari lahir Buddha", "Buddhas Geburtstag", "Naissance du Bouddha", "花まつり", "Nacimiento de Buda"},   // B_BUDDHA
  {"Ullambana today", "今天是盂兰盆节", "今天是盂蘭盆節", "Hari ini Ullambana", "Heute ist Ullambana", "C'est Ullambana", "今日は盂蘭盆会", "Hoy es Ullambana"},   // B_ULAMBANA
  {"Bodhi Day", "佛陀成道日", "佛陀成道日", "Hari Bodhi", "Bodhi-Tag", "Jour de Bodhi", "成道会", "Día del Bodhi"},   // B_BODHI
  {"Uposatha day", "今天斋日", "今天齋日", "Hari Uposatha", "Uposatha-Tag", "Jour d'uposatha", "今日は布薩の日", "Día de uposatha"},   // B_UPOSATHA
  {"Happy Islamic New Year!", "伊斯兰新年快乐！", "伊斯蘭新年快樂！", "Selamat Tahun Baru Islam!", "Frohes islamisches Neujahr!", "Bonne année de l'Hégire !", "イスラム新年おめでとう！", "¡Feliz Año Nuevo islámico!"},   // G_HIJRI_NY
  {"Mawlid an-Nabi today", "今天是圣纪节", "今天是聖紀節", "Selamat Maulid Nabi!", "Heute ist Maulid an-Nabi", "C'est le Mawlid", "今日はマウリド", "Hoy es el Mawlid"},   // G_MAWLID
  {"Isra' Mi'raj today", "今天是登霄节", "今天是登霄節", "Hari ini Isra Mikraj", "Heute ist Isra und Miradsch", "C'est Isra et Miraj", "今日はイスラー・ミウラージュ", "Hoy es Isra y Miraj"},   // G_ISRA
  {"Ramadan Mubarak!", "斋月吉庆！", "齋月吉慶！", "Marhaban ya Ramadan!", "Ramadan Mubarak!", "Ramadan moubarak !", "ラマダン・ムバーラク！", "¡Ramadán Mubarak!"},   // G_RAMADAN
  {"Eid Mubarak!", "开斋节快乐！", "開齋節快樂！", "Selamat Idul Fitri!", "Frohes Zuckerfest!", "Bonne fête de l'Aïd !", "イード・ムバーラク！", "¡Feliz Eid al-Fitr!"},   // G_EID_FITR
  {"Day of Arafah", "今天是阿拉法日", "今天是阿拉法日", "Hari Arafah", "Tag von Arafat", "Jour d'Arafat", "今日はアラファの日", "Día de Arafat"},   // G_ARAFAH
  {"Eid al-Adha Mubarak!", "古尔邦节快乐！", "古爾邦節快樂！", "Selamat Idul Adha!", "Frohes Opferfest!", "Bonne fête de l'Aïd el-Kébir !", "犠牲祭おめでとう！", "¡Feliz Eid al-Adha!"},   // G_EID_ADHA
  {"Happy %s!", "%s 快乐！", "%s 快樂！", "Selamat %s!", "Frohes %s!", "Joyeux %s !", "%s おめでとう！", "¡Feliz %s!"},   // HAPPY_FMT
  {"%d/%d/%d H", "伊斯兰历%d年%d月%d日", "伊斯蘭曆%d年%d月%d日", "%d/%d/%d H", "%d/%d/%d H", "%d/%d/%d H", "ヒジュラ暦%d年%d月%d日", "%d/%d/%d H"},   // HIJRI_CJK
  {"Buddhist year %d", "佛历 %d 年", "佛曆 %d 年", "Tahun Buddha %d", "Buddh. Jahr %d", "Ère bouddhique %d", "仏暦 %d 年", "Año budista %d"},   // BE_FMT
  {"%s%s %s %d, VS %d", "%s%s %s%d VS%d", "%s%s %s%d VS%d", "%s%s %s %d, VS %d", "%s%s %s %d, VS %d", "%s%s %s %d, VS %d", "%s%s %s%d VS%d", "%s%s %s %d, VS %d"},   // HINDU_FMT
  {"Shukla", "白分", "白分", "Shukla", "Shukla", "Shukla", "白分", "Shukla"},   // SHUKLA
  {"Krishna", "黑分", "黑分", "Krishna", "Krishna", "Krishna", "黒分", "Krishna"},   // KRISHNA
  {"Iftar in %dh %02dm", "距开斋 %d小时%02d分", "距開齋 %d小時%02d分", "Buka puasa %dj %02dm", "Iftar in %d h %02d min", "Iftar dans %dh%02d", "イフタールまで%d時間%02d分", "Iftar en %dh %02dm"},   // IFTAR_IN
  {"%s at %s", "%s %s", "%s %s", "%s pukul %s", "%s um %s", "%s à %s", "%s %s", "%s a las %s"},   // NEXT_PRAYER
  {"Prayers", "礼拜时间", "禮拜時間", "Jadwal salat", "Gebetszeiten", "Prières", "礼拝時間", "Oraciones"},   // PRAYER_TIMES
  {"Imsak", "封斋", "封齋", "Imsak", "Imsak", "Imsak", "断食開始", "Imsak"},   // IMSAK
  {"Iftar", "开斋", "開齋", "Buka", "Iftar", "Iftar", "断食明け", "Iftar"},   // IFTAR
  {"Fasting times", "斋戒时间", "齋戒時間", "Waktu puasa", "Fastenzeiten", "Horaires du jeûne", "断食の時間", "Horario de ayuno"},   // FASTING
  {"Ramadan day %d", "斋月第%d天", "齋月第%d天", "Ramadan hari ke-%d", "Ramadan Tag %d", "Ramadan, jour %d", "ラマダン%d日目", "Ramadán, día %d"},   // RAMADAN_DAY
  {"Time for %s", "%s 礼拜时间到了", "%s 禮拜時間到了", "Waktunya salat %s", "Zeit für %s", "C'est l'heure de %s", "%s の時間です", "Es la hora de %s"},   // PRAYER_NOW
  {"Time to break the fast!", "开斋时间到了！", "開齋時間到了！", "Waktunya berbuka puasa!", "Zeit fürs Fastenbrechen!", "C'est l'heure de rompre le jeûne !", "断食明けの時間です！", "¡Hora de romper el ayuno!"},   // IFTAR_NOW
  {"Finding a quote...", "正在找名言…", "正在找名言…", "Mencari kutipan...", "Suche ein Zitat...", "Je cherche une citation...", "名言を探し中…", "Buscando una cita..."},   // FETCH_QUOTE
  {"Translation", "翻译", "翻譯", "Terjemahan", "Übersetzung", "Traduction", "翻訳", "Traducción"},   // TRANSLATION
  {"Reflection", "感悟", "感悟", "Renungan", "Gedanke", "Réflexion", "ひとこと", "Reflexión"},   // REFLECTION
  {"Break: %u:%02u left", "休息：还剩 %u:%02u", "休息：還剩 %u:%02u", "Istirahat: sisa %u:%02u", "Pause: noch %u:%02u", "Pause : encore %u:%02u", "休憩：残り %u:%02u", "Descanso: quedan %u:%02u"},   // BREAK_LEFT
  {"Break's over! Ready for another focus round?", "休息结束！准备好再专注一轮了吗？", "休息結束！準備好再專注一輪了嗎？", "Istirahat selesai! Siap fokus lagi?", "Pause vorbei! Bereit für die nächste Runde?", "Fin de la pause ! On repart pour une session ?", "休憩おわり！もう一回集中する？", "¡Fin del descanso! ¿Otra ronda de enfoque?"},   // BREAK_OVER
  {"Breathe in", "吸气", "吸氣", "Tarik napas", "Einatmen", "Inspire", "吸って", "Inhala"},   // BREATHE_IN
  {"Hold", "屏住", "屏住", "Tahan", "Halten", "Retiens", "止めて", "Mantén"},   // BREATHE_HOLD
  {"Breathe out", "呼气", "呼氣", "Embuskan", "Ausatmen", "Expire", "吐いて", "Exhala"},   // BREATHE_OUT
  {"Breathe", "呼吸", "呼吸", "Bernapas", "Atmen", "Respire", "深呼吸", "Respira"},   // BREATHE_TITLE
  {"Nice and calm", "很好，放松了", "很好，放鬆了", "Tenang sekali", "Schön ruhig", "Bien, tout calme", "落ち着いたね", "Muy bien, con calma"},   // BREATHE_DONE
  {"hold to skip break", "按住跳过休息", "按住跳過休息", "tahan: lewati", "halten: überspringen", "maint. : passer", "長押しでスキップ", "mantén: saltar"},   // HOLD_SKIP
  {"hold to stop", "按住停止", "按住停止", "tahan: berhenti", "halten: beenden", "maint. : arrêter", "長押しで終了", "mantén: parar"},   // HOLD_STOP
  {"hold:focus 2s:breathe", "按住专注 2秒呼吸", "按住專注 2秒呼吸", "tahan:fokus 2d:napas", "halten:Fokus 2s:Atmen", "maint.:focus 2s:respi", "押す:集中 2秒:呼吸", "mant.:foco 2s:respira"},   // HOLD_START2
  {"So many pats! I feel so loved", "摸了好多下！好幸福", "摸了好多下！好幸福", "Banyak sekali elusannya! Aku merasa disayang", "So viele Streicheleinheiten! Ich fühl mich geliebt", "Que de caresses ! Je me sens aimé", "いっぱいなでてくれた！しあわせ", "¡Cuántas caricias! Me siento querido"},   // LOTS_PATS
  {"Whoa, I'm dizzy!", "哇，好晕！", "哇，好暈！", "Waduh, pusing!", "Huch, mir ist schwindelig!", "Ouh, j'ai la tête qui tourne !", "わあ、目が回る！", "¡Uf, qué mareo!"},   // DIZZY
  {"Eek! Careful, I almost fell!", "哎呀！小心，我差点摔了！", "哎呀！小心，我差點摔了！", "Aduh! Hati-hati, aku hampir jatuh!", "Huch! Vorsicht, fast wäre ich gefallen!", "Aïe ! Attention, j'ai failli tomber !", "わっ！気をつけて、落ちそう！", "¡Ay! Cuidado, ¡casi me caigo!"},   // DROP
  {"I'm tilting! Set me straight?", "我歪了！扶正我好吗？", "我歪了！扶正我好嗎？", "Aku miring! Tolong luruskan?", "Ich kippe! Stellst du mich gerade?", "Je penche ! Tu me redresses ?", "傾いてるよ！まっすぐにして？", "¡Me inclino! ¿Me enderezas?"},   // TILT
  {"Hijri: standard", "伊斯兰历：标准", "伊斯蘭曆：標準", "Hijriah: standar", "Hidschra: Standard", "Hégire : standard", "ヒジュラ暦：標準", "Hégira: estándar"},   // HIJRI_ADJ0
  {"Hijri: +1 day", "伊斯兰历：+1天", "伊斯蘭曆：+1天", "Hijriah: +1 hari", "Hidschra: +1 Tag", "Hégire : +1 jour", "ヒジュラ暦：+1日", "Hégira: +1 día"},   // HIJRI_ADJP
  {"Hijri: -1 day", "伊斯兰历：-1天", "伊斯蘭曆：-1天", "Hijriah: -1 hari", "Hidschra: -1 Tag", "Hégire : -1 jour", "ヒジュラ暦：-1日", "Hégira: -1 día"},   // HIJRI_ADJM
  {"Public holiday: %s", "公众假期：%s", "公眾假期：%s", "Libur nasional: %s", "Feiertag: %s", "Jour férié : %s", "祝日：%s", "Festivo: %s"},   // HOL_TODAY
  {"Tomorrow: %s", "明天：%s", "明天：%s", "Besok: %s", "Morgen: %s", "Demain : %s", "明日は%s", "Mañana: %s"},   // HOL_TMR
  {"%s in %d days", "%s 还有 %d 天", "%s 還有 %d 天", "%s %d hari lagi", "%s in %d Tagen", "%s dans %d jours", "%sまであと%d日", "%s en %d días"},   // HOL_IN
  {"Today is %s. Enjoy the holiday!", "今天是%s，假期愉快！", "今天是%s，假期愉快！", "Hari ini %s. Selamat berlibur!", "Heute ist %s. Schönen Feiertag!", "Aujourd'hui : %s. Bon jour férié !", "今日は%s。よい休日を！", "Hoy es %s. ¡Feliz día festivo!"},   // HOL_GREET
  {"%d %s %d", "犹太历%d年%s%d日", "猶太曆%d年%s%d日", "%d %s %d", "%d %s %d", "%d %s %d", "ユダヤ暦%d年%s%d日", "%d %s %d"},   // HEBREW_CJK
  {"Rosh Hashanah", "犹太新年", "猶太新年", "Rosh Hashanah", "Rosch ha-Schana", "Roch Hachana", "ローシュ・ハッシャーナー", "Rosh Hashaná"},   // JF_ROSH
  {"Yom Kippur", "赎罪日", "贖罪日", "Yom Kippur", "Jom Kippur", "Yom Kippour", "ヨム・キプール", "Yom Kipur"},   // JF_YOM_KIPPUR
  {"Sukkot, day %d", "住棚节第%d天", "住棚節第%d天", "Sukkot hari ke-%d", "Sukkot, Tag %d", "Souccot, jour %d", "仮庵の祭り%d日目", "Sucot, día %d"},   // JF_SUKKOT
  {"Shemini Atzeret", "圣会节", "聖會節", "Shemini Atzeret", "Schemini Azeret", "Chemini Atseret", "シェミニ・アツェレト", "Sheminí Atzéret"},   // JF_SHEMINI
  {"Simchat Torah", "西赫托拉节", "西赫托拉節", "Simchat Torah", "Simchat Tora", "Sim'hat Torah", "シムハット・トーラー", "Simjat Torá"},   // JF_SIMCHAT
  {"Shemini Atzeret & Simchat Torah", "圣会节和西赫托拉节", "聖會節暨西赫托拉節", "Shemini Atzeret & Simchat Torah", "Schemini Azeret & Simchat Tora", "Chemini Atseret & Sim'hat Torah", "シェミニ・アツェレト／シムハット・トーラー", "Sheminí Atzéret y Simjat Torá"},   // JF_SHEMINI_SIMCHAT
  {"Hanukkah, day %d", "光明节第%d天", "光明節第%d天", "Hanukkah hari ke-%d", "Chanukka, Tag %d", "Hanoukka, jour %d", "ハヌカ%d日目", "Janucá, día %d"},   // JF_HANUKKAH
  {"Tu BiShvat", "树木新年", "樹木新年", "Tu BiShvat", "Tu Bischwat", "Tou Bichvat", "トゥ・ビ・シュヴァット", "Tu Bishvat"},   // JF_TU_BISHVAT
  {"Purim", "普林节", "普珥節", "Purim", "Purim", "Pourim", "プーリーム", "Purim"},   // JF_PURIM
  {"Passover, day %d", "逾越节第%d天", "逾越節第%d天", "Paskah Yahudi hari ke-%d", "Pessach, Tag %d", "Pessa'h, jour %d", "過越祭%d日目", "Pésaj, día %d"},   // JF_PESACH
  {"Shavuot", "七七节", "七七節", "Shavuot", "Schawuot", "Chavouot", "シャブオット", "Shavuot"},   // JF_SHAVUOT
  {"Tisha B'Av", "圣殿被毁日", "聖殿被毀日", "Tisha B'Av", "Tischa beAw", "Tisha BeAv", "ティシャ・ベ・アヴ", "Tisha b'Av"},   // JF_TISHA_BAV
  {"Shana tova! Happy New Year", "新年快乐！Shana tova", "新年快樂！Shana tova", "Shana tova! Selamat Tahun Baru", "Schana Towa! Gutes neues Jahr", "Chana tova ! Bonne année", "シャナ・トヴァ！良い一年を", "¡Shaná tová! Feliz Año Nuevo"},   // G_SHANA_TOVA
  {"Yom Kippur: G'mar chatima tova", "赎罪日：愿你蒙福", "贖罪日：願你蒙福", "Yom Kippur: G'mar chatima tova", "Jom Kippur: G'mar Chatima Towa", "Yom Kippour : Gmar 'hatima tova", "ヨム・キプール：良き一年を", "Yom Kipur: Gmar jatimá tová"},   // G_YOM_KIPPUR
  {"Chag sameach! %s", "节日快乐！%s", "節日快樂！%s", "Chag sameach! %s", "Chag Sameach! %s", "Hag saméa'h ! %s", "祭日おめでとう！%s", "¡Jag saméaj! %s"},   // G_CHAG
  {"Happy Hanukkah!", "光明节快乐！", "光明節快樂！", "Selamat Hanukkah!", "Frohes Chanukka!", "Joyeux Hanoukka !", "ハッピー・ハヌカ！", "¡Feliz Janucá!"},   // G_HANUKKAH
  {"Today is %s", "今天是%s", "今天是%s", "Hari ini %s", "Heute ist %s", "Aujourd'hui : %s", "今日は%s", "Hoy es %s"},   // G_FAST
  {"Shabbat shalom!", "安息日平安！", "安息日平安！", "Shabbat shalom!", "Schabbat Schalom!", "Chabbat chalom !", "シャバット・シャローム！", "¡Shabat shalom!"},   // G_SHABBAT
  {"Candle lighting %s", "安息日点烛 %s", "安息日點燭 %s", "Menyalakan lilin %s", "Kerzenzünden %s", "Allumage des bougies %s", "ろうそく点灯 %s", "Encendido de velas %s"},   // CANDLES
  {"Air quality", "空气质量", "空氣品質", "Kualitas udara", "Luftqualität", "Qualité de l'air", "空気の質", "Calidad del aire"},   // AIR_QUALITY
  {"Good", "优", "良好", "Baik", "Gut", "Bonne", "良い", "Buena"},   // AQI_GOOD
  {"Moderate", "良", "普通", "Sedang", "Mäßig", "Moyenne", "普通", "Moderada"},   // AQI_MODERATE
  {"Unhealthy for sensitive groups", "轻度污染", "對敏感族群不健康", "Tidak sehat bagi kelompok sensitif", "Ungesund für Empfindliche", "Mauvaise pour les personnes sensibles", "敏感な人には悪い", "Dañina para grupos sensibles"},   // AQI_SENSITIVE
  {"Unhealthy", "中度污染", "對所有族群不健康", "Tidak sehat", "Ungesund", "Mauvaise", "健康に悪い", "Dañina"},   // AQI_UNHEALTHY
  {"Very unhealthy", "重度污染", "非常不健康", "Sangat tidak sehat", "Sehr ungesund", "Très mauvaise", "とても悪い", "Muy dañina"},   // AQI_VERY
  {"Hazardous", "严重污染", "危害", "Berbahaya", "Gefährlich", "Dangereuse", "危険", "Peligrosa"},   // AQI_HAZARD
  {"Poor air (AQI %d): go easy outside", "空气较差（AQI %d），少在户外", "空氣較差（AQI %d），少在戶外", "Udara buruk (AQI %d), kurangi di luar", "Schlechte Luft (AQI %d): draußen sachte", "Air pollué (AQI %d) : doucement dehors", "空気が悪いよ（AQI %d）", "Aire malo (AQI %d): con calma fuera"},   // AQI_ADVICE
  {"Rock paper scissors", "石头剪刀布", "剪刀石頭布", "Gunting batu kertas", "Schere Stein Papier", "Pierre feuille ciseaux", "じゃんけん", "Piedra papel tijera"},   // GM_RPS
  {"Tic-tac-toe", "井字棋", "井字遊戲", "Tic-tac-toe", "Tic-Tac-Toe", "Morpion", "三目並べ", "Tres en raya"},   // GM_TTT
  {"Guess my number", "猜数字", "猜數字", "Tebak angkaku", "Rate meine Zahl", "Devine mon nombre", "数当て", "Adivina mi número"},   // GM_GUESS
  {"Memory", "记忆挑战", "記憶挑戰", "Ingatan", "Gedächtnis", "Mémoire", "記憶ゲーム", "Memoria"},   // GM_SIMON
  {"You win!", "你赢了！", "你贏了！", "Kamu menang!", "Du gewinnst!", "Tu gagnes !", "きみの勝ち！", "¡Ganas tú!"},   // GM_YOU_WIN
  {"I win!", "我赢了！", "我贏了！", "Aku menang!", "Ich gewinne!", "J'ai gagné !", "やった、勝ち！", "¡Gano yo!"},   // GM_I_WIN
  {"Draw!", "平局！", "平手！", "Seri!", "Remis!", "Égalité !", "引き分け！", "¡Empate!"},   // GM_DRAW
  {"Your turn", "轮到你了", "輪到你了", "Giliranmu", "Du bist dran", "À toi", "きみの番", "Tu turno"},   // GM_YOUR_TURN
  {"Hmm...", "让我想想……", "讓我想想……", "Hmm...", "Hmm...", "Hmm...", "うーん…", "Mmm..."},   // GM_HMM
  {"Higher!", "再大一点！", "再大一點！", "Lebih besar!", "Höher!", "Plus grand !", "もっと大きい！", "¡Más alto!"},   // GM_HIGHER
  {"Lower!", "再小一点！", "再小一點！", "Lebih kecil!", "Niedriger!", "Plus petit !", "もっと小さい！", "¡Más bajo!"},   // GM_LOWER
  {"Yes! %d tries", "猜中了！%d 次", "猜中了！%d 次", "Benar! %d kali", "Richtig! %d Versuche", "Gagné ! %d essais", "正解！%d 回", "¡Sí! %d intentos"},   // GM_GOT_IT
  {"I've picked 1-100", "我想好了：1-100", "我想好了：1-100", "Aku pilih 1-100", "Ich denke an 1-100", "J'ai choisi 1-100", "1-100 から選んだよ", "Pensé en 1-100"},   // GM_PICKED
  {"Watch me!", "看好了！", "看好了！", "Lihat aku!", "Schau genau!", "Regarde bien !", "よく見てね！", "¡Mírame!"},   // GM_WATCH
  {"Level %d", "第 %d 关", "第 %d 關", "Level %d", "Level %d", "Niveau %d", "レベル %d", "Nivel %d"},   // GM_LEVEL
  {"Oops! Best %d", "哎呀！最高 %d", "哎呀！最高 %d", "Ups! Terbaik %d", "Hoppla! Rekord %d", "Oups ! Record %d", "あっ！最高 %d", "¡Ups! Récord %d"},   // GM_OOPS
  {"You %d  Me %d", "你 %d 我 %d", "你 %d 我 %d", "Kamu %d Aku %d", "Du %d Ich %d", "Toi %d Moi %d", "きみ %d わたし %d", "Tú %d Yo %d"},   // GM_SCORE
  {"You", "你", "你", "Kamu", "Du", "Toi", "きみ", "Tú"},   // GM_YOU
  {"Me", "我", "我", "Aku", "Ich", "Moi", "わたし", "Yo"},   // GM_ME
  {"*cough* The air feels heavy today...", "咳咳……今天空气好闷……", "咳咳……今天空氣好悶……", "*uhuk* Udaranya berat hari ini...", "*hust* Die Luft ist heute so schwer...", "*tousse* L'air est lourd aujourd'hui...", "ゴホッ…今日は空気が重いね…", "*cof* El aire está pesado hoy..."},   // AQI_COUGH
  {"Thanks... let's stay cosy indoors today", "谢谢……今天就待在室内吧", "謝謝……今天就待在室內吧", "Makasih... hari ini di dalam saja, ya", "Danke... bleiben wir heute drinnen", "Merci... restons à l'intérieur aujourd'hui", "ありがとう…今日はおうちにいようね", "Gracias... hoy mejor quedarnos dentro"},   // AQI_PAT
  {"Star roll", "滚球摘星", "滾球摘星", "Bola bintang", "Sternenrollen", "Attrape-étoiles", "ほしころがし", "Rueda estrellas"},   // GM_ROLL
  {"Tilt me!", "把我倾斜！", "把我傾斜！", "Miringkan aku!", "Kipp mich!", "Penche-moi !", "かたむけて！", "¡Inclíname!"},   // GM_TILT_ME
  {"Time's up! %d stars", "时间到！%d 颗星", "時間到！%d 顆星", "Waktu habis! %d bintang", "Zeit um! %d Sterne", "Fini ! %d étoiles", "時間切れ！ほし %d こ", "¡Tiempo! %d estrellas"},   // GM_TIME_UP
  {"New best: %d stars!", "新纪录：%d 颗星！", "新紀錄：%d 顆星！", "Rekor baru: %d bintang!", "Neuer Rekord: %d!", "Record : %d étoiles !", "新記録：ほし %d こ！", "¡Récord: %d!"},   // GM_NEW_BEST
  {"Start on your phone", "在手机上开始吧", "在手機上開始吧", "Mulai di ponselmu", "Starte am Handy", "Lance sur ton tél.", "スマホで始めてね", "Empieza en el móvil"},   // GM_PHONE
  {"Riddle", "猜谜", "猜謎", "Teka-teki", "Rätsel", "Devinette", "なぞなぞ", "Adivinanza"},   // GM_RIDDLE
  {"Not quite!", "不对哦！", "不對哦！", "Belum tepat!", "Nicht ganz!", "Pas tout à fait !", "ちがうよ！", "¡No es eso!"},   // GM_NOT_QUITE
  {"Hint:", "提示：", "提示：", "Petunjuk:", "Tipp:", "Indice :", "ヒント：", "Pista:"},   // GM_HINT
  {"It was:", "答案是：", "答案是：", "Jawabannya:", "Lösung:", "C'était :", "答え：", "Era:"},   // GM_IT_WAS
  {"I need my AI key", "需要 AI 密钥", "需要 AI 金鑰", "Perlu kunci AI", "Brauche KI-Schlüssel", "Il me faut l'IA", "AI キーが必要", "Necesito la IA"},   // GM_NO_AI
  {"Quest", "冒险", "冒險", "Petualangan", "Abenteuer", "Aventure", "ぼうけん", "Aventura"},   // GM_QUEST
  {"Get ready...", "准备好……", "準備好……", "Bersiap...", "Mach dich bereit...", "Prépare-toi...", "じゅんびして…", "¡Prepárate!"},   // GM_READY
  {"Nice!", "好棒！", "好棒！", "Mantap!", "Super!", "Bravo !", "いいね！", "¡Bien!"},   // GM_NICE
  {"Missed!", "失手了！", "失手了！", "Meleset!", "Daneben!", "Raté !", "ミス！", "¡Fallaste!"},   // GM_MISS
  {"You made it! Score %d", "冒险成功！得分 %d", "冒險成功！得分 %d", "Berhasil! Skor %d", "Geschafft! %d Punkte", "Réussi ! Score %d", "やったね！%d 点", "¡Lo lograste! %d puntos"},   // GM_QUEST_WIN
  {"Out of hearts! Score %d", "没有爱心了！得分 %d", "沒有愛心了！得分 %d", "Nyawa habis! Skor %d", "Keine Herzen mehr! %d", "Plus de cœurs ! %d", "ハートがなくなった！%d 点", "¡Sin corazones! %d"},   // GM_QUEST_LOSE
  {"Quick tap", "快手拍拍", "快手拍拍", "Tepuk kilat", "Schnell tippen", "Tape vite", "はやタッチ", "Toque rápido"},   // GM_TAP
  {"Tap my head to start", "点我的头开始", "點我的頭開始", "Ketuk kepalaku untuk mulai", "Tipp auf meinen Kopf", "Touche ma tête", "頭をタッチでスタート", "Toca mi cabeza"},   // GM_TAP_HOW
  {"Wait for it...", "等一下……", "等一下……", "Tunggu...", "Warte...", "Attends...", "まだだよ…", "Espera..."},   // GM_WAIT
  {"TAP!", "拍！", "拍！", "TEPUK!", "JETZT!", "TAPE !", "いま！", "¡YA!"},   // GM_TAP_NOW
  {"Too soon!", "太早了！", "太早了！", "Terlalu cepat!", "Zu früh!", "Trop tôt !", "はやすぎ！", "¡Muy pronto!"},   // GM_TOO_SOON
  {"Too slow!", "太慢了！", "太慢了！", "Terlambat!", "Zu langsam!", "Trop lent !", "おそい！", "¡Muy lento!"},   // GM_TOO_SLOW
  {"%d ms", "%d 毫秒", "%d 毫秒", "%d ms", "%d ms", "%d ms", "%d ミリ秒", "%d ms"},   // GM_MS
  {"Average %d ms", "平均 %d 毫秒", "平均 %d 毫秒", "Rata-rata %d ms", "Schnitt %d ms", "Moyenne %d ms", "平均 %d ミリ秒", "Media %d ms"},   // GM_TAP_AVG
  {"New record: %d ms!", "新纪录：%d 毫秒！", "新紀錄：%d 毫秒！", "Rekor baru: %d ms!", "Rekord: %d ms!", "Record : %d ms !", "新記録：%d ミリ秒！", "Récord: %d ms!"},   // GM_TAP_BEST
};
#define T(s) (STR[(s)][LANG < L_COUNT ? LANG : 0])

static const char* const WEEKDAY[L_COUNT][7] = {
  {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"},
  {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"},
  {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"},
  {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"},
  {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"},
  {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"},
  {"日", "月", "火", "水", "木", "金", "土"},
  {"domingo", "lunes", "martes", "miércoles", "jueves", "viernes", "sábado"},
};
static const char* const MONTH_LONG[L_COUNT][12] = {   // empty for CJK (they write 10月)
  {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"},
  {"", "", "", "", "", "", "", "", "", "", "", ""},
  {"", "", "", "", "", "", "", "", "", "", "", ""},
  {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"},
  {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"},
  {"janvier", "février", "mars", "avril", "mai", "juin", "juillet", "août", "septembre", "octobre", "novembre", "décembre"},
  {"", "", "", "", "", "", "", "", "", "", "", ""},
  {"enero", "febrero", "marzo", "abril", "mayo", "junio", "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"},
};
static const char* const MONTH_SHORT[L_COUNT][12] = {   // empty for CJK (they write 10月)
  {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"},
  {"", "", "", "", "", "", "", "", "", "", "", ""},
  {"", "", "", "", "", "", "", "", "", "", "", ""},
  {"Jan", "Feb", "Mar", "Apr", "Mei", "Jun", "Jul", "Agu", "Sep", "Okt", "Nov", "Des"},
  {"Jan.", "Feb.", "März", "Apr.", "Mai", "Juni", "Juli", "Aug.", "Sep.", "Okt.", "Nov.", "Dez."},
  {"janv.", "févr.", "mars", "avr.", "mai", "juin", "juil.", "août", "sept.", "oct.", "nov.", "déc."},
  {"", "", "", "", "", "", "", "", "", "", "", ""},
  {"ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sept", "oct", "nov", "dic"},
};

// One-letter (or one-character) weekday heads for the month grid
static const char* const WD_MIN[L_COUNT][7] = {
  {"S", "M", "T", "W", "T", "F", "S"},
  {"日", "一", "二", "三", "四", "五", "六"},
  {"日", "一", "二", "三", "四", "五", "六"},
  {"M", "S", "S", "R", "K", "J", "S"},
  {"S", "M", "D", "M", "D", "F", "S"},
  {"D", "L", "M", "M", "J", "V", "S"},
  {"日", "月", "火", "水", "木", "金", "土"},
  {"D", "L", "M", "X", "J", "V", "S"},
};
static const char* const WD_SHORT[L_COUNT][7] = {   // the next-days forecast columns
  {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
  {"周日", "周一", "周二", "周三", "周四", "周五", "周六"},
  {"週日", "週一", "週二", "週三", "週四", "週五", "週六"},
  {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"},
  {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"},
  {"dim.", "lun.", "mar.", "mer.", "jeu.", "ven.", "sam."},
  {"日曜", "月曜", "火曜", "水曜", "木曜", "金曜", "土曜"},
  {"dom", "lun", "mar", "mié", "jue", "vie", "sáb"},
};
// Fajr, sunrise, Dhuhr, Asr, Maghrib, Isha. CJK names stay two characters: they share a
// half-width column with the time on the prayer page.
static const char* const PRAYER_NAME[L_COUNT][6] = {
  {"Fajr", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"},
  {"晨礼", "日出", "晌礼", "哺礼", "昏礼", "宵礼"},
  {"晨禮", "日出", "晌禮", "晡禮", "昏禮", "宵禮"},
  {"Subuh", "Terbit", "Zuhur", "Asar", "Magrib", "Isya"},
  {"Fadschr", "Sonnenaufgang", "Dhuhr", "Asr", "Maghrib", "Ischa"},
  {"Fajr", "Chourouq", "Dhohr", "Asr", "Maghreb", "Icha"},
  {"夜明", "日の出", "正午", "午後", "日没", "夜"},
  {"Fajr", "Amanecer", "Dhuhr", "Asr", "Magrib", "Isha"},
};
// Hebrew months, from Nisan = 1 (Tishrei = 7, Adar II = 13); [0] is Adar I in a leap year
static const char* const HEBREW_MONTH[L_COUNT][14] = {
  {"Adar I", "Nisan", "Iyar", "Sivan", "Tammuz", "Av", "Elul", "Tishrei", "Cheshvan", "Kislev", "Tevet", "Shevat", "Adar", "Adar II"},
  {"亚达一月", "尼散月", "依雅尔月", "西弯月", "搭模斯月", "埃波月", "以禄月", "提斯利月", "玛西班月", "基斯流月", "提别月", "细罢特月", "亚达月", "亚达二月"},
  {"亞達一月", "尼散月", "以珥月", "西彎月", "搭模斯月", "埃波月", "以祿月", "提斯利月", "瑪西班月", "基斯流月", "提別月", "細罷特月", "亞達月", "亞達二月"},
  {"Adar I", "Nisan", "Iyar", "Sivan", "Tammuz", "Av", "Elul", "Tishrei", "Cheshvan", "Kislev", "Tevet", "Shevat", "Adar", "Adar II"},
  {"Adar I", "Nisan", "Ijjar", "Siwan", "Tammus", "Aw", "Elul", "Tischri", "Cheschwan", "Kislew", "Tevet", "Schevat", "Adar", "Adar II"},
  {"Adar I", "Nissan", "Iyar", "Sivan", "Tammouz", "Av", "Eloul", "Tichri", "Hechvan", "Kislev", "Tevet", "Chevat", "Adar", "Adar II"},
  {"アダル1", "ニサン", "イヤール", "シワン", "タンムズ", "アブ", "エルル", "ティシュリ", "ヘシュワン", "キスレウ", "テベト", "シェバト", "アダル", "アダル2"},
  {"Adar I", "Nisán", "Iyar", "Siván", "Tamuz", "Av", "Elul", "Tishrei", "Jeshván", "Kislev", "Tevet", "Shevat", "Adar", "Adar II"},
};
// TC-EXTRA: 週日一二三四五六晨禮日出晌晡昏宵亞達一二月尼散以珥西彎搭模斯埃波祿提利瑪班基流別細罷特
