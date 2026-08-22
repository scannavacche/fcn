
Per le postazioni “ex‑Xerox” dove oggi non c’è alcun driver HP, la cosa più pulita è:

1. **pre‑caricare il driver UPD PCL6 8.2** sui client,
2. poi **creare la stampante TCP/IP puntando all’E730** e agganciandola a quel driver,
3. facendo tutto in modo ripetibile via script / tool di gestione, così non devi passare da ogni PC.[^16_1][^16_2][^16_3][^16_4]

Ti propongo un flusso operativo pensato per il tuo scenario di stampa diretta IP.

***

## 1. Preparare il pacchetto driver

- Metti l’eseguibile `upd-pcl6-win10-x64-8.2.0.26778.exe` su una **share di sola lettura** accessibile a tutti (es. `\\server\share\HP_UPD_8_2`).
- Esegui una volta l’exe su un sistema di test ma **solo per scompattare** i file, senza lanciare l’installer automatico HP (spunta “unzip only” o equivalente).[^16_2][^16_1]
- Prendi nota della cartella dove sta l’`INF` (tipicamente qualcosa tipo `...\x64\Driver\hpcu118u.inf` o simile).

Questo ti permette di usare il driver “puro” in modalità *Add-PrinterDriver* o “Disco driver” senza procedure guidate interattive.[^16_5][^16_1]

***

## 2. Installare il driver sulle postazioni nuove (idea PowerShell)

Per ogni client (Win 10/11) puoi usare uno script tipo:

1. **Aggiungi il driver al sistema di stampa locale**:
```powershell
pnputil /add-driver "\\server\share\HP_UPD_8_2\x64\Driver\hpcu*.inf" /install
```

oppure, su PowerShell:

```powershell
Add-PrinterDriver -Name "HP Universal Printing PCL 6"
                  -InfPath "\\server\share\HP_UPD_8_2\x64\Driver\hpcu*.inf"
```

2. **Crea la porta TCP/IP e la coda per la nuova E730**:
```powershell
$ip = "192.168.10.50"           # IP della multifunzione
$printerName = "HP E730dn - Ufficio 1"

# Porta
Add-PrinterPort -Name "IP_$ip" -PrinterHostAddress $ip -ErrorAction SilentlyContinue

# Stampante
Add-Printer -Name $printerName `
            -DriverName "HP Universal Printing PCL 6" `
            -PortName "IP_$ip"
```

Questi comandi sono supportati su Windows 10 e 11 e ti permettono di **automatizzare completamente** la creazione della stampante su ogni PC.[^16_6][^16_7]

Lo script lo puoi eseguire:

- via **GPO (Startup Script / Scheduled Task)**,
- via **Intune / altro RMM**,
- o, in mancanza d’altro, con una sessione remota PowerShell a mano sulle 10–15 macchine che devono usare quella specifica E730.

***

## 3. Dettagli pratici

- Il nome driver da usare (`"HP Universal Printing PCL 6"`) deve combaciare esattamente con come appare in “Gestione stampa” dopo che hai aggiunto il driver su una macchina di test. Vale la pena verificare e poi copiarlo alla lettera.[^16_8][^16_1]
- Puoi creare **più code** (Es. “E730dn – Piano Terra”, “E730dn – Primo Piano”, ecc.) sullo stesso driver: lo script diventa solo una lista di IP/nome.
- Sulle postazioni dove c’è solo Xerox, questo non tocca nulla di esistente: aggiungi semplicemente una o più nuove stampanti corrispondenti alle HP.

***

## 4. Variante “semi‑manuale” se non vuoi script subito

Per le pochissime macchine critiche puoi anche:

1. Lanciare l’EXE HP solo per scompattare.
2. Pannello di controllo → Dispositivi e stampanti → Aggiungi stampante → “La stampante che voglio non è nell’elenco” → “Aggiungi usando indirizzo IP o nome host”.[^16_9][^16_6]
3. Tipo dispositivo: TCP/IP, metti IP dell’E730, **non** lasciare che Windows scarichi driver da Windows Update.
4. Quando chiede il driver: “Disco driver” → sfoglia fino all’INF nella cartella scompattata (`...\x64\Driver\...inf`) e seleziona “HP Universal Printing PCL 6”.[^16_10][^16_9]

Questo conferma che il pacchetto e il nome driver sono corretti prima di automatizzare.

***

Se mi dici se hai a disposizione un dominio AD con GPO e PowerShell Remoting abilitato, nel prossimo passo posso darti uno script un po’ più “da produzione” (loop su elenco IP/nome stampante, logging minimale, controllo se il driver è già presente, ecc.).
<span style="display:none">[^16_11][^16_12][^16_13][^16_14][^16_15][^16_16][^16_17]</span>

<div align="center">⁂</div>

[^16_1]: http://www.unixwiz.net/techtips/hp-universal-pcl.html

[^16_2]: https://ccm.net/downloads/tools/5859-all-in-one-hp-printer-drivers/

[^16_3]: https://support.microsoft.com/it-it/windows/scaricare-e-installare-i-driver-della-stampante-più-recenti-4ff66446-a2ab-b77f-46f4-a6d3fe4bf661

[^16_4]: https://www.devadmin.it/2015/11/05/universal-print-driver/

[^16_5]: https://h10032.www1.hp.com/ctg/Manual/c01449237.pdf

[^16_6]: http://support-download.com/services/device/webhlp/p_manual/europe/IMC6010/it/booklist/int/190466204.htm

[^16_7]: https://support.microsoft.com/it-it/topic/gestione-della-stampa-di-rete-in-un-ambiente-windows-8e06c364-e4bf-8842-915a-ba9f077f3bda

[^16_8]: https://www.driver-soft.com/html/500374.html

[^16_9]: https://www.youtube.com/watch?v=hq0T1o_hKbA

[^16_10]: https://medvet.unipg.it/files/strutture/servizi_informatici/guida_canon_windows.pdf

[^16_11]: https://de.ccm.net/download/downloaden-4371-treiber-fuer-hp-universal-print-upd-pcl6

[^16_12]: https://www.hwdrivers.com/drivers/007568

[^16_13]: https://www.driverscape.com/download/hp-universal-printing-pcl-6

[^16_14]: https://www.reddit.com/r/k12sysadmin/comments/vtn0ck/hp_universal_printer_driver_and_windows_11/

[^16_15]: https://assets.ctfassets.net/aoj6ik149luu/4OrLGDF6H8NGlBCxARGLFb/c2f9df6d161f97f6a85555e819c9f268/GC_GBR_2018_07_HP_Installazione_driver_Universal_-_1_Emiss_16102018.pdf

[^16_16]: https://www.youtube.com/watch?v=mGm1a2NkopU

[^16_17]: https://www.youtube.com/watch?v=rGlocvaug_4


---

# Verifica dello stato dei driver installati su workstation remote

Per verificare in modo sistematico i driver di stampa installati sulle workstation remote (sia driver in sé, sia quali code li usano) ti conviene usare PowerShell con il modulo **PrintManagement** e/o WMI/CIM.[^17_1][^17_2][^17_3][^17_4]

## 1. Elenco dei driver installati su un client remoto

Su una macchina di amministrazione con PowerShell:

```powershell
$pc = "NOME-PC-REMOTE"

Get-PrinterDriver -ComputerName $pc |
    Select-Object Name, Manufacturer, MajorVersion, DriverPath
```

Questo comando ti restituisce l’elenco completo dei driver di stampa installati su quella workstation, con nome, produttore, versione principale e percorso dei file.[^17_2][^17_3][^17_1]
Se vuoi filtrare solo quelli non Microsoft (per evitare “Print to PDF”, XPS, ecc.):

```powershell
Get-PrinterDriver -ComputerName $pc |
    Where-Object { $_.Manufacturer -notmatch "Microsoft" } |
    Select-Object Name, Manufacturer, MajorVersion
```


## 2. Verificare se l’UPD HP PCL6 è presente e quale versione

Per controllare la presenza del driver “HP Universal Printing PCL 6”:

```powershell
$pc = "NOME-PC-REMOTE"

Get-PrinterDriver -ComputerName $pc -Name "HP Universal Printing PCL 6" |
    Select-Object Name, InfPath, MajorVersion, DriverPath
```

Questo ti dice se il driver esiste e da quale INF è stato installato, utile per distinguere, ad esempio, una vecchia 7.6 da una 8.2 (puoi incrociare `DriverPath`/`InfPath` con la tua cartella driver).[^17_5][^17_1][^17_2]

Se vuoi interrogare più PC in una volta:

```powershell
$computers = @("PC01","PC02","PC03")

$computers | ForEach-Object {
    $pc = $_
    Get-PrinterDriver -ComputerName $pc -Name "HP Universal Printing PCL 6" |
        Select-Object @{n='Computer';e={$pc}}, Name, MajorVersion, InfPath
}
```


## 3. Capire quali code usano quali driver

Per ogni client puoi incrociare stampanti e driver:

```powershell
$pc = "NOME-PC-REMOTE"

Get-Printer -ComputerName $pc |
    Select-Object Name, DriverName, PortName
```

Così vedi ad esempio che il PC ha code tipo “HP E730dn – Piano Terra” con `DriverName = HP Universal Printing PCL 6` o ancora Xerox/Kyocera.[^17_6][^17_7][^17_8]

Combinato con il filtro:

```powershell
Get-Printer -ComputerName $pc |
    Where-Object { $_.DriverName -like "*HP Universal*" } |
    Select-Object Name, DriverName, PortName
```


## 4. Alternativa WMI/CIM (utile su versioni senza PrintManagement)

Se ti capita qualche client dove il modulo `PrintManagement` non è disponibile, puoi usare WMI:

```powershell
$pc = "NOME-PC-REMOTE"

Get-CimInstance -ClassName Win32_PrinterDriver -ComputerName $pc |
    Select-Object Name, DriverPath
```

Questo dà lo stesso tipo di informazione, solo con una sintassi un po’ più “vecchia scuola”.

***

Se mi dici se hai già un elenco di hostname/IP delle postazioni, posso proporti uno script completo che:

- cicla su tutti i PC,
- ti costruisce una tabellina CSV `PC;DriverName;Version;InfPath` limitata ai driver HP,
- in modo da vedere subito dove manca l’UPD o dove è ancora installata la 7.6 invece della 8.2.
<span style="display:none">[^17_10][^17_11][^17_12][^17_13][^17_14][^17_15][^17_16][^17_9]</span>

<div align="center">⁂</div>

[^17_1]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printerdriver?view=windowsserver2025-ps

[^17_2]: https://docs.amfr.fr/books/powershell-qAr/page/get-printerdriver

[^17_3]: https://ithero.eu/microsoft-2/printers/

[^17_4]: https://community.spiceworks.com/topic/2484474-list-printer-drivers-installed-on-remote-machine

[^17_5]: https://woshub.com/extract-printer-drivers-windows/

[^17_6]: https://www.action1.com/blog/how-to-install-and-remove-printer-with-powershell-on-windows/

[^17_7]: http://guides.wmlcloud.com/technology/windows-server-2012---managing-print-servers-using-windows-powershell-(part-2)---managing-printers,-printer-drivers,-and-print-jobs.aspx

[^17_8]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printer?view=windowsserver2025-ps

[^17_9]: image.jpeg

[^17_10]: https://forum.bigfix.com/t/list-of-print-drivers-on-clients/1306

[^17_11]: https://devblogs.microsoft.com/scripting/how-can-i-retrieve-information-about-the-printer-driver-used-by-a-printer/

[^17_12]: https://www.youtube.com/watch?v=SGCEde5vbN0

[^17_13]: https://stackoverflow.com/questions/40507450/grab-the-printerdriver-version-with-powershell-2-0

[^17_14]: https://stackoverflow.com/questions/41187221/powershell-list-network-printers-on-remote-pc

[^17_15]: https://powershell.one/wmi/root/cimv2/win32_printerdriver

[^17_16]: https://stackoverflow.com/questions/54235934/how-to-query-win32-printer-for-installed-remote-printer-invalid-query


---

# Si, sarebbe perfetto se potessi:

- leggere l'elenco di nomi NETBIOS delle postazioni da un altro file sdi testo che riporta solo un nome per riga
- inserire nella tabellina tutti i dati che hai mostrato nelle varie opzioni, sia i percorsi degli inf per distinguere nome, provenienza e versioni ed i dati di connessione (porta, coda ecc...)

Ti propongo uno script PowerShell che fa esattamente questo: legge un file di testo con i nomi NETBIOS, interroga ciascun client, e produce un CSV con **driver installati** (inclusi percorsi INF) e **stampanti/porte** associate.[^18_1][^18_2][^18_3][^18_4]

## 1. Formato del file di input

Crea un file di testo, ad esempio `C:\temp\pc_list.txt`, con un nome NETBIOS per riga:

```text
PC-UFFICIO01
PC-UFFICIO02
PC-LAB03
...
```

Niente altro: niente virgole, niente commenti.

## 2. Script PowerShell

Puoi eseguirlo da una macchina admin con privilegi adeguati (e WinRM attivo sui client).

```powershell
# Percorso file con nomi NETBIOS
$computerListPath = "C:\temp\pc_list.txt"

# Output CSV
$outputCsv = "C:\temp\Report_DriverStampanti_HP.csv"

# Legge elenco PC (salta righe vuote o commenti con #)
$computers = Get-Content -Path $computerListPath |
             Where-Object { $_ -and $_.Trim() -ne "" -and -not $_.Trim().StartsWith("#") }

$results = @()

foreach ($pc in $computers) {
    Write-Host "Interrogo $pc ..." -ForegroundColor Cyan
    try {
        # Driver di stampa (tutti)
        $drivers = Get-PrinterDriver -ComputerName $pc -ErrorAction Stop

        # Stampanti installate
        $printers = Get-Printer -ComputerName $pc -ErrorAction Stop

        foreach ($drv in $drivers) {
            # Tutte le code che usano questo driver
            $printersUsingDriver = $printers | Where-Object { $_.DriverName -eq $drv.Name }

            if ($printersUsingDriver) {
                foreach ($prn in $printersUsingDriver) {
                    $obj = [PSCustomObject]@{
                        ComputerName   = $pc
                        DriverName     = $drv.Name
                        DriverManufacturer = $drv.Manufacturer
                        DriverVersion  = $drv.MajorVersion
                        DriverInfPath  = $drv.InfPath
                        DriverPath     = $drv.DriverPath
                        PrinterName    = $prn.Name
                        PrinterDriver  = $prn.DriverName
                        PrinterPort    = $prn.PortName
                        PrinterShare   = $prn.ShareName
                        PrinterComment = $prn.Comment
                    }
                    $results += $obj
                }
            }
            else {
                # Driver installato ma non associato a nessuna coda
                $obj = [PSCustomObject]@{
                    ComputerName   = $pc
                    DriverName     = $drv.Name
                    DriverManufacturer = $drv.Manufacturer
                    DriverVersion  = $drv.MajorVersion
                    DriverInfPath  = $drv.InfPath
                    DriverPath     = $drv.DriverPath
                    PrinterName    = $null
                    PrinterDriver  = $null
                    PrinterPort    = $null
                    PrinterShare   = $null
                    PrinterComment = $null
                }
                $results += $obj
            }
        }
    }
    catch {
        Write-Warning "Impossibile interrogare $pc : $($_.Exception.Message)"
        $obj = [PSCustomObject]@{
            ComputerName   = $pc
            DriverName     = $null
            DriverManufacturer = $null
            DriverVersion  = $null
            DriverInfPath  = $null
            DriverPath     = $null
            PrinterName    = $null
            PrinterDriver  = $null
            PrinterPort    = $null
            PrinterShare   = $null
            PrinterComment = "ERRORE: $($_.Exception.Message)"
        }
        $results += $obj
    }
}

$results | Export-Csv -Path $outputCsv -NoTypeInformation -Encoding UTF8
Write-Host "Report generato in $outputCsv" -ForegroundColor Green
```

Questo usa i cmdlet `Get-PrinterDriver` e `Get-Printer` del modulo **PrintManagement** per estrarre driver e stampanti da ciascun host, e `InfPath/DriverPath` per distinguere versioni e provenienza driver.[^18_2][^18_3][^18_4][^18_5][^18_1]

## 3. Come leggerlo per gli HP

Nel CSV potrai filtrare, ad esempio, con Excel o PowerShell:

- tutte le righe con `DriverName` contenente `"HP Universal Printing PCL 6"` per vedere dove è installato e con quale INF/percorso;
- tutte le righe con `PrinterDriver` contenente `"HP Universal"` per sapere quali code lo usano e su quali porte (`PrinterPort` ti dirà se è `IP_192.168.x.y`, `WSD-...`, ecc.).

Se vuoi, nel prossimo passo posso aggiungere un filtro nel report per registrare **solo** driver HP e relative code, così il CSV resta più compatto.
<span style="display:none">[^18_10][^18_11][^18_12][^18_13][^18_14][^18_15][^18_6][^18_7][^18_8][^18_9]</span>

<div align="center">⁂</div>

[^18_1]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printerdriver?view=windowsserver2025-ps

[^18_2]: https://docs.amfr.fr/books/powershell-qAr/page/get-printerdriver

[^18_3]: https://powershell.one/wmi/root/cimv2/win32_printerdriver

[^18_4]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printer?view=windowsserver2025-ps

[^18_5]: https://ithero.eu/microsoft-2/printers/

[^18_6]: https://docs.oracle.com/cd/E19957-01/806-0440-10/z40005fe46fd/index.html

[^18_7]: https://www.gabrielecaracciolo.com/nbtstat-leggere-la-tabella-dei-nomi-netbios/

[^18_8]: https://it.macspots.com/netbios-what-it-is

[^18_9]: https://it.wikipedia.org/wiki/NetBIOS

[^18_10]: http://web.tiscali.it/redalarm/MANUALI/netbios.htm

[^18_11]: https://d1.amobbs.com/bbs_upload782111/files_6/armok01131795.pdf

[^18_12]: https://www.jnetworld.com/netbios.htm

[^18_13]: https://it.wikipedia.org/wiki/NetBIOS_su_TCP

[^18_14]: https://hackita.it/articoli/nbtscan/

[^18_15]: https://www.why-tech.it/comandi-dos-rete.html


---

# sta funzionando bene ma ora mi sorge un'altra domanda:

Io vedo tutti i drivers di stampa presenti a sistema, compresi quelli delle stampanti che non sono piu' presenti nel pannello "dispositivi e stampanti".

c'e' un modo per fare pulizia e disinstallare drivers non piu' utilizzati, magari elencandone il DriverInfPath ?

Sì, si può fare pulizia in modo abbastanza controllato, partendo proprio dall’elenco che hai già (Name + InfPath), e rimuovendo i driver che non sono più associati a nessuna coda. L’idea è:

1. identificare i driver “orfani”;
2. per quelli, usare `Remove-PrinterDriver` e, se vuoi essere più aggressivo, anche `pnputil` sull’`oemXX.inf` corrispondente.[^19_1][^19_2][^19_3][^19_4]

Ti propongo prima la logica, poi uno script-tipo.

***

## 1. Trovare i driver “non più usati”

Hai già qualcosa del genere:

```powershell
$drivers  = Get-PrinterDriver -ComputerName $pc
$printers = Get-Printer       -ComputerName $pc
```

Per ogni driver puoi verificare se qualche stampante lo usa:

```powershell
foreach ($drv in $drivers) {
    $printersUsing = $printers | Where-Object { $_.DriverName -eq $drv.Name }
    if (-not $printersUsing) {
        # driver orfano
    }
}
```

Questo è il criterio più sicuro: rimuovi solo i driver che **non compaiono come DriverName di nessuna coda** su quella macchina.

***

## 2. Rimozione via PowerShell (API di stampa)

Per un driver orfano:

```powershell
Remove-PrinterDriver -ComputerName $pc -Name $drv.Name
```

`Remove-PrinterDriver` usa lo stesso backend delle “Proprietà server di stampa” nella GUI; è il primo passo raccomandato per pulire driver non più necessari.[^19_2][^19_5]
Se va in errore con “in uso”, spesso basta:

- assicurarsi che non ci siano code che lo usano (già fatto),
- riavviare lo spooler (`net stop spooler` / `net start spooler`),
- riprovare.

***

## 3. Rimozione del pacchetto INF (opzionale, più radicale)

Anche rimosso a livello “Print Server Properties”, il **pacchetto INF** può restare nello store driver (`oemXX.inf`) e venire riusato automaticamente in futuro.[^19_3][^19_1]

Procedura classica:

1. Elenco INF installati:
```powershell
pnputil.exe /enum-drivers
```

(o `pnputil /e` sulle versioni più vecchie).[^19_1][^19_3]

2. Individui le voci con `Driver Package Provider` = HP e nome che corrisponde al tuo UPD/driver legacy, sfruttando i dati di versione e data.[^19_3][^19_1]
3. Rimuovi il pacchetto:
```powershell
pnputil.exe /delete-driver oem123.inf /uninstall /force
```

(o `pnputil.exe -f -d oem123.inf` nelle sintassi precedenti).[^19_1][^19_3]

Questo step lo farei solo:

- su driver che sai essere definitivamente obsoleti (vecchie Kyocera, pacchetti vendor che non userai più),
- e possibilmente dopo un riavvio o comunque con spooler fermo, perché `pnputil` fallisce se il driver è ancora in uso.[^19_5][^19_3][^19_1]

***

## 4. Script di base per elenco + pulizia driver orfani (senza toccare INF)

Ti lascio uno scheletro sicuro, che **per ora si limita a loggare** i driver orfani, e puoi decidere tu quali rimuovere (basta togliere il commento sulla riga `Remove-PrinterDriver`).

```powershell
$pcListPath = "C:\temp\pc_list.txt"
$logCsv     = "C:\temp\DriverOrfani.csv"

$pcs = Get-Content $pcListPath |
       Where-Object { $_ -and $_.Trim() -ne "" -and -not $_.StartsWith("#") }

$orphans = @()

foreach ($pc in $pcs) {
    Write-Host "Analizzo $pc ..." -ForegroundColor Cyan
    try {
        $drivers  = Get-PrinterDriver -ComputerName $pc -ErrorAction Stop
        $printers = Get-Printer       -ComputerName $pc -ErrorAction Stop

        foreach ($drv in $drivers) {
            $printersUsing = $printers | Where-Object { $_.DriverName -eq $drv.Name }

            if (-not $printersUsing) {
                # Driver installato ma non usato da nessuna coda
                $obj = [PSCustomObject]@{
                    ComputerName   = $pc
                    DriverName     = $drv.Name
                    Manufacturer   = $drv.Manufacturer
                    MajorVersion   = $drv.MajorVersion
                    InfPath        = $drv.InfPath
                    DriverPath     = $drv.DriverPath
                    Note           = "ORFANO"
                }
                $orphans += $obj

                # RIMOZIONE (quando sei sicuro):
                # Remove-PrinterDriver -ComputerName $pc -Name $drv.Name -ErrorAction SilentlyContinue
            }
        }
    }
    catch {
        Write-Warning "Errore su $pc : $($_.Exception.Message)"
    }
}

$orphans | Export-Csv -Path $logCsv -NoTypeInformation -Encoding UTF8
Write-Host "Report driver orfani salvato in $logCsv" -ForegroundColor Green
```

Con questo:

- ti fai prima un’idea precisa di **quali** driver sono inutilizzati (con InfPath/DriverPath);
- puoi filtrare il CSV per vendor, versione, ecc.;
- poi ri‑esegui lo script con la riga `Remove-PrinterDriver` scommentata, magari limitando il `Where-Object` a driver con `Manufacturer -like "HP*"` o a vecchi pacchetti Xerox/Kyocera che vuoi proprio togliere.

Se vuoi, al prossimo passo posso estendere lo script per generare **anche un file batch/PowerShell con i comandi `pnputil` mirati** sugli `oemXX.inf` corrispondenti ai driver orfani loggati, così hai una “fase 2” di pulizia dello store driver solo quando sei sicuro di quello che stai rimuovendo.
<span style="display:none">[^19_10][^19_6][^19_7][^19_8][^19_9]</span>

<div align="center">⁂</div>

[^19_1]: https://turbolab.it/stampanti-70/rimuovere-completamente-driver-stampante-non-funziona-non-si-reinstalla-138

[^19_2]: https://www.ilsoftware.it/focus/come-rimuovere-un-driver-di-stampa-difettoso-in-windows-7_6846/

[^19_3]: https://help.nicelabel.com/hc/it/articles/4413996133009-Rimozione-dei-driver-dall-archivio-dei-driver-di-Windows

[^19_4]: https://learn.microsoft.com/it-it/windows-hardware/drivers/install/using-device-manager-to-uninstall-devices-and-driver-packages

[^19_5]: https://www.oki.com/printing/online-manuals-Z005/EE8001-1215/id/contents/contents/38931545.html

[^19_6]: https://answers.microsoft.com/it-it/windows/forum/all/driver-di-stampa/288d8d42-395a-4586-91b4-9b03765c8a7c

[^19_7]: https://www.epson.it/it_IT/faq/KA-01056/contents?loc=it-it

[^19_8]: https://www.youtube.com/watch?v=9B2zQdU_hug

[^19_9]: https://www.aranzulla.it/come-disinstallare-driver-951697.html

[^19_10]: https://www.youtube.com/watch?v=PqxRQTAjSRY


---

# anche questo script funziona bene. Per decidere se un driver e' orfano controlla soltanto   \$printersUsing = \$printers | Where-Object { \$_.DriverName -eq \$drv.Name)  ?

Esatto: nello script che ti ho proposto il criterio di “orfano” è **solo**:

```powershell
$printersUsing = $printers | Where-Object { $_.DriverName -eq $drv.Name }
if (-not $printersUsing) {
    # considerato orfano
}
```

cioè: *“nessuna coda di stampa locale su quella macchina usa questo driver come `DriverName`”*.[^20_1][^20_2]

## Cosa implica esattamente

- Vengono considerati **non orfani** solo i driver che hanno almeno una stampante installata che li referenzia esplicitamente (come succede nelle proprietà di una coda in `Dispositivi e stampanti`).[^20_2][^20_1]
- Tutti gli altri driver che risultano installati ma non legati ad alcuna coda effettiva (magari residui di vecchie stampanti rimosse) finiscono nel report come “orfani” e candidati alla rimozione.[^20_3][^20_4]

È una definizione volutamente **conservativa dal punto di vista funzionale**:

- Non guarda se il pacchetto INF è ancora nello store (`oemXX.inf`), ma solo se il driver è usato da qualche stampante visibile a `Get-Printer`.
- Non distingue tra driver caricati “di riserva” da qualche software vendor (rari in ambito stampanti), ma in pratica, per le stampanti classiche, se non c’è nessuna coda che lo usa, è davvero un residuo storico.[^20_5][^20_3]

Se vuoi raffinare il criterio (es. trattare come non‑orfani tutti gli HP UPD anche se non ancora agganciati a una coda, perché li vuoi tenere come base comune), puoi banalmente filtrare:

```powershell
if (-not $printersUsing -and $drv.Name -notlike "HP Universal Printing PCL 6*") {
    # orfano "da pulire"
}
```

Vuoi che ti prepari una variante dello script che:

- considera *sempre* “protetti” i driver di una certa whitelist (es. HP UPD PCL6 8.2),
- e marca come orfani solo gli altri, per ridurre ulteriormente il rischio di pulizie troppo aggressive?

<div align="center">⁂</div>

[^20_1]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printerdriver?view=windowsserver2025-ps

[^20_2]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printer?view=windowsserver2025-ps

[^20_3]: https://turbolab.it/stampanti-70/rimuovere-completamente-driver-stampante-non-funziona-non-si-reinstalla-138

[^20_4]: https://www.ilsoftware.it/focus/come-rimuovere-un-driver-di-stampa-difettoso-in-windows-7_6846/

[^20_5]: https://help.nicelabel.com/hc/it/articles/4413996133009-Rimozione-dei-driver-dall-archivio-dei-driver-di-Windows


---

# no, va bene cosi'

In effetti la necessita' potrebbe essere quella di eliminare un inf file nel caso windows si ostinasse a caricare quello invece di una versione specifica che voglio installare.
Pero' e' un problema che si presenta anche con altre periferiche,soprattutto con el schede di rete. Devo proprio riprendere a studiare powershell perche' ci sono implicazioni molto utili

Hai centrato il punto: quando Windows “insiste” a ripescare sempre lo stesso pacchetto dallo store (stampante, NIC, ecc.), l’unico modo davvero risolutivo è agire sul **driver store** (via `pnputil`) oltre che sulla parte “logica” (device/queue).[^21_1][^21_2][^21_3]

Per collegare i pezzi:

- Livello “stampante” / “periferica”: rimuovi code, dispositivi e driver associati (GUI o `Remove-PrinterDriver`, `Remove-WindowsDriver`, ecc.).[^21_4][^21_5]
- Livello “INF”: finché lo `oemXX.inf` resta nello store, Windows può continuare a preferirlo in automatico alla prima occasione; è qui che `pnputil /enum-drivers` + `/delete-driver oemXX.inf /uninstall /force` diventa l’arma finale, non solo per le stampanti ma anche per le NIC e il resto.[^21_2][^21_3][^21_1]

PowerShell, da questo punto di vista, è il coltellino svizzero giusto:

- ti fa mappare in modo ripetibile **cosa è installato dove**,
- ti permette di orchestrare modifiche mirate (anche delicate come sostituire driver di rete) con logging e rollback ragionati.

Se ti va, prossima volta possiamo costruire insieme un mini “toolkit PowerShell per driver” con tre moduli separati:

1) inventario driver (stampa, NIC, storage),
2) individuazione e rimozione pacchetti INF “sbagliati”,
3) pre‑caricamento di driver “buoni” su tutto il parco.

<div align="center">⁂</div>

[^21_1]: https://turbolab.it/stampanti-70/rimuovere-completamente-driver-stampante-non-funziona-non-si-reinstalla-138

[^21_2]: https://help.nicelabel.com/hc/it/articles/4413996133009-Rimozione-dei-driver-dall-archivio-dei-driver-di-Windows

[^21_3]: https://learn.microsoft.com/it-it/windows-hardware/drivers/install/using-device-manager-to-uninstall-devices-and-driver-packages

[^21_4]: https://www.ilsoftware.it/focus/come-rimuovere-un-driver-di-stampa-difettoso-in-windows-7_6846/

[^21_5]: https://www.oki.com/printing/online-manuals-Z005/EE8001-1215/id/contents/contents/38931545.html


---

# si, questa e' una bella idea

Allora la mettiamo in cantiere. Ti propongo questa roadmap, così la prossima volta sappiamo da dove ripartire.

## 1. Modulo 1 – Inventario driver

Obiettivo: sapere “cosa c’è dove”, per tipo di periferica.

- Stampanti: `Get-PrinterDriver`, `Get-Printer`, già impostato.[^22_1][^22_2]
- NIC: `Get-NetAdapter`, `Get-CimInstance Win32_PnPSignedDriver` filtrato per classi `Net`, `NetService`.[^22_3]
- Storage / altre periferiche critiche: sempre via `Win32_PnPSignedDriver`, con filtri su `DeviceClass` e `DriverProviderName`.[^22_4][^22_3]

Output: uno o più CSV con `Computer; Device; DriverName; Version; InfName; Provider`, che ti danno la fotografia completa del parco.

## 2. Modulo 2 – Trovare e neutralizzare INF “sbagliati”

Obiettivo: identificare e, quando serve, rimuovere pacchetti driver che non vuoi più vengano usati.

- Query dello store: `pnputil /enum-drivers` o `Get-CimInstance Win32_PnPSignedDriver` per estrarre mappa `oemXX.inf` ↔ dispositivo/driver.[^22_5][^22_6]
- Matching con una lista nera o con versioni specifiche (es. “qualsiasi HP UPD < 8.0”, “qualunque Realtek older than…”).[^22_6][^22_4]
- Generazione di uno **script di rimozione**:

```powershell
pnputil /delete-driver oem123.inf /uninstall /force
```

da eseguire solo quando hai validato l’inventario.[^22_5][^22_6]


## 3. Modulo 3 – Pre‑carico dei driver “buoni”

Obiettivo: fare in modo che, quando aggiungi/ricolleghi una periferica:

- Windows scelga **il tuo** pacchetto,
- e non vada a ripescare roba vecchia/di Windows Update.

Strumenti:

- `pnputil /add-driver path\to\*.inf /install` per caricare nello store e pre‑installare driver su tutte le macchine target.[^22_4][^22_6]
- Script PowerShell che:
    - distribuisce il contenuto di una share driver locale,
    - verifica la presenza del driver “buono”,
    - eventualmente crea/ricrea la periferica (per stampanti: `Add-PrinterDriver`, `Add-PrinterPort`, `Add-Printer`).[^22_2][^22_7][^22_1]

***

Direi che, per cominciare, ha senso partire dal **Modulo 1** per NIC + stampanti, così hai un inventario unico driver/periferiche da cui far discendere decisioni su INF da tenere o rimuovere.

Vuoi che alla prossima sessione partiamo direttamente costruendo lo script di inventario “multi‑periferica” (stampanti + schede di rete) prendendo sempre la lista di PC dal file di testo, come abbiamo fatto per le stampanti?

<div align="center">⁂</div>

[^22_1]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printerdriver?view=windowsserver2025-ps

[^22_2]: https://learn.microsoft.com/en-us/powershell/module/printmanagement/get-printer?view=windowsserver2025-ps

[^22_3]: https://powershell.one/wmi/root/cimv2/win32_printerdriver

[^22_4]: https://learn.microsoft.com/it-it/windows-hardware/drivers/install/using-device-manager-to-uninstall-devices-and-driver-packages

[^22_5]: https://turbolab.it/stampanti-70/rimuovere-completamente-driver-stampante-non-funziona-non-si-reinstalla-138

[^22_6]: https://help.nicelabel.com/hc/it/articles/4413996133009-Rimozione-dei-driver-dall-archivio-dei-driver-di-Windows

[^22_7]: https://woshub.com/extract-printer-drivers-windows/

