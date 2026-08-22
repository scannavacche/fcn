<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# OS - Problemi di condivisione code di stampa

Peppe abbiamo un altro problema:

code di stampa condivise da una postazione win11 24H2 che non si riescono ne' a visualizzare in browsing delle share locali ne' a connettere da un client win11 24H2 remoto.
Se pero' accedo con client 25H2 riesco ad ottenere la lista delle shares.

C'e' casistica su questo fenomeno che potrebbe essere legato alla condivisione senza restrizioni di risorse locali?

Sì, c’è casistica specifica su problemi di accesso a share (incluse code di stampa condivise) fra macchine Windows 11 24H2, e in diversi casi è effettivamente legata alle nuove policy SMB e alle restrizioni sulle condivisioni “aperte” (guest / Everyone) introdotte o irrigidite con gli ultimi update.[^1_1][^1_2][^1_3][^1_4]

***

## Quadro del problema

Nel tuo scenario hai:

- Server: Windows 11 24H2 che espone shares di code di stampa.
- Client A: Windows 11 24H2 che non vede né le share né riesce a connettersi.
- Client B: Windows 11 25H2 che invece naviga correttamente le stesse share.

Questa asimmetria è coerente con due fenomeni noti:

- Aggiornamenti di 24H2 che modificano configurazione SMB client/server, disabilitando guest e logon insicuri di default, rompendo l’accesso a risorse condivise fra 24H2.[^1_2][^1_3][^1_1]
- Fix o cambiamenti in 25H2 che rendono la discovery e l’accesso alle share più robusti (o semplicemente con impostazioni leggermente diverse).[^1_5][^1_4]

***

## Policy SMB e condivisioni “senza restrizioni”

È emerso più volte che la condivisione “senza restrizioni” basata su Everyone / guest / anonimous, che funzionava su versioni precedenti, dopo 24H2 può fallire in silenzio sui client, pur restando visibile o accessibile da macchine con configurazioni leggermente diverse (come il tuo 25H2).[^1_6][^1_3][^1_1]

Alcuni elementi rilevanti:

- **SMB guest e logon insicuri**
    - Opzioni SMB client tipo `EnableInsecureGuestLogons` e `AllowInsecureGuestAuth` vengono spesso forzate a `false` con gli aggiornamenti, rompendo l’accesso alle condivisioni senza autenticazione.[^1_1]
    - Analogamente la richiesta di firma (Security Signature) può essere attivata solo da una parte, generando incompatibilità.[^1_1]
- **Condivisione protetta da password**
    - Se su un PC la “condivisione protetta da password” è abilitata e sull’altro no (o gli account effettivi non coincidono), spesso si ottiene il comportamento che descrivi: browsing che non mostra le share, o tentativi di connessione che falliscono in modo poco trasparente.[^1_6][^1_1]
- **Servizi di rete e individuazione**
    - Su 24H2 sono stati segnalati problemi di rete in cui il PC “non viene visto” sulla LAN, con ricadute su condivisione di file e stampanti; a volte la causa è la configurazione dei servizi (es. servizio di individuazione rete, o servizi SMB in avvio “Manuale (avvio trigger)” invece che “Automatico”).[^1_2]

***

## Cosa è stato documentato come workaround

Dalla casistica (blog tecnici, Q\&A Microsoft, articoli di troubleshooting) si vedono due strade principali.[^1_7][^1_3][^1_8][^1_4][^1_6][^1_1]

### 1. Ripristinare una condivisione “classica” ma comunque autenticata

È la via più pulita, consigliabile in ambiente di produzione:

- Allineare su **tutti** i PC le “Impostazioni di condivisione avanzate”:
    - Attiva individuazione rete, attiva condivisione file e stampanti, e assicurati che tutti i profili di rete usati (Privata/Dominio) siano coerenti.[^1_7][^1_6]
- Creare un **account locale** sul server di stampa (24H2) con password, e usare lo stesso username/password sul client 24H2, oppure usare credenziali esplicite in Windows Credential Manager, dando a tale utente permessi su share e NTFS della stampante/cartella (oppure permessi a Everyone se accetti il rischio ma con accesso comunque autenticato).[^1_9][^1_1]
- Verificare che **condivisione protetta da password** abbia lo stesso stato su tutte le macchine (o meglio, tenerla attiva e usare account/credenziali coerenti).[^1_6][^1_1]

Questo riduce la dipendenza da guest/anonymous e interagisce meglio con le policy di sicurezza introdotte negli ultimi build.

### 2. Riabilitare esplicitamente guest / logon insicuri (sconsigliato ma usato come test)

Su vari forum/guide, per ripristinare l’accesso a condivisioni non protette dopo aggiornamenti di Windows 11 24H2/25H2, suggeriscono temporaneamente:[^1_1]

- Da PowerShell elevata sul client:
    - `Set-SmbClientConfiguration -EnableInsecureGuestLogons $true -Force`
    - `Set-SmbClientConfiguration -RequireSecuritySignature $false -Force`
- Sul server:
    - `Set-SmbServerConfiguration -RequireSecuritySignature $false -Force`
- E registro:
    - `HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters\AllowInsecureGuestAuth = 1`
    - In alcuni casi anche livelli LMCompatibility abbassati (`LmCompatibilityLevel = 1`).

Questa strada conferma se il problema è davvero legato alla **condivisione senza restrizioni**; una volta verificata la diagnosi, di solito conviene tornare su un modello autenticato e rimuovere queste impostazioni “insecure”.

***

## Peculiarità note di 24H2 vs 25H2

Su 24H2 ci sono segnalazioni di:

- Condivisioni che smettono di essere visibili dopo specifici aggiornamenti cumulativi (esempi riportati per aggiornamenti come KB5065426 per 24H2/25H2).[^1_1]
- Host 24H2 non visibili agli altri PC (scoperta rete e SMB a metà), risolvibili talvolta cambiando tipo di avvio di alcuni servizi da “Manuale (avvio trigger)” a “Automatico”.[^1_2]
- Q\&A Microsoft che confermano “problemi di condivisione di file in rete tra PC da quando ci sono stati aggiornamenti di Windows”, in particolare tra 24H2 e altre build.[^1_3][^1_4]

È quindi plausibile che:

- Il client 25H2 abbia già un fix o comunque una combinazione di impostazioni che gli permette di parlare correttamente con il server 24H2.[^1_4][^1_5]
- Il client 24H2, invece, rimanga “spezzato” per via di configurazione SMB più restrittiva o servizi non allineati, e questo si manifesta proprio sulla discovery e sull’accesso alle share di stampa.

***

## Cosa ti suggerirei di fare in pratica

Per restringere il campo e capire se è davvero “colpa” della condivisione senza restrizioni:

1. Dal client 24H2 che non vede le share, prova accesso diretto a:
    - `\\NOME-PC-24H2\` e `\\IP-PC-24H2\` invece del browsing.[^1_10]
    - Se chiede credenziali, usa un account locale presente sul server.
2. Allinea le impostazioni di rete e condivisione:
    - Verifica da Control Panel → Centro connessioni di rete e condivisione → Impostazioni di condivisione avanzate che su tutti i 24H2 le opzioni (individuazione rete, condivisione file e stampanti, condivisione protetta da password) siano identiche.[^1_7][^1_6]
3. Crea temporaneamente una share “di test” su 24H2 con permessi espliciti:
    - Nuova cartella condivisa con permessi NTFS e share per un utente specifico, non Everyone/guest.[^1_9][^1_1]
    - Verifica se 24H2 ↔ 24H2 funziona in questo scenario: se sì, il problema è proprio la condivisione “aperta”.
4. Solo come debugging aggressivo (non come soluzione finale):
    - Abilita `EnableInsecureGuestLogons` e `AllowInsecureGuestAuth` sul client 24H2 e vedi se compare la lista di share.[^1_1]

Se con questi passi il 24H2 torna a vedere le share, puoi concludere con buona confidenza che la causa è una combinazione di:

- Hardening SMB sui 24H2,
- Uso di condivisioni senza autenticazione / Everyone,
- Eventuali servizi di rete non allineati.

Per darti indicazioni più mirate, mi puoi dire se sulle share di stampa siete attualmente in modalità “Everyone / nessuna password” o se usate già utenti/password (dominio o locali)?
<span style="display:none">[^1_11][^1_12][^1_13][^1_14][^1_15]</span>

<div align="center">⁂</div>

[^1_1]: https://turbolab.it/windows-10/come-ripristinare-condivisione-cartelle-windows-11-4459

[^1_2]: https://www.punto-informatico.it/windows-11-24h2-bsod-problemi-rete/

[^1_3]: https://learn.microsoft.com/it-it/answers/questions/3919170/condivisione-files-in-rete-win11

[^1_4]: https://learn.microsoft.com/it-it/answers/questions/5590843/accesso-a-risorse-condivise-da-windows-11-pro-25h2

[^1_5]: https://synsphere.it/notizie/windows-11-25h2-aggiornamento-guida-pmi/

[^1_6]: https://learn.microsoft.com/it-it/answers/questions/5566176/problemi-di-connessione-tra-2-pc-nuovi-con-win11-p

[^1_7]: https://support.microsoft.com/it-it/windows/condivisione-di-file-in-una-rete-in-windows-b58704b2-f53a-4b82-7bc1-80f9994725bf

[^1_8]: https://support.microsoft.com/it-it/windows/risolvere-i-problemi-di-connessione-e-stampa-in-windows-fb830bff-7702-6349-33cd-9443fe987f73

[^1_9]: https://www.youtube.com/watch?v=sIL8NhZ0vhM

[^1_10]: https://trueimagetech.it/blogs/all/come-condividere-la-stampnate-in-rete-una-semplice-guida

[^1_11]: https://www.ilsoftware.it/focus/aiuto-non-riesco-a-vedere-le-cartelle-condivise-in-windows_21048/

[^1_12]: https://www.youtube.com/watch?v=IyCwCSOSmto

[^1_13]: https://support.microsoft.com/it-it/windows/correggere-il-processo-di-stampa-bloccato-negli-errori-in-coda-in-windows-9ee21bad-4488-45b5-b497-2d8299bd74aa

[^1_14]: https://www.youtube.com/watch?v=QMs3bavpNlg

[^1_15]: https://learn.microsoft.com/it-it/answers/questions/2615999/configurazione-impostazione-di-condivisione-avanza?page=1


---

```
# <q>Dal client 24H2 che non vede le share, prova accesso diretto a:\\NOME-PC-24H2\ e \\IP-PC-24H2\ invece del browsing.Se chiede credenziali, usa un account locale presente sul server.</q> e' quello che ho provato ma rifiuta credenziali sia di utenti di dominio che locali
```

Allora sei già nel caso “peggiore”: non solo non vedi il browse, ma il 24H2 rifiuta anche credenziali valide (dominio e locali), quindi il problema è proprio a livello di autenticazione SMB, non di semplice guest/browsing.[^2_1][^2_2][^2_3]

***

## Cosa indica il rifiuto delle credenziali

Su 24H2 ci sono diverse segnalazioni in cui:

- Accesso a share su altro 24H2 fallisce con prompt credenziali ripetuto o “nome utente o password non corretta”, anche usando esattamente le credenziali del target.[^2_3][^2_1]
- In ambienti senza dominio o con scenari misti, la combinazione di:
    - richiesta SMB signing,
    - blocco guest fallback,
    - restrizioni su NTLM/LMCompatibility
porta esattamente al comportamento che descrivi.[^2_4][^2_5][^2_1]

Il fatto che il 25H2 si autentichi e il 24H2 no suggere fortemente che sul client 24H2 hai un profilo SMB più “duro” (policy locali o GPO) che impedisce sia guest sia NTLM “tradizionale”.[^2_1][^2_4]

***

## Controlli mirati lato client 24H2

Ti metto una check‑list sintetica, da fare sul 24H2 che non entra, assumendo che il server sia o in workgroup o in dominio ma comunque funzionante (visto che il 25H2 ci entra):

1. **Pulizia Credential Manager**
    - Apri “Gestione credenziali” e rimuovi tutte le credenziali Windows relative a quel server (nome host e IP).[^2_6]
    - Riavvia e riprova l’accesso con `\\server` inserendo credenziali “fresche”.
2. **AllowInsecureGuestAuth (test guest)**
Anche se tu vuoi usare credenziali, questo test ti dice se sei bloccato da policy “no guest”:
    - Registro sul client 24H2:
        - `HKLM\SOFTWARE\Policies\Microsoft\Windows\LanmanWorkstation`
            - `AllowInsecureGuestAuth` = `1` (DWORD)[^2_2][^2_5]
    - In parallelo, da PowerShell elevata:
        - `Set-SmbClientConfiguration -EnableInsecureGuestLogons $true`
    - Riavvia il client e verifica se almeno l’accesso guest parte.[^2_5][^2_4]

Se con questo l’accesso cambia (per esempio riesci a entrare o l’errore diventa diverso), hai confermato che 24H2 stava bloccando pesantemente guest/NTLM.
3. **SMB signing e livelli di sicurezza LAN Manager**

Molti casi su 24H2 si risolvono disattivando la richiesta obbligatoria di firma SMB lato client e allentando LAN Manager Authentication Level.[^2_7][^2_4][^2_5][^2_3]
    - Da PowerShell elevata sul client 24H2:
        - `Set-SmbClientConfiguration -RequireSecuritySignature $false`[^2_4][^2_5]
    - Oppure via .reg (equivalente a livello di registro):[^2_7]
        - `HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters`
            - `EnableSecuritySignature` = 1
            - `RequireSecuritySignature` = 0
    - In `secpol.msc` / `gpedit.msc` (se non siete sotto GPO dominante):
        - Criteri locali → Opzioni di sicurezza →
            - “Client di rete Microsoft: firma digitale le comunicazioni (sempre)” = Disabilitato.[^2_4]
            - “Sicurezza di rete: livello di autenticazione LAN Manager” → tipicamente “Invia solo risposta NTLMv2”.[^2_8][^2_1]
4. **Scenario dominio vs workgroup**
    - Se siete in dominio:
        - Verifica che il client sia correttamente joinato, orologio sincronizzato, nessun errore Kerberos/NTLM nel registro (ID 4625, 4776).[^2_8]
        - Verifica che stai usando davvero credenziali di dominio `DOMINIO\utente` e non un locale omonimo.[^2_9][^2_8]
    - Se siete in workgroup:
        - Usa un utente LOCALE del server, forzando il formato `SERVER\utente`.
        - Su 24H2 in workgroup è documentato che se “l’host richiede SMB signing e l’altro no”, l’accesso fallisce come se fossero sbagliate le credenziali.[^2_5][^2_1][^2_4]

***

## Peculiarità specifiche per stampanti condivise

Oltre al lato SMB puro, per la condivisione stampanti ci sono due tweak che a volte servono sui client moderni:

- Chiave `RpcAuthnLevelPrivacyEnabled` su client Windows 11 per problemi di connessione a stampanti condivise:
    - `HKLM\SYSTEM\CurrentControlSet\Control\Print`
        - `RpcAuthnLevelPrivacyEnabled` = `0` (DWORD)[^2_10]
    - Riavvio dello spooler (“Spooler di stampa”).[^2_11][^2_10]

Questo è più stampante‑specifico (RPC), ma combinato con SMB hardening può spiegare perché la 25H2 entra e la 24H2 no.

***

## Strategie di debug concrete per il tuo caso

Ti suggerirei questa sequenza molto pragmatica sul client 24H2:

1. Svuota credenziali relative al server.[^2_6]
2. Applica su 24H2:
    - `Set-SmbClientConfiguration -RequireSecuritySignature $false`
    - `Set-SmbClientConfiguration -EnableInsecureGuestLogons $true`
    - `reg add HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f`[^2_2][^2_5][^2_4]
3. Riavvia il client 24H2 e prova di nuovo `\\SERVER` e `\\IP`.
    - Se ora accede (magari anche senza chiedere credenziali), hai la conferma che 24H2 blocca per policy la combinazione guest/NTLM e/o firma.
4. A questo punto “chiudi il cerchio”:
    - Decidi se vuoi una condivisione pulita (account con password, LAN Manager su NTLMv2, firma SMB coerente) e disabilita poi le opzioni insicure, oppure accetti consapevolmente il profilo più aperto.

***

Ti torna se facciamo un passo alla volta partendo da `Set-SmbClientConfiguration` sul client 24H2 e da `AllowInsecureGuestAuth` nel registro, così vediamo se almeno cambia il tipo di errore (da “credenziali rifiutate” a altro)?
<span style="display:none">[^2_12][^2_13][^2_14][^2_15]</span>

<div align="center">⁂</div>

[^2_1]: https://learn.microsoft.com/es-mx/answers/questions/5664154/windows-11-24h2-fallo-de-autenticaci-n-smb-(error

[^2_2]: https://learn.microsoft.com/it-ch/answers/questions/2196712/buongiorno-con-laggiornamento-di-windows-24h2-non

[^2_3]: https://learn.microsoft.com/it-it/answers/questions/2190584/immettere-le-credenziali-di-rete-nome-utente-o-pas

[^2_4]: https://angolodiwindows.com/2024/12/come-risolvere-problemi-smb-windows-11-24h2-nas/

[^2_5]: https://www.ilsoftware.it/focus/errore-0x80070035-in-windows-non-permette-di-accedere-alle-risorse-di-rete/

[^2_6]: https://www.reddit.com/r/unRAID/comments/1o4o3g5/winows_11_24h2_has_issues_connecting_to_unraid/

[^2_7]: https://www.trickit.it/windows/windows-11-24h2-errore-crittografia-accesso-share

[^2_8]: https://learn.microsoft.com/en-ca/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^2_9]: https://manuals.konicaminolta.eu/bizhub-C4051i-C3351i/IT/contents/TS_07_03.html

[^2_10]: https://www.reddit.com/r/WindowsHelp/comments/1cvyooi/cant_access_a_shared_printer_from_windows_11_pc/

[^2_11]: https://support.microsoft.com/it-it/windows/risolvere-i-problemi-di-connessione-della-stampante-condivisa-in-windows-27f7b136-02e6-406a-9a25-fc87838062b8

[^2_12]: https://www.ilsoftware.it/focus/dopo-laggiornamento-a-windows-11-cartelle-condivise-e-stampanti-non-si-vedono-come-risolvere-subito/

[^2_13]: https://learn.microsoft.com/it-it/answers/questions/4118366/problema-condivisione-stampante-in-windows-11

[^2_14]: https://www.reddit.com/r/sysadmin/comments/1or1lk4/windows11_shared_folderprinters_domain_auth_not/

[^2_15]: https://learn.microsoft.com/it-ch/answers/questions/5590843/accesso-a-risorse-condivise-da-windows-11-pro-25h2


---

# Registro sul client 24H2:

HKLM\SOFTWARE\Policies\Microsoft\Windows\LanmanWorkstation

E se non c'e' la chiave LanManWorkstation ?

Se la chiave LanmanWorkstation non c’è sotto Policies è normale: significa solo che non hai ancora alcuna policy esplicita impostata lì; in quel caso la procedura corretta è proprio crearla “da zero”.[^3_1][^3_2][^3_3]

***

## Come procedere se LanmanWorkstation non esiste

Sul client 24H2:

1. Apri l’Editor del Registro (`regedit`).
2. Vai a:
`HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows`
3. Se sotto `Windows` non esiste la chiave `LanmanWorkstation`:
    - Tasto destro su `Windows` → Nuovo → Chiave → nome: `LanmanWorkstation`.[^3_2][^3_1]
4. Seleziona `LanmanWorkstation`, poi:
    - Tasto destro → Nuovo → Valore DWORD (32 bit) → nome: `AllowInsecureGuestAuth`.[^3_4][^3_1][^3_2]
    - Doppio clic su `AllowInsecureGuestAuth` → Dati valore = `1` → OK.[^3_1][^3_2]
5. Riavvia il PC o almeno il servizio “Workstation” per essere sicuro che la policy venga riletta.

Questa chiave sotto `SOFTWARE\Policies` è l’equivalente “forzato da policy di gruppo” del parametro, ma Windows 11 24H2 controlla anche la posizione classica sotto `SYSTEM`:[^3_5][^3_3][^3_4]

- `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters`
    - `AllowInsecureGuestAuth` = `1` (DWORD, se assente lo crei).[^3_3][^3_5][^3_4]

Puoi quindi settarlo in entrambe le posizioni (Policies e Parameters) per evitare ambiguità, poi riavvia e riprova l’accesso `\\SERVER` / `\\IP`.[^3_5][^3_3][^3_1]

***

## Nota su policy vs registro

In alternativa (o in aggiunta), lo stesso flag si può impostare via `gpedit.msc`:

- Configurazione computer → Modelli amministrativi → Rete → Workstation Lanman →
“Abilita gli accessi guest non sicuri” = Attivata.[^3_6][^3_3][^3_1]

Questo crea le voci appropriate sotto `SOFTWARE\Policies\Microsoft\Windows\LanmanWorkstation` in modo “ufficiale”, che è spesso preferibile al puro editing manuale del registro.[^3_6][^3_3][^3_1]

Se dopo aver creato LanmanWorkstation e AllowInsecureGuestAuth=1 il 24H2 continua a rifiutare le credenziali, il passo successivo sarà agire su `RequireSecuritySignature` e sulle opzioni di “Client di rete Microsoft: firma digitale (sempre)” su quel client; vuoi che ti schematizzi anche quei due tweak in modo altrettanto operativo?
<span style="display:none">[^3_10][^3_11][^3_12][^3_13][^3_14][^3_15][^3_7][^3_8][^3_9]</span>

<div align="center">⁂</div>

[^3_1]: https://learn.microsoft.com/it-it/answers/questions/2196712/buongiorno-con-laggiornamento-di-windows-24h2-non?forum=windowsclient-all

[^3_2]: https://www.asus.com/it/support/faq/1054736/

[^3_3]: https://www.windowsblogitalia.com/2024/08/windows-11-errore-nas-hdd-smb/

[^3_4]: https://www.reddit.com/r/sysadmin/comments/1j2vce0/smb_access_issues_after_24h2_update/

[^3_5]: https://www.quattr.in/wordpress/2024/10/16/windows-11-24h2-disattivare-smb-signing-e-attivare-guest-fallback/

[^3_6]: https://microsofters.com/software/windows-11/como-resolver-los-errores-de-nas-con-smb-en-windows-11-24h2/

[^3_7]: https://www.trickit.it/windows/windows-11-24h2-errore-crittografia-accesso-share

[^3_8]: https://learn.microsoft.com/en-us/answers/questions/2187042/about-the-network-credentials-in-windows-11-new-up

[^3_9]: https://learn.microsoft.com/pt-br/windows/client-management/mdm/policy-csp-LanmanWorkstation

[^3_10]: http://tecnopcecuador.blogspot.com/2024/12/configurar-el-uso-compartido-de.html

[^3_11]: https://nnmclub.to/forum/viewtopic.php?t=1757024

[^3_12]: https://www.facebook.com/groups/windows11ita/posts/1937508356795289/

[^3_13]: https://www.reddit.com/r/unRAID/comments/1hhl28h/windows_11_24h2_and_cannot_login_smb_without/

[^3_14]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/enable-insecure-guest-logons-smb2-and-smb3

[^3_15]: https://www.youtube.com/watch?v=SwUlegjtSC0


---

# nulla di tutto cio' ha funzionato. Se vogliamo far euna prova mettendoci nelle stesse condizioni dei client che funzionano, effettuo l'aggiornamento a win11 25H2 e poi applico cumulative update identico a quello dei client funzionanti, poi ne riparliamo.

A quel punto, avrai idea di cosa andare a cercare modificato dagli aggiornamenti, nel caso tutto funzionasse?

Ha senso fare l’esperimento: se dopo upgrade a 25H2 + stesso cumulative dei client “buoni” tutto funziona, abbiamo un forte indizio che il problema sia una combinazione specifica di patch/policy su 24H2, non qualcosa di “ambientale” (DNS, firewall, AD, ecc.).[^4_1][^4_2][^4_3]

***

## Cosa cambia tra 24H2 e 25H2 (rilevante per SMB)

A livello architetturale, 24H2 e 25H2 condividono la stessa piattaforma (“Germanium”); 25H2 è sostanzialmente un enablement package che attiva feature e porta qualche fix, ma non un nuovo kernel/stacks completamente diversi.[^4_2][^4_3][^4_1]
Questo implica che:

- Tutte le modifiche allo stack SMB (firma obbligatoria, guest bloccato, enforcement NTLM/NTLMv2, ecc.) sono di fatto comuni alle due versioni e arrivano tramite gli stessi cumulative update.[^4_3][^4_1][^4_2]
- Differenze reali tra due macchine 24H2 e 25H2 “parimenti patchate” si riducono spesso a: feature enable/disable, nuove GPO disponibili, qualche bugfix specifico (ad esempio sul supporto SMBv1 o su NetBIOS over TCP/IP).[^4_4][^4_5][^4_3]

In pratica, se 25H2 + stesso cumulative del resto della flotta funziona, il sospetto principale diventa:

- la macchina 24H2 precedente aveva:
    - o patch/mancanza di patch intermedie (una sequenza di KB differente),
    - o policy locali/di dominio divergenti (security baseline, nuove GPO applicate solo ad alcune versioni/OU),
    - o stato “sporco” di credenziali/registry che il re‑install + enablement package risolve di fatto “per pulizia”.

***

## Cosa andare a cercare se 25H2 funziona

Mettiamo che:

1. Porti il PC problematico a Windows 11 25H2.
2. Applichi esattamente lo stesso cumulative update dei client che già accedono correttamente alle share (stesso build number).[^4_6][^4_7]
3. Verifichi che ora:
    - vedi il server in browsing,
    - `\\SERVER` e `\\IP` accettano credenziali e mostrano le share di stampa.

In questo caso, le direzioni interessanti da analizzare sono:

### 1. Differenze di patch level e KB “sensibili a SMB”

Un’idea utile è confrontare (prima e dopo upgrade) la lista dei KB relativi a:

- hardening SMB,
- enforcement firma SMB,
- problemi noti con SMBv1/NetBT e fix associati.[^4_8][^4_5][^4_4]

In part., Microsoft ha confermato problemi e fix che toccano:

- impossibilità di usare SMBv1 su NetBIOS/NetBT dopo certi update;[^4_5][^4_4]
- enforcement della firma lato client che rompe l’accesso a share più “legacy”, risolto disattivando il requisito via GPO o patch successive.[^4_9][^4_8]

Se 25H2 funziona e 24H2 no, ma condividono la stessa linea di servicing, è probabile che:

- su 24H2 ti mancasse un cumulative “critico”, oppure
- su 24H2 un cumulative intermedio avesse introdotto un bug che 25H2 (o un successivo cumulative) ha fixato.

Da qui, si possono isolare i KB “di mezzo” (ad es. guardando la sequenza di KB installati nella cronologia) e capire quale introduce il cambio di comportamento SMB.

### 2. Differenze di policy / GPO dopo l’upgrade

25H2 introduce e/o abilita nuove GPO e baseline di sicurezza, ma la cosa importante è: molte di queste GPO sono *disponibili* anche in 24H2, solo che potrebbero non essere state applicate allo stesso modo.[^4_10][^4_11][^4_12]

Se dopo l’upgrade la macchina “problematica” si comporta come i client buoni, io andrei a confrontare:

- `gpresult /H report.html` (o `gpresult /SCOPE COMPUTER /V`) del client prima e dopo l’upgrade (o almeno con un 24H2 che non va vs un 25H2 che va), guardando in particolare:
    - Criteri di sicurezza → opzioni SMB (firma sempre/mai; guest; NTLM SSP “richiedi crittografia a 128 bit”, LAN Manager authentication level).[^4_8][^4_9]
- Il ramo registro relativo a LanmanWorkstation e alle opzioni di sicurezza di rete:
    - `HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters`
    - `HKLM\SOFTWARE\Policies\Microsoft\Windows\LanmanWorkstation`
    - `HKLM\SYSTEM\CurrentControlSet\Control\Lsa` (NTLM, LMCompatibilityLevel, ecc.).[^4_13][^4_9][^4_8]

Se trovi, ad esempio, che dopo l’upgrade alcune voci vengono riportate a default o rimosse (es. `RequireSecuritySignature` o opzioni per la privacy RPC di stampa), hai identificato l’elemento che “sbloccava” l’accesso.[^4_14][^4_9][^4_13]

### 3. Controllo SIDs duplicati e sicurezza NTLM/Kerberos

Con 24H2/25H2 Microsoft ha iniziato a essere più rigida sui SID duplicati a livello di macchina; sistemi clonati senza sysprep possono non autenticare più correttamente su NTLM/Kerberos.[^4_10]
Se il client problematico deriva da un’immagine “clonata” e gli altri no, e l’upgrade a 25H2 risolve, potrebbe essere che il processo di upgrade abbia rigenerato qualche aspetto legato alla sicurezza, oppure che le nuove regole SID siano implementate in modo leggermente diverso.

In questo caso andare a confrontare:

- SID macchina (`whoami /user`, `wmic useraccount get name,sid` per locali),
- eventuali log eventi sicurezza (4625, 4776) durante i tentativi di accesso, rispetto a una macchina funzionante.[^4_10]

***

## Come strutturare la “fase 2” dell’analisi, se 25H2 funziona

Se dopo l’upgrade tutto funziona, la strategia che ti proporrei è:

1. Su una macchina 24H2 ancora “rotta”, salvare:
    - `gpresult /H 24h2_rotta.html`
    - export dei rami registro SMB/LSA rilevanti.
2. Su quella upgrade a 25H2 + cumulative e, una volta verificato che funziona, salvare:
    - `gpresult /H 25h2_ok.html`
    - export degli stessi rami registro.
3. Confrontare offline (anche con strumenti diff) i due stati, per identificare:
    - chiavi/valori scomparsi o modificati,
    - policy che passano da “Configured” a “Not configured”.

A quel punto sì, ti posso aiutare a “mappare” esattamente quali impostazioni sono cambiate e che cosa conviene replicare via GPO/registry sulle rimanenti macchine 24H2 (se ne avrai ancora), senza necessariamente passare tutte a 25H2.

Se ti torna, quando hai fatto upgrade + cumulative e verifichi il comportamento, puoi riportarmi:

- build esatta prima/dopo,
- se l’accesso alle share funziona pienamente da 25H2,
e poi possiamo ragionare su quali rami di registro e policy diffare in modo sistematico.
<span style="display:none">[^4_15][^4_16][^4_17]</span>

<div align="center">⁂</div>

[^4_1]: https://blogs.windows.com/windows-insider/2025/08/29/releasing-windows-11-version-25h2-to-the-release-preview-channel/

[^4_2]: https://www.windowscentral.com/software-apps/windows-11/windows-11-version-25h2-faq

[^4_3]: https://pureinfotech.com/windows-11-25h2-vs-24h2/

[^4_4]: https://www.windowslatest.com/2025/10/01/microsoft-confirms-four-issues-in-windows-11-25h2-but-theyre-not-a-dealbreaker/

[^4_5]: https://allthings.how/windows-11-25h2-safety-check-who-should-upgrade-now/

[^4_6]: https://vpncentral.com/microsoft-releases-kb5083769-for-windows-11-25h2-and-24h2-with-security-fixes-and-reset-bug-repair/

[^4_7]: https://learn.microsoft.com/en-us/windows/release-health/windows-message-center

[^4_8]: https://www.reddit.com/r/truenas/comments/1fh3dqh/latest_windows_11_update_may_break_smb_shares/

[^4_9]: https://learn.microsoft.com/en-us/answers/questions/5575853/problems-with-windows-11-pro-update-to-25h2

[^4_10]: https://www.wincert.net/cast/windows-11-25h2-tightens-enterprise-security-with-new-sid-rules/

[^4_11]: https://www.cyber.gov.au/sites/default/files/2026-01/change_log_-_hardening_microsoft_windows_11_workstation_january_2026.pdf

[^4_12]: https://4sysops.com/archives/new-windows-11-25h2-group-policy-settings/

[^4_13]: https://www.windowsblogitalia.com/2024/08/windows-11-errore-nas-hdd-smb/

[^4_14]: https://www.trickit.it/windows/windows-11-24h2-errore-crittografia-accesso-share

[^4_15]: https://learn.microsoft.com/en-us/answers/questions/5576910/25h2-and-24h2

[^4_16]: https://www.linkedin.com/posts/nishan-shrestha-0069a914a_why-windows-11-25-h2-updates-got-heavy-activity-7417851574271496193-Z6X6

[^4_17]: https://www.youtube.com/watch?v=ViC4Ph5Kq2g


---

# Ho aggiornato a 25H2 stessa build dei miei due client funzionanti eppure continua a non accedere alle share delle stampanti.

Ho anche riportato su un client queste impostazioni che usiamo sul server, ma non hanno funzionato:

- SU WINDOWS 10 BISOGNA AGGIUNGERE UNA CHIAVE DI REGISTRO SU:
percorso HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Print.
 clic destro su Stampa e scegli New > DWORD-32 bit Valore.
Assegna un nome alla voce appena creata in questo modo RpcAuthnLevelPrivacyEnabled.
Fai clic destro su di esso e scegli modificare. Cambia il numero sotto Valore di a 0 e fare clic su OK per salvare le modifiche.
- SU WINDOWS 11, OLTRE CHE QUESTA CHIAVE DA AGGIUNGERE,  BISOGNA ABILITARE, DA GPEDIT.MSC ----> CONFIGURAZIONE COMPUTER ---> MODELLI AMMINISTRATIVI ---> STAMPANTI ---->, LE SEGUENTI IMPOSTAZIONI:
"Configura impostazioni listerner RPC" ------> Attivita e sotto dove c'è "Protocolli per consentire connessioni RCP in ingresso ------> RPC su named pipe e TCP
"Ricerca stampanti" ----------> Attivata.

Il fatto che ora tu sia su 25H2 stessa build degli altri e continui a non accedere, mentre gli altri sì, sposta il problema dal “bug/patch di versione” a qualcosa di specifico della macchina (autorizzazioni, profilo SMB, o stack RPC/Print) che rimane diverso nonostante l’upgrade.[^5_1][^5_2]

Le modifiche che citi (RpcAuthnLevelPrivacyEnabled=0 e le due GPO su RPC / listener) sono corrette e in linea con quanto Microsoft e vari blog suggeriscono per problemi di stampa di rete su Windows 11, ma agiscono “solo” sul layer RPC della stampa, non sul fatto che il client non riesca nemmeno ad autenticarsi alle share del server.[^5_3][^5_4][^5_5]

***

## Due verifiche chiave: cosa succede a livello SMB e a livello Print

Per capire dove si rompe la catena, io separerei:

1. **SMB puro (prima ancora della stampante)**
    - Dal client problematico, prova a:
        - Collegare una *cartella* condivisa sullo stesso server delle stampanti (o crearne una ad hoc e condividerla).
        - Se non riesci ad aprire nemmeno la share di file, il problema è ancora sul piano SMB/autenticazione, non in RPC Print.[^5_6][^5_1]
    - Se invece la cartella condivisa si apre regolarmente ma la stampante no, allora siamo proprio nel caso “RPC Print only”, e le chiavi che hai messo sono nella direzione giusta.[^5_5][^5_3]
2. **Errore preciso del tentativo di aggiungere la stampante**
    - Ti chiede credenziali e le rifiuta sempre?
    - Oppure ottieni un errore “Impossibile connettersi alla stampante”, “Accesso negato”, “0x00000709” o simili?[^5_7][^5_8][^5_9]

Senza questo dettaglio, SMB vs RPC restano un po’ mescolati; se riesci a dirmi *“una cartella condivisa sullo stesso server si apre/non si apre”* e *“l’errore numerico testuale quando provo ad aggiungere la stampante”* diventa molto più facile stringere il campo.

***

## Lato RPC / stampanti: cosa hai fatto e cosa manca

Hai già messo:

- `HKLM\SYSTEM\CurrentControlSet\Control\Print\RpcAuthnLevelPrivacyEnabled = 0` (DWORD) – questo dice al client di non pretendere il livello privacy-by-default sull’RPC di stampa.[^5_7][^5_3]
- In GPO (Configurazione computer → Modelli amministrativi → Stampanti):
    - “Configura impostazioni listener RPC” = Attivata, “RPC su named pipe e TCP”.[^5_3][^5_5]
    - “Ricerca stampanti” = Attivata.[^5_3]

Mancano però, lato *client*, due pezzi che spesso fanno la differenza su 22H2+ / 25H2:[^5_4][^5_5][^5_7][^5_3]

1. Chiavi in `HKLM\SOFTWARE\Policies\Microsoft\Windows NT\Printers\RPC`:
    - `RpcUseNamedPipeProtocol` = 1 (DWORD, per forzare RPC over Named Pipes).[^5_4][^5_5][^5_3]
    - `RpcProtocols` = 7 (DWORD, 0x7 = binario 111, abilita TCP + Named Pipes + ncalrpc, tipico valore usato nelle guide).[^5_7][^5_3]
    - Facoltativo ma sempre più usato in contesti AD: `ForceKerberosForRpc` = 1 (DWORD) se vuoi escludere NTLM sui canali RPC di stampa.[^5_7]
2. In alcune guide GPO stampanti si suggerisce anche di impostare esplicitamente la policy “Configura impostazioni di connessione RPC” su *“RPC su named pipe”* (o “RPC su named pipe e TCP”) e, congiuntamente, “Configura impostazioni listener RPC” con gli stessi protocolli.[^5_10][^5_5][^5_3]

Queste impostazioni sono complementari a `RpcAuthnLevelPrivacyEnabled` e mirano proprio a riallineare il comportamento di Windows 11 moderno a quello atteso da server/print server più vecchi o configurati per Named Pipes.[^5_5][^5_3]

***

## Lato SMB/autenticazione: cosa rimane da controllare visto che 25H2 è allineato

Dal momento che:

- sei su 25H2 stessa build di altri client funzionanti,
- hai replicato le GPO stampanti,
- eppure *questo* client ancora non si autentica alle share del server,

il focus diventa differenza di stato locale (account, SPN, credenziali memorizzate, nome macchina) e policy di sicurezza di rete.[^5_11][^5_12][^5_13][^5_1]

Piste concrete:

1. **Account macchina / dominio**
    - Se siete in AD: `nltest /sc_verify:DOMINIO` e controllo log di sicurezza su DC mentre il client prova ad accedere (eventi 4776, 4625).
    - Una macchina con trust rotto o SPN duplicati può rifiutare autenticazioni verso risorse di rete in modo “insidioso”, mentre altre macchine nello stesso dominio funzionano.[^5_13][^5_14]
2. **Nome macchina e SID**
    - Se la macchina è stata clonata/immagine vecchia, potresti avere SID macchina o nomi duplicati; 24H2/25H2 hanno hardening più forte su questo.[^5_14]
    - Verifica se questa macchina è “un clone” di una delle altre due che funzionano e se ha passato sysprep; in quel caso potremmo dover rigenerare SID o valutare un re‑join al dominio.
3. **Criteri di sicurezza locali (secpol.msc)** – rispetto a una macchina funzionante:
    - “Sicurezza di rete: livello di autenticazione LAN Manager” (idealmente “Invia soltanto risposta NTLMv2”).[^5_12][^5_13]
    - “Sicurezza di rete: limitare NTLM SSP” e affini (eventuali blocchi su NTLM verso server specifici).[^5_15]
    - “Client di rete Microsoft: firma digitale delle comunicazioni (sempre)” / “(se accettata dal server)” – se qui hai “Sempre” e il server non firma, il comportamento può essere di rifiuto delle credenziali.[^5_12][^5_13]
4. **Profilo di rete e firewall su quel client**
    - Che la rete sia Privata, con “Condivisione file e stampanti” attiva nel profilo corretto.[^5_1]
    - Che eventuali firewall di terze parti o security suite non blocchino il traffico SMB/RPC diversamente dagli altri client.[^5_16][^5_9]

***

## Prossimo passo sensato

Visto che hai già:

- allineato la versione (25H2 + cumulative),
- allineato le GPO stampanti principali,
- e giocato con RpcAuthnLevelPrivacyEnabled,

il passo successivo, pragmaticamente, sarebbe:

1. Confermare se una *cartella condivisa* sullo stesso host delle stampanti è accessibile o no da quel client.[^5_6]
2. Se non lo è, guardare AD / secpol / gpresult e log di sicurezza durante il tentativo (per vedere se è un rifiuto Kerberos/NTLM lato server).[^5_11][^5_13][^5_12]
3. Se la cartella è accessibile ma la stampante continua a non connettersi, allora andare decisi su:
    - chiavi `HKLM\SOFTWARE\Policies\Microsoft\Windows NT\Printers\RPC` (RpcUseNamedPipeProtocol=1, RpcProtocols=7, eventualmente ForceKerberosForRpc=1),
    - doppio check delle due GPO “Configura impostazioni di connessione RPC” e “Configura impostazioni listener RPC”, impostate con coerenza identica tra client funzionanti e non.[^5_4][^5_5][^5_3][^5_7]

Mi puoi dire se da quel client 25H2 riesci almeno ad aprire una cartella condivisa sul server di stampa, e quale messaggio/errore ottieni esattamente quando provi ad aggiungere la stampante condivisa (codice tipo 0x00000709 o altro)?
<span style="display:none">[^5_17][^5_18][^5_19][^5_20]</span>

<div align="center">⁂</div>

[^5_1]: https://learn.microsoft.com/it-it/answers/questions/5620951/windows-11-ver-25h2-sopo-linstallazione-non-vedo-p

[^5_2]: https://pureinfotech.com/windows-11-25h2-vs-24h2/

[^5_3]: https://www.ilsoftware.it/cambia-la-modalita-di-comunicazione-con-le-stampanti-di-rete-in-windows-11_25194/

[^5_4]: https://www.thewindowsclub.com/switch-network-printing-between-tcp-and-rpc

[^5_5]: https://www.ghacks.net/2022/11/02/windows-11-22h2-network-printing-switched-to-rcp-over-tcp/

[^5_6]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^5_7]: https://www.reddit.com/r/HomeNetworking/comments/1rij83t/windows_11_25h2_shared_printer_error_0x0000709/

[^5_8]: https://angolodiwindows.com/2025/11/windows-11-0x00000709-stampante-rete-come-risolvere/

[^5_9]: https://www.deltainfor.it/impossibile-connettersi-stampante-windows/

[^5_10]: https://calcolo.mi.infn.it/?page_id=2174

[^5_11]: https://learn.microsoft.com/it-it/answers/questions/5602061/dopo-istallazione-25h2-la-connessione-via-smb-dell

[^5_12]: https://www.reddit.com/r/truenas/comments/1fh3dqh/latest_windows_11_update_may_break_smb_shares/

[^5_13]: https://learn.microsoft.com/en-us/answers/questions/5575853/problems-with-windows-11-pro-update-to-25h2

[^5_14]: https://www.wincert.net/cast/windows-11-25h2-tightens-enterprise-security-with-new-sid-rules/

[^5_15]: https://www.cyber.gov.au/sites/default/files/2026-01/change_log_-_hardening_microsoft_windows_11_workstation_january_2026.pdf

[^5_16]: https://support.microsoft.com/it-it/windows/risolvere-i-problemi-di-connessione-e-stampa-in-windows-fb830bff-7702-6349-33cd-9443fe987f73

[^5_17]: https://pt.scribd.com/document/670218956/NOVO-ERRO-Impressoras-0x00000709

[^5_18]: https://www.youtube.com/watch?v=7-T6o_7d99Y

[^5_19]: https://phatthanhdat.com/index.php/mang-may-tinh/khac-phuc-loi-chia-se-may-in-tren-windows-11-158.html

[^5_20]: https://www.youtube.com/watch?v=ZQHc96mrYZY


---

```
# <q>Errore preciso del tentativo di aggiungere la stampanteTi chiede credenziali e le rifiuta sempre?</q> confermo questo, sembra considerare non valide le credenziali
```

Se il prompt di credenziali appare, tu inserisci credenziali sicuramente corrette (dominio o locali) e le rifiuta in loop, allora il problema non è più “RPC stampa”, ma l’autenticazione SMB/NTLM/Kerberos tra quel client e il server, anche se gli altri client 25H2 funzionano.[^6_1][^6_2][^6_3][^6_4]

***

## Cosa implica il rifiuto credenziali in questo contesto

In scenari che assomigliano molto al tuo, Microsoft e vari Q\&A riportano che:

- L’aggiornamento a 24H2/25H2 introduce hardening su autenticazione (Kerberos/NTLM) e firma SMB, che può far sì che un client entri in un “limbo” dove chiede credenziali ma le considera sempre non valide.[^6_3][^6_4][^6_5][^6_6]
- Questo si vede spesso con:
    - macchine in dominio con trust danneggiato o out‑of‑sync,
    - differenze di policy di sicurezza (LAN Manager, NTLM restriction) tra client,
    - oppure con NAS/print server che non supportano i requisiti di crittografia/firma richiesti dal client.[^6_2][^6_5][^6_7][^6_1]

Il fatto che *solo* questo client si comporti così, a parità di build, suggerisce un problema specifico di:

- stato macchina nel dominio (account computer, SPN, orologio),
- credenziali memorizzate e/o conflitti di identità,
- policy di sicurezza locali diverse (anche ereditarie da vecchie GPO).[^6_4][^6_8][^6_1][^6_2]

***

## Tre verifiche mirate lato autenticazione

Ti propongo tre controlli/azioni molto “chirurgici”, tipicamente risolutivi in casistiche simili.

### 1. Verifica e, se serve, reset del trust di dominio (se siete in AD)

Nei casi documentati, “chiede user/pass e li rifiuta” con stampanti e share in dominio è spesso dovuto al fatto che il client non autentica più correttamente con il DC (Kerberos rotto o account macchina corrotto).[^6_1][^6_2]

Sul client problematico:

1. Verifica trust:
    - `nltest /sc_verify:TUO_DOMINIO`
    - Se esce errore di secure channel o simili, il trust è rotto.[^6_1]
2. Procedura classica:
    - Rimuovi e riaggiungi il PC al dominio (join a workgroup → reboot → rejoin dominio → reboot).
    - Dopo il rejoin, riprova l’accesso alla share/stampante.

Questo spesso “magicamente” risolve i casi in cui solo una macchina in dominio rifiuta credenziali mentre le altre no.[^6_2][^6_1]

Se NON siete in dominio (solo workgroup), allora questa parte si riduce al controllo che:

- il nome del PC non sia duplicato,
- il SID macchina non sia duplicato rispetto ad altre immagini clonate (in quel caso la soluzione strutturale è ricreare il sistema o usare sysprep).[^6_9]


### 2. Pulizia credenziali e allineamento nome utente/ambito

Su Windows 11 è facile entrare in situazioni in cui:

- ci sono credenziali cacheate con un “ambito” diverso (locale vs dominio),
- oppure stai passando `utente` mentre il server si aspetta `DOMINIO\utente` o `SERVER\utente`.[^6_10][^6_1]

Azioni:

1. Pannello di controllo → Gestione credenziali → credenziali Windows:
    - Rimuovi qualsiasi credenziale verso quel server (nome e IP).[^6_11][^6_12]
2. Quando riprovi la connessione alla share/stampante, inserisci esplicitamente:
    - `DOMINIO\utente` + password (se in AD), oppure
    - `SERVER\utente_locale` + password (se lavorate con account locali sul server).[^6_10][^6_1]

Spesso l’uso di un principal completamente qualificato (`DOMINIO\user` o `server\user`) e la pulizia delle vecchie credenziali risolve il loop di richieste sei credenziali.[^6_10][^6_1]

### 3. Confronto e riallineamento delle policy di sicurezza di rete (NTLM, LM, firma)

Ci sono casi documentati in cui, dopo 24H2/25H2, il client ha:

- “Network security: LAN Manager authentication level” su un valore più restrittivo (es. “Send NTLMv2 response only, refuse LM \& NTLM”),
- policy di limitazione NTLM che vietano NTLM verso determinati server,
- o “Client di rete Microsoft: firma digitale le comunicazioni (sempre)” abilitato.[^6_5][^6_8][^6_4]

Questo, combinato con un server che non firma o usa NTLM “vecchio”, produce il sintomo “credenziali sempre rifiutate”.[^6_4][^6_5]

Operativamente, sul client problematico, rispetto a uno che funziona:

1. Apri `secpol.msc` → Criteri locali → Opzioni di sicurezza e confronta:
    - “Sicurezza di rete: livello di autenticazione LAN Manager” → mettilo come sui client buoni (tipicamente “Invia solo risposta NTLMv2”).[^6_8][^6_1]
    - “Sicurezza di rete: limitare NTLM SSP (NTLMv2 session security)” e similari → verifica che non ci siano restrizioni extra solo su questa macchina.[^6_13]
    - “Client di rete Microsoft: firma digitale le comunicazioni (sempre)” → prova temporaneamente a metterla a Disabilitato, come suggerito anche in varie guide SMB/NAS.[^6_8][^6_4]
2. Da PowerShell admin, puoi anche assicurarti di essere allineato alle macchine che funzionano:
    - `Set-SmbClientConfiguration -RequireSecuritySignature $false`
    - `Set-SmbClientConfiguration -EnableInsecureGuestLogons $true` (solo per test, poi eventualmente lo rimetti com’era).[^6_4]

***

## Perché insisto su AD / autenticazione più che sulle chiavi RPC

La casistica che mostra esattamente il tuo comportamento (prompt credenziali ripetuto su stampante condivisa, con cartelle spesso nello stesso stato) è quasi sempre legata a:

- autenticazione di dominio “sporca” o rotta,
- differenze di policy di sicurezza di rete,
più che alle impostazioni RPC della stampa, che intervengono solo dopo che SMB ha già autenticato con successo.[^6_14][^6_15][^6_2][^6_1]

Le chiavi `RpcAuthnLevelPrivacyEnabled` e le GPO “Configura impostazioni di connessione RPC” / “listener RPC” sono *necessarie* per problemi tipo 0x00000709, accesso negato, ecc., ma **non risolvono** un rifiuto sistematico delle credenziali da parte di SMB.[^6_16][^6_17][^6_18][^6_19]

***

## Passo pratico successivo

Visto dove siamo arrivati, il prossimo step che ti darei è:

1. Se siete in dominio AD:
    - esegui `nltest /sc_verify:DOMINIO` sul client rotto e confronta con un client che funziona,
    - se il secure channel non è “OK”, fai leave/rejoin del dominio.[^6_1]
2. In ogni caso:
    - pulisci tutte le credenziali verso il server,
    - riprova con `DOMINIO\utente` o `SERVER\utente_locale`,
    - confronta e allinea LAN Manager auth level e firma SMB rispetto a un client buono.[^6_5][^6_8][^6_4][^6_1]

Mi puoi dire se siete in dominio AD o in semplice workgroup, e se da questo client riesci ad accedere almeno a una cartella condivisa su *qualche altro* server Windows della rete usando le stesse credenziali?
<span style="display:none">[^6_20][^6_21][^6_22]</span>

<div align="center">⁂</div>

[^6_1]: https://learn.microsoft.com/en-ca/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^6_2]: https://www.ilsoftware.it/focus/dopo-laggiornamento-a-windows-11-cartelle-condivise-e-stampanti-non-si-vedono-come-risolvere-subito/

[^6_3]: https://learn.microsoft.com/it-it/answers/questions/5602061/dopo-istallazione-25h2-la-connessione-via-smb-dell

[^6_4]: https://angolodiwindows.com/2024/12/come-risolvere-problemi-smb-windows-11-24h2-nas

[^6_5]: https://www.trickit.it/windows/windows-11-24h2-errore-crittografia-accesso-share

[^6_6]: https://learn.microsoft.com/it-it/answers/questions/5590843/accesso-a-risorse-condivise-da-windows-11-pro-25h2

[^6_7]: https://manuals.konicaminolta.eu/bizhub-C3321i/IT/contents/TS_07_03.html

[^6_8]: https://www.reddit.com/r/truenas/comments/1fh3dqh/latest_windows_11_update_may_break_smb_shares/

[^6_9]: https://www.wincert.net/cast/windows-11-25h2-tightens-enterprise-security-with-new-sid-rules/

[^6_10]: https://learn.microsoft.com/it-it/answers/questions/4186796/stampante-condivisa-da-oggi-chiede-una-password

[^6_11]: https://www.reddit.com/r/unRAID/comments/1o4o3g5/winows_11_24h2_has_issues_connecting_to_unraid/

[^6_12]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^6_13]: https://www.cyber.gov.au/sites/default/files/2026-01/change_log_-_hardening_microsoft_windows_11_workstation_january_2026.pdf

[^6_14]: https://learn.microsoft.com/it-it/answers/questions/4118366/problema-condivisione-stampante-in-windows-11

[^6_15]: https://learn.microsoft.com/it-it/answers/questions/4265591/lenigma-delle-stampanti-condivise

[^6_16]: https://learn.microsoft.com/it-it/answers/questions/4118366/problema-condivisione-stampante-in-windows-11?page=2

[^6_17]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/cannot-install-shared-network-printer-locally

[^6_18]: https://www.ilsoftware.it/cambia-la-modalita-di-comunicazione-con-le-stampanti-di-rete-in-windows-11_25194/

[^6_19]: https://www.ghacks.net/2022/11/02/windows-11-22h2-network-printing-switched-to-rcp-over-tcp/

[^6_20]: https://learn.microsoft.com/it-it/answers/questions/5620951/windows-11-ver-25h2-sopo-linstallazione-non-vedo-p

[^6_21]: https://support.microsoft.com/it-it/windows/risolvere-i-problemi-di-connessione-della-stampante-condivisa-in-windows-27f7b136-02e6-406a-9a25-fc87838062b8

[^6_22]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/best-practices-analyzer/smb-open-file-sharing-ports


---

# <q>Verifica trust:nltest /sc_verify:TUO_DOMINIO</q> H:\>nltest /sc_verify:GASLINILAN

Errore di I_NetLogonControl: Status = 5 0x5 ERROR_ACCESS_DENIED

L’output `Status = 5 0x5 ERROR_ACCESS_DENIED` su `nltest /sc_verify` indica che il **secure channel di dominio di quella macchina è rotto**: il computer account nel dominio non è più in uno stato coerente, ed è esattamente una delle condizioni classiche che porta a rifiuto sistematico delle credenziali verso risorse condivise (share e stampanti) pur essendo le credenziali corrette.[^7_1][^7_2][^7_3]

***

## Cosa significa esattamente quell’errore

Per `nltest /sc_verify` i casi “buoni” devono restituire `Status = 0x0 NERR_Success`.[^7_2][^7_4]
Se invece ottieni `ERROR_ACCESS_DENIED` o `ERROR_INVALID_PASSWORD`, la documentazione e varie analisi indicano che:

- il secure channel tra il computer e il dominio è corrotto o non è più sincronizzato,
- in pratica, il **computer account nel dominio** e la macchina locale non concordano sulla password del canale sicuro.[^7_3][^7_5][^7_1]

In questa situazione:

- Kerberos/NTLM per quell’account macchina falliscono,
- le connessioni SMB verso server di dominio possono chiedere credenziali ma rifiutarle sistematicamente, anche se l’utente e la password sono corretti (perché lo stato del computer account è considerato non valido).[^7_6][^7_1][^7_3]

***

## Come ripristinare il secure channel del client

Qui siamo fuori dalla sfera “stampante/SMB” e dentro il mondo AD puro. Le soluzioni riconosciute sono due: **rejoin al dominio** o **reset della password dell’account macchina**.[^7_7][^7_8][^7_5][^7_1]

### Opzione 1: Disjoin/rejoin del dominio (metodo “pulito”)

1. Accedi al client con un account **locale** amministrativo.
2. Impostazioni di sistema → Cambia appartenenza al dominio:
    - Porta la macchina in un **workgroup** (es. `WORKGROUP`), conferma e riavvia.[^7_8]
3. Una volta tornato su, rientra con account locale e:
    - re‑join al dominio `GASLINILAN` con credenziali di un Domain Admin,
    - riavvia di nuovo.[^7_7][^7_8]
4. Dopo il rejoin, da prompt admin locale, verifica:
    - `nltest /sc_verify:GASLINILAN`
    - Se ora hai `NERR_Success`, il secure channel è ripristinato.[^7_4][^7_2]

Questa è la procedura che in pratica si usa ovunque quando si vede il messaggio “il rapporto di trust tra questa workstation e il dominio primario è fallito”, e gli articoli che analizzano `sc_verify` con `ERROR_ACCESS_DENIED` dicono esplicitamente che **la cura è la ri‑partecipazione al dominio**.[^7_9][^7_1][^7_8]

### Opzione 2: Reset della password dell’account computer (metodo “soft”)

Se preferisci evitare il disjoin, puoi usare `netdom` o `nltest` per resettare la password del computer account direttamente dal client o da un server, con credenziali di Domain Admin.[^7_5][^7_8][^7_7]

Esempi (da prompt elevato, loggato come amministratore di dominio o usando `/userd`):

- Con `netdom` (se disponibile):[^7_8][^7_7]

```cmd
netdom resetpwd /server:NOME_DC /userd:DOMINIO\Administrator /passwordd:*
```

- Oppure sequenza `nltest /sc_reset` come da doc MS/spiceworks:[^7_5][^7_7]

Dopo il reset:

- riavvii la macchina,
- rifai `nltest /sc_verify:GASLINILAN`,
- e se passa a `NERR_Success` hai rimesso in sesto il canale sicuro.[^7_4][^7_9]

***

## Perché questo dovrebbe sbloccare anche le stampanti

Finché il secure channel è rotto:

- la macchina è “sospetta” per il dominio,
- Kerberos e NTLM legati all’account macchina falliscono e alcune risorse di dominio (share, stampanti condivise) possono comportarsi come se **nessuna credenziale fosse valida**, proprio il sintomo che stai osservando.[^7_1][^7_3][^7_6]

Una volta ripristinato il trust:

- il client torna un “cittadino AD normale”,
- le autenticazioni verso server di dominio (compreso il server di stampa) dovrebbero tornare a funzionare con le stesse credenziali che già usi sugli altri client.

A quel punto:

- se l’accesso alle share e alle stampanti si sblocca, sappiamo che **il problema non era più SMB/RPC ma AD trust**;
- se invece rimane qualche errore residuo solo per la stampa (ma le share funzionano), ci possiamo concentrare serenamente solo sulle chiavi RPC/GPO stampanti, sapendo che il sottostante AD/SMB è sano.

***

In questa fase ti consiglierei di partire proprio dal ripristino del secure channel (disjoin/rejoin o reset password computer account), poi evitare qualunque ulteriore tweak di registry finché non vedi se `nltest /sc_verify:GASLINILAN` torna a `NERR_Success`: se dopo quello le credenziali iniziano ad essere accettate, abbiamo trovato il vero root cause.
<span style="display:none">[^7_10][^7_11][^7_12][^7_13][^7_14][^7_15]</span>

<div align="center">⁂</div>

[^7_1]: https://spiral.hatenadiary.org/entry/20121217/1355720545

[^7_2]: https://ticonsigliotech.com/2025/11/10/utente-user-a-dominio-non-accetta-credenziali-dei-domain-administrators/

[^7_3]: https://stackoverflow.com/questions/12673506/verify-active-directory-domain-trust-relationship-of-the-local-machine

[^7_4]: https://www.reddit.com/r/PowerShell/comments/1eb494u/best_way_to_confirm_a_domain_computer_has_line_of/

[^7_5]: https://groups.google.com/g/microsoft.public.tr.windows.server/c/0XhSpO-_L7Y

[^7_6]: https://support.microsoft.com/it-it/topic/errori-di-autenticazione-kerberos-e-ntlm-dovuti-a-sid-duplicati-76f7394d-c460-4882-9ed1-d27e0960f949

[^7_7]: https://www.reddit.com/r/sysadmin/comments/1mcvbnq/windows_2008_r2_server_not_able_to_authenticate/

[^7_8]: https://community.spiceworks.com/t/the-trust-relationship-between-this-workstation-and-the-primary-domain-failed/335803

[^7_9]: https://thdy.hatenablog.jp/entry/2017/11/25/012756

[^7_10]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/active-directory/cant-sign-the-domain

[^7_11]: https://learn.microsoft.com/de-de/windows/release-health/resolved-issues-windows-11-25h2

[^7_12]: https://learn.microsoft.com/es-es/windows/release-health/resolved-issues-windows-11-25h2

[^7_13]: https://www.dell.com/support/kbdoc/en-my/000226052/domain

[^7_14]: https://learn.microsoft.com/pt-br/windows/release-health/resolved-issues-windows-11-25h2

[^7_15]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/windows-security/troubleshoot-kerberos-domain-not-found-event-id-5719


---

# no, direi che come administrator locale il risultato e' diverso

C:\Users\Administrator>nltest /sc_verify:GASLINILAN
Contrassegni: 400000b0 HAS_IP  HAS_TIMESERV  Servizio di autenticazione: Netlogon
Nome del controller di dominio trusted \\srvad01.gaslini.lan
Stato del controller di dominio trusted Status = 0 0x0 NERR_Success
Verifica attendibilità Status = 0 0x0 NERR_Success
Esecuzione comando riuscita

Il fatto che come Administrator locale tu ottenga `NERR_Success` sul `sc_verify` è un’ottima notizia: il secure channel macchina‑dominio è integro e il problema non è un trust rotto, ma piuttosto **come vengono usate le credenziali utente** (ambito/local vs dominio) e/o differenze di policy di sicurezza per gli utenti normali rispetto all’Administrator locale.[^8_1][^8_2]

***

## Cosa implica la differenza di risultato

Riassumendo:

- Come Administrator locale: `nltest /sc_verify:GASLINILAN` → OK.[^8_1]
- Con l’utente “normale” con cui vedi il problema di stampa:
    - ti chiede credenziali per la stampante condivisa,
    - rifiuta sistematicamente anche credenziali corrette (dominio/locali).

Questo quadro è coerente con scenari in cui:

- il **secure channel a livello di computer** è sano,
- ma per uno specifico utente o per gli utenti non admin ci sono:
    - credenziali cacheate/sbagliate verso quel server,
    - o policy NTLM/LM/firma SMB più restrittive,
    - o addirittura qualche blocco AD (account, gruppo, diritti di accesso a quella share/stampante).[^8_3][^8_4][^8_5][^8_6]

***

## Passi mirati lato utente/domino

Ti propongo una verifica in due fasi, sfruttando il fatto che come Administrator locale “vedi” un dominio sano.

### 1. Dal client problematico, usando *un utente di dominio che funziona sugli altri PC*

1. Pulisci credenziali Windows per quel server (dal profilo utente con cui fai la prova):
    - Pannello di controllo → Gestione credenziali → Credenziali Windows → elimina tutto ciò che punta a `\\srvstampa` o all’IP del server di stampa.[^8_7][^8_8]
2. Apri `\\SERVER_DI_STAMPA\` da Esplora risorse, ma **eseguito come un utente che sai funzionare su un altro client** (es. `DOMINIO\utenteX` che sugli altri PC accede alle stesse stampanti).
    - Se puoi, fai un test esplicito: loggati completamente con quel `DOMINIO\utenteX` sul client problematico (non solo runas) e prova a:
        - accedere a una cartella condivisa sul server di stampa,
        - poi aggiungere la stampante.[^8_4][^8_9][^8_3]
3. Inserisci le credenziali in forma piena:
    - `DOMINIO\utenteX` + password (non solo “utenteX”).[^8_10][^8_3]

Se con lo stesso utente di dominio che funziona altrove, sul client problematico **le credenziali continuano ad essere rifiutate**, allora:

- non è un problema specifico dell’account utente,
- ma un blocco locale (policy, credenziali corrotte, firewall, endpoint security) su quel client.[^8_5][^8_3][^8_4]

Se invece con `DOMINIO\utenteX` funziona, ma con l’utente di cui parlavi prima no, allora il problema è davvero legato a quell’account o al suo token di sicurezza.

### 2. Confronto policy e diritti tra client buono e client problematico

Visto che entrambi sono 25H2 stessa build, la prossima mossa “da sistemista” per un caso così ostinato è:

1. Su un **client che funziona**, esegui come Administrator locale:
    - `gpresult /H C:\temp\gp_ok.html`
2. Sul **client problematico**, come Administrator locale:
    - `gpresult /H C:\temp\gp_bad.html`
3. Apri i due report e confronta, in particolare:
    - Configurazione computer → Criteri di sicurezza:
        - “Sicurezza di rete: livello di autenticazione LAN Manager”,
        - criteri che limitano NTLM (Network security: Restrict NTLM),
        - “Client di rete Microsoft: firma digitale comunicazioni (sempre/se accettata)”.[^8_11][^8_12][^8_5]
    - Configurazione utente, se stai applicando GPO per utenti che possano influire su accesso rete.

Spesso in casistiche 25H2 + SMB/print, una differenza banale (LAN Manager level, NTLM restrictions, firma obbligatoria per il client) è sufficiente per spiegare perché un client rifiuta credenziali e un altro no.[^8_12][^8_5][^8_11]

***

## Direzione concreta da qui

Dato che:

- il secure channel è sano con Administrator locale,
- il problema è “solo” l’utente che si vede rifiutare le credenziali verso le share/stampanti,

io farei due esperimenti in quest’ordine:

1. Test con un **utente di dominio che sugli altri PC funziona** (dopo aver pulito credenziali) per vedere se il rifiuto è generalizzato a tutti gli utenti su quel client o solo a uno.[^8_3][^8_10]
2. In parallelo, generare i due `gpresult` (client buono vs client problematico) e dare un’occhiata specificamente alle opzioni di sicurezza di rete legate a NTLM/firma SMB: la minima divergenza qui, su 24H2/25H2, è spesso determinante.[^8_5][^8_11][^8_12]

Se ti va, puoi dirmi:

- se un utente di dominio “di controllo” viene rifiutato anche su quel client,
oppure
- se per quello invece funziona, così capiamo se concentrare il debugging sull’account utente o sulla macchina.

<div align="center">⁂</div>

[^8_1]: https://ticonsigliotech.com/2025/11/10/utente-user-a-dominio-non-accetta-credenziali-dei-domain-administrators/

[^8_2]: https://www.reddit.com/r/PowerShell/comments/1eb494u/best_way_to_confirm_a_domain_computer_has_line_of/

[^8_3]: https://learn.microsoft.com/en-ca/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^8_4]: https://www.ilsoftware.it/focus/dopo-laggiornamento-a-windows-11-cartelle-condivise-e-stampanti-non-si-vedono-come-risolvere-subito/

[^8_5]: https://angolodiwindows.com/2024/12/come-risolvere-problemi-smb-windows-11-24h2-nas

[^8_6]: https://support.microsoft.com/it-it/topic/errori-di-autenticazione-kerberos-e-ntlm-dovuti-a-sid-duplicati-76f7394d-c460-4882-9ed1-d27e0960f949

[^8_7]: https://www.reddit.com/r/unRAID/comments/1o4o3g5/winows_11_24h2_has_issues_connecting_to_unraid/

[^8_8]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^8_9]: https://learn.microsoft.com/it-it/answers/questions/4118366/problema-condivisione-stampante-in-windows-11

[^8_10]: https://learn.microsoft.com/it-it/answers/questions/4186796/stampante-condivisa-da-oggi-chiede-una-password

[^8_11]: https://www.trickit.it/windows/windows-11-24h2-errore-crittografia-accesso-share

[^8_12]: https://www.reddit.com/r/truenas/comments/1fh3dqh/latest_windows_11_update_may_break_smb_shares/


---

# non ci sono utenti di dominio funzionanti dai pc che non accedono alle shares, sui pc che non hanno problemi qualsiasi utente di dominio riesce ad avere accesso

Il fatto che su quel client **nessun utente di dominio** riesca ad autenticarsi alle share/stampanti, mentre sugli altri 25H2 funziona chiunque, e che il `sc_verify` passi solo come Administrator locale, indica un problema **di macchina** (config SMB/NTLM/policy locali) e non di singoli account utente.[^9_1][^9_2][^9_3]

***

## Cosa sappiamo adesso

- Dominio: il secure channel del computer verso il dominio è sano (come Administrator locale vedi `NERR_Success`).[^9_4][^9_5]
- Versione: il client problematico è 25H2, stessa build dei client che funzionano.
- Sintomo: da quel client, qualunque user di dominio inserito nel prompt delle credenziali viene rifiutato; sugli altri client, gli stessi utenti funzionano.
- Interventi già tentati: chiavi RPC/Print, alcune impostazioni SMB guest, upgrade di versione, senza effetto.

Il quadro è coerente con:

- una **configurazione SMB/NTLM più restrittiva su quel client** (policy o registro), oppure
- un **blocco NTLM su SMB** impostato solo lì (GPO “Block NTLM” / `Set-SmbClientConfiguration -BlockNTLM`),
che fa sì che le richieste di autenticazione verso quel server non vengano nemmeno processate correttamente, producendo rifiuto credenziali per tutti gli utenti.[^9_2][^9_6][^9_3][^9_1]

***

## Due direzioni pratiche per sbloccare

A questo punto, avendo escluso un problema di versione e di trust globale, ti propongo due azioni pragmatiche:

### 1. Verifica/annulla blocco NTLM e hardening SMB su quel client

Sui client 24H2/25H2 è possibile (via GPO o PowerShell) **bloccare NTLM sulle connessioni SMB**, il che produce esattamente situazioni di “username o password errati” per qualsiasi utente verso certi server, a differenza di altri client dove la policy non è attiva.[^9_6][^9_3][^9_2]

Sul client problematico, da PowerShell elevato:

1. Controlla se è attivo il blocco NTLM SMB:
```powershell
Get-SmbClientConfiguration | Select-Object BlockNTLM, RequireSecuritySignature
```

- Se `BlockNTLM` risulta `True`, è probabile che tu abbia centrato il problema.[^9_3][^9_2][^9_6]

2. Per test, disabilita il blocco NTLM e la firma obbligatoria:
```powershell
Set-SmbClientConfiguration -BlockNTLM $false -RequireSecuritySignature $false -Force
```

3. Verifica in `gpedit.msc`:

- Configurazione computer → Modelli amministrativi → Rete → **Workstation Lanman**:
    - “Block NTLM (LM, NTLM, NTLMv2)” deve essere **Disabilitato**.[^9_7][^9_6][^9_2]
    - “Enable insecure guest logons” per test può essere abilitato, ma il punto critico qui è Block NTLM.[^9_8][^9_9]

4. Riavvia il client, riprova la connessione alla share/stampante con un utente di dominio.

Se dopo questa modifica le credenziali vengono finalmente accettate, hai trovato il “colpevole”: quel client stava **bloccando NTLM in SMB** (o richiedendo una combinazione di firma/crypto che il server non forniva), a differenza degli altri.[^9_1][^9_6][^9_3][^9_2]

### 2. Allineare completamente le policy di sicurezza al modello di un client “buono”

Dato che hai almeno un client 25H2 identico che funziona, il metodo più robusto è:

1. Su un client funzionante:
    - `gpresult /H C:\temp\gp_ok.html`
2. Sul client problematico:
    - `gpresult /H C:\temp\gp_bad.html`
3. Confrontarli (anche a colpo d’occhio) guardando in particolare:
    - Configurazione computer → Impostazioni di Windows → Impostazioni di sicurezza → Criteri locali → Opzioni di sicurezza:
        - “Network security: LAN Manager authentication level” (deve essere lo stesso del client buono, tipicamente **Send NTLMv2 response only**).[^9_10][^9_1]
        - “Network security: Restrict NTLM: …” e voci correlate: verifica che non ci siano restrizioni NTLM più aggressive sul client problematico.[^9_3][^9_2]
        - “Microsoft network client: Digitally sign communications (always)” – se su uno è Abilitato e sull’altro No, può spiegare differenze.[^9_10][^9_3]
    - Modelli amministrativi → Rete → Workstation Lanman:
        - “Block NTLM (LM, NTLM, NTLMv2)”,
        - “Enable insecure guest logons”,
        - eventuali altri settaggi SMB client.

Se trovi qualche voce configurata diversamente rispetto al client buono, riallineala e riprova la connessione; vista la casistica su 24H2/25H2, differenze in queste policy sono *esattamente* il tipo di cosa che genera quello che vedi.[^9_11][^9_1][^9_2][^9_3]

***

## Come procederei “da admin” adesso

Riassumendo il workplan minimo:

1. Su client problematico:
    - `Get-SmbClientConfiguration` → vedi se `BlockNTLM` è `True` o altre opzioni anomale rispetto a un client buono.[^9_6][^9_2]
    - Se sì, `Set-SmbClientConfiguration -BlockNTLM $false -RequireSecuritySignature $false -Force`, riavvia, ritesta.[^9_2][^9_3]
2. In parallelo, generare i due `gpresult` e confrontare **solo** le sezioni SMB/NTLM/LM di cui sopra; ogni differenza comportamentale, su una build e un dominio omogenei, deve venire da lì (o da un agent di sicurezza di terze parti, se presente).[^9_11][^9_1][^9_2]

Se ti va, il prossimo passo potrebbe essere: esegui `Get-SmbClientConfiguration` sul client problematico e su uno funzionante e incolla (o riassumi) le differenze sui campi `BlockNTLM`, `RequireSecuritySignature`, `EnableInsecureGuestLogons` e affini; da lì possiamo puntare il dito quasi chirurgicamente.
<span style="display:none">[^9_12][^9_13][^9_14][^9_15][^9_16][^9_17]</span>

<div align="center">⁂</div>

[^9_1]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^9_2]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^9_3]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening

[^9_4]: https://ticonsigliotech.com/2025/11/10/utente-user-a-dominio-non-accetta-credenziali-dei-domain-administrators/

[^9_5]: https://www.reddit.com/r/PowerShell/comments/1eb494u/best_way_to_confirm_a_domain_computer_has_line_of/

[^9_6]: https://learn.microsoft.com/is-is/windows-server/storage/file-server/smb-ntlm-blocking

[^9_7]: https://www.reddit.com/r/sysadmin/comments/1pzolhu/ntlm_authentication_issues_on_25h2/

[^9_8]: https://www.dell.com/support/kbdoc/it-it/000104551/powerscale-onefs-gli-utenti-smb-sembrano-disporre-di-autorizzazioni-complete-ma-ricevono-un-errore-di-accesso-negato

[^9_9]: https://www.asus.com/it/support/faq/1054736/

[^9_10]: https://turbolab.it/windows-10/come-ripristinare-condivisione-cartelle-windows-11-4459

[^9_11]: https://wktaaeu.my.site.com/itcommunity/s/article/Nota-sistemistica-B-Point-Problemi-di-accesso-dopo-l-installazione-di-Windows-11

[^9_12]: https://manuals.konicaminolta.eu/bizhub-367-287-227/IT/contents/id08-_102791751.html

[^9_13]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/target-account-name-incorrect-error-access-file-share

[^9_14]: https://learn.microsoft.com/it-it/answers/questions/5590843/accesso-a-risorse-condivise-da-windows-11-pro-25h2

[^9_15]: https://www.dell.com/support/kbdoc/it-it/000059264/powerscale-l-autenticazione-smb-tramite-ldap-richiede-ntlm-in-onefs-6-5-e-versioni-successive

[^9_16]: https://www.reddit.com/r/sysadmin/comments/1hnas4d/windows_11_24h2_update_cannot_access_network/

[^9_17]: https://forum.tomshw.it/threads/windows-11-cartelle-condivise-username-o-password-errato.1225219/


---

```
# <q>Se BlockNTLM risulta True, è probabile che tu abbia centrato il problema</q> no, e' false
```

Se `BlockNTLM` è già `False`, allora non è quello a bloccare l’autenticazione; resta però il fatto che **solo quella macchina rifiuta tutte le credenziali di dominio**, mentre il secure channel è sano e gli altri client 25H2 funzionano, quindi per esclusione il problema è in qualche combinazione di **policy di sicurezza (NTLM / LAN Manager / firma) o GPO locali** diversa da quella dei client sani.[^10_1][^10_2][^10_3]

***

## Dove guardare ora in modo mirato

Dato il quadro attuale, ci sono tre insiemi di impostazioni che statisticamente spiegano proprio quel comportamento:

1. **LAN Manager auth level / NTLM restrictions**
    - In `secpol.msc` sul client rotto, confronta con un client buono:
        - Criteri locali → Opzioni di sicurezza →
            - “Sicurezza di rete: livello di autenticazione LAN Manager”
            - Tutte le voci “Sicurezza di rete: limitare NTLM…” (Restrict NTLM).[^10_2][^10_3][^10_1]
    - Anche una singola voce “Restrict NTLM: Outgoing NTLM traffic” impostata in modo più duro (es. “Deny all”) può portare a rifiuto credenziali verso alcuni server pur con `BlockNTLM=False`.[^10_3][^10_4][^10_2]
2. **Firma SMB lato client**
    - “Client di rete Microsoft: firma digitale le comunicazioni (sempre)”
    - “Client di rete Microsoft: firma digitale le comunicazioni (se accettata)”
Se su quel client è forzato “sempre” mentre sul server o sugli altri client no, puoi avere sintomi di autenticazione fallita.[^10_5][^10_6][^10_7]
3. **GPO extra sul client rotto**
    - In `gpresult /H` confronta le sezioni:
        - Configurazione computer → Impostazioni di Windows → Impostazioni di sicurezza → Criteri locali → Opzioni di sicurezza
        - Modelli amministrativi → Rete → Workstation Lanman
rispetto a un client funzionante, e cerca qualunque policy “Configurata” solo sul client rotto.[^10_8][^10_1][^10_2]

***

## Prossimo passo pragmatico

Visto che:

- `BlockNTLM=False`,
- il secure channel è ok,
- e il problema è riproducibile per *tutti* gli utenti di dominio solo su quella macchina,

l’unico modo affidabile per capire cosa la distingue è:

1. Eseguire `gpresult /H` su client buono e client rotto.[^10_1][^10_8]
2. Confrontare (anche ad occhio, limitandoti alle sezioni SMB/NTLM/LM/firma) e individuare la o le policy che divergono.
3. Allineare quelle del client rotto a quelle del client sano e ritestare.

Se ti è comodo, puoi iniziare proprio da:

- “Sicurezza di rete: livello di autenticazione LAN Manager”,
- tutte le “Sicurezza di rete: limitare NTLM…”,
- “Client di rete Microsoft: firma digitale…”,

e dirmi se vedi una di queste impostata diversamente tra client buono e client rotto; tipicamente ce n’è una o due che spiegano il rifiuto credenziali in blocco.

<div align="center">⁂</div>

[^10_1]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^10_2]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^10_3]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening

[^10_4]: https://learn.microsoft.com/is-is/windows-server/storage/file-server/smb-ntlm-blocking

[^10_5]: https://turbolab.it/windows-10/come-ripristinare-condivisione-cartelle-windows-11-4459

[^10_6]: https://www.trickit.it/windows/windows-11-24h2-errore-crittografia-accesso-share

[^10_7]: https://www.reddit.com/r/truenas/comments/1fh3dqh/latest_windows_11_update_may_break_smb_shares/

[^10_8]: https://wktaaeu.my.site.com/itcommunity/s/article/Nota-sistemistica-B-Point-Problemi-di-accesso-dopo-l-installazione-di-Windows-11


---

# aspetta, stiamo facendo i test sul server che a qualche macchina concede l'accesso e ad altre no.

Vuoi tornare a fare test sui client che non accedono? Sono 3 e non erano stati toccati mentre e' il server la macchina reinstallata

Grazie per il chiarimento: sì, ha senso tornare a ragionare con la prospettiva giusta, cioè **server reinstallato che espone le share** e **tre client che non accedono**, mentre altri client accedono normalmente allo stesso server.[^11_1][^11_2]

Per rimetterci allineati:

- Server: reinstallato (ora 25H2), condivisione stampanti e cartelle ok per alcuni client.
- Client “buoni”: accedono alle share del nuovo server senza problemi (stessa versione / dominio).
- Client “problematici” (3): non accedono alle share dello stesso server, rifiuto credenziali per qualunque utente di dominio.

Quindi il pattern è:

- lato server: configurazione sufficientemente corretta (altrimenti i client buoni non funzionerebbero);
- lato client: esiste un sottogruppo di macchine che, per qualche motivo (policy, stato di dominio, cache credenziali, configurazione rete/SMB), non riesce ad autenticarsi verso quel server reinstallato.[^11_3][^11_4][^11_5]

***

## Come procedere in modo sistematico sui 3 client “rotti”

Ti propongo una checklist mirata, da applicare su UNO dei client problematici (poi replicabile sugli altri), confrontandolo sempre con un client che funziona.

### 1. Verifica base AD / rete dal client problematico

Sul client problematico, con un account admin locale:

- `nltest /sc_verify:DOMINIO` → assicuriamoci che anche qui il secure channel verso il dominio sia `NERR_Success`.[^11_6][^11_7]
- Verifica che il client riesca ad accedere a **qualche altra share in dominio** (es. share di un file server diverso dal server di stampa). Se accede altrove ma non al server reinstallato, restringiamo il problema al binomio client↔quel server; se non accede a nulla, il problema è più generale.[^11_8][^11_3]


### 2. Pulizia credenziali e test con utente di dominio noto funzionante

Sempre su un client problematico:

- Pulisci tutte le credenziali verso il server reinstallato in Gestione credenziali → Credenziali Windows.[^11_4]
- Effettua il test con un **utente di dominio che sai funzionare dai client buoni** (stesse credenziali, stessa share).[^11_9]
- Usa sempre formato `DOMINIO\utente` nel prompt.[^11_10]

Se su quel client le credenziali vengono rifiutate ma sugli altri no, siamo di nuovo davanti a un blocco lato client (policy/NTLM/firma) specifico per quel server.[^11_11][^11_12][^11_9]

### 3. Confronto configurazione SMB/NTLM tra un client buono e un client problematico

Su un client buono e su uno problematico:

- Esegui `Get-SmbClientConfiguration` e confronta almeno:
    - `RequireSecuritySignature`
    - `EnableInsecureGuestLogons`
    - `EnableSMB1Protocol` (non dovrebbe servire, ma utile a sapere)
    - eventuali flag meno ovvi (es. `EnableSecuritySignature`).[^11_13][^11_14][^11_8]
- Esegui `gpresult /H` su entrambi e confronta le sezioni:
    - Criteri locali → Opzioni di sicurezza, in particolare:
        - LAN Manager auth level
        - Restrict NTLM
        - Microsoft network client/server: digital sign communications.[^11_12][^11_9][^11_11]
- Verifica che **impostazioni di condivisione avanzate** (individuazione rete, condivisione file/stampanti, condivisione protetta da password) siano allineate tra client buoni e problematici.[^11_14][^11_3]

Questo confronto ti dirà cosa è diverso lato client rispetto a quelli che accedono correttamente al server reinstallato.

***

## Perché ha senso fare adesso il focus sui client

Visto che:

- il server reinstallato dimostra di funzionare con alcuni client,
- e c’è un gruppo coeso di 3 client che falliscono allo stesso modo,

la via più efficiente è trattare **i 3 client come “varianti patologiche”** in un ambiente per il resto sano, e:

- verificare AD / secure channel su ciascuno,
- confrontare SMB/NTLM/lanman/firma con un client buono,
- e solo in seconda battuta ritornare al server se qualcosa suggerisce configurazioni “per‑client” (es. ACL/permessi share filtrati per IP o computer account).[^11_2][^11_15][^11_8]

Se vuoi, possiamo prendere un client problematico alla volta: mi dici per uno di questi:

- versione/build,
- esito di `nltest /sc_verify:DOMINIO`,
- se accede ad altre share di dominio diverse dal server di stampa,

e poi affiniamo a partire da lì.
<span style="display:none">[^11_16][^11_17][^11_18][^11_19][^11_20]</span>

<div align="center">⁂</div>

[^11_1]: https://support.microsoft.com/it-it/windows/risolvere-i-problemi-di-connessione-e-stampa-in-windows-fb830bff-7702-6349-33cd-9443fe987f73

[^11_2]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/troubleshoot-printing-known-issues

[^11_3]: https://learn.microsoft.com/it-it/answers/questions/4028102/problema-rete-interna-e-condivisione-win-10-e-win

[^11_4]: https://www.reddit.com/r/synology/comments/15u1frh/smb_windows_11_auth_failing/

[^11_5]: https://learn.microsoft.com/it-it/answers/tags/318/windows-business-windows-client-it-pros-networking-network-connectivity-file-sharing

[^11_6]: https://ticonsigliotech.com/2025/11/10/utente-user-a-dominio-non-accetta-credenziali-dei-domain-administrators/

[^11_7]: https://www.reddit.com/r/PowerShell/comments/1eb494u/best_way_to_confirm_a_domain_computer_has_line_of/

[^11_8]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance

[^11_9]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^11_10]: https://rigacci.org/wiki/doku.php/doc/appunti/linux/tux/samba

[^11_11]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^11_12]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening

[^11_13]: https://support.keenetic.com/peak/kn-2710/it/43345-how-to-enable-access-to-a-network-drive-from-windows-11.html

[^11_14]: https://turbolab.it/windows-10/come-ripristinare-condivisione-cartelle-windows-11-4459

[^11_15]: https://learn.microsoft.com/it-it/troubleshoot/windows-client/networking/access-denied-access-smb-file-share

[^11_16]: https://manuals.konicaminolta.eu/bizhub-C3321i/IT/contents/TS_07_03.html

[^11_17]: https://www.youtube.com/watch?v=iQ2clO_lZ1o

[^11_18]: https://www.reddit.com/r/unRAID/comments/1hhl28h/windows_11_24h2_and_cannot_login_smb_without/

[^11_19]: https://forum.ubuntu-it.org/viewtopic.php?t=649650

[^11_20]: https://www.net-usb.com/it/share-usb/how-to-share-printer/


---

# LOGIN A WINDOWS COME ADMINISTRATOR, quindi uso lo stesso utente:

apro CMD normale, nltest fallisce con errore 5
apro CMD elevato e riesce:

C:\Windows\System32>nltest /sc_verify:GASLINILAN
Contrassegni: 400000b0 HAS_IP  HAS_TIMESERV  Servizio di autenticazione: Netlogon
Nome del controller di dominio trusted \\srvad02.gaslini.lan
Stato del controller di dominio trusted Status = 0 0x0 NERR_Success
Verifica attendibilità Status = 0 0x0 NERR_Success
Esecuzione comando riuscita

C:\Windows\System32>

Il comportamento che descrivi è in realtà normale: `nltest /sc_verify` richiede privilegi elevati, quindi in un prompt non elevato restituisce `ERROR_ACCESS_DENIED (5)`, mentre nello stesso login, ma in CMD elevato, il secure channel risulta `NERR_Success`.[^12_1][^12_2]
Quindi: il **trust macchina↔dominio sui client è a posto**; il problema di accesso alle share non dipende da un canale sicuro rotto.

***

## Cosa possiamo dedurre da questo

Per i 3 client che non accedono alle share del nuovo server:

- Il secure channel (account computer in dominio) è sano quando verificato in CMD elevato.[^12_2][^12_3]
- Gli stessi utenti di dominio funzionano da altre macchine, ma non da questi 3 verso il server reinstallato.[^12_4]

Quindi il “collo di bottiglia” è:

- la **combinazione tra configurazione SMB/NTLM/firma sui 3 client** e la configurazione del nuovo server,
- non un problema di dominio rotto per quei computer.

***

## Prossimo passo mirato

Per andare avanti in modo utile sui client che non accedono al server reinstallato, conviene ora:

1. Verificare se da uno di questi 3 client:
    - riesci ad accedere ad **altre share di dominio** (file server diversi dal server di stampa), usando le stesse credenziali di dominio.[^12_5][^12_6]
    - Se sì, il problema è specifico client↔quel server. Se no, è un problema di rete/SMB più generale sul client.
2. Raccogliere su un client “rotto” e su un client “buono”:
    - output di `Get-SmbClientConfiguration` (almeno i campi `RequireSecuritySignature`, `EnableInsecureGuestLogons`, `EnableSecuritySignature`, `EnableSMB1Protocol`),[^12_6][^12_7][^12_8]
    - e poi, in seconda battuta, confrontare le policy di sicurezza (LAN Manager, Restrict NTLM, firma SMB) via `gpresult` o `secpol.msc`.[^12_9][^12_10][^12_4]

Appena hai conferma se i client “rotti” accedono o meno ad altre share di dominio (e magari mi riporti `Get-SmbClientConfiguration` di un client rotto e di uno buono), possiamo stringere ulteriormente su cosa li distingue.

<div align="center">⁂</div>

[^12_1]: https://spiral.hatenadiary.org/entry/20121217/1355720545

[^12_2]: https://ticonsigliotech.com/2025/11/10/utente-user-a-dominio-non-accetta-credenziali-dei-domain-administrators/

[^12_3]: https://www.reddit.com/r/PowerShell/comments/1eb494u/best_way_to_confirm_a_domain_computer_has_line_of/

[^12_4]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^12_5]: https://learn.microsoft.com/it-it/answers/questions/4028102/problema-rete-interna-e-condivisione-win-10-e-win

[^12_6]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance

[^12_7]: https://turbolab.it/windows-10/come-ripristinare-condivisione-cartelle-windows-11-4459

[^12_8]: https://support.keenetic.com/peak/kn-2710/it/43345-how-to-enable-access-to-a-network-drive-from-windows-11.html

[^12_9]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^12_10]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening


---

# Verificare se da uno di questi 3 client:

riesci ad accedere ad altre share di dominio (file server diversi dal server di stampa), usando le stesse credenziali di dominio.

Verso un mio pc win11 25H2, chiede le credenziali, fornisco le mie e le accetta

Questo dato è molto utile: i client “rotti” riescono ad accedere senza problemi ad altre share di dominio (per esempio al tuo PC 25H2) usando le stesse credenziali, quindi il problema è **specifico verso il nuovo server di stampa**, non generale su AD/NTLM/SMB lato client.[^13_1][^13_2]

***

## Cosa implica questo

Se:

- dagli stessi client problematici → altre share di dominio (es. il tuo PC) funzionano con le stesse credenziali;[^13_1]
- dagli altri client “buoni” → le share del server di stampa funzionano;[^13_3][^13_4]

allora:

- AD, NTLM e la configurazione SMB generale dei client sono sostanzialmente sani;
- il “triangolo” problematico è specificamente **(client X, server di stampa)**, cioè qualcosa nella configurazione del server (ACL, permessi share, firewall, firma SMB, o policy mirate) fa sì che SOLO alcuni client vengano rifiutati.[^13_2][^13_5][^13_6]

In altre parole: il server è ok “in generale”, i client sono ok “in generale”, ma c’è un’incompatibilità mirata tra quei tre client e quel server reinstallato.

***

## Dove ha senso guardare adesso sul server

Visto questo pattern, i punti caldi sul server sono:

1. **Permessi di condivisione e NTFS**
    - Verifica che sulle share delle stampanti/cartelle tu abbia davvero permessi a livello share/NTFS per `Domain Users` (o i gruppi usati), non solo per alcuni utenti specifici o per gruppi di computer.[^13_7][^13_1]
    - Controlla che non ci siano ACL “per computer” che escludono i tre client (o OU specifiche).[^13_1]
2. **Policy di sicurezza SMB lato server**
    - `RequireSecuritySignature`, `EncryptData` e simili sul server possono essere configurati in modo tale che solo alcuni client (con certe combinazioni di firma/crypto) falliscano autenticazione.[^13_8][^13_9][^13_6]
    - Vale la pena confrontare `Get-SmbServerConfiguration` sul server con quello di un file server che tutti vedono senza problemi.[^13_10][^13_8]
3. **Firewall / sicurezza sul server**
    - Un profilo firewall o un prodotto di sicurezza (AV/EDR) sul server potrebbe applicare regole per IP o subnet tali da colpire solo i tre client “rotti”, mentre altri passano.[^13_6][^13_11]

***

Dato che ora è chiaro che i tre client funzionano verso altri server di dominio ma non verso il server reinstallato, conviene spostare l’attenzione sul nuovo server: possiamo guardare insieme permessi share/NTFS e configurazione SMB del server, così verifichiamo se c’è qualche discriminante che potrebbe colpire solo quei tre client. Puoi dirmi se le share delle stampanti sul server sono pubbliche per `Domain Users` o se usate ACL più granulari?
<span style="display:none">[^13_12][^13_13][^13_14][^13_15][^13_16][^13_17][^13_18][^13_19]</span>

<div align="center">⁂</div>

[^13_1]: https://ticonsigliotech.com/2026/02/12/come-configurare-condivisione-file-server-utenti-dominio/

[^13_2]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^13_3]: https://learn.microsoft.com/it-it/answers/questions/5590843/accesso-a-risorse-condivise-da-windows-11-pro-25h2

[^13_4]: https://www.ilsoftware.it/focus/dopo-laggiornamento-a-windows-11-cartelle-condivise-e-stampanti-non-si-vedono-come-risolvere-subito/

[^13_5]: https://support.microsoft.com/it-it/windows/risolvere-i-problemi-di-connessione-e-stampa-in-windows-fb830bff-7702-6349-33cd-9443fe987f73

[^13_6]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance

[^13_7]: https://learn.microsoft.com/it-it/troubleshoot/windows-client/networking/access-denied-access-smb-file-share

[^13_8]: https://support.microsoft.com/it-it/help/887429/overview-of-server-message-block-signing

[^13_9]: https://etcsec.com/it/blog/firma-smb-disabilitata-ntlm-relay

[^13_10]: https://turbolab.it/windows-10/come-ripristinare-condivisione-cartelle-windows-11-4459

[^13_11]: https://support.keenetic.com/peak/kn-2710/it/43345-how-to-enable-access-to-a-network-drive-from-windows-11.html

[^13_12]: https://learn.microsoft.com/de-de/windows-server/storage/file-server/smb-ntlm-blocking?tabs=group-policy

[^13_13]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^13_14]: https://learn.microsoft.com/pt-br/windows-server/storage/file-server/smb-ntlm-blocking

[^13_15]: https://www.reddit.com/r/sysadmin/comments/1pzolhu/ntlm_authentication_issues_on_25h2/

[^13_16]: https://www.reddit.com/r/sysadmin/comments/1hnas4d/windows_11_24h2_update_cannot_access_network/

[^13_17]: https://www.facebook.com/groups/1240160309439552/posts/24640060859022834/

[^13_18]: https://learn.microsoft.com/it-it/answers/questions/5602061/dopo-istallazione-25h2-la-connessione-via-smb-dell

[^13_19]: https://www.youtube.com/watch?v=sIL8NhZ0vhM


---

```
# <q>Puoi dirmi se le share delle stampanti sul server sono pubbliche per Domain Users o se usate ACL più granulari?</q> e da dove lo vediamo, visto che le shares sono create dal pannello di controllo delle stampanti?
```

Per le stampanti condivise non vedi una “share” come per le cartelle, ma **i permessi effettivi stanno dentro le proprietà della stampante**, nelle schede Condivisione e Sicurezza; da lì capiamo se il server sta filtrando alcuni client/utenti.[^14_1][^14_2][^14_3]

***

## Dove guardare i permessi di condivisione della stampante

Sul server (dove è installata la stampante):

1. Apri la console delle stampanti sul server
    - Da Pannello di controllo → Dispositivi e stampanti, oppure
    - meglio: Server Manager → Tools → Print Management (Gestione stampa), che è il modo “server‑style”.[^14_4][^14_2][^14_3]
2. Individua la stampante condivisa:
    - In Print Management: Print Servers → NOME-SERVER → Printers.[^14_2][^14_3]
3. Clic destro sulla stampante → Proprietà stampante.[^14_5][^14_1][^14_2]

Qui hai due schede interessanti:
    - **Scheda Condivisione (Sharing)**
        - Vedi se è spuntato “Condividi la stampante” e il nome della share.[^14_6][^14_7][^14_3]
        - Di solito qui non gestisci i permessi di accesso dettagliati, ma solo l’abilitazione e il nome.
    - **Scheda Sicurezza (Security)**
        - È qui che vedi **chi può stampare**: gruppi come `Everyone`, `Authenticated Users`, `Domain Users` o gruppi più specifici.[^14_8][^14_1][^14_2]
        - Se, per esempio, sono autorizzati solo alcuni gruppi/utenti (o è stato rimosso `Domain Users` / `Authenticated Users`), è perfettamente possibile che:
            - da alcuni client (gli admin / pc di test) funzioni,
            - e da altri, dove ti logghi con utenti che non rientrano nei gruppi autorizzati, venga rifiutato l’accesso.[^14_9][^14_10][^14_8]

Per la parte “share” in senso SMB, la stampa di fatto passa tramite il servizio di stampa del server, quindi la **ACL di Sicurezza della stampante** è quella che conta, non una share NTFS separata.[^14_1][^14_2][^14_8]

***

Se ti va, il prossimo passo potrebbe essere: apri le proprietà di una delle stampanti che dà problemi, vai nella scheda Sicurezza e dimmi quali gruppi/utenti vedi (ad esempio: Everyone, Authenticated Users, Domain Users, singoli utenti), così capiamo se c’è già un filtro che spiega perché solo certi client/utenti riescono a connettersi.
<span style="display:none">[^14_11][^14_12][^14_13][^14_14][^14_15]</span>

<div align="center">⁂</div>

[^14_1]: https://www.guidoscuderi.it/informatica/FLIP/guida_win10/files/basic-html/page729.html

[^14_2]: https://giovannilubrano.blogspot.com/2020/04/33-windows-server-2019-configurare-le.html

[^14_3]: https://www.thesolving.com/it/sala-server/come-condividere-stampanti-attraverso-group-policy-gpo/

[^14_4]: https://www.html.it/pag/38977/configurare-la-condivisione-dei-file-e-stampanti/

[^14_5]: https://www.youtube.com/watch?v=5_oO3e4UtIk

[^14_6]: https://support.epson-europe.com/onlineguides/it/lq2190/html/soft_8.htm

[^14_7]: https://ij.manual.canon/ij/webmanual/PrinterDriver/W/TM-300/1.0/IT/PPG/dg-c_network_pp01.html

[^14_8]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-groups

[^14_9]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/use-group-policy-to-control-ad-printer

[^14_10]: https://www.reddit.com/r/sysadmin/comments/kz713d/configuring_access_control_for_printers_in/

[^14_11]: https://www.youtube.com/watch?v=rii3lTTkPYE

[^14_12]: https://www.ryadel.com/windows-server-permessi-nfts-cartelle-locali-rete-dominio-lan-audit-credenziali-acl-network/

[^14_13]: https://www.youtube.com/watch?v=7-T6o_7d99Y

[^14_14]: https://rdr-it.com/it/mappatura-della-stampante-gpo-e-script/

[^14_15]: https://microsoft.public.it.winserver.narkive.com/HpfhSRtf/problema-con-la-condivisione-di-stampante


---

# qui c'e' una grossa anomalia, un oggetto non viene risolto e potrebbe essere domain users o authenticated users

Hai centrato il punto: quell’SID “grezzo” non risolto nella scheda Sicurezza è quasi certamente un vecchio oggetto di dominio (tipicamente un gruppo come `Domain Users` o `Authenticated Users`) che sul nuovo server non viene più tradotto, e questo può creare esattamente situazioni di accesso incoerente tra client.[^15_1][^15_2]

Nello screenshot si vede:

- `Everyone`
- `TUTTI I PACCHETTI APPLICAZIONI`
- `CREATOR OWNER`
- Un SID lungo `S-1-5-13-1024-4048435139-...` non risolto
- `Administrator`
- `Administrators`

Se quel SID corrispondeva al gruppo che dava i diritti di stampa agli utenti di dominio prima della reinstallazione, ora hai una ACL “semispezzata”: gli admin locali/dominio vedono e usano la stampante, ma gli utenti normali possono trovarsi senza permessi effettivi (o con permessi che Windows non riesce più ad associare al loro token).[^15_2][^15_3][^15_1]

***

## Cosa fare operativamente sulla scheda Sicurezza

Sul server, per ciascuna stampante condivisa:

1. Apri Proprietà stampante → scheda **Sicurezza** (come nello screenshot).[^15_4][^15_5]
2. Rimuovi in modo esplicito l’oggetto non risolto (il SID lungo):
    - Seleziona la voce `S-1-5-13-1024-...` → Rimuovi.[^15_6][^15_1]
3. Aggiungi i gruppi corretti del nuovo dominio, in chiaro:
    - `Domain Users` → permesso **Stampa**.[^15_3][^15_2]
    - (opzionale ma spesso utile) `Authenticated Users` → Stampa.[^15_2]

Eventualmente, se volete essere più selettivi, aggiungi anche gruppi specifici (es. un gruppo “UtentiStampaGaslini”) e assegna Stampa.[^15_3][^15_2]
4. Verifica che `Everyone` non sia necessario (molti ambienti lo lasciano, ma è preferibile basarsi su `Domain Users`/`Authenticated Users`); l’importante è che ci sia almeno un gruppo che contenga gli utenti che stampano.[^15_2][^15_3]
5. Applica, chiudi, e **prova da uno dei client che prima rifiutavano credenziali**:
    - Aggiungere la stampante condivisa dal server,
    - oppure collegarsi tramite `\\server` e doppio clic sulla stampante.

Se il problema era dovuto a quel SID non risolto (vecchio gruppo di dominio), dopo averlo eliminato e sostituito con il gruppo corretto (`Domain Users`/`Authenticated Users`) la logica di autorizzazione di Windows torna coerente e i client dovrebbero smettere di richiedere credenziali all’infinito, permettendo la connessione con gli stessi utenti di dominio che funzionano su altri server.[^15_1][^15_3][^15_2]

***

In pratica, il passo chiave ora è: pulire quel SID orfano e aggiungere esplicitamente `Domain Users` con diritto di stampa su tutte le stampanti condivise del server. Una volta fatto, ti va di testare subito da uno dei tre client “rotti” se riesci finalmente a connettere la stampante usando un normale utente di dominio?

<div align="center">⁂</div>

[^15_1]: image.jpg

[^15_2]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-groups

[^15_3]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/use-group-policy-to-control-ad-printer

[^15_4]: https://giovannilubrano.blogspot.com/2020/04/33-windows-server-2019-configurare-le.html

[^15_5]: https://www.thesolving.com/it/sala-server/come-condividere-stampanti-attraverso-group-policy-gpo/

[^15_6]: https://www.ryadel.com/windows-server-permessi-nfts-cartelle-locali-rete-dominio-lan-audit-credenziali-acl-network/


---

# aggiunto authenticated users con stampa e gestione documenti ma dopo applica non e' cambiatonulla, ora sto riavviando

L’aggiunta di `Authenticated Users` con “Stampa” e “Gestione documenti” è il passo corretto lato ACL; se dopo il riavvio il comportamento non cambia, significa che il problema non è solo nei permessi della stampante, ma anche nella **fase di autenticazione SMB verso il server** (dato che i client problematici continuano a chiedere credenziali e rifiutarle).[^16_1][^16_2][^16_3]

***

## Cosa aspettarsi dopo il riavvio

Dopo aver:

- rimosso il SID non risolto,
- aggiunto `Authenticated Users` (e/o `Domain Users`) con permessi di stampa,[^16_4][^16_5][^16_1]

il server è, dal punto di vista ACL, in una configurazione “standard” in cui qualsiasi utente autenticato nel dominio ha i diritti necessari sulla stampante.[^16_5][^16_4]

Se al riavvio:

- i client “rotti” continuano a chiedere e rifiutare credenziali per la stampante,
mentre
- gli stessi utenti di dominio da quei client accedono correttamente ad altre share di dominio (es. al tuo PC 25H2),

allora la parte “autorizzazione stampante” è a posto, e resta da capire **perché l’autenticazione SMB verso *quel* server ancora non viene accettata**.

***

## Prossimo passo subito dopo il riavvio

Appena il server è tornato su, ti suggerirei di fare questi test, sempre da uno dei client problematici:

1. Accedere a una share “di test” sul server (non una stampante, ma una cartella condivisa sul nuovo server di stampa, con ACL NTFS+share per `Authenticated Users` / `Domain Users`).[^16_2][^16_6]
    - Se la cartella si apre con le stesse credenziali di dominio, la parte SMB verso quel server funziona e il problema è specifico alla stampante.
    - Se la cartella NON si apre e continua a chiedere/rifiutare credenziali, allora il problema è proprio SMB/auth verso quel server, e non la stampante in sé.[^16_7][^16_8][^16_9]
2. Provare la connessione alla stampante dopo aver aperto (o tentato di aprire) la cartella di test, così vedi se uno sblocca l’altro.

Se mi confermi cosa succede al riavvio:

- “la share di test si apre/non si apre”,
- “la stampante continua/ha smesso di chiedere credenziali”,

possiamo decidere se rimanere concentrati sulla stampa (RPC/Print) o tornare a guardare SMB/auth verso quel server.

<div align="center">⁂</div>

[^16_1]: image.jpg

[^16_2]: https://turbolab.it/windows-10/come-ripristinare-condivisione-cartelle-windows-11-4459

[^16_3]: https://learn.microsoft.com/it-it/answers/questions/5590843/accesso-a-risorse-condivise-da-windows-11-pro-25h2

[^16_4]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-groups

[^16_5]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/use-group-policy-to-control-ad-printer

[^16_6]: https://ticonsigliotech.com/2026/02/12/come-configurare-condivisione-file-server-utenti-dominio/

[^16_7]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^16_8]: https://learn.microsoft.com/it-it/troubleshoot/windows-client/networking/access-denied-access-smb-file-share

[^16_9]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance


---

```
# <q>Accedere a una share “di test” sul server (non una stampante, ma una cartella condivisa sul nuovo server di stampa, con ACL NTFS+share per Authenticated Users / Domain Users).Se la cartella si apre con le stesse credenziali di dominio, la parte SMB verso quel server funziona e il problema è specifico alla stampante.Se la cartella NON si apre e continua a chiedere/rifiutare credenziali, allora il problema è proprio SMB/auth verso quel server, e non la stampante in sé.</q> non puo' aprirsi se non vedo prima la lista dlele shares su \\590-subint01
```

Hai ragione: se il client non riesce a vedere nemmeno la lista delle share su `\\590-subint01`, non puoi “cliccare” sulla cartella di test; però il test si può comunque fare creando e verificando la share **dal lato server**, poi provando l’accesso diretto dal client.[^17_1][^17_2]

***

## Come impostare una share di test sul server

Sul server `590-SUBINT01`:

1. Crea una cartella, ad esempio `C:\TESTSHARE`.[^17_3]
2. Tasto destro → Proprietà → scheda **Condivisione** → Condivisione avanzata:
    - Spunta “Condividi la cartella”.
    - Dai un nome semplice, es. `TESTSHARE`.[^17_4][^17_3]
3. Clic su “Autorizzazioni” della condivisione:
    - Aggiungi `Authenticated Users` (o `Domain Users`) con “Lettura”.[^17_4]
4. Scheda **Sicurezza** (NTFS):
    - Assicurati che `Authenticated Users` / `Domain Users` abbiano almeno Lettura su `C:\TESTSHARE`.[^17_5][^17_4]

In questo modo, dal punto di vista SMB, hai una share “pulita” sul server, con ACL standard per dominio.

***

## Come testare la share dal client problematico

Dal client che non vede le share in browsing:

- Prova ad aprire direttamente `\\590-SUBINT01\TESTSHARE` nella barra degli indirizzi di Esplora risorse (o da `Win+R`).[^17_6][^17_1]

Non serve vedere l’elenco; l’importante è ciò che succede quando chiedi **quella** share:

- Se ti apre `TESTSHARE` chiedendo una sola volta le credenziali e poi le accetta, significa che **SMB/auth verso il server funziona**; resta solo il problema di browsing (NetBIOS discovery) e di stampante.[^17_7][^17_8]
- Se continua a chiedere/rifiutare credenziali anche per `\\590-SUBINT01\TESTSHARE`, allora il problema è proprio SMB/auth tra quel client e quel server (mentre tra lo stesso client e altri server funziona).[^17_2][^17_9][^17_10]

Quindi, anche senza lista di share, possiamo comunque isolare il comportamento su una share di test.

Ti va di provare `\\590-SUBINT01\TESTSHARE` dopo aver creato quella share e dirmi se il client riesce ad aprirla o continua a chiedere e rifiutare le credenziali?
<span style="display:none">[^17_11][^17_12][^17_13][^17_14][^17_15][^17_16][^17_17][^17_18][^17_19][^17_20]</span>

<div align="center">⁂</div>

[^17_1]: https://www.ilsoftware.it/focus/cartelle-condivise-in-windows-come-ottenere-la-lista-completa_15269/

[^17_2]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/file-server-smb-overview

[^17_3]: https://rdr-it.com/it/windows-server-2025-condivisione-di-una-cartella/

[^17_4]: https://learn.microsoft.com/it-it/answers/questions/2185945/visualizzare-le-cartelle-condivise-ai-soli-utenti

[^17_5]: https://www.ryadel.com/windows-server-permessi-nfts-cartelle-locali-rete-dominio-lan-audit-credenziali-acl-network/

[^17_6]: https://www.pcprofessionale.it/news/software-news/windows/come-elencare-condivisioni-rete-riga-comando-windows/

[^17_7]: https://learn.microsoft.com/it-it/answers/questions/4028102/problema-rete-interna-e-condivisione-win-10-e-win

[^17_8]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance

[^17_9]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^17_10]: https://learn.microsoft.com/it-it/troubleshoot/windows-client/networking/access-denied-access-smb-file-share

[^17_11]: https://rdr-it.com/it/come-vedere-i-file-aperti-di-una-condivisione-di-rete-su-un-server-windows/

[^17_12]: https://www.reddit.com/r/sysadmin/comments/1mopin/command_to_list_network_shares_on_a_server/

[^17_13]: https://netwrix.com/it/resources/guides/how-to-find-shared-folders/

[^17_14]: http://www.silenacampana.altervista.org/SitoNT/04risorse.html

[^17_15]: https://learn.microsoft.com/it-it/windows/win32/netshare/network-share-functions

[^17_16]: https://www.reddit.com/r/WindowsServer/comments/tewdex/windows_server_2008_how_to_get_shares_list/

[^17_17]: https://www.ilsoftware.it/focus/condividere-cartelle-in-windows-a-cosa-servono-le-condivisioni-amministrative_20911/

[^17_18]: https://support.firecore.com/hc/it/articles/18674431299991-Abilitazione-della-condivisione-dei-file

[^17_19]: https://turbolab.it/reti-1448/guida-windows-10-come-visualizzare-tutte-cartelle-condivise-rete-locale-lan-pc-corrente-1089

[^17_20]: http://www.pclinesas.com/download/gestionale1_servicepack/G1_Configurazioni%20di%20rete%20avanzate.pdf


---

# c'e' gia' e dal server stesso, via UNC, ci accedo liberamente

il server lista le proprie share senza problemi

questa e' \\590-subint01\demo  e compare insieme alle stampanti condivise e riesce ad accedervi. Lui stesso, i client invece no

Il quadro è quindi: sul server `590-SUBINT01` la share `\\590-subint01\demo` esiste, il server la vede e vi accede via UNC senza problemi, mentre i tre client “rotti” non vedono né `demo` né le share di stampa e continuano a chiedere/rifiutare credenziali solo verso questo server (ma verso altri host di dominio funzionano).[^18_1][^18_2][^18_3]

Questo conferma che:

- Il server è configurato correttamente dal proprio punto di vista (browsing e accesso a `demo` ok).
- I client sono in grado di autenticarsi via SMB verso altri host di dominio.[^18_4][^18_5]
- L’anomalia è specifica nella *relazione SMB/auth* tra quei client e `590-subint01`, non nella singola stampante o nella singola share.

***

## Cosa questo suggerisce tecnicamente

La casistica che corrisponde a “alcuni client non accedono a nessuna share su un server reinstallato, mentre altri sì, e l’ACL è stata ripulita” tipicamente chiama in causa:

- differenze di **policy SMB/NTLM** applicate ai client (via GPO) che interagiscono in modo diverso con il nuovo server,
- differenze di **profili di rete / firewall / endpoint security** sui client, che bloccano o interferiscono con l’handshake SMB solo verso quell’IP/hostname,
- oppure un mismatch di **nome/risoluzione** (es. il nome NetBIOS `590-subint01` risolve a IP diverso rispetto a quello che il server vede, o i client puntano a un altro host) che fa sì che il server giusto funzioni, ma i client targettino qualcosa di diverso.[^18_3][^18_6][^18_7][^18_1]

Dato che dal server stesso la share `demo` è perfettamente accessibile, mentre dai client non lo è, la direzione più utile ora è:

- guardare cosa succede sui client quando risolvono il nome `590-subint01`,
- e confrontare la configurazione SMB client e il profilo di rete tra un client buono e uno problematico.

***

Per mantenere il discorso coerente con tutta l’analisi che abbiamo fatto finora, il passo successivo sensato sui client “rotti” è:

- verificare che `ping 590-subint01` e `ping nomeFQDN` puntino davvero allo stesso IP che il server usa,
- e poi confrontare `Get-SmbClientConfiguration` e le impostazioni di rete/SMB tra client buoni e client rotti, per vedere se c’è un vincolo o differenza che si attiva solo verso questo server reinstallato.

Appena riesci, puoi dirmi da uno dei client “rotti” se `ping 590-subint01` e `ping 590-subint01.gaslini.lan` risolvono all’IP corretto del nuovo server, o se vedi qualche differenza/host diverso?

<div align="center">⁂</div>

[^18_1]: https://learn.microsoft.com/it-it/answers/questions/4028102/problema-rete-interna-e-condivisione-win-10-e-win

[^18_2]: https://www.ilsoftware.it/focus/cartelle-condivise-in-windows-come-ottenere-la-lista-completa_15269/

[^18_3]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/file-server-smb-overview

[^18_4]: https://ticonsigliotech.com/2026/02/12/come-configurare-condivisione-file-server-utenti-dominio/

[^18_5]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^18_6]: https://learn.microsoft.com/it-it/troubleshoot/windows-client/networking/access-denied-access-smb-file-share

[^18_7]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance


---

# nslookup risolve correttamente all' ip effettivo del server

Il fatto che `nslookup` risolva correttamente `590-subint01` all’IP effettivo del server elimina la pista “DNS/nomi sbagliati”: i client parlano davvero con il server giusto, ma l’autenticazione SMB verso quel server viene rifiutata, mentre verso altri host di dominio funziona.[^19_1][^19_2]

***

## Cosa rimane in gioco a questo punto

Con tutto quello che hai verificato finora:

- DNS/risoluzione: ok (nome host e FQDN puntano all’IP corretto).[^19_1]
- Trust di dominio lato client: ok (in CMD elevato `sc_verify` dà `NERR_Success`).[^19_3][^19_4]
- ACL stampante: ripulita, con `Authenticated Users` (e/o `Domain Users`) con permessi di stampa.[^19_5][^19_6][^19_7]
- I client problematici accedono correttamente ad altre share di dominio (es. il tuo PC 25H2) con le stesse credenziali.[^19_8][^19_9]
- Il server stesso vede e accede alle proprie share (`\\590-subint01\demo`).[^19_2][^19_10]

Restano quindi pochissimi “gradi di libertà”, tutti lato **combinazione tra SMB/NTLM client e configurazione SMB del server**:

- policy SMB/NTLM/firma sul server che discriminano alcuni client (differenze di build, di cipher suite, di firma richiesta, di encryption),[^19_11][^19_12][^19_13]
- policy SMB/NTLM/firma sui tre client, che rispetto agli altri sono leggermente più rigide e, con il server reinstallato, sfociano nel rifiuto credenziali.[^19_14][^19_15][^19_16][^19_17]

Dal punto di vista sistemistico, siamo in un punto in cui serve proprio un “diff” tra:

- configurazione SMB del server `590-subint01` e quella di un file server che tutti vedono,
- configurazione SMB client (e GPO di sicurezza) tra client buoni e client rotti.

***

Dato che hai già verificato DNS e trust, il prossimo passo con il massimo rapporto segnale/rumore è:

1. Su `590-subint01`:
    - raccogliere `Get-SmbServerConfiguration` (firma, encryption, SMB1/2/3, ecc.).[^19_2][^19_11]
2. Su un client buono e su uno “rotto”:
    - raccogliere `Get-SmbClientConfiguration` (già parzialmente fatto),
    - e `gpresult /H` per confrontare le policy di sicurezza su NTLM/firma.[^19_15][^19_17][^19_18][^19_14]

Con quelle informazioni possiamo finalmente vedere se c’è un requisito (es. firma obbligatoria, encryption, blocco NTLM verso SMB) che si applica solo alla coppia “client rotto ↔ nuovo server” e non alle altre combinazioni che funzionano.

Quando hai un attimo, ti va di raccogliere:

- `Get-SmbServerConfiguration` su `590-subint01`,
- `Get-SmbClientConfiguration` su un client buono e su uno rotto (almeno i campi firma/encryption/smb1),

così possiamo guardare nel dettaglio quali flag di sicurezza potrebbero essere responsabili del comportamento attuale?

<div align="center">⁂</div>

[^19_1]: https://learn.microsoft.com/it-it/answers/questions/4028102/problema-rete-interna-e-condivisione-win-10-e-win

[^19_2]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/file-server-smb-overview

[^19_3]: https://ticonsigliotech.com/2025/11/10/utente-user-a-dominio-non-accetta-credenziali-dei-domain-administrators/

[^19_4]: https://www.reddit.com/r/PowerShell/comments/1eb494u/best_way_to_confirm_a_domain_computer_has_line_of/

[^19_5]: image.jpg

[^19_6]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-groups

[^19_7]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/use-group-policy-to-control-ad-printer

[^19_8]: https://ticonsigliotech.com/2026/02/12/come-configurare-condivisione-file-server-utenti-dominio/

[^19_9]: https://ticonsigliotech.com/2025/05/06/impossibile-accedere-alla-cartella-condivisa-dopo-aggiornamento-windows/

[^19_10]: https://www.ilsoftware.it/focus/cartelle-condivise-in-windows-come-ottenere-la-lista-completa_15269/

[^19_11]: https://support.microsoft.com/it-it/help/887429/overview-of-server-message-block-signing

[^19_12]: https://etcsec.com/it/blog/firma-smb-disabilitata-ntlm-relay

[^19_13]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance

[^19_14]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^19_15]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^19_16]: https://www.reddit.com/r/sysadmin/comments/1pzolhu/ntlm_authentication_issues_on_25h2/

[^19_17]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening

[^19_18]: https://wktaaeu.my.site.com/itcommunity/s/article/Nota-sistemistica-B-Point-Problemi-di-accesso-dopo-l-installazione-di-Windows-11


---

# questo e' il server

PS C:\WINDOWS\system32> Get-SmbServerConfiguration

AnnounceComment                        :
AnnounceServer                         : False
AsynchronousCredits                    : 64
AuditClientCertificateAccess           : False
AuditClientDoesNotSupportEncryption    : False
AuditClientDoesNotSupportSigning       : False
AuditInsecureGuestLogon                : False
AuditSmb1Access                        : False
AutoDisconnectTimeoutInMinutesV1       : 15
AutoDisconnectTimeoutInSecondsV2       : 900
AutoShareServer                        : True
AutoShareWorkstation                   : True
CachedOpenLimit                        : 10
DisableCompression                     : False
DisableSmbEncryptionOnSecureConnection : True
DurableHandleV2TimeoutInSeconds        : 180
EnableAuthenticateUserSharing          : False
EnableAuthRateLimiter                  : True
EnableDirectoryHandleLeasing           : True
EnableDownlevelTimewarp                : False
EnableForcedLogoff                     : True
EnableLeasing                          : True
EnableMailslots                        : False
EnableMultiChannel                     : True
EnableOplocks                          : True
EnableSecuritySignature                : False
EnableSMB1Protocol                     : True
EnableSMB2Protocol                     : True
EnableSMBQUIC                          : True
EnableStrictNameChecking               : True
EncryptData                            : False
EncryptionCiphers                      : AES_128_GCM, AES_128_CCM, AES_256_GCM, AES_256_CCM
InvalidAuthenticationDelayTimeInMs     : 2000
IrpStackSize                           : 15
KeepAliveTime                          : 2
MaxChannelPerSession                   : 32
MaxMpxCount                            : 50
MaxSessionPerConnection                : 16384
MaxThreadsPerQueue                     : 20
MaxWorkItems                           : 1
NullSessionPipes                       :
NullSessionShares                      :
OplockBreakWait                        : 35
PendingClientTimeoutInSeconds          : 120
RejectUnencryptedAccess                : True
RequestCompression                     : False
RequireSecuritySignature               : False
RestrictNamedpipeAccessViaQuic         : True
ServerHidden                           : True
Smb2CreditsMax                         : 2048
Smb2CreditsMin                         : 128
Smb2DialectMax                         : None
Smb2DialectMin                         : None
SmbServerNameHardeningLevel            : 0
TreatHostAsStableStorage               : False
ValidateAliasNotCircular               : True
ValidateShareScope                     : True
ValidateShareScopeNotAliased           : True
ValidateTargetName                     : True

e questo e' il client

PS C:\WINDOWS\system32> Get-SmbServerConfiguration

AnnounceComment                        :
AnnounceServer                         : False
AsynchronousCredits                    : 64
AuditClientCertificateAccess           : False
AuditClientDoesNotSupportEncryption    : False
AuditClientDoesNotSupportSigning       : False
AuditInsecureGuestLogon                : False
AuditSmb1Access                        : False
AutoDisconnectTimeoutInMinutesV1       : 15
AutoDisconnectTimeoutInSecondsV2       : 900
AutoShareServer                        : True
AutoShareWorkstation                   : True
CachedOpenLimit                        : 10
DisableCompression                     : False
DisableSmbEncryptionOnSecureConnection : True
DurableHandleV2TimeoutInSeconds        : 180
EnableAuthenticateUserSharing          : False
EnableAuthRateLimiter                  : True
EnableDirectoryHandleLeasing           : True
EnableDownlevelTimewarp                : False
EnableForcedLogoff                     : True
EnableLeasing                          : True
EnableMailslots                        : False
EnableMultiChannel                     : True
EnableOplocks                          : True
EnableSecuritySignature                : False
EnableSMB1Protocol                     : True
EnableSMB2Protocol                     : True
EnableSMBQUIC                          : True
EnableStrictNameChecking               : True
EncryptData                            : False
EncryptionCiphers                      : AES_128_GCM, AES_128_CCM, AES_256_GCM, AES_256_CCM
InvalidAuthenticationDelayTimeInMs     : 2000
IrpStackSize                           : 15
KeepAliveTime                          : 2
MaxChannelPerSession                   : 32
MaxMpxCount                            : 50
MaxSessionPerConnection                : 16384
MaxThreadsPerQueue                     : 20
MaxWorkItems                           : 1
NullSessionPipes                       :
NullSessionShares                      :
OplockBreakWait                        : 35
PendingClientTimeoutInSeconds          : 120
RejectUnencryptedAccess                : True
RequestCompression                     : False
RequireSecuritySignature               : False
RestrictNamedpipeAccessViaQuic         : True
ServerHidden                           : True
Smb2CreditsMax                         : 2048
Smb2CreditsMin                         : 128
Smb2DialectMax                         : None
Smb2DialectMin                         : None
SmbServerNameHardeningLevel            : 0
TreatHostAsStableStorage               : False
ValidateAliasNotCircular               : True
ValidateShareScope                     : True
ValidateShareScopeNotAliased           : True
ValidateTargetName                     : True

Server e client hanno una configurazione SMB server **praticamente identica**, con punti chiave: `EnableSMB1Protocol = True`, `RequireSecuritySignature = False`, `EncryptData = False`, `RejectUnencryptedAccess = True`.[^20_1][^20_2]
Quindi non emerge, da questi parametri, una differenza evidente che spieghi perché quei client falliscono solo verso `590-subint01`.

***

## Cosa possiamo dedurre da queste configurazioni

Dal dump di `Get-SmbServerConfiguration`:

- Firma SMB non obbligatoria:
    - `EnableSecuritySignature = False`
    - `RequireSecuritySignature = False`[^20_2][^20_3]
- Crittografia SMB non obbligatoria, ma il server **rifiuta accessi non cifrati**:
    - `EncryptData = False`
    - `RejectUnencryptedAccess = True`[^20_4][^20_1][^20_2]
- SMB1 e SMB2/3 abilitati; QUIC abilitato.

Il fatto che `RejectUnencryptedAccess = True` con `EncryptData = False` è interessante: significa che il server, per alcune share o in certe condizioni, può pretendere cifratura e rifiutare sessioni che non negoziano encryption, anche se globalmente non è impostato “Encrypt all”.[^20_1][^20_4]

Tuttavia:

- Siccome server e client hanno la stessa configurazione,
- e i client problematici riescono ad accedere ad altre share di dominio (su altri host) con le stesse credenziali,

non possiamo attribuire il comportamento solo a queste opzioni generali; la discriminante è con tutta probabilità **a livello di share specifica, ACL, o policy SMB applicate per quell’host**, non di configurazione globale.

***

## Dove ha senso guardare ora

Dato che:

- lato SMB server globale non ci sono differenze tra 590-subint01 e il client che funge da “server” quando condividi dal tuo PC,
- lato ACL stampante abbiamo ripulito il SID orfano e aggiunto `Authenticated Users`,

rimangono due piani su cui conviene concentrarsi:

1. **Impostazioni specifiche della share `demo` e delle share di stampa sul server**
    - Permessi di condivisione (share permissions) e NTFS su `demo` e sulle cartelle eventualmente usate dalle stampanti (se presente un driver che crea directory spool/condivisione).[^20_5][^20_6]
    - Eventuali “limitazioni di accesso” per computer/utenti specifici (es. share visibili solo a certi gruppi).[^20_7][^20_5]
2. **Policy di sicurezza sui client “rotti” applicate via GPO**
    - `gpresult` su client buono vs client rotto, limitandoci alle voci su:
        - NTLM (Restrict NTLM),
        - LAN Manager auth level,
        - firma SMB client.[^20_8][^20_9][^20_10][^20_11]

Le configurazioni SMB server che hai mostrato ci dicono che non c’è un “hardening extra” su 590-subint01 rispetto alla macchina dove condividi dal tuo PC; quindi il focus rimane su:

- differenze negli ACL per quella share/stampante,
- e differenze nelle policy di sicurezza SMB/NTLM applicate ai tre client.

***

Visto che i parametri SMB server di 590-subint01 non mostrano anomalie rispetto al client, ti va di fare il passo successivo che avevamo accennato: generare `gpresult /H` per un client buono e uno rotto e guardare solo le voci NTLM/LAN Manager/firma SMB, così possiamo individuare se c’è almeno una policy diversa tra i due?

<div align="center">⁂</div>

[^20_1]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/file-server-smb-overview

[^20_2]: https://support.microsoft.com/it-it/help/887429/overview-of-server-message-block-signing

[^20_3]: https://etcsec.com/it/blog/firma-smb-disabilitata-ntlm-relay

[^20_4]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance

[^20_5]: https://learn.microsoft.com/it-it/answers/questions/2185945/visualizzare-le-cartelle-condivise-ai-soli-utenti

[^20_6]: https://rdr-it.com/it/windows-server-2025-condivisione-di-una-cartella/

[^20_7]: https://learn.microsoft.com/it-it/troubleshoot/windows-client/networking/access-denied-access-smb-file-share

[^20_8]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^20_9]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^20_10]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening

[^20_11]: https://wktaaeu.my.site.com/itcommunity/s/article/Nota-sistemistica-B-Point-Problemi-di-accesso-dopo-l-installazione-di-Windows-11


---

# perche' li vuoi in html?

In HTML non è “obbligatorio”, è solo il formato che di solito rende più comodo confrontare le policy perché puoi aprire i due report in un browser e cercare/affiancare le sezioni rilevanti.

Dal punto di vista tecnico:

- `gpresult /H` genera un report strutturato, con le sezioni ben separate (Configurazione computer, Configurazione utente, Criteri di sicurezza, Modelli amministrativi, ecc.).
- In HTML hai testo, titoli e formattazione che rendono più semplice individuare voci come:
    - “Sicurezza di rete: livello di autenticazione LAN Manager”
    - “Sicurezza di rete: limitare NTLM…”
    - “Client di rete Microsoft: firma digitale le comunicazioni…”

Se per te è più comodo, puoi usare anche:

- `gpresult /V > gpresult_client_buono.txt` e `gpresult /V > gpresult_client_rotto.txt`
- e poi fare diff dei due `.txt` concentrandoti sulle sezioni “Opzioni di sicurezza” e “Modelli amministrativi → Rete → Workstation Lanman”.

Quello che veramente ci serve non è il formato, ma:

- l’elenco delle policy **diverse** tra client buono e client rotto, in particolare quelle legate a NTLM, LAN Manager e firma SMB.

Se vuoi, puoi semplicemente copiare a mano (o incollare qui) le righe di quelle tre voci per ciascun client, invece di generare l’HTML completo.

---

# non riesco ad espandere le sezioni per far funzionare il search, ti passo i files interi?

Sì, se per te è più pratico puoi passarmi i file interi: va benissimo usare i `.html` generati con `gpresult /H` o anche dei `.txt` ottenuti con `gpresult /V > file.txt`.[^22_1]

Il vincolo, in questo contesto, è che:

- io ho bisogno di vedere **le parti rilevanti** (NTLM, LAN Manager, firma SMB) per capire le differenze tra client buono e client rotto;
- non è strettamente necessario che tu faccia search/expand localmente, possiamo lavorare direttamente sui contenuti dei file.

Quindi, se non ti è comodo usare la ricerca nel browser:

- allega qui i due file (`gpresult_client_buono.html` e `gpresult_client_rotto.html`, o le versioni `.txt`),
- oppure, se preferisci, copia/incolla i blocchi “Opzioni di sicurezza” dalle due sezioni “Configurazione computer”.

Con quelli riesco a indicarti precisamente quali voci NTLM/LAN Manager/firma differiscono tra i tre client che non accedono e quelli che funzionano, evitando che tu debba fare il diff manuale.
<span style="display:none">[^22_10][^22_2][^22_3][^22_4][^22_5][^22_6][^22_7][^22_8][^22_9]</span>

<div align="center">⁂</div>

[^22_1]: https://learn.microsoft.com/it-it/answers/questions/2815032/ricerca-testo-interno-file

[^22_2]: https://learn.microsoft.com/it-it/answers/questions/5492708/problemi-windows-search?translated=true

[^22_3]: https://forum.ubuntu-it.org/viewtopic.php?t=650614

[^22_4]: https://learn.microsoft.com/it-it/answers/questions/2676562/problemi-di-ricerca-contenuto-file?forum=windows-all

[^22_5]: https://support.microsoft.com/it-it/topic/cercare-contenuto-74392516-1395-4e7e-acc7-76cbf37f89fd

[^22_6]: https://www.aranzulla.it/ricercare-file-alla-velocita-della-luce-8114.html

[^22_7]: https://turbolab.it/windows-10/10-programmi-fare-ricerca-testuale-cartella-4113

[^22_8]: https://learn.microsoft.com/it-it/visualstudio/ide/find-in-files?view=visualstudio

[^22_9]: https://learn.microsoft.com/it-it/answers/questions/2410582/la-funzione-cerca-non-trova-il-testo-contenuto-nei

[^22_10]: https://www.fastweb.it/fastweb-plus/digital-dev-security/consigli-per-migliorare-la-ricerca-in-windows-11/


---

# questo e' il client rotto

        Opzioni di sicurezza
        --------------------
            Oggetto Criteri di gruppo: Default Domain Policy
                Criterio:            PasswordComplexity
                Impostazione computer:  Attivato
    
            Oggetto Criteri di gruppo: Default Domain Policy
                Criterio:            ClearTextPassword
                Impostazione computer:  Non abilitato
    
            Oggetto Criteri di gruppo: Default Domain Policy
                Criterio:            ForceLogoffWhenHourExpire
                Impostazione computer:  Non abilitato
    
            Oggetto Criteri di gruppo: Default Domain Policy
                Criterio:            RequireLogonToChangePassword
                Impostazione computer:  Non abilitato
    
            Oggetto Criteri di gruppo: Default Domain Policy
                Criterio:            @wsecedit.dll,-59029
                NomeValore:         MACHINE\Software\Microsoft\Windows\CurrentVersion\Policies\System\LegalNoticeCaption
    Quello che hai incollato è solo l’inizio delle “Opzioni di sicurezza” (parte password/logon); non contiene ancora le voci critiche per SMB/NTLM (tipo “Sicurezza di rete: livello di autenticazione LAN Manager”, “Sicurezza di rete: limitare NTLM…” o “Client di rete Microsoft: firma digitale le comunicazioni…”), quindi da questa porzione non si vede nulla che spieghi il problema.[^23_1][^23_2][^23_3]

Per capire se il client “rotto” ha policy diverse dai client buoni proprio sulle aree che ci interessano, ci servono le righe che in `gpresult` appaiono con etichette simili a:

- “Sicurezza di rete: livello di autenticazione LAN Manager”
- “Sicurezza di rete: limitare NTLM SSP (NTLMv2 session security)” / “Restrict NTLM”
- “Client di rete Microsoft: firma digitale le comunicazioni (sempre)”
- “Client di rete Microsoft: firma digitale le comunicazioni (se accettata dal server)”

Queste, in `gpresult`, stanno più in basso nella stessa sezione “Opzioni di sicurezza” sotto Configurazione computer.[^23_2][^23_3][^23_4][^23_1]

Se ti è più comodo, puoi:

- cercare nel file `gpresult` le stringhe “Sicurezza di rete:” o “Client di rete Microsoft:” e copiare tutto quel blocco per il client rotto,
- e fare lo stesso per un client buono, così possiamo vedere se almeno una di queste voci è diversa.

Altrimenti, se il file è lungo e scomodo da gestire, puoi incollare qui “tutta” la sezione Opzioni di sicurezza del client rotto e poi quella del client buono; guarderò io quali voci NTLM/LAN Manager/firma differiscono.

<div align="center">⁂</div>

[^23_1]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^23_2]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^23_3]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening

[^23_4]: https://wktaaeu.my.site.com/itcommunity/s/article/Nota-sistemistica-B-Point-Problemi-di-accesso-dopo-l-installazione-di-Windows-11


---

# non ricorre la parola rete

Se in quella sezione non trovi nessuna voce con “Sicurezza di rete:” o “Client di rete Microsoft…”, allora, per quel client, **non ci sono policy di sicurezza esplicite su NTLM / LAN Manager / firma SMB impostate via GPO**: sta usando i default di sistema.[^24_1][^24_2][^24_3]
Questo è già un’informazione utile: se i client buoni sono nella stessa OU e hanno la stessa Default Domain Policy, anche loro con molta probabilità non hanno policy SMB/NTLM specifiche, quindi il problema non nasce da una GPO diversa.

***

## Cosa implica “nessuna voce Sicurezza di rete”

In `gpresult`, se non compaiono voci tipo:

- “Sicurezza di rete: livello di autenticazione LAN Manager”
- “Sicurezza di rete: limitare NTLM…”
- “Client di rete Microsoft: firma digitale le comunicazioni…”

significa che:

- nessuna GPO sta sovrascrivendo queste impostazioni;
- il client usa i default di Windows su NTLM/LM/firma SMB.[^24_2][^24_3][^24_1]

Se i client buoni sono nello stesso dominio/OU e non hanno voci diverse, allora la differenza tra client rotto e client buono non è nelle **policy di sicurezza** ma altrove (per esempio nello **stato locale**: cache credenziali, profilo di rete, firewall, endpoint security).

***

A questo punto, visto che:

- GPO sulle opzioni di sicurezza non mostrano differenze,
- SMB server/global è identico,
- DNS e trust AD sono ok,
- i client rotti accedono ad altre share di dominio ma non a quelle di 590-subint01,

abbiamo ristretto il problema a uno “strato” molto sottile (probabilmente interazione tra spooler/Print RPC e SMB su quel server). Il passo successivo sensato è tornare al comportamento concreto dopo il riavvio:

- i client rotti continuano a chiedere e rifiutare credenziali sia per `\\590-subint01\demo` sia per la stampante,
o è cambiato qualcosa?

Puoi confermare cosa succede adesso, dopo riavvio e modifica ACL stampante, quando da un client “rotto” provi ad accedere a `\\590-subint01\demo` e ad aggiungere la stampante?

<div align="center">⁂</div>

[^24_1]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^24_2]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^24_3]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening


---

# allora:

- ho riavviato tutto e non e' cambiato niente
- ho aggiunto una stampante di prova sul server e mi ha ricreato una clausola di sicurezza per quel SID non risolto, secondo me non e' stato corretto cancellare quella precedente
- da sestesso via UNC vedo tutto compresa la file share demo e la nuova stampante

Il fatto che aggiungendo una nuova stampante il server ti ricrei **di nuovo lo stesso SID non risolto** nelle ACL conferma che quello non è “spazzatura vecchia”, ma un oggetto ancora usato dalla configurazione (probabilmente un gruppo di dominio o un SID well‑known che la GUI non riesce a risolvere correttamente). Cancellarlo senza sapere cosa sia può effettivamente alterare il modello di sicurezza previsto.[^25_1][^25_2][^25_3]

***

## Cosa ci dice il comportamento attuale

Dopo riavvio:

- nulla è cambiato sui client: continuano a chiedere/rifiutare credenziali verso `590-subint01` (share + stampanti).
- Sul server:
    - via UNC `\\590-subint01` il server stesso vede tutte le share, incluse `demo` e la nuova stampante.
    - Creando una nuova stampante, la scheda Sicurezza mostra di nuovo quell’SID lungo non risolto.

Quindi:

- la logica di condivisione stampante sul server **dipende** da quel SID (probabilmente per “Authenticated Users” o un gruppo speciale),
- i client buoni riescono a connettersi nonostante quel SID,
- i client rotti no, e l’aver rimosso la precedente voce SID potrebbe aver peggiorato la situazione per alcune stampanti.[^25_2][^25_3][^25_1]

***

## Cosa fare adesso con quel SID

Dato che il server continua a creare quella voce:

- la cosa sensata non è più rimuoverla, ma **capire chi è** e replicare esplicitamente il gruppo corrispondente in forma risolta (nome leggibile), mantenendo però il SID originale finché non sappiamo che è ridondante.[^25_3][^25_2]

Passi operativi sul server:

1. In `lusrmgr.msc` / “Utenti e gruppi locali” e in AD Users \& Computers, cerca se hai un gruppo di dominio con SID corrispondente (anche solo con strumenti tipo `whoami /groups` o `wmic group get name,sid` per vedere SIDs dei gruppi standard vs quello della ACL).[^25_4][^25_2]
2. In alternativa, usa `icacls` o `powershell` per esportare la ACL della stampante e vedere il SID completo; confrontalo con i SID di gruppi standard come `Authenticated Users`, `Domain Users`, `Users`, ecc.[^25_5][^25_2]
3. Una volta identificato chi è quel SID (es. `Authenticated Users`), aggiungi il gruppo corrispondente **in chiaro** alla ACL (con Stampa/Gestione documenti), senza cancellare il SID finché non sei sicuro di avere equivalenza.[^25_2][^25_3]

***

## Perché i client buoni vedono tutto e i client rotti no

Dall’insieme dei test, il modello che regge è:

- Server:
    - ACL stampanti con Everyone + SID speciale + Admin;
    - SMB server configurato in modo standard (SMB1/2/3, senza firma obbligatoria).[^25_6][^25_7]
- Client buoni:
    - risolvono il SID e/o hanno token/gruppi che lo soddisfano,
    - accedono a share e stampanti su `590-subint01` normalmente.
- Client rotti:
    - autenticazione SMB verso `590-subint01` parte (ti chiede credenziali),
    - ma l’autorizzazione non arriva a un token/gruppo che il server riconosce come avente diritto (ACL + SID particolare),
    - quindi rifiuta le credenziali, mentre verso altri host, con ACL più semplici, gli stessi utenti funzionano.[^25_8][^25_9][^25_10]

In pratica, sei davanti a un problema di **modellazione di ACL lato server** che interagisce male con la composizione dei gruppi di alcuni client/utenti, non con NTLM/SMB “in generale”.

***

## Direzione concreta da qui

Visto che:

- il server continua a rigenerare quel SID,
- il riavvio non cambia nulla,
- e i client rotti continuano a fallire solo verso questo server,

le mosse sensate ora sono:

- identificare con precisione cosa rappresenta quel SID e aggiungere esplicitamente i gruppi utente giusti (`Domain Users`, `Authenticated Users`) alle ACL,
- verificare se, su una stampante “nuova” con ACL costruita pulita (SID + `Domain Users` aggiunto), almeno un client “rotto” riesce a connettersi,
- se continua a fallire, guardare esclusivamente lato client le differenze di gruppi nel token (che gruppi ha l’utente loggato su quei client rispetto agli utenti su client buoni).

Intanto hai fatto bene a notare che la ricomparsa del SID indica che cancellare la precedente voce non era la soluzione strutturale; al prossimo passo, appena ti è possibile, ti conviene estrarre il SID completo e mappare chi è, così possiamo impostare ACL robusti che non dipendano da un identificatore “misterioso”.

Vuoi che ti schematizzi rapidamente i comandi per ottenere la lista `name,sid` dei gruppi standard sul server, in modo da confrontarla con il SID che vedi nella ACL?

<div align="center">⁂</div>

[^25_1]: image.jpg

[^25_2]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-groups

[^25_3]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/use-group-policy-to-control-ad-printer

[^25_4]: https://support.microsoft.com/it-it/topic/errori-di-autenticazione-kerberos-e-ntlm-dovuti-a-sid-duplicati-76f7394d-c460-4882-9ed1-d27e0960f949

[^25_5]: https://www.ryadel.com/windows-server-permessi-nfts-cartelle-locali-rete-dominio-lan-audit-credenziali-acl-network/

[^25_6]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/file-server-smb-overview

[^25_7]: https://support.microsoft.com/it-it/help/887429/overview-of-server-message-block-signing

[^25_8]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^25_9]: https://learn.microsoft.com/it-it/troubleshoot/windows-client/networking/access-denied-access-smb-file-share

[^25_10]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/networking/troubleshoot-smb-guidance


---

```
# <q>verificare se, su una stampante “nuova” con ACL costruita pulita (SID + Domain Users aggiunto), almeno un client “rotto” riesce a connettersi,</q> gia' fatto, comunque non arriviamo alla lista delle shares dai client rotti.
```

quel sid non sembra corrispondere a nulla di locale pero' se chiedo quel wmic ad un client buono vedo altre ai nomi locali (dei gruppi) anche i gruppi di dominio mentre dal server vedo solo i locali

Il comportamento che descrivi è molto indicativo: il server, con `wmic`, vede solo gruppi locali; i client buoni vedono anche i gruppi di dominio; e ogni volta che aggiungi una nuova stampante, il server ricrea quel SID non risolto nella ACL. Questo quadro combacia con due fenomeni documentati: **SID “speciali” non risolti dalle GUI** e problemi di **SID duplicati / hardening SID** che impattano autenticazione SMB/NTLM in 24H2/25H2.[^26_1][^26_2][^26_3]

***

## Cosa suggerisce il SID non risolto che torna sempre

Quando Windows mostra in una ACL un SID lungo non risolto e lo ricrea ogni volta, la documentazione distingue due casi:[^26_1]

- **SID di funzionalità (capability SID)**: sono SIDs che non vengono mai risolti in nomi “amichevoli” per design; appartengono a classi di funzionalità (es. pacchetti applicazioni, capability di sistema).
- **SID non risolti di oggetti AD o locali**: es. gruppi di dominio, account di servizio, ecc.

Microsoft suggerisce precisamente:

- Controllare il ramo
`HKLM\SOFTWARE\Microsoft\SecurityManager\CapabilityClasses\AllCachedCapabilities`
per vedere se il SID appare tra i SID di funzionalità; se sì, è un SID speciale usato da Windows e non va cancellato.[^26_1]
- Se non è un SID di funzionalità noto, è un oggetto “normale” (utente/gruppo) che per qualche ragione non si risolve; in questo caso è sensato mappare a chi corrisponde e sostituirlo con una ACL esplicita (es. `Authenticated Users`, `Domain Users`).[^26_4][^26_1]

Il fatto che il server continui a ricreare quel SID nelle ACL delle stampanti, mentre i client buoni riescono a stampare e i client rotti no, fa pensare che:

- quel SID partecipi alla logica di “chi può stampare”,
- ma la sua risoluzione/proiezione sul token di sicurezza degli utenti può essere diversa tra macchine con SID/locali diversi o tra sistemi clonati e non.[^26_5][^26_2][^26_3][^26_4]

***

## Ruolo possibile dei SID duplicati / imaging

Gli aggiornamenti di fine 2025 su Windows 11 24H2/25H2 e Windows Server 2025 introducono controlli più rigidi sui **SID duplicati** (macchine clonate senza Sysprep, account macchina non unici, ecc.).[^26_2][^26_3]

Conseguenze illustrate:

- su sistemi con SID duplicati, l’autenticazione NTLM/Kerberos può fallire in modo selettivo (es. verso certi server o servizi), pur funzionando altrove;
- si vedono sintomi come rifiuto di credenziali corrette, problemi con RDP, SMB, stampa;
- compaiono eventi LsaSrv 6167 nel System log.[^26_3][^26_5][^26_2]

Nel tuo scenario:

- il server è stato reinstallato, quindi ha SID macchina “pulito”;
- i client rotti potrebbero derivare da immagini storiche/clonate, con SID macchina duplicati tra loro o rispetto a altre macchine;[^26_5][^26_2][^26_3]
- questo, combinato con hardening SID introdotto dalle patch, può far sì che il server `590-subint01` rifiuti handshake NTLM/SMB da quei client, mentre altri server ancora li accettano o non sono ancora “irrigiiditi” allo stesso modo.[^26_3]

È coerente con:

- accesso SMB OK verso altri host,
- rifiuto sistematico delle credenziali solo verso il server reinstallato,
- nessuna differenza GPO evidente su NTLM/LAN Manager,
- presenza di un SID non risolto nelle ACL di stampanti, che potrebbe essere interpretato diversamente in base ai SID macchina/utente.[^26_6][^26_7][^26_8][^26_4][^26_3]

***

## Direzione pragmatica data la complessità

Qui stiamo entrando in un territorio dove la combinazione di:

- ACL stampante con SID speciali,
- hardening SID/NTLM di 24H2/25H2,
- storia di imaging/clonazione dei client,

diventa difficile da risolvere solo “a colpi di flag SMB”. Le linee guida sistemistiche per SID duplicati e problemi di autenticazione sono piuttosto nette:[^26_2][^26_3]

- verificare se i client rotti derivano da immagini clonata senza Sysprep (stessi SID macchina);
- se sì, rigenerare quei sistemi in modo conforme (nuova installazione o immagine correttamente generalizzata);
- nel frattempo, cercare eventi LsaSrv 6167 su server e client durante i tentativi di accesso, per confermare che hardening SID sta intervenendo.[^26_5][^26_2][^26_3]

Dato quanta analisi hai già fatto e quanto è specifico il problema (tre client vs un server reinstallato), la soluzione strutturale potrebbe essere più efficiente:

- **ricostruire almeno uno dei client rotti** da zero (installazione pulita di 25H2, join a dominio, nessun imaging, GPO standard),
- verificare se così la connessione alle share/stampanti di `590-subint01` funziona subito;
- se sì, hai la conferma che lo stato storico di quella macchina (SID, registry, policy residue, imaging) è il vero discriminante, più che singole chiavi SMB/NTLM.

A questo punto, visto quanto hai già investito, ti chiedo: preferisci continuare a dissezionare il comportamento dei SID/ACL su questi client, o ti è più accettabile rigenerare almeno una delle tre macchine e usarla come “golden reference” per il parco, se quella nuova si comporta correttamente verso il server di stampa?
<span style="display:none">[^26_10][^26_11][^26_12][^26_13][^26_14][^26_15][^26_16][^26_17][^26_18][^26_9]</span>

<div align="center">⁂</div>

[^26_1]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/windows-security/sids-not-resolve-into-friendly-names

[^26_2]: https://www.officineinformaticheroma.it/37390/problemi-comuni-di-integrita-dei-dati-su-active-directory/windows10/recruitingservices71roma/2025/04/19/17/12/

[^26_3]: https://www.ictpower.it/sistemi-operativi/hardening-delle-azioni-amministrative-cosa-cambia-per-sid-duplicait-imaging-kerberos-ntlm-e-privilegi-admin.htm

[^26_4]: https://www.netsetup.it/windows-server/utenti-e-gruppi-predefiniti-in-active-directory

[^26_5]: https://www.reddit.com/r/msp/comments/1odcso1/shared_printer_issues_since_windows_11_upgrades/

[^26_6]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^26_7]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-ntlm-blocking

[^26_8]: https://learn.microsoft.com/it-it/windows-server/storage/file-server/smb-security-hardening

[^26_9]: https://learn.microsoft.com/it-it/previous-versions/system-center/system-center-2012-R2/hh230698(v=sc.12)

[^26_10]: https://www.reddit.com/r/sysadmin/comments/zz7fn4/windows_the_local_users_group_and_domain/

[^26_11]: https://it.letmeknow.ch/fixed-active-directory-domain-services-is-currently-unavailable-printer-error

[^26_12]: https://learn.microsoft.com/it-it/answers/questions/2187255/dominio-con-windows-server-2019-errore-sul-client

[^26_13]: https://groups.google.com/g/microsoft.public.it.winserver/c/JIhvof6LxBw

[^26_14]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/system-management-components/scenario-guide-troubleshoot-wmi-connectivity-access-issues

[^26_15]: https://www.ictpower.it/sistemi-operativi/funzionamento-delle-group-policy-in-windows-server-facciamo-un-po-di-chiarezza.htm

[^26_16]: https://support.microsoft.com/it-it/topic/l-aggiunta-guidata-stampante-elenca-lentamente-le-stampanti-pubblicate-in-servizi-di-dominio-active-directory-in-windows-vista-windows-server-2008-windows-7-o-windows-server-2008-r2-30974a31-6d5b-19a5-da3a-c03ad26edaa5

[^26_17]: https://www.youtube.com/watch?v=w5FagMjg2os

[^26_18]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/use-group-policy-to-control-ad-printer


---

# aspetta che prima chiedo wmic sulla stessa macchina con user di dominio

Perfetto, ha senso verificare il comportamento di `wmic` sulla stessa macchina con un utente di dominio: è un buon modo per capire se il server riesce o meno a vedere i gruppi di dominio e quindi a risolvere correttamente i SID dei gruppi (in particolare quello misterioso che vedi nella ACL della stampante).[^27_1][^27_2]

***

## Cosa ti conviene controllare con WMIC

Sul server `590-subint01`, loggato come utente di dominio con privilegi adeguati (idealmente un Domain Admin):

1. Elenco gruppi con nome e SID:

```cmd
wmic group get domain,name,sid
```

Questo ti mostra sia i gruppi locali (domain = nome del computer) sia quelli di dominio (domain = GASLINILAN, o simile).[^27_3][^27_2][^27_1]
2. Se hai copiato il SID lungo che compare nella ACL della stampante, puoi cercare se compare nell’output:
    - Se lo ritrovi associato a un gruppo di dominio (es. `Domain Users`, `Authenticated Users`, qualche gruppo applicativo), allora sai esattamente chi rappresenta.[^27_4][^27_5][^27_2]
    - Se non compare, è molto probabile che sia:
        - un oggetto AD eliminato, oppure
        - un SID di funzionalità/capability, che per design non viene risolto.[^27_6][^27_5][^27_4]
3. In alternativa, per un SID specifico:
    - con PowerShell sul server:

```powershell
Get-ADGroup -Identity 'S-1-5-…SID_COMPLETO…' 
```

(se hai RSAT/AD tools e il SID appartiene a un gruppo di dominio).[^27_2][^27_7]

***

## Perché è utile farlo con un utente di dominio

Come hai già notato:

- da un client buono, `wmic group` ti mostra anche i gruppi di dominio;
- dal server, come utente locale, vede solo i gruppi locali.

Loggandoti sul server come utente di dominio e rilanciando `wmic group get domain,name,sid`, puoi verificare se:

- il server riesce a interrogare AD e vedere tutti i gruppi di dominio (incluso quello del SID misterioso),[^27_1][^27_2]
- oppure se, per qualche ragione, dal server i gruppi di dominio non vengono enumerati correttamente.

Questo ti dirà se quel SID è:

- un gruppo di dominio ancora esistente, che puoi aggiungere in chiaro alle ACL, oppure
- un SID orfano/speciale, che non ha più un oggetto corrispondente e che quindi non porta effettivamente permessi ai client.

***

Quando hai fatto la prova con `wmic group get domain,name,sid` sul server loggato come utente di dominio, puoi riportarmi:

- se il SID lungo della ACL compare e con quale nome di gruppo,
oppure
- se non compare affatto tra i gruppi,

così possiamo decidere se sostituirlo esplicitamente con `Domain Users` / `Authenticated Users` o se trattarlo come SID orfano da eliminare.
<span style="display:none">[^27_10][^27_11][^27_12][^27_13][^27_14][^27_15][^27_8][^27_9]</span>

<div align="center">⁂</div>

[^27_1]: https://www.mcbsys.com/blog/2012/12/list-users-and-their-sids/

[^27_2]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-identifiers

[^27_3]: https://windowstech.net/wmic-commands/

[^27_4]: https://www.sentinelone.com/it/cybersecurity-101/cybersecurity/security-identifier/

[^27_5]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/windows-security/sids-not-resolve-into-friendly-names

[^27_6]: https://www.reddit.com/r/sysadmin/comments/4pabw0/why_am_i_getting_a_sid_those_long_s15211131234324/

[^27_7]: https://stackoverflow.com/questions/52118013/retrieve-users-of-a-group-by-group-sid

[^27_8]: https://okayamadaiti.github.io/blog/posts/20230208/

[^27_9]: https://learn.microsoft.com/en-us/archive/blogs/gaurav/security-identifiersid-getsid-of-a-userobject-using-registry-wmic-powershell

[^27_10]: https://www.shellspells.net/windows/system-ops/users-and-groups/enumerate

[^27_11]: https://www.dell.com/support/kbdoc/it-it/000135060/risoluzione-degli-errori-di-elaborazione-di-group-policy-in-un-dominio-active-directory

[^27_12]: https://it.ittrip.xyz/windows/troubleshooting/trust-workstation-dominio

[^27_13]: https://help.hotdocs.com/hotdocshub/onpremise/1.19.0/help/admin/Find_the_SID_of_an_Active_Directory_user.htm

[^27_14]: https://consumer.huawei.com/en/support/content/en-us15808748/

[^27_15]: https://www.manageengine.com/it/ad-recovery-manager/help/ts/ad-troubleshooting.html


---

# confermo, se fai wmic da user di dominio escono anche i gruppi di  dominio, se lo fai da administrator vedi solo quelli locali

S-1-5-21-2601942648-4138712042-247975646  e quest ae' la radice del SID di dominio

Quindi abbiamo chiarito un punto importante:

- Da **utente di dominio** sul server, `wmic` mostra anche i gruppi di dominio; da `Administrator` locale vedi solo gruppi locali.[^28_1][^28_2]
- Il SID di dominio ha radice `S-1-5-21-2601942648-4138712042-247975646`, che è l’identificatore del dominio GASLINILAN; tutti i SID di utenti/gruppi di dominio partono da quella radice + RID finale.[^28_2][^28_3]

Questo conferma che:

- il SID “misterioso” nella ACL della stampante è quasi certamente **un gruppo di dominio** (non un SID locale né un SID “random”),
- il fatto che la GUI delle proprietà stampante non lo risolva in nome può dipendere dal contesto con cui apri la finestra (es. Administrator locale vs utente di dominio) o da una leggera inconsistenza AD/ACL, ma non è “garbage”.[^28_4][^28_5][^28_2]

***

## Perché questo è rilevante per i client che non accedono

Hai quindi:

- ACL stampante che include un SID di gruppo di dominio (radice `S-1-5-21-2601942648-4138712042-247975646-…`) non risolto in nome leggibile.[^28_6][^28_2]
- Client “buoni”, i cui utenti di dominio risultano membri di un insieme di gruppi (incluso forse quello corrispondente a quel SID) e riescono a stampare ed accedere alle share del server.
- Client “rotti”, dove gli stessi utenti di dominio non vengono (per qualche motivo) riconosciuti come appartenenti a quel gruppo/ACL lato server, e quindi le credenziali sono rifiutate solo verso `590-subint01`.[^28_7][^28_8]

In pratica, se quel SID nella ACL rappresenta un gruppo di dominio X, e:

- alcuni utenti di dominio (su client buoni) sono membri di X,
- mentre altri (su client rotti) non lo sono (o non vengono proiettati correttamente nel token di sicurezza verso quel server),

puoi ottenere esattamente il comportamento che vedi: accesso selettivo alle share/stampanti di quel server.

***

## Direzione di analisi immediata

Ora che hai identificato la radice del SID di dominio, ti conviene:

1. Estrarre il SID **completo** che vedi nella ACL della stampante (quel S-1-5-… lungo).
2. Dal server o da un DC, con strumenti AD, verificare quale gruppo di dominio sia:
    - Esempio con PowerShell su un DC/server con RSAT AD:

```powershell
Get-ADGroup -Identity 'S-1-5-21-2601942648-4138712042-247975646-XXXXX'
```

dove `XXXXX` è il RID finale del SID della ACL.[^28_9][^28_2]
    - Oppure via `wmic group` filtrando sul SID:

```cmd
wmic group where sid="S-1-5-21-2601942648-4138712042-247975646-XXXXX" get name,domain,sid
``` [^28_1][^28_10]



3. Una volta noto il **nome del gruppo di dominio** (es. “StampantiGASLINI”, “UtentiDipartimentoX”, ecc.), puoi:
    - decidere se quell’ACL è coerente con chi deve stampare,
    - aggiungere esplicitamente `Domain Users` / `Authenticated Users` se vuoi che tutti i domini abbiano accesso,
    - e verificare se gli utenti che usi dai client “rotti” sono effettivamente membri di quel gruppo di dominio (via ADUC o `whoami /groups`).[^28_8][^28_11]

Se scopri che il SID è, ad esempio, un vecchio gruppo AD ormai non usato o con membri diversi da quelli che ti aspetti, è molto plausibile che:

- i client buoni stiano usando utenti che rientrano in gruppi con accesso,
- mentre i client rotti siano loggati con utenti che non rientrano, e quindi il server rifiuta le credenziali perché, lato ACL, non hanno diritti su share/stampante.

***

Ora che hai la radice del SID di dominio, il passo cruciale è identificare il gruppo esatto dietro quel SID lungo dell’ACL e verificare se gli utenti che falliscono ne fanno parte oppure no. Appena hai il SID completo della voce “misteriosa” nella scheda Sicurezza e il nome del gruppo AD corrispondente, possiamo ragionare se basta allineare membership/ACL o se serve cambiare completamente il modello di autorizzazione (ad esempio usando solo `Authenticated Users` + `Domain Users`).

<div align="center">⁂</div>

[^28_1]: https://www.mcbsys.com/blog/2012/12/list-users-and-their-sids/

[^28_2]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-identifiers

[^28_3]: https://www.sentinelone.com/it/cybersecurity-101/cybersecurity/security-identifier/

[^28_4]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/windows-security/sids-not-resolve-into-friendly-names

[^28_5]: https://www.reddit.com/r/sysadmin/comments/4pabw0/why_am_i_getting_a_sid_those_long_s15211131234324/

[^28_6]: image.jpg

[^28_7]: https://learn.microsoft.com/en-nz/answers/questions/5769825/dominio-microsoft-condivisione-stampante-e-cartell

[^28_8]: https://www.netsetup.it/windows-server/utenti-e-gruppi-predefiniti-in-active-directory

[^28_9]: https://stackoverflow.com/questions/52118013/retrieve-users-of-a-group-by-group-sid

[^28_10]: https://help.hotdocs.com/hotdocshub/onpremise/1.19.0/help/admin/Find_the_SID_of_an_Active_Directory_user.htm

[^28_11]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-groups


---

# ma e' possibile che 1087000422 si un RID?

Sì, è perfettamente possibile: in un SID di dominio del tipo
`S-1-5-21-2601942648-4138712042-247975646-1087000422`

la parte `2601942648-4138712042-247975646` è il **Domain ID** (96 bit) e l’ultimo numero `1087000422` è proprio il **RID** (Relative Identifier) del singolo oggetto (utente/gruppo/pc) dentro quel dominio.[^29_1][^29_2][^29_3][^29_4]

In generale:

- Tutti i SID di oggetti AD in un dominio condividono lo stesso “prefisso” `S-1-5-21-<32bit>-<32bit>-<32bit>`, che identifica il dominio.[^29_2][^29_5][^29_6]
- L’ultimo numero è il RID, assegnato dal RID Master:
    - RID < 1000 corrispondono ai gruppi builtin (Administrator, Domain Admins, ecc.).[^29_1][^29_2]
    - RID ≥ 1000 sono oggetti “definiti dall’admin” (utenti, gruppi di sicurezza, account macchina) creati nel tempo; non c’è limite superiore “pratico”, quindi valori grandi come 1087000422 sono possibili in domini molto longevi o con molte creazioni.[^29_7][^29_8][^29_4][^29_1]

Quindi, nel tuo SID:

- `S-1-5-21-2601942648-4138712042-247975646` = SID del dominio GASLINILAN.
- `1087000422` = RID assegnato a **quell’oggetto specifico** (verosimilmente un gruppo di dominio usato per la stampa).[^29_3][^29_9][^29_4][^29_6]

Non è “strano” che il RID sia un numero molto grande: significa solo che il dominio ha allocato molti RID nel corso della sua storia, o che c’è stata una certa attività nel pool RID.[^29_8][^29_10][^29_7]

Se ora prendi il SID completo della voce non risolta nella ACL stampante e lo cerchi con `wmic group where sid="..." get name,domain` o con `Get-ADGroup -Identity "..."`, dovresti ottenere il **nome del gruppo AD** corrispondente; da lì puoi verificare se gli utenti che falliscono ne fanno parte, e decidere se ha senso sostituirlo o affiancarlo con `Domain Users` / `Authenticated Users`.[^29_6][^29_11][^29_12][^29_13]
<span style="display:none">[^29_14][^29_15][^29_16][^29_17][^29_18][^29_19]</span>

<div align="center">⁂</div>

[^29_1]: https://office-skill.site/it/windows-Understanding_Windows_SIDs/

[^29_2]: https://en.wikipedia.org/wiki/Security_Identifier

[^29_3]: https://virot.eu/sids-and-rids-in-the-domain/

[^29_4]: https://www.tenfold-security.com/en/wiki/sid-security-identifier/

[^29_5]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^29_6]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-identifiers

[^29_7]: https://devblogs.microsoft.com/oldnewthing/20040315-00/?p=40253

[^29_8]: https://support.microsoft.com/it-it/topic/è-disponibile-un-aggiornamento-per-rilevare-e-impedire-un-consumo-eccessivo-del-pool-di-rid-globale-in-un-controller-di-dominio-909a36d2-340c-8058-192d-19ad09f1b9b1

[^29_9]: https://turbolab.it/windows-10/come-trovare-identificatore-protezione-sid-utente-windows-computer-2181

[^29_10]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/managing-rid-issuance

[^29_11]: https://www.mcbsys.com/blog/2012/12/list-users-and-their-sids/

[^29_12]: https://help.hotdocs.com/hotdocshub/onpremise/1.19.0/help/admin/Find_the_SID_of_an_Active_Directory_user.htm

[^29_13]: https://stackoverflow.com/questions/52118013/retrieve-users-of-a-group-by-group-sid

[^29_14]: https://www.reddit.com/r/activedirectory/comments/z1sgw0/how_is_the_sid_of_a_computerobject_linked_to_the/

[^29_15]: https://www.tenfold-security.com/wiki/sid-security-identifier/

[^29_16]: https://www.ninjaone.com/it/blog/come-trovare-il-sid-degli-utenti/

[^29_17]: https://www.raffaelechiatto.com/cambiare-sid-windows-server-2012-r2/

[^29_18]: https://ftpdocs.broadcom.com/cadocs/0/CA AppLogic 3 7-ITA/Bookshelf_Files/HTML/AppLogicDoc/wincfg_WWindows_Configuration_Utility_Windows_Server_2008_SID.html

[^29_19]: https://documentation.arcserve.com/Arcserve-UDP/Available/V6/ITA/Bookshelf_Files/HTML/Solutions%20Guide/caudp_trbleshootdomaindoesntexist.htm


---

# si, so cos'e' un RID, mi chiedevo se non avesse una limitazione numerica

Non c’è una “limitazione numerica bassa” per il RID: tecnicamente il campo è un intero a 32 bit, quindi può arrivare fino a $2^{32}-1 = 4\,294\,967\,295$, e valori grandi come 1087000422 sono perfettamente legali.[^30_1][^30_2][^30_3]

***

## Campo RID e limiti pratici

Nei SID di dominio, la struttura è:

```
- `S-1-5-21-<DomainID1>-<DomainID2>-<DomainID3>-<RID>`  
```

- Il **RID** è un `ULONG` (32 bit), quindi in teoria ogni dominio potrebbe avere fino a circa 4,3 miliardi di oggetti univoci (utenti, gruppi, account macchina, ecc.) prima di esaurire lo spazio dei RID.[^30_2][^30_3][^30_4][^30_1]

Microsoft, lato controller di dominio:

- gestisce un **pool globale di RID** e ne distribuisce blocchi ai DC;[^30_4][^30_5]
- monitora e protegge il consumo del pool per evitare esaurimenti o sprechi (es. bug che consumano RID in massa).[^30_5][^30_4]

Quindi:

- non c’è un “limite piccolo” tipo 1 milione o 16 bit: i RID possono arrivare tranquillamente nell’ordine di centinaia di milioni.[^30_1][^30_2]
- il tuo valore 1087000422 rientra tranquillamente nel range ammissibile; semmai è un segnale che quel dominio ha allocato già una quota non trascurabile di RID, ma non è di per sé un’anomalia.[^30_4][^30_5][^30_1]

In sintesi: sì, 1087000422 può essere un RID, e non viola nessun limite numerico del formato SID.

<div align="center">⁂</div>

[^30_1]: https://office-skill.site/it/windows-Understanding_Windows_SIDs/

[^30_2]: https://en.wikipedia.org/wiki/Security_Identifier

[^30_3]: https://www.tenfold-security.com/en/wiki/sid-security-identifier/

[^30_4]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/managing-rid-issuance

[^30_5]: https://support.microsoft.com/it-it/topic/è-disponibile-un-aggiornamento-per-rilevare-e-impedire-un-consumo-eccessivo-del-pool-di-rid-globale-in-un-controller-di-dominio-909a36d2-340c-8058-192d-19ad09f1b9b1


---

# CommandNotFoundException,  non riconosce GetADGroup

Il `CommandNotFoundException` su `Get-ADGroup` significa semplicemente che il modulo Active Directory non è caricato o non è installato sulla macchina da cui stai lanciando il comando; è normale su client/servizi che non hanno RSAT o le feature AD.[^31_1][^31_2][^31_3]

***

## Come risolvere (o aggirare) il problema

Hai due opzioni:

### 1. Usare PowerShell con modulo ActiveDirectory

Su una macchina dove hai RSAT / strumenti AD:

- Assicurati di avere il modulo installato (su un DC o su un server con RSAT):
    - In PowerShell:

```powershell
Import-Module ActiveDirectory
```

    - Se questo va a buon fine, puoi usare:

```powershell
Get-ADGroup -Identity "S-1-5-21-2601942648-4138712042-247975646-1087000422"
```

(oppure filtrare con `-Filter` o cercare per `-LDAPFilter` se preferisci).[^31_2][^31_4][^31_5]

Se `Import-Module ActiveDirectory` dà errore, vuol dire che la feature non è installata su quella macchina; in quel caso conviene farlo direttamente da un Domain Controller o da un server di gestione AD dove sai che gli strumenti sono presenti.[^31_4][^31_3]

### 2. Usare solo WMIC / strumenti standard (senza Get-ADGroup)

Se al momento non vuoi/no puoi installare il modulo AD, puoi continuare con:

```cmd
wmic group where sid="S-1-5-21-2601942648-4138712042-247975646-1087000422" get name,domain,sid
```

Se l’oggetto esiste ed è raggiungibile, `wmic` ti restituirà `name` e `domain`; se non restituisce nulla, quel SID non ha più un gruppo corrispondente (es. oggetto cancellato) o non è visibile nel contesto in cui lo esegui.[^31_6][^31_7][^31_8]

***

Quindi, per proseguire nella mappatura di quel SID:

- se hai già un DC o un server di gestione AD, esegui `Import-Module ActiveDirectory` lì e poi `Get-ADGroup` sul SID;
- in alternativa, usa direttamente `wmic group where sid=...` da un contesto in cui vedi i gruppi di dominio.

Appena riesci ad ottenere il `name` del gruppo AD corrispondente a quel SID, possiamo ragionare su membership utenti e su come allineare le ACL della stampante per includere esplicitamente i gruppi che ti interessano (es. `Domain Users`).
<span style="display:none">[^31_10][^31_11][^31_12][^31_13][^31_14][^31_15][^31_16][^31_17][^31_18][^31_9]</span>

<div align="center">⁂</div>

[^31_1]: https://www.reddit.com/r/PowerShell/comments/9hj9a4/getadgroupmember_command_fails_on_windows_10_yet/

[^31_2]: https://www.lepide.com/how-to/powershell-get-adgroup-to-list-active-directory-groups.html

[^31_3]: https://community.spiceworks.com/t/active-directory-groups-via-powershell/962253

[^31_4]: https://www.carlovicardi.it/2026/05/17/powershell-e-active-directory-gestione-gruppi/

[^31_5]: https://learn.microsoft.com/en-us/powershell/module/activedirectory/get-adgroupmember?view=windowsserver2025-ps

[^31_6]: https://www.mcbsys.com/blog/2012/12/list-users-and-their-sids/

[^31_7]: https://help.hotdocs.com/hotdocshub/onpremise/1.19.0/help/admin/Find_the_SID_of_an_Active_Directory_user.htm

[^31_8]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-identifiers

[^31_9]: https://www.reddit.com/r/PowerShell/comments/4w63ls/how_do_you_find_the_adgroup_with_the_most_members/

[^31_10]: https://www.reddit.com/r/PowerShell/comments/rcik3z/getadgroup_cannot_find_an_object_with_identity/

[^31_11]: https://netwrix.com/it/resources/guides/how-to-list-ad-group-members/

[^31_12]: https://stackoverflow.com/questions/44708772/why-wont-powershell-recognize-active-directory-group

[^31_13]: https://netwrix.com/it/resources/guides/how-to-get-ad-groups-for-user-accounts-powershell/

[^31_14]: https://stackoverflow.com/questions/13862881/powershell-get-adgroups-returns-wrong-result

[^31_15]: https://www.youtube.com/watch?v=e55iHOqTIDc

[^31_16]: https://forums.powershell.org/t/get-adgroupmember-showing-no-results/5089

[^31_17]: https://activedirectoryfaq.com/2016/09/ad-powershell-basics-adgroup-cmdlets/

[^31_18]: https://stackoverflow.com/questions/45365245/get-adgroup-with-recursive-is-not-working


---

# io non ho accesso agli ADC

Capito, se non hai accesso ai domain controller (né RSAT/ActiveDirectory sul server di stampa), ti mancano gli strumenti “diretti” (Get-ADGroup) per risolvere quel SID dal punto di vista di AD.[^32_1][^32_2][^32_3]

In questa situazione, sul piano strettamente tecnico, restano due strumenti a basso livello:

- `wmic group get domain,name,sid` eseguito sul server o su un client con visibilità di dominio, che **già ti mostra** i gruppi di dominio con i loro SID;[^32_4][^32_5][^32_6]
- eventuali strumenti AD che il tuo team di gestione dominio può usare dal lato DC per dirti “quel SID è il gruppo X”.

Dal punto di vista operativo, il limite di accesso che hai è più organizzativo che tecnico: **senza poter interrogare AD dai DC**, la mappatura precisa SID→nome gruppo va fatta da chi ha quegli privilegi.

***

## Cosa puoi ancora fare lato server/clients senza accedere ai DC

Anche senza Get-ADGroup, ci sono ancora alcune verifiche che non richiedono accesso diretto al DC:

1. Verificare su un **client buono** (dove la stampante funziona) le appartenenze ai gruppi dell’utente che stampa:
    - `whoami /groups`
    - oppure `net user nomeutente /domain` (che elenca i gruppi globali/di dominio).[^32_7][^32_8]

Se vedi un gruppo con SID che coincide con quello della ACL, sai che gli utenti “buoni” ne fanno parte.
2. Verificare sul **client rotto** le appartenenze ai gruppi dello stesso utente di dominio:
    - `whoami /groups`

Se *non* compare lo stesso gruppo/SID, questo spiega perché il server rifiuta credenziali da quei client: il token inviato verso la share/stampante non contiene il gruppo richiesto dalla ACL.[^32_9][^32_10][^32_11]
3. Lato server di stampa, indipendentemente da quel SID, puoi comunque:
    - aggiungere esplicitamente `Domain Users` o `Authenticated Users` alla ACL delle stampanti e della share `demo` con permessi di stampa/lettura;[^32_11][^32_12][^32_13]
    - verificare se **dopo** questa aggiunta, i client rotti continuano a chiedere/rifiutare credenziali.

Se anche con `Domain Users`/`Authenticated Users` in ACL, e con trust/dns già verificati, i client rotti continuano a fallire, la conclusione pratica è che il problema sta più nel loro stato locale (profilo, SID macchina, eventuale hardening/security software) che nel singolo gruppo di dominio; e in questi contesti, come molte linee guida su SID/hardening suggeriscono, spesso la soluzione più pulita è rigenerare almeno una macchina da zero e verificare se il problema scompare.[^32_14][^32_15][^32_16][^32_17]

***

Visto che hai già fatto una notevole analisi e sei molto vicino al “limite” di ciò che puoi fare senza accesso ai DC, personalmente ti suggerirei:

- in parallelo, segnalare al team AD il SID completo e chiedere cosa rappresenta,
- e sul piano operativo su workstation, rigenerare almeno uno dei tre client che non accedono per vedere se un’installazione pulita di 25H2 + join a dominio elimina il problema verso `590-subint01`.

Se la nuova macchina entra subito nel dominio e accede alla share e alle stampanti del server senza problemi, avrai una conferma forte che il problema è legato allo stato storico/locale dei tre client (SID, imaging, security hardening), più che alle impostazioni attuali di ACL/server.
<span style="display:none">[^32_18][^32_19][^32_20][^32_21][^32_22][^32_23][^32_24][^32_25][^32_26][^32_27]</span>

<div align="center">⁂</div>

[^32_1]: https://netwrix.com/it/resources/guides/how-to-get-ad-user-group-membership/

[^32_2]: https://www.lepide.com/how-to/powershell-get-adgroup-to-list-active-directory-groups.html

[^32_3]: https://community.spiceworks.com/t/active-directory-groups-via-powershell/962253

[^32_4]: https://www.mcbsys.com/blog/2012/12/list-users-and-their-sids/

[^32_5]: https://help.hotdocs.com/hotdocshub/onpremise/1.19.0/help/admin/Find_the_SID_of_an_Active_Directory_user.htm

[^32_6]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-identifiers

[^32_7]: https://www.youtube.com/watch?v=B04J-H8odhs

[^32_8]: https://netwrix.com/it/resources/guides/how-to-check-user-groups-in-windows-command-line/

[^32_9]: https://learn.microsoft.com/it-it/windows/win32/ad/security-properties

[^32_10]: https://learn.microsoft.com/it-it/windows/win32/secmgmt/translating-between-names-and-sids

[^32_11]: https://www.netsetup.it/windows-server/utenti-e-gruppi-predefiniti-in-active-directory

[^32_12]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-groups

[^32_13]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/printing/use-group-policy-to-control-ad-printer

[^32_14]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^32_15]: https://www.officineinformaticheroma.it/37390/problemi-comuni-di-integrita-dei-dati-su-active-directory/windows10/recruitingservices71roma/2025/04/19/17/12/

[^32_16]: https://www.ictpower.it/sistemi-operativi/hardening-delle-azioni-amministrative-cosa-cambia-per-sid-duplicait-imaging-kerberos-ntlm-e-privilegi-admin.htm

[^32_17]: https://www.raffaelechiatto.com/cambiare-sid-windows-server-2012-r2/

[^32_18]: https://stackoverflow.com/questions/63636600/powershell-not-able-to-find-an-ad-group-by-name

[^32_19]: https://stackoverflow.com/questions/73445098/get-user-group-memberships-from-sid

[^32_20]: https://www.reddit.com/r/PowerShell/comments/1rjvrgs/im_stuck_on_this_command_newadgroup/

[^32_21]: https://giovannilubrano.blogspot.com/2022/01/powershell-identificare-il-gruppo.html

[^32_22]: https://stackoverflow.com/questions/44708772/why-wont-powershell-recognize-active-directory-group

[^32_23]: https://learn.microsoft.com/en-us/answers/questions/1534708/get-azadgroupmember-not-working

[^32_24]: https://stackoverflow.com/questions/78674478/powershell-7-4-2-get-adgroupmember-get-aduser-not-working

[^32_25]: https://www.reddit.com/r/PowerShell/comments/ky1tpf/getadgroup_help/

[^32_26]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/active-directory/get-adgroupmember-error-remote-forest-members

[^32_27]: https://netwrix.com/it/resources/blog/how-to-add-and-remove-ad-groups-and-objects-in-groups-with-powershell/


---

```
# <q>wmic group where sid="S-1-5-21-2601942648-4138712042-247975646-1087000422" get name,domain,sid</q> parametro non previsto a questo livello
```

Quel messaggio “parametro non previsto a questo livello” su `wmic group where sid="..." get name,domain,sid` di solito indica che la sintassi va leggermente adattata: con `wmic` bisogna specificare la *classe* corretta e la `where` in forma di stringa, non sempre funziona la forma abbreviata.[^33_1][^33_2][^33_3]

***

## Sintassi corretta per WMIC con SID di gruppo

Per i gruppi, la classe WMI è `Win32_Group`. La forma più robusta è:

```cmd
wmic path Win32_Group where "SID='S-1-5-21-2601942648-4138712042-247975646-1087000422'" get Name,Domain,SID
```

Note:

- `path Win32_Group` esplicita la classe, evitando ambiguità.[^33_4][^33_3][^33_1]
- La condizione `where` è una stringa tra virgolette doppie, all’interno della quale il SID è tra apici singoli.
- In italiano, l’errore “parametro non previsto a questo livello” compare spesso quando la `where` non è tra virgolette o quando la classe predefinita non supporta il filtro che stai usando.

Se il SID corrisponde a un gruppo di dominio visibile nel contesto in cui esegui il comando, ti dovrebbe tornare qualcosa tipo:

```text
Domain  Name             SID
GASLINILAN NomeGruppoX   S-1-5-21-2601942648-4138712042-247975646-1087000422
```

Se invece non ritorna nulla:

- o quel SID non ha più un oggetto gruppo corrispondente (oggetto AD eliminato),[^33_5][^33_6]
- o, nel contesto in cui esegui, quella classe non riesce a interrogare il dominio (es: problemi di permessi/rete).

***

Se ti va, prova quella forma con `wmic path Win32_Group` e dimmi se ritorna il nome del gruppo oppure nessun risultato; da lì possiamo capire se stiamo parlando di un gruppo AD esistente o di un SID orfano.
<span style="display:none">[^33_10][^33_11][^33_12][^33_7][^33_8][^33_9]</span>

<div align="center">⁂</div>

[^33_1]: https://stackoverflow.com/questions/53755993/wmic-useraccount-list-full-and-nothing-happen

[^33_2]: https://www.shellhacks.com/windows-get-user-sid-cmd-powershell/

[^33_3]: https://www.vanetta.net/2011/05/windows-security-regarding-well-known.html?m=1

[^33_4]: https://gist.github.com/DanielRTeixeira/d80f404c627c057b520ac4e87314bcf4

[^33_5]: https://www.reddit.com/r/sysadmin/comments/4pabw0/why_am_i_getting_a_sid_those_long_s15211131234324/

[^33_6]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/windows-security/sids-not-resolve-into-friendly-names

[^33_7]: https://www.tenforums.com/tutorials/84467-find-security-identifier-sid-user-windows.html

[^33_8]: https://www.mcseboard.de/topic/221249-powershell-unerwartetes-ergebnis/

[^33_9]: https://www.ibm.com/support/pages/domain-group-names-do-not-return-creds-output-windows-7-and-windows-2008-r2

[^33_10]: https://forums.powershell.org/t/domain-sid-not-resolving-on-workgroup-pc/21348

[^33_11]: https://learn.microsoft.com/en-us/archive/msdn-technet-forums/c1af7f1c-2a4e-436c-8d81-9f6e185ee981

[^33_12]: https://stackoverflow.com/questions/40098648/wmic-useraccount-sid-blank-spaces-empty-lines/40103052


---

# sempre parametro non previsto a questo livello

Con `wmic` l’errore “parametro non previsto a questo livello” è quasi sempre sintassi: `wmic` è molto rigido su spazi, virgolette e ordine degli elementi.[^34_1][^34_2]

Prova una di queste varianti, molto “letterali”:

```cmd
wmic path Win32_Group where "SID = 'S-1-5-21-2601942648-4138712042-247975646-1087000422'" get Name,Domain,SID
```

oppure, spezzando in due step:

```cmd
wmic path Win32_Group get Name,Domain,SID | findstr 2601942648-4138712042-247975646-1087000422
```

Note pratiche:

- Nella `where` devono esserci **virgolette doppie** attorno all’intera condizione, e **apici singoli** attorno al SID; lo spazio prima e dopo `=` spesso fa la differenza.[^34_3][^34_1]
- Se continui a ricevere “parametro non previsto”, è più semplice usare il secondo comando con `findstr`, che non dipende da `where` e ti permette comunque di individuare la riga con quel SID nell’elenco dei gruppi.[^34_4][^34_3]

Se anche con il secondo comando non appare nessuna riga con quel SID, vuol dire che nel contesto di quella macchina non esiste più un gruppo WMI corrispondente (oggetto AD non risolto o eliminato); a quel punto, l’unica strada per sapere cosa fosse davvero è chiedere al team AD di cercare quel SID nel dominio.

<div align="center">⁂</div>

[^34_1]: https://stackoverflow.com/questions/53755993/wmic-useraccount-list-full-and-nothing-happen

[^34_2]: https://gist.github.com/DanielRTeixeira/d80f404c627c057b520ac4e87314bcf4

[^34_3]: https://www.shellhacks.com/windows-get-user-sid-cmd-powershell/

[^34_4]: https://stackoverflow.com/questions/40098648/wmic-useraccount-sid-blank-spaces-empty-lines/40103052


---

# non vi sono istanze disponbili

“Non vi sono istanze disponibili” da `wmic path Win32_Group …` significa che, nel contesto in cui stai interrogando, **non esiste alcun oggetto gruppo con quel SID** (o comunque WMI non riesce a vederlo).[^35_1][^35_2]

In pratica:

- Il SID `S-1-5-21-2601942648-4138712042-247975646-1087000422` compare nelle ACL della stampante, ma WMI non trova nessuna istanza `Win32_Group` con quell’identificatore.
- Questo è coerente con i casi in cui la ACL contiene il SID di un **gruppo di dominio eliminato** o non più risolvibile; la GUI continua a mostrare il SID, ma il gruppo non esiste più, quindi non porta permessi reali agli utenti.[^35_3][^35_4][^35_5]

***

# Conclusione delle prove inutili

Se quel SID nella ACL è orfano:

- Gli utenti di dominio che arrivano dai client “buoni” ottengono accesso probabilmente grazie ad altre voci (es. `Everyone`, `Authenticated Users`, `Domain Users` che hai aggiunto).
- I client “rotti” invece continuano a fallire, il che indica che per loro **anche le voci sane della ACL non stanno funzionando correttamente** (nonostante l’aggiunta di `Authenticated Users`), e che il SID orfano non è l’unico fattore.

Dato che:

- il SID non corrisponde più a un gruppo visibile via WMI,
- l’hai già affiancato con `Authenticated Users` / `Domain Users` in ACL senza miglioramenti,
- i client rotti funzionano verso altri server di dominio ma non verso questo,

siamo in un punto in cui:

- il SID orfano è “sospetto”, ma non sembra essere da solo la causa del rifiuto credenziali,
- il problema complessivo è probabilmente legato allo **stato locale** di quei client (storico di imaging/SID macchina, security hardening, software di sicurezza) che si manifesta solo verso il server reinstallato.

***

Vista la quantità di test già fatti e il fatto che ora sappiamo che il SID ACL non ha più un oggetto corrispondente visibile, la mossa più efficace per isolare la variabile “client” rimane:

- rigenerare almeno **uno dei tre client** con un’installazione pulita di Windows 11 25H2, join a dominio standard,
- verificare se quella macchina nuova accede **subito** alle share e alle stampanti di `590-subint01` con gli stessi utenti di dominio.

Se il nuovo client funziona:

- hai la conferma che il problema è legato allo stato storico delle tre macchine (SID macchina, profilo, eventuali residui di policy/AV) e non alla configurazione attuale del server, né alle ACL che hai già allineato.
- puoi usare il nuovo client come riferimento per capire quali differenze di configurazione/registri esistono rispetto ai tre “vecchi” e decidere se conviene migrare il parco alla nuova baseline.

Se preferisci, possiamo ancora provare a fare un paio di verifiche mirate sui client (es. `whoami /groups` su client buono vs client rotto per lo stesso utente), ma considerato dove siamo arrivati, la reinstallazione di almeno una macchina è l’unica prova davvero decisiva che non richiede accesso ai DC.


# Analisi della clonazione

## la reinstallazione di una macchina cleint e' stata gia' testata in un' occasion precedente e non ha cambiato nulla. Forse puo' essere d'aiuto sapere che lemacchine vengono create per clonazione di un' immagine clonezilla, fuori dominio, e successivamente unite a dominio per sanare il SID

Questa informazione è molto rilevante: i client vengono creati clonando un’immagine Clonezilla **fuori dominio** e poi vengono uniti al dominio; inoltre hai già provato a reinstallare una macchina client e non è cambiato nulla.

Dal punto di vista Microsoft, la clonazione senza Sysprep, anche fuori dominio, può comunque generare **SID macchina duplicati**, e con 24H2/25H2/Server 2025 gli aggiornamenti degli ultimi anni hanno introdotto un hardening esplicito: Kerberos/NTLM può rifiutare l’autenticazione tra macchine che condividono la stessa machine SID, e uno degli scenari di rottura comuni è proprio SMB (file share e stampanti).[^36_1][^36_2][^36_3][^36_4][^36_5][^36_6]

***

## Clonazione senza Sysprep e problemi SMB/NTLM

Le note tecniche e Q\&A recenti riportano in modo abbastanza chiaro:

- Se si clonano installazioni di Windows (anche fuori dominio) senza passare da `sysprep /generalize`, le macchine risultanti condividono lo stesso **machine SID**.[^36_2][^36_3][^36_7][^36_1]
- Su Windows 11 24H2/25H2, dopo specifici aggiornamenti (es. KB5064081, KB5065426), Windows applica controlli più rigidi sui SID, con effetti su Kerberos/NTLM e quindi su SMB, RDP, ecc.[^36_8][^36_4][^36_5][^36_9]
- In queste condizioni, è possibile che:
    - alcuni pair di macchine continuino a funzionare,
    - altri (come il tuo trio di client verso il server reinstallato) rifiutino l’autenticazione SMB nonostante le credenziali siano corrette,
    - e che questo si manifesti solo fra macchine che condividono SID duplicati o lotti specifici di SID.[^36_10][^36_4][^36_11][^36_5][^36_8]

In altre parole: il tuo workflow Clonezilla “fuori dominio e poi join” previene conflitti di **computer account** in AD, ma non garantisce l’unicità del **machine SID**; e con gli ultimi patch, Windows ha iniziato a trattare i duplicati come condizioni di errore in autenticazione NTLM/Kerberos tra certi nodi.[^36_3][^36_4][^36_5][^36_2]

***

## Perché la reinstallazione di un solo client può non bastare

Se la “reinstallazione” che hai testato era:

- reinstallare Windows sul client ma poi re‑clonare nuovamente da una stessa immagine non sysprepata,
oppure
- reinstallare senza usare Sysprep /generalize prima di catturare la nuova immagine,

allora è possibile che il nuovo client abbia comunque ricevuto lo **stesso machine SID** degli altri nel gruppo “rotto”, e quindi non risolva il problema di autenticazione verso il server.[^36_5][^36_1][^36_2][^36_3]

Le linee guida ufficiali e i casi recenti di “SMB Sharing Fails on Windows 11 25H2” dicono esplicitamente:[^36_4][^36_9][^36_10][^36_8][^36_5]

- controllare se le macchine affette sono state imaged senza Sysprep;
- se sì, confermare i SID duplicati (es. confrontando il SID dell’Administrator locale senza il `-500` finale);
- la **soluzione strutturale** è rigenerare i SID tramite Sysprep /generalize sulla gold image, quindi ridistribuire e rejoinare; non c’è un “SID changer” supportato in‑place per 24H2/25H2.

***

## Test mirato che puoi fare senza DC

Un test semplice suggerito nelle note tecniche:[^36_4][^36_5]

Su due client sospetti, in PowerShell:

```powershell
(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$',''
```

Se il valore risultante è **identico** su due macchine diverse, condividono il machine SID.[^36_5][^36_4]

Puoi farlo:

- su uno dei client “rotti”,
- su uno dei client “buoni”,
- e su altri, per vedere se il gruppo di tre che non riescono a parlare col server appartiene a un “lotto” di macchine con SID duplicato fra loro o con il server.

Se scopri che i tre client “rotti” condividono la stessa machine SID (e magari questo SID coincide con quello del server precedente o con qualche nodo chiave), è una spiegazione molto naturale del perché:

- verso il nuovo server (con SID diverso) l’autenticazione SMB viene controllata e rifiutata,
- verso altri host (non ancora aggiornati alla stessa enforcement o con relazioni diverse) funziona.

***

## Cosa implica per il tuo workflow Clonezilla

La pratica attuale:

- clonare fuori dominio con Clonezilla,
- poi unire le macchine a dominio,

funziona da anni finché Windows non applica enforcement sui machine SID; ma con 24H2/25H2 e gli update recenti, le best practice aggiornate sono:[^36_7][^36_1][^36_2][^36_3]

- eseguire **Sysprep /generalize** sulla macchina di riferimento *prima* di catturare l’immagine,
- usare quell’immagine sysprepata per clonare,
- alla prima accensione ogni clone genera un SID univoco e solo **poi** lo si unisce al dominio,
- evitare immagini non sysprepate per macchine destinate a essere in dominio.

Per i sistemi già distribuiti, i documenti su “Kerberos and NTLM authentication failures due to duplicate SIDs” suggeriscono:[^36_9][^36_4][^36_5]

- confermare quali macchine condividono SID,
- rigenerarle con Sysprep /generalize o re‑imaging supportato,
- come mitigazione temporanea, Microsoft fornisce a volte GPO speciali per rilassare il check, ma è solo per mantenere l’operatività mentre si ricostruiscono le immagini.

***

Dato tutto questo, il fatto che tu veda problemi di SMB/stampa molto selettivi su un gruppo di 3 client e non su altri è altamente compatibile con conflitti di machine SID legati al vostro processo Clonezilla. Il passo successivo, se vuoi restare in analisi tecnica, è:

- eseguire il test del machine SID su almeno due client “rotti” e uno “buono”,
- vedere se i tre problematici condividono un SID comune,
- e se sì, sapere che la vera soluzione robusta è rigenerare l’immagine con Sysprep /generalize e ridistribuire i client, più che continuare a intervenire su singole chiavi SMB/ACL.

Ti torna se ti metti a vedere se i SID macchina dei tre client sono effettivamente identici fra loro (e/o diversi da quelli dei client buoni)?
<span style="display:none">[^36_12][^36_13][^36_14][^36_15]</span>

# SID del server

Dal comando sul server:

```powershell
(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''
```

ottieni qualcosa del tipo:

`S-1-5-21-3753021204-3844753684-2580916038`[^37_1]

Quello è il **machine SID** del server `590-subint01`: tutti gli account locali su quella macchina (Administrator, Users, ecc.) condividono questo prefisso, con RID diversi alla fine.[^37_2][^37_3]

***

## Come usare questo dato

Ora hai il SID macchina del server; il passo successivo è:

- eseguire lo stesso comando su almeno:
    - uno dei **client buoni** (che accedono alle share/stampanti del server),
    - uno dei **client rotti** (che rifiutano credenziali verso il server),

e confrontare:

- Se il machine SID dei client rotti è **uguale** a quello del server o uguale tra loro, mentre i client buoni hanno SID diversi, hai una prova forte di SID duplicati che interagiscono col nuovo hardening (Kerberos/NTLM/SMB) e spiegano il comportamento selettivo.[^37_4][^37_5][^37_6][^37_2]
- Se invece tutti i SID macchina (server, client buoni, client rotti) sono diversi, allora possiamo scartare l’ipotesi “SID duplicato” e tornare a cercare una causa più sottile (policy locali, endpoint security, ecc.).[^37_6][^37_7]

Quindi questo valore del server ti serve come “baseline”: confrontalo ora con il risultato dello stesso comando sui tre client problematici e su almeno un client buono.


# Client KO, stesso SID del server

Sul client “rotto”, lo stesso comando:

```powershell
(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''
```

ti dà `S-1-5-21-3753021204-3844753684-2580916038`, cioè **esattamente lo stesso machine SID del server**.[^38_1][^38_2]

Questa è una scoperta cruciale: server e client condividono il **medesimo SID macchina**, esattamente lo scenario che la documentazione su Windows 11 24H2/25H2 segnala come causa di **fallimenti di autenticazione Kerberos/NTLM su SMB** dopo imaging/clonazione senza Sysprep.[^38_3][^38_4][^38_5][^38_6][^38_7]

***

## Implicazioni dirette per il tuo problema

In sintesi:

- Il server `590-subint01` e almeno un client “rotto” hanno lo stesso machine SID.[^38_2][^38_1]
- Hai confermato che le macchine vengono create da Clonezilla a partire da immagini non sysprepate, fuori dominio, poi unite al dominio.[^38_4][^38_8][^38_9]
- I problemi di SMB/stampa compaiono solo tra un sottoinsieme di client e questo server reinstallato, mentre altre combinazioni funzionano.

Questo corrisponde perfettamente ai casi descritti come:

> “SMB authentication failure between domain-joined machines after deploying multiple Windows 11 24H2/25H2 devices using a cloned image with duplicate machine SIDs”.[^38_5][^38_6][^38_7]

Gli effetti tipici includono:

- prompt di credenziali ripetuto e rifiuto anche di credenziali corrette verso certe share,
- problemi selettivi tra host con SID duplicati,
- funzionamento apparentemente normale verso altri host non coinvolti.[^38_6][^38_7][^38_10][^38_5]

***

## Cosa significa in pratica e cosa conviene fare

Dal punto di vista sistemistico:

- Il tuo parco ha almeno **una coppia server–client con SID duplicato**, e molto probabilmente più client condividono tra loro lo stesso SID (derivando dalla stessa immagine Clonezilla).[^38_7][^38_8][^38_9][^38_3]
- Gli aggiornamenti recenti di Windows 11 (24H2/25H2) hanno iniziato a considerare questo scenario come **non supportato** e a bloccare o rendere inaffidabile l’autenticazione SMB/NTLM tra tali macchine.[^38_11][^38_6][^38_7]

Le linee guida ufficiali, in casi come questo, sono molto nette:[^38_3][^38_4][^38_6][^38_7][^38_11]

- Non è supportato l’uso di “SID changers” o workaround locali;
- La soluzione strutturale è **ricostruire l’immagine** con `sysprep /generalize` sulla macchina di riferimento, quindi ricreare server e client da immagini sysprepate, in modo che ogni macchina abbia un machine SID univoco;
- Nel frattempo, esistono talvolta GPO temporanee per mitigare il blocco, ma sono pensate come ponte verso una ricostruzione corretta, non come soluzione definitiva.

***

In altre parole: hai individuato il root cause di fondo. Nonostante tutta la diagonale su ACL, NTLM, RPC, GPO, la ragione per cui quei client rifiutano credenziali verso il server è che **server e client hanno lo stesso SID macchina**, condizione che Windows 11 25H2, con gli ultimi update, non tollera più per autenticazioni SMB affidabili.[^38_5][^38_6][^38_7][^38_11]

Per proseguire in modo sano, il prossimo passo reale non è tanto inseguire ulteriori tweak SMB, ma:

- pianificare la rigenerazione di almeno il server di stampa e i client “rotti” da immagini sysprepate,
- aggiornare il workflow Clonezilla in modo che la gold image venga preparata con `sysprep /generalize` prima del capture,
- dopo di che, rifare join a dominio e verificare che SMB/stampa funziona senza più rifiuti di credenziali.

Ti torna se ora metti a confronto anche gli altri due client “rotti” rispetto al server per vedere se condividono lo stesso machine SID, così quantifichi il problema, e poi porti all’IT la proposta di aggiornare il processo di imaging con Sysprep?

---

# Client OK SID diverso

Sul client “buono”, lo stesso comando:

```powershell
(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''
```

restituisce un SID macchina diverso: `S-1-5-21-2589957146-4074959585-2131959901`, non quello del server.[^39_1]

Quindi la situazione è:

- Server `590-subint01`: machine SID `S-1-5-21-3753021204-3844753684-2580916038`.[^39_2]
- Client rotto (almeno quello che hai mostrato): stesso machine SID del server `S-1-5-21-3753021204-3844753684-2580916038`.[^39_3]
- Client buono: machine SID diverso `S-1-5-21-2589957146-4074959585-2131959901`.[^39_1]

Questo è esattamente il pattern “server + alcuni client clonati condividono SID macchina; altri client hanno SID unici” che le note su Windows 11 24H2/25H2 collegano a problemi di autenticazione SMB/Kerberos/NTLM selettivi tra quelle macchine.[^39_4][^39_5][^39_6][^39_7][^39_8]

***

## Sintesi del quadro, con i tuoi dati

- I client buoni hanno SID macchina **diversi** dal server e tra loro.[^39_8][^39_1]
- Almeno un client rotto ha lo **stesso SID macchina** del server, derivando chiaramente dalla stessa immagine clonezza senza Sysprep.[^39_5][^39_9][^39_2][^39_3][^39_4]
- Su Windows 11 24H2/25H2, con gli ultimi update, questa configurazione è dichiarata non supportata e può causare “SMB authentication failure between domain-joined machines” proprio come stai vedendo.[^39_6][^39_7][^39_10][^39_8]

Questo dà una spiegazione tecnica coerente del perché:

- gli utenti di dominio, dai client clonati con SID uguale al server, vedono credenziali rifiutate verso le share/stampanti di `590-subint01`,
- mentre dagli altri client (SID macchina diverso) le stesse credenziali funzionano.

***

A questo punto, direi che hai individuato con buona certezza la causa primaria:

- non è più ACL, NTLM, RPC o GPO in sé,
- ma il **SID macchina duplicato** tra server e alcuni client, frutto del processo di imaging Clonezilla senza Sysprep, che con 25H2 inizia a rompersi proprio su SMB/stampa.

Per sanare davvero la situazione sarà necessario:

- ricreare il server e i client “rotti” da una immagine preparata correttamente con `sysprep /generalize` (o installazione pulita),
- aggiornare il workflow Clonezilla in modo che la gold image aziendale venga sysprepata prima del capture, così ogni nuova macchina ottiene un SID macchina univoco.

Ti è utile se ti metto nero su bianco una mini “nota tecnica” da poter riportare internamente (hardening SID 24H2/25H2 + effetti su SMB, più best practice Sysprep), oppure preferisci chiudere qui la diagnosi e passare direttamente al piano operativo?


# nota tecnica sintetica su SID duplicati e SMB 25H2

Ti metto una nota tecnica sintetica, con riferimenti ai documenti Microsoft, che puoi riutilizzare internamente.

***

## SID duplicati e SMB su Windows 11 24H2 / 25H2

### Contesto tecnico

- Ogni macchina Windows ha un **machine SID** (`S-1-5-21-…`), derivato dalla SID dell’utente `Administrator` senza il suffisso `-500`.[^40_1][^40_2][^40_3]
- Questo SID viene usato da Kerberos/NTLM per identificare in modo univoco il computer nelle relazioni di trust, nell’accesso alle risorse (SMB, RDP, ecc.) e nei log di sicurezza.[^40_4][^40_5]

Con Windows 11 24H2 / 25H2 e Windows Server 2025, a partire dagli aggiornamenti di fine agosto–settembre 2025 (KB5064081, KB5065426), Microsoft ha introdotto **controlli di sicurezza aggiuntivi** che:

- rilevano i casi in cui **più dispositivi hanno lo stesso machine SID**,
- bloccano gli handshake di autenticazione Kerberos e NTLM tra tali dispositivi,
- causando errori di autenticazione su SMB, accesso a share, stampanti di rete, login interattivo o RDP.[^40_6][^40_7][^40_8][^40_9][^40_10]

Microsoft documenta esplicitamente che:

> “Potresti riscontrare errori di autenticazione Kerberos e NTLM tra dispositivi con SID duplicati […] dopo gli aggiornamenti rilasciati il 29 agosto 2025 e successivi. Gli aggiornamenti includono protezioni che applicano i controlli sui SID, causando errori di autenticazione quando i dispositivi hanno SID duplicati. Questa modifica blocca gli handshake di autenticazione tra tali dispositivi.”[^40_7][^40_10][^40_6]

Gli eventi correlati compaiono come ID evento 6167 del servizio LSA (`lsasrv.dll`) nel registro Sistema, e l’analisi porta spesso a scenari di clonazione/disk imaging senza Sysprep.[^40_11][^40_8][^40_10][^40_6]

***

### Origine dei SID duplicati

SID duplicati nascono tipicamente quando:

- si clonano installazioni di Windows (Clonezilla, Acronis, ecc.) **senza** eseguire `sysprep /generalize` sulla macchina di riferimento prima del capture;[^40_12][^40_2][^40_13][^40_14][^40_15]
- si distribuiscono più macchine in dominio a partire dalla stessa immagine “non generalizzata”, creando dispositivi con lo stesso machine SID.[^40_8][^40_2][^40_13][^40_14]

Fino a Windows 10 / Windows 11 pre‑24H2, questo scenario era “tecnicamente tollerato” (pur non supportato) e spesso non dava problemi evidenti; con 24H2/25H2 + patch 2025, non è più accettato:

> “Con Windows 11 24H2 e 25H2, Microsoft ha deciso di porre fine all’uso di SID duplicati. Un PC clonato senza rigenerazione dei SID non può più partecipare a un dominio, autenticarsi su risorse di rete, stabilire sessioni remote sicure.”[^40_16][^40_17][^40_18][^40_9]

***

### Effetto pratico su SMB e stampa

Su macchine con SID duplicati, dopo gli aggiornamenti di sicurezza:

- le richieste di autenticazione NTLM/Kerberos tra quei dispositivi possono essere rifiutate, anche con credenziali corrette;[^40_10][^40_6][^40_7][^40_8]
- questo produce sintomi come:
    - prompt di credenziali in loop verso alcune share SMB,
    - impossibilità di accedere a cartelle condivise su server specifici,
    - errori nella connessione a stampanti di rete (share di stampa) tra host che condividono SID.[^40_19][^40_20][^40_21][^40_12]

Nel tuo caso concreto hai verificato che:

- il server `590-subint01` e almeno un client “rotto” condividono lo stesso machine SID (`S-1-5-21-3753021204-3844753684-2580916038`),
- i client “buoni” hanno SID macchina diversi,
- i problemi SMB/stampa compaiono solo tra macchine con SID uguale al server, mentre verso altri host di dominio le stesse credenziali funzionano.

Questo è esattamente lo scenario descritto dai documenti Microsoft come “errori di autenticazione Kerberos/NTLM dovuti a SID duplicati” su Windows 11 24H2/25H2, con impatto diretto sulla condivisione file/stampanti.[^40_20][^40_22][^40_6][^40_8][^40_10]

***

### Mitigazione e soluzione raccomandata

Microsoft raccomanda:[^40_17][^40_18][^40_2][^40_13][^40_22][^40_6][^40_16][^40_12][^40_10]

1. **Identificare i SID duplicati**
    - Usare comandi come
$(Get-LocalUser -Name 'Administrator').Sid.Value -replace '-500$', ''$
e confrontare tra macchine sospette.[^40_22][^40_10]
2. **Soluzione strutturale**
    - Ricostruire le macchine con SID duplicati (server e client) usando metodi di clonazione **supportati**, cioè:
        - preparare una gold image con `sysprep /generalize /oobe /shutdown`,[^40_18][^40_2][^40_13][^40_16][^40_17][^40_12]
        - catturare l’immagine dopo Sysprep,
        - distribuire nuove macchine da quella immagine;
        - ogni clone genererà un SID macchina univoco al primo boot, prima del join al dominio.
3. **Mitigazione temporanea (non definitiva)**
    - Microsoft menziona un **Criterio di gruppo speciale** che può disabilitare/attenuare i controlli sui SID per consentire il funzionamento temporaneo di ambienti con clonazioni storiche, ma è pensato solo come ponte verso una rigenerazione corretta e va ottenuto tramite supporto Microsoft.[^40_23][^40_6][^40_8][^40_10]

In termini pratici per la tua infrastruttura:

- il fatto che server e client condividano machine SID spiega perché SMB/stampa rifiuta credenziali verso `590-subint01`, ma funziona verso altri host;
- la correzione robusta è aggiornare il workflow Clonezilla introducendo Sysprep sulla gold image e rigenerare almeno:
    - il server di stampa,
    - i client con SID uguale al server.

Se ti serve, posso aiutarti a scrivere una procedura operativa per preparare una nuova immagine 25H2 con `sysprep /generalize` e linee guida di migrazione graduale dei client verso la nuova baseline.
<span style="display:none">[^40_24]</span>

---

# procedura sintetica per Sysprep e nuova immagine

Ti propongo una procedura sintetica “seria” per preparare una nuova immagine 25H2 con Sysprep e usarla con Clonezilla, in linea con le linee guida Microsoft.

***

## Obiettivo

- Creare una **gold image** di Windows 11 25H2 per i client (e, se volete, per i server non DC)
- garantendo che ogni macchina clonata abbia un **SID macchina univoco**, così da evitare i problemi di autenticazione Kerberos/NTLM/SMB introdotti dagli update 2025.[^41_1][^41_2][^41_3][^41_4][^41_5][^41_6][^41_7][^41_8][^41_9]

***

## Fase 1 – Preparare la macchina di riferimento

Su una macchina “di laboratorio” che diventerà la gold image:

1. Installare Windows 11 25H2 in modo pulito (ISO ufficiale, stesso SKU dei client).[^41_10][^41_1]
2. Configurare solo ciò che deve essere uguale su tutte le macchine:
    - lingua, layout, fuso orario;
    - driver di base (chipset, storage, NIC) se comuni;
    - aggiornamenti di Windows fino alla build desiderata 25H2;
    - software di base aziendale (AV, agent di monitoraggio, Office, ecc.), evitando cose fortemente legate all’identità macchina.[^41_11][^41_4][^41_5]
3. **Non** unire la macchina di riferimento al dominio.
4. Creare un account locale amministrativo (es. `AdminLocal`) da usare per Sysprep e manutenzione dell’immagine.[^41_5][^41_11]

***

## Fase 2 – Generalizzare con Sysprep

Quando la macchina di riferimento è nello stato desiderato:

1. Aprire un prompt dei comandi **elevato**.
2. Eseguire Sysprep con opzioni per imaging di produzione:
```cmd
C:\Windows\System32\sysprep\sysprep.exe /oobe /generalize /shutdown
```

- `/generalize` rimuove informazioni univoche: SID macchina, activation state, hardware‑specific info;[^41_2][^41_3][^41_1]
- `/oobe` fa sì che, al primo avvio su ogni macchina clonata, parta l’Out‑Of‑Box Experience (configurazione iniziale);[^41_1][^41_10]
- `/shutdown` spegne il sistema una volta completato Sysprep, pronto per il capture.[^41_2][^41_1]

Importantissimo:

- **Non riavviare** la macchina di riferimento dopo Sysprep prima di catturare l’immagine; se Windows parte, l’installazione non è più considerata “sysprepped” per quell ciclo.[^41_3][^41_10][^41_2]

***

## Fase 3 – Catturare l’immagine con Clonezilla

Con la macchina spenta dopo Sysprep:

1. Avviare Clonezilla da USB/ISO sulla macchina di riferimento.[^41_4][^41_12]
2. Eseguire il capture del disco o della partizione di sistema verso un’immagine Clonezilla (FFU/WIM o formato nativo).
3. Archiviare l’immagine in un repository sicuro (NAS, server imaging) come gold image 25H2 sysprepata.

***

## Fase 4 – Distribuire nuove macchine

Per ogni nuovo client:

1. Clonare l’immagine sysprepata con Clonezilla sul nuovo hardware (PC fisico o VM).[^41_12][^41_4]
2. Avviare la macchina:
    - Windows eseguirà OOBE, genererà un **nuovo SID macchina unico** e chiederà le informazioni base (nome PC, account iniziale, ecc.).[^41_10][^41_5][^41_1]
3. Eseguire le customizzazioni per quella macchina (nome host, eventuali driver aggiuntivi, ecc.).
4. Solo ora **unire al dominio**:
    - join via `System Properties` o `Add-Computer`,
    - verificare `nltest /sc_verify:DOMINIO` per confermare il secure channel.[^41_13][^41_4]

***

## Fase 5 – Migrazione dal parco esistente

Nel tuo scenario:

- Identificare i client che condividono machine SID con il server (come hai fatto).[^41_9][^41_14][^41_15]
- Pianificare la sostituzione di:
    - il server `590-subint01` (ricreato da gold image sysprepata, configurato come print/file server, poi join a dominio),
    - i client “rotti” con SID duplicato, migrando i profili utente (profili locali o roaming) e le applicazioni se necessario.
- Dopo la migrazione, verificare:
    - che ciascuna macchina abbia un machine SID diverso (lo stesso comando `Get-LocalUser Administrator`),[^41_7][^41_14][^41_15]
    - che SMB/share e stampanti su `590-subint01` funzionino con le stesse credenziali di dominio, senza loop di autenticazione.

***

## Note operative

- Sysprep ha un numero limitato di “rearm” per l’attivazione (tipicamente 8 su Windows 8+/10/11): una gold image va aggiornata raramente; se superi il limite, devi creare una nuova immagine da zero.[^41_16][^41_17][^41_11]
- Evita Sysprep su macchine già in dominio; se necessario, rimuovi dal dominio prima di generalizzare.[^41_18][^41_3]
- Se usi software di sicurezza/inventory, verifica che siano compatibili con Sysprep e imaging (alcuni agent vanno reinstallati o riconfigurati post‑clone).[^41_4][^41_5]

***

Se vuoi, nella prossima iterazione posso aiutarti a strutturare un mini documento operativo con sezione “Diagnosi” (SID duplicati e SMB), “Nuovo workflow imaging” (Sysprep + Clonezilla), e “Piano di migrazione” per server e client, in stile nota interna IT.
<span style="display:none">[^41_19]</span>

---

# Sysprep impraticabile

## Avevo gia' analizzato la best practice Microsoft, quando la clonazione era solo deprecata ma  l'uso di sysprep si e' rivelato inapplicabile alle nostre necessita' di deploy di configurazioni caratterizzate da applicazioni molto delicate che vanno installate e configurate una volta per tutte.  E' quasi inutile avere una gold image sulla quale va installata e configurata la parte specifica dopo OOBE. Cerchiamo invece di agire per una tra due strade, quella meglio percorribile:  insegnare a clonare senza portarsi appresso un machine SID oppure inibire, pur temporaneamente, il controllo su same-Machine-SID in attesa di una revisione del parco macchine

La tua esigenza è chiara: Sysprep “classico” è troppo distruttivo per la vostra gold image complessa, quindi volete o (A) clonare senza portarsi dietro il machine SID, oppure (B) disattivare temporaneamente l’enforcement sui SID duplicati in 24H2/25H2 in attesa di una revisione del parco.

Ti sintetizzo le due strade con ciò che oggi è documentato/sopportato.

***

## A) Clonare senza portarsi dietro il machine SID (senza Sysprep per OOBE)

Punto chiave: Microsoft continua a dire che **l’unico metodo supportato** per rigenerare correttamente il SID macchina è Sysprep.[^42_1][^42_2][^42_3][^42_4]

> “Microsoft non supporta i computer configurati con strumenti che duplicano il SID diversi da Sysprep. Sysprep deve essere eseguito prima della cattura dell’immagine.”[^42_3][^42_4][^42_1]

Non esiste, al momento, un comando built‑in documentato che permetta di cambiare il SID macchina “in place” preservando tutto, senza generalizzare. Gli strumenti terzi che promettono “SID change senza Sysprep” (come alcuni clone tools con funzione SID changer) non sono supportati da Microsoft, e gli articoli tecnici raccomandano di non usarli in ambienti di dominio o con enforcement dei SID.[^42_5][^42_2][^42_3]

Detto questo, per avvicinarsi alle tue necessità ci sono due approcci pratici (ma comunque basati su Sysprep):

- **Gold image “post‑applicativo” + Sysprep solo sul clone**, non sull’originale:
    - mantenete una macchina “master” con tutte le applicazioni delicate installate e configurate;
    - quando serve clonare, create una **copia** della macchina master, eseguite Sysprep sulla copia, spegnete e catturate l’immagine della copia;[^42_6][^42_2]
    - l’originale rimane intatto (non viene generalizzato), la gold image sysprepata serve solo per deployment.
- **Pipeline di imaging a due stadi**:
    - gold image “software” (senza join a dominio, senza dati utente), sysprepata;
    - script di post‑deploy che reinstalla/configura i pezzi ultra‑specifici (licenze, binding, ecc.) dopo il primo boot, sfruttando automazione (PowerShell, MDT, Intune, ecc.).[^42_7][^42_8][^42_9]

Ma se l’installazione “delicata” non è automatizzabile e davvero non può sopportare Sysprep, l’unica strada pienamente supportata resta: **macchine installate una per una**, senza clonazione, oppure usare strumenti terzi per cambiare SID consapevoli che si entra in zona non supportata.[^42_2][^42_4][^42_3]

***

## B) Disattivare (temporaneamente) l’enforcement sui SID duplicati

Qui la buona notizia è che Microsoft ha previsto una **mitigazione temporanea via Group Policy** specifica per gli errori Kerberos/NTLM dovuti a SID duplicati su Windows 11 24H2/25H2 e Server 2025.[^42_10][^42_11][^42_12][^42_13][^42_14][^42_15]

Nel documento “Errori di autenticazione Kerberos e NTLM dovuti a SID duplicati”, Microsoft dice:[^42_11][^42_13][^42_15][^42_10]

> “Per una risoluzione permanente, i dispositivi con SID duplicati devono essere ricompilati con metodi supportati.
> Gli amministratori IT possono risolvere temporaneamente il problema installando e configurando uno **speciale Criterio di gruppo**.”

Altre fonti tecniche (Q\&A, blog) precisano:[^42_16][^42_17][^42_12][^42_14][^42_15]

- La mitigazione non è una GPO standard già presente; è una policy speciale distribuita come **Known Issue Rollback (KIR)** o pacchetto GPO legato agli update (es. KB5065426, KB5064081).
- Si ottiene **solo** aprendo un ticket con Microsoft Support per clienti business / enterprise, chiedendo esplicitamente la “special Group Policy to relax SID uniqueness enforcement” riferita all’articolo sugli errori Kerberos/NTLM con SID duplicati.[^42_12][^42_14][^42_15][^42_16]
- Una volta ricevuto il pacchetto/policy:
    - viene aggiunta una voce in **Computer Configuration → Administrative Templates → System → Security Identifiers (SID) Enforcement** (il nome può variare);[^42_14]
    - configurandola, si disabilita o rilassa la nuova verifica sui SID duplicati, consentendo di nuovo autenticazione NTLM/Kerberos tra macchine con SID identici.[^42_15][^42_12][^42_14]
- Microsoft sottolinea che questa mitigazione è pensata **solo come ponte temporaneo**, e che la soluzione definitiva rimane rigenerare SID univoci.[^42_10][^42_11][^42_15]

In parallelo, alcuni scenari possono essere parzialmente mitigati regolando le impostazioni “Network security: Restrict NTLM” in GPO, per permettere NTLM dove era stato bloccato, ma questo agisce sul protocollo, non sul controllo di SID duplicati; la mitigazione ufficiale è quella special GPO.[^42_18][^42_15][^42_10]

***

## Cosa è realisticamente percorribile per voi

Considerando le tue due vie “percorribili”:

1) **Insegnare a clonare senza portarsi dietro il machine SID**
    - Supportato: usare Sysprep sulla copia della gold image prima del capture, così il SID viene rigenerato sui cloni, preservando l’originale con le app delicate.[^42_6][^42_2][^42_7]
    - Non supportato (ma tecnicamente possibile): usare strumenti terzi di “SID change” in‑place (es. utility che promettono cambio SID senza Sysprep); oggi, con l’enforcement, Microsoft sconsiglia e non supporta questa via in dominio.[^42_5][^42_2][^42_3]
2) **Inibire temporaneamente il controllo same‑Machine‑SID**
    - Supportato: aprire un ticket con Microsoft Support (commerciale) chiedendo la special GPO / KIR per SID duplicati, applicarla a OU/gruppi interessati per disattivare il check durante la migrazione del parco.[^42_16][^42_12][^42_14][^42_15][^42_10]
    - Non è una policy che puoi “inventare” da zero: va ottenuta da MS, è legata alle build 24H2/25H2/Server 2025 e agli KB specifici.

Quindi, se Sysprep è davvero impraticabile sulla gold image principale:

- **breve termine**:
    - verificare quali macchine condividono SID,
    - aprire un caso con Microsoft per ottenere e applicare la GPO speciale di mitigazione SID duplicate verso le OU dove risiedono server e client clonati,
    - ripristinare la funzionalità SMB/stampa mentre pianificate la revisione del processo di deploy.[^42_12][^42_14][^42_15][^42_10]
- **medio termine**:
    - rivedere il workflow Clonezilla introducendo Sysprep sulla *copia* della gold image (non sull’originale con le app delicate),
    - migrare gradualmente i sistemi critici verso nuove installazioni con SID univoci.

***

Se vuoi, nel prossimo passo posso aiutarti a scrivere un messaggio/ticket “bozza” da mandare al supporto Microsoft (in inglese) per chiedere la special Group Policy di mitigazione SID duplicati, con i riferimenti agli articoli e KB corretti, così riduci l’attrito nella richiesta.
<span style="display:none">[^42_19]</span>


<div align="center">⁂</div>

[^35_1]: https://stackoverflow.com/questions/53755993/wmic-useraccount-list-full-and-nothing-happen

[^35_2]: https://www.shellhacks.com/windows-get-user-sid-cmd-powershell/

[^35_3]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/windows-security/sids-not-resolve-into-friendly-names

[^35_4]: https://www.reddit.com/r/sysadmin/comments/4pabw0/why_am_i_getting_a_sid_those_long_s15211131234324/

[^35_5]: https://www.ibm.com/support/pages/domain-group-names-do-not-return-creds-output-windows-7-and-windows-2008-r2



<div align="center">⁂</div>

[^36_1]: https://turbolab.it/windows-10/come-utilizzare-sysprep-preparare-computer-aziendali-prima-clonazione-3605

[^36_2]: https://sourceforge.net/p/clonezilla/discussion/Clonezilla_live/thread/c49e293d/?limit=25

[^36_3]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^36_4]: https://learn.microsoft.com/en-my/answers/questions/5596430/kerberos-and-ntlm-authentication-failures-due-to-d

[^36_5]: https://learn.microsoft.com/en-us/answers/questions/5683259/duplicate-sids-on-windows-11p-preventing-file-shar

[^36_6]: https://learn.microsoft.com/it-it/answers/questions/3270922/rete-aziendale-allinterno-di-un-dominio

[^36_7]: https://www.reddit.com/r/sysadmin/comments/3frkhm/cloning_and_joining_domain/

[^36_8]: https://learn.microsoft.com/en-ca/answers/questions/5848589/windows-11-24h2-smb-authentication-failure-between

[^36_9]: https://learn.microsoft.com/en-us/answers/questions/5627372/special-group-policy-to-mitigate-duplicate-sid-aut

[^36_10]: https://learn.microsoft.com/en-ca/answers/questions/5638696/smb-sharing-fails-on-windows-11-25h2?page=1

[^36_11]: https://www.reddit.com/r/sysadmin/comments/1ovhg9v/win_11_computers_reject_one_another_shared_folders/

[^36_12]: https://www.youtube.com/watch?v=_7UEJ4YxR-o

[^36_13]: https://groups.google.com/g/it.comp.reti.locali/c/sxR0BMTQrSU

[^36_14]: https://www.ubackup.com/it/articoli/clonezilla-non-riuscito-a-clonare.html

[^36_15]: https://community.spiceworks.com/t/can-i-run-sysprep-after-joining-a-domain/243473


---


<div align="center">⁂</div>

[^37_1]: image.jpg

[^37_2]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^37_3]: https://www.raffaelechiatto.com/cambiare-sid-windows-server-2012-r2/

[^37_4]: https://learn.microsoft.com/en-ca/answers/questions/5848589/windows-11-24h2-smb-authentication-failure-between

[^37_5]: https://learn.microsoft.com/en-my/answers/questions/5596430/kerberos-and-ntlm-authentication-failures-due-to-d

[^37_6]: https://learn.microsoft.com/en-us/answers/questions/5683259/duplicate-sids-on-windows-11p-preventing-file-shar

[^37_7]: https://learn.microsoft.com/en-us/answers/questions/5627372/special-group-policy-to-mitigate-duplicate-sid-aut


---

<div align="center">⁂</div>

[^38_1]: image.jpg

[^38_2]: image.jpg

[^38_3]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^38_4]: https://turbolab.it/windows-10/come-utilizzare-sysprep-preparare-computer-aziendali-prima-clonazione-3605

[^38_5]: https://learn.microsoft.com/en-ca/answers/questions/5848589/windows-11-24h2-smb-authentication-failure-between

[^38_6]: https://learn.microsoft.com/en-my/answers/questions/5596430/kerberos-and-ntlm-authentication-failures-due-to-d

[^38_7]: https://learn.microsoft.com/en-us/answers/questions/5683259/duplicate-sids-on-windows-11p-preventing-file-shar

[^38_8]: https://sourceforge.net/p/clonezilla/discussion/Clonezilla_live/thread/c49e293d/?limit=25

[^38_9]: https://learn.microsoft.com/it-it/answers/questions/3270922/rete-aziendale-allinterno-di-un-dominio

[^38_10]: https://www.reddit.com/r/sysadmin/comments/1ovhg9v/win_11_computers_reject_one_another_shared_folders/

[^38_11]: https://learn.microsoft.com/en-us/answers/questions/5627372/special-group-policy-to-mitigate-duplicate-sid-aut



<div align="center">⁂</div>

[^39_1]: image.jpg

[^39_2]: image.jpg

[^39_3]: image.jpg

[^39_4]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^39_5]: https://turbolab.it/windows-10/come-utilizzare-sysprep-preparare-computer-aziendali-prima-clonazione-3605

[^39_6]: https://learn.microsoft.com/en-ca/answers/questions/5848589/windows-11-24h2-smb-authentication-failure-between

[^39_7]: https://learn.microsoft.com/en-my/answers/questions/5596430/kerberos-and-ntlm-authentication-failures-due-to-d

[^39_8]: https://learn.microsoft.com/en-us/answers/questions/5683259/duplicate-sids-on-windows-11p-preventing-file-shar

[^39_9]: https://sourceforge.net/p/clonezilla/discussion/Clonezilla_live/thread/c49e293d/?limit=25

[^39_10]: https://learn.microsoft.com/en-us/answers/questions/5627372/special-group-policy-to-mitigate-duplicate-sid-aut


<div align="center">⁂</div>

[^40_1]: https://office-skill.site/it/windows-Understanding_Windows_SIDs/

[^40_2]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^40_3]: https://turbolab.it/windows-10/come-trovare-identificatore-protezione-sid-utente-windows-computer-2181

[^40_4]: https://learn.microsoft.com/it-it/windows-server/identity/ad-ds/manage/understand-security-identifiers

[^40_5]: https://www.tenfold-security.com/en/wiki/sid-security-identifier/

[^40_6]: https://support.microsoft.com/it-it/topic/errori-di-autenticazione-kerberos-e-ntlm-dovuti-a-sid-duplicati-76f7394d-c460-4882-9ed1-d27e0960f949

[^40_7]: https://jmtella.com/foro/forum/windows/windows-11/56826-windows-11-25h2-24h2-los-sid-duplicados-provocan-errores-de-autenticación-kerberos-y-ntlm

[^40_8]: https://www.nexsys.it/sid-duplicati-e-aggiornamenti-windows-login-bloccati/

[^40_9]: https://www.vietnam.vn/it/microsoft-thay-doi-bao-mat-windows-11-may-trung-sid-co-the-bi-khoa-dang-nhap

[^40_10]: https://learn.microsoft.com/en-my/answers/questions/5596430/kerberos-and-ntlm-authentication-failures-due-to-d

[^40_11]: https://www.officineinformaticheroma.it/43180/active-directory-oggetti-con-sid-duplicati/activedirectory/recruitingservices71roma/2025/04/17/14/31/

[^40_12]: https://blog.eosdev.it/2025/12/lincubo-della-condivisione-file-su.html

[^40_13]: https://turbolab.it/windows-10/come-utilizzare-sysprep-preparare-computer-aziendali-prima-clonazione-3605

[^40_14]: https://sourceforge.net/p/clonezilla/discussion/Clonezilla_live/thread/c49e293d/?limit=25

[^40_15]: https://learn.microsoft.com/it-it/answers/questions/3270922/rete-aziendale-allinterno-di-un-dominio

[^40_16]: https://www.ilsoftware.it/microsoft-introduce-un-nuovo-requisito-per-windows-11-stop-ai-pc-clonati-senza-sysprep/

[^40_17]: https://tugatech.com.pt/t73774-windows-11-25h2-esta-a-bloquear-o-acesso-a-rede-a-culpa-e-de-uma-nova-e-importante-regra-de-seguranca

[^40_18]: https://www.msnloop.com/microsoft-renforce-la-securite-windows-pour-eviter-la-duplication-de-sid/

[^40_19]: https://learn.microsoft.com/en-ca/answers/questions/5638696/smb-sharing-fails-on-windows-11-25h2?page=1

[^40_20]: https://learn.microsoft.com/en-ca/answers/questions/5848589/windows-11-24h2-smb-authentication-failure-between

[^40_21]: https://www.reddit.com/r/sysadmin/comments/1ovhg9v/win_11_computers_reject_one_another_shared_folders/

[^40_22]: https://learn.microsoft.com/en-us/answers/questions/5683259/duplicate-sids-on-windows-11p-preventing-file-shar

[^40_23]: https://learn.microsoft.com/en-us/answers/questions/5627372/special-group-policy-to-mitigate-duplicate-sid-aut

[^40_24]: https://www.reddit.com/r/sysadmin/comments/1oim7n8/windows_11_24h2_and_25h2_dont_authenticate_to/



<div align="center">⁂</div>

[^41_1]: https://learn.microsoft.com/it-it/windows-hardware/manufacture/desktop/sysprep-command-line-options?view=windows-11

[^41_2]: https://rdr-it.com/it/windows-esegui-un-sysprep/

[^41_3]: https://www.thesolving.com/it/sala-server/quando-e-come-utilizzare-sysprep/

[^41_4]: https://turbolab.it/windows-10/come-utilizzare-sysprep-preparare-computer-aziendali-prima-clonazione-3605

[^41_5]: https://www.ilsoftware.it/focus/preparare-un-sistema-per-la-migrazione-o-la-clonazione-con-sysprep_4759/

[^41_6]: https://support.microsoft.com/it-it/topic/errori-di-autenticazione-kerberos-e-ntlm-dovuti-a-sid-duplicati-76f7394d-c460-4882-9ed1-d27e0960f949

[^41_7]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^41_8]: https://www.ilsoftware.it/microsoft-introduce-un-nuovo-requisito-per-windows-11-stop-ai-pc-clonati-senza-sysprep/

[^41_9]: https://www.nexsys.it/sid-duplicati-e-aggiornamenti-windows-login-bloccati/

[^41_10]: https://learn.microsoft.com/it-it/windows/iot/iot-enterprise/get-started/quickstart-sysprep-capture-deploy

[^41_11]: https://www.devadmin.it/2011/08/30/creazione-di-unimmagine-di-windows-7-tramite-sysprep/

[^41_12]: https://sourceforge.net/p/clonezilla/discussion/Clonezilla_live/thread/c49e293d/?limit=25

[^41_13]: https://learn.microsoft.com/it-it/answers/questions/3270922/rete-aziendale-allinterno-di-un-dominio

[^41_14]: https://learn.microsoft.com/en-my/answers/questions/5596430/kerberos-and-ntlm-authentication-failures-due-to-d

[^41_15]: https://learn.microsoft.com/en-us/answers/questions/5683259/duplicate-sids-on-windows-11p-preventing-file-shar

[^41_16]: https://www.devadmin.it/2013/12/17/scenari-di-utilizzo-di-sysprep/

[^41_17]: https://vmserv.com.br/criando-imagens-de-sistema-com-o-utilitario-sysprep/

[^41_18]: https://community.spiceworks.com/t/can-i-run-sysprep-after-joining-a-domain/243473

[^41_19]: https://www.youtube.com/watch?v=NpkMx9D-uho



<div align="center">⁂</div>

[^42_1]: https://learn.microsoft.com/fr-fr/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^42_2]: https://www.security-insider.de/server-clonen-ohne-sid-probleme-mit-sysprep-a-281796/

[^42_3]: https://www.baboo.com.br/desinformacao/clonar-um-pc-sem-usar-sysprep-e-perigoso/

[^42_4]: https://learn.microsoft.com/it-it/troubleshoot/windows-server/setup-upgrade-and-drivers/windows-installations-disk-duplication

[^42_5]: https://www.inkl.com/news/sysprep-alternative-a-non-destructive-way-to-resolve-windows-sid-conflicts

[^42_6]: https://www.security-insider.de/server-clonen-ohne-sid-probleme-mit-sysprep-gal-281796/?p=6

[^42_7]: https://turbolab.it/windows-10/come-utilizzare-sysprep-preparare-computer-aziendali-prima-clonazione-3605

[^42_8]: https://www.thesolving.com/it/sala-server/quando-e-come-utilizzare-sysprep/

[^42_9]: https://www.ilsoftware.it/focus/preparare-un-sistema-per-la-migrazione-o-la-clonazione-con-sysprep_4759/

[^42_10]: https://support.microsoft.com/it-it/topic/errori-di-autenticazione-kerberos-e-ntlm-dovuti-a-sid-duplicati-76f7394d-c460-4882-9ed1-d27e0960f949

[^42_11]: https://support.microsoft.com/es-es/topic/errores-de-autenticación-kerberos-y-ntlm-debidos-a-sid-duplicados-76f7394d-c460-4882-9ed1-d27e0960f949

[^42_12]: https://borncity.com/win/2025/10/23/windows-11-24h2-25-h2-server-2025-sid-duplicates-cause-ntlm-kerberos-authentication-errors/

[^42_13]: https://support.microsoft.com/fr-fr/topic/échecs-d-authentification-kerberos-et-ntlm-dus-à-des-sid-en-double-76f7394d-c460-4882-9ed1-d27e0960f949

[^42_14]: https://learn.microsoft.com/en-us/answers/questions/5651040/doppelte-sid-windows-server-2025-spezial-gpo

[^42_15]: https://www.ilsoftware.it/windows-11-e-server-2025-aggiornamenti-bloccano-login-e-desktop-remoto-per-colpa-dei-sid-duplicati/

[^42_16]: https://learn.microsoft.com/en-us/answers/questions/5617043/obtaining-the-special-group-policy-to-resolve-issu?page=1

[^42_17]: https://www.youtube.com/watch?v=dY_icuzo1Dg

[^42_18]: https://learn.microsoft.com/en-us/answers/questions/5648842/latest-win-11-updates-have-enforced-sid-and-now-i

[^42_19]: https://learn.microsoft.com/en-us/answers/questions/5650732/how-to-get-group-policy-for-duplicate-sid

