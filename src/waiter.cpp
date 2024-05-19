#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <string>
#include <queue>
#include <mutex>

using namespace std;

#define SPAWN_RATE 4
#define MIN_PREP_TIME 20
#define MIN_EATING_TIME 10
#define MIN_TIME 10
#define MAX_CUSTOMERS_PER_TABLE 6
#define TABLE_COUNT 6

int delay(int time);

int delay(int time) { // tiempo aleatorio para preparacion de comida y liberacion de mesa
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int delay = rand() % 6 + 1;
    int totalTime = time + delay;

    if (totalTime <= 0) {
        return MIN_EATING_TIME;
    } else {
        return totalTime;
    }
}

// Estructuras y clases:
struct Meal {
    string name;
    int cookingDuration;
}; 

struct Order {
    string customerName;
    string mealName;

    Order(string customerName, string mealName) : customerName(customerName), mealName(mealName) {}
};

class Customer {
private:
    string selectedMeal; // Plato seleccionado para este cliente

public:
    string name;
    Customer(string name, string meal) : name(name), selectedMeal(meal) {}

    string getMeal() {
        return selectedMeal;
    }

    void orderMeal() {
        // imprime el plato elegido por el cliente
        cout << " -" << name << " ordered: " << selectedMeal << endl;
    }

    void eatMeal(int durationTime);
};

class Chef {
private:
public:
    Chef();
    void cookMeal(int preparationTime);
};

class Waiter {
private:
    queue<Order>* orderQueue;
    mutex* orderQueueMutex;
public:
    Waiter(queue<Order>* orderQueue, mutex* orderQueueMutex) : orderQueue(orderQueue), orderQueueMutex(orderQueueMutex) {}

    void takeOrder(Customer* customer) {
        lock_guard<mutex> lock(*orderQueueMutex);
        Order order(customer->name, customer->getMeal());
        orderQueue->push(order);
        cout << "Waiter took order from " << customer->name << " for " << customer->getMeal() << endl;
    }
};

class Party {
public:
    int size;
    Customer* customers[MAX_CUSTOMERS_PER_TABLE];

    Party(int size, int& customerCounter) : size(size) {
        // array de platos en el menu
        string menu[] = {"Cinnamon Rolls", "Apple Pie", "Cookies", "Chocolate Cupcakes", "Donuts", "Croissant", "Brownies", "Carrot Cake", "Caramel Flan", "Banana Muffins"};
        
        for (int i = 0; i < size; ++i) {
            int choice = rand() % 10; // elige plato aleatoriamente
            string selectedMeal = menu[choice];

            customers[i] = new Customer("Customer" + to_string(customerCounter++), selectedMeal); // crear cada obj customer, nombre: counterCount, selecciona su plato
        }
    }
};

class Table {
private:
    int number;
    int capacity;
    bool occupied;
    Party* currentParty;
    mutex mtx;
    Waiter* waiter;

public:
    Table(int num, Waiter* waiter) : number(num), capacity(MAX_CUSTOMERS_PER_TABLE), occupied(false), currentParty(nullptr), waiter(waiter) {}

    void simulateEating() {
        if (currentParty) {
            cout << "Table " << number << " is now serving a party of " << currentParty->size << " customers.\n";
            for (int i = 0; i < currentParty->size; ++i) {
                currentParty->customers[i]->orderMeal(); // cada cliente de la mesa hace pide su plato
                waiter->takeOrder(currentParty->customers[i]); // el mesero toma la orden de cada cliente
            }
            thread releaseThread(&Table::freeTable, this); // crear un hilo para liberar la mesa una vez que el grupo haya terminado de comer
            releaseThread.detach(); // lo hace un hilo independiente
            this_thread::sleep_for(chrono::seconds(delay(MIN_EATING_TIME)));
        }
    }

    void assignParty(Party* party) {
        lock_guard<mutex> lock(mtx); // crea un objeto lock_guard que bloquea el mutex mtx asociado a la mesa actual
        currentParty = party; // asigna el party a la mesa
        occupied = true; // la mesa ahora esta ocupada
        simulateEating();
    }

    void freeTable() { // libera la mesa
        lock_guard<mutex> lock(mtx);
        occupied = false;
        currentParty = nullptr;
        cout << "Table " << number << " is now free.\n";
    }

    bool isOccupied() { // delvuelve el estado ed la mesa
        lock_guard<mutex> lock(mtx);
        return occupied;
    }

    void manageTables(queue<Party*>& customerQueue, mutex& queueMutex) { // asignacion de grupos de clientes a las mesas
        while (true) {
            Party* party = nullptr;
            {
                lock_guard<mutex> lock(queueMutex);
                if (!customerQueue.empty() && !isOccupied()) { // verifica disponibilidad de mesa y que existan clientes en la cola 
                    party = customerQueue.front(); // primer grupo de clientes de la cola
                    customerQueue.pop();
                }
            }

            if (party) { // se asigna el grupo a una mesa
                assignParty(party);
            }
            else {
                this_thread::sleep_for(chrono::milliseconds(100)); // si no hay clientes, espera un momento y despues vuelve a revisar
            }
        }
    }
};

class Restaurant {
private:
    queue<Party*> customerQueue; // cola de clientes: cada elemento es un puntero a un objeto de la clase Party
    queue<Order> orderQueue; // cola de órdenes
    Table* tables[TABLE_COUNT];
    mutex queueMutex; // asegurarse de que ningun otro hilo acceda a la cola en ese momento
    mutex orderQueueMutex; // asegurarse de que ningun otro hilo acceda a la cola de órdenes
    int customerCounter;  // contador de clientes
public:
    Restaurant() { // constructor: inicializar los obj table del restaurante
        customerCounter = 1; // inicializar contador en 1
        for (int i = 0; i < TABLE_COUNT; ++i) {
            tables[i] = new Table(i + 1, new Waiter(&orderQueue, &orderQueueMutex));
        }
    }

    void addPartyToQueue(Party* party) {
        lock_guard<mutex> lock(queueMutex);
        customerQueue.push(party); // coloca al grupo de clientes al final de la cola
    }

    void assignTables() {
        while (true) {
            for (int i = 0; i < TABLE_COUNT; ++i) {
                if (!tables[i]->isOccupied()) { // itera sobre las mesas disponibles
                    Party* party = nullptr;
                    {
                        lock_guard<mutex> lock(queueMutex);
                        if (!customerQueue.empty()) {
                            party = customerQueue.front(); // primer grupo en la cola para ser atentido
                            customerQueue.pop(); // eliminar de la cola
                        }
                    }
                    if (party) {
                        tables[i]->assignParty(party); // asignar a la mesa
                    }
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    void simulation() { // inicia la simulacion del restaurante
        thread tableThreads[TABLE_COUNT]; // arreglo de hilos
        for (int i = 0; i < TABLE_COUNT; ++i) { // crear un hilo por mesa
            tableThreads[i] = thread(&Table::manageTables, tables[i], ref(customerQueue), ref(queueMutex));
        }

        for (int i = 0; i < TABLE_COUNT; ++i) { // permite que los hilos se ejecuten de forma independiente al hilo principal
                    tableThreads[i].detach();
        }

        assignTables();
    }

    Party* createParty(int size) {
        return new Party(size, customerCounter);
    }
};

// Otras funciones
void runRestaurant() { // simulacion general del restaurante
    srand(static_cast<unsigned int>(time(0)));
    Restaurant restaurant;
    thread restaurantThread(&Restaurant::simulation, &restaurant);

    // simular la llegada de grupos de clientes al restaurante
    while (true) {
        int partySize = rand() % 6 + 1;
        restaurant.addPartyToQueue(restaurant.createParty(partySize));
        this_thread::sleep_for(chrono::seconds(SPAWN_RATE));
    }

    restaurantThread.join();
}

int main() {
    runRestaurant();
    return 0;
}
