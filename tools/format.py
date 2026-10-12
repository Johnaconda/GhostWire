#!/usr/bin/env python3
"""Format project sources only; never rewrite upstream third-party files."""
import argparse
from pathlib import Path
import shutil
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
tool = shutil.which('clang-format')
if not tool:
    parser.error('clang-format is required; install it or add it to PATH')
sources = sorted(p for directory in ['include', 'src', 'apps', 'tests']
                 for p in (root / directory).rglob('*') if p.suffix in {'.cpp', '.hpp'})
subprocess.run([tool, *(['--dry-run', '--Werror'] if args.check else ['-i']),
                *map(str, sources)], check=True)
