# Indice delle prove JL e JL+PCA

Archivio FCN Lab, 10–13 novembre 2026. Percorsi verificati sul commit `8334ed6` contro l’ultimo albero fornito. I nomi di cartelle e file non sono stati cambiati.

## Percorso per la tesi

- [Crescita di k al variare di n](<2026/11/10/coefficiente_n/194656_359_JL_coeff_n/NOTE.md>)
- [Ruolo di d e frazione relativa eliminata](<2026/11/10/coefficiente_d/194259_963_JL_coeff_d/NOTE.md>)
- [Calibrazione della tabella dei k](<2026/11/10/coefficiente_n/204027_870_JL_coeff_n/NOTE.md>)
- [PCA originale su sottoinsiemi annidati](<2026/11/11/pca_originale/203831_386_legacy/NOTE.md>)
- [Confronto PCA e JL+PCA](<2026/11/11/confronti_spettrali/214942_247_JL_PCA/NOTE.md>)
- [Ripetizione del confronto su più semi](<2026/11/11/monte_carlo_spettrale/225535_132_JL_PCA_MC/NOTE.md>)
- [Stabilità dello spettro variando k](<2026/11/11/stabilita_spettrale_k/233629_422_JL_PCA_K/NOTE.md>)
- [Classificazione su cluster ben separati](<2026/11/12/classificazione/165216_969_PCA_JL_CLASS1/NOTE.md>)
- [Classificazione con D_ref = 2](<2026/11/12/classificazione/210411_282_PCA_JL_CLASS1/NOTE.md>)
- [Distorsione più bassa ma una predizione corretta in meno](<2026/11/12/classificazione/211201_253_PCA_JL_CLASS1/NOTE.md>)
- [Con D_ref = 4 migliora il gap, ma cambia il punto errato](<2026/11/12/classificazione/212722_043_PCA_JL_CLASS1/NOTE.md>)

I lanci di collaudo del 13 verificano letture e scritture. Dove riusano dataset e semi, non costituiscono nuove repliche statistiche.

## Inventario per giorno

### 10 novembre

| Run | Campagna | Nota |
|---|---|---|
| `173700_000_JL_coeff_n` | `coefficiente_n` | [Prima scansione del coefficiente variando n](<2026/11/10/coefficiente_n/173700_000_JL_coeff_n/NOTE.md>) |
| `194259_963_JL_coeff_d` | `coefficiente_d` | [Dimensione originale crescente a n fisso](<2026/11/10/coefficiente_d/194259_963_JL_coeff_d/NOTE.md>) |
| `194656_359_JL_coeff_n` | `coefficiente_n` | [Numero di punti crescente con d = 500](<2026/11/10/coefficiente_n/194656_359_JL_coeff_n/NOTE.md>) |
| `204027_870_JL_coeff_n` | `coefficiente_n` | [Tabella di k per il confronto PCA](<2026/11/10/coefficiente_n/204027_870_JL_coeff_n/NOTE.md>) |

### 11 novembre

| Run | Campagna | Nota |
|---|---|---|
| `203831_386_legacy` | `pca_originale` | [PCA centrata senza riduzione JL](<2026/11/11/pca_originale/203831_386_legacy/NOTE.md>) |
| `214513_987_JL_PCA` | `confronti_spettrali` | [Primo confronto spettrale su una combinazione](<2026/11/11/confronti_spettrali/214513_987_JL_PCA/NOTE.md>) |
| `214942_247_JL_PCA` | `confronti_spettrali` | [Che cosa cambia nello spettro PCA dopo JL?](<2026/11/11/confronti_spettrali/214942_247_JL_PCA/NOTE.md>) |
| `225535_132_JL_PCA_MC` | `monte_carlo_spettrale` | [L’effetto spettrale si ripete cambiando proiezione?](<2026/11/11/monte_carlo_spettrale/225535_132_JL_PCA_MC/NOTE.md>) |
| `233312_433_JL_PCA_K` | `stabilita_spettrale_k` | [Primo collaudo del confronto con k variabile](<2026/11/11/stabilita_spettrale_k/233312_433_JL_PCA_K/NOTE.md>) |
| `233629_422_JL_PCA_K` | `stabilita_spettrale_k` | [Quanto dello spettro si recupera aumentando k?](<2026/11/11/stabilita_spettrale_k/233629_422_JL_PCA_K/NOTE.md>) |

### 12 novembre

| Run | Campagna | Nota |
|---|---|---|
| `165216_969_PCA_JL_CLASS1` | `classificazione` | [Classificazione — Federico, k = 202](<2026/11/12/classificazione/165216_969_PCA_JL_CLASS1/NOTE.md>) |
| `185044_074_PCA_JL_CLASS1` | `classificazione` | [Classificazione — Federico, k = 400](<2026/11/12/classificazione/185044_074_PCA_JL_CLASS1/NOTE.md>) |
| `203548_602_PCA_JL_CLASS1` | `classificazione` | [Classificazione — Federico, esito osservato, k = 202](<2026/11/12/classificazione/203548_602_PCA_JL_CLASS1/NOTE.md>) |
| `203733_167_PCA_JL_CLASS1` | `classificazione` | [Classificazione — Esito Federico, k = 202](<2026/11/12/classificazione/203733_167_PCA_JL_CLASS1/NOTE.md>) |
| `210411_282_PCA_JL_CLASS1` | `classificazione` | [Classificazione — D_ref = 2, k = 202](<2026/11/12/classificazione/210411_282_PCA_JL_CLASS1/NOTE.md>) |
| `210647_722_PCA_JL_CLASS1` | `classificazione` | [Classificazione — D_ref = 2, k = 400](<2026/11/12/classificazione/210647_722_PCA_JL_CLASS1/NOTE.md>) |
| `210853_389_PCA_JL_CLASS1` | `classificazione` | [Classificazione — D_ref = 2, k = 600](<2026/11/12/classificazione/210853_389_PCA_JL_CLASS1/NOTE.md>) |
| `211201_253_PCA_JL_CLASS1` | `classificazione` | [Classificazione — D_ref = 2, k = 800](<2026/11/12/classificazione/211201_253_PCA_JL_CLASS1/NOTE.md>) |
| `212556_125_PCA_JL_CLASS1` | `classificazione` | [Classificazione — D_ref = 4, k = 202](<2026/11/12/classificazione/212556_125_PCA_JL_CLASS1/NOTE.md>) |
| `212722_043_PCA_JL_CLASS1` | `classificazione` | [Classificazione — D_ref = 4, k = 400](<2026/11/12/classificazione/212722_043_PCA_JL_CLASS1/NOTE.md>) |

### 13 novembre

| Run | Campagna | Nota |
|---|---|---|
| `195644_105_PCA_JL_MC_2` | `monte_carlo_spettrale` | [Collaudo in formato precedente](<2026/11/13/monte_carlo_spettrale/195644_105_PCA_JL_MC_2/NOTE.md>) |
| `195724_951994_PCA_JL_K_MC_0` | `stabilita_spettrale_k` | [Collaudo della stabilità spettrale variando k](<2026/11/13/stabilita_spettrale_k/195724_951994_PCA_JL_K_MC_0/NOTE.md>) |
| `201308_964093_PCA_JL_CLASS_1` | `classificazione` | [Classificazione — D_ref = 4, k = 202](<2026/11/13/classificazione/201308_964093_PCA_JL_CLASS_1/NOTE.md>) |
| `223146_830836_JL_K_C_n_0` | `coefficiente_n` | [Collaudo della stima di k al crescere di n](<2026/11/13/coefficiente_n/223146_830836_JL_K_C_n_0/NOTE.md>) |
| `232810_473574_JL_K_C_d_1` | `coefficiente_d` | [Collaudo con d crescente fino a 4000](<2026/11/13/coefficiente_d/232810_473574_JL_K_C_d_1/NOTE.md>) |
| `233402_292121_PCA_JL_2` | `confronti_spettrali` | [Collaudo del confronto PCA e JL+PCA](<2026/11/13/confronti_spettrali/233402_292121_PCA_JL_2/NOTE.md>) |
| `233503_013227_PCA_JL_MC_3` | `monte_carlo_spettrale` | [Collaudo del Monte Carlo spettrale](<2026/11/13/monte_carlo_spettrale/233503_013227_PCA_JL_MC_3/NOTE.md>) |

## Baseline operativa

Il programma cerca `JL_PCA_originale_20261111_203831_386.csv` in `data/state/` e poi in `data/state/cache/`. Nell’albero reale c’è una sola copia operativa in `data/state/cache/`. La copia fredda documenta la [prova originale](<2026/11/11/pca_originale/203831_386_legacy/NOTE.md>) e non è letta dal programma.
