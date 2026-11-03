// Built-in European, modern and scripture quotes (the Chinese ones live in Momowo.ino with
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

// European thinkers and writers, each in the language they wrote in. The English line is the
// translation shown in English (nullptr for English writers) and is given to the AI so other
// translations stay faithful. Only authors whose works are long out of copyright.
enum EuroLang : uint8_t { EL_GREEK, EL_LATIN, EL_ENGLISH, EL_FRENCH, EL_GERMAN, EL_ITALIAN, EL_SPANISH,
                          EL_PORTUGUESE, EL_RUSSIAN };
static const char* const EURO_LANG_NAME[] = {"Ancient Greek", "Latin", "English", "French", "German",
                                             "Italian", "Spanish", "Portuguese", "Russian"};
struct EuroQuote { const char* text; const char* who; const char* english; uint8_t lang; uint16_t moods; };

static const EuroQuote EUROPE_QUOTES[] = {
  // Ancient Greek
  {"Τα πάντα ρει.", "Heraclitus", "Everything flows.", EL_GREEK, QM_AUTUMN|QM_WIND|QM_RAIN},
  {"Ποταμώ ουκ έστιν εμβήναι δις τω αυτώ.", "Heraclitus", "You cannot step into the same river twice.", EL_GREEK, QM_RAIN|QM_AUTUMN},
  {"Γνώθι σαυτόν.", "Delphic maxim", "Know yourself.", EL_GREEK, QM_STARS|QM_MOON|QM_AUTUMN},
  {"Μηδέν άγαν.", "Delphic maxim", "Nothing in excess.", EL_GREEK, QM_SUMMER},
  {"Ο δε ανεξέταστος βίος ου βιωτός ανθρώπω.", "Socrates", "The unexamined life is not worth living.", EL_GREEK, QM_STARS|QM_MOON|QM_AUTUMN},
  {"Εν οίδα ότι ουδέν οίδα.", "Socrates", "I know one thing: that I know nothing.", EL_GREEK, QM_STARS|QM_MOON},
  {"Αρχή ήμισυ παντός.", "Plato", "The beginning is half of everything.", EL_GREEK, QM_SPRING|QM_SUN},
  {"Μία χελιδών έαρ ου ποιεί.", "Aristotle", "One swallow does not make a spring.", EL_GREEK, QM_SPRING},
  {"Ο βίος βραχύς, η δε τέχνη μακρή.", "Hippocrates", "Life is short, and the art is long.", EL_GREEK, QM_AUTUMN},
  {"Λάθε βιώσας.", "Epicurus", "Live quietly, unnoticed.", EL_GREEK, QM_MOON|QM_STARS|QM_WINTER},
  {"Ταράσσει τους ανθρώπους ου τα πράγματα, αλλά τα δόγματα.", "Epictetus",
   "People are troubled not by things, but by their opinions about things.", EL_GREEK, QM_RAIN|QM_WIND|QM_CLOUD},
  {"Βάπτεται υπό των φαντασιών η ψυχή.", "Marcus Aurelius", "The soul takes on the colour of its thoughts.", EL_GREEK, QM_AUTUMN|QM_CLOUD},
  // Latin
  {"Carpe diem, quam minimum credula postero.", "Horace", "Seize the day, trusting as little as possible in tomorrow.", EL_LATIN, QM_SUN|QM_SPRING|QM_SUMMER},
  {"Dum loquimur, fugerit invida aetas.", "Horace", "While we are talking, envious time has already fled.", EL_LATIN, QM_AUTUMN|QM_WIND},
  {"Non est ad astra mollis e terris via.", "Seneca", "There is no easy way from the earth to the stars.", EL_LATIN, QM_STARS|QM_WINTER|QM_SNOW},
  {"Dum differtur vita transcurrit.", "Seneca", "While we put things off, life rushes past.", EL_LATIN, QM_AUTUMN|QM_WIND},
  {"Si vis amari, ama.", "Seneca", "If you want to be loved, love.", EL_LATIN, QM_SPRING|QM_SUN},
  {"Gutta cavat lapidem.", "Ovid", "Dripping water hollows out stone.", EL_LATIN, QM_RAIN|QM_WINTER},
  {"Vivere est cogitare.", "Cicero", "To live is to think.", EL_LATIN, QM_MOON|QM_STARS},
  // English
  {"This above all: to thine own self be true.", "Shakespeare", nullptr, EL_ENGLISH, QM_SUN|QM_SPRING},
  {"We are such stuff as dreams are made on, and our little life is rounded with a sleep.", "Shakespeare", nullptr, EL_ENGLISH, QM_MOON|QM_STARS},
  {"No man is an island, entire of itself.", "John Donne", nullptr, EL_ENGLISH, QM_CLOUD|QM_WINTER},
  // French
  {"Il faut cultiver notre jardin.", "Voltaire", "We must tend our garden.", EL_FRENCH, QM_SPRING|QM_SUMMER|QM_SUN},
  {"Le cœur a ses raisons que la raison ne connaît point.", "Pascal", "The heart has its reasons, which reason does not know.", EL_FRENCH, QM_MOON|QM_STARS|QM_AUTUMN},
  {"Tout le malheur des hommes vient d'une seule chose, qui est de ne savoir pas demeurer en repos dans une chambre.", "Pascal",
   "All of humanity's troubles come from one thing: not knowing how to sit quietly in a room.", EL_FRENCH, QM_RAIN|QM_WINTER|QM_SNOW},
  {"Je pense, donc je suis.", "Descartes", "I think, therefore I am.", EL_FRENCH, QM_STARS|QM_MOON},
  {"Patience et longueur de temps font plus que force ni que rage.", "La Fontaine", "Patience and time do more than strength or rage.", EL_FRENCH, QM_RAIN|QM_WINTER|QM_CLOUD},
  {"Que sais-je ?", "Montaigne", "What do I know?", EL_FRENCH, QM_CLOUD|QM_MOON},
  // German
  {"Es irrt der Mensch, so lang er strebt.", "Goethe", "As long as we strive, we make mistakes.", EL_GERMAN, QM_CLOUD|QM_RAIN|QM_AUTUMN},
  {"Edel sei der Mensch, hilfreich und gut!", "Goethe", "Let people be noble, helpful and good!", EL_GERMAN, QM_SUN|QM_SUMMER},
  {"Habe Mut, dich deines eigenen Verstandes zu bedienen!", "Kant", "Have the courage to use your own understanding!", EL_GERMAN, QM_SPRING|QM_SUN},
  {"Der bestirnte Himmel über mir und das moralische Gesetz in mir.", "Kant", "The starry sky above me and the moral law within me.", EL_GERMAN, QM_STARS|QM_MOON},
  {"Ohne Musik wäre das Leben ein Irrtum.", "Nietzsche", "Without music, life would be a mistake.", EL_GERMAN, QM_SUMMER|QM_SUN},
  // Italian
  {"E quindi uscimmo a riveder le stelle.", "Dante", "And then we came out to see the stars again.", EL_ITALIAN, QM_STARS|QM_MOON|QM_WINTER},
  {"Fatti non foste a viver come bruti, ma per seguir virtute e canoscenza.", "Dante",
   "You were not made to live like beasts, but to follow virtue and knowledge.", EL_ITALIAN, QM_SUN|QM_SPRING},
  {"Nel mezzo del cammin di nostra vita mi ritrovai per una selva oscura.", "Dante",
   "Midway along the journey of our life, I found myself in a dark wood.", EL_ITALIAN, QM_AUTUMN|QM_CLOUD},
  {"E il naufragar m'è dolce in questo mare.", "Leopardi", "And to be shipwrecked is sweet to me in this sea.", EL_ITALIAN, QM_MOON|QM_STARS|QM_WIND},
  // Spanish
  {"Caminante, no hay camino, se hace camino al andar.", "Antonio Machado", "Traveller, there is no road; the road is made by walking.", EL_SPANISH, QM_SPRING|QM_SUN|QM_WIND},
  {"Toda la vida es sueño, y los sueños, sueños son.", "Calderon de la Barca", "All of life is a dream, and dreams are only dreams.", EL_SPANISH, QM_MOON|QM_STARS},
  {"El que lee mucho y anda mucho, ve mucho y sabe mucho.", "Cervantes", "Whoever reads much and walks much sees much and knows much.", EL_SPANISH, QM_RAIN|QM_WINTER|QM_CLOUD},
  {"La diligencia es madre de la buena ventura.", "Cervantes", "Diligence is the mother of good fortune.", EL_SPANISH, QM_SUN|QM_SUMMER},
  // Portuguese
  {"Tudo vale a pena se a alma não é pequena.", "Fernando Pessoa", "Everything is worth it if the soul is not small.", EL_PORTUGUESE, QM_SUN|QM_SUMMER},
  {"Mudam-se os tempos, mudam-se as vontades.", "Camoes", "Times change, and so do our wishes.", EL_PORTUGUESE, QM_AUTUMN|QM_WIND},
  // Russian
  {"Все счастливые семьи похожи друг на друга, каждая несчастливая семья несчастлива по-своему.", "Leo Tolstoy",
   "All happy families are alike; each unhappy family is unhappy in its own way.", EL_RUSSIAN, QM_AUTUMN|QM_CLOUD},
  {"Красота спасёт мир.", "Dostoevsky", "Beauty will save the world.", EL_RUSSIAN, QM_SPRING|QM_SUN},
};
static const int EUROPE_COUNT = sizeof(EUROPE_QUOTES) / sizeof(EUROPE_QUOTES[0]);

static const EuroQuote* europeQuote(const char* text) {   // a European quote by its original text
  for (auto& q : EUROPE_QUOTES) if (!strcmp(q.text, text)) return &q;
  return nullptr;
}

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
