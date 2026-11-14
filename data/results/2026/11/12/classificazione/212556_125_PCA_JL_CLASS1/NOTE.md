# Classificazione — D_ref = 4, k = 202

12 novembre 2026, ore 21:25:56.125. ID originale nel CSV: `20261112_212556_125`.

**Ruolo:** Campagna a separazione controllata, voce 9.

## Perché questa prova

A D_ref = 2 la separazione era stata volutamente indebolita. Dare più coordinate a JL aiutava, ma non faceva muovere all’unisono distorsione e accuratezza. Ci siamo allora chiesti se il problema fosse anche riconoscere, nelle prime componenti stimate, la direzione dei centroidi quando il rumore pesa molto sulla geometria osservata.

Per metterlo alla prova raddoppiamo D_ref, senza toccare il rumore isotropo a σ = 0,3, e torniamo a k = 202. Il codice sposta rigidamente ciascun gruppo lungo la stessa direzione e riusa le realizzazioni gaussiane. A k, d e seed_A uguali si usa anche la stessa A. Il confronto con D_ref = 2 e k = 202 isola dunque la separazione nominale. Ridurre σ era un’altra leva discussa, non quella usata qui.

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

La classificazione JL+PCA passa da 133/160 a 159/160 e il gap è molto meno negativo. La quota di energia della prima componente nella PCA diretta passa da circa 1,92% a 5,09%, coerentemente con un segnale fra gruppi più marcato. Non è una misura diretta dell’allineamento fra PC1 e l’asse dei centroidi. La distorsione massima coincide nelle due prove, ma questo non dice che tutte le distorsioni siano identiche. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

A parità di k e di distorsione massima osservata, la classificazione migliora nettamente. Ora che la geometria è meno difficile, teniamo D_ref = 4 e [ripetiamo il confronto a k = 400](<../212722_043_PCA_JL_CLASS1/NOTE.md>). Vogliamo vedere se comprimere meno recupera ancora qualcosa nella separazione e nell’accuratezza, già vicina al tetto.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
