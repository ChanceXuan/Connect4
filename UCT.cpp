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
int whosTurn;//��˭��,1��ʾuser�£�2��ʾmachine��

using namespace std;

//��ʼ�����̣�����ȫ����Ϊ0
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

//��ʼ�����ؿ�����
void clearTree(Node* root, Node* simulationNode, State* simulationState)
{
	simulationNode = root; //ָ����ڵ�
	simulationState->_whosTurn = machineGo; //���ڵ���machine����
}

//���㵱ǰ״̬����
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
	return UNTERMINAL_STATE; //δ������ֹ״̬ 
}

bool isExpandable(Node *judgeNode)//�Ƿ����չ
{
	int expandableNum = 0;
	Node** childNode = new Node*[N]; //��С�����������ӽڵ����� 
	int* expandableNode = new int[N]; //�ɵ����ӽڵ��ŵ����� 
	for (int i = 0; i < N; i ++) 
	{
		if (top[i] != 0) //����i�п����� 
			expandableNode[expandableNum++] = i;
		childNode[i] = NULL;
	}
	return expandableNum > 0;
}

bool isTerminal(Node *judgeNode, State *judgeState) //�Ƿ�Ϊ��ֹ�ڵ�
{
	x = judgeState->_lastX;
	y = judgeState->_lastY;
	M = judgeState->_M;
	N = judgeState->_N;
	board = judgeState->boardState;
	top = judgeState->topState;
	if (x == -1 && y == -1) //��Ϊ���ڵ� 
		return false;
	if ((whosTurn == userGo && machineWin(x, y, M, N, board)) || //�����ʤ�� 
		(whosTurn == machineGo && userWin(x, y, N, N, board)) || //���ʤ�� 
		(isTie(N, top))) //ƽ�� 
		return true;
	return false;
}

//�����ӽڵ�
Node* bestChild(Node* parent, State* presentState)
{
	Node* best;
	Node** childNode = parent->child;
	whosTurn = presentState->_whosTurn;
	int parentVisitedNum = parent->_visitedNum; //���ڵ������ 
	int childVisitedNum = childNode[0]->_visitedNum; //�ӽڵ������ 
	double maxProfitRatio = -RAND_MAX;
	for (int i = 0; i < N; i ++)
	{
		if (childNode == NULL) continue;
		double modifiedProfit = (whosTurn == userGo ? -1 : 1) * childNode[i]->_profit; //��������ֵ
		double tempProfitRatio = modifiedProfit / childVisitedNum + 
			coefficient * sqrtl(2 * logl(parentVisitedNum) / childVisitedNum); //�����ۺ������� 
		//ѡ���ۺ������������ӽڵ� 
		if (tempProfitRatio > maxProfitRatio || (tempProfitRatio == maxProfitRatio && rand() % 2 == 0))
		{
			maxProfitRatio = tempProfitRatio;
			best = childNode[i];
		}
	}
	return best;
}

//���������� 
Node* TreePolicy(Node *presentNode, State* presentState)
{
	while (!isTerminal(presentNode, presentState)) //�ڵ㲻����ֹ�ڵ� 
	{
		if (isExpandable(presentNode)) //��ӵ��δ�����ʵ���״̬ 
			return Expand(presentNode); //��չ�ýڵ� 
		else
			presentNode = bestChild(presentNode, presentState); //ѡ�������ӽڵ� 
	}
	return presentNode;
}

//��չ�ڵ�
Node* Expand(Node* presentNode)
{
	return presentNode->child[0];
}

//������� 
void moveSimulation(State *simulationState)
{
	int column = simulationState->_N;
	int* tope = simulationState->topState;
	int y = rand() % column; //���ѡ��һ�� 
	while (top[y] == 0) //������������ 
		y = rand() % column; //�����ѡ��һ�� 
	int x = top[y] - 1; //ȷ�����Ӹ߶� 
	simulationState->boardState[x][y] = whosTurn; //���� 
	if (x == simulationState->_noX && y == simulationState->_noY) //������λ�����Ϸ����ڲ������ӵ� 
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
	while (profit == UNTERMINAL_STATE) //����ǰ״̬δ����ֹ״̬ 
	{
		depth++;
		moveSimulation(presentState); //������� 
		profit = Profit(presentState); //�������� 
	}
	return profit;
}

//���ݸ���
void Backup(Node* selectedNode, int deltaProfit)
{
	Node* tempNode = selectedNode;
	while (tempNode)
	{
		tempNode->_visitedNum ++; //���ʴ���+1 
		tempNode->_profit += deltaProfit; //��������delta 
		tempNode = tempNode->parent;
	}
}