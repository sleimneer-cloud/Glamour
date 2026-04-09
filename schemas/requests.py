from pydantic import BaseModel

class ClientRequest(BaseModel):
    user_id: int = 1 
    text: str
    situation: str = "일반 비즈니스"
    intent: str
    error_limit: int = 10

class RetryRequest(BaseModel):
    log_id: int
    retry_count: int
