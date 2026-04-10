from fastapi import APIRouter, Request
from schemas.requests import ClientRequest, RetryRequest
from services.grammar_service import GrammarService
from core.limiter import limiter

router = APIRouter()
grammar_service = GrammarService()

@router.post("/process-text")
@limiter.limit("5/minute")
def process_client_text(request: Request, client_req: ClientRequest):
    return grammar_service.process_text(client_req)

@router.post("/update-retries")
@limiter.limit("15/minute")
def update_retries(request: Request, client_req: RetryRequest):
    return grammar_service.update_retries(client_req.log_id, client_req.retry_count)
