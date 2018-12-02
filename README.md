# 四子棋AI程序
MonteCarlo + Upper Confidence Bound

## 实验简介
重力四子棋是一款经典的类游戏。游戏双方分别持不同颜色的棋子，设A持白子，B持黑子，以某一方为先手依次落子。假设A为先手，落子规则如下：在M行N列的棋盘中，棋手每次只能在每一列当前的最底部落子。棋手的目标是在横向、纵向、两个斜向共四个方向中的任意一个方向上，使自己的棋子连成四个（或四个以上），并阻止对方达到同样的企图。先形成四连子的一方获胜，如果直到棋盘落满双方都没能达目标，则为平局。

本实验则要求现一个四子棋的AI，当两个策略对弈时，棋盘的大小是随机的（ 宽、高在9至12之间），而且会随机生成不可落子的点。如下图中红叉所示，当一方在小黑点处落子时，下次该列的可落子位置将是小绿点处。

实验说明中推荐了α-β剪枝算法，但是其难点在于估价函数的设计上，需要大量的专家经验。所以我采用了蒙特卡罗搜索树（MCTS）算法 ，并且将UCB1算法加入其中 ，即用了信心上限树算法 (UCT) 来完成四子棋AI。算法共包含四个基本步骤，分别为选择算法共包含四个基本步骤，分别为选择 (Selection)、扩展 (Expansion)、 模拟 (Simulation)、回溯 (Back propagation)，如下图所示。

图中的每个节点表示博弈过程的一个局面状态，每条边表示在父节点上采取一个行动，并得到子节点所对应的局面状态。这四个基本步骤依次执行，从而完成一次搜索，具体说来这四个步骤为：
1. 选择（Selection）：从根节点出发，在搜索树上自上而下选择UCB1的信心上界值最大的子节点（可落子但没有被访问的节点信心上界值为正无穷），直至找到当前最为紧迫的可扩展节点。可扩展节点 ⇔ 节点所对应的状态是非停止状态，且拥有未被访问过的子节点；
2. 扩展（Expansion）：在被选择的节点上扩展一个或多子节点；
3. 模拟（Simulation）：根据默认策略（Default Policity）在扩展出来的一个（或多个）子节点上执行蒙特卡洛棋局模拟，并确定的估计值；
4. 回溯（Back propagation）：根据模拟结果向上依次更新祖先节点的估计根据模拟结果向上依次更新祖先节点的估计值，并更新其状态。

执行上述搜索过程直到时间达到规定，然后在根节点的儿子中选择信心上界值最大的节点给出落子操作。

信心上限树算法（UCT）伪代码描述如下 ：
``` javascript
function UCTSEARCH(s0) {
    以状态s0创建根节点v0;
    while 尚未用完计算时长 do:
        v_l ← TREEPOLICY(v_0);
        ∆ ← DEFAULTPOLICY(s(v_l));
        BACKUP(v_l,∆);
    end while
    return a(BESTCHILD(v_0,0)) }

function TREEPOLICY(v) {
    while 节点v不是终止节点 do:
        if 节点v是可扩展的 then:
            return EXPAND(v)
        else:
            v ← BESTCHILD(v,c)
    return v }

function EXPAND(v) {
    选择行动a∈A(state(v))中尚未选择过的行动
    向节点v添加子节点v'使得s(v')=f(s(v),a), a(v')=a
    return v' }

function BESTCHILD(v,c) {
    return 〖argmax〗_(v'∈children of v) ((Q(v'))/(N(v')) + c*√((2ln(N(v)))/(N(v'))))}

function DEFAULTPOLICY(s) {
    while s不是终止状态 do:
        以等概率选择行动a∈A(s)
        s←f(s,a)
    return 状态s的收益 }

function BACKUP(v,Δ) {
    while v≠NULL do:
        N(v)←N(v)+1
        Q(v)←Q(v)+∆
        ∆←-∆
        v←v的父节点 }
```

> copyright ©️清华大学美术学院信息艺术设计系（交叉学科） 美研171宣程2017213554
