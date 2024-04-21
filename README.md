# 原版暗黑2一键施法

我使用的是D2GL，在配置文件中添加d2fastspell.dll，如果不用D2GL，大箱子插件和MH插件都可以实现

使用的特征码，V1.14d确认没问题，其他版本没试过不知道，自行测试

原理就是在你使用技能快捷键的时候自动帮你在鼠标位置按一下鼠标右键，当然，你要把右键施法改掉的话肯定就没效果了

```conf
; Comma-delimited DLLs to load (early: right after attached).
load_dlls_early=d2fps.dll:stdcall:_Init@0,d2fastspell.dll
;d2fps.dll:stdcall:_Init@0是他默认的运动补帧，你只需要把本dll放在逗号后面就可以了
```