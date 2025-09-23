import psycopg2
import json

def get_query_plan(conn, query):
    """
    Get the query execution plan in json format from PostgreSQL
    """
    try:
        with conn.cursor() as cursor:
            #Get query execution plan in json format
            cursor.execute(f"EXPLAIN (FORMAT JSON) {query}")
            result = cursor.fetchone()
            return json.dumps(result[0][0], indent=2)
    except psycopg2.Error as e:
        raise Exception(f"Database error: {str(e)}")
    except Exception as e:
        raise Exception(f"Error getting query plan: {str(e)}")

def validate_query(conn, query):
    """
    Validate that the query has correct syntax
    """
    try:
        with conn.cursor() as cursor:
            cursor.execute(f"EXPLAIN {query}")
        return True
    except psycopg2.Error:
        return False