

import sys
import json
import re

def parse_catch2_benchmarks(input_path):
    with open(input_path, 'r') as f:
        lines = f.readlines()

    benchmarks = []
    current_benchmark = None

    # Regex patterns for parsing
    benchmark_name_re = re.compile(r'^benchmark name:\s*(.+)$', re.IGNORECASE)
    mean_time_re = re.compile(r'^mean:\s*([\d.]+)\s*(\w+)$', re.IGNORECASE)

    for line in lines:
        line = line.strip()

        match_name = benchmark_name_re.match(line)
        if match_name:
            current_benchmark = {"name": match_name.group(1)}
            continue

        match_time = mean_time_re.match(line)
        if match_time and current_benchmark:
            time_value = float(match_time.group(1))
            unit = match_time.group(2).lower()

            multiplier = {
                "ns": 1,
                "us": 1_000,
                "ms": 1_000_000,
                "s": 1_000_000_000,
            }.get(unit, 1)

            current_benchmark["mean_ns"] = int(time_value * multiplier)
            benchmarks.append(current_benchmark)
            current_benchmark = None

    return benchmarks

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 parse_benchmarks.py <input.txt> <output.json>")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]

    data = parse_catch2_benchmarks(input_path)

    with open(output_path, 'w') as out:
        json.dump(data, out, indent=2)

if __name__ == "__main__":
    main()