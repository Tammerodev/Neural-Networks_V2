#pragma once
#include <algorithm>
#include <random>


float randFloat() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int randomInt(int x) {
    return std::rand() % (x);
}

float distance(int x1, int y1, int x2, int y2)
{
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}