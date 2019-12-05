Percorrendo le strade francesi è  da anni facile trovare cartelli che invitano le auto a sorpassare i ciclisti lasciando una distanza di sicurezza di un metro e mezzo. Qualcosa del genere inizia ad accadere in Italia. Indipendentemente dall'opinione che si ha dei ciclisti capita di trovarsi ad essere superati da auto che ti fanno 'il pelo' anche a gran velocità.

è difficile dire quando vicino ti passino e l'opinione sulla stessa cosa tra chi guida l'auto o la bici (ma anche i pedoni) è molto diversa. 

Quindi? si continua a vivere di opinioni? Abbiamo la possibilità di provare ad avere dei dati? Con l'esperienza del robot Arianna ho usato un Lidar per la misura delle distanze. Quello che sto costruendo è un dispositivo da montare sulla bicicletta. Misura la distanza sul lato sinistro. Misurando continuamente questa distanza mi prefiggo di identificare quando un'auto mi affianca, misurare la sua distanza durante il sorpasso, dal tempo di affiancamento avere una stima della sua velocità e altre informazioni.

l'esercizio è interessante perché voglio vedere come si possano correlare dati rilevati con altre informazioni ricavate tramite dei "reasonable guess". Effettuerò le misure sul percorso casa lavoro, è un percorso noto e ripetuto. Tramite la misurazione del tempo e la mappa sarò in grado di stimare la posizione anche senza un GPS. La mia velocità è approssimativamente nota, cosi come le lunghezze medie delle auto: questo mi darà una ragionevole stima della velocità dell'auto. Questo e altre cose che scoprirò usandolo.

# il lidar

il lidar è un misuratore di distanza basato su un laser. Caratteristica del sensore usato è la il range di misura da alcune decine di cm sino a 12 metri dichiarati e l'angolo di misura di 2.4 gradi. L'angolo di misura è il cono dove un oggetto deve stare per essere percepito. Questo lidar è direzionale, "vede" solo gli oggetti di fronte.

http://wiki.seeedstudio.com/Grove-TF_Mini_LiDAR/

il suo funzionamento è semplice. Alimentatolo inizia a misurare e trasmette il valore letto. Il suo rate è di 100 letture al secondo. Un nuovo dato ogni 10 ms.


cose da verificare!!
come usarlo all'aperto ??
schermato dal sole basta ??

# La CPU usata

ho usato un ESP8266 montato su basette WEMOS D1 mini. Questi circuiti si montato uno sull'altro e hanno dimensioni molto ridotte. Ho aggiunto lo shield che gestisce la una batteria LiPo. il tutto è compatto, portatile e comprende già la carica da micro usb.

L'ESP8266 ha una sola porta seriale. questa è collegata alla USB ed è usata per la programmazione. Leggendo il data sheet si vede che esiste una **seconda porta seriale** di cui però solo il canale di trasmissione è disponibile sul pin D4.

Ho collegato il LIDAR alla porta zero e uso la porta uno per inviare messaggi di debug.



```
#define debugPort Serial1

//------------------------------------------------------------------------
void setup() {
    debugPort.begin(115200);
    Serial.begin(115200);
    debugPort.println("\n \n port1 initialized");
```

*ATTENZIONE: quando carico il programma nell'ESP devo scollegare il pin TX del LIDAR.*



# programma di test uno

il primo test è fatto aggiungendo al esempio del LIDAR una macchina a stati. Due piccole note sul codice:

nel loop viene testato l'arrivo del pacchetto con la distanza, il codice non procede sinché il dato non arriva. 

```c
void loop() {

  if(dataAvalilable(&distance, &strength)) {
    //debugPort.println(distance);
  }
  else return; // questo fa esegueire la SM solo con nuovi dati
```

l'affiancamento dell'auto viene stabilito quando la distanza scende sotto soglia e vi rimane per 10 cicli pari a 100 ms.
L'ultimo valore giunto è salvato come distanza iniziale.
Questo è stato fatto perché durante i test si è visto che la distanza non cambiava a gradino ma impiegava alcuni campioni, una decina appunto, per arrivare la valore finale.

```c
    // wait distance goes below CROSS_DISTANCE
    case NO_CAR:
  		if (distance < CROSS_DISTANCE){

        counter++;
        if (counter > 10){
            carIndex++;
            if(carIndex >= 1000) carIndex = 0;
      			
      			carCrossing[carIndex].tInit 		= millis();
      			carCrossing[carIndex].initDistance= distance;
      			sm = CAR_ASIDE;
      			if (verbose){
      				debugPort.println("CAR_ASIDE");
      			}
  			}
      }
      else counter = 0;
      break;
```

questo accade in maniera simile alla fine dell'affiancamento.

```c
    case CAR_ASIDE:
  		if (distance > CROSS_DISTANCE){
        
  			carCrossing[carIndex].tEnd 		= millis();

        // index 10 samples before
  			carCrossing[carIndex].endDistance	= points[pointIndex - 10].d;
```



la stampa è effettuata inviando una riga per ogni misura ricevuta per essere certi di non sovrapporsi alla ricezione del LIDAR.

```c
case CAR_PRINT:
    if (verbose){
        switch (riga){
        case 0:   debugPort.print("CAR: ");                       break;
        case 1:   debugPort.println(carIndex);                    break;
        case 2:   debugPort.print("start time:");                 break;
        case 3:   debugPort.println(carCrossing[carIndex].tInit );break;
        case 4:   debugPort.print("aside time:")         ;        break;
        case 5:   debugPort.println(carCrossing[carIndex].tEnd - carCrossing[carIndex].tInit );break;
        case 6:   debugPort.print("init, end, average distance:");break;
        case 7:   debugPort.print(carCrossing[carIndex].initDistance);break;
        case 8:   debugPort.print(", ");                          break;
        case 9:   debugPort.print(carCrossing[carIndex].endDistance );break;
        case 10:  debugPort.print(", ");                          break;
        case 11:
        debugPort.println(carCrossing[carIndex].averageDistance );
        sm = INIT;
        break;
        }
    riga++;
    }
break;
```

