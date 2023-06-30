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