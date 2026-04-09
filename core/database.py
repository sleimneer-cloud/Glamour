import pymysql
from core.config import DB_CONFIG

class DatabaseManager:
    def get_connection(self):
        try:
            return pymysql.connect(**DB_CONFIG)
        except pymysql.MySQLError as e:
            print(f"Database connection error: {e}")
            raise e
