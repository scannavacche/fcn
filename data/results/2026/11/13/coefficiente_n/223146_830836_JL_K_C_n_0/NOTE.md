# Collaudo della stima di k al crescere di n

13 novembre 2026, ore 22:31:46.830836. ID di run: `223146_830836_JL_K_C_n_0`.

**Ruolo:** Collaudo del nuovo logging, voce 4.

## Domanda

Verificare scan e summary nel nuovo albero con d = 1000, n = 10, 20, 40, 80, 160, T = 120 ed ε = 0,30, 0,40, 0,50.

## Esito

15/15 combinazioni raggiungono il target. Rispetto alla campagna del 10 novembre, sui casi comuni coincidono 14 k su 15. A n = 40 ed ε = 0,40 il primo successo è 213 invece di 207.

## Lettura

La lista delle soglie cambia la scansione condivisa e il raffinamento. Il primo k campionato che passa può cambiare anche con gli stessi semi, senza che il comportamento a un k fissato sia cambiato. La voce 6 mantiene la tabella storica.

## Passo successivo

Per la calibrazione della PCA conservare [la tabella originaria a cinque soglie](<../../../10/coefficiente_n/204027_870_JL_coeff_n/NOTE.md>).

**File della prova:** [scan.csv](./scan.csv), [summary.csv](./summary.csv).
