#include <iostream>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

const int SPAWN_RATE = 1; // time between each random number push

void pushRandomNumber(std::queue<int>& q) {
    int randomNumber = rand() % 6 + 1;
    q.push(randomNumber);

    printf("Pushed: %d\n", randomNumber);
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr))); 
    std::queue<int> q;

    while (true) {
        pushRandomNumber(q);
        std::this_thread::sleep_for(std::chrono::seconds(SPAWN_RATE));
    }

    return 0;
}
