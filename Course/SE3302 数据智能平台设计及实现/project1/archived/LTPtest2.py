import torch
from ltp import LTP
from ltp import StnSplit

print(torch.__version__)
print(torch.cuda.is_available())

# 分句
ltp= LTP() # 默认加载 LTP/Small 模型
sents = StnSplit().split("汤姆生病了。他去了医院。")
print(sents)
# [
#   "汤姆生病了。",
#   "他去了医院。"
# ]
sents = StnSplit().batch_split(["他叫汤姆去拿外衣。", "汤姆生病了。他去了医院。"])
print(sents)
# [
#   "他叫汤姆去拿外衣。",
#   "汤姆生病了。",
#   "他去了医院。"
# ]

# 用户自定义词典 貌似没法这么用
# 也可以在代码中添加自定义的词语
# ltp.add_words(word="长江大桥", freq = 2)

# 分词
words = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws"], return_dict = False)
print(words)
# [['他', '叫', '汤姆', '去', '拿', '外衣', '。']]

# 词性标注
result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","pos"])
print(result.pos)
# [['他', '叫', '汤姆', '去', '拿', '外衣', '。']]
# [['r', 'v', 'nh', 'v', 'v', 'n', 'wp']]

# 命名实体识别
result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","ner"])
print(result.ner)
# [[('Nh', '汤姆', 2, 2)]]

# 语义角色标注
result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","srl"])
print(result.srl)
# [[{'index': 1, 'predicate': '叫', 'arguments': 
# [('A0', '他', 0, 0), ('A1', '汤姆', 2, 2), ('A2', '去拿外衣', 3, 5)]}, 
# {'index': 4, 'predicate': '拿', 'arguments': 
# [('A0', '汤姆', 2, 2), ('A1', '外衣', 5, 5)]}]]

# 依存句法分析
result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","dep"])
print(result.dep)
# [{'head': [2, 0, 2, 5, 2, 5, 2], 
# 'label': ['SBV', 'HED', 'DBL', 'ADV', 'VOB', 'VOB', 'WP']}]

# 语义依存分析(树)
result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","sdp"])
print(result.sdp)
# [{'head': [2, 0, 2, 2, 4, 5, 2], 
# 'label': ['AGT', 'Root', 'DATV', 'eSUCC', 'eSUCC', 'PAT', 'mPUNC']}]

# 语义依存分析(图)
result = ltp.pipeline(["他叫汤姆去拿外衣。"], tasks = ["cws","sdpg"])
print(result.sdpg)
# [[(1, 2, 'AGT'), (2, 0, 'Root'), (3, 2, 'DATV'), (3, 4, 'AGT'), 
# (3, 5, 'AGT'), (4, 2, 'eSUCC'), (5, 2, 'eSUCC'), (5, 4, 'eSUCC'), 
# (6, 5, 'PAT'), (7, 2, 'mPUNC')]]