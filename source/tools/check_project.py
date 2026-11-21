#!/usr/bin/env python3
"""Offline R10 smoke suite; on mirbi-linux uses local portable TCC as fallback.
No hardware writes, no firmware flashes. A failing test terminates with nonzero.
"""
from pathlib import Path
import os, shutil, subprocess, sys, tempfile

ROOT=Path(__file__).resolve().parents[1]
TESTS=["test_core","test_negative","test_route04","test_battle05",
       "test_capture06","test_party07","test_effects08","test_turns09",
       "test_moves10","test_state_stress10","test_move_forget11",
       "test_type12","test_migration12", "test_pipeline12b","test_status12c","test_status12c2"]
tcc=Path("/tmp/yellow_build_tools/tcc/usr/bin/tcc")
flags=[
 "-B/tmp/yellow_build_tools/tcc/usr/lib/x86_64-linux-gnu/tcc",
 "-I/tmp/yellow_build_tools/libc/usr/include",
 "-I/tmp/yellow_build_tools/libc/usr/include/x86_64-linux-gnu",
 "-L/lib/x86_64-linux-gnu",
 "-L/tmp/yellow_build_tools/libc/usr/lib/x86_64-linux-gnu",
 "-I.",
]
cc=shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
env=os.environ.copy()
env["PKYELLOW_SOURCE"]=env.get("PKYELLOW_SOURCE",
    str(ROOT.parents[1]/"upstream/pokeyellow"))
print("Source:",ROOT,flush=True)
for name in TESTS:
    src=ROOT/"tests"/(name+".c")
    if tcc.is_file():
        cmd=[str(tcc),"-run",*flags,str(src)]
        subprocess.run(cmd,check=True,cwd=ROOT)
    elif cc:
        with tempfile.TemporaryDirectory() as directory:
            executable=Path(directory)/name
            subprocess.run([cc,"-std=c11","-O1","-Wall","-I.",
                            str(src),"-o",str(executable)],check=True,cwd=ROOT)
            subprocess.run([str(executable)],check=True,cwd=ROOT)
    else:
        raise SystemExit("No host C compiler; install cc/clang or provide TCC")
subprocess.run([sys.executable,str(ROOT/"tests/test_original_data.py")],
               check=True,cwd=ROOT,env=env)
subprocess.run([sys.executable,str(ROOT/"tests/test_type_source12.py")],
               check=True,cwd=ROOT,env=env)
print("R12C2_HOST_SMOKE_SUITE_PASS",flush=True)
