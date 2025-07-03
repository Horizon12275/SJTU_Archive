import os
import shutil

# 设置路径
input_dir = './ResumesPDF'  # 输入的文件夹
output_base_dir = './ResumesPDF'  # 输出的基础文件夹

# 获取所有PDF文件
pdfs = [f for f in os.listdir(input_dir) if f.endswith('.pdf')]

# 生成输出文件夹
def create_output_folder(folder_name):
    output_path = os.path.join(output_base_dir, folder_name)
    if not os.path.exists(output_path):
        os.makedirs(output_path)
    return output_path

# 将PDF文件按每1000个一组移动到不同文件夹
def move_files():
    total_files = len(pdfs)
    group_size = 1000
    total_groups = (total_files // group_size) + (1 if total_files % group_size != 0 else 0)

    for i in range(total_groups):
        # 为每个组创建一个文件夹
        output_folder = create_output_folder(f'ResumesPDF_{i+1}')
        
        # 计算当前组的文件范围
        start_index = i * group_size
        end_index = min((i + 1) * group_size, total_files)
        
        # 移动文件到对应文件夹
        for j in range(start_index, end_index):
            file_name = pdfs[j]
            src_file = os.path.join(input_dir, file_name)
            dest_file = os.path.join(output_folder, file_name)
            shutil.move(src_file, dest_file)
            print(f"Moved: {file_name} to {output_folder}")

if __name__ == "__main__":
    move_files()
