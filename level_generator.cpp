#include "dimensions.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

typedef std::array<char, board_size> level;

template <typename T> void randomizeLevel(level &generated_level, T &random_generator) {

    /*
        If we color the board chess-like with black and white fields,
        every move changes all white blocks to black blocks and vice-versa
    */

    std::array<unsigned int, board_half_size> white_indices, black_indices;
    std::array<char, board_half_size> white_values, black_values;

    for (unsigned int y = 0; y < side; y++) {
        unsigned int starting_white_x = y % 2;
        unsigned int starting_black_x = 1 - y % 2;
        for (unsigned int x_local = 0; x_local < half_side; x_local++) {

            unsigned int index = y * half_side + x_local;

            // white_indices[index] =
            white_indices[index] = y * side + starting_white_x + x_local * 2;
            white_values[index] = static_cast<char>(white_indices[index]);
            black_indices[index] = y * side + starting_black_x + x_local * 2;
            black_values[index] = static_cast<char>(black_indices[index]);
        }
    }

    std::shuffle(white_values.begin(), white_values.end(), random_generator);
    std::shuffle(black_values.begin(), black_values.end(), random_generator);

    // decide if there needs to be even or odd amount of moves
    if (random_generator() % 2) {
        for (unsigned int i = 0; i < board_half_size; i++) {
            generated_level[white_indices[i]] = white_values[i];
            generated_level[black_indices[i]] = black_values[i];
        }
    } else {
        for (unsigned int i = 0; i < board_half_size; i++) {
            generated_level[white_indices[i]] = black_values[i];
            generated_level[black_indices[i]] = white_values[i];
        }
    }
}

int main() {
    std::random_device rd;
    std::mt19937 random_generator(rd());

    std::ofstream of("levels.txt");

    std::cout << "Enter the number of levels you want to generate:\n";
    int num_levels;
    std::cin >> num_levels;

    level generated_level;
    for (int i = 0; i < num_levels; i++) {
        randomizeLevel(generated_level, random_generator);
        of.write(generated_level.data(), generated_level.size());
    }
}
