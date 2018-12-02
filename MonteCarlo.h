#ifndef _MonteCarlo_H_
#define _MonteCarlo_H_

void moveChess(const int , const int , const int* , int** , const int , const int , int, int, int& , int&);
int buildNewChild(int,int);
int chooseMaxPutDown(int);
void init_board();
void clearTree();
void copyInfo(const int,const int,const int*,int**,const int, const int,const int, const int,const int);
void recoverBoardInfo();
int canPutThiscolumn(int);
void putChess(int,int);
int simulation(int);
int expansion(int);
int select();
int UCT();
int chooseMaxPutDown(int);
int getNextPos(int);
void backpropagation(int);
bool rootDownAfterOtherGo();
int chessNum(int **,int,int);

#endif