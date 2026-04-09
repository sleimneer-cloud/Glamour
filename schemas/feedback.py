from pydantic import BaseModel

class TextRequest(BaseModel):
    text: str
    situation: str = "일반 비즈니스"
    past_errors: str = "아직 기록이 없는 새로운 학생입니다."
    intent: str
