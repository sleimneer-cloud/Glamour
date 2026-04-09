import os
from dotenv import load_dotenv

load_dotenv()

GROK_API_KEY = os.getenv("GROK_API_KEY") # xAI 발급 키
GROK_MODEL_NAME = os.getenv("GROK_MODEL_NAME", "meta-llama/llama-4-scout-17b-16e-instruct") # Groq 모델 이름
