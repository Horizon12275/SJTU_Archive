**一、正确性测试：**

- make clean

- make grade，能获得正确性测试结果

**二、研究参数 M 的影响**

- 修改 parameter.hpp 文件中 M 以及 M_max 的值（M=M_max=10,20,30,40,50）

- make clean

- make grade，获得不同参数下串行查询时延。

**三、性能测试**

- make clean

- make test

- 修改 parameter.hpp 文件中 M 和 M_max 的值，来获得不同参数下的实验结果

- 输出结果为插入时间和并行查询时间（串行查询时间在先前得到）

- 使用线程池的并行优化版本在根目录下的 threadpool_test.cpp 中、因本次 lab 未要求嵌入 test.cpp，因此未修改 makefile，可通过将代码全部复制到 test.cpp 后、执行 make clean 与 make test 指令进行线程池版本的数据进行运行
