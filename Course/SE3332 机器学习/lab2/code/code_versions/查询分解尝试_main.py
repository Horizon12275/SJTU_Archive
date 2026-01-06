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
from typing import List, Dict, Tuple

# Initialize models and client
model = SentenceTransformer('all-MiniLM-L6-v2')
client = OpenAI(base_url="http://47.242.151.133:12596/v1", api_key="abc123")

# --- Load preprocessed data ---
def load_embeddings():
    """Load precomputed embeddings and chunk information"""
    embeddings = np.load("embeddings/chunk_embeddings.npy")
    with open("embeddings/chunk_info.json", "r") as f:
        chunk_info = json.load(f)
    
    corpus = [info['chunk'] for info in chunk_info]
    tokenized_corpus = [doc.split() for doc in corpus]
    bm25 = BM25Okapi(tokenized_corpus)
    
    return embeddings, chunk_info, bm25

# --- Query Decomposition ---
def decompose_query(query: str) -> Tuple[List[str], str]:
    """
    Decompose complex query into sub-questions using LLM
    
    Args:
        query: Original complex question
        
    Returns:
        Tuple of (sub_questions, reasoning_steps)
    """
    decomposition_prompt = [
        {
            "role": "system",
            "content": "You are an expert at breaking down complex questions into simpler sub-questions. "
                       "Identify the key components needed to answer the main question."
        },
        {
            "role": "user",
            "content": f"Original question: {query}\n\n"
                      "Break this down into 2-3 simpler sub-questions that would help answer it. "
                      "Format your response as:\n"
                      "1. [Sub-question 1]\n"
                      "2. [Sub-question 2]\n"
                      "...\n"
                      "Reasoning: [Brief explanation of how these connect to the original question]"
        }
    ]
    
    response = client.chat.completions.create(
        model="meta-llama/Llama-3.1-8B-Instruct",
        messages=decomposition_prompt,
        max_tokens=256,
        temperature=0.3
    )
    
    decomposition = response.choices[0].message.content
    
    # Parse the response to extract sub-questions
    sub_questions = []
    reasoning = ""
    
    lines = decomposition.split('\n')
    for line in lines:
        if line.strip().startswith(('1.', '2.', '3.', '4.')):
            sub_q = line.split('.', 1)[1].strip()
            sub_questions.append(sub_q)
        elif line.lower().startswith('reasoning:'):
            reasoning = line.split(':', 1)[1].strip()
    
    return sub_questions, reasoning

# --- Hybrid retrieval function (updated for sub-questions) ---
def retrieve_relevant_chunks(query: str, 
                           sub_questions: List[str],
                           embeddings: np.ndarray, 
                           chunk_info: List[Dict], 
                           bm25: BM25Okapi,
                           top_k: int = 3,
                           alpha: float = 0.5) -> List[Dict]:
    """
    Retrieve relevant chunks using hybrid BM25 + vector similarity approach
    with consideration of sub-questions
    
    Args:
        query: Original user query/question
        sub_questions: List of decomposed sub-questions
        embeddings: Precomputed chunk embeddings
        chunk_info: Metadata about chunks
        bm25: Initialized BM25 model
        top_k: Number of chunks to return per query
        alpha: Weight for BM25 score (1-alpha for vector similarity)
    
    Returns:
        List of relevant chunks with combined scores, sorted by relevance
    """
    all_queries = [query] + sub_questions
    combined_scores = np.zeros(len(embeddings))
    
    for q in all_queries:
        # Vector similarity retrieval
        query_embedding = model.encode([q])
        vector_similarities = cosine_similarity(query_embedding, embeddings)[0]
        
        # BM25 retrieval
        tokenized_query = q.split()
        bm25_scores = bm25.get_scores(tokenized_query)
        
        # Normalize scores
        norm_vector_scores = (vector_similarities - vector_similarities.min()) / \
                            (vector_similarities.max() - vector_similarities.min() + 1e-10)
        norm_bm25_scores = (bm25_scores - bm25_scores.min()) / \
                          (bm25_scores.max() - bm25_scores.min() + 1e-10)
        
        # Combine and accumulate scores across all queries
        combined_scores += alpha * norm_bm25_scores + (1 - alpha) * norm_vector_scores
    
    # Get top_k indices (now top_k*3 since we have more queries)
    top_indices = np.argsort(combined_scores)[-top_k*3:][::-1]
    
    # Prepare results
    retrieved_chunks = []
    for idx in top_indices:
        retrieved_chunks.append({
            'text': chunk_info[idx]['chunk'],
            'doc_id': chunk_info[idx]['doc_id'],
            'similarity': float(combined_scores[idx]),
            'query': query if idx < top_k else sub_questions[idx % len(sub_questions)]
        })
    
    # Remove duplicates while preserving order
    seen_texts = set()
    unique_chunks = []
    for chunk in retrieved_chunks:
        if chunk['text'] not in seen_texts:
            seen_texts.add(chunk['text'])
            unique_chunks.append(chunk)
    
    return unique_chunks[:top_k*2]  # Return slightly more chunks for multi-hop

# --- Model interaction ---
def query_chat_model(messages: List[Dict], max_tokens: int = 128) -> str:
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

# --- Evaluation functions ---
def normalize_answer(s):
    """Normalize answer for evaluation"""
    def remove_articles(text):
        return re.sub(r'\b(a|an|the)\b', ' ', text)
    def white_space_fix(text):
        return ' '.join(text.split())
    def remove_punc(text):
        exclude = set(string.punctuation)
        return ''.join(ch for ch in text if ch not in exclude or ch == '.')
    def lower(text):
        return text.lower()
    def remove_trailing_period(text):
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

# --- Enhanced RAG pipeline with query decomposition ---
def run_rag_evaluation(num_samples: int = 200):
    """Run RAG evaluation with query decomposition and multi-hop reasoning"""
    # Load preprocessed data
    print("Loading embeddings and initializing BM25...")
    embeddings, chunk_info, bm25 = load_embeddings()
    
    # Load dataset
    print("Loading dataset...")
    with open("hotpotqa_longbench.json", "r") as f:
        dataset = json.load(f)[:num_samples]
    
    results = []
    
    for item in tqdm(dataset, desc="Processing questions"):
        # 1. Query decomposition
        sub_questions, reasoning = decompose_query(item['question'])
        
        # 2. Retrieve relevant chunks with hybrid approach (now using sub-questions)
        retrieved_chunks = retrieve_relevant_chunks(
            query=item['question'],
            sub_questions=sub_questions,
            embeddings=embeddings,
            chunk_info=chunk_info,
            bm25=bm25,
            top_k=5  # Retrieve more chunks for multi-hop reasoning
        )
        
        # 3. Build context with decomposition info
        context = "### Original Question:\n" + item['question'] + "\n\n"
        context += "### Sub-Questions and Reasoning:\n"
        context += f"Reasoning: {reasoning}\n"
        for i, sq in enumerate(sub_questions):
            context += f"{i+1}. {sq}\n"
        
        context += "\n### Retrieved Context:\n"
        context += "\n\n".join([
            f"Document {i+1} (Relevance: {chunk['similarity']:.2f}, "
            f"Matched to: '{chunk['query']}'):\n{chunk['text']}"
            for i, chunk in enumerate(retrieved_chunks)
        ])
        
        # 4. Construct multi-hop reasoning prompt
        messages = [
            {
                "role": "system",
                "content": "You are an assistant specialized in answering complex questions using sub-question decomposition. "
                           "Follow these steps:\n"
                           "1. Review the sub-questions and reasoning\n"
                           "2. Examine each retrieved document\n"
                           "3. For each sub-question, identify relevant information\n"
                           "4. Combine information to answer the original question\n"
                           "5. Provide ONLY the final answer without explanation"
            },
            {
                "role": "user",
                "content": f"{context}\n\n"
                        "Question: {item['question']}\n\n"
                        "Instructions:\n"
                        "- Answer using ONLY the provided context\n"
                        "- For Yes/No questions, respond with exactly 'Yes' or 'No'\n"
                        "- For others, provide ONLY the exact answer (name, date, number, or brief phrase)\n"
                        "- Do not include reasoning or explanations\n"
                        "- Final answer should be prefixed with 'Final Answer: '"
            }
        ]
        
        # 5. Query LLM with increased max_tokens
        predicted_answer = query_chat_model(messages, max_tokens=256)
        
        if predicted_answer is not None:
            # Extract final answer if present
            final_answer_match = re.search(r"Final Answer:\s*(.+)", predicted_answer, re.IGNORECASE)
            if final_answer_match:
                final_answer = final_answer_match.group(1).strip()
            else:
                # Fallback: take the first sentence if no explicit final answer
                final_answer = predicted_answer.split('.')[0].strip()
            
            results.append({
                'id': item['id'],
                'question': item['question'],
                'sub_questions': sub_questions,
                'reasoning': reasoning,
                'predicted_answer': final_answer,
                'full_response': predicted_answer,
                'golden_answer': item['answer'],
                'retrieved_chunks': [chunk['text'] for chunk in retrieved_chunks]
            })
    
    # 6. Evaluation
    em_scores = [compute_em(r['golden_answer'], r['predicted_answer']) for r in results]
    f1_scores = [compute_f1(r['golden_answer'], r['predicted_answer']) for r in results]
    
    avg_em = np.mean(em_scores)
    avg_f1 = np.mean(f1_scores)
    
    print("\n" + "="*50)
    print(f"Performance with Query Decomposition: EM = {avg_em:.3f}, F1 = {avg_f1:.3f}")
    print("="*50 + "\n")
    
    # 7. Save results
    os.makedirs("results", exist_ok=True)
    
    with open("results/decomposition_outputs.json", "w") as f:
        json.dump([{
            'id': r['id'], 
            'pred_answer': r['predicted_answer'],
            'sub_questions': r['sub_questions']
        } for r in results], f, indent=2)
    
    with open("results/decomposition_full_results.json", "w") as f:
        json.dump(results, f, indent=2)
    
    return results, avg_em, avg_f1

if __name__ == "__main__":
    run_rag_evaluation()