# InVideo
## 工具介绍
1、基于UE5实现的rtsp的视频播放插件  
2、基于UE5实现的运行时输出mp4文件

# 使用示例
## 1、视频播放示例
参考插件工程InVideo/Content/NewWorld.umap  

播放效果如下：  
![demo](./Images/demo.jpg) 


### 使用方法
新建一个蓝图widget继承自InVideoWgidget，并新建一个Image对象，重新命名为ImageVideo。  
即可进行调用，可以参考videoUmg组件。  

打开视频方法    
![startplay](./Images/startplay.jpg) 

关闭视频方法    
![stopplay](./Images/stopplay.jpg) 

## 2、运行时输出mp4
第一步设置默认viewportclient  
![viewportclient](./Images/ViewportClient.jpg) 
开始录像示例  
![outputmp4](./Images/outputmp4.jpg) 

