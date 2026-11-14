# Primo collaudo del confronto con k variabile

11 novembre 2026, ore 23:33:12.433. ID originale nel CSV: `20261111_233312_433`.

**Ruolo:** Collaudo storico, voce 8.

## Domanda

Verificare il percorso accoppiato e l’esportazione su una sola combinazione, prima di percorrere la griglia di k.

## Esito

Una sola riga di trial, n = 160, d = 1000, k = 202, seed_A = 70000, ε = 0,50. JL riuscito con distorsione massima 0,36885 e Q₂ = 0,72308.

## Lettura

Il risultato coincide con il primo caso della campagna completa. Da solo non dice come evolva lo spettro con k.

## Passo successivo

Completare [i cinque trial accoppiati su quattro k](<../233629_422_JL_PCA_K/NOTE.md>).

**Perimetro dei tempi storici.** Il tempo PCA comprende i prodotti interni $V_0^T V_0$ e $U_0^T U_0$ del percorso Bidiag–SVD, anche con il dump disattivato. Un rapporto di tempi ottenuto con questa baseline è **al lordo** di quei prodotti, che costano meno sulla matrice compressa. Non è uno speedup netto. Il [benchmark dedicato](../../../../../../../docs/JL_BENCHMARK_PCA.md) misura i due percorsi nello stesso eseguibile e distingue i due regimi. I CSV di questa prova restano invariati.

**File della prova:** [stat.csv](./stat.csv), [trial.csv](./trial.csv).
