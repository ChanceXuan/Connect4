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

#define UNTERMINATED_STATE 2 //非终止状态

using namespace std;

class Node
{
public:
	int _M,_N;
	int _profit; //当前状态我方收益
	int _visitedNum; //被访问次数
	int expandableNum; //可扩展节点数量 
	int *expandableNode; //可扩展节点编号 

	Node* parent; //构造父节点指针
	Node** child; //构造子节点指针
	
	void clear() //空间释放
	{
		delete [] expandableNode;
		for (int i = 0; i < _N; i ++)
			if (child[i])
			{
				child[i] -> clear();
				delete child[i];
			}
		delete [] child;
	}

	//构造函数
	Node(const int M, const int N)
	{
		_M = M; _N = N;
		_profit = 0;
		_visitedNum = 0;
		parent = NULL;
		child = new Node*[_N]; //指针children指向数组N个子节点数组，每个数组存放Node*类型的元素
		expandableNum = _N;
		expandableNode = new int[_N];
		for (int i = 0; i < _N; i ++)
			child[i] = NULL;
			
	}

	//拷贝构造函数
	Node(const Node &node, const int M, const int N)
	{
		_M = M; _N = N;
		_profit = node._profit;
		_visitedNum = node._visitedNum;
		expandableNum = node.expandableNum;
		*expandableNode = *node.expandableNode;
		*parent = *node.parent;
		**child = **node.child;
	}

	//析构函数
	~Node()
	{
		clear();
	}
};

class State
{
public:
	int _M,_N; //棋盘大小（M,N）
	int *topState; //顶端状态
	int **boardState; //棋局状态
	int _lastX, _lastY; //对手上次的落子位置
	int _whosTurn; //持子属性

	int* TopState(int* _top) const //复制棋盘顶端状态数组topState
	{
		int *presentTop = new int[_N];
		for (int i = 0; i < _N; i ++)
			presentTop[i] = _top[i];
		return presentTop;
	}

	int** BoardState(int** _board) const //复制棋盘状态数组boardState
	{
		int **presentBoardState = new int*[_N];
		for (int i = 0; i < _M; i ++) 
		{
			presentBoardState[i] = new int[_N];
			for(int j = 0; j < _N; j ++)
				presentBoardState[i][j] = _board[i][j];
		}
		return presentBoardState;
	}

	//构造函数
	State(int** board, int* top, int lastX, int lastY, int playingRight, const int M, const int N)
	{
		_M = M; _N = N;
		_lastX = lastX; _lastY = lastY;
		boardState = BoardState(board);
		topState = TopState(top);
		_whosTurn = playingRight;
	}

	//拷贝构造函数
	State(const State &state, const int M, const int N)
	{
		_M = M; _N = N;
		_lastX = state._lastX;
		_lastY = state._lastY;
		boardState = state.boardState;
		topState = state.topState;
		_whosTurn = state._whosTurn;
	}

	//析构函数
	~State()
	{
		for (int i = 0; i < _M; i++)
			delete [] boardState[i];
		delete [] boardState;
		delete [] topState;
	}
};

int countChess(int**, int, int);
int** initBoard();
void initConst(int*, const int, const int, const int, const int);
Node* Expand(Node*, State*);
Node* TreePolicy(Node*, State*);
void getMove(int&, int&);
State* stateUpdate(State*);
int DefaultPolicy(State*);
Node* bestChild(Node*, State*, double);
void Backup(int, Node*);