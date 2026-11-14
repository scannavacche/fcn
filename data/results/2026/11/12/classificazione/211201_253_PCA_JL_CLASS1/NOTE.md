# Classificazione — D_ref = 2, k = 800

12 novembre 2026, ore 21:12:01.253. ID originale nel CSV: `20261112_211201_253`.

**Ruolo:** Campagna a separazione controllata, voce 9.

## Domanda

A D_ref = 2, una distorsione massima ancora più piccola implica più classificazioni corrette?

## Esito

Legge di potenza, D_ref = 2, d_ref = 1000, α = 0. n = 160, d = 1000, k = 800, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 158/160 | 150/160 |
| Distanza tra i centroidi nel piano PCA | 2,47722 | 2,41865 |
| Distanza / somma dei raggi RMS | 1,09310 | 0,84450 |
| Gap assiale | 0,10362 | -1,90388 |

Distorsione relativa massima 0,21690, violazioni JL 0/12720.

## Lettura

No. Da k = 600 a 800, la distorsione massima scende da 0,24563 a 0,21690, il gap e il rapporto separazione/raggi migliorano, ma si passa da 151/160 a 150/160. Il punto 123 torna corretto e i punti 81 e 101 diventano errati. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Cambiare la difficoltà della classificazione portando la distanza nominale a 4. Ripartire da [k = 202](<../212556_125_PCA_JL_CLASS1/NOTE.md>) per confrontarlo con la prima prova a D_ref = 2.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
