# Collaudo del confronto PCA e JL+PCA

13 novembre 2026, ore 23:34:02.292121. ID di run: `233402_292121_PCA_JL_2`.

**Ruolo:** Collaudo del nuovo logging, voce 6.

## Domanda

Verificare il salvataggio sotto confronti_spettrali senza alterare il confronto con la baseline storica.

## Esito

25 righe in confronto.csv, 150 in spettri.csv. Stessi 22/25 esiti positivi, stesse distorsioni e stesso spettro della prova dell’11 novembre a parità di semi. Cambiano l’ID e le misure di tempo.

## Lettura

Il test conferma la riproducibilità numerica della prova archiviata, non aggiunge 25 proiezioni indipendenti.

## Passo successivo

Per la lettura scientifica usare [il confronto originale](<../../../11/confronti_spettrali/214942_247_JL_PCA/NOTE.md>).

**Perimetro dei tempi storici.** Il tempo PCA comprende i prodotti interni $V_0^T V_0$ e $U_0^T U_0$ del percorso Bidiag–SVD, anche con il dump disattivato. Un rapporto di tempi ottenuto con questa baseline è **al lordo** di quei prodotti, che costano meno sulla matrice compressa. Non è uno speedup netto. Il [benchmark dedicato](../../../../../../../docs/JL_BENCHMARK_PCA.md) misura i due percorsi nello stesso eseguibile e distingue i due regimi. I CSV di questa prova restano invariati.

**File della prova:** [confronto.csv](./confronto.csv), [spettri.csv](./spettri.csv).
