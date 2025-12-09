# Applikáció fejlesztése TPMS szenzor méréséhez

 

## A Projektről

 

Ez a repository a "Applikáció fejlesztése TPMS szenzor méréséhez" című szakdolgozathoz tartozó kódot és releváns fájlokat tartalmazza. A projekt a Robert Bosch Kft. által biztosított téma keretében készült, és az ipari konzulensem és kollégáim támogatásával valósult meg.

A projekt célja egy olyan rendszer létrehozása, amely lehetővé teszi a TPMS (Tire Pressure Monitoring System) szenzorok adatainak gyűjtését és feldolgozását egy BLE (Bluetooth Low Energy) dongle segítségével.

 

A repositoryban a következő kulcsfontosságú komponenseket találja meg:

 

*   **TPMS Szenzor Firmware:** A TPMS szenzor működéséért felelős teljes forráskód, beleértve a szükséges include fájlokat és a fordítást automatizáló Makefile-t. Ezen felül a szenzorra közvetlenül feltölthető, előre fordított bináris (HEX) fájl is rendelkezésre áll.

*   **Dongle Scriptek:** A hardveres dongle kezeléséhez és a PC-vel való kommunikációhoz szükséges Python scriptek.

*   **Dongle Python Csomag:** Egy WHL kiterjesztésű Python csomag, amely a dongle szoftveres vezérlésének egyszerű telepítését és használatát biztosítja, megkönnyítve a PC és a dongle közötti interakciót.

 

### Beépített Eszközök és Technológiák

 

A projekt a következő hardver- és szoftvereszközöket használja:

 

*   **Nordic nRF52840 Dongle:** A BLE kommunikáció alapját képező hardver.

*   **TPMS Szenzor:** A gumiabroncsnyomás ellenőrző szenzor.

*   **SDK (v4.1):** A szenzor firmware-ének fejlesztéséhez használt szoftverfejlesztő készlet.

*   **Python (v3.11):** A dongle scriptek és a kezelőfelület fejlesztési nyelve.

 

## Főbb Funkciók

 

A rendszer a következő főbb funkciókat biztosítja:

 

*   **BLE Kommunikáció:** Stabil és megbízható BLE kapcsolat létesítése a Nordic nRF52840 Dongle és a TPMS szenzor között.

*   **Adatgyűjtés:** A TPMS szenzor által mért adatok (pl. nyomás, hőmérséklet) fogadása a dongle-n keresztül.

*   **Adatküldés és Konfiguráció:** A szenzor beállításainak módosítására a BLE kapcsolaton keresztül.
