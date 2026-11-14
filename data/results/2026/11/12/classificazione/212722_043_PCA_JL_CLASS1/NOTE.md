# Classificazione — D_ref = 4, k = 400

12 novembre 2026, ore 21:27:22.043. ID originale nel CSV: `20261112_212722_043`.

**Ruolo:** Campagna a separazione controllata, voce 9.

## Perché questa prova

Il raddoppio della distanza, a k = 202, aveva portato JL+PCA quasi al tetto dell’accuratezza. A quel punto la domanda era se, senza spostare di nuovo le nuvole, dare più coordinate alla proiezione migliorasse ancora la separazione o il conteggio dei punti corretti. Ripetiamo quindi a D_ref = 4 lo stesso passaggio k = 202 → 400 già provato a D_ref = 2. Si può così leggere l’effetto di k a geometria fissa e, separatamente, quello della distanza confrontando prove allo stesso k.

## Domanda

A D_ref = 4, aumentare k da 202 a 400 cambia anche le predizioni o soltanto gli indicatori geometrici?

## Esito

Legge di potenza, D_ref = 4, d_ref = 1000, α = 0. n = 160, d = 1000, k = 400, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 160/160 | 159/160 |
| Distanza tra i centroidi nel piano PCA | 4,30284 | 4,36400 |
| Distanza / somma dei raggi RMS | 1,96764 | 1,42679 |
| Gap assiale | 2,71412 | 0,52790 |

Distorsione relativa massima 0,28515, violazioni JL 0/12720. Unico punto errato dopo JL: id 60 (indice nel CSV).

## Lettura

Il gap assiale cambia segno (−0,40630 → +0,52790) e il rapporto separazione/raggi migliora. L’accuratezza resta 159/160. Il gap positivo separa gli intervalli sull’asse tra i centroidi, ma non dice dove cada il confine del classificatore. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

**Rilettura dei punti.** Solo confrontando dopo i due `points.csv` si vede che l’unico errore non è lo stesso: il punto 153 torna corretto e il 60 diventa errato. Non era il motivo del lancio. È il motivo per cui, nel seguito, non basta confrontare i due conteggi. Con un solo seme non sappiamo se lo stesso effetto si ripeta.

## Passo successivo

Un seguito naturale è rifare questo confronto con più semi di A, accoppiando i k all’interno del seme. Oltre all’accuratezza, confrontare quali punti cambiano predizione e come si muovono i margini. Non è un esperimento già eseguito.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
