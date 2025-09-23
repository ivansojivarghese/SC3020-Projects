import streamlit as st
import psycopg2
from psycopg2 import OperationalError
from psycopg2 import sql
from pipesyntax import generate_pipe_syntax
from preprocessing import get_query_plan
import time

def init_page():
    st.set_page_config(
        page_title="SQL Pipe Syntax Converter",
        layout="wide",
        initial_sidebar_state="expanded"
    )
    st.title("SQL to Pipe Syntax Converter")
    st.markdown("""
    Convert your SQL queries to readable pipe syntax using PostgreSQL query execution plans.
    """)

def db_connection_sidebar():
    with st.sidebar:
        st.header("Database Connection")
        host = st.text_input("Host", value="localhost", key="db_host")
        database = st.text_input("Database", value="TPC-H", key="db_name")
        user = st.text_input("Username", value="postgres", key="db_user")
        password = st.text_input("Password", type="password", key="db_pass")
        
        col1, col2 = st.columns(2)
        with col1:
            if st.button("Connect", key="connect_btn"):
                try:
                    conn = psycopg2.connect(
                        host=host,
                        database=database,
                        user=user,
                        password=password
                    )
                    st.session_state.conn = conn
                    st.success("Connected successfully!")
                    time.sleep(1)
                    st.rerun()
                except OperationalError as e:
                    st.error(f"Connection failed: {str(e)}")
        with col2:
            if st.button("Disconnect", key="disconnect_btn"):
                if 'conn' in st.session_state:
                    st.session_state.conn.close()
                    del st.session_state.conn
                    st.info("Disconnected from database")
                    time.sleep(1)
                    st.rerun()

def query_input_section():
    st.subheader("SQL Query Input")
    query = st.text_area(
        "Enter your SQL query:", 
        height=200,
        placeholder="SELECT * FROM customers WHERE...",
        key="sql_query"
    )
    return query

def display_results(pipe_syntax, qep):
    tab1, tab2 = st.tabs(["Pipe Syntax Output", "Query Execution Plan"])
    
    with tab1:
        st.subheader("Converted Pipe Syntax")
        st.code(pipe_syntax, language="sql", line_numbers=True)
        
    with tab2:
        st.subheader("Query Execution Plan")
        st.json(qep, expanded=False)

def perform_rollback(conn):
    try:
        with conn.cursor() as cursor:
            cursor.execute("ROLLBACK;")
            conn.commit()
            st.info("Transaction rolled back due to an error.")
    except Exception as rollback_error:
        st.error(f"Failed to perform ROLLBACK: {str(rollback_error)}")

def main():
    init_page()
    db_connection_sidebar()
    
    if 'conn' not in st.session_state:
        st.warning("Please connect to a database first")
        return
    
    query = query_input_section()
    
    if st.button("Convert to Pipe Syntax", type="primary"):
        if not query.strip():
            st.error("Please enter a SQL query")
            return
            
        with st.spinner("Converting query..."):
            try:
                # Attempt to get the query plan
                qep = get_query_plan(st.session_state.conn, query)
                
                # If successful, generate pipe syntax
                pipe_syntax = generate_pipe_syntax(qep)
                display_results(pipe_syntax, qep)
                st.success("Conversion successful!")
                
            except psycopg2.DatabaseError as e:
                # If a database error occurs, perform a rollback
                st.error(f"Database error: {str(e)}")
                perform_rollback(st.session_state.conn)  # Perform ROLLBACK
                
            except Exception as e:
                # Catch other errors and display them
                st.error(f"Conversion failed: {str(e)}")

if __name__ == "__main__":
    main()
