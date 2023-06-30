#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
//���ֶ�Ӧ������״̬��
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
}LStack;//����ջ�����ڴ洢��������

typedef struct CStackNode{
    StackNode *node;
    struct CStackNode *next;
}CStacknode;

typedef struct CStack{
    CStacknode *node;
    int nodenum;
}CStack;//����ջ�����ڴ洢һ�����ӱ��

typedef struct ClauseNodeInMap{
    int data;
    struct ClauseNodeInMap *next;
}CNInMap;

typedef struct LiteralMapNode{
    CNInMap *node;
    int frequency;//���ڼ�����������������г���Ƶ�ʵļ�Ȩ
}LMapNode;//���ֱ����Ľ�㣬���ڱ��ÿ���������г��ֵ���䣬�Լ�ÿ�����ֳ��ֵ�Ƶ��

typedef struct ClauseNode{
    struct LiteralNode *literal;
    struct ClauseNode *next;
    int tag;
    int literalnum;
}ClauseNode;    //�Ӿ���Ķ���

typedef struct Clause{
    int clausesum;
    int literalsum;
    ClauseNode *elem;
}Clause;

typedef struct LiteralNode{
    int data;
    struct LiteralNode *next;
    int tag;
}LiteralNode; //���ֽڵ�Ķ���

int clausemap[12000] = {0};//0�����ѱ�����

int clausenum = 0, literalnum = 0;

LStack literalstack;//����ջ�����ڸ�ԭ����
CStack clausestack;//�Ӿ�ջ�����ڸ�ԭ�Ӿ�
LStack depthstack;

int testcounter = 0;
//-----------------------------------------------------------------------
//����ջջ��ʼ��������
void IniLStack(LStack &literalstack)
{
    literalstack.node = NULL;
    literalstack.nodenum = 0;
}
//-----------------------------------------------------------------------
//����ջ��ջ������
void PushL(LStack &literalstack, int n)
{
    StackNode *p = (StackNode *)malloc(sizeof(StackNode));
    p->data = n;
    p->next = literalstack.node;
    literalstack.node = p;
    literalstack.nodenum ++;
}

//-----------------------------------------------------------------------
//����ջ��ջ������
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
//�Ӿ�ջ��ʼ��������
void IniCstack(CStack &clausestack)
{
    clausestack.node = NULL;
    clausestack.nodenum = 0;
}

//-----------------------------------------------------------------------
//�Ӿ�ջ��ջ������
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
//�Ӿ�ջ��ջ������
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
//��ջ���Ӿ��ӵĺ�����
void AddTopC(CStack &clausestack, int n, int state)
{
    StackNode *p = (StackNode *)malloc(sizeof(StackNode));
    p->data = n;
    p->next = clausestack.node->node;
    clausestack.node->node = p;
    testcounter++;
}

//-----------------------------------------------------------------------
//ɾ��ջ���е�һ�����ӣ�����״̬����state
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
//�жϾ���ջ���޾��ӣ�
int EmptyTop(CStack clausestack)
{
    if(clausestack.node->node == NULL)
        return SATISFIABLE;
    return INFEASIBLE;
}


//-----------------------------------------------------------------------
//���ļ��ж�ȡcnf��ʽ��
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
    fscanf(fp, "%s", type);//��ȡ�ļ����ͣ�cnf��ʽ
    fscanf(fp, "%d", &literalnum);//��ȡ��ʽ��������
    fscanf(fp, "%d", &clausenum);//��ȡ��ʽ���Ӿ���
    printf("\n%s %d %d\n", type, literalnum, clausenum);
    clause.elem = (ClauseNode *)malloc(clausenum*sizeof(ClauseNode));//��ÿ���Ӿ䶥�㴴���ռ�
    clause.clausesum = clausenum;//����clausennum
    clause.literalsum = literalnum;//����literalnum
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
                clause.elem[i].literal = p;//����ͷ�巨�������ֽڵ�
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

int literalmap[3500];//����������ֵ�״̬���桢�ٻ򲻶�
//----------------------------------------------------------------------------------
//�������д���ļ���
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
//�ж������Ӿ����Ƿ��п��Ӿ�
int ContainsEmptyClause(Clause clause)
{
    for(int i=0; i<clause.clausesum; i++)
    {
        if(clause.elem[i].literalnum == 0)
            return SATISFIABLE;//����true�����п��Ӿ�
    }
    return INFEASIBLE;
}
//----------------------------------------------------------------------------------
//��ĳ��������Ϊ���桱
int SetLiteralTrue(Clause &clause, int literal)
{
    int emptyflag = 0;
    literalmap[literal] = TRUE;//����map�е�״̬Ϊ���桱
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
                if(p->data == literal)//��Ԫliteral������
                {
                    if(p->tag == 1)//��Ϊliteral�������
                        clause.elem[i].tag = TRUE;
                    else
                        clause.elem[i].literalnum--;//��Ϊ��literal����
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
//��ĳ��������Ϊ���١�
int SetLiteralFalse(Clause &clause, int literal)
{
    int emptyflag = 0;
    literalmap[literal] = FALSE;//����map�е�״̬Ϊ���١�
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
                if(p->data == literal)//��Ԫliteral������
                {
                    if(p->tag == -1)//��Ϊ��literal����
                        clause.elem[i].tag = TRUE;
                    else
                        clause.elem[i].literalnum--;//��Ϊliteral����
                    AddTopC(clausestack, i, clause.elem[i].tag);
                    if(clause.elem[i].literalnum == 0)//�����˿��Ӿ�
                        emptyflag = 1;//Ϊ�˷�ֹ����ʱ����һ��Ҫȫ������֮����ܷ���
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
//��������һ״̬��
int Revert(Clause &clause)
{
    int n, m;
    if(literalstack.node == NULL)
        printf("����ջΪ�գ�");
    PopL(literalstack, n);//����ջ��ջ
    int clausenumber;
    int state;
    while(EmptyTop(clausestack) == INFEASIBLE)//ջ���㲻Ϊ��
    {
        DeleteTopC(clausestack, m, state);//����ȥ��һ���Ӿ�
        /*if(clause.elem[m].literalnum == 0)
            printf("�գ�");*/
        if(clause.elem[m].tag == TRUE)
            clause.elem[m].tag = UNCERTAIN;//����һ����ΪTRUE��˵��֮ǰһ����UNCERTAIN
        else
            clause.elem[m].literalnum++;//Ҫô����һ����ΪFALSE��Ҫô������һ������UNCERTAIN�������������literalnum++
    }
    PopC(clausestack);//ջ��ȥ���Ӿ��Ϊ�գ���ôֱ�ӳ�ջ����ɾ��һ��
    literalmap[n] = UNCERTAIN;//��literalmap��n����ֵ�ָ�ΪUNCERTAIN
    /*for(int i=1; i<=clause.literalsum; i++)
        printf("%d\n", literalmap[i]);*/
    return SATISFIABLE;
}

//---------------------------------------------------------------------------------
//�ж��Ƿ��е��Ӿ䣬�����򷵻ص��Ӿ���Ψһһ��ʣ������֣����򷵻�FALSE
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
//�ж������Ƿ��ѱ�����
int Judge(Clause clause)
{
    for(int i=0; i<clause.clausesum; i++)
        if(clause.elem[i].tag != TRUE)
            return INFEASIBLE;
    return SATISFIABLE;
}

//----------------------------------------------------------------------------------
//�²�һ������Ϊ��
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
//DPLL�㷨��
int DPLL(Clause &clause)
{
    count++;
    int dpllstart = clock();
    int dpllend;
    int i;
    int flag = 0;
    int literal;
    int cnt = 0;//�������Զ��ٸ����ֵ���ֵ���й��Ķ����������
    while(TRUE)
    {
        int returnsaver = 0;//���ڴ洢����ֵ�����⺯���ظ�����
        if(returnsaver = UnitClause(clause))//�ҵ����Ӿ�
        {
            if(returnsaver > 0)//���Ӿ��е���������
            {
                cnt++;
                revertcount ++;
                if(SetLiteralTrue(clause, returnsaver) == INFEASIBLE)//�ѵ��Ӿ��е�������Ϊ��
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

            else//���Ӿ��е��ǡ��ǡ�����
            {
                cnt++;
                revertcount++;
                if(SetLiteralFalse(clause, -returnsaver) == INFEASIBLE)//�ѵ��Ӿ��е�������Ϊ��
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
        return SATISFIABLE;//��������
    int guess = GuessLiteral(clause);
    SetLiteralTrue(clause, guess);//���²��ֵ��Ϊ��
    dpllend = clock();
    if(DPLL(clause) == SATISFIABLE)
    {
        return SATISFIABLE;
    }
    else
    {
        Revert(clause);//ֻ��guess��������һ������
        SetLiteralFalse(clause, guess);//�ԵĲ����Ǿ���ɴ��
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
//�����ǰ����
void PrintLayout()
{
    printf("���������Щ������Щ������i�������Щ������Щ������i�������Щ������Щ�������\n");
    for(int i=1; i<=9; i++)
    {
        printf("��");
        for(int j=1; j<=9; j++)
        {
            if(layout[i][j] != 0)
            {
                if(j%3 == 0 && j != 9)
                    printf("  %d�U", layout[i][j]);
                else if(j != 9)
                    printf("  %d��", layout[i][j]);
                else
                    printf("  %d��", layout[i][j]);
            }
            else
            {
                if(j%3 == 0 && j != 9)
                    printf("   �U", layout[i][j]);
                else if(j != 9)
                    printf("   ��", layout[i][j]);
                else
                    printf("   ��", layout[i][j]);
            }
        }

        if(i == 9)
            printf("\n���������ة������ة������l�������ة������ة������l�������ة������ة�������\n");
        else if(i%3 == 0)
            printf("\n�b�T�T�T�n�T�T�T�n�T�T�T�p�T�T�T�n�T�T�T�n�T�T�T�p�T�T�T�n�T�T�T�n�T�T�T�e\n");
        else
            printf("\n���������੬�����੬�����o�������੬�����੬�����o�������੬�����੬������\n");

    }
}


//----------------------------------------------------------------------------------
//�����������������������������ʼ����
int seed = 0;
int GenerateSudoku(Clause &clause)
{
    srand(time(NULL));
    int sum = rand()%13 + 17;//������ɳ�ʼ���ֵ�������������17��30����
    int cnt = 0;
    int blockrow = 0;
    int blockcolumn = 0;
    int flag = 0;//0�����ڳ��ڸ�ÿ����������������ֵ����
    int recordrow[10] = {0};//����ͳ��ÿ�ж��ж�����
    int recordcolumn[10] = {0};//����ͳ��ÿ�ж��ж�����
    while(cnt < sum)
    {
        seed++;
        srand(time(NULL) + pow(seed, 3));
        int randnum = rand()%9+1;//����һ����1��9����
        seed++;
        srand(time(NULL) + pow(seed, 3));
        int posinblock = rand()%9+1;//����һ���Ź����е�λ��
        int literal = 0;
        if(flag == 0)
        {
            if(posinblock%3 == 0)
                literal = (3*blockrow+posinblock/3)*100 + (3*blockcolumn+3)*10 + randnum;
            else
                literal = (3*blockrow+posinblock/3+1)*100 + (3*blockcolumn+posinblock%3)*10 + randnum;
            if(SetLiteralTrue(clause, literal) == INFEASIBLE)
            {

                Revert(clause);//�µ���������Լ��
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
                    }//���ر�֤ÿ��������������
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
                flag = 2;//û���������л�������
                continue;
            }

            else if(newrow == 0)
            {
                seed++;
                srand(time(NULL) + pow(seed, 3));
                newcolumn = rand()%9 + 1;
            }//ֻ����������
            else if(newcolumn == 0)
            {
                seed++;
                srand(time(NULL) + pow(seed, 3));
                newrow = rand()%9 + 1;
            }//ֻ����������
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
//���װ��������������н⣩
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
//���ɳ�ʼCNF��ʽ
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
                p->data = i*100 + j*10 + k;//�����i�е�j����k
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
                    p->data = i*100 + j*10 + k;//�����i�е�j����k
                    p->tag = FALSE;
                    p->next = clause.elem[current].literal;
                    clause.elem[current].literal = p;

                    LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                    q->data = i*100 + j*10 + h;//�����i�е�j����k
                    q->tag = FALSE;
                    q->next = clause.elem[current].literal;
                    clause.elem[current].literal = q;
                    current++;//����17���ǽ�����ͬһ���в���������������ת��Ϊ����Ϊ2���Ӿ�
                }
            }
        }
    }

    for(int i=1; i<=9; i++)
    {
        for(int j=1; j<=9; j++)//j��������1~9
        {
            clause.elem[current].literal = NULL;
            clause.elem[current].literalnum = 9;
            clause.elem[current].tag = UNCERTAIN;
            for(int k=1; k<=9; k++)
            {
                LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                p->data = i*100 + k*10 + j;//�����i�е�k����j
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
                    p->data = i*100 + j*10 + h;//�����i�е�j����h
                    p->tag = FALSE;
                    p->next = clause.elem[current].literal;
                    clause.elem[current].literal = p;


                    LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                    q->data = i*100 + k*10 + h;//�����i�е�k����h
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
        for(int j=1; j<=9; j++)//j��������1~9
        {
            clause.elem[current].literal = NULL;
            clause.elem[current].literalnum = 9;
            clause.elem[current].tag = UNCERTAIN;
            for(int k=1; k<=9; k++)
            {
                LiteralNode *p = (LiteralNode *)malloc(sizeof(LiteralNode));
                p->data = k*100 + i*10 + j;//�����k�е�i����j
                p->tag = TRUE;
                p->next = clause.elem[current].literal;
                clause.elem[current].literal = p;
            }
            current++;
        }

        //����ÿ�в��ܳ���ͬ��������ת��Ϊ�Ӿ�
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
                    p->data = j*100 + i*10 + h;//�����i�е�j����k
                    p->tag = FALSE;
                    p->next = clause.elem[current].literal;
                    clause.elem[current].literal = p;

                    LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                    q->data = k*100 + i*10 + h;//�����i�е�k����h
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
                        p->data = (3*i+k)*100 + (3*j+h)*10 + m;//�����k�е�i����j
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
                            p->data = (3*i+k/3)*100 + (3*j+3)*10 + m;//�Ź��������
                        else
                            p->data = (3*i+k/3+1)*100 + (3*j+k%3)*10 + m;//���ǾŹ����е�����������������
                        p->tag = FALSE;
                        p->next = clause.elem[current].literal;
                        clause.elem[current].literal = p;

                        LiteralNode *q = (LiteralNode *)malloc(sizeof(LiteralNode));
                        if(h%3 == 0)
                            q->data = (3*i+h/3)*100 + (3*j+3)*10 + m;//�Ź��������
                        else
                            q->data = (3*i+(h-1)/3+1)*100 + (3*j+h%3)*10 + m;//���ǾŹ����е�����������������
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
    clause.literalsum = 729;//81�����ӣ�ÿ��������9���
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
//�������
int SolveSudoku(Clause &clause)
{
    if(DPLL(clause))
    {
        for(int i=111; i<=999; i++)
            if(literalmap[i] == TRUE)
                layout[i/100][(i%100)/10] = i%10;
    }
    //else
        //printf("�޽⣡");
}
//----------------------------------------------------------------------------------
//��ʽ��������������״̬�ָ�
void Format()
{
    int n;
    int state;
    while(literalstack.node != NULL)
        PopL(literalstack, n);//������ջ���
    while(clausestack.node != NULL)
    {
        while(EmptyTop(clausestack) == INFEASIBLE)
            DeleteTopC(clausestack, n, state);
        PopC(clausestack);
    }//���Ӿ�ջ���
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
    IniLStack(depthstack);//��ɾ
    int op = 1;
    while(op)
    {
        Clause clause;
        clause.clausesum = 0;
        clause.literalsum = 0;
        clause.elem = NULL;
        Format();//��ʽ��
        system("cls");//����
        printf("������������������������������������������������������������������������������\n");
        printf("��     ��ӭʹ��SAT�������������Ϸ     ��\n");
        printf("��            ��ѡ�����Ĳ�����         ��\n");
        printf("��                                     ��\n");
        printf("��       1.SAT���     2.������Ϸ      ��\n");
        printf("��              (0.�˳�)               ��\n");
        printf("������������������������������������������������������������������������������\n");//��������
        scanf("%d", &op);
        if(op == 1)
        {
            system("cls");//����
            printf("������������������������������������������������������������������������������\n");
            printf("��          ��ӭʹ��SAT�������        ��\n");
            printf("��         ������CNF��ʽ�ļ�����       ��\n");
            printf("������������������������������������������������������������������������������\n");
            char filename[50];
            scanf("%s", filename);
            loadfile(clause, filename);
            int SATstart = clock();
            if(DPLL(clause) == SATISFIABLE)
            {
                int SATend = clock();
                printf("�����н⣡�����Ϊ��");
                for(int i=1; i<=clause.literalsum; i++)
                    printf("%d ",i*literalmap[i]);
                printf("\n���ʱ�䣺%dms\n", SATend - SATstart);
                getchar();
                printf("������������������������������������������������������������������������������\n");
                printf("��        ����Ҫ�ѽ�����ļ���       ��\n");
                printf("��         1.��Ҫ��    2.���!         ��\n");
                printf("������������������������������������������������������������������������������\n");
                int op1;
                scanf("%d", &op1);
                if(op1 == 1)
                {
                    printf("�������ļ�����");
                    scanf("%s", filename);
                    savefile(clause, filename, SATend-SATstart, 1);
                    printf("����ɹ���");
                    getchar();
                    getchar();
                }
            }
            else
            {
                int SATend = clock();
                printf("�����޽⣡\n");
                printf("\n���ʱ�䣺%dms\n", SATend - SATstart);
                getchar();
                getchar();
            }
        }
        else if(op == 2)
        {
            system("cls");//����
            printf("������������������������������������������������������������������������������\n");
            printf("��           ��ӭ����������Ϸ��        ��\n");
            printf("��         �������������������        ��\n");
            printf("������������������������������������������������������������������������������\n");
            GenerateCNF(clause);
            getchar();
            getchar();
            GenerateSudokuPro(clause);
            PrintLayout();
            printf("������������������������������������������������������������������������������\n");
            printf("��              ���������ɣ�           ��\n");
            printf("�� ���룺1.������2.�鿴�����𰸣��վ֣���\n");
            printf("������������������������������������������������������������������������������\n");
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

                    printf("\n1.��������  2.����������öԲ���  0.�����ˣ�\n");
                    scanf("%d", &op3);
                    if(op3 == 1)
                    {
                        printf("����������������������������֣�������0���������\n");
                        int row, column, num;
                        scanf("%d%d%d", &row, &column, &num);
                        layout[row][column] = num;
                        if(num == 0)
                        {
                            if(cnt == 0)
                                printf("\n��δ��д��");
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
                            printf("�н⣡\n");
                        else
                            printf("�޽⣡\n");
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

            printf("�����£�");
            int solvestart = clock();
            SolveSudoku(clause);
            int solveend = clock();
            printf("���ʱ��Ϊ��%dms\n", solveend - solvestart);
            PrintLayout();
            getchar();
            getchar();
        }
        else if(op == 0)
        {
            printf("\n��л����ʹ�ã��ټ���\n");
            break;
        }
    }
    int endclock = clock();
    printf("\n������ʱ�䣺%dms", endclock-startclock);
    return 0;
}
