#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <chrono>

using namespace std;

int N_TASK = 30; // Numero di task totali
int N_WORKERS = 1; // Numero di worker che lavorano sulla coda
mutex mut;
mutex printmut;


// ----- GENERATORE NUMERI RANDOM ----- //
int random() {
    static bool inizializzato = false;
    if (!inizializzato) {
        srand(time(nullptr));   // Inizializza il generatore SOLO la prima volta
        inizializzato = true;
    }
    return (rand() % 4) + 1;     // Restituisce un numero tra 1 e 4
}


// ----- CLASSE TASK ----- //
class Task {
public:
    int id;

    // Costruttore che inizializza l'ID
    Task(int id_) : id(id_) {}
};

// ----- CLASSE WORKER ------ //
class Worker {
// Puntatore alla coda condivisa
private:
    queue<Task>* taskQueue;
    int id;
    vector<thread> taskThreads;
    
public:

    // Costruttore
    Worker(queue<Task>* queue, int id_) : taskQueue(queue), id(id_) {}
    

    // Metodo per controllare se ci sono task nella coda
    bool controllaTask() {
        
        mut.lock();

        if(taskQueue->empty()) {
            mut.unlock();
            return false; // Ritorna false se non ci sono task nella coda per interrompere il ciclo nel main
        }

        Task t = taskQueue->front(); // Prende la prima task dalla coda
        taskQueue->pop();            // Toglie la prima Task dalla coda
        mut.unlock();

        elaboraTask(t);              // Elabora il task
        return true;                 // Ritorna true se è stato trovato almeno un task
    }
    
    // Metodo per elaborare il task
    void elaboraTask(Task t) {

        // Thread Lambda per simulare l'elaborazione del task
        taskThreads.emplace_back([this, t](){
            this_thread::sleep_for(chrono::seconds(random())); // Simula un tempo di elaborazione casuale
            lock_guard<mutex> guard(printmut);
            cout << "[WORKER "<< id << "] elaborato task ["<<t.id<<"]\n";
        });                  
    }

    void joinAll() {
        for(auto& t : taskThreads) {
            if(t.joinable())
                t.join();
        }
    }
    
};

class Master {
private:
    queue<Task>* taskQueue;
public:
    vector<Worker*> workers;
    Master(queue<Task>* q) : taskQueue(q) {}

    void generaTasks() {
        for(int i = 0; i < N_TASK; i++) {
            taskQueue->push(Task(i)); // Aggiungi task con ID i
        }
    }

    void generaWorkers() {
        for(int i = 0; i < N_WORKERS; i++) {
            workers.push_back(new Worker(taskQueue,i));
        }
    }
};

// ----- MAIN ----- //
int main() {
    queue<Task> tasks;      // Coda condivisa

    Master master(&tasks);  // Creazione dell'oggetto Master

    master.generaTasks();   // Genera i task
    master.generaWorkers(); // Genera gli worker

    // Avvia i thread degli worker
    vector<thread> threads;
    for(auto& w : master.workers){
        threads.emplace_back([w]() {
            while(w->controllaTask());
        });
    }

    // Join di tutti i thread degli worker
    for(auto& t : threads) {
        if(t.joinable())
            t.join();
    }

    // Join di tutti i thread di elaborazione dei task dei worker
    for(auto& w : master.workers) {
        w->joinAll();
    }

    return 0;
}
