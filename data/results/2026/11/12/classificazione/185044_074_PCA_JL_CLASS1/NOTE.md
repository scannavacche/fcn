# Classificazione — Federico, k = 400

12 novembre 2026, ore 18:50:44.074. ID originale nel CSV: `20261112_185044_074`.

**Ruolo:** Confronto storico, voce 9.

## Perché questa prova

Il caso k = 202 aveva già ottenuto 160/160. Un risultato identico a k = 400 non poteva migliorare quel conteggio. Poteva però dirci se le nuvole tornavano più simili alle originali riducendo la compressione. Perciò confrontiamo anche gli indicatori geometrici, senza cambiare la generazione dei dati.

## Domanda

Con la stessa coppia di semi, quale effetto ha aumentare k da 202 a 400 sul dataset Federico?

## Esito

Federico. n = 160, d = 1000, k = 400, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 160/160 | 160/160 |
| Distanza tra i centroidi nel piano PCA | 25,60605 | 26,24484 |
| Distanza / somma dei raggi RMS | 11,84375 | 8,83346 |
| Gap assiale | 24,27968 | 22,73025 |

Distorsione relativa massima 0,27599, violazioni JL 0/12720.

## Lettura

La distorsione massima scende da 0,46484 a 0,27599 e il rapporto separazione/raggi sale, ma il gap assiale cala leggermente e la classificazione resta a 160/160. Sono misure di proprietà diverse. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Il 160/160 resiste, anche se gli indicatori geometrici cambiano. Il modello Federico separa i gruppi troppo bene per mettere sotto pressione il classificatore in questi due casi. Introduciamo allora una separazione nominale controllabile, senza cambiare il rumore. Prima [controlliamo il default](<../203548_602_PCA_JL_CLASS1/NOTE.md>), poi proviamo [D_ref = 2](<../210411_282_PCA_JL_CLASS1/NOTE.md>) per vedere che cosa succede quando la direzione fra i centri è meno evidente.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
