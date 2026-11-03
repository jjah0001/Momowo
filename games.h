// Momowo mini-games page: served by Momowo to your phone at /games. You play here; Momowo plays
// along on its own screen (its moves, its score and its reactions). Same rules as portal.h: kept
// out of the .ino because of the JavaScript, labels in eight languages in the I18N table below.
#pragma once
#include <pgmspace.h>

static const char GAMES_HTML[] PROGMEM = R"GAMES(<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#ff8fab"><title>Momowo</title><style>
:root{--pink:#ff8fab;--deep:#e86a8e;--ink:#3d2c3a;--soft:#fff6f8;--line:#f4d6de}
*{box-sizing:border-box}body{margin:0;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Noto Sans",sans-serif;background:var(--soft);color:var(--ink)}
header{background:var(--pink);color:#fff;text-align:center;padding:22px 16px 18px;border-radius:0 0 28px 28px}
header h1{margin:0;font-size:26px}header p{margin:6px 0 0;font-size:14px;opacity:.95}
main{max-width:480px;margin:0 auto;padding:6px 14px 30px}
.tabs{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:14px 0}
.tabs button{font-size:15px;padding:12px 8px;border:1px solid var(--line);border-radius:14px;background:#fff;color:var(--ink)}
.tabs button.on{background:var(--deep);color:#fff;border-color:var(--deep)}
.card{background:#fff;border:1px solid var(--line);border-radius:20px;padding:16px;margin:14px 0;text-align:center}
.card h2{font-size:18px;margin:0 0 6px}.hint{font-size:14px;opacity:.7;margin:4px 0 12px;line-height:1.4}
.big{display:flex;gap:10px;justify-content:center}.big button{font-size:40px;width:84px;height:84px;border-radius:20px;border:1px solid var(--line);background:var(--soft)}
button{font-family:inherit}button:disabled{opacity:.45}
.act{width:100%;font-size:16px;padding:13px;border:0;border-radius:14px;background:var(--deep);color:#fff;font-weight:700;margin-top:12px}
.ghost{background:none;color:var(--ink);border:1px solid var(--line);font-weight:500}
.status{font-size:18px;font-weight:700;min-height:26px;margin:14px 0 4px}.score{font-size:14px;opacity:.7}
.grid{display:grid;grid-template-columns:repeat(3,76px);gap:6px;justify-content:center}
.grid button{height:76px;font-size:38px;border-radius:14px;border:1px solid var(--line);background:var(--soft);color:var(--ink)}
.pad{display:grid;grid-template-columns:repeat(3,76px);gap:6px;justify-content:center}
.pad button{height:76px;font-size:34px;border-radius:14px;border:1px solid var(--line);background:var(--soft)}
.pad button.hit{background:var(--pink)}
input{width:100%;font-size:22px;padding:12px;border:1px solid var(--line);border-radius:12px;text-align:center}
a{color:var(--deep)}.foot{text-align:center;margin:20px 0;font-size:14px}
</style></head><body>
<header><h1>🎮 <span data-t="title"></span></h1><p data-t="sub"></p></header>
<main>
<div class="tabs"><button id="t0" onclick="pick(0)" data-t="rps"></button><button id="t1" onclick="pick(1)" data-t="ttt"></button>
<button id="t2" onclick="pick(2)" data-t="guess"></button><button id="t3" onclick="pick(3)" data-t="simon"></button>
<button id="t4" onclick="pick(4)" data-t="roll"></button><button id="t5" onclick="pick(5)" data-t="riddle"></button>
<button id="t6" onclick="pick(6)" data-t="quest"></button><button id="t7" onclick="pick(7)" data-t="tapG"></button></div>

<div class="card" id="p0"><h2 data-t="rps"></h2><p class="hint" data-t="rpsHelp"></p>
<div class="big"><button onclick="rps(0)">✊</button><button onclick="rps(1)">✋</button><button onclick="rps(2)">✌️</button></div>
<div class="status" id="rpsSt"></div><div class="score" id="rpsSc"></div></div>

<div class="card" id="p1"><h2 data-t="ttt"></h2><p class="hint" data-t="tttHelp"></p>
<div class="grid" id="board"></div><div class="status" id="tttSt"></div>
<button class="act" onclick="ttt({a:'new'})" data-t="newGame"></button></div>

<div class="card" id="p2"><h2 data-t="guess"></h2><p class="hint" data-t="guessHelp"></p>
<input id="num" type="number" min="1" max="100" inputmode="numeric" onkeydown="if(event.key=='Enter')guess()">
<button class="act" id="gBtn" onclick="guess()" data-t="guessBtn"></button>
<div class="status" id="gSt"></div>
<button class="act ghost" onclick="guessNew()" data-t="newNum"></button></div>

<div class="card" id="p3"><h2 data-t="simon"></h2><p class="hint" data-t="simonHelp"></p>
<div class="pad"><span></span><button id="d0" onclick="tap(0)">⬆️</button><span></span>
<button id="d3" onclick="tap(3)">⬅️</button><span></span><button id="d1" onclick="tap(1)">➡️</button>
<span></span><button id="d2" onclick="tap(2)">⬇️</button><span></span></div>
<div class="status" id="sSt"></div><div class="score" id="sSc"></div>
<button class="act" id="sGo" onclick="simonStart()" data-t="start"></button></div>

<div class="card" id="p4"><h2 data-t="roll"></h2><p class="hint" data-t="rollHelp"></p>
<div class="status" id="rSt"></div><div class="score" id="rSc"></div>
<button class="act" id="rGo" onclick="rollStart()" data-t="start"></button></div>

<div class="card" id="p5"><h2 data-t="riddle"></h2><p class="hint" data-t="riddleHelp"></p>
<p id="qText" style="font-size:18px;line-height:1.45;margin:10px 0"></p><p class="hint" id="qHint"></p>
<div id="qPlay" style="display:none"><input id="qIn" type="text" autocomplete="off" onkeydown="if(event.key=='Enter')rGuess()">
<button class="act" id="qBtn" onclick="rGuess()" data-t="guessBtn"></button>
<div style="display:flex;gap:8px"><button class="act ghost" onclick="rAct('hint')" data-t="hintBtn"></button>
<button class="act ghost" onclick="rAct('reveal')" data-t="giveUp"></button></div></div>
<div class="status" id="qSt"></div>
<button class="act" id="qNew" onclick="rNew()" data-t="newRiddle"></button></div>

<div class="card" id="p6"><h2 data-t="quest"></h2><p class="hint" data-t="questHelp"></p>
<p id="xTitle" style="font-size:18px;font-weight:700;margin:8px 0 0"></p>
<p id="xText" style="font-size:18px;line-height:1.45;margin:8px 0"></p>
<div class="status" id="xSt"></div><div class="score" id="xSc"></div>
<button class="act" id="xGo" onclick="questStart()" data-t="start"></button></div>

<div class="card" id="p7"><h2 data-t="tapG"></h2><p class="hint" data-t="tapHelp"></p>
<div class="status" id="kSt"></div><div class="score" id="kSc"></div>
<button class="act" id="kGo" onclick="tapStart()" data-t="start"></button></div>

<p class="foot"><a href="/" data-t="settings"></a></p>
</main><script>
const $=i=>document.getElementById(i);let L=0;
// en, zh-Hans, zh-Hant, id, de, fr, ja, es: the same order as Momowo's Language setting
const I18N={
title:["Play with Momowo","和摸摸我玩","和摸摸我玩","Main dengan Momowo","Spiel mit Momowo","Joue avec Momowo","Momowo とあそぼう","Juega con Momowo"],
sub:["You play here. I play along on my screen!","你在这里玩，我在屏幕上陪你！","你在這裡玩，我在螢幕上陪你！","Kamu main di sini, aku ikut main di layarku!","Du spielst hier, ich spiele auf meinem Bildschirm mit!","Tu joues ici, je joue avec toi sur mon écran !","ここで遊んでね。わたしは画面でいっしょに遊ぶよ！","Tú juegas aquí y yo te acompaño en mi pantalla."],
rps:["✊ Rock paper scissors","✊ 石头剪刀布","✊ 剪刀石頭布","✊ Gunting batu kertas","✊ Schere Stein Papier","✊ Pierre feuille ciseaux","✊ じゃんけん","✊ Piedra papel tijera"],
ttt:["⭕ Tic-tac-toe","⭕ 井字棋","⭕ 井字遊戲","⭕ Tic-tac-toe","⭕ Tic-Tac-Toe","⭕ Morpion","⭕ 三目並べ","⭕ Tres en raya"],
guess:["🔢 Guess my number","🔢 猜数字","🔢 猜數字","🔢 Tebak angkaku","🔢 Rate meine Zahl","🔢 Devine mon nombre","🔢 数当て","🔢 Adivina mi número"],
simon:["🧠 Memory","🧠 记忆挑战","🧠 記憶挑戰","🧠 Ingatan","🧠 Gedächtnis","🧠 Mémoire","🧠 記憶ゲーム","🧠 Memoria"],
rpsHelp:["Pick one. I pick at the same time: look at my screen!","选一个。我会同时出拳，看我的屏幕！","選一個。我會同時出拳，看我的螢幕！","Pilih satu. Aku memilih bersamaan, lihat layarku!","Wähl eins. Ich wähle gleichzeitig: schau auf meinen Bildschirm!","Choisis. Je joue en même temps : regarde mon écran !","ひとつ選んでね。わたしも同時に出すよ、画面を見て！","Elige uno. Yo elijo a la vez: ¡mira mi pantalla!"],
tttHelp:["You are ✕, I am ◯. Tap a square.","你是 ✕，我是 ◯。点一个格子。","你是 ✕，我是 ◯。點一個格子。","Kamu ✕, aku ◯. Ketuk satu kotak.","Du bist ✕, ich bin ◯. Tipp auf ein Feld.","Tu es ✕, je suis ◯. Touche une case.","きみは ✕、わたしは ◯。マスをタップしてね。","Tú eres ✕ y yo ◯. Toca una casilla."],
guessHelp:["I'm thinking of a number from 1 to 100.","我想了一个 1 到 100 的数字。","我想了一個 1 到 100 的數字。","Aku memikirkan angka dari 1 sampai 100.","Ich denke an eine Zahl von 1 bis 100.","Je pense à un nombre entre 1 et 100.","1 から 100 の数を考えたよ。","Estoy pensando en un número del 1 al 100."],
simonHelp:["Watch the arrows on my screen, then tap them here in the same order.","看我屏幕上的箭头，再按同样的顺序点这里。","看我螢幕上的箭頭，再按同樣的順序點這裡。","Lihat panah di layarku, lalu ketuk di sini dengan urutan yang sama.","Schau dir die Pfeile auf meinem Bildschirm an und tipp sie hier in derselben Reihenfolge.","Regarde les flèches sur mon écran, puis touche-les ici dans le même ordre.","画面の矢印を見て、同じ順番でここをタップしてね。","Mira las flechas en mi pantalla y tócalas aquí en el mismo orden."],
youWin:["You win! 🎉","你赢了！🎉","你贏了！🎉","Kamu menang! 🎉","Du gewinnst! 🎉","Tu gagnes ! 🎉","きみの勝ち！🎉","¡Ganas tú! 🎉"],
iWin:["I win! 😄","我赢了！😄","我贏了！😄","Aku menang! 😄","Ich gewinne! 😄","J'ai gagné ! 😄","わたしの勝ち！😄","¡Gano yo! 😄"],
draw:["A draw!","平局！","平手！","Seri!","Unentschieden!","Égalité !","引き分け！","¡Empate!"],
iPicked:["I picked {m}.","我出了 {m}。","我出了 {m}。","Aku pilih {m}.","Ich habe {m}.","J'ai joué {m}.","わたしは {m}。","Yo saqué {m}."],
score:["You {a} · Momowo {b}","你 {a} · 摸摸我 {b}","你 {a} · 摸摸我 {b}","Kamu {a} · Momowo {b}","Du {a} · Momowo {b}","Toi {a} · Momowo {b}","きみ {a} · Momowo {b}","Tú {a} · Momowo {b}"],
yourTurn:["Your turn!","轮到你了！","輪到你了！","Giliranmu!","Du bist dran!","À toi !","きみの番！","¡Tu turno!"],
newGame:["New game","再来一局","再來一局","Main lagi","Neues Spiel","Nouvelle partie","もう一回","Nueva partida"],
guessBtn:["Guess","猜！","猜！","Tebak","Raten","Deviner","当てる","Adivinar"],
higher:["Higher! ⬆️","再大一点！⬆️","再大一點！⬆️","Lebih besar! ⬆️","Höher! ⬆️","Plus grand ! ⬆️","もっと大きい！⬆️","¡Más alto! ⬆️"],
lower:["Lower! ⬇️","再小一点！⬇️","再小一點！⬇️","Lebih kecil! ⬇️","Niedriger! ⬇️","Plus petit ! ⬇️","もっと小さい！⬇️","¡Más bajo! ⬇️"],
gotIt:["Yes! You got it in {n} tries 🎉","猜中了！用了 {n} 次 🎉","猜中了！用了 {n} 次 🎉","Benar! {n} kali tebakan 🎉","Richtig! {n} Versuche 🎉","Gagné ! En {n} essais 🎉","正解！{n} 回で当たり 🎉","¡Sí! Lo lograste en {n} intentos 🎉"],
tries:["Tries: {n}","已猜 {n} 次","已猜 {n} 次","Tebakan: {n}","Versuche: {n}","Essais : {n}","{n} 回目","Intentos: {n}"],
newNum:["New number","换个数字","換個數字","Angka baru","Neue Zahl","Nouveau nombre","新しい数","Nuevo número"],
start:["Start","开始","開始","Mulai","Los geht's","Commencer","スタート","Empezar"],
watch:["Watch my screen… 👀","看我的屏幕…… 👀","看我的螢幕…… 👀","Lihat layarku… 👀","Schau auf meinen Bildschirm… 👀","Regarde mon écran… 👀","画面を見てね… 👀","Mira mi pantalla… 👀"],
level:["Level {n}","第 {n} 关","第 {n} 關","Level {n}","Level {n}","Niveau {n}","レベル {n}","Nivel {n}"],
oops:["Oops! You reached level {n}.","哎呀！你到了第 {n} 关。","哎呀！你到了第 {n} 關。","Ups! Kamu sampai level {n}.","Hoppla! Du hast Level {n} erreicht.","Oups ! Tu as atteint le niveau {n}.","あっ！レベル {n} まで行ったよ。","¡Ups! Llegaste al nivel {n}."],
best:["Best: {n}","最高：{n}","最高：{n}","Terbaik: {n}","Rekord: {n}","Record : {n}","最高：{n}","Récord: {n}"],
again:["Play again","再玩一次","再玩一次","Main lagi","Nochmal","Rejouer","もう一回","Otra vez"],
offline:["I can't reach Momowo. Is your phone on the same Wi-Fi?","连不上摸摸我。手机连的是同一个 Wi-Fi 吗？","連不上摸摸我。手機連的是同一個 Wi-Fi 嗎？","Tidak bisa terhubung ke Momowo. Apakah ponselmu di Wi-Fi yang sama?","Ich erreiche Momowo nicht. Ist dein Handy im selben WLAN?","Je n'arrive pas à joindre Momowo. Ton téléphone est-il sur le même Wi-Fi ?","Momowo につながらないよ。同じ Wi-Fi にいる？","No puedo conectar con Momowo. ¿Tu móvil está en la misma WiFi?"],
roll:["🌟 Star roll","🌟 滚球摘星","🌟 滾球摘星","🌟 Bola bintang","🌟 Sternenrollen","🌟 Attrape-étoiles","🌟 ほしころがし","🌟 Rueda estrellas"],
riddle:["🧩 Riddles","🧩 猜谜","🧩 猜謎","🧩 Teka-teki","🧩 Rätsel","🧩 Devinettes","🧩 なぞなぞ","🧩 Adivinanzas"],
rollHelp:["Hold Momowo and tilt it to roll the ball onto the stars on my screen. You have 30 seconds! From 3 stars, watch out for the rain cloud.","拿起摸摸我，倾斜它，让屏幕上的小球滚到星星上。你有 30 秒！拿到 3 颗星后，小心雨云。","拿起摸摸我，傾斜它，讓螢幕上的小球滾到星星上。你有 30 秒！拿到 3 顆星後，小心雨雲。","Pegang Momowo dan miringkan untuk menggulirkan bola ke bintang di layarku. Waktumu 30 detik! Mulai 3 bintang, awas awan hujan.","Halte Momowo und kipp es, damit die Kugel auf meinem Bildschirm zu den Sternen rollt. Du hast 30 Sekunden! Ab 3 Sternen: Vorsicht vor der Regenwolke.","Prends Momowo et penche-le pour faire rouler la bille sur les étoiles de mon écran. Tu as 30 secondes ! Dès 3 étoiles, attention au nuage de pluie.","Momowo を持ってかたむけて、画面のボールをほしまで転がしてね。30 秒だよ！ほしが 3 こになったら雨雲に気をつけて。","Sujeta a Momowo e inclínalo para llevar la bola a las estrellas de mi pantalla. ¡Tienes 30 segundos! Desde 3 estrellas, cuidado con la nube de lluvia."],
riddleHelp:["My AI makes up a riddle for you. Type your answer here; it's on my screen too.","我的 AI 给你出一道谜语。在这里输入答案，我的屏幕上也会显示谜语。","我的 AI 給你出一道謎語。在這裡輸入答案，我的螢幕上也會顯示謎語。","AI-ku membuat teka-teki untukmu. Ketik jawabanmu di sini; teka-tekinya juga ada di layarku.","Meine KI denkt sich ein Rätsel für dich aus. Tipp hier deine Antwort; das Rätsel steht auch auf meinem Bildschirm.","Mon IA invente une devinette pour toi. Écris ta réponse ici ; elle s'affiche aussi sur mon écran.","AI がなぞなぞを作るよ。答えはここに入力してね。画面にも出るよ。","Mi IA inventa una adivinanza para ti. Escribe tu respuesta aquí; también sale en mi pantalla."],
noMpu:["This game needs the motion sensor (MPU6050).","这个游戏需要运动传感器（MPU6050）。","這個遊戲需要動作感測器（MPU6050）。","Permainan ini perlu sensor gerak (MPU6050).","Dieses Spiel braucht den Bewegungssensor (MPU6050).","Ce jeu a besoin du capteur de mouvement (MPU6050).","このゲームにはモーションセンサー（MPU6050）が必要だよ。","Este juego necesita el sensor de movimiento (MPU6050)."],
tiltMe:["Get ready… tilt me! 🌟","准备……倾斜我！🌟","準備……傾斜我！🌟","Siap-siap… miringkan aku! 🌟","Achtung… kipp mich! 🌟","Prêt… penche-moi ! 🌟","よーい… かたむけて！🌟","Prepárate… ¡inclíname! 🌟"],
left:["{s} s left","还剩 {s} 秒","還剩 {s} 秒","Sisa {s} detik","Noch {s} s","Encore {s} s","のこり {s} 秒","Quedan {s} s"],
timeUp:["Time's up! You caught {n} stars.","时间到！你拿到了 {n} 颗星。","時間到！你拿到了 {n} 顆星。","Waktu habis! Kamu dapat {n} bintang.","Zeit um! Du hast {n} Sterne.","Terminé ! Tu as attrapé {n} étoiles.","時間切れ！ほしを {n} こ取ったよ。","¡Tiempo! Atrapaste {n} estrellas."],
newBest:["New record: {n} stars! 🎉","新纪录：{n} 颗星！🎉","新紀錄：{n} 顆星！🎉","Rekor baru: {n} bintang! 🎉","Neuer Rekord: {n} Sterne! 🎉","Nouveau record : {n} étoiles ! 🎉","新記録：ほし {n} こ！🎉","¡Nuevo récord: {n} estrellas! 🎉"],
thinking:["Thinking of a riddle… 🤔","正在想谜语…… 🤔","正在想謎語…… 🤔","Sedang memikirkan teka-teki… 🤔","Ich denke mir ein Rätsel aus… 🤔","Je réfléchis à une devinette… 🤔","なぞなぞを考え中… 🤔","Pensando una adivinanza… 🤔"],
checking:["Let me check… 🤔","让我看看…… 🤔","讓我看看…… 🤔","Coba kucek… 🤔","Moment, ich prüfe… 🤔","Je vérifie… 🤔","確かめるね… 🤔","Déjame ver… 🤔"],
notQuite:["Not \"{g}\". Try again!","不是“{g}”，再试试！","不是「{g}」，再試試！","Bukan \"{g}\". Coba lagi!","Nicht „{g}“. Nochmal!","Pas « {g} ». Réessaie !","「{g}」じゃないよ。もう一回！","No es «{g}». ¡Otra vez!"],
solved:["Yes, it's {a}! You got it in {n} tries 🎉","答对了，是{a}！你猜了 {n} 次 🎉","答對了，是{a}！你猜了 {n} 次 🎉","Benar, {a}! {n} kali tebakan 🎉","Richtig, {a}! Nach {n} Versuchen 🎉","Oui, c'est {a} ! En {n} essais 🎉","正解、{a}！{n} 回で当たり 🎉","¡Sí, es {a}! En {n} intentos 🎉"],
itWas:["The answer was: {a}","答案是：{a}","答案是：{a}","Jawabannya: {a}","Die Lösung: {a}","La réponse : {a}","答えは：{a}","La respuesta era: {a}"],
noAi:["I couldn't think of one. Riddles need my AI key and Wi-Fi (and today's AI limit not used up).","我想不出来。猜谜需要 AI 密钥和 Wi-Fi（今天的 AI 次数也不能用完）。","我想不出來。猜謎需要 AI 金鑰和 Wi-Fi（今天的 AI 次數也不能用完）。","Aku tidak bisa membuatnya. Teka-teki perlu kunci AI dan Wi-Fi (dan batas AI hari ini belum habis).","Mir fällt keins ein. Rätsel brauchen meinen KI-Schlüssel und WLAN (und das heutige KI-Limit darf nicht aufgebraucht sein).","Je n'en trouve pas. Les devinettes ont besoin de ma clé d'IA et du Wi-Fi (et de la limite d'IA du jour pas épuisée).","思いつかなかった。なぞなぞには AI キーと Wi-Fi が必要だよ（今日の AI 上限が残っていることも）。","No se me ocurre ninguna. Las adivinanzas necesitan mi clave de IA y Wi-Fi (y no haber gastado el límite de IA de hoy)."],
hintBtn:["💡 Hint","💡 提示","💡 提示","💡 Petunjuk","💡 Tipp","💡 Indice","💡 ヒント","💡 Pista"],
giveUp:["Give up","放弃","放棄","Menyerah","Aufgeben","Abandonner","ギブアップ","Me rindo"],
newRiddle:["New riddle","新谜语","新謎語","Teka-teki baru","Neues Rätsel","Nouvelle devinette","新しいなぞなぞ","Nueva adivinanza"],
quest:["🗺️ Quest","🗺️ 冒险","🗺️ 冒險","🗺️ Petualangan","🗺️ Abenteuer","🗺️ Aventure","🗺️ ぼうけん","🗺️ Aventura"],
questHelp:["My AI tells a little adventure on my screen. For each moment, do what the picture shows before the bar runs out: tap my head, hold it, tilt me the way the arrow points, or shake me (gently!). You have 3 hearts, and quick answers score double.","我的 AI 会在屏幕上讲一个小冒险。每一刻，在时间条走完前照着图标做：点我的头、按住、朝箭头方向倾斜我，或者（轻轻地）摇一摇我。你有 3 颗爱心，反应快得分翻倍。","我的 AI 會在螢幕上講一個小冒險。每一刻，在時間條走完前照著圖示做：點我的頭、按住、朝箭頭方向傾斜我，或者（輕輕地）搖一搖我。你有 3 顆愛心，反應快得分加倍。","AI-ku menceritakan petualangan kecil di layarku. Di setiap momen, lakukan sesuai gambar sebelum batang waktu habis: ketuk kepalaku, tahan, miringkan aku ke arah panah, atau goyangkan aku (pelan-pelan!). Kamu punya 3 nyawa, dan jawaban cepat dapat skor ganda.","Meine KI erzählt auf meinem Bildschirm ein kleines Abenteuer. Mach bei jedem Moment, was das Bild zeigt, bevor der Balken abläuft: tipp auf meinen Kopf, halt ihn gedrückt, kipp mich in Pfeilrichtung oder schüttel mich (sanft!). Du hast 3 Herzen, schnelle Antworten zählen doppelt.","Mon IA raconte une petite aventure sur mon écran. À chaque moment, fais ce que montre l'image avant la fin de la barre : touche ma tête, garde le doigt dessus, penche-moi dans le sens de la flèche ou secoue-moi (doucement !). Tu as 3 cœurs, et les réponses rapides comptent double.","わたしの AI が画面で小さなぼうけんのお話をするよ。時間のバーがなくなる前に、絵のとおりにしてね：頭をタップ、長押し、矢印の方へかたむける、または（やさしく）ふる。ハートは 3 つ、すばやく答えると点数 2 倍！","Mi IA cuenta una pequeña aventura en mi pantalla. En cada momento, haz lo que muestra el dibujo antes de que se acabe la barra: tócame la cabeza, mantén el dedo, inclíname hacia la flecha o agítame (¡con cuidado!). Tienes 3 corazones y las respuestas rápidas valen el doble."],
questThink:["Writing your adventure… ✍️","正在写你的冒险…… ✍️","正在寫你的冒險…… ✍️","Sedang menulis petualanganmu… ✍️","Ich schreibe dein Abenteuer… ✍️","J'écris ton aventure… ✍️","ぼうけんを書いているよ… ✍️","Escribiendo tu aventura… ✍️"],
questReady:["Get ready… look at my screen! 👀","准备……看我的屏幕！👀","準備……看我的螢幕！👀","Siap-siap… lihat layarku! 👀","Achtung… schau auf meinen Bildschirm! 👀","Prêt… regarde mon écran ! 👀","よーい… 画面を見てね！👀","Prepárate… ¡mira mi pantalla! 👀"],
questStep:["Moment {i} of {n}","第 {i} / {n} 刻","第 {i} / {n} 刻","Momen {i} dari {n}","Moment {i} von {n}","Moment {i} sur {n}","{i} / {n}","Momento {i} de {n}"],
questWin:["You made it! Score {s} 🎉","冒险成功！得分 {s} 🎉","冒險成功！得分 {s} 🎉","Berhasil! Skor {s} 🎉","Geschafft! {s} Punkte 🎉","Réussi ! Score {s} 🎉","やったね！{s} 点 🎉","¡Lo lograste! {s} puntos 🎉"],
questLose:["Out of hearts! Score {s}","没有爱心了！得分 {s}","沒有愛心了！得分 {s}","Nyawa habis! Skor {s}","Keine Herzen mehr! {s} Punkte","Plus de cœurs ! Score {s}","ハートがなくなった！{s} 点","¡Sin corazones! {s} puntos"],
questNoStory:["No AI story this time, so it's a quick one: just follow the pictures.","这次没有 AI 故事，来个快速版：照着图标做就行。","這次沒有 AI 故事，來個快速版：照著圖示做就好。","Kali ini tanpa cerita AI, jadi versi cepat: ikuti saja gambarnya.","Diesmal ohne KI-Geschichte, also die schnelle Version: folg einfach den Bildern.","Pas d'histoire IA cette fois, version rapide : suis les images.","今回は AI のお話なしの早わざ版：絵のとおりにしてね。","Esta vez sin historia de IA, versión rápida: sigue los dibujos."],
questNoMpu:["No motion sensor, so I'll only ask for taps and holds.","没有运动传感器，所以只会让你点和按住。","沒有動作感測器，所以只會讓你點和按住。","Tanpa sensor gerak, jadi hanya ketuk dan tahan.","Kein Bewegungssensor, also nur Tippen und Halten.","Pas de capteur de mouvement : seulement toucher et maintenir.","モーションセンサーがないので、タップと長押しだけだよ。","Sin sensor de movimiento, solo toques y mantener."],
tapG:["⚡ Quick tap","⚡ 快手拍拍","⚡ 快手拍拍","⚡ Tepuk kilat","⚡ Schnell tippen","⚡ Tape vite","⚡ はやタッチ","⚡ Toque rápido"],
tapHelp:["A reaction game on my touch pad. When my screen flashes and I beep, tap the top of my head as fast as you can! Tap too soon and the round starts again, and watch out: sometimes I only pretend. 5 rounds, your average counts.","用我的触摸点玩的反应游戏。我的屏幕一闪、发出哔声时，用最快的速度点我的头顶！点太早这一轮就重来。小心，我有时只是假装哦。一共 5 轮，看平均时间。","用我的觸摸點玩的反應遊戲。我的螢幕一閃、發出嗶聲時，用最快的速度點我的頭頂！點太早這一輪就重來。小心，我有時只是假裝喔。一共 5 輪，看平均時間。","Permainan refleks di tombol sentuhku. Saat layarku berkedip dan aku berbunyi, ketuk atas kepalaku secepat mungkin! Terlalu cepat, ronde diulang. Hati-hati, kadang aku cuma pura-pura. 5 ronde, rata-ratanya yang dihitung.","Ein Reaktionsspiel mit meinem Touchfeld. Wenn mein Bildschirm aufblitzt und ich piepe, tipp so schnell du kannst oben auf meinen Kopf! Zu früh getippt, und die Runde beginnt neu. Aufgepasst: Manchmal tu ich nur so. 5 Runden, dein Schnitt zählt.","Un jeu de réflexes sur ma touche tactile. Quand mon écran clignote et que je bipe, touche le haut de ma tête le plus vite possible ! Trop tôt, et la manche recommence. Attention, parfois je fais semblant. 5 manches, ta moyenne compte.","タッチパッドで遊ぶ反応ゲーム。画面が光ってピッと鳴ったら、できるだけ早く頭のてっぺんをタッチ！早すぎるとその回はやり直し。ときどきフェイントもあるから気をつけてね。5 回の平均で勝負。","Un juego de reflejos con mi botón táctil. Cuando mi pantalla parpadee y pite, ¡toca la parte de arriba de mi cabeza lo más rápido que puedas! Si tocas antes, la ronda empieza de nuevo. Cuidado: a veces solo finjo. 5 rondas, cuenta tu media."],
tapHead:["Tap my head to start, or press Start.","点我的头开始，或按“开始”。","點我的頭開始，或按「開始」。","Ketuk kepalaku untuk mulai, atau tekan Mulai.","Tipp auf meinen Kopf oder drück auf Los geht's.","Touche ma tête ou appuie sur Commencer.","頭をタッチするか、スタートを押してね。","Toca mi cabeza o pulsa Empezar."],
tapRound:["Round {i} of {n}","第 {i} / {n} 轮","第 {i} / {n} 輪","Ronde {i} dari {n}","Runde {i} von {n}","Manche {i} sur {n}","{i} / {n} 回目","Ronda {i} de {n}"],
tapWait:["wait for the flash… 👀","等闪光…… 👀","等閃光…… 👀","tunggu kilatannya… 👀","warte auf den Blitz… 👀","attends le flash… 👀","光るまで待ってね… 👀","espera el destello… 👀"],
tapNow:["Now! ⚡","就是现在！⚡","就是現在！⚡","Sekarang! ⚡","Jetzt! ⚡","Maintenant ! ⚡","いま！⚡","¡Ya! ⚡"],
tapSoon:["Too soon! That round starts again.","太早了！这一轮重来。","太早了！這一輪重來。","Terlalu cepat! Ronde itu diulang.","Zu früh! Die Runde beginnt neu.","Trop tôt ! La manche recommence.","はやすぎ！その回はやり直し。","¡Muy pronto! Esa ronda se repite."],
tapSlow:["Too slow!","太慢了！","太慢了！","Terlambat!","Zu langsam!","Trop lent !","おそい！","¡Muy lento!"],
tapMs:["{ms} ms","{ms} 毫秒","{ms} 毫秒","{ms} ms","{ms} ms","{ms} ms","{ms} ミリ秒","{ms} ms"],
tapAvg:["Your average: {ms} ms","你的平均：{ms} 毫秒","你的平均：{ms} 毫秒","Rata-ratamu: {ms} ms","Dein Schnitt: {ms} ms","Ta moyenne : {ms} ms","平均：{ms} ミリ秒","Tu media: {ms} ms"],
tapNewBest:["New record: {ms} ms! 🎉","新纪录：{ms} 毫秒！🎉","新紀錄：{ms} 毫秒！🎉","Rekor baru: {ms} ms! 🎉","Neuer Rekord: {ms} ms! 🎉","Nouveau record : {ms} ms ! 🎉","新記録：{ms} ミリ秒！🎉","¡Nuevo récord: {ms} ms! 🎉"],
bestMs:["Best: {n} ms","最好：{n} 毫秒","最好：{n} 毫秒","Terbaik: {n} ms","Rekord: {n} ms","Record : {n} ms","最高：{n} ミリ秒","Récord: {n} ms"],
settings:["⚙️ Momowo's settings","⚙️ 摸摸我的设置","⚙️ 摸摸我的設定","⚙️ Pengaturan Momowo","⚙️ Momowos Einstellungen","⚙️ Réglages de Momowo","⚙️ Momowo の設定","⚙️ Ajustes de Momowo"]};
const t=(k,v)=>{let s=(I18N[k]||[k])[L]||I18N[k][0];if(v)for(const x in v)s=s.replace("{"+x+"}",v[x]);return s};
const HANDS=["✊","✋","✌️"],GAMES=["rps","ttt","guess","simon","roll","riddle","quest","tap"];let cur=-1,questTimer=0,tapTimer=0;
function relabel(){document.documentElement.lang=["en","zh-Hans","zh-Hant","id","de","fr","ja","es"][L];
document.querySelectorAll("[data-t]").forEach(e=>e.textContent=t(e.dataset.t))}
async function post(o){try{const r=await fetch("/game",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(o)});
if(!r.ok)throw 0;return await r.json()}catch(e){status(t("offline"));return null}}
function status(s){const id=["rpsSt","tttSt","gSt","sSt","rSt","qSt","xSt","kSt"][cur];if(id)$(id).textContent=s}
const wait=ms=>new Promise(r=>setTimeout(r,ms));
function pick(i){cur=i;clearTimeout(questTimer);clearTimeout(rollTimer);clearTimeout(tapTimer);for(let k=0;k<8;k++){$("p"+k).style.display=k==i?"block":"none";$("t"+k).classList.toggle("on",k==i)}
if(i==1)ttt({});else if(i==2)guessNew(true);else post({g:GAMES[i],a:"open"}).then(r=>{if(!r)return;if(i==0)score(r);if(i==4)rollShow(r);if(i==5){rShow(r);if(r.phase==1||r.phase==3)rWait(r).then(x=>{if(x&&cur==5)rShow(x)})}if(i==3)simonIdle(r);if(i==7){tapShow(r);tapTimer=setTimeout(tapPoll,600)}if(i==6){questShow(r);if(r.phase>=1&&r.phase<=3)questTimer=setTimeout(questPoll,700)}})}
function score(r){$("rpsSc").textContent=t("score",{a:r.wins,b:r.losses})}
let busy=false;
async function rps(m){if(busy)return;busy=true;$("rpsSt").textContent="✊ ✊ ✊";const r=await post({g:"rps",m});
if(r){await wait(r.ms||0);$("rpsSt").textContent=t("iPicked",{m:HANDS[r.me]})+" "+[t("draw"),t("youWin"),t("iWin")][r.res];score(r)}busy=false}
function drawBoard(b,over){const g=$("board");g.innerHTML="";b.forEach((v,i)=>{const c=document.createElement("button");
c.textContent=v==1?"✕":v==2?"◯":"";c.disabled=!!v||!!over;c.onclick=()=>ttt({c:i});g.appendChild(c)})}
async function ttt(o){if(busy)return;busy=true;const mine=o.c;
if(mine!==undefined){const cells=[...$("board").children];cells[mine].textContent="✕";cells.forEach(c=>c.disabled=true)}
const r=await post(Object.assign({g:"ttt"},o));
if(r){if(mine!==undefined)await wait(450);drawBoard(r.b,r.over);$("tttSt").textContent=r.over?[0,t("youWin"),t("iWin"),t("draw")][r.over]:t("yourTurn")}busy=false}
async function guess(){const n=+$("num").value;if(!(n>=1&&n<=100))return;const r=await post({g:"guess",n});if(!r)return;
$("num").value="";$("num").focus();
$("gSt").textContent=r.hint==2?t("gotIt",{n:r.tries}):(r.hint>0?t("higher"):t("lower"))+"  ·  "+t("tries",{n:r.tries});
$("gBtn").disabled=r.hint==2}
async function guessNew(open){const r=await post(open?{g:"guess",a:"open"}:{g:"guess",a:"new"});if(!r)return;
$("gBtn").disabled=r.hint==2;$("gSt").textContent=r.tries&&r.hint!=2?t("tries",{n:r.tries}):""}
function arrows(on){for(let d=0;d<4;d++)$("d"+d).disabled=!on}
async function watch(r){arrows(false);$("sSt").textContent=t("level",{n:r.len})+" · "+t("watch");$("sSc").textContent=r.best?t("best",{n:r.best}):"";
await wait(r.ms);arrows(true);$("sSt").textContent=t("level",{n:r.len})+" · "+t("yourTurn")}
async function simonStart(){$("sGo").style.display="none";const r=await post({g:"simon",a:"start"});if(r)watch(r)}
async function tap(d){const b=$("d"+d);b.classList.add("hit");setTimeout(()=>b.classList.remove("hit"),200);
const r=await post({g:"simon",a:"tap",d});if(!r)return;
if(r.fail){arrows(false);$("sSt").textContent=t("oops",{n:r.reached});$("sSc").textContent=t("best",{n:r.best});
$("sGo").textContent=t("again");$("sGo").style.display="block"}else if(r.ms>0)watch(r);else simonIdle(r)}
function simonIdle(r){if(r&&!r.len&&!r.fail){arrows(false);$("sGo").style.display="block"}}
let rollTimer=0;
function rollShow(r){if(!r.mpu){$("rSt").textContent=t("noMpu");$("rGo").disabled=true;return}
if(r.run){$("rGo").style.display="none";$("rSt").textContent=r.wait?t("tiltMe"):"🌟 "+r.stars+"  ·  "+t("left",{s:Math.ceil(r.left/1000)})}
else{$("rGo").style.display="block";if(r.played){$("rSt").textContent=r.newBest?t("newBest",{n:r.stars}):t("timeUp",{n:r.stars});$("rGo").textContent=t("again")}}
$("rSc").textContent=r.best?t("best",{n:r.best}):""}
async function rollPoll(){clearTimeout(rollTimer);const r=await post({g:"roll",a:"state"});if(!r)return;rollShow(r);
if(r.run&&cur==4)rollTimer=setTimeout(rollPoll,700)}
async function rollStart(){const r=await post({g:"roll",a:"start"});if(!r)return;rollShow(r);if(r.run)rollTimer=setTimeout(rollPoll,700)}
function rShow(r){const p=r.phase;$("qText").textContent=p>=2&&p<=5?r.q||"":"";$("qHint").textContent=r.hint?"💡 "+r.hint:"";
$("qPlay").style.display=p==2?"block":"none";$("qNew").style.display=p==1||p==2||p==3?"none":"block";
$("qSt").textContent=p==1?t("thinking"):p==3?t("checking"):p==4?t("solved",{a:r.a,n:r.tries}):p==5?t("itWas",{a:r.a}):p==6?t("noAi"):""}
async function rWait(r){for(let k=0;cur==5&&r&&(r.phase==1||r.phase==3)&&k<150;k++){await wait(800);if(cur!=5)break;r=await post({g:"riddle",a:"state"})}return r}
async function rNew(){$("qSt").textContent=t("thinking");$("qNew").style.display="none";$("qText").textContent="";$("qHint").textContent="";
const r=await rWait(await post({g:"riddle",a:"new"}));if(r){rShow(r);if(r.phase==2)$("qIn").focus()}}
async function rGuess(){const g=$("qIn").value.trim();if(!g)return;$("qBtn").disabled=true;$("qSt").textContent=t("checking");
const r=await rWait(await post({g:"riddle",a:"guess",t:g}));$("qBtn").disabled=false;if(!r)return;rShow(r);
if(r.phase==2){$("qSt").textContent=t("notQuite",{g});$("qIn").value="";$("qIn").focus()}}
async function rAct(a){const r=await post({g:"riddle",a});if(r)rShow(r)}
function questShow(r){const p=r.phase,run=p>=1&&p<=3;$("xGo").style.display=run?"none":"block";
$("xTitle").textContent=p>=2&&r.title?r.title:"";$("xText").textContent=p==3&&r.t?r.t:p==4&&r.end?r.end:"";
$("xSt").textContent=p==1?t("questThink"):p==2?t("questReady"):p==3?"❤️".repeat(r.hearts)+"🤍".repeat(Math.max(0,3-r.hearts))+"  ·  "+t("questStep",{i:r.step,n:r.n}):p==4?(r.won?t("questWin",{s:r.score}):t("questLose",{s:r.score})):"";
if(p==4)$("xGo").textContent=t("again");
$("xSc").textContent=(p>=2&&!r.story?t("questNoStory")+" ":"")+(!r.mpu?t("questNoMpu")+" ":"")+(r.best?t("best",{n:r.best}):"")}
async function questPoll(){clearTimeout(questTimer);if(cur!=6)return;const r=await post({g:"quest",a:"state"});
if(cur!=6)return;if(!r){questTimer=setTimeout(questPoll,1500);return}questShow(r);
if(r.phase>=1&&r.phase<=3&&cur==6)questTimer=setTimeout(questPoll,700)}
async function questStart(){const r=await post({g:"quest",a:"start"});if(!r)return;questShow(r);if(r.phase>=1&&r.phase<=3)questTimer=setTimeout(questPoll,700)}
// Quick tap is played on Momowo's head, so the page just follows along
function tapShow(r){const p=r.phase,run=p>=1&&p<=3;$("kGo").style.display=run?"none":"block";
$("kSt").textContent=p==0?t("tapHead"):p==1?t("tapRound",{i:r.round,n:r.n})+" · "+t("tapWait"):p==2?t("tapNow")
:p==3?(r.last==-1?t("tapSoon"):r.last==-2?t("tapSlow"):t("tapRound",{i:r.round,n:r.n})+" · "+t("tapMs",{ms:r.last}))
:r.newBest?t("tapNewBest",{ms:r.avg}):t("tapAvg",{ms:r.avg});
if(p==4)$("kGo").textContent=t("again");$("kSc").textContent=r.best?t("bestMs",{n:r.best}):""}
async function tapPoll(){clearTimeout(tapTimer);if(cur!=7)return;const r=await post({g:"tap",a:"state"});if(cur!=7)return;
if(r)tapShow(r);tapTimer=setTimeout(tapPoll,r&&r.phase>=1&&r.phase<=3?500:1500)}
async function tapStart(){const r=await post({g:"tap",a:"start"});if(!r)return;tapShow(r);clearTimeout(tapTimer);tapTimer=setTimeout(tapPoll,500)}
addEventListener("pagehide",()=>{try{fetch("/game",{method:"POST",keepalive:true,headers:{"Content-Type":"application/json"},body:JSON.stringify({a:"quit"})})}catch(e){}});
(async()=>{try{const i=await(await fetch("/info")).json();L=i.lang||0}catch(e){}relabel();arrows(false);pick(0)})();
</script></body></html>
)GAMES";
