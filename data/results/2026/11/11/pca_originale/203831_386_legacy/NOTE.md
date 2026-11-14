# PCA centrata senza riduzione JL

11 novembre 2026, ore 20:38:31.386. ID originale nel CSV: `20261111_203831_386`.

**Ruolo:** Baseline storica e copia fredda.

## Domanda

Misurare spettro e tempi prima di JL. Si parte da un master Federico con n = 160, d = 1000 e seed_X = 42 e si estraggono sottoinsiemi bilanciati e annidati.

## Esito

Il CSV ha cinque righe, per n = 10, 20, 40, 80, 160, con sei valori singolari, rapporti, quote di energia e tempi. A n = 160, σ₁ ≈ 161,98931, σ₂ ≈ 13,17953, quota PC1 ≈ 0,64950 e tempo ≈ 141,476 s.

## Lettura

Questo è il riferimento della SVD originale per quel preciso dataset, non una cache rigenerata automaticamente. La copia operativa è in `data/state/cache/` e al commit 8334ed6 è identica byte per byte a questa copia fredda. La classificazione con D_ref controllato non deve riusarla.

## Passo successivo

Usare la baseline per [il confronto spettrale](<../../confronti_spettrali/214942_247_JL_PCA/NOTE.md>) e per i Monte Carlo. Non confrontare direttamente questo tempo con quello della PCA Gram/Jacobi della classificazione.

**File della prova:** [JL_PCA_originale_20261111_203831_386.csv](./JL_PCA_originale_20261111_203831_386.csv).
