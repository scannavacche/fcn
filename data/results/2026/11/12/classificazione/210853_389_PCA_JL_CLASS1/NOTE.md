# Classificazione — D_ref = 2, k = 600

12 novembre 2026, ore 21:08:53.389. ID originale nel CSV: `20261112_210853_389`.

**Ruolo:** Campagna a separazione controllata, voce 9.

## Perché questa prova

Da k = 202 a 400 erano migliorate sia la distorsione massima sia l’accuratezza. Aumentiamo ancora k con lo stesso X e gli stessi semi per verificare se la tendenza continua. Il confronto riguarda anche il costo in compressione: più ci avviciniamo a d, meno coordinate risparmiamo.

## Domanda

A D_ref = 2, il miglioramento continua passando da k = 400 a k = 600?

## Esito

Legge di potenza, D_ref = 2, d_ref = 1000, α = 0. n = 160, d = 1000, k = 600, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 158/160 | 151/160 |
| Distanza tra i centroidi nel piano PCA | 2,47722 | 2,43326 |
| Distanza / somma dei raggi RMS | 1,09310 | 0,82029 |
| Gap assiale | 0,10362 | -2,28804 |

Distorsione relativa massima 0,24563, violazioni JL 0/12720.

## Lettura

Si sale da 144 a 151 classificazioni corrette e il rapporto separazione/raggi migliora. Il gap assiale rimane quasi invariato rispetto a k = 400. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Si arriva a 151/160, ma il gap assiale si muove appena. Il [lancio a k = 800](<../211201_253_PCA_JL_CLASS1/NOTE.md>) serve a capire se i progressi nella distorsione, nella separazione e nelle predizioni proseguono davvero insieme.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
