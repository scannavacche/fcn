# Monte Carlo spettrale prima della migrazione della voce 7

13 novembre 2026, ore 19:56:44.105. ID originale `20261113_195644_105`.

**Ruolo:** collaudo precedente alla migrazione della voce 7, riclassificato a mano.

**Domanda.** La voce 7 riesce a leggere la baseline dopo la prima migrazione, anche se scrive ancora nel formato precedente?

**Esito.** I due CSV sono presenti, con 20/20 proiezioni riuscite, rapporto medio σ₂(Y)/σ₂(X) ≈ 1,43307 e Q₂ ≈ 0,70153. Il tempo totale medio registrato è circa 1,426 s.

**Lettura.** I semi 60000–60019 coincidono con quelli della [campagna originale](<../../../11/monte_carlo_spettrale/225535_132_JL_PCA_MC/NOTE.md>). Non è un’altra replica statistica. La cartella di run e i nomi brevi dei CSV sono stati assegnati in archivio. Non sono stati prodotti dal logger di allora.

**Passo successivo.** Confrontare con [il lancio delle 23:35 dopo la migrazione](<../233503_013227_PCA_JL_MC_3/NOTE.md>).

**Perimetro dei tempi storici.** Il tempo PCA comprende i prodotti interni $V_0^T V_0$ e $U_0^T U_0$ del percorso Bidiag–SVD, anche con il dump disattivato. Un rapporto di tempi ottenuto con questa baseline è **al lordo** di quei prodotti, che costano meno sulla matrice compressa. Non è uno speedup netto. Il [benchmark dedicato](../../../../../../../docs/JL_BENCHMARK_PCA.md) misura i due percorsi nello stesso eseguibile e distingue i due regimi. I CSV di questa prova restano invariati.

**File della prova:** [trial](./trial.csv), [statistiche](./stat.csv).
