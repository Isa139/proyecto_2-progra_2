#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <ctime> 
#include <random>
#include <string>
#include <queue>
#include <cassert>

using namespace std;

#define SPAWN_RATE 0.5
#define MIN_PREP_TIME 20
#define MIN_EATING_TIME 10
#define MIN_TIME 10

int delay(int time);

//Classes and structures:

struct Meal {
    string name;
    int cookingDuration;
};

class Customer {
    private:
    public: 
        Customer();
        void orderMeal();
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
    public:
        Waiter();
        void checkStock();
        void informOutOfStock();
        void deliverMeal();
};

class Table {
    private:
        int customerCount, number;
        string status; //esto es un string?
    public: 
        Table();
};

class Host {
    private:
    public: 
        Host();
        
};

class Party {
    private: 
        int tableID, quantity;
    public:
        Party();
        void leaveTable();
        void stormOut();

};

class Queue {
    private:
        
    public:
        Queue();
        void add();
        void pop();
};

enum customerStatus {ordering, waiting, eating};
enum mealStatus {cooked, delivered};

//Constructors:

Chef::Chef() {
}

Customer::Customer() {
}

Table::Table() {
}

Waiter::Waiter() {
}

//Methods: 

void Customer::eatMeal(int eatingTime) {
    int realTime = delay(eatingTime);
    std::this_thread::sleep_for(std::chrono::seconds(realTime)); 
}

void Chef::cookMeal(int preparationTime) {
    int realTime = delay(preparationTime);
    printf("Cooking. Estimated time: %i seconds\n", realTime);
    std::this_thread::sleep_for(std::chrono::seconds(realTime)); 
    printf("Done cooking\n");
}

//Other functions:
int delay(int time) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int delay = rand() % 6 + 1;
    int actualTime = time + delay;
    if (actualTime <= 0) {
        printf("Min Time: %d\n", MIN_TIME);
        return MIN_TIME;
    } else {
        printf("Regular time: %d\n", actualTime);
        return actualTime;
    }
}

Table** createTables(int amount) {
    Table** tables = new Table*[amount];
    for (int i = 0; i < amount; ++i) {
        tables[i] = new Table; 
    }
    return tables;
}





int main() {
    printf("Beginning\n");

    Chef chef;

    chef.cookMeal(3);
    chef.cookMeal(2); 

    delay(10);
    delay(20);
    delay(5);
  
    printf("Ended");

    return 0;
}
