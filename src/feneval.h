#pragma once

#include <fstream>
#include <iostream>
#include <istream>
#include <vector>

#include "position.h"
#include "thread.h"

// From Andrew Grant's code
constexpr int KPRECISION = 11;
constexpr int NPOSITIONS = 100000;
constexpr char* FILENAME = "FENSSHUFFLED";

typedef struct {
    double eval; 
    double result; 
    std::string fen;
} FenData;

void get_fenfile_mse(Position& current, std::istream& is, StateListPtr& states);