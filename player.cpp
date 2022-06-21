#include <array>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <queue>
#include <cmath>
#include <algorithm>

enum SPOT_STATE {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

int player;
const int SIZE = 15;
const int MAX_DEPTH = 2;
std::array<std::array<int, SIZE>, SIZE> board;

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

    Node(){
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

int state_value1(std::array<std::array<int, SIZE>, SIZE> board, int player) {
    int value = 0;
    int tmp = 0, opp = 3 - player;
    int num[3]={0, 0, 0};
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            if (board[i][j] == BLACK) tmp++;
        }
    }
    if(tmp==1 && board[7][7] == BLACK) return 1;

    //橫
    for (int i = 0; i < SIZE; i++) {
        tmp = 0;
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == player) {
                tmp++;
            } else {
                if(tmp == 2) value += 1;
                else if (tmp == 3)
                    value += 5;
                else if (tmp == 4)
                    value += 8;
                else if (tmp == 5) {
                    value += 10000;
                    return value;
                }
                tmp = 0;
            }
        }
    }
    //直
    for (int i = 0; i < SIZE; i++) {
        tmp = 0;
        for (int j = 0; j < SIZE; j++) {
            if (board[j][i] == player) {
                tmp++;
            } else {
                if(tmp == 2) value += 1;
                else if (tmp == 3)
                    value += 5;
                else if (tmp == 4)
                    value += 8;
                else if (tmp == 5) {
                    value += 10000;
                    return value;
                }
                tmp = 0;
            }
        }
    }
    //左上到右下
    for (int i = 0; i < 11; i++) {
        tmp = 0;
        for (int k = 0; k + i < SIZE; k++) {
            if (board[i + k][k] == player) {
                tmp++;
            } else {
                if(tmp == 2) value += 1;
                else if (tmp == 3)
                    value += 5;
                else if (tmp == 4)
                    value += 8;
                else if (tmp == 5) {
                    value += 10000;
                    return value;
                }
                tmp = 0;
            }
        }
    }
    for (int i = 1; i < 11; i++) {
        tmp = 0;
        for (int k = 0; k + i < SIZE; k++) {
            if (board[k][i + k] == player) {
                tmp++;
            } else {
                if(tmp == 2) value += 1;
                else if (tmp == 3)
                    value += 5;
                else if (tmp == 4)
                    value += 8;
                else if (tmp == 5) {
                    value += 10000;
                    return value;
                }
                tmp = 0;
            }
        }
    }
    //左下到右上
    for (int i = 14; i > 3; i--) {
        tmp = 0;
        for (int k = 0; i - k < SIZE && i - k >= 0; k++) {
            if (board[i - k][k] == player) {
                tmp++;
            } else {
                if(tmp == 2) value += 1;
                else if (tmp == 3)
                    value += 5;
                else if (tmp == 4)
                    value += 8;
                else if (tmp == 5) {
                    value += 10000;
                    return value;
                }
                tmp = 0;
            }
        }
    }
    for (int i = 1; i < 11; i++) {
        tmp = 0;
        for (int k = 0; 14 - k - i >= 0 && k < SIZE; k++) {
            if (board[14 - k - i][k] == player) {
                tmp++;
            } else {
                if(tmp == 2) value += 1;
                else if (tmp == 3)
                    value += 5;
                else if (tmp == 4)
                    value += 8;
                else if (tmp == 5) {
                    value += 10000;
                    return value;
                }
                tmp = 0;
            }
        }
    }

    return value;
}

int state_value2(std::array<std::array<int, SIZE>, SIZE> board, int player, int x, int y) {
    int value = 0;
    int tmp = 0, opp_num = 0;
    int opp = 3 - player;
    bool head = 0; //0->empty 1->opp
    int dirX[8] = {0, 0, -1, 1, -1, -1, 1, 1};
    int dirY[8] = {-1, 1, 0, 0, -1, 1, -1, 1};

    //棋盤空的就下正中間
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            if (board[i][j] == BLACK) tmp++;
        }
    }
    if(tmp == 1 && board[7][7] == BLACK) return 1;

    //對方連三或蓮寺的時候要考慮擋住
    //(x, y)是最後下的那個點，防守只看它的四周，從它往八個方向延伸看看有沒有做到防守
    for(int d = 0; d < 8; d++){
        opp_num = 0;
        for(int i = 1; i <= 5; i++){
            int X = x + ( i * dirX[d] );
            int Y = y + ( i * dirY[d] );
            if(X >= 0 && X < SIZE && Y >= 0 && Y < SIZE){
                if(board[X][Y] == opp){
                    opp_num++;
                }else if(board[X][Y] == player){
                    if(opp_num == 3){         //夾住一組3
                        value += 200;
                    }else if(opp_num == 4){   //夾住一組4
                        value += 10000;
                        //return value;
                    }
                    opp_num = 0;
                }else{
                    if(opp_num == 3){         //堵一組兩邊開的3的一邊
                        value += 250;
                    }else if(opp_num == 4){   //堵一組兩邊開的4的一邊
                        value += 5;
                    }
                    opp_num = 0;
                }
            }else{
                if(opp_num == 3){
                    value += 200;
                }else if(opp_num == 4){
                    value += 10000;
                }
                opp_num = 0;
            }
        }
    }

    //算自己有多少連線
    //橫
    for (int i = 0; i < SIZE; i++) {
        tmp = 0;
        head = false;
        for (int j = 0; j < SIZE; j++) {
            if (board[i][j] == player) {
                tmp++;
            } else if(board[i][j] == opp){
                if(head != true){
                    if(tmp == 2) value += 2;
                    else if(tmp == 3) value += 30;
                    else if(tmp == 4) value += 100;
                    else if(tmp == 5) return 100000;
                }
                tmp = 0;
                head = true;
            }else {
                if(head != true){
                    if(tmp == 2) value += 3;
                    else if (tmp == 3) value += 100;
                    else if (tmp == 4) value += 400;
                    else if (tmp == 5) return 100000;
                }else{
                    if(tmp == 2) value += 2;
                    else if (tmp == 3) value += 30;
                    else if (tmp == 4) value += 100;
                    else if (tmp == 5) return 100000;
                }
                tmp = 0;
                head = false;
            }
        }
    }
    //直
    for (int i = 0; i < SIZE; i++) {
        tmp = 0;
        for (int j = 0; j < SIZE; j++) {
            if (board[j][i] == player) {
                tmp++;
            } else if(board[j][i] == opp){
                if(head != true){
                    if(tmp == 2) value += 2;
                    else if(tmp == 3) value += 30;
                    else if(tmp == 4) value += 100;
                    else if(tmp == 5) return 100000;
                }
                tmp = 0;
                head = true;
            }else {
                if(head != true){
                    if(tmp == 2) value += 3;
                    else if (tmp == 3) value += 100;
                    else if (tmp == 4) value += 400;
                    else if (tmp == 5) return 100000;
                }else{
                    if(tmp == 2) value += 2;
                    else if (tmp == 3) value += 30;
                    else if (tmp == 4) value += 100;
                    else if (tmp == 5) return 100000;
                }
                tmp = 0;
                head = false;
            }
        }
    }
    //左上到右下
    for (int i = 0; i < 11; i++) {
        tmp = 0;
        for (int k = 0; k + i < SIZE; k++) {
            if (board[i + k][k] == player) {
                tmp++;
            } else if(board[i + k][k] == opp){
                if(head != true){
                    if(tmp == 2) value += 2;
                    else if(tmp == 3) value += 30;
                    else if(tmp == 4) value += 100;
                    else if(tmp == 5) return 100000;
                }
                tmp = 0;
                head = true;
            }else {
                if(head != true){
                    if(tmp == 2) value += 3;
                    else if (tmp == 3) value += 100;
                    else if (tmp == 4) value += 400;
                    else if (tmp == 5) return 100000;
                }else{
                    if(tmp == 2) value += 2;
                    else if (tmp == 3) value += 30;
                    else if (tmp == 4) value += 100;
                    else if (tmp == 5) return 100000;
                }
                tmp = 0;
                head = false;
            }
        }
    }
    for (int i = 1; i < 11; i++) {
        tmp = 0;
        for (int k = 0; k + i < SIZE; k++) {
            if (board[k][i + k] == player) {
                tmp++;
            } else if(board[k][i + k] == opp){
                if(head != true){
                    if(tmp == 2) value += 2;
                    else if(tmp == 3) value += 30;
                    else if(tmp == 4) value += 100;
                    else if(tmp == 5) return 100000;
                }
                tmp = 0;
                head = true;
            }else {
                if(head != true){
                    if(tmp == 2) value += 3;
                    else if (tmp == 3) value += 100;
                    else if (tmp == 4) value += 400;
                    else if (tmp == 5) return 100000;
                }else{
                    if(tmp == 2) value += 2;
                    else if (tmp == 3) value += 30;
                    else if (tmp == 4) value += 100;
                    else if (tmp == 5) return 100000;
                }
                tmp = 0;
                head = false;
            }
        }
    }
    //左下到右上
    for (int i = 14; i > 3; i--) {
        tmp = 0;
        for (int k = 0; i - k < SIZE && i - k >= 0; k++) {
            if (board[i - k][k] == player) {
                tmp++;
            } else if(board[i - k][k] == opp){
                if(head != true){
                    if(tmp == 2) value += 2;
                    else if(tmp == 3) value += 30;
                    else if(tmp == 4) value += 100;
                    else if(tmp == 5) return 100000;
                }
                tmp = 0;
                head = true;
            }else {
                if(head != true){
                    if(tmp == 2) value += 3;
                    else if (tmp == 3) value += 100;
                    else if (tmp == 4) value += 400;
                    else if (tmp == 5) return 100000;
                }else{
                    if(tmp == 2) value += 2;
                    else if (tmp == 3) value += 30;
                    else if (tmp == 4) value += 100;
                    else if (tmp == 5) return 100000;
                }
                tmp = 0;
                head = false;
            }
        }
    }
    for (int i = 1; i < 11; i++) {
        tmp = 0;
        for (int k = 0; 14 - k - i >= 0 && k < SIZE; k++) {
            if (board[14 - k - i][k] == player) {
                tmp++;
            } else if(board[14 - k - i][k] == opp){
                if(head != true){
                    if(tmp == 2) value += 2;
                    else if(tmp == 3) value += 30;
                    else if(tmp == 4) value += 100;
                    else if(tmp == 5) return 100000;
                }
                tmp = 0;
                head = true;
            }else {
                if(head != true){
                    if(tmp == 2) value += 3;
                    else if (tmp == 3) value += 100;
                    else if (tmp == 4) value += 400;
                    else if (tmp == 5) return 100000;
                }else{
                    if(tmp == 2) value += 2;
                    else if (tmp == 3) value += 30;
                    else if (tmp == 4) value += 100;
                    else if (tmp == 5) return 100000;
                }
                tmp = 0;
                head = false;
            }
        }
    }
    
    return value;
}

int minimax(Node* node) {
    if(node->depth == MAX_DEPTH){
        return -state_value2(node->board, node->cur_player, node->spot_X, node->spot_Y);
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
                if(next_val > max_val){
                    max_val = next_val;
                    node->next_spot = Point(x, y);
                    node->next_spot_value = next_val;
                }
                delete nextnode;
                if(max_val >= 100000) return -node->next_spot_value;
            }
        }
    }
    return -node->next_spot_value;
}
/* int alpha_beta(Node* node) {
    if(node->depth == MAX_DEPTH){
        return -state_value(node->board);
    }
    if(node->cur_player==1){
        int max_val = -0x7fffffff;
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                if (node->board[x][y] == EMPTY) {
                    Node* nextnode = new Node;
                    nextnode->board = node->board;
                    nextnode->board[x][y] = node->cur_player;
                    nextnode->cur_player = 3 - node->cur_player;
                    nextnode->depth = node->depth + 1;
                    nextnode->parent=node;

                    int next_val = alpha_beta(nextnode);
                    if(next_val > max_val){
                        max_val = next_val;
                        node->next_spot = Point(x, y);
                        node->next_spot_value = next_val;
                    }
                    delete nextnode;
                    if(max_val>=10000) return -node->next_spot_value;
                    if(node->next_spot_value>=node->parent->next_spot_value) break;
                }
            }
        }
    }else{
        int min_val=0x7fffffff;
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                if (node->board[x][y] == EMPTY) {
                    Node* nextnode = new Node;
                    nextnode->board = node->board;
                    nextnode->board[x][y] = node->cur_player;
                    nextnode->cur_player = 3 - node->cur_player;
                    nextnode->depth = node->depth + 1;
                    nextnode->parent=node;

                    int next_val = alpha_beta(nextnode);
                    if(next_val < min_val){
                        min_val = next_val;
                        node->next_spot = Point(x, y);
                        node->next_spot_value = next_val;
                    }
                    delete nextnode;
                    if(node->next_spot_value<=node->parent->next_spot_value) break;
                }
            }
        }
    }
    return -node->next_spot_value;
} */
int alpha_beta(Node* node) {
    if(node->depth == MAX_DEPTH){
        return -state_value1(node->board, node->cur_player);
    }
    int max_val = -0x7fffffff;
    int min_val = 0x7fffffff;
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (node->board[x][y] == EMPTY) {
                Node* nextnode = new Node;
                nextnode->board = node->board;
                nextnode->board[x][y] = node->cur_player;
                nextnode->cur_player = 3 - node->cur_player;
                nextnode->depth = node->depth + 1;
                nextnode->parent = node;

                int next_val = alpha_beta(nextnode);

                if(node->cur_player == BLACK){
                    if(next_val > max_val){
                        max_val = next_val;
                        node->next_spot = Point(x, y);
                        node->next_spot_value = next_val;
                    }
                    delete nextnode;
                    if(max_val>=100000) return -node->next_spot_value;
                    if(node->parent)
                        if(node->next_spot_value >= node->parent->next_spot_value) break;
                }else {
                    if(next_val < min_val){
                        min_val = next_val;
                        node->next_spot = Point(x, y);
                        node->next_spot_value = next_val;
                    }
                    delete nextnode;
                    //if(max_val>=10000) return -node->next_spot_value;
                    if(node->parent)
                        if(node->next_spot_value <= node->parent->next_spot_value) break;
                }
            }
        }
    }
    return -node->next_spot_value;
}

void write_valid_spot(std::ofstream& fout) {
    srand(time(NULL));
    Node* root = new Node;
    root->board = board;
    root->cur_player = player;
    root->depth = 0;
    //alpha_beta(root);
    minimax(root);

    fout << root->next_spot.x << " " << root->next_spot.y << std::endl;
    fout.flush();
    delete root;
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
