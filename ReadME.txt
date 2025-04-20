# Client_pico #
Program pre sifrovanu komunikaciu klient-server so steganografickou podporou
(Elligator 2 a PADME)
--------------------------------------------------------------------------------
2025-02-23, v.0.9.0pi, Nikita Kuropatkin KEMT FEI TUKE

# Specialna verzia klient-softveru pre Wiznet W5100S-EVB-Pico

Tato verzia klient-softveru bola vyvyjata specificky pre MCU 
Wiznet W5100S-EVB-Pico a obsahuje niekolko zmien oproti verziam pre Windows 
a Linux. Hlavne rozdiely:

# Program bezi v slucke: Po skonceni komunikacie sa okamzite zacina nova 
komunikacia.

# Restart pri chybe: Pri vyskyte chyby sa cely cip automaticky restartuje 
po 5 sekundach.

# Generovanie nahodnych cisel: Pouziva XDRBG generator pre vytvaranie 
nahodnych cisel.

# Komunikacia cez UART: Komunikacia s pocitacom, ktory riadi zariadenie 
(klient), je zabezpecena pomocou UART. 
To znamena, ze vsetky vystupy na MCU (`stdin`, `stdout`, `stderr`) su 
smerovane na UART piny (`Rx` a `Tx`).

# Pripojenie cez Ethernet: Ethernet sa pouziva na komunikaciu so serverom.

# Citanie zamaskovaneho kluca a soli z flash pamati: Kluc a sol musia byt 
predtym nahrate do flash pamati pomocou programu `flash_key_salt`. 
Hodnoty su ulozene v predposlednom sektore flash pamati procesora 
(Flash sector), na zaciatku stranky flash pamati procesora (Flash page).

Zakladny ciel programu:
---------------------------
Cielom tohto riesenia je pomocou zakladnych stavebnych prvkov, ktore ponuka 
Monocypher, vytvorit system na sifrovanu komunikaciu medzi dvoma stranami.  
System ma zaistit, aby komunikacia bola tazko analyzovatelna a zaroven skryt
samotnu existenciu komunikacie pomocou eliminacie tzv. metadat, medzi ktore 
patria rozmery klucov, nonce a MAC. Okrem toho zabezpecuje integritu dat a 
overenie zdrojov, taktiez je zabezpecena dopredna bezpecnost (forward secrecy).  

Pouzite bloky:
------------------
AEAD (ChaCha20 + Poly): Funkcia, ktora zarucuje sifrovanie dat a ich 
autentifikaciu naraz.
Pouzita implementacia vyuziva dva zakladne bloky:

ChaCha20: Prudova sifra, ktora sa pouziva na sifrovanie sprav medzi klientom 
a serverom.

Poly1305: Funkcia na generovanie jednorazovej hodnoty 
MAC (message authentication code), tzv. "tag", na zaistenie integrity 
posielanych dat.

X25519: Algoritmus na vymenu klucov zalozeny na eliptickej krivke Curve25519. V
tomto programe sa pouziva funkcia crypto_x25519_dirty_small, ktorá ku
vygenerovanému verejnému kľúču pridáva bod krivky s nízkym rádom. To umožňuje
generovať verejné kľúče na celej krivke, nielen v tzv. hlavnej podgrupy krivky.
Táto funkcia je použitá kvôli kompatibilite s Elligatorom.

Blake2b: Hasovacia funkcia zvolena na implementaciu KDF (Key Derivation
Function), teda na odvodenie spolocneho kluca po vymene klucov medzi dvoma
stranami.

Argon2i: Funkcia na hashovanie hesiel. Bezi v konstantnom case, co ju robi 
odolnou voci utokom postrannymi kanalami.
Zaroven je narocna na hardware, co vyrazne spomaluje metodu odhadovania hesla 
utokom hrubou silou.

Elligator 2: Tento blok je pouzity na zabezpecenie steganografie v programe.
Verejny kluc, ktory bol vygenerovany, je pomocou Elligatora namapovany na
skalar. Dovodom je, ze verejny kluc (pri sifrovani na zaklade eliptickej krivky
Curve25519) je bod na krivke, ktory ma tri vlastnosti, ktore mozu byt
analyzovane a mozu prezradit, ze prebieha vymena klucov, co je neziaduce. Dve z
tychto moznosti pre analyzu vieme odstranit pomocou Elligatora 2:

(1) Overenie rovnice krivky: V pripade Curve25519 sa to nevyplati, pretoze
nepracujeme s hodnotou "y".
(2) Overenie, ci ma rovnost x^3 + 486662x^2 + x mod(2^255-19) odmocninu: 
Ak posielame bod na krivke, tato rovnost bude mat odmocninu
so 100% pravdepodobnostou, ale ked posielame skalar (pseudo-nahodny), tato
pravdepodobnost klesne na 50%. Preto chceme bod na krivke namapovat na skalar,
ktory nebude davat odpovedajucemu systemu informacie o tom, ze prebieha vymena
klucov.

PADME: Algoritmus na upravu velkosti sprav, ktory eliminuje tretiu moznost
analyzy, a to analyzu velkosti sprav. Aj ked sme pouzili Elligator 2 na skrytie
verejneho kluca a odstranili dve moznosti analyzy, odpovedajuci system moze
stale analyzovat velkosti sprav. Kluce aj nonce pre rozne sifry maju pevne
stanovene velkosti (napr. kluc pre Curve25519 ma velkost 32 bajtov, nonce pre
ChaCha20 ma velkost 24 bajtov). Hoci tieto informacie same o sebe neodhaluju,
ci dochadza k vymene klucov, mozu byt pre nahodneho odpocuvaca indikatorom, ze
prebieha sifrovana komunikacia. PADME tento problem riesi tym, ze zmeni rozmery
sprav a skryje metadata spojene s komunikaciou.
Doplnkove:
LZRW3-A: Algoritmus bezstratovej kompresie, navrhnuty Rossom Williamsom.

Algoritmus prace programu:
---------------------------
(0) Server vlastni dlhodobo zdielany kluc (SK), klient ma jeho verziu, 
zabezpecenu pomocou PIN-kodu.

(1)Na zaciatku sa vytvori socket pre komunikaciu vo funkcii main().

(2)Potom sa zavola funkcia key_exc_ell, ktora zabezpeci vymenu klucov a
generovanie vysielajuceho(writing) a primajuceho(reading) klucov
na oboch stranach v tychto krokoch:
      
    1. Klient si zvoli svoj sukromny kluc (SK), na zaklade ktoreho sa vygeneruje
    verejny kluc (PK). PK sa nasledne pomocou Elligatora namapuje na skalar. 
    Po uprave velkosti spravy sa tento skryty PK posle serveru.

    2. Server dostane "skryty" PK od klienta, upravi jeho velkost a potom 
    pomocou Elligatora namapuje skalar na prislusny bod na krivke, 
    cim ziska PK klienta.

    3. Server pomocou funkcie key_hidden() vygeneruje svoj prvy SK a PK, 
    ktory Elligator namapuje na prislusny skalar. Tento skalar, upraveny pomocou 
    PADME, sa nasledne posle klientovi.

    4. Klient vykona rovnake kroky ako server v bode 2, aby ziskal PK servera.

    5. Obe strany zavolaju funkciu kdf() na generovanie prveho zdielaneho kluca.

        **Funkcia kdf() obsahuje dva nasledujuce kroky:
         - Obe strany pomocou funkcie `x25519` vygeneruju "shared secret," 
           co je surovy zdielany kluc.
         - Funkcia Blake2b sa pouzije na generovanie prveho zdielaneho kluca 
           pre AEAD (ChaCha20 + Poly1305). Pre server to bude prijimaci kluc 
           (reading key, na desifrovanie textu) a pre klienta vysielaci kluc 
           (writing key, na sifrovanie textu). 
           Pri generovani sa pouziju verejne kluce oboch stran a "shared secret".

    6. Na klientskej strane si system vyziada PIN, ktory sa pomocou funkcie 
    Argon2 zahasuje a zoxoruje so zdielanym klucom. Tym sa ziska dlhodoby 
    zdielany kluc (DSK) v cistej podobe, nezabezpeceny PIN-om. 
    (Predvolena hodnota PIN-u je "777777".)

    7. Blake2b sa pouzije na generovanie MAC, ktory si strany navzajom preposlu. 
    Tato vymena sluzi na overenie legitimity druhej strany. Ak su MAC rovnake, 
    znamena to, ze druha strana vlastni zdielany SK. Po overeni sa kluce 
    vygenerovane funkciou kdf() pouziju na dalsiu komunikaciu.

    8. Server pomocou funkcie key_hidden() vytvori svoj druhy par klucov 
    (SK a PK), ktory Elligator namapuje na skalar. Tento skalar, upraveny 
    pomocou PADME, sa posle klientovi.

    9. Obe strany zavolaju funkciu kdf() na generovanie druheho zdielaneho kluca.

       **Druha iteracia funkcie `kdf()` obsahuje nasledujuce kroky:
        - Obe strany pomocou funkcie `x25519` opat vygeneruju "shared secret".
        - Blake2b sa pouzije na generovanie druheho zdielaneho kluca pre 
        AEAD (ChaCha20 + Poly1305). 
        Pre server to bude vysielaci kluc (writing key, na sifrovanie textu) a 
        pre klienta prijimaci kluc (reading key, na desifrovanie textu). 
        Pri generovani sa pouzije druhy PK servera, PK klienta a "shared secret".

    10. Blake2b sa opat pouzije na generovanie MAC, ktory si strany preposlu na 
    overenie legitimity, rovnako ako v bode 7. Po uspechu sa druhe kluce 
    vygenerovane funkciou kdf() pouziju na dalsiu komunikaciu.

(3)Po uspesnej vymene klucov sa zavola funkcia chat, kde bude prebiehat samotna
komunikacia:
      
      1)Na zaciatku obe strany vygeneruju nonce a potom ho vyplnia pomocou
      PAMDE a poslu druhej strane.
      
      2)Kazda zo stran nainicializuje dva bloky AEAD: jeden pre autentifikovane 
      sifrovanie a druhy pre autentifikovane desifrovanie sprav od druhej strany. 
      Pricom pre inicializaciu kazdeho z blokov sa pouzije prislusny 
      zdielany kluc (vysielaci (writing key) alebo prijimaci (reading key)).

      3)Klient ziska spravu z konzoloveho vstupu, spravu komprimuje pomocou
      LZRW3-A a potom zasifruje a autentifikuje pomocou AEAD
      (ChaCha20 sa pouzije na sifrovanie a Poly1305 na generovanie MAC spravy).
      Nasledne sifrovanu spravu a MAC posle druhej strane.

      4)Server desifruje a autentifikuje prijatu spravu pomocou AEAD, potom ju
      dekomprimuje pomocou LZRW3-A, vykona rovnake kroky ako klient a posle
      klientovi zasifrovanu spravu a MAC.
      
      5)Komunikacia sa ukonci, ak niektora zo stran posle stop-slovo
      (predvolena hodnota je "exit") alebo ak niektora zo stran ukonci
      beh programu.

Navod na pouzitie:
-------------------
Program je kompatibilny pre vyvojovu dosku Wiznet W5100S-EVB-Pico:

#Verzie nastrojov:
arm-none-eabi 14.2.Rel1
Pico SDK 2.1.0
Wiznet SDK 2.0.0
Cmake: 3.31.3
Python: 3.11.9
Ninja: 1.12.1

Tento softver je kompatibilny s verziou 0.7.5 softveru pre Windows a Linux.
Program zabezpecuje pouzitie dosky Wiznet W5100S-EVB-Pico ako klienta 
pri chatovani.
Aby tento softver fungoval spravne, uzivatel musi na zaciatku nahrat 
zabezpecovaci kluc a sol do flash pamate dosky pomocou programu flash_key_salt.
Po nahrati mozete nahrat tento softver na dosku, ktora bude spojena 
s pocitacom pomocou serioveho kabla cez UART0, a ethernet kablom bude 
spojena so serverom.

Pri spusteni je dolezite nastavit terminal takto:
Baud rate: 115200
Line ending: LF
Local echo: on
Flow control: CTS/RTS
Data: 8 bit
Parity: none
Stop bits: 1
Po nastaveni mozete zapojit dosku a otvorit terminal.

####################
Sposob kompilacie:

#CMAKE#

      1) Musite mat nainstalovane vsetky nastroje pre kompilaciu programov
         pre dosku Wiznet W5100s, vratane Wiznet SDK.

      2) Pridajte projekt do priecinka examples.

      3) Upravte CMakeLists.txt, aby ste pridali riadok:
         add_subdirectory(client_pico).

      4) Otvorte cmd v priecinku build a napisite:
         cmake ..
         ninja

      5) Vytvoreny subor .uf2 nahrate do dosky.


 # Chybove kody #
     0 - program bol normalne ukonceny (ziadna chyba sa nevyskytla).  
     1 - chyba: nepodarilo sa spustit WSA.  
     2 - chyba: nepodarilo sa vytvorit socket.  
     3 - chyba: nepodarilo sa spojit socket s IP adresou (binding).  
     4 - chyba: nepodarilo sa spustit funkciu listen pre socket.  
     5 - chyba: nepodarilo sa pripojit klienta ku serveru (server accept).  
     6 - chyba: nepodarilo sa nahrat vstup.  
     7 - chyba: nepodarilo sa prijat data.  
     8 - chyba: nepodarilo sa poslat data.  
     9 - chyba: nepodarilo sa vygenerovat nahodne cisla.  
     10 - chyba: klientovi sa nepodarilo pripojit ku serveru.  
     11 - chyba: nespravne zadane cislo portu.  
     12 - chyba: nespravne zadana ip adresa.  
     13 - chyba: sprava bola modifikovana pocas prenosu.  
     14 - chyba: ina strana nie je legetimnou(nevlastni spolocny zdielany kluc).
     15 - chyba: klient zadal nespravny PIN pre SK.
     16 - chyba: klient nedodrzial podmienky formatovania pinu.
     17 - chyba: alokovanie pamate pre hashovanie zlyhalo.
     18 - chyba: rozmer komprimovaneho textu je vacsi ako buffer, kam sa ulozi.
     (Buffer, kam sa ulozi, je vacsi o 100 znakov ako nekomprimovany text,
     co znamena, ze LZRW3-A musi rozsirit nekomprimovany text o 100 znakov.
     To som nevedel dosiahnut pocas svojho testovania. Aj autor naznacoval,
     ze algoritmus by nemal vyrazne rozsirit povodny text,
     takze tato chyba ma EXTREMNE malu pravdepodobnost vyskytu.)
     19 - chyba: zlyhalo otvorenie suboru(File not found).
     20 - chyba: nespravny format data v txt subore.
     (Data musia byt ulozene v hexadecimalnej podobe).
     21 - chyba: nepodporovany rozmer pre vyplnenie.

 ################
# Zdroje #
https://elligator.org/
https://monocypher.org/
http://www.ross.net/compression/lzrw3a.html 