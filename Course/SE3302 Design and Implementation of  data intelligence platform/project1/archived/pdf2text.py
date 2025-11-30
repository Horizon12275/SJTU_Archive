from pypdf import PdfReader

def pdf2text(pdf_path, txt_output_path):
    # 打开输出的 txt 文件
    try:
        # 读取 PDF 文件
        reader = PdfReader(pdf_path)
        
        with open(txt_output_path, 'w', encoding='utf-8') as txt_file:
            # 遍历每一页
            for page_num in range(len(reader.pages)):
                # 获取当前页面的文本内容
                page = reader.pages[page_num]
                text = page.extract_text()

                # 如果提取到文本，则写入到文本文件
                if text:
                    txt_file.write(f'--- Page {page_num + 1} ---\n')
                    txt_file.write(text)
                    txt_file.write('\n\n')

        print(f'Text extracted and saved to {txt_output_path}')
    except Exception as e:
        print(f'Error: {e}')
        print(f'Failed to extract text from {pdf_path}')

# 遍历pdf文件夹下的所有pdf文件
import os
pdf_dir = './ResumesPDF'
# 设置输出的 txt 文件夹路径
txt_dir = './ResumesTXT'
os.makedirs(txt_dir, exist_ok=True)

for pdf_file in os.listdir(pdf_dir):
    if pdf_file.endswith('.pdf'):
        pdf_path = os.path.join(pdf_dir, pdf_file)
        txt_output_path = os.path.join(txt_dir, pdf_file.replace('.pdf', '.txt'))
        pdf2text(pdf_path, txt_output_path)