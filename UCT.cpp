#include "UCT.h"

const int RANGE = 12;
const double coefficient = 0.8;
const int maxNodeNumber = 9000000;
const int maxDepth = 9000000;

int x,y;
int M,N;
int** board;
int* top;
int depth = 0;
int whosTurn;//该谁下,1表示user下，2表示machine下

using namespace std;

//初始化棋盘，将其全部置为0
void initBoard()
{
	if(board == NULL)
	{
		board = new int*[M];
		for(int i = 0; i < M; i++)
		{
			board[i] = new int[N];
			for(int j = 0; j < M; j++)
				board[i][j] = 0;
		}
	}
	else
	{
		for(int i = 0; i < N; i++)
			for(int j = 0; j < M; j++)
				board[i][j] = 0;
	}
}

//初始化蒙特卡洛树
void clearTree(Node* root, Node* simulationNode, State* simulationState)
{
	simulationNode = root; //指向根节点
	simulationState->_whosTurn = machineGo; //根节点由machine先走
}

//计算当前状态收益
int Profit(State* presentState)
{
	x = presentState->_lastX;
	y = presentState->_lastY;
	M = presentState->_M;
	N = presentState->_N;
	board = presentState->boardState;
	top = presentState->topState;
	whosTurn = presentState->_whosTurn;
	if (whosTurn == userGo && userWin(x, y, M, N, board))
		return PLAYER_WIN_PROFIT;
	if (whosTurn == machineGo && machineWin(x, y, M, N, board))
		return COMPUTER_WIN_PROFIT;
	if (isTie(N, top))
		return TIE_PROFIT;
	return UNTERMINAL_STATE; //未进入终止状态 
}

bool isExpandable(Node *judgeNode)//是否可扩展
{
	int expandableNum = 0;
	Node** childNode = new Node*[N]; //大小等于行数的子节点数组 
	int* expandableNode = new int[N]; //可到达子节点编号的数组 
	for (int i = 0; i < N; i ++) 
	{
		if (top[i] != 0) //若第i列可落子 
			expandableNode[expandableNum++] = i;
		childNode[i] = NULL;
	}
	return expandableNum > 0;
}

bool isTerminal(Node *judgeNode, State *judgeState) //是否为终止节点
{
	x = judgeState->_lastX;
	y = judgeState->_lastY;
	M = judgeState->_M;
	N = judgeState->_N;
	board = judgeState->boardState;
	top = judgeState->topState;
	if (x == -1 && y == -1) //若为根节点 
		return false;
	if ((whosTurn == userGo && machineWin(x, y, M, N, board)) || //计算机胜利 
		(whosTurn == machineGo && userWin(x, y, N, N, board)) || //玩家胜利 
		(isTie(N, top))) //平局 
		return true;
	return false;
}

//最优子节点
Node* bestChild(Node* parent, State* presentState)
{
	Node* best;
	Node** childNode = parent->child;
	whosTurn = presentState->_whosTurn;
	int parentVisitedNum = parent->_visitedNum; //父节点访问数 
	int childVisitedNum = childNode[0]->_visitedNum; //子节点访问数 
	double maxProfitRatio = -RAND_MAX;
	for (int i = 0; i < N; i ++)
	{
		if (childNode == NULL) continue;
		double modifiedProfit = (whosTurn == userGo ? -1 : 1) * childNode[i]->_profit; //修正收益值
		double tempProfitRatio = modifiedProfit / childVisitedNum + 
			coefficient * sqrtl(2 * logl(parentVisitedNum) / childVisitedNum); //计算综合收益率 
		//选择综合收益率最大的子节点 
		if (tempProfitRatio > maxProfitRatio || (tempProfitRatio == maxProfitRatio && rand() % 2 == 0))
		{
			maxProfitRatio = tempProfitRatio;
			best = childNode[i];
		}
	}
	return best;
}

//搜索树策略 
Node* TreePolicy(Node *presentNode, State* presentState)
{
	while (!isTerminal(presentNode, presentState)) //节点不是终止节点 
	{
		if (isExpandable(presentNode)) //且拥有未被访问的子状态 
			return Expand(presentNode); //扩展该节点 
		else
			presentNode = bestChild(presentNode, presentState); //选择最优子节点 
	}
	return presentNode;
}

//扩展节点
Node* Expand(Node* presentNode)
{
	return presentNode->child[0];
}

//随机落子 
void moveSimulation(State *simulationState)
{
	int column = simulationState->_N;
	int* tope = simulationState->topState;
	int y = rand() % column; //随机选择一列 
	while (top[y] == 0) //若此列已下满 
		y = rand() % column; //再随机选择一列 
	int x = top[y] - 1; //确定落子高度 
	simulationState->boardState[x][y] = whosTurn; //落子 
	if (x == simulationState->_noX && y == simulationState->_noY) //若落子位置正上方紧邻不可落子点 
		x --;
	simulationState->_lastX = x;
	simulationState->_lastY = y;
}

int DefaultPolicy(State *presentState, Node *selectedNode)
{
	whosTurn = presentState->_whosTurn;
	int x = presentState->_lastX;
	int y = presentState->_lastY;
	int depth = selectedNode->_depth;
	int profit = 0;
	while (profit == UNTERMINAL_STATE) //若当前状态未达终止状态 
	{
		depth++;
		moveSimulation(presentState); //随机落子 
		profit = Profit(presentState); //计算收益 
	}
	return profit;
}

//回溯更新
void Backup(Node* selectedNode, int deltaProfit)
{
	Node* tempNode = selectedNode;
	while (tempNode)
	{
		tempNode->_visitedNum ++; //访问次数+1 
		tempNode->_profit += deltaProfit; //收益增加delta 
		tempNode = tempNode->parent;
	}
}