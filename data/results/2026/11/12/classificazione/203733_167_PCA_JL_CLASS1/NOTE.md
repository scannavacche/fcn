# Classificazione — Esito Federico, k = 202

12 novembre 2026, ore 20:37:33.167. ID originale nel CSV: `20261112_203733_167`.

**Ruolo:** Diagnosi, configurazione D_ref = 2 non applicata.

## Perché questa prova

Dopo il controllo di default, questo doveva essere il primo caso difficile. La previsione era che, a rumore invariato, avvicinare i centri avrebbe fatto emergere differenze che il 160/160 storico non mostrava. La coincidenza dei risultati con il caso di default ci ha fatto verificare prima la chiamata al generatore, anziché interpretare quella coincidenza come insensibilità della PCA alla separazione.

## Domanda

La modifica del default a distanza fissa 2 è arrivata al generatore usato dalla classificazione?

## Esito

Esito Federico. n = 160, d = 1000, k = 202, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 160/160 | 160/160 |
| Distanza tra i centroidi nel piano PCA | 25,60605 | 27,30406 |
| Distanza / somma dei raggi RMS | 11,84375 | 8,04200 |
| Gap assiale | 24,27968 | 23,03620 |

Distorsione relativa massima 0,46484, violazioni JL 0/12720.

## Lettura

No. Il risultato coincide ancora con il caso Federico a k = 202. Non usare questo CSV come prova a distanza 2. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Passare esplicitamente la configurazione alla generazione e ripetere. [Prima prova effettiva a D_ref = 2](<../210411_282_PCA_JL_CLASS1/NOTE.md>).

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
