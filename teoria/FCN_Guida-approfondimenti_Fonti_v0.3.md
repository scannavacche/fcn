# FCN — Guida agli approfondimenti per argomento
## Fonti principali: Bini–Capovani–Menchi; Quarteroni–Sacco–Saleri–Gervasio

Versione: 0.3  
Data: 2026-07-30

> **Convenzione sulle pagine.** Le pagine indicate sono quelle stampate nei libri, non il numero fisico della pagina nel PDF.
>
> **Scopo.** Questa guida non segue l'indice dei singoli volumi: riorganizza il materiale per argomenti del corso Fassino–Piana, così da poter confrontare più fonti sullo stesso tema.

> **Edizione Lawson–Hanson.** Charles L. Lawson, Richard J. Hanson, *Solving Least Squares Problems*, SIAM, Classics in Applied Mathematics 15, 1995. Edizione rivista e integrale della prima edizione Prentice-Hall del 1974.

---

# 1. Condizionamento, errore e stabilità

## Fondamenti: buona posizione e condizionamento

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 33–37 | Trattazione generale: buona posizione, sensibilità ai dati, numero di condizionamento |
| Bini–Capovani–Menchi | 136–140 | Trattazione applicata ai sistemi lineari |

## Stabilità dei metodi numerici

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 37–41 | Stabilità in senso generale e relazione con la convergenza |
| Bini–Capovani–Menchi | 164–172 | Stabilità del metodo di Gauss |
| Bini–Capovani–Menchi | 187–191 | Stabilità del metodo di Householder |
| Lawson–Hanson | 83–106 | Analisi degli errori di calcolo nelle trasformazioni di Householder e nella soluzione LS; estensione alla precisione mista |

## Analisi a priori, a posteriori, in avanti e all'indietro

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 42–44 | Impostazione generale dell'analisi dell'errore |
| Quarteroni–Sacco–Saleri–Gervasio | 62–66 | Applicazione ai sistemi lineari |

## Sorgenti di errore e modello computazionale

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 44–46 | Sorgenti di errore, modello matematico e modello computazionale |
| Fassino–Piana | Cap. 1 | Errore di approssimazione, inerente, algoritmico e totale |

## Numeri floating point e aritmetica di macchina

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 46–56 | Sistema posizionale, floating point, distribuzione, IEEE, arrotondamento, operazioni di macchina |
| Bini–Capovani–Menchi | 164–170 | Errore floating point visto dentro l'analisi di Gauss |

**Nota di approfondimento:** Quarteroni è particolarmente utile per chiarire la terminologia moderna: epsilon macchina, unità di roundoff, chopping, underflow, distribuzione dei numeri floating point.

---

# 2. Norme

## Prodotto scalare e norme vettoriali

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 17–20 | Prodotto scalare, norme, disuguaglianze fondamentali |
| Bini–Capovani–Menchi | 108–113 | Norme vettoriali e proprietà |

## Norme matriciali

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 20–25 | Norme matriciali, norme compatibili/indotte, relazione col raggio spettrale |
| Bini–Capovani–Menchi | 113–122 | Norme matriciali, norme indotte 1, 2, infinito, equivalenze e proprietà |

## Norma 2 e raggio spettrale

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 24–25 | Relazione tra norma matriciale e raggio spettrale |
| Bini–Capovani–Menchi | 114–121 | Derivazione esplicita delle principali norme indotte |

---

# 3. Sistemi lineari

## Rango, immagine, nucleo, consistenza

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 8–9 | Rango, range/immagine, nucleo |
| Bini–Capovani–Menchi | 13–16 | Immagine, nucleo, rango, consistenza e unicità |

## Condizionamento del sistema

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 60–66 | Numero di condizionamento e analisi in avanti/all'indietro |
| Bini–Capovani–Menchi | 136–141 | Perturbazione di matrice e termine noto |

## Sistemi triangolari

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 66–71 | Sostituzioni, implementazione, errore, inversa triangolare |
| Bini–Capovani–Menchi | 141–143; 165–167 | Soluzione e analisi dell'errore |

## Eliminazione di Gauss e fattorizzazione LU

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 71–81 | MEG, interpretazione come LU, arrotondamento, implementazione, forme compatte |
| Bini–Capovani–Menchi | 143–180 | Fattorizzazioni, matrici elementari, LU, Gauss, errore, pivoting, implementazione |

## Fattorizzazione QR

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 84–88 | QR per matrici rettangolari |
| Bini–Capovani–Menchi | 182–196 | Householder, analisi dell'errore, Givens |
| Lawson–Hanson | 9–17 | Decomposizioni ortogonali elementari: Householder, Givens, esistenza della QR |
| Lawson–Hanson | 53–62 | Implementazione numerica delle trasformazioni ortogonali |

## Householder e Givens

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 188–200 | Trattazione dettagliata nel contesto Hessenberg–QR |
| Bini–Capovani–Menchi | 182–196 | Householder e Givens nel contesto dei sistemi e delle fattorizzazioni |
| Lawson–Hanson | 9–17; 53–62 | Costruzione geometrica e uso computazionale diretto di Householder e Givens |

## Pivoting

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 88–93 | Pivoting e stabilità del MEG |
| Bini–Capovani–Menchi | 172–178 | Massimo pivot e controllo dell'errore |

## Calcolo dell'inversa

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 93 | Calcolo dell'inversa |
| Bini–Capovani–Menchi | 180–182 | Gauss–Jordan e inversa |

---

# 4. Autovalori e autovettori

## Richiami spettrali e similitudine

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 11–16 | Autovalori/autovettori, similitudine, decomposizione di Schur |
| Bini–Capovani–Menchi | 45–73 | Trattazione teorica estesa |

## Gershgorin e localizzazione geometrica

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 165–168 | Localizzazione geometrica, teorema di Gershgorin |
| Bini–Capovani–Menchi | 76–82; 316–319 | Gershgorin prima teorico, poi numerico |

## Perturbazione e condizionamento degli autovalori

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 168–174 | Bauer–Fike, stime a priori e a posteriori |
| Bini–Capovani–Menchi | 319–330 | Teoremi di perturbazione e caso hermitiano |

## Metodo delle potenze

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 174–178 | Autovalore di modulo massimo |
| Bini–Capovani–Menchi | 371–378 | Convergenza e varianti |

## Metodo delle potenze inverse

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 178–184 | Autovalore di modulo minimo, aspetti computazionali |
| Bini–Capovani–Menchi | 378–386 | Potenze inverse, shift e varianti |

## Metodi QR e Jacobi — approfondimento

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 184–203 | QR, Hessenberg, shift |
| Quarteroni–Sacco–Saleri–Gervasio | 203–212 | Jacobi e Sturm per matrici simmetriche |
| Bini–Capovani–Menchi | 333–386 | Tridiagonalizzazione, Hessenberg, QR, Jacobi, potenze |

---

# 5. Decomposizione ai valori singolari — SVD

## Definizione e proprietà fondamentali

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 16–17 | Presentazione molto compatta: SVD, rango, nucleo, range, pseudoinversa |
| Bini–Capovani–Menchi | 443–454 | Trattazione estesa, costruzione e proprietà |
| Lawson–Hanson | 18–22 | SVD come decomposizione ortogonale del problema LS; legame con \(A^T A\) e \(AA^T\) |

## Pseudoinversa di Moore–Penrose

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 16–17 | Definizione immediatamente dopo la SVD |
| Bini–Capovani–Menchi | 456–459 | Trattazione dedicata |
| Lawson–Hanson | 36–40 | Capitolo dedicato alla pseudoinversa |
| Lawson–Hanson | 41–48 | Perturbazioni della pseudoinversa |

## Perturbazione dei valori singolari

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Bini–Capovani–Menchi | 462–466 | Teoremi di perturbazione |
| Lawson–Hanson | 23–27 | Teoremi di perturbazione per i valori singolari |
| Lawson–Hanson | 284–311 | Appendice D: sviluppi 1974–1995, con aggiornamenti bibliografici anche su perturbazioni e SVD |
| Quarteroni–Sacco–Saleri–Gervasio | — | Non sviluppata come sezione autonoma nel percorso principale |

---

# 6. Problema ai minimi quadrati e soluzione di norma minima

## Sistemi sovra/sottodeterminati e soluzione di minima norma

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 108–112 | Sistemi non quadrati; SVD e pseudoinversa; soluzione euclidea di minima norma |
| Bini–Capovani–Menchi | 432–438 | Formulazione generale ed equazioni normali |
| Lawson–Hanson | 1–8 | Formulazione del Problem LS; casi sovra-, sotto- ed esattamente determinati; soluzione di norma minima |

## Equazioni normali

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 108–112 | Equazioni normali nel contesto dei sistemi non quadrati |
| Bini–Capovani–Menchi | 432–438 | Derivazione e proprietà |
| Lawson–Hanson | 121–128 | Equazioni normali con fattorizzazione di Cholesky, presentate come metodo alternativo alla via ortogonale |

## Soluzione mediante QR

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 108–112 | QR per rango pieno; uso di QR della trasposta per minima norma |
| Bini–Capovani–Menchi | 438–443 | Metodo QR dedicato ai minimi quadrati |
| Lawson–Hanson | 63–82 | Algoritmi QR per problemi sovradeterminati, sottodeterminati e a pseudorango eventualmente carente |
| Lawson–Hanson | 129–133 | Ortogonalizzazione di Gram–Schmidt modificata come metodo alternativo |

## Soluzione mediante SVD e pseudoinversa

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 108–112 | Formula \(x^{*}=A^\dagger b\) e instabilità in caso di rango non pieno |
| Bini–Capovani–Menchi | 454–459 | Soluzione via valori singolari e pseudoinversa |
| Lawson–Hanson | 107–120 | Calcolo della SVD e soluzione del Problem LS mediante SVD |
| Lawson–Hanson | 36–40 | Pseudoinversa e caratterizzazione della soluzione di minima norma |

## Perturbazioni e condizionamento del problema LS

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Lawson–Hanson | 23–27 | Sensibilità dei valori singolari alle perturbazioni |
| Lawson–Hanson | 28–35 | Stime del numero di condizionamento di matrici triangolari |
| Lawson–Hanson | 41–48 | Perturbazioni della pseudoinversa |
| Lawson–Hanson | 49–52 | Perturbazioni della soluzione del Problem LS |
| Lawson–Hanson | 83–106 | Errore computazionale e lettura backward degli algoritmi basati su Householder |
| Lawson–Hanson | 180–206 | Analisi pratica del problema LS, trasformazioni dei dati e analisi tramite valori singolari |

**Punto particolarmente utile:** Lawson–Hanson separa con grande chiarezza la perturbazione dei dati dall'errore prodotto dall'algoritmo. Il capitolo 8 parte esplicitamente dalla precisione limitata di misure e osservazioni, mentre i capitoli 15–17 reinterpretano gli errori di arrotondamento come perturbazioni dei dati del problema.

## Minimi quadrati come approssimazione di funzioni

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 357–360 | Minimi quadrati discreti e polinomio di migliore approssimazione |

---

# 7. Equazioni differenziali ordinarie

## Problema di Cauchy

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 377–380 | Formulazione del problema ai valori iniziali |
| Bini–Capovani–Menchi | — | Non trattato |

## Metodi a un passo

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 380–391 | Metodi a un passo, errore, zero-stabilità, convergenza, stabilità assoluta |

## Eulero e Heun

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 380–391 | Eulero e metodi a un passo; Heun ricompare anche come predictor-corrector |

## Metodi multistep e Adams

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 396–410 | Multistep, Adams, BDF, consistenza, condizione delle radici, convergenza e stabilità |

## Predictor–corrector

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 410–417 | PEC, PECE, P(EC)^m, Adams–Bashforth / Adams–Moulton, Heun |

## Runge–Kutta

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 417–424 | Derivazione, adattività, regioni di stabilità assoluta |

## Stabilità assoluta

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 388–391 | Stabilità assoluta per metodi a un passo |
| Quarteroni–Sacco–Saleri–Gervasio | 407–410 | Stabilità assoluta per multistep |
| Quarteroni–Sacco–Saleri–Gervasio | 423–424 | Regioni di stabilità per Runge–Kutta |

---

# 8. Argomenti trasversali e lessico utile

## Matrici ortogonali/unitarie

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 6–7; 17–20 | Definizioni e geometria |
| Bini–Capovani–Menchi | 1–10 | Richiami, basi ortonormali, Gram–Schmidt |

## Matrici definite positive

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 27–29 | Definite positive, dominanza diagonale, M-matrici |
| Bini–Capovani–Menchi | 10–11; 73–76 | Definizioni e proprietà spettrali |

## Schur

| Fonte | Pagine | Ruolo |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 13–16 | Decomposizione di Schur |
| Bini–Capovani–Menchi | 59–73 | Forme canoniche e Schur |

---

# 9. Mappa rapida corso → fonti

| FCN | Argomento | Quarteroni | Bini | Lawson–Hanson |
|---:|---|---:|---:|---:|
| 1 | Condizionamento e stabilità | 33–56 | 136–140; 164–172; 187–191 | 83–106 (applicato a LS) |
| 2 | Norme | 17–25 | 108–122 | 233–239 (richiami) |
| 3 | Sistemi lineari / QR | 60–112 | 136–203 | 9–17; 53–82 |
| 4 | Autovalori | 165–212 | 45–82; 316–386 | solo come supporto a SVD/QR |
| 5 | SVD | 16–17; 108–112 | 443–466 | 18–27; 107–120 |
| 6 | Minimi quadrati | 108–112; 357–360 | 432–462 | 1–133; 180–206 |
| 7 | ODE | 377–428 | — | — |

---

# 10. Criterio per aggiungere nuove fonti

Ogni nuovo libro va indicizzato **dentro gli stessi argomenti**, non creando un indice parallelo per autore.

Esempio:

## Norma matriciale indotta

| Fonte | Pagine | Nota |
|---|---:|---|
| Quarteroni–Sacco–Saleri–Gervasio | 20–25 | Inquadramento sintetico e relazione col raggio spettrale |
| Bini–Capovani–Menchi | 113–122 | Dimostrazioni più estese delle norme indotte principali |
| Lawson–Hanson | 233–239 | Richiami di algebra lineare e proiezioni, utili come supporto ai minimi quadrati |
| Epperson | — | da inserire |

La colonna "Ruolo/Nota" deve dire **perché aprire proprio quella fonte**: definizione, dimostrazione, interpretazione, algoritmo, stabilità, implementazione o esercizio.
