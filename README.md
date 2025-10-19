![header](https://capsule-render.vercel.app/api?type=waving&height=230&color=gradient&customColorList=24&text=Glossario%20Thread&textBg=false&fontAlignY=40&reversal=false&animation=fadeIn)

## Thread (`thread`)

Un **thread** è un'unità di esecuzione che può essere gestita in parallelo all'interno di un programma. In C++, è possibile creare e gestire thread tramite la libreria `<thread>`.

### Funzioni principali:

- **`thread`**  
  Classe che rappresenta un thread di esecuzione; il suo costruttore può essere chiamato per istanziare un thread.
  
  ```cpp
  #include <iostream>
  #include <thread>

  using namespace std;

  void function() { ... }
  
  int main() {
      thread t1(function);
  }
  ```

- **`thread::join`**  
  Blocca il thread chiamante e attende che il thread chiamato termini la sua esecuzione.

  ```cpp
  #include <iostream>
  #include <thread>

  using namespace std;

  void function() { ... }
  
  int main() {
      thread t1(function);
  
      t1.join();
  }
  ```

- **`thread::detach`**  
  Rilascia il thread consentendo la sua esecuzione indipendente.

  ```cpp
  #include <iostream>
  #include <thread>

  using namespace std;

  void function() { ... }
  
  int main() {
      thread t1(function);
  
      t1.detach();
  }
  ```

- **`thread::joinable`**  
  Verifica se un thread può essere “agganciato” (joined).

  ```cpp
  #include <iostream>
  #include <thread>

  using namespace std;

  void function() { ... }

  int main() {
      thread t1(function);

      if (t1.joinable()) {
          t1.join();
      }
  }
  ```

---

## Mutex (`std::mutex`)

Un **mutex** (mutual exclusion) è un meccanismo utilizzato per impedire che più thread accedano contemporaneamente a una risorsa condivisa.

### Funzioni più usate:

- **`lock()`**  
  Blocca il mutex. Se è già bloccato da un altro thread, attende.

  ```cpp
  mutex m;
  m.lock();
  ```

- **`unlock()`**  
  Sblocca il mutex permettendo ad altri thread di accedervi.

  ```cpp
  m.unlock();
  ```

- **`try_lock()`**  
  Prova a bloccare il mutex senza attendere. Restituisce `true` se il blocco ha successo.

  ```cpp
  if (m.try_lock()) {
      // sezione critica
      m.unlock();
  }
  ```

---

## Semaphore (`std::counting_semaphore`)

Un **semaforo** è una struttura usata per controllare l’accesso a una risorsa condivisa tramite un contatore.

### Funzioni più usate:

- **`acquire()`**  
  Decrementa il contatore. Se è zero, il thread resta in attesa.

  ```cpp
  #include <semaphore>
  using namespace std;

  counting_semaphore<1> sem(1);

  int main() {
      sem.acquire();
      // sezione critica
  }
  ```

- **`release()`**  
  Incrementa il contatore e sblocca eventuali thread in attesa.

  ```cpp
  sem.release();
  ```

- **`try_acquire()`** *(se disponibile)*  
  Prova ad acquisire il semaforo senza bloccare. Restituisce `true` se riesce.

  ```cpp
  if (sem.try_acquire()) {
      // operazioni consentite
      sem.release();
  }
  ```
