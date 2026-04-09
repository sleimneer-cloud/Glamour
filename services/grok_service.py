import json
from openai import OpenAI
from core.config import GROK_API_KEY, GROK_MODEL_NAME
from core.prompts import FEEDBACK_PROMPT_TEMPLATE
from schemas.feedback import TextRequest

class GrokService:
    def __init__(self):
        # Groq API는 OpenAI python SDK와 호환됩니다! (base_url만 바꿔주면 됩니다)
        self.client = OpenAI(
            api_key=GROK_API_KEY,
            base_url="https://api.groq.com/openai/v1"
        )

    def generate_feedback(self, request: TextRequest) -> dict:
        prompt = FEEDBACK_PROMPT_TEMPLATE.format(
            situation=request.situation,
            past_errors=request.past_errors,
            text=request.text,
            intent=request.intent
        )

        try:
            response = self.client.chat.completions.create(
                model=GROK_MODEL_NAME,
                messages=[
                    {"role": "system", "content": "You are a helpful assistant that outputs JSON format."},
                    {"role": "user", "content": prompt}
                ],
            )
            raw_text = response.choices[0].message.content.strip()
            
            import re
            
            # 먼저 ```json ... ``` 형태의 마크다운 블록이 있는지 확인하고 추출
            match = re.search(r'```(?:json)?(.*?)```', raw_text, re.DOTALL)
            if match:
                clean_text = match.group(1).strip()
            else:
                # 마크다운이 없다면 가장 바깥쪽 {} 형태를 찾아 추출
                first_brace = raw_text.find('{')
                last_brace = raw_text.rfind('}')
                if first_brace != -1 and last_brace != -1 and first_brace <= last_brace:
                    clean_text = raw_text[first_brace:last_brace+1]
                else:
                    clean_text = raw_text.strip()
            
            result_json = json.loads(clean_text)
            return {
                "status": "success",
                "data": result_json
            }
        except json.JSONDecodeError as e:
            print("\n" + "="*50)
            print("🚨 [AI Server] JSON 파싱 에러 발생!")
            print(f"오류 내용: {e}")
            if 'raw_text' in locals():
                print(f"AI가 보낸 원본 텍스트 (범인):\n{raw_text}")
            print("="*50 + "\n")
            return {"status": "error", "message": f"JSON 파싱 실패: {e}"}
        except Exception as e:
            return {"status": "error", "message": str(e)}

# 싱글톤 인스턴스
grok_service = GrokService()
