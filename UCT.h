//ע��COMPUTER_WIN�����ҵ�AIӮ������
 
#include "Judge.h" //�����ж�ʤ���ĺ���userWin��machineWin��isTie
#include <ctime>
#include <cmath>
#include <cstdlib>

const int userGo = 1;
const int machineGo = 2;
const int PLAYER_WIN_PROFIT = -1; //��һ�ʤʱ������
const int COMPUTER_WIN_PROFIT = 1; //�ҷ�AI��ʤʱ������
const int TIE_PROFIT = 0; //ƽ������

#define UNTERMINATED_STATE 2 //����ֹ״̬

using namespace std;

class Node
{
public:
	int _M,_N;
	int _profit; //��ǰ״̬�ҷ�����
	int _visitedNum; //�����ʴ���
	int expandableNum; //����չ�ڵ����� 
	int *expandableNode; //����չ�ڵ��� 

	Node* parent; //���츸�ڵ�ָ��
	Node** child; //�����ӽڵ�ָ��
	
	void clear() //�ռ��ͷ�
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

	//���캯��
	Node(const int M, const int N)
	{
		_M = M; _N = N;
		_profit = 0;
		_visitedNum = 0;
		parent = NULL;
		child = new Node*[_N]; //ָ��childrenָ������N���ӽڵ����飬ÿ��������Node*���͵�Ԫ��
		expandableNum = _N;
		expandableNode = new int[_N];
		for (int i = 0; i < _N; i ++)
			child[i] = NULL;
			
	}

	//�������캯��
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

	//��������
	~Node()
	{
		clear();
	}
};

class State
{
public:
	int _M,_N; //���̴�С��M,N��
	int *topState; //����״̬
	int **boardState; //���״̬
	int _lastX, _lastY; //�����ϴε�����λ��
	int _whosTurn; //��������

	int* TopState(int* _top) const //�������̶���״̬����topState
	{
		int *presentTop = new int[_N];
		for (int i = 0; i < _N; i ++)
			presentTop[i] = _top[i];
		return presentTop;
	}

	int** BoardState(int** _board) const //��������״̬����boardState
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

	//���캯��
	State(int** board, int* top, int lastX, int lastY, int playingRight, const int M, const int N)
	{
		_M = M; _N = N;
		_lastX = lastX; _lastY = lastY;
		boardState = BoardState(board);
		topState = TopState(top);
		_whosTurn = playingRight;
	}

	//�������캯��
	State(const State &state, const int M, const int N)
	{
		_M = M; _N = N;
		_lastX = state._lastX;
		_lastY = state._lastY;
		boardState = state.boardState;
		topState = state.topState;
		_whosTurn = state._whosTurn;
	}

	//��������
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