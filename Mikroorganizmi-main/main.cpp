#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <stack>
#include <algorithm>
#include <limits>
#include <chrono>
#include <memory> // Dodana knjižnica za spremljanje porabe pomnilnika
#include <windows.h>
#include <psapi.h>

using namespace std;
using namespace std::chrono;
// Define the type for a shape (vector of coordinates)
using Shape = vector<pair<int, int>>;

// Function to normalize the shape by rotating it to the standard orientation
Shape normalizeShape(const Shape& shape) {
    Shape normalizedShape = shape;

    // Find the top-left cell of the shape
    pair<int, int> minCoords = {numeric_limits<int>::max(), numeric_limits<int>::max()};
    for (const auto& coord : shape) {
        minCoords.first = min(minCoords.first, coord.first);
        minCoords.second = min(minCoords.second, coord.second);
    }

    // Normalize the shape by subtracting the top-left cell coordinates
    for (auto& coord : normalizedShape) {
        coord.first -= minCoords.first;
        coord.second -= minCoords.second;
    }

    // Try all 4 rotations and mirror reflections, and select the one with the minimal representation
    Shape bestTransformation = normalizedShape;
    for (int i = 0; i < 4; ++i) {
        // Rotate the shape 90 degrees clockwise
        for (auto& coord : normalizedShape) {
            coord = {coord.second, -coord.first};
        }

        // Reflect the shape horizontally
        for (auto& coord : normalizedShape) {
            coord.first = -coord.first;
        }

        // Reflect the shape vertically
        for (auto& coord : normalizedShape) {
            coord.second = -coord.second;
        }

        // Find the top-left cell of the transformed shape
        pair<int, int> transformedMinCoords = {numeric_limits<int>::max(), numeric_limits<int>::max()};
        for (const auto& coord : normalizedShape) {
            transformedMinCoords.first = min(transformedMinCoords.first, coord.first);
            transformedMinCoords.second = min(transformedMinCoords.second, coord.second);
        }

        // Normalize the transformed shape by subtracting the top-left cell coordinates
        for (auto& coord : normalizedShape) {
            coord.first -= transformedMinCoords.first;
            coord.second -= transformedMinCoords.second;
        }

        // Sort the transformed shape coordinates
        sort(normalizedShape.begin(), normalizedShape.end());

        // Update bestTransformation if the current transformation is "smaller"
        if (normalizedShape < bestTransformation) {
            bestTransformation = normalizedShape;
        }
    }
    return bestTransformation;
}

// Non-recursive DFS using a stack
void dfs(vector<vector<int>>& grid, int startRow, int startCol, int shapeId, Shape& shape) {
    int numRows = grid.size();
    int numCols = grid[0].size();

    stack<pair<int, int>> dfsStack; //Uporabili bomo lasten sklad (stack), ki ga bomo ročno upravljali, da bi se izognili presežku skladovnega prostora, ki ga povzroča rekurzivni DFS (depth-first search) na velikih matrikah
    dfsStack.push({startRow, startCol});

    while (!dfsStack.empty()) {
        auto current = dfsStack.top();
        dfsStack.pop();

        int r = current.first;
        int c = current.second;

        if (r < 0 || c < 0 || r >= numRows || c >= numCols || grid[r][c] != 1) {
            continue;
        }

        grid[r][c] = shapeId;
        shape.push_back({r, c});

        // Add neighboring cells to the stack
        dfsStack.push({r + 1, c});
        dfsStack.push({r - 1, c});
        dfsStack.push({r, c + 1});
        dfsStack.push({r, c - 1});
    }
}

int countShapes(vector<vector<int>>& grid) {
    int shapeCount = 0;
    set<Shape> shapeSet;

    int numRows = grid.size();
    int numCols = grid[0].size();

    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            if (grid[i][j] == 1) {
                shapeCount++;
                Shape shape;
                dfs(grid, i, j, shapeCount + 1, shape);
                Shape uniqueShape = normalizeShape(shape);

                // Preveri, ali je ta oblika že bila dodana v drugi rotaciji
                if (shapeSet.find(uniqueShape) == shapeSet.end()) {
                    shapeSet.insert(uniqueShape);
                }
            }
        }
    }

    return shapeSet.size();
}

int main(int argc, char* argv[]) {
    // Open the input file
    ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        cerr << "Error opening input file." << endl;
        return 1;
    }

    // Read grid dimensions
    int numRows, numCols;
    inputFile >> numRows >> numCols;

    // Read the grid from the file
    vector<vector<int>> grid(numRows, vector<int>(numCols));
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            char cellValue;
            inputFile >> cellValue;
            grid[i][j] = cellValue - '0'; // Convert char to int
        }
    }

    // Close the input file
    inputFile.close();

    // Measure execution time
    auto start = high_resolution_clock::now();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T startUsage = pmc.PrivateUsage;
    // Call countShapes with the read grid
    int result = countShapes(grid);

    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T endUsage = pmc.PrivateUsage;

    // Izračunamo razliko v porabi pomnilnika
    SIZE_T memoryUsed = endUsage - startUsage;

    // Pretvorimo bajte v mebibajte
    memoryUsed /= 1024;

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    // Print the result and execution time
    cout << "Stevilo mikroorganizmov " << result << endl;
    cout << "Cas izvajanja: " << duration.count() << " milisekund" << endl;
    cout << "Poraba pomnilnika: " << memoryUsed << " KB " << endl;

    return 0;
}

/*
 #include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <limits>
#include <stack>
#include <chrono>
#include <windows.h>
#include <psapi.h>

using namespace std;
using namespace std::chrono;

// Define the type for a shape (vector of coordinates)
using Shape = vector<pair<int, int>>;

// Function to normalize the shape by rotating it to the standard orientation
// Function to normalize the shape by rotating and mirroring it to the standard orientation
Shape normalizeShape(const Shape& shape) {
    Shape normalizedShape = shape;

    // Find the top-left cell of the shape
    pair<int, int> minCoords = {numeric_limits<int>::max(), numeric_limits<int>::max()};
    for (const auto& coord : shape) {
        minCoords.first = min(minCoords.first, coord.first);
        minCoords.second = min(minCoords.second, coord.second);
    }

    // Normalize the shape by subtracting the top-left cell coordinates
    for (auto& coord : normalizedShape) {
        coord.first -= minCoords.first;
        coord.second -= minCoords.second;
    }

    // Try all 4 rotations and mirror reflections, and select the one with the minimal representation
    Shape bestTransformation = normalizedShape;
    for (int i = 0; i < 4; ++i) {
        // Rotate the shape 90 degrees clockwise
        for (auto& coord : normalizedShape) {
            coord = {coord.second, -coord.first};
        }

        // Reflect the shape horizontally
        for (auto& coord : normalizedShape) {
            coord.first = -coord.first;
        }

        // Reflect the shape vertically
        for (auto& coord : normalizedShape) {
            coord.second = -coord.second;
        }

        // Find the top-left cell of the transformed shape
        pair<int, int> transformedMinCoords = {numeric_limits<int>::max(), numeric_limits<int>::max()};
        for (const auto& coord : normalizedShape) {
            transformedMinCoords.first = min(transformedMinCoords.first, coord.first);
            transformedMinCoords.second = min(transformedMinCoords.second, coord.second);
        }

        // Normalize the transformed shape by subtracting the top-left cell coordinates
        for (auto& coord : normalizedShape) {
            coord.first -= transformedMinCoords.first;
            coord.second -= transformedMinCoords.second;
        }

        // Sort the transformed shape coordinates
        sort(normalizedShape.begin(), normalizedShape.end());

        // Update bestTransformation if the current transformation is "smaller"
        if (normalizedShape < bestTransformation) {
            bestTransformation = normalizedShape;
        }
    }
    return bestTransformation;
}


// Brute-force approach to count shapes
int countShapes(vector<vector<int>>& grid) {
    int shapeCount = 0;
    set<Shape> shapeSet;

    int numRows = grid.size();
    int numCols = grid[0].size();

    // Brute-force approach using nested loops
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            if (grid[i][j] == 1) {
                shapeCount++;

                // Brute-force search for shape
                Shape shape;
                stack<pair<int, int>> dfsStack;
                dfsStack.push({i, j});

                while (!dfsStack.empty()) {
                    auto current = dfsStack.top();
                    dfsStack.pop();

                    int r = current.first;
                    int c = current.second;

                    if (r < 0 || c < 0 || r >= numRows || c >= numCols || grid[r][c] != 1) {
                        continue;
                    }

                    grid[r][c] = 0;  // Mark the cell as visited
                    shape.push_back({r, c});

                    // Add neighboring cells to the stack
                    dfsStack.push({r + 1, c});
                    dfsStack.push({r - 1, c});
                    dfsStack.push({r, c + 1});
                    dfsStack.push({r, c - 1});
                }

                // Check all transformations (rotations and reflections)
                for (int rotate = 0; rotate < 4; ++rotate) {
                    for (int reflect = 0; reflect < 2; ++reflect) {
                        Shape uniqueShape = normalizeShape(shape);

                        // Check if the shape already exists in the set
                        if (shapeSet.find(uniqueShape) == shapeSet.end()) {
                            shapeSet.insert(uniqueShape);
                        }

                        // Reflect the shape horizontally
                        for (auto& coord : shape) {
                            coord.first = -coord.first;
                        }

                        // Reflect the shape vertically
                        for (auto& coord : shape) {
                            coord.second = -coord.second;
                        }
                    }

                    // Rotate the shape 90 degrees clockwise
                    for (auto& coord : shape) {
                        coord = {coord.second, -coord.first};
                    }
                }
            }
        }
    }

    return shapeSet.size();
}


int main(int argc, char* argv[]) {
    // Open the input file
    ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        cerr << "Error opening input file." << endl;
        return 1;
    }

    // Read grid dimensions
    int numRows, numCols;
    inputFile >> numRows >> numCols;

    // Read the grid from the file
    vector<vector<int>> grid(numRows, vector<int>(numCols));
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            char cellValue;
            inputFile >> cellValue;
            grid[i][j] = cellValue - '0'; // Convert char to int
        }
    }

    // Close the input file
    inputFile.close();

    // Measure execution time
    auto start = high_resolution_clock::now();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T startUsage = pmc.PrivateUsage;
    // Call countShapes with the read grid
    int result = countShapes(grid);

    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T endUsage = pmc.PrivateUsage;

    // Izračunamo razliko v porabi pomnilnika
    SIZE_T memoryUsed = endUsage - startUsage;

    // Pretvorimo bajte v mebibajte
    memoryUsed /= 1024;

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    // Print the result and execution time
    cout << "Stevilo mikroorganizmov: " << result << endl;
    cout << "Cas izvajanja: " << duration.count() << " milisekund" << endl;
    cout << "Poraba pomnilnika: " << memoryUsed << " KB " << endl;

    return 0;
}
 */
