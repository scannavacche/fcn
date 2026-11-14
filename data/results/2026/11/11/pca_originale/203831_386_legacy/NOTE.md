# PCA centrata senza riduzione JL

11 novembre 2026, ore 20:38:31.386. ID originale nel CSV: `20261111_203831_386`.

**Ruolo:** Baseline storica e copia fredda.

## Perché questa prova

Una volta ottenuti dei k utilizzabili, serve il termine di paragone. Prima di proiettare vogliamo sapere quali valori singolari e quote di energia produce la PCA sui dati originali. Il master unico evita di cambiare nuvola a ogni n. Il sottoinsieme più grande aggiunge punti al precedente, mantenendo metà punti per classe. Per ogni n salviamo la PCA originale una volta e la riusiamo nei confronti con soglie e proiezioni diverse.

## Domanda

Misurare spettro e tempi prima di JL. Si parte da un master Federico con n = 160, d = 1000 e seed_X = 42 e si estraggono sottoinsiemi bilanciati e annidati.

## Esito

Il CSV ha cinque righe, per n = 10, 20, 40, 80, 160, con sei valori singolari, rapporti, quote di energia e tempi. A n = 160, σ₁ ≈ 161,98931, σ₂ ≈ 13,17953, quota PC1 ≈ 0,64950 e tempo ≈ 141,476 s.

## Lettura

Questo è il riferimento della SVD originale per quel preciso dataset, non una cache rigenerata automaticamente. La copia operativa è in `data/state/cache/` e al commit 8334ed6 è identica byte per byte a questa copia fredda. La classificazione con D_ref controllato non deve riusarla.

## Passo successivo

Usare la baseline per [il confronto spettrale](<../../confronti_spettrali/214942_247_JL_PCA/NOTE.md>) e per i Monte Carlo. Non confrontare direttamente questo tempo con quello della PCA Gram/Jacobi della classificazione.

**Perimetro dei tempi storici.** Il tempo PCA comprende i prodotti interni $V_0^T V_0$ e $U_0^T U_0$ del percorso Bidiag–SVD, anche con il dump disattivato. Un rapporto di tempi ottenuto con questa baseline è **al lordo** di quei prodotti, che costano meno sulla matrice compressa. Non è uno speedup netto. Il [benchmark dedicato](../../../../../../../docs/JL_BENCHMARK_PCA.md) misura i due percorsi nello stesso eseguibile e distingue i due regimi. I CSV di questa prova restano invariati.

**File della prova:** [JL_PCA_originale_20261111_203831_386.csv](./JL_PCA_originale_20261111_203831_386.csv).
