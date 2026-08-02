// --- INITIAL STATE ---
let isDataReceived = false;
let currentTimestamp = 0;
let requestID = null;
let driftCorrection = 0;

// --- FIREBASE INITIALIZATION (Compat SDK) ---
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

const database = firebase.database();
const auth = firebase.auth(); // Compat Auth
const dataRef = database.ref('/NetTime');

// --- AUTHENTICATION UI LOGIC ---
const emailInput = document.getElementById("userEmail");
const passwordInput = document.getElementById("userPassword");
const signInButton = document.getElementById("btnSignIn");
const signOutButton = document.getElementById("btnSignOut");
const statusDisplay = document.getElementById("authStatus");

// Giriş İşlemi
signInButton.addEventListener("click", () => {
    auth.signInWithEmailAndPassword(emailInput.value, passwordInput.value)
        .catch(err => alert("Giriş hatası: " + err.message));
});

// Çıkış İşlemi
signOutButton.addEventListener("click", () => {
    auth.signOut();
});

// Oturum Durumu Gözlemcisi
auth.onAuthStateChanged((user) => {
    if (user) {
        statusDisplay.innerText = "Status: Logged in as " + user.email;
        signInButton.style.display = "none";
        signOutButton.style.display = "inline-block";
        // Yetkili kullanıcı işlemleri burada aktifleşir
    } else {
        statusDisplay.innerText = "Status: Not logged in";
        signInButton.style.display = "inline-block";
        signOutButton.style.display = "none";
    }
});

// --- VERİ DİNLEME VE ARAYÜZ GÜNCELLEME ---
dataRef.on('value', (snapshot) => {
    const d = snapshot.val();
    if (d) {
        isDataReceived = true;
        document.getElementById('tp').innerText = d.sicaklik ?? "--";
        document.getElementById('hm').innerText = d.nem ?? "--";

        // ... (Kalan veri işleme kodlarınız buraya) ...

        driftCorrection = Date.now() - (d.timestamp * 1000);
        if (!requestID) startFluidClock();
    }
});

// --- BAĞLANTI TAKİBİ ---
firebase.database().ref(".info/connected").on("value", (snap) => {
    if (snap.val() !== true) {
        isDataReceived = false;
        updateUI();
    }
});

// --- DİĞER YARDIMCI FONKSİYONLAR ---
function updateUI() {
    const t = translations[currentLang];
    // UI güncellemeleriniz...
    document.getElementById('status').innerText = isDataReceived ? t.status_ok : t.waiting;
}

function startFluidClock() {
    // Saat döngünüz...
}