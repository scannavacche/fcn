# Quanto dello spettro si recupera aumentando k?

11 novembre 2026, ore 23:36:29.422. ID originale nel CSV: `20261111_233629_422`.

**Ruolo:** Campagna di riferimento, voce 8.

## Perché questa prova

Il Monte Carlo a k = 202 aveva mostrato che σ₂ cresceva sistematicamente in quella configurazione, senza un aumento analogo di σ₁ o dell’energia totale media. Volevamo capire se, comprimendo meno, l’eccesso vicino alla testa dello spettro si attenuasse e la coda recuperasse. Usiamo proiezioni accoppiate per seguire questa variazione, non per imporre che ogni metrica sia monotona.

## Domanda

Su n = 160 e d = 1000, confrontare k = 202, 304, 502, 800 per ognuno dei cinque semi 70000–70004. Per ogni seme si riusano le prime righe gaussiane, con scala 1/√k.

## Esito

Tutte le 20 proiezioni rispettano ε = 0,50.

| k | σ₂(Y)/σ₂(X), media | Q₂, media | Energia della coda Y/X, media |
|---:|---:|---:|---:|
| 202 | 1,41810 | 0,70730 | 0,92498 |
| 304 | 1,31544 | 0,78268 | 0,94612 |
| 502 | 1,21965 | 0,85054 | 0,96660 |
| 800 | 1,14542 | 0,88525 | 0,97631 |

## Lettura

La coda è l’energia oltre i primi sei valori singolari, rapportata alla coda originale. Nelle cinque traiettorie σ₂ relativa cala e la coda recupera, ma questo non garantisce monotonia seme per seme. La somma degli autovalori del Gram dal settimo in poi è concava come funzione del Gram. Per medie gaussiane iid la coda attesa cresce con k, per Jensen applicata alle medie leave-one-out.

## Passo successivo

La coda recupera, ma dai valori singolari non sappiamo ancora se la separazione dei due gruppi ne risenta. Passiamo alla [classificazione sulle prime due componenti](<../../../12/classificazione/165216_969_PCA_JL_CLASS1/NOTE.md>). La domanda cambia: non più soltanto come si distribuisce l’energia, ma se cambia un risultato ottenuto a valle.

**File della prova:** [stat.csv](./stat.csv), [trial.csv](./trial.csv).
