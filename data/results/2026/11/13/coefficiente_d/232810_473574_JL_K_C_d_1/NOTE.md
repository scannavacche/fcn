# Collaudo con d crescente fino a 4000

13 novembre 2026, ore 23:28:10.473574. ID di run: `232810_473574_JL_K_C_d_1`.

**Ruolo:** Collaudo del nuovo logging, voce 5.

## Domanda

Verificare il nuovo archivio su n = 30, d = 500, 1000, 2000, 4000, T = 120, target 95% e soglie 0,20, 0,25, 0,30. Il tetto comune è 499, pari al più piccolo d meno uno.

## Esito

A ε = 0,30 il primo k trovato vale 309, 327, 309, 345. A ε = 0,25 vale 483, 444, 444, 475. A ε = 0,20 il target non è raggiunto entro i k provati. A k = 499, per ε = 0,30, le riuscite sono 120/120 in tutti e quattro i casi e la media dei massimi varia solo da 0,1894 a 0,1930.

Il primo k trovato a d = 500 (309, 117/120) dà 113/120 a d = 1000 e 114/120 a d = 2000. A d = 4000 non è stato provato, quindi lì non si può assegnargli un esito.

## Lettura

Il k sufficiente del lemma può essere scelto senza d, ma il primo k utile di una scansione empirica non deve coincidere per dataset diversi. A d = 1000 il confronto al 95% si gioca su una riuscita, non su una soglia teorica. Non interpolare un k non misurato.

## Passo successivo

Affiancare questa nota al [caso storico a K = 413](<../../../10/coefficiente_d/194259_963_JL_coeff_d/NOTE.md>). Se serve una verifica indipendente, fissare k prima di osservare i nuovi risultati, dichiarare i semi e misurare quel k a ogni d.

**File della prova:** [scan.csv](./scan.csv), [summary.csv](./summary.csv).
