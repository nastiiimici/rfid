const express = require("express");
const fs = require("fs");
const cors = require("cors");
const app = express();

app.use(cors());
app.use(express.json());
app.use(express.static("public"));

const DATA_FILE = "./data.json";

// --- Funkce pro správu dat (načítání a automatická tvorba) ---
const loadData = () => {
  // Definujeme si výchozí šablonu, kterou chceme v souboru mít
  const defaultData = {
    uidNames: { 
      "D123F6A9": "Karta", 
      "E760E914": "Cip" 
    },
    activeCards: [],
    history: {}
  };

  // Pokud soubor neexistuje, vytvoříme ho
  if (!fs.existsSync(DATA_FILE)) {
    console.log(`Soubor ${DATA_FILE} nenalezen. Vytvářím nový s výchozími daty...`);
    fs.writeFileSync(DATA_FILE, JSON.stringify(defaultData, null, 2), "utf8");
    return defaultData;
  }

  // Pokud existuje, načteme ho
  try {
    const rawData = fs.readFileSync(DATA_FILE, "utf8");
    return JSON.parse(rawData);
  } catch (err) {
    console.error("Chyba při čtení data.json, soubor je možná poškozený. Používám čistý stav.");
    return defaultData;
  }
};

const saveData = (data) => {
  try {
    fs.writeFileSync(DATA_FILE, JSON.stringify(data, null, 2), "utf8");
  } catch (err) {
    console.error("Nepodařilo se uložit data do souboru:", err);
  }
};

// Inicializace databáze při startu
let db = loadData();

// --- Endpointy ---

app.get("/uids", (req, res) => {
  res.json(db.uidNames);
});

app.get("/active", (req, res) => {
  res.json(db.activeCards);
});

app.get("/history/:uid", (req, res) => {
  const uid = req.params.uid;
  const hist = db.history[uid] || [];
  const sortedHist = [...hist].sort((a, b) => new Date(b.time) - new Date(a.time));
  res.json(sortedHist);
});

app.post("/rfid", (req, res) => {
  const { uid } = req.body;
  if (!uid) return res.status(400).json({ error: "UID required" });
  
  const now = new Date();
  const isComing = !db.activeCards.includes(uid);
  
  // Aktualizace stavu v paměti
  if (isComing) {
    db.activeCards.push(uid);
  } else {
    db.activeCards = db.activeCards.filter(u => u !== uid);
  }

  if (!db.history[uid]) db.history[uid] = [];
  db.history[uid].push({
    type: isComing ? "PRICHOD" : "ODCHOD",
    time: now
  });

  // Zápis do JSONu
  saveData(db);

  console.log(`[RFID] ${uid} (${db.uidNames[uid] || "Neznámý"}) -> ${isComing ? "PŘÍCHOD" : "ODCHOD"}`);
  res.json({ ok: true, status: isComing ? "PRICHOD" : "ODCHOD" });
});

const PORT = 20002;
app.listen(PORT, () => {
  console.log(`-------------------------------------------`);
  console.log(`Server úspěšně běží na http://localhost:${PORT}`);
  console.log(`Databáze: ${DATA_FILE}`);
  console.log(`-------------------------------------------`);
});