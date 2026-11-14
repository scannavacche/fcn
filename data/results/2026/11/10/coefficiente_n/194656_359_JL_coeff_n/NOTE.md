# Numero di punti crescente con d = 500

10 novembre 2026, ore 19:46:56.359. ID originale nel CSV: `20261110_194656_359`.

**Ruolo:** Campagna storica, voce 4.

## Domanda

A d fisso, il primo k trovato cresce con n? Si provano n = 50, 100, 200, 400, 800, con T = 120 e obiettivo 95%.

## Esito

Per ε = 0,40, k cresce da 217 a 369. Il rapporto 369/217 ≈ 1,700 è vicino a log(800)/log(50) ≈ 1,709. I casi positivi sono 22/35. A ε = 0,30, per n = 200 il tetto k = 499 arriva a 113/120, una riuscita sotto il target.

## Lettura

Il confronto è coerente con la scala logaritmica, ma non la dimostra. Il vincolo di riduzione k < d limita la ricerca per soglie più strette.

## Passo successivo

Portare d a 1000 e restringere la griglia di n e di ε per ottenere una [tabella di k utilizzabile nel confronto PCA](<../204027_870_JL_coeff_n/NOTE.md>).

**File della prova:** [scan.csv](./scan.csv), [summary.csv](./summary.csv).
