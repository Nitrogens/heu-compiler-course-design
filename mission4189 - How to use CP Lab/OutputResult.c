#include "OutputResult.h"

int g_address[100] = {0};
int g_count = 0;

/*
功能：
	输出 NFA。

参数：
	result -- NFA 的开始状态指针。
*/
void OutputResult(NFAState* result)
{
	if (result == NULL)
	{
		return;
	}
	int k = 0;
	for (k = 0; k < 100; k++)
	{
		if ((int)result == g_address[k])
		{
			return;
		}	
	}
	if (result != NULL)
	{
		g_address[g_count++] = (int)result;
		
		if (result -> Next1 != NULL)
		{	
			printf("%d(%d)", result->Name, result->AcceptFlag);
			if (result->Transform == '$')
			{
				printf("->$->%d", result->Next1->Name);
			}
			else
			{
				printf("->%c->%d", result->Transform, result->Next1->Name);
			}	
	
			printf("\n");	
		}
		if (result -> Next2 != NULL)
		{
			printf("%d(%d)", result->Name, result->AcceptFlag);
			if (result->Transform == '$')
			{
				printf("->$->%d", result->Next2->Name);
			}
			else
			{
				printf("->%c->%2d", result->Transform, result->Next2->Name);
			}		
			printf("\n");
		}
		
		if (result -> Next1 != NULL)
		{
			NFAState* result1 = result -> Next1;
			OutputResult(result1);
		}
		if (result -> Next2 != NULL)
		{
			NFAState* result2 = result -> Next2;
			OutputResult(result2);
		}	
		
		if (result->AcceptFlag == 1)
		{	
			printf("%d(%d)\n", result->Name, result->AcceptFlag);	
		}	
					
	}	
}
