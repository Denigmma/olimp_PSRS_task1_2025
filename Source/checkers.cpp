#include "../Include/checkers.h"
#include <algorithm>
#include <cctype>
#include <limits>
#include <thread>

// Конструктор
CheckersBoard::CheckersBoard() {
    board.resize(BOARD_SIZE, std::vector<Piece>(BOARD_SIZE, Piece::EMPTY));
    initBoard();
    whiteToMove = true;
}

// Инициализация стандартной расстановки
void CheckersBoard::initBoard() {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            board[r][c] = Piece::EMPTY;
        }
    }
    // Белые (W) на верхних трёх рядах (r=0..2), чёрные (B) - на нижних (r=5..7)
    // Только на тёмных клетках ((r+c)%2 == 1).
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if ((r + c) % 2 == 1) {
                board[r][c] = Piece::W;
            }
        }
    }
    for (int r = 5; r < 8; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if ((r + c) % 2 == 1) {
                board[r][c] = Piece::B;
            }
        }
    }
}

// Отладочный вывод одной шашки
std::string CheckersBoard::pieceToString(Piece p) const {
    switch (p) {
    case Piece::W:   return "W";    // белая простая
    case Piece::B:   return "B";    // чёрная простая
    case Piece::DW:  return "DW";   // дамка белая
    case Piece::DB:  return "DB";   // дамка чёрная
    default:         return ".";
    }
}

// Печать доски
void CheckersBoard::printBoard() {
    std::cout << "   A B C D E F G H\n";
    std::cout << "  -----------------\n";
    for (int r = 0; r < BOARD_SIZE; ++r) {
        std::cout << (r + 1) << " |";
        for (int c = 0; c < BOARD_SIZE; ++c) {
            std::cout << pieceToString(board[r][c]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

bool CheckersBoard::isWhiteToMove() const {
    return whiteToMove;
}

void CheckersBoard::setWhiteToMove(bool w) {
    whiteToMove = w;
}

// Проверяем, может ли текущий игрок сделать ход
bool CheckersBoard::canCurrentPlayerMove() {
    auto moves = getAllPossibleMoves(whiteToMove);
    return !moves.empty();
}

// Сбор всех ходов: сперва рубки, если есть — только они, иначе обычные
std::vector<Move> CheckersBoard::getAllPossibleMoves(bool whiteSide) {
    std::vector<Move> captureMoves;
    std::vector<Move> normalMoves;

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Piece p = board[r][c];
            if (!isColor(p, whiteSide)) continue;

            // Смотрим, есть ли рубки
            std::vector<Move> pieceCaptures;
            getAllCapturesForPiece(r, c, pieceCaptures);
            if (!pieceCaptures.empty()) {
                captureMoves.insert(captureMoves.end(),
                    pieceCaptures.begin(),
                    pieceCaptures.end());
            }
            else {
                // Если нет рубок, смотрим обычные ходы
                std::vector<Move> pieceNormals;
                getAllNormalMovesForPiece(r, c, pieceNormals);
                normalMoves.insert(normalMoves.end(),
                    pieceNormals.begin(),
                    pieceNormals.end());
            }
        }
    }

    // Принудительная рубка
    if (!captureMoves.empty()) {
        return captureMoves;
    }
    return normalMoves;
}

// makeMove: применяем путь из Move
bool CheckersBoard::makeMove(const Move& move) {
    if (move.path.size() < 2) {
        return false;
    }

    auto backupBoard = board;
    bool savedWhiteToMove = whiteToMove;

    Coord start = move.from();
    if (!isValidPos(start.r, start.c)) {
        return false;
    }
    Piece startP = board[start.r][start.c];
    if (startP == Piece::EMPTY || !isColor(startP, whiteToMove)) {
        return false;
    }

    Piece curP = startP;
    board[start.r][start.c] = Piece::EMPTY;

    for (size_t i = 0; i < move.path.size() - 1; ++i) {
        Coord c0 = move.path[i];
        Coord c1 = move.path[i + 1];

        if (!isValidPos(c1.r, c1.c)) {
            board = backupBoard;
            whiteToMove = savedWhiteToMove;
            return false;
        }
        if (board[c1.r][c1.c] != Piece::EMPTY) {
            board = backupBoard;
            whiteToMove = savedWhiteToMove;
            return false;
        }

        int dr = c1.r - c0.r;
        int dc = c1.c - c0.c;

        bool isKing = (curP == Piece::DW || curP == Piece::DB);
        bool isCapture = (std::abs(dr) >= 2 && std::abs(dr) == std::abs(dc));

        if (isCapture) {
            int stepR = (dr > 0) ? 1 : -1;
            int stepC = (dc > 0) ? 1 : -1;

            bool foundEnemy = false;
            int rr = c0.r + stepR;
            int cc = c0.c + stepC;
            while (true) {
                if (rr == c1.r && cc == c1.c) break;
                if (!isValidPos(rr, cc)) {
                    board = backupBoard;
                    whiteToMove = savedWhiteToMove;
                    return false;
                }
                if (isColor(board[rr][cc], !whiteToMove)) {
                    board[rr][cc] = Piece::EMPTY;
                    foundEnemy = true;
                }
                rr += stepR;
                cc += stepC;
            }
            if (!foundEnemy) {
                board = backupBoard;
                whiteToMove = savedWhiteToMove;
                return false;
            }
        }
        else {
            if (std::abs(dr) != std::abs(dc)) {
                board = backupBoard;
                whiteToMove = savedWhiteToMove;
                return false;
            }
            if (!isKing) {
                if (std::abs(dr) != 1) {
                    board = backupBoard;
                    whiteToMove = savedWhiteToMove;
                    return false;
                }
                if (curP == Piece::W && dr < 0) {
                    board = backupBoard;
                    whiteToMove = savedWhiteToMove;
                    return false;
                }
                if (curP == Piece::B && dr > 0) {
                    board = backupBoard;
                    whiteToMove = savedWhiteToMove;
                    return false;
                }
            }
            else {
                int stepR = (dr > 0) ? 1 : -1;
                int stepC = (dc > 0) ? 1 : -1;
                int steps = std::abs(dr);
                int rr = c0.r + stepR, cc = c0.c + stepC;
                for (int st = 1; st < steps; ++st) {
                    if (board[rr][cc] != Piece::EMPTY) {
                        board = backupBoard;
                        whiteToMove = savedWhiteToMove;
                        return false;
                    }
                    rr += stepR;
                    cc += stepC;
                }
            }
        }
    }

    // Ставим шашку в конечную клетку
    Coord end = move.to();
    board[end.r][end.c] = curP;

    // Превращение в дамку
    if (curP == Piece::W && end.r == BOARD_SIZE - 1) {
        board[end.r][end.c] = Piece::DW;
    }
    else if (curP == Piece::B && end.r == 0) {
        board[end.r][end.c] = Piece::DB;
    }

    whiteToMove = !whiteToMove;
    return true;
}

// Оценка позиции
int CheckersBoard::evaluateBoard() const {
    int score = 0;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Piece p = board[r][c];
            switch (p) {
            case Piece::W:  score += 1;  break;
            case Piece::DW: score += 3;  break;
            case Piece::B:  score -= 1;  break;
            case Piece::DB: score -= 3;  break;
            default: break;
            }
        }
    }
    return score;
}

// Minimax с альфа-бета и ограниченной параллельностью
int CheckersBoard::minimax(int depth, int alpha, int beta, bool maximizingPlayer)
{
    if (depth == 0) {
        return evaluateBoard();
    }
    bool savedTurn = whiteToMove;
    whiteToMove = maximizingPlayer;
    auto moves = getAllPossibleMoves(maximizingPlayer);
    whiteToMove = savedTurn;

    if (moves.empty()) {
        return maximizingPlayer ? -9999 : 9999;
    }
    bool useParallel = (depth == 5);

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();

        if (useParallel) {
            // Параллельный перебор 
            std::vector<std::thread> threads;
            threads.reserve(moves.size());
            std::vector<int> results(moves.size(), std::numeric_limits<int>::min());

            for (size_t i = 0; i < moves.size(); i++) {
                threads.emplace_back(
                    [this, &moves, i, depth, alpha, beta, &results]()
                    {
                        CheckersBoard temp = *this;
                        temp.whiteToMove = true; // ход белых
                        if (!temp.makeMove(moves[i])) {
                            // Если ход невозможен
                            results[i] = std::numeric_limits<int>::min() + 1;
                            return;
                        }
                        int localAlpha = alpha;
                        int localBeta = beta;
                        // Глубже не параллелим (depth-1 < 5):
                        int val = temp.minimax(depth - 1, localAlpha, localBeta, false);
                        results[i] = val;
                    }
                );
            }
            for (auto& t : threads) {
                t.join();
            }
            for (auto val : results) {
                if (val > maxEval) {
                    maxEval = val;
                }
                if (maxEval > alpha) {
                    alpha = maxEval;
                }
                if (beta <= alpha) {
                    break; // отсечение
                }
            }
        }
        else {
            // Однопоточно
            for (auto& mv : moves) {
                CheckersBoard temp = *this;
                temp.whiteToMove = true;
                if (!temp.makeMove(mv)) {
                    continue;
                }
                int val = temp.minimax(depth - 1, alpha, beta, false);
                if (val > maxEval) {
                    maxEval = val;
                }
                if (maxEval > alpha) {
                    alpha = maxEval;
                }
                if (beta <= alpha) {
                    break; // отсечение
                }
            }
        }
        return maxEval;
    }
    else {
        // minimizingPlayer
        int minEval = std::numeric_limits<int>::max();

        if (useParallel) {
            std::vector<std::thread> threads;
            threads.reserve(moves.size());
            std::vector<int> results(moves.size(), std::numeric_limits<int>::max());

            for (size_t i = 0; i < moves.size(); i++) {
                threads.emplace_back(
                    [this, &moves, i, depth, alpha, beta, &results]()
                    {
                        CheckersBoard temp = *this;
                        temp.whiteToMove = false; // ход чёрных
                        if (!temp.makeMove(moves[i])) {
                            results[i] = std::numeric_limits<int>::max() - 1;
                            return;
                        }
                        int localAlpha = alpha;
                        int localBeta = beta;
                        int val = temp.minimax(depth - 1, localAlpha, localBeta, true);
                        results[i] = val;
                    }
                );
            }
            for (auto& t : threads) {
                t.join();
            }
            for (auto val : results) {
                if (val < minEval) {
                    minEval = val;
                }
                if (minEval < beta) {
                    beta = minEval;
                }
                if (beta <= alpha) {
                    break; // отсечение
                }
            }
        }
        else {
            for (auto& mv : moves) {
                CheckersBoard temp = *this;
                temp.whiteToMove = false;
                if (!temp.makeMove(mv)) {
                    continue;
                }
                int val = temp.minimax(depth - 1, alpha, beta, true);
                if (val < minEval) {
                    minEval = val;
                }
                if (minEval < beta) {
                    beta = minEval;
                }
                if (beta <= alpha) {
                    break;
                }
            }
        }
        return minEval;
    }
}

// Возвращаем лучший ход для текущего whiteToMove
Move CheckersBoard::getBestMove(int depth) {
    bool maximizing = whiteToMove;
    auto moves = getAllPossibleMoves(maximizing);

    if (moves.empty()) {
        return Move();
    }

    Move bestMove;
    int bestEval = maximizing ? std::numeric_limits<int>::min()
        : std::numeric_limits<int>::max();

    for (auto& mv : moves) {
        CheckersBoard temp = *this;
        temp.whiteToMove = maximizing;
        if (!temp.makeMove(mv)) {
            continue;
        }
        int eval = temp.minimax(depth - 1,
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(),
            !maximizing);
        if (maximizing) {
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = mv;
            }
        }
        else {
            if (eval < bestEval) {
                bestEval = eval;
                bestMove = mv;
            }
        }
    }
    return bestMove;
}

// Парсинг строки: "A3 B4 C5" -> Move
bool CheckersBoard::parseUserMove(const std::string& input, Move& move) {
    std::vector<std::string> tokens;
    {
        std::string tmp;
        for (char ch : input) {
            if (std::isspace(static_cast<unsigned char>(ch))) {
                if (!tmp.empty()) {
                    tokens.push_back(tmp);
                    tmp.clear();
                }
            }
            else {
                tmp.push_back(ch);
            }
        }
        if (!tmp.empty()) {
            tokens.push_back(tmp);
        }
    }
    if (tokens.size() < 2) {
        return false;
    }

    std::vector<Coord> path;
    for (auto& tk : tokens) {
        if (tk.size() < 2) return false;
        char colCh = std::toupper(tk[0]);
        char rowCh = tk[1];
        if (colCh < 'A' || colCh > 'H') return false;
        if (rowCh < '1' || rowCh > '8') return false;

        int c = colCh - 'A';      // колонка
        int r = (rowCh - '1');    // строка
        path.push_back(Coord(r, c));
    }

    move.path = path;
    return true;
}

// === Вспомогательные методы ===

// Проверка границ
bool CheckersBoard::isValidPos(int r, int c) const {
    return (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE);
}

// Проверка, принадлежит ли шашка p данному цвету (whiteSide?)
bool CheckersBoard::isColor(Piece p, bool whiteSide) const {
    if (whiteSide) {
        return (p == Piece::W || p == Piece::DW);
    }
    else {
        return (p == Piece::B || p == Piece::DB);
    }
}

// Генерация всех рубящих ходов для (r,c)
void CheckersBoard::getAllCapturesForPiece(int r, int c, std::vector<Move>& captures) {
    Piece p = board[r][c];
    if (p == Piece::EMPTY) return;

    // Стартуем DFS с путём, где первая клетка — (r,c)
    std::vector<Coord> path;
    path.push_back(Coord(r, c));

    std::vector<Move> results;
    dfsCaptures(path, results);
    if (!results.empty()) {
        captures.insert(captures.end(), results.begin(), results.end());
    }
}

// Рекурсивный поиск цепочек рубки. Учитываем, что дамка может бить «далеко».
void CheckersBoard::dfsCaptures(std::vector<Coord>& path, std::vector<Move>& results) {
    Coord cur = path.back();
    Piece p = board[cur.r][cur.c];

    bool isKing = (p == Piece::DW || p == Piece::DB);
    bool pIsWhite = isColor(p, true);

    bool foundCapture = false;

    static const int DR[4] = { 1, 1, -1, -1 };
    static const int DC[4] = { 1, -1, 1, -1 };

    for (int i = 0; i < 4; ++i) {
        int stepR = DR[i];
        int stepC = DC[i];

        if (!isKing) {
            int nr = cur.r + 2 * stepR;
            int nc = cur.c + 2 * stepC;
            int mr = cur.r + stepR;
            int mc = cur.c + stepC;

            if (!isValidPos(nr, nc) || !isValidPos(mr, mc)) {
                continue;
            }
            if (isColor(board[mr][mc], !pIsWhite) &&
                board[nr][nc] == Piece::EMPTY)
            {
                Piece savedMid = board[mr][mc];
                Piece savedCur = board[cur.r][cur.c];
                Piece savedLanding = board[nr][nc];

                board[cur.r][cur.c] = Piece::EMPTY;
                board[mr][mc] = Piece::EMPTY;
                board[nr][nc] = p;

                path.push_back(Coord(nr, nc));
                dfsCaptures(path, results);

                path.pop_back();
                board[cur.r][cur.c] = savedCur;
                board[mr][mc] = savedMid;
                board[nr][nc] = savedLanding;

                foundCapture = true;
            }
        }
        else {
            int r2 = cur.r + stepR;
            int c2 = cur.c + stepC;

            bool foundOpponent = false;
            int oppR = -1, oppC = -1;
            while (isValidPos(r2, c2)) {
                if (board[r2][c2] != Piece::EMPTY) {
                    if (!foundOpponent && isColor(board[r2][c2], !pIsWhite)) {
                        foundOpponent = true;
                        oppR = r2;
                        oppC = c2;
                    }
                    else {
                        break;
                    }
                }
                r2 += stepR;
                c2 += stepC;
            }

            if (!foundOpponent) {
                continue;
            }

            int landingR = oppR + stepR;
            int landingC = oppC + stepC;

            while (isValidPos(landingR, landingC) &&
                board[landingR][landingC] == Piece::EMPTY)
            {
                // Виртуально рубим
                Piece savedOpp = board[oppR][oppC];
                Piece savedCur = board[cur.r][cur.c];
                Piece savedLand = board[landingR][landingC];

                board[cur.r][cur.c] = Piece::EMPTY;
                board[oppR][oppC] = Piece::EMPTY;
                board[landingR][landingC] = p;

                path.push_back(Coord(landingR, landingC));
                dfsCaptures(path, results);

                // Откат
                path.pop_back();
                board[cur.r][cur.c] = savedCur;
                board[oppR][oppC] = savedOpp;
                board[landingR][landingC] = savedLand;

                foundCapture = true;
                landingR += stepR;
                landingC += stepC;
            }
        }
    }

    if (!foundCapture) {
        if (path.size() > 1) {
            results.push_back(Move(path));
        }
    }
}

// Генерация обычных ходов (без взятия)
void CheckersBoard::getAllNormalMovesForPiece(int r, int c, std::vector<Move>& moves) {
    Piece p = board[r][c];
    if (p == Piece::EMPTY) return;

    static const int DR[4] = { 1, -1, 1, -1 };
    static const int DC[4] = { 1, 1, -1, -1 };

    bool isKing = (p == Piece::DW || p == Piece::DB);

    for (int i = 0; i < 4; ++i) {
        int stepR = DR[i];
        int stepC = DC[i];

        if (!isKing) {
            int nr = r + stepR;
            int nc = c + stepC;
            if (p == Piece::W && stepR < 0) {
                continue;
            }
            if (p == Piece::B && stepR > 0) {
                continue;
            }

            if (isValidPos(nr, nc) && board[nr][nc] == Piece::EMPTY) {
                // Формируем ход
                std::vector<Coord> path;
                path.push_back(Coord(r, c));
                path.push_back(Coord(nr, nc));
                moves.push_back(Move(path));
            }
        }
        else {
            int nr = r + stepR;
            int nc = c + stepC;
            while (isValidPos(nr, nc) && board[nr][nc] == Piece::EMPTY) {
                std::vector<Coord> path;
                path.push_back(Coord(r, c));
                path.push_back(Coord(nr, nc));
                moves.push_back(Move(path));

                nr += stepR;
                nc += stepC;
            }
        }
    }
}