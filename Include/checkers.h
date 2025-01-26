#ifndef CHECKERSBOARD_H
#define CHECKERSBOARD_H

#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <limits>

// Типы для шашек
enum class Piece {
    EMPTY,
    W,   // Белая простая
    B,   // Чёрная простая
    DW,  // Дамка белая (ранее WK)
    DB   // Дамка чёрная (ранее BK)
};

// Координаты клетки
struct Coord {
    int r, c;
    Coord(int rr, int cc) : r(rr), c(cc) {}
    // Перегрузка оператора ==
    bool operator==(const Coord& other) const {
        return r == other.r && c == other.c;
    }
};

// Ход (цепочка клеток)
struct Move {
    std::vector<Coord> path;
    Move() {}
    Move(const std::vector<Coord>& p) : path(p) {}

    Coord from() const { return path.front(); }
    Coord to()   const { return path.back(); }
    size_t size() const { return path.size(); }
};

class CheckersBoard {
public:
    static const int BOARD_SIZE = 8;

    CheckersBoard();
    void initBoard();
    std::string pieceToString(Piece p) const;
    void printBoard();
    bool isWhiteToMove() const;
    void setWhiteToMove(bool w);

    bool canCurrentPlayerMove();
    std::vector<Move> getAllPossibleMoves(bool whiteSide);

    bool makeMove(const Move& move);

    // Оценочная функция
    int evaluateBoard() const;

    // Minimax c альфа-бета отсечением и многопоточностью
    int minimax(int depth, int alpha, int beta, bool maximizingPlayer);

    // Вернуть лучший ход для текущего whiteToMove
    Move getBestMove(int depth);

    // Парсим ввод вида "A3 B4" -> путь
    bool parseUserMove(const std::string& input, Move& move);

private:
    std::vector<std::vector<Piece>> board;
    bool whiteToMove;

    bool isValidPos(int r, int c) const;
    bool isColor(Piece p, bool whiteSide) const;

    // Генерация возможных рубок (цепочек) для одной шашки
    void getAllCapturesForPiece(int r, int c, std::vector<Move>& captures);

    // Рекурсивный поиск всех цепочек рубки
    void dfsCaptures(std::vector<Coord>& path, std::vector<Move>& results);

    // Генерация обычных ходов (без рубки)
    void getAllNormalMovesForPiece(int r, int c, std::vector<Move>& moves);
};

#endif // CHECKERSBOARD_H