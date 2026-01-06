from openai import OpenAI
from datasets import load_from_disk
import json
import argparse
import base64
from tqdm import tqdm
import os
from PIL import Image, ImageEnhance, ImageFilter
from paddleocr import PaddleOCR
import numpy as np
import cv2

client = OpenAI(
    base_url="http://47.242.151.133:24576/v1/",
    api_key="ml2025",
)

def load_data(path):
    '''
    Load data from disk
    Args:
        path: str, the path to the data
    Returns:
        ds: Dataset, the data
    '''
    ds = load_from_disk(path)
    return ds

# Initialize PaddleOCR
ocr = PaddleOCR(use_angle_cls=True, lang='en', show_log=False)

def preprocess_image(example):
    image = example["image"]
    
    # 转换为灰度图像并增强对比度
    if image.mode != 'L':
        image = image.convert('L')
    
    # 可选：应用图像增强
    enhancer = ImageEnhance.Contrast(image)
    image = enhancer.enhance(2.0)
    
    return image

def extract_text_from_image(image):
    img_array = np.array(image)
    
    # 确保是8-bit无符号整数
    if img_array.dtype != np.uint8:
        if img_array.dtype == bool:
            img_array = img_array.astype(np.uint8) * 255
        else:
            img_array = img_array.astype(np.uint8)
    
    # 确保是2D或3D数组(灰度或RGB)
    if len(img_array.shape) == 2:
        img_array = cv2.cvtColor(img_array, cv2.COLOR_GRAY2BGR)
    
    try:
        result = ocr.ocr(img_array, cls=True)
        # ... rest of the function ...
    except Exception as e:
        print(f"OCR处理出错: {e}")
        return ""

def generate_answer(example):
    image = preprocess_image(example)
    ocr_text = extract_text_from_image(image)
    
    # Convert image to base64
    tmp_dir = "./tmp"
    os.makedirs(tmp_dir, exist_ok=True)
    tmp_file = os.path.join(tmp_dir, "tmp_image.png")
    image.save(tmp_file, format="PNG")

    with open(tmp_file, "rb") as f:
        encoded_image = base64.b64encode(f.read())
    encoded_image_text = encoded_image.decode("utf-8")
    base64_image = f"data:image;base64,{encoded_image_text}"

    # Combined prompt strategy
    prompt = (
        "You are an expert document analyst. Below is OCR-extracted text from the document:\n"
        f"{ocr_text}\n\n"
        "Carefully examine both the original document image and the extracted text to "
        f"answer this question precisely: {example['question']}\n"
        "Focus on:\n"
        "- Exact matches from the text\n"
        "- Layout and visual elements in the image\n"
        "- Numerical values, names, dates, etc.\n"
        "Answer concisely with just the required information:"
    )

    chat_response = client.chat.completions.create(
        model="Qwen/Qwen2.5-VL-3B-Instruct",
        messages=[
            {"role": "system", "content": "You are a precise document analysis assistant."},
            {
                "role": "user",
                "content": [
                    {
                        "type": "image_url",
                        "image_url": {
                            "url": base64_image
                        },
                    },
                    {"type": "text", "text": prompt},
                ],
            },
        ],
        temperature=0.5,
    )
    return chat_response.choices[0].message.content


def evaluate_results(results):
    '''
    Evaluate the results
    Args:
        results: list, the results
    Returns:
        score: float, the score
    '''
    # Calculate the score
    score = 0
    for result in results:
        lower_answers = [answer.lower() for answer in result["answers"]]
        if result["generation"].lower() in lower_answers:
            score += 1
    return round(score / len(results), 2)


def main(args):
    if not args.eval_only:
        # Load data
        ds = load_data(args.data_path)

        # Generate
        results = []
        for example in tqdm(ds, desc="Generating answers", total=len(ds), leave=True, position=0):
            print(example)
            answer = generate_answer(example)
            print(answer)
            results.append({"generation": answer, "answers": example['answers']})

        # Evaluate
        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")

        # Save results to disk
        output_dir = os.path.dirname(args.output_path)
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        with open(args.output_path, "w") as f:
            json.dump(results, f)
    else:
        # Load results
        with open(args.output_path, "r") as f:
            results = json.load(f)
        # Evaluate
        pass_rate = evaluate_results(results)
        print(f"Pass rate: {pass_rate}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_path", type=str, default="./data/docvqa_100")
    parser.add_argument("--output_path", type=str, default="./results/docvqa_100_results_combined_strategy.json")
    parser.add_argument("--eval_only", action="store_true")
    args = parser.parse_args()

    main(args)
