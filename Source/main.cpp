#include "../Include/checkers.h"

int main() {
    setlocale(LC_ALL, "ru");

    std::cout << "Добро пожаловать в игру \"Классические шашки\"!\n";
    std::cout << "Выберите, за кого хотите играть (W - белые, B - чёрные): ";

    char side;
    std::cin >> side;
    side = std::toupper(side);
    CheckersBoard board;
    bool userIsWhite = (side == 'W');
    board.setWhiteToMove(true);
    if (!userIsWhite) {
        Move aiMove = board.getBestMove(5); // глубина = 5
        if (aiMove.size() > 0) {
            board.makeMove(aiMove);
        }
    }

    // Чистим буфер после чтения side
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (true) {
        board.printBoard();

        // Проверяем, может ли текущая сторона ходить
        if (!board.canCurrentPlayerMove()) {
            if (board.isWhiteToMove()) {
                std::cout << "Белые не могут ходить! Победили чёрные!\n";
            }
            else {
                std::cout << "Чёрные не могут ходить! Победили белые!\n";
            }
            break;
        }

        bool curIsWhite = board.isWhiteToMove();
        if ((curIsWhite && userIsWhite) || (!curIsWhite && !userIsWhite)) {
            std::vector<Move> allMoves = board.getAllPossibleMoves(curIsWhite);

            std::vector<Move> mandatoryMoves;
            for (const auto& mv : allMoves) {
                if (mv.size() > 2) {
                    mandatoryMoves.push_back(mv);
                }
            }

            bool hasMandatoryMoves = !mandatoryMoves.empty();
            const std::vector<Move>& validMoves = hasMandatoryMoves ? mandatoryMoves : allMoves;

            while (true) {
                std::cout << "Ваш ход (формат B3 A4): ";
                std::string line;
                std::getline(std::cin, line);
                if (line.empty()) {
                    std::getline(std::cin, line);
                }
                Move userMove;
                if (!board.parseUserMove(line, userMove)) {
                    std::cout << "Некорректный ввод. Пример: A3 B4\n";
                    continue;
                }

                // Проверяем, что ход есть в списке validMoves
                bool isValidMove = false;
                for (const auto& mv : validMoves) {
                    if (mv.path == userMove.path) {
                        isValidMove = true;
                        break;
                    }
                }

                if (!isValidMove) {
                    std::cout << "Недопустимый ход! Попробуйте снова.\n";
                    continue;
                }
                if (!board.makeMove(userMove)) {
                    std::cout << "Не удалось применить ход (возможно, логическая ошибка). Попробуйте снова.\n";
                    continue;
                }
                break;
            }
        }
        else {
            // Ход компьютера
            std::cout << "Ход Компьютера...\n";
            Move aiMove = board.getBestMove(5);
            if (aiMove.size() == 0) {
                std::cout << "Компьютер не может ходить... Похоже, игра заканчивается.\n";
                break;
            }
            board.makeMove(aiMove);
        }
    }
    return 0;
}
