# Dimensione originale crescente a n fisso

10 novembre 2026, ore 19:42:59.963. ID originale nel CSV: `20261110_194259_963`.

**Ruolo:** Campagna di riferimento, voce 5.

## Domanda

A n = 50, il k che raggiunge il 95% di successo cambia se si aumenta d da 500 a 1000 e 2000? La scansione usa 120 proiezioni per caso e un tetto comune k = 499, così ogni k candidato è una riduzione per tutti i d.

## Esito

A ε = 0,30 si ha k₀ = 44 e il primo K trovato in griglia è 413 per tutti e tre i d. Il confronto a K fisso si ricava dalle righe già presenti nella scansione.

| d | Proiezioni riuscite | Media di ε empirico | Coordinate eliminate, (d − K)/d |
|---:|---:|---:|---:|
| 500 | 119/120 | 0,231235 | 17,4% |
| 1000 | 115/120 | 0,233695 | 58,7% |
| 2000 | 119/120 | 0,226452 | 79,4% |

## Lettura

La distorsione relativa sulle distanze al quadrato resta simile, non scende monotonamente. La media di ε empirico è la media, sulle 120 matrici, del massimo sulle coppie, non il massimo tra tutte le matrici. Migliora la **frazione relativa di coordinate eliminate**. È un riscontro della sufficienza di un k che non deve crescere con d, non un minimo universale. A ε = 0,20 il target non è raggiunto entro la griglia comune.

## Passo successivo

Per una verifica indipendente, scegliere K a d = 500 e rifare il controllo a K fisso con semi nuovi. La scansione del [13 novembre](<../../../13/coefficiente_d/232810_473574_JL_K_C_d_1/NOTE.md>) è un confronto utile su un altro n, non quella verifica a semi nuovi.

**File della prova:** [scan.csv](./scan.csv), [summary.csv](./summary.csv).
