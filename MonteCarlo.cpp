#include "MonteCarlo.h"
#include "Judge.h"
#include <ctime>

using namespace std;
 
const int EMPTY 0 //δ���� 
const int TIME_LIMITATION 3000 //����ʱ������
const int PLAYER_WIN_PROFIT -1 //��һ�ʤʱ������ 
const int COMPUTER_WIN_PROFIT 1 //�ҷ�AI��ʤʱ������ 
const int TIE_PROFIT 0 //ƽ������
const int UNTERMINAL_STATE 2 //����ֹ״̬ 
const int COEFFICIENT 0.8 //����ϵ��c 
const int RANGE = 12;
const int ourGo = 1;
const int rivalGo = 0;
const double COEFFICIENT = 0.8;

const int maxNodeNumber = 9000000;
const int maxDepth = 9000000;
const double timeLimit = 2.5;//2.5s

int startTime;
int row,column;
int noX,noY,lastX,lastY;

int whichGo;//��˭��,1��ʾuser�£�0��ʾ������
int root;//���ڵ�
int top[RANGE],board[RANGE][RANGE];
int chessNumber = 999;//��һ���������ϵ�������Ŀ
int path[maxDepth];
int pathTop;
 
using namespace std;
 
class UCT;
 
class Node
{
private: 
	int **boardState; //���״̬
	int *topState; //����״̬
	int row, column; //���̴�С��M, N��
	int _noX, _noY; //�������ӵ�λ�� 
	int _chessman; //�ҷ��������� 
	int _x, _y; //ǰһ������λ��
	int visitedNum; //�����ʴ��� 
	double profit; //��ǰ״̬�ҷ�����
	int _depth; //�ڵ���� 
	Node *father; //���ڵ�
	Node **children; //�ӽڵ�
	int expandableNum; //����չ�ڵ����� 
	int *expandableNode; //����չ�ڵ��� 
	friend class UCT;
	
	int *TopState() const //�������̶���״̬����topState
	{
		int *presentTop = new int[column];
		for (int i = 0; i != column; i ++)
			presentTop[i] = topState[i];
		return presentTop;
	}

	int **BoardState() const //��������״̬����boardState
	{
		int **presentBoardState = new int*[row];
		for (int i = 0; i < row; i ++) 
		{
			presentBoardState[i] = new int[column];
			for(int j = 0; j < column; j ++)
				presentBoardState[i][j] = boardState[i][j];
		}
		return presentBoardState;
	}

	void clear() //�ռ��ͷ�
	{
		for (int i = 0; i != row; i ++)
			delete [] boardState[i];
		delete [] boardState;
		delete [] topState;
		delete [] expandableNode;
		for (int i = 0; i != column; i ++)
			if (children[i])
			{
				children[i] -> clear();
				delete children[i];
			}
		delete [] children;
	}
	
public:
	//���캯�� 
	Node(int **board, int *top, int r, int c, int noX, int noY, int depth = 0, int x = -1, int y = -1, int playingRight = COMPUTER_CHANCE, Node* _father = NULL): 
		boardState(board), topState(top), row(r), column(c), _noX(noX), _noY(noY), _depth(depth), _x(x), _y(y), _chessman(playingRight), visitedNum(0), profit(0), father(_father) 
		{
			expandableNum = 0; 
			children = new Node*[column]; //��С�����������ӽڵ����� 
			expandableNode = new int[column]; //�ɵ����ӽڵ��ŵ����� 
			for (int i = 0; i != column; i ++) 
			{
				if (topState[i] != 0) //����i�п����� 
					expandableNode[expandableNum ++] = i;
				children[i] = NULL;
			}
		}
	int x() const { return _x; }
	int y() const { return _y; }
	int chessman() const { return _chessman; }
	bool isExpandable() const { return expandableNum > 0; }//�Ƿ����չ
	//�Ƿ�Ϊ��ֹ�ڵ� 
	bool isTerminal() {
		if (_x == -1 && _y == -1) //��Ϊ���ڵ� 
			return false;
		if ((_chessman == PLAYER_CHANCE && machineWin(_x, _y, row, column, boardState)) || //�����ʤ�� 
			(_chessman == COMPUTER_CHANCE && userWin(_x, _y, row, column, boardState)) || //���ʤ�� 
			(isTie(column, topState))) //ƽ�� 
			return true;
		return false;
	}
};
 
class UCT
{
private:
	Node *_root; //���ڵ�
	int _row, _column; //����������
	int _noX, _noY; //�������ӵ��λ�� 
	int startTime; //���㿪ʼʱ��
	
	//���㵱ǰ״̬����
	int Profit(int **board, int *top, int chessman, int x, int y) const { 
		if (chessman == PLAYER_CHANCE && userWin(x, y, _row, _column, board))
			return PLAYER_WIN_PROFIT;
		if (chessman == COMPUTER_CHANCE && machineWin(x, y, _row, _column, board))
			return COMPUTER_WIN_PROFIT;
		if (isTie(_column, top))
			return TIE_PROFIT;
		return UNTERMINAL_STATE; //δ������ֹ״̬ 
	}
	//������� 
	void placeChessman(int **board, int *top, int chessman, int &x, int &y) {
		y = rand() % _column; //���ѡ��һ�� 
		while (top[y] == 0) //������������ 
			y = rand() % _column; //�����ѡ��һ�� 
		x = -- top[y]; //ȷ�����Ӹ߶� 
		board[x][y] = chessman; //���� 
		if (x - 1 == _noX && y == _noY) //������λ�����Ϸ����ڲ������ӵ� 
			top[y] --;
	}
	//��Ȩ�任 
	int rightChange(int chessman) const {
		if (chessman == PLAYER_CHANCE)
			return COMPUTER_CHANCE;
		else if (chessman == COMPUTER_CHANCE)
			return PLAYER_CHANCE;
		else
			return -1;
	} 
	
	//���������� 
	Node *TreePolicy(Node *presentNode) {
		while (!presentNode -> isTerminal()) { //�ڵ㲻����ֹ�ڵ� 
			if (presentNode -> isExpandable()) //��ӵ��δ�����ʵ���״̬ 
				return Expand(presentNode); //��չ�ýڵ� 
			else
				presentNode = BestChild(presentNode); //ѡ�������ӽڵ� 
		}
		return presentNode;
	}

	//��չ�ڵ� 
	Node *expand(Node *presentNode) 
	{
		playingRight = rightChange(presentNode -> chessman())
		int index = rand() % expandableNum; //���ȷ��һ������ֵ 
		int **newBoardState = BoardState(); //��������״̬���� 
		int *newTopState = TopState(); //�������̶���״̬���� 
		int newY = expandableNode[index], newX = -- newTopState[newY]; //ȷ���������� 
		newBoardState[newX][newY] = chessman(); //���� 
		if (newX - 1 == _noX && newY == _noY) //������λ�õ����Ϸ�λ���ǲ������ӵ� 
			newTopState[newY] --; //�������̶���״̬����
		//Ϊ��ǰ�ڵ㴴����չ�ӽڵ� 
		children[newY] = new Node(newBoardState, newTopState, row, column, _noX, _noY, _depth + 1, newX, newY, playingRight, this);
		swap(expandableNode[index], expandableNode[-- expandableNum]); //����ѡ���ӽڵ����û���Ŀ¼ĩβ
		return presentNode -> children[newY]
	}

	//�����ӽڵ�
	Node *bestChild(Node *father)
	{
		Node* best;
		double maxProfitRatio = -RAND_MAX;
		for (int i = 0; i != column; i ++) {
			if (children[i] == NULL) continue;
			double modifiedProfit = (_chessman == PLAYER_CHANCE ? -1 : 1) * children[i] -> profit; //��������ֵ
			int childVisitedNum = children[i] -> visitedNum; //�ӽڵ������ 
			double tempProfitRatio = modifiedProfit / childVisitedNum + 
				sqrtl(2 * logl(visitedNum) / childVisitedNum) * VITALITY_COEFFICIENT; //�����ۺ������� 
			if (tempProfitRatio > maxProfitRatio || (tempProfitRatio == maxProfitRatio && rand() % 2 == 0)) { //ѡ���ۺ������������ӽڵ� 
				maxProfitRatio = tempProfitRatio;
				best = children[i];
			}
		}
		return father -> best;
	} 
	//���ݸ���
	void backup(double deltaProfit) {
		Node *temp = this;
		while (temp) {
			temp -> visitedNum ++; //���ʴ���+1 
			temp -> profit += deltaProfit; //��������delta 
			temp = temp -> father;
		}
	}

	//ģ����� 
	double DefaultPolicy(Node *selectedNode) { 
		int **boardState = selectedNode -> BoardState(), *top = selectedNode -> TopState();
		int chessman = selectedNode -> chessman(), depth = selectedNode -> _depth;
		int x = selectedNode -> x(), y = selectedNode -> y();
		int profit = Profit(boardState, top, rightChange(chessman), x, y); //�������� 
		while (profit == UNTERMINAL_STATE) { //����ǰ״̬δ����ֹ״̬ 
			depth ++;
			placeChessman(boardState, top, chessman, x, y); //������� 
			profit = Profit(boardState, top, chessman, x, y); //�������� 
			chessman = rightChange(chessman); //��Ȩ�任 
		}
		for (int i = 0; i != _row; i ++)
			delete [] boardState[i];
		delete [] boardState;
		delete [] top;
		return double(profit);// / logl(depth + 1); //�����Լ���
	}
	//���ݸ�������(���Խ������ԽС)
	void Backup(Node *selectedNode, double deltaProfit) { selectedNode -> backup(deltaProfit); }
	
public:
	//���캯�� 
	UCT(int row, int column, int noX, int noY): _row(row), _column(column), _noX(noX), _noY(noY), startTime(clock()) {}
	//�������������� 
	Node *UCTSearch(int **boardState, int *topState) {
		_root = new Node (boardState, topState, _row, _column, _noX, _noY); //�Ե�ǰ״̬�������ڵ� 
		while (clock() - startTime <= TIME_LIMITATION) { //��δ�ľ�����ʱ�� 
			Node *selectedNode = TreePolicy(_root); //�������������Խڵ� 
			double deltaProfit = DefaultPolicy(selectedNode); //����ģ����Զ�ѡ�нڵ����һ�����ģ�� 
			Backup(selectedNode, deltaProfit); //��ģ�������ݷ����������� 
		}
		return BestChild(_root);
	}
	//��������
	~UCT() { _root -> clear(); delete _root; } 
};