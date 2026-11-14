# Che cosa cambia nello spettro PCA dopo JL?

11 novembre 2026, ore 21:49:42.247. ID originale nel CSV: `20261111_214942_247`.

**Ruolo:** Campagna di riferimento, voce 6.

## Domanda

Confrontare PCA diretta e JL+PCA per 25 combinazioni n–ε. Una matrice gaussiana per combinazione, con i k della [campagna di calibrazione](<../../../10/coefficiente_n/204027_870_JL_coeff_n/NOTE.md>).

## Esito

22/25 proiezioni rispettano la soglia su tutte le coppie. In tutti i 25 casi σ₂(Y)/σ₂(X) > 1 e Q₂ < 1, dove Q₂ è il rapporto (σ₁/σ₂) dopo/prima. A n = 160, k = 202, ε = 0,50, la distorsione massima è 0,33971.

## Lettura

Il controllo delle distanze non coincide con la conservazione dello spettro. Una matrice per configurazione non basta a stimare la frequenza dell’effetto. I 22/25 non sono un tasso di successo per una singola configurazione.

## Passo successivo

Fissare n = 160, k = 202, ε = 0,50 e [variare soltanto il seme di A](<../../monte_carlo_spettrale/225535_132_JL_PCA_MC/NOTE.md>).

**File della prova:** [confronto.csv](./confronto.csv), [spettri.csv](./spettri.csv).
