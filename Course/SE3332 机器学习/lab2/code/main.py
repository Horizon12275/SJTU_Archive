import json
import numpy as np
from sentence_transformers import SentenceTransformer
from sklearn.metrics.pairwise import cosine_similarity
from openai import OpenAI
import os
from tqdm import tqdm
import re
import string
import collections

# 初始化模型和客户端
model = SentenceTransformer('all-MiniLM-L6-v2')  # 与预处理使用相同的模型
client = OpenAI(base_url="http://47.242.151.133:12596/v1", api_key="abc123")

# --- 加载预处理数据 ---
def load_embeddings():
    """加载预处理好的embedding和chunk信息"""
    embeddings = np.load("embeddings/chunk_embeddings.npy")
    with open("embeddings/chunk_info.json", "r") as f:
        chunk_info = json.load(f)
    return embeddings, chunk_info

# --- 检索相关文本块 ---
def retrieve_relevant_chunks(query, embeddings, chunk_info, top_k=3):
    """
    检索与查询最相关的文本块
    Args:
        query: 查询问题
        embeddings: 所有chunk的嵌入向量
        chunk_info: chunk的元信息
        top_k: 返回最相关的k个chunk
    Returns:
        list: 相关chunk的列表，按相关性排序
    """
    # 生成查询的embedding
    query_embedding = model.encode([query])
    
    # 计算余弦相似度
    similarities = cosine_similarity(query_embedding, embeddings)[0]
    
    # 获取最相关的top_k个chunk
    top_indices = np.argsort(similarities)[-top_k:][::-1]
    
    # 返回相关信息
    retrieved_chunks = []
    for idx in top_indices:
        retrieved_chunks.append({
            'text': chunk_info[idx]['chunk'],
            'doc_id': chunk_info[idx]['doc_id'],
            'similarity': float(similarities[idx])
        })
    
    return retrieved_chunks

# --- 模型交互 ---
def query_chat_model(messages, max_tokens=32):
    """查询LLM模型"""
    try:
        response = client.chat.completions.create(
            model="meta-llama/Llama-3.1-8B-Instruct",
            messages=messages,
            max_tokens=max_tokens,
            temperature=0.7,
        )
        return response.choices[0].message.content
    except Exception as e:
        print(f"Error querying model: {e}")
        return None

# --- 评估函数 ---
def normalize_answer(s):
    """标准化答案用于评估"""
    def remove_articles(text):
        return re.sub(r'\b(a|an|the)\b', ' ', text)
    def white_space_fix(text):
        return ' '.join(text.split())
    def remove_punc(text):
        exclude = set(string.punctuation)
        return ''.join(ch for ch in text if ch not in exclude)
    def lower(text):
        return text.lower()
    return white_space_fix(remove_articles(remove_punc(lower(s))))

def compute_em(a_gold, a_pred):
    """计算精确匹配分数"""
    return int(normalize_answer(a_gold) == normalize_answer(a_pred))

def compute_f1(a_gold, a_pred):
    """计算F1分数"""
    gold_toks = normalize_answer(a_gold).split()
    pred_toks = normalize_answer(a_pred).split()

    if not gold_toks or not pred_toks:
        return int(gold_toks == pred_toks)

    common = collections.Counter(gold_toks) & collections.Counter(pred_toks)
    num_same = sum(common.values())

    if num_same == 0:
        return 0

    precision = 1.0 * num_same / len(pred_toks)
    recall = 1.0 * num_same / len(gold_toks)
    f1 = (2 * precision * recall) / (precision + recall)
    return f1

# --- RAG主流程 ---
def run_rag_evaluation(num_samples=10):
    """运行RAG评估"""
    # 加载预处理数据
    print("Loading embeddings...")
    embeddings, chunk_info = load_embeddings()
    
    # 加载数据集
    print("Loading dataset...")
    with open("hotpotqa_longbench.json", "r") as f:
        dataset = json.load(f)[:num_samples]
    
    results = []
    
    for item in tqdm(dataset, desc="Processing questions"):
        # 1. 检索相关chunk
        retrieved_chunks = retrieve_relevant_chunks(
            query=item['question'],
            embeddings=embeddings,
            chunk_info=chunk_info,
            top_k=8  # 返回最相关的3个chunk
        )
        
        # 2. 构建上下文
        context = "\n\n".join([f"Document {i+1} (Relevance: {chunk['similarity']:.2f}):\n{chunk['text']}" 
                             for i, chunk in enumerate(retrieved_chunks)])
        
        # 3. 构建LLM提示
        messages = [
            {
                "role": "system",
                "content": "You are a helpful assistant in answering multi-hop questions. "
                           "Answer based ONLY on the provided context."
            },
            {
                "role": "user",
                "content": f"Context:\n{context}\n\nQuestion: {item['question']}\n\n"
                            "Instruction: Based *only* on the provided context, answer the question concisely. "
                            "For Yes/No questions, answer only 'Yes' or 'No'. "
                            "For other questions, provide only the exact answer phrase (e.g., a name, date, number, or short phrase), "
                            "without forming a complete sentence."
            }
        ]
        
        # 4. 查询LLM
        predicted_answer = query_chat_model(messages)
        
        if predicted_answer is not None:
            results.append({
                'id': item['id'],
                'question': item['question'],
                'predicted_answer': predicted_answer,
                'golden_answer': item['answer'],
                'retrieved_chunks': [chunk['text'] for chunk in retrieved_chunks]  # 保存检索结果用于分析
            })
    
    # 5. 评估
    em_scores = [compute_em(r['golden_answer'], r['predicted_answer']) for r in results]
    f1_scores = [compute_f1(r['golden_answer'], r['predicted_answer']) for r in results]
    
    avg_em = np.mean(em_scores)
    avg_f1 = np.mean(f1_scores)
    
    print("\n" + "="*50)
    print(f"RAG Performance: EM = {avg_em:.3f}, F1 = {avg_f1:.3f}")
    print("="*50 + "\n")
    
    # 6. 保存结果
    os.makedirs("results", exist_ok=True)
    
    # 保存精简结果（仅ID和预测答案）
    with open("results/outputs.json", "w") as f:
        json.dump([{'id': r['id'], 'pred_answer': r['predicted_answer']} for r in results], f, indent=2)
    
    # 保存完整结果（含检索上下文）
    with open("results/full_results.json", "w") as f:
        json.dump(results, f, indent=2)
    
    return results, avg_em, avg_f1

if __name__ == "__main__":
    run_rag_evaluation()