#include "First.h"
#include <string.h>
#include <stdlib.h>

const char* VoidSymbol = "$"; // "ε"

char rule_table_ci[20][256];
char ruleNameArr[20][64];

int main(int argc, char* argv[])
{
	//
	// 调用 InitRules 函数初始化文法
	//
#ifdef CODECODE_CI
	Rule* pHead = InitRules_CI();  	// 此行代码在线上流水线运行
#else
	Rule* pHead = InitRules();		// 此行代码在 CP Lab 中运行
#endif
	
	//
	// 输出文法
	//
	PrintRule(pHead);
	
	//
	// 调用 First 函数求文法的 First 集合
	//
	SetList FirstSet;
	FirstSet.nSetCount = 0;
	First(pHead, &FirstSet);
	
	//
	// 输出First集合
	// 
	PrintFirstSet(&FirstSet);

	return 0;
}

/*
功能：
	添加一个 Set 到 SetList。

参数：
	pSetList -- SetList 指针。
	pName -- 集合名称字符串指针。 
*/
void AddOneSet(SetList* pSetList, const char* pName)
{
	// 如果当前要添加的 Set 已经存在于 SetList 中，则直接返回
	if (GetSet(pSetList, pName) != NULL)
	{
		return;
	}
	strcpy(pSetList->Sets[pSetList->nSetCount].Name, pName);
	pSetList->Sets[pSetList->nSetCount].nTerminalCount = 0;
	pSetList->nSetCount++;
}

/*
功能：
	根据名称在 SetList 中查找 Set。

参数：
	pSetList -- SetList 指针。
	pName -- 集合名称字符串指针。
	  
返回值：
	如果找到返回 Set 的指针，否则返回 NULL。
*/
Set* GetSet(SetList* pSetList, const char* pName)
{
	Set *pSet = pSetList->Sets;
	int i;
	for (i = 0; i < pSetList->nSetCount; i++)
	{
		if (strcmp(pSet->Name, pName) == 0)
		{
			return pSet;
		}
		pSet++;
	}
	return NULL;
}

/*
功能：
	根据名称在 Set 中查找 Terminal。

参数：
	pSet -- Set 指针。
	pName -- 终结符名称字符串指针。
	  
返回值：
	如果找到返回 1，否则返回 0。
*/
int GetTerminal(Set* pSet, const char* pName)
{
	int i;
	for (i = 0; i < pSet->nTerminalCount; i++)
	{
		if (strcmp(pSet->Terminal[i], pName) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
功能：
	添加一个终结符到 Set。

参数：
	pSet -- Set 指针。
	pTerminal -- 终结符名称指针。
	  
返回值：
	添加成功返回 1。
	添加不成功返回 0。
*/
int AddTerminalToSet(Set* pSet, const char* pTerminal)
{
	// 判断当前 Set 是否已满，如果已满直接返回 0
	if (pSet->nTerminalCount == 32)
	{
		return 0;
	}
	// 如果当前 Set 中没有要添加的 Terminal，则将这个 Terminal 添加到当前 Set 中
	if (GetTerminal(pSet, pTerminal) == 0)
	{
		strcpy(pSet->Terminal[pSet->nTerminalCount], pTerminal);
		pSet->nTerminalCount++;
		return 1;
	}
	// 如果未能添加，返回 0
	return 0;
}

/*
功能：
	将源 Set 中的所有终结符添加到目标 Set 中。

参数：
	pDesSet -- 目标 Set 指针。
	pSrcSet -- 源 Set 指针。
	  
返回值：
	添加成功返回 1，否则返回 0。
*/
int AddSetToSet(Set* pDesSet, const Set* pSrcSet)
{
	// 如果源 Set 和目标 Set 指针相同，则这一操作无法进行，直接返回 0
	if (pDesSet == pSrcSet)
	{
		return 0;
	}
	int i;
	for (i = 0; i < pSrcSet->nTerminalCount; i++)
	{
		// 尝试插入当前终结符，如果插入失败，则返回 0
		if (AddTerminalToSet(pDesSet, pSrcSet->Terminal[i]) == 0)
		{
			return 0;
		}
	}
	return 1;
}

/*
功能：
	判断 Set 的终结符中是否含有ε。

参数：
	pSet -- Set 指针。
	  
返回值：
	存在返回 1。
	不存在返回 0。
*/
int SetHasVoid(const Set* pSet)
{
	int i;
	for (i = 0; i < pSet->nTerminalCount; i++)
	{
		// 等于 0 表示当前的终结符为ε
		if (strcmp(pSet->Terminal[i], VoidSymbol) == 0)
		{
			return 1;
		}
	}
	return 0;			
}

/*
功能：
	求文法的 First 集合。

参数：
	pHead -- 文法的头指针。
	pFirstSetList -- First 集合指针。
*/
void First(const Rule* pHead, SetList* pFirstSetList)
{
	const Rule* pRule;  // Rule 指针
	int isChange;	    // 集合是否被修改的标志
	RuleSymbol* pSymbol;// Symbol 游标

	// 使用文法链表初始化 First 集合
	for(pRule = pHead; pRule != NULL; pRule = pRule->pNextRule)
	{
		AddOneSet(pFirstSetList, pRule->RuleName);
	}

	do
	{
		isChange = 0; // 设置修改标志

		for(pRule = pHead; pRule != NULL; pRule = pRule->pNextRule)
		{
			// 根据文法名称在 pFirstSetList 中查找 Set
			Set* pDesSet = GetSet(pFirstSetList, pRule->RuleName);

			int hasVoid = 1; // First 集合中是否含有ε的标志
			for(pSymbol = pRule->pFirstSymbol; pSymbol != NULL && hasVoid; pSymbol = pSymbol->pNextSymbol)
			{
				if(pSymbol->isToken)	// 终结符
				{
					// 调用 AddTerminalToSet 函数将终结符添加到 pDesSet，并设置修改标志
					if(AddTerminalToSet(pDesSet, pSymbol->SymbolName))
						isChange = 1;

					hasVoid = 0; // 设置 First 集合中是否含有ε的标志
				}
				else	// 非终结符
				{
					// 根据非终结符名称在 pFirstSetList 中查找 Set
					Set* pSrcSet = GetSet(pFirstSetList, pSymbol->SymbolName);

					// 调用 AddSetToSet 函数，将源 Set 中的所有终结符添加到目标 Set 中，并设置修改标志
					if(AddSetToSet(pDesSet, pSrcSet))
						isChange = 1;

					// 调用 SetHasVoid 函数，判断源 Set 中是否含有ε
					hasVoid = SetHasVoid(pSrcSet);
				}
			}
			if(hasVoid)
			{
				// 调用 AddTerminalToSet 函数将ε加入到目标集合中
				AddTerminalToSet(pDesSet, VoidSymbol);
			}
		}
	} while(isChange);
	
}

typedef struct _SYMBOL
{
	int isToken;
	char SymbolName[MAX_STR_LENGTH];
}SYMBOL;

typedef struct _RULE_ENTRY
{
	char RuleName[MAX_STR_LENGTH];
	SYMBOL Symbols[64];
}RULE_ENTRY;

static const RULE_ENTRY rule_table[] =
{
	/* exp -> exp addop term| term */
	{ "exp", { { 0, "exp" }, { 0, "addop"}, { 0, "term"} } },
	{ "exp", { { 0, "term" } } },

	/* addop -> + | - */
	{ "addop", { { 1, "+" } } },
	{ "addop", { { 1, "-" } } },

	/* term -> term mulop factor | factor */
	{ "term", { { 0, "term" }, { 0, "mulop"}, { 0, "factor"} } },
	{ "term", { { 0, "factor" } } },

	/* mulop -> * */
	{ "mulop", { { 1, "*" } } },

	/* factor -> (exp) | number */
	{ "factor", { { 1, "(" }, { 0, "exp"}, { 1, ")"} } },
	{ "factor", { { 1, "number" } } },
};

/*
功能：
	初始化文法链表。
	
返回值：
	文法的头指针。
*/
Rule* InitRules()
{
	Rule *pHead, *pRule;
	RuleSymbol **pSymbolPtr, *pNewSymbol;
	// 根据 rule_table 的占用空间和一个 rule 的占用空间，计算 rule 的数量
	int nRuleCount = sizeof(rule_table) / sizeof(rule_table[0]);
	int i, j;

	// pRulePtr 存储当前所要创建的 rule 的指针的地址
	Rule** pRulePtr = &pHead;
	// 创建每一个 rule
	for (i = 0; i < nRuleCount; i++)
	{
		// 根据名称创建新的 rule
		*pRulePtr = CreateRule(rule_table[i].RuleName);
		// 将指向当前 rule 的下一个 rule 的指针的地址存储到 pRulePtr 中，以备创建下一个 rule
		pRulePtr = &(*pRulePtr)->pNextRule;
	}

	// 遍历所有的 rule
	pRule = pHead;
	for (i = 0; i < nRuleCount; i++)
	{
		pSymbolPtr = &pRule->pFirstSymbol;
		// 遍历当前 rule 中定义好的所有 Symbol
		for (j = 0; rule_table[i].Symbols[j].SymbolName[0] != '\0'; j++)
		{
			// 存储预先定义的 symbol 作为后续创建 symbol 节点的参考
			const SYMBOL* pSymbol = &rule_table[i].Symbols[j];

			pNewSymbol = CreateSymbol();
			pNewSymbol->isToken = pSymbol->isToken;
			strcpy(pNewSymbol->SymbolName, pSymbol->SymbolName);
			// 在 pSymbolPtr 所指出的地址创建新的 symbol 节点
			*pSymbolPtr = pNewSymbol;
			
			// 将 pSymbolPtr 的值更新为下一个 symbol 指针的存储地址
			pSymbolPtr = &pNewSymbol->pNextSymbol;
		}

		pRule = pRule->pNextRule;
	}

	return pHead;
}

/*
功能：
	初始化文法链表(在执行流水线时调用)。
	
返回值：
	文法的头指针。
*/
Rule* InitRules_CI()
{
	int nRuleCount = 0;
	// 需要读取多行文本
	for (int i = 0; i < 20; i++)
	{
		gets(rule_table_ci[i]);	
		int length = strlen(rule_table_ci[i]);
		if (length == 0)
		{
			break;
		}
		
		for (int j = 0; j < length; j++)
		{
			if (rule_table_ci[i][j] == ' ')
			{
				ruleNameArr[i][j] = '\0';
				break;
			}
			ruleNameArr[i][j]= rule_table_ci[i][j];
		}	  
		nRuleCount++;
	}
	
	Rule *pHead, *pRule;
	RuleSymbol **pSymbolPtr, *pNewSymbol;

	Rule** pRulePtr = &pHead;
	for (int i=0; i<nRuleCount; i++)
	{
		*pRulePtr = CreateRule(ruleNameArr[i]);
		pRulePtr = &(*pRulePtr)->pNextRule;
	}

	pRule = pHead;
	for (int i=0; i<nRuleCount; i++)
	{
		pSymbolPtr = &pRule->pFirstSymbol;
		
		int start = 0;
		for (int j=0; rule_table_ci[i][j] != '\0'; j++)
		{
			if (rule_table_ci[i][j] == ' '
			 && rule_table_ci[i][j + 1] == '-'
			&& rule_table_ci[i][j + 2] == '>' 
			&& rule_table_ci[i][j + 3] == ' ')
			{
				start = j + 4;
				break;
			}
		}
			
		for (int k=start; rule_table_ci[i][k] != '\0'; k++)
		{
			if (rule_table_ci[i][k] == ' ')
			{
				continue;
			}
				
			pNewSymbol = CreateSymbol();
			char tokenName[MAX_STR_LENGTH] = {};
			
			for (int m = 0; ;m++)
			{
				if (rule_table_ci[i][k] ==  ' ' || rule_table_ci[i][k] == '\0' || rule_table_ci[i][k] == '\n')
				{
					tokenName[m] = '\0';
					break;
				}
				tokenName[m] = rule_table_ci[i][k++];
				
			}
			
			
			strcpy(pNewSymbol->SymbolName, tokenName);
			
			pNewSymbol->isToken = 1;
			for (int n = 0; n < nRuleCount; n++)
			{
				if (strcmp(pNewSymbol->SymbolName, ruleNameArr[n]) == 0)
				{
					pNewSymbol->isToken = 0;
					break;
				}
			}		
			
			*pSymbolPtr = pNewSymbol;

			pSymbolPtr = &pNewSymbol->pNextSymbol;
			
		}
			
		pRule = pRule->pNextRule;
	}

	return pHead;
}

/*
功能：
	创建一个新的文法。
	
参数：
	pRuleName -- 文法的名字。	
	
返回值：
	文法的指针。
*/
Rule* CreateRule(const char* pRuleName)
{
	Rule* pRule = (Rule*)malloc(sizeof(Rule));

	strcpy(pRule->RuleName, pRuleName);
	pRule->pFirstSymbol = NULL;
	pRule->pNextRule = NULL;

	return pRule;
}

/*
功能：
	创建一个新的符号。
	
返回值：
	符号的指针。
*/
RuleSymbol* CreateSymbol()
{
	RuleSymbol* pSymbol = (RuleSymbol*)malloc(sizeof(RuleSymbol));

	pSymbol->pNextSymbol = NULL;
	pSymbol->isToken = -1;
	pSymbol->SymbolName[0] = '\0';

	return pSymbol;
}

/*
功能：
	输出文法。
	
参数：
	pHead -- 文法的头指针。
*/
void PrintRule(Rule* pHead)
{
	Rule *pRule = pHead;
	// 遍历所有的 Rule
	while (pRule != NULL)
	{
		// 输出 Rule 的名称
		printf("%s ->", pRule->RuleName);
		RuleSymbol *pSymbol = pRule->pFirstSymbol;
		// 遍历当前 Rule 下所有的 Symbol
		while (pSymbol != NULL)
		{
			printf(" %s", pSymbol->SymbolName);
			pSymbol = pSymbol->pNextSymbol;
		}
		puts("");
		pRule = pRule->pNextRule;
	}
}

/*
功能：
	输出 First 集合。

参数：
	pFirstSetList -- First 集合指针。
*/
void PrintFirstSet(SetList* pFirstSetList)
{
	printf("\nThe First Set:\n");
	for (int i = 0; i < pFirstSetList->nSetCount; i++)
	{
		printf("First(%s) = { ", pFirstSetList->Sets[i].Name);
		for (int j = 0; j < pFirstSetList->Sets[i].nTerminalCount; j++)
		{
			if (j == pFirstSetList->Sets[i].nTerminalCount - 1)
			{
				printf("%s ", pFirstSetList->Sets[i].Terminal[j]);
			}
			else
			{
				printf("%s, ", pFirstSetList->Sets[i].Terminal[j]);
			}
			
		}
		printf("}\n");
	}
}

