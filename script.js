// --- THEME ---
document.getElementById('theme-toggle').addEventListener('click', () => {
    const html = document.documentElement;
    const newTheme = html.getAttribute('data-theme') === 'light' ? 'dark' : 'light';
    html.setAttribute('data-theme', newTheme);
    localStorage.setItem('theme', newTheme);
});

// --- LANGUAGE ---
let currentLang = 'tr';
const translations = {
    tr: { title: "NetTime Sunucusu", date: "Tarih", time: "Saat", temp: "Sıcaklık", hum: "Nem", status_ok: "Canlı Veri Akışı Aktif", btn: "EN" },
    en: { title: "NetTime Server", date: "Date", time: "Time", temp: "Temperature", hum: "Humidity", status_ok: "Live Data Feed Active", btn: "TR" }
};
const langBtn = document.getElementById('btn-lang');
langBtn.addEventListener('click', () => {
    currentLang = currentLang === 'tr' ? 'en' : 'tr';
    updateUI();
});
function updateUI() {
    const t = translations[currentLang];
    document.getElementById('lbl-title').innerText = t.title;
    document.getElementById('lbl-date').innerText = t.date;
    document.getElementById('lbl-time').innerText = t.time;
    document.getElementById('lbl-temp').innerText = t.temp;
    document.getElementById('lbl-hum').innerText = t.hum;
    langBtn.innerText = t.btn;
}

// --- FIREBASE ---
const firebaseConfig = {
    apiKey: "AIzaSyBuymOotkf84zMBQYSbT3HZgFmQXoO-o-0",
    authDomain: "nettime-server.firebaseapp.com",
    databaseURL: "https://nettime-server-default-rtdb.europe-west1.firebasedatabase.app",
    projectId: "nettime-server",
    storageBucket: "nettime-server.firebasestorage.app",
    messagingSenderId: "495196153506",
    appId: "1:495196153506:web:53a57cd0048d4a0dcf99fb"
};
if (!firebase.apps.length) firebase.initializeApp(firebaseConfig);
const dataRef = firebase.database().ref('/NetTime');
dataRef.on('value', (snapshot) => {
    const d = snapshot.val();
    if (d) {
        document.getElementById('tp').innerText = d.sicaklik ?? "--";
        document.getElementById('hm').innerText = d.nem ?? "--";
        document.getElementById('t').innerText = d.son_guncelleme ?? "--:--:--";
        document.getElementById('date').innerText = d.tarih ?? "--/--/--";
        const st = document.getElementById('status');
        st.innerText = translations[currentLang].status_ok;
        st.style.color = "#4CAF50";
    }
});
updateUI();