# Tabella di k per il confronto PCA

10 novembre 2026, ore 20:40:27.870. ID originale nel CSV: `20261110_204027_870`.

**Ruolo:** Campagna di riferimento, voce 4.

## Perché questa prova

Nella scansione precedente, a d = 500, il vincolo k < d limitava parecchi casi alle soglie più strette. Qui alziamo d a 1000, restringiamo n e lasciamo fuori le soglie sotto 0,30. Lo scopo immediato è ottenere casi in cui il k utile cada dentro il dominio di ricerca. La tabella risultante ci darà un punto di partenza per confrontare PCA diretta e PCA dopo JL, senza scegliere k a occhio.

## Domanda

Trovare un k empirico per ognuna delle 25 combinazioni tra n = 10, 20, 40, 80, 160 e ε = 0,30, 0,35, 0,40, 0,45, 0,50, a d = 1000.

## Esito

25/25 combinazioni raggiungono il target del 95% su 120 matrici. Ĉ = k ε²/log n va da circa 8,22 a 10,48. Per n = 160, i k trovati alle soglie 0,30, 0,40 e 0,50 sono 502, 304 e 202.

## Lettura

Sono i primi successi della griglia, non minimi matematici. La voce 6 incorpora questa tabella. I suoi dataset annidati e le sue proiezioni sono diversi dalla calibrazione, quindi il rispetto di ε va ricontrollato.

## Passo successivo

Preparare [la PCA di riferimento sul master annidato](<../../../11/pca_originale/203831_386_legacy/NOTE.md>) e poi lanciare [il confronto con JL](<../../../11/confronti_spettrali/214942_247_JL_PCA/NOTE.md>).

**File della prova:** [scan.csv](./scan.csv), [summary.csv](./summary.csv).
