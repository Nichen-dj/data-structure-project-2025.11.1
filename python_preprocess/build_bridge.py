import os
import subprocess
import json
import re

class SearchEngineBridge:
    # def __init__(self, c_engine_path="../c_core/search_engine.exe", index_dir="../c_core/index_data"): 
    def __init__(self, c_engine_path="../c_core/search_engine.exe", index_dir="index_data"): 
        """
        初始化桥接器
        :param c_engine_path: C引擎可执行文件路径（相对python_preprocess目录）
        :param index_dir: C引擎索引目录（需与main.c的INDEX_DIR一致）
        """
        self.c_engine_path = c_engine_path
        self.index_dir = index_dir
        
        # 验证C引擎路径是否存在
        if not os.path.exists(self.c_engine_path):
            raise FileNotFoundError(f"C引擎程序不存在：{self.c_engine_path}，请先编译C代码")
        
        # 确保索引目录存在（C引擎构建索引时会自动创建，此处仅提示）
        if not os.path.exists(self.index_dir):
            print(f"警告：索引目录 {self.index_dir} 不存在，需先构建索引")

    def build_index(self, doc_dir):
        """调用C程序构建索引（文档目录为绝对路径或相对C引擎的路径）"""
        # 转换为绝对路径（避免相对路径混乱）
        doc_dir_abs = os.path.abspath(doc_dir)
        if not os.path.exists(doc_dir_abs):
            print(f"文档目录不存在：{doc_dir_abs}")
            return False
        
        try:
            # 调用C引擎构建索引（C引擎会在自身目录生成index_data）
            result = subprocess.run(
                [self.c_engine_path, doc_dir_abs],  # 参数：文档目录绝对路径
                capture_output=True,
                text=True,
                check=True,
                encoding='utf-8',
                errors='ignore'
            )
            
            print("=== 索引构建输出 ===")
            print(result.stdout)
            if result.stderr:
                print("=== 警告信息 ===")
                print(result.stderr)
            return True
        except subprocess.CalledProcessError as e:
            print(f"索引构建失败（返回码：{e.returncode}）：")
            print(f"错误输出：{e.stderr}")
            return False
        except Exception as e:
            print(f"索引构建过程中发生错误：{str(e)}")
            return False

    def search(self, query):
        """调用C程序进行搜索（命令行模式）"""
        if not query.strip():
            print("查询词不能为空")
            return []
        
        try:
            # 调用C引擎的命令行搜索模式：search_engine.exe search "查询词"
            result = subprocess.run(
                [self.c_engine_path, "search", query.strip()],
                capture_output=True,
                text=True,
                check=True,
                encoding='utf-8',
                errors='ignore'
            )
            
            stdout = result.stdout
            stderr = result.stderr
            
            # 打印调试信息（可选）
            print("=== C引擎原始输出 ===")
            print(stdout)
            if stderr:
                print("=== C引擎错误输出 ===")
                print(stderr)
            
            # 解析搜索结果
            parsed_results = self._parse_search_results(stdout, query)
            return parsed_results
        except subprocess.CalledProcessError as e:
            print(f"搜索失败（返回码：{e.returncode}）：")
            print(f"错误输出：{e.stderr}")
            return []
        except Exception as e:
            print(f"搜索过程中出错：{str(e)}")
            return []

    def _parse_search_results(self, output, query):
        """解析C引擎的搜索输出"""
        results = []
        lines = [line.strip() for line in output.split('\n') if line.strip()]
        
        # 找到结果开始的行（匹配"找到 X 个结果："）
        result_start_idx = -1
        for idx, line in enumerate(lines):
            if re.match(r'^找到 \d+ 个结果：$', line):
                result_start_idx = idx + 1
                break
        
        if result_start_idx == -1:
            return results  # 无结果
        
        # 解析每个结果行（格式：1. 文档: 路径 (分数: 0.1234)）
        result_pattern = re.compile(r'^(\d+)\. 文档: (.*?) \(分数: ([\d.]+)\)$')
        for line in lines[result_start_idx:]:
            match = result_pattern.match(line)
            if match:
                doc_path = match.group(2)
                score = float(match.group(3))
                
                # 标准化文档路径（处理Windows/Linux斜杠差异）
                doc_path_norm = os.path.normpath(doc_path)
                
                # 获取文档预览（最多200字符）
                preview = self._get_document_preview(doc_path_norm)
                
                results.append({
                    "doc_path": doc_path_norm,
                    "score": score,
                    "preview": preview
                })
        
        return results

    def _get_document_preview(self, doc_path, max_chars=200):
        """获取文档内容预览（处理编码和路径错误）"""
        if not os.path.exists(doc_path):
            return "文档不存在或路径无效"
        
        try:
            with open(doc_path, 'r', encoding='utf-8', errors='ignore') as file:
                content = file.read(max_chars).strip()
                if len(content) > max_chars:
                    return content[:max_chars] + "..."
                return content if content else "文档内容为空"
        except Exception as e:
            return f"获取预览失败：{str(e)[:50]}"

    def run_server(self, host="localhost", port=8000):
        """启动HTTP服务器，提供搜索和建议API（修复参数传递问题）"""
        from http.server import BaseHTTPRequestHandler, HTTPServer
        import urllib.parse

        # -------------------------- 核心修复：用类继承传递bridge参数 --------------------------
        class SearchServerHandler(BaseHTTPRequestHandler):
            # 类属性：存储bridge实例（替代lambda传递）
            bridge = self

            def _send_json_response(self, data, status_code=200):
                """发送JSON格式响应，处理跨域"""
                self.send_response(status_code)
                self.send_header('Content-type', 'application/json; charset=utf-8')
                self.send_header('Access-Control-Allow-Origin', '*')  # 允许所有跨域请求（开发环境）
                self.send_header('Access-Control-Allow-Methods', 'GET, OPTIONS')
                self.end_headers()
                self.wfile.write(json.dumps(data, ensure_ascii=False).encode('utf-8'))

            def do_OPTIONS(self):
                """处理预检请求（跨域必要）"""
                self._send_json_response({}, 200)

            def do_GET(self):
                parsed_path = urllib.parse.urlparse(self.path)
                query_params = urllib.parse.parse_qs(parsed_path.query)

                # 1. 搜索API：/search?q=查询词
                if parsed_path.path == '/search' and 'q' in query_params:
                    query = query_params['q'][0]
                    results = self.bridge.search(query)  # 直接使用类属性bridge
                    self._send_json_response(results)
                
                # 2. 建议API：/suggest?q=前缀（基于Trie的前缀匹配）
                elif parsed_path.path == '/suggest' and 'q' in query_params:
                    prefix = query_params['q'][0].strip()
                    if len(prefix) < 2:
                        self._send_json_response([])
                        return
                    suggestions = self._get_prefix_suggestions(prefix)
                    self._send_json_response(suggestions[:5])  # 最多返回5个建议
                
                # 3. 无效API
                else:
                    self._send_json_response({"error": "无效API路径，支持/search和/suggest"}, 404)

            def _get_prefix_suggestions(self, prefix):
                """简化的前缀建议：从搜索结果中提取以prefix开头的词"""
                suggestions = set()
                results = self.bridge.search(prefix)
                for res in results:
                    words = re.findall(r'\b[a-z]+\b', res['preview'].lower())
                    for word in words:
                        if word.startswith(prefix.lower()) and len(word) > len(prefix):
                            suggestions.add(word)
                suggestions.add(prefix.lower())
                return sorted(suggestions)

        # -------------------------- 修复服务器初始化：直接传递Handler类 --------------------------
        # 不再用lambda，直接传递SearchServerHandler类（类属性已绑定bridge）
        server_address = (host, port)
        httpd = HTTPServer(server_address, SearchServerHandler)

        print(f"=== 搜索服务器启动 ===")
        print(f"地址：http://{host}:{port}")
        print(f"搜索示例：http://{host}:{port}/search?q=ai")
        print(f"建议示例：http://{host}:{port}/suggest?q=ai")
        print(f"按Ctrl+C关闭服务器")

        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n=== 服务器正在关闭 ===")
            httpd.shutdown()

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='C-Python前端桥接程序（搜索引擎）')
    parser.add_argument('--build-index', help='构建索引的文档目录（绝对路径或相对当前目录）')
    parser.add_argument('--search', help='测试搜索：--search "查询词"')
    parser.add_argument('--server', action='store_true', help='启动HTTP服务器（默认localhost:8000）')
    parser.add_argument('--host', default='localhost', help='服务器主机（默认localhost）')
    parser.add_argument('--port', type=int, default=8000, help='服务器端口（默认8000）')

    args = parser.parse_args()

    try:
        # 初始化桥接器（默认路径：../c_core/search_engine.exe）
        bridge = SearchEngineBridge()

        # 模式1：构建索引
        if args.build_index:
            doc_dir = args.build_index
            print(f"开始构建索引，文档目录：{doc_dir}")
            success = bridge.build_index(doc_dir)
            print("索引构建成功！" if success else "索引构建失败！")
        
        # 模式2：测试搜索
        elif args.search:
            query = args.search
            print(f"测试搜索：{query}")
            results = bridge.search(query)
            if results:
                print(f"\n找到 {len(results)} 个结果：")
                for i, res in enumerate(results, 1):
                    print(f"{i}. 路径：{res['doc_path']}")
                    print(f"   分数：{res['score']:.4f}")
                    print(f"   预览：{res['preview'][:100]}...\n")
            else:
                print("未找到结果")
        
        # 模式3：启动服务器
        elif args.server:
            bridge.run_server(host=args.host, port=args.port)
        
        # 无参数：显示帮助
        else:
            parser.print_help()
    except Exception as e:
        print(f"程序初始化失败：{str(e)}")