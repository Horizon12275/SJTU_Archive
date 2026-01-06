import json
import numpy as np
from sentence_transformers import SentenceTransformer
from tqdm import tqdm
import os

# 更新为更现代的轻量级模型（无需均值池化警告）
model = SentenceTransformer('all-MiniLM-L6-v2')  # 替换原来的模型

def load_dataset(file_path="hotpotqa_longbench.json"):
    """加载数据集"""
    with open(file_path, "r") as f:
        data = json.load(f)
    return data

def chunk_text(text, chunk_size=100, overlap=30):
    """将长文本分块"""
    words = text.split()
    chunks = []
    start = 0
    while start < len(words):
        end = min(start + chunk_size, len(words))
        chunk = ' '.join(words[start:end])
        chunks.append(chunk)
        if end == len(words):
            break
        start = end - overlap
    return chunks

def preprocess_and_save_embeddings():
    """预处理数据并保存embedding"""
    print("Loading dataset...")
    dataset = load_dataset()
    
    # 创建存储目录
    os.makedirs("embeddings", exist_ok=True)
    
    # 处理每个文档
    all_chunks = []
    all_embeddings = []
    doc_info = []
    
    for item in tqdm(dataset, desc="Processing documents"):
        # 分块处理上下文
        chunks = chunk_text(item['context'])
        
        # 为每个chunk生成embedding（自动批量处理，无需show_progress_bar）
        embeddings = model.encode(chunks, batch_size=32)  # 增加batch_size加速
        
        # 保存信息
        for i, (chunk, embedding) in enumerate(zip(chunks, embeddings)):
            all_chunks.append(chunk)
            all_embeddings.append(embedding)
            doc_info.append({
                'doc_id': item['id'],
                'chunk_id': i,
                'chunk': chunk,
                'answer': item['answer']
            })
    
    # 保存embedding和相关信息
    np.save("embeddings/chunk_embeddings.npy", np.array(all_embeddings))
    with open("embeddings/chunk_info.json", "w") as f:
        json.dump(doc_info, f, ensure_ascii=False, indent=2)  # 更好的可读性
    
    print(f"Preprocessing complete. Saved {len(all_chunks)} chunks to embeddings/")

if __name__ == "__main__":
    preprocess_and_save_embeddings()