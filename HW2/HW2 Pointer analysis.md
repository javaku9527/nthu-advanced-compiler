# HW2 Pointer analysis

## How to run

1. 用makefile將測試檔案變成`.ll`檔案
    `make test`
    
2. 執行`run.sh`加上欲測試`.ll`檔名
    `./run.sh *.ll`
    
3. Pass印出分析出的Reaching Definition和Flow Equations結果

## Test Cases
此Pass能夠分析出每個Statement的Reaching Definition和Flow Equations, 範例如下:

測試案例1:

```
#include <stdio.h>

void icpp() 
{
  int x, y, *p, **pp;
  p = &x;
  pp = &p;
  *pp = &y;
  *p = 3;
}

```

結果:

![](https://i.imgur.com/hoOPae7.png)


## Implement

1. 此pass第一步爲尋找`Store`指令

2. 解析`=`號左方產生`TGEN`, 若type爲Pointer時(即出現`*`符號)要特別處理並加進`TREF`

3. 解析`=`號右方,歷遍Syntax Tree將leaf加進`TREF`,若出現`&`時將並變數和步驟2求出的`TGEN`放入`TEQUIV`,放入時會跟新`TEQUIV`確認是否有新的pair產生或移除

4. 根據`TEQUIV`將alias的變數放入`TGEN`和`TREF`

5. 將`TDEF`分別與`TGEN`和`TREF`比對,算出Flow/Output Dependency

6. 跟新`TDEF`,並印出結果

7. 家輝妹妹生日快樂
