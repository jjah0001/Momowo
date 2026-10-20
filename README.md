# Momowo 摸摸我

**Momowo** ("pat me" in Chinese) is a little desk friend that helps you relax. It sits next to your screen, and its animated eyes come alive when you pat it. Through the day it keeps you company with the time, your calendars and festivals, the weather, the news, a quote for the moment and a focus timer. You can also play games with it from your phone.

It's made for people from many different backgrounds. Momowo speaks eight languages (English, Simplified Chinese, Traditional Chinese, Indonesian, German, French, Japanese and Spanish). It can show the Chinese, Islamic, Buddhist, Jewish and Hindu calendars alongside the usual one, and its quotes come from many traditions, each in its original language.

You set it up entirely from your phone, so there's no app to install and no coding needed.

## What it does

- **Eyes that feel.** Momowo blinks, looks around and gets happy, sleepy, surprised or dizzy. It has one touch spot on its head, and it also senses when you pat, shake, tilt or drop it. Hearts float up when you pat it.
- **Clock and prayer times.** A big clock that handles summer time by itself, even offline. With the Islamic calendar on, it also shows the five daily prayers, each with its own icon, and during Ramadan it shows fasting times.
- **Calendars and holidays.** A month view with the public holidays of the country you're in, plus optional Chinese lunar, Islamic (Hijri), Buddhist, Jewish (Hebrew) and Hindu calendars with their festivals. On festival days Momowo greets you.
- **Weather and air quality.** The weather now and for the next few days, sunrise and sunset, and air quality (AQI, PM2.5, PM10). When the air is bad, Momowo looks tired and coughs.
- **News with a reporter character.** An AI summary of the headlines, read in the voice of a character you choose. It sometimes connects a story to your hobbies. News sources include English, Chinese and Indonesian outlets.
- **Quotes for the moment.** Classical Chinese poetry, Greek philosophy, the Bible, the Quran, the Torah, the Dhammapada, modern quotes and famous people you pick. Each is shown in its original language, then translated or explained in yours. The choice of quote suits the time of day, season and weather.
- **Focus timer.** A Pomodoro timer with a guided breathing exercise during breaks.
- **Games.** Rock paper scissors, tic-tac-toe, guess my number, Memory, Star roll (tilt Momowo to roll a ball), AI riddles, Quest (an AI adventure you play by tapping, holding, tilting and shaking Momowo) and Quick tap (a reaction game).
- **Optional AI.** Connect Claude, ChatGPT, Kimi or DeepSeek to get kind thoughts through the day, the news reporter, translated quotes, riddles and quests. Everything else works without it, and you can cap how many AI requests it makes per day.
- **Gentle at night.** Between bedtime and wake-up time it gets sleepy, dims, then switches its screen off.

## Guides

- **[User Guide](Momowo-User-Guide.md):** how to use a finished Momowo. No electronics knowledge needed, and it's available in all eight languages.
- **[Build Guide](Momowo-Build-Guide.md):** how to build one from parts, written for beginners. It covers the shopping list, soldering, flashing the firmware from your web browser, wiring and assembling the case. It ends with notes for tinkerers, including Momowo's JSON API.

## What's in this repository

| Path | What it is |
| --- | --- |
| [`Momowo_V2/`](Momowo_V2) | The Arduino sketch. Open `Momowo_V2.ino`, and the other files open as tabs. |
| [`Momowo-esp32c3-merged.bin`](Momowo-esp32c3-merged.bin) | Ready-to-flash firmware. Write it at address `0x0`, for example with the [ESP Web Flasher](https://espressif.github.io/esptool-js/). |
| [`enclosure/`](enclosure) | The 3D-printable case (front plate and back box), as STL for printing, STEP for other CAD tools and F3D for Fusion. |

## Hardware

- ESP32-C3 SuperMini
- 1.3" SH1106 128x64 OLED (I2C)
- MPU6050 motion sensor
- DS3231 real-time clock (optional, keeps time without Wi-Fi)
- TTP223 touch pad
- Passive buzzer
- The printed case, with four M3x12 screws

The Build Guide has the full parts list and wiring table.

## Building the firmware yourself

1. Install the Arduino IDE and the **esp32** boards package by Espressif.
2. Install these libraries: **Adafruit SH110X** (which brings in Adafruit GFX and BusIO), **ArduinoJson** 7 and **U8g2_for_Adafruit_GFX**.
3. Choose the board **ESP32C3 Dev Module**, set *Partition Scheme* to **Huge APP (3MB No OTA)** and turn on *USB CDC On Boot*.
4. Open `Momowo_V2/Momowo_V2.ino` and upload.

From the command line:

```
arduino-cli compile --fqbn esp32:esp32:esp32c3:PartitionScheme=huge_app,CDCOnBoot=cdc Momowo_V2
```

Wi-Fi passwords and AI keys are never stored in the code. You type them into Momowo's phone setup page, and they're kept on the device only.

## Credits

Weather, place search and air quality come from [Open-Meteo](https://open-meteo.com), with data from the Copernicus Atmosphere Monitoring Service. Public holidays come from [Nager.Date](https://date.nager.at). Quotes come from [ZenQuotes.io](https://zenquotes.io), 今日诗词 and Hitokoto. The Bible and Torah in English are from the King James Version, and the Dhammapada is in F. Max Müller's translation. Prayer times are based on PrayTimes.org. The fonts are Fusion Pixel by TakWolf (SIL Open Font License 1.1), WenQuanYi, and the u8g2 fonts by olikraus.
