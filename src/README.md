# Paradigma Map-Reduce (Programare Paralela)

> Am verificat tema, folosind Docker, cu care am obtinut punctaj maxim.


## Structuri de Date


### Rezultatul Mapper-ilor

Fiecare thread Mapper produce un rezultat, acesta fiind un vector de perechi **word - fileID**.
De vreme ce acelasi Mapper poate colecta acelasi cuvant din fisiere diferite,
**MapperResult** nu se poate implementa drept un dictionar **word - fileID**,
ci ca un vector, in care aceste perechi sa fie unice.



### WordList-ul

WordList-ul implementat de mine foloseste doua structuri de date (intermediare):
- Un vector de dictionare
- Un vector de liste

> Cate un dictionar si cate o lista pentru fiecare litera din alfabet.


### Variabile Partajate

In main, se va crea o instanta a unei clase **SharedVariables**
care retine toate valorile partajate intre thread-uri.
La crearea unui thread, **SharedVariables** de constructia argumentului thread-ului,
fiecare thread primind referinte la aceste variabile partajate.
In functie de tipul thread-ului creat, se vor referentia variabile diferite
(thread-urilor Mapper/Reducer li se vor da doar referintele de care au nevoie, nu mai mult).


Variabilele partajate intre thread-uri contin atat datele de interes
(numele fisierele de intrare, rezultatele Mapper-ilor ...),
cat si primitivele de sincronizare din **pthread.h** (mutex-uri, o variabila conditionala si o bariera).


Aceasta clasa (**SharedVariables**) permite crearea cu usurinta a argumentelor thread-urilor.

In plus, tot **SharedVariables** se ocupa si de:
- Alocarea si dezalocarea memoriei
- Initializarea variabilelor partajate


### Clase pentru thread-urile Mapper/Reducer


Cele doua tipuri de thread-uri vor avea cate o clasa,
iar instantele lor vor avea referinte la variabilele partajate intre thread-uri
(mai putin ID-urile thread-urilor, acestea sunt unice si nu se vor partaja intre thread-uri).

Totusi, avand in vedere semnatura (header-ul) functiei `pthread_create`,
metoda furnizata ca argument lui `pthread_create` nu are voie sa apartina vreunei instante.
Eu am implementat metoda pe care thread-urile o executa ca fiind metoda **statica** (`static void* routine(void *arg);`) in cadrul acestor clase.
Metodei `routine` i se va da ca argument o referinta la o istanta din cele doua clase (**MapperThread**/**ReducerThread**).
In interiorul functiei `routine`, argumentul se va dereferentia la un tip de date concret.




## Programarea Paralela

### Problema Producatori - Consumatori

Paradigma Map-Reduce rezolva problemea producator-consumator, in ipostaza mai multi producatori - mai multi consumatori:
- N-M: mai multi producatori (fisiere de intrare), mai multi consumatori (Mapperi)
- N-M: mai multi producatori (rezultatele Mapper-ilor), mai multi consumatori (cate un dictionar din WordList pentru fiecare litera din alfabet)
- N-M: mai multi producatori (cate un dictionar din WordList pentru fiecare litera din alfabet),
  mai multi consumatorii (cele 26 de sectiuni din WordList - implicit si fisierele de iesire corespondente - pentru fiecare litera mica din alfabetul englez)


> NOTA:
> Totusi, daca as fi implementat WordList-ul drept un dictionar/lista mare,
> si nu l-ar fi impartit pentru litera din alfabet,
> ultimele doua puncte din lista ar fi fost **many to one**, respectiv **one to many**.


### Thread-urile Mapper


Mapperii rezolva problema producator - consumator in ipostaza **many to many**:
mai multi producatori (fisierele de intrare), mai multi consumatori (rezultatele Mapper-ilor).

Un thread Mapper parcurge fisierele de intrare
si extrage cuvintele unice in cate un **set** pentru fiecare fisier.  


Un thread Mapper:
- Primeste o referinta la o instanta a clasei `MapperThread`,
  care contine referinte la variabilele partajate de care are nevoie
- Intr-o bucla infinita:
   - Pune **lock** pe **mutex-ul cozii indicilor fisierelor**
   - Daca coada este vida, elibereaza mutex-ul (ii da **unlock**) si opreste imediat bucla
      Altfel, extrage din coada un index al unui fisier de intrare,
      elibereaza mutex-ul (**unlock**) si continua cu procesarea fisierului
  - Pentru indexul extras din coada:
    - Parcurge fisierul caracter cu caracter
    - Construieste cuvintele (acestea contin doar litere mici) in felul urmator:
      - Foloseste o variabila auxiliara **word**
      - Cand intalnim o litera (a-z sau A-Z), o convertim la litera mica si o adaug in **word**
      - Daca intalnesc un caracter de spatiu (` ` sau `\t`), sau ajung la capat de linie,
      iar cuvantul contine macar un caracter, atunci il adaug in **set**-ul de cuvinte unice al fisierului
      (operatia de insert intr-un set garanteaza intotdeauna ca set-ul nu va contine duplicate) 
    - Dupa ce am parcurs fisierul, iterez **set**-ul de cuvinte unice din fisier, si adaug fiecare cuvant,
    alaturi de **ID**-ului fisierului de intrare in rezultatul Mapper-ului,
    avand indexul egal cu **ID**-ul thread-ului curent Mapper 
    - > Inserarea in rezultatul Mapper-uilor nu are nevoie de niciun mutex.
    - > Fiecare thread Mapper isi adauga rezultatele intr-un vector la indecsi diferiti
    - > (indecsii reprezinta ID-urile thread-ulor Mapper) 

- La iesire din bucla (adica dupa terminarea procesarii tuturor fisierelor)
  - Thread-ul curent incrementeaza o variabila partajata (atat thread-urilor Mapper, cat si Reducer),
  care numara cati Mapperi si-au terminat activitatea
  Accesul la aceasta variabila este projat tot printr-un **mutex**.
  Inaintea de a se face incrementarea si o urmatoare comparatie a acestei variabilei,
  thread-ul care incearca sa o acceseze pune **lock** pe **mutex**,
  iar dupa ce se executa sectiune critica de cod, elibereaza **mutex**-ul prin **unlock**
  - Daca toti Mapperii au terminat
    (cu alte cuvinte, daca valoarea acestei variabile partajate este egala cu numarul total de thread-uri Mapper),
    atunci se trimite un semnal **broadcast** printr-o **variabila conditionala** (`pthread_cond_t`),
    care actioneaza ca un fel de *bariera* intre Mapperi si Reduceri
    Cand **variabila conditionala** face **broadcast**, de abia atunci Reduceri isi pot incepe executia




### Thread-urile Reducer


Reduceri rezolva problema producator-consumator, in ipostaza **mai multi producator - mai multi consumatori**:
- Mai intai: mai multi producatori (rezultatele Mapper-ilor)
    si mai multi consumatori (cate un dictionar in WordList pentru fiecare litera din alfabet)
- Apoi: mai multi producatori (cate un dictionar pentru fiecare litera din alfabet)
    si mai multi consumatori (pentru fiecare dictionar: cate un vector si un fisier de iesire)



Un thread Reducer itereaza rezultatele Mapperilor si le concateneaza in dictionare,
grupandu-le in functie de litera cu care cuvintele incep: cate un dictionar pentru fiecare litera din alfabet.
De asemenea, transforma mai apoi, pentru fiecare litera din alfabet,
dictionarul aferent literei intr-un vector,
pe care mai apoi il sorteaza (folosind **std::sort**) si scrie rezultatele in fisierele de iesire.

Un thread Reducer:
- Primeste ca argument **o referinta la o instanta** a clasei **ReducerThread**
  (clasa contine referinte la toate variabilele partajate de care are thread-ul nevoie)
- Pune **lock** pe **mutex**-ul ce protejeaza variabila pentru numarul de Mapperi finalizati
- Verifica intr-un **if** daca numarul de Mapperi finalizati este egal cu numarul total de Mapperi
  - Daca egalitatea nu este indeplinita, thread-ul este pus sa astepte
  semnalul de **broadcast** (pentru a-si putea incepe executia) de la **variabila conditionala**, cea cu `pthread_cond_t`,
  care actioneaza ca un fel de *bariera* intre thread-urile Mapper si Reducer 
- Da **unlock** la **mutex**-ul mentionat anterior


> Doar un singur thread Reducer asteapta semnalul (primul care ia **lock**-ul de pe **mutex**).
> Restul thread-urilor, in momentul in care se pune **lock** pe mutex,
> numarul de rezultate Mapper completate va fi egal cu numarul total de thread-uri Mapper
> si doar se da **unlock** pe mutex si se trece mai departe. 

Mai apoi:
- Intr-o bucla infinita:
  - Pune **lock** pe **mutex-ul cozii indicilor rezultatelor produse de Mapperi**
  - Daca coada este vida, elibereaza mutex-ul (ii da **unlock**) si iese imediat din bucla
    Altfel, extrage din coada un index al unui rezultat produs de Mapper,
    elibereaza mutex-ul (**unlock**) si:
      - Itereaza fiecare pereche de forma **word - set de fileIDs** din rezultatul Mapper-ului 
      - In functie de prima litera a cuvantului:
        - Blocheaza **mutex**-ul dictionarului acelei literei (pune **lock** pe el)
        - Adauga intrarea din rezultatul Mapper-ului in dictionarului aferent acelei litere:
          - Daca cuvantul nu se afla in dictionar, se adauga cheia lui **word**,
             cu valorile egale cu set-ul de indici de fisiere
          - Altfel, se adauga fiecare element din set-ul rezultatului in set-ul deja existent in dictionar
             (practic, se face reuniunea dintre cele doua set-uri)
          - Cu aceasta idee, WordList-ul este construit grupand cuvintele in functie de litera cu care incep
        - La final, thread-ul elibereaza **mutex**-ul care protejeaza accesul la dictionarul acelei litere
          (ii da **unlock** la **mutex**)


> Fiecare litera are propriul ei mutex (26 mutex-uri) - un array de mutex-uri.
> Astfel, permite a doua sau mai multe thread-uri sa insereze simultan
> la indecsi diferite (de litere) in vectorul de dictionare.
>
> Asa...daca am fi avut un singur mutex pentru intreg vectorul de dictionare,
> cand un thread ar fi vrut sa adauge in dictionar la o anumita litera,
> ar fi pus lock pe mutex,
> iar toate celelalte thread-uri ar fi fost fortate sa astepte dupa ele
> (chair daca insereaza total alta zona de memorie)
> ... si practic... nu ar fi fost niciun paralelism.


Iar, in final:
- La iesirea din bucla, am folosit o **bariera**
  pentru a delimita partea de constructie a dictionarelolor pentru fiecare litera
  cu partea de convertire a dictionarelor in liste, sortarea si scrierea lor in fisierele de iesire.
  - Bariera asteapta toti Reduceri,
  drept pentru care este initialaza cu numarul de thread-uri reduceri.
  - **TL;DR**: Aceasta asigura ca toate dictionarele din WordList au fost construite inainte de a trece mai departe
  - > Am nevoie de bariera, pentru a impune ca toate dictionarele sa fie construite in totalitate inainte sa fie sortate si scrise in fisiere.
- Mai repet inca o data logica cu alta coada (da data aceasta coada retine indecsi literelor din aflabet),
  pentru a converti dictioanerele la vectori, a sorta vectorii si a-i scrie in fisierele de iesire:
  - Intr-o bucla infinita:
    - Thread-ul pune **lock** pe mutex-ul care protejeaza **coada cu indexul literelor din alfabet**
      - Extrage un index (al unei litere) si da imediat **unlock**
      - Daca **coada** este vida, iese din bucla
    - Pentru indexul (literei) extras din coada, thread-ul curent face urmatoarele lucruri:
      - Converteste dictionarul din WordList aferent literei la un vector
      - Sorteaza vectorul construit mai inaite:
        - Crescator dupa numarul de fisiere (lungimea set-ului fileID-urilor) in care cuvantul apare
        - Alfabetic dupa cuvant
      - Scrie elementele vectorului in fisierul text de iesire asociat literei respective

### Elemente de sincronizare folosite

- Multe mutex-uri (array-uri chiar, acolo unde este cazul) - pentru a proteja accesul la variabilele partajate
- O variabila conditionala
  - Semnaleaza unui thread Reducer ca toate thread-urile Mapper si-au finalizat task-urile
  - Actioneaza ca un fel de *"bariera"* intre thread-urile Mapper si Reducer
- O bariera (in WordList) - impune ca toate dictionarele fiecarei litere sa fie pe deplin construite inainte de a trece mai departe
  (cu convertirea la vector, sortarea si scrierea in fisiere)


## Main


Main-ul se ocupa exclusiv de:
- Citirea argumentelor din linia de comanda
- Citirea numelor (path-urilor) fisierelor de intrare pentru Mapperi
- Crearea a argv[1] + argv[2] thread-uri:
  - Primele argv[1] vor fi thread-uri Mapper:
    se referentieaza o instanta la clasa **MapperThread** si se da ca argument functiei thread-ului
  - Urmatoarele argv[2] thread-uri vor fi Reduceri: la fel, doar ca pt clasa **ReducerThread**
- Imbinarea thread-urilor 


