/*
 * feneval.cpp
 * 
 * I don't think there's anything original here at all lol
 * 
 */

/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2020 Marco Costalba, Joona Kiiski, Gary Linscott, Tord
  Romstad

  (Additionally, I've added a ton of code from Ethereal, which is by Andrew Grant)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <math.h>
#include <vector>

#include "position.h"
#include "thread.h"
#include "feneval.h"
#include "evaluate.h"

using namespace std;

// Written by Andrew Grant
double sigmoid(double K, double S) {
    return 1.0 / (1.0 + exp(-K * S / 400.0));
}

// Written by Andrew Grant
static double completeEvaluationError(FenData* data, double K)
{

    double total = 0.0;

    for (int i = 0; i < NPOSITIONS; i++) {
        total += pow(data[i].result - sigmoid(K, data[i].eval), 2);
    }

    return total / (double)NPOSITIONS;
}

// Written by Andrew Grant
static double computeOptimalK(FenData* data)
{

    double start = -10.0, end = 10.0, delta = 1.0;
    double curr = start, error, best = completeEvaluationError(data, start);

    for (int i = 0; i < KPRECISION; i++) {

        curr = start - delta;
        while (curr < end) {
            curr  = curr + delta;
            error = completeEvaluationError(data, curr);
            if (error <= best)
                best = error, start = curr;
        }

        printf("COMPUTING K ITERATION [%d] K = %f E = %f\n", i, start, best);

        end   = start + delta;
        start = start - delta;
        delta = delta / 10.0;
    }

    return start;
}

void get_fenfile_mse(
    Position& current, istream& is, StateListPtr& states)
{

    vector<FenData> fens;
    // string          token;

    // Assign default values to missing arguments
    // string ttSize    = (is >> token) ? token : "16";
    // string threads   = (is >> token) ? token : "1";
    // string limit     = (is >> token) ? token : "13";
    // string limitType = (is >> token) ? token : "depth";

    {
        string   fen;
        ifstream file(FILENAME);

        if (!file.is_open()) {
            cerr << "Unable to open file " << FILENAME << endl;
            exit(EXIT_FAILURE);
        }

        int i = 0;
        while (getline(file, fen) && ++i < NPOSITIONS) {
            FenData data;
            
            data.fen = fen;

            // Written by Andrew Grant
            data.eval  = atoi(strstr(fen.c_str(), "] ") + 2);

            if (strstr(fen.c_str(), " b "))
                data.eval *= -1;

            // Determine the result of the game
            if (strstr(fen.c_str(), "[1.0]")) {
                data.result = 1.0;
            } else if (strstr(fen.c_str(), "[0.5]")) {
                data.result = 0.5;
            } else if (strstr(fen.c_str(), "[0.0]")) {
                data.result = 0.0;
            } else {
                printf("Cannot Parse %s\n", fen.c_str());
                exit(EXIT_FAILURE);
            }

            if (!fen.empty()) {
                fens.push_back(data);
            }
        }

        file.close();
    }

    computeOptimalK(&fens[0]);

    for (const FenData& data : fens) {
        current.set(data.fen, false, &states->back(), Threads.main());
        
        current.this_thread()->contempt = SCORE_ZERO;  // perhaps suboptimal

        auto eval = Eval::evaluate(current);
        printf("Cached eval: %f\tResult: %f\tSF eval: %d\n", 
            data.eval, data.result, eval
        );
    }
}
