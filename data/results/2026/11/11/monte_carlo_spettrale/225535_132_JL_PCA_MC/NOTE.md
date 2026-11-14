# L’effetto spettrale si ripete cambiando proiezione?

11 novembre 2026, ore 22:55:35.132. ID originale nel CSV: `20261111_225535_132`.

**Ruolo:** Campagna di riferimento, voce 7.

## Domanda

Sul master Federico, tenere fissi n = 160, d = 1000, k = 202, ε = 0,50 e usare 20 matrici con semi 60000–60019.

## Esito

20/20 proiezioni rispettano JL. In media σ₁(Y)/σ₁(X) ≈ 1,00535, σ₂(Y)/σ₂(X) ≈ 1,43307 e Q₂ ≈ 0,70153. σ₂ cresce e Q₂ resta sotto 1 in tutti i trial. La quota PC1 è 0,64950 prima e in media 0,65237 dopo JL.

## Lettura

L’effetto si ripete in questa configurazione anche quando l’energia totale media è quasi invariata. Non implica che la seconda direzione originale sia conservata o che 20/20 stimi con precisione la probabilità di riuscita. I circa 1,644 s medi del percorso JL+PCA contro 141,476 s della baseline sono indicativi, non un benchmark end-to-end controllato.

## Passo successivo

Provare [k diversi con proiezioni accoppiate](<../../stabilita_spettrale_k/233629_422_JL_PCA_K/NOTE.md>) per misurare la dipendenza dalla compressione.

**File della prova:** [stat.csv](./stat.csv), [trial.csv](./trial.csv).
