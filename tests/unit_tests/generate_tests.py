import json
import os
import random
from itertools import count
from pathlib import Path
from jinja2 import Environment, FileSystemLoader, StrictUndefined

from checkpoints import assert_eq, checkpoint, read_checkpoints

TEMPLATE_DIR = os.path.join(os.path.dirname(__file__), 'templates')
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), 'build', 'prysma_code_generation')


def get_combinations(t, group):
    combinations = []
    # Test values representing edge cases and standard bounds
    values = [0, 1, 2, 10, 50]
    
    def cast_v(v):
        if t == "bool": return "true" if float(v) != 0 else "false"
        if t == "float": return str(float(v))
        return str(int(v))

    for L in values:
        for R in values:
            if t == "bool":
                # Only addition (+) behaves as a logical OR broadly or simply skip complex math on bool
                continue
                
            if group == "addition":
                combinations.append({"id": f"add_{L}_{R}", "left": cast_v(L), "right": cast_v(R), "op": "+", "exp": cast_v(L + R)})
            elif group == "subtraction":
                if L >= R: # Avoid negatives to keep simple logic working uniformly if unsigned behavior unknown
                    combinations.append({"id": f"sub_{L}_{R}", "left": cast_v(L), "right": cast_v(R), "op": "-", "exp": cast_v(L - R)})
            elif group == "multiplication":
                combinations.append({"id": f"mul_{L}_{R}", "left": cast_v(L), "right": cast_v(R), "op": "*", "exp": cast_v(L * R)})
            elif group == "division_modulo":
                if R != 0:
                    exp_div = L / R if t == "float" else L // R
                    combinations.append({"id": f"div_{L}_{R}", "left": cast_v(L), "right": cast_v(R), "op": "/", "exp": cast_v(exp_div)})
                    if t != "float": # modulo is generally integer math only
                        combinations.append({"id": f"mod_{L}_{R}", "left": cast_v(L), "right": cast_v(R), "op": "%", "exp": cast_v(L % R)})

    return combinations

def get_random_equations(t, count=20):
    # Ensure reproducible tests (hash for strings can vary between Python runs, so use length or strict mapping)
    seeds = {"int32": 42, "int64": 43, "float": 44}
    rng = random.Random(seeds.get(t, 45))
    equations = []
    is_float = (t == "float")
    
    def generate_expr(depth):
        if depth == 0:
            v = rng.randint(1, 15)
            # Add decimal .0 if float to enforce type correctness implicitly in string
            return f"{float(v)}" if is_float else f"{v}"
        op = rng.choice(['+', '-', '*'])
        left = generate_expr(depth - 1)
        right = generate_expr(depth - 1)
        if rng.random() > 0.5:
            return f"({left} {op} {right})"
        return f"{left} {op} {right}"

    for i in range(count):
        while True:
            depth = rng.randint(1, 3)
            eq_str = generate_expr(depth)
            if len(eq_str) <= 50:
                break
                
        try:
            expected = eval(eq_str)
            if is_float:
                expected_str = str(float(expected))
            else:
                expected_str = str(int(expected))
            
            equations.append({
                "id": i,
                "expression": eq_str,
                "expected": expected_str
            })
        except Exception:
            pass
            
    return equations

def generate_tests():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    env = Environment(loader=FileSystemLoader(TEMPLATE_DIR), undefined=StrictUndefined)

    env.filters["from_json"] = json.loads

    output = Path(OUTPUT_DIR)
    generated = set()

    def write_test(filename, template, context):
        checkpoint_ids = count(1)

        def render_checkpoint(*, trace_positions):
            return checkpoint(next(checkpoint_ids), trace_positions=trace_positions)

        def render_assert_eq(expected, expression, *, trace_positions):
            return assert_eq(next(checkpoint_ids), expected, expression, trace_positions=trace_positions)

        source = template.render(checkpoint=render_checkpoint, assert_eq=render_assert_eq, **context)
        read_checkpoints(source, filename)
        target = output / filename
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(source + "\n", encoding="utf-8")
        generated.add(target)

    for template_path in sorted(Path(TEMPLATE_DIR).rglob("*.p.jinja")):
        template_name = template_path.relative_to(TEMPLATE_DIR).as_posix()
        base_name = template_path.name.removesuffix(".p.jinja")
        template = env.get_template(template_name)
        if template_name.startswith("scenarios/"):
            write_test(template_name.removesuffix(".jinja"), template, {})
            continue
        
        for t in ["bool", "int32", "int64", "float"]:
            if t == 'bool' and (base_name.startswith('fv_') or base_name in ['mem_negative', 'mem_extreme', 'mem_pass_two_ptr', 'operand_combinations', 'operand_random_equations', 'edge_case_divisions']):
                continue
            
            if t != 'bool' and base_name == 'edge_case_logic_bool':
                continue
                
            def cast_val(v):
                if t == "bool": return "true" if float(v) != 0 else "false"
                if t == "float": return str(float(v))
                return str(int(v))
            
            context = {
                "type_name": t,
                "typed": {"bool": bool, "int32": int, "int64": int, "float": float}[t],
                "val1": cast_val(1),
                "val2": cast_val(0),
                "val_extreme": "2147483" if t in ["int32", "int64", "bool"] else "0.001"
            }
            
            for v in [0, 1, 2, 3, 4, 5, 10, 11, 15, 20, 25.5, 26, 30, 42, 50, 70, 99, 100, 999]:
                key = f"val_{str(v).replace('.', '_')}" if isinstance(v, float) else f"val_{v}"
                context[key] = cast_val(v)
            
            if base_name == "operand_combinations":
                # Generate a separate file for each operations group to avoid one massive file that times out
                for group in ["addition", "subtraction", "multiplication", "division_modulo"]:
                    combs = get_combinations(t, group)
                    if combs:
                        context["combinations"] = combs
                        context["op_group_name"] = group
                        file_name = f"{base_name}_{group}_{t}.p"
                        write_test(file_name, template, context)
            elif base_name == "operand_random_equations":
                eqs = get_random_equations(t, 25)
                if eqs:
                    context["equations"] = eqs
                    file_name = f"{base_name}_{t}.p"
                    write_test(file_name, template, context)
            else:
                write_test(f"{base_name}_{t}.p", template, context)

    for stale in sorted(set(output.rglob("*.p")) - generated):
        stale.unlink()


if __name__ == "__main__":
    generate_tests()
