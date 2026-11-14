# Classificazione — D_ref = 2, k = 400

12 novembre 2026, ore 21:06:47.722. ID originale nel CSV: `20261112_210647_722`.

**Ruolo:** Campagna a separazione controllata, voce 9.

## Perché questa prova

Il caso a k = 202 ha fatto emergere un divario che il dataset Federico nascondeva. Ora non spostiamo più i centri: vogliamo sapere quanta parte del divario si riduce dando a JL più coordinate. Così non confondiamo il beneficio di comprimere meno con quello di rendere i cluster più separati.

## Domanda

A D_ref = 2, che cosa recuperiamo passando da k = 202 a k = 400?

## Esito

Legge di potenza, D_ref = 2, d_ref = 1000, α = 0. n = 160, d = 1000, k = 400, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 158/160 | 144/160 |
| Distanza tra i centroidi nel piano PCA | 2,47722 | 2,39042 |
| Distanza / somma dei raggi RMS | 1,09310 | 0,73830 |
| Gap assiale | 0,10362 | -2,28995 |

Distorsione relativa massima 0,27986, violazioni JL 0/12720.

## Lettura

Si recuperano 11 classificazioni corrette (133 → 144). Il gap migliora, ma resta negativo. L’esito è condizionato al dataset e al seme usati. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

Il recupero da 133 a 144 punti corretti è visibile, ma la PCA diretta è a 158 e il gap resta negativo. Continuiamo a [k = 600](<../210853_389_PCA_JL_CLASS1/NOTE.md>) per vedere se il recupero prosegue.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
