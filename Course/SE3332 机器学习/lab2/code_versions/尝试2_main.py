from openai import OpenAI
import json
import re
import string
import collections
import numpy as np
import os
from tqdm import tqdm
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.metrics.pairwise import cosine_similarity
from transformers import BertTokenizer, BertModel

# Configure OpenAI client to use remote vLLM server
client = OpenAI(base_url="http://47.242.151.133:12596/v1", api_key="abc123")

# --- Data Loading ---
def load_dataset(file_path="hotpotqa_longbench.json", num_samples=None):
    """Loads data from a JSON file.

    Args:
        file_path (str): Path to the JSON file.
        num_samples (int, optional): Number of samples to load. Loads all if None. Defaults to None.

    Returns:
        list: A list of dictionaries, each representing a data sample.
    """
    with open(file_path, "r") as f:
        data = json.load(f)
    if num_samples is not None:
        return data[:num_samples]
    return data

import torch

# Ensure CUDA is available and set device to GPU
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# Load tokenizer and model, move to GPU
tokenizer = BertTokenizer.from_pretrained('bert-base-uncased')
model = BertModel.from_pretrained('bert-base-uncased').to(device)

def bert_embedding(text):
    inputs = tokenizer(text, padding=True, truncation=True, max_length=512, return_tensors='pt')
    inputs = inputs.to(device)
    outputs = model(**inputs)
    # Take the mean of the last hidden state along the sequence dimension
    embeddings = outputs.last_hidden_state.mean(dim=1)
    return embeddings.detach().cpu().numpy().squeeze(0)  # Squeeze the batch dimension

# ... (rest of the code remains the same)

def bert_retrieve(query, documents):
    query_embedding = bert_embedding(query)
    document_embeddings = [bert_embedding(doc) for doc in documents]
    similarities = cosine_similarity(query_embedding.reshape(1, -1), document_embeddings).flatten()
    most_relevant_indices = similarities.argsort()[-5:][::-1]
    return [documents[i] for i in most_relevant_indices]

# --- Model Interaction ---
def query_chat_model(messages, max_tokens: int = 32):
    """Queries the chat model with the provided messages.

    Args:
        messages (list): A list of message dictionaries, following the OpenAI API format. 
                         Each dictionary should have 'role' (e.g., 'user', 'assistant') 
                         and 'content' keys.
        max_tokens (int, optional): The maximum number of tokens to generate in the response. 
                                    Defaults to 32 since we only need a short answer.
        temperature (float, optional): Controls the randomness of the output. Lower values (e.g., 0.2) 
                                     make the output more deterministic, while higher values 
                                     (e.g., 1.0) make it more random. Defaults to 0.7.

    Returns:
        str or None: The content of the model's response, or None if an error occurred.
    """
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

# --- Evaluation Functions ---
def normalize_answer(s):
    """Lower text and remove punctuation, articles and extra whitespace."""
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
    """Computes Exact Match score."""
    return int(normalize_answer(a_gold) == normalize_answer(a_pred))

def compute_f1(a_gold, a_pred):
    """Computes F1 score."""
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

# --- Score Calculation and Printing ---
def calculate_and_print_scores(results):
    """Calculates EM/F1 scores for results, prints them, and returns averages.

    Args:
        results (list): A list of dictionaries, each containing 'golden_answer' 
                        and 'predicted_answer'.

    Returns:
        tuple: A tuple containing the average EM and average F1 scores (avg_em, avg_f1).
    """
    em_scores = []
    f1_scores = []
    
    print("=" * 80)
    print("Individual Results:")
    print("=" * 80)

    for i, result in enumerate(results):
        gold = result['golden_answer']
        pred = result['predicted_answer']
        
        em = compute_em(gold, pred)
        f1 = compute_f1(gold, pred)
        
        result['em'] = em
        result['f1'] = f1
        em_scores.append(em)
        f1_scores.append(f1)

        print(f"--- Sample {i+1} (ID: {result['id']}) ---")
        print(f"Question: {result['question']}")
        print(f"Predicted Answer: {pred}")
        print(f"Golden Answer: {gold}")
        print(f"EM: {em}")
        print(f"F1: {f1:.4f}")
        print("-" * 40)

    avg_em = np.mean(em_scores) if em_scores else 0
    avg_f1 = np.mean(f1_scores) if f1_scores else 0

    print("=" * 80)
    print("Overall Scores:")
    print("=" * 80)
    print(f"Average EM: {avg_em:.4f} ({avg_em * 100:.2f}%)")
    print(f"Average F1: {avg_f1:.4f} ({avg_f1 * 100:.2f}%)")
    print("=" * 80)

    return avg_em, avg_f1


# --- Save Results Function ---
def save_results_to_json(results, output_path):
    """Saves the prediction results to a JSON file.

    Args:
        results (list): A list of result dictionaries, each containing 'id' and 'predicted_answer'.
        output_path (str): The path to the output JSON file.
    """
    output_data = [{'id': r['id'], 'pred_answer': r['predicted_answer']} for r in results]

    print(f"Saving results to {output_path}...")
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(output_data, f, ensure_ascii=False, indent=4)
    print("Results saved successfully.")


# Main execution logic
def run_evaluation(num_samples_to_run=200):
    dataset = load_dataset(num_samples=num_samples_to_run)
    results = []

    for i, item in enumerate(tqdm(dataset)):
        context = item['context']
        question = item['question']
        golden_answer = item['answer']

        # Split context into chunks
        chunks = context.split('\n')  # Assuming context is split by newlines

        # Retrieve relevant chunks using Bert
        query = question + " " + golden_answer  # Combine question and answer for better retrieval
        relevant_chunks = bert_retrieve(query, chunks)

        # Combine retrieved chunks into a single context
        combined_context = ' '.join(relevant_chunks)

        # Query the model
        query_to_model = f"Context: {combined_context}\nQuestion: {question}\nInstruction: Answer the question concisely."
        messages = [
            {
                "role": "system",
                "content": "You are a helpful assistant in answering multi-hop questions. "
                           "Answer based ONLY on the provided context."
            },
            {
                "role": "user",
                "content": f"{query_to_model}"
                            "Instruction: Based *only* on the provided context, answer the question concisely. "
                            "For Yes/No questions, answer only 'Yes' or 'No'. "
                            "For other questions, provide only the exact answer phrase (e.g., a name, date, number, or short phrase), "
                            "without forming a complete sentence."
            }
        ]
        predicted_answer = query_chat_model(messages)

        if predicted_answer is not None:
            results.append({
                'id': item['id'],
                'question': item['question'],
                'predicted_answer': predicted_answer,
                'golden_answer': golden_answer
            })
        else:
            print(f"Skipping sample {i+1} due to model query error.")

    # Calculate and print scores
    avg_em, avg_f1 = calculate_and_print_scores(results)
    print(f"Final Average EM: {avg_em:.4f}, Final Average F1: {avg_f1:.4f}")

    # Save results
    output_dir = "results"
    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, "outputs.json")
    save_results_to_json(results, output_path)

if __name__ == "__main__":
    run_evaluation(num_samples_to_run=200) # Run evaluation for 200 samples
