#include <iostream>
#include "UCT.h"

#include <conio.h>
#include <atlstr.h>

const int RANGE = 12;
const int maxNodeNumber = 9000000;
const int maxDepth = 9000000;

int expandRound = 1;

int x=-1; int y=-1;
int M,N;
int noX,noY;
int** board;
int* top;
int depth = 0;
int whosTurn; //该谁下,1表示user下，2表示machine下

using namespace std;

//统计棋盘上棋子数目
int countChess(int** _board, int row,int column)
{
	int count = 0;
	for(int i = 0;i < row;i++)
		for(int j = 0;j < column;j++)
			if(_board[i][j] > 0)
				count++;
	return count;
}

void initConst(int* _top, int _M, int _N, int _noX, int _noY)
{
	top = _top;
	M = _M;
	N = _N;
	noX = _noX;
	noY = _noY;
}

//初始化棋盘，将其全部置为0
int** initBoard()
{
	board = new int*[M];
	for(int i = 0; i < M; i++)
	{
		board[i] = new int[N];
		for(int j = 0; j < N; j++)
			board[i][j] = 0;
	}
	return board;
}

bool isExpandable(Node* judgeNode) //是否可扩展
{
	return judgeNode->expandableNum > 0;
}

bool isTerminal(State* judgeState) //是否为终止节点
{
	if (x == -1 && y == -1) //若为根节点
	{
		//_cprintf("presentNode is root!\n");
		return false;
	}
	if ((judgeState->_whosTurn == userGo && machineWin(x, y, M, N, board)) || //计算机胜利 
		(judgeState->_whosTurn == machineGo && userWin(x, y, N, N, board)) || //玩家胜利 
		(isTie(N, top))) //平局
	{
		if (judgeState->_whosTurn == userGo && machineWin(x, y, M, N, board))
		if (judgeState->_whosTurn == machineGo && userWin(x, y, N, N, board))
		if (isTie(N, top))
		return true;
	}
	return false;
}

//最优子节点
Node* bestChild(Node* presentNode, State* presentState, double coefficient)
{
	Node* best;
	Node** childNode = presentNode->child;
	whosTurn = presentState->_whosTurn;
	int visitedNum = presentNode->_visitedNum; //父节点访问数 
	double maxProfitRatio = -RAND_MAX;
	int bestColumn;

	int j = 1;
	for (int i = 0; i < N; i ++)
	{
		if (childNode[i] == NULL) continue;
		int judegeGo = (whosTurn == machineGo ? 1 : -1);
		int modifiedProfit = (whosTurn == machineGo ? 1 : -1) * childNode[i]->_profit; //修正收益值
		int childVisitedNum = childNode[i]->_visitedNum; //子节点访问数 
		double knownProfit = modifiedProfit*1.0/childVisitedNum;
		double unknownProfit = coefficient * sqrt(2 * log(visitedNum) / childVisitedNum);
		double tempProfitRatio = knownProfit + unknownProfit; //计算综合收益率 

		//选择综合收益率最大的子节点 
		if (tempProfitRatio > maxProfitRatio || (tempProfitRatio == maxProfitRatio && rand() % 2 == 0))
		{
			maxProfitRatio = tempProfitRatio;
			best = childNode[i];
			_cprintf("choose childNode%d\n",i);
			bestColumn = i;
		}
	}
	y = bestColumn;
	x = top[y] - 1;
	if(y == noY && x == noX)
		x--;
	_cprintf("-----------------> After move, x = %d,y = %d\n", x,y);
	return best;
}

//搜索树策略
Node* TreePolicy(Node* presentNode, State* presentState)//传入root节点和root节点的状态
{
	while (!isTerminal(presentState)) //节点是否处于终止状态
	{
		if (isExpandable(presentNode)) //且拥有未被访问的子状态
		{
			presentNode = Expand(presentNode, presentState); //扩展该节点
			presentNode = presentNode->child[y];
			return presentNode;
		}
		else
			presentNode = bestChild(presentNode, presentState, 0.8); //选择最优子节点 
	}
	return presentNode;
}

void getMove(int& moveX, int& moveY)
{
	moveX = x;
	moveY = y;
}

//棋权变换 
int rightChange(int chessman)
{
	if (chessman == userGo)
		return machineGo;
	else if (chessman == machineGo)
		return userGo;
	else
		return 0;
} 

//扩展节点
Node* Expand(Node* presentNode, State* presentState)
{
	int expandableNum = presentNode->expandableNum;
	int index = rand() % expandableNum; //随机确定一个索引值 
	y = presentNode->expandableNode[index];
	x = top[y] - 1; //确定落子坐标
	if (x == noX && y == noY) //若落子位置的正上方位置是不可落子点 
		x--; //更新棋盘顶端状态数组

	//为当前节点创建扩展子节点 
	presentNode->child[y] = new Node(M, N);
	presentNode->child[y]->parent = presentNode;
	swap(presentNode->expandableNode[index], presentNode->expandableNode[--expandableNum]); //将被选中子节点编号置换到目录末尾
	return presentNode;
}

State* stateUpdate(State* presentState)
{
	presentState->_lastX = x;
	presentState->_lastY = y; //上一次落子位置
	return presentState;
}

//随机落子 
State* moveSimulation(State *simulationState)
{
	int simulationY = rand() % N; //随机选择一列 
	while (simulationState->topState[simulationY] == 0) //若此列已下满 
		simulationY = rand() % N; //再随机选择一列

	int simulationX = --simulationState->topState[simulationY];

	simulationState->boardState[simulationX][simulationY] = simulationState->_whosTurn; //落子 

	if (simulationX == noX && simulationY == noY) //若落子位置正上方紧邻不可落子点 
		simulationX--;

	simulationState->_lastX = simulationX;
	simulationState->_lastY = simulationY;

	return simulationState;
}

//计算当前状态收益
int Profit(State* presentState)
{
	int simulationX = presentState->_lastX;
	int simulationY = presentState->_lastY;
	whosTurn = presentState->_whosTurn;
	if (whosTurn == userGo && userWin(simulationX, simulationY, M, N, presentState->boardState))
		return PLAYER_WIN_PROFIT;
	if (whosTurn == machineGo && machineWin(simulationX, simulationY, M, N, presentState->boardState))
		return COMPUTER_WIN_PROFIT;
	if (isTie(N, presentState->topState))
		return TIE_PROFIT;
	return UNTERMINATED_STATE; //未进入终止状态 
}

int DefaultPolicy(State* simulationState)
{
	int profit = UNTERMINATED_STATE;
	int round = 0;
	while (profit == UNTERMINATED_STATE) //若当前状态未达终止状态 
	{
		simulationState = moveSimulation(simulationState); //随机落子 
		profit = Profit(simulationState); //计算收益
		simulationState->_whosTurn = rightChange(simulationState->_whosTurn);
	}
	return profit;
}

//回溯更新
void Backup(int deltaProfit, Node* expandedNode)
{
	Node* tempNode = expandedNode;
	while (tempNode)
	{
		tempNode->_visitedNum++; //访问次数+1 
		tempNode->_profit += deltaProfit; //收益增加delta 
		deltaProfit = -deltaProfit; //极大极小原理
		tempNode = tempNode->parent;
	}
}