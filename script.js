// --- INITIAL STATE ---
let isDataReceived = false; // Flag to track if Firebase data has arrived
let currentTimestamp = 0;
let requestID = null;
let driftCorrection = 0;

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
    tr: {
        title: "NetTime Sunucusu", date: "Tarih", time: "Saat", temp: "Sıcaklık", hum: "Nem", isFed: "Yemleme Durumu", lastFed: "Yemleme Zamanı", status_ok: "Canlı Veri Akışı Aktif", /* New--> */ waiting: "Bağlantı Bekleniyor..." /* <--New */, btn: "EN"
    },
    en: { title: "NetTime Server", date: "Date", time: "Time", temp: "Temperature", hum: "Humidity", isFed: "Feeding Status", lastFed: "Feeding Time", status_ok: "Live Data Feed Active", /* New--> */ waiting: "Waiting For Connection..." /* <--New */, btn: "TR" }
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
    document.getElementById('lbl-isfed').innerText = t.isFed;
    document.getElementById('lbl-fedtp').innerText = t.lastFed;

    const st = document.getElementById('status');
    if (isDataReceived) {
        st.innerText = t.status_ok;
        st.classList.add('connected'); // Sets green color defined in CSS for active
    } else {
        st.innerText = t.waiting;
        st.classList.remove('connected'); // Sets the theme-specific yellow color defined in CSS for waiting
    }

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
        isDataReceived = true; // Set flag to true when data arrives
        document.getElementById('tp').innerText = d.sicaklik ?? "--";
        document.getElementById('hm').innerText = d.nem ?? "--";
        const isFedText = d.isFed
            ? (currentLang === 'tr' ? "Beslendi" : "Fed")
            : (currentLang === 'tr' ? "Yemleme Bekleniyor" : "Waiting for Feed");

        document.getElementById('data-isfed').innerText = isFedText;
        if (d.lastFedTime && d.lastFedTime !== "----") {
            const fedDate = new Date(d.lastFedTime * 1000);
            // HH:MM format
            const formattedTime = fedDate.toLocaleTimeString('tr-TR', {
                hour: '2-digit',
                minute: '2-digit',
                hour12: false
            });
            document.getElementById('data-fedtp').innerText = formattedTime;
        } else {
            document.getElementById('data-fedtp').innerText = "----";
        }
        const newTimestamp = d.timestamp;

        driftCorrection = Date.now() - (newTimestamp * 1000);

        if (!requestID && d.timestamp > 0) {
            startFluidClock();
        }
        updateUI();
    }
});

// --- CONNECTION MONITOR ---
// This section monitors the real-time connection state with the Firebase server
const connectedRef = firebase.database().ref(".info/connected");
connectedRef.on("value", (snap) => {
    if (snap.val() === true) {
        // Connection established, but still waiting for specific data (NetTime)
        // updateUI() will decide based on the isDataReceived flag
    } else {
        // Connection lost!
        isDataReceived = false; // Reset the flag
        updateUI(); // Revert the UI to "Waiting" state
    }
});

function startFluidClock() {
    if (requestID) cancelAnimationFrame(requestID);
    function update() {
        const now = Date.now();
        const actualTimestamp = Math.floor((now - driftCorrection) / 1000);

        let dateObj = new Date(actualTimestamp * 1000);

        let timeString = dateObj.toLocaleTimeString('tr-TR', { hour12: false });
        let dateString = dateObj.toLocaleDateString('tr-TR').replace(/\./g, '/');

        if (document.getElementById('t').innerText !== timeString) {
            document.getElementById('t').innerText = timeString;
            document.getElementById('date').innerText = dateString;
        }

        requestID = requestAnimationFrame(update);
    }

    requestID = requestAnimationFrame(update);
}

updateUI();