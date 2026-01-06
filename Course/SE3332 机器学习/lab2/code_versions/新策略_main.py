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
from rank_bm25 import BM25Okapi
from typing import List, Dict

# Initialize models and client
model = SentenceTransformer('all-MiniLM-L6-v2')  # Same model as used in preprocessing
client = OpenAI(base_url="http://47.242.151.133:12596/v1", api_key="abc123")

# --- Load preprocessed data ---
def load_embeddings():
    """Load precomputed embeddings and chunk information"""
    embeddings = np.load("embeddings/chunk_embeddings.npy")
    with open("embeddings/chunk_info.json", "r") as f:
        chunk_info = json.load(f)
    
    # Prepare text corpus for BM25
    corpus = [info['chunk'] for info in chunk_info]
    tokenized_corpus = [doc.split() for doc in corpus]  # Simple whitespace tokenizer (no jieba)
    bm25 = BM25Okapi(tokenized_corpus)
    
    return embeddings, chunk_info, bm25

# --- Hybrid retrieval function ---
def retrieve_relevant_chunks(query: str, 
                           embeddings: np.ndarray, 
                           chunk_info: List[Dict], 
                           bm25: BM25Okapi,
                           top_k: int = 3,
                           alpha: float = 0.5) -> List[Dict]:
    """
    Retrieve relevant chunks using hybrid BM25 + vector similarity approach
    
    Args:
        query: User query/question
        embeddings: Precomputed chunk embeddings
        chunk_info: Metadata about chunks
        bm25: Initialized BM25 model
        top_k: Number of chunks to return
        alpha: Weight for BM25 score (1-alpha for vector similarity)
    
    Returns:
        List of relevant chunks with combined scores, sorted by relevance
    """
    # Vector similarity retrieval
    query_embedding = model.encode([query])
    vector_similarities = cosine_similarity(query_embedding, embeddings)[0]
    
    # BM25 retrieval
    tokenized_query = query.split()  # Simple whitespace tokenizer
    bm25_scores = bm25.get_scores(tokenized_query)
    
    # Normalize scores to [0,1] range
    norm_vector_scores = (vector_similarities - vector_similarities.min()) / \
                        (vector_similarities.max() - vector_similarities.min() + 1e-10)
    norm_bm25_scores = (bm25_scores - bm25_scores.min()) / \
                      (bm25_scores.max() - bm25_scores.min() + 1e-10)
    
    # Combine scores
    combined_scores = alpha * norm_bm25_scores + (1 - alpha) * norm_vector_scores
    
    # Get top_k indices
    top_indices = np.argsort(combined_scores)[-top_k:][::-1]
    
    # Prepare results
    retrieved_chunks = []
    for idx in top_indices:
        retrieved_chunks.append({
            'text': chunk_info[idx]['chunk'],
            'doc_id': chunk_info[idx]['doc_id'],
            'similarity': float(combined_scores[idx]),
            'vector_score': float(vector_similarities[idx]),
            'bm25_score': float(bm25_scores[idx])
        })
    
    return retrieved_chunks

# --- Model interaction ---
def query_chat_model(messages: List[Dict], max_tokens: int = 64) -> str:
    """Query LLM model with enhanced error handling"""
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

# --- Evaluation functions (unchanged) ---
def normalize_answer(s):
    """Normalize answer for evaluation"""
    def remove_articles(text):
        return re.sub(r'\b(a|an|the)\b', ' ', text)
    def white_space_fix(text):
        return ' '.join(text.split())
    def remove_punc(text):
        exclude = set(string.punctuation)
        # Remove all punctuation except periods (we'll handle them separately)
        return ''.join(ch for ch in text if ch not in exclude or ch == '.')
    def lower(text):
        return text.lower()
    def remove_trailing_period(text):
        # Only remove period if it's at the very end
        return text[:-1] if text.endswith('.') else text
    
    normalized = white_space_fix(remove_articles(remove_punc(lower(s))))
    return remove_trailing_period(normalized)

def compute_em(a_gold, a_pred):
    """Compute exact match score"""
    return int(normalize_answer(a_gold) == normalize_answer(a_pred))

def compute_f1(a_gold, a_pred):
    """Compute F1 score"""
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

# --- Enhanced RAG pipeline with multi-hop prompting ---
def run_rag_evaluation(num_samples: int = 200):
    """Run RAG evaluation with hybrid retrieval and multi-hop prompting"""
    # Load preprocessed data
    print("Loading embeddings and initializing BM25...")
    embeddings, chunk_info, bm25 = load_embeddings()
    
    # Load dataset
    print("Loading dataset...")
    with open("hotpotqa_longbench.json", "r") as f:
        dataset = json.load(f)[:num_samples]
    
    results = []
    
    for item in tqdm(dataset, desc="Processing questions"):
        # 1. Retrieve relevant chunks with hybrid approach
        retrieved_chunks = retrieve_relevant_chunks(
            query=item['question'],
            embeddings=embeddings,
            chunk_info=chunk_info,
            bm25=bm25,
            top_k=20  # Retrieve more chunks for multi-hop reasoning
        )
        
        # 2. Build context with relevance scores
        context = "\n\n".join([
            f"Document {i+1} (Relevance: {chunk['similarity']:.2f}, "
            f"Vector: {chunk['vector_score']:.2f}, BM25: {chunk['bm25_score']:.2f}):\n{chunk['text']}"
            for i, chunk in enumerate(retrieved_chunks)
        ])
        
        # 3. Construct multi-hop reasoning prompt
        messages = [
            {
                "role": "system",
                "content": "You are an assistant specialized in answering multi-hop questions using provided context."
            },
            {
                "role": "user",
                "content": f"Context:\n{context}\n\nQuestion: {item['question']}\n\n"
                        "Instruction: Using ONLY the given context, provide a precise answer to this multi-hop question. "
                        "You should reason through the context and by delicately considering each part but don't output your reasoning. "
                        "For Yes/No questions, respond strictly with 'Yes' or 'No'. "
                        "For all others, give only the exact required information (name, date, number, or brief phrase) "
                        "without forming complete sentences or adding explanations."
            }
        ]
        
        # 4. Query LLM with increased max_tokens for reasoning
        predicted_answer = query_chat_model(messages, max_tokens=128)
        
        if predicted_answer is not None:
            # Extract just the final answer if present
            final_answer_match = re.search(r"Final Answer:\s*(.+)", predicted_answer, re.IGNORECASE)
            final_answer = final_answer_match.group(1).strip() if final_answer_match else predicted_answer.strip()
            
            results.append({
                'id': item['id'],
                'question': item['question'],
                'predicted_answer': final_answer,
                'full_response': predicted_answer,
                'golden_answer': item['answer'],
                'retrieved_chunks': [chunk['text'] for chunk in retrieved_chunks]
            })
    
    # 5. Evaluation
    em_scores = [compute_em(r['golden_answer'], r['predicted_answer']) for r in results]
    f1_scores = [compute_f1(r['golden_answer'], r['predicted_answer']) for r in results]
    
    avg_em = np.mean(em_scores)
    avg_f1 = np.mean(f1_scores)
    
    print("\n" + "="*50)
    print(f"Hybrid RAG Performance: EM = {avg_em:.3f}, F1 = {avg_f1:.3f}")
    print("="*50 + "\n")
    
    # 6. Save results
    os.makedirs("results", exist_ok=True)
    
    with open("results/hybrid_outputs.json", "w") as f:
        json.dump([{'id': r['id'], 'pred_answer': r['predicted_answer']} for r in results], f, indent=2)
    
    with open("results/hybrid_full_results.json", "w") as f:
        json.dump(results, f, indent=2)
    
    return results, avg_em, avg_f1

if __name__ == "__main__":
    run_rag_evaluation()