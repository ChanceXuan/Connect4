//注意COMPUTER_WIN代表我的AI赢！！！
 
#include "Judge.h" //包含判断胜负的函数userWin，machineWin和isTie
#include <ctime>
#include <cmath>
#include <cstdlib>

const int userGo = 1;
const int machineGo = 2;
const int PLAYER_WIN_PROFIT = -1; //玩家获胜时的收益
const int COMPUTER_WIN_PROFIT = 1; //我方AI获胜时的收益
const int TIE_PROFIT = 0; //平局收益

#define UNTERMINAL_STATE 2 //非终止状态

using namespace std;

class Node
{
public:
	int _depth; //节点深度
	int _profit; //当前状态我方收益
	int _visitedNum; //被访问次数
	int _M,_N;

	Node* parent; //构造父节点指针
	Node** child; //构造子节点指针
	
	//构造函数
	Node()
	{
		_profit = 0;
		_visitedNum = 0;
		parent = NULL;
		child = new Node*[_N]; //指针children指向数组N个子节点数组，每个数组存放Node*类型的元素
		for (int i = 0; i < _N; i ++) 
			child[i] = NULL;
	}

	//析构函数
	~Node()
	{
		for (int i = 0; i < 12; i ++)
			if(child[i])
				delete child[i];
	}
};

class State
{
public:
	int _M,_N; //棋盘大小（M,N）
	int *topState; //顶端状态
	int **boardState; //棋局状态
	int _noX, _noY; //不可落子点位置 
	int _lastX, _lastY; //上一次落子位置
	int _whosTurn; //持子属性 

	//构造函数
	State(int **board, int *top, int noX, int noY, int lastX, int lastY, int playingRight, const int M, const int N)
	{
		boardState = board;
		topState = top;
		_noX = noX; _noY = noY;
		_lastX = lastX; _lastY = lastY;
		_M = M; _N = N;
		_whosTurn = playingRight;
	}

	//析构函数
	~State()
	{
		for (int i = 0; i < _M; i++)
			delete [] boardState[i];
		delete [] boardState;
	}
};

void initBoard();
void clearTree(Node*, Node*, State*);
Node* Expand(Node*);
Node* TreePolicy(Node*, State*);
int DefaultPolicy(State*, Node*);
void Backup(Node*, int);