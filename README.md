# 🌤️ IoT Meteostanice: Vizualizace a zpracování dat

Vítejte v repozitáři mého maturitního projektu! Tento projekt se zaměřuje na návrh, realizaci a komplexní zpracování dat z domácí IoT meteostanice[cite: 29, 46]. Cílem bylo nejen data naměřit, ale především prakticky otestovat a porovnat různé moderní technologie pro jejich vizualizaci a dlouhodobé ukládání[cite: 26, 47, 50].

## 🛠️ Použitý Hardware

Srdcem celého systému je kompaktní mikrokontroler s Wi-Fi konektivitou, na který jsou napojeny senzory prostředí:

* **WeMos D1 R1:** Vývojová deska s čipem ESP8266, která zajišťuje sběr dat ze senzorů a jejich odesílání přes Wi-Fi[cite: 76, 252, 253].
* **DHT11:** Senzor pro měření aktuální teploty a vlhkosti vzduchu[cite: 80, 261].
* **BMP180:** Precizní senzor měřící barometrický tlak, ze kterého je následně dopočítávána nadmořská výška[cite: 78, 273, 278].

**Zapojení hardware:** <img width="189" height="215" alt="SCR-20260511-qyix" src="https://github.com/user-attachments/assets/6da5863c-cadd-4fdb-9e56-b8ff79bbe939" />
**Schéma zapojení:** <img width="267" height="281" alt="SCR-20260511-qyju" src="https://github.com/user-attachments/assets/8ef3981d-474a-4778-9465-61848341ed82" />


## 💻 Tech Stack & Architektura toku dat

Projekt využívá moderní přístup k IoT. Kód pro vývojovou desku byl napsán v **Arduino IDE**[cite: 287, 291]. Data jsou z desky odesílána dvěma nezávislými cestami, abych mohl otestovat různé technologie zpracování:

1.  **Real-time zpracování (Node-RED & MQTT):**
    * Naměřená data se formátují do JSON dokumentu a odesílají se přes odlehčený **MQTT protokol** (využit veřejný broker `broker.emqx.io`)[cite: 138, 230, 294, 297, 616].
    * Platforma **Node-RED** zprávy zachytává, dekonstruuje JSON a rovnou je zobrazuje na svém jednoduchém lokálním dashboardu[cite: 351, 354, 642, 643].

2.  **Dlouhodobé ukládání a pokročilá analytika (InfluxDB & Grafana / Web):**
    * Data jsou asynchronně zapisována přímo do **InfluxDB** – cloudové time-series databáze, která je optimalizovaná na časové řady z IoT senzorů[cite: 92, 139, 375, 632].
    * Pro komplexní analytiku, sledování trendů a interaktivní grafy je na InfluxDB napojena **Grafana** (čtení zajištěno přes bezpečný API klíč)[cite: 96, 418, 421, 649].
    * Jako alternativní vizualizační frontend byla vytvořena vlastní webová aplikace využívající **HTML, JavaScript a Google Charts**[cite: 97, 99, 102, 423].

## 📊 Ukázky vizualizací

Během projektu jsem testoval a srovnával tři různé přístupy k prezentaci dat. Níže jsou ukázky jednotlivých řešení.

### 1. Grafana (Pokročilý monitoring)
Grafana se ukázala jako nejlepší řešení pro komplexní dashboardy, historická data a sledování trendů díky nativnímu propojení s InfluxDB[cite: 508, 510, 511, 517].

**Grafana Dashboard:** <img width="1010" height="541" alt="SCR-20260511-qygp" src="https://github.com/user-attachments/assets/4c31d22c-831f-4274-95b7-714fbccce22b" />
*

### 2. Node-RED (Rychlé prototypování)
Ideální pro jednoduchou a okamžitou vizualizaci dat v reálném čase bez nutnosti nastavovat složité databáze[cite: 490, 491, 496].

**Node-RED flow:** <img width="1092" height="471" alt="SCR-20260511-qxxy" src="https://github.com/user-attachments/assets/5ef0ef14-01bb-41ab-ae90-91f8dcf67ef1" />


### 3. Google Charts (Webová integrace)
Skvělá volba pro embedování dat přímo do vlastních interaktivních online prezentací a webových stránek[cite: 484, 527, 532].

**Google Charts:** <img width="846" height="524" alt="SCR-20260511-qydy" src="https://github.com/user-attachments/assets/f5ab776e-0269-43de-b81c-3f3fc8aeeb35" />


---
*Tento projekt vznikl jako dlouhodobá maturitní práce v oboru Informační technologie - Vývoj aplikací a správa systémů (VOŠ a SPŠE Plzeň)[cite: 2, 3, 21, 22].*
