#include "RemoveLeftRecursion.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char* VoidSymbol = "$"; // "ε"
const char* Postfix = "'";

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
	// 输出消除左递归之前的文法
	//
	printf("Before Remove Left Recursion:\n");
	PrintRule(pHead);

	//
	// 调用 RemoveLeftRecursion 函数消除文法中的左递归
	//
	RemoveLeftRecursion(pHead);
	
	//
	// 输出消除左递归之后的文法
	//
	printf("\nAfter Remove Left Recursion:\n");
	PrintRule(pHead);
	
	return 0;
}

/*
功能：
	判断当前 Rule 中的一个 Symbol 是否需要被替换。
	如果 Symbol 是一个非终结符，且 Symbol 对应的
	Rule 在当前 Rule 之前，就需要被替换。

参数：
	pCurRule -- 当前 Rule 的指针。
	pSymbol -- Symbol 指针。
	pHead -- Rule 的头指针。
	  
返回值：
	需要替换返回 1。
	不需要替换返回 0。
*/
int SymbolNeedReplace(Rule* pHead, const Rule* pCurRule, RuleSymbol* pSymbol)
{
	// 如果当前 Symbol 是终结符的话，就一定不需要替换
	if (1 == pSymbol->isToken)
	{
		return 0;
	}
	Rule* pRule = pHead;
	// 遍历在当前 Rule 之前已经被处理的 Rule
	while (pRule != pCurRule)
	{
		// 如果当前 Rule 的名称与当前 Symbol 所指向的 Rule 名称相同，则表明需要替换
		if (pSymbol->pRule->RuleName == pRule->RuleName)
		{
			return 1;
		}
		pRule = pRule->pNextRule;
	}
	return 0;	
}

/*
功能：
	拷贝一个 Symbol。

参数：
	pSymbolTemplate -- 需要被拷贝的 Symbol 指针。
	  
返回值：
	拷贝获得的新 Symbol 的指针。
*/
RuleSymbol* CopySymbol(const RuleSymbol* pSymbolTemplate)
{
	if (pSymbolTemplate == NULL) 
	{
		return NULL;
	}
	// 创建新的 Symbol
	RuleSymbol *newSymbol = CreateSymbol();
	// 根据需要被拷贝的 Symbol 设置新 Symbol 的信息
	newSymbol->pNextSymbol = pSymbolTemplate->pNextSymbol;
	newSymbol->pOther = pSymbolTemplate->pOther;
	newSymbol->isToken = pSymbolTemplate->isToken;
	strcpy(newSymbol->TokenName, pSymbolTemplate->TokenName);
	newSymbol->pRule = pSymbolTemplate->pRule;
	return newSymbol;
}

/*
功能：
	拷贝一个 Select。

参数：
	pSelectTemplate -- 需要被拷贝的 Select 指针。
	  
返回值：
	拷贝获得的新 Select 的指针。
*/
RuleSymbol* CopySelect(RuleSymbol* pSelectTemplate)
{
	
	RuleSymbol *pSelect = NULL;				// 新 Select 链表的头指针
	RuleSymbol *pSymbol = pSelectTemplate;	// 当前正在被拷贝的 Symbol
	RuleSymbol *pNewSymbol;					// 新 Select 链表的末尾元素指针
	// 遍历原 Select 中的所有 Symbol，完成复制
	while (pSymbol != NULL)
	{
		// 如果头指针为空，则直接替换
		if (pSelect == NULL)
		{
			pSelect = CopySymbol(pSymbol);
			pNewSymbol = pSelect;
		}
		// 如果头指针非空，则将新的节点接到链表末尾
		else
		{
			pNewSymbol->pNextSymbol = CopySymbol(pSymbol);
			pNewSymbol = pNewSymbol->pNextSymbol;
		}
		pSymbol = pSymbol->pNextSymbol;
	}
	// 将末尾节点的下一个节点设置为空
	pNewSymbol->pNextSymbol = NULL;
	return pSelect;
}

/*
功能：
	替换一个 Select 的第一个 Symbol。

参数：
	pSelectTemplate -- 需要被替换的 Select 指针。
	  
返回值：
	替换后获得的新 Select 的指针。
	注意，替换后可能会有一个新的 Select，
	也可能会有多个 Select 链接在一起。
*/
RuleSymbol* ReplaceSelect(RuleSymbol* pSelectTemplate)
{
	// result 为新的 Select 的头节点指针
	RuleSymbol *result = NULL;
	// pNextSelect 指针指向新的 Select 指针存储的位置
	RuleSymbol **pNextSelect;
	// 如果为非终结符，则进行替换
	if (pSelectTemplate->isToken == 0)
	{
		RuleSymbol *pSelect = pSelectTemplate->pRule->pFirstSymbol;
		// 遍历所指向的 Rule 的所有 Select
		while (pSelect != NULL)
		{
			// 将正在遍历的 Select 复制一份
			RuleSymbol *pNewSelect = CopySelect(pSelect);
			// 将被替换部分的后继 Symbol 链表复制一份
			RuleSymbol *pNewSelectTail = CopySelect(pSelectTemplate->pNextSymbol);
			// 将复制好的 Select 与 pNewSelectTail 进行连接
			AddSymbolToSelect(pNewSelect, pNewSelectTail);
			// 生成的 Select 显然是新的 Select 链表的末尾
			pNewSelect->pOther = NULL;
			// 如果 result 为空，则直接将 result 替换为当前节点
			if (result == NULL)
			{
				result = pNewSelect;
				pNextSelect = &(pNewSelect->pOther);
			}
			// 如果 result 非空，则将其添加到链表末尾
			else
			{
				*pNextSelect = pNewSelect;
				pNextSelect = &(pNewSelect->pOther);
			}
			pSelect = pSelect->pOther;
		}
		
	}
	// 如果为终结符，则无需替换，直接复制一份之后返回即可
	else
	{
		result = CopySymbol(pSelectTemplate);
	}
	return result; 
}

/*
功能：
	释放一个 Select 的内存。

参数：
	pSelect -- 需要释放的 Select 的指针。
*/
void FreeSelect(RuleSymbol* pSelect)
{
	RuleSymbol *pSymbol = pSelect;
	// 遍历当前 Select 下的所有 Symbol 并进行删除
	while (pSymbol != NULL)
	{
		RuleSymbol *pTemp = pSymbol;
		pSymbol = pSymbol->pNextSymbol;
		free(pTemp);
	}
}

/*
功能：
	判断一条 Rule 是否存在左递归。

参数：
	prRule -- Rule 指针。
	  
返回值：
	存在返回 1。
	不存在返回 0。
*/
int RuleHasLeftRecursion(Rule* pRule)
{
	RuleSymbol *pSelect = pRule->pFirstSymbol;
	// 遍历所有的 Select
	while (pSelect != NULL)
	{
		// 如果当前 Select 指向的 Rule 就是自身的 Rule，表明存在左递归
		if (pSelect->isToken == 0 && pSelect->pRule->RuleName == pRule->RuleName)
		{
			return 1;
		}
		pSelect = pSelect->pOther;
	}
	return 0;
}

/*
功能：
	将一个 Symbol 添加到 Select 的末尾。

参数：
	pSelect -- Select 指针。
	pNewSymbol -- Symbol 指针。
*/
void AddSymbolToSelect(RuleSymbol* pSelect, RuleSymbol* pNewSymbol)
{
	RuleSymbol *pSymbol = pSelect;
	// 遍历查找下一个 Symbol 为空的 Symbol 指针，即当前 Select 的末尾 Symbol
	while (pSymbol != NULL && pSymbol->pNextSymbol != NULL)
	{
		pSymbol = pSymbol->pNextSymbol;
	}
	// 将新的 Symbol 加入到末尾 Symbol 的后面
	pSymbol->pNextSymbol = pNewSymbol;
	pNewSymbol->pNextSymbol = NULL;
	//assert(pSymbol->pNextSymbol != 0x1);
}

/*
功能：
	将一个 Select 加入到文法末尾，当 Select 为 NULL 时就将一个ε终结符加入到文法末尾。

参数：
	pRule -- 文法指针。
	pNewSelect -- Select 指针。
*/
void AddSelectToRule(Rule* pRule, RuleSymbol* pNewSelect)
{
	RuleSymbol *pSelect;
	// 如果新的 Select 为空则创建一个新的ε终结符
	if (pNewSelect == NULL)
	{
		pNewSelect = CreateSymbol();
		pNewSelect->pNextSymbol = NULL;
		pNewSelect->pOther = NULL;
		pNewSelect->isToken = 1;
		strcpy(pNewSelect->TokenName, VoidSymbol);
	}
	pSelect = pRule->pFirstSymbol;
	// 遍历查找下一个 Select 为空的 Select 指针，即当前 Rule 的末尾 Select
	while (pSelect != NULL && pSelect->pOther != NULL)
	{
		pSelect = pSelect->pOther;
	}
	// 如果 pSelect 非空
	if (pSelect != NULL)
	{
		// 将新的 Select 加入到末尾 Select 的后面
		pSelect->pOther = pNewSelect;
	}
	else
	{
		// 将新的 Select 加入到当前的 Rule 中
		pRule->pFirstSymbol = pNewSelect;
	}
}

/*
功能：
	消除左递归。

参数：
	pHead -- 文法链表的头指针。
*/
void RemoveLeftRecursion(Rule* pHead)
{
	Rule* pRule;				// Rule 游标
	RuleSymbol* pSelect; 		// Select 游标
	Rule* pNewRule;			  	// Rule 指针
	int isChange;				// Rule 是否被替换的标记
	RuleSymbol **pSelectPrePtr; // Symbol 指针的指针
	
	for (pRule = pHead; pRule != NULL; pRule = pRule->pNextRule)
	{
		//
		// 替换
		//
		do
		{
			isChange = 0;

			// 在 Rule 的所有 Select 中查找是否需要替换
			for (pSelect = pRule->pFirstSymbol, pSelectPrePtr = &pRule->pFirstSymbol;
				pSelect != NULL;
				pSelectPrePtr = &pSelect->pOther, pSelect = pSelect->pOther)
			{
				if (SymbolNeedReplace(pHead, pRule, pSelect)) // 判断 Select 的第一个 Symbol 是否需要替换
				{
					isChange = 1;

					// 调用 ReplaceSelect 函数，替换 Select 的第一个 Symbol 后得到新的 Selects
					RuleSymbol* pNewSelects = ReplaceSelect(pSelect);
					// 使用新的 Selects 替换原有的 Select，并调用 FreeSelect 函数释放原有的 Select 内存
					RuleSymbol* pNewSelectsTail = pNewSelects;
					while (pNewSelectsTail->pOther)
					{
						pNewSelectsTail = pNewSelectsTail->pOther;
					}
					// 将新的 Selects 的末尾连接当前 Select 的下一个 Select
					pNewSelectsTail->pOther = pSelect->pOther;
					// 将上一个 Select 的末尾连接当前 Select
					*pSelectPrePtr = pNewSelects;
					// 暂存当前 Select 的指针
					RuleSymbol *pTemp = pSelect;
					// 将当前 Select 更新为新的 Select
					pSelect = pNewSelects;
					// 释放被替换的 Select 所占用的内存
					FreeSelect(pTemp);
					break;
				}

				if (isChange)
					break;
			}
		} while (isChange);

		// 忽略没有左递归的 Rule;
		if (!RuleHasLeftRecursion(pRule))
			continue;

		//
		// 消除左递归
		//
		pNewRule = CreateRule(pRule->RuleName); // 创建新 Rule
		strcat(pNewRule->RuleName, Postfix);

		pSelect = pRule->pFirstSymbol; // 初始化 Select 游标
		pSelectPrePtr = &pRule->pFirstSymbol;
		while (pSelect != NULL) // 循环处理所有的 Select
		{
			if (0 == pSelect->isToken && pSelect->pRule == pRule) // Select 存在左递归
			{
				// 移除包含左递归的 Select，将其转换为右递归后添加到新 Rule 的末尾，并移动游标
				// 将当前 Select 的第二个 Symbol 及其后继复制一份，作为新的 Select 的开始
				RuleSymbol *pNewSelect = CopySelect(pSelect->pNextSymbol);
				// 将当前 Select 的头节点复制一份，作为新的 Select 的末尾
				RuleSymbol *pNewSelectTail = CopySymbol(pSelect);
				
				// 设置新的 Select 的末尾的参数
				pNewSelectTail->pRule = pNewRule;
				pNewSelectTail->pNextSymbol = NULL;
				pNewSelectTail->pOther = NULL;
				
				// 将新的 Select 的末尾连接上
				AddSymbolToSelect(pNewSelect, pNewSelectTail);
				// 设置新的 Select 的参数
				pNewSelect->pOther = NULL;
				// 将新的 Select 加入到新的 Rule 中
				AddSelectToRule(pNewRule, pNewSelect);
				
				// 暂存一份原始 Select 的指针备用
				RuleSymbol *pTemp = pSelect;
				// 更新存储指向上一个节点的下一个节点的指针
				*pSelectPrePtr = pSelect->pOther;
				// 将当前节点更新为当前节点的下一个节点
				pSelect = pSelect->pOther;
				
				// 清除暂存的 Select 所占的内存
				FreeSelect(pTemp);
			}
			else // Select 不存在左递归
			{
				// 在没有左递归的 Select 末尾添加指向新 Rule 的非终结符，并移动游标
				// 创建一个新的非终结符 Symbol，并设置参数
				RuleSymbol *pNewSymbol = CreateSymbol();
				pNewSymbol->pNextSymbol = NULL;
				pNewSymbol->pOther = NULL;
				pNewSymbol->isToken = 0;
				pNewSymbol->pRule = pNewRule;
				
				// 将新的非终结符加入到当前 Select 的末尾
				AddSymbolToSelect(pSelect, pNewSymbol);
				
				// 更新指向存储指向上一个节点的下一个节点的指针的指针
				pSelectPrePtr = &(pSelect->pOther);
				// 更新当前 Select
				pSelect = pSelect->pOther;
			}
		}

		// 在新 Rule 的最后加入ε(用 '$' 代替)
		// 将新 Rule 插入文法链表
		// 创建一个新的终结符ε，并设置参数
		RuleSymbol *pNewSymbol = CreateSymbol();
		pNewSymbol->pNextSymbol = NULL;
		pNewSymbol->pOther = NULL;
		pNewSymbol->isToken = 1;
		strcpy(pNewSymbol->TokenName, VoidSymbol);
		
		// 在新 Rule 的最后加入ε
		AddSelectToRule(pNewRule, pNewSymbol);
		
		pNewRule->pNextRule = pRule->pNextRule;
		pRule->pNextRule = pNewRule;
		pRule = pNewRule;
	}
}

/*
功能：
	使用给定的数据初始化文法链表

返回值：
	Rule 指针
*/
typedef struct _SYMBOL
{
	int isToken;
	char Name[MAX_STR_LENGTH];
}SYMBOL;

typedef struct _RULE_ENTRY
{
	char RuleName[MAX_STR_LENGTH];
	SYMBOL Selects[64][64];
}RULE_ENTRY;

static const RULE_ENTRY rule_table[] =
{
	/* A -> Ba | Aa | c */
	{ "A", 
			{
				{ { 0, "B" }, { 1, "a"} },
				{ { 0, "A" }, { 1, "a"} },
				{ { 1, "c" } }
			}	
	},

	/* B -> Bb | Ab | d */
	{ "B", 
			{
				{ { 0, "B" }, { 1, "b"} },
				{ { 0, "A" }, { 1, "b"} },
				{ { 1, "d" } }
			}	
	}
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
	RuleSymbol **pSymbolPtr1, **pSymbolPtr2;
	// 根据 rule_table 的占用空间和一个 rule 的占用空间，计算 rule 的数量
	int nRuleCount = sizeof(rule_table) / sizeof(rule_table[0]);
	int i, j, k;

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
		pSymbolPtr1 = &pRule->pFirstSymbol;
		// 遍历当前 rule 中定义好的所有 select
		for (j = 0; rule_table[i].Selects[j][0].Name[0] != '\0'; j++)
		{
			pSymbolPtr2 = pSymbolPtr1;
			// 遍历当前 select 中定义好的所有 rule
			for (k = 0; rule_table[i].Selects[j][k].Name[0] != '\0'; k++)
			{
				// 存储预先定义的 symbol 作为后续创建 symbol 节点的参考
				const SYMBOL* pSymbol = &rule_table[i].Selects[j][k];
				
				// 在 pSymbolPtr2 所指出的地址创建新的 symbol 节点
				*pSymbolPtr2 = CreateSymbol();
				// 根据定义的 isToken 设置节点的 isToken
				(*pSymbolPtr2)->isToken = pSymbol->isToken;
				// 如果是终结符，则直接将名称复制到 TokenName 中
				if (1 == pSymbol->isToken)
				{
					strcpy((*pSymbolPtr2)->TokenName, pSymbol->Name);
				}
				// 如果是非终结符，则先找到名称所对应的节点再进行连接
				else
				{
					(*pSymbolPtr2)->pRule = FindRule(pHead, pSymbol->Name);
					if (NULL == (*pSymbolPtr2)->pRule)
					{
						printf("Init rules error, miss rule \"%s\"\n", pSymbol->Name);
						exit(1);
					}
				}
				// 将 pSymbolPtr2 的值更新为下一个 symbol 指针的存储地址
				pSymbolPtr2 = &(*pSymbolPtr2)->pNextSymbol;
			}
			// 将 pSymbolPtr1 的值更新为下一个 select 指针的存储地址
			pSymbolPtr1 = &(*pSymbolPtr1)->pOther;
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
	RuleSymbol **pSymbolPtr1, **pSymbolPtr2;
		
	int i, j, k;

	Rule** pRulePtr = &pHead;
	for (i=0; i<nRuleCount; i++)
	{
		*pRulePtr = CreateRule(ruleNameArr[i]);
		pRulePtr = &(*pRulePtr)->pNextRule;
	}

	pRule = pHead;
	for (i=0; i<nRuleCount; i++)
	{
		pSymbolPtr1 = &pRule->pFirstSymbol;
		
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
			
		for (k = start; rule_table_ci[i][k] != '\0'; k++)
		{
			if (rule_table_ci[i][k] == '|')
			{
				pSymbolPtr1 = &(*pSymbolPtr1)->pOther;
				pSymbolPtr2 = pSymbolPtr1;
				continue;
			}
			if (rule_table_ci[i][k] == ' ')
			{
				continue;
			}
			if (k == start)
			{
				pSymbolPtr2 = pSymbolPtr1;
			}

			*pSymbolPtr2 = CreateSymbol();
			
			char tokenName[MAX_STR_LENGTH] = {};
			tokenName[0] = rule_table_ci[i][k];
			tokenName[1] = '\0';
			(*pSymbolPtr2)->isToken = 1;
			for (int m = 0; m < nRuleCount; m++)
			{
				if (strcmp(tokenName, ruleNameArr[m]) == 0)
				{
					(*pSymbolPtr2)->isToken = 0;
					(*pSymbolPtr2)->pRule = FindRule(pHead, tokenName);
					if (NULL == (*pSymbolPtr2)->pRule)
					{
						printf("Init rules error, miss rule \"%s\"\n", tokenName);
						exit(1);
					}
				}
			}
			if ((*pSymbolPtr2)->isToken == 1)
			{
				strcpy((*pSymbolPtr2)->TokenName, tokenName);
			}
			
			pSymbolPtr2 = &(*pSymbolPtr2)->pNextSymbol;
			
		}
			
		pRule = pRule->pNextRule;
	}

	return pHead;
}

/*
功能：
	创建一个新的 Rule。

参数：
	pRuleName -- 文法的名字。
	
返回值：
	Rule 指针
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
	创建一个新的 Symbol。
	
返回值：
	RuleSymbol 指针	
*/
RuleSymbol* CreateSymbol()
{
	RuleSymbol* pSymbol = (RuleSymbol*)malloc(sizeof(RuleSymbol));

	pSymbol->pNextSymbol = NULL;
	pSymbol->pOther = NULL;
	pSymbol->isToken = -1;
	pSymbol->TokenName[0] = '\0';
	pSymbol->pRule = NULL;

	return pSymbol;
}

/*
功能：
	根据 RuleName 在文法链表中查找名字相同的文法。

参数：
	pHead -- 文法的头指针。
	RuleName -- 文法的名字。
	
返回值：
	Rule 指针	
*/
Rule* FindRule(Rule* pHead, const char* RuleName)
{
	Rule* pRule;
	for (pRule = pHead; pRule != NULL; pRule = pRule->pNextRule)
	{
		if (0 == strcmp(pRule->RuleName, RuleName))
		{
			break;
		}
	}
	
	return pRule;
}

/*
功能：
	输出文法。

参数：
	pHead -- 文法的头指针。
*/
void PrintRule(Rule* pHead)
{
	// 遍历所有的 Rule
	while (pHead != NULL)
	{
		// 输出 Rule 的名称
		printf("%s->", pHead->RuleName);
		RuleSymbol *pSelect = pHead->pFirstSymbol;
		// 遍历当前 Rule 下所有的 Select
		while (pSelect != NULL)
		{
			RuleSymbol *pSymbol = pSelect;
			// 遍历当前 Select 下所有的 Symbol
			while (pSymbol != NULL)
			{
				// 如果是终结符，则直接输出名称
				if (pSymbol->isToken == 1)
				{
					printf("%s", pSymbol->TokenName);
				}
				// 如果是非终结符，则输出所指向 Rule 的名称
				else
				{
					printf("%s", pSymbol->pRule->RuleName);
				}
				pSymbol = pSymbol->pNextSymbol;
			}
			pSelect = pSelect->pOther;
			if (pSelect != NULL)
			{
				printf("|");
			}
		}
		puts("");
		pHead = pHead->pNextRule;
	}
}
