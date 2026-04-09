from fastapi import APIRouter
from schemas.requests import ClientRequest, RetryRequest
from services.grammar_service import GrammarService

router = APIRouter()
grammar_service = GrammarService()

@router.post("/process-text")
def process_client_text(request: ClientRequest):
    return grammar_service.process_text(request)

@router.post("/update-retries")
def update_retries(request: RetryRequest):
    return grammar_service.update_retries(request.log_id, request.retry_count)
