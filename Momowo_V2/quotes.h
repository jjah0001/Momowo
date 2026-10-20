// Built-in Ancient Greek, modern and scripture quotes (the Chinese ones live in Momowo.ino with
// their themes).
// Quotes are shown in their original language (the Bible in English). The English line goes with
// them: shown as the translation in English, and given to the AI so other translations stay faithful.
#pragma once

// Moods: one bit per quote theme, in QTheme order (Momowo.ino). Each quote lists the seasons, weather
// and times of day it suits, so every source can follow the moment like the Chinese poems do:
// comfort and patience for rain, snow, cloud and wind; hope and action for sunshine, morning and
// spring; rest and stillness for the moon and stars; change and letting go for autumn.
enum : uint16_t { QM_SPRING = 1, QM_SUMMER = 2, QM_AUTUMN = 4, QM_WINTER = 8, QM_RAIN = 16, QM_SNOW = 32,
                  QM_CLOUD = 64, QM_WIND = 128, QM_SUN = 256, QM_MOON = 512, QM_STARS = 1024 };

struct WorldQuote { const char* text; const char* who; const char* english; uint16_t moods; };

static const WorldQuote GREEK_QUOTES[] = {
  {"Τα πάντα ρει.", "Ηράκλειτος", "Everything flows.", QM_AUTUMN|QM_WIND|QM_RAIN},
  {"Ποταμώ ουκ έστιν εμβήναι δις τω αυτώ.", "Ηράκλειτος", "You cannot step into the same river twice.", QM_RAIN|QM_AUTUMN},
  {"Ήθος ανθρώπω δαίμων.", "Ηράκλειτος", "A person's character is their destiny.", QM_AUTUMN|QM_STARS},
  {"Φύσις κρύπτεσθαι φιλεί.", "Ηράκλειτος", "Nature loves to hide.", QM_CLOUD|QM_MOON},
  {"Γνώθι σαυτόν.", "Δελφοί", "Know yourself.", QM_STARS|QM_MOON|QM_AUTUMN},
  {"Μηδέν άγαν.", "Δελφοί", "Nothing in excess.", QM_SUMMER},
  {"Καιρόν γνώθι.", "Πιττακός", "Know the right moment.", QM_SPRING|QM_SUN},
  {"Μέτρον άριστον.", "Κλεόβουλος", "Moderation is best.", QM_SUMMER|QM_AUTUMN},
  {"Μελέτη το παν.", "Περίανδρος", "Practice is everything.", QM_SUN|QM_SUMMER},
  {"Ο δε ανεξέταστος βίος ου βιωτός ανθρώπω.", "Σωκράτης", "The unexamined life is not worth living.", QM_STARS|QM_MOON|QM_AUTUMN},
  {"Εν οίδα ότι ουδέν οίδα.", "Σωκράτης", "I know one thing: that I know nothing.", QM_STARS|QM_MOON},
  {"Αρχή ήμισυ παντός.", "Πλάτων", "The beginning is half of everything.", QM_SPRING|QM_SUN},
  {"Μία χελιδών έαρ ου ποιεί.", "Αριστοτέλης", "One swallow does not make a spring.", QM_SPRING},
  {"Ο άνθρωπος φύσει πολιτικόν ζώον.", "Αριστοτέλης", "Humans are by nature social beings.", QM_SUMMER|QM_SUN},
  {"Ο βίος βραχύς, η δε τέχνη μακρή.", "Ιπποκράτης", "Life is short, and the art is long.", QM_AUTUMN},
  {"Χρόνος γαρ ευμαρής θεός.", "Σοφοκλής", "Time is a gentle god.", QM_WINTER|QM_MOON|QM_CLOUD},
  {"Λάθε βιώσας.", "Επίκουρος", "Live quietly, unnoticed.", QM_MOON|QM_STARS|QM_WINTER},
  {"Κοσμοπολίτης ειμί.", "Διογένης", "I am a citizen of the world.", QM_SUMMER|QM_SUN},
  {"Εύρηκα!", "Αρχιμήδης", "I have found it!", QM_SUN},
  {"Δος μοι πα στω και ταν γαν κινάσω.", "Αρχιμήδης", "Give me a place to stand and I will move the earth.", QM_SUN|QM_SUMMER},
  {"Ταράσσει τους ανθρώπους ου τα πράγματα, αλλά τα δόγματα.", "Επίκτητος",
   "People are troubled not by things, but by their opinions about things.", QM_RAIN|QM_WIND|QM_CLOUD},
  {"Των όντων τα μεν εστίν εφ' ημίν, τα δε ουκ εφ' ημίν.", "Επίκτητος",
   "Some things are up to us, and some are not.", QM_WIND|QM_CLOUD|QM_SNOW},
  {"Βάπτεται υπό των φαντασιών η ψυχή.", "Μάρκος Αυρήλιος", "The soul takes on the colour of its thoughts.", QM_AUTUMN|QM_CLOUD},
  {"Ο κόσμος αλλοίωσις, ο βίος υπόληψις.", "Μάρκος Αυρήλιος", "The world is change; life is what we make of it.", QM_AUTUMN|QM_WIND},

};
static const int GREEK_COUNT = sizeof(GREEK_QUOTES) / sizeof(GREEK_QUOTES[0]);

// English names for the Greek authors (for the AI prompts and the English source line)
static const char* const GREEK_WHO_EN[][2] = {
  {"Ηράκλειτος", "Heraclitus"}, {"Δελφοί", "Delphic maxim"}, {"Πιττακός", "Pittacus"},
  {"Κλεόβουλος", "Cleobulus"}, {"Περίανδρος", "Periander"}, {"Σωκράτης", "Socrates"},
  {"Πλάτων", "Plato"}, {"Αριστοτέλης", "Aristotle"}, {"Ιπποκράτης", "Hippocrates"},
  {"Σοφοκλής", "Sophocles"}, {"Επίκουρος", "Epicurus"}, {"Διογένης", "Diogenes"},
  {"Αρχιμήδης", "Archimedes"}, {"Επίκτητος", "Epictetus"}, {"Μάρκος Αυρήλιος", "Marcus Aurelius"}};

static const WorldQuote MODERN_QUOTES[] = {
  {"Nothing in life is to be feared, it is only to be understood.", "Marie Curie", nullptr, QM_WIND|QM_RAIN},
  {"The best way out is always through.", "Robert Frost", nullptr, QM_RAIN|QM_SNOW|QM_WINTER|QM_WIND},
  {"In three words I can sum up everything I've learned about life: it goes on.", "Robert Frost", nullptr, QM_AUTUMN|QM_WINTER|QM_CLOUD},
  {"Do what you can, with what you have, where you are.", "Theodore Roosevelt", nullptr, QM_SUN|QM_SUMMER},
  {"Rest is not idleness.", "John Lubbock", nullptr, QM_MOON|QM_STARS|QM_SUMMER},
  {"Adopt the pace of nature: her secret is patience.", "Ralph Waldo Emerson", nullptr, QM_SPRING|QM_AUTUMN},
  {"Write it on your heart that every day is the best day in the year.", "Ralph Waldo Emerson", nullptr, QM_SUN|QM_SPRING},
  {"Not all those who wander are lost.", "J. R. R. Tolkien", nullptr, QM_AUTUMN|QM_WIND|QM_STARS},
  {"Hope is the thing with feathers that perches in the soul.", "Emily Dickinson", nullptr, QM_SPRING|QM_CLOUD|QM_RAIN},
  {"Act as if what you do makes a difference. It does.", "William James", nullptr, QM_SUN|QM_SUMMER},
  {"The art of being wise is the art of knowing what to overlook.", "William James", nullptr, QM_CLOUD|QM_AUTUMN},
  {"Life is like riding a bicycle. To keep your balance you must keep moving.", "Albert Einstein", nullptr, QM_SUMMER|QM_WIND|QM_SUN},
  {"Well done is better than well said.", "Benjamin Franklin", nullptr, QM_SUN|QM_SUMMER},
  {"Energy and persistence conquer all things.", "Benjamin Franklin", nullptr, QM_WINTER|QM_SNOW|QM_RAIN},
  {"You can't use up creativity. The more you use, the more you have.", "Maya Angelou", nullptr, QM_SPRING|QM_SUN},
  {"Do not let what you cannot do interfere with what you can do.", "John Wooden", nullptr, QM_CLOUD|QM_RAIN},
  {"Keep calm and carry on.", "British poster, 1939", nullptr, QM_RAIN|QM_WIND|QM_CLOUD},
  {"Be kind, for everyone you meet is fighting a hard battle.", "Ian Maclaren", nullptr, QM_WINTER|QM_RAIN|QM_CLOUD},

};
static const int MODERN_COUNT = sizeof(MODERN_QUOTES) / sizeof(MODERN_QUOTES[0]);

static const char* greekWhoEnglish(const char* who) {
  for (auto& e : GREEK_WHO_EN) if (!strcmp(e[0], who)) return e[1];
  return who;
}

// Scripture: short, comforting verses. The Bible in the King James Version (English, public
// domain); the Torah in unpointed Hebrew (the divine name written ה׳, as in many Jewish books), the
// Quran in Arabic without vowel marks, and the Dhammapada in romanised Pali. Partial verses are the
// well-known opening words.
// english: the King James Version for the Torah, F. Max Muller's 1881 translation for the
// Dhammapada (both public domain), and a short rendering of the meaning for the Quran.
struct Verse { const char* text; const char* english; const char* ref; uint16_t moods; };

static const Verse BIBLE_VERSES[] = {
  {"Be still, and know that I am God.", nullptr, "Psalm 46:10", QM_MOON|QM_STARS|QM_WIND},
  {"Come unto me, all ye that labour and are heavy laden, and I will give you rest.", nullptr, "Matthew 11:28", QM_MOON|QM_STARS|QM_RAIN|QM_CLOUD},
  {"Peace I leave with you, my peace I give unto you: not as the world giveth, give I unto you. "
   "Let not your heart be troubled, neither let it be afraid.", nullptr, "John 14:27", QM_MOON|QM_WIND|QM_CLOUD},
  {"Take therefore no thought for the morrow: for the morrow shall take thought for the things of itself.",
   nullptr, "Matthew 6:34", QM_MOON|QM_STARS|QM_AUTUMN},
  {"A merry heart doeth good like a medicine.", nullptr, "Proverbs 17:22", QM_SUN|QM_SUMMER},
  {"To every thing there is a season, and a time to every purpose under the heaven.", nullptr, "Ecclesiastes 3:1", QM_SPRING|QM_SUMMER|QM_AUTUMN|QM_WINTER},
  {"Casting all your care upon him; for he careth for you.", nullptr, "1 Peter 5:7", QM_RAIN|QM_CLOUD|QM_MOON|QM_AUTUMN},
  {"The LORD is my shepherd; I shall not want.", nullptr, "Psalm 23:1", QM_SPRING|QM_SUN},
  {"He healeth the broken in heart, and bindeth up their wounds.", nullptr, "Psalm 147:3", QM_RAIN|QM_CLOUD|QM_WINTER},
  {"Weeping may endure for a night, but joy cometh in the morning.", nullptr, "Psalm 30:5", QM_SUN|QM_MOON},
  {"Charity suffereth long, and is kind.", nullptr, "1 Corinthians 13:4", QM_SUMMER|QM_SPRING},
  {"Be ye kind one to another, tenderhearted, forgiving one another.", nullptr, "Ephesians 4:32", QM_SPRING|QM_SUMMER},
  {"A soft answer turneth away wrath.", nullptr, "Proverbs 15:1", QM_WIND|QM_CLOUD},
  {"Let all your things be done with charity.", nullptr, "1 Corinthians 16:14", QM_SUMMER|QM_SUN},
  {"Rejoicing in hope; patient in tribulation.", nullptr, "Romans 12:12", QM_RAIN|QM_SNOW|QM_WINTER},
  {"Blessed are the peacemakers: for they shall be called the children of God.", nullptr, "Matthew 5:9", QM_SPRING|QM_SUN},
  {"In quietness and in confidence shall be your strength.", nullptr, "Isaiah 30:15", QM_MOON|QM_STARS|QM_SNOW},

};

static const Verse TORAH_VERSES[] = {
  {"ואהבת לרעך כמוך", "Thou shalt love thy neighbour as thyself.", "Leviticus 19:18", QM_SUMMER|QM_SPRING|QM_SUN},
  {"יברכך ה׳ וישמרך", "The LORD bless thee, and keep thee.", "Numbers 6:24", QM_MOON|QM_STARS},
  {"יאר ה׳ פניו אליך ויחנך", "The LORD make his face shine upon thee, and be gracious unto thee.", "Numbers 6:25", QM_SUN},
  {"ישא ה׳ פניו אליך וישם לך שלום", "The LORD lift up his countenance upon thee, and give thee peace.", "Numbers 6:26", QM_MOON|QM_STARS|QM_CLOUD},
  {"חזקו ואמצו אל תיראו", "Be strong and of a good courage, fear not.", "Deuteronomy 31:6", QM_WIND|QM_RAIN|QM_SNOW|QM_WINTER},
  {"לא על הלחם לבדו יחיה האדם", "Man doth not live by bread only.", "Deuteronomy 8:3", QM_AUTUMN},
  {"צדק צדק תרדף", "That which is altogether just shalt thou follow.", "Deuteronomy 16:20", QM_SUN|QM_SUMMER},
  {"ששת ימים תעבד וביום השביעי תשבת", "Six days thou shalt work, but on the seventh day thou shalt rest.", "Exodus 34:21", QM_AUTUMN|QM_MOON},
  {"ואהבתם את הגר כי גרים הייתם בארץ מצרים",
   "Love ye therefore the stranger: for ye were strangers in the land of Egypt.", "Deuteronomy 10:19", QM_WINTER|QM_CLOUD},
  {"וירא אלהים את כל אשר עשה והנה טוב מאד",
   "And God saw every thing that he had made, and, behold, it was very good.", "Genesis 1:31", QM_SUN|QM_SPRING|QM_STARS},
  {"אל תיראו התיצבו וראו את ישועת ה׳", "Fear ye not, stand still, and see the salvation of the LORD.", "Exodus 14:13", QM_WIND|QM_RAIN},
  {"ובחרת בחיים למען תחיה אתה וזרעך", "Therefore choose life, that both thou and thy seed may live.", "Deuteronomy 30:19", QM_SPRING|QM_SUN},
  {"לא תשנא את אחיך בלבבך", "Thou shalt not hate thy brother in thine heart.", "Leviticus 19:17", QM_CLOUD|QM_AUTUMN},
  {"שמע ישראל ה׳ אלהינו ה׳ אחד", "Hear, O Israel: The LORD our God is one LORD.", "Deuteronomy 6:4", QM_SUN|QM_MOON|QM_STARS},
  {"ה׳ ילחם לכם ואתם תחרשון", "The LORD shall fight for you, and ye shall hold your peace.", "Exodus 14:14", QM_WIND|QM_RAIN|QM_SNOW},
  {"פתח תפתח את ידך לאחיך לעניך ולאבינך בארצך",
   "Thou shalt open thine hand wide unto thy brother, to thy poor, and to thy needy.", "Deuteronomy 15:11", QM_WINTER|QM_SNOW},

};

static const Verse DHAMMAPADA_VERSES[] = {
  {"Manopubbaṅgamā dhammā, manoseṭṭhā manomayā.", "All that we are is the result of what we have thought.", "Dhammapada 1", QM_CLOUD|QM_STARS|QM_AUTUMN},
  {"Na hi verena verāni, sammantīdha kudācanaṃ; averena ca sammanti, esa dhammo sanantano.",
   "Hatred does not cease by hatred at any time: hatred ceases by love, this is an old rule.", "Dhammapada 5", QM_WIND|QM_RAIN|QM_WINTER},
  {"Akkodhena jine kodhaṃ, asādhuṃ sādhunā jine.",
   "Let a man overcome anger by love, let him overcome evil by good.", "Dhammapada 223", QM_WIND|QM_SUMMER},
  {"Yo sahassaṃ sahassena, saṅgāme mānuse jine; ekañca jeyyamattānaṃ, sa ve saṅgāmajuttamo.",
   "If one man conquer in battle a thousand times thousand men, and if another conquer himself, "
   "he is the greatest of conquerors.", "Dhammapada 103", QM_SUN|QM_SUMMER},
  {"Selo yathā ekaghano, vātena na samīrati; evaṃ nindāpasaṃsāsu, na samiñjanti paṇḍitā.",
   "As a solid rock is not shaken by the wind, wise people falter not amidst blame and praise.", "Dhammapada 81", QM_WIND|QM_RAIN|QM_SNOW|QM_WINTER},
  {"Ārogyaparamā lābhā, santuṭṭhiparamaṃ dhanaṃ.", "Health is the greatest of gifts, contentedness the best riches.", "Dhammapada 204", QM_SUMMER|QM_AUTUMN|QM_SUN},
  {"Susukhaṃ vata jīvāma, verinesu averino.", "Let us live happily then, not hating those who hate us!", "Dhammapada 197", QM_SPRING|QM_SUN},
  {"Yo bālo maññati bālyaṃ, paṇḍito vāpi tena so.", "The fool who knows his foolishness, is wise at least so far.", "Dhammapada 63", QM_CLOUD},
  {"Appamādo amatapadaṃ, pamādo maccuno padaṃ.",
   "Earnestness is the path of immortality, thoughtlessness the path of death.", "Dhammapada 21", QM_SUN|QM_SPRING},
  {"Yathāpi ruciraṃ pupphaṃ, vaṇṇavantaṃ agandhakaṃ; evaṃ subhāsitā vācā, aphalā hoti akubbato.",
   "Like a beautiful flower, full of colour, but without scent, are the fine but fruitless words of "
   "him who does not act accordingly.", "Dhammapada 51", QM_SPRING|QM_SUMMER},
  {"Yo ca pubbe pamajjitvā, pacchā so nappamajjati; somaṃ lokaṃ pabhāseti, abbhā muttova candimā.",
   "He who formerly was reckless and afterwards became sober, brightens up this world, like the moon "
   "when freed from clouds.", "Dhammapada 172", QM_MOON|QM_CLOUD|QM_STARS},
  {"Upasanto sukhaṃ seti, hitvā jayaparājayaṃ.",
   "He who has given up both victory and defeat, he, the contented, is happy.", "Dhammapada 201", QM_MOON|QM_AUTUMN},

};

static const Verse QURAN_VERSES[] = {
  {"فإن مع العسر يسرا", "For indeed, with hardship comes ease.", "Quran 94:5", QM_RAIN|QM_SNOW|QM_WINTER|QM_CLOUD},
  {"إن مع العسر يسرا", "Indeed, with hardship comes ease.", "Quran 94:6", QM_RAIN|QM_SNOW|QM_WINTER|QM_CLOUD},
  {"ألا بذكر الله تطمئن القلوب", "Surely, in the remembrance of Allah do hearts find rest.", "Quran 13:28", QM_MOON|QM_STARS|QM_WIND},
  {"فاذكروني أذكركم واشكروا لي ولا تكفرون", "So remember Me; I will remember you. And be grateful to Me.", "Quran 2:152", QM_SUN|QM_MOON},
  {"واستعينوا بالصبر والصلاة", "And seek help through patience and prayer.", "Quran 2:45", QM_RAIN|QM_CLOUD|QM_WINTER},
  {"ما ودعك ربك وما قلى", "Your Lord has not forsaken you, nor is He displeased.", "Quran 93:3", QM_SUN|QM_MOON},
  {"ولا تهنوا ولا تحزنوا", "So do not weaken and do not grieve.", "Quran 3:139", QM_CLOUD|QM_RAIN|QM_AUTUMN},
  {"فبأي آلاء ربكما تكذبان", "So which of the favours of your Lord would you deny?", "Quran 55:13", QM_SPRING|QM_SUMMER|QM_STARS|QM_SUN},
  {"لا تخافا إنني معكما أسمع وأرى", "Fear not. Indeed, I am with you both; I hear and I see.", "Quran 20:46", QM_WIND|QM_SNOW|QM_RAIN},
  {"إن الله مع الصابرين", "Indeed, Allah is with the patient.", "Quran 8:46", QM_WINTER|QM_SNOW|QM_CLOUD},
  {"لا يكلف الله نفسا إلا وسعها", "Allah does not burden a soul beyond what it can bear.", "Quran 2:286", QM_RAIN|QM_WIND|QM_CLOUD},
  {"لا تقنطوا من رحمة الله", "Do not despair of the mercy of Allah.", "Quran 39:53", QM_RAIN|QM_MOON|QM_AUTUMN},

};

#define COUNT_OF(a) (int)(sizeof(a) / sizeof((a)[0]))
