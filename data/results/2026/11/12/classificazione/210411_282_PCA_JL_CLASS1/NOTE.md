# Classificazione — D_ref = 2, k = 202

12 novembre 2026, ore 21:04:11.282. ID originale nel CSV: `20261112_210411_282`.

**Ruolo:** Campagna a separazione controllata, voce 9.

## Domanda

A D_ref = 2 e k = 202, rispettare la soglia sulle distanze basta a conservare la classificazione dopo PCA?

## Esito

Legge di potenza, D_ref = 2, d_ref = 1000, α = 0. n = 160, d = 1000, k = 202, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 158/160 | 133/160 |
| Distanza tra i centroidi nel piano PCA | 2,47722 | 2,27505 |
| Distanza / somma dei raggi RMS | 1,09310 | 0,60305 |
| Gap assiale | 0,10362 | -3,23115 |

Distorsione relativa massima 0,46484, violazioni JL 0/12720.

## Lettura

No in questo caso. Nonostante nessuna coppia oltre ε, si passa da 158 a 133 punti corretti e gli intervalli proiettati sull’asse dei centroidi si sovrappongono dopo JL. D_ref è la separazione nominale del generatore, non quella misurata nel piano PCA. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Ridurre la compressione a [k = 400](<../210647_722_PCA_JL_CLASS1/NOTE.md>), senza cambiare X, semi o soglia.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
