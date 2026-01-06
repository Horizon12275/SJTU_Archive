from openai import OpenAI
from datasets import load_from_disk
import json
import argparse
import base64
from tqdm import tqdm
import os
from datetime import datetime

client = OpenAI(
    base_url="http://47.242.151.133:24576/v1/",
    api_key="ml2025",
)

def load_data(path):
    ds = load_from_disk(path)
    return ds

def preprocess_image(example):
    image = example["image"]
    return image

def log_errors(results, log_file="./results/error_log.json"):
    """
    记录错误答案到文件
    Args:
        results: list, 所有结果
        log_file: str, 错误日志文件路径
    """
    errors = []
    for result in results:
        lower_answers = [answer.lower() for answer in result["answers"]]
        if result["generation"].lower() not in lower_answers:
            error_entry = {
                "question": result.get("question", "N/A"),  # 如果results中有问题文本
                "generated_answer": result["generation"],
                "correct_answers": result["answers"],
                "timestamp": datetime.now().isoformat()
            }
            errors.append(error_entry)
    
    if errors:
        os.makedirs(os.path.dirname(log_file), exist_ok=True)
        with open(log_file, "w") as f:
            json.dump(errors, f, indent=2)
        print(f"记录 {len(errors)} 个错误答案到 {log_file}")

def generate_answer(example):
    image = preprocess_image(example)
    
    tmp_dir = "./tmp"
    os.makedirs(tmp_dir, exist_ok=True)
    tmp_file = os.path.join(tmp_dir, "tmp_image.png")
    image.save(tmp_file, format="PNG")

    with open(tmp_file, "rb") as f:
        encoded_image = base64.b64encode(f.read())
    encoded_image_text = encoded_image.decode("utf-8")
    base64_image = f"data:image;base64,{encoded_image_text}"

    text = f"{example['question']}\nOnly return the answer, no other words."

    chat_response = client.chat.completions.create(
        model="Qwen/Qwen2.5-VL-3B-Instruct",
        messages=[
            {"role": "system", "content": "You are a helpful assistant."},
            {
                "role": "user",
                "content": [
                    {"type": "image_url", "image_url": {"url": base64_image}},
                    {"type": "text", "text": text},
                ],
            },
        ],
    )
    return chat_response.choices[0].message.content

def evaluate_results(results):
    score = 0
    for result in results:
        lower_answers = [answer.lower() for answer in result["answers"]]
        if result["generation"].lower() in lower_answers:
            score += 1
    return round(score / len(results), 2)

def main(args):
    if not args.eval_only:
        ds = load_data(args.data_path)
        results = []
        for example in tqdm(ds, desc="Generating answers", total=len(ds)):
            answer = generate_answer(example)
            # 将问题文本也存入结果中便于错误分析
            result_entry = {
                "question": example.get("question", "N/A"),
                "generation": answer,
                "answers": example['answers']
            }
            results.append(result_entry)

        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")

        output_dir = os.path.dirname(args.output_path)
        os.makedirs(output_dir, exist_ok=True)
        with open(args.output_path, "w") as f:
            json.dump(results, f)
        
        # 记录错误答案
        log_errors(results)
    else:
        with open(args.output_path, "r") as f:
            results = json.load(f)
        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")
        log_errors(results)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_path", type=str, default="./data/docvqa_100")
    parser.add_argument("--output_path", type=str, default="./results/docvqa_100_results.json")
    parser.add_argument("--error_log", type=str, default="./results/error_log.json")
    parser.add_argument("--eval_only", action="store_true")
    args = parser.parse_args()

    main(args)