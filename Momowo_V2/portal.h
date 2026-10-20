// Momowo setup page: served by Momowo itself to your phone (no app to install).
// Kept in this separate file because the Arduino IDE's function scanner misreads
// JavaScript inside .ino files. Edit the HTML/CSS/JS freely between the PORTAL markers.
// Every label is translated in the I18N table near the bottom: data-t sets an element's text,
// data-h its HTML and data-p its placeholder. The page follows the Language choice live.
#pragma once
#include <pgmspace.h>

static const char PORTAL_HTML[] PROGMEM = R"PORTAL(<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="apple-mobile-web-app-capable" content="yes"><meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-title" content="Momowo"><meta name="theme-color" content="#ff8fab">
<title>Momowo</title><style>
:root{--pink:#ff8fab;--deep:#e86a8e;--ink:#3d2c3a;--soft:#fff6f8;--line:#f4d6de}
*{box-sizing:border-box}body{margin:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Noto Sans",sans-serif;background:var(--soft);color:var(--ink)}
header{background:var(--pink);color:#fff;text-align:center;padding:26px 16px 22px;border-radius:0 0 28px 28px}
header h1{margin:0;font-size:34px;letter-spacing:4px}header p{margin:6px 0 0;font-size:15px;opacity:.95}
main{max-width:480px;margin:0 auto;padding:6px 14px 30px}
.card{background:#fff;border:1px solid var(--line);border-radius:20px;padding:14px 16px;margin:14px 0}
.card h2{font-size:17px;margin:0 0 4px}.card .sub{font-size:13px;opacity:.65;margin:0 0 6px}.opt{font-weight:400;opacity:.6}
label{display:block;font-size:14px;font-weight:600;margin:12px 0 5px}
input,select{width:100%;font-size:16px;padding:11px 12px;border:1px solid var(--line);border-radius:12px;background:#fff;color:var(--ink);-webkit-appearance:none;appearance:none}
input:focus,select:focus{outline:2px solid var(--pink);border-color:var(--pink)}
.row{display:flex;gap:10px}.row>div{flex:1}
.check{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:12px 0 2px;font-size:15px}
.check input{flex:none;width:24px;height:24px;-webkit-appearance:auto;appearance:auto;accent-color:var(--deep)}
input[type=range]{-webkit-appearance:auto;appearance:auto;padding:0;border:0;accent-color:var(--deep)}
.scale{display:flex;justify-content:space-between;font-size:12px;opacity:.6}
.inset{border-left:3px solid var(--line);padding-left:12px;margin:6px 0 4px}
.hint{font-size:13px;opacity:.7;margin:6px 0 0;line-height:1.4}a{color:var(--deep)}
.badge{display:inline-block;font-size:13px;padding:6px 10px;border-radius:10px;background:var(--soft);margin:4px 0}
button{width:100%;font-size:17px;padding:15px;border:0;border-radius:16px;background:var(--deep);color:#fff;font-weight:700;margin-top:8px}
button:disabled{opacity:.6}button.ghost{background:none;color:var(--ink);border:1px solid var(--line);font-weight:500;font-size:15px;padding:11px}
#done{display:none;text-align:center;padding:30px 10px}#done .big{font-size:64px;margin:10px 0}
.play{display:block;text-align:center;background:#fff;border:1px solid var(--line);border-radius:20px;padding:14px;margin:14px 0;font-size:17px;font-weight:700;text-decoration:none}
</style></head><body>
<header><h1>摸摸我</h1><p id="hello" data-t="hello"></p><p id="clk" style="display:none;font-size:13px" data-t="clk"></p></header>
<main><a class="play" href="/games" data-t="playLink"></a><div id="form">

<div class="card"><h2>🌐 <span data-t="language"></span></h2>
<select id="lang" onchange="relabel()"></select>
<p class="hint" data-t="langHint"></p></div>

<div class="card"><h2>📶 <span data-t="wifi"></span> <span class="opt" data-t="optional"></span></h2><p class="sub" data-t="wifiSub"></p>
<label for="ssid" data-t="network"></label><select id="ssid" onchange="sync()"></select>
<input id="ssidOther" data-p="typeNet" style="display:none;margin-top:8px">
<div id="passbox"><label for="pass" data-t="password"></label><input id="pass" type="password" autocomplete="off" data-p="password"></div>
<button class="ghost" type="button" onclick="scan()" data-t="rescan"></button>
<p class="hint" data-t="ghz"></p></div>

<div class="card"><h2>🔁 <span data-t="backup"></span> <span class="opt" data-t="optional"></span></h2>
<p class="sub" data-t="backupSub"></p>
<label for="ssid2" data-t="network"></label><select id="ssid2" onchange="sync()"></select>
<input id="ssid2Other" data-p="typeNet" style="display:none;margin-top:8px">
<div id="pass2box"><label for="pass2" data-t="password"></label><input id="pass2" type="password" autocomplete="off" data-p="password"></div>
<p class="hint" data-t="hotspot"></p></div>

<div class="card"><h2>😊 <span data-t="about"></span></h2>
<label for="name" data-t="name"></label><input id="name" maxlength="20" data-p="namePh">
<label for="gender"><span data-t="gender"></span> <span class="opt" data-t="optional"></span></label><select id="gender"></select>
<p class="hint" data-t="genderHint"></p>
<label for="hobbies"><span data-t="hobbies"></span> <span class="opt" data-t="optional"></span></label><input id="hobbies" maxlength="120" data-p="hobbiesPh">
<p class="hint" data-t="hobbiesHint"></p></div>

<div class="card"><h2>📍 <span data-t="where"></span></h2>
<div class="check"><span data-t="autoloc"></span><input id="autoloc" type="checkbox" onchange="sync()"></div>
<p class="hint" id="autohint" data-t="autoHint"></p>
<div id="locbox"><div class="row"><div><label for="suburb" data-t="suburb"></label><input id="suburb" maxlength="60" data-p="suburbPh"></div>
<div style="flex:.8"><label for="cc" data-t="country"></label><select id="cc" onchange="sync()"></select></div></div>
<p class="hint" data-t="locHint"></p></div></div>

<div class="card"><h2>🕑 <span data-t="clock"></span></h2>
<div class="check"><span data-t="h24"></span><input id="h24" type="checkbox"></div>
<div class="row"><div><label for="sleep" data-t="sleep"></label><select id="sleep"></select></div>
<div><label for="wake" data-t="wake"></label><select id="wake"></select></div></div>
<p class="hint" data-t="nightHint"></p></div>

<div class="card"><h2>📅 <span data-t="cals"></span></h2><p class="sub" data-t="calsSub"></p>
<div class="check"><span data-t="calCn"></span><input id="calCn" type="checkbox"></div>
<div class="check"><span data-t="calIs"></span><input id="calIs" type="checkbox" onchange="sync()"></div>
<div id="isbox" class="inset">
<label for="pmethod" data-t="pmethod"></label><select id="pmethod"></select>
<p class="hint" id="pmethodNow"></p>
<div class="check"><span data-t="hanafi"></span><input id="hanafi" type="checkbox"></div>
<div class="check"><span data-t="premind"></span><input id="premind" type="checkbox"></div>
<label for="hijriAdj" data-t="hijriAdj"></label><select id="hijriAdj"></select>
<p class="hint" data-t="hijriHint"></p></div>
<div class="check"><span data-t="calBu"></span><input id="calBu" type="checkbox"></div>
<div class="check"><span data-t="calHi"></span><input id="calHi" type="checkbox"></div>
<div class="check"><span data-t="calJu"></span><input id="calJu" type="checkbox"></div>
<p class="hint" data-t="calsHint"></p></div>

<div class="card"><h2>🍅 <span data-t="focus"></span></h2>
<div class="row"><div><label for="fmin" data-t="fmin"></label><select id="fmin"></select></div>
<div><label for="bmin" data-t="bmin"></label><select id="bmin"></select></div></div>
<p class="hint" data-t="focusHint"></p></div>

<div class="card"><h2>🤸 <span data-t="motion"></span></h2>
<p class="badge" id="mpu"></p>
<div class="check"><span data-t="motionOn"></span><input id="motion" type="checkbox"></div>
<label for="msens" data-t="msens"></label><input id="msens" type="range" min="1" max="3" step="1">
<div class="scale"><span data-t="gentle"></span><span data-t="sensitive"></span></div>
<div class="check"><span data-t="mirror"></span><input id="mmirror" type="checkbox"></div>
<p class="hint" data-t="motionHint"></p></div>

<div class="card"><h2>🔔 <span data-t="sounds"></span></h2>
<div class="check"><span data-t="soundOn"></span><input id="sound" type="checkbox"></div>
<label for="vol" data-t="volume"></label><input id="vol" type="range" min="1" max="10">
<div class="check"><span data-t="click"></span><input id="click" type="checkbox"></div>
<div class="check"><span data-t="chime"></span><input id="chime" type="checkbox"></div></div>

<div class="card"><h2>📰 <span data-t="news"></span></h2><p class="sub" data-t="needsWifi"></p>
<label for="news" data-t="newsSrc"></label><select id="news" onchange="sync()"></select>
<input id="newsUrl" placeholder="https://… (RSS)" style="display:none;margin-top:8px">
<div class="check"><span data-t="ntrans"></span><input id="ntrans" type="checkbox"></div>
<label for="nstyle" data-t="nstyle"></label><select id="nstyle"></select>
<label for="voice" data-t="voice"></label><select id="voice" onchange="sync()"></select>
<input id="voiceOwn" maxlength="60" data-p="voicePh" style="margin-top:8px">
<p class="hint" data-t="voiceHint"></p></div>

<div class="card"><h2>📜 <span data-t="quotes"></span></h2>
<p class="sub" data-t="qsrc"></p>
<div class="check"><span data-t="q0"></span><input id="qm0" type="checkbox"></div>
<div class="check"><span data-t="q1"></span><input id="qm1" type="checkbox"></div>
<div class="check"><span data-t="q2"></span><input id="qm2" type="checkbox"></div>
<div class="check"><span data-t="q3"></span><input id="qm3" type="checkbox"></div>
<div class="check"><span data-t="q4"></span><input id="qm4" type="checkbox"></div>
<div class="check"><span data-t="q5"></span><input id="qm5" type="checkbox"></div>
<div class="check"><span data-t="q6"></span><input id="qm6" type="checkbox"></div>
<div class="check"><span data-t="q7"></span><input id="qm7" type="checkbox" onchange="sync()"></div>
<input id="famous" maxlength="98" data-p="famousPh" style="display:none">
<p class="hint" data-t="q7Hint"></p>
<label for="qai" data-t="qai"></label><select id="qai"></select>
<p class="hint" data-t="quoteHint"></p></div>

<div class="card"><h2>✨ <span data-t="ai"></span> <span class="opt" data-t="optional"></span></h2>
<p class="sub" data-t="aiSub"></p>
<label for="aiprovider" data-t="aiProvider">AI provider</label><select id="aiprovider" onchange="aiProviderChanged()"><option value="0">Claude (Anthropic)</option><option value="1">ChatGPT (OpenAI API)</option><option value="2">Kimi (Moonshot)</option><option value="3">DeepSeek</option></select>
<label for="aikey" data-t="aikey"></label><input id="aikey" type="password" autocomplete="off" placeholder="API key" maxlength="511">
<div class="check" id="aiclearRow" style="display:none"><span data-t="aiclear"></span><input id="aiclear" type="checkbox"></div>
<label for="aimodel" data-t="aimodel"></label><select id="aimodel" onchange="aiModelSelections[+$('aiprovider').value]=this.value"></select>
<div id="aiEffortRow"><label for="aieffort" data-t="aieffort"></label><select id="aieffort"></select>
<p class="hint" data-t="aiModelHint"></p></div>
<label for="feelmin" data-t="feelEvery"></label><select id="feelmin"></select>
<p class="hint" data-t="feelHint"></p>
<label for="ailimit" data-t="aiLimit"></label><select id="ailimit"></select>
<p class="hint" id="aiUsed"></p>
<button class="ghost" type="button" id="aiReset" onclick="aiReset()" data-t="aiReset"></button>
<p class="hint" data-h="aiHint"></p></div>

<button id="save" onclick="save()" data-t="save"></button>
<button class="ghost" type="button" onclick="forget()" style="margin-top:22px" data-t="forget"></button>
</div>

<div id="done"><div class="big">💗</div><h2 id="doneTitle" data-t="saved"></h2>
<p id="doneText"></p>
<p class="hint" id="later" data-t="later"></p></div>
</main><script>
const $=i=>document.getElementById(i);let info={},L=0;
// en, zh-Hans, zh-Hant, id, de, fr, ja, es: the same order as Momowo's own Language setting
const I18N={
hello:["Hi! I'm Momowo. Let's get me set up.","你好！我是摸摸我，一起来设置吧。","你好！我是摸摸我，一起來設定吧。","Hai! Aku Momowo. Yuk, atur aku.","Hallo! Ich bin Momowo. Lass uns loslegen.","Coucou ! Je suis Momowo. On me configure ?","こんにちは！Momowo だよ。設定しよう。","¡Hola! Soy Momowo. Vamos a configurarme."],
clk:["🕑 My clock is now set from your phone.","🕑 我的时钟已按你的手机校准。","🕑 我的時鐘已按你的手機校準。","🕑 Jamku sudah disetel dari ponselmu.","🕑 Meine Uhr ist jetzt nach deinem Handy gestellt.","🕑 Mon horloge est réglée sur ton téléphone.","🕑 時計をスマホに合わせたよ。","🕑 Mi reloj ya está en hora con tu móvil."],
language:["Language","语言","語言","Bahasa","Sprache","Langue","言語","Idioma"],
langHint:["For everything I say: my screen, feelings, translations and news summaries.","用于我说的一切：屏幕文字、心情、翻译和新闻摘要。","用於我說的一切：螢幕文字、心情、翻譯和新聞摘要。","Untuk semua yang kukatakan: layar, perasaan, terjemahan, dan ringkasan berita.","Für alles, was ich sage: Bildschirm, Gefühle, Übersetzungen und News.","Pour tout ce que je dis : écran, humeurs, traductions et résumés d'infos.","画面、気持ち、翻訳、ニュースのまとめ、すべてこの言語で話すよ。","Para todo lo que digo: pantalla, emociones, traducciones y noticias."],
optional:["(optional)","（可选）","（可選）","(opsional)","(optional)","(facultatif)","（任意）","(opcional)"],
wifi:["Wi-Fi","Wi-Fi","Wi-Fi","Wi-Fi","WLAN","Wi-Fi","Wi-Fi","WiFi"],
wifiSub:["I work without it. With Wi-Fi I also show weather and news, and keep my clock exact.","没有也能用。连上 Wi-Fi 后还能显示天气和新闻，时钟也更准。","沒有也能用。連上 Wi-Fi 後還能顯示天氣和新聞，時鐘也更準。","Aku bisa bekerja tanpa itu. Dengan Wi-Fi aku juga menampilkan cuaca, berita, dan jam yang tepat.","Ich laufe auch ohne. Mit WLAN zeige ich Wetter und News und meine Uhr bleibt genau.","Je marche sans. Avec le Wi-Fi, j'affiche aussi la météo et les infos, et mon heure reste exacte.","なくても動くよ。Wi-Fi があれば天気やニュースも見られて、時計も正確になるよ。","Funciono sin él. Con WiFi también muestro el tiempo y las noticias, y mi reloj va exacto."],
network:["Network","网络","網路","Jaringan","Netzwerk","Réseau","ネットワーク","Red"],
typeNet:["Type the network name","输入网络名称","輸入網路名稱","Ketik nama jaringan","Netzwerkname eingeben","Nom du réseau","ネットワーク名を入力","Escribe el nombre de la red"],
password:["Password","密码","密碼","Kata sandi","Passwort","Mot de passe","パスワード","Contraseña"],
rescan:["↻ Look for networks again","↻ 重新搜索网络","↻ 重新搜尋網路","↻ Cari jaringan lagi","↻ Erneut nach Netzwerken suchen","↻ Chercher à nouveau","↻ もう一度さがす","↻ Buscar redes de nuevo"],
ghz:["I can only use 2.4 GHz Wi-Fi. Most home routers have it.","我只能用 2.4 GHz Wi-Fi，大多数家用路由器都有。","我只能用 2.4 GHz Wi-Fi，大多數家用路由器都有。","Aku hanya bisa memakai Wi-Fi 2,4 GHz. Kebanyakan router rumah punya.","Ich kann nur 2,4-GHz-WLAN. Die meisten Router haben es.","Je n'utilise que le Wi-Fi 2,4 GHz. La plupart des box l'ont.","2.4GHz の Wi-Fi だけ使えるよ。ほとんどのルーターにあるよ。","Solo uso WiFi de 2,4 GHz. Casi todos los routers lo tienen."],
backup:["Backup Wi-Fi","备用 Wi-Fi","備用 Wi-Fi","Wi-Fi cadangan","Ersatz-WLAN","Wi-Fi de secours","予備の Wi-Fi","WiFi de respaldo"],
backupSub:["For example your phone's hotspot, used when home Wi-Fi is out of reach.","比如手机热点，家里 Wi-Fi 连不上时使用。","比如手機熱點，家裡 Wi-Fi 連不上時使用。","Misalnya hotspot ponsel, dipakai saat Wi-Fi rumah tak terjangkau.","Zum Beispiel dein Handy-Hotspot, wenn das Heim-WLAN fehlt.","Par exemple le partage de connexion de ton téléphone, si le Wi-Fi est hors de portée.","たとえばスマホのテザリング。家の Wi-Fi が届かないときに使うよ。","Por ejemplo, el punto de acceso del móvil, cuando no llega el WiFi de casa."],
hotspot:["On an iPhone, turn on Personal Hotspot → Maximise Compatibility.","iPhone 请打开个人热点 → 最大兼容性。","iPhone 請開啟個人熱點 → 最大相容性。","Di iPhone, nyalakan Hotspot Pribadi → Maksimalkan Kompatibilitas.","Am iPhone: Persönlicher Hotspot → Kompatibilität maximieren.","Sur iPhone : Partage de connexion → Maximiser la compatibilité.","iPhone ではインターネット共有 →「互換性を優先」をオンにしてね。","En iPhone, activa Punto de acceso personal → Maximizar compatibilidad."],
about:["About you","关于你","關於你","Tentang kamu","Über dich","À propos de toi","あなたのこと","Sobre ti"],
name:["Your name","你的名字","你的名字","Namamu","Dein Name","Ton prénom","あなたの名前","Tu nombre"],
namePh:["So I can say good morning","这样我就能跟你说早安","這樣我就能跟你說早安","Supaya aku bisa bilang selamat pagi","Damit ich Guten Morgen sagen kann","Pour te dire bonjour","おはようって言えるように","Para darte los buenos días"],
q7:["Famous people (you choose)","名人名言（你来选）","名人名言（你來選）","Tokoh terkenal (pilihanmu)","Berühmte Menschen (deine Wahl)","Personnalités (à toi de choisir)","有名人の言葉（自分で選ぶ）","Personajes famosos (tú eliges)"],
famousPh:["e.g. Einstein, Marie Curie, Jack Ma","例如：爱因斯坦、居里夫人、马云","例如：愛因斯坦、居禮夫人、馬雲","mis. Einstein, B.J. Habibie, Marie Curie","z. B. Einstein, Marie Curie, Goethe","ex. Einstein, Marie Curie, Victor Hugo","例：アインシュタイン、マリー・キュリー、イチロー","p. ej. Einstein, Marie Curie, Frida Kahlo"],
q7Hint:["For famous people, my AI picks a real quote by someone you name, to suit the moment. It needs an AI key; without one I show a modern quote instead.","名人名言由我的 AI 从你填写的人里挑一句真实的话，配合当下。需要 AI 密钥，没有时改为现代名言。","名人名言由我的 AI 從你填寫的人裡挑一句真實的話，配合當下。需要 AI 金鑰，沒有時改為現代名言。","Untuk tokoh terkenal, AI-ku memilih kutipan asli dari orang yang kamu tulis, sesuai suasana. Perlu kunci AI; tanpa itu aku tampilkan kutipan modern.","Für berühmte Menschen wählt meine KI ein echtes Zitat einer Person, die du nennst, passend zum Moment. Braucht einen KI-Schlüssel, sonst zeige ich ein modernes Zitat.","Pour les personnalités, mon IA choisit une vraie citation d'une personne que tu nommes, selon le moment. Clé AI nécessaire, sinon j'affiche une citation moderne.","有名人の言葉は、入力した人の本当の名言を AI がその時に合わせて選ぶよ。AI キーが必要。ないときは現代の名言を出すよ。","Para personajes famosos, mi IA elige una cita real de alguien que nombres, según el momento. Necesita clave de IA; sin ella muestro una cita moderna."],
playLink:["🎮 Play a game with me","🎮 和我玩个游戏","🎮 和我玩個遊戲","🎮 Main game bersamaku","🎮 Spiel mit mir","🎮 Joue avec moi","🎮 いっしょにゲームしよう","🎮 Juega conmigo"],
hobbies:["Your hobbies & interests","你的爱好","你的興趣","Hobi & minatmu","Deine Hobbys","Tes loisirs","趣味や好きなこと","Tus aficiones"],
hobbiesPh:["e.g. badminton, K-dramas, baking","例如：羽毛球、韩剧、烘焙","例如：羽毛球、韓劇、烘焙","mis. bulu tangkis, drama Korea, membuat kue","z. B. Badminton, K-Dramen, Backen","ex. badminton, K-dramas, pâtisserie","例：バドミントン、韓国ドラマ、お菓子作り","p. ej. bádminton, K-dramas, repostería"],
hobbiesHint:["So I can chat in a more personal way. Only sent to the AI, and only when AI is on.","让我聊天更贴心。只在开启 AI 时发给 AI。","讓我聊天更貼心。只在開啟 AI 時傳給 AI。","Supaya obrolanku lebih personal. Hanya dikirim ke AI, dan hanya jika AI aktif.","Damit ich persönlicher plaudern kann. Nur an die KI, und nur wenn KI an ist.","Pour discuter de façon plus perso. Partagé seulement avec l'IA, si elle est activée.","もっとあなたらしい話ができるように。AI がオンのときだけ AI に送るよ。","Para charlar de forma más personal. Solo se comparte con la IA, y solo si está activada."],
where:["Where you are","你的位置","你的位置","Lokasimu","Wo du bist","Où tu es","いる場所","Dónde estás"],
autoloc:["Find my location automatically","自动定位","自動定位","Temukan lokasiku otomatis","Ort automatisch finden","Trouver ma position automatiquement","場所を自動で調べる","Encontrar mi ubicación automáticamente"],
autoHint:["I'll work out your city from your internet connection. Needs Wi-Fi.","我会根据网络连接判断你的城市，需要 Wi-Fi。","我會根據網路連線判斷你的城市，需要 Wi-Fi。","Aku akan menebak kotamu dari koneksi internet. Perlu Wi-Fi.","Ich erkenne deine Stadt an der Internetverbindung. Braucht WLAN.","Je devine ta ville grâce à ta connexion. Wi-Fi nécessaire.","インターネット接続から街を調べるよ。Wi-Fi が必要。","Averiguo tu ciudad por la conexión a internet. Necesita WiFi."],
suburb:["Suburb or area","区域或街区","區域或街區","Kelurahan atau area","Ort oder Stadtteil","Quartier ou ville","地域・町名","Barrio o zona"],
suburbPh:["e.g. Tampines","例如：淡滨尼","例如：淡濱尼","mis. Kemang","z. B. Schwabing","ex. Montmartre","例：渋谷","p. ej. Chamberí"],
country:["Country","国家/地区","國家/地區","Negara","Land","Pays","国・地域","País"],
locHint:["For weather, sunrise and prayer times closer to home. Needs Wi-Fi.","让天气、日出和礼拜时间更贴近你家，需要 Wi-Fi。","讓天氣、日出和禮拜時間更貼近你家，需要 Wi-Fi。","Agar cuaca, matahari terbit, dan jadwal salat lebih tepat. Perlu Wi-Fi.","Für Wetter, Sonnenaufgang und Gebetszeiten nah bei dir. Braucht WLAN.","Pour la météo, le lever du soleil et les prières au plus près. Wi-Fi nécessaire.","天気、日の出、礼拝時間をより正確にするため。Wi-Fi が必要。","Para el tiempo, el amanecer y las oraciones más exactos. Necesita WiFi."],
clock:["Clock & bedtime","时钟与作息","時鐘與作息","Jam & waktu tidur","Uhr & Schlafenszeit","Heure & coucher","時計と寝る時間","Reloj y hora de dormir"],
h24:["24-hour clock","24 小时制","24 小時制","Jam 24 jam","24-Stunden-Uhr","Format 24 h","24時間表示","Reloj de 24 h"],
sleep:["I sleep at","我几点睡","我幾點睡","Aku tidur jam","Ich schlafe um","Je me couche à","寝る時間","Me duermo a las"],
wake:["I wake at","我几点起","我幾點起","Aku bangun jam","Ich wache auf um","Je me lève à","起きる時間","Me despierto a las"],
nightHint:["I dim my screen and stay quiet at night.","晚上我会调暗屏幕，保持安静。","晚上我會調暗螢幕，保持安靜。","Di malam hari layarku redup dan aku diam.","Nachts dimme ich den Bildschirm und bin leise.","La nuit, j'atténue mon écran et je reste silencieux.","夜は画面を暗くして静かにしているよ。","Por la noche atenúo la pantalla y me quedo callado."],
cals:["Calendars","日历","日曆","Kalender","Kalender","Calendriers","カレンダー","Calendarios"],
calsSub:["The Western (Gregorian) calendar and the public holidays of the country you're in are always on. Add any of these:","公历和你所在国家的公众假期一直显示，还可以加上：","公曆和你所在國家的公眾假期一直顯示，還可以加上：","Kalender Masehi dan hari libur nasional negaramu selalu tampil. Tambahkan juga:","Der gregorianische Kalender und die Feiertage deines Landes sind immer an. Dazu kannst du wählen:","Le calendrier grégorien et les jours fériés de ton pays sont toujours là. Tu peux ajouter :","西暦と、いる国の祝日はいつも表示。ほかにも追加できるよ：","El calendario gregoriano y los festivos de tu país siempre están. Puedes añadir:"],
calCn:["Chinese lunar calendar & festivals","农历与传统节日","農曆與傳統節日","Kalender Imlek & hari raya","Chinesischer Mondkalender & Feste","Calendrier lunaire chinois & fêtes","旧暦（中国暦）と行事","Calendario lunar chino y fiestas"],
calIs:["Islamic (Hijri) calendar & prayer times","伊斯兰历与礼拜时间","伊斯蘭曆與禮拜時間","Kalender Hijriah & jadwal salat","Islamischer Kalender & Gebetszeiten","Calendrier hégirien & heures de prière","ヒジュラ暦と礼拝時間","Calendario islámico y oraciones"],
pmethod:["Prayer time method","礼拜时间算法","禮拜時間算法","Metode jadwal salat","Berechnungsmethode","Méthode de calcul","礼拝時間の計算方法","Método de cálculo"],
auto:["Automatic (from my country)","自动（按国家）","自動（按國家）","Otomatis (sesuai negara)","Automatisch (nach Land)","Automatique (selon le pays)","自動（国に合わせる）","Automático (según el país)"],
nowUsing:["Now using: ","目前使用：","目前使用：","Sekarang: ","Aktuell: ","Actuellement : ","現在：","Ahora: "],
hanafi:["Later Asr (Hanafi)","晡礼晚算（哈乃斐派）","晡禮晚算（哈乃斐派）","Asar lebih lambat (Hanafi)","Späteres Asr (hanafitisch)","Asr plus tardif (hanafite)","遅いアスル（ハナフィー派）","Asr más tarde (hanafí)"],
premind:["Chime at each prayer time","每次礼拜时间提醒","每次禮拜時間提醒","Bunyikan saat waktu salat","Bei jeder Gebetszeit klingen","Sonner à chaque prière","礼拝時間にお知らせ","Avisar en cada oración"],
hijriAdj:["Hijri date adjustment","伊斯兰历日期调整","伊斯蘭曆日期調整","Penyesuaian tanggal Hijriah","Hidschra-Datum anpassen","Ajuster la date de l'Hégire","ヒジュラ暦の日付調整","Ajuste de la fecha islámica"],
hijriHint:["If the moon sighting where you live is a day ahead or behind, set it here. You can also hold me on the calendar page.","如果当地见月日期早一天或晚一天，可以在这里调整，也可以在日历页面按住我。","如果當地見月日期早一天或晚一天，可以在這裡調整，也可以在日曆頁面按住我。","Jika rukyat di tempatmu maju atau mundur sehari, atur di sini. Bisa juga dengan menahanku di halaman kalender.","Wenn die Mondsichtung bei dir einen Tag abweicht, stell es hier ein oder halte mich auf der Kalenderseite.","Si l'observation de la lune chez toi a un jour d'écart, règle-le ici, ou maintiens-moi sur la page calendrier.","地域の新月観測と1日ずれるときはここで調整。カレンダー画面で長押ししても変えられるよ。","Si el avistamiento de la luna donde vives va un día adelantado o atrasado, ajústalo aquí o mantenme pulsado en el calendario."],
calBu:["Buddhist calendar & holy days","佛历与佛教节日","佛曆與佛教節日","Kalender Buddha & hari suci","Buddhistischer Kalender & Feiertage","Calendrier bouddhique & jours saints","仏暦と仏教の行事","Calendario budista y días sagrados"],
calHi:["Hindu calendar (panchang) & festivals","印度教历（潘查格）与节日","印度教曆（潘查格）與節日","Kalender Hindu (panchang) & hari raya","Hindu-Kalender (Panchang) & Feste","Calendrier hindou (panchang) & fêtes","ヒンドゥー暦（パンチャーンガ）と祭り","Calendario hindú (panchang) y fiestas"],
calsHint:["Religious dates are calculated, not announced: check with your community for the exact day.","宗教日期为推算结果，具体日子请以当地宗教团体公布为准。","宗教日期為推算結果，具體日子請以當地宗教團體公布為準。","Tanggal keagamaan dihitung, bukan diumumkan: cek ke komunitasmu untuk tanggal pastinya.","Religiöse Daten sind berechnet, nicht verkündet: frag deine Gemeinde nach dem genauen Tag.","Les dates religieuses sont calculées, pas annoncées : vérifie auprès de ta communauté.","宗教の日付は計算によるものだよ。正確な日はコミュニティに確認してね。","Las fechas religiosas son calculadas, no anunciadas: confírmalas con tu comunidad."],
focus:["Focus timer","专注计时","專注計時","Timer fokus","Fokus-Timer","Minuteur de concentration","集中タイマー","Temporizador de enfoque"],
fmin:["Focus for","专注","專注","Fokus selama","Fokus","Concentration","集中","Enfoque"],
bmin:["Then rest for","然后休息","然後休息","Lalu istirahat","Dann Pause","Puis pause","休憩","Luego descanso"],
min:["min","分钟","分鐘","menit","Min.","min","分","min"],
focusHint:["Every fourth break is three times longer. Hold me for 2 seconds on the timer page for a calming breathing exercise.","每第四次休息会长三倍。在计时页面按住我 2 秒，可以做放松呼吸练习。","每第四次休息會長三倍。在計時頁面按住我 2 秒，可以做放鬆呼吸練習。","Setiap istirahat keempat tiga kali lebih lama. Tahan aku 2 detik di halaman timer untuk latihan napas.","Jede vierte Pause ist dreimal so lang. Halte mich 2 Sekunden auf der Timer-Seite für eine Atemübung.","Une pause sur quatre est trois fois plus longue. Maintiens-moi 2 s sur le minuteur pour un exercice de respiration.","4回目ごとの休憩は3倍長いよ。タイマー画面で2秒長押しすると深呼吸ができるよ。","Cada cuarto descanso es tres veces más largo. Mantenme 2 s en el temporizador para un ejercicio de respiración."],
motion:["Motion sensor","动作传感器","動作感應器","Sensor gerak","Bewegungssensor","Capteur de mouvement","モーションセンサー","Sensor de movimiento"],
mpuYes:["✅ MPU6050 found. Pat me, shake me or tilt me!","✅ 找到 MPU6050。拍拍我、摇摇我、歪歪我吧！","✅ 找到 MPU6050。拍拍我、搖搖我、歪歪我吧！","✅ MPU6050 ditemukan. Tepuk, goyang, atau miringkan aku!","✅ MPU6050 gefunden. Klopf, schüttel oder kipp mich!","✅ MPU6050 trouvé. Tapote-moi, secoue-moi, penche-moi !","✅ MPU6050 を見つけたよ。なでたり、振ったり、傾けたりしてね！","✅ MPU6050 encontrado. ¡Dame palmaditas, agítame o inclíname!"],
mpuNo:["No MPU6050 found. My touch pad still works as usual.","没有找到 MPU6050，触摸照常可用。","沒有找到 MPU6050，觸控照常可用。","MPU6050 tidak ditemukan. Sentuhan tetap berfungsi.","Kein MPU6050 gefunden. Das Touchfeld geht trotzdem.","Pas de MPU6050. Le toucher marche toujours.","MPU6050 が見つからないよ。タッチはいつも通り使えるよ。","No encontré el MPU6050. El toque sigue funcionando."],
motionOn:["React to pats, shakes and tilts","感应拍拍、摇晃和倾斜","感應拍拍、搖晃和傾斜","Bereaksi pada tepukan, goyangan & kemiringan","Auf Klopfen, Schütteln und Kippen reagieren","Réagir aux tapes, secousses et inclinaisons","なでる・振る・傾けるに反応する","Reaccionar a palmaditas, sacudidas e inclinación"],
msens:["Sensitivity","灵敏度","靈敏度","Kepekaan","Empfindlichkeit","Sensibilité","感度","Sensibilidad"],
gentle:["Gentle","迟钝","遲鈍","Rendah","Gering","Faible","低い","Baja"],
sensitive:["Sensitive","灵敏","靈敏","Tinggi","Hoch","Forte","高い","Alta"],
mirror:["My eyes look the wrong way when tilted","倾斜时眼睛看反了","傾斜時眼睛看反了","Mataku melihat ke arah yang salah saat miring","Meine Augen schauen beim Kippen falsch herum","Mes yeux regardent du mauvais côté","傾けると目が逆を向く","Mis ojos miran al revés al inclinarme"],
motionHint:["Pat my top for a pat, shake me for a new quote or to make me dizzy, and I'll notice if I'm dropped.","拍拍我的头，摇一摇换名言（或让我头晕），掉下去我也会知道。","拍拍我的頭，搖一搖換名言（或讓我頭暈），掉下去我也會知道。","Tepuk kepalaku, goyangkan untuk kutipan baru (atau bikin aku pusing), dan aku tahu kalau jatuh.","Klopf mir auf den Kopf, schüttel mich für ein neues Zitat (oder mir wird schwindelig), und ich merke, wenn ich falle.","Tapote ma tête, secoue-moi pour une citation (ou pour me donner le tournis), et je sens si je tombe.","頭をポンポン、振ると新しい名言（か目が回る）、落ちたらわかるよ。","Dame palmaditas, agítame para otra cita (o para marearme) y notaré si me caigo."],
sounds:["Sounds","声音","聲音","Suara","Töne","Sons","サウンド","Sonidos"],
soundOn:["Sounds on","开启声音","開啟聲音","Suara aktif","Töne an","Sons activés","音を鳴らす","Sonido activado"],
volume:["Volume","音量","音量","Volume","Lautstärke","Volume","音量","Volumen"],
click:["Click when touched","触摸时发出咔嗒声","觸控時發出咔嗒聲","Bunyi klik saat disentuh","Klicken bei Berührung","Clic au toucher","タッチ音","Clic al tocar"],
chime:["Beep on the hour","整点报时","整點報時","Bunyi tiap jam","Stündlicher Piep","Bip à chaque heure","時報","Pitido cada hora"],
news:["News","新闻","新聞","Berita","Nachrichten","Infos","ニュース","Noticias"],
needsWifi:["Needs Wi-Fi.","需要 Wi-Fi。","需要 Wi-Fi。","Perlu Wi-Fi.","Braucht WLAN.","Wi-Fi nécessaire.","Wi-Fi が必要。","Necesita WiFi."],
newsSrc:["News source","新闻来源","新聞來源","Sumber berita","Nachrichtenquelle","Source","ニュースの配信元","Fuente de noticias"],
otherFeed:["Another news site (RSS link)…","其他新闻网站（RSS 链接）…","其他新聞網站（RSS 連結）…","Situs berita lain (tautan RSS)…","Andere Seite (RSS-Link)…","Autre site (lien RSS)…","ほかのサイト（RSS リンク）…","Otro sitio (enlace RSS)…"],
ntrans:["Translate headlines into my language (AI)","把标题翻译成我的语言（AI）","把標題翻譯成我的語言（AI）","Terjemahkan judul ke bahasaku (AI)","Schlagzeilen in meine Sprache übersetzen (KI)","Traduire les titres dans ma langue (IA)","見出しを私の言語に翻訳（AI）","Traducir titulares a mi idioma (IA)"],
nstyle:["AI news summary","AI 新闻摘要","AI 新聞摘要","Ringkasan berita AI","KI-Zusammenfassung","Résumé IA","AI ニュースまとめ","Resumen de IA"],
ns0:["One-sentence headline","一句话摘要","一句話摘要","Satu kalimat","Ein Satz","Une phrase","ひとことで","Una frase"],
ns1:["News-of-the-day bulletin","今日新闻播报","今日新聞播報","Buletin berita hari ini","Tagesnachrichten-Bulletin","Journal du jour","今日のニュース速報","Boletín del día"],
voice:["News reporter","新闻主播","新聞主播","Pembaca berita","Nachrichtensprecher","Présentateur","ニュースキャスター","Presentador"],
voicePh:["e.g. a grumpy cat, a Shakespearean actor","例如：一只爱抱怨的猫、莎士比亚演员","例如：一隻愛抱怨的貓、莎士比亞演員","mis. kucing yang suka menggerutu","z. B. eine grummelige Katze","ex. un chat grognon","例：不機嫌なネコ、シェイクスピア俳優","p. ej. un gato gruñón"],
voiceHint:["Who sums up the news, first on my News page. Now and then they'll link a story to your hobbies. Needs an AI key.","由谁来播报新闻，显示在新闻页第一条，偶尔还会联系到你的爱好。需要 AI 密钥。","由誰來播報新聞，顯示在新聞頁第一條，偶爾還會聯繫到你的興趣。需要 AI 金鑰。","Siapa yang merangkum berita, tampil pertama di halaman Berita, sesekali dikaitkan dengan hobimu. Perlu kunci AI.","Wer die News zusammenfasst, zuerst auf meiner News-Seite, ab und zu mit Bezug zu deinen Hobbys. Braucht einen KI-Schlüssel.","Qui résume les infos, en premier sur ma page Infos, avec parfois un clin d'œil à tes loisirs. Clé AI nécessaire.","ニュースページの最初にまとめを話す人。ときどきあなたの趣味にも触れるよ。AI キーが必要。","Quién resume las noticias, primero en mi página, y a veces las relaciona con tus aficiones. Necesita clave de IA."],
quotes:["Quotes","名言","名言","Kutipan","Zitate","Citations","名言","Citas"],
qsrc:["Pick any mix of sources. Each new quote comes from the next one you ticked.","可任意组合来源，每句新名言会轮流取自你勾选的来源。","可任意組合來源，每句新名言會輪流取自你勾選的來源。","Pilih sumber sesukamu. Setiap kutipan baru bergiliran dari sumber yang kamu centang.","Wähle beliebige Quellen. Jedes neue Zitat kommt reihum aus deiner Auswahl.","Choisis les sources que tu veux. Chaque nouvelle citation vient à tour de rôle de celles cochées.","好きな出典をいくつでも選んでください。新しい名言は選んだ出典から順番に出ます。","Elige las fuentes que quieras. Cada cita nueva sale por turno de las que marques."],
q0:["Ancient China","中国古典","中國古典","Tiongkok kuno","Altes China","Chine ancienne","古代中国","China antigua"],
q1:["Ancient Greece","古希腊","古希臘","Yunani kuno","Antikes Griechenland","Grèce antique","古代ギリシャ","Antigua Grecia"],
q2:["Modern thinkers","近现代名人","近現代名人","Tokoh modern","Moderne Denker","Penseurs modernes","近現代の人々","Pensadores modernos"],
q3:["Bible (King James Version)","圣经（英王钦定本）","聖經（英王欽定本）","Alkitab (King James Version)","Bibel (King James Version)","Bible (version King James)","聖書（欽定訳）","Biblia (versión King James)"],
q4:["Quran","古兰经","古蘭經","Al-Qur'an","Koran","Coran","クルアーン","Corán"],
q5:["Torah (the five books of Moses)","妥拉（摩西五经）","妥拉（摩西五經）","Taurat (lima kitab Musa)","Tora (die fünf Bücher Mose)","Torah (les cinq livres de Moïse)","トーラー（モーセ五書）","Torá (los cinco libros de Moisés)"],
q6:["Dhammapada (Buddha's sayings)","法句经（佛陀语录）","法句經（佛陀語錄）","Dhammapada (sabda Buddha)","Dhammapada (Worte des Buddha)","Dhammapada (paroles du Bouddha)","法句経（ブッダの言葉）","Dhammapada (palabras del Buda)"],
gender:["You are","你的性别","你的性別","Kamu","Du bist","Tu es","性別","Eres"],
g0:["Prefer not to say","不想说","不想說","Tidak ingin menyebut","Keine Angabe","Je préfère ne pas le dire","答えない","Prefiero no decirlo"],
g1:["Male","男","男","Laki-laki","Männlich","Un homme","男性","Hombre"],
g2:["Female","女","女","Perempuan","Weiblich","Une femme","女性","Mujer"],
g3:["Other / non-binary","其他 / 非二元","其他 / 非二元","Lainnya / non-biner","Divers / nicht-binär","Autre / non-binaire","その他・ノンバイナリー","Otro / no binario"],
genderHint:["Lets me word things the way that suits you, in languages where it matters. With 'Other' or no answer I keep it neutral.","让我说话更贴合你。选择“其他”或不回答时，我会用中性的说法。","讓我說話更貼合你。選擇「其他」或不回答時，我會用中性的說法。","Supaya aku bisa menyapamu dengan pas. Jika 'Lainnya' atau tidak dijawab, aku memakai bahasa netral.","Damit ich dich passend anspreche, gerade in Sprachen, wo es zählt. Bei 'Divers' oder ohne Angabe bleibe ich neutral.","Pour accorder mes phrases comme il te convient. Avec « Autre » ou sans réponse, je reste neutre.","あなたに合った言い方をするためです。「その他」や未回答なら中立的な言い方にします。","Así puedo hablarte como te va mejor, sobre todo en idiomas con género. Con 'Otro' o sin respuesta, uso un lenguaje neutro."],
calJu:["Jewish (Hebrew) calendar, festivals & Shabbat","犹太历、节日与安息日","猶太曆、節日與安息日","Kalender Yahudi (Ibrani), hari raya & Sabat","Jüdischer Kalender, Feste & Schabbat","Calendrier hébraïque, fêtes & Chabbat","ユダヤ暦・祭日・安息日","Calendario hebreo, fiestas y Shabat"],
qai:["AI should","AI 要做什么","AI 要做什麼","AI akan","Die KI soll","L'IA doit","AI の役割","La IA debe"],
qa0:["Translate the quote into my language","把名言翻译成我的语言","把名言翻譯成我的語言","Menerjemahkan kutipan ke bahasaku","Das Zitat in meine Sprache übersetzen","Traduire la citation dans ma langue","名言を私の言語に翻訳する","Traducir la cita a mi idioma"],
qa1:["Explain what it means for today","解释它对今天的意义","解釋它對今天的意義","Menjelaskan maknanya untuk hari ini","Erklären, was es heute bedeutet","Expliquer ce qu'elle veut dire aujourd'hui","今の暮らしにどう生きるか説明する","Explicar qué significa hoy"],
quoteHint:["Quotes show in their original language: Chinese, Greek, Hebrew, Arabic or Pali, with the Bible in English (King James). Then comes the translation: English is built in, and with an AI key the AI gives it in your language. If a quote is already in your language, the AI explains it instead. Shake me on the quotes page for a new one.","名言以原文显示：中文、希腊文、希伯来文、阿拉伯文或巴利文，圣经则用英文（钦定本）。接着显示译文：内置英文译文，有 AI 密钥时 AI 会译成你的语言。如果名言本来就是你的语言，AI 会改为讲解。在名言页摇一摇换一句。","名言以原文顯示：中文、希臘文、希伯來文、阿拉伯文或巴利文，聖經則用英文（欽定本）。接著顯示譯文：內建英文譯文，有 AI 金鑰時 AI 會譯成你的語言。如果名言本來就是你的語言，AI 會改為講解。在名言頁搖一搖換一句。","Kutipan tampil dalam bahasa aslinya: Tionghoa, Yunani, Ibrani, Arab atau Pali, dengan Alkitab dalam bahasa Inggris (King James). Lalu terjemahannya: bahasa Inggris sudah tersedia, dan dengan kunci AI, AI memberikannya dalam bahasamu. Jika kutipan sudah dalam bahasamu, AI menjelaskannya. Goyangkan aku di halaman kutipan untuk yang baru.","Zitate erscheinen in ihrer Originalsprache: Chinesisch, Griechisch, Hebräisch, Arabisch oder Pali, die Bibel auf Englisch (King James). Danach die Übersetzung: Englisch ist eingebaut, und mit KI-Schlüssel gibt die KI sie in deiner Sprache. Ist ein Zitat schon in deiner Sprache, erklärt die KI es stattdessen. Schüttle mich auf der Zitatseite für ein neues.","Les citations s'affichent dans leur langue d'origine : chinois, grec, hébreu, arabe ou pali, et la Bible en anglais (King James). Vient ensuite la traduction : l'anglais est intégré, et avec une clé d'IA l'IA la donne dans ta langue. Si une citation est déjà dans ta langue, l'IA l'explique plutôt. Secoue-moi sur la page citations pour en avoir une autre.","名言は原語で表示します（中国語、ギリシャ語、ヘブライ語、アラビア語、パーリ語。聖書は英語の欽定訳）。続いて訳を表示：英訳は内蔵、AI キーがあれば AI があなたの言語に訳します。名言がすでにあなたの言語なら、AI が代わりに解説します。名言ページで振ると次の名言に。","Las citas aparecen en su idioma original: chino, griego, hebreo, árabe o pali, y la Biblia en inglés (King James). Luego viene la traducción: el inglés va incluido, y con una clave de IA la IA la da en tu idioma. Si una cita ya está en tu idioma, la IA la explica en su lugar. Agítame en la página de citas para otra."],
ai:["AI","AI","AI","AI","KI","IA","AI","IA"],
aiSub:["With an AI key I share my feelings, translate quotes and headlines, and sum up the news.","有了 AI 密钥，我能说说心情、翻译名言和标题、总结新闻。","有了 AI 金鑰，我能說說心情、翻譯名言和標題、總結新聞。","Dengan kunci AI aku bisa berbagi perasaan, menerjemahkan kutipan & judul, dan merangkum berita.","Mit einem KI-Schlüssel teile ich Gefühle, übersetze Zitate und Schlagzeilen und fasse News zusammen.","Avec une clé d'IA, je partage mes humeurs, traduis citations et titres, et résume les infos.","AI キーがあれば、気持ちを話したり、名言や見出しを翻訳したり、ニュースをまとめたりできるよ。","Con una clave de IA comparto emociones, traduzco citas y titulares y resumo las noticias."],
aikey:["AI API key","AI API 密钥","AI API 金鑰","Kunci API AI","KI-API-Schlüssel","Clé API de l'IA","AI API キー","Clave API de IA"],
aimodel:["AI model","AI 模型","AI 模型","Model AI","KI-Modell","Modèle d'IA","AI モデル","Modelo de IA"],
am0:["Haiku 5.5 (fast, cheapest)","Haiku 5.5（快，最便宜）","Haiku 5.5（快，最便宜）","Haiku 5.5 (cepat, termurah)","Haiku 5.5 (schnell, am günstigsten)","Haiku 5.5 (rapide, le moins cher)","Haiku 5.5（速い・いちばん安い）","Haiku 5.5 (rápido, el más barato)"],
am1:["Sonnet 5.5 (balanced)","Sonnet 5.5（均衡）","Sonnet 5.5（均衡）","Sonnet 5.5 (seimbang)","Sonnet 5.5 (ausgewogen)","Sonnet 5.5 (équilibré)","Sonnet 5.5（バランス型）","Sonnet 5.5 (equilibrado)"],
am2:["Opus 5.5 (clever)","Opus 5.5（聪明）","Opus 5.5（聰明）","Opus 5.5 (cerdas)","Opus 5.5 (klug)","Opus 5.5 (intelligent)","Opus 5.5（かしこい）","Opus 5.5 (inteligente)"],
am3:["Fable 5.1 (cleverest, priciest)","Fable 5.1（最聪明，最贵）","Fable 5.1（最聰明，最貴）","Fable 5.1 (tercerdas, termahal)","Fable 5.1 (am klügsten, am teuersten)","Fable 5.1 (le plus intelligent, le plus cher)","Fable 5.1（いちばんかしこい・いちばん高い）","Fable 5.1 (el más inteligente y caro)"],
aieffort:["How hard the AI thinks","AI 思考力度","AI 思考力度","Seberapa keras AI berpikir","Wie gründlich die KI nachdenkt","Réflexion de l'IA","AI がどれだけ考えるか","Cuánto piensa la IA"],
ae0:["Low","低","低","Rendah","Wenig","Faible","少し","Bajo"],
ae1:["Medium","中","中","Sedang","Mittel","Moyen","ふつう","Medio"],
ae2:["High","高","高","Tinggi","Viel","Élevé","しっかり","Alto"],
ae3:["Very high","很高","很高","Sangat tinggi","Sehr viel","Très élevé","かなり","Muy alto"],
ae4:["Maximum","最高","最高","Maksimum","Maximal","Maximum","最大","Máximo"],
aiModelHint:["Thinking effort is for Claude only. Higher effort can cost more and take longer.","思考强度只适用于 Claude。强度越高，费用可能越多，也更慢。","思考強度只適用於 Claude。強度越高，費用可能越多，也更慢。","Tingkat berpikir hanya untuk Claude. Makin tinggi, bisa makin mahal dan lambat.","Denkaufwand gibt es nur bei Claude. Mehr Aufwand kann mehr kosten und länger dauern.","L'effort de réflexion ne concerne que Claude. Plus il est élevé, plus c'est cher et lent.","思考の深さは Claude だけ。深いほど料金と時間がかかることがあるよ。","El esfuerzo de razonamiento es solo para Claude. Más esfuerzo puede costar más y tardar más."],
aiProvider:["AI provider","AI 服务","AI 服務","Penyedia AI","KI-Anbieter","Service d'IA","AI サービス","Proveedor de IA"],
aikeyPh:["API key","API 密钥","API 金鑰","Kunci API","API-Schlüssel","Clé API","API キー","Clave API"],
feelEvery:["How often I share a feeling","我多久说一次心情","我多久說一次心情","Seberapa sering aku berbagi perasaan","Wie oft ich ein Gefühl teile","À quelle fréquence je partage mon humeur","気持ちを話す頻度","Cada cuánto cuento cómo me siento"],
feelHint:["A new feeling comes this often while I'm awake, and with 90 minutes or less also when the weather changes. Each one uses one AI request.","我醒着时按这个频率换一个新心情；选 90 分钟或更短时，天气变化时也会换。每次用一次 AI 请求。","我醒著時按這個頻率換一個新心情；選 90 分鐘或更短時，天氣變化時也會換。每次用一次 AI 請求。","Perasaan baru muncul sesering ini selama aku bangun, dan jika 90 menit atau kurang, juga saat cuaca berubah. Masing-masing memakai satu permintaan AI.","So oft kommt ein neues Gefühl, solange ich wach bin, und bei 90 Minuten oder weniger auch, wenn das Wetter umschlägt. Jedes braucht eine KI-Anfrage.","Une nouvelle humeur arrive à ce rythme quand je suis réveillé, et avec 90 minutes ou moins, aussi quand la météo change. Chacune utilise une requête d'IA.","起きている間はこの間隔で新しい気持ちになるよ。90 分以下なら天気が変わったときも。1 回ごとに AI リクエストを 1 回使うよ。","Un sentimiento nuevo llega con esta frecuencia mientras estoy despierto, y con 90 minutos o menos, también cuando cambia el tiempo. Cada uno usa una petición de IA."],
feelOff:["Never","从不","從不","Tidak pernah","Nie","Jamais","話さない","Nunca"],
feelMins:["Every {n} minutes","每 {n} 分钟","每 {n} 分鐘","Setiap {n} menit","Alle {n} Minuten","Toutes les {n} minutes","{n} 分ごと","Cada {n} minutos"],
feelHour:["Every hour","每小时","每小時","Setiap jam","Jede Stunde","Toutes les heures","1 時間ごと","Cada hora"],
feelHours:["Every {n} hours","每 {n} 小时","每 {n} 小時","Setiap {n} jam","Alle {n} Stunden","Toutes les {n} heures","{n} 時間ごと","Cada {n} horas"],
feelDay:["Once a day","每天一次","每天一次","Sekali sehari","Einmal am Tag","Une fois par jour","1日1回","Una vez al día"],
aiLimit:["Daily AI limit","每日 AI 上限","每日 AI 上限","Batas AI harian","KI-Tageslimit","Limite d'IA par jour","1日の AI 上限","Límite diario de IA"],
aiPerDay:["{n} requests a day","每天 {n} 次","每天 {n} 次","{n} permintaan sehari","{n} Anfragen am Tag","{n} requêtes par jour","1日 {n} 回","{n} peticiones al día"],
aiUsedTxt:["Used today: {u} of {n}","今天已用：{u} / {n}","今天已用：{u} / {n}","Terpakai hari ini: {u} dari {n}","Heute genutzt: {u} von {n}","Utilisé aujourd'hui : {u} sur {n}","今日の使用：{u} / {n}","Usadas hoy: {u} de {n}"],
aiReset:["Reset today's count","重置今天的次数","重設今天的次數","Atur ulang hitungan hari ini","Heutigen Zähler zurücksetzen","Remettre à zéro le compteur du jour","今日の回数をリセット","Reiniciar el contador de hoy"],
aiResetDone:["Done: I can use the AI again today.","好了：今天可以再用 AI 了。","好了：今天可以再用 AI 了。","Selesai: aku bisa memakai AI lagi hari ini.","Erledigt: Ich kann heute wieder die KI nutzen.","C'est fait : je peux de nouveau utiliser l'IA aujourd'hui.","リセットしたよ。今日はまた AI を使えるよ。","Listo: hoy puedo volver a usar la IA."],
aiclear:["Remove my saved key","删除已保存的密钥","刪除已儲存的金鑰","Hapus kunci tersimpan","Gespeicherten Schlüssel löschen","Supprimer la clé enregistrée","保存したキーを削除","Borrar la clave guardada"],
aiHint:["Use an API key from your selected provider: <a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a>, or <a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>. Keys are saved separately. API usage is billed by the provider.","使用你所选服务的 API 密钥：<a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a> 或 <a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>。每个服务的密钥分开保存，费用由该服务收取。","使用你所選服務的 API 金鑰：<a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a> 或 <a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>。每個服務的金鑰分開儲存，費用由該服務收取。","Pakai kunci API dari penyedia pilihanmu: <a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a>, atau <a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>. Kunci disimpan terpisah. Biaya ditagih oleh penyedia.","Nimm einen API-Schlüssel deines Anbieters: <a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a> oder <a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>. Schlüssel werden getrennt gespeichert. Die Kosten rechnet der Anbieter ab.","Utilise une clé API du service choisi : <a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a> ou <a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>. Chaque clé est gardée à part. Le service facture l\'usage.","選んだサービスの API キーを使ってね：<a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a>、<a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>。キーは別々に保存。料金は各サービスが請求するよ。","Usa una clave API del proveedor elegido: <a href=\"https://console.anthropic.com\" target=\"_blank\">Anthropic</a>, <a href=\"https://platform.openai.com/api-keys\" target=\"_blank\">OpenAI</a>, <a href=\"https://platform.moonshot.ai\" target=\"_blank\">Kimi</a> o <a href=\"https://platform.deepseek.com\" target=\"_blank\">DeepSeek</a>. Las claves se guardan por separado. El proveedor cobra el uso."],
save:["Save","保存","儲存","Simpan","Speichern","Enregistrer","保存","Guardar"],
saving:["Saving…","正在保存…","正在儲存…","Menyimpan…","Speichere…","Enregistrement…","保存中…","Guardando…"],
forget:["Forget all settings","清除所有设置","清除所有設定","Lupakan semua pengaturan","Alle Einstellungen löschen","Tout oublier","設定をすべて消す","Borrar todos los ajustes"],
saved:["Saved!","已保存！","已儲存！","Tersimpan!","Gespeichert!","Enregistré !","保存したよ！","¡Guardado!"],
later:["To change my settings later, hold me for 6 seconds and scan my code again.","以后要改设置，按住我 6 秒再扫码即可。","以後要改設定，按住我 6 秒再掃碼即可。","Untuk mengubah pengaturan nanti, tahan aku 6 detik dan pindai kodeku lagi.","Um später etwas zu ändern, halte mich 6 Sekunden und scanne den Code erneut.","Pour changer mes réglages plus tard, maintiens-moi 6 secondes et rescanne mon code.","あとで設定を変えるときは、6秒長押ししてコードをもう一度スキャンしてね。","Para cambiar mis ajustes, mantenme 6 segundos y vuelve a escanear el código."],
laterHost:["Later, open <b>{h}.local</b> on a phone on the same Wi-Fi, or hold me for 6 seconds.","以后可在同一 Wi-Fi 的手机上打开 <b>{h}.local</b>，或按住我 6 秒。","以後可在同一 Wi-Fi 的手機上開啟 <b>{h}.local</b>，或按住我 6 秒。","Nanti, buka <b>{h}.local</b> di ponsel dengan Wi-Fi yang sama, atau tahan aku 6 detik.","Später: <b>{h}.local</b> im selben WLAN öffnen oder mich 6 Sekunden halten.","Plus tard, ouvre <b>{h}.local</b> sur le même Wi-Fi, ou maintiens-moi 6 secondes.","あとで同じ Wi-Fi のスマホで <b>{h}.local</b> を開くか、6秒長押ししてね。","Luego, abre <b>{h}.local</b> en un móvil con el mismo WiFi, o mantenme 6 segundos."],
wifiErr:["I couldn't join “{s}”: {e} Please check it below.","我连不上“{s}”：{e} 请在下面检查一下。","我連不上「{s}」：{e} 請在下面檢查一下。","Aku tidak bisa terhubung ke “{s}”: {e} Cek di bawah, ya.","Ich komme nicht in „{s}“: {e} Bitte unten prüfen.","Je n'arrive pas à rejoindre « {s} » : {e} Vérifie ci-dessous.","「{s}」につながらないよ：{e} 下で確認してね。","No pude conectarme a «{s}»: {e} Revísalo abajo."],
looking:["Looking for networks…","正在搜索网络…","正在搜尋網路…","Mencari jaringan…","Suche Netzwerke…","Recherche des réseaux…","ネットワークを探し中…","Buscando redes…"],
noWifi:["No Wi-Fi (I'll work offline)","不用 Wi-Fi（离线使用）","不用 Wi-Fi（離線使用）","Tanpa Wi-Fi (bekerja offline)","Kein WLAN (offline)","Pas de Wi-Fi (hors ligne)","Wi-Fi なし（オフライン）","Sin WiFi (sin conexión)"],
noNets:["No networks found: no Wi-Fi, or tap Other…","没找到网络：不用 Wi-Fi，或选“其他…”","沒找到網路：不用 Wi-Fi，或選「其他…」","Tidak ada jaringan: tanpa Wi-Fi, atau pilih Lainnya…","Keine Netzwerke: kein WLAN, oder Andere…","Aucun réseau : pas de Wi-Fi, ou Autre…","見つからない：Wi-Fi なし、または「その他…」","Sin redes: sin WiFi, o elige Otra…"],
none:["None","无","無","Tidak ada","Keins","Aucun","なし","Ninguna"],
other:["Other…","其他…","其他…","Lainnya…","Andere…","Autre…","その他…","Otra…"],
savedKeep:["Saved (leave empty to keep it)","已保存（留空则保留）","已儲存（留空則保留）","Tersimpan (kosongkan untuk tetap)","Gespeichert (leer lassen zum Behalten)","Enregistré (laisser vide pour garder)","保存済み（空欄のままで保持）","Guardada (déjala vacía para mantenerla)"],
doneJoin:["I'm restarting now and will join “{s}”. You can close this page.","我正在重启，并会连接“{s}”。你可以关闭这个页面了。","我正在重新啟動，並會連線「{s}」。你可以關閉這個頁面了。","Aku memulai ulang dan akan terhubung ke “{s}”. Halaman ini boleh ditutup.","Ich starte neu und verbinde mich mit „{s}“. Du kannst die Seite schließen.","Je redémarre et je rejoins « {s} ». Tu peux fermer cette page.","再起動して「{s}」につなぐよ。このページは閉じて大丈夫。","Me reinicio y me conectaré a «{s}». Puedes cerrar esta página."],
doneOff:["Done! I'm restarting now, with my clock set from your phone. You can close this page.","好了！我正在重启，时钟已按你的手机校准。你可以关闭这个页面了。","好了！我正在重新啟動，時鐘已按你的手機校準。你可以關閉這個頁面了。","Selesai! Aku memulai ulang dengan jam dari ponselmu. Halaman ini boleh ditutup.","Fertig! Ich starte neu, mit der Uhrzeit deines Handys. Du kannst die Seite schließen.","C'est fait ! Je redémarre, à l'heure de ton téléphone. Tu peux fermer cette page.","できた！スマホの時刻で再起動するよ。このページは閉じて大丈夫。","¡Listo! Me reinicio con la hora de tu móvil. Puedes cerrar esta página."],
saveFail:["Sorry, I couldn't save that. Please try again.","抱歉，保存失败，请再试一次。","抱歉，儲存失敗，請再試一次。","Maaf, gagal menyimpan. Coba lagi.","Das hat leider nicht geklappt. Bitte noch einmal.","Désolé, l'enregistrement a échoué. Réessaie.","ごめんね、保存できなかった。もう一度試してね。","Lo siento, no pude guardar. Inténtalo de nuevo."],
forgetQ:["Forget all my settings and start fresh?","清除所有设置并重新开始？","清除所有設定並重新開始？","Lupakan semua pengaturan dan mulai dari awal?","Alle Einstellungen löschen und neu anfangen?","Tout oublier et repartir de zéro ?","設定をすべて消して最初からにする？","¿Borrar todos mis ajustes y empezar de cero?"],
forgotten:["All forgotten","已全部清除","已全部清除","Semua dilupakan","Alles gelöscht","Tout est oublié","すべて消したよ","Todo borrado"],
factory:["I'm restarting with my factory settings.","我正在以出厂设置重启。","我正在以出廠設定重新啟動。","Aku memulai ulang dengan pengaturan pabrik.","Ich starte mit Werkseinstellungen neu.","Je redémarre avec mes réglages d'usine.","工場出荷時の設定で再起動するよ。","Me reinicio con los ajustes de fábrica."],
anywhere:["Anywhere","其他地区","其他地區","Di mana saja","Anderswo","Ailleurs","その他","Otro lugar"],
asCalc:["as calculated","按推算","按推算","sesuai hitungan","wie berechnet","comme calculé","計算どおり","según el cálculo"],
dayU:["day","天","天","hari","Tag","jour","日","día"],
v0:["Alan Partridge (comedy character)","艾伦·帕特里奇（喜剧角色）","艾倫·帕特里奇（喜劇角色）","Alan Partridge (tokoh komedi)","Alan Partridge (Comedy-Figur)","Alan Partridge (personnage comique)","アラン・パートリッジ（コメディの人物）","Alan Partridge (personaje cómico)"],
v1:["Sherlock Holmes","夏洛克·福尔摩斯","夏洛克·福爾摩斯","Sherlock Holmes","Sherlock Holmes","Sherlock Holmes","シャーロック・ホームズ","Sherlock Holmes"],
v2:["A cheerful pirate captain","快乐的海盗船长","快樂的海盜船長","Kapten bajak laut yang ceria","Ein fröhlicher Piratenkapitän","Un joyeux capitaine pirate","陽気な海賊船長","Un alegre capitán pirata"],
v3:["Wildlife documentary narrator","野生动物纪录片旁白","野生動物紀錄片旁白","Narator dokumenter satwa liar","Tierfilm-Erzähler","Narrateur de documentaire animalier","動物ドキュメンタリーのナレーター","Narrador de documentales de naturaleza"],
v4:["Excitable sports commentator","激动的体育解说员","激動的體育解說員","Komentator olahraga yang heboh","Aufgeregter Sportkommentator","Commentateur sportif survolté","興奮気味のスポーツ実況","Comentarista deportivo emocionado"],
v5:["Calm newsreader","沉稳的新闻主播","沉穩的新聞主播","Pembaca berita yang tenang","Ruhiger Nachrichtensprecher","Présentateur calme","落ち着いたアナウンサー","Presentador tranquilo"],
v6:["Your own reporter…","自定义主播…","自訂主播…","Pembaca berita pilihanmu…","Eigene Figur…","Ton propre présentateur…","自分で決める…","Tu propio presentador…"]
};
const t=k=>(I18N[k]||[k])[L]||I18N[k][0];
const LOCALE=["en","zh-Hans","zh-Hant","id","de","fr","ja","es"];
const LANGS=[[0,"English"],[1,"简体中文"],[2,"繁體中文"],[3,"Bahasa Indonesia"],[4,"Deutsch"],[5,"Français"],[6,"日本語"],[7,"Español"]];
// [RSS link, credit shown on screen, name in the list, language group]
const NEWS=[["https://www.channelnewsasia.com/api/v1/rss-outbound-feed?_format=xml&category=6311","CNA","CNA (Channel NewsAsia)",0],
["https://www.scmp.com/rss/91/feed","SCMP","SCMP · Latest news",0],
["https://www.scmp.com/rss/2/feed","SCMP","SCMP · Hong Kong",0],
["https://www.scmp.com/rss/4/feed","SCMP","SCMP · China",0],
["https://feeds.bbci.co.uk/news/rss.xml","BBC News","BBC News · Top stories",0],
["https://feeds.bbci.co.uk/news/world/rss.xml","BBC News","BBC News · World",0],
["https://feeds.bbci.co.uk/news/business/rss.xml","BBC News","BBC News · Business",0],
["https://feeds.bbci.co.uk/news/technology/rss.xml","BBC News","BBC News · Technology",0],
["https://www.aljazeera.com/xml/rss/all.xml","Al Jazeera","Al Jazeera",0],
["https://feeds.npr.org/1001/rss.xml","NPR","NPR News",0],
["https://feeds.bbci.co.uk/zhongwen/simp/rss.xml","BBC中文","BBC 中文（简体）",1],
["https://www.chinanews.com.cn/rss/scroll-news.xml","中新网","中国新闻网 · 即时新闻（中国）",1],
["https://www.chinanews.com.cn/rss/china.xml","中新网","中国新闻网 · 国内（中国）",1],
["https://www.8world.com/api/v1/rss-outbound-feed?_format=xml","8视界","8视界新闻网（新加坡）",1],
["https://www.malaysiakini.com/rss/zh/news.rss","当今大马","当今大马（马来西亚）",1],
["https://feeds.bbci.co.uk/zhongwen/trad/rss.xml","BBC中文","BBC 中文（繁體）",2],
["https://feeds.feedburner.com/rsscna/politics","中央社","中央社 · 政治（台灣）",2],
["https://feeds.feedburner.com/rsscna/intworld","中央社","中央社 · 國際（台灣）",2],
["https://feeds.feedburner.com/rsscna/mainland","中央社","中央社 · 兩岸（台灣）",2],
["https://news.ltn.com.tw/rss/all.xml","自由時報","自由時報 · 即時（台灣）",2],
["https://news.pts.org.tw/xml/newsfeed.xml","公視新聞","公視新聞網（台灣）",2],
["https://rthk9.rthk.hk/rthk/news/rss/c_expressnews_clocal.xml","香港電台","香港電台 · 本地（香港）",2],
["https://rthk9.rthk.hk/rthk/news/rss/c_expressnews_cinternational.xml","香港電台","香港電台 · 國際（香港）",2],
["https://feeds.bbci.co.uk/indonesia/rss.xml","BBC Indonesia","BBC News Indonesia",3],
["https://rss.kompas.com/api/feed/social?apikey=bc58c81819dff4b8d5c53540a2fc7ffd83e6314a","Kompas","Kompas.com",3],
["https://www.antaranews.com/rss/terkini.xml","ANTARA","ANTARA · Terkini",3],
["https://www.cnnindonesia.com/nasional/rss","CNN Indonesia","CNN Indonesia · Nasional",3],
["https://www.cnbcindonesia.com/news/rss","CNBC Indonesia","CNBC Indonesia · News",3],
["https://news.detik.com/berita/rss","detikNews","detikNews · Berita",3],
["https://www.republika.co.id/rss/nasional/","Republika","Republika · Nasional",3],
["https://feed.liputan6.com/rss/news","Liputan6","Liputan6 · News",3],
["https://www.tagesschau.de/index~rss2.xml","tagesschau","tagesschau.de",4],
["https://rss.dw.com/xml/rss-de-all","DW","Deutsche Welle (Deutsch)",4],
["https://www.lemonde.fr/rss/une.xml","Le Monde","Le Monde · À la une",5],
["https://www.france24.com/fr/rss","France 24","France 24 (français)",5],
["https://www3.nhk.or.jp/rss/news/cat0.xml","NHK","NHK ニュース",6],
["https://feeds.bbci.co.uk/japanese/rss.xml","BBC","BBC ニュース日本語",6],
["https://feeds.bbci.co.uk/mundo/rss.xml","BBC Mundo","BBC News Mundo",7],
["https://www.france24.com/es/rss","France 24","France 24 (español)",7]];
// The same order as the methods in astro.h
const METHODS=["Muslim World League","ISNA (North America)","Egyptian Authority","Umm al-Qura (Makkah)","Karachi","MUIS (Singapore)","Kemenag (Indonesia)","JAKIM (Malaysia)","Diyanet (Turkey)","UOIF (France)","Gulf / Dubai"];
const CC=["SG","MY","ID","BN","HK","CN","TW","JP","KR","IN","PK","BD","PH","TH","VN","AU","NZ","SA","AE","QA","KW","OM","BH","EG","TR","GB","FR","DE","ES","MX","US","CA",""];
function fill(sel,list,val){sel.innerHTML="";list.forEach(([v,x])=>{const o=document.createElement("option");o.value=v;o.textContent=x;sel.appendChild(o)});if(val!==undefined)sel.value=String(val)}
function keep(id,f){const v=$(id).value;f();if(v!=="")$(id).value=v}
function hr(h){return String(h).padStart(2,"0")+":00"}
function region(c){if(!c)return t("anywhere");try{return new Intl.DisplayNames([LOCALE[L]],{type:"region"}).of(c)}catch(e){return c}}
function newsList(){const sel=$("news"),v=sel.value||info.news;sel.innerHTML="";
const order=[L].concat([0,1,2,3,4,5,6,7].filter(g=>g!=L));
order.forEach(g=>{const items=NEWS.filter(n=>n[3]==g);if(!items.length)return;const og=document.createElement("optgroup");og.label=LANGS[g][1];
items.forEach(n=>{const o=document.createElement("option");o.value=n[0];o.textContent=n[2];og.appendChild(o)});sel.appendChild(og)});
const o=document.createElement("option");o.value="__o";o.textContent=t("otherFeed");sel.appendChild(o);
const known=NEWS.find(n=>n[0]==v);sel.value=known?v:(v?"__o":NEWS.find(n=>n[3]==L)[0])}
// Put every label in the chosen language
function relabel(){L=+$("lang").value||0;document.documentElement.lang=LOCALE[L];
document.querySelectorAll("[data-t]").forEach(e=>e.textContent=t(e.dataset.t));
document.querySelectorAll("[data-h]").forEach(e=>e.innerHTML=t(e.dataset.h));
document.querySelectorAll("[data-p]").forEach(e=>e.placeholder=t(e.dataset.p));aiKeyPh();
keep("cc",()=>fill($("cc"),CC.map(c=>[c,region(c)]).sort((a,b)=>a[0]?b[0]?a[1].localeCompare(b[1]):-1:1)));
keep("pmethod",()=>fill($("pmethod"),[[255,t("auto")]].concat(METHODS.map((m,i)=>[i,m]))));
keep("hijriAdj",()=>fill($("hijriAdj"),[[-2,"-2 "+t("dayU")],[-1,"-1 "+t("dayU")],[0,"0 ("+t("asCalc")+")"],[1,"+1 "+t("dayU")],[2,"+2 "+t("dayU")]]));
keep("fmin",()=>fill($("fmin"),[15,20,25,30,40,45,50,60,90].map(m=>[m,m+" "+t("min")])));
keep("bmin",()=>fill($("bmin"),[3,5,10,15,20].map(m=>[m,m+" "+t("min")])));
keep("nstyle",()=>fill($("nstyle"),[[0,t("ns0")],[1,t("ns1")]]));
keep("gender",()=>fill($("gender"),[0,1,2,3].map(i=>[i,t("g"+i)])));
keep("qai",()=>fill($("qai"),[[0,t("qa0")],[1,t("qa1")]]));
aiModelFill($("aimodel").value);
keep("feelmin",()=>fill($("feelmin"),[[30,t("feelMins").replace("{n}",30)],[60,t("feelHour")],[90,t("feelMins").replace("{n}",90)],[180,t("feelHours").replace("{n}",3)],[360,t("feelHours").replace("{n}",6)],[1440,t("feelDay")],[0,t("feelOff")]]));
keep("ailimit",()=>fill($("ailimit"),[25,50,100,200,500].map(n=>[n,t("aiPerDay").replace("{n}",n)])));
keep("aieffort",()=>fill($("aieffort"),[0,1,2,3,4].map(i=>[i,t("ae"+i)])));
keep("voice",()=>fill($("voice"),[0,1,2,3,4,5,6].map(i=>[i,t("v"+i)])));
newsList();
$("mpu").textContent=info.mpu?t("mpuYes"):t("mpuNo");
if(!info.mpu&&info.i2c){const s=document.createElement("small");s.style.display="block";s.textContent="I2C: "+info.i2c+" · ID "+info.mpuId+" · "+info.build;$("mpu").append(s)}
if(info.wifiErr&&info.ssid)$("hello").textContent=t("wifiErr").replace("{s}",info.ssid).replace("{e}",info.wifiErr);
if(info.host&&info.ssid)$("later").innerHTML=t("laterHost").replace("{h}",info.host);
["pass","pass2"].forEach((id,i)=>{if(i?info.hasPass2:info.hasPass)$(id).placeholder=t("savedKeep")});

if(info.methodNow)$("pmethodNow").textContent=t("nowUsing")+info.methodNow;
if($("ssid").options.length)scanFill();sync()}
function sync(){$("ssidOther").style.display=$("ssid").value=="__o"?"block":"none";
$("passbox").style.display=$("ssid").value?"block":"none";
$("ssid2Other").style.display=$("ssid2").value=="__o"?"block":"none";
$("pass2box").style.display=$("ssid2").value?"block":"none";
$("newsUrl").style.display=$("news").value=="__o"?"block":"none";
$("voiceOwn").style.display=$("voice").value=="6"?"block":"none";
$("isbox").style.display=$("calIs").checked?"block":"none";
$("famous").style.display=$("qm7").checked?"block":"none";
const auto=$("autoloc").checked;$("locbox").style.display=auto?"none":"block";$("autohint").style.display=auto?"block":"none"}
function net(id){return $(id).value=="__o"?$(id+"Other").value.trim():$(id).value}
let names=null,scanPrev=null;
function scanFill(){if(names===null)return;const pick=(id,saved,first)=>{const cur=id=="ssid"&&scanPrev!==null?scanPrev:$(id).value,other=$(id+"Other").value;
const l=first.concat(names.map(n=>[n,n]),[["__o",t("other")]]);fill($(id),l);
if(cur&&cur!=="__o"&&names.includes(cur))$(id).value=cur;else if(cur==="__o"||(saved&&!names.includes(saved))){$(id).value="__o";$(id+"Other").value=other||saved}
else $(id).value=saved&&names.includes(saved)?saved:""};
pick("ssid",info.ssid,[["",names.length?t("noWifi"):t("noNets")]]);pick("ssid2",info.ssid2,[["",t("none")]]);scanPrev=null;sync()}
async function scan(){if(scanPrev===null)scanPrev=$("ssid").value;$("ssid").innerHTML='<option value="__busy">'+t("looking")+"</option>";let nets=[];
try{nets=await(await fetch("/scan")).json()}catch(e){}
names=[...new Set(nets.map(n=>n.ssid).filter(Boolean))];scanFill()}
// this year's summer-time changes as a POSIX TZ rule, so Momowo keeps them with no network ("" if none)
function tzRule(){try{const y=new Date().getFullYear(),e=t=>-new Date(t).getTimezoneOffset(),ch=[];
let t=Date.UTC(y,0,1),p=e(t);
for(;t<Date.UTC(y+1,0,1);t+=36e5){const o=e(t);if(o==p)continue;let a=t-36e5;
while(t-a>6e4){const m=a+Math.max(1,Math.floor((t-a)/12e4))*6e4;if(e(m)==o)t=m;else a=m}ch.push([t,p,o]);p=o}
if(ch.length!=2||ch[0][1]!=ch[1][2])return"";
const f=m=>{const a=Math.abs(m);return(m>0?"-":"")+Math.floor(a/60)+(a%60?":"+String(a%60).padStart(2,"0"):"")},
std=Math.min(ch[0][1],ch[0][2]),dst=Math.max(ch[0][1],ch[0][2]),
r=c=>{const d=new Date(c[0]+c[1]*6e4),n=d.getUTCDate(),last=new Date(Date.UTC(d.getUTCFullYear(),d.getUTCMonth()+1,0)).getUTCDate();
return",M"+(d.getUTCMonth()+1)+"."+(n+7>last?5:Math.ceil(n/7))+"."+d.getUTCDay()+"/"+d.getUTCHours()+(d.getUTCMinutes()?":"+String(d.getUTCMinutes()).padStart(2,"0"):"")};
return"STD"+f(std)+"DST"+f(dst)+r(ch.find(c=>c[2]==dst))+r(ch.find(c=>c[2]==std))}catch(e){return""}}
async function load(){try{const r=await fetch("/time",{method:"POST",headers:{"Content-Type":"application/json"},
body:JSON.stringify({t:Math.floor(Date.now()/1000),off:-new Date().getTimezoneOffset()*60,tz:tzRule()})});if(r.ok)$("clk").style.display="block"}catch(e){}
try{info=await(await fetch("/info")).json()}catch(e){}
let guess=0;const nl=(navigator.language||"en").toLowerCase();
if(nl.startsWith("zh"))guess=/tw|hk|mo|hant/.test(nl)?2:1;else{const i=["en","","","id","de","fr","ja","es"].indexOf(nl.slice(0,2));if(i>0)guess=i}
fill($("lang"),LANGS,info.lang??guess);
const hs=[];for(let h=18;h<24;h++)hs.push([h,hr(h)]);for(let h=0;h<4;h++)hs.push([h,hr(h)]);fill($("sleep"),hs,info.sleep??22);
const ws=[];for(let h=4;h<12;h++)ws.push([h,hr(h)]);fill($("wake"),ws,info.wake??7);
relabel();
$("cc").value=info.cc??"SG";
$("name").value=info.name||"";$("hobbies").value=info.hobbies||"";$("suburb").value=info.suburb||"";$("autoloc").checked=!info.suburb;$("h24").checked=info.h24??true;
$("voice").value=info.voice??0;$("voiceOwn").value=info.voicetxt||"";$("sound").checked=info.sound??true;$("vol").value=info.vol??6;$("click").checked=info.click??true;$("chime").checked=info.chime??true;
const cals=info.cals??1;$("calCn").checked=cals&1;$("calIs").checked=cals&2;$("calBu").checked=cals&4;$("calHi").checked=cals&8;$("calJu").checked=cals&16;
$("pmethod").value=info.pmethod??255;$("hanafi").checked=!!info.hanafi;$("premind").checked=info.premind??true;$("hijriAdj").value=info.hijriAdj??0;
$("fmin").value=info.fmin??25;$("bmin").value=info.bmin??5;
$("motion").checked=info.motion??true;$("msens").value=info.msens??2;$("mmirror").checked=!!info.mmirror;
$("aiprovider").value=info.aiprovider??0;aiProviderChanged();
$("ntrans").checked=!!info.ntrans;$("nstyle").value=info.nstyle??0;const qm=info.qmask??1;for(let i=0;i<8;i++)$("qm"+i).checked=qm>>i&1;$("famous").value=info.famous||"";$("gender").value=info.gender??0;$("qai").value=info.qai??0;$("ailimit").value=info.ailimit??100;$("feelmin").value=info.feelmin??90;aiUsedShow(info.aiUsed??0);$("aieffort").value=info.aieffort??0;
const known=NEWS.find(n=>n[0]==info.news);if(info.news){$("news").value=known?info.news:"__o";if(!known)$("newsUrl").value=info.news}

sync();scan()}
async function save(){const ssid=net("ssid");
const ssid2=net("ssid2");const d={aiprovider:+$("aiprovider").value,...aiModelSettings(),ssid,ssid2,lang:+$("lang").value,name:$("name").value.trim(),hobbies:$("hobbies").value.trim(),suburb:$("autoloc").checked?"":$("suburb").value.trim(),cc:$("cc").value,
h24:$("h24").checked,sleep:+$("sleep").value,wake:+$("wake").value,sound:$("sound").checked,vol:+$("vol").value,voice:$("voice").value=="6"&&!$("voiceOwn").value.trim()?0:+$("voice").value,voicetxt:$("voiceOwn").value.trim(),
click:$("click").checked,chime:$("chime").checked,
cals:($("calCn").checked?1:0)|($("calIs").checked?2:0)|($("calBu").checked?4:0)|($("calHi").checked?8:0)|($("calJu").checked?16:0),
pmethod:+$("pmethod").value,hanafi:$("hanafi").checked,premind:$("premind").checked,hijriAdj:+$("hijriAdj").value,
fmin:+$("fmin").value,bmin:+$("bmin").value,motion:$("motion").checked,msens:+$("msens").value,mmirror:$("mmirror").checked,
ntrans:$("ntrans").checked,nstyle:+$("nstyle").value,qmask:[0,1,2,3,4,5,6,7].reduce((m,i)=>m|($("qm"+i).checked?1<<i:0),0)||1,famous:$("famous").value.trim(),gender:+$("gender").value,qai:+$("qai").value,ailimit:+$("ailimit").value,feelmin:+$("feelmin").value,aieffort:+$("aieffort").value};
if($("pass").value||ssid!==(info.ssid||""))d.pass=$("pass").value;
if($("pass2").value||ssid2!==(info.ssid2||""))d.pass2=$("pass2").value;
if(ssid=="__busy"){delete d.ssid;delete d.pass}
if($("news").value=="__o"){const u=$("newsUrl").value.trim();if(u){d.news=u;try{d.credit=new URL(u).hostname.replace(/^www\./,"")}catch(e){d.credit="News"}}}
else{const n=NEWS.find(n=>n[0]==$("news").value);d.news=n[0];d.credit=n[1]}
if($("aikey").value.trim())d.aikey=$("aikey").value.trim();if($("aiclear").checked)d.aiclear=true;
const b=$("save");b.disabled=true;b.textContent=t("saving");
try{const r=await fetch("/save",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(d)});if(!r.ok)throw 0;
$("form").style.display="none";$("doneText").textContent=ssid?t("doneJoin").replace("{s}",ssid):t("doneOff");$("done").style.display="block"}
catch(e){alert(t("saveFail"));b.disabled=false;b.textContent=t("save")}}
// IDs are stored using the existing provider-specific preferences.
const AI_MODEL_OPTIONS=[
["claude-haiku-5-5","claude-sonnet-5-5","claude-opus-5-5","claude-fable-5-1"],
["gpt-4.1-mini","gpt-4.1-nano","gpt-4.1"],
["kimi-k2.6"],
["deepseek-flash","deepseek-v4-pro"]
];
const AI_MODEL_LABELS=[[],["GPT-4.1 mini","GPT-4.1 nano","GPT-4.1"],["Kimi K2.6"],["DeepSeek Flash","DeepSeek V4 Pro"]];
const aiModelSelections={};
function aiModelFill(selected){
const p=+$("aiprovider").value||0;
const saved=info.aiModels?.[p]||(p===0?AI_MODEL_OPTIONS[0][info.aimodel??0]:"");
const value=selected||saved||AI_MODEL_OPTIONS[p][0];
const options=AI_MODEL_OPTIONS[p].map((id,i)=>[id,p===0?t("am"+i):AI_MODEL_LABELS[p][i]]);
// Preserve an existing model override when upgrading older firmware.
if(saved&&!options.some(([id])=>id===saved))options.push([saved,saved]);
fill($("aimodel"),options,options.some(([id])=>id===value)?value:options[0][0]);
}
function aiModelSettings(){
const p=+$("aiprovider").value,model=$("aimodel").value;
const ix=AI_MODEL_OPTIONS[0].indexOf(model);
// Keep Claude's original preset behavior, including its reporter lookup fallback.
return p===0&&ix>=0?{aimodel:ix,aicustom:""}:{aicustom:model};
}
function aiKeyPh(){const p=+$("aiprovider").value;const has=info.aiKeys?!!info.aiKeys[p]:(p===0&&info.hasKey);$("aikey").placeholder=has?t("savedKeep"):t("aikeyPh")}
function aiProviderChanged(){
const p=+$("aiprovider").value;
$("aikey").value="";$("aiclear").checked=false;
const has=info.aiKeys?!!info.aiKeys[p]:(p===0&&info.hasKey);
aiKeyPh();
$("aiclearRow").style.display=has?"flex":"none";
aiModelFill(aiModelSelections[p]);
$("aiEffortRow").style.display=p===0?"block":"none";
$("aieffort").disabled=p!==0;
}
function aiUsedShow(u){$("aiUsed").textContent=t("aiUsedTxt").replace("{u}",u).replace("{n}",info.ailimit??100)}
async function aiReset(){try{const r=await fetch("/aireset",{method:"POST",headers:{"Content-Type":"application/json"},body:"{}"});if(!r.ok)throw 0;aiUsedShow(0);$("aiReset").textContent=t("aiResetDone");$("aiReset").disabled=true}catch(e){alert(t("saveFail"))}}
async function forget(){if(!confirm(t("forgetQ")))return;
try{await fetch("/reset",{method:"POST",headers:{"Content-Type":"application/json"},body:"{}"})}catch(e){}
$("form").style.display="none";$("doneTitle").textContent=t("forgotten");
$("doneText").textContent=t("factory");$("later").style.display="none";$("done").style.display="block"}
load();
</script></body></html>
)PORTAL";
