#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <chrono>
#include <future>
#include <atomic>

using namespace std;

// ----- PARAMETRI DI CONFIGURAZIONE ----- //
int N_TASK = 100;
int N_WORKERS = 10;
int MAX_TASK_TIME = 10;

mutex memoryMutex, outputMutex; // Mutex per l'accesso ai dati e alla console

// ----- GENERATORE NUMERI RANDOM ----- //
int trandom() {
    static bool inizializzato = false;
    if (!inizializzato) {
        srand(time(nullptr));   // Inizializza il generatore SOLO la prima volta
        inizializzato = true;
    }
    return (rand() % MAX_TASK_TIME) + 1;
}

// ----- CLASSE TASK ----- 
class Task {
public:
    int id;
    int tempoEsecuzione = trandom();

    Task(int i) : id(i) {}
};

// ----- CLASSE WORKER -----
class Worker {
private: 
    queue<Task>& codaTask; // Coda delle task, passata per riferimento
    int id;                // ID del worker
    thread workerThread;   // Thread del worker, avviato nel costruttore
    atomic<bool>& stop;    // Flag per indicare se il worker deve fermarsi
    promise<string> prom;  // Promise per comunicare il risultato finale

public:
    // Costruttore
    Worker(queue<Task>& q, int id, atomic<bool>& stopFlag)
        : codaTask(q), id(id), stop(stopFlag) {
        workerThread = thread(&Worker::ElaboraTask, this);  // Avvio il thread del worker nel costruttore
    }

    // Distruttore (Joina il thread del worker)
    ~Worker() {
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    // Restituisce il future associato a questa promise
    future<string> getFuture() {
        return prom.get_future();
    }

    // Metodo per elaborare i task
    void ElaboraTask() {
        int count = 0;              // conta quante task ha svolto
        string resultFinale;        // accumula info finali
        bool promiseSet = false;    // evita doppia set_value

        while (true) {
            Task task(0);            // task temporanea
            bool haTask = false;     // Indica se il worker ha prelevato una task

            // Preleva la task dalla coda
            {
                lock_guard<mutex> lock(memoryMutex);
                if (!codaTask.empty()) {
                    task = codaTask.front();
                    codaTask.pop();
                    haTask = true;
                } else if (stop) {
                    break; // esce se non ci sono più task e stop è attivo
                }
            }

            // Se ha una task, procede con l'elaborazione
            if (haTask) {
                {
                    lock_guard<mutex> lock(outputMutex);
                    cout << "[WORKER #" << id << "] Ricevuta task con ID: " << task.id << ", Tempo di esecuzione: " << task.tempoEsecuzione << endl;
                }

                // Simula l'elaborazione con uno sleep e stampa il messaggio di completamento
                this_thread::sleep_for(chrono::seconds(task.tempoEsecuzione));
                {
                    lock_guard<mutex> lock(outputMutex);
                    cout << "[WORKER #" << id << "] Task " << task.id << " completata." << endl;
                }

                count++; // Incrementa il contatore di task svolte
                resultFinale += to_string(task.id) + ", ";

            } else {
                // Evita busy-waiting quando la coda è vuota
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }

        // Restituisce il risultato finale
        if (!promiseSet) {
            string res = "Worker " + to_string(id) + " ha completato " + to_string(count) + " task. Task svolte: " + resultFinale;
            prom.set_value(res);
            promiseSet = true;
        }

        // Messaggio di terminazione del worker
        {
            lock_guard<mutex> lock(outputMutex);
            cout << "[WORKER #" << id << "] Terminato dopo " << count << " task.\n";
        }
    }

    // Metodo per attendere il thread del worker (usato dal Master)
    void join() {
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }
};

// ----- CLASSE MASTER -----
class Master {
private:
    queue<Task> codaTask;                   // Coda delle task
    vector<unique_ptr<Worker>> workers;     // Vettore dei workers
    atomic<bool> stopFlag{false};           // Flag per la terminazione dei worker
    vector<future<string>> futures;         // memorizza i future dei worker
    
public:

    // Costruttore (non passo alcun paramentro nel costruttore perchè non serve)
    Master() {}

    // Metodo per generare le task
    void generaTasks() {
        for (int i = 0; i < N_TASK; i++) {
            lock_guard<mutex> lock(memoryMutex);
            codaTask.emplace(i); // Creo la Task direttamente nella coda

            cout << "[MASTER] Generato task con ID: " << i << ", Tempo di esecuzione: " << codaTask.back().tempoEsecuzione << endl;
        }
    }

    // Metodo per generare i workers
    void generaWorkers() {
        for (int i = 0; i < N_WORKERS; i++) {
            auto worker = make_unique<Worker>(codaTask, i, stopFlag);   // Creo un worker con un puntatore unico
            futures.push_back(worker->getFuture());                     // salvo il future del worker
            workers.push_back(move(worker));                            // Aggiungi il worker al vettore
        }
    }

    // Metodo principale
    void execute() {
        generaTasks();                                                  // Genera le task
        this_thread::sleep_for(chrono::seconds(1));                     // Attendi
        cout << "\n-------------------------------------------\n\n";
        generaWorkers();                                                // Genera i workers

        // Ciclo per verificare se ci sono task da elaborare, in caso contrario distruggi i workers
        while (!stopFlag) {
            {
                lock_guard<mutex> lock(memoryMutex);
                if (codaTask.empty()) {
                    cout << "[MASTER] Tutte le tasks prese in elaborazione" << endl;
                    stopFlag = true;
                }
            }
            this_thread::sleep_for(chrono::seconds(1)); // Controlla ogni secondo per evitare busy waiting
        }
    }

    // Attende che tutti i worker finiscano
    void waitWorkers() {
        cout << "[MASTER] In attesa che tutti i worker terminino...\n";
        for (auto& w : workers) {
            w->join();              // Joina tutti i worker
        }
        cout << "[MASTER] Tutti i worker terminati.\n";
    }
    
    void restituisciRisultati() {
        // Stampa Vettore proms
        cout << "\n========== RISULTATI ==========\n";
        for (auto& f : futures) {
            cout << f.get() << endl;
        }
        cout << "==============================\n";
    }
};

int main() {
    Master master;                  // Crea un oggetto Master
    master.execute();               // Esegue il program
    master.waitWorkers();           // Aspetta la terminazione di tutti i worker
    master.restituisciRisultati();  // Restituisce i risultati tramite i futures
    return 0;
}
