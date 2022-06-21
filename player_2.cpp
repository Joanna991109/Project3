#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>

enum SPOT_STATE {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

int player;
const int SIZE = 15;
const int MAX_DEPTH = 2;
std::array<std::array<int, SIZE>, SIZE> board;
int boardX[] = {7, 6, 7, 8, 8, 8, 7, 6, 6,
                5, 6, 7, 8, 9, 9, 9, 9, 9, 8, 7, 6, 5, 5, 5, 5,
                4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4,
                3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 9, 8, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3,
                2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int boardY[] = {7, 6, 6, 6, 7, 8, 8, 8, 7,
                5, 5, 5, 5, 5, 6, 7, 8, 9, 9, 9, 9, 9, 8, 7, 6,
                4, 4, 4, 4, 4, 4, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5,
                3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 9, 8, 7, 6, 5, 4,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
                14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

struct Point {
    int x, y;
    Point() : Point(0, 0) {}
    Point(float x, float y) : x(x), y(y) {}
    bool operator==(const Point& rhs) const {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Point& rhs) const {
        return !operator==(rhs);
    }
    Point operator+(const Point& rhs) const {
        return Point(x + rhs.x, y + rhs.y);
    }
    Point operator-(const Point& rhs) const {
        return Point(x - rhs.x, y - rhs.y);
    }
};

// minimax
struct Node {
    std::array<std::array<int, SIZE>, SIZE> board;
    int score;
    int cur_player;
    int depth;
    int spot_X, spot_Y;
    Node* parent;
    std::vector<Node*> children;
    Point next_spot;
    int next_spot_value;

    Node() {
        parent = nullptr;
        next_spot_value = -0x7fffffff;
    }
};

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

const int pt_2 = 1;
const int pt_3_1 = 10;
const int pt_3_2 = 1000;
const int pt_4_1 = 100;
const int pt_4_2 = 10000;
int state_value(std::array<std::array<int, SIZE>, SIZE>& board, int Player) {
    int value = 0;
    int opp = 3 - Player;
    int num = 0, emp = 0;

    for (int i = 0; i < SIZE; i++) {
        num = 0, emp = 0;
        for (int j = 0; j <= SIZE; j++) {
            if (j < SIZE && board[i][j] == Player)
                num++;
            else {
                if (j < SIZE && board[i][j] == EMPTY)
                    emp++;
                if (j - num - 1 >= 0)
                    if (board[i][j - num - 1] == EMPTY) emp++;

                if (num == 2)
                    value += pt_2;
                else if (num == 3) {
                    if (emp == 2)
                        value += pt_3_2;
                    else if (emp == 1)
                        value += pt_3_1;
                } else if (num == 4) {
                    if (emp == 2)
                        value += pt_4_2;
                    else if (emp == 1)
                        value += pt_4_1;
                }
                num = 0, emp = 0;
            }
        }
    }
    for (int j = 0; j < SIZE; j++) {
        num = 0, emp = 0;
        for (int i = 0; i <= SIZE; i++) {
            if (i < SIZE && board[i][j] == Player)
                num++;
            else {
                if (i < SIZE && board[i][j] == EMPTY)
                    emp++;
                if (i - num - 1 >= 0)
                    if (board[i - num - 1][j] == EMPTY) emp++;

                if (num == 2)
                    value += pt_2;
                else if (num == 3) {
                    if (emp == 2)
                        value += pt_3_2;
                    else if (emp == 1)
                        value += pt_3_1;
                } else if (num == 4) {
                    if (emp == 2)
                        value += pt_4_2;
                    else if (emp == 1)
                        value += pt_4_1;
                }
                num = 0, emp = 0;
            }
        }
    }
    for (int i = 0; i < 11; i++) {
        num = 0, emp = 0;
        for (int k = 0; i + k <= SIZE; k++) {
            if (i + k < SIZE && board[i + k][k] == Player)
                num++;
            else {
                if (i + k < SIZE && board[i + k][k] == EMPTY)
                    emp++;
                if (k - num - 1 >= 0)
                    if (board[i + k - num - 1][k - num - 1] == EMPTY) emp++;

                if (num == 2)
                    value += pt_2;
                else if (num == 3) {
                    if (emp == 2)
                        value += pt_3_2;
                    else if (emp == 1)
                        value += pt_3_1;
                } else if (num == 4) {
                    if (emp == 2)
                        value += pt_4_2;
                    else if (emp == 1)
                        value += pt_4_1;
                }
                num = 0, emp = 0;
            }
        }
    }
    for (int i = 1; i < 11; i++) {
        num = 0, emp = 0;
        for (int k = 0; i + k <= SIZE; k++) {
            if (i + k < SIZE && board[k][i + k] == Player)
                num++;
            else {
                if (i + k < SIZE && board[k][i + k] == EMPTY)
                    emp++;
                if (k - num - 1 >= 0)
                    if (board[k - num - 1][i + k - num - 1] == EMPTY) emp++;

                if (num == 2)
                    value += pt_2;
                else if (num == 3) {
                    if (emp == 2)
                        value += pt_3_2;
                    else if (emp == 1)
                        value += pt_3_1;
                } else if (num == 4) {
                    if (emp == 2)
                        value += pt_4_2;
                    else if (emp == 1)
                        value += pt_4_1;
                }
                num = 0, emp = 0;
            }
        }
    }
    for (int i = 14; i >= 4; i--) {
        num = 0, emp = 0;
        for (int k = 0; i - k >= -1; k++) {
            if (i - k >= 0 && board[i - k][k] == Player)
                num++;
            else {
                if (i - k >= 0 && board[i - k][k] == EMPTY)
                    emp++;
                if (k - num - 1 >= 0)
                    if (board[i - k + num + 1][k - num - 1] == EMPTY) emp++;

                if (num == 2)
                    value += pt_2;
                else if (num == 3) {
                    if (emp == 2)
                        value += pt_3_2;
                    else if (emp == 1)
                        value += pt_3_1;
                } else if (num == 4) {
                    if (emp == 2)
                        value += pt_4_2;
                    else if (emp == 1)
                        value += pt_4_1;
                }
                num = 0, emp = 0;
            }
        }
    }

    for (int i = 1; i < 11; i++) {
        num = 0, emp = 0;
        for (int k = 0; i + k <= SIZE; k++) {
            if (i + k <= SIZE && board[14 - k][i + k] == Player)
                num++;
            else {
                if (i + k <= SIZE && board[14 - k][i + k] == EMPTY)
                    emp++;
                if (14 - k + num + 1 < SIZE)
                    if (board[14 - k + num + 1][i + k - num - 1] == EMPTY) emp++;

                if (num == 2)
                    value += pt_2;
                else if (num == 3) {
                    if (emp == 2)
                        value += pt_3_2;
                    else if (emp == 1)
                        value += pt_3_1;
                } else if (num == 4) {
                    if (emp == 2)
                        value += pt_4_2;
                    else if (emp == 1)
                        value += pt_4_1;
                }
                num = 0, emp = 0;
            }
        }
    }
    return value;
}

int state_value_total(int Player, int opp) {
    return Player - opp;
}

bool win(std::array<std::array<int, SIZE>, SIZE>& cur_board, int Player, int x, int y) {
    int cnt = 1;
    for (int y2 = y + 1; y2 < SIZE && y2 < y + 5 && cur_board[x][y2] == Player; y2++) cnt++;
    for (int y2 = y - 1; y2 >= 0 && y2 > y - 5 && cur_board[x][y2] == Player; y2--) cnt++;
    if (cnt >= 5) return true;
    cnt = 1;
    for (int x2 = x + 1; x2 < SIZE && x2 < x + 5 && cur_board[x2][y] == Player; x2++) cnt++;
    for (int x2 = x - 1; x2 >= 0 && x2 > x - 5 && cur_board[x2][y] == Player; x2--) cnt++;
    if (cnt >= 5) return true;
    cnt = 1;
    for (int x2 = x + 1, y2 = y + 1; x2 < SIZE && x2 < x + 5 && y2 < SIZE && y2 < y + 5 && cur_board[x2][y2] == Player; x2++, y2++) cnt++;
    for (int x2 = x - 1, y2 = y - 1; x2 >= 0 && x2 > x - 5 && y2 >= 0 && y2 > y - 5 && cur_board[x2][y2] == Player; x2--, y2--) cnt++;
    if (cnt >= 5) return true;
    cnt = 1;
    for (int x2 = x + 1, y2 = y - 1; x2 < SIZE && x2 < x + 5 && y2 >= 0 && y2 > y - 5 && cur_board[x2][y2] == Player; x2++, y2--) cnt++;
    for (int x2 = x - 1, y2 = y + 1; x2 >= 0 && x2 > x - 5 && y2 < SIZE && y2 < y + 5 && cur_board[x2][y2] == Player; x2--, y2++) cnt++;
    if (cnt >= 5) return true;
    return false;
}

/* int minimax(Node* node) {
    if (node->depth == MAX_DEPTH) {
        return -state_value1(node->board, node->cur_player);
    }
    int max_val = -0x7fffffff;
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (node->board[x][y] == EMPTY) {
                Node* nextnode = new Node;
                nextnode->board = node->board;
                nextnode->board[x][y] = node->cur_player;
                nextnode->cur_player = 3 - node->cur_player;
                nextnode->depth = node->depth + 1;
                nextnode->spot_X = x;
                nextnode->spot_Y = y;

                int next_val = minimax(nextnode);
                if (next_val > max_val) {
                    max_val = next_val;
                    node->next_spot = Point(x, y);
                    node->next_spot_value = next_val;
                }
                delete nextnode;
                if (max_val >= 100000)
                    return -node->next_spot_value;
            }
        }
    }
    return -node->next_spot_value;
} */
int near(std::array<std::array<int, SIZE>, SIZE>& cur_board, int x, int y) {
    if (cur_board[7][7] == EMPTY) return true;
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            if (x + i >= 0 && x + i < SIZE && y + j >= 0 && y + j < SIZE) {
                if (cur_board[x + i][y + j] != EMPTY) return true;
            }
        }
    }
    return false;
}
int alpha_beta(std::array<std::array<int, SIZE>, SIZE>& cur_board, int depth, int alpha, int beta, int cur_player, std::ofstream& fout) {
    if (depth == 0)
        return state_value_total(state_value(cur_board, player), state_value(cur_board, 3 - player));
    if (cur_player == player) {
        int value = -0x7fffffff;
        for (int i = 0; i < 225; i++) {
            int x = boardX[i];
            int y = boardY[i];
            if (cur_board[x][y] == EMPTY && near(cur_board, x, y)) {
                cur_board[x][y] = cur_player;
                if (win(cur_board, cur_player, x, y))
                    value = 0x7fffffff;
                else
                    value = std::max(value, alpha_beta(cur_board, depth - 1, alpha, beta, 3 - cur_player, fout));
                if (value > alpha || value == -0x7fffffff) {  // alpha = max(alpha, value);
                    alpha = value;
                    fout << x << " " << y << "\n";
                    fout.flush();
                }
                cur_board[x][y] = EMPTY;

                if (beta <= alpha)
                    break;
            }
        }
        return value;
    } else {
        int value = 0x7fffffff;
        for (int i = 0; i < 225; i++) {
            int x = boardX[i];
            int y = boardY[i];
            if (cur_board[x][y] == EMPTY && near(cur_board, x, y)) {
                cur_board[x][y] = cur_player;
                if (win(cur_board, cur_player, x, y))
                    value = -0x7fffffff;
                else
                    value = std::min(value, alpha_beta(cur_board, depth - 1, alpha, beta, 3 - cur_player, fout));
                beta = std::min(beta, value);
                cur_board[x][y] = EMPTY;
                if (beta <= alpha)
                    break;
            }
        }
        return value;
    }
}

// minimax
/* void write_valid_spot1(std::ofstream& fout) {
    srand(time(NULL));
    Node* root = new Node;
    root->board = board;
    root->cur_player = player;
    root->depth = 0;
    minimax(root);
    fout << root->next_spot.x << " " << root->next_spot.y << std::endl;
    fout.flush();
    delete root;
} */

// alpha beta pruning
void write_valid_spot2(std::ofstream& fout) {
    srand(time(NULL));
    // Node* root = new Node;
    // root->board = board;
    // root->cur_player = player;
    // root->depth = 0;
    // minimax(root);
    alpha_beta(board, 3, -0x7fffffff, 0x7fffffff, player, fout);

    // fout << root->next_spot.x << " " << root->next_spot.y << std::endl;
    fout.flush();
    // delete root;
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    // std::cout << state_value(board, player) << '\n';
    //  write_valid_spot1(fout);        //minimax
    write_valid_spot2(fout);  // alpha beta1

    fin.close();
    fout.close();
    return 0;
}
