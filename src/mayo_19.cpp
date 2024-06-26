#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <string>
#include <queue>
#include <mutex>
#include "ActualizaInventario.cpp"

using namespace std;

#define SPAWN_RATE 5
#define MIN_PREP_TIME 20
#define MIN_EATING_TIME 10
#define MIN_TIME 10
#define MAX_CUSTOMERS_PER_TABLE 6
#define TABLE_COUNT 6

int delay(int time);
struct recipe;


int binaryRandom()
{
    if (std::rand() % 2 == 0)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

int randomRange(int min, int max)
{
    return min + (std::rand() % (max - min + 1));
}

void eatMeal(int eatingTime)
{
    int realTime = delay(eatingTime);
    std::this_thread::sleep_for(std::chrono::seconds(realTime));
}

// Estructuras y clases:
struct Meal
{
    string name;
    int cookingDuration;
};

struct Order
{
    string customerName;
    string mealName;

    Order(string customerName, string mealName) : customerName(customerName), mealName(mealName) {}
};

class Customer
{
private:
    string selectedMeal; // Plato seleccionado para este cliente
public:
    string name;
    char status;
    // S: Seated
    // O: ordered
    // W: waiting
    // E: eating
    // D: Done
    Customer(string name, string meal) : name(name), selectedMeal(meal) {}

    string getMeal()
    {
        return selectedMeal;
    }

    void orderMeal()
    {
        // imprime el plato elegido por el cliente
        cout << " -" << name << " ordered: " << selectedMeal << endl;
    }
    void eatMeal(int durationTime);
};

class Chef
{
private:
public:
    Chef();

    int cookMeal(int preparationTime)
    {
    int calculatePreparationTime(int preparationTime);
    int randomFactor = binaryRandom();
    int prepDelay = delay(preparationTime);
    int randomRangeValue = randomRange(0, prepDelay);

    int calculatedTime = preparationTime + (preparationTime * randomFactor * randomRangeValue);

    if (calculatedTime < MIN_PREP_TIME)
    {
        calculatedTime = MIN_PREP_TIME;
    }
    return calculatedTime;
    }
};

class Waiter
{
private:
    queue<Order> *orderQueue;
    mutex *orderQueueMutex;

public:
    Waiter(queue<Order> *orderQueue, mutex *orderQueueMutex) : orderQueue(orderQueue), orderQueueMutex(orderQueueMutex) {}

    void takeOrder(Customer *customer)
    {
        lock_guard<mutex> lock(*orderQueueMutex);
        Order order(customer->name, customer->getMeal());
        orderQueue->push(order);
        cout << "Waiter took order from " << customer->name << " for " << customer->getMeal() << endl;
    }

    void checkStock();
    void informOutOfStock();
    void deliverMeal();
};

class Party
{
public:
    int size;
    Customer *customers[MAX_CUSTOMERS_PER_TABLE];

    Party(int size, int &customerCounter) : size(size)
    {
       
    }
};

class Table
{
private:
    int number;
    int capacity;
    bool occupied;
    Party *currentParty;
    condition_variable cv;
    mutex mtx;
    Waiter *waiter;

public:
    Table(int num, Waiter *waiter) : number(num), capacity(MAX_CUSTOMERS_PER_TABLE), occupied(false), currentParty(nullptr), waiter(waiter) {}

    void simulateEating()
    {
        if (currentParty)
        {
            printf("Table %i is now serving a party of %i customers.\n", number, currentParty->size);
            for (int i = 0; i < currentParty->size; ++i)
            {
                currentParty->customers[i]->status = 'S';
                cout << "Customer " << currentParty->customers[i]->name << "'s status is" << currentParty->customers[i]->status << "\n";
                currentParty->customers[i]->orderMeal(); // cada cliente de la mesa pide su plato
                waiter->takeOrder(currentParty->customers[i]);
                currentParty->customers[i]->status = 'W';
                cout << "Customer " << currentParty->customers[i]->name << "'s status is" << currentParty->customers[i]->status << "\n";
                cout << currentParty->customers[i]->name << " has begun eating.\n";
                currentParty->customers[i]->status = 'E';
                cout << "Customer " << currentParty->customers[i]->name << "'s status is" << currentParty->customers[i]->status << "\n";
                int eatingTime = delay(1);
                printf("They will last %i seconds.\n", eatingTime);
                eatMeal(eatingTime);
                cout << "Customer " << currentParty->customers[i]->name << " is done eating.\n";
                currentParty->customers[i]->status = 'D';
                cout << "Customer " << currentParty->customers[i]->name << "'s status is" << currentParty->customers[i]->status << "\n";
            }
            // this_thread::sleep_for(chrono::seconds(delay(MIN_EATING_TIME)));
            // freeTable();

            thread releaseThread(&Table::freeTable, this); // crear un hilo para liberar la mesa una vez que el grupo haya terminado de comer
            releaseThread.detach();                        // lo hace un hilo independiente
        }
    }

    void assignParty(Party *party)
    {
        lock_guard<mutex> lock(mtx); // crea un objeto lock_guard que bloquea el mutex mtx asociado a la mesa actual
        currentParty = party;        // asigna el party a la mesa
        occupied = true;             // la mesa ahora esta ocupada
        simulateEating();
    }

    void checkAndFreeTable()
    {
        unique_lock<mutex> lock(mtx);
        while (true)
        {
            cv.wait(lock, [this]()
                    {
                for (int i = 0; i < currentParty->size; ++i) {
                    if (currentParty->customers[i]->status != 'D') {
                        return false;
                    }
                }
                return true; });

            freeTable();
            return;
        }
    }

    void freeTable()
    { // libera la mesa
        lock_guard<mutex> lock(mtx);
        occupied = false;
        currentParty = nullptr;
        cout << "Table " << number << " is now free.\n";
    }

    bool isOccupied()
    { // delvuelve el estado ed la mesa
        lock_guard<mutex> lock(mtx);
        return occupied;
    }

    void manageTables(queue<Party *> &customerQueue, mutex &queueMutex)
    { // asignacion de grupos de clientes a las mesas
        while (true)
        {
            Party *party = nullptr;
            {
                lock_guard<mutex> lock(queueMutex);
                if (!customerQueue.empty() && !isOccupied())
                {                                  // verifica disponibilidad de mesa y que existan clientes en la cola
                    party = customerQueue.front(); // primer grupo de clientes de la cola
                    customerQueue.pop();
                }
            }

            if (party)
            { // se asigna el grupo a una mesa
                assignParty(party);
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(100)); // si no hay clientes, espera un momento y despues vuelve a revisar
            }
        }
    }
};

class Restaurant
{
private:
    queue<Party *> customerQueue; // cola de clientes: cada elemento es un puntero a un objeto de la clase Party
    queue<Order> orderQueue;      // cola de órdenes
    Table *tables[TABLE_COUNT];
    mutex queueMutex;      // asegurarse de que ningun otro hilo acceda a la cola en ese momento
    mutex orderQueueMutex; // asegurarse de que ningun otro hilo acceda a la cola de órdenes
    int customerCounter;   // contador de clientes
public:
    Restaurant()
    {                        // constructor: inicializar los obj table del restaurante
        customerCounter = 1; // inicializar contador en 1
        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i] = new Table(i + 1, new Waiter(&orderQueue, &orderQueueMutex));
        }
    }

    void addPartyToQueue(Party *party)
    {
        lock_guard<mutex> lock(queueMutex);
        customerQueue.push(party); // coloca al grupo de clientes al final de la cola
    }

    void assignTables()
    {
        while (true)
        {
            for (int i = 0; i < TABLE_COUNT; ++i)
            {
                if (!tables[i]->isOccupied())
                { // itera sobre las mesas disponibles
                    Party *party = nullptr;
                    {
                        lock_guard<mutex> lock(queueMutex);
                        if (!customerQueue.empty())
                        {
                            party = customerQueue.front(); // primer grupo en la cola para ser atentido
                            customerQueue.pop();           // eliminar de la cola
                        }
                    }
                    if (party)
                    {
                        tables[i]->assignParty(party); // asignar a la mesa
                    }
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    void simulation()
    {                                     // inicia la simulacion del restaurante
        thread tableThreads[TABLE_COUNT]; // arreglo de hilos
        for (int i = 0; i < TABLE_COUNT; ++i)
        { // crear un hilo por mesa
            tableThreads[i] = thread(&Table::manageTables, tables[i], ref(customerQueue), ref(queueMutex));
        }

        for (int i = 0; i < TABLE_COUNT; ++i)
        { // permite que los hilos se ejecuten de forma independiente al hilo principal del programa
            tableThreads[i].detach();
        }

        assignTables();
    }

    Party *createParty(int size)
    {
        return new Party(size, customerCounter);
    }
};

// otras funciones
int delay(int time)
{ // tiempo aleatorio para preparacion de comida y liberacion de mesa
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int delay = rand() % 6 + 1;
    int totalTime = time + delay;

    if (totalTime <= 0)
    {
        return MIN_EATING_TIME;
    }
    else
    {
        return totalTime;
    }
}

void runRestaurant()
{ // simulacion general del restaurante
    srand(static_cast<unsigned int>(time(0)));
    Restaurant restaurant;
    thread restaurantThread(&Restaurant::simulation, &restaurant);

    // simular la llegada de grupos de clientes al restaurante
    while (true)
    {
        int partySize = rand() % 6 + 1;
        restaurant.addPartyToQueue(restaurant.createParty(partySize));
        this_thread::sleep_for(chrono::seconds(SPAWN_RATE));
    }

    restaurantThread.join();
}

int main()
{
    runRestaurant();
    
    return 0;
}
