# Sandnix 0.0.2 架构,模块及接口
***
[TOC]
***
## A. 文档说明
该文档在GPL v3协议下发布.
***
## B. 整体架构及代码风格
### 整体架构
#### 设计原则
* 尽量避免在内核实现驱动程序,初始化用的ramdisk除外.
* 要严格遵守代码风格来保证代码有较好的可读性.
* 不要用全局变量来传参.模块内部使用的变量要加static.
* HAL层以上的代码(不包括HAL层)要尽量使用面向对象的方式来编写.

#### 项目架构
![项目架构图](http://7xsd89.com1.z0.glb.clouddn.com/project-arch.png)
##### HAL层
该层的作用是将管理cpu,mmu,中断以及内核传参等实现os基本功能所必须打交道的在不同架构下存在显著差异部分剥离出来,为工作在它上面的core层,和subsystem层提供一个与硬件环境无关的统一接口.模块列表如下:
* init
* mmu
* early\_print
* kparam
* cpu
* io
* sys\_gate
* power
* exception

##### core层
该层是内核的核心部分,大部分模块都位于此处.主要以面向对象的方式编写.模块列表如下:
* main
* interrupt
* mm
* kconsole
* pm
* ipc
* msg
* vfs
* rtl
* exception

##### subsystem层
负责提供系统调用,并将系统调用翻译成对内核函数的调用.模块列表如下:
* lib
* driver
* linux

#### 代码风格要求(参照Linux kernel风格改编)
##### 缩进
缩进为4个空格,8个空格会使代码很靠右,尽量用空格而不是tab(这一点利用astyle可以很轻松的搞定)..因为gihub上面tab看起来相当的糟糕.switch语句里面的case应当缩进来使得代码层次清晰.
不要一行放多个语句.
由于标准的Linux终端是24行80列的,接近或大于80个字符的较长语句要折行写,折行后用空格和上面的表达式或参数对齐.
较长的字符串可以断成多个字符串然后分行书写,例如:
```c
printf("Thisis such a long sentence that " 
"it cannot be held within a line\n");
```
C编译器会自动把相邻的多个字符串接在一起,以上两个字符串相当于一个字符串"This is such a long sentence that it cannot be held within a line\n".
汇编代码里边函数名要顶格写,循环,分支等结构应当增加缩进以便阅读.
##### 空白
关键字if,while,,for与其后的控制表达式的(括号之间插入一个空格分隔,但括号内的表达式应紧贴括号.
双目运算符的两侧插入一个空格分隔,单目运算符和操作数之间不加空格.
后缀运算符和操作数之间也不加空格,例如取结构体成员`s.a,p_s->a`,函数调用`foo(arg1)`、取数组成员`a[i].`
,号和;号之后要加空格,这是英文的书写习惯,例如`for(i=1; i<10; i++), foo(arg1,arg2),`
##### 括号的位置
C语言风格中另外一个常见问题是大括号的放置。和缩进大小不同，选择或弃用某种放置策略并没有多少技术上的原因，不过首选的方式，就像Kernighan和Ritchie展示给我们的，是把起始大括号放在行尾，而把结束大括号放在行首.像这样:
```c
if (xxx) {
	xxxxxx;
} else {
	xxx;
}
```
如果小括号,中括号要分行写,也遵从上述规则.
不过,有一种特殊情况,命名函数.它们的起始大括号放置于下一行的开头,像这样：
```c
int func(int x)
{
	xxxx
}
```
##### 命名规则
请使用小写字母,数字和下划线给函数,变量以及数据结构命名,大写字母是留给宏使用的.
原则上宏应尽量使用大写字母命名,但是函数式宏定义除外,他们和函数遵守一样的命名规则.
函数,参数,变量的名字要能体现出它的作用.在不破坏可读性的前提条件下尽量使用缩写.
模块导出函数的命名应遵从以下格式:
	`层名_模块名_函数名`
函数名应尽量对每个单词使用缩写,要能描述出这个函数的作用是什么.构造函数是个例外.
结构体的定义遵守以下规范:
```c
typedef struct _结构体名 {
	成员表;
} 结构体名, *p结构体名;
```
一般的,结构体名,type定义的类型名要以\_t结尾,如果结构体为一个类的定义的话要以\_obj\_t结尾.u8,u16,u32这类出于跨平台兼容为目的考虑的对基本类型的重定义除外.
函数内部的标号要用下划线(\_)开头.
##### 面向对象
一个类名应当以\_obj结尾,类的声明方式如下:
```c
typedef struct _类名_t {
	父类对象;
	指向成员函数的指针列表;
	成员变量;
} 类名_t , *p类名_t ;
```
特别的,obj\_t是一切类的最终父类.
类的构造函数名遵从以下结构:
	`类名_后缀`
后缀是可选的,当存在需要重载构造函数的情况时后缀用于区别这些函数.
构造函数的职责包括为新对象分配内存,初始化成员变量并将成员函数的地址填到列表中.
引用和释放一个对象的时候别忘了增减引用计数.
##### 注释
注释的职责是让人知道代码是干啥的,而不是告诉别人代码是怎么干的.除非你写了一大堆烂代码.
如果一个函数/宏的调用有什么注意事项,或者是需要解释点烂七八糟协议之类的什么东西,一定要用注释写出来.
用`//TODO:`的形式标出未做的工作是个不错的主意,正常点的编辑器都会把它高亮的.
##### Astyle参数
```bash
astyle --suffix=none --style=linux --indent=spaces=4 --attach-namespaces --attach-classes --attach-inlines --attach-extern-c --indent-classes --indent-cases --indent-preproc-cond –indent-switches --indent-preproc-define --indent-preproc-block --indent-col1-comments --break-blocks=all --pad-oper --unpad-paren --mode=c $filename
```

* * *
###配置工具Skconfig
该工具由python编写,作用是为内核的配置和编译提供一个直观有效的配置工具,类似于linux的kconfig.
内核项目比较特殊,很多配置工具都是量身定做的,想通用很难.如果采用autotools,编译的时候那选项够你爽的.如果采用kconfig,linux关于模块的处理方式和本项目差异又很大,根本没法通用,至于自己写makefile,那工作量和维护难度我只能说是呵呵呵呵了,所以说还是要自己动手丰衣足食.
####如何管理项目的编译
由于该项目在编译的时候有多种架构,我把一个项目分成一下两个元素:
* 目标(target)
* 架构(arch)

两者都是可以嵌套的,架构嵌套架构,目标嵌套目标.每个target都会包含一个或者多个arch.工具在扫描配置文件的时候会根据arch里的信息生成编译用的命令模板,然后通过读target里的选项生成编译选项,最后生成编译用的命令.父target必须包含子target的所有arch.子target里有arch的话子target的arch会覆盖父target的arch,否则继承父target的arch.特别的,父target依赖于子target.
一个target的构建分为一下几个步骤:
* 构建依赖target
* 构建子target
* 执行编译前命令(pre-compile)
* 生成依赖(dep)
* 编译(compile)
* 链接(link)
* 执行编译后命令(after-compile)

每个target由一个target.xml描述,该文件放在每个target的根目录下.同时位于该目录下的还有sources,sources.$(arch)文件,用于记录需要编译的源文件路径,其中sources.$(arch)文件会优先被扫描.
在项目的根目录下执行./skconfig打开配置界面,./configuren生成makefile.每一个target在扫描和编译的时候当前目录都是target所在的目录.
sources文件内部可以用"#"来注释,"#"后面的内容将会被忽略.
target.xml文件结构如下:
```xml
<!--target标签的name属性是目标的名字,type为build或virtual-->
<?xml version="1.0" encoding="utf-8"?><target name="" type="">
    <!--输出文件名-->
    <output name="输出文件名" />
    <!--输出目录-->
    <outdir path="输出目录" />
    <!--中间文件目录-->
    <middir path="中间文件目录" />
    <!--说明-->
    <introduction>sandnix</introduction>
    <!--架构列表-->
    <archs actived="x86">
        <arch name="x86">
            <!--所有的命令都会被原样写到gnu makefile中,都对应着同名变量-->
            <!--没写的架构会继承父项目的-->
            <PREV>这里是编译前的命令</PREV>
            <DEP>生成依赖的工具</DEP>
            <DEPRULE>生成依赖的命令</DEPRULE>
            <CC>c编译器</CC>
            <CFLAGS>c编译选项</CFLAGS>
            <CCRULE>c编译命令</CCRULE>
            <AS>汇编编译器</AS>
            <ASFLAGS>汇编编译选项</ASFLAGS>
            <ASRULE>汇编编译命令</ASRULE>
            <LD>链接器</LD>
            <LDFLAGS>链接选项</LDFLAGS>
            <LDRULE>链接命令</LDRULE>
            <AFTER>编译后执行的命令</AFTER>
        </arch>
        <!--用"."表示层次关系-->
        <arch name="arm.raseberry">
            <PREV>这里是编译前的命令</PREV>
            <DEP>生成依赖的工具</DEP>
            <DEPRULE>生成依赖的命令</DEPRULE>
            <CC>c编译器</CC>
            <CFLAGS>c编译选项</CFLAGS>
            <CCRULE>c编译命令</CCRULE>
            <AS>汇编编译器</AS>
            <ASFLAGS>汇编编译选项</ASFLAGS>
            <ASRULE>汇编编译命令</ASRULE>
            <LD>链接器</LD>
            <LDFLAGS>链接选项</LDFLAGS>
            <LDRULE>链接命令</LDRULE>
            <AFTER>编译后执行的命令</AFTER>
        </arch>
        <!--其他的架构在这里-->
    </archs>
    <dependencies>
        <!--依赖的target的列表,填当前target目录的-->
        <dep path="sub target path" />
    </dependencies>
    <sub-targets>
        <!--子target列表-->
        <target enable="true" path="" />
    </sub-targets>
    <options>
        <!--单选框-->
        <option type="checkbox" name="Enable Debigging" value="-g" enable="true" target="CFLAGS|ASFLAGS|LDFLAGS" />
        <!--多选一-->
        <option type="list" name="Bootloader" selected="0" target="CFLAGS|ASFLAGS|LDFLAGS">
            <item name="grub2" value="-DGRUB" />
            <item name="uboot" value="-DUBOOT" />
        </option>
        <!--单行文本框-->
        <option type="input" name="Comment" marco="-DCOMMENT" value="" target="CFLAGS|ASFLAGS|LDFLAGS" />
        <!--菜单-->
        <option type="menu" name="Early print options">
            <!--这里是在该子菜单中的选项-->
        </option>
    </options>
</target>
```

###公共基本类型
代码位于src/common下,提供一些基本的宏及类型定义.
####基本数据类型
```c
//无符号8位数
u8

//无符号16位数
u16

//无符号32位数
u32

//无符号64位数
u64

//有符号8位数
s8

//有符号16位数
s16

//有符号32位数
s32

//有符号64位数
s64

//无符号大小
size_t

//无符号大小
ssize_t

//布尔类型
bool

//地址,该类型通常在对地址进行数学运算时使用
address_t

```
###可引导映像头
代码位于src/sandnix/header下,包括定义映像头的数据结构的.S文件以及相应的生成脚本.
###内核主体
代码位于src/sandnix/kernel下,为内核主体部分代码所在位置.
####HAL层
HAL层负责将不同架构的硬件如CPU,中断控制器,MMU等抽象出统一的接口.该层包括以下模块:
#####init
该模块为整个内核的入口,负责准备内核运行的基本环境,调用其他模块的初始化接口,并在这之后将控制权转移到Core层.
######模块路径
```c
src/sandnix/kernel/hal/init
```
######接口数据结构
```c
//记录bootloader传来的物理内存信息,架构相关.
boot_mem_map_t;

//负责记录初始化内存盘的位置,physical memory map,启动参数和内核代码段的地址.
kernel_header_t:
```
######接口函数及宏:
```c
//初始化
//整个内核的入口函数.
void _start():

//返回bootloader传来的memory map,格式为physical_memory_info_t,类型定义在mmu模块中.
list_t hal_init_get_boot_memory_map():

//返回初始化内存盘的位置和大小.
void hal_init_get_initrd_addr(void** p_addr, size_t* p_size):

//获得内核的命令行参数.
char* hal_init_get_kernel_cmdline():
```
######文件列表
```c
//c运行环境初始化代码
src/sandnix/kernel/hal/init/arch/$(arch)/init.s

//kernel_header初始化代码
src/sandnix/kernel/hal/init/arch/$(arch)/header.h
src/sandnix/kernel/hal/init/arch/$(arch)/header.c

//HAL层初始化代码
src/sandnix/kernel/hal/init/init.c

//接口头文件
src/sandnix/kernel/hal/init/init.h
```
#####kparam
负责解析内核参数.
######模块路径
```
src/sandnix/kernel/hal/kparam
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化
//初始化模块,解析内核参数
void hal_kparam_init();

//获得内核参数
bool hal_kparam_get_value(char* key, char* buf, size_t size);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/kparam/kparam.h

//参数解析代码
src/sandnix/kernel/hal/kparam/kparam.c
```
#####mmu
负责初始化分页机制,管理mmu以及提供分页管理的接口.
######模块路径
```
src/sandnix/kernel/hal/mmu
```
######接口数据结构
```c
//物理内存信息
physical_memory_info_t
```
######接口函数及宏
```c
//页面大小
#define	SANDNIX_KERNEL_PAGE_SIZE	4096

//初始化
//启动分页
void start_paging(u32 cpuid);

//初始化mmu模块
void hal_mmu_init();

//初始化处理器核心
void hal_mmu_core_init(int cpuid);

//物理内存管理
//申请物理内存
bool hal_mmu_phymem_alloc(void** p_addr, size_t page_num);

//释放物理内存
void hal_mmu_phymem_free(void* addr, size_t page_num);

//获得物理内存信息,返回所需内存大小
size_t hal_mmu_get_phymem_info(pphysical_memory_info_t p_buf, size_t size);

//分页管理
//获得内核地址范围
void hal_mmu_get_krnl_addr_rgn(void** p_base, size_t* p_size);

//获得用户地址范围
void hal_mmu_get_usr_addr_rgn(void** p_base, size_t* p_size);

//创建页表
bool hal_mmu_pg_tbl_create(u32* page_id);

//销毁页表
void hal_mmu_pg_tbl_destroy(u32* page_id);

//设置页表条目,krnl_pg_tbl_t定义在mm模块中
void hal_mmu_pg_tbl_set(void* virt_addr, u32 num, pkrnl_pg_tbl_t page_tables);

//刷新页表cache
void hal_mmu_pg_tbl_refresh();

//切换到指定页表
void hal_mmu_pg_tbl_switch(u32 id);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/mmu/mmu.h

//具体架构页表操作
src/sandnix/kernel/hal/mmu/paging/paging.h
src/sandnix/kernel/hal/mmu/paging/arch/$(arch)/page_table.h
src/sandnix/kernel/hal/mmu/paging/arch/$(arch)/paging.c

//具体架构物理内存操作
src/sandnix/kernel/hal/mmu/phymem/phymem.h
src/sandnix/kernel/hal/mmu/phymem/arch/$(arch)/phymem.c
```
#####early\_print
负责在tty设备无法使用的情况下内核信息的输出.
######模块路径
```
src/sandnix/kernel/hal/early_print
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化
//初始化临时终端
void hal_early_print_init();

//设置临时终端颜色
void hal_early_print_color(u32 fg, u32 bg);

//打印字符串
void hal_early_print(char* str);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/early_print/early_print.h

//临时终端实现代码
src/sandnix/kernel/hal/early_print/arch/$(arch)/early_print.c
```
#####io
负责中断的管理,虚拟IRQ的映射,系统时钟的管理.
######模块路径
```
src/sandnix/kernel/hal/io
```
######接口数据结构
```c
//中断处理回调
int_callback_t
```
######接口函数及宏
```c
//初始化
//初始化模块
void hal_io_init()

//初始化处理器核心
void hal_io_core_init(u32 cpuid);

//中断管理
//禁止当前核心中断
void hal_io_int_disable();

//允许当前核心中断
void hal_io_int_enable();

//允许所有IRQ
void hal_io_irq_enable_all();

//禁止所有IRQ
void hal_io_irq_disable_all();

//允许IRQ
void hal_io_irq_enable(u32 num);

//禁止IRQ
void hal_io_irq_disable(u32 num);

//注册/取消中断回调
void* hal_io_int_callback_reg(u32 num, int_callback_t callback);

//注册时钟回调
void* hal_io_clock_callback_reg(u32 num, int_callback_t callback);

//IO管理
//IN
u8 hal_io_in_8(address_t port);
u16 hal_io_in_16(address_t port);
u32 hal_io_in_32(address_t port);
u64 hal_io_in_64(address_t port);

//INS
void hal_io_ins_8(void* dest, size_t count, address_t port);
void hal_io_ins_16(void* dest, size_t count, address_t port);
void hal_io_ins_32(void* dest, size_t count, address_t port);
void hal_io_ins_64(void* dest, size_t count, address_t port);

//OUT
void hal_io_out_8(address_t port, u8 data);
void hal_io_out_16(address_t port, u8 data);
void hal_io_out_32(address_t port, u8 data);
void hal_io_out_64(address_t port, u8 data);

//OUTS
void hal_io_outs_8(address_t port, size_t count, void* src);
void hal_io_outs_16(address_t port, size_t count, void* src);
void hal_io_outs_32(address_t port, size_t count, void* src);
void hal_io_outs_64(address_t port, size_t count, void* src);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/io/io.h

//中断
src/sandnix/kernel/hal/io/interrupt/$(arch)/

//IRQ
src/sandnix/kernel/hal/io/irq/$(arch)/
```
#####cpu
负责管理cpu的状态,以及保护线程上下文.
######模块路径
```
src/sandnix/kernel/hal/cpu
```
######接口数据结构
```c
//线程上下文
context_t

//cpu信息
cpuinfo_t
```
######接口函数及宏
```c
//初始化
//初始化cpu模块
void hal_cpu_init();

//线程上下文管理
//保存线程上下文
#define hal_cpu_context_save(p_context)

//加载线程上下文
#define hal_cpu_context_load(p_context)

//多核心管理
//停止当前核心
void hal_cpu_core_halt();

//cpu信息
void hal_cpu_get_info(pcpuinfo_t p_ret);

//性能管理
//读取cpu频率
u32 hal_cpu_get_frequency();

//设置cpu频率
u32 hal_cpu_set_frequency();
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/cpu/cpu.h

//上下文
src/sandnix/kernel/hal/cpu/context/$(arch)

//cpu信息
src/sandnix/kernel/hal/cpu/cpu-info/$(arch)
```
#####exception
负责处理kernel panic以及调用错误处理程序.
######模块路径
```
src/sandnix/kernel/hal/exception
```
######接口数据结构
```c
//void mem_fault_hndlr(u32 thread_id, address_t address, u32 operation)
typedef void	(*mem_fault_hndlr)(u32, address_t, u32);

//void privilege_fault_hndlr(u32 thread_id, address_t address)
typedef void	(*privilege_fault_hndlr)(u32, address_t);

//void undef_op_fault_hndlr(u32 thread_id, address_t address
typedef void	(*undef_op_fault_hndlr)(u32, address_t;

//错误处理程序链表
hndlr_info;
```
######接口函数及宏
```c
//初始化模块
void hal_exception_init();

//初始化处理器核心
void hal_exception_core_init();

//报错并终止整个系统
void hal_exception_panic(u32 error_code, char* fmt ,...);

//添加错误处理程序
//内存错误
void hal_exception_mem_fault_hndlr_set(
	phndlr_info p_hndlr_info);

//越权操作
void hal_exception_privilege_fault_hndlr_set(
	phndlr_info p_hndlr_info);

//未定义指令
void hal_exception_undef_op_fault_hndlr_set(
	phndlr_info p_hndlr_info);

//Assert
#define ASSERT(x)
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/exception/exception.h

//错误处理
src/sandnix/kernel/hal/exception/%(arch)/exception.c

//Panic
src/sandnix/kernel/hal/exception/%(arch)/panic.c
```
#####power
负责处理基本的电源操作,比如断电,复位,睡眠等.
######模块路径
```
src/sandnix/kernel/hal/power/
```
######接口数据结构
```c

```
######接口函数及宏
```c
//h初始化模块
void hal_power_init();

//初始化处理器核心
void hal_power_core_init();

//断电
void hal_power_off();

//睡眠
void hal_power_sleep();

//复位
void hal_power_reset();
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/power/power.h

//实现代码
src/sandnix/kernel/hal/power/$(arch)/power.c
```
#####sys\_gate
负责提供用户态和内核态之间的切换.
######模块路径
```
src/sandnix/kernel/hal/sys_gate/
```
######接口数据结构
```c

```
######接口函数及宏
```c
//初始化模块
void hal_sys_gate_init();

//初始化cpu核心
void hal_sys_gate_core_init();

//设置入口地址
void hal_sys_gate_set_entry(void* entry);

//返回到用户空间
void hal_sys_gate_ret(pcontext_t p_context)

//转移到用户空间
void hal_sys_gate_go_to_usr(void* entry, int argc, char* argv[]);
```
######文件列表
```c
//接口头文件
src/sandnix/kernel/hal/sys_gate/sys_gate.h

//实现文件
src/sandnix/kernel/hal/sys_gate/$(arch)/sys_gate.c
```
####Core层
#####main
Core层的入口
######模块路径
```
src/sandnix/kernel/core/main
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####interrupt
中断管理
######模块路径
```
src/sandnix/kernel/core/interrupt
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####mm
内存管理,包括页面的分配,映射,释放,物理内存的分配,释放,堆管理等
######模块路径
```
src/sandnix/kernel/core/mm
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####kconsole
内核控制台
######模块路径
```
src/sandnix/kernel/core/kconsole
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####pm
进程管理
######模块路径
```
src/sandnix/kernel/core/pm
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####ipc
POSIX进程通信
######模块路径
```
src/sandnix/kernel/core/ipc
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####msg
内核消息机制
######模块路径
```
src/sandnix/kernel/core/msg
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####vfs
虚拟文件系统以及初始化内存盘的驱动
######模块路径
```
src/sandnix/kernel/core/vfs
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####rtl
c运行库,面向对象以及各种数据结构
######模块路径
```
src/sandnix/kernel/core/rtl
```
######接口数据结构
```c
//可变参
va_list

//面向对象
//所有对象的祖宗
obj_t

//字符串对象
kstring_obj_t

//数据结构
//链表
list_t

//线性表
linear_list_t

//哈希表
hash_table_t

//映射
map_t

//向量
vector_t

//队列
queue_t

//缓冲区
buffer_t

//回调函数类型
//销毁对象
//void item_destroyer(void* p_item, void* p_arg)
typedef void (*item_destroyer_t)(void*, void*);

//比较大小.item1 > item2返回值 > 0
//等于返回0
//小于返回值 < 0
//int item_compare(void* p_item1, void* p_item2);
typedef int (*item_compare_t)(void*, void*);

//哈希函数
//u32 hash_func(void* p_item)
typedef u32 (*hash_func_t)(void*)
```
######接口函数及宏
```c
//可变参
#define va_start
#define va_arg
#define va_end

//c运行库
//字符串处理函数,作用参照标准库
void* core_rtl_memccpy(void* dest, const void* src, u8 ch, size_t size);
void* core_rtl_memchr(const void* buf, u8 ch, size_t size);
int core_rtl_memcmp(const void* buf1, const void* buf2, size_t size);
void* core_rtl_memcpy(void* dest, const void* src, size_t size);
void* core_rtl_memmove(void* dest, const void* src, size_t size);
void* core_rtl_memset(void* dest, u8 value, size_t size);
char* core_rtl_strchr(const char* str, char c);
size_t core_rtl_strcspn(const char* str, const char* reject);
size_t core_rtl_strlen(const char* str);
char* core_rtl_strncat(char *dest, const char *src, size_t len);
int core_rtl_strncmp(const char* dest, const char* src, size_t len);
char* core_rtl_strncpy(char* dest, const char* src, size_t len);
char* core_rtl_strpbrk(const char* str1, const char* str2);
char* core_rtl_strrchr(const char* str, char ch);
size_t core_rtl_strspn(const char* str, const char* accept);
char* core_rtl_strstr(const char* str1, const char* str2);

//分割字符串
char* core_rtl_strsplit(const char *str, const char *delim, char* buf, size_t size);

//格式化字符串函数
char* core_rtl_snprintf(char* buf, size_t size, const char* fmt, ...);
char* core_rtl_vsnprintf(char* buf, size_t size, const char* fmt, va_list ap);

//格式化输出函数
char* core_rtl_kprintf(const char* fmt, ...);

//数据结构
//链表
#define core_rtl_list_init(list)
#define core_rtl_list_empty(list)
core_rtl_list_insert_before
core_rtl_list_insert_after
core_rtl_list_remove
core_rtl_list_destroy
core_rtl_list_join
core_rtl_list_next
core_rtl_list_qsort

//线性表
core_rtl_linear_list_init
core_rtl_linear_list_get
//设成NULL即remove
core_rtl_linear_list_set
core_rtl_linear_list_used
core_rtl_linear_list_size
core_rtl_linear_list_get_current_max_index
core_rtl_linear_list_get_free_index
core_rtl_linear_list_get_free_index_num
core_rtl_linear_list_destroy

//哈希表
core_rtl_hash_table_init
core_rtl_hash_table_get()

//设成NULL即remove
core_rtl_hash_table_set()
core_rtl_hash_table_destroy()

//面向对象
//obj_t

//kstring_obj_t

```
######文件列表
```c

```
#####exception
异常处理
######模块路径
```
src/sandnix/kernel/core/exception
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
####Subsystem层
#####lib
subsystem层的运行库,包括跨权限缓冲区处理等调用
######模块路径
```
src/sandnix/kernel/subsystem/lib
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####driver
为驱动程序提供系统调用的子系统
######模块路径
```
src/sandnix/kernel/subsystem/driver
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####linux
为linux应用提供系统调用的子系统
######模块路径
```
src/sandnix/kernel/subsystem/linux
```
######接口数据结构
```c

```
######接口函数及宏
```c

```
######文件列表
```c

```
#####驱动程序
位于src/drivers下.