@echo cleaning up
DEL *.bak

pause
exit

 
REM del 
Elimina uno o più file.

DEL [/P] [/F] [/S] [/Q] [/A[[:]attributi]] nomefile
ERASE [/P] [/F] [/S] [/Q] [/A[[:]attributi]] nomefile

 nomefile       Specifica un elenco di uno o più file odirectory.
                Usare i caratteri jolly per eliminare più file.
                Specificando una directory, tutti i file al suo
                interno saranno eliminati.

  /P         Chiede conferma prima di eliminare ogni file.
  /F         Forza l'eliminazione dei file di sola lettura.
  /S         Elimina i file specificati da tutte le sottodirectory.
  /Q         Modalità silenziosa, non chiede conferma per eliminazioni globali.
  /A         Seleziona i file da eliminare in base agli attributi.
  attributi  R  File di sola lettura         S  File di sistema
             H  File nascosti                A  File di archivio
             -  Prefisso per negare l'attributo

 
