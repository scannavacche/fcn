# Classificazione — Federico, esito osservato, k = 202

12 novembre 2026, ore 20:35:48.602. ID originale nel CSV: `20261112_203548_602`.

**Ruolo:** Controllo di regressione del generatore.

## Perché questa prova

Per rendere più difficile la classificazione vogliamo avvicinare i centri, ma senza cambiare contemporaneamente il rumore o perdere il generatore originale. Prima di usare il parametro nuovo, il default deve quindi riprodurre il risultato già noto. Altrimenti non sapremmo se un cambiamento dipenda dalla distanza scelta o dall’introduzione del generatore parametrico.

## Domanda

I default del generatore parametrico conservano il caso storico?

## Esito

Federico, esito osservato. n = 160, d = 1000, k = 202, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 160/160 | 160/160 |
| Distanza tra i centroidi nel piano PCA | 25,60605 | 27,30406 |
| Distanza / somma dei raggi RMS | 11,84375 | 8,04200 |
| Gap assiale | 24,27968 | 23,03620 |

Distorsione relativa massima 0,46484, violazioni JL 0/12720.

## Lettura

I valori geometrici, spettrali e di classificazione coincidono con il primo caso Federico a k = 202. È un controllo del percorso di default, non un nuovo campione. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Impostare la distanza nominale fissa e verificare che [l’impostazione sia effettivamente usata](<../203733_167_PCA_JL_CLASS1/NOTE.md>).

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
