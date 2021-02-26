# Model-of-Population-Evolution
Progetto modellazione di una popolazione composta da elementi di tipo A e di tipo B (maschi e femmine)

## Correct Compilation and Execution
Una corretta compilazione può essere fatta in due modi differenti:
* A moduli separati, eseguendo il file eseguibile:<br/>
./jcompiler<br/>
* A moduli insieme, eseguendo il file eseguibile:<br/>
./jcompiler_default<br/>

## Correct Setup
Si possono modificare differenti impostazioni modificando il file settings.txt.<br/>
Le impostazioni sono date inserendo numeri interi uno per riga:<br/>
L'ordine di lettura delle impostazioni è: </br>
- init_people<br/>
- genes<br/>
- birth_death<br/>
- sim_time<br/>

Ad esempio, impostando il file di impostazioni in questo modo:<br/>
10<br/>
1000<br/>
3<br/>
20<br/>
<br/>
Questo farà eseguire il programma con:<br/>
- Una popolazione da 10 elementi<br/>
- Il gene degli elementi iniziali sarà un numero fra 2 e 1002<br/>
- Ogni 3 secondi avverrà una birth_death che ucciderà un ughino e ne creerà uno nuovo<br/>
- Dopo 20 secondi la simulazione terminerà
