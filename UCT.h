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

#define UNTERMINAL_STATE 2 //����ֹ״̬

using namespace std;

class Node
{
public:
	int _depth; //�ڵ����
	int _profit; //��ǰ״̬�ҷ�����
	int _visitedNum; //�����ʴ���
	int _M,_N;

	Node* parent; //���츸�ڵ�ָ��
	Node** child; //�����ӽڵ�ָ��
	
	//���캯��
	Node()
	{
		_profit = 0;
		_visitedNum = 0;
		parent = NULL;
		child = new Node*[_N]; //ָ��childrenָ������N���ӽڵ����飬ÿ��������Node*���͵�Ԫ��
		for (int i = 0; i < _N; i ++) 
			child[i] = NULL;
	}

	//��������
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
	int _M,_N; //���̴�С��M,N��
	int *topState; //����״̬
	int **boardState; //���״̬
	int _noX, _noY; //�������ӵ�λ�� 
	int _lastX, _lastY; //��һ������λ��
	int _whosTurn; //�������� 

	//���캯��
	State(int **board, int *top, int noX, int noY, int lastX, int lastY, int playingRight, const int M, const int N)
	{
		boardState = board;
		topState = top;
		_noX = noX; _noY = noY;
		_lastX = lastX; _lastY = lastY;
		_M = M; _N = N;
		_whosTurn = playingRight;
	}

	//��������
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