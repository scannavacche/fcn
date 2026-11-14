# L’effetto spettrale si ripete cambiando proiezione?

11 novembre 2026, ore 22:55:35.132. ID originale nel CSV: `20261111_225535_132`.

**Ruolo:** Campagna di riferimento, voce 7.

## Perché questa prova

Nel confronto precedente, il secondo valore singolare aumentava in tutti i 25 casi e il rapporto σ₁/σ₂ si riduceva. L’osservazione era interessante, ma cambiavano insieme dataset, k e proiezione. Qui congeliamo X e k e ripetiamo il sorteggio di A. Se il fenomeno sparisce, poteva essere legato alle proiezioni della prima griglia. Se si ripete, abbiamo una ragione per misurare come dipenda dalla compressione in questa configurazione.

## Domanda

Sul master Federico, tenere fissi n = 160, d = 1000, k = 202, ε = 0,50 e usare 20 matrici con semi 60000–60019.

## Esito

20/20 proiezioni rispettano JL. In media σ₁(Y)/σ₁(X) ≈ 1,00535, σ₂(Y)/σ₂(X) ≈ 1,43307 e Q₂ ≈ 0,70153. σ₂ cresce e Q₂ resta sotto 1 in tutti i trial. La quota PC1 è 0,64950 prima e in media 0,65237 dopo JL.

## Lettura

L’effetto si ripete in questa configurazione anche quando l’energia totale media è quasi invariata. Non implica che la seconda direzione originale sia conservata o che 20/20 stimi con precisione la probabilità di riuscita. I circa 1,644 s medi del percorso JL+PCA contro 141,476 s della baseline sono indicativi, non un benchmark end-to-end controllato.

## Passo successivo

Il fenomeno si ripete anche quando tutte le proiezioni passano il controllo JL. Vogliamo ora capire se si attenua riducendo la compressione. Nella [prova con k accoppiati](<../../stabilita_spettrale_k/233629_422_JL_PCA_K/NOTE.md>) i diversi k di un trial usano le prime righe dello stesso blocco gaussiano, riscalate, così non cambiamo insieme k e l’intero sorteggio.

**Perimetro dei tempi storici.** Il tempo PCA comprende i prodotti interni $V_0^T V_0$ e $U_0^T U_0$ del percorso Bidiag–SVD, anche con il dump disattivato. Un rapporto di tempi ottenuto con questa baseline è **al lordo** di quei prodotti, che costano meno sulla matrice compressa. Non è uno speedup netto. Il [benchmark dedicato](../../../../../../../docs/JL_BENCHMARK_PCA.md) misura i due percorsi nello stesso eseguibile e distingue i due regimi. I CSV di questa prova restano invariati.

**File della prova:** [stat.csv](./stat.csv), [trial.csv](./trial.csv).
