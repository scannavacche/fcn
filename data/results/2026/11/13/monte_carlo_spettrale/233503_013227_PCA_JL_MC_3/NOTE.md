# Collaudo del Monte Carlo spettrale

13 novembre 2026, ore 23:35:03.013227. ID di run: `233503_013227_PCA_JL_MC_3`.

**Ruolo:** Collaudo del nuovo logging, voce 7.

## Domanda

Verificare che trial e statistiche finiscano insieme in una run del nuovo albero.

## Esito

20 trial e 14 righe di statistiche. 20/20 successi, media σ₂(Y)/σ₂(X) ≈ 1,43307 e Q₂ ≈ 0,70153. Risultati numerici identici alla prova dell’11 novembre, esclusi ID e tempi. Tempo totale medio misurato ≈ 1,724 s.

## Lettura

Stessi semi 60000–60019, quindi non sommare questi 20 trial ai 20 storici. Il valore cronometrico di un lancio non è da solo un benchmark.

## Passo successivo

Per metodo e interpretazione usare [la campagna originale](<../../../11/monte_carlo_spettrale/225535_132_JL_PCA_MC/NOTE.md>). Per il passaggio dal vecchio logging vedere [la nota del lancio delle 19:56](../195644_105_PCA_JL_MC_2/NOTE.md).

**File della prova:** [stat.csv](./stat.csv), [trial.csv](./trial.csv).
