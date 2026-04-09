from fastapi import FastAPI
from api.routers import router

app = FastAPI(
    title="Glamour Main Server",
    description="클라이언트 요청을 받아 AI 서버로 넘기고, 결과를 DB에 저장하는 메인 서버 (Layered Architecture)"
)

# API 라우터 등록
app.include_router(router)
