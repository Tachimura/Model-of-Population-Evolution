# Model-of-Population-Evolution
Progetto modellazione di una popolazione composta da elementi di tipo A e di tipo B (maschi e femmine)

## Correct Compilation and Execution
Una corretta compilazione può essere fatta in due modi differenti:

* A moduli separati, eseguendo il file eseguibile:<br/>
./jcompiler<br/>
* A moduli insieme, eseguendo il file eseguibile:<br/>
./jcompiler_default<br/>
<br/>
PER IL FILE DI CONFIGURAZIONE (settings.txt), I DATI INTERI SONO DA INSERIRE 1 PER RIGA:<br/>
init_people<br/>
genes<br/>
birth_death<br/>
sim_time<br/>
<br/>
Ad esempio:<br/>
10<br/>
1000<br/>
3<br/>
20<br/>
<br/>
Questo farà eseguire il programma con:<br/>
-una popolazione da 10 elementi<br/>
-il gene degli elementi iniziali sarà un numero fra 2 e 1002<br/>
-ogni 3 secondi avverrà una birth_death che ucciderà un ughino e ne creerà uno nuovo<br/>
-dopo 20 secondi la simulazione terminerà
