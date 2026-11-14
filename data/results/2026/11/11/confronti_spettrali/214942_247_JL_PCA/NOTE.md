# Che cosa cambia nello spettro PCA dopo JL?

11 novembre 2026, ore 21:49:42.247. ID originale nel CSV: `20261111_214942_247`.

**Ruolo:** Campagna di riferimento, voce 6.

## Perché questa prova

La calibrazione dei k ci diceva se una proiezione rispettava una soglia sulle distanze. Non ci diceva se, dopo aver proiettato, la PCA avrebbe visto la stessa gerarchia di valori singolari. Confrontiamo dunque, per ciascun n, lo stesso sottoinsieme prima e dopo JL. I k vengono dalla scansione, ma qui il dataset è annidato e i semi di A sono diversi da quelli di calibrazione. Per questo ricontrolliamo le distanze invece di dare per acquisito il successo.

## Domanda

Confrontare PCA diretta e JL+PCA per 25 combinazioni n–ε. Una matrice gaussiana per combinazione, con i k della [campagna di calibrazione](<../../../10/coefficiente_n/204027_870_JL_coeff_n/NOTE.md>).

## Esito

22/25 proiezioni rispettano la soglia su tutte le coppie. In tutti i 25 casi σ₂(Y)/σ₂(X) > 1 e Q₂ < 1, dove Q₂ è il rapporto (σ₁/σ₂) dopo/prima. A n = 160, k = 202, ε = 0,50, la distorsione massima è 0,33971.

## Lettura

Il controllo delle distanze non coincide con la conservazione dello spettro. Una matrice per configurazione non basta a stimare la frequenza dell’effetto. I 22/25 non sono un tasso di successo per una singola configurazione.

## Passo successivo

Lo stesso verso del cambiamento di σ₂ e Q₂ si vede in tutti i casi, ma con una sola A per configurazione non sappiamo quanto conti la scelta della matrice. [Fissiamo n, d, k ed ε e cambiamo soltanto il seme di A](<../../monte_carlo_spettrale/225535_132_JL_PCA_MC/NOTE.md>) per separare le due questioni.

**Perimetro dei tempi storici.** Il tempo PCA comprende i prodotti interni $V_0^T V_0$ e $U_0^T U_0$ del percorso Bidiag–SVD, anche con il dump disattivato. Un rapporto di tempi ottenuto con questa baseline è **al lordo** di quei prodotti, che costano meno sulla matrice compressa. Non è uno speedup netto. Il [benchmark dedicato](../../../../../../../docs/JL_BENCHMARK_PCA.md) misura i due percorsi nello stesso eseguibile e distingue i due regimi. I CSV di questa prova restano invariati.

**File della prova:** [confronto.csv](./confronto.csv), [spettri.csv](./spettri.csv).
