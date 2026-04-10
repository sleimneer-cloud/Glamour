from fastapi import FastAPI
from api.routers import router
from core.limiter import limiter
from slowapi import _rate_limit_exceeded_handler
from slowapi.errors import RateLimitExceeded
app = FastAPI(
    title="Glamour Main Server",
    description="클라이언트 요청을 받아 AI 서버로 넘기고, 결과를 DB에 저장하는 메인 서버 (Layered Architecture)"
)

# API 라우터 등록
app.include_router(router)

# Rate limiter 주입 및 예외 핸들러 등록
app.state.limiter = limiter
app.add_exception_handler(RateLimitExceeded, _rate_limit_exceeded_handler)
