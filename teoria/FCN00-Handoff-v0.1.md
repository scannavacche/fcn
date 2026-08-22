# Handoff — Appunti ragionati di Fondamenti di Calcolo Numerico

Versione: 0.1  
Data: 2026-07-30

## Materiale da fornire per ogni capitolo

- `FCN00-Convenzioni-v0.x.md`
- ultimo capitolo approvato come modello editoriale
- capitolo corrispondente delle dispense Fassino–Piana
- pagine corrispondenti degli appunti manoscritti Fassino
- eventuali capitoli di libri o dispense esterne scelti per chiarire punti specifici
- eventuali figure/sorgenti `.dot` già prodotti

## Messaggio di handoff

Stiamo costruendo gli **Appunti ragionati di Fondamenti di Calcolo Numerico** in preparazione dell'orale.

Usa `FCN00-Convenzioni-v0.x.md` come manuale di stile e metodo.

Il **Markdown è il formato sorgente canonico**; il PDF è un artefatto compilato.

### Fonti

Usa:

1. gli appunti manoscritti delle lezioni di **Claudia Fassino** per ricostruire il contenuto effettivamente spiegato, l'ordine narrativo, le dimostrazioni svolte e le indicazioni sull'orale;
2. le dispense **Fassino–Piana** per struttura formale, enunciati, notazione, completezza matematica e controllo;
3. libri e altre dispense solo come supporto mirato per chiarire passaggi, dimostrazioni, algoritmi e convenzioni.

Non usare la rete come fonte matematica primaria salvo richiesta esplicita.

### Completezza

Principio guida: **non dare nulla per scontato**.

Preserva:

- Definizioni;
- Teoremi;
- Dimostrazioni;
- Esempi;
- Osservazioni;
- formule e passaggi matematici significativi.

Non trasformare il capitolo in un riassunto.

Se una dimostrazione delle dispense non è richiesta all'orale, **non eliminarla**: mantienila in un riquadro esplicitamente marcato:

> **Dimostrazione di approfondimento — non richiesta all'orale.**

Le dimostrazioni svolte a lezione vanno ricostruite con particolare dettaglio, esplicitando motivazioni e passaggi intermedi.

Se gli appunti segnalano una domanda tipica o un punto particolarmente caro all'orale, usa:

> **Orale — punto da saper ricostruire.**

### Narrazione

Il testo finale deve essere autonomo.

Non citare nel corpo del capitolo "gli appunti", "le dispense", "la professoressa", "il manoscritto" o il processo di redazione.

Usa le fonti dietro le quinte per ottenere un'esposizione unitaria.

Per ogni oggetto nuovo, quando possibile, rendi riconoscibili:

1. definizione;
2. interpretazione;
3. uso.

Mantieni visibile il filo:

```text
problema -> difficoltà numerica -> idea -> risultato teorico
         -> algoritmo -> errore/stabilità -> conseguenze
```

### Notazione

Segui con tranquillità la nomenclatura Fassino–Piana.

Quando emerga un conflitto reale con nomenclature consolidate di altri corsi, conserva la notazione del corso e aggiungi solo se utile un riquadro:

> **Lessico / traduzione.**

Non sostituire automaticamente un formalismo con quello più familiare.

### Grafica matematica

- matrici e strutture a blocchi: LaTeX;
- grafi di calcolo/errore: preferibilmente Graphviz `.dot` -> SVG per preview + PDF vettoriale per Pandoc/XeLaTeX;
- figure geometriche: vettoriali quando possibile;
- evitare rasterizzazione di formule e matrici.

### Revisione

Lavora per patch incrementali.

Non riscrivere da zero un capitolo già approvato.

Se una fonte è ambigua o contraddittoria, segnala il punto da revisionare anziché scegliere arbitrariamente.

## Obiettivo della singola lavorazione

Produrre una release Markdown progressivamente adatta a:

1. studio;
2. interrogazione orale;
3. conversione Pandoc/XeLaTeX;
4. conservazione come testo completo dopo l'esame.

## Compilazione

```bash
pandoc NOMEFILE.md \
  -o NOMEFILE.pdf \
  --pdf-engine=xelatex \
  -V geometry:"top=1.6cm, bottom=1.6cm, left=2cm, right=2cm" \
  --from=markdown+tex_math_single_backslash
```

Front YAML minimo:

```yaml
---
header-includes: |
  \usepackage{needspace}
  \usepackage{mathtools}
  \usepackage{cancel}
---
```
