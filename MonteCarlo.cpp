#include "MonteCarlo.h"
#include "Judge.h"
#include <ctime>

using namespace std;
 
const int EMPTY 0 //未落子 
const int TIME_LIMITATION 3000 //计算时长限制
const int PLAYER_WIN_PROFIT -1 //玩家获胜时的收益 
const int COMPUTER_WIN_PROFIT 1 //我方AI获胜时的收益 
const int TIE_PROFIT 0 //平局收益
const int UNTERMINAL_STATE 2 //非终止状态 
const int COEFFICIENT 0.8 //比例系数c 
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

int whichGo;//该谁下,1表示user下，0表示对手下
int root;//根节点
int top[RANGE],board[RANGE][RANGE];
int chessNumber = 999;//上一步的棋盘上的棋子数目
int path[maxDepth];
int pathTop;
 
using namespace std;
 
class UCT;
 
class Node
{
private: 
	int **boardState; //棋局状态
	int *topState; //顶端状态
	int row, column; //棋盘大小（M, N）
	int _noX, _noY; //不可落子点位置 
	int _chessman; //我方持子属性 
	int _x, _y; //前一上落子位置
	int visitedNum; //被访问次数 
	double profit; //当前状态我方收益
	int _depth; //节点深度 
	Node *father; //父节点
	Node **children; //子节点
	int expandableNum; //可扩展节点数量 
	int *expandableNode; //可扩展节点编号 
	friend class UCT;
	
	int *TopState() const //复制棋盘顶端状态数组topState
	{
		int *presentTop = new int[column];
		for (int i = 0; i != column; i ++)
			presentTop[i] = topState[i];
		return presentTop;
	}

	int **BoardState() const //复制棋盘状态数组boardState
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

	void clear() //空间释放
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
	//构造函数 
	Node(int **board, int *top, int r, int c, int noX, int noY, int depth = 0, int x = -1, int y = -1, int playingRight = COMPUTER_CHANCE, Node* _father = NULL): 
		boardState(board), topState(top), row(r), column(c), _noX(noX), _noY(noY), _depth(depth), _x(x), _y(y), _chessman(playingRight), visitedNum(0), profit(0), father(_father) 
		{
			expandableNum = 0; 
			children = new Node*[column]; //大小等于行数的子节点数组 
			expandableNode = new int[column]; //可到达子节点编号的数组 
			for (int i = 0; i != column; i ++) 
			{
				if (topState[i] != 0) //若第i列可落子 
					expandableNode[expandableNum ++] = i;
				children[i] = NULL;
			}
		}
	int x() const { return _x; }
	int y() const { return _y; }
	int chessman() const { return _chessman; }
	bool isExpandable() const { return expandableNum > 0; }//是否可扩展
	//是否为终止节点 
	bool isTerminal() {
		if (_x == -1 && _y == -1) //若为根节点 
			return false;
		if ((_chessman == PLAYER_CHANCE && machineWin(_x, _y, row, column, boardState)) || //计算机胜利 
			(_chessman == COMPUTER_CHANCE && userWin(_x, _y, row, column, boardState)) || //玩家胜利 
			(isTie(column, topState))) //平局 
			return true;
		return false;
	}
};
 
class UCT
{
private:
	Node *_root; //根节点
	int _row, _column; //行数、列数
	int _noX, _noY; //不可落子点的位置 
	int startTime; //计算开始时间
	
	//计算当前状态收益
	int Profit(int **board, int *top, int chessman, int x, int y) const { 
		if (chessman == PLAYER_CHANCE && userWin(x, y, _row, _column, board))
			return PLAYER_WIN_PROFIT;
		if (chessman == COMPUTER_CHANCE && machineWin(x, y, _row, _column, board))
			return COMPUTER_WIN_PROFIT;
		if (isTie(_column, top))
			return TIE_PROFIT;
		return UNTERMINAL_STATE; //未进入终止状态 
	}
	//随机落子 
	void placeChessman(int **board, int *top, int chessman, int &x, int &y) {
		y = rand() % _column; //随机选择一列 
		while (top[y] == 0) //若此列已下满 
			y = rand() % _column; //再随机选择一列 
		x = -- top[y]; //确定落子高度 
		board[x][y] = chessman; //落子 
		if (x - 1 == _noX && y == _noY) //若落子位置正上方紧邻不可落子点 
			top[y] --;
	}
	//棋权变换 
	int rightChange(int chessman) const {
		if (chessman == PLAYER_CHANCE)
			return COMPUTER_CHANCE;
		else if (chessman == COMPUTER_CHANCE)
			return PLAYER_CHANCE;
		else
			return -1;
	} 
	
	//搜索树策略 
	Node *TreePolicy(Node *presentNode) {
		while (!presentNode -> isTerminal()) { //节点不是终止节点 
			if (presentNode -> isExpandable()) //且拥有未被访问的子状态 
				return Expand(presentNode); //扩展该节点 
			else
				presentNode = BestChild(presentNode); //选择最优子节点 
		}
		return presentNode;
	}

	//扩展节点 
	Node *expand(Node *presentNode) 
	{
		playingRight = rightChange(presentNode -> chessman())
		int index = rand() % expandableNum; //随机确定一个索引值 
		int **newBoardState = BoardState(); //复制棋盘状态数组 
		int *newTopState = TopState(); //复制棋盘顶端状态数组 
		int newY = expandableNode[index], newX = -- newTopState[newY]; //确定落子坐标 
		newBoardState[newX][newY] = chessman(); //落子 
		if (newX - 1 == _noX && newY == _noY) //若落子位置的正上方位置是不可落子点 
			newTopState[newY] --; //更新棋盘顶端状态数组
		//为当前节点创建扩展子节点 
		children[newY] = new Node(newBoardState, newTopState, row, column, _noX, _noY, _depth + 1, newX, newY, playingRight, this);
		swap(expandableNode[index], expandableNode[-- expandableNum]); //将被选中子节点编号置换到目录末尾
		return presentNode -> children[newY]
	}

	//最优子节点
	Node *bestChild(Node *father)
	{
		Node* best;
		double maxProfitRatio = -RAND_MAX;
		for (int i = 0; i != column; i ++) {
			if (children[i] == NULL) continue;
			double modifiedProfit = (_chessman == PLAYER_CHANCE ? -1 : 1) * children[i] -> profit; //修正收益值
			int childVisitedNum = children[i] -> visitedNum; //子节点访问数 
			double tempProfitRatio = modifiedProfit / childVisitedNum + 
				sqrtl(2 * logl(visitedNum) / childVisitedNum) * VITALITY_COEFFICIENT; //计算综合收益率 
			if (tempProfitRatio > maxProfitRatio || (tempProfitRatio == maxProfitRatio && rand() % 2 == 0)) { //选择综合收益率最大的子节点 
				maxProfitRatio = tempProfitRatio;
				best = children[i];
			}
		}
		return father -> best;
	} 
	//回溯更新
	void backup(double deltaProfit) {
		Node *temp = this;
		while (temp) {
			temp -> visitedNum ++; //访问次数+1 
			temp -> profit += deltaProfit; //收益增加delta 
			temp = temp -> father;
		}
	}

	//模拟策略 
	double DefaultPolicy(Node *selectedNode) { 
		int **boardState = selectedNode -> BoardState(), *top = selectedNode -> TopState();
		int chessman = selectedNode -> chessman(), depth = selectedNode -> _depth;
		int x = selectedNode -> x(), y = selectedNode -> y();
		int profit = Profit(boardState, top, rightChange(chessman), x, y); //计算收益 
		while (profit == UNTERMINAL_STATE) { //若当前状态未达终止状态 
			depth ++;
			placeChessman(boardState, top, chessman, x, y); //随机落子 
			profit = Profit(boardState, top, chessman, x, y); //计算收益 
			chessman = rightChange(chessman); //棋权变换 
		}
		for (int i = 0; i != _row; i ++)
			delete [] boardState[i];
		delete [] boardState;
		delete [] top;
		return double(profit);// / logl(depth + 1); //非线性加速
	}
	//回溯更新收益(深度越深收益越小)
	void Backup(Node *selectedNode, double deltaProfit) { selectedNode -> backup(deltaProfit); }
	
public:
	//构造函数 
	UCT(int row, int column, int noX, int noY): _row(row), _column(column), _noX(noX), _noY(noY), startTime(clock()) {}
	//信心上限树搜索 
	Node *UCTSearch(int **boardState, int *topState) {
		_root = new Node (boardState, topState, _row, _column, _noX, _noY); //以当前状态创建根节点 
		while (clock() - startTime <= TIME_LIMITATION) { //尚未耗尽计算时长 
			Node *selectedNode = TreePolicy(_root); //运用搜索树策略节点 
			double deltaProfit = DefaultPolicy(selectedNode); //运用模拟策略对选中节点进行一次随机模拟 
			Backup(selectedNode, deltaProfit); //将模拟结果回溯反馈给各祖先 
		}
		return BestChild(_root);
	}
	//析构函数
	~UCT() { _root -> clear(); delete _root; } 
};