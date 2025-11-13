
# 基于Trie树与倒排索引的文本搜索引擎原型
## 项目概述
本项目是一款面向文本文档的搜索引擎原型，旨在通过结合**Trie树**与**倒排索引**两种经典数据结构，实现“前缀匹配查询+高效文档定位”的核心功能。  
项目采用分层设计架构：  
- **底层（C语言）**：实现Trie树、倒排索引、哈希表等核心数据结构与算法（确保性能与数据结构实践目标）；  
- **中间层（Python）**：负责文本预处理、C引擎调用桥接、HTTP API服务（衔接底层算法与前端交互）；  
- **表层（前端）**：通过HTML/CSS/JavaScript实现可视化交互界面（提供搜索框、实时建议、结果高亮展示）。  
整体覆盖“文档预处理→索引构建→实时搜索→结果展示”全链路，可直接运行并处理英文文本文档，体现数据结构在实际工程中的应用价值。

## 核心技术栈与数据结构
| 层级         | 技术/工具                  | 核心作用                                  |
|--------------|----------------------------|-------------------------------------------|
| 核心算法层   | C语言（MinGW/GCC编译）     | 实现Trie树、倒排索引、TF-IDF排序算法      |
| 数据处理层   | Python 3.x（NLTK库）       | 文本清洗、分词、C引擎调用、HTTP API服务   |
| 前端交互层   | HTML5/CSS3/JavaScript      | 搜索界面设计、实时建议、结果高亮展示      |
| 核心数据结构 | Trie树、倒排索引、哈希表   | 前缀匹配（Trie）、文档定位（倒排索引）、高效查询（哈希表） |
| 辅助工具     | VS Code（Live Server插件）  | 前端页面运行与调试                        |

## 功能模块与实现细节
### 1. 核心数据结构功能（C语言实现）
#### （1）Trie树（`trie.c`/`trie.h`）
| 函数名                  | 功能描述                                                                 |
|-------------------------|--------------------------------------------------------------------------|
| `trie_create_node`      | 创建Trie树节点（初始化26个字母子节点指针与`is_end_of_word`标记）         |
| `trie_insert`           | 插入词条到Trie树（仅处理小写英文字母，过滤非法字符）                     |
| `trie_search`           | 检查词条是否为Trie树中的完整词（用于精确匹配判断）                       |
| `trie_get_prefix_matches` | 获取所有以指定前缀开头的词条（前端实时建议功能的核心）                 |
| `trie_free`             | 递归释放Trie树内存（避免内存泄漏）                                       |

#### （2）倒排索引（`inverted_index.c`/`inverted_index.h`）
| 函数名                          | 功能描述                                                                 |
|---------------------------------|--------------------------------------------------------------------------|
| `hash_function`                 | 哈希函数（将词条映射到指定桶，提升查询效率）                             |
| `inverted_index_create`         | 创建倒排索引（初始化哈希桶数组，指定桶数量与总文档数）                   |
| `inverted_index_add_term`       | 向倒排索引添加词条（记录词条-文档ID-词频映射，支持同一文档词频累加）     |
| `inverted_index_get_postings`   | 获取词条对应的Postings列表（包含所有含该词条的文档ID与词频）             |
| `inverted_index_free`           | 释放倒排索引内存（递归释放索引节点与Postings列表）                       |

#### （3）TF-IDF排序（`tfidf.c`/`tfidf.h`）
| 函数名                          | 功能描述                                                                 |
|---------------------------------|--------------------------------------------------------------------------|
| `calculate_tfidf`               | 计算单个词条在文档中的TF-IDF分数（TF=对数归一化词频，IDF=逆文档频率）    |
| `calculate_document_scores`     | 累加多词条的TF-IDF分数（得到文档总相关性分数）                           |
| `sort_doc_scores`               | 文档分数降序排序（基于快速排序`qsort`，确保结果按相关性优先展示）         |

### 2. 数据预处理功能（Python实现）
#### （1）文本清洗（`data_cleaning.py`）
- 功能：处理原始文档中的噪声数据，输出标准化文本  
- 核心步骤：  
  1. 文本小写化（统一词条格式，避免“AI”与“ai”被视为不同词）；  
  2. 移除非ASCII字符与特殊符号（保留英文大小写字母与空格）；  
  3. 标准化空格（将多空格、换行符替换为单个空格）；  
  4. 输出到`cleaned_docs`目录（供后续索引构建使用）。

#### （2）高级预处理（`preprocess.py`）
- 功能：基于NLTK库实现更精细的文本处理（需提前安装`nltk`）  
- 核心步骤：  
  1. 去除HTML标签、URL与数字；  
  2. 分词（`word_tokenize`）与停用词过滤（过滤“the”“a”等无意义词，依赖`stopwords`数据集）；  
  3. 词干提取（`PorterStemmer`，将“running”“ran”统一为“run”）；  
  4. 输出到`processed_docs`目录（适合更复杂的文本场景）；  
  5. **NLTK数据集依赖**：需下载`punkt`、`punkt_tab`（适配NLTK 3.8+版本）、`stopwords`，放入`C:\Users\YourUsername\nltk_data`对应子目录（`tokenizers`：`punkt`/`punkt_tab`；`corpora`：`stopwords`）。

### 3. 前后端桥接与API服务（`build_bridge.py`）
- 核心作用：连接C语言引擎与前端，提供可调用的HTTP API  
- 主要功能：  
  1. **索引构建调用**：通过`subprocess`调用C引擎（`search_engine.exe`），从清洗后的文档生成索引文件（`trie.dat`/`inverted_index.dat`/`doc_paths.dat`），索引文件默认存储于`python_preprocess/index_data`目录；  
  2. **搜索调用**：接收前端查询请求，调用C引擎的命令行搜索模式（`search_engine.exe search "查询词"`），解析结果并返回JSON格式（包含`doc_path`文档路径、`score`相关性分数、`preview`内容预览）；  
  3. **HTTP API服务**：提供两个核心接口：  
     - `/search?q=查询词`：返回包含文档路径、相关性分数、预览的搜索结果；  
     - `/suggest?q=前缀`：返回基于Trie树的前缀匹配建议词（最多5个，输入≥2个字符触发）；  
  4. **跨域支持**：添加`Access-Control-Allow-Origin: *`头，确保前端可正常调用API；  
  5. **路径处理**：自动转换文档绝对路径，处理Windows/Linux斜杠差异，确保文档预览功能正常。

### 4. 前端交互功能（HTML/CSS/JS实现）
#### （1）界面布局（`index.html`/`style.css`）
- 核心组件：  
  1. 搜索框与按钮（支持回车触发搜索，输入框默认聚焦）；  
  2. 实时建议列表（输入≥2个字符时显示，hover效果优化，点击可自动填充搜索框并执行搜索）；  
  3. 加载状态与无结果提示（加载时显示旋转动画，无结果时显示友好提示文本）；  
  4. 结果展示区（显示文档路径、相关性分数、高亮预览，hover时轻微上浮并增强阴影）；  
  5. 响应式设计（适配移动端与PC端，移动端优化搜索框尺寸与按钮布局）。

#### （2）交互逻辑（`script.js`）
- 核心流程：  
  1. **实时建议**：通过防抖函数（300ms延迟）调用`/suggest` API，避免频繁请求，建议词前缀部分用`<strong>`标签高亮；  
  2. **搜索执行**：点击按钮或回车后，隐藏建议列表，显示加载状态，调用`/search` API，搜索完成后展示结果统计（含模拟搜索时间）；  
  3. **结果高亮**：使用正则表达式（全局+不区分大小写）匹配查询词，将预览中的匹配部分用`<strong>`标签高亮；  
  4. 交互优化：点击页面其他区域关闭建议列表，页面加载后自动聚焦搜索框，结果项hover时显示交互反馈。

## 项目目录结构
```
data-structure
├── .vscode\                   # VS Code配置文件
│   ├── c_cpp_properties.json  # C/C++编译配置（MinGW路径：C:/mingw64/bin/gcc.exe）
│   ├── launch.json            # 调试配置（C_core目录路径与可执行文件路径）
│   └── settings.json          # C/C++ Runner插件配置（编译器/调试器路径、警告选项）
├── c_core\                    # C语言核心引擎目录
│   ├── trie.c/.h              # Trie树实现（插入/前缀匹配/序列化）
│   ├── inverted_index.c/.h    # 倒排索引实现（哈希桶/Postings列表/序列化）
│   ├── tfidf.c/.h             # TF-IDF排序实现（分数计算/文档排序）
│   ├── search.c/.h            # 搜索逻辑实现（查询分词/前缀扩展/结果封装）
│   ├── utils.c/.h             # 工具函数（文档读取、索引构建、停用词加载）
│   ├── main.c                 # 入口函数（支持3种模式：构建索引/交互搜索/命令行搜索）
│   ├── search_engine.exe      # 编译后的C引擎可执行文件
│   └── stop_words.txt         # 停用词列表（过滤"the""a"等无意义词，供utils.c加载）
├── frontend\                  # 前端目录
│   ├── index.html             # 搜索界面HTML（搜索框/建议列表/结果区组件）
│   ├── style.css              # 界面样式（响应式布局/加载动画/hover效果）
│   └── script.js              # 交互逻辑（防抖建议/搜索执行/结果高亮）
└── python_preprocess\         # Python数据处理与桥接目录
    ├── data_cleaning.py       # 文本清洗脚本（生成cleaned_docs）
    ├── preprocess.py          # 高级预处理脚本（NLTK分词/词干提取，生成processed_docs）
    ├── build_bridge.py        # C引擎调用与API服务（索引构建/搜索/HTTP服务）
    ├── sample_docs\           # 原始文档目录（存放待处理的英文.txt文档）
    ├── cleaned_docs\          # 清洗后文档目录（索引构建默认数据源）
    ├── processed_docs\        # 高级预处理后文档目录（可选数据源）
    └── index_data\            # 索引文件目录（自动生成，C引擎默认读取路径）
        ├── trie.dat           # Trie树序列化文件
        ├── inverted_index.dat # 倒排索引序列化文件
        └── doc_paths.dat      # 文档路径列表文件（记录文档ID与绝对路径映射）
```

## 项目运行步骤
### 前置条件
1. 安装C编译器：确保已安装MinGW（或GCC），配置环境变量后通过`gcc -v`验证（VS Code配置中`compilerPath`指向`C:/mingw64/bin/gcc.exe`）；  
2. 安装Python 3.x：配置环境变量，通过`python -V`验证，执行`pip install nltk`安装预处理依赖；  
3. 前端工具：VS Code安装“Live Server”插件（用于运行前端页面，默认端口5500）；  
4. NLTK数据集：下载`punkt`、`punkt_tab`、`stopwords`，放入`C:\Users\YourUsername\nltk_data`对应子目录（`tokenizers`/`corpora`）。

### 步骤1：编译C语言核心引擎
0. 打开 .vscode\目录下的launch.json文件，修改"cwd"和"program"为本地地址。

1. 打开命令行，进入`c_core`目录：  
   ```bash
   cd c_core
   ```
<!-- 2. 执行分步编译命令（确保依赖顺序，生成`search_engine.exe`）：  
   ```bash
   # 编译各模块目标文件
   gcc -c trie.c -o trie.o
   gcc -c inverted_index.c -o inverted_index.o
   gcc -c tfidf.c -o tfidf.o
   gcc -c utils.c -o utils.o
   gcc -c search.c -o search.o
   gcc -c main.c -o main.o
   # 链接所有目标文件生成可执行文件
   gcc trie.o inverted_index.o tfidf.o utils.o search.o main.o -o search_engine.exe
   ``` -->
2. 编译

   方式 1：分步编译
    执行以下命令，先编译各模块生成目标文件（.o），再链接生成可执行文件：
    ```bash
    # 编译各模块目标文件（按依赖关系排序：基础数据结构→工具→业务逻辑→入口）
    gcc -c trie.c -o trie.o          # Trie树核心实现（无外部依赖）
    gcc -c inverted_index.c -o inverted_index.o  # 倒排索引（依赖哈希函数，无其他模块依赖）
    gcc -c tfidf.c -o tfidf.o        # TF-IDF算法（依赖倒排索引结构体，需在其之后编译）
    gcc -c utils.c -o utils.o        # 工具函数（依赖Trie树/倒排索引，需在两者之后编译）
    gcc -c search.c -o search.o      # 搜索逻辑（依赖Trie树/倒排索引/TF-IDF，需在三者之后编译）
    gcc -c main.c -o main.o          # 入口函数（依赖所有模块，最后编译）

    # 链接所有目标文件，生成最终可执行文件
    gcc trie.o inverted_index.o tfidf.o utils.o search.o main.o -o search_engine.exe
    ```
    方式 2：Makefile 一键编译（推荐，高效便捷）
    （1）创建 Makefile（仅需执行一次）
    在c_core目录下新建Makefile文件，复制以下内容（已有，这里做代码展示）：
    ```
    CC = gcc
    CFLAGS = -Wall -O2
    LDFLAGS = -lm

    all: search_engine

    search_engine: main.o trie.o inverted_index.o search.o tfidf.o utils.o
        $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

    main.o: main.c trie.h inverted_index.h search.h utils.h
        $(CC) $(CFLAGS) -c -o $@ $<

    trie.o: trie.c trie.h
        $(CC) $(CFLAGS) -c -o $@ $<

    inverted_index.o: inverted_index.c inverted_index.h
        $(CC) $(CFLAGS) -c -o $@ $<

    search.o: search.c search.h trie.h inverted_index.h tfidf.h
        $(CC) $(CFLAGS) -c -o $@ $<

    tfidf.o: tfidf.c tfidf.h inverted_index.h
        $(CC) $(CFLAGS) -c -o $@ $<

    utils.o: utils.c utils.h trie.h inverted_index.h
        $(CC) $(CFLAGS) -c -o $@ $<

    clean:
        del /f /q *.o search_engine.exe
    ```
    （2）执行编译 / 清理命令
    一键编译：在c_core目录下执行以下命令，自动完成所有编译步骤：
    ```bash
    make
    ```
    清理编译产物：若需重新编译（如修改代码后），可先清理旧文件再编译：
    ```bash
    make clean  # 清理所有.o文件和search_engine.exe
    make        # 重新编译生成新的可执行文件
    ```

3. 验证编译结果：`c_core`目录下出现`search_engine.exe`即成功。

### 步骤2：预处理文档（可选，推荐）
1. 准备原始文档：将英文文本文档（`.txt`格式）放入`python_preprocess/sample_docs`目录；  
2. 执行文本清洗脚本（生成`cleaned_docs`目录）：  
   ```bash
   # 进入Python目录
   cd ..\python_preprocess
   # 执行清洗脚本
   python data_cleaning.py
   ```
3. 执行高级预处理（需先完成NLTK数据集配置）：  
    - 访问NLTK下载页面：`https://www.nltk.org/data.html` ，下载`punkt`、`punkt_tab`和`stopwords`数据集（用于分词和停用词过滤）；
    - 放入`nltk_data`目录（默认路径：`C:\Users\YourUsername\nltk_data`）的`tokenizers`和`corpora`子目录下，（tokenizers：punkt、punkt_tab，corpora：stopwords）。
   ```bash
   # 执行高级预处理脚本
   python preprocess.py
   ```
   执行完成后，`processed_docs`目录将生成处理后的文档（分词+词干提取+停用词过滤）。

### 步骤3：构建索引
1. 在`python_preprocess`目录下，执行索引构建命令（默认基于`cleaned_docs`目录，若用`processed_docs`可替换目录名）：  
   ```bash
   python build_bridge.py --build-index cleaned_docs
   ```
2. 验证索引生成：`python_preprocess/index_data`目录下生成3个非空文件：  
   - `trie.dat`（Trie树序列化）、`inverted_index.dat`（倒排索引序列化）、`doc_paths.dat`（文档路径映射），即索引构建成功。

### 步骤4：启动API服务器
1. 在`python_preprocess`目录下，启动Python HTTP服务（默认端口8080，若端口占用可指定其他端口，如`--port 8888`）：  
   ```bash
   python build_bridge.py --server --port 8080
   ```
2. 验证服务启动：命令行显示以下信息即成功：  
   ```
   === 搜索服务器启动 ===
   地址：http://localhost:8080
   搜索示例：http://localhost:8080/search?q=ai
   建议示例：http://localhost:8080/suggest?q=ai
   按Ctrl+C关闭服务器
   ```
   若更换端口，需同步修改`frontend/script.js`中的`API_BASE_URL`（如`http://localhost:8888`）。

### 步骤5：运行前端页面
1. 用VS Code打开`frontend`目录，右键`index.html`文件；  
2. 选择“Open with Live Server”，浏览器自动打开前端页面（默认地址：`http://localhost:5501/frontend/index.html`）；  
3. 测试核心功能：  
   - **实时建议**：输入“ai”（≥2个字符），建议列表显示以“ai”开头的词（如“artificial”“intelligence”）；  
   - **搜索功能**：输入“climate”并回车，页面显示含该词的文档列表，预览中“climate”字样高亮，结果按相关性分数降序排列；  
   - **交互优化**：点击建议词自动填充搜索框并执行搜索，无结果时显示“无匹配结果”提示。

## 关键功能验证
1. **索引构建验证**：索引构建后，`python_preprocess/index_data`目录下3个文件（`trie.dat`/`inverted_index.dat`/`doc_paths.dat`）大小均不为0，且无编译或运行错误；  
2. **API验证**：浏览器访问`http://localhost:8080/search?q=ai`，返回JSON格式结果（含`doc_path`/`score`/`preview`字段）；访问`http://localhost:8080/suggest?q=ai`，返回5个以内前缀匹配建议词；  
3. **前端验证**：  
   - 输入查询词后，建议列表正常显示，无控制台报错；  
   - 搜索结果中查询词高亮正确，文档路径与实际预处理后文档路径一致；  
   - 移动端访问时，搜索框与结果区自适应屏幕宽度，交互逻辑正常。

## 项目亮点与数据结构应用总结
1. **Trie树应用**：核心用于“前缀匹配”，支持实时建议功能，查询时间复杂度O(L)（L为查询词长度），相比传统字符串遍历（O(N*L)，N为总词条数）更高效，尤其适合输入联想场景；  
2. **倒排索引应用**：核心用于“文档定位”，通过“词条→文档列表（含词频）”的映射关系，快速定位包含查询词的文档，避免全文档遍历，查询效率提升显著；  
3. **哈希表应用**：用于倒排索引的桶存储，将词条通过哈希函数映射到指定桶，降低词条查询时间复杂度（平均O(1)），解决线性查找效率低的问题；  
4. **分层设计优势**：C语言保障底层算法高性能（内存占用低、执行速度快），Python简化数据处理与API开发（代码简洁、库支持丰富），前端提升用户交互体验，符合工程化项目的“高性能+高开发效率”设计思路。

## 注意事项
1. **文档格式限制**：当前仅支持`.txt`格式英文文档，中文文档需扩展C语言字符处理逻辑（如支持Unicode）与Python预处理的中文分词（如引入`jieba`库）；  
2. **索引路径不可手动修改**：C引擎（`main.c`）与Python桥接层（`build_bridge.py`）默认读取`python_preprocess/index_data`目录下的索引文件，手动修改路径会导致索引加载失败；  
3. **端口占用解决方案**：若启动API服务器时提示“端口被占用”，可通过`--port`参数更换端口（如`python build_bridge.py --server --port 8888`），并同步修改`frontend/script.js`中的`API_BASE_URL`为新端口；  
4. **内存管理说明**：C引擎已实现完整的内存释放逻辑（`trie_free`/`inverted_index_free`/`free_search_results`），避免内存泄漏，无需手动干预；  
5. **NLTK版本兼容**：若运行`preprocess.py`时提示“`punkt`数据集不存在”，需检查是否下载`punkt_tab`（适配NLTK 3.8+版本），旧版本`punkt`可能无法正常分词。
