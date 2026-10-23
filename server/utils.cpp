#include "utils.hpp"
#include <random>

int generateRandomInt()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dist(10000, 99999);

    return dist(gen);
}