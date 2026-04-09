import os
from dotenv import load_dotenv

load_dotenv()

DB_CONFIG = {
    'host': os.environ.get('DB_HOST', '127.0.0.1'),
    'user': os.environ.get('DB_USER', 'glamour_admin'), 
    'password': os.environ.get('DB_PASSWORD', ''),       
    'database': os.environ.get('DB_DATABASE', 'glamour'),
    'charset': os.environ.get('DB_CHARSET', 'utf8mb4')
}
AI_SERVER_URL = os.environ.get('AI_SERVER_URL', 'http://127.0.0.1:8001/feedback')
