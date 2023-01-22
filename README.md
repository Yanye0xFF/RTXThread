## 文档链接
https://www.cnblogs.com/yanye0xff/p/17034882.html

### 编译方法
    + 方法1：进入RTXThread/Release目录，打开终端，输入make
    + 方法2：下载Eclipse IDE for Embedded C/C++ Developers，File -> inport导入工程，Project -> Build Project执行编译
### 2023-01-04
    + SPI驱动重新支持SPI0，2线模式，仅支持DMA模式(用户态buffer)8bit
### 2022-10-13
    + 移除工程对KEIL-MDK的支持
### 2022-10-12
    + SPI驱动移除对SPI0的支持，SPI2仅使用2线模式，仅支持DMA模式(用户态buffer)，支持8/16bit`./src/bsp/driver/drv_spi.c`
### 2022-10-11
    + 内核升级RT-Thread v4.1.1，finsh组件升级
    + 外设库升级GD32F30x_Firmware _Library_V2.1.4