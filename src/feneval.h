#pragma once

#include <fstream>
#include <iostream>
#include <istream>
#include <vector>

#include "position.h"
#include "thread.h"

typedef struct {
    int eval; 
    int result; 
    std::string fen;
} Evaluation;

std::vector<std::string> get_fenfile_mse(Position& current, std::istream& is, StateListPtr& states);