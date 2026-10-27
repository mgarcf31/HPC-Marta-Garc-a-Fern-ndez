import matplotlib.pyplot as plt
import csv
import numpy as np
from collections import defaultdict

# Estructura de datos: datos[n][hilos] = tiempo
datos = defaultdict(dict)

# 1. Leer los datos del archivo CSV
with open('resultados_multiples.csv', mode='r', encoding='utf-8-sig') as f:
    reader = csv.DictReader(f)
    for row in reader:
        row = {k.strip(): v.strip() for k, v in row.items()}
        n = int(row['N'])
        hilos = int(row['Hilos'])
        tiempo = float(row['Tiempo'])
        datos[n][hilos] = tiempo

# Extraer listas de N e Hilos ordenadas
lista_n = sorted(datos.keys())
lista_hilos = sorted(list(next(iter(datos.values())).keys()))

# Configuración del gráfico de barras agrupadas
x = np.arange(len(lista_n))  # Posiciones en el eje X para cada N
ancho_barra = 0.2           # Ancho de cada barra dentro del grupo

plt.figure(figsize=(10, 6))

# Dibujar las barras para cada configuración de hilos
for i, h in enumerate(lista_hilos):
    tiempos_hilo = [datos[n][h] for n in lista_n]
    posicion_barras = x + (i - len(lista_hilos)/2 + 0.5) * ancho_barra
    plt.bar(posicion_barras, tiempos_hilo, width=ancho_barra, label=f'{h} hilos')

# Personalización del gráfico
plt.xlabel('Tamaño del problema (N)', fontweight='bold')
plt.ylabel('Tiempo de Ejecución (segundos)', fontweight='bold')
plt.title('Tiempo de Ejecución según N y Número de Hilos (OpenMP)', fontweight='bold')
plt.xticks(x, [f'N = {n}' for n in lista_n])
plt.yscale('log')  # Escala logarítmica para visualizar adecuadamente desde N=38 hasta N=44
plt.grid(True, which="both", axis='y', linestyle=':', alpha=0.6)
plt.legend(title="Configuración")

plt.tight_layout()

# Guardar la imagen en el disco y mostrarla
plt.savefig('grafica_barras_tiempos.png', dpi=300)
plt.show()