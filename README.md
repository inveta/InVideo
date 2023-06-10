# InVideo

## 工具介绍

1、基于 UE5 实现的 rtsp 的视频播放插件  
2、基于 UE5 实现的运行时输出 mp4 文件

# 使用示例

## 1、视频播放示例

参考插件工程 InVideo/Content/NewWorld.umap

播放效果如下：  
![demo](./Images/demo.jpg)

### 使用方法

新建一个蓝图 widget 继承自 InVideoWgidget，并新建一个 Image 对象，重新命名为 ImageVideo。  
即可进行调用，可以参考 videoUmg 组件。

打开视频方法  
![startplay](./Images/startplay.jpg)

关闭视频方法  
![stopplay](./Images/stopplay.jpg)

## 2、运行时输出 mp4

第一步设置默认 viewportclient  
![viewportclient](./Images/ViewportClient.jpg)
开始录像示例  
![outputmp4](./Images/outputmp4.jpg)

# 更新记录

1、2023-05-23 解决了在 5.2 版本崩溃的 bug  
2、2023-06-11 重大更新，实现视频打开和关闭全异步化，不会再阻塞蓝图线程
