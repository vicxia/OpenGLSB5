# OpenGL超级宝典第五版代码联系工程
>学习OpenGL过程中，遇到了一些sb5的源码在OSX上，基于Xcode9无法正确运行的问题，经过调试，目前可以正确运行第六章代码+shader。

主要解决的问题如下：
* GLTools的源码集成到练习的Demo工程
* 可以兼容运行版本为410的Shader
* GLTriangleBatch等可以与Shader协调工作
