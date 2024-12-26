#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <chrono>
#include <thread>


using namespace std;

class Board {
public:
    Board(int size);
    bool placeShip(int x, int y);
    bool isHit(int x, int y);
    void markHit(int x, int y);
    void markMiss(int x, int y);
    bool allShipsSunk() const;
    void display(bool showShips) const; // Argumento para decidir si mostrar barcos o no
    int getSize() const;
    bool isMissed(int x, int y) const;
    bool isAttacked(int x, int y) const; // Verifica si ya ha sido atacada

private:
    int size;
    vector<vector<char>> grid;
    bool isValidPosition(int x, int y) const;
};

Board::Board(int size) : size(size), grid(size, vector<char>(size, '~')) {}

bool Board::placeShip(int x, int y) {
    if (isValidPosition(x, y)) {
        grid[x][y] = 'B';
        return true;
    }
    return false;
}

bool Board::isHit(int x, int y) {
    return grid[x][y] == 'B';
}

void Board::markHit(int x, int y) {
    grid[x][y] = 'X'; // Marcar como impacto
}

void Board::markMiss(int x, int y) {
    grid[x][y] = 'O'; // Marcar como agua
}

bool Board::allShipsSunk() const {
    for (const auto& row : grid) {
        for (char cell : row) {
            if (cell == 'B') {
                return false;
            }
        }
    }
    return true;
}

void Board::display(bool showShips) const {
    cout << "  ";
    for (int j = 0; j < size; ++j) {
        cout << j << " ";
    }
    cout << endl;
    for (int i = 0; i < size; ++i) {
        cout << i << " ";
        for (int j = 0; j < size; ++j) {
            if (showShips) {
                cout << grid[i][j] << " ";
            } else {
                cout << (grid[i][j] == 'B' ? '~' : grid[i][j]) << " "; // Oculta los barcos
            }
        }
        cout << endl;
    }
}

int Board::getSize() const {
    return size;
}

bool Board::isMissed(int x, int y) const {
    return grid[x][y] == 'O';
}

bool Board::isAttacked(int x, int y) const {
    return grid[x][y] == 'O' || grid[x][y] == 'X'; // Verifica si la posición ya ha sido atacada
}

bool Board::isValidPosition(int x, int y) const {
    return x >= 0 && x < size && y >= 0 && y < size && grid[x][y] == '~';
}

class Player {
public:
    Player(int size, const string& name, bool isHuman);
    bool placeShip(int x, int y);
    bool guess(int x, int y);
    bool allShipsSunk() const;
    void displayBoard(bool showShips) const; // Mostrar el tablero con la opción de ocultar barcos
    string getName() const;
    bool isHumanPlayer() const;
    bool randomAttack(Player& opponent);
    void attack(Player& opponent);

private:
    Board board;
    string playerName;
    bool isHuman;
};

Player::Player(int size, const string& name, bool isHuman)
    : board(size), playerName(name), isHuman(isHuman) {}

bool Player::placeShip(int x, int y) {
    return board.placeShip(x, y);
}

bool Player::guess(int x, int y) {
    if (board.isAttacked(x, y)) { // Verifica si ya fue atacada
        cout << "¡Ya has atacado esta posición antes! Elige otra.\n";
        return false; // Si ya fue atacada, no hace nada
    }

    if (board.isHit(x, y)) {
        board.markHit(x, y);
        return true; // Impacto
    } else {
        board.markMiss(x, y);
        return false; // Agua
    }
}

bool Player::allShipsSunk() const {
    return board.allShipsSunk();
}

void Player::displayBoard(bool showShips) const {
    cout << playerName << "'s Tablero:\n";
    board.display(showShips); // Mostrar el tablero con la opción de ocultar barcos
    cout << endl;
}

string Player::getName() const {
    return playerName;
}

bool Player::isHumanPlayer() const {
    return isHuman;
}

bool Player::randomAttack(Player& opponent) {
    int x, y;
    do {
        x = rand() % board.getSize();
        y = rand() % board.getSize();
    } while (opponent.board.isAttacked(x, y)); // Verifica que la posición no haya sido atacada
    return opponent.guess(x, y);
}

void Player::attack(Player& opponent) {
    int x, y;
    cout << playerName << ", ingresa las coordenadas para atacar (fila columna): ";

    auto start = chrono::steady_clock::now(); // Iniciar el temporizador
    bool validInput = false;

    // Establecer el tiempo límite
    auto timeLimit = chrono::seconds(15);

    while (!validInput) {
        // Verifica el tiempo transcurrido
        auto now = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration_cast<chrono::seconds>(now - start);

        // Calcular el tiempo restante
        int timeRemaining = chrono::duration_cast<chrono::seconds>(timeLimit - elapsedTime).count();

        // Si el tiempo aún no ha terminado, mostrar la cuenta regresiva
        if (timeRemaining > 0) {
            cout << "Tiempo restante: " << timeRemaining << " segundos.\n";
        } else {
            cout << "¡Tiempo agotado! Has perdido tu oportunidad de atacar.\n";
            validInput = true;  // Sale del bucle si el tiempo se agotó
            break;
        }

        // Solo permite que el jugador ingrese las coordenadas mientras no se haya agotado el tiempo
        if (cin >> x >> y) {
            if (!opponent.board.isAttacked(x, y)) {
                validInput = true;
                if (opponent.guess(x, y)) {
                    cout << "¡Impacto!" << endl;
                } else {
                    cout << "¡Agua!" << endl;
                }
            } else {
                cout << "¡Ya has atacado esta posición! Elige otra.\n";
            }
        }

        // Para que no se quede en un bucle infinito, espera 1 segundo entre verificaciones
        if (!validInput) {
            this_thread::sleep_for(chrono::seconds(1)); // Añadir un pequeño delay para esperar
            system("clear"); // Limpia la pantalla para mostrar la cuenta regresiva actualizada
        }
    }
}


class Game {
public:
    Game(int size, int ships);
    void start();
    void playAgain();

private:
    int size;
    int ships;
    Player player{size, "", true}; // Inicializar sin nombre hasta que se establezca
    Player opponent{size, "Computadora", false}; // Oponente con nombre fijo
    string secondPlayerName;

    void setupPlayer(const string& name, Player& player);
    void setupComputer(Player& player);
    void playAgainstComputer();
    void play1v1();
    void askToPlayAgain();
    void getSecondPlayerName();
};

Game::Game(int size, int ships) : size(size), ships(ships) {
    srand(static_cast<unsigned int>(time(0))); // Inicializa la semilla para rand
}

void Game::start() {
	
	cout << "~\n";
    cout << "**      HUNDIR EL BARCO       **\n";
    cout << "~\n";
    cout << "Ingresa tu nombre: ";
    string playerName;
    getline(cin, playerName); // Usar getline para capturar el nombre
    player = Player(size, playerName, true); // Establecer nombre del jugador

    cout << "Selecciona el modo de juego:\n1. 1 vs 1\n2. Un jugador\n";
    int mode;
    cin >> mode;
    cin.ignore(); // Limpiar el buffer de entrada

    if (mode == 1) {
    	cout << "\nInstrucciones para el modo 1 vs 1:\n";
        cout << "1. Cada jugador coloca sus barcos en el tablero.\n";
        cout << "2. Los barcos se colocan especificando las coordenadas de fila y columna.\n";
        cout << "3. En cada turno, los jugadores intentan adivinar las posiciones de los barcos enemigos.\n";
        cout << "4. El jugador que hunda todos los barcos del oponente primero, gana.\n";
        cout << "\n*Posicionamiento de barcos:*\n";
        cout << "   - El tablero tiene filas y columnas numeradas de 0 a 4.\n";
        cout << "   - Ingresas primero el numero de fila (de arriba a abajo) y luego el número de columna (de izquierda a derecha).\n";
        cout << "   Ejemplo: Si deseas colocar un barco en la fila 2 y la columna 3, ingresa: 2 3\n";
        getSecondPlayerName(); // Capturar el nombre del segundo jugador
        setupPlayer(playerName, player);
        setupPlayer(secondPlayerName, opponent);
        play1v1();
        
    } else {
    	cout << "\nInstrucciones para el modo 1 jugador:\n";
        cout << "1. El jugador coloca sus barcos en el tablero.\n";
        cout << "2. La computadora coloca sus barcos de manera aleatoria.\n";
        cout << "3. El jugador intenta adivinar las posiciones de los barcos enemigos.\n";
        cout << "4. El jugador gana si hunde todos los barcos de la computadora.\n";
        cout << "\n*Posicionamiento de barcos:*\n";
        cout << "   - El tablero tiene filas y columnas numeradas de 0 a 4.\n";
        cout << "   - Ingresas primero el número de fila (de arriba a abajo) y luego el número de columna (de izquierda a derecha).\n";
        cout << "   Ejemplo: Si deseas colocar un barco en la fila 2 y la columna 3, ingresa: 2 3\n";
        setupPlayer(playerName, player);
        setupComputer(opponent);
        playAgainstComputer();
    }
}

void Game::setupPlayer(const string& name, Player& player) {
    player = Player(size, name, true);
    cout << "Coloca tus barcos en el tablero.\n";
    for (int i = 0; i < ships; ) {
        int x, y;
        cout << "Ingresa las coordenadas del barco (fila columna): ";
        cin >> x >> y;
        if (player.placeShip(x, y)) {
            ++i;
        } else {
            cout << "Posición inválida o ya ocupada. Intenta de nuevo.\n";
        }
    }
}

void Game::setupComputer(Player& player) {
    cout << "La computadora colocará sus barcos.\n";
    for (int i = 0; i < ships; ) {
        int x = rand() % size;
        int y = rand() % size;
        if (player.placeShip(x, y)) {
            ++i;
        }
    }
}

void Game::playAgainstComputer() {
    while (true) {
        player.attack(opponent);
        player.displayBoard(true); // Mostrar el tablero del jugador
        opponent.displayBoard(false); // No mostrar barcos de la computadora
        if (opponent.allShipsSunk()) {
            cout << player.getName() << " ha ganado.\n";
            break;
        }

        // Ataque de la computadora
        cout << "La computadora está atacando...\n";
        opponent.randomAttack(player);
        player.displayBoard(true); // Mostrar el tablero del jugador
        opponent.displayBoard(false); // No mostrar barcos de la computadora
        if (player.allShipsSunk()) {
            cout << "La computadora ha ganado.\n";
            break;
        }
    }
    askToPlayAgain();
}

void Game::play1v1() {
    while (true) {
        player.attack(opponent);
        player.displayBoard(true); // Mostrar el tablero del jugador
        opponent.displayBoard(true); // Mostrar el tablero del oponente
        if (opponent.allShipsSunk()) {
            cout << player.getName() << " ha ganado.\n";
            break;
        }

        opponent.attack(player);
        player.displayBoard(true); // Mostrar el tablero del jugador
        opponent.displayBoard(true); // Mostrar el tablero del oponente
        if (player.allShipsSunk()) {
            cout << opponent.getName() << " ha ganado.\n";
            break;
        }
    }
    askToPlayAgain();
}

void Game::askToPlayAgain() {
    cout << "\n¿Quieres jugar nuevamente? (1: Sí, 2: No): ";
    int option;
    cin >> option;
    if (option == 1) {
        start(); // Reinicia el juego
    } else {
        cout << "Gracias por jugar.\n";
    }
}

void Game::getSecondPlayerName() {
    cout << "Ingresa el nombre del segundo jugador: ";
    getline(cin, secondPlayerName);
}

int main() {
    Game game(5, 3); // Tamaño de tablero 5x5 y 3 barcos
    game.start();
    return 0;
}