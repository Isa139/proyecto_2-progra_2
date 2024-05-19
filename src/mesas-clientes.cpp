#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <ctime>
#include <string>
#include <queue>
#include <mutex>
#include <vector>
#include <functional> 

using namespace std;

#define SPAWN_RATE 2
#define MIN_PREP_TIME 20
#define MIN_EATING_TIME 10
#define MIN_TIME 10
#define MAX_CUSTOMERS_PER_TABLE 6
#define TABLE_COUNT 6

// Initialize the random seed once
std::random_device rd;
std::mt19937 gen(rd());

int delay(int time);

// Estructuras y clases:
struct Meal {
    string name;
    int cookingDuration;
};

class Customer {
public:
    string name;
    Customer(string name) : name(name) {}
    void orderMeal();
    void eatMeal(int durationTime);
};

class Chef {
public:
    Chef() {}
    void cookMeal(int preparationTime);
};

class Waiter {
public:
    Waiter() {}
    void checkStock();
    void informOutOfStock();
    void deliverMeal();
};

class Party {
public:
    int size;
    vector<Customer*> customers;

    Party(int size, int& customerCounter) : size(size) {
        for (int i = 0; i < size; ++i) {
            customers.push_back(new Customer("Customer" + to_string(customerCounter++)));
        }
    }

    ~Party() {
        for (Customer* customer : customers) {
            delete customer;
        }
    }
};

class Table {
private:
    int number;
    bool occupied;
    Party* currentParty;
    mutex mtx;

public:
    Table(int num) : number(num), occupied(false), currentParty(nullptr) {}

    void simulateEating() {
        if (currentParty) {
            printf("Table %i is now serving a party of %i customers.\n", number, currentParty->size);
            //cout << "Table " << number << " is now serving a party of " << currentParty->size << " customers.\n";
            for (int i = 0; i < currentParty->size; ++i) {
                cout << " - " << currentParty->customers[i]->name << endl;
                cout << "Customer " << currentParty->customers[i]->name << "has begun eating.\n";
                int eatingTime = delay(1);
                printf("They will last %i seconds.\n", eatingTime);
                eatMeal(eatingTime);
                cout << "Customer " << currentParty->customers[i]->name << "is done eating.\n";
            }
            //this_thread::sleep_for(chrono::seconds(delay(MIN_EATING_TIME)));
            freeTable();
        }
    }

    void assignParty(Party* party) {
        lock_guard<mutex> lock(mtx);
        currentParty = party;
        occupied = true;
        simulateEating();
    }

    void freeTable() {
        lock_guard<mutex> lock(mtx);
        occupied = false;
        currentParty = nullptr;
        cout << "Table " << number << " is now free.\n";
    }

    bool isOccupied() {
        lock_guard<mutex> lock(mtx);
        return occupied;
    }

    void manageTables(queue<Party*>& customerQueue, mutex& queueMutex) {
        while (true) {
            Party* party = nullptr;
            {
                lock_guard<mutex> lock(queueMutex);
                if (!customerQueue.empty() && !isOccupied()) {
                    party = customerQueue.front();
                    customerQueue.pop();
                }
            }

            if (party) {
                assignParty(party);
            } else {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
    }
};

class Restaurant {
private:
    queue<Party*> customerQueue;
    Table* tables[TABLE_COUNT];
    mutex queueMutex;
    int customerCounter;

public:
    Restaurant() {
        customerCounter = 1;
        for (int i = 0; i < TABLE_COUNT; ++i) {
            tables[i] = new Table(i + 1);
        }
    }

    ~Restaurant() {
        for (int i = 0; i < TABLE_COUNT; ++i) {
            delete tables[i];
        }
    }

    void addPartyToQueue(Party* party) {
        lock_guard<mutex> lock(queueMutex);
        customerQueue.push(party);
    }

    void assignTables() {
        while (true) {
            for (int i = 0; i < TABLE_COUNT; ++i) {
                Party* party = nullptr;
                {
                    lock_guard<mutex> lock(queueMutex);
                    if (!tables[i]->isOccupied() && !customerQueue.empty()) {
                        party = customerQueue.front();
                        customerQueue.pop();
                    }
                }
                if (party) {
                    tables[i]->assignParty(party);
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    void simulation() {
        vector<thread> tableThreads;
        for (int i = 0; i < TABLE_COUNT; ++i) {
            tableThreads.push_back(thread(&Table::manageTables, tables[i], ref(customerQueue), ref(queueMutex)));
        }

        for (auto& th : tableThreads) {
            th.detach();
        }

        assignTables();
    }

    Party* createParty(int size) {
        return new Party(size, customerCounter);
    }
};

int delay(int time) {
    int delay = rand() % 6 + 1;
    int totalTime = time + delay;

    if (totalTime <= 0) {
        return MIN_EATING_TIME;
    } else {
        return totalTime;
    }
}

void runRestaurant() {
    srand(static_cast<unsigned int>(time(0)));
    Restaurant restaurant;
    thread restaurantThread(&Restaurant::simulation, &restaurant);

    while (true) {
        int partySize = rand() % 6 + 1;
        restaurant.addPartyToQueue(restaurant.createParty(partySize));
        this_thread::sleep_for(chrono::seconds(SPAWN_RATE));
    }

    restaurantThread.join();
}

void eatMeal(int eatingTime) {
    int realTime = delay(eatingTime);
    std::this_thread::sleep_for(std::chrono::seconds(realTime)); 
}

int main() {
    runRestaurant();
    return 0;
}
