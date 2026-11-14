# Classificazione — D_ref = 4, k = 400

12 novembre 2026, ore 21:27:22.043. ID originale nel CSV: `20261112_212722_043`.

**Ruolo:** Campagna a separazione controllata, voce 9.

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

Il gap assiale cambia segno (−0,40630 → +0,52790) e il rapporto separazione/raggi migliora. L’accuratezza resta 159/160, ma cambia l’errore: il punto 153 torna corretto e il punto 60 diventa errato. Il gap riguarda gli intervalli sull’asse tra i centroidi, non garantisce che il confine del classificatore LOO separi tutti i punti. Con il solo seme comune non si conclude che il fenomeno sia sistematico. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Ripetere con più semi di A e confrontare i cambi di predizione, oltre alle medie. In seguito variare un solo parametro del generatore per volta, separazione oppure rumore.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
