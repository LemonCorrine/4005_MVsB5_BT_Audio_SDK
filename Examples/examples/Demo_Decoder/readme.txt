Demo Decoder工程主要演示了解码器的工作流程。
由三部分组成：FatFs文件系统，decoder，dac

硬件环境要求:
    1. BP15xx系列开发板。
    2. 外接串口小板，TX/RX/GND接至RX(A9)/TX(A10)/GND  (波特率 2000000)；
    3. 需要有SD卡接口，或USB接口以便插入U盘。

软件使用说明:	
在SD卡或U盘上存入适量歌曲(mp3，flac，wav，m4a等常用格式)，插入开发板后，开机
测试程序会轮流播放这些歌曲
查看代码可以获知DAC的配置，及解码器decoder的使用流程
	