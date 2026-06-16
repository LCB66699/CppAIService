# C++ AI 应用服务平台

基于自研 C++ HTTP 框架构建的 AI 应用服务平台，支持多模型对话、RAG 检索增强、轻量级 MCP 工具调用、ASR/TTS 语音、图像识别、消息队列异步化及多租户会话管理。

## 架构

```
┌─────────────────────────────────────────────────────┐
│          客户端层 (Web / CLI / SDK)                    │
├─────────────────────────────────────────────────────┤
│       业务服务层 (ChatServer + 13 个 Handler)          │
│     ┌───────────────────────────────────────┐       │
│     │  AIHelper → StrategyFactory → Strategy │       │
│     │  AIToolRegistry (MCP)                 │       │
│     │  ImageRecognizer (ONNX + OpenCV)       │       │
│     │  AISpeechProcessor (TTS/ASR)          │       │
│     └───────────────────────────────────────┘       │
├─────────────────────────────────────────────────────┤
│      数据与消息层 (MySQL + RabbitMQ)                  │
├─────────────────────────────────────────────────────┤
│  推理与第三方平台 (阿里百炼 / 豆包 / ONNX Runtime)     │
└─────────────────────────────────────────────────────┘
```

![](images/image1.jpg)

## 核心技术点

### 多模型策略解耦（Strategy + Factory）

```
AIStrategy (抽象基类)
├── AliyunStrategy       → 阿里百炼 / 通义千问
├── AliyunRAGStrategy    → 百炼 RAG 检索增强
├── AliyunMcpStrategy    → 百炼 MCP 工具调用
└── DouBaoStrategy       → 字节豆包
```

- 统一抽象 `AIStrategy` 接口，模型差异完全封装在子类内
- `StrategyFactory` 单例 + 注册表模式，通过字符串键创建策略实例
- `StrategyRegister<T>` 模板利用静态全局对象在 `main()` 前自动注册
- `AIHelper::chat()` 通过 `modelType` 参数动态切换，业务代码零侵入

### 轻量级 MCP 工具调用

- 配置驱动的工具注册（`config.json`），工具定义与代码分离
- 两段式推理：模型判断工具调用 → C++ 端执行工具 → 结果注入二次回答
- Prompt 模板化，通过 `{user_input}` / `{tool_list}` 占位符动态填充

### 多租户会话隔离

```cpp
unordered_map<userId, unordered_map<sessionId, shared_ptr<AIHelper>>>
```

- 用户级 + 会话级两级隔离，不同用户/会话上下文互不干扰
- 启动时从 MySQL `chat_message` 表按 `(ts ASC, id ASC)` 回放历史消息
- 再次对话自动续接上下文

### RabbitMQ 异步持久化

- 请求线程同步写内存后立即返回，SQL 通过 RabbitMQ 异步投递
- `MQManager` 管理 Channel 连接池（默认 5 通道），原子计数器 Round-Robin 无锁分配
- `RabbitMQThreadPool` 消费者线程池批量消费入库，削峰解耦

### 语音链路（TTS）

- 集成百度 TTS API：任务创建 → 轮询状态 → 回传音频 URL
- 支持参数化语速、音色调整
- ASR 接口封装预留

### 图像识别

- 基于 OpenCV + ONNX Runtime 的本地推理管线
- 按用户维度的 `ImageRecognizer` 实例管理

## 项目结构

```
CppAIService/
├── CMakeLists.txt
├── HttpServer/                        # 自研 HTTP 框架
│   ├── include/
│   │   ├── http/                      # HttpServer, HttpRequest, HttpResponse
│   │   ├── router/                    # Router, RouterHandler（静态+正则动态路由）
│   │   ├── middleware/                # MiddlewareChain, CorsMiddleware
│   │   ├── session/                   # SessionManager, SessionStorage（内存存储）
│   │   ├── ssl/                       # SslContext, SslConnection
│   │   └── utils/                     # MysqlUtil（连接池）, JsonUtil, FileUtil, DbConnectionPool
│   └── src/                           # 对应实现文件
├── AIApps/ChatServer/                 # 业务服务
│   ├── include/
│   │   ├── ChatServer.h               # 核心调度类
│   │   ├── handlers/                  # 13 个 Handler（登录/注册/对话/历史/TTS/上传）
│   │   └── AIUtil/                    # AIStrategy, AIFactory, AIHelper, AIToolRegistry
│   ├── src/                           # 对应实现文件
│   └── resource/
│       └── config.json                # MCP 工具与 Prompt 配置
└── images/                            # 架构图
```

## API 端点

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/` `/entry` | 入口 |
| POST | `/login` | 登录 |
| POST | `/register` | 注册 |
| POST | `/user/logout` | 登出 |
| GET | `/chat` | 聊天页面 |
| POST | `/chat/send` | 发送消息（核心对话） |
| POST | `/chat/send-new-session` | 创建新会话并发送 |
| POST | `/chat/history` | 历史记录 |
| GET | `/chat/sessions` | 会话列表 |
| POST | `/chat/tts` | 语音合成 |
| GET | `/menu` | AI 功能菜单 |
| GET | `/upload` | 图片上传页 |
| POST | `/upload/send` | 图片 + 对话 |

## 依赖

| 依赖 | 用途 |
|------|------|
| muduo | 网络库（Reactor 事件驱动） |
| mysqlcppconn | MySQL 连接与连接池 |
| SimpleAmqpClient + librabbitmq | RabbitMQ 消息队列 |
| OpenSSL | SSL/TLS 加密传输 |
| libcurl | HTTP 客户端（调用模型 API） |
| OpenCV | 图像预处理 |
| ONNX Runtime | 本地模型推理 |
| nlohmann/json | JSON 解析（内嵌） |

## 构建与运行

### 依赖安装（Ubuntu/Debian）

```bash
# MySQL 客户端
apt install libmysqlcppconn-dev

# RabbitMQ 客户端
apt install librabbitmq-dev libsimpleamqpclient-dev

# OpenCV & ONNX Runtime
apt install libopencv-dev

# 其余依赖
apt install libcurl4-openssl-dev libssl-dev
```

muduo 和 ONNX Runtime 需从源码编译安装。

### 编译

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行

```bash
# 设置模型 API Key
export DASHSCOPE_API_KEY="your-aliyun-api-key"
export DOUBAO_API_KEY="your-doubao-api-key"      # 可选
export Knowledge_Base_ID="your-rag-app-id"        # 可选（RAG 模式需要）

# 启动服务
./http_server -p 8117
```

### Docker

确保 MySQL 与 RabbitMQ 已运行，服务启动后监听 `0.0.0.0:8117`。

## 请求示例

```bash
# 登录
curl -X POST http://localhost:8117/login \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"123456"}'

# 发送对话
curl -X POST http://localhost:8117/chat/send \
  -H "Content-Type: application/json" \
  -H "Cookie: SessionId=xxx" \
  -d '{"question":"你好","sessionId":"default","modelType":"1"}'
```

`modelType` 取值：`"1"` 阿里百炼 / `"2"` 豆包 / `"3"` 百炼 RAG / `"4"` 百炼 MCP。

## License

MIT License
