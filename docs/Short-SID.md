---
header-includes: |
  \usepackage{needspace}
  \usepackage{endnotes}
  \let\footnote=\endnote  
---

# Problemi con le shares SMB Peer to peer

Dal punto di vista Microsoft, la clonazione senza Sysprep, anche fuori dominio, può comunque generare **SID macchina duplicati**, e con 24H2/25H2/Server 2025 gli aggiornamenti degli ultimi anni hanno introdotto un hardening esplicito: Kerberos/NTLM può rifiutare l’autenticazione tra macchine che condividono la stessa machine SID, e uno degli scenari di rottura comuni è proprio SMB (file share e stampanti).

Le note tecniche e Q\&A recenti riportano in modo abbastanza chiaro:

- Se si clonano installazioni di Windows (anche fuori dominio) senza passare da `sysprep /generalize`, le macchine risultanti condividono lo stesso **machine SID**.
- Su Windows 11 24H2/25H2, dopo specifici aggiornamenti (es. KB5064081, KB5065426), Windows applica controlli più rigidi sui SID, con effetti su Kerberos/NTLM e quindi su SMB, RDP, ecc.
- In queste condizioni, è possibile che:
    - alcuni pair di macchine continuino a funzionare,
    - altri (come il tuo trio di client verso il server reinstallato) rifiutino l’autenticazione SMB nonostante le credenziali siano corrette,
    - e che questo si manifesti solo fra macchine che condividono SID duplicati o lotti specifici di SID.
	
In altre parole: il tuo workflow Clonezilla “fuori dominio e poi join” previene conflitti di **computer account** in AD, ma non garantisce l’unicità del **machine SID**; e con gli ultimi patch, Windows ha iniziato a trattare i duplicati come condizioni di errore in autenticazione NTLM/Kerberos tra certi nodi.

Le linee guida ufficiali e i casi recenti di “SMB Sharing Fails on Windows 11 25H2” dicono esplicitamente:

- controllare se le macchine affette sono state imaged senza Sysprep;
- se sì, confermare i SID duplicati (es. confrontando il SID dell’Administrator locale senza il `-500` finale);
- la **soluzione strutturale** è rigenerare i SID tramite Sysprep /generalize sulla gold image, quindi ridistribuire e rejoinare; non c’è un “SID changer” supportato in‑place per 24H2/25H2.

## Test server:

```powershell
(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''
```
`S-1-5-21-3753021204-3844753684-2580916038`

Quello è il **machine SID** del server `590-subint01`: tutti gli account locali su quella macchina (Administrator, Users, ecc.) condividono questo prefisso, con RID diversi alla fine.

## Client KO

```powershell
(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''
```
`S-1-5-21-3753021204-3844753684-2580916038`

cioè **esattamente lo stesso machine SID del server**.

Questa è una scoperta cruciale: server e client condividono il **medesimo SID macchina**, esattamente lo scenario che la documentazione su Windows 11 24H2/25H2 segnala come causa di **fallimenti di autenticazione Kerberos/NTLM su SMB** dopo imaging/clonazione senza Sysprep.

## Client OK SID diverso

```powershell
(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''
```
`S-1-5-21-2589957146-4074959585-2131959901`, non quello del server.

\newpage

# Implicazioni dirette per il tuo problema

In sintesi:

- Il server `590-subint01` ed i client 590-subint02, 03 e 04 hanno lo stesso machine SID.
- I problemi di SMB/stampa compaiono solo tra un sottoinsieme di client e questo server reinstallato, mentre altre combinazioni funzionano.

Questo corrisponde perfettamente ai casi descritti come:

> “SMB authentication failure between domain-joined machines after deploying multiple Windows 11 24H2/25H2 devices using a cloned image with duplicate machine SIDs”.

Gli effetti tipici includono:

- prompt di credenziali ripetuto e rifiuto anche di credenziali corrette verso certe share,
- problemi selettivi tra host con SID duplicati,
- funzionamento apparentemente normale verso altri host non coinvolti.

## Cosa significa in pratica e cosa conviene fare

Dal punto di vista sistemistico:

- Il tuo parco ha almeno **una coppia server–client con SID duplicato**, e molto probabilmente più client condividono tra loro lo stesso SID (derivando dalla stessa immagine Clonezilla).
- Gli aggiornamenti recenti di Windows 11 (24H2/25H2) hanno iniziato a considerare questo scenario come **non supportato** e a bloccare o rendere inaffidabile l’autenticazione SMB/NTLM tra tali macchine.

Le linee guida ufficiali, in casi come questo, sono molto nette:

- Non è supportato l’uso di “SID changers” o workaround locali;
- La soluzione strutturale è **ricostruire l’immagine** con `sysprep /generalize` sulla macchina di riferimento, quindi ricreare server e client da immagini sysprepate, in modo che ogni macchina abbia un machine SID univoco;
- Nel frattempo, esistono talvolta GPO temporanee per mitigare il blocco, ma sono pensate come ponte verso una ricostruzione corretta, non come soluzione definitiva.

### Nel frattempo io invece direi di rifare 590-subint01 senza partire dalla stessa immagine ;)

\newpage

## SID duplicati e SMB su Windows 11 24H2 / 25H2

### Contesto tecnico

- Ogni macchina Windows ha un **machine SID** (`S-1-5-21-…`), derivato dalla SID dell’utente `Administrator` senza il suffisso `-500`.[^40_1][^40_2][^40_3]
- Questo SID viene usato da Kerberos/NTLM per identificare in modo univoco il computer nelle relazioni di trust, nell’accesso alle risorse (SMB, RDP, ecc.) e nei log di sicurezza.[^40_4][^40_5]

Con Windows 11 24H2 / 25H2 e Windows Server 2025, a partire dagli aggiornamenti di fine agosto–settembre 2025 (KB5064081, KB5065426), Microsoft ha introdotto **controlli di sicurezza aggiuntivi** che:

- rilevano i casi in cui **più dispositivi hanno lo stesso machine SID**,
- bloccano gli handshake di autenticazione Kerberos e NTLM tra tali dispositivi,
- causando errori di autenticazione su SMB, accesso a share, stampanti di rete, login interattivo o RDP.

Microsoft documenta esplicitamente che:

> “Potresti riscontrare errori di autenticazione Kerberos e NTLM tra dispositivi con SID duplicati […] dopo gli aggiornamenti rilasciati il 29 agosto 2025 e successivi. Gli aggiornamenti includono protezioni che applicano i controlli sui SID, causando errori di autenticazione quando i dispositivi hanno SID duplicati. Questa modifica blocca gli handshake di autenticazione tra tali dispositivi.”

Gli eventi correlati compaiono come ID evento 6167 del servizio LSA (`lsasrv.dll`) nel registro Sistema, e l’analisi porta spesso a scenari di clonazione/disk imaging senza Sysprep.

### Mitigazione e soluzione raccomandata

Microsoft raccomanda:

1. **Identificare i SID duplicati**
    - Usare comandi come
$(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''$
e confrontare tra macchine sospette.
2. **Soluzione strutturale**
    - Ricostruire le macchine con SID duplicati (server e client) usando metodi di clonazione **supportati**, cioè:
        - preparare una gold image con `sysprep /generalize /oobe /shutdown`,
        - catturare l’immagine dopo Sysprep,
        - distribuire nuove macchine da quella immagine;
        - ogni clone genererà un SID macchina univoco al primo boot, prima del join al dominio.
3. **Mitigazione temporanea (non definitiva)**
    - Microsoft menziona un **Criterio di gruppo speciale** che può disabilitare/attenuare i controlli sui SID per consentire il funzionamento temporaneo di ambienti con clonazioni storiche, ma è pensato solo come ponte verso una rigenerazione corretta e va ottenuto tramite supporto Microsoft.

\newpage

# Sysprep impraticabile

La tua esigenza è chiara: Sysprep “classico” è troppo distruttivo per la vostra gold image complessa, quindi volete o (A) clonare senza portarsi dietro il machine SID, oppure (B) disattivare temporaneamente l’enforcement sui SID duplicati in 24H2/25H2 in attesa di una revisione del parco.

## A) Clonare senza portarsi dietro il machine SID (senza Sysprep per OOBE)

Punto chiave: Microsoft continua a dire che **l’unico metodo supportato** per rigenerare correttamente il SID macchina è Sysprep.

> “Microsoft non supporta i computer configurati con strumenti che duplicano il SID diversi da Sysprep. Sysprep deve essere eseguito prima della cattura dell’immagine.”

Non esiste, al momento, un comando built‑in documentato che permetta di cambiare il SID macchina “in place” preservando tutto, senza generalizzare. Gli strumenti terzi che promettono “SID change senza Sysprep” (come alcuni clone tools con funzione SID changer) non sono supportati da Microsoft, e gli articoli tecnici raccomandano di non usarli in ambienti di dominio o con enforcement dei SID.

Detto questo, per avvicinarsi alle tue necessità ci sono due approcci pratici (ma comunque basati su Sysprep):

- **Gold image “post‑applicativo” + Sysprep solo sul clone**, non sull’originale:
    - mantenete una macchina “master” con tutte le applicazioni delicate installate e configurate;
    - quando serve clonare, create una **copia** della macchina master, eseguite Sysprep sulla copia, spegnete e catturate l’immagine della copia;]
    - l’originale rimane intatto (non viene generalizzato), la gold image sysprepata serve solo per deployment.
- **Pipeline di imaging a due stadi**:
    - gold image “software” (senza join a dominio, senza dati utente), sysprepata;
    - script di post‑deploy che reinstalla/configura i pezzi ultra‑specifici (licenze, binding, ecc.) dopo il primo boot, sfruttando automazione (PowerShell, MDT, Intune, ecc.).

Ma se l’installazione “delicata” non è automatizzabile e davvero non può sopportare Sysprep, l’unica strada pienamente supportata resta: **macchine installate una per una**, senza clonazione, oppure usare strumenti terzi per cambiare SID consapevoli che si entra in zona non supportata.

## B) Disattivare (temporaneamente) l’enforcement sui SID duplicati

Qui la buona notizia è che Microsoft ha previsto una **mitigazione temporanea via Group Policy** specifica per gli errori Kerberos/NTLM dovuti a SID duplicati su Windows 11 24H2/25H2 e Server 2025.

Nel documento “Errori di autenticazione Kerberos e NTLM dovuti a SID duplicati”, Microsoft dice:

> “Per una risoluzione permanente, i dispositivi con SID duplicati devono essere ricompilati con metodi supportati.
> Gli amministratori IT possono risolvere temporaneamente il problema installando e configurando uno **speciale Criterio di gruppo**.”

Altre fonti tecniche (Q\&A, blog) precisano:

- La mitigazione non è una GPO standard già presente; è una policy speciale distribuita come **Known Issue Rollback (KIR)** o pacchetto GPO legato agli update (es. KB5065426, KB5064081).
- Si ottiene **solo** aprendo un ticket con Microsoft Support per clienti business / enterprise, chiedendo esplicitamente la “special Group Policy to relax SID uniqueness enforcement” riferita all’articolo sugli errori Kerberos/NTLM con SID duplicati.
- Una volta ricevuto il pacchetto/policy:
    - viene aggiunta una voce in **Computer Configuration → Administrative Templates → System → Security Identifiers (SID) Enforcement** (il nome può variare);
    - configurandola, si disabilita o rilassa la nuova verifica sui SID duplicati, consentendo di nuovo autenticazione NTLM/Kerberos tra macchine con SID identici.
- Microsoft sottolinea che questa mitigazione è pensata **solo come ponte temporaneo**, e che la soluzione definitiva rimane rigenerare SID univoci.

In parallelo, alcuni scenari possono essere parzialmente mitigati regolando le impostazioni “Network security: Restrict NTLM” in GPO, per permettere NTLM dove era stato bloccato, ma questo agisce sul protocollo, non sul controllo di SID duplicati; la mitigazione ufficiale è quella special GPO.

\newpage 

## Cosa è realisticamente percorribile per voi

Considerando le tue due vie “percorribili”:

1) **Insegnare a clonare senza portarsi dietro il machine SID**
    - Supportato: usare Sysprep sulla copia della gold image prima del capture, così il SID viene rigenerato sui cloni, preservando l’originale con le app delicate.
    - Non supportato (ma tecnicamente possibile): usare strumenti terzi di “SID change” in‑place (es. utility che promettono cambio SID senza Sysprep); oggi, con l’enforcement, Microsoft sconsiglia e non supporta questa via in dominio.
2) **Inibire temporaneamente il controllo same‑Machine‑SID**
    - Supportato: aprire un ticket con Microsoft Support (commerciale) chiedendo la special GPO / KIR per SID duplicati, applicarla a OU/gruppi interessati per disattivare il check durante la migrazione del parco.
    - Non è una policy che puoi “inventare” da zero: va ottenuta da MS, è legata alle build 24H2/25H2/Server 2025 e agli KB specifici.

Quindi, se Sysprep è davvero impraticabile sulla gold image principale:

- **breve termine**:
    - verificare quali macchine condividono SID,
    - aprire un caso con Microsoft per ottenere e applicare la GPO speciale di mitigazione SID duplicate verso le OU dove risiedono server e client clonati,
    - ripristinare la funzionalità SMB/stampa mentre pianificate la revisione del processo di deploy.
- **medio termine**:
    - rivedere il workflow Clonezilla introducendo Sysprep sulla *copia* della gold image (non sull’originale con le app delicate),
    - migrare gradualmente i sistemi critici verso nuove installazioni con SID univoci.

\newpage


# Appendice Sysprep:

***

## Quello che dovrebbe essere fatto e che avevamo gia' provato anni fa su win 7 

- Creare una **gold image** di Windows 11 25H2 per i client (e, se volete, per i server non DC)
- garantendo che ogni macchina clonata abbia un **SID macchina univoco**, così da evitare i problemi di autenticazione Kerberos/NTLM/SMB introdotti dagli update 2025.

## Fase 1 – Preparare la macchina di riferimento

Su una macchina “di laboratorio” che diventerà la gold image:

1. Installare Windows 11 25H2 in modo pulito (ISO ufficiale, stesso SKU dei client).
2. Configurare solo ciò che deve essere uguale su tutte le macchine:
    - lingua, layout, fuso orario;
    - driver di base (chipset, storage, NIC) se comuni;
    - aggiornamenti di Windows fino alla build desiderata 25H2;
    - software di base aziendale (AV, agent di monitoraggio, Office, ecc.), evitando cose fortemente legate all’identità macchina.
3. **Non** unire la macchina di riferimento al dominio.
4. Creare un account locale amministrativo (es. `AdminLocal`) da usare per Sysprep e manutenzione dell’immagine.

## Fase 2 – Generalizzare con Sysprep

Quando la macchina di riferimento è nello stato desiderato:

1. Aprire un prompt dei comandi **elevato**.
2. Eseguire Sysprep con opzioni per imaging di produzione:
```cmd
C:\Windows\System32\sysprep\sysprep.exe /oobe /generalize /shutdown
```

- `/generalize` rimuove informazioni univoche: SID macchina, activation state, hardware‑specific info;
- `/oobe` fa sì che, al primo avvio su ogni macchina clonata, parta l’Out‑Of‑Box Experience (configurazione iniziale);
- `/shutdown` spegne il sistema una volta completato Sysprep, pronto per il capture.

Importantissimo:

- **Non riavviare** la macchina di riferimento dopo Sysprep prima di catturare l’immagine; se Windows parte, l’installazione non è più considerata “sysprepped” per quell ciclo.


## Fase 3 – Catturare l’immagine con Clonezilla

Con la macchina spenta dopo Sysprep:

1. Avviare Clonezilla da USB/ISO sulla macchina di riferimento.
2. Eseguire il capture del disco o della partizione di sistema verso un’immagine Clonezilla (FFU/WIM o formato nativo).
3. Archiviare l’immagine in un repository sicuro (NAS, server imaging) come gold image 25H2 sysprepata.

\newpage 

## Fase 4 – Distribuire nuove macchine

Per ogni nuovo client:

1. Clonare l’immagine sysprepata con Clonezilla sul nuovo hardware (PC fisico o VM).
2. Avviare la macchina:
    - Windows eseguirà OOBE, genererà un **nuovo SID macchina unico** e chiederà le informazioni base (nome PC, account iniziale, ecc.).
3. Eseguire le customizzazioni per quella macchina (nome host, eventuali driver aggiuntivi, ecc.).
4. Solo ora **unire al dominio**:
    - join via `System Properties` o `Add-Computer`,
    - verificare `nltest /sc_verify:DOMINIO` per confermare il secure channel.

## Fase 5 – Migrazione dal parco esistente

Nel tuo scenario:

- Identificare i client che condividono machine SID con il server (come hai fatto).
- Pianificare la sostituzione di:
    - il server `590-subint01` (ricreato da gold image sysprepata, configurato come print/file server, poi join a dominio),
    - i client “rotti” con SID duplicato, migrando i profili utente (profili locali o roaming) e le applicazioni se necessario.
- Dopo la migrazione, verificare:
    - che ciascuna macchina abbia un machine SID diverso (lo stesso comando `Get-LocalUser Administrator`),
    - che SMB/share e stampanti su `590-subint01` funzionino con le stesse credenziali di dominio, senza loop di autenticazione.

## Note operative

- Sysprep ha un numero limitato di “rearm” per l’attivazione (tipicamente 8 su Windows 8+/10/11): una gold image va aggiornata raramente; se superi il limite, devi creare una nuova immagine da zero.
- Evita Sysprep su macchine già in dominio; se necessario, rimuovi dal dominio prima di generalizzare.
- Se usi software di sicurezza/inventory, verifica che siano compatibili con Sysprep e imaging (alcuni agent vanno reinstallati o riconfigurati post‑clone).

