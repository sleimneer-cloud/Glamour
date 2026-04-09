from fastapi import FastAPI
from schemas.feedback import TextRequest
from services.grok_service import grok_service

app = FastAPI(
    title="Glamour AI Server",
    description="Grok API 기반 작문/어법 교정 서버입니다."
)

@app.post("/feedback")
async def generate_feedback(request: TextRequest):
    # 비즈니스 로직을 모두 service 계층으로 위임합니다.
    return grok_service.generate_feedback(request)