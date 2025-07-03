import os

api_key = "sk-tcegYgeYLJyxYhzWRdPIxEvLqS8aotCcv35rASiIX79Ke368"
#api_key = "sk-x5x013KqfDXilu2h98RX6PmRbAkklEe7yDhH5dr6BXl9VD7m"
api_base = "https://api.chatanywhere.tech/v1"
os.environ["OPENAI_API_KEY"] = api_key
os.environ["OPENAI_API_BASE"] = api_base

from langchain_neo4j import Neo4jGraph

# Set the environment variables for the Neo4j connection
os.environ["NEO4J_URI"] = "bolt://localhost:7687"
os.environ["NEO4J_USERNAME"] = "neo4j"
os.environ["NEO4J_PASSWORD"] = "12345678"

graph = Neo4jGraph(refresh_schema=False)

from langchain_experimental.graph_transformers import LLMGraphTransformer
from langchain_openai import ChatOpenAI

# Create an instance of the ChatOpenAI class
llm = ChatOpenAI(temperature=0, model_name="gpt-4o-mini")
llm_transformer = LLMGraphTransformer(llm=llm)

from langchain_core.documents import Document

# 从文件夹中读取markdown文件
def read_markdown_files(folder_path):
    for file in os.listdir(folder_path):
        if file.endswith(".md"):
            with open(os.path.join(folder_path, file), "r") as f:
                text = f.read()

    return text

# 将文档转换为图文档
def MY_convert_to_graph_documents(text):

    documents = [Document(page_content=text)]

    # Restrict the allowed nodes and relationships
    # Location: 单独将“Location”作为一个节点类型，用于表示地理位置（城市、国家等），而不是作为属性。这样可以更好地支持地理位置的查询和分析。
    allowed_nodes = [
        "Person", "Organization", "Skill", "Certification", "Location"
    ]

    allowed_relationships = [
        ("Person", "WORKED_AT", "Organization"),
        ("Person", "HAS_SKILL", "Skill"),
        ("Person", "HAS_CERTIFICATION", "Certification"),
        ("Person", "LIVES_IN", "Location"),
        ("Certification", "RELATED_TO", "Skill"),
        ("Organization", "LOCATED_IN", "Location"),
    ]

    node_properties = [
        # Person
        "person_name", "person_email", "person_phone", "person_address", "person_gender", "person_age","person_national","person_project",
        # Organization
        "organization_name", "organization_type",
        # Skill
        "skill_name", "skill_type",
        # Certification
        # Certification
        "certification_name", "certification_issue_date", "certification_type",
        # Location
        "location_city", "location_country",
    ]

    llm_transformer_tuple = LLMGraphTransformer(
        llm=llm,
        allowed_nodes=allowed_nodes,
        allowed_relationships=allowed_relationships,
        node_properties=node_properties,
    )
    graph_documents_filtered = llm_transformer_tuple.convert_to_graph_documents(documents)
    # print(f"Nodes:{graph_documents_filtered[0].nodes}")
    # print(f"Relationships:{graph_documents_filtered[0].relationships}")

    # 添加索引的基础标签
    graph.add_graph_documents(graph_documents_filtered, baseEntityLabel=True, include_source=True)

# folder = "ResumesMD/cv (1).md/cv (1)/auto"
folder_num = 1

for folder_num in range(3201, 3400):
    folder = f"ResumesMD/cv ({folder_num}).md/cv ({folder_num})/auto"
    try:
        graph_documents = read_markdown_files(folder)
        MY_convert_to_graph_documents(graph_documents)
        print(f"Folder {folder_num} processed successfully")
    except Exception as e:
        print(e)
        print(f"Error in folder {folder_num}")
        continue