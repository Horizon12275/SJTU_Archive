#!/bin/bash

# 设置输入和输出目录
input_dir="/ResumesPDF"
output_dir="/ResumesMD"

# 检查输入和输出目录是否存在
if [ ! -d "$input_dir" ]; then
  echo "输入目录 $input_dir 不存在"
  exit 1
fi

if [ ! -d "$output_dir" ]; then
  echo "输出目录 $output_dir 不存在，正在创建..."
  mkdir -p "$output_dir"
fi

# 遍历输入目录中的所有 PDF 文件
for pdf_file in "$input_dir"/*.pdf; do
  # 检查文件是否存在（避免没有 PDF 文件时出错）
  if [ ! -f "$pdf_file" ]; then
    echo "没有找到 PDF 文件"
    break
  fi

  # 提取文件名（去掉路径和扩展名）
  base_name=$(basename "$pdf_file" .pdf)

  # 生成输出文件路径
  output_file="$output_dir/$base_name.md"

  # 执行 magic-pdf 转换命令
  echo "正在转换 $pdf_file 到 $output_file ..."
  magic-pdf -p "$pdf_file" -o "$output_file"

  # 检查转换是否成功
  if [ $? -eq 0 ]; then
    echo "$pdf_file 转换成功！"
  else
    echo "$pdf_file 转换失败！"
  fi
done

echo "转换完成！"
