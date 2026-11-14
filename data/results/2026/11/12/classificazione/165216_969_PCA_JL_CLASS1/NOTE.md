# Classificazione — Federico, k = 202

12 novembre 2026, ore 16:52:16.969. ID originale nel CSV: `20261112_165216_969`.

**Ruolo:** Riferimento storico, voce 9.

## Domanda

La differenza spettrale osservata compromette la separazione dei due cluster di Federico?

## Esito

Federico. n = 160, d = 1000, k = 202, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 160/160 | 160/160 |
| Distanza tra i centroidi nel piano PCA | 25,60605 | 27,30406 |
| Distanza / somma dei raggi RMS | 11,84375 | 8,04200 |
| Gap assiale | 24,27968 | 23,03620 |

Distorsione relativa massima 0,46484, violazioni JL 0/12720.

## Lettura

Qui l’accuratezza è satura. Il rapporto separazione/raggi scende malgrado l’aumento della distanza fra centroidi. I gruppi restano molto separati. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Cambiare solo k e passare a [k = 400](<../185044_074_PCA_JL_CLASS1/NOTE.md>).

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
