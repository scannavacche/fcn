---
header-includes: |
  \usepackage{needspace}
  \usepackage{endnotes}
  \let\footnote=\endnote  
---

# FCN - Sviluppo reflectors

I riflettori “classici” che entrano nei metodi di fattorizzazione sono essenzialmente tre famiglie: trasformazioni elementari di Gauss (che non sono ortogonali), riflettori di Householder (riflessioni ortogonali di rango 1) e rotazioni di Givens (ortogonali a supporto 2×2).[^1][^2][^3][^4][^5][^6]

Schematizzazione unificata con indicazioni d’uso e due esempi concreti (uno Householder, uno Givens).

***

## 1. Matrici elementari in forma unificata

Una buona prospettiva astratta è: molte trasformazioni usate in algoritmi di eliminazione o di ortogonalizzazione sono matrici del tipo

$$
M = I + \sigma\, u v^{T},
$$

con $u,v \in \mathbb{R}^n$ e $\sigma \in \mathbb{R}$, da cui segue che $M - I$ ha rango al più 1.[^2]

- Nel caso Householder, $M = I - 2vv^{T}$ con $\|v\|_2=1$, quindi $\sigma=-2$ e $u=v$. [^4][^5][^6]
- Molte matrici di Gauss che realizzano “una singola mossa” sono della forma $I+\sigma e_i e_j^{T}$ con $i\neq j$, quindi ancora rango 1.[^1][^2]

Le rotazioni di Givens sono invece matrici identità modificate su un blocco 2×2 ortogonale; la struttura è ancora molto localizzata, ma il rango di $G - I$ è tipicamente 2, non 1.[^3][^4]

***

## 2. Matrici elementari di Gauss

Queste sono le matrici che rappresentano algebricamente le tre operazioni elementari sulle righe (o colonne) usate nell’eliminazione di Gauss e nelle fattorizzazioni LU.[^7][^8][^9][^1]

Operazioni elementari sulle righe (versione “righe”):

- Scambio di due righe $i$ e $j$: matrice di permutazione $P_{ij}$, ottenuta scambiando le righe $i$ e $j$ dell’identità.[^7][^1]
- Moltiplicazione della riga $i$ per uno scalare non nullo $\alpha$: matrice diagonale $D$ con $D_{ii} = \alpha$, altri diagonal = 1.[^1][^7]
- Somma di un multiplo della riga $j$ alla riga $i$:
  $$
  E = I + \sigma e_i e_j^{T},\quad \sigma\in\mathbb{R},
  $$
  che aggiunge $\sigma$ volte la riga $j$ alla riga $i$.[^2][^7][^1]

Uso tipico:

- Eliminazione di Gauss / Gauss con pivoting: si applica una successione di matrici elementari di tipo “somma di multipli di righe” (più eventualmente permutazioni e scalamenti) per introdurre zeri sotto la diagonale della matrice; la fattorizzazione LU si può leggere come prodotto di queste matrici (o delle loro inverse).[^10][^9][^2]

Pro e contro, computazionalmente:

- Pro: struttura estrema (una sola riga non banale, o una coppia di righe scambiate), costo $O(n)$ per l’applicazione ad un vettore.[^10][^2]
- Contro: non sono ortogonali (tranne permutazioni e scalamenti con modulo 1), quindi possono amplificare gli errori di arrotondamento; per problemi di least squares e factorizzazioni ortogonali si preferiscono Householder o Givens.[^4][^11]

***

## 3. Riflettori di Householder

Definizione:

Una trasformazione di Householder in $\mathbb{R}^n$ ha la forma

$$
H = I - 2 v v^{T},
$$

con $v\in\mathbb{R}^n$, $\|v\|_2=1$. [^4][^5][^6]

Proprietà:

- $H$ è simmetrica e ortogonale: $H^{T} = H$, $H^{T}H = I$.[^5][^6][^4]
- Geometricamente è una riflessione rispetto all’iperpiano ortogonale a $v$.[^4][^5]
- Moltiplicare un vettore $x$ per $H$ costa $O(n)$: si calcola prima $\alpha = v^{T}x$, poi $Hx = x - 2\alpha v$.[^6][^4]

Uso tipico:

- Fattorizzazione QR “globale”: dato $A \in \mathbb{R}^{m\times n}$, si costruisce una sequenza di riflettori $H_1,\dots,H_k$ tali che

$$
H_k\cdots H_1 A = R,
$$

con $R$ triangolare superiore, e $Q = H_1^{T}\cdots H_k^{T}$ è ortogonale.[^3][^6][^4]
- Riduzione a forma bidiagonale (per SVD), Hessenberg, ecc.: sempre giocando con riflettori applicati da sinistra e/o destra.[^11][^2]

Scelta di $v$ per “zerare” una colonna:

- Dato $x \in \mathbb{R}^m$ (per esempio, la parte sotto-diagonale di una colonna), si vuole un $H$ che mandi $x$ in uno scalare times $e_1$: $Hx = \pm\|x\|_2 e_1$. [^4][^5][^6]
- Un costrutto numericamente stabile è:
    - $\alpha = \mathrm{sign}(x_1)\|x\|_2$.
    - $u = x + \alpha e_1$.
    - $v = u / \|u\|_2$.
    - Allora $H = I - 2vv^{T}$.[^5][^6][^4]

***

## 4. Rotazioni di Givens

Definizione:

Una rotazione di Givens agisce su un piano generato da due coordinate $i$ e $j$; in forma 2×2:

$$
G_2 =
\begin{bmatrix}
c & -s\\
s & c
\end{bmatrix},
\quad c^2 + s^2 = 1.
$$

[^3][^4]

In dimensione $n$, la Givens $\mathcal{G}_{ij}$ è l’identità tranne nel blocco 2×2 sugli indici $i,j$, dove compare $G_2$.[^4][^3]

Uso tipico:

- Fattorizzazione QR “locale”: si eliminano gli elementi subdiagonali uno per volta applicando una successione di rotazioni di Givens che agiscono su coppie di righe.[^3][^4]
- Molto usate quando la matrice è sparsa o quando si vogliono operazioni “locali” (per esempio aggiornamenti di QR quando si aggiunge una riga/colonna).[^11][^4]

Scelta di $c,s$ per annullare una componente:

- Dato un vettore $(x, y)^{T}$, si vogliono $c,s$ tali che

$$
\begin{bmatrix}
c & -s\\
s & c
\end{bmatrix}
\begin{bmatrix} x\\ y \end{bmatrix}
=
\begin{bmatrix} r\\ 0 \end{bmatrix}.
$$
- Una scelta stabile è

$$
r = \sqrt{x^2 + y^2},\quad c = x/r,\quad s = -y/r.
$$

[^4]

Pro e contro (rispetto a Householder):

- Pro: agiscono su pochi elementi; ideali per aggiornamenti a bassa complessità e per matrici sparse, perché non “riempiono” intere colonne.[^11][^4]
- Contro: per trasformazioni dense (QR completo) costano più floating-point operations rispetto a Householder, dato che servono molte rotazioni per annullare una colonna completa.[^11][^4]

***

## 5. Esempi numerici (schemi d’uso)

### Esempio 1: Householder che annulla la coda di un vettore

Dato

$$
x =
\begin{bmatrix}
4\\
3\\
0
\end{bmatrix},
\quad x \in \mathbb{R}^3,
$$

costruiamo un Householder $H$ che mandi $x$ in $\pm\|x\|_2 e_1$.

1. Norma: $\|x\|_2 = \sqrt{4^2 + 3^2 + 0^2} = 5$.
2. Scegli $\alpha = \mathrm{sign}(4)\cdot 5 = 5$.
3. $u = x + \alpha e_1 = \begin{bmatrix} 4+5 \\ 3 \\ 0 \end{bmatrix} = \begin{bmatrix} 9\\3\\0\end{bmatrix}$.
4. $\|u\|_2 = \sqrt{9^2 + 3^2} = \sqrt{81+9} = \sqrt{90} = 3\sqrt{10}$.
5. $v = u / \|u\|_2 = \frac{1}{3\sqrt{10}}\begin{bmatrix} 9\\3\\0\end{bmatrix} = \begin{bmatrix} 3/\sqrt{10}\\1/\sqrt{10}\\0 \end{bmatrix}$.
6. $H = I - 2 v v^{T}$.

Calcoliamo $v v^{T}$:

$$
v v^{T} = 
\begin{bmatrix}
3/\sqrt{10} \\ 1/\sqrt{10} \\ 0
\end{bmatrix}
\begin{bmatrix}
3/\sqrt{10} & 1/\sqrt{10} & 0
\end{bmatrix}
=
\frac{1}{10}
\begin{bmatrix}
9 & 3 & 0\\
3 & 1 & 0\\
0 & 0 & 0
\end{bmatrix}.
$$

Allora

$$
H = I - \frac{2}{10}
\begin{bmatrix}
9 & 3 & 0\\
3 & 1 & 0\\
0 & 0 & 0
\end{bmatrix}
=
\begin{bmatrix}
1 - 1.8 & -0.6 & 0\\
-0.6 & 1 - 0.2 & 0\\
0 & 0 & 1
\end{bmatrix}
=
\begin{bmatrix}
-0.8 & -0.6 & 0\\
-0.6 & 0.8 & 0\\
0 & 0 & 1
\end{bmatrix}.
$$

Controllo:

$$
Hx =
\begin{bmatrix}
-0.8 & -0.6 & 0\\
-0.6 & 0.8 & 0\\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
4\\
3\\
0
\end{bmatrix}
=
\begin{bmatrix}
-5\\
0\\
0
\end{bmatrix}
= -5 e_1,
$$

come previsto.[^6][^5][^4]

Uso in QR: questo $H$ sarebbe il primo riflettore applicato alla prima colonna di una matrice 3×3 per introdurre zeri sotto la diagonale; i riflettori successivi operano sui subblocchi inferiori destri.[^6][^3][^4]

***

### Esempio 2: Givens che annulla una singola componente

Vogliamo annullare la seconda componente del vettore

$$
z =
\begin{bmatrix}
3\\
4
\end{bmatrix},
$$

mediante una rotazione di Givens 2×2.[^4]

1. $r = \sqrt{3^2 + 4^2} = 5$.
2. $c = x/r = 3/5$.
3. $s = -y/r = -4/5$.

La rotazione

$$
G_2 =
\begin{bmatrix}
3/5 & 4/5\\
-4/5 & 3/5
\end{bmatrix}
$$

(dove ho usato la convenzione con il segno su $s$ compatibile con l’annullamento) soddisfa:

$$
G_2
\begin{bmatrix}
3\\
4
\end{bmatrix}
=
\begin{bmatrix}
5\\
0
\end{bmatrix}.
$$

Se inseriamo questo blocco in una matrice identità 3×3, operante su righe 2 e 3, otteniamo una rotazione di Givens $\mathcal{G}_{23}$ in $\mathbb{R}^3$ che annulla l’elemento in posizione (3,1) di una matrice, agendo sulle righe 2 e 3.[^3][^4]

***

## 6. Altri trasformatori “simili” a riflettori

Oltre a Gauss, Householder e Givens, nel design di algoritmi entrano altre trasformazioni “elementari” affini:

- Riflettori generalizzati di rango 1: $M = I - \sigma u u^{T}$ con $\sigma\neq 2$ possono essere usati per aggiornamenti rank-1 in contesti simili a Householder (non necessariamente ortogonali).[^2][^11]
- Trasformazioni unitariamente equivalenti: nelle versioni complesse, riflettori di Householder diventano $H = I - 2 \frac{v v^{*}}{v^{*}v}$, con $v^{*}$ coniugato trasposto; Givens diventano rotazioni complesse.[^5][^11][^4]

In pratica, la “triade” **Gauss–Householder–Givens** copre la maggior parte dei costrutti elementari usati in fattorizzazioni LU/QR, riduzioni a forma canonica (Hessenberg, tridiagonale, bidiagonale) e aggiornamenti strutturati.[^10][^2][^11][^4]


<div align="center">⁂</div>

[^1]: https://it.wikipedia.org/wiki/Matrice_elementare

[^2]: https://people.dm.unipi.it/bini/Didattica/AnaNum/testi/Dispense/matrici_elementari.pdf

[^3]: https://pages.stat.wisc.edu/~bwu62/771/04_hg.html

[^4]: https://www.cs.cornell.edu/~bindel/class/cs6210-f12/notes/lec16.pdf

[^5]: https://it.wikipedia.org/wiki/Trasformazione_di_Householder

[^6]: https://tex.unica.it/~gppe/did/ca/tesine/2006/06nescuc.pdf

[^7]: https://www.math.unipd.it/~parmeggi/stat1/LEZIONE5.pdf

[^8]: https://www.lezionidimatematica.net/matrici/lezioni/matrici_lezione_57.htm

[^9]: https://www.di.univr.it/documenti/OccorrenzaIns/matdid/matdid161234.pdf

[^10]: https://people.dm.unipi.it/bini/Didattica/AnaNum/testi/Dispense/gauss-householder.pdf

[^11]: https://www.sciencedirect.com/science/article/pii/002437958790111X/pdf?md5=ca77932f5fa12a519984da4257689fce\&pid=1-s2.0-002437958790111X-main.pdf

[^12]: http://www.sbai.uniroma1.it/~giovanni.cerulliirelli/supporto/appunti/Matrici Elementari.pdf

[^13]: https://www.docenti.unina.it/webdocenti-be/allegati/materiale-didattico/34665565

[^14]: https://www.math.unipd.it/~gerotto/FALG15/spazi_vettoriali_2.pdf

[^15]: https://www.youtube.com/playlist?list=PLpkXLf6Zhdx1myEYbwHWdAgqKjjkXX1Aj

\newpage


## Strategia didattica

Per riflettori e trasformazioni elementari,  **prima separare bene i comportamenti numerici**, solo dopo costruire un'interfaccia comune, perché Gauss, Householder e Givens hanno struttura, costo e uso algoritmico diversi anche se tutti agiscono come operatori elementari su righe/colonne o sottospazi molto piccoli.[^4][^3][^5][^6]

La progressione più naturale è: Householder per il caso “riflettore puro”, Givens per la trasformazione locale su due componenti, Gauss per la mossa eliminativa non ortogonale; così lo studente vede subito differenze geometriche, algebriche e numeriche prima della sintesi API.[^3][^7][^1]

In pratica:

- Householder: annulla in un colpo tutta la coda di un vettore o di una colonna, ed è il mattone standard per QR dense e riduzioni ortogonali.[^8][^1][^3]
- Givens: annulla una sola componente alla volta, ma è molto adatto a aggiornamenti locali e matrici sparse.[^2][^9][^3]
- Gauss: è il generatore naturale per eliminazione/LU, ma non preserva ortogonalità e quindi va tenuto concettualmente distinto dai riflettori ortogonali.[^10][^6][^11]


## Interfaccia prima dell’unificazione

Prima dell’interfaccia unica, conviene dare a ciascun trasformatore un piccolo contratto operativo comune: costruzione, applicazione a vettore, applicazione a matrice da sinistra/destra, eventuale inversione o trasposto.[^5][^1][^3]

Uno schema FCN plausibile potrebbe essere:

- `build(x, policy)` oppure `from_column_segment(...)`
- `apply_left(A, k0, k1, ...)`
- `apply_right(A, k0, k1, ...)`
- `apply(x)`
- `as_dense()` solo per test e didattica, non per uso interno prestazionale.[^1][^2][^3]

Questo ti permette di fare sperimentazione comparativa uniforme senza forzare subito una parametrizzazione artificiale comune.[^2][^4][^3]

## Criterio di unificazione

Dopo la fase didattica, l’unificazione ha senso se l’interfaccia comune descrive **l’azione** e non la formula interna del trasformatore.[^3][^1]

Per esempio, l’astrazione giusta non è necessariamente “tutti sono $I+\sigma uv^T$”, perché questo descrive bene Gauss e Householder ma non Givens in modo naturale, dato che Givens vive meglio come operatore ortogonale con supporto 2×2 su indici $(i,j)$.[^5][^3]
Molto meglio una base concettuale del tipo:

- `Transformation`
- `OrthogonalTransformation`
- `EliminationTransformation`

e poi specializzazioni:

- `HouseholderReflector`
- `GivensRotation`
- `GaussEliminationStep`[^7][^1][^3]


## Minimo insieme di varianti

Per minimizzare le varianti, suggerirei di fissare subito poche scelte non negoziabili:

- Convenzione unica di segno per Householder, per evitare cancellazione numerica nella costruzione del vettore riflettente.[^7][^1][^3]
- Givens sempre costruita da coppia $(x,y)$ con routine stabile per $c,s$.[^3]
- Gauss distinta in versione con e senza pivoting, ma con la stessa interfaccia esterna.[^6][^12]
- Rappresentazione implicita come default; esplicita solo per debug, test unitari e note didattiche.[^1][^3]

Così l’unificazione finale non cancella le differenze matematiche, ma riduce il rumore implementativo.[^2][^3]

## Proposta operativa

Io partirei con questo ordine di sviluppo:

1. `HouseholderReflector`, come prototipo di trasformatore ortogonale implicito e “globale” su una coda.[^8][^1]
2. `GivensRotation`, come prototipo di trasformatore ortogonale locale.[^2][^3]
3. `GaussEliminationStep`, come trasformatore eliminativo non ortogonale.[^11][^6]
4. Test comparativi comuni su: annullamento, costo operativo, stabilità, fill-in e uso in QR/LU.[^9][^4][^3][^2]
5. Solo a quel punto, una facciata unificata `apply_left/apply_right/apply/vector_action`.[^1][^3]

Questa sequenza è coerente con un’impostazione didattica-sperimentale e ti lascia aperta una futura unificazione elegante senza “violentare” Givens dentro una forma troppo pensata per Householder.[^5][^3]

Se vuoi, nel passo successivo posso proporti direttamente una **bozza C++ dell’interfaccia FCN** a tre classi distinte ma già predisposte per la successiva unificazione.
<span style="display:none">[^13][^14][^15][^16][^17][^18][^19][^20]</span>

<div align="center">⁂</div>

[^1]: https://www.math.ubbcluj.ro/~tradu/nlalgslides/lec3b_HouseGivens.pdf

[^2]: https://pages.stat.wisc.edu/~bwu62/771/04_hg.html

[^3]: https://www.cs.cornell.edu/~bindel/class/cs6210-f12/notes/lec16.pdf

[^4]: https://www.sciencedirect.com/science/article/pii/002437958790111X/pdf?md5=ca77932f5fa12a519984da4257689fce\&pid=1-s2.0-002437958790111X-main.pdf

[^5]: https://people.dm.unipi.it/bini/Didattica/AnaNum/testi/Dispense/matrici_elementari.pdf

[^6]: https://people.dm.unipi.it/bini/Didattica/AnaNum/testi/Dispense/gauss-householder.pdf

[^7]: https://it.wikipedia.org/wiki/Trasformazione_di_Householder

[^8]: https://receiptroller.co/en/technotes/p/householder-reflections-qr-decomposition-numerical-linear-algebra

[^9]: https://discourse.julialang.org/t/givens-rotation-based-compact-qr-for-sparse-matrices/5603

[^10]: https://johnkerl.org/doc/hh.pdf

[^11]: https://it.wikipedia.org/wiki/Matrice_elementare

[^12]: https://www.di.univr.it/documenti/OccorrenzaIns/matdid/matdid161234.pdf

[^13]: https://en.wikipedia.org/wiki/Householder_transformation

[^14]: https://sites.uclouvain.be/HHXIX/BookOfAbstracts.pdf

[^15]: https://arxiv.org/pdf/2104.04180.pdf

[^16]: https://www.studocu.vn/vn/document/truong-dai-hoc-bach-khoa-ha-noi/dieu-khien-logic-va-plc/matrix-computation-1-householder-transformations-and-qr-decomposition-fall/162352910

[^17]: https://www.youtube.com/watch?v=n0zDgkbFyQk

[^18]: https://householder-symposium.github.io/abstracts/paper102.html

[^19]: https://www.scribd.com/document/880365358/ln10

[^20]: https://www.scribd.com/document/986898202/Householders-Method-Group-9-1

\newpage


Il piano che ho in mente separa esplicitamente e didatticamente le due fasi per ciascun trasformatore: **costruzione** (scelta di parametri $u,v$, o $v$ solo, o $c,s$ per Givens, o moltiplicatori di Gauss in funzione del target), e **applicazione** (uso ripetuto, possibilmente ottimizzato, di quella trasformazione su vettori/matrici).[^1][^2][^3][^4][^5][^6]

comune e poi il dettaglio per Householder, Givens e Gauss.

***

## 1. Schema concettuale comune e dettagli per Householder, Givens e Gauss.

Per ogni trasformazione $T$ avremo due famiglie di operazioni ben distinte:

1. **Fase di costruzione (design):**
    - Input: “target” (ad esempio un vettore colonna da trasformare, o una coppia di componenti da azzerare, o una colonna di matrice in eliminazione).
    - Output: un set minimo di parametri che rappresentano $T$ in forma implicita (es. $v$ per Householder, $c,s$ per Givens, moltiplicatori per Gauss).[^2][^3][^4][^1]
2. **Fase di applicazione (use):**
    - Operazioni: `apply(x)` (vettore), `apply_left(A, range)`, `apply_right(A, range)`.
    - Tutto scritto in modo da rendere evidente la formula: proiezioni scalari, aggiornamento outer-product, ecc., senza “nascondere” la struttura sotto un generico `operator*` black-box.[^3][^4][^1][^2]

In termini di interfaccia penserei proprio a due gruppi di metodi: `construct_*` vs `apply_*`, o un costruttore che fa solo la parte “geometrica” e metodi d’istanza che fanno la parte “algebrica”.[^4][^5][^1][^2]

***

## 2. Householder: costruzione vs applicazione

**Principio:** un riflettore di Householder è $H=I-2vv^{T}$ con $\|v\|_2=1$; la costruzione consiste nello scegliere $v$ in funzione di un target $x$ che vogliamo mandare su una direzione data (tipicamente $e_1$), l’applicazione consiste nel calcolare $Hx = x-2(v^{T}x)v$. [^2][^1][^7][^8]

### Fase di costruzione (scelta di $v$)

Dato $x \in \mathbb{R}^m$ (ad esempio la sotto-colonna da riga $k$ a $m$), si vuole $H$ con
$Hx = \pm\|x\|_2 e_1$ (nel sottospazio considerato). [^2][^1][^7][^8]

Algoritmo numericamente stabile:[^9][^10][^2]

1. $\alpha = \mathrm{sign}(x_1)\|x\|_2$ (scelta del segno per evitare cancellazioni). [^2][^9]
2. $u = x + \alpha e_1$.
3. $v = u / \|u\|_2$.
4. Memorizzi $v$ (non $H$).[^1][^2]

Qui è importante la scelta del segno di $\alpha$ (analisi della cancellazione in $x_1 - \alpha$) e il fatto che $H$ è una riflessione rispetto all’iperpiano ortogonale a $v$.[^10][^9][^2][^1]

### Fase di applicazione

Data la $v$ scelta:

- Su vettore:

1. $\beta = v^{T}x$.
2. $y = x - 2\beta v$.[^7][^2][^1]
- Su matrice a sinistra (aggiorno alcune righe):

1. $\beta = v^{T}A$ (riga o sottoblocco di righe).
2. $A \leftarrow A - 2 v \beta$.[^8][^2][^7]

Tutto in forma implicita, con la formula scritta “in chiaro” nel codice; l’interfaccia può avere metodi separati `construct_from_column(...)` e `apply_left(...)`, quindi la distinzione è esplicita anche a livello didattico.[^2][^7][^8]

***

## 3. Givens: costruzione vs applicazione

**Principio:** una rotazione di Givens è un’ortogonale che agisce solo sul piano $(i,j)$, e serve a nullificare una singola componente $y$ usando la coppia $(x,y)$.[^11][^3]

### Fase di costruzione (scelta di $c,s$)

Dato $(x,y)$, con $x$ la componente che “sopravvive” e $y$ quella da eliminare, si cercano $c,s$ tali che

$$
\begin{bmatrix}
c & s\\
- s & c
\end{bmatrix}
\begin{bmatrix}
x\\ y
\end{bmatrix}
=
\begin{bmatrix}
r\\ 0
\end{bmatrix}.
$$

[^12][^3][^11]

Tipico algoritmo stabile:[^3][^11]

- $r = \sqrt{x^2 + y^2}$.
- $c = x/r$, $s = y/r$ (o variante con segni secondo la convenzione).

In codice, constructor o `construct(x,y)` che memorizza $c,s$ e gli indici $i,j$ su cui agire.[^11][^12][^3]

### Fase di applicazione

- Su vettore: aggiornare solo le componenti $i,j$:

$$
\begin{aligned}
t_i &= c x_i + s x_j,\\
t_j &= -s x_i + c x_j.
\end{aligned}
$$

[^3][^11]

- Su matrice a sinistra: combinare solo le righe $i,j$. Su matrice a destra: combinare solo le colonne $i,j$.[^12][^11][^3]

Ancora, la separazione è naturale: un metodo `construct(i,j,x,y)` produce i parametri, poi `apply_left(A)` implementa in chiaro la combinazione lineare di righe.[^11][^3]

***

## 4. Gauss: costruzione vs applicazione (elementary matrices)

**Principio:** l’eliminazione di Gauss è la costruzione di una sequenza di matrici elementari $E_k$ che realizzano scambi, scalamenti, e aggiunge multipli di righe.[^13][^5][^6][^4]

Per la singola mossa di eliminazione su una colonna:

### Fase di costruzione (scelta dei moltiplicatori)

Dato un pivot $a_{kk}$ e l’elemento $a_{ik}$ da annullare, il moltiplicatore è

$$
m_{ik} = a_{ik}/a_{kk}.
$$

[^5][^6][^4][^13]

La matrice elementare implicita è $E = I - m_{ik} e_i e_k^{T}$; in pratica memorizzi $m_{ik}$ e gli indici $i,k$.[^6][^4][^5]

### Fase di applicazione

- Su matrice: riga $i \gets \text{riga }i - m_{ik} \cdot \text{riga }k$.[^4][^13][^5][^6]
- Su vettore (per applicare l’operatore al lato destro in un sistema): stessa regola sulle componenti corrispondenti.[^5][^4]

Anche qui è molto didattico lasciare a vista la relazione con la forma $E = I + \sigma e_i e_j^{T}$ per legare il tutto alle trasformazioni di rango 1.[^4][^5]

***

## 5. Come riflettere tutto questo nell’interfaccia

Per mantenere la distinzione concettuale, ma prepararsi all’unificazione, io imposterei così:

- Ogni classe ha un **metodo di costruzione esplicito** (o costruttore statico) che usa **target + politica numerica**:
    - `HouseholderReflector::from_column_segment(x, policy_sign_choice)`
    - `GivensRotation::from_pair(i,j,x,y, policy_sign_choice)`
    - `GaussEliminationStep::from_pivot(a_kk, a_ik, pivot_policy)`[^9][^2][^3][^4]
- Ogni classe espone metodi di **applicazione ben visibili**:
    - `apply(Vector&) const`
    - `apply_left(Matrix&, Range rows, Range cols) const`
    - `apply_right(Matrix&, Range rows, Range cols) const`

in cui il codice implementa esplicitamente le formule che hai spiegato nella parte teorica.[^1][^2][^5][^3][^4]

- In una fase successiva, potrai interfacciare queste classi a una gerarchia comune tipo `LinearOperator` / `OrthogonalOperator`, ma senza nascondere mai i metodi di costruzione specifici (che sono la parte “pedagogica”).[^7][^2][^3]



<div align="center">⁂</div>

[^1]: https://en.wikipedia.org/wiki/Householder_transformation

[^2]: https://www.cs.cornell.edu/~bindel/class/cs6210-f12/notes/lec16.pdf

[^3]: https://www.sciencedirect.com/topics/engineering/givens-rotation

[^4]: https://receiptroller.co/en/technotes/p/row-operations-and-elementary-matrices-numerical-linear-algebra

[^5]: https://math.libretexts.org/Bookshelves/Linear_Algebra/Fundamentals_of_Matrix_Algebra_(Hartman)/01:_Systems_of_Linear_Equations/1.03:_Elementary_Row_Operations_and_Gaussian_Elimination

[^6]: https://en.wikipedia.org/wiki/Gaussian_elimination

[^7]: https://www.sciencedirect.com/topics/mathematics/householder-transformation

[^8]: https://mmids-textbook.github.io/chap02_ls/04_qr/roch-mmids-ls-qr.html

[^9]: https://www.aimsciences.org/article/doi/10.3934/naco.2023025

[^10]: https://epubs.siam.org/doi/10.1137/S0895479898338561

[^11]: https://student.cs.uwaterloo.ca/~cs475/CS475-Lecture13.pdf

[^12]: https://www.youtube.com/watch?v=MxZy0LLEDLY

[^13]: https://uomustansiriyah.edu.iq/media/lectures/5/5_2020_03_26!02_08_53_AM.pdf

[^14]: https://www.scribd.com/document/17131026/Householder-transformations

[^15]: https://pennylane.ai/demos/tutorial_givens_rotations/

\newpage


# Modello OOP (esempio)

Scheletro C++ per un **HouseholderReflector** didattico, con le due fasi volutamente separate e le formule rese esplicite, sul modello “genera riflettore” / “applica riflettore” che usano anche LAPACK (DLARFG per la generazione, DLARF per l’applicazione).[^1][^2][^3][^4]

L’idea è: la classe incapsula il vettore $v$ (e, facoltativamente, $\tau = 2$ o l’equivalente $\tau = 2/(v^{T}v)$ nella versione “non normalizzata”), mentre i metodi `construct_from_segment` e `apply_*` esplicitano rispettivamente la fase geometrica (scelta di $v$) e la fase algebrica (calcolo $x - 2(v^{T}x)v$ o $A - 2v(v^{T}A)$).[^5][^6][^1]

***

## Scheletro di `HouseholderReflector`

Assumo:

- un container tipo `Vector`/`Matrix` già tuo (o `std::vector<double>` + funzioni helper),
- reali in doppia precisione,
- versione “classica”: vogliamo che il riflettore mandi un segmento di colonna in un multiplo di $e_1$.[^7][^6][^5][^1]

```cpp
// HouseholderReflector.hpp
#pragma once
#include <cstddef>
#include <vector>
#include <cmath>
#include <cassert>

class HouseholderReflector {
public:
    using index_type = std::size_t;
    using real_type  = double;

    // Costruito vuoto, da riempire con construct_from_segment.
    HouseholderReflector() = default;

    // Costruttore da vettore v già normalizzato (uso avanzato/didattico).
    explicit HouseholderReflector(const std::vector<real_type>& v)
        : v_(v)
    {
        // opzionale: verifiche su norma(v) ~ 1
    }

    // ---------------------------
    // FASE DI COSTRUZIONE
    // ---------------------------
    //
    // Costruisce il riflettore che agisce sul segmento x[begin:begin+len)
    // mandandolo in +/- ||x_segment|| e1 (nel sottospazio).
    //
    // Il vettore v_ memorizzato ha la stessa dimensione di x_segment;
    // in applicazione si suppone di applicare H = I - 2 v v^T
    // all'interno di quel sottospazio, con embedding nel vettore/matrice globale.
    //
    void construct_from_segment(const real_type* x, index_type len)
    {
        assert(x != nullptr);
        assert(len > 0);

        v_.assign(len, real_type{0});

        // Copia x in v_ (per poterlo manipolare)
        for (index_type i = 0; i < len; ++i)
            v_[i] = x[i];

        // Norma euclidea di x_segment
        real_type norm_x2 = 0;
        for (index_type i = 0; i < len; ++i)
            norm_x2 += v_[i] * v_[i];

        const real_type norm_x = std::sqrt(norm_x2);

        if (norm_x == real_type{0}) {
            // Riflettore identità: v = e1 (per convenzione)
            v_[^0] = 1;
            for (index_type i = 1; i < len; ++i) v_[i] = 0;
            return;
        }

        // Scelta del segno per evitare cancellazione: alpha = sign(x1) * ||x||
        const real_type x0    = v_[^0];
        const real_type sign  = (x0 >= 0) ? 1.0 : -1.0;
        const real_type alpha = sign * norm_x;

        // u = x + alpha e1
        v_[^0] += alpha;

        // Ora v_ = u. Normalizziamo per ottenere v = u / ||u||
        real_type norm_u2 = 0;
        for (index_type i = 0; i < len; ++i)
            norm_u2 += v_[i] * v_[i];

        const real_type norm_u = std::sqrt(norm_u2);
        if (norm_u == real_type{0}) {
            // Caso patologico (non dovrebbe accadere se norm_x!=0).
            // Prendiamo ancora v = e1.
            v_[^0] = 1;
            for (index_type i = 1; i < len; ++i) v_[i] = 0;
            return;
        }

        const real_type inv_norm_u = real_type{1} / norm_u;
        for (index_type i = 0; i < len; ++i)
            v_[i] *= inv_norm_u;
    }

    // Accesso al vettore v (solo lettura)
    const std::vector<real_type>& vector() const noexcept { return v_; }

    // Dimensione del riflettore
    index_type size() const noexcept { return v_.size(); }

    // ---------------------------
    // FASE DI APPLICAZIONE
    // ---------------------------

    // Applica H = I - 2 v v^T al vettore y (segmento in-place).
    // Si assume che y_segment corrisponda allo stesso sottospazio
    // usato in construct_from_segment (stessa dimensione len).
    void apply_to_segment(real_type* y, index_type len) const
    {
        assert(y    != nullptr);
        assert(len == v_.size());

        // beta = v^T y
        real_type beta = 0;
        for (index_type i = 0; i < len; ++i)
            beta += v_[i] * y[i];

        // y <- y - 2 beta v
        const real_type factor = 2 * beta;
        for (index_type i = 0; i < len; ++i)
            y[i] -= factor * v_[i];
    }

    // Versione che applica H a sinistra: y = H*y, su un vettore globale
    // dove il riflettore agisce solo sulle componenti k..n-1.
    void apply_left_to_vector(real_type* y, index_type n,
                              index_type offset) const
    {
        assert(y != nullptr);
        assert(offset + v_.size() <= n);

        // beta = v^T y_sub
        real_type beta = 0;
        for (index_type i = 0; i < v_.size(); ++i)
            beta += v_[i] * y[offset + i];

        const real_type factor = 2 * beta;
        for (index_type i = 0; i < v_.size(); ++i)
            y[offset + i] -= factor * v_[i];
    }

    // Applicazione H*A (a sinistra) su matrice memorizzata column-major:
    // A ha dimensioni m x n, riflettore agisce sulle righe offset .. offset+len-1.
    void apply_left_to_matrix(real_type* A,
                              index_type m, index_type n,
                              index_type row_offset) const
    {
        assert(A != nullptr);
        assert(row_offset + v_.size() <= m);

        // Per ogni colonna j:
        for (index_type j = 0; j < n; ++j) {

            // beta = v^T (colonna_j_sub)
            real_type beta = 0;
            real_type* col_j = A + j * m;

            for (index_type i = 0; i < v_.size(); ++i)
                beta += v_[i] * col_j[row_offset + i];

            const real_type factor = 2 * beta;

            for (index_type i = 0; i < v_.size(); ++i)
                col_j[row_offset + i] -= factor * v_[i];
        }
    }

    // Variante per applicazione a destra: A <- A * H^T (uguale a A*H, H simmetrica)
    // qui riflettore agisce sulle colonne col_offset .. col_offset+len-1.
    void apply_right_to_matrix(real_type* A,
                               index_type m, index_type n,
                               index_type col_offset) const
    {
        assert(A != nullptr);
        assert(col_offset + v_.size() <= n);

        // Per ogni riga i:
        for (index_type i = 0; i < m; ++i) {

            // beta = (row_i_sub) * v = sum_k A[i, col_offset+k] * v[k]
            real_type beta = 0;

            for (index_type k = 0; k < v_.size(); ++k)
                beta += A[i + (col_offset + k) * m] * v_[k];

            const real_type factor = 2 * beta;

            for (index_type k = 0; k < v_.size(); ++k)
                A[i + (col_offset + k) * m] -= factor * v_[k];
        }
    }

private:
    std::vector<real_type> v_; // vettore riflettente (normalizzato)
};
```

Questo rispecchia molto da vicino la struttura “$v,tau$” delle routine LAPACK `xLARFG` (generate) e `xLARF` (apply), solo che qui per semplicità uso il $v$ normalizzato e fisso $\tau = 2$.[^2][^3][^4][^1]

***

## Note didattiche da agganciare al codice

- In `construct_from_segment` si vede direttamente la scelta $\alpha = \mathrm{sign}(x_1)\|x\|$ e la costruzione $u = x + \alpha e_1$, $v = u/\|u\|$, esattamente come nelle note standard e nei testi di analisi numerica. [^6][^5][^1][^8][^9]
- In `apply_*` la formula $y \leftarrow y - 2(v^{T}y)v$ è chiaramente riconoscibile in due passi: prodotto scalare, poi outer product scalato.[^6][^5][^1]
- Le versioni `apply_left_to_matrix` e `apply_right_to_matrix` ti permettono di mostrare la differenza tra azione su righe e su colonne (si muove l’offset di riga vs l’offset di colonna), e si leggono come “aggiornamento di tutta la colonna/di tutte le colonne interessate” come nei pseudocodici di QR via Householder.[^8][^9][^1]

### TO DO


- prendere un vettore casuale,
- costruire il riflettore,
- verificare numericamente che la coda sia nulla e che $H$ sia ortogonale (via $H^{T}H\approx I$ ricostruito esplicitamente solo per piccole dimensioni).
<span style="display:none">[^10][^11][^12][^13][^14][^15][^16]</span>


<div align="center">⁂</div>

[^1]: https://www.math.chalmers.se/~larisa/www/NumLinAlg/Lecture9_2019.pdf

[^2]: https://www.netlib.org/lapack/explore-html/d7/d09/group__reflector__aux__grp.html

[^3]: https://www.netlib.org/lapack/explore-html/d2/d97/group__larf.html

[^4]: https://icl.utk.edu/~mgates3/lapack/html/d7/df4/group__reflector__aux.html

[^5]: https://en.wikipedia.org/wiki/Householder_transformation

[^6]: https://www.cs.cornell.edu/~bindel/class/cs6210-f12/notes/lec16.pdf

[^7]: https://elearning.di.unipi.it/mod/resource/view.php?id=2378

[^8]: https://ranger.uta.edu/~weems/NOTES4351/hansen.pdf

[^9]: https://perso.ensta.fr/~mbonnet/aln.pdf

[^10]: https://discourse.julialang.org/t/implementation-of-householder-reflection-matrices-with-efficient-multiplication/81257

[^11]: https://www.dealii.org/current/doxygen/deal.II/classHouseholder.html

[^12]: https://www.youtube.com/watch?v=gvnFVCWI044

[^13]: https://stackoverflow.com/questions/65112046/householder-qr-factorization-using-lapack-and-fortran

[^14]: https://stackoverflow.com/questions/60956390/performing-householder-reflection-of-a-vector-for-qr-decomposition

[^15]: https://github.com/chrundle/householder-QR

[^16]: https://www.youtube.com/watch?v=pOiOH3yESPM

\newpage 

# Householder in stile imperativo (C++ con stile ANSI C)

Questa nota didattica presenta il riflettore di Householder separando in modo esplicito la **fase di costruzione** dalla **fase di applicazione**, con codice in C++ scritto in stile imperativo, senza classi e con interfacce a puntatori e dimensioni esplicite.[^1][^2][^3][^4]

## Idea matematica

Un riflettore di Householder ha la forma

$$
H = I - 2vv^T
$$

con $\|v\|_2 = 1$. Applicato a un vettore $y$, produce

$$
Hy = y - 2(v^T y)v
$$

che è esattamente la formula da tradurre in codice: prima un prodotto scalare, poi una correzione lungo la direzione $v$.[^2][^5][^1]

Nel contesto della QR, dato un segmento di colonna $x$, si costruisce $v$ in modo che il riflettore mandi $x$ in un multiplo di $e_1$, cioè in un vettore con sola prima componente non nulla nel sottospazio attivo.[^6][^7][^1]

## Separazione didattica delle fasi

La struttura consigliata è la seguente:[^3][^4][^6]

1. **Costruzione**: da un target $x$ si determina il vettore riflettente $v$.
2. **Applicazione**: il vettore $v$ viene riusato per agire su vettori o matrici.

Questa separazione è concettualmente la stessa che compare nelle routine LAPACK per la generazione del riflettore (`DLARFG`) e per la sua applicazione (`DLARF`), ma scritta in C++ “piatto”.[^4][^8][^3]

## Fase di costruzione del vettore di Householder (fase geometrica)

Dato $x \in \mathbb{R}^m$, occorre generare $v$ tale che il riflettore $H = I - 2vv^{T}$ mandi $x$ in $\pm\|x\|_2 e_1$ e si usa la costruzione standard:[^1][^2][^6]

$$
\alpha = \operatorname{sign}(x_1)\|x\|_2
$$

$$
u = x + \alpha e_1
$$

$$
v = \frac{u}{\|u\|_2}
$$

La scelta del segno in $\alpha$ serve a evitare cancellazioni numeriche nella prima componente durante la costruzione del vettore riflettente.[^3][^1]

### Funzione di costruzione

```cpp
#include <cstddef>
#include <cmath>
#include <cassert>

// Costruisce il vettore v (dimensione len) a partire da x (segmento len).
// Risultato: v memorizza il vettore riflettente normalizzato.
// Convenzione: se x == 0, prendiamo v = e1.

void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v)
{
    assert(x   != nullptr);
    assert(v   != nullptr);
    assert(len > 0);

    // Copia x in v per manipolarlo
	
    std::size_t i;
    for (i = 0; i < len; ++i)
        v[i] = x[i];

    // Norma euclidea di x
	
    double norm_x2 = 0.0;
	
	// Riflettore identità (nel sottospazio) -> v = e1
		
    for (i = 0; i < len; ++i)
        norm_x2 += v[i] * v[i];

    if (norm_x2 == 0.0) {
        v[^0] = 1.0;
        for (i = 1; i < len; ++i)
            v[i] = 0.0;
        return;
    }

    const double norm_x = std::sqrt(norm_x2);

    // Scelta del segno: alpha = sign(x0) * ||x||

    const double x0     = v[^0];
    const double sign   = (x0 >= 0.0) ? 1.0 : -1.0;
    const double alpha  = sign * norm_x;

    // u = x + alpha e1  (scriviamo u direttamente in v)
	
    v[^0] += alpha;

    // Norma di u

    double norm_u2 = 0.0;
    for (i = 0; i < len; ++i)
        norm_u2 += v[i] * v[i];

    if (norm_u2 == 0.0) {

        // Caso teoricamente raro: ripieghiamo su e1.

        v[^0] = 1.0;
        for (i = 1; i < len; ++i)
            v[i] = 0.0;
        return;
    }

    const double norm_u = std::sqrt(norm_u2);
    const double inv_norm_u = 1.0 / norm_u;

    // v = u / ||u||

    for (i = 0; i < len; ++i)
        v[i] *= inv_norm_u;
}
```

### Osservazioni didattiche

- La funzione non costruisce mai esplicitamente la matrice $H$; costruisce solo il suo parametro essenziale, cioè il vettore $v$.[^5][^4][^1]
- Questo è il punto giusto per discutere perché l’approccio implicito sia migliore dell’espansione esplicita della matrice, sia per costo sia per chiarezza algoritmica.[^4][^6][^3]
- In caso di vettore nullo, si adotta la convenzione $v=e_1$, che rende il riflettore equivalente all’identità nel sottospazio attivo.[^2][^1]
- Qui si vedono *esplicitamente* le formule teoriche: $\alpha = \mathrm{sign}(x_1)\|x\|$, $u = x + \alpha e_1$, $v = u/\|u\|$. 

## Fase di applicazione a un vettore

Una volta costruito $v$, l’applicazione usa direttamente la formula

$$
y \leftarrow y - 2(v^T y)v
$$

che è il vero cuore computazionale del metodo.[^5][^1][^2]

### Funzione di applicazione a un segmento

```cpp

// Applica H = I - 2 v v^T ad un segmento di vettore y[0:len).

void householder_apply_to_segment(double*       y,
                                  std::size_t   len,
                                  const double* v)
{
    assert(y   != nullptr);
    assert(v   != nullptr);
    assert(len > 0);

    std::size_t i;

    // beta = v^T y

    double beta = 0.0;

    for (i = 0; i < len; ++i)
        beta += v[i] * y[i];

    const double factor = 2.0 * beta;

    // y <- y - 2 beta v

    for (i = 0; i < len; ++i)
        y[i] -= factor * v[i];
}
```

### Lettura algoritmica

L’algoritmo si legge in due passi molto puliti:[^1][^2]

1. proiezione scalare di $y$ su $v$, cioè $\beta = v^T y$;
2. sottrazione della componente riflessa, cioè $y \leftarrow y - 2\beta v$. E' la proiezione lungo $v$.

Questa è la forma più adatta per far vedere agli studenti che il riflettore non è una “scatola nera”, ma un’operazione elementare composta da prodotto scalare più aggiornamento vettoriale.[^5][^1]

## Applicazione a una matrice

Per la QR si applica il riflettore a sinistra alla matrice, cioè $A \leftarrow HA$, agendo sulle righe del sottoblocco attivo a partire dalla riga del pivot.[^7][^6]

### Versione left: $A \leftarrow HA$

Si assume memorizzazione **column‑major** (stile Fortran/LAPACK), dimensioni $m\times n$, riflettore che agisce su righe `row_offset .. row_offset+len-1`., coerente con le librerie numeriche classiche.[^3][^4]

Formula: su ogni colonna $j$, sostituisci il blocco di righe attive con $A_{sub} \leftarrow A_{sub} - 2v(v^{T}A_{sub})$.[^9][^6]

```cpp

// Applica H = I - 2 v v^T a sinistra: A <- H*A.
// A è m x n, column-major. Il riflettore agisce sulle righe
// row_offset .. row_offset+len-1 (sottospazio attivo).

void householder_apply_left_to_matrix(double*       A,
                                      std::size_t   m,
                                      std::size_t   n,
                                      std::size_t   row_offset,
                                      std::size_t   len,
                                      const double* v)
{
    assert(A != nullptr);
    assert(v != nullptr);
    assert(row_offset + len <= m);

    std::size_t j, i;

    // Loop sulle colonne

    for (j = 0; j < n; ++j) {
        double* col_j = A + j * m;

        // beta = v^T (sottovettore delle righe attive)

        double beta = 0.0;
        for (i = 0; i < len; ++i)
            beta += v[i] * col_j[row_offset + i];

        const double factor = 2.0 * beta;

        // Aggiornamento della parte attiva della colonna

        for (i = 0; i < len; ++i)
            col_j[row_offset + i] -= factor * v[i];
    }
}
```

Questa è la trascrizione diretta del passo interno dell’algoritmo di Householder QR: per ogni colonna dal pivot in poi, proiezione $v^{T}A_{sub}$ e sottrazione dell’outer product.[^10][^6][^9]

### Versione right: $A \leftarrow AH$

Questa versione è utile quando il riflettore va applicato sulle colonne, con $A \leftarrow A H$ (o $A H^{T}$, ma $H^{T}=H$), ad esempio in riduzioni bilaterali o in contesti diversi dalla QR elementare.[^2][^1]

```cpp

// Applica H = I - 2 v v^T a destra: A <- A*H.
// A è m x n, column-major. Il riflettore agisce sulle colonne
// col_offset .. col_offset+len-1.

void householder_apply_right_to_matrix(double*       A,
                                       std::size_t   m,
                                       std::size_t   n,
                                       std::size_t   col_offset,
                                       std::size_t   len,
                                       const double* v)
{
    assert(A != nullptr);
    assert(v != nullptr);
    assert(col_offset + len <= n);

    std::size_t i, k;

    // Loop sulle righe

    for (i = 0; i < m; ++i) {
	
        // beta = (row_i_sub) * v
    
		double beta = 0.0;

        for (k = 0; k < len; ++k)
            beta += A[i + (col_offset + k) * m] * v[k];

        const double factor = 2.0 * beta;

        // Aggiornamento delle colonne attive della riga i

        for (k = 0; k < len; ++k)
            A[i + (col_offset + k) * m] -= factor * v[k];
    }
}
```

## Esempio completo di test

Il seguente `main` costruisce il riflettore a partire dal vettore $(4,3,0)^T$, lo applica, e verifica che il risultato sia $(-5,0,0)^T$ a errore numerico vicino alla precisione macchina, come previsto dalla teoria.[^1][^2][^5]

```cpp
#include <cstdio>
#include <cstddef>
#include <cmath>
#include <cassert>

void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v);

void householder_apply_to_segment(double*       y,
                                  std::size_t   len,
                                  const double* v);

static void print_vector(const char* name,
                         const double* x,
                         std::size_t n)
{
    std::size_t i;
    std::printf("%s = [", name);
    for (i = 0; i < n; ++i) {
        std::printf("% .12f", x[i]);
        if (i + 1 < n) std::printf(", ");
    }
    std::printf("]\n");
}

int main()
{
    const std::size_t n = 3;
    double x[n] = {4.0, 3.0, 0.0};
    double v[n];
    double y[n];

    std::size_t i;
    for (i = 0; i < n; ++i)
        y[i] = x[i];

    householder_construct_segment(x, n, v);
    householder_apply_to_segment(y, n, v);

    print_vector("x", x, n);
    print_vector("v", v, n);
    print_vector("H*x", y, n);

    const double err_tail = std::sqrt(y[^1]*y[^1] + y[^2]*y[^2]);
    const double err_norm = std::fabs(std::sqrt(y[^0]*y[^0] + y[^1]*y[^1] + y[^2]*y[^2]) - 5.0);

    std::printf("errore coda     = %.12e\n", err_tail);
    std::printf("errore norma    = %.12e\n", err_norm);

    return 0;
}
```


### Cosa ci si aspetta in output

Per il vettore $x=(4,3,0)^T$, si ottiene $\|x\|_2 = 5$, quindi il riflettore deve portare il vettore in $(-5,0,0)^T$ oppure $(5,0,0)^T$ a seconda della convenzione di segno adottata nella costruzione.[^1][^2][^5]

Con la convenzione usata qui, il risultato atteso è vicino a

```text
x   = [ 4.000000000000,  3.000000000000,  0.000000000000]
v   = [ 0.948683298051,  0.316227766017,  0.000000000000]
H*x = [-5.000000000000,  0.000000000000,  0.000000000000]
```

Le quantità `errore coda` ed `errore norma` devono risultare molto piccole, tipicamente dell’ordine di `1e-15` o vicino, a seconda della piattaforma e del compilatore.[^5][^1]

## Compilazione minima

Un esempio minimale di compilazione può essere:[^6]

```bash
g++ -O2 -std=c++17 householder_test.cpp -o householder_test
./householder_test
```

## Punti didattici da fissare (principio “costruzione vs applicazione”)

- **Definiamo la trasformazione a livello matematico**:
	$H = I - 2vv^{T}$, $Hx = x - 2(v^{T}x)v$.[^2][^6]
- **Deriviamo la scelta di $v$** a partire da $x$ (target) come in `householder_construct_segment`.[^6][^1]
- Il riflettore **non si memorizza come matrice**, ma tramite il suo vettore generatore $v$.[^4][^3]
- La distinzione tra costruzione e applicazione è essenziale, sia teoricamente sia computazionalmente.[^3][^4]
- La formula operativa è sempre la stessa: prodotto scalare più aggiornamento rank‑1 del blocco attivo.[^4][^1][^5]
- In QR, il sottospazio attivo cambia a ogni passo: il riflettore opera solo sulla coda della colonna corrente.[^7][^6]


## TO DO main di test che:

- genera un vettore,
- costruisce `v` con `householder_construct_segment`,
- applica il riflettore e verifica numericamente che il risultato ha solo la prima componente non nulla e norma conservata, così da chiudere il cerchio teoria‑codice‑verifica.
<span style="display:none">[^12][^13][^14][^15]</span>

Incapsulare queste stesse funzioni in una routine `qr_householder(...)` che, colonna dopo colonna, costruisce il vettore riflettente sulla coda attiva e lo applica al sottoblocco rimanente della matrice.[^6][^7][^4]

<div align="center">⁂</div>

[^1]: https://www.cs.cornell.edu/~bindel/class/cs6210-f12/notes/lec16.pdf

[^2]: https://en.wikipedia.org/wiki/Householder_transformation

[^3]: https://gensoft.pasteur.fr/docs/lapack/3.9.0/d8/d9b/group__double_o_t_h_e_rauxiliary_gaabb59655e820b3551af27781bd716143.html

[^4]: https://www.netlib.org/lapack/explore-html/d2/d97/group__larf.html

[^5]: https://www.math.chalmers.se/~larisa/www/NumLinAlg/Lecture9_2019.pdf

[^6]: https://student.cs.uwaterloo.ca/~cs475/CS475-Lecture12.pdf

[^7]: https://en.wikipedia.org/wiki/QR_decomposition

[^8]: https://www.netlib.org/lapack/explore-html/d8/d0d/group__larfg.html

[^9]: https://receiptroller.co/en/technotes/p/householder-reflections-qr-decomposition-numerical-linear-algebra

[^10]: https://en.wikipedia.org/wiki/QR_decomposition

[^11]: https://www.studocu.vn/vn/document/truong-dai-hoc-bach-khoa-dai-hoc-quoc-gia-thanh-pho-ho-chi-minh/linear-algebra/qr-decomposition-using-householder-transformations-cc08-project-report/144883875

[^12]: https://www.youtube.com/watch?v=gvnFVCWI044

[^13]: https://stackoverflow.com/questions/65112046/householder-qr-factorization-using-lapack-and-fortran

[^14]: https://www.appletonaudio.com/blog/2020/an-attempt-at-an-intuitive-description-of-the-qr-decomposition-using-householder-reflectors/

[^15]: https://discourse.julialang.org/t/implementation-of-householder-reflection-matrices-with-efficient-multiplication/81257


\newpage


## QR completa via Householder

In stile imperativo, molto vicino agli schemi classici delle note su Householder‑QR.[^1][^2][^3][^4]

- uno schema algoritmico;
- una versione C++ “alla C” che usa le funzioni `householder_construct_segment` e `householder_apply_left_to_matrix` ;
- qualche nota su come confrontarla con la bidiagonalizzazione usata per l’SVD.[^3][^5][^6][^7]

***

## 1. Schema algoritmico Householder‑QR

Per $A \in \mathbb{R}^{m\times n}$, $m \ge n$, vogliamo $A = QR$ con $Q$ ortogonale e $R$ triangolare superiore.[^2][^1][^3]

Algoritmo (colonna per colonna):[^4][^5][^3]

1. Per $k = 0,1,\dots,n-1$:
    - consideri il **sottoblocco attivo** $A_{k:m-1,\;k:n-1}$;
    - estrai la **coda della colonna k**: $x = A_{k:m-1,\;k}$;
    - costruisci il vettore di Householder $v_k$ che manda $x$ in $\pm \|x\| e_1$;
    - applichi il riflettore a sinistra al sottoblocco;
    - memorizzi $v_k$ (tipicamente dentro la stessa matrice $A$).


### Algoritmo colonna per colonna con sottoblocco attivo

Alla iterazione $k$ (con $k = 0,1,\dots,n-1$), la matrice ha già le prime $k$ colonne in forma triangolare superiore; il resto è ancora “grezzo”.[^4][^3]

La situazione a step $k$ può essere schematizzata come:

```text
                colonne
          0      1      2          k        k+1             n-1
        ---------------------------------------------------------
riga 0 |  *      *      *   ...   *        *      ...        *
riga 1 |  0      *      *   ...   *        *      ...        *
riga 2 |  0      0      *   ...   *        *      ...        *
  ...  |  .      .      .          .        .                 .
riga k |  0      0      0   ...   [x]      ?      ...        ?
riga k+1| 0      0      0   ...   [x]      ?      ...        ?
  ...  |  .      .      .          .        .                 .
riga m-1|0      0      0   ...   [x]      ?      ...        ?
```

Dove:

- il blocco tra parentesi quadre `[...]` nella colonna `k` rappresenta il **vettore attivo**

$$
x = A_{k:m-1,\;k},
$$
- il rettangolo che parte da riga `k`, colonna `k` e arriva in basso a destra è il **sottoblocco attivo**

$$
A_{k:m-1,\;k:n-1}.
$$

Più esplicitamente, separando per blocchi:

```text
A =
[  R_{k×k}      *              ]
[  0_{(m-k)×k}  A_{k:m-1,k:n-1}]
```

dove $R_{k\times k}$ è già triangolare superiore (prime $k$ colonne “finite”), e $A_{k:m-1,\;k:n-1}$ è il sottoblocco su cui agiamo alla iterazione $k$.[^17][^3]

***

**Passo $k$:**

1. **Estrazione della coda della colonna $k$**
Prendiamo il vettore colonna a partire dalla riga $k$:

$$
x = A_{k:m-1,\;k}.
$$

Graficamente è il segmento verticale marcato `[x]`:

```text
riga k   | ... [x]
riga k+1 | ... [x]
...
riga m-1 | ... [x]
```

2. **Costruzione del vettore di Householder $v_k$**
Costruiamo $v_k \in \mathbb{R}^{m-k}$ che manda $x$ in $\pm\|x\| e_1$; questo è il passo di “design” del riflettore:

$$
H_k = I - 2 v_k v_k^{T}, \quad H_k x = \pm \|x\| e_1.
$$
3. **Applicazione del riflettore al sottoblocco attivo**
Applichiamo $H_k$ a sinistra al sottoblocco inferiore destro:

$$
A_{k:m-1,\;k:n-1} \leftarrow H_k \, A_{k:m-1,\;k:n-1}.
$$

Graficamente, agiamo solo sul rettangolo:

```text
             colonne
           k        k+1             n-1
         -------------------------------
riga k   |  [ * ]    [ * ]   ...      [ * ]
riga k+1 |  [ 0 ]    [ * ]   ...      [ * ]
...    |   .         .                .
riga m-1 |  [ 0 ]    [ * ]   ...      [ * ]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
sottoblocco attivo

```

Dopo l’applicazione, la colonna \(k\) nel sottoblocco ha zeri sotto la diagonale; le colonne da \(k+1\) in poi sono state aggiornate coerentemente.[^3][^17][^4]

4. **Memorizzazione di \(v_k\)**  
La parte triangolare superiore aggiornata della colonna \(k\) contiene ora l’elemento diagonale di \(R\), mentre la parte sotto la diagonale può essere usata per memorizzare la **coda** del vettore \(v_k\) (le componenti \(2,\dots,m-k\) nel sottospazio).[^17][^3]

***

**Passo \(k+1\):**

Alla iterazione successiva, la struttura della matrice è:

```text
             colonne
       0      1      ...   k      k+1          n-1
     ------------------------------------------------
riga 0 |  *      *    ...    *      *     ...      *
riga 1 |  0      *    ...    *      *     ...      *
...  |  .      .           .      .              .
riga k |  0      0    ...    *      *     ...      *
riga k+1|0      0    ...    0     [x]    ...      ?
...  |  .      .           .      .             .
riga m-1|0      0    ...    0     [x]    ...      ?
```

cioè:

- le colonne $0,\dots,k$ sono ormai in forma “quasi finale”: triangolare superiore nella parte rilevante;
- il nuovo **sottoblocco attivo** è ora

$$
A_{k+1:m-1,\;k+1:n-1},
$$

e la nuova **coda** su cui costruire $v_{k+1}$ è

$$
x = A_{k+1:m-1,\;k+1}.
$$

Si ripete lo stesso schema:

1. estrazione di $x = A_{k+1:m-1,\;k+1}$;
2. costruzione di $v_{k+1}$;
3. applicazione di $H_{k+1}$ al sottoblocco attivo $A_{k+1:m-1,\;k+1:n-1}$;
4. memorizzazione della coda di $v_{k+1}$ sotto la diagonale nella colonna $k+1$.[^4][^3][^17]

Iterando fino a $k = n-1$, il sottoblocco attivo si restringe sempre di più nell’angolo in basso a destra, fino a ridursi a una dimensione triviale; la matrice intera è così trasformata in una triangolare superiore $R$, con i vettori $v_k$ “nascosti” sotto la diagonale e pronti per ricostruire o applicare $Q$.[^2][^1][^3]
<span style="display:none">[^16][^14][^22][^23][^24][^25][^26][^18][^19][^20][^21]</span>

<div align="center">⁂</div>


### Alla fine:

- $R$ è la parte superiore di $A$ modificata;
- $Q$ si ottiene come prodotto $Q = H_0 H_1 \cdots H_{n-1}$, dove $H_k = I - 2 v_k v_k^{T}$ esteso all’identità fuori dal sottospazio attivo.[^6][^1][^3]

Per uso pratico spesso si memorizzano solo i vettori $v_k$ e un array di scalari, e si applica $Q$ implicitamente; ma per il confronto didattico probabilmente ti interessa anche una funzione che costruisce $Q$ esplicito.[^8][^1]

***

## 2. QR via Householder in stile imperativo

Assumo:

- `A` in column‑major, dimensioni `m x n`;
- riuso della matrice `A` per contenere:
    - sopra la diagonale: $R$;
    - sotto la diagonale: “code” dei vettori $v_k$;
- un array `tau` opzionale se un domani vuoi passare alla forma LAPACK‑like $H_k = I - \tau_k v_k v_k^{T}$; qui possiamo anche fissare $\tau_k = 2.0$ dato che $v_k$ è normalizzato.[^9][^1][^3]


### Funzioni ausiliarie (già viste)

Uso le versioni che abbiamo costruito prima:[^3][^4]

```cpp
void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v);

void householder_apply_left_to_matrix(double*       A,
                                      std::size_t   m,
                                      std::size_t   n,
                                      std::size_t   row_offset,
                                      std::size_t   len,
                                      const double* v);
```


### Routine `qr_householder_inplace`

```cpp
#include <cstddef>
#include <cmath>
#include <cassert>

// A: m x n, column-major
// Output:
//   - parte triangolare superiore di A contiene R
//   - parte strettamente sotto-diagonale di A contiene le "code" dei vettori v_k
//   - opzionalmente si può riempire tau[k] = 2.0 (se v è normalizzato)
void qr_householder_inplace(double*     A,
                            std::size_t m,
                            std::size_t n)
{
    assert(A != nullptr);
    std::size_t k, i;

    const std::size_t K = (m < n) ? m : n;

    for (k = 0; k < K; ++k) {

        // Dimensione del sottospazio attivo (coda da riga k a m-1)
        const std::size_t len = m - k;

        // Puntatore all'inizio del segmento x = A(k:m-1, k)
        double* col_k = A + k * m;
        double* x     = col_k + k;

        // Costruisci il vettore v di dimensione len
        double vbuf[/*max_len*/ 1024]; // per didattica: in pratica usare alloc dinamica
        assert(len <= 1024);           // o usare std::vector se permesso

        householder_construct_segment(x, len, vbuf);

        // Applica il riflettore H_k al sottoblocco A(k:m-1, k:n-1)
        // riutilizzando householder_apply_left_to_matrix su un sotto-array.
        // Per farlo, passiamo A_sub = A + k (offset di riga), ma mantenendo
        // la stessa leading dimension m. L'offset di riga interno sarà 0.
        householder_apply_left_to_matrix(A + k,  // inizio riga k di colonna 0
                                         m - k, // nuove "m" locali
                                         n - k, // nuove "n" locali
                                         0,     // row_offset relativo al sottoblocco
                                         len,   // = m-k
                                         vbuf);

        // Ora vogliamo memorizzare la coda di vbuf nella colonna k di A
        // secondo la convenzione classica:
        // A(k,k) = norma firmata, A(k+1:m-1,k) = componenti 2..len di v (o simili).
        // Per mantenere un confronto semplice con la teoria,
        // possiamo scrivere:
        //
        // - col_k[k] = valore aggiornato (pivot R)
        // - col_k[k+1..m-1] = vbuf[1..len-1]
        //
        // così la colonna k contiene in basso la coda del vettore di Householder.
        for (i = 1; i < len; ++i)
            col_k[k + i] = vbuf[i];
    }
}
```

Note:

- Ho usato un buffer statico `vbuf[^1024]` solo per non introdurre `std::vector`; ovviamente in una versione reale metteresti un’allocazione dinamica o `std::vector<double> v(len);`.
- La logica è esattamente quella delle note di QR: alla colonna `k` memorizzi la parte strettamente sotto‐diagonale del vettore riflettente.[^5][^1][^3]

***

## 3. Costruzione esplicita di Q (per confronto didattico)

Per confrontarti con il vecchio lavoro empirico e con l’uso nella bidiagonalizzazione può tornare utile **formare Q esplicito**, anche se in pratica spesso lo si usa solo implicitamente.[^1][^8][^6]

L’idea standard:[^1][^3]

- dopo `qr_householder_inplace`, la matrice `A` contiene:
    - in alto R;
    - sotto la diagonale, per ogni colonna `k`, la coda di `v_k` (con $v_{k,0} = 1$ per convenzione implicita).
- per ottenere $Q$, parti da $Q = I_m$ e applica **all’indietro** i riflettori memorizzati:
$Q \leftarrow H_{K-1} \cdots H_1 H_0 Q$.

Esempio di funzione:

```cpp
// Ricostruisce Q esplicito a partire da A modificata da qr_householder_inplace.
// Q: matrice m x m, column-major, da riempire (inizialmente identità).
void qr_form_Q_from_householder(const double* A,
                                std::size_t   m,
                                std::size_t   n,
                                double*       Q)
{
    assert(A != nullptr);
    assert(Q != nullptr);

    std::size_t i, j, k;
    const std::size_t K = (m < n) ? m : n;

    // Inizializza Q all'identità
    for (j = 0; j < m; ++j) {
        for (i = 0; i < m; ++i)
            Q[i + j * m] = (i == j) ? 1.0 : 0.0;
    }

    // Applica i riflettori H_k in ordine inverso: k = K-1, ..., 0
    for (std::size_t kk = 0; kk < K; ++kk) {
        k = K - 1 - kk;  // k discendente

        const std::size_t len = m - k;

        // Ricostruisci il vettore v_k di dimensione len a partire da A
        double vbuf[^1024];
        assert(len <= 1024);

        // Convenzione: v_k[^0] = 1, v_k[i] = A[k+1+i-1, k] per i>=1
        vbuf[^0] = 1.0;
        const double* col_k = A + k * m;

        for (i = 1; i < len; ++i)
            vbuf[i] = col_k[k + i];

        // Applica H_k a sinistra a Q(k:m-1, :)
        // cioè al sottoblocco di Q di dimensione (m-k) x m
        // Per farlo, usiamo householder_apply_left_to_matrix
        householder_apply_left_to_matrix(Q + k,  // inizio riga k di Q
                                         m - k, // m locale
                                         m,     // n locale (tutte le colonne di Q)
                                         0,     // row_offset relativo
                                         len,
                                         vbuf);
    }
}
```

Questa routine è quasi una traduzione diretta degli pseudocodici MATLAB per “apply Q from compact Householder form”, con le dovute semplificazioni (tau=2, v normalizzati).[^8][^5][^1]

***

## 4. Micro‑test QR (per chiudere il giro)

Puoi ora scrivere un `main` che:

1. costrui sce una piccola matrice (anche la stessa usata per le prove di bidiagonalizzazione);
2. chiama `qr_householder_inplace`;
3. ricostruisce Q con `qr_form_Q_from_householder`;
4. verifica numericamente:
    - $A \approx Q R$;
    - $Q^{T}Q \approx I$.

Gli schemi di verifica sono gli stessi che si usano nei testi di QR/SVD.[^10][^2][^6]

***

## 5. Collegamento con la bidiagonalizzazione SVD

Confrontando questo QR con la tua bidiagonalizzazione Householder per SVD puoi far emergere in modo molto chiaro la struttura comune:[^11][^12][^7]

- QR: *solo* riflettori a sinistra, colonna dopo colonna, per rendere la matrice triangolare superiore.[^4][^3]
- Bidiagonalizzazione: riflettori a sinistra per annullare la coda della colonna, riflettori a destra per annullare la coda della riga successiva; si alternano sinistra/destra.[^12][^7][^11]

A livello di codice, la differenza principale è solo la combinazione di `householder_apply_left_to_matrix` e `householder_apply_right_to_matrix` in sequenza, seguendo esattamente lo schema Golub–Kahan.[^7][^12]

<span style="display:none">[^13][^14][^15][^16]</span>

<div align="center">⁂</div>

[^1]: https://www.cs.cornell.edu/~bindel/class/cs6210-f09/lec18.pdf

[^2]: https://en.wikipedia.org/wiki/QR_decomposition

[^3]: https://student.cs.uwaterloo.ca/~cs475/CS475-Lecture12.pdf

[^4]: https://receiptroller.co/en/technotes/p/householder-reflections-qr-decomposition-numerical-linear-algebra

[^5]: https://byumcl.bitbucket.io/bootcamp2014/_downloads/Lab13v1.pdf

[^6]: https://blogs.mathworks.com/cleve/2016/10/03/householder-reflections-and-the-qr-decomposition/

[^7]: https://www.math.iit.edu/~fass/477577_Chapter_12.pdf

[^8]: https://discourse.julialang.org/t/in-place-computation-of-q-from-qr-decomposition/90881

[^9]: https://gensoft.pasteur.fr/docs/lapack/3.9.0/d8/d9b/group__double_o_t_h_e_rauxiliary_gaabb59655e820b3551af27781bd716143.html

[^10]: https://people.inf.ethz.ch/gander/papers/qrneu.pdf

[^11]: https://www.active-analytics.com/blog/householder-bigiag-in-d/

[^12]: https://people.inf.ethz.ch/gander/talks/SVDnew.pdf

[^13]: https://www.youtube.com/watch?v=n0zDgkbFyQk

[^14]: https://epubs.siam.org/doi/10.1137/22M1544555

[^15]: https://www.reddit.com/r/LinearAlgebra/comments/1ts997e/householder_reflection_qr_decomposition/

[^16]: https://www.scribd.com/document/880365358/ln10

[^17]: https://www.scribd.com/document/880365666/LecN8

[^18]: https://www.osti.gov/servlets/purl/1236219

[^19]: https://ejml.org/javadoc/org/ejml/dense/row/decompose/qr/QRDecompositionHouseholderColumn_ZDRM.html

[^20]: https://www.datacamp.com/fr/tutorial/qr-decomposition

[^21]: https://gist.github.com/jakelevi1996/512fd1c1291edb07b4242574151088de

[^22]: https://www.math.ucla.edu/~yanovsky/Teaching/Math151B/handouts/GramSchmidt.pdf

[^23]: https://stackoverflow.com/questions/36637322/qr-decomposition-in-r-forcing-a-positive-diagonal

[^24]: https://www.sciencedirect.com/science/article/pii/S0893965999000282

[^25]: https://graphsearch.epfl.ch/en/concept/305223

[^26]: https://www.cs.utexas.edu/~flame/laff/alaff/chapter03-blocked.html



\newpage


## Applicazione a destra: sottoblocco attivo sulle colonne/righe

Supponi di essere dopo il riflettore sinistro $U_k$ del passo $k$ (quello che annulla gli elementi sotto $a_{kk}$); ora vuoi applicare un riflettore destro $V_k$ per eliminare gli elementi a destra di $a_{k,k+1}$ nella riga $k$.[^2][^3][^1]

La situazione tipica (per $m \ge n$) dopo il **riflettore sinistro** al passo $k$ è:

```text
                    colonne
        0      1      ...     k      k+1        k+2         n-1
      -----------------------------------------------------------
riga 0 | *      *     ...     *       *          *          *
riga 1 | 0      *     ...     *       *          *          *
  ...  | .      .             .       .          .          .
riga k | 0      0     ...     *      [x]       [x]  ...    [x]
riga k+1|0      0     ...     0       *          *          *
  ...  | .      .             .       .          .          .
riga m-1|0      0     ...     0       *          *          *
```

A sinistra della colonna $k+1$, la riga $k$ ha già la forma desiderata; a destra, vogliamo trasformare la parte

$$
x = A_{k,\;k+1:n-1}
$$

in un vettore con una sola componente non nulla in posizione $k+1$.[^3][^2]

Più schematicamente, separando a blocchi:

```text
A =
[  *           *                ]
[  0_{(k)×(k+1)}   *           ]
[  0       a_{k,k+1}   x_tail  ]
[  0           *       *       ]
```

dove $x_\text{tail}$ è la parte da azzerare con un riflettore applicato **a destra** (cioè sulle colonne).[^2][^3]

***

### Passo destro al livello $k$

1. **Estrazione della coda di riga attiva**

Consideriamo la riga $k$ a partire dalla colonna $k+1$:

$$
x = A_{k,\;k+1:n-1}.
$$

Graficamente:

```text
riga k | ... a_{k,k}   [x]  [x]  ...  [x]
              ^        ^           ^
              |        \--- coda da azzerare ---/
```

Questo è il segmento che vogliamo trasformare in $\pm\|x\| e_1$ nel sottospazio orizzontale. [^3][^2][^1]
2. **Costruzione del vettore di Householder destro $w_k$**

Costruiamo un vettore $w_k \in \mathbb{R}^{n-(k+1)}$ (dimensione: numero di colonne da $k+1$ a $n-1$) tale che il riflettore

$$
V_k = I - 2\,w_k w_k^{T}
$$

soddisfi

$$
x V_k = \pm \|x\| e_1.
$$

È esattamente la stessa formula di costruzione usata per il riflettore sinistro, ma applicata al vettore riga $x$ invece che al vettore colonna; in implementazione si usa la stessa routine `householder_construct_segment` ma lavorando sulle colonne di $A$ (perché $A$ è column‑major).[^4][^3][^2]
3. **Applicazione del riflettore a destra al sottoblocco attivo**

Ora applichiamo $V_k$ a destra sul **sottoblocco attivo sulle colonne**:

$$
A_{k:m-1,\;k+1:n-1} \leftarrow A_{k:m-1,\;k+1:n-1} V_k.
$$

Questo significa che:
    - agiamo su tutte le righe dalla $k$-esima in giù;
    - modifichiamo solo le colonne da $k+1$ a $n-1$, cioè il “rettangolo” a destra del pivot corrente.[^1][^3][^2]

Graficamente:

```text
                 colonne
            k      k+1          k+2         n-1
          ----------------------------------------

riga k   |   *    [ * ]        [ * ]  ...    [ * ]
riga k+1 |   0    [ * ]        [ * ]  ...    [ * ]
...    |   .      .            .             .
riga m-1 |   0    [ * ]        [ * ]  ...    [ * ]
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
sottoblocco attivo destro

```

Dopo l’applicazione:

- la riga \(k\) ha zeri a destra tranne in colonna \(k+1\);  
- il sottoblocco sottostante è stato aggiornato coerentemente.[^5][^3][^2]

4. **Memorizzazione di \(w_k\)**

In maniera completamente analoga al lato sinistro, si memorizza la **coda** del vettore \(w_k\) nelle posizioni “inutili” della riga \(k\), cioè nelle colonne \(k+2,\dots,n-1\), lasciando \(A_{k,k+1}\) come elemento di bidiagonale:[^3][^1][^2]

- \(A_{k,k+1}\) diventa l’elemento della superdiagonale della matrice bidiagonale;  
- \(A_{k,k+2},\dots,A_{k,n-1}\) contengono le componenti \(2,\dots\) del vettore riflettente \(w_k\).

***

### Passo successivo (livello \(k+1\))

Dopo aver applicato **sia** il riflettore sinistro \(U_k\) **sia** il riflettore destro \(V_k\) al passo \(k\), la matrice ha questa struttura:[^2][^3]

```text
                 colonne
     0      1     ...     k      k+1      k+2        n-1
   --------------------------------------------------------
riga 0 | *      *    ...     *      *         0   ...   0
riga 1 | 0      *    ...     *      *         0   ...   0
...  | .      .            .      .         .        .
riga k | 0      0    ...     *      *         0   ...   0
riga k+1|0      0    ...     0     [x]       [x]  ...  [x]
...  | .      .            .      .         .        .
riga m-1|0      0    ...     0     [x]       [x]  ...  [x]
```

- le prime $k$ righe e colonne hanno già la forma bidiagonale desiderata nelle loro posizioni pertinenti;
- la prossima **colonna attiva** è la $k+1$ (per il riflettore sinistro successivo);
- la prossima **riga attiva** sarà la $k+1$, a partire dalla colonna $k+2$ (per il riflettore destro successivo).[^1][^3][^2]

Il ciclo completo di bidiagonalizzazione Householder (Golub–Kahan) si riassume quindi così:[^6][^3][^2]

1. Al passo $k$:
    - riflettore sinistro $U_k$ su $A_{k:m-1,\;k:n-1}$ per annullare la coda di colonna $k$;
    - riflettore destro $V_k$ su $A_{k:m-1,\;k+1:n-1}$ per annullare la coda della riga $k$ (dal colonna $k+2$ in poi).
2. Il sottoblocco attivo si restringe alla parte $A_{k+1:m-1,\;k+2:n-1}$ per il passo successivo.

Iterando, ottieni $U^{T} A V = B$ con $B$ bidiagonale superiore, e i vettori di Householder di sinistra e di destra sono memorizzati rispettivamente sotto la diagonale e sopra la superdiagonale, in maniera del tutto analoga al caso QR/LQ.[^7][^6][^3][^2]

Puoi prendere questa sezione e inserirla accanto alla descrizione del QR, così hai i due “disegni” speculari (sinistra e destra) per confrontare direttamente il tuo codice di bidiagonalizzazione con la versione teorico‑didattica.

VEdiamo un esempio completo di `main` QR (piccola matrice 3×3/4×3) con stampa di $A$, $Q$, $R$, e residui numerici da confrontare con il  vecchio codice SVD_QR ga' scritto.

<span style="display:none">[^10][^11][^12][^13][^14][^15][^8][^9]</span>

<div align="center">⁂</div>

[^1]: https://lup.lub.lu.se/student-papers/record/9112096/file/9112097.pdf

[^2]: https://en.wikipedia.org/wiki/Bidiagonalization

[^3]: https://www.math.iit.edu/~fass/477577_Chapter_12.pdf

[^4]: https://en.wikipedia.org/wiki/Householder_transformation

[^5]: https://github.com/emchinn/Bidiagonalization/blob/master/Golub-Kahan.ipynb

[^6]: https://www.geometrictools.com/GTE/Mathematics/SingularValueDecomposition.h

[^7]: https://www.netlib.org/lapack/lawnspdf/lawn209.pdf

[^8]: https://www.youtube.com/watch?v=gvnFVCWI044

[^9]: https://arxiv.org/html/2510.12705v2

[^10]: https://www.math.uri.edu/~jbaglama/papers/paper17.pdf

[^11]: https://qucs.sourceforge.net/tech/node99.html

[^12]: https://slepc.upv.es/release/_downloads/a3d876fe442fff13fd6b8ed1ccd75b08/str8.pdf

[^13]: http://www.fp.tul.cz/~plesinger/my_publications/conference/GAMM_SIAM_DUS06.pdf

[^14]: https://www.cs.umd.edu/users/oleary/reprints/j11.pdf

[^15]: https://www.netlib.org/utk/people/JackDongarra/etemplates/node198.html

\newpage


Vediamo un **main completo** che:

- costruisce una 4×3 semplice ma non banale;
- fa QR in‑place con `qr_householder_inplace`;
- ricostruisce $Q$ con `qr_form_Q_from_householder`;
- stampa $A$, $Q$, $R$ e le norme dei residui $\|A-QR\|_F$ e $\|Q^{T}Q-I\|_F$. [^1][^2][^3][^4]

Puoi incollare questo blocco nella tua “dispensa QR” accanto alle funzioni già scritte.

***

## Esempio completo di main QR (4×3, Householder imperativo)

Assumo che nel tuo progetto tu abbia già definito:

- `householder_construct_segment`
- `householder_apply_left_to_matrix`
- `qr_householder_inplace`
- `qr_form_Q_from_householder`

come nelle sezioni precedenti.[^1][^2]

### File di esempio: `qr_householder_test.cpp`

```cpp
#include <cstdio>
#include <cstddef>
#include <cmath>
#include <cassert>

// Dichiarazioni delle funzioni (implementazioni in altri file/precedenti)
void householder_construct_segment(const double* x,
                                   std::size_t    len,
                                   double*        v);

void householder_apply_left_to_matrix(double*       A,
                                      std::size_t   m,
                                      std::size_t   n,
                                      std::size_t   row_offset,
                                      std::size_t   len,
                                      const double* v);

void qr_householder_inplace(double*     A,
                            std::size_t m,
                            std::size_t n);

void qr_form_Q_from_householder(const double* A,
                                std::size_t   m,
                                std::size_t   n,
                                double*       Q);

// Funzioni di utilità per stampare e calcolare norme Frobenius

static void print_matrix(const char* name,
                         const double* A,
                         std::size_t m,
                         std::size_t n)
{
    std::printf("%s =\n", name);
    for (std::size_t i = 0; i < m; ++i) {
        std::printf("  [");
        for (std::size_t j = 0; j < n; ++j) {
            std::printf(" % .10f", A[i + j * m]);
            if (j + 1 < n) std::printf(",");
        }
        std::printf(" ]\n");
    }
    std::printf("\n");
}

static double frobenius_norm(const double* A,
                             std::size_t m,
                             std::size_t n)
{
    double sum = 0.0;
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            double a = A[i + j * m];
            sum += a * a;
        }
    }
    return std::sqrt(sum);
}

// C = A * B   (A: m×k, B: k×n), column-major
static void matmul(const double* A,
                   const double* B,
                   double*       C,
                   std::size_t   m,
                   std::size_t   k,
                   std::size_t   n)
{
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            double sum = 0.0;
            for (std::size_t p = 0; p < k; ++p)
                sum += A[i + p * m] * B[p + j * k];
            C[i + j * m] = sum;
        }
    }
}

// C = A^T * B   (A: m×k, B: m×n), result C: k×n
static void matmul_AT_B(const double* A,
                        const double* B,
                        double*       C,
                        std::size_t   m,
                        std::size_t   k,
                        std::size_t   n)
{
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t p = 0; p < k; ++p) {
            double sum = 0.0;
            for (std::size_t i = 0; i < m; ++i)
                sum += A[i + p * m] * B[i + j * m];
            C[p + j * k] = sum;
        }
    }
}

// C = A - B   (stessa dimensione)
static void mat_subtract(const double* A,
                         const double* B,
                         double*       C,
                         std::size_t   m,
                         std::size_t   n)
{
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            C[i + j * m] = A[i + j * m] - B[i + j * m];
        }
    }
}

int main()
{
    // Matrice di test 4x3 (column-major):
    // A =
    // [ 12   -51    4 ]
    // [  6    167  -68]
    // [ -4    24   -41]
    // [ -1     1    0 ]
    //
    // (è un classico esempio didattico di QR via Householder)
    const std::size_t m = 4;
    const std::size_t n = 3;

    double A[m * n] = {
        // colonna 0
        12.0,  6.0, -4.0, -1.0,
        // colonna 1
        -51.0, 167.0, 24.0, 1.0,
        // colonna 2
        4.0,  -68.0, -41.0, 0.0
    };

    // Copia di A per controllo residuo A - Q*R
    double A_orig[m * n];
    for (std::size_t j = 0; j < n; ++j)
        for (std::size_t i = 0; i < m; ++i)
            A_orig[i + j * m] = A[i + j * m];

    std::printf("=== QR Householder (esempio 4x3) ===\n\n");
    print_matrix("A (originale)", A_orig, m, n);

    // Fattorizzazione in-place: A -> [R; vettori di Householder sotto la diag]
    qr_householder_inplace(A, m, n);

    // Estrai R: parte triangolare superiore di A
    double R[m * n];
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < m; ++i) {
            if (i <= j)
                R[i + j * m] = A[i + j * m];
            else
                R[i + j * m] = 0.0;
        }
    }

    // Costruisci Q esplicito (m x m)
    double Q[m * m];
    qr_form_Q_from_householder(A, m, n, Q);

    print_matrix("R (triangolare sup.)", R, m, n);
    print_matrix("Q (ortogonale)", Q, m, m);

    // Calcola residuo A_orig - Q*R
    double QR[m * n];
    matmul(Q, R, QR, m, m, n);

    double RES[m * n];
    mat_subtract(A_orig, QR, RES, m, n);

    double norm_A   = frobenius_norm(A_orig, m, n);
    double norm_RES = frobenius_norm(RES, m, n);

    std::printf("||A||_F         = %.12e\n", norm_A);
    std::printf("||A - Q*R||_F   = %.12e\n", norm_RES);
    std::printf("rel. residuo    = %.12e\n\n", norm_RES / norm_A);

    // Verifica ortogonalità: Q^T Q ≈ I
    double QTQ[m * m];
    matmul_AT_B(Q, Q, QTQ, m, m, m);

    // Costruisci identità I
    double I[m * m];
    for (std::size_t j = 0; j < m; ++j)
        for (std::size_t i = 0; i < m; ++i)
            I[i + j * m] = (i == j) ? 1.0 : 0.0;

    double QTQmI[m * m];
    mat_subtract(QTQ, I, QTQmI, m, m);

    double norm_QTQmI = frobenius_norm(QTQmI, m, m);

    std::printf("||Q^T*Q - I||_F = %.12e\n", norm_QTQmI);

    return 0;
}
```


### Cosa ti aspetti numericamente

Su questo esempio classico, una implementazione corretta di Householder‑QR ti dà:[^5][^6][^7]

- un residuo relativo $\|A-QR\|_F / \|A\|_F$ dell’ordine di $10^{-15}$–$10^{-14}$;
- una devianza da identità $\|Q^{T}Q - I\|_F$ dello stesso ordine;
- una $R$ triangolare superiore con diagonale (nel caso completo teorico) positiva a seconda della convenzione di segno.[^2][^6][^4]

Questo `main` affiancato al tuo vecchio codice di bidiagonalizzazione/SVD ti permette di:

- verificare che il “cuore” Householder (costruzione + applicazione) stia facendo esattamente quello che pensavi nei tentativi empirici;
- confrontare strutturalmente il pattern “solo sinistra per QR” con “sinistra+destra alternati” per la bidiagonalizzazione Golub–Kahan.[^8][^9][^10]

Poi vediamo una versione analoga di `main` per la bidiagonalizzazione (stessa matrice, stampa di $U^{T}AV$ e residuo rispetto a una bidiagonale teorica), per chiudere il confronto QR/SVD.

<span style="display:none">[^11][^12][^13][^14][^15][^16][^17][^18]</span>

<div align="center">⁂</div>

[^1]: https://student.cs.uwaterloo.ca/~cs475/CS475-Lecture12.pdf

[^2]: https://www.cs.cornell.edu/~bindel/class/cs6210-f09/lec18.pdf

[^3]: https://sites.pitt.edu/~kimwong/lab07/index.html

[^4]: https://elearning.di.unipi.it/mod/resource/view.php?id=2378

[^5]: https://www.math.chalmers.se/~larisa/www/NumLinAlg/Lecture9_2019.pdf

[^6]: https://blogs.mathworks.com/cleve/2016/10/03/householder-reflections-and-the-qr-decomposition/

[^7]: https://receiptroller.co/en/technotes/p/householder-reflections-qr-decomposition-numerical-linear-algebra

[^8]: https://www.math.iit.edu/~fass/477577_Chapter_12.pdf

[^9]: https://en.wikipedia.org/wiki/Bidiagonalization

[^10]: https://github.com/emchinn/Bidiagonalization/blob/master/Golub-Kahan.ipynb

[^11]: https://stackoverflow.com/questions/39849941/writing-a-householder-qr-factorization-function-in-r-code

[^12]: https://rpubs.com/aaronsc32/qr-decomposition-householder

[^13]: https://gist.github.com/jakelevi1996/512fd1c1291edb07b4242574151088de

[^14]: https://cs.fit.edu/~dmitra/SciComp/19Fall/Presentations/QRDecomposition_20190910.pdf

[^15]: https://en.wikipedia.org/wiki/QR_decomposition

[^16]: https://www.appletonaudio.com/blog/2020/an-attempt-at-an-intuitive-description-of-the-qr-decomposition-using-householder-reflectors/

[^17]: https://www.scribd.com/document/313714438/QR-Decomposition

[^18]: https://www.scribd.com/document/880365358/ln10

\newpage


## 1. Ricostruzione di U dai riflettori sinistri

Assumo che:

- dopo `bidiag_householder_inplace(A,m,n)` i riflettori sinistri $u_k$ siano memorizzati come code nelle colonne:
    - per ogni $k$, colonna $k$, righe `k+1..m-1` contengono le componenti $2..$ di $u_k$;
    - la prima componente è implicita $u_k=1$.[^4][^3][^1]

La ricostruzione è analoga a `qr_form_Q_from_householder`:[^3][^1]

```cpp
// U: m x m, column-major
void bidiag_form_U_from_left(const double* A,
                             std::size_t   m,
                             std::size_t   n,
                             double*       U)
{
    assert(A != nullptr);
    assert(U != nullptr);

    std::size_t i, j, k;
    const std::size_t K = n; // numero riflettori sinistri (m>=n)

    // U = I_m
    for (j = 0; j < m; ++j)
        for (i = 0; i < m; ++i)
            U[i + j * m] = (i == j) ? 1.0 : 0.0;

    // Applica i riflettori U_k in ordine inverso
    for (std::size_t kk = 0; kk < K; ++kk) {
        k = K - 1 - kk;  // k = K-1, ..., 0

        const std::size_t len = m - k;
        double ubuf[^1024];
        assert(len <= 1024);

        // Ricostruisci u_k di dimensione len
        ubuf[^0] = 1.0;
        const double* col_k = A + k * m;

        for (i = 1; i < len; ++i)
            ubuf[i] = col_k[k + i];

        // Applica H_k = I - 2 u u^T a sinistra su U(k:m-1, :)
        householder_apply_left_to_matrix(U + k,
                                         m - k, // m locale
                                         m,     // tutte le colonne di U
                                         0,
                                         len,
                                         ubuf);
    }
}
```

Così ottieni $U = U_0 U_1 \dots U_{n-1}$, con $U_k$ applicati in ordine inverso alla identità, come prescritto per le sequenze di Householder.[^5][^1][^3]

***

## 2. Ricostruzione di V dai riflettori destri

Per i riflettori destri $V_k$:[^2][^1][^3]

- dopo `bidiag_householder_inplace`, ogni riga $k$ memorizza la coda del vettore $w_k$ sopra la superdiagonale:
    - la superdiagonale $A_{k,k+1}$ è l’elemento di $B$;
    - le posizioni $A_{k,k+2},\dots,A_{k,n-1}$ contengono le componenti $2..$ di $w_k$;
    - la prima componente implicita è $w_k = 1$.
- questi riflettori sono stati applicati a **destra** all’originale A; per ricostruire $V$ è più comodo lavorare su $V$ direttamente come prodotto di Householder applicati a destra.[^1][^2][^3]

Una via semplice (coerente con la tua `householder_apply_right_to_matrix`) è:

- inizializza $V = I_n$;
- per $k = K-2,\dots,0$ (l’ultimo riflettore destro è al massimo $k = n-2$):
    - ricostruisci $w_k$ dalla riga $k$;
    - applica $V \leftarrow V H_k$ usando `householder_apply_right_to_matrix` su tutte le righe di $V$, con sottospazio di colonne `k+1..n-1`.[^2][^3][^1]

```cpp
// V: n x n, column-major
void bidiag_form_V_from_right(const double* A,
                              std::size_t   m,
                              std::size_t   n,
                              double*       V)
{
    assert(A != nullptr);
    assert(V != nullptr);

    std::size_t i, j, k;

    // V = I_n
    for (j = 0; j < n; ++j)
        for (i = 0; i < n; ++i)
            V[i + j * n] = (i == j) ? 1.0 : 0.0;

    // Riflettori destri esistono per k = 0 .. n-2
    if (n < 2) return;

    const std::size_t K = n - 1;

    // Applica V_k in ordine inverso: k = K-1, ..., 0
    for (std::size_t kk = 0; kk < K; ++kk) {
        k = K - 1 - kk;  // k scende da n-2 a 0

        const std::size_t len = n - (k + 1);
        if (len == 0) continue;

        double wbuf[^1024];
        assert(len <= 1024);

        // Ricostruisci w_k (dimensione len) dalla riga k di A
        // w_k[^0] = 1, w_k[j] = A(k, k+1+ j) per j>=1
        wbuf[^0] = 1.0;

        for (std::size_t jloc = 1; jloc < len; ++jloc) {
            std::size_t col = k + 1 + jloc;
            const double* col_ptr = A + col * m;
            wbuf[jloc] = col_ptr[k];
        }

        // Applica H_k = I - 2 w w^T a destra di V(:, k+1:n-1)
        householder_apply_right_to_matrix(V,
                                          n,    // m locale (V è n x n)
                                          n,    // n locale
                                          k + 1,
                                          len,
                                          wbuf);
    }
}
```

Questa funzione costruisce $V = V_0 V_1 \dots V_{n-2}$, parallelamente a quanto fatto per $U$.[^3][^1][^2]

***

## 3. Uso nel main di test

Agganci queste due funzioni al `main` di bidiagonalizzazione che avevamo abbozzato:

- chiami `bidiag_householder_inplace(A,m,n)`;
- estrai $B$ da diag/superdiag;
- chiami:

```cpp
bidiag_form_U_from_left(A, m, n, U);
bidiag_form_V_from_right(A, m, n, V);
```

- ricomponi $U B V^{T}$ e misuri il residuo.[^4][^1][^3]

A questo punto hai la **coppia completa**:

- QR via Householder (solo sinistra) + test $A \approx Q R$;
- Bidiagonalizzazione via Householder (sinistra+destra alternati) + test $A \approx U B V^{T}$.[^6][^7][^1][^3]

Resta solo da armonizzare i dettagli di memorizzazione dei vettori $u_k, w_k$ tra la tua `bidiag_householder_inplace` e queste due funzioni, ma concettualmente seguono lo schema che hai già adottato per la QR.
<span style="display:none">[^10][^11][^12][^13][^14][^15][^16][^17][^8][^9]</span>

<div align="center">⁂</div>

[^1]: https://www.math.iit.edu/~fass/477577_Chapter_12.pdf

[^2]: https://drsfenner.org/blog/2016/03/householder-bidiagonalization/

[^3]: https://en.wikipedia.org/wiki/Bidiagonalization

[^4]: https://www.active-analytics.com/blog/householder-bigiag-in-d/

[^5]: https://www.netlib.org/utk/people/JackDongarra/etemplates/node198.html

[^6]: https://www.geometrictools.com/GTE/Mathematics/SingularValueDecomposition.h

[^7]: https://github.com/emchinn/Bidiagonalization/blob/master/Golub-Kahan.ipynb

[^8]: https://www.sciencedirect.com/science/article/abs/pii/S0096300306003808

[^9]: https://www.netlib.org/lapack/lawnspdf/lawn209.pdf

[^10]: https://en.wikipedia.org/wiki/Householder_transformation

[^11]: https://arxiv.org/pdf/math/0603251.pdf

[^12]: https://polytopes.net/courses/Stat624S08/tutorial4.pdf

[^13]: http://www.fp.tul.cz/~plesinger/my_publications/conference/GAMM_SIAM_DUS06.pdf

[^14]: https://www.youtube.com/watch?v=2OW5Yi6QOdY

[^15]: https://people.inf.ethz.ch/gander/talks/SVDnew.pdf

[^16]: https://iris.unica.it/handle/11584/312870

[^17]: https://pagesperso.locean-ipsl.upmc.fr/terray/statpack2.1/manuals/Module_SVD_Procedures.html

\newpage

\section*{Note}


