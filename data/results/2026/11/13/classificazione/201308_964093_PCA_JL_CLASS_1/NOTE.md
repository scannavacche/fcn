# Classificazione — D_ref = 4, k = 202

13 novembre 2026, ore 20:13:08.964093. ID di run: `201308_964093_PCA_JL_CLASS_1`.

**Ruolo:** Collaudo del nuovo logging, voce 9.

## Domanda

La nuova cartella run conserva i quattro prodotti della classificazione e riproduce il caso storico?

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

Sì. Ripete il caso a D_ref = 4 e k = 202 del 12 novembre, inclusa la predizione errata sul punto 153. È una regressione del logging con gli stessi semi, non una replica indipendente. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Per il confronto scientifico rimandare a [D_ref = 4, k = 202](<../../../12/classificazione/212556_125_PCA_JL_CLASS1/NOTE.md>) e [D_ref = 4, k = 400](<../../../12/classificazione/212722_043_PCA_JL_CLASS1/NOTE.md>).

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
