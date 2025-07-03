import torch
from ltp import LTP
from ltp import StnSplit

print(torch.__version__)
print(torch.cuda.is_available())

# 从txt文件中读取
with open("cv1.txt", "r") as f:
    cv1 = f.read()
print(cv1)

# # 分句
ltp= LTP() # 默认加载 LTP/Small 模型
sents = StnSplit().split(cv1)
print(sents)
# [
#   "汤姆生病了。",
#   "他去了医院。"
# ]
sents = StnSplit().batch_split([cv1])
print(sents)
# # [
# #   "他叫汤姆去拿外衣。",
# #   "汤姆生病了。",
# #   "他去了医院。"
# # ]

# # 分词
words = ltp.pipeline([cv1], tasks = ["cws"], return_dict = False)
print(words)
# # [['他', '叫', '汤姆', '去', '拿', '外衣', '。']]

# # 词性标注
result = ltp.pipeline([cv1], tasks = ["cws","pos"])
print(result.pos)
# # [['他', '叫', '汤姆', '去', '拿', '外衣', '。']]
# # [['r', 'v', 'nh', 'v', 'v', 'n', 'wp']]

# # 命名实体识别
# result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","ner"])
# print(result.ner)
# # [[('Nh', '汤姆', 2, 2)]]

# # 语义角色标注
# result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","srl"])
# print(result.srl)
# # [[{'index': 1, 'predicate': '叫', 'arguments': 
# # [('A0', '他', 0, 0), ('A1', '汤姆', 2, 2), ('A2', '去拿外衣', 3, 5)]}, 
# # {'index': 4, 'predicate': '拿', 'arguments': 
# # [('A0', '汤姆', 2, 2), ('A1', '外衣', 5, 5)]}]]
