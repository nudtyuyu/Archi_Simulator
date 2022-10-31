# RiscV_Simulator

### 文件说明

- ```src```目录：模拟器源代码即测试所需的文件。
- ```test```目录：测试程序的c语言代码及对应的RISC-V汇编代码。

### 编译和运行

进入```src```目录，

编译：```g++ startup.cpp -o startup.exe```
运行：```.\startup.exe```

应当看到如下所示的结果：

`fib[10] =55`

`1 to exit, 2 to continue:`


其中的交互为```ebreak```指令将控制返回上一级，这里用来模拟程序结束。如果输入```2```，应当得到一条```Invalid Instruction!```报错，原因为程序执行完最后一条指令后读到全零指令。由于输出中文存在编码问题，故测试程序输出为英文。
