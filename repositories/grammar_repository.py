from core.database import DatabaseManager

class GrammarRepository:
    def __init__(self):
        self.db = DatabaseManager()

    def get_past_errors(self, user_id: int, error_limit: int) -> str:
        past_errors_text = "아직 기록이 없는 새로운 학생입니다."
        conn = None
        try:
            conn = self.db.get_connection()
            cursor = conn.cursor()

            fetch_sql = """
                SELECT error_category, COUNT(*) as cnt
                FROM (
                    SELECT error_category
                    FROM error_details
                    WHERE log_id IN (
                        SELECT log_id 
                        FROM grammar_logs 
                        WHERE user_id = %s
                    )
                    ORDER BY created_at DESC
                    LIMIT %s
                ) AS recent_errors
                GROUP BY error_category
                ORDER BY cnt DESC
                LIMIT 3 
            """
            cursor.execute(fetch_sql, (user_id, error_limit))
            rows = cursor.fetchall()

            if rows:
                error_list = [f"'{row[0]}' (총 {row[1]}회)" for row in rows]
                past_errors_text = ", ".join(error_list)

        except Exception as e:
            print(f"DB 조회 에러: {e}")
            past_errors_text = "과거 기록 조회 실패"
        finally:
            if conn and conn.open:
                conn.close()
                
        return past_errors_text

    def save_log_and_feedback(self, user_id: int, original_text: str, corrected_text: str, error_category: str, feedback: dict) -> int:
        conn = None
        inserted_log_id = -1
        try:
            conn = self.db.get_connection()
            cursor = conn.cursor()

            # 1. 그래머 로그 테이블 저장
            log_sql = "INSERT INTO grammar_logs (user_id, original_text, corrected_text) VALUES (%s, %s, %s)"
            cursor.execute(log_sql, (user_id, original_text, corrected_text))
            inserted_log_id = cursor.lastrowid 

            # 2. 에러 디테일 테이블 저장 
            safe_category = error_category[:255] if error_category else ""
            
            err_sql = """
                INSERT INTO error_details (log_id, error_category, strengths, improvements, cheer_up) 
                VALUES (%s, %s, %s, %s, %s)
            """
            cursor.execute(err_sql, (
                inserted_log_id, 
                safe_category, 
                feedback.get("strengths", ""),
                feedback.get("improvements", ""),
                feedback.get("cheer_up", "")
            ))

            conn.commit()
        except Exception as e:
            if conn:
                conn.rollback()
            raise e
        finally:
            if conn and conn.open:
                conn.close()
                
        return inserted_log_id

    def update_retries(self, log_id: int, retry_count: int):
        conn = None
        try:
            conn = self.db.get_connection()
            cursor = conn.cursor()
            
            update_sql = "UPDATE grammar_logs SET assembly_retries = %s WHERE log_id = %s"
            cursor.execute(update_sql, (retry_count, log_id))
            conn.commit()
        except Exception as e:
            if conn:
                conn.rollback()
            raise e
        finally:
            if conn and conn.open:
                conn.close()
