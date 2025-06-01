import pandas as pd
import matplotlib.pyplot as plt
import numpy as np # Для вычисления позиций столбцов

# Данные о производительности процессоров
# Этот список словарей содержит результаты тестов для различных процессоров
# при разном количестве потоков.
# 'Processor': Название процессора
# 'Threads': Количество задействованных потоков
# 'OM_Time': Время выполнения для теста OM (в секундах)
# 'SA_Time': Время выполнения для теста SA (в секундах)
data = [
    {'Processor': 'Intel Core i9-13900K', 'Threads': 1, 'OM_Time': 165.998, 'SA_Time': 7.6108},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 2, 'OM_Time': 172.821, 'SA_Time': 1.7591},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 3, 'OM_Time': 173.704, 'SA_Time': 1.7450},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 4, 'OM_Time': 173.420, 'SA_Time': 1.3633},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 5, 'OM_Time': 167.302, 'SA_Time': 1.1606},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 6, 'OM_Time': 179.375, 'SA_Time': 1.0053},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 7, 'OM_Time': 177.839, 'SA_Time': 1.0019},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 8, 'OM_Time': 170.422, 'SA_Time': 0.9992},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 9, 'OM_Time': 171.258, 'SA_Time': 0.9981},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 10, 'OM_Time': 173.748, 'SA_Time': 0.9709},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 11, 'OM_Time': 181.269, 'SA_Time': 0.9746},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 12, 'OM_Time': 179.265, 'SA_Time': 0.9603},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 13, 'OM_Time': 178.592, 'SA_Time': 0.9578},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 14, 'OM_Time': 173.257, 'SA_Time': 0.9528},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 15, 'OM_Time': 174.146, 'SA_Time': 0.9478},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 16, 'OM_Time': 167.549, 'SA_Time': 0.9428},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 17, 'OM_Time': 167.967, 'SA_Time': 0.8903},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 18, 'OM_Time': 161.175, 'SA_Time': 0.8778},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 19, 'OM_Time': 176.955, 'SA_Time': 0.8553},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 20, 'OM_Time': 159.841, 'SA_Time': 0.8328},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 21, 'OM_Time': 158.875, 'SA_Time': 0.6303},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 22, 'OM_Time': 180.061, 'SA_Time': 0.6102},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 23, 'OM_Time': 173.793, 'SA_Time': 0.5891},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 24, 'OM_Time': 179.338, 'SA_Time': 0.5230},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 25, 'OM_Time': 180.219, 'SA_Time': 0.5115},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 26, 'OM_Time': 181.192, 'SA_Time': 0.5000},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 27, 'OM_Time': 177.954, 'SA_Time': 0.4885},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 28, 'OM_Time': 166.130, 'SA_Time': 0.4170},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 29, 'OM_Time': 176.288, 'SA_Time': 0.4055},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 30, 'OM_Time': 178.632, 'SA_Time': 0.3540},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 31, 'OM_Time': 171.817, 'SA_Time': 0.3425},
    {'Processor': 'Intel Core i9-13900K', 'Threads': 32, 'OM_Time': 171.786, 'SA_Time': 0.3110},

    {'Processor': 'Intel Core i5-12400', 'Threads': 1, 'OM_Time': 228.78, 'SA_Time': 11.84313},
    {'Processor': 'Intel Core i5-12400', 'Threads': 2, 'OM_Time': 205.55, 'SA_Time': 3.1455},
    {'Processor': 'Intel Core i5-12400', 'Threads': 3, 'OM_Time': 207.25, 'SA_Time': 3.0223},
    {'Processor': 'Intel Core i5-12400', 'Threads': 4, 'OM_Time': 219.59, 'SA_Time': 2.79028},
    {'Processor': 'Intel Core i5-12400', 'Threads': 5, 'OM_Time': 219.04, 'SA_Time': 2.71200},
    {'Processor': 'Intel Core i5-12400', 'Threads': 6, 'OM_Time': 219.08, 'SA_Time': 2.197453},
    {'Processor': 'Intel Core i5-12400', 'Threads': 7, 'OM_Time': 210.11, 'SA_Time': 2.17898},
    {'Processor': 'Intel Core i5-12400', 'Threads': 8, 'OM_Time': 217.61, 'SA_Time': 2.03873},
    {'Processor': 'Intel Core i5-12400', 'Threads': 9, 'OM_Time': 225.80, 'SA_Time': 2.00546},
    {'Processor': 'Intel Core i5-12400', 'Threads': 10, 'OM_Time': 224.65, 'SA_Time': 1.90800},
    {'Processor': 'Intel Core i5-12400', 'Threads': 11, 'OM_Time': 207.40, 'SA_Time': 1.89340},
    {'Processor': 'Intel Core i5-12400', 'Threads': 12, 'OM_Time': 229.25, 'SA_Time': 1.83452},

    {'Processor': 'Intel Core i5-10400F', 'Threads': 1, 'OM_Time': 279.456, 'SA_Time': 12.84313},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 2, 'OM_Time': 291.123, 'SA_Time': 3.1490},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 3, 'OM_Time': 288.789, 'SA_Time': 2.9650},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 4, 'OM_Time': 294.001, 'SA_Time': 2.8585},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 5, 'OM_Time': 280.555, 'SA_Time': 2.8417},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 6, 'OM_Time': 292.345, 'SA_Time': 2.8274},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 7, 'OM_Time': 285.678, 'SA_Time': 2.5675},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 8, 'OM_Time': 290.987, 'SA_Time': 2.4387},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 9, 'OM_Time': 270.234, 'SA_Time': 2.3254},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 10, 'OM_Time': 287.890, 'SA_Time': 2.2180},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 11, 'OM_Time': 293.456, 'SA_Time': 2.2008},
    {'Processor': 'Intel Core i5-10400F', 'Threads': 12, 'OM_Time': 286.789, 'SA_Time': 2.1399},

    {'Processor': 'Intel Xeon X5680', 'Threads': 1, 'OM_Time': 485.123, 'SA_Time': 32.2033},
    {'Processor': 'Intel Xeon X5680', 'Threads': 2, 'OM_Time': 501.789, 'SA_Time': 31.0291},
    {'Processor': 'Intel Xeon X5680', 'Threads': 3, 'OM_Time': 490.567, 'SA_Time': 31.4650},
    {'Processor': 'Intel Xeon X5680', 'Threads': 4, 'OM_Time': 483.999, 'SA_Time': 31.4585},
    {'Processor': 'Intel Xeon X5680', 'Threads': 5, 'OM_Time': 504.111, 'SA_Time': 31.8711},
    {'Processor': 'Intel Xeon X5680', 'Threads': 6, 'OM_Time': 496.222, 'SA_Time': 30.9563},
    {'Processor': 'Intel Xeon X5680', 'Threads': 7, 'OM_Time': 488.333, 'SA_Time': 30.0775},
    {'Processor': 'Intel Xeon X5680', 'Threads': 8, 'OM_Time': 499.444, 'SA_Time': 29.3552},
    {'Processor': 'Intel Xeon X5680', 'Threads': 9, 'OM_Time': 484.555, 'SA_Time': 29.1227},
    {'Processor': 'Intel Xeon X5680', 'Threads': 10, 'OM_Time': 503.666, 'SA_Time': 28.6135},
    {'Processor': 'Intel Xeon X5680', 'Threads': 11, 'OM_Time': 492.777, 'SA_Time': 28.4155},
    {'Processor': 'Intel Xeon X5680', 'Threads': 12, 'OM_Time': 487.888, 'SA_Time': 28.2818},

    {'Processor': 'Intel Core i7-8600U', 'Threads': 1, 'OM_Time': 737.18, 'SA_Time': 15.4439},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 2, 'OM_Time': 708.64, 'SA_Time': 5.6211},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 3, 'OM_Time': 727.26, 'SA_Time': 5.2782},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 4, 'OM_Time': 719.03, 'SA_Time': 4.9870},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 5, 'OM_Time': 713.47, 'SA_Time': 4.8363},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 6, 'OM_Time': 737.33, 'SA_Time': 4.7900},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 7, 'OM_Time': 724.43, 'SA_Time': 4.7912},
    {'Processor': 'Intel Core i7-8600U', 'Threads': 8, 'OM_Time': 745.19, 'SA_Time': 4.3633},

    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 1, 'OM_Time': 337.085, 'SA_Time': 13.1583},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 2, 'OM_Time': 312.039, 'SA_Time': 3.6231},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 3, 'OM_Time': 322.966, 'SA_Time': 3.3233},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 4, 'OM_Time': 331.815, 'SA_Time': 2.7739},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 5, 'OM_Time': 323.198, 'SA_Time': 2.7235},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 6, 'OM_Time': 338.604, 'SA_Time': 2.6260},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 7, 'OM_Time': 318.339, 'SA_Time': 2.5235},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 8, 'OM_Time': 324.357, 'SA_Time': 2.45652},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 9, 'OM_Time': 313.659, 'SA_Time': 2.47323},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 10, 'OM_Time': 323.354, 'SA_Time': 2.5379},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 11, 'OM_Time': 316.371, 'SA_Time': 2.3661},
    {'Processor': 'AMD Ryzen 5 7535HS', 'Threads': 12, 'OM_Time': 338.241, 'SA_Time': 2.31333},

    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 1, 'OM_Time': 250.940, 'SA_Time': 12.999141},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 2, 'OM_Time': 255.135, 'SA_Time': 3.375539},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 3, 'OM_Time': 256.637, 'SA_Time': 3.275369},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 4, 'OM_Time': 255.057, 'SA_Time': 2.330764},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 5, 'OM_Time': 253.923, 'SA_Time': 2.853088},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 6, 'OM_Time': 249.392, 'SA_Time': 2.735806},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 7, 'OM_Time': 250.393, 'SA_Time': 2.56469},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 8, 'OM_Time': 255.239, 'SA_Time': 2.467956},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 9, 'OM_Time': 257.817, 'SA_Time': 2.423144},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 10, 'OM_Time': 259.677, 'SA_Time': 2.327546},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 11, 'OM_Time': 253.456, 'SA_Time': 2.704307},
    {'Processor': 'AMD Ryzen 5 7530U', 'Threads': 12, 'OM_Time': 250.298, 'SA_Time': 2.502371}
]

# Преобразование списка словарей в DataFrame pandas
df = pd.DataFrame(data)

def plot_processor_performance(dataframe, processors_to_plot, time_metric):
    """
    Строит столбчатую диаграмму производительности для выбранных процессоров и метрики времени.

    Args:
        dataframe (pd.DataFrame): DataFrame с данными о производительности.
        processors_to_plot (list): Список названий процессоров для отображения.
        time_metric (str): Метрика времени для отображения ('OM_Time' или 'SA_Time').
    """
    if not isinstance(processors_to_plot, list):
        processors_to_plot = [processors_to_plot]

    plt.style.use('seaborn-v0_8-whitegrid')
    fig, ax = plt.subplots(figsize=(15, 8)) # Увеличен размер для лучшего отображения столбцов

    # Фильтруем данные по выбранным процессорам, чтобы найти все релевантные количества потоков
    relevant_data_for_threads = dataframe[dataframe['Processor'].isin(processors_to_plot)]
    if relevant_data_for_threads.empty:
        print(f"Предупреждение: Данные для выбранных процессоров не найдены в DataFrame.")
        ax.set_title(f"Нет данных для {', '.join(processors_to_plot)}")
        plt.show()
        return
        
    # Определяем все уникальные значения количества потоков для выбранных процессоров
    # Сортируем их для упорядоченного отображения на оси X
    all_thread_counts = sorted(relevant_data_for_threads['Threads'].unique())
    
    num_processors = len(processors_to_plot)
    if num_processors == 0:
        print("Предупреждение: Список процессоров для построения графика пуст.")
        ax.set_title("Список процессоров пуст")
        plt.show()
        return

    # X-координаты для групп столбцов (например, 0, 1, 2 для количеств потоков 1, 2, 4)
    x_indices = np.arange(len(all_thread_counts))
    
    # Рассчитываем ширину столбца: делаем ее уже, если процессоров много
    # Общая ширина для группы столбцов (например, 0.8, чтобы оставить место между группами)
    total_group_width = 0.8 
    bar_width = total_group_width / num_processors
    
    # Итерируемся по каждому процессору для построения его столбцов
    for i, processor_name in enumerate(processors_to_plot):
        processor_data = dataframe[dataframe['Processor'] == processor_name].sort_values(by='Threads')
        
        if not processor_data.empty:
            # Для каждого общего значения количества потоков получаем метрику времени для текущего процессора
            # Если у процессора нет данных для определенного количества потоков, используем NaN.
            current_processor_times = []
            # Индексируем данные процессора по 'Threads' для удобного поиска
            processor_data_indexed = processor_data.set_index('Threads') 

            for tc in all_thread_counts:
                if tc in processor_data_indexed.index:
                    current_processor_times.append(processor_data_indexed.loc[tc, time_metric])
                else:
                    # Добавляем NaN, если у этого процессора нет данных для данного количества потоков
                    # plt.bar пропустит значения NaN, создавая пробелы, что является желаемым поведением.
                    current_processor_times.append(np.nan)
            
            # Рассчитываем смещение для столбцов текущего процессора внутри каждой группы
            # Это позиционирует столбцы рядом для разных процессоров при одинаковом количестве потоков
            offset = (i - num_processors / 2 + 0.5) * bar_width 
            bar_positions = x_indices + offset
            
            ax.bar(bar_positions, current_processor_times, width=bar_width, label=processor_name)
        else:
            print(f"Предупреждение: Данные для процессора '{processor_name}' не найдены (пропускается).")

    ax.set_xlabel("Количество потоков")
    ax.set_ylabel(f"{time_metric.replace('_', ' ')} (секунды)")
    ax.set_title(f"Производительность процессоров: {time_metric.replace('_', ' ')} vs Потоки (Столбчатая диаграмма)")
    
    # Устанавливаем метки на оси X в центре каждой группы столбцов, подписывая их количеством потоков
    ax.set_xticks(x_indices) 
    ax.set_xticklabels(all_thread_counts)

    # Корректируем пределы оси X для обеспечения видимости всех столбцов
    if len(x_indices) > 0:
         ax.set_xlim(x_indices[0] - 0.5, x_indices[-1] + 0.5) # Небольшой отступ по краям

    ax.legend(title="Процессор")
    plt.grid(True, axis='y', linestyle='--', alpha=0.7) # Делаем сетку по оси Y менее навязчивой
    plt.tight_layout() 
    plt.show()

# --- Примеры использования ---
if __name__ == "__main__":
    # Получаем список всех уникальных процессоров из данных
    all_processors = df['Processor'].unique().tolist()

    # 1. Построить график OM_Time для всех процессоров
    print("Построение графика OM_Time для всех процессоров...")
    plot_processor_performance(df, all_processors, 'OM_Time')

    # 2. Построить график SA_Time для всех процессоров
    print("Построение графика SA_Time для всех процессоров...")
    plot_processor_performance(df, all_processors, 'SA_Time')

    # 3. Построить график OM_Time для выбранных процессоров Intel
    intel_processors = ['Intel Core i9-13900K', 'Intel Core i5-12400', 'Intel Core i5-10400F']
    print(f"Построение графика OM_Time для {', '.join(intel_processors)}...")
    plot_processor_performance(df, intel_processors, 'OM_Time')

    # 4. Построить график SA_Time для выбранных процессоров AMD
    amd_processors = ['AMD Ryzen 5 7535HS', 'AMD Ryzen 5 7530U']
    print(f"Построение графика SA_Time для {', '.join(amd_processors)}...")
    plot_processor_performance(df, amd_processors, 'SA_Time')

    # 5. Построить график OM_Time для одного процессора
    single_processor = ['Intel Core i9-13900K'] # Должен быть списком для функции
    print(f"Построение графика OM_Time для {single_processor[0]}...")
    plot_processor_performance(df, single_processor, 'OM_Time')

    # 6. Построить график SA_Time для одного процессора
    print(f"Построение графика SA_Time для {single_processor[0]}...")
    plot_processor_performance(df, single_processor, 'SA_Time')

    # Пример сравнения Intel Core i9-13900K и AMD Ryzen 5 7535HS по OM_Time:
    custom_comparison = ['Intel Core i9-13900K', 'AMD Ryzen 5 7535HS']
    print(f"Построение графика OM_Time для {', '.join(custom_comparison)}...")
    plot_processor_performance(df, custom_comparison, 'OM_Time')