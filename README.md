![header](https://capsule-render.vercel.app/api?type=waving&height=230&color=gradient&customColorList=24&text=Glossario%20Thread&textBg=false&fontAlignY=40&reversal=false&animation=fadeIn)

## Thread (`thread`)

Un **thread** è un'unità di esecuzione che può essere gestita in parallelo all'interno di un programma. In C++, è possibile creare e gestire thread tramite la libreria `<thread>`.

### Funzioni principali:
- **`thread`**: Classe che rappresenta un thread di esecuzione; Il suo costruttore può essere chiamato per istanziare un thread.
  
  ```cpp
  #include <iostream>
  #include <thread>

  using namespace std;

  void function() { ... }
  
  int main() {
      thread t1(function);
  }
  ```
  
- **`thread::join`**`: Blocca il thread chiamante e attenda che il thread chiamato termini la sua esecuzione.

  ```cpp
  #include <iostream>
  #include <thread>

  using namespace std;

  void function() { ... }
  
  int main() {
      thread t1(function);
  
      thread.join(t1);
  }
  ```

- **`thread::detach`**`: Rilascia il thread consentendo la sua esecuzione indipendente
    ```cpp
  #include <iostream>
  #include <thread>

  using namespace std;

  void function() { ... }
  
  int main() {
      thread t1(function);
  
      t1.detach;
  }
  ```

