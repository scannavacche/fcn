---
title: "Convenzioni editoriali e notazionali"
subtitle: "Appunti ragionati di Fondamenti di Calcolo Numerico"
version: "0.1"
date: 2026-07-30
header-includes: |
  \usepackage{needspace}
  \usepackage{mathtools}
  \usepackage{cancel}
---

# Convenzioni editoriali e notazionali

Questo file raccoglie le convenzioni da mantenere negli **Appunti ragionati di Fondamenti di Calcolo Numerico**.
Lo scopo non è introdurre nuova teoria, ma stabilizzare metodo, notazione, gerarchia delle fonti e trattamento del materiale d'esame.

# 1. Obiettivo del progetto

Gli appunti devono servire contemporaneamente a due scopi:

1. costruire un testo matematicamente completo, autosufficiente e leggibile anche dopo l'esame;
2. rendere immediatamente riconoscibile il perimetro da preparare per l'orale.

Il testo non deve essere un riassunto delle dispense né una semplice trascrizione delle lezioni. Deve ricostruire il filo logico del corso rendendo espliciti i passaggi che nelle fonti sono impliciti, compressi o affidati alla spiegazione orale.

Principio guida:

> **Non diamo nulla per scontato.**

# 2. Formato sorgente

Il formato sorgente canonico è il **Markdown**.

Il PDF è un artefatto derivato:

```text
Markdown -> Pandoc -> XeLaTeX -> PDF
```

Le correzioni si applicano al `.md`, non direttamente al PDF.

Le figure prodotte ex novo devono avere, quando possibile, un sorgente modificabile separato dal file renderizzato.

# 3. Gerarchia delle fonti

La gerarchia adottata è:

1. **appunti manoscritti delle lezioni di Claudia Fassino**, per ricostruire il contenuto effettivamente esposto, l'ordine narrativo, le dimostrazioni svolte e le indicazioni sull'orale;
2. **dispense Fassino–Piana**, per struttura formale, enunciati, notazione, completezza matematica e controllo dei passaggi;
3. **libri e dispense esterne selezionati**, per chiarire dimostrazioni, motivazioni, formalismi o passaggi particolarmente compressi;
4. **rete**, solo se richiesta o necessaria per aspetti tecnici/editoriali, non come fonte matematica primaria.

Le dispense non vengono usate per ampliare in modo inconsapevole il programma d'esame, ma il materiale matematicamente utile non viene eliminato.

# 4. Perimetro dell'orale e dimostrazioni

Tutti gli enunciati, le definizioni e le dimostrazioni utili alla completezza del capitolo possono essere inclusi.

Quando una dimostrazione è presente nelle dispense ma risulta esplicitamente **non richiesta all'orale**, viene mantenuta e marcata tipograficamente:

> **Dimostrazione di approfondimento — non richiesta all'orale.**  
> Testo della dimostrazione.

Una dimostrazione svolta a lezione va invece ricostruita con particolare cura:

- esplicitare perché si introduce ogni passaggio;
- indicare il risultato precedente utilizzato;
- evitare salti algebrici non banali;
- distinguere il nucleo concettuale dalla manipolazione formale;
- chiarire che cosa conclude realmente il passaggio.

Quando negli appunti compare un'indicazione esplicita su domande frequenti o tipiche dell'orale:

> **Orale — punto da saper ricostruire.**  
> Indicazione sintetica del contenuto da saper spiegare.

Questi riquadri non sostituiscono la teoria: la teoria resta nel flusso principale.

# 5. Struttura dei capitoli

Ogni capitolo dovrebbe contenere, quando utile:

## Obiettivo

Quale problema affronta il capitolo e perché viene introdotto.

## Filo logico

Sequenza concettuale del tipo:

```text
problema -> difficoltà numerica -> idea -> risultato teorico
         -> algoritmo -> errore/stabilità -> conseguenze
```

## Corpo

Si segue la struttura delle dispense, ma la narrazione viene integrata con il contenuto delle lezioni.

Quando si introduce un nuovo oggetto, distinguere se possibile:

1. **Definizione** — che cos'è;
2. **Interpretazione** — che cosa misura o significa;
3. **Uso** — perché serve e dove verrà impiegato.

# 6. Ambienti testuali

Usare blocchi semplici compatibili con Pandoc:

> **Definizione.** Testo.

> **Teorema.** Testo.

> **Dimostrazione.** Testo.

> **Esempio.** Testo.

> **Osservazione.** Testo.

> **Richiamo.** Testo.

> **Idea chiave.** Testo.

> **Attenzione.** Testo.

> **Orale — punto da saper ricostruire.** Testo.

> **Dimostrazione di approfondimento — non richiesta all'orale.** Testo.

Gli ambienti devono avere funzione semantica, non decorativa.

# 7. Rapporto tra fonti e testo finale

Nel testo finale non devono comparire formule come:

- "negli appunti si dice...";
- "la professoressa qui aggiunge...";
- "nelle dispense invece...";
- "abbiamo deciso di...".

Le fonti guidano la redazione, ma il capitolo finale deve essere autonomo.

Se le fonti sono in conflitto o una trascrizione manoscritta è ambigua, il dubbio va segnalato durante la revisione, non nascosto.

# 8. Completezza contro sintesi

Non comprimere una dimostrazione o un algoritmo soltanto perché il risultato finale è noto.

Le formule non vanno omesse salvo:

- doppione letterale;
- passaggio del tutto meccanico e già mostrato immediatamente prima;
- materiale chiaramente estraneo all'argomento.

Una spiegazione può essere resa più leggibile, ma non impoverita.

# 9. Notazione

La notazione Fassino–Piana è il riferimento principale.

Quando una nomenclatura entra in conflitto con convenzioni consolidate di Algebra Lineare, Analisi, Probabilità o Meccanica, non si modifica arbitrariamente il corso.

Si può invece introdurre un riquadro:

> **Lessico / traduzione.**  
> Nel corso si usa ... ; in altri corsi o testi è frequente ...

Questi riquadri vanno usati solo dove la doppia nomenclatura crea un rischio concreto di confusione.

# 10. Distinzioni concettuali da preservare

In particolare, non confondere:

- **problema** e **algoritmo**;
- **condizionamento** del problema e **stabilità** dell'algoritmo;
- **errore inerente** ed **errore algoritmico**;
- errore sui dati ed errore introdotto dall'aritmetica finita;
- risultato matematico esatto e risultato `fl(...)` calcolato dal computer.

Queste distinzioni sono parte della struttura concettuale del corso, non semplici scelte lessicali.

# 11. Matrici e formule

Matrici, vettori, sistemi, fattorizzazioni e strutture a blocchi vengono ricostruiti in LaTeX.

Esempio:

```latex
\[
A=
\begin{pmatrix}
a_{11} & \cdots & a_{1n}\\
\vdots & \ddots & \vdots\\
a_{m1} & \cdots & a_{mn}
\end{pmatrix}.
\]
```

Una matrice diventa una figura solo quando la disposizione spaziale comunica informazione che la notazione matematica ordinaria non rende con sufficiente chiarezza.

# 12. Grafi, diagrammi e figure

Le figure vengono introdotte quando veicolano informazione geometrica, topologica o algoritmica non esprimibile altrettanto chiaramente con formule.

Per i **grafi degli errori**:

- sorgente preferito: Graphviz `.dot`;
- artefatti vettoriali: `.svg` per consultazione/preview e `.pdf` per la pipeline Pandoc/XeLaTeX;
- il Markdown destinato alla stampa può referenziare il `.pdf`, mantenendo `.dot` e `.svg` accanto alla figura;
- niente rasterizzazione salvo necessità;
- nodi, archi e coefficienti devono rappresentare la struttura matematica, non imitare graficamente la pagina delle dispense.

Struttura consigliata:

```text
figures/
    errore-operazione.dot
    errore-operazione.svg
```

# 13. Metadati delle lezioni

Data, docente e numero `TEORIA xx` sono metadati e non determinano la struttura editoriale del capitolo.

Possono essere conservati in una nota iniziale di lavoro, ma il testo finale segue i capitoli tematici.

# 14. Appendici delle dispense

Le note in appendice riferite a un singolo capitolo vengono conservate insieme al capitolo cui appartengono.

Editorialmente possono essere:

- collocate nel punto in cui servono come richiamo;
- mantenute in una sezione finale di approfondimento;

purché non si perda la loro origine e non si alteri il perimetro dell'orale.

# 15. Fonti esterne

Un libro esterno viene consultato con uno scopo preciso:

- decriptare una dimostrazione;
- esplicitare una motivazione;
- chiarire una convenzione;
- confrontare un algoritmo;
- colmare un buco reale.

Non si introduce materiale solo perché "interessante".

# 16. Revisione e versioni

Si lavora per **patch incrementali**.

Una versione già buona non viene riscritta da zero: si conservano le parti approvate e si modificano localmente quelle da migliorare.

Schema indicativo:

- `v0.1`: prima bozza strutturale;
- `v0.3`: integrazione fonti e matematica;
- `v0.5`: testo completo da studiare;
- `v0.6+`: revisioni editoriali e correzioni;
- `v1.0`: versione canonica finale.

# 17. Pipeline di compilazione

Comando di riferimento:

```bash
pandoc NOMEFILE.md \
  -o NOMEFILE.pdf \
  --pdf-engine=xelatex \
  -V geometry:"top=1.6cm, bottom=1.6cm, left=2cm, right=2cm" \
  --from=markdown+tex_math_single_backslash
```

La pipeline può essere estesa per la gestione delle figure vettoriali, ma il Markdown resta il sorgente canonico.
