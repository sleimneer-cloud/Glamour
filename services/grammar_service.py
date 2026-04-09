import requests
from core.config import AI_SERVER_URL
from schemas.requests import ClientRequest
from repositories.grammar_repository import GrammarRepository

class GrammarService:
    def __init__(self):
        self.repository = GrammarRepository()

    def process_text(self, request: ClientRequest) -> dict:
        # 1. 과거 에러 이력 조회
        past_errors_text = self.repository.get_past_errors(request.user_id, request.error_limit)
        
        # 2. AI 서버 통신
        try:
            ai_response = requests.post(
                AI_SERVER_URL,
                json={
                    "text": request.text, 
                    "situation": request.situation,
                    "past_errors": past_errors_text,
                    "intent": request.intent
                }
            )
            ai_data = ai_response.json()
        except Exception as e:
            return {"status": "error", "message": f"AI 서버 통신 실패: {str(e)}"}

        if ai_data.get("status") != "success": 
            return {"status": "error", "message": ai_data.get("message", "AI 분석 실패")}

        result = ai_data.get("data", {})
        feedback = result.get("feedback", {})
        error_category = result.get("error_category", "")

        # 3. DB 저장
        try:
            log_id = self.repository.save_log_and_feedback(
                user_id=request.user_id,
                original_text=result.get("original", ""),
                corrected_text=result.get("corrected", ""),
                error_category=error_category,
                feedback=feedback
            )
        except Exception as e:
            return {"status": "error", "message": f"DB 저장 실패: {str(e)}"}

        return {
            "status": "success",
            "message": "맞춤형 AI 분석 및 DB 저장 완료!",
            "data": result,
            "past_errors_used": past_errors_text,
            "log_id": log_id  
        }

    def update_retries(self, log_id: int, retry_count: int) -> dict:
        try:
            self.repository.update_retries(log_id, retry_count)
            return {"status": "success", "message": "오답 횟수 업데이트 완료!"}
        except Exception as e:
            return {"status": "error", "message": f"오답 업데이트 실패: {str(e)}"}
