import sys

# Debug: Scrive su STDERR per capire se lo script parte
sys.stderr.write("DEBUG: Lo script è stato avviato!\n")
sys.stderr.flush()

# Scrive direttamente l'output HTTP minimo
print("Content-Type: text/plain")
print()
print("Hello from CGI script!")

# Debug: Scrive ancora su STDERR
sys.stderr.write("DEBUG: Lo script ha finito l'esecuzione!\n")
sys.stderr.flush()
