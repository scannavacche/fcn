# FCN — Guida agli approfondimenti per argomento
## Fonte 01 — Bini, Capovani, Menchi, *Metodi numerici per l'algebra lineare*

Versione: 0.1  
Data: 2026-07-30

> **Convenzione sulle pagine.** Le pagine indicate sono quelle stampate nel libro, non il numero fisico della pagina nel PDF scansito.

> **Scopo.** Questo documento non segue l'indice del libro: riorganizza il materiale di Bini–Capovani–Menchi secondo gli argomenti del corso Fassino–Piana, per usarlo come guida agli approfondimenti e al confronto fra fonti.

---

# 1. Condizionamento, errore e stabilità

## Problema ben posto, perturbazioni, condizionamento

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 136–140 | Analisi dell'errore, teoria della perturbazione, problemi ben posti / ben condizionati, condizionamento della matrice |

**Uso consigliato:** supporto teorico al Cap. 1 FCN e soprattutto introduzione al condizionamento dei sistemi lineari.

**Nota:** Bini tratta l'errore soprattutto nel contesto dell'algebra lineare; non sostituisce la costruzione generale dell'errore inerente di Fassino–Piana per una funzione \(f:\mathbb R^n\to\mathbb R\).

## Aritmetica floating point e arrotondamento

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 164–170 | Errore di arrotondamento e propagazione nell'analisi del metodo di Gauss |
| Bini–Capovani–Menchi | 178 circa | Richiami a underflow e aspetti implementativi |

**Uso consigliato:** confronto applicativo con la teoria generale di precisione di macchina e propagazione dell'errore del Cap. 1 FCN.

## Stabilità di algoritmi concreti

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 164–172 | Analisi dell'errore del metodo di Gauss |
| Bini–Capovani–Menchi | 172–178 | Pivoting e controllo della crescita degli errori |
| Bini–Capovani–Menchi | 187–191 | Analisi dell'errore del metodo di Householder |

## Costo computazionale

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 200–203 | Confronto dei costi dei principali metodi diretti |
| Bini–Capovani–Menchi | 159–163 | Costo del metodo di Gauss |
| Bini–Capovani–Menchi | 180–181 | Costo di Gauss–Jordan |
| Bini–Capovani–Menchi | 194 circa | Costo del metodo di Givens |

---

# 2. Norme

## Norme vettoriali

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 108–113 | Definizione, norme principali, proprietà |

## Norme matriciali

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 113–118 | Norme matriciali; norma indotta |
| Bini–Capovani–Menchi | 118–121 | Proprietà delle norme |
| Bini–Capovani–Menchi | 121–122 | Relazioni fra le principali norme matriciali |
| Bini–Capovani–Menchi | 122–135 | Esercizi e risultati aggiuntivi; equivalenza, raggio spettrale, applicazioni |

**Punto particolarmente utile:** la norma matriciale indotta è definita come

\[
\|A\|=\max_{\|x\|=1}\|Ax\|,
\]

e vengono ricavate esplicitamente le norme indotte \(1\), \(2\) e \(\infty\).

---

# 3. Sistemi lineari

## Richiami strutturali: rango, immagine, nucleo, consistenza

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 13–16 | Immagine, nucleo, rango, sistemi consistenti, unicità della soluzione |

**Uso consigliato:** riferimento per la terminologia da confrontare con quella del corso e con Algebra Lineare.

## Condizionamento del sistema

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 136–141 | Perturbazione e condizionamento del problema lineare |
| Bini–Capovani–Menchi | 203–208 | Ulteriori risultati sul numero di condizionamento negli esercizi |

## Sistemi triangolari

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 141–143 | Risoluzione di sistemi triangolari |
| Bini–Capovani–Menchi | 165–167 | Analisi dell'errore nella sostituzione triangolare |

## Fattorizzazioni e LU

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 143–149 | Fattorizzazioni |
| Bini–Capovani–Menchi | 149–157 | Matrici elementari e fattorizzazione mediante matrici elementari |
| Bini–Capovani–Menchi | 157–159 | Metodo di Gauss per la fattorizzazione LU |
| Bini–Capovani–Menchi | 159–164 | Metodo di Gauss per la soluzione del sistema |
| Bini–Capovani–Menchi | 164–172 | Analisi dell'errore |
| Bini–Capovani–Menchi | 172–178 | Massimo pivot |
| Bini–Capovani–Menchi | 178–180 | Implementazione |

## Fattorizzazione QR — Householder

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 182–185 | Metodo di Householder |
| Bini–Capovani–Menchi | 185–187 | Implementazione |
| Bini–Capovani–Menchi | 187–191 | Analisi dell'errore |

## Fattorizzazione QR — Givens

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 191–196 | Matrici di Givens e fattorizzazione QR |

**Nota:** utile come via alternativa rispetto alla costruzione QR privilegiata nel corso.

## Calcolo dell'inversa

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 12–13 | Definizione e proprietà della matrice inversa |
| Bini–Capovani–Menchi | 180–182 | Gauss–Jordan e calcolo dell'inversa |
| Bini–Capovani–Menchi | 186 circa | Calcolo dell'inversa tramite Householder |

---

# 4. Autovalori e autovettori

## Richiami spettrali

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 45–59 | Definizioni, proprietà di autovalori e autovettori, similitudine |
| Bini–Capovani–Menchi | 59–73 | Forme canoniche e decomposizioni spettrali |

## Forma normale di Schur

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 63–66 circa | Forma normale di Schur e costruzione mediante similitudine unitaria |

## Localizzazione degli autovalori — Gershgorin

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 76–80 | Teoremi e cerchi di Gershgorin |
| Bini–Capovani–Menchi | 316–319 | Teoremi di localizzazione nel capitolo numerico |

**Nota:** l'argomento compare quindi due volte: prima come teoria delle matrici, poi nel contesto dei metodi numerici.

## Perturbazione degli autovalori

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 319–324 circa | Teoremi di perturbazione; Bauer–Fike e casi con matrici hermitiane |
| Bini–Capovani–Menchi | 324–330 | Teoremi di separazione |

## Metodo delle potenze

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 371–378 | Metodo delle potenze, convergenza, casi critici |

## Metodo delle potenze inverse

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 378–386 | Varianti del metodo delle potenze; variante di Wielandt, potenze inverse, shift |

**Punto utile:** Bini presenta direttamente il metodo inverso come applicazione del metodo delle potenze a \(A^{-1}\), evitando di formare esplicitamente l'inversa e risolvendo un sistema lineare a ogni passo.

---

# 5. Decomposizione ai valori singolari — SVD

## Premesse: norme per matrici rettangolari

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 443–444 | Norme di matrici non quadrate |

## Decomposizione ai valori singolari

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 444–454 | Definizione, costruzione e proprietà della SVD |

## Proprietà strutturali derivate dalla SVD

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 448–454 circa | Rango, nucleo, immagine, norme \(2\) e di Frobenius, legame con \(A^HA\) |

## Perturbazione dei valori singolari

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 462–466 | Teoremi di perturbazione dei valori singolari |

## Approfondimento computazionale

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 466–474 | Forma normale di Schur di \(A^HA\) |
| Bini–Capovani–Menchi | 479–481 | Metodo di Lanczos per valori e vettori singolari |

---

# 6. Problema lineare dei minimi quadrati

## Formulazione ed equazioni normali

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 432–438 | Problema dei minimi quadrati, residuo, equazioni normali |

## Soluzione mediante QR

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 438–443 | Soluzione con fattorizzazione QR; Householder |

## Soluzione mediante SVD

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 454–456 | Risoluzione del problema con i valori singolari |

## Pseudoinversa e soluzione di norma minima

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 456–459 | Pseudoinversa di Moore–Penrose e caratterizzazione delle soluzioni |
| Bini–Capovani–Menchi | 474–479 | Calcolo della soluzione di minima norma con gradiente coniugato |

## Condizionamento dei minimi quadrati

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 459–462 | Perturbazione e condizionamento; confronto QR / equazioni normali |

**Punto particolarmente utile:** viene evidenziato che il sistema delle equazioni normali coinvolge \(A^HA\), il cui numero di condizionamento in norma 2 è il quadrato di quello di \(A\); questo fornisce una motivazione numerica forte per preferire QR quando il condizionamento conta.

---

# 7. Equazioni differenziali ordinarie

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | — | Non trattate: il volume è dedicato all'algebra lineare numerica |

Per il Cap. 7 FCN serviranno quindi altre fonti.

---

# 8. Argomenti trasversali utili al corso

## Matrici unitarie / ortogonali

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 1–10 | Richiami fondamentali; ortogonalità e Gram–Schmidt |
| Bini–Capovani–Menchi | 63–73 | Uso nelle forme spettrali |
| Bini–Capovani–Menchi | 182–196 | Householder e Givens |

## Matrici definite positive

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 10–11 | Definizioni di base |
| Bini–Capovani–Menchi | 73–76 | Proprietà spettrali |
| Bini–Capovani–Menchi | 198–200 | Cholesky |

## Rango, immagine, nucleo

| Fonte | Pagine | Contenuto |
|---|---:|---|
| Bini–Capovani–Menchi | 13–16 | Teoria di base |
| Bini–Capovani–Menchi | 448–454 | Lettura tramite SVD |

---

# 9. Mappa rapida corso → Bini

| FCN | Argomento | Bini — pagine principali |
|---:|---|---:|
| 1 | Condizionamento e stabilità | 136–140; 164–172; 187–191; 200–203 |
| 2 | Norme | 108–122 |
| 3 | Sistemi lineari | 136–203 |
| 4 | Autovalori | 45–80; 316–330; 371–386 |
| 5 | SVD | 443–466 |
| 6 | Minimi quadrati | 432–462; 474–479 |
| 7 | ODE | non trattate |

---

# 10. Formato per le future fonti

Ogni nuovo libro viene aggiunto **per argomento**, non come indice separato del volume.

Esempio:

## Norma matriciale indotta

| Fonte | Pagine | Nota |
|---|---:|---|
| Bini–Capovani–Menchi | 113–118 | Definizione assiomatica e norme \(1,2,\infty\) |
| Quarteroni | — | da inserire |
| Epperson | — | da inserire |

Lo stesso argomento può quindi avere più riferimenti paralleli, con indicazione della funzione della fonte: teoria, dimostrazione, interpretazione, algoritmo, implementazione o esercizio.
