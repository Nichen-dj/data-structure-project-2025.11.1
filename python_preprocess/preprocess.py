import os
import re
import string
import nltk
from nltk.tokenize import word_tokenize
from nltk.corpus import stopwords
from nltk.stem import PorterStemmer



class TextPreprocessor:
    def __init__(self):
        self.stop_words = set(stopwords.words('english'))
        self.stemmer = PorterStemmer()
        self.punctuation = set(string.punctuation)
        
    def clean_text(self, text):
        """清理文本：去除特殊字符、转换为小写等"""
        # 转换为小写
        text = text.lower()
        
        # 去除HTML标签
        text = re.sub(r'<.*?>', '', text)
        
        # 去除URL
        text = re.sub(r'http\S+|www\S+|https\S+', '', text, flags=re.MULTILINE)
        
        # 去除数字
        text = re.sub(r'\d+', '', text)
        
        # 去除标点符号
        text = text.translate(str.maketrans('', '', string.punctuation))
        
        # 去除额外的空格
        text = re.sub(r'\s+', ' ', text).strip()
        
        return text
    
    def tokenize(self, text):
        """分词并进行词干提取和停用词去除"""
        # 分词
        tokens = word_tokenize(text)
        
        # 去除停用词和单字母词
        tokens = [token for token in tokens if token not in self.stop_words and len(token) > 1]
        
        # 词干提取
        tokens = [self.stemmer.stem(token) for token in tokens]
        
        return tokens
    
    def process_document(self, file_path):
        """处理单个文档并返回处理后的词列表"""
        try:
            with open(file_path, 'r', encoding='utf-8') as file:
                text = file.read()
            
            cleaned_text = self.clean_text(text)
            tokens = self.tokenize(cleaned_text)
            
            return tokens
        except Exception as e:
            print(f"处理文档 {file_path} 时出错: {e}")
            return []
    
    def process_directory(self, dir_path, output_dir=None):
        """处理目录中的所有文档"""
        if not os.path.isdir(dir_path):
            print(f"目录 {dir_path} 不存在")
            return
        
        # 如果指定了输出目录，创建它
        if output_dir and not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        # 处理目录中的所有文件
        for filename in os.listdir(dir_path):
            file_path = os.path.join(dir_path, filename)
            
            # 只处理文本文件
            if os.path.isfile(file_path) and filename.endswith('.txt'):
                print(f"处理文件: {filename}")
                tokens = self.process_document(file_path)
                
                # 如果指定了输出目录，保存处理结果
                if output_dir:
                    output_path = os.path.join(output_dir, filename)
                    with open(output_path, 'w', encoding='utf-8') as out_file:
                        out_file.write(' '.join(tokens))

if __name__ == "__main__":
    preprocessor = TextPreprocessor()
    
    # 处理示例文档
    input_dir = "sample_docs"
    output_dir = "processed_docs"
    
    print(f"开始处理目录 {input_dir} 中的文档...")
    preprocessor.process_directory(input_dir, output_dir)
    print("文档处理完成，结果保存在", output_dir)