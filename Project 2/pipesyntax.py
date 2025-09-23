import json

def generate_pipe_syntax(qep_json):
    """
    Convert a PostgreSQL query execution plan to pipe syntax SQL.
    """
    try:
        # Log the input QEP JSON
        # print("Input QEP JSON:")
        # print(qep_json)
        
        qep = json.loads(qep_json)  # Parse the JSON
        
        # Ensure to access the Plan key at the top level of the JSON
        pipe_lines = []
        _process_plan(qep["Plan"], pipe_lines)  # Use qep["Plan"] instead of qep[0]["Plan"]
        
        return "\n".join(pipe_lines)
    
    except json.JSONDecodeError as e:
        print(f"JSONDecodeError: {str(e)}")  # Log the error message
        return "Error: Could not parse QEP JSON"
    
    except Exception as e:
        print(f"Exception occurred: {str(e)}")  # Log the error message
        return f"Error generating pipe syntax: {str(e)}"

def _process_plan(plan, pipe_lines, indent_level=0):
    """
    Recursively process the query execution plan nodes
    """
    indent = "  " * indent_level  # Indentation for nested plans
    node_type = plan.get("Node Type", "Unknown Node")
    cost = plan.get("Total Cost", "N/A")
    
    # Handle different node types
    if node_type == "Seq Scan":
        table = plan.get("Relation Name", "unknown_table")
        condition = plan.get("Filter", "")
        line = f"{indent}FROM {table}"
        if condition:
            line += f" WHERE {condition}"
        pipe_lines.append(line + f" [Cost: {cost}]")

        # Process child plans
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)
    
    elif node_type == "Index Scan":
        table = plan.get("Relation Name", "unknown_table")
        index = plan.get("Index Name", "unknown_index")
        condition = plan.get("Index Cond", "")
        line = f"{indent}FROM {table} USING INDEX {index}"
        if condition:
            line += f" WHERE {condition}"
        pipe_lines.append(line + f" [Cost: {cost}]")

        # Process child plans
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)
    
    elif node_type == "Hash Join":
        join_type = plan.get("Join Type", "INNER").upper().replace(" ", "_")
        condition = plan.get("Hash Cond", "").replace("(", "").replace(")", "")
        line = f"{indent}➤ {join_type} JOIN ON {condition}"
        pipe_lines.append(line + f" [Cost: {cost}]")
        
        # Process child plans
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)
    
    elif node_type == "Nested Loop":
        join_type = plan.get("Join Type", "INNER").upper().replace(" ", "_")
        condition = plan.get("Join Filter", "")
        line = f"{indent}➤ {join_type} NESTED LOOP JOIN"
        if condition:
            line += f" ON {condition}"
        pipe_lines.append(line + f" [Cost: {cost}]")
        
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)
    
    elif node_type == "Aggregate":
        strategy = plan.get("Strategy", "").upper()
        group_key = plan.get("Group Key", [])
        line = f"{indent}➤ AGGREGATE"
        if strategy:
            line += f" {strategy}"
        if group_key:
            line += f" GROUP BY {', '.join(group_key)}"
        pipe_lines.append(line + f" [Cost: {cost}]")

        # Process child plans
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)
    
    elif node_type == "Sort":
        keys = plan.get("Sort Key", [])
        line = f"{indent}➤ ORDER BY " + ", ".join(keys)
        pipe_lines.append(line + f" [Cost: {cost}]")

        # Process child plans
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)
    
    elif node_type == "Limit":
        line = f"{indent}➤ LIMIT"
        pipe_lines.append(line + f" [Cost: {cost}]")

        # Process child plans
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)
    
    else:
        # Generic handling for other node types
        pipe_lines.append(f"{indent}➤ {node_type.upper()} [Cost: {cost}]")
        for child in plan.get("Plans", []):
            _process_plan(child, pipe_lines, indent_level + 1)

