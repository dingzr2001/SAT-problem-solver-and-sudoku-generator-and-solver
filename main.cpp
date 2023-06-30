#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
//文字对应的四种状态：
#define TRUE 1
#define FALSE -1
#define UNCERTAIN 0
#define UNEXISt 3
#define SINGLE 1
#define EMPTY 0
#define SATISFIABLE 1
#define INFEASIBLE 0

typedef struct StackNode{
    int data;
    int oldstate;
    struct StackNode *next;
}StackNode;

typedef struct LStack{
    StackNode *node;
    int nodenum;
}LStack;//简易栈，用于存储单个文字

typedef struct CStackNode{
    StackNode *node;
    struct CStackNode *next;
}CStacknode;

typedef struct CStack{
    CStacknode *node;
    int nodenum;
}CStack;//复杂栈，用于存储一串句子编号

typedef struct ClauseNodeInMap{
    int data;
    struct ClauseNodeInMap *next;
}CNInMap;

typedef struct LiteralMapNode{
    CNInMap *node;
    int frequency;//用于计算文字在所有语句中出现频率的加权
}LMapNode;//文字标记组的结点，用于标记每个文字所有出现的语句，以及每个文字出现的频率

typedef struct ClauseNode{
    struct LiteralNode *literal;
    struct ClauseNode *next;
    int tag;
    int literalnum;
}ClauseNode;    //子句结点的定义

typedef struct Clause{
    int clausesum;
    int literalsum;
    ClauseNode *elem;
}Clause;

typedef struct LiteralNode{
    int data;
    struct LiteralNode *next;
    int tag;
}LiteralNode; //文字节点的定义

int clausemap[12000] = {0};//0代表已被访问

int clausenum = 0, literalnum = 0;

LStack literalstack;//文字栈，用于复原文字
CStack clausestack;//子句栈，用于复原子句
LStack depthstack;

int testcounter = 0;
//-----------------------------------------------------------------------
//文字栈栈初始化函数：
void IniLStack(LStack &literalstack)
{
    literalstack.node = NULL;
    literalstack.nodenum = 0;
}
//-----------------------------------------------------------------------
//文字栈进栈函数：
void PushL(LStack &literalstack, int n)
{
    StackNode *p = (StackNode *)malloc(sizeof(StackNode));
    p->data = n;
    p->next = literalstack.node;
    literalstack.node = p;
    literalstack.nodenum ++;
}

//-----------------------------------------------------------------------
//文字栈退栈函数：
void PopL(LStack &literalstack, int &n)
{
    StackNode *p = literalstack.node;
    if(literalstack.nodenum == 0)
        printf("ERROR!");
    if(p)
    {
        n = p->data;
        literalstack.node = p->next;
        free(p);
        literalstack.nodenum --;
    }
}

//-----------------------------------------------------------------------
//子句栈初始化函数：
void IniCstack(CStack &clausestack)
{
    clausestack.node = NULL;
    clausestack.nodenum = 0;
}

//-----------------------------------------------------------------------
//子句栈入栈函数：
void PushC(CStack &clausestack)
{
    CStackNode *p = (CStackNode *)malloc(sizeof(CStackNode));
    p->next = NULL;
    p->node = NULL;
    p->next = clausestack.node;
    clausestack.node = p;
    clausestack.nodenum ++;
}

//-----------------------------------------------------------------------
//子句栈退栈函数：
void PopC(CStack &clausestack)
{
    StackNode *p = clausestack.node->node;
    if(clausestack.nodenum == 0)
        printf("ERROR!");
    while(p)
    {
        StackNode *q = p;
        p = p->next;
        free(q);
    }
    clausestack.nodenum --;
    CStackNode *r = clausestack.node;
    clausestack.node = r->next;
    free(r);
}

//-----------------------------------------------------------------------
//在栈顶接句子的函数：
void AddTopC(CStack &clausestack, int n, int state)
{
    StackNode *p = (StackNode *)malloc(sizeof(StackNode));
    p->data = n;
    p->next = clausestack.node->node;
    clausestack.node->node = p;
    testcounter++;
}

//-----------------------------------------------------------------------
//删除栈顶中的一个句子，将其状态赋给state
int DeleteTopC(CStack &clausestack, int &n, int &state)
{
    if(clausestack.node->node == NULL)
        return INFEASIBLE;
    testcounter --;
    StackNode *p = clausestack.node->node;
    n = p->data;
    state = p->oldstate;
    clausestack.node->node = p->next;
    free(p);
    return SATISFIABLE;
}

//-----------------------------------------------------------------------
//判断句子栈顶无句子：
int EmptyTop(CStack clausestack)
{
    if(clausestack.node->node == NULL)
        return SATISFIABLE;
    return INFEASIBLE;
}


//-----------------------------------------------------------------------
//从文件中读取cnf范式：
void loadfile(Clause &clause, char filename[])
{
    int clausenum = 0, literalnum = 0;
    FILE *fp = fopen(filename, "r");
    char ch;
    while((ch = fgetc(fp)) == 'c')
    {
        char trash[101];
        fgets(trash, 100, fp);
    }

    char type[5];
    fscanf(fp, "%s", type);//读取文件类型：cnf范式
    fscanf(fp, "%d", &literalnum);//读取范式中文字数
    fscanf(fp, "%d", &clausenum);//读取范式中子句数
    printf("\n%s %d %d\n", type, literalnum, clausenum);
    clause.elem = (ClauseNode *)malloc(clausenum*sizeof(ClauseNode));//给每个子句顶点创建空间
    clause.clausesum = clausenum;//更新clausennum
    clause.literalsum = literalnum;//更新literalnum
    for(int i=0; i<clausenum; i++)
    {
        clause.elem[i].literal = NULL;
        clause.elem[i].literalnum = 0;
        clause.elem[i].tag = UNCERTAIN;
        while(TRUE)
        {
            int a;
            fscanf(fp, "%d", &a);
            if(a == 0)
                break;
            else
            {

                LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                p->data = fabs(a);
                p->next = NULL;
                p->tag = p->data/a;
                p->next = clause.elem[i].literal;
                clause.elem[i].literal = p;//采用头插法插入文字节点
                clause.elem[i].literalnum ++;
                clausemap[i]++;
            }
        }
    }
    /*for(int i=0; i<clausenum; i++)
    {
        LiteralNode *p = clause.elem[i].literal;
        while(p)
        {
            printf("%d ", p->data*p->tag);
            p = p->next;
        }
        printf("\n");
    }*/
    fclose(fp);
    return ;
}

int literalmap[3500];//用来标记文字的状态：真、假或不定
//----------------------------------------------------------------------------------
//将求解结果写入文件：
void savefile(Clause clause, char filename[], int runtime, int state)
{
    FILE *fp = fopen(filename, "w");
    fprintf(fp, "s %d\nv", state);
    for(int i=1; i<=clause.literalsum; i++)
        fprintf(fp, " %d", i*literalmap[i]);
    fprintf(fp, "\nt %dms", runtime);
    fclose(fp);
}
//----------------------------------------------------------------------------------
//判断所有子句中是否含有空子句
int ContainsEmptyClause(Clause clause)
{
    for(int i=0; i<clause.clausesum; i++)
    {
        if(clause.elem[i].literalnum == 0)
            return SATISFIABLE;//返回true代表有空子句
    }
    return INFEASIBLE;
}
//----------------------------------------------------------------------------------
//把某个文字设为“真”
int SetLiteralTrue(Clause &clause, int literal)
{
    int emptyflag = 0;
    literalmap[literal] = TRUE;//更改map中的状态为“真”
    int flag = 0;
    PushL(literalstack, literal);
    PushC(clausestack);
    for(int i=0; i<clause.clausesum; i++)
    {
        if(clause.elem[i].tag != TRUE)
        {
            LiteralNode *p = clause.elem[i].literal;
            while(p)
            {
                if(p->data == literal)//变元literal出现了
                {
                    if(p->tag == 1)//作为literal本身出现
                        clause.elem[i].tag = TRUE;
                    else
                        clause.elem[i].literalnum--;//作为非literal出现
                    AddTopC(clausestack, i, clause.elem[i].tag);
                    if(clause.elem[i].literalnum == 0)
                        emptyflag = 1;
                }
                p = p->next;
            }
        }
    }
    if(emptyflag == 1)
        return INFEASIBLE;
    return SATISFIABLE;
}

//----------------------------------------------------------------------------------
//把某个文字设为“假”
int SetLiteralFalse(Clause &clause, int literal)
{
    int emptyflag = 0;
    literalmap[literal] = FALSE;//更改map中的状态为“假”
    int flag = 0;
    PushL(literalstack, literal);
    PushC(clausestack);
    for(int i=0; i<clause.clausesum; i++)
    {
        if(clause.elem[i].tag != TRUE)
        {
            LiteralNode *p = clause.elem[i].literal;
            while(p)
            {
                if(p->data == literal)//变元literal出现了
                {
                    if(p->tag == -1)//作为非literal出现
                        clause.elem[i].tag = TRUE;
                    else
                        clause.elem[i].literalnum--;//作为literal出现
                    AddTopC(clausestack, i, clause.elem[i].tag);
                    if(clause.elem[i].literalnum == 0)//出现了空子句
                        emptyflag = 1;//为了防止回溯时出错，一定要全部改完之后才能返回
                }
                p = p->next;
            }
        }
    }
    if(emptyflag == 1)
        return INFEASIBLE;
    return SATISFIABLE;
}
//=================================================================================
//回溯至上一状态：
int Revert(Clause &clause)
{
    int n, m;
    if(literalstack.node == NULL)
        printf("文字栈为空！");
    PopL(literalstack, n);//文字栈出栈
    int clausenumber;
    int state;
    while(EmptyTop(clausestack) == INFEASIBLE)//栈顶层不为空
    {
        DeleteTopC(clausestack, m, state);//顶层去掉一个子句
        /*if(clause.elem[m].literalnum == 0)
            printf("空！");*/
        if(clause.elem[m].tag == TRUE)
            clause.elem[m].tag = UNCERTAIN;//由上一步变为TRUE，说明之前一定是UNCERTAIN
        else
            clause.elem[m].literalnum++;//要么由上一步变为FALSE，要么经过上一步仍是UNCERTAIN，哪种情况都是literalnum++
    }
    PopC(clausestack);//栈顶去掉子句后为空，那么直接出栈，即删掉一层
    literalmap[n] = UNCERTAIN;//把literalmap中n的真值恢复为UNCERTAIN
    /*for(int i=1; i<=clause.literalsum; i++)
        printf("%d\n", literalmap[i]);*/
    return SATISFIABLE;
}

//---------------------------------------------------------------------------------
//判断是否有单子句，若有则返回单子句中唯一一个剩余的文字，否则返回FALSE
int UnitClause(Clause &clause)
{
    for(int i=0; i<clause.clausesum; i++)
    {
        if(clause.elem[i].tag != TRUE && clause.elem[i].literalnum == 1)
        {
            LiteralNode *p = clause.elem[i].literal;
            while(p)
            {
                if(literalmap[p->data] == UNCERTAIN)
                    return (p->data * p->tag);
                p = p->next;
            }
        }
    }
    return INFEASIBLE;
}

//----------------------------------------------------------------------------------
//判断命题是否已被满足
int Judge(Clause clause)
{
    for(int i=0; i<clause.clausesum; i++)
        if(clause.elem[i].tag != TRUE)
            return INFEASIBLE;
    return SATISFIABLE;
}

//----------------------------------------------------------------------------------
//猜测一个文字为真
int GuessLiteral(Clause clause)
{
    for(int i=1; i<clause.clausesum; i++)
    {
        LiteralNode *p = clause.elem[i].literal;
        while(p)
        {
            if(literalmap[p->data] == UNCERTAIN)
                return p->data;
            p = p->next;
        }
    }
}


int count = 0;
int revertcount = 0;
//----------------------------------------------------------------------------------
//DPLL算法：
int DPLL(Clause &clause)
{
    count++;
    int dpllstart = clock();
    int dpllend;
    int i;
    int flag = 0;
    int literal;
    int cnt = 0;//用来数对多少个文字的真值进行过改动，方便回溯
    while(TRUE)
    {
        int returnsaver = 0;//用于存储返回值，避免函数重复调用
        if(returnsaver = UnitClause(clause))//找到单子句
        {
            if(returnsaver > 0)//单子句中的是正文字
            {
                cnt++;
                revertcount ++;
                if(SetLiteralTrue(clause, returnsaver) == INFEASIBLE)//把单子句中的文字设为真
                {
                    while(cnt != 0)
                    {
                        Revert(clause);
                        cnt--;
                        revertcount --;
                    }
                    dpllend = clock();
                    return INFEASIBLE;
                }
            }

            else//单子句中的是“非”文字
            {
                cnt++;
                revertcount++;
                if(SetLiteralFalse(clause, -returnsaver) == INFEASIBLE)//把单子句中的文字设为假
                {
                    while(cnt != 0)
                    {
                        Revert(clause);
                        cnt--;
                        revertcount--;
                    }
                    dpllend = clock();
                    return INFEASIBLE;
                }
            }
        }
        else
            break;
    }
    if(Judge(clause))
        return SATISFIABLE;//满足题意
    int guess = GuessLiteral(clause);
    SetLiteralTrue(clause, guess);//将猜测的值设为真
    dpllend = clock();
    if(DPLL(clause) == SATISFIABLE)
    {
        return SATISFIABLE;
    }
    else
    {
        Revert(clause);//只把guess设成真的那一步回溯
        SetLiteralFalse(clause, guess);//对的不行那就设成错的
        cnt++;
        revertcount++;
        if(DPLL(clause) == SATISFIABLE)
        {

            return SATISFIABLE;
        }
        else
        {
            while(cnt > 0)
            {
                Revert(clause);
                cnt--;
                revertcount--;
            }
            return INFEASIBLE;
        }
    }
}

//===============================================================================================
int layout[10][10] = {0};

int TracelessDPLL(Clause &clause)
{
    if(DPLL(clause) == SATISFIABLE)
    {
        while(revertcount > 0)
        {
            Revert(clause);
            revertcount--;
        }
        return SATISFIABLE;
    }
    else
        return INFEASIBLE;
}
//----------------------------------------------------------------------------------
//输出当前布局
void PrintLayout()
{
    printf("┌───┬───┬───╥───┬───┬───╥───┬───┬───┐\n");
    for(int i=1; i<=9; i++)
    {
        printf("│");
        for(int j=1; j<=9; j++)
        {
            if(layout[i][j] != 0)
            {
                if(j%3 == 0 && j != 9)
                    printf("  %d║", layout[i][j]);
                else if(j != 9)
                    printf("  %d┆", layout[i][j]);
                else
                    printf("  %d│", layout[i][j]);
            }
            else
            {
                if(j%3 == 0 && j != 9)
                    printf("   ║", layout[i][j]);
                else if(j != 9)
                    printf("   ┆", layout[i][j]);
                else
                    printf("   │", layout[i][j]);
            }
        }

        if(i == 9)
            printf("\n└───┴───┴───╨───┴───┴───╨───┴───┴───┘\n");
        else if(i%3 == 0)
            printf("\n╞═══╪═══╪═══╬═══╪═══╪═══╬═══╪═══╪═══╡\n");
        else
            printf("\n├┈┈┈┼┈┈┈┼┈┈┈╫┈┈┈┼┈┈┈┼┈┈┈╫┈┈┈┼┈┈┈┼┈┈┈┤\n");

    }
}


//----------------------------------------------------------------------------------
//生成数独：利用随机数生成数独初始界面
int seed = 0;
int GenerateSudoku(Clause &clause)
{
    srand(time(NULL));
    int sum = rand()%13 + 17;//随机生成初始布局的数字总量，从17到30不等
    int cnt = 0;
    int blockrow = 0;
    int blockcolumn = 0;
    int flag = 0;//0代表在初期给每个宫都随机生成数字的情况
    int recordrow[10] = {0};//用于统计每行都有多少数
    int recordcolumn[10] = {0};//用于统计每列都有多少数
    while(cnt < sum)
    {
        seed++;
        srand(time(NULL) + pow(seed, 3));
        int randnum = rand()%9+1;//生成一个从1到9的数
        seed++;
        srand(time(NULL) + pow(seed, 3));
        int posinblock = rand()%9+1;//生成一个九宫格中的位置
        int literal = 0;
        if(flag == 0)
        {
            if(posinblock%3 == 0)
                literal = (3*blockrow+posinblock/3)*100 + (3*blockcolumn+3)*10 + randnum;
            else
                literal = (3*blockrow+posinblock/3+1)*100 + (3*blockcolumn+posinblock%3)*10 + randnum;
            if(SetLiteralTrue(clause, literal) == INFEASIBLE)
            {

                Revert(clause);//猜的数不满足约束
            }
            else
            {
                layout[literal/100][(literal%100)/10] = literal%10;
                recordrow[literal/100]++;
                recordcolumn[(literal%100)/10]++;
                cnt++;
                if(blockrow == 2 && blockcolumn == 2)
                    flag = 1;
                else
                {
                    blockcolumn++;
                    if(blockcolumn == 3)
                    {
                        blockrow++;
                        blockcolumn = 0;
                    }//最大地保证每个宫都生成了数
                }
            }
        }

        else if(flag == 1)
        {
            int newrow = 0, newcolumn = 0;
            for(int i=2; i<=8; i++)
            {
                if(recordrow[i] == 0 &&
                   (recordrow[i-1] == 0 || recordrow[i+1] == 0))
                {
                    newrow = i;
                    break;
                }
            }

            for(int i=2; i<=8; i++)
            {
                if(recordcolumn[i] == 0 &&
                   (recordcolumn[i-1] == 0 || recordcolumn[i+1] == 0))
                {
                    newcolumn = i;
                    break;
                }
            }
            if(newrow == 0 && newcolumn == 0)
            {
                flag = 2;//没有连空两行或者两列
                continue;
            }

            else if(newrow == 0)
            {
                seed++;
                srand(time(NULL) + pow(seed, 3));
                newcolumn = rand()%9 + 1;
            }//只有连空两行
            else if(newcolumn == 0)
            {
                seed++;
                srand(time(NULL) + pow(seed, 3));
                newrow = rand()%9 + 1;
            }//只有连空两列
            literal = newrow*100 + newcolumn*10 + randnum;
            if(SetLiteralTrue(clause, literal) == INFEASIBLE)
            {
                Revert(clause);
            }
            else
            {
                layout[literal/100][(literal%100)/10] = literal%10;
                recordrow[newrow]++;
                recordcolumn[newcolumn]++;
                cnt++;
            }
        }

        else if(flag == 2)
        {
            int newrow = 0;
            int newcolumn = 0;
            seed++;
            srand(time(NULL) + pow(seed, 3));
            newrow = rand()%9 + 1;
            seed++;
            srand(time(NULL) + pow(seed, 3));
            newcolumn = rand()%9 + 1;
            if(layout[newrow][newcolumn] == UNCERTAIN)
            {
                int randnum = 0;
                seed++;
                srand(time(NULL) + pow(seed, 3));
                randnum = rand()%9 + 1;
                literal = newrow*100 + newcolumn*10 + randnum;
                if(SetLiteralTrue(clause, literal) == INFEASIBLE)
                {
                    Revert(clause);
                }
                else
                {
                    layout[literal/100][(literal%100)/10] = literal%10;
                    recordrow[newrow]++;
                    recordcolumn[newcolumn]++;
                    cnt++;
                }
            }
        }
    }
    return sum;
}
//----------------------------------------------------------------------------------
//进阶版生成数独（必有解）
int GenerateSudokuPro(Clause &clause)
{
    int returnsaver = GenerateSudoku(clause);
    while(TracelessDPLL(clause) == INFEASIBLE)
    {
        while(returnsaver > 0)
        {
            Revert(clause);
            returnsaver--;
        }
        for(int i=1; i<=9; i++)
            for(int j=1; j<=9; j++)
                layout[i][j] = 0;
        returnsaver = GenerateSudoku(clause);
    }
}
//----------------------------------------------------------------------------------
//生成初始CNF范式
void GenerateCNF(Clause &clause)
{
    int current = 0;
    clause.elem = (ClauseNode *)malloc(12000*sizeof(ClauseNode));
    for(int i=1; i<=9; i++)
    {
        for(int j=1; j<=9; j++)
        {
            clause.elem[current].literal = NULL;
            clause.elem[current].literalnum = 9;
            clause.elem[current].tag = UNCERTAIN;

            for(int k=1; k<=9; k++)
            {
                LiteralNode *p = clause.elem[current].literal;
                p = (LiteralNode *)malloc(sizeof(LiteralNode));
                p->data = i*100 + j*10 + k;//代表第i行第j列填k
                p->tag = TRUE;
                p->next = clause.elem[current].literal;
                clause.elem[current].literal = p;
            }
            current++;
            for(int k=1; k<=8; k++)
            {
                for(int h=k+1; h<=9; h++)
                {
                    clause.elem[current].literal = NULL;
                    clause.elem[current].literalnum = 2;
                    clause.elem[current].tag = UNCERTAIN;
                    LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                    p->data = i*100 + j*10 + k;//代表第i行第j列填k
                    p->tag = FALSE;
                    p->next = clause.elem[current].literal;
                    clause.elem[current].literal = p;

                    LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                    q->data = i*100 + j*10 + h;//代表第i行第j列填k
                    q->tag = FALSE;
                    q->next = clause.elem[current].literal;
                    clause.elem[current].literal = q;
                    current++;//以上17行是将“在同一格中不能填入两个数”转化为长度为2的子句
                }
            }
        }
    }

    for(int i=1; i<=9; i++)
    {
        for(int j=1; j<=9; j++)//j代表数字1~9
        {
            clause.elem[current].literal = NULL;
            clause.elem[current].literalnum = 9;
            clause.elem[current].tag = UNCERTAIN;
            for(int k=1; k<=9; k++)
            {
                LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                p->data = i*100 + k*10 + j;//代表第i行第k列填j
                p->tag = TRUE;
                p->next = clause.elem[current].literal;
                clause.elem[current].literal = p;
            }
            current++;
        }
        for(int j=1; j<=8; j++)
        {
            for(int k=j+1; k<=9; k++)
            {
                for(int h=1; h<=9; h++)
                {
                    clause.elem[current].literal = NULL;
                    clause.elem[current].literalnum = 2;
                    clause.elem[current].tag = UNCERTAIN;
                    LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                    p->data = i*100 + j*10 + h;//代表第i行第j列填h
                    p->tag = FALSE;
                    p->next = clause.elem[current].literal;
                    clause.elem[current].literal = p;


                    LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                    q->data = i*100 + k*10 + h;//代表第i行第k列填h
                    q->tag = FALSE;
                    q->next = clause.elem[current].literal;
                    clause.elem[current].literal = q;
                    current++;
                }
            }
        }
    }

    for(int i=1; i<=9; i++)
    {
        for(int j=1; j<=9; j++)//j代表数字1~9
        {
            clause.elem[current].literal = NULL;
            clause.elem[current].literalnum = 9;
            clause.elem[current].tag = UNCERTAIN;
            for(int k=1; k<=9; k++)
            {
                LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                p->data = k*100 + i*10 + j;//代表第k行第i列填j
                p->tag = TRUE;
                p->next = clause.elem[current].literal;
                clause.elem[current].literal = p;
            }
            current++;
        }

        //将“每列不能出现同样的数”转化为子句
        for(int j=1; j<=8; j++)
        {
            for(int k=j+1; k<=9; k++)
            {
                for(int h=1; h<=9; h++)
                {
                    clause.elem[current].literal = NULL;
                    clause.elem[current].literalnum = 2;
                    clause.elem[current].tag = UNCERTAIN;
                    LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                    p->data = j*100 + i*10 + h;//代表第i行第j列填k
                    p->tag = FALSE;
                    p->next = clause.elem[current].literal;
                    clause.elem[current].literal = p;

                    LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                    q->data = k*100 + i*10 + h;//代表第i行第k列填h
                    q->tag = FALSE;
                    q->next = clause.elem[current].literal;
                    clause.elem[current].literal = q;
                    current++;
                }
            }
        }
    }

    for(int i=0; i<=2; i++)
    {
        for(int j=0; j<=2; j++)
        {
            for(int m=1; m<=9; m++)
            {
                clause.elem[current].literal = NULL;
                clause.elem[current].literalnum = 9;
                clause.elem[current].tag = UNCERTAIN;
                for(int k=1; k<=3; k++)
                {
                    for(int h=1; h<=3; h++)
                    {
                        LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                        p->data = (3*i+k)*100 + (3*j+h)*10 + m;//代表第k行第i列填j
                        p->tag = TRUE;
                        p->next = clause.elem[current].literal;
                        clause.elem[current].literal = p;
                    }
                }
                current++;
                for(int k=1; k<=8; k++)
                {
                    for(int h=k+1; h<=9; h++)
                    {
                        clause.elem[current].literal = NULL;
                        clause.elem[current].literalnum = 2;
                        clause.elem[current].tag = UNCERTAIN;
                        LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                        if(k%3 == 0)
                            p->data = (3*i+k/3)*100 + (3*j+3)*10 + m;//九宫格第三列
                        else
                            p->data = (3*i+k/3+1)*100 + (3*j+k%3)*10 + m;//商是九宫格中的行数，余数是列数
                        p->tag = FALSE;
                        p->next = clause.elem[current].literal;
                        clause.elem[current].literal = p;

                        LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                        if(h%3 == 0)
                            q->data = (3*i+h/3)*100 + (3*j+3)*10 + m;//九宫格第三列
                        else
                            q->data = (3*i+(h-1)/3+1)*100 + (3*j+h%3)*10 + m;//商是九宫格中的行数，余数是列数
                        q->tag = FALSE;
                        q->next = clause.elem[current].literal;
                        clause.elem[current].literal = q;
                        current++;
                    }
                }
            }
        }
    }
    clause.clausesum = current;
    clause.literalsum = 729;//81个格子，每个格子有9种填法
}
//----------------------------------------------------------------------------------
void LayouttoCNF(Clause &clause)
{
    for(int i=1; i<=9; i++)
        for(int j=1; j<=9; j++)
        {
            if(layout[i][j] != UNCERTAIN)
                SetLiteralTrue(clause, i*100 + j*10 + layout[i][j]);
        }
}
//----------------------------------------------------------------------------------
//求解数独
int SolveSudoku(Clause &clause)
{
    if(DPLL(clause))
    {
        for(int i=111; i<=999; i++)
            if(literalmap[i] == TRUE)
                layout[i/100][(i%100)/10] = i%10;
    }
    //else
        //printf("无解！");
}
//----------------------------------------------------------------------------------
//格式化函数：将所有状态恢复
void Format()
{
    int n;
    int state;
    while(literalstack.node != NULL)
        PopL(literalstack, n);//将文字栈清空
    while(clausestack.node != NULL)
    {
        while(EmptyTop(clausestack) == INFEASIBLE)
            DeleteTopC(clausestack, n, state);
        PopC(clausestack);
    }//将子句栈清空
    for(int i=0; i<3500; i++)
        literalmap[i] = UNCERTAIN;
    for(int i=0; i<12000; i++)
        clausemap[i] = UNCERTAIN;
    revertcount = 0;
    clausenum = 0, literalnum = 0;
    IniLStack(literalstack);
    IniCstack(clausestack);
}

int main()
{
    int startclock = clock();
    IniLStack(literalstack);
    IniCstack(clausestack);
    IniLStack(depthstack);//待删
    int op = 1;
    while(op)
    {
        Clause clause;
        clause.clausesum = 0;
        clause.literalsum = 0;
        clause.elem = NULL;
        Format();//格式化
        system("cls");//清屏
        printf("┏─────────────────────────────────────┓\n");
        printf("│     欢迎使用SAT求解器、数独游戏     │\n");
        printf("│            请选择您的操作：         │\n");
        printf("│                                     │\n");
        printf("│       1.SAT求解     2.数独游戏      │\n");
        printf("│              (0.退出)               │\n");
        printf("┗─────────────────────────────────────┛\n");//交互界面
        scanf("%d", &op);
        if(op == 1)
        {
            system("cls");//清屏
            printf("┏─────────────────────────────────────┓\n");
            printf("│          欢迎使用SAT求解器！        │\n");
            printf("│         请输入CNF范式文件名：       │\n");
            printf("┗─────────────────────────────────────┛\n");
            char filename[50];
            scanf("%s", filename);
            loadfile(clause, filename);
            int SATstart = clock();
            if(DPLL(clause) == SATISFIABLE)
            {
                int SATend = clock();
                printf("问题有解！解情况为：");
                for(int i=1; i<=clause.literalsum; i++)
                    printf("%d ",i*literalmap[i]);
                printf("\n求解时间：%dms\n", SATend - SATstart);
                getchar();
                printf("┏─────────────────────────────────────┓\n");
                printf("│        您需要把解存入文件吗？       │\n");
                printf("│         1.需要！    2.别存!         │\n");
                printf("┗─────────────────────────────────────┛\n");
                int op1;
                scanf("%d", &op1);
                if(op1 == 1)
                {
                    printf("请输入文件名：");
                    scanf("%s", filename);
                    savefile(clause, filename, SATend-SATstart, 1);
                    printf("保存成功！");
                    getchar();
                    getchar();
                }
            }
            else
            {
                int SATend = clock();
                printf("问题无解！\n");
                printf("\n求解时间：%dms\n", SATend - SATstart);
                getchar();
                getchar();
            }
        }
        else if(op == 2)
        {
            system("cls");//清屏
            printf("┏─────────────────────────────────────┓\n");
            printf("│           欢迎来到数独游戏！        │\n");
            printf("│         按任意键即可生成数独        │\n");
            printf("┗─────────────────────────────────────┛\n");
            GenerateCNF(clause);
            getchar();
            getchar();
            GenerateSudokuPro(clause);
            PrintLayout();
            printf("┏─────────────────────────────────────┓\n");
            printf("│              数独已生成！           │\n");
            printf("│ 您想：1.玩数独2.查看数独答案（终局）│\n");
            printf("┗─────────────────────────────────────┛\n");
            int op2;
            scanf("%d", &op2);
            if(op2 == 1)
            {
                int op3 = 1;
                int layout2[10][10];
                for(int i=1; i<=9; i++)
                    for(int j=1; j<=9; j++)
                        layout2[i][j] = layout[i][j];
                int cnt = 0;
                while(op3)
                {
                    system("cls");
                    PrintLayout();

                    printf("\n1.填入数字  2.看看现在填得对不对  0.不玩了！\n");
                    scanf("%d", &op3);
                    if(op3 == 1)
                    {
                        printf("请依次输入行数、列数、填的数字：（数字0代表擦除）\n");
                        int row, column, num;
                        scanf("%d%d%d", &row, &column, &num);
                        layout[row][column] = num;
                        if(num == 0)
                        {
                            if(cnt == 0)
                                printf("\n还未填写！");
                            else
                            {
                                Revert(clause);
                                cnt--;
                            }
                        }
                        else
                        {
                            SetLiteralTrue(clause, row*100 + column*10 + num);
                            cnt++;
                        }
                    }
                    else if(op3 == 2)
                    {
                        if(TracelessDPLL(clause) == SATISFIABLE)
                            printf("有解！\n");
                        else
                            printf("无解！\n");
                        revertcount = 0;
                        getchar();
                        getchar();
                    }
                    else if(op3 == 0)
                    {
                        while(cnt > 0)
                        {
                            Revert(clause);
                            cnt--;
                        }
                        for(int i=1; i<=9; i++)
                            for(int j=1; j<=9; j++)
                                layout[i][j] = layout2[i][j];
                    }
                }
            }

            printf("答案如下，");
            int solvestart = clock();
            SolveSudoku(clause);
            int solveend = clock();
            printf("求解时间为：%dms\n", solveend - solvestart);
            PrintLayout();
            getchar();
            getchar();
        }
        else if(op == 0)
        {
            printf("\n感谢您的使用，再见！\n");
            break;
        }
    }
    int endclock = clock();
    printf("\n运行总时间：%dms", endclock-startclock);
    return 0;
}
