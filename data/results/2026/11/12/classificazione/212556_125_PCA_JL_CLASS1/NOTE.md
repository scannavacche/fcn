# Classificazione — D_ref = 4, k = 202

12 novembre 2026, ore 21:25:56.125. ID originale nel CSV: `20261112_212556_125`.

**Ruolo:** Campagna a separazione controllata, voce 9.

## Domanda

Con gli stessi semi e k = 202, quanto cambia la classificazione se D_ref passa da 2 a 4?

## Esito

Legge di potenza, D_ref = 4, d_ref = 1000, α = 0. n = 160, d = 1000, k = 202, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 160/160 | 159/160 |
| Distanza tra i centroidi nel piano PCA | 4,30284 | 4,46404 |
| Distanza / somma dei raggi RMS | 1,96764 | 1,27122 |
| Gap assiale | 2,71412 | -0,40630 |

Distorsione relativa massima 0,46484, violazioni JL 0/12720. Unico punto errato dopo JL: id 153 (indice nel CSV).

## Lettura

La classificazione JL+PCA passa da 133/160 a 159/160. La distorsione massima rimane identica alla prova a distanza 2, ma il gap assiale è molto meno negativo. La separazione dei dati conta anche a parità della distorsione peggiore. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Tenere D_ref = 4 e passare a [k = 400](<../212722_043_PCA_JL_CLASS1/NOTE.md>) per distinguere recupero geometrico e accuratezza.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
