# Classificazione — Federico, k = 202

12 novembre 2026, ore 16:52:16.969. ID originale nel CSV: `20261112_165216_969`.

**Ruolo:** Riferimento storico, voce 9.

## Perché questa prova

Le prove precedenti avevano mostrato che rispettare le distanze non impediva di alterare lo spettro. Restava da capire se questo cambiamento avesse un effetto pratico: i due cluster restano distinguibili quando il classificatore vede soltanto le prime due componenti? Partiamo dal dataset storico e da k = 202, già usato nel Monte Carlo spettrale. L’overlay e il rapporto separazione/raggi aiutano a vedere differenze che il solo numero di classificazioni corrette può nascondere.

## Domanda

La differenza spettrale osservata compromette la separazione dei due cluster di Federico?

## Esito

Federico. n = 160, d = 1000, k = 202, seed_X = 42, seed_A = 80000, ε = 0,50.

| Misura | PCA diretta | JL + PCA |
|---|---:|---:|
| Classificazione a centroidi sulle prime 2 PC | 160/160 | 160/160 |
| Distanza tra i centroidi nel piano PCA | 25,60605 | 27,30406 |
| Distanza / somma dei raggi RMS | 11,84375 | 8,04200 |
| Gap assiale | 24,27968 | 23,03620 |

Distorsione relativa massima 0,46484, violazioni JL 0/12720.

## Lettura

Qui l’accuratezza è satura. Il rapporto separazione/raggi scende malgrado l’aumento della distanza fra centroidi. I gruppi restano molto separati. Il leave-one-out ricalcola il centroide senza il punto, non la PCA. L’allineamento SVG serve alla visualizzazione.

## Passo successivo

La classificazione è già perfetta, ma le nuvole dopo JL sono più larghe rispetto alla separazione. Passiamo a [k = 400](<../185044_074_PCA_JL_CLASS1/NOTE.md>) con lo stesso dataset e gli stessi semi per capire se comprimere meno avvicina anche gli indicatori geometrici al riferimento.

**File della prova:** [centroidi.svg](./centroidi.svg), [overlay.svg](./overlay.svg), [points.csv](./points.csv), [summary.csv](./summary.csv).
