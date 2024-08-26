# 一个抓包工具.

> 我本来想写个阿尔比恩的雷达的.
> 
> 现在他是一个UDP抓包并通过TCP转发的工具.

## 用法


### 一步到位(以阿尔比恩```-pName Albion-Online.exe```举例),仅尝试于```powershell.exe```,```cmd.exe```能否执行这个命令我并不知道

> * 通过服务RCV的地址 ```$env:PROCESS_ADDRESS_USING="$(.\filterGetter.exe -pName Albion-Online.exe)";echo "filterGetter.exe的输出:$env:PROCESS_ADDRESS_USING";.\transport.exe -addresses "$env:PROCESS_ADDRESS_USING"```
>
>> 如果是服务端,应该使用这种方式


> * 通过服务SND的地址 ```$env:PROCESS_ADDRESS_USING="$(.\filterGetterForRemoteAddr.exe -pName Albion-Online.exe)";echo "filterGetter.exe的输出:$env:PROCESS_ADDRESS_USING";.\transport.exe -addresses "$env:PROCESS_ADDRESS_USING"```
>
>> 如果是客户端 应该使用这种方式

### 分别执行

>+ ```filterGetter.exe```是一个前置软件,他可以搞清楚```-pName```都听哪些UDP的地址


>+ ```filterGetterForRemoteAddr.exe```是一个前置软件,他可以搞清楚```-pName```都往哪些地址发包


>+ ```transport.exe```是转发软件会按```-addresses```进行一个过滤,对满足关键字```-deviceKeywords```的网卡设备开始抓包,然后自己听0.0.0.0:```-port```的TCP,只要有人连接,就会按下表的消息格式发送消息
>
>> ```-addresses```没有默认值
> 
>> ```-deviceKeywords```的默认值是```Netease```,因为我用网易UU,会虚拟一个网卡```Netease UU TAP-Win32 Adapter V9.21```
>
>> ```-port```的默认值是```32999```

| Index | 0,1                    | 2,3                                             | ...  |   |
|-------|------------------------|-------------------------------------------------|------|---|
| Means | ```Uint16```,**小端序**,数据长度 | ```Uint16```,**小端序**,```255```表示对应进程发出的包,```0```则是收到 | 数据 |   |


## 关于加速器

我使用的网易UU可以改成路由模式,如果是进程模式抓到的包会被加密.