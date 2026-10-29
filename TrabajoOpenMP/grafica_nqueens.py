import matplotlib.pyplot as plt

# ---------------------------------------------------------------------------
# DATOS REALES DE TU EJECUCIÓN (N = 12)
# ---------------------------------------------------------------------------
hilos = [1, 2, 4, 8]  # Configuraciones de OMP_NUM_THREADS

# Tiempos de la versión paralela con tareas OpenMP
tiempos_paralelo = [3.092, 5.625, 5.993, 21.155] 

# Tiempo medio de la versión secuencial pura (referencia fija)
tiempo_secuencial_medio = sum([0.184, 0.170, 0.090, 0.096]) / 4  # ~0.135s

# ---------------------------------------------------------------------------
# CREACIÓN DE LA GRÁFICA DE TIEMPOS
# ---------------------------------------------------------------------------
plt.figure(figsize=(9, 6))

# Línea base: Versión Secuencial
plt.axhline(
    y=tiempo_secuencial_medio, 
    color='red', 
    linestyle='--', 
    linewidth=2, 
    label=f'Secuencial Promedio ({tiempo_secuencial_medio:.3f} s)'
)

# Línea de rendimiento: Versión Paralela con Tareas OpenMP
plt.plot(
    hilos, 
    tiempos_paralelo, 
    marker='o', 
    color='blue', 
    linestyle='-', 
    linewidth=2, 
    markersize=8, 
    label='Paralelo (OpenMP Tasks)'
)

# Etiquetar los puntos con sus tiempos exactos
for h, t in zip(hilos, tiempos_paralelo):
    plt.annotate(
        f'{t:.3f} s', 
        (h, t), 
        textcoords="offset points", 
        xytext=(0, 10), 
        ha='center', 
        fontsize=9, 
        weight='bold'
    )

# Configuración de ejes y títulos
plt.title('Comparativa de Tiempos de Ejecución N-Queens (N=12)', fontsize=14, pad=15)
plt.xlabel('Número de Procesadores / Hilos (OMP_NUM_THREADS)', fontsize=12)
plt.ylabel('Tiempo de Ejecución (segundos)', fontsize=12)
plt.xticks(hilos)
plt.ylim(0, max(tiempos_paralelo) + 3)
plt.grid(True, linestyle=':', alpha=0.6)
plt.legend(fontsize=11)

# Guardar la imagen en el directorio actual
plt.savefig('grafica_tiempos_nqueens.png', dpi=300, bbox_inches='tight')
plt.show()