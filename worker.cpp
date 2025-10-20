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

int N_TASK = 3;
int N_WORKERS = 1;
int MAX_TASK_TIME = 3;

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

        Task(int i) : id(i){}
};

// ----- CLASSE WORKER -----
class Worker {
    private: 
        queue<Task>& codaTask;
        int id;
        thread workerThread;
        atomic<bool>& stop;
        vector<thread> taskThreads;

    public:
        // Costruttore
        Worker(queue<Task>& q, int id, atomic<bool>& stopFlag): codaTask(q), id(id), stop(stopFlag) {
            workerThread = thread(&Worker::ElaboraTask, this);  // Avvio il thread del worker nel costruttore
        }

        // Distruttore (Joina il thread del worker)
        ~Worker() {
            if (workerThread.joinable()) {
                workerThread.join();
            }
        }

        // Metodo per elaborare i task
        void ElaboraTask() {
            while(true) {

                Task task(0); // task di default

                // Preleva la task dalla coda
                {
                    lock_guard<mutex> lock(memoryMutex);
                    if (!codaTask.empty()) {
                        task = codaTask.front();
                        codaTask.pop();
                    } else if(stop) {
                        break;
                    }
                }


                {
                    lock_guard<mutex> lock(outputMutex);
                    cout << "[WORKER #" << id << "] Ricevuta task con ID: " << task.id << ", Tempo di esecuzione: " << task.tempoEsecuzione << endl;
                }

                // Creazione dei threads per l'elaborazione dei task
                taskThreads.push_back(thread([this, task]() {
                    this_thread::sleep_for(chrono::seconds(task.tempoEsecuzione));
                    lock_guard<mutex> lock(outputMutex);
                    cout << "[WORKER #" << id << "] Terminata task con ID: " << task.id << endl;
                }));
            }

            // Joina tutti i thread attivi che ha creato
            for(auto& t : taskThreads) {
                if(t.joinable())
                    t.join();
            }
        }
};

// ----- CLASSE MASTER -----
class Master {
    private:
        queue<Task> codaTask;
        vector<unique_ptr<Worker>> workers;
        atomic<bool> stopFlag{false};
    public:
        Master() {}

        void generaTasks() {
            for(int i = 0; i < N_TASK; i++) {
                lock_guard<mutex> lock(memoryMutex);
                codaTask.emplace(i); // Creo la Task direttamente nella coda
                cout << "[MASTER] Generato task con ID: " << i << ", Tempo di esecuzione: " << codaTask.back().tempoEsecuzione << endl;
            }
        }

        void generaWorkers() {
            for(int i = 0; i < N_WORKERS; i++) {
                workers.emplace_back(make_unique<Worker>(codaTask, i, stopFlag)); // Crea un nuovo worker e lo aggiunge al vettore
            }
        }

        void execute() {
            generaTasks();
            this_thread::sleep_for(chrono::seconds(3));
            cout << "\n-------------------------------------------\n\n";
            generaWorkers();

            // Ciclo per verificare se ci sono task da elaborare, in caso contrario distruggi i workers
            while(!stopFlag) {
                lock_guard<mutex> lock(memoryMutex);
                if(codaTask.empty()) {
                    cout << "[MASTER] Tutte le tasks elaborate" << endl;
                    stopFlag = true;
                }
                this_thread::sleep_for(chrono::seconds(1)); // Controlla ogni secondo per evitare busy waiting
            }

        }
};

int main() {

    Master master;

    master.execute();

    return 0;
}
