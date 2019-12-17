#include "PickupLeftFactor.h"
#include <string.h>
#include <stdlib.h>

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
	// 输出提取左因子之前的文法
	//
	printf("Before Pickup Left Factor:\n");
	PrintRule(pHead);

	//
	// 调用 PickupLeftFactor 函数对文法提取左因子
	//
	PickupLeftFactor(pHead);
	
	//
	// 输出提取左因子之后的文法
	//
	printf("\nAfter Pickup Left Factor:\n");
	PrintRule(pHead);
	
	return 0;
}

/*
功能：
	根据下标找到 Select 中的一个 Symbol。

参数：
	pSelect -- Select 指针。
	index -- 下标。
	  
返回值：
	如果存在，返回找到的 Symbol 指针，否则返回 NULL。
*/
RuleSymbol* GetSymbol(RuleSymbol* pSelect, int index)
{
	int i = 0;
	RuleSymbol* pRuleSymbol;
	for (pRuleSymbol = pSelect, i = 0; pRuleSymbol != NULL; 
		pRuleSymbol = pRuleSymbol->pNextSymbol, i++)
	{
		if (i == index)
		{
			return pRuleSymbol;
		}
	}

	return NULL;	
}

/*
功能：
	以 SelectTemplate 为模板，确定左因子的最大长度。

参数：
	pSelectTemplate -- 作为模板的 Select 指针。
	  
返回值：
	左因子的最大长度，如果返回 0 说明不存在左因子。
*/
int LeftFactorMaxLength(RuleSymbol* pSelectTemplate)
{
	int result = 0;									// result 存储左因子的最大长度
	RuleSymbol* pSelect = pSelectTemplate->pOther;	// 从 pSelectTemplate 之后的 Select 开始遍历
	while (pSelect != NULL)
	{
		RuleSymbol* pSymbol = pSelect;
		int length = 0; // length 记录当前 Select 的左因子被匹配的长度
		while (pSymbol != NULL)
		{
			// 通过下标找到模板中对应的 Symbol
			RuleSymbol* pFound = GetSymbol(pSelectTemplate, length);
			// 如果未找到，直接跳出循环
			if (pFound == NULL)
			{
				break;
			}
			// 比较结果为 1 证明当前 Symbol 的内容与模板相同，length 加 1 之后继续遍历
			if (SymbolCmp(pSymbol, pFound) == 1)
			{
				length++;
			}
			// 如果结果为 0，证明当前 Symbol 的内容与模板不同，跳出循环
			else
			{
				break;
			}
			pSymbol = pSymbol->pNextSymbol;
		}
		// 更新最大答案
		if (length > result)
		{
			result = length;
		}
		pSelect = pSelect->pOther;
	}
	return result;
}

/*
功能：
	比较两个相同类型(同为终结符或同为非终结符)的 Symbol 是否具有相同的名字。

参数：
	pSymbol1 -- Symbol 指针。
	pSymbol2 -- Symbol 指针。
	  
返回值：
	相同返回 1，不同返回 0。
*/
int SymbolCmp(RuleSymbol* pSymbol1, RuleSymbol* pSymbol2)
{
	// 如果为终结符，则直接比较名字是否相同
	if (pSymbol1->isToken == 1)
	{
		if (pSymbol2->isToken == 1 && strcmp(pSymbol1->TokenName, pSymbol2->TokenName) == 0)
		{
			return 1;
		}
	}
	// 如果为非终结符，则比较对应 Rule 的名字是否相同
	else
	{
		if (pSymbol2->isToken == 0 && pSymbol1->pRule != NULL && pSymbol2->pRule != NULL && strcmp(pSymbol1->pRule->RuleName, pSymbol2->pRule->RuleName) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
功能：
	取文法中的一个 Select 与 SelectTemplate 进行比较，判断该 Select 是否需要提取左因子。

参数：
	pSelectTemplate -- 作为模板的 Select 指针。
	Count -- SelectTemplate 中已确定的左因子的数量。
	pSelect -- Select 指针。
	  
返回值：
	如果 Select 包含左因子返回 1，否则返回 0。
*/
int NeedPickup(RuleSymbol* pSelectTemplate, int Count, RuleSymbol* pSelect)
{
	int i;
	// 遍历模板和当前 Select，比较每一个对应位置的 Symbol 是否都相同
	// 如果都相同，证明需要提取左因子
	for (i = 0; i < Count; i++)
	{
		RuleSymbol *pSelect1 = GetSymbol(pSelectTemplate, i);
		RuleSymbol *pSelect2 = GetSymbol(pSelect, i);
		if (pSelect2 == NULL)
		{
			return 0;
		}
		if (SymbolCmp(pSelect1, pSelect2) == 0)
		{
			return 0;
		}
	}
	return 1;
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
		pNewSelect->pOther = NULL;
	}
	else
	{
		// 将新的 Select 加入到当前的 Rule 中
		pRule->pFirstSymbol = pNewSelect;
		pNewSelect->pOther = NULL;
	}
}

/*
功能：
	将 pRuleName 与文法中的其他 RuleName 比较, 如果相同就增加一个后缀。

参数：
	pHead -- Rule 链表的头指针。
	pRuleName -- Rule 的名字。
*/
void GetUniqueRuleName(Rule* pHead, char* pRuleName)
{
	Rule* pRuleCursor = pHead;
	for (; pRuleCursor != NULL;)
	{
		if (0 == strcmp(pRuleCursor->RuleName, pRuleName))
		{
			strcat(pRuleName, Postfix);
			pRuleCursor = pHead;
			continue;
		}
		pRuleCursor = pRuleCursor->pNextRule;
	}	
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
	提取左因子。

参数：
	pHead -- 文法的头指针。
*/
void PickupLeftFactor(Rule* pHead)
{
	Rule* pRule;		    	 // Rule 游标
	int isChange;				 // Rule 是否被提取左因子的标志
	RuleSymbol* pSelectTemplate; // Select 游标
	Rule* pNewRule; 			 // Rule 指针
	RuleSymbol* pSelect;		 // Select 游标
	
	do
	{
		isChange = 0;

		for (pRule = pHead; pRule != NULL; pRule = pRule->pNextRule)
		{
			// 取 Rule 中的一个 Select 作为模板，调用 LeftFactorMaxLength 函数确定左因子的最大长度
			int Count = 0;
			for (pSelectTemplate = pRule->pFirstSymbol; pSelectTemplate != NULL; pSelectTemplate = pSelectTemplate->pOther)
			{
				if ((Count = LeftFactorMaxLength(pSelectTemplate)) > 0)
					break;
			}

			// 忽略没用左因子的 Rule
			if (Count == 0)
				continue; 

			pNewRule = CreateRule(pRule->RuleName); // 创建新 Rule
			GetUniqueRuleName(pRule, pNewRule->RuleName);
			isChange = 1; // 设置标志

			// 调用 AddSelectToRule 函数把模板左因子之后的部分加到新 Rule 的末尾
			// 将模板左因子之后的部分替换为指向新 Rule 的非终结符
			AddSelectToRule(pNewRule, GetSymbol(pSelectTemplate, Count));
			// 存储左因子的结尾指针
			RuleSymbol *pSelectHead = GetSymbol(pSelectTemplate, Count - 1);
			// 创建一个新的指向新的 Rule 非终结符
			RuleSymbol *pSelectTail = CreateSymbol();
			pSelectTail->isToken = 0;
			pSelectTail->pRule = pNewRule;
			pSelectTail->pOther = NULL;
			pSelectTail->pNextSymbol = NULL;
			// 将左因子的末尾与新的非终结符进行连接
			pSelectHead->pNextSymbol = pSelectTail;

			// 从模板之后的位置循环查找包含左因子的 Select，并提取左因子
			pSelect = pSelectTemplate->pOther;
			RuleSymbol **pSelectPtr = &pSelectTemplate->pOther;
			while (pSelect != NULL)
			{
				if (NeedPickup(pSelectTemplate, Count, pSelect)) // Select 包含左因子
				{
					// 调用 AddSelectToRule 函数把左因子之后的部分加到新 Rule 的末尾
					// 将该 Select 从 Rule 中移除，释放内存，并移动游标
					RuleSymbol *pSelectTail = CopySelect(GetSymbol(pSelect, Count));
					AddSelectToRule(pNewRule, pSelectTail);
					*pSelectPtr = pSelect->pOther; 
					FreeSelect(pSelect);
					pSelect = *pSelectPtr;
				}
				else // Select 不包含左因子
				{
					// 移动游标
					pSelectPtr = &(pSelect->pOther);
					pSelect = pSelect->pOther;
				}
			}

			// 将新 Rule 加入到文法链表
			pNewRule->pNextRule = pRule->pNextRule;
			pRule->pNextRule = pNewRule;
			pRule = pRule->pNextRule;
		}

	} while (isChange == 1);
}

/*
功能：
	使用给定的数据初始化文法链表

返回值：
	文法的头指针
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
	/* A -> abC | abcD | abcE */
	{ "A", 
			{
				{ { 1, "a" }, { 1, "b" }, { 1, "C" } },
				{ { 1, "a" }, { 1, "b" }, { 1, "c" }, { 1, "D" } },
				{ { 1, "a" }, { 1, "b" }, { 1, "c" }, { 1, "E" } }
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
