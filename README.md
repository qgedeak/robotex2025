# Vonalkövető Projekt

## Dokumentálás célja
- Memorandum
- Tanulási lehetőség másoknak

Ez a dokumentáció igyekszik megmutatni és összegyűjti mindazt, ami a 2025-ös Prosuli vonalkövető versennyel kapcsolatban készült.

## Előzmény
A vonalkövetési feladatok a robotversenyek leggyakoribb feladatai (FLL, WRO, RoboCup, RobotOlimpia, stb.).  
Vonalkövetésről készítettünk egy tanulmányt 2015-ben, Prezi formában még elérhető: [prezi](https://prezi.com/3eiayjo-uein/lego-robot-programozas/)

## Versenyszabályzat a magyar versenyről
- [Szabályok mentett](mds/2025.Line Following Rules.hu.v.04.07..md)
- [Szabályok mentett](mds/2025.Line%20Following%20Rules.hu.v.04.07..md)
- [Szabályok online](https://docs.google.com/document/d/1thQl6xXX_qvgtK1BIwJ8JObkCabkKbEllVdfRe9Oe6A/edit?pli=1&tab=t.0#heading=h.xe0xnnmimg7h)

## Versenyszabályok a nemzetközi versenyről
- [Szabályok online](https://robotex.international/wp-content/uploads/2024/07/Line-following-2024-ENG.pdf)

## Inspirációs videók
[!<img src="img/italy.jpg" alt="Robot kép" width="400">](https://youtu.be/MHesNWMKTPU)

## RobotC fejlesztőkörnyezetről
A RobotC a LEGO EV3 programozásában **közvetlen hardvervezérlést** kínál, ami gyors és precíz működést tesz lehetővé a grafikus (EV3-G) vagy Python megoldásokhoz képest.

---

### Előnyök

- **Valós idejű hardver-hozzáférés**  
  Motorok és szenzorok állapota azonnal olvasható és írható.

- **Precíz vezérlés**  
  Pontos ciklusok, időzítés és komplex algoritmusok közvetlen implementálása. Ideális PID szabályozáshoz vagy vonalkövetéshez.

- **Részletes hibakeresés**  
  A változók és szenzorértékek futás közben nyomon követhetők.

- **Teljes hardverhozzáférés**  
  Minden motor és szenzor közvetlenül vezérelhető, nincs „elrejtett réteg”.

---

### Összehasonlítás más megoldásokkal

| Megoldás | Előnyök | Hátrányok |
|----------|---------|-----------|
| **EV3-G (grafikus)** | Egyszerű, vizuális blokkok | Korlátozott hardver-hozzáférés |
| **Python (főiskolai verziók)** | Könnyen olvasható, magas szintű | Lassabb reakcióidő, kevésbé precíz |
| **RobotC** | Gyors, valós idejű, precíz | Több kódolást igényel, kevésbé vizuális |

### A Következő képen a Magyar nemzeti döntőben nyertes robotunk és a képre kattintva a videó a versenyszereplésről:
A programról: robotC-ben készült. A két középső szenzorral kétágú vonalkövés, a két szélső a vonal elhagyását detektálja.
[!<img src="img/alulrol.jpg" alt="Robot" title="go to youtube video" width="400">](https://youtube.com/shorts/WkEWVOLTY2w?feature=share)

## Robot kialakítása

Az inspiráció videójában látható konstrukció hardver paraméterei:
- EV3 Brick
- Power Functions → Powered Up adapter
- LEGO Power Functions L-Motor (88003)
- 3 db Color Sensor

### Várt előnyök
- Az L-motornak köszönhetően 380 fordulat/perc sebesség.
- 3 szenzor elhelyezésének köszönhetően a szenzorok súlyozott összege pontos és PID vonalkövetést tesz lehetővé. A vonal elvesztését a szélső szenzoroknak kell detektálniuk.
- RobotC fejlesztőkörnyezetnek köszönhetően a ciklusidő 2 ms alatt van. Így a legnagyobb sebesség esetén is a vonalon való áthaladásról több szenzorérték is elérhető a programban.

**Elért eredmények**
- Finomhangolt PID, nagy sebesség mellett is.
- Megoldatlan probléma: a tesztpályán 90°-os törésnél a robot elvesztette a vonalat, mert a szenzoradatokból eldönthetetlen volt, hogy melyik oldalra hagyja el a vonalat. Ennek magyarázatát a következő ábra mutatja:

| Leírás | Diagram | 
|----------|-----------|
| A diagramon a sarokra ráfutó robot 3 szenzorja által gyűjtött szenzorértékek vannak megjelenítve. Tisztán látszik, hogy nagyon hasonló a két minta, mégis az egyiken jobbra, a másikon balra hagyja el a vonalat. Így nem találtunk megoldást, hogy meghatározzuk merre halad a vonalat. | A szenzorok: S1 bal, S2 középső, S3 jobb szenzor. Az is látszik a diagramon, hogy az S2 hamarabb elhagyja a vonalat, miközben a másik kettő érzékeli a vonalat, de nem csak érintőlegesen, hanem rajta van, majd hirtelen eltűnik mindkét szélső szenzor számára, így nem lehet eldönteni, merre tűnt el! ![3 szenzor diagram](img/3_szenzor.jpg) |

### 3 vagy 4 szenzor?

| Leírás | Diagram | 
|----------|-----------|
| A három szenzor esetén sok információ érkezik a vonal környezetéből, pontos az error meghatározása!   **error = -S1 + S3 / (S1 + S2 + S3)**. Az error jelenti azt a távolságot, amely a vonal középvonalától a szenzorcsoport közepéig tart. 4 szenzor esetén a képen látható elhelyezésből nagy területről jön információ, viszont nem olyan precíz az error, mint 3 szenzor esetén. | ![3_4szenzor diagram](img/3_4sensor.jpg) |

### 4 szenzorral hogyan határozzuk meg a vonaltól való távolságot?

Ha egymás mellett helyezzük el a szenzorokat, akkor egyszerre **egy** vagy részben **két** szenzor látja a vonalat.  
- Ha a két középső látja részben, akkor jó helyen van.  
- Ha csak a középsők közül az egyik, akkor kis korrekcióra van szükség.  
- Ha a szélsők valamelyike, akkor nagy korrekcióra van szükség.  
- Ha egyik sem, akkor elvesztette a vonalat!  

![3_4szenzor diagram](img/4_szenzor_error.jpg)

# Line Follower Robot

Egy professzionális vonalkövető robot szoftver megoldás intelligens szenzor feldolgozással és adaptív PID vezérléssel.

## 📋 Tartalomjegyzék
- [Áttekintés](#áttekintés)
- [Főbb jellemzők](#főbb-jellemzők)
- [Hardver követelmények](#hardver-követelmények)
- [Szenzor elrendezés](#szenzor-elrendezés)
- [Telepítés](#telepítés)
- [Technikai dokumentáció](#technikai-dokumentáció)

## 🎯 Áttekintés

Ez a projekt egy fejlett vonalkövető robot szoftver implementációját tartalmazza, amely intelligens szenzor feldolgozással, adaptív PID szabályozással és speciális helyzetkezeléssel rendelkezik.

## ✨ Főbb jellemzők

- 🎯 **Speciális szenzor feldolgozás** - Normalizált és linearizált szenzor értékek
- 📈 **Adaptív PID vezérlés** - Dinamikus paraméter váltás sebesség alapján
- 🚀 **EMA sebesség becslés** - Sima sebesség számítás exponenciális mozgóátlaggal
- 🧠 **Okos helyzet kezelés** - Kereszteződés érzékelés, vonalvesztés kezelés
- ⚡ **Valós idejű teljesítmény** - Konzisztens 1-3ms ciklusidők

## 💻 Hardver követelmények

- 4x Analóg vonal szenzor
- 2x DC motor motorvezérlővel
- Mikrokontroller (Arduino kompatibilis)
- Tápegység

## 📡 Szenzor elrendezés

### 4 szenzorral hogyan határozzuk meg a vonaltól való távolságot?

A szenzorok egymás mellett helyezkednek el, és a következő logika alapján működnek:

- ✅ **Két középső szenzor érzékeli** → Robot pontosan a vonalon van
- ✅ **Csak egy középső szenzor érzékeli** → Kis korrekció szükséges
- ✅ **Szélső szenzor érzékel** → Nagy korrekció szükséges
- ❌ **Egyik szenzor sem érzékel** → Vonal elvesztése

## 🔧 Telepítés

1. **Kód feltöltése** a mikrokontrollerre
2. **Szenzorok kalibrálása** - automatikus maximum érték felvétel
3. **PID paraméterek beállítása** a pálya viszonyokhoz

## 🎮 Használat

A robot automatikusan elindul és követi a vonalat. Speciális helyzetekben:

- **Kereszteződés**: Egyenesen halad tovább
- **Vonalvesztés**: Visszatér az utolsó ismert irányba
- **Éles kanyar**: Sima forduló adaptív PID-vel

## 🔬 Technikai dokumentáció

### Szenzor normalizálás
```c
struct Sensors {
    byte S1, S2, S3, S4;
    byte S1MAX, S2MAX, S3MAX, S4MAX;
};

