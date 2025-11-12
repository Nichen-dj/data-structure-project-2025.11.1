import os
import re
import string
import unicodedata

def remove_non_ascii(text):
    """移除非ASCII字符"""
    return unicodedata.normalize('NFKD', text).encode('ascii', 'ignore').decode('utf-8', 'ignore')

def remove_special_characters(text):
    """移除特殊字符"""
    pattern = r'[^a-zA-Z0-9\s]'
    return re.sub(pattern, '', text)

def normalize_whitespace(text):
    """标准化空格"""
    return re.sub(r'\s+', ' ', text).strip()

def clean_text(text):
    """完整的文本清洗流程"""
    # 转换为小写
    text = text.lower()
    
    # 移除非ASCII字符
    text = remove_non_ascii(text)
    
    # 移除特殊字符
    text = remove_special_characters(text)
    
    # 标准化空格
    text = normalize_whitespace(text)
    
    return text

def clean_document(input_path, output_path):
    """清洗单个文档"""
    try:
        with open(input_path, 'r', encoding='utf-8', errors='ignore') as file:
            text = file.read()
        
        cleaned_text = clean_text(text)
        
        with open(output_path, 'w', encoding='utf-8') as file:
            file.write(cleaned_text)
            
        return True
    except Exception as e:
        print(f"清洗文档 {input_path} 时出错: {e}")
        return False

def clean_directory(input_dir, output_dir):
    """清洗目录中的所有文档"""
    if not os.path.exists(input_dir):
        print(f"输入目录 {input_dir} 不存在")
        return
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    for filename in os.listdir(input_dir):
        input_path = os.path.join(input_dir, filename)
        
        if os.path.isfile(input_path) and filename.endswith('.txt'):
            print(f"清洗文件: {filename}")
            output_path = os.path.join(output_dir, filename)
            clean_document(input_path, output_path)

if __name__ == "__main__":
    input_directory = "sample_docs"
    output_directory = "cleaned_docs"
    
    print(f"开始清洗目录 {input_directory} 中的文档...")
    clean_directory(input_directory, output_directory)
    print(f"文档清洗完成，结果保存在 {output_directory}")