import json
import numpy as np
from sentence_transformers import SentenceTransformer
from tqdm import tqdm
import os
from langchain_experimental.text_splitter import SemanticChunker
from langchain_community.embeddings import HuggingFaceEmbeddings

# 初始化模型
model = SentenceTransformer('all-MiniLM-L6-v2')

def load_dataset(file_path="hotpotqa_longbench.json"):
    """加载数据集"""
    with open(file_path, "r") as f:
        data = json.load(f)
    return data

def semantic_chunk_text(text):
    # 使用兼容的嵌入模型
    embeddings = HuggingFaceEmbeddings(model_name="all-MiniLM-L6-v2")
    text_splitter = SemanticChunker(
        embeddings,
        breakpoint_threshold_type="percentile"
    )
    return text_splitter.split_text(text)

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
        # 语义分块处理上下文
        chunks = semantic_chunk_text(item['context'])
        
        # 为每个chunk生成embedding（自动批量处理）
        embeddings = model.encode(chunks, batch_size=32)
        
        # 保存信息（添加语义元数据）
        for i, (chunk, embedding) in enumerate(zip(chunks, embeddings)):
            all_chunks.append(chunk)
            all_embeddings.append(embedding)
            doc_info.append({
                'doc_id': item['id'],
                'chunk_id': i,
                'chunk': chunk,
                'answer': item['answer'],
            })
    
    # 保存embedding和相关信息
    np.save("embeddings/chunk_embeddings.npy", np.array(all_embeddings))
    with open("embeddings/chunk_info.json", "w") as f:
        json.dump(doc_info, f, ensure_ascii=False, indent=2)
    
    print(f"Preprocessing complete. Saved {len(all_chunks)} semantic chunks to embeddings/")

if __name__ == "__main__":
    preprocess_and_save_embeddings()