# RFID Docházkový systém

Jednoduchý systém pro evidenci docházky pomocí RFID karet.


## O projektu

Projekt umožňuje zaznamenávat příchody a odchody uživatelů pomocí RFID čipů.
Systém propojuje hardware (ESP32 + RFID) s webovou aplikací.


## Jak to funguje

* Přiložení RFID karty ke čtečce
* Načtení UID karty
* Odeslání dat přes WiFi na server
* Zpracování a uložení dat
* Zobrazení stavu uživatele na webu
* Zvuková a vizuální odezva (OLED + bzučák)


## Použité technologie

* ESP32
* RFID čtečka
* OLED displej
* Web (HTML/CSS + ExpressJS Server)
* WiFi komunikace


## Webová část

Webová aplikace slouží pro:

* zobrazení aktuální docházky
* identifikaci uživatelů podle karty
* jednoduchý přehled (dashboard)


## Využití

* školní docházkový systém
* evidence zaměstnanců
* přístupové systémy
