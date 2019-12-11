#include "NFAToDFA.h"
#include "RegexpToPost.h"
#include "PostToNFA.h"
#include "NFAStateStack.h"
#include "NFAFragmentStack.h"
#include "OutputResult.h"
#include <stdlib.h>

NFAFragmentStack FragmentStack; // 栈。用于储存 NFA 片段
NFAStateStack StateStack;		// 栈。用于储存 NFA 状态

const char VoidTrans = '$'; // 表示空转换


char* regexp = "a(a|1)*";			 // 例 1
// char* regexp = "(aa|b)*a(a|bb)*"; // 例 2
// char* regexp = "(a|b)*a(a|b)?"; 	 // 例 3

char regexp_ci[256];

int main(int argc, char **argv)
{
	char *post;
	DFA* dfa = (DFA*)malloc(sizeof(DFA));
	dfa->length = 0;
	
	//
	// 初始化栈
	//
	InitNFAFragmentStack(&FragmentStack);
	
	// 在 CP Lab中执行程序时不会使用下面宏定义包含的代码，提交作业后，在线上运行流水线时会包含宏定义这部分代码。
	// 下面宏定义代码会在运行流水线时从input.txt文件中读取正则表达式，进行算例的验证。
	// 其中，input1.txt ～ input3.txt文件中包含的正则表达式与例1 ～ 例3的正则表达式是对应的。	
#ifdef CODECODE_CI
	scanf("%255s", regexp_ci);
	regexp = regexp_ci;  	 
#endif	
	
	//
	// 调用 re2post 函数将正则表达式字符串转换成解析树的后序遍历序列
	//
	post = re2post(regexp);
	
	//
	// 调用 post2dfa 函数将解析树的后序遍历序列转换为 DFA 
	//
	dfa = post2dfa(dfa, post);
	
	//
	// 将 DFA 打印输出
	//
	OutputResult(dfa);
	
	FreeDFA(dfa);
	FreeNFA();
				
	return 0;
}

/*
功能：
	创建一个 DFA 状态的转换。
	
参数：
	TransformChar -- 转换符号。
	NFAStateArray -- NFA 状态指针数组。
	Count -- 数组元素个数。	
	  
返回值：
	 Transform 结构体指针。
*/
Transform* CreateDFATransform(char TransformChar, NFAState** NFAStateArray, int Count)
{
	int i;
	Transform* pTransform = (Transform*)malloc(sizeof(Transform));
	
	for (i = 0; i < Count; i++)
	{
		pTransform->NFAlist[i] = NFAStateArray[i];
	}
	
	pTransform->NFAStateCount = Count;	
	pTransform->TransformChar = TransformChar;
	pTransform->DFAStateIndex = -1;
	pTransform->NextTrans = NULL;
	
	return pTransform;
}

/*
功能：
	创建一个 DFA 状态。
	
参数：
	pTransform -- DFA 状态转换指针。	
	  
返回值：
	 DFAState 结构体指针。
*/
DFAState* CreateDFAState(Transform* pTransform)
{
	int i;
	DFAState* pDFAState = (DFAState*)malloc(sizeof(DFAState));
	
	for (i = 0; i < pTransform->NFAStateCount; i++)
	{
		pDFAState->NFAlist[i] = pTransform->NFAlist[i];
	}

	pDFAState->NFAStateCount = pTransform->NFAStateCount;
	pDFAState->firstTran = NULL;

	return pDFAState;
}

/*
功能：
	判断一个转换中的 NFA 状态集合是否为某一个 DFA 状态中 NFA 状态集合的子集。
	
参数：
	pDFA -- DFA 指针。
	pTransform -- DFA 状态转换指针。	
	  
返回值：
	 如果存在返回 DFA 状态下标，不存在返回 -1。
*/
int NFAStateIsSubset(DFA* pDFA, Transform* pTransform)
{
	int i, j, k;	
	// 遍历当前的 DFA 中的所有状态
	for (i = 0; i < pDFA->length; i++)
	{
		// cnt 用于记录 pTransform 中能够在当前 DFA 状态的 NFA 集合中找到的 NFA 数量
		int cnt = 0;
		// 遍历 pTransform 中的 NFA 集合
		for (j = 0; j < pTransform->NFAStateCount; j++)
		{
			// 遍历当前 DFA 状态的 NFA 集合
			for (k = 0; k < pDFA->DFAlist[i]->NFAStateCount; k++)
			{
				// 找到匹配的 NFA，数量加 1 并跳出循环
				if (pTransform->NFAlist[j] == pDFA->DFAlist[i]->NFAlist[k])
				{
					cnt++;
					break;
				}
			}
		}
		// 如果 pTransform 中的所有 NFA 均能被匹配
		// 则表明 pTransform 转换中的 NFA 状态集合为第 i 个 DFA 状态中 NFA 状态集合的子集
		if (cnt == pTransform->NFAStateCount)
		{
			return i;
		}
	}
	return -1;
}

/*
功能：
	判断某个 DFA 状态的转换链表中是否已经存在一个字符的转换。
	
参数：
	pDFAState -- DFAState 指针。
	TransformChar -- 转换标识符。
	  
返回值：
	 Transform 结构体指针。
*/
Transform* IsTransformExist(DFAState* pDFAState, char TransformChar)
{
	Transform* current;
	// 遍历当前 DFA 的所有转换，寻找匹配的转换
	for (current = pDFAState->firstTran; current != NULL; current = current->NextTrans)
	{
		// 如果匹配，返回当前转换的指针
		if (current->TransformChar == TransformChar) 
		{
			return current;
		}
	}
	// 未找到匹配的转换，返回空指针
	return NULL;
}

/*
功能：
	将一个 NFA 集合合并到一个 DFA 转换中的 NFA 集合中。
	注意，合并后的 NFA 集合中不应有重复的 NFA 状态。
	
参数：
	NFAStateArray -- NFA 状态指针数组，即待加入的 NFA 集合。
	Count -- 待加入的 NFA 集合中元素个数。
	pTransform -- 转换指针。
*/
void AddNFAStateArrayToTransform(NFAState** NFAStateArray, int Count, Transform* pTransform)
{
	int i;
	NFAState** pNFA = NFAStateArray;
	// 遍历 NFA 状态指针数组
	for (i = 0; i < Count; i++)
	{
		// 记录在 DFA 转换中是否找到当前所要加入的 NFA 状态，0 表示未找到
		int is_find = 0;
		// 遍历当前 DFA 转换中的 NFA 状态集合
		for (int j = 0; j < pTransform->NFAStateCount; j++)
		{
			// 发现重复，置 is_find 标记为 1 并跳出循环
			if (pTransform->NFAlist[j] == pNFA[i])
			{
				is_find = 1;
				break;
			}
		}
		// 如果未找到，则将当前 NFA 状态加入到 DFA 状态集合中
		if (is_find == 0)
		{
			pTransform->NFAlist[pTransform->NFAStateCount] = pNFA[i];
			pTransform->NFAStateCount++;
		}
	}
}

/*
功能：
	使用二叉树的先序遍历算法求一个 NFA 状态的ε-闭包。
	
参数：
	State -- NFA 状态指针。从此 NFA 状态开始求ε-闭包。
	StateArray -- NFA 状态指针数组。用于返回ε-闭包。
	Count -- 元素个数。	用于返回ε-闭包中 NFA 状态的个数。
*/
// DFS 函数用于实现对二叉树的先序遍历
void DFS(NFAState* State, NFAState** StateArray, int* Count)
{
	// 将当前状态存入 NFA 状态集合中
	StateArray[*Count] = State;
	(*Count)++;
	// 如果左子树存在且为 eplison 转换，则向左子树遍历
	if (State->Next1 != NULL && State->Transform == '$')
	{
		DFS(State->Next1, StateArray, Count);
	}
	// 如果右子树存在且为 eplison 转换，则向右子树遍历
	if (State->Next2 != NULL && State->Transform == '$')
	{
		DFS(State->Next2, StateArray, Count);
	}
}
void Closure(NFAState* State, NFAState** StateArray, int* Count)
{
	DFS(State, StateArray, Count);
}

/*
功能：
	将解析树的后序遍历序列转换为 DFA。

参数：
	pDFA -- DFA 指针。
	postfix -- 正则表达式的解析树后序遍历序列。
	  
返回值：
	DFA 指针。
*/
NFAState* Start = NULL;
DFA* post2dfa(DFA* pDFA, char *postfix)
{
	int i, j;								// 游标
	Transform* pTFCursor;  					// 转换指针
	NFAState* NFAStateArray[MAX_STATE_NUM]; // NFA 状态指针数组。用于保存ε-闭包
	int Count = 0;							// ε-闭包中元素个数
	
	//
	// 调用 post2nfa 函数将解析树的后序遍历序列转换为 NFA 并返回开始状态
	//
	Start = post2nfa(postfix);
	
	//
	// 调用 Closure 函数构造开始状态的ε-闭包
	//
	Closure(Start, NFAStateArray, &Count);

	// 调用 CreateDFATransform 函数创建一个转换(忽略转换字符)，
	// 然后，调用 CreateDFAState 函数，利用刚刚创建的转换新建一个 DFA 状态
	Transform* pTransform = CreateDFATransform('\0', NFAStateArray, Count);
	DFAState* pDFAState = CreateDFAState(pTransform);

	// 将 DFA 状态加入到 DFA 状态线性表中
	pDFA->DFAlist[pDFA->length++] = pDFAState;

	// 遍历线性表中所有 DFA 状态
	for (i = 0; i < pDFA->length; i++)
	{
		// 遍历第 i 个 DFA 状态中的所有 NFA 状态
		for (j = 0; j < pDFA->DFAlist[i]->NFAStateCount; j++)
		{
			NFAState* NFAStateTemp = pDFA->DFAlist[i]->NFAlist[j];

			// 如果 NFA 状态是接受状态或者转换是空转换，就跳过此 NFA 状态
			if (NFAStateTemp->Transform == VoidTrans || NFAStateTemp->AcceptFlag == 1)
			{
				continue;
			}

			// 调用 Closure 函数构造 NFA 状态的ε-闭包
			Count = 0;
			Closure(NFAStateTemp->Next1, NFAStateArray, &Count);

			// 调用 IsTransfromExist 函数判断当前 DFA 状态的转换链表中是否已经存在该 NFA 状态的转换
			pTransform = IsTransformExist(pDFA->DFAlist[i], NFAStateTemp->Transform);
			if (pTransform == NULL)
			{
				// 不存在，调用 CreateDFATransform 函数创建一个转换，并将这个转换插入到转换链表的开始位置
				pTransform = CreateDFATransform(NFAStateTemp->Transform, NFAStateArray, Count);
				pTransform->NextTrans = pDFA->DFAlist[i]->firstTran;
				pDFA->DFAlist[i]->firstTran = pTransform;
			}
			else
			{
				//存在，调用 AddNFAStateArrayToTransform 函数把ε-闭包合并到已存在的转换中
				AddNFAStateArrayToTransform(NFAStateArray, Count, pTransform);
			}
		}

		// 遍历 DFA 状态的转换链表，根据每个转换创建对应的 DFA 状态
		for (pTFCursor = pDFA->DFAlist[i]->firstTran; pTFCursor != NULL; pTFCursor = pTFCursor->NextTrans)
		{
			// 调用 NFAStateIsSubset 函数判断转换中的 NFA 状态集合是否为某一个 DFA 状态中 NFA 状态集合的子集
			int Index = NFAStateIsSubset(pDFA, pTFCursor);
			if (Index == -1)
			{
				// 不是子集，调用 CreateDFAState 函数创建一个新的 DFA 状态并加入 DFA 线性表中
				// 将转换的 DFAStateIndex 赋值为新加入的 DFA 状态的下标
				pTFCursor->DFAStateIndex = pDFA->length;
				pDFA->DFAlist[pDFA->length] = CreateDFAState(pTFCursor);
				pDFA->length++;
			}
			else
			{
				// 是子集，将转换的 DFAStateIndex 赋值为 Index
				pTFCursor->DFAStateIndex = Index;
			}
		}
	}

	return pDFA;
}

// 清除所有的 NFA
void FreeNFA()
{
	int i;
	for (i = 1; i <= cnt; i++)
	{
		free(NFAStateList[i]);
	}
}

// 清除所有的 DFA
void FreeDFA(DFA* dfa)
{
	int i;
	Transform* pTransform;
	Transform* nxt;
	for (i = 0; i < dfa->length; i++)
	{
		// 遍历当前 DFA 状态的所有转换并清除
		for (pTransform = dfa->DFAlist[i]->firstTran; pTransform != NULL; pTransform = nxt)
		{
			nxt = pTransform->NextTrans;
			free(pTransform);
		}
		// 清除当前 DFA 状态	
		free(dfa->DFAlist[i]);
	}
	// 清除 DFA 状态列表
	free(dfa);
}
