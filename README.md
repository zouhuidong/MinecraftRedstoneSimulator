# MinecraftRedstoneSimulator
![Downloads](https://img.shields.io/github/downloads/zouhuidong/MinecraftRedstoneSimulator/total)
![GitHub Version](https://img.shields.io/github/v/release/zouhuidong/MinecraftRedstoneSimulator)
[![Blog](https://img.shields.io/badge/blog-huidong.xyz-green.svg)](http://huidong.xyz)
[![EasyX](https://img.shields.io/badge/graphics-EasyX-orange.svg)](https://easyx.cn)

使用 EasyX 制作的平面红石电路模拟器

## 前言

> 相信 MC 红石电路的强大是人尽皆知的。在 MC 中，你甚至可以用红石电路造出 CPU，实现计算机。
>
> 此程序模拟 MC 红石运行环境，支持平面红石电路设计和仿真运行，可以实现许多复杂的逻辑电路。

## 编译环境

Windows 10 | Visual Studio 2022 | EasyX 20220610

依赖库 [HiEasyX](https://www.github.com/zouhuidong/HiEasyX)

## 模拟效果

下面将由简到繁，展示此模拟器实现的电路效果

<div align=center>
<img src="./screenshots/开始.png"><br>
<b>开始界面</b>
</div><br>

<div align=center>
<img src="./screenshots/非门.gif"><br>
<b>非门</b>
</div><br>

<div align=center>
<img src="./screenshots/与门.gif"><br>
<b>与门</b>
</div><br>

<div align=center>
<img src="./screenshots/搭建与门.gif"><br>
<b>搭建与门</b>
</div><br>

<div align=center>
<img src="./screenshots/异或门.gif"><br>
<b>异或门</b>
</div><br>

<div align=center>
<img src="./screenshots/重设地图大小.gif"><br>
<b>重设地图大小</b>
</div><br>

<div align=center>
<img src="./screenshots/点阵灯.gif"><br>
<b>点阵灯</b>
</div><br>

<div align=center>
<img src="./screenshots/寄存器.gif"><br>
<b>寄存器</b>
</div><br>

<div align=center>
<img src="./screenshots/加法器.gif"><br>
<b>加法器</b>
</div><br>

更复杂的电路还在等待您的实现~

## 注意事项

1. 中继器仅用于单向导电，不支持其它特性
2. 红石火把必须用中继器充能才会熄灭
3. 目前导入项目必须在终端使用 `import` 指令，详情可在终端输入 `help` 查看。

