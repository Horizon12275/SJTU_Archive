from openai import OpenAI
from datasets import load_from_disk
import json
import argparse
import base64
from tqdm import tqdm
import os
from paddleocr import PaddleOCR
import numpy as np
from PIL import Image
from sentence_transformers import SentenceTransformer
from sklearn.metrics.pairwise import cosine_similarity

# 初始化PaddleOCR
ocr = PaddleOCR(use_angle_cls=True, lang='en', show_log=False)
# 初始化句子嵌入模型
embedding_model = SentenceTransformer('all-MiniLM-L6-v2')

client = OpenAI(
    base_url="http://47.242.151.133:24576/v1/",
    api_key="ml2025",
)

def load_data(path):
    '''
    从磁盘加载数据
    '''
    ds = load_from_disk(path)
    return ds

def extract_text_from_image(image):
    '''
    使用PaddleOCR从图像中提取文本
    '''
    # 确保图像是RGB格式
    image = image.convert('RGB')
    img_array = np.array(image)
    
    # 确保数组有3个维度(高度, 宽度, 通道)
    if len(img_array.shape) == 2:
        img_array = np.expand_dims(img_array, axis=-1)
        img_array = np.repeat(img_array, 3, axis=-1)
    
    # 执行OCR
    result = ocr.ocr(img_array, cls=True)
    
    # 从所有检测到的框中提取文本，进行非空监测
    if not result:
        return ""
    
    for line in result:
        if not line:  # 检查line是否非空
            return ""

    text = ' '.join([line[1][0] for res in result for line in res if res])
    return text

def preprocess_image(example):
    '''
    使用OCR+RAG预处理图像，选择最相关的图像
    '''
    # 获取所有图像键(假设命名为image_1, image_2等)
    image_keys = [key for key in example.keys() if key.startswith('image_')]
    
    # 把为空的图像键排除
    image_keys = [key for key in image_keys if example[key] is not None]
    
    # 提取文本和生成嵌入
    texts = []
    embeddings = []
    for key in image_keys:
        image = example[key]
        text = extract_text_from_image(image)
        texts.append(text)
        embeddings.append(embedding_model.encode(text))
    
    # 为问题生成嵌入
    question_embedding = embedding_model.encode(example['question'])
    
    # 计算相似度分数
    similarities = cosine_similarity([question_embedding], embeddings)[0]
    
    # 按相似度排序
    scored_images = sorted(zip(image_keys, similarities), key=lambda x: x[1], reverse=True)
    
    # 选择最相关的图像(最多3个)
    top_images = scored_images[:3]
    print(f"选择的图像: {[img_key for img_key, _ in top_images]}")
    
    # 垂直拼接图像
    selected_images = [example[img_key] for img_key, _ in top_images]
    widths, heights = zip(*(img.size for img in selected_images))
    
    new_image = Image.new('RGB', (max(widths), sum(heights)))
    y_offset = 0
    for img in selected_images:
        new_image.paste(img, (0, y_offset))
        y_offset += img.size[1]
    
    return new_image

def generate_answer(example):
    '''
    为示例生成答案
    '''
    image = preprocess_image(example)
    
    # 转换为base64
    tmp_dir = "./tmp"
    os.makedirs(tmp_dir, exist_ok=True)
    tmp_file = os.path.join(tmp_dir, "tmp_image.png")
    image.save(tmp_file, format="PNG")

    with open(tmp_file, "rb") as f:
        encoded_image = base64.b64encode(f.read()).decode("utf-8")
    
    base64_image = f"data:image;base64,{encoded_image}"
    text = f"{example['question']}\n只返回答案，不要其他文字。"

    response = client.chat.completions.create(
        model="Qwen/Qwen2.5-VL-3B-Instruct",
        messages=[
            {"role": "system", "content": "你是一个有帮助的助手。"},
            {
                "role": "user",
                "content": [
                    {"type": "image_url", "image_url": {"url": base64_image}},
                    {"type": "text", "text": text},
                ],
            },
        ],
    )
    return response.choices[0].message.content

def evaluate_results(results):
    '''
    评估结果
    '''
    score = sum(1 for result in results if result["generation"].lower() in result["answers"].lower())
    return round(score / len(results), 2)

def main(args):
    if not args.eval_only:
        ds = load_data(args.data_path)
        results = []
        
        for example in tqdm(ds, desc="生成答案", total=len(ds)):
            print(example)
            answer = generate_answer(example)
            print(answer)
            results.append({"generation": answer, "answers": example['answers']})
        
        pass_rate = evaluate_results(results)
        print(f"通过率: {pass_rate}")
        
        os.makedirs(os.path.dirname(args.output_path), exist_ok=True)
        with open(args.output_path, "w") as f:
            json.dump(results, f)
    else:
        with open(args.output_path, "r") as f:
            results = json.load(f)
        pass_rate = evaluate_results(results)
        print(f"通过率: {pass_rate}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_path", default="./data/mp_docvqa_100")
    parser.add_argument("--output_path", default="./results/mp_docvqa_100_results_ocr_rag.json")
    parser.add_argument("--eval_only", action="store_true")
    args = parser.parse_args()
    main(args)