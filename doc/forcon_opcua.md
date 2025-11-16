
## OPC UA 服务端说明(v0.0.1)

### 一、连接
* 格式: opc.tcp://ip:5650
* 加密：none
* 身份: Anonymous

### 二、变量节点信息
    > 所有变量节点都挂载于 /Objects/Forcon 下
    > 前缀为 "S-"的变量，为服务器控制的变量，客户端一般只能读取
    > 前缀为 "C-"的变量，为客户端控制的变量，客户端拥有写权力，一般是对服务器下达指令
1. **S-ProductName**
    * 类型: String
    * 访问：读
    * 说明：当前加工的产品名称
2. **S-RunningStatus**
    * 类型：String
    * 访问：读
    * 说明：当前设备运行状态，可能的值如下
        * running 运行中
        * idle 空闲
        * stoped 停止
        * fault 错误
3. **C-RequestProductChange**
    * 类型：String
    * 访问：读/写
    * 说明：对产品切换进行预约，预约成功后，要等操作员到设备端进行人工确认
    * 注意：请不要持续写入，一次性写入即可
4. **S-ResponseProductChange**
    * 类型：String
    * 访问：读
    * 说明：设备对产品预约切换的回应(此时服务器会将C-ProductChangeRequest的值置为 none)，可能的值如下
        * accepted 已接受，请求有效
        * pending 待处理，请求已接受，如果当前尚有产品加工未完成，或者等待人工确认的时候，会显示此状态
        * rejected 已拒绝：请求不被允许，将不被执行
        * none 无回应，此状态才可以接受客户端的预约请求
    * 注意：值为accepted/pending时，则表示当前正在处理预约，不会响应新的预约
5. **S-ProductionStartTime**
    * 类型：DateTime
    * 访问：读
    * 说明：当前产品的加工开始时间
6. **S-ProductionProgress**
    * 类型: Float
    * 访问：读
    * 说明：当前产品的加工进度，值范围是0 到 1（1代表100%）
7. **C-RequestStartProduction**
    * 类型：Bool
    * 访问：读/写
    * 说明：当状态置为 true时，则意味着对设备发起开始加工请求，如果设备在条件允许的情况下，会开始执行加工流程
    * 注意：请不要持续写入，一次性写入即可
8. 。。待续
