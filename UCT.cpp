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
int whosTurn; //��˭��,1��ʾuser�£�2��ʾmachine��

using namespace std;

//ͳ��������������Ŀ
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

//��ʼ�����̣�����ȫ����Ϊ0
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

bool isExpandable(Node* judgeNode) //�Ƿ����չ
{
	return judgeNode->expandableNum > 0;
}

bool isTerminal(State* judgeState) //�Ƿ�Ϊ��ֹ�ڵ�
{
	if (x == -1 && y == -1) //��Ϊ���ڵ�
	{
		//_cprintf("presentNode is root!\n");
		return false;
	}
	if ((judgeState->_whosTurn == userGo && machineWin(x, y, M, N, board)) || //�����ʤ�� 
		(judgeState->_whosTurn == machineGo && userWin(x, y, N, N, board)) || //���ʤ�� 
		(isTie(N, top))) //ƽ��
	{
		if (judgeState->_whosTurn == userGo && machineWin(x, y, M, N, board))
		if (judgeState->_whosTurn == machineGo && userWin(x, y, N, N, board))
		if (isTie(N, top))
		return true;
	}
	return false;
}

//�����ӽڵ�
Node* bestChild(Node* presentNode, State* presentState, double coefficient)
{
	Node* best;
	Node** childNode = presentNode->child;
	whosTurn = presentState->_whosTurn;
	int visitedNum = presentNode->_visitedNum; //���ڵ������ 
	double maxProfitRatio = -RAND_MAX;
	int bestColumn;

	int j = 1;
	for (int i = 0; i < N; i ++)
	{
		if (childNode[i] == NULL) continue;
		int judegeGo = (whosTurn == machineGo ? 1 : -1);
		int modifiedProfit = (whosTurn == machineGo ? 1 : -1) * childNode[i]->_profit; //��������ֵ
		int childVisitedNum = childNode[i]->_visitedNum; //�ӽڵ������ 
		double knownProfit = modifiedProfit*1.0/childVisitedNum;
		double unknownProfit = coefficient * sqrt(2 * log(visitedNum) / childVisitedNum);
		double tempProfitRatio = knownProfit + unknownProfit; //�����ۺ������� 

		//ѡ���ۺ������������ӽڵ� 
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

//����������
Node* TreePolicy(Node* presentNode, State* presentState)//����root�ڵ��root�ڵ��״̬
{
	while (!isTerminal(presentState)) //�ڵ��Ƿ�����ֹ״̬
	{
		if (isExpandable(presentNode)) //��ӵ��δ�����ʵ���״̬
		{
			presentNode = Expand(presentNode, presentState); //��չ�ýڵ�
			presentNode = presentNode->child[y];
			return presentNode;
		}
		else
			presentNode = bestChild(presentNode, presentState, 0.8); //ѡ�������ӽڵ� 
	}
	return presentNode;
}

void getMove(int& moveX, int& moveY)
{
	moveX = x;
	moveY = y;
}

//��Ȩ�任 
int rightChange(int chessman)
{
	if (chessman == userGo)
		return machineGo;
	else if (chessman == machineGo)
		return userGo;
	else
		return 0;
} 

//��չ�ڵ�
Node* Expand(Node* presentNode, State* presentState)
{
	int expandableNum = presentNode->expandableNum;
	int index = rand() % expandableNum; //���ȷ��һ������ֵ 
	y = presentNode->expandableNode[index];
	x = top[y] - 1; //ȷ����������
	if (x == noX && y == noY) //������λ�õ����Ϸ�λ���ǲ������ӵ� 
		x--; //�������̶���״̬����

	//Ϊ��ǰ�ڵ㴴����չ�ӽڵ� 
	presentNode->child[y] = new Node(M, N);
	presentNode->child[y]->parent = presentNode;
	swap(presentNode->expandableNode[index], presentNode->expandableNode[--expandableNum]); //����ѡ���ӽڵ����û���Ŀ¼ĩβ
	return presentNode;
}

State* stateUpdate(State* presentState)
{
	presentState->_lastX = x;
	presentState->_lastY = y; //��һ������λ��
	return presentState;
}

//������� 
State* moveSimulation(State *simulationState)
{
	int simulationY = rand() % N; //���ѡ��һ�� 
	while (simulationState->topState[simulationY] == 0) //������������ 
		simulationY = rand() % N; //�����ѡ��һ��

	int simulationX = --simulationState->topState[simulationY];

	simulationState->boardState[simulationX][simulationY] = simulationState->_whosTurn; //���� 

	if (simulationX == noX && simulationY == noY) //������λ�����Ϸ����ڲ������ӵ� 
		simulationX--;

	simulationState->_lastX = simulationX;
	simulationState->_lastY = simulationY;

	return simulationState;
}

//���㵱ǰ״̬����
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
	return UNTERMINATED_STATE; //δ������ֹ״̬ 
}

int DefaultPolicy(State* simulationState)
{
	int profit = UNTERMINATED_STATE;
	int round = 0;
	while (profit == UNTERMINATED_STATE) //����ǰ״̬δ����ֹ״̬ 
	{
		simulationState = moveSimulation(simulationState); //������� 
		profit = Profit(simulationState); //��������
		simulationState->_whosTurn = rightChange(simulationState->_whosTurn);
	}
	return profit;
}

//���ݸ���
void Backup(int deltaProfit, Node* expandedNode)
{
	Node* tempNode = expandedNode;
	while (tempNode)
	{
		tempNode->_visitedNum++; //���ʴ���+1 
		tempNode->_profit += deltaProfit; //��������delta 
		deltaProfit = -deltaProfit; //����Сԭ��
		tempNode = tempNode->parent;
	}
}