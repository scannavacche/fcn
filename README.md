# fcn
Numerical Analysis Snippets 

## Compilare FCN Lab

Dalla directory `fcn`, con GNU make, g++ e Matplot++ installati:

- `make` compila e collega `fcn_all`. `make -j2` compila due sorgenti in parallelo.
- `make run` aggiorna il programma se necessario e lo avvia.
- `make clean` elimina `.build/` e `fcn_all`, senza toccare `data/`.

Il Makefile cerca GNPORT in `../VBD2Cpp` e usa di default le librerie del
notebook WSL. Per indicare un'altra posizione di GNPORT si può passare
`GNPORT_DIR=/percorso/VBD2Cpp`. Per il profilo lab che usa soltanto
Matplot++ e libm, passare `MATPLOT_LIBS='-lmatplot -lm'`.
`MATPLOT_CPPFLAGS` e `MATPLOT_LDFLAGS` permettono di indicare percorsi non standard.
Se si cambiano le opzioni di compilazione o link dalla riga di comando,
fare prima `make clean`.

Gli oggetti e le dipendenze dagli header sono in `.build/`. Il wrapper SVD
viene compilato una sola volta e collegato da `.build/libsvdqr.a` insieme
agli oggetti ALGLIB. `gonew` resta disponibile durante il collaudo del Makefile.
