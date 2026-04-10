from slowapi import Limiter
from slowapi.util import get_remote_address

# Client의 IP 주소를 기준으로 Request 수를 제한하는 Limiter 객체 생성
limiter = Limiter(key_func=get_remote_address)
