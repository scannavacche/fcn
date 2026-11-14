# Collaudo della stabilità spettrale variando k

13 novembre 2026, ore 19:57:24.951994. ID di run: `195724_951994_PCA_JL_K_MC_0`.

**Ruolo:** Collaudo del nuovo logging, voce 8.

## Domanda

Controllare che i trial accoppiati e le statistiche per k finiscano nella stessa nuova cartella.

## Esito

20 trial e 56 righe di statistiche. Si ritrovano 20/20 esiti JL positivi e gli stessi valori numerici spettrali delle cinque traiettorie storiche. Cambiano l’ID e i tempi e si aggiungono campi di tracciabilità.

## Lettura

La prova ripete gli stessi semi e non aumenta il campione. I dati rimangono utili per controllare la migrazione dei file.

## Passo successivo

Per le conclusioni sulla coda e su Q₂ usare [la campagna originale](<../../../11/stabilita_spettrale_k/233629_422_JL_PCA_K/NOTE.md>).

**Perimetro dei tempi storici.** Il tempo PCA comprende i prodotti interni $V_0^T V_0$ e $U_0^T U_0$ del percorso Bidiag–SVD, anche con il dump disattivato. Un rapporto di tempi ottenuto con questa baseline è **al lordo** di quei prodotti, che costano meno sulla matrice compressa. Non è uno speedup netto. Il [benchmark dedicato](../../../../../../../docs/JL_BENCHMARK_PCA.md) misura i due percorsi nello stesso eseguibile e distingue i due regimi. I CSV di questa prova restano invariati.

**File della prova:** [stat.csv](./stat.csv), [trial.csv](./trial.csv).
