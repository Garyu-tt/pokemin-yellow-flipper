Pokémon Yellow graph v1

Static source-grounded graph of upstream pret/pokeyellow and Flipper-native sources. Not executable.

Files: pokemon_yellow_graph.sqlite3 (indexed nodes,edges,metadata), nodes.jsonl.gz, edges.jsonl.gz, world_map.dot, build_report.json.

Example: sqlite3 pokemon_yellow_graph.sqlite3 "SELECT kind,count(*) FROM edges GROUP BY kind;"

Rebuild: python3 /home/mirbi/projects/pokemon-yellow-native/graph_tools/build_yellow_graph.py

Next: typed CFG/SSA, bank-switch resolution, event predicates, trace-based execution parity, lowering to Tagma/ARM.
